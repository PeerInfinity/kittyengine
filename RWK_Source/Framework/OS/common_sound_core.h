#pragma once
//
// The Sound_Core interface, shared by every platform.
//
// Framework/OS/<Platform>/sound_core.h is a one-line include of this, and
// Framework/OS/<Platform>/sound_core.cpp a one-line include of
// common_sound_core.cpp.  Before this file there were three headers: Linux's
// and Windows' were identical at 77 lines, WASM's was a 69-line near-copy that
// had drifted (no Multitasking, no AllowSoundsInBackground, no PauseMusic, and
// an AdPause of its own).  This is their union.
//
#include "os_headers.h"


namespace Sound_Core
{
	//
	// Startup/Shutdown sound system
	//
	void				Startup();
	void				Shutdown();

	//
	// Set global volume
	//
	void				SetGlobalMusicVolume(float theVolume);
	void				SetGlobalSoundVolume(float theVolume);

	//
	// Pauses sound (put in for multitasking)
	//
	void				Pause(bool theState);
	void				AdPause(bool theState);		// special pause for ads (WASM)

	//
	// Handles multitasking cleanups...
	//
	void				Multitasking(bool isForeground);
	void				AllowSoundsInBackground(bool theState=true);

	//
	// Load/Play Sounds
	//
	unsigned int		LoadSound(char *theFilename, int theDuplicates);
	unsigned int		GetSoundBuffer(unsigned int theSoundID);
	void				PlaySoundBuffer(unsigned int theBufferID);
	void				StopSoundBuffer(unsigned int theBufferID);
	void				SetSoundBufferVolume(unsigned int theBufferID, float theVolume);
	void				SetSoundBufferFrequency(unsigned int theBufferID, float theFrequency);
	void				SetSoundBufferLooping(unsigned int theBufferID, bool theState);
	float				GetSoundBufferFrequency(unsigned int theBufferID);
	bool				IsSoundBufferPlaying(unsigned int theBufferID);
	void				UnloadSound(unsigned int theSoundID);

	//
	// Load/Unload Streams
	//
	unsigned int		LoadStreamSound(char *theFilename);
	void				UnloadStreamSound(unsigned int theSoundID);
	void				GetStreamLevel(unsigned int theSoundID, float* theLeft, float* theRight);

	//
	// Load/Play Music
	//
	// TRACKER-MODULE MUSIC IS NOT SUPPORTED.  These are logging stubs; RWK
	// reaches none of them (every asset it ships is .ogg, and its music plays
	// through SoundStream).  Kept so rapt_audio.cpp still links unchanged.
	//
	unsigned int		LoadMusic(char *theFilename);
	void				UnloadMusic(unsigned int theHandle);
	void				PlayMusic(unsigned int theHandle, unsigned int theOffset);
	void				StopMusic(unsigned int theHandle);
	void				SetMusicVolume(unsigned int theHandle, float theVolume);
	void				SetMusicTrackVolume(unsigned int theHandle, unsigned int theTrack, float theVolume);
	void				PauseMusic(int theHandle, bool thePause);

	//
	// Queue sounds to position
	//
	void				SetSoundPosition(unsigned int theHandle, float thePos);
	float				GetSoundPosition(unsigned int theHandle);

	//
	// Advance stuff... put this in while trying to port BeepBox...
	//
	void				GetSampleRate(unsigned int& theMin, unsigned int& theMax);
	unsigned int		GetFrequency();
	unsigned int		CreateDynamicSound(void* theDataPacket, int theBufferLength);
}
