#include "Beepbox.h"
#define number float

BeepBoxConfig* gBeepboxConfig=NULL;

#ifdef _TRACE
Tracer gTrace;
#endif
int gSynthesizerThreadLock=-1;

bool gBadSong=false;
int gBase64CharCodeToInt[]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,62,62,0,0,1,2,3,4,5,6,7,8,9,0,0,0,0,0,0,0,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,0,0,0,0,63,0,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,0,0,0,0,0};
int Base64CharCodeToInt(char theValue)
{
	if (theValue<0 || theValue>=128) {gBadSong=true;return 0;}
	return gBase64CharCodeToInt[theValue];
}

class BitFieldReader 
{
public:
	Array<char>		_bits;//private _bits: number[] = [];
	int				_readIndex;//private _readIndex: number = 0;

	BitFieldReader(String theString, int theStartIndex, int theStopIndex)
	{
		int aPutPos=0;

		_readIndex=0;
		_bits.GuaranteeSize((theStopIndex-theStartIndex)*6);
		for (int aCount=theStartIndex;aCount<theStopIndex;aCount++)
		{
			int aValue=Base64CharCodeToInt(theString[aCount]);
			_bits[aPutPos++]=(aValue>>5)&0x1;
			_bits[aPutPos++]=(aValue>>4)&0x1;
			_bits[aPutPos++]=(aValue>>3)&0x1;
			_bits[aPutPos++]=(aValue>>2)&0x1;
			_bits[aPutPos++]=(aValue>>1)&0x1;
			_bits[aPutPos++]=(aValue&0x1);
		}
	}

	int Read(int theBitCount)
	{
		int aResult=0;
		while (theBitCount>0) 
		{
			aResult=aResult<<1;
			aResult+=_bits[_readIndex++];
			theBitCount--;
		}
		return aResult;
	}

	int ReadLongTail(int theMinValue,int theMinBits)
	{
		int aResult=theMinValue;
		int aNumBits=theMinBits;
		while (_bits[_readIndex++]) 
		{
			aResult+=1<<aNumBits;
			aNumBits++;
		}
		while (aNumBits > 0) 
		{
			aNumBits--;
			if (_bits[_readIndex++]) 
			{
				aResult+=1<<aNumBits;
			}
		}
		return aResult;
	}

	int ReadPartDuration() {return ReadLongTail(1,2);}
	int ReadPinCount() {return ReadLongTail(1,0);}
	int ReadPitchInterval() {if (Read(1)) return -ReadLongTail(1,3);else return ReadLongTail(1,3);}
};

BeepBoxConfig::BeepBoxConfig()
{
#ifdef _TRACE
	gTrace.mFilename=PointAtDesktop("trace.txt");
#endif

	mOperatorEnvelopeType.GuaranteeSize(15);mOperatorEnvelopeType[0]=EnvelopeType_custom;mOperatorEnvelopeType[1]=EnvelopeType_steady;mOperatorEnvelopeType[2]=EnvelopeType_punch;mOperatorEnvelopeType[3]=EnvelopeType_flare;mOperatorEnvelopeType[4]=EnvelopeType_flare;mOperatorEnvelopeType[5]=EnvelopeType_flare;mOperatorEnvelopeType[6]=EnvelopeType_pluck;mOperatorEnvelopeType[7]=EnvelopeType_pluck;mOperatorEnvelopeType[8]=EnvelopeType_pluck;mOperatorEnvelopeType[9]=EnvelopeType_pluck;mOperatorEnvelopeType[10]=EnvelopeType_pluck;mOperatorEnvelopeType[11]=EnvelopeType_pluck;mOperatorEnvelopeType[12]=EnvelopeType_tremolo;mOperatorEnvelopeType[13]=EnvelopeType_tremolo;mOperatorEnvelopeType[14]=EnvelopeType_tremolo;
	mKeyTransposes.GuaranteeSize(12,true);mKeyTransposes[0]=23;mKeyTransposes[1]=22;mKeyTransposes[2]=21;mKeyTransposes[3]=20;mKeyTransposes[4]=19;mKeyTransposes[5]=18;mKeyTransposes[6]=17;mKeyTransposes[7]=16;mKeyTransposes[8]=15;mKeyTransposes[9]=14;mKeyTransposes[10]=13;mKeyTransposes[11]=12;
	mPartCounts.GuaranteeSize(4,true);mPartCounts[0]=3;mPartCounts[1]=4;mPartCounts[2]=6;mPartCounts[3]=8;
	mWaveVolumes.GuaranteeSize(9,true);mWaveVolumes[0]=1.0f;mWaveVolumes[1]=0.5f;mWaveVolumes[2]=0.5f;mWaveVolumes[3]=0.5f;mWaveVolumes[4]=0.65f;mWaveVolumes[5]=0.5f;mWaveVolumes[6]=0.4f;mWaveVolumes[7]=0.4f;mWaveVolumes[8]=0.94f;
	mDrumVolumes.GuaranteeSize(5,true);mDrumVolumes[0]=0.25f;mDrumVolumes[1]=1.0f;mDrumVolumes[2]=0.4f;mDrumVolumes[3]=0.3f;mDrumVolumes[4]=1.5f;
	mDrumBasePitches.GuaranteeSize(5,true);mDrumBasePitches[0]=69;mDrumBasePitches[1]=69;mDrumBasePitches[2]=69;mDrumBasePitches[3]=69;mDrumBasePitches[4]=96;
	mDrumPitchFilterMult.GuaranteeSize(5,true);mDrumPitchFilterMult[0]=100.0f;mDrumPitchFilterMult[1]=8.0f;mDrumPitchFilterMult[2]=100.0f;mDrumPitchFilterMult[3]=100.0f;mDrumPitchFilterMult[4]=1.0f;
	mDrumWaveIsSoft.GuaranteeSize(5,true);mDrumWaveIsSoft[0]=false;mDrumWaveIsSoft[1]=true;mDrumWaveIsSoft[2]=false;mDrumWaveIsSoft[3]=false;mDrumWaveIsSoft[4]=true;
	mFilterBases.GuaranteeSize(7,true);mFilterBases[0]=0.0f;mFilterBases[1]=2.0f;mFilterBases[2]=3.5f;mFilterBases[3]=5.0f;mFilterBases[4]=1.0f;mFilterBases[5]=2.5f;mFilterBases[6]=4.0f;
	mFilterDecays.GuaranteeSize(7,true);mFilterDecays[0]=0.0f;mFilterDecays[1]=0.0f;mFilterDecays[2]=0.0f;mFilterDecays[3]=0.0f;mFilterDecays[4]=10.0f;mFilterDecays[5]=7.0f;mFilterDecays[6]=4.0f;
	mFilterVolumes.GuaranteeSize(7,true);mFilterVolumes[0]=0.2f;mFilterVolumes[1]=0.4f;mFilterVolumes[2]=0.7f;mFilterVolumes[3]=1.0f;mFilterVolumes[4]=0.5f;mFilterVolumes[5]=0.75f;mFilterVolumes[6]=1.0f;
	mEffectVibratos.GuaranteeSize(6,true);mEffectVibratos[0]=0.0f;mEffectVibratos[1]=0.15f;mEffectVibratos[2]=0.3f;mEffectVibratos[3]=0.45f;mEffectVibratos[4]=0.0f;mEffectVibratos[5]=0.0f;
	mEffectTremolos.GuaranteeSize(6,true);mEffectTremolos[0]=0.0f;mEffectTremolos[1]=0.0f;mEffectTremolos[2]=0.0f;mEffectTremolos[3]=0.0f;mEffectTremolos[4]=0.25f;mEffectTremolos[5]=0.5f;
	mEffectVibratoDelays.GuaranteeSize(6,true);mEffectVibratoDelays[0]=0;mEffectVibratoDelays[1]=0;mEffectVibratoDelays[2]=3;mEffectVibratoDelays[3]=0;mEffectVibratoDelays[4]=0;mEffectVibratoDelays[5]=0;
	mChorusIntervals.GuaranteeSize(9,true);mChorusIntervals[0]=0.0f;mChorusIntervals[1]=0.02f;mChorusIntervals[2]=0.05f;mChorusIntervals[3]=0.1f;mChorusIntervals[4]=0.25f;mChorusIntervals[5]=3.5f;mChorusIntervals[6]=6.0f;mChorusIntervals[7]=0.02f;mChorusIntervals[8]=0.05f;
	mChorusOffsets.GuaranteeSize(9,true);mChorusOffsets[0]=0.0f;mChorusOffsets[1]=0.0f;mChorusOffsets[2]=0.0f;mChorusOffsets[3]=0.0f;mChorusOffsets[4]=0.0f;mChorusOffsets[5]=3.5f;mChorusOffsets[6]=6.0f;mChorusOffsets[7]=0.0f;mChorusOffsets[8]=0.0f;
	mChorusVolumes.GuaranteeSize(9,true);mChorusVolumes[0]=0.7f;mChorusVolumes[1]=0.8f;mChorusVolumes[2]=1.0f;mChorusVolumes[3]=1.0f;mChorusVolumes[4]=0.9f;mChorusVolumes[5]=0.9f;mChorusVolumes[6]=0.8f;mChorusVolumes[7]=1.0f;mChorusVolumes[8]=1.0f;
	mChorusSigns.GuaranteeSize(9,true);mChorusSigns[0]=1.0f;mChorusSigns[1]=1.0f;mChorusSigns[2]=1.0f;mChorusSigns[3]=1.0f;mChorusSigns[4]=1.0f;mChorusSigns[5]=1.0f;mChorusSigns[6]=1.0f;mChorusSigns[7]=-1.0f;mChorusSigns[8]=1.0f;
	mChorusHarmonizes.GuaranteeSize(9,true);mChorusHarmonizes[0]=false;mChorusHarmonizes[1]=false;mChorusHarmonizes[2]=false;mChorusHarmonizes[3]=false;mChorusHarmonizes[4]=false;mChorusHarmonizes[5]=false;mChorusHarmonizes[6]=false;mChorusHarmonizes[7]=false;mChorusHarmonizes[8]=true;
	mVolumeValues.GuaranteeSize(6,true);mVolumeValues[0]=0.0f;mVolumeValues[1]=0.5f;mVolumeValues[2]=1.0f;mVolumeValues[3]=1.5f;mVolumeValues[4]=2.0f;mVolumeValues[5]=-1.0f;

	//mOperatorModulatedBy.GuaranteeSize(13,true);mOperatorModulatedBy[0].GuaranteeSize(1,true);mOperatorModulatedBy[0][0].GuaranteeSize(3,true);mOperatorModulatedBy[0][0][0]=2;mOperatorModulatedBy[0][0][1]=3;mOperatorModulatedBy[0][0][2]=4;mOperatorModulatedBy[1].GuaranteeSize(1,true);mOperatorModulatedBy[1][0].GuaranteeSize(2,true);mOperatorModulatedBy[1][0][0]=2;mOperatorModulatedBy[1][0][1]=3;mOperatorModulatedBy[2].GuaranteeSize(2,true);mOperatorModulatedBy[2][0].GuaranteeSize(1,true);mOperatorModulatedBy[2][0][0]=2;mOperatorModulatedBy[2][1].GuaranteeSize(2,true);mOperatorModulatedBy[2][1][0]=3;mOperatorModulatedBy[2][1][1]=4;mOperatorModulatedBy[3].GuaranteeSize(3,true);mOperatorModulatedBy[3][0].GuaranteeSize(2,true);mOperatorModulatedBy[3][0][0]=2;mOperatorModulatedBy[3][0][1]=3;mOperatorModulatedBy[3][1].GuaranteeSize(1,true);mOperatorModulatedBy[3][1][0]=4;mOperatorModulatedBy[3][2].GuaranteeSize(1,true);mOperatorModulatedBy[3][2][0]=4;mOperatorModulatedBy[4].GuaranteeSize(3,true);mOperatorModulatedBy[4][0].GuaranteeSize(1,true);mOperatorModulatedBy[4][0][0]=2;mOperatorModulatedBy[4][1].GuaranteeSize(1,true);mOperatorModulatedBy[4][1][0]=3;mOperatorModulatedBy[4][2].GuaranteeSize(1,true);mOperatorModulatedBy[4][2][0]=4;mOperatorModulatedBy[5].GuaranteeSize(2,true);mOperatorModulatedBy[5][0].GuaranteeSize(1,true);mOperatorModulatedBy[5][0][0]=3;mOperatorModulatedBy[5][1].GuaranteeSize(1,true);mOperatorModulatedBy[5][1][0]=4;mOperatorModulatedBy[6].GuaranteeSize(0,true);mOperatorModulatedBy[7].GuaranteeSize(0,true);mOperatorModulatedBy[8].GuaranteeSize(3,true);mOperatorModulatedBy[8][0].GuaranteeSize(1,true);mOperatorModulatedBy[8][0][0]=3;mOperatorModulatedBy[8][1].GuaranteeSize(1,true);mOperatorModulatedBy[8][1][0]=3;mOperatorModulatedBy[8][2].GuaranteeSize(1,true);mOperatorModulatedBy[8][2][0]=4;mOperatorModulatedBy[9].GuaranteeSize(2,true);mOperatorModulatedBy[9][0].GuaranteeSize(2,true);mOperatorModulatedBy[9][0][0]=3;mOperatorModulatedBy[9][0][1]=4;mOperatorModulatedBy[9][1].GuaranteeSize(2,true);mOperatorModulatedBy[9][1][0]=3;mOperatorModulatedBy[9][1][1]=4;mOperatorModulatedBy[10].GuaranteeSize(0,true);mOperatorModulatedBy[11].GuaranteeSize(3,true);mOperatorModulatedBy[11][0].GuaranteeSize(1,true);mOperatorModulatedBy[11][0][0]=4;mOperatorModulatedBy[11][1].GuaranteeSize(1,true);mOperatorModulatedBy[11][1][0]=4;mOperatorModulatedBy[11][2].GuaranteeSize(1,true);mOperatorModulatedBy[11][2][0]=4;mOperatorModulatedBy[12].GuaranteeSize(0,true);

	mOperatorModulatedBy.GuaranteeSize(13,true);for (int aCount=0;aCount<13;aCount++) mOperatorModulatedBy[aCount].GuaranteeSize(4,true);
	mOperatorModulatedBy[0][0]+=2;mOperatorModulatedBy[0][0]+=3;mOperatorModulatedBy[0][0]+=4;
	mOperatorModulatedBy[1][0]+=2;mOperatorModulatedBy[1][0]+=3;mOperatorModulatedBy[1][2]+=4;
	mOperatorModulatedBy[2][0]+=2;mOperatorModulatedBy[2][1]+=3;mOperatorModulatedBy[2][1]+=4;
	mOperatorModulatedBy[3][0]+=2;mOperatorModulatedBy[3][0]+=3;mOperatorModulatedBy[3][1]+=4;mOperatorModulatedBy[3][2]+=4;
	mOperatorModulatedBy[4][0]+=2;mOperatorModulatedBy[4][1]+=3;mOperatorModulatedBy[4][2]+=4;
	mOperatorModulatedBy[5][0]+=3;mOperatorModulatedBy[5][1]+=4;
	mOperatorModulatedBy[6][1]+=3;mOperatorModulatedBy[6][1]+=4;
	mOperatorModulatedBy[7][1]+=3;mOperatorModulatedBy[7][2]+=4;
	mOperatorModulatedBy[8][0]+=3;mOperatorModulatedBy[8][1]+=3;mOperatorModulatedBy[8][2]+=4;
	mOperatorModulatedBy[9][0]+=3;mOperatorModulatedBy[9][0]+=4;mOperatorModulatedBy[9][1]+=3;mOperatorModulatedBy[9][1]+=4;
	mOperatorModulatedBy[10][2]+=4;
	mOperatorModulatedBy[11][0]+=4;mOperatorModulatedBy[11][1]+=4;mOperatorModulatedBy[11][2]+=4;

	mOperatorAssociatedCarrier.GuaranteeSize(13,true);mOperatorAssociatedCarrier[0].GuaranteeSize(4,true);mOperatorAssociatedCarrier[0][0]=1;mOperatorAssociatedCarrier[0][1]=1;mOperatorAssociatedCarrier[0][2]=1;mOperatorAssociatedCarrier[0][3]=1;mOperatorAssociatedCarrier[1].GuaranteeSize(4,true);mOperatorAssociatedCarrier[1][0]=1;mOperatorAssociatedCarrier[1][1]=1;mOperatorAssociatedCarrier[1][2]=1;mOperatorAssociatedCarrier[1][3]=1;mOperatorAssociatedCarrier[2].GuaranteeSize(4,true);mOperatorAssociatedCarrier[2][0]=1;mOperatorAssociatedCarrier[2][1]=1;mOperatorAssociatedCarrier[2][2]=1;mOperatorAssociatedCarrier[2][3]=1;mOperatorAssociatedCarrier[3].GuaranteeSize(4,true);mOperatorAssociatedCarrier[3][0]=1;mOperatorAssociatedCarrier[3][1]=1;mOperatorAssociatedCarrier[3][2]=1;mOperatorAssociatedCarrier[3][3]=1;mOperatorAssociatedCarrier[4].GuaranteeSize(4,true);mOperatorAssociatedCarrier[4][0]=1;mOperatorAssociatedCarrier[4][1]=1;mOperatorAssociatedCarrier[4][2]=1;mOperatorAssociatedCarrier[4][3]=1;mOperatorAssociatedCarrier[5].GuaranteeSize(4,true);mOperatorAssociatedCarrier[5][0]=1;mOperatorAssociatedCarrier[5][1]=2;mOperatorAssociatedCarrier[5][2]=1;mOperatorAssociatedCarrier[5][3]=2;mOperatorAssociatedCarrier[6].GuaranteeSize(4,true);mOperatorAssociatedCarrier[6][0]=1;mOperatorAssociatedCarrier[6][1]=2;mOperatorAssociatedCarrier[6][2]=2;mOperatorAssociatedCarrier[6][3]=2;mOperatorAssociatedCarrier[7].GuaranteeSize(4,true);mOperatorAssociatedCarrier[7][0]=1;mOperatorAssociatedCarrier[7][1]=2;mOperatorAssociatedCarrier[7][2]=2;mOperatorAssociatedCarrier[7][3]=2;mOperatorAssociatedCarrier[8].GuaranteeSize(4,true);mOperatorAssociatedCarrier[8][0]=1;mOperatorAssociatedCarrier[8][1]=2;mOperatorAssociatedCarrier[8][2]=2;mOperatorAssociatedCarrier[8][3]=2;mOperatorAssociatedCarrier[9].GuaranteeSize(4,true);mOperatorAssociatedCarrier[9][0]=1;mOperatorAssociatedCarrier[9][1]=2;mOperatorAssociatedCarrier[9][2]=2;mOperatorAssociatedCarrier[9][3]=2;mOperatorAssociatedCarrier[10].GuaranteeSize(4,true);mOperatorAssociatedCarrier[10][0]=1;mOperatorAssociatedCarrier[10][1]=2;mOperatorAssociatedCarrier[10][2]=3;mOperatorAssociatedCarrier[10][3]=3;mOperatorAssociatedCarrier[11].GuaranteeSize(4,true);mOperatorAssociatedCarrier[11][0]=1;mOperatorAssociatedCarrier[11][1]=2;mOperatorAssociatedCarrier[11][2]=3;mOperatorAssociatedCarrier[11][3]=3;mOperatorAssociatedCarrier[12].GuaranteeSize(4,true);mOperatorAssociatedCarrier[12][0]=1;mOperatorAssociatedCarrier[12][1]=2;mOperatorAssociatedCarrier[12][2]=3;mOperatorAssociatedCarrier[12][3]=4;
	mOperatorCarrierCounts.GuaranteeSize(13,true);mOperatorCarrierCounts[0]=1;mOperatorCarrierCounts[1]=1;mOperatorCarrierCounts[2]=1;mOperatorCarrierCounts[3]=1;mOperatorCarrierCounts[4]=1;mOperatorCarrierCounts[5]=2;mOperatorCarrierCounts[6]=2;mOperatorCarrierCounts[7]=2;mOperatorCarrierCounts[8]=2;mOperatorCarrierCounts[9]=2;mOperatorCarrierCounts[10]=3;mOperatorCarrierCounts[11]=3;mOperatorCarrierCounts[12]=4;
	mOperatorCarrierChorus.GuaranteeSize(4,true);mOperatorCarrierChorus[0]=0.0f;mOperatorCarrierChorus[1]=0.04f;mOperatorCarrierChorus[2]=-0.073f;mOperatorCarrierChorus[3]=0.091f;
	mOperatorFrequencies.GuaranteeSize(15,true);mOperatorFrequencies[0]=1.0f;mOperatorFrequencies[1]=1.0f;mOperatorFrequencies[2]=2.0f;mOperatorFrequencies[3]=2.0f;mOperatorFrequencies[4]=3.0f;mOperatorFrequencies[5]=4.0f;mOperatorFrequencies[6]=5.0f;mOperatorFrequencies[7]=6.0f;mOperatorFrequencies[8]=7.0f;mOperatorFrequencies[9]=8.0f;mOperatorFrequencies[10]=9.0f;mOperatorFrequencies[11]=11.0f;mOperatorFrequencies[12]=13.0f;mOperatorFrequencies[13]=16.0f;mOperatorFrequencies[14]=20.0f;
	mOperatorHzOffsets.GuaranteeSize(15,true);mOperatorHzOffsets[0]=0.0f;mOperatorHzOffsets[1]=1.5f;mOperatorHzOffsets[2]=0.0f;mOperatorHzOffsets[3]=-1.3f;mOperatorHzOffsets[4]=0.0f;mOperatorHzOffsets[5]=0.0f;mOperatorHzOffsets[6]=0.0f;mOperatorHzOffsets[7]=0.0f;mOperatorHzOffsets[8]=0.0f;mOperatorHzOffsets[9]=0.0f;mOperatorHzOffsets[10]=0.0f;mOperatorHzOffsets[11]=0.0f;mOperatorHzOffsets[12]=0.0f;mOperatorHzOffsets[13]=0.0f;mOperatorHzOffsets[14]=0.0f;
	mOperatorAmplitudeSigns.GuaranteeSize(15,true);mOperatorAmplitudeSigns[0]=1.0f;mOperatorAmplitudeSigns[1]=-1.0f;mOperatorAmplitudeSigns[2]=1.0f;mOperatorAmplitudeSigns[3]=-1.0f;mOperatorAmplitudeSigns[4]=1.0f;mOperatorAmplitudeSigns[5]=1.0f;mOperatorAmplitudeSigns[6]=1.0f;mOperatorAmplitudeSigns[7]=1.0f;mOperatorAmplitudeSigns[8]=1.0f;mOperatorAmplitudeSigns[9]=1.0f;mOperatorAmplitudeSigns[10]=1.0f;mOperatorAmplitudeSigns[11]=1.0f;mOperatorAmplitudeSigns[12]=1.0f;mOperatorAmplitudeSigns[13]=1.0f;mOperatorAmplitudeSigns[14]=1.0f;
	mOperatorEnvelopeSpeed.GuaranteeSize(15,true);mOperatorEnvelopeSpeed[0]=0.0f;mOperatorEnvelopeSpeed[1]=0.0f;mOperatorEnvelopeSpeed[2]=0.0f;mOperatorEnvelopeSpeed[3]=32.0f;mOperatorEnvelopeSpeed[4]=8.0f;mOperatorEnvelopeSpeed[5]=2.0f;mOperatorEnvelopeSpeed[6]=32.0f;mOperatorEnvelopeSpeed[7]=8.0f;mOperatorEnvelopeSpeed[8]=2.0f;mOperatorEnvelopeSpeed[9]=32.0f;mOperatorEnvelopeSpeed[10]=8.0f;mOperatorEnvelopeSpeed[11]=2.0f;mOperatorEnvelopeSpeed[12]=4.0f;mOperatorEnvelopeSpeed[13]=2.0f;mOperatorEnvelopeSpeed[14]=1.0f;
	mOperatorEnvelopeInverted.GuaranteeSize(15,true);mOperatorEnvelopeInverted[0]=false;mOperatorEnvelopeInverted[1]=false;mOperatorEnvelopeInverted[2]=false;mOperatorEnvelopeInverted[3]=false;mOperatorEnvelopeInverted[4]=false;mOperatorEnvelopeInverted[5]=false;mOperatorEnvelopeInverted[6]=false;mOperatorEnvelopeInverted[7]=false;mOperatorEnvelopeInverted[8]=false;mOperatorEnvelopeInverted[9]=true;mOperatorEnvelopeInverted[10]=true;mOperatorEnvelopeInverted[11]=true;mOperatorEnvelopeInverted[12]=false;mOperatorEnvelopeInverted[13]=false;mOperatorEnvelopeInverted[14]=false;
	mOperatorFeedbackIndices.GuaranteeSize(18);	for (int aCount=0;aCount<18;aCount++) mOperatorFeedbackIndices[aCount].GuaranteeSize(4);mOperatorFeedbackIndices[0][0].Set(0)=1;mOperatorFeedbackIndices[1][1].Set(0)=2;mOperatorFeedbackIndices[2][2].Set(0)=3;mOperatorFeedbackIndices[3][3].Set(0)=4;mOperatorFeedbackIndices[4][0].Set(0)=1;mOperatorFeedbackIndices[4][1].Set(0)=2;mOperatorFeedbackIndices[5][2].Set(0)=3;mOperatorFeedbackIndices[5][3].Set(0)=4;mOperatorFeedbackIndices[6][0].Set(0)=1;mOperatorFeedbackIndices[6][1].Set(0)=2;mOperatorFeedbackIndices[6][2].Set(0)=3;mOperatorFeedbackIndices[7][1].Set(0)=2;mOperatorFeedbackIndices[7][2].Set(0)=3;mOperatorFeedbackIndices[7][3].Set(0)=4;mOperatorFeedbackIndices[8][0].Set(0)=1;mOperatorFeedbackIndices[8][1].Set(0)=2;mOperatorFeedbackIndices[8][2].Set(0)=3;mOperatorFeedbackIndices[8][3].Set(0)=4;mOperatorFeedbackIndices[9][1].Set(0)=1;mOperatorFeedbackIndices[10][2].Set(0)=1;mOperatorFeedbackIndices[11][3].Set(0)=1;mOperatorFeedbackIndices[12][2].Set(0)=2;mOperatorFeedbackIndices[13][3].Set(0)=2;mOperatorFeedbackIndices[14][3].Set(0)=3;mOperatorFeedbackIndices[15][2].Set(0)=1;mOperatorFeedbackIndices[15][3].Set(0)=2;mOperatorFeedbackIndices[16][2].Set(0)=2;mOperatorFeedbackIndices[16][3].Set(0)=1;mOperatorFeedbackIndices[17][1].Set(0)=1;mOperatorFeedbackIndices[17][2].Set(0)=2;mOperatorFeedbackIndices[17][3].Set(0)=3;

	Array<double> aWave0;aWave0.GuaranteeSize(32,true);aWave0[0]=1.0/15.0;aWave0[1]=3.0/15.0;aWave0[2]=5.0/15.0;aWave0[3]=7.0/15.0;aWave0[4]=9.0/15.0;aWave0[5]=11.0/15.0;aWave0[6]=13.0/15.0;aWave0[7]=15.0/15.0;aWave0[8]=15.0/15.0;aWave0[9]=13.0/15.0;aWave0[10]=11.0/15.0;aWave0[11]=9.0/15.0;aWave0[12]=7.0/15.0;aWave0[13]=5.0/15.0;aWave0[14]=3.0/15.0;aWave0[15]=1.0/15.0;aWave0[16]=-1.0/15.0;aWave0[17]=-3.0/15.0;aWave0[18]=-5.0/15.0;aWave0[19]=-7.0/15.0;aWave0[20]=-9.0/15.0;aWave0[21]=-11.0/15.0;aWave0[22]=-13.0/15.0;aWave0[23]=-15.0/15.0;aWave0[24]=-15.0/15.0;aWave0[25]=-13.0/15.0;aWave0[26]=-11.0/15.0;aWave0[27]=-9.0/15.0;aWave0[28]=-7.0/15.0;aWave0[29]=-5.0/15.0;aWave0[30]=-3.0/15.0;aWave0[31]=-1.0/15.0;
	Array<double> aWave1;aWave1.GuaranteeSize(2,true);aWave1[0]=1.0;aWave1[1]=-1.0;
	Array<double> aWave2;aWave2.GuaranteeSize(4,true);aWave2[0]=1.0;aWave2[1]=-1.0;aWave2[2]=-1.0;aWave2[3]=-1.0;
	Array<double> aWave3;aWave3.GuaranteeSize(8,true);aWave3[0]=1.0;aWave3[1]=-1.0;aWave3[2]=-1.0;aWave3[3]=-1.0;aWave3[4]=-1.0;aWave3[5]=-1.0;aWave3[6]=-1.0;aWave3[7]=-1.0;
	Array<double> aWave4;aWave4.GuaranteeSize(32,true);aWave4[0]=1.0/31.0;aWave4[1]=3.0/31.0;aWave4[2]=5.0/31.0;aWave4[3]=7.0/31.0;aWave4[4]=9.0/31.0;aWave4[5]=11.0/31.0;aWave4[6]=13.0/31.0;aWave4[7]=15.0/31.0;aWave4[8]=17.0/31.0;aWave4[9]=19.0/31.0;aWave4[10]=21.0/31.0;aWave4[11]=23.0/31.0;aWave4[12]=25.0/31.0;aWave4[13]=27.0/31.0;aWave4[14]=29.0/31.0;aWave4[15]=31.0/31.0;aWave4[16]=-31.0/31.0;aWave4[17]=-29.0/31.0;aWave4[18]=-27.0/31.0;aWave4[19]=-25.0/31.0;aWave4[20]=-23.0/31.0;aWave4[21]=-21.0/31.0;aWave4[22]=-19.0/31.0;aWave4[23]=-17.0/31.0;aWave4[24]=-15.0/31.0;aWave4[25]=-13.0/31.0;aWave4[26]=-11.0/31.0;aWave4[27]=-9.0/31.0;aWave4[28]=-7.0/31.0;aWave4[29]=-5.0/31.0;aWave4[30]=-3.0/31.0;aWave4[31]=-1.0/31.0;
	Array<double> aWave5;aWave5.GuaranteeSize(16,true);aWave5[0]=0.0;aWave5[1]=-0.2;aWave5[2]=-0.4;aWave5[3]=-0.6;aWave5[4]=-0.8;aWave5[5]=-1.0;aWave5[6]=1.0;aWave5[7]=-0.8;aWave5[8]=-0.6;aWave5[9]=-0.4;aWave5[10]=-0.2;aWave5[11]=1.0;aWave5[12]=0.8;aWave5[13]=0.6;aWave5[14]=0.4;aWave5[15]=0.2;
	Array<double> aWave6;aWave6.GuaranteeSize(16,true);aWave6[0]=1.0;aWave6[1]=1.0;aWave6[2]=1.0;aWave6[3]=1.0;aWave6[4]=1.0;aWave6[5]=-1.0;aWave6[6]=-1.0;aWave6[7]=-1.0;aWave6[8]=1.0;aWave6[9]=1.0;aWave6[10]=1.0;aWave6[11]=1.0;aWave6[12]=-1.0;aWave6[13]=-1.0;aWave6[14]=-1.0;aWave6[15]=-1.0;
	Array<double> aWave7;aWave7.GuaranteeSize(6,true);aWave7[0]=1.0;aWave7[1]=-1.0;aWave7[2]=1.0;aWave7[3]=-1.0;aWave7[4]=1.0;aWave7[5]=0.0;
	Array<double> aWave8;aWave8.GuaranteeSize(64,true);aWave8[0]=0.0;aWave8[1]=0.2;aWave8[2]=0.4;aWave8[3]=0.5;aWave8[4]=0.6;aWave8[5]=0.7;aWave8[6]=0.8;aWave8[7]=0.85;aWave8[8]=0.9;aWave8[9]=0.95;aWave8[10]=1.0;aWave8[11]=1.0;aWave8[12]=1.0;aWave8[13]=1.0;aWave8[14]=1.0;aWave8[15]=1.0;aWave8[16]=1.0;aWave8[17]=1.0;aWave8[18]=1.0;aWave8[19]=1.0;aWave8[20]=1.0;aWave8[21]=1.0;aWave8[22]=1.0;aWave8[23]=1.0;aWave8[24]=1.0;aWave8[25]=0.95;aWave8[26]=0.9;aWave8[27]=0.85;aWave8[28]=0.8;aWave8[29]=0.7;aWave8[30]=0.6;aWave8[31]=0.5;aWave8[32]=0.4;aWave8[33]=0.2;aWave8[34]=0.0;aWave8[35]=-0.2;aWave8[36]=-0.4;aWave8[37]=-0.5;aWave8[38]=-0.6;aWave8[39]=-0.7;aWave8[40]=-0.8;aWave8[41]=-0.85;aWave8[42]=-0.9;aWave8[43]=-0.95;aWave8[44]=-1.0;aWave8[45]=-1.0;aWave8[46]=-1.0;aWave8[47]=-1.0;aWave8[48]=-1.0;aWave8[49]=-1.0;aWave8[50]=-1.0;aWave8[51]=-1.0;aWave8[52]=-1.0;aWave8[53]=-1.0;aWave8[54]=-1.0;aWave8[55]=-0.95;aWave8[56]=-0.9;aWave8[57]=-0.85;aWave8[58]=-0.8;aWave8[59]=-0.7;aWave8[60]=-0.6;aWave8[61]=-0.5;aWave8[62]=-0.4;aWave8[63]=-0.2;

	mWaves.GuaranteeSize(9);
	mWaves[0].Copy(CenterWave(aWave0));
	mWaves[1].Copy(CenterWave(aWave1));
	mWaves[2].Copy(CenterWave(aWave2));
	mWaves[3].Copy(CenterWave(aWave3));
	mWaves[4].Copy(CenterWave(aWave4));
	mWaves[5].Copy(CenterWave(aWave5));
	mWaves[6].Copy(CenterWave(aWave6));
	mWaves[7].Copy(CenterWave(aWave7));
	mWaves[8].Copy(CenterWave(aWave8));

	mSineWaveLength=1<<8; // 256
	mSineWave.Copy(GenerateSineWave());
}

Array<double>& BeepBoxConfig::GenerateSineWave()
{
	static Array<double> aResult;
	aResult.GuaranteeSize(mSineWaveLength+1);
	for (float aI=0;aI<mSineWaveLength+1;aI++) aResult[(int)aI]=(float)sin(aI*gMath.mPI*2.0f/mSineWaveLength);
	return aResult;
}

Array<double>& BeepBoxConfig::CenterWave(Array<double>& theWave)
{
	static Array<double> aResult;aResult.Reset();aResult.GuaranteeSize(theWave.Size());
	double aSum=0.0f;for (int aI=0;aI<theWave.Size();aI++) {aResult[aI]=theWave[aI];aSum+=theWave[aI];}
	double aAverage=aSum/(float)theWave.Size();for (int aI=0;aI<theWave.Size();aI++) aResult[aI]-=aAverage;
	return aResult;
}

ReadOnlyArray<double>& BeepBoxConfig::GetDrumWave(int theIndex)
{
	ReadOnlyArray<double>& aWave=mDrumWaves[theIndex];
	if (aWave.Size()==0) //if (wave == null)
	{
		aWave.GuaranteeSize(32768);
		for (int aCount=0;aCount<aWave.Size();aCount++) aWave[aCount]=0;
		if (theIndex==0)
		{
			int aDrumBuffer=1;
			for (int i=0;i<32768;i++) 
			{
				aWave[i]=(aDrumBuffer&1)*2.0f-1.0f;
				int aNewBuffer=aDrumBuffer>>1;
				if (((aDrumBuffer + aNewBuffer) & 1) == 1) aNewBuffer+=1<<14;
				aDrumBuffer=aNewBuffer;
			}
		}
		else if (theIndex==1) 
		{
			for (int i=0;i<32768;i++) aWave[i]=gRand.GetF()*2.0f-1.0f;
		} 
		else if (theIndex==2) 
		{
			int aDrumBuffer=1;
			for (int i=0;i<32768;i++)
			{
				aWave[i]=(aDrumBuffer&1)*2.0f-1.0f;
				int aNewBuffer=aDrumBuffer>>1;
                if (((aDrumBuffer+aNewBuffer)&1)==1) aNewBuffer+=2<<14;
                aDrumBuffer=aNewBuffer;
            }
        } 
		else if (theIndex==3) 
		{
			int aDrumBuffer=1;
			for (int i=0;i<32768;i++)
			{
				aWave[i]=(aDrumBuffer&1)*2.0f-1.0f;
				int aNewBuffer=aDrumBuffer>>1;
                if (((aDrumBuffer+aNewBuffer)&1)==1) aNewBuffer+=10<<2;
                aDrumBuffer=aNewBuffer;
            }
		} 
		else if (theIndex==4) 
		{
			DrawNoiseSpectrum(aWave, 10, 11, 1, 1, 0);
			DrawNoiseSpectrum(aWave, 11, 14, -2, -2, 0);
			InverseRealFourierTransform(aWave);
			ScaleElementsByFactor(aWave,1.0f/(float)sqrt((float)aWave.Size()));
		} 
		else 
		{
			gOut.Out("BeepBox Error: Unrecognized Drum Index");
		}
	}

	//gOut.Out("GetDrumWave(%d).Size()=%d",theIndex, aWave.Size());
	return aWave;
}

int CountBits(int theN)
{
	if (!gMath.IsPowerOfTwo(theN)) {gOut.Out("BeepBox Error: Need pow^2 index!");return 0;}
	return (int)(gMath.Round((float)log((float)theN)/(float)log(2.0f)));
}

void BeepBoxConfig::ScaleElementsByFactor(Array<double>& theArray, float theFactor)
{
	for (int i=0;i<theArray.Size();i++) theArray[i]*=theFactor;
}


void BeepBoxConfig::InverseRealFourierTransform(Array<double>& theArray)
{
	int aFullArrayLength=theArray.Size();

	int aTotalPasses=CountBits(aFullArrayLength);
	if (aFullArrayLength<4) {gOut.Out("BeepBox Error: FFT Array length must be at least 4!");return;}
	
	// Perform all but the last few passes in reverse.
	for (int aPass=aTotalPasses-1;aPass>=2;aPass--) 
	{
		int aSubStride=1<<aPass;
		int aMidSubStride=aSubStride>>1;
		int aStride=aSubStride<<1;
		float aRadiansIncrement=gMath.mPI*2.0f/(float)aStride;
		float aCosIncrement=(float)cos(aRadiansIncrement);
		float aSinIncrement=(float)sin(aRadiansIncrement);
		float aOscillatorMultiplier=2.0f*aCosIncrement;
			
		for (int aStartIndex=0;aStartIndex<aFullArrayLength;aStartIndex+=aStride)
		{
			int aStartIndexA=aStartIndex;
			int aMidIndexA=aStartIndexA+aMidSubStride;
			int aStartIndexB=aStartIndexA+aSubStride;
			int aMidIndexB=aStartIndexB+aMidSubStride;
			int aStopIndex=aStartIndexB+aSubStride;
			double aRealStartA=theArray[aStartIndexA];
			double aImagStartB=theArray[aStartIndexB];
			theArray[aStartIndexA]=aRealStartA+aImagStartB;
			theArray[aMidIndexA]*=2;
			theArray[aStartIndexB]=aRealStartA-aImagStartB;
			theArray[aMidIndexB]*=2;
			double aC=aCosIncrement;
			double aS=-aSinIncrement;
			double aCPrev=1.0f;
			double aSPrev=0.0f;
			for (int aIndex=1;aIndex<aMidSubStride;aIndex++) 
			{
				int aIndexA0=aStartIndexA+aIndex;
				int aIndexA1=aStartIndexB-aIndex;
				int aIndexB0=aStartIndexB+aIndex;
				int aIndexB1=aStopIndex-aIndex;
				double aReal0=theArray[aIndexA0];
				double aReal1=theArray[aIndexA1];
				double aImag0=theArray[aIndexB0];
				double aImag1=theArray[aIndexB1];
				double aTempA=aReal0-aReal1;
				double aTempB=aImag0+aImag1;
				theArray[aIndexA0]=aReal0+aReal1;
				theArray[aIndexA1]=aImag1-aImag0;
				theArray[aIndexB0]=aTempA*aC-aTempB*aS;
				theArray[aIndexB1]=aTempB*aC+aTempA*aS;
				double aCTemp=aOscillatorMultiplier*aC-aCPrev;
				double aSTemp=aOscillatorMultiplier*aS-aSPrev;
				aCPrev=aC;
				aSPrev=aS;
				aC=aCTemp;
				aS=aSTemp;
			}
		}
	}

	for (int aIndex=0;aIndex<aFullArrayLength;aIndex+=4) 
	{
		int	aIndex1=aIndex+1;
		int aIndex2=aIndex+2;
		int aIndex3=aIndex + 3;
		double aReal0=theArray[aIndex];
		double aReal1=theArray[aIndex1] * 2;
		double aImag2=theArray[aIndex2];
		double aImag3=theArray[aIndex3] * 2;
		double aTempA=aReal0+aImag2;
		double aTempB=aReal0-aImag2;
		theArray[aIndex]=aTempA+aReal1;
		theArray[aIndex1]=aTempA-aReal1;
		theArray[aIndex2]=aTempB+aImag3;
		theArray[aIndex3]=aTempB-aImag3;
	}
	ReverseIndexBits(theArray);
}

void BeepBoxConfig::ReverseIndexBits(Array<double>& theArray)
{
	int aFullArrayLength=theArray.Size();
	int aBitCount=CountBits(aFullArrayLength);
	if (aBitCount>16) {gOut.Out("BeepBox Error: FFT Array length must be greater than 2^16!");return;}

	int aFinalShift=16-aBitCount;
	for (int i=0;i<aFullArrayLength;i++) 
	{
		// Dear Javascript: Please support bit order reversal intrinsics. Thanks! :D
		int j;
		j=((i&0xaaaa)>>1) | ((i&0x5555)<<1);
		j=((j&0xcccc)>>2) | ((j&0x3333)<<2);
		j=((j&0xf0f0)>>4) | ((j&0x0f0f)<<4);
		j=((j        >>8) | ((j&  0xff)<<8))>>aFinalShift;
		if (j>i) 
		{
			double aTemp=theArray[i];
			theArray[i]=theArray[j];
			theArray[j]=aTemp;
		}
	}
}



void BeepBoxConfig::DrawNoiseSpectrum(Array<double>& theWave, int lowOctave, int highOctave, int lowPower, int highPower, int overalSlope)
{
	int aReferenceOctave=11;
	int aReferenceIndex=1<<aReferenceOctave;
	int aLowIndex=(int)pow(2.0f,lowOctave);
	int aHighIndex=(int)pow(2.0f,highOctave);
	float aLog2=(float)log(2.0f);

	for (int i=aLowIndex;i<aHighIndex;i++) 
	{
		float aAmplitude=(float)pow(2.0f,lowPower+(highPower-lowPower)*((float)log((float)i)/aLog2-lowOctave)/(highOctave-lowOctave));
		aAmplitude*=(float)pow((float)i/(float)aReferenceIndex,overalSlope);
		float aRadians=gRand.GetF()*gMath.mPI*2.0f;
		theWave[i]=(float)cos(aRadians)*aAmplitude;
		theWave[32768-i]=(float)sin(aRadians)*aAmplitude;
	}
}


enum
{
	SongTagCode_beatCount='a',
	SongTagCode_bars='b',
	SongTagCode_effect='c',
	SongTagCode_transition='d',
	SongTagCode_loopEnd='e',
	SongTagCode_filter='f',
	SongTagCode_barCount='g',
	SongTagCode_chorus='h',
	SongTagCode_instrumentCount='i',
	SongTagCode_patternCount='j',
	SongTagCode_key='k',
	SongTagCode_loopStart='l',
	SongTagCode_reverb='m',
	SongTagCode_channelCount='n',
	SongTagCode_channelOctave='o',
	SongTagCode_patterns='p',
	SongTagCode_rhythm='r',
	SongTagCode_scale='s',
	SongTagCode_tempo='t',
	SongTagCode_volume='v',
	SongTagCode_wave='w',
	SongTagCode_algorithm='A',
	SongTagCode_feedbackAmplitude='B',
	SongTagCode_operatorEnvelopes='E',
	SongTagCode_feedbackType='F',
	SongTagCode_operatorAmplitudes='P',
	SongTagCode_operatorFrequencies='Q',
	SongTagCode_startInstrument='T',
	SongTagCode_feedbackEnvelope='V',
};




void BeepBoxSong::InitToDefault(bool andResetChannels)
{
	mScale = 0;
	mKey=BeepBoxConfig::mKeyNames_Length-1;
	mLoopStart=0;
	mLoopLength=4;
	mTempo=7;
	mReverb=0;
	mBeatsPerBar=8;
	mBarCount=16;
	mPatternsPerChannel=8;
	mPartsPerBeat=4;
	mInstrumentsPerChannel=1;

	if (andResetChannels)
	{
		mPitchChannelCount=3;
		mDrumChannelCount=1;

		for (int aChannelIndex=0;aChannelIndex<GetChannelCount();aChannelIndex++)
		{
			while (mChannels.GetCount()<=aChannelIndex) mChannels+=new Channel;
			Smart(Channel) aChannel=mChannels[aChannelIndex];
			aChannel->mOctave=3-aChannelIndex;

			//
			// In the original TypeScript, these lines of code are kinda weird... the coder has to do some weirdly esoteric stuff to
			// re-use old patterns/instruments/etc, and then explicitely clips the lists.  Which I assume is just how other languages work.
			// In c++ we just say clear it out and remake it.
			//
			aChannel->mPatterns.Clear();for (int aPattern=0;aPattern<mPatternsPerChannel;aPattern++) aChannel->mPatterns+=new Pattern;
			aChannel->mInstruments.Clear();for (int aInstrument=0;aInstrument<mInstrumentsPerChannel;aInstrument++) aChannel->mInstruments+=new Instrument;
			aChannel->mBars.Reset();for (int aBar=mBarCount-1;aBar>=0;aBar--) aChannel->mBars.Set(aBar)=1;
		}
	}
}


bool BeepBoxSong::Load(String theCompressed)
{
	gBadSong=false;
	if (theCompressed.Len()==0) return false;
	if (!gBeepboxConfig) gBeepboxConfig=new BeepBoxConfig;


	int aHashTag=theCompressed.Find('#');
	if (aHashTag<0) return false;
	theCompressed=theCompressed.GetSegment(aHashTag,999999);
	if (theCompressed.Len()==0) return false;

	int aCharIndex=0;
	while (theCompressed[aCharIndex]<=32) aCharIndex++;
	if (theCompressed[aCharIndex]=='#') aCharIndex++;
	if (theCompressed[aCharIndex]=='{') return false; // BeepBox can import JSON, but it would take motivation beyond anything I've ever felt in my life to convince me to put in a JSON reader into this thing.

	int aVersion=Base64CharCodeToInt(theCompressed[aCharIndex++]);
	if (gBadSong) return false;
	int aMinVersion=5;
	int aMaxVersion=6;
	if (aVersion==-1 || aVersion>aMaxVersion || aVersion<aMinVersion) return false;

	bool aBeforeThree=aVersion<3;
	bool aBeforeFour=aVersion<4;
	bool aBeforeFive=aVersion<5;
	bool aBeforeSix=aVersion<6;
	InitToDefault(aBeforeSix);

	if (aBeforeThree) {EnumSmartList(Channel,aChannel,mChannels) aChannel->mInstruments[0]->mTransition=0;mChannels[3]->mInstruments[0]->mWave=0;}

	int aInstrumentChannelIterator=0;
	int aInstrumentIndexIterator=-1;

	while (aCharIndex<theCompressed.Len()) 
	{
		if (gBadSong) return false;
		int aCommand=theCompressed[aCharIndex++];
#ifdef _TRACE
		gTrace.Out("Command[%d]: %d (%c)",aCharIndex-1,aCommand,aCommand);
#endif
		int aChannel;
		switch (aCommand)
		{
		case SongTagCode_channelCount:
			mPitchChannelCount=Base64CharCodeToInt(theCompressed[aCharIndex++]);
			mDrumChannelCount=Base64CharCodeToInt(theCompressed[aCharIndex++]);
			mPitchChannelCount=Clip(BeepBoxConfig::mPitchChannelCountMin,BeepBoxConfig::mPitchChannelCountMax+1,mPitchChannelCount);
			mDrumChannelCount=Clip(BeepBoxConfig::mDrumChannelCountMin,BeepBoxConfig::mDrumChannelCountMax+1,mDrumChannelCount);
			mChannels.Reset();for (int aChannelIndex=0;aChannelIndex<GetChannelCount();aChannelIndex++) mChannels+=new Channel();

#ifdef _TRACE
			gTrace.Out("ChannelInfo: %d %d %d",mPitchChannelCount,mDrumChannelCount,mChannels.GetCount());
#endif


			break;
		case SongTagCode_scale:
			mScale=Base64CharCodeToInt(theCompressed[aCharIndex++]);
			if (aBeforeThree && mScale==10) mScale=11;
			break;
		case SongTagCode_key:
			mKey=Base64CharCodeToInt(theCompressed[aCharIndex++]);
			break;
		case SongTagCode_loopStart:
			if (aBeforeFive) mLoopStart=Base64CharCodeToInt(theCompressed[aCharIndex++]);
			else 
			{
				mLoopStart=(Base64CharCodeToInt(theCompressed[aCharIndex++])<<6);
				mLoopStart+=Base64CharCodeToInt(theCompressed[aCharIndex++]);
			}
			break;
		case SongTagCode_loopEnd:
			if (aBeforeFive) mLoopLength=Base64CharCodeToInt(theCompressed[aCharIndex++]);
			else 
			{
				mLoopLength=(Base64CharCodeToInt(theCompressed[aCharIndex++])<<6);
				mLoopLength+=Base64CharCodeToInt(theCompressed[aCharIndex++])+1;
			}
			break;
		case SongTagCode_tempo:
			if (aBeforeFour) 
			{
				int aTempoList[]={1, 4, 7, 10};
				mTempo=aTempoList[Base64CharCodeToInt(theCompressed[aCharIndex++])];
			}
			
			else mTempo=Base64CharCodeToInt(theCompressed[aCharIndex++]);
			mTempo=Clip(0,BeepBoxConfig::mTempoSteps,mTempo);
			break;
		case SongTagCode_reverb:
			mReverb=Base64CharCodeToInt(theCompressed[aCharIndex++]);
			mReverb=Clip(0,BeepBoxConfig::mReverbRange,mReverb);
			break;
		case SongTagCode_beatCount:
			if (aBeforeThree) 
			{
				int aBeatsList[]={6, 7, 8, 9, 10};
				mBeatsPerBar=aBeatsList[Base64CharCodeToInt(theCompressed[aCharIndex++])];
			} 
			else mBeatsPerBar=Base64CharCodeToInt(theCompressed[aCharIndex++])+1;
			mBeatsPerBar=_max(BeepBoxConfig::mBeatsPerBarMin,_min(BeepBoxConfig::mBeatsPerBarMax,mBeatsPerBar));
			break;
		case SongTagCode_barCount:
			//
			// Broke this into two lines because the VS code optimizer seems to SERIOUSLY mangle mBarCount's value
			// if we leave them on the same line!
			//
			mBarCount=(Base64CharCodeToInt(theCompressed[aCharIndex++])<<6);
			mBarCount+=Base64CharCodeToInt(theCompressed[aCharIndex++])+1;
			mBarCount=_max(BeepBoxConfig::mBarCountMin,_min(BeepBoxConfig::mBarCountMax,mBarCount));
			for (aChannel=0;aChannel<GetChannelCount();aChannel++) {mChannels[aChannel]->mBars.Reset();for (int aBar=mBarCount-1;aBar>=0;aBar--) mChannels[aChannel]->mBars.Set(aBar)=1;}
			break;
		case SongTagCode_patternCount:
			mPatternsPerChannel=Base64CharCodeToInt(theCompressed[aCharIndex++])+1;
			mPatternsPerChannel=_max(BeepBoxConfig::mPatternsPerChannelMin,_min(BeepBoxConfig::mPatternsPerChannelMax,mPatternsPerChannel));
			for (aChannel=0;aChannel<GetChannelCount();aChannel++) {mChannels[aChannel]->mPatterns.Clear();for (int aPattern=0;aPattern<mPatternsPerChannel;aPattern++) mChannels[aChannel]->mPatterns+=new Pattern;}
			break;
		case SongTagCode_instrumentCount:
			mInstrumentsPerChannel=Base64CharCodeToInt(theCompressed[aCharIndex++])+1;
			mInstrumentsPerChannel=_max(BeepBoxConfig::mInstrumentsPerChannelMin,_min(BeepBoxConfig::mInstrumentsPerChannelMax,mInstrumentsPerChannel));
			for (aChannel=0;aChannel<GetChannelCount();aChannel++) {mChannels[aChannel]->mInstruments.Clear();for (int aInstrument=0;aInstrument<mInstrumentsPerChannel;aInstrument++) mChannels[aChannel]->mInstruments+=new Instrument;}
			break;
		case SongTagCode_rhythm:
			mPartsPerBeat=gBeepboxConfig->mPartCounts[Base64CharCodeToInt(theCompressed[aCharIndex++])];
			break;
		case SongTagCode_channelOctave:
			if (aBeforeThree) {aChannel=Base64CharCodeToInt(theCompressed[aCharIndex++]);mChannels[aChannel]->mOctave=Clip(0,5,Base64CharCodeToInt(theCompressed[aCharIndex++]));}
			else {for (aChannel=0;aChannel<GetChannelCount();aChannel++) mChannels[aChannel]->mOctave=Clip(0, 5,Base64CharCodeToInt(theCompressed[aCharIndex++]));}
			break;
		case SongTagCode_startInstrument:
			{
				aInstrumentIndexIterator++;
				if (aInstrumentIndexIterator>=mInstrumentsPerChannel) {aInstrumentChannelIterator++;aInstrumentIndexIterator=0;}
				if (aInstrumentChannelIterator>=BeepBoxConfig::mChannelCount) return false;

				Smart(Instrument) aInstrument=mChannels[aInstrumentChannelIterator]->mInstruments[aInstrumentIndexIterator];
				aInstrument->SetTypeAndReset(Clip(0,2,Base64CharCodeToInt(theCompressed[aCharIndex++])));
			}
			break;
		case SongTagCode_wave:
			if (aBeforeThree) 
			{
				aChannel=Base64CharCodeToInt(theCompressed[aCharIndex++]);
				mChannels[aChannel]->mInstruments[0]->mWave=Clip(0,BeepBoxConfig::mWaveNames_Length, Base64CharCodeToInt(theCompressed[aCharIndex++]));
			} 
			else if (aBeforeSix) 
			{
				for (aChannel=0;aChannel<GetChannelCount();aChannel++) 
				{
					bool aIsDrums=(aChannel>=mPitchChannelCount);
					for (int aCount=0;aCount<mInstrumentsPerChannel;aCount++) mChannels[aChannel]->mInstruments[aCount]->mWave=Clip(0,aIsDrums?BeepBoxConfig::mDrumNames_Length : BeepBoxConfig::mWaveNames_Length, Base64CharCodeToInt(theCompressed[aCharIndex++]));
				}
			}
			else 
			{
				bool aIsDrums=(aInstrumentChannelIterator>=mPitchChannelCount);
				if (aInstrumentIndexIterator<0) return false;
				mChannels[aInstrumentChannelIterator]->mInstruments[aInstrumentIndexIterator]->mWave=Clip(0,aIsDrums?BeepBoxConfig::mDrumNames_Length : BeepBoxConfig::mWaveNames_Length,Base64CharCodeToInt(theCompressed[aCharIndex++]));
			}
			break;
		case SongTagCode_filter:
			if (aBeforeThree) 
			{
				aChannel=Base64CharCodeToInt(theCompressed[aCharIndex++]);
				char aFilter[]={1, 3, 4, 5};
				mChannels[aChannel]->mInstruments[0]->mFilter=aFilter[Clip(0,BeepBoxConfig::mFilterNames_Length, Base64CharCodeToInt(theCompressed[aCharIndex++]))];
			} 
			else if (aBeforeSix) 
			{
				for (aChannel=0;aChannel<GetChannelCount();aChannel++) 
				{
					for (int aCount=0;aCount<mInstrumentsPerChannel;aCount++) 
					{
						mChannels[aChannel]->mInstruments[aCount]->mFilter=Clip(0,BeepBoxConfig::mFilterNames_Length,Base64CharCodeToInt(theCompressed[aCharIndex++])+1);
					}
				}
			}
			else 
			{
				if (aInstrumentIndexIterator<0) return false;
				mChannels[aInstrumentChannelIterator]->mInstruments[aInstrumentIndexIterator]->mFilter=Clip(0,BeepBoxConfig::mFilterNames_Length,Base64CharCodeToInt(theCompressed[aCharIndex++]));
			}
			break;
		case SongTagCode_transition:
			if (aBeforeThree) 
			{
				aChannel=Base64CharCodeToInt(theCompressed[aCharIndex++]);
				mChannels[aChannel]->mInstruments[0]->mTransition=Clip(0,BeepBoxConfig::mTransitionNames_Length,Base64CharCodeToInt(theCompressed[aCharIndex++]));
			} else if (aBeforeSix) 
			{
				for (aChannel=0;aChannel<GetChannelCount();aChannel++) 
				{
					for (int aCount=0;aCount<mInstrumentsPerChannel;aCount++) 
					{
						mChannels[aChannel]->mInstruments[aCount]->mTransition=Clip(0,BeepBoxConfig::mTransitionNames_Length,Base64CharCodeToInt(theCompressed[aCharIndex++]));
					}
				}
			} 
			else 
			{
				if (aInstrumentIndexIterator<0) return false;
				mChannels[aInstrumentChannelIterator]->mInstruments[aInstrumentIndexIterator]->mTransition=Clip(0,BeepBoxConfig::mTransitionNames_Length,Base64CharCodeToInt(theCompressed[aCharIndex++]));
			}
			break;
		case SongTagCode_effect:
			if (aBeforeThree) 
			{
				aChannel=Base64CharCodeToInt(theCompressed[aCharIndex++]);
				int aEffect=Clip(0, BeepBoxConfig::mEffectNames_Length,Base64CharCodeToInt(theCompressed[aCharIndex++]));
				if (aEffect==1) aEffect=3;
				else if (aEffect==3) aEffect=5;
				mChannels[aChannel]->mInstruments[0]->mEffect=aEffect;
			} 
			else if (aBeforeSix) 
			{
				for (aChannel=0;aChannel<GetChannelCount();aChannel++) 
				{
					for (int aCount=0;aCount<mInstrumentsPerChannel;aCount++) 
					{
						mChannels[aChannel]->mInstruments[aCount]->mEffect=Clip(0,BeepBoxConfig::mEffectNames_Length,Base64CharCodeToInt(theCompressed[aCharIndex++]));
					}
				}
			} 
			else 
			{
				if (aInstrumentIndexIterator<0) return false;
				mChannels[aInstrumentChannelIterator]->mInstruments[aInstrumentIndexIterator]->mEffect=Clip(0,BeepBoxConfig::mEffectNames_Length, Base64CharCodeToInt(theCompressed[aCharIndex++]));
			}
			break;
		case SongTagCode_chorus:
			if (aBeforeThree) 
			{
				aChannel=Base64CharCodeToInt(theCompressed[aCharIndex++]);
				mChannels[aChannel]->mInstruments[0]->mChorus=Clip(0,BeepBoxConfig::mChorusNames_Length,Base64CharCodeToInt(theCompressed[aCharIndex++]));
			} 
			else if (aBeforeSix) 
			{
				for (aChannel=0;aChannel<GetChannelCount();aChannel++) 
				{
					for (int aCount=0;aCount<mInstrumentsPerChannel;aCount++) 
					{
						mChannels[aChannel]->mInstruments[aCount]->mChorus=Clip(0,BeepBoxConfig::mChorusNames_Length, Base64CharCodeToInt(theCompressed[aCharIndex++]));
					}
				}
			} 
			else 
			{
				if (aInstrumentIndexIterator<0) return false;
				mChannels[aInstrumentChannelIterator]->mInstruments[aInstrumentIndexIterator]->mChorus=Clip(0,BeepBoxConfig::mChorusNames_Length,Base64CharCodeToInt(theCompressed[aCharIndex++]));
			}
			break;
		case SongTagCode_volume:
			if (aBeforeThree) 
			{
				aChannel=Base64CharCodeToInt(theCompressed[aCharIndex++]);
				mChannels[aChannel]->mInstruments[0]->mVolume=Clip(0,BeepBoxConfig::mVolumeNames_Length, Base64CharCodeToInt(theCompressed[aCharIndex++]));
			} 
			else if (aBeforeSix) 
			{
				for (aChannel=0;aChannel<GetChannelCount();aChannel++) 
				{
					for (int aCount=0;aCount<mInstrumentsPerChannel;aCount++) 
					{
						mChannels[aChannel]->mInstruments[aCount]->mVolume=Clip(0,BeepBoxConfig::mVolumeNames_Length,Base64CharCodeToInt(theCompressed[aCharIndex++]));
					}
				}
			}
			else 
			{
				if (aInstrumentIndexIterator<0) return false;
				mChannels[aInstrumentChannelIterator]->mInstruments[aInstrumentIndexIterator]->mVolume=Clip(0,BeepBoxConfig::mVolumeNames_Length,Base64CharCodeToInt(theCompressed[aCharIndex++]));
			}
			break;
		case SongTagCode_algorithm:
			if (aInstrumentIndexIterator<0) return false;
			mChannels[aInstrumentChannelIterator]->mInstruments[aInstrumentIndexIterator]->mAlgorithm=Clip(0,BeepBoxConfig::mOperatorAlgorithmNames_Length, Base64CharCodeToInt(theCompressed[aCharIndex++]));
			break;
		case SongTagCode_feedbackType:
			if (aInstrumentIndexIterator<0) return false;
			mChannels[aInstrumentChannelIterator]->mInstruments[aInstrumentIndexIterator]->mFeedbackType=Clip(0,BeepBoxConfig::mOperatorFeedbackNames_Length, Base64CharCodeToInt(theCompressed[aCharIndex++]));
			break;
		case SongTagCode_feedbackAmplitude:
			if (aInstrumentIndexIterator<0) return false;
			mChannels[aInstrumentChannelIterator]->mInstruments[aInstrumentIndexIterator]->mFeedbackAmplitude=Clip(0,BeepBoxConfig::mOperatorAmplitudeMax+1, Base64CharCodeToInt(theCompressed[aCharIndex++]));
			break;
		case SongTagCode_feedbackEnvelope:
			if (aInstrumentIndexIterator<0) return false;
			mChannels[aInstrumentChannelIterator]->mInstruments[aInstrumentIndexIterator]->mFeedbackEnvelope=Clip(0,BeepBoxConfig::mOperatorEnvelopeNames_Length, Base64CharCodeToInt(theCompressed[aCharIndex++]));
			break;
		case SongTagCode_operatorFrequencies:
			if (aInstrumentIndexIterator<0) return false;
			for (int aCount=0;aCount<BeepBoxConfig::mOperatorCount;aCount++) mChannels[aInstrumentChannelIterator]->mInstruments[aInstrumentIndexIterator]->mOperators[aCount]->mFrequency=Clip(0, BeepBoxConfig::mOperatorFrequencyNames_Length, Base64CharCodeToInt(theCompressed[aCharIndex++]));
			break;
		case SongTagCode_operatorAmplitudes:
			if (aInstrumentIndexIterator<0) return false;
			for (int aCount=0;aCount<BeepBoxConfig::mOperatorCount;aCount++) mChannels[aInstrumentChannelIterator]->mInstruments[aInstrumentIndexIterator]->mOperators[aCount]->mAmplitude=Clip(0,BeepBoxConfig::mOperatorAmplitudeMax+1,Base64CharCodeToInt(theCompressed[aCharIndex++]));
			break;
		case SongTagCode_operatorEnvelopes:
			if (aInstrumentIndexIterator<0) return false;
			for (int aCount=0;aCount<BeepBoxConfig::mOperatorCount;aCount++) mChannels[aInstrumentChannelIterator]->mInstruments[aInstrumentIndexIterator]->mOperators[aCount]->mEnvelope=Clip(0, BeepBoxConfig::mOperatorEnvelopeNames_Length, Base64CharCodeToInt(theCompressed[aCharIndex++]));
			break;
		case SongTagCode_bars:
			{
				int aSubStringLength=0;
				if (aBeforeThree) 
				{
					aChannel=Base64CharCodeToInt(theCompressed[aCharIndex++]);
					int aBarCount=Base64CharCodeToInt(theCompressed[aCharIndex++]);
					aSubStringLength=(int)(ceil((float)aBarCount*0.5f));

					BitFieldReader aBits(theCompressed,aCharIndex,aCharIndex+aSubStringLength);
					for (int i=0;i<aBarCount;i++) 
					{
						mChannels[aChannel]->mBars.Set(i)=aBits.Read(3)+1;
						//gOut.Out("Channel[%d]->mBars[%d]=%d",aChannel,i,mChannels[aChannel]->mBars[i]);
					}
				} 
				else if (aBeforeFive) 
				{
					int aNeededBits=0;
					while ((1<<aNeededBits)<mPatternsPerChannel) aNeededBits++;
					aSubStringLength=(int)ceil((float)(GetChannelCount()*mBarCount*aNeededBits)/6.0f);
					BitFieldReader aBits(theCompressed,aCharIndex,aCharIndex+aSubStringLength);
					for (aChannel=0;aChannel<GetChannelCount();aChannel++) for (int i=0;i<mBarCount;i++) 
					{
						mChannels[aChannel]->mBars.Set(i)=aBits.Read(aNeededBits)+1;
						//gOut.Out("Channel[%d]->mBars[%d]=%d",aChannel,i,mChannels[aChannel]->mBars[i]);
					}
				} 
				else 
				{
					int aNeededBits=0;
					while ((1<<aNeededBits)<mPatternsPerChannel+1) aNeededBits++;
					aSubStringLength=(int)ceil((float)(GetChannelCount()*mBarCount*aNeededBits)/6.0f);

					BitFieldReader aBits(theCompressed,aCharIndex,aCharIndex+aSubStringLength);
					for (aChannel=0;aChannel<GetChannelCount();aChannel++) for (int i=0;i<mBarCount;i++) 
					{
						mChannels[aChannel]->mBars.Set(i)=aBits.Read(aNeededBits);
						//gOut.Out("Channel[%d]->mBars[%d]=%d",aChannel,i,mChannels[aChannel]->mBars[i]);
					}
				}
				aCharIndex+=aSubStringLength;
			}
			break;
		case SongTagCode_patterns:
			{
				int aBitStringLength=0;
				if (aBeforeThree) 
				{
					aChannel=Base64CharCodeToInt(theCompressed[aCharIndex++]);
					aCharIndex++;
					aBitStringLength=Base64CharCodeToInt(theCompressed[aCharIndex++]);
					aBitStringLength=aBitStringLength << 6;
					aBitStringLength+=Base64CharCodeToInt(theCompressed[aCharIndex++]);
				} 
				else 
				{
					aChannel=0;
					int aBitStringLengthLength=Base64CharCodeToInt(theCompressed[aCharIndex++]);
					while (aBitStringLengthLength>0) 
					{
						aBitStringLength=aBitStringLength<<6;
						aBitStringLength+=Base64CharCodeToInt(theCompressed[aCharIndex++]);
						aBitStringLengthLength--;
					}
				}

				BitFieldReader aBits(theCompressed,aCharIndex,aCharIndex+aBitStringLength);
				aCharIndex+=aBitStringLength;

				int aNeededInstrumentBits=0;
				while ((1<<aNeededInstrumentBits) < mInstrumentsPerChannel) aNeededInstrumentBits++;
				while (true) 
				{
					bool aIsDrum=GetChannelIsDrum(aChannel);
					int aOctaveOffset=aIsDrum ? 0 : mChannels[aChannel]->mOctave*12;
					Smart(Note) aNote=NULL;
					Smart(NotePin) aPin=NULL;
					float aLastPitch=(float)((aIsDrum?4:12)+aOctaveOffset);
	#ifdef _TRACE
					gTrace.Out("IsDrum:%s OctaveOffset: %d lastPitch: %f",aIsDrum?"true":"false",aOctaveOffset,aLastPitch);
	#endif

					//const recentPitches: number[] = isDrum ? [4,6,7,2,3,8,0,10] : [12, 19, 24, 31, 36, 7, 0];
					float aDrumPitches[]={4,6,7,2,3,8,0,10,-1};
					float aNotDrumPitches[]={12,19,24,31,36,7,0,-1};
					Array<float> aRecentPitches;
					if (aIsDrum) for (int aCount=0;;aCount++) {if (aDrumPitches[aCount]<0) break;aRecentPitches+=aDrumPitches[aCount];}
					else for (int aCount=0;;aCount++) {if (aNotDrumPitches[aCount]<0) break;aRecentPitches+=aNotDrumPitches[aCount];}

					//
					// So the original TypeScript said "any" ... when I changed it to "number" it seemed to work fine too.
					// BUT: I think this possibly should hold floats.  If any trouble, come back and make it floats!
					//

					struct Any
					{
						int mPitchCount;
						int mPinCount;
						float mInitialVolume;
						SmartList(Any) mPins;
						int mLength;
						int mBendCount;
						bool mPitchBend;
						int mTime;
						float mVolume;
					};
					#define any Smart(Any)
					SmartList(Any) aRecentShapes;	// const recentShapes: any[] = [];

					for (int i=0;i<aRecentPitches.Size();i++) aRecentPitches[i]+=aOctaveOffset;

					for (int i=0;i<mPatternsPerChannel;i++) 
					{
						Smart(Pattern) aNewPattern=mChannels[aChannel]->mPatterns[i];
						aNewPattern->Reset();
						aNewPattern->mInstrument=aBits.Read(aNeededInstrumentBits);

						if (!aBeforeThree && aBits.Read(1)==0) continue;

						int aCurPart=0;
						SmartList(Note)& aNewNotes=aNewPattern->mNotes;//const newNotes: Note[] = newPattern.notes;

						while (aCurPart<mBeatsPerBar*mPartsPerBeat) 
						{
							bool aUseOldShape=(aBits.Read(1)==1);
							bool aNewNote=false;
							int aShapeIndex=0;

							if (aUseOldShape) 
							{
								aShapeIndex=aBits.ReadLongTail(0,0);
							}
							else aNewNote=(aBits.Read(1)==1);
							if (!aUseOldShape && !aNewNote) 
							{
								int aRestLength=aBits.ReadPartDuration();
								aCurPart+=aRestLength;
							} 
							else 
							{
								any aShape;
								any aPinObj;
								float aPitch;

								if (aUseOldShape) 
								{
									aShape=aRecentShapes[aShapeIndex];
									aRecentShapes.Remove(aShapeIndex); //recentShapes.splice(shapeIndex, 1);
								} 
								else 
								{
									aShape=new Any;
									aShape->mPitchCount=1;
									while (aShape->mPitchCount<4 && aBits.Read(1)==1) aShape->mPitchCount++;
									aShape->mPinCount=aBits.ReadPinCount();
									aShape->mInitialVolume=(float)aBits.Read(2);
									aShape->mPins.Clear(); //shape.pins = [];
									aShape->mLength=0;
									aShape->mBendCount=0;

									for (int j=0;j<aShape->mPinCount;j++) 
									{
										aPinObj=new Any;
										aPinObj->mPitchBend=aBits.Read(1)==1;
										if (aPinObj->mPitchBend) aShape->mBendCount++;
										aShape->mLength+=aBits.ReadPartDuration();
										aPinObj->mTime=aShape->mLength;
										aPinObj->mVolume=(float)aBits.Read(2);
										aShape->mPins+=aPinObj;
									}
								}
								aRecentShapes.Insert(aShape,0);//	recentShapes.unshift(shape);
								if (aRecentShapes.GetCount()>10) aRecentShapes.Remove(aRecentShapes.GetCount()-1);
								aNote=MakeNote(0,aCurPart,aCurPart+aShape->mLength,aShape->mInitialVolume);
								aNote->mPitches.Reset(); //note.pitches = [];
								while (aNote->mPins.GetCount()>1) aNote->mPins.Remove(aNote->mPins.GetCount()-1); //note.pins.length = 1;

								Array<float> aPitchBends;

								for (int j=0;j<aShape->mPitchCount+aShape->mBendCount;j++) 
								{
									bool aUseOldPitch=(aBits.Read(1)==1);

									if (!aUseOldPitch) 
									{
										int aInterval=aBits.ReadPitchInterval();
										aPitch=aLastPitch;
										int aIntervalIter=aInterval;
										while (aIntervalIter>0) 
										{
											aPitch++;
											while (aRecentPitches.FindValue(aPitch)!=-1) aPitch++;
											aIntervalIter--;
										}
										while (aIntervalIter<0)
										{
											aPitch--;
											while (aRecentPitches.FindValue(aPitch)!=-1) aPitch--;
											aIntervalIter++;
										}
									}
									else 
									{
										int aPitchIndex=aBits.Read(3);
										aPitch=aRecentPitches[aPitchIndex];
										aRecentPitches.DeleteElement(aPitchIndex); //recentPitches.splice(pitchIndex, 1);
									}

									aRecentPitches.InsertElement(aPitch,0);	//aRecentPitches.unshift(pitch); 
									if (aRecentPitches.Size()>8) aRecentPitches.DeleteElement(aRecentPitches.Size()-1);

									if (j<aShape->mPitchCount) aNote->mPitches+=aPitch;
									else aPitchBends+=aPitch;

									if (j==aShape->mPitchCount-1) aLastPitch=aNote->mPitches[0];
									else aLastPitch=aPitch;
								}
								aPitchBends.InsertElement(aNote->mPitches[0]); //pitchBends.unshift(note.pitches[0]); 


								EnumSmartList(Any,aPinObj,aShape->mPins) //for (const pinObj of shape.pins) {
								{
									if (aPinObj->mPitchBend) aPitchBends.DeleteElement(0);

									aPin=MakeNotePin((int)(aPitchBends[0]-aNote->mPitches[0]),aPinObj->mTime,aPinObj->mVolume);
									aNote->mPins+=aPin;
								}
								aCurPart=aNote->mEnd;
								aNewNotes+=aNote;

	#ifdef _TRACE
								gTrace.Out("Note: %d - %d",aNote->mStart,aNote->mEnd);
	#endif
								String aOutStr;
								for (int aCount=0;aCount<aNote->mPitches.Size();aCount++) aOutStr+=Sprintf("%f ",aNote->mPitches[aCount]);
	#ifdef _TRACE
								gTrace.Out("Note.pitches = %s",aOutStr.c());
	#endif
								aOutStr="";
								EnumSmartList(NotePin,aNP,aNote->mPins) aOutStr+=Sprintf("(%d,%d,%f)",aNP->mInterval,aNP->mTime,aNP->mVolume);
	#ifdef _TRACE
								gTrace.Out("Note.pins = %s",aOutStr.c());
	#endif
							}
						}
					}

					if (aBeforeThree) break;
					else 
					{
						aChannel++;
						if (aChannel>=GetChannelCount()) break;
					}
				}
			}
			break;
		default:
			_DLOG("Song Load error [%c]: --> %s <--",aCommand,theCompressed.c());
			return false;
			break;
		}
	}
	return true;
}


void BeepBoxCallback(short* theBuffer, unsigned int theLength, void* extraData)
{
	ThreadLock(gSynthesizerThreadLock);
	BeepBox* aBB=(BeepBox*)extraData;
	if (aBB->mSong.IsNotNull()) aBB->Synthesize(theBuffer,theLength/2);
	ThreadUnlock(gSynthesizerThreadLock);
}

BeepBox::BeepBox()
{
	mDidConstruct=false;
}

BeepBox::~BeepBox()
{
	if (IsPlaying())
	{
		ThreadLock(gSynthesizerThreadLock);
		Stop();
		ThreadUnlock(gSynthesizerThreadLock);
		Sleep(10);	// Allow the sound controller thread to process...
	}
	if (gAppPtr && !gAppPtr->IsQuit())
	{
		ThreadLock(gSynthesizerThreadLock);
		mSong=NULL;
		ThreadUnlock(gSynthesizerThreadLock);
	}
}

void BeepBox::Load(String theSong) 
{
	SoundStreamDynamic::Stop();
	Construct();
	ThreadLock(gSynthesizerThreadLock);
	mSong=new BeepBoxSong;
	if (!mSong->Load(theSong)) mSong=NULL;
	ThreadUnlock(gSynthesizerThreadLock);
	PrepareToPlay();
}

void BeepBox::Unload() 
{
	ThreadLock(gSynthesizerThreadLock);
	Stop();
	mSong=NULL;
	ThreadUnlock(gSynthesizerThreadLock);
}



void BeepBox::PrepareToPlay()
{
	if (!mDidConstruct) Construct();
	if (gSynthesizerThreadLock==-1) gSynthesizerThreadLock=CreateThreadLock();

	if (!mPaused) return;
	if (!mSong) return;
	mPaused=false;
	SoundStreamDynamic::Load(BeepBoxCallback,this,2048);
	WarmUpSynthesizer(mSong);
}

void BeepBox::WarmUpSynthesizer(Smart(BeepBoxSong) theSong)
{
	if (!mSong) return;
	//
	// Make all the synth channels...
	//
	mChannels.Clear();for (int i=mChannels.GetCount();i<mSong->GetChannelCount();i++) mChannels+=new BeepBoxChannel();

	//
	// Okay, we don't generate a synthesizer here.
	// The reason is, BeepBox does this for javascript speed... but in C++ we won't need that!
	//
		
	for (int aBar=0;aBar<mSong->mBarCount;aBar++) for (int aChannel=0;aChannel<mSong->mPitchChannelCount+mSong->mDrumChannelCount;aChannel++) mInstrumentType.Set(aChannel).Set(aBar)=InstrumentType_unknown;

	for (int aBar=0;aBar<mSong->mBarCount;aBar++) 
	{
		for (int aChannel=0;aChannel<mSong->mPitchChannelCount+mSong->mDrumChannelCount;aChannel++)
		{
			if (aChannel<mSong->mPitchChannelCount) mInstrumentType.Set(aChannel).Set(aBar)=mSong->mChannels[aChannel]->mInstruments[mSong->GetPatternInstrument(aChannel,mBar)]->mType;
			else mInstrumentType.Set(aChannel).Set(aBar)=InstrumentType_noise;
			mInstruments.Set(aChannel).Set(aBar)=mSong->mChannels[aChannel]->mInstruments[mSong->GetPatternInstrument(aChannel,aBar)];
			//gOut.Out("Instruments[%d][%d]=%d",aChannel,aBar,mInstrumentType[aChannel][aBar]);
		}
	}

	for (int i=0;i<mSong->mInstrumentsPerChannel;i++)
	{
		for (int j=mSong->mPitchChannelCount;j<mSong->mPitchChannelCount+mSong->mDrumChannelCount;j++)
		{
			gBeepboxConfig->GetDrumWave(mSong->mChannels[j]->mInstruments[i]->mWave);
		}
	}


}

int BeepBox::GetSamplesPerArpeggio()
{
	if (mSong.IsNull()) return 0;
	float aBeatsPerMinute=gMath.Round(120.0f*(float)pow(2.0f,(-4.0f+mSong->mTempo)/9.0f)); //mSong->GetBeatsPerMinute();
	float aBeatsPerSecond=aBeatsPerMinute/60.0f;
	float aPartsPerSecond=aBeatsPerSecond*mSong->mPartsPerBeat;
	float aArpeggioPerSecond=aPartsPerSecond*4.0f;
	float aResult=(mSamplesPerSecond/aArpeggioPerSecond);
	return (int)floor(aResult);
}

void BeepBox::Synthesize(short* theBuffer, unsigned int theBufferLength)
{
	if (mSong.IsNull()) {memset(theBuffer,0,theBufferLength);return;}

	/*
	//
	// Test code... just plays some crazy sin wave
	//
	static float aValue=0;
	static float aPlus=1.0f;
	for (unsigned int aCount=0;aCount<theBufferLength/sizeof(float);aCount++)
	{
		*theBuffer=gMath.Sin(aValue);
		aValue+=aPlus;
		theBuffer++;
	}
	aPlus+=.1f;
	return;
	/**/

	int aSamplesPerArpeggio=GetSamplesPerArpeggio();
	int aBufferIndex=0;
	bool aEnded=false;
	
	if (mArpeggioSampleCountdown==0 || mArpeggioSampleCountdown>aSamplesPerArpeggio) mArpeggioSampleCountdown=aSamplesPerArpeggio;

	if (mPart>=mSong->mPartsPerBeat) 
	{
		mBeat++;
		mPart=0;
		mArpeggio=0;
		mArpeggioSampleCountdown=aSamplesPerArpeggio;
	}

	if (mBeat>=mSong->mBeatsPerBar) 
	{
		mBar++;
		mBeat=0;
		mPart=0;
		mArpeggio=0;
		mArpeggioSampleCountdown=aSamplesPerArpeggio;

		if (mLoopCount==-1) 
		{
			if (mBar<mSong->mLoopStart && !mEnableIntro) mBar=mSong->mLoopStart;
			if (mBar>=mSong->mLoopStart + mSong->mLoopLength && !mEnableOutro) mBar=mSong->mLoopStart;
		}
	}

	if (mBar>=mSong->mBarCount) 
	{
		if (mEnableOutro) 
		{
			mBar=0;
			mEnableIntro=true;
			aEnded=true;
			Stop();
		} 
		else mBar=mSong->mLoopStart;
	}
	if (mBar>=mSong->mLoopStart) mEnableIntro=false;

	while (true) 
	{
		if (aEnded) {memset(theBuffer,0,theBufferLength);break;}

		//const generatedSynthesizer: Function = Synth.getGeneratedSynthesizer(this.song, this.bar);
		//bufferIndex = generatedSynthesizer(this, this.song, data, bufferLength, bufferIndex, samplesPerArpeggio);
		aBufferIndex=SynthesizeCore(theBuffer,(int)theBufferLength,aBufferIndex,aSamplesPerArpeggio);

		bool aFinishedBuffer=(aBufferIndex==-1);
		if (aFinishedBuffer) 
		{
			break;
		}
		else
		{
			// bar changed, reset for next bar:
			mBeat=0;
			mEffectPhase=0.0f;
			mBar++;

			if (mBar<mSong->mLoopStart) {if (!mEnableIntro) {mBar=mSong->mLoopStart;}}
			else mEnableIntro=false;

			if (mBar>=mSong->mLoopStart+mSong->mLoopLength) 
			{
				if (mLoopCount>0) mLoopCount--;
				if (mLoopCount>0 || !mEnableOutro) {mBar=mSong->mLoopStart;}
			}
			if (mBar>=mSong->mBarCount) 
			{
				mBar = 0;
				mEnableIntro=true;
				aEnded=true;
				Stop();
			}
		}
	}

	mPlayheadInternal=(((mArpeggio+1.0f-mArpeggioSampleCountdown/aSamplesPerArpeggio)/4.0f+mPart)/mSong->mPartsPerBeat+mBeat)/mSong->mBeatsPerBar+mBar;
}


// Key things to look for:
//				// ALL : For all channels
//				// PITCH : All channels < mSongs->mPitchChannelCount
//				// CHIP : All channels before mSong->mPitchChannelCount
//				// NOISE All channels >= mSong->mPitchChannelCount
//
//				// FM does something
//				// CARRIER OUTPUTS does something
//				// INSERT OPERATOR COMPUTATION HERE will probably be the nasty one.
// 
//
// Random porting tips: 
//			"+x" in Javascript means coerce x to float. 
//			"x|0" means coerce to int, dropping the fractional component. 
//			"x%1" means keep only the fractional component!
//


#define ISCHIP(channel) (mInstrumentType[channel][mBar]==InstrumentType_chip)
#define ISNOISE(channel) (channel>=mSong->mPitchChannelCount)
#define ISFM(channel) (mInstrumentType[channel][mBar]==InstrumentType_fm)
#define ISPITCH(channel) (channel<mSong->mPitchChannelCount)

/*
static int gDebugCounter=0;
static int gDebugSampleCounter;

String ArrayToString(Array<float>* theArray) {String aStr;for (int aCount=0;aCount<theArray->Size();aCount++) aStr+=Sprintf("%f,",theArray->Element(aCount));return aStr;}
String ArrayToString(Array<double>* theArray) {String aStr;for (int aCount=0;aCount<theArray->Size();aCount++) aStr+=Sprintf("%f,",theArray->Element(aCount));return aStr.GetSegment(0,100);}
void BeepBox::Out(char* format, ...) 
{	
	if (gDebugCounter<3)
	{
		String aString;
		va_list argp;
		va_start(argp, format);
		aString=Sprintf(format,argp);
		va_end(argp);
		gOut.Out(aString.c());
	}
}
*/

int BeepBox::SynthesizeCore(short* theBuffer, int theBufferLength, int theBufferIndex, int aSamplesPerArpeggio)
{
	//
	// All these variables appear to change only when the song is changed (IF then)
	// and thus don't really need to be here...
	//
	static float aSampleTime;
	static float aEffectYMult;
	static float aLimitDecay;
	static float aVolume;
	static float aReverb;

	theBuffer+=theBufferIndex;

	aSampleTime = 1.0f / mSamplesPerSecond;
	aEffectYMult = mEffectYMult;
	aLimitDecay=mLimitDecay;
	aVolume=mVolume;
	aReverb = (float)pow((float)mSong->mReverb / (float)gBeepboxConfig->mReverbRange, 0.667f) * 0.425f;

	//
	// Initialize instruments based on current pattern.
	//
	// Some of these are probably unnecessary-- some of them are in fact just temporary
	// variables that don't need to be set up in an array.  But this doesn't really slow things
	// down, and the memory is negligible really.  
	//
	static int aInstrumentChannel[BeepBoxConfig::mChannelCount];
	static BeepBoxSong::Instrument* aInstrument[BeepBoxConfig::mChannelCount];
	static ReadOnlyArray<double>* aChannelWave[BeepBoxConfig::mChannelCount];
	static int aChannelWaveLength[BeepBoxConfig::mChannelCount];
	static float aChannelFilterBase[BeepBoxConfig::mChannelCount];
	static float aChannelTremoloScale[BeepBoxConfig::mChannelCount];

	for (int aChannel=0;aChannel<mSong->mPitchChannelCount+mSong->mDrumChannelCount;aChannel++) 
	{
		// FINISHME: We are storing mInstruments which duplicates the info.  We could eliminate this lookup, yes?
		aInstrumentChannel[aChannel]=mSong->GetPatternInstrument(aChannel,mBar); // ALL

		aInstrument[aChannel]=mSong->mChannels[aChannel]->mInstruments[aInstrumentChannel[aChannel]]; // ALL

		if (ISCHIP(aChannel))
		{
			aChannelWave[aChannel]=&gBeepboxConfig->mWaves[aInstrument[aChannel]->mWave]; // CHIP
			aChannelWaveLength[aChannel]=aChannelWave[aChannel]->Size(); // CHIP
			aChannelFilterBase[aChannel]=(float)pow(2.0f,-gBeepboxConfig->mFilterBases[aInstrument[aChannel]->mFilter]); // CHIP
		}
		if (ISNOISE(aChannel)) 
		{
			aChannelWave[aChannel]=&gBeepboxConfig->GetDrumWave(aInstrument[aChannel]->mWave); // NOISE
		}
		if (ISPITCH(aChannel)) 
		{
			aChannelTremoloScale[aChannel]=gBeepboxConfig->mEffectTremolos[aInstrument[aChannel]->mEffect]; // PITCH
		}
	}

	static Smart(BeepBoxChannel) aSynthChannel[BeepBoxConfig::mChannelCount];
	static float aChannelChorusA[BeepBoxConfig::mChannelCount];
	static float aChannelChorusB[BeepBoxConfig::mChannelCount];
	static float aChannelChorusSign[BeepBoxConfig::mChannelCount];
	static float aChannelChorusDeltaRatio[BeepBoxConfig::mChannelCount];
	static float aChannelPhase[BeepBoxConfig::mChannelCount];
	static float aChannelPhaseA[BeepBoxConfig::mChannelCount];
	static float aChannelPhaseB[BeepBoxConfig::mChannelCount];
	static float aChannelPhaseDelta[BeepBoxConfig::mChannelCount];
	static float aChannelPhaseDeltaScale[BeepBoxConfig::mChannelCount];
	static float aChannelVolume[BeepBoxConfig::mChannelCount];
	static float aChannelVolumeDelta[BeepBoxConfig::mChannelCount];
	static float aChannelFilter[BeepBoxConfig::mChannelCount];
	static float aChannelFilterScale[BeepBoxConfig::mChannelCount];
	static float aChannelVibratoScale[BeepBoxConfig::mChannelCount];
	static float aChannelOperatorPhase[BeepBoxConfig::mChannelCount][BeepBoxConfig::mOperatorCount];
	static float aChannelOperatorPhaseDelta[BeepBoxConfig::mChannelCount][BeepBoxConfig::mOperatorCount];
	static float aChannelOperatorOutputMult[BeepBoxConfig::mChannelCount][BeepBoxConfig::mOperatorCount];
	static float aChannelOperatorOutputDelta[BeepBoxConfig::mChannelCount][BeepBoxConfig::mOperatorCount];
	static float aChannelOperatorOutput[BeepBoxConfig::mChannelCount][BeepBoxConfig::mOperatorCount];
	static float aChannelOperatorScaled[BeepBoxConfig::mChannelCount][BeepBoxConfig::mOperatorCount];


/*
	static Array<Smart(BeepBoxChannel)> aSynthChannel;
	static Array<float> aChannelChorusA;
	static Array<float> aChannelChorusB;
	static Array<float> aChannelChorusSign;
	static Array<float> aChannelChorusDeltaRatio;
	static Array<float> aChannelPhase;
	static Array<float> aChannelPhaseA;
	static Array<float> aChannelPhaseB;
	static Array<float> aChannelPhaseDelta;
	static Array<float> aChannelPhaseDeltaScale;
	static Array<float> aChannelVolume;
	static Array<float> aChannelVolumeDelta;
	static Array<float> aChannelFilter;
	static Array<float> aChannelFilterScale;
	static Array<float> aChannelVibratoScale;
	static Array<Array<float> > aChannelOperatorPhase;
	static Array<Array<float> > aChannelOperatorPhaseDelta;
	static Array<Array<float> > aChannelOperatorOutputMult;
	static Array<Array<float> > aChannelOperatorOutputDelta;
	static Array<Array<float> > aChannelOperatorOutput;
	static Array<Array<float> > aChannelOperatorScaled;
*/

/*
	if (aChannelOperatorPhase.Size()==0) for (int aCount=9;aCount>=0;aCount--) 
	{
		aChannelOperatorPhase[aCount].GuaranteeSize(0);
		aChannelOperatorPhaseDelta[aCount].GuaranteeSize(0);
		aChannelOperatorOutputMult[aCount].GuaranteeSize(0);
		aChannelOperatorOutputDelta[aCount].GuaranteeSize(0);
		aChannelOperatorOutput[aCount].GuaranteeSize(0);
		aChannelOperatorScaled[aCount].GuaranteeSize(0);
	}
*/


/*
	static Array<float> aChannelFeedbackMult;
	static Array<float> aChannelFeedbackDelta;
	static Array<float> aChannelSample;
	static Array<float> aChannelVibrato;
	static Array<float> aChannelTremolo;
*/

	static float aChannelFeedbackMult[BeepBoxConfig::mChannelCount];
	static float aChannelFeedbackDelta[BeepBoxConfig::mChannelCount];
	static float aChannelSample[BeepBoxConfig::mChannelCount];
	static float aChannelVibrato[BeepBoxConfig::mChannelCount];
	static float aChannelTremolo[BeepBoxConfig::mChannelCount];

	int gDebugLoopCounter=0;
	while (theBufferIndex<theBufferLength)
	{
		static int aSamples;
		static int aSamplesLeftInBuffer;

		//aSamples;
		aSamplesLeftInBuffer=theBufferLength-theBufferIndex;

		if (mArpeggioSampleCountdown<=aSamplesLeftInBuffer) aSamples=mArpeggioSampleCountdown;
		else aSamples=aSamplesLeftInBuffer;
		mArpeggioSampleCountdown-=aSamples;

		static int aTime;
		static float aEffectY;
		static float aPrevEffectY;

		aTime=mPart+mBeat*mSong->mPartsPerBeat;
		aEffectY=(float)sin(mEffectPhase);
		aPrevEffectY=(float)sin(mEffectPhase-mEffectAngle);

		//
		// BOOKMARK 1
		//
		for (int aChannel=0;aChannel<mSong->mPitchChannelCount+mSong->mDrumChannelCount;aChannel++) 
		{
			ComputeChannelInstrument(aChannel,aTime,aSampleTime,aSamplesPerArpeggio,aSamples); // ALL
			aSynthChannel[aChannel]=mChannels[aChannel]; // ALL

			if (ISCHIP(aChannel))
			{
				aChannelChorusA[aChannel]=(float)pow(2.0f, (gBeepboxConfig->mChorusOffsets[aInstrument[aChannel]->mChorus]+gBeepboxConfig->mChorusIntervals[aInstrument[aChannel]->mChorus]) / 12.0f); // CHIP
				aChannelChorusB[aChannel]=(float)pow(2.0f, (gBeepboxConfig->mChorusOffsets[aInstrument[aChannel]->mChorus]-gBeepboxConfig->mChorusIntervals[aInstrument[aChannel]->mChorus]) / 12.0f); // CHIP
				aChannelChorusSign[aChannel]=aSynthChannel[aChannel]->mHarmonyVolumeMult*gBeepboxConfig->mChorusSigns[aInstrument[aChannel]->mChorus]; // CHIP
				if (aInstrument[aChannel]->mChorus==0) aSynthChannel[aChannel]->mPhases[1]=aSynthChannel[aChannel]->mPhases[0]; // CHIP
				aChannelChorusB[aChannel]*=aSynthChannel[aChannel]->mHarmonyMult; // CHIP
				aChannelChorusDeltaRatio[aChannel]=aChannelChorusB[aChannel]/aChannelChorusA[aChannel]; // CHIP
				aChannelPhaseDelta[aChannel]=aSynthChannel[aChannel]->mPhaseDeltas[0]*aChannelChorusA[aChannel]; // CHIP
				aChannelVolume[aChannel]=aSynthChannel[aChannel]->mVolumeStarts[0]; // CHIP
				aChannelVolumeDelta[aChannel]=aSynthChannel[aChannel]->mVolumeDeltas[0]; // CHIP
				aChannelFilter[aChannel]=aSynthChannel[aChannel]->mFilter*aChannelFilterBase[aChannel]; // CHIP
				aChannelFilterScale[aChannel]=aSynthChannel[aChannel]->mFilterScale; // CHIP
				aChannelPhaseA[aChannel]=gMath.Decimal(mChannels[aChannel]->mPhases[0]); // CHIP
				aChannelPhaseB[aChannel]=gMath.Decimal(mChannels[aChannel]->mPhases[1]); // CHIP
			}
			if (ISNOISE(aChannel)) 
			{
				aChannelPhaseDelta[aChannel]=aSynthChannel[aChannel]->mPhaseDeltas[0] / 32768.0f; // NOISE
				aChannelVolume[aChannel]=aSynthChannel[aChannel]->mVolumeStarts[0]; // NOISE
				aChannelVolumeDelta[aChannel]=aSynthChannel[aChannel]->mVolumeDeltas[0]; // NOISE
				aChannelFilter[aChannel]=aSynthChannel[aChannel]->mFilter; // NOISE
				aChannelPhase[aChannel]=gMath.Decimal(mChannels[aChannel]->mPhases[0]); // NOISE
			}
			if (ISPITCH(aChannel)) aChannelVibratoScale[aChannel]=aSynthChannel[aChannel]->mVibratoScale; // PITCH
			aChannelPhaseDeltaScale[aChannel]=aSynthChannel[aChannel]->mPhaseDeltaScale; // ALL
			if (ISFM(aChannel))
			{
				for (int j=0;j<gBeepboxConfig->mOperatorCount;j++)
				{
					aChannelOperatorPhase[aChannel][j]       = ((gMath.Decimal(mChannels[aChannel]->mPhases[j]))+(float)gBeepboxConfig->mNegativePhaseGuard)*(float)gBeepboxConfig->mSineWaveLength; // FM
					aChannelOperatorPhaseDelta[aChannel][j]  = aSynthChannel[aChannel]->mPhaseDeltas[j]; // FM
					aChannelOperatorOutputMult[aChannel][j]  = aSynthChannel[aChannel]->mVolumeStarts[j]; // FM
					aChannelOperatorOutputDelta[aChannel][j] = aSynthChannel[aChannel]->mVolumeDeltas[j]; // FM
					aChannelOperatorOutput[aChannel][j]      = aSynthChannel[aChannel]->mFeedbackOutputs[j]; // FM
				}
				aChannelFeedbackMult[aChannel]         = aSynthChannel[aChannel]->mFeedbackMult; // FM
				aChannelFeedbackDelta[aChannel]        = aSynthChannel[aChannel]->mFeedbackDelta; // FM	
			}
			aChannelSample[aChannel]=(float)mChannels[aChannel]->mSample; // ALL
		}

		static int aDelayPos;
		static float aDelayFeedback0;
		static float aDelayFeedback1;
		static float aDelayFeedback2;
		static float aDelayFeedback3;
		static float aLimit;

		aDelayPos=(int)mDelayPos;
		aDelayFeedback0=mDelayFeedback0;
		aDelayFeedback1=mDelayFeedback1;
		aDelayFeedback2=mDelayFeedback2;
		aDelayFeedback3=mDelayFeedback3;
		aLimit=mLimit;

		//
		// BOOKMARK 2
		//

		while (aSamples) 
		{
			for (int aChannel=0;aChannel<mSong->mPitchChannelCount+mSong->mDrumChannelCount;aChannel++) 
			{
				if (ISPITCH(aChannel))
				{
					aChannelVibrato[aChannel]=1.0f+aChannelVibratoScale[aChannel]*(float)aEffectY; // PITCH
					aChannelTremolo[aChannel]=1.0f+aChannelTremoloScale[aChannel]*(float)(aEffectY-1.0f); // PITCH
				}
			}	
			static float aTemp;
			aTemp=aEffectY;
			aEffectY=aEffectYMult*aEffectY-aPrevEffectY;
			aPrevEffectY=aTemp;

			//
			// BOOKMARK 3
			//
			for (int aChannel=0;aChannel<mSong->mPitchChannelCount+mSong->mDrumChannelCount;aChannel++) 
			{
				if (ISCHIP(aChannel))
				{
					//aChannelSample[aChannel] += (float)((aChannelWave[aChannel] [(int)(aChannelPhaseA[aChannel]*aChannelWaveLength[aChannel])]+aChannelWave[aChannel][(int)(aChannelPhaseB[aChannel]*aChannelWaveLength[aChannel])]

					aChannelSample[aChannel] += (float)((aChannelWave[aChannel]->Element((int)(aChannelPhaseA[aChannel]*aChannelWaveLength[aChannel]))+aChannelWave[aChannel]->Element((int)(aChannelPhaseB[aChannel]*aChannelWaveLength[aChannel]))*aChannelChorusSign[aChannel])*aChannelVolume[aChannel]*aChannelTremolo[aChannel]-aChannelSample[aChannel])*aChannelFilter[aChannel]; // CHIP
					aChannelVolume[aChannel] += aChannelVolumeDelta[aChannel]; // CHIP
					aChannelPhaseA[aChannel] += aChannelPhaseDelta[aChannel]*aChannelVibrato[aChannel]; // CHIP
					aChannelPhaseB[aChannel] += aChannelPhaseDelta[aChannel]*aChannelVibrato[aChannel]*aChannelChorusDeltaRatio[aChannel]; // CHIP
					aChannelFilter[aChannel] *= aChannelFilterScale[aChannel]; // CHIP
					aChannelPhaseA[aChannel] -= (int)aChannelPhaseA[aChannel]; // CHIP
					aChannelPhaseB[aChannel] -= (int)aChannelPhaseB[aChannel]; // CHIP
					aChannelPhaseDelta[aChannel] *= aChannelPhaseDeltaScale[aChannel]; // CHIP
				}
				if (ISNOISE(aChannel))
				{
					aChannelSample[aChannel] += (float)(aChannelWave[aChannel]->Element((int)(aChannelPhase[aChannel]*32768.0f))*aChannelVolume[aChannel]-aChannelSample[aChannel])*aChannelFilter[aChannel]; // NOISE

					aChannelVolume[aChannel] += aChannelVolumeDelta[aChannel]; // NOISE
					aChannelPhase[aChannel] += aChannelPhaseDelta[aChannel]; // NOISE
					aChannelPhase[aChannel] -= (int)aChannelPhase[aChannel]; // NOISE
					aChannelPhaseDelta[aChannel] *= aChannelPhaseDeltaScale[aChannel]; // NOISE
				}

				if (ISFM(aChannel))
				{
					//
					// Looks like the error is either in this block of code, or in the instrumentsetup that sets up the stuff this code block uses.
					//
					for (int j = gBeepboxConfig->mOperatorCount-1;j>=0;j--) 
					{
						float aOperatorPhaseMix=aChannelOperatorPhase[aChannel][j];

						// Typescript version left here in case this is quirky and needs adjustment...
						//for (const modulatorNumber of Config.operatorModulatedBy[Synth.instruments[channel].algorithm][j]) operatorPhaseMix+=channelOperatorScaled[channel][modulatorNumber-1];
						
						for (int aCount=0;aCount<gBeepboxConfig->mOperatorModulatedBy[mInstruments[aChannel][mBar]->mAlgorithm][j].Size();aCount++)
						{
							int aModulatorNumber=gBeepboxConfig->mOperatorModulatedBy[mInstruments[aChannel][mBar]->mAlgorithm][j][aCount];
							aOperatorPhaseMix+=aChannelOperatorScaled[aChannel][aModulatorNumber-1];
						}

						Array<int>& aFeedbackIndices=gBeepboxConfig->mOperatorFeedbackIndices[mInstruments[aChannel][mBar]->mFeedbackType][j];
						if (aFeedbackIndices.Size()>0)
						{
							float aSum=0;
							// Typescript version left here in case this is quirky and needs adjustment...
							//for (const modulatorNumber of feedbackIndices) sum+=channelOperatorOutput[channel][modulatorNumber-1];
							for (int aCount=0;aCount<aFeedbackIndices.Size();aCount++) 
							{
								int aModulatorNumber=aFeedbackIndices[aCount];
								aSum+=aChannelOperatorOutput[aChannel][aModulatorNumber-1];
							}
							aSum*=aChannelFeedbackMult[aChannel];
							aOperatorPhaseMix+=aSum;
						}

						static int aOperatorPhaseInt;
						static int aOperatorIndex;
						static double aOperatorSample;

						aOperatorPhaseInt=(int)aOperatorPhaseMix;
						aOperatorIndex=aOperatorPhaseInt&255;
						aOperatorSample=gBeepboxConfig->mSineWave[aOperatorIndex];

						aChannelOperatorOutput[aChannel][j]=(float)(aOperatorSample+(gBeepboxConfig->mSineWave[aOperatorIndex+1]-aOperatorSample)*(aOperatorPhaseMix-aOperatorPhaseInt));
						aChannelOperatorScaled[aChannel][j]=(float)(aChannelOperatorOutputMult[aChannel][j]*aChannelOperatorOutput[aChannel][j]);
					}

					static float aCarrierOutput;
					aCarrierOutput=0;
					for (int j = 0; j<gBeepboxConfig->mOperatorCarrierCounts[mInstruments[aChannel][mBar]->mAlgorithm];j++) aCarrierOutput+=aChannelOperatorScaled[aChannel][j];
					aChannelSample[aChannel]=aChannelTremolo[aChannel]*aCarrierOutput; // CARRIER OUTPUTS

					//static int gMyCounter=0;
					//this->Out("%d <--> channelSample[%d] = %f",gMyCounter++,aChannel,aChannelSample[aChannel]);


					aChannelFeedbackMult[aChannel]+=aChannelFeedbackDelta[aChannel]; // FM
					for (int j=0;j<gBeepboxConfig->mOperatorCount;j++)
					{
						aChannelOperatorOutputMult[aChannel][j]+=aChannelOperatorOutputDelta[aChannel][j]; // FM
						aChannelOperatorPhase[aChannel][j]+=aChannelOperatorPhaseDelta[aChannel][j] * aChannelVibrato[aChannel]; // FM
						aChannelOperatorPhaseDelta[aChannel][j]*=aChannelPhaseDeltaScale[aChannel]; // FM
					}
				}
			}


			// Reverb, implemented using a feedback delay network with a Hadamard matrix and lowpass filters.
			// good ratios:    0.555235 + 0.618033 + 0.818 +   1.0 = 2.991268
			// Delay lengths:  3041     + 3385     + 4481  +  5477 = 16384 = 2^14
			// Buffer offsets: 3041    -> 6426   -> 10907 -> 16384
			static int aDelayPos1;
			static int aDelayPos2;
			static int aDelayPos3;
			static float aDelaySample0;
			aDelayPos1 = (aDelayPos +  3041) & 0x3FFF;
			aDelayPos2 = (aDelayPos +  6426) & 0x3FFF;
			aDelayPos3 = (aDelayPos + 10907) & 0x3FFF;
			aDelaySample0 = mDelayLine[aDelayPos];

			for (int aChannel=0;aChannel<mSong->mPitchChannelCount;aChannel++) aDelaySample0+=aChannelSample[aChannel]; // PITCH

			//
			// Bookmark 4
			//
			static float aDelaySample1;
			static float aDelaySample2;
			static float aDelaySample3;
			static float aDelayTemp0;
			static float aDelayTemp1;
			static float aDelayTemp2;
			static float aDelayTemp3;
			aDelaySample1=mDelayLine[aDelayPos1];
			aDelaySample2=mDelayLine[aDelayPos2];
			aDelaySample3=mDelayLine[aDelayPos3];
			aDelayTemp0=-aDelaySample0+aDelaySample1;
			aDelayTemp1=-aDelaySample0-aDelaySample1;
			aDelayTemp2=-aDelaySample2+aDelaySample3;
			aDelayTemp3=-aDelaySample2-aDelaySample3;

			aDelayFeedback0 += ((aDelayTemp0 + aDelayTemp2) * aReverb - aDelayFeedback0) * 0.5f;
			aDelayFeedback1 += ((aDelayTemp1 + aDelayTemp3) * aReverb - aDelayFeedback1) * 0.5f;
			aDelayFeedback2 += ((aDelayTemp0 - aDelayTemp2) * aReverb - aDelayFeedback2) * 0.5f;
			aDelayFeedback3 += ((aDelayTemp1 - aDelayTemp3) * aReverb - aDelayFeedback3) * 0.5f;

			mDelayLine[aDelayPos1] = aDelayFeedback0;
			mDelayLine[aDelayPos2] = aDelayFeedback1;
			mDelayLine[aDelayPos3] = aDelayFeedback2;
			mDelayLine[aDelayPos ] = aDelayFeedback3;
			aDelayPos = (aDelayPos + 1) & 0x3FFF;

			static float aSample;
			aSample = aDelaySample0+aDelaySample1+aDelaySample2+aDelaySample3;
			for (int aChannel=mSong->mPitchChannelCount;aChannel<mSong->mPitchChannelCount+mSong->mDrumChannelCount;aChannel++) aSample+=aChannelSample[aChannel]; // NOISE

			static float aAbs;
			aAbs = abs(aSample);//aSample<0.0f ? -aSample:aSample;
			aLimit-=aLimitDecay;
			if (aLimit<aAbs) aLimit=aAbs;
			aSample/=aLimit*0.75f+0.25f;
			aSample*=aVolume;

			if (aSample==aSample) *theBuffer=(short)(aSample*32767);

			//theBuffer[theBufferIndex]=(short)(aSample*32767);
			
			theBuffer++;

			theBufferIndex++;
			aSamples--;
		}

		//
		// BOOKMARK 5
		//
		for (int aChannel=0;aChannel<mSong->mPitchChannelCount+mSong->mDrumChannelCount;aChannel++) 
		{
			if (ISCHIP(aChannel))
			{
				aSynthChannel[aChannel]->mPhases[0] = aChannelPhaseA[aChannel]; // CHIP
				aSynthChannel[aChannel]->mPhases[1] = aChannelPhaseB[aChannel]; // CHIP
			}
			if (ISNOISE(aChannel))
			{
				aSynthChannel[aChannel]->mPhases[0] = aChannelPhase[aChannel]; // NOISE
			}
			if (ISFM(aChannel))
			{
				for (int j=0;j<gBeepboxConfig->mOperatorCount;j++)
				{
					aSynthChannel[aChannel]->mPhases[j] = aChannelOperatorPhase[aChannel][j] / gBeepboxConfig->mSineWaveLength; // FM
					aSynthChannel[aChannel]->mFeedbackOutputs[j] = aChannelOperatorOutput[aChannel][j]; // FM
				}
			}
			aSynthChannel[aChannel]->mSample=aChannelSample[aChannel]; // ALL
		}

		mDelayPos = (float)aDelayPos;
		mDelayFeedback0 = aDelayFeedback0;
		mDelayFeedback1 = aDelayFeedback1;
		mDelayFeedback2 = aDelayFeedback2;
		mDelayFeedback3 = aDelayFeedback3;
		aLimit = mLimit; // Was mLimit=mLimit???


		//gOut.Out("Here... %f [%f]",mEffectPhase,aEffectY);
		if (aEffectYMult*aEffectY-aPrevEffectY>aPrevEffectY) mEffectPhase=(float)asin(_clamp(-1.0f,aEffectY,1.0f));
		else mEffectPhase=gMath.mPI-(float)asin(_clamp(-1.0f,aEffectY,1.0f));
		//gOut.Out("After Here... %f [%f]",mEffectPhase,aEffectY);

		if (mArpeggioSampleCountdown == 0) 
		{
			mArpeggio++;
			mArpeggioSampleCountdown=aSamplesPerArpeggio;
			if (mArpeggio == 4) 
			{
				mArpeggio = 0;
				mPart++;
				if (mPart==mSong->mPartsPerBeat) 
				{
					mPart = 0;
					mBeat++;
					if (mBeat==mSong->mBeatsPerBar) return theBufferIndex;
				}
			}
		}
	}
	return -1;
}


float BeepBox::ComputeOperatorEnvelope(int theEnvelope, float theTime, float theBeats, float theCustomVolume)
{
	switch	(gBeepboxConfig->mOperatorEnvelopeType[theEnvelope]) 
	{
	case EnvelopeType_custom: return theCustomVolume;
	case EnvelopeType_steady: return 1.0f;
	case EnvelopeType_pluck:
		{
			float aCurve=1.0f/(1.0f + theTime * gBeepboxConfig->mOperatorEnvelopeSpeed[theEnvelope]);
			if (gBeepboxConfig->mOperatorEnvelopeInverted[theEnvelope]) return 1.0f-aCurve;
			else return aCurve;
		}
	case EnvelopeType_tremolo: return 0.5f - (float)cos(theBeats*2.0f*gMath.mPI*gBeepboxConfig->mOperatorEnvelopeSpeed[theEnvelope]) * 0.5f;
	case EnvelopeType_punch: return _max(1.0f, 2.0f - theTime * 10.0f);
	case EnvelopeType_flare:
		{
			float aSpeed=gBeepboxConfig->mOperatorEnvelopeSpeed[theEnvelope];
			float aAttack=0.25f / (float)sqrt(aSpeed);
			return theTime<aAttack ? theTime / aAttack : 1.0f / (1.0f + (theTime - aAttack) * aSpeed);
		}
	default: return 0;
	}
}


void BeepBox::ComputeChannelInstrument(int theChannel, int theTime, float theSampleTime, int theSamplesPerArpeggio, int theSamples)
{
	static bool aIsDrum;
	static BeepBoxChannel* aSynthChannel=mChannels[theChannel];
	static BeepBoxSong::Pattern* aPattern;
	static BeepBoxSong::Instrument* aInstrument;
	static bool aPianoMode;
	static float aBasePitch;
	static float aIntervalScale;
	static float aPitchDamping;
	static float aSecondsPerPart;
	static float aBeatsPerPart;

	aIsDrum=mSong->GetChannelIsDrum(theChannel);
	aSynthChannel=mChannels[theChannel];
	aPattern=mSong->GetPattern(theChannel,mBar);
	aInstrument=mSong->mChannels[theChannel]->mInstruments[(aPattern==NULL) ? 0 : aPattern->mInstrument];
	aPianoMode=(mPianoPressed && theChannel==mPianoChannel);
	aBasePitch=aIsDrum ? gBeepboxConfig->mDrumBasePitches[aInstrument->mWave] : gBeepboxConfig->mKeyTransposes[mSong->mKey];
	aIntervalScale=aIsDrum ? (float)gBeepboxConfig->mDrumInterval : 1.0f;
	aPitchDamping=aIsDrum ? (gBeepboxConfig->mDrumWaveIsSoft[aInstrument->mWave] ? 24.0f : 60.0f) : 48.0f;
	aSecondsPerPart=4.0f*(float)theSamplesPerArpeggio/mSamplesPerSecond;
	aBeatsPerPart=1.0f/(float)mSong->mPartsPerBeat;

	aSynthChannel->mPhaseDeltaScale = 0.0f;
	aSynthChannel->mFilter = 1.0f;
	aSynthChannel->mFilterScale = 1.0f;
	aSynthChannel->mVibratoScale = 0.0f;
	aSynthChannel->mHarmonyMult = 1.0f;
	aSynthChannel->mHarmonyVolumeMult = 1.0f;

	static float aPartsSinceStart;
	static int aArpeggio;
	static int aArpeggioSampleCountdown;

	aPartsSinceStart=0.0f;
	aArpeggio=mArpeggio;
	aArpeggioSampleCountdown=mArpeggioSampleCountdown;


	static ReadOnlyArray<float>* aPitches;
	static bool aResetPhases;
	static float aIntervalStart;
	static float aIntervalEnd;
	static float aTransitionVolumeStart;
	static float aTransitionVolumeEnd;
	static float aEnvelopeVolumeStart;
	static float aEnvelopeVolumeEnd;
	static float aPartTimeStart;
	static float aPartTimeEnd;
	static float aDecayTimeStart;
	static float aDecayTimeEnd;

	aPitches=NULL;
	aResetPhases=true;
	aIntervalStart=0.0f;
	aIntervalEnd=0.0f;
	aTransitionVolumeStart=1.0f;
	aTransitionVolumeEnd=1.0f;
	aEnvelopeVolumeStart=0.0f;
	aEnvelopeVolumeEnd=0.0f;
	aPartTimeStart=0.0f;
	aPartTimeEnd=0.0f;
	aDecayTimeStart=0.0f;
	aDecayTimeEnd=0.0f;

	for (int i=0;i<gBeepboxConfig->mOperatorCount; i++) 
	{
		aSynthChannel->mPhaseDeltas[i] = 0.0f;
		aSynthChannel->mVolumeStarts[i] = 0.0f;
		aSynthChannel->mVolumeDeltas[i] = 0.0f;
	}

	if (aPianoMode) 
	{
		aPitches=&mPianoPitch;
		aTransitionVolumeStart=aTransitionVolumeEnd=1;
		aEnvelopeVolumeStart=aEnvelopeVolumeEnd=1;
		aResetPhases=false;
	}
	else if (aPattern) 
	{
		static BeepBoxSong::Note* aNote;
		static BeepBoxSong::Note* aPrevNote;
		static BeepBoxSong::Note* aNextNote;

		aNote=NULL;
		aPrevNote=NULL;
		aNextNote=NULL;

		for (int i=0;i<aPattern->mNotes.GetCount(); i++) 
		{
			if (aPattern->mNotes[i]->mEnd<=theTime) aPrevNote=aPattern->mNotes[i];
			else if (aPattern->mNotes[i]->mStart<=theTime && aPattern->mNotes[i]->mEnd>theTime) aNote=aPattern->mNotes[i];
			else if (aPattern->mNotes[i]->mStart>theTime) 
			{
				aNextNote=aPattern->mNotes[i];
				break;
			}
		}
		if (aNote != NULL && aPrevNote != NULL && aPrevNote->mEnd != aNote->mStart) aPrevNote = NULL;
		if (aNote != NULL && aNextNote != NULL && aNextNote->mStart != aNote->mEnd) aNextNote = NULL;

		if (aNote != NULL) 
		{
			aPitches = &aNote->mPitches;
			aPartsSinceStart = (float)(theTime - aNote->mStart);

			static int aEndPinIndex;
			for (aEndPinIndex = 1; aEndPinIndex < aNote->mPins.GetCount() - 1; aEndPinIndex++) if (aNote->mPins[aEndPinIndex]->mTime + aNote->mStart > theTime) break;

			static BeepBoxSong::NotePin* aStartPin;
			static BeepBoxSong::NotePin* aEndPin;
			static int aNoteStart;
			static int aNoteEnd;
			static int aPinStart;
			static int aPinEnd;
			static int aTickTimeStart;
			static int aTickTimeEnd;
			static float aPinRatioStart;
			static float aPinRatioEnd;
			static float aEnvelopeVolumeTickStart;
			static float aEnvelopeVolumeTickEnd;
			static float aTransitionVolumeTickStart;
			static float aTransitionVolumeTickEnd;
			static float aIntervalTickStart;
			static float aIntervalTickEnd;
			static float aPartTimeTickStart;
			static float aPartTimeTickEnd;
			static float aDecayTimeTickStart;
			static float aDecayTimeTickEnd;
			static float aStartRatio;
			static float aEndRatio;

			aStartPin=aNote->mPins[aEndPinIndex-1];
			aEndPin=aNote->mPins[aEndPinIndex];
			aNoteStart=aNote->mStart*4;
			aNoteEnd=aNote->mEnd*4;
			aPinStart=(aNote->mStart+aStartPin->mTime) * 4;
			aPinEnd=(aNote->mStart+aEndPin->mTime) * 4;
			aTickTimeStart=(theTime*4+aArpeggio);
			aTickTimeEnd=(theTime*4+aArpeggio + 1);
			aPinRatioStart=(float)(aTickTimeStart-aPinStart)/(float)(aPinEnd-aPinStart);
			aPinRatioEnd=(float)(aTickTimeEnd-aPinStart)/(float)(aPinEnd-aPinStart);
			aEnvelopeVolumeTickStart=aStartPin->mVolume + (aEndPin->mVolume-aStartPin->mVolume)*aPinRatioStart;
			aEnvelopeVolumeTickEnd=aStartPin->mVolume + (aEndPin->mVolume-aStartPin->mVolume)*aPinRatioEnd;
			aTransitionVolumeTickStart=1.0f;
			aTransitionVolumeTickEnd=1.0f;
			aIntervalTickStart=aStartPin->mInterval + (aEndPin->mInterval - aStartPin->mInterval) * aPinRatioStart;
			aIntervalTickEnd=aStartPin->mInterval + (aEndPin->mInterval - aStartPin->mInterval) * aPinRatioEnd;
			aPartTimeTickStart=aStartPin->mTime + (aEndPin->mTime-aStartPin->mTime) * aPinRatioStart;
			aPartTimeTickEnd=aStartPin->mTime + (aEndPin->mTime-aStartPin->mTime) * aPinRatioEnd;
			aDecayTimeTickStart=aPartTimeTickStart;
			aDecayTimeTickEnd=aPartTimeTickEnd;
			aStartRatio=1.0f - (float)(aArpeggioSampleCountdown + theSamples) / (float)theSamplesPerArpeggio;
			aEndRatio=1.0f - (float)(aArpeggioSampleCountdown) / (float)theSamplesPerArpeggio;

			aResetPhases=((aTickTimeStart+aStartRatio-aNoteStart) == 0.0f);

			int aTransition=aInstrument->mTransition;
			if (aTickTimeStart == aNoteStart) 
			{
				if (aTransition == 0) aResetPhases = false;
				else if (aTransition == 2) aTransitionVolumeTickStart = 0.0f;
				else if (aTransition == 3) 
				{
					if (aPrevNote==NULL) aTransitionVolumeTickStart = 0.0f;
					else if (aPrevNote->mPins[aPrevNote->mPins.GetCount()-1]->mVolume==0 || aNote->mPins[0]->mVolume==0) aTransitionVolumeTickStart = 0.0f;
					else 
					{
						aIntervalTickStart = (aPrevNote->mPitches[0] + aPrevNote->mPins[aPrevNote->mPins.GetCount()-1]->mInterval - aNote->mPitches[0]) * 0.5f;
						aDecayTimeTickStart = (aPrevNote->mPins[aPrevNote->mPins.GetCount()-1]->mTime * 0.5f);
						aResetPhases = false;
					}
				}
			}
			if (aTickTimeEnd == aNoteEnd) 
			{
				if (aTransition == 0) 
				{
					// seamless ending: fade out, unless adjacent to another note or at end of bar.
					if (aNextNote==NULL && aNote->mStart + aEndPin->mTime != mSong->mPartsPerBeat * mSong->mBeatsPerBar) aTransitionVolumeTickEnd = 0.0f;
				} 
				else if (aTransition == 1 || aTransition == 2) 
				{
					// sudden/smooth ending
					aTransitionVolumeTickEnd = 0.0f;
				}
				else if (aTransition == 3) 
				{
					// slide ending
					if (aNextNote == NULL) 
					{
						aTransitionVolumeTickEnd = 0.0f;
					} 
					else if (aNote->mPins[aNote->mPins.GetCount()-1]->mVolume == 0 || aNextNote->mPins[0]->mVolume == 0) 
					{
						aTransitionVolumeTickEnd = 0.0f;
					} 
					else 
					{
						aIntervalTickEnd = (aNextNote->mPitches[0] - aNote->mPitches[0] + aNote->mPins[aNote->mPins.GetCount()-1]->mInterval) * 0.5f;
						aDecayTimeTickEnd *= 0.5f;
					}
				}
			}

			aIntervalStart = aIntervalTickStart + (aIntervalTickEnd - aIntervalTickStart) * aStartRatio;
			aIntervalEnd   = aIntervalTickStart + (aIntervalTickEnd - aIntervalTickStart) * aEndRatio;
			aEnvelopeVolumeStart = VolumeConversion(aEnvelopeVolumeTickStart + (aEnvelopeVolumeTickEnd - aEnvelopeVolumeTickStart) * aStartRatio);
			aEnvelopeVolumeEnd   = VolumeConversion(aEnvelopeVolumeTickStart + (aEnvelopeVolumeTickEnd - aEnvelopeVolumeTickStart) * aEndRatio);
			aTransitionVolumeStart = aTransitionVolumeTickStart + (aTransitionVolumeTickEnd - aTransitionVolumeTickStart) * aStartRatio;
			aTransitionVolumeEnd   = aTransitionVolumeTickStart + (aTransitionVolumeTickEnd - aTransitionVolumeTickStart) * aEndRatio;
			aPartTimeStart = aNote->mStart + aPartTimeTickStart + (aPartTimeTickEnd - aPartTimeTickStart) * aStartRatio;
			aPartTimeEnd   = aNote->mStart + aPartTimeTickStart + (aPartTimeTickEnd - aPartTimeTickStart) * aEndRatio;
			aDecayTimeStart = aDecayTimeTickStart + (aDecayTimeTickEnd - aDecayTimeTickStart) * aStartRatio;
			aDecayTimeEnd   = aDecayTimeTickStart + (aDecayTimeTickEnd - aDecayTimeTickStart) * aEndRatio;
		}
	}

	if (aPitches!=NULL) 
	{
		if (!aIsDrum && aInstrument->mType == InstrumentType_fm) 
		{
			// phase modulation!
			static float aSineVolumeBoost;
			static float aTotalCarrierVolume;
			static int aCarrierCount;

			aSineVolumeBoost=1.0f;
			aTotalCarrierVolume=0.0f;
			aCarrierCount=gBeepboxConfig->mOperatorCarrierCounts[aInstrument->mAlgorithm];

			for (int i=0;i<gBeepboxConfig->mOperatorCount;i++) 
			{
				int aAssociatedCarrierIndex=gBeepboxConfig->mOperatorAssociatedCarrier[aInstrument->mAlgorithm][i]-1;

				static float aPitch;
				static float aFreqMult;
				static float aChorusInterval;
				static float aStartPitch;
				static float aStartFreq;

				aPitch=aPitches->Element((i < aPitches->Size()) ? i : ((aAssociatedCarrierIndex < aPitches->Size()) ? aAssociatedCarrierIndex : 0));
				aFreqMult=gBeepboxConfig->mOperatorFrequencies[aInstrument->mOperators[i]->mFrequency];
				aChorusInterval=gBeepboxConfig->mOperatorCarrierChorus[aAssociatedCarrierIndex];
				aStartPitch=(aPitch + aIntervalStart) * aIntervalScale + aChorusInterval;
				aStartFreq=aFreqMult*(FrequencyFromPitch(aBasePitch + aStartPitch)) + gBeepboxConfig->mOperatorHzOffsets[aInstrument->mOperators[i]->mFrequency];

				aSynthChannel->mPhaseDeltas[i] = aStartFreq * theSampleTime * gBeepboxConfig->mSineWaveLength;
				if (aResetPhases) aSynthChannel->Reset();

				static float aAmplitudeCurve;
				static float aAmplitudeMult;
				static float aVolumeStart;
				static float aVolumeEnd;

				aAmplitudeCurve=OperatorAmplitudeCurve((float)aInstrument->mOperators[i]->mAmplitude);
				aAmplitudeMult=aAmplitudeCurve*gBeepboxConfig->mOperatorAmplitudeSigns[aInstrument->mOperators[i]->mFrequency];
				aVolumeStart=aAmplitudeMult;
				aVolumeEnd=aAmplitudeMult;

				if (i < aCarrierCount) 
				{
					// carrier
					static float aVolumeMult;
					static float aEndPitch;
					static float aPitchVolumeStart;
					static float aPitchVolumeEnd;

					aVolumeMult=0.03f;
					aEndPitch=(aPitch + aIntervalEnd) * aIntervalScale;
					aPitchVolumeStart=(float)pow(2.0f, -aStartPitch / aPitchDamping);
					aPitchVolumeEnd=(float)pow(2.0f,   -aEndPitch / aPitchDamping);

					aVolumeStart *= aPitchVolumeStart * aVolumeMult * aTransitionVolumeStart;
					aVolumeEnd *= aPitchVolumeEnd * aVolumeMult * aTransitionVolumeEnd;
					aTotalCarrierVolume += aAmplitudeCurve;
				} 
				else 
				{
					// modulator
					aVolumeStart *= gBeepboxConfig->mSineWaveLength * 1.5f;
					aVolumeEnd *= gBeepboxConfig->mSineWaveLength * 1.5f;
					aSineVolumeBoost*=1.0f - _min(1.0f, aInstrument->mOperators[i]->mAmplitude / 15.0f);
				}
				int aEnvelope=aInstrument->mOperators[i]->mEnvelope;

				aVolumeStart *= ComputeOperatorEnvelope(aEnvelope,aSecondsPerPart * aDecayTimeStart, aBeatsPerPart * aPartTimeStart, aEnvelopeVolumeStart);
				aVolumeEnd *= ComputeOperatorEnvelope(aEnvelope, aSecondsPerPart * aDecayTimeEnd, aBeatsPerPart * aPartTimeEnd, aEnvelopeVolumeEnd);

				aSynthChannel->mVolumeStarts[i] = aVolumeStart;
				aSynthChannel->mVolumeDeltas[i] = (aVolumeEnd - aVolumeStart) / theSamples;
			}

			static float aFeedbackAmplitude;
			static float aFeedbackStart;
			static float aFeedbackEnd;

			aFeedbackAmplitude=gBeepboxConfig->mSineWaveLength * 0.3f * aInstrument->mFeedbackAmplitude / 15.0f;
			aFeedbackStart=aFeedbackAmplitude * ComputeOperatorEnvelope(aInstrument->mFeedbackEnvelope, aSecondsPerPart * aDecayTimeStart, aBeatsPerPart * aPartTimeStart, aEnvelopeVolumeStart);
			aFeedbackEnd=aFeedbackAmplitude * ComputeOperatorEnvelope(aInstrument->mFeedbackEnvelope, aSecondsPerPart * aDecayTimeEnd, aBeatsPerPart * aPartTimeEnd, aEnvelopeVolumeEnd);

			aSynthChannel->mFeedbackMult = aFeedbackStart;
			aSynthChannel->mFeedbackDelta = (aFeedbackEnd - aSynthChannel->mFeedbackMult) / theSamples;

			aSineVolumeBoost *= 1.0f - aInstrument->mFeedbackAmplitude / 15.0f;

			aSineVolumeBoost *= 1.0f - _min(1.0f, _max(0.0f, aTotalCarrierVolume - 1) / 2.0f);

			for (int i=0;i<aCarrierCount;i++) 
			{
				aSynthChannel->mVolumeStarts[i] *= 1.0f + aSineVolumeBoost * 3.0f;
				aSynthChannel->mVolumeDeltas[i] *= 1.0f + aSineVolumeBoost * 3.0f;
			}
		} 
		else 
		{
			float aPitch=aPitches->Element(0);

			if (gBeepboxConfig->mChorusHarmonizes[aInstrument->mChorus]) 
			{
				static float aHarmonyOffset;
				aHarmonyOffset=0.0f;
				
				if (aPitches->Size()==2) aHarmonyOffset=aPitches->Element(1)-aPitches->Element(0);
				else if (aPitches->Size()==3) aHarmonyOffset=aPitches->Element((aArpeggio >> 1) + 1)-aPitches->Element(0);
				else if (aPitches->Size()==4) aHarmonyOffset = aPitches->Element((aArpeggio == 3 ? 1 : aArpeggio) + 1) - aPitches->Element(0);

				aSynthChannel->mHarmonyMult = (float)pow(2.0f, aHarmonyOffset/12.0f);
				aSynthChannel->mHarmonyVolumeMult = (float)pow(2.0f, -aHarmonyOffset/aPitchDamping);
			} 
			else 
			{
				if (aPitches->Size()== 2) aPitch = aPitches->Element(aArpeggio >> 1);
				else if (aPitches->Size() == 3) aPitch=aPitches->Element(aArpeggio == 3 ? 1 : aArpeggio);
				else if (aPitches->Size() == 4) aPitch=aPitches->Element(aArpeggio);
			}

			static float aStartPitch;
			static float aEndPitch;
			static float aStartFreq;
			static float aPitchVolumeStart;
			static float aPitchVolumeEnd;

			aStartPitch=(aPitch + aIntervalStart) * aIntervalScale;
			aEndPitch=(aPitch + aIntervalEnd) * aIntervalScale;
			aStartFreq=FrequencyFromPitch(aBasePitch + aStartPitch);
			aPitchVolumeStart=(float)pow(2.0f, -aStartPitch / aPitchDamping);
			aPitchVolumeEnd=(float)pow(2.0f,   -aEndPitch / aPitchDamping);

			if (aIsDrum && gBeepboxConfig->mDrumWaveIsSoft[aInstrument->mWave]) aSynthChannel->mFilter = _min(1.0f, aStartFreq * theSampleTime * gBeepboxConfig->mDrumPitchFilterMult[aInstrument->mWave]);
			float aSettingsVolumeMult;
			if (!aIsDrum) 
			{
				static float aFilterScaleRate;
				static float aEndFilter;

				aFilterScaleRate = gBeepboxConfig->mFilterDecays[aInstrument->mFilter];
				aSynthChannel->mFilter = (float)pow(2, -aFilterScaleRate * aSecondsPerPart * aDecayTimeStart);
				aEndFilter=	(float)pow(2, -aFilterScaleRate * aSecondsPerPart * aDecayTimeEnd);
				aSynthChannel->mFilterScale = (float)pow(aEndFilter / aSynthChannel->mFilter, 1.0f / theSamples);
				aSettingsVolumeMult = 0.27f * 0.5f * gBeepboxConfig->mWaveVolumes[aInstrument->mWave]*gBeepboxConfig->mFilterVolumes[aInstrument->mFilter] * gBeepboxConfig->mChorusVolumes[aInstrument->mChorus];
			} 
			else aSettingsVolumeMult = 0.19f * gBeepboxConfig->mDrumVolumes[aInstrument->mWave];
			if (aResetPhases && !aIsDrum) aSynthChannel->Reset();

			aSynthChannel->mPhaseDeltas[0]=aStartFreq * theSampleTime;

			static float aInstrumentVolumeMult;
			static float aVolumeEnd;
			aInstrumentVolumeMult=(aInstrument->mVolume == 5) ? 0.0f : (float)pow(2, -gBeepboxConfig->mVolumeValues[aInstrument->mVolume]);
			aSynthChannel->mVolumeStarts[0] = aTransitionVolumeStart * aEnvelopeVolumeStart * aPitchVolumeStart * aSettingsVolumeMult * aInstrumentVolumeMult;
			aVolumeEnd=aTransitionVolumeEnd * aEnvelopeVolumeEnd * aPitchVolumeEnd * aSettingsVolumeMult * aInstrumentVolumeMult;
			aSynthChannel->mVolumeDeltas[0] = (aVolumeEnd - aSynthChannel->mVolumeStarts[0]) / theSamples;
		}

		aSynthChannel->mPhaseDeltaScale = (float)pow(2.0f, ((aIntervalEnd - aIntervalStart) * aIntervalScale / 12.0f) / theSamples);
		aSynthChannel->mVibratoScale = (aPartsSinceStart < gBeepboxConfig->mEffectVibratoDelays[aInstrument->mEffect]) ? 0.0f : (float)pow(2.0f, gBeepboxConfig->mEffectVibratos[aInstrument->mEffect] / 12.0f) - 1.0f;
	} 
	else 
	{
		aSynthChannel->Reset();
		for (int i=0;i<gBeepboxConfig->mOperatorCount; i++) 
		{
			aSynthChannel->mPhaseDeltas[0] = 0.0f;
			aSynthChannel->mVolumeStarts[0] = 0.0f;
			aSynthChannel->mVolumeDeltas[0] = 0.0f;
		}
	}
}

