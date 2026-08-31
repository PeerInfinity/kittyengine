#include "rapt_color.h"
#include "graphics_core.h"
#include "rapt.h"

char *gColorNames[]=
{
	"AliceBlue","#F0F8FF",
	"AntiqueWhite","#FAEBD7",
	"Aqua","#00FFFF",
	"Aquamarine","#7FFFD4",
	"Azure","#F0FFFF",
	"Beige","#F5F5DC",
	"Bisque","#FFE4C4",
	"Black","#000000",
	"BlanchedAlmond","#FFEBCD",
	"Blue","#0000FF",
	"BlueViolet","#8A2BE2",
	"Brown","#A52A2A",
	"BurlyWood","#DEB887",
	"CadetBlue","#5F9EA0",
	"Chartreuse","#7FFF00",
	"Chocolate","#D2691E",
	"Coral","#FF7F50",
	"CornflowerBlue","#6495ED",
	"Cornsilk","#FFF8DC",
	"Crimson","#DC143C",
	"Cyan","#00FFFF",
	"DarkBlue","#00008B",
	"DarkCyan","#008B8B",
	"DarkGoldenRod","#B8860B",
	"DarkGray","#A9A9A9",
	"DarkGreen","#006400",
	"DarkKhaki","#BDB76B",
	"DarkMagenta","#8B008B",
	"DarkOliveGreen","#556B2F",
	"DarkOrange","#FF8C00",
	"DarkOrchid","#9932CC",
	"DarkRed","#8B0000",
	"DarkSalmon","#E9967A",
	"DarkSeaGreen","#8FBC8F",
	"DarkSlateBlue","#483D8B",
	"DarkSlateGray","#2F4F4F",
	"DarkTurquoise","#00CED1",
	"DarkViolet","#9400D3",
	"DeepPink","#FF1493",
	"DeepSkyBlue","#00BFFF",
	"DimGray","#696969",
	"DodgerBlue","#1E90FF",
	"FireBrick","#B22222",
	"FloralWhite","#FFFAF0",
	"ForestGreen","#228B22",
	"Fuchsia","#FF00FF",
	"Gainsboro","#DCDCDC",
	"GhostWhite","#F8F8FF",
	"Gold","#FFD700",
	"GoldenRod","#DAA520",
	"Gray","#808080",
	"Green","#008000",
	"GreenYellow","#ADFF2F",
	"HoneyDew","#F0FFF0",
	"HotPink","#FF69B4",
	"IndianRed","#CD5C5C",
	"Indigo","#4B0082",
	"Ivory","#FFFFF0",
	"Khaki","#F0E68C",
	"Lavender","#E6E6FA",
	"LavenderBlush","#FFF0F5",
	"LawnGreen","#7CFC00",
	"LemonChiffon","#FFFACD",
	"LightBlue","#ADD8E6",
	"LightCoral","#F08080",
	"LightCyan","#E0FFFF",
	"LightGoldenRodYellow","#FAFAD2",
	"LightGray","#D3D3D3",
	"LightGreen","#90EE90",
	"LightPink","#FFB6C1",
	"LightSalmon","#FFA07A",
	"LightSeaGreen","#20B2AA",
	"LightSkyBlue","#87CEFA",
	"LightSlateGray","#778899",
	"LightSteelBlue","#B0C4DE",
	"LightYellow","#FFFFE0",
	"Lime","#00FF00",
	"LimeGreen","#32CD32",
	"Linen","#FAF0E6",
	"Magenta","#FF00FF",
	"Maroon","#800000",
	"MediumAquaMarine","#66CDAA",
	"MediumBlue","#0000CD",
	"MediumOrchid","#BA55D3",
	"MediumPurple","#9370DB",
	"MediumSeaGreen","#3CB371",
	"MediumSlateBlue","#7B68EE",
	"MediumSpringGreen","#00FA9A",
	"MediumTurquoise","#48D1CC",
	"MediumVioletRed","#C71585",
	"MidnightBlue","#191970",
	"MintCream","#F5FFFA",
	"MistyRose","#FFE4E1",
	"Moccasin","#FFE4B5",
	"NavajoWhite","#FFDEAD",
	"Navy","#000080",
	"OldLace","#FDF5E6",
	"Olive","#808000",
	"OliveDrab","#6B8E23",
	"Orange","#FFA500",
	"OrangeRed","#FF4500",
	"Orchid","#DA70D6",
	"PaleGoldenRod","#EEE8AA",
	"PaleGreen","#98FB98",
	"PaleTurquoise","#AFEEEE",
	"PaleVioletRed","#DB7093",
	"PapayaWhip","#FFEFD5",
	"PeachPuff","#FFDAB9",
	"Peru","#CD853F",
	"Pink","#FFC0CB",
	"Plum","#DDA0DD",
	"PowderBlue","#B0E0E6",
	"Purple","#800080",
	"RebeccaPurple","#663399",
	"Red","#FF0000",
	"RosyBrown","#BC8F8F",
	"RoyalBlue","#4169E1",
	"SaddleBrown","#8B4513",
	"Salmon","#FA8072",
	"SandyBrown","#F4A460",
	"SeaGreen","#2E8B57",
	"SeaShell","#FFF5EE",
	"Sienna","#A0522D",
	"Silver","#C0C0C0",
	"SkyBlue","#87CEEB",
	"SlateBlue","#6A5ACD",
	"SlateGray","#708090",
	"Snow","#FFFAFA",
	"SpringGreen","#00FF7F",
	"SteelBlue","#4682B4",
	"Tan","#D2B48C",
	"Teal","#008080",
	"Thistle","#D8BFD8",
	"Tomato","#FF6347",
	"Turquoise","#40E0D0",
	"Violet","#EE82EE",
	"Wheat","#F5DEB3",
	"White","#FFFFFF",
	"WhiteSmoke","#F5F5F5",
	"Yellow","#FFFF00",
	"YellowGreen","#9ACD32",
	"Lilac","#C8A2C8",

	"xxx","#000000"
};




Color::Color()
{
	mR=mG=mB=1.0f;
	mA=1.0f;
}

Color::Color(float theAlpha)
{
	mR=mG=mB=1;
	mA=theAlpha;
}

Color::Color(int theAlpha)
{
	mR=mG=mB=1;
	mA=(float)theAlpha;
}

Color::Color(float theRed, float theGreen, float theBlue, float theAlpha)
{
	mR=theRed;
	mG=theGreen;
	mB=theBlue;
	mA=theAlpha;
}

Color::Color(char* theColor)
{
	FromText(theColor);
}


void Color::Mix(Color theColor, float thePercent, bool  Alpha)
{
	float aInversePercent=1.0f-thePercent;
	mR=mR*aInversePercent+theColor.mR*thePercent;
	mG=mG*aInversePercent+theColor.mG*thePercent;
	mB=mB*aInversePercent+theColor.mB*thePercent;

	if ( Alpha)
	{
		mA=mA*aInversePercent+theColor.mA*thePercent;
	}
}

Color Color::Pastel(float theThreshold)
{
	Color aNewColor=*this;

	aNewColor*=1.5f;
	aNewColor=aNewColor.Clip();

	aNewColor.mR=_max(theThreshold,aNewColor.mR);
	aNewColor.mG=_max(theThreshold,aNewColor.mG); 
	aNewColor.mB=_max(theThreshold,aNewColor.mB);

	return aNewColor;

/*
	*this*=1.5f;
	*this=Clip();
	mR=_max(theThreshold,mR);
	mG=_max(theThreshold,mG);
	mB=_max(theThreshold,mB);
	return *this;
*/
}

#ifdef USING_DIRECTX
IColor::IColor(int theRed, int theGreen, int theBlue, int theAlpha)
{
	mR=(float)theRed/255.0f;
	mG=(float)theGreen/255.0f;
	mB=(float)theBlue/255.0f;
	mA=(float)theAlpha/255.0f;
}

IColor::IColor(int theAlpha)
{
	mR=1.0f;
	mG=1.0f;
	mB=1.0f;
	mA=(float)theAlpha/255.0f;
}
#endif

XColor::XColor(int theHex)
{
	if (theHex&0xFF000000) mA=(float)((theHex&0xFF000000)>>24)/255.0f;
	else mA=1.0f;

	mR=(float)((theHex&0x00FF0000)>>16)/255.0f;
	mG=(float)((theHex&0x0000FF00)>>8)/255.0f;
	mB=(float)((theHex&0x000000FF))/255.0f;
}

Color Color::GetSpectrum(int theSlot)
{
	Color aC;
	aC.Spectrum(theSlot);
	return aC;
}

Color Color::Spectrum(int theSlot)
{
	theSlot%=10;
	switch (theSlot)
	{
	//Swatch
	case 0:Set(1,0,0);break;
	case 1:Set(1,.5f,0);break;
	case 2:Set(1,1,0);break;
	case 3:Set(.5f,1,0);break;
	case 4:Set(0,1,0);break;
	case 5:Set(0,1,1);break;
	case 6:Set(0,.5f,1);break;
	case 7:Set(0,0,1);break;
	case 8:Set(.5f,0,1);break;
	case 9:Set(1,0,1);break;
	}
	return *this;
}

Color Color::Desaturate(float thePercent)
{
	//float aGray=(mR+mG+mB)/3;

	float aGray=mR*0.3086f+mG*0.6094f+mB*0.0820f;

	Color aNewColor;
	aNewColor.mR=mR*(1-thePercent)+aGray*(thePercent);
	aNewColor.mG=mG*(1-thePercent)+aGray*(thePercent);
	aNewColor.mB=mB*(1-thePercent)+aGray*(thePercent);
	aNewColor.mA=mA;
	return aNewColor.Clip();
}

Color Color::Darken(float thePercent)
{
	Color aNewColor;
	aNewColor.mR=mR*(1-thePercent);
	aNewColor.mG=mG*(1-thePercent);
	aNewColor.mB=mB*(1-thePercent);
	aNewColor.mA=mA;
	return aNewColor;
}

Color Color::Lighten(float thePercent)
{
	Color aNewColor;
	aNewColor.mR=mR*(1+thePercent);
	aNewColor.mG=mG*(1+thePercent);
	aNewColor.mB=mB*(1+thePercent);
	aNewColor.mA=mA;
	return aNewColor.Clip();
}

void Color::FromWeb(char* theHex)
{
	String aString=theHex;
	aString.Remove("#");
	aString.Insert("0x",0);
	
	int aResult=0;
	sscanf(aString.c(), "0x%X", &aResult);

	mB=(aResult&255)/255.0f;
	mG=((aResult>>8)&255)/255.0f;
	mR=((aResult>>16)&255)/255.0f;
	mA=1.0f;
}

char* Color::ToWeb()
{
	int aValue=
		(((int)(mR*255.0f)&0xFF)<<16)|
		(((int)(mG*255.0f)&0xFF)<<8)|
		(((int)(mB*255.0f)&0xFF));

	static char aData[25];
	sprintf(aData,"#%06X",aValue);
	return aData;
}

void Color::FromText(char* theRGB)
{
	String aString=theRGB;
	if (aString.Len()==0)
	{
		mR=mG=mB=mA=1.0f;
		return;
	}
	if (aString[0]=='#')
	{
		FromWeb(theRGB);
		return;
	}
	if (!aString.Contains(',') && !aString.IsNumber())
	{
		aString.RemoveLeadingSpaces();
		aString.RemoveTrailingSpaces();
		for (int aCount=0;aCount<99999;aCount+=2) {if (strcmp(gColorNames[aCount],"xxx")==0) break;if (gColorNames[aCount]==aString) {FromWeb(gColorNames[aCount+1]);return;}}
		mR=mG=mB=mA=1.0f;
		return;
	}

	aString.Remove("f");
	String aR=aString.GetToken(',');
	String aG=aString.GetNextToken(',');
	String aB=aString.GetNextToken(',');
	String aA=aString.GetNextToken(',');

	if (aR.Len() && !aG.Len()) {mR=aR.ToFloat();mG=mR;mB=mR;mA=1.0f;}
	else if (aR.Len() && aG.Len() && aB.Len() && !aA.Len())
	{
		mR=aR.ToFloat();
		mG=aG.ToFloat();
		mB=aB.ToFloat();
		mA=1.0f;
	}
	else if (aR.Len() && aG.Len() && aB.Len() && aA.Len())
	{ 
		mR=aR.ToFloat();
		mG=aG.ToFloat();
		mB=aB.ToFloat();
		mA=aA.ToFloat();
	}
}


Color WebColor(char *theHex)
{
	Color aColor;
	aColor.FromWeb(theHex);
	return aColor;
}

Color Color::Interpolate(Color theFinalColor, float thePercent)
{
	thePercent=_clamp(0,thePercent,1);
	Color aResult;
	aResult.mR=gMath.InterpolateLinear(mR,theFinalColor.mR,thePercent);
	aResult.mG=gMath.InterpolateLinear(mG,theFinalColor.mG,thePercent);
	aResult.mB=gMath.InterpolateLinear(mB,theFinalColor.mB,thePercent);
	aResult.mA=gMath.InterpolateLinear(mA,theFinalColor.mA,thePercent);
	return aResult;
}

/*
void Color::FromInt(int theValue)
{
	unsigned char aA,aR,aG,aB;
	INT_TO_RGBA(theValue,aR,aG,aB,aA);

	mA=(float)aA/255.0f;
	mR=(float)aR/255.0f;
	mG=(float)aG/255.0f;
	mB=(float)aB/255.0f;
}
 */

char* Color::ToString(int thePrecision)
{
	String& aStr=GetDiscardableString();
	char* aFormat="%.2f,%.2f,%.2f,%.2f";
	switch (thePrecision)
	{
	case 0: aFormat="%.0f,%.0f,%.0f,%.0f";break;
	case 1: aFormat="%.1f,%.1f,%.1f,%.1f";break;
	case 2: aFormat="%.3f,%.3f,%.3f,%.3f";break;
	case 3: aFormat="%.4f,%.4f,%.4f,%.4f";break;
	}
	aStr=Sprintf(aFormat,mR,mG,mB,mA);
	return aStr.c();
}

Color GetColorSwatch(int theColor)
{
	Color aC;
	aC.Spectrum(theColor);
	return aC;
}

void Color::RGBToHSV()
{
	float aH,aS,aV;
	float aMin=_min(_min(mR,mG),mB);
	float aMax=_max(_max(mR,mG),mB);

	aV=aMax;
	float aDelta=aMax-aMin;
	if(aMax!=0) aS=aDelta/aMax;
	else {aS=0;aH=-1;mV=aV;mH=aH;mS=aS;return;} // rgb 0,0,0 doesn't work
	if(mR==aMax) aH=(mG-mB)/aDelta;      // between yellow & magenta
	else if(mG==aMax) aH=2.0f+(mB-mR)/aDelta;  // between cyan & yellow
	else aH=4.0f+(mR-mG)/aDelta;  // between magenta & cyan

	aH*=60;                // degrees
	if(aH<0) aH+=360;
	if (aH!=aH) aH=0; // NAN

	mH=aH;
	mS=aS;
	mV=aV;
}


void Color::HSVToRGB()
{
	float aR,aG,aB;
	float aF,aP,aQ,aT;
	int aI;

	if (mS==0) {aR=aG=aB=mV;mR=aR;mG=aG;mB=aB;return;}

	mH/=60.0f;            // sector 0 to 5
	aI=(int)floorf(mH);
	aF=mH-aI;          // factorial part of h
	aP=mV*(1.0f-mS);
	aQ=mV*(1.0f-mS*aF);
	aT=mV*(1.0f-mS*(1.0f-aF));
	switch(aI) 
	{
	case 0:aR=mV;aG=aT;aB=aP;break;
	case 1:aR=aQ;aG=mV;aB=aP;break;
	case 2:aR=aP;aG=mV;aB=aT;break;
	case 3:aR=aP;aG=aQ;aB=mV;break;
	case 4:aR=aT;aG=aP;aB=mV;break;
	default:aR=mV;aG=aP;aB=aQ;break;
	}
	mR=aR;mG=aG;mB=aB;
}

Color Color::Maximize()
{
	Color aNewColor=*this;
	float aMult=_min(_min(1.0f/mR,1.0f/mG),1.0f/mB);
	aNewColor.mR*=aMult;
	aNewColor.mG*=aMult;
	aNewColor.mB*=aMult;
	return aNewColor;
}

Color RandomColor() {return GetColorSwatch(gRand.Get(100));}
