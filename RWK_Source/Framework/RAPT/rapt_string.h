#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "rapt_point.h"
#include "rapt_color.h"
#include "util_core.h"
#include "rapt_rect.h"

#include <string>

#define PUSHCURSOR int aHoldCursor=mCursor-mData
#define POPCURSOR if (aHoldCursor>=0) mCursor=mData+aHoldCursor
//#define PUSHCURSOR 
//#define POPCURSOR

struct StringString;
class String
{
public:
	//
	//Stuff used by Format()
	//
	typedef unsigned int flag;
	//
	//Bits of flags used for parsing via Format()
	//
#define FLEAD_ZEROS		0x1		//1		//
#define FSIGNED			0x2		//2		//
#define FFORCE_SIGN		0x4		//3		//
#define FSPACE_FOR_PLUS	0x8		//4		//
#define FLEFT_JUSTIFY	0x10	//5		//
#define FSPECIAL		0x20	//6		//
#define FLOWER_CASE		0x40	//7		// Force lower case.
#define FDECIMAL_PT		0x80	//8		// Percision
#define FCOMMAS			0x100	//9		// 123,456,789 - Only works without leading zeros on decimal numbers.
#define FFORCE_DECIMAL	0x200	//9		// Force decimal points
#define FRADIX			0x00007E00//(0x200+0x400+0x800+0x1000+0x2000+0x4000)////(0x200+0x400+0x800+0x1000+0x2000+0x4000)//10-15
#define FFIELD_WIDTH	0x007F8000//(0x8000+0x10000+0x20000+0x40000+0x80000+0x100000+0x200000+0x400000)//16 -> 23 (8)
#define FFRAC_NUMBERS	0x7F800000//(0x800000+0x1000000+0x2000000+0x4000000+0x8000000+0x10000000+0x20000000+0x40000000)//24 -> 31 (8)
	//
	//Flag manipulating macros.
	//
#define SET_RADIX(theFlag,theRadix)theFlag &= (~FRADIX); theFlag |= ((theRadix & 63) << 9)
#define GET_RADIX(theFlag,theVar)theVar = ((theFlag & FRADIX) >> 9);
#define SET_WIDTH(theFlag,theNumber)theFlag &= (~FFIELD_WIDTH);theFlag |= ((theNumber & 255) << 15)
#define GET_WIDTH(theFlag,theVar)theVar = ((theFlag & FFIELD_WIDTH) >> 15)
#define SET_FRAC(theFlag,theNumber)theFlag &= (~FFRAC_NUMBERS);theFlag |= ((theNumber & 255) << 23)
#define GET_FRAC(theFlag,theVar)theVar = ((theFlag & FFRAC_NUMBERS) >> 23)
	//
	//Default flag - radix = 10, signed = true
	//
#define FDEFAULT		(0x1400 + FSIGNED)
	//
	//End stuff used by Format()
	//

#define DEFAULT_CASE_SENSITIVITY false


	String(void){Constructor();}
	String(const String& theString){Constructor();*this=theString;}
	String(char *theString){Constructor();*this=theString;}
	String(std::string &theString){Constructor();*this=theString.c_str();}
	String(std::string *theString){Constructor();*this=theString->c_str();}
	String(char *theString, int theCount){Constructor();Write(theString,0,theCount);}
	String(const char *theString){Constructor();*this=theString;}
	String(int theInt){Constructor();ParseInt(theInt);}
	String(unsigned int theInt){Constructor();ParseInt(theInt);}
	String(char theChar){Constructor();mData=new char[2];mLength=1;mRefs=new int;*mRefs=1;mData[0]=theChar;mData[1]=0;}
	String(longlong thelong){Constructor();ParseLong(thelong);}
	String(float theFloat){Constructor();ParseFloat(theFloat);}
	String(double theDouble){Constructor();ParseDouble(theDouble);}
	String(void *thePtr){Constructor();ParseAddress(thePtr);}
	String(bool theBool){Constructor();ParseBool(theBool);}
	virtual ~String(void) {Free();mExtraData=NULL;}
	
	//
	//Raw character data.
	//Don't alter this unless you know what you're doing!
	//Strings may be sharing char arrays when you set them
	//equal to each other, so try to 
	//
	char					*mData;
	char					*mCursor;
	int						*mRefs;
	int						mLength;
	void*					mExtraData;

	inline bool				IsCursorAtEnd() {return (mCursor-mData)>=Len();}

	inline void				Init(){mData=0;mLength=0;mRefs=0;mCursor=0;}

	//
	//Don't use Realize() if you set mData to another String's mData... It'll crash...
	//
	void					Realize(){if(!mData)Init();else{mLength=Length(mData);mRefs=new int;*mRefs=1;}}

	//
	//Token processing...
	//
	String					GetToken(char theSeperator=0, bool wantTokenInString=false);
	String					GetNextToken(char theSeperator=0, bool wantTokenInString=false);
	String					GetBracedToken(char thePositionBrace, char theEndBrace, bool wantTokenInString=false);				
	String					GetNextBracedToken(char thePositionBrace, char theEndBrace, bool wantTokenInString=false);
	inline bool				IsMoreTokens() {return mMoreTokens;}
	inline void				AddToken(String theToken, String theSeperator="|") {if (Len()>0) *this+=theSeperator;*this+=theToken;}
	inline void				AppendToken(String theToken, String theSeperator="|") {AddToken(theToken,theSeperator);}
	bool					mMoreTokens;
	void					Tokenize(char theSeperator,Array<String>& theTokens);
	void					TokenizeCode(char theSeperator,Array<String>& theTokens);

	//
	// Gets tokens with special provisions for code.  For instance, it won't break inside a string.
	//
	String					GetCodeToken(char theSeperator=0, bool wantTokenInString=false);
	String					GetNextCodeToken(char theSeperator=0, bool wantTokenInString=false);

	//
	// Helpers for other kinds of parsing...
	//
	String					GetCodeBlockByName(String theSectionName);	// Fetches content for something like "global {myData;}" ... ask for global, get "mydata"
	bool					GetCodeBlock(String& theName, String& theContents);
	bool					GetNextCodeBlock(String& theName, String& theContents);
	bool					GetCodeBlocks(Array<StringString>& theResult);

	void					RemoveCodeComments();
		


	//
	//Calculate the length of a character array.
	//[Must be terminated with a null space]
	//
	static int				Length(char *theString)
    {
		//
		// Did you crash here by sending a char* into a SprintF statement, like so:
		// char* aData="Whatever";
		// Sprintf("Muh Data: %s",aData);
		// Something in 2019 hates that... but if you change it to const char* aData, it works.
		//
        if(!theString) return 0;
        char* aPtr=theString;
        while(*aPtr!=0) aPtr++;
        return (int)(aPtr-theString);
    }

	//
	//Convert a String that represents another data type to the desired data type...
	//
	int						ToInt() {if(Length())return (int)atoi(mData);return 0;}
	longlong				ToLongLong() 
	{
		char* aIn=mData;
		longlong retval;
		retval = 0;
		for (; *aIn; aIn++) retval = 10*retval + (*aIn-'0');
		return retval;
	}
	char					ToChar() {if(Length())return (char)atoi(mData);return 0;}
	float					ToFloat() {if(Length())return (float)atof(mData);return 0;}
	double					ToDouble() {if(Length())return (double)atof(mData);return 0;}
	void					ToPoint(float &theX, float &theY);
	inline Point			ToPoint() {Point aPoint;ToPoint(aPoint.mX,aPoint.mY);return aPoint;}
	void					ToPointEval(float &theX, float &theY);
	inline Point			ToPointEval() {Point aPoint;ToPointEval(aPoint.mX,aPoint.mY);return aPoint;}
	bool					ToBool();
	//inline Rect				ToRect() {Point aP1=GetStringBefore("- ").ToPoint();Point aP2=GetStringAfter("- ").ToPoint();return Rect(aP1.mX,aP1.mY,aP2.mX,aP2.mY);}
	Rect					ToRect();

	inline String			GetNumber(){return GetNumber(0);}
	inline String			GetNextNumber(){return GetNumber((int)(mCursor-mData));}
	String					GetNumber(int thePosition);

	longlong				CastAsLongLong(bool fromTheEnd=false);

	//
	//Resize the String. Extra spaces
	//will be filled in with ' '.
	//
	void					Size(int theSize);
	inline void				EnsureSize(int theSize){if(theSize > mLength)Size(theSize);}
	inline void				Truncate(int theNewSize){if(theNewSize < mLength)Size(theNewSize);}

	void					Set(char theChar, int theSize) {EnsureSize(theSize);SetAllChars('-');}

	//
	//Get the length of the String nice and quick.
	//
	inline int				Length(){return mLength;}
	inline int				GetLength(){return mLength;}

	//
	//Honestly, these shouldn't ever be used outside of the String function...
	//If you set up the char array manually, just use Realize()
	//
	inline int				GetLen(){return Length(mData);}
	inline int				Len(){return Length(mData);}
	inline int				strlen(){return Length(mData);}

	//
	//Some advanced operations for removing all characters from a string,
	//or removing everything but some characters from a string.
	//

	void					Filter(char *theAllowed);
	void					Filter(String &theAllowed){Filter(theAllowed.mData);}

	void					Remove(char *theDisallowed);
	void					Remove(String &theDisallowed){Remove(theDisallowed.mData);}

	inline void				RemoveCharacters(char *theDisallowed){Remove(theDisallowed);}
	inline void				RemoveNotCharacters(char *theAllowed){Filter(theAllowed);}

	bool					GetWordAtPosition(int thePosition,int& theStart, int& theLen);
	bool					GetWordStartingAtPosition(int thePosition,int& theStart, int& theLen);
	void					RemoveNonPrintableCharacters();
	inline void				ReplaceCRLF(char replaceWith=' ') {Replace(10,replaceWith);char aRemove[2]={13,0};RemoveCharacters(aRemove);}
	inline void				RemoveCRLF() {char aRemove[3]={13,10,0};RemoveCharacters(aRemove);}



	//
	//Returns true if the String is NULL or "".
	//
	bool					IsNull(){return mLength == 0;}

	//
	//{+/-}{.}0-9{.}0-9{.} Where one or none of the decimals exist...
	//
	bool					IsNumber();

	//
	//Resets the String to null. (Not "", null)
	//
	virtual void			Free(){if(mData){if((--*mRefs)==0){delete [] mData;mData=NULL;delete mRefs;mRefs=NULL;}}Init();}
	inline void				Clear(){Free();}
	inline void				Reset(){Free();}

	//
	//Get a subString of the String.
	//
	String					GetSubString(int thePosition, int theLength);
	String					GetSubString(int thePosition){return GetSubString(thePosition, mLength);}
	inline String			GetSegment(int thePosition, int theLength){return GetSubString(thePosition,theLength);}
	String					GetSegmentAfter(char afterThisCharacter) {for (int aCount=0;aCount<Len();aCount++) if (mData[aCount]==afterThisCharacter) return GetSubString(aCount+1,Len());return "";}
	String					GetSegmentAfter(String afterThisString) {int aPos=Find(afterThisString);if (aPos==-1) return mData;return GetSegment(aPos+afterThisString.Len(),Len());}
	inline String			GetStringAfter(char afterThisCharacter) {return GetSegmentAfter(afterThisCharacter);}
	inline String			GetStringAfter(String afterThisString) {return GetSegmentAfter(afterThisString);}
	String					GetSegmentBefore(char beforeThisCharacter) {for (int aCount=0;aCount<Len();aCount++) if (mData[aCount]==beforeThisCharacter) return GetSubString(0,aCount);return *this;}
	String					GetSegmentBefore(String beforeThisString) {int aPos=Find(beforeThisString);if (aPos==-1) return mData;return GetSegment(0,aPos);}
	inline String			GetStringBefore(char afterThisCharacter) {return GetSegmentBefore(afterThisCharacter);}
	inline String			GetStringBefore(String afterThisString) {return GetSegmentBefore(afterThisString);}


	//
	//Write over the String starting at the specified location...
	//"Hat".Write("Jam", 2) -> "HaJam"
	//"abcdefg".Write("123", 0) -> "123defg"
	//
	void					Write(char theChar, int thePosition=0);
	void					Write(char *theString, int thePosition=0);
	void					Write(String &theString, int thePosition=0);
	void					Write(char *theString, int thePosition, int theAmount);
	void					Write(String &theString, int thePosition, int theAmount);
	void					Write(char theChar, int theQuantity, int thePosition);

	inline void				strpad(char theChar, int theQuantity){Write(theChar,theQuantity);}

	inline void				strcpy(char *theString){*this=theString;}
	inline void				strcpy(const char *theString){*this=(char*)theString;}
	inline void				strcpy(String &theString){*this=theString;}
	inline void				strcpy(){Free();}
	inline void				strncpy(char *theString, int theLen){Free();Write(theString,0,theLen);}
	inline void				strncpy(String &theString, int theLen){Free();Write(theString,0,theLen);}

	//
	//Insert new text starting at the specified location...
	//"Hat".Insert("Jam", 2) -> "HaJamt"
	//"abcdefg".Insert("123", 0) -> "123abcdefg"
	//
	void					Insert(char theChar, int thePosition=0);
	void					Insert(char *theString, int thePosition=0);
	void					Insert(String &theString, int thePosition=0);
	void					Insert(char *theString, int thePosition, int theAmount);
	void					Insert(String &theString, int thePosition, int theAmount);
	void					Insert(char theChar, int theQuantity, int thePosition);

	String					GetLongestCommonSubsequence(String &theString);
	inline String			GetLongestCommonSubsequence(char *theString){String aStr=theString;return GetLongestCommonSubsequence(aStr);}

	float					PercentSimilar(String &theString);
	inline float			PercentSimilar(char *theString){String aStr=theString;return PercentSimilar(aStr);}

	//
	//Format the String...
	//
	void					Format(char *theFormat, va_list &aArgs);
	void					Format(char *theFormat, ...){va_list aArgs;va_start(aArgs, theFormat);Format(theFormat, aArgs);va_end(aArgs);}
	void					sprintf(char *theFormat, ...){va_list aArgs;va_start(aArgs, theFormat);Format(theFormat, aArgs);va_end(aArgs);}

	//
	//String representations of other data types...
	//
	void					ParseDouble(double theDouble, flag theFlags=FDEFAULT);
	inline void				ParseFloat(float theFloat, flag theFlags=FDEFAULT){ParseDouble(theFloat,theFlags);}
	inline void				ParseAddress(void *theAddr){ParseLong((longlong)theAddr, 0x52021);}
	inline void				ParseAddress(int theAddr){ParseLong((longlong)theAddr, 0x52021);}
	void					ParseChar(char *theChar, flag theFlags=FDEFAULT); 
	void					ParseBool(bool theBool){if(theBool)*this="TRUE";else *this="FALSE";}
	void					ParseLong(longlong theLong, flag theFlags=FDEFAULT);
	inline void				ParseInt(int theInt, flag theFlags=FDEFAULT){ParseLong(theInt,theFlags);}

	//
	//Eliminate "spaces" (ASCII characters less than 33) from the
	//beginning or end of the String...
	//
	void					RemoveLeadingSpaces();
	void					RemoveTrailingSpaces();
	void					RemoveExtraSpaces();

	//
	// Filename fixer... gets rid of redundant slashes and stuff like that...
	//
	void					RemoveRedundantFilenameCharacters();


	String					GetQuotes();	// Removes the first and last quotes from the string

	//
	//Goes through the String and replaces all cases
	//of one substring with different text...
	//
	bool					Replace(char *theOriginal, char *theNew, int startAt=0);
	bool					ReplaceI(char *theOriginal, char *theNew, int startAt=0);
	bool					Replace(char theOriginal, char theNew, int startAt=0);
	bool					Replace(Array<StringString>& theReplace, int startAt=0);

	//
	//Remove stuff from a string.
	//
	void					Delete(char *theString);
	void					Delete(int thePosition, int theLength);
	void					Delete(int thePosition);
	void					DeleteI(char *theString);

	

	//
	//Reverse the string? Why the hell not!
	//
	void					Reverse();

	//
	//For oldschool windows machines that need carriage return before line feeds.
	//
	inline void				AppendCRLF() {*this += "\r\n";}
	inline void				AppendCR() {*this += "\r";}
	inline void				AppendLF() {*this += "\n";}
	inline void				Append(char* theChar, int theCount) {for (int aCount=0;aCount<theCount;aCount++) *this+=theChar;}
	inline void				AppendCSV(char* theText, char* theSeperator=",") {if (Len()>0) *this+=theSeperator;*this+=theText;}

	//
	//Set character at an index to something.
	//
	void					SetChar(int theIndex, char theChar){if(theIndex >= 0 && theIndex < mLength){mData[theIndex]=theChar;if(theChar == 0)mLength = theIndex;}}
	void					SetAllChars(char theChar) {for (int aCount=0;aCount<mLength;aCount++) mData[aCount]=theChar;}

	//
	//Find the first location in the String where
	//the specified data is stored. Returns -1
	//if the specified data does not exist in the String.
	//
	int						Find(char *theString, int thePosition);
	inline int				Find(String &theString, int thePosition){return Find(theString.mData, thePosition);}
	inline int				Find(const char *theString, int thePosition){return Find((char*)theString, thePosition);}
	int						Find(char theRangeLow, char theRangeHigh, int thePosition);
	int						Find(char theChar, int thePosition);
	inline int				Find(char *theString){return Find(theString,0);}
	inline int				Find(const char *theString){return Find((char*)theString,0);}
	inline int				Find(String &theString){return Find(theString.mData,0);}
	inline int				Find(char theRangeLow, char theRangeHigh){return Find(theRangeLow,theRangeHigh,0);}
	inline int				Find(char theChar){return Find(theChar,0);}

	int						FindI(char *theString, int thePosition);
	inline int				FindI(String &theString, int thePosition){return FindI(theString.mData, thePosition);}
	inline int				FindI(const char *theString, int thePosition){return FindI((char*)theString, thePosition);}
	int						FindI(char theChar, int thePosition);
	inline int				FindI(char *theString){return FindI(theString, 0);}
	inline int				FindI(String &theString){return FindI(theString.mData, 0);}
	inline int				FindI(const char *theString){return FindI((char*)theString, 0);}
	inline int				FindI(char theChar){return FindI(theChar, 0);}


	inline int				strchr(char theChar){return Find(theChar);}

	//
	//Uhm... Finds return -1 when they don't find that which they seek...
	//
	const static int		mNotFound=-1;

	//
	//Does the String contain some of this stuff? Find out today!
	//
	inline bool				Contains(char theRangeLow, char theRangeHigh){return Find(theRangeLow,theRangeHigh)!=-1;}
	inline bool				Contains(char theChar){return Find(theChar)!=-1;}
	inline bool				Contains(char *theString){return Find(theString,0) != -1;}
	inline bool				Contains(String &theString){return Find(theString.mData,0) != -1;}

	inline bool				ContainsI(char *theString){return FindI(theString,0) != -1;}
	inline bool				ContainsI(String &theString){return FindI(theString.mData,0) != -1;}


	//
	//Generates a number between 0 and theTableSize.
	//Each String produces a unique hash value,
	//but different Strings may produce the same
	//hash value.
	//
	longlong				GetHash();
	longlong				GetHashI();

	static longlong			GetHash(char *theString);
	static longlong			GetHashI(char *theString);

	static longlong			GetHash(char *theString, int theCount);
	static longlong			GetHashI(char *theString, int theCount);

	//
	//Sets the case of all letters.
	//
	void					Uppercase();
	void					Lowercase();
	void					Capitalize(bool startLowercase=true);


	//
	//Get a copy of the String with a certain case.
	//
	String					ToUpper(){String aReturn = *this;aReturn.Uppercase();return aReturn;}
	String					ToLower(){String aReturn = *this;aReturn.Lowercase();return aReturn;}
	String					ToCapitalized() {String aReturn=*this;aReturn.Capitalize();return aReturn;}

	//
	//Get a copy of the character array.
	//*This must be deleted manually!*
	//
	char					*ToCharArray(){if(mLength < 1)return 0;char *aChar = new char[mLength+1];aChar[mLength]=0;Stamp(aChar,mData,0,mLength);return aChar;}



	//
	//Only effects operators and Compare().
	//
	bool					mCaseSensitive;
	inline void				CaseSensitive(){mCaseSensitive=true;}
	inline void				CaseInensitive(){mCaseSensitive=false;}
	inline void				i(){mCaseSensitive=false;}
	inline void				s(){mCaseSensitive=true;}
	inline void				SetCaseSensitive(bool theState){mCaseSensitive=theState;}


	//
	//Compares this String with another String.
	//-1 = less than
	// 0 = equal
	// 1 = greater than
	//
	inline int				Compare(char *theString){if(mCaseSensitive)return CompareS(theString);else return CompareI(theString);}
	inline int				Compare(const char *theString){if(mCaseSensitive)return CompareS(theString);else return CompareI(theString);}
	inline int				Compare(String &theString){if(mCaseSensitive && theString.mCaseSensitive)return CompareS(theString);else return CompareI(theString);}
	inline int				Compare(char *theString, int theCount){if(mCaseSensitive)return CompareS(theString,theCount);else return CompareI(theString,theCount);}
	inline int				Compare(const char *theString, int theCount){if(mCaseSensitive)return CompareS(theString,theCount);else return CompareI(theString,theCount);}
	inline int				Compare(String &theString, int theCount){if(mCaseSensitive && theString.mCaseSensitive)return CompareS(theString,theCount);else return CompareI(theString,theCount);}

	//Compare force case insensitive
	int						CompareI(char *theString);
	inline int				CompareI(const char *theString){return CompareI((char*)theString);}
	inline int				CompareI(String &theString){return CompareI(theString.mData);}
	int						CompareI(char *theString, int theCount);
	inline int				CompareI(const char *theString, int theCount){return CompareI((char*)theString,theCount);}
	inline int				CompareI(String &theString, int theCount){return CompareI(theString.mData,theCount);}
    
    //
    // Special compares for filenames, for Mac/PC compatiblity...
    //
    bool                    CompareFilename(String theString, int theCount=-1);

	inline bool				strcmpi(char *theString){return CompareI(theString)==0;}
	inline bool				strcmpi(const char *theString){return CompareI(theString)==0;}
	inline bool				strcmpi(String &theString){return CompareI(theString.mData)==0;}
	inline bool				strnicmp(char *theString, int theLength=-1){return CompareI(theString, theLength)==0;}
	inline bool 			strnicmp(String &theString, int theLength=-1){return CompareI(theString.mData, theLength)==0;}

	//Compare force case sensitive
	int						CompareS(char *theString);
	inline int				CompareS(const char *theString){return CompareS((char*)theString);}
	inline int				CompareS(String &theString){return CompareS(theString.mData);}
	int						CompareS(char *theString, int theCount);
	inline int				CompareS(const char *theString, int theCount){return CompareS((char*)theString,theCount);}
	inline int				CompareS(String &theString, int theCount){return CompareS(theString.mData,theCount);}

	inline bool				strcmp(char *theString){return CompareS(theString)==0;}
	inline bool				strcmp(const char *theString){return CompareS(theString)==0;}
	inline bool				strcmp(String &theString){return CompareS(theString.mData)==0;}
	inline bool 			strncmp(char *theString, int theLength){return Compare(theString, theLength)==0;}
	inline bool				strncmp(String &theString, int theLength){return Compare(theString.mData, theLength)==0;}

	//
	// Extra compares, for convenience...
	//
	inline bool				StartsWith(String &theString) {return Compare(theString,theString.Len())==0;}
	inline bool				StartsWith(char *theString) {return Compare(theString,::strlen(theString))==0;}
	inline bool				StartsWith(const char *theString) {return Compare(theString,::strlen(theString))==0;}

	inline bool				EndsWith(String &theString) {return (GetSegment(Len()-theString.Len(),Len())==theString);}
	inline bool				EndsWith(char* theString) {return (GetSegment(Len()-::strlen(theString),Len())==theString);}
	inline bool				EndsWith(const char* theString) {return (GetSegment(Len()-::strlen(theString),Len())==theString);}


	//
	//Connect two Strings together!
	//Use +=, + instead unless you need a certain length!
	//

	inline void				Concatenate(char *theString){*this += theString;}
	inline void				Concatenate(const char *theString){*this += theString;}
	inline void				Concatenate(char theChar){*this += theChar;}
	inline void				Concatenate(String &theString){*this += theString;}
	void					Concatenate(char *theString, int theNum);
	void					Concatenate(String &theString, int theNum);
	inline void				Concatenate(const char *theString, int theNum){Concatenate((char*)theString,theNum);}

	inline void				strncat(char *theString, int theNum){Concatenate(theString, theNum);}
	inline void				strncat(const char *theString, int theNum){Concatenate(theString, theNum);}
	inline void				strncat(String &theString, int theNum){Concatenate(theString, theNum);}
	inline void				strcat(char *theString){*this += theString;}
	inline void				strcat(String &theString){*this += theString;}
	inline void				strncat(const char *theString){*this += theString;}
	inline void				strcat(char theChar){*this += theChar;}

	//
	//Fetch raw character data.
	//Don't alter this unless you know what you're doing!
	//
	inline char				*c() {if (!mData) strcpy("");return mData;}
	inline char				*ToString() {if (!mData) strcpy("");return mData;}
	char					*c_copy();


	//operator				char*();
	//operator				const char*(){return mData;}
	//


	inline bool				operator!=(char*theString){return Compare(theString)!=0;}
	inline bool				operator!=(const char*theString){return Compare(theString)!=0;}
	inline bool				operator!=(String &theString){return Compare(theString.mData)!=0;}

	inline bool				operator==(char*theString){return Compare(theString)==0;}
	inline bool				operator==(const char*theString){return Compare(theString)==0;}
	inline bool				operator==(String &theString){return Compare(theString.mData)==0;}

	inline bool				operator>(char*theString){return Compare(theString)>0;}
	inline bool				operator>(const char*theString){return Compare(theString)>0;}
	inline bool				operator>(String &theString){return Compare(theString.mData)>0;}

	inline bool				operator>=(char*theString){return Compare(theString)>=0;}
	inline bool				operator>=(const char*theString){return Compare(theString)>=0;}
	inline bool				operator>=(String &theString){return Compare(theString.mData)>=0;}

	inline bool				operator<(char*theString){return Compare(theString)<0;}
	inline bool				operator<(const char*theString){return Compare(theString)<0;}
	// PDN - 01/07/2012
	//
	// Had to hack constness of the call below.  This entire class really should be const-aware, because
	// interfacing with outside libraries (like the STL and possibly onther third party libraries.)
	//
	// In this specific case, I needed to make use of the std::map class and this requires that the
	// objects stored in the map are compared using operator<(). It also requires that the operator<()
	// strictly follows C++ conventions (including the constness of the operator<() member function itself.)
	//
	// The code below was hacked to make the function const. However, since the Compare() function was not
	// const, I had to cast the 'this' pointer to non-const in order to call it. This is a big no-no, but it
	// was the smallest change I could make (without going down the chain of Compare() functions and making
	// them const as well) to enable the functionality I needed.
	//
	// Side note: This is a shame, really, because constness is one of the cool aspects of C++. Dealing
	// with const-related issues can be a pain in the arse if you don't understand it fully, but once you
	// do, it is easy to work with and can help you a lot. It also helps enforce clean design patterns.
	inline bool				operator<(const String &theString) const {return const_cast<String*>(this)->Compare(theString.mData)<0;}

	inline bool				operator<=(char*theString){return Compare(theString)<=0;}
	inline bool				operator<=(const char*theString){return Compare(theString)<=0;}
	inline bool				operator<=(String &theString){return Compare(theString.mData)<=0;}
	
	void					operator=(char*theString){Free();if(theString){mLength = Length(theString);mData = new char[mLength+1];mData[mLength]=0;Stamp(mData,theString,0,mLength);mRefs = new int;*mRefs=1;}}
	void					operator=(const char*theString){*this=(char*)theString;}
	void					operator=(std::string theString){*this=theString.c_str();}
	void					operator=(std::string* theString){*this=theString->c_str();}
	void					operator=(int theInt){ParseInt(theInt);}
	void					operator=(const String &theString){if(this != &theString){Free();mData=theString.mData;mRefs=theString.mRefs;if(mRefs)++*mRefs;mLength=theString.mLength;}}
	void					operator=(longlong theLong){ParseLong(theLong);}
	void					operator=(float theFloat){ParseDouble((double)theFloat);}
	void					operator=(double theDouble){ParseDouble(theDouble);}
	void					operator=(void *thePtr){ParseAddress(thePtr);}
	void					operator=(bool theBool){ParseBool(theBool);}
	void					operator=(char theChar){Free();mData=new char[2];mLength=1;mRefs=new int;*mRefs=1;mData[0]=theChar;mData[1]=0;}
	
	void					operator+=(char*theString);
	inline void				operator+=(const char*theString){*this += (char*)theString;}
	void					operator+=(String &theString);
	void					operator+=(char theChar);
	void					operator+=(unsigned char theChar);
	inline void				operator+=(int theInt){*this += String(theInt);}
	inline void				operator+=(unsigned int theInt){*this += String(theInt);}
	inline void				operator+=(longlong theDouble){*this += String(theDouble);}
	inline void				operator+=(void *thePtr){*this += String(thePtr);}
	inline void				operator+=(float theFloat){*this += String(theFloat);}
	inline void				operator+=(double theDouble){*this += String(theDouble);}
	inline void				operator+=(bool theBool){*this += String(theBool);}

	inline void				operator--(){Delete(0,1);}

	inline void				operator-=(int theNumber){Truncate(mLength-theNumber);}

	String					operator+(String &theString);
	String					operator+(char *theString);
	String					operator+(char theChar);
	inline String			operator+(const char *theString){return *this + (char*)theString;}
	inline String			operator+(bool theBool){return *this + String(theBool);}
	virtual String			operator+(int theInt){return *this + String(theInt);}
	inline String			operator+(longlong theLong){return *this + String(theLong);}
	inline String			operator+(float theFloat){return *this + String(theFloat);}
	inline String			operator+(double theDouble){return *this + String(theDouble);}
	inline String			operator+(void *thePtr){return *this + String(thePtr);}

	virtual char&			operator[](int theSlot) {return GetChar(theSlot);}
	inline char				LastChar() {if (Len()==0) return 0;return mData[Len()-1];}

	operator				char*() {return c();}
	inline char&			GetChar(int theSlot) {static char aZero=0;if (theSlot<0 || theSlot>=Len()) return aZero;return mData[theSlot];}

	//
	// These are functions for getting pieces of a string in a C format
	//
	String					CGetToken(char theSeperator, bool wantTokenInString=false);			// Gets the first token in the string-- tokens are not noticed if inside ' or "
	String					CGetNextToken(char theSeperator=0, bool wantTokenInString=false);	// Gets the next token in the string-- tokens are not noticed if inside ' or "
	void					CParseArray(Array<char> &theList); // Takes format '1','2','3','4' and produces a list of chars
	void					CParseArray(Array<int> &theList); // Takes format 1,2,3,4 and produces a list of ints
	void					CParseArray(Array<Point> &theList); // Takes format {1,1),(2,2),(3,3) and produces a list of points
	void					CParseArray(Array<Rect> &theList); // Takes format {1,1,100,100),(2,2,100,100) and produces a list of Rects
	void					CParseArray(Array<String> &theList); // Takes format "One","Two","Three","Four" and produces a list of strings

	//
	// This encodes a string to work as a URL (takes out spaces and whatnot)
	//
	void					URLEncode();
	void					URLDecode();
	String					GetURLEncode();
	String					GetURLDecode();
	void					Base64Encode();
	String					GetBase64Encode();
	void					Base64Decode();
	String					GetBase64Decode();

	void					CloudKeyEncode(char theEncodeChar='~');
	inline String			GetCloudKeyEncode() {String aResult=*this;aResult.CloudKeyEncode();return aResult;}
	void					CloudKeyUnencode(char theEncodeChar='~');
	inline String			GetCloudKeyUnencode() {String aResult=*this;aResult.CloudKeyUnencode();return aResult;}

	
	//
	//END STUFF IMPORTED FROM StringEx
	//

	//
	// XML Parsing helpers... if the string is full of XML, this will return pertinent XML strings.
	// Calling with no parameters resets and prepares the string to parseXML from the start.
	//
	inline void				ParseXML() {mParseXMLCursor=0;}
	bool					ParseXML(String& theTag, String& theData, Array<String>* theParams=NULL);
	bool					ParseXMLTag(String& theTag, Array<String>* theParams=NULL);	// Return true if self-closing
	String					GetXML(String theTag);
	void					TokenizeXML(Array<StringString>& theResult);
	int						mParseXMLCursor;

	String					ParseURLValue(String theParamName);
	void					ParseAllURLValues(Array<String>& theResult);

	//
	// This steps through the string step by step, returning either chars, or full tags, depending on
	// what's next.  It doesn't try to find closing tags, etc-- it simply gives you the next element that's
	// arriving.
	//
	bool					StepXML(String& theResult);
	char					StepXMLChar();

	//
	// And RML... that's Raptisoft Markup Language.  This is put in specifically so it can be used
	// in filenames.  RML is in this format:  [TAG]data[!TAG]
	//
	inline void				ParseRML() {mParseXMLCursor=0;}
	bool					ParseRML(String& theTag, String& theData, Array<String>* theParams=NULL);
	bool					ParseRMLTag(String& theTag, Array<String>* theParams=NULL);	// Return true if self-closing
	String					GetRML(String theTag);

	bool					StepRML(String& theResult);
	char					StepRMLChar();

	//
	// Reads a param from a list of params (which is seperated by ; usually, but could be done with & to read web)
	//
	String					GetParam(String theParam, char theSeperator=';');
	inline String			GetWebParam(String theParam) {return GetParam(theParam,'&');}


	//
	// Removes a 'tail' from the string.  For instance, in the following string:
	// filename.extension, ".extension" is the tail.  It'll start from the end of
	// the string, and clear things out after the tail identifier.
	// 
	String					RemoveTail(char theTailCharacter);
	String					GetTail(char theTailCharacter);
	String					GetNotTail(char theTailCharacter);
	int						FindTail(char theTailCharacter);

	inline void				Unique() {PrepForInlineModification();}
	inline void				MakeUnique() {PrepForInlineModification();}

	//
	// Evaluates an arithmatic expression into a string
	//
	String					 Evaluate();

protected:

	/*
	inline void					Stamp(char *theDestination, char *theString, int thePosition, int theLength){char *aFinish = &theDestination[thePosition+theLength];char *aChar = &theDestination[thePosition];while (aChar < aFinish){*aChar=*theString;aChar++;theString++;}}
	inline char					*GetStamp(char *theDestination, char *theString, int thePosition, int theLength){char *aFinish = &theDestination[thePosition+theLength];char *aChar = &theDestination[thePosition];while (aChar < aFinish){*aChar=*theString;aChar++;theString++;}return aFinish;}
	inline char					*GetStamp(char *theDestination, char *thePosition, char *theEnd){char *aChar = theDestination;while (thePosition < theEnd){*aChar=*thePosition;aChar++;thePosition++;}return aChar;}
	inline void					Stamp(char *theDestination, char *thePosition, char *theEnd){char *aChar = theDestination;while (thePosition < theEnd){*aChar=*thePosition;aChar++;thePosition++;}}
	inline void					StampBlank(char *theDestination, int thePosition, int theLength){char *aFinish = &theDestination[thePosition+theLength];char *aChar = &theDestination[thePosition];while (aChar < aFinish){*aChar=32;aChar++;}}
	inline void					Stamp(char *theDestination, char theChar, int thePosition, int theLength){char *aFinish = &theDestination[thePosition+theLength];char *aChar = &theDestination[thePosition];while (aChar < aFinish){*aChar=theChar;aChar++;}}
	inline char					*GetStampBlank(char *theDestination, int thePosition, int theLength){char *aFinish = &theDestination[thePosition+theLength];char *aChar = &theDestination[thePosition];while (aChar < aFinish){*aChar=32;aChar++;}return aFinish;}
	*/

	//int						BitsNeeded(unsigned longlong theNumber);
	//inline void				StampBits(char *theString, unsigned longlong theData, unsigned longlong theStartBit, char theBitCount);
	//unsigned longlong 		ReadBits(char *theString, unsigned longlong theStartBit, char theBitCount);

	inline void				Stamp(char *theDestination, char *theString, int thePosition, int theLength){memcpy(&theDestination[thePosition],theString,theLength);}
	inline void				Stamp(char *theDestination, char *thePosition, char *theEnd){memcpy(theDestination,thePosition,theEnd-thePosition);}
	inline void				StampBlank(char *theDestination, int thePosition, int theLength){memset(&theDestination[thePosition],' ',theLength);}
	inline void				Stamp(char *theDestination, char theChar, int thePosition, int theLength){memset(&theDestination[thePosition],theChar,theLength);}
	inline char				*GetStamp(char *theDestination, char *theString, int thePosition, int theLength){memcpy(&theDestination[thePosition],theString,theLength);return &theDestination[thePosition+theLength];}
	inline char				*GetStamp(char *theDestination, char *thePosition, char *theEnd){int aJump = (int)(theEnd-thePosition); memcpy(theDestination,thePosition,aJump);return &theDestination[aJump];}
	inline char				*GetStampBlank(char *theDestination, int thePosition, int theLength){memset(&theDestination[thePosition],' ',theLength);return &theDestination[thePosition+theLength];}	

	void					PrepForInlineModification(){if(mData && mLength > 0){if(*mRefs > 1){--*mRefs;char *aChar = mData;mRefs = new int;*mRefs=1;aChar = new char[mLength+1];aChar[mLength]=0;Stamp(aChar,mData,0,mLength);mData=aChar;}}}
	bool					Contains(char *theString, char theChar){if(!theString)return false;while(*theString){if(theChar == *theString)return true;theString++;}return false;}
	String					*ProcessSpecifier(char *thePosition, int &theLength, va_list &aArgs);

	inline void				Constructor(){mExtraData=NULL;Init();mCaseSensitive=DEFAULT_CASE_SENSITIVITY;mParseXMLCursor=0;}

	struct ChrList
	{
		struct CharPtr{char *mData;CharPtr*mNext;};
		ChrList(){mHead=mTail=mCursor=0;}
		~ChrList(){if(mHead){while(mHead){CharPtr *aHold=mHead->mNext;delete mHead;mHead=aHold;}}}
		CharPtr *mHead, *mTail, *mCursor;
		void operator += (char *theChar){CharPtr *aPtr = new CharPtr();aPtr->mData=theChar;aPtr->mNext=0;if(mTail){mTail->mNext=aPtr;}else{mHead=aPtr;mCursor=mHead;}mTail=aPtr;}
		char *Get(){if(mCursor){char *aChar=mCursor->mData;mCursor=mCursor->mNext;return aChar;}return 0;}
	};
	struct StringList
	{
		struct StringPtr{String *mString;StringPtr*mNext;};
		StringList(){mHead=mTail=mCursor=0;}
		~StringList(){if(mHead){while(mHead){StringPtr *aHold=mHead->mNext;delete mHead;mHead=aHold;}}}
		StringPtr *mHead, *mTail, *mCursor;
		void operator += (String *theChar){StringPtr *aPtr = new StringPtr();aPtr->mString=theChar;aPtr->mNext=0;if(mTail){mTail->mNext=aPtr;}else{mHead=aPtr;mCursor=mHead;}mTail=aPtr;}
		String *Get(){if(mCursor){String *aChar=mCursor->mString;mCursor=mCursor->mNext;return aChar;}return 0;}
	};
};

inline void		operator--(String &theString, int theDummy){theString.Size(theString.mLength-1);}

inline String	operator+(char * theChar, String &theString){return String(theChar) + theString;}
inline String	operator+(const char * theChar, String &theString){return String(theChar) + theString;}
inline String	operator+(int theInt, String &theString){return String(theInt) + theString;}
inline String	operator+(unsigned int theInt, String &theString){return String(theInt) + theString;}
inline String	operator+(longlong theLong, String &theString){return String(theLong) + theString;}
inline String	operator+(bool theBool, String &theString){return String(theBool) + theString;}
inline String	operator+(float theFloat, String &theString){return String(theFloat) + theString;}
inline String	operator+(double theDouble, String &theString){return String(theDouble) + theString;}
inline String	operator+(void *thePtr, String &theString){return String(thePtr) + theString;}
inline String	operator+(char theChar, String &theString){return String(theChar) + theString;}

inline bool		operator==(char *theChar, String theString){return theString == theChar;}

inline String	Format(char *theFormat, ...){String aReturn; va_list aArgs;va_start(aArgs, theFormat);aReturn.Format(theFormat, aArgs);va_end(aArgs);return aReturn;}
inline String	Sprintf(char *theFormat, ...){String aReturn; va_list aArgs;va_start(aArgs, theFormat);aReturn.Format(theFormat, aArgs);va_end(aArgs);return aReturn;}
inline String	Sprintf(char *theFormat, va_list& theArgs){String aReturn; aReturn.Format(theFormat, theArgs);return aReturn;}

String			SuperscriptNumber(int theNumber);
String			IntToString(int theInt, bool addCommas);
String			FloatToString(float theFloat, bool addCommas, bool cutNondecimal);
String			FormatDate(int theMon, int theDay, int theYear, int theWeekday=-1);
String			FormatTime(int theHour, int theMin);

/*
class Dictionary
{
public:
	Array<String>		mList[129];
	void				Add(String theString);
	bool				Contains(String theString);
};
*/

String& GetDiscardableString();
bool IsNumber(char theChar, bool allowOperators=false);

//
// UTF8 Strings...
//
#define UTF8_PRIVATE_USE 0xE000

class StringUTF8 : public String
{
public:
	StringUTF8() {}
	StringUTF8(String theString) {this->strcpy(theString);}
	StringUTF8(char* theString) {this->strcpy(theString);}

	virtual ~StringUTF8(void) {Free();if (mExtraData) delete (UTF8Data*)mExtraData;}

	struct UTF8Data
	{
		char*				mUTF8Cursor;
	};

	inline UTF8Data*		HelpUTF8() {if (!mExtraData) {mExtraData=new UTF8Data;StartUTF8();}return (UTF8Data*)mExtraData;}

	void					StartUTF8(int theOffset=0) {HelpUTF8()->mUTF8Cursor=((char*)mData+theOffset);}
	int						GetUTF8();
	int						PeekUTF8();
	int						PeekUTF8Len(int aPos);
	int						GetCharAt(int aPos);
	int						TellUTF8() {return HelpUTF8()->mUTF8Cursor-(char*)mData;}
	void					AppendUTF8(int theNumber) {InsertUTF8(theNumber,Len());}
	void					RewindUTF8();
	int						PosUTF8(int thePos);
	void					InsertUTF8(int theChar, int thePosition=0);
	void					DeleteUTF8(int thePosition);
	int						LenUTF8();
};

struct StringString 
{
	StringString() {}
	StringString(String theStr1, String theStr2) {mString[0]=theStr1;mString[1]=theStr2;}
	StringString(char* theStr1, char* theStr2) {mString[0]=theStr1;mString[1]=theStr2;}

	String mString[2];
	virtual String& operator[](int theSlot) {return mString[theSlot%2];}
	inline String& Tag() {return mString[0];}
	inline String& Command() {return mString[0];}
	inline String& Cmd() {return mString[0];}
	inline String& Value() {return mString[1];}
	inline char* c()
	{
		String& aResult=GetDiscardableString();
		aResult="";aResult+=mString[0];aResult+=" - ";aResult+=mString[1];
		return aResult.c();
	}
	inline char* ToString() {return c();}
	inline bool	operator==(char*theString){return mString[0].Compare(theString)==0;}
	inline bool	operator==(const char*theString){return mString[0].Compare(theString)==0;}
	inline bool	operator==(String &theString){return mString[0].Compare(theString.mData)==0;}

};

class XMLTokens : public Array<StringString>
{
public:
	XMLTokens() {}
	XMLTokens(String& theSource) {theSource.TokenizeXML(*this);}
};


