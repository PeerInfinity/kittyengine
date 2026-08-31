#pragma once
#include "rapt_string.h"
#include "rapt_rect.h"
#include "rapt_point.h"
#include "rapt_sprite.h"
#include "rapt_color.h"
#include "rapt_console.h"
//#include "util_core.h"

#ifdef UTF8_PRIVATE_USE

#define CHARSETSIZE 1024
#define ToCharset(x) (x/CHARSETSIZE)
#define ToCharsetChar(x) (x%CHARSETSIZE)

#define HOOKFONTDRAW(func) [&](int theChar, Point thePos, Font::Character* theCharacterInfo)->bool func
#define HOOKFONTDRAWPTR std::function<auto(int theChar, Point thePos, Font::Character* theCharacterInfo)->bool>
// HOOKFONTDRAW ... return false if you drew the character yourself.


class Font
{
public:
	Font();
	virtual ~Font();

	inline void				SetAscent(float theAscent) {mAscent=theAscent;}
	inline void				SetPointSize(float thePointSize) {mPointSize=thePointSize;}
	inline void				SetCharacterWidth(int theCharacter, float theWidth) {CreateCharacter(theCharacter)->mWidth=theWidth;}
	inline void				SetSpaceWidth(float theWidth) {Smart(Character) aC=CreateCharacter(32);aC->mWidth=theWidth;}

	void					ManualLoad(int theTexture, IOBuffer& theBuffer);
	void					ManualLoad(int theCharacter, Sprite& theSprite, Point theOffset);
	void					ManualLoad(int theCharacter, Sprite& theSprite, Point theOffset, float theWidth);

	void					BundleLoad(char theVersion, Array<int>& theTextureList, IOBuffer& theBuffer);


	void					SetupCharacter(int theCharacter, int theWidth, Point theOffset, int theTexture, Rect theTextureRect, int theSpriteWidth, int theSpriteHeight, float theDrawWidth, float theDrawHeight, float theSpriteMoveX, float theSpriteMoveY);
	void					AddKerningPair(String thePair, float theKern);

	void					Draw(StringUTF8 theText, float theX, float theY);
	inline void				Draw(StringUTF8 theText, Point thePos) {Draw(theText,thePos.mX,thePos.mY);}
	inline void				Left(StringUTF8 theText, float theX, float theY) {int aOldJustify=mJustify;Left();Draw(theText,theX,theY);mJustify=aOldJustify;}
	inline void				Left(StringUTF8 theText, Point thePos) {Left(theText,thePos.mX,thePos.mY);}
	inline void				Center(StringUTF8 theText, float theX, float theY) {int aOldJustify=mJustify;Center();Draw(theText,theX,theY);mJustify=aOldJustify;}
	inline void				Center(StringUTF8 theText, Point thePos) {Center(theText,thePos.mX,thePos.mY);}
	inline void				Right(StringUTF8 theText, float theX, float theY) {int aOldJustify=mJustify;Right();Draw(theText,theX,theY);mJustify=aOldJustify;}
	inline void				Right(StringUTF8 theText, Point thePos) {Right(theText,thePos.mX,thePos.mY);}


	float					Width(StringUTF8 theText);
	float					Width(StringUTF8 theText, int breakAt);
	inline float			GetWidth(StringUTF8 theText) {return Width(theText);}
	inline float			GetWidth(StringUTF8 theText, int breakAt) {return Width(theText,breakAt);}
	Point					CharacterPos(StringUTF8 theText, int breakAt);
	inline Point			GetCharacterPos(StringUTF8 theText, int breakAt) {return CharacterPos(theText,breakAt);}
	String					Wrap(StringUTF8 theString, float theWidth);

	float					Height(StringUTF8 theText);
	float					Height(StringUTF8 theText, int breakAt);
	inline float			GetHeight(StringUTF8 theText) {return Height(theText);}
	inline float			GetHeight(StringUTF8 theText, int breakAt) {return Height(theText,breakAt);}

	Point					Size(StringUTF8 theText);
	inline Point			GetSize(StringUTF8 theText) {return Size(theText);}

	//
	// GetJustifyWidth returns pertinent information for justifying text if you're writing it
	// manually.  For instance, if you are left justified, or have no paragraphing, it'll just give you the width of the line.
	// If you're right or center justified with paragraphing, it will give you the width of the LONGEST line.
	//
	float					GetJustifyWidth(StringUTF8 theText, int &lineFeedPosition, int& lineFeedCharacter);

	//
	// Copies a character's data to another location (useful for replacing key formatting characters with exact duplicates, for instance,
	// to have '<' in xml...
	//
	void					CopyCharacter(int theSource, int theDest);


	//
	// To justify the font... gets rid of all the messy
	// multiple draw routines...
	//
	inline void				SetJustify(int theJustify=1) {mJustify=theJustify;}
	inline void				SetJustifyParagraph(bool theState=false) {mJustifyParagraph=theState;}
	//
	inline void				Left() {SetJustify(1);}
	inline void				Center() {SetJustify(0);}
	inline void				Right() {SetJustify(-1);}
	//
	int						mJustify;			// How to justify the text
	bool					mJustifyParagraph;	// If we should justify by the paragraph width, not the line width

	int						mFirstChar;	// First character loaded in font
	int						mLastChar;	// Last character loaded in font
	struct Character
	{
		Character() {mWidth=0;}
		Sprite				mSprite;
		float				mWidth;
		Point				mOffset;
		int					mValue; // Ascii (or UTF8) value.

		CleanArray<float>	mKern;
	};
	struct Charset {Array<Smart(Character)>	mCharacter;};
	Array<Smart(Charset)>	mCharsetList;

	Smart(Character)		mUnknown;	// Unknown character!

	inline float			SpaceWidth() {return GetCharacterPtr(32)->mWidth;}
	inline bool				IsChar(int theSlot) 
	{
		if (theSlot==32) return true;
		if (theSlot<mFirstChar || theSlot>mLastChar) return false;

		int aSet=ToCharset(theSlot);
		if (mCharsetList[aSet].IsNull()) return false;
		return mCharsetList[aSet]->mCharacter[ToCharsetChar(theSlot)].IsNotNull();
	}
	inline Smart(Character)	CreateCharacter(int theSlot)
	{
		if (mFirstChar==-1) mFirstChar=theSlot;else mFirstChar=_min(mFirstChar,theSlot);
		if (mLastChar==-1) mLastChar=theSlot;else mLastChar=_max(mLastChar,theSlot);

		int aSet=ToCharset(theSlot);
		int aChar=ToCharsetChar(theSlot);

		if (mCharsetList[aSet].IsNull()) mCharsetList[aSet]=new Charset;
		if (mCharsetList[aSet]->mCharacter[aChar].IsNull()) mCharsetList[aSet]->mCharacter[aChar]=new Character;
		return mCharsetList[aSet]->mCharacter[aChar];
	}
	inline Smart(Character)	GetCharacter(int theSlot)
	{
		if (mUnknown.IsNull()) mUnknown=new Character;	// kittyengine asset existence guard
		if (theSlot<mFirstChar || theSlot>mLastChar) return mUnknown;

		int aSet=ToCharset(theSlot);
		int aChar=ToCharsetChar(theSlot);
		if (mCharsetList[aSet].IsNull() || aChar>=mCharsetList[aSet]->mCharacter.Size() || mCharsetList[aSet]->mCharacter[aChar].IsNull()) return mUnknown;
		return mCharsetList[aSet]->mCharacter[aChar];
	}
	inline Smart(Character)& GetCharacterRef(int theSlot)
	{
		if (mUnknown.IsNull()) mUnknown=new Character;	// kittyengine asset existence guard
		if (theSlot<mFirstChar || theSlot>mLastChar) return mUnknown;

		int aSet=ToCharset(theSlot);
		int aChar=ToCharsetChar(theSlot);
		if (mCharsetList[aSet].IsNull() || aChar>=mCharsetList[aSet]->mCharacter.Size() || mCharsetList[aSet]->mCharacter[aChar].IsNull()) return mUnknown;
		return mCharsetList[aSet]->mCharacter[aChar];
	}
	inline Character* GetCharacterPtr(int theSlot, bool isGuaranteed=false)
	{
		if (mUnknown.IsNull()) mUnknown=new Character;	// kittyengine asset existence guard
		if (theSlot<mFirstChar || theSlot>mLastChar) 
		{
			if (isGuaranteed) return CreateCharacter(theSlot).GetPointer();
			return mUnknown.GetPointer();
		}

		int aSet=ToCharset(theSlot);
		int aChar=ToCharsetChar(theSlot);
		if (mCharsetList[aSet].IsNull() || aChar>=mCharsetList[aSet]->mCharacter.Size() || mCharsetList[aSet]->mCharacter[aChar].IsNull()) 
		{
			if (isGuaranteed) return CreateCharacter(theSlot).GetPointer();
			return mUnknown.GetPointer();
		}
		return mCharsetList[aSet]->mCharacter[aChar].GetPointer();
	}

	float								mAscent;
	float								mPointSize;
	//float								mKern[256][256];
	//CleanArray<CleanArray<float>>		mKern;

	//
	// The DrawEX calls do some formatting with the text.
	// The format character is normally underscore, so that
	// you can say turn on formatting thus:
	//
	// _i					-> Italics on/off
	// _b					-> Boldface on/off
	// _c[0]				-> Change text color to one of the color slots.  The slots are set up to keep it quick and small
	// _c(1,.5,1)			-> Change text color to rgb.  This is slower!
	// _c					-> Return text color to what it was when we started
	// _s(1.0)				-> Scale the font
	// _o(x,y)				-> Offset drawing by x,y
	//
	// If you need to display the underscore, however, then you
	// can change the format character to be whatever is most
	// appropriate.
	//
	char					mFormatCharacter;
	inline void				SetFormatCharacter(char aCharacter) {mFormatCharacter=aCharacter;}
	inline void				SetFormatCharacter() {mFormatCharacter=0;}
	//
	// Some operations that affect the way the formatting is applied.
	//
	float					mItalicSkewFactor;
	inline void				SetItalicSkewFactor(float theFactor) {mItalicSkewFactor=theFactor;}
	int						mBoldPixels;
	inline void				SetBoldPixels(int thePixels) {mBoldPixels=thePixels;}
	Array<Color>			mColorSlot;
	inline void				SetColorSlot(int theSlot, Color theColor) {if (theSlot>=0) mColorSlot[theSlot]=theColor;}

	void					DrawEX(String theText, float theX, float theY);
	inline void				DrawEX(String theText, Point thePos) {DrawEX(theText,thePos.mX,thePos.mY);}
	inline void				LeftEX(String theText, float theX, float theY) {int aOldJustify=mJustify;Left();DrawEX(theText,theX,theY);mJustify=aOldJustify;}
	inline void				LeftEX(String theText, Point thePos) {LeftEX(theText,thePos.mX,thePos.mY);}
	inline void				CenterEX(String theText, float theX, float theY) {int aOldJustify=mJustify;Center();DrawEX(theText,theX,theY);mJustify=aOldJustify;}
	inline void				CenterEX(String theText, Point thePos) {CenterEX(theText,thePos.mX,thePos.mY);}
	inline void				RightEX(String theText, float theX, float theY) {int aOldJustify=mJustify;Right();DrawEX(theText,theX,theY);mJustify=aOldJustify;}
	inline void				RightEX(String theText, Point thePos) {RightEX(theText,thePos.mX,thePos.mY);}

	float					WidthEX(String theText);
	inline float			GetWidthEX(String theText) {return WidthEX(theText);}

	//
	// Lets you get a RectComplex around part of the drawn thingie... (Used to put hyperlinks into Cubic Castles)
	//
	void					GetHotArea(StringUTF8 theText, RectComplex& theRect, int theStart, int theEnd, Point thePos=Point(0,0));
	void					GetHotAreaEX(String theText, RectComplex& theRect, int theStart, int theEnd, Point thePos=Point(0,0));


	//
	// Removes format characters from the string...
	//
	String					Unformat(StringUTF8 theText);

	//
	// This gives us a more specialized way of getting font width, so that
	// we can handle a centered or right-justified multi-line text...
	//
	float					GetJustifyWidthEX(String theText, int &lineFeedPosition);
	String					WrapEX(String theString, float theWidth);
	String					Truncate(StringUTF8 theString, float theWidth, String theTruncateString="...");

	//
	// You can use this to point all lowercase letters as uppercase, if you have
	// a font where they look similar, and you don't want to include them...
	//
	void					PointLowercaseAtUppercase();

	//
	// Extra spacing lets us put more space between characters, in case we want to space something out
	//
	float					mExtraSpacing;
	inline void				SetExtraSpacing(float theSpacing) {mExtraSpacing=theSpacing;}

	//
	// CRSpacing is extra carriage return spacing.  You can mod it to customize the font when encountering /r
	//
	float					mCRSpacing;
	inline void				SetCRSpacing(float theAmount) {mCRSpacing=theAmount;}

	//
	// Use GetBoundingRect to get the bounding rect of text displayed at a position...
	//
	Rect					GetBoundingRect(StringUTF8 theText, float theX, float theY, int theJustify=1);
	inline Rect				GetBoundingRect(StringUTF8 theText, Point thePos, int theJustify=1) {return GetBoundingRect(theText,thePos.mX,thePos.mY,theJustify);}
	inline Rect				GetBoundingRect(StringUTF8 theText) {return GetBoundingRect(theText,0,0,1);}

	Rect					GetBoundingRectEX(StringUTF8 theText, float theX, float theY, int theJustify=1);
	inline Rect				GetBoundingRectEX(StringUTF8 theText, Point thePos, int theJustify=1) {return GetBoundingRectEX(theText,thePos.mX,thePos.mY,theJustify);}
	inline Rect				GetBoundingRectEX(StringUTF8 theText) {return GetBoundingRectEX(theText,0,0,1);}

	//
	// Offsets the font's drawing
	//
	inline void				TweakOffset(Point theOffset, int theStart=-1, int theEnd=-1) 
	{
		if (theStart==-1) theStart=mFirstChar;
		if (theEnd==-1) theEnd=mLastChar;
		for (int aCount=theStart;aCount<theEnd;aCount++) 
		{
			Smart(Character) aChar=GetCharacter(aCount);
			if (aChar!=mUnknown) aChar->mOffset+=theOffset;
		}
	}

	inline void				TweakWidth(float theTweak, int theStart=-1, int theEnd=-1)
	{
		if (theStart==-1) theStart=mFirstChar;
		if (theEnd==-1) theEnd=mLastChar;
		for (int aCount=theStart;aCount<theEnd;aCount++) 
		{
			Smart(Character) aChar=GetCharacter(aCount);
			if (aChar!=mUnknown) aChar->mWidth+=theTweak;
		}
	}

	HOOKFONTDRAWPTR			mDrawHook=NULL; // Function to run before drawing each letter... Sends theChar, thePos
												// return TRUE if you drew the char, return FALSE if you didn't draw.

	inline void	SetDrawHook(HOOKFONTDRAWPTR theHook) {mDrawHook=theHook;} 
};

extern bool gSharpenFonts;
inline void SharpenFonts(bool theState=true) {gSharpenFonts=theState;}

#else
#pragma once
#include "rapt_string.h"
#include "rapt_rect.h"
#include "rapt_point.h"
#include "rapt_sprite.h"
#include "rapt_color.h"

class Font
{
public:
	Font();
	virtual ~Font();

	inline void				SetAscent(float theAscent) {mAscent=theAscent;}
	inline void				SetPointSize(float thePointSize) {mPointSize=thePointSize;}
	inline void				SetCharacterWidth(int theCharacter, float theWidth) {mCharacter[theCharacter].mIsInitialized=true;mCharacter[theCharacter].mWidth=theWidth;}
	inline void				SetSpaceWidth(float theWidth) {SetCharacterWidth(32,theWidth);}

	void					ManualLoad(int theTexture, IOBuffer& theBuffer);
	void					ManualLoad(int theCharacter, Sprite& theSprite, Point theOffset);
	void					ManualLoad(int theCharacter, Sprite& theSprite, Point theOffset, float theWidth);
	void					SetupCharacter(int theCharacter, int theWidth, Point theOffset, int theTexture, Rect theTextureRect, int theSpriteWidth, int theSpriteHeight, float theDrawWidth, float theDrawHeight, float theSpriteMoveX, float theSpriteMoveY);
	void					AddKerningPair(String thePair, float theKern);

	void					Draw(String theText, float theX, float theY);
	inline void				Draw(String theText, Point thePos) {Draw(theText,thePos.mX,thePos.mY);}
	inline void				Left(String theText, float theX, float theY) {int aOldJustify=mJustify;Left();Draw(theText,theX,theY);mJustify=aOldJustify;}
	inline void				Left(String theText, Point thePos) {Left(theText,thePos.mX,thePos.mY);}
	inline void				Center(String theText, float theX, float theY) {int aOldJustify=mJustify;Center();Draw(theText,theX,theY);mJustify=aOldJustify;}
	inline void				Center(String theText, Point thePos) {Center(theText,thePos.mX,thePos.mY);}
	inline void				Right(String theText, float theX, float theY) {int aOldJustify=mJustify;Right();Draw(theText,theX,theY);mJustify=aOldJustify;}
	inline void				Right(String theText, Point thePos) {Right(theText,thePos.mX,thePos.mY);}


	float					Width(String theText);
	float					Width(String theText, int breakAt);
	inline float			GetWidth(String theText) {return Width(theText);}
	inline float			GetWidth(String theText, int breakAt) {return Width(theText,breakAt);}
	Point					CharacterPos(String theText, int breakAt);
	inline Point			GetCharacterPos(String theText, int breakAt) {return CharacterPos(theText,breakAt);}
	String					Wrap(String theString, float theWidth);

	float					Height(String theText);
	float					Height(String theText, int breakAt);
	inline float			GetHeight(String theText) {return Height(theText);}
	inline float			GetHeight(String theText, int breakAt) {return Height(theText,breakAt);}

	Point					Size(String theText);
	inline Point			GetSize(String theText) {return Size(theText);}

	//
	// GetJustifyWidth returns pertinent information for justifying text if you're writing it
	// manually.  For instance, if you are left justified, or have no paragraphing, it'll just give you the width of the line.
	// If you're right or center justified with paragraphing, it will give you the width of the LONGEST line.
	//
	float					GetJustifyWidth(String theText, int &lineFeedPosition, char& lineFeedCharacter);

	//
	// Copies a character's data to another location (useful for replacing key formatting characters with exact duplicates, for instance,
	// to have '<' in xml...
	//
	void					CopyCharacter(int theSource, int theDest);


	//
	// To justify the font... gets rid of all the messy
	// multiple draw routines...
	//
	inline void				SetJustify(int theJustify=1) {mJustify=theJustify;}
	inline void				SetJustifyParagraph(bool theState=false) {mJustifyParagraph=theState;}
	//
	inline void				Left() {SetJustify(1);}
	inline void				Center() {SetJustify(0);}
	inline void				Right() {SetJustify(-1);}
	//
	int						mJustify;			// How to justify the text
	bool					mJustifyParagraph;	// If we should justify by the paragraph width, not the line width

	struct Character
	{
		Character()
		{
			mIsInitialized=false;
			mWidth=0;
		}
		bool				mIsInitialized;

		Sprite				mSprite;
		float				mWidth;
		Point				mOffset;
	} mCharacter[256];

	inline bool				IsChar(unsigned char theSlot) {return mCharacter[theSlot].mIsInitialized;}
	inline float			SpaceWidth() {return mCharacter[32].mWidth;}
	inline Character*		GetCharacterPtr(unsigned char theSlot) {return &mCharacter[theSlot];}


	float					mAscent;
	float					mPointSize;
	float					mKern[256][256];

	//
	// The DrawEX calls do some formatting with the text.
	// The format character is normally underscore, so that
	// you can say turn on formatting thus:
	//
	// _i					-> Italics on/off
	// _b					-> Boldface on/off
	// _c[0]				-> Change text color to one of the color slots.  The slots are set up to keep it quick and small
	// _c(1,.5,1)			-> Change text color to rgb.  This is slower!
	// _c					-> Return text color to what it was when we started
	// _s(1.0)				-> Scale the font
	// _o(x,y)				-> Offset drawing by x,y
	//
	// If you need to display the underscore, however, then you
	// can change the format character to be whatever is most
	// appropriate.
	//
	char					mFormatCharacter;
	inline void				SetFormatCharacter(char aCharacter) {mFormatCharacter=aCharacter;}
	inline void				SetFormatCharacter() {mFormatCharacter=0;}
	//
	// Some operations that affect the way the formatting is applied.
	//
	float					mItalicSkewFactor;
	inline void				SetItalicSkewFactor(float theFactor) {mItalicSkewFactor=theFactor;}
	int						mBoldPixels;
	inline void				SetBoldPixels(int thePixels) {mBoldPixels=thePixels;}
	Array<Color>			mColorSlot;
	inline void				SetColorSlot(int theSlot, Color theColor) {if (theSlot>=0) mColorSlot[theSlot]=theColor;}

	void					DrawEX(String theText, float theX, float theY);
	inline void				DrawEX(String theText, Point thePos) {DrawEX(theText,thePos.mX,thePos.mY);}
	inline void				LeftEX(String theText, float theX, float theY) {int aOldJustify=mJustify;Left();DrawEX(theText,theX,theY);mJustify=aOldJustify;}
	inline void				LeftEX(String theText, Point thePos) {LeftEX(theText,thePos.mX,thePos.mY);}
	inline void				CenterEX(String theText, float theX, float theY) {int aOldJustify=mJustify;Center();DrawEX(theText,theX,theY);mJustify=aOldJustify;}
	inline void				CenterEX(String theText, Point thePos) {CenterEX(theText,thePos.mX,thePos.mY);}
	inline void				RightEX(String theText, float theX, float theY) {int aOldJustify=mJustify;Right();DrawEX(theText,theX,theY);mJustify=aOldJustify;}
	inline void				RightEX(String theText, Point thePos) {RightEX(theText,thePos.mX,thePos.mY);}

	float					WidthEX(String theText);
	inline float			GetWidthEX(String theText) {return WidthEX(theText);}

	//
	// Lets you get a RectComplex around part of the drawn thingie... (Used to put hyperlinks into Cubic Castles)
	//
	void					GetHotArea(String theText, RectComplex& theRect, int theStart, int theEnd, Point thePos=Point(0,0));
	void					GetHotAreaEX(String theText, RectComplex& theRect, int theStart, int theEnd, Point thePos=Point(0,0));


	//
	// Removes format characters from the string...
	//
	String					Unformat(String theText);

	//
	// This gives us a more specialized way of getting font width, so that
	// we can handle a centered or right-justified multi-line text...
	//
	float					GetJustifyWidthEX(String theText, int &lineFeedPosition);
	String					WrapEX(String theString, float theWidth);
	String					Truncate(String theString, float theWidth, String theTruncateString="...");

	//
	// You can use this to point all lowercase letters as uppercase, if you have
	// a font where they look similar, and you don't want to include them...
	//
	void					PointLowercaseAtUppercase();

	//
	// Extra spacing lets us put more space between characters, in case we want to space something out
	//
	float					mExtraSpacing;
	inline void				SetExtraSpacing(float theSpacing) {mExtraSpacing=theSpacing;}

	//
	// CRSpacing is extra carriage return spacing.  You can mod it to customize the font when encountering /r
	//
	float					mCRSpacing;
	inline void				SetCRSpacing(float theAmount) {mCRSpacing=theAmount;}

	//
	// Use GetBoundingRect to get the bounding rect of text displayed at a position...
	//
	//Rect					GetBoundingRect(String theText, float theX=0, float theY=0, int theJustify=1);
	//inline Rect			GetBoundingRect(String theText, Point thePos=Point(0,0), int theJustify=1) {return GetBoundingRect(theText,thePos.mX,thePos.mY,theJustify);}

	Rect					GetBoundingRect(String theText, float theX, float theY, int theJustify=1);
	inline Rect				GetBoundingRect(String theText, Point thePos, int theJustify=1) {return GetBoundingRect(theText,thePos.mX,thePos.mY,theJustify);}
	inline Rect				GetBoundingRect(String theText) {return GetBoundingRect(theText,0,0,1);}


	Rect					GetBoundingRectEX(String theText, float theX, float theY, int theJustify=1);
	inline Rect				GetBoundingRectEX(String theText, Point thePos, int theJustify=1) {return GetBoundingRectEX(theText,thePos.mX,thePos.mY,theJustify);}

	//
	// Offsets the font's drawing
	//
	inline void				TweakOffset(Point theOffset, int theStart=0, int theEnd=256) {for (int aCount=theStart;aCount<theEnd;aCount++) mCharacter[aCount].mOffset+=theOffset;}

};

extern bool gSharpenFonts;
inline void SharpenFonts(bool theState=true) {gSharpenFonts=theState;}
#endif

Point GetLastFontDrawPos();

