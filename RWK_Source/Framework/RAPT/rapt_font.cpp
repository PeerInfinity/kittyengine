#include "rapt_font.h"
#include "rapt.h"
bool gSharpenFonts=true;

Point gFontDrawPos;
Point GetLastFontDrawPos() {return gFontDrawPos;}

#ifdef UTF8_PRIVATE_USE

Font::Font()
{
	SetFormatCharacter('_');
	SetItalicSkewFactor(.125f);
	SetBoldPixels(2);
	for (int aCount=0;aCount<10;aCount++) SetColorSlot(aCount,Color().Spectrum(aCount));
	SetJustify(1);
	SetJustifyParagraph(false);
	mFormatCharacter='_';
	mExtraSpacing=0;
	mCRSpacing=0;

	mFirstChar=-1;
	mLastChar=-1;
}

Font::~Font()
{
}

void Font::BundleLoad(char theVersion, Array<int>& theTextureList, IOBuffer& theBuffer)
{
	if (theBuffer.IsEnd()) return;	// kittyengine asset existence guard
	float aSize=theBuffer.ReadFloat();
	float aSpaceWidth=theBuffer.ReadFloat();
	float aAscent=theBuffer.ReadFloat();
	aAscent=aSize;	// For Raptisoft stuff, Ascent==Size.

	SetSpaceWidth(aSpaceWidth);
	SetAscent(aAscent);
	SetPointSize(aSize);

	//
	// Load Kerns...
	//
	for (;;)
	{
		short aC1=theBuffer.ReadShort();
		short aC2=theBuffer.ReadShort();
		if (aC1==0 && aC2==0) break;

		Smart(Character) aCC=CreateCharacter(aC1);
		aCC->mKern[aC2]=theBuffer.ReadFloat();

		//gOut.Out("Kern: %c -> %c = %f",aC1,aC2,aCC->mKern[aC2]);
		//		mKern[aC1][aC2]=theBuffer.ReadFloat();
	}

	for (;;)
	{
		short aChar=theBuffer.ReadShort();if (aChar==0) break;

		Smart(Character) aCC=CreateCharacter(aChar);
		aCC->mWidth=theBuffer.ReadFloat();
		aCC->mOffset=theBuffer.ReadPoint();
		aCC->mSprite.BundleLoad(0,theTextureList,theBuffer);

		if (aChar=='?') if (mUnknown.IsNull()) mUnknown=aCC;
	}

	if (mUnknown.IsNull()) mUnknown=new Character;
}

void Font::ManualLoad(int theTexture, IOBuffer& theBuffer)
{
	if (theBuffer.IsEnd()) return;	// kittyengine asset existence guard
	float aSize=theBuffer.ReadFloat();
	float aSpaceWidth=theBuffer.ReadFloat();
	float aAscent=theBuffer.ReadFloat();

	aAscent=aSize;	// For Raptisoft stuff, Ascent==Size.

	SetSpaceWidth(aSpaceWidth);
	SetAscent(aAscent);
	SetPointSize(aSize);

	//
	// Load Kerns...
	//
	for (;;)
	{
		short aC1=theBuffer.ReadShort();
		short aC2=theBuffer.ReadShort();
		if (aC1==0 && aC2==0) break;

		Smart(Character) aCC=CreateCharacter(aC1);
		aCC->mKern[aC2]=theBuffer.ReadFloat();

		//gOut.Out("Kern: %c -> %c = %f",aC1,aC2,aCC->mKern[aC2]);
//		mKern[aC1][aC2]=theBuffer.ReadFloat();
	}

	for (;;)
	{
		short aChar=theBuffer.ReadShort();if (aChar==0) break;

		Smart(Character) aCC=CreateCharacter(aChar);
		aCC->mWidth=theBuffer.ReadFloat();
		aCC->mOffset=theBuffer.ReadPoint();
		aCC->mSprite.ManualLoad(theTexture,theBuffer);

		if (aChar=='?') if (mUnknown.IsNull()) mUnknown=aCC;
	}

	if (mUnknown.IsNull()) mUnknown=new Character;
}

void Font::ManualLoad(int theCharacter, Sprite& theSprite, Point theOffset)
{
	Smart(Character) aChar=CreateCharacter(theCharacter);
	aChar->mSprite.ManualLoad(theSprite);
	aChar->mWidth=(float)theSprite.mWidth;
	aChar->mOffset=theOffset;

	if (theCharacter=='?') if (mUnknown.IsNull()) mUnknown=aChar;

	//mCharacter[theCharacter].mOffset+=Point(theSprite.HalfWidthF(),theSprite.HalfHeightF());
}

void Font::ManualLoad(int theCharacter, Sprite& theSprite, Point theOffset, float theWidth)
{
	Smart(Character) aChar=CreateCharacter(theCharacter);
	aChar->mSprite.ManualLoad(theSprite);
	aChar->mWidth=(float)theSprite.mWidth;
	aChar->mOffset=theOffset;
	//mCharacter[theCharacter].mOffset+=Point(theSprite.HalfWidthF(),theSprite.HalfHeightF());
	aChar->mWidth=theWidth;

	if (theCharacter=='?') if (mUnknown.IsNull()) mUnknown=aChar;
}


void Font::SetupCharacter(int theCharacter, int theWidth, Point theOffset, int theTexture, Rect theTextureRect, int theSpriteWidth, int theSpriteHeight, float theDrawWidth, float theDrawHeight, float theSpriteMoveX, float theSpriteMoveY)
{
	Smart(Character) aChar=CreateCharacter(theCharacter);

	aChar->mSprite.ManualLoad(theTexture,theTextureRect,theSpriteWidth,theSpriteHeight,theDrawWidth,theDrawHeight,theSpriteMoveX,theSpriteMoveY);
	aChar->mWidth=(float)theWidth;
	aChar->mOffset=theOffset;

	if (theCharacter=='?') if (mUnknown.IsNull()) mUnknown=aChar;

	//mCharacter[theCharacter].mSprite.GetTextureQuad().LowerLeft().mY+=.5f;
	//mCharacter[theCharacter].mSprite.GetTextureQuad().LowerRight().mY+=.5f;
}

void Font::AddKerningPair(String thePair, float theKern)
{
	if (thePair[0]>0 && thePair[1]>0) 
	{
		Smart(Character) aC=CreateCharacter(thePair[0]);
		aC->mKern[thePair[1]]=theKern;
		//mKern[thePair[0]][thePair[1]]=theKern;
	}
}

void Font::Draw(StringUTF8 theText, float theX, float theY)
{
	int aLastChar=0;
	if (mJustify!=1)
	{
		int aLineFeedPosition;
		int aLineFeedChar;
		float aWidth=GetJustifyWidth(theText,aLineFeedPosition,aLineFeedChar);
		if (mJustify==0) aWidth/=2;
		if (aLineFeedPosition!=-1 && !mJustifyParagraph)
		{

			Draw(theText.GetSegment(0,aLineFeedPosition),theX,theY);
			theY+=mPointSize+1;
			if (aLineFeedChar==13) 
			{
				theY+=mPointSize+1;
				theY+=mCRSpacing;
			}
			Draw(theText.GetSegment(aLineFeedPosition+1,9999),theX,theY);
			return;
		}
		theX-=aWidth;
	}

	gG.PushTranslate();
	if (gSharpenFonts) gG.TranslateToIntegers();

	//gG.Translate(0.0f,-mAscent);
	Point aStartPos=Point((int)theX,(int)theY);
	Point aPos=aStartPos;
	theText.StartUTF8();
	for (;;)
	//for (int aCount=0;aCount<theText.Len();aCount++)
	{
		int aChar=theText.GetUTF8();if (aChar==0) break;
		
		//unsigned char aChar=theText[aCount];
		if (aChar==10)
		{
			aPos.mX=aStartPos.mX;
			aPos.mY+=mPointSize+1;
		}
		else if (aChar==13)
		{
			aPos.mX=aStartPos.mX;
			aPos.mY+=mPointSize+1;
			aPos.mY+=mPointSize+1;
			aPos.mY+=mCRSpacing;
		}
		else
		{
			Character* aCPtr=GetCharacterPtr(aChar);
			if (aLastChar>0) aPos.mX+=GetCharacterPtr(aLastChar)->mKern[aChar];
//			aPos.mX+=mKern[aLastChar][aChar];
			if (mDrawHook) {if (!mDrawHook(aChar,aPos+aCPtr->mOffset,aCPtr)) aCPtr->mSprite.Center(aPos+aCPtr->mOffset);}
			else aCPtr->mSprite.Center(aPos+aCPtr->mOffset);
			aPos.mX+=aCPtr->mWidth+mExtraSpacing;
		}
		aLastChar=aChar;
	}

	gFontDrawPos=aPos;
	//gG.Translate(0.0f,mAscent);

	gG.PopTranslate();

}

float Font::Width(StringUTF8 theText)
{
	int aLastChar=0;
	float aResult=0;
	float aMaxWidth=0;
	theText.StartUTF8();
	//for (int aCount=0;aCount<theText.Len();aCount++)
	for (;;)
	{
		int aChar=theText.GetUTF8();if (aChar==0) break;

		if (aChar==10 || aChar==13)
		{
			aMaxWidth=_max(aMaxWidth,aResult);
			aResult=0;
		}
		else
		{
			if (aLastChar>0) aResult+=GetCharacterPtr(aLastChar)->mKern[aChar];
			//aResult+=mKern[aLastChar][aChar];
			aResult+=GetCharacterPtr(aChar)->mWidth+mExtraSpacing;
		}
		aLastChar=aChar;
	}

	aResult=_max(aResult,aMaxWidth);
	return aResult;
}

float Font::Height(StringUTF8 theText)
{
	float aResult=mPointSize;
	bool aNextChar=false;
	bool aDouble=false;

	unsigned char aPrevChar=-1;
	theText.StartUTF8();
	//for (int aCount=0;aCount<theText.Len();aCount++)
	for (;;)
	{
		int aChar=theText.GetUTF8();if (aChar==0) break;

		if (aChar==10) aNextChar=true;
		else if (aChar==13) {aNextChar=true;aDouble=true;}
		else if (aNextChar)
		{
			aNextChar=false;
			aResult+=mPointSize+1;
			if (aDouble) 
			{
				aResult+=mPointSize+1;
				aResult+=mCRSpacing;
			}
			aDouble=false;
		}

		if (aNextChar && aPrevChar==10) {aResult+=mPointSize+1;}
		if (aNextChar && aPrevChar==13) {aResult+=mPointSize+1;aResult+=mPointSize+1;aResult+=mCRSpacing;}
		aPrevChar=aChar;
	}
	return aResult;
}

Point Font::Size(StringUTF8 theText)
{
	int aLastChar=0;
	Point aResult;
	aResult.mY=mPointSize;

	bool aNextChar=false;
	bool aDouble=false;

	float aMaxWidth=0;
	theText.StartUTF8();
	//for (int aCount=0;aCount<theText.Len();aCount++)
	for (;;)
	{
		int aChar=theText.GetUTF8();if (aChar==0) break;
		if (aChar==10 || aChar==13)
		{
			aNextChar=true;
			if (aChar==13) aDouble=true;

			aMaxWidth=_max(aMaxWidth,aResult.mX);
			aResult.mX=0;
		}
		else 
		{
			if (aNextChar)
			{
				aNextChar=false;
				aResult.mY+=mPointSize+1;
				if (aDouble) 
				{
					aResult.mY+=mPointSize+1;
					aResult.mY+=mCRSpacing;
				}
				aDouble=false;
			}
			if (aLastChar>0) aResult.mX+=GetCharacterPtr(aLastChar)->mKern[aChar];
			//aResult.mX+=mKern[aLastChar][aChar];
			aResult.mX+=GetCharacterPtr(aChar)->mWidth+mExtraSpacing;
		}

		if (aNextChar && aLastChar==10) {aResult.mY+=mPointSize+1;}
		if (aNextChar && aLastChar==13) {aResult.mY+=mPointSize+1;aResult.mY+=mPointSize+1;aResult+=mCRSpacing;}
		aLastChar=aChar;
	}

	aResult.mX=_max(aResult.mX,aMaxWidth);
	return aResult;
}

float Font::Width(StringUTF8 theText, int breakAt)
{
	int aLastChar=0;
	float aResult=0;
	float aMaxWidth=0;

	int aCount=0;

	theText.StartUTF8();
	//for (int aCount=0;aCount<theText.Len();aCount++)
	for (;;)
	{
		if (aCount==breakAt) break;
		aCount++;

		int aChar=theText.GetUTF8();if (aChar==0) break;

		if (aChar==10 || aChar==13)
		{
			aMaxWidth=_max(aMaxWidth,aResult);
			aResult=0;
		}
		else 
		{
			if (aLastChar>0) aResult+=GetCharacterPtr(aLastChar)->mKern[aChar];
			//aResult+=mKern[aLastChar][aChar];
			aResult+=GetCharacterPtr(aChar)->mWidth+mExtraSpacing;
		}
		aLastChar=aChar;
	}
	aResult=_max(aResult,aMaxWidth);
	return aResult;
}


float Font::Height(StringUTF8 theText, int breakAt)
{
	float aResult=mPointSize;
	bool aNextChar=false;
	unsigned char aPrevChar=-1;
	bool aDouble=false;

	int aCount=0;

	theText.StartUTF8();
	//for (int aCount=0;aCount<theText.Len();aCount++)

	for (;;)
	{

		if (aCount==breakAt) break;
		aCount++;
		int aChar=theText.GetUTF8();if (aChar==0) break;

		if (aChar==10) aNextChar=true;
		else if (aChar==13) {aNextChar=true;aDouble=true;}
		else if (aNextChar)
		{
			aNextChar=false;
			aResult+=mPointSize+1;
			if (aDouble) aResult+=mPointSize+1;
			aDouble=false;
		}
		if (aNextChar && aPrevChar==10) {aResult+=mPointSize+1;}
		if (aNextChar && aPrevChar==13) {aResult+=mPointSize+1;aResult+=mPointSize+1;}
		aPrevChar=aChar;
	}
	return aResult;
}


void Font::DrawEX(String theText, float theX, float theY)
{
	if (mJustify!=1)
	{
		int aLineFeedPosition;
		float aWidth=GetJustifyWidthEX(theText,aLineFeedPosition);
		if (mJustify==0) aWidth/=2;
		if (aLineFeedPosition!=-1 && !mJustifyParagraph)
		{
			DrawEX(theText.GetSegment(0,aLineFeedPosition),theX,theY);
			theY+=mPointSize+1;
			DrawEX(theText.GetSegment(aLineFeedPosition+1,9999),theX,theY);
			return;
		}
		theX-=aWidth;
	}

	gG.PushTranslate();
	if (gSharpenFonts) gG.TranslateToIntegers();

	Color aNormalColor=gG.GetColor();
	int aLastChar=0;
	float aISkew=mPointSize*mItalicSkewFactor;

	int aEffect=0;
	bool aBold=false;
	bool aItalics=false;
	float aScale=1.0f;

	//gG.Translate(0.0f,-mAscent);
	Point aStartPos=Point((int)theX,(int)theY);
	Point aPos=aStartPos;

	bool aHoldFilter=gG.mFilter;
	if (!aHoldFilter)
	{
		//
		// If we're filtering, let's go ahead and put
		// our text on int boundaries.
		//

		aStartPos=IPoint(aStartPos.mX,aStartPos.mY);
		aPos=IPoint(aPos.mX,aPos.mY);
	}

	Point aOffset=Point(0,0);
	for (int aCount=0;aCount<theText.Len();aCount++)
	{
		unsigned char aChar=theText[aCount];
		if (aChar==10)
		{
			aPos.mX=aStartPos.mX;
			aPos.mY+=mPointSize+1;
		}
		else if (aChar==13)
		{
			aPos.mX=aStartPos.mX;
			aPos.mY+=mPointSize+1;
			aPos.mY+=mPointSize+1;
			aPos.mY+=mCRSpacing;
		}
		else if (aChar==mFormatCharacter)
		{
			aCount++;
			unsigned char aFormat=tolower(theText[aCount]);
			
			if (aFormat=='i') aItalics=!aItalics;
			if (aFormat=='b') aBold=!aBold;
			if (aFormat=='s')
			{
				int aCPos=aCount+1;
				if (theText[aCPos]=='(')
				{
					int aStart=aCPos+1;
					for (;;)
					{
						aCount++;
						if (theText[aCount]==')' || theText[aCount]==0)
						{
							aScale=theText.GetSegment(aStart,aCount-aStart).ToFloat();
							break;
						}
					}
				}

				if (aScale!=1.0f) gG.Sharpen(false);
				else gG.Sharpen(!aHoldFilter);
			}
			if (aFormat=='o')
			{
				int aCPos=aCount+1;
				if (theText[aCPos]=='(')
				{
					int aStart=aCPos+1;
					for (;;)
					{
						aCount++;
						if (theText[aCount]==')' || theText[aCount]==0)
						{
							String aSeg=theText.GetSegment(aStart,aCount-aStart);

							if (aSeg.Len()<1) aOffset=Point(0,0);
							else
							{
								float aX=aSeg.GetToken(',').ToFloat();
								float aY=aSeg.GetNextToken(',').ToFloat();
								aOffset=Point(aX,aY);
							}
							break;
						}

					}
				}
			}
			if (aFormat=='c')
			{
				int aCPos=aCount+1;
				if (theText[aCPos]=='[')
				{
					int aValue=theText[aCPos+1]-48;
					gG.SetColor(mColorSlot[aValue]);
					aCount+=2;
					if (theText[aCPos+2]==']') aCount++;
				}
				else if (theText[aCPos]=='(')
				{
					int aStart=aCPos+1;
					for (;;)
					{
						aCount++;
						if (theText[aCount]==')' || theText[aCount]==0)
						{
							float aAlpha=1.0f;
							String aSeg=theText.GetSegment(aStart,aCount-aStart);
							float aR=aSeg.GetToken(',').ToFloat();
							float aG=aSeg.GetNextToken(',').ToFloat();
							float aB=aSeg.GetNextToken(',').ToFloat();
							String aAA=aSeg.GetNextToken(',');if (aAA.Len()>0) aAlpha=aAA.ToFloat();
							gG.SetColor(aR,aG,aB,aAlpha);
							break;
						}

					}

				}
				else
				{
					gG.SetColor(aNormalColor);
				}
				//
				// Change text color...
				//
			}
			aEffect=aItalics|aBold;
		}
		else
		{
			Character* aCPtr=GetCharacterPtr(aChar);
			if (aLastChar>0) aPos.mX+=GetCharacterPtr(aLastChar)->mKern[aChar]*aScale;
			//aPos.mX+=(float)(mKern[aLastChar][aChar])*aScale;
//				if (!aEffect) mCharacter[aChar].mSprite.DrawScaled(aPos+aOffset+(mCharacter[aChar].mOffset*aScale)+mCharacter[aChar].mSprite.GetHalfSize(),aScale);
			if (!aEffect) aCPtr->mSprite.DrawScaled(aPos+aOffset+(aCPtr->mOffset*aScale),aScale);
//				if (!aEffect) mCharacter[aChar].mSprite.Draw(aPos+mCharacter[aChar].mOffset);
			else
			{
				Quad aDrawQuad=aCPtr->mSprite.GetDrawQuad();
				if (aItalics) 
				{
					aDrawQuad.UpperLeft().mX+=aISkew;
					aDrawQuad.UpperRight().mX+=aISkew;
					aDrawQuad.LowerLeft().mX-=aISkew;
					aDrawQuad.LowerRight().mX-=aISkew;
				}
				if (aScale!=.0f) aDrawQuad.Scale(aScale);

				gG.PushTranslate();
				gG.Translate((aPos+aOffset+aCPtr->mOffset));//+mCharacter[aChar].mSprite.GetHalfSize()));
				gG.SetTexture(aCPtr->mSprite.mTexture);

				if (aBold)
				{
					//gG.Sharpen(false);
					//gG.Translate(-(float)(mBoldPixels+1)/2,0.0f);
					for (int aCount=0;aCount<mBoldPixels;aCount++)
					{
						gG.DrawTexturedQuad(aDrawQuad,aCPtr->mSprite.GetTextureQuad());
						gG.Translate(1,0);
					}
					//gG.Sharpen(!aHoldFilter);
				}
				else
				{
					gG.Sharpen(false);
					gG.DrawTexturedQuad(aDrawQuad,aCPtr->mSprite.GetTextureQuad());
					gG.Sharpen(!aHoldFilter);
				}
				gG.PopTranslate();
			}
			aPos.mX+=(aCPtr->mWidth+mExtraSpacing)*aScale;

			aLastChar=aChar;
		}
	}
	gFontDrawPos=aPos;

	//gG.Translate(0.0f,mAscent);
	gG.SetColor(aNormalColor);

	gG.Sharpen(!aHoldFilter);
	gG.PopTranslate();
}

float Font::WidthEX(String theText)
{
	float aResult=0;
	int aLastChar=0;
	float aMaxWidth=0;
	float aScale=1.0f;

	for (int aCount=0;aCount<theText.Len();aCount++)
	{
		unsigned char aChar=theText[aCount];
		if (aChar==10 || aChar==13)
		{
			aMaxWidth=_max(aResult,aMaxWidth);
			aResult=0;
		}
		if (aChar==mFormatCharacter)
		{
			aCount++;
			unsigned char aFormat=tolower(theText[aCount]);

			if (aFormat=='s')
			{
				int aCPos=aCount+1;
				if (theText[aCPos]=='(')
				{
					int aStart=aCPos+1;
					for (;;)
					{
						aCount++;
						if (theText[aCount]==')' || theText[aCount]==0)
						{
							aScale=theText.GetSegment(aStart,aCount-aStart).ToFloat();
							break;
						}
					}
				}
			}
			if (aFormat=='o')
			{
				int aCPos=aCount+1;
				if (theText[aCPos]=='(')
				{
					int aStart=aCPos+1;
					for (;;)
					{
						aCount++;
						if (theText[aCount]==')' || theText[aCount]==0) break;
					}
				}
			}
			if (aFormat=='c')
			{
				int aCPos=aCount+1;
				if (theText[aCPos]=='[') {aCount+=2;if (theText[aCPos+2]==']') aCount++;}
				else if (theText[aCPos]=='(')
				{
					for (;;)
					{
						aCount++;
						if (theText[aCount]==')' || theText[aCount]==0) break;
					}

				}
			}
			if (aFormat=='o')
			{
				int aCPos=aCount+1;
				if (theText[aCPos]=='(')
				{
					for (;;)
					{
						aCount++;
						if (theText[aCount]==')' || theText[aCount]==0) break;
					}
				}
			}
		}
		else 
		{
			if (aLastChar>0) aResult+=GetCharacterPtr(aLastChar)->mKern[aChar]*aScale;
			//aResult+=mKern[aLastChar][aChar]*aScale;
			aResult+=(GetCharacterPtr(aChar)->mWidth+mExtraSpacing)*aScale;
			aLastChar=aChar;
		}
	}

	aResult=_max(aResult,aMaxWidth);
	return aResult;
}

float Font::GetJustifyWidth(StringUTF8 theText, int &lineFeedPosition, int &lineFeedCharacter)
{
	float aResult=0;
	lineFeedCharacter=0;

	if (mJustifyParagraph)
	{
		lineFeedPosition=-1;
		int aLastChar=0;
		float aMaxWidth=0;
		theText.StartUTF8();
		//for (int aCount=0;aCount<theText.Len();aCount++)
		for (;;)
		{
			int aChar=theText.GetUTF8();if (aChar==0) break;
			if (aChar==10 || aChar==13)
			{
				if (lineFeedPosition==-1) 
				{
					lineFeedPosition=theText.TellUTF8();
					lineFeedCharacter=aChar;
				}
				aResult=0;
			}

			if (aLastChar>0) aResult+=GetCharacterPtr(aLastChar)->mKern[aChar];
			//aResult+=mKern[aLastChar][aChar];
			aResult+=GetCharacterPtr(aChar)->mWidth+mExtraSpacing;
			aLastChar=aChar;

			aMaxWidth=_max(aResult,aMaxWidth);
		}
		aResult=aMaxWidth;
	}
	else
	{
		lineFeedPosition=-1;
		int aLastChar=0;
		//for (int aCount=0;aCount<theText.Len();aCount++)
		theText.StartUTF8();
		//for (int aCount=0;aCount<theText.Len();aCount++)
		for (;;)
		{
			int aTell=theText.TellUTF8();
			int aChar=theText.GetUTF8();if (aChar==0) break;
			if (aChar==10 || aChar==13)
			{
				lineFeedPosition=aTell;
				lineFeedCharacter=aChar;
				break;
			}

			if (aLastChar>0) aResult+=GetCharacterPtr(aLastChar)->mKern[aChar];
			//aResult+=mKern[aLastChar][aChar];
			aResult+=GetCharacterPtr(aChar)->mWidth+mExtraSpacing;
			aLastChar=aChar;
		}
	}
	return aResult;
}

float Font::GetJustifyWidthEX(String theText, int &lineFeedPosition)
{
	float aResult=0;
	lineFeedPosition=-1;
	float aScale=1.0f;

	if (mJustifyParagraph)
	{
		float aMaxWidth=0;
		int aLastChar=0;
		for (int aCount=0;aCount<theText.Len();aCount++)
		{
			unsigned char aChar=theText[aCount];
			if (aChar==10 || aChar==13)
			{
				lineFeedPosition=aCount;
				aResult=0;
			}
			if (aChar==mFormatCharacter)
			{
				aCount++;
				unsigned char aFormat=theText[aCount];

				if (aFormat=='s')
				{
					int aCPos=aCount+1;
					if (theText[aCPos]=='(')
					{
						int aStart=aCPos+1;
						for (;;)
						{
							aCount++;
							if (theText[aCount]==')' || theText[aCount]==0)
							{
								aScale=theText.GetSegment(aStart,aCount-aStart).ToFloat();
								break;
							}
						}
					}
				}

				if (aFormat=='c')
				{
					int aCPos=aCount+1;
					if (theText[aCPos]=='[') {aCount+=2;if (theText[aCPos+2]==']') aCount++;}
					else if (theText[aCPos]=='(')
					{
						for (;;)
						{
							aCount++;
							if (theText[aCount]==')' || theText[aCount]==0) break;
						}

					}
				}
			}
			else
			{
				if (aLastChar>0) aResult+=GetCharacterPtr(aLastChar)->mKern[aChar]*aScale;
				//aResult+=mKern[aLastChar][aChar]*aScale;
				aResult+=(GetCharacterPtr(aChar)->mWidth+mExtraSpacing)*aScale;
				aMaxWidth=_max(aResult,aMaxWidth);

				aLastChar=aChar;
			}
		}
		aResult=aMaxWidth;
	}
	else 
	{
		int aLastChar=0;
		for (int aCount=0;aCount<theText.Len();aCount++)
		{
			unsigned char aChar=theText[aCount];
			if (aChar==10 || aChar==13)
			{
				lineFeedPosition=aCount;
				break;
			}
			if (aChar==mFormatCharacter)
			{
				aCount++;
				unsigned char aFormat=theText[aCount];
				if (aFormat=='s')
				{
					int aCPos=aCount+1;
					if (theText[aCPos]=='(')
					{
						int aStart=aCPos+1;
						for (;;)
						{
							aCount++;
							if (theText[aCount]==')' || theText[aCount]==0)
							{
								aScale=theText.GetSegment(aStart,aCount-aStart).ToFloat();
								break;
							}
						}
					}
				}
				if (aFormat=='o')
				{
					int aCPos=aCount+1;
					if (theText[aCPos]=='(')
					{
						int aStart=aCPos+1;
						for (;;)
						{
							aCount++;
							if (theText[aCount]==')' || theText[aCount]==0) break;
						}
					}
				}


				if (aFormat=='c')
				{
					int aCPos=aCount+1;
					if (theText[aCPos]=='[') {aCount+=2;if (theText[aCPos+2]==']') aCount++;}
					else if (theText[aCPos]=='(')
					{
						for (;;)
						{
							aCount++;
							if (theText[aCount]==')' || theText[aCount]==0) break;
						}

					}
				}
			}
			else
			{
				if (aLastChar>0) aResult+=GetCharacterPtr(aLastChar)->mKern[aChar]*aScale;
				//aResult+=mKern[aLastChar][aChar]*aScale;
				aResult+=(GetCharacterPtr(aChar)->mWidth+mExtraSpacing)*aScale;
				aLastChar=aChar;
			}
		}
	}
	return aResult;
}

String Font::Wrap(StringUTF8 theString, float theWidth)
{
	//
	// There is a glitch ere... if we wrap this:
	//
	//		You are a half-wit!
	//
	//  We get this:
	//
	//		You are a half
	//		wit!
	//
	//  But I expected:
	//
	//		You are a half-
	//		wit!
	//

	//*
	StringUTF8 aNewString=theString.c();

	float aWidth=0;
	int aLastBreak=0;

	int aStartLine=false;

	aNewString.StartUTF8();
	for (;;)
	{
/*
#ifdef _DEBUG
		static int aFuc=0;
		if (++aFuc>1000) exit(0);
#endif
*/

		int aTell=aNewString.TellUTF8();
		int aChar=aNewString.GetUTF8();if (aChar==0) break;

//		gOut.Out("Got %c @ %d",aChar,aTell);

		float aAddWidth=0;
		if (aChar==10 || aChar==13) aWidth=aAddWidth=0;

		if (aChar==' ') 
		{
			if (aStartLine)
			{
				//
				// Since we're starting a line, let's delete all 
				// leading spaces...
				//
				int aTell2;
				for (;;)
				{
					aTell2=aNewString.TellUTF8();
					int aLeadingSpaces=aNewString.GetUTF8();
					if (aLeadingSpaces!=' ') break;
				}
				if (aTell2>aTell) 
				{
					aNewString.Delete(aTell,aTell2-aTell);
				}

				aNewString.StartUTF8(aTell);
				aNewString.RewindUTF8();
			}
			else aAddWidth+=(GetCharacterPtr(32)->mWidth+mExtraSpacing);
		}
		else 
		{
			aStartLine=false;
			aAddWidth+=(float)GetCharacterPtr(aChar)->mWidth+mExtraSpacing;
		}

		if (aWidth+aAddWidth>=theWidth)
		{
			//
			// Go backwards...
			//

			aWidth=0;
			aNewString.RewindUTF8();
			char* aHoldSource=aNewString.HelpUTF8()->mUTF8Cursor;

			for (;;)
			{
				if (aNewString.TellUTF8()==aLastBreak) {aNewString.HelpUTF8()->mUTF8Cursor=aHoldSource;break;}
				int aPeekChar=aNewString.PeekUTF8();
				if (aPeekChar==' ' || aPeekChar=='-') break;
				aNewString.RewindUTF8();
			}
			aLastBreak=aNewString.TellUTF8();
			int aPeekChar=aNewString.PeekUTF8();

			if (aPeekChar==' ' || aPeekChar=='-') 
			{
				*aNewString.HelpUTF8()->mUTF8Cursor=10;
			}
			else 
			{
				if (aNewString.TellUTF8()>1) {aNewString.RewindUTF8();}
				int aTell=aNewString.TellUTF8();
				aNewString.Insert(10,1,aTell);
				aNewString.Insert('-',1,aTell);
				aNewString.StartUTF8(aTell);
			}

			aStartLine=true;
		}
		aWidth+=aAddWidth;
	}

	//
	// A problem... splitting spaces across two lines.
	// 
	return aNewString;
	/**/
/*
	String aNewString=theString.c();
	int aPtr=0;

	float aWidth=0;
	int aLastBreak=0;

	int aStartLine=false;
	
	for (;;)
	{
		float aAddWidth=0;

		if (aNewString[aPtr]==0) break;
		if (aNewString[aPtr]==10 || aNewString[aPtr]==13) aWidth=aAddWidth=0;

		if (aNewString[aPtr]==' ') 
		{
			if (aStartLine)
			{
				//
				// Since we're starting a line, let's delete all 
				// leading spaces...
				//
				int aLeadCount=1;
				for (;;)
				{
					if (aNewString[aPtr+aLeadCount]!=' ') break;
					aLeadCount++;
				}

				aNewString.Delete(aPtr,aLeadCount);
				aPtr--;
			}
			else aAddWidth+=(mCharacter[32]->mWidth+mExtraSpacing);
		}
		else if (mCharacter[(unsigned char)aNewString[aPtr]])
		{
			aStartLine=false;
			aAddWidth+=(float)mCharacter[(unsigned char)aNewString[aPtr]]->mWidth+mExtraSpacing;
		}

		if (aWidth+aAddWidth>theWidth)
		{
			//
			// Go backwards...
			//

			aWidth=0;
			int aHoldSource=aPtr;

			for (;;)
			{
				aPtr--;
				if (aPtr==aLastBreak)
				{
					aPtr=aHoldSource;
					break;
				}

				if (aNewString[aPtr]==' ' || aNewString[aPtr]=='-') break;
			}
			aLastBreak=aPtr;
			if (aNewString[aPtr]==' ' || aNewString[aPtr]=='-') aNewString.SetChar(aPtr,10);
			else 
			{
				if (aPtr>1) aPtr--;
				aNewString.Insert(10,1,aPtr);
				aNewString.Insert('-',1,aPtr);
			}
			
			aStartLine=true;
		}

		aWidth+=aAddWidth;
		aPtr++;
	}

	//
	// A problem... splitting spaces across two lines.
	// 
	return aNewString;
	/**/
}

String Font::WrapEX(String theString, float theWidth)
{
	String aNewString=theString.c();
	int aPtr=0;

	float aWidth=0;
	int aLastBreak=0;

	int aStartLine=false;

	for (;;)
	{
		unsigned char aChar=aNewString[aPtr];
		if (aChar==mFormatCharacter)
		{
			aPtr++;
			unsigned char aFormat=aNewString[aPtr];
			if (aFormat=='c')
			{
				int aCPos=aPtr+1;
				if (aNewString[aCPos]=='[') {aPtr+=2;if (aNewString[aCPos+2]==']') aPtr++;}
				else if (aNewString[aCPos]=='(')
				{
					for (;;)
					{
						aPtr++;
						if (aNewString[aPtr]==')' || aNewString[aPtr]==0) break;
					}
				}
			}
			aPtr++;

		}
		else
		{
			float aAddWidth=0;

			if (aNewString[aPtr]==0) break;
			if (aNewString[aPtr]==10 || aNewString[aPtr]==13) aWidth=aAddWidth=0;

			if (aNewString[aPtr]==' ') 
			{
				if (aStartLine)
				{
					//
					// Since we're starting a line, let's delete all 
					// leading spaces...
					//
					int aLeadCount=1;
					for (;;)
					{
						if (aNewString[aPtr+aLeadCount]!=' ') break;
						aLeadCount++;
					}

					aNewString.Delete(aPtr,aLeadCount);
					aPtr--;
				}
				else aAddWidth+=(GetCharacterPtr(32)->mWidth+mExtraSpacing);
			}
			else
			{
				aStartLine=false;
				aAddWidth+=(float)GetCharacter((unsigned char)aNewString[aPtr])->mWidth+mExtraSpacing;
			}

			if (aWidth+aAddWidth>theWidth)
			{
				//
				// Go backwards...
				//

				aWidth=0;
				int aHoldSource=aPtr;

				for (;;)
				{
					aPtr--;
					if (aPtr==aLastBreak)
					{
						aPtr=aHoldSource;
						break;
					}

					if (aNewString[aPtr]==' ' || aNewString[aPtr]=='-') break;
				}
				aLastBreak=aPtr;
				//aNewString.SetChar(aPtr,10);
				if (aNewString[aPtr]==' ' || aNewString[aPtr]=='-') aNewString.SetChar(aPtr,10);
				else 
				{
					if (aPtr>1) aPtr--;
					aNewString.Insert(10,1,aPtr);
					aNewString.Insert('-',1,aPtr);
				}


				aStartLine=true;
			}

			aWidth+=aAddWidth;
			aPtr++;
		}
	}

	//
	// A problem... splitting spaces across two lines.
	// 
	return aNewString;
}

void Font::PointLowercaseAtUppercase()
{
	for (int aCount='a';aCount<='z';aCount++)
	{
		int aUpperRef=(aCount-'a')+'A';
		GetCharacterRef(aCount)=GetCharacterRef(aUpperRef);

		/*
		for (int aNextCount='a';aNextCount<='z';aNextCount++)
		{
			int aNextUpperRef=(aNextCount-'a')+'A';
			mKern[aCount][aNextCount]=mKern[aUpperRef][aNextUpperRef];
		}
		*/

	}
}

String Font::Truncate(StringUTF8 theString, float theWidth, String theTruncateString)
{
	String aResult=theString;

	float aWidth=0;
	float aMaxWidth=theWidth-Width(theTruncateString);
	int aPtr=0;

	for (;;)
	{
		unsigned char aChar=theString[aPtr];
		if (aChar==13 || aChar==10 || aChar==0) break;
		
		float aAddWidth=0;
		aAddWidth=(float)GetCharacterPtr(aChar)->mWidth+mExtraSpacing;
		if (aWidth+aAddWidth>aMaxWidth) 
		{
			aResult=theString.GetSegment(0,aPtr-1);
			aResult+=theTruncateString;
			break;
		}
		aWidth+=aAddWidth;
		aPtr++;
	}
	return aResult;
}

Rect Font::GetBoundingRect(StringUTF8 theText, float theX, float theY, int theJustify)
{
	Rect aResult;
	aResult.mX=theX;
	aResult.mY=theY-mAscent;
	aResult.mWidth=1;
	aResult.mHeight=1;

	int aLastChar=0;
	if (theJustify!=1)
	{
		int aLineFeedPosition;
		int aLineFeedChar;
		float aWidth=GetJustifyWidth(theText,aLineFeedPosition,aLineFeedChar);
		if (theJustify==0) aWidth/=2;
		if (aLineFeedPosition!=-1 && !mJustifyParagraph)
		{
			aResult=aResult.Union(GetBoundingRect(theText.GetSegment(0,aLineFeedPosition),theX,theY,theJustify));
			theY+=mPointSize+1;
			if (aLineFeedChar==13) 
			{
				theY+=mPointSize+1;
				theY+=mCRSpacing;
			}
			aResult=aResult.Union(GetBoundingRect(theText.GetSegment(aLineFeedPosition+1,9999),theX,theY,theJustify));
			return aResult;
		}
		theX-=aWidth;
	}

	//gG.Translate(0.0f,-mAscent);
	Point aStartPos=Point(theX,theY);

	Point aPos=aStartPos;
	theText.StartUTF8();
	//for (int aCount=0;aCount<theText.Len();aCount++)
	for (;;)
	{
		int aTell=theText.TellUTF8();
		int aChar=theText.GetUTF8();if (aChar==0) break;
		if (aChar==10)
		{
			aPos.mX=aStartPos.mX;
			aPos.mY+=mPointSize+1;
		}
		if (aChar==13)
		{
			aPos.mX=aStartPos.mX;
			aPos.mY+=mPointSize+1;
			aPos.mY+=mPointSize+1;
			aPos.mY+=mCRSpacing;
		}
		{
			if (aLastChar>0) aPos.mX+=GetCharacterPtr(aLastChar)->mKern[aChar];
			//aPos.mX+=mKern[aLastChar][aChar];
			//mCharacter[aChar].mSprite.Center(aPos+mCharacter[aChar].mOffset);
			aPos.mX+=GetCharacterPtr(aChar)->mWidth+mExtraSpacing;
		}
		aLastChar=aChar;
		aResult.mWidth=_max(aResult.mWidth,aPos.mX-aStartPos.mX);
	}
	//gG.Translate(0.0f,mAscent);

	aResult.mHeight=aPos.mY-aResult.mY;
	aResult.mX=theX;

	return aResult;
}

Rect Font::GetBoundingRectEX(StringUTF8 theText, float theX, float theY, int theJustify)
{
	Rect aResult;
	aResult.mX=theX;
	aResult.mY=theY-mAscent;
	aResult.mWidth=1;
	aResult.mHeight=1;

	float aScale=1.0f;

	int aLastChar=0;
	if (theJustify!=1)
	{
		int aLineFeedPosition;
		float aWidth=GetJustifyWidthEX(theText,aLineFeedPosition);
		if (theJustify==0) aWidth/=2;
		if (aLineFeedPosition!=-1 && !mJustifyParagraph)
		{
			aResult=aResult.Union(GetBoundingRectEX(theText.GetSegment(0,aLineFeedPosition),theX,theY,theJustify));
			theY+=mPointSize+1;
			aResult=aResult.Union(GetBoundingRectEX(theText.GetSegment(aLineFeedPosition+1,9999),theX,theY,theJustify));
			return aResult;
		}
		theX-=aWidth;
	}

	//gG.Translate(0.0f,-mAscent);
	Point aStartPos=Point(theX,theY);

	Point aPos=aStartPos;
	for (int aCount=0;aCount<theText.Len();aCount++)
	{
		unsigned char aChar=theText[aCount];
		if (aChar==10)
		{
			aPos.mX=aStartPos.mX;
			aPos.mY+=mPointSize+1;
		}
		else if (aChar==13)
		{
			aPos.mX=aStartPos.mX;
			aPos.mY+=mPointSize+1;
			aPos.mY+=mPointSize+1;
			aPos.mY+=mCRSpacing;
		}
		else if (aChar==mFormatCharacter)
		{
			aCount++;
			unsigned char aFormat=tolower(theText[aCount]);

			if (aFormat=='s')
			{
				int aCPos=aCount+1;
				if (theText[aCPos]=='(')
				{
					int aStart=aCPos+1;
					for (;;)
					{
						aCount++;
						if (theText[aCount]==')' || theText[aCount]==0)
						{
							aScale=theText.GetSegment(aStart,aCount-aStart).ToFloat();
							break;
						}
					}
				}
			}
			if (aFormat=='o')
			{
				int aCPos=aCount+1;
				if (theText[aCPos]=='(')
				{
					int aStart=aCPos+1;
					for (;;)
					{
						aCount++;
						if (theText[aCount]==')' || theText[aCount]==0) break;
					}
				}
			}
			if (aFormat=='c')
			{
				int aCPos=aCount+1;
				if (theText[aCPos]=='[') {aCount+=2;if (theText[aCPos+2]==']') aCount++;}
				else if (theText[aCPos]=='(')
				{
					for (;;)
					{
						aCount++;
						if (theText[aCount]==')' || theText[aCount]==0) break;
					}

				}
			}
			if (aFormat=='o')
			{
				int aCPos=aCount+1;
				if (theText[aCPos]=='(')
				{
					for (;;)
					{
						aCount++;
						if (theText[aCount]==')' || theText[aCount]==0) break;
					}
				}
			}
		}
		else
		{
			if (aLastChar>0) aPos.mX+=GetCharacterPtr(aLastChar)->mKern[aChar];
			//aPos.mX+=mKern[aLastChar][aChar];
			aPos.mX+=(GetCharacterPtr(aChar)->mWidth+mExtraSpacing)*aScale;
			aLastChar=aChar;
			aResult.mWidth=_max(aResult.mWidth,aPos.mX-aStartPos.mX);
		}
	}
	//gG.Translate(0.0f,mAscent);

	aResult.mHeight=aPos.mY-aResult.mY;
	aResult.mX=theX;


	return aResult;
}

String Font::Unformat(StringUTF8 theText)
{
	String aResult;
	int aLastChar=0;
	float aMaxWidth=0;
	float aScale=1.0f;

	for (int aCount=0;aCount<theText.Len();aCount++)
	{
		unsigned char aChar=theText[aCount];
		if (aChar==10 || aChar==13)
		{
//			aResult+=aChar;
		}
		if (aChar==mFormatCharacter)
		{
			aCount++;
			unsigned char aFormat=tolower(theText[aCount]);

			if (aFormat=='s')
			{
				int aCPos=aCount+1;
				if (theText[aCPos]=='(')
				{
					int aStart=aCPos+1;
					for (;;)
					{
						aCount++;
						if (theText[aCount]==')' || theText[aCount]==0)
						{
							aScale=theText.GetSegment(aStart,aCount-aStart).ToFloat();
							break;
						}
					}
				}
			}
			if (aFormat=='o')
			{
				int aCPos=aCount+1;
				if (theText[aCPos]=='(')
				{
					int aStart=aCPos+1;
					for (;;)
					{
						aCount++;
						if (theText[aCount]==')' || theText[aCount]==0) break;
					}
				}
			}
			if (aFormat=='c')
			{
				int aCPos=aCount+1;
				if (theText[aCPos]=='[') {aCount+=2;if (theText[aCPos+2]==']') aCount++;}
				else if (theText[aCPos]=='(')
				{
					for (;;)
					{
						aCount++;
						if (theText[aCount]==')' || theText[aCount]==0) break;
					}

				}
			}
			if (aFormat=='o')
			{
				int aCPos=aCount+1;
				if (theText[aCPos]=='(')
				{
					for (;;)
					{
						aCount++;
						if (theText[aCount]==')' || theText[aCount]==0) break;
					}
				}
			}
		}
		else
		{
			aResult.strcat(aChar);
			aLastChar=aChar;
		}
	}

	return aResult;
}

void Font::GetHotAreaEX(String theText, RectComplex& theRect, int theStart, int theEnd, Point thePos)
{
	if (mJustify!=1)
	{
		int aLineFeedPosition;
		float aWidth=GetJustifyWidthEX(theText,aLineFeedPosition);
		if (mJustify==0) aWidth/=2;
		if (aLineFeedPosition!=-1 && !mJustifyParagraph)
		{
			GetHotAreaEX(theText.GetSegment(0,aLineFeedPosition),theRect,theStart,theEnd,thePos);
			thePos.mY+=mPointSize+1;
			GetHotAreaEX(theText.GetSegment(aLineFeedPosition+1,9999),theRect,theStart,theEnd,thePos);
			return;
		}
		thePos.mX-=aWidth;
	}

	int aLastChar=0;

	int aEffect=0;
	bool aBold=false;
	bool aItalics=false;
	bool aUnderline=false;
	float aScale=1.0f;

	//gG.Translate(0.0f,-mAscent);
	Point aStartPos=thePos;
	Point aPos=aStartPos;

	Point aOffset=Point(0,0);

	theStart=_max(0,theStart);
	theEnd=_min(theEnd,theText.Len()-1);

	float aMin=-mAscent;
	float aMax=-9999;
	for (int aCount=theStart;aCount<=theEnd;aCount++)
	{
		unsigned char aChar=theText[aCount];
		Quad aDrawQuad=GetCharacterPtr(aChar)->mSprite.GetDrawQuad();
		aMin=_min(aMin,aDrawQuad.GetBounds().mY);
		aMax=_max(aMax,aDrawQuad.GetBounds().LowerLeft().mY);
	}

	float aHeight=aMax-aMin;


	for (int aCount=0;aCount<theText.Len();aCount++)
	{
		unsigned char aChar=theText[aCount];
		if (aChar==10)
		{
			aPos.mX=aStartPos.mX;
			aPos.mY+=mPointSize+1;
		}
		else if (aChar==13)
		{
			aPos.mX=aStartPos.mX;
			aPos.mY+=mPointSize+1;
			aPos.mY+=mPointSize+1;
			aPos.mY+=mCRSpacing;
		}
		else if (aChar==mFormatCharacter)
		{
			aCount++;
			unsigned char aFormat=tolower(theText[aCount]);

			if (aFormat=='i') aItalics=!aItalics;
			if (aFormat=='b') aBold=!aBold;
			if (aFormat=='u') aUnderline=!aUnderline;
			if (aFormat=='s')
			{
				int aCPos=aCount+1;
				if (theText[aCPos]=='(')
				{
					int aStart=aCPos+1;
					for (;;)
					{
						aCount++;
						if (theText[aCount]==')' || theText[aCount]==0)
						{
							aScale=theText.GetSegment(aStart,aCount-aStart).ToFloat();
							break;
						}
					}
				}
			}
			if (aFormat=='o')
			{
				int aCPos=aCount+1;
				if (theText[aCPos]=='(')
				{
					int aStart=aCPos+1;
					for (;;)
					{
						aCount++;
						if (theText[aCount]==')' || theText[aCount]==0)
						{
							String aSeg=theText.GetSegment(aStart,aCount-aStart);

							if (aSeg.Len()<1) aOffset=Point(0,0);
							else
							{
								float aX=aSeg.GetToken(',').ToFloat();
								float aY=aSeg.GetNextToken(',').ToFloat();
								aOffset=Point(aX,aY);
							}
							break;
						}

					}
				}
			}
			if (aFormat=='c')
			{
				int aCPos=aCount+1;
				if (theText[aCPos]=='[')
				{
					int aValue=theText[aCPos+1]-48;
					aCount+=2;
					if (theText[aCPos+2]==']') aCount++;
				}
				else if (theText[aCPos]=='(')
				{
					int aStart=aCPos+1;
					for (;;)
					{
						aCount++;
						if (theText[aCount]==')' || theText[aCount]==0)
						{
							float aAlpha=1.0f;
							String aSeg=theText.GetSegment(aStart,aCount-aStart);
							float aR=aSeg.GetToken(',').ToFloat();
							float aG=aSeg.GetNextToken(',').ToFloat();
							float aB=aSeg.GetNextToken(',').ToFloat();
							String aAA=aSeg.GetNextToken(',');if (aAA.Len()>0) aAlpha=aAA.ToFloat();
							break;
						}

					}

				}
				//
				// Change text color...
				//
			}
			aEffect=aItalics|aBold|aUnderline;
		}
		else
		{
			Character* aCPtr=GetCharacterPtr(aChar);
			if (aLastChar>0) aPos.mX+=GetCharacterPtr(aLastChar)->mKern[aChar]*aScale;
			//aPos.mX+=(float)(mKern[aLastChar][aChar])*aScale;

			if (aCount>=theStart && aCount<=theEnd)
			{
				Quad aDrawQuad=aCPtr->mSprite.GetDrawQuad();
				if (aScale!=.0f) aDrawQuad.Scale(aScale);
				aDrawQuad.Translate((aPos+aOffset+aCPtr->mOffset));
				Rect aRect=Rect(aPos.mX,aPos.mY-mAscent,aCPtr->mWidth,aHeight);
				theRect.Add(aRect);
			}
			aPos.mX+=(aCPtr->mWidth+mExtraSpacing)*aScale;

			aLastChar=aChar;
		}
	}
}

void Font::GetHotArea(StringUTF8 theText, RectComplex& theRect, int theStart, int theEnd, Point thePos)
{
	if (theStart>theText.Len() || theEnd<0) return;

	if (mJustify!=1)
	{
		int aLineFeedPosition;
		int aLineFeedChar;
		float aWidth=GetJustifyWidth(theText,aLineFeedPosition,aLineFeedChar);
		if (mJustify==0) aWidth/=2;
		if (aLineFeedPosition!=-1 && !mJustifyParagraph)
		{
			GetHotArea(theText.GetSegment(0,aLineFeedPosition),theRect,theStart,theEnd,thePos);
			thePos.mY+=mPointSize+1;
			if (aLineFeedChar==13) 
			{
				thePos.mY+=mPointSize+1;
				thePos.mY+=mCRSpacing;
			}
			GetHotArea(theText.GetSegment(aLineFeedPosition+1,9999),theRect,theStart-(aLineFeedPosition+1),theEnd-(aLineFeedPosition+1),thePos);
			return;
		}
		thePos.mX-=aWidth;
	}

	int aLastChar=0;

	int aEffect=0;
	bool aBold=false;
	bool aItalics=false;
	bool aUnderline=false;
	float aScale=1.0f;

	//gG.Translate(0.0f,-mAscent);
	Point aStartPos=thePos;
	Point aPos=aStartPos;

	Point aOffset=Point(0,0);

	theStart=_max(0,theStart);
	theEnd=_min(theEnd,theText.Len()-1);

	float aMin=-mAscent;
	float aMax=-9999;

	int aRealStart=theStart;//theText.PosUTF8(theStart);
	int aRealEnd=theEnd;//theText.PosUTF8(theEnd);

	theText.StartUTF8(aRealStart);

	//for (int aCount=theStart;aCount<=theEnd;aCount++)
	for (;;)
	{
		if (theText.TellUTF8()>aRealEnd) break;
		int aChar=theText.GetUTF8();if (aChar==0) break;

		Quad aDrawQuad=GetCharacterPtr(aChar)->mSprite.GetDrawQuad();
		aMin=_min(aMin,aDrawQuad.GetBounds().mY);
		aMax=_max(aMax,aDrawQuad.GetBounds().LowerLeft().mY);
	}

	float aHeight=aMax-aMin;

	theText.StartUTF8();
	//for (int aCount=0;aCount<theText.Len();aCount++)
	for (;;)
	{
		int aTell=theText.TellUTF8();
		int aChar=theText.GetUTF8();if (aChar==0) break;
		if (aChar==10)
		{
			aPos.mX=aStartPos.mX;
			aPos.mY+=mPointSize+1;
		}
		else if (aChar==13)
		{
			aPos.mX=aStartPos.mX;
			aPos.mY+=mPointSize+1;
			aPos.mY+=mPointSize+1;
			aPos.mY+=mCRSpacing;
		}
		else
		{
			Character* aCPtr=GetCharacterPtr(aChar);
			if (aLastChar>0) aPos.mX+=GetCharacterPtr(aLastChar)->mKern[aChar]*aScale;
			//aPos.mX+=(float)(mKern[aLastChar][aChar])*aScale;


			if (aTell>=aRealStart && aTell<=aRealEnd)
			{
				Quad aDrawQuad=aCPtr->mSprite.GetDrawQuad();
				if (aScale!=.0f) aDrawQuad.Scale(aScale);
				aDrawQuad.Translate((aPos+aOffset+aCPtr->mOffset));
				Rect aRect=Rect(aPos.mX,aPos.mY-mAscent,aCPtr->mWidth,aHeight);
				theRect.Add(aRect);
			}
			aPos.mX+=(aCPtr->mWidth+mExtraSpacing)*aScale;
			aLastChar=aChar;
		}
	}
}

Point Font::CharacterPos(StringUTF8 theText, int breakAt)
{
	int aLastChar=0;
	Point aResult;
	float aMaxWidth=0;

	int aCount=0;
	theText.StartUTF8();
	//for (int aCount=0;aCount<theText.Len();aCount++)
	for (;;)
	{
		if (aCount>=breakAt) break;
		aCount++;

		int aChar=theText.GetUTF8();if (aChar==0) break;

		if (aChar==10 || aChar==13)
		{
			aResult.mX=0;
			aResult.mY+=mPointSize+1;
			if (aChar==13) 
			{
				aResult.mY+=mPointSize+1;
				aResult.mY+=mCRSpacing;
			}
		}
		else 
		{
			if (aLastChar>0) aResult.mX+=GetCharacterPtr(aLastChar)->mKern[aChar];
			//aResult.mX+=mKern[aLastChar][aChar];
			aResult.mX+=GetCharacterPtr(aChar)->mWidth+mExtraSpacing;
		}
		aLastChar=aChar;
	}

	return aResult;
}

void Font::CopyCharacter(int theSource, int theDest)
{
	if (IsChar(theSource))
	{
		Smart(Character) aChar=CreateCharacter(theDest);
		Character* aCopyFrom=GetCharacterPtr(theSource);
		aChar->mOffset=aCopyFrom->mOffset;
		aChar->mWidth=aCopyFrom->mWidth;
		aChar->mSprite=aCopyFrom->mSprite;
	}
}
#else
Font::Font()
{
	memset(mKern,0,sizeof(mKern));
	SetFormatCharacter('_');
	SetItalicSkewFactor(.125f);
	SetBoldPixels(2);
	for (int aCount=0;aCount<10;aCount++) SetColorSlot(aCount,Color().Spectrum(aCount));
	SetJustify(1);
	SetJustifyParagraph(false);
	mFormatCharacter='_';
	mExtraSpacing=0;
	mCRSpacing=0;
}

Font::~Font()
{
}


void Font::ManualLoad(int theTexture, IOBuffer& theBuffer)
{
	if (theBuffer.IsEnd()) return;	// kittyengine asset existence guard
	float aSize=theBuffer.ReadFloat();
	float aSpaceWidth=theBuffer.ReadFloat();
	float aAscent=theBuffer.ReadFloat();

	aAscent=aSize;	// For Raptisoft stuff, Ascent==Size.

	SetSpaceWidth(aSpaceWidth);
	SetAscent(aAscent);
	SetPointSize(aSize);

	//
	// Load Kerns...
	//
	for (;;)
	{
		short aC1=theBuffer.ReadShort();
		short aC2=theBuffer.ReadShort();
		if (aC1==0 && aC2==0) break;
		mKern[aC1][aC2]=theBuffer.ReadFloat();
	}

	for (;;)
	{
		short aChar=theBuffer.ReadShort();
		if (aChar==0) break;

		mCharacter[aChar].mIsInitialized=true;
		mCharacter[aChar].mWidth=theBuffer.ReadFloat();
		mCharacter[aChar].mOffset=theBuffer.ReadPoint();
		mCharacter[aChar].mSprite.ManualLoad(theTexture,theBuffer);
	}
}

void Font::ManualLoad(int theCharacter, Sprite& theSprite, Point theOffset)
{
	mCharacter[theCharacter].mSprite.ManualLoad(theSprite);
	mCharacter[theCharacter].mWidth=(float)theSprite.mWidth;
	mCharacter[theCharacter].mOffset=theOffset;
	//mCharacter[theCharacter].mOffset+=Point(theSprite.HalfWidthF(),theSprite.HalfHeightF());
	mCharacter[theCharacter].mIsInitialized=true;
}

void Font::ManualLoad(int theCharacter, Sprite& theSprite, Point theOffset, float theWidth)
{
	mCharacter[theCharacter].mSprite.ManualLoad(theSprite);
	mCharacter[theCharacter].mWidth=(float)theSprite.mWidth;
	mCharacter[theCharacter].mOffset=theOffset;
	//mCharacter[theCharacter].mOffset+=Point(theSprite.HalfWidthF(),theSprite.HalfHeightF());
	mCharacter[theCharacter].mWidth=theWidth;
	mCharacter[theCharacter].mIsInitialized=true;
}


void Font::SetupCharacter(int theCharacter, int theWidth, Point theOffset, int theTexture, Rect theTextureRect, int theSpriteWidth, int theSpriteHeight, float theDrawWidth, float theDrawHeight, float theSpriteMoveX, float theSpriteMoveY)
{
	mCharacter[theCharacter].mSprite.ManualLoad(theTexture,theTextureRect,theSpriteWidth,theSpriteHeight,theDrawWidth,theDrawHeight,theSpriteMoveX,theSpriteMoveY);
	mCharacter[theCharacter].mWidth=(float)theWidth;
	mCharacter[theCharacter].mOffset=theOffset;
	mCharacter[theCharacter].mIsInitialized=true;
	//mCharacter[theCharacter].mSprite.GetTextureQuad().LowerLeft().mY+=.5f;
	//mCharacter[theCharacter].mSprite.GetTextureQuad().LowerRight().mY+=.5f;
}

void Font::AddKerningPair(String thePair, float theKern)
{
	int aChar1=thePair[0];
	int aChar2=thePair[1];

	if (aChar1>0 && aChar1<256 && aChar2>0 && aChar2<256) mKern[thePair[0]][thePair[1]]=theKern;
}

void Font::Draw(String theText, float theX, float theY)
{
	int aLastChar=0;
	if (mJustify!=1)
	{
		int aLineFeedPosition;
		char aLineFeedChar;
		float aWidth=GetJustifyWidth(theText,aLineFeedPosition,aLineFeedChar);
		if (mJustify==0) aWidth/=2;
		if (aLineFeedPosition!=-1 && !mJustifyParagraph)
		{
			Draw(theText.GetSegment(0,aLineFeedPosition),theX,theY);
			theY+=mPointSize+1;
			if (aLineFeedChar==13) 
			{
				theY+=mPointSize+1;
				theY+=mCRSpacing;
			}
			Draw(theText.GetSegment(aLineFeedPosition+1,9999),theX,theY);
			return;
		}
		theX-=aWidth;
	}

	gG.PushTranslate();
	if (gSharpenFonts) gG.TranslateToIntegers();

	//gG.Translate(0.0f,-mAscent);
	Point aStartPos=Point((int)theX,(int)theY);
	Point aPos=aStartPos;
	for (int aCount=0;aCount<theText.Len();aCount++)
	{
		unsigned char aChar=theText[aCount];
		if (aChar==10)
		{
			aPos.mX=aStartPos.mX;
			aPos.mY+=mPointSize+1;
		}
		else if (aChar==13)
		{
			aPos.mX=aStartPos.mX;
			aPos.mY+=mPointSize+1;
			aPos.mY+=mPointSize+1;
			aPos.mY+=mCRSpacing;
		}
		else if (mCharacter[aChar].mIsInitialized) 
		{
			aPos.mX+=mKern[aLastChar][aChar];
			mCharacter[aChar].mSprite.Center(aPos+mCharacter[aChar].mOffset);
		}
		aPos.mX+=mCharacter[aChar].mWidth+mExtraSpacing;
		aLastChar=aChar;
	}
	//gG.Translate(0.0f,mAscent);
	gFontDrawPos=aPos;

	gG.PopTranslate();

}

float Font::Width(String theText)
{
	int aLastChar=0;
	float aResult=0;
	float aMaxWidth=0;
	for (int aCount=0;aCount<theText.Len();aCount++)
	{
		unsigned char aChar=theText[aCount];
		if (aChar==10 || aChar==13)
		{
			aMaxWidth=_max(aMaxWidth,aResult);
			aResult=0;
		}
		else
		{
			aResult+=mKern[aLastChar][aChar];
			aResult+=mCharacter[aChar].mWidth+mExtraSpacing;
		}
		aLastChar=aChar;
	}

	aResult=_max(aResult,aMaxWidth);
	return aResult;
}

float Font::Height(String theText)
{
	float aResult=mPointSize;
	bool aNextChar=false;
	bool aDouble=false;

	unsigned char aPrevChar=-1;
	for (int aCount=0;aCount<theText.Len();aCount++)
	{
		unsigned char aChar=theText[aCount];

		if (aChar==10) aNextChar=true;
		else if (aChar==13) {aNextChar=true;aDouble=true;}
		else if (aNextChar)
		{
			aNextChar=false;
			aResult+=mPointSize+1;
			if (aDouble) 
			{
				aResult+=mPointSize+1;
				aResult+=mCRSpacing;
			}
			aDouble=false;
		}

		if (aNextChar && aPrevChar==10) {aResult+=mPointSize+1;}
		if (aNextChar && aPrevChar==13) {aResult+=mPointSize+1;aResult+=mPointSize+1;aResult+=mCRSpacing;}
		aPrevChar=aChar;
	}
	return aResult;
}

Point Font::Size(String theText)
{
	int aLastChar=0;
	Point aResult;
	aResult.mY=mPointSize;

	bool aNextChar=false;
	bool aDouble=false;

	float aMaxWidth=0;
	for (int aCount=0;aCount<theText.Len();aCount++)
	{
		unsigned char aChar=theText[aCount];
		if (aChar==10 || aChar==13)
		{
			aNextChar=true;
			if (aChar==13) aDouble=true;

			aMaxWidth=_max(aMaxWidth,aResult.mX);
			aResult.mX=0;
		}
		else
		{
			if (aNextChar)
			{
				aNextChar=false;
				aResult.mY+=mPointSize+1;
				if (aDouble) 
				{
					aResult.mY+=mPointSize+1;
					aResult.mY+=mCRSpacing;
				}
				aDouble=false;
			}
			aResult.mX+=mKern[aLastChar][aChar];
			aResult.mX+=mCharacter[aChar].mWidth+mExtraSpacing;
		}

		if (aNextChar && aLastChar==10) {aResult.mY+=mPointSize+1;}
		if (aNextChar && aLastChar==13) {aResult.mY+=mPointSize+1;aResult.mY+=mPointSize+1;aResult+=mCRSpacing;}
		aLastChar=aChar;
	}

	aResult.mX=_max(aResult.mX,aMaxWidth);
	return aResult;
}


float Font::Width(String theText, int breakAt)
{
	int aLastChar=0;
	float aResult=0;
	float aMaxWidth=0;
	for (int aCount=0;aCount<theText.Len();aCount++)
	{
		if (aCount==breakAt) break;

		unsigned char aChar=theText[aCount];

		if (aChar==10 || aChar==13)
		{
			aMaxWidth=_max(aMaxWidth,aResult);
			aResult=0;
		}
		else
		{
			aResult+=mKern[aLastChar][aChar];
			aResult+=mCharacter[aChar].mWidth+mExtraSpacing;
		}
		aLastChar=aChar;
	}
	aResult=_max(aResult,aMaxWidth);
	return aResult;
}

float Font::Height(String theText, int breakAt)
{
	float aResult=mPointSize;
	bool aNextChar=false;
	unsigned char aPrevChar=-1;
	bool aDouble=false;

	for (int aCount=0;aCount<theText.Len();aCount++)
	{

		if (aCount==breakAt) break;

		unsigned char aChar=theText[aCount];


		if (aChar==10) aNextChar=true;
		else if (aChar==13) {aNextChar=true;aDouble=true;}
		else if (aNextChar)
		{
			aNextChar=false;
			aResult+=mPointSize+1;
			if (aDouble) aResult+=mPointSize+1;
			aDouble=false;
		}
		if (aNextChar && aPrevChar==10) {aResult+=mPointSize+1;}
		if (aNextChar && aPrevChar==13) {aResult+=mPointSize+1;aResult+=mPointSize+1;}
		aPrevChar=aChar;
	}
	return aResult;
}


void Font::DrawEX(String theText, float theX, float theY)
{
	if (mJustify!=1)
	{
		int aLineFeedPosition;
		float aWidth=GetJustifyWidthEX(theText,aLineFeedPosition);
		if (mJustify==0) aWidth/=2;
		if (aLineFeedPosition!=-1 && !mJustifyParagraph)
		{
			DrawEX(theText.GetSegment(0,aLineFeedPosition),theX,theY);
			theY+=mPointSize+1;
			DrawEX(theText.GetSegment(aLineFeedPosition+1,9999),theX,theY);
			return;
		}
		theX-=aWidth;
	}

	gG.PushTranslate();
	if (gSharpenFonts) gG.TranslateToIntegers();

	Color aNormalColor=gG.GetColor();
	int aLastChar=0;
	float aISkew=mPointSize*mItalicSkewFactor;

	int aEffect=0;
	bool aBold=false;
	bool aItalics=false;
	float aScale=1.0f;

	//gG.Translate(0.0f,-mAscent);
	Point aStartPos=Point((int)theX,(int)theY);
	Point aPos=aStartPos;

	bool aHoldFilter=gG.mFilter;
	if (!aHoldFilter)
	{
		//
		// If we're filtering, let's go ahead and put
		// our text on int boundaries.
		//

		aStartPos=IPoint(aStartPos.mX,aStartPos.mY);
		aPos=IPoint(aPos.mX,aPos.mY);
	}

	Point aOffset=Point(0,0);
	for (int aCount=0;aCount<theText.Len();aCount++)
	{
		unsigned char aChar=theText[aCount];
		if (aChar==10)
		{
			aPos.mX=aStartPos.mX;
			aPos.mY+=mPointSize+1;
		}
		else if (aChar==13)
		{
			aPos.mX=aStartPos.mX;
			aPos.mY+=mPointSize+1;
			aPos.mY+=mPointSize+1;
			aPos.mY+=mCRSpacing;
		}
		else if (aChar==mFormatCharacter)
		{
			aCount++;
			unsigned char aFormat=tolower(theText[aCount]);

			if (aFormat=='i') aItalics=!aItalics;
			if (aFormat=='b') aBold=!aBold;
			if (aFormat=='s')
			{
				int aCPos=aCount+1;
				if (theText[aCPos]=='(')
				{
					int aStart=aCPos+1;
					for (;;)
					{
						aCount++;
						if (theText[aCount]==')' || theText[aCount]==0)
						{
							aScale=theText.GetSegment(aStart,aCount-aStart).ToFloat();
							break;
						}
					}
				}

				if (aScale!=1.0f) gG.Sharpen(false);
				else gG.Sharpen(!aHoldFilter);
			}
			if (aFormat=='o')
			{
				int aCPos=aCount+1;
				if (theText[aCPos]=='(')
				{
					int aStart=aCPos+1;
					for (;;)
					{
						aCount++;
						if (theText[aCount]==')' || theText[aCount]==0)
						{
							String aSeg=theText.GetSegment(aStart,aCount-aStart);

							if (aSeg.Len()<1) aOffset=Point(0,0);
							else
							{
								float aX=aSeg.GetToken(',').ToFloat();
								float aY=aSeg.GetNextToken(',').ToFloat();
								aOffset=Point(aX,aY);
							}
							break;
						}

					}
				}
			}
			if (aFormat=='c')
			{
				int aCPos=aCount+1;
				if (theText[aCPos]=='[')
				{
					int aValue=theText[aCPos+1]-48;
					gG.SetColor(mColorSlot[aValue]);
					aCount+=2;
					if (theText[aCPos+2]==']') aCount++;
				}
				else if (theText[aCPos]=='(')
				{
					int aStart=aCPos+1;
					for (;;)
					{
						aCount++;
						if (theText[aCount]==')' || theText[aCount]==0)
						{
							float aAlpha=1.0f;
							String aSeg=theText.GetSegment(aStart,aCount-aStart);
							float aR=aSeg.GetToken(',').ToFloat();
							float aG=aSeg.GetNextToken(',').ToFloat();
							float aB=aSeg.GetNextToken(',').ToFloat();
							String aAA=aSeg.GetNextToken(',');if (aAA.Len()>0) aAlpha=aAA.ToFloat();
							gG.SetColor(aR,aG,aB,aAlpha);
							break;
						}

					}

				}
				else
				{
					gG.SetColor(aNormalColor);
				}
				//
				// Change text color...
				//
			}
			aEffect=aItalics|aBold;
		}
		else
		{
			if (mCharacter[aChar].mIsInitialized) 
			{
				aPos.mX+=(float)(mKern[aLastChar][aChar])*aScale;
				//				if (!aEffect) mCharacter[aChar].mSprite.DrawScaled(aPos+aOffset+(mCharacter[aChar].mOffset*aScale)+mCharacter[aChar].mSprite.GetHalfSize(),aScale);
				if (!aEffect) mCharacter[aChar].mSprite.DrawScaled(aPos+aOffset+(mCharacter[aChar].mOffset*aScale),aScale);
				//				if (!aEffect) mCharacter[aChar].mSprite.Draw(aPos+mCharacter[aChar].mOffset);
				else
				{
					Quad aDrawQuad=mCharacter[aChar].mSprite.GetDrawQuad();
					if (aItalics) 
					{
						aDrawQuad.UpperLeft().mX+=aISkew;
						aDrawQuad.UpperRight().mX+=aISkew;
						aDrawQuad.LowerLeft().mX-=aISkew;
						aDrawQuad.LowerRight().mX-=aISkew;
					}
					if (aScale!=.0f) aDrawQuad.Scale(aScale);

					gG.PushTranslate();
					gG.Translate((aPos+aOffset+mCharacter[aChar].mOffset));//+mCharacter[aChar].mSprite.GetHalfSize()));
					gG.SetTexture(mCharacter[aChar].mSprite.mTexture);

					if (aBold)
					{
						//gG.Sharpen(false);
						//gG.Translate(-(float)(mBoldPixels+1)/2,0.0f);
						for (int aCount=0;aCount<mBoldPixels;aCount++)
						{
							gG.DrawTexturedQuad(aDrawQuad,mCharacter[aChar].mSprite.GetTextureQuad());
							gG.Translate(1,0);
						}
						//gG.Sharpen(!aHoldFilter);
					}
					else
					{
						gG.Sharpen(false);
						gG.DrawTexturedQuad(aDrawQuad,mCharacter[aChar].mSprite.GetTextureQuad());
						gG.Sharpen(!aHoldFilter);
					}
					gG.PopTranslate();
				}
			}
			aPos.mX+=(mCharacter[aChar].mWidth+mExtraSpacing)*aScale;
			aLastChar=aChar;
		}
	}
	//gG.Translate(0.0f,mAscent);
	gG.SetColor(aNormalColor);
	gFontDrawPos=aPos;

	gG.Sharpen(!aHoldFilter);
	gG.PopTranslate();
}

float Font::WidthEX(String theText)
{
	float aResult=0;
	int aLastChar=0;
	float aMaxWidth=0;
	float aScale=1.0f;

	for (int aCount=0;aCount<theText.Len();aCount++)
	{
		unsigned char aChar=theText[aCount];
		if (aChar==10 || aChar==13)
		{
			aMaxWidth=_max(aResult,aMaxWidth);
			aResult=0;
		}
		if (aChar==mFormatCharacter)
		{
			aCount++;
			unsigned char aFormat=tolower(theText[aCount]);

			if (aFormat=='s')
			{
				int aCPos=aCount+1;
				if (theText[aCPos]=='(')
				{
					int aStart=aCPos+1;
					for (;;)
					{
						aCount++;
						if (theText[aCount]==')' || theText[aCount]==0)
						{
							aScale=theText.GetSegment(aStart,aCount-aStart).ToFloat();
							break;
						}
					}
				}
			}
			if (aFormat=='o')
			{
				int aCPos=aCount+1;
				if (theText[aCPos]=='(')
				{
					int aStart=aCPos+1;
					for (;;)
					{
						aCount++;
						if (theText[aCount]==')' || theText[aCount]==0) break;
					}
				}
			}
			if (aFormat=='c')
			{
				int aCPos=aCount+1;
				if (theText[aCPos]=='[') {aCount+=2;if (theText[aCPos+2]==']') aCount++;}
				else if (theText[aCPos]=='(')
				{
					for (;;)
					{
						aCount++;
						if (theText[aCount]==')' || theText[aCount]==0) break;
					}

				}
			}
			if (aFormat=='o')
			{
				int aCPos=aCount+1;
				if (theText[aCPos]=='(')
				{
					for (;;)
					{
						aCount++;
						if (theText[aCount]==')' || theText[aCount]==0) break;
					}
				}
			}
		}
		else
		{
			aResult+=mKern[aLastChar][aChar]*aScale;
			aResult+=(mCharacter[aChar].mWidth+mExtraSpacing)*aScale;
			aLastChar=aChar;
		}
	}

	aResult=_max(aResult,aMaxWidth);
	return aResult;
}

float Font::GetJustifyWidth(String theText, int &lineFeedPosition, char &lineFeedCharacter)
{
	float aResult=0;
	lineFeedCharacter=0;

	if (mJustifyParagraph)
	{
		lineFeedPosition=-1;
		int aLastChar=0;
		float aMaxWidth=0;
		for (int aCount=0;aCount<theText.Len();aCount++)
		{
			unsigned char aChar=theText[aCount];
			if (aChar==10 || aChar==13)
			{
				if (lineFeedPosition==-1) 
				{
					lineFeedPosition=aCount;
					lineFeedCharacter=aChar;
				}
				aResult=0;
			}
			aResult+=mKern[aLastChar][aChar];
			aResult+=mCharacter[aChar].mWidth+mExtraSpacing;
			aLastChar=aChar;

			aMaxWidth=_max(aResult,aMaxWidth);
		}
		aResult=aMaxWidth;
	}
	else
	{
		lineFeedPosition=-1;
		int aLastChar=0;
		for (int aCount=0;aCount<theText.Len();aCount++)
		{
			unsigned char aChar=theText[aCount];
			if (aChar==10 || aChar==13)
			{
				lineFeedPosition=aCount;
				lineFeedCharacter=aChar;
				break;
			}
			aResult+=mKern[aLastChar][aChar];
			aResult+=mCharacter[aChar].mWidth+mExtraSpacing;
			aLastChar=aChar;
		}
	}
	return aResult;
}

float Font::GetJustifyWidthEX(String theText, int &lineFeedPosition)
{
	float aResult=0;
	lineFeedPosition=-1;
	float aScale=1.0f;

	if (mJustifyParagraph)
	{
		float aMaxWidth=0;
		int aLastChar=0;
		for (int aCount=0;aCount<theText.Len();aCount++)
		{
			unsigned char aChar=theText[aCount];
			if (aChar==10 || aChar==13)
			{
				lineFeedPosition=aCount;
				aResult=0;
			}
			if (aChar==mFormatCharacter)
			{
				aCount++;
				unsigned char aFormat=theText[aCount];

				if (aFormat=='s')
				{
					int aCPos=aCount+1;
					if (theText[aCPos]=='(')
					{
						int aStart=aCPos+1;
						for (;;)
						{
							aCount++;
							if (theText[aCount]==')' || theText[aCount]==0)
							{
								aScale=theText.GetSegment(aStart,aCount-aStart).ToFloat();
								break;
							}
						}
					}
				}

				if (aFormat=='c')
				{
					int aCPos=aCount+1;
					if (theText[aCPos]=='[') {aCount+=2;if (theText[aCPos+2]==']') aCount++;}
					else if (theText[aCPos]=='(')
					{
						for (;;)
						{
							aCount++;
							if (theText[aCount]==')' || theText[aCount]==0) break;
						}

					}
				}
			}
			else
			{
				aResult+=mKern[aLastChar][aChar]*aScale;
				aResult+=(mCharacter[aChar].mWidth+mExtraSpacing)*aScale;
				aLastChar=aChar;
				aMaxWidth=_max(aResult,aMaxWidth);
			}
		}
		aResult=aMaxWidth;
	}
	else 
	{
		int aLastChar=0;
		for (int aCount=0;aCount<theText.Len();aCount++)
		{
			unsigned char aChar=theText[aCount];
			if (aChar==10 || aChar==13)
			{
				lineFeedPosition=aCount;
				break;
			}
			if (aChar==mFormatCharacter)
			{
				aCount++;
				unsigned char aFormat=theText[aCount];
				if (aFormat=='s')
				{
					int aCPos=aCount+1;
					if (theText[aCPos]=='(')
					{
						int aStart=aCPos+1;
						for (;;)
						{
							aCount++;
							if (theText[aCount]==')' || theText[aCount]==0)
							{
								aScale=theText.GetSegment(aStart,aCount-aStart).ToFloat();
								break;
							}
						}
					}
				}
				if (aFormat=='o')
				{
					int aCPos=aCount+1;
					if (theText[aCPos]=='(')
					{
						int aStart=aCPos+1;
						for (;;)
						{
							aCount++;
							if (theText[aCount]==')' || theText[aCount]==0) break;
						}
					}
				}


				if (aFormat=='c')
				{
					int aCPos=aCount+1;
					if (theText[aCPos]=='[') {aCount+=2;if (theText[aCPos+2]==']') aCount++;}
					else if (theText[aCPos]=='(')
					{
						for (;;)
						{
							aCount++;
							if (theText[aCount]==')' || theText[aCount]==0) break;
						}

					}
				}
			}
			else
			{
				aResult+=mKern[aLastChar][aChar]*aScale;
				aResult+=(mCharacter[aChar].mWidth+mExtraSpacing)*aScale;
				aLastChar=aChar;
			}
		}
	}
	return aResult;
}

String Font::Wrap(String theString, float theWidth)
{
	String aNewString=theString.c();
	int aPtr=0;

	float aWidth=0;
	int aLastBreak=0;

	int aStartLine=false;

	for (;;)
	{
		float aAddWidth=0;

		if (aNewString[aPtr]==0) break;
		if (aNewString[aPtr]==10 || aNewString[aPtr]==13) aWidth=aAddWidth=0;

		if (aNewString[aPtr]==' ') 
		{
			if (aStartLine)
			{
				//
				// Since we're starting a line, let's delete all 
				// leading spaces...
				//
				int aLeadCount=1;
				for (;;)
				{
					if (aNewString[aPtr+aLeadCount]!=' ') break;
					aLeadCount++;
				}

				aNewString.Delete(aPtr,aLeadCount);
				aPtr--;
			}
			else aAddWidth+=(mCharacter[32].mWidth+mExtraSpacing);
		}
		else if (mCharacter[(unsigned char)aNewString[aPtr]].mIsInitialized)
		{
			aStartLine=false;
			aAddWidth+=(float)mCharacter[(unsigned char)aNewString[aPtr]].mWidth+mExtraSpacing;
		}

		if (aWidth+aAddWidth>theWidth)
		{
			//
			// Go backwards...
			//

			aWidth=0;
			int aHoldSource=aPtr;

			for (;;)
			{
				aPtr--;
				if (aPtr==aLastBreak)
				{
					aPtr=aHoldSource;
					break;
				}

				if (aNewString[aPtr]==' ' || aNewString[aPtr]=='-') break;
			}
			aLastBreak=aPtr;
			if (aNewString[aPtr]==' ' || aNewString[aPtr]=='-') aNewString.SetChar(aPtr,10);
			else 
			{
				if (aPtr>1) aPtr--;
				aNewString.Insert(10,1,aPtr);
				aNewString.Insert('-',1,aPtr);
			}

			aStartLine=true;
		}

		aWidth+=aAddWidth;
		aPtr++;
	}

	//
	// A problem... splitting spaces across two lines.
	// 
	return aNewString;
}

String Font::WrapEX(String theString, float theWidth)
{
	String aNewString=theString.c();
	int aPtr=0;

	float aWidth=0;
	int aLastBreak=0;

	int aStartLine=false;

	for (;;)
	{
		unsigned char aChar=aNewString[aPtr];
		if (aChar==mFormatCharacter)
		{
			aPtr++;
			unsigned char aFormat=aNewString[aPtr];
			if (aFormat=='c')
			{
				int aCPos=aPtr+1;
				if (aNewString[aCPos]=='[') {aPtr+=2;if (aNewString[aCPos+2]==']') aPtr++;}
				else if (aNewString[aCPos]=='(')
				{
					for (;;)
					{
						aPtr++;
						if (aNewString[aPtr]==')' || aNewString[aPtr]==0) break;
					}
				}
			}
			aPtr++;

		}
		else
		{
			float aAddWidth=0;

			if (aNewString[aPtr]==0) break;
			if (aNewString[aPtr]==10 || aNewString[aPtr]==13) aWidth=aAddWidth=0;

			if (aNewString[aPtr]==' ') 
			{
				if (aStartLine)
				{
					//
					// Since we're starting a line, let's delete all 
					// leading spaces...
					//
					int aLeadCount=1;
					for (;;)
					{
						if (aNewString[aPtr+aLeadCount]!=' ') break;
						aLeadCount++;
					}

					aNewString.Delete(aPtr,aLeadCount);
					aPtr--;
				}
				else aAddWidth+=(mCharacter[32].mWidth+mExtraSpacing);
			}
			else if (mCharacter[(unsigned char)aNewString[aPtr]].mIsInitialized)
			{
				aStartLine=false;
				aAddWidth+=(float)mCharacter[(unsigned char)aNewString[aPtr]].mWidth+mExtraSpacing;
			}

			if (aWidth+aAddWidth>theWidth)
			{
				//
				// Go backwards...
				//

				aWidth=0;
				int aHoldSource=aPtr;

				for (;;)
				{
					aPtr--;
					if (aPtr==aLastBreak)
					{
						aPtr=aHoldSource;
						break;
					}

					if (aNewString[aPtr]==' ' || aNewString[aPtr]=='-') break;
				}
				aLastBreak=aPtr;
				//aNewString.SetChar(aPtr,10);
				if (aNewString[aPtr]==' ' || aNewString[aPtr]=='-') aNewString.SetChar(aPtr,10);
				else 
				{
					if (aPtr>1) aPtr--;
					aNewString.Insert(10,1,aPtr);
					aNewString.Insert('-',1,aPtr);
				}


				aStartLine=true;
			}

			aWidth+=aAddWidth;
			aPtr++;
		}
	}

	//
	// A problem... splitting spaces across two lines.
	// 
	return aNewString;
}

void Font::PointLowercaseAtUppercase()
{
	for (int aCount='a';aCount<='z';aCount++)
	{
		int aUpperRef=(aCount-'a')+'A';
		mCharacter[aCount]=mCharacter[aUpperRef];

		for (int aNextCount='a';aNextCount<='z';aNextCount++)
		{
			int aNextUpperRef=(aNextCount-'a')+'A';
			mKern[aCount][aNextCount]=mKern[aUpperRef][aNextUpperRef];
		}

	}
}

String Font::Truncate(String theString, float theWidth, String theTruncateString)
{
	String aResult=theString;

	float aWidth=0;
	float aMaxWidth=theWidth-Width(theTruncateString);
	int aPtr=0;

	for (;;)
	{
		unsigned char aChar=theString[aPtr];
		if (aChar==13 || aChar==10 || aChar==0) break;


		float aAddWidth=(float)mCharacter[aChar].mWidth+mExtraSpacing;
		if (aWidth+aAddWidth>aMaxWidth) 
		{
			aResult=theString.GetSegment(0,aPtr-1);
			aResult+=theTruncateString;
			break;
		}
		aWidth+=aAddWidth;
		aPtr++;
	}
	return aResult;
}

Rect Font::GetBoundingRect(String theText, float theX, float theY, int theJustify)
{
	Rect aResult;
	aResult.mX=theX;
	aResult.mY=theY-mAscent;
	aResult.mWidth=1;
	aResult.mHeight=1;

	int aLastChar=0;
	if (theJustify!=1)
	{
		int aLineFeedPosition;
		char aLineFeedChar;
		float aWidth=GetJustifyWidth(theText,aLineFeedPosition,aLineFeedChar);
		if (theJustify==0) aWidth/=2;
		if (aLineFeedPosition!=-1 && !mJustifyParagraph)
		{
			aResult=aResult.Union(GetBoundingRect(theText.GetSegment(0,aLineFeedPosition),theX,theY,theJustify));
			theY+=mPointSize+1;
			if (aLineFeedChar==13) 
			{
				theY+=mPointSize+1;
				theY+=mCRSpacing;
			}
			aResult=aResult.Union(GetBoundingRect(theText.GetSegment(aLineFeedPosition+1,9999),theX,theY,theJustify));
			return aResult;
		}
		theX-=aWidth;
	}

	//gG.Translate(0.0f,-mAscent);
	Point aStartPos=Point(theX,theY);

	Point aPos=aStartPos;
	for (int aCount=0;aCount<theText.Len();aCount++)
	{
		unsigned char aChar=theText[aCount];
		if (aChar==10)
		{
			aPos.mX=aStartPos.mX;
			aPos.mY+=mPointSize+1;
		}
		if (aChar==13)
		{
			aPos.mX=aStartPos.mX;
			aPos.mY+=mPointSize+1;
			aPos.mY+=mPointSize+1;
			aPos.mY+=mCRSpacing;
		}
		if (mCharacter[aChar].mIsInitialized) 
		{
			aPos.mX+=mKern[aLastChar][aChar];
			//mCharacter[aChar].mSprite.Center(aPos+mCharacter[aChar].mOffset);
		}
		aPos.mX+=mCharacter[aChar].mWidth+mExtraSpacing;
		aLastChar=aChar;
		aResult.mWidth=_max(aResult.mWidth,aPos.mX-aStartPos.mX);
	}
	//gG.Translate(0.0f,mAscent);

	aResult.mHeight=aPos.mY-aResult.mY;
	aResult.mX=theX;

	return aResult;
}

Rect Font::GetBoundingRectEX(String theText, float theX, float theY, int theJustify)
{
	Rect aResult;
	aResult.mX=theX;
	aResult.mY=theY-mAscent;
	aResult.mWidth=1;
	aResult.mHeight=1;

	float aScale=1.0f;

	int aLastChar=0;
	if (theJustify!=1)
	{
		int aLineFeedPosition;
		float aWidth=GetJustifyWidthEX(theText,aLineFeedPosition);
		if (theJustify==0) aWidth/=2;
		if (aLineFeedPosition!=-1 && !mJustifyParagraph)
		{
			aResult=aResult.Union(GetBoundingRectEX(theText.GetSegment(0,aLineFeedPosition),theX,theY,theJustify));
			theY+=mPointSize+1;
			aResult=aResult.Union(GetBoundingRectEX(theText.GetSegment(aLineFeedPosition+1,9999),theX,theY,theJustify));
			return aResult;
		}
		theX-=aWidth;
	}

	//gG.Translate(0.0f,-mAscent);
	Point aStartPos=Point(theX,theY);

	Point aPos=aStartPos;
	for (int aCount=0;aCount<theText.Len();aCount++)
	{
		unsigned char aChar=theText[aCount];
		if (aChar==10)
		{
			aPos.mX=aStartPos.mX;
			aPos.mY+=mPointSize+1;
		}
		else if (aChar==13)
		{
			aPos.mX=aStartPos.mX;
			aPos.mY+=mPointSize+1;
			aPos.mY+=mPointSize+1;
			aPos.mY+=mCRSpacing;
		}
		else if (aChar==mFormatCharacter)
		{
			aCount++;
			unsigned char aFormat=tolower(theText[aCount]);

			if (aFormat=='s')
			{
				int aCPos=aCount+1;
				if (theText[aCPos]=='(')
				{
					int aStart=aCPos+1;
					for (;;)
					{
						aCount++;
						if (theText[aCount]==')' || theText[aCount]==0)
						{
							aScale=theText.GetSegment(aStart,aCount-aStart).ToFloat();
							break;
						}
					}
				}
			}
			if (aFormat=='o')
			{
				int aCPos=aCount+1;
				if (theText[aCPos]=='(')
				{
					int aStart=aCPos+1;
					for (;;)
					{
						aCount++;
						if (theText[aCount]==')' || theText[aCount]==0) break;
					}
				}
			}
			if (aFormat=='c')
			{
				int aCPos=aCount+1;
				if (theText[aCPos]=='[') {aCount+=2;if (theText[aCPos+2]==']') aCount++;}
				else if (theText[aCPos]=='(')
				{
					for (;;)
					{
						aCount++;
						if (theText[aCount]==')' || theText[aCount]==0) break;
					}

				}
			}
			if (aFormat=='o')
			{
				int aCPos=aCount+1;
				if (theText[aCPos]=='(')
				{
					for (;;)
					{
						aCount++;
						if (theText[aCount]==')' || theText[aCount]==0) break;
					}
				}
			}
		}
		else
		{
			if (mCharacter[aChar].mIsInitialized) 
			{
				aPos.mX+=mKern[aLastChar][aChar];
				//mCharacter[aChar].mSprite.Center(aPos+mCharacter[aChar].mOffset);
			}
			aPos.mX+=(mCharacter[aChar].mWidth+mExtraSpacing)*aScale;
			aLastChar=aChar;
			aResult.mWidth=_max(aResult.mWidth,aPos.mX-aStartPos.mX);
		}
	}
	//gG.Translate(0.0f,mAscent);

	aResult.mHeight=aPos.mY-aResult.mY;
	aResult.mX=theX;


	return aResult;
}

String Font::Unformat(String theText)
{
	String aResult;
	int aLastChar=0;
	float aMaxWidth=0;
	float aScale=1.0f;

	for (int aCount=0;aCount<theText.Len();aCount++)
	{
		unsigned char aChar=theText[aCount];
		if (aChar==10 || aChar==13)
		{
			//			aResult+=aChar;
		}
		if (aChar==mFormatCharacter)
		{
			aCount++;
			unsigned char aFormat=tolower(theText[aCount]);

			if (aFormat=='s')
			{
				int aCPos=aCount+1;
				if (theText[aCPos]=='(')
				{
					int aStart=aCPos+1;
					for (;;)
					{
						aCount++;
						if (theText[aCount]==')' || theText[aCount]==0)
						{
							aScale=theText.GetSegment(aStart,aCount-aStart).ToFloat();
							break;
						}
					}
				}
			}
			if (aFormat=='o')
			{
				int aCPos=aCount+1;
				if (theText[aCPos]=='(')
				{
					int aStart=aCPos+1;
					for (;;)
					{
						aCount++;
						if (theText[aCount]==')' || theText[aCount]==0) break;
					}
				}
			}
			if (aFormat=='c')
			{
				int aCPos=aCount+1;
				if (theText[aCPos]=='[') {aCount+=2;if (theText[aCPos+2]==']') aCount++;}
				else if (theText[aCPos]=='(')
				{
					for (;;)
					{
						aCount++;
						if (theText[aCount]==')' || theText[aCount]==0) break;
					}

				}
			}
			if (aFormat=='o')
			{
				int aCPos=aCount+1;
				if (theText[aCPos]=='(')
				{
					for (;;)
					{
						aCount++;
						if (theText[aCount]==')' || theText[aCount]==0) break;
					}
				}
			}
		}
		else
		{
			aResult.strcat(aChar);
			aLastChar=aChar;
		}
	}

	return aResult;
}

void Font::GetHotAreaEX(String theText, RectComplex& theRect, int theStart, int theEnd, Point thePos)
{
	if (mJustify!=1)
	{
		int aLineFeedPosition;
		float aWidth=GetJustifyWidthEX(theText,aLineFeedPosition);
		if (mJustify==0) aWidth/=2;
		if (aLineFeedPosition!=-1 && !mJustifyParagraph)
		{
			GetHotAreaEX(theText.GetSegment(0,aLineFeedPosition),theRect,theStart,theEnd,thePos);
			thePos.mY+=mPointSize+1;
			GetHotAreaEX(theText.GetSegment(aLineFeedPosition+1,9999),theRect,theStart,theEnd,thePos);
			return;
		}
		thePos.mX-=aWidth;
	}

	int aLastChar=0;

	int aEffect=0;
	bool aBold=false;
	bool aItalics=false;
	bool aUnderline=false;
	float aScale=1.0f;

	//gG.Translate(0.0f,-mAscent);
	Point aStartPos=thePos;
	Point aPos=aStartPos;

	Point aOffset=Point(0,0);

	theStart=_max(0,theStart);
	theEnd=_min(theEnd,theText.Len()-1);

	float aMin=-mAscent;
	float aMax=-9999;
	for (int aCount=theStart;aCount<=theEnd;aCount++)
	{
		unsigned char aChar=theText[aCount];
		if (mCharacter[aChar].mIsInitialized)
		{
			Quad aDrawQuad=mCharacter[aChar].mSprite.GetDrawQuad();
			aMin=_min(aMin,aDrawQuad.GetBounds().mY);
			aMax=_max(aMax,aDrawQuad.GetBounds().LowerLeft().mY);
		}
	}

	float aHeight=aMax-aMin;


	for (int aCount=0;aCount<theText.Len();aCount++)
	{
		unsigned char aChar=theText[aCount];
		if (aChar==10)
		{
			aPos.mX=aStartPos.mX;
			aPos.mY+=mPointSize+1;
		}
		else if (aChar==13)
		{
			aPos.mX=aStartPos.mX;
			aPos.mY+=mPointSize+1;
			aPos.mY+=mPointSize+1;
			aPos.mY+=mCRSpacing;
		}
		else if (aChar==mFormatCharacter)
		{
			aCount++;
			unsigned char aFormat=tolower(theText[aCount]);

			if (aFormat=='i') aItalics=!aItalics;
			if (aFormat=='b') aBold=!aBold;
			if (aFormat=='u') aUnderline=!aUnderline;
			if (aFormat=='s')
			{
				int aCPos=aCount+1;
				if (theText[aCPos]=='(')
				{
					int aStart=aCPos+1;
					for (;;)
					{
						aCount++;
						if (theText[aCount]==')' || theText[aCount]==0)
						{
							aScale=theText.GetSegment(aStart,aCount-aStart).ToFloat();
							break;
						}
					}
				}
			}
			if (aFormat=='o')
			{
				int aCPos=aCount+1;
				if (theText[aCPos]=='(')
				{
					int aStart=aCPos+1;
					for (;;)
					{
						aCount++;
						if (theText[aCount]==')' || theText[aCount]==0)
						{
							String aSeg=theText.GetSegment(aStart,aCount-aStart);

							if (aSeg.Len()<1) aOffset=Point(0,0);
							else
							{
								float aX=aSeg.GetToken(',').ToFloat();
								float aY=aSeg.GetNextToken(',').ToFloat();
								aOffset=Point(aX,aY);
							}
							break;
						}

					}
				}
			}
			if (aFormat=='c')
			{
				int aCPos=aCount+1;
				if (theText[aCPos]=='[')
				{
					int aValue=theText[aCPos+1]-48;
					aCount+=2;
					if (theText[aCPos+2]==']') aCount++;
				}
				else if (theText[aCPos]=='(')
				{
					int aStart=aCPos+1;
					for (;;)
					{
						aCount++;
						if (theText[aCount]==')' || theText[aCount]==0)
						{
							float aAlpha=1.0f;
							String aSeg=theText.GetSegment(aStart,aCount-aStart);
							float aR=aSeg.GetToken(',').ToFloat();
							float aG=aSeg.GetNextToken(',').ToFloat();
							float aB=aSeg.GetNextToken(',').ToFloat();
							String aAA=aSeg.GetNextToken(',');if (aAA.Len()>0) aAlpha=aAA.ToFloat();
							break;
						}

					}

				}
				//
				// Change text color...
				//
			}
			aEffect=aItalics|aBold|aUnderline;
		}
		else
		{
			if (mCharacter[aChar].mIsInitialized) 
			{
				aPos.mX+=(float)(mKern[aLastChar][aChar])*aScale;

				if (aCount>=theStart && aCount<=theEnd)
				{
					Quad aDrawQuad=mCharacter[aChar].mSprite.GetDrawQuad();
					if (aScale!=.0f) aDrawQuad.Scale(aScale);
					aDrawQuad.Translate((aPos+aOffset+mCharacter[aChar].mOffset));
					Rect aRect=Rect(aPos.mX,aPos.mY-mAscent,mCharacter[aChar].mWidth,aHeight);
					theRect.Add(aRect);
				}
			}
			aPos.mX+=(mCharacter[aChar].mWidth+mExtraSpacing)*aScale;
			aLastChar=aChar;
		}
	}
}

void Font::GetHotArea(String theText, RectComplex& theRect, int theStart, int theEnd, Point thePos)
{
	if (mJustify!=1)
	{
		int aLineFeedPosition;
		char aLineFeedChar;
		float aWidth=GetJustifyWidth(theText,aLineFeedPosition,aLineFeedChar);
		if (mJustify==0) aWidth/=2;
		if (aLineFeedPosition!=-1 && !mJustifyParagraph)
		{
			GetHotArea(theText.GetSegment(0,aLineFeedPosition),theRect,theStart,theEnd,thePos);
			thePos.mY+=mPointSize+1;
			if (aLineFeedChar==13) 
			{
				thePos.mY+=mPointSize+1;
				thePos.mY+=mCRSpacing;
			}
			GetHotArea(theText.GetSegment(aLineFeedPosition+1,9999),theRect,theStart,theEnd,thePos);
			return;
		}
		thePos.mX-=aWidth;
	}

	int aLastChar=0;

	int aEffect=0;
	bool aBold=false;
	bool aItalics=false;
	bool aUnderline=false;
	float aScale=1.0f;

	//gG.Translate(0.0f,-mAscent);
	Point aStartPos=thePos;
	Point aPos=aStartPos;

	Point aOffset=Point(0,0);

	theStart=_max(0,theStart);
	theEnd=_min(theEnd,theText.Len()-1);

	float aMin=-mAscent;
	float aMax=-9999;
	for (int aCount=theStart;aCount<=theEnd;aCount++)
	{
		unsigned char aChar=theText[aCount];
		if (mCharacter[aChar].mIsInitialized)
		{
			Quad aDrawQuad=mCharacter[aChar].mSprite.GetDrawQuad();
			aMin=_min(aMin,aDrawQuad.GetBounds().mY);
			aMax=_max(aMax,aDrawQuad.GetBounds().LowerLeft().mY);
		}
	}

	float aHeight=aMax-aMin;

	for (int aCount=0;aCount<theText.Len();aCount++)
	{
		unsigned char aChar=theText[aCount];
		if (aChar==10)
		{
			aPos.mX=aStartPos.mX;
			aPos.mY+=mPointSize+1;
		}
		else if (aChar==13)
		{
			aPos.mX=aStartPos.mX;
			aPos.mY+=mPointSize+1;
			aPos.mY+=mPointSize+1;
			aPos.mY+=mCRSpacing;
		}
		else
		{
			if (mCharacter[aChar].mIsInitialized) 
			{
				aPos.mX+=(float)(mKern[aLastChar][aChar])*aScale;

				if (aCount>=theStart && aCount<=theEnd)
				{
					Quad aDrawQuad=mCharacter[aChar].mSprite.GetDrawQuad();
					if (aScale!=.0f) aDrawQuad.Scale(aScale);
					aDrawQuad.Translate((aPos+aOffset+mCharacter[aChar].mOffset));
					Rect aRect=Rect(aPos.mX,aPos.mY-mAscent,mCharacter[aChar].mWidth,aHeight);
					theRect.Add(aRect);
				}
			}
			aPos.mX+=(mCharacter[aChar].mWidth+mExtraSpacing)*aScale;
			aLastChar=aChar;
		}
	}
}

Point Font::CharacterPos(String theText, int breakAt)
{
	int aLastChar=0;
	Point aResult;
	float aMaxWidth=0;
	for (int aCount=0;aCount<theText.Len();aCount++)
	{
		if (aCount==breakAt) break;

		unsigned char aChar=theText[aCount];
		if (aChar==10 || aChar==13)
		{
			aResult.mX=0;
			aResult.mY+=mPointSize+1;
			if (aChar==13) 
			{
				aResult.mY+=mPointSize+1;
				aResult.mY+=mCRSpacing;
			}
		}
		else
		{
			aResult.mX+=mKern[aLastChar][aChar];
			aResult.mX+=mCharacter[aChar].mWidth+mExtraSpacing;
		}
		aLastChar=aChar;
	}

	return aResult;
}

void Font::CopyCharacter(int theSource, int theDest)
{
	mCharacter[theDest]=mCharacter[theSource];
}
#endif