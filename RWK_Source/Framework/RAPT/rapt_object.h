#pragma once

#include "rapt_smartpointer.h"
#include "util_core.h"
#include "rapt_list.h"

#include <new>



//
// Object Notes:
// Objects contain a flag called AutoKill which is defaulted to false.
// However, Object overrides new so that this flag can be somewhat intelligently
// set up to let ObjectManager handle cleanup.  So, saying:
//
// aObject=new Object(); // Will set mAutoKill=true;
// Object aObject		// Will set mAutoKill=false;
// 
// This isn't perfect!  If you do aObject=new Object[5] they will NOT be marked with
// autocleanup!
//

#ifndef GL30
typedef int UIDType;
#else
typedef unsigned int UIDType;
#endif

extern UIDType gObjectUID;
inline void GuaranteeObjectID(UIDType theValue) {gObjectUID=_max(gObjectUID,theValue+1);}
inline UIDType GetObjectID() {return gObjectUID++;}

class SyncBuffer;
class Object
{
public:
	Object();
	virtual ~Object() {}

	virtual void			Initialize() {}

	virtual void			Update() {}
	virtual void			Draw() {}
	virtual void			Notify(void *theData) {}
	virtual void			Sync(SyncBuffer& theBuffer) {}
	virtual void*			Query(char* theQuery) {return NULL;}

	//
	// We override New and Delete so that we can determine if this object
	// is on the heap or stack.  Autokill is set so that ObjectManager
	// only deletes objects that were allocated with new.
	//
	#ifdef _LEAKTESTING
	#undef new
	#undef delete
	#endif

	static void*			operator new(size_t sz);
	void					operator delete (void *p);

	#ifdef _LEAKTESTING
	#define	new		(m_setOwner  (__FILE__,__LINE__,__FUNCTION__),false) ? NULL : new
	#define	delete		(m_setOwner  (__FILE__,__LINE__,__FUNCTION__),false) ? m_setOwner("",0,"") : delete
	#endif

public:
	//
	// Some flags
	//
	union
	{
		char mFlags;
		struct
		{
			bool					mFirstUpdate:1;	// If true, call initialize
			bool					mKill:1;		// Flag ObjectManager to kill this object now
			bool					mAutoKill:1;	// Flag ObjectManager to kill this object when the ObjectManager is killed
		};
	};

	virtual void			Kill() {mKill=true;}
	inline void				AutoKill(bool theState=true) {mAutoKill=theState;}
	int						mID;		// An ID variable for the purpose of identifying within the program (and for help with syncing)
	UIDType					mUID;		// An Unique identifier for this specific object, that can be transferred across the network
										// or for identification or something.

	inline void				SetID(int theID) {mID=theID;}
	inline void				SetID(char* theID) {mID=*((int*)theID);}
	inline bool				IsID(char* theID) {return (mID==*((int*)theID));}

	int						mSort;		// Sort tag for ObjectManager

	//
	// Manual way to run the first update... used when you must have initialized called
	// before processing stuff.
	//
	inline void				RunFirstUpdate() {if (mFirstUpdate) {Initialize();mFirstUpdate=false;}}
	inline void				FirstUpdate() {RunFirstUpdate();}
	inline void				DoFirstUpdate() {RunFirstUpdate();}

	//
	// For UpdateConditionally
	//
	virtual bool			CanUpdate() {return true;}

};

class ObjectManager : public PointerList<Object*>
{
public:
	ObjectManager();
	virtual ~ObjectManager();

	void					Update();
	void					UpdateConditionally();
	void					Draw();

	// If you want to make your own update loop, it goes like this:
	//
	// HouseKeeping();
	// Enum(...)
	// {
	// 	   Object->Update();
	// 	   if (Object->mKill) {MoveObjectToKillList(aObject);EnumListRewind(...);}
	// }
	void					HouseKeeping();
	void					MoveObjectToKillList(Object* theObject);

	//
	// Allows us to notify every object in the list...
	//
	void					Notify(void *theData);

	//
	// Count all objects of a certain ID (good for saying, like, how many
	// ID_SKELETONs are in the list)
	//
	int						CountID(int theID);

	//
	// Overrided remove item, because we need to also remove
	// them from the kill lists...
	//
	bool					Remove(Object *theItem);

	//
	// Frees all AutoKill objects in the manager...
	//
	virtual void			Free();

	//
	// Override so it's easier to delete objects...
	//
	virtual void			DestroyObject(Object *theObject);
	virtual void			DestroyListOfObjects(List &theList);

	//
	// Sorts objects depending on the contents of the mSort variable in Object...
	// If it's pointing to a float, use the F implementation.
	//
	void					Sort();

	//
	// If you ever need to remove "killed" objects from the list immediately, this
	// cleans up the list.
	//
	void					Cleanup();



public:
	List					mKillList[2];

	//
	// Current object being processed
	//
	Object					*mCurrentObject;
};


//typedef PObjectManager<void*> ObjectManager;

template <class var_type>
class SmartObjectManager : public SmartList(var_type)
{
public:
	SmartObjectManager() {SmartList(var_type)::Unique();}
	~SmartObjectManager() {Free();}

	void					Clear() {Free();}
	void					Free()
	{
		SmartObjectManager<var_type>& aThis=*this;
		EnumSmartList(var_type,aObject,aThis)
		{
			DestroyObject(aObject);
			EnumSmartListRewind(var_type);
		}
	}

	inline void				HouseKeeping() {}
	inline void				MoveObjectToKillList(Smart(var_type) theObject) {DestroyObject(theObject);}

	void					Update()
	{
		mCurrentObject=NULL;
		if (SmartList(var_type)::Count()<=0) return;

		SmartObjectManager<var_type>& aThis=*this;
		EnumSmartList(var_type,aObject,aThis)
		{
			mCurrentObject=aObject;
			if (aObject->mKill)
			{
				DestroyObject(aObject);
				EnumSmartListRewind(var_type);

			}
			else
			{
				if (aObject->mFirstUpdate)
				{
					aObject->mFirstUpdate=false;
					aObject->Initialize();
				}
				aObject->Update();
			}
		}
		mCurrentObject=NULL;
	}
	void					UpdateConditionally()
	{
		mCurrentObject=NULL;
		if (SmartList(var_type)::Count()<=0) return;

		SmartObjectManager<var_type>& aThis=*this;
		EnumSmartList(var_type,aObject,aThis)
		{
			mCurrentObject=aObject;
			if (aObject->mKill)
			{
				DestroyObject(aObject);
				EnumSmartListRewind(var_type);

			}
			else if (aObject->CanUpdate())
			{
				if (aObject->mFirstUpdate)
				{
					aObject->mFirstUpdate=false;
					aObject->Initialize();
				}
				aObject->Update();
			}
		}
		mCurrentObject=NULL;
	}
	void					Draw()
	{
		mCurrentObject=NULL;
		if (SmartList(var_type)::Count()<=0) return;

		SmartObjectManager<var_type>& aThis=*this;
		EnumSmartList(var_type,aObject,aThis) if (!aObject->mKill) aObject->Draw();
		mCurrentObject=NULL;
	}

	//
	// Allows us to notify every object in the list...
	//
	void					Notify(void *theData)
	{
		if (SmartList(var_type)::Count()<=0) return;
		SmartObjectManager<var_type>& aThis=*this;
		EnumSmartList(var_type,aObject,aThis) if (!aObject->mKill) aObject->Notify(theData);
	}

	//
	// Count all objects of a certain ID (good for saying, like, how many
	// ID_SKELETONs are in the list)
	//
	int						CountID(int theID)
	{
		SmartObjectManager<var_type>& aThis=*this;

		int aCount=0;
		EnumSmartList(var_type,aObject,aThis) if (aObject->mID==theID) aCount++;
		return aCount;
	}

	//
	// Override so it's easier to delete objects...
	//
	virtual void			DestroyObject(Smart(var_type) theObject)
	{
		SmartObjectManager& aThis=*this;
		aThis-=theObject;
	}

	//
	// Sorts objects depending on the contents of the mSort variable in Object...
	// If it's pointing to a float, use the F implementation.
	//
	void					Sort()
	{
		SmartList(var_type) aSortedList;

		EnumSmartList(var_type,aZ,*this)
		{
			bool aFound=false;
			if (aZ->mSort) 
			{
				int aIPoint=0;
				EnumSmartList(var_type,aCheckZ,aSortedList)
				{
					if (!aCheckZ->mSort) continue;
					if (aCheckZ->mSort>aZ->mSort)
					{
						aSortedList.Insert(aZ,aIPoint);
						aFound=true;
						break;
					}
					aIPoint++;
				}
			}
			if (!aFound) aSortedList+=aZ;
		}

		Clear();
		*this+=aSortedList;
	}

	void					Cleanup()
	{
		SmartObjectManager& aThis=*this;

		EnumSmartList(var_type,aObject,aThis)
		{
			if (aObject->mKill)
			{
				aThis-=aObject;
//				mKillList[gAppPtr->GetAppTime()%2]+=aObject;
				EnumListRewind(Object);
			}
		}
	}



public:
	//
	// Current object being processed
	//
	Smart(var_type)			mCurrentObject;
};


/*
//
// SyncObject
// SyncObject is an object that doesn't have any variables that are not clouded up into arrays.
// These arrays get saved, so you don't have to manually do a "sync," you can just call one sync
// and not have to change things all the time.
// 
// NOTE: Don't put it here... need to put it someplace else where we can include rapt.h ...
//
class IOBuffer;
class Color;
class String;
class SyncObject : public Object
{
public:
	SyncObject();

	void							SetCloudSize(int theSize);
	void							Sync(IOBuffer& theBuffer);
	void							Sync(SyncBuffer& theBuffer);

	Array<int>						mCloud;
	Array<float>					mCloud_Float;
	Array<Point>					mCloud_Point;
	Array<IPoint>					mCloud_IPoint;
	Array<Color>					mCloud_Color;
	Array<short>					mCloud_Short;
	Array<char>						mCloud_Char;
	Array<String>					mCloud_String;

};
*/


