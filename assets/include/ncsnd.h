/**
 * @file ncsnd.h
 * @brief CSND interface for the Nintendo 3DS.
 */
#pragma once

#include <3ds/types.h>

#ifdef __cplusplus
extern "C" {
#endif

/// CSND handle.
extern Handle ncsndCSNDHandle;

/// Maximum number of CSND channels.
#define NCSND_NUM_CHANNELS 32

/// Maximum volume for a direct sound.
#define NCSND_DIRECTSOUND_MAX_VOLUME 32768

/// Output modes for a direct sound.
typedef enum
{
	NCSND_SOUNDOUTPUT_MONO = 0, ///< Mono output
	NCSND_SOUNDOUTPUT_STEREO = 1 ///< Stereo output
} ncsndSoundOutputMode;

/// Possible sample encodings.
typedef enum
{
	NCSND_ENCODING_PCM8 = 0, ///< PCM8
	NCSND_ENCODING_PCM16,    ///< PCM16
	NCSND_ENCODING_ADPCM,    ///< IMA-ADPCM
} ncsndSoundFormat;

/// ADPCM context.
typedef struct
{
    u16 data;	///< Sample
    u8 tableIndex; ///< table + index
    u8 padding;
} ncsndADPCMContext;

/// Channel related data for a direct sound.
typedef struct 
{
	u8 channelAmount; ///< Amount of channels used, either 1 or 2.
	u8 channelEncoding; ///< Sample encoding.
	bool isLeftPhys; ///< If leftSampleData is a physical address (ignore if unknown).
	bool isRightPhys; ///< If rightSampleData is a physical address (ignore if unknown).
	u32 sampleRate; ///< Sample rate for both channels.
	void* leftSampleData; ///< Pointer to the left ear or mono sample data.
	void* rightSampleData; ///< Pointer to the right ear or mono sample data.
	u32 sampleDataLength; ///< Size of an individual sample data buffer in bytes.
	ncsndADPCMContext leftAdpcmContext; ///< IMA ADPCM context for left ear or mono channel.
	ncsndADPCMContext rightAdpcmContext; ///< IMA ADPCM context for right ear channel.
} ncsndDirectSoundChannelData;

/// Modifiers applied to a direct sound.
typedef struct
{
	float speedMultiplier; ///< Sound playback speed, default: 1.
	s32 channelVolumes[2]; ///< Volume of each individual channel, max: 32768.
	u8 unknown0; ///< Unknown
	u8 padding[3]; ///< Padding? Never used.
	float unknown1; ///< Unknown, seems to be set to 1. Some sort of play delay?
	u32 unknown2; ///< Unknown, maybe related to unknown1. Some sort of play delay?
	u8 ignoreVolumeSlider; ///< Set to 1 to play at maximum volume, ignoring the volume slider.
	u8 forceSpeakerOutput; ///< Set to 1 to play on the speakers, even if headphones are connected.
	u8 playOnSleep; ///< Set to 0 to pause the sound on sleep and 1 to continue playing on sleep. 
	u8 padding1; ///< Padding? Never used.
} ncsndDirectSoundModifiers;

/// Direct sound struct.
typedef struct
{
	u8 always0; ///< Always set to 0 by applets.
	u8 soundOutputMode; ///< Output mode (ncsndSoundOutputMode).
	u8 padding[2]; ///< Padding? Never used.
	ncsndDirectSoundChannelData channelData; ///< Channel related data.
	ncsndDirectSoundModifiers soundModifiers; ///< Modifiers applied to sound playback.
} ncsndDirectSound;

/// Sound struct.
typedef struct
{
	bool isPhysAddr; ///< Whether sampleData and loopSampleData hold physical addresses or not (ignore if unknown).
	void* sampleData; ///< Pointer to sample data.
	void* loopSampleData; ///< Pointer to the loop point sample (set to sampleData if loop unused).
	u32 totalSizeBytes; ///< Total size in bytes from start to end.

	ncsndSoundFormat encoding; ///< Sample data encoding.
	bool loopPlayback; ///< Whether to loop the playback or not (uses loopSampleData as the loop point).
	ncsndADPCMContext context; ///< ADPCM context.
	ncsndADPCMContext loopContext; ///< ADPCM context at the loop point (set to context if loop unused).

	u32 sampleRate; ///< Sample rate of the sound.
	float volume; ///< Volume of the sound.
	float pitch; ///< Pitch of the sound.
	float pan; ///< Panning of the sound.
	bool linearInterpolation; ///< Whether to enable or disable linear interpolation.

} ncsndSound;

///< Bitmask of channels that are allowed for usage.
extern u32 ncsndChannels;

/**
 * @brief Initializes the ncsnd interface, including CSND.
 * @param installAptHook Whether to handle recieving APT notifications automatically.
 * @return Result of the operation.
 * 
 * APT notifications only work properly in normal applications. Use manual handling for services, applets or plugins.
*/
Result ncsndInit(bool installAptHook);

/**
 * @brief Notifies an APT event, only needed if false was passed to ncsndInit
 * @param event APT event to notify, check the APT_HookType enum.
 * 
 * APT notification manual handling is needed for services, applets or plugins.
*/
void ncsndNotifyAptEvent(APT_HookType event);

/**
 * @brief Terminates the ncsnd interface, including CSND.
*/
void ncsndExit(void);

/**
 * @brief Initializes a direct sound to the default values.
 * @param sound The direct sound to initialize.
*/
void ncsndInitializeDirectSound(ncsndDirectSound* sound);

/**
 * @brief Plays a direct sound.
 * @param chn Direct sound channel to use. Range [0, 3].
 * @param priority Direct sound priority, used if the channel is already playing. Smaller value -> Higher priority. Range [0, 31].
 * @param sound Pointer to a direct sound struct to play.
 * @return Result of the operation.
*/
Result ncsndPlayDirectSound(u32 chn, u32 priority, ncsndDirectSound* sound);

/**
 * @brief Initializes a sound to the default values.
 * @param sound The sound to initialize.
*/
void ncsndInitializeSound(ncsndSound* sound);

/**
 * @brief Plays a sound.
 * @param chn Sound channel to use. Make sure to check availability with ncsndChannels.
 * @param sound Pointer to a sound struct to play.
 * @return Result of the operation.
*/
Result ncsndPlaySound(u32 chn, ncsndSound* sound);

/**
 * @brief Stops a channel.
 * @param chn Sound channel to stop.
*/
void ncsndStopSound(u32 chn);

/**
 * @brief Sets the volume for a channel.
 * @param chn Sound channel to change.
 * @param volume Volume value. Range [0, 1].
 * @param pan Panning value. Range [-1, 1].
*/
void ncsndSetVolume(u32 chn, float volume, float pan);

/**
 * @brief Sets the rate for a channel.
 * @param chn Sound channel to change.
 * @param volume Sample rate value.
 * @param pan Pitch value. Range [0, 1].
*/
void ncsndSetRate(u32 chn, u32 sampleRate, float pitch);

/**
 * @brief Check playing state of a channel.
 * @param chn Sound channel to check.
*/
bool ncsndIsPlaying(u32 chn);

#ifdef __cplusplus
}
#endif