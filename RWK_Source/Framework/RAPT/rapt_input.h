#pragma once
#include "rapt_headers.h"

//#define JOYSTICK_TRIGGER1 MAX_JOYSTICK_BUTTONS
//#define JOYSTICK_TRIGGER2 MAX_JOYSTICK_BUTTONS+1

enum
{
	JOYBUTTON_A=0,
	JOYBUTTON_B,
	JOYBUTTON_X,
	JOYBUTTON_Y,
	JOYBUTTON_BACK,
	JOYBUTTON_GUIDE,
	JOYBUTTON_START,
	JOYBUTTON_LEFTSTICK,
	JOYBUTTON_RIGHTSTICK,
	JOYBUTTON_LEFTSHOULDER,
	JOYBUTTON_RIGHTSHOULDER,
	// On some joysticks, buttons end here
	JOYBUTTON_DPADUP,
	JOYBUTTON_DPADDOWN,
	JOYBUTTON_DPADLEFT,
	JOYBUTTON_DPADRIGHT,
	JOYBUTTON_MISC1,
	JOYBUTTON_PADDLE1,
	JOYBUTTON_PADDLE2,
	JOYBUTTON_PADDLE3,
	JOYBUTTON_PADDLE4,
	JOYBUTTON_TOUCHPAD,
	// Added these myself, so I can use triggers as buttons.
	JOYBUTTON_LEFTTRIGGER,
	JOYBUTTON_RIGHTTRIGGER,
	JOYBUTTON_MAX
};




class Input : public CPU
{
public:
	Input();
	virtual ~Input();

	//
	// Startup/Shutdown
	//
	void					Go();
	void					Stop();

	//
	// Processes...
	//
	void					Update();

	//
	// Decides WHAT we're processing... 
	//
	bool					mEnableKeyboard;
	inline void				EnableKeyboard(bool theState=true) {mEnableKeyboard=theState;}

	//
	// Keyboard buffer
	//
	char					mKeys[2][512+3];
	int						mKeyBuffer;

	//
	// Get info about keys
	//
	String					GetKeyName(int theKey);
	String					GetShortKeyName(int theKey);

	//
	// Info about pressing keys
	//
	bool					KeyState(int theKey);
	bool					KeyPressed(int theKey);
	void					ResetKeyboard();	// Resets keyboard's state

	//
	// Restore (so if we regain focus)...
	//
	void					Multitasking(bool isForeground);

	//
	// Joysticks
	//
	int						GetJoystickCount();
	String					GetJoystickName(int theJoyStick);
	Point					GetJoystickDir(int theJoyStick, int theThumbStick=0);
	Point					GetJoystickHat(int theJoyStick);
	bool					PollJoystick(int theJoyStick);
	bool					CalibrateJoystick(int theJoystick);	// Returns FALSE if the user should calibrate it, true if the system has a specific joystick (like XBox, Nintendo)
	void					ResetJoysticks();

	int						GetJoystickButtonCount(int theJoystick);
	//
	// This Calibrate Joystick just is our list of joystick names, and what needs
	// to be done to calibrate them.
	//
	// Took these out because I think I have an auto-calibrate that will work.
	// Be prepared to put them back in!
	//
	//void					CalibrateJoystick(String theJoystickName);
	//void					CalibrateJoystick(int theAxis, String theXReference, String theYReference);
	//void					CalibrateJoystick(int theAxis, String theReference);

	struct JoystickData
	{
		JoystickData() 
		{
			mPollTime=-9999;
			mTrigger1=mTrigger2=0;
			mCalibrated=false;
		}

		void Reset()
		{
			mPollTime=0;
			mStickDir[0]=0;
			mStickDir[1]=0;
			mHat=0;
			mButton[0].Reset();
			mButton[1].Reset();
			mTrigger1=0;
			mTrigger2=0;
			mAxis_Stick1X=0;
			mAxis_Stick1Y=0;
			mAxis_Stick2X=0;
			mAxis_Stick2Y=0;
			mAxis_Trigger1=0;
			mAxis_Trigger2=0;
			mCalibrated=false;
		}

		int					mPollTime;
		Point				mStickDir[2];
		IPoint				mHat;
		Array<char>			mButton[2];
		float				mTrigger1;
		float				mTrigger2;

		//
		// Here is the connection to what axis equals what on this system.
		// Input_Core CAN fill it out automagically for us if we have a system with
		// a specific input derangement, but if input core returns FALSE, then the
		// game should ask them to calibrate.
		//
		char				mAxis_Stick1X;
		char				mAxis_Stick1Y;
		char				mAxis_Stick2X;
		char				mAxis_Stick2Y;
		char				mAxis_Trigger1;
		char				mAxis_Trigger2;
		Array<char>			mButtonMap;
		bool				mCalibrated;


	};
	Array<JoystickData>		mJoystickData;

	bool					HasAccelerometer();
	Vector					PollAccelerometer();
};

extern Input* gInputPtr;

inline bool IsKeyDown(int theKey) {return gInputPtr->KeyState(theKey);}
inline bool WasKeyPressed(int theKey) {return gInputPtr->KeyPressed(theKey);}
inline bool IsKeyPressed(int theKey) {return gInputPtr->KeyPressed(theKey);}

inline Point GetJoystickDir(int theJoystick, int whichThumbStick) {if (gInputPtr->PollJoystick(theJoystick)) return gInputPtr->mJoystickData[theJoystick].mStickDir[whichThumbStick]; return Point(0,0);}
inline bool IsJoystickButtonDown(int theJoystick, int theButton) {if (gInputPtr->PollJoystick(theJoystick)) {if (gInputPtr->mJoystickData[theJoystick].mButtonMap[theButton]<0) return false;return (gInputPtr->mJoystickData[theJoystick].mButton[gInputPtr->mJoystickData[theJoystick].mPollTime%2][gInputPtr->mJoystickData[theJoystick].mButtonMap[theButton]]!=0);}return false;}
inline bool IsJoystickButtonPressed(int theJoystick, int theButton) {if (theJoystick<0 || theButton<0 || theButton>=gInputPtr->mJoystickData[theJoystick].mButton[0].Size()) return false;if (gInputPtr->PollJoystick(theJoystick)) if (IsJoystickButtonDown(theJoystick,theButton)) return (gInputPtr->mJoystickData[theJoystick].mButton[gInputPtr->mJoystickData[theJoystick].mPollTime%2][gInputPtr->mJoystickData[theJoystick].mButtonMap[theButton]]!=gInputPtr->mJoystickData[theJoystick].mButton[1-(gInputPtr->mJoystickData[theJoystick].mPollTime%2)][gInputPtr->mJoystickData[theJoystick].mButtonMap[theButton]]);	return false;}
inline bool DoesJoystickButtonExist(int theJoystick, int theButton) {return (gInputPtr->mJoystickData[theJoystick].mButtonMap[theButton]>=0);}
//
// This allows you to quickly get joystick info, summarized.
// For instance, GetJoystickDir will give ALL sticks + hat as the result.
//
Point GetJoystickDir(int theJoystick); // Treats all sticks as the main stick
Point GetJoystickDirClampThumbsticks(int theJoystick,Point theClamp=Point(1,1)); // Clamps the thumbsticks to 1:1 ... this should happen normally?  Why did I include this?
inline Point GetJoystickHat(int theJoystick) {return gInputPtr->GetJoystickHat(theJoystick);}

