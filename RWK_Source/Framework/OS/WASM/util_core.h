#pragma once
#include "os_headers.h"
#define __HELPERS
#include "common.h"
#undef __HELPERS

#ifdef __LEGACY_UTIL_CORE
inline int QRand(int theSeed)
{
    int aSeed=(214013*theSeed+2531011);
    return (aSeed>>16)&0x7FFF;
}

//
// This horrifying template complexity makes it so that the arrays start at zero if they're primitives,
// or if they're pointers, but not if you make, say, an array of classes that might be constructing stuff.
//
// So if you did:
//
//		Array<Class*> mArray;
//		mArray[2]=blah;
//
// ...then you can count on mArray[0] and mArray[1] being null.
//
// BUT, if you did:
//
//		struct Something {int mX;}
//		Array<Something> mArray;
//		mArray[2].mX=blah;
//
// ...then mArray[0].mX and mArray[1].mX are undefined, unless you set them in the constructor of the array.
//
// Note that this only happens with the CleanArray class (or if you set mZeroData to true) in order to keep it
// quarantined, and give an option to not use the functionality unless needed.  This was originally put in because
// the sprite BillboardShape array was non-zero and had no good way to zero it efficiently.
// 

template<typename T> struct IsPointer { static const bool value = false; };
template<typename T> struct IsPointer<T*> { static const bool value = true; };
template <typename var_type> inline void SetArrayData(var_type* thePos,char theData, int theLen) {}
//
// Note to self: theLen in the functions below already has sizeof factored in.  Don't do theLen*sizeof(type)
//
template <> inline void SetArrayData<char>(char* thePos,char theData, int theLen) {memset(thePos,theData,theLen);}
template <> inline void SetArrayData<bool>(bool* thePos,char theData, int theLen) {memset(thePos,theData,theLen);}
template <> inline void SetArrayData<int>(int* thePos,char theData, int theLen) {memset(thePos,theData,theLen);}
template <> inline void SetArrayData<float>(float* thePos,char theData, int theLen) {memset(thePos,theData,theLen);}
template <> inline void SetArrayData<unsigned int>(unsigned int* thePos,char theData, int theLen) {memset(thePos,theData,theLen);}
template <> inline void SetArrayData<unsigned char>(unsigned char* thePos,char theData, int theLen) {memset(thePos,theData,theLen);}
template <> inline void SetArrayData<double>(double* thePos,char theData, int theLen) {memset(thePos,theData,theLen);}

//
// Note: Use CleanArray if you need values zeroed out
// (And it won't zero values out for classes anyway)
//
template <typename var_type>
class Array
{
public:
	Array() {mArray=NULL;mSize=0;mLockSize=false;mZeroData=false;}
	Array(int initialSize) {mArray=NULL;mSize=0;mLockSize=false;GuaranteeSize(initialSize);mZeroData=false;}
	Array(int initialSize, var_type theValue) {mArray=NULL;mSize=0;mLockSize=false;Reset(initialSize,theValue);mZeroData=false;}
	Array(Array<var_type>& theValue) {mArray=NULL;mSize=0;mLockSize=0;mZeroData=theValue.mZeroData;Copy(theValue);}
	~Array() {Reset();}

	var_type		*mArray=NULL;
	int				mSize=0;
	inline int		Size() {return mSize;}

	bool			mLockSize=false;
	bool			mZeroData=false;

	void Reset() {delete [] mArray;mArray=NULL;mSize=0;mLockSize=false;}
	void Reset(int theSize) {delete [] mArray;mArray=NULL;mSize=0;Reset(theSize,NULL);mLockSize=false;}
	void Reset(int theSize, var_type theValue) {_DeleteArray(mArray);mArray=NULL;mSize=0;GuaranteeSize(theSize);for (int aCount=0;aCount<Size();aCount++) mArray[aCount]=theValue;mLockSize=false;}

	inline operator void* () {return (void*)mArray;}
	Array<var_type>&				operator=(Array<var_type>& rhs)
	{
		Copy(rhs);
		return *this;
	}
	Array<var_type>&				operator=(std::initializer_list<var_type> theValues)
	{
		Reset();GuaranteeSize(theValues.size());
		int aSlot=0;
		for (auto aItem=theValues.begin();aItem!=theValues.end();++aItem) {Element(aSlot++)=*aItem;}
		return *this;
	}

	inline void			Copy(Array<var_type>& rhs)
	{
		Reset();
		GuaranteeSize(rhs.Size());
		for (int aCount=0;aCount<rhs.Size();aCount++) Element(aCount)=rhs.Element(aCount);
	}
	
	inline var_type&	operator [] (int theSlot) {return Element(theSlot);}
	var_type&			Element(int theSlot) {if (theSlot>=mSize) GuaranteeSize(theSlot+1);return mArray[theSlot];}
	var_type&			Element(int theSlot, var_type theDefault) {if (theSlot>=mSize) {GuaranteeSize(theSlot+1);mArray[theSlot]=theDefault;};return mArray[theSlot];}
	var_type&			Set(int theSlot) {if (theSlot>=mSize) GuaranteeSize(theSlot+1);return mArray[theSlot];}
	inline var_type&	First() {return Element(0);}
	inline var_type&	Last() {return Element(_max(0,Size()-1));}
	inline var_type&	AddLast() {return Element(Size());}
	virtual void operator+=		(var_type theObject) {Element(Size())=theObject;}
	virtual void		Add(var_type theObject) {Element(Size())=theObject;}
	inline void operator+=		(Array<var_type> &theOther) {Append(theOther);}
	void						Append(Array<var_type> &theOther)
	{
		int aCursor=Size();
		GuaranteeSize(Size()+theOther.Size());
		for (int aCount=0;aCount<theOther.Size();aCount++) Element(aCount+aCursor)=theOther.Element(aCount);
	}

	inline void			Clip(int theSize)
	{
		if (theSize>=mSize) return;
		int aOldSize=mSize;
		mSize=theSize;
		var_type *aNewArray=new var_type[theSize];

		if (mArray)
		{
			for (int aCount=0;aCount<_min(aOldSize,mSize);aCount++) aNewArray[aCount]=mArray[aCount];
			if (mZeroData && mSize>aOldSize) ZeroData(&aNewArray[aOldSize],(mSize-aOldSize)*sizeof(var_type));
			_DeleteArray(mArray);
		}
		else if (mZeroData) ZeroData(aNewArray,(theSize)*sizeof(var_type));
		mArray=aNewArray;
	}

	inline int			ClampIndex(int theIndex) {return _clamp(0,theIndex,mSize-1);}
	inline int			WrapIndex(int theIndex) {return theIndex%(mSize);}
	inline void			SetSize(int theSize) {GuaranteeSize(theSize);}
	inline void			InitialSize(int theSize) {GuaranteeSize(theSize);}
	inline void			StartSize(int theSize) {GuaranteeSize(theSize);}
	inline bool			IsEmpty() {return mSize==0;}
	void GuaranteeSize(int theSize, bool lockSize=false)
	{
		if (mLockSize || theSize<=0) return;
		mLockSize=lockSize;

		if (theSize<=mSize) return;
		int aOldSize=mSize;
		var_type *aNewArray=new var_type[theSize];
		mSize=theSize;

		if (mArray)
		{
			for (int aCount=0;aCount<aOldSize;aCount++) aNewArray[aCount]=mArray[aCount];
            if (mZeroData) ZeroData(&aNewArray[aOldSize],(mSize-aOldSize)*sizeof(var_type));
			delete [] mArray;
		}
        else if (mZeroData) ZeroData(aNewArray,(theSize)*sizeof(var_type));
		mArray=aNewArray;
	}
    
	inline void ZeroData(var_type* theData, int theSize)
	{
		memset(theData,0,theSize);
		
		/*
		if (IsPointer<var_type>::value) 
		{
			for (int aCount=0;aCount<theSize;aCount++) memset((char*)&theData[aCount],0,sizeof(theData[aCount]));
			//memset(theData,0,theSize);
		}
		else SetArrayData(theData,0,theSize);
		*/
	}
	inline void Clean() {ZeroData(mArray,Size()*sizeof(var_type));}
	inline void ForceSize(int theSize) {if (Size()>theSize) Clip(theSize); if (Size()<theSize) GuaranteeSize(theSize);}
	
	void InsertElement(var_type theObject, int thePos=0)
	{
		GuaranteeSize(Size()+1);
		for (int aCount=Size()-1;aCount>thePos;aCount--) Element(aCount)=Element(aCount-1);
		Element(thePos)=theObject;
	}
   
	void DeleteElement(int theSlot)
	{
		if (theSlot<0 || theSlot>=Size()) return;
		if (Size()==1) {Reset();return;}

		var_type* aNewArray=new var_type[Size()-1]; 
		int aNewPtr=0;
		for (int aCount=0;aCount<Size();aCount++)
		{
			if (aCount!=theSlot)
			{
				aNewArray[aNewPtr]=mArray[aCount];
				aNewPtr++;
			}
		}

		delete [] mArray;
		mArray=aNewArray;
		mSize--;
	}
	inline void Truncate(int theNewSize)
	{
		if (theNewSize>=Size()) return;

		var_type* aNewArray=new var_type[theNewSize]; 
		int aNewPtr=0;
		for (int aCount=0;aCount<theNewSize;aCount++)
		{
			aNewArray[aNewPtr]=mArray[aCount];
			aNewPtr++;
		}
		delete [] mArray;
		mArray=aNewArray;
		mSize=theNewSize;
	}

	void Shuffle(int theRandomSeed)
	{
		int aSeed=theRandomSeed;
		for (int aCount=0;aCount<Size();aCount++)
		{
			aSeed=QRand(aSeed);
			int aOther=aSeed%Size();
			var_type aHold=Element(aCount);
			Element(aCount)=Element(aOther);
			Element(aOther)=aHold;
		}
	}

	int FindValue(var_type aValue)
	{
		for (int aCount=0;aCount<Size();aCount++) if (Element(aCount)==aValue) return aCount;
		return -1;
	}

	void SwapValues(var_type aValue1, var_type aValue2)
	{
		int aValue1Slot=FindValue(aValue1);
		int aValue2Slot=FindValue(aValue2);
		if (aValue1Slot>=0 && aValue2Slot>=0)
		{
			Element(aValue2Slot)=aValue1;
			Element(aValue1Slot)=aValue2;
		}
	}
};

//
// ReadOnlyArray does NO bounds checking or expanding.
// You MUST call GuaranteeSize before using it.  This exists mainly as compatibility for porting from other languages, where
// you have an array of data and it has specific size, but you need the length of the array constantly in the code... but you
// don't want the overhead every time you call it of checking if you're in the bounds.
//
// Again, only difference is no bounds checking when you read/set items.  MUST CALL GUARANTEESIZE!
//
template <typename var_type>
class ReadOnlyArray : public Array<var_type>
{
public:
	inline var_type&	operator [] (int theSlot) {return this->Element(theSlot);}
	var_type&			Element(int theSlot) {return this->mArray[theSlot];}

	//
	// If you do need to expand a read only array, use "Set"
	//
};



//
// Use a CleanArray when you need the data to be zeroed
//
template <typename var_type>
class CleanArray : public Array<var_type>
{
public:
    CleanArray() {Array<var_type>::mArray=NULL;Array<var_type>::mSize=0;Array<var_type>::mLockSize=false;Array<var_type>::mZeroData=true;}
	CleanArray(int initialSize) {Array<var_type>::mArray=NULL;Array<var_type>::mSize=0;Array<var_type>::mLockSize=false;Array<var_type>::mZeroData=true;Array<var_type>::GuaranteeSize(initialSize);}
	CleanArray(int initialSize, var_type theValue) {Array<var_type>::mArray=NULL;Array<var_type>::mSize=0;Array<var_type>::mLockSize=false;Array<var_type>::mZeroData=true;Array<var_type>::Reset(initialSize,theValue);}
};

template <class var_type>
class Stack : public Array<var_type>
{
public:
	Stack() {mStackPos=0;}

	inline void		Push(var_type theValue) {this->Element(this->mStackPos)=theValue;this->mStackPos++;}
	inline var_type	Pop() {this->mStackPos=_max(0,this->mStackPos-1);return this->Element(mStackPos);}
	inline var_type	Peek() {return this->Element(_max(0,mStackPos-1));}
	inline var_type	Peek(int howFarBack) {return this->Element(_max(0,mStackPos-howFarBack));}
	inline void		Empty() {this->mStackPos=0;}
	inline bool		IsEmpty() {return (this->mStackPos<=0);}
	inline int		GetStackCount() {return mStackPos;}

	//
	// Removes an item from the stack.
	//
	void			Unpush(var_type theItem)
	{
		int aStart=-1;
		for (int aCount=0;aCount<mStackPos;aCount++) if (this->Element(aCount)==theItem) {aStart=aCount;break;}
		if (aStart>=0) {mStackPos--;for (int aCount=aStart;aCount<mStackPos;aCount++) this->Element(aCount)=this->Element(aCount+1);}
	}

public:
	int				mStackPos;
};
#endif


