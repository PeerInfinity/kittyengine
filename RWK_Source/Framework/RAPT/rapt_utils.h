#pragma once

#include "rapt_list.h"
#include "rapt_string.h"
#include "rapt_smartpointer.h"
#include "rapt_math.h"



class Object;
class Image;
class Utils
{
public:
	Utils(void);

	///////////////////////////////////////////////////////////////////////////////////////////
	//
	// Here's some utilities for handling lists of points
	//
	//
	static void AddUniquePointToList(List &theList, Point thePoint);
	static void AddUniquePointToArray(Array<Point>& theList, Point thePoint, float theThreshold=gMath.mMachineEpsilon);
	static void AddUniqueIPointToArray(Array<IPoint>& theList, IPoint thePoint);
	static bool IsIPointInArray(Array<IPoint>& theList, IPoint thePoint);
	static bool IsPointInArray(Array<Point>& theList, Point thePoint);
	static void RemoveIPointFromArray(Array<IPoint>& theList, IPoint thePoint);
	static void RemovePointFromArray(Array<Point>& theList, Point thePoint);
	static void SortIPointArrayByX(Array<IPoint>& theArray);
	static void SortIPointArrayByY(Array<IPoint>& theArray);
	static void SortPointListByClosest(List &theList, Point theFirstPoint);
	static void SortPointListByClosest(List &theList, void *theObject,Point *thePointPosition, Point theFirstPoint);
	template <class var_type> static void ShuffleArray(Array<var_type>& theArray,Random& theRandom) {for (int aCount=0;aCount<theArray.Size();aCount++) {int aOther=theRandom.Get(theArray.Size());var_type aHold=theArray.Element(aCount);theArray.Element(aCount)=theArray.Element(aOther);theArray.Element(aOther)=aHold;}}

	//
	// This allows you to sort a list of items where the "Point" is an offset.
	// If the object is a list of points, you can just leave off the start/end positions,
	// and it'll behave correctly.
	//
	static void	SortPointListByY(List &theList, void *theObject=NULL, Point *thePointPosition=NULL);
	static void	SortPointListByX(List &theList, void *theObject=NULL, Point *thePointPosition=NULL);
	static void	SortListByZ(List &theList, void *theObject=NULL, float *theZPosition=NULL);
	static void	SortListByZ(List &theList, void *theObject=NULL, int *theZPosition=NULL);
	static void	SortListOfObjects(List &theList);
	//
	static void QuicksortObjects(List& theList, int theStart=0, int theEnd=-1);	// List of objects
	static void QuicksortObjectsReverse(List& theList, int theStart=0, int theEnd=-1);	// List of objects
	static void QuicksortObjects(SmartList(Object)* theList, int theStart=0, int theEnd=-1);
	static void QuicksortObjectsReverse(SmartList(Object)* theList, int theStart=0, int theEnd=-1);
	//
	static void Alphabetize(Array<String> &theList,int theStart=0,int theEnd=-1);
	static void Alphabetize(Array<char*> &theList,int theStart=0,int theEnd=-1);
	//
	///////////////////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////////////////
	//
	// These are some utilities to handle bit operations (like finding out how many
	// shifts a bit is over)
	//
	static unsigned int GetBitShift(unsigned int theBits);	// Tells you how many shifts brings the bits down to the left
	static unsigned int GetBitLength(unsigned int theBits, int theMax=8);	// Tells you how long the span of bits is, in theBits
	//
	///////////////////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////////////////
	//
	// Some vertex building utilities...
	//
	static void	QuadsToVerts(Quad& theDrawQuad, Quad& theUVQuad, Array<Vertex2D>& theVerts);
	static void VertsToIndexedVerts(int theVCount,Vertex2D* theVertices,Array<Vertex2D>& theResultVerts, Array<vbindex>& theResultIndices);
	static void VertsToIndexedVerts(int theVCount,Vertex2DMT* theVertices,Array<Vertex2DMT>& theResultVerts, Array<vbindex>& theResultIndices);
	static void VertsToIndexedVerts(int theVCount,Vertex2DN* theVertices,Array<Vertex2DN>& theResultVerts, Array<vbindex>& theResultIndices);
	static void VertsToIndexedVertsInVolume(Cube theBound, int theVCount,Vertex2DMT* theVertices,Array<Vertex2DMT>& theResultVerts, Array<vbindex>& theResultIndices);

	//
	///////////////////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////////////////
	//
	// Turns a string to an int, by order.  Used to backwards process text-to-enum
	// or something.  You can put values in brackets to make it accept that value, for
	// instance OPTION[15]
	//
	static int PickStringToInt(String myString, String theList, int theDefault=-1);
	// 
	// Turns a string or span of numbers into an array of ints.  For instance, turns this:
	// 1,3-5,7,9-12 into 1,3,4,5,7,9,10,11,12
	//
	static void NumberStringToIntArray(String myString, Array<int>& theArray);
	static void NumberStringToShortArray(String myString, Array<short>& theArray);
	static int NumberStringToRandomInt(String myString);
	///////////////////////////////////////////////////////////////////////////////////////////

	//
	// Turns an image into code, for embedding (good for things like masks, etc)
	// Automatically ignores transparency.
	//
	static void ImageToMask(Image& theImage, String theOutFile);

	//
	// Turns a comma-seperated string to an array
	//
	static void CSVToArray(String theString, Array<String>& theArray, char theSeperator=',');
	static void CSVToArray(String theString, Array<int>& theArray, char theSeperator=',');
	static void CSVToArray(String theString, Array<char>& theArray, char theSeperator=',');
	static void CSVToArray(String theString, Array<bool>& theArray, char theSeperator=',');
	static String ArrayToCSV(Array<String>& theArray, char theSeperator=',');
	static String ArrayToCSV(Array<int>& theArray, char theSeperator=',');
	static String ArrayToCSV(Array<short>& theArray, char theSeperator=',');
	static String ArrayToCSV(Array<char>& theArray, char theSeperator=',');
	static String ArrayToCSV(Array<bool>& theArray, char theSeperator=',');
	static void CSVToSmartList(String theString, SmartList(String)& theArray, char theSeperator=',');
	static String SmartListToCSV(SmartList(String)& theArray, char theSeperator=',');

	//
	// This is an easy helper to do a regular glint.  I used it for the light specular attention-drawing
	// flash on hints/popups in Chuzzle 2.  Basically you can say:
	//
	// DrawGraphicNormally();
	// float aFlicker=GetFlicker(500,50,AppTime());
	// if (aFlicker) DrawGraphicWhite(transparency=aFlicker);
	// 
	// And it'll cause regular, nicely scaled sin pulses of glint.
	//
	static float GetFlicker(int theTimeBetween, int theFlickerLength, int theTick=-1);

	//
	// Difference between dates
	//
	static int DateToInt(int theMonth, int theDay, int theYear);

	//
	// Simple has function
	//
	static longlong Hash(void* theData, int theLen);

	//
	// Snap a floating value to a certain threshold
	//
	static inline void Snap(float& theValue, float snapTo)
	{
		theValue/=snapTo;
		if (theValue>0) theValue=floorf(theValue+.5f);else theValue=ceilf(theValue-.5f);
		theValue*=snapTo;
	}
	static inline void Snap(Point& theValue, float snapTo)
	{
		Snap(theValue.mX,snapTo);
		Snap(theValue.mY,snapTo);
	}
	static inline void Snap(Vector& theValue, float snapTo)
	{
		Snap(theValue.mX,snapTo);
		Snap(theValue.mY,snapTo);
		Snap(theValue.mZ,snapTo);
	}

	static inline void Round(float& theValue, float roundTo, char upDown=1)
	{
		theValue/=roundTo;
		if (theValue>0) {if (upDown==1) theValue=ceilf(theValue);else theValue=floorf(theValue);}
		else  {if (upDown==1) theValue=floorf(theValue);else theValue=ceilf(theValue);}
		theValue*=roundTo;
	}

	static inline void Round(Point& theValue, float roundTo, char upDown=1)
	{
		Round(theValue.mX,roundTo,upDown);
		Round(theValue.mY,roundTo,upDown);
	}
	static inline void Snap(Vector& theValue, float roundTo, char upDown=1)
	{
		Round(theValue.mX,roundTo,upDown);
		Round(theValue.mY,roundTo,upDown);
		Round(theValue.mZ,roundTo,upDown);
	}


	static void GetLineIntersections(Array<Line> &theLines, Array<Point>& theResults);
	static void GetLineIntersections(Array<Line3D> &theLines, Array<Vector>& theResults);
	static void GetLineIntersections(Line theKeyLine, Array<Line> &theLines, Array<Point>& theResults);
};

class SmartVariable
{
public:
	int					mInt;
	bool				mBool;
	float				mFloat;
	String				mString;

	void				FromInt(int theValue);
	void				FromBool(bool theValue);
	void				FromFloat(float theValue);
	void				FromString(String theValue);

public:
	SmartVariable(const int& theValue) {FromInt(theValue);}
	SmartVariable(const bool& theValue) {FromBool(theValue);}
	SmartVariable(const float& theValue) {FromFloat(theValue);}
	SmartVariable(const String& theValue) {FromString(theValue);}

	SmartVariable& operator=(int& theValue)  {FromInt(theValue);return *this;}
	SmartVariable& operator=(bool& theValue)  {FromBool(theValue);return *this;}
	SmartVariable& operator=(float& theValue) {FromFloat(theValue);return *this;}
	SmartVariable& operator=(String& theValue) {FromString(theValue);return *this;}

	SmartVariable& operator=(int theValue)  {FromInt(theValue);return *this;}
	SmartVariable& operator=(bool theValue)  {FromBool(theValue);return *this;}
	SmartVariable& operator=(float theValue) {FromFloat(theValue);return *this;}
	SmartVariable& operator=(String theValue) {FromString(theValue);return *this;}

	operator int() {return mInt;}
	operator bool() {return mBool;}
	operator float() {return mFloat;}
	operator String() {return mString;}
    operator char*() {return mString.c();}
};

/*
template <class var_type>
class WatchVariable
{
public:
	WatchVariable() {mChanged=false;}
	WatchVariable(var_type& theVar) {mVariable=theVar;mChanged=false;}

	var_type		mVariable;
	bool			mChanged;
	inline bool		IsChanged() {bool aResult=mChanged;mChanged=false;return aResult;}

	inline operator var_type&() {return mVariable;}
	inline operator var_type*() {return &mVariable;}
	inline var_type& operator=(var_type& theVar) {mVariable=theVar;mChanged=true;return *this;}
	inline var_type& operator=(var_type theVar) {mVariable=theVar;mChanged=true;return *this;}
};

typedef WatchVariable<int> WatchInt;
typedef WatchVariable<float> WatchFloat;
typedef WatchVariable<Point> WatchPoint;
typedef WatchVariable<float> WatchFloat;
typedef WatchVariable<bool> WatchBool;
*/

class GlobalID
{
public:
	GlobalID() {Null();}

	union
	{
		struct 
		{
			unsigned int mData1;
			unsigned short  mData2;
			unsigned short  mData3;
			unsigned char  mData4[8];
		} mGuid;

		unsigned int		mData[4];
		unsigned char		mDataBytes[16];
	};

	void Null() {mData[0]=mData[1]=mData[2]=mData[3]=0;}
	void Invalidate() {mData[0]=mData[1]=mData[2]=mData[3]=0xFFFFFFFF;}
	bool IsNull() {return (mData[0]==0 && mData[1]==0 && mData[2]==0 && mData[3]==0);}
	bool IsInvalid() {return (mData[0]==0xFFFFFFFF && mData[1]==0xFFFFFFFF && mData[2]==0xFFFFFFFF && mData[3]==0xFFFFFFFF);}
	inline bool	operator==(const GlobalID &theGUID) {return ((mData[0]==theGUID.mData[0])&(mData[1]==theGUID.mData[1])&(mData[2]==theGUID.mData[2])&(mData[3]==theGUID.mData[3]));}
	inline bool	operator!=(const GlobalID &theGUID) {return ((mData[0]!=theGUID.mData[0])|(mData[1]!=theGUID.mData[1])|(mData[2]!=theGUID.mData[2])|(mData[3]!=theGUID.mData[3]));}
	inline GlobalID &operator=(const GlobalID &theGUID) {if (this!=&theGUID) {mData[0]=theGUID.mData[0];mData[1]=theGUID.mData[1];mData[2]=theGUID.mData[2];mData[3]=theGUID.mData[3];}return *this;}
	GlobalID &operator=(String &theGUIDString);
	inline int ToInt() {return mData[0]+mData[1]+mData[2]+mData[3];}
	String ToString();
	String ToHex();
	void FromString(String theString);
};

GlobalID GetGlobalID(Random& theRand=gRand);
GlobalID GetNullGlobalID();
GlobalID GetInvalidGlobalID();

class PointerVolume
{
public:
	void				Go(Cube theVolume, float theSubvolumeSize, bool useUnique=false);
	void				Add(Vector thePos, void* thePointer);
	List&				GetList(Vector thePos);

	inline int			PosToGrid(int theX, int theY, int theZ) {return theX+(theY*mXSize)+(theZ*mXSize*mYSize);}
	inline int			PosToGrid(float theX, float theY, float theZ) {return PosToGrid((int)theX,(int)theY,(int)theZ);}
	inline int			PosToGrid(Vector theVector) {return PosToGrid((int)theVector.mX,(int)theVector.mY,(int)theVector.mZ);}

public:
	Cube				mVolume;
	float				mSubVolumeSize;
	bool				mUnique;

	int					mXSize;
	int					mYSize;
	int					mZSize;

	Array<List>			mListGrid;
	List				mNullList;
};

class Stopwatch
{
public:
	void				Start();
	int					Stop();
	int					mTick;
};

//
// For blinking eyes...
//
class Blinker
{
public:
	Blinker() {mMinBlinkWait=200;mMaxBlinkWait=300;mBlinkWait=gRand.Get(mMaxBlinkWait);mBlink=0;mDoubleBlink=0;mBlinkLength=10;}
	inline void				BlinkSoon(int howSoon) {mBlinkWait=howSoon;}
	void					Blink()
	{
		char aDoubleBlink=mDoubleBlink;
		mDoubleBlink=_max(0,mDoubleBlink-1);
		mBlink=mBlinkLength;
		mBlinkWait=gRand.GetSpan(mMinBlinkWait,mMaxBlinkWait);
		if (!aDoubleBlink) if (gRand.Get(10)==1) {mBlinkWait=20;mDoubleBlink=3;}
	}
	void					NoBlink()
	{
		mBlink=0;
		mBlinkWait=gRand.GetSpan(mMinBlinkWait,mMaxBlinkWait);
	}
	void					Update()
	{
		mBlink=_max(0,mBlink-1);
		if (--mBlinkWait<0) Blink();
	}
	inline bool				IsBlinking() {return mBlink>0;}

public:
	int						mBlinkWait;
	int						mBlink;
	int						mBlinkLength;	// How long we keep eyes closed
	char					mDoubleBlink;

	int						mMinBlinkWait;
	int						mMaxBlinkWait;
	inline void				SetBlinkSpeed(int minBlinkWait, int maxBlinkWait) {mMinBlinkWait=minBlinkWait;mMaxBlinkWait=maxBlinkWait;mBlinkWait=gRand.Get(mMaxBlinkWait);}
};


class ColorCycler
{
public:
	ColorCycler(void);
	virtual ~ColorCycler(void);

	void			AddColor(Color theColor, float theSpan=1.0f);
	void			Generate();
	inline void		Go() {Generate();}

	Color			Get(float theValue);
	inline int		GetCount() {return mList.GetCount()-1;}
	inline int		Count() {return GetCount();}

public:

	struct SpanStruct
	{
		float		mSpan;
		Color		mColor;
	};
	SmartList(SpanStruct)	mList;

	float			mSpanCount;
	inline float	GetSpanCount() {return mSpanCount;}
	bool			mDidGenerate;

};

//#include <memory>
//#include <typeinfo>

class Any;

template<class Type> Type AnyCast(Any&);
template<class Type> Type AnyCast(const Any&);
template<class Type> Type* AnyCast(Any*);
template<class Type> const Type* AnyCast(const Any*);

//struct BadAnyCast : public std::bad_cast {};

class Any
{
public:

	template<class Type> friend Type AnyCast(Any&);
	template<class Type> friend Type AnyCast(const Any&);
	template<class Type> friend Type* AnyCast(Any*);
	template<class Type> friend const Type* AnyCast(const Any*);

	Any() : mPointer(NULL) {}
	Any(Any&& x) : mPointer(x.mPointer) {}
	Any(const Any& x) {if (x.mPointer) mPointer=x.mPointer->Clone();}
	Any& operator=(Any&& rhs) {mPointer=(rhs.mPointer);return (*this);}
	Any& operator=(const Any& rhs) {mPointer=(Any(rhs).mPointer);return (*this);}

	/*
	template<class Type> Any(const Type& x) : mPointer(new Concrete<typename std::decay<const Type>::type>(x)) {}
	template<class T> Any& operator=(T&& x) {mPointer=(new Concrete<typename std::decay<T>::type>(typename std::decay<T>::type(x)));return (*this);}  
	template<class T> Any& operator=(const T& x) {mPointer=(new Concrete<typename std::decay<T>::type>(typename std::decay<T>::type(x)));return (*this);}
	/**/

	//*
	template<class Type> Any(const Type& x) : mPointer(new Concrete<Type>(x)) {}
	template<class T> Any& operator=(T&& x) {mPointer=(new Concrete<T>(x));return (*this);}  
	template<class T> Any& operator=(const T& x) {mPointer=(new Concrete<typename T::type>(x));return (*this);}
	/**/

	void Reset() {mPointer=NULL;}
	inline bool IsNull() {return mPointer.IsNull();}
	inline bool IsNotNull() {return mPointer.IsNotNull();}
	//inline bool OK() {return mPointer.IsNotNull();} // Won't compile on Linux???
	inline bool Ok() {return mPointer.IsNotNull();}
	inline bool Okay() {return mPointer.IsNotNull();}
	inline bool IsOkay() {return mPointer.IsNotNull();}

	inline bool				operator==(void* theValue) {return mPointer.GetPointer()==theValue;}
	inline bool				operator!=(void* theValue) {return mPointer.GetPointer()!=theValue;}
	inline bool				operator!() const {return mPointer.IsNull();}
	//inline 					operator bool() const {return mPointer.IsNotNull();}

	//const std::type_info& Type() const {return (!IsEmpty()) ? mPointer->Type() : typeid(void);}
	
private:

	struct Placeholder 
	{
		virtual Smart(Placeholder) Clone() const=0;
		//virtual const std::type_info& Type() const = 0;
		virtual ~Placeholder() {}
	};

	template<class T> struct Concrete : public Placeholder 
	{
		Concrete(T&& x) : mValue(x) {}
		Concrete(const T& x) : mValue(x) {}

		Smart(Placeholder) Clone() const override {return new Concrete<T>(mValue);}
		//virtual const std::type_info& Type() const override {return typeid(T);}

		T mValue;

	};

	Smart(Placeholder) mPointer;

public:
	template<class T>operator T() {return AnyCast<T>(*this);}
};

template<class Type> Type AnyCast(Any& val) {/*if (val.mPointer->Type() != typeid(Type)) throw BadAnyCast();*/return static_cast<Any::Concrete<Type>*>(val.mPointer.GetPointer())->mValue;}
template<class Type> Type AnyCast(const Any& val) {return AnyCast<Type>(Any(val));}
template<class Type> Type* AnyCast(Any* ptr) {return dynamic_cast<Type*>(ptr->mPointer.GetPointer());}
template<class Type> const Type* AnyCast(const Any* ptr) {return dynamic_cast<const Type*>(ptr->mPointer.GetPointer());}

extern Any NullAny;

//
// Holds a lot of words, and you can check if a word exists by querying.
// It's faster because it breaks everything up into alphabetical arrays.
// A word must be at least two characters long, though!
//
class Dictionary
{
public:
	Dictionary() {mLoaded=false;}
	void				Load(String theFilename);
	bool				Exists(String theLowercaseWord);
	int					Exists(char* thePtr, int theLen=-1);
	inline bool			IsLoaded() {return mLoaded;}

public:
	SmartList(String)	mWords;

	Array<Array<IPoint> >		mLookup;
	int							mLookupBase[2];	
	bool						mLoaded;
	// Where our alphabetics start/end
};

//
// Helpful function for "glinting" things...
// Delay is how long between glints
// Duration is how long the glint stays lit up
// Tick is just whatever timer
// Returns the color (0, if not time to glint)
// Example Usage: if (float aGlint=GetGlint(AppTime(),500,35)) {gG.SetColor(aGlint);DrawThing();gG.SetColor();}
//
float GetGlint(int theTick,int theDelay=500, int theDuration=35);

//
// A little timer helper, for debugging...
//
extern int gDebugTimerTick;
extern String gDebugTimerString;
inline void StartTimer(String theString) {gDebugTimerString=theString;gDebugTimerTick=OS_Core::Tick();}
inline void EndTimer(...) {int aTime=OS_Core::Tick()-gDebugTimerTick;OS_Core::Printf("Timer [%s] = %d",gDebugTimerString.c(),aTime);}

template <typename var_type>
class WatchValue
{
public:
	WatchValue() {}
	WatchValue(var_type& theValue) {mValue=theValue;}
	WatchValue(var_type theValue) {mValue=theValue;}

	var_type	mValue;

	inline WatchValue &operator=(var_type theValue) {mValue=theValue;return *this;}

	inline bool	IsEqual(var_type theOther) {bool aResult=(mValue==theOther);mValue=theOther;return aResult;}
	inline bool	IsChanged(var_type theOther) {return !IsEqual(theOther);}
};

typedef WatchValue<int> WatchInt;
typedef WatchValue<char> WatchChar;
typedef WatchValue<short> WatchShort;

//
// Cloud helpers...
//
namespace CloudQueue
{
	void Put(String& theName,IOBuffer* theBuffer);
	void Dispatch(bool doThreaded);	// Sends the cloud queue up to the cloud...
	void Kill(); // Kills the queue without sending it (when you do a total copy to the cloud, it gets rid of any pending things with this)

	void Disable(); // Disable all clouding
	void Enable(); // Enable all clouding
}

//
// Whatever you vtype it with has to have .PrettyClose defined
//
template <typename vtype>
class UniqueCoordinateList
{
public:
	Array<vtype> mList;
	int mSize=0;

	inline void PreAlloc(int theSize) {mList.GuaranteeSize(theSize);}
	inline void GuaranteeSize(int theSize) {mList.GuaranteeSize(theSize);}
	inline void Add(vtype theValue)
	{
		int aCount;
		for (aCount=0;aCount<mSize;aCount++) if (mList[aCount].PrettyClose(theValue)) return;
		mList[aCount]=theValue;
		mSize++;
	}
	void operator+=	(vtype theObject) {Add(theObject);}

	inline vtype Average()
	{
		vtype aResult;
		for (int aCount=0;aCount<mSize;aCount++) aResult+=mList[aCount];
		aResult/=(float)mSize;
		return aResult;
	}
	inline vtype Normal()
	{
		vtype aResult;
		for (int aCount=0;aCount<mSize;aCount++) aResult+=mList[aCount];
		return aResult.GetNormal();
	}
};
typedef UniqueCoordinateList<Vector> UVectorList;
typedef UniqueCoordinateList<Point> UPointList;


