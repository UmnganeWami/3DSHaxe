#ifndef CWAVENV_H
#define CWAVENV_H
#include "cwav.h"
#include "internal/cwav_defs.h"

void cwavEnvUseEnvironment(cwavEnvMode_t envMode);
cwavEnvMode_t cwavEnvGetEnvironment();

void cwavEnvInitialize();
void cwavEnvFinalize();

bool cwavEnvCompatibleEncoding(cwavEncoding_t encoding);

u32 cwavEnvGetChannelAmount();
bool cwavEnvIsChannelAvailable(u32 channel);

#ifndef CWAV_DISABLE_CSND
bool cwavEnvPlayDirectSound(CWAV* cwav, int leftChannel, int rightChannel, u32 directSoundChannel, u32 directSoundPriority, ncsndDirectSoundModifiers* soundModifiers);
#endif

void cwavEnvPlay(u32 channel, bool isLooped, cwavEncoding_t encoding, u32 sampleRate, float volume, float pan, float pitch, void* block0, void* block1, u32 loopStart, u32 loopEnd, u32 totalSize, cwavIMAADPCMInfo_t* IMAADPCMInfos, cwavDSPADPCMInfo_t* DSPADPCMInfos);
bool cwavEnvChannelIsPlaying(u32 channel);
void cwavEnvStop(u32 channel);

#endif