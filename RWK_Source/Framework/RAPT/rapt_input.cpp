#include "rapt.h"
#include "rapt_input.h"
#include "input_core.h"

#include "os_core.h"

Input* gInputPtr=NULL;

Input::Input()
{
	EnableKeyboard();
	mKeyBuffer=0;
	memset(mKeys,0,sizeof(mKeys));

	gInputPtr=this;
}

Input::~Input()
{
	if (gInputPtr==this) gInputPtr=NULL;
}


void Input::Go()
{
	Input_Core::Startup();
	gAppPtr->AddBackgroundProcess(this);
}

void Input::Stop()
{
	//
	// Write out joystick calibration data
	//
	/*
	for (int aJoystick=0;aJoystick<mJoystickData.Size();aJoystick++)
	{
		if (mJoystickData[aJoystick].mCalibrated)
		{
			String aName=GetJoystickName(aJoystick);
			if (aName!="Invalid Joystick")
			{
				String aKey=Sprintf("joystick.calibrate.%s",aName.c());
				String aXKey=aKey;aXKey+=".calibrated";gAppPtr->mSettings.SetBool(aXKey,true);
				aXKey=aKey;aXKey+=".axisx1";gAppPtr->mSettings.SetChar(aXKey,mJoystickData[aJoystick].mAxis_Stick1X);
				aXKey=aKey;aXKey+=".axisy1";gAppPtr->mSettings.SetChar(aXKey,mJoystickData[aJoystick].mAxis_Stick1Y);
				aXKey=aKey;aXKey+=".axisx2";gAppPtr->mSettings.SetChar(aXKey,mJoystickData[aJoystick].mAxis_Stick2X);
				aXKey=aKey;aXKey+=".axisy2";gAppPtr->mSettings.SetChar(aXKey,mJoystickData[aJoystick].mAxis_Stick2Y);
				aXKey=aKey;aXKey+=".trigger1";gAppPtr->mSettings.SetChar(aXKey,mJoystickData[aJoystick].mAxis_Trigger1);
				aXKey=aKey;aXKey+=".trigger2";gAppPtr->mSettings.SetChar(aXKey,mJoystickData[aJoystick].mAxis_Trigger2);
			}
		}
	}
	*/

	if (gAppPtr) gAppPtr->RemoveBackgroundProcess(this);
	Input_Core::Shutdown();
}

void Input::Update()
{
	mKeyBuffer=1-mKeyBuffer;
	if (mEnableKeyboard) 
	{
		Input_Core::PollKeyboard(mKeys[mKeyBuffer]);
		//
		// Get mouse state as keys, too...
		//
		if (!gAppPtr->IsTouchDevice())
		{
			mKeys[mKeyBuffer][KB_LEFTMOUSE]=0;
			mKeys[mKeyBuffer][KB_RIGHTMOUSE]=0;
			mKeys[mKeyBuffer][KB_MIDDLEMOUSE]=0;
			if (gAppPtr->GetTouchStatus()&TOUCH_LEFT) mKeys[mKeyBuffer][KB_LEFTMOUSE]=1;
			if (gAppPtr->GetTouchStatus()&TOUCH_RIGHT) mKeys[mKeyBuffer][KB_RIGHTMOUSE]=1;
			if (gAppPtr->GetTouchStatus()&TOUCH_MIDDLE) mKeys[mKeyBuffer][KB_MIDDLEMOUSE]=1;
		}
	}
}

bool Input::KeyState(int theKey)
{
	if (theKey<0 || theKey>=512) 
	{
		gOut.Out("Input::KeyState() -> Key out of range: %d",theKey);
		return 0;
	}
	return (mKeys[mKeyBuffer][theKey]!=0);
}

bool Input::KeyPressed(int theKey)
{
	if (theKey<0 || theKey>=512) 
	{
		gOut.Out("Input::KeyPressed() -> Key out of range: %d",theKey);
		return 0;
	}
	bool aResult=(mKeys[mKeyBuffer][theKey] && !mKeys[1-mKeyBuffer][theKey]);
	//mKeys[mKeyBuffer][theKey]=0;
	return aResult;
}

void Input::ResetKeyboard()
{
	memset(mKeys,0,sizeof(mKeys));
}


void Input::Multitasking(bool isForeground)
{
	Input_Core::Multitasking(isForeground);
}

String Input::GetKeyName(int theKey)
{
	return Input_Core::GetKeyName(theKey);
}

String Input::GetShortKeyName(int theKey)
{
	return Input_Core::GetShortKeyName(theKey);
}

int Input::GetJoystickCount()
{
	return Input_Core::GetJoystickCount();
}

String Input::GetJoystickName(int theJoyStick)
{
	if (theJoyStick<0 || theJoyStick>=GetJoystickCount()) return "Invalid Joystick";
	char aName[MAX_PATH];
	Input_Core::GetJoystickName(theJoyStick,aName);

	String aResult=aName;
	aResult.DeleteI("(controller)");
	aResult.Delete("=");
	aResult.RemoveLeadingSpaces();
	aResult.RemoveTrailingSpaces();
	return aResult;
}

Point Input::GetJoystickDir(int theJoyStick, int theThumbStick)
{
	if (theThumbStick<0 || theThumbStick>1) return Point(0,0);
	if (!PollJoystick(theJoyStick)) return Point(0,0);
	return mJoystickData[theJoyStick].mStickDir[theThumbStick];
}

Point Input::GetJoystickHat(int theJoyStick)
{
	if (theJoyStick<0 || theJoyStick>=GetJoystickCount()) return Point(0,0);
	if (!PollJoystick(theJoyStick)) return Point(0,0);
	return Point(mJoystickData[theJoyStick].mHat.mX,mJoystickData[theJoyStick].mHat.mY);
}

bool Input::PollJoystick(int theJoystick)
{
#ifdef _STEAM
#ifdef _DEBUG
	if (IsKeyPressed(KB_I)) Input_Core::Startup();
#endif
#endif
	if (theJoystick<0 || theJoystick>=GetJoystickCount()) return false;
	if (mJoystickData[theJoystick].mPollTime==gAppPtr->AppTime()) return true;
	if (!mJoystickData[theJoystick].mCalibrated) CalibrateJoystick(theJoystick);
	mJoystickData[theJoystick].mPollTime=gAppPtr->AppTime();

	static Array<float> aAxes;
	static Array<short> aHats;

	Input_Core::PollJoystick(theJoystick,aAxes,aHats,mJoystickData[theJoystick].mButton[mJoystickData[theJoystick].mPollTime%2]);
	mJoystickData[theJoystick].mStickDir[0].mX=aAxes[mJoystickData[theJoystick].mAxis_Stick1X];
	mJoystickData[theJoystick].mStickDir[0].mY=aAxes[mJoystickData[theJoystick].mAxis_Stick1Y];
	mJoystickData[theJoystick].mStickDir[1].mX=aAxes[mJoystickData[theJoystick].mAxis_Stick2X];
	mJoystickData[theJoystick].mStickDir[1].mY=aAxes[mJoystickData[theJoystick].mAxis_Stick2Y];
	mJoystickData[theJoystick].mTrigger1=aAxes[mJoystickData[theJoystick].mAxis_Trigger1];
	mJoystickData[theJoystick].mTrigger2=aAxes[mJoystickData[theJoystick].mAxis_Trigger2];
	if (aHats==0) 
	{
		// If no hats, send the dpad as the hat...
		mJoystickData[theJoystick].mHat=0;
		if (mJoystickData[theJoystick].mButton[mJoystickData[theJoystick].mPollTime%2][JOYBUTTON_DPADUP]) mJoystickData[theJoystick].mHat.mY=-1;
		if (mJoystickData[theJoystick].mButton[mJoystickData[theJoystick].mPollTime%2][JOYBUTTON_DPADDOWN]) mJoystickData[theJoystick].mHat.mY=1;
		if (mJoystickData[theJoystick].mButton[mJoystickData[theJoystick].mPollTime%2][JOYBUTTON_DPADLEFT]) mJoystickData[theJoystick].mHat.mX=-1;
		if (mJoystickData[theJoystick].mButton[mJoystickData[theJoystick].mPollTime%2][JOYBUTTON_DPADRIGHT]) mJoystickData[theJoystick].mHat.mX=1;
	}
	else
	{
		mJoystickData[theJoystick].mHat.mX=(signed char)LoByte(aHats[0]);
		mJoystickData[theJoystick].mHat.mY=(signed char)HiByte(aHats[0]);
	}

	//gOut.Out("Z: %f",mJoystickData[theJoystick].mZAxis);
	//
	// Kludge to make it so triggers can be treated as buttons...
	//
	mJoystickData[theJoystick].mButton[mJoystickData[theJoystick].mPollTime%2][mJoystickData[theJoystick].mButtonMap[JOYBUTTON_LEFTTRIGGER]]=false;
	mJoystickData[theJoystick].mButton[mJoystickData[theJoystick].mPollTime%2][mJoystickData[theJoystick].mButtonMap[JOYBUTTON_RIGHTTRIGGER]]=false;
	if (mJoystickData[theJoystick].mTrigger1>.9f) mJoystickData[theJoystick].mButton[mJoystickData[theJoystick].mPollTime%2][mJoystickData[theJoystick].mButtonMap[JOYBUTTON_LEFTTRIGGER]]=true;
	if (mJoystickData[theJoystick].mTrigger2>.9f) mJoystickData[theJoystick].mButton[mJoystickData[theJoystick].mPollTime%2][mJoystickData[theJoystick].mButtonMap[JOYBUTTON_RIGHTTRIGGER]]=true;
	return true;
}

Point GetJoystickDir(int theJoystick) 
{
	Point aResult;
	if (!gInputPtr->PollJoystick(theJoystick)) return Point(0,0);
	aResult+=gInputPtr->mJoystickData[theJoystick].mStickDir[0];
	aResult+=gInputPtr->mJoystickData[theJoystick].mStickDir[1];
	aResult+=gInputPtr->mJoystickData[theJoystick].mHat;
	return aResult;
}

Point GetJoystickDirClampThumbsticks(int theJoystick, Point theClamp) 
{
	Point aResult;
	if (!gInputPtr->PollJoystick(theJoystick)) return Point(0,0);
	aResult+=gInputPtr->mJoystickData[theJoystick].mStickDir[0]*theClamp;
	aResult+=gInputPtr->mJoystickData[theJoystick].mStickDir[1]*theClamp;
	aResult+=gInputPtr->mJoystickData[theJoystick].mHat;
	return aResult;
}


void Input::ResetJoysticks()
{
	for (int aCount=0;aCount<mJoystickData.Size();aCount++) 
	{
		mJoystickData[aCount].Reset();
		mJoystickData[aCount].mPollTime=gAppPtr->AppTime();
	}
}

bool Input::CalibrateJoystick(int theJoystick)
{
	/*
	String aKey=Sprintf("joystick.calibrate.%s",GetJoystickName(theJoystick).c());
	String aXKey=aKey;aXKey+=".calibrated";

	mJoystickData[theJoystick].mCalibrated=true;
	if (gAppPtr->mSettings.Exists(aXKey))
	{
		aXKey=aKey;aXKey+=".axisx1";mJoystickData[theJoystick].mAxis_Stick1X=gAppPtr->mSettings.GetChar(aXKey);
		aXKey=aKey;aXKey+=".axisy1";mJoystickData[theJoystick].mAxis_Stick1Y=gAppPtr->mSettings.GetChar(aXKey);
		aXKey=aKey;aXKey+=".axisx2";mJoystickData[theJoystick].mAxis_Stick2X=gAppPtr->mSettings.GetChar(aXKey);
		aXKey=aKey;aXKey+=".axisy2";mJoystickData[theJoystick].mAxis_Stick2Y=gAppPtr->mSettings.GetChar(aXKey);
		aXKey=aKey;aXKey+=".trigger1";mJoystickData[theJoystick].mAxis_Trigger1=gAppPtr->mSettings.GetChar(aXKey);
		aXKey=aKey;aXKey+=".trigger2";mJoystickData[theJoystick].mAxis_Trigger2=gAppPtr->mSettings.GetChar(aXKey);
		return true;
	}
	*/
	//mJoystickData[theJoystick].mCalibrated=true;
	mJoystickData[theJoystick].mCalibrated=Input_Core::CalibrateJoystick(theJoystick,mJoystickData[theJoystick].mAxis_Stick1X,mJoystickData[theJoystick].mAxis_Stick1Y,mJoystickData[theJoystick].mAxis_Stick2X,mJoystickData[theJoystick].mAxis_Stick2Y,mJoystickData[theJoystick].mAxis_Trigger1,mJoystickData[theJoystick].mAxis_Trigger2,mJoystickData[theJoystick].mButtonMap);

	//
	// Kludge to make it so triggers can be treated as buttons...
	//
	int aMapSize=mJoystickData[theJoystick].mButtonMap.Size();

	//
	// Any buttons that don't exist just go -1.  And we add two fake buttons at the end to be the trigger buttons, 
	// which come in as axis!
	//
	for (int aCount=aMapSize;aCount<JOYBUTTON_MAX;aCount++) mJoystickData[theJoystick].mButtonMap[aCount]=-1;
	mJoystickData[theJoystick].mButtonMap[JOYBUTTON_LEFTTRIGGER]=JOYBUTTON_LEFTTRIGGER;
	mJoystickData[theJoystick].mButtonMap[JOYBUTTON_RIGHTTRIGGER]=JOYBUTTON_RIGHTTRIGGER;
	mJoystickData[theJoystick].mButton[0].GuaranteeSize(JOYBUTTON_MAX);
	mJoystickData[theJoystick].mButton[1].GuaranteeSize(JOYBUTTON_MAX);
	for (int aCount=0;aCount<2;aCount++) foreach(aB,mJoystickData[theJoystick].mButton[aCount]) aB=0;

	return mJoystickData[theJoystick].mCalibrated;
}

bool Input::HasAccelerometer()
{
	return Input_Core::HasAccelerometer();
}

Vector Input::PollAccelerometer()
{
	Vector aV;
	Input_Core::PollAccelerometer(&aV.mX,&aV.mY,&aV.mZ);
	return aV;
}

int Input::GetJoystickButtonCount(int theJoystick)
{
	return JOYBUTTON_MAX; // Always the same...
	//if (!mJoystickData[theJoystick].mCalibrated) CalibrateJoystick(theJoystick);
	//return mJoystickData[theJoystick].mButtonMap.Size()+2; // +2 for the triggers
}
