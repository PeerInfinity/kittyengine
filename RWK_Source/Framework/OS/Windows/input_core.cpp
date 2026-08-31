#include "input_core.h"
#include "os_core.h"

#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>


namespace Input_Core
{
	Array<SDL_Joystick*> gJoystickList;
	Array<SDL_GameController*> gGameControllerList;
#ifdef _STEAM
	struct SteamJoyInfo
	{
		InputHandle_t mHandle=NULL;
		InputHandle_t mActionSetHandle=NULL;
	};

	Array<SteamJoyInfo> gSteamJoystickList;
#endif

	void MapJoysticks();
};

#ifdef _STEAM
//
// Crazy Steam thread to watch for us losing our SDL joysticks.
//
bool gUseSteamInput=false;
bool gCheckingSteamInput=true;
void SteamInitThread(void* theArg)
{
	static int aTickTock=OS_Core::Tick();
	for (;;)
	{
//*
#ifdef _DEBUG
		OS_Core::Printf("+_SteamInitThread...[%d]",SDL_NumJoysticks());
#endif
/**/
		if (SDL_NumJoysticks()==0)
		{
			InputHandle_t aJoystickHandle[STEAM_INPUT_MAX_COUNT];
			int aControllers=SteamInput()->GetConnectedControllers(aJoystickHandle);


			Input_Core::gSteamJoystickList.GuaranteeSize(aControllers);
			for (int aCount=0;aCount<aControllers;aCount++) 
			{
				Input_Core::gSteamJoystickList[aCount].mHandle=aJoystickHandle[aCount];
				Input_Core::gSteamJoystickList[aCount].mActionSetHandle=SteamInput()->GetActionSetHandle("GameControls");

				OS_Core::Printf("Bullshit: %ld %ld",SteamInput()->GetActionSetHandle("GameControls"),SteamInput()->GetActionSetHandle("MenuControls"));
			}
#ifdef _DEBUG
			OS_Core::Printf("$ Switching to Steam Controllers = %d",aControllers);
#endif
			gUseSteamInput=(aControllers>0);
			//SteamInput()->ShowBindingPanel(Input_Core::gSteamJoystickList[0]);
			if (gUseSteamInput) break; // We can't reliably stop unless this is true...
		}
		OS_Core::Sleep(25);
		if ((OS_Core::Tick()-aTickTock)>60000*2) // Two minutes
		{
#ifdef _DEBUG
			OS_Core::Printf("* Timeout of Steam Controller watch thread");
#endif
			break;
		}
	}
	gCheckingSteamInput=false;
}
#endif

void Input_Core::Startup()
{
#ifdef _DEBUG
	OS_Core::Printf("$Inpute_Core::Startup()");
#endif
	SDL_Init(SDL_INIT_GAMECONTROLLER);
	if (SDL_NumJoysticks()>0)
	{
		gJoystickList[SDL_NumJoysticks()-1]=NULL;
		for (int aCount=0;aCount<SDL_NumJoysticks();aCount++) 
		{
			gJoystickList[aCount]=NULL;
			gGameControllerList[aCount]=NULL;
		}
		MapJoysticks();
	}

#ifdef _STEAM
	//
	// If steam, try to invoke SteamInput... 
	//
	if (SteamInput()==NULL) {OS_Core::MessageBoxA("REQUIRES STEAM","Steam must be running to play this!",0);exit(0);}
	if (!SteamInput()->Init(true)) OS_Core::Printf("! FAILED SteamInput()");
	else OS_Core::Printf("_Starting watching for SteamInput...");

	OS_Core::Thread(SteamInitThread,NULL);
#endif

}

void Input_Core::Shutdown()
{
	//
	// To shut down the unput subsystem, if need be.
	//
	// #ifdef _PORT_INPUT
	// !ERROR
	// #endif
	//
#ifdef _STEAM
	if (gUseSteamInput) 
	{
		SteamInput()->Shutdown();
		return;
	}
#endif
	for (int aCount=0;aCount<gJoystickList.Size();aCount++) SDL_JoystickClose(gJoystickList[aCount]);
}

void Input_Core::Multitasking(bool isForeground)
{
	//
	// To do anything the input subsystem on this port needs to do in case of lost or gained focus...
	//
	// #ifdef _PORT_MULTITASK
	// !ERROR
	// #endif
	//
}

void Input_Core::PollKeyboard(char *theResult)
{
	//
	// Get the state of all keys on the keyboard.  The user will usually send in &char[512] as theResult.
	//
	// #ifdef _PORT_KEYBOARD
	// !ERROR
	// #endif
	//
	int aKeys;
	memcpy(theResult,SDL_GetKeyboardState(&aKeys),512);
	memset(theResult+aKeys,0,512-aKeys);
}

void Input_Core::AcquireKeyboard()
{
	//
	// On some systems, it's necessary to "acquire" the keyboard focus from other apps.  
	// This provides a place for the framework to acquire it when necessary (on startup, or on
	// recovery of focus)
	//
	// #ifdef _PORT_KEYBOARD
	// !ERROR
	// #endif
	//

}

char* Input_Core::GetKeyName(int theKey)
{
	//
	// Get the name of keys here.  Note that the framework will expect to pass KB_LEFTMOUSE, KB_RIGHTMOUSE, and KB_MIDDLEMOUSE
	// (as defined in os_headers.h) here to get a text version of it.
	//
	// #ifdef _PORT_KEYBOARD
	// !ERROR
	// #endif
	//

	switch (theKey)
	{
	case KB_LEFTMOUSE: return "Left Mouse";
	case KB_RIGHTMOUSE: return "Right Mouse";
	case KB_MIDDLEMOUSE: return "Middle Mouse";
	}

	return (char*)SDL_GetKeyName(SDL_GetKeyFromScancode((SDL_Scancode)theKey));
}

char* Input_Core::GetShortKeyName(int theKey)
{
	//
	// This just returns some shortened versions of certain key names.  Some names are pretty long, so if you need to squeeze them
	// into a small space, this is just a translation layer.  Likely you won't need to change anything in here, but if you do, here's where it goes.
	//
	// #ifdef _PORT_KEYBOARD
	// !ERROR
	// #endif
	//


	switch (theKey)
	{
	case KB_ESCAPE: return "Esc";
	case KB_BACKSPACE: return "BSpc";
	case KB_RETURN: return "Ret";
	case KB_LCONTROL: return "LCtrl";
	case KB_LSHIFT: return "LShft";
	case KB_RSHIFT: return "RShft";
	case KB_LEFTALT: return "LAlt";
	case KB_SPACE: return "Spc";
	case KB_CAPSLOCK: return "Caps";
	case KB_NUMLOCK: return "NLock";
	case KB_SCROLLLOCK: return "SLock";
	case KB_NUMPAD7: return "NP7";
	case KB_NUMPAD8: return "NP8";
	case KB_NUMPAD9: return "NP9";
	case KB_NUMPADMINUS: return "NP-";
	case KB_NUMPAD4: return "NP4";
	case KB_NUMPAD5: return "NP5";
	case KB_NUMPAD6: return "NP6";
	case KB_NUMPADPLUS: return "NP+";
	case KB_NUMPAD1: return "NP1";
	case KB_NUMPAD2: return "NP2";
	case KB_NUMPAD3: return "NP3";
	case KB_NUMPAD0: return "NP0";
	case KB_NUMPADPERIOD: return "NP.";
	case KB_NUMPADEQUALS: return "NP=";
	case KB_NUMPADENTER: return "NPRet";
	case KB_RCONTROL: return "RCtrl";
	case KB_NUMPADDIVIDE: return "NP/";
	case KB_NUMPADSTAR: return "NP*";
	case KB_RIGHTALT: return "RAlt";
	case KB_HOME: return "Home";
	case KB_UP: return "Up";
	case KB_PAGEUP: return "PgUp";
	case KB_LEFT: return "Left";
	case KB_RIGHT: return "Right";
	case KB_END: return "End";
	case KB_DOWN: return "Down";
	case KB_PAGEDOWN: return "PgDn";
	case KB_INSERT: return "Ins";
	case KB_DELETE: return "Del";
	case KB_LEFTOS: return "LWin";
	case KB_RIGHTOS: return "RWin";
	case KB_APPS: return "Apps";
	case KB_LEFTMOUSE: return "LBtn";
	case KB_RIGHTMOUSE: return "RBtn";
	case KB_MIDDLEMOUSE: return "MBtn";
	}

	return GetKeyName(theKey);
}

int Input_Core::GetJoystickCount()
{
#ifdef _STEAM
	if (gUseSteamInput) return gJoystickList.Size();
#endif

	//
	// Just returns the number of joysticks attached to the system
	//
	// #ifdef _PORT_JOYSTICK
	// !ERROR
	// #endif
	//
	return SDL_NumJoysticks();
}

void AttachJoystick(int theNumber)
{
	if (Input_Core::gJoystickList[theNumber]==NULL) 
	{
		Input_Core::gJoystickList[theNumber]=SDL_JoystickOpen(theNumber);
		Input_Core::gGameControllerList[theNumber]=SDL_GameControllerOpen(theNumber);

		if (Input_Core::gGameControllerList[theNumber]==0) OS_Core::Printf("!GameControllerProblem = %s",SDL_GetError());
		else OS_Core::Printf("$GameController[%d] = %d",theNumber,Input_Core::gGameControllerList[theNumber]);

	}
}

void Input_Core::GetJoystickName(int theNumber, char *theName)
{
	//
	// Returns the name of an attached joystick
	//
	// #ifdef _PORT_JOYSTICK
	// !ERROR
	// #endif
	//
	if (theNumber<0 || theNumber>=SDL_NumJoysticks()) strcpy(theName,"Unknown");
	else 
	{
		AttachJoystick(theNumber);
		strcpy(theName,SDL_JoystickName(gJoystickList[theNumber]));
	}

}

bool Input_Core::PollJoystick(int theNumber, Array<float>& theAxes, Array<short>& theHats,Array<char> &theButtons)
{
	//
	// Poll the joystick for stick/button information
	//
	// #ifdef _PORT_JOYSTICK
	// !ERROR
	// #endif
	//
	if (theNumber<0 || theNumber>=SDL_NumJoysticks()) return false;
	AttachJoystick(theNumber);

	SDL_JoystickUpdate();
	int aAxisCount=SDL_JoystickNumAxes(gJoystickList[theNumber]);
	theAxes.GuaranteeSize(aAxisCount);
	for (int aCount=0;aCount<aAxisCount;aCount++) 
	{
		int aValue=SDL_JoystickGetAxis(gJoystickList[theNumber],aCount);
		/*
		if (aValue>32767)
		{
			aValue-=32767;
			aValue=(32767-aValue)*-1;
		}
		*/
		theAxes[aCount]=(float)aValue/32767.0f;
		if (theAxes[aCount]<.25f && theAxes[aCount]>-.25f) theAxes[aCount]=0;
	}

	// Stick 2
	/*
	OS_Core::Printf("Fact: %d,%d",
		SDL_JoystickGetAxis(gJoystickList[theNumber],3),
		SDL_JoystickGetAxis(gJoystickList[theNumber],4)
	);
	*/
	//OS_Core::Printf("Fact: %d,%d",SDL_JoystickGetAxis(gJoystickList[theNumber],2));

	int aHatCount=SDL_JoystickNumHats(gJoystickList[theNumber]);
	theHats.GuaranteeSize(aHatCount);
	for (int aCount=0;aCount<aHatCount;aCount++)
	{
		Uint8 aHat=SDL_JoystickGetHat(gJoystickList[theNumber],aCount);
		signed char* aHatX=(signed char*)&theHats[aCount];
		signed char* aHatY=(signed char*)(&theHats[aCount])+1;
		switch (aHat)
		{
		case SDL_HAT_CENTERED:*aHatX=0;*aHatY=0;break;
		case SDL_HAT_UP:*aHatX=0;*aHatY=-1;break;
		case SDL_HAT_RIGHTUP:*aHatX=1;*aHatY=-1;break;
		case SDL_HAT_RIGHT:*aHatX=1;*aHatY=0;break;
		case SDL_HAT_RIGHTDOWN:*aHatX=1;*aHatY=1;break;
		case SDL_HAT_DOWN:*aHatX=0;*aHatY=1;break;
		case SDL_HAT_LEFTDOWN:*aHatX=-1;*aHatY=1;break;
		case SDL_HAT_LEFT:*aHatX=-1;*aHatY=0;break;
		case SDL_HAT_LEFTUP:*aHatX=-1;*aHatY=-1;break;
		}
	}

	int aButtonCount=SDL_JoystickNumButtons(gJoystickList[theNumber]);
	theButtons.GuaranteeSize(aButtonCount);
	for (int aCount=0;aCount<aButtonCount;aCount++) theButtons[aCount]=SDL_JoystickGetButton(gJoystickList[theNumber],aCount);
	for (int aCount=aButtonCount;aCount<theButtons.Size();aCount++) theButtons[aCount]=0;
	return true;
}

bool Input_Core::CalibrateJoystick(int theNumber, char& axis1X, char& axis1Y, char& axis2X, char& axis2Y, char& trigger1, char& trigger2,Array<char>& buttonMap)
{
	AttachJoystick(theNumber);
	//
	// A default calibration for the joystick... if this system has a guaranteed specific joystick (like XBox) then return
	// true.  If it will end up requriing calibration, give XBox360 default numbers and return false.
	//
	// #ifdef _PORT_JOYSTICK
	// !ERROR
	// #endif
	//

	// X-Input standard as of 2022
	axis1X=0;
	axis1Y=1;
	axis2X=4;
	axis2Y=5;
	trigger1=3;
	trigger2=6;

	axis1X=SDL_GameControllerGetBindForAxis(gGameControllerList[theNumber],SDL_CONTROLLER_AXIS_LEFTX).value.axis;
	axis1Y=SDL_GameControllerGetBindForAxis(gGameControllerList[theNumber],SDL_CONTROLLER_AXIS_LEFTY).value.axis;
	axis2X=SDL_GameControllerGetBindForAxis(gGameControllerList[theNumber],SDL_CONTROLLER_AXIS_RIGHTX).value.axis;
	axis2Y=SDL_GameControllerGetBindForAxis(gGameControllerList[theNumber],SDL_CONTROLLER_AXIS_RIGHTY).value.axis;
	trigger1=SDL_GameControllerGetBindForAxis(gGameControllerList[theNumber],SDL_CONTROLLER_AXIS_TRIGGERLEFT).value.axis;
	trigger2=SDL_GameControllerGetBindForAxis(gGameControllerList[theNumber],SDL_CONTROLLER_AXIS_TRIGGERRIGHT).value.axis;

	buttonMap.GuaranteeSize(SDL_JoystickNumButtons(gJoystickList[theNumber]));
	foreach(aMap,buttonMap) aMap=-1;
	for (int aCount=0;aCount<buttonMap.Size();aCount++) buttonMap[aCount]=SDL_GameControllerGetBindForButton(gGameControllerList[theNumber],(SDL_GameControllerButton)aCount).value.button;

	/*
	char* aBName[]={"SDL_CONTROLLER_BUTTON_A","SDL_CONTROLLER_BUTTON_B","SDL_CONTROLLER_BUTTON_X","SDL_CONTROLLER_BUTTON_Y","SDL_CONTROLLER_BUTTON_BACK","SDL_CONTROLLER_BUTTON_GUIDE","SDL_CONTROLLER_BUTTON_START","SDL_CONTROLLER_BUTTON_LEFTSTICK","SDL_CONTROLLER_BUTTON_RIGHTSTICK","SDL_CONTROLLER_BUTTON_LEFTSHOULDER","SDL_CONTROLLER_BUTTON_RIGHTSHOULDER","SDL_CONTROLLER_BUTTON_DPAD_UP","SDL_CONTROLLER_BUTTON_DPAD_DOWN","SDL_CONTROLLER_BUTTON_DPAD_LEFT","SDL_CONTROLLER_BUTTON_DPAD_RIGHT","SDL_CONTROLLER_BUTTON_MISC1","SDL_CONTROLLER_BUTTON_PADDLE1","SDL_CONTROLLER_BUTTON_PADDLE2","SDL_CONTROLLER_BUTTON_PADDLE3","SDL_CONTROLLER_BUTTON_PADDLE4","SDL_CONTROLLER_BUTTON_TOUCHPAD"};
	OS_Core::Printf("*----------Calibrate Controller [%d]-------",gGameControllerList[theNumber]);
	char* aMap=SDL_GameControllerMapping(Input_Core::gGameControllerList[theNumber]);
	OS_Core::Printf("*Map = %s",aMap);
	OS_Core::Printf("*stick1x = %d",axis1X);
	OS_Core::Printf("*stick1y = %d",axis1Y);
	OS_Core::Printf("*stick2x = %d",axis2X);
	OS_Core::Printf("*stick2y = %d",axis2Y);
	OS_Core::Printf("*left trigger = %d",trigger1);
	OS_Core::Printf("*right trigger = %d",trigger2);
	OS_Core::Printf("*Buttons = %d",SDL_JoystickNumButtons(gJoystickList[theNumber]));
	for (int aCount=0;aCount<buttonMap.Size();aCount++) OS_Core::Printf("* [%s] = %d",aBName[aCount],buttonMap[aCount]);
	/**/

	return true;
}

#define __HEADER
#include "..\common.h"
#undef __HEADER
void Input_Core::MapJoysticks()
{
	char aFN[MAX_PATH];
	Common::FixPath("data://controllers.txt",aFN);
	int aResult=SDL_GameControllerAddMappingsFromFile(aFN);
	if (aResult==-1) OS_Core::Printf("!Error: Unable to map controllers with data://controllers.txt");
}

void* Input_Core::Query(char* theQuery)
{
	if (theQuery=="STEAMINPUT?")
	{
#ifdef _STEAM
		if (gCheckingSteamInput) return (void*)"Init";
		if (gUseSteamInput) return (void*)"Yes";
#endif
		return (void*)"No";

	}
	return NULL;
}

//
// STEAMINPUT stuff:
// https://partner.steamgames.com/doc/features/steam_controller/getting_started_for_devs#implement_steam_input
// 
