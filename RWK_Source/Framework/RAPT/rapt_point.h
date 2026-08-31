#pragma once
#include <math.h>
#include <stdlib.h>

#ifndef _max
#define _max(a,b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef _min
#define _min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#define Point RaptPoint

struct Vertex2D;
struct Vertex2DMT;
class Vector;
class IPoint;
bool PrettyClose(float theF1, float theF2);


class IOBuffer;
class SyncBuffer;

class Point
{
public:
	Point() {mX=mY=0.0f;}
	//Point(const float theValue) {mX=theValue;mY=theValue;}
	//Point (const int theValue) {mX=(float)theValue;mY=(float)theValue;}
	Point(const float theX, const float theY) {mX=theX;mY=theY;}
	Point(const int theX, const int theY) {mX=(float)theX;mY=(float)theY;}
	Point(const Point &thePoint) {*this=thePoint;}
	Point(const IPoint& thePoint);
	Point(const Vector& theVector);
	Point(const float theVal) {mX=theVal;mY=theVal;}

	inline Point &operator=(const Point &thePoint) {if (this!=&thePoint) {mX=thePoint.mX;mY=thePoint.mY;}return *this;}
	inline Point &operator=(float &theValue) {mX=theValue;mY=theValue;return *this;}
	inline Point &operator=(float theValue) {mX=theValue;mY=theValue;return *this;}
	inline Point &operator=(int &theValue) {mX=(float)theValue;mY=(float)theValue;return *this;}
	inline Point &operator=(int theValue) {mX=(float)theValue;mY=(float)theValue;return *this;}
	//inline Point &operator=(float* theValue) {mX=(float)theValue[0];mY=(float)theValue[1];return *this;}
	//inline Point &operator=(int* theValue) {mX=(float)theValue[0];mY=(float)theValue[1];return *this;}
	Point &operator=(Vector& theVector);

	inline bool IsNAN() {return (mX!=mX || mY!=mY);}
	inline void SetNAN() {mX=(float)nan("");}

	inline void Reset() {mX=mY=0;}
	void Assign(Point thePos) {*this=thePos;}
	inline Point ToVector() {return *this;} // Compatibility layer (some functions expect to call ToVector to get the position component of things)

	char* ToString(int thePrecision=2);

#ifdef _DEBUG
	char* ToCString();
#else
#endif


	inline bool	operator==(const Point &thePoint) {return (mX==thePoint.mX)&(mY==thePoint.mY);}
	inline bool	operator!=(const Point &thePoint) {return (mX!=thePoint.mX)|(mY!=thePoint.mY);}

	inline Point Lerp(Point theOther, float theAmount) {return *this+((theOther-*this)*theAmount);}
	inline Point LerpNormal(Point theOther, float theAmount) {return Point(*this+((theOther-*this)*theAmount)).Normal();}
	inline bool PrettyClose(Point thePoint) {return (::PrettyClose(mX,thePoint.mX) && ::PrettyClose(mY,thePoint.mY));}
	
	inline const Point	operator*(const Point &thePoint) const {return Point(mX*thePoint.mX,mY*thePoint.mY);}
	inline Point &operator*=(const Point &thePoint) {mX*=thePoint.mX;mY*=thePoint.mY;return *this;}
	inline const Point operator*(const float theFloat) const {return Point(mX*theFloat,mY*theFloat);}
	inline Point &operator*=(const float theFloat) {mX*=theFloat;mY*=theFloat;return *this;}
	inline const Point	operator/(const Point &thePoint) const {return Point(mX/thePoint.mX,mY/thePoint.mY);}
	inline Point &operator/=(const Point &thePoint) {mX/=thePoint.mX;mY/=thePoint.mY;return *this;}
	inline const Point operator/(const float theFloat) const {return Point(mX/theFloat,mY/theFloat);}
	inline Point &operator/=(const float theFloat) {mX/=theFloat;mY/=theFloat;return *this;}
	inline const Point operator+(const Point &thePoint) const {return Point(mX+thePoint.mX,mY+thePoint.mY);}
	inline Point &operator+=(const Point &thePoint) {mX+=thePoint.mX;mY+=thePoint.mY;return *this;}
	inline const Point operator+(const float theFloat) const {return Point(mX+theFloat,mY+theFloat);}
	inline Point &operator+=(const float theFloat) {mX+=theFloat;mY+=theFloat;return *this;}
	inline const Point operator-(const Point &thePoint) const {return Point(mX-thePoint.mX,mY-thePoint.mY);}
	inline Point &operator-=(const Point &thePoint) {mX-=thePoint.mX;mY-=thePoint.mY;return *this;}
	inline const Point operator-(const float theFloat) const {return Point(mX-theFloat,mY-theFloat);}
	inline Point & operator-=(const float theFloat) {mX-=theFloat;mY-=theFloat;return *this;}
	inline const Point operator-() const {return Point(-mX,-mY);}

	inline float Dot(const Point &thePoint) const {return mX*thePoint.mX+mY*thePoint.mY;}
	inline float AbsDot(const Point &thePoint) const {return (float)fabs(mX*thePoint.mX+mY*thePoint.mY);}
	inline float DotAbs(const Point &thePoint) const {return (float)fabs(mX*thePoint.mX+mY*thePoint.mY);}
	inline float PerpDot(const Point& thePoint) const {return mY*thePoint.mX+mX*thePoint.mY;}
	inline const float Cross(const Point &thePoint) const {return mX*thePoint.mY-mY*thePoint.mX;}
	inline const float Determinant(const Point &thePoint) const {return mX*thePoint.mY-mY*thePoint.mX;} // Same as cross

	inline float LengthSquared() const {return Dot(*this);}
	inline float Length() const {float aLen=LengthSquared();if (aLen<=0) return 0;return static_cast<float>(sqrt(aLen));}
	float GetLengthSquared() {return Dot(*this);}
	float GetLength() {float aLen=LengthSquared();if (aLen<=0) return 0;return static_cast<float>(sqrt(aLen));}
	inline bool IsNull() const {return mX==0 && mY==0;}
	inline void SetLength(const float theLen) {float aScale=Length();if (aScale>0) aScale=theLen/aScale;*this*=aScale;}
	inline void SetDir(Point theDir) {float aLen=Length();*this=theDir;SetLength(aLen);}
	inline Point Normalize(){SetLength(1);return Point(mX,mY);}
	inline Point Swap() {float aHold=mX;mX=mY;mY=aHold;return Point(mX,mY);}

	inline Point Normal()
	{
		const float aLenSQ=mX*mX+mY*mY;
		if (aLenSQ==0) return Point(0,0);
		const float aInvLen=1.0f/(float)sqrt(aLenSQ);
		return Point(mX*aInvLen,mY*aInvLen);
	//	Point aP=*this;aP.SetLength(1);return aP;
	}
	inline Point Abs() {Point	aResult;aResult.mX=(mX<0)? -mX:mX;aResult.mY=(mY<0)? -mY:mY;return aResult;}
	inline Point Sign() {Point aResult(0,0);if (mX<0) aResult.mX=-1;else if (mX>0) aResult.mX=1;if (mY<0) aResult.mY=-1;else if (mY>0) aResult.mY=1;return aResult;}
	inline Point Mod(float theNumber) {return Point((float)fmod(mX,theNumber),(float)fmod(mY,theNumber));}
	inline Point Mod(Point theNumber) {return Point((float)fmod(mX,theNumber.mX),(float)fmod(mY,theNumber.mY));}
	Point RMod(float theNumber);
	Point RMod(Point theNumber);
	inline Point Floor() {return Point(floorf(mX),floorf(mY));}
	inline Point Ceil() {return Point(ceilf(mX),ceilf(mY));}
	inline void Null() {mX=mY=0;}
	inline void Nullify() {mX=mY=0;}
	inline void RotateCW() {float aHold=mX;mX=-mY;mY=aHold;}
	inline float Area() {return mX*mY;}
	inline float Diff() {return mY-mX;}

	inline Point MajorAxis()
	{
		Point aResult=*this;
		if (fabs(mX)>fabs(mY)) aResult.mY=0;
		else aResult.mX=0;
		return aResult;
	}
	inline Point MinorAxis()
	{
		Point aResult=*this;
		if (fabs(mX)<fabs(mY)) aResult.mY=0;
		else aResult.mX=0;
		return aResult;
	}


	inline float Biggest() {return (float)_max(fabs(mX),fabs(mY));}
	inline float Smallest() {return (float)_min(fabs(mX),fabs(mY));}
	inline float Max() {return (float)_max(fabs(mX),fabs(mY));}
	inline float Min() {return (float)_min(fabs(mX),fabs(mY));}

	inline Point& Maximize(Point theOther) {mX=_max(mX,theOther.mX);mY=_max(mY,theOther.mY);return *this;}
	inline Point& Minimize(Point theOther) {mX=_min(mX,theOther.mX);mY=_min(mY,theOther.mY);return *this;}
	inline Point& Maximize() {float aMax=Biggest();mX=mY=aMax;return *this;}
	inline Point& Minimize() {float aMin=Smallest();mX=mY=aMin;return *this;}

	void Write(IOBuffer& theBuffer);
	void Read(IOBuffer& theBuffer);
	void Sync(SyncBuffer& theBuffer);


	//
	// Faster, but approximate...
	// Removed because apple no longer permits Carmack's bit shifting Genius.
	// The only way to "Think Different" in Silicon Valley now is to STOP letting people do brilliant things.
	//
	//void SetLengthFast(const float theLen);

	//
	// Gives you the point's perpendicular
	//
	inline Point Perp() {return Point(mY,-mX);}
	inline Point Transpose() {return Point(mY,mX);}

	Point& Randomize(float theMagnitude=1, bool doNormalize=true);

	//
	// Lets you know whether the point is at origin
	//
	inline bool IsOrigin() {return ((mX==0)&&mY==0);}

	//
	// This removes the fractional component of a point (and rounds it up/down to the
	// nearest number.
	//
	Point	ToInt()
	{
		float aX,aY;
		aX=aY=0;
		if (mX>0) aX=(float)((int)(mX+.5f));
		if (mX<0) aX=(float)((int)(mX-.5f));
		if (mY>0) aY=(float)((int)(mY+.5f));
		if (mY<0) aY=(float)((int)(mY-.5f));

		return Point(aX,aY);
		//mX=aX;
		//mY=aY;
	}

	float					operator[](int theSlot) {return mData[theSlot];}
	inline float*			AsFloat() {return mData;}

	union
	{
		float mData[2];
		struct
		{
			union
			{
				float mX;
				float mWidth;
				float mW;
				float mData1;
				float mStart;
				float mLow;
				float mU;
			};
			union
			{
				float mY;
				float mHeight;
				float mH;
				float mData2;
				float mEnd;
				float mHigh;
				float mV;
			};
		};
	};
};

class IPoint
{
public:
	IPoint() {mX=mY=0;}
	IPoint(const float theValue) {mX=(int)theValue;mY=(int)theValue;}
	IPoint (const int theValue) {mX=theValue;mY=theValue;}
	IPoint(const float theX, const float theY) {mX=(int)theX;mY=(int)theY;}
	IPoint(const int theX, const int theY) {mX=theX;mY=theY;}
	IPoint(const IPoint &theIPoint) {*this=theIPoint;}
	IPoint(const Point &thePoint) {mX=(int)thePoint.mX;mY=(int)thePoint.mY;}
	inline IPoint &operator=(const IPoint &theIPoint) {if (this!=&theIPoint) {mX=theIPoint.mX;mY=theIPoint.mY;}return *this;}

	inline IPoint &operator=(float &theValue) {mX=(int)theValue;mY=(int)theValue;return *this;}
	inline IPoint &operator=(int &theValue) {mX=theValue;mY=theValue;return *this;}

	char* ToString(int theNothing=0);	// TheNothing is just there to make it same format as Point::ToString


	inline bool	operator==(const IPoint &theIPoint) {return mX==theIPoint.mX && mY==theIPoint.mY;}
	inline bool	operator!=(const IPoint &theIPoint) {return mX!=theIPoint.mX || mY!=theIPoint.mY;}

	inline void Reset() {mX=mY=0;}
	inline int Area() {return mX*mY;}

	inline const IPoint operator*(const float theFloat) const {return IPoint(mX*theFloat,mY*theFloat);}
	inline IPoint &operator*=(const float theFloat) {mX=(int)((float)mX*theFloat);mY=(int)((float)mY*theFloat);return *this;}
	inline const IPoint	operator*(const IPoint &theIPoint) const {return IPoint(mX*theIPoint.mX,mY*theIPoint.mY);}
	inline IPoint &operator*=(const IPoint &theIPoint) {mX*=theIPoint.mX;mY*=theIPoint.mY;return *this;}
	inline const IPoint operator*(const int theFloat) const {return IPoint(mX*theFloat,mY*theFloat);}
	inline IPoint &operator*=(const int theFloat) {mX*=theFloat;mY*=theFloat;return *this;}
	inline const IPoint	operator/(const IPoint &theIPoint) const {return IPoint(mX/theIPoint.mX,mY/theIPoint.mY);}
	inline IPoint &operator/=(const IPoint &theIPoint) {mX/=theIPoint.mX;mY/=theIPoint.mY;return *this;}
	inline const IPoint operator/(const int theFloat) const {return IPoint(mX/theFloat,mY/theFloat);}
	inline IPoint &operator/=(const int theFloat) {mX/=theFloat;mY/=theFloat;return *this;}
	inline const IPoint operator+(const IPoint &theIPoint) const {return IPoint(mX+theIPoint.mX,mY+theIPoint.mY);}
	inline IPoint &operator+=(const IPoint &theIPoint) {mX+=theIPoint.mX;mY+=theIPoint.mY;return *this;}
	inline const IPoint operator+(const int theFloat) const {return IPoint(mX+theFloat,mY+theFloat);}
	inline IPoint &operator+=(const int theFloat) {mX+=theFloat;mY+=theFloat;return *this;}
	inline const IPoint operator-(const IPoint &theIPoint) const {return IPoint(mX-theIPoint.mX,mY-theIPoint.mY);}
	inline IPoint &operator-=(const IPoint &theIPoint) {mX-=theIPoint.mX;mY-=theIPoint.mY;return *this;}
	inline const IPoint operator-(const int theFloat) const {return IPoint(mX-theFloat,mY-theFloat);}
	inline IPoint & operator-=(const int theFloat) {mX-=theFloat;mY-=theFloat;return *this;}
	inline const IPoint operator-() const {return IPoint(-mX,-mY);}

	inline bool IsNull() const {return mX==0 && mY==0;}
	inline IPoint Abs() {IPoint	aResult;aResult.mX=(mX<0)? -mX:mX;aResult.mY=(mY<0)? -mY:mY;return aResult;}
	inline IPoint Sign() {IPoint aResult(0,0);if (mX<0) aResult.mX=-1;else if (mX>0) aResult.mX=1;if (mY<0) aResult.mY=-1;else if (mY>0) aResult.mY=1;return aResult;}
	inline void Null() {mX=mY=0;}

	inline bool IsOrigin() {return ((mX==0)&&mY==0);}

	inline int Biggest() {return _max(mX,mY);}
	inline int Smallest() {return _min(mX,mY);}
	inline IPoint& Maximize() {int aMax=Biggest();mX=mY=aMax;return *this;}
	inline IPoint& Minimize() {int aMin=Smallest();mX=mY=aMin;return *this;}
	inline IPoint Swap() {int aHold=mX;mX=mY;mY=aHold;return IPoint(mX,mY);}



	union
	{
		int mX;
		int mWidth;
		int mW;
		int mData1;
		int mStart;
		int mLow;
	};
	union
	{
		int mY;
		int mHeight;
		int mH;
		int mData2;
		int mEnd;
		int mHigh;
	};


};

template <class var_type>
class DataPair
{
public:
	DataPair() {/*mData1=mData2=NULL;*/}
	//DataPair(const float theValue) {mData1=(var_type)theValue;mData2=(var_type)theValue;}
	DataPair (const var_type theValue) {mData1=theValue;mData2=theValue;}
	//DataPair(const float theX, const float theY) {mData1=(var_type)theX;mData2=(var_type)theY;}
	DataPair(const var_type theX, const var_type theY) {mData1=theX;mData2=theY;}
	DataPair(const DataPair &theDataPair) {*this=theDataPair;}
	DataPair(const Point &thePoint) {mData1=(var_type)thePoint.mX;mData2=(var_type)thePoint.mY;}
	inline DataPair &operator=(const DataPair &theDataPair) {if (this!=&theDataPair) {mData1=theDataPair.mData1;mData2=theDataPair.mData2;}return *this;}

	//inline DataPair &operator=(float &theValue) {mData1=(var_type)theValue;mData2=(var_type)theValue;return *this;}
	inline DataPair &operator=(var_type &theValue) {mData1=theValue;mData2=theValue;return *this;}

	inline bool	operator==(const DataPair &theDataPair) {return mData1==theDataPair.mData1 && mData2==theDataPair.mData2;}
	inline bool	operator!=(const DataPair &theDataPair) {return mData1!=theDataPair.mData1 || mData2!=theDataPair.mData2;}

	inline const DataPair	operator*(const DataPair &theDataPair) const {return DataPair(mData1*theDataPair.mData1,mData2*theDataPair.mData2);}
	inline DataPair &operator*=(const DataPair &theDataPair) {mData1*=theDataPair.mData1;mData2*=theDataPair.mData2;return *this;}
	inline const DataPair operator*(const var_type theFloat) const {return DataPair(mData1*theFloat,mData2*theFloat);}
	inline DataPair &operator*=(const var_type theFloat) {mData1*=theFloat;mData2*=theFloat;return *this;}
	inline const DataPair	operator/(const DataPair &theDataPair) const {return DataPair(mData1/theDataPair.mData1,mData2/theDataPair.mData2);}
	inline DataPair &operator/=(const DataPair &theDataPair) {mData1/=theDataPair.mData1;mData2/=theDataPair.mData2;return *this;}
	inline const DataPair operator/(const var_type theFloat) const {return DataPair(mData1/theFloat,mData2/theFloat);}
	inline DataPair &operator/=(const var_type theFloat) {mData1/=theFloat;mData2/=theFloat;return *this;}
	inline const DataPair operator+(const DataPair &theDataPair) const {return DataPair(mData1+theDataPair.mData1,mData2+theDataPair.mData2);}
	inline DataPair &operator+=(const DataPair &theDataPair) {mData1+=theDataPair.mData1;mData2+=theDataPair.mData2;return *this;}
	inline const DataPair operator+(const var_type theFloat) const {return DataPair(mData1+theFloat,mData2+theFloat);}
	inline DataPair &operator+=(const var_type theFloat) {mData1+=theFloat;mData2+=theFloat;return *this;}
	inline const DataPair operator-(const DataPair &theDataPair) const {return DataPair(mData1-theDataPair.mData1,mData2-theDataPair.mData2);}
	inline DataPair &operator-=(const DataPair &theDataPair) {mData1-=theDataPair.mData1;mData2-=theDataPair.mData2;return *this;}
	inline const DataPair operator-(const var_type theFloat) const {return DataPair(mData1-theFloat,mData2-theFloat);}
	inline DataPair & operator-=(const var_type theFloat) {mData1-=theFloat;mData2-=theFloat;return *this;}
	inline const DataPair operator-() const {return DataPair(-mData1,-mData2);}
	var_type& operator[](int theSlot) {return (theSlot==0)?mData1:mData2;}

	inline bool IsNull() const {return mData1==0 && mData2==0;}
	inline DataPair Abs() {DataPair	aResult;aResult.mData1=(mData1<0)? -mData1:mData1;aResult.mData2=(mData2<0)? -mData2:mData2;return aResult;}
	inline DataPair Sign() {DataPair aResult(0,0);if (mData1<0) aResult.mData1=-1;else if (mData1>0) aResult.mData1=1;if (mData2<0) aResult.mData2=-1;else if (mData2>0) aResult.mData2=1;return aResult;}

	inline bool IsOrigin() {return ((mData1==0)&&mData2==0);}

	var_type mData1;
	var_type mData2;
};

typedef DataPair<float> Pair;
typedef DataPair<int> IPair;

// ---------------------------------------------------------------------------------------------------------------------------------
// Global operators for <float> <op> <Point>  -- so you can do stuff like "1 - vector"
// ---------------------------------------------------------------------------------------------------------------------------------
typedef Point XPoint;

inline Point operator-(const float lhs, const Point & rhs)
{
	return Point(lhs-rhs.mX, lhs-rhs.mY);
}
inline Point operator+(const float lhs, const Point & rhs)
{
	return Point(lhs+rhs.mX, lhs+rhs.mY);
}
inline Point operator*(const float lhs, const Point & rhs)
{
	return Point(lhs*rhs.mX, lhs*rhs.mY);
}
inline Point operator/(const float lhs, const Point & rhs)
{
	return Point(lhs/rhs.mX, lhs/rhs.mY);
}

inline IPoint operator-(const float lhs, const IPoint & rhs)
{
	return IPoint(lhs-rhs.mX, lhs-rhs.mY);
}
inline IPoint operator+(const float lhs, const IPoint & rhs)
{
	return IPoint(lhs+rhs.mX, lhs+rhs.mY);
}
inline IPoint operator*(const float lhs, const IPoint & rhs)
{
	return IPoint(lhs*rhs.mX, lhs*rhs.mY);
}
inline IPoint operator/(const float lhs, const IPoint & rhs)
{
	return IPoint(lhs/rhs.mX, lhs/rhs.mY);
}


//
// Vector for 3D stuff...
//
class Vector
{
public:
	Vector() {mX=mY=mZ=0;}
	Vector(const float theX, const float theY, const float theZ) {mX=theX;mY=theY;mZ=theZ;}
	Vector(const int theX, const int theY, const int theZ) {mX=(float)theX;mY=(float)theY;mZ=(float)theZ;}

	Vector(const int theX, const int theY, const float theZ) {mX=(float)theX;mY=(float)theY;mZ=(float)theZ;}
	Vector(const int theX, const float theY, const float theZ) {mX=(float)theX;mY=(float)theY;mZ=(float)theZ;}
	Vector(const float theX, const float theY, const int theZ) {mX=(float)theX;mY=(float)theY;mZ=(float)theZ;}
	Vector(const float theX, const int theY, const int theZ) {mX=(float)theX;mY=(float)theY;mZ=(float)theZ;}
	Vector(const float theX, const int theY, const float theZ) {mX=(float)theX;mY=(float)theY;mZ=(float)theZ;}
	Vector(const int theX, const float theY, const int theZ) {mX=(float)theX;mY=(float)theY;mZ=(float)theZ;}
	Vector(const Point theXY, const float theZ) {mX=(float)theXY.mX;mY=(float)theXY.mY;mZ=(float)theZ;}

	Vector(const float theVal) {mX=theVal;mY=theVal;mZ=theVal;}

/*    
	Vector(const float theX, const float theY, const int theZ) {mX =(float)theX;mY=(float)theY;mZ=(float)theZ;}
	Vector(const float theX, const int theY, const float theZ) {mX =(float)theX;mY=(float)theY;mZ=(float)theZ;}
	Vector(const int theX, const float theY, const float theZ) {mX =(float)theX;mY=(float)theY;mZ=(float)theZ;}
	Vector(const int theX, const float theY, const int theZ) {mX =(float)theX;mY=(float)theY;mZ=(float)theZ;}
*/    

	Vector(const Vector &theVector) {*this = theVector;}
	Vector(const Point& thePoint) {mX=thePoint.mX;mY=thePoint.mY;mZ=0;}

	inline void Reset() {mX=mY=mZ=0;}
	char* ToString(int thePrecision=9999);
#ifdef _DEBUG
	char* ToCString();
#endif

	inline Point ToPoint() {return Point(mX,mY);}
	inline Point ToPointYZ() {return Point(mY,mZ);}
	inline Point ToPointZY() {return Point(mZ,mY);}
	inline Point ToPointXZ() {return Point(mX,mZ);}
	inline Point ToPointZX() {return Point(mZ,mX);}
	inline Point ToPointXY() {return Point(mX,mY);}
	inline Point ToPointYX() {return Point(mY,mX);}

	Vector& Randomize(float theMagnitude=1, bool doNormalize=true);
	static Vector GetRandom(float theMagnitude=1, bool doNormalize=true) {Vector aV;aV.Randomize(theMagnitude,doNormalize);return aV;}

	inline Vector& SwapXY() {float aHold=mX;mX=mY;mY=aHold;return *this;}
	inline Vector& SwapXZ() {float aHold=mX;mX=mZ;mZ=aHold;return *this;}
	inline Vector& SwapYZ() {float aHold=mY;mY=mZ;mZ=aHold;return *this;}

	inline bool IsNAN() {return (mX!=mX || mY!=mY || mZ!=mZ);}
	inline void SetNAN() {mX=(float)nan("");}

	inline Vector ToVector() {return *this;} // Compatibility layer

	inline float Biggest() {return (float)_max(_max(fabs(mX),fabs(mY)),fabs(mZ));}
	inline float Smallest() {return (float)_min(_min(fabs(mX),fabs(mY)),fabs(mZ));}
	inline float Max() {return Biggest();}
	inline float Min() {return Smallest();}

	inline float AngleBetween(Vector theOther) {return acosf(Dot(theOther)/(Length()*theOther.Length()));}



	inline Vector&			operator=(const Vector& theVector)
	{
		if (this != &theVector)
		{
			mX = theVector.mX;
			mY = theVector.mY;
			mZ = theVector.mZ;
		}
		return *this;
	}
	inline Vector&			operator=(const Point& thePoint)
	{
		mX = thePoint.mX;
		mY = thePoint.mY;
		mZ = 0;
		return *this;
	}
	inline Vector&			operator=(const float& theFloat)
	{
		mX=theFloat;
		mY=theFloat;
		mZ=theFloat;
		return *this;
	}

	//inline Vector &operator=(float* theValue) {mX=(float)theValue[0];mY=(float)theValue[1];mZ=(float)theValue[1];return *this;}
	//inline Vector &operator=(int* theValue) {mX=(float)theValue[0];mY=(float)theValue[1];mZ=(float)theValue[1];return *this;}

	// Equality, Inequality

	inline bool				operator==(const Vector& theVector) {return mX==theVector.mX && mY==theVector.mY && mZ==theVector.mZ;}
	inline bool				operator!=(const Vector& theVector) {return mX!=theVector.mX || mY!=theVector.mY || mZ!=theVector.mZ;}
	inline bool				PrettyClose(Vector theVector) {return (::PrettyClose(mX,theVector.mX) && ::PrettyClose(mY,theVector.mY) && ::PrettyClose(mZ,theVector.mZ));}
	inline Vector			Lerp(Vector theOther, float theAmount) {return *this+((theOther-*this)*theAmount);}
	inline Vector			LerpNormal(Vector theOther, float theAmount) {return Vector(*this+((theOther-*this)*theAmount)).Normal();}

	inline Vector			FastOrtho(bool normalize = true) 
	{
		float aSqr=mX*mX+mY*mY;
		if(aSqr>0.0f) 
		{
			float aIm=normalize? 1.0f/sqrtf(aSqr):1.0f;
			return Vector(-mY*aIm,mX*aIm, 0.0f);
		}
		else 
		{
			aSqr=mY*mY+mZ*mZ;
			float aIm=normalize? 1.0f/sqrtf(aSqr):1.0f;
			return Vector(0.0f,mZ*aIm,mY*aIm);
		}
	}

	Vector			Slerp(Vector theOther, float t);


	// Component-wise operations

	inline const Vector		operator*(const Vector &theVector) const {return Vector(mX*theVector.mX,mY*theVector.mY,mZ*theVector.mZ);}
	inline Vector&			operator*=(const Vector &theVector) {mX*=theVector.mX;mY*=theVector.mY;mZ*=theVector.mZ;return *this;}
	inline const Vector		operator*(const float theFloat) const {return Vector(mX*theFloat,mY*theFloat,mZ*theFloat);}
	inline Vector&			operator*=(const float theFloat) {mX*=theFloat;mY*=theFloat;mZ*=theFloat;return *this;}
	inline const Vector		operator/(const Vector& theVector) const {return Vector(mX/theVector.mX,mY/theVector.mY,mZ/theVector.mZ);}
	inline Vector&			operator/=(const Vector& theVector) {mX/=theVector.mX;mY/=theVector.mY;mZ/=theVector.mZ;return *this;}
	inline const Vector		operator/(const float theFloat) const {return Vector(mX/theFloat,mY/theFloat,mZ/theFloat);}
	inline Vector&			operator/=(const float theFloat) {mX/=theFloat;mY/=theFloat;mZ/=theFloat;return *this;}
	inline const Vector		operator+(const Vector& theVector) const {return Vector(mX+theVector.mX,mY+theVector.mY,mZ+theVector.mZ);}
	inline Vector&			operator+=(const Vector& theVector) {mX+=theVector.mX;mY+=theVector.mY;mZ+=theVector.mZ;return *this;}
	inline const Vector		operator+(const float theFloat) const {return Vector(mX+theFloat,mY+theFloat,mZ+theFloat);}
	inline Vector&			operator+=(const float theFloat) {mX+=theFloat;mY+=theFloat;mZ+=theFloat;return *this;}
	inline const Vector		operator-(const Vector& theVector) const {return Vector(mX-theVector.mX,mY-theVector.mY,mZ-theVector.mZ);}
	inline Vector&			operator-=(const Vector& theVector) {mX-=theVector.mX;mY-=theVector.mY;mZ-=theVector.mZ;return *this;}
	inline const Vector		operator-(const float theFloat) const {return Vector(mX-theFloat,mY-theFloat,mZ-theFloat);}
	inline Vector&			operator-=(const float theFloat) {mX-=theFloat;mY-=theFloat;mZ-=theFloat;return *this;}

	inline Vector			Negate() {return Vector(-mX,-mY,-mZ);}
	inline Vector			Plus(Vector theOther) {return Vector(mX+theOther.mX,mY+theOther.mY,mZ+theOther.mZ);}
	inline Vector			Minus(Vector theOther) {return Vector(mX-theOther.mX,mY-theOther.mY,mZ-theOther.mZ);}
	inline Vector			Times(float theMult) {return Vector(mX*theMult,mY*theMult,mZ*theMult);}
	inline Vector			DividedBy(float theDiv) {return Vector(mX/theDiv,mY/theDiv,mZ/theDiv);}


	// Negation
	inline const Vector		operator-() const {return Vector(-mX,-mY,-mZ);}

	// Standard vector operations
	inline float			Dot(const Vector& theVector) const {return mX*theVector.mX+mY*theVector.mY+mZ*theVector.mZ;}
	inline float			DotAbs(const Vector& theVector) const {return (float)fabs((float)(mX*theVector.mX+mY*theVector.mY+mZ*theVector.mZ));}
	inline float			AbsDot(const Vector& theVector) const {return (float)fabs((float)(mX*theVector.mX+mY*theVector.mY+mZ*theVector.mZ));}
	inline double			DotD(const Vector& theVector) const {return (double)mX*(double)theVector.mX+(double)mY*(double)theVector.mY+(double)mZ*(double)theVector.mZ;}

	inline Vector			Cross(const Vector& theVector) const
	{
		Vector	aResult;

		aResult.mX=(mY*theVector.mZ)-(theVector.mY*mZ);
		aResult.mY=(mZ*theVector.mX)-(theVector.mZ*mX);
		aResult.mZ=(mX*theVector.mY)-(theVector.mX*mY);

		return aResult;
	}

	inline Vector			Perp()
	{
		if (LengthSquared()==0) return Vector(0,0,0);
		if (fabs(mX)<.0001f) return Vector(1,0,0);
		if (fabs(mY)<.0001f) return Vector(0,1,0);
		if (fabs(mZ)<.0001f) return Vector(0,0,1);
		Vector aResult=Vector(1.0f, 1.0f, -1.0f * (mX+mY)/mZ);
		return aResult.Normal();
	}

	Vector					Bounce(const Vector& theNormal);

	inline float			LengthSquared() const {return Dot(*this);}
	inline float			Length() const {float aLen=LengthSquared();if (aLen<=0) return 0;return static_cast<float>(sqrt((double)aLen));}
	//float					FastLength();
	inline bool				IsNull() const {return mX==0 && mY==0 && mZ==0;}
	inline bool				IsOrigin() const {return IsNull();}
	inline void				SetLength(float theLength) {float aScale=Length();if (aScale>0) aScale=theLength/aScale;*this*=aScale;}
	inline void				SetDir(Vector theDir) {float aLen=Length();*this=theDir;SetLength(aLen);}
	inline void				Normalize() {SetLength(1);}
	inline Vector			Normal() {Vector aP=*this;aP.SetLength(1);return aP;}
	inline float			Norm2() {return mX*mX+mY*mY+mZ*mZ;}
	inline Vector			GetNormal() {return Normal();}
	inline Vector			Abs()
	{
		Vector	aResult;
		aResult.mX = (mX<0) ? -mX:mX;
		aResult.mY = (mY<0) ? -mY:mY;
		aResult.mZ = (mZ<0) ? -mZ:mZ;
		return aResult;
	}
	inline Vector			Sign()
	{
		Vector	aResult;
		if (mX!=0) aResult.mX = (mX<0) ? -1.0f:1.0f;
		if (mY!=0) aResult.mY = (mY<0) ? -1.0f:1.0f;
		if (mZ!=0) aResult.mZ = (mZ<0) ? -1.0f:1.0f;
		return aResult;
	}
	inline void Null() {mX=mY=mZ=0;}

	Vector					MajorAxis();
	inline void				Rotate2DCW() {float aHold=mX;mX=-mY;mY=aHold;}
	inline Vector			Floor() {return Vector(floorf(mX),floorf(mY),floorf(mZ));}
	inline Vector			Ceil() {return Vector(ceilf(mX),ceilf(mY),ceilf(mZ));}
	inline void				Shift() {float aHold=mX;mX=mY;mY=mZ;mZ=aHold;}

	float					operator[](int theSlot) {return mData[theSlot];}
	inline float*			AsFloat() {return mData;}

	//
	// Gets the vector component along another vector direction (theDir should be normalized)
	//
	//Vector					Component(Vector theDir) {return Vector(mX,mY,mZ)*Dot(theDir);}
	//Vector					Component(Vector theDir) {return theDir*theDir.Dot(*this);}
	Vector					Component(Vector theDir) {return theDir*theDir.Dot(*this);}
	float					ComponentLength(Vector theDir) {return (float)fabs(Dot(theDir));}
	float					ComponentLengthSigned(Vector theDir) {return Dot(theDir);}

	inline Vector&			Maximize(Vector theOther) {mX=_max(mX,theOther.mX);mY=_max(mY,theOther.mY);mZ=_max(mY,theOther.mZ);return *this;}
	inline Vector&			Minimize(Vector theOther) {mX=_min(mX,theOther.mX);mY=_min(mY,theOther.mY);mZ=_min(mZ,theOther.mZ);return *this;}
	inline Vector&			Maximize() {float aMax=Biggest();mX=aMax;mY=aMax;mZ=aMax;return *this;}
	inline Vector&			Minimize() {float aMin=Smallest();mX=aMin;mY=aMin;mZ=aMin;return *this;}

	void Write(IOBuffer& theBuffer);
	void Read(IOBuffer& theBuffer);
	void Sync(SyncBuffer& theBuffer);



	// The 3D components
	union
	{
		float mData[3];
		struct 
		{
			float		mX;
			float		mY;
			float		mZ;
		};
		struct
		{
			float		mMin;
			union
			{
				float		mMid;
				float		mFraction;
				float		mInterpolation;
			};
			float		mMax;
		};
	};
};

class VectorW : public Vector
{
public:
	float	mW=1.0f;

	char* ToString(int thePrecision=9999);
};


class IVector
{
public:
	IVector() {mX=mY=mZ=0;}
	IVector(const float theX, const float theY, const float theZ) {mX = (int)theX;mY=(int)theY;mZ=(int)theZ;}
	IVector(const int theX, const int theY, const int theZ) {mX =theX;mY=theY;mZ=theZ;}

	IVector(const Vector &theVector) {mX=(int)theVector.mX;mY=(int)theVector.mY;mZ=(int)theVector.mZ;}
	IVector(const IPoint& thePoint) {mX=(int)thePoint.mX;mY=(int)thePoint.mY;mZ=0;}
	//IVector(const IPoint& thePoint) {mX=(int)thePoint.mX;mY=(int)thePoint.mY;mZ=0;}
	IVector(const int theInt) {mX=theInt;mY=theInt;mZ=theInt;}

	char* ToString();


	inline void Reset() {mX=mY=mZ=0;}

	inline IVector&			Maximize(IVector theOther) {mX=_max(mX,theOther.mX);mY=_max(mY,theOther.mY);mZ=_max(mY,theOther.mZ);return *this;}
	inline IVector&			Minimize(IVector theOther) {mX=_min(mX,theOther.mX);mY=_min(mY,theOther.mY);mZ=_min(mZ,theOther.mZ);return *this;}


	// Operator =

	inline IVector&			operator=(const IVector& theVector)
	{
		if (this != &theVector)
		{
			mX = theVector.mX;
			mY = theVector.mY;
			mZ = theVector.mZ;
		}
		return *this;
	}
	inline IVector&			operator=(const Point& thePoint)
	{
		mX =(int)thePoint.mX;
		mY =(int)thePoint.mY;
		mZ = 0;
		return *this;
	}


	// Equality, Inequality

	inline bool				operator==(const IVector& theVector) {return mX==theVector.mX && mY==theVector.mY && mZ==theVector.mZ;}
	inline bool				operator!=(const IVector& theVector) {return mX!=theVector.mX || mY!=theVector.mY || mZ!=theVector.mZ;}

	// Component-wise operations

	inline const IVector		operator*(const IVector &theVector) const {return Vector(mX*theVector.mX,mY*theVector.mY,mZ*theVector.mZ);}
	inline IVector&			operator*=(const IVector &theVector) {mX*=theVector.mX;mY*=theVector.mY;mZ*=theVector.mZ;return *this;}
	inline const IVector		operator*(const int theFloat) const {return Vector(mX*theFloat,mY*theFloat,mZ*theFloat);}
	inline IVector&			operator*=(const int theFloat) {mX*=theFloat;mY*=theFloat;mZ*=theFloat;return *this;}
	inline const IVector		operator/(const IVector& theVector) const {return Vector(mX/theVector.mX,mY/theVector.mY,mZ/theVector.mZ);}
	inline IVector&			operator/=(const IVector& theVector) {mX/=theVector.mX;mY/=theVector.mY;mZ/=theVector.mZ;return *this;}
	inline const IVector		operator/(const int theFloat) const {return Vector(mX/theFloat,mY/theFloat,mZ/theFloat);}
	inline IVector&			operator/=(const int theFloat) {mX/=theFloat;mY/=theFloat;mZ/=theFloat;return *this;}
	inline const IVector		operator+(const IVector& theVector) const {return Vector(mX+theVector.mX,mY+theVector.mY,mZ+theVector.mZ);}
	inline IVector&			operator+=(const IVector& theVector) {mX+=theVector.mX;mY+=theVector.mY;mZ+=theVector.mZ;return *this;}
	inline const IVector		operator+(const int theFloat) const {return Vector(mX+theFloat,mY+theFloat,mZ+theFloat);}
	inline IVector&			operator+=(const int theFloat) {mX+=theFloat;mY+=theFloat;mZ+=theFloat;return *this;}
	inline const IVector		operator-(const IVector& theVector) const {return Vector(mX-theVector.mX,mY-theVector.mY,mZ-theVector.mZ);}
	inline IVector&			operator-=(const IVector& theVector) {mX-=theVector.mX;mY-=theVector.mY;mZ-=theVector.mZ;return *this;}
	inline const IVector		operator-(const int theFloat) const {return Vector(mX-theFloat,mY-theFloat,mZ-theFloat);}
	inline IVector&			operator-=(const int theFloat) {mX-=theFloat;mY-=theFloat;mZ-=theFloat;return *this;}

	// Negation
	inline const IVector	operator-() const {return IVector(-mX,-mY,-mZ);}

	inline void				Null() {mX=mY=mZ=0;}
	inline void				Rotate2DCW() {int aHold=mX;mX=-mY;mY=aHold;}

	void Write(IOBuffer& theBuffer);
	void Read(IOBuffer& theBuffer);
	void Sync(SyncBuffer& theBuffer);


	// The 3D components

	int			mX;
	int			mY;
	int			mZ;
};

class DVector
{
public:
	DVector() {mX=mY=mZ=0;}
	DVector(const double theX, const double theY, const double theZ) {mX = theX;mY=theY;mZ=theZ;}
	DVector(const int theX, const int theY, const int theZ) {mX =(double)theX;mY=(double)theY;mZ=(double)theZ;}


	DVector(const double theX, const double theY, const int theZ) {mX =(double)theX;mY=(double)theY;mZ=(double)theZ;}
	DVector(const double theX, const int theY, const double theZ) {mX =(double)theX;mY=(double)theY;mZ=(double)theZ;}
	DVector(const int theX, const double theY, const double theZ) {mX =(double)theX;mY=(double)theY;mZ=(double)theZ;}
	DVector(const int theX, const double theY, const int theZ) {mX =(double)theX;mY=(double)theY;mZ=(double)theZ;}


	DVector(const DVector &theDVector) {*this = theDVector;}
	DVector(const Vector &theVector) {mX=theVector.mX;mY=theVector.mY;mZ=theVector.mZ;}
	DVector(const Point& thePoint) {mX=thePoint.mX;mY=thePoint.mY;mZ=0;}

	inline bool PrettyClose(DVector theVector) 
	{
		double aD1=theVector.mX-mX;double aD2=theVector.mY-mY;double aD3=theVector.mZ-mZ;
		double aDD=((aD1*aD1)+(aD2*aD2)+(aD3*aD3));
		return (aDD<.000001*.000001);
	}


	inline void Reset() {mX=mY=mZ=0;}

	// Operator =

	inline DVector&			operator=(const DVector& theDVector)
	{
		if (this != &theDVector)
		{
			mX = theDVector.mX;
			mY = theDVector.mY;
			mZ = theDVector.mZ;
		}
		return *this;
	}
	inline DVector&			operator=(const Point& thePoint)
	{
		mX = thePoint.mX;
		mY = thePoint.mY;
		mZ = 0;
		return *this;
	}
	inline DVector&			operator=(const Vector& theVector)
	{
		mX = theVector.mX;
		mY = theVector.mY;
		mZ = theVector.mZ;
		return *this;
	}


	// Equality, Inequality

	inline bool				operator==(const DVector& theDVector) {return mX==theDVector.mX && mY==theDVector.mY && mZ==theDVector.mZ;}
	inline bool				operator!=(const DVector& theDVector) {return mX!=theDVector.mX || mY!=theDVector.mY || mZ!=theDVector.mZ;}
	inline bool				operator==(const Vector& theVector) {return mX==theVector.mX && mY==theVector.mY && mZ==theVector.mZ;}

	// Component-wise operations

	inline const DVector		operator*(const DVector &theDVector) const {return DVector(mX*theDVector.mX,mY*theDVector.mY,mZ*theDVector.mZ);}
	inline DVector&			operator*=(const DVector &theDVector) {mX*=theDVector.mX;mY*=theDVector.mY;mZ*=theDVector.mZ;return *this;}
	inline const DVector		operator*(const double thedouble) const {return DVector(mX*thedouble,mY*thedouble,mZ*thedouble);}
	inline DVector&			operator*=(const double thedouble) {mX*=thedouble;mY*=thedouble;mZ*=thedouble;return *this;}
	inline const DVector		operator/(const DVector& theDVector) const {return DVector(mX/theDVector.mX,mY/theDVector.mY,mZ/theDVector.mZ);}
	inline DVector&			operator/=(const DVector& theDVector) {mX/=theDVector.mX;mY/=theDVector.mY;mZ/=theDVector.mZ;return *this;}
	inline const DVector		operator/(const double thedouble) const {return DVector(mX/thedouble,mY/thedouble,mZ/thedouble);}
	inline DVector&			operator/=(const double thedouble) {mX/=thedouble;mY/=thedouble;mZ/=thedouble;return *this;}
	inline const DVector		operator+(const DVector& theDVector) const {return DVector(mX+theDVector.mX,mY+theDVector.mY,mZ+theDVector.mZ);}
	inline DVector&			operator+=(const DVector& theDVector) {mX+=theDVector.mX;mY+=theDVector.mY;mZ+=theDVector.mZ;return *this;}
	inline const DVector		operator+(const double thedouble) const {return DVector(mX+thedouble,mY+thedouble,mZ+thedouble);}
	inline DVector&			operator+=(const double thedouble) {mX+=thedouble;mY+=thedouble;mZ+=thedouble;return *this;}
	inline const DVector		operator-(const DVector& theDVector) const {return DVector(mX-theDVector.mX,mY-theDVector.mY,mZ-theDVector.mZ);}
	inline DVector&			operator-=(const DVector& theDVector) {mX-=theDVector.mX;mY-=theDVector.mY;mZ-=theDVector.mZ;return *this;}
	inline const DVector		operator-(const double thedouble) const {return DVector(mX-thedouble,mY-thedouble,mZ-thedouble);}
	inline DVector&			operator-=(const double thedouble) {mX-=thedouble;mY-=thedouble;mZ-=thedouble;return *this;}

	inline const DVector		operator*(const Vector &theVector) const {return DVector(mX*theVector.mX,mY*theVector.mY,mZ*theVector.mZ);}
	inline DVector&			operator*=(const Vector &theVector) {mX*=theVector.mX;mY*=theVector.mY;mZ*=theVector.mZ;return *this;}
	inline const DVector		operator/(const Vector& theVector) const {return DVector(mX/theVector.mX,mY/theVector.mY,mZ/theVector.mZ);}
	inline DVector&			operator/=(const Vector& theVector) {mX/=theVector.mX;mY/=theVector.mY;mZ/=theVector.mZ;return *this;}
	inline const DVector		operator+(const Vector& theVector) const {return DVector(mX+theVector.mX,mY+theVector.mY,mZ+theVector.mZ);}
	inline DVector&			operator+=(const Vector& theVector) {mX+=theVector.mX;mY+=theVector.mY;mZ+=theVector.mZ;return *this;}
	inline const DVector		operator-(const Vector& theVector) const {return DVector(mX-theVector.mX,mY-theVector.mY,mZ-theVector.mZ);}
	inline DVector&			operator-=(const Vector& theVector) {mX-=theVector.mX;mY-=theVector.mY;mZ-=theVector.mZ;return *this;}

	// Negation
	inline const DVector		operator-() const {return DVector(-mX,-mY,-mZ);}

	// Standard DVector operations
	inline double			Dot(const DVector& theDVector) const {return mX*theDVector.mX+mY*theDVector.mY+mZ*theDVector.mZ;}

	inline const DVector		Cross(const DVector& theDVector) const
	{
		DVector	aResult;
		aResult.mX=(mY*theDVector.mZ)-(theDVector.mY*mZ);
		aResult.mY=(mZ*theDVector.mX)-(theDVector.mZ*mX);
		aResult.mZ=(mX*theDVector.mY)-(theDVector.mX*mY);
		return aResult;
	}
	inline double			LengthSquared() const {return Dot(*this);}
	inline double			Length() const {double aLen=LengthSquared();if (aLen<=0) return 0;return static_cast<double>(sqrt((double)aLen));}
	//double					FastLength();
	inline bool				IsNull() const {return mX==0 && mY==0 && mZ==0;}
	inline bool				IsOrigin() const {return IsNull();}
	inline void				SetLength(double theLength) {double aScale=Length();if (aScale>0) aScale=theLength/aScale;*this*=aScale;}
	inline void				Normalize() {SetLength(1);}
	inline DVector			Normal() {DVector aP=*this;aP.SetLength(1);return aP;}
	inline DVector			Lerp(DVector theOther, float theAmount) {return *this+((theOther-*this)*theAmount);}
	inline DVector			LerpNormal(DVector theOther, float theAmount) {return DVector(*this+((theOther-*this)*theAmount)).Normal();}

	void Write(IOBuffer& theBuffer);
	void Read(IOBuffer& theBuffer);
	void Sync(SyncBuffer& theBuffer);


	inline DVector			Abs()
	{
		DVector	aResult;
		aResult.mX = (mX<0) ? -mX:mX;
		aResult.mY = (mY<0) ? -mY:mY;
		aResult.mZ = (mZ<0) ? -mZ:mZ;
		return aResult;
	}
	inline DVector			Sign()
	{
		DVector aResult(0,0,0);
		if (mX<0) aResult.mX=-1;
		else if (mX>0) aResult.mY=1;
		if (mY<0) aResult.mY=-1;
		else if (mY>0) aResult.mY=1;
		if (mZ<0) aResult.mZ=-1;
		else if (mZ>0) aResult.mZ=1;
		return aResult;
	}
	inline void Null() {mX=mY=mZ=0;}



	// The 3D components

	double		mX;
	double		mY;
	double		mZ;
};


#include "util_core.h"
class Plane
{
public:
	Plane() {mD=0;}
	inline void Null() {mD=0;}
	//Plane(Vector& thePoint,Vector& theNormal) {Create(thePoint,theNormal);}
	Plane(Vector thePoint,Vector theNormal) {Create(thePoint,theNormal);}
	Plane(Vector theP1, Vector theP2, Vector theP3) {Create(theP1,theP2,theP3);}
	Plane(DVector theP1, DVector theP2, DVector theP3) {Create(theP1,theP2,theP3);}

public:
	Vector		mPos;
	Vector		mNormal;
	float		mD;

	// Equation is mNormal.mX,mNormal.mY,mNormal.mZ,mD

	void			Create(Vector thePoint, Vector theNormal, bool isNormalized=false)
	{
		if (!isNormalized) theNormal.Normalize();
		mPos=thePoint;
		mNormal=theNormal;
		mD=-theNormal.Dot(thePoint);
		//if (isNormalized) mD=-theNormal.Dot(thePoint);
		//else mD=-theNormal.Normal().Dot(thePoint);
	}
	inline void Rehup() {mD=-mNormal.Normal().Dot(mPos);}
	inline void Freshen() {Rehup();}

	void			Create(DVector thePoint, DVector theNormal, bool isNormalized=false)
	{
		if (!isNormalized) theNormal.Normalize();
		mPos.mX=(float)thePoint.mX;
		mPos.mY=(float)thePoint.mY;
		mPos.mZ=(float)thePoint.mZ;
		mNormal.mX=(float)theNormal.mX;
		mNormal.mY=(float)theNormal.mY;
		mNormal.mZ=(float)theNormal.mZ;
		mD=(float)-theNormal.Dot(thePoint);
	}

	void			Create(const Vector& theP1,const Vector& theP2, const Vector& theP3)
	{
		mNormal=-(theP2-theP1).Cross(theP3-theP1); // Negative for CCW
		mNormal.Normalize();
		mPos=(theP1+theP2+theP3)/3;
		mD=-mNormal.Dot(mPos);
	}

	void			Create(const DVector& theP1,const DVector& theP2, const DVector& theP3)
	{
		DVector aNormal=-(theP2-theP1).Cross(theP3-theP1);  // Negative for CCW
		aNormal.Normalize();
		DVector aPos=(theP1+theP2+theP3)/3;
		mD=-mNormal.Dot(mPos);
		mPos.mX=(float)aPos.mX;
		mPos.mY=(float)aPos.mY;
		mPos.mZ=(float)aPos.mZ;
		mNormal.mX=(float)aNormal.mX;
		mNormal.mY=(float)aNormal.mY;
		mNormal.mZ=(float)aNormal.mZ;
	}


	void			Normalize()
	{
		float aDistance = sqrtf(mPos.mX*mPos.mX + mPos.mY*mPos.mY + mPos.mZ*mPos.mZ);
		mPos.mX/=aDistance;
		mPos.mY/=aDistance;
		mPos.mZ/=aDistance;
		mD/=aDistance;
	}

	void			NormalizePlus(float theMult)
	{
		float aDistance = sqrtf(mPos.mX*mPos.mX + mPos.mY*mPos.mY + mPos.mZ*mPos.mZ);
		aDistance*=theMult;

		mPos.mX/=aDistance;
		mPos.mY/=aDistance;
		mPos.mZ/=aDistance;
		mD/=aDistance;
	}


	inline float	Dot(Vector theVec) {return (((mPos.mX)*(theVec.mX))+((mPos.mY)*(theVec.mY))+((mPos.mZ)*(theVec.mZ))+(mD));}
	inline bool		IsFacing(const Vector& theDirection) const {double aDot=mNormal.DotD(theDirection);return (aDot<=0);}
	inline float	SignedDistanceTo(const Vector& thePoint) const {return (thePoint.Dot(mNormal))+mD;}
	inline float	DistanceTo(const Vector& thePoint) const {return (float)fabs((thePoint.Dot(mNormal))+mD);}
	inline double	SignedDistanceToD(const Vector& thePoint) const {return (thePoint.DotD(mNormal))+mD;}
	inline double	DistanceToD(const Vector& thePoint) const {return (float)fabs((thePoint.DotD(mNormal))+mD);}
	inline void		ToFloats(float* theResult) {*theResult=mNormal.mX;theResult++;*theResult=mNormal.mY;theResult++;*theResult=mNormal.mZ;theResult++;*theResult=-mD;}
	inline float	F0() {return mNormal.mX;}
	inline float	F1() {return mNormal.mY;}
	inline float	F2() {return mNormal.mZ;}
	inline float	F3() {return -mD;}

	bool			IsSame(Plane theOther, float theThreshold=.00001f);
	bool			IsCoplanar(Plane theOther, float theThreshold=.00001f);
	bool			IsOpposite(Plane theOther, float theThreshold=.00001f);
	inline Vector	ClosestPoint(Vector thePos) {return thePos-mNormal*SignedDistanceTo(thePos);}

	Vector			Planify(Vector thePos);	 // Returns thePos moved into an XY plane (assuming it lies on this plane) so we can make angular computations and stuff
	Vector			PlanifyN(Vector thePos);	 // Returns thePos moved into an XY plane (assuming it lies on this plane) so we can make angular computations and stuff
	Vector			UnPlanify(Vector thePos);	 // Returns thePos moved into an XY plane (assuming it lies on this plane) so we can make angular computations and stuff
	Vector			UnPlanifyN(Vector thePos);	 // Returns thePos moved into an XY plane (assuming it lies on this plane) so we can make angular computations and stuff
	inline Vector	FlattenPos(Vector thePos) {return Planify(thePos);}

	Vector			ProjectOnto(Vector thePos);

	void			Planify(Array<Vector>& theList);	 // Returns thePos moved into an XY plane (assuming it lies on this plane) so we can make angular computations and stuff
	void			UnPlanify(Array<Vector>& theList);	 // Returns thePos moved into an XY plane (assuming it lies on this plane) so we can make angular computations and stuff

	void			Planify(Vector* theList, int theCount);	 // Returns thePos moved into an XY plane (assuming it lies on this plane) so we can make angular computations and stuff
	void			UnPlanify(Vector* theList, int theCount);	 // Returns thePos moved into an XY plane (assuming it lies on this plane) so we can make angular computations and stuff


	void			Planify(Array<Vector>& theList,Array<Point>& theResult, float theSnap=0);	 // Returns thePos moved into an XY plane (assuming it lies on this plane) so we can make angular computations and stuff
	void			UnPlanify(Array<Point>& theInList, Array<Vector>& theOutList);	 // Returns thePos moved into an XY plane (assuming it lies on this plane) so we can make angular computations and stuff

	char			ClassifyPoint(Vector thePoint, float theEpsilon=0.0005f) {Vector aDir=mPos-thePoint;float aD=aDir.Dot(mNormal);if (aD<-theEpsilon) return 1;if (aD>theEpsilon) return -1;return 0;}
	inline char		ClassifyPoint(Point thePoint, float theEpsilon=0.0005f) {Vector aPoint=Vector(thePoint,mPos.mZ);return ClassifyPoint(aPoint,theEpsilon);}
	char			ClassifyPointFrontBack(Vector thePoint, float theEpsilon=0.0005f) {Vector aDir=mPos-thePoint;float aD=aDir.Dot(mNormal);if (aD<=0) return 1;return -1;} // Never return 0
	inline bool		IsBehind(Vector thePoint, float theEpsilon=0.0005f) {return ClassifyPoint(thePoint,theEpsilon)<0;}
	inline bool		IsInFront(Vector thePoint, float theEpsilon=0.0005f) {return ClassifyPoint(thePoint,theEpsilon)>0;}
	inline bool		IsOn(Vector thePoint, float theEpsilon=0.0005f) {return ClassifyPoint(thePoint,theEpsilon)==0;}
	inline bool		IsNotInFront(Vector thePoint, float theEpsilon=0.0005f) {return ClassifyPoint(thePoint,theEpsilon)<=0;}
	inline bool		IsNotBehind(Vector thePoint, float theEpsilon=0.0005f) {return ClassifyPoint(thePoint,theEpsilon)>=0;}
	inline bool		IsNotOn(Vector thePoint, float theEpsilon=0.0005f) {return ClassifyPoint(thePoint,theEpsilon)!=0;}

	bool			GetSurfaceVectors(Vector& theResult1, Vector& theResult2);
};

class DPlane
{
public:
	DPlane() {mD=0;}
	inline void Null() {mD=0;}
	//Plane(Vector& thePoint,Vector& theNormal) {Create(thePoint,theNormal);}
	DPlane(Vector thePoint,Vector theNormal) {Create(thePoint,theNormal);}
	DPlane(Vector theP1, Vector theP2, Vector theP3) {Create(theP1,theP2,theP3);}
	DPlane(DVector theP1, DVector theP2, DVector theP3) {Create(theP1,theP2,theP3);}

public:
	DVector		mPos;
	DVector		mNormal;
	double		mD;

	// Equation is mNormal.mX,mNormal.mY,mNormal.mZ,mD

	void			Create(DVector thePoint, DVector theNormal, bool isNormalized=false)
	{
		if (!isNormalized) theNormal.Normalize();
		mPos=thePoint;
		mNormal=theNormal;
		mD=-theNormal.Dot(thePoint);
		//if (isNormalized) mD=-theNormal.Dot(thePoint);
		//else mD=-theNormal.Normal().Dot(thePoint);
	}
	inline void Rehup() {mD=-mNormal.Normal().Dot(mPos);}
	inline void Freshen() {Rehup();}

	void			Create(Vector thePoint, Vector theNormal, bool isNormalized=false)
	{
		if (!isNormalized) theNormal.Normalize();
		mPos.mX=(double)thePoint.mX;
		mPos.mY=(double)thePoint.mY;
		mPos.mZ=(double)thePoint.mZ;
		mNormal.mX=(double)theNormal.mX;
		mNormal.mY=(double)theNormal.mY;
		mNormal.mZ=(double)theNormal.mZ;
		mD=(double)-theNormal.Dot(thePoint);
	}

	void			Create(const Vector& theP1,const Vector& theP2, const Vector& theP3)
	{
		mNormal=-(theP2-theP1).Cross(theP3-theP1); // Negative for CCW
		mNormal.Normalize();
		mPos=(theP1+theP2+theP3)/3;
		mD=-mNormal.Dot(mPos);
	}

	void			Create(const DVector& theP1,const DVector& theP2, const DVector& theP3)
	{
		DVector aNormal=-(theP2-theP1).Cross(theP3-theP1);  // Negative for CCW
		aNormal.Normalize();
		DVector aPos=(theP1+theP2+theP3)/3;
		mD=-mNormal.Dot(mPos);
		mPos.mX=(double)aPos.mX;
		mPos.mY=(double)aPos.mY;
		mPos.mZ=(double)aPos.mZ;
		mNormal.mX=(double)aNormal.mX;
		mNormal.mY=(double)aNormal.mY;
		mNormal.mZ=(double)aNormal.mZ;
	}


	void			Normalize()
	{
		double aDistance = sqrt(mPos.mX*mPos.mX + mPos.mY*mPos.mY + mPos.mZ*mPos.mZ);
		mPos.mX/=aDistance;
		mPos.mY/=aDistance;
		mPos.mZ/=aDistance;
		mD/=aDistance;
	}

	inline double	Dot(DVector theVec) {return (((mPos.mX)*(theVec.mX))+((mPos.mY)*(theVec.mY))+((mPos.mZ)*(theVec.mZ))+(mD));}
	inline bool		IsFacing(const DVector& theDirection) const {double aDot=mNormal.Dot(theDirection);return (aDot<=0);}
	inline double	SignedDistanceTo(const DVector& thePoint) const {return (thePoint.Dot(mNormal))+mD;}
	inline double	DistanceTo(const DVector& thePoint) const {return (double)fabs((thePoint.Dot(mNormal))+mD);}
	inline void		ToDoubles(double* theResult) {*theResult=mNormal.mX;theResult++;*theResult=mNormal.mY;theResult++;*theResult=mNormal.mZ;theResult++;*theResult=-mD;}
	inline double	F0() {return mNormal.mX;}
	inline double	F1() {return mNormal.mY;}
	inline double	F2() {return mNormal.mZ;}
	inline double	F3() {return -mD;}

	bool			IsSame(DPlane theOther, double theThreshold=.00001f);
	bool			IsCoplanar(DPlane theOther, double theThreshold=.00001f);
	bool			IsOpposite(DPlane theOther, double theThreshold=.00001f);
	inline DVector	ClosestPoint(DVector thePos) {return thePos-mNormal*SignedDistanceTo(thePos);}

	DVector			Planify(DVector thePos);	 // Returns thePos moved into an XY plane (assuming it lies on this plane) so we can make angular computations and stuff
	DVector			PlanifyN(DVector thePos);	 // Returns thePos moved into an XY plane (assuming it lies on this plane) so we can make angular computations and stuff
	DVector			UnPlanify(DVector thePos);	 // Returns thePos moved into an XY plane (assuming it lies on this plane) so we can make angular computations and stuff
	DVector			UnPlanifyN(DVector thePos);	 // Returns thePos moved into an XY plane (assuming it lies on this plane) so we can make angular computations and stuff
	inline DVector	FlattenPos(DVector thePos) {return Planify(thePos);}

	DVector			ProjectOnto(DVector thePos);

	void			Planify(Array<DVector>& theList);	 // Returns thePos moved into an XY plane (assuming it lies on this plane) so we can make angular computations and stuff
	void			UnPlanify(Array<DVector>& theList);	 // Returns thePos moved into an XY plane (assuming it lies on this plane) so we can make angular computations and stuff

	void			Planify(DVector* theList, int theCount);	 // Returns thePos moved into an XY plane (assuming it lies on this plane) so we can make angular computations and stuff
	void			UnPlanify(DVector* theList, int theCount);	 // Returns thePos moved into an XY plane (assuming it lies on this plane) so we can make angular computations and stuff


	void			Planify(Array<DVector>& theList,Array<Point>& theResult, double theSnap=0);	 // Returns thePos moved into an XY plane (assuming it lies on this plane) so we can make angular computations and stuff
	void			UnPlanify(Array<Point>& theInList, Array<Vector>& theOutList);	 // Returns thePos moved into an XY plane (assuming it lies on this plane) so we can make angular computations and stuff

	char			ClassifyPoint(DVector thePoint, double theEpsilon=0.0005f) {DVector aDir=mPos-thePoint;double aD=aDir.Dot(mNormal);if (aD<-theEpsilon) return 1;if (aD>theEpsilon) return -1;return 0;}
	char			ClassifyPointFrontBack(DVector thePoint, double theEpsilon=0.0005f) {DVector aDir=mPos-thePoint;double aD=aDir.Dot(mNormal);if (aD<=0) return 1;return -1;} // Never return 0
	inline bool		IsBehind(DVector thePoint, double theEpsilon=0.0005f) {return ClassifyPoint(thePoint,theEpsilon)<0;}
	inline bool		IsInFront(DVector thePoint, double theEpsilon=0.0005f) {return ClassifyPoint(thePoint,theEpsilon)>0;}
	inline bool		IsOn(DVector thePoint, double theEpsilon=0.0005f) {return ClassifyPoint(thePoint,theEpsilon)==0;}
	inline bool		IsNotInFront(DVector thePoint, double theEpsilon=0.0005f) {return ClassifyPoint(thePoint,theEpsilon)<=0;}
	inline bool		IsNotBehind(DVector thePoint, double theEpsilon=0.0005f) {return ClassifyPoint(thePoint,theEpsilon)>=0;}
	inline bool		IsNotOn(DVector thePoint, double theEpsilon=0.0005f) {return ClassifyPoint(thePoint,theEpsilon)!=0;}

	bool			GetSurfaceVectors(DVector& theResult1, DVector& theResult2);
};

// ---------------------------------------------------------------------------------------------------------------------------------
// Global operators for <float> <op> <Vector>  -- so you can do stuff like "1 - Vector"
// ---------------------------------------------------------------------------------------------------------------------------------

inline Vector operator-(const float lhs, const Vector & rhs)
{
	return Vector(lhs-rhs.mX, lhs-rhs.mY, lhs-rhs.mZ);
}
inline Vector operator+(const float lhs, const Vector & rhs)
{
	return Vector(lhs+rhs.mX, lhs+rhs.mY, lhs+rhs.mZ);
}
inline Vector operator*(const float lhs, const Vector & rhs)
{
	return Vector(lhs*rhs.mX, lhs*rhs.mY, lhs*rhs.mZ);
}
inline Vector operator/(const float lhs, const Vector & rhs)
{
	return Vector(lhs/rhs.mX, lhs/rhs.mY, lhs/rhs.mZ);
}

inline IVector operator-(const float lhs, const IVector & rhs)
{
	return IVector(lhs-rhs.mX, lhs-rhs.mY, lhs-rhs.mZ);
}
inline IVector operator+(const float lhs, const IVector & rhs)
{
	return IVector(lhs+rhs.mX, lhs+rhs.mY, lhs+rhs.mZ);
}
inline IVector operator*(const float lhs, const IVector & rhs)
{
	return IVector(lhs*rhs.mX, lhs*rhs.mY, lhs*rhs.mZ);
}
inline IVector operator/(const float lhs, const IVector & rhs)
{
	return IVector(lhs/rhs.mX, lhs/rhs.mY, lhs/rhs.mZ);
}

inline DVector operator-(const double lhs, const DVector & rhs)
{
	return DVector(lhs-rhs.mX, lhs-rhs.mY, lhs-rhs.mZ);
}
inline DVector operator+(const double lhs, const DVector & rhs)
{
	return DVector(lhs+rhs.mX, lhs+rhs.mY, lhs+rhs.mZ);
}
inline DVector operator*(const double lhs, const DVector & rhs)
{
	return DVector(lhs*rhs.mX, lhs*rhs.mY, lhs*rhs.mZ);
}
inline DVector operator/(const double lhs, const DVector & rhs)
{
	return DVector(lhs/rhs.mX, lhs/rhs.mY, lhs/rhs.mZ);
}

inline Point PointMin(const Point& a, const Point& b)
{
	return Point(_min(a.mX, b.mX),_min(a.mY, b.mY));
}

inline Point PointMax(const Point& a, const Point& b)
{
	return Point(_max(a.mX, b.mX),_max(a.mY, b.mY));
}

inline Vector VectorMin(const Vector& a, const Vector& b)
{
	return Vector(_min(a.mX, b.mX),_min(a.mY, b.mY),_min(a.mZ, b.mZ));
}

inline Vector VectorMax(const Vector& a, const Vector& b)
{
	return Vector(_max(a.mX, b.mX),_max(a.mY, b.mY),_max(a.mZ, b.mZ));
}

inline Point Normalize(Point thePoint) {return thePoint.Normal();}
inline Vector Normalize(Vector theVector) {return theVector.Normal();}
inline DVector Normalize(DVector theVector) {return theVector.Normal();}

#include "util_core.h"
class SuperBits
{
public:
	void						Set(short theFlag);
	void						Unset(short theFlag);
	void						Reset();
	void						Set(char* theString);	// Comma seperated string
	bool						IsSet(short theFlag);

	inline void					operator|=(short theValue) {Set(theValue);}
	inline void					operator^=(short theValue) {Unset(theValue);}
	inline bool					operator&(short theValue) {return IsSet(theValue);}

	CleanArray<unsigned int>	mBits;	// The bits
};

/*
class Fraction
{
public:
	Fraction(float theValue) {mValue=(unsigned char)((theValue*255.0f)+.5f);}

	unsigned char				mValue;
	
	inline Fraction &operator=(const float &theValue) {mValue=(unsigned char)(((theValue)*255.0f)+.5f);return *this;}
	virtual operator float() {return (float)mValue/255.0f;}
};
*/

typedef float float4[4];

/*
class DPoint
{
public:
	DPoint() {mX=mY=0.0f;}
	//Point(const float theValue) {mX=theValue;mY=theValue;}
	//Point (const int theValue) {mX=(float)theValue;mY=(float)theValue;}
	DPoint(const float theX, const float theY) {mX=theX;mY=theY;}
	DPoint(const double theX, const double theY) {mX=theX;mY=theY;}
	DPoint(const int theX, const int theY) {mX=(float)theX;mY=(float)theY;}
	DPoint(const Point &thePoint) {*this=thePoint;}
	DPoint(const DPoint &thePoint) {*this=thePoint;}
	DPoint(const IPoint& thePoint);
	DPoint(const Vector& theVector);
	DPoint(const float theVal) {mX=theVal;mY=theVal;}

	inline DPoint &operator=(const DPoint &thePoint) {if (this!=&thePoint) {mX=thePoint.mX;mY=thePoint.mY;}return *this;}
	inline DPoint &operator=(const Point &thePoint) {mX=thePoint.mX;mY=thePoint.mY;return *this;}
	inline DPoint &operator=(float &theValue) {mX=theValue;mY=theValue;return *this;}
	inline DPoint &operator=(float theValue) {mX=theValue;mY=theValue;return *this;}
	inline DPoint &operator=(int &theValue) {mX=(float)theValue;mY=(float)theValue;return *this;}
	inline DPoint &operator=(int theValue) {mX=(float)theValue;mY=(float)theValue;return *this;}

	inline bool IsNAN() {return (mX!=mX || mY!=mY);}


	inline void Reset() {mX=mY=0;}
	void Assign(Point thePos) {*this=thePos;}

	inline bool	operator==(const DPoint &thePoint) {return (mX==thePoint.mX)&(mY==thePoint.mY);}
	inline bool	operator!=(const DPoint &thePoint) {return (mX!=thePoint.mX)|(mY!=thePoint.mY);}

	inline DPoint Lerp(DPoint theOther, float theAmount) {return *this+((theOther-*this)*theAmount);}
	inline DPoint LerpNormal(DPoint theOther, float theAmount) {return DPoint(*this+((theOther-*this)*theAmount)).Normal();}
	inline bool PrettyClose(DPoint thePoint) {return (::PrettyClose(mX,thePoint.mX) && ::PrettyClose(mY,thePoint.mY));}

	inline const DPoint	operator*(const DPoint &thePoint) const {return DPoint(mX*thePoint.mX,mY*thePoint.mY);}
	inline DPoint &operator*=(const DPoint &thePoint) {mX*=thePoint.mX;mY*=thePoint.mY;return *this;}
	inline const DPoint operator*(const float theFloat) const {return DPoint(mX*theFloat,mY*theFloat);}
	inline DPoint &operator*=(const float theFloat) {mX*=theFloat;mY*=theFloat;return *this;}
	inline const DPoint	operator/(const DPoint &thePoint) const {return DPoint(mX/thePoint.mX,mY/thePoint.mY);}
	inline DPoint &operator/=(const DPoint &thePoint) {mX/=thePoint.mX;mY/=thePoint.mY;return *this;}
	inline const DPoint operator/(const float theFloat) const {return DPoint(mX/theFloat,mY/theFloat);}
	inline DPoint &operator/=(const float theFloat) {mX/=theFloat;mY/=theFloat;return *this;}
	inline const DPoint operator+(const DPoint &thePoint) const {return DPoint(mX+thePoint.mX,mY+thePoint.mY);}
	inline DPoint &operator+=(const DPoint &thePoint) {mX+=thePoint.mX;mY+=thePoint.mY;return *this;}
	inline const DPoint operator+(const float theFloat) const {return DPoint(mX+theFloat,mY+theFloat);}
	inline DPoint &operator+=(const float theFloat) {mX+=theFloat;mY+=theFloat;return *this;}
	inline const DPoint operator-(const DPoint &thePoint) const {return DPoint(mX-thePoint.mX,mY-thePoint.mY);}
	inline DPoint &operator-=(const DPoint &thePoint) {mX-=thePoint.mX;mY-=thePoint.mY;return *this;}
	inline const DPoint operator-(const float theFloat) const {return DPoint(mX-theFloat,mY-theFloat);}
	inline DPoint & operator-=(const float theFloat) {mX-=theFloat;mY-=theFloat;return *this;}
	inline const DPoint operator-() const {return DPoint(-mX,-mY);}

	inline double Dot(const DPoint &thePoint) const {return mX*thePoint.mX+mY*thePoint.mY;}
	inline float AbsDot(const DPoint &thePoint) const {return (float)fabs(mX*thePoint.mX+mY*thePoint.mY);}
	inline float DotAbs(const DPoint &thePoint) const {return (float)fabs(mX*thePoint.mX+mY*thePoint.mY);}
	inline float PerpDot(const DPoint& thePoint) const {return mY*thePoint.mX+mX*thePoint.mY;}
	inline const double Cross(const DPoint &thePoint) const {return mX*thePoint.mY-mY*thePoint.mX;}
	inline const double Determinant(const DPoint &thePoint) const {return mX*thePoint.mY-mY*thePoint.mX;} // Same as cross

	inline double LengthSquared() const {return Dot(*this);}
	inline double Length() const {double aLen=LengthSquared();if (aLen<=0) return 0;return static_cast<double>(sqrt(aLen));}
	float GetLengthSquared() {return Dot(*this);}
	float GetLength() {return Length();}
	inline bool IsNull() const {return mX==0 && mY==0;}
	inline void SetLength(const double theLen) {double aScale=Length();if (aScale>0) aScale=theLen/aScale;*this*=aScale;}
	inline void SetDir(DPoint theDir) {double aLen=Length();*this=theDir;SetLength(aLen);}
	inline DPoint Normalize(){SetLength(1);return DPoint(mX,mY);}

	inline DPoint Normal()
	{
		const double aLenSQ=mX*mX+mY*mY;
		if (aLenSQ==0) return DPoint(0,0);
		const double aInvLen=1.0f/(float)sqrt(aLenSQ);
		return DPoint(mX*aInvLen,mY*aInvLen);
		//	Point aP=*this;aP.SetLength(1);return aP;
	}
	inline DPoint Abs() {DPoint	aResult;aResult.mX=(mX<0)? -mX:mX;aResult.mY=(mY<0)? -mY:mY;return aResult;}
	inline DPoint Sign() {DPoint aResult(0,0);if (mX<0) aResult.mX=-1;else if (mX>0) aResult.mX=1;if (mY<0) aResult.mY=-1;else if (mY>0) aResult.mY=1;return aResult;}
	inline DPoint Mod(float theNumber) {return DPoint(fmod(mX,theNumber),fmod(mY,theNumber));}
	inline DPoint Mod(DPoint theNumber) {return DPoint(fmod(mX,theNumber.mX),fmod(mY,theNumber.mY));}
	DPoint RMod(double theNumber);
	DPoint RMod(DPoint theNumber);
	inline DPoint Floor() {return DPoint(floor(mX),floor(mY));}
	inline DPoint Ceil() {return DPoint(ceil(mX),ceil(mY));}
	inline void Null() {mX=mY=0;}
	inline void RotateCW() {double aHold=mX;mX=-mY;mY=aHold;}
	inline double Area() {return mX*mY;}
	inline double Diff() {return mY-mX;}

	inline DPoint MajorAxis()
	{
		DPoint aResult=*this;
		if (fabs(mX)>fabs(mY)) aResult.mY=0;
		else aResult.mX=0;
		return aResult;
	}
	inline DPoint MinorAxis()
	{
		DPoint aResult=*this;
		if (fabs(mX)<fabs(mY)) aResult.mY=0;
		else aResult.mX=0;
		return aResult;
	}


	inline float Biggest() {return (float)_max(fabs(mX),fabs(mY));}
	inline float Smallest() {return (float)_min(fabs(mX),fabs(mY));}
	inline float Max() {return (float)_max(fabs(mX),fabs(mY));}
	inline float Min() {return (float)_min(fabs(mX),fabs(mY));}

	inline DPoint& Maximize(Point theOther) {mX=_max(mX,theOther.mX);mY=_max(mY,theOther.mY);return *this;}
	inline DPoint& Minimize(Point theOther) {mX=_min(mX,theOther.mX);mY=_min(mY,theOther.mY);return *this;}
	inline DPoint& Maximize() {float aMax=Biggest();mX=mY=aMax;return *this;}
	inline DPoint& Minimize() {float aMin=Smallest();mX=mY=aMin;return *this;}


	//
	// Faster, but approximate...
	//
	void SetLengthFast(const float theLen);

	//
	// Gives you the point's perpendicular
	//
	inline DPoint Perp() {return DPoint(mY,-mX);}
	inline DPoint Transpose() {return DPoint(mY,mX);}

	//
	// Lets you know whether the point is at origin
	//
	inline bool IsOrigin() {return ((mX==0)&&mY==0);}

	//
	// This removes the fractional component of a point (and rounds it up/down to the
	// nearest number.
	//
	DPoint	ToInt()
	{
		float aX,aY;
		aX=aY=0;
		if (mX>0) aX=(float)((int)(mX+.5f));
		if (mX<0) aX=(float)((int)(mX-.5f));
		if (mY>0) aY=(float)((int)(mY+.5f));
		if (mY<0) aY=(float)((int)(mY-.5f));

		return DPoint(aX,aY);
		//mX=aX;
		//mY=aY;
	}

	double					operator[](int theSlot) {return mData[theSlot];}
	inline double*			AsDouble() {return mData;}

	union
	{
		double mData[2];
		struct
		{
			union
			{
				double mX;
				double mWidth;
				double mW;
				double mData1;
				double mStart;
				double mLow;
				double mU;
			};
			union
			{
				double mY;
				double mHeight;
				double mH;
				double mData2;
				double mEnd;
				double mHigh;
				double mV;
			};
		};
	};
};

inline DPoint operator-(const float lhs, const DPoint & rhs)
{
	return DPoint(lhs-rhs.mX, lhs-rhs.mY);
}
inline DPoint operator+(const float lhs, const DPoint & rhs)
{
	return DPoint(lhs+rhs.mX, lhs+rhs.mY);
}
inline DPoint operator*(const float lhs, const DPoint & rhs)
{
	return DPoint(lhs*rhs.mX, lhs*rhs.mY);
}
inline DPoint operator/(const float lhs, const DPoint & rhs)
{
	return DPoint(lhs/rhs.mX, lhs/rhs.mY);
}
*/

inline Point RandomPoint(float theMagnitude=1.0f, bool doNormalize=true) {Point aP;aP.Randomize(theMagnitude,doNormalize);return aP;}
inline Vector RandomVector(float theMagnitude=1.0f, bool doNormalize=true) {Vector aV;aV.Randomize(theMagnitude,doNormalize);return aV;}
