#include "rapt_CPU.h"
#include "rapt.h"

CPU *gCurrentCPU=NULL;			// Current CPU... this is always the CPU that is making a call to an override.
CPU *gCurrentUserFocus=NULL;

CPU::CPU() : Object()
{
	Size();
	Forward(NULL);

	mAge=0;
	ManualAge(false);
	mNotify=NULL;
	mCPUManager=NULL;
	mTimerList=NULL;
	mVariablePointer=NULL;
	mThrottle=false;
	mThrottleFinishing=false;
	SetFeatures(CPU_DEFAULT);
	mPauseCount=0;
	mNotifyData.mParam=0;
	mNotifyData.mExtraData=NULL;
	mParent=NULL;
	SetCursor();
	CaptureTouches();

}

CPU::~CPU()
{
	if (mThrottleHook) ThrottleRelease(0);
	if (gCurrentUserFocus==this) {gCurrentUserFocus->LostUserFocus();gCurrentUserFocus=NULL;}

	if (gAppPtr) gAppPtr->UnfocusTouchesByCPU(this);
	if (gAppPtr) gAppPtr->UnfocusSpins(this);
	//FocusSpins(false);

	FocusTouches(false);
	if (gAppPtr && !gAppPtr->IsQuit()) FocusKeyboard(false); 
	FocusAccelerometer(false);
	FocusBackButton(false);

	if (gAppPtr) gAppPtr->UnsetHoverCPU(this);
	if (mCPUManager)
	{
		delete mCPUManager;
		mCPUManager=NULL;
	}
	if (mTimerList) 
	{
		_FreeList(CPUTimer,*mTimerList);
		delete mTimerList;
		mTimerList=NULL;
	}

	if (gAppPtr) Unhook();
}

void CPU::KillChildren() {delete mCPUManager;mCPUManager=NULL;}

void CPU::Unhook()
{
	//if (gAppPtr) if (!gAppPtr->IsQuit())
	{
		if (mVariablePointer)
		{
			CPU** aCPU=(CPU**)mVariablePointer;
			if (*aCPU==this) *aCPU=NULL;
			mVariablePointer=NULL;
		}
	}
}

void CPU::SetCurrentCPU(CPU *theCPU)
{
	gCurrentCPU=theCPU;
}

void CPU::Size()
{
	if (gAppPtr) Size(gG.GetPageRect());

}

void CPU::Size(float theWidth, float theHeight)
{
	if (gAppPtr) Size((gG.WidthF()/2)-(theWidth/2),(gG.HeightF()/2)-(theHeight/2),theWidth,theHeight);
}

void CPU::Resize(Point theP1, Point theP2)
{
	Point aUL=Point(_min(theP1.mX,theP2.mX),_min(theP1.mY,theP2.mY));
	Point aLR=Point(_max(theP1.mX,theP2.mX),_max(theP1.mY,theP2.mY));
	aLR-=aUL;

	Resize(aUL.mX,aUL.mY,aLR.mX,aLR.mY);
}

void CPU::Resize(float theX, float theY, float theWidth, float theHeight)
{
	mX=theX;
	mY=theY;
	mWidth=theWidth;
	mHeight=theHeight;
	SizeChanged();
}

CPU* CPU::GetCurrentCPU()
{
	return gCurrentCPU;
}


void CPU::Kill()
{
	if (gAppPtr) gAppPtr->UnIsolateCPU(this);
	Object::Kill();
}


void CPU::HookTo(void* thePointer)
{
	if (thePointer) 
	{
		CPU** aCPU=(CPU**)thePointer;
		*aCPU=this;
		mVariablePointer=thePointer;
	}
	else mVariablePointer=NULL;
}

void CPU::HookToSingleton(void* thePointer)
{
	mVariablePointer=NULL;
	if (thePointer) 
	{
		CPU** aCPU=(CPU**)thePointer;
		if (*aCPU==NULL)
		{
			*aCPU=this;
			mVariablePointer=thePointer;
		}
		else Kill();
	}
}

void CPU::Core_Initialize()
{
	if (gAppPtr->IsQuit()) return;
	Initialize();
	if (mInitializeHook) mInitializeHook(this);
}

void CPU::Core_Update()
{
	if (gAppPtr->IsQuit()) return;

	//if (mFeatures&CPU_UPDATE)
	{
		if (!IsPaused())
		{
			DISPATCHPROCESS(mForwardUpdate,Update());
			if (mUpdateHook) mUpdateHook(this);

			if (!mManualAge) mAge++;
			if (mCPUManager) mCPUManager->Update();
			if (mTimerList)
			{
				EnumList(CPUTimer,aT,*mTimerList)
				{
					if (--aT->mTime<=0)
					{
						Notify(aT->mNotify);
						*mTimerList-=aT;
						delete aT;
						EnumListRewind(CPUTimer);
					}
				}
				if (mTimerList->GetCount()==0)
				{
					delete mTimerList;
					mTimerList=NULL;
				}

			}
		}
		//else if (mPause>0) mPause--;
	}
}

void CPU::Core_Draw()
{
	if (gAppPtr->IsQuit()) return;

	if (mFeatures&CPU_DRAW)
	{
		if (mFeatures&CPU_CLIP)
		{
			gG.PushClip();
			gG.Clip(mX,mY,mWidth,mHeight);
		}
		gG.PushTranslate();
		gG.PushColor();

		if (mFeatures&CPU_CENTERED) gG.Translate(Center());
		else gG.Translate(UpperLeft());
		DISPATCHPROCESS(mForwardDraw,Draw());
		if (mDrawHook) mDrawHook(this);

		if (mCPUManager) mCPUManager->Draw();
		DISPATCHPROCESS(mForwardDraw,DrawOverlay());
		if (mDrawOverHook) mDrawOverHook(this);

		gG.PopColor();
		gG.PopTranslate();
		if (mFeatures&CPU_CLIP) gG.PopClip();
		mFirstDraw=false;
	}
}

void CPU::Core_Multitasking(bool isForeground)
{
	//
	// Note: Notifies self, then children.
	// Is this the right direction to do this in?
	//
	Multitasking(isForeground);
	if (mCPUManager) mCPUManager->Multitasking(isForeground);
}

void CPU::Core_SystemNotify(String theNotify)
{
	SystemNotify(theNotify);
	if (mCPUManager) mCPUManager->SystemNotify(theNotify);
}


IPoint CPU::FixCPUCoordinates(int x, int y)
{
	Point aMod=Point(0,0);//UpperLeft();
	if (mFeatures&CPU_CENTERED) aMod=Center()-UpperLeft();
	x-=(int)aMod.mX;
	y-=(int)aMod.mY;
	return IPoint(x,y);
}

void CPU::Core_TouchStart(int x, int y)
{
	mDragging=false;
	if (gAppPtr->IsQuit()) return;
	if (mCaptureTouches) if (gAppPtr) {gAppPtr->FocusTouchesByID(gAppPtr->GetTouchID(),this);}

	IPoint aPos=FixCPUCoordinates(x,y);

	DISPATCHPROCESS(mForwardTouches,TouchStart(aPos.mX,aPos.mY));

	if (gCurrentUserFocus) if (gCurrentUserFocus!=this) gCurrentUserFocus->LostUserFocus();
	gCurrentUserFocus=this;
}

void CPU::Core_TouchMove(int x, int y)
{
	if (gAppPtr->IsQuit()) return;
	IPoint aPos=FixCPUCoordinates(x,y);

	if (mFeatures&CPU_DRAG && !mDragging && gAppPtr->IsTouching()) if (gAppPtr->GetTouchMoveLength()>mDragThreshold)
	{
		mDragging=true;
		//if (gAppPtr) {gAppPtr->FocusTouchesByID(gAppPtr->GetTouchID(),this);}
		{DISPATCHPROCESS(mForwardTouches,DragStart(aPos.mX,aPos.mY));}
	}
//	if (mFeatures&CPU_DRAG && !mDragging && gAppPtr->IsTouching()) if (gAppPtr->GetTouchMoveLength()>mDragThreshold) {mDragging=true;{DISPATCHPROCESS(mForwardTouches,DragStart(aPos.mX,aPos.mY));}}
	if (mDragging) {DISPATCHPROCESS(mForwardTouches,Drag(aPos.mX,aPos.mY));}
	else {DISPATCHPROCESS(mForwardTouches,TouchMove(aPos.mX,aPos.mY));}
}

void CPU::Core_TouchEnd(int x, int y)
{
	if (gAppPtr->IsQuit()) return;

	IPoint aPos=FixCPUCoordinates(x,y);
	if (gAppPtr) gAppPtr->UnfocusTouchesByID(gAppPtr->GetTouchID());

	if (mDragging) {DISPATCHPROCESS(mForwardTouches,DragEnd(aPos.mX,aPos.mY));}
	DISPATCHPROCESS(mForwardTouches,TouchEnd(aPos.mX,aPos.mY));
	mDragging=false;
}

void CPU::Core_Spin(int theDir)
{
	if (gAppPtr->IsQuit()) return;

	DISPATCHPROCESS(mForwardTouches,Spin(theDir));
}


void CPU::FocusTouches(bool theState)
{
	if (gAppPtr)
	{
		if (theState) gAppPtr->FocusTouches(this);
		else if (gAppPtr->GetFocusTouches()==this) gAppPtr->FocusTouches(NULL);
	}
}

void CPU::FocusSpins(bool theState)
{
	if (gAppPtr)
	{
		if (theState) gAppPtr->FocusSpins(this);
		else gAppPtr->UnfocusSpins(this);
	}
}

void CPU::FocusBackButton(bool theState)
{
	if (gAppPtr)
	{
		if (theState) gAppPtr->FocusBackButton(this);
		else gAppPtr->UnfocusBackButton(this);
	}
}

void CPU::FocusKeyboard(bool theState)
{
	if (gAppPtr)
	{
		if (theState) gAppPtr->FocusKeyboard(this);
		else if (gAppPtr->GetFocusKeyboard()==this) gAppPtr->FocusKeyboard(NULL);
	}
}

bool CPU::IsFocusKeyboard(bool includeChildren)
{
	CPU* aF=gAppPtr->GetFocusKeyboard();
	if (aF==this) return true;
	if (includeChildren && mCPUManager)	{EnumList(CPU,aCPU,*mCPUManager) if (aCPU->IsFocusKeyboard()) return true;}
	return false;
}


void CPU::FocusAccelerometer(bool theState)
{
	if (gAppPtr)
	{
		if (theState) gAppPtr->FocusAccelerometer(this);
		else if (gAppPtr->GetFocusAccelerometer()==this) gAppPtr->FocusAccelerometer(NULL);
	}
}

CPU* CPU::HitTest(float x, float y)
{
	if (mFeatures&CPU_TOUCH)
	{
		bool aCP=Expand(mHitBorder).ContainsPoint(x,y);
		if (aCP || mFeatures&CPU_OUTERCHILDREN)
		{
			if (mCPUManager) 
			{
				Point aWork=Point(x,y);

				if (mFeatures&CPU_CENTERED) aWork-=Center();
				else aWork-=UpperLeft();

				CPU *aSubCPU=mCPUManager->Pick(aWork.mX,aWork.mY);
				if (aSubCPU) return aSubCPU;

			}
			if (aCP) return this;
		}
	}
	else if (mFeatures&CPU_TOUCHCHILDREN)
	{
		//
		// CPU can be set up to only pass children's touches...
		//
		if (mCPUManager) 
		{
			Point aWork=Point(x,y);

			if (mFeatures&CPU_CENTERED) aWork-=Center();
			else aWork-=UpperLeft();

			CPU *aSubCPU=mCPUManager->Pick(aWork.mX,aWork.mY);
			if (aSubCPU) return aSubCPU;
		}
	}
	return NULL;
}

void CPU::Core_KeyDown(int theKey)
{
	if (gAppPtr->IsQuit()) return;
	DISPATCHPROCESS(mForwardKeyboard,KeyDown(theKey));
}

void CPU::Core_KeyUp(int theKey)
{
	if (gAppPtr->IsQuit()) return;
	DISPATCHPROCESS(mForwardKeyboard,KeyUp(theKey));
}

void CPU::Core_Char(int theKey)
{
	if (gAppPtr->IsQuit()) return;
	DISPATCHPROCESS(mForwardKeyboard,Char(theKey));
}

void CPU::Core_Accelerometer(int theX, int theY, int theZ)
{
	if (gAppPtr->IsQuit()) return;
	DISPATCHPROCESS(mForwardAccelerometer,Accelerometer(theX,theY,theZ));
}

void CPU::Core_GotKeyboardFocus()
{
	if (gAppPtr->IsQuit()) return;
	DISPATCHPROCESS(mForwardKeyboard,GotKeyboardFocus());
}

void CPU::Core_LostKeyboardFocus()
{
	if (gAppPtr->IsQuit()) return;
	DISPATCHPROCESS(mForwardKeyboard,LostKeyboardFocus());
}



void CPU::AddCPU(CPU* theCPU)
{
	if (!theCPU) return;
	if (!mCPUManager) mCPUManager=new CPUManager;
	*mCPUManager+=theCPU;

	theCPU->mParent=this;
	if (gAppPtr && theCPU->mFeatures&CPU_TIMESLICE) gAppPtr->mTimeSliceProcesses+=theCPU;
}

void CPU::AddCPUToBottom(CPU* theCPU)
{
	if (!theCPU) return;
	if (!mCPUManager) mCPUManager=new CPUManager;
	mCPUManager->Insert(theCPU);
	theCPU->mParent=this;
}

void CPU::RemoveCPU(CPU* theCPU)
{
	if (!theCPU) return;
	if (!mCPUManager) return;
	*mCPUManager-=theCPU;

	if (theCPU->mParent==this) theCPU->mParent=NULL;
	if (gAppPtr && theCPU->mFeatures&CPU_TIMESLICE) gAppPtr->mTimeSliceProcesses-=theCPU;
}

void CPU::SetCursor(int theCursor)
{
	mCursor=theCursor;
	if (gAppPtr) gAppPtr->RefreshCursor();
}


bool CPU::IsHover()
{
	return (gAppPtr->GetHoverCPU()==this);
}

void CPU::Core_Enter()
{
	if (gAppPtr->IsQuit()) return;
	Enter();
}

void CPU::Core_Leave()
{
	if (gAppPtr->IsQuit()) return;
	Leave();
}

#ifndef NO_THREADS
int CPU::Throttle()
{
	if (gAppPtr->IsQuit()) return 0;

	gAppPtr->System_Clear();
	gAppPtr->RefreshTouches();
	mThrottle=true;
	mThrottleFinishing=true;
	mThrottleResult=0;

	while (mThrottle && !mKill && !gAppPtr->IsQuit()) 
	{
		gAppPtr->Throttle();
	}
	mThrottleFinishing=false;
	return mThrottleResult;
}
#endif

void CPU::Throttle(CPUHOOKPTR theHook, bool autoKill, bool runHookOnlyOnNonNullResult)
{
	if (gAppPtr->IsQuit()) return;

	mThrottleHook=theHook;
	mThrottleAutokill=autoKill;
	mThrottleNonNullHook=runHookOnlyOnNonNullResult;

	gAppPtr->System_Clear();
	gAppPtr->RefreshTouches();

	mThrottle=true;
	mThrottleFinishing=false;	// False because this variable is designed to let it walk out of throttle stacks while keeping the object in existence.
								// There is no throttle stack with a lambda'd throttle, so no need...
	mThrottleResult=0;
}

void CPU::ThrottleRelease(int theResult)
{
	mThrottle=false;
	mThrottleResult=theResult;

	if (mThrottleHook) {if (!mThrottleNonNullHook || mThrottleResult) mThrottleHook(this);}
	mThrottleHook=NULL;
	if (mThrottleAutokill) Kill();
}

Point CPU::ConvertScreenToLocal(Point thePos)
{
	CPU* aAlignCPU=this;
	Point aFixPos=thePos;
	while (aAlignCPU)
	{
		aFixPos-=aAlignCPU->UpperLeft();
		aAlignCPU=aAlignCPU->mParent;
	}
	return aFixPos;
}

Point CPU::ConvertLocalToScreen(Point thePos)
{
	CPU* aAlignCPU=this;
	Point aFixPos=thePos;

	while (aAlignCPU)
	{
		aFixPos+=aAlignCPU->UpperLeft();
		aAlignCPU=aAlignCPU->mParent;
	}
	return aFixPos;
}

Rect CPU::ConvertRectToScreen()
{
	Rect aRect=*this;
	CPU* aAlignCPU=this;
	Point aFixPos=Point(0,0);//aRect.UpperLeft();
	while (aAlignCPU)
	{
		aFixPos+=aAlignCPU->UpperLeft();
		aAlignCPU=aAlignCPU->mParent;
	}
	aRect.mX=aFixPos.mX;
	aRect.mY=aFixPos.mY;
	return aRect;
}

Rect CPU::ConvertRectToLocal()
{
	Rect aRect=*this;
	CPU* aAlignCPU=this;
	Point aFixPos=Point(0,0);//aRect.UpperLeft();
	while (aAlignCPU)
	{
		aFixPos-=aAlignCPU->UpperLeft();
		aAlignCPU=aAlignCPU->mParent;
	}
	aRect.mX=aFixPos.mX;
	aRect.mY=aFixPos.mY;
	return aRect;
}

Rect CPU::ConvertRectToLocal(Rect theRect)
{
	Point aP1=ConvertScreenToLocal(theRect.UpperLeft());
	Point aP2=ConvertScreenToLocal(theRect.LowerRight());

	Rect aRect=Rect(aP1,aP2);
	return aRect;
}

void CPU::AddTimer(void *theNotify, int theTime)
{
	if (mTimerList==NULL) mTimerList=new List;

	CPUTimer *aT=new CPUTimer;
	aT->mNotify=theNotify;
	aT->mTime=theTime;
	*mTimerList+=aT;
}


Graphics* CPU::G()
{
	return &gG;
}





////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// CPUManager...
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CPUManager::~CPUManager()
{
	Free();
}


void CPUManager::Update()
{
	CPUManager& aThis=*this;
	mCurrentObject=NULL;
	if (mKillList[gAppPtr->GetAppTime()%2].Count()>0) 
	{
		/*
		EnumList(CPU,aCPU,mKillList[gAppPtr->GetAppTime()%2]) 
		{
			gOut.Out("KillList=%d",aCPU);
			mKillList[gAppPtr->GetAppTime()%2]-=aCPU;
			delete aCPU;
			EnumListRewind(CPU);
		}
		*/
		DestroyListOfObjects(mKillList[gAppPtr->GetAppTime()%2]);
	}
	if (mThrottledList.Count()>0)
	{
		EnumList(CPU,aObject,mThrottledList) if (!aObject->mThrottleFinishing)
		{
			mThrottledList-=aObject;
			mKillList[gAppPtr->GetAppTime()%2]+=aObject;
			EnumListRewind(CPU);
		}
	}
	if (Count()<=0) return;

	EnumList(CPU,aObject,aThis)
	{
		mCurrentObject=aObject;
		if (aObject->mKill)
		{
			*this-=aObject;
			if (!aObject->mThrottleFinishing) mKillList[gAppPtr->GetAppTime()%2]+=aObject;
			else 
			{
				aObject->Unhook();
				mThrottledList+=aObject;
			}
			EnumListRewind(CPU);
		}
		else if (aObject->mFeatures&CPU_UPDATE)	// NOTE: I changed this to happen here, so that initializes won't happen if a CPU isn't updating.
		{
			if (aObject->mFirstUpdate)
			{
				aObject->mFirstUpdate=false;
				aObject->Core_Initialize();
				if (gAppPtr) gAppPtr->RefreshTouches();
			}
			aObject->Core_Update();
		}
	}
	mCurrentObject=NULL;
}


void CPUManager::Draw()
{
	CPUManager& aThis=*this;
	EnumList(CPU,aObject,aThis)
	{
		mCurrentObject=aObject;
		if (!aObject->mKill && (!aObject->mFirstUpdate || !(aObject->mFeatures&CPU_UPDATE))) aObject->Core_Draw();
	}
	mCurrentObject=NULL;
}

CPU* CPUManager::Pick(float x, float y)
{
	CPUManager& aThis=*this;
	for (int aCount=aThis.GetCount()-1;aCount>=0;aCount--)
	{

		CPU *aCPU=(CPU*)aThis[aCount];
		if (aCPU->mKill) continue;

		CPU *aPickCPU=aCPU->HitTest(x,y);
		if (aPickCPU) return aPickCPU;
	}

	return NULL;
}

void CPUManager::Multitasking(bool theState)
{
	CPUManager& aThis=*this;
	EnumList(CPU,aObject,aThis) aObject->Core_Multitasking(theState);
}

void CPUManager::SystemNotify(String theNotify)
{
	CPUManager& aThis=*this;
	EnumList(CPU,aObject,aThis) aObject->Core_SystemNotify(theNotify);
}

void CPUManager::DestroyObject(Object *theObject)
{
	CPU *aCPU=(CPU*)theObject;
//	aCPU->Destroy();
//	aCPU->Core_Destroy();

	if (gAppPtr) gAppPtr->mTimeSliceProcesses-=aCPU;


	if (theObject->mAutoKill) {delete aCPU;}

	//
	// If a CPU gets destroyed, force garbage collection on
	// managed systems... some of the CPUs require work in their
	// destructor to handle themselves, so this will force it.
	//
	gAppPtr->GarbageCollect();
}

void CPUManager::ClearKillLists()
{
	if (mKillList[0].Count()>0) {DestroyListOfObjects(mKillList[0]);}
	if (mKillList[1].Count()>0) {DestroyListOfObjects(mKillList[1]);}
}

void CPU::KidnapTouch()
{
	mDragging=false;
	if (mCaptureTouches) if (gAppPtr) {gAppPtr->FocusTouchesByID(gAppPtr->GetTouchID(),this);}
	if (gCurrentUserFocus) if (gCurrentUserFocus!=this) gCurrentUserFocus->LostUserFocus();
	gCurrentUserFocus=this;

	gAppPtr->UniquifyTouchID(this);
}

bool CPU::IsFPSCursor() {return gAppPtr->IsFPSCursor();}
