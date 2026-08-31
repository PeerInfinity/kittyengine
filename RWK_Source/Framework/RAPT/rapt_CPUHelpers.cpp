#include "rapt_CPUHelpers.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Pause Overlay...
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PauseOverlay::PauseOverlay()
{
	if (gAppPtr)
	{
		if (gAppPtr->IsPaused()) {Kill();return;}
		else 
		{
			gAppPtr->Pause();
			gAudio.Pause();
		}

		if (gAppPtr->mIsolatedCPUManager) 
		{
			*gAppPtr+=this;
			*gAppPtr->mIsolatedCPUManager+=this;
		}
		else
		{
			*gAppPtr+=this;
			gAppPtr->IsolateCPU(this);
		}
	}
}

void PauseOverlay::TouchStart(int x, int y)
{
	if (gAppPtr)
	{
		Kill();
		gAudio.Pause(false);
		gAppPtr->Unpause();
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Joypad
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Joypad::Joypad()
{
	EnableFeature(CPU_CENTERED);
	mSprite=NULL;
	mIsDown=false;
	mIsRDown=false;
	mStateChanged=false;
	mStickAngle=0;
	mStickAngleOffset=0;
	mPointAtTouch=false;
	mBinding=0;
	mRadius=0;
	mDragPad=false;
}


void Joypad::Update()
{
	//
	// We would poll a joystick here if attached to one (or the accelerometer)
	//
	if (mBinding)
	{
		if (mBinding&gBind_To_Keyboard)
		{
			mStickPosition=Point(0,0);
			bool aOldIsDown=mIsDown;
			mIsDown=false;
			if (gAppPtr->GetFocusKeyboard()==this || !gAppPtr->GetFocusKeyboard())
			{
				if (gInput.KeyState(mKeyBind[0])) {mStickPosition.mY-=1;mIsDown=true;}
				if (gInput.KeyState(mKeyBind[1])) {mStickPosition.mY+=1;mIsDown=true;}
				if (gInput.KeyState(mKeyBind[2])) {mStickPosition.mX-=1;mIsDown=true;}
				if (gInput.KeyState(mKeyBind[3])) {mStickPosition.mX+=1;mIsDown=true;}
			}
			if (aOldIsDown!=mIsDown) mStateChanged=true;
		}
		if (mBinding&gBind_To_Joystick)
		{
			mStickPosition=gInput.GetJoystickDir(mJoystickNumber,mThumbstickNumber);
		}

		mStickPosition.SetLength(1);
		mStickAngle=gMath.VectorToAngle(mStickPosition);
	}
}

void Joypad::Go(Point theCenter, float theRadius, Sprite* theSprite)
{
	Size(theCenter.mX-theRadius,theCenter.mY-theRadius,theRadius*2,theRadius*2);
	mSprite=theSprite;

	mRadius=theRadius;
}

void Joypad::Go(Point theCenter, Sprite* theSprite)
{
	Go(theCenter,(float)_max(theSprite->mWidth/2,theSprite->mHeight/2),theSprite);
}

void Joypad::BindToKeyboard(int theUp, int theDown, int theLeft, int theRight)
{
	//
	// No binding to keyboard on a touch device.
	//
	if (gAppPtr->IsTouchDevice()) return;

	mKeyBind[0]=theUp;
	mKeyBind[1]=theDown;
	mKeyBind[2]=theLeft;
	mKeyBind[3]=theRight;
	mBinding|=gBind_To_Keyboard;

	DisableFeature(CPU_DRAW);
	DisableFeature(CPU_TOUCH);
}

void Joypad::BindToJoystick(int theJoystick, int theThumbstick)
{
	//
	// No binding to joystick on a touch device.
	//
	if (gAppPtr->IsTouchDevice()) return;

	mBinding|=gBind_To_Joystick;
	mJoystickNumber=theJoystick;
	mThumbstickNumber=theThumbstick;

	DisableFeature(CPU_DRAW);
	DisableFeature(CPU_TOUCH);
}

void Joypad::BindToFPSMouse(Point theCenter)
{
	if (!(mBinding&gBind_To_FPSMouse))
	{
		if (gAppPtr->IsTouchDevice()) return;
		mBinding|=gBind_To_FPSMouse;
		DisableFeature(CPU_DRAW);
		Size();
	}

	mCenterOffset=theCenter-Center();
	if (IsDown()) RefreshTouch();
}



void Joypad::Draw()
{
	if (mSprite)
	{
		if (mPointAtTouch) mStickAngleOffset=0;
		mSprite->DrawRotated(0,0,mStickAngle-mStickAngleOffset);
	}
}

void Joypad::TouchStart(int x, int y)
{
	mIsRDown=false;
	if (gAppPtr->GetMouseButton()==-1 || (mBinding&gBind_To_FPSMouse))
	{
/*
		if (mDragPad)
		{
			mDragOffset=Point(x,y)-mCenterOffset;
			mStickPosition=(Point(x,y)-mDragOffset);

			if (mStickPosition.mX!=0 || mStickPosition.mY!=0) 
			{
				float aOldAngle=mStickAngle-mStickAngleOffset;
				mStickAngle=gMath.VectorToAngle(mStickPosition);
				mStickAngleOffset=mStickAngle-aOldAngle;
			}
		}
*/
		mLastTouchPos=IPoint(x,y);

		if (mDragPad) mDragOffset=Point(x,y);
		else mDragOffset=Point(0,0);

		if (!mIsDown) mStateChanged=true;
		mIsDown=true;
		if (gAppPtr->GetMouseButton()==1) mIsRDown=true;

		mStickPosition=(Point(x,y)-mDragOffset)-mCenterOffset;
		if (mStickPosition.mX!=0 || mStickPosition.mY!=0) 
		{
			float aOldAngle=mStickAngle-mStickAngleOffset;
			mStickAngle=gMath.VectorToAngle(mStickPosition);
			mStickAngleOffset=mStickAngle-aOldAngle;
		}

		mStickPosition.SetLength(_min(1.0f,mStickPosition.Length()/mRadius));
	}

}

void Joypad::TouchMove(int x, int y)
{
	if (mIsDown)
	{
		mLastTouchPos=IPoint(x,y);
        mStickPosition=(Point(x,y)-mDragOffset)-mCenterOffset;	// This was +mCenterOffset... but I think it's supposed to be minus.  If you get weirdness with FPS binding, that's why.

		if (mStickPosition.mX!=0 || mStickPosition.mY!=0) 
		{
			mStickAngle=gMath.VectorToAngle(mStickPosition);
		}

		float aRad=_max(mWidth,mHeight)/2;
		mStickPosition.SetLength(_min(1.0f,mStickPosition.Length()/mRadius));
	}
}

void Joypad::TouchEnd(int x, int y)
{
	mLastTouchPos=IPoint(x,y);
	if (mIsDown) mStateChanged=true;
    mIsDown=false;
	mIsRDown=false;
    mStickPosition=Point(0,0);
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Button
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Button::mSkipNotify=false;
Button::Button()
{
	mDragThruV=false;
	mDragThruH=false;
	mDrawCentered=false;
	mDragThruIgnoreTouchEnd=true;

	mDragThruThreshold=5;
	if (gAppPtr->IsTouchDevice()) mDragThruThreshold=15;
	mHolding=false;

	NotifyOnRelease();
	DisableFeature(CPU_DRAW);	// We don't draw unless we explicitely set a sprite...
	DisableFeature(CPU_UPDATE);	// We don't update unless we are bound to a key...

	mBinding=0;
	*mIsDown=false;
	mIsOver=false;
	mHiliteSound=NULL;
	mPressSound=NULL;
	mReleaseSound=NULL;

	mSprite[0]=NULL;
    mSpriteScale=1.0f;

	SetCursor(CURSOR_FINGER);
}

void Button::Update()
{
	if (mHolding) return;
	if (mBinding&gBind_To_Keyboard)
	{
		if (gAppPtr->GetFocusKeyboard()==this || !gAppPtr->GetFocusKeyboard())
		{
			if (gInput.KeyPressed(mKey[0])) 
			{
				if (IsKeyDown(mKey[1]) || mKey[1]==-1) 
				{
					if (IsKeyDown(mKey[2]) || mKey[2]==-1) 
					{
						TouchStart(0,0);
						TouchEnd(0,0);
					}
				}
			}
		}

		bool aDown=false;
		if (gAppPtr->GetFocusKeyboard()==this || !gAppPtr->GetFocusKeyboard()) if (IsKeyDown(mKey[0])) if (mKey[1]==-1 || IsKeyDown(mKey[1])) if (mKey[2]==-1 || IsKeyDown(mKey[2])) aDown=true;
		*mIsDown=aDown;
	}
	if (mBinding&gBind_To_DoubleKeyboard)
	{
		if (gAppPtr->GetFocusKeyboard()==this || !gAppPtr->GetFocusKeyboard())
		{
			if (gInput.KeyPressed(mKey[0]) || gInput.KeyPressed(mKey[1])) 
			{
				if (IsKeyDown(mKey[2]) || mKey[2]==-1) 
				{
					if (IsKeyDown(mKey[3]) || mKey[3]==-1) 
					{
						TouchStart(0,0);
						TouchEnd(0,0);
					}
				}
			}
		}

		bool aDown=false;
		if (gAppPtr->GetFocusKeyboard()==this || !gAppPtr->GetFocusKeyboard()) if (IsKeyDown(mKey[0]) || IsKeyDown(mKey[1])) if (mKey[2]==-1 || IsKeyDown(mKey[2])) if (mKey[3]==-1 || IsKeyDown(mKey[3])) aDown=true;
		*mIsDown=aDown;
	}
}

void Button::Draw()
{
	int aState=GetDrawState();
#ifndef LEGACY_GL
	if (aState>=mSprite.Size()) aState=0;
#else
	if (aState==1 && !mSprite[1]) aState=0;
	if (aState==2 && !mSprite[2]) aState=0;
#endif
	
	if (mSprite[aState])
    {
		if (!mDrawCentered) mSprite[aState]->DrawScaled(Center()-UpperLeft(),mSpriteScale);
		else mSprite[aState]->DrawScaled(mWidth/2,mHeight/2,mSpriteScale);
    }
}

void Button::BindToKeyboard(int theKey,int theKey2, int theKey3)
{
    if (gAppPtr->IsTouchDevice()) return;
    mBinding=gBind_To_Keyboard;
    mKey[0]=theKey;
	mKey[1]=theKey2;
	mKey[2]=theKey3;
	EnableUpdate();
    DisableDraw();
}

void Button::BindToDoubleKeyboard(int theKey, int theOtherKey, int theKey2, int theKey3)
{
	if (gAppPtr->IsTouchDevice()) return;
	mBinding=gBind_To_DoubleKeyboard;
	mKey[0]=theKey;
	mKey[1]=theOtherKey;
	mKey[2]=theKey2;
	mKey[3]=theKey3;
	EnableUpdate();
	DisableDraw();
}

void Button::TouchStart(int x, int y)
{
	if (*mIsDown) return;

	*mIsDown=true;
	mHolding=true;

	if (mNotifyOnPress || (mNotifyOnDoubleClick && gAppPtr->IsDoubleClick()))
	{
		mNotifyData.mParam=HELPER_NOTIFY_PRESS;
		if (!mSkipNotify)
		{
			if (mExe) mExe(this);
			if (mNotify) mNotify->Notify(this);
		}
	}
	if (mPressSound) mPressSound->Play();
}

void Button::TouchMove(int x, int y)
{
	if (*mIsDown)
	{
		if (mNotifyOnMove) 
		{
			mNotifyData.mParam=HELPER_NOTIFY_MOVE;
			if (!mSkipNotify)
			{
				if (mExe) mExe(this);
				if (mNotify) mNotify->Notify(this);
			}
		}

		if ((mDragThruV && gMath.Abs(gAppPtr->mMessageData_TouchPosition.mY-gAppPtr->mMessageData_TouchStartPosition.mY)>mDragThruThreshold) || (mDragThruH && gMath.Abs(gAppPtr->mMessageData_TouchPosition.mX-gAppPtr->mMessageData_TouchStartPosition.mX)>mDragThruThreshold))
		{
			*mIsDown=false;
			Point aHoldPos=gAppPtr->mMessageData_TouchPosition;
			gAppPtr->mMessageData_TouchPosition=Point(-9999,-9999);
			gAppPtr->ForceTouchEnd(mDragThruIgnoreTouchEnd);
			int aHold=mFeatures;
			gAppPtr->mMessageData_TouchPosition=aHoldPos;
			DisableFeature(CPU_TOUCH);
			gAppPtr->ForceTouchStart();
			mFeatures=aHold;
		}
	}


}

void Button::TouchEnd(int x, int y)
{
	mHolding=false;

	if (!(*mIsDown)) return;
	*mIsDown=false;
	
	//if (mNotifyOnRelease) if (gAppPtr->GetOverCPU()==this) if (mNotify)

	if (mReleaseSound) mReleaseSound->Play();
	//if (mNotifyOnRelease &&Rect(0,0,mWidth,mHeight).ContainsPoint(x,y))
	if (mNotifyOnRelease && (mAlwaysNotifyOnRelease || Rect(0,0,mWidth,mHeight).ContainsPoint(x,y)))
	{
		mNotifyData.mParam=HELPER_NOTIFY_RELEASE;
		if (!mSkipNotify)
		{
			if (mExe) mExe(this);
			if (mNotify) mNotify->Notify(this);
		}
	}
}

void Button::DragStart(int x, int y)
{
	if (mNotifyOnDrag) if (mNotify)
	{
		mNotifyData.mParam=HELPER_NOTIFY_DRAG;
		if (!mSkipNotify)
		{
			if (mExe) mExe(this);
			mNotify->Notify(this);
		}
		mDragging=false;
		*mIsDown=false;
	}
}



void Button::Enter()
{
	if (mHiliteSound) mHiliteSound->Play();
	mIsOver=true;
}

void Button::Leave()
{
	mIsOver=false;
	*mIsDown=false;
}

void Button::SetSprites(Array<Sprite> &theArray)
{
	//mSprite.Reset();
	//mSprite.GuaranteeSize(theArray.Size(),true);
	
#ifndef LEGACY_GL
	mSprite.ForceSize(theArray.Size());
	mSprite.mLockSize=true;
	for (int aCount=0;aCount<theArray.Size();aCount++) mSprite[aCount]=&theArray[aCount];
#else
	if (theArray.Size()==2) SetSprites(theArray[0],theArray[1]);
	if (theArray.Size()>=3) SetSprites(theArray[0],theArray[1],theArray[2]);
#endif

	EnableDraw();
	Autosize();
}

int	Button::GetDrawState(int theButtonStates)
{
	if (theButtonStates==1) return 0;
	if (theButtonStates==2)
	{
		if (IsDown()) return 1;
		return 0;
	}
	if (theButtonStates==3)
	{
		if (IsDown()) return 1;
		if (IsOver()) return 2;
		return 0;
	}

	return 0;
}

void Button::SetSprites(Point thePos, Array<Sprite> &theArray,bool thePosIsCenter)
{
	mX=thePos.mX;
	mY=thePos.mY;
	mWidth=(float)theArray[0].mWidth;
	mHeight=(float)theArray[0].mHeight;

	SetSprites(theArray);

	if (thePosIsCenter)
	{
		mX-=mWidth/2;
		mY-=mHeight/2;
	}
	Autosize();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// CheckBox
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CheckBox::TouchStart(int x, int y)
{
	bool aHold=!(*mIsDown);
	*mIsDown=false;
	mSkipNotify=true;
	Button::TouchStart(x,y);
	mSkipNotify=false;
	*mIsDown=aHold;
	mHolding=false;

	if (mNotify)
	{
		mNotifyData.mParam=HELPER_NOTIFY_PRESS;
		if (mExe) mExe(this);
		mNotify->Notify(this);
	}
}

void CheckBox::Leave()
{
	mIsOver=false;
}

void CheckBox::Update()
{
	if (mHolding) return;
	if (mBinding&gBind_To_Keyboard)
	{
		if (gAppPtr->GetFocusKeyboard()==this || !gAppPtr->GetFocusKeyboard())
		{
			if (IsKeyPressed(mKey[0])) if (mKey[1]==-1 || IsKeyDown(mKey[1])) if (mKey[2]==-1 || IsKeyDown(mKey[2])) 
			{
				bool aHold=*mIsDown;
				TouchStart(0,0);
				TouchEnd(0,0);
				*mIsDown=!aHold;
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Cycler
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Cycler::TouchStart(int x, int y)
{
	*mIsDown=false;
	mSkipNotify=true;
	Button::TouchStart(x,y);
	mSkipNotify=false;
	mState++;
#ifndef LEGACY_GL
	if (mState>=mSprite.Size()) mState-=mSprite.Size();
#else
	if (mSprite[mState]==NULL) mState=0;
#endif
	*mIsDown=false;
	mHolding=false;
	
	if (mNotify)
	{
		mNotifyData.mParam=HELPER_NOTIFY_PRESS;
		if (mExe) mExe(this);
		mNotify->Notify(this);
	}
}

void Cycler::Leave()
{
	mIsOver=false;
}

void Cycler::Update()
{
	if (mHolding) return;
	if (mBinding&gBind_To_Keyboard)
	{
		if (gAppPtr->GetFocusKeyboard()==this || !gAppPtr->GetFocusKeyboard())
		{
			if (IsKeyPressed(mKey[0])) if (mKey[1]==-1 || IsKeyDown(mKey[1])) if (mKey[2]==-1 || IsKeyDown(mKey[2])) 
			{
				bool aHold=*mIsDown;
				TouchStart(0,0);
				TouchEnd(0,0);
				*mIsDown=!aHold;
			}
		}
	}
}

void Cycler::Draw()
{
	if (!mDrawCentered) mSprite[mState]->DrawScaled(Center()-UpperLeft(),mSpriteScale);
	else mSprite[mState]->DrawScaled(mWidth/2,mHeight/2,mSpriteScale);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Slider
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Slider::Slider()
{
	mID=CPUID_SLIDER;
	mBorder=0;

	*this+=&mThumb;
	mThumb.SetNotify(this);
	mThumb.SetNotifyConditions(true,true,true,false,false);
	mThumb.SetCursor(CURSOR_HAND);
	mThumb.mAlwaysNotifyOnRelease=true;
	mBackground=NULL;
	mIsRepeater=false;

	DisableFeature(CPU_UPDATE);
	DisableFeature(CPU_DRAW);

	SetValue(0);
}

void Slider::SetBorder(float theBorderWidth)
{
	mBorder=theBorderWidth;
	SetValue();
}

void Slider::Size(Point thePos, float theWidth)
{
	mX=thePos.mX;
	mY=thePos.mY;

	if (!mBackground)
	{
		gOut.Out("Slider::Size(Point thePos, float theWidth) called without setting a sprite first!  Height of slider unknown!");
		mWidth=theWidth;
		mHeight=10;
		return;
	}

	mWidth=theWidth;
	mHeight=(float)mBackground->mHeight;
}

void Slider::Draw()
{
	if (mSDrawHook) {mSDrawHook(this);return;}
	if (mSimple)
	{
		gG.SetColor(mSimple_BackgroundColor);
		gG.FillRect(0,0,mWidth,mHeight,_min(mWidth/2,mHeight/2));
		gG.SetColor(mSimple_ThumbColor[mThumb.IsDown()]);
		Rect aTRect=mThumb.Expand(-3);
		gG.FillRect(aTRect,_min(aTRect.mWidth/2,aTRect.mHeight/2));

		//gG.SetColor(0,0,0,1);gG.TempFont()->Center(Sprintf("%f",GetValue()),aTRect.Center());

		gG.SetColor();

		return;
	}
	if (mBackground) 
	{
		if (!mIsRepeater) mBackground->Center(Center()-UpperLeft());
		else mBackground->DrawTelescopedH(0,0,mWidth);
	}
}

void Slider::DrawOverlay()
{
	// This actually needs to go above EVERYTHING...
	if (mDrawThumbHook) if (mThumb.IsDown()) gAppPtr->ToolTip(this);
}

void Slider::DrawToolTip()
{
	if (mDrawThumbHook)
	{
		gG.PushTranslate();
		gG.Translate(mThumb.Center());
		mDrawThumbHook(this);
		gG.PopTranslate();
	}
}

void Slider::ChangeValue(float theStep) 
{
	SetInternalValue(gMath.Clamp(mInternalValue+theStep,0.0f,1.0f));
	*mValue=GetValue();

	if (mExe) mExe(this);
	if (mNotify) mNotify->Notify(this);
}

void Slider::Notify(void *theData)
{
	if (theData==&mThumb)
	{
		if (mThumb.mNotifyData.mParam==HELPER_NOTIFY_PRESS) mThumbDragOffset=gAppPtr->GetTouchPositionF()-UpperLeft()-mThumb.Center();
		if (mThumb.mNotifyData.mParam==HELPER_NOTIFY_MOVE)
		{
			float aWorkValue=(gAppPtr->GetTouchPositionF().mX-mThumbDragOffset.mX)-mX;
			aWorkValue-=mBorder;
			float aWidth=mWidth-(mBorder*2);
			SetInternalValue(gMath.Clamp(aWorkValue/aWidth,0.0f,1.0f));
			if (mExe) mExe(this);
			if (mNotify) mNotify->Notify(this);
		}
		if (mThumb.mNotifyData.mParam==HELPER_NOTIFY_RELEASE) {if (mReleaseHook) mReleaseHook(this);}
	}
}

Slider* Slider::SetValue(float theValue)
{
	//mValue=theValue;
	mInternalValue=gMath.ReverseInterpolate(mMinMax.mX,theValue,mMinMax.mY);
	*mValue=GetValue();

	//
	// Position the thumb...
	//
	float aWidth=mWidth-(mBorder*2);
	aWidth*=mInternalValue;

	mThumb.mX=mBorder+aWidth;
	mThumb.mX-=mThumb.mWidth/2;

	return this;
}

Slider* Slider::SetInternalValue(float theValue)
{
	//mValue=theValue;
	mInternalValue=theValue;
	*mValue=GetValue();

	//
	// Position the thumb...
	//
	float aWidth=mWidth-(mBorder*2);
	aWidth*=mInternalValue;

	mThumb.mX=mBorder+aWidth;
	mThumb.mX-=mThumb.mWidth/2;

	return this;
}

Slider* Slider::SetValue()
{
	//
	// Position the thumb...
	//
	float aWidth=mWidth-(mBorder*2);
	aWidth*=mInternalValue;
	*mValue=GetValue();


	mThumb.mX=mBorder+aWidth;
	mThumb.mX-=mThumb.mWidth/2;

	return this;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Ticker
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Ticker::Ticker()
{
	SetFeatures(CPU_UPDATE);

	mTick=10;
	mTickCounter=0;
	mStateMax=1;
	mState=false;
	mIsChanged=true;
}

void Ticker::Update()
{
	mTickCounter++;
	if (mTickCounter>=mTick)
	{
		mTickCounter=0;
		mState++;
		if (mState>mStateMax) mState=0;
		mIsChanged=true;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// SwipeBox
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SwipeBox::SwipeBox()
{
	mID=CPUID_SWIPEBOX;
	EnableFeature(CPU_CLIP);
	mIsDown=false;
	mSwipeWidth=0;
	mSwipeHeight=0;
	mFriction=.95f;
	mSwipeSpeedMod=1.0f;
	mReverseMouse=false;
	mCanScroll=0;
	mHitThreshold=false;
	mSwipeThreshold=0;
	if (OS_Core::IsTouchDevice()) mSwipeThreshold=5;

	SetCursor(CURSOR_HAND);

	mScrollX=0;
	mScrollY=0;
	mSpinSpeed=25;

	mScrollLock=Point(1,1);
}

void SwipeBox::Core_Update()
{
	CPU::Core_Update();

	mSwipeSpeed*=mScrollLock;
	if (mSwipeSpeed.mX || mSwipeSpeed.mY) SetPos(mScrollX+mSwipeSpeed.mX,mScrollY+mSwipeSpeed.mY);
	//if (!gAppPtr->IsTouchDevice()) mSwipeSpeed.Null(); // On non-touch devices, scroll with the mouse steadily...
	mSwipeSpeed*=mFriction;

}

void SwipeBox::SetPos(float theX, float theY)
{
	float aOldX=mScrollX;
	float aOldY=mScrollY;

	mScrollX=theX;
	mScrollY=theY;

	Clamp();

	float aMovedX=mScrollX-aOldX;
	float aMovedY=mScrollY-aOldY;

	//
	// Move any child objects by this amount!
	//

	if (mCPUManager)
	{
		EnumList(CPU,aCPU,(*mCPUManager))
		{
			aCPU->mX-=aMovedX;
			aCPU->mY-=aMovedY;
		}
	}
}

Rect SwipeBox::GetVisibleArea()
{
	return Rect(mScrollX,mScrollY,mWidth,mHeight);
}



void SwipeBox::Clamp()
{
	if (mSwipeWidth<mWidth) mScrollX=0;
	else mScrollX=gMath.Clamp(mScrollX,0.0f,mSwipeWidth-mWidth);
	if (mSwipeHeight<mHeight) mScrollY=0;
	else mScrollY=gMath.Clamp(mScrollY,0.0f,mSwipeHeight-mHeight);
}

void SwipeBox::SwipeAreaResized()
{
	SetCursor(CURSOR_NORMAL);
	mCanScroll=0;
	if (mSwipeWidth>mWidth) {mCanScroll|=0x02;SetCursor(CURSOR_HAND);}
	if (mSwipeHeight>mHeight) {mCanScroll|=0x01;SetCursor(CURSOR_HAND);}
	SetPos(mScrollX,mScrollY);
}

void SwipeBox::Core_Draw()
{
	if (mFeatures&CPU_DRAW)
	{
		if (mFeatures&CPU_CLIP)
		{
			gG.PushClip();
			gG.PushFreezeClip();

			gG.Clip(mX,mY,mWidth,mHeight);
			gG.FreezeClip(true);
		}

		gG.PushTranslate();

		if (mFeatures&CPU_CENTERED) gG.Translate(Center());
		else gG.Translate(UpperLeft());

		gG.PushTranslate();
		Core_DrawUnder();

		//gG.PushTranslate();
		gG.Translate(-mScrollX,-mScrollY);
		gG.Translate(mDrawOffset);
		Draw();
		gG.Translate(-mDrawOffset);
		//gG.PopTranslate();
		gG.Translate(mScrollX,mScrollY);

		//
		// Objects draw in the same place...
		//
		gG.Translate(mDrawOffset);
		if (mCPUManager) mCPUManager->Draw();
		gG.Translate(-mDrawOffset);

		gG.Translate(-mScrollX,-mScrollY);

		DrawOverlay();

		if (mFeatures&CPU_CLIP) 
		{
			gG.PopFreezeClip();
			gG.PopClip();
		}

		gG.PopTranslate();
		gG.Translate(mDrawOffset);
		DrawOver();
		gG.Translate(-mDrawOffset);
		gG.PopTranslate();
	}
}

void SwipeBox::Core_DrawUnder()
{
/*
	gG.SetColor(.5f);
	gG.FillRect(0,0,mWidth,mHeight);
	gG.SetColor();
/**/
	gG.Translate(mDrawOffset);
	DrawUnder();
	gG.Translate(-mDrawOffset);
}


void SwipeBox::Core_TouchStart(int x, int y)
{
	mHitThreshold=false;
	mSwipeAnchor=Point(x,y);
	mIsDown=true;
	mAbortSwipe=false;
	CPU::Core_TouchStart(x,y);
}

void SwipeBox::Core_TouchMove(int x, int y)
{
	gIgnoreNextTouchEnd=false;
	if (mIsDown && !mAbortSwipe)
	{
		if (!mHitThreshold)
		{
			if (gMath.DistanceSquared(mSwipeAnchor,Point(x,y))>mSwipeThreshold*mSwipeThreshold) mHitThreshold=true;
			else return;
		}
		
		mSwipeSpeed=(mSwipeAnchor-Point(x,y))*mSwipeSpeedMod;
		//if (gAppPtr->IsTouchDevice()) mSwipeSpeed*=.3f; // On a touch device, decrease thing, since we have friction scrolling...
		if (mReverseMouse) mSwipeSpeed*=-1;
		mSwipeAnchor=Point(x,y);
	}

	CPU::Core_TouchMove(x,y);
}

void SwipeBox::Core_TouchEnd(int x, int y)
{
	mIsDown=false;
	mAbortSwipe=false;
	CPU::Core_TouchEnd(x,y);
}

Point SwipeBox::ConvertVisibleToWorld(float theX, float theY)
{
	float aX=theX;
	float aY=theY;
	aX+=mScrollX;
	aY+=mScrollY;
	return Point(aX,aY);
}

Point SwipeBox::ConvertWorldToVisible(float theX, float theY)
{
	float aX=theX;
	float aY=theY;
	aX-=mScrollX;
	aY-=mScrollY;
	return Point(aX,aY);
}

void SwipeBox::Spin(int theDir)
{
	Point aDir=-(float)gMath.Sign(theDir)*mSpinDir;
	aDir*=mSpinSpeed;
	SetPos(Point(mScrollX+aDir.mX,mScrollY+aDir.mY));
}

void SwipeBox::GuaranteeVisible(float theX, float theY)
{
	Rect aVis=GetVisibleArea();
	if (aVis.ContainsPoint(theX,theY)) return;

	float aWantX=mScrollX;
	float aWantY=mScrollY;
	if (theX<aVis.UpperRight().mX)
	{
		aWantX=theX;
	}
	if (theX>aVis.LowerRight().mX)
	{
		float aStep=theY-aVis.LowerRight().mY;
		aWantX=mScrollX+aStep;
	}

	if (theY<aVis.UpperRight().mY)
	{
		aWantY=theY;
	}
	if (theY>aVis.LowerRight().mY)
	{
		float aStep=theY-aVis.LowerRight().mY;
		aWantY=mScrollY+aStep;
	}

	SetPos(aWantX,aWantY);

}


/*
CPU* SwipeBox::HitTest(float x, float y)
{
	if (mFeatures&CPU_TOUCH)
	{
		if (mCPUManager) 
		{
			Point aWork=Point(x,y);

			if (mFeatures&CPU_CENTERED) aWork-=Center();
			else aWork-=UpperLeft();

			aWork=ConvertVisibleToWorld(aWork);

			CPU *aSubCPU=mCPUManager->Pick(aWork.mX,aWork.mY);
			if (aSubCPU) return aSubCPU;
		}
		if (ContainsPoint(x,y)) return this;
	}
	return NULL;
}
*/

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Swipe List
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//
// It's templated now!
//


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// TextBox
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

TextBox::TextBox()
{
	mID=CPUID_TEXTBOX;
	mFont=NULL;
	mText=&mRealText;
	mBackgroundColor=Color(0,0,0,1);
	mTextColor=Color(1);
	mBackgroundSprite=NULL;
	mBackgroundMagicSprite=NULL;
	mIsPassword=NULL;
	mForceUppercase=false;
	mJustify=-1;
	mXSprite=NULL;
	mMaxTextWidth=0;
	mClickClear=false;
	mMaxCharCount=0;
	mHasX=true;
	mMultiLine=false;
	mAllowEnter=false;
	mScrollY=0;
	mFilterInput=0;
	SetCursor(CURSOR_IBEAM);

	SetCursorWidth(3);

	mCursorBlink=new Ticker();
	mCursorBlink->SetTick((int)gAppPtr->SecondsToUpdates(.25f));
	mCursorBlink->SetState(true);
	*this+=mCursorBlink;
	mBorder=0;

	mCursor=0;
	mOldCursor=0;
	mScroll=0;

	mIsChanged=false;
}

TextBox::~TextBox()
{
	if (gAppPtr) gAppPtr->UnfocusKeyboard(this);
}

void TextBox::Initialize()
{
	mXButton.Size(-1,-1,-1,-1);
	if (mHasX)
	{
		mXButton.SetNotify(this);
		*this+=&mXButton;
	}

	if (mTotalConversion.IsNull()) AdjustTextArea();
}

void TextBox::AdjustTextArea()
{
	if (mSuffix.Len())
	{
		if (!mSuffixFont) mSuffixFont=mFont;
		float aSufW=mSuffixFont->WidthEX(mSuffix);
		//mBorder.mX+=(int)(aSufW/2);

		if (mJustify>0) mTextOffset.mX-=aSufW;
	}
}

void TextBox::InitializeTC()
{
	mHasX=false;
	switch(mTotalConversion->mID)
	{
	case TC_UPDATES: mTotalConversion->mLowHiStep*=gAppPtr->SecondsToUpdates(1); // Here because we can't ref gAppPtr in rapt_CPUHelpers.h
	case TC_NUMBERF:
		{
			Numeric();
			//CenterText();
			RightJustify();
			mBorder.mX+=(int)mHeight;

			//mTotalConversion->mButtonList.GuaranteeSize(2);
			mTotalConversion->mButtonList[0]=new RepeatingButton;mTotalConversion->mButtonList[0]->Size(-mStroke.mX,-mStroke.mY,mHeight+mStroke.mX*2,mHeight+(mStroke.mY*2));
			mTotalConversion->mButtonList[1]=new RepeatingButton;mTotalConversion->mButtonList[1]->Size(mWidth-mHeight-mStroke.mX,-mStroke.mY,mHeight+mStroke.mX*2,mHeight+(mStroke.mY*2));
			foreach(aButton,mTotalConversion->mButtonList)
			{
				aButton->SetNotify(this);
				aButton->NotifyOnPress();
				if (mTotalConversion->mSoundList.Size()>0) aButton->SetSounds(mTotalConversion->mSoundList[0]);
				*this+=aButton;
			}
			TCSetText();
		}
		break;
	}

	AdjustTextArea();
}

void TextBox::TCSetText()
{
	switch(mTotalConversion->mID)
	{
	case TC_NUMBERF:
		{
			float* aPtr=(float*)mTotalConversion->mPtr;
			*aPtr=_clamp(mTotalConversion->mLowHiStep.mX,*aPtr,mTotalConversion->mLowHiStep.mY);
			SetText(Sprintf(mTotalConversion->mFormat.c(),*aPtr));
			break;
		}
	case TC_UPDATES:
		{
			int* aPtr=(int*)mTotalConversion->mPtr;
			*aPtr=_clamp((int)mTotalConversion->mLowHiStep.mX,*aPtr,(int)mTotalConversion->mLowHiStep.mY);
			SetText(Sprintf(mTotalConversion->mFormat.c(),((float)*aPtr)/gAppPtr->SecondsToUpdates(1)));
			break;
		}
	}
}


void TextBox::LostKeyboardFocusTC()
{
	switch(mTotalConversion->mID)
	{
	case TC_NUMBERF:
		{
			float aValue=GetText().ToFloat();
			aValue=_clamp(mTotalConversion->mLowHiStep.mX,aValue,mTotalConversion->mLowHiStep.mY);
			float* aPtr=(float*) mTotalConversion->mPtr;*aPtr=aValue;
			TCSetText();
			//SetText(Sprintf(mTotalConversion->mFormat.c(),*aPtr));
			break;
		}
	case TC_UPDATES:
		{
			int aValue=(int)(GetText().ToFloat()*gAppPtr->SecondsToUpdates(1));
			aValue=_clamp((int)mTotalConversion->mLowHiStep.mX,aValue,(int)mTotalConversion->mLowHiStep.mY);
			int* aPtr=(int*)mTotalConversion->mPtr;*aPtr=aValue;
			TCSetText();
			break;
		}

	}
}

void TextBox::DrawTC()
{
	switch(mTotalConversion->mID)
	{
	case TC_NUMBERF:
	case TC_UPDATES:
		{
			gG.PushClip();
			int aBNo=0;
			foreach(aButton,mTotalConversion->mButtonList)
			{
				gG.Clip(*aButton);
				gG.SetColor(mTextColor);
				Rect aR=Rect(-mStroke.mX,-mStroke.mY,mWidth+(mStroke.mX*2),mHeight+(mStroke.mY*2));
				gG.FillRect(aR,mBevel);
				gG.SetColor(mBackgroundColor);
				float aXOff=(aBNo==0)?1.0f:-1.0f;
				float aYOff=aButton->IsDown()*2.0f;
				gG.FillRect(aButton->mX+4+aXOff,(aButton->mY+aButton->mH/2)-3+aYOff,aButton->mW-8,6);
				if (aBNo==1) gG.FillRect((aButton->mX+aButton->mW/2)-3+aXOff,aButton->mY+4+aYOff,6,aButton->mH-8);

				aBNo++;
			}
			gG.PopClip();
		}
	break;
	}
}

void TextBox::NotifyTC(void* theData)
{
	bool aChanged=false;
	switch(mTotalConversion->mID)
	{
	case TC_NUMBERF:
		{
			float aValue=GetText().ToFloat();
			if (theData==mTotalConversion->mButtonList[0]) {aValue-=mTotalConversion->mLowHiStep.mZ;aChanged=true;}
			else if (theData==mTotalConversion->mButtonList[1]) {aValue+=mTotalConversion->mLowHiStep.mZ;aChanged=true;}
			aValue=_clamp(mTotalConversion->mLowHiStep.mX,aValue,mTotalConversion->mLowHiStep.mY);
			float* aPtr=(float*) mTotalConversion->mPtr;*aPtr=aValue;

			TCSetText();
			break;
		}
	case TC_UPDATES:
		{
			int aValue=(int)(GetText().ToFloat()*gAppPtr->SecondsToUpdates(1));
			if (theData==mTotalConversion->mButtonList[0]) {aValue-=(int)mTotalConversion->mLowHiStep.mZ;aChanged=true;}
			else if (theData==mTotalConversion->mButtonList[1]) {aValue+=(int)mTotalConversion->mLowHiStep.mZ;aChanged=true;}
			aValue=_clamp((int)mTotalConversion->mLowHiStep.mX,aValue,(int)mTotalConversion->mLowHiStep.mY);
			int* aPtr=(int*) mTotalConversion->mPtr;*aPtr=aValue;
			TCSetText();
			break;
		}
	}

	if (aChanged) if (mExe) mExe(this);
}

void TextBox::Notify(void* theData)
{
	if (theData==&mXButton) 
	{
		SetText("");
		mIsChanged=true;
		FocusKeyboard();
	}

	if (mNotify)
	{
		mNotifyData.mParam=HELPER_NOTIFY_CLEAR;
		if (mExe) mExe(this);
		mNotify->Notify(this);
	}

	if (mTotalConversion.IsNotNull()) NotifyTC(theData);
}


void TextBox::Update()
{
	if (mHasX) 
	{
		if (mXSprite) mXButton.Size(mWidth-mXSprite->mWidth-mBorder.mX,0,mXSprite->WidthF(),mHeight);
		else mXButton.Size(mWidth-20-mBorder.mX,0,20,mHeight);
	}

	if (gAppPtr->GetFocusKeyboard()==this)
	{
		if (gAppPtr->IsTouchDevice())
		{
			//
			// If we're a touch device, then if we're focused, scroll everything up
			// if we're under the keyboard.
			//
		}
		if (mCursor!=mOldCursor)
		{
			//
			// If cursor is off our rect, scroll...
			//
			mOldCursor=mCursor;
			mCursorBlink->SetState(true);

			Rect aRect=GetDrawCursorRect();
			if (aRect.mX<mX+mBorder.mX) mScroll+=aRect.mX-(mX+mBorder.mX);
			if (aRect.mX>mX+mWidth-3-mBorder.mX-(mHasX*15)) mScroll+=aRect.mX-(mX+mWidth-3-mBorder.mX-(mHasX*15));

			if (mScroll<0) mScroll=0;
		}
	}
}


void TextBox::Draw()
{
	if (mTotalConversion.IsNotNull() && mTotalConversion->mID&TC_STARTUP) {UNSETBITS(mTotalConversion->mID,TC_STARTUP);InitializeTC();}

	if (mMultiLine) {MLDraw();return;}
	if (mSharpen)
	{
		gG.PushTranslate();
		gG.TranslateToIntegers();
		gG.Sharpen(true);
	}

	gG.PushColor();
	gG.SetColor(mBackgroundColor);
	if (mBackgroundSprite) mBackgroundSprite->DrawTelescoped(0,0,mWidth,mHeight);
	else if (mBackgroundMagicSprite) mBackgroundMagicSprite->DrawMagicBorderH(0,0,mWidth);
	else if (mBackgroundColor.mA) gG.FillRect(-mStroke.mX,-mStroke.mY,mWidth+(mStroke.mX*2),mHeight+(mStroke.mY*2),mBevel);
	Point aPos=GetDrawTextPos()-UpperLeft();
	if (mFont)
	{
		gG.PushClip();
		Rect aWantClip=Rect((float)mBorder.mX,0.0f,mWidth-(mBorder.mX*2)-(mHasX*15),mHeight);
		Rect aRealRect=ConvertRectToScreen();
		aWantClip.mX+=aRealRect.mX;
		aWantClip.mY+=aRealRect.mY;
        aWantClip.mY+=gAppPtr->mVisibilityScroll;
		aWantClip.ClipInto(gG.GetClipRect());
		aWantClip.mX-=aRealRect.mX;
		aWantClip.mY-=aRealRect.mY;
        aWantClip.mY-=gAppPtr->mVisibilityScroll;
		gG.Clip(aWantClip);

		if (mText->Len()==0 && mGhostText.Len())
		{
			gG.SetColor(mTextColor,.25f);
			mFont->Draw(mGhostText,aPos);
		}
		if (mSuffix.Len())
		{
			gG.SetColor(mTextColor,.5f);
			mSuffixFont->Right(mSuffix,Point(aWantClip.mX+aWantClip.mWidth,aPos.mY));
		}
	
		gG.SetColor(mTextColor);
		if (mIsPassword)
		{
			String aString=mText->c();
			aString.SetAllChars('*');
			mFont->Draw(aString,aPos);
		}
		else mFont->Draw(*mText,aPos);
		if (gAppPtr->GetFocusKeyboard()==this && mCursorBlink->GetState())
		{
			Rect aRect=GetDrawCursorRect();
			aRect.mX-=mX;
			aRect.mY-=mY;
			gG.FillRect(aRect.Expand(mCursorTweak));
		}

		gG.PopClip();
	}
	if (mSharpen)
	{
		gG.PopTranslate();
		gG.Sharpen(false);
	}

	if (mHasX)
	{
		if (mXSprite) 
		{
			gG.SetColor(mTextColor,.5f);
			mXSprite->Center(mXButton.Center());
			gG.SetColor();
		}
		else
		{
			Point aXCenter=Point(mWidth-12-mBorder.mX,mHeight/2);
			gG.SetColor(mTextColor,.5f);
			gG.DrawLine(aXCenter+Point(-4,-4),aXCenter+Point(4,4));
			gG.DrawLine(aXCenter+Point(-4,4),aXCenter+Point(4,-4));
			gG.Translate(-1,0);
			gG.DrawLine(aXCenter+Point(-4,-4),aXCenter+Point(4,4));
			gG.DrawLine(aXCenter+Point(-4,4),aXCenter+Point(4,-4));
			gG.Translate(2,0);
			gG.DrawLine(aXCenter+Point(-4,-4),aXCenter+Point(4,4));
			gG.DrawLine(aXCenter+Point(-4,4),aXCenter+Point(4,-4));
			gG.Translate(-1,0);
		}
	}
	
	gG.PopColor();

	if (mTotalConversion.IsNotNull()) DrawTC();
}

void TextBox::GuaranteeCursorVisible(bool doFast)
{
	//if (!mDisplayText.Len()) return;

	Rect aCursor=GetDrawCursorRect();
	aCursor.mX-=mX;
	aCursor.mY-=mY;
	aCursor.mY-=mBorder.mY;
	
	if (doFast) 
	{
		while (aCursor.mY<-mScrollY) mScrollY+=mFont->mPointSize/10;
		while (aCursor.LowerLeft().mY>-mScrollY+(mHeight-mBorder.mY-mFont->mPointSize/3)) mScrollY-=mFont->mPointSize/10;
	}
	else 
	{
		if (aCursor.mY<-mScrollY) mScrollY+=mFont->mPointSize/10;
		if (aCursor.LowerLeft().mY>-mScrollY+(mHeight-mBorder.mY-mFont->mPointSize/3)) mScrollY-=mFont->mPointSize/10;
	}
}

void TextBox::MLDraw()
{
	GuaranteeCursorVisible();
	if (mSharpen)
	{
		gG.PushTranslate();
		gG.TranslateToIntegers();
		gG.Sharpen(true);
	}
	gG.PushColor();
	gG.SetColor(mBackgroundColor);
	if (mBackgroundSprite) mBackgroundSprite->DrawTelescoped(0,0,mWidth,mHeight);
	else if (mBackgroundMagicSprite) mBackgroundMagicSprite->DrawMagicBorderH(0,0,mWidth);
	else if (mBackgroundColor.mA) gG.FillRect(0,0,mWidth,mHeight,mBevel);
	Point aPos=GetDrawTextPos()-UpperLeft();
	if (mFont)
	{
		gG.PushTranslate();
		gG.PushClip();
		Rect aWantClip=Rect((float)mBorder.mX,0.0f,mWidth-(mBorder.mX*2)-(mHasX*15),mHeight);
		Rect aRealRect=ConvertRectToScreen();
		aWantClip.mX+=aRealRect.mX;
		aWantClip.mY+=aRealRect.mY;
		aWantClip.mY+=gAppPtr->mVisibilityScroll;
		aWantClip.ClipInto(gG.GetClipRect());
		aWantClip.mX-=aRealRect.mX;
		aWantClip.mY-=aRealRect.mY;
		aWantClip.mY-=gAppPtr->mVisibilityScroll;
		gG.Clip(aWantClip);

		gG.SetColor(mTextColor);
		gG.Translate(0.0f,mScrollY);

		if (mText->Len()==0 && mGhostText.Len())
		{
			gG.SetColor(mTextColor,.25f);
			mFont->Draw(mGhostText,aPos);
		}

		mFont->Draw(mDisplayText,aPos);
		if (gAppPtr->GetFocusKeyboard()==this && mCursorBlink->GetState())
		{
			Rect aRect=GetDrawCursorRect();
			aRect.mX-=mX;
			aRect.mY-=mY;
			aRect.mX=_max(mBorder.mX,aRect.mX);
			//aRect.mY=_max(mBorder.mY,aRect.mY);
			gG.FillRect(aRect);
		}
		gG.PopClip();
		gG.PopTranslate();
	}
	if (mSharpen)
	{
		gG.PopTranslate();
		gG.Sharpen(false);
	}
	gG.PopColor();
}

Point TextBox::GetDrawTextPos()
{
	if (mMultiLine && mFont)
	{
		return UpperLeft()+Point(mBorder.mX,-mBorder.mY)+mTextOffset+Point(0.0f,mFont->mPointSize);
	}
	else
	{
		switch (mJustify)
		{
		case -1: return LowerLeft()+Point(mBorder.mX,-mBorder.mY)+mTextOffset+Point(-mScroll,0.0f);
		case 0: return Point(mX+(mWidth/2)-(mFont->WidthEX(*mText)/2),mY+mHeight+mTextOffset.mY);
		case 1: return Point(mX+(mWidth-mBorder.mX)-(mFont->WidthEX(*mText))+mTextOffset.mX,mY+mHeight+mTextOffset.mY);
		default: return LowerLeft()+Point(mBorder.mX,-mBorder.mY)+mTextOffset+Point(-mScroll,0.0f); // Same as -1
		}

		//if (mJustify==0) return Point(mX+(mWidth/2)-(mFont->WidthEX(*mText)/2),mY+mHeight+mTextOffset.mY);//return (LowerLeft()+Point(mBorder.mX,-mBorder.mY)+mTextOffset+Point(-mScroll,0.0f))+Point((mWidth/2)-(mFont->WidthEX(*mText)/2),0.0f);
		//return LowerLeft()+Point(mBorder.mX,-mBorder.mY)+mTextOffset+Point(-mScroll,0.0f);
	}
}

Rect TextBox::GetDrawCursorRect()
{
	Point aPos=GetDrawTextPos();
	if (mMultiLine)
	{
		Point aCPos=mFont->GetCharacterPos(mDisplayText,mCursor);
		return Rect(_max(mX+mCursorWidth/2,aPos.mX+aCPos.mX-(mCursorWidth/2)+mCursorOffset.mX),aPos.mY+aCPos.mY-mFont->mPointSize+mCursorOffset.mY,mCursorWidth,mFont->mPointSize);
		//return Rect(aPos.mX+aCPos.mX-(mCursorWidth/2)+mCursorOffset.mX,aPos.mY+aCPos.mY-mFont->mPointSize+mCursorOffset.mY,mCursorWidth,mFont->mPointSize);
	}

	float aCX;
	if (mIsPassword)
	{
		String aString=mText->c();
		aString.SetAllChars('*');
		aCX=mFont->GetWidth(aString,mCursor);
	}
	else aCX=mFont->GetWidth(*mText,mCursor);

	return Rect(aPos.mX+aCX-(mCursorWidth/2)+mCursorOffset.mX,aPos.mY-mFont->mPointSize+mCursorOffset.mY,mCursorWidth,mFont->mPointSize);
	//return Rect(_max(mX+mCursorWidth/2,aPos.mX+aCX-(mCursorWidth/2)+mCursorOffset.mX),aPos.mY-mFont->mPointSize+mCursorOffset.mY,mCursorWidth,mFont->mPointSize);
}

void TextBox::LostUserFocus()
{
	FocusKeyboard(false);
}

void TextBox::GotKeyboardFocus()
{
	mIsChanged=false;
}

void TextBox::LostKeyboardFocus()
{
	if (mIsChanged) 
	{
		mNotifyData.mParam=HELPER_NOTIFY_LOSTFOCUS;
		if (mNotify) mNotify->Notify(this);
		if (mIsChanged) if (mExe) mExe(this);
		SaveChanges();
		if (mTotalConversion.IsNotNull()) LostKeyboardFocusTC();

		mIsChanged=false;
	}
}


void TextBox::Char(int theKey)
{
	//mLastCursorPos=mCursor;

	//
	// The defines for the system CHAR values used below are in OS_Headers.h
	//
	mClickClear=false;

	if (theKey==CHAR_BACKSPACE) // 8
	{
		if (mCursor>0) 
		{
#ifdef UTF8_PRIVATE_USE
			StringUTF8* aText=(StringUTF8*)mText;
			aText->DeleteUTF8(--mCursor);
#else
			mText->Delete(--mCursor,1);
#endif
			mIsChanged=true;
			ChangedText();
		}
		return;
	}

	//
	// Touch Devices don't get a "keydown" so process
	// enter here...
	//
/*	
	if (gAppPtr->IsTouchDevice())
	if (theKey==10 || theKey==13)
	{
		FocusKeyboard(false);
		if (mNotify) mNotify->Notify(this);
		return;
	}
*/ 

	if (mFilterInput)
	{
		if (mFilterInput&1) if (!mAllowString.Contains((char)theKey)) return;
		if (mFilterInput&2) if (mForbidString.Contains((char)theKey)) return;
	}

	if (mForceUppercase)
	{
		String aS=Sprintf("%c",theKey).ToUpper();
		theKey=aS[0];
	}

	if (mMaxTextWidth>0) 
	{
		String aS=Sprintf("%c",theKey);
		if (mFont->WidthEX(*mText)>mMaxTextWidth-mFont->Width(aS)) return;
	}
	if (mMaxCharCount>0) 
	{
#ifdef UTF8_PRIVATE_USE
		StringUTF8* aText=(StringUTF8*)mText;
		if (aText->LenUTF8()>=mMaxCharCount) return;
#else
		if (mText->Len()>=mMaxCharCount) return;
#endif
	}


	if (mFont)
	{
		if (mFont->IsChar(theKey) || (mAllowEnter && theKey==10))
		{
			mIsChanged=true;
#ifdef UTF8_PRIVATE_USE
			StringUTF8* aText=(StringUTF8*)mText;
			aText->InsertUTF8(theKey,mCursor);
#else
			mText->Insert(theKey,mCursor);
#endif
			mCursor++;
		}
	}

	ChangedText();
}

void TextBox::KeyDown(int theKey)
{
	//mLastCursorPos=mCursor;

	//
	// The defines for the system KEYDOWN values used below are in OS_Headers.h
	//
	mClickClear=false;
   
	switch (theKey)
	{
	case 13:if (mAllowEnter) {Char(10);break;}
	case 10:
		mNotifyData.mParam=HELPER_NOTIFY_ENTER;
		if (mNotify) mNotify->Notify(this);
		if (mExe) mExe(this);
		FocusKeyboard(false);
		SaveChanges();
		break;
	case 27:
		FocusKeyboard(false);
		mNotifyData.mParam=HELPER_NOTIFY_ESC;
		if (mNotify) mNotify->Notify(this);
		if (mExe) mExe(this);
		SaveChanges();
		break;
	case KEYDOWN_TAB:
		FocusKeyboard(false);
		mNotifyData.mParam=HELPER_NOTIFY_TAB;
		if (mNotify) mNotify->Notify(this);
		if (mExe) mExe(this);
		SaveChanges();
		break;
	case KEYDOWN_UPARROW:if (mMultiLine) MoveMLCursor(-1);break;
	case KEYDOWN_DOWNARROW:if (mMultiLine) MoveMLCursor(1);break;
    case KEYDOWN_LEFTARROW:
		mCursor=_max(0,mCursor-1);
		break;
    case KEYDOWN_RIGHTARROW:
		{
#ifdef UTF8_PRIVATE_USE
			StringUTF8* aText=(StringUTF8*)mText;
			mCursor=_min(mCursor+1,aText->LenUTF8());
#else
			mCursor=_min(mCursor+1,mText->Len());
#endif
		}
		break;
    case KEYDOWN_DELETE:
		{
#ifdef UTF8_PRIVATE_USE
			StringUTF8* aText=(StringUTF8*)mText;
			aText->DeleteUTF8(mCursor);
#else
			mText->Delete(mCursor,1);
#endif
		}
		mIsChanged=true;
		break;
    case KEYDOWN_END:
		if (mMultiLine)
		{
			Rect aRect=GetDrawCursorRect();
			aRect.mX-=mX;
			aRect.mY-=mY;
			aRect.mY+=mBorder.mY;
			aRect.mX+=mWidth;
			mCursor=PickMLCursor((int)aRect.CenterX(),(int)aRect.CenterY());
		}
		else 
		{
#ifdef UTF8_PRIVATE_USE
			StringUTF8* aText=(StringUTF8*)mText;
			mCursor=aText->LenUTF8();
#else
			mCursor=mText->Len();
#endif
		}
		break;
    case KEYDOWN_HOME:
		if (mMultiLine)
		{
			Rect aRect=GetDrawCursorRect();
			aRect.mX-=mX;
			aRect.mY-=mY;
			aRect.mY+=mBorder.mY;
			aRect.mX=0;
			mCursor=PickMLCursor((int)aRect.CenterX(),(int)aRect.CenterY());
		}
		else mCursor=0;
		break;
	case KEYDOWN_PAGEDOWN:
		{
#ifdef UTF8_PRIVATE_USE
			StringUTF8* aText=(StringUTF8*)mText;
			mCursor=aText->LenUTF8();
#else
			mCursor=mText->Len();
#endif
		}
		break;
	case KEYDOWN_PAGEUP:mCursor=0;break;
	}
	ChangedText();
}

void TextBox::MoveMLCursor(int theYDir)
{
	Rect aRect=GetDrawCursorRect();
	aRect.mX-=mX;
	aRect.mY-=mY;
	aRect.mY+=mBorder.mY;
	aRect.mY+=theYDir*mFont->mPointSize;

	if (aRect.mY>=0)
	{
		//aRect.mY+=mScrollY;
		Point aPos=GetDrawTextPos()-UpperLeft();
		Rect aTextRect=mFont->GetBoundingRect(mDisplayText,aPos).ExpandH(5);
		IPoint aPick=IPoint((int)aRect.CenterX(),(int)aRect.CenterY());
		if (aTextRect.ContainsPoint(aPick.mX,aPick.mY))
		{
			int aCursor=PickMLCursor(aPick.mX,aPick.mY);
			mCursor=aCursor;
		}
	}
	
	//mCursor+=theYDir;
}

#ifdef UTF8_PRIVATE_USE
int TextBox::PickMLCursor(int x, int y)
{
	Point aPos=GetDrawTextPos()-UpperLeft();
	int aCursor=0;
	int aCC=0;
	StringUTF8* aDisplayText=(StringUTF8*)&mDisplayText;

	int aCount=0;
	aDisplayText->StartUTF8();
	//for (int aCount=0;aCount<=aDisplayText->LenUTF8();aCount++)
	for (;;)
	{
		Point aTPos=aPos+mFont->CharacterPos(mDisplayText,aCount);
		if (y>=aTPos.mY-mFont->mPointSize && y<=aTPos.mY) if (x>=aTPos.mX) aCursor=aCC;

		int aChar=aDisplayText->GetUTF8();if (aChar==0) break;

		aCC++;
		aCount++;
	}
	return aCursor;
}
#else
int TextBox::PickMLCursor(int x, int y)
{
	Point aPos=GetDrawTextPos()-UpperLeft();
	int aCursor=0;
	int aCC=0;
	for (int aCount=0;aCount<=mDisplayText.Len();aCount++)
	{
		Point aTPos=aPos+mFont->CharacterPos(mDisplayText,aCount);
		if (y>=aTPos.mY-mFont->mPointSize && y<=aTPos.mY)
		{
			if (x>=aTPos.mX)
			{
				aCursor=aCC;
			}
		}
		aCC++;
	}
	return aCursor;
}
#endif

void TextBox::TouchStart(int x, int y)
{
	if (gAppPtr->GetFocusKeyboard()!=this) FocusKeyboard();

	if (mClickClear)
	{
		SetText("");
		mClickClear=false;
	}

	y-=(int)mScrollY;

	Point aPos=GetDrawTextPos()-UpperLeft();
	mCursor=0;
	//if (mCenterText) x+=mFont->mCharacter['I'].mWidth/2;

	if (x>=aPos.mX)
	{
		if (mMultiLine)
		{
			mCursor=PickMLCursor(x,y);
		}
		else if (mIsPassword)
		{
			String aString=mText->c();
			aString.SetAllChars('*');

			for (int aCount=0;aCount<=mText->Len();aCount++)
			{
				float aCX=mFont->GetWidth(aString,aCount);
				if (x>=aPos.mX+aCX)
				{
					mCursor=aCount;
				}
			}

		}
		else
		{
			for (int aCount=0;aCount<=mText->Len();aCount++)
			{
				float aCX=mFont->GetWidth(*mText,aCount);
				if (x>=aPos.mX+aCX)
				{
					mCursor=aCount;
				}
			}
		}
		if (mJustify==0) mCursor=_min(mCursor+1,mText->Len());
	}
}

void TextBox::ChangedText()
{
	if (mMultiLine) mDisplayText=mFont->Wrap(*mText,(float)(mWidth-(mBorder.mX*2)));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Key Grabber
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
KeyGrabber::KeyGrabber()
{
	mWaitForClear=true;
}

KeyGrabber::~KeyGrabber()
{
}

void KeyGrabber::Go(CPUHOOKPTR theHook)
{
	gAppPtr->AddCPU(this);
	Throttle(theHook,mAutoKill,true);
	//if (gAppPtr->IsTouchDevice()) ThrottleRelease(0);
}

/*
int KeyGrabber::Go()
{
	gAppPtr->AddCPU(this);
	if (gAppPtr->IsTouchDevice()) return -1;


	int aResult=Throttle();

	if (mAutoKill) Kill();
	else gAppPtr->RemoveCPU(this);

	return aResult;
}
*/

void KeyGrabber::Update()
{
	if (mWaitForClear)
	{
		bool aClear=true;
		for (int aCount=0;aCount<512;aCount++)
		{
			if (gInput.KeyPressed(aCount))
			{
				aClear=false;
				break;
			}
		}
		if (aClear) mWaitForClear=false;
	}
	else for (int aCount=0;aCount<512;aCount++)
	{
		if (gInput.KeyPressed(aCount))
		{
			ThrottleRelease(aCount);
			break;
		}
	}
}

void KeyGrabber::TouchStart(int x, int y)
{
	//if (gAppPtr->GetMouseButton()==-1) ThrottleRelease(KB_LEFTMOUSE);
	if (gAppPtr->GetMouseButton()==1) ThrottleRelease(KB_RIGHTMOUSE);
	else if (gAppPtr->GetMouseButton()==0) ThrottleRelease(KB_MIDDLEMOUSE);
	else ThrottleRelease(-1);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Joystick Button Grabber
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

JoyButtonGrabber::JoyButtonGrabber()
{
	mWaitForClear=true;
	mResult=-1;
}

JoyButtonGrabber::~JoyButtonGrabber()
{
}

void JoyButtonGrabber::Go(CPUHOOKPTR theHook)
{
	gAppPtr->AddCPU(this);
	Throttle(theHook,mAutoKill,true);
	if (gAppPtr->IsTouchDevice()) ThrottleRelease(0);
}

void JoyButtonGrabber::Update()
{
	if (mWaitForClear)
	{
		bool aClear=true;
		for (int aJCount=0;aJCount<gInput.GetJoystickCount();aJCount++)
		{
			for (int aCount=0;aCount<gInput.GetJoystickButtonCount(aJCount);aCount++)
			{
				if (IsJoystickButtonPressed(aJCount,aCount))
				{
					aClear=false;
					break;
				}
			}
		}
		if (aClear)
		{
			mWaitForClear=false;
			if (mResult!=-1) ThrottleRelease(mResult);
		}
	}
	else 
	{
		for (int aJCount=0;aJCount<gInput.GetJoystickCount();aJCount++)
		{
			for (int aCount=0;aCount<gInput.GetJoystickButtonCount(aJCount);aCount++)
			{
				if (IsJoystickButtonPressed(aJCount,aCount))
				{
					mResult=aCount;
					mWaitForClear=true;
					break;
				}
			}
		}
	}
}

void JoyButtonGrabber::TouchStart(int x, int y)
{
	//if (gAppPtr->GetMouseButton()==-1) ThrottleRelease(KB_LEFTMOUSE);
	if (gAppPtr->GetMouseButton()==1) ThrottleRelease(KB_RIGHTMOUSE);
	else if (gAppPtr->GetMouseButton()==0) ThrottleRelease(KB_MIDDLEMOUSE);
	else ThrottleRelease(-1);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// CPanel... much improved ControlPanel
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CPanel::CPanel()
{
	mScrollDir=0;
	mScrollCount=0;
	mMainRollout=NULL;
	mSyncState=0;
	mInstantScroll=false;
	mDefaultControlHeight=35;

	mVanishingRollout=NULL;
	mCurrentRollout=NULL;
	mCleanupRolloutList.Unique();

	EnableFeature(CPU_CLIP);

	mClickedControl=NULL;
}

CPanel::~CPanel()
{
	//
	// Cleanup: Make sure any panel we created gets killed off, too.
	//
	EnumList(CPanelRollout,aCPW,mCleanupRolloutList) *this+=aCPW;

}

void CPanel::TouchMove(int x, int y)
{
	if (mDraggable && mIsDown) 
	{
		mX+=gAppPtr->GetLastTouchMove().mX;mY+=gAppPtr->GetLastTouchMove().mY;
		if (mDragHook) {mDragState=0;mDragHook(this);}
	}
}

void CPanel::Kill()
{
	//
	// Unfocuses our keyboard right when we kill this panel, in case any callbacks or exes are attached to any children.
	// Put this in because TextBox with an EXE bound to it would lose keyboard focus when destroyed, and it would get destroyed
	// during the container's destruction.
	//
	EnumList(CPanelRollout,aRollout,mCleanupRolloutList) aRollout->UnfocusKeyboard();

	if (!mKill && mApplyChangesOnClose)
	{
		ApplyChanges();
		if (mApplyChangesHook) mApplyChangesHook(this,NULL);
	}
	CPU::Kill();
}

void CPanel::Initialize()
{
	AreDefaultsSet();

	mMainRollout=GetMainRollout();
	mMainRollout->Size(mWorkAreaRect);
	mMainRollout->BindSpinsV();
	mMainRollout->SetName(mName);
	mCurrentRollout=mMainRollout;
	*this+=mMainRollout;

	*this+=&mButton;
	mButton.SetNotify(this);

	RepositionButton(&mButton,0);

	if (mInitializeHook) mInitializeHook(this,NULL);

	if (mShrinkWrap)
	{
		mMainRollout->Rehup();
		mWorkAreaRect.mHeight=_min(mWorkAreaRect.mHeight,mMainRollout->mSwipeHeight);
		mHeight=mWorkAreaRect.mHeight+mWorkAreaRect.mY;
	}

}

void CPanel::SetName(String theName)
{
	mName=theName;
	if (GetMainRollout()) GetMainRollout()->SetName(theName);
}


CPanelRollout* CPanel::CreateRollout(String theName)
{
	CPanelRollout* aPanel=new CPanelRollout;
	aPanel->mCPanel=this;
	aPanel->mName=theName;
	if (mDraggable) aPanel->Ghost(); // If it's draggable then the rollout passes touches through

	mCleanupRolloutList+=aPanel;

	return aPanel;
}

CPanelRollout* CPanel::GetMainRollout()
{
	if (!mMainRollout)
	{
		mMainRollout=CreateRollout();
		mWorkAreaStack.Push(mMainRollout);

	}
	return mMainRollout;
}


void CPanel::Notify(void* theData)
{
	mClickedControl=NULL;
	gAppPtr->FocusKeyboard(NULL);

	if (theData==&mButton)
	{
		if (mWorkAreaStack.GetStackCount()<=1)
		{
			if (!mApplyChangesOnClose)
			{
				ApplyChanges();
				if (mApplyChangesHook) mApplyChangesHook(this,NULL);
			}
			PressedDoneButton();
			if (mPressedDoneButtonHook) mPressedDoneButtonHook(this,NULL);
			Kill();
		}
		else
		{
			GoToPreviousPanel();
		}
	}
}

void CPanel::Core_Update()
{
	if (mScrollCount>0)
	{
		float aScrollAmount=_min(mScrollCount,mWidth/20);
		if (mInstantScroll) {mScrollCount=aScrollAmount=mWidth;mInstantScroll=false;}

		mScrollCount-=aScrollAmount;

		for (int aCount=mWorkAreaStack.GetStackCount()-1;aCount>=0;aCount--)
		{
			mWorkAreaStack[aCount]->mX+=aScrollAmount*mScrollDir;
		}

		if (mScrollCount<=0)
		{
			*this-=mVanishingRollout;
			if (mWorkAreaStack.Peek()==mVanishingRollout) mWorkAreaStack.Pop();

			mButton.EnableTouch();
			RepositionButton(&mButton,0);

			if (mWorkAreaStack.GetStackCount()<=1) RepositionButton(&mButton,0);
			else RepositionButton(&mButton,1);

			mVanishingRollout=NULL;
		}
	}

	CPU::Core_Update();
}

void CPanel::GoToNextPanel(CPanelRollout* thePanel)
{
	if (mScrollCount>0) return;
	gAppPtr->FocusKeyboard(NULL);


	mVanishingRollout=mWorkAreaStack.Peek();
	mWorkAreaStack.Push(thePanel);
	thePanel->BindSpinsV();
	*this+=thePanel;
	thePanel->Size(mWorkAreaRect);
	thePanel->mX+=mWidth;

	mScrollDir=-1;
	mScrollCount=mWidth;
	mButton.DisableTouch();

	SwitchToRollout(thePanel,mVanishingRollout);
	mCurrentRollout=thePanel;

}

void CPanel::GoToPreviousPanel()
{
	if (mScrollCount>0) return;
	gAppPtr->FocusKeyboard(NULL);

	//
	// Need to pop and then push to keep the vanishing
	// panel in the display area while still getting
	// the panel we want to appear.
	//
	mVanishingRollout=mWorkAreaStack.Pop();
	*this+=mWorkAreaStack.Peek();
	SwitchToRollout(mWorkAreaStack.Peek(),mVanishingRollout);
	mCurrentRollout=mWorkAreaStack.Peek();

	mWorkAreaStack.Peek()->BindSpinsV();
	mWorkAreaStack.Push(mVanishingRollout);

	mScrollDir=1;
	mScrollCount=mWidth;
	mButton.DisableTouch();
}

void CPanel::Core_Draw()
{
	gG.PushClip();
	if (mFeatures&CPU_DRAW)
	{
		if (mFeatures&CPU_CLIP)
		{
			gG.PushClip();
			gG.PushFreezeClip();

			gG.Clip(mX,mY,mWidth,mHeight);
			gG.FreezeClip();
		}
		gG.PushTranslate();
		gG.PushColor();

		if (mFeatures&CPU_CENTERED) gG.Translate(Center());
		else gG.Translate(UpperLeft());
		DISPATCHPROCESS(mForwardDraw,Draw());
		if (mCPUManager) mCPUManager->Draw();
		DISPATCHPROCESS(mForwardDraw,DrawOverlay());

		//
		// For these external draws, we clip so that we can draw infinitely up and down,
		// but clipped horizontally (because things slide back and forth)
		//
		gG.PushClip();
		gG.PushFreezeClip();
		gG.FreezeClip(false);
		gG.Clip(0,-10000,mWidth,mHeight+20000);

		for (int aCount=mWorkAreaStack.GetStackCount()-1;aCount>=0;aCount--)
		{
			gG.PushTranslate();
			Point aCenter=Point(mWidth/2,mHeight/2)-mWorkAreaStack[aCount]->Center();
			gG.Translate(-aCenter.mX,0.0);
			DrawPanelName(mWorkAreaStack[aCount]->GetName());
			if (aCount==0) DrawPanelButton(0,&mButton);
			else DrawPanelButton(1,&mButton);
			gG.PopTranslate();
		}

		gG.PopFreezeClip();
		gG.PopClip();




		gG.PopColor();
		gG.PopTranslate();
		if (mFeatures&CPU_CLIP) 
		{
			gG.PopFreezeClip();
			gG.PopClip();
		}
	}
	gG.PopClip();

}

CPanelControl* CPanel::FindControl(String theName, char theSeperator)
{
	String aSearchFor=theName;

	List aSearchList;
	if (theName.Contains(theSeperator))
	{
		String aRolloutName=theName.GetToken(theSeperator);
		aSearchFor=theName.GetNextToken();
		EnumList(CPanelRollout,aR,mCleanupRolloutList) if (aR->mName==aRolloutName) aSearchList+=aR;
	}
	else aSearchList+=mCleanupRolloutList;

	EnumList(CPanelRollout,aR,aSearchList)
	{
		CPanelControl* aC=aR->FindControl(aSearchFor);
		if (aC) return aC;
		
	}

	return NULL;
}

CPanelControl* CPanel::FindControl(int theID)
{
	EnumList(CPanelRollout,aR,mCleanupRolloutList)
	{
		CPanelControl* aC=aR->FindControl(theID);
		if (aC) return aC;
	}
	return NULL;
}

CPanelRollout* CPanel::FindRollout(String theName, bool doCreate)
{
	EnumList(CPanelRollout,aR,mCleanupRolloutList)
	{
		if (aR->mName==theName) return aR;
	}

	if (!doCreate) return NULL;
	CPanelRollout* aR=CreateRollout(theName);
	return aR;
}


void CPanel::SetControlValue(String theName, SmartVariable theValue)
{
	CPanelControl* aControl=FindControl(theName);
	if (!aControl) 
	{
		gOut.Out("SetControlValue(\"%s\") ... control not found.",theName.c());
		return;
	}

	aControl->SetValue(theValue);
}

SmartVariable CPanel::GetControlValue(String theName)
{
	CPanelControl* aControl=FindControl(theName);
	if (!aControl) 
	{
		gOut.Out("GetControlValue(\"%s\") ... control not found.",theName.c());
		return 0;
	}

	return aControl->GetValue();
}


void CPanel::Sync(String theName, bool& theVariable)
{
	if (!mSyncState) SetControlValue(theName,theVariable);
	else theVariable=GetControlValue(theName);
}

void CPanel::Sync(String theName, char& theVariable)
{
	if (!mSyncState) SetControlValue(theName,(int)theVariable);
	else theVariable=(char)(int)GetControlValue(theName);
}

void CPanel::Sync(String theName, float& theVariable)
{
	if (!mSyncState) SetControlValue(theName,theVariable);
	else theVariable=GetControlValue(theName);
}

void CPanel::Sync(String theName, int& theVariable)
{
	if (!mSyncState) SetControlValue(theName,theVariable);
	else theVariable=GetControlValue(theName);
}

void CPanel::Sync(String theName, String& theVariable)
{
	if (!mSyncState) SetControlValue(theName,theVariable);
	else 
	{
		String aString=GetControlValue(theName);
		theVariable=aString;
	}
}

CPanelGroup* CPanel::GetCurrentGroup()
{
	return GetMainRollout()->GetCurrentGroup();
}

CPanelGroup* CPanel::StartGroup(String theName, bool showBackground,bool canClose, bool initiallyOpen)
{
	AreDefaultsSet();
	mMainRollout=GetMainRollout();
	CPanelGroup* aGroup=mMainRollout->StartGroup(theName,showBackground,canClose,initiallyOpen);
	return aGroup;
}










////////////////////////////////////////////////////////////////////////////////////////////
//
// Work Area...
//
////////////////////////////////////////////////////////////////////////////////////////////

CPanelRollout::CPanelRollout()
{
	mCurrentGroup=NULL;
	mMustSeeGroup=NULL;
}

CPanelRollout::~CPanelRollout()
{
	Reset();
}

void CPanelRollout::UnfocusKeyboard()
{
	bool aUnfocus=false;
	EnumList(CPanelControl,aCPC,mControlList) if (aCPC->IsFocusKeyboard()) {aUnfocus=true;break;}
	if (aUnfocus) gAppPtr->FocusKeyboard(NULL);
}

void CPanelRollout::Reset()
{
	mCurrentGroup=NULL;

	EnumList(CPanelGroup,aG,mGroupList) *this-=&aG->mExpand;

	_FreeList(CPanelGroup,mGroupList);
	EnumList(CPanelControl,aCPC,mControlList) 
	{
		mControlList-=aCPC;
		*this-=aCPC;
		delete aCPC;

		EnumListRewind(CPanelControl);
	}
	mGroupList.Clear();
	mControlList.Clear();
}

void CPanelRollout::Initialize()
{
	//gOut.Out("$ Rollout... Rehup");
	Rehup();
}

void CPanelRollout::Update()
{
	if (mWantRehup) Rehup();
}

void CPanelRollout::Draw()
{
	EnumList(CPanelGroup,aG,mGroupList)
	{
		if (aG->mShowGroupBackground)
		{
			mCPanel->DrawGroupBox(aG->mName,aG->mRect,aG->mHiliteRect,aG);
			if (aG->IsOpen())
			{
				EnumList(CPanelControl,aCPC,aG->mControlList)
				{
					if (aCPC==aG->mControlList.FetchLast()) break;
					mCPanel->DrawControl_Border(Rect(aCPC->mReservedArea.mX,aCPC->mReservedArea.mY+aCPC->mReservedArea.mHeight,aCPC->mReservedArea.mWidth,1));
				}
			}
		}
	}
}

void CPanelRollout::Rehup()
{
	float aHoldX=mScrollX;
	float aHoldY=mScrollY;
	SetPos(0,0);

	mWantRehup=false;

	float aBorder=mCPanel->mPanelBorder;			// Borders on the CPanel
	float aGroupBorder=mCPanel->mControlSpacing;	// Borders at the top and bottom of groups

	float aXPos=aBorder;
	float aYPos=(aBorder/2);
	float aWidth=mWidth-(aBorder*2);

	EnumList(CPanelControl,aCPC,mControlList) *this-=aCPC;


	EnumList(CPanelGroup,aG,mGroupList)
	{
		aYPos+=mCPanel->mGroupSpacing;
		float aGroupNameWidth=0;
		float aGroupNameHeight=0;
		if (mCPanel->mFont) if (aG->mName.Len()>0) 
		{
			aGroupNameWidth=mCPanel->mFont->GetWidth(aG->mName);
			aGroupNameHeight=mCPanel->mFont->GetHeight(aG->mName);
		}

		aYPos+=aGroupNameHeight;

		aG->mRect=Rect(aXPos,aYPos,aWidth,1);

		if (aG->mIsOpen)
		{
			//
			// If group is closed, we skip all this...
			//

			float aYTop=aYPos;
			EnumList(CPanelControl,aCPC,aG->mControlList)
			{
				Rect aControlRect=Rect(aXPos,aYPos,aWidth,aCPC->GetHeight()+aGroupBorder);

				aCPC->mX=aXPos;
				aCPC->mWidth=aWidth;
				aCPC->mY=aControlRect.Center().mY-(aCPC->GetHeight()/2);
				aCPC->mReservedArea=aControlRect;

				if (aCPC==aG->mControlList[0])
				{
					aCPC->mReservedArea.mY-=aGroupBorder;
					aCPC->mReservedArea.mHeight+=aGroupBorder;
				}
				if (aCPC==aG->mControlList.Last()) aCPC->mReservedArea.mHeight+=aGroupBorder;

				*this+=aCPC;

				aYPos=aControlRect.LowerLeft().mY;

				if (aCPC!=aG->mControlList.Last()) 
				{
					aYPos++;
					//aYPos+=aGroupBorder/2;
				}

				aG->mRect=aG->mRect.Union(aControlRect);
			
				aCPC->mIsSizeChanged=true;
				aCPC->Rehup();
			}

			//
			// If we have alignments, align them!
			//
			if (aG->mAlignList.GetCount())
			{
				EnumList(CPanelGroup::AlignStruct,aA,aG->mAlignList) for (int aCount=0;aCount<aA->mControlList.Size();aCount++) 
				{
					aA->mControlList[aCount]->Draw();
				}
				EnumList(CPanelGroup::AlignStruct,aA,aG->mAlignList) 
				{
					float aAlignX=0;
					for (int aCount=0;aCount<aA->mControlList.Size();aCount++)
					{
						CPU* aAlignCPU=NULL;
						switch (aA->mControlList[aCount]->mID)
						{
							case CPANELCONTROL_TEXTBOX:
								{
									CPanelControl_Textbox* aC=(CPanelControl_Textbox*)aA->mControlList[aCount];
									aAlignCPU=(CPU*)&aC->mTextBox;
									break;
								}
							case CPANELCONTROL_SLIDER:
								{
									CPanelControl_Slider* aC=(CPanelControl_Slider*)aA->mControlList[aCount];
									aAlignCPU=(CPU*)&aC->mSlider;
									break;
								}
						}
						if (aAlignCPU) aAlignX=_max(aAlignX,aAlignCPU->mX);
					}

					for (int aCount=0;aCount<aA->mControlList.Size();aCount++)
					{
						CPU* aAlignCPU=NULL;
						switch (aA->mControlList[aCount]->mID)
						{
						case CPANELCONTROL_TEXTBOX:
							{
								CPanelControl_Textbox* aC=(CPanelControl_Textbox*)aA->mControlList[aCount];
								aAlignCPU=(CPU*)&aC->mTextBox;

								break;
							}
						case CPANELCONTROL_SLIDER:
							{
								CPanelControl_Slider* aC=(CPanelControl_Slider*)aA->mControlList[aCount];
								aAlignCPU=(CPU*)&aC->mSlider;

								break;
							}
						}
						if (aAlignCPU)
						{
							float aX1=aAlignCPU->mX;
							float aX2=aAlignCPU->mX+aAlignCPU->mWidth;
							aAlignCPU->mX=aAlignX;
							aAlignCPU->mWidth=aX2-aAlignX;
						}
					}
				}
			}
		}

		*this-=&aG->mExpand;
		if (aG->CanClose())
		{
			//
			// Set up the expand/close button
			//
			aG->mExpand.Size(aG->mRect.mX,aG->mRect.mY-aGroupNameHeight*2,aGroupNameWidth+25,aGroupNameHeight*2);
			aG->mExpand.SetNotify(this);
			//
			// Notify to panel?
			//
			*this+=&aG->mExpand;
		}
		else aG->mExpand.Size(-1,-1,-1,-1);


		aG->mRect.mY-=mCPanel->mGroupVBorders;
		aG->mRect.mHeight+=mCPanel->mGroupVBorders*2;
		if (aG->IsOpen()) aYPos+=aBorder;
	}

	SetSwipeSize(0,aYPos);
	SetPos(aHoldX,aHoldY);

	if (mMustSeeGroup)
	{
		//
		// QUESTION!
		// Don't we have to factor the YPos into this?
		// Or is it already factored in when we gen it?
		//
		Rect aVisible=GetVisibleArea();

		if (mMustSeeGroup->mRect.Center().mY>=aVisible.LowerRight().mY)
		{
			float aAdd=mMustSeeGroup->mRect.LowerRight().mY-aVisible.LowerRight().mY;
			if (mMustSeeGroup->mRect.mHeight>=mHeight) aAdd=mMustSeeGroup->mRect.Center().mY-aVisible.LowerRight().mY;
			SetPos(aHoldX,aHoldY+aAdd);
		}

		mMustSeeGroup=NULL;
	}
}

void CPanelRollout::Notify(void *theData)
{
	EnumList(CPanelGroup,aG,mGroupList) if (theData==&aG->mExpand) 
	{
		aG->mIsOpen=!aG->mIsOpen;
		mWantRehup=true;
		mCPanel->ExpandedGroup(aG);

		if (aG->IsOpen()) mMustSeeGroup=aG;
	}
}


CPanelGroup* CPanelRollout::CreateGroup(String theName, bool showGroupBackground)
{
	mWantRehup=true;

	CPanelGroup* aGroup=new CPanelGroup;
	aGroup->mMyRollout=this;
	aGroup->mName=theName;
	aGroup->mShowGroupBackground=showGroupBackground;
	mGroupList+=aGroup;
	return aGroup;
}

void CPanelRollout::BackupGroupState(Array<bool>& theArray)
{
	if (mGroupList.GetCount()==0) return;

	int aCount=0;
	theArray[mGroupList.GetCount()-1]=false;
	EnumList(CPanelGroup,aG,mGroupList) theArray[aCount++]=aG->IsOpen();
}

void CPanelRollout::RestoreGroupState(Array<bool>& theArray)
{
	if (theArray.Size()==0) return;

	int aCount=0;
	EnumList(CPanelGroup,aG,mGroupList) 
	{
		if (aCount>theArray.Size()) break;
		aG->mIsOpen=theArray[aCount++];
	}
}

CPanelControl* CPanelRollout::FindControl(String theName)
{
	EnumList(CPanelGroup,aG,mGroupList)
	{
		CPanelControl* aC=aG->FindControl(theName);
		if (aC) return aC;
	}
	return NULL;
}

CPanelControl* CPanelRollout::FindControl(int theID)
{
	EnumList(CPanelGroup,aG,mGroupList)
	{
		CPanelControl* aC=aG->FindControl(theID);
		if (aC) return aC;
	}
	return NULL;
}

CPanelGroup* CPanelRollout::FindGroup(String theName)
{
	EnumList(CPanelGroup,aG,mGroupList) if (aG->mName==theName) return aG;
	return NULL;
}

CPanelControl* CPanelRollout::FindControl(int x, int y)
{
	x-=(int)mCPanel->mWorkAreaRect.mX;
	y-=(int)mCPanel->mWorkAreaRect.mY;
	EnumList(CPanelGroup,aG,mGroupList)
	{
		EnumList(CPanelControl,aC,aG->mControlList)
		{
			if (aC->mReservedArea.ContainsPoint(x,y)) return aC;
		}
	}
	return NULL;
}


CPanelGroup* CPanelRollout::GetCurrentGroup()
{
	if (!mCurrentGroup) mCurrentGroup=StartGroup("");
	return mCurrentGroup;
}

CPanelGroup* CPanelRollout::StartGroup(String theName, bool showBackground, bool canClose, bool initiallyOpen)
{
	mCurrentGroup=CreateGroup(theName,showBackground);
	mCurrentGroup->mCanClose=canClose;
	mCurrentGroup->mIsOpen=initiallyOpen;
	return mCurrentGroup;
}





////////////////////////////////////////////////////////////////////////////////////////////
//
// Control Group...
//
////////////////////////////////////////////////////////////////////////////////////////////

CPanelControl* CPanelGroup::AddControl(CPanelControl* theControl)
{
	mMyRollout->mControlList+=theControl;
	theControl->mMyRollout=mMyRollout;
	theControl->mMyGroup=this;
	mControlList+=theControl;
	mMyRollout->mWantRehup=true;

	if (theControl->mHeight==0) theControl->mHeight=mMyRollout->mCPanel->mDefaultControlHeight;
	return theControl;
}

CPanelControl* CPanelGroup::FindControl(String theName)
{
	EnumList(CPanelControl,aCC,mControlList) if (aCC->mName==theName) return aCC;
	return NULL;
}

void CPanelGroup::ClearControls()
{
	EnumList(CPanelControl,aCPC,mControlList) 
	{
		mControlList-=aCPC;
		mMyRollout->mControlList-=aCPC;
		mMyRollout->RemoveCPU(aCPC);
		delete aCPC;

		EnumListRewind(CPanelControl);
	}
	_FreeList(AlignStruct,mAlignList);
}

void CPanelGroup::GetControls(int theID, List& theList)
{
	theList.Clear();
	EnumList(CPanelControl,aCC,mControlList)
	{
		if (aCC->mID==theID) theList+=aCC;
	}
}

CPanelControl* CPanelGroup::FindControl(int theID)
{
	EnumList(CPanelControl,aCC,mControlList) if (aCC->mID==theID) return aCC;
	return NULL;
}

#define CPanelControlDefaultTextColor(var) if (mMyRollout) if (mMyRollout->mCPanel->mDefaultTextColor.mA!=0) var->SetTextColor(mMyRollout->mCPanel->mDefaultTextColor)
CPanelControl_Checkbox* CPanelGroup::AddCheckbox(String theName, bool theState)
{
	CPanelControl_Checkbox* aCB=new CPanelControl_Checkbox;
	aCB->SetName(theName);
	aCB->SetValue(theState);
	aCB->GetCheckBox()->ExtraData()=aCB;
	CPanelControlDefaultTextColor(aCB);
	AddControl(aCB);
	aCB->GetButton()->SetExe(HOOKARG(
		{
			CheckBox* aCB=(CheckBox*)theArg;
			CPanelControl* aC=(CPanelControl*)aCB->ExtraData();

			//
			// Put in setting "IsTouching" so that catching a checkbox change acts like they unclicked (and aren't holding)
			// Put in because Hamsterball checks "IsTouching=false" to see if the user COMPLETED their change (i.e. with slider) to mark objects as saved.
			//
			CPanel::mForceNoTouching=true;
			aC->mMyGroup->mMyRollout->mCPanel->DoControlChanged(aC);
		}
	));
	//aCC->GetSlider()->SetReleaseHook(aCC->GetSlider()->mExe); // Gets us a final adjustment on release.

	return aCB;
}

CPanelControl* CPanelGroup::AddStatic(String theName)
{
	CPanelControl* aCC=new CPanelControl;
	aCC->SetName(theName);
	CPanelControlDefaultTextColor(aCC);
	AddControl(aCC);
	return aCC;
}

CPanelControl_Rollout* CPanelGroup::AddRollout(String theName, String theRolloutText, CPanelRollout* theRolloutPanel)
{
	CPanelControl_Rollout* aCC=new CPanelControl_Rollout;
	aCC->SetName(theName);
	aCC->SetRolloutText(theRolloutText);
	aCC->SetRollout(theRolloutPanel);
	CPanelControlDefaultTextColor(aCC);
	AddControl(aCC);
	return aCC;
}


CPanelControl_Slider* CPanelGroup::AddSlider(String theName, float theValue)
{
	CPanelControl_Slider* aCC=new CPanelControl_Slider;
	aCC->GetSlider()->Copy(&mMyRollout->mCPanel->DefaultSlider());
	aCC->GetSlider()->ExtraData()=aCC;
	aCC->GetSlider()->SetExe(HOOKARG(
		{
			Slider* aS=(Slider*)theArg;
			CPanelControl* aC=(CPanelControl*)aS->ExtraData();
			aC->mMyGroup->mMyRollout->mCPanel->DoControlChanged(aC);
		}
		));
	aCC->GetSlider()->SetReleaseHook(aCC->GetSlider()->mExe); // Gets us a final adjustment on release.

	if (mMyRollout->mCPanel->mDraggable) mMyRollout->Ghost(); // If it's draggable then the rollout passes touches through

	aCC->SetName(theName);
	aCC->SetValue(theValue);
	CPanelControlDefaultTextColor(aCC);
	AddControl(aCC);
	if (mMyRollout->mCPanel->DefaultSlider().mHeight>0 && mMyRollout->mCPanel->DefaultSlider().mHeight<gG.Height()) aCC->mHeight=mMyRollout->mCPanel->DefaultSlider().mHeight+aCC->GetPanel()->mControlSpacing*2;//+mMyRollout->mCPanel->mPanelBorder*2;
	return aCC;
}

CPanelControl_Radiobutton* CPanelGroup::AddRadiobutton(String theName, bool theState)
{
	CPanelControl_Radiobutton* aCB=new CPanelControl_Radiobutton;
	aCB->SetName(theName);
	aCB->SetValue(theState);
	CPanelControlDefaultTextColor(aCB);
	AddControl(aCB);

	return aCB;
}

CPanelControl_Button* CPanelGroup::AddButton(String theName)
{
	CPanelControl_Button* aCC=new CPanelControl_Button;
	aCC->SetName(theName);
	CPanelControlDefaultTextColor(aCC);
	AddControl(aCC);

	aCC->Buttonize();

	return aCC;
}

CPanelControl_Button* CPanelGroup::AddButton(String theName, Sprite* theSprite)
{
	CPanelControl_Button* aC=AddButton(theName);
	aC->SetSprite(theSprite);
	CPanelControlDefaultTextColor(aC);
	return aC;
}

CPanelControl_Textbox* CPanelGroup::AddTextbox(String theName, String theText, String allowCharacters)
{
	CPanelControl_Textbox* aCC=new CPanelControl_Textbox;
	aCC->GetTextBox()->Copy(&mMyRollout->mCPanel->DefaultTextBox());
	aCC->GetTextBox()->ExtraData()=aCC;
	aCC->GetTextBox()->SetExe(HOOKARG(
		{
			TextBox* aS=(TextBox*)theArg;
			CPanelControl* aC=(CPanelControl*)aS->ExtraData();

			CPanel::mForceNoTouching=true;
			aC->mMyGroup->mMyRollout->mCPanel->DoControlChanged(aC);
		}
	));

	if (!aCC->GetTextBox()->mFont) aCC->GetTextBox()->mFont=mMyRollout->mCPanel->mFont;
	aCC->SetName(theName);
	aCC->SetValue(theText);
	if (allowCharacters.Len()>0) aCC->mTextBox.AllowCharacters(allowCharacters);
	CPanelControlDefaultTextColor(aCC);
	aCC->mHeight=aCC->GetTextBox()->mFont->mPointSize+aCC->GetTextBox()->mBorder.mY+5;

	AddControl(aCC);

	return aCC;
}

CPanelControl_Keygrabber* CPanelGroup::AddKeyGrabber(String theName, int theKey)
{
	CPanelControl_Keygrabber* aCC=new CPanelControl_Keygrabber;
	aCC->SetName(theName);
	aCC->SetValue(theKey);
	CPanelControlDefaultTextColor(aCC);
	AddControl(aCC);

	return aCC;
}

CPanelControl_Listbox* CPanelGroup::AddListbox(String theName, String theElements, int theDefault, char theSeperator)
{
	CPanelControl_Listbox* aCC=new CPanelControl_Listbox;
	aCC->SetName(theName);
	aCC->SetValue(theDefault);
	CPanelControlDefaultTextColor(aCC);
	aCC->mIsSpriteList=false;
	AddControl(aCC);

	//
	// Need to make a whole rollout with all the elements in it.
	//
	CPanelRollout* aRollout=mMyRollout->mCPanel->CreateRollout();
	aRollout->SetName(theName);
	aRollout->UnGhost();
	aCC->SetRollout(aRollout);

	CPanelGroup* aGroup=aRollout->CreateGroup("");

	int aCount=0;
	String aToken=theElements.GetToken(theSeperator);
	while (!aToken.IsNull())
	{
		CPanelControl_Button *aButton=(CPanelControl_Button *)aGroup->AddButton(aToken);
		aButton->mControlButton->SetNotify(aCC);
		aCC->mChildButtonList+=aButton;

		if (aCount==theDefault) aCC->SetRolloutText(aToken);

		aToken=theElements.GetNextToken(theSeperator);
		aCount++;
	}
		


	return aCC;
}

CPanelControl_Listbox* CPanelGroup::AddListbox(Sprite& theSprite, Array<Sprite>& theElements, int theDefault)
{
	CPanelControl_Listbox* aCC=new CPanelControl_Listbox;
	aCC->SetSprite(&theSprite);
	aCC->SetValue(theDefault);
	CPanelControlDefaultTextColor(aCC);
	aCC->mIsSpriteList=true;
	AddControl(aCC);

	//
	// Need to make a whole rollout with all the elements in it.
	//
	CPanelRollout* aRollout=mMyRollout->mCPanel->CreateRollout();
	aRollout->SetName("");
	aRollout->UnGhost();
	aCC->SetRollout(aRollout);

	CPanelGroup* aGroup=aRollout->CreateGroup("");

	for (int aCount=0;aCount<theElements.Size();aCount++)
	{
		CPanelControl_Button *aButton=(CPanelControl_Button *)aGroup->AddButton("",&theElements[aCount]);
		aButton->mControlButton->SetNotify(aCC);
		aCC->mChildButtonList+=aButton;
	}



	return aCC;
}

void CPanelGroup::Align(String theControls)
{
	AlignStruct *aA=new AlignStruct;

	String aToken=theControls.GetToken('|');
	while (!aToken.IsNull())
	{
		EnumList(CPanelControl_Button,aCP,mControlList) if (aCP->mName==aToken) aA->mControlList+=aCP;
		aToken=theControls.GetNextToken('|');
	}

	if (aA->mControlList.Size()>0) mAlignList+=aA;
	else delete aA;
}






////////////////////////////////////////////////////////////////////////////////////////////
//
// Various implementations of controls, that need it...
//
////////////////////////////////////////////////////////////////////////////////////////////

CPanelControl* CPanelControl::Buttonize(bool isCheckbox, bool isDown)
{
	if (!isCheckbox) mControlButton=new Button;
	else
	{
		CheckBox* aC=new CheckBox;
		aC->SetState(isDown);
		mControlButton=aC;
	}


	mControlButton->SetNotify(this);
	mControlButton->NotifyOnRelease();

	AddCPUToBottom(mControlButton);
	SizeChanged();

	mControlButton->DragThruV();

	mMyRollout->mCPanel->ButtonizedControl(this,mControlButton);

	return this;
}

void CPanelControl::ResizeControlButton()
{
	if (IsSizeChanged()) if (mControlButton) mControlButton->Size(0,0,mWidth,mHeight);
}


void CPanelControl::Update()
{
	if (mControlButton)
	{
		if (mControlButton->IsDown()) 
		{
			mMyGroup->mHiliteRect=mReservedArea;
			mWasDown=true;
		}
		else 
		{
			if (mWasDown) 
			{
				mMyGroup->mHiliteRect=Rect(-1,-1,-1,-1);
				mWasDown=false;
			}
		}
	}
}

void CPanelControl_Rollout::Notify(void *theData)
{
	mMyRollout->mCPanel->mClickedControl=this;


	if (theData==&mButton || theData==mControlButton)
	{
		if (mRollout) mMyRollout->mCPanel->GoToNextPanel(mRollout);
		else 
		{
			mMyRollout->mCPanel->DoControlChanged(this);
		}
	}
}



bool CPanel::mForceNoTouching=false;
void CPanel::DoControlChanged(CPanelControl* theControl)
{
	bool aHold;
	if (mForceNoTouching)
	{
		aHold=gAppPtr->IsTouching();
		gAppPtr->SetIsTouching(false);
	}

	ControlChanged(theControl);
	if (mControlChangedHook) mControlChangedHook(this,theControl);

	if (mForceNoTouching)
	{
		gAppPtr->SetIsTouching(aHold);
		mForceNoTouching=false;
	}
}


void CPanelControl_Radiobutton::Notify(void *theData)
{
	mMyRollout->mCPanel->mClickedControl=this;
	if (theData==mControlButton) {mMyRollout->mCPanel->DoControlChanged(this);return;}
	//
	// Turn off all other radios in the group.
	//
	List aList;
	mMyGroup->GetControls(CPANELCONTROL_RADIOBUTTON,aList);
	EnumList(CPanelControl_Radiobutton,aR,aList)
	{
		if (aR==this) continue;
		bool aOldState=aR->mButton.GetState();
		aR->mButton.SetState(false);

		if (aOldState) mMyRollout->mCPanel->DoControlChanged(aR);
	}

	//
	// Turn us on...
	//
	mButton.SetState(true);
	mMyRollout->mCPanel->DoControlChanged(this);
}
/*
void CPanelControl_Button::Update()
{
	if (mButton.IsDown()) 
	{
		mMyGroup->mHiliteRect=mReservedArea;
		mWasDown=true;
	}
	else 
	{
		if (mWasDown) 
		{
			mMyGroup->mHiliteRect=Rect(-1,-1,-1,-1);
			mWasDown=false;
		}
	}
}
*/

void CPanelControl_Keygrabber::Notify(void *theData)
{
	mMyRollout->mCPanel->mClickedControl=this;
	if (theData==mControlButton) {mMyRollout->mCPanel->DoControlChanged(this);return;}

	if (mButton.mPressSound) mButton.mPressSound->Play();
	mWaitingForKey=true;
	KeyGrabber* aKG=new KeyGrabber;
	aKG->ExtraData()=this;
	aKG->Go(CPUHOOK(
		{
			CPanelControl_Keygrabber* aKG=(CPanelControl_Keygrabber*)theThis->ExtraData();
			aKG->SetValue(theThis->mThrottleResult);
			aKG->mWaitingForKey=false;
			aKG->mMyRollout->mCPanel->DoControlChanged(this);
		}
	));
	/*
	if (aNewKey!=-1) SetValue(aNewKey);
	mWaitingForKey=false;
	mMyRollout->mCPanel->ControlChanged(this);
	*/
}

void CPanelControl_Listbox::Notify(void* theData)
{
	mMyRollout->mCPanel->mClickedControl=this;
	if (theData==mControlButton) {mMyRollout->mCPanel->DoControlChanged(this);return;}

	int aCount=0;
	EnumList(CPanelControl_Button,aB,mChildButtonList)
	{
		// FUNK
		if (theData==aB->mControlButton)
		{
			mMyRollout->mCPanel->GoToPreviousPanel();
			SetValue(aCount);

			if (mIsSpriteList) SetSprite(aB->mSprite);
			else SetRolloutText(aB->mName);

			SizeChanged();
			mMyRollout->mCPanel->DoControlChanged(this);
			return;
		}
		aCount++;
	}
	CPanelControl_Rollout::Notify(theData);
}

CPanelControl_Listbox* CPanelControl_Listbox::SetValue(SmartVariable theValue)
{
	*mValue=theValue;
	CPanelControl_Button* aB=(CPanelControl_Button*)mChildButtonList[*mValue];
	if (aB)
	{
		SetRolloutText(aB->mName);
		SizeChanged();
	}
	return this;
}

SmartVariable CPanelControl_Listbox::GetValue()
{
	SmartVariable aSmarty=*mValue;
	CPanelControl_Button* aB=(CPanelControl_Button*)mChildButtonList[*mValue];
	if (aB) aSmarty.mString=aB->mName;

	return aSmarty;
}

CPanelControl_Keygrabber* CPanelControl_Keygrabber::SetValue(SmartVariable theValue)
{
	mValue=theValue;mTextValue=gInput.GetKeyName(mValue);
	return this;
}


void SwipePages::Core_Update()
{
	CPU::Core_Update();

	float aDiffX=mWantScrollX-mScrollX;
	float aDiffY=mWantScrollY-mScrollY;

	if (aDiffX || aDiffY)
	{
		if (gMath.Abs(aDiffX)>1.00f) aDiffX*=.15f;
		if (gMath.Abs(aDiffY)>1.00f) aDiffY*=.15f;
		SetPos(mScrollX+aDiffX,mScrollY+aDiffY);
	}

/*
	mSwipeSpeed*=mScrollLock;
	if (mSwipeSpeed.mX || mSwipeSpeed.mY) SetPos(mScrollX+mSwipeSpeed.mX,mScrollY+mSwipeSpeed.mY);
	if (!gAppPtr->IsTouchDevice()) mSwipeSpeed.Null(); // On non-touch devices, scroll with the mouse steadily...
	mSwipeSpeed*=mFriction;
*/
}

void SwipePages::Core_TouchStart(int x, int y)
{
	mSwipeAnchor=Point(x,y);
	mIsDown=true;

	CPU::Core_TouchStart(x,y);
}

void SwipePages::Core_TouchMove(int x, int y)
{
	if (mIsDown)
	{
		Point aSwipeSize=mSwipeAnchor-Point(x,y);
		if (aSwipeSize.LengthSquared()>3)
		{
			float aPageX=mPageSizeX;
			float aPageY=mPageSizeY;

			if (aPageX==0) aPageX=mWidth;
			if (aPageY==0) aPageY=mHeight;

			if (aSwipeSize.mX>0) mWantScrollX+=aPageX;
			if (aSwipeSize.mX<0) mWantScrollX-=aPageX;
			if (aSwipeSize.mY>0) mWantScrollY+=aPageY;
			if (aSwipeSize.mY<0) mWantScrollY-=aPageY;

			mWantScrollX=gMath.Clamp(mWantScrollX,0.0f,mSwipeWidth-mWidth);
			mWantScrollY=gMath.Clamp(mWantScrollY,0.0f,mSwipeHeight-mHeight);

			mIsDown=false;
		}
	}

	CPU::Core_TouchMove(x,y);
}

void SwipePages::Core_TouchEnd(int x, int y)
{
	mIsDown=false;

	CPU::Core_TouchEnd(x,y);
}

void SwipePages::ScrollTo(int thePageX, int thePageY)
{
	float aPageX=mPageSizeX;
	float aPageY=mPageSizeY;
	if (aPageX==0) aPageX=mWidth;
	if (aPageY==0) aPageY=mHeight;

	mWantScrollX=aPageX*thePageX;
	mWantScrollY=aPageY*thePageY;

	mWantScrollX=gMath.Clamp(mWantScrollX,0.0f,mSwipeWidth-mWidth);
	mWantScrollY=gMath.Clamp(mWantScrollY,0.0f,mSwipeHeight-mHeight);
}

RepeatingButton::RepeatingButton()
{
}

void RepeatingButton::Update()
{
	Button::Update();
	if (mHolding)
	{
		if (--mRepeatDelay==0)
		{
			mNotifyData.mParam=HELPER_NOTIFY_HOLDING;
			if (mExe) mExe(this);
			if (mNotify) mNotify->Notify(this);
			mRepeatDelay=10;
			mRepeatCount++;
			if (mRepeatCount>20) mRepeatDelay=5;
			if (mRepeatCount>50) mRepeatDelay=2;
		}
	}
}

void RepeatingButton::TouchStart(int x, int y)
{
	mNotifyOnPress=true;
	EnableFeature(CPU_UPDATE);
	Button::TouchStart(x,y);
	mRepeatDelay=70;
	mRepeatCount=0;

}

void RepeatingButton::TouchEnd(int x, int y)
{
	DisableFeature(CPU_UPDATE);
	Button::TouchEnd(x,y);
}



void TextGroup::Add(Font& theFont, String theString, Color theColor)
{
	Smart(TG) aTG=new TG;
	aTG->mFont=&theFont;
	aTG->mString=theFont.WrapEX(theString,mWidth);
	aTG->mColor=theColor;
	aTG->mRect=aTG->mFont->GetBoundingRectEX(aTG->mString,Point(0,0));
	mTGList+=aTG;

	mNeedRehup=true;
}

void TextGroup::AddSpace(float theSpace)
{
	Smart(TG) aTG=new TG;
	aTG->mFont=NULL;
	aTG->mRect=Rect(0,0,0,theSpace);

	mTGList+=aTG;
	mNeedRehup=true;
}


void TextGroup::Rehup()
{
	mHeight=0;
	float aMaxWidth=0;
	float aY=0;
	EnumSmartList(TG,aTG,mTGList)
	{
		aMaxWidth=_max(aMaxWidth,aTG->mRect.mWidth);
		mHeight+=aTG->mRect.mHeight;
	}

	mWidth=aMaxWidth;
	mNeedRehup=false;
}

DelayedMouseTouch::DelayedMouseTouch(Point thePos, int theDelay)
{
	mPos=thePos;
	mCountdown=theDelay;
	gAppPtr->mBackgroundProcesses+=this;
}

DelayedMouseTouch::~DelayedMouseTouch()
{
	gAppPtr->mBackgroundProcesses-=this;
}

void DelayedMouseTouch::Update()
{
	if (--mCountdown<=0)
	{
		gAppPtr->System_PostTouch(mPos);
		Kill();
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Developer Console...
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class DevConsole* gDevConsole=NULL;

void Log(char *format, ...)
{
	if (gDevConsole)
	{
		va_list argp;
		va_start(argp, format);

		String aS;
		aS.Format(format,argp);
		gDevConsole->Log(aS);
		va_end(argp);
	}
}

void ClearLog()
{
	if (gDevConsole) gDevConsole->Clear();
}

DevConsole::DevConsole()
{
	mFont=mHeaderFont=NULL;
	mPlotLog=0;
	gDevConsole=this;
	mBorderSize=1;
}

DevConsole::~DevConsole()
{
	if (gAppPtr) gAppPtr->mSettings.SetRect("DevConsole::Size",*this);
	if (gDevConsole==this) gDevConsole=NULL;
}

void DevConsole::Initialize()
{
	if (gAppPtr->mSettings.Exists("DevConsole::Size")) 
	{
		Size(gAppPtr->mSettings.GetRect("DevConsole::Size"));
		if (gG.GetPageRect().Expand(-5).ContainsPoint(UpperLeft()) && gG.GetPageRect().ContainsPoint(UpperRight()) && gG.GetPageRect().ContainsPoint(LowerLeft()) && gG.GetPageRect().ContainsPoint(LowerRight())) return;

	}

	Size(10,10,gG.WidthF()/2.0f,gG.HeightF()/2.0f);
}

void DevConsole::Update()
{
	if (IsKeyPressed(KB_TILDE)) 
	{
		if (mFeatures&CPU_DRAW) Hide(true);
		else Hide(false);
	}
}

void DevConsole::Draw()
{
	gG.SetColor(0,0,0,.5f);
	gG.FillRect(0,0,mWidth,mHeight);
	gG.SetColor(.5f);
	gG.FillRect(-mBorderSize,-mBorderSize,mWidth+mBorderSize,mBorderSize);
	gG.FillRect(-mBorderSize,0,mBorderSize,mHeight);
	gG.FillRect(mWidth,-mBorderSize,mBorderSize,mHeight+mBorderSize);
	gG.FillRect(-mBorderSize,mHeight,mWidth+mBorderSize*2,mBorderSize);

	if (!mFont) return;
	if (!mHeaderFont) mHeaderFont=mFont;


	mWorkArea=Rect(0,mHeaderFont->mPointSize+mBorderSize+2,mWidth,mHeight);
	mWorkArea.mY++;
	mWorkArea.mHeight-=(mHeaderFont->mPointSize+mBorderSize+2)+1;


	gG.FillRect(0,mHeaderFont->mPointSize+mBorderSize,mWidth,1);
	if (mFooter.Len()) gG.FillRect(0,mHeight-2-(mHeaderFont->mPointSize+mBorderSize),mWidth,1);

	gG.SetColor(.5f,.5f,.5f);
	mHeaderFont->Draw(Sprintf("%d",gAppPtr->FPS()),2,mHeaderFont->mPointSize);
	if (mHeader.Len()) mHeaderFont->Right(mHeader,mWidth-2,mHeaderFont->mPointSize);
	if (mFooter.Len())
	{
		mHeaderFont->Draw(mFooter,2,mHeight-mBorderSize);
		mWorkArea.mHeight-=2+(mHeaderFont->mPointSize+mBorderSize);
	}
	gG.SetColor();

	gG.PushClip();
	gG.Clip(mWorkArea);

	Point aPos=mWorkArea.LowerLeft()+Point(2,-2);
	int aRead=mPlotLog;
	int aStart=aRead;

	while (aPos.mY>=mWorkArea.mY)
	{
		aRead--;if (aRead<0) aRead+=LOGMAX;
		if (aRead==aStart) break;

		if (mLog[aRead].Len()) mFont->Draw(mLog[aRead],aPos);
		aPos.mY-=mFont->mPointSize;

	}
	//
	// Show text!
	// 

	gG.PopClip();
}

CPU* DevConsole::HitTest(float x, float y)
{
	if (mFeatures&CPU_TOUCH)
	{
		if (Expand(5).ContainsPoint(x,y)) 
		{
			x-=mX;
			y-=mY;
			if (mWorkArea.Expand(-2).ContainsPoint(x,y)) return NULL;
			return this;
		}
	}
	return NULL;
}

void DevConsole::TouchStart(int x, int y)
{
	mDrag=true;
	mDragWhat=0;
	mDragLast=gAppPtr->mMessageData_TouchStartPosition;

	if (x<10 && y<10) mDragWhat=1;
	else if (x>mWidth-10 && y<10) mDragWhat=2;
	else if (y<10) mDragWhat=3;
	else if (x<10 && y>mHeight-10) mDragWhat=4;
	else if (x>mWidth-10 && y>mHeight-10) mDragWhat=5;
	else if (x<10) mDragWhat=6;
	else if (x>mWidth-10) mDragWhat=7;
	else if (y>mHeight-10) mDragWhat=8;
}

void DevConsole::TouchMove(int x, int y)
{
	if (mDrag)
	{
		SetCursor(CURSOR_DRAG);
		Point aMove=gAppPtr->mMessageData_TouchPosition-mDragLast;

		Point aP1=UpperLeft();
		Point aP2=LowerRight();
		switch (mDragWhat)
		{
		case 0:
			aP1+=aMove;
			aP2+=aMove;
			break;
		case 1:aP1+=aMove;break;
		case 2:
			aP2.mX+=aMove.mX;
			aP1.mY+=aMove.mY;
			break;
		case 3:aP1.mY+=aMove.mY;break;
		case 4:aP1.mX+=aMove.mX;aP2.mY+=aMove.mY;break;
		case 5:aP2+=aMove;break;
		case 6:aP1.mX+=aMove.mX;break;
		case 7:aP2.mX+=aMove.mX;break;
		case 8:aP2.mY+=aMove.mY;break;
		}

		Resize(aP1,aP2);
		mDragLast=gAppPtr->mMessageData_TouchPosition;
	}
}

void DevConsole::TouchEnd(int x, int y)
{
	mDrag=false;
}

void DevConsole::Log(String theString)
{
	mLog[mPlotLog++]=theString;if (mPlotLog>=LOGMAX) mPlotLog-=LOGMAX;
	gAppPtr->BringChildToTop(this);
}

void DevConsole::Clear()
{
	for (int aCount=0;aCount<LOGMAX;aCount++) mLog[aCount]="";
}

void StaticText::Draw()
{
	gG.SetColor(mColor);
	mFont->Draw(mText,mTextPos);
	gG.SetColor();
}


void Closer::Draw()
{
	if (mColor.mA>0)
	{
		gG.SetColor(mColor);
		gG.FillRect();
		gG.SetColor();
	}
}

Notifier::Notifier()
{
	mID=CPUID_NOTIFIER;
	DisableDraw();
	DisableTouch();
}

void Notifier::Update()
{
	if (IsPaused()) return;

	if (mNotifyList.GetCount()==0) Kill();
	else
	{
		Smart(NotifyStruct) aNF=mNotifyList[0];
		if (--aNF->mDelay<=0)
		{
			mNotifyList-=aNF;
			aNF->mWho->mNotifyData.mExtraData=this;
			aNF->mWho->SystemNotify(aNF->mWhat);
		}
	}
}

void Notifier::AddNotify(CPU* notifyWho, String theCode, int theDelay)
{
	Smart(NotifyStruct) aNF=new NotifyStruct;
	aNF->mWho=notifyWho;
	aNF->mWhat=theCode;
	aNF->mDelay=theDelay;
	mNotifyList+=aNF;
}

DelayedEXE::DelayedEXE(int theDelay, bool guaranteeADraw) 
{
	mExtraData=NULL;
	mCallback=NULL;
	mDelay=theDelay;
	mWaitForDraw=guaranteeADraw;
	mAppDrawNumber=gAppPtr->AppDrawNumber();
	mDone=false;
}

DelayedEXE::DelayedEXE(void (*theCallback)(void* theExtraData),int theDelay, bool guaranteeADraw, void* theExtraData)
{
	mExtraData=theExtraData;
	mCallback=theCallback;;
	mDelay=theDelay;
	mWaitForDraw=guaranteeADraw;
	mAppDrawNumber=gAppPtr->AppDrawNumber();
	mDone=false;
}

void DelayedEXE::Update()
{
	if (!gAppPtr || gAppPtr->IsQuit() || mDone) return;

	if (mAppDrawNumber<gAppPtr->AppDrawNumber()) mWaitForDraw=false;
	if (--mDelay<=0 && !mWaitForDraw) {mDone=true;if (mCallback) mCallback(mExtraData);EXE();Kill();}
}


DelayedHOOK::DelayedHOOK(HOOKPTR theHook,int theDelay, bool guaranteeADraw)
{
	mHook=theHook;
	mDelay=theDelay;
	mWaitForDraw=guaranteeADraw;
	mAppDrawNumber=gAppPtr->AppDrawNumber();
	mDone=false;
}

void DelayedHOOK::Update()
{
	if (!gAppPtr || gAppPtr->IsQuit() || mDone) return;

	if (mAppDrawNumber<gAppPtr->AppDrawNumber()) mWaitForDraw=false;
	if (--mDelay<=0 && !mWaitForDraw) {mDone=true;if (mHook) mHook();Kill();}
}

void RunAfterUpdates(int theUpdates, void (*theCallback)(void* theExtraData), void* theExtraData) {gAppPtr->mBackgroundProcesses+=new DelayedEXE(theCallback,theUpdates,false,theExtraData);}
void RunAfterNextDraw(void (*theCallback)(void* theExtraData), void* theExtraData) {gAppPtr->mBackgroundProcesses+=new DelayedEXE(theCallback,0,true,theExtraData);}
void RunAfterUpdates(int theUpdates, HOOKPTR theHook) {gAppPtr->mBackgroundProcesses+=new DelayedHOOK(theHook,theUpdates,false);}
void RunAfterNextDraw(HOOKPTR theHook) {gAppPtr->mBackgroundProcesses+=new DelayedHOOK(theHook,0,true);}

void StepProcessor::Update()
{
	static int aSlowly=0;
	if ((++aSlowly%5)==0)
	if (AppTime()>mLastDrawUpdate)
	{
		if (mPCounter>=mProcessList.Size())
		{
			Kill();
			if (mNextProcessor) {gAppPtr->AddCPU(mNextProcessor);mNextProcessor=NULL;}
			if (mOnComplete) mOnComplete(this);
		}
		else
		{
			//
			// Move a file!
			//
			if (mProcessHook) mProcessHook(this);
			else Process(mProcessList[mPCounter]);

			//OS_Core::Sleep(400);
			mPCounter++;
			mProgress=(float)mPCounter/(float)mProcessList.Size();
			gAppPtr->CatchUp();
		}
	}
}

void StepProcessor::Draw()
{
	mLastDrawUpdate=AppTime();
}

#ifdef LEGACY_GL
void MoveFolderToCloud::Process(String theData)
{
	IOBuffer aBuffer;
	String aCloudKey=theData.GetSegment(mFromFolder.Len(),MAX_PATH);

	aBuffer.Load(theData);
	if (!aBuffer.CloudSave(aCloudKey)) mError=true;
}

void MoveCloudToFolder::Process(String theData)
{
	String aFinal=mToFolder;aFinal+=theData;

	IOBuffer aBuffer;
	if (!aBuffer.CloudLoad(theData)) mError=true;
	else {MakeDirectory(aFinal);aBuffer.CommitFile(aFinal);}
}
#endif

struct CloudInfo
{
	Array<DataPair<String>> mCloudConversions;
} *gCloudInfo=NULL;

//
// Cloud conversions just does an automatic search and replace.  Most commonly it just takes sandbox:// and turns it into / so that it becomes a path to "root" within the cloud area.
// For iOS, because cloud was inside sandbox, I had to do some special stuff.  This is essentially becoming obsolete because the cloud:// folder is depreciated as of 2021, all of
// sandbox clouds.
//
void SetCloudConversions(String theConversions) 
{
	if (gCloudInfo)	delete gCloudInfo;
	gCloudInfo=new CloudInfo;
	if (theConversions.Len()==0)
	{
		gCloudInfo->mCloudConversions[0][0]="sandbox://";
		gCloudInfo->mCloudConversions[0][1]="/";
	}
	else
	{
		String aToken=theConversions.GetToken(';');
		while (aToken.Len()>0)
		{
			int aNext=gCloudInfo->mCloudConversions.Size();
			gCloudInfo->mCloudConversions[aNext][0]=aToken.GetSegmentBefore("->");
			gCloudInfo->mCloudConversions[aNext][1]=aToken.GetSegmentAfter("->");
			aToken=theConversions.GetNextToken(';');
		}
	}
}

String FilenameToCloudKey(String theFN)
{
	if (!gCloudInfo) SetCloudConversions("");

	String aFix=gAppPtr->GetStorageLocation(theFN);
	for (int aCount=0;aCount<aFix.Len();aCount++) if (aFix[aCount]=='\\') aFix[aCount]='/';
	for (int aCount=0;aCount<gCloudInfo->mCloudConversions.Size();aCount++)
	{
		if (aFix.StartsWith(gCloudInfo->mCloudConversions[aCount][0]))
		{
			String aSeg=aFix.GetSegment(gCloudInfo->mCloudConversions[aCount][0].Len(),MAX_PATH);
			aFix=gCloudInfo->mCloudConversions[aCount][1];
			aFix+=aSeg;

		}
	}

	//
	// Remove all double //'s unless it's '://'
	//
	aFix.RemoveRedundantFilenameCharacters();
	return aFix;
}

StepProcessor* BackupToCloud(String theFolder)
{
	if (!gCloudInfo) SetCloudConversions("");

	StepProcessor* aProc=new StepProcessor;
	if (!gAppPtr) return aProc;

	aProc->mProcessHook=CPUHOOK(
		{
			StepProcessor* aThis=(StepProcessor*)theThis;
			String aProcess=aThis->GetProcess();
			if (aProcess.StartsWith("<GO>"))
			{
				String aBackupFolder=aProcess.GetSegment(4,99999);
				String aFolder=aBackupFolder.GetToken(';');
				//if (aFolder.Len()) RemoveFileFromCloud("*.*");

				Array<String> aInCloudList;
				EnumCloud(aInCloudList);

				Array<String> aLocalList;

				while (aFolder.Len()) 
				{
					EnumDirectory(aFolder,aLocalList,true);
					aFolder=aBackupFolder.GetNextToken(';');
				}

				//
				// Add of LocalList to process List...
				// Anyone in InCloudList but not local gets added to the deleted list.
				//
				for (int aCount=0;aCount<aLocalList.Size();aCount++) aThis->mProcessList+=aLocalList[aCount];
				for (int aCount=0;aCount<aInCloudList.Size();aCount++)
				{
					bool aFound=false;
					for (int aCount2=0;aCount2<aLocalList.Size();aCount2++)
					{
						String aCheck=FilenameToCloudKey(aLocalList[aCount2]);
						if (aInCloudList[aCount]==aCheck) {aFound=true;break;}
					}
					if (!aFound) aThis->mProcessList+=Sprintf("-%s",aInCloudList[aCount].c());
				}
				CloudQueue::Kill();
			}
			else if (!aProcess.ContainsI(".nocloud"))
			{
				if (aProcess[0]=='-') 
				{
					String aKillIt=aProcess.GetSegment(1,aProcess.Len());
					OS_Core::DeleteCloud(aKillIt);
				}
				else
				{
					IOBuffer aBuffer;
					aBuffer.Load(aProcess);
					aBuffer.CloudSave(FilenameToCloudKey(aProcess),true);
				}
			}
		});

	//
	// We put <GO> into the process list so that the user gets immediate visual feedback in case the cloud takes time to connect.
	//
	String aFirst="<GO>";
	aFirst+=theFolder;
	aProc->mProcessList+=aFirst;
	gAppPtr->AddCPU(aProc);
	return aProc;
}

StepProcessor* RestoreFromCloud(bool wipeLocalData)
{
	if (!gCloudInfo) SetCloudConversions("");

	//
	// Need to delete the contents of sandbox and cloud... but DON'T cloud delete anything.
	//
	StepProcessor* aProc=new StepProcessor;
	if (!gAppPtr) return aProc;
	aProc->mProcessHook=CPUHOOK(
		{
			StepProcessor* aThis=(StepProcessor*)theThis;
			String aProcess=aThis->GetProcess();
			if (aProcess=="<GO>")
			{
				CloudQueue::Kill();
				EnumCloud(aThis->mProcessList);
				bool aHold=gAppPtr->mAutoCloud;
				gAppPtr->mAutoCloud=false;

				if (wipeLocalData)
				{
					DeleteDirectory("sandbox://");
					DeleteDirectory("cloud://");
					MakeDirectory("sandbox://");
				}

				gAppPtr->mAutoCloud=aHold;
			}
			else if (!aProcess.ContainsI(".nocloud"))
			{
				IOBuffer aBuffer;
				aBuffer.CloudLoad(aProcess);
				String aFix=aProcess;

				// Gotta unixify...
				for (int aCount=0;aCount<aFix.Len();aCount++) if (aFix[aCount]=='\\') aFix[aCount]='/';
				for (int aCount=0;aCount<gCloudInfo->mCloudConversions.Size();aCount++) {if (aFix.StartsWith(gCloudInfo->mCloudConversions[aCount][1])){String aSeg=aFix.GetSegment(gCloudInfo->mCloudConversions[aCount][1].Len(),MAX_PATH);aFix=gCloudInfo->mCloudConversions[aCount][0];aFix+=aSeg;}}

				CloudQueue::Disable();
				aBuffer.CommitFile(aFix);
				CloudQueue::Enable();
			}
		});

	aProc->mProcessList+="<GO>";



	//gOut.Out("----------- RESTORE FROM CLOUD -----------");
	//for (int aCount=0;aCount<aProc->mProcessList.Size();aCount++) gOut.Out("Our Enum: %s",aProc->mProcessList[aCount].c());

	gAppPtr->AddCPU(aProc);
	return aProc;
}

