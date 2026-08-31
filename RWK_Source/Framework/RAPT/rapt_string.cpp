#include "rapt_string.h"
#include "rapt_list.h"
#include "rapt_rect.h"

#include "rapt.h"

#define ISUTF(c) (((c)&0xC0)!=0x80)
static const unsigned int gOffsetsFromUTF8[6] = 
{
	0x00000000UL, 0x00003080UL, 0x000E2080UL,
	0x03C82080UL, 0xFA082080UL, 0x82082080UL
};


//#include "parsexml.h"
//#include "List.h"

//BEGIN IMPORTED

#define MAXDISCARDABLESTRING 10
String gDiscardableString[MAXDISCARDABLESTRING];
int gDiscardableStringPtr=0;

String& GetDiscardableString()
{
	gDiscardableStringPtr++;
	if (gDiscardableStringPtr>=MAXDISCARDABLESTRING) gDiscardableStringPtr=0;
	return gDiscardableString[gDiscardableStringPtr];
}

String String::CGetToken(char theSeperator, bool wantTokenInString)
{
	mCursor=mData;
	mMoreTokens=true;
	return this->CGetNextToken(theSeperator,wantTokenInString);
}

String String::CGetNextToken(char theSeperator, bool wantTokenInString)
{
	if (mCursor-mData>=Len())
	{
		mMoreTokens=false;
		String aString="";
		return aString;
	}

	char *aOldCursor=mCursor;
	//
	// Look for the delimiter...
	//
	bool aInQuote=false;
	bool aInDoubleQuote=false;
	int aInParenthesis=0;

	for (;;)
	{
		if (*(mCursor)=='\'' && !aInDoubleQuote) aInQuote=!aInQuote;
		if (*(mCursor)=='\"' && !aInQuote) aInDoubleQuote=!aInDoubleQuote;

		if (!aInQuote && !aInDoubleQuote) 
		{
			if (*(mCursor)=='(') aInParenthesis++;
			if (*(mCursor)==')') aInParenthesis--;
		}

		if ((*(mCursor)==theSeperator && !aInQuote && !aInDoubleQuote && !aInParenthesis) || *(mCursor)==0) break;
		mCursor++;
	}
	//
	// Move the cursor OVER the delimiter...
	//
	mCursor++;
	return GetSegment((unsigned int)(aOldCursor-mData),(unsigned int)(mCursor-aOldCursor)-(!wantTokenInString));
}

void String::CParseArray(Array<char> &theList)
{
	String aCommaToken=CGetToken(',');
	while (!aCommaToken.IsNull())
	{
		aCommaToken.RemoveLeadingSpaces();

		char aBrace=aCommaToken[0];
		String aCharacter=aCommaToken.GetBracedToken(aBrace,aBrace);
		theList[theList.Size()]=aCharacter[0];
		aCommaToken=CGetNextToken(',');
	}
}

#include "os_core.h"
void String::CParseArray(Array<String> &theList)
{
	String aCommaToken=CGetToken(',');
	while (!aCommaToken.IsNull())
	{
		aCommaToken.RemoveLeadingSpaces();

		char aBrace=aCommaToken[0];
		String aCharacter=aCommaToken.GetBracedToken(aBrace,aBrace);
		theList[theList.Size()]=aCharacter;
		aCommaToken=CGetNextToken(',');
	}
}


void String::CParseArray(Array<int> &theList)
{
	//
	// Okay, we have recieved something that looks like this:
	// 1,2,3,4,5,6,7,8
	//
	// Just add those to an int list
	//
	String aCommaToken=CGetToken(',');
	while (!aCommaToken.IsNull())
	{
		aCommaToken.RemoveLeadingSpaces();
		theList[theList.Size()]=aCommaToken.ToInt();
		aCommaToken=CGetNextToken(',');
	}
}

void String::CParseArray(Array<Point> &theList)
{
	//
	// Okay, we have something that looks like this...
	// (120,100),(50,50),(301,32)
	//
	// And we'll return a list of points
	//
	String aPointToken=GetBracedToken('(',')');
	while (!aPointToken.IsNull())
	{
		Point aPoint;
		aPoint.mX=aPointToken.GetToken(',').ToFloat();
		aPoint.mY=aPointToken.GetNextToken(',').ToFloat();
		theList[theList.Size()]=aPoint;

		aPointToken=GetNextBracedToken('(',')');
	}
}

void String::CParseArray(Array<Rect> &theList)
{
	//
	// Okay, we have something that looks like this...
	// (120,100),(50,50),(301,32)
	//
	// And we'll return a list of points
	//
	String aPointToken=GetBracedToken('(',')');
	while (!aPointToken.IsNull())
	{
		Rect aRect;

		aRect.mX=aPointToken.GetToken(',').ToFloat();
		aRect.mY=aPointToken.GetNextToken(',').ToFloat();
		aRect.mWidth=aPointToken.GetNextToken(',').ToFloat();
		aRect.mHeight=aPointToken.GetNextToken(',').ToFloat();
		theList[theList.Size()]=aRect;
		aPointToken=GetNextBracedToken('(',')');
	}
}

String String::GetURLEncode()
{
	char *aEncoders="$&+,/:;=?@ \"<>#%{}|\\^~[]`";
	String aNewString;

	for (int aCount=0;aCount<Len();aCount++)
	{
		char aChar=mData[aCount];
		bool aEncode=false;
		for (int aCheck=0;aCheck<(int)::strlen(aEncoders);aCheck++)
		{
			if (aChar==aEncoders[aCheck])
			{
				aEncode=true;
				break;
			}
		}

		if (aEncode) aNewString+=Sprintf("%%%.2x",aChar);
		else aNewString+=Sprintf("%c",aChar);
	}

	return aNewString;
}

String String::GetURLDecode()
{
	String aNewString;

	char ch[2];ch[1]=0;
	int i, ii;
	for (i=0; i<Length(); i++) 
	{
		if (mData[i]=='%') 
		{
			sscanf(GetSegment(i+1,2).c(), "%x", &ii);
			ch[0]=(char)(ii);
			aNewString+=ch;
			i=i+2;
		} 
		else 
		{
			ch[0]=mData[i];
			aNewString+=ch;
		}
	}

	return aNewString;
}


void String::URLEncode()
{
	strcpy(GetURLEncode());
}

void String::URLDecode()
{
	strcpy(GetURLDecode());
}

//END IMPORTED

void String::CloudKeyEncode(char theEncodeChar)
{
	MakeUnique();
	String aForbidden=" /\\:*\"/\\[]:;|=,.$&+;?@<>#%{}^`";

	for (int aCount=0;aCount<Len();aCount++)
	{
		if (aForbidden.Find(mData[aCount])>=0 || mData[aCount]==theEncodeChar)
		{
			char aWas=mData[aCount];
			mData[aCount]=theEncodeChar;
			Insert(Sprintf("%.3d",aWas,aCount),aCount+1);
			aCount+=3;
		}
	}
}

void String::CloudKeyUnencode(char theEncodeChar)
{
	MakeUnique();
	for (int aCount=0;aCount<Len();aCount++)
	{
		if (mData[aCount]==theEncodeChar)
		{
			char aValue[4];
			aValue[0]=mData[aCount+1];
			aValue[1]=mData[aCount+2];
			aValue[2]=mData[aCount+3];
			aValue[3]=0;
			mData[aCount]=atoi(aValue);
			Delete(aCount+1,3);
		}
	}
}





String String::GetNumber(int thePosition)
{
	String aReturn;
	if(mData && thePosition < mLength && thePosition >= 0)
	{
		char *aSeek = &mData[thePosition];
		while(*aSeek && !(*aSeek >= '0' && *aSeek <= '9'))aSeek++;
		if(!*aSeek)goto RETURN;
		char *aStart = aSeek;
		bool aHaveDecimal = false;
		if(aStart > &mData[thePosition])
		{
			aSeek--;
			if(*aSeek == '+' || *aSeek == '-' || *aSeek == '.')
			{
				aStart--;
				if(*aSeek == '.')
				{
					aHaveDecimal=true;
					if(aStart > &mData[thePosition])
					{
						aStart--;
						if(*aStart != '+' && *aStart != '-')aStart++;
					}
				}
			}
			aSeek++;
		}
		while(*aSeek && (*aSeek >= '0' && *aSeek <= '9'))aSeek++;
		if(!aHaveDecimal)
		{
			if(*aSeek == '.')
			{
				aSeek++;
				while(*aSeek && (*aSeek >= '0' && *aSeek <= '9'))aSeek++;
			}
		}
		int aLength = (int)(aSeek - aStart);
		char *aNumber = new char[aLength + 1];
		aNumber[aLength] = 0;
		memcpy(aNumber, aStart, aLength);
		aReturn.mData = aNumber;
		aReturn.mLength = aLength;
		aReturn.mRefs=new int;*aReturn.mRefs=1;
		mCursor=aSeek;
	}
RETURN:;
	return aReturn;
}

longlong String::CastAsLongLong(bool fromTheEnd)
{
	char aData[16];
	memset(aData,0,16);
	if (!fromTheEnd) memcpy(aData,mData,_min(16,Len()));
	else memcpy(aData,mData+_max(0,Len()-16),_min(16,Len()));
	return *(longlong*)aData;
}


void String::ToPointEval(float &theX, float &theY)
{
	String aFirst=GetSegmentBefore(',');
	String aSecond=GetSegmentAfter(',');
	theX=gMath.Eval(aFirst);
	theY=gMath.Eval(aSecond);
}

void String::ToPoint(float &theX, float &theY)
{
	theX=0;
	theY=0;
	if(mData)
	{
		char *aPtr = mData;
		while(*aPtr && !(*aPtr == '.' || *aPtr == '-' || (*aPtr >= '0' && *aPtr <= '9')))aPtr++;
		char *aXStart=aPtr;
		if(*aPtr == '-')aPtr++;
		while(*aPtr && (*aPtr == '.' || (*aPtr >= '0' && *aPtr <= '9')))aPtr++;
		int aXLength = (int)(aPtr - aXStart);
		while(*aPtr && !(*aPtr == '.' || *aPtr == '-' || (*aPtr >= '0' && *aPtr <= '9')))aPtr++;
		char *aYStart=aPtr;
		if(*aPtr == '-')aPtr++;
		while(*aPtr && (*aPtr == '.' || (*aPtr >= '0' && *aPtr <= '9')))aPtr++;
		int aYLength = (int)(aPtr - aYStart);
		if(aXLength > 0 && aYLength > 0)
		{
			char *aX = new char[aXLength + 1];
			char *aY = new char[aYLength + 1];
			aX[aXLength]=0;
			aY[aYLength]=0;
			memcpy(aX, aXStart, aXLength);
			memcpy(aY, aYStart, aYLength);
			theX = (float)atof(aX);
			theY = (float)atof(aY);
			_DeleteArray(aX);
			_DeleteArray(aY);
		}
	}
}

bool IsNumber(char theChar, bool allowOperators)
{
	if (allowOperators) if(theChar == '-' || theChar == '+') return true;
	if(theChar >= '0' && theChar <= '9') return true;
	if(theChar == '.') return true;
	return false;
}

bool String::IsNumber()
{
	if(!mData)return false;
	char *aChar = mData;
	bool aReturn = false;
	if(*aChar == '-' || *aChar == '+')aChar++;
	if(*aChar >= '0' && *aChar <= '9')
	{
		aReturn=true;
		while(*aChar >= '0' && *aChar <= '9')aChar++;
	}
	if(*aChar == '.')aChar++;
	if(*aChar >= '0' && *aChar <= '9')
	{
		aReturn=true;
		while(*aChar >= '0' && *aChar <= '9')aChar++;
	}
	aReturn &= *aChar == 0;
	return aReturn;
}

void String::Concatenate(char *theString, int theNum)
{
	if(theNum > 0 && theString != 0)
	{
		int aNum = theNum;
		char *aTraverse = theString;
		while(aNum && *aTraverse){--aNum;++aTraverse;}
		aNum = (theNum - aNum);
		if(theNum > 0)
		{
			int aNewLen = mLength + aNum;
			char *aChar = new char[aNewLen + 1];aChar[aNewLen]=0;
			Stamp(aChar,mData,0,mLength);
			Stamp(aChar,theString,mLength,aNum);
			Free();
			mData=aChar;
			mLength=aNewLen;
			mRefs=new int;*mRefs=1;
		}
	}
}

void String::Concatenate(String &theString, int theNum)
{
	if(theNum > 0 && theString.mLength > 0)
	{
		int aNum = _min(theNum, theString.mLength);
		int aNewLen = mLength + aNum;
		char *aChar = new char[aNewLen + 1];aChar[aNewLen]=0;
		Stamp(aChar,mData,0,mLength);
		Stamp(aChar,theString.mData,mLength,aNum);
		Free();
		mData=aChar;
		mLength=aNewLen;
		mRefs=new int;*mRefs=1;
	}
}

String String::GetLongestCommonSubsequence(String &theString)
{
	String aReturn;
	int aLength = theString.mLength;
	if(mLength && aLength)
	{
		int **aTemp = new int*[mLength+1];
		aTemp[0] = new int[aLength+1];
		for(int i=0;i<=aLength;i++)aTemp[0][i]=0;
		for(int i=1;i<=mLength;i++)
		{
			aTemp[i] = new int[aLength+1];
			aTemp[i][0]=0;
		}
		char *aChar = theString.mData;
		for(int i=1;i<=mLength;i++)for(int j=1;j<=aLength;j++)
		{
			if(mData[i-1] == aChar[j-1])aTemp[i][j]=aTemp[i-1][j-1] + 1;
            else aTemp[i][j]=_max(aTemp[i][j-1], aTemp[i-1][j]);
		}

		int i=mLength, j=aLength;
		int aLen = aTemp[mLength][aLength];
		char *aFinal = new char[aLen + 1];aFinal[aLen]=0;
		char *aStamp = &aFinal[aLen-1];
		while(i && j)
		{
			char aOne = mData[i-1];
			if(aOne == aChar[j-1])
			{
				*aStamp=aOne;
				aStamp--;
				i--;
				j--;
			}
			else if(aTemp[i][j-1] > aTemp[i-1][j])
			{
				j--;
			}
			else
			{
				i--;
			}
		}
		aReturn.mData=aFinal;
		aReturn.mLength = aLen;
		aReturn.mRefs=new int;*aReturn.mRefs=1;
		for(int i=0;i<=mLength;i++) _DeleteArray(aTemp[i]);
		_DeleteArray(aTemp);
	}
	else
	{
		aReturn="";
	}
	return aReturn;
}

void String::Reverse()
{
	if(mLength > 1)
	{
		PrepForInlineModification();
		char *aStart = mData;
		char *aEnd = &mData[mLength-1];
		char *aHalf = &mData[mLength / 2];
		char aTemp;
		while(aStart < aHalf)
		{
			aTemp = *aStart;
			*aStart = *aEnd;
			*aEnd = aTemp;
			aStart++;
			aEnd--;
		}
	}
}

void String::Remove(char *theDisallowed)
{
	if(!theDisallowed || mLength == 0)return;
	char *aNew = new char[mLength+1];
	char *aStamp = aNew;
	char *aChar = mData;
	char *aTraverse;
	while(*aChar)
	{
		aTraverse = theDisallowed;
		*aStamp = *aChar;
		aStamp++;
		while(*aTraverse)
		{
			if(*aTraverse == *aChar)
			{
				aStamp--;
				break;
			}
			aTraverse++;
		}
		aChar++;
	}
	*aStamp=0;
	Free();
	mLength = (int)(aStamp - aNew);
	mData = aNew;
	mRefs = new int;*mRefs=1;
}

void String::RemoveNonPrintableCharacters()
{
	if(mLength == 0)return;
	char *aNew = new char[mLength+1];
	char *aStamp = aNew;
	char *aChar = mData;
	while(*aChar)
	{
		*aStamp = *aChar;
		aStamp++;
		if (*aChar<32) aStamp--;
		aChar++;
	}
	*aStamp=0;
	Free();
	mLength = (int)(aStamp - aNew);
	mData = aNew;
	mRefs = new int;*mRefs=1;
}

void String::Filter(char *theAllowed)
{
	if(!theAllowed || mLength == 0)
	{
		Free();
		return;
	}
	char *aNew = new char[mLength+1];
	char *aStamp = aNew;
	char *aChar = mData;
	char *aTraverse;
	while(*aChar)
	{
		aTraverse = theAllowed;
		while(*aTraverse)
		{
			if(*aTraverse == *aChar)
			{
				*aStamp = *aTraverse;
				aStamp++;
				break;
			}
			aTraverse++;
		}
		aChar++;
	}
	*aStamp=0;
	Free();
	mLength = (int)(aStamp - aNew);
	mData = aNew;
	mRefs = new int;*mRefs=1;
}

void String::Delete(char *theString)
{
	if(mLength == 0)return;
	int aLen = Length(theString);
	if(aLen == 0)return;
	int aFind = Find(theString);
	if(aFind != -1)
	{
		ChrList aList;
		int aLength = mLength;
		while(aFind != -1)
		{
			aList += &mData[aFind];
			aFind = Find(theString, aFind += aLen);
			aLength -= aLen;
		}
		char *aChar = new char[aLength + 1];
		aChar[aLength]=0;
		char *aStamp = aChar;
		char *aNew = aList.Get();
		char *aLast=mData;
		while(aNew)
		{
			aStamp = GetStamp(aStamp,aLast,aNew);
			aLast = aNew + aLen;
			aNew = aList.Get();
		}
		Stamp(aStamp, aLast, &mData[mLength]);
		Free();
		mLength=aLength;
		mRefs=new int;*mRefs=1;
		mData=aChar;
	}
}

void String::DeleteI(char *theString)
{
	if(mLength == 0)return;
	int aLen = Length(theString);
	if(aLen == 0)return;
	int aFind = FindI(theString);
	if(aFind != -1)
	{
		Delete(aFind,aLen);
/*
		ChrList aList;
		int aLength = mLength;
		while(aFind != -1)
		{
			aList += &mData[aFind];
			aFind = Find(theString, aFind += aLen);
			aLength -= aLen;
		}
		char *aChar = new char[aLength + 1];
		aChar[aLength]=0;
		char *aStamp = aChar;
		char *aNew = aList.Get();
		char *aLast=mData;
		while(aNew)
		{
			aStamp = GetStamp(aStamp,aLast,aNew);
			aLast = aNew + aLen;
			aNew = aList.Get();
		}
		Stamp(aStamp, aLast, &mData[mLength]);
		Free();
		mLength=aLength;
		mRefs=new int;*mRefs=1;
		mData=aChar;
*/
	}
}


void String::Delete(int thePosition, int theLength)
{
	int aFinish = thePosition + theLength;
	if(aFinish > mLength)aFinish = mLength;
	if(thePosition < 0)thePosition=0;
	theLength=aFinish-thePosition;
	if(thePosition >= mLength || mLength == 0 || theLength < 1)return;
	int aNewLength = thePosition + (mLength - (thePosition + theLength));
	char *aChar = new char[aNewLength + 1];
	aChar[aNewLength]=0;
	Stamp(aChar, mData, 0, thePosition);
	Stamp(aChar, &mData[thePosition + theLength], thePosition, (mLength - (thePosition + theLength)));
	Free();
	mData = aChar;
	mRefs = new int;*mRefs=1;
	mLength = aNewLength;
}

void String::Delete(int thePosition)
{
	if(thePosition >= mLength || mLength == 0)return;
	if(thePosition <= 0){Free();return;}
	char *aChar = new char[thePosition + 1];aChar[thePosition]=0;
	Stamp(aChar, mData, 0, thePosition);
	Free();
	mData = aChar;
	mRefs = new int;*mRefs=1;
	mLength = thePosition;
}

String String::GetToken(char theSeperator, bool wantTokenInString)
{
	mCursor = mData;
	mMoreTokens=true;
	return GetNextToken(theSeperator,wantTokenInString);
}

String String::GetNextToken(char theSeperator, bool wantTokenInString)
{
	String aReturn;
	if(mCursor != 0 && (mCursor - mData) < mLength)
	{
		char *aStart = mCursor;
		while(*mCursor != 0 && *mCursor != theSeperator)mCursor++;
		aReturn=GetSubString((int)(aStart - mData),(int)(mCursor-aStart+wantTokenInString));
	}
	else mMoreTokens=false;
	mCursor++;
	
	return aReturn;
}

String String::GetCodeToken(char theSeperator, bool wantTokenInString)
{
	mCursor = mData;
	mMoreTokens=true;
	return GetNextCodeToken(theSeperator,wantTokenInString);
}

String String::GetNextCodeToken(char theSeperator, bool wantTokenInString)
{
	String aReturn;
	char aInQuote=0;
	enum
	{
		INSIDE_QUOTE=0x01,
		INSIDE_1QUOTE=0x02,
		INSIDE_COMMENT=0x04,
		INSIDE_BIGCOMMENT=0x08,
	};

	if(mCursor != 0 && (mCursor - mData) < mLength)
	{
		char *aStart = mCursor;
		while(*mCursor != 0 && (*mCursor != theSeperator || aInQuote)) 
		{
			if (*mCursor=='\"')
			{
				if (aInQuote&INSIDE_QUOTE) aInQuote^=INSIDE_QUOTE;
				else if (!aInQuote) aInQuote|=INSIDE_QUOTE;
			}
			else if (*mCursor=='\'')
			{
				if (aInQuote&INSIDE_1QUOTE) aInQuote^=INSIDE_1QUOTE;
				else if (!aInQuote) aInQuote|=INSIDE_1QUOTE;
			}
			//else if (*mCursor=='/' && *(mCursor+1)=='/') aInQuote|=INSIDE_COMMENT;
			//else if (*mCursor==10) if (aInQuote&INSIDE_COMMENT) {mCursor++;break;}
		
			mCursor++;
		}
		aReturn=GetSubString((int)(aStart - mData),(int)(mCursor-aStart+wantTokenInString));
	}
	else mMoreTokens=false;
	mCursor++;

	return aReturn;

	/*
	Array<char> aNew;aNew.GuaranteeSize(Len()+1);
	int aWrite=0;

	char aInside=0;
	enum
	{
		INSIDE_QUOTE=0x01,
		INSIDE_1QUOTE=0x02,
		INSIDE_COMMENT=0x04,
		INSIDE_BIGCOMMENT=0x08,
	};

	while(*mCursor != 0 && (*mCursor != theSeperator || aInside)) 
	{
		bool aSkip=false;
		char aChar=*mCursor;
		char aNextChar=*(mCursor+1);
		if (aChar=='\"') {aInside^=INSIDE_QUOTE;}
		else if (aChar=='\'') {aInside^=INSIDE_1QUOTE;}
		else if (aChar=='/' && aNextChar=='/') {aInside|=INSIDE_COMMENT;}
		else if (aChar=='/' && aNextChar=='*') {aInside|=INSIDE_BIGCOMMENT;}
		else if (aChar=='*' && aNextChar=='/') {aInside|=INSIDE_BIGCOMMENT;aInside^=INSIDE_BIGCOMMENT;mCursor++;aSkip=true;}
		else if (aChar==10) {aInside|=INSIDE_COMMENT;aInside^=INSIDE_COMMENT;}

		if (!aSkip && !(aInside&(INSIDE_COMMENT|INSIDE_BIGCOMMENT))) 
		{
			if (aChar>=32) aNew[aWrite++]=aChar;
		}
		mCursor++;
	}
	if (wantTokenInString) if (*mCursor==theSeperator) aNew[aWrite++]=*mCursor;
	mCursor++;

	aNew[aWrite++]=0;
	String aResult;aResult.strcpy(aNew.mArray);
	return aResult;
	*/
}



String String::GetBracedToken(char thePositionBrace, char theEndBrace, bool wantTokenInString)
{
	mCursor = mData;
	return GetNextBracedToken(thePositionBrace,theEndBrace,wantTokenInString);
}

String String::GetNextBracedToken(char theStartBrace, char theEndBrace, bool wantTokenInString)
{
		String aReturn;
		if(mCursor != 0 && (mCursor - mData) < mLength)
		{
			while(*mCursor != 0 && *mCursor != theStartBrace)mCursor++;
			if(*mCursor == theStartBrace)
			{
				mCursor++;
				char *aStart = mCursor;
				int aDepth = 1;
				while(*mCursor)
				{
					if(*mCursor == theStartBrace)
					{
						if(theStartBrace!=theEndBrace)aDepth++;
					}
					if(*mCursor == theEndBrace)
					{
						if(--aDepth == 0)break;
					}
					mCursor++;
				}
				if(!aDepth)
				{
					return GetSubString((int)(aStart - mData - wantTokenInString), (int)(mCursor - aStart + wantTokenInString + wantTokenInString));
				}
				else
				{
					mCursor = 0;
				}
			}
		}
		mCursor++;
		return aReturn;
}

String String::GetSubString(int thePosition, int theLength)
{
	String aReturn;
	if(thePosition < mLength && theLength > 0)
	{
		int aEnd = thePosition + theLength;
		if(thePosition < 0)thePosition=0;
		if(aEnd > mLength)aEnd=mLength;
		theLength = aEnd - thePosition;
		if(aEnd > thePosition)
		{
			char *aChar = new char[theLength+1];
			aChar[theLength]=0;
			Stamp(aChar, &mData[thePosition],0,theLength);

			aReturn.mData=aChar;
			aReturn.mLength=theLength;
			aReturn.mRefs=new int;*aReturn.mRefs=1;
		}
	}
	return aReturn;
}

void String::RemoveLeadingSpaces()
{
	if(mLength == 0)return;
	if(*mData > 32 || *mData<0)return;
	char *aFinal = &mData[mLength-1];
	char *aChar = mData;
	while(aChar <= aFinal && *aChar < 33){aChar++;}
	int aOff = (int)(aChar - mData);
	int aLength = mLength - aOff;
	aChar = new char[aLength + 1];
	aChar[aLength]=0;
	Stamp(aChar,&mData[aOff],0,aLength);
	Free();
	mData=aChar;
	mRefs=new int;*mRefs=1;
	mLength=aLength;
}


void String::RemoveTrailingSpaces()
{
	if(mLength == 0)return;
	char *aChar = &mData[mLength-1];
	if(*aChar > 32 || *aChar<0)return;
	while(aChar >= mData && *aChar < 33){aChar--;}
	int aLength = (int)(aChar - mData + 1);
	aChar = new char[aLength + 1];
	aChar[aLength]=0;
	Stamp(aChar,mData,0,aLength);
	Free();
	mData=aChar;
	mRefs=new int;*mRefs=1;
	mLength=aLength;
}

void String::Insert(char theChar, int theQuantity, int thePosition)
{
	if(theQuantity <= 0)return;

	if(thePosition < 0)thePosition = 0;
	if(thePosition > mLength)thePosition=mLength;
	int aNewLen = mLength + theQuantity;
	char *aChar = new char[aNewLen + 1];
	Stamp(aChar, mData, 0, thePosition);

	char *aStamp = &aChar[thePosition];
	char *aStampFin = &aChar[thePosition + theQuantity];
	while(aStamp < aStampFin)
	{
		*aStamp=theChar;
		aStamp++;
	}

	Stamp(aChar, &mData[thePosition], thePosition + theQuantity, mLength - thePosition);
	Free();
	mLength = aNewLen;
	aChar[mLength]=0;
	mRefs=new int;*mRefs=1;
	mData = aChar;
}

void String::Insert(char theChar, int thePosition)
{
	if(theChar)
	{
		if(thePosition < 0)thePosition = 0;
		if(thePosition > mLength)thePosition=mLength;
		int aNewLen = mLength + 1;
		char *aChar = new char[aNewLen + 1];
		Stamp(aChar, mData, 0, thePosition);
		aChar[thePosition]=theChar;
		Stamp(aChar, &mData[thePosition], thePosition + 1, mLength - thePosition);
		Free();
		mLength = aNewLen;
		aChar[mLength]=0;
		mRefs=new int;*mRefs=1;
		mData = aChar;
	}
}

void String::Insert(char *theString, int thePosition)
{
	if(int aLen = Length(theString))
	{
		if(thePosition < 0)thePosition = 0;
		if(thePosition > mLength)thePosition=mLength;
		int aNewLen = mLength + aLen;
		char *aChar = new char[aNewLen + 1];
		Stamp(aChar, mData, 0, thePosition);
		Stamp(aChar, theString, thePosition, aLen);
		Stamp(aChar, &mData[thePosition], thePosition + aLen, mLength - thePosition);
		Free();
		mLength = aNewLen;
		aChar[mLength]=0;
		mRefs=new int;*mRefs=1;
		mData = aChar;
	}
}

void String::Insert(char *theString, int thePosition, int theAmount)
{
	int aLen = _min(Length(theString), theAmount);
	if(aLen > 0)
	{
		if(thePosition < 0)thePosition = 0;
		if(thePosition > mLength)thePosition=mLength;
		int aNewLen = mLength + aLen;
		char *aChar = new char[aNewLen + 1];
		Stamp(aChar, mData, 0, thePosition);
		Stamp(aChar, theString, thePosition, aLen);
		Stamp(aChar, &mData[thePosition], thePosition + aLen, mLength - thePosition);
		Free();
		mLength = aNewLen;
		aChar[mLength]=0;
		mRefs=new int;*mRefs=1;
		mData = aChar;
	}
}

void String::Insert(String &theString, int thePosition, int theAmount)
{
	int aLen = _min(theString.mLength, theAmount);
	if(aLen > 0)
	{
		if(thePosition < 0)thePosition = 0;
		if(thePosition > mLength)thePosition=mLength;
		int aNewLen = mLength + aLen;
		char *aChar = new char[aNewLen + 1];
		Stamp(aChar, mData, 0, thePosition);
		Stamp(aChar, theString.mData, thePosition, aLen);
		Stamp(aChar, &mData[thePosition], thePosition + aLen, mLength - thePosition);
		Free();
		mLength = aNewLen;
		aChar[mLength]=0;
		mRefs=new int;*mRefs=1;
		mData = aChar;
	}
}

void String::Insert(String &theString, int thePosition)
{
	if(thePosition < 0)thePosition = 0;
	if(thePosition > mLength)thePosition=mLength;
	if(int aLen = theString.mLength)
	{
		int aNewLen = mLength + aLen;
		char *aChar = new char[aNewLen + 1];
		Stamp(aChar, mData, 0, thePosition);
		Stamp(aChar, theString.mData, thePosition, aLen);
		Stamp(aChar, &mData[thePosition], thePosition + aLen, mLength - thePosition);
		Free();
		mLength = aNewLen;
		aChar[mLength]=0;
		mRefs=new int;*mRefs=1;
		mData = aChar;
	}
}

void String::Write(char theChar, int thePosition)
{
	if(theChar)
	{
		if(thePosition < 0)thePosition = 0;
		if(thePosition > mLength)thePosition=mLength;
		int aNewLen = 1 + thePosition;
		char *aChar;
		if(aNewLen > mLength)
		{
			aChar = new char[aNewLen + 1];
			Stamp(aChar, mData, 0, mLength);
			Free();
			aChar[thePosition]=theChar;
		}
		else
		{
			aNewLen=mLength;
			aChar = new char[mLength + 1];
			Stamp(aChar, mData, 0, thePosition);
			Stamp(aChar, &mData[thePosition + 1], thePosition + 1, mLength - thePosition - 1);
			Free();
			aChar[thePosition]=theChar;
		}
		mLength = aNewLen;
		aChar[mLength]=0;
		mRefs=new int;*mRefs=1;
		mData = aChar;
	}
}

void String::Write(char theChar, int theQuantity, int thePosition)
{
	if(thePosition < 0)thePosition = 0;
	if(thePosition > mLength)thePosition=mLength;
	if(theQuantity > 0)
	{
		int aNewLen = theQuantity + thePosition;
		char *aChar;
		if(aNewLen > mLength)
		{
			aChar = new char[aNewLen + 1];
			Stamp(aChar, mData, 0, mLength);
			Free();
		}
		else
		{
			aNewLen=mLength;
			aChar = new char[mLength + 1];
			Stamp(aChar, mData, 0, thePosition);
			Stamp(aChar, &mData[thePosition + theQuantity], thePosition + theQuantity, mLength - thePosition - theQuantity);
			Free();
		}

		char *aStamp = &aChar[thePosition];
		char *aStampFin = &aChar[thePosition + theQuantity];
		while(aStamp < aStampFin)
		{
			*aStamp=theChar;
			aStamp++;
		}

		mLength = aNewLen;
		aChar[mLength]=0;
		mRefs=new int;*mRefs=1;
		mData = aChar;
	}
}

void String::Write(char *theString, int thePosition, int theAmount)
{
	if(thePosition < 0)thePosition = 0;
	if(thePosition > mLength)thePosition=mLength;
	int aLen = _min(theAmount,Length(theString));
	if(aLen > 0)
	{
		int aNewLen = aLen + thePosition;
		char *aChar;
		if(aNewLen > mLength)
		{
			aChar = new char[aNewLen + 1];
			Stamp(aChar, mData, 0, mLength);
			Free();
			Stamp(aChar, theString, thePosition, aLen);
		}
		else
		{
			aNewLen=mLength;
			aChar = new char[mLength + 1];
			Stamp(aChar, mData, 0, thePosition);
			Stamp(aChar, &mData[thePosition + aLen], thePosition + aLen, mLength - thePosition - aLen);
			Free();
			Stamp(aChar, theString, thePosition, aLen);
		}
		mLength = aNewLen;
		aChar[mLength]=0;
		mRefs=new int;*mRefs=1;
		mData = aChar;
	}
}

void String::Write(String &theString, int thePosition, int theAmount)
{
	if(thePosition < 0)thePosition = 0;
	if(thePosition > mLength)thePosition=mLength;
	int aLen = _min(theAmount,theString.mLength);
	if(aLen > 0)
	{
		int aNewLen = aLen + thePosition;
		char *aChar;
		if(aNewLen > mLength)
		{
			aChar = new char[aNewLen + 1];
			Stamp(aChar, mData, 0, mLength);
			Free();
			Stamp(aChar, theString.mData, thePosition, aLen);
		}
		else
		{
			aNewLen=mLength;
			aChar = new char[mLength + 1];
			Stamp(aChar, mData, 0, thePosition);
			Stamp(aChar, &mData[thePosition + aLen], thePosition + aLen, mLength - thePosition - aLen);
			Free();
			Stamp(aChar, theString.mData, thePosition, aLen);
		}
		mLength = aNewLen;
		aChar[mLength]=0;
		mRefs=new int;*mRefs=1;
		mData = aChar;
	}
}

void String::Write(char *theString, int thePosition)
{
	if(thePosition < 0)thePosition = 0;
	if(thePosition > mLength)thePosition=mLength;
	if(int aLen = Length(theString))
	{
		int aNewLen = aLen + thePosition;
		char *aChar;
		if(aNewLen > mLength)
		{
			aChar = new char[aNewLen + 1];
			Stamp(aChar, mData, 0, mLength);
			Free();
			Stamp(aChar, theString, thePosition, aLen);
		}
		else
		{
			aNewLen=mLength;
			aChar = new char[mLength + 1];
			Stamp(aChar, mData, 0, thePosition);
			Stamp(aChar, &mData[thePosition + aLen], thePosition + aLen, mLength - thePosition - aLen);
			Free();
			Stamp(aChar, theString, thePosition, aLen);
		}
		mLength = aNewLen;
		aChar[mLength]=0;
		mRefs=new int;*mRefs=1;
		mData = aChar;
	}
}

void String::Write(String &theString, int thePosition)
{
	if(thePosition < 0)thePosition = 0;
	if(thePosition > mLength)thePosition=mLength;
	if(int aLen = theString.mLength)
	{
		int aNewLen = aLen + thePosition;
		char *aChar;
		if(aNewLen > mLength)
		{
			aChar = new char[aNewLen + 1];
			Stamp(aChar, mData, 0, mLength);
			Free();
			Stamp(aChar, theString.mData, thePosition, aLen);
		}
		else
		{
			aNewLen=mLength;
			aChar = new char[mLength + 1];
			Stamp(aChar, mData, 0, thePosition);
			Stamp(aChar, &mData[thePosition + aLen], thePosition + aLen, mLength - thePosition - aLen);
			Free();
			Stamp(aChar, theString.mData, thePosition, aLen);
		}
		mLength = aNewLen;
		aChar[mLength]=0;
		mRefs=new int;*mRefs=1;
		mData = aChar;
	}
}

String String::operator+(char theChar)
{
	int aLength = mLength + 1;
	char *aChar = new char[aLength + 1];
	aChar[aLength]=0;
	String aReturn;
	aReturn.mRefs=new int;*aReturn.mRefs=1;
	aReturn.mLength=aLength;
	aReturn.mData=aChar;
	Stamp(aChar,mData,0,mLength);
	aChar[mLength]=theChar;
	return aReturn;
}

String String::operator + (String &theString)
{
	int aLength = mLength + theString.mLength;
	char *aChar = new char[aLength + 1];
	aChar[aLength]=0;
	String aReturn;
	aReturn.mRefs=new int;*aReturn.mRefs=1;
	aReturn.mLength=aLength;
	aReturn.mData=aChar;
	Stamp(aChar,mData,0,mLength);
	Stamp(aChar,theString.mData,mLength,theString.mLength);
	return aReturn;
}

String String::operator + (char *theString)
{
	int aCharLength = Length(theString);
	int aLength = mLength + aCharLength;
	char *aChar = new char[aLength + 1];
	aChar[aLength]=0;
	String aReturn;
	aReturn.mRefs=new int;*aReturn.mRefs=1;
	aReturn.mLength=aLength;
	aReturn.mData=aChar;
	Stamp(aChar,mData,0,mLength);
	Stamp(aChar,theString,mLength,aCharLength);
	return aReturn;
}

void String::operator+=(char theChar)
{
	if(!theChar)return;

	PUSHCURSOR;

	int aLength = mLength + 1;
	char *aChar = new char[aLength + 1];
	aChar[aLength]=0;
	Stamp(aChar,mData,0,mLength);
	aChar[mLength]=theChar;

	Free();
	mData = aChar;
	mLength = aLength;
	mRefs=new int;*mRefs=1;

	POPCURSOR;
}

void String::operator+=(unsigned char theChar)
{
	if(!theChar)return;

	PUSHCURSOR;

	int aLength = mLength + 1;
	char *aChar = new char[aLength + 1];
	aChar[aLength]=0;
	Stamp(aChar,mData,0,mLength);
	aChar[mLength]=theChar;
	Free();
	mData = aChar;
	mLength = aLength;
	mRefs=new int;*mRefs=1;

	POPCURSOR;
}

void String::operator+=(char*theString)
{
	if(!theString)return;
	if(*theString==0)return;

	PUSHCURSOR;

	int aCharLength = Length(theString);
	int aLength = mLength + aCharLength;
	char *aChar = new char[aLength + 1];
	aChar[aLength]=0;
	Stamp(aChar,mData,0,mLength);
	Stamp(aChar,theString,mLength,aCharLength);
	Free();
	mData = aChar;
	mLength = aLength;
	mRefs=new int;*mRefs=1;

	POPCURSOR;
}

void String::operator+=(String &theString)
{
	char *aOther=theString.mData;
	if(!aOther)return;
	if(*aOther==0)return;

	PUSHCURSOR;

	int aLength = mLength + theString.mLength;
	char *aChar = new char[aLength + 1];
	aChar[aLength]=0;
	Stamp(aChar,mData,0,mLength);
	Stamp(aChar,theString.mData,mLength,theString.mLength);
	Free();
	mData = aChar;
	mLength = aLength;
	mRefs=new int;*mRefs=1;

	POPCURSOR;
}

void String::Size(int theSize)
{
	if(theSize == mLength)return;
	if(theSize <= 0)
	{
		Free();
	}
	else
	{
		char *aChar = new char[theSize+1];
		aChar[theSize]=0;
		int aCeil = _min(mLength, theSize);
		Stamp(aChar, mData, 0, aCeil);
		StampBlank(aChar,aCeil,theSize-aCeil);
		Free();
		mData=aChar;
		mRefs=new int;*mRefs=1;
		mLength=theSize;
	}
}

#define lower(c)(((c)>=65&&(c)<=90)?(c)+32:(c))
int String::CompareI(char *theString)
{
	if(!mData)
	{
		if(theString) return -1;
		return 0;
	}
	if(!theString) return 1;

	char *aChar = mData;
	while(lower(*aChar)==lower(*theString)&&*aChar)
	{
		aChar++;
		theString++;
	}
	if(*aChar>*theString)return 1;
	if(*aChar<*theString)return -1;
	return 0;
}

int String::CompareS(char *theString)
{
	if (!mData)
	{
		if(theString) return -1;
		return 0;
	}
	if(!theString)return 1;

	char *aChar = mData;
	while(*aChar==*theString&&*aChar)
	{
		aChar++;
		theString++;
	}
	if(*aChar>*theString)return 1;
	if(*aChar<*theString)return -1;
	return 0;
}

int String::CompareI(char *theString, int theCount)
{
	if (theCount==-1) theCount=::strlen(theString);
	if(theCount<=0)return 0;
	if(!mData){if(theString)return -1;return 0;}if(!theString)return 1;
	char *aChar = mData;
	while(lower(*aChar)==lower(*theString)&&*aChar&&theCount)
	{
		aChar++;
		theString++;
		theCount--;
	}
	if(theCount)
	{
		if(*aChar>*theString)return 1;
		if(*aChar<*theString)return -1;
	}
	return 0;
}

bool String::CompareFilename(String theString, int theCount)
{
    if (theCount<0) theCount=_max(theString.Len(),Len());
    
    for (int aCount=0;aCount<theCount;aCount++)
    {
        if (aCount>=theString.Len()) return false;
        if (aCount>=Len()) return false;
        
        if (theString[aCount]=='/') continue;
        if (theString[aCount]=='\\') continue;

        if (theString[aCount]!=mData[aCount]) return false;
    }
    return true;
}


int String::CompareS(char *theString, int theCount)
{
	if(theCount<=0)return 0;
	if(!mData){if(theString)return -1;return 0;}if(!theString)return 1;
	char *aChar = mData;
	while(*aChar==*theString&&*aChar&&theCount)
	{
		aChar++;
		theString++;
		theCount--;
	}
	if(theCount)
	{
		if(*aChar>*theString)return 1;
		if(*aChar<*theString)return -1;
	}
	return 0;
}

int String::FindI(char theChar, int thePosition)
{
	if(thePosition >= mLength || thePosition < 0)return -1;
	if(theChar >= 'A' && theChar <= 'Z')theChar+=32;
	char *aFinal = &mData[mLength];
	char *aChar = &mData[thePosition];
	while(aChar < aFinal)
	{
		char aCompare = *aChar;
		if(aCompare >= 'A' && aCompare <= 'Z')aCompare+=32;
		if(aCompare == theChar)return (int)(aChar - mData);
		aChar++;
	}
	return -1;
}


int String::Find(char theRangeLow, char theRangeHigh, int thePosition)
{
	if(thePosition >= mLength || thePosition < 0)return -1;
	char *aFinal = &mData[mLength];
	char *aChar = &mData[thePosition];
	while(aChar < aFinal)
	{
		if(*aChar >= theRangeLow && *aChar <= theRangeHigh)return (int)(aChar - mData);
		aChar++;
	}
	return -1;
}

int String::Find(char theChar, int thePosition)
{
	if(thePosition >= mLength || thePosition < 0)return -1;
	char *aFinal = &mData[mLength];
	char *aChar = &mData[thePosition];
	while(aChar < aFinal)
	{
		if(*aChar == theChar)return (int)(aChar - mData);
		aChar++;
	}
	return -1;
}

int String::Find(char *theString, int thePosition)
{
	if(!theString || thePosition >= mLength || thePosition < 0)return -1;
	char *aChar = &mData[thePosition];
	char *aFinish = &mData[mLength];
	while(aChar<aFinish)
	{
		if(*aChar == *theString)
		{
			char *aChar2 = aChar;
			char *aString2 = theString;
			while(aChar2 <= aFinish)
			{
				if(*aString2 == 0)return (int)(aChar - mData);
				if(*aString2 != *aChar2)break;
				aString2++;
				aChar2++;
			}
		}
		aChar++;
	}
	return -1;
}

int String::FindI(char *theString, int thePosition)
{
	if(!theString || thePosition >= mLength || thePosition < 0)return -1;
	char *aChar = &mData[thePosition];
	char *aFinish = &mData[mLength];
	char aC1, aC2;
	while(aChar<aFinish)
	{
		aC1 = *aChar;
		aC2 = *theString;
		if(aC1 >= 'A' && aC1 <= 'Z')aC1+=32;
		if(aC2 >= 'A' && aC2 <= 'Z')aC2+=32;
		if(aC1 == aC2)
		{
			char *aChar2 = aChar;
			char *aString2 = theString;
			while(aChar2 <= aFinish)
			{
				if(*aString2 == 0)return (int)(aChar - mData);
				aC1 = *aString2;
				aC2 = *aChar2;
				if(aC1 >= 'A' && aC1 <= 'Z')aC1+=32;
				if(aC2 >= 'A' && aC2 <= 'Z')aC2+=32;
				if(aC1 != aC2)break;
				aString2++;
				aChar2++;
			}
		}
		aChar++;
	}
	return -1;
}

longlong String::GetHash(char *theString, int theCount)//, int &theLength)
{
	if(!theString) return 0;
	longlong aReturn = 5381;
	char *aString=theString;
	while(int aChar = *aString++ != 0 && theCount--)aReturn=((aReturn<< 5)+aReturn)^aChar;
	//theLength = (int)(aString - theString);
	return aReturn;
}

longlong String::GetHashI(char *theString, int theCount)//, int &theLength)
{
	if(!theString)return 0;
	longlong aReturn = 5381;
	char *aString=theString;
	while(int aChar = *aString++ != 0 && theCount--)
	{
		if(aChar >= 'A' && aChar <= 'Z')aChar+=32;
		aReturn=((aReturn<< 5)+aReturn)^aChar;
	}
	return aReturn;
}

longlong String::GetHash(char *theString)
{
	if(!theString)return 0;
	longlong aReturn = 5381;
	char *aString=theString;
	while(int aChar = *aString++)aReturn=((aReturn<< 5)+aReturn)^aChar;
	return aReturn;
}

longlong String::GetHashI(char *theString)
{
	if(!theString)return 0;
	longlong aReturn = 5381;
	char *aString=theString;
	while(int aChar = *aString++)
	{
		if(aChar >= 'A' && aChar <= 'Z')aChar+=32;
		aReturn=((aReturn<< 5)+aReturn)^aChar;
	}
	return aReturn;
}

longlong String::GetHash()
{
	if(!mData)return 0;
	longlong aReturn = 5381;
	char *aString=mData;
	while(int aChar = *aString++)aReturn=((aReturn<< 5)+aReturn)^aChar;
	return aReturn;
}

longlong String::GetHashI()
{
	if(!mData)return 0;
	longlong aReturn = 5381;
	char *aString=mData;
	while(int aChar = *aString++)
	{
		if(aChar >= 'A' && aChar <= 'Z')aChar+=32;
		aReturn=((aReturn<< 5)+aReturn)^aChar;
	}
	return aReturn;
}

bool String::Replace(char theOriginal, char theNew, int startAt)
{
	if(mLength == 0)return false;
	int aFind = Find(theOriginal,startAt);
	if (aFind != -1)
	{
		PrepForInlineModification();
		char *aChar = &mData[aFind];
		while(*aChar)
		{
			if(*aChar == theOriginal)*aChar=theNew;
			aChar++;
		}
		return true;
	}
	return false;
}

bool String::Replace(Array<StringString>& theReplace, int startAt)
{
	if(mLength == 0)return false;
	if (startAt>=Len()) return false;

	Array<char> aNew;
	int aWrite=0;

	bool aNeedPrep=true;
	for (int aCount=startAt;aCount<Len();aCount++)
	{
		char* aPtr=mData+aCount;
		bool aDidit=false;
		foreach(aR,theReplace)
		{
			if (::strncmp(aPtr,aR[0].c(),aR[0].Len())==0)
			{
				if (aNeedPrep) 
				{
					aNew.GuaranteeSize(Len()*10);
					memcpy(aNew.mArray,mData,aCount);
					aWrite=aCount;

					aNeedPrep=false;
				}

				for (int aW=0;aW<aR[1].Len();aW++) aNew[aWrite++]=aR[1][aW];
				aCount+=aR[0].Len()-1;

				aDidit=true;
			}
		}

		if (!aDidit && !aNeedPrep)
		{
			aNew[aWrite++]=*aPtr;
		}
	}

	if (aNew.Size())
	{
		aNew[aWrite++]=0;
		strcpy(aNew.mArray);
	}


	return false;
}


bool CompI(char* theSource, char* theOther, int theLen) {for (int aCheck=0;aCheck<theLen;aCheck++) {if (lower(theSource[aCheck])!=lower(theOther[aCheck]) || theSource[aCheck]==0 || theOther[aCheck]==0) return false;}return true;}
bool Comp(char* theSource, char* theOther, int theLen) {for (int aCheck=0;aCheck<theLen;aCheck++) {if ((theSource[aCheck])!=(theOther[aCheck]) || theSource[aCheck]==0 || theOther[aCheck]==0) return false;}return true;}

bool String::Replace(char *theOriginal, char *theNew, int startAt)
{
	if(mLength == 0 || !theOriginal || !theNew) return false;
	if (theOriginal==theNew) return false;
	bool aResult=false;

	//*
	int aLengthO=Length(theOriginal);
	int aLengthN=Length(theNew);

	Array<char> aNew;aNew.GuaranteeSize(Len()*4);
	int aRead=0;
	int aWrite=0;

	for (int aCount=0;aCount<startAt;aCount++) {aNew[aWrite++]=mData[aRead++];}

	while (aRead<Len())
	{
		if (Comp(mData+aRead,theOriginal,aLengthO))
		{
			if (aWrite>aNew.Size()-aLengthN) aNew.GuaranteeSize(aWrite*4);
			for (int aCopy=0;aCopy<aLengthN;aCopy++) aNew[aWrite++]=*(theNew+aCopy);
			aRead+=(aLengthO-1);
			aResult=true;
		}
		else aNew[aWrite++]=*(mData+aRead);
		aRead++;

	}
	if (aResult)
	{
		aNew[aWrite++]=0;
		strcpy(aNew.mArray);
	}
	/**/


	/*
	int aFind = Find(theOriginal,startAt);
	while (aFind != -1)
	{
		int aLengthO = Length(theOriginal);
		int aLengthN = Length(theNew);
		int aDiff = aLengthN - aLengthO;
		int aNewLength = mLength;
		ChrList aFindList;
		while(aFind != -1)
		{
			aFindList += &mData[aFind];
			aFind = Find(theOriginal, aFind+1);
			aNewLength += aDiff;
		}
		char *aChar = new char[aNewLength + 1];
		aChar[aNewLength] = 0;
		char *aNew = aChar;
		char *aOld = mData;
		while(char *aGet = aFindList.Get())
		{
			while(aOld < aGet)
			{
				*aNew=*aOld;
				aNew++;
				aOld++;
			}
			char *aCopy = theNew;
			while(*aCopy)
			{
				*aNew=*aCopy;
				aNew++;
				aCopy++;
			}
			aOld+=aLengthO;
		}
		char *aFinal = &mData[mLength];
		while(aOld<aFinal)
		{
			*aNew=*aOld;
			aNew++;
			aOld++;
		}
		Free();
		mData=aChar;
		mRefs=new int;*mRefs=1;
		mLength=aNewLength;

		aResult=true;
		aFind = Find(theOriginal,startAt);
	}
	/**/
	
	return aResult;
}

bool String::ReplaceI(char *theOriginal, char *theNew, int startAt)
{
	if(mLength == 0 || !theOriginal || !theNew) return false;
	if (theOriginal==theNew) return false;

	bool aResult=false;

	int aLengthO = Length(theOriginal);
	int aLengthN = Length(theNew);

	Array<char> aNew;aNew.GuaranteeSize(Len()*4);
	int aRead=0;
	int aWrite=0;

	while (aRead<Len())
	{
		if (CompI(mData+aRead,theOriginal,aLengthO))
		{
			if (aWrite>aNew.Size()-aLengthN) aNew.GuaranteeSize(aWrite*4);
			for (int aCopy=0;aCopy<aLengthN;aCopy++) aNew[aWrite++]=*(theNew+aCopy);
			aRead+=(aLengthO-1);
			aResult=true;
		}
		else aNew[aWrite++]=*(mData+aRead);
		aRead++;

	}
	if (aResult)
	{
		aNew[aWrite++]=0;
		strcpy(aNew.mArray);
	}
	return aResult;
}

void String::Uppercase()
{
	int aFind = Find('a','z');
	if(aFind != -1)
	{
		PrepForInlineModification();
		char *aChar = &mData[aFind];
		char *aFinish = &mData[mLength];
		while(aChar < aFinish)
		{
			if(*aChar >= 'a' && *aChar <= 'z')*aChar-=32;
			aChar++;
		}
	}
}

void String::Lowercase()
{
	int aFind = Find('A','Z');
	if(aFind != -1)
	{
		PrepForInlineModification();
		char *aChar = &mData[aFind];
		char *aFinish = &mData[mLength];
		while(aChar < aFinish)
		{
			if(*aChar >= 'A' && *aChar <= 'Z')*aChar+=32;
			aChar++;
		}
	}
}

void String::Capitalize(bool startLowercase)
{
	if (startLowercase) Lowercase();
	bool aUpper=true;
	for (int aCount=0;aCount<Len();aCount++)
	{
		if (mData[aCount]==' ' || mData[aCount]=='_') aUpper=true;
		else if (aUpper)
		{
			if (mData[aCount]>='a' && mData[aCount]<='z') mData[aCount]-=32;
			aUpper=false;
		}
	}
}


void String::ParseChar(char *theChar, flag theFlags)
{
	Free();
	if(int aLen = Length(theChar))
	{
		int aWidth;
		GET_WIDTH(theFlags,aWidth);
		if(aWidth < aLen)aWidth=aLen;
		char *aChar = new char[aWidth+1];aChar[aWidth]=0;
		char *aFinal = &aChar[aWidth];
		char *aHold = aChar;
		int aDiff = aWidth - aLen;
		if(!(theFlags & (FLEFT_JUSTIFY)))while(aDiff--)*aChar++ = ' ';
		aChar = GetStamp(aChar,theChar,0,aLen);
		while(aChar < aFinal)*aChar++ = ' ';
		mLength = aWidth;
		mData = aHold;
		mRefs = new int;*mRefs=1;
	}
}

void String::ParseLong(longlong theLong, flag theFlags)
{
	static char	*gLower = "0123456789abcdefghijklmnopqrstuvwxyz";
	static char	*gUpper = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	Free();
	char *aDigit;
	int aWidth, aPrecision, aRadix;
	GET_WIDTH(theFlags,aWidth);
	GET_FRAC(theFlags,aPrecision);
	GET_RADIX(theFlags, aRadix);
	if(aRadix < 2 || aRadix > 36)aRadix = 10;
	char *aRaw = new char[66];
	char *aChar = new char[66 + 66/3 + 2 + 1 + aWidth + aPrecision];char *aHold = aChar;
	if(theFlags & FLOWER_CASE)aDigit=gLower;
	else aDigit=gUpper;

	if(theFlags & FLEFT_JUSTIFY)theFlags &= ~FLEAD_ZEROS;
	char aFill = (theFlags & FLEAD_ZEROS)? '0' : ' ';
	char aSign = 0;
	if(theFlags & FSIGNED)
	{
		if(theLong < 0)
		{
			aSign = '-';
			theLong = -theLong;
			aWidth--;
		}
		else if(theFlags & FFORCE_SIGN)
		{
			aSign = '+';
			aWidth--;
		}
		else if(theFlags & FSPACE_FOR_PLUS)
		{
			aSign = ' ';
			aWidth--;
		}
	}

	if(theFlags & FSPECIAL)
	{
		if(aRadix == 16)aWidth -= 2;
		else if(aRadix == 8)aWidth--;
	}

	int aIndex = 0;
	if(!theLong)aRaw[aIndex++] = '0';
	else
	{
		while(theLong != 0)
		{
			aRaw[aIndex++] = aDigit[((unsigned int)theLong)% (unsigned)aRadix];
			theLong = ((unsigned int)theLong)/ (unsigned)aRadix;
		}
	}

	int aCommaCount=0;
	int aCommaOffset;
	bool aCommas = (theFlags & FCOMMAS) != 0;
	if(aCommas)
	{
		if(aIndex < 4 || aIndex < aPrecision || theFlags & FLEAD_ZEROS || aRadix != 10)
		{
			aCommas=false;
		}
		if(aIndex < 4 || aIndex < aPrecision || theFlags & FLEAD_ZEROS || aRadix != 10)
		{
			aCommas=false;
		}
		else
		{
			aCommaCount=aIndex / 3;
			aCommaOffset=aIndex % 3;
			if(aCommaOffset == 0)
			{
				aCommaOffset=3;
				aCommaCount--;
			}
		}

	}

	if(aIndex > aPrecision)aPrecision = aIndex;
	aWidth -= (aPrecision + aCommaCount);

	if(!(theFlags & (FLEAD_ZEROS | FLEFT_JUSTIFY)))while(aWidth-- > 0)*aChar++ = ' ';

	if(aSign)*aChar++ = aSign;

	if(theFlags & FSPECIAL)
	{
		if(aRadix == 8)
		{
			*aChar++ = '0';
		}
		else if(aRadix == 16)
		{
			*aChar++ = '0';
			*aChar++ = 'x';
		}
	}

	if(!(theFlags & FLEFT_JUSTIFY))while(aWidth-- > 0)*aChar++ = aFill;

	while(aIndex < aPrecision--)*aChar++ = '0';
	if(aCommas)
	{
		while(aIndex-- > 0)
		{
			*aChar++ = aRaw[aIndex];
			if(--aCommaOffset == 0 && aCommaCount-- > 0)
			{
				*aChar++ = ',';
				aCommaOffset=3;
			}
		}
	}
	else
	{
		while(aIndex-- > 0)*aChar++ = aRaw[aIndex];
	}
	while(aWidth-- > 0)*aChar++ = ' ';
	*aChar=0;

	_DeleteArray(aRaw);

	*this = aHold;
	_DeleteArray(aHold);
}

/*
// Most recent version in use...
void String::ParseDouble(double theDouble, flag theFlags)
{
	Free();
	int aWidth, aPrecision;
	GET_WIDTH(theFlags,aWidth);
	if(theFlags & FDECIMAL_PT)GET_FRAC(theFlags,aPrecision);
	else aPrecision=6;
	bool aAddDecimal = !(aPrecision == 0 && (theFlags & FDECIMAL_PT) && !(theFlags & FSPECIAL));
	if(aAddDecimal)aWidth--;
	if(theFlags & FLEFT_JUSTIFY)
	{
		theFlags &= ~FLEAD_ZEROS;
	}
	char aFill = (theFlags & FLEAD_ZEROS)? '0' : ' ';
	char aSign = 0;
	if(theDouble < 0)
	{
		aSign = '-';
		theDouble = -theDouble;
		aWidth--;
	}
	else if(theFlags & FFORCE_SIGN)
	{
		aSign = '+';
		aWidth--;
	}
	else if(theFlags & FSPACE_FOR_PLUS)
	{
		aSign = ' ';
		aWidth--;
	}
	
	
#define BUFF_SIZE (350 + 255)
	char *aChar = new char[BUFF_SIZE+aWidth+aPrecision];
	//char *aRaw = new char[BUFF_SIZE];
	char *aHold = aChar;
	int aDecimal, aSignPtr;
#ifdef WIN32
	char *aRaw = new char[BUFF_SIZE];
	_fcvt_s(aRaw,BUFF_SIZE,theDouble,aPrecision,&aDecimal,&aSignPtr);
#else
	char *aRaw = new char[BUFF_SIZE];
	::strcpy((char*)aRaw,fcvt(theDouble,aPrecision,&aDecimal,&aSignPtr));
#endif
	
	int aLen = Length(aRaw);
	if(aDecimal < 1)
	{
		char *aNewBuff = new char[BUFF_SIZE];
		char *aBufPtr = aNewBuff;
		while(aDecimal < 1 && aPrecision >= 0)
		{
			*aBufPtr='0';aBufPtr++;
			aDecimal++;
			aPrecision--;
		}
		aDecimal = 1;
		aBufPtr = GetStamp(aBufPtr, aRaw, 0, aLen);
		*aBufPtr = 0;
		_DeleteArray(aRaw);
		aRaw = aNewBuff;
		aLen = Length(aNewBuff);
	}
	int aCommaCount=0;
	int aCommaOffset;
	bool aCommas = (theFlags & FCOMMAS) != 0;
	if(aCommas)
	{
		if(aDecimal < 4 || theFlags & FLEAD_ZEROS)
		{
			aCommas=false;
		}
		else
		{
			aCommaCount=aDecimal / 3;
			aCommaOffset=aDecimal % 3;
			if(aCommaOffset == 0)
			{
				aCommaOffset=3;
				aCommaCount--;
			}
		}

	}
	aWidth -= (aLen + aCommaCount);
	if(!(theFlags & (FLEAD_ZEROS | FLEFT_JUSTIFY)))while(aWidth-- > 0)*aChar++ = ' ';
	if(aSign)*aChar++ = aSign;
	if(!(theFlags & FLEFT_JUSTIFY))while(aWidth-- > 0)*aChar++ = aFill;
	if(aCommas)
	{
		int aHoldCommaCount = aCommaCount;
		char* aHoldRaw = aRaw;
		char *aShelf = &aRaw[aDecimal];
		while(aRaw < aShelf)
		{
			*aChar++ = *aRaw++;
			if(--aCommaOffset == 0 && aCommaCount-- > 0)
			{
				*aChar++ = ',';
				aCommaOffset=3;
			}
		}
		aRaw=aHoldRaw;
		aCommaCount=aHoldCommaCount;
	}
	else
	{
		aChar = GetStamp(aChar,aRaw,&aRaw[aDecimal]);
	}
	if(aAddDecimal)*aChar++='.';
	aChar = GetStamp(aChar,&aRaw[aDecimal + aCommaCount],&aRaw[aLen]);
	while(aWidth-- > 0)*aChar++ = ' ';
	*aChar=0;
	*this = aHold;
	_DeleteArray(aRaw);
	_DeleteArray(aHold);
}
/**/

/*
void String::ParseDouble(double theDouble, flag theFlags)
{
	Free();

	int aWidth, aPrecision;
	GET_WIDTH(theFlags,aWidth);
	if(theFlags & FDECIMAL_PT)GET_FRAC(theFlags,aPrecision);
	else aPrecision=6;

	bool aSign=false;
	int aSignPlus=0;
	if(theDouble<0)
	{
		aSign=true;
		theDouble=-theDouble;
		aSignPlus=1;
	}
	int aWholeNumber=(int)theDouble;
	double aFraction=theDouble-(float)aWholeNumber;
	int aWholeDigits=0;

	int aMultiply=10;
	int aHold=aPrecision;
	while(aHold>0)
	{
		aMultiply*=10;
		aHold--;
	}
	aFraction *= (float)aMultiply;
	int aFractionNumber = (int)aFraction;
	int aLastDigit = ((int)aFractionNumber) % 10;
	aFractionNumber/=10;

	//.567[8] = .568
	if(aLastDigit>=5)aFractionNumber++;

	//.9999999[9] = 1
	if(aFractionNumber >= (aMultiply / 10))
	{
		aFractionNumber=0;
		aWholeNumber++;
	}

	aHold=aWholeNumber;
	while(aHold)
	{
		aWholeDigits++;
		aHold /= 10;
	}
	if(aWholeDigits==0) aWholeDigits=1;
	if (aFractionNumber==0 && theFlags&FSPECIAL) aPrecision=0;

	mLength=aWholeDigits + 1 + aPrecision + aSignPlus;
	if (aFractionNumber==0 && theFlags&FSPECIAL) mLength--;
	//if (aFractionNumber==0) mLength--;

	mData = new char[mLength+1];
	mRefs = new int;
	*mRefs=1;

	if (aSignPlus) mData[0]='-';
	for(int i=aWholeDigits-1;i>=0;i--)
	{
		mData[i+aSignPlus]=((char)(aWholeNumber%10))+'0';
		aWholeNumber/=10;
	}

	if (mLength-1>aWholeDigits)
	{
		mData[aWholeDigits+aSignPlus]='.';
		for(int i=(mLength-1);i>aWholeDigits+aSignPlus;i--)
		{
			mData[i]=((char)(aFractionNumber%10))+'0';
			aFractionNumber/=10;
		}
	}
	mData[mLength]=strlen();
}
/**/

//* //Nick's newer version
void String::ParseDouble(double theDouble, flag theFlags)
{
	Free();

	int aWidth, aPrecision;
	//GET_WIDTH(theFlags,aWidth);
	if(theFlags & FDECIMAL_PT)GET_FRAC(theFlags,aPrecision);
	else aPrecision=6;

	bool aSign=false;
	if(theDouble<0)
	{
		aSign=true;
		theDouble=-theDouble;
	}
	int aWholeNumber=(int)theDouble;
	double aFraction=theDouble-(float)aWholeNumber;
	int aWholeDigits=0;

	int aMultiply=10;
	int aHold=aPrecision;
	while(aHold>0)
	{
		aMultiply*=10;
		aHold--;
	}
	aFraction *= (float)aMultiply;
	int aFractionNumber = (int)aFraction;
	int aLastDigit = ((int)aFractionNumber) % 10;
	aFractionNumber/=10;

	//.567[8] = .568
	if(aLastDigit>=5)aFractionNumber++;

	//.9999999[9] = 1
	if(aFractionNumber >= (aMultiply / 10))
	{
		aFractionNumber=0;
		aWholeNumber++;
	}

	aHold=aWholeNumber;
	while(aHold)
	{
		aWholeDigits++;
		aHold /= 10;
	}
	if(aWholeDigits==0)aWholeDigits=1;
	if (aFractionNumber==0 && !(theFlags&FFORCE_DECIMAL)/* && theFlags&FCLEANFLOAT*/) aPrecision=-1;

	mLength=aWholeDigits + 1 + aPrecision;

	if(aSign)
	{
		mLength += 1;
	}

	mData = new char[mLength+1];
	mRefs = new int;
	*mRefs=1;

	if(aSign)
	{
		mData[0] = '-';
		for(int i=aWholeDigits-1;i>=0;i--)
		{
			mData[i+1]=((char)(aWholeNumber%10))+'0';
			aWholeNumber/=10;
		}

		aWholeDigits++;
		mData[aWholeDigits]='.';

		for(int i=mLength-1;i>aWholeDigits;i--)
		{
			mData[i]=((char)(aFractionNumber%10))+'0';
			aFractionNumber/=10;
		}
	}

	else
	{
		for(int i=aWholeDigits-1;i>=0;i--)
		{
			mData[i]=((char)(aWholeNumber%10))+'0';
			aWholeNumber/=10;
		}
		mData[aWholeDigits]='.';
		for(int i=mLength-1;i>aWholeDigits;i--)
		{
			mData[i]=((char)(aFractionNumber%10))+'0';
			aFractionNumber/=10;
		}
	}



	mData[mLength]=0;
}
/**/




void String::Format(char *theFormat, va_list &aArgs)
{
	Free();if(!theFormat)return;
	char *aSearch = theFormat;
	int aFind=-1;
	while(*aSearch && *aSearch != '%')aSearch++;
	if(*aSearch){aFind = (int)(aSearch - theFormat);aSearch++;}
	if(aFind != -1)
	{
		mRefs=new int;*mRefs=1;
		mLength = 0;
		char *aCopyStart = &theFormat[0];
		char *aCopyShelf = &theFormat[aFind];
		ChrList aStart;
		ChrList aShelf;
		StringList aStrings;
		int aPrevious = 0;
		while(aFind != -1)
		{
			int aSpecWid;
			String *aNewString;
			if(*(aCopyShelf + 1) == '%')
			{
				aSpecWid = 1;
				aCopyShelf++;
				aSearch++;
				aNewString=0;
			}
			else
			{
				aNewString = ProcessSpecifier(aCopyShelf,aSpecWid,aArgs);
			}
			char *aDest = &aCopyShelf[aSpecWid];
			mLength += (int)(aCopyShelf - aCopyStart);
			if(aNewString)mLength+=aNewString->mLength;
			aStart += aCopyStart;
			aShelf += aCopyShelf;
			aStrings += aNewString;
			aPrevious=aFind;
			aFind=-1;
			while(*aSearch && *aSearch != '%')aSearch++;
			if(*aSearch){aFind = (int)(aSearch - theFormat);aSearch++;}
			aCopyStart = aDest;
			aCopyShelf = &theFormat[aFind];
		}
		mLength += (int)(aSearch - aCopyStart);
		mData = new char[mLength + 1];
		mData[mLength] = 0;
		char *aStamp = mData;
		while(char *aCopyStart=aStart.Get())
		{
			aCopyShelf=aShelf.Get();
			aStamp = GetStamp(aStamp, aCopyStart, aCopyShelf);
			String *aString = aStrings.Get();
			if(aString)aStamp=GetStamp(aStamp,aString->mData,0,aString->mLength);
			delete aString;
		}
		Stamp(aStamp,aCopyStart,aSearch);
	}
	else
	{
		mLength = (int)(aSearch - theFormat);
		mData = new char[mLength+1];mData[mLength]=0;
		Stamp(mData,theFormat,0,mLength);
		mRefs=new int;*mRefs=1;
	}
}

String *String::ProcessSpecifier(char *thePosition, int &theLength, va_list &aArgs)
{
	char *aStart = thePosition;
	String *aString = new String();
	if(*thePosition == '%')
	{
		flag aFlag = FDEFAULT;
RELOOP:
		thePosition++;

		if(*thePosition == '0' && (*(thePosition-1)!='.'))
		{
			aFlag |= FLEAD_ZEROS;
			goto RELOOP;
		}
		else if(*thePosition == ' ')
		{
			aFlag |= FSPACE_FOR_PLUS;
			goto RELOOP;
		}
		else if(*thePosition == 'u')
		{
			aFlag &= (~FSIGNED);
			goto RELOOP;
		}
		else if(*thePosition == '*')
		{
			int aVal = va_arg(aArgs, int);
			if(aFlag & FDECIMAL_PT){SET_FRAC(aFlag,aVal);}
			else {SET_WIDTH(aFlag,aVal);}
			goto RELOOP;
		}
		else if(*thePosition == '-')
		{
			aFlag |= FLEFT_JUSTIFY;
			goto RELOOP;
		}
		else if(*thePosition == '+')
		{
			aFlag |= FFORCE_SIGN;
			goto RELOOP;
		}
		else if(*thePosition == '^')
		{
			aFlag &= (~FSIGNED);
			goto RELOOP;
		}
		else if(*thePosition == '.')
		{
			aFlag |= FDECIMAL_PT;
			if(*(thePosition+1) == '-') // %.-1f ... forces decimal to appear
			{
				aFlag |= FFORCE_DECIMAL;
				thePosition++;
			}
			goto RELOOP;
		}
		else if(*thePosition == '#')
		{
			aFlag |= FSPECIAL;
			goto RELOOP;
		}
		else if(*thePosition == ',')
		{
			aFlag |= FCOMMAS;
			goto RELOOP;
		}
		else if(*thePosition >= '0' && *thePosition <= '9')
		{
			char *aNum = thePosition;int aSum = 0;
			while (*aNum >= '0' && *aNum <= '9')
			{
				aSum = aSum * 10 + *aNum - '0';
				aNum++;
			}
			thePosition += aNum - thePosition - 1;
			if(aFlag & FDECIMAL_PT){SET_FRAC(aFlag,aSum);}
			else {SET_WIDTH(aFlag,aSum);}
			goto RELOOP;
		}
		else if(*thePosition == 's' || *thePosition == 'S')
		{
			aString->ParseChar(va_arg(aArgs, char*),aFlag);
		}
		else if(*thePosition == 'c' || *thePosition == 'C')
		{
         // SVERO : USING char here crashes GCC/Android
         //*aString = (char)(va_arg(aArgs, char));
			*aString = (char)(va_arg(aArgs, int));
		}
		else if(*thePosition == 'f' || *thePosition == 'g' || *thePosition == 'e' || *thePosition == 'F' || *thePosition == 'G' || *thePosition == 'E')
		{
			flag aF=aFlag;
			if (*thePosition=='F') aF|=FSPECIAL;
			aString->ParseDouble(va_arg(aArgs, double), aF);
		}
		else if(*thePosition == 'd' || *thePosition == 'i' || *thePosition == 'I' || *thePosition == 'D')
		{
			aString->ParseLong(va_arg(aArgs, int), aFlag);
		}
		else if(*thePosition == 'l' || *thePosition == 'L')
		{
			if (*(thePosition+1)=='l')
			{
				thePosition++;

				static char aLongLong[100];
				longlong aValue=va_arg(aArgs, longlong);
#ifdef _WIN32
				::sprintf_s(aLongLong,100,"%llu",aValue);
#else
				::sprintf(aLongLong,"%llu",aValue);
#endif
				*aString=aLongLong;
			}
			else aString->ParseLong(va_arg(aArgs, longlong), aFlag);
		}
		else if(*thePosition == 'x')
		{
			aFlag |= FLOWER_CASE;
			goto PARSE_HEX;
		}
		else if(*thePosition == 'X')
		{
PARSE_HEX:
			SET_RADIX(aFlag, 16);
			aString->ParseLong(va_arg(aArgs, int), aFlag);
		}
		else if(*thePosition == 'P' || *thePosition == 'p')
		{
			SET_RADIX(aFlag, 16);
			aFlag &= (~FSIGNED);
			aString->ParseLong(va_arg(aArgs, int), aFlag);
		}
		else if(*thePosition == 'b' || *thePosition == 'B')
		{
			SET_RADIX(aFlag, 2);
			aString->ParseLong(va_arg(aArgs, int), aFlag);
		}
		else if(*thePosition == 'o' || *thePosition == 'O')
		{
			SET_RADIX(aFlag, 8);
			aString->ParseLong(va_arg(aArgs, int), aFlag);
		}
		else
		{
			thePosition--;
		}
	}
	theLength = (int)(thePosition - aStart + 1);
	return aString;
}

String IntToString(int theInt, bool addCommas)
{
	String aReturn;
	String::flag aFlag = FDEFAULT;
	if(addCommas)aFlag |= FCOMMAS;
	aReturn.ParseInt(theInt,aFlag);
	return aReturn;
}


String FloatToString(float theFloat, bool addCommas, bool cutNondecimal)
{
	String aReturn;
	String::flag aFlag = FDEFAULT;
	if(addCommas) aFlag|=FCOMMAS;
	if (cutNondecimal) aFlag|=FSPECIAL;
	aReturn.ParseDouble(theFloat,aFlag);
	return aReturn;
}

String SuperscriptNumber(int theNumber)
{
	String aNumber;
	aNumber.ParseInt(theNumber,0);
	char aFinal=aNumber[aNumber.mLength-1];
	if (aNumber.mLength>=2) if (aNumber[aNumber.mLength-2]=='1')aFinal='0';
	switch (aFinal)
	{
	case '0':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		aNumber+="th";
		break;
	case '1':
		aNumber+="st";
		break;
	case '2':
		aNumber+="nd";
		break;
	case '3':
		aNumber+="rd";
		break;
	}
	return aNumber;
}

float String::PercentSimilar(String &theString)
{
	if(mLength == 0)
	{
		if(theString.mLength == 0)return 1;
		else return 0;
	}
	if(theString.mLength == 0)return 0;
	String aLCSS = GetLongestCommonSubsequence(theString);
	return (float)aLCSS.Length() / (float)_max(mLength,theString.mLength);
}

bool String::ToBool()
{
	if (strcmpi("true")) return true;
	if (strcmpi("yes")) return true;
	if (strcmpi("on")) return true;
	if (strcmpi("1")) return true;
	if (strcmpi("random")) 
	{
		if ((rand()%2)==0) return true;
		return false;
	}
	return false;
}

bool String::ParseXMLTag(String& theTag, Array<String>* theParams)
{
	theTag="";
	if (theParams) theParams->Reset();

	RemoveLeadingSpaces();
	RemoveTrailingSpaces();

	theTag=GetCodeToken(' ');
	String aParam=GetNextCodeToken(' ');
	while (aParam.Len())
	{
		if (aParam.LastChar()=='/') aParam=aParam.GetSegment(0,aParam.Len()-1);
		if (aParam.Len()) theParams->Add(aParam);
		aParam=GetNextCodeToken(' ');
	}

	if (theTag.LastChar()=='/') theTag=theTag.GetSegment(0,theTag.Len()-1);

	if (LastChar()=='/') return true;
	return false;
}


bool String::ParseXML(String& theTag, String& theData,  Array<String>* theParams)
{
	if (theParams) theParams->Reset();

	if (mParseXMLCursor>=Length())
	{
		theTag="";
		theData="";
		return false;
	}
    
	//
	// Start by looking for '<'
	//
	String aTag="";
	theTag="";
	theData="";
	for (;;)
	{
		char aC=mData[mParseXMLCursor++];
		if (aC==0) return false;

		if (aC=='<') break;
	}
    

	//
	// Now we're in the tag... grab everything up until the '>'
	//

	for (;;)
	{
		char aC=mData[mParseXMLCursor++];
		if (aC==0) {theTag="";theData="";return false;}
		if (aC=='>') break;
		aTag+=aC;
	}

	//
	// If it's self-closing, just return...
	//
	if (aTag.ParseXMLTag(theTag,theParams)) return true;




	//
	// Catch this:
	// <My_Tag/> ... which means its its own closing tag
	//
	

	//
	// Okay, gather data...
	//
	int aInTag=0;
	bool aAllowLF=false;
	for (;;)
	{
		char aC=mData[mParseXMLCursor++];

		if (aC==0) break;
		if (aC=='<') 
		{
			char aCC=mData[mParseXMLCursor];
			if (aCC=='/') 
			{
				if (aInTag==0)
				{
					String aEnd=GetSegment(mParseXMLCursor+1,theTag.Len());
					//if (mData[mParseXMLCursor+2]=='>') aEnd=theTag;	// So if we say </>, that just ends current one.
					if (aEnd==theTag)
					{
						mParseXMLCursor+=1;
						mParseXMLCursor+=theTag.Len();
						if (mData[mParseXMLCursor]=='>') 
						{
							int aSegmentLen=theData.Len()-1;
							for (int aCount=aSegmentLen;aCount>=0;aCount--)
							{
								if (theData[aCount]>=32 || theData[aCount]<0) break;
								aSegmentLen--;
							}
							if (aSegmentLen<theData.Len()-1) 
							theData=theData.GetSegment(0,aSegmentLen+1);
							return true;
						}
					}
				}
				aInTag--;
				if (aInTag<0)
				{
					theTag="";
					theData="";

					gOut.Out("PARSEXML Error: Too many closing tags found.");

					return false;
				}
			}
			else aInTag++;
		}
		if (aC!=13 && (aC!=10 || aAllowLF)) 
		{
			theData+=aC;
			aAllowLF=true;
		}
	}

	//
	// End tag wasn't found
	//

	gOut.Out("PARSEXML Error: End tag (%s) not found.",theTag.c());
	theTag="";
	theData="";
	return false;
}

String String::GetXML(String theTag)
{
	ParseXML();
	String aTag,aData;

	while (ParseXML(aTag,aData)) if (aTag==theTag) return aData;
	return "";
}

bool String::ParseRML(String& theTag, String& theData,  Array<String>* theParams)
{
	if (theParams) theParams->Reset();

	if (mParseXMLCursor>=Length())
	{
		theTag="";
		theData="";
		return false;
	}

	//
	// Start by looking for '<'
	//
	String aTag="";
	theTag="";
	theData="";
	for (;;)
	{
		char aC=mData[mParseXMLCursor++];
		if (aC==0) return false;

		if (aC=='[') break;
	}


	//
	// Now we're in the tag... grab everything up until the '>'
	//

	for (;;)
	{
		char aC=mData[mParseXMLCursor++];
		if (aC==0) {theTag="";theData="";return false;}
		if (aC==']') break;
		aTag+=aC;
	}

	//
	// If it's self-closing, just return...
	//
	if (aTag.ParseXMLTag(theTag,theParams)) return true;




	//
	// Catch this:
	// [My_Tag/] ... which means its its own closing tag
	//


	//
	// Okay, gather data...
	//
	int aInTag=0;
	bool aAllowLF=false;
	for (;;)
	{
		char aC=mData[mParseXMLCursor++];

		if (aC==0) break;
		if (aC=='[') 
		{
			char aCC=mData[mParseXMLCursor];
			if (aCC=='/') 
			{
				if (aInTag==0)
				{
					String aEnd=GetSegment(mParseXMLCursor+1,theTag.Len());
					if (aEnd==theTag)
					{
						mParseXMLCursor+=1;
						mParseXMLCursor+=theTag.Len();
						if (mData[mParseXMLCursor]==']') 
						{
							int aSegmentLen=theData.Len()-1;
							for (int aCount=aSegmentLen;aCount>=0;aCount--)
							{
								if (theData[aCount]>=32 || theData[aCount]<0) break;
								aSegmentLen--;
							}
							if (aSegmentLen<theData.Len()-1) 
								theData=theData.GetSegment(0,aSegmentLen+1);
							return true;
						}
					}
				}
				aInTag--;
				if (aInTag<0)
				{
					theTag="";
					theData="";

					gOut.Out("PARSEXML Error: Too many closing tags found.");

					return false;
				}
			}
			else aInTag++;
		}
		if (aC!=13 && (aC!=10 || aAllowLF)) 
		{
			theData+=aC;
			aAllowLF=true;
		}
	}

	//
	// End tag wasn't found
	//

	gOut.Out("PARSERML Error: End tag (%s) not found.",theTag.c());
	theTag="";
	theData="";
	return false;
}

String String::GetRML(String theTag)
{
	ParseRML();
	String aTag,aData;

	while (ParseRML(aTag,aData)) if (aTag==theTag) return aData;
	return "";
}

bool String::ParseRMLTag(String& theTag, Array<String>* theParams)
{
	theTag="";
	if (theParams) theParams->Reset();

	RemoveLeadingSpaces();
	RemoveTrailingSpaces();

	theTag=GetCodeToken(' ');
	String aParam=GetNextCodeToken(' ');
	while (aParam.Len())
	{
		if (aParam.LastChar()=='/') aParam=aParam.GetSegment(0,aParam.Len()-1);
		if (aParam.Len()) theParams->Add(aParam);
		aParam=GetNextCodeToken(' ');
	}

	if (theTag.LastChar()=='/') theTag=theTag.GetSegment(0,theTag.Len()-1);

	if (LastChar()=='/') return true;
	return false;
}


String String::RemoveTail(char theTailCharacter)
{
	if (Len()==0) return "";
//	PrepForInlineModification();
	for (int aCount=Len();aCount>=0;aCount--) if (mData[aCount]==theTailCharacter) return GetSegment(0,aCount);
	return *this;
}

String String::GetTail(char theTailCharacter)
{
	if (Len()==0) return "";
	for (int aCount=Len();aCount>=0;aCount--) if (mData[aCount]==theTailCharacter) return GetSegment(aCount+1,Len()+1);
	return "";
	//return *this;
}

int String::FindTail(char theTailCharacter)
{
	if (Len()==0) return -1;
	for (int aCount=Len();aCount>=0;aCount--) if (mData[aCount]==theTailCharacter) return aCount;
	return -1;
}


String String::GetNotTail(char theTailCharacter)
{
	if (Len()==0) return "";
	for (int aCount=Len();aCount>=0;aCount--) if (mData[aCount]==theTailCharacter) return GetSegment(0,aCount);
	return *this;
}

bool String::GetWordAtPosition(int thePosition,int& theStart, int& theLen)
{
	theStart=theLen=0;

	if (thePosition<0) return false;
	if (thePosition>=Len()) return false;
	if (!isalnum((unsigned char)mData[thePosition])) return false;

	int aLow,aHigh;
	aLow=0;
	aHigh=Len()-1;

	if (thePosition>aLow) for (aLow=thePosition-1;aLow>0;aLow--) {if (!isalnum((unsigned char)mData[aLow])) {aLow++;break;}}
	if (thePosition<aHigh) for (aHigh=thePosition;aHigh<Len();aHigh++) {if (!isalnum((unsigned char)mData[aHigh])) {break;}}

	theStart=aLow;
	theLen=(aHigh-aLow);
	return true;
}

bool String::GetWordStartingAtPosition(int thePosition,int& theStart, int& theLen)
{
	theStart=theLen=0;

	if (thePosition<0) return false;
	if (thePosition>=Len()) return false;
	if (!isalnum((unsigned char)mData[thePosition])) return false;

	int aLow,aHigh;
	aLow=thePosition;
	aHigh=Len()-1;

	if (thePosition<aHigh) for (aHigh=thePosition;aHigh<Len();aHigh++) 
	{
		if (!isalnum((unsigned char)mData[aHigh])) {break;}
	}

	theStart=aLow;
	theLen=(aHigh-aLow);
	return true;
}

/*
void Dictionary::Add(String theString)
{
	int aList=_min((unsigned char)theString[0],129);
	mList[aList]+=theString;
}

bool Dictionary::Contains(String theString)
{
	int aList=_min((unsigned char)theString[0],129);
	for (int aCount=0;aCount<mList[aList].Size();aCount++) if (mList[aList][aCount]==theString) return true;
	return false;
}
*/


String String::Evaluate()
{
	float aResult=gMath.Eval(c());
	//
	// Need to parse forward.  Ignore anything within parenthesis.
	// Break out whenever you hit an operator.
	//
	return String(aResult);
}

char String::StepXMLChar()
{
	char aC=mData[mParseXMLCursor++];
	if (aC=='<' && mData[mParseXMLCursor]=='-' && mData[mParseXMLCursor+1]=='-')
	{
		while (mParseXMLCursor<Len())
		{
			aC=mData[mParseXMLCursor++];
			if (aC=='-' && mData[mParseXMLCursor]=='-' && mData[mParseXMLCursor+1]=='>')
			{
				mParseXMLCursor+=2;
				aC=mData[mParseXMLCursor++];
				break;
			}
			aC=0;
		}
	}
	return aC;
}

bool String::StepXML(String& theResult)
{
	theResult="";
	if (mParseXMLCursor>=Len()) return false;
	char aC=StepXMLChar();
	if (aC=='<')
	{
		theResult+=aC;
		while (mParseXMLCursor<Len())
		{
			aC=StepXMLChar();
			theResult+=aC;
			if (aC=='>') break;
		}
	}
	else 
	{
		theResult+=aC;
		while (mParseXMLCursor<Len())
		{
			aC=StepXMLChar();
			if (aC=='<') 
			{
				mParseXMLCursor--;
				break;
			}
			theResult+=aC;
		}
	}
	return true;
}

char String::StepRMLChar()
{
	char aC=mData[mParseXMLCursor++];
	if (aC=='[' && mData[mParseXMLCursor]=='-' && mData[mParseXMLCursor+1]=='-')
	{
		while (mParseXMLCursor<Len())
		{
			aC=mData[mParseXMLCursor++];
			if (aC=='-' && mData[mParseXMLCursor]=='-' && mData[mParseXMLCursor+1]==']')
			{
				mParseXMLCursor+=2;
				aC=mData[mParseXMLCursor++];
				break;
			}
			aC=0;
		}
	}
	return aC;
}

bool String::StepRML(String& theResult)
{
	theResult="";
	if (mParseXMLCursor>=Len()) return false;
	char aC=StepRMLChar();
	if (aC=='[')
	{
		theResult+=aC;
		while (mParseXMLCursor<Len())
		{
			aC=StepRMLChar();
			theResult+=aC;
			if (aC==']') break;
		}
	}
	else 
	{
		theResult+=aC;
		while (mParseXMLCursor<Len())
		{
			aC=StepRMLChar();
			if (aC=='[') 
			{
				mParseXMLCursor--;
				break;
			}
			theResult+=aC;
		}
	}
	return true;
}


void String::RemoveExtraSpaces()
{
	RemoveLeadingSpaces();
	RemoveTrailingSpaces();

	int aStep=1;
	while (aStep<Len())
	{
		if (mData[aStep]<=' ')
		{
			if (mData[aStep-1]==mData[aStep])
			{
				Delete(aStep,1);
				continue;
			}
		}
		aStep++;
	}
}

void String::RemoveRedundantFilenameCharacters()
{
	RemoveLeadingSpaces();
	RemoveTrailingSpaces();

	int aStep=1;
	while (aStep<Len())
	{
		if (mData[aStep]=='\\' || mData[aStep]=='/')	// Look for two slashes in a row
		{
			if (mData[aStep-1]=='\\' || mData[aStep-1]=='/')
			{
				if (aStep<2 || mData[aStep-2]!=':') // But :// is allowed because that's a "location"
				{
					Delete(aStep,1);
					continue;
				}
			}
		}
		aStep++;
	}

}


static inline bool IsBase64(unsigned char c) {return (isalnum((unsigned char)c) || (c == '+') || (c == '/'));}
static String gBase64Chars="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

void String::Base64Encode()
{
	strcpy(GetBase64Encode());
}

String String::GetBase64Encode()
{
	int aLen=Len();
	String aResult;
	int aI=0;
	int aJ=0;
	unsigned char aCharArray3[3];
	unsigned char aCharArray4[4];
	char* aBytesToEncode=mData;

	while (aLen--) 
	{
		aCharArray3[aI++]=*(aBytesToEncode++);
		if (aI==3) 
		{
			aCharArray4[0]=(aCharArray3[0]&0xfc)>>2;
			aCharArray4[1]=((aCharArray3[0]&0x03)<<4)+((aCharArray3[1]&0xf0)>>4);
			aCharArray4[2]=((aCharArray3[1]&0x0f)<<2)+((aCharArray3[2]&0xc0)>>6);
			aCharArray4[3]=aCharArray3[2]&0x3f;

			for(aI=0;(aI<4);aI++) aResult+=gBase64Chars[aCharArray4[aI]];
			aI=0;
		}
	}

	if (aI)
	{
		for (aJ=aI;aJ<3;aJ++) aCharArray3[aJ]='\0';

		aCharArray4[0]=(aCharArray3[0]&0xfc)>>2;
		aCharArray4[1]=((aCharArray3[0]&0x03)<<4)+((aCharArray3[1]&0xf0)>>4);
		aCharArray4[2]=((aCharArray3[1]&0x0f)<<2)+((aCharArray3[2]&0xc0)>>6);
		aCharArray4[3]=aCharArray3[2]&0x3f;

		for (aJ=0;(aJ<aI+1);aJ++) aResult+=gBase64Chars[aCharArray4[aJ]];
		while((aI++<3)) aResult+='=';
	}
	return aResult;
}

void String::Base64Decode()
{
	strcpy(GetBase64Decode());
}

String String::GetBase64Decode()
{
	int aLen=Len();
	int aI=0;
	int aJ=0;
	int aIn=0;
	unsigned char aCharArray4[4];
	unsigned char aCharArray3[3];


	char* aEncodedString=mData;

	String aResult;

	while (aLen-- && (aEncodedString[aIn]!='=') && IsBase64(aEncodedString[aIn]))
	{
		aCharArray4[aI++]=aEncodedString[aIn];aIn++;
		if (aI==4)
		{
			for (aI=0;aI<4;aI++) aCharArray4[aI]=gBase64Chars.Find(aCharArray4[aI]);

			aCharArray3[0]=(aCharArray4[0]<<2)+((aCharArray4[1]&0x30)>>4);
			aCharArray3[1]=((aCharArray4[1]&0xf)<<4)+((aCharArray4[2]&0x3c)>>2);
			aCharArray3[2]=((aCharArray4[2]&0x3)<<6)+aCharArray4[3];

			for (aI=0;(aI<3);aI++) aResult+=aCharArray3[aI];
			aI=0;
		}
	}

	if (aI)
	{
		for (aJ=aI;aJ<4;aJ++) aCharArray4[aJ]=0;
		for (aJ=0;aJ<4;aJ++) aCharArray4[aJ]=gBase64Chars.Find(aCharArray4[aJ]);

		aCharArray3[0]=(aCharArray4[0]<<2)+((aCharArray4[1]&0x30)>>4);
		aCharArray3[1]=((aCharArray4[1]&0xf)<<4)+((aCharArray4[2]&0x3c)>>2);
		aCharArray3[2]=((aCharArray4[2]&0x3)<<6)+aCharArray4[3];

		for (aJ=0;(aJ<aI-1);aJ++) aResult+=aCharArray3[aJ];
	}


	return aResult;
}

void String::ParseAllURLValues(Array<String>& theResult)
{
	String aQuery=GetSegmentAfter('?');
	if (aQuery.Len()==0) aQuery=*this;

	theResult.Reset();
	String aToken=aQuery.GetToken('&');
	while (!aToken.IsNull())
	{
		theResult.Add(aToken);
		aToken=aQuery.GetNextToken('&');
	}
}

String String::ParseURLValue(String theParamName)
{
	String aResult;
	String aQuery=GetSegmentAfter('?');
	if (aQuery.Len()==0) aQuery=*this;

	String aToken=aQuery.GetToken('&');
	while (!aToken.IsNull())
	{
		String aParam=aToken.GetSegmentBefore('=');
		if (aParam==theParamName)
		{
			aResult=aToken.GetSegmentAfter('=');
			break;
		}
		aToken=aQuery.GetNextToken('&');
	}
	return aResult;
}

char* String::c_copy()
{
	if (!mData) strcpy("");
	char *aNewData=new char[Len()+1];
#ifdef _WIN32
	::strcpy_s(aNewData,Len()+1,mData);
#else
	::strcpy(aNewData,mData);
#endif
	return aNewData;
}

String String::GetQuotes()
{
	int aStart=0;
	int aEnd=Len()-1;

	for (int aCount=0;aCount<aEnd;aCount++) if (GetChar(aCount)=='\"') {aStart=aCount+1;break;}
	for (int aCount=aEnd;aCount>=aStart;aCount--) if (GetChar(aCount)=='\"') {aEnd=aCount;break;}

	return GetSegment(aStart,aEnd-aStart);
}

#ifdef UTF8_PRIVATE_USE
int StringUTF8::GetUTF8()
{
	if (Len()==0) return 0;

	int aChar=0;
	int aSZ=0;

	do {
		aChar<<=6;
		aChar+=(unsigned char)(*HelpUTF8()->mUTF8Cursor);
		if (aChar==0) return 0;
		HelpUTF8()->mUTF8Cursor++;
		aSZ++;
	} 
	while (*HelpUTF8()->mUTF8Cursor && !ISUTF(*HelpUTF8()->mUTF8Cursor));
	aChar-=gOffsetsFromUTF8[aSZ-1];

	/*
	if (aChar<0 || aChar>255)
	{
		gOut.Out("Fucked: %d",aChar);
		gOut.Out("Fucked String: [%s]",c());
		gOut.Out("Cursor: %d",HelpUTF8()->mUTF8Cursor-mData);
		gOut.Out("Yeah");
	}
	*/
	return aChar;
}

int StringUTF8::PeekUTF8()
{
	if (Len()==0) return 0;

	int aChar=0;
	int aSZ=0;

	char* aCursor=HelpUTF8()->mUTF8Cursor;

	do 
	{
		aChar<<=6;
		aChar+=(unsigned char)(*aCursor);
		if (aChar==0) return 0;

		aCursor++;
		aSZ++;
	} 
	while (*aCursor && !ISUTF(*aCursor));
	aChar-=gOffsetsFromUTF8[aSZ-1];
	return aChar;
}

int StringUTF8::GetCharAt(int thePos)
{
	if (Len()==0) return 0;

	void* aHold=mExtraData;
	mExtraData=new UTF8Data;

	int aCount=0;
	StartUTF8();
	for (;;)
	{
		if (aCount==thePos) break;
		if (GetUTF8()==0) break;
		aCount++;
	}
	int aResult=GetUTF8();

	delete (UTF8Data*)mExtraData;
	mExtraData=aHold;

	return aResult;

}


int StringUTF8::PeekUTF8Len(int aPos)
{
	if (Len()==0) return 0;

	int aSZ=0;
	char* aCursor=mData+aPos;
	do 
	{
		aCursor++;
		aSZ++;
	} 
	while (*aCursor && !ISUTF(*aCursor));
	return aSZ;
}

int StringUTF8::PosUTF8(int thePos)
{
	if (Len()==0) return 0;

	void* aHold=mExtraData;
	mExtraData=new UTF8Data;

	int aCount=0;
	StartUTF8();
	for (;;)
	{
		if (aCount==thePos) break;
		if (GetUTF8()==0) break;
		aCount++;
	}

	int aTell=TellUTF8();

	delete (UTF8Data*)mExtraData;
	mExtraData=aHold;

	return aTell;
}

void StringUTF8::RewindUTF8()
{
	if (Len()==0) return;

	(void)(ISUTF(*(--HelpUTF8()->mUTF8Cursor)) || ISUTF(*(--HelpUTF8()->mUTF8Cursor)) || ISUTF(*(--HelpUTF8()->mUTF8Cursor)) || --HelpUTF8()->mUTF8Cursor);
	if (HelpUTF8()->mUTF8Cursor<mData) HelpUTF8()->mUTF8Cursor=mData;
}

void StringUTF8::InsertUTF8(int theChar, int thePosition)
{
	int aPos=PosUTF8(thePosition);

	char aInsert[5];
	aInsert[0]=0;

	if (theChar<0x80) 
	{
		aInsert[0]=(char)theChar;
		aInsert[1]=0;
	}
	else if (theChar<0x800) 
	{
		aInsert[0]=(char)((theChar>>6)|0xC0);
		aInsert[1]=(((theChar & 0x3F)|0x80));
		aInsert[2]=0;
	}
	else if (theChar<0x10000) 
	{
		aInsert[0]=((char)((theChar>>12)|0xE0));
		aInsert[1]=((char)(((theChar>>6) & 0x3F)|0x80));
		aInsert[2]=((char)((theChar & 0x3F)|0x80));
		aInsert[3]=0;
	}
	else if (theChar<0x110000) 
	{
		aInsert[0]=((char)((theChar>>18)|0xF0));
		aInsert[1]=((char)(((theChar>>12) & 0x3F)|0x80));
		aInsert[2]=((char)(((theChar>>6) & 0x3F)|0x80));
		aInsert[3]=((char)((theChar & 0x3F)|0x80));
		aInsert[4]=0;
	}
	String::Insert(aInsert,aPos);
}


void StringUTF8::DeleteUTF8(int thePosition)
{
	if (Len()==0) return;

	int aPos=PosUTF8(thePosition);
	int aLen=PeekUTF8Len(aPos);
	String::Delete(aPos,aLen);
}

int StringUTF8::LenUTF8()
{
	if (Len()==0) return 0;
	void* aHold=mExtraData;
	mExtraData=new UTF8Data;

	int aCount=0;
	StartUTF8();
	for (;;) {if (GetUTF8()==0) break;aCount++;}

	delete (UTF8Data*)mExtraData;
	mExtraData=aHold;
	return aCount;
}
#endif

String FormatDate(int theMon, int theDay, int theYear, int theWeekday)
{
	int aMonth=_clamp(0,theMon,13);
	int aWeekday=-1;
	if (theWeekday>=0) aWeekday=_clamp(0,theWeekday,7);

	char* aWeekdayString[]={"Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday","???"};
	char* aMonthString[]={"???","January","February","March","April","May","June","July","August","September","October","November","December","???"};

	if (aWeekday!=-1) return Sprintf("%s, %s %s, %.4d",_TT(aWeekdayString[aWeekday]),_TT(aMonthString[aMonth]),SuperscriptNumber(theDay).c(),theYear);
	else return Sprintf("%s %s, %.4d",_TT(aMonthString[aMonth]),SuperscriptNumber(theDay).c(),theYear);
}

String FormatTime(int theHour, int theMin)
{
	int aHoldHour=theHour;

	if (theHour==0) theHour=12;
	if (theHour>12) theHour-=12;

	return Sprintf("%d:%.2d%s",theHour,theMin,(aHoldHour>=12)?"pm":"am");
}

void String::Tokenize(char theSeperator,Array<String>& theTokens)
{
	theTokens.Reset();
	String aToken=GetToken(theSeperator);
	while (aToken.Len())
	{
		theTokens+=aToken;
		aToken=GetNextToken(theSeperator);
	}
}

void String::TokenizeCode(char theSeperator,Array<String>& theTokens)
{
	theTokens.Reset();
	String aToken=GetCodeToken(theSeperator);
	while (aToken.Len())
	{
		aToken.RemoveLeadingSpaces();
		aToken.RemoveTrailingSpaces();
		aToken.RemoveExtraSpaces();
		aToken.Replace('\t',' ');
		aToken.RemoveNonPrintableCharacters();
		theTokens+=aToken;
		aToken=GetNextCodeToken(theSeperator);
	}
}

bool String::GetCodeBlock(String& theName, String& theContents)
{
	mCursor=mData;
	mMoreTokens=true;
	return GetNextCodeBlock(theName,theContents);
}

bool String::GetNextCodeBlock(String& theName, String& theContents)
{
	//
	// Find starting code block...
	//
	theName="";
	theContents="";

	for (;;)
	{
		char aC=*mCursor;
		if (aC==0) {return false;}
		if (aC=='{') break;
		mCursor++;
	}
	char* aHold=mCursor;
	//
	// Go backwards to get the name...
	//
	while (mCursor>=mData)
	{
		char aC=*mCursor;
		if (aC=='}' || aC==';') {mCursor++;break;}
		mCursor--;
	}

	if (mCursor<mData) mCursor=mData;

	int aLen=aHold-mCursor;
	if (aLen<=0) {return false;}

	String aCodeBlock;
	String aCodeName=GetSegment(mCursor-mData,aLen);
	aCodeName.RemoveLeadingSpaces();

	mCursor=aHold;
	mCursor++;
	char* aStartCode=mCursor;
	int aBraceCount=1;
	for (;;)
	{
		char aC=*mCursor;
		if (aC==0) {return false;}
		if (aC=='{') aBraceCount++;
		if (aC=='}') if (--aBraceCount<=0) break;
		mCursor++;
	}

	aLen=mCursor-aStartCode;
	mCursor++;
	//if (aLen<=0) {return false;}

	aCodeBlock=GetSegment(aStartCode-mData,aLen);

	aCodeName.RemoveTrailingSpaces();
	aCodeName.RemoveExtraSpaces();
	aCodeName.Replace('\t',' ');
	aCodeName.RemoveNonPrintableCharacters();

	aCodeBlock.RemoveLeadingSpaces();
	aCodeBlock.RemoveTrailingSpaces();
	aCodeBlock.RemoveExtraSpaces();
	aCodeBlock.Replace('\t',' ');
	aCodeBlock.RemoveCodeComments(); // Need to remove comments... (because we're getting rid of nonprintables so we won't have any char10's to detect the end of comments)
	aCodeBlock.RemoveNonPrintableCharacters();

	theContents=aCodeBlock;
	theName=aCodeName;

	return true;
}

void String::RemoveCodeComments()
{
	if (!mData) return;
	Array<char> aNew;aNew.GuaranteeSize(Len());
	int aWrite=0;
	bool aDidIt=false;

	char* aPtr=mData;
	char aInComment=0;
	while (*aPtr!=0)
	{
		if (*aPtr=='/' && *(aPtr+1)=='/') {aDidIt=true;aInComment=true;}
		if (*aPtr==10) aInComment=false;

		if (!aInComment) aNew[aWrite++]=*aPtr;
		aPtr++;
	}

	if (aDidIt) 
	{
		aNew[aWrite++]=0;
		strcpy(aNew.mArray);
	}

	
}

bool String::GetCodeBlocks(Array<StringString>& theResult)
{
	String aGet[2];
	bool aMore=GetCodeBlock(aGet[0],aGet[1]);
	while (aMore)
	{
		theResult+=StringString(aGet[0],aGet[1]);
		aMore=GetNextCodeBlock(aGet[0],aGet[1]);
	}
	return theResult.Size()>0;
}

String String::GetCodeBlockByName(String theTitle)
{
	int aFindTitle=FindI(theTitle);
	if (aFindTitle==-1) return "";

	int aPos=aFindTitle;aPos+=theTitle.Len();
	for (;;)
	{
		if (mData[aPos]=='{') break;
		if (mData[aPos]>32 || mData[aPos]==0) return "";
		aPos++;
	}

	int aStartPos=aPos+1;
	int aBraceCount=0;

	for (;;)
	{
		if (mData[aPos]=='}') if (--aBraceCount==0)
		{
			return GetSegment(aStartPos,(aPos-1)-aStartPos);
		}
		if (mData[aPos]=='{') aBraceCount++;
		if (mData[aPos]==0) return "";
		aPos++;
	}
	return "";
}

String String::GetParam(String theParam, char theSeperator)
{
	String aToken=GetToken(theSeperator);
	while (aToken.Len())
	{
		String aCommand=aToken.GetSegmentBefore('=');
		if (aCommand==theParam) return aToken.GetSegmentAfter('=');
		aToken=GetNextToken(';');
	}
	return "";
}

Rect String::ToRect()
{
	// This has become stupidly complicated, because I chose to use a "-" as my rect delimeter, as in:
	// 5,5-10,10
	// Except what about this, John? -5,5-10,10
	// We are essentially looking for a - that follows a number or a space.

	bool aAfterNumberOrSpace=false;
	int aBreak=0;
	for (int aCount=0;aCount<Len();aCount++)
	{
		char aC=GetChar(aCount);
		if (aC=='-' && aAfterNumberOrSpace) {aBreak=aCount;break;}
		if (isdigit(aC) || aC==' ') aAfterNumberOrSpace=true;
		else aAfterNumberOrSpace=false;
	}

	Point aP1=GetSegment(0,aBreak).ToPoint();
	Point aP2=GetSegment(aBreak+1,Len()).ToPoint();
	return Rect(aP1.mX,aP1.mY,aP2.mX,aP2.mY);
}

void String::TokenizeXML(Array<StringString>& theResult)
{
	ParseXML();
	String aTag,aValue;
	while (ParseXML(aTag,aValue))
	{
		int aInsert=theResult.Size();
		theResult[aInsert][0]=aTag;
		theResult[aInsert][1]=aValue;
	}
}
