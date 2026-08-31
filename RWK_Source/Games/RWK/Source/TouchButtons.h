
#pragma once
#include "rapt.h"

class TouchButtons
{
public:
	TouchButtons(void);
	virtual ~TouchButtons(void);
	
	void			Update();

	bool			MouseDown(int x, int y);
	void			MouseMove(int x, int y);
	void			MouseUp(int x, int y);

	void			AddButton(int theID,Rect theRect, bool canHold=false);
	void			MoveButton(int theID,Point thePos);
	void			EnableButton(int theID, bool theState);
	bool			IsPressed(int theID, Point *theTouchPos=NULL);

	Rect			GetButtonRect(int theID);

	void			Reset();
	
public:

	struct ButtonStruct
	{
		ButtonStruct()
		{
			mCanHold=false;
			mIsPressed=false;
			mDisabled=false;
			mCooldown=gAppPtr->AppTime()-1000;
			mPressHash=-1;
		}

		int			mID;			// Button ID
		Rect		mRect;			// Rect of button
		bool		mIsPressed;		// Whether it's down
		Point		mTouchPos;		// Position it is being pressed on
		bool		mCanHold;		// If it can't be held, IsPressed will reset this
		bool		mDisabled;		// If disabled, it's ignored!
		
		int			mPressHash;		// Hash of what was pressed
		int			mCooldown;		// For special circumstances (such as a button being pushed
									// via MouseMove instead of MouseDown), there is a brief
									// cooldown so that the button doesn't get quickly pressed twice.
	};

	List			mButtonList;
	List			mCooldownList;

	ButtonStruct	*GetButton(int theX, int theY, bool allowClosest=false);
	ButtonStruct	*GetButtonFromHash(int theHash);
	ButtonStruct	*GetPressedButton(int theX, int theY, bool allowClosest=false);
	ButtonStruct	*GetNotPressedButton(int theX, int theY, bool allowClosest=false);
	
	void			Press(ButtonStruct *theButton, int x, int y);
	
};
