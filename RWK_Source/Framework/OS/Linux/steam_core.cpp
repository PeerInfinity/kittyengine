#include "os_headers.h"
#ifdef _STEAM
#include "steam_api.h"
#include "isteaminput.h"
#include "isteamutils.h"
#include "os_core.h"

bool gSteamOK=false;
bool gSteamStartedConnecting=false;
bool gSteamConnecting=true;
bool gSteamOverlayShowing=false;
int gSteamAppID=-1;

void SteamStartup();
void SteamShutdown();
void SteamPump();

//
// If we need them...
//
void* PartnerQuery(char* theQuery, void* extraInfo) {return NULL;}
char* PartnerQueryString(char* theQuery, void* extraInfo) {return "";}
void PartnerQueryBytes(char* theQuery, void** thePtr, int* theSize) {}

char* GetGlyph(int theController, int theAction);


void* PartnerQuery(longlong theHash,...)
{
	return NULL;
}

class SteamHandler
{
public:
	STEAM_CALLBACK(SteamHandler, OnUserStatsReceived, UserStatsReceived_t, mUserStatsReceived);
	STEAM_CALLBACK(SteamHandler,OnInputDismissed,GamepadTextInputDismissed_t);
	STEAM_CALLBACK(SteamHandler, OnGameOverlayActivated, GameOverlayActivated_t);

	SteamHandler() :
	mUserStatsReceived( this, &SteamHandler::OnUserStatsReceived )
	{
	}

};

void SteamHandler::OnGameOverlayActivated( GameOverlayActivated_t* pCallback )
{
	gSteamOverlayShowing=pCallback->m_bActive;
	if (!pCallback->m_bActive) OS_Core::PushOSMessage(OS_Core::OS_MULTITASKING,0,true);
	else OS_Core::PushOSMessage(OS_Core::OS_MULTITASKING,0,false);
}

void SteamHandler::OnUserStatsReceived(UserStatsReceived_t *theCallback)
{
}

void SteamHandler::OnInputDismissed(GamepadTextInputDismissed_t* theParam)
{
}

SteamHandler* gSteamHandler=NULL;
namespace Input_Core
{
	extern bool gIsStarted;
}

void SteamStartup()
{
}

void SteamShutdown()
{
}

void SteamPump() //::SteamPump
{
}

/*
void			SubmitAchievement(char *theAchievement, float thePercentComplete)
{
}

void ShowKeyboard(bool theState)
{
	SteamUtils()->ShowGamepadTextInput(EGamepadTextInputMode::k_EGamepadTextInputModeNormal, EGamepadTextInputLineMode::k_EGamepadTextInputLineModeSingleLine, "pchDescription", 99, "pchExistingText");
}
*/

// SDL mappings for us to help with...
enum
{
	JOYBUTTON_A=0,
	JOYBUTTON_B,
	JOYBUTTON_X,
	JOYBUTTON_Y,
	JOYBUTTON_BACK,
	JOYBUTTON_GUIDE,
	JOYBUTTON_HOME=JOYBUTTON_GUIDE,
	JOYBUTTON_START,
	JOYBUTTON_LEFTSTICK,
	JOYBUTTON_RIGHTSTICK,
	JOYBUTTON_LEFTSHOULDER,
	JOYBUTTON_RIGHTSHOULDER, //10
	// On some joysticks, buttons end here
	JOYBUTTON_DPADUP,
	JOYBUTTON_DPADDOWN,
	JOYBUTTON_DPADLEFT,
	JOYBUTTON_DPADRIGHT,
	JOYBUTTON_MISC1,		// 15
	JOYBUTTON_PADDLE1,
	JOYBUTTON_PADDLE2,
	JOYBUTTON_PADDLE3,
	JOYBUTTON_PADDLE4,
	JOYBUTTON_TOUCHPAD,		// 20
	// Added these myself, so I can use triggers as buttons.
	JOYBUTTON_LEFTTRIGGER,
	JOYBUTTON_RIGHTTRIGGER,

	JOYBUTTON_MAX,

	THUMBSTICK1=JOYBUTTON_MAX,
	THUMBSTICK2,

	JOYMAPPINGS_MAX
};

char* GetGlyph(int theController, int theAction)
{
	//
	// SPECIAL NOTE!
	// During DEV this will NOT work correctly.  It's going to return unknown joysticks for everything.
	// When you publish and run it through Steam, it gives you the correct stuff.  So don't flip out when
	// you're developing and it says joysticks are unknown.  When you publish it will "know" them.
	//
	InputHandle_t aHandle = SteamInput()->GetControllerForGamepadIndex(theController);
	ESteamInputType aType = SteamInput()->GetInputTypeForHandle(aHandle);

	static Array<EInputActionOrigin> aActions;
	EInputActionOrigin aAction;
	if (aActions.Size()==0)
	{
		aActions.GuaranteeSize(JOYMAPPINGS_MAX);
		aActions[JOYBUTTON_A]=k_EInputActionOrigin_XBox360_A;
		aActions[JOYBUTTON_B]=k_EInputActionOrigin_XBox360_B;
		aActions[JOYBUTTON_X]=k_EInputActionOrigin_XBox360_X;
		aActions[JOYBUTTON_Y]=k_EInputActionOrigin_XBox360_Y;
		aActions[JOYBUTTON_BACK]=k_EInputActionOrigin_XBox360_Back;
		aActions[JOYBUTTON_GUIDE]=k_EInputActionOrigin_Switch_Capture;
		aActions[JOYBUTTON_START]=k_EInputActionOrigin_XBox360_Start;
		aActions[JOYBUTTON_LEFTSTICK]=k_EInputActionOrigin_XBox360_LeftStick_Click;
		aActions[JOYBUTTON_RIGHTSTICK]=k_EInputActionOrigin_XBox360_RightStick_Click;
		aActions[JOYBUTTON_LEFTSHOULDER]=k_EInputActionOrigin_XBox360_LeftBumper;
		aActions[JOYBUTTON_RIGHTSHOULDER]=k_EInputActionOrigin_XBox360_RightBumper;
		aActions[JOYBUTTON_DPADUP]=k_EInputActionOrigin_XBox360_DPad_North;
		aActions[JOYBUTTON_DPADDOWN]=k_EInputActionOrigin_XBox360_DPad_South;
		aActions[JOYBUTTON_DPADLEFT]=k_EInputActionOrigin_XBox360_DPad_West;
		aActions[JOYBUTTON_DPADRIGHT]=k_EInputActionOrigin_XBox360_DPad_East;
		aActions[JOYBUTTON_MISC1]=k_EInputActionOrigin_None;
		aActions[JOYBUTTON_PADDLE1]=k_EInputActionOrigin_None;
		aActions[JOYBUTTON_PADDLE2]=k_EInputActionOrigin_None;
		aActions[JOYBUTTON_PADDLE3]=k_EInputActionOrigin_None;
		aActions[JOYBUTTON_PADDLE4]=k_EInputActionOrigin_None;
		aActions[JOYBUTTON_TOUCHPAD]=k_EInputActionOrigin_None;
		aActions[JOYBUTTON_LEFTTRIGGER]=k_EInputActionOrigin_XBox360_LeftTrigger_Click;
		aActions[JOYBUTTON_RIGHTTRIGGER]=k_EInputActionOrigin_XBox360_RightTrigger_Click;
		aActions[JOYBUTTON_MAX]=k_EInputActionOrigin_None;
		aActions[THUMBSTICK1]=k_EInputActionOrigin_XBox360_LeftStick_Move;
		aActions[THUMBSTICK2]=k_EInputActionOrigin_XBox360_RightStick_Move;
	}
	if (theAction>=0 && theAction<aActions.Size())
	{
		aAction=aActions[theAction];
		EInputActionOrigin aNew=SteamInput()->TranslateActionOrigin(aType,aAction);
		char* aResult=(char*)SteamInput()->GetGlyphForActionOrigin_Legacy(aNew);
		return aResult;
	}
	return NULL;
}

#endif


