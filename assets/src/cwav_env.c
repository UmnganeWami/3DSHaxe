#include "internal/cwav_env.h"
#include "3ds.h"
#include <string.h>
#include <stdlib.h>

#ifdef CWAV_DISABLE_DSP
#ifdef CWAV_DISABLE_CSND
#error "Cannot disable DSP and CSND at the same time!"
#endif
#endif

#ifndef CWAV_DISABLE_CSND
#include "ncsnd.h"
#define CWAVTOIMPL(c) ((cwav_t*)c->cwav)
#endif

#ifdef CWAV_DISABLE_DSP
#pragma message "DSP service support has been disabled!"
#endif
#ifdef CWAV_DISABLE_CSND
#pragma message "CSND service support has been disabled!"
#endif

#if !defined CWAV_DISABLE_DSP && defined CWAV_DISABLE_CSND
static const u32 g_currentEnv = CWAV_ENV_DSP;
#elif defined CWAV_DISABLE_DSP && !defined CWAV_DISABLE_CSND
static const u32 g_currentEnv = CWAV_ENV_CSND;
#else
static u32 g_currentEnv = CWAV_ENV_DSP;
#endif

#ifndef CWAV_DISABLE_DSP
static ndspWaveBuf* g_ndspWaveBuffers = NULL;
#endif

#ifndef CWAV_DISABLE_CSND
u32 cwav_defaultVAToPA(const void* addr)
{
    return osConvertVirtToPhys(addr);
}
vaToPaCallback_t cwavCurrentVAPAConvCallback = cwav_defaultVAToPA;
#else
vaToPaCallback_t cwavCurrentVAPAConvCallback = NULL;
#endif

void cwavEnvUseEnvironment(cwavEnvMode_t envMode)
{
#ifndef CWAV_DISABLE_CSND
#ifndef CWAV_DISABLE_DSP
    g_currentEnv = envMode;
#endif
#endif
}

cwavEnvMode_t cwavEnvGetEnvironment()
{
    return g_currentEnv;
}

void cwavEnvInitialize()
{

    if (g_currentEnv == CWAV_ENV_DSP)
    {
#ifndef CWAV_DISABLE_DSP
        g_ndspWaveBuffers = malloc(sizeof(ndspWaveBuf) * 24 * 2);
        memset(g_ndspWaveBuffers, 0, sizeof(ndspWaveBuf) * 24 * 2);
#endif
    }
}

void cwavEnvFinalize()
{
    if (g_currentEnv == CWAV_ENV_DSP)
    {
#ifndef CWAV_DISABLE_DSP
        if (g_ndspWaveBuffers)
            free(g_ndspWaveBuffers);
        g_ndspWaveBuffers = NULL;
#endif
    }
}

#ifndef CWAV_DISABLE_DSP
static inline ndspWaveBuf* cwavEnvGetNdspWaveBuffer(u32 channel, u32 block)
{
    return &g_ndspWaveBuffers[channel * 2 + block];
}
#endif

bool cwavEnvCompatibleEncoding(cwavEncoding_t encoding)
{
    if (encoding == PCM8 || encoding == PCM16)
        return true;

#ifndef CWAV_DISABLE_CSND
    if (encoding == IMA_ADPCM && g_currentEnv == CWAV_ENV_CSND)
        return true;
#endif

#ifndef CWAV_DISABLE_DSP
    if (encoding == DSP_ADPCM && g_currentEnv == CWAV_ENV_DSP)
        return true;
#endif

    return false;
}

u32 cwavEnvGetChannelAmount()
{
    if (g_currentEnv == CWAV_ENV_CSND) 
    {
#ifndef CWAV_DISABLE_CSND
        return NCSND_NUM_CHANNELS;
#endif
    }
    else if (g_currentEnv == CWAV_ENV_DSP)
    {
#ifndef CWAV_DISABLE_DSP
        return 24;
#endif
    }
    return 0;
}

bool cwavEnvIsChannelAvailable(u32 channel) 
{
    if (g_currentEnv == CWAV_ENV_CSND) 
    {
#ifndef CWAV_DISABLE_CSND
        return ((ncsndChannels >> channel) & 1);
#endif
    }
    else if (g_currentEnv == CWAV_ENV_DSP)
    {
#ifndef CWAV_DISABLE_DSP
        return true;
#endif
    }
    return false;
}

#ifndef CWAV_DISABLE_CSND
bool cwavEnvPlayDirectSound(CWAV* cwav, int leftChannel, int rightChannel, u32 directSoundChannel, u32 directSoundPriority, ncsndDirectSoundModifiers* soundModifiers)
{
    cwav_t* cwav_ = CWAVTOIMPL(cwav);
    ncsndDirectSound dirSound;

    ncsndInitializeDirectSound(&dirSound);

    u32 channelCount = 0;
    if (leftChannel >= 0 && leftChannel < (int)cwav_->channelcount && rightChannel >= 0 && rightChannel < (int)cwav_->channelcount)
        channelCount = 2;
    else if (leftChannel >= 0 && leftChannel < (int)cwav_->channelcount)
        channelCount = 1;
    else
        return false;
    
    u32 encoding = cwav_->cwavInfo->encoding;
    u32 encFlag = 0;
    u32 size = 0;
    if (cwav_->cwavInfo->isLooped)
        return false;
    
    u8* leftSampleData = (u8*)((u32)cwav_->channelInfos[leftChannel]->samples.offset + (u8*)(&(cwav_->cwavData->data)));
    u8* rightSampleData = 0;
    if (channelCount == 2)
        rightSampleData = (u8*)((u32)cwav_->channelInfos[rightChannel]->samples.offset + (u8*)(&(cwav_->cwavData->data)));

    switch (encoding)
    {
    case IMA_ADPCM:
        encFlag = NCSND_ENCODING_ADPCM;
        size = (cwav_->cwavInfo->LoopEnd / 2);

        memcpy(&dirSound.channelData.leftAdpcmContext, &cwav_->IMAADPCMInfos[leftChannel]->context, sizeof(ncsndADPCMContext));
        if (channelCount == 2)
            memcpy(&dirSound.channelData.rightAdpcmContext, &cwav_->IMAADPCMInfos[rightChannel]->context, sizeof(ncsndADPCMContext));
        
        break;
    case PCM8:
        encFlag = NCSND_ENCODING_PCM8;
        size = (cwav_->cwavInfo->LoopEnd);
        break;
    case PCM16:
        encFlag = NCSND_ENCODING_PCM16;
        size = (cwav_->cwavInfo->LoopEnd * 2);
        break;
    default:
        break;
    }

    soundModifiers->channelVolumes[0] = soundModifiers->channelVolumes[0] * cwav->volume;
    soundModifiers->channelVolumes[1] = soundModifiers->channelVolumes[1] * cwav->volume;

    memcpy(&dirSound.soundModifiers, soundModifiers, sizeof(ncsndDirectSoundModifiers));

    dirSound.channelData.channelAmount = channelCount;
    dirSound.channelData.channelEncoding = encFlag;
    dirSound.channelData.sampleRate = cwav_->cwavInfo->sampleRate;
    dirSound.channelData.sampleDataLength = size;
    dirSound.channelData.isLeftPhys = true;
    dirSound.channelData.isRightPhys = true;
    if (leftSampleData)
        dirSound.channelData.leftSampleData = (void*)cwavCurrentVAPAConvCallback(leftSampleData);
    if (rightSampleData)
        dirSound.channelData.rightSampleData = (void*)cwavCurrentVAPAConvCallback(rightSampleData);

    return R_SUCCEEDED(ncsndPlayDirectSound(directSoundChannel, directSoundPriority, &dirSound));
}
#endif

void cwavEnvPlay(u32 channel, bool isLooped, cwavEncoding_t encoding, u32 sampleRate, float volume, float pan, float pitch, void* block0, void* block1, u32 loopStart, u32 loopEnd, u32 totalSize, cwavIMAADPCMInfo_t* IMAADPCMInfos, cwavDSPADPCMInfo_t* DSPADPCMInfos)
{
    if (g_currentEnv == CWAV_ENV_CSND)
    {
#ifndef CWAV_DISABLE_CSND
        ncsndSound sound;
        ncsndInitializeSound(&sound);

        switch (encoding)
        {
        case PCM8:
            sound.encoding = NCSND_ENCODING_PCM8;
            break;
        case PCM16:
            sound.encoding = NCSND_ENCODING_PCM16;
            break;
        case IMA_ADPCM:
            sound.encoding = NCSND_ENCODING_ADPCM;

            sound.context.data = IMAADPCMInfos->context.data;
            sound.context.tableIndex = IMAADPCMInfos->context.tableIndex;
            sound.loopContext.data = IMAADPCMInfos->loopContext.data;
            sound.loopContext.tableIndex = IMAADPCMInfos->loopContext.tableIndex;

            break;
        default:
            break;
        }

        sound.isPhysAddr = true;
        sound.sampleData = (void*)cwavCurrentVAPAConvCallback(block0);
        if (isLooped) {
            sound.loopSampleData = (void*)cwavCurrentVAPAConvCallback(block1);
        } else {
            sound.loopSampleData = sound.sampleData;
        }
        sound.totalSizeBytes = totalSize;

        sound.loopPlayback = isLooped;
        sound.sampleRate = sampleRate;
        sound.volume = volume;
        sound.pitch = pitch;
        sound.pan = pan;

        ncsndPlaySound(channel, &sound);
#endif
    }
    else if (g_currentEnv == CWAV_ENV_DSP)
    {
#ifndef CWAV_DISABLE_DSP
        u32 encFlag = 0;

        ndspWaveBuf* block0Buff = cwavEnvGetNdspWaveBuffer(channel, 0);
        ndspWaveBuf* block1Buff = cwavEnvGetNdspWaveBuffer(channel, 1);

        switch (encoding)
        {
        case PCM8:
            encFlag = NDSP_FORMAT_PCM8;
            break;
        case PCM16:
            encFlag = NDSP_FORMAT_PCM16;
            break;
        case DSP_ADPCM:
            encFlag = NDSP_FORMAT_ADPCM;

            ndspChnSetAdpcmCoefs(channel, DSPADPCMInfos->param.coefs);

            if (isLooped)
            {
                block0Buff->adpcm_data = (ndspAdpcmData*)&DSPADPCMInfos->context;
                block1Buff->adpcm_data = (ndspAdpcmData*)&DSPADPCMInfos->loopContext;
            }
            else
            {
                block1Buff->adpcm_data = (ndspAdpcmData*)&DSPADPCMInfos->context;
            }

            break;
        default:
            break;
        }

        float mix[12] = {0};
        float rightPan = (pan + 1.f) / 2.f;
        float leftPan = 1.f - rightPan;
        mix[0] = 0.8f * leftPan * volume; // Left front
        mix[2] = 0.2f * leftPan * volume; // Left back
        mix[1] = 0.8f * rightPan * volume; // Right front
        mix[3] = 0.2f * rightPan * volume; // Right back

        ndspChnSetFormat(channel, encFlag);
        ndspChnSetRate(channel, (float)(sampleRate) * pitch);
        ndspChnSetMix(channel, mix);

        block1Buff->data_vaddr = isLooped ? block1 : block0;
        block1Buff->nsamples = loopEnd - loopStart;
        block1Buff->looping = isLooped;

        if (isLooped)
        {
            block0Buff->data_vaddr = block0;
            block0Buff->nsamples = loopStart;
            block0Buff->looping = false;
            
            ndspChnWaveBufAdd(channel, block0Buff);
        }

        ndspChnWaveBufAdd(channel, block1Buff);
#endif
    }
}

bool cwavEnvChannelIsPlaying(u32 channel) 
{
    if (g_currentEnv == CWAV_ENV_CSND)
    {
#ifndef CWAV_DISABLE_CSND
        return ncsndIsPlaying(channel);
#endif
    }
    else if (g_currentEnv == CWAV_ENV_DSP)
    {
#ifndef CWAV_DISABLE_DSP
        ndspWaveBuf* block0Buff = cwavEnvGetNdspWaveBuffer(channel, 0);
        ndspWaveBuf* block1Buff = cwavEnvGetNdspWaveBuffer(channel, 1);

        return (block0Buff->status == NDSP_WBUF_QUEUED || block0Buff->status == NDSP_WBUF_PLAYING ||
                block1Buff->status == NDSP_WBUF_QUEUED || block1Buff->status == NDSP_WBUF_PLAYING);
#endif
    }
    return false;
}

void cwavEnvStop(u32 channel)
{
    if (g_currentEnv == CWAV_ENV_CSND)
    {
#ifndef CWAV_DISABLE_CSND
        ncsndStopSound(channel);
#endif
    }
    else if (g_currentEnv == CWAV_ENV_DSP)
    {
#ifndef CWAV_DISABLE_DSP
        ndspChnReset(channel);
        ndspWaveBuf* block0Buff = cwavEnvGetNdspWaveBuffer(channel, 0);
        ndspWaveBuf* block1Buff = cwavEnvGetNdspWaveBuffer(channel, 1);
        block0Buff->status = block1Buff->status = NDSP_WBUF_FREE;
#endif
    }
}