#ifndef _AUDIO_LIB_H_
#define _AUDIO_LIB_H_
#include "gfx/vec.h"
#include <string>
bool AUDInit();
void AUDDestroy();
///Sets the size in which all sounds are going to be played
void AUDListenerSize (const float size);
void AUDListener (const Vector &pos, const Vector &vel);
///Checks if sounds are still playing
void AUDRefreshSounds ();
///Will the sound be played
void AUDListenerOrientation (const Vector & i, const Vector &j, const Vector &k);
void AUDListenerGain (const float gain);
///creates a buffer if one doesn't already exists, and then creates a source
int AUDCreateSoundWAV (const std::string &, const bool LOOP=false);
///creates a buffer for an mp3 sound if one doesn't already exist, then creates a source
int AUDCreateSoundMP3 (const std::string &, const bool LOOP=false);
///creates a buffer if one doesn't already exists, and then creates a source
int AUDCreateMusicWAV (const std::string &, const bool LOOP=false);
///creates a buffer for an mp3 sound if one doesn't already exist, then creates a source
int AUDCreateMusicMP3 (const std::string &, const bool LOOP=false);
///copies other sound loaded through AUDCreateSound
int AUDCreateSound (int sound,const bool LOOP=false);
///guesses the type of sound by extension
int AUDCreateSound (const std::string &,const bool LOOP=false);
///guesses the type of sound by extension
int AUDCreateMusic (const std::string &,const bool LOOP=false);

///deletes a given sound
void AUDDeleteSound (int sound, bool music=false);
///Changes the velocity and/or position of a given sound
void AUDAdjustSound (const int sound, const Vector &pos, const Vector &vel);
///Changes the gain of a loaded sound
void AUDSoundGain (const int sound, const float gain);
///Is a loaded sound still playing
bool AUDIsPlaying (const int sound);
///Stops a loaded sound
void AUDStopPlaying (const int sound);
///Plays a loaded sound
void AUDStartPlaying (const int sound);
///Queries if the sound should be culled. If not, plays
void AUDPlay (const int sound, const Vector & pos, const Vector & vel, const float gain);
///Pauses a loaded sound
void AUDPausePlaying (const int sound);
//#define PERFRAMESOUND
#endif
