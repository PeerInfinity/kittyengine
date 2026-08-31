#pragma once

#include "rapt_point.h"
#include "rapt_matrix.h"
#include "rapt_list.h"
#include <assert.h>

#ifndef _max
	#define _max(a,b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef _min
	#define _min(a,b) (((a) < (b)) ? (a) : (b))
#endif

#define RINTERPOLATE(theLow,theNumber,theHigh) (((theNumber)-(theLow))/((theHigh)-(theLow)))

#define Rect RaptRect
#define Line RaptLine

class Rect;
class String;
String& GetDiscardableString();
class AABBRect;
class Line
{
public:
	Line() {}
	Line(Point theP1, Point theP2) {mPos[0]=theP1;mPos[1]=theP2;}
	Line(float theX1, float theY1, float theX2, float theY2) {mPos[0].mX=theX1;mPos[0].mY=theY1;mPos[1].mX=theX2;mPos[1].mY=theY2;}
	Point		mPos[2];

	inline Point			Mid() {return (mPos[0]+mPos[1])/2;}
	inline float			Length() {return Point(mPos[0]-mPos[1]).Length();}
	inline float			Len() {return Length();}

	inline Line				Translate(Point thePos) {return Line(mPos[0]+thePos,mPos[1]+thePos);}
	inline Line				Translate(float theX, float theY) {return Translate(Point(theX,theY));}

	inline float			LengthSquared() {return Point(mPos[0]-mPos[1]).LengthSquared();}
	inline float			LenSquared() {return LengthSquared();}

	inline Point			Direction() {return Point(mPos[1]-mPos[0]).Normal();}
	inline Point			Dir() {return Direction();}

	inline Point			GetPointAt(float thePercent) {return mPos[0]+(ToVector()*thePercent);}

	inline Point			Diff() {return mPos[1]-mPos[0];}
	inline Point			ToVector() {return mPos[1]-mPos[0];}
	inline Point			Span() {return ToVector();}
	void					Expand(float theAmount) {Point aMove=Direction()*theAmount;mPos[0]-=aMove;mPos[1]+=aMove;}
	Line					GetExpand(float theAmount) {Line aResult=*this;Point aMove=Direction()*theAmount;aResult.mPos[0]-=aMove;aResult.mPos[1]+=aMove;return aResult;}
	void					Reverse() {Point aHold=mPos[0];mPos[0]=mPos[1];mPos[1]=aHold;}

	Rect					GetBound();// {return Rect(mPos[0],mPos[1]);}
	AABBRect				GetAABB();

	Line					GetPart(float theStartPercent, float theEndPercent);
	inline Line				GetGetment(float theStartPercent, float theEndPercent) {return GetPart(theStartPercent,theEndPercent);}

	bool					IsConnectedTo(Line theLine);
	bool					IsConnectedTo(Point thePoint);

	void					SetLength(float theLength);	// Sets length of line from center
	char*					ToString();

	float					Slope() {float aX=mPos[1].mX-mPos[0].mX;if (aX==0) return FLOATMAX;return (mPos[1].mY-mPos[0].mY)/aX;}
	float					YIntercept(float theY=0) 
	{
		float aDX=mPos[1].mX-mPos[0].mX;
		float aDY=mPos[1].mY-mPos[0].mY;
		float aDeltaY=theY-mPos[1].mY;
		if (aDY!=0) return mPos[1].mX+(aDX/aDY)*aDeltaY;
		else return NAN;
	}

	float					GetPercentAt(Point thePos);	// thePos is a point on the line... returns what percent it's at (reverse of GetVectorAt)

	void					Clip(Plane thePlane, char frontOrBack=-1);


};

class Line3D
{
public:
	Line3D() {}
	Line3D(Vector theP1, Vector theP2) {mPos[0]=theP1;mPos[1]=theP2;}
	Line3D(float theX1, float theY1, float theX2, float theY2) {mPos[0].mX=theX1;mPos[0].mY=theY1;mPos[1].mX=theX2;mPos[1].mY=theY2;}
	Line3D(int theValue) {mPos[0].mX=mPos[0].mY=mPos[0].mZ=mPos[1].mX=mPos[1].mY=mPos[1].mZ=(float)theValue;}

	Vector		mPos[2];

	inline Vector			Mid() {return (mPos[0]+mPos[1])/2;}
	inline float			Length() {return (mPos[0]-mPos[1]).Length();}
	inline float			Len() {return Length();}

	inline Line3D			Translate(Vector thePos) {return Line3D(mPos[0]+thePos,mPos[1]+thePos);}
	Line3D					Translate(float theX, float theY, float theZ);

	inline void				SetNAN() {mPos[0].SetNAN();}
	inline bool				IsNAN() {return mPos[0].IsNAN();}

	float					LengthSquared();
	float					LenSquared();

	Vector					Direction();
	Vector					Dir();

	Vector					Span();

	void					Expand(float theAmount);
	void					Reverse();

	Line3D					GetPart(float theStartPercent, float theEndPercent);
	void					SetLength(float theLength);	// Sets length of line from center
	void					SetLength(float theLength, char fromEnd);	// Sets length of line from center

	char*					ToString();

	Vector					GetSize();

	Vector					GetVectorAt(float thePercent);
	float					GetPercentAt(Vector thePos);	// thePos is a point on the line... returns what percent it's at (reverse of GetVectorAt)

};



class ILine
{
public:
	ILine() {}
	ILine(IPoint theP1, IPoint theP2) {mPos[0]=theP1;mPos[1]=theP2;}
	IPoint		mPos[2];
};


class Rect
{
public:
	Rect(void);
	Rect(float theX, float theY, float theWidth, float theHeight);
	Rect(float theWidth, float theHeight) {mX=0;mY=0;mWidth=theWidth;mHeight=theHeight;}
	Rect(Point theP1, Point theP2);
	Rect(Point theP1);
	Rect(Line theLine) {Resize(theLine.mPos[0],theLine.mPos[1]);}
	Rect(const Rect& theRect) {*this=theRect;}

	// Couple helpers for zany situations where compiler behavior might be unexpected...
	Rect				ToRect() {return *this;}
	void				FromRect(Rect theRect) {mX=theRect.mX;mY=theRect.mY;mW=theRect.mW;mH=theRect.mH;}

	void				Resize(float theX, float theY, float theWidth, float theHeight);
	void				Resize(Point theP1, Point theP2);
	void				SetSize(float theX, float theY, float theWidth, float theHeight) {Resize(theX,theY,theWidth,theHeight);}
	void				Size(float theX, float theY, float theWidth, float theHeight) {Resize(theX,theY,theWidth,theHeight);}
	void				Size(Rect theRect) {Resize(theRect.mX,theRect.mY,theRect.mWidth,theRect.mHeight);}
	void				Size(float theWidth, float theHeight);
	void				Size(Point theSize) {Size(theSize.mX,theSize.mY);}
	void				Size();

	void				Tweak(String theCommand);
	//
	// SIZE COMMANDS:
	// 	  x+=value;
	// 	  y+=value;
	//    w+=value;
	// 	  h+=value;
	// 	  left+=value; (reduces width so right stays in same place)
	// 	  top+=value; (reduces height so bottom stays in same place)
	//    right+=value;
	// 	  bottom+=value;
	//

	char*				ToString();

	bool				ContainsPoint(float theX, float theY);
	inline bool			ContainsPoint(int theX, int theY) {return ContainsPoint((float)theX,(float)theY);}
	inline bool			ContainsPoint(Point thePoint) {return ContainsPoint(thePoint.mX,thePoint.mY);}
	inline bool			ContainsPoint(IPoint thePoint) {return ContainsPoint(thePoint.mX,thePoint.mY);}

	bool				Intersects(float theX, float theY, float theWidth, float theHeight);
	inline bool			Intersects(Rect theRect) {return Intersects(theRect.mX,theRect.mY,theRect.mWidth,theRect.mHeight);}
	Rect				GetIntersection(Rect &theRect);

	inline bool			IsInside(Rect theRect) {return (theRect.ContainsPoint(UpperLeft()) && theRect.ContainsPoint(LowerRight()));}

	inline Point		InterpolateInto(Point thePos) {return Point(RINTERPOLATE(mX,thePos.mX,mX+mWidth),RINTERPOLATE(mY,thePos.mY,mY+mHeight));}

	Rect				Union(Rect theRect);
	Rect				Union(Point thePoint) {return Union(Rect(thePoint.mX,thePoint.mY,0.0f,0.0f));}
	Rect				Union(float theX, float theY) {return Union(Rect(theX,theY,0.0f,0.0f));}
	Rect				Union(float theX, float theY, float theWidth, float theHeight) {return Union(Rect(theX,theY,theWidth,theHeight));}


	void				MoveInto(Rect theIntoRect); // Moves a rect so it's inside theIntoRect (if theIntoRect is smaller, you'll get screwy behavior)
	inline void			FitInto(Rect theIntoRect) {MoveInto(theIntoRect);}
	void				ClipInto(Rect theIntoRect);
	void				CenterInto(Rect theIntoRect);
	inline void			CenterAt(float theX, float theY) {mX=theX-(mWidth/2);mY=theY-(mHeight/2);}
	inline void			CenterAt(Point thePos) {CenterAt(thePos.mX,thePos.mY);}

	inline void			CenterXAt(float theX) {mX=theX-(mWidth/2);}
	inline void			CenterYAt(float theY) {mY=theY-(mHeight/2);}

	inline Rect			GetCenterAt(float theX, float theY) {return Rect(theX-(mWidth/2),theY-(mHeight/2),mWidth,mHeight);}
	inline Rect			GetCenterAt(Point thePos) {return GetCenterAt(thePos.mX,thePos.mY);}

	void				Confine(Rect theIntoRect);

	Rect				GetClipInto(Rect theIntoRect);

	Rect				Interpolate(float theX, float theY, float theWidth, float theHeight, float thePercent);
	inline Rect			Interpolate(Rect theOtherRect, float thePercent) {return Interpolate(theOtherRect.mX,theOtherRect.mY,theOtherRect.mWidth,theOtherRect.mHeight,thePercent);}

	//
	// Getting some results from our rect...
	//
	Point				ClampPoint(Point thePos);

	inline Point		Center() {return Point(mX+(mWidth/2),mY+(mHeight/2));}
	inline float		CenterX() {return mX+(mWidth/2);}
	inline float		CenterY() {return mY+(mHeight/2);}
	inline Point		HalfSize() {return Point((mWidth/2),(mHeight/2));}
	inline Point		UpperLeft() {return Point(mX,mY);}
	inline Point		UpperRight() {return Point(mX+(mWidth),mY);}
	inline Point		LowerRight() {return Point(mX+(mWidth),mY+(mHeight));}
	inline Point		LowerLeft() {return Point(mX,mY+(mHeight));}
	inline Point		GetUpperLeft() {return Point(mX,mY);}
	inline Point		GetUpperRight() {return Point(mX+(mWidth),mY);}
	inline Point		GetLowerRight() {return Point(mX+(mWidth),mY+(mHeight));}
	inline Point		GetLowerLeft() {return Point(mX,mY+(mHeight));}
	inline Point		GetUL() {return Point(mX,mY);}
	inline Point		GetUR() {return Point(mX+(mWidth),mY);}
	inline Point		GetLR() {return Point(mX+(mWidth),mY+(mHeight));}
	inline Point		GetLL() {return Point(mX,mY+(mHeight));}


	inline Point		LeftMiddle() {return Point(mX,mY+(mHeight/2));}
	inline Point		RightMiddle() {return Point(mX+mWidth,mY+(mHeight/2));}
	inline Point		UpperMiddle() {return Point(mX+(mWidth/2),mY);}
	inline Point		LowerMiddle() {return Point(mX+(mWidth/2),mY+mHeight);}

	inline Point		LeftCenter() {return Point(mX,mY+(mHeight/2));}
	inline Point		RightCenter() {return Point(mX+mWidth,mY+(mHeight/2));}
	inline Point		TopCenter() {return Point(mX+(mWidth/2),mY);}
	inline Point		BottomCenter() {return Point(mX+(mWidth/2),mY+mHeight);}

	inline Rect			Expand(float theAmount) {return Rect(mX-theAmount,mY-theAmount,mWidth+(theAmount*2),mHeight+(theAmount*2));}
	inline Rect			Expand(float theAmountX, float theAmountY) {return Rect(mX-theAmountX,mY-theAmountY,mWidth+(theAmountX*2),mHeight+(theAmountY*2));}
	inline Rect			Expand(Point theAmount) {return Rect(mX-theAmount.mX,mY-theAmount.mY,mWidth+(theAmount.mX*2),mHeight+(theAmount.mY*2));}
	inline void			ExpandMe(float theAmount) {mX-=theAmount;mY-=theAmount;mWidth+=(theAmount*2);mHeight+=(theAmount*2);}
	inline void			ExpandMe(float theAmountX, float theAmountY) {mX-=theAmountX;mY-=theAmountY;mWidth+=(theAmountX*2);mHeight+=(theAmountY*2);}
	inline Rect			ExpandV(float theAmount) {return Rect(mX,mY-theAmount,mWidth,mHeight+(theAmount*2));}
	inline Rect			ExpandH(float theAmount) {return Rect(mX-theAmount,mY,mWidth+(theAmount*2),mHeight);}
	inline Rect			Contract(float theAmount) {return Expand(-theAmount);}
	inline bool	operator==(const Rect &theRect) {return (mX==theRect.mX)&(mY==theRect.mY)&(mWidth==theRect.mWidth)&(mHeight==theRect.mHeight);}
	inline Rect &operator=(const Rect &theRect) {if (this!=&theRect) {mX=theRect.mX;mY=theRect.mY;mWidth=theRect.mWidth;mHeight=theRect.mHeight;}return *this;}

	inline Rect			ExpandUp(float theAmount) {return Rect(mX,mY-theAmount,mWidth,mHeight+(theAmount));}
	inline Rect			ExpandDown(float theAmount) {return Rect(mX,mY,mWidth,mHeight+theAmount);}
	inline Rect			ExpandLeft(float theAmount) {return Rect(mX-theAmount,mY,mWidth+(theAmount),mHeight);}
	inline Rect			ExpandRight(float theAmount) {return Rect(mX,mY,mWidth+theAmount,mHeight);}

	Rect				ExpandPercent(float theAmount);
	Rect				ExpandPercentH(float theAmount);

	inline float		Width() {return mWidth;}
	inline float		Height() {return mHeight;}
	inline float		HalfWidth() {return mWidth/2;}
	inline float		HalfHeight() {return mHeight/2;}

	inline Rect			Translate(Point thePoint) {return Rect(mX+thePoint.mX,mY+thePoint.mY,mWidth,mHeight);}
	inline Rect			Translate(float theX, float theY) {return Rect(mX+theX,mY+theY,mWidth,mHeight);}

	inline float		Area() {return mWidth*mHeight;}
	inline float		GetArea() {return Area();}

	inline Line			Top() {Line aLine(UpperLeft(),UpperRight());return aLine;}
    inline Line			Bottom() {return Line(LowerLeft(),LowerRight());}
	inline Line			Left() {return Line(UpperLeft(),LowerLeft());}
	inline Line			Right() {return Line(UpperRight(),LowerRight());}
	inline Line			Diagonal1() {return Line(UpperLeft(),LowerRight());}
	inline Line			Diagonal2() {return Line(UpperRight(),LowerLeft());}
	inline Line			GetSideClockwise(int theSide, bool linesClockwiseToo=true) {Line aResult;switch (theSide) {case 0:aResult=Top();break;case 1:aResult=Right();break;case 2:aResult=Bottom();if (linesClockwiseToo) aResult.Reverse();break;case 3:aResult=Left();if (linesClockwiseToo) aResult.Reverse();break;}return aResult;}
	inline Point		GetCornerClockwise(int theCorner) {switch (theCorner) {case 0:return UpperLeft();case 1:return UpperRight();case 2:return LowerRight();case 3:return LowerLeft();}return UpperLeft();}

	// What's this?  Quickie for letting us get the corner that starts (or ends) a side, with the boolean to let us also get it in "positive xy flow"
	inline Point		GetSideStartCorner(int theSide, bool linesClockwiseToo=true) {return GetSideClockwise(theSide,linesClockwiseToo).mPos[0];}
	inline Point		GetSideEndCorner(int theSide, bool linesClockwiseToo=true) {return GetSideClockwise(theSide,linesClockwiseToo).mPos[1];}


	Point				ClosestPointTo(Point theCamera);

	inline void			ToInt() {mX=(float)(int)mX;mY=(float)(int)mY;mWidth=(float)(int)mWidth;mHeight=(float)(int)mHeight;}
	inline void			Floor() {mX=(float)(int)mX;mY=(float)(int)mY;mWidth=(float)(int)mWidth;mHeight=(float)(int)mHeight;}
	inline void			Round() {mX=(float)(int)(mX+.5f);mY=(float)(int)(mY+.5f);mWidth=(float)(int)(mWidth+.5f);mHeight=(float)(int)(mHeight+.5f);}

	inline void			AlignUL(Point thePos) {mX=thePos.mX;mY=thePos.mY;}
	inline void			AlignUR(Point thePos) {mX=thePos.mX-mWidth;mY=thePos.mY;}
	inline void			AlignLL(Point thePos) {mX=thePos.mX;mY=thePos.mY-mHeight;}
	inline void			AlignLR(Point thePos) {mX=thePos.mX-mWidth;mY=thePos.mY-mHeight;}

	Rect				GetScaled(float theScaleX, float theScaleY);
	inline Rect			GetScaled(float theScale) {return GetScaled(theScale,theScale);}
	inline void			Scale(float theScaleX, float theScaleY) {Rect aS=GetScaled(theScaleX, theScaleY);aS.CenterAt(Center());Size(aS);}
	inline void			Scale(float theScale) {Scale(theScale,theScale);}

	inline Rect			GetSquare() {return Rect(mX,mY,_max(mWidth,mHeight),_max(mWidth,mHeight));}
	inline float		MaxAxis() {return _max(mWidth,mHeight);}
	inline float		MajorAxis() {return _max(mWidth,mHeight);}
	inline float		MinAxis() {return _min(mWidth,mHeight);}
	inline float		MinorAxis() {return _min(mWidth,mHeight);}

	inline Point		MapInto(Point thePoint) {return MapInto(thePoint.mX,thePoint.mY);} // Converts actual rect sub-coordinate into 0-1
	Point				MapInto(float x, float y);
	inline Point		MapOutof(Point thePoint) {return MapOutof(thePoint.mX,thePoint.mY);} // Converts 0-1 to actual rect coordinates
	Point				MapOutof(float x, float y);

public:

	union
	{
		Point			mXY;
		struct
		{
			union
			{
				float			mX;
				float			mData1;
				float			mLeft;
			};
			union 
			{
				float			mY;
				float			mData2;
				float			mTop;
			};
		};
	};

	union
	{
		Point			mWH;
		struct
		{
			union 
			{
				float			mWidth;
				float			mData3;
				float			mRight;
				float			mW;
			};
			union 
			{
				float			mHeight;
				float			mData4;
				float			mBottom;
				float			mH;
			};
		};
	};
};


class IRect
{
public:
	IRect(void);
	IRect(int theX, int theY, int theWidth, int theHeight);

	void				Resize(int theX, int theY, int theWidth, int theHeight);
	void				SetSize(int theX, int theY, int theWidth, int theHeight) {Resize(theX,theY,theWidth,theHeight);}
	void				Size(int theX, int theY, int theWidth, int theHeight) {Resize(theX,theY,theWidth,theHeight);}
	void				Size(Rect theRect) {Resize((int)theRect.mX,(int)theRect.mY,(int)theRect.mWidth,(int)theRect.mHeight);}
	void				Size(IRect theRect) {Resize(theRect.mX,theRect.mY,theRect.mWidth,theRect.mHeight);}
	void				Size();

	bool				ContainsPoint(int theX, int theY);
	inline bool			ContainsPoint(float theX, float theY) {return ContainsPoint((int)theX,(int)theY);}
	inline bool			ContainsPoint(Point thePoint) {return ContainsPoint(thePoint.mX,thePoint.mY);}
	inline bool			ContainsPoint(IPoint thePoint) {return ContainsPoint(thePoint.mX,thePoint.mY);}

	bool				Intersects(int theX, int theY, int theWidth, int theHeight);
	inline bool			Intersects(IRect theRect) {return Intersects(theRect.mX,theRect.mY,theRect.mWidth,theRect.mHeight);}
	IRect				GetIntersection(IRect &theRect);

	IRect				Union(IRect theRect);
	IRect				Union(IPoint thePoint) {return Union(IRect(thePoint.mX,thePoint.mY,1,1));}
	inline IRect		Union(int theX, int theY, int theWidth, int theHeight) {return Union(IRect(theX,theY,theWidth,theHeight));}

	void				ClipInto(IRect theIntoRect);

	inline IPoint		Center() {return IPoint(mX+(mWidth/2),mY+(mHeight/2));}
	inline IPoint		HalfSize() {return IPoint((mWidth/2),(mHeight/2));}
	inline IPoint		UpperLeft() {return IPoint(mX,mY);}
	inline IPoint		UpperRight() {return IPoint(mX+(mWidth),mY);}
	inline IPoint		LowerRight() {return IPoint(mX+(mWidth),mY+(mHeight));}
	inline IPoint		LowerLeft() {return IPoint(mX,mY+(mHeight));}

	inline IRect		Expand(int theAmount) {return IRect(mX-theAmount,mY-theAmount,mWidth+(theAmount*2),mHeight+(theAmount*2));}
	inline IRect		ExpandV(int theAmount) {return IRect(mX,mY-theAmount,mWidth,mHeight+(theAmount*2));}
	inline IRect		ExpandH(int theAmount) {return IRect(mX-theAmount,mY,mWidth+(theAmount*2),mHeight);}
	inline IRect		Contract(int theAmount) {return Expand(-theAmount);}
	inline bool	operator==(const IRect &theRect) {return (mX==theRect.mX)&(mY==theRect.mY)&(mWidth==theRect.mWidth)&(mHeight==theRect.mHeight);}
	inline IRect &operator=(const IRect &theRect) {if (this!=&theRect) {mX=theRect.mX;mY=theRect.mY;mWidth=theRect.mWidth;mHeight=theRect.mHeight;}return *this;}



public:
	int				mX;
	int				mY;
	int				mWidth;
	int				mHeight;
};

//
// A version of IRect that uses X1,Y1,X2,Y2, instead of width/height
//
class XYRect
{
public:
	XYRect(void);
	XYRect(int theX1, int theY1, int theX2, int theY2);

	void				Resize(int theX1, int theY1, int theX2, int theY2);
	void				SetSize(int theX1, int theY1, int theX2, int theY2) {Resize(theX1,theY1,theX2,theY2);}
	void				Size(int theX1, int theY1, int theX2, int theY2) {Resize(theX1,theY1,theX2,theY2);}
	void				Size(Rect theRect) {Resize((int)theRect.mX,(int)theRect.mY,(int)(theRect.mX+theRect.mWidth),(int)(theRect.mY+theRect.mHeight));}
	void				Size(IRect theRect) {Resize(theRect.mX,theRect.mY,theRect.mX+theRect.mWidth,theRect.mY+theRect.mHeight);}
	char*				ToString();
	Rect				ToRect() {return Rect((float)mX1,(float)mY1,(float)(mX2-mX1)+1,(float)(mY2-mY1)+1);}

	bool				ContainsPoint(int theX, int theY) {return (theX>=mX1 && theY>=mY1 && theX<=(mX2) && theY<=(mY2));}
	inline bool			ContainsPoint(float theX, float theY) {return ContainsPoint((int)theX,(int)theY);}
	inline bool			ContainsPoint(Point thePoint) {return ContainsPoint(thePoint.mX,thePoint.mY);}
	inline bool			ContainsPoint(IPoint thePoint) {return ContainsPoint(thePoint.mX,thePoint.mY);}

	inline bool			ContainsRect(XYRect theRect) const
	{
		bool aResult=true;
		aResult=aResult && (mX1<=theRect.mX1);
		aResult=aResult && (mY1<=theRect.mY1);
		aResult=aResult && (theRect.mX2<=mX2);
		aResult=aResult && (theRect.mY2<=mY2);
		return aResult;
	}


	bool				Intersects(int theX1, int theY1, int theX2, int theY2);
	inline bool			Intersects(IRect theRect) {return Intersects(theRect.mX,theRect.mY,theRect.mX+theRect.mWidth,theRect.mY+theRect.mHeight);}
	inline bool			Intersects(XYRect theRect) {return Intersects(theRect.mX1,theRect.mY1,theRect.mX2,theRect.mY2);}
	XYRect				GetIntersection(XYRect &theRect);

	bool				IsTouching(int theX1, int theY1, int theX2, int theY2);
	inline bool			IsTouching(XYRect theRect) {return IsTouching(theRect.mX1,theRect.mY1,theRect.mX2,theRect.mY2);}

	XYRect				Union(XYRect theRect);
	XYRect				Union(IPoint thePoint) {return Union(XYRect(thePoint.mX,thePoint.mY,thePoint.mX,thePoint.mY));}
	inline XYRect		Union(int theX1, int theY1, int theX2, int theY2) {return Union(XYRect(theX1,theY1,theX2,theY2));}

	inline IPoint		Center() {return IPoint((mX1+mX2)/2,(mY1+mY2)/2);}
	inline IPoint		HalfSize() {return Point((mX2-mX1)/2,(mY2-mY1)/2);}
	inline IPoint		UpperLeft() {return IPoint(mX1,mY1);}
	inline IPoint		UpperRight() {return IPoint(mX2,mY1);}
	inline IPoint		LowerRight() {return IPoint(mX2,mY2);}
	inline IPoint		LowerLeft() {return IPoint(mX1,mY2);}

	inline XYRect		Expand(int theAmount) {return XYRect(mX1-theAmount,mY1-theAmount,mX2+theAmount,mY2+theAmount);}
	inline XYRect		ExpandV(int theAmount) {return XYRect(mX1,mY1-theAmount,mX2,mY2+(theAmount));}
	inline XYRect		ExpandH(int theAmount) {return XYRect(mX1-theAmount,mY1,mX2+(theAmount),mY2);}
	inline XYRect		Contract(int theAmount) {return Expand(-theAmount);}
	inline XYRect &operator=(const XYRect &theRect) {if (this!=&theRect) {mX1=theRect.mX1;mY1=theRect.mY1;mX2=theRect.mX2;mY2=theRect.mY2;}return *this;}

	inline int			Width() {return (mX2-mX1)+1;}
	inline int			Height() {return (mY2-mY1)+1;}

	inline XYRect		Translate(IPoint theDir) {return XYRect(mX1+theDir.mX,mY1+theDir.mY,mX2+theDir.mX,mY2+theDir.mY);}
	inline XYRect		Translate(int theX, int theY) {return XYRect(mX1+theX,mY1+theY,mX2+theX,mY2+theY);}



public:
	int				mX1;
	int				mY1;
	int				mX2;
	int				mY2;
};

class AABBRect
{
public:
	AABBRect(void);
	AABBRect(float theX1, float theY1, float theX2, float theY2);
	AABBRect(Point theP1, Point theP2)
	{
		Point aMin=PointMin(theP1,theP2);
		Point aMax=PointMax(theP1,theP2);
		mX1=aMin.mX;
		mY1=aMin.mY;
		mX2=aMax.mX;
		mY2=aMax.mY;
	}
	AABBRect(Rect theRect)
	{
		Point aMin=theRect.UpperLeft();
		Point aMax=theRect.LowerRight();
		mX1=aMin.mX;
		mY1=aMin.mY;
		mX2=aMax.mX;
		mY2=aMax.mY;
	}

	void				Resize(float theX1, float theY1, float theX2, float theY2);
	void				SetSize(float theX1, float theY1, float theX2, float theY2) {Resize(theX1,theY1,theX2,theY2);}
	void				Size(float theX1, float theY1, float theX2, float theY2) {Resize(theX1,theY1,theX2,theY2);}
	void				Size(Rect theRect) {Resize(theRect.mX,theRect.mY,(theRect.mX+theRect.mWidth),(theRect.mY+theRect.mHeight));}
	void				Size(IRect theRect) {Resize((float)theRect.mX,(float)theRect.mY,(float)(theRect.mX+theRect.mWidth),(float)(theRect.mY+theRect.mHeight));}
	char*				ToString();
	Rect				ToRect() {return Rect(mX1,mY1,(mX2-mX1)+1,(mY2-mY1)+1);}

	inline void			SizeFromCenter(Point theCenter, Point theSize) {mX1=theCenter.mX-(theSize.mX);mY1=theCenter.mY-(theSize.mY);mX2=theCenter.mX+(theSize.mX);mY2=theCenter.mY+(theSize.mY);}

	inline bool			ContainsPoint(float theX, float theY) {return (theX>=mX1 && theY>=mY1 && theX<=(mX2) && theY<=(mY2));}
	inline bool			ContainsPoint(int theX, int theY) {return ContainsPoint((float)theX,(float)theY);}
	inline bool			ContainsPoint(Point thePoint) {return ContainsPoint(thePoint.mX,thePoint.mY);}
	inline bool			ContainsPoint(IPoint thePoint) {return ContainsPoint(thePoint.mX,thePoint.mY);}

	inline void			CenterAt(float theX, float theY) {float aW=Width()/2;float aH=Height()/2;mX1=theX-aW;mX2=theX+aW;mY1=theY-aH;mY2=theY+aH;}
	inline void			CenterAt(Point thePos) {CenterAt(thePos.mX,thePos.mY);}
	inline bool			Contains(AABBRect theRect) const
	{
		bool aResult=true;
		aResult=aResult && (mX1<=theRect.mX1);
		aResult=aResult && (mY1<=theRect.mY1);
		aResult=aResult && (theRect.mX2<=mX2);
		aResult=aResult && (theRect.mY2<=mY2);
		return aResult;
	}


	inline bool			Intersects(float theX1, float theY1, float theX2, float theY2) {return !((theX2<mX1) || (theY2<mY1) || (theX1>mX2) || (theY1>mY2));}
	inline bool			Intersects(IRect theRect) {return Intersects((float)theRect.mX,(float)theRect.mY,(float)(theRect.mX+theRect.mWidth),(float)(theRect.mY+theRect.mHeight));}
	inline bool			Intersects(AABBRect theRect) {return Intersects(theRect.mX1,theRect.mY1,theRect.mX2,theRect.mY2);}
	AABBRect			GetIntersection(AABBRect &theRect);

	bool				IsTouching(float theX1, float theY1, float theX2, float theY2);
	inline bool			IsTouching(AABBRect theRect) {return IsTouching(theRect.mX1,theRect.mY1,theRect.mX2,theRect.mY2);}
	float				GetPerimeter() {return 2.0f * ((mX2-mX1) + (mY2-mY1));}


	AABBRect			Union(AABBRect theRect);
	AABBRect			Union(Point thePoint) {return Union(AABBRect(thePoint.mX,thePoint.mY,thePoint.mX,thePoint.mY));}
	AABBRect			Union(IPoint thePoint) {return Union(AABBRect((float)thePoint.mX,(float)thePoint.mY,(float)thePoint.mX,(float)thePoint.mY));}
	inline AABBRect		Union(float theX1, float theY1, float theX2, float theY2) {return Union(AABBRect(theX1,theY1,theX2,theY2));}
	inline void			Flip(float flipX, float flipY) 
	{
		mX1*=flipX;
		mY1*=flipY;
		mX2*=flipX;
		mY2*=flipY;
		if (mX2<mX1) {float aHold=mX1;mX1=mX2;mX2=aHold;}
		if (mY2<mY1) {float aHold=mY1;mY1=mY2;mY2=aHold;}
	}

	inline Point		Center() {return Point((mX1+mX2)/2,(mY1+mY2)/2);}
	inline float		CenterX() {return (mX1+mX2)/2;}
	inline float		CenterY() {return (mY1+mY2)/2;}
	inline Point		HalfSize() {return Point((mX2-mX1)/2,(mY2-mY1)/2);}
	inline Point		UpperLeft() {return Point(mX1,mY1);}
	inline Point		UpperRight() {return Point(mX2,mY1);}
	inline Point		LowerRight() {return Point(mX2,mY2);}
	inline Point		LowerLeft() {return Point(mX1,mY2);}

	inline Point		LowerBound() {return Point(mX1,mY1);}
	inline Point		UpperBound() {return Point(mX2,mY2);}
	inline void			SetLowerBound(Point thePoint) {mX1=thePoint.mX;mY1=thePoint.mY;}
	inline void			SetUpperBound(Point thePoint) {mX2=thePoint.mX;mY2=thePoint.mY;}

	inline AABBRect		Expand(float theAmount) {return AABBRect(mX1-theAmount,mY1-theAmount,mX2+theAmount,mY2+theAmount);}
	inline AABBRect		ExpandV(float theAmount) {return AABBRect(mX1,mY1-theAmount,mX2,mY2+(theAmount));}
	inline AABBRect		ExpandH(float theAmount) {return AABBRect(mX1-theAmount,mY1,mX2+(theAmount),mY2);}
	inline AABBRect		Contract(float theAmount) {return Expand(-theAmount);}
	inline AABBRect		&operator=(const AABBRect &theRect) {if (this!=&theRect) {mX1=theRect.mX1;mY1=theRect.mY1;mX2=theRect.mX2;mY2=theRect.mY2;}return *this;}

	inline float		Width() {return (mX2-mX1)+1;}
	inline float		Height() {return (mY2-mY1)+1;}
	inline Point		GetSize() {return Point((mX2-mX1)+1,(mY2-mY1)+1);}


	inline AABBRect		Translate(Point theDir) {return AABBRect(mX1+theDir.mX,mY1+theDir.mY,mX2+theDir.mX,mY2+theDir.mY);}
	inline AABBRect		Translate(float theX, float theY) {return AABBRect(mX1+theX,mY1+theY,mX2+theX,mY2+theY);}


	inline void Combine(const AABBRect& aabb)
	{
		mX1=_min(mX1,aabb.mX1);
		mY1=_min(mY1,aabb.mY1);
		mX2=_max(mX2,aabb.mX2);
		mY2=_max(mY2,aabb.mY2);
	}

	inline void Combine(const AABBRect& aabb1, const AABBRect& aabb2)
	{
		mX1=_min(aabb1.mX1,aabb2.mX1);
		mY1=_min(aabb1.mY1,aabb2.mY1);
		mX2=_max(aabb1.mX2,aabb2.mX2);
		mY2=_max(aabb1.mY2,aabb2.mY2);
	}


public:
	float			mX1;
	float			mY1;
	float			mX2;
	float			mY2;
};

template <typename var_type>
class QuadT
{
public:

	QuadT() {}
	QuadT(Rect theRect) {mCorner[0]=theRect.UpperLeft();mCorner[1]=theRect.UpperRight();mCorner[2]=theRect.LowerLeft();mCorner[3]=theRect.LowerRight();}
	QuadT(var_type theP1, var_type theP2, var_type theP3, var_type theP4) {mCorner[0]=theP1;mCorner[1]=theP2;mCorner[2]=theP3;mCorner[3]=theP4;}


	var_type				mCorner[4];
	inline var_type&		UpperLeft() {return mCorner[0];}
	inline var_type&		UpperRight() {return mCorner[1];}
	inline var_type&		LowerLeft() {return mCorner[2];}
	inline var_type&		LowerRight() {return mCorner[3];}
	inline var_type			Center() {return (mCorner[0]+mCorner[1]+mCorner[2]+mCorner[3])/4;}
	inline var_type			Mid(int corner1, int corner2) {return (mCorner[corner1]+mCorner[corner2])/2;}
	inline var_type			Mid(int corner1, int corner2, int corner3) {return (mCorner[corner1]+mCorner[corner2]+mCorner[corner3])/3;}
	inline var_type			Mid() {return Center();}
	inline var_type			Mid(int corner1, int corner2, float theInterpolation) {return mCorner[corner1]*(1-theInterpolation)+(mCorner[corner2]*theInterpolation);}

	bool					ContainsPoint(var_type thePoint)
	{
		var_type& aCorner0=mCorner[0];
		var_type& aCorner1=mCorner[1];
		var_type& aCorner2=mCorner[3];
		var_type& aCorner3=mCorner[2];

		var_type aV1=aCorner1-aCorner0;
		var_type aV2=aCorner2-aCorner1;
		var_type aV3=aCorner3-aCorner2;
		var_type aV4=aCorner0-aCorner3;

		var_type aW1=thePoint-aCorner0;
		var_type aW2=thePoint-aCorner1;
		var_type aW3=thePoint-aCorner2;
		var_type aW4=thePoint-aCorner3;

		float aDot=aV1.Dot(aW1);if (aDot<0) return false;
		aDot=aV2.Dot(aW2);if (aDot<0) return false;
		aDot=aV3.Dot(aW3);if (aDot<0) return false;
		aDot=aV4.Dot(aW4);if (aDot<0) return false;
		return true;
	}
	

#if defined(_WIN32) // || defined(_LINUX)
	//
	// Hrm... I do want this in linux, but I get an issue where I need String defined prior, but String includes Rect.
	//
	char*					ToString()
	{
		String& aResult=GetDiscardableString();
		aResult=Sprintf("%s  ->  %s  ->  %s  ->  %s",mCorner[0].ToString(),mCorner[1].ToString(),mCorner[2].ToString(),mCorner[3].ToString());
		return aResult.c();
	}
#endif

	virtual var_type&		operator[](int theSlot) {return mCorner[theSlot];}

	inline void				CenterAt(var_type thePos)
	{
		var_type aDiff=thePos-Center();
		mCorner[0]+=aDiff;
		mCorner[1]+=aDiff;
		mCorner[2]+=aDiff;
		mCorner[3]+=aDiff;
	}

	inline Line			GetLine(int theLine)
	{
		switch (theLine)
		{
		case 0: return Line(mCorner[0],mCorner[1]);
		case 1: return Line(mCorner[2],mCorner[3]);
		case 2: return Line(mCorner[0],mCorner[2]);
		case 3: return Line(mCorner[1],mCorner[3]);
		}
		return Line(Center(),Center());
	}
	inline Line3D		GetLine3D(int theLine)
	{
		switch (theLine)
		{
			case 0: return Line3D(mCorner[0],mCorner[1]);
			case 1: return Line3D(mCorner[2],mCorner[3]);
			case 2: return Line3D(mCorner[0],mCorner[2]);
			case 3: return Line3D(mCorner[1],mCorner[3]);
		}
		return Line3D(Center(),Center());
	}

	inline Line			GetLine(int theCorner1, int theCorner2) {return Line(mCorner[theCorner1],mCorner[theCorner2]);}
	inline Line3D		GetLine3D(int theCorner1, int theCorner2) {return Line3D(mCorner[theCorner1],mCorner[theCorner2]);}

	inline void			MoveTo(var_type theMove)
	{
		var_type aMov=mCorner[0]-theMove;
		for (int aCount=0;aCount<4;aCount++) mCorner[aCount]-=aMov;
	}
	inline void			MoveTo(float theX, float theY)
	{
		var_type aMov=mCorner[0]-var_type(theX,theY);
		for (int aCount=0;aCount<4;aCount++) mCorner[aCount]-=aMov;
	}

	void Translate(var_type thePos) 
	{
		mCorner[0]+=thePos;
		mCorner[1]+=thePos;
		mCorner[2]+=thePos;
		mCorner[3]+=thePos;
	}

	void Rotate(float theAngle)
	{
		var_type aCenter=Center();
		mCorner[0]-=aCenter;
		mCorner[1]-=aCenter;
		mCorner[2]-=aCenter;
		mCorner[3]-=aCenter;

		Matrix aMat;
		aMat.Rotate2D(theAngle);
		aMat.ProcessPoints(mCorner,4);

		mCorner[0]+=aCenter;
		mCorner[1]+=aCenter;
		mCorner[2]+=aCenter;
		mCorner[3]+=aCenter;
	}

	void Rotate(float theAngle, Vector theAxis)
	{
		var_type aCenter=Center();
		mCorner[0]-=aCenter;
		mCorner[1]-=aCenter;
		mCorner[2]-=aCenter;
		mCorner[3]-=aCenter;

		Matrix aMat;
		aMat.RotateAroundAxis3D(theAxis,theAngle);
		aMat.ProcessPoints(mCorner,4);

		mCorner[0]+=aCenter;
		mCorner[1]+=aCenter;
		mCorner[2]+=aCenter;
		mCorner[3]+=aCenter;
	}


	void Process(Matrix& theMat)
	{
		var_type aCenter=Center();
		mCorner[0]-=aCenter;
		mCorner[1]-=aCenter;
		mCorner[2]-=aCenter;
		mCorner[3]-=aCenter;

		theMat.ProcessPoints(mCorner,4);

		mCorner[0]+=aCenter;
		mCorner[1]+=aCenter;
		mCorner[2]+=aCenter;
		mCorner[3]+=aCenter;
	}


	inline void			Scale(float theScale)
	{
		var_type aCenter=Center();
		mCorner[0]-=aCenter;
		mCorner[1]-=aCenter;
		mCorner[2]-=aCenter;
		mCorner[3]-=aCenter;
		mCorner[0]*=theScale;
		mCorner[1]*=theScale;
		mCorner[2]*=theScale;
		mCorner[3]*=theScale;
		mCorner[0]+=aCenter;
		mCorner[1]+=aCenter;
		mCorner[2]+=aCenter;
		mCorner[3]+=aCenter;
	}

	inline QuadT<var_type>&		operator+=(const var_type &thePoint) {mCorner[0]+=thePoint;mCorner[1]+=thePoint;mCorner[2]+=thePoint;mCorner[3]+=thePoint;return *this;}
	inline QuadT<var_type>&		operator-=(const var_type &thePoint) {mCorner[0]-=thePoint;mCorner[1]-=thePoint;mCorner[2]-=thePoint;mCorner[3]-=thePoint;return *this;}
	inline QuadT<var_type>&		operator*=(const var_type &thePoint) {mCorner[0]*=thePoint;mCorner[1]*=thePoint;mCorner[2]*=thePoint;mCorner[3]*=thePoint;return *this;}
	inline QuadT<var_type>&		operator/=(const var_type &thePoint) {mCorner[0]/=thePoint;mCorner[1]/=thePoint;mCorner[2]/=thePoint;mCorner[3]/=thePoint;return *this;}
	inline QuadT<var_type>&		operator=(Rect &theRect) {mCorner[0]=theRect.UpperLeft();mCorner[1]=theRect.UpperRight();mCorner[2]=theRect.LowerLeft();mCorner[3]=theRect.LowerRight();return *this;}

	QuadT<var_type> GetSubQuad(float theX1, float theY1, float theX2, float theY2)
	{
		var_type aTopVector=UpperRight()-UpperLeft();
		var_type aBottomVector=LowerRight()-LowerLeft();
		var_type aLeftVector=LowerLeft()-UpperLeft();
		var_type aRightVector=LowerRight()-UpperRight();

		QuadT<var_type> aQ;
		aQ.UpperLeft()=UpperLeft()+var_type(aTopVector.mX*theX1,aLeftVector.mY*theY1);
		aQ.UpperRight().mX=UpperLeft().mX+aTopVector.mX*(theX2);
		aQ.UpperRight().mY=UpperRight().mY+aRightVector.mY*(theY1);
		aQ.LowerLeft()=UpperLeft()+var_type(aBottomVector.mX*theX1,aLeftVector.mY*(theY2));
		aQ.LowerRight().mX=LowerLeft().mX+aBottomVector.mX*(theX2);
		aQ.LowerRight().mY=UpperRight().mY+aRightVector.mY*(theY2);

		return aQ;
	}

	inline QuadT<var_type> GetSubQuad(Rect theRect) {return GetSubQuad(theRect.mX,theRect.mY,theRect.mX+theRect.mWidth,theRect.mY+theRect.mHeight);}

	//
	// Warning: GetSubPoint is only effective on square quads...
	// This will take a 0-1 value and map it into the quad.
	//
	var_type				GetSubPoint(float theX, float theY)
	{
		var_type aQVector=LowerRight()-UpperLeft();
		var_type aP=UpperLeft();
		aP.mX+=aQVector.mX*theX;
		aP.mY+=aQVector.mY*theY;
		return aP;
	}

	inline Point		GetSubPoint(var_type thePos) {return GetSubPoint(thePos.mX,thePos.mY);}
	inline Point		GetUV(float theX, float theY) {return GetSubPoint(theX,theY);}
	inline Point		GetUV(var_type thePos) {return GetSubPoint(thePos.mX,thePos.mY);}

	inline Rect			GetBounds()
	{
		var_type aUpperLeft=mCorner[0];
		var_type aLowerRight=mCorner[0];

		for (int aCount=1;aCount<4;aCount++)
		{
			aUpperLeft.mX=_min(aUpperLeft.mX,mCorner[aCount].mX);
			aUpperLeft.mY=_min(aUpperLeft.mY,mCorner[aCount].mY);
			aLowerRight.mX=_max(aLowerRight.mX,mCorner[aCount].mX);
			aLowerRight.mY=_max(aLowerRight.mY,mCorner[aCount].mY);
		}

		return Rect(aUpperLeft.mX,aUpperLeft.mY,aLowerRight.mX-aUpperLeft.mX,aLowerRight.mY-aUpperLeft.mY);
	}

	inline QuadT<var_type> FlipH() 
	{
		var_type aHold1=mCorner[0];
		var_type aHold2=mCorner[2];
		mCorner[0]=mCorner[1];
		mCorner[2]=mCorner[3];
		mCorner[1]=aHold1;
		mCorner[3]=aHold2;
		return *this;
	}
	inline QuadT<var_type> FlipV() 
	{
		var_type aHold1=mCorner[0];
		var_type aHold2=mCorner[1];
		mCorner[0]=mCorner[2];
		mCorner[1]=mCorner[3];
		mCorner[2]=aHold1;
		mCorner[3]=aHold2;
		return *this;
	}
	inline QuadT<var_type> FlipHV() 
	{
		QuadT<var_type> aHold=*this;
		mCorner[0]=aHold.mCorner[3];
		mCorner[1]=aHold.mCorner[2];
		mCorner[2]=aHold.mCorner[1];
		mCorner[3]=aHold.mCorner[0];
		return *this;
	}


	inline QuadT<var_type> GetFlipH() 
	{
		var_type aHold1=mCorner[0];
		var_type aHold2=mCorner[2];
		QuadT<var_type> aResult;
		aResult.mCorner[0]=mCorner[1];
		aResult.mCorner[2]=mCorner[3];
		aResult.mCorner[1]=aHold1;
		aResult.mCorner[3]=aHold2;
		return aResult;
	}
	inline QuadT<var_type> GetFlipV() 
	{
		var_type aHold1=mCorner[0];
		var_type aHold2=mCorner[1];
		QuadT<var_type> aResult;
		aResult.mCorner[0]=mCorner[2];
		aResult.mCorner[1]=mCorner[3];
		aResult.mCorner[2]=aHold1;
		aResult.mCorner[3]=aHold2;
		return aResult;
	}

	void				InflateLine(Line theLine, float theWidth)
	{
		theWidth/=2;

		var_type aPerp=theLine.Dir().Perp();
		UpperLeft()=theLine.mPos[0]+aPerp*theWidth;
		UpperRight()=theLine.mPos[1]+aPerp*theWidth;
		LowerLeft()=theLine.mPos[0]-aPerp*theWidth;
		LowerRight()=theLine.mPos[1]-aPerp*theWidth;
	}

	void				GuaranteeDistance(int theCorner1, int theCorner2, float theDistance)
	{
		var_type aVec=mCorner[theCorner1]-mCorner[theCorner2];
		if (aVec.LengthSquared()<(theDistance*theDistance))
		{
			Line aLine;
			aLine.mPos[0]=mCorner[theCorner1];
			aLine.mPos[1]=mCorner[theCorner2];
			aLine.SetLength(theDistance);
			mCorner[theCorner1]=aLine.mPos[0];
			mCorner[theCorner2]=aLine.mPos[1];
		}
	}

	inline var_type		GetCenter() {return (mCorner[0]+mCorner[1]+mCorner[2]+mCorner[3])/4;}

	void				FromPlane(Plane thePlane, float theWidth, float theHeight)
	{
		mCorner[0]=(var_type(-theWidth/2,-theHeight/2));
		mCorner[1]=(var_type(theWidth/2,-theHeight/2));
		mCorner[2]=(var_type(-theWidth/2,theHeight/2));
		mCorner[3]=(var_type(theWidth/2,theHeight/2));
		thePlane.UnPlanify(mCorner,4);
	}
	void				FromRect(float theWidth, float theHeight)
	{
		mCorner[0]=(var_type(-theWidth/2,-theHeight/2));
		mCorner[1]=(var_type(theWidth/2,-theHeight/2));
		mCorner[2]=(var_type(-theWidth/2,theHeight/2));
		mCorner[3]=(var_type(theWidth/2,theHeight/2));
	}

	void				FromRect(Rect theRect)
	{
		mCorner[0]=theRect.UpperLeft();
		mCorner[1]=theRect.UpperRight();
		mCorner[2]=theRect.LowerLeft();
		mCorner[3]=theRect.LowerRight();
	}


	inline Rect			ToRect() {return Rect(mCorner[0].mX,mCorner[0].mY,mCorner[3].mX-mCorner[0].mX,mCorner[3].mY-mCorner[0].mY);}

};

typedef QuadT<Point> Quad;
typedef QuadT<Vector> Quad3D;


class QuadPair
{
public:
	Quad				mQuad[2];
};

class Poly
{
public:
	Poly(void);
	~Poly(void);

	//
	//Do these two line segments intersect?
	//
	static bool				SegmentsIntersect(Point theStart1, Point theEnd1, Point theStart2, Point theEnd2);

	//
	//Does the triangle T1, T2, T3 contain the point?
	//
	static bool				TriangleContainsPoint(Point theT1, Point theT2, Point theT3, Point thePoint);

	//
	//Just a list of vertex indeces that make
	//up a subPoly in split Polys!
	//

	struct SubPoly
	{
	public:
		SubPoly(Point *thePointList){mPoint=thePointList;}
		~SubPoly()
		{
			delete [] mIndex;
			mIndex=0;
		}

		bool		Intersects(SubPoly *theSub);
		bool		WillIntersect(SubPoly *theSub, Point theRelativeVelocity);

		Point		*mPoint;
		int			*mIndex;
		int			mCount;
	};

	struct SubPolyList
	{
		SubPolyList(Poly *thePoly){mPoly=thePoly;mSub=0;mCount=mSize=0;}
		~SubPolyList(){Clear();}

		//
		//Add a new sub Poly.
		//
		void					operator+=(SubPoly *theSubPoly);

		//
		//Clear all of the sub Polys.
		//
		void					Clear();

		//
		//Array Housekeeping for sub Poly list.
		//
		void					Tidy();
		void					Size(int theSize);

		bool					Intersects(SubPolyList *theList);

		bool					WillIntersect(SubPolyList*theList,Point theRelativeVelocity);

		//
		//List of sub Polys.
		//
		SubPoly					**mSub;

		//
		//Back pointer to Poly points.
		//
		Poly					*mPoly;

		//
		//Sub Poly list count.
		//
		int						mCount;

		//
		//Sub Poly list size.
		//
		int						mSize;

	};

	//
	//Represents a 1-D interval. [min, ..., max]
	//
	struct Interval
	{
	public:
		Interval(){}
		Interval(float theMin, float theMax){mMin=theMin;mMax=theMax;}
		~Interval(){}
		float	mMin, mMax;
		float	DistanceBetween(Interval theInterval)
		{
			if(mMin<theInterval.mMin)return theInterval.mMin-mMax;
			else return mMin-theInterval.mMax;
		}
		//
		//If the distance between intervals is negative, they intersect.
		//
		inline bool	Intersects(Interval theInterval)
		{
			return DistanceBetween(theInterval)<=0;
		}
	};

	//
	//These are helper functions for SegmentsIntersect
	//
	static float			TriangleArea(float x1, float y1, float x2, float y2, float x3, float y3);
	static bool				Between(float x1, float y1, float x2, float y2, float x3, float y3);


	//
	//After triangulation, this allows us to easily snag a triangle.
	//
	void					GetTriangle(int theIndex, Point *thePoint1, Point *thePoint2, Point *thePoint3);
	void					GetTriangleIndexes(int theIndex, int& theIndex1, int& theIndex2, int& theIndex3);
	inline void				GetTriangleIndicia(int theIndex, int& theIndex1, int& theIndex2, int& theIndex3) {GetTriangleIndexes(theIndex,theIndex1,theIndex2,theIndex3);}

	//
	//The number of triangles.
	//
	inline int				GetTriangleCount(){return GetSubPolyCount();}

	//
	//The number of sub Polys.
	//
	inline int				GetSubPolyCount(){return mSub ? mSub->mCount : 0;}

	//
	//Unmodified point list.
	//
	//
	Point					*mPointBase;

	//
	//Point list after transformations, this is kept
	//as a separate list to prevent an accumulation of
	//rounding errors. This also allows us to do transformations
	//from the center everytime without "untranslating."
	//
	Point					*mPoint;

	//Fetch a point from the point list.
	inline Point &operator[](int theIndex) {return mPoint[theIndex];}
	inline Point& GetPoint(int theIndex) {return mPoint[theIndex];}

	//
	//Add a new point to the Poly. Don't re-add the first point to close the Poly.
	//
	void					AddPoint(Point thePoint){AddPoint(thePoint.mX,thePoint.mY);}
	void                    AddPoint(float theX, float theY);
	void                    AddUniquePoint(float theX, float theY);
	inline void             AddUniquePoint(Point thePos) {AddUniquePoint(thePos.mX,thePos.mY);}
	inline void				operator+=(const Point &thePoint){AddPoint(thePoint.mX,thePoint.mY);}

	//
	//Fits the point arrays to the data exactly.
	//
	void					Tidy();

	//
	//Test two moving Polys and determine if they will intersect.
	//Only works for convex Polys
	//
	bool					WillIntersect(Poly* thePoly,Point *theTranslationVector,Point theVelocity1=Point(0,0),Point theVelocity2=Point(0,0));

	bool					WillIntersect(Poly* thePoly,Point theVelocity1=Point(0,0),Point theVelocity2=Point(0,0));
	bool					WillIntersect(SubPoly* theSub, Point theRelativeVelocity);
	bool					WillIntersect(SubPolyList*theList, Point theRelativeVelocity);

	//
	//Intersection tests for simple, convex Polys.
	//
	bool					Intersects(Poly* thePoly);
	bool					Intersects(SubPolyList *theList);
	bool					Intersects(SubPoly *theSub);
	inline bool				Intersects(Point thePoint){return Intersects(thePoint.mX,thePoint.mY);}
	bool					Intersects(float theX, float theY);
	inline bool				Intersects(Rect theRect){return Intersects(theRect.mX,theRect.mY,theRect.mWidth,theRect.mHeight);}
	bool					Intersects(float theX, float theY, float theWidth, float theHeight);
	bool					Intersects(Line theLine);
	bool					Intersects(Point theCenter, Point theRadii);

	//
	//Does the Poly contain a point? (Even works for complex Polys)
	//
	inline bool				ContainsPoint(Point thePoint){return Intersects(thePoint.mX,thePoint.mY);}
	inline bool				ContainsPoint(float theX, float theY){return Intersects(theX,theY);}

	//
	//Perform affine transformations with matrices.
	//
	void					Transform(Matrix theMatrix);
	void					Transform(Point theTranslate=Point(0,0), float theRotation=0, float theScale=1);

	inline void				Translate(Point thePos) {Translate(thePos.mX,thePos.mY);}
	void					Translate(float theX, float theY);

	//
	//Regular Poly: All sides are equal length and all angles are equal.
	//
	void					MakeRegular(Point theCenter, int theVertices, float theRadius);

	//
	//Puts the center of the Poly at its centroid.
	//
	Point					Center();

	//
	//Shifts all the points such that the center is moved to the specified location.
	//
	void					CenterAt(Point thePoint=Point(0,0));

	//
	//Mostly for testing purposes.
	//
	void					Draw();

	//
	//Start over fresh and free up all the memory.
	//
	void					Clear();

	//
	//Convex: Poly contains no angles > 180 degrees.
	//
	bool					IsConvex();
	//
	//Concave: Non-convex
	//
	inline bool				IsConcave(){return !IsConvex();}

	//
	//Are the points in clockwise or counter-clockwise order?
	//
	bool					IsClockwise();
	inline bool				IsCounterClockwise(){return !IsClockwise();}

	//
	//Does the Poly self-intersect?
	//
	bool					IsSimple();
	inline bool				IsComplex(){return !IsSimple();}

	//
	//Is the Poly composed of any subPolys?
	//
	inline bool				IsSplit(){return mSub != 0;}

	//
	//Do these three points form a CW or CCW kink?
	//
	inline bool				IsCounterClockwise(Point aStart, Point aMiddle, Point aEnd){return !IsClockwise(aStart,aMiddle,aEnd);}
	bool					IsClockwise(Point aStart, Point aMiddle, Point aEnd);

	//
	//Just reverses the vertex list. CW Poly becomes CCW.
	//
	void					ReverseDirection();

	//
	//Sets the size of the base point array.
	//
	void					Size(int theSize);

	//
	//The next two functions only work on
	//SIMPLE Polys. Simple Polys do not
	//self-intersect!
	//

	//
	//Break the Poly down into N-2 triangles,
	//where N is the number of vertices.
	//If there are 3 or less vertices,
	//the Poly remains simple.
	//
	void					Triangulate();
	void					Untriangulate(); // Clears the memory
	void					Triangulate(Array<vbindex>& theResult);

	//
	//The plan is to break the Poly down into
	//the minimal number of convex subPolys.
	//
	void					Decompose();

	//
	//The center of mass of the Poly.
	//
	Point					GetCentroid();

	//
	//Center of the Poly.
	//
	inline Point			GetCenter(){return mCenter;}

	//
	//Retrieve the number of edges.
	//
	inline int				GetEdgeCount(){return _max(0,mCount-1);}

	//
	//Retrieve the number of vertices.
	//
	inline int				GetVertexCount(){return mCount;}
	inline int				GetPointCount(){return mCount;}
	inline int				GetCount(){return mCount;}

	//
	//Get the mass for a given density. Mass = area (or volume) * density
	//
	inline float			GetMass(float theDensity=0.5f){return GetArea()*theDensity;}

	//
	//Absolute value of the Poly's area.
	//(Negative regions counted as positive!)
	//
	float					GetArea();

	//
	//The moment of inertia. (Unverified)
	//
	float					GetInertia();

	//
	//Gives the max distance of the vertices from the center, useful for creating
	//bounding circles. This will be the radius disregarding any transformations.
	//
	float					GetBoundingRadius();

	//
	//Gives us a bounding rectangle, will not
	//necessarily bound Poly after rotations.
	//Disregards transformations. This is good for
	//bounding static platforms and what not.
	//
	Rect					GetBoundingRect();
	Rect					mBoundingRect;

	//
	//A square that bounds the utransformed Poly.
	//We can use a square instead of a rect if we plan on rotating the Poly.
	//
	Rect					GetBoundingBox();

	//
	// Inflates the polygon around the center, or a center.
	//
	void					Inflate(float theAmount) {Inflate(Center(), theAmount);}
	void					Inflate(Point theCenter, float theAmount);


	//protected:
	//
	//Specialized function for decomposition.
	//Does a line between the first point and the second point
	//cross any other line or lie outside of the Poly?
	//
	bool					Diagonal(int theIndex1, int theIndex2);

	//
	//Specialized function for triangulation. Look up Poly "ears."
	//
	bool					IsEar(int *theIndexList, int theVertexCount, int theIndex);

	//
	//The center of the Poly disregarding translations.
	//
	Point					mCenterBase;

	//
	//The center of the Poly after translations.
	//
	Point					mCenter;

	//
	//Matrix that holds the last transformation. We may want to peek at this later.
	//
	Point					mTranslate;

	//
	//List of sub Polys.
	//
	SubPolyList				*mSub;

	//
	//Size of the point list.
	//
	int						mSize;

	//
	//Number of vertices the Poly has.
	//
	int						mCount;

	//
	//Find the interval of a list of points projected onto an axis.
	//
	Interval				FindInterval(Point theAxis);
	Interval				FindInterval(Point theAxis, Point theRelativeVelocity);
	static Interval			FindInterval(SubPoly *theSub, Point theAxis);
	static Interval			FindInterval(SubPoly *theSub, Point theAxis, Point theRelativeVelocity);
	Interval				FindInterval(Point *thePoints, int thePointCount, Point theAxis);
	Interval				FindInterval(Point *thePoints, int thePointCount, Point theAxis, Point theRelativeVelocity);
};


class Triangle
{
public:
	Triangle()
	{
	}

	Triangle(Point thePos1, Point thePos2, Point thePos3)
	{
		mPos[0]=thePos1;
		mPos[1]=thePos2;
		mPos[2]=thePos3;
	}

	Triangle(Vector thePos1, Vector thePos2, Vector thePos3)
	{
		mPos[0]=Point(thePos1.mX,thePos1.mY);
		mPos[1]=thePos2;
		mPos[2]=thePos3;
	}


	inline Point &operator[](int theIndex) {return mPos[_min(2,_max(0,theIndex))];}


	Point					GetVertex(int theNumber) {return mPos[theNumber];}
	Point					GetCentroid() {return (mPos[0]+mPos[1]+mPos[2])/3;}
	Rect					GetBoundingRect();

	bool					IsSharedVertex(Triangle* theT);
	bool					IsSharedEdge(Triangle* theT, Line* theSharedLine=NULL);

	bool					ContainsPoint(Point thePos);

	bool					IsDegenerate();
	Line					GetHypotenuse();

	//
	// If you ever want to implement area, use "Heron's formula"
	//
	/*
	float					Area()
	{
		float aL1=gMath.Distance(mPos[1],mPos[0]);
		Vector aE1=mPos[1]-mPos[0];
		Vector aE2=mPos[2]-mPos[0];
		Vector aE3=Vector(aE1
	}
	*/

	void Expand(float theAmount)
	{
		/*
		Point aPos[3];
		aPos[0]=mPos[0];aPos[1]=mPos[1];aPos[2]=mPos[2];

		Point a01=aPos[0]-aPos[1];a01.SetLength(theAmount);
		Point a12=aPos[1]-aPos[2];a12.SetLength(theAmount);
		Point a02=aPos[0]-aPos[2];a02.SetLength(theAmount);
		Point a10=-a01;
		Point a21=-a12;
		Point a20=-a02;

		mPos[0]+=a01;mPos[0]+=a02;
		mPos[1]+=a10;mPos[1]+=a12;
		mPos[2]+=a20;mPos[2]+=a21;
		/**/
	}


public:
	Point					mPos[3];


};


class RectComplex
{
public:
	RectComplex(void);
	RectComplex(float theX, float theY, float theWidth, float theHeight);
	RectComplex(Rect theRect);
	RectComplex(RectComplex &theRect);
	virtual ~RectComplex(void);

	//
	// Resets the complex content
	//
	void				Reset() {_FreeList(Rect,mRectList);if(mPerimeterData)delete mPerimeterData;mPerimeterData=NULL;}
	inline void			Clear() {Reset();}

	//
	// Simple list rectangle append
	//
	void				AppendRect(float theX, float theY, float theWidth, float theHeight);
	inline void			AppendRect(Rect theRect) {AppendRect(theRect.mX,theRect.mY,theRect.mWidth,theRect.mHeight);}

	//
	// Get rectangle info
	// 
	inline int			GetRectCount() {return mRectList.GetCount();}
	inline Rect			GetRect(int theNumber) {return *(Rect*)mRectList[theNumber];};

	//
	// Adds a rectangle to the complex
	//
	void				Add(float theX, float theY, float theWidth, float theHeight);
	inline void			Add(Rect theRect) {Add(theRect.mX,theRect.mY,theRect.mWidth,theRect.mHeight);}
	void				Add(RectComplex &theRect);
	void operator+=		(Rect &theRect) {Add(theRect);}
	void operator+=		(RectComplex &theRect) {Add(theRect);}

	//
	// Removes a rectangle from the complex
	//
	void				Subtract(float theX, float theY, float theWidth, float theHeight);
	inline void			Subtract(Rect theRect) {Subtract(theRect.mX,theRect.mY,theRect.mWidth,theRect.mHeight);}
	void				Subtract(RectComplex &theRect);
	void operator-=		(Rect &theRect) {Subtract(theRect);}
	void operator-=		(RectComplex &theRectComplex) {Subtract(theRectComplex);}

	//
	// Just makes us match someone else
	//
	void operator=		(RectComplex &theRect);

	//
	// Tells if we contain a point, just like Rect
	//
	bool				ContainsPoint(float theX, float theY);
	inline bool			ContainsPoint(int theX, int theY) {return ContainsPoint((float)theX,(float)theY);}

	//
	// Gets some information about the complex area
	//
	float				GetMaxY();
	float				GetMinY();
	float				GetMaxX();
	float				GetMinX();
	inline float		GetWidth() {return GetMaxX()-GetMinX();}
	inline float		GetHeight() {return GetMaxY()-GetMinY();}
	float				GetArea();

	//
	// For adjusting the attributes/position/etc of all the rects.
	//
	void				Move(float theX, float theY);

	//
	// Intersection info
	//
	bool				Intersects(RectComplex &theRectComplex);
	bool				Intersects(Rect theRect);

	//
	// Consolidates rects into smallest possible number of rects
	// (i.e. two rects forming a perfect rectangle merge into one)
	// 
	//
	void				Consolidate();

/*
	//
	// Removes any rect info outside the given area
	//
	void				Clip(float theX, float theY, float theWidth, float theHeight);
	void				Clip(Rect theRect) {Clip(theRect.mX,theRect.mY,theRect.mWidth,theRect.mHeight);}
*/
public:
	//
	// List of rectangles
	//
	List				mRectList;
	inline List&		GetRects() {return mRectList;}

	
	
protected:

	struct		PerimeterData
	{
		List			mCornerData;
		List			mCorners;
		List			mCornerNormals;
		List			mCornerNormalLines;
		List			mLines;
		List			mLineNormals;
		~PerimeterData()
		{
			_FreeList(Corner,mCornerData);
			_FreeList(Line,mLines);
			_FreeList(Point,mCorners);
			_FreeList(Point,mCornerNormals);
			_FreeList(Point,mLineNormals);
			_FreeList(Line,mCornerNormalLines);
		}
	};

	PerimeterData			*mPerimeterData;

	void BuildPerimeterCorner(List *theCorners, List *theCornerPoints, List *theCornerNormal, Point thePotentialCorner, float theFudge=0.001f);

public:

	struct Corner
	{
		Corner(Point thePos)
		{
			mPos=thePos;
			mU=false;
			mD=false;
			mR=false;
			mL=false;
			mInner=false;
		}
		Point mPos;
		bool mU, mD, mR, mL, mInner;
	};

	//
	//Populates lists of corners, edges, corner normals, and edge normals
	//and stores them. Call this BEFORE the functions below.
	//
	void				CalculatePerimeter(float theFudge=0.01f);

	//
	//Get the number of corners (and corner normals)
	//from the outer perimeter of the rectangle group.
	//
	int					GetPerimeterCornerCount(){if (mPerimeterData) return mPerimeterData->mCorners.mCount; return 0;}

	//
	//Get the number of lines (and line normals)
	//from the outer perimeter of the rectangle group.
	//
	int					GetPerimeterLineCount(){if (mPerimeterData) return mPerimeterData->mLines.mCount; return 0;}

	//
	//Fetch items from the perimeter lists.
	//
	Line				GetPerimeterLine(int theIndex) {if (mPerimeterData) return *((Line*)mPerimeterData->mLines.mData[theIndex]);return Line(Point(0,0),Point(0,0));}
	Point				GetPerimeterLineNormal(int theIndex) {if (mPerimeterData) return *((Point*)mPerimeterData->mLineNormals.mData[theIndex]);return Point(0,0);}
	Point				GetPerimeterCorner(int theIndex) {if (mPerimeterData) return *((Point*)mPerimeterData->mCorners.mData[theIndex]);return Point(-999999999999.0f,-999999999999.0f);}
	Point				GetPerimeterCornerNormal(int theIndex) {if (mPerimeterData) return *((Point*)mPerimeterData->mCornerNormals.mData[theIndex]);return Point(0,0);}
	Line				GetPerimeterCornerNormalsByLine(int theIndex){if (mPerimeterData) return *((Line*)mPerimeterData->mCornerNormalLines.mData[theIndex]);return Line();}
	bool				IsInnerCorner(int theIndex);
	bool				IsInnerLine(int theIndex);

};


template <typename var_type>
class CubeT
{
public:
	CubeT(void) {mX=0;mY=0;mZ=0;mXSize=0;mYSize=0;mZSize=0;}
	CubeT(float theX, float theY, float theZ, float theXSize, float theYSize, float theZSize) {Resize(theX,theY,theZ,theXSize,theYSize,theZSize);}
	CubeT(int theX, int theY, int theZ, int theXSize, int theYSize, int theZSize) {Resize((var_type)theX,(var_type)theY,(var_type)theZ,(var_type)theXSize,(var_type)theYSize,(var_type)theZSize);}
	CubeT(float theXSize, float theYSize, float theZSize) {mX=0;mY=0;mZ=0;mXSize=(var_type)theXSize;mYSize=(var_type)theYSize;mZSize=(var_type)theZSize;}
	CubeT(int theXSize, int theYSize, int theZSize) {mX=0;mY=0;mZ=0;mXSize=(var_type)theXSize;mYSize=(var_type)theYSize;mZSize=(var_type)theZSize;}
	CubeT(Vector theP1, Vector theP2) {Resize(theP1,theP2);}
	CubeT(IVector theP1, IVector theP2) {Resize(theP1,theP2);}
	CubeT(Vector theP1, Vector theP2, Vector theP3) {Resize(theP1,theP2,theP3);}
	CubeT(Vector thePos, float theRadius) {Size(theRadius*2,theRadius*2,theRadius*2);CenterAt(thePos);}

	bool				IsNull() {return (mX==0 && mY==0 && mZ==0 && mXSize==0 && mYSize==0 && mZSize==0);}

	char*				ToString();// {String& aResult=GetDiscardableString();aResult=Sprintf("%.2f,%.2f,%.2f - %.2f,%.2f,%.2f",(float)mX,(float)mY,(float)mZ,(float)mXSize,(float)mYSize,(float)mZSize);return aResult.c();}
	char*				ToStringI();// {String& aResult=GetDiscardableString();aResult=Sprintf("%d,%d,%d - %d,%d,%d",(int)mX,(int)mY,(int)mZ,(int)mXSize,(int)mYSize,(int)mZSize);return aResult.c();}

	void				Reset() {mX=mY=mZ=mXSize=mYSize=mZSize=0;}
	void				Resize(float theX, float theY, float theZ, float theXSize, float theYSize, float theZSize) {mX=(var_type)theX;mY=(var_type)theY;mZ=(var_type)theZ;mXSize=(var_type)theXSize;mYSize=(var_type)theYSize;mZSize=(var_type)theZSize;}
	void				Resize(int theX, int theY, int theZ, int theXSize, int theYSize, int theZSize) {mX=(var_type)theX;mY=(var_type)theY;mZ=(var_type)theZ;mXSize=(var_type)theXSize;mYSize=(var_type)theYSize;mZSize=(var_type)theZSize;}
	void				Resize(Vector theP1, Vector theP2)  {Vector aUL=Vector(_min(theP1.mX,theP2.mX),_min(theP1.mY,theP2.mY),_min(theP1.mZ,theP2.mZ));Vector aLR=Vector(_max(theP1.mX,theP2.mX),_max(theP1.mY,theP2.mY),_max(theP1.mZ,theP2.mZ));aLR-=aUL;Resize((var_type)aUL.mX,(var_type)aUL.mY,(var_type)aUL.mZ,(var_type)aLR.mX,(var_type)aLR.mY,(var_type)aLR.mZ);}
	void				Resize(IVector theP1, IVector theP2) {Vector aUL=Vector(_min(theP1.mX,theP2.mX),_min(theP1.mY,theP2.mY),_min(theP1.mZ,theP2.mZ));Vector aLR=Vector(_max(theP1.mX,theP2.mX),_max(theP1.mY,theP2.mY),_max(theP1.mZ,theP2.mZ));aLR-=aUL;Resize((var_type)aUL.mX,(var_type)aUL.mY,(var_type)aUL.mZ,(var_type)aLR.mX,(var_type)aLR.mY,(var_type)aLR.mZ);}
	inline void			SetSize(float theX, float theY, float theZ, float theXSize, float theYSize, float theZSize) {Resize(theX,theY,theZ, theXSize, theYSize, theZSize);}
	inline void			SetSize(int theX, int theY, int theZ, int theXSize, int theYSize, int theZSize) {Resize(theX,theY,theZ, theXSize, theYSize, theZSize);}
	void				Size(float theX, float theY, float theZ, float theXSize, float theYSize, float theZSize) {Resize(theX,theY,theZ, theXSize, theYSize, theZSize);}
	void				Size(int theX, int theY, int theZ, int theXSize, int theYSize, int theZSize) {Resize(theX,theY,theZ, theXSize, theYSize, theZSize);}
	void				Size(CubeT<float> theCube) {Resize(theCube.mX,theCube.mY,theCube.mZ,theCube.mXSize,theCube.mYSize,theCube.mZSize);}
	void				Size(CubeT<int> theCube) {Resize(theCube.mX,theCube.mY,theCube.mZ,theCube.mXSize,theCube.mYSize,theCube.mZSize);}
	void				Size(float theXSize, float theYSize, float theZSize) {mXSize=(var_type)theXSize;mYSize=(var_type)theYSize;mZSize=(var_type)theZSize;}
	void				Size(int theXSize, int theYSize, int theZSize) {mXSize=(var_type)theXSize;mYSize=(var_type)theYSize;mZSize=(var_type)theZSize;}
	void				Size(Vector theSize) {Size(theSize.mX,theSize.mY,theSize.mZ);}
	void				Size(IVector theSize) {Size(theSize.mX,theSize.mY,theSize.mZ);}
	void				Size() {Size(0,0,0,0,0,0);}

	void				Resize(Vector theP1, Vector theP2,Vector theP3)  {Vector aUL=Vector(_min(theP3.mX,_min(theP1.mX,theP2.mX)),_min(theP3.mY,_min(theP1.mY,theP2.mY)),_min(theP3.mZ,_min(theP1.mZ,theP2.mZ)));Vector aLR=Vector(_max(theP3.mX,_max(theP1.mX,theP2.mX)),_max(theP3.mY,_max(theP1.mY,theP2.mY)),_max(theP3.mY,_max(theP1.mZ,theP2.mZ)));aLR-=aUL;Resize((var_type)aUL.mX,(var_type)aUL.mY,(var_type)aUL.mZ,(var_type)aLR.mX,(var_type)aLR.mY,(var_type)aLR.mZ);}
	inline void			FromTriangle(Vector theP1, Vector theP2,Vector theP3) {Resize(theP1,theP2,theP3);}

	bool				ContainsPoint(float theX, float theY, float theZ) {return (theX>=mX && theY>=mY && theZ>=mZ && theX<(mX+mXSize) && theY<(mY+mYSize) && theZ<(mZ+mZSize));}
	bool				ContainsPoint(int theX, int theY, int theZ) {return (theX>=mX && theY>=mY && theZ>=mZ && theX<(mX+mXSize) && theY<(mY+mYSize) && theZ<(mZ+mZSize));}
	inline bool			ContainsPoint(Vector thePoint) {return ContainsPoint(thePoint.mX,thePoint.mY,thePoint.mZ);}
	inline bool			ContainsPoint(IVector thePoint) {return ContainsPoint(thePoint.mX,thePoint.mY,thePoint.mZ);}

	bool				ContainsPointExpanded(float theX, float theY, float theZ, float theXP) {return (theX>=mX-theXP && theY>=mY-theXP && theZ>=mZ-theXP && theX<(mX+mXSize+theXP) && theY<(mY+mYSize+theXP) && theZ<(mZ+mZSize+theXP));}
	bool				ContainsPointExpanded(int theX, int theY, int theZ, int theXP) {return (theX>=mX-theXP && theY>=mY-theXP && theZ>=mZ-theXP && theX<(mX+mXSize+theXP) && theY<(mY+mYSize+theXP) && theZ<(mZ+mZSize+theXP));}
	inline bool			ContainsPointExpanded(Vector thePoint, float theXP) {return ContainsPointExpanded(thePoint.mX,thePoint.mY,thePoint.mZ, theXP);}
	inline bool			ContainsPointExpanded(IVector thePoint, int theXP) {return ContainsPointExpanded(thePoint.mX,thePoint.mY,thePoint.mZ,theXP);}

	inline bool			Contains(CubeT<var_type> theCube) const
	{
		bool aResult=true;
		aResult=aResult && (mX<=theCube.mX);
		aResult=aResult && (mY<=theCube.mY);
		aResult=aResult && (mZ<=theCube.mZ);
		aResult=aResult && (theCube.mX+theCube.mXSize<=mX+mXSize);
		aResult=aResult && (theCube.mY+theCube.mYSize<=mY+mYSize);
		aResult=aResult && (theCube.mZ+theCube.mZSize<=mZ+mZSize);
		return aResult;
	}



	bool				Intersects(float theX, float theY, float theZ, float theXSize, float theYSize, float theZSize) {return !((theX+theXSize<=mX) || (theY+theYSize<=mY) || (theZ+theZSize<=mZ) || (theX>=mX+mXSize) || (theY>=mY+mYSize) || (theZ>=mZ+mZSize));}
	bool				Intersects(int theX, int theY, int theZ, int theXSize, int theYSize, int theZSize) {return !((theX+theXSize<=mX) || (theY+theYSize<=mY) || (theZ+theZSize<=mZ) || (theX>=mX+mXSize) || (theY>=mY+mYSize) || (theZ>=mZ+mZSize));}
	inline bool			Intersects(CubeT<float> theCube) {return Intersects(theCube.mX,theCube.mY,theCube.mZ,theCube.mXSize,theCube.mYSize,theCube.mZSize);}
	inline bool			Intersects(CubeT<int> theCube) {return Intersects(theCube.mX,theCube.mY,theCube.mZ,theCube.mXSize,theCube.mYSize,theCube.mZSize);}
	CubeT<var_type>		GetIntersection(CubeT<float> theCube) {var_type x1=(var_type)_max(mX,theCube.mX);var_type x2=(var_type)_min(mX+mXSize,theCube.mX+theCube.mXSize);var_type y1=(var_type)_max(mY,theCube.mY);var_type y2=(var_type)_min(mY+mYSize,theCube.mY+theCube.mYSize);var_type z1=(var_type)_max(mZ,theCube.mZ);var_type z2=(var_type)_min(mZ+mZSize,theCube.mZ+theCube.mZSize);if (((x2-x1)<0) || ((y2-y1)<0) || ((z2-z1)<0)) return CubeT<var_type>(0,0,0,0,0,0);else return CubeT<var_type>((var_type)x1,(var_type)y1,(var_type)z1,(var_type)(x2-x1),(var_type)(y2-y1),(var_type)(z2-z1));}
	CubeT<var_type>		GetIntersection(CubeT<int> theCube) {var_type x1=(var_type)_max(mX,theCube.mX);var_type x2=(var_type)_min(mX+mXSize,theCube.mX+theCube.mXSize);var_type y1=(var_type)_max(mY,theCube.mY);var_type y2=(var_type)_min(mY+mYSize,theCube.mY+theCube.mYSize);var_type z1=(var_type)_max(mZ,theCube.mZ);var_type z2=(var_type)_min(mZ+mZSize,theCube.mZ+theCube.mZSize);if (((x2-x1)<0) || ((y2-y1)<0) || ((z2-z1)<0)) return CubeT<var_type>(0,0,0,0,0,0);else return CubeT<var_type>((var_type)x1,(var_type)y1,(var_type)z1,(var_type)(x2-x1),(var_type)(y2-y1),(var_type)(z2-z1));}

	bool				IsInside(CubeT<float> theCube) {return (theCube.ContainsPoint(Corner(0)) && theCube.ContainsPoint(Corner(7)));}
	bool				IsInside(CubeT<int> theCube) {return (theCube.ContainsPoint(Corner(0)) && theCube.ContainsPoint(Corner(7)));}

	CubeT<var_type>		Union(CubeT<var_type> theCube) {if (mX==0 && mY==0 && mZ==0 && mXSize==0 && mYSize==0 && mZSize==0) return CubeT<var_type>((var_type)theCube.mX,(var_type)theCube.mY,(var_type)theCube.mZ,(var_type)theCube.mXSize,(var_type)theCube.mYSize,(var_type)theCube.mZSize);var_type x1 =(var_type)_min(mX, theCube.mX);var_type x2 =(var_type)_max(mX + mXSize, theCube.mX + theCube.mXSize);var_type y1 =(var_type)_min(mY, theCube.mY);var_type y2 =(var_type) _max(mY + mYSize, theCube.mY + theCube.mYSize);var_type z1 = (var_type)_min(mZ, theCube.mZ);var_type z2 = (var_type)_max(mZ + mZSize, theCube.mZ + theCube.mZSize);return CubeT<var_type>((var_type)x1, (var_type)y1, (var_type)z1, (var_type)(x2 - x1), (var_type)(y2 - y1), (var_type)(z2 - z1));}
	CubeT<var_type>		Union(float theX, float theY, float theZ) {if (mX==0 && mY==0 && mZ==0 && mXSize==0 && mYSize==0 && mZSize==0) return CubeT<var_type>((var_type)theX,(var_type)theY,(var_type)theZ,(var_type)0,(var_type)0,(var_type)0);var_type x1 =(var_type)_min(mX, theX);var_type x2 =(var_type) _max(mX + mXSize, theX);var_type y1 =(var_type) _min(mY, theY);var_type y2 =(var_type) _max(mY + mYSize, theY);var_type z1 =(var_type) _min(mZ, theZ);var_type z2 =(var_type) _max(mZ + mZSize, theZ);return CubeT<var_type>((var_type)x1, (var_type)y1, (var_type)z1, (var_type)(x2 - x1), (var_type)(y2 - y1), (var_type)(z2 - z1));}
	CubeT<var_type>		Union(int theX, int theY, int theZ) {if (mX==0 && mY==0 && mZ==0 && mXSize==0 && mYSize==0 && mZSize==0) return CubeT<var_type>((var_type)theX,(var_type)theY,(var_type)theZ,(var_type)0,(var_type)0,(var_type)0);var_type x1 = _min(mX, theX);var_type x2 = _max(mX + mXSize, theX);var_type y1 = _min(mY, theY);var_type y2 = _max(mY + mYSize, theY);var_type z1 = _min(mZ, theZ);var_type z2 = _max(mZ + mZSize, theZ);return CubeT<var_type>(x1, y1, z1, x2 - x1, y2 - y1, z2 - z1);}
	CubeT<var_type>		Union(Vector thePoint) {return Union(thePoint.mX,thePoint.mY,thePoint.mZ);}
	CubeT<var_type>		Union(IVector thePoint) {return Union(thePoint.mX,thePoint.mY,thePoint.mZ);}
	CubeT<var_type>		Union(float theX, float theY, float theZ, float theXSize, float theYSize, float theZSize) {if (mX==0 && mY==0 && mZ==0 && mXSize==0 && mYSize==0 && mZSize==0) return CubeT<var_type>((var_type)theX,(var_type)theY,(var_type)theZ,(var_type)theXSize,(var_type)theYSize,(var_type)theZSize);var_type x1=(var_type)_min(mX, theX);var_type x2=(var_type)_max(mX + mXSize, theX + theXSize);var_type y1=(var_type)_min(mY, theY);var_type y2=(var_type)_max(mY + mYSize, theY + theYSize);var_type z1=(var_type)_min(mZ, theZ);var_type z2=(var_type)_max(mZ + mZSize, theZ + theZSize);return CubeT<var_type>((var_type)x1, (var_type)y1, (var_type)z1, (var_type)(x2 - x1), (var_type)(y2 - y1), (var_type)(z2 - z1));}
	CubeT<var_type>		Union(int theX, int theY, int theZ, int theXSize, int theYSize, int theZSize) {if (mX==0 && mY==0 && mZ==0 && mXSize==0 && mYSize==0 && mZSize==0) return CubeT<var_type>((var_type)theX,(var_type)theY,(var_type)theZ,(var_type)theXSize,(var_type)theYSize,(var_type)theZSize);var_type x1 = (var_type)_min(mX, theX);var_type x2 = (var_type)_max(mX + mXSize, theX + theXSize);var_type y1 = (var_type)_min(mY, theY);var_type y2 = (var_type)_max(mY + mYSize, theY + theYSize);var_type z1 = (var_type)_min(mZ, theZ);var_type z2 = (var_type)_max(mZ + mZSize, theZ + theZSize);return CubeT<var_type>((var_type)x1, (var_type)y1, (var_type)z1, (var_type)(x2 - x1), (var_type)(y2 - y1), (var_type)(z2 - z1));}

	inline void			Unionize(CubeT<var_type> theCube) {Size(Union(theCube));}
	inline void			Unionize(float theX, float theY, float theZ) {Size(Union(theX,theY,theZ));}
	inline void			Unionize(int theX, int theY, int theZ) {Size(Union(theX,theY,theZ));}
	inline void			Unionize(Vector thePoint) {Size(Union(thePoint));}
	inline void			Unionize(IVector thePoint) {Size(Union(thePoint));}
	inline void			Unionize(float theX, float theY, float theZ, float theXSize, float theYSize, float theZSize) {Size(Union(theX,theY,theZ,theXSize,theYSize,theZSize));}
	inline void			Unionize(int theX, int theY, int theZ, int theXSize, int theYSize, int theZSize) {Size(Union(theX,theY,theZ,theXSize,theYSize,theZSize));}

	inline void			UnionWith(CubeT<var_type> theCube) {Size(Union(theCube));}
	inline void			UnionWith(float theX, float theY, float theZ) {Size(Union(theX,theY,theZ));}
	inline void			UnionWith(int theX, int theY, int theZ) {Size(Union(theX,theY,theZ));}
	inline void			UnionWith(Vector thePoint) {Size(Union(thePoint));}
	inline void			UnionWith(IVector thePoint) {Size(Union(thePoint));}
	inline void			UnionWith(float theX, float theY, float theZ, float theXSize, float theYSize, float theZSize) {Size(Union(theX,theY,theZ,theXSize,theYSize,theZSize));}
	inline void			UnionWith(int theX, int theY, int theZ, int theXSize, int theYSize, int theZSize) {Size(Union(theX,theY,theZ,theXSize,theYSize,theZSize));}


	void				ClipInto(CubeT<float> theIntoCube) {float aX1=(float)mX;float aY1=(float)mY;float aZ1=(float)mZ;float aX2=(float)(mX+mXSize);float aY2=(float)(mY+mYSize);float aZ2=(float)(mZ+mZSize);if (aX1<theIntoCube.mX) aX1=(float)theIntoCube.mX;if (aX2<theIntoCube.mX) aX2=(float)theIntoCube.mX;if (aY1<theIntoCube.mY) aY1=(float)theIntoCube.mY;if (aY2<theIntoCube.mY) aY2=(float)theIntoCube.mY;if (aZ1<theIntoCube.mZ) aZ1=(float)theIntoCube.mZ;if (aZ2<theIntoCube.mZ) aZ2=(float)theIntoCube.mZ;if (aX1>(theIntoCube.mX+theIntoCube.mXSize)) aX1=(float)(theIntoCube.mX+theIntoCube.mXSize);if (aX2>(theIntoCube.mX+theIntoCube.mXSize)) aX2=(float)(theIntoCube.mX+theIntoCube.mXSize);if (aY1>(theIntoCube.mY+theIntoCube.mYSize)) aY1=(float)(theIntoCube.mY+theIntoCube.mYSize);if (aY2>(theIntoCube.mY+theIntoCube.mYSize)) aY2=(float)(theIntoCube.mY+theIntoCube.mYSize);if (aZ1>(theIntoCube.mZ+theIntoCube.mZSize)) aZ1=(float)(theIntoCube.mZ+theIntoCube.mZSize);if (aZ2>(theIntoCube.mZ+theIntoCube.mZSize)) aZ2=(float)(theIntoCube.mZ+theIntoCube.mZSize);mX=(var_type)aX1;mY=(var_type)aY1;mZ=(var_type)aY1;mXSize=(var_type)(aX2-aX1);mYSize=(var_type)(aY2-aY1);mZSize=(var_type)(aZ2-aZ1);}
	void				ClipInto(CubeT<int> theIntoCube) {float aX1=(float)mX;float aY1=(float)mY;float aZ1=(float)mZ;float aX2=(float)(mX+mXSize);float aY2=(float)(mY+mYSize);float aZ2=(float)(mZ+mZSize);if (aX1<theIntoCube.mX) aX1=(float)theIntoCube.mX;if (aX2<theIntoCube.mX) aX2=(float)theIntoCube.mX;if (aY1<theIntoCube.mY) aY1=(float)theIntoCube.mY;if (aY2<theIntoCube.mY) aY2=(float)theIntoCube.mY;if (aZ1<theIntoCube.mZ) aZ1=(float)theIntoCube.mZ;if (aZ2<theIntoCube.mZ) aZ2=(float)theIntoCube.mZ;if (aX1>(theIntoCube.mX+theIntoCube.mXSize)) aX1=(float)(theIntoCube.mX+theIntoCube.mXSize);if (aX2>(theIntoCube.mX+theIntoCube.mXSize)) aX2=(float)(theIntoCube.mX+theIntoCube.mXSize);if (aY1>(theIntoCube.mY+theIntoCube.mYSize)) aY1=(float)(theIntoCube.mY+theIntoCube.mYSize);if (aY2>(theIntoCube.mY+theIntoCube.mYSize)) aY2=(float)(theIntoCube.mY+theIntoCube.mYSize);if (aZ1>(theIntoCube.mZ+theIntoCube.mZSize)) aZ1=(float)(theIntoCube.mZ+theIntoCube.mZSize);if (aZ2>(theIntoCube.mZ+theIntoCube.mZSize)) aZ2=(float)(theIntoCube.mZ+theIntoCube.mZSize);mX=(var_type)aX1;mY=(var_type)aY1;mZ=(var_type)aY1;mXSize=(var_type)(aX2-aX1);mYSize=(var_type)(aY2-aY1);mZSize=(var_type)(aZ2-aZ1);}
	void				CenterInto(CubeT<float> theIntoCube) {mX=(var_type)(theIntoCube.Center().mX-((float)mXSize/2));mY=(var_type)(theIntoCube.Center().mY-((float)mYSize/2));mZ=(var_type)(theIntoCube.Center().mZ-((float)mZSize/2));}
	void				CenterInto(CubeT<int> theIntoCube) {mX=(var_type)((float)theIntoCube.Center().mX-((float)mXSize/2));mY=(var_type)((float)theIntoCube.Center().mY-((float)mYSize/2));mZ=(var_type)((float)theIntoCube.Center().mZ-((float)mZSize/2));}
	inline void			CenterAt(float theX, float theY, float theZ) {mX=(var_type)(theX-((float)mXSize/2));mY=(var_type)(theY-((float)mYSize/2));mZ=(var_type)(theZ-((float)mZSize/2));}
	inline void			CenterAt(Vector thePos) {CenterAt(thePos.mX,thePos.mY,thePos.mZ);}

	inline void			CenterXAt(float theX) {mX=(var_type)(theX-((float)mXSize/2));}
	inline void			CenterYAt(float theY) {mY=(var_type)(theY-((float)mYSize/2));}
	inline void			CenterZAt(float theZ) {mZ=(var_type)(theZ-((float)mZSize/2));}

	CubeT<var_type>		GetCenterAt(float theX, float theY, float theZ) {return CubeT<var_type>((var_type)(theX-(mXSize/2)),(var_type)(theY-(mYSize/2)),(var_type)(theZ-(mZSize/2)),(var_type)mXSize,(var_type)mYSize,(var_type)mZSize);}
	CubeT<var_type>		GetCenterAt(Vector thePos) {return GetCenterAt(thePos.mX,thePos.mY,thePos.mZ);}

	void				Confine(CubeT<var_type> theIntoCube) {if (mX<theIntoCube.mX) mX=theIntoCube.mX;if (mY<theIntoCube.mY) mY=theIntoCube.mY;if (mZ<theIntoCube.mZ) mZ=theIntoCube.mZ;if (mX+mXSize>=theIntoCube.mX+theIntoCube.mXSize) mX=(theIntoCube.mX+theIntoCube.mXSize)-mXSize;if (mY+mYSize>=theIntoCube.mY+theIntoCube.mYSize) mY=(theIntoCube.mY+theIntoCube.mYSize)-mYSize;if (mZ+mZSize>=theIntoCube.mZ+theIntoCube.mZSize) mZ=(theIntoCube.mZ+theIntoCube.mZSize)-mZSize;}
	CubeT<var_type>		GetClipInto(CubeT<var_type> theIntoCube) {var_type aX1=mX;var_type aY1=mY;var_type aZ1=mZ;var_type aX2=(mX+mXSize);var_type aY2=(mY+mYSize);var_type aZ2=(mZ+mZSize);if (aX1<theIntoCube.mX) aX1=theIntoCube.mX;if (aX2<theIntoCube.mX) aX2=theIntoCube.mX;if (aY1<theIntoCube.mY) aY1=theIntoCube.mY;if (aY2<theIntoCube.mY) aY2=theIntoCube.mY;if (aZ1<theIntoCube.mZ) aZ1=theIntoCube.mZ;if (aZ2<theIntoCube.mZ) aZ2=theIntoCube.mZ;if (aX1>(theIntoCube.mX+theIntoCube.mXSize)) aX1=(theIntoCube.mX+theIntoCube.mXSize);if (aX2>(theIntoCube.mX+theIntoCube.mXSize)) aX2=(theIntoCube.mX+theIntoCube.mXSize);if (aY1>(theIntoCube.mY+theIntoCube.mYSize)) aY1=(theIntoCube.mY+theIntoCube.mYSize);if (aY2>(theIntoCube.mY+theIntoCube.mYSize)) aY2=(theIntoCube.mY+theIntoCube.mYSize);if (aZ1>(theIntoCube.mZ+theIntoCube.mZSize)) aZ1=(theIntoCube.mZ+theIntoCube.mZSize);if (aZ2>(theIntoCube.mZ+theIntoCube.mZSize)) aZ2=(theIntoCube.mZ+theIntoCube.mZSize);return CubeT<var_type>((var_type)aX1,(var_type)aY1,(var_type)aZ1,(var_type)(aX2-aX1),(var_type)(aY2-aY1),(var_type)(aZ2-aZ1));}

	//
	// Getting some results from our rect...
	//
	Vector				ClampPoint(Vector thePos) {thePos.mX=_max(mX,thePos.mX);thePos.mX=_min(mX+mXSize,thePos.mX);thePos.mY=_max(mY,thePos.mY);thePos.mY=_min(mY+mYSize,thePos.mY);thePos.mZ=_max(mZ,thePos.mZ);thePos.mZ=_min(mZ+mZSize,thePos.mZ);return thePos;}

	inline Vector		Center() {return Vector(mX+(mXSize/2),mY+(mYSize/2),mZ+(mZSize/2));}
	inline IVector		CenterI() {return IVector(mX+(mXSize/2),mY+(mYSize/2),mZ+(mZSize/2));}
	inline float		CenterX() {return (float)(mX+(mXSize/2));}
	inline float		CenterY() {return (float)(mY+(mYSize/2));}
	inline float		CenterZ() {return (float)(mZ+(mZSize/2));}
	inline var_type		MaxX() {return mX+mXSize;}
	inline var_type		MaxY() {return mY+mYSize;}
	inline var_type		MaxZ() {return mZ+mZSize;}
	inline var_type		MinX() {return mX;}
	inline var_type		MinY() {return mY;}
	inline var_type		MinZ() {return mZ;}
	inline var_type		MaxAxis() {return _max(_max(mXSize,mYSize),mZSize);}
	inline var_type		MajorAxis() {return _max(_max(mXSize,mYSize),mZSize);}
	inline var_type		MinAxis() {return _min(_min(mXSize,mYSize),mZSize);}
	inline var_type		MinoAxis() {return _min(_min(mXSize,mYSize),mZSize);}

	inline Vector		Corner(int theCorner) {switch (theCorner) {case 0:return Vector(mX,mY,mZ);case 1:return Vector(mX+mXSize,mY,mZ);case 2:return Vector(mX,mY+mYSize,mZ);case 3:return Vector(mX+mXSize,mY+mYSize,mZ);case 4:return Vector(mX,mY,mZ+mZSize);case 5:return Vector(mX+mXSize,mY,mZ+mZSize);case 6:return Vector(mX,mY+mYSize,mZ+mZSize);case 7:return Vector(mX+mXSize,mY+mYSize,mZ+mZSize);}return Center();}
	inline IVector		CornerI(int theCorner) {switch (theCorner) {case 0:return IVector(mX,mY,mZ);case 1:return IVector(mX+mXSize,mY,mZ);case 2:return IVector(mX,mY+mYSize,mZ);case 3:return IVector(mX+mXSize,mY+mYSize,mZ);case 4:return IVector(mX,mY,mZ+mZSize);case 5:return IVector(mX+mXSize,mY,mZ+mZSize);case 6:return IVector(mX,mY+mYSize,mZ+mZSize);case 7:return IVector(mX+mXSize,mY+mYSize,mZ+mZSize);}return CenterI();}
	inline Vector		CenterTop() {return (Corner(0)+Corner(1)+Corner(2)+Corner(3))/4;}
	inline Vector		CenterBottom() {return (Corner(4)+Corner(5)+Corner(6)+Corner(7))/4;}

	CubeT<var_type>		Expand(float theAmount) {return CubeT<var_type>(mX-theAmount,mY-theAmount,mZ-theAmount,mXSize+(theAmount*2),mYSize+(theAmount*2),mZSize+(theAmount*2));}
	CubeT<var_type>		Expand(int theAmount) {return CubeT<var_type>(mX-theAmount,mY-theAmount,mZ-theAmount,mXSize+(theAmount*2),mYSize+(theAmount*2),mZSize+(theAmount*2));}
	CubeT<var_type>		Expand(float theAmountX, float theAmountY, float theAmountZ) {return CubeT<var_type>((var_type)(mX-theAmountX),(var_type)(mY-theAmountY),(var_type)(mZ-theAmountZ),(var_type)(mXSize+(theAmountX*2)),(var_type)(mYSize+(theAmountY*2)),(var_type)(mZSize+(theAmountZ*2)));}
	CubeT<var_type>		Expand(int theAmountX, int theAmountY, int theAmountZ) {return CubeT<var_type>((var_type)(mX-theAmountX),(var_type)(mY-theAmountY),(var_type)(mZ-theAmountZ),(var_type)(mXSize+(theAmountX*2)),(var_type)(mYSize+(theAmountY*2)),(var_type)(mZSize+(theAmountZ*2)));}
	CubeT<var_type>		Expand(Vector theSize) {return Expand(theSize.mX,theSize.mY,theSize.mZ);}
	CubeT<var_type>		Expand(IVector theSize) {return Expand(theSize.mX,theSize.mY,theSize.mZ);}
	CubeT<var_type>		ExpandX(float theAmount) {return CubeT<var_type>((var_type)(mX-theAmount),(var_type)mY,(var_type)mZ,(var_type)(mXSize+(theAmount*2)),(var_type)mYSize,(var_type)mZSize);}
	CubeT<var_type>		ExpandX(int theAmount) {return CubeT<var_type>((var_type)(mX-theAmount),(var_type)mY,(var_type)mZ,(var_type)(mXSize+(theAmount*2)),(var_type)mYSize,(var_type)mZSize);}
	CubeT<var_type>		ExpandY(float theAmount) {return CubeT<var_type>((var_type)mX,(var_type)(mY-theAmount),(var_type)mZ,(var_type)mXSize,(var_type)(mYSize+(theAmount*2)),(var_type)mZSize);}
	CubeT<var_type>		ExpandY(int theAmount) {return CubeT<var_type>((var_type)mX,(var_type)(mY-theAmount),(var_type)mZ,(var_type)mXSize,(var_type)(mYSize+(theAmount*2)),(var_type)mZSize);}
	CubeT<var_type>		ExpandZ(float theAmount) {return CubeT<var_type>((var_type)mX,(var_type)mY,(var_type)(mZ-theAmount),(var_type)mXSize,(var_type)mYSize,(var_type)(mZSize+(theAmount*2)));}
	CubeT<var_type>		ExpandZ(int theAmount) {return CubeT<var_type>((var_type)mX,(var_type)mY,(var_type)(mZ-theAmount),(var_type)mXSize,(var_type)mYSize,(var_type)(mZSize+(theAmount*2)));}
	CubeT<var_type>		Contract(float theAmount) {return Expand(-theAmount);}
	CubeT<var_type>		Contract(int theAmount) {return Expand(-theAmount);}
	inline bool	operator==(const CubeT<var_type> &theCube) {return (mX==theCube.mX)&(mY==theCube.mY)&(mZ==theCube.mZ)&(mXSize==theCube.mXSize)&(mYSize==theCube.mYSize)&(mZSize==theCube.mZSize);}
	inline bool	operator!=(const CubeT<var_type> &theCube) {return (mX!=theCube.mX)|(mY!=theCube.mY)|(mZ!=theCube.mZ)|(mXSize!=theCube.mXSize)|(mYSize!=theCube.mYSize)|(mZSize!=theCube.mZSize);}
	inline CubeT<var_type> &operator=(const CubeT<var_type> &theCube) {if (this!=&theCube) {mX=theCube.mX;mY=theCube.mY;mZ=theCube.mZ;mXSize=theCube.mXSize;mYSize=theCube.mYSize;mZSize=theCube.mZSize;}return *this;}

	CubeT<var_type>		ExpandPercent(float theAmount) {Vector aCenter=Center();Vector aUpperLeft=Corner(0)-Center();Vector aLowerRight=Corner(7)-Center();aUpperLeft*=theAmount;aLowerRight*=theAmount;aUpperLeft+=Center();aLowerRight+=Center();return CubeT<var_type>(aUpperLeft,aLowerRight);}

	inline var_type		XSize() {return mXSize;}
	inline var_type		YSize() {return mYSize;}
	inline var_type		ZSize() {return mZSize;}
	inline var_type		HalfXSize() {return mXSize/2;}
	inline var_type		HalfYSize() {return mYSize/2;}
	inline var_type		HalfZSize() {return mZSize/2;}
	inline Vector		HalfSize() {return Vector((float)mXSize/2,(float)mYSize/2,(float)mZSize/2);}

	CubeT<var_type>		Translate(Vector thePoint) {return CubeT<var_type>((var_type)(mX+thePoint.mX),(var_type)(mY+thePoint.mY),(var_type)(mZ+thePoint.mZ),(var_type)mXSize,(var_type)mYSize,(var_type)mZSize);}
	CubeT<var_type>		Translate(IVector thePoint) {return CubeT<var_type>((var_type)(mX+thePoint.mX),(var_type)(mY+thePoint.mY),(var_type)(mZ+thePoint.mZ),(var_type)mXSize,(var_type)mYSize,(var_type)mZSize);}
	CubeT<var_type>		Translate(float theX, float theY, float theZ) {return CubeT<var_type>((var_type)(mX+theX),(var_type)(mY+theY),(var_type)(mZ+theZ),(var_type)mXSize,(var_type)mYSize,(var_type)mZSize);}
	CubeT<var_type>		Translate(int theX, int theY, int theZ) {return CubeT<var_type>((var_type)(mX+theX),(var_type)(mY+theY),(var_type)(mZ+theZ),(var_type)mXSize,(var_type)mYSize,(var_type)mZSize);}

	inline float		Area() {return (float)(mXSize*mYSize*mZSize);}
	inline int			AreaI() {return (int)(mXSize*mYSize*mZSize);}
	inline float		GetArea() {return Area();}

	Vector				ClosestPointTo(Vector theCamera) {if (ContainsPoint(theCamera)) return theCamera;if (theCamera.mX<mX) theCamera.mX=(float)mX;else if (theCamera.mX>mX+mXSize) theCamera.mX=(float)(mX+mXSize);if (theCamera.mY<mY) theCamera.mY=(float)mY;else if (theCamera.mY>mY+mYSize) theCamera.mY=(float)(mY+mYSize);if (theCamera.mZ<mZ) theCamera.mZ=(float)mZ;else if (theCamera.mZ>mZ+mZSize) theCamera.mZ=(float)(mZ+mZSize);return theCamera;}

	inline Vector		InterpolateInto(Vector thePos) {return Vector(RINTERPOLATE(mX,thePos.mX,mX+mXSize),RINTERPOLATE(mY,thePos.mY,mY+mYSize),RINTERPOLATE(mZ,thePos.mZ,mZ+mZSize));}

	inline Rect			ToRect() {return Rect((float)mX,(float)mY,(float)mXSize,(float)mYSize);}
	Plane				GetPlane(int theNo)
	{
		int aSide[4];
		switch (theNo)
		{
		case 0:aSide[0]=0;aSide[1]=1;aSide[2]=2;aSide[3]=3;break; // Z-
		case 1:aSide[0]=1;aSide[1]=0;aSide[2]=0+4;aSide[3]=1+4;break; // Y-
		case 2:aSide[0]=3;aSide[1]=1;aSide[2]=3+4;aSide[3]=1+4;break; // X+
		case 3:aSide[0]=2;aSide[1]=3;aSide[2]=3+4;aSide[3]=2+4;break; // Y+
		case 4:aSide[0]=0;aSide[1]=2;aSide[2]=0+4;aSide[3]=2+4;break; // X-
		case 5:aSide[0]=1+4;aSide[1]=0+4;aSide[2]=3+4;aSide[3]=2+4;break; // Z+
		}

		Vector aNorm;
		Vector aVec1=GetCorner(aSide[1])-GetCorner(aSide[0]);
		Vector aVec2=GetCorner(aSide[2])-GetCorner(aSide[0]);
		aNorm=-aVec1.Cross(aVec2);
		aNorm.Normalize();
		return Plane(GetMid(aSide[0],aSide[1],aSide[2],aSide[3]),aNorm);

		//return Plane(GetMid(aSide[0],aSide[1],aSide[2],aSide[3]),gMath.GetTriangleNormal(GetCorner(aSide[0]),GetCorner(aSide[1]),GetCorner(aSide[2])));
	}

	inline Vector		GetCorner(int theCorner)
	{
		switch (theCorner)
		{
		case 0:return Vector(MinX(),MinY(),MinZ());
		case 1:return Vector(MaxX(),MinY(),MinZ());
		case 2:return Vector(MinX(),MaxY(),MinZ());
		case 3:return Vector(MaxX(),MaxY(),MinZ());
		case 4:return Vector(MinX(),MinY(),MaxZ());
		case 5:return Vector(MaxX(),MinY(),MaxZ());
		case 6:return Vector(MinX(),MaxY(),MaxZ());
		case 7:return Vector(MaxX(),MaxY(),MaxZ());
		}
		return Vector(0,0,0);
	}
	inline Vector		GetMidpoint(int theCorner1, int theCorner2)	{return (GetCorner(theCorner1)+GetCorner(theCorner2))/2;}
	inline Vector		GetMid(int theCorner1, int theCorner2)	{return (GetCorner(theCorner1)+GetCorner(theCorner2))/2;}
	inline Vector		GetMid(int theCorner1, int theCorner2, int theCorner3)	{return (GetCorner(theCorner1)+GetCorner(theCorner2)+GetCorner(theCorner3))/3;}
	inline Vector		GetMid(int theCorner1, int theCorner2, int theCorner3, int theCorner4)	{return (GetCorner(theCorner1)+GetCorner(theCorner2)+GetCorner(theCorner3)+GetCorner(theCorner4))/4;}
	inline Vector		GetCenter() {return Vector(mX+mXSize/2,mY+mYSize/2,mZ+mZSize/2);}

	inline float		GetLength(int theCorner1, int theCorner2) {Vector aV=GetCorner(theCorner2)-GetCorner(theCorner1);return aV.Length();}
	inline float		GetLen(int theCorner1, int theCorner2) {return GetLength(theCorner1, theCorner2);}
	inline float		Length(int theCorner1, int theCorner2) {return GetLength(theCorner1, theCorner2);}
	inline float		Len(int theCorner1, int theCorner2) {return GetLength(theCorner1, theCorner2);}

	inline Vector		GetMin() {return Vector(mX,mY,mZ);}
	inline Vector		GetMax() {return Vector(mX+mXSize,mY+mYSize,mZ+mZSize);}
	inline Vector		LowerBound() {return GetMin();}
	inline Vector		UpperBound() {return GetMax();}

	bool				PrettyClose(const CubeT<var_type> &theCube) {return (::PrettyClose(mX,theCube.mX) && ::PrettyClose(mY,theCube.mY) && ::PrettyClose(mZ,theCube.mZ) && ::PrettyClose(mXSize,theCube.mXSize) && ::PrettyClose(mYSize,theCube.mYSize) && ::PrettyClose(mZSize,theCube.mZSize));}
	inline void			ToArray(Array<Vector>& theArray) {theArray.Reset();theArray.GuaranteeSize(8);for (int aCount=0;aCount<8;aCount++) theArray[aCount]=GetCorner(aCount);}
	//longlong			Hash() {return Utils::Hash(&mX,(sizeof(var_type)*6)/sizeof(int));}
	inline float		GetPerimeter() {return 2.0f * ((mXSize) + (mYSize) + (mZSize));}

	CubeT<var_type>		Multiply(Matrix& theMat) {Array<Vector> aCorners;ToArray(aCorners);theMat.ProcessPoints(aCorners);CubeT<var_type> aResult;for (int aCount=0;aCount<aCorners.Size();aCount++) aResult=aResult.Union(aCorners[aCount]);return aResult;}

	void				Maximize() 
	{
		Vector aCenter=Center();
		var_type aMax=_max(mZSize,_max(mXSize,mYSize));
		mXSize=mYSize=mZSize=(var_type)aMax;
		CenterAt(aCenter);
	}
	inline void			MakeSquare() {Maximize();}

	CubeT<var_type>		GetMaximize() // Return a "square" cube with all sizes = the maximum size
	{
		CubeT<var_type> aResult;
		var_type aMax=_max(mZSize,_max(mXSize,mYSize));
		aResult.mXSize=aResult.mYSize=aResult.mZSize=(var_type)aMax;
		aResult.CenterAt(Center());
		return aResult;
	}

	static CubeT<var_type>		MakeBox(Vector theCenter, float theSize) {float aS2=theSize/2;return CubeT<var_type>(theCenter.mX-aS2,theCenter.mY-aS2,theCenter.mZ-aS2,theSize,theSize,theSize);}
	static CubeT<var_type>		MakeCube(Vector theCenter, float theX, float theY, float theZ) {return CubeT<var_type>(theCenter.mX-(theX/2),theCenter.mY-(theY/2),theCenter.mZ-(theZ/2),theX,theY,theZ);}

	CubeT<var_type>				GetScaled(var_type theScale, Vector theCenter)
	{
		CubeT<var_type> aResult=GetCenterAt(0);
		aResult.mX*=theScale;
		aResult.mY*=theScale;
		aResult.mZ*=theScale;
		aResult.mXSize*=theScale;
		aResult.mYSize*=theScale;
		aResult.mZSize*=theScale;
		return aResult.GetCenterAt(theCenter);
	}

	inline CubeT<var_type>				GetScaled(var_type theScale)
	{
		CubeT<var_type> aResult=GetCenterAt(0);
		aResult.mX*=theScale;
		aResult.mY*=theScale;
		aResult.mZ*=theScale;
		aResult.mXSize*=theScale;
		aResult.mYSize*=theScale;
		aResult.mZSize*=theScale;
		return aResult.GetCenterAt(GetCenter());
	}

public:
	var_type			mX;
	var_type			mY;
	var_type			mZ;
	var_type			mXSize;
	var_type			mYSize;
	var_type			mZSize;
};

typedef CubeT<float> Cube;
typedef CubeT<int> ICube;

class AABBCube
{
public:
	AABBCube(void) {}
	AABBCube(float theX1, float theY1, float theZ1, float theX2, float theY2, float theZ2) {mX1=theX1;mY1=theY1;mZ1=theZ1;mX2=theX2;mY2=theY2;mZ2=theZ2;}
	AABBCube(Vector theP1, Vector theP2)
	{
		Vector aMin=VectorMin(theP1,theP2);
		Vector aMax=VectorMax(theP1,theP2);
		mX1=aMin.mX;
		mY1=aMin.mY;
		mZ1=aMin.mZ;
		mX2=aMax.mX;
		mY2=aMax.mY;
		mZ2=aMax.mZ;

	}
	AABBCube(Cube theCube)
	{
		Vector aMin=VectorMin(theCube.Corner(0),theCube.Corner(7));
		Vector aMax=VectorMax(theCube.Corner(0),theCube.Corner(7));
		mX1=aMin.mX;
		mY1=aMin.mY;
		mZ1=aMin.mZ;
		mX2=aMax.mX;
		mY2=aMax.mY;
		mZ2=aMax.mZ;
	}
	/*
	AABBCube(AABBCube& theCube)
	{
		mMin=theCube.mMin;
		mMax=theCube.mMax;
	}
	*/
	AABBCube(float theX, float theY, float theZ) {Size(theX,theY,theZ);}


	void				Resize(float theX1, float theY1, float theZ1, float theX2, float theY2, float theZ2);
	void				SetSize(float theX1, float theY1, float theZ1, float theX2, float theY2, float theZ2) {Resize(theX1,theY1,theZ1,theX2,theY2,theZ2);}
	void				Size(float theX1, float theY1, float theZ1, float theX2, float theY2, float theZ2) {Resize(theX1,theY1,theZ1,theX2,theY2,theZ2);}
	void				Size(Cube theCube) {Resize(theCube.mX,theCube.mY,theCube.mZ,theCube.mX+theCube.mXSize,theCube.mY+theCube.mYSize,theCube.mZ+theCube.mZSize);}
	inline void			Resize(float theXSize, float theYSize, float theZSize) {Resize(-(theXSize/2),-(theYSize/2),-(theZSize/2),theXSize/2,theYSize/2,theZSize/2);}
	inline void			Size(float theXSize, float theYSize, float theZSize) {Resize(-(theXSize/2),-(theYSize/2),-(theZSize/2),theXSize/2,theYSize/2,theZSize/2);}
	char*				ToString();
	Cube				ToCube() {return Cube(mX1,mY1,mZ1,(mX2-mX1),(mY2-mY1),(mZ2-mZ1));}

	inline void			SizeFromCenter(Vector theCenter, Vector theSize) {mX1=theCenter.mX-(theSize.mX);mY1=theCenter.mY-(theSize.mY);mZ1=theCenter.mZ-(theSize.mZ);mX2=theCenter.mX+(theSize.mX);mY2=theCenter.mY+(theSize.mY);mZ2=theCenter.mZ+(theSize.mZ);}

	inline bool			ContainsPoint(float theX, float theY, float theZ) {return (theX>=mX1 && theY>=mY1 && theZ>=mZ1 && theX<=(mX2) && theY<=(mY2) && theZ<=(mZ2));}
	inline bool			ContainsPoint(Vector thePoint) {return ContainsPoint(thePoint.mX,thePoint.mY,thePoint.mZ);}

	inline void			CenterAt(float theX, float theY, float theZ) {float aW=Width()/2;float aH=Height()/2;float aD=Depth()/2;mX1=theX-aW;mX2=theX+aW;mY1=theY-aH;mY2=theY+aH;mZ1=theZ-aD;mZ2=theZ+aD;}
	inline void			CenterAt(Vector thePos) {CenterAt(thePos.mX,thePos.mY,thePos.mZ);}
	inline bool			Contains(AABBCube theCube) const
	{
		bool aResult=true;
		aResult=aResult && (mX1<=theCube.mX1);
		aResult=aResult && (mY1<=theCube.mY1);
		aResult=aResult && (mZ1<=theCube.mZ1);
		aResult=aResult && (theCube.mX2<=mX2);
		aResult=aResult && (theCube.mY2<=mY2);
		aResult=aResult && (theCube.mZ2<=mZ2);
		return aResult;
	}
	inline bool			Contains(CubeT<float> theCube) const
	{
		bool aResult=true;
		aResult=aResult && (mX1<=theCube.mX);
		aResult=aResult && (mY1<=theCube.mY);
		aResult=aResult && (mZ1<=theCube.mZ);
		aResult=aResult && (theCube.mX+theCube.mXSize<=mX2);
		aResult=aResult && (theCube.mY+theCube.mYSize<=mY2);
		aResult=aResult && (theCube.mZ+theCube.mZSize<=mZ2);
		return aResult;
	}


	inline bool			Intersects(float theX1, float theY1, float theZ1, float theX2, float theY2, float theZ2) {return !((theX2<mX1) || (theY2<mY1) || (theZ2<mZ1) || (theX1>mX2) || (theY1>mY2) || (theZ1>mZ2));}
	inline bool			Intersects(AABBCube theCube) {return Intersects(theCube.mX1,theCube.mY1,theCube.mZ1,theCube.mX2,theCube.mY2,theCube.mZ2);}
	AABBCube			GetIntersection(AABBCube &theCube);

	bool				IsTouching(float theX1, float theY1, float theZ1, float theX2, float theY2, float theZ2);
	inline bool			IsTouching(AABBCube theCube) {return IsTouching(theCube.mX1,theCube.mY1,theCube.mZ1,theCube.mX2,theCube.mY2,theCube.mZ2);}
	float				GetPerimeter() {return 2.0f * ((mX2-mX1) + (mY2-mY1) + (mZ2-mZ1));}


	AABBCube			Union(AABBCube theCube);
	AABBCube			Union(Vector thePos) 
	{
		return Union(AABBCube(thePos.mX,thePos.mY,thePos.mZ,thePos.mX,thePos.mY,thePos.mZ));
	}
	inline AABBCube		Union(float theX1, float theY1, float theZ1, float theX2, float theY2, float theZ2) {return Union(AABBCube(theX1,theY1,theZ1,theX2,theY2,theZ2));}

	inline Vector		Center() {return Vector((mX1+mX2)/2,(mY1+mY2)/2,(mZ1+mZ2)/2);}
	inline float		CenterX() {return (mX1+mX2)/2;}
	inline float		CenterY() {return (mY1+mY2)/2;}
	inline float		CenterZ() {return (mZ1+mZ2)/2;}
	inline Vector		HalfSize() {return Vector((mX2-mX1)/2,(mY2-mY1)/2,(mZ2-mZ1)/2);}

	inline Vector		LowerBound() {return Vector(mX1,mY1,mZ1);}
	inline Vector		UpperBound() {return Vector(mX2,mY2,mZ2);}
	inline void			SetLowerBound(Vector thePos) {mX1=thePos.mX;mY1=thePos.mY;mZ1=thePos.mZ;}
	inline void			SetUpperBound(Vector thePos) {mX2=thePos.mX;mY2=thePos.mY;mZ2=thePos.mZ;}

	inline AABBCube		Expand(float theAmount) {return AABBCube(mX1-theAmount,mY1-theAmount,mZ1-theAmount,mX2+theAmount,mY2+theAmount,mZ2+theAmount);}
	inline AABBCube		ExpandX(float theAmount) {return AABBCube(mX1,mY1-theAmount,mZ1,mX2,mY2+(theAmount),mZ2);}
	inline AABBCube		ExpandY(float theAmount) {return AABBCube(mX1-theAmount,mY1,mZ1,mX2+(theAmount),mY2,mZ2);}
	inline AABBCube		ExpandZ(float theAmount) {return AABBCube(mX1,mY1,mZ1-theAmount,mX2,mY2,mZ2+theAmount);}
	inline AABBCube		Contract(float theAmount) {return Expand(-theAmount);}
	inline AABBCube		&operator=(const AABBCube &theCube) {if (this!=&theCube) {mX1=theCube.mX1;mY1=theCube.mY1;mZ1=theCube.mZ1;mX2=theCube.mX2;mY2=theCube.mY2;mZ2=theCube.mZ2;}return *this;}

	inline float		Width() {return (mX2-mX1);}
	inline float		Height() {return (mY2-mY1);}
	inline float		Depth() {return (mZ2-mZ1);}
	inline Vector		GetSize() {return Vector((mX2-mX1),(mY2-mY1),(mZ2-mZ1));}


	inline AABBCube		Translate(Vector theDir) {return AABBCube(mX1+theDir.mX,mY1+theDir.mY,mZ1+theDir.mZ,mX2+theDir.mX,mY2+theDir.mY,mZ2+theDir.mZ);}
	inline AABBCube		Translate(float theX, float theY, float theZ) {return AABBCube(mX1+theX,mY1+theY,mZ1+theZ,mX2+theX,mY2+theY,mZ2+theZ);}


	inline void Combine(const AABBCube& aabb)
	{
		mX1=_min(mX1,aabb.mX1);
		mY1=_min(mY1,aabb.mY1);
		mZ1=_min(mZ1,aabb.mZ1);
		mX2=_max(mX2,aabb.mX2);
		mY2=_max(mY2,aabb.mY2);
		mZ2=_max(mZ2,aabb.mZ2);
	}

	inline void Combine(const AABBCube& aabb1, const AABBCube& aabb2)
	{
		mX1=_min(aabb1.mX1,aabb2.mX1);
		mY1=_min(aabb1.mY1,aabb2.mY1);
		mZ1=_min(aabb1.mZ1,aabb2.mZ1);
		mX2=_max(aabb1.mX2,aabb2.mX2);
		mY2=_max(aabb1.mY2,aabb2.mY2);
		mZ2=_max(aabb1.mZ2,aabb2.mZ2);
	}

	float			mX1;
	float			mY1;
	float			mZ1;

	float			mX2;
	float			mY2;
	float			mZ2;

	Vector			 Min() {return Vector(mX1,mY1,mZ1);}
	Vector			 Max() {return Vector(mX2,mY2,mZ2);}


/*
public:
	union
	{
		Vector				mMin;
		struct
		{
			float			mX1;
			float			mY1;
			float			mZ1;
		};
	};
	union
	{
		Vector				mMax;
		struct
		{
			float			mX2;
			float			mY2;
			float			mZ2;
		};
	};
*/
};


enum
{
	FRUSTRUM_RIGHT=0,
	FRUSTRUM_LEFT,
	FRUSTRUM_NEAR,
	FRUSTRUM_FAR,
	FRUSTRUM_TOP,
	FRUSTRUM_BOTTOM,
};

class Triangle3D
{
public:
	Triangle3D()
	{
	}

	Triangle3D(Vector thePos1, Vector thePos2, Vector thePos3)
	{
		mPos[0]=thePos1;
		mPos[1]=thePos2;
		mPos[2]=thePos3;
	}

	inline Vector &operator[](int theIndex) {return mPos[_min(2,_max(0,theIndex))];}



	Vector					GetVertex(int theNumber) {return mPos[theNumber];}
	Vector					GetCentroid() {return (mPos[0]+mPos[1]+mPos[2])/3;}
	Vector					GetIncenter()
	{
		Vector aAB=mPos[0]-mPos[1];
		Vector aBC=mPos[1]-mPos[2];
		Vector aCA=mPos[2]-mPos[0];
		float aA=aBC.Length();
		float aB=aCA.Length();
		float aC=aAB.Length();
		float aSum=aA+aB+aC;

		return Vector(
			(aA*mPos[0].mX+aB*mPos[1].mX+aC*mPos[2].mX)/aSum,
			(aA*mPos[0].mY+aB*mPos[1].mY+aC*mPos[2].mY)/aSum,
			(aA*mPos[0].mZ+aB*mPos[1].mZ+aC*mPos[2].mZ)/aSum);
	}
	Vector					GetOrthoCenter();
	Cube					GetBoundingCube();
	Rect					GetBoundingXYRect();
	AABBRect				GetBoundingXYRectAABB();

	void					GetLegs(Array<Line3D>& theLegArray, bool isCW=true);
	Line3D					GetHypotenuse();
	inline Line3D			GetLine(int theNumber) {return Line3D(mPos[theNumber%3],mPos[(theNumber+1)%3]);}

	inline Vector			GetNormal() {return Normal();}
	Vector Normal()
	{
		Vector aResult;
		Vector aVec1=mPos[1]-mPos[0];
		Vector aVec2=mPos[2]-mPos[0];
		aResult=-aVec1.Cross(aVec2);
		aResult.Normalize();
		return aResult;
	}

	//
	// For getting an unnormalized normal (for speed reasons)
	//
	inline Vector			GetNormalDir() {return NormalDir();}
	Vector NormalDir()
	{
		Vector aResult;
		Vector aVec1=mPos[1]-mPos[0];
		Vector aVec2=mPos[2]-mPos[0];
		aResult=-aVec1.Cross(aVec2);
		//aResult.Normalize();
		return aResult;
	}

	Vector GetPointOnTriangle(float theOff1, float theOff2) // For like UV aspects
	{
		Vector aAB=mPos[1]-mPos[0];
		Vector aAC=mPos[2]-mPos[0];

		if (theOff1+theOff2>=1.0f)
		{
			theOff1=1-theOff1;
			theOff2=1-theOff2;
		}
		return mPos[0]+(theOff1*aAB)+theOff2*(aAC);
	}

	bool IsDegenerate();
	void Expand(float theAmount);
	bool SamePoints(Triangle3D& theOther);

public:
	Vector					mPos[3];
};

class Triangle3DNormal : public Triangle3D
{
public:
	Vector					mNormal;

	Triangle3DNormal()
	{
	}

	Triangle3DNormal(Vector thePos1, Vector thePos2, Vector thePos3, Vector theNormal)
	{
		mPos[0]=thePos1;
		mPos[1]=thePos2;
		mPos[2]=thePos3;
		mNormal=theNormal;
	}

};


class Triangle3DPtr
{
public:
	Triangle3DPtr()
	{
	}

	Triangle3DPtr(Vector* thePos1, Vector* thePos2, Vector* thePos3)
	{
		mPos[0]=thePos1;
		mPos[1]=thePos2;
		mPos[2]=thePos3;
	}

	inline Vector& operator[](int theIndex) {return *mPos[_min(2,_max(0,theIndex))];}



	Vector					GetVertex(int theNumber) {return *mPos[theNumber];}
	Vector					GetCentroid() {return (*mPos[0]+*mPos[1]+*mPos[2])/3;}
	Vector					GetIncenter()
	{
		Vector aAB=*mPos[0]-*mPos[1];
		Vector aBC=*mPos[1]-*mPos[2];
		Vector aCA=*mPos[2]-*mPos[0];
		float aA=aBC.Length();
		float aB=aCA.Length();
		float aC=aAB.Length();
		float aSum=aA+aB+aC;

		return Vector(
			(aA*mPos[0]->mX+aB*mPos[1]->mX+aC*mPos[2]->mX)/aSum,
			(aA*mPos[0]->mY+aB*mPos[1]->mY+aC*mPos[2]->mY)/aSum,
			(aA*mPos[0]->mZ+aB*mPos[1]->mZ+aC*mPos[2]->mZ)/aSum);
	}
	Vector					GetOrthoCenter();


	Cube					GetBoundingCube();
	Rect					GetBoundingXYRect();
	AABBRect				GetBoundingXYRectAABB();

	void					GetLegs(Array<Line3D>& theLegArray, bool isCW=true);
	Line3D					GetHypotenuse();

	inline Vector			GetNormal() {return Normal();}
	Vector Normal()
	{
		Vector aResult;
		Vector aVec1=*mPos[1]-*mPos[0];
		Vector aVec2=*mPos[2]-*mPos[0];
		aResult=-aVec1.Cross(aVec2);
		aResult.Normalize();
		return aResult;
	}

	//
	// For getting an unnormalized normal (for speed reasons)
	//
	inline Vector			GetNormalDir() {return NormalDir();}
	Vector NormalDir()
	{
		Vector aResult;
		Vector aVec1=*mPos[1]-*mPos[0];
		Vector aVec2=*mPos[2]-*mPos[0];
		aResult=-aVec1.Cross(aVec2);
		//aResult.Normalize();
		return aResult;
	}

	Vector GetPointOnTriangle(float theOff1, float theOff2) // For like UV aspects
	{
		Vector aAB=*mPos[1]-*mPos[0];
		Vector aAC=*mPos[2]-*mPos[0];

		if (theOff1+theOff2>=1.0f)
		{
			theOff1=1-theOff1;
			theOff2=1-theOff2;
		}
		return *mPos[0]+(theOff1*aAB)+theOff2*(aAC);
	}

	void Expand(float theAmount)
	{
		Vector aPos[3];
		aPos[0]=*mPos[0];aPos[1]=*mPos[1];aPos[2]=*mPos[2];

		Vector a01=aPos[0]-aPos[1];a01.SetLength(theAmount);
		Vector a12=aPos[1]-aPos[2];a12.SetLength(theAmount);
		Vector a02=aPos[0]-aPos[2];a02.SetLength(theAmount);
		Vector a10=-a01;
		Vector a21=-a12;
		Vector a20=-a02;

		*mPos[0]+=a01;*mPos[0]+=a02;
		*mPos[1]+=a10;*mPos[1]+=a12;
		*mPos[2]+=a20;*mPos[2]+=a21;
	}

	bool IsDegenerate();
	bool SamePoints(Triangle3DPtr& theOther);

public:
	Vector*					mPos[3];
};


class ViewFrustum
{
public:
	Plane					mPlane[6]; // These are normalized
	float					mFOV;

	void					FromMatrix(Matrix& theMatrix, float theMult=1.0f);

	bool					ContainsPoint(Vector thePos);
	bool					ContainsCube(Cube theCube);
	bool					ContainsSphere(Vector thePos, float theRadius);
	bool					ContainsCube(Cube theCube, Matrix theMatrix);

	bool					IsSphereBehind(char thePlane, Vector thePos, float theRadius);

	//
	// Isometric calls don't check top/bottom... they assume all clipping will be within the left/right/near/far planes
	//
	bool					ContainsPointISO(Vector thePos);
	bool					ContainsCubeISO(Cube theCube);
	bool					ContainsSphereISO(Vector thePos, float theRadius);

	Vector					CastOut(Vector theStart, Vector theRay);
};

class Volume
{
public:
	Array<Plane>			mPlane;

	inline Plane&			Right() {return mPlane[FRUSTRUM_RIGHT];}
	inline Plane&			Left() {return mPlane[FRUSTRUM_LEFT];}
	inline Plane&			Near() {return mPlane[FRUSTRUM_NEAR];}
	inline Plane&			Far() {return mPlane[FRUSTRUM_FAR];}
	inline Plane&			Top() {return mPlane[FRUSTRUM_TOP];}
	inline Plane&			Bottom() {return mPlane[FRUSTRUM_BOTTOM];}

	//Vector					CastOut(Vector theStart, Vector theRay);


	inline void				AddPlane(Plane thePlane) {mPlane+=thePlane;}
	void					FromCube(Cube theCube, Matrix theMatrix);	// Multiply cube by matrix
	inline void				FromCube(Cube theCube, Vector pointAt) {Matrix aMat;aMat.VectorToVector(Vector(0,0,1),pointAt);FromCube(theCube,aMat);}
	inline void				FromCube(Cube theCube) {Matrix aMat;FromCube(theCube,aMat);}
	void					FromTriangle(Triangle3D aT);
	void					FromTriangle(Triangle3D aT, Triangle3D aTN); // Plus normals
	inline void				FromTriangle(Vector theT1, Vector theT2, Vector theT3) {Triangle3D aT(theT1,theT2,theT3);FromTriangle(aT);}
	inline void				FromTriangle(Triangle3DPtr aTPtr) {Triangle3D aT(*aTPtr.mPos[0],*aTPtr.mPos[1],*aTPtr.mPos[2]);FromTriangle(aT);}
	void					FromInfiniteCylinder(Vector theOrigin, Vector theAxis, float theRadius, int theSides);

	inline void				Invert() {for (int aCount=0;aCount<mPlane.Size();aCount++) mPlane[aCount].mNormal*=-1;}

	Vector					ClampInsideToEdge(Vector thePos); // Clamp a point that's inside the volume to the edge of the volume
	Vector					ClampInsideToEdge(Vector thePos, Array<char>& theEdges); // Clamp a point that's inside the volume to the edge of the volume (but only for these eges)

	//
	// Move planes along their normals...
	//
	inline void				MovePlanes(float theAmount) {for (int aCount=0;aCount<mPlane.Size();aCount++) {mPlane[aCount].mPos+=mPlane[aCount].mNormal*theAmount;mPlane[aCount].Rehup();}}
	inline void				Expand(float theAmount) {MovePlanes(theAmount);}

	inline bool	ContainsPoint(Vector theV)
	{
		for (int aCount=0;aCount<mPlane.Size();aCount++) if (ClassifyPoint(theV,aCount)>0) return false;
		return true;
	};
	inline bool	EnclosesPoint(Vector theV)
	{
		for (int aCount=0;aCount<mPlane.Size();aCount++) if (ClassifyPoint(theV,aCount)>=0) return false;
		return true;
	};
	inline bool	ContainsSphere(Vector thePos, float theRadius)
	{
		if (ContainsPoint(thePos)) return true;
		for (int aCount=0;aCount<mPlane.Size();aCount++) if (mPlane[aCount].DistanceTo(thePos)<=theRadius) return true;
		return false;
	}
	inline bool ContainsLine(Vector theLine1, Vector theLine2)
	{
		for (int aCount=0;aCount<mPlane.Size();aCount++) if (ClassifyLine(theLine1,theLine2,aCount)>0) return false;
		return true;
	}
	inline bool ContainsCube(Cube theCube)
	{
		for (int aCount=0;aCount<mPlane.Size();aCount++) if (ClassifyCube(theCube,aCount)>0) return false;
		return true;
	}
	inline bool	ContainsTriangle(Vector theV1, Vector theV2, Vector theV3)	// Contains Triangle... (Remember, lines might cross)
	{
		for (int aCount=0;aCount<mPlane.Size();aCount++) if (ClassifyPoint(theV1,aCount)+ClassifyPoint(theV2,aCount)+ClassifyPoint(theV3,aCount)==3) return false;
		return true;
	}
	inline bool	EnclosesTriangle(Vector theV1, Vector theV2, Vector theV3)	// Contains Triangle... (Triangle must be fully inside)
	{
		if (!ContainsPoint(theV1)) return false;
		if (!ContainsPoint(theV2)) return false;
		if (!ContainsPoint(theV3)) return false;
		return true;
	}


//	inline char ClassifyPoint(Vector thePoint, char thePlane) {Vector aDir=mPlane[thePlane].mPos-thePoint;float aD=aDir.Dot(mPlane[thePlane].mNormal);if (aD<-0.0005f) return 1;if (aD>0.0005f) return -1;return 0;}
	inline char ClassifyPoint(Vector thePoint, char thePlane) {return ClassifyPoint(thePoint.mX,thePoint.mY,thePoint.mZ,thePlane);}
	inline char ClassifyPoint(float x, float y, float z, char thePlane)
	{
		Vector& aPos=mPlane[thePlane].mPos;
		Vector& aNorm=mPlane[thePlane].mNormal;
		float aD=(aPos.mX-x)*aNorm.mX+(aPos.mY-y)*aNorm.mY+(aPos.mZ-z)*aNorm.mZ;
		if (aD<-0.005f) return -1;
		if (aD>0.005f) return 1;
		return 0;
	}
	inline char	ClassifyLine(Vector theL1, Vector theL2, char thePlane)	{char aP1=ClassifyPoint(theL1,thePlane);char aP2=ClassifyPoint(theL2,thePlane);return aP1+aP2;}
	inline char ClassifyCube(Cube theCube, char thePlane)
	{
		Vector aP1=Vector(theCube.mX,theCube.mY,theCube.mZ);
		Vector aP2=Vector(theCube.mX+theCube.mXSize,theCube.mY+theCube.mYSize,theCube.mZ+theCube.mZSize);
		return ClassifyCube(aP1,aP2,thePlane);
	}
	inline char ClassifyCube(Vector theP1, Vector theP2, char thePlane)
	{
		int aResult=
			ClassifyPoint(theP1,thePlane)+
			ClassifyPoint(Vector(theP2.mX,theP1.mY,theP1.mZ),thePlane)+
			ClassifyPoint(Vector(theP1.mX,theP2.mY,theP1.mZ),thePlane)+
			ClassifyPoint(Vector(theP2.mX,theP2.mY,theP1.mZ),thePlane)+
			ClassifyPoint(Vector(theP1.mX,theP1.mY,theP2.mZ),thePlane)+
			ClassifyPoint(Vector(theP2.mX,theP1.mY,theP2.mZ),thePlane)+
			ClassifyPoint(Vector(theP1.mX,theP2.mY,theP2.mZ),thePlane)+
			ClassifyPoint(theP2,thePlane);

		if (aResult>=6) return 1;	// All points in front of plane
		if (aResult>=0) return 0;	// Intersecting
		return -1;	// All points behind plane
	}
};


class Circle // I want to call this Ellipse but friggin' Win32 has reserved that word!?!?!?!???
{
public:
	Circle() {mRadius=0;}
	Circle(Point thePos, float theRadius) {mPos=thePos;mRadius=Point(theRadius,theRadius);}
	Circle(Point thePos, Point theRadius) {mPos=thePos;mRadius=theRadius;}
	Circle(float theX, float theY, float theRadius) {mPos.mX=theX;mPos.mY=theY;mRadius=Point(theRadius,theRadius);}
	Circle(float theX, float theY, Point theRadius) {mPos.mX=theX;mPos.mY=theY;mRadius=theRadius;}

	bool					ContainsPoint(Point thePos);
	Rect					GetBound() {return Rect(mPos.mX-mRadius.mX,mPos.mY-mRadius.mY,mRadius.mX*2,mRadius.mY*2);}
	AABBRect				GetAABB() {return AABBRect(mPos.mX-mRadius.mX,mPos.mY-mRadius.mX,mPos.mX+mRadius.mX,mPos.mY+mRadius.mY);}

	inline void				SetPos(Point thePos) {mPos=thePos;}
	inline void				SetPos(float theX, float theY) {mPos.mX=theX;mPos.mY=theY;}
	inline void				SetRadius(float theRadius) {mRadius=Point(theRadius,theRadius);}
	inline void				SetRadius(Point theRadius) {mRadius=theRadius;}
	inline Point			GetPos() {return mPos;}
	inline Point			GetRadius() {return mRadius;}
	char*					ToString();

	//private:
	Point					mPos;
	Point					mRadius;
};

#define NULLNODE (-1)
template <class var_type>
struct PointerTreeNode3D
{
	PointerTreeNode3D() {mGhost=false;}
	bool IsLeaf() {return mChild1 == NULLNODE;}

	/// Enlarged AABB
	Cube mAABB;
	var_type mData;

	union
	{
		int mParent;
		int mNext;
	};

	int mChild1;
	int mChild2;
	int mHeight;

	bool	mGhost;
};

/// A dynamic AABB tree broad-phase, inspired by Nathanael Presson's btDbvt.
/// A dynamic tree arranges data in a binary tree to accelerate
/// queries such as volume queries and ray casts. Leafs are proxies
/// with an AABB. In the tree we expand the proxy AABB by b2_fatAABBFactor
/// so that the proxy AABB is bigger than the client object. This allows the client
/// object to move by small amounts without triggering a tree update.
///
/// Nodes are pooled and relocatable, so we use node indices rather than pointers.
bool DoesLineIntersectAABB(AABBCube theCube, Vector theLine1, Vector theLine2);
bool DoesRayIntersectAABB(AABBCube theCube, Vector theLine1, Vector theLine2);
#ifdef _DEBUG
void GXDrawLine(Vector theStart, Vector theEnd,int theColor);
void GXDrawCube(Cube theCube,int theColor);
#endif
template <class var_type>
class PointerTree3D
{
public:
	PointerTree3D()
	{
		Prep();
	}
	~PointerTree3D() {Reset();}

	int mFlag=0;

	float mNodeFatten=.005f;
	float mNodeMultiplier=2.0f;

	void Prep()
	{
		mRoot=NULLNODE;
		mStack.GuaranteeSize(256);
		mResultStackPtr=0;
		GetResults();

		mNodeCapacity=16;
		mNodeCount=0;
		mNodes=new PointerTreeNode3D<var_type>[mNodeCapacity];//PointerTreeNode<var_type>*)malloc(mNodeCapacity * sizeof(PointerTreeNode<var_type>));
		memset(mNodes, 0, mNodeCapacity * sizeof(PointerTreeNode3D<var_type>));
		for (int i=0; i<mNodeCapacity-1; ++i) {mNodes[i].mNext=i+1;mNodes[i].mHeight=-1;}
		mNodes[mNodeCapacity-1].mNext=NULLNODE;
		mNodes[mNodeCapacity-1].mHeight=-1;
		mFreeList=0;
		mPath=0;
		mInsertionCount=0;
	}

	void Reset()
	{
		delete [] mNodes;//free(mNodes);
		mNodes=NULL;
		for (int aCount=0;aCount<mResultStack.Size();aCount++) delete mResultStack[aCount];
		mResultStackPtr=0;
		mResultStack.Reset();
		Prep();
	}


	int Add(var_type theData, Cube aabb)
	{
		int theID=AllocateNode();
		mNodes[theID].mAABB=aabb.Expand(mNodeFatten);
		mNodes[theID].mData=theData;
		mNodes[theID].mHeight=0;
		InsertLeaf(theID);
		return theID;
	}

	int	Remove(int theID)
	{
		if (theID==-1) return -1;
		assert(0<= theID && theID<mNodeCapacity);
		assert(mNodes[theID].IsLeaf());
		RemoveLeaf(theID);
		FreeNode(theID);
		return -1;
	}


	//
	// Returns true if move happened...
	// the aabb should be an AABB that is unioned with the new position
	//
	bool Move(int theID,  Cube aabb,  Vector displacement)
	{
		assert(0<= theID && theID<mNodeCapacity);
		assert(mNodes[theID].IsLeaf());
		if (mNodes[theID].mAABB.Contains(aabb)) return false;
		RemoveLeaf(theID);
		AABBCube b=aabb.Expand(mNodeFatten);
		Vector d=mNodeMultiplier*displacement;

		if (d.mX<0.0f) b.mX1 += d.mX;
		else b.mX2 += d.mX;
		if (d.mY<0.0f) b.mY1 += d.mY;
		else b.mY2 += d.mY;
		if (d.mZ<0.0f) b.mZ1 += d.mZ;
		else b.mZ2 += d.mZ;

		mNodes[theID].mAABB=b;
		InsertLeaf(theID);
		return true;
	}

	//
	// Move to a new aabb cube location.  Returns true if we go into a new leaf.
	//
	bool Move(int theID,  Cube aabb)
	{
		assert(0<= theID && theID<mNodeCapacity);
		assert(mNodes[theID].IsLeaf());

		if (theID<0 || mNodes[theID].mAABB.Contains(aabb)) return false; // No move warranted!

		RemoveLeaf(theID);
		Cube b=aabb.Expand(mNodeFatten);
		mNodes[theID].mAABB=b;
		mNodes[theID].mHeight=0;
		InsertLeaf(theID);

		return true;
	}

#ifdef _DEBUG
	bool DebugMove(int theID,  Cube aabb)
	{
		assert(0<= theID && theID<mNodeCapacity);
		assert(mNodes[theID].IsLeaf());

		if (mNodes[theID].mAABB.Contains(aabb)) return false; // No move warranted!
		RemoveLeaf(theID);
		Cube b=aabb.Expand(mNodeFatten);
		mNodes[theID].mAABB=b;
		mNodes[theID].mHeight=0;
		InsertLeaf(theID);
		return true;
	}
#endif


	void						SetGhost(int theID, bool isGhost) {mNodes[theID].mGhost=isGhost;}
	var_type					GetData(int theID) {assert(0 <= theID && theID < mNodeCapacity);return mNodes[theID].mData;}
	Cube&						GetFatAABB(int theID) {assert(0 <= theID && theID < mNodeCapacity);return mNodes[theID].mAABB;}
	Array<var_type>& Get(Vector thePos, Array<var_type>* theList=NULL) 
	{
		if (!theList) theList=&GetResults();
		theList->Reset();
		theList->GuaranteeSize(100);int aLCount=0;

		mStack.Empty();
		mStack.Push(mRoot);

		while (mStack.GetStackCount()>0)
		{
			int aNodeId=mStack.Pop();
			if (aNodeId==NULLNODE) continue;
			PointerTreeNode3D<var_type>* aNode=mNodes+aNodeId;
			if (!aNode->mGhost && aNode->mAABB.ContainsPoint(thePos))
			{
				if (aNode->IsLeaf()) 
				{
					theList->Element(aLCount++)=aNode->mData;
					if (aLCount>=theList->Size()) theList->GuaranteeSize(theList->Size()*2);
				}
				else {mStack.Push(aNode->mChild1);mStack.Push(aNode->mChild2);}
			}
		}

		theList->Clip(aLCount);
		return *theList;
	}

	Array<var_type>& Get(Cube theAABB, Array<var_type>* theList=NULL)
	{
		if (!theList) theList=&GetResults();
		theList->Reset();
		theList->GuaranteeSize(100);int aLCount=0;

		mStack.Empty();
		mStack.Push(mRoot);

		while (mStack.GetStackCount()>0)
		{
			int aNodeId=mStack.Pop();
			if (aNodeId==NULLNODE) continue;
			PointerTreeNode3D<var_type>* aNode=mNodes+aNodeId;
			if (!aNode->mGhost && aNode->mAABB.Intersects(theAABB))
			{
				if (aNode->IsLeaf()) 
				{
					theList->Element(aLCount++)=aNode->mData;
					if (aLCount>=theList->Size()) theList->GuaranteeSize(theList->Size()*2);
				}
				else {mStack.Push(aNode->mChild1);mStack.Push(aNode->mChild2);}
			}
		}
		theList->Clip(aLCount);
		return *theList;
	}
	Array<var_type>& Get(Vector theStart, Vector theEnd, Array<var_type>* theList=NULL)
	{
		if (!theList) theList=&GetResults();
		theList->Reset();
		theList->GuaranteeSize(100);int aLCount=0;

		mStack.Empty();
		mStack.Push(mRoot);

		while (mStack.GetStackCount()>0)
		{
			int aNodeId=mStack.Pop();
			if (aNodeId==NULLNODE) continue;
			PointerTreeNode3D<var_type>* aNode=mNodes+aNodeId;
			if (!aNode->mGhost && DoesLineIntersectAABB(aNode->mAABB,theStart,theEnd))
			{
				if (aNode->IsLeaf()) 
				{
					theList->Element(aLCount++)=aNode->mData;
					if (aLCount>=theList->Size()) theList->GuaranteeSize(theList->Size()*2);
				}
				else {mStack.Push(aNode->mChild1);mStack.Push(aNode->mChild2);}
			}
		}

		theList->Clip(aLCount);
		return *theList;
	}
	Array<var_type>& Get(Vector theStart, Vector theEnd, float theRadius, Array<var_type>* theList=NULL)
	{
		if (!theList) theList=&GetResults();
		theList->Reset();
		theList->GuaranteeSize(100);int aLCount=0;

		mStack.Empty();
		mStack.Push(mRoot);

		while (mStack.GetStackCount()>0)
		{
			int aNodeId=mStack.Pop();
			if (aNodeId==NULLNODE) continue;
			PointerTreeNode3D<var_type>* aNode=mNodes+aNodeId;
			if (!aNode->mGhost && DoesLineIntersectAABB(aNode->mAABB.Expand(theRadius),theStart,theEnd))
			{
				if (aNode->IsLeaf()) 
				{
					theList->Element(aLCount++)=aNode->mData;
					if (aLCount>=theList->Size()) theList->GuaranteeSize(theList->Size()*2);
				}
				else {mStack.Push(aNode->mChild1);mStack.Push(aNode->mChild2);}
			}
		}

		theList->Clip(aLCount);
		return *theList;
	}
	Array<var_type>& Get(ViewFrustum& theVF, Array<var_type>* theList=NULL) 
	{
		if (!theList) theList=&GetResults();
		theList->Reset();
		theList->GuaranteeSize(100);int aLCount=0;

		mStack.Empty();
		mStack.Push(mRoot);

		while (mStack.GetStackCount()>0)
		{
			int aNodeId=mStack.Pop();
			if (aNodeId==NULLNODE) continue;
			PointerTreeNode3D<var_type>* aNode=mNodes+aNodeId;
			if (!aNode->mGhost && theVF.ContainsCube(aNode->mAABB))
			{
				if (aNode->IsLeaf()) 
				{
					theList->Element(aLCount++)=aNode->mData;
					if (aLCount>=theList->Size()) theList->GuaranteeSize(theList->Size()*2);
				}
				else {mStack.Push(aNode->mChild1);mStack.Push(aNode->mChild2);}
			}
		}
		theList->Clip(aLCount);
		return *theList;
	}

	Array<var_type>& GetAll(Array<var_type>* theList=NULL)
	{
		if (!theList) theList=&GetResults();
		theList->Reset();
		theList->GuaranteeSize(100);int aLCount=0;

		mStack.Empty();
		mStack.Push(mRoot);

		while (mStack.GetStackCount()>0)
		{
			int aNodeId=mStack.Pop();
			if (aNodeId==NULLNODE) continue;
			PointerTreeNode3D<var_type>* aNode=mNodes+aNodeId;
			if (aNode->IsLeaf()) 
			{
				theList->Element(aLCount++)=aNode->mData;
				if (aLCount>=theList->Size()) theList->GuaranteeSize(theList->Size()*2);
			}
			else {mStack.Push(aNode->mChild1);mStack.Push(aNode->mChild2);}
		}

		theList->Clip(aLCount);
		return *theList;
	}
	inline Array<var_type>& GetEverything(Array<var_type>* theList=NULL) {return GetAll(theList);}

#ifdef _DEBUG
	Array<PointerTreeNode3D<var_type>*>& GetAllNodes()
	{
		static Array<PointerTreeNode3D<var_type>*> aTempNodes;
		aTempNodes.GuaranteeSize(100);int aLCount=0;

		mStack.Empty();
		mStack.Push(mRoot);

		while (mStack.GetStackCount()>0)
		{
			int aNodeId=mStack.Pop();
			if (aNodeId==NULLNODE) continue;
			PointerTreeNode3D<var_type>* aNode=mNodes+aNodeId;
			if (aNode->IsLeaf()) 
			{
				aTempNodes.Element(aLCount++)=aNode;
				if (aLCount>=aTempNodes.Size()) aTempNodes.GuaranteeSize(aTempNodes.Size()*2);
			}
			else {mStack.Push(aNode->mChild1);mStack.Push(aNode->mChild2);}
		}

		aTempNodes.Clip(aLCount);
		return aTempNodes;
	}
#endif

	Array<var_type>& GetRay(Vector theStart, Vector theDir, Array<var_type>* theList=NULL)
	{
		if (theDir.LengthSquared()>1.0f) theDir.Normalize();

		if (!theList) theList=&GetResults();
		theList->Reset();
		theList->GuaranteeSize(100);int aLCount=0;

		mStack.Empty();
		mStack.Push(mRoot);

		while (mStack.GetStackCount()>0)
		{
			int aNodeId=mStack.Pop();
			if (aNodeId==NULLNODE) continue;
			PointerTreeNode3D<var_type>* aNode=mNodes+aNodeId;
			if (!aNode->mGhost && DoesRayIntersectAABB(aNode->mAABB,theStart,theDir))
			{
				if (aNode->IsLeaf()) 
				{
					theList->Element(aLCount++)=aNode->mData;
					if (aLCount>=theList->Size()) theList->GuaranteeSize(theList->Size()*2);
				}
				else {mStack.Push(aNode->mChild1);mStack.Push(aNode->mChild2);}
			}
		}

		theList->Clip(aLCount);
		return *theList;
	}

	void GetNodes(Cube theAABB, Array<PointerTreeNode3D<var_type>*>& theList)
	{
		theList.Reset();
		theList.GuaranteeSize(100);int aLCount=0;

		mStack.Empty();
		mStack.Push(mRoot);

		while (mStack.GetStackCount()>0)
		{
			int aNodeId=mStack.Pop();
			if (aNodeId==NULLNODE) continue;
			PointerTreeNode3D<var_type>* aNode=mNodes+aNodeId;
			if (!aNode->mGhost && aNode->mAABB.Intersects(theAABB))
			{
				if (aNode->IsLeaf()) 
				{
					theList.Element(aLCount++)=aNode;
					if (aLCount>=theList.Size()) theList.GuaranteeSize(theList.Size()*2);
				}
				else {mStack.Push(aNode->mChild1);mStack.Push(aNode->mChild2);}
			}
		}
		theList.Clip(aLCount);
	}



	void						Validate()
	{
		ValidateStructure(mRoot);
		ValidateMetrics(mRoot);
		int freeCount=0;
		int freeIndex=mFreeList;
		while (freeIndex!=NULLNODE)
		{
			assert(0<= freeIndex && freeIndex<mNodeCapacity);
			freeIndex=mNodes[freeIndex].mNext;
			++freeCount;
		}
		assert(GetHeight()==ComputeHeight());
		assert(mNodeCount+freeCount==mNodeCapacity);
	}

	int						GetHeight()
	{
		if (mRoot==NULLNODE) return 0;
		return mNodes[mRoot].mHeight;
	}

	int							GetMaxBalance()
	{
		int maxBalance=0;
		for (int i=0; i<mNodeCapacity; ++i)
		{
			PointerTreeNode3D<var_type>* node=mNodes+i;
			if (node->mHeight<= 1) continue;
			assert(node->IsLeaf()==false);
			int child1=node->mChild1;
			int child2=node->mChild2;
			int balance=abs(mNodes[child2].mHeight-mNodes[child1].mHeight);
			maxBalance=_max(maxBalance, balance);
		}

		return maxBalance;
	}

	float						GetAreaRatio()
	{
		if (mRoot==NULLNODE) return 0.0f;
		PointerTreeNode3D<var_type>* root=mNodes+mRoot;
		float rootArea=root->mAABB.GetPerimeter();

		float totalArea=0.0f;
		for (int i=0; i<mNodeCapacity; ++i)
		{
			PointerTreeNode3D<var_type>* node=mNodes+i;
			if (node->mHeight<0) continue;
			totalArea += node->mAABB.GetPerimeter();
		}
		return totalArea / rootArea;
	}

	void						RebuildBottomUp()
	{
		int* nodes=(int*)malloc(mNodeCount * sizeof(int));
		int count=0;

		// Build array of leaves. Free the rest.
		for (int i=0; i<mNodeCapacity; ++i)
		{
			if (mNodes[i].mHeight<0)
			{
				// free node in pool
				continue;
			}

			if (mNodes[i].IsLeaf())
			{
				mNodes[i].mParent=NULLNODE;
				nodes[count]=i;
				++count;
			}
			else
			{
				FreeNode(i);
			}
		}

		while (count > 1)
		{
			float minCost=FLOATMAX;
			int iMin=-1, jMin=-1;
			for (int i=0; i<count; ++i)
			{
				Cube aabbi=mNodes[nodes[i]].mAABB;

				for (int j=i+1; j<count; ++j)
				{
					Cube aabbj=mNodes[nodes[j]].mAABB;
					//Cube b;
					//b.Combine(aabbi, aabbj);
					Cube b=aabbi.Union(aabbj);
					float cost=b.GetPerimeter();
					if (cost<minCost)
					{
						iMin=i;
						jMin=j;
						minCost=cost;
					}
				}
			}

			int index1=nodes[iMin];
			int index2=nodes[jMin];
			PointerTreeNode3D<var_type>* child1=mNodes+index1;
			PointerTreeNode3D<var_type>* child2=mNodes+index2;

			int parentIndex=AllocateNode();
			PointerTreeNode3D<var_type>* parent=mNodes+parentIndex;
			parent->mChild1=index1;
			parent->mChild2=index2;
			parent->mHeight=1+_max(child1->mHeight, child2->mHeight);
			//parent->mAABB.Combine(child1->mAABB, child2->mAABB);
			parent->mAABB=child1->mAABB.Union(child2->mAABB);
			parent->mParent=NULLNODE;

			child1->mParent=parentIndex;
			child2->mParent=parentIndex;

			nodes[jMin]=nodes[count-1];
			nodes[iMin]=parentIndex;
			--count;
		}
		mRoot=nodes[0];
		free(nodes);
		Validate();
	}

	void						ShiftOrigin(Vector& newOrigin)
	{
		// Build array of leaves. Free the rest.
		for (int i=0; i<mNodeCapacity; ++i)
		{
			mNodes[i].mAABB.mX1+=newOrigin.mX;
			mNodes[i].mAABB.mY1+=newOrigin.mY;
			mNodes[i].mAABB.mZ1+=newOrigin.mZ;
			mNodes[i].mAABB.mX2+=newOrigin.mX;
			mNodes[i].mAABB.mY2+=newOrigin.mY;
			mNodes[i].mAABB.mZ2+=newOrigin.mZ;
		}
	}


private:
public:
	CleanArray<Array<var_type>*>		mResultStack;
	short								mResultStackPtr;
	inline void							PushResults() {mResultStackPtr++;}
	inline void							PopResults() {mResultStackPtr--;if (mResultStackPtr<0) {mResultStackPtr=0;OS_Core::Printf("Error!  PointerGrid::mResultStackPtr<0");}}
	Array<var_type>&					GetResults() {if (mResultStack.Size()<=mResultStackPtr) {Array<var_type>* aNewList=new Array<var_type>;mResultStack[mResultStackPtr]=aNewList;}return *mResultStack[mResultStackPtr];}
	void						CleanResults()
	{
		for (int aCount=0;aCount<mResultStack.Size();aCount++) if (mResultStack[aCount]) delete mResultStack[aCount];
		mResultStack.Reset();
		mResultStackPtr=0;
	}

	Array<var_type>				mNullList;
	Array<var_type>&			NullResults() {return mNullList;}
	Stack<int>					mStack;

	int						AllocateNode()
	{
		// Expand the node pool as needed.
		if (mFreeList==NULLNODE)
		{
			assert(mNodeCount==mNodeCapacity);

			// The free list is empty. Rebuild a bigger pool.
			PointerTreeNode3D<var_type>* oldNodes=mNodes;
			mNodeCapacity *= 2;
			mNodes=new PointerTreeNode3D<var_type>[mNodeCapacity];//(PointerTreeNode<var_type>*)malloc(mNodeCapacity * sizeof(PointerTreeNode<var_type>));
			for (int aCount=0;aCount<mNodeCount;aCount++) mNodes[aCount]=oldNodes[aCount];
			//memcpy(mNodes, oldNodes, mNodeCount * sizeof(PointerTreeNode<var_type>));
			delete [] oldNodes;//free(oldNodes);

							   // Build a linked list for the free list. The parent
							   // pointer becomes the "next" pointer.
			for (int i=mNodeCount; i<mNodeCapacity-1; ++i)
			{
				mNodes[i].mNext=i+1;
				mNodes[i].mHeight=-1;
			}
			mNodes[mNodeCapacity-1].mNext=NULLNODE;
			mNodes[mNodeCapacity-1].mHeight=-1;
			mFreeList=mNodeCount;
		}

		// Peel a node off the free list.
		int nodeId=mFreeList;
		mFreeList=mNodes[nodeId].mNext;
		mNodes[nodeId].mParent=NULLNODE;
		mNodes[nodeId].mChild1=NULLNODE;
		mNodes[nodeId].mChild2=NULLNODE;
		mNodes[nodeId].mHeight=0;
		mNodes[nodeId].mData=NULL;
		++mNodeCount;
		return nodeId;
	}

	void						FreeNode(int nodeId)
	{
		assert(0<= nodeId && nodeId<mNodeCapacity);
		assert(0<mNodeCount);
		mNodes[nodeId].mData=NULL;
		mNodes[nodeId].mNext=mFreeList;
		mNodes[nodeId].mHeight=-1;
		mFreeList=nodeId;
		--mNodeCount;
	}

	void						InsertLeaf(int leaf)
	{
		++mInsertionCount;

		if (mRoot==NULLNODE)
		{
			mRoot=leaf;
			mNodes[mRoot].mParent=NULLNODE;
			return;
		}

		// Find the best sibling for this node
		Cube leafAABB=mNodes[leaf].mAABB;
		int index=mRoot;
		while (mNodes[index].IsLeaf()==false)
		{
			int child1=mNodes[index].mChild1;
			int child2=mNodes[index].mChild2;

			float area=mNodes[index].mAABB.GetPerimeter();

			Cube combinedAABB;
			combinedAABB=mNodes[index].mAABB.Union(leafAABB);
			float combinedArea=combinedAABB.GetPerimeter();

			// Cost of creating a new parent for this node and the new leaf
			float cost=2.0f * combinedArea;

			// Minimum cost of pushing the leaf further down the tree
			float inheritanceCost=2.0f * (combinedArea-area);

			// Cost of descending into child1
			float cost1;
			if (mNodes[child1].IsLeaf())
			{
				Cube aabb;
				aabb=leafAABB.Union(mNodes[child1].mAABB);
				cost1=aabb.GetPerimeter()+inheritanceCost;
			}
			else
			{
				Cube aabb;
				aabb=leafAABB.Union(mNodes[child1].mAABB);
				float oldArea=mNodes[child1].mAABB.GetPerimeter();
				float newArea=aabb.GetPerimeter();
				cost1=(newArea-oldArea)+inheritanceCost;
			}

			// Cost of descending into child2
			float cost2;
			if (mNodes[child2].IsLeaf())
			{
				Cube aabb;
				aabb=leafAABB.Union(mNodes[child2].mAABB);
				cost2=aabb.GetPerimeter()+inheritanceCost;
			}
			else
			{
				Cube aabb;
				aabb=leafAABB.Union(mNodes[child2].mAABB);
				float oldArea=mNodes[child2].mAABB.GetPerimeter();
				float newArea=aabb.GetPerimeter();
				cost2=newArea-oldArea+inheritanceCost;
			}

			// Descend according to the minimum cost.
			if (cost<cost1 && cost<cost2)
			{
				break;
			}

			// Descend
			if (cost1<cost2)
			{
				index=child1;
			}
			else
			{
				index=child2;
			}
		}

		int sibling=index;

		// Create a new parent.
		int oldParent=mNodes[sibling].mParent;
		int newParent=AllocateNode();
		mNodes[newParent].mParent=oldParent;
		mNodes[newParent].mData=NULL;
		mNodes[newParent].mAABB=leafAABB.Union(mNodes[sibling].mAABB);
		mNodes[newParent].mHeight=mNodes[sibling].mHeight+1;

		if (oldParent!=NULLNODE)
		{
			// The sibling was not the root.
			if (mNodes[oldParent].mChild1==sibling)
			{
				mNodes[oldParent].mChild1=newParent;
			}
			else
			{
				mNodes[oldParent].mChild2=newParent;
			}

			mNodes[newParent].mChild1=sibling;
			mNodes[newParent].mChild2=leaf;
			mNodes[sibling].mParent=newParent;
			mNodes[leaf].mParent=newParent;
		}
		else
		{
			// The sibling was the root.
			mNodes[newParent].mChild1=sibling;
			mNodes[newParent].mChild2=leaf;
			mNodes[sibling].mParent=newParent;
			mNodes[leaf].mParent=newParent;
			mRoot=newParent;
		}
		// Walk back up the tree fixing heights and AABBs
		index=mNodes[leaf].mParent;
		while (index!=NULLNODE)
		{
			index=Balance(index);
			int child1=mNodes[index].mChild1;
			int child2=mNodes[index].mChild2;
			assert(child1!=NULLNODE);
			assert(child2!=NULLNODE);
			mNodes[index].mHeight=1+_max(mNodes[child1].mHeight, mNodes[child2].mHeight);
			mNodes[index].mAABB=mNodes[child1].mAABB.Union(mNodes[child2].mAABB);
			index=mNodes[index].mParent;
		}
	}

	void						RemoveLeaf(int leaf)
	{
		if (leaf==mRoot)
		{
			mRoot=NULLNODE;
			return;
		}

		int parent=mNodes[leaf].mParent;
		int grandParent=mNodes[parent].mParent;
		int sibling;
		if (mNodes[parent].mChild1==leaf)
		{
			sibling=mNodes[parent].mChild2;
		}
		else
		{
			sibling=mNodes[parent].mChild1;
		}

		if (grandParent!=NULLNODE)
		{
			// Destroy parent and connect sibling to grandParent.
			if (mNodes[grandParent].mChild1==parent)
			{
				mNodes[grandParent].mChild1=sibling;
			}
			else
			{
				mNodes[grandParent].mChild2=sibling;
			}
			mNodes[sibling].mParent=grandParent;
			FreeNode(parent);

			// Adjust ancestor bounds.
			int index=grandParent;
			while (index!=NULLNODE)
			{
				index=Balance(index);

				int child1=mNodes[index].mChild1;
				int child2=mNodes[index].mChild2;

				mNodes[index].mAABB.Unionize(mNodes[child1].mAABB);
				mNodes[index].mAABB.Unionize(mNodes[child2].mAABB);
				//mNodes[index].mAABB.Combine(mNodes[child1].mAABB, mNodes[child2].mAABB);
				mNodes[index].mHeight=1+_max(mNodes[child1].mHeight, mNodes[child2].mHeight);

				index=mNodes[index].mParent;
			}
		}
		else
		{
			mRoot=sibling;
			mNodes[sibling].mParent=NULLNODE;
			FreeNode(parent);
		}

		//Validate();
	}

	int							Balance(int iA)
	{
		assert(iA!=NULLNODE);

		PointerTreeNode3D<var_type>* A=mNodes+iA;
		if (A->IsLeaf() || A->mHeight<2)
		{
			return iA;
		}

		int iB=A->mChild1;
		int iC=A->mChild2;
		assert(0<= iB && iB<mNodeCapacity);
		assert(0<= iC && iC<mNodeCapacity);

		PointerTreeNode3D<var_type>* B=mNodes+iB;
		PointerTreeNode3D<var_type>* C=mNodes+iC;

		int balance=C->mHeight-B->mHeight;

		// Rotate C up
		if (balance > 1)
		{
			int iF=C->mChild1;
			int iG=C->mChild2;
			PointerTreeNode3D<var_type>* F=mNodes+iF;
			PointerTreeNode3D<var_type>* G=mNodes+iG;
			assert(0<= iF && iF<mNodeCapacity);
			assert(0<= iG && iG<mNodeCapacity);

			// Swap A and C
			C->mChild1=iA;
			C->mParent=A->mParent;
			A->mParent=iC;

			// A's old parent should point to C
			if (C->mParent!=NULLNODE)
			{
				if (mNodes[C->mParent].mChild1==iA)
				{
					mNodes[C->mParent].mChild1=iC;
				}
				else
				{
					assert(mNodes[C->mParent].mChild2==iA);
					mNodes[C->mParent].mChild2=iC;
				}
			}
			else
			{
				mRoot=iC;
			}

			// Rotate
			if (F->mHeight > G->mHeight)
			{
				C->mChild2=iF;
				A->mChild2=iG;
				G->mParent=iA;
				A->mAABB=B->mAABB.Union(G->mAABB);
				C->mAABB=A->mAABB.Union(F->mAABB);

				A->mHeight=1+_max(B->mHeight, G->mHeight);
				C->mHeight=1+_max(A->mHeight, F->mHeight);
			}
			else
			{
				C->mChild2=iG;
				A->mChild2=iF;
				F->mParent=iA;
				A->mAABB=B->mAABB.Union(F->mAABB);
				C->mAABB=A->mAABB.Union(G->mAABB);

				A->mHeight=1+_max(B->mHeight, F->mHeight);
				C->mHeight=1+_max(A->mHeight, G->mHeight);
			}

			return iC;
		}

		// Rotate B up
		if (balance<-1)
		{
			int iD=B->mChild1;
			int iE=B->mChild2;
			PointerTreeNode3D<var_type>* D=mNodes+iD;
			PointerTreeNode3D<var_type>* E=mNodes+iE;
			assert(0<= iD && iD<mNodeCapacity);
			assert(0<= iE && iE<mNodeCapacity);

			// Swap A and B
			B->mChild1=iA;
			B->mParent=A->mParent;
			A->mParent=iB;

			// A's old parent should point to B
			if (B->mParent!=NULLNODE)
			{
				if (mNodes[B->mParent].mChild1==iA)
				{
					mNodes[B->mParent].mChild1=iB;
				}
				else
				{
					assert(mNodes[B->mParent].mChild2==iA);
					mNodes[B->mParent].mChild2=iB;
				}
			}
			else
			{
				mRoot=iB;
			}

			// Rotate
			if (D->mHeight > E->mHeight)
			{
				B->mChild2=iD;
				A->mChild1=iE;
				E->mParent=iA;
				A->mAABB=C->mAABB.Union(E->mAABB);
				B->mAABB=A->mAABB.Union(D->mAABB);

				A->mHeight=1+_max(C->mHeight, E->mHeight);
				B->mHeight=1+_max(A->mHeight, D->mHeight);
			}
			else
			{
				B->mChild2=iE;
				A->mChild1=iD;
				D->mParent=iA;
				A->mAABB=C->mAABB.Union(D->mAABB);
				B->mAABB=A->mAABB.Union(E->mAABB);

				A->mHeight=1+_max(C->mHeight, D->mHeight);
				B->mHeight=1+_max(A->mHeight, E->mHeight);
			}

			return iB;
		}

		return iA;
	}

	inline int					ComputeHeight() {return ComputeHeight(mRoot);}
	int							ComputeHeight(int nodeId)
	{
		assert(0<= nodeId && nodeId<mNodeCapacity);
		PointerTreeNode3D<var_type>* node=mNodes+nodeId;
		if (node->IsLeaf())	return 0;
		int height1=ComputeHeight(node->mChild1);
		int height2=ComputeHeight(node->mChild2);
		return 1+_max(height1, height2);
	}

	void						ValidateStructure(int index)
	{
		if (index==NULLNODE) return;
		if (index==mRoot) assert(mNodes[index].mParent==NULLNODE);
		PointerTreeNode3D<var_type>* node=mNodes+index;
		int child1=node->mChild1;
		int child2=node->mChild2;
		if (node->IsLeaf())
		{
			assert(child1==NULLNODE);
			assert(child2==NULLNODE);
			assert(node->mHeight==0);
			return;
		}
		assert(0<= child1 && child1<mNodeCapacity);
		assert(0<= child2 && child2<mNodeCapacity);
		assert(mNodes[child1].mParent==index);
		assert(mNodes[child2].mParent==index);
		ValidateStructure(child1);
		ValidateStructure(child2);
	}

	void						ValidateMetrics(int index) 
	{
		if (index==NULLNODE) return;
		PointerTreeNode3D<var_type>* node=mNodes+index;
		int child1=node->mChild1;
		int child2=node->mChild2;
		if (node->IsLeaf())
		{
			assert(child1==NULLNODE);
			assert(child2==NULLNODE);
			assert(node->mHeight==0);
			return;
		}
		assert(0<=child1 && child1<mNodeCapacity);
		assert(0<=child2 && child2<mNodeCapacity);
		int height1=mNodes[child1].mHeight;
		int height2=mNodes[child2].mHeight;
		int height;
		height=1+_max(height1, height2);
		assert(node->mHeight==height);
		//Cube aabb;
		//aabb.Combine(mNodes[child1].mAABB, mNodes[child2].mAABB);
		Cube aabb=mNodes[child1].mAABB.Union(mNodes[child2].mAABB);
		assert(aabb.LowerBound()==node->mAABB.LowerBound());
		assert(aabb.UpperBound()==node->mAABB.UpperBound());
		ValidateMetrics(child1);
		ValidateMetrics(child2);
	}

	int						mRoot;

	PointerTreeNode3D<var_type>*	mNodes;
	int								mNodeCount;
	int								mNodeCapacity;
	int								mFreeList;
	unsigned int					mPath;
	int								mInsertionCount;

	Cube							GetNodeAABB(int theNode) {return mNodes[theNode].mAABB;}

	//
	// Leaving this here just so it can be an example...
	//
	/*
	void					Draw()
	{
		for (int aCount=0;aCount<mNodeCount;aCount++)
		{
		PointerTreeNode3D<var_type>* node=mNodes+aCount;

		gG.DrawLine(node->mAABB.UpperLeft(),node->mAABB.UpperRight(),1);
		gG.DrawLine(node->mAABB.UpperRight(),node->mAABB.LowerRight(),1);
		gG.DrawLine(node->mAABB.LowerRight(),node->mAABB.LowerLeft(),1);
		gG.DrawLine(node->mAABB.LowerLeft(),node->mAABB.UpperLeft(),1);
		}
	}
	*/
};

#ifndef Tree3D
#define SmartTree3D(n) PointerTree3D<Smart(n)>
#define Tree3D(n) PointerTree3D<n>
#endif

template <class var_type>
struct PointerTreeNode
{
	PointerTreeNode() {mGhost=false;}
	bool IsLeaf() {return mChild1 == NULLNODE;}

	/// Enlarged AABB
	AABBRect mAABB;
	var_type mData;

	union
	{
		int mParent;
		int mNext;
	};

	int mChild1;
	int mChild2;
	int mHeight;

	bool	mGhost;
};

/// A dynamic AABB tree broad-phase, inspired by Nathanael Presson's btDbvt.
/// A dynamic tree arranges data in a binary tree to accelerate
/// queries such as volume queries and ray casts. Leafs are proxies
/// with an AABB. In the tree we expand the proxy AABB by b2_fatAABBFactor
/// so that the proxy AABB is bigger than the client object. This allows the client
/// object to move by small amounts without triggering a tree update.
///
/// Nodes are pooled and relocatable, so we use node indices rather than pointers.
/// 

bool DoesLineIntersectAABB(AABBRect theRect, Point theLine1, Point theLine2);
template <class var_type, class list_type>
class PointerTree
{
public:
	PointerTree()
	{
		Prep();
	}
	~PointerTree() {Reset();}

	int mFlag=0;
	float mNodeFatten=.005f;
	float mNodeMultiplier=2.0f;

	void Prep()
	{
		mRoot=NULLNODE;
		mStack.GuaranteeSize(256);
		mResultStackPtr=0;
		GetResults();

		mNodeCapacity=16;
		mNodeCount=0;
		mNodes=new PointerTreeNode<var_type>[mNodeCapacity];//PointerTreeNode<var_type>*)malloc(mNodeCapacity * sizeof(PointerTreeNode<var_type>));
		memset(mNodes, 0, mNodeCapacity * sizeof(PointerTreeNode<var_type>));
		for (int i=0; i<mNodeCapacity-1; ++i) {mNodes[i].mNext=i+1;mNodes[i].mHeight=-1;}
		mNodes[mNodeCapacity-1].mNext=NULLNODE;
		mNodes[mNodeCapacity-1].mHeight=-1;
		mFreeList=0;
		mPath=0;
		mInsertionCount=0;
	}

	void Reset()
	{
		delete [] mNodes;//free(mNodes);
		mNodes=NULL;
		for (int aCount=0;aCount<mResultStack.Size();aCount++) delete mResultStack[aCount];
		mResultStackPtr=0;
		mResultStack.Reset();

		Prep();
	}


	int							Add(var_type theData, AABBRect aabb)
	{
		int theID=AllocateNode();
		mNodes[theID].mAABB=aabb.Expand(mNodeFatten);
		mNodes[theID].mData=theData;
		mNodes[theID].mHeight=0;
		InsertLeaf(theID);
		return theID;
	}

	int						Remove(int theID)
	{
		if (theID==-1) return -1;
		assert(0<= theID && theID<mNodeCapacity);
		assert(mNodes[theID].IsLeaf());
		RemoveLeaf(theID);
		FreeNode(theID);
		return -1;
	}


	//
	// Returns true if move happened...
	// the aabb should be an AABB that is unioned with the new position
	//
	bool						Move(int theID,  AABBRect aabb,  Point displacement=Point(0,0)) // Include displacement if you want the old position to also trigger a hit
	{
		assert(0<= theID && theID<mNodeCapacity);
		assert(mNodes[theID].IsLeaf());
		if (mNodes[theID].mAABB.Contains(aabb)) return false;
		RemoveLeaf(theID);

		AABBRect b=aabb.Expand(mNodeFatten);
		if (displacement.LengthSquared())
		{
			Point d=mNodeMultiplier*displacement;
			if (d.mX<0.0f) b.mX1 += d.mX;
			else b.mX2 += d.mX;
			if (d.mY<0.0f) b.mY1 += d.mY;
			else b.mY2 += d.mY;
		}

		mNodes[theID].mAABB=b;
		InsertLeaf(theID);
		return true;
	}


	void						SetGhost(int theID, bool isGhost) {mNodes[theID].mGhost=isGhost;}
	var_type					GetData(int theID) {assert(0 <= theID && theID < mNodeCapacity);return mNodes[theID].mData;}
	AABBRect&					GetFatAABB(int theID) {assert(0 <= theID && theID < mNodeCapacity);return mNodes[theID].mAABB;}
	list_type&					Get(Point thePos, list_type* theList=NULL) 
	{
		if (!theList) theList=&GetResults();
		theList->Reset();

		mStack.Empty();
		mStack.Push(mRoot);

		while (mStack.GetStackCount()>0)
		{
			int aNodeId=mStack.Pop();
			if (aNodeId==NULLNODE) continue;
			PointerTreeNode<var_type>* aNode=mNodes+aNodeId;
			if (!aNode->mGhost && aNode->mAABB.ContainsPoint(thePos))
			{
				if (aNode->IsLeaf()) theList->Add(aNode->mData);
				else {mStack.Push(aNode->mChild1);mStack.Push(aNode->mChild2);}
			}
		}
		return *theList;
	}

	list_type&					Get(AABBRect theAABB, list_type* theList=NULL)
	{
		if (!theList) theList=&GetResults();
		theList->Reset();

		mStack.Empty();
		mStack.Push(mRoot);

		while (mStack.GetStackCount()>0)
		{
			int aNodeId=mStack.Pop();
			if (aNodeId==NULLNODE) continue;
			PointerTreeNode<var_type>* aNode=mNodes+aNodeId;
			if (!aNode->mGhost && aNode->mAABB.Intersects(theAABB))
			{
				if (aNode->IsLeaf()) theList->Add(aNode->mData);
				else {mStack.Push(aNode->mChild1);mStack.Push(aNode->mChild2);}
			}
		}
		return *theList;
	}
	list_type&					Get(Point theStart, Point theEnd, list_type* theList=NULL)
	{
		if (!theList) theList=&GetResults();
		theList->Reset();

		Point aR=theEnd-theStart;
		if (aR.LengthSquared()==0) return *theList;
		//assert(aR.LengthSquared() > 0.0f);
		aR.Normalize();

		// v is perpendicular to the segment.
		Point aV=aR.Perp();
		Point aAbsV=aV.Abs();
		float aMaxFraction=1.0f;

		AABBRect aSegmentAABB;
		{
			Point aT=theStart+aMaxFraction*(theEnd-theStart);
			aSegmentAABB.SetLowerBound(PointMin(theStart,aT));
			aSegmentAABB.SetUpperBound(PointMax(theStart,aT));
		}

		mStack.Empty();
		mStack.Push(mRoot);

		while (mStack.GetStackCount()>0)
		{
			int aNodeId=mStack.Pop();
			if (aNodeId==NULLNODE) continue;

			PointerTreeNode<var_type>* aNode=mNodes+aNodeId;
			//if (aNode->mGhost || !aNode->mAABB.Intersects(aSegmentAABB)) continue;
			//
			// ^ More exact but slower....
			//

			if (aNode->mGhost || !DoesLineIntersectAABB(aNode->mAABB,theStart,theEnd)) continue;
			//
			// ^ Much faster but sloppy!  Gives false positives!
			//

			Point aC=aNode->mAABB.Center();
			Point aH=aNode->mAABB.GetSize();
			float aSeparation=(float)fabs(aV.Dot(theStart-aC))-aAbsV.Dot(aH);
			if (aSeparation>0.0f) continue;

			if (aNode->IsLeaf()) theList->Add(aNode->mData);
			else {mStack.Push(aNode->mChild1);mStack.Push(aNode->mChild2);}
		}
		return *theList;
	}

	list_type& GetAll(list_type* theList=NULL)
	{
		if (!theList) theList=&GetResults();
		theList->Reset();
		theList->GuaranteeSize(100);int aLCount=0;

		mStack.Empty();
		mStack.Push(mRoot);

		while (mStack.GetStackCount()>0)
		{
			int aNodeId=mStack.Pop();
			if (aNodeId==NULLNODE) continue;
			PointerTreeNode<var_type>* aNode=mNodes+aNodeId;
			if (aNode->IsLeaf()) theList->Add(aNode->mData);
			else {mStack.Push(aNode->mChild1);mStack.Push(aNode->mChild2);}
		}
		return *theList;
	}
	inline list_type& GetEverything(list_type* theList=NULL) {return GetAll(theList);}


	//
	// Lets you specify a border area, to get more range
	//
	list_type&					Get(Point theStart, Point theEnd, float theBorderArea, list_type* theList=NULL)
	{
		if (!theList) theList=&GetResults();
		theList->Reset();

		Point aR=theEnd-theStart;
		if (aR.LengthSquared()==0) return *theList;
		//assert(aR.LengthSquared() > 0.0f);
		aR.Normalize();

		// v is perpendicular to the segment.
		Point aV=aR.Perp();
		Point aAbsV=aV.Abs();
		float aMaxFraction=1.0f;

		AABBRect aSegmentAABB;
		{
			Point aT=theStart+aMaxFraction*(theEnd-theStart);
			aSegmentAABB.SetLowerBound(PointMin(theStart,aT));
			aSegmentAABB.SetUpperBound(PointMax(theStart,aT));
			aSegmentAABB=aSegmentAABB.Expand(theBorderArea);
		}

		mStack.Empty();
		mStack.Push(mRoot);

		while (mStack.GetStackCount()>0)
		{
			int aNodeId=mStack.Pop();
			if (aNodeId==NULLNODE) continue;

			PointerTreeNode<var_type>* aNode=mNodes+aNodeId;
			//if (aNode->mGhost || !aNode->mAABB.Intersects(aSegmentAABB)) continue;
			//
			// ^ More exact but slower....
			//

			if (aNode->mGhost || !DoesLineIntersectAABB(aNode->mAABB,theStart,theEnd)) continue;
			//
			// ^ Much faster but sloppy!  Gives false positives!
			//

			Point aC=aNode->mAABB.Center();
			Point aH=aNode->mAABB.GetSize();
			float aSeparation=abs(aV.Dot(theStart-aC))-aAbsV.Dot(aH);
			if (aSeparation>0.0f) continue;

			if (aNode->IsLeaf()) theList->Add(aNode->mData);
			else {mStack.Push(aNode->mChild1);mStack.Push(aNode->mChild2);}
		}
		return *theList;
	}

	void						Validate()
	{
		ValidateStructure(mRoot);
		ValidateMetrics(mRoot);
		int freeCount=0;
		int freeIndex=mFreeList;
		while (freeIndex!=NULLNODE)
		{
			assert(0<= freeIndex && freeIndex<mNodeCapacity);
			freeIndex=mNodes[freeIndex].mNext;
			++freeCount;
		}
		assert(GetHeight()==ComputeHeight());
		assert(mNodeCount+freeCount==mNodeCapacity);
	}

	int						GetHeight()
	{
		if (mRoot==NULLNODE) return 0;
		return mNodes[mRoot].mHeight;
	}

	int							GetMaxBalance()
	{
		int maxBalance=0;
		for (int i=0; i<mNodeCapacity; ++i)
		{
			PointerTreeNode<var_type>* node=mNodes+i;
			if (node->mHeight<= 1) continue;
			assert(node->IsLeaf()==false);
			int child1=node->mChild1;
			int child2=node->mChild2;
			int balance=abs(mNodes[child2].mHeight-mNodes[child1].mHeight);
			maxBalance=_max(maxBalance, balance);
		}

		return maxBalance;
	}

	float						GetAreaRatio()
	{
		if (mRoot==NULLNODE) return 0.0f;
		PointerTreeNode<var_type>* root=mNodes+mRoot;
		float rootArea=root->mAABB.GetPerimeter();

		float totalArea=0.0f;
		for (int i=0; i<mNodeCapacity; ++i)
		{
			PointerTreeNode<var_type>* node=mNodes+i;
			if (node->mHeight<0) continue;
			totalArea += node->mAABB.GetPerimeter();
		}
		return totalArea / rootArea;
	}

	void						RebuildBottomUp()
	{
		int* nodes=(int*)malloc(mNodeCount * sizeof(int));
		int count=0;

		// Build array of leaves. Free the rest.
		for (int i=0; i<mNodeCapacity; ++i)
		{
			if (mNodes[i].mHeight<0)
			{
				// free node in pool
				continue;
			}

			if (mNodes[i].IsLeaf())
			{
				mNodes[i].mParent=NULLNODE;
				nodes[count]=i;
				++count;
			}
			else
			{
				FreeNode(i);
			}
		}

		while (count > 1)
		{
			float minCost=FLOATMAX;
			int iMin=-1, jMin=-1;
			for (int i=0; i<count; ++i)
			{
				AABBRect aabbi=mNodes[nodes[i]].mAABB;

				for (int j=i+1; j<count; ++j)
				{
					AABBRect aabbj=mNodes[nodes[j]].mAABB;
					AABBRect b;
					b.Combine(aabbi, aabbj);
					float cost=b.GetPerimeter();
					if (cost<minCost)
					{
						iMin=i;
						jMin=j;
						minCost=cost;
					}
				}
			}

			int index1=nodes[iMin];
			int index2=nodes[jMin];
			PointerTreeNode<var_type>* child1=mNodes+index1;
			PointerTreeNode<var_type>* child2=mNodes+index2;

			int parentIndex=AllocateNode();
			PointerTreeNode<var_type>* parent=mNodes+parentIndex;
			parent->mChild1=index1;
			parent->mChild2=index2;
			parent->mHeight=1+_max(child1->mHeight, child2->mHeight);
			parent->mAABB.Combine(child1->mAABB, child2->mAABB);
			parent->mParent=NULLNODE;

			child1->mParent=parentIndex;
			child2->mParent=parentIndex;

			nodes[jMin]=nodes[count-1];
			nodes[iMin]=parentIndex;
			--count;
		}
		mRoot=nodes[0];
		free(nodes);
		Validate();
	}

	void						ShiftOrigin(Point& newOrigin)
	{
		// Build array of leaves. Free the rest.
		for (int i=0; i<mNodeCapacity; ++i)
		{
			mNodes[i].mAABB.mX1+=newOrigin.mX;
			mNodes[i].mAABB.mY1+=newOrigin.mY;
			mNodes[i].mAABB.mX2+=newOrigin.mX;
			mNodes[i].mAABB.mY2+=newOrigin.mY;
		}
	}


private:
public:
	CleanArray<list_type*>		mResultStack;
	short						mResultStackPtr;
	inline void					PushResults() {mResultStackPtr++;}
	inline void					PopResults() {mResultStackPtr--;if (mResultStackPtr<0) {mResultStackPtr=0;OS_Core::Printf("Error!  PointerGrid::mResultStackPtr<0");}}
	list_type&					GetResults() 
	{
		if (mResultStack.Size()<=mResultStackPtr)
		{
			list_type* aNewList=new list_type;
			mResultStack[mResultStackPtr]=aNewList;
			mResultStack[mResultStackPtr]->Unique();
			mResultStack[mResultStackPtr]->SetCanShrink(false);
		}
		return *mResultStack[mResultStackPtr];
	}
	void						CleanResults()
	{
		for (int aCount=0;aCount<mResultStack.Size();aCount++)
		{
			if (mResultStack[aCount]) mResultStack[aCount]->Zero();
		}
	}

	list_type					mNullList;
	list_type&					NullResults() {return mNullList;}
	Stack<int>					mStack;

	int						AllocateNode()
	{
		// Expand the node pool as needed.
		if (mFreeList==NULLNODE)
		{
			assert(mNodeCount==mNodeCapacity);

			// The free list is empty. Rebuild a bigger pool.
			PointerTreeNode<var_type>* oldNodes=mNodes;
			mNodeCapacity *= 2;
			mNodes=new PointerTreeNode<var_type>[mNodeCapacity];//(PointerTreeNode<var_type>*)malloc(mNodeCapacity * sizeof(PointerTreeNode<var_type>));
			for (int aCount=0;aCount<mNodeCount;aCount++) mNodes[aCount]=oldNodes[aCount];
			//memcpy(mNodes, oldNodes, mNodeCount * sizeof(PointerTreeNode<var_type>));
			delete [] oldNodes;//free(oldNodes);

							   // Build a linked list for the free list. The parent
							   // pointer becomes the "next" pointer.
			for (int i=mNodeCount; i<mNodeCapacity-1; ++i)
			{
				mNodes[i].mNext=i+1;
				mNodes[i].mHeight=-1;
			}
			mNodes[mNodeCapacity-1].mNext=NULLNODE;
			mNodes[mNodeCapacity-1].mHeight=-1;
			mFreeList=mNodeCount;
		}

		// Peel a node off the free list.
		int nodeId=mFreeList;
		mFreeList=mNodes[nodeId].mNext;
		mNodes[nodeId].mParent=NULLNODE;
		mNodes[nodeId].mChild1=NULLNODE;
		mNodes[nodeId].mChild2=NULLNODE;
		mNodes[nodeId].mHeight=0;
		mNodes[nodeId].mData=NULL;
		++mNodeCount;
		return nodeId;
	}

	void						FreeNode(int nodeId)
	{
		assert(0<= nodeId && nodeId<mNodeCapacity);
		assert(0<mNodeCount);
		mNodes[nodeId].mData=NULL;
		mNodes[nodeId].mNext=mFreeList;
		mNodes[nodeId].mHeight=-1;
		mFreeList=nodeId;
		--mNodeCount;
	}

	void						InsertLeaf(int leaf)
	{
		++mInsertionCount;

		if (mRoot==NULLNODE)
		{
			mRoot=leaf;
			mNodes[mRoot].mParent=NULLNODE;
			return;
		}

		// Find the best sibling for this node
		AABBRect leafAABB=mNodes[leaf].mAABB;
		int index=mRoot;
		while (mNodes[index].IsLeaf()==false)
		{
			int child1=mNodes[index].mChild1;
			int child2=mNodes[index].mChild2;

			float area=mNodes[index].mAABB.GetPerimeter();

			AABBRect combinedAABB;
			combinedAABB.Combine(mNodes[index].mAABB, leafAABB);
			float combinedArea=combinedAABB.GetPerimeter();

			// Cost of creating a new parent for this node and the new leaf
			float cost=2.0f * combinedArea;

			// Minimum cost of pushing the leaf further down the tree
			float inheritanceCost=2.0f * (combinedArea-area);

			// Cost of descending into child1
			float cost1;
			if (mNodes[child1].IsLeaf())
			{
				AABBRect aabb;
				aabb.Combine(leafAABB, mNodes[child1].mAABB);
				cost1=aabb.GetPerimeter()+inheritanceCost;
			}
			else
			{
				AABBRect aabb;
				aabb.Combine(leafAABB, mNodes[child1].mAABB);
				float oldArea=mNodes[child1].mAABB.GetPerimeter();
				float newArea=aabb.GetPerimeter();
				cost1=(newArea-oldArea)+inheritanceCost;
			}

			// Cost of descending into child2
			float cost2;
			if (mNodes[child2].IsLeaf())
			{
				AABBRect aabb;
				aabb.Combine(leafAABB, mNodes[child2].mAABB);
				cost2=aabb.GetPerimeter()+inheritanceCost;
			}
			else
			{
				AABBRect aabb;
				aabb.Combine(leafAABB, mNodes[child2].mAABB);
				float oldArea=mNodes[child2].mAABB.GetPerimeter();
				float newArea=aabb.GetPerimeter();
				cost2=newArea-oldArea+inheritanceCost;
			}

			// Descend according to the minimum cost.
			if (cost<cost1 && cost<cost2)
			{
				break;
			}

			// Descend
			if (cost1<cost2)
			{
				index=child1;
			}
			else
			{
				index=child2;
			}
		}

		int sibling=index;

		// Create a new parent.
		int oldParent=mNodes[sibling].mParent;
		int newParent=AllocateNode();
		mNodes[newParent].mParent=oldParent;
		mNodes[newParent].mData=NULL;
		mNodes[newParent].mAABB.Combine(leafAABB, mNodes[sibling].mAABB);
		mNodes[newParent].mHeight=mNodes[sibling].mHeight+1;

		if (oldParent!=NULLNODE)
		{
			// The sibling was not the root.
			if (mNodes[oldParent].mChild1==sibling)
			{
				mNodes[oldParent].mChild1=newParent;
			}
			else
			{
				mNodes[oldParent].mChild2=newParent;
			}

			mNodes[newParent].mChild1=sibling;
			mNodes[newParent].mChild2=leaf;
			mNodes[sibling].mParent=newParent;
			mNodes[leaf].mParent=newParent;
		}
		else
		{
			// The sibling was the root.
			mNodes[newParent].mChild1=sibling;
			mNodes[newParent].mChild2=leaf;
			mNodes[sibling].mParent=newParent;
			mNodes[leaf].mParent=newParent;
			mRoot=newParent;
		}
		// Walk back up the tree fixing heights and AABBs
		index=mNodes[leaf].mParent;
		while (index!=NULLNODE)
		{
			index=Balance(index);
			int child1=mNodes[index].mChild1;
			int child2=mNodes[index].mChild2;
			assert(child1!=NULLNODE);
			assert(child2!=NULLNODE);
			mNodes[index].mHeight=1+_max(mNodes[child1].mHeight, mNodes[child2].mHeight);
			mNodes[index].mAABB.Combine(mNodes[child1].mAABB, mNodes[child2].mAABB);
			index=mNodes[index].mParent;
		}
	}

	void						RemoveLeaf(int leaf)
	{
		if (leaf==mRoot)
		{
			mRoot=NULLNODE;
			return;
		}

		int parent=mNodes[leaf].mParent;
		int grandParent=mNodes[parent].mParent;
		int sibling;
		if (mNodes[parent].mChild1==leaf)
		{
			sibling=mNodes[parent].mChild2;
		}
		else
		{
			sibling=mNodes[parent].mChild1;
		}

		if (grandParent!=NULLNODE)
		{
			// Destroy parent and connect sibling to grandParent.
			if (mNodes[grandParent].mChild1==parent)
			{
				mNodes[grandParent].mChild1=sibling;
			}
			else
			{
				mNodes[grandParent].mChild2=sibling;
			}
			mNodes[sibling].mParent=grandParent;
			FreeNode(parent);

			// Adjust ancestor bounds.
			int index=grandParent;
			while (index!=NULLNODE)
			{
				index=Balance(index);

				int child1=mNodes[index].mChild1;
				int child2=mNodes[index].mChild2;

				mNodes[index].mAABB.Combine(mNodes[child1].mAABB, mNodes[child2].mAABB);
				mNodes[index].mHeight=1+_max(mNodes[child1].mHeight, mNodes[child2].mHeight);

				index=mNodes[index].mParent;
			}
		}
		else
		{
			mRoot=sibling;
			mNodes[sibling].mParent=NULLNODE;
			FreeNode(parent);
		}

		//Validate();
	}

	int							Balance(int iA)
	{
		assert(iA!=NULLNODE);

		PointerTreeNode<var_type>* A=mNodes+iA;
		if (A->IsLeaf() || A->mHeight<2)
		{
			return iA;
		}

		int iB=A->mChild1;
		int iC=A->mChild2;
		assert(0<= iB && iB<mNodeCapacity);
		assert(0<= iC && iC<mNodeCapacity);

		PointerTreeNode<var_type>* B=mNodes+iB;
		PointerTreeNode<var_type>* C=mNodes+iC;

		int balance=C->mHeight-B->mHeight;

		// Rotate C up
		if (balance > 1)
		{
			int iF=C->mChild1;
			int iG=C->mChild2;
			PointerTreeNode<var_type>* F=mNodes+iF;
			PointerTreeNode<var_type>* G=mNodes+iG;
			assert(0<= iF && iF<mNodeCapacity);
			assert(0<= iG && iG<mNodeCapacity);

			// Swap A and C
			C->mChild1=iA;
			C->mParent=A->mParent;
			A->mParent=iC;

			// A's old parent should point to C
			if (C->mParent!=NULLNODE)
			{
				if (mNodes[C->mParent].mChild1==iA)
				{
					mNodes[C->mParent].mChild1=iC;
				}
				else
				{
					assert(mNodes[C->mParent].mChild2==iA);
					mNodes[C->mParent].mChild2=iC;
				}
			}
			else
			{
				mRoot=iC;
			}

			// Rotate
			if (F->mHeight > G->mHeight)
			{
				C->mChild2=iF;
				A->mChild2=iG;
				G->mParent=iA;
				A->mAABB.Combine(B->mAABB, G->mAABB);
				C->mAABB.Combine(A->mAABB, F->mAABB);

				A->mHeight=1+_max(B->mHeight, G->mHeight);
				C->mHeight=1+_max(A->mHeight, F->mHeight);
			}
			else
			{
				C->mChild2=iG;
				A->mChild2=iF;
				F->mParent=iA;
				A->mAABB.Combine(B->mAABB, F->mAABB);
				C->mAABB.Combine(A->mAABB, G->mAABB);

				A->mHeight=1+_max(B->mHeight, F->mHeight);
				C->mHeight=1+_max(A->mHeight, G->mHeight);
			}

			return iC;
		}

		// Rotate B up
		if (balance<-1)
		{
			int iD=B->mChild1;
			int iE=B->mChild2;
			PointerTreeNode<var_type>* D=mNodes+iD;
			PointerTreeNode<var_type>* E=mNodes+iE;
			assert(0<= iD && iD<mNodeCapacity);
			assert(0<= iE && iE<mNodeCapacity);

			// Swap A and B
			B->mChild1=iA;
			B->mParent=A->mParent;
			A->mParent=iB;

			// A's old parent should point to B
			if (B->mParent!=NULLNODE)
			{
				if (mNodes[B->mParent].mChild1==iA)
				{
					mNodes[B->mParent].mChild1=iB;
				}
				else
				{
					assert(mNodes[B->mParent].mChild2==iA);
					mNodes[B->mParent].mChild2=iB;
				}
			}
			else
			{
				mRoot=iB;
			}

			// Rotate
			if (D->mHeight > E->mHeight)
			{
				B->mChild2=iD;
				A->mChild1=iE;
				E->mParent=iA;
				A->mAABB.Combine(C->mAABB, E->mAABB);
				B->mAABB.Combine(A->mAABB, D->mAABB);

				A->mHeight=1+_max(C->mHeight, E->mHeight);
				B->mHeight=1+_max(A->mHeight, D->mHeight);
			}
			else
			{
				B->mChild2=iE;
				A->mChild1=iD;
				D->mParent=iA;
				A->mAABB.Combine(C->mAABB, D->mAABB);
				B->mAABB.Combine(A->mAABB, E->mAABB);

				A->mHeight=1+_max(C->mHeight, D->mHeight);
				B->mHeight=1+_max(A->mHeight, E->mHeight);
			}

			return iB;
		}

		return iA;
	}

	inline int					ComputeHeight() {return ComputeHeight(mRoot);}
	int							ComputeHeight(int nodeId)
	{
		assert(0<= nodeId && nodeId<mNodeCapacity);
		PointerTreeNode<var_type>* node=mNodes+nodeId;
		if (node->IsLeaf())	return 0;
		int height1=ComputeHeight(node->mChild1);
		int height2=ComputeHeight(node->mChild2);
		return 1+_max(height1, height2);
	}

	void						ValidateStructure(int index)
	{
		if (index==NULLNODE) return;
		if (index==mRoot) assert(mNodes[index].mParent==NULLNODE);
		PointerTreeNode<var_type>* node=mNodes+index;
		int child1=node->mChild1;
		int child2=node->mChild2;
		if (node->IsLeaf())
		{
			assert(child1==NULLNODE);
			assert(child2==NULLNODE);
			assert(node->mHeight==0);
			return;
		}
		assert(0<= child1 && child1<mNodeCapacity);
		assert(0<= child2 && child2<mNodeCapacity);
		assert(mNodes[child1].mParent==index);
		assert(mNodes[child2].mParent==index);
		ValidateStructure(child1);
		ValidateStructure(child2);
	}

	void						ValidateMetrics(int index) 
	{
		if (index==NULLNODE) return;
		PointerTreeNode<var_type>* node=mNodes+index;
		int child1=node->mChild1;
		int child2=node->mChild2;
		if (node->IsLeaf())
		{
			assert(child1==NULLNODE);
			assert(child2==NULLNODE);
			assert(node->mHeight==0);
			return;
		}
		assert(0<=child1 && child1<mNodeCapacity);
		assert(0<=child2 && child2<mNodeCapacity);
		int height1=mNodes[child1].mHeight;
		int height2=mNodes[child2].mHeight;
		int height;
		height=1+_max(height1, height2);
		assert(node->mHeight==height);
		AABBRect aabb;
		aabb.Combine(mNodes[child1].mAABB, mNodes[child2].mAABB);
		assert(aabb.LowerBound()==node->mAABB.LowerBound());
		assert(aabb.UpperBound()==node->mAABB.UpperBound());
		ValidateMetrics(child1);
		ValidateMetrics(child2);
	}

	int						mRoot;

	PointerTreeNode<var_type>*	mNodes;
	int							mNodeCount;
	int						mNodeCapacity;
	int						mFreeList;
	unsigned int				mPath;
	int							mInsertionCount;

	AABBRect					GetNodeAABB(int theNode) {return mNodes[theNode].mAABB;}

	//
	// Just here as a reference... to show how to draw it for debug purposes.
	//
	/*
#ifdef _DEBUG
	void					Debug(float theScalar=1.0f)
	{
		for (int aCount=0;aCount<mNodeCount;aCount++)
		{
			PointerTreeNode<var_type>* node=mNodes+aCount;

			gG.DrawLine(node->mAABB.UpperLeft()*theScalar,node->mAABB.UpperRight()*theScalar,1);
			gG.DrawLine(node->mAABB.UpperRight()*theScalar,node->mAABB.LowerRight()*theScalar,1);
			gG.DrawLine(node->mAABB.LowerRight()*theScalar,node->mAABB.LowerLeft()*theScalar,1);
			gG.DrawLine(node->mAABB.LowerLeft()*theScalar,node->mAABB.UpperLeft()*theScalar,1);
		}
	}
#endif
	 */
};

#ifndef Tree
#define SmartTree(n) PointerTree<Smart(n),SmartList(n)>
#define Tree(n) PointerTree<n,List>
#endif

