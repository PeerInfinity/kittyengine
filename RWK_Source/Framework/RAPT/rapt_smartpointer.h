#pragma once

#include <stddef.h>
#include "os_headers.h"

struct SmartPointerRef
{
	SmartPointerRef(void* thePtr=NULL, unsigned theCount=1) : mPtr(thePtr), mCount(theCount) {}
	void* mPtr;
	unsigned int mCount;
};


template <typename T>
class SmartPointer
{
public:

	SmartPointerRef*						mRef;

private:
	void Acquire(SmartPointerRef* theRef)
	{
		mRef=theRef;
		if(theRef) theRef->mCount+=1;
	}

	void Release()
	{
		if (mRef) 
		{
			mRef->mCount-=1;
			if(!mRef->mCount) 
			{
				T* aRef=(T*)mRef->mPtr;
				delete aRef;
				delete mRef;
			}
			mRef=NULL;
		}
	}

public:

	template<typename U>
	SmartPointer(const SmartPointer<U> &thePtr)
	{
		Acquire(thePtr.mRef);
	}

	SmartPointer(T* thePtr=NULL) : mRef(NULL)
	{
		if (thePtr) mRef=new SmartPointerRef(thePtr);
	}

	~SmartPointer()
	{
		Release();
	}

	SmartPointer(const SmartPointer& thePtr)
	{
		Acquire(thePtr.mRef);
	}

	SmartPointer& operator=(const SmartPointer& thePtr)
	{
		if (this != &thePtr)
		{
			Release();
			Acquire(thePtr.mRef);
		}
		return *this;
	}

	operator T*() const											{ if (!this->mRef) return NULL;return (T*)mRef->mPtr;}
	//operator T&() const										{ if (!this->mRef) return NULL;return (T&)*mRef->mPtr;}
	//T& operator*() const										{ return (T&)*mRef->mPtr; }
	T* operator->()	const										{ return (T*)mRef->mPtr; }
	T* ptr() const												{ return mRef ? (T*)mRef->mPtr:NULL; }
	T* Ptr() const												{ return mRef ? (T*)mRef->mPtr:NULL; }
	T* GetPointer() const										{ return mRef ? (T*)mRef->mPtr:NULL; }
	bool IsUnique() const										{ return (mRef ? mRef->mCount==1:true); }
	bool IsNull() const											{ return mRef ? mRef->mPtr==NULL:true; }
	bool IsNotNull() const										{ return mRef ? mRef->mPtr!=NULL:false; }
	bool operator==(void* theValue) const						{ if (!this->mRef) return false;return (this->mRef->mPtr==theValue);}
	bool operator!=(void* theValue) const						{ if (!this->mRef) return false;return (this->mRef->mPtr!=theValue);}
	bool operator==(const SmartPointer& theValue) const			{ if (!this->mRef) return false;return (this->mRef->mPtr==theValue.ptr());}
	bool operator!=(const SmartPointer& theValue) const			{ if (!this->mRef) return true;return (this->mRef->mPtr!=theValue.ptr());}
	bool operator !() const										{ if (!this->mRef) return true;return (this->mRef->mPtr==NULL);}
	operator bool() const										{ if (!this->mRef) return false;return (this->mRef->mPtr!=NULL);}

};


#define Smart(n) SmartPointer<n>
#define SmartList(n) PointerList<Smart(n)>
#define CastSmartList(type,list) (SmartList(type)*)&list
#define AsSmartObjects(list) (SmartList(Object)*)&list
#define SmartArray(n) Array<Smart(n)>

//
// For when you want to have a value, but you MIGHT want to point it at another value instead.  It's just a pointer, with mOwned to signify whether to delete it or not.
// Example use: I have a widget that holds a value.  But sometimes I want to point the value somewhere else.  This lets me do that seamlessly.
//
template <typename T>
class XPointer
{
public:
	virtual ~XPointer() {if (mOwned) delete mPtr;mPtr=NULL;}

	bool		mOwned=true;
	T*			mPtr=new T;

	operator T*() const {return mPtr;}
	inline void		PointAt(T* thePtr) {delete mPtr;mOwned=false;mPtr=thePtr;}
	inline void		Redirect(T* thePtr) {PointAt(thePtr);}
	inline void		Use(T* thePtr) {PointAt(thePtr);}
	inline T*		WhoAreYou()	{if (mOwned) return NULL;return mPtr;}
	//inline void		HookTo(T* thePtr) {PointAt(thePtr);} // Can't use HookTo... that's reserved for making an object connect to a pointer.

};

#define XInt XPointer<int>
#define XFloat XPointer<float>
#define XChar XPointer<char>
#define XBool XPointer<bool>


