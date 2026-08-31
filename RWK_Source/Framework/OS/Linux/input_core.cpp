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
	Array<SDL_GameController*> gGameControllerList;
	Array<SDL_Joystick*> gJoystickList; // Fallback generic Joystick if GameController fails...
	void MapJoysticks();

	bool gIsStarted=false;
};

/*
#ifdef _STEAM
#include "isteaminput.h"
#include "isteamutils.h"
void SteamInputThread(void* theArg)
{
	if (SteamInput()==NULL)	return;
	if (!SteamInput()->Init(false)) {OS_Core::Printf("! FAILED SteamInput()");return;}

	int aMaxWait=0;
	int aControllers=0;
	while (aControllers==0)
	{
		SteamInput()->RunFrame();
		//Input_Core::gGamePadCount=SteamInput()->GetConnectedControllers(Input_Core::gGamePadList);
		aControllers=SDL_NumJoysticks();
		if (aControllers) break;

		if (aMaxWait>100) OS_Core::Sleep(100);
		else OS_Core::Sleep(50);
		aMaxWait++;
	}
	Echo("  Gamepads: %d   (Tries:%d)",aControllers,aMaxWait);
}
#endif
*/

void Input_Core::Startup()
{
	//
	// Don't do anything here... we init the GAMECONTROLLERS in the core,
	// and our joystick info happens in Query
	//
	//
	//*
	if (!gIsStarted)
	{
		SDL_Init(SDL_INIT_GAMECONTROLLER);
		if (SDL_NumJoysticks()>0)
		{
			//gJoystickList[SDL_NumJoysticks()-1]=NULL;
			for (int aCount=0;aCount<SDL_NumJoysticks();aCount++)
			{
				//gJoystickList[aCount]=NULL;
				gGameControllerList[aCount]=NULL;
			}
			MapJoysticks();
		}
	}
	/**/

	//OS_Core::Thread(SteamInputThread,NULL);
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
	//for (int aCount=0;aCount<gJoystickList.Size();aCount++) SDL_JoystickClose(gJoystickList[aCount]);
	for (int aCount=0;aCount<gGameControllerList.Size();aCount++) SDL_GameControllerClose(gGameControllerList[aCount]);
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
	memcpy(theResult,SDL_GetKeyboardState(&aKeys),MAXKEYS);
	memset(theResult+aKeys,0,MAXKEYS-aKeys); // Clear the rest of the buffer (Rapt_Input will help it out with mouse buttons)
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
		case KB_EXTRAMOUSE1: return "Mouse Extra 1";
		case KB_EXTRAMOUSE2: return "Mouse Extra 2";
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
	case KB_EXTRAMOUSE1: return "XBtn1";
	case KB_EXTRAMOUSE2: return "XBtn2";
	}

	return GetKeyName(theKey);
}

int Input_Core::GetJoystickCount()
{
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
	if (Input_Core::gGameControllerList[theNumber]==NULL && Input_Core::gJoystickList[theNumber]==NULL)
	{
		Input_Core::gGameControllerList[theNumber]=SDL_GameControllerOpen(theNumber);
		if (Input_Core::gGameControllerList[theNumber]==0)
		{
			OS_Core::Printf("This?");
			OS_Core::Printf("!   [%d]GameControllerProblem = %s",theNumber,SDL_GetError());
			Input_Core::gJoystickList[theNumber]=SDL_JoystickOpen(theNumber);
			if (Input_Core::gJoystickList[theNumber]==0)
			{
				OS_Core::Printf("!   [%d]Fallback joystick problem = %s",theNumber,SDL_GetError());
			}

		}
		//Echo("$GameController[%d] = %d",theNumber,Input_Core::gGameControllerList[theNumber]);
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
		//strcpy(theName,SDL_JoystickName(gJoystickList[theNumber]));
		if (gGameControllerList[theNumber]) strcpy(theName,SDL_GameControllerName(gGameControllerList[theNumber]));
		else if (gJoystickList[theNumber]) strcpy(theName,SDL_JoystickName(gJoystickList[theNumber]));
	}
}

bool Input_Core::PollJoystick(int theNumber, Array<float>& theAxes, Array<short>& theHats,Array<char> &theButtons)
{
	if (theNumber<0 || theNumber>=SDL_NumJoysticks()) return false;
	AttachJoystick(theNumber);

	if (gGameControllerList[theNumber])
	{
		SDL_JoystickUpdate();
		theAxes.GuaranteeSize(SDL_CONTROLLER_AXIS_MAX);
		int aAxisCount=0;
		int aValue;
		aValue=SDL_GameControllerGetAxis(gGameControllerList[theNumber],SDL_CONTROLLER_AXIS_LEFTX);theAxes[aAxisCount]=(float)aValue/32767.0f;if (theAxes[aAxisCount]<.25f && theAxes[aAxisCount]>-.25f) theAxes[aAxisCount]=0;aAxisCount++;
		aValue=SDL_GameControllerGetAxis(gGameControllerList[theNumber],SDL_CONTROLLER_AXIS_LEFTY);theAxes[aAxisCount]=(float)aValue/32767.0f;if (theAxes[aAxisCount]<.25f && theAxes[aAxisCount]>-.25f) theAxes[aAxisCount]=0;aAxisCount++;
		aValue=SDL_GameControllerGetAxis(gGameControllerList[theNumber],SDL_CONTROLLER_AXIS_RIGHTX);theAxes[aAxisCount]=(float)aValue/32767.0f;if (theAxes[aAxisCount]<.25f && theAxes[aAxisCount]>-.25f) theAxes[aAxisCount]=0;aAxisCount++;
		aValue=SDL_GameControllerGetAxis(gGameControllerList[theNumber],SDL_CONTROLLER_AXIS_RIGHTY);theAxes[aAxisCount]=(float)aValue/32767.0f;if (theAxes[aAxisCount]<.25f && theAxes[aAxisCount]>-.25f) theAxes[aAxisCount]=0;aAxisCount++;
		aValue=SDL_GameControllerGetAxis(gGameControllerList[theNumber],SDL_CONTROLLER_AXIS_TRIGGERLEFT);theAxes[aAxisCount]=(float)aValue/32767.0f;if (theAxes[aAxisCount]<.25f && theAxes[aAxisCount]>-.25f) theAxes[aAxisCount]=0;aAxisCount++;
		aValue=SDL_GameControllerGetAxis(gGameControllerList[theNumber],SDL_CONTROLLER_AXIS_TRIGGERRIGHT);theAxes[aAxisCount]=(float)aValue/32767.0f;if (theAxes[aAxisCount]<.25f && theAxes[aAxisCount]>-.25f) theAxes[aAxisCount]=0;aAxisCount++;

		int aButtonCount=0;
		theButtons.GuaranteeSize(SDL_CONTROLLER_BUTTON_MAX);
		theButtons[aButtonCount++]=SDL_GameControllerGetButton(gGameControllerList[theNumber],SDL_CONTROLLER_BUTTON_A);
		theButtons[aButtonCount++]=SDL_GameControllerGetButton(gGameControllerList[theNumber],SDL_CONTROLLER_BUTTON_B);
		theButtons[aButtonCount++]=SDL_GameControllerGetButton(gGameControllerList[theNumber],SDL_CONTROLLER_BUTTON_X);
		theButtons[aButtonCount++]=SDL_GameControllerGetButton(gGameControllerList[theNumber],SDL_CONTROLLER_BUTTON_Y);
		theButtons[aButtonCount++]=SDL_GameControllerGetButton(gGameControllerList[theNumber],SDL_CONTROLLER_BUTTON_BACK);
		theButtons[aButtonCount++]=SDL_GameControllerGetButton(gGameControllerList[theNumber],SDL_CONTROLLER_BUTTON_GUIDE);
		theButtons[aButtonCount++]=SDL_GameControllerGetButton(gGameControllerList[theNumber],SDL_CONTROLLER_BUTTON_START);
		theButtons[aButtonCount++]=SDL_GameControllerGetButton(gGameControllerList[theNumber],SDL_CONTROLLER_BUTTON_LEFTSTICK);
		theButtons[aButtonCount++]=SDL_GameControllerGetButton(gGameControllerList[theNumber],SDL_CONTROLLER_BUTTON_RIGHTSTICK);
		theButtons[aButtonCount++]=SDL_GameControllerGetButton(gGameControllerList[theNumber],SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
		theButtons[aButtonCount++]=SDL_GameControllerGetButton(gGameControllerList[theNumber],SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
		theButtons[aButtonCount++]=SDL_GameControllerGetButton(gGameControllerList[theNumber],SDL_CONTROLLER_BUTTON_DPAD_UP);
		theButtons[aButtonCount++]=SDL_GameControllerGetButton(gGameControllerList[theNumber],SDL_CONTROLLER_BUTTON_DPAD_DOWN);
		theButtons[aButtonCount++]=SDL_GameControllerGetButton(gGameControllerList[theNumber],SDL_CONTROLLER_BUTTON_DPAD_LEFT);
		theButtons[aButtonCount++]=SDL_GameControllerGetButton(gGameControllerList[theNumber],SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
		theButtons[aButtonCount++]=SDL_GameControllerGetButton(gGameControllerList[theNumber],SDL_CONTROLLER_BUTTON_MISC1);
		theButtons[aButtonCount++]=SDL_GameControllerGetButton(gGameControllerList[theNumber],SDL_CONTROLLER_BUTTON_PADDLE1);
		theButtons[aButtonCount++]=SDL_GameControllerGetButton(gGameControllerList[theNumber],SDL_CONTROLLER_BUTTON_PADDLE2);
		theButtons[aButtonCount++]=SDL_GameControllerGetButton(gGameControllerList[theNumber],SDL_CONTROLLER_BUTTON_PADDLE3);
		theButtons[aButtonCount++]=SDL_GameControllerGetButton(gGameControllerList[theNumber],SDL_CONTROLLER_BUTTON_PADDLE4);
		theButtons[aButtonCount++]=SDL_GameControllerGetButton(gGameControllerList[theNumber],SDL_CONTROLLER_BUTTON_TOUCHPAD);
		for (int aCount=aButtonCount;aCount<theButtons.Size();aCount++) theButtons[aCount]=0;
	}
	else if (gJoystickList[theNumber])
	{
		SDL_JoystickUpdate();
		int aAxisCount=SDL_JoystickNumAxes(gJoystickList[theNumber]);
		theAxes.GuaranteeSize(aAxisCount);
		for (int aCount=0;aCount<aAxisCount;aCount++)
		{
			theAxes[aCount]=(float)SDL_JoystickGetAxis(gJoystickList[theNumber],aCount)/32767.0f;
			if (theAxes[aCount]<.25f && theAxes[aCount]>-.25f) theAxes[aCount]=0;
		}

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
		return true;
	}

	/*
	char aDebug[1024];
	for (int aCount=0;aCount<theButtons.Size();aCount++)
	{
		if (theButtons[aCount]) aDebug[aCount]='1';
		else aDebug[aCount]='o';
		aDebug[aCount+1]=0;
	}
	OS_Core::Printf("Buttons: %s",aDebug);
	/**/



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

	axis1X=0;
	axis1Y=1;
	axis2X=2;
	axis2Y=3;
	trigger1=4;
	trigger2=5;

	buttonMap.GuaranteeSize(SDL_CONTROLLER_BUTTON_MAX);
	for (int aCount=0;aCount<buttonMap.Size();aCount++) buttonMap[aCount]=aCount;


/*
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

	buttonMap.GuaranteeSize(SDL_CONTROLLER_BUTTON_MAX);
	foreach(aMap,buttonMap) aMap=-1;
	for (int aCount=0;aCount<buttonMap.Size();aCount++) buttonMap[aCount]=SDL_GameControllerGetBindForButton(gGameControllerList[theNumber],(SDL_GameControllerButton)aCount).value.button;
	*/

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
	for (int aCount=0;aCount<buttonMap.Size();aCount++) OS_Core::Printf("* [Button%d] = %d",aCount,buttonMap[aCount]);
	/**/

	return true;
}

#define __HEADER
#include "../common.h"
#undef __HEADER
void Input_Core::MapJoysticks()
{
	char aFN[MAX_PATH];
	Common::FixPath("data://gamecontrollerdb.txt",aFN);
	int aResult=SDL_GameControllerAddMappingsFromFile(aFN);
	#ifdef _DEBUG
	if (aResult==-1) OS_Core::Printf("!Error: Unable to map controllers with data://gamecontrollerdb.txt");
	#endif
}

void* Input_Core::Query(char* theQuery)
{
	if (strcasecmp(theQuery,"REHUPCONTROLLERS")==0)
	{
		if (SDL_NumJoysticks()>0)
		{
			MapJoysticks();
			char aJName[256];
			for (int aCount=0;aCount<SDL_NumJoysticks();aCount++)
			{
				gGameControllerList[aCount]=NULL;
				gJoystickList[aCount]=NULL;
				AttachJoystick(aCount);
				if (gGameControllerList[aCount])
				{
					GetJoystickName(aCount,aJName);
				}
			}
		}
	}
	return PartnerQuery(theQuery);
}
