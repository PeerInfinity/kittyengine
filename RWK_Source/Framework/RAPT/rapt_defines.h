//#ifndef RAPT_DEFINES

#define RAPT_DEFINES

//
// An attempt to give more control over delete, so that
// we can eventually intercept deletes on garbage collection systems...
//

//#ifndef _Delete
#ifndef _DestroyCPU
//#define _Delete(theObject) {delete theObject;theObject=NULL;}
//#define _Destroy(theObject) {delete theObject;theObject=NULL;}
#define _DestroyCPU(theObject) {delete theObject;theObject=NULL;}
#define _DeleteArray(theObject) {delete [] theObject;theObject=NULL;}
#define _GarbageCollect() /*GC.Collect();*/
#define _Kill(theObject) {if (theObject) {theObject->Kill();theObject=NULL;}}
#endif

//
// Lets you cast memory as an int...why do this?  Because:
// int aID=CASTINT("MYID");
// if (aID==CASTINT("MYID")) ...
//
#ifndef CASTINT
#define CASTINT(aMem) (*((int*)aMem))
#define CAST(asWhat,aMem) ((asWhat)aMem)
#define CASTLONGLONG(aMem) CAST(longlong*,aMem)
#define RTEMPLATE template <typename vtype>
#endif


#define gG gAppPtr->mGraphics
#define gAudio gAppPtr->mAudio
#define gInput gAppPtr->mInput
#define gOut gConsole
#define gFactory gAppPtr->Factory
#define gTilesetSize		47

#ifdef _DEBUG
#define IsDebugMode() (true)
#define breakbecause(x) {gOut.Out(x);break;}
#else
#define IsDebugMode() (false)
#define breakbecause(x) break;
#endif

//
// Foreach ... works on any class with begin() and end() defined that can be iterated through by incrementing a pointer.  See array for help.
//
#define EXPAND( x ) x
#define foreach0() error!
#define foreach1(arrayname) for (auto& i:arrayname)
#define foreach2(varname,arrayname) for (auto& varname : arrayname)
#define GET_FOREACH(_1,_2,NAME,...) NAME
#define foreach(...) EXPAND(GET_FOREACH(__VA_ARGS__, foreach2, foreach1)(__VA_ARGS__))

#define SETBITS(var,x) (var|=x)
#define UNSETBITS(var, x) (var&=~((unsigned int)x))


#ifndef GL30
typedef short vbindex; // Compatibility mode (Chuzzle/Hoggy)
#else
typedef unsigned short vbindex;
#endif


#define CURSOR_NORMAL			0
#define CURSOR_IBEAM			1
#define CURSOR_FINGER			2
#define CURSOR_BLANK			3
#define CURSOR_HAND				4
#define CURSOR_FINGER2			5
#define CURSOR_DRAG				6
#define CURSOR_DRAGV			7
#define CURSOR_DRAGH			8


#define TOUCH_LEFT				0x01
#define TOUCH_RIGHT				0x02
#define TOUCH_MIDDLE			0x04

#ifndef _max
#define _max(a,b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef _min
#define _min(a,b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef _abs
#define _abs(x) ((x) > 0) ? (x) : -(x)
#endif

#ifndef _clamp
#define _clamp(minimum,v,maximum) _max((minimum),_min((maximum),(v)))
#define _clamp01(v) _max((0),_min((1),(v)))
#endif

#ifndef swap
#define _swap(a,b) {(a)=(a)+(b); (b)=(a)-(b); (a)=(a)-(b);}
#endif

#ifndef _sign
#define _sign(x) ((x)>0)-((x)<0)
#endif


#ifndef longlong 
#define longlong long long
#endif

#ifndef INTERPOLATE
#define INTERPOLATE(val,min,max) (((max-min)*val)+min)
#endif

#ifndef FLOATMAX
#define FLOATMAX     3.40282347E+37F
#endif
#ifndef MAXFLOAT
#define MAXFLOAT     3.40282347E+37F
#endif


//#endif

