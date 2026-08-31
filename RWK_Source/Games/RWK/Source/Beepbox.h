#pragma once
#include "rapt.h"


enum {EnvelopeType_custom,EnvelopeType_steady,EnvelopeType_punch,EnvelopeType_flare,EnvelopeType_pluck,EnvelopeType_tremolo};
enum {InstrumentType_unknown=-1,InstrumentType_chip=0,InstrumentType_fm=1,InstrumentType_noise=2};

class BeepBoxConfig
{
public:
	BeepBoxConfig();
	Array<double>&		CenterWave(Array<double>& theArray);
	Array<double>&		GenerateSineWave();
	ReadOnlyArray<double>&		GetDrumWave(int theIndex);
	void				DrawNoiseSpectrum(Array<double>& theWave, int lowOctave, int highOctave, int lowPower, int highPower, int overalSlope);
	void				InverseRealFourierTransform(Array<double>& theArray);
	void				ReverseIndexBits(Array<double>& theArray);
	void				ScaleElementsByFactor(Array<double>& theArray, float theFactor);

public:
	ReadOnlyArray<float> mKeyTransposes;
	ReadOnlyArray<int> mPartCounts;
	ReadOnlyArray<float> mWaveVolumes;
	ReadOnlyArray<float> mDrumVolumes;
	ReadOnlyArray<float> mDrumBasePitches;
	ReadOnlyArray<float> mDrumPitchFilterMult;
	ReadOnlyArray<bool> mDrumWaveIsSoft;
	ReadOnlyArray<float> mFilterBases;
	ReadOnlyArray<float> mFilterDecays;
	ReadOnlyArray<float> mFilterVolumes;
	ReadOnlyArray<float> mEffectVibratos;
	ReadOnlyArray<float> mEffectTremolos;
	ReadOnlyArray<float> mEffectVibratoDelays;
	ReadOnlyArray<float> mChorusIntervals;
	ReadOnlyArray<float> mChorusOffsets;
	ReadOnlyArray<float> mChorusVolumes;
	ReadOnlyArray<float> mChorusSigns;
	ReadOnlyArray<bool> mChorusHarmonizes;
	ReadOnlyArray<float> mVolumeValues;
	ReadOnlyArray<int> mOperatorEnvelopeType;
	ReadOnlyArray<ReadOnlyArray<ReadOnlyArray<int> > > mOperatorModulatedBy;
	ReadOnlyArray<ReadOnlyArray<int> > mOperatorAssociatedCarrier;
	ReadOnlyArray<int> mOperatorCarrierCounts;
	ReadOnlyArray<float> mOperatorCarrierChorus;
	ReadOnlyArray<float> mOperatorFrequencies;
	ReadOnlyArray<float> mOperatorHzOffsets;
	ReadOnlyArray<float> mOperatorAmplitudeSigns;
	ReadOnlyArray<float> mOperatorEnvelopeSpeed;
	ReadOnlyArray<bool> mOperatorEnvelopeInverted;
	ReadOnlyArray<ReadOnlyArray<ReadOnlyArray<int> > > mOperatorFeedbackIndices;
	ReadOnlyArray<ReadOnlyArray<double> > mWaves;
	ReadOnlyArray<double> mDrumWaves[5];

	static const int mTempoSteps=15;
	static const int mReverbRange=4;
	static const int mBeatsPerBarMin=3;
	static const int mBeatsPerBarMax=16;
	static const int mBarCountMin=1;
	static const int mBarCountMax=128;
	static const int mPatternsPerChannelMin=1;
	static const int mPatternsPerChannelMax=64;
	static const int mInstrumentsPerChannelMin=1;
	static const int mInstrumentsPerChannelMax=10;
	static const int mWaveNames_Length=9;
	static const int mDrumNames_Length=5;
	static const int mFilterNames_Length=7;
	static const int mTransitionNames_Length=4;
	static const int mEffectNames_Length=6;
	static const int mChorusNames_Length=9;
	static const int mVolumeNames_Length=6;
	static const int mOperatorCount=4;
	static const int mChannelCount=10;
	static const int mOperatorAlgorithmNames_Length=13;
	static const int mOperatorAmplitudeMax=15;
	static const int mOperatorFrequencyNames_Length=15;
	static const int mOperatorEnvelopeNames_Length=15;
	static const int mOperatorFeedbackNames_Length=19;
	static const int mKeyNames_Length=12;
	static const int mDrumInterval=6;
	static const int mDrumCount=12;
	static const int mPitchCount=37;
	static const int mMaxPitch=84;
	static const int mPitchChannelCountMin=1;
	static const int mPitchChannelCountMax=6;
	static const int mDrumChannelCountMin=0;
	static const int mDrumChannelCountMax=2;
	static const int mNegativePhaseGuard=1000;

	int mSineWaveLength;
	ReadOnlyArray<double> mSineWave;
};

class BeepBoxSong
{
public:
	bool Load(String theCompressed);

public:

	struct NotePin
	{
		int		mInterval;	// Possible int?
		int		mTime;		// Possible int?
		float	mVolume;	// Possible int, probably not
	};

	NotePin* MakeNotePin(int theInterval, int theTime, float theVolume) {NotePin* aNotePin=new NotePin;aNotePin->mInterval=theInterval;aNotePin->mTime=theTime;aNotePin->mVolume=theVolume;return aNotePin;}

	struct Note 
	{
		ReadOnlyArray<float>mPitches;
		SmartList(NotePin)	mPins;
		int					mStart;	// Possible int?
		int					mEnd;	// Possible int?
	};

	Note* MakeNote(float thePitch, int theStart, int theEnd, float theVolume, bool isFadeOut=false)
	{
		Note* aNote=new Note;
		aNote->mPitches+=thePitch;
		aNote->mPins+=MakeNotePin(0,0,theVolume);
		aNote->mPins+=MakeNotePin(0,theEnd-theStart,isFadeOut?0:theVolume);
		aNote->mStart=theStart;
		aNote->mEnd=theEnd;
		return aNote;
	}

	struct Pattern 
	{
		Pattern()
		{
			mInstrument=0;
		}
		SmartList(Note) mNotes;
		int mInstrument;
		//SmartList(Note)& CloneNotes() // Looks like this function is never used?
		
		void Reset()
		{
			mNotes.Clear();
			mInstrument=0;
		}
	};

	struct Operator 
	{
		Operator(int theIndex) {Reset(theIndex);}
		void Reset(int theIndex) {mFrequency=0;mAmplitude=(theIndex<=1) ? BeepBoxConfig::mOperatorAmplitudeMax : 0;mEnvelope=(theIndex==0) ? 0:1;}
		void Copy(Operator* theOther) {mFrequency = theOther->mFrequency;mAmplitude = theOther->mAmplitude;mEnvelope = theOther->mEnvelope;}

		int mFrequency;
		int mAmplitude;
		int mEnvelope;
	};

	struct Instrument 
	{
		Instrument() 
		{
			mType=0;
			mWave=1;
			mFilter=1;
			mTransition=1;
			mEffect=0;
			mChorus=0;
			mVolume=0;
			mAlgorithm=0;
			mFeedbackType=0;
			mFeedbackEnvelope=0;
			mFeedbackAmplitude=0;

			for (int aCount=0;aCount<BeepBoxConfig::mOperatorCount;aCount++)
			{
				mOperators+=new Operator(aCount);
			}
		}
		int mType;
		int mWave;
		int	mFilter;
		int mTransition;
		int mEffect;
		int mChorus;
		int mVolume;
		int mAlgorithm;
		int mFeedbackType;
		int mFeedbackAmplitude;
		int mFeedbackEnvelope;
		SmartList(Operator) mOperators;

		void Reset()
		{
			mType = 0;
			mWave = 1;
			mFilter = 1;
			mTransition = 1;
			mEffect = 0;
			mChorus = 0;
			mVolume = 0;
			mAlgorithm = 0;
			mFeedbackType = 0;
			mFeedbackAmplitude = 0;
			mFeedbackEnvelope = 1;
			int aCounter=0;
			EnumSmartList(Operator,aO,mOperators) aO->Reset(aCounter++);
		}

		void SetTypeAndReset(int theType)
		{
			mType=theType;
			switch (mType) 
			{
			case InstrumentType_chip:mWave = 1;mFilter = 1;mTransition = 1;mEffect = 0;mChorus = 0;mVolume = 0;break;
			case InstrumentType_fm:mWave = 1;mTransition = 1;mVolume = 0;break;
			case InstrumentType_noise:mTransition = 1;mEffect = 0;mAlgorithm = 0;mFeedbackType = 0;mFeedbackAmplitude = 0;mFeedbackEnvelope = 1;int aCounter=0;EnumSmartList(Operator,aO,mOperators) aO->Reset(aCounter++);break;
			}
		}

		void Copy(Instrument* theOther)
		{
			mType = theOther->mType;
			mWave = theOther->mWave;
			mFilter = theOther->mFilter;
			mTransition = theOther->mTransition;
			mEffect = theOther->mEffect;
			mChorus = theOther->mChorus;
			mVolume = theOther->mVolume;
			mAlgorithm = theOther->mAlgorithm;
			mFeedbackType = theOther->mFeedbackType;
			mFeedbackAmplitude = theOther->mFeedbackAmplitude;
			mFeedbackEnvelope = theOther->mFeedbackEnvelope;

			int aPtr=0;EnumSmartList(Operator,aO,mOperators) aO->Copy(theOther->mOperators[aPtr++]);
		}
	};

	struct Channel 
	{
		int mOctave;	// Possibly needs to be float?
		SmartList(Instrument) mInstruments;
		SmartList(Pattern) mPatterns;
		ReadOnlyArray<int> mBars;
	};

	int			mScale;
	int			mKey;
	int			mTempo;
	int			mReverb;
	int			mBeatsPerBar;
	int			mBarCount;
	int			mPatternsPerChannel;
	int			mPartsPerBeat;
	int			mInstrumentsPerChannel;
	int			mLoopStart;
	int			mLoopLength;
	int			mPitchChannelCount;
	int			mDrumChannelCount;
	SmartList(Channel) mChannels;

	inline int GetChannelCount() {return (int)(mPitchChannelCount+mDrumChannelCount);}
	inline int Clip(int theMin, int theMax, int theValue) {theMax=theMax-1;if (theValue<=theMax) {if (theValue>=theMin) return theValue;else return theMin;} else {return theMax;}}
	inline bool GetChannelIsDrum(int theChannel) {return (theChannel>=mPitchChannelCount);}

	void InitToDefault(bool andResetChannels=true);

	Smart(Pattern) GetPattern(int theChannel, int theBar)
	{
		int aPatternIndex=mChannels[theChannel]->mBars[theBar];
		if (aPatternIndex==0) return NULL;
		return mChannels[theChannel]->mPatterns[aPatternIndex-1];
	}

	int GetPatternInstrument(int theChannel, int theBar)
	{
		Smart(Pattern) aPattern=GetPattern(theChannel,theBar);
		if (aPattern.IsNull()) return 0;
		return aPattern->mInstrument;
	}


};


class BeepBoxChannel 
{
public:
	float				mSample;//=0.0;

	float				mPhases[BeepBoxConfig::mOperatorCount];
	float				mPhaseDeltas[BeepBoxConfig::mOperatorCount];
	float				mVolumeStarts[BeepBoxConfig::mOperatorCount];
	float				mVolumeDeltas[BeepBoxConfig::mOperatorCount];

	/*
	Array<float>		mPhases;
	Array<float>		mPhaseDeltas;
	Array<float>		mVolumeStarts;
	Array<float>		mVolumeDeltas;
	*/

	float				mPhaseDeltaScale;//=0.0;
	float				mFilter;//=0.0;
	float				mFilterScale;//=0.0
	float				mVibratoScale;//=0.0f;
	float				mHarmonyMult;//=0.0f;
	float				mHarmonyVolumeMult;//=1.0f;
	float				mFeedbackOutputs[BeepBoxConfig::mOperatorCount];
	float				mFeedbackMult;//=0.0f;
	float				mFeedbackDelta;//=0.0f;

	BeepBoxChannel() {mSample=mPhaseDeltaScale=mFilter=mFilterScale=mVibratoScale=mHarmonyMult=mFeedbackMult=mFeedbackDelta=0.0f;mHarmonyVolumeMult=1.0f;Reset();}
	void Reset() {for (int i=0;i<BeepBoxConfig::mOperatorCount;i++) {mPhases[i]=0.0f;mFeedbackOutputs[i]=0.0f;}mSample=0.0f;}
};


class BeepBox : public SoundStreamDynamic
{
public:
	BeepBox();
	virtual ~BeepBox();

	void					Stop() {SoundStreamDynamic::Stop();Construct();}
	void					Load(String theSong);
	inline bool				IsLoaded() {return mSong.IsNotNull();}
	void					Unload();

	void					PrepareToPlay();
	void					WarmUpSynthesizer(Smart(BeepBoxSong) theSong);
	void					Synthesize(short* theBuffer, unsigned int theBufferLength);
	int 					SynthesizeCore(short* theBuffer, int theBufferLength, int theBufferIndex, int aSamplesPerArpeggio);
	float					VolumeConversion(float theNoteVolume) {return (float)pow(theNoteVolume / 3.0f, 1.5f);}
	float					FrequencyFromPitch(float thePitch) {return 440.0f * (float)pow(2.0f, (thePitch - 69.0f) / 12.0f);}
	float					OperatorAmplitudeCurve(float theAmplitude) {return ((float)pow(16.0f, theAmplitude / 15.0f) - 1.0f) / 15.0f;}
	float					ComputeOperatorEnvelope(int theEnvelope, float theTime, float theBeats, float theCustomVolume);
	int						GetSamplesPerArpeggio();
	void					ComputeChannelInstrument(int theChannel, int theTime, float theSampleTime, int theSamplesPerArpeggio, int theSamples);

public:
	bool					mDidConstruct;
	void					Construct()
	{
		mDidConstruct=true;
		mPaused=true;
		mEffectDuration=.14f;

		//
		// There's some funny difference between the javascript version and this.  I'm having to multiply the
		// frequency by two, otherwise the song plays at double speed.
		//
		mSamplesPerSecond=gAudio.GetFrequency()*2;

		mEffectAngle=gMath.mPI*2.0f/(mEffectDuration*mSamplesPerSecond);
		mEffectYMult=2.0f*(float)cos(mEffectAngle);
		mLimitDecay=1.0f/(2.0f*mSamplesPerSecond);

		mPianoPressed=false;
		mPianoPitch+=0;
		mPianoChannel=0;
		mEnableIntro=true;
		mEnableOutro=false;
		mLoopCount=-1;
		mVolume=1.0f;

		mPlayheadInternal=0.0f;
		mBar=0;
		mBeat=0;
		mPart=0;
		mArpeggio=0;
		mArpeggioSampleCountdown=0;
		mStillGoin=false;
		mEffectPhase=0.0f;
		mLimit=0.0f;
		mDelayLine.GuaranteeSize(16384);
		for (int aCount=0;aCount<mDelayLine.Size();aCount++) mDelayLine[aCount]=0;
		mDelayPos=0;
		mDelayFeedback0=0.0f;
		mDelayFeedback1=0.0f;
		mDelayFeedback2=0.0f;
		mDelayFeedback3=0.0f;
	}

	bool					mPaused;
	Smart(BeepBoxSong)		mSong;
	float					mSamplesPerSecond;
	float					mEffectDuration;
	float					mEffectAngle;
	float					mEffectYMult;
	float					mLimitDecay;

	bool					mPianoPressed;
	ReadOnlyArray<float>	mPianoPitch;
	int						mPianoChannel;
	bool					mEnableIntro;
	bool					mEnableOutro;
	int						mLoopCount;
	float					mVolume;

	float					mPlayheadInternal;
	int						mBar;
	int						mBeat;
	int						mPart;
	int						mArpeggio;
	int						mArpeggioSampleCountdown;

	SmartList(BeepBoxChannel)	mChannels;
	bool						mStillGoin;
	float						mEffectPhase;
	float						mLimit;

	ReadOnlyArray<float>		mDelayLine;
	float						mDelayPos;
	float						mDelayFeedback0;
	float						mDelayFeedback1;
	float						mDelayFeedback2;
	float						mDelayFeedback3;

	//char						mInstrumentType[BeepBoxConfig::mChannelCount][BeepBoxConfig::mBarCountMax];
	//BeepBoxSong::Instrument*	mInstruments[BeepBoxConfig::mChannelCount][BeepBoxConfig::mInstrumentsPerChannelMax];
	ReadOnlyArray<Array<char> >								mInstrumentType;
	ReadOnlyArray<Array<Smart(BeepBoxSong::Instrument)> >	mInstruments;



public:


};

