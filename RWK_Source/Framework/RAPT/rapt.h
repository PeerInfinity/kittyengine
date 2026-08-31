//
// Include this file in classes...
//

#ifdef _WIN32
#include "os_headers.h"
#endif
#include "rapt_headers.h"

#ifdef _DEBUG
#define SetCPUName(cpu,thename) {cpu->mName=thename;}
#else
#define SetCPUName(cpu,thename) {}
#endif

//
// For anonymous functions...
// ANON((String theString){printf("%s",theString.c());})
// FUNC((String theString){printf("%s",theString.c());})
//
/*
#define ANON(x) [] x	// Don't use ANON, use HOOK
#define FUNC(x) [] x
#define QUICKFUNC(x) [] x
#define ANONFUNC(x) [] x
#define HOOK(body) ANON((void* theThis) body)
typedef void (*HookPtr)(void* theThis);
*/

#define HOOK(func) [&](void) func
#define HOOKPTR std::function<void(void)>
#define HOOKLOCAL(func) [=](void) func

#define LOSHORT(a) ((short)(a))
#define HISHORT(a) ((short)(((long)(a) >> 16) & 0xFFFF))
#define LOWSHORT(a) ((short)(a))
#define HIGHSHORT(a) ((short)(((long)(a) >> 16) & 0xFFFF))
#define MAKEINT(low,hi) ((short)low|((short)hi<<16))

#define LOCHAR(a) ((char)(a))
#define HICHAR(a) ((char)(((short)(a) >> 8) & 0xFF))
#define LOWCHAR(a) ((char)(a))
#define HIGHCHAR(a) ((char)(((short)(a) >> 8) & 0xFF))
#define MAKESHORT(low,hi) ((short)low|((short)hi<<8))


//
// Quickie hook helpers...
//
#define HOOKBOOL(func) [&](bool theValue) func
#define HOOKBOOLPTR std::function<void(bool theValue)>
#define HOOKCHAR(func) [&](char theValue) func
#define HOOKCHARPTR std::function<void(char theValue)>
#define HOOKINT(func) [&](int theValue) func
#define HOOKINTPTR std::function<void(int theValue)>
#define HOOKFLOAT(func) [&](float theValue) func
#define HOOKFLOATPTR std::function<void(float theValue)>
#define HOOKOBJECT(class,func) [&](class* theValue) func
#define HOOKOBJECTPTR(class) std::function<void(class* theValue)>
#define HOOKSTRING(func) [&](String theString) func
#define HOOKSTRINGPTR std::function<void(String theString)>

#ifndef HOOKARG
#define HOOKARG(func) [&](void* theArg) func
#define HOOKARGG(func) [](void* theArg) func
#define HOOKARGPTR std::function<void(void* theArg)>
#endif


#define SQUARED(x) ((x)*(x))
#define MID(x,y) (((x)+(y))/2)
#define SNAPFPS(x) (Snap(x,.00001f))

#define AVG(x,y) (((x)+(y))/2)
#define AVG2(x,y) (((x)+(y))/2)
#define AVG3(x,y,z) (((x)+(y)+(z))/3)


#define INITARRAY(type) std::initializer_list<type>

//
// A helper for Array sorts.  Whatever object you are sorting, you just need to put
// SORTHELPER(myClass,mySortVariable) in the class definition and arrays will sort okay.
//
#define SORTHELPER(classname,varname) inline bool operator<(classname& theOther){return varname<theOther.varname;}

//
// Example Hook:
//
// HOOKPTR aFunc; // HOOKPTR is a variable to story a hook in...
// aFunc=HOOK({gOut.Out("This is a test! %d",this);})
// aFunc();
//

#define HOOKEXPTR(param) std::function<void param>
#define HOOKEX(param, func) [&] param func
//
// Example Hook with params... (put the whole param stack in parenthesis so the define doesn't trip on the commas)
//
// HOOKEXPTR((void* theArg)) aFunc;
// aFunc=HOOKEX((void* theArg) {gOut.Out("Arg = %d",theArg);})
// aFunc(ptr);
//

#define IOHOOK(func) [=](void* theIn)->void* func
#define IOHOOKPTR std::function<void*(void* theIn)>
//
// IOHOOK lets you pass a void* in, and get a void* out.
// Make sure what you're returning is static or otherwise non-local tho!
//

//
// Expose the Common namespace so that we can hit it up if we want to...
//
namespace Common
{
	void FixPath(char* thePath, char* theResult);
	void GetCustomPath(char* thePrefix, char* theResult);
}




//
// Most common includes, so that we typically only need to include "rapt.h"
//

#include "rapt_app.h"
#include "rapt_graphics.h"
#include "rapt_audio.h"
#include "rapt_input.h"
#include "rapt_CPUHelpers.h"
#include "rapt_utils.h"
#include "rapt_social.h"
#include "rapt_comm.h"

#ifndef SETBITS
#define SETBITS(var,x) (var|=x)
#define UNSETBITS(var, x) (var&=~((unsigned int)x))
#endif



#ifndef RAPT_LIBRARY
#include "globals.h"
#endif

#ifdef _DEBUG
#define _RECORDER
#endif


