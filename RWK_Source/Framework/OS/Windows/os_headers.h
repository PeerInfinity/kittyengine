
//
// OS specific header files, to get at
// system stuff... set up for Windows, but the whole
// file will depend upon the OS itself, including the defines.
// 
// Put any defines, etc, here that will have the same name across all
// systems, but need to have different values.
//

//
// Win32 SDL using GL30...
// (Comment this out for compatibility mode for Chuzzle2, Hoggy2)
//
//#define LEGACY_GL
//#define RAPT_LEGACY

#ifndef LEGACY_GL
#ifndef GL30
#define GL30
#endif
#endif


#include "rapt_defines.h"

//
// Defines for Windows...
//
#define WIN32_LEAN_AND_MEAN             
#define _CRT_SECURE_NO_DEPRECATE 1

//#define NO_THREADS
#ifndef NO_THREADS
#define MULTICORE_LOAD
#endif

#define _compare strcmp
#define _comparen strncmp


#include <windows.h>
#undef DeleteFile
#undef CopyFile

#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <mmreg.h>
#include <math.h>
#include <mmsystem.h>
#include <functional>
#include <memory>
//
// RGB Helper (since some systems do them in reverse order)
//
#define RGBA_TO_INT(red,green,blue,alpha)						\
	(															\
		(((int)(alpha*255.0f)&0xFF)<<24)|						\
		(((int)(blue*255.0f)&0xFF)<<16)|						\
		(((int)(green*255.0f)&0xFF)<<8)|						\
		(((int)(red*255.0f)&0xFF))								\
	)

#define INT_TO_RGBA(myint, red, green, blue, alpha)				\
	alpha=(float)(((myint)&0xFF000000)>>24)/255.0f;		\
	blue=(float)(((myint)&0x00FF0000)>>16)/255.0f;			\
	green=(float)(((myint)&0x0000FF00)>>8)/255.0f;			\
	red=(float)(((myint)&0x000000FF))/255.0f;				


//
// HighByte and LowByte in case they're not defined...
//
#define LoByte(x)   ((x) & 0xFF)
#define HiByte(x)   (((x)>>8) & 0xFF)

//
// Some joystick helper info...
//
#define MAX_JOYSTICK_BUTTONS		32

#ifndef _max
#define _max(a,b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef _min
#define _min(a,b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef _abs
#define _abs(x) ((x) > 0) ? (x) : -(x)
#endif


//
// Keyboard Scan Codes
//
#define KB_ESCAPE          SDL_SCANCODE_ESCAPE
#define KB_1               SDL_SCANCODE_1
#define KB_2               SDL_SCANCODE_2
#define KB_3               SDL_SCANCODE_3
#define KB_4               SDL_SCANCODE_4
#define KB_5               SDL_SCANCODE_5
#define KB_6               SDL_SCANCODE_6
#define KB_7               SDL_SCANCODE_7
#define KB_8               SDL_SCANCODE_8
#define KB_9               SDL_SCANCODE_9
#define KB_0               SDL_SCANCODE_0
#define KB_MINUS           SDL_SCANCODE_MINUS  
#define KB_EQUALS          SDL_SCANCODE_EQUALS
#define KB_BACKSPACE       SDL_SCANCODE_BACKSPACE
#define KB_TAB             SDL_SCANCODE_TAB
#define KB_Q               SDL_SCANCODE_Q
#define KB_W               SDL_SCANCODE_W
#define KB_E               SDL_SCANCODE_E
#define KB_R               SDL_SCANCODE_R
#define KB_T               SDL_SCANCODE_T
#define KB_Y               SDL_SCANCODE_Y
#define KB_U               SDL_SCANCODE_U
#define KB_I               SDL_SCANCODE_I
#define KB_O               SDL_SCANCODE_O
#define KB_P               SDL_SCANCODE_P
#define KB_LBRACKET        SDL_SCANCODE_LEFTBRACKET
#define KB_RBRACKET        SDL_SCANCODE_RIGHTBRACKET
#define KB_LEFTBRACKET     SDL_SCANCODE_LEFTBRACKET
#define KB_RIGHTBRACKET    SDL_SCANCODE_RIGHTBRACKET
#define KB_RETURN          SDL_SCANCODE_RETURN
#define KB_ENTER           SDL_SCANCODE_RETURN
#define KB_LCONTROL        SDL_SCANCODE_LCTRL
#define KB_LEFTCONTROL     SDL_SCANCODE_LCTRL
#define KB_A               SDL_SCANCODE_A
#define KB_S               SDL_SCANCODE_S
#define KB_D               SDL_SCANCODE_D
#define KB_F               SDL_SCANCODE_F
#define KB_G               SDL_SCANCODE_G
#define KB_H               SDL_SCANCODE_H
#define KB_J               SDL_SCANCODE_J
#define KB_K               SDL_SCANCODE_K
#define KB_L               SDL_SCANCODE_L
#define KB_SEMICOLON       SDL_SCANCODE_SEMICOLON
#define KB_APOSTROPHE      SDL_SCANCODE_QUOTE
#define KB_TILDE           126
#define KB_LSHIFT          SDL_SCANCODE_LSHIFT
#define KB_LEFTSHIFT       SDL_SCANCODE_LSHIFT
#define KB_BACKSLASH       SDL_SCANCODE_BACKSLASH
#define KB_Z               SDL_SCANCODE_Z
#define KB_X               SDL_SCANCODE_X
#define KB_C               SDL_SCANCODE_C
#define KB_V               SDL_SCANCODE_V
#define KB_B               SDL_SCANCODE_B
#define KB_N               SDL_SCANCODE_N
#define KB_M               SDL_SCANCODE_M
#define KB_COMMA           SDL_SCANCODE_COMMA
#define KB_PERIOD          SDL_SCANCODE_PERIOD
#define KB_SLASH           SDL_SCANCODE_SLASH
#define KB_RSHIFT          SDL_SCANCODE_RSHIFT
#define KB_RIGHTSHIFT      SDL_SCANCODE_RSHIFT
#define KB_ASTERISK        SDL_SCANCODE_KP_MULTIPLY
#define KB_LEFTALT         SDL_SCANCODE_LALT
#define KB_SPACE           SDL_SCANCODE_SPACE
#define KB_CAPSLOCK        SDL_SCANCODE_CAPSLOCK
#define KB_F1              SDL_SCANCODE_F1
#define KB_F2              SDL_SCANCODE_F2
#define KB_F3              SDL_SCANCODE_F3
#define KB_F4              SDL_SCANCODE_F4
#define KB_F5              SDL_SCANCODE_F5
#define KB_F6              SDL_SCANCODE_F6
#define KB_F7              SDL_SCANCODE_F7
#define KB_F8              SDL_SCANCODE_F8
#define KB_F9              SDL_SCANCODE_F9
#define KB_F10             SDL_SCANCODE_F10
#define KB_NUMLOCK         SDL_SCANCODE_NUMLOCKCLEAR
#define KB_SCROLLLOCK	   SDL_SCANCODE_SCROLLLOCK
#define KB_NUMPAD7         SDL_SCANCODE_KP_7
#define KB_NUMPAD8         SDL_SCANCODE_KP_8
#define KB_NUMPAD9         SDL_SCANCODE_KP_9
#define KB_NUMPADMINUS     SDL_SCANCODE_KP_MINUS
#define KB_NUMPAD4         SDL_SCANCODE_KP_4
#define KB_NUMPAD5         SDL_SCANCODE_KP_5
#define KB_NUMPAD6         SDL_SCANCODE_KP_6
#define KB_NUMPADPLUS      SDL_SCANCODE_KP_PLUS
#define KB_NUMPAD1         SDL_SCANCODE_KP_1
#define KB_NUMPAD2         SDL_SCANCODE_KP_2
#define KB_NUMPAD3         SDL_SCANCODE_KP_3
#define KB_NUMPAD0         SDL_SCANCODE_KP_0
#define KB_NUMPADPERIOD    SDL_SCANCODE_PERIOD
#define KB_NUMPADSTAR		SDL_SCANCODE_KP_MULTIPLY
#define KB_NUMPADASTERISK	SDL_SCANCODE_KP_MULTIPLY
#define KB_NUMPADMULTIPLY	SDL_SCANCODE_KP_MULTIPLY
#define KB_F11             SDL_SCANCODE_F11
#define KB_F12             SDL_SCANCODE_F12
#define KB_F13             SDL_SCANCODE_F13
#define KB_F14             SDL_SCANCODE_F14
#define KB_F15             SDL_SCANCODE_F15
#define KB_NUMPADEQUALS    SDL_SCANCODE_KP_EQUALS
#define KB_NUMPADENTER     SDL_SCANCODE_KP_ENTER
#define KB_RCONTROL        SDL_SCANCODE_RCTRL
#define KB_RIGHTCONTROL    SDL_SCANCODE_RCTRL
#define KB_NUMPADDIVIDE    SDL_SCANCODE_KP_DIVIDE
#define KB_RIGHTALT        SDL_SCANCODE_RALT
#define KB_HOME            SDL_SCANCODE_HOME
#define KB_UP              SDL_SCANCODE_UP
#define KB_UPARROW         SDL_SCANCODE_UP
#define KB_PAGEUP          SDL_SCANCODE_PAGEUP
#define KB_PGUP            SDL_SCANCODE_PAGEUP
#define KB_LEFT            SDL_SCANCODE_LEFT
#define KB_LEFTARROW       SDL_SCANCODE_LEFT
#define KB_RIGHT           SDL_SCANCODE_RIGHT
#define KB_RIGHTARROW      SDL_SCANCODE_RIGHT
#define KB_END             SDL_SCANCODE_END
#define KB_DOWN            SDL_SCANCODE_DOWN
#define KB_DOWNARROW       SDL_SCANCODE_DOWN
#define KB_PAGEDOWN        SDL_SCANCODE_PAGEDOWN
#define KB_PGDOWN          SDL_SCANCODE_PGDOWN
#define KB_INSERT          SDL_SCANCODE_INSERT
#define KB_DELETE          SDL_SCANCODE_DELETE
#define KB_LEFTOS          SDL_SCANCODE_LGUI
#define KB_RIGHTOS         SDL_SCANCODE_RGUI
#define KB_APPS            SDL_SCANCODE_MODE

//
// Keyboard values for keypresses that come
// right from the system (use these in textboxes or
// other 'system' things)
//

#define CHAR_BACKSPACE      SDLK_DELETE
#define KEYDOWN_DELETE      SDLK_DELETE
#define KEYDOWN_LEFTARROW   SDLK_LEFT
#define KEYDOWN_RIGHTARROW  SDLK_RIGHT
#define KEYDOWN_UPARROW		SDLK_UP
#define KEYDOWN_DOWNARROW	SDLK_DOWN
#define KEYDOWN_END         SDLK_END
#define KEYDOWN_HOME        SDLK_HOME
#define KEYDOWN_PAGEUP		SDLK_PAGEUP
#define KEYDOWN_PAGEDOWN	SDLK_PAGEDOWN
#define KEYDOWN_TAB			SDLK_TAB


//
// A couple extra helpers...
// These let us get the mouse state as a keyboard
// event.  I put this in for Solomon Dark, so that
// the player can use the right mouse button for KB
// input, and not have to put in a seperate input
// method.
// 

#define KB_LEFTMOUSE		512
#define KB_RIGHTMOUSE		513
#define KB_MIDDLEMOUSE		514
//
// Port defines... eventually you will remove all of these
//
#include "include/SDL.h"
#include "include/SDL_image.h"

#define ThreadLocal __thread

#pragma warning (disable: 4996)

#define WARNING_Stringize( L )     #L 
#define WARNING_MakeString( M, L ) M(L)
#define WARNING_$Line WARNING_MakeString( WARNING_Stringize, __LINE__ )
#define WARNING_Reminder __FILE__ "(" WARNING_$Line ") : Reminder: "
#define WARNING(text) __pragma(message(WARNING_Reminder text "<-------------------------------------------------------------------------------------------------"))
