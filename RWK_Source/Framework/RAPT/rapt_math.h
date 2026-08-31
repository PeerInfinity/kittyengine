#pragma once

//#define NORANDOM
#include "rapt_console.h"

#include "rapt_point.h"
#include "rapt_rect.h"
#ifndef NORANDOM
#include "rapt_random.h"
#endif


#ifndef NULL
	#define NULL 0
#endif

#define Squared(x) ((x)*(x))

template<typename T>
inline void Swap(T &theVal1, T &theVal2)
{
	T aTemp = theVal1;
	theVal1=theVal2;theVal2=aTemp;
}

class Math
{
public:
	Math(void);

	void			Go();

	////////////////////////////////////////////////////////////////////
	//
	// Rounds up to the nearest power of two.  For instance,
	// 3 rounds up to 4, 27 rounds up to 32, etc, etc.
	//
	int				NearestPowerOfTwo(int theNumber);
	bool			IsPowerOfTwo(int theNumber);
	int				Clip(int theNumber, int theLow, int theHigh);
	int				GetBitPos(unsigned int theBit);
	inline bool		IsEven(int theNumber) {return ((theNumber%2)==0);}
	inline bool		IsOdd(int theNumber) {return ((theNumber%2)!=0);}
	inline float	Decimal(float x) {return x-floorf(x);}
	inline bool		IsNan(float theValue) {return (theValue!=theValue);}
	inline bool		IsNan(int theValue) {return (theValue!=theValue);}
	inline bool		IsNan(longlong theValue) {return (theValue!=theValue);}
	inline bool		IsNan(double theValue) {return (theValue!=theValue);}
	inline bool		IsNAN(float theValue) {return (theValue!=theValue);}
	inline bool		IsNAN(int theValue) {return (theValue!=theValue);}
	inline bool		IsNAN(longlong theValue) {return (theValue!=theValue);}
	inline bool		IsNAN(double theValue) {return (theValue!=theValue);}
	//
	////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////
	//
	// Degrees to radians, and whatnot
	//
	inline float			Deg(float theAngle) {return theAngle*180/mPI;}
	inline float			Rad(float theAngle) {return theAngle*mPI/180.0f;}
	inline float			Deg(int theAngle) {return (float)theAngle*180/mPI;}
	inline float			Rad(int theAngle) {return (float)theAngle*mPI/180.0f;}
	//
	////////////////////////////////////////////////////////////////////

	inline float			Dot2(Vector aV) {return aV.Dot(aV);}

	////////////////////////////////////////////////////////////////////
	//
	//
	// Quick angle to vector...
	//
	Point					AngleToVector(float theAngle);
	inline float			VectorToAngle(Point theVector) {return PointAtAngle2D(theVector);}
	inline float			VectorToAngle(Point theCamera, Point theTarget) {return PointAtAngle2D(theTarget-theCamera);}
#ifndef NORANDOM	
	Point					AngleToVector(Random *theRandom=NULL);
#endif
	//
	////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////
	//
	// Quick abs...
	//
	/*
	static inline int		Abs(int theValue) {return (theValue + (theValue >> 31)) ^ (theValue >> 31);}
	static inline longlong	Abs(longlong theValue) {return (theValue + (theValue >> 63)) ^ (theValue >> 63);}
	*/
	static inline int		Abs(int theValue) {return (theValue>0) ? theValue : -theValue;}
	static inline longlong	Abs(longlong theValue) {return (theValue>0) ? theValue : -theValue;}
	static inline float		Abs(float theValue) {return (theValue>0) ? theValue : -theValue;}
	static inline unsigned int		Abs(unsigned int theValue) {return theValue;} // Crazy catch-all for weird situations
	//
	////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////
	//
	// Mods that will cross 0 linearly without flipping 
	// sign and reversing order
	//
	inline float RMod(float theNumber, float theMod) {float aFix=theNumber;if (theNumber<0) aFix*=-1;float aResult=fmodf(aFix,theMod);if (theNumber<0) {aResult=(theMod)-aResult;if (aResult>=theMod) aResult=0;}return aResult;}
	int RMod(int theNumber, int theMod)	{int aRet=theNumber%theMod;if (aRet<0) aRet=theMod+aRet;return aRet;}
	inline float TowardZero(float theNumber, float theAmount) {if (theNumber>0) return theNumber-theAmount;return theNumber+theAmount;}
	inline int TowardZero(int theNumber, int theAmount) {if (theNumber>0) return theNumber-theAmount;return theNumber+theAmount;}
	//
	////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////
	//
	// Fast sqrt... from Quake3
	// Warning: Not accurate... but accurate enough for most purposes!
	// Now depreciated: Apple's latest compiler has turned this ito garbage on iOS, so it's no
	// longer cross-platform code.  Apple: Undoing geniuses, every day.
	//
	/*
	inline float		FastSqrt(float number)
	{
		return 1.0f/ISqrt(number);
	}

	inline float		ISqrt(float theNumber) 
	{
		const float aThreehalfs = 1.5F;
		float aY=theNumber;

		long aI=*(long*)&aY;

		aI=0x5f3759df-(aI>>1);
		aY=*(float*)&aI;
		aY=aY*(aThreehalfs-((theNumber*0.5F)*aY*aY));
		return aY;
	}


	inline double		FastSqrt(double number)
	{
		unsigned int i;
		double x2, y;
		const double threehalfs=1.5F;

		x2=number*0.5F;
		y=number;
		i=*(unsigned int*)&y;       
		i=0x5f3759df-(i>>1);  
		y=*(double*)&i;
		y=y*(threehalfs-(x2*y*y));
		//y=y*(threehalfs-(x2*y*y)); // Adds accuracy
		return (1/y);
	}
	 */

	//
	////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////
	//
	// Trigonometry stuff...
	//
	float				mMachineEpsilon;
	float				mTiny;	// Tiny number
	float				mPI;
	const float			mJigNormal=1.414214f; // Point(1,1).mNormal().Length()*mJiggerNormal == Point(1,1).Length();

	inline float		Sin(float theAngle) {return (float)sin(Rad(theAngle));}
	inline float		Cos(float theAngle) {return -(float)cos(Rad(theAngle));}
	inline float		ACos(float theAngle) {return -(float)acos(Rad(theAngle));}
	inline float		ASin(float theAngle) {return -(float)asin(Rad(theAngle));}
	inline float		Tan(float theAngle) {return (float)tan(Rad(theAngle));}
	inline float		Cot(float theAngle) {return (1.0f/(float)tan(Rad(theAngle)));}
	inline float		SinAbs(float theAngle) {return Abs((float)sin(Rad(theAngle)));}
	inline float		CosAbs(float theAngle) {return Abs(-(float)cos(Rad(theAngle)));}
	inline float		Sin(int theAngle) {return (float)sin(Rad(theAngle));}
	inline float		Cos(int theAngle) {return -(float)cos(Rad(theAngle));}
	inline float		ACos(int theAngle) {return -(float)acos(Rad(theAngle));}
	inline float		ASin(int theAngle) {return -(float)asin(Rad(theAngle));}
	inline float		Tan(int theAngle) {return (float)tan(Rad(theAngle));}
	inline float		Cot(int theAngle) {return (1.0f/(float)tan(Rad(theAngle)));}
	inline float		SinAbs(int theAngle) {return Abs((float)sin(Rad(theAngle)));}
	inline float		CosAbs(int theAngle) {return Abs(-(float)cos(Rad(theAngle)));}
	bool				SolveQuadratic(float theA, float theB, float theC, Point& theResult);
	//
	Point				Rotate2D(Point thePoint, Point theCenter, float theAngle);
	//
	// PointAtAngle2D gives you the point-at angle for two points (or for a simple vector).
	// This is set up to be compatible with the display-- i.e. you can pass an angle
	// result from this to a matrix and draw correctly (point-at assumes that the top of a
	// displayed sprite is the pointer)
	//
	//inline float		PointAtAngle2D(float theX, float theY) {return (float)Deg((float)atan2(theX,-theY))+360;}
	inline float		PointAtAngle2D(float theX, float theY) {float aResult=(float)Deg((float)atan2(theX,-theY));if (aResult<0) aResult+=360;return aResult;}
	inline float		PointAtAngle2D(Point thePoint) {return PointAtAngle2D(thePoint.mX,thePoint.mY);}
	inline float		PointAtAngle2D(float theX, float theY, float theCenterX, float theCenterY) {return PointAtAngle2D(theX-theCenterX,theY-theCenterY);}
	inline float		PointAtAngle2D(Point thePoint, Point theCenter) {return PointAtAngle2D(thePoint.mX,thePoint.mY,theCenter.mX,theCenter.mY);}
	inline float		VectorToAngle2D(Point theVector) {return PointAtAngle2D(theVector);}
	float				GetAngleBetweenVectors(Point theVec1, Point theVec2) {theVec1.Normalize();theVec2.Normalize();float aDot=theVec1.Dot(theVec2);aDot=(aDot<-1.0f?-1.0f:(aDot>1.0f?1.0f:aDot));return Deg(acosf(aDot));}
	float				GetAngleBetweenVectorsCW(Point theVec1, Point theVec2) {theVec1.Normalize();theVec2.Normalize();float aDot=theVec1.Dot(theVec2);float aDet=theVec1.Cross(theVec2);float aResult=Deg(atan2f(aDet,aDot));if (aResult<0) aResult+=360;return aResult;}

	//
	// Put this in for sorting angles... floating point imprecision can sometimes make angle zero be -.00000001 and thus trigger the +360 wrap.
	// This lets you set a wrap number.
	//
	inline float		VectorToAngleEX(float theX, float theY, float theWrapNumber) {float aResult=(float)Deg((float)atan2(theX,-theY));if (aResult<theWrapNumber) aResult+=360;return aResult;}
	inline float		VectorToAngleEX(Point theVec, float theWrapNumber) {return VectorToAngleEX(theVec.mX,theVec.mY,theWrapNumber);}

	Point				RotatePointAroundCenter(Point thePos, Point theCenter, float theDegrees) {Point aT=thePos-theCenter;float aAngle=atan2f(aT.mX,aT.mY);float aD=aT.Length();aAngle-=0.0174533f*theDegrees;return Point(theCenter.mX+aD*sinf(aAngle),theCenter.mY+aD*cosf(aAngle));}

	//
	////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////
	//
	// Quick check if a range intersects
	//
	bool				RangeIntersect(Point theRange1, Point theRange2);
	//
	////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////
	//
	// Distance
	//
	float				Distance(float theX1, float theY1, float theX2, float theY2);
	float				Distance(Point thePoint1, Point thePoint2) {return Distance(thePoint1.mX,thePoint1.mY,thePoint2.mX,thePoint2.mY);}
	float				Distance(float theX1, float theY1, float theZ1, float theX2, float theY2, float theZ2);
	inline double		Distance(double theX1, double theY1, double theZ1, double theX2, double theY2, double theZ2) {return sqrt(DistanceSquared(theX1,theY1,theZ1,theX2,theY2,theZ2));}
	inline float		Distance(float theX1, float theX2) {return Abs(theX1-theX2);}
	inline int			Distance(int theX1, int theX2) {return Abs(theX1-theX2);}
	inline float		Distance(Vector theP1, Vector theP2) {return Distance(theP1.mX,theP1.mY,theP1.mZ,theP2.mX,theP2.mY,theP2.mZ);}
	inline double		Distance(DVector theP1, DVector theP2) {return Distance(theP1.mX,theP1.mY,theP1.mZ,theP2.mX,theP2.mY,theP2.mZ);}
	//
	inline int			DistanceSquared(int theValue) {return (theValue*theValue);}
	inline float		DistanceSquared(float theValue) {return (theValue*theValue);}
	inline float		DistanceSquared(float theX1, float theY1, float theX2, float theY2) {float aD1=theX2-theX1;float aD2=theY2-theY1;return (float)(aD1*aD1+aD2*aD2);}
	inline double		DistanceSquared(double theX1, double theY1, double theX2, double theY2) {double aD1=theX2-theX1;double aD2=theY2-theY1;return (double)(aD1*aD1+aD2*aD2);}
	float				DistanceSquared(Point thePoint1, Point thePoint2) {return DistanceSquared(thePoint1.mX,thePoint1.mY,thePoint2.mX,thePoint2.mY);}
	inline float		DistanceSquared(float theX1, float theY1, float theZ1, float theX2, float theY2, float theZ2) {float aD1=theX2-theX1;float aD2=theY2-theY1;float aD3=theZ2-theZ1;return (float)((aD1*aD1)+(aD2*aD2)+(aD3*aD3));}
	float				DistanceSquared(Vector thePoint1, Vector thePoint2) {return DistanceSquared(thePoint1.mX,thePoint1.mY,thePoint1.mZ,thePoint2.mX,thePoint2.mY,thePoint2.mZ);}
	float				DistanceSquaredXY(Vector thePoint1, Vector thePoint2) {return DistanceSquared(thePoint1.mX,thePoint1.mY,thePoint2.mX,thePoint2.mY);}
	inline double		DistanceSquared(double theX1, double theY1, double theZ1, double theX2, double theY2, double theZ2) {double aD1=theX2-theX1;double aD2=theY2-theY1;double aD3=theZ2-theZ1;return (double)((aD1*aD1)+(aD2*aD2)+(aD3*aD3));}
	double				DistanceSquared(DVector thePoint1, DVector thePoint2) {return DistanceSquared(thePoint1.mX,thePoint1.mY,thePoint1.mZ,thePoint2.mX,thePoint2.mY,thePoint2.mZ);}
	double				DistanceSquaredXY(DVector thePoint1, DVector thePoint2) {return DistanceSquared(thePoint1.mX,thePoint1.mY,thePoint2.mX,thePoint2.mY);}

	inline double		DistanceSquaredD(float theX1, float theY1, float theX2, float theY2) {double aD1=theX2-theX1;double aD2=theY2-theY1;return (double)(aD1*aD1+aD2*aD2);}
	double				DistanceSquaredD(Point thePoint1, Point thePoint2) {return DistanceSquaredD(thePoint1.mX,thePoint1.mY,thePoint2.mX,thePoint2.mY);}
	inline double		DistanceSquaredD(float theX1, float theY1, float theZ1, float theX2, float theY2, float theZ2) {double aD1=theX2-theX1;double aD2=theY2-theY1;double aD3=theZ2-theZ1;return (double)((aD1*aD1)+(aD2*aD2)+(aD3*aD3));}
	double				DistanceSquaredD(Vector thePoint1, Vector thePoint2) {return DistanceSquaredD(thePoint1.mX,thePoint1.mY,thePoint1.mZ,thePoint2.mX,thePoint2.mY,thePoint2.mZ);}
	double				DistanceSquaredXYD(Vector thePoint1, Vector thePoint2) {return DistanceSquaredD(thePoint1.mX,thePoint1.mY,thePoint2.mX,thePoint2.mY);}

	//
	bool				IsCCW(Point& theP1, Point& theP2, Point& theP3);
	bool				IsCCW(Vector theP1, Vector theP2, Vector theP3, Vector viewFrom);
	bool				IsTriangleDegenerate(DVector theP1, DVector theP2, DVector theP3);
	bool				IsTriangleDegenerate(Vector theP1, Vector theP2, Vector theP3);
	bool				IsTriangleDegenerate(Point theP1, Point theP2, Point theP3);
	bool				IsTriangleLinear(Vector theP1, Vector theP2, Vector theP3, float thePrettyCloseThreshold);	// If two vertices are within the threshold
	bool				IsTriangleLinear(Point theP1, Point theP2, Point theP3, float thePrettyCloseThreshold);	// If two vertices are within the threshold

	inline bool			IsTriangleBad(Vector theP1, Vector theP2, Vector theP3, float thePrettyCloseThreshold) {bool aResult=IsTriangleDegenerate(theP1,theP2,theP3);if (!aResult) aResult=IsTriangleLinear(theP1,theP2,theP3,thePrettyCloseThreshold);return aResult;}

	//
	////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////
	//
	// Line stuff
	//
	inline Point		ClosestPointOnLine(Point& thePoint, Line& theLine) {return ClosestPointOnLine(thePoint,theLine.mPos[0],theLine.mPos[1]);}
	Point				ClosestPointOnLine(Point& thePoint, Point& theLine1, Point& theLine2);

	inline Point		ClosestPointOnInfiniteLine(Point& thePoint, Line theLine) {return ClosestPointOnInfiniteLine(thePoint,theLine.mPos[0],theLine.mPos[1]);}
	Point				ClosestPointOnInfiniteLine(Point& thePoint, Point& theLine1, Point& theLine2);

	inline Vector		ClosestPointOnInfiniteLine(Vector thePoint, Line3D theLine) {return ClosestPointOnInfiniteLine(thePoint,theLine.mPos[0],theLine.mPos[1]);}
	inline Vector		GetClosestPointOnInfiniteLine(Vector thePoint, Line3D theLine) {return ClosestPointOnInfiniteLine(thePoint,theLine.mPos[0],theLine.mPos[1]);}
	inline Vector		GetClosestPointOnInfiniteLine(Vector thePoint, Vector theLine1, Vector theLine2) {return ClosestPointOnInfiniteLine(thePoint,theLine1,theLine2);}
	Vector				ClosestPointOnInfiniteLine(Vector thePoint, Vector theLine1, Vector theLine2);
	Vector				ClosestPointToAxis(Vector thePos, Vector theAxisPos, Vector theAxisDir);	// TheCylinderDir should be normalized

	inline Point		RandomPointOnLine(Line& theLine1) {return RandomPointOnLine(theLine1.mPos[0],theLine1.mPos[1]);}
	Point				RandomPointOnLine(Point& theLine1, Point& theLine2);
	//bool				LineIntersectLine(Point theLine1Start, Point theLine1End, Point theLine2Start, Point theLine2End, Point *theIntersection=NULL);

	bool                DoesLineIntersectLine(Point theLine1Start, Point theLine1End, Point theLine2Start, Point theLine2End);
	inline bool			DoesLineIntersectLine(Line theLine1, Line theLine2) {return DoesLineIntersectLine(theLine1.mPos[0],theLine1.mPos[1],theLine2.mPos[0],theLine2.mPos[1]);}

	bool				DoesLineIntersectCircle(Point theLine1Start, Point theLine1End, Point theCircleCenter, float theCircleRadius)
	{
		Point aLocalP1=theLine1Start-theCircleCenter;
		Point aLocalP2=theLine1End-theCircleCenter;
		Point aP2MinusP1=aLocalP2-aLocalP1;

		float aA=(aP2MinusP1.mX)*(aP2MinusP1.mX)+(aP2MinusP1.mY)*(aP2MinusP1.mY);
		float aB=2*((aP2MinusP1.mX*aLocalP1.mX)+(aP2MinusP1.mY*aLocalP1.mY));
		float aC=(aLocalP1.mX*aLocalP1.mX)+(aLocalP1.mY*aLocalP1.mY)-(theCircleRadius*theCircleRadius);
		float aDelta=aB*aB-(4*aA*aC);
		if (aDelta<0) return false;
		else if (aDelta==0) return true;

		float aSquareRootDelta=(float)sqrt(aDelta);

		float aU1=(-aB+aSquareRootDelta)/(2*aA);
		float aU2=(-aB-aSquareRootDelta)/(2*aA);

		if (aU1>=0 && aU1<=1) return true;
		if (aU2>=0 && aU2<=1) return true;
		return false;
	}
	bool				DoesLineIntersectSphere(Vector theLine1Start, Vector theLine1End, Vector theCircleCenter, float theCircleRadius);


	bool                DoesLineIntersectQuad(Point theLine1Start, Point theLine1End, Quad theQuad);
	inline bool         DoesLineIntersectQuad(Line theLine, Quad theQuad) {return DoesLineIntersectQuad(theLine.mPos[0],theLine.mPos[1],theQuad);}

	bool                DoesLineIntersectTriangle(Point theLine1Start, Point theLine1End, Point aT1, Point aT2, Point aT3);
	inline bool         DoesLineIntersectTriangle(Point theLine1Start, Point theLine1End, Triangle theTriangle) {return DoesLineIntersectTriangle(theLine1Start,theLine1End,theTriangle.mPos[0],theTriangle.mPos[1],theTriangle.mPos[2]);}
	inline bool			DoesLineIntersectTriangle(Line theLine, Triangle theTriangle) {return DoesLineIntersectTriangle(theLine.mPos[0],theLine.mPos[1],theTriangle);}

	bool				GetLineIntersection(Point theLine1Start, Point theLine1End, Point theLine2Start, Point theLine2End, Point *theIntersection);
	float				GetLineIntersectionPercent(Point theLine1Start, Point theLine1End, Point theLine2Start, Point theLine2End);
	inline float		GetLineIntersectionPercent(Line theL1, Line theL2) {return GetLineIntersectionPercent(theL1.mPos[0],theL1.mPos[1],theL2.mPos[0],theL2.mPos[1]);}
	bool				GetLineIntersection(Vector theLine1Start, Vector theLine1End, Vector theLine2Start, Vector theLine2End, Vector *theIntersection);
	inline bool			GetLineIntersection(Line3D theLine1, Line3D theLine2, Vector *theIntersection) {return GetLineIntersection(theLine1.mPos[0],theLine1.mPos[1],theLine2.mPos[0],theLine2.mPos[1],theIntersection);}
	inline bool			GetLineIntersection(Line theLine1, Line theLine2, Point *theIntersection) {return GetLineIntersection(theLine1.mPos[0],theLine1.mPos[1],theLine2.mPos[0],theLine2.mPos[1],theIntersection);}
	bool				GetInfiniteLineIntersection(Point theLine1Start, Point theLine1End, Point theLine2Start, Point theLine2End, Point *theIntersection);
	inline bool			GetInfiniteLineIntersection(Line theLine1, Line theLine2, Point *theIntersection) {return GetInfiniteLineIntersection(theLine1.mPos[0],theLine1.mPos[1],theLine2.mPos[0],theLine2.mPos[1],theIntersection);}
	bool				GetInfiniteLineIntersection(Vector theLine1Start, Vector theLine1End, Vector theLine2Start, Vector theLine2End, Vector *theIntersection);
	inline bool			GetInfiniteLineIntersection(Line3D theLine1, Line3D theLine2, Vector *theIntersection) {return GetInfiniteLineIntersection(theLine1.mPos[0],theLine1.mPos[1],theLine2.mPos[0],theLine2.mPos[1],theIntersection);}

	bool				DoesLineIntersectRectangle(Rect theRect, Point theLine1, Point theLine2);
	inline bool			DoesLineIntersectRectangle(Rect theRect, Line theLine) {return DoesLineIntersectRectangle(theRect,theLine.mPos[0],theLine.mPos[1]);}

	bool				DoesLineIntersectAABB(AABBRect theRect, Point theLine1, Point theLine2);
	inline bool			DoesLineIntersectAABB(AABBRect theRect, Line theLine) {return DoesLineIntersectAABB(theRect,theLine.mPos[0],theLine.mPos[1]);}

	bool				DoesLineIntersectAABB(AABBCube theCube, Vector theLine1, Vector theLine2);
	inline bool			DoesLineIntersectAABB(AABBCube theCube, Line3D theLine) {return DoesLineIntersectAABB(theCube,theLine.mPos[0],theLine.mPos[1]);}
	bool				DoesRayIntersectAABB(AABBCube theCube, Vector theOrigin, Vector theDir);

	bool				DoesLineIntersectAABB(Cube theCube, Vector theLine1, Vector theLine2);
	inline bool			DoesLineIntersectAABB(Cube theCube, Line3D theLine) {return DoesLineIntersectAABB(theCube,theLine.mPos[0],theLine.mPos[1]);}
	bool				DoesRayIntersectAABB(Cube theCube, Vector theOrigin, Vector theDir);


	Point				LineIntersectX(Line theLine, float theX) {return Point(theX,theLine.mPos[0].mY+(theLine.mPos[1].mY-theLine.mPos[0].mY)*(theX-theLine.mPos[0].mX)/(theLine.mPos[1].mX-theLine.mPos[0].mX));}
	Point				LineIntersectY(Line theLine, float theY) {return Point(theLine.mPos[0].mX+(theLine.mPos[1].mX-theLine.mPos[0].mX)*(theY-theLine.mPos[0].mY)/(theLine.mPos[1].mY-theLine.mPos[0].mY),theY);}

	Line				ClipLineToRectangle(Line theLine, Rect theRect);
	Line3D				ClipLineToCube(Line3D theLine, Cube theCube);
	Line3D				ClipLineToCube(Line3D theLine, Cube theCube, Matrix rotateCube);

	bool				DoesLineIntersectPlane(Vector theLine1, Vector theLine2, Plane thePlane, Vector* theIntersectPos);
	bool				DoesLineIntersectPlane(DVector theLine1, DVector theLine2, Plane thePlane, DVector* theIntersectPos);
	inline bool			DoesLineIntersectPlane(Line3D theLine, Plane thePlane, Vector* theIntersectPos) {return DoesLineIntersectPlane(theLine.mPos[0],theLine.mPos[1],thePlane,theIntersectPos);}
	bool				DoesInfiniteLineIntersectPlane(Vector theLine1, Vector theLine2, Plane thePlane, Vector* theIntersectPos);
	inline bool			DoesInfiniteLineIntersectPlane(Line3D theLine, Plane thePlane, Vector* theIntersectPos) {return DoesInfiniteLineIntersectPlane(theLine.mPos[0],theLine.mPos[1],thePlane,theIntersectPos);}
	bool				DoesRayIntersectPlane(Vector theLine1, Vector theDir, Plane thePlane, Vector* theIntersectPos);


	//
	////////////////////////////////////////////////////////////////////


	//
	// Maps circle coordinates onto a square
	// From here: http://squircular.blogspot.com/2015/09/mapping-circle-to-square.html
	//
	Point 				MapCircleToSquare(Point theCircle);


	//
	// 	   Circle helpers... if you have a radius and want to know how many points to put around it to make it look "nice" you can use GetIdealCircleSides.
	// 	   If you want to know the angle of an arc from it's circumference length, GetAngleFromArcSize
	//
	inline float		GetIdealCircleSides(float theRadius, float theErrorFactor=.05f) {return mPI/acosf(1.0f-theErrorFactor/theRadius);}
	//inline float		GetBestCircleSides(float theRadius, float theErrorFactor=.05f) {return GetBestCircleSides(theRadius,theErrorFactor);}
	inline float		GetArcAngle(float theRadius, float theArcLen) {return (theArcLen/(2*mPI*theRadius))*360;}

	//
	// Seriously: John, do not try to implement this again (this is twice now).  It always comes up with the same answer.
	// There's always one specific angle on the circle that will dot this, so you get the same sides for radius=1 or radius=999999.
	// 
	//float				GetBestSmoothCircleSides(float theRadius, float startTestAngle=90, float theDot=.9f);
	//float				GetCircleStepByDot(float radius, float dotThreshold, float theMaxAngle=45.0f); // It makes zero sense to do this because they'll always be dot



	////////////////////////////////////////////////////////////////////
	//
	// Ellipse/Circle Intersections
	//
	bool				EllipseIntersect(Point theCenter1, Point theRadius1, Point theCenter2, Point theRadius2);
	bool				CircleIntersectCircle(Point theCenter1, float theRadius1, Point theCenter2, float theRadius2);
	inline bool			CircleIntersectRect(Point theCenter1, float theRadius1, Rect theRect) {return RectangleIntersectCircle(theRect,theCenter1,theRadius1);}
	bool				EllipseIntersectLine(Point theCenter, Point theRadius, Point theLine1, Point theLine2);
	inline bool			DoesEllipseIntersectLine(Point theCenter, Point theRadius, Point theLine1, Point theLine2) {return EllipseIntersectLine(theCenter,theRadius,theLine1,theLine2);}
	inline bool			DoesEllipseIntersectLine(Point theCenter, Point theRadius, Line theLine) {return EllipseIntersectLine(theCenter,theRadius,theLine.mPos[0],theLine.mPos[1]);}
	inline bool			EllipseIntersectPoint(Point theCenter, Point theRadius, Point thePoint)
	{
		float aDX=thePoint.mX-theCenter.mX;
		float aDY=thePoint.mY-theCenter.mY;

		return ((aDX*aDX)/(theRadius.mX*theRadius.mX)+(aDY*aDY)/(theRadius.mY*theRadius.mY))<=1;
	}

	inline bool			IsPointInEllipse(Point theCenter, Point theRadius, Point thePoint) {return EllipseIntersectPoint(theCenter,theRadius,thePoint);}

	//
	// Check if a circle is completely contained inside another circle
	// (Write this for the light reject testing in Solomon Dark)
	//
	inline bool			CircleIntersectPoint(Point theCenter, float theRadius, Point thePoint)
	{
		float aDX=thePoint.mX-theCenter.mX;
		float aDY=thePoint.mY-theCenter.mY;
		return ((aDX*aDX)/(theRadius*theRadius)+(aDY*aDY)/(theRadius*theRadius))<=1;
	}
	inline bool			IsPointInCircle(Point theCenter, float theRadius, Point thePoint) {return CircleIntersectPoint(theCenter,theRadius,thePoint);}
	bool				IsCircleInsideCircle(Point theCircle, float theRadius, Point theInsideCircle, float theInsideRadius);

	Vector				GetCircleCenter(Vector theP1, Vector theP2, Vector theP3);
	float				GetCircleRadius(Vector theP1, Vector theP2, Vector theP3);
	Vector				GetCircleNormal(Vector theP1, Vector theP2, Vector theP3);
	bool				GetCircleInfo(Vector theP1, Vector theP2, Vector theP3, Vector* theCenter, float* theRadius=NULL, Vector* theNormal=NULL);

	//
	// Check if a circle hits a line
	//
	inline bool			CircleIntersectLine(Point theCenter, float theRadius, Point theLine1, Point theLine2)
	{
		Point aClosest=ClosestPointOnLine(theCenter,theLine1,theLine2);
		Point aVec=aClosest-theCenter;
		if (aVec.LengthSquared()<=(theRadius*theRadius)) return true;
		return false;
	}
	bool GetCircleIntersectLine(Point theCenter, float theRadius, Point theLine1, Point theLine2, Point& theResult)
	{
		Point aVertex1to2=theLine2-theLine1;
		Point aCircleToVertex1=theCenter-theLine1;
		float aDot=aVertex1to2.Dot(aCircleToVertex1);
		Point aProj1=aVertex1to2*(aDot/aVertex1to2.LengthSquared());
		Point aMidpt=theLine1+aProj1;
		Point aCircleToMidpt=aMidpt-theCenter;
		float aDistSqToCenter=aCircleToMidpt.LengthSquared();
		if (aDistSqToCenter>theRadius*theRadius) return false;
		if (aDistSqToCenter==theRadius*theRadius)
		{
			theResult=aMidpt;
			return true;
		}
		float aDistToIntersection;
		if (aDistSqToCenter==0) aDistToIntersection=theRadius;
		else aDistToIntersection=(float)sqrt(theRadius*theRadius-aDistSqToCenter);

		aVertex1to2=aVertex1to2.Normalize();
		aVertex1to2*=aDistToIntersection;

		if (DistanceSquared(theLine1,aMidpt+aVertex1to2)<DistanceSquared(theLine1,aMidpt-aVertex1to2)) theResult=aMidpt+aVertex1to2;
		else theResult=aMidpt-aVertex1to2;
		return true;
	}
	bool				CircleIntersectTriangle(Point theCenter, float theRadius, Point theT1, Point theT2, Point theT3);
	bool				IsPointInCircumCircle(Point theT1, Point theT2, Point theT3, Point thePos);
	//bool				IsPointInCircumCircle(DPoint theT1, DPoint theT2, DPoint theT3, DPoint thePos);
	inline bool			IsPointInCircumCircle(Triangle& theT, Point thePos) {return IsPointInCircumCircle(theT.mPos[0],theT.mPos[1],theT.mPos[2],thePos);}
	Point				GetCircumCenter(Point theT1, Point theT2, Point theT3);
	inline Point		GetCircumCenter(Triangle& theT) {return GetCircumCenter(theT.mPos[0],theT.mPos[1],theT.mPos[2]);}
	//
	// Check if an Ellipse intersects a quad
	//
	bool				EllipseIntersectQuad(Point theCenter, Point theRadius, Quad& theQuad);

	bool				GetLineCircleIntersections(Point theCenter, float theRadius, Point theLine1, Point theLine2, Array<Point>& theResults);
	bool				GetLineEllipseIntersections(Point theCenter, Point theRadius, Line theLine, Array<Point>& theResults);
	bool				GetInfiniteLineCircleIntersections(Point theCenter, float theRadius, Point theLine1, Point theLine2, Array<Point>& theResults);
	Point				GetNextPointAroundEllipseCW(Point P, Point C, Point theRadius, float theDegrees);

	//
	////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////
	//
	// Rectangle Intersections
	//
	inline bool			RectangleIntersectCircle(Rect theRect, Point theCircleCenter, float theCircleRadius) {return ((theRect.ClampPoint(theCircleCenter)-theCircleCenter).LengthSquared()<=(theCircleRadius*theCircleRadius));}
	bool				RectangleIntersectQuad(Rect theRect, Quad& theQuad);
	Point				RandomPointInRect(Rect theRect, Random& theRandom=gRand);
	//
	////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////
	//
	// Randomness
	//
#ifndef NORANDOM	
	Point				GetRandomPoint(float theRadius);
#endif	
	//
	////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////
	//
	// Some quickie operations to get information about values
	//
	inline int			Sign(int theValue) {if (theValue==0) return 0;return (theValue<0)?-1:1;}
	inline float		Sign(float theValue) {if (theValue==0) return 0;return (theValue<0.0f)?-1.0f:1.0f;}
	inline int			Clamp(int theValue, int theMin=0, int theMax=1) {return (theValue<theMin)?theMin:(theValue>theMax)?theMax:theValue;}
	inline float		Clamp(float theValue, float theMin=0.0f, float theMax=1.0f) {return (theValue<theMin)?theMin:(theValue>theMax)?theMax:theValue;}
	int					Wrap(int theValue, int theMin, int theMax);
	float				Wrap(float theValue, float theMin, float theMax);
	//
	////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////
	//
	// Some simple collision primitives
	//
	bool				CollideCircleCircle(Point theMovingCenter, float theMovingRadius, Point theMovementVector, Point theStationaryCenter, float theStationaryRadius, Point *theResultVector=NULL);
	//
	////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////
	//
	// Some higher-order trig-- you're pointed at angle x.  You want
	// to angle y... this will give you 1 or -1, telling you which direction
	// to rotate is shorter.
	//
	float				NormalizeAngle(float theAngle);
	float				GetBestRotation(float theSourceAngle, float theDestAngle); 
	float				GetDistanceBetweenAngles(float theAngle1, float theAngle2);
	float				GetDifferenceBetweenAngles(float theAngle1, float theAngle2);
	//
	////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////
	//
	// Quickie function to see if numbers are across a threshold
	//
	bool				CrossTheshold(float theFirstNumber, float theSecondNumber, float theThreshold);
	inline bool			CrossTheshold(int theFirstNumber, int theSecondNumber, int theThreshold) {return CrossTheshold((float)theFirstNumber,(float)theSecondNumber,(float)theThreshold);}
	//
	////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////
	//
	// EZ access to a directional list of numbers, for vector of
	// the 8 cardinal directions
	//
	IPoint				mCardinalDirList[9];
	IPoint				mCardinal8DirList[8];
	IPoint				mCardinal4DirList[4];
	IVector				mCardinalDirList3D[9*3];
	IVector				mCardinal4DirList3D[6];
	IPoint				mCardinal4DirListCW[4];

	inline IPoint		GetCardinalDirection(int theNumber) {return mCardinalDirList[theNumber];}
	inline Point		GetCardinalDirectionFloat(int theNumber) {return Point(mCardinalDirList[theNumber].mX,mCardinalDirList[theNumber].mY);}
	inline IPoint		GetCardinal4Direction(int theNumber) {return mCardinal4DirList[theNumber];}
	inline Point		GetCardinal4DirectionFloat(int theNumber) {return Point(mCardinal4DirList[theNumber].mX,mCardinal4DirList[theNumber].mY);}
	inline IPoint		GetCardinal4DirectionCW(int theNumber) {return mCardinal4DirListCW[theNumber];}
	inline Point		GetCardinal4DirectionFloatCW(int theNumber) {return Point(mCardinal4DirListCW[theNumber].mX,mCardinal4DirListCW[theNumber].mY);}
	inline IPoint		GetCardinal8Direction(int theNumber) {return mCardinal8DirList[theNumber];}
	inline Point		GetCardinal8DirectionFloat(int theNumber) {return Point(mCardinal8DirList[theNumber].mX,mCardinal8DirList[theNumber].mY);}
	inline IPoint		GetCardinal9Direction(int theNumber) {return GetCardinalDirection(theNumber);}
	inline Point		GetCardinal9DirectionFloat(int theNumber) {return GetCardinalDirectionFloat(theNumber);}


	inline IVector		GetCardinalDirection3D(int theNumber) {return mCardinalDirList3D[theNumber];}
	inline Vector		GetCardinalDirection3DFloat(int theNumber) {return Vector(mCardinalDirList3D[theNumber].mX,mCardinalDirList3D[theNumber].mY,mCardinalDirList3D[theNumber].mZ);}
	inline IVector		GetCardinal4Direction3D(int theNumber) {return mCardinal4DirList3D[theNumber];}
	inline Vector		GetCardinal4Direction3DFloat(int theNumber) {return Vector(mCardinal4DirList3D[theNumber].mX,mCardinal4DirList3D[theNumber].mY,mCardinalDirList3D[theNumber].mZ);}


	//
	////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////
	//
	// Cones (all cones assumed infinite)
	//
	bool				IsPointInCone(Point theConeCenter, Point theConeVector, float coneAngleHalfWidth, Point theTestPoint);
	bool				IsPointInSector(Point theCenter, float theStartAngle, float theEndAngle, Point theTestPoint);
	//
	////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////
	//
	// Quads
	//
	bool				IsPointInQuad(Point thePoint, Quad& theQuad);
	//
	////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////
	//
	// Triangles
	//
	bool				IsPointInTriangle(Point thePoint, Point theTriange1, Point theTriangle2, Point theTriangle3);
	bool				IsPointInTriangle(Point thePoint, Vector* theTriange1, Vector* theTriangle2, Vector* theTriangle3);
	bool				IsPointInTriangle(Point* thePoint, Point* theTriange1, Point* theTriangle2, Point* theTriangle3);
	inline bool			IsPointInTriangle(Point thePoint, Point* theT) {return IsPointInTriangle(thePoint,theT[0],theT[1],theT[2]);}
	inline bool			IsPointInTriangle(Point thePoint, Array<Point> theT) {return IsPointInTriangle(thePoint,theT[0],theT[1],theT[2]);}

	bool				IsPointInTrianglePrecise(Point thePoint, Point theTriange1, Point theTriangle2, Point theTriangle3, float theEpsilon=.0001f);
	inline bool			IsPointInTrianglePrecise(Point thePoint, Point* theT, float theEpsilon=.0001f) {return IsPointInTrianglePrecise(thePoint,theT[0],theT[1],theT[2],theEpsilon);}
	inline bool			IsPointInTrianglePrecise(Point thePoint, Array<Point> theT, float theEpsilon=.0001f) {return IsPointInTrianglePrecise(thePoint,theT[0],theT[1],theT[2],theEpsilon);}


	inline bool			IsPointInTriangle(Point thePoint, Triangle theTriangle) {return IsPointInTriangle(thePoint,theTriangle.mPos[0],theTriangle.mPos[1],theTriangle.mPos[2]);}
	inline bool			IsPointInTriangle(Point thePoint, Triangle* theTriangle) {return IsPointInTriangle(thePoint,theTriangle->mPos[0],theTriangle->mPos[1],theTriangle->mPos[2]);}
	Vector				GetFaceNormal(Vector v1, Vector v2, Vector v3);
	inline Vector		GetTriangleNormal(Vector v1, Vector v2, Vector v3) {return GetFaceNormal(v1,v2,v3);}
	Vector				GetFaceNormalUnNormalized(Vector v1, Vector v2, Vector v3);
	inline Vector		GetTriangleNormalUnNormalized(Vector v1, Vector v2, Vector v3) {return GetFaceNormalUnNormalized(v1,v2,v3);}
	float				GetTriangleArea(Vector v1, Vector v2, Vector v3);
	float				GetTriangleArea(Point v1, Point v2, Point v3);
	Vector				InterpolateTriangle(Vector thePoint, Vector theV1, Vector theV2, Vector theV3, Vector theInterpolate1,Vector theInterpolate2,Vector theInterpolate3);

	inline Vector		InterpolateTriangle(Vector theInterpolations, Vector theV1, Vector theV2, Vector theV3) {return (theV1*theInterpolations.mX)+(theV2*theInterpolations.mY)+(theV3*theInterpolations.mZ);}
	inline Point		InterpolateTriangle(Vector theInterpolations, Point theV1, Point theV2, Point theV3) {return (theV1*theInterpolations.mX)+(theV2*theInterpolations.mY)+(theV3*theInterpolations.mZ);}
	inline float		InterpolateTriangle(Vector theInterpolations, float theV1, float theV2, float theV3) {return (theV1*theInterpolations.mX)+(theV2*theInterpolations.mY)+(theV3*theInterpolations.mZ);}
	inline int			InterpolateTriangle(Vector theInterpolations, int theV1, int theV2, int theV3) {return (int)(((float)theV1*theInterpolations.mX)+((float)theV2*theInterpolations.mY)+((float)theV3*theInterpolations.mZ));}
	inline short		InterpolateTriangle(Vector theInterpolations, short theV1, short theV2, short theV3) {return (short)(((float)theV1*theInterpolations.mX)+((float)theV2*theInterpolations.mY)+((float)theV3*theInterpolations.mZ));}

	inline int			InterpolateTriangleFlags(Vector theInterpolations, int theV1, int theV2, int theV3)
	{
		int *aI1,*aI2;
		if (theInterpolations.mX>=theInterpolations.mZ && theInterpolations.mY>=theInterpolations.mZ) {aI1=&theV1;aI2=&theV2;}
		if (theInterpolations.mY>=theInterpolations.mX && theInterpolations.mZ>=theInterpolations.mX) {aI1=&theV2;aI2=&theV3;}
		if (theInterpolations.mX>=theInterpolations.mY && theInterpolations.mZ>=theInterpolations.mY) {aI1=&theV3;aI2=&theV1;}
		return (*aI1)&(*aI2);
	}

	inline Point		InterpolateTriangle(Point thePoint, Point theV1, Point theV2, Point theV3, Point theInterpolate1,Point theInterpolate2,Point theInterpolate3) {Vector aV=InterpolateTriangle(Vector(thePoint),Vector(theV1),Vector(theV2),Vector(theV3),Vector(theInterpolate1),Vector(theInterpolate2),Vector(theInterpolate3));return Point(aV.mX,aV.mY);}

	inline Vector		GetNormal(Vector theV) {return theV.Normal();}
	inline Point		GetNormal(Point theV) {return theV.Normal();}



	//
	////////////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////////////
	//
	// Interpolation
	//
	inline float Interpolate(float a,float b,float x) {float ft=x*3.1415927f;float f=(1.0f-(float)cos(ft))* 0.5f;return a*(1.0f-f)+b*f;}
	inline Point Interpolate(Point a,Point b,float x) {float ft=x*3.1415927f;float f=(1.0f-(float)cos(ft))* 0.5f;return a*(1.0f-f)+b*f;}
	inline Vector Interpolate(Vector a,Vector b,float x) {float ft=x*3.1415927f;float f=(1.0f-(float)cos(ft))* 0.5f;return a*(1.0f-f)+b*f;}
	inline float InterpolateLinear(float a, float b, float x) {return(a*(1-x)+b*x);}
	inline Point InterpolateLinear(Point a, Point b, float x) {return(a*(1-x)+b*x);}
	inline Point Lerp(Point a, Point b, float x) {return(a*(1-x)+b*x);}
	inline Vector InterpolateLinear(Vector a, Vector b, float x) {return(a*(1-x)+b*x);}
	inline Vector Lerp(Vector a, Vector b, float x) {return(a*(1-x)+b*x);}
	inline int Lerp(int a, int b, float x) {return (int)(a+((float)(b-a)*x));}
	inline longlong Lerp(longlong a, longlong b, float x) {return (longlong)(a+((float)(b-a)*x));}
	inline short Lerp(short a, short b, float x) {return (short)(a+((float)(b-a)*x));}
	inline float Lerp(float a, float b, float x) {return (a+((b-a)*x));}
	inline float Lerp2(float a, float b, float x) {return a + x * (b - a);}

	inline float InterpolateSquare(float theTopLeft, float theTopRight, float theBottomLeft, float theBottomRight, float xPercent, float yPercent)
	{
		xPercent=_clamp(0,xPercent,1);
		yPercent=_clamp(0,yPercent,1);
		float aLeft = theTopLeft*(1-yPercent)+theBottomLeft*yPercent;
		float aRight = theTopRight*(1-yPercent)+theBottomRight*yPercent;
		return aLeft*(1-xPercent)+aRight*(xPercent);
	}
	inline Point InterpolateSquare(Point theTopLeft, Point theTopRight, Point theBottomLeft, Point theBottomRight, float xPercent, float yPercent)
	{
		xPercent=_clamp(0,xPercent,1);
		yPercent=_clamp(0,yPercent,1);
		Point aLeft = theTopLeft*(1-yPercent)+theBottomLeft*yPercent;
		Point aRight = theTopRight*(1-yPercent)+theBottomRight*yPercent;
		return aLeft*(1-xPercent)+aRight*(xPercent);
	}
	inline Point InterpolateSquare(Quad theQuad, float xPercent, float yPercent) {return InterpolateSquare(theQuad.mCorner[0],theQuad.mCorner[1],theQuad.mCorner[2],theQuad.mCorner[3],xPercent,yPercent);}
	inline float InterpolateCube(float theTop1, float theTop2, float theTop3, float theTop4,float theBottom1, float theBottom2, float theBottom3, float theBottom4, float xPercent, float yPercent, float zPercent)
	{
		xPercent=_clamp(0,xPercent,1);
		yPercent=_clamp(0,yPercent,1);
		zPercent=_clamp(0,zPercent,1);
		float aTop=InterpolateSquare(theTop1,theTop2,theTop3,theTop4,xPercent,yPercent);
		float aBottom=InterpolateSquare(theBottom1,theBottom2,theBottom3,theBottom4,xPercent,yPercent);
		return InterpolateLinear(aTop,aBottom,zPercent);
	}

	//
	// Tells you where you are between two numbers... returns 0-1 (if between)
	//
	inline float GetPercent(float theLow, float theNumber, float theHigh) {return ((theNumber-theLow)/(theHigh-theLow));}
	inline float ReverseInterpolate(float theLow, float theNumber, float theHigh) {return GetPercent(theLow,theNumber,theHigh);} 
	inline float SmoothStep(float theLow, float theNumber, float theHigh) {return GetPercent(theLow,theNumber,theHigh);}

	//
	// Easings...
	//
	float		 Ease(char theEasing, float progress);


	//
	////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////
	//
	// Noise Functions
	//
	inline float Noise(float x) {int aX = ((int)x<<13)^(int)x;return (float)(1.0f-((aX*(aX*aX*15731+789221)+1376312589)&0x7fffffff)/1073741824.0f);}
	inline float Noise2D(float x,float y) {int n=(int)x+(int)y*57;n=(n<<13)^n;int nn=(n*(n*n*60493+19990303)+1376312589)&0x7fffffff;return 1.0f-((float)nn/1073741824.0f);}
	float PerlinNoise(float x, float y);
	//
	////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////
	//
	// Loses precision on a number... I use this for stuttery effects
	//
	inline float	LosePrecision(float theNumber, float thePrecisionLoss) {return (float)((int)(theNumber/thePrecisionLoss))*thePrecisionLoss;}
	//
	////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////
	//
	// Some redundancies with vectors, to give us clarity when
	// pasting code from other places, basically.
	//
	Vector			Cross(Vector theV1, Vector theV2);
	float			Dot(Vector theV1, Vector theV2);
	//
	////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////////////
	//
	// Bunch of 3D Helpers....
	//
	Vector			GetClosestPointOnPlane(Vector thePos,Plane& thePlane) {float aD=thePlane.SignedDistanceTo(thePos);return thePos-thePlane.mNormal*aD;}
	char			ClassifyPoint(Vector thePoint, Vector thePlanePos, Vector thePlaneNormal);
	inline char		ClassifyPoint(Vector thePoint, Plane& thePlane) {return ClassifyPoint(thePoint,thePlane.mPos,thePlane.mNormal);}
	Vector			GetTangentPlaneNormalOfEllipsoid(Vector thePoint, Vector theEllipseCenter, Vector theEllipseRadius);
	bool			IsPointInSphere(Vector thePoint, Vector theSphereCenter, float theRadius);
	Point			GetClosestPointOnTriangle(Point thePos, Point theT1, Point theT2, Point theT3);
	Vector			GetClosestPointOnTriangle(Vector thePos, Vector theT1, Vector theT2, Vector theT3);
	inline Vector	GetClosestPointOnTriangle(Vector thePos, Triangle3D& theTriangle) {return GetClosestPointOnTriangle(thePos,theTriangle.mPos[0],theTriangle.mPos[1],theTriangle.mPos[2]);}
	inline Vector	GetClosestPointOnTriangle(Vector thePos, Triangle3DPtr& theTriangle) {return GetClosestPointOnTriangle(thePos,*theTriangle.mPos[0],*theTriangle.mPos[1],*theTriangle.mPos[2]);}
	Vector			GetClosestPointOnLine(Vector thePos, Vector theL1, Vector theL2);
	DVector			GetClosestPointOnLine(DVector thePos, DVector theL1, DVector theL2);
	inline Vector	GetClosestPointOnLine(Vector thePos, Line3D theLine) {return GetClosestPointOnLine(thePos,theLine.mPos[0],theLine.mPos[1]);}

	//Vector			GetClosestPointOnLine2(Vector thePos, Vector theL1, Vector theL2);
	//inline Vector	GetClosestPointOnLine2(Vector thePos, Line3D& theLine) {return GetClosestPointOnLine2(thePos,theLine.mPos[0],theLine.mPos[1]);}


	float			GetClosestPointOnLinePercent(Vector thePos, Vector theL1, Vector theL2);
	float			GetClosestPointOnInfiniteLinePercent(Vector thePos, Vector theL1, Vector theL2);
	double			GetClosestPointOnInfiniteLinePercent(DVector thePos, DVector theL1, DVector theL2);
	double			GetClosestPointOnInfiniteLinePercentD(Vector thePos, Vector theL1, Vector theL2);
	float			GetClosestPointOnLinePercent(Point thePos, Point theL1, Point theL2);

	bool			IsPointInTriangle(Vector thePos, Vector theT1, Vector theT2, Vector theT3);
	bool			IsPointInTriangle(DVector thePos, DVector theT1, DVector theT2, DVector theT3);
	inline bool		IsPointInTriangle(Vector thePos, Triangle3D& theTriangle) {return IsPointInTriangle(thePos,theTriangle.mPos[0],theTriangle.mPos[1],theTriangle.mPos[2]);}
	float			IntersectRaySphere(Vector theRayOrigin, Vector theRayDir, Vector theSphereCenter, float theRadius);
	float			IntersectRayPlane(Vector theRayOrigin, Vector theRayDir, Vector thePlanePos, Vector thePlaneNormal);
	bool			IntersectRayPlane(Vector theRayOrigin, Vector theRayDir, Vector thePlanePos, Vector thePlaneNormal, float &result);
	inline Vector	Wedge(Vector theV1, Vector theV2) {Vector aResult;aResult.mX=(theV1.mY * theV2.mZ) - (theV2.mY * theV1.mZ);aResult.mY=(theV1.mZ * theV2.mX) - (theV2.mZ * theV1.mX);aResult.mZ=(theV1.mX * theV2.mY) - (theV2.mX * theV1.mY);return (aResult);}
	Point			GetClosestPointOnLine(Point thePos, Point theL1, Point theL2);
	inline Point	GetClosestPointOnLine(Point thePos, Line& theLine) {return GetClosestPointOnLine(thePos,theLine.mPos[0],theLine.mPos[1]);}

	float			DistanceToLine(Point thePos, Point theL1, Point theL2);
	inline float	DistanceToLine(Point thePos, Line& theLine) {return DistanceToLine(thePos,theLine.mPos[0],theLine.mPos[1]);}
	float			DistanceToLineSquared(Point thePos, Point theL1, Point theL2);
	inline float	DistanceToLineSquared(Point thePos, Line& theLine) {return DistanceToLineSquared(thePos,theLine.mPos[0],theLine.mPos[1]);}

	inline bool		AreTrianglesJoined(Triangle3D* aT1, Triangle3D* aT2)
	{
		int aCount=0;
		if (DistanceSquared(aT1->mPos[0],aT2->mPos[0])<=mMachineEpsilon || DistanceSquared(aT1->mPos[0],aT2->mPos[1])<=mMachineEpsilon || DistanceSquared(aT1->mPos[0],aT2->mPos[2])<=mMachineEpsilon) aCount++;
		if (DistanceSquared(aT1->mPos[1],aT2->mPos[0])<=mMachineEpsilon || DistanceSquared(aT1->mPos[1],aT2->mPos[1])<=mMachineEpsilon || DistanceSquared(aT1->mPos[1],aT2->mPos[2])<=mMachineEpsilon) aCount++;
		if (DistanceSquared(aT1->mPos[2],aT2->mPos[0])<=mMachineEpsilon || DistanceSquared(aT1->mPos[2],aT2->mPos[1])<=mMachineEpsilon || DistanceSquared(aT1->mPos[2],aT2->mPos[2])<=mMachineEpsilon) aCount++;
		return (aCount>=2);
	}

	//
	////////////////////////////////////////////////////////////////////

	//
	// Solves quadratic equations...
	//
	bool			GetLowestRoot(float theA, float theB, float theC, float maxR, float* theRoot);

	//
	// Rounding (rountTo = nearest value to round to)
	//
	inline int		Round(int theValue, int roundTo) {return (int)((theValue+(roundTo/2.0f))/roundTo)*roundTo;}

	//
	// Normal rounding of a floating point number
	//
	inline float	Round(float theValue)
	{
		if (theValue-floor(theValue)>=.5f) return (float)ceil(theValue);
		return (float)floor(theValue);
	}

	//
	// Simple eval of a string expression (must be all numbers, any variables, etc, have to be evaluated beforehand)
	//
	float			Eval(String theEval);

	//
	// Helpers
	//
	inline Point	MaxPoint() {return Point(FLT_MAX,FLT_MAX);}
	inline Vector	MaxVector() {return Vector(FLT_MAX,FLT_MAX,FLT_MAX);}
	inline Point	BiggestPoint() {return MaxPoint();}
	inline Vector	VectorPoint() {return MaxVector();}


};

class Math3D
{
public:
	Math3D(void);

	void				Go();

	float				Distance(Vector theVector1, Vector theVector2) {return (float)sqrt(DistanceSquared(theVector1,theVector2));}
	float				DistanceSquared(Vector theVector1, Vector theVector2);
	inline float		Distance(Vector theVector, Plane& thePlane) {return Math::Abs((float)thePlane.SignedDistanceTo(theVector));}

	inline float		DistanceToPlane(Vector theVector, Plane& thePlane) {return Distance(theVector,thePlane);}

	void				MakeSphere(float thePoints, float theRadius, Array<Vector>& theArray);

	bool				DoesLineIntersectCube(Vector theLine1, Vector theLine2, Vector theBox1, Vector theBox2, Vector* theIntersection);
	inline bool			DoesLineIntersectCube(Line3D theLine, Cube theCube, Vector* theIntersection) {return DoesLineIntersectCube(theLine.mPos[0],theLine.mPos[1],theCube.Corner(0),theCube.Corner(7),theIntersection);}
	inline bool			DoesLineIntersectCube(Line3D theLine, Cube theCube)  {if (theCube.ContainsPoint(theLine.mPos[0]) || theCube.ContainsPoint(theLine.mPos[1])) return true;return DoesLineIntersectCube(theLine.mPos[0],theLine.mPos[1],theCube.Corner(0),theCube.Corner(7),NULL);}

	bool				DoesLineIntersectTriangle(Vector theLine1, Vector theLine2, Vector theT1, Vector theT2, Vector theT3, Vector* theIntersection=NULL);
	bool				DoesLineIntersectTriangle(DVector theLine1, DVector theLine2, DVector theT1, DVector theT2, DVector theT3, DVector* theIntersection=NULL);
	inline bool			DoesLineIntersectTriangle(Line3D theLine, Triangle3D theTri, Vector* theIntersection=NULL) {return DoesLineIntersectTriangle(theLine.mPos[0],theLine.mPos[1],theTri.mPos[0],theTri.mPos[1],theTri.mPos[2],theIntersection);}

	bool				DoesSphereIntersectRoundedCone(Vector spherePos, float sphereRadius, Vector coneStart, float coneStartRadius, Vector coneEnd, float coneEndRadius);
	float				DistanceToRoundedCone(Vector thePos, Vector coneStart, float coneStartRadius, Vector coneEnd, float coneEndRadius);
	//float				DistanceToRoundedConeSquared(Vector thePos, Vector coneStart, float coneStartRadius, Vector coneEnd, float coneEndRadius);
	float				DistanceToCubeSquared(Vector thePos, Cube theCube);
	inline float		DistanceToCube(Vector thePos, Cube theCube) {return (float)sqrt(DistanceToCubeSquared(thePos,theCube));}
	inline bool			DoesSphereIntersectCube(Vector spherePos, float sphereRadius, Cube theCube)	{return (DistanceToCubeSquared(spherePos,theCube)<=(sphereRadius*sphereRadius));}

	Point				TriangleXYZToUV(Vector thePos, Vertex2D* theTriangle);	// Note: Results are not guaranteed if thePos isn't within the triangle!
	Point				TriangleXYZToUV(Vector thePos, Vector theTPos1, Vector theTPos2, Vector theTPos3, Point theUVPos1, Point theUVPos2, Point theUVPos3);	// Note: Results are not guaranteed if thePos isn't within the triangle!
	Vector				TriangleUVToXYZ(Point theUV, Vertex2D* theTriangle);	// Note: Results are not guaranteed if theUV isn't within the triangle!  ...doClamp will clamp it tho.
	Vector				TriangleUVToXYZ(Point theUV, Vector* theTPos, Point *theTUV);	// Note: Results are not guaranteed if theUV isn't within the triangle!  ...doClamp will clamp it tho.
	inline Vector		TriangleUVToXYZ(Point theUV, Array<Vector>& theTPos, Array<Point> theTUV) {return TriangleUVToXYZ(theUV,theTPos.mArray,theTUV.mArray);}


	Vector				GetTriangleInterpolations(Vector thePos, Vector theT1, Vector theT2, Vector theT3);
	inline Vector		GetTriangleInterpolations(Vector thePos, Triangle3D& theT) {return GetTriangleInterpolations(thePos,theT.mPos[0],theT.mPos[1],theT.mPos[2]);}
	inline Vector		GetBaryCentric(Vector thePos, Triangle3D& theT) {return GetTriangleInterpolations(thePos,theT.mPos[0],theT.mPos[1],theT.mPos[2]);}
	inline Vector		GetBaryCentric(Vector thePos, Triangle3DPtr& theT) {return GetTriangleInterpolations(thePos,*theT.mPos[0],*theT.mPos[1],*theT.mPos[2]);}
	inline Vector		GetBaryCentric(Vector thePos, Vector theT1, Vector theT2, Vector theT3) {return GetTriangleInterpolations(thePos,theT1,theT2,theT3);}

	inline Vector		BaryCentricBlend(Vector theBlend, Vector theValue1, Vector theValue2, Vector theValue3) {return (theValue1*theBlend.mX)+(theValue2*theBlend.mY)+(theValue3*theBlend.mZ);}
	inline Vector		BaryCentricBlend(Vector thePos, Vector theT1, Vector theT2, Vector theT3, Vector theBlend1, Vector theBlend2, Vector theBlend3) {Vector aBlend=GetBaryCentric(thePos,theT1,theT2,theT3);return BaryCentricBlend(aBlend,theBlend1, theBlend2, theBlend3);}

	inline Vector		LerpT(Vector theBlend, Vector theValue1, Vector theValue2, Vector theValue3) {return (theValue1*theBlend.mX)+(theValue2*theBlend.mY)+(theValue3*theBlend.mZ);}
	inline Point		LerpT(Vector theBlend, Point theValue1, Point theValue2, Point theValue3) {return (theValue1*theBlend.mX)+(theValue2*theBlend.mY)+(theValue3*theBlend.mZ);}
	inline float		LerpT(Vector theBlend, float theValue1, float theValue2, float theValue3) {return (theValue1*theBlend.mX)+(theValue2*theBlend.mY)+(theValue3*theBlend.mZ);}
	inline Color		LerpT(Vector theBlend, Color theC0, Color theC1, Color theC2) {Color aResult;aResult.mR=(theC0.mR*theBlend.mX)+(theC1.mR*theBlend.mY)+(theC2.mR*theBlend.mZ);aResult.mG=(theC0.mG*theBlend.mX)+(theC1.mG*theBlend.mY)+(theC2.mG*theBlend.mZ);aResult.mB=(theC0.mB*theBlend.mX)+(theC1.mB*theBlend.mY)+(theC2.mB*theBlend.mZ);aResult.mA=(theC0.mA*theBlend.mX)+(theC1.mA*theBlend.mY)+(theC2.mA*theBlend.mZ);return aResult;}
	inline int			LerpT(Vector theBlend, int theValue1, int theValue2, int theValue3) {return (int)(((float)theValue1*theBlend.mX)+((float)theValue2*theBlend.mY)+((float)theValue3*theBlend.mZ));}
	inline longlong		LerpT(Vector theBlend, longlong theValue1, longlong theValue2, longlong theValue3) {return (longlong)(((double)theValue1*theBlend.mX)+((double)theValue2*theBlend.mY)+((double)theValue3*theBlend.mZ));}


	inline Vector		GetPointOnTriangle(Vector theT1, Vector theT2, Vector theT3, float factor1,float factor2) {return ((1-sqrtf(factor1))*theT1+(sqrtf(factor1)*(1-factor2))*theT2+(sqrtf(factor1)*factor2)*theT3);}
						// factor1/factor2 have to be numbers between 0-1 ... good for random points on triangle.


	Vector				ClosestPointOnTriangle(Vector sourcePos, Vector theT0, Vector theT1, Vector theT2);
	DVector				ClosestPointOnTriangle(DVector sourcePos, DVector theT0, DVector theT1, DVector theT2);
	inline Vector		ClosestPointOnTriangle(Vector sourcePos, Triangle3D theT) {return ClosestPointOnTriangle(sourcePos,theT.mPos[0],theT.mPos[1],theT.mPos[2]);}
	inline Vector		ClosestPointOnTriangle(Vector thePos, Triangle3DPtr& theTriangle) {return ClosestPointOnTriangle(thePos,*theTriangle.mPos[0],*theTriangle.mPos[1],*theTriangle.mPos[2]);}

	bool				DoesTriangleIntersectTriangle(Vector T1A,Vector T1B,Vector T1C,Vector T2A,Vector T2B,Vector T2C, float theFudgeFactor=0);
	bool				DoesTriangleIntersectCube(Vector theV1,Vector theV2,Vector theV3, Cube theCube) {return DoesTriangleIntersectCubeQD(theV1,theV2,theV3,theCube);}
	inline bool			DoesTriangleIntersectCube(Triangle3D theT, Cube theCube) {return DoesTriangleIntersectCube(theT.mPos[0],theT.mPos[1],theT.mPos[2],theCube);}
	// QD = "Quick n' Dirty" ... for when we don't need precise...
	bool				DoesTriangleIntersectCubeQD(Vector theV1,Vector theV2,Vector theV3, Cube theCube);
	inline bool			DoesTriangleIntersectCubeQD(Triangle3D theT, Cube theCube) {return DoesTriangleIntersectCube(theT.mPos[0],theT.mPos[1],theT.mPos[2],theCube);}

	bool				IsPointInTriangleVolume(Vector thePos, Vector theT0, Vector theT1, Vector theT2);	// If the point is within the triangle's open space along the triangle's axis
	bool				IsPointInTriangleVolume(DVector thePos, DVector theT0, DVector theT1, DVector theT2);	// If the point is within the triangle's open space along the triangle's axis
	inline bool			IsPointInTriangleVolume(Vector thePos, Triangle3D theTriangle) {return IsPointInTriangleVolume(thePos, theTriangle.mPos[0],theTriangle.mPos[1],theTriangle.mPos[2]);}
	bool				GetLineSphereIntersections(Vector theCenter, float theRadius, Vector theLine1, Vector theLine2, Array<Vector>& theResults);
	bool				GetLineCylinderIntersections(Vector theOrigin, Vector theAxis, float theRadius, Vector theLine1, Vector theLine2, Array<Vector>& theResults);
	bool				GetLineCubeIntersections(Cube theCube, Vector theLine1, Vector theLine2, Array<Vector>& theResults);

	float				DistanceToLine(Vector thePos, Vector theL1, Vector theL2);
	inline float		DistanceToLine(Vector thePos, Line3D& theLine) {return DistanceToLine(thePos,theLine.mPos[0],theLine.mPos[1]);}
	float				DistanceToLineSquared(Vector thePos, Vector theL1, Vector theL2);
	inline float		DistanceToLineSquared(Vector thePos, Line3D& theLine) {return DistanceToLineSquared(thePos,theLine.mPos[0],theLine.mPos[1]);}
	float				DistanceToInfiniteLineSquared(Vector thePos, Vector theL1, Vector theL2);
	inline float		DistanceToInfiniteLineSquared(Vector thePos, Line3D& theLine) {return DistanceToInfiniteLineSquared(thePos,theLine.mPos[0],theLine.mPos[1]);}
	inline float		DistanceToInfiniteLine(Vector thePos, Vector theL1, Vector theL2) {return (float)sqrt(DistanceToInfiniteLineSquared(thePos,theL1,theL2));}
	inline float		DistanceToInfiniteLine(Vector thePos, Line3D& theLine) {return DistanceToInfiniteLine(thePos,theLine.mPos[0],theLine.mPos[1]);}

	// Shortest Line segment between two lines
	Line3D				ClosestLineLine(Vector theL1A, Vector theL1B, Vector theL2A, Vector theL2B, float* resultInterpolationA=NULL, float* resultInterpolationB=NULL); 
	inline Line3D		ClosestLineLine(Line3D aL1, Line3D aL2, float* resultInterpolationA=NULL, float* resultInterpolationB=NULL) {return ClosestLineLine(aL1.mPos[0],aL1.mPos[1],aL2.mPos[0],aL2.mPos[1],resultInterpolationA,resultInterpolationB);}
	Vector				ClosestPointOnTriangleToLine(Vector theT0, Vector theT1, Vector theT2, Vector theL1, Vector theL2);

	float				GetAngleBetweenVectors(Vector theV1, Vector theV2, Vector thePlaneNormal);
	inline float		GetAngleBetweenVectors(Vector theV1, Vector theV2, Plane thePlane) {return GetAngleBetweenVectors(theV1,theV1,thePlane.mNormal);}

	Line3D				GetSharedSeam(Triangle3D* aT1, Triangle3D* aT2, float theCloseEnough=.001f);
	Vector				GetPlanarLineIntersection(Plane thePlane,Vector theL1Start,Vector theL1End, Vector theL2Start, Vector theL2End);
	inline Vector		GetPlanarLineIntersection(Vector theL1Start,Vector theL1End, Vector theL2Start, Vector theL2End) {return GetPlanarLineIntersection(Plane(theL1Start,theL1End,theL2Start),theL1Start,theL1End,theL2Start,theL2End);}

	bool				GetSpherePlaneIntersection(Vector sphereCenter, float sphereRadius, Plane thePlane, Vector* theIntersection=NULL, float* theRadius=NULL);
	bool				DoesCylinderIntersectTriangle(Vector cylinderBase, Vector cylinderTop, float cylinderRadius, Vector triangleVertex1, Vector triangleVertex2, Vector triangleVertex3);
};


extern Math gMath;
extern Math3D gMath3D;

extern float gPrettyCloseNumber;
void SetPrettyCloseThreshold(float theNumber=gMath.mMachineEpsilon);
inline void SetPrettyClose(float theNumber=gMath.mMachineEpsilon) {SetPrettyCloseThreshold(theNumber);}
inline bool FEqual(float theF1, float theF2) {return (abs(theF1-theF2)<=gPrettyCloseNumber);}
inline bool DEqual(double theF1, double theF2) {return (abs(theF1-theF2)<=gPrettyCloseNumber);}
inline bool PrettyClose(float theF1, float theF2) {return FEqual(theF1,theF2);}
inline bool PrettyClose(double theF1, double theF2) {return DEqual(theF1,theF2);}
inline bool PrettyClose(int theF1, int theF2) {return (theF1==theF2);}
inline bool PrettyClose(unsigned int theF1, unsigned int theF2) {return (theF1==theF2);}
inline bool CloseEnough(float theF1, float theF2, float theEpsilon) {return (abs(theF1-theF2)<=theEpsilon);}

// Global RMod definitions for ease
inline float RMod(float theNumber, float theMod) {float aFix=theNumber;if (theNumber<0) aFix*=-1;float aResult=fmodf(aFix,theMod);if (theNumber<0) {aResult=(theMod)-aResult;if (aResult>=theMod) aResult=0;}return aResult;}
inline int RMod(int theNumber, int theMod)	{int aRet=theNumber%theMod;if (aRet<0) aRet=theMod+aRet;return aRet;}
inline float TowardZero(float theNumber, float theAmount) {if (theNumber>0) return theNumber-theAmount;return theNumber+theAmount;}
inline int TowardZero(int theNumber, int theAmount) {if (theNumber>0) return theNumber-theAmount;return theNumber+theAmount;}


inline float Snap(float theValue, float snapTo) {theValue/=snapTo;if (theValue>0) theValue=floorf(theValue+.5f);else theValue=ceilf(theValue-.5f);return theValue*snapTo;}
inline Point Snap(Point theValue, float snapTo) {return Point(Snap(theValue.mX,snapTo),Snap(theValue.mY,snapTo));}
inline Point Snap(Point theValue, Point snapTo) {return Point(Snap(theValue.mX,snapTo.mX),Snap(theValue.mY,snapTo.mY));}
inline Vector Snap(Vector theValue, float snapTo) {return Vector(Snap(theValue.mX,snapTo),Snap(theValue.mY,snapTo),Snap(theValue.mZ,snapTo));}
inline Vector Snap(Vector theValue, Vector snapTo) {return Vector(Snap(theValue.mX,snapTo.mX),Snap(theValue.mY,snapTo.mY),Snap(theValue.mZ,snapTo.mZ));}

inline float Round(float theValue, float roundTo, char upDown=1) {theValue/=roundTo;if (theValue>0) {if (upDown==1) theValue=ceilf(theValue);else theValue=floorf(theValue);}else  {if (upDown==1) theValue=floorf(theValue);else theValue=ceilf(theValue);}return theValue*roundTo;}
inline Point Round(Point theValue, float roundTo, char theDir=1) {return Point(Round(theValue.mX,roundTo,theDir),Round(theValue.mY,roundTo,theDir));}
inline Point Round(Point theValue, Point roundTo, char theDir=1) {return Point(Round(theValue.mX,roundTo.mX,theDir),Round(theValue.mY,roundTo.mY,theDir));}
inline Vector Round(Vector theValue, float roundTo, char theDir=1) {return Vector(Round(theValue.mX,roundTo),Round(theValue.mY,roundTo,theDir),Round(theValue.mZ,roundTo,theDir));}
inline Vector Round(Vector theValue, Vector roundTo, char theDir=1) {return Vector(Round(theValue.mX,roundTo.mX,theDir),Round(theValue.mY,roundTo.mY,theDir),Round(theValue.mZ,roundTo.mZ,theDir));}

inline float SnapDown(float theValue, float snapTo) {theValue/=snapTo;if (theValue>0) theValue=floorf(theValue);else theValue=ceilf(theValue);return theValue*snapTo;}
inline Point SnapDown(Point theValue, float snapTo) {return Point(SnapDown(theValue.mX,snapTo),SnapDown(theValue.mY,snapTo));}
inline Point SnapDown(Point theValue, Point snapTo) {return Point(SnapDown(theValue.mX,snapTo.mX),SnapDown(theValue.mY,snapTo.mY));}
inline Vector SnapDown(Vector theValue, float snapTo) {return Vector(SnapDown(theValue.mX,snapTo),SnapDown(theValue.mY,snapTo),SnapDown(theValue.mZ,snapTo));}
inline Vector SnapDown(Vector theValue, Vector snapTo) {return Vector(SnapDown(theValue.mX,snapTo.mX),SnapDown(theValue.mY,snapTo.mY),SnapDown(theValue.mZ,snapTo.mZ));}


#define HASH4(s) ((s[0])|(s[1]*256)|(s[2]*256*256)|(s[3]*256*256*256))
#define HASH8(s) (longlong)( ((longlong)s[0])|((longlong)s[1]*(longlong)256)|((longlong)s[2]*(longlong)(256*256))|((longlong)s[3]*(longlong)(256*256*256)) | ((longlong)s[4]*(longlong)((longlong)256*(longlong)256*(longlong)256 *(longlong)256))|((longlong)s[5]*(longlong)256*(longlong)256*(longlong)256*(longlong)256*(longlong)256)|((longlong)s[6]*(longlong)256*(longlong)256*(longlong)256 *(longlong)256*(longlong)256*(longlong)256)|((longlong)s[7]*(longlong)256*(longlong)256*(longlong)256 *(longlong)256*(longlong)256*(longlong)256*(longlong)256) )
inline int Hash4(char* theValue) {char aStr[5]={0,0,0,0,0};strncpy(aStr,theValue,4);return HASH4(aStr);}
inline longlong Hash8(char* theValue) {char aStr[9]={0,0,0,0,0,0,0,0,0};strncpy(aStr,theValue,8);return HASH8(aStr);}

enum
{
	EASE_NONE=0,
	EASE_SIN,
	EASE_QUAD,
	EASE_CUBE,
	EASE_QUART,
	EASE_QUINT,
	EASE_EXPO,
	EASE_CIRC,
	EASE_REBOUND,
	EASE_ELASTIC,
	EASE_BOUNCE,

	EASE_BOUNCEIN,
	EASE_BOUNCEOUT,
};















