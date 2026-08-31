#include "rapt.h"
#include "rapt_audio.h"
#include "sound_core.h"
#include "os_core.h"

Audio *gAudioPtr=NULL;

int gSoundDelay=0;
void DelaySound(int theWait) {gSoundDelay=theWait;}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
// Audio
// 
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
Audio::Audio()
{
	mSoundVolume=1.0f;
	mMusicVolume=.75f;
	mClipSoundVolume=1.0f;
	mClipMusicVolume=1.0f;

	mSoundList.Unique();
	SetFeatures(CPU_UPDATE);
	gAudioPtr=this;

	mPauseCount=0;
	mMuteSounds=false;

	ForceEchoes(false);

	Set3DSoundCenter(Vector(0,0,0),99999,999999);
}

Audio::~Audio()
{
	if (gAudioPtr==this) gAudioPtr=NULL;
}

void Audio::Go()
{
	gAppPtr->mBackgroundProcesses+=this;
	Sound_Core::Startup();

	Load();

	SetSoundVolume(mSoundVolume);
	SetMusicVolume(mMusicVolume);
	SetEchoData(gAppPtr->SecondsToUpdates(.25f),3,.25f);
}

void Audio::Stop()
{
	Sound_Core::Shutdown();
	if (gAppPtr) 
	{
		gAppPtr->mBackgroundProcesses-=this;
		Save();
	}
}

void Audio::Load()
{
    //gAppPtr->mSettings.RegisterVariable("Audio.SoundVolume",mSoundVolume,1.0f);
    //gAppPtr->mSettings.RegisterVariable("Audio.MusicVolume",mMusicVolume,.75f);
	if (gAppPtr->mSettings.Exists("Audio.SoundVolume")) mSoundVolume=gAppPtr->mSettings.GetFloat("Audio.SoundVolume");
	if (gAppPtr->mSettings.Exists("Audio.MusicVolume")) mMusicVolume=gAppPtr->mSettings.GetFloat("Audio.MusicVolume");
}

void Audio::Save()
{
	gAppPtr->mSettings.SetFloat("Audio.SoundVolume",mSoundVolume);
	gAppPtr->mSettings.SetFloat("Audio.MusicVolume",mMusicVolume);
}

void Audio::SetMusicVolume(float theVolume)
{
	mMusicVolume=theVolume;
	Sound_Core::SetGlobalMusicVolume(theVolume*mClipMusicVolume);

	bool aPauseMusic=false;
	if (mMusicVolume<=0.001f) aPauseMusic=true;
	EnumList(Music,aM,mMusicList) aM->Pause(aPauseMusic);
#ifdef SUPPORT_MUSIC_SAMPLES
	EnumList(SoundStream,aSS,mMusicSampleList) aSS->SetVolume(theVolume*mClipMusicVolume);
#endif
}

void Audio::SetSoundVolume(float theVolume)
{
	mSoundVolume=theVolume;

#ifdef SUPPORT_MUSIC_SAMPLES
	if (mMusicSampleList.GetCount()) 
	{
		Sound_Core::SetGlobalSoundVolume(1.0f);
		EnumList(Sound,aS,mSoundList) EnumList(SoundInstance,aSI,aS->mBufferList) aSI->SetVolume(theVolume);
		EnumList(SoundStream,aSS,mSoundSteamList) aSS->SetVolume(theVolume);
	}
	else Sound_Core::SetGlobalSoundVolume(theVolume*mClipSoundVolume);
#else
	Sound_Core::SetGlobalSoundVolume(theVolume*mClipSoundVolume);
#endif
}

void Audio::Multitasking(bool isForeground)
{
	if (!isForeground) mPauseCount++;
	else mPauseCount=_max(0,mPauseCount-1);
    
//    gOut.Out("Pause Audio: %d (%d %d)",(isForeground&(mPauseCount==0)),isForeground,mPauseCount);
   
    Sound_Core::Multitasking((isForeground&(mPauseCount==0)));
}

void Audio::Pause(bool doPause)
{
	if (doPause) mPauseCount++;
	else mPauseCount=_max(0,mPauseCount-1);
	Sound_Core::Pause((mPauseCount!=0));
}


void Audio::Update()
{
	mUpdateSoundList.Update();
}

void Audio::StopAllSound()
{
	EnumList(Sound,aS,mUpdateSoundList) aS->Stop();
}

void Audio::StopAllMusic()
{
	EnumList(Music,aM,mMusicList) aM->Stop();
#ifdef SUPPORT_MUSIC_SAMPLES
	EnumList(SoundStream,aS,mMusicSampleList) aS->Stop();
#endif
}

#ifdef SUPPORT_MUSIC_SAMPLES
void Audio::SetMusicSample(SoundStream* theSound)
{
	mSoundList-=theSound;
	mMusicSampleList+=theSound;
	theSound->SetLooping(true);
}
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
// SoundInstance
// 
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
SoundInstance::SoundInstance(unsigned int theHandle)
{
	mBuffer=Sound_Core::GetSoundBuffer(theHandle);
	mFrequency=(float)Sound_Core::GetSoundBufferFrequency(mBuffer);
	mVolume=1.0f;
	mClipVolume=1.0f;
}

void SoundInstance::Play() {if (gAudio.mMuteSounds) return;Sound_Core::PlaySoundBuffer(mBuffer);}
void SoundInstance::SetVolume(float theVolume) 
{
#ifdef SUPPORT_MUSIC_SAMPLES
	if (gAudioPtr->mMusicSampleList.GetCount()) theVolume*=gAudio.mSoundVolume;
#endif
	mVolume=theVolume;Sound_Core::SetSoundBufferVolume(mBuffer,theVolume*mClipVolume);
}
void SoundInstance::SetClipVolume(float theVolume) {mClipVolume=theVolume;SetVolume(mVolume);}
void SoundInstance::SetPitch(float thePitch) {Sound_Core::SetSoundBufferFrequency(mBuffer,mFrequency*thePitch);}
bool SoundInstance::IsPlaying() {return Sound_Core::IsSoundBufferPlaying(mBuffer);}
void SoundInstance::Stop() {Sound_Core::StopSoundBuffer(mBuffer);}
void SoundInstance::SetLooping(bool theState) {Sound_Core::SetSoundBufferLooping(mBuffer,theState);}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
// Sound
// 
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
Sound::Sound()
{
	if (gAudioPtr) gAudioPtr->mSoundList+=this;
}

Sound::~Sound()
{
	_FreeList(SoundInstance,mBufferList);
	Sound_Core::UnloadSound(mHandle);
	if (gAudioPtr) gAudioPtr->mSoundList-=this;
}

void Sound::Load(String theFilename, int theDuplicates)
{
	//if (IsLoaded()) return; // Already loaded!

	String aFN=theFilename;aFN+=".ogg";
    if (!DoesFileExist(aFN))
    {
        aFN=theFilename;aFN+=".caf";
        if (!DoesFileExist(aFN))
        {
            aFN=theFilename;aFN+=".wav";
            if (!DoesFileExist(aFN))
            {
                aFN=theFilename;aFN+=".mp3";
                if (!DoesFileExist(aFN))
                {
                    OS_Core::Printf("Sound Not Found: [%s]",theFilename.c());
                }
            }
        }
    }

	mHandle=Sound_Core::LoadSound(aFN.c(),theDuplicates);
	mDuplicates=theDuplicates;
}

void Sound::SetVolume(float theVolume)
{
	mVolume=theVolume;
	EnumList(SoundInstance,aSI,mBufferList) aSI->SetVolume(aSI->mVolume*mVolume);
}

void Sound::SetClipVolume(float theVolume)
{
	mClipVolume=theVolume;
	EnumList(SoundInstance,aSI,mBufferList) aSI->SetClipVolume(mClipVolume);
	SetVolume(mVolume);
}

SoundInstance* Sound::GetSoundInstance()
{
	SoundInstance *aSI=NULL;
	EnumList(SoundInstance,aSearchSI,mBufferList) if (!aSearchSI->IsPlaying()) {aSI=aSearchSI;break;}
	if (!aSI) 
	{
		if (mBufferList.GetCount()<mDuplicates)
		{
			aSI=new SoundInstance(mHandle);
			aSI->SetClipVolume(mClipVolume);
			mBufferList+=aSI;
		}
	}
	return aSI;
}

SoundInstance* Sound::StealSoundInstance()
{
	SoundInstance *aSI=GetSoundInstance();
	mBufferList-=aSI;

	aSI->SetClipVolume(1.0f);
	return aSI;
}

void Sound::Play(float theVolume)
{
	if (gAudio.mMuteSounds) return;
	if (gAudioPtr) if (gAudioPtr->mEchoes.mForceEchoes) {PlayEchoed(theVolume);return;}

	if (gSoundDelay>0)
	{
		if (gAudioPtr) SoundDelayed* aSD=new SoundDelayed(this,1.0f,theVolume,gSoundDelay);
		gSoundDelay=0;
		return;
	}

	PlayNormal(theVolume);
}

void Sound::PlayNormal(float theVolume)
{
	if (gAudio.mMuteSounds) return;
	SoundInstance *aSI=GetSoundInstance();
	if (aSI)
	{
		aSI->SetVolume(theVolume*mVolume);
		aSI->SetPitch(1.0f);
		aSI->Play();
	}
}

void Sound::Play3D(Vector thePos, float theVolume)
{
	if (gAudio.mMuteSounds) return;

#ifdef SUPPORT_MUSIC_SAMPLES
	if (gAudioPtr->mMusicSampleList.GetCount()) theVolume*=gAudio.mSoundVolume;
#endif

	if (gAudioPtr) 
	{
		theVolume=gAudioPtr->Get3DVolume(thePos,theVolume);
		if (gAudioPtr->mEchoes.mForceEchoes) {PlayEchoed(theVolume);return;}
	}

	if (gSoundDelay>0)
	{
		if (gAudioPtr) SoundDelayed3D* aSD=new SoundDelayed3D(thePos,this,1.0f,theVolume,gSoundDelay);
		gSoundDelay=0;
		return;
	}


	PlayNormal(theVolume);
}

void Sound::Play2D(Vector thePos, float theVolume)
{
	if (gAudio.mMuteSounds) return;

#ifdef SUPPORT_MUSIC_SAMPLES
	if (gAudioPtr->mMusicSampleList.GetCount()) theVolume*=gAudio.mSoundVolume;
#endif

	if (gAudioPtr) 
	{
		theVolume=gAudioPtr->Get2DVolume(thePos,theVolume);
		if (gAudioPtr->mEchoes.mForceEchoes) {PlayEchoed(theVolume);return;}
	}

	if (gSoundDelay>0)
	{
		if (gAudioPtr) SoundDelayed2D* aSD=new SoundDelayed2D(thePos,this,1.0f,theVolume,gSoundDelay);
		gSoundDelay=0;
		return;
	}
	PlayNormal(theVolume);
}

void Sound::PlayPitched(float thePitch, float theVolume)
{
	if (gAudio.mMuteSounds) return;

	if (gAudioPtr) if (gAudioPtr->mEchoes.mForceEchoes) {PlayPitchedEchoed(thePitch,theVolume);return;}

	if (gSoundDelay>0)
	{
		if (gAudioPtr) SoundDelayed* aSD=new SoundDelayed(this,thePitch,theVolume,gSoundDelay);
		gSoundDelay=0;
		return;
	}
	PlayPitchedNormal(thePitch,theVolume);
}

void Sound::PlayPitched3D(Vector thePos, float thePitch, float theVolume)
{
	if (gAudio.mMuteSounds) return;

	if (gAudioPtr)
	{
		theVolume=gAudioPtr->Get3DVolume(thePos,theVolume);
		if (gAudioPtr->mEchoes.mForceEchoes) {PlayPitchedEchoed(thePitch,theVolume);return;}
	}

	if (gSoundDelay>0)
	{
		if (gAudioPtr) SoundDelayed3D* aSD=new SoundDelayed3D(thePos,this,thePitch,theVolume,gSoundDelay);
		gSoundDelay=0;
		return;
	}

	PlayPitchedNormal(thePitch,theVolume);
}

void Sound::PlayPitched2D(Vector thePos, float thePitch, float theVolume)
{
	if (gAudio.mMuteSounds) return;

	if (gAudioPtr)
	{
		theVolume=gAudioPtr->Get2DVolume(thePos,theVolume);
		if (gAudioPtr->mEchoes.mForceEchoes) {PlayPitchedEchoed(thePitch,theVolume);return;}
	}

	if (gSoundDelay>0)
	{
		if (gAudioPtr) SoundDelayed2D* aSD=new SoundDelayed2D(thePos,this,thePitch,theVolume,gSoundDelay);
		gSoundDelay=0;
		return;
	}

	PlayPitchedNormal(thePitch,theVolume);
}

void Sound::PlayPitchedNormal(float thePitch, float theVolume)
{
	if (gAudio.mMuteSounds) return;

	SoundInstance *aSI=GetSoundInstance();
	if (aSI)
	{
		aSI->SetVolume(theVolume*mVolume);
		aSI->SetPitch(thePitch);
		aSI->Play();
	}
}

void Sound::PlayEchoed(float theVolume)
{
	if (gAudio.mMuteSounds) return;

	if (gAudioPtr) SoundEcho *aE=new SoundEcho(this,1.0f,gAudioPtr->mEchoes.mDelay,theVolume,gAudioPtr->mEchoes.mCount,gAudioPtr->mEchoes.mVolumeMultiplier);
}

void Sound::PlayPitchedEchoed(float thePitch, float theVolume)
{
	if (gAudio.mMuteSounds) return;

	if (gAudioPtr) SoundEcho *aE=new SoundEcho(this,thePitch,gAudioPtr->mEchoes.mDelay,theVolume,gAudioPtr->mEchoes.mCount,gAudioPtr->mEchoes.mVolumeMultiplier);
}

void Sound::Stop()
{
	EnumList(SoundInstance,aSI,mBufferList) aSI->Stop();
	EnumList(SoundDelayed,aO,gAudioPtr->mUpdateSoundList) if (aO->mID==ID_SOUNDDELAYED) if (aO->mSound==this) aO->Kill();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
// SoundLoop
// 
//
/////////////////////////////////////////////////////////////////////////////////////////////////////

SoundLoop::SoundLoop()
{
	mInstance=NULL;
	mCounter=0;
	mFadeInSpeed=0;
	mFadeOutSpeed=0;
	mFadeVolume=1.0f;
	mVolume=1.0f;
	mUpdateType=0;
}

SoundLoop::~SoundLoop()
{
	StopAll();

	if (gAudioPtr) gAudioPtr->mUpdateSoundList-=this;
	if (mHandle>=0) Sound_Core::UnloadSound(mHandle);
	mInstance=NULL;
}

void SoundLoop::Load(String theFilename)
{

	//String aFN=theFilename;aFN+=".ogg";if (!DoesFileExist(aFN)) {aFN=theFilename;aFN+=".caf";if (!DoesFileExist(aFN)) aFN=theFilename;aFN+=".wav";}
	//mSound.Load(aFN.c(),10);
	mSound.Load(theFilename,1);
	mInstance=mSound.GetSoundInstance();
	if (mInstance) mInstance->SetLooping(true);
}

void SoundLoop::Go()
{
	if (!mInstance) return;
	if (mCounter==0) mInstance->Play();
	mFadeVolume=1.0f;
	mCounter++;
}

void SoundLoop::Stop()
{
	if (!mInstance) return;
	if (--mCounter<=0) 
	{
		mInstance->Stop();
		mFadeVolume=0.0f;
		mCounter=0;
	}
}

void SoundLoop::FadeIn(float theFadeInSpeed)
{
	mFadeInSpeed=theFadeInSpeed;
	if (mInstance) 
	{
		mUpdateType=1;
		if (gAudioPtr) gAudioPtr->mUpdateSoundList+=this;
		if (mCounter==0) 
		{
			mFadeVolume=0;
			mInstance->SetVolume(mVolume*mFadeVolume);
			mInstance->Play();
		}
	}
	mCounter++;
}

void SoundLoop::FadeOut(float theFadeOutSpeed)
{
	mFadeOutSpeed=theFadeOutSpeed;
	if (mInstance && mCounter>0) 
	{
		mUpdateType=2;
		if (gAudioPtr) gAudioPtr->mUpdateSoundList+=this;
	}
}

void SoundLoop::Update()
{
	if (!mUpdateType || !mInstance) return;
	switch (mUpdateType)
	{
	case 1:
		{
			mFadeVolume+=mFadeInSpeed;
			if (mFadeVolume>1.0f) 
			{
				if (gAudioPtr) gAudioPtr->mUpdateSoundList-=this;
				mFadeVolume=1.0f;
			}
			mInstance->SetVolume(mVolume*mFadeVolume);
			break;
		}
	case 2:
		{
			mFadeVolume-=mFadeOutSpeed;
			if (mFadeVolume<=0.0f) 
			{
				if (gAudioPtr) gAudioPtr->mUpdateSoundList-=this;
				mFadeVolume=0.0f;
				Stop();
			}
			mInstance->SetVolume(mVolume*mFadeVolume);
			break;
		}
	}
}

SoundLoopGroup::SoundLoopGroup()
{
}

SoundLoopGroup::~SoundLoopGroup()
{
}

void SoundLoopGroup::Load(String theFilename, int theCount)
{
	mSound.Load(theFilename,theCount);
	mLoops.GuaranteeSize(theCount);
	foreach(aL,mLoops) 
	{
		aL.mInstance=mSound.GetSoundInstance();
		aL.mInstance->SetLooping(true);
	}
}

void SoundLoopGroup::Load(Sound& theSound)
{
	mLoops.GuaranteeSize(theSound.mDuplicates);
	foreach(aL,mLoops) 
	{
		aL.mSound.mBufferList+=theSound.StealSoundInstance();
		aL.mInstance=aL.mSound.GetSoundInstance();
		aL.mInstance->SetLooping(true);
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
// Echo
// 
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
SoundEcho::SoundEcho(Sound *theSound, float thePitch, float theDelay, float theStartVolume, int theEchoCount, float theVolumeMult)
{
	mID=ID_SOUNDECHO;
	mSound=theSound;
	mDelay=theDelay;
	mVolume=theStartVolume;
	mEchoCount=theEchoCount;
	mDelayCounter=0;
	mVolumeMult=theVolumeMult;
	mPitch=thePitch;

	if (gAudioPtr) gAudioPtr->mUpdateSoundList+=this;
}

void SoundEcho::Update()
{
	if (--mDelayCounter<=0)
	{
		mDelayCounter=mDelay;

		mSound->PlayPitchedNormal(mPitch,mVolume);
		mVolume*=mVolumeMult;
		if (mEchoCount<=0) Kill();
		mEchoCount--;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
// Delayed Sound
// 
//
/////////////////////////////////////////////////////////////////////////////////////////////////////
SoundDelayed::SoundDelayed(Sound* theSound, float thePitch, float theVolume, int theDelay)
{
	mID=ID_SOUNDDELAYED;
	mSound=theSound;
	mDelay=theDelay;
	mVolume=theVolume;
	mPitch=thePitch;

	if (gAudioPtr) gAudioPtr->mUpdateSoundList+=this;
}

void SoundDelayed::Update() {if (--mDelay<=0) PlayIt();}
void SoundDelayed::PlayIt() {mSound->PlayPitched(mPitch,mVolume);Kill();}
void SoundDelayed3D::PlayIt() {mSound->PlayPitched3D(mPos,mPitch,mVolume);Kill();}
void SoundDelayed2D::PlayIt() {mSound->PlayPitched2D(mPos,mPitch,mVolume);Kill();}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
// Music
// 
//
/////////////////////////////////////////////////////////////////////////////////////////////////////

Music::Music()
{
	mPaused=false;
	mCurrentHandle=0;
	mVolume=1.0f;
	mFadeInVolume=1.0f;
	mFadeOutVolume=1.0f;
	mCurrentSongPtr=NULL;
	mMaxChannels=0;
	mLoaded=false;

	if (gAudioPtr) if (gAudioPtr->mMusicVolume<.001f) mPaused=true;
}

Music::~Music()
{
	_FreeList(Song,mSongList);
	if (mCurrentHandle) Sound_Core::UnloadMusic(mCurrentHandle);
	if (gAudioPtr) gAudioPtr->mMusicList-=this;
}

void Music::Load(String theFilename)
{
	//if (IsLoaded()) return;

	if (gAudioPtr) 
	{
		gAudioPtr->mMusicList+=this;
		if (gAudioPtr->mMusicVolume<.001f) mPaused=true;
	}

	mLoaded=false;
	String aJukeboxFile=Sprintf("package://%s/music.txt",theFilename.c());
	String aMusicFile=Sprintf("package://%s/music.mo3",theFilename.c());

	if (!DoesFileExist(aMusicFile)) aMusicFile=Sprintf("package://%s/music.it",theFilename.c());
	if (DoesFileExist(aMusicFile) && DoesFileExist(aJukeboxFile))
	{
		mLoaded=true;
		//
		// A Jukebox file exists, so this is MO3 music...
		//
		for (int aCount=0;aCount<mMaxHandles;aCount++) mHandle[aCount]=Sound_Core::LoadMusic(aMusicFile.c());
		
		//
		// Parse the jukebox file...
		//
		IOBuffer aBuffer;
		aBuffer.Load(aJukeboxFile);
		{
			Song *aCurrentSong=NULL;

			while (!aBuffer.IsEnd())
			{
				String aLine=aBuffer.ReadLine();
				if (aLine.Len()==0) continue;
				if (aLine[0]=='/') continue;

				aLine.RemoveLeadingSpaces();
				aLine.RemoveTrailingSpaces();

				String aCommand=aLine.GetToken('=');
				String aValue=aLine.GetNextToken();

				if (aCommand=="song")
				{
					String aTitle=aValue.GetToken(':');
					String aPosString=aValue.GetNextToken();
					int aPos=0;

					if (aPosString.StartsWith("0x") || aPosString.StartsWith("0X")) 
					{
						String aS=aPosString.GetSegment(2,9999);
						sscanf(aS.c(),"%x",&aPos);
					}
					else sscanf(aPosString.c(),"%d",&aPos);

					Song *aSong=new Song;
					aSong->mHighestChannel=0;
					aCurrentSong=aSong;

					aCurrentSong->mName=aTitle;
					aCurrentSong->mOffset=aPos;
					mSongList+=aSong;
				}
				if (aCommand=="track")
				{
					if (aCurrentSong)
					{
						String aTrackName=aValue.GetToken(':');
						String aTrackList=aValue.GetNextToken();
						Track *aTrack=new Track;
						aCurrentSong->mTrackList+=aTrack;
						aTrack->mName=aTrackName;
						String aChannel=aTrackList.GetToken(',');

						for (;;)
						{
							if (aChannel.Len()<=0) break;

							int aChan=aChannel.ToInt()-1;
							if (aChan>=0)
							{
								aTrack->mChannel[aTrack->mChannel.Size()]=aChan;
								aCurrentSong->mChannelVolume[aChan]=1.0f;
								aCurrentSong->mChannelWantVolume[aChan]=1.0f;

								mMaxChannels=_max(aChan,mMaxChannels);
							}

							aChannel=aTrackList.GetNextToken(',');
						}

						aCurrentSong->mHighestChannel=_max(aCurrentSong->mHighestChannel,aTrack->mChannel.Size());
					}
				}
			}
		}
	}
	else
	{
		//
		// No Jukebox file means we're going to just be
		// streaming WAV's.
		//
	}
}

void Music::SetVolume(float theVolume)
{
	for (int aCount=0;aCount<mMaxHandles;aCount++) Sound_Core::SetMusicVolume(mHandle[aCount],theVolume);
	mVolume=theVolume;
}

void Music::Update()
{
	bool aRemoveUpdates=true;
	if (mCrossfadeSong)
	{
		mFadeInVolume=_min(1.0f,mFadeInVolume+mFadeVolumeStep);
		mFadeOutVolume=_max(0.0f,mFadeOutVolume-mFadeVolumeStep);

		Sound_Core::SetMusicVolume(mHandle[mFadeOutHandle],mFadeOutVolume);
		Sound_Core::SetMusicVolume(mHandle[mCurrentHandle],mFadeInVolume);

		if (mFadeInVolume>=1.0f && mFadeOutVolume<=0.0f) 
		{
			Sound_Core::StopMusic(mHandle[mFadeOutHandle]);
		}
		else aRemoveUpdates=false;
	}
	if (mCrossfadeTrack)
	{
		if (mCurrentSongPtr)
		{
			for (int aCount=0;aCount<mCurrentSongPtr->mChannelVolume.Size();aCount++)
			{
				if (mCurrentSongPtr->mChannelVolume[aCount]>mCurrentSongPtr->mChannelWantVolume[aCount])
				{
					mCurrentSongPtr->mChannelVolume[aCount]=_max(mCurrentSongPtr->mChannelWantVolume[aCount],mCurrentSongPtr->mChannelVolume[aCount]-mFadeVolumeStep);
					Sound_Core::SetMusicTrackVolume(mHandle[mCurrentHandle],aCount,mCurrentSongPtr->mChannelVolume[aCount]);
					aRemoveUpdates=false;
				}
				if (mCurrentSongPtr->mChannelVolume[aCount]<mCurrentSongPtr->mChannelWantVolume[aCount])
				{
					mCurrentSongPtr->mChannelVolume[aCount]=_min(mCurrentSongPtr->mChannelWantVolume[aCount],mCurrentSongPtr->mChannelVolume[aCount]+mFadeVolumeStep);
					Sound_Core::SetMusicTrackVolume(mHandle[mCurrentHandle],aCount,mCurrentSongPtr->mChannelVolume[aCount]);
					aRemoveUpdates=false;
				}
			}
		}
	}

	if (aRemoveUpdates) if (gAudioPtr) gAudioPtr->mUpdateSoundList-=this;
}

void Music::Play(String theSong, bool clearTrackInfo)
{
	if (theSong==mCurrentSong) return;

	for (int aCount=0;aCount<mMaxHandles;aCount++) Sound_Core::StopMusic(mHandle[aCount]);
	mCurrentHandle=0;

	EnumList(Song,aSong,mSongList) if (aSong->mName==theSong) 
	{
		if (clearTrackInfo) SetAllTrackVolume(theSong,1.0f);
		if (!mPaused) {Sound_Core::PlayMusic(mHandle[mCurrentHandle],aSong->mOffset);}
		Sound_Core::SetMusicVolume(mHandle[mCurrentHandle],1.0f);

		mCurrentSong=theSong;
		mPausedSong=theSong;
		mCurrentSongPtr=aSong;
		break;
	}

	mCrossfadeSong=false;
	mCrossfadeTrack=false;
}

void Music::Play(String theSong, String theTrack)
{
	if (theSong!=mCurrentSong)
	{
		for (int aCount=0;aCount<mMaxHandles;aCount++) Sound_Core::StopMusic(mHandle[aCount]);
		mCurrentHandle=0;
	}

	EnumList(Song,aSong,mSongList) if (aSong->mName==theSong) 
	{
		SetAllOtherTrackVolume(theSong,theTrack,0.0f);
		SetTrackVolume(theSong,theTrack,1.0f);
		if (theSong!=mCurrentSong)
		{
            if (!mPaused) Sound_Core::PlayMusic(mHandle[mCurrentHandle],aSong->mOffset);
			Sound_Core::SetMusicVolume(mHandle[mCurrentHandle],1.0f);
			mCurrentSong=theSong;
			mPausedSong=theSong;
			mCurrentSongPtr=aSong;
		}
		break;
	}

	mCrossfadeSong=false;
	mCrossfadeTrack=false;
}

void Music::CrossFade(String theSong, float theUpdatesToCrossFade)
{
	if (mCurrentSong) if (theSong==mCurrentSong) return;
	if (theUpdatesToCrossFade==-1) theUpdatesToCrossFade=gAppPtr->SecondsToUpdates(1.0f);

	mCrossfadeSong=true;
	mCrossfadeTrack=false;

	mFadeOutVolume=mFadeInVolume;
	mFadeInVolume=0.0f;
	mFadeVolumeStep=1.0f/theUpdatesToCrossFade;
	mFadeOutHandle=mCurrentHandle;
	mCurrentHandle=1-mCurrentHandle;

	if (theSong.Len()<=0)
	{
		Sound_Core::StopMusic(mHandle[mCurrentHandle]);
		mCurrentSong="";
		mPausedSong="";
	}
	else
	{
		EnumList(Song,aSong,mSongList) if (aSong->mName==theSong) 
		{
			SetAllTrackVolume(theSong,1.0f);
			Sound_Core::SetMusicVolume(mHandle[mCurrentHandle],0.0f);

            if (!mPaused) Sound_Core::PlayMusic(mHandle[mCurrentHandle],aSong->mOffset);
			mCurrentSong=theSong;
			mPausedSong=theSong;
			mCurrentSongPtr=aSong;
			break;
		}
	}

	if (gAudioPtr) gAudioPtr->mUpdateSoundList+=this;
}

void Music::CrossFade(String theSong, String theTrack, float theUpdatesToCrossFade)
{
	//if (theSong==mCurrentSong) return;

	if (theUpdatesToCrossFade==-1) theUpdatesToCrossFade=gAppPtr->SecondsToUpdates(1.0f);

	mFadeVolumeStep=1.0f/theUpdatesToCrossFade;
	if (theSong!=mCurrentSong)
	{
		mFadeOutVolume=mFadeInVolume;
		mFadeInVolume=0.0f;
		mFadeOutHandle=mCurrentHandle;
		mCurrentHandle=1-mCurrentHandle;

		mCrossfadeSong=true;
	}
	if (theSong.Len()<=0)
	{
		Sound_Core::StopMusic(mHandle[mCurrentHandle]);
	}
	else
	{
		EnumList(Song,aSong,mSongList) if (aSong->mName==theSong) 
		{
			SetAllOtherTrackWantVolume(theSong,theTrack,0.0f);
			SetAllTrackVolume(theSong,0.0f);
			SetTrackVolume(theSong,theTrack,1.0f);
			if (theSong!=mCurrentSong)
			{
                if (!mPaused) Sound_Core::PlayMusic(mHandle[mCurrentHandle],aSong->mOffset);
				mCurrentSong=theSong;
				mPausedSong=theSong;
				mCurrentSongPtr=aSong;
			}
			break;
		}
	}

	mCrossfadeTrack=true;
	if (gAudioPtr) gAudioPtr->mUpdateSoundList+=this;
}



void Music::Stop()
{
	for (int aCount=0;aCount<mMaxHandles;aCount++) Sound_Core::StopMusic(mHandle[aCount]);
	mCurrentHandle=0;
    mCurrentSong="";
	mPausedSong="";
	mCurrentSongPtr=NULL;

	mCrossfadeSong=false;
	mCrossfadeTrack=false;
}

void Music::Pause(bool theState)
{
	if (mPaused==theState) return;

	mPaused=theState;
	if (theState) 
	{
		mPausedSong=mCurrentSong;

		for (int aCount=0;aCount<mMaxHandles;aCount++) Sound_Core::StopMusic(mHandle[aCount]);
		mCurrentHandle=0;
		mCurrentSong="";
		mCurrentSongPtr=NULL;
		mCrossfadeSong=false;
		mCrossfadeTrack=false;
	}
	else
    {
        mCurrentSong="";
        Play(mPausedSong,false);
    }
}


void Music::SetTrackVolume(String theSong, String theTrack, float theVolume)
{
	EnumList(Song,aSong,mSongList) if (aSong->mName==theSong) 
	{
		if (aSong->mTrackList.GetCount()==0)
		{
			for (int aCount=0;aCount<mMaxChannels;aCount++)
			{
				Sound_Core::SetMusicTrackVolume(mHandle[mCurrentHandle],aCount,theVolume);
				aSong->mChannelVolume[aCount]=theVolume;
				aSong->mChannelWantVolume[aCount]=theVolume;
			}
		}
		else
		{
			EnumList(Track,aTrack,aSong->mTrackList)
			{
				if (aTrack->mName==theTrack)
				for (int aChanCount=0;aChanCount<aTrack->mChannel.Size();aChanCount++) 
				{
					Sound_Core::SetMusicTrackVolume(mHandle[mCurrentHandle],aTrack->mChannel[aChanCount],theVolume);
					aSong->mChannelVolume[aTrack->mChannel[aChanCount]]=theVolume;
					aSong->mChannelWantVolume[aTrack->mChannel[aChanCount]]=theVolume;
				}
			}
		}
		break;
	}
}

void Music::SetAllTrackVolume(String theSong, float theVolume)
{
	EnumList(Song,aSong,mSongList) if (aSong->mName==theSong) 
	{
		for (int aCount=0;aCount<mMaxChannels;aCount++)
		{
			Sound_Core::SetMusicTrackVolume(mHandle[mCurrentHandle],aCount,theVolume);
			aSong->mChannelVolume[aCount]=theVolume;
			aSong->mChannelWantVolume[aCount]=theVolume;
		}
		break;
	}
}

void Music::SetAllOtherTrackVolume(String theSong, String theTrack, float theVolume)
{
	EnumList(Song,aSong,mSongList) if (aSong->mName==theSong) 
	{
		if (aSong->mTrackList.GetCount()==0)
		{
			for (int aCount=0;aCount<mMaxChannels;aCount++)
			{
				Sound_Core::SetMusicTrackVolume(mHandle[mCurrentHandle],aCount,theVolume);
				aSong->mChannelVolume[aCount]=theVolume;
				aSong->mChannelWantVolume[aCount]=theVolume;
			}
		}
		else
		{
			EnumList(Track,aTrack,aSong->mTrackList)
			{
				if (aTrack->mName!=theTrack)
				{
					for (int aChanCount=0;aChanCount<aTrack->mChannel.Size();aChanCount++) 
					{
						Sound_Core::SetMusicTrackVolume(mHandle[mCurrentHandle],aTrack->mChannel[aChanCount],theVolume);
						aSong->mChannelVolume[aTrack->mChannel[aChanCount]]=theVolume;
						aSong->mChannelWantVolume[aTrack->mChannel[aChanCount]]=theVolume;
					}
				}
			}
		}
		break;
	}
}

void Music::SetTrackWantVolume(String theSong, String theTrack, float theVolume)
{
	EnumList(Song,aSong,mSongList) if (aSong->mName==theSong) 
	{
		if (aSong->mTrackList.GetCount()==0)
		{
			for (int aCount=0;aCount<mMaxChannels;aCount++)
			{
				aSong->mChannelWantVolume[aCount]=theVolume;
			}
		}
		else
		{
			EnumList(Track,aTrack,aSong->mTrackList)
			{
				if (aTrack->mName==theTrack)
					for (int aChanCount=0;aChanCount<aTrack->mChannel.Size();aChanCount++) 
					{
						aSong->mChannelWantVolume[aTrack->mChannel[aChanCount]]=theVolume;
					}
			}
		}
		break;
	}
}

void Music::SetAllTrackWantVolume(String theSong, float theVolume)
{
	EnumList(Song,aSong,mSongList) if (aSong->mName==theSong) 
	{
		if (aSong->mTrackList.GetCount()==0)
		{
			for (int aCount=0;aCount<mMaxChannels;aCount++)
			{
				aSong->mChannelWantVolume[aCount]=theVolume;
			}
		}
		else
		{
			EnumList(Track,aTrack,aSong->mTrackList)
			{
				for (int aChanCount=0;aChanCount<aTrack->mChannel.Size();aChanCount++) 
				{
					aSong->mChannelWantVolume[aTrack->mChannel[aChanCount]]=theVolume;
				}
			}
		}
		break;
	}
}

void Music::SetAllOtherTrackWantVolume(String theSong, String theTrack, float theVolume)
{
	EnumList(Song,aSong,mSongList) if (aSong->mName==theSong) 
	{
		if (aSong->mTrackList.GetCount()==0)
		{
			for (int aCount=0;aCount<mMaxChannels;aCount++)
			{
				aSong->mChannelWantVolume[aCount]=theVolume;
			}
		}
		else
		{
			EnumList(Track,aTrack,aSong->mTrackList)
			{
				if (aTrack->mName!=theTrack)
				{
					for (int aChanCount=0;aChanCount<aTrack->mChannel.Size();aChanCount++) 
					{
						aSong->mChannelWantVolume[aTrack->mChannel[aChanCount]]=theVolume;
					}
				}
			}
		}
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
// Sound Streams
// 
//
/////////////////////////////////////////////////////////////////////////////////////////////////////

SoundStream::SoundStream()
{
	if (gAudioPtr) gAudioPtr->mSoundSteamList+=this;
}

SoundStream::~SoundStream()
{
	Stop();
	Sound_Core::UnloadStreamSound(mHandle);
	if (gAudioPtr) gAudioPtr->mSoundSteamList-=this;
}

void SoundStream::Load(String theFilename)
{
	//if (IsLoaded()) return;

	String aFN=theFilename;aFN+=".ogg";
	if (!DoesFileExist(aFN)) 
	{
		aFN=theFilename;aFN+=".caf";
		if (!DoesFileExist(aFN)) 
		{
			aFN=theFilename;aFN+=".mp3";
			if (!DoesFileExist(aFN))
			{
				aFN=theFilename;aFN+=".wav";
				if (!DoesFileExist(aFN))
				{
					aFN=theFilename;aFN+=".mp3";
				}
			}
		}
	}
	mHandle=Sound_Core::LoadStreamSound(aFN.c());
}

void SoundStreamDynamic::Load(void (*theCallback)(short* theBuffer, unsigned int theLength, void* theExtraData), void* theExtraData, int theBufferLength)
{
	//if (IsLoaded()) return;

	mDynaData.mCallback=theCallback;
	mDynaData.mExtraData=theExtraData;
	mHandle=Sound_Core::CreateDynamicSound(&mDynaData,theBufferLength);
}

void SoundStream::SetLooping(bool theState) 
{
	Sound_Core::SetSoundBufferLooping(mHandle,theState);
}

void SoundStream::Play(float theVolume)
{
	if (gAudio.mMuteSounds) return;

#ifdef SUPPORT_MUSIC_SAMPLES
	if (gAudioPtr->mMusicSampleList.GetCount()) 
	{
		bool aWasMusic=false;
		EnumList(SoundStream,aSS,gAudioPtr->mMusicSampleList) if (aSS==this) {theVolume*=gAudio.mMusicVolume;aWasMusic=true;break;}
		if (!aWasMusic) theVolume*=gAudio.mSoundVolume;
	}
#endif

	Sound_Core::SetSoundBufferVolume(mHandle,theVolume);
	Sound_Core::PlaySoundBuffer(mHandle);
}

void SoundStream::FadeIn(float theTime)
{
	if (IsPlaying()) return;

	EnumList(SoundStreamFader,aSSF,gAudioPtr->mUpdateSoundList) if (aSSF->mStream==this) aSSF->Kill();

	Play(0);
	SoundStreamFader* aF=new SoundStreamFader;
	aF->mStream=this;
	aF->mFadeVolume=0;
	aF->mFadeVolumeTo=1.0f;
	aF->mFadeVolumeStep=1.0f/gAppPtr->SecondsToUpdates(theTime);
	gAudioPtr->mUpdateSoundList+=aF;
}

void SoundStream::FadeOut(float theTime)
{
	EnumList(SoundStreamFader,aSSF,gAudioPtr->mUpdateSoundList) if (aSSF->mStream==this) aSSF->Kill();

	SoundStreamFader* aF=new SoundStreamFader;
	aF->mStream=this;
	aF->mFadeVolume=1.0f;
	aF->mFadeVolumeTo=0.0f;
	aF->mFadeVolumeStep=-1.0f/gAppPtr->SecondsToUpdates(theTime);
	gAudioPtr->mUpdateSoundList+=aF;
}

void SoundStream::Stop()
{
	Sound_Core::StopSoundBuffer(mHandle);
}

void SoundStream::SetVolume(float theVolume)
{
#ifdef SUPPORT_MUSIC_SAMPLES
	if (gAudioPtr->mMusicSampleList.GetCount()) EnumList(SoundStream,aSS,gAudioPtr->mMusicSampleList) if (aSS==this) {theVolume*=gAudio.mMusicVolume;break;}
#endif

	Sound_Core::SetSoundBufferVolume(mHandle,theVolume);
}

bool SoundStream::IsPlaying()
{
	return Sound_Core::IsSoundBufferPlaying(mHandle);
}

float SoundStream::GetLevel()
{
	float aLeft;
	float aRight;
	Sound_Core::GetStreamLevel(mHandle,&aLeft,&aRight);

	return (float)(aLeft+aRight)/2;
}

float SoundInstance::GetLevel()
{
	float aLeft;
	float aRight;
	Sound_Core::GetStreamLevel(mBuffer,&aLeft,&aRight);
	return (float)(aLeft+aRight)/2;
}


void SoundStream::SetPosition(float theSeconds)
{
	Sound_Core::SetSoundPosition(mHandle,theSeconds);
}

float SoundStream::GetPosition()
{
	return Sound_Core::GetSoundPosition(mHandle);
}

void SoundInstance::SetPosition(float theSeconds)
{
	Sound_Core::SetSoundPosition(mBuffer,theSeconds);
}

float SoundInstance::GetPosition()
{
	return Sound_Core::GetSoundPosition(mBuffer);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
// Sound Streams
// 
//
/////////////////////////////////////////////////////////////////////////////////////////////////////

AmbientSound::AmbientSound()
{
	mVolume=0;
	mOldVolume=0;
	mLoop=NULL;
	mFadeMultiplier=0.5f;
}

AmbientSound::~AmbientSound()
{
	if (mLoop) mLoop->Stop();
}

void AmbientSound::Update()
{
	if (!mLoop || gAppPtr->IsQuit()) return;
	if (mVolume!=mOldVolume)
	{
		if (mOldVolume==0 && mVolume!=0) mLoop->Go();
		if (mVolume==0 && mOldVolume!=0) mLoop->Stop();
	}
	if (mVolume) mLoop->SetVolume(mVolume);

	mOldVolume=mVolume;
	mVolume*=mFadeMultiplier;//=0;
	if (mVolume<.001f) mVolume=0;
}

AmbientQuickSound::AmbientQuickSound()
{
	mVolume=0;
	mOldVolume=0;
	mSound=NULL;
}

AmbientQuickSound::~AmbientQuickSound()
{
	if (mSound) mSound->Stop();
}

void AmbientQuickSound::Update()
{
	if (!mSound) return;
	if (mVolume!=mOldVolume)
	{
		if (mOldVolume==0 && mVolume!=0) 
		{
			mSound->Play(mVolume);
		}
	}
	mOldVolume=mVolume;
	mVolume=0;
}

void SoundStreamFader::Update()
{
	mFadeVolume+=mFadeVolumeStep;
	if (mFadeVolume<=0)
	{
		mFadeVolume=0;
		mStream->Stop();
		gAudioPtr->mUpdateSoundList-=this;
	}
	if (mFadeVolume>=1.0f) 
	{
		gAudioPtr->mUpdateSoundList-=this;
		mFadeVolume=1.0f;
	}

	mStream->SetVolume(mFadeVolume);
}

float Audio::GetMinimumSampleRate()
{
	unsigned int aMin,aMax;
	Sound_Core::GetSampleRate(aMin,aMax);
	return (float)aMin;
}

float Audio::GetMaximumSampleRate()
{
	unsigned int aMin,aMax;
	Sound_Core::GetSampleRate(aMin,aMax);
	return (float)aMax;
}

float Audio::GetFrequency()
{
	return (float)Sound_Core::GetFrequency();
}
