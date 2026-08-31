#include "TouchButtons.h"
#include "MyApp.h"

TouchButtons::TouchButtons(void)
{
}

TouchButtons::~TouchButtons(void)
{
	_FreeList(ButtonStruct,mButtonList);
}

TouchButtons::ButtonStruct *TouchButtons::GetButton(int theX, int theY, bool allowClosest)
{
	allowClosest=false;
	float aDist=999*999;
	ButtonStruct *aClosest=NULL;

	EnumList(ButtonStruct,aB,mButtonList) 
	{
		if (aB->mDisabled) continue;
		Rect aHitArea=aB->mRect.Expand(4);
		aHitArea.mY-=3;
		aHitArea.mHeight+=6;
		if (aHitArea.ContainsPoint(theX,theY)) return aB;
		if (allowClosest)
		{
			float aTestDist=gMath.DistanceSquared(aB->mRect.Center(),Point(theX,theY));
			if (aTestDist<aDist)
			{
				aDist=aTestDist;
				aClosest=aB;
			}
		}
	}
	return aClosest;
}

TouchButtons::ButtonStruct *TouchButtons::GetButtonFromHash(int theHash)
{
	
	EnumList(ButtonStruct,aB,mButtonList) if (aB->mPressHash==theHash) 
	{
		aB->mPressHash=-1;
		return aB;
	}
	return NULL;
}

TouchButtons::ButtonStruct *TouchButtons::GetPressedButton(int theX, int theY, bool allowClosest)
{
	float aDist=999*999;
	ButtonStruct *aClosest=NULL;

	EnumList(ButtonStruct,aB,mButtonList) 
	{
		if (aB->mDisabled) continue;
		if (!aB->mIsPressed) continue;
		if (aB->mRect.ContainsPoint(theX,theY)) return aB;
		if (allowClosest)
		{
			float aTestDist=gMath.DistanceSquared(aB->mRect.Center(),Point(theX,theY));
			if (aTestDist<aDist)
			{
				aDist=aTestDist;
				aClosest=aB;
			}
		}
	}
	return aClosest;
}

TouchButtons::ButtonStruct *TouchButtons::GetNotPressedButton(int theX, int theY, bool allowClosest)
{
	float aDist=999*999;
	ButtonStruct *aClosest=NULL;
	
	EnumList(ButtonStruct,aB,mButtonList) 
	{
		if (aB->mDisabled) continue;
		if (aB->mIsPressed) continue;
		if (aB->mRect.ContainsPoint(theX,theY)) return aB;
		if (allowClosest)
		{
			
			float aTestDist=gMath.DistanceSquared(aB->mRect.Center(),Point(theX,theY));
			if (aTestDist<aDist)
			{
				aDist=aTestDist;
				aClosest=aB;
			}
		}
	}
	return aClosest;
}

void TouchButtons::Press(ButtonStruct *theButton, int x, int y)
{
	if (!theButton->mCanHold) if (theButton->mCooldown>gAppPtr->AppTime()) return;
	
	theButton->mIsPressed=true;
	theButton->mTouchPos=Point(x,y)-theButton->mRect.Center();
	theButton->mPressHash=gApp.mMessageData_TouchID;
	
	if (!theButton->mCanHold) theButton->mCooldown=gAppPtr->AppTime()+15;
}

void TouchButtons::Reset()
{
	EnumList(ButtonStruct,aBS,mButtonList)
	{
		aBS->mIsPressed=false;
	}
}

bool TouchButtons::MouseDown(int x, int y)
{
	//
	// See if we're in a rect!
	//
	ButtonStruct *aB=GetButton(x,y,true);
	if (aB)
	{
		aB->mCooldown=gAppPtr->AppTime();

		//gOut.Out("Press Button: %d",aB);

		Press(aB,x,y);
	}
	
	mCooldownList.Clear();
	return (aB!=NULL);
}

void TouchButtons::MouseUp(int x, int y)
{

	ButtonStruct *aB=GetButtonFromHash(gAppPtr->mMessageData_TouchID);
	if (aB) 
	{
		//gOut.Out("Unpress Button: %d",aB);
		aB->mIsPressed=false;
	}
	
	mCooldownList.Clear();
}

void TouchButtons::MouseMove(int x, int y)
{
	ButtonStruct *aB=GetButton(x,y);
	if (aB) 
	{
		if (aB->mPressHash!=gAppPtr->mMessageData_TouchID)
		{
			if (!mCooldownList.Exists(aB))			{
				//
				// Hey, possible "rocking" of our finger, such as from a fire button to
				// a jump button.  If this button isn't "canhold" then press it.
				//
				if (!aB->mCanHold)
				{
					Press(aB,0,0);
					aB->mPressHash=-1;
					mCooldownList+=aB;
				}
			}
		}

		aB->mTouchPos=Point(x,y)-aB->mRect.Center();
		if (mCooldownList.GetCount()) EnumList(ButtonStruct,aBB,mCooldownList) if (!aBB->mRect.ContainsPoint(x,y)) mCooldownList-=aBB;
	}
	
}


void TouchButtons::AddButton(int theID, Rect theRect, bool canHold)
{
	ButtonStruct *aB=new ButtonStruct;
	aB->mID=theID;
	aB->mRect=theRect;
	aB->mCanHold=canHold;
	mButtonList+=aB;
}

bool TouchButtons::IsPressed(int theID, Point *theTouchPos)
{
	EnumList(ButtonStruct,aB,mButtonList) 
	{
		if (aB->mID==theID)
		{
			if (aB->mIsPressed)
			{
				if (theTouchPos)
				{
					theTouchPos->mX=aB->mTouchPos.mX;
					theTouchPos->mY=aB->mTouchPos.mY;
				}
				if (!aB->mCanHold) aB->mIsPressed=false;
				return true;
			}
			return false;
		}
	}
	return false;
}

Rect TouchButtons::GetButtonRect(int theID)
{
	EnumList(ButtonStruct,aB,mButtonList) if (aB->mID==theID) return aB->mRect;
	return Rect(-9999,-9999,0,0);
}

void TouchButtons::EnableButton(int theID, bool theState)
{
	EnumList(ButtonStruct,aB,mButtonList) if (aB->mID==theID)
	{
		aB->mDisabled=!theState;
	}
}

void TouchButtons::MoveButton(int theID,Point thePos)
{
	EnumList(ButtonStruct,aB,mButtonList) if (aB->mID==theID)
	{
		aB->mRect.mX=thePos.mX-(aB->mRect.mWidth/2);
		aB->mRect.mY=thePos.mY-(aB->mRect.mHeight/2);
	}
}


