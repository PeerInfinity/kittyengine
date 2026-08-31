#pragma once
#include "os_headers.h"
#include "util_core.h"

namespace Input_Core
{
	//
	// Starts up and shutdowns the input system.  Does whatever needs to be done
	// on the OS to start handling input and whatnot...
	//
	void			Startup();
	void			Shutdown();
	void			Multitasking(bool isForeground);

	//
	// Gets the keyboard state
	//
	void			PollKeyboard(char *theResult);

	//
	// Acquire the keyboard
	// 
	void			AcquireKeyboard();

	//
	// For getting the name of keys
	//
	char			*GetKeyName(int theKey);
	char			*GetShortKeyName(int theKey);

	//
	// Joystick functions
	//
	int				GetJoystickCount();
	void			GetJoystickName(int theNumber, char *theName);
	bool			PollJoystick(int theNumber, Array<float>& theAxes, Array<short>& theHats, Array<char>& theButtons);

	//
	// Call this to get an axis calibration.  If it returns false, it gives a "default" setup, but the program should ask
	// them to calibrate (wiggle stick1, wiggle stick2, press trigger)
	//
	bool			CalibrateJoystick(int theNumber, char& axis1X, char& axis1Y, char& axis2X, char& axis2Y, char& trigger1, char& trigger2, Array<char>& buttonMap);

	//
	// Tilt/accelerometer functions
	//
	inline bool		HasAccelerometer() {return false;}
	inline void		PollAccelerometer(float* x, float* y, float* z) {}

	//
	// misc Query
	//
	void*			Query(char* theQuery);
}
