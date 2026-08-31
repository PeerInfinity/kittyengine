#pragma once

//
// Having a random statement and you don't know where it's coming from?
// Turn this on to hit it.
//
//#define TRACKDOWN

#include "os_core.h"
#define Random RaptRandom

inline int QuickRand(int theSeed)
{
	//if (theSeed==0) theSeed=1;
	theSeed++;
	//
	// Note: This can only go up to 32767!
	// So we can't really use it...
	//
    //int aSeed=(214013*theSeed+2531011);
    //return (aSeed>>16)&0x7FFF;
    
    //*
	//return ((theSeed+1)*0x0a67cfcf)&0x7FFFFFFF;

	theSeed ^= (theSeed << 21);
	theSeed ^= (theSeed >> 11);
	theSeed ^= (theSeed << 4);

	int aResult=theSeed*0x0a67cfcf;
	if (aResult&2147483648u) aResult=(2147483648u-(aResult&2147483647u));
    return (aResult&0x3FFFFFFF);
     /**/
}

inline float QuickRandFraction(int theSeed) {return (float)(QuickRand(theSeed)%1000000)/1000000.0f;}
inline float SeedToScale(int theSeed) {return (float)(theSeed%1000000)/1000000.0f;}
inline float SeedToSignedScale(int theSeed) {return ((float)(theSeed%200000)/100000.0f)-1.0f;}
inline float SeedToRange(int theSeed, float theLow, float theHigh) {return theLow+((theHigh-theLow)*SeedToScale(theSeed));}
inline float SeedToSignedRange(int theSeed, float theLow, float theHigh) {return theLow+((theHigh-theLow)*SeedToSignedScale(theSeed));}

//
// A daily seed you can use if you ever need a unique number on a daily basis...
//

#define OLDRANDOM
#ifdef OLDRANDOM
//*
class Random
{
public:
	Random(void);

	inline void			SetFloatPrecision(int thePrecision=100000) {mFloatPrecision=thePrecision;}

	inline void			Go() {Seed();}
	void				Seed(int theSeed);
	inline void			Seed() {Seed(OS_Core::Tick());}
	inline int			GetSeed() {return mSeed;}

	int					Generate(void);
	int					Get(int theMax);
	inline float		GetF(float theMax=1.0f) {return ((float)Get(mFloatPrecision+1)/(float)mFloatPrecision)*theMax;}
	inline bool			GetBool() {return (Get(10)<5);}
	inline float		GetAngle() {return GetF(360);}
	inline int			Negate(int theNumber) {if (GetBool()) return theNumber;return -theNumber;}
	inline float		NegateF(float theNumber) {if (GetBool()) return theNumber;return -theNumber;}
	inline int			Neg(int theNumber) {return Negate(theNumber);}
	inline float		NegF(float theNumber) {return NegateF(theNumber);}
	inline bool			Chance(int theMax) {if (theMax>1) return (Get(theMax)==1);return (theMax==1);}

	inline int			GetSpan(int theLow, int theHigh) {if (theLow>=theHigh) return theLow;return Get((theHigh-theLow)+1)+theLow;}
	inline int			GetSpan(int theNumber) {return GetSpan(-theNumber,theNumber);}
	inline float		GetSpanF(float theLow, float theHigh) {if (theLow>=theHigh) return theLow;return GetF((theHigh-theLow))+theLow;}
	inline float		GetSpanF(float theNumber) {return GetSpanF(-theNumber,theNumber);}

public:
	int					mSeed;
	int					mFloatPrecision;
	int					rgiState[2+55];

	//
	// Helpers for compatibility....
	//
	inline int			GetStateDataSize() {return 2+55;}
	inline int&			GetStateData(int theSlot) {return rgiState[theSlot];}

};
#else
class Random
{
public:
	int					mSeed;
	int					mFloatPrecision;

	Random() {SetFloatPrecision();Go();}

	inline void			SetFloatPrecision(int thePrecision=100000) {mFloatPrecision=thePrecision;}

	inline void			Go() {Seed(OS_Core::Tick());}
	inline void			Seed(int theSeed) {mSeed=theSeed;}
	inline int			GetSeed() {return mSeed;}
	inline int			Get(int theMax)  {if (theMax==0) return 0;mSeed=QuickRand(mSeed);return (mSeed%theMax);}
	inline float		GetF(float theMax=1.0f) {return ((float)Get(mFloatPrecision+1)/(float)mFloatPrecision)*theMax;}
	inline bool			GetBool() {return (Get(10)<5);}
	inline float		GetAngle() {return GetF(360);}
	inline int			Negate(int theNumber) {if (GetBool()) return theNumber;return -theNumber;}
	inline float		NegateF(float theNumber) {if (GetBool()) return theNumber;return -theNumber;}
	inline int			Neg(int theNumber) {return Negate(theNumber);}
	inline float		NegF(float theNumber) {return NegateF(theNumber);}
	inline bool			Chance(int theMax) {if (theMax>1) return (Get(theMax)==1);return (theMax==1);}

	inline int			GetSpan(int theLow, int theHigh) {if (theLow>=theHigh) return theLow;return Get((theHigh-theLow)+1)+theLow;}
	inline int			GetSpan(int theNumber) {return GetSpan(-theNumber,theNumber);}
	inline float		GetSpanF(float theLow, float theHigh) {if (theLow>=theHigh) return theLow;return GetF((theHigh-theLow))+theLow;}
	inline float		GetSpanF(float theNumber) {return GetSpanF(-theNumber,theNumber);}

};
#endif

/*
inline unsigned int FastRand(unsigned int theSeed)
{
	//theSeed=theSeed*1103515245 + 12345;
	//return theSeed;

	//return ((theSeed+1)*0x0a67cfcf)&0x7FFFFFFF;

	theSeed ^= (theSeed << 21);
	theSeed ^= (theSeed >> 11);
	theSeed ^= (theSeed << 4);

	int aResult=theSeed*0x0a67cfcf;
	if (aResult&2147483648u) return (2147483648u-(aResult&2147483647u));
	return aResult;
}

inline float FastRandFraction(unsigned int theSeed)
{
	//theSeed=theSeed*1103515245 + 12345;
	//return theSeed;

	//return ((theSeed+1)*0x0a67cfcf)&0x7FFFFFFF;

	theSeed ^= (theSeed << 21);
	theSeed ^= (theSeed >> 11);
	theSeed ^= (theSeed << 4);

	int aResult=theSeed*0x0a67cfcf;
	if (aResult&2147483648u) aResult=(2147483648u-(aResult&2147483647u));
	return (float)(aResult%100000)/100000.0f;
}
*/




extern Random gRandom;
extern Random& gRand;


