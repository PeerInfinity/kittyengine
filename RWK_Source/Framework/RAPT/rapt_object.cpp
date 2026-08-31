#include "rapt.h"

UIDType gObjectUID=100;
void *gWantHeap=0;

Object::Object()
{
	mID=0;
	mUID=GetObjectID();
	AutoKill(this==gWantHeap);

	mKill=false;
	mFirstUpdate=true;
	mSort=NULL;
}

#ifdef _LEAKTESTING
#undef new
#undef delete
#endif

void* Object::operator new(size_t sz)
{
	gWantHeap=::operator new(sz);
	return gWantHeap;
}

void Object::operator delete (void *p)
{
	gWantHeap=NULL;
	::operator delete(p);
}

#ifdef _LEAKTESTING
#define	new		(m_setOwner  (__FILE__,__LINE__,__FUNCTION__),false) ? NULL : new
#define	delete		(m_setOwner  (__FILE__,__LINE__,__FUNCTION__),false) ? m_setOwner("",0,"") : delete
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// ObjectManager...
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ObjectManager::ObjectManager()
{
	Unique();	// Object Managers... always unique.
}

ObjectManager::~ObjectManager()
{
	Free();
}

void ObjectManager::Free()
{
	if (mKillList[0].Count()>0) {DestroyListOfObjects(mKillList[0]);}
	if (mKillList[1].Count()>0) {DestroyListOfObjects(mKillList[1]);}
	if (Count()<=0) return;

	ObjectManager& aThis=*this;
	EnumList(Object,aObject,aThis)
	{
		*this-=aObject;
		DestroyObject(aObject);
		EnumListRewind(Object);
	}
}

void ObjectManager::Update()
{
	mCurrentObject=NULL;
	HouseKeeping();
	if (Count()<=0) return;

	ObjectManager& aThis=*this;
	EnumList(Object,aObject,aThis)
	{
		mCurrentObject=aObject;
		if (aObject->mKill)
		{
			MoveObjectToKillList(aObject);
			EnumListRewind(Object);
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

void ObjectManager::UpdateConditionally()
{
	mCurrentObject=NULL;
	HouseKeeping();
	if (Count()<=0) return;

	ObjectManager& aThis=*this;
	EnumList(Object,aObject,aThis)
	{
		mCurrentObject=aObject;
		if (aObject->mKill)
		{
			MoveObjectToKillList(aObject);
			EnumListRewind(Object);
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


void ObjectManager::HouseKeeping()
{
	if (mKillList[gAppPtr->GetAppTime()%2].Count()>0) {if (mKillList[gAppPtr->GetAppTime()%2].Count()>0) DestroyListOfObjects(mKillList[gAppPtr->GetAppTime()%2]);}
}

void ObjectManager::MoveObjectToKillList(Object* theObject)
{
	*this-=theObject;
	mKillList[gAppPtr->GetAppTime()%2]+=theObject;
}


void ObjectManager::Cleanup()
{
	ObjectManager& aThis=*this;
	EnumList(Object,aObject,aThis)
	{
		if (aObject->mKill)
		{
			*this-=aObject;
			mKillList[gAppPtr->GetAppTime()%2]+=aObject;
			EnumListRewind(Object);
		}
	}
}


void ObjectManager::Draw()
{
	mCurrentObject=NULL;
	if (Count()<=0) return;

	ObjectManager& aThis=*this;
	EnumList(Object,aObject,aThis) if (!(aObject->mKill|aObject->mFirstUpdate)) aObject->Draw();
	mCurrentObject=NULL;
}

int ObjectManager::CountID(int theID)
{
	ObjectManager& aThis=*this;

	int aCount=0;
	EnumList(Object,aObject,aThis) if (aObject->mID==theID) aCount++;
	return aCount;
}

bool ObjectManager::Remove(Object *theItem)
{
	bool aReturn=false;
	aReturn|=PointerList<Object*>::Remove(theItem);
	aReturn|=mKillList[0].Remove(theItem);
	aReturn|=mKillList[1].Remove(theItem);
	return aReturn;
}

void ObjectManager::Notify(void *theData)
{
	if (Count()<=0) return;

	ObjectManager& aThis=*this;
	EnumList(Object,aObject,aThis) if (!aObject->mKill) aObject->Notify(theData);
}

void ObjectManager::DestroyObject(Object *theObject)
{
	if (theObject->mAutoKill) {delete theObject;}
}

void ObjectManager::DestroyListOfObjects(List &theList)
{
	EnumList(Object,aObject,theList) DestroyObject(aObject);
	theList.Clear();
}

void ObjectManager::Sort()
{
	PointerList<Object*> aSortedList;

	EnumList(Object,aZ,*this)
	{
		bool aFound=false;
		if (aZ->mSort) 
		{
			int aIPoint=0;
			EnumList(Object,aCheckZ,aSortedList)
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

