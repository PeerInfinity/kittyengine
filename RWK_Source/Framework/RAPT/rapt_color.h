#pragma once

#include "os_headers.h"
#include "os_core.h"

#define BLACK	Color(0,0,0)
#define RED		Color(1,0,0)
#define GREEN	Color(0,1,0)
#define BLUE	Color(0,0,1)
#define PURPLE	Color(1,0,1)
#define CYAN	Color(0,1,1)
#define WHITE	Color(1,1,1)
#define YELLOW	Color(1,1,0)
#define ORANGE	Color(1.0f,.5f,0.0f)

#define CH(x) ((float)x/255.0f)

class Color
{
public:
	Color();
	//Color(int theColor) {Color((float)theColor);}
	Color(int theAlpha);
	Color(float theAlpha);
	Color(float theRed, float theGreen, float theBlue, float theAlpha=1.0f);
	Color(char* theColor);
	Color(Color theColor, float theA) {*this=theColor;mA=theA;}
	//Color(char* theColor) {FromWeb(theColor);}

	void				FromWeb(char* theHex);	// From format "#FFFFFFFF"
	char*				ToWeb();	// To format "#FFFFFFFF"
	void				FromText(char* theRGB);	// From format "1.0,1.0,1.0"
	void				Mix(Color theColor, float thePercent, bool mixAlpha=false);
	inline Color		Clip() {return Color(_max(0,_min(1,mR)),_max(0,_min(1,mG)),_max(0,_min(1,mB)),_max(0,_min(1,mA)));}
	Color				Pastel(float theThreshold=.5f);
	Color				Add(Color theColor);
	Color				Desaturate(float thePercent);
	Color				Darken(float thePercent);
	Color				Lighten(float thePercent);
	Color				Interpolate(Color theFinalColor, float thePercent);
	Color				Maximize();

	Color				MultiplyRGB(float theAmount) {return Color(mR*theAmount,mG*theAmount,mB*theAmount,mA);}

	inline void			Set(float theRed, float theGreen, float theBlue, float theAlpha=1.0f) 
	{
		mR=theRed;
		mG=theGreen;
		mB=theBlue;
		mA=theAlpha;
	}

	inline void			Primary(int theColor)
	{
		switch (theColor)
		{
			case 0: *this=RED;break;
			case 1: *this=ORANGE;break;
			case 2: *this=YELLOW;break;
			case 3: *this=GREEN;break;
			case 4: *this=CYAN;break;
			case 5: *this=BLUE;break;
			case 6: *this=PURPLE;break;
			default: *this=WHITE;break;
		}
	}

	inline bool	operator==(const Color &theColor) {return (mR==theColor.mR)&(mG==theColor.mG)&(mB==theColor.mB)&(mA==theColor.mA);}


	inline void operator+=(Color &theColor){mR+=theColor.mR;mG+=theColor.mG;mB+=theColor.mB;mA+=theColor.mA;}
	inline void operator-=(Color &theColor){mR-=theColor.mR;mG-=theColor.mG;mB-=theColor.mB;mA-=theColor.mA;}
	inline void operator*=(Color &theColor){mR*=theColor.mR;mG*=theColor.mG;mB*=theColor.mB;mA*=theColor.mA;}
	inline void operator/=(Color &theColor){mR/=theColor.mR;mG/=theColor.mG;mB/=theColor.mB;mA/=theColor.mA;}
	inline void operator+=(float theVal){mR+=theVal;mG+=theVal;mB+=theVal;}
	inline void operator-=(float theVal){mR-=theVal;mG-=theVal;mB-=theVal;}
	inline void operator*=(float theVal){mR*=theVal;mG*=theVal;mB*=theVal;}
	inline void operator/=(float theVal){mR/=theVal;mG/=theVal;mB/=theVal;}

	inline Color operator+(Color &theColor){return Color(mR+theColor.mR,mG+theColor.mG,mB+theColor.mB,mA+theColor.mA);}
	inline Color operator-(Color &theColor){return Color(mR-theColor.mR,mG-theColor.mG,mB-theColor.mB,mA-theColor.mA);}
	inline Color operator*(Color &theColor){return Color(mR*theColor.mR,mG*theColor.mG,mB*theColor.mB,mA*theColor.mA);}
	inline Color operator/(Color &theColor){return Color(mR/theColor.mR,mG/theColor.mG,mB/theColor.mB,mA/theColor.mA);}
	inline Color operator+(float theVal){return Color(mR+theVal,mG+theVal,mB+theVal,mA+theVal);}
	inline Color operator-(float theVal){return Color(mR-theVal,mG-theVal,mB-theVal,mA-theVal);}
	inline Color operator*(float theVal){return Color(mR*theVal,mG*theVal,mB*theVal,mA*theVal);}
	inline Color operator/(float theVal){return Color(mR/theVal,mG/theVal,mB/theVal,mA/theVal);}

#ifndef USING_DIRECTX
#define RGBA_TO_INT(red,green,blue,alpha)						\
	(															\
		(((int)(alpha*255.0f)&0xFF)<<24)|						\
		(((int)(blue*255.0f)&0xFF)<<16)|						\
		(((int)(green*255.0f)&0xFF)<<8)|						\
		(((int)(red*255.0f)&0xFF))								\
	)
#else
#undef RGBA_TO_INT
#define RGBA_TO_INT(red,green,blue,alpha)						\
	(															\
		(((int)(alpha*255.0f)&0xFF)<<24)|						\
		(((int)(red*255.0f)&0xFF)<<16)|							\
		(((int)(green*255.0f)&0xFF)<<8)|						\
		(((int)(blue*255.0f)&0xFF))								\
	)

#endif

	inline int ToInt() {return RGBA_TO_INT(mR,mG,mB,mA);}
	#ifdef INT_TO_RGBA
	static inline Color FromInt(int theInt) 
	{
		if (theInt==0) return Color(0,0,0,0);
		if (theInt==-1) return Color(1);
		Color aC;
		INT_TO_RGBA(theInt,aC.mR,aC.mG,aC.mB,aC.mA);
		return aC;
	}
	#endif
	//
	// Premultiplied Alpha...
	//
	//inline int ToInt() {return RGBA_TO_INT(mR*mA,mG*mA,mB*mA,mA);}
	//void FromInt(int theValue);

	Color			Spectrum(int theSlot);
	static Color	GetSpectrum(int theSlot=0);
	static Color	GetSpectrum(Color theColor) {return theColor;}
	static inline Color	MakeSpectrum(int theSlot=0) {return GetSpectrum(theSlot);}
	inline bool		IsWhite() {return !((mR+mG+mB+mA)<4.0f);}
	inline bool		IsBlack() {return ((mR+mG+mB)<.000001f);}

	//
	// Helpers for debugging (and making defines).  I put this in so I could gGX.DrawPoints more quickly while developing Hamsterball
	//
	static Color	DebugColor(Color theColor) {return theColor;}
	static Color	DebugColor(int theSlot) {static Color aCC[]={Color(0,0,0),Color(1,0,0),Color(0,1,0),Color(0,0,1),Color(1,.5f,0),Color(1,1,0),Color(0,1,1),Color(1)};return aCC[(theSlot+1)%8];}

	char*			ToString(int thePrecision=2);

	void			RGBToHSV();
	void			HSVToRGB();

public:

	union {float mR;float mH;};
	union {float mG;float mS;};
	union {float mB;float mV;};
	float mA;
};

#ifdef USING_DIRECTX
class IColor : public Color
{
public:
	IColor(int theRed, int theGreen, int theBlue, int theAlpha=255);
	IColor(int theAlpha=255);
};
#else
inline Color IColor(int r,int g,int b,int a=255) {return Color((float)r/255.0f,(float)g/255.0f,(float)b/255.0f,(float)a/255.0f);}
inline Color IColor(int a) {return Color(0,0,0,(float)a/255.0f);}
#endif

/*
class IColor : public Color
{
public:
	IColor(int theRed, int theGreen, int theBlue, int theAlpha=255);
	IColor(int theAlpha=255);
};
*/

class XColor : public Color
{
public:
	XColor(int theHex);
};

inline Color operator*(const Color lhs, const Color & rhs)
{
    return Color(lhs.mR*rhs.mR,lhs.mG*rhs.mG,lhs.mB*rhs.mB,lhs.mA*rhs.mA);
}

inline Color operator/(const Color lhs, const Color & rhs)
{
    return Color(lhs.mR/rhs.mR,lhs.mG/rhs.mG,lhs.mB/rhs.mB,lhs.mA/rhs.mA);
}

inline Color operator+(const Color lhs, const Color & rhs)
{
    return Color(lhs.mR+rhs.mR,lhs.mG+rhs.mG,lhs.mB+rhs.mB,lhs.mA+rhs.mA).Clip();
}

inline Color operator-(const Color lhs, const Color & rhs)
{
    return Color(lhs.mR-rhs.mR,lhs.mG-rhs.mG,lhs.mB-rhs.mB,lhs.mA-rhs.mA).Clip();
}

extern Color WebColor(char *theHex);

inline Color InterpolateColors(Color theStart, Color theEnd, float theStep)
{
	Color aResult;
	aResult.mR=INTERPOLATE(theStep,theStart.mR,theEnd.mR);
	aResult.mG=INTERPOLATE(theStep,theStart.mG,theEnd.mG);
	aResult.mB=INTERPOLATE(theStep,theStart.mB,theEnd.mB);
	aResult.mA=INTERPOLATE(theStep,theStart.mA,theEnd.mA);
	return aResult;
}

inline Color MixColors(Color theC1, Color theC2, float theMix) {Color aC=theC1;aC.Mix(theC2,theMix);return aC;}
inline Color LerpColors(Color theC1, Color theC2, float theStep) {return InterpolateColors(theC1,theC2,theStep);}

inline int ColorInt(float r, float g, float b, float a=1.0f) {return RGBA_TO_INT(r,g,b,a);}
inline int ColorInt(float a=1.0f) {return RGBA_TO_INT(1,1,1,a);}

extern char *gColorNames[];
#define COLOR_NAME_COUNT (140)

Color GetColorSwatch(int theColor);
Color RandomColor();

