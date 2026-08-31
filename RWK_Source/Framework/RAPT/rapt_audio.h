#pragma once
#include "rapt_headers.h"
#include "sound_core.h"

#define SUPPORT_MUSIC_SAMPLES

//
// Allows you to delay the playing of a sound for a period.
// Call it right before playing the sound.  It's invalidated when you call a sound play.
// So like:
// DelaySound(10);
// gSounds->mSound.Play(); <- is delayed
// gSounds->mSound.Stop(); <- will stop delayed ones too
//
void DelaySound(int theWait);
inline void DelaySound(float theWait) {DelaySound((int)(theWait+.5f));}

enum
{
	ID_SOUNDECHO=100000,
	ID_SOUNDDELAYED,
};

class SoundStream;
class Audio : public CPU
{
public:
	Audio();
	virtual ~Audio();

	void				Go();
	void				Stop();

	//
	// Save/load settings from the settings file...
	//
	void				Load();
	void				Save();

	//
	// Handles global volume
	//
	float				mMusicVolume;
	float				mSoundVolume;
	void				SetMusicVolume(float theVolume);
	void				SetSoundVolume(float theVolume);
	inline float		GetMusicVolume() {return mMusicVolume;}
	inline float		GetSoundVolume() {return mSoundVolume;}

	//
	// Clipping volumes, helpful for when you need to reduce volume of things...
	//
	float				mClipMusicVolume;
	float				mClipSoundVolume;
	inline void			ClipSoundVolume(float theVolume=1.0f) {mClipSoundVolume=theVolume;SetSoundVolume(mSoundVolume);}
	inline void			ClipMusicVolume(float theVolume=1.0f) {mClipMusicVolume=theVolume;SetMusicVolume(mMusicVolume);}

	//
	// Update processes any echos, crossfades, etc...
	//
	void				Update();

	//
	// Pauses things if we get minimized...
	//
	void				Multitasking(bool isForeground);
	void				Pause(bool doPause=true);
	int					mPauseCount;

	//
	// Lets us cut off our audio if we need to
	// (like if we have a long sound that needs to be clobbered)
	//
	void				StopAllSound();
	void				StopAllMusic();

	//
	// List of all sounds and music we have loaded,
	// so we can stop them all if we want to.
	//
	List				mSoundList;
	List				mSoundSteamList;
	List				mMusicList;

	//
	// Object Manager... this handles any music or sounds that need to be processed
	// beyond a simple invocation...
	//
	ObjectManager		mUpdateSoundList;

	//
	// Echo attributes... echoes will be global, since we don't want to have to
	// set echo info EACH TIME we play a sound!
	//
	inline void			SetEchoData(float theDelayBetweenEchoes, int theEchoCount, float theVolumeMultiplier)
	{
		mEchoes.mDelay=theDelayBetweenEchoes;
		mEchoes.mVolumeMultiplier=theVolumeMultiplier;
		mEchoes.mCount=theEchoCount;

	}
	inline void			ForceEchoes(bool theState=true) {mEchoes.mForceEchoes=theState;}
	struct 
	{
		bool				mForceEchoes;
		float				mDelay;
		float				mVolumeMultiplier;
		int					mCount;	
	} mEchoes;

	Vector				m3DSoundCenter;
	float				m3DSoundRadius[2];
	float				m3DSoundRadiusSize;
	inline void Set3DSoundCenter(Vector theCenter, float theInnerRadius, float theOuterRadius)
	{
		m3DSoundCenter=theCenter;
		m3DSoundRadius[0]=theInnerRadius*theInnerRadius;
		m3DSoundRadius[1]=theOuterRadius*theOuterRadius;
		m3DSoundRadiusSize=m3DSoundRadius[1]-m3DSoundRadius[0];
	}
	float Get3DVolume(Vector thePos, float theVolume)
	{
		float aMod=1.0f;
		float aDist=gMath.DistanceSquared(thePos,m3DSoundCenter);

		if (aDist>=m3DSoundRadius[1]) aMod=0;
		else if (aDist>m3DSoundRadius[0]) aMod=1.0f-((aDist-m3DSoundRadius[0])/m3DSoundRadiusSize);
		return theVolume*aMod;
	}
	float Get2DVolume(Vector thePos, float theVolume)
	{
		float aMod=1.0f;
		float aDist=gMath.DistanceSquaredXY(thePos,m3DSoundCenter);

		if (aDist>=m3DSoundRadius[1]) aMod=0;
		else if (aDist>m3DSoundRadius[0]) aMod=1.0f-((aDist-m3DSoundRadius[0])/m3DSoundRadiusSize);
		return theVolume*aMod;
	}

	//
	// This allows us to use sound+music all as sound files (I.E. loading an MP3 or WAV as music...)
	//
#ifdef SUPPORT_MUSIC_SAMPLES
	void				SetMusicSample(SoundStream* theSound);
	List				mMusicSampleList;
#endif

	float				GetMinimumSampleRate();
	float				GetMaximumSampleRate();
	float				GetFrequency();

	//
	// Sometimes we want to mute all sound-- for instance, if we're doing some background processing before saving or whatnot.
	// This will prevent any "Play" functions from happening.
	//
	void				MuteSounds(bool theState=true) {mMuteSounds=theState;} 
	bool				mMuteSounds;

};

void Set3DSoundCenter(Vector theCenter, float theInnerRadius, float theOuterRadius);

class Sound;
class SoundInstance
{
public:
	SoundInstance(unsigned int theHandle);

	void				Play();
	void				SetLooping(bool theState);
	void				SetVolume(float theVolume);
	void				SetClipVolume(float theVolume);
	void				SetPitch(float thePitch);
	bool				IsPlaying();
	void				Stop();
	void				SetPosition(float theSeconds);
	float				GetPosition();
	float				GetLevel();

public:
	unsigned int		mBuffer;
	float				mFrequency;
	float				mVolume;
	float				mClipVolume;
};

class Sound
{
public:
	Sound();
	virtual ~Sound();

	void				Load(String theFilename, int theDuplicates=10);
	bool				IsLoaded() {return mHandle>=0;}

	//
	// Handle to the number of this sound, in hardware.
	//
	unsigned int		mHandle=-1;
	List				mBufferList;
	int					mDuplicates=1;


	//
	// Playback!
	// Play plays with all the features of gAudio... for instance, if you have
	// forced echoing on, play will actually go to echo.
	//
	void				Play(float theVolume=1.0f);
	void				Play2D(Vector thePos, float theVolume=1.0f);
	void				Play3D(Vector thePos, float theVolume=1.0f);
	void				PlayPitched(float thePitch, float theVolume=1.0f);
	void				PlayPitched2D(Vector thePos, float thePitch, float theVolume=1.0f);
	void				PlayPitched3D(Vector thePos, float thePitch, float theVolume=1.0f);

	//
	// Echo plays with echo, always
	//
	void				PlayEchoed(float theVolume=1.0f);
	void				PlayPitchedEchoed(float thePitch, float theVolume=1.0f);
	//
	// Playnormal always plays normally
	//
	void				PlayNormal(float theVolume=1.0f);
	void				PlayPitchedNormal(float thePitch, float theVolume=1.0f);
	
	//
	// A volume specifically for this sound.  All volume calls will get 
	// multiplied by this.  I put this in so I could lower the volume of 
	// certain sounds when Solomon is talking.
	//
	void				SetVolume(float theVolume);
	float				mVolume=1.0f;

	//
	// A special global volume.  The purpose of this is to allow you to 
	// group sounds together and control their volume as a sort of instance.
	// For Solomon Dark, I used this to lower sound while a character was
	// speaking.
	//
	void				SetClipVolume(float theVolume);
	float				mClipVolume=1.0f;

	//
	// Stop all buffers
	//
	void				Stop();

	//
	// To get a single SoundInstance...
	// (Stealing removes it from the sound completely, and it must
	// be deleted elsewhere)
	//
	SoundInstance*		GetSoundInstance();
	SoundInstance*		StealSoundInstance();
};


class SoundLoop : public Object
{
public:
	SoundLoop();
	virtual ~SoundLoop();

	void				Load(String theFilename);
	bool				IsLoaded() {return (mInstance!=NULL);}
	void				Update();

	unsigned int		mHandle=-1;
	Sound				mSound;
	SoundInstance*		mInstance;
	float				mVolume;
	float				mPitch=1.0f;

	inline void			SetVolume(float theVolume) {mVolume=theVolume;if (mInstance) mInstance->SetVolume(theVolume*mFadeVolume);}
	inline void			SetPitch(float thePitch) {mPitch=thePitch;if (mInstance) mInstance->SetPitch(thePitch);}
	inline bool			IsPlaying() {if (mInstance) return mInstance->IsPlaying();return false;}
	inline void			StopNow() {mCounter=0;Stop();}
	inline void			StopAll() {StopNow();}

	//
	// Call Go and Stop to make looping start or stop.
	// Go will increase counter, and stop will decrease.
	// This controls play so you can call go and stop
	// repeatedly.
	//
	void				Go();
	void				Stop();
	int					mCounter;

	//
	// If you want the sound to fade out or in when it gets increased or
	// decreased, you can set this.
	//
	float				mFadeInSpeed;
	float				mFadeOutSpeed;
	void				FadeIn(float theFadeInSpeed=0.0f);
	void				FadeOut(float theFadeOutSpeed=0.0f);
	//
	// We do some processing on the soundloop, for instance if we're
	// fading in or out.  These are the variables that help control that...
	//
	int					mUpdateType;
	float				mFadeVolume;
};

class SoundLoopGroup // A group of soundloops... allows you to specify a handle (if the handle is out of range, it will find one for you).
					 // Put in for Hamsterball, where I expected to have lots of loops for the wobbly platforms, but wanted to just grab whichever
					 // one might be available.
{
public:
	SoundLoopGroup();
	virtual ~SoundLoopGroup();
	void				Load(String theFilename, int theCount=5);
	void				Load(Sound& theLoop);

	bool				IsLoaded() {return (mLoops.Size()>0);}

	Sound				mSound;
	Array<SoundLoop>	mLoops;
	SoundLoop			mNullLoop;

	SoundLoop&			GetLoop(int& theHandle) 
	{
		if (theHandle==-1)	
		{
			for (int aCount=0;aCount<mLoops.Size();aCount++) if (!mLoops[aCount].IsPlaying()) {theHandle=aCount;return mLoops[theHandle];}
			theHandle=-1;return mNullLoop;
		}
		return mLoops[theHandle];
	}

	// Returns true if other operations are reasonable.  If you're requesting volume 0, it'll return false, since nothing else matters.
	// This way you can say if(SetVolume(...)) SetPitch() or whatever.
	bool				SetVolume(int& theHandle, float theVolume)
	{
		if (theVolume>0)
		{
			SoundLoop& aLoop=GetLoop(theHandle);
			aLoop.SetVolume(theVolume);
			if (!aLoop.IsPlaying()) aLoop.Go();
			return true;
		}
		else if (theHandle!=-1)
		{
			SoundLoop& aLoop=GetLoop(theHandle);
			aLoop.SetVolume(theVolume);
			if (aLoop.IsPlaying()) aLoop.Stop();
			theHandle=-1;
		}
		return false;
	}

	// Sets volume and pitch together
	bool				SetVolume(int& theHandle, float theVolume, float thePitch)
	{
		if (theVolume>0)
		{
			SoundLoop& aLoop=GetLoop(theHandle);
			aLoop.SetVolume(theVolume);
			aLoop.SetPitch(thePitch);
			if (!aLoop.IsPlaying()) aLoop.Go();
			return true;
		}
		else if (theHandle!=-1)
		{
			SoundLoop& aLoop=GetLoop(theHandle);
			aLoop.SetVolume(theVolume);
			if (aLoop.IsPlaying()) aLoop.Stop();
			aLoop.SetPitch(1.0f);
			theHandle=-1;
		}
		return false;
	}

	inline void			SetPitch(int& theHandle, float thePitch) {GetLoop(theHandle).SetPitch(thePitch);}
	inline bool			IsPlaying(int& theHandle) {return GetLoop(theHandle).IsPlaying();}
	inline void			StopNow(int& theHandle) {GetLoop(theHandle).StopNow();}
	inline void			StopAll() {foreach(aL,mLoops) aL.StopNow();}

	inline void			Go(int& theHandle) {GetLoop(theHandle).Go();}
	inline void			Stop(int& theHandle) {GetLoop(theHandle).Stop();}

	inline void			FadeIn(int& theHandle, float theFadeInSpeed=0.0f) {GetLoop(theHandle).FadeIn(theFadeInSpeed);}
	inline void			FadeOut(int& theHandle, float theFadeOutSpeed=0.0f) {GetLoop(theHandle).FadeOut(theFadeOutSpeed);}


};

class SoundEcho : public Object
{
public:
	SoundEcho(Sound *theSound, float thePitch, float theDelay, float theStartVolume, int theEchoCount, float theVolumeMult);

	void				Update();

public:
	Sound				*mSound;

	int					mEchoCount;
	float				mPitch;
	float				mVolume;
	float				mDelay;
	float				mDelayCounter;
	float				mVolumeMult;
};

class SoundDelayed : public Object
{
public:
	SoundDelayed(Sound* theSound, float thePitch, float theVolume, int theDelay);
	void				Update();
	virtual void		PlayIt();

public:
	Sound*				mSound;
	float				mPitch;
	float				mVolume;
	int					mDelay;


};

class SoundDelayed2D : public SoundDelayed
{
public:
	SoundDelayed2D(Vector thePos, Sound* theSound, float thePitch, float theVolume, int theDelay) : SoundDelayed(theSound,thePitch,theVolume,theDelay) {mPos=thePos;}
	void PlayIt();
public:
	Vector				mPos;
};

class SoundDelayed3D : public SoundDelayed
{
public:
	SoundDelayed3D(Vector thePos, Sound* theSound, float thePitch, float theVolume, int theDelay) : SoundDelayed(theSound,thePitch,theVolume,theDelay) {mPos=thePos;}
	void PlayIt();
public:
	Vector				mPos;
};

class Music : public Object
{
public:
	Music();
	virtual ~Music();

	void				Load(String theFilename);
	bool				IsLoaded() {return mLoaded;}

	//
	// Play song (or song + tracks)
	//
	void				Play(String theSong, bool clearTrackInfo=true);
	void				Play(String theSong, String theTrack);
	void				Pause(bool theState);
	void				Stop();

	//
	// Crossfade to another song
	//
	inline void			FadeOut(float theUpdatesToFadeOut=-1) {CrossFade("",theUpdatesToFadeOut);}
	inline void			FadeIn(String theSong, float theUpdatesToFadeOut=-1) {CrossFade(theSong,theUpdatesToFadeOut);}
	inline void			FadeIn(String theSong, String theTrack, float theUpdatesToFadeOut=-1) {CrossFade(theSong,theTrack,theUpdatesToFadeOut);}
	void				CrossFade(String theSong, float theUpdatesToCrossFade=-1);
	void				CrossFade(String theSong, String theTrack, float theUpdatesToCrossFade=-1);

	//
	// Set the music volume (use if you need to lower the
	// volume temporarily or something, but don't want the
	// volume change saved or anything)
	//
	void				SetVolume(float theVolume);
	float				mVolume;
	
	//
	// Set the volume of various tracks... unless you're doing something fancy,
	// these get handled by play functions and you can ignore them...
	//
	void				SetTrackVolume(String theSong, String theTrack, float theVolume);
	void				SetAllTrackVolume(String theSong, float theVolume);
	void				SetAllOtherTrackVolume(String theSong, String theTrack, float theVolume);

	void				SetTrackWantVolume(String theSong, String theTrack, float theVolume);
	void				SetAllTrackWantVolume(String theSong, float theVolume);
	void				SetAllOtherTrackWantVolume(String theSong, String theTrack, float theVolume);

	//
	// Update function for crossfading...
	//
	void				Update();

public:
	static const int	mMaxHandles=2;
	unsigned int		mHandle[mMaxHandles];
	int					mCurrentHandle;
	bool				mLoaded;

	struct Track
	{
		String			mName;
		Array<int>		mChannel;
	};


	struct Song
	{
		~Song() {_FreeList(Track,mTrackList);}

		String			mName;
		int				mOffset;

		List			mTrackList;

		//
		// All the channels in all the tracks...
		// This allows us to smoothly crossfade them...
		//
		int				mHighestChannel;
		Array<float>	mChannelVolume;
		Array<float>	mChannelWantVolume;
	};

	int					mMaxChannels;	// Maximum number of tracks ever referenced (we need this to set volume on songs that don't have a track list)

	List				mSongList;
	String				mCurrentSong;
	Song*				mCurrentSongPtr;
	inline String		GetSongName(int theSlot) {Song* aS=(Song*)mSongList[theSlot];if (!aS) return "";return aS->mName;}

	int					mFadeOutHandle;
	float				mFadeInVolume;
	float				mFadeOutVolume;
	float				mFadeVolumeStep;

	//
	// Type of crossfade...
	//
	bool				mCrossfadeSong;
	bool				mCrossfadeTrack;

	//
	// For Pausing/resuming
	//
	bool				mPaused;
	String				mPausedSong;


};

/*
class Loader;
class SoundBundle
{
public:
	virtual void			Load(Loader *theLoader) {}
};
*/

class SoundStream
{
public:
	SoundStream();
	virtual ~SoundStream();

	virtual void			Load(String theFilename);
	bool					IsLoaded() {return mHandle>=0;}


	void					SetLooping(bool theState);
	virtual void			Play(float theVolume=1.0f);
	virtual void			SetVolume(float theVolume);


	virtual void			Stop();
	virtual bool			IsPlaying();

	virtual float			GetLevel();
	virtual void			SetPosition(float theSeconds);
	virtual float			GetPosition();

	int						mHandle=-1;

	void					FadeIn(float theTime);
	void					FadeOut(float theTime);
};

class SoundStreamDynamic : public SoundStream
{
public:
	void					Load(void (*theCallback)(short* theBuffer, unsigned int theLength, void* theExtraData), void* theExtraData=NULL, int theBufferLength=200);

	struct 
	{
		void (*mCallback)(short* theBuffer, unsigned int theLength, void* theExtraData);
		void *mExtraData;

	} mDynaData;
	
};

class SoundStreamFader : public Object
{
public:
	SoundStream*			mStream;

	float					mFadeVolume;
	float					mFadeVolumeTo;
	float					mFadeVolumeStep;
	void					Update();

};

//
// Ambient sounds must be updated every update, or they lose their volume...
//
class AmbientSound
{
public:
	AmbientSound();
	virtual ~AmbientSound();
	
	inline void				SetSound(SoundLoop& theLoop) {mLoop=&theLoop;}
	inline void				SetSoundLoop(SoundLoop& theLoop) {mLoop=&theLoop;}
	void					Update();
	inline AmbientSound&	operator=(float theValue) {mVolume=_max(mVolume,theValue);return *this;}
	operator float&			() {return mVolume;}

	void					SetVolume(float theValue) {mVolume=_clamp(0.0f,_max(mVolume,theValue),1.0f);}
	float&					GetVolume() {return mVolume;}
	float&					GetPitch() {return mLoop->mPitch;}
	void					SetPitch(float thePitch) {mLoop->SetPitch(thePitch);}


public:
	float					mVolume;
	float					mOldVolume;
	float					mFadeMultiplier;	// How fast we fade away... 0.0 = stop instantly.
	SoundLoop*				mLoop;
};

//
// Ambient quick sounds are sounds that play just once, but otherwise act just like ambient sounds
//
class AmbientQuickSound
{
public:
	AmbientQuickSound();
	virtual ~AmbientQuickSound();

	inline void					SetSound(Sound& theSound) {mSound=&theSound;}
	void						Update();
	inline AmbientQuickSound&	operator=(float theValue) {mVolume=_max(mVolume,theValue);return *this;}
	operator					float&() {return mVolume;}

public:
	float					mVolume;
	float					mOldVolume;
	Sound*					mSound;
};

extern Audio *gAudioPtr;
