/**
 * @file cwav.h
 * @brief libcwav - Library to play (b)cwav files on the 3DS.
*/
#pragma once
#include "3ds.h"

#ifndef CWAV_DISABLE_CSND
#include "ncsnd.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/// Posible status values.
typedef enum
{
    // General status values.
    CWAV_NOT_ALLOCATED = 0, ///< CWAV is not allocated and cannot be used.
    CWAV_SUCCESS = 1, ///< Operation succeded.
    CWAV_INVALID_ARGUMENT = 2, ///< An invalid argument was passed to the function call.

    // Load status values.
    CWAV_FILE_OPEN_FAILED = 3, ///< Failed to open the specified file.
    CWAV_FILE_READ_FAILED = 4, ///< The file failed to be read into memory.
    CWAV_UNKNOWN_FILE_FORMAT = 5, ///< The specified file is not a valid CWAV file.
    CWAV_INVAID_INFO_BLOCK = 6, ///< The INFO block in the CWAV file is invalid or not supported.
    CWAV_INVAID_DATA_BLOCK = 7, ///< The DATA block in the CWAV file is invalid or not supported.
    CWAV_UNSUPPORTED_AUDIO_ENCODING = 8, ///< The audio encoding is not supported.

    // Play status values.
    CWAV_INVALID_CWAV_CHANNEL = 9, ///< The specified channel is not in the CWAV.
    CWAV_NO_CHANNEL_AVAILABLE = 10, ///< No DSP/CSND channels available to play the sound.

} cwavStatus_t;

/// Possible environments.
typedef enum
{
    CWAV_ENV_DSP = 0, // DSP Service, only available for applications.
    CWAV_ENV_CSND = 1 // CSND Service, only available for applets and 3GX plugins.
} cwavEnvMode_t;

/// Information returned by cwavPlay.
typedef struct cwavPlayResult_s
{
    cwavStatus_t    playStatus;         ///< Value from the cwavStatus_t enum.
    u8              monoLeftChannel;    ///< Mono or left ear DSP/CSND channel the sound is playing on.
    u8              rightChannel;       ///< Right ear DSP/CSND channel the sound is playing on.
} cwavPlayResult;

/// CWAV structure, some values can be read [R] or written [W] to.
typedef struct CWAV_s
{
    void*           cwav;           ///< Pointer to internal cwav data, should not be used.
    void*           dataBuffer;     ///< [RW] Pointer to the buffer where the CWAV was loaded (used by cwavFileLoad and cwavFileFree). Otherwise, can be used by the user to store the allocation address (ignored by the library).
    cwavStatus_t    loadStatus;     ///< [R] Value from the cwavStatus_t enum. Set when the CWAV is loaded. 
    float           monoPan;        ///< [RW] Value in the range [-1.0, 1.0]. -1.0 for left ear and 1.0 for right ear. Only used if played in mono. Default: 0.0
    float           volume;         ///< [RW] Value in the range [0.0, 1.0]. 0.0 muted and 1.0 full volume. Default: 1.0
    float           pitch;          ///< [RW] Changes the playback speed. Default: 1.0 (no pitch change)
    u32             sampleRate;     ///< [R] The sample rate of the audio data.
    u8              numChannels;    ///< [R] Number of CWAV channels stored in the file.
    u8              isLooped;       ///< [R] Whether the file is looped or not.
} CWAV;

/// vAddr to pAddr conversion callback definition.
typedef u32(*vaToPaCallback_t)(const void*);

/**
 * @brief Sets the environment that libcwav will use.
 * 
 * This call does not initialize the desired service. ndspInit/csndInit should be called before.
 * By default, DSP is used. Modifying the enviroment after loading the first CWAV causes undefined behaviour.
 * Remember to call cwavDoAptHook or use cwavNotifyAptEvent if CSND has to be used.
*/
void cwavUseEnvironment(cwavEnvMode_t envMode);

/**
 * @brief Sets a custom virtual address to physical address conversion callback. (Only used for CSND)
 * @param callback Function callback to use.
 * 
 * The function callback must take the virtual address as a const void* argument and return the physical address as a u32.
 * If not set, the default conversion is used. Use NULL to reset to default.
*/
void cwavSetVAToPACallback(vaToPaCallback_t callback);

/**
 * @brief Loads a CWAV from a buffer in linear memory.
 * @param bcwavFileBuffer Pointer to the buffer in linear memory (e.g.: linearAlloc()) containing the CWAV file.
 * @param maxSPlays Amount of times this CWAV can be played simultaneously (should be >0).
 * @return A pointer to a CWAV struct.
 * 
 * Use the loadStatus struct member to determine if the load was successful.
 * Wether the load was successful or not, cwavFree must be always called to clean up and free the memory.
 * The bcwavFileBuffer must be manually freed by the user after calling cwavFree (e.g.: linearFree()).
 */
void cwavLoad(CWAV* out, const void* bcwavFileBuffer, u8 maxSPlays);

/**
 * @brief Loads a CWAV from the file system.
 * @param bcwavFileName Path to the (b)CWAV file in the filesystem.
 * @param maxSPlays Amount of times this CWAV can be played simultaneously (should be >0).
 * 
 * Use the loadStatus struct member to determine if the load was successful.
 * Wether the load was successful or not, cwavFileFree must be always called to clean up and free the memory.
 * Do not use cwavFree, as it will not properly free the bcwav buffer.
 * 
 * This function does not work with 3GX plugins.
 */
void cwavFileLoad(CWAV* out, const char* bcwavFileName, u8 maxSPlays);

/**
 * @brief Frees the CWAV.
 * @param cwav The CWAV to free.
 * 
 * Must be called even if the CWAV load fails.
 * The bcwavFileBuffer used in cwavLoad must be manually freed afterwards (e.g.: linearFree()).
 * The CWAV* struct itself must be freed manually if it has been allocated.
*/
void cwavFree(CWAV* cwav);

/**
 * @brief Frees a CWAV loaded from the filesystem.
 * @param cwav The CWAV to free.
 * 
 * Must be called even if the CWAV file load fails.
 * Use this if you have used cwavFileLoad, otherwise use cwavFree.
 * The CWAV* struct itself must be freed manually if it has been allocated.
*/
void cwavFileFree(CWAV* cwav);

#ifndef CWAV_DISABLE_CSND
/**
 * @brief Plays the CWAV channels as a direct sound (only available if using CSND).
 * @param cwav The CWAV to play.
 * @param leftChannel The CWAV channel to play on the left ear.
 * @param rigtChannel The CWAV channel to play on the right ear.
 * @param directSoundChannel The direct sound channel to play the sound on. Range [0-3].
 * @param directSoundPrioriy The direct sound priority to use if the specified channel is in use (smaller value -> higher priority). Range [0-31].
 * @param soundModifiers CSND direct sound modifiers to apply to the sound.
 * 
 * To play a single channel in mono for both ears, set rightChannel to -1.
 * The individual channel volumes are multiplied by the CWAV volume.
*/
bool cwavPlayAsDirectSound(CWAV* cwav, int leftChannel, int rightChannel, u32 directSoundChannel, u32 directSoundPriority, ncsndDirectSoundModifiers* soundModifiers);
#endif

/**
 * @brief Plays the specified channels in the bcwav file.
 * @param cwav The CWAV to play.
 * @param leftChannel The CWAV channel to play on the left ear.
 * @param rigtChannel The CWAV channel to play on the right ear.
 * @return A cwavPlayResult struct with the status code and which audio channels were assigned.
 * 
 * To play a single channel in mono for both ears, set rightChannel to -1.
*/
cwavPlayResult cwavPlay(CWAV* cwav, int leftChannel, int rightChannel);

/**
 * @brief Stops the specified channels in the bcwav file.
 * @param cwav The CWAV to play.
 * @param leftChannel The CWAV channel to stop (left ear).
 * @param rigtChannel The CWAV channel to stop (right ear).
 * 
 * Setting both channels to -1 stops all the CWAV channels.
*/
void cwavStop(CWAV* cwav, int leftChannel, int rightChannel);

/**
 * @brief Checks whether the cwav is currently playing or not.
 * @return Boolean representing the playing state.
*/
bool cwavIsPlaying(CWAV* cwav);

/**
 * @brief Gets a bitmap representing the playing state of the channels for the selected environment.
 * @return Bitmap of the playing channels. First channel is the LSB.
*/
u32 cwavGetEnvironmentPlayingChannels();

#ifdef __cplusplus
}
#endif