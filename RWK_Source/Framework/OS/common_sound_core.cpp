//
// common_sound_core.cpp -- ONE Sound_Core implementation for every platform.
//
// This file is not compiled on its own.  Each platform's
// Framework/OS/<Platform>/sound_core.cpp is a one-line #include of it, the same
// way OS/<Platform>/gl30.h includes ../common_gl30.h.  That keeps the three
// build systems' non-recursive OS/<Platform>/*.cpp globs working unchanged.
//
// It replaces:
//   * OS/Linux/sound_core.cpp   -- BASS (proprietary, not free to redistribute)
//   * OS/Windows/sound_core.cpp -- BASS, ditto (and 32-bit only)
//   * OS/WASM/sound_core.cpp    -- OpenAL + libvorbis + libopenmpt + an SDL
//                                  music mixer; ~981 lines, and its streaming
//                                  was a full decode, its seek a no-op and its
//                                  level meter binary.
// with SDL2's audio callback (SDL2 was already a dependency on all three) and
// the public-domain stb_vorbis (Framework/OS/ThirdParty/stb).
//
// TRACKER-MODULE MUSIC IS GONE.  The Music/LoadMusic path below is a logging
// stub.  RWK ships 61 sound assets and every one of them is .ogg; the module
// path was wired in RAPT but never reached by the game.
//
// ---------------------------------------------------------------------------
// The model
// ---------------------------------------------------------------------------
// BASS puts samples and channels in one handle space, so this does too: one
// array of Obj, a handle is (index+1), and 0 means "no/failed" exactly as a
// BASS handle of 0 does.  An Obj is one of:
//
//   kSample   decoded PCM held in memory (BASS_SampleLoad).  Not playable.
//   kVoice    one playing instance of a kSample (BASS_SampleGetChannel).
//   kStream   an .ogg decoded INCREMENTALLY by stb_vorbis from the compressed
//             file held in memory, with a real seek (BASS_StreamCreateFile).
//   kDynamic  a push-callback voice: the mixer calls the game's function to
//             fill each block (BASS_StreamCreate + STREAMPROC).  This is what
//             the in-game BeepBox synth plays custom-level music through, and
//             it is the one path that must not regress.
//
// Every playable Obj is a voice with volume, playback rate (the pitch control:
// RAPT's SetPitch multiplies the sample's native rate, so this resamples --
// exactly what BASS_ATTRIB_FREQ did, and just as much "not a time-stretch"),
// loop flag, fractional position, a bus, and its own L/R peak meter.
//
// ---------------------------------------------------------------------------
// Locking -- read this before adding a lock anywhere
// ---------------------------------------------------------------------------
// The mixer runs on SDL's audio thread.  BeepBox's callback takes the game's
// synthesizer lock INSIDE that thread, and the game takes that same lock and
// then calls Sound_Core::StopSoundBuffer (Beepbox.cpp: ~BeepBox, Unload).  So
// if the parameter setters took the audio-device lock, the two locks would be
// taken in opposite orders and the game would deadlock on exit.
//
// Therefore: the PARAMETER setters (play/stop/volume/rate/loop) take NO lock --
// they write plain fields, and the worst case is one 23 ms block of stale
// value.  Only the STRUCTURAL operations (Startup/Shutdown, load, unload,
// create, and the stream seek, which touches the decoder) take the device
// lock, and none of those is ever called under the synthesizer lock.
//

#include "sound_core.h"
#include "os_core.h"

#define __HEADER
#include "common.h"
#undef __HEADER

// stb_vorbis is a .c file meant to be dropped straight into a build.  Pull in
// the standard headers it wants at GLOBAL scope first (they are all guarded, so
// its own copies become no-ops), then compile it inside a namespace so its ~40
// file-scope types -- Codebook, Floor, Residue, Mode, Page ... -- can never
// collide with a RAPT type of the same name.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <limits.h>
#ifndef _WIN32
#include <alloca.h>
#endif

namespace stbv {
#define STB_VORBIS_NO_PUSHDATA_API
#define STB_VORBIS_NO_STDIO
#include "ThirdParty/stb/stb_vorbis.c"
}

namespace Sound_Core
{

// ---------------------------------------------------------------------------
// Tuning.  Nothing downstream may assume a rate: the DEVICE reports its rate
// in Startup and every rate-dependent number is derived from gRate.
// ---------------------------------------------------------------------------
static const int   kRequestRate   = 44100;  // a request only; have.freq wins
static const int   kRequestFrames = 1024;   // ~23 ms at 44.1 kHz
static const int   kMaxObjects    = 512;
static const int   kLevelWindowMs = 20;     // BASS_ChannelGetLevel's window
static const int   kStreamBufFrames = 8192; // stream decode-ahead, per stream

enum ObjKind {kFree=0,kSample,kVoice,kStream,kDynamic,kMusic};
enum Bus     {kBusSound=0,kBusMusic};

// The layout RAPT hands us through CreateDynamicSound's void* -- it is the
// anonymous SoundStreamDynamic::mDynaData in rapt_audio.h.  Same two members,
// same order; changing rapt_audio.h means changing this.
struct DynamicSoundData
{
	void (*mCallback)(short *theBuffer, unsigned int theLength, void* theExtraData);
	void *mExtraData;
};

struct Obj
{
	int				mKind;

	// --- kSample: the decoded PCM, always interleaved stereo float ---
	float*			mPCM;
	int				mFrames;
	int				mDuplicates;

	// --- kVoice/kStream/kDynamic: voice state ---
	unsigned int	mOwner;		// kVoice -> its kSample handle
	bool			mPlaying;
	bool			mLooping;
	float			mVolume;
	float			mRate;		// current playback rate in Hz (the pitch knob)
	int				mBaseRate;	// native rate; what GetSoundBufferFrequency returns
	double			mPos;		// position in SOURCE frames, fractional
	int				mBus;

	// --- kStream: the compressed file, the decoder, the decode-ahead window ---
	unsigned char*	mFile;
	int				mFileLen;
	stbv::stb_vorbis* mVorbis;
	float*			mDec;		// kStreamBufFrames * 2 floats
	int				mDecStart;	// source frame index of mDec[0]
	int				mDecFill;	// frames valid in mDec
	bool			mDecEOF;

	// --- kDynamic ---
	DynamicSoundData mDyna;

	// --- level meter (BASS_ChannelGetLevel) ---
	float			mAccL,mAccR;
	int				mAccFrames;
	float			mLevelL,mLevelR;
};

static Obj			gObj[kMaxObjects];
static bool			gStarted=false;
static SDL_AudioDeviceID gDevice=0;
static int			gRate=0;			// the device's ACTUAL output rate
static int			gBlockFrames=0;		// the device's ACTUAL block size
static float*		gMix=NULL;			// gBlockFrames*2 floats
static short*		gDynScratch=NULL;	// gBlockFrames*2 shorts
static float		gSoundGain=1.0f;	// BASS_CONFIG_GVOL_SAMPLE + _GVOL_STREAM
static float		gMusicGain=1.0f;	// BASS_CONFIG_GVOL_MUSIC (module music)
static bool			gPaused=false;
static bool			gBackgroundPaused=false;
static bool			gAdPaused=false;
static bool			gAllowSoundsInBackground=false;
static int			gLevelWindowFrames=0;

// Structural changes only -- see the locking note at the top of the file.
struct AudioLock
{
	AudioLock()  {if (gDevice) SDL_LockAudioDevice(gDevice);}
	~AudioLock() {if (gDevice) SDL_UnlockAudioDevice(gDevice);}
};

static Obj* Get(unsigned int theHandle, int theKind)
{
	if (!gStarted || !theHandle || theHandle>(unsigned int)kMaxObjects) return NULL;
	Obj* aO=&gObj[theHandle-1];
	if (aO->mKind!=theKind) return NULL;
	return aO;
}

// Any handle that can be fed to the channel operations: BASS made no
// distinction between a sample's channel and a stream, and neither does RAPT
// (SoundStream::Play calls PlaySoundBuffer on a stream handle).
static Obj* GetVoice(unsigned int theHandle)
{
	if (!gStarted || !theHandle || theHandle>(unsigned int)kMaxObjects) return NULL;
	Obj* aO=&gObj[theHandle-1];
	if (aO->mKind==kVoice || aO->mKind==kStream || aO->mKind==kDynamic || aO->mKind==kMusic) return aO;
	return NULL;
}

static unsigned int Alloc(int theKind)
{
	for (int i=0;i<kMaxObjects;i++) if (gObj[i].mKind==kFree)
	{
		Obj* aO=&gObj[i];
		memset(aO,0,sizeof(Obj));
		aO->mKind=theKind;
		aO->mVolume=1.0f;
		aO->mRate=(float)gRate;
		aO->mBaseRate=gRate;
		aO->mBus=kBusSound;
		return (unsigned int)(i+1);
	}
	OS_Core::Printf("!Sound_Core: out of sound objects (%d)",kMaxObjects);
	return 0;
}

// ---------------------------------------------------------------------------
// File loading
// ---------------------------------------------------------------------------

// Reads a whole file into a malloc'd block.  The path has already been through
// _FixPath, exactly as it had before BASS opened it.
static unsigned char* ReadWholeFile(const char* theFilename, int* theLength)
{
	*theLength=0;
	FILE* aF=fopen(theFilename,"rb");
	if (!aF) return NULL;
	fseek(aF,0,SEEK_END);
	long aLen=ftell(aF);
	fseek(aF,0,SEEK_SET);
	if (aLen<=0) {fclose(aF);return NULL;}
	unsigned char* aBuf=(unsigned char*)malloc((size_t)aLen);
	if (!aBuf) {fclose(aF);return NULL;}
	size_t aGot=fread(aBuf,1,(size_t)aLen,aF);
	fclose(aF);
	if (aGot!=(size_t)aLen) {free(aBuf);return NULL;}
	*theLength=(int)aLen;
	return aBuf;
}

// A minimal PCM RIFF/WAVE reader.  RAPT probes for .wav after .ogg (see
// Sound::Load), and BASS read those natively; every asset RWK actually ships is
// .ogg, so this exists to keep a modder's .wav working, nothing more.
// 8/16/24/32-bit integer and 32-bit float PCM, any channel count.
static bool DecodeWav(const unsigned char* theData, int theLen, float** thePCM, int* theFrames, int* theRate)
{
	if (theLen<44) return false;
	if (memcmp(theData,"RIFF",4) || memcmp(theData+8,"WAVE",4)) return false;

	int aPos=12,aChannels=0,aBits=0,aFormat=0;
	*theRate=0;
	const unsigned char* aPCM=NULL;int aPCMLen=0;
	while (aPos+8<=theLen)
	{
		const unsigned char* aID=theData+aPos;
		unsigned int aSize=(unsigned int)aID[4]|((unsigned int)aID[5]<<8)|((unsigned int)aID[6]<<16)|((unsigned int)aID[7]<<24);
		const unsigned char* aBody=theData+aPos+8;
		if (aPos+8+(int)aSize>theLen) aSize=(unsigned int)(theLen-aPos-8);
		if (!memcmp(aID,"fmt ",4) && aSize>=16)
		{
			aFormat  =(int)(aBody[0]|(aBody[1]<<8));
			aChannels=(int)(aBody[2]|(aBody[3]<<8));
			*theRate =(int)((unsigned int)aBody[4]|((unsigned int)aBody[5]<<8)|((unsigned int)aBody[6]<<16)|((unsigned int)aBody[7]<<24));
			aBits    =(int)(aBody[14]|(aBody[15]<<8));
		}
		else if (!memcmp(aID,"data",4)) {aPCM=aBody;aPCMLen=(int)aSize;}
		aPos+=8+(int)aSize+((int)aSize&1);
	}
	if (!aPCM || !aChannels || !*theRate) return false;
	if (aFormat!=1 && aFormat!=3 && aFormat!=0xFFFE) return false;   // PCM, float, extensible

	int aBytes=aBits/8;
	if (aBytes<1 || aBytes>4) return false;
	int aFrames=aPCMLen/(aBytes*aChannels);
	if (aFrames<=0) return false;

	float* aOut=(float*)malloc((size_t)aFrames*2*sizeof(float));
	if (!aOut) return false;
	for (int f=0;f<aFrames;f++)
	{
		float aS[2]={0,0};
		for (int c=0;c<aChannels;c++)
		{
			const unsigned char* aP=aPCM+((size_t)f*aChannels+c)*aBytes;
			float aV=0;
			if (aBits==8)       aV=((float)aP[0]-128.0f)/128.0f;
			else if (aBits==16) aV=(float)(short)(aP[0]|(aP[1]<<8))/32768.0f;
			else if (aBits==24) {int aI=(int)(aP[0]|(aP[1]<<8)|(aP[2]<<16));if (aI&0x800000) aI|=~0xFFFFFF;aV=(float)aI/8388608.0f;}
			else if (aBits==32)
			{
				unsigned int aU=(unsigned int)aP[0]|((unsigned int)aP[1]<<8)|((unsigned int)aP[2]<<16)|((unsigned int)aP[3]<<24);
				if (aFormat==3) {float aTmp;memcpy(&aTmp,&aU,4);aV=aTmp;}
				else aV=(float)(int)aU/2147483648.0f;
			}
			if (c<2) aS[c]=aV;
			if (aChannels==1) aS[1]=aV;
		}
		aOut[f*2]=aS[0];aOut[f*2+1]=aS[1];
	}
	*thePCM=aOut;*theFrames=aFrames;
	return true;
}

// Fully decodes an .ogg to interleaved stereo float.  stb_vorbis is asked for
// 2 channels, so mono files arrive duplicated and multichannel ones downmixed;
// the mixer then never has to branch on channel count.
static bool DecodeOgg(const unsigned char* theData, int theLen, float** thePCM, int* theFrames, int* theRate)
{
	int aErr=0;
	stbv::stb_vorbis* aV=stbv::stb_vorbis_open_memory(theData,theLen,&aErr,NULL);
	if (!aV) return false;
	stbv::stb_vorbis_info aInfo=stbv::stb_vorbis_get_info(aV);
	unsigned int aTotal=stbv::stb_vorbis_stream_length_in_samples(aV);
	if (!aTotal) {stbv::stb_vorbis_close(aV);return false;}

	float* aOut=(float*)malloc((size_t)aTotal*2*sizeof(float));
	if (!aOut) {stbv::stb_vorbis_close(aV);return false;}
	int aGot=stbv::stb_vorbis_get_samples_float_interleaved(aV,2,aOut,(int)aTotal*2);
	stbv::stb_vorbis_close(aV);
	if (aGot<=0) {free(aOut);return false;}

	*thePCM=aOut;*theFrames=aGot;*theRate=(int)aInfo.sample_rate;
	return true;
}

static bool DecodeFile(const char* theFilename, float** thePCM, int* theFrames, int* theRate)
{
	int aLen=0;
	unsigned char* aData=ReadWholeFile(theFilename,&aLen);
	if (!aData) {OS_Core::Printf("!Sound_Core: cannot open [%s]",theFilename);return false;}

	bool aOK=false;
	if (aLen>4 && !memcmp(aData,"OggS",4))      aOK=DecodeOgg(aData,aLen,thePCM,theFrames,theRate);
	else if (aLen>4 && !memcmp(aData,"RIFF",4)) aOK=DecodeWav(aData,aLen,thePCM,theFrames,theRate);
	else OS_Core::Printf("!Sound_Core: [%s] is neither Ogg Vorbis nor RIFF/WAVE",theFilename);

	free(aData);
	if (!aOK) OS_Core::Printf("!Sound_Core: cannot decode [%s]",theFilename);
	return aOK;
}

// ---------------------------------------------------------------------------
// The mixer
// ---------------------------------------------------------------------------

// Pulls source frame theIndex (and theIndex+1, for the interpolation) of a
// stream into its decode window, decoding forward as needed.  Returns false at
// end of stream.  Runs on the audio thread; there is no file I/O here, the
// whole compressed file is already in memory.
static bool StreamReach(Obj* theO, int theIndex)
{
	if (theIndex<theO->mDecStart) return false;		// only SetSoundPosition goes backwards
	while (theIndex+1>=theO->mDecStart+theO->mDecFill)
	{
		if (theO->mDecEOF) return theIndex<theO->mDecStart+theO->mDecFill;

		// Drop what the voice has already passed, then decode into the tail.
		int aDrop=theIndex-theO->mDecStart;
		if (aDrop>0)
		{
			if (aDrop>theO->mDecFill) aDrop=theO->mDecFill;
			memmove(theO->mDec,theO->mDec+(size_t)aDrop*2,(size_t)(theO->mDecFill-aDrop)*2*sizeof(float));
			theO->mDecStart+=aDrop;
			theO->mDecFill -=aDrop;
		}
		int aRoom=kStreamBufFrames-theO->mDecFill;
		if (aRoom<=0) return true;
		int aGot=stbv::stb_vorbis_get_samples_float_interleaved(theO->mVorbis,2,
					theO->mDec+(size_t)theO->mDecFill*2,aRoom*2);
		if (aGot<=0) {theO->mDecEOF=true;continue;}
		theO->mDecFill+=aGot;
	}
	return true;
}

static void StreamRewind(Obj* theO, int theFrame)
{
	if (theO->mVorbis)
	{
		if (theFrame<=0) stbv::stb_vorbis_seek_start(theO->mVorbis);
		else             stbv::stb_vorbis_seek(theO->mVorbis,(unsigned int)theFrame);
	}
	theO->mDecStart=theFrame<0?0:theFrame;
	theO->mDecFill =0;
	theO->mDecEOF  =false;
}

static void RenderVoice(Obj* theO, float* theMix, int theFrames)
{
	float aBus=(theO->mBus==kBusMusic)?gMusicGain:gSoundGain;

	if (theO->mKind==kDynamic)
	{
		// The push callback owns the block: it writes theFrames stereo frames
		// of 16-bit at the DEVICE rate, and is told the length in BYTES --
		// which is what BASS's STREAMPROC passed and what BeepBox halves to
		// get its count of shorts.
		if (!theO->mDyna.mCallback) return;
		memset(gDynScratch,0,(size_t)theFrames*2*sizeof(short));
		theO->mDyna.mCallback(gDynScratch,(unsigned int)(theFrames*2*sizeof(short)),theO->mDyna.mExtraData);
		for (int f=0;f<theFrames;f++)
		{
			float aL=(float)gDynScratch[f*2  ]/32768.0f;
			float aR=(float)gDynScratch[f*2+1]/32768.0f;
			aL*=theO->mVolume;aR*=theO->mVolume;
			float aAL=aL<0?-aL:aL, aAR=aR<0?-aR:aR;
			if (aAL>theO->mAccL) theO->mAccL=aAL;
			if (aAR>theO->mAccR) theO->mAccR=aAR;
			theMix[f*2  ]+=aL*aBus;
			theMix[f*2+1]+=aR*aBus;
		}
		theO->mAccFrames+=theFrames;
		return;
	}

	double aStep=(double)theO->mRate/(double)gRate;
	if (aStep<=0) return;

	for (int f=0;f<theFrames;f++)
	{
		int   aI=(int)theO->mPos;
		float aFrac=(float)(theO->mPos-(double)aI);
		float aL=0,aR=0;

		if (theO->mKind==kStream)
		{
			if (!StreamReach(theO,aI))
			{
				if (theO->mLooping) {StreamRewind(theO,0);theO->mPos=0;aI=0;aFrac=0;
									 if (!StreamReach(theO,0)) {theO->mPlaying=false;break;}}
				else {theO->mPlaying=false;break;}
			}
			int aOff=aI-theO->mDecStart;
			if (aOff<0 || aOff>=theO->mDecFill) {theO->mPlaying=false;break;}
			int aNext=(aOff+1<theO->mDecFill)?(aOff+1):aOff;
			aL=theO->mDec[aOff*2  ]+(theO->mDec[aNext*2  ]-theO->mDec[aOff*2  ])*aFrac;
			aR=theO->mDec[aOff*2+1]+(theO->mDec[aNext*2+1]-theO->mDec[aOff*2+1])*aFrac;
		}
		else // kVoice: PCM held by the owning sample
		{
			Obj* aS=Get(theO->mOwner,kSample);
			if (!aS || !aS->mPCM || aS->mFrames<=0) {theO->mPlaying=false;break;}
			if (aI>=aS->mFrames)
			{
				if (!theO->mLooping) {theO->mPlaying=false;break;}
				theO->mPos-=(double)aS->mFrames;
				aI=(int)theO->mPos;aFrac=(float)(theO->mPos-(double)aI);
				if (aI>=aS->mFrames) {theO->mPos=0;aI=0;aFrac=0;}
			}
			int aNext=aI+1;
			if (aNext>=aS->mFrames) aNext=theO->mLooping?0:aI;
			aL=aS->mPCM[aI*2  ]+(aS->mPCM[aNext*2  ]-aS->mPCM[aI*2  ])*aFrac;
			aR=aS->mPCM[aI*2+1]+(aS->mPCM[aNext*2+1]-aS->mPCM[aI*2+1])*aFrac;
		}

		aL*=theO->mVolume;aR*=theO->mVolume;
		float aAL=aL<0?-aL:aL, aAR=aR<0?-aR:aR;
		if (aAL>theO->mAccL) theO->mAccL=aAL;
		if (aAR>theO->mAccR) theO->mAccR=aAR;

		theMix[f*2  ]+=aL*aBus;
		theMix[f*2+1]+=aR*aBus;

		theO->mPos+=aStep;
		theO->mAccFrames++;
	}
}

static void SDLCALL Mix(void* theUser, Uint8* theStream, int theLen)
{
	int aTotal=theLen/(int)(2*sizeof(short));
	short* aOut=(short*)theStream;
	bool aSilent=(!gStarted || gPaused || gBackgroundPaused || gAdPaused);

	while (aTotal>0)
	{
		int aFrames=aTotal<gBlockFrames?aTotal:gBlockFrames;

		if (aSilent) memset(aOut,0,(size_t)aFrames*2*sizeof(short));
		else
		{
			memset(gMix,0,(size_t)aFrames*2*sizeof(float));
			for (int i=0;i<kMaxObjects;i++)
			{
				Obj* aO=&gObj[i];
				if (aO->mKind!=kVoice && aO->mKind!=kStream && aO->mKind!=kDynamic) continue;
				if (!aO->mPlaying) continue;
				RenderVoice(aO,gMix,aFrames);

				if (aO->mAccFrames>=gLevelWindowFrames)
				{
					aO->mLevelL=aO->mAccL;aO->mLevelR=aO->mAccR;
					aO->mAccL=aO->mAccR=0;aO->mAccFrames=0;
				}
			}
			for (int s=0;s<aFrames*2;s++)
			{
				float aV=gMix[s]*32767.0f;
				if (aV> 32767.0f) aV= 32767.0f;
				if (aV<-32768.0f) aV=-32768.0f;
				aOut[s]=(short)aV;
			}
		}
		aOut  +=aFrames*2;
		aTotal-=aFrames;
	}
}

// ---------------------------------------------------------------------------
// Startup / shutdown
// ---------------------------------------------------------------------------

void Startup()
{
	if (gStarted) return;
	memset(gObj,0,sizeof(gObj));

	if (SDL_InitSubSystem(SDL_INIT_AUDIO)!=0)
	{
		OS_Core::Printf("!Sound_Core: SDL_INIT_AUDIO failed (%s) -- running silent",SDL_GetError());
		return;
	}

	SDL_AudioSpec aWant,aHave;
	memset(&aWant,0,sizeof(aWant));
	aWant.freq    =kRequestRate;
	aWant.format  =AUDIO_S16SYS;
	aWant.channels=2;
	aWant.samples =kRequestFrames;
	aWant.callback=Mix;

	// The rate and the block size may be whatever the device prefers, but the
	// FORMAT and CHANNEL COUNT may not: the dynamic (BeepBox) callback's
	// contract is 16-bit stereo interleaved, and every rate-dependent number
	// below is read back out of aHave rather than assumed.
	gDevice=SDL_OpenAudioDevice(NULL,0,&aWant,&aHave,
			SDL_AUDIO_ALLOW_FREQUENCY_CHANGE|SDL_AUDIO_ALLOW_SAMPLES_CHANGE);
	if (!gDevice)
	{
		OS_Core::Printf("!Sound_Core: SDL_OpenAudioDevice failed (%s) -- running silent",SDL_GetError());
		return;
	}

	gRate       =aHave.freq;
	gBlockFrames=aHave.samples;
	gLevelWindowFrames=(gRate*kLevelWindowMs)/1000;
	if (gLevelWindowFrames<1) gLevelWindowFrames=1;

	gMix       =(float*)malloc((size_t)gBlockFrames*2*sizeof(float));
	gDynScratch=(short*)malloc((size_t)gBlockFrames*2*sizeof(short));
	if (!gMix || !gDynScratch)
	{
		OS_Core::Printf("!Sound_Core: out of memory for the mix buffers");
		SDL_CloseAudioDevice(gDevice);gDevice=0;
		return;
	}

	gStarted=true;
	SDL_PauseAudioDevice(gDevice,0);
	OS_Core::Printf("Sound_Core: SDL2 + stb_vorbis, device \"%s\", %d Hz, %d-frame blocks",
		SDL_GetAudioDeviceName(0,0)?SDL_GetAudioDeviceName(0,0):"default",gRate,gBlockFrames);
}

static void FreeObj(Obj* theO)
{
	if (theO->mPCM)    {free(theO->mPCM);theO->mPCM=NULL;}
	if (theO->mVorbis) {stbv::stb_vorbis_close(theO->mVorbis);theO->mVorbis=NULL;}
	if (theO->mFile)   {free(theO->mFile);theO->mFile=NULL;}
	if (theO->mDec)    {free(theO->mDec);theO->mDec=NULL;}
	memset(theO,0,sizeof(Obj));
	theO->mKind=kFree;
}

void Shutdown()
{
	if (!gStarted) return;
	gStarted=false;
	SDL_PauseAudioDevice(gDevice,1);
	SDL_CloseAudioDevice(gDevice);
	gDevice=0;
	for (int i=0;i<kMaxObjects;i++) FreeObj(&gObj[i]);
	if (gMix) {free(gMix);gMix=NULL;}
	if (gDynScratch) {free(gDynScratch);gDynScratch=NULL;}
}

// ---------------------------------------------------------------------------
// Buses, pause, multitasking
// ---------------------------------------------------------------------------

void SetGlobalMusicVolume(float theVolume)
{
	// BASS_CONFIG_GVOL_MUSIC only ever scaled MOD/tracker channels.  With the
	// module path dropped nothing rides this bus, and that is exactly the
	// behaviour BASS gave RWK, which loaded no modules either: the game's
	// music volume reaches its .ogg streams through SoundStream::SetVolume
	// (rapt_audio's SUPPORT_MUSIC_SAMPLES path).
	if (!gStarted) return;
	gMusicGain=theVolume;
}

void SetGlobalSoundVolume(float theVolume)
{
	if (!gStarted) return;
	gSoundGain=theVolume;		// BASS set GVOL_SAMPLE and GVOL_STREAM together
}

void Multitasking(bool isForeground)
{
	if (!gStarted) return;
	if (gAllowSoundsInBackground) return;
	gBackgroundPaused=!isForeground;
}

void AllowSoundsInBackground(bool theState)
{
	gAllowSoundsInBackground=theState;
	if (theState) gBackgroundPaused=false;
}

void Pause(bool theState)
{
	if (!gStarted) return;
	gPaused=theState;
}

void AdPause(bool theState)
{
	gAdPaused=theState;
}

// ---------------------------------------------------------------------------
// Samples
// ---------------------------------------------------------------------------

unsigned int LoadSound(char *theFilename, int theDuplicates)
{
	if (!gStarted) return 0;
	_FixPath(theFilename);

	float* aPCM=NULL;int aFrames=0,aRate=0;
	if (!DecodeFile(theFilename,&aPCM,&aFrames,&aRate)) return 0;

	AudioLock aLock;
	unsigned int aH=Alloc(kSample);
	if (!aH) {free(aPCM);return 0;}
	Obj* aO=&gObj[aH-1];
	aO->mPCM       =aPCM;
	aO->mFrames    =aFrames;
	aO->mBaseRate  =aRate;
	aO->mRate      =(float)aRate;
	aO->mDuplicates=theDuplicates<1?1:theDuplicates;
	return aH;
}

unsigned int GetSoundBuffer(unsigned int theSoundID)
{
	if (!gStarted) return 0;
	AudioLock aLock;
	Obj* aS=Get(theSoundID,kSample);
	if (!aS) return 0;

	unsigned int aH=Alloc(kVoice);
	if (!aH) return 0;
	Obj* aO=&gObj[aH-1];
	aO->mOwner   =theSoundID;
	aO->mBaseRate=aS->mBaseRate;
	aO->mRate    =(float)aS->mBaseRate;
	aO->mPlaying =false;
	return aH;
}

void UnloadSound(unsigned int theSoundID)
{
	if (!gStarted) return;
	AudioLock aLock;
	Obj* aS=Get(theSoundID,kSample);
	if (!aS) return;
	// BASS_SampleFree took the sample's channels with it.
	for (int i=0;i<kMaxObjects;i++)
		if (gObj[i].mKind==kVoice && gObj[i].mOwner==theSoundID) FreeObj(&gObj[i]);
	FreeObj(aS);
}

// ---------------------------------------------------------------------------
// Channel operations -- these take NO lock (see the note at the top)
// ---------------------------------------------------------------------------

void PlaySoundBuffer(unsigned int theBufferID)
{
	Obj* aO=GetVoice(theBufferID);
	if (!aO) return;
	// BASS_ChannelPlay(handle,TRUE): restart from the beginning.
	if (aO->mKind==kStream && aO->mPos!=0) {AudioLock aLock;StreamRewind(aO,0);}
	aO->mPos=0;
	aO->mAccL=aO->mAccR=0;aO->mAccFrames=0;
	aO->mPlaying=true;			// written last: the mixer must never see a
								// live voice with a stale position
}

void StopSoundBuffer(unsigned int theBufferID)
{
	Obj* aO=GetVoice(theBufferID);
	if (!aO) return;
	aO->mPlaying=false;			// BASS_ChannelPause: position is kept
}

void SetSoundBufferVolume(unsigned int theBufferID, float theVolume)
{
	Obj* aO=GetVoice(theBufferID);
	if (!aO) return;
	aO->mVolume=theVolume;
}

void SetSoundBufferFrequency(unsigned int theBufferID, float theFrequency)
{
	Obj* aO=GetVoice(theBufferID);
	if (!aO) return;
	if (theFrequency<=0) return;
	aO->mRate=theFrequency;		// BASS_ATTRIB_FREQ: resample, not time-stretch
}

float GetSoundBufferFrequency(unsigned int theBufferID)
{
	Obj* aO=GetVoice(theBufferID);
	if (!aO) return 0;
	return (float)aO->mBaseRate;	// BASS_CHANNELINFO.freq: the DEFAULT rate,
									// which is what RAPT multiplies for pitch
}

bool IsSoundBufferPlaying(unsigned int theBufferID)
{
	Obj* aO=GetVoice(theBufferID);
	if (!aO) return false;
	return aO->mPlaying;
}

void SetSoundBufferLooping(unsigned int theBufferID, bool theState)
{
	Obj* aO=GetVoice(theBufferID);
	if (!aO) return;
	aO->mLooping=theState;
}

// ---------------------------------------------------------------------------
// Streams
// ---------------------------------------------------------------------------

unsigned int LoadStreamSound(char *theFilename)
{
	if (!gStarted) return 0;
	_FixPath(theFilename);

	int aLen=0;
	unsigned char* aData=ReadWholeFile(theFilename,&aLen);
	if (!aData) {OS_Core::Printf("!Sound_Core: cannot open stream [%s]",theFilename);return 0;}

	// Only the COMPRESSED file is held; the PCM is decoded a window at a time
	// by the mixer.  (The WASM/OpenAL backend this replaces decoded the whole
	// thing up front and called it streaming.)
	int aErr=0;
	stbv::stb_vorbis* aV=NULL;
	if (aLen>4 && !memcmp(aData,"OggS",4)) aV=stbv::stb_vorbis_open_memory(aData,aLen,&aErr,NULL);
	if (!aV)
	{
		// Not Ogg (or unreadable): fall back to a fully decoded sample voice so
		// a .wav stream still plays.
		free(aData);
		unsigned int aSample=LoadSound(theFilename,1);
		if (!aSample) return 0;
		unsigned int aVoice=GetSoundBuffer(aSample);
		if (!aVoice) {UnloadSound(aSample);return 0;}
		return aVoice;
	}

	stbv::stb_vorbis_info aInfo=stbv::stb_vorbis_get_info(aV);
	unsigned int aTotal=stbv::stb_vorbis_stream_length_in_samples(aV);

	AudioLock aLock;
	unsigned int aH=Alloc(kStream);
	if (!aH) {stbv::stb_vorbis_close(aV);free(aData);return 0;}
	Obj* aO=&gObj[aH-1];
	aO->mFile    =aData;
	aO->mFileLen =aLen;
	aO->mVorbis  =aV;
	aO->mDec     =(float*)malloc((size_t)kStreamBufFrames*2*sizeof(float));
	aO->mFrames  =(int)aTotal;
	aO->mBaseRate=(int)aInfo.sample_rate;
	aO->mRate    =(float)aInfo.sample_rate;
	aO->mDecStart=0;aO->mDecFill=0;aO->mDecEOF=false;
	if (!aO->mDec) {FreeObj(aO);return 0;}
	return aH;
}

void UnloadStreamSound(unsigned int theSoundID)
{
	if (!gStarted) return;
	AudioLock aLock;
	// SoundStream's destructor calls this for file streams AND for the dynamic
	// (BeepBox) stream, and LoadStreamSound can hand back a sample voice, so
	// accept any of them.
	Obj* aO=GetVoice(theSoundID);
	if (!aO) return;
	if (aO->mKind==kVoice)
	{
		unsigned int aOwner=aO->mOwner;
		FreeObj(aO);
		Obj* aS=Get(aOwner,kSample);
		if (aS)
		{
			bool aUsed=false;
			for (int i=0;i<kMaxObjects;i++) if (gObj[i].mKind==kVoice && gObj[i].mOwner==aOwner) aUsed=true;
			if (!aUsed) FreeObj(aS);
		}
		return;
	}
	FreeObj(aO);
}

void GetStreamLevel(unsigned int theSoundID, float* theLeft, float* theRight)
{
	*theLeft=0;*theRight=0;
	Obj* aO=GetVoice(theSoundID);
	if (!aO || !aO->mPlaying) return;
	// BASS_ChannelGetLevel: the peak of the last ~20 ms, after the channel's
	// own volume and before the global buses.  The BASS backend divided the
	// two 16-bit halves by 32768; these are already normalised.
	*theLeft =aO->mLevelL;
	*theRight=aO->mLevelR;
}

// ---------------------------------------------------------------------------
// Position
// ---------------------------------------------------------------------------

void SetSoundPosition(unsigned int theHandle, float thePos)
{
	Obj* aO=GetVoice(theHandle);
	if (!aO) return;
	int aFrame=(int)(thePos*(float)aO->mBaseRate);
	if (aFrame<0) aFrame=0;
	if (aO->mKind==kStream)
	{
		AudioLock aLock;			// touches the decoder: structural
		StreamRewind(aO,aFrame);
		aO->mPos=aFrame;
		return;
	}
	aO->mPos=aFrame;
}

float GetSoundPosition(unsigned int theHandle)
{
	Obj* aO=GetVoice(theHandle);
	if (!aO || !aO->mBaseRate) return 0;
	return (float)(aO->mPos/(double)aO->mBaseRate);
}

// ---------------------------------------------------------------------------
// Device rates
// ---------------------------------------------------------------------------

void GetSampleRate(unsigned int& theMin, unsigned int& theMax)
{
	// BASS_INFO.minrate/maxrate.  SDL2 does not expose a device's rate range,
	// and the mixer resamples anything, so report the one rate we actually
	// output.  (Nothing in RAPT or RWK reads these: Audio::GetMinimumSampleRate
	// and GetMaximumSampleRate have no callers.)
	theMin=(unsigned int)gRate;
	theMax=(unsigned int)gRate;
}

unsigned int GetFrequency()
{
	return (unsigned int)gRate;		// BASS_INFO.freq: the DEVICE's output rate
}

// ---------------------------------------------------------------------------
// The push-callback (dynamic) stream -- BeepBox
// ---------------------------------------------------------------------------

unsigned int CreateDynamicSound(void* theDataPacket, int theBufferLength)
{
	if (!gStarted || !theDataPacket) return 0;
	AudioLock aLock;
	unsigned int aH=Alloc(kDynamic);
	if (!aH) return 0;
	Obj* aO=&gObj[aH-1];
	aO->mDyna    =*(DynamicSoundData*)theDataPacket;
	aO->mBaseRate=gRate;			// BASS_StreamCreate(aInfo.freq,2,...)
	aO->mRate    =(float)gRate;	// so the callback's output is never resampled
	// theBufferLength was BASS_CONFIG_BUFFER, a playback-buffer length in ms.
	// SDL's buffer is fixed at open time, so it is advisory here.
	(void)theBufferLength;
	return aH;
}

// ---------------------------------------------------------------------------
// Tracker-module music -- DROPPED (ruling, 2026-08-28)
// ---------------------------------------------------------------------------
// RWK reaches none of this: all 61 of its assets are .ogg and its campaign
// music is played through SoundStream, not Music.  Dropping it is what takes
// libopenmpt (openmpt.o) out of the wasm link.  These stay as stubs so
// rapt_audio.cpp -- which is untouched by this change -- still links.

static bool gWarnedMusic=false;
static void WarnMusic(const char* theWhat)
{
	if (gWarnedMusic) return;
	gWarnedMusic=true;
	OS_Core::Printf("Sound_Core: tracker-module music is not supported in this build (%s); "
					"use an .ogg through SoundStream",theWhat);
}

unsigned int LoadMusic(char *theFilename)                  {WarnMusic(theFilename);return 0;}
void UnloadMusic(unsigned int theHandle)                   {}
void PlayMusic(unsigned int theHandle, unsigned int theOffset) {WarnMusic("PlayMusic");}
void StopMusic(unsigned int theHandle)                     {}
void SetMusicVolume(unsigned int theHandle, float theVolume) {}
void SetMusicTrackVolume(unsigned int theHandle, unsigned int theTrack, float theVolume) {}
void PauseMusic(int theHandle, bool thePause)              {}

} // namespace Sound_Core
