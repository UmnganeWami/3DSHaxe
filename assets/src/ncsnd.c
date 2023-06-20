#include <stdlib.h>
#include <string.h>
#include <3ds/types.h>
#include <3ds/result.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/allocator/mappable.h>
#include <3ds/os.h>
#include <3ds/ipc.h>
#include <3ds/synchronization.h>
#include <3ds/services/apt.h>

#include <ncsnd.h>

/// Creates a CSND timer value from a sample rate.
#define NCSND_TIMER(n) (0x3FEC3FC / ((u32)(n)))

static inline u32 NCSND_VOL(float vol, float pan)
{
	float rpan;
	u32 lvol, rvol;

	if (vol < 0.0f) vol = 0.0f;
	else if (vol > 1.0f) vol = 1.0f;

	rpan = (pan+1) / 2;
	if (rpan < 0.0f) rpan = 0.0f;
	else if (rpan > 1.0f) rpan = 1.0f;

	lvol = vol*(1-rpan) * 0x8000;
	rvol = vol*rpan * 0x8000;
	return lvol | (rvol << 16);
}

/// CSND loop modes.
enum
{
	NCSND_LOOPMODE_MANUAL = 0, ///< Manual loop.
	NCSND_LOOPMODE_NORMAL,     ///< Normal loop.
	NCSND_LOOPMODE_ONESHOT,    ///< Do not loop.
	NCSND_LOOPMODE_NORELOAD,   ///< Don't reload.
};

/// Creates a sound channel value from a channel number.
#define NCSND_SOUND_CHANNEL(n) ((u32)(n) & 0x1F)

/// Creates a sound format value from an encoding.
#define NCSND_SOUND_FORMAT(n) ((u32)(n) << 12)

/// Creates a sound loop mode value from a loop mode.
#define NCSND_SOUND_LOOPMODE(n) ((u32)(n) << 10)

/// Sound flags.
enum
{
	NCSND_SOUND_LINEAR_INTERP = BIT(6),                           ///< Linear interpolation.
	NCSND_SOUND_ENABLE = BIT(14),                                 ///< Enable sound.
};

/// Capture modes.
enum
{
	NCSND_CAPTURE_REPEAT = 0,           ///< Repeat capture.
	NCSND_CAPTURE_ONE_SHOT = BIT(0),    ///< Capture once.
	NCSND_CAPTURE_FORMAT_16BIT = 0,     ///< PCM16
	NCSND_CAPTURE_FORMAT_8BIT = BIT(1), ///< PCM8
	NCSND_CAPTURE_ENABLE = BIT(15),     ///< Enable capture.
};

/// Duty cycles for a PSG channel.
typedef enum
{
	NCSND_DutyCycle_0  = 7, ///< 0.0% duty cycle
	NCSND_DutyCycle_12 = 0, ///< 12.5% duty cycle
	NCSND_DutyCycle_25 = 1, ///< 25.0% duty cycle
	NCSND_DutyCycle_37 = 2, ///< 37.5% duty cycle
	NCSND_DutyCycle_50 = 3, ///< 50.0% duty cycle
	NCSND_DutyCycle_62 = 4, ///< 62.5% duty cycle
	NCSND_DutyCycle_75 = 5, ///< 75.0% duty cycle
	NCSND_DutyCycle_87 = 6  ///< 87.5% duty cycle
} NCSND_DutyCycle;

/// Channel info.
typedef union
{
	u32 value[3]; ///< Raw values.
	struct
	{
		u8 active;       ///< Channel active.
		u8 _pad1;        ///< Padding.
		u16 _pad2;       ///< Padding.
		s16 adpcmSample; ///< Current ADPCM sample.
		u8 adpcmIndex;   ///< Current ADPCM index.
		u8 _pad3;        ///< Padding.
		u32 unknownZero; ///< Unknown.
	};
} NCSND_ChnInfo;

vu32* ncsndSharedMem;
u32 ncsndSharedMemSize;
u32 ncsndChannels;
u32 ncsndOffsets[4];
Handle ncsndCSNDHandle;

static Handle ncsndMutex;
static Handle ncsndSharedMemBlock;

static int ncsndRefCount = 0;
static u32 ncsndCmdBlockSize = 0x2000;
static u32 ncsndCmdStartOff;
static u32 ncsndCmdCurOff;

static aptHookCookie ncsndCookie;
static bool ncsndCookieHooked = false;

static Result NCSND_Initialize(Handle* mutex, Handle* sharedMem, u32 sharedMemSize, u32* offsets)
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x1,5,0); // 0x10140
	cmdbuf[1] = sharedMemSize;
	memcpy(&cmdbuf[2], &offsets[0], 4*sizeof(u32));

	if(R_FAILED(ret = svcSendSyncRequest(ncsndCSNDHandle)))return ret;

	if(mutex) *mutex = cmdbuf[3];
	if(sharedMem) *sharedMem = cmdbuf[4];

	return (Result)cmdbuf[1];
}

static Result NCSND_Shutdown()
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x2,0,0); // 0x20000

	if(R_FAILED(ret = svcSendSyncRequest(ncsndCSNDHandle)))return ret;

	return (Result)cmdbuf[1];
}

static Result NCSND_ExecuteCommands(u32 offset)
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x3,1,0); // 0x30040
	cmdbuf[1] = offset;

	if(R_FAILED(ret = svcSendSyncRequest(ncsndCSNDHandle)))return ret;

	return (Result)cmdbuf[1];
}

static Result NCSND_PlaySoundDirectly(u32 channel, u32 priority)
{
	Result ret=0;

	// If the values are outside the range, CSND crashes.
	if (channel > 3 || priority > 31) return MAKERESULT(RL_PERMANENT, RS_INVALIDARG, RM_CSND, RD_OUT_OF_RANGE);

	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x4,2,0); // 0x40080
	cmdbuf[1] = channel;
	cmdbuf[2] = priority;

	if(R_FAILED(ret = svcSendSyncRequest(ncsndCSNDHandle)))return ret;

	return (Result)cmdbuf[1];
}

static Result NCSND_AcquireSoundChannels(u32* channelMask)
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x5,0,0); // 0x50000

	if(R_FAILED(ret = svcSendSyncRequest(ncsndCSNDHandle)))return ret;

	*channelMask = cmdbuf[2];

	return (Result)cmdbuf[1];
}

static Result NCSND_ReleaseSoundChannels(void)
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x6,0,0); // 0x60000

	if(R_FAILED(ret = svcSendSyncRequest(ncsndCSNDHandle)))return ret;

	return (Result)cmdbuf[1];
}

/*static Result NCSND_FlushDataCache(const void* adr, u32 size)
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0x9,2,2); // 0x90082
	cmdbuf[1] = (u32)adr;
	cmdbuf[2] = size;
	cmdbuf[3] = IPC_Desc_SharedHandles(1);
	cmdbuf[4] = CUR_PROCESS_HANDLE;

	if(R_FAILED(ret = svcSendSyncRequest(ncsndCSNDHandle)))return ret;

	return cmdbuf[1];
}*/

/*static Result NCSND_StoreDataCache(const void* adr, u32 size)
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xA,2,2); // 0xA0082
	cmdbuf[1] = (u32)adr;
	cmdbuf[2] = size;
	cmdbuf[3] = IPC_Desc_SharedHandles(1);
	cmdbuf[4] = CUR_PROCESS_HANDLE;

	if(R_FAILED(ret = svcSendSyncRequest(ncsndCSNDHandle)))return ret;

	return cmdbuf[1];
}*/

/*static Result NCSND_InvalidateDataCache(const void* adr, u32 size)
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xB,2,2); // 0xB0082
	cmdbuf[1] = (u32)adr;
	cmdbuf[2] = size;
	cmdbuf[3] = IPC_Desc_SharedHandles(1);
	cmdbuf[4] = CUR_PROCESS_HANDLE;

	if(R_FAILED(ret = svcSendSyncRequest(ncsndCSNDHandle)))return ret;

	return cmdbuf[1];
}*/

/*static Result NCSND_Reset(void)
{
	Result ret=0;
	u32 *cmdbuf = getThreadCommandBuffer();

	cmdbuf[0] = IPC_MakeHeader(0xC,0,0); // 0xC0000

	if(R_FAILED(ret = svcSendSyncRequest(ncsndCSNDHandle)))return ret;

	return (Result)cmdbuf[1];
}*/

typedef enum
{
	NCSNDCMD_SETPLAYSTATE_R = 0x0,
	NCSNDCMD_SETPLAYSTATE = 0x1,
	NCSNDCMD_SETENCODING = 0x2,
	NCSNDCMD_SETLOOPBLOCK = 0x3,
	NCSNDCMD_SETLOOPMODE = 0x4,
	NCSNDCMD_SETBIT7 = 0x5,
	NCSNDCMD_SETINTERP = 0x6,
	NCSNDCMD_SETDUTY = 0x7,
	NCSNDCMD_SETTIMER = 0x8,
	NCSNDCMD_SETVOL = 0x9,
	NCSNDCMD_SETBLOCK = 0xA,
	NCSNDCMD_SETADPCMSTATE = 0xB,
	NCSNDCMD_SETLOOPADPCMSTATE = 0xC,
	NCSNDCMD_SETADPCMRELOAD = 0xD,
	NCSNDCMD_SETCHNPARAMS = 0xE,
	NCSNDCMD_SETCHNPARAMSPSG = 0xF,
	NCSNDCMD_SETCHNPARAMSNOISE = 0x10,

	NCSNDCMD_UPDATECHNINFO = 0x300
} NCSNDCmd_ID;

static u32* NCSNDCmd_Add(NCSNDCmd_ID cmdid)
{
	vu16* ptr;
	u32 prevoff;
	s32 outindex=0;

	svcWaitSynchronizationN(&outindex, &ncsndMutex, 1, 0, ~0);

	if (ncsndCmdStartOff != ncsndCmdCurOff)
	{
		if (ncsndCmdCurOff>=0x20)
			prevoff = ncsndCmdCurOff-0x20;
		else
			prevoff = ncsndCmdBlockSize-0x20;

		ptr = (vu16*)&ncsndSharedMem[prevoff>>2];
		*ptr = ncsndCmdCurOff;
	}

	ptr = (vu16*)&ncsndSharedMem[ncsndCmdCurOff>>2];

	ptr[0] = 0xFFFF;
	ptr[1] = cmdid;
	ptr[2] = 0;
	ptr[3] = 0;
	u32* ret = (u32*)&ptr[4];

	ncsndCmdCurOff += 0x20;
	if (ncsndCmdCurOff >= ncsndCmdBlockSize)
		ncsndCmdCurOff = 0;

	svcReleaseMutex(ncsndMutex);
	return ret;
}

static Result NCSNDCmd_Execute(bool waitDone)
{
	Result ret=0;

	// Check we actually wrote commands
	if (ncsndCmdStartOff == ncsndCmdCurOff)
		return ret;

	vu8* flag = (vu8*)&ncsndSharedMem[(ncsndCmdStartOff + 4) >> 2];

	ret = NCSND_ExecuteCommands(ncsndCmdStartOff);
	ncsndCmdStartOff = ncsndCmdCurOff;
	if (R_FAILED(ret)) return ret;

	while (waitDone && *flag == 0);

	return ret;
}

static void NCSNDCmd_SetPlayState(u32 channel, u32 value)
{
	u32* cmdparams = NCSNDCmd_Add(NCSNDCMD_SETPLAYSTATE);

	cmdparams[0] = channel & 0x1f;
	cmdparams[1] = value;
}

/*static void NCSNDCmd_SetEncoding(u32 channel, u32 value)
{
	u32* cmdparams = NCSNDCmd_Add(NCSNDCMD_SETENCODING);

	cmdparams[0] = channel & 0x1f;
	cmdparams[1] = value;
}*/

static void NCSNDCmd_SetBlock(u32 channel, int block, u32 physaddr, u32 size)
{
	u32* cmdparams = NCSNDCmd_Add(block ? NCSNDCMD_SETLOOPBLOCK : NCSNDCMD_SETBLOCK);

	cmdparams[0] = channel & 0x1f;
	cmdparams[1] = physaddr;
	cmdparams[2] = size;
}

/*static void NCSNDCmd_SetLooping(u32 channel, u32 value)
{
	u32* cmdparams = NCSNDCmd_Add(NCSNDCMD_SETLOOPMODE);

	cmdparams[0] = channel & 0x1f;
	cmdparams[1] = value;
}*/

/*static void NCSNDCmd_SetBit7(u32 channel, bool set)
{
	u32* cmdparams = NCSNDCmd_Add(NCSNDCMD_SETBIT7);

	cmdparams[0] = channel & 0x1f;
	cmdparams[1] = set ? 1 : 0;
}*/

/*static void NCSNDCmd_SetInterp(u32 channel, bool interp)
{
	u32* cmdparams = NCSNDCmd_Add(NCSNDCMD_SETINTERP);

	cmdparams[0] = channel & 0x1f;
	cmdparams[1] = interp ? 1 : 0;
}*/

/*static void NCSNDCmd_SetDuty(u32 channel, NCSND_DutyCycle duty)
{
	u32* cmdparams = NCSNDCmd_Add(NCSNDCMD_SETDUTY);

	cmdparams[0] = channel & 0x1f;
	cmdparams[1] = duty;
}*/

static void NCSNDCmd_SetTimer(u32 channel, u32 timer)
{
	u32* cmdparams = NCSNDCmd_Add(NCSNDCMD_SETTIMER);

	cmdparams[0] = channel & 0x1f;
	cmdparams[1] = timer;
}

static void NCSNDCmd_SetVol(u32 channel, u32 chnVolumes, u32 capVolumes)
{
	u32* cmdparams = NCSNDCmd_Add(NCSNDCMD_SETVOL);

	cmdparams[0] = channel & 0x1f;
	cmdparams[1] = chnVolumes;
	cmdparams[2] = capVolumes;
}

static void NCSNDCmd_SetAdpcmState(u32 channel, int block, int sample, int index)
{
	u32* cmdparams = NCSNDCmd_Add(block ? NCSNDCMD_SETLOOPADPCMSTATE : NCSNDCMD_SETADPCMSTATE);

	cmdparams[0] = channel & 0x1f;
	cmdparams[1] = sample & 0xFFFF;
	cmdparams[2] = index & 0x7F;
}

/*static void NCSNDCmd_SetAdpcmReload(u32 channel, bool reload)
{
	u32* cmdparams = NCSNDCmd_Add(NCSNDCMD_SETADPCMRELOAD);

	cmdparams[0] = channel & 0x1f;
	cmdparams[1] = reload ? 1 : 0;
}*/

static void NCSNDCmd_SetChnParams(u32 flags, u32 physaddr0, u32 physaddr1, u32 totalbytesize, u32 chnVolumes, u32 capVolumes)
{
	u32* cmdparams = NCSNDCmd_Add(NCSNDCMD_SETCHNPARAMS);

	cmdparams[0] = flags;
	cmdparams[1] = chnVolumes;
	cmdparams[2] = capVolumes;
	cmdparams[3] = physaddr0;
	cmdparams[4] = physaddr1;
	cmdparams[5] = totalbytesize;
}

/*static void NCSNDCmd_SetChnParamsPSG(u32 flags, u32 chnVolumes, u32 capVolumes, NCSND_DutyCycle duty)
{
	u32* cmdparams = NCSNDCmd_Add(NCSNDCMD_SETCHNPARAMSPSG);

	cmdparams[0] = flags;
	cmdparams[1] = chnVolumes;
	cmdparams[2] = capVolumes;
	cmdparams[3] = duty;
}*/

/*static void NCSNDCmd_SetChnRegsNoise(u32 flags, u32 chnVolumes, u32 capVolumes)
{
	u32* cmdparams = NCSNDCmd_Add(NCSNDCMD_SETCHNPARAMSNOISE);

	cmdparams[0] = flags;
	cmdparams[1] = chnVolumes;
	cmdparams[2] = capVolumes;
}*/

static void NCSNDCmd_UpdateInfo()
{
	NCSNDCmd_Add(NCSNDCMD_UPDATECHNINFO);
}

void ncsndNotifyAptEvent(APT_HookType event) 
{
	if (!ncsndRefCount) return;
	switch (event)
    {
    case APTHOOK_ONSUSPEND:
    case APTHOOK_ONEXIT:
    case APTHOOK_ONSLEEP:
        for (int i = 0; i < NCSND_NUM_CHANNELS; i++) 
		{
			if (!(ncsndChannels & BIT(i)))
				continue;
			
			NCSNDCmd_SetPlayState(i, 0);
		}
		NCSNDCmd_Execute(true);
        break;
    default:
        break;
    }
}

static void ncsndAptHook(APT_HookType hook, void* param)
{
    ncsndNotifyAptEvent(hook);
}

Result ncsndInit(bool doAptHook)
{
	Result ret=0;

	if (AtomicPostIncrement(&ncsndRefCount)) return ret;

	ret = srvGetServiceHandle(&ncsndCSNDHandle, "csnd:SND");
	if (R_FAILED(ret)) goto cleanup0;

	// Calculate offsets and sizes required by the CSND module
	ncsndOffsets[0] = ncsndCmdBlockSize;                         			// Offset to DSP semaphore and irq disable flags
	ncsndOffsets[1] = ncsndOffsets[0] + 8;                       			// Offset to sound channel information
	ncsndOffsets[2] = ncsndOffsets[1] + 32*sizeof(NCSND_ChnInfo); 			// Offset to capture unit information
	ncsndOffsets[3] = ncsndOffsets[2] + 2*8;                     			// Offset to the direct sound struct
	ncsndSharedMemSize = ncsndOffsets[3] + sizeof(ncsndDirectSound);      // Total size of the CSND shared memory

	ret = NCSND_Initialize(&ncsndMutex, &ncsndSharedMemBlock, ncsndSharedMemSize, ncsndOffsets);
	if (R_FAILED(ret)) goto cleanup1;

	ncsndSharedMem = (vu32*)mappableAlloc(ncsndSharedMemSize);
	if(!ncsndSharedMem)
	{
		ret = -1;
		goto cleanup1;
	}

	ret = svcMapMemoryBlock(ncsndSharedMemBlock, (u32)ncsndSharedMem, MEMPERM_READWRITE, MEMPERM_DONTCARE);
	if (R_FAILED(ret)) goto cleanup2;

	memset((void*)ncsndSharedMem, 0, ncsndSharedMemSize);

	ret = NCSND_AcquireSoundChannels(&ncsndChannels);
	if (R_FAILED(ret)) goto cleanup2;

    if (!ncsndCookieHooked && doAptHook)
    {
        aptHook(&ncsndCookie, ncsndAptHook, NULL);
        ncsndCookieHooked = true;
    }

	return ret;
cleanup2:
	svcCloseHandle(ncsndSharedMemBlock);
	if(ncsndSharedMem != NULL)
	{
		mappableFree((void*) ncsndSharedMem);
		ncsndSharedMem = NULL;
	}
cleanup1:
	svcCloseHandle(ncsndCSNDHandle);
cleanup0:
	AtomicDecrement(&ncsndRefCount);
	return ret;
}

void ncsndExit(void)
{
	if (AtomicDecrement(&ncsndRefCount)) return;

	NCSND_ReleaseSoundChannels();

	svcUnmapMemoryBlock(ncsndSharedMemBlock, (u32)ncsndSharedMem);
	svcCloseHandle(ncsndSharedMemBlock);

	NCSND_Shutdown();
	svcCloseHandle(ncsndCSNDHandle);

	if(ncsndSharedMem != NULL)
	{
		mappableFree((void*) ncsndSharedMem);
		ncsndSharedMem = NULL;
	}

	if (ncsndCookieHooked)
	{
		aptUnhook(&ncsndCookie);
		ncsndCookieHooked = false;
	}
}

void ncsndInitializeDirectSound(ncsndDirectSound* sound)
{
	memset(sound, 0, sizeof(ncsndDirectSound));

	sound->soundOutputMode = NCSND_SOUNDOUTPUT_STEREO;
	sound->soundModifiers.channelVolumes[0] = NCSND_DIRECTSOUND_MAX_VOLUME;
	sound->soundModifiers.channelVolumes[1] = NCSND_DIRECTSOUND_MAX_VOLUME;
	sound->soundModifiers.unknown1 = 1.f;
	sound->soundModifiers.speedMultiplier = 1.f;
}

Result ncsndPlayDirectSound(u32 chn, u32 priority, ncsndDirectSound* sound)
{

	ncsndDirectSound* sharedMemSound = (ncsndDirectSound*)((vu8*)ncsndSharedMem + ncsndOffsets[3]);
	memcpy(sharedMemSound, sound, sizeof(ncsndDirectSound));

	if (!sharedMemSound->channelData.isLeftPhys)
		if (sharedMemSound->channelData.leftSampleData) sharedMemSound->channelData.leftSampleData = (void*)osConvertVirtToPhys(sharedMemSound->channelData.leftSampleData);
		
	if (!sharedMemSound->channelData.isRightPhys)
		if (sharedMemSound->channelData.rightSampleData) sharedMemSound->channelData.rightSampleData = (void*)osConvertVirtToPhys(sharedMemSound->channelData.rightSampleData);
	
	sharedMemSound->channelData.isLeftPhys = sharedMemSound->channelData.isRightPhys = 0;

	return NCSND_PlaySoundDirectly(chn, priority);
}

void ncsndInitializeSound(ncsndSound* sound)
{
	memset(sound, 0, sizeof(ncsndSound));

	sound->volume = 1.f;
	sound->pitch = 1.f;
}

Result ncsndPlaySound(u32 chn, ncsndSound* sound)
{
	if (!(ncsndChannels & BIT(chn)))
		return 1;

	u32 flags = 0;
	void* sampleData = sound->sampleData;
	void* loopSampleData = sound->loopSampleData;

	if (!sound->isPhysAddr) {
		if (sampleData) sampleData = (void*)osConvertVirtToPhys(sampleData);
		if (loopSampleData) loopSampleData = (void*)osConvertVirtToPhys(loopSampleData);
	}

	if (sound->encoding == NCSND_ENCODING_ADPCM)
	{
		NCSNDCmd_SetAdpcmState(chn, 0, sound->context.data, sound->context.tableIndex);
		NCSNDCmd_SetAdpcmState(chn, 1, sound->loopContext.data, sound->loopContext.tableIndex);
	}

	u32 timer = NCSND_TIMER((u32)(sound->sampleRate * sound->pitch));
	if (timer < 0x0042) timer = 0x0042;
	else if (timer > 0xFFFF) timer = 0xFFFF;

	flags |= (sound->loopPlayback ? NCSND_SOUND_LOOPMODE(NCSND_LOOPMODE_NORMAL) : NCSND_SOUND_LOOPMODE(NCSND_LOOPMODE_ONESHOT));
	flags |= (sound->linearInterpolation ? NCSND_SOUND_LINEAR_INTERP : 0);
	flags |= NCSND_SOUND_ENABLE | NCSND_SOUND_CHANNEL(chn) | NCSND_SOUND_FORMAT(sound->encoding) | (timer << 16);

	u32 volumes = NCSND_VOL(sound->volume, sound->pan);
	NCSNDCmd_SetChnParams(flags, (u32)sampleData, (u32)loopSampleData, sound->totalSizeBytes, volumes, volumes);

	if (sound->loopPlayback && loopSampleData > sampleData)
	{
		// Now that the first block is playing, configure the size of the subsequent blocks
		NCSNDCmd_SetBlock(chn, 1, (u32)loopSampleData, sound->totalSizeBytes - (loopSampleData - sampleData));
	}

	return NCSNDCmd_Execute(true);
}

static inline u32 chnGetSharedMemIdx(u32 channel)
{
	return __builtin_popcount(((1<<channel)-1) & ncsndChannels);
}

static Result NCSND_GetState(u32 channel, NCSND_ChnInfo* out)
{
	Result ret = 0;
	channel = chnGetSharedMemIdx(channel);

	NCSNDCmd_UpdateInfo();

	if (R_FAILED(ret = NCSNDCmd_Execute(true))) return ret;

	memcpy(out, (const void*)&ncsndSharedMem[(ncsndOffsets[1] + channel*0xc) >> 2], 0xc);

	return 0;
}

void ncsndSetVolume(u32 chn, float volume, float pan)
{
	u32 volumes = NCSND_VOL(volume, pan);

	NCSNDCmd_SetVol(chn, volumes, volumes);
	NCSNDCmd_Execute(true);
}

void ncsndSetRate(u32 chn, u32 sampleRate, float pitch)
{
	u32 timer = NCSND_TIMER((u32)(sampleRate * pitch));
	if (timer < 0x0042) timer = 0x0042;
	else if (timer > 0xFFFF) timer = 0xFFFF;

	NCSNDCmd_SetTimer(chn, timer);
	NCSNDCmd_Execute(true);
}

void ncsndStopSound(u32 chn)
{
	if (!(ncsndChannels & BIT(chn)))
		return;
	
	NCSNDCmd_SetPlayState(chn, 0);
	NCSNDCmd_Execute(true);
}

bool ncsndIsPlaying(u32 chn)
{
	if (!(ncsndChannels & BIT(chn)))
		return false;
	
	NCSND_ChnInfo entry;

	NCSND_GetState(chn, &entry);

	return entry.active;
}
