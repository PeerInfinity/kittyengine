#pragma once

#include "os_headers.h"
#include "rapt_random.h"
#include "rapt_point.h"


#define _PASTE(a,b) a ## b
#define PASTE(a,b) _PASTE(a, b)
#define UNIQUENAME(prefix) PASTE(prefix,__LINE__)

//
// These functions help you step through lists or free lists easier...
//
#define EnumList(__type__,__name__,__list__) for (__type__ *UNIQUENAME=0, *__name__=(__type__*)(__list__)[(intptr_t)UNIQUENAME], **aEnumCursor=&UNIQUENAME;(intptr_t)UNIQUENAME<(__list__).GetCount();UNIQUENAME=(__type__*)((intptr_t)UNIQUENAME+1),__name__=(__type__*)(__list__)[(intptr_t)UNIQUENAME])
#define _FreeList(__type__,__list__) {for (int UNIQUENAME=0;UNIQUENAME<(__list__).GetCount();UNIQUENAME++) {__type__ *aFreeMe=(__type__*)(__list__)[(intptr_t)UNIQUENAME];delete aFreeMe;}(__list__).Clear();}
#define _DestroyList(__type__,__list__) {for (int UNIQUENAME=0;UNIQUENAME<(__list__).GetCount();UNIQUENAME++) {__type__ *aFreeMe=(__type__*)(__list__)[(intptr_t)UNIQUENAME];_Destroy(aFreeMe);}(__list__).Clear();}
#define FillList(__type__,__count__,__list__) {while ((__list__).GetCount()<__count__) __list__+=new __type__;}
#define EnumListCounter (int)(size_t)(*aEnumCursor)

//
// Step, Seek and Rewind let you move around in the list, in case you need to back
// up or go forward or seek the beginning or end.  You'll get compile errors if you use these outside of an EnumList
//
#define EnumListStep(__type__,theStep) *aEnumCursor=(__type__*)((intptr_t)*aEnumCursor+(theStep));
#define EnumListSeek(__type__,thePos) *aEnumCursor=(__type__*)(thePos-1);
#define EnumListRewind(__type__) EnumListStep(__type__,-1)
#define EnumListForward(__type__) EnumListStep(__type__,1)

//
// For smart lists...
// These require different behavior (__name__ is not a pointer)
//
#define EnumSmartList(__type__,__name__,__list__) for (Smart(__type__) *UNIQUENAME=0, __name__=(Smart(__type__))(__list__)[(intptr_t)UNIQUENAME], **aEnumCursor=&UNIQUENAME;(intptr_t)UNIQUENAME<(__list__).GetCount();UNIQUENAME=(Smart(__type__)*)((intptr_t)UNIQUENAME+1),__name__=(Smart(__type__))(__list__)[(intptr_t)UNIQUENAME])
#define EnumSmartListRewind(__type__) EnumListStep(Smart(__type__),-1)
#define EnumSmartListForward(__type__) EnumListStep(Smart(__type__),1)
#define FillSmartList(__type__,__count__,__list__) {while ((__list__).GetCount()<__count__) __list__+=new __type__;}

void Debug(char *theString, ...);
//
// Pointer List
// This list should be used for pointers ONLY.
// DO NOT attempt to use it for Smart Pointers.
// Use SmartList for that.
//
template <class var_type>
class PointerList
{
public:
	PointerList(void)
	{
		mCount=0;
		mSize=0;
		mData=NULL;
		mUnique=false;
		mCanShrink=true;
		mCanGrow=true;
	}
	virtual ~PointerList(void)
	{
		_DeleteArray(mData);
		mData=NULL;
	}
    


public:
	///////////////////////////////////////////////////
	//
	// If a list has the Unique bit set, then it won't
	// add a pointer to the list twice.  This is
	// slower.
	//
	bool						mUnique;
	inline void                 Unique() {SetUnique(true);}
	inline void					NotUnique() {SetUnique(false);}
	inline void					SetUnique(bool theState) {PointerList<var_type>::mUnique=theState;}
	//
	///////////////////////////////////////////////////

	///////////////////////////////////////////////////
	//
	// Determines if we can add null to the list...
	//
	bool						mAllowNull=false;
	inline void					AllowNull() {mAllowNull=true;}
	inline void					AllowNulls() {mAllowNull=true;}
	inline void					AllowZero() {mAllowNull=true;}
	inline void					DisallowNull() {mAllowNull=false;}
	//
	///////////////////////////////////////////////////


	///////////////////////////////////////////////////
	//
	// Determines how many items are in the List
	//
	int							mCount;
	inline int					Count() {return mCount;}
	inline int					GetCount() {return Count();}
	inline float				GetCountF() {return (float)Count();}
	//
	///////////////////////////////////////////////////

	///////////////////////////////////////////////////
	//
	// List Housekeeping
	//
	int							mSize;
	bool						mCanGrow;
	bool						mCanShrink;
	void						GuaranteeSize(int theSize)
	{
		//
		// Remember... this is NOT "GuaranteeCount."
		// This doesn't change mCount, it just allocates the space.
		//
		if (mSize<theSize)
		{
			theSize+=(mSize/2);
			if (!mData) mData=new var_type[theSize];
			else
			{
				var_type* aNewData;
				aNewData=new var_type[theSize];
				for (int aCount=0;aCount<mSize;aCount++) 
				{
					aNewData[aCount]=mData[aCount];
				}
				_DeleteArray(mData);
				mData=aNewData;
			}
			mSize=theSize;
		}
	}

	inline void					SetStatic(int theCount=0) {mCanGrow=true;mCanShrink=true; if (theCount) {SetSize(theCount);mCanGrow=false;mCanShrink=false;}}
	inline void					SetCanGrow(bool theState) {mCanGrow=theState;}
	inline void					SetCanShrink(bool theState) {mCanShrink=theState;}
	void						SetSize(int theSize)
	{
		if (theSize==mSize) return;
		if (theSize>mSize)
		{
			GuaranteeSize(theSize);
			return;
		}

		if (theSize<1) theSize=1;
		if (!mData) mData=new var_type[theSize];
		else
		{
			var_type* aNewData;
			aNewData=new var_type[theSize];

			for (int aCount=0;aCount<theSize;aCount++) aNewData[aCount]=mData[aCount];
			_DeleteArray(mData);
			mData=aNewData;
		}
		mSize=theSize;
	}
	//
	///////////////////////////////////////////////////


	///////////////////////////////////////////////////
	//
	// For adding items to the list
	//
	virtual void				Add(var_type theItem) {Insert(theItem,mCount);}
	virtual void				Insert(var_type theItem, int thePosition=0)
	{
		if (!theItem && !mAllowNull) return;
		if (mUnique) if (Exists(theItem)) return;
		if (mCanGrow) GuaranteeSize(mCount+1);
		else if (mCount>=(mSize)) return;
		if (thePosition<0) thePosition=0;
		if (thePosition>mCount) thePosition=mCount;
		if (thePosition<mCount) for (int aCount=mCount;aCount>thePosition;aCount--) mData[aCount]=mData[aCount-1];
		mData[thePosition]=theItem;
		mCount++;
	}

	virtual void				Set(var_type theItem, int thePosition=0)
	{
		if (!theItem && !mAllowNull) return;
		if (mUnique) if (Exists(theItem)) return;

		//
		// Fix this weird else structure... how is it SUPPOSED to work???
		//
		if (mSize<thePosition+1) {if (mCanGrow) SetSize(thePosition+1);else if (mCount>=(mSize)) return;}
		if (thePosition<0) return;

		mData[thePosition]=theItem;
		mCount=_max(mCount,thePosition+1);
	}

	virtual void operator+=		(var_type theObject) {Add(theObject);}
	void operator+=				(PointerList<var_type>& theList) {AddList(theList);}

	virtual void				AddList(PointerList<var_type>& theList) {for (int aCount=0;aCount<theList.mCount;aCount++) {Add(theList[aCount]);}}

	void operator =				(PointerList<var_type>& theList) {Clear();AddList(theList);}
	virtual void operator=		(var_type theObject) {Clear();Add(theObject);}

	//
	///////////////////////////////////////////////////

	///////////////////////////////////////////////////
	//
	// For removing items from the list
	//
	virtual bool				Remove(int theSlot)
	{
		if (theSlot<0 || theSlot>=mCount) return false;

		mData[theSlot]=NULL;
		for (int aCount=theSlot;aCount<(mCount-1);aCount++) mData[aCount]=mData[aCount+1];
		mData[mCount-1]=NULL;

		mCount=_max(0,mCount-1);

		if (mCanShrink)
		{
			if (mCount<mSize/2)
			{
				//
				// Okay, our list has gotten significantly smaller.
				// So let's shrink our data area down to the size of
				// our count.
				//
				SetSize(mCount);
			}
		}
		return true;
	}
	void						Remove(PointerList<var_type>& theList) {for (int aCount=0;aCount<theList.mCount;aCount++) {Remove(theList[aCount]);}}
	inline bool					RemoveLast() {return Remove(Count()-1);}


	//
	// Smart Object Problem:
	// If you add smart objects into the list WITHOUT smart containers around them, then trying to remove
	// an object does the following:
	//
	// 1. It binds the object into theItem below.
	// 2. It removes the object from the list, which releases it.
	// 3. It then tries to release the object from within theItem.  Alas, it's already been released!
	//
	virtual bool				Remove(var_type theItem) {if (!theItem && !mAllowNull) return false;int aSlot=Find(theItem);if (aSlot!=-1) {Remove(aSlot);return true;}return false;}
	bool						RemovePtr(void* theItem) {if (!theItem && !mAllowNull) return false;int aSlot=FindPtr(theItem);if (aSlot!=-1) {Remove(aSlot);return true;}return false;}

	void operator-=				(var_type theObject) {Remove(theObject);}
	void operator-=				(PointerList<var_type>& theList) {for (int aCount=0;aCount<theList.mCount;aCount++){Remove(theList[aCount]);}}
	void						Clear()
	{
		if (!mCanShrink) mCount=0;
		else
		{
			delete [] mData;
			//_DeleteArray(mData);
			mData=NULL;
			mCount=0;
			mSize=0;
		}
	}
	void						Zero()
	{
		if (!mCanShrink) 
		{
			for (int aCount=0;aCount<mSize;aCount++) mData[aCount]=NULL;
			mCount=0;
		}
		else Clear();
	}
	//
	// For non-shrinking lists, lets you set all the upper bounds to zero... important for smart objects.
	//
	void						ZeroTop() {if (!mCanShrink) for (int aCount=mCount;aCount<mSize;aCount++) mData[aCount]=NULL;}


	inline void					Reset() {Clear();}
	//
	///////////////////////////////////////////////////

	///////////////////////////////////////////////////
	//
	// To see if items are in the list
	//
	virtual int					Find(var_type theItem) {if (!mData) return -1;for (int aCount=0;aCount<mCount;aCount++) if (mData[aCount]==theItem) return aCount;return -1;}
	virtual int					FindPtr(void* theItem) {if (!mData) return -1;for (int aCount=0;aCount<mCount;aCount++) if ((void*)mData[aCount]==theItem) return aCount;return -1;}
	inline bool					Exists(var_type theItem) {return (Find(theItem)!=-1);}
	inline bool					ExistsPtr(void* theItem) {return (FindPtr(theItem)!=-1);}
	//
	///////////////////////////////////////////////////

	///////////////////////////////////////////////////
	//
	// For retrieving items from the list
	//
	var_type					Fetch(int theSlot)
	{
		if (theSlot<0 || theSlot>=mCount) return NULL;
		return mData[theSlot];
	}
	var_type					FetchFirst() {return Fetch(0);}
	var_type					Last() {return Fetch(Count()-1);}
	var_type					FetchLast() {return Fetch(Count()-1);}
	var_type					FetchPtr(void* theItem) {int aSlot=FindPtr(theItem);if (aSlot>=0) return mData[aSlot];return NULL;}
	var_type					FetchRandom()
	{
		return Fetch(gRand.Get(GetCount()));
	}
	var_type					operator [] (int theSlot) {return Fetch(theSlot);}
	//
	///////////////////////////////////////////////////

	///////////////////////////////////////////////////
	//
	// Quickie ways to change the order of things
	// in a list...
	//
	void						Shuffle()
	{
		if (mCount<=1) return;
		for (int aCount=0;aCount<mCount;aCount++)
		{
			var_type aHold=mData[aCount];
			int aPick=aCount;while (aPick==aCount) aPick=gRand.Get(GetCount());
			mData[aCount]=mData[aPick];
			mData[aPick]=aHold;
		}
	}
	void						Shuffle(Random &theRandomizer)
	{
		if (mCount<=1) return;
		for (int aCount=0;aCount<mCount;aCount++)
		{
			var_type aHold=mData[aCount];
			int aPick=aCount;while (aPick==aCount) aPick=theRandomizer.Get(GetCount());
			mData[aCount]=mData[aPick];
			mData[aPick]=aHold;
		}
	}
	void						Reverse()
	{
		if (!mData) return;
		if (mCount==1) return;

		var_type* mTempData=new var_type[mCount];

		int aTPtr=mCount-1;
		for (int aCount=0;aCount<mCount;aCount++) {mTempData[aCount]=mData[aTPtr];aTPtr--;}
		for (int aCount=0;aCount<mCount;aCount++) {mData[aCount]=mTempData[aCount];}

		_DeleteArray(mTempData);
	}
	void						Swap(var_type theFirst,var_type theSecond)
	{
		int aSlot1=Find(theFirst);
		int aSlot2=Find(theSecond);
		if (aSlot1>=0 && aSlot2>=0) Swap(aSlot1,aSlot2);
	}
	void						Swap(int theSlot1,int theSlot2)
	{
		var_type aHold=mData[theSlot1];
		mData[theSlot1]=mData[theSlot2];
		mData[theSlot2]=aHold;
	}

	var_type					Replace(var_type theExisting, var_type theReplace)	// Returns the old copy so you can delete it if you want...
	{
		int aSlot=Find(theExisting);if (aSlot<0) return NULL;
		var_type aHold=mData[aSlot];
		mData[aSlot]=theReplace;
		return aHold;
	}

	//
	///////////////////////////////////////////////////

public:
	//
	// Actual list data
	//
	var_type*					mData;
};

typedef PointerList<void*> List;
#define ListT(n) PointerList<n>

#define EnumLList(_type_,_var_,_list_) LinkedListNode<_type_*>* UNIQUENAME=(_list_).GetFirst();for (_type_* _var_=UNIQUENAME->GetData();UNIQUENAME!=&(_list_).mNullNode;UNIQUENAME=UNIQUENAME->GetNext(),_var_=UNIQUENAME->GetData())
#define EnumLListPtr(_type_,_var_,_list_) LinkedListNode<_type_*>* UNIQUENAME=(_list_)->GetFirst();for (_type_* _var_=UNIQUENAME->GetData();UNIQUENAME!=&(_list_)->mNullNode;UNIQUENAME=UNIQUENAME->GetNext(),_var_=UNIQUENAME->GetData())
#define EnumLListWithNode(_nodename_,_type_,_var_,_list_) LinkedListNode<_type_*>* _nodename_=_list_.GetFirst();for (_type_* _var_=_nodename_->GetData();_nodename_!=&_list_.mNullNode;_nodename_=_nodename_->GetNext(),_var_=_nodename_->GetData())
#define EnumLListFromNode(_node_,_type_,_var_,_list_) LinkedListNode<_type_*>* UNIQUENAME=_node_->GetNext();for (_type_* _var_=UNIQUENAME->GetData();UNIQUENAME!=&_list_.mNullNode;UNIQUENAME=UNIQUENAME->GetNext(),_var_=UNIQUENAME->GetData())

template <typename T>
class LinkedListNode
{
public:
	LinkedListNode() {mData=NULL;mNext=mPrev=NULL;}

	operator					T() {return mData;}

	T							mData;
	inline T&					GetData() {return mData;}
	LinkedListNode*				mNext;
	LinkedListNode*				mPrev;

	inline LinkedListNode*		GetNext() {return mNext;}
	inline LinkedListNode*		GetPrev() {return mPrev;}
};

template <typename T>
class LinkedList
{
public:
	LinkedList() {mHead=&mNullNode;mTail=&mNullNode;mCount=0;}
	virtual ~LinkedList() {Reset();}


	LinkedListNode<T>*				mHead;
	LinkedListNode<T>*				mTail;

	LinkedListNode<T>				mNullNode;

	int								mCount;
	inline int						GetCount() {return mCount;}

	inline LinkedListNode<T>*		GetFirst() {return mHead;}
	inline LinkedListNode<T>*		GetTail() {return mTail;}

	void Add(T theData)
	{
		LinkedListNode<T>* aNode=new LinkedListNode<T>;
		aNode->mData=theData;
		aNode->mNext=&mNullNode;
		if (mTail==&mNullNode)
		{
			mTail=aNode;
			mHead=aNode;
		}
		else
		{
			mTail->mNext=aNode;
			aNode->mPrev=mTail;
			mTail=aNode;
		}
		mCount++;
	}

	void Remove(T theData)
	{
		LinkedListNode<T>* aNode=GetFirst();
		while (aNode)
		{
			if (aNode->mData==theData)
			{
				aNode->mPrev->mNext=aNode->mNext;
				delete aNode;
				break;
			}
			aNode=aNode->GetNext();
		}
	}

	void Reset()
	{
		LinkedListNode<T>* aNode=GetFirst();
		while (aNode)
		{
			LinkedListNode<T>* aKillNode=aNode;
			aNode=aNode->GetNext();
			delete aKillNode;
		}
		mHead=mTail=&mNullNode;
		mCount=0;
	}

	LinkedList<T>& operator+= (const T& t) { Add(t); return *this; }
	LinkedList<T>& operator-= (const T& t) { Remove(t); return *this; }

};

/*
template <typename T>
class LinkedList
{
private :

	unsigned int m_nCount;
	LinkedListNode<T>* m_pHead;
	LinkedListNode<T>* m_pTail;

	inline LinkedListNode<T>* GetFirstNode() {return m_pHead;}

	LinkedListNode<T>* GetNode(unsigned int n) const
	{
		unsigned int nCurrNode=0;
		LinkedListNode<T>* pCurr = m_pHead;

		for(;;)
		{
			if(nCurrNode==n)
			{
				return pCurr;
			}
			pCurr = pCurr->pNext;
			nCurrNode++;
		}
	}

	PNODE FindNode(const T& element)
	{
		PNODE pCurr = m_pHead;

		while(pCurr)
		{
			if(pCurr->element==element)
			{
				return pCurr;
			}
			pCurr = pCurr->pNext;
		}

		return NULL;
	}

public:
	/////////////////////////////////////////
	// NODE BASED STUFF
	/////////////////////////////////////////
	struct POSITION { PNODE p; POSITION() : p(NULL) {} };

	POSITION GetHeadPosition() const
	{
		POSITION p;
		p.p=m_pHead;
		return p;
	}

	POSITION GetTailPosition() const
	{
		POSITION p;
		p.p=m_pTail;
		return p;
	}

	T& GetNext(POSITION& pos) const
	{
		PNODE pn = pos.p;
		pos.p = pos.p->pNext;
		return pn->element;
	}

	T& GetPrev(POSITION& pos) const
	{
		PNODE pn = pos.p;
		pos.p = pos.p->pPrev;
		return pn->element;
	}

	void EraseElementAt(POSITION pos)
	{
		if(pos.p->pPrev) pos.p->pPrev->pNext=pos.p->pNext; else m_pHead=pos.p->pNext;
		if(pos.p->pNext) pos.p->pNext->pPrev=pos.p->pPrev; else m_pTail = pos.p->pPrev;
		m_nCount--;
		delete pos.p;
	}


	/////////////////////////////////////////
	// Adding and assigning
	/////////////////////////////////////////
	LinkedList& operator=(const LinkedList& other)
	{
		Reset();
		for(int i=0; i<other.Count(); i++)
		{
			this->AddLast(other.Fetch(i));
		}
		return *this;
	}

	LinkedList(const LinkedList& other)
	{
		m_nCount=0;
		m_pHead=NULL;
		m_pTail=NULL;

		for(int i=0; i<other.Count(); i++)
		{
			this->AddLast(other.Fetch(i));
		}
	}

	void AddFirst(T newitem)
	{
		m_nCount++;
		PNODE pn = new NODE;
		pn->element = newitem;
		if(!m_pHead)
		{
			m_pHead = pn;
			m_pTail = pn;
		}
		else
		{
			m_pHead->pPrev=pn;
			pn->pNext=m_pHead;
			m_pHead = pn;
		}
	}

	void Add(T newitem)
	{
		AddLast(newitem);
	}

	void AddLast(T newitem)
	{
		m_nCount++;
		PNODE pn = new NODE;
		pn->element = newitem;
		if(!m_pTail)
		{
			m_pTail = pn;
			m_pHead = pn;
		}
		else
		{
			m_pTail->pNext = pn;
			pn->pPrev = m_pTail;
			m_pTail = pn;
		}
	}

	void Insert(T newitem, unsigned int nIndex=0)
	{
		if(nIndex==0) 
		{
			AddFirst(newitem);
		}
		else if (nIndex>=m_nCount) 
		{
			AddLast(newitem);
		}
		else if(nIndex>0 && nIndex<m_nCount)
		{
			POSITION posNext;
			posNext.p=GetNode(nIndex);
			POSITION posPrev;
			posPrev.p=GetNode(nIndex-1);

			m_nCount++;
			PNODE pn = new NODE;
			pn->element = newitem;

			// link the inserted node
			pn->pNext = posNext.p;
			pn->pPrev = posPrev.p;

			// adjust the surrounding nodes
			posNext.p->pPrev=pn;
			posPrev.p->pNext=pn;
		}
	}

	LinkedList& operator+= (const T& t) { AddLast(t); return *this; }
	LinkedList& operator+= (const LinkedList<T>& other) 
	{
		POSITION pos = other.GetHeadPosition();
		if(!pos.p) return *this;

		do
		{
			AddLast(other.GetNext(pos));
		}
		while(pos.p!=NULL);

		return *this; 
	}
	LinkedList& operator-= (const T& t) { Remove(t); return *this; }


	/////////////////////////////////////////
	// Getting stuff from the list
	/////////////////////////////////////////
	T& Fetch(unsigned int n)
	{
		return GetNode(n)->element;
	}

	T Fetch(unsigned int n) const
	{
		return GetNode(n)->element;
	}

	T& operator[] (unsigned int n)
	{
		return Fetch(n);
	}

	T& FetchFirst()
	{
		return m_pHead->element;
	}

	T& FetchLast()
	{
		return m_pTail->element;
	}

	T& FetchRandom()
	{
		return Fetch(gRand.GetSpan(0, Count()-1));
	}

	/////////////////////////////////////////
	// Removing stuff from the list
	/////////////////////////////////////////
	void Remove(const T& t)
	{
		POSITION pos;
		pos.p=FindNode(t);
		if(pos.p) EraseElementAt(pos);
	}

	void Remove(unsigned int n)
	{
		POSITION pos;
		pos.p=GetNode(n);
		EraseElementAt(pos);
	}

	T RemoveLast()
	{
		T element = m_pTail->element;
		PNODE pn = m_pTail;
		m_pTail=m_pTail->pPrev;
		if(m_pTail) m_pTail->pNext=NULL;
		m_nCount--;
		if(m_nCount==0) m_pHead=NULL;
		delete pn;
		return element;
	}

	T RemoveFirst()
	{
		T element = m_pHead->element;
		PNODE pn = m_pHead;
		m_pHead=m_pHead->pNext;
		if(m_pHead) m_pHead->pPrev=NULL;
		m_nCount--;
		if(m_nCount==0) m_pTail=NULL;
		delete pn;
		return element;
	}

	/////////////////////////////////////////
	// Counting and Querying the list
	/////////////////////////////////////////
	bool Exists(const T& t)
	{
		POSITION pos;
		pos.p=FindNode(t);
		if(pos.p) return true;
		return false;
	}

	int Find(const T& element)
	{
		int index=0;
		PNODE pCurr = m_pHead;

		while(pCurr)
		{
			if(pCurr->element==element)
			{
				return index;
			}
			pCurr = pCurr->pNext;
			index++;
		}

		return -1;
	}

	inline int Count() const { return m_nCount; }
	inline int GetCount() const { return Count(); }
	bool IsEmpty() const { return m_nCount==0; }

	/////////////////////////////////////////
	// Clearing and resetting the list
	/////////////////////////////////////////
	void Reset()
	{
		PNODE pCurr = m_pHead;
		while(pCurr)
		{
			PNODE pNext = pCurr->pNext;
			delete pCurr;
			pCurr = pNext;
		}

		m_pHead = NULL;
		m_pTail = NULL;
		m_nCount=0;
	}

	void Clear() { Reset(); }

	/////////////////////////////////////////
	// Modifying the list & randomization
	/////////////////////////////////////////
	void Swap(unsigned int x, unsigned int y)
	{
		PNODE pnodex = GetNode(x);
		PNODE pnodey = GetNode(y);

		T temp = pnodex->element;
		pnodex->element = pnodey->element;
		pnodey->element = temp;
	}

	void Shuffle()
	{
		int iCount = Count();
		if(iCount<2) return;

		for(int j=0;j<2; j++)
		{
			for(int i=0;i <iCount-2;i++)
			{
				int k = gRand.GetSpan(i+1, iCount-1);
				Swap(i, k);
			}
		}
	}

	LinkedList() : m_nCount(0), m_pHead(NULL), m_pTail(NULL) {}
	virtual ~LinkedList() { Reset(); }
};
*/

typedef LinkedList<void*> LList;





