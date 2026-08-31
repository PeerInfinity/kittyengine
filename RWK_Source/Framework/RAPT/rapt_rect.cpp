#include "rapt_rect.h"
#include "rapt_app.h"
#include "rapt_string.h"


Rect::Rect(void)
{
	mX=0;
	mY=0;
	mWidth=0;
	mHeight=0;
}

char* Rect::ToString() 
{
	String& aResult=GetDiscardableString();
	aResult=Sprintf("%.2f,%.2f - %.2f,%.2f",mX,mY,mWidth,mHeight);
	return aResult.c();
}

void Rect::Tweak(String theCommand)
{
	if (theCommand.Len()==0) return;


	String aToken=theCommand.GetToken(';');
	Rect aOriginal=*this;
	while (aToken.Len())
	{
		int aEQ=aToken.Find('=');if (aEQ<1) continue;
		float aValue=gMath.Eval(aToken.GetSegment(aEQ+1,99999));

		bool aReset=false;
		if (aToken[aEQ-1]=='-') aValue*=-1;
		else if (aToken[aEQ-1]!='+') aReset=true;

		float* aV1=NULL;
		float* aV2=NULL;

		char aT=aToken[0];
		if (aT=='m') aT=aToken[1];

		switch(aT)
		{
		case 'X':case 'x':aV1=&mX;break;
		case 'Y':case 'y':aV1=&mY;break;
		case 'W':case 'w':case 'R':case 'r':aV1=&mW;break;
		case 'H':case 'h':case 'B':case 'b':aV1=&mH;break;
		case 'L':case 'l':aV1=&mX;aV2=&mW;break;
		case 'T':case 't':aV1=&mY;aV2=&mH;break;
		}

		if (aV1) {if (aReset) *aV1=0;*aV1+=aValue;}
		if (aV2) 
		{
			if (aReset)
			{
				if (aV1==&mX) aValue=mX-aOriginal.mX;
				if (aV1==&mY) aValue=mY-aOriginal.mY;
			}
			*aV2-=aValue;
		}

		aToken=theCommand.GetNextToken(';');
	}
}


char* Circle::ToString() 
{
	String& aResult=GetDiscardableString();
	aResult=Sprintf("%s:%s",mPos.ToString(),mRadius.ToString());
	return aResult.c();
}

char* Line::ToString() 
{
	String& aResult=GetDiscardableString();
	aResult=Sprintf("%.2f,%.2f - %.2f,%.2f",mPos[0].mX,mPos[0].mY,mPos[1].mX,mPos[1].mY);
	return aResult.c();
}

char* Line3D::ToString() 
{
	String& aResult=GetDiscardableString();
	aResult=Sprintf("%.2f,%.2f,%.2f - %.2f,%.2f,%.2f",mPos[0].mX,mPos[0].mY,mPos[0].mZ,mPos[1].mX,mPos[1].mY,mPos[1].mZ);
	return aResult.c();
}

char* XYRect::ToString() 
{
	String& aResult=GetDiscardableString();
	aResult=Sprintf("%d,%d - %d,%d",mX1,mY1,mX2,mY2);
	return aResult.c();
}

char* AABBRect::ToString() 
{
	String& aResult=GetDiscardableString();
	aResult=Sprintf("%.2f,%.2f - %.2f,%.2f",mX1,mY1,mX2,mY2);
	return aResult.c();
}

char* AABBCube::ToString() 
{
	String& aResult=GetDiscardableString();
	aResult=Sprintf("%.2f,%.2f,%.2f - %.2f,%.2f,%.2f",mX1,mY1,mZ1,mX2,mY2,mZ2);
	return aResult.c();
}


void Rect::Size()
{
	if (gAppPtr) Size(gG.GetPageRect());
}

void Rect::Size(float theWidth, float theHeight)
{
	if (gAppPtr) Size((gG.WidthF()/2)-(theWidth/2),(gG.HeightF()/2)-(theHeight/2),theWidth,theHeight);
}


Rect::Rect(float theX, float theY, float theWidth, float theHeight)
{
	Resize(theX,theY,theWidth,theHeight);
}

Rect::Rect(Point theP1, Point theP2)
{
	Resize(theP1,theP2);
}

Rect::Rect(Point theP1)
{
	Size(theP1);
}

void Rect::Resize(Point theP1, Point theP2)
{
	Point aUL=Point(_min(theP1.mX,theP2.mX),_min(theP1.mY,theP2.mY));
	Point aLR=Point(_max(theP1.mX,theP2.mX),_max(theP1.mY,theP2.mY));
	aLR-=aUL;

	Resize(aUL.mX,aUL.mY,aLR.mX,aLR.mY);
}

Rect Rect::ExpandPercent(float theAmount)
{
	Point aCenter=Center();

	Point aUpperLeft=UpperLeft()-Center();
	Point aLowerRight=LowerRight()-Center();

	aUpperLeft*=theAmount;
	aLowerRight*=theAmount;

	aUpperLeft+=Center();
	aLowerRight+=Center();

	return Rect(aUpperLeft,aLowerRight);
}

Rect Rect::ExpandPercentH(float theAmount)
{
	Point aCenter=Center();
	float aY1=mY;
	float aY2=mY+mHeight;

	Point aUpperLeft=UpperLeft()-Center();
	Point aLowerRight=LowerRight()-Center();

	aUpperLeft*=theAmount;
	aLowerRight*=theAmount;

	aUpperLeft+=Center();
	aLowerRight+=Center();
	aUpperLeft.mY=aY1;
	aLowerRight.mY=aY2;

	return Rect(aUpperLeft,aLowerRight);
}


void Rect::Resize(float theX, float theY, float theWidth, float theHeight)
{
	mX=theX;
	mY=theY;
	mWidth=theWidth;
	mHeight=theHeight;
}

bool Rect::ContainsPoint(float theX, float theY)
{
	return (theX>=mX && theY>=mY && theX<(mX+mWidth) && theY<(mY+mHeight));
}


void Rect::MoveInto(Rect theInRect)
{
	Point aTranslate;
	if (mX<theInRect.mX) aTranslate.mX+=theInRect.mX-mX;
	if (mY<theInRect.mY) aTranslate.mY+=theInRect.mY-mY;

	Point aLR=LowerRight();
	Point aInLR=theInRect.LowerRight();

	if (aLR.mX>aInLR.mX) aTranslate.mX-=aLR.mX-aInLR.mX;
	if (aLR.mY>aInLR.mY) aTranslate.mY-=aLR.mY-aInLR.mY;

	mX+=aTranslate.mX;
	mY+=aTranslate.mY;
}

void Rect::ClipInto(Rect theIntoRect)
{
	float aX1=mX;
	float aY1=mY;
	float aX2=mX+mWidth;
	float aY2=mY+mHeight;

	if (aX1<theIntoRect.mX) aX1=theIntoRect.mX;
	if (aX2<theIntoRect.mX) aX2=theIntoRect.mX;
	if (aY1<theIntoRect.mY) aY1=theIntoRect.mY;
	if (aY2<theIntoRect.mY) aY2=theIntoRect.mY;

	if (aX1>(theIntoRect.mX+theIntoRect.mWidth)) aX1=(theIntoRect.mX+theIntoRect.mWidth);
	if (aX2>(theIntoRect.mX+theIntoRect.mWidth)) aX2=(theIntoRect.mX+theIntoRect.mWidth);
	if (aY1>(theIntoRect.mY+theIntoRect.mHeight)) aY1=(theIntoRect.mY+theIntoRect.mHeight);
	if (aY2>(theIntoRect.mY+theIntoRect.mHeight)) aY2=(theIntoRect.mY+theIntoRect.mHeight);

	mX=aX1;
	mY=aY1;
	mWidth=(aX2-aX1);
	mHeight=(aY2-aY1);
}

Rect Rect::GetClipInto(Rect theIntoRect)
{
	float aX1=mX;
	float aY1=mY;
	float aX2=mX+mWidth;
	float aY2=mY+mHeight;

	if (aX1<theIntoRect.mX) aX1=theIntoRect.mX;
	if (aX2<theIntoRect.mX) aX2=theIntoRect.mX;
	if (aY1<theIntoRect.mY) aY1=theIntoRect.mY;
	if (aY2<theIntoRect.mY) aY2=theIntoRect.mY;

	if (aX1>(theIntoRect.mX+theIntoRect.mWidth)) aX1=(theIntoRect.mX+theIntoRect.mWidth);
	if (aX2>(theIntoRect.mX+theIntoRect.mWidth)) aX2=(theIntoRect.mX+theIntoRect.mWidth);
	if (aY1>(theIntoRect.mY+theIntoRect.mHeight)) aY1=(theIntoRect.mY+theIntoRect.mHeight);
	if (aY2>(theIntoRect.mY+theIntoRect.mHeight)) aY2=(theIntoRect.mY+theIntoRect.mHeight);

	return Rect(aX1,aY1,(aX2-aX1),(aY2-aY1));
}


void Rect::CenterInto(Rect theIntoRect)
{
	mX=theIntoRect.Center().mX-(mWidth/2);
	mY=theIntoRect.Center().mY-(mHeight/2);
}


void Rect::Confine(Rect theIntoRect)
{
	if (mX<theIntoRect.mX) mX=theIntoRect.mX;
	if (mY<theIntoRect.mY) mY=theIntoRect.mY;

	if (mX+mWidth>=theIntoRect.mX+theIntoRect.mWidth) mX=(theIntoRect.mX+theIntoRect.mWidth)-mWidth;
	if (mY+mHeight>=theIntoRect.mY+theIntoRect.mHeight) mY=(theIntoRect.mY+theIntoRect.mHeight)-mHeight;
}

Point Rect::ClampPoint(Point thePos)
{
	thePos.mX=_max(mX,thePos.mX);
	thePos.mX=_min(mX+mWidth,thePos.mX);
	thePos.mY=_max(mY,thePos.mY);
	thePos.mY=_min(mY+mHeight,thePos.mY);
	return thePos;
}


bool Rect::Intersects(float theX, float theY, float theWidth, float theHeight)
{
	return !((theX+theWidth<=mX) || (theY+theHeight<=mY) || (theX>=mX+mWidth) || (theY>=mY+mHeight));
}

Rect Rect::GetIntersection(Rect &theRect)
{
	float x1=_max(mX,theRect.mX);
	float x2=_min(mX+mWidth,theRect.mX+theRect.mWidth);
	float y1=_max(mY,theRect.mY);
	float y2=_min(mY+mHeight,theRect.mY+theRect.mHeight);

	if (((x2-x1)<0) || ((y2-y1)<0)) return Rect(0,0,0,0);
	else return Rect(x1,y1,x2-x1,y2-y1);
}

/*
Rect Rect::Union(Point thePoint)
{
	if (mX==0 && mY==0 && mWidth==0 && mHeight==0)
	{
		//
		// Our rect is null right now, so just return the union'd rect.
		//
		return Rect(thePoint.mX,thePoint.mY,0,0);
	}

	Point aMe1=UpperLeft();
	Point aMe2=LowerRight();

	float aX1=_min(aMe1.mX,thePoint.mX);
	float aX2=_max(aMe2.mX,thePoint.mX);
	float aY1=_min(aMe1.mY,thePoint.mY);
	float aY2=_max(aMe2.mY,thePoint.mY);

	return Rect(aX1,aY1,_max(mWidth,aX2-aX1-1),_max(mHeight,aY2-aY1-1));

}
*/


Rect Rect::Union(Rect theRect)	
{
	if (mX==0 && mY==0 && mWidth==0 && mHeight==0)
	{
		//
		// Our rect is null right now, so just return the union'd rect.
		//
		if (theRect.mW==0 && theRect.mH==0) {theRect.mW=.0000001f;} // Just to ensure we're not "null"
		return theRect;
	}

	float x1 = _min(mX, theRect.mX);
	float x2 = _max(mX + mWidth, theRect.mX + theRect.mWidth);
	float y1 = _min(mY, theRect.mY);
	float y2 = _max(mY + mHeight, theRect.mY + theRect.mHeight);
	if (x2==x1 && y2==y1)
	{
		x2=x1+.0000001f;
	}
	return Rect(x1, y1, x2 - x1, y2 - y1);
}


//
// IRect
//

IRect::IRect(void)
{
	mX=0;
	mY=0;
	mWidth=0;
	mHeight=0;
}

void IRect::Size()
{
	if (gAppPtr) Size(gG.GetPageRect());
}


IRect::IRect(int theX, int theY, int theWidth, int theHeight)
{
	Resize(theX,theY,theWidth,theHeight);
}

void IRect::Resize(int theX, int theY, int theWidth, int theHeight)
{
	mX=theX;
	mY=theY;
	mWidth=theWidth;
	mHeight=theHeight;
}

bool IRect::ContainsPoint(int theX, int theY)
{
	return (theX>=mX && theY>=mY && theX<(mX+mWidth) && theY<(mY+mHeight));
}

void IRect::ClipInto(IRect theIntoRect)
{
	int aX1=mX;
	int aY1=mY;
	int aX2=mX+mWidth;
	int aY2=mY+mHeight;

	if (aX1<theIntoRect.mX) aX1=theIntoRect.mX;
	if (aX2<theIntoRect.mX) aX2=theIntoRect.mX;
	if (aY1<theIntoRect.mY) aY1=theIntoRect.mY;
	if (aY2<theIntoRect.mY) aY2=theIntoRect.mY;

	if (aX1>(theIntoRect.mX+theIntoRect.mWidth)) aX1=(theIntoRect.mX+theIntoRect.mWidth);
	if (aX2>(theIntoRect.mX+theIntoRect.mWidth)) aX2=(theIntoRect.mX+theIntoRect.mWidth);
	if (aY1>(theIntoRect.mY+theIntoRect.mHeight)) aY1=(theIntoRect.mY+theIntoRect.mHeight);
	if (aY2>(theIntoRect.mY+theIntoRect.mHeight)) aY2=(theIntoRect.mY+theIntoRect.mHeight);

	mX=aX1;
	mY=aY1;
	mWidth=(aX2-aX1);
	mHeight=(aY2-aY1);
}


bool IRect::Intersects(int theX, int theY, int theWidth, int theHeight)
{
	return !((theX+theWidth<=mX) || (theY+theHeight<=mY) || (theX>=mX+mWidth) || (theY>=mY+mHeight));
}

IRect IRect::GetIntersection(IRect &theRect)
{
	int x1=_max(mX,theRect.mX);
	int x2=_min(mX+mWidth,theRect.mX+theRect.mWidth);
	int y1=_max(mY,theRect.mY);
	int y2=_min(mY+mHeight,theRect.mY+theRect.mHeight);

	if (((x2-x1)<0) || ((y2-y1)<0)) return IRect(0,0,0,0);
	else return IRect(x1,y1,x2-x1,y2-y1);
}

IRect IRect::Union(IRect theRect)	
{
	if (mX==0 && mY==0 && mWidth==0 && mHeight==0)
	{
		//
		// Our rect is null right now, so just return the union'd rect.
		//
		return theRect;
	}
	else
	{
		int x1 = _min(mX, theRect.mX);
		int x2 = _max(mX + mWidth, theRect.mX + theRect.mWidth);
		int y1 = _min(mY, theRect.mY);
		int y2 = _max(mY + mHeight, theRect.mY + theRect.mHeight);
		return IRect(x1, y1, x2 - x1, y2 - y1);
	}
}






//
// Poly...
//

Poly::Poly(void)
{
	mBoundingRect=Rect(-1,-1);
	mSize=0;
	mCount=0;
	mSub=0;
	mPoint=0;
	mPointBase=0;
}

Poly::~Poly(void)
{
	Clear();
}

void Poly::AddPoint(float theX, float theY)
{
	mCount++;
	if(mCount > mSize)Size(mCount * 2 + 1);
	mPointBase[mCount-1]=mPoint[mCount-1]=Point(theX,theY);
	mPointBase[mCount-1]=mPoint[mCount-1]=Point(theX,theY);
	mBoundingRect=Rect(-1,-1);
}

void Poly::AddUniquePoint(float theX, float theY)
{
	for (int aCount=0;aCount<mCount;aCount++) if (mPoint[aCount].mX==theX && mPoint[aCount].mY==theY) return;
	AddPoint(theX,theY);
}


Poly::Interval Poly::FindInterval(Point theAxis)
{
	Poly::Interval aReturn;
	//Project each point from poly to the axis using dot product!
	//Keep track of minimum value and maximum value, these represent
	//the interval that the Poly spans on this axis in 1D.
	float aDot = aReturn.mMin = aReturn.mMax = mPoint[0].Dot(theAxis);
	for(int i=1;i<mCount;i++)
	{
		aDot=mPoint[i].Dot(theAxis);
		if(aDot<aReturn.mMin)aReturn.mMin=aDot;
		else if(aDot>aReturn.mMax)aReturn.mMax=aDot;
	}
	return aReturn;
}

Poly::Interval Poly::FindInterval(Point theAxis, Point theRelativeVelocity)
{
	Poly::Interval aReturn;
	float aDot = aReturn.mMin = aReturn.mMax = mPoint[0].Dot(theAxis);
	for(int i=1;i<mCount;i++)
	{
		aDot=mPoint[i].Dot(theAxis);
		if(aDot<aReturn.mMin)aReturn.mMin=aDot;
		else if(aDot>aReturn.mMax)aReturn.mMax=aDot;
	}
	//Factor in the relative velocity.
	float aVelocityAdjustment = theRelativeVelocity.Dot(theAxis);
	if (aVelocityAdjustment < 0)aReturn.mMin += aVelocityAdjustment;
	else aReturn.mMax += aVelocityAdjustment;
	return aReturn;
}

Poly::Interval Poly::FindInterval(SubPoly *theSub, Point theAxis)
{
	Poly::Interval aReturn;
	float aDot = aReturn.mMin = aReturn.mMax = theSub->mPoint[theSub->mIndex[0]].Dot(theAxis);
	for(int i=1;i<theSub->mCount;i++)
	{
		aDot=theSub->mPoint[theSub->mIndex[i]].Dot(theAxis);
		if(aDot<aReturn.mMin)aReturn.mMin=aDot;
		else if(aDot>aReturn.mMax)aReturn.mMax=aDot;
	}
	return aReturn;
}

Poly::Interval Poly::FindInterval(SubPoly *theSub, Point theAxis, Point theRelativeVelocity)
{
	Poly::Interval aReturn;
	float aDot = aReturn.mMin = aReturn.mMax = theSub->mPoint[theSub->mIndex[0]].Dot(theAxis);
	for(int i=1;i<theSub->mCount;i++)
	{
		aDot=theSub->mPoint[theSub->mIndex[i]].Dot(theAxis);
		if(aDot<aReturn.mMin)aReturn.mMin=aDot;
		else if(aDot>aReturn.mMax)aReturn.mMax=aDot;
	}

	float aVelocityAdjustment = theRelativeVelocity.Dot(theAxis);
	if (aVelocityAdjustment < 0)aReturn.mMin += aVelocityAdjustment;
	else aReturn.mMax += aVelocityAdjustment;
	return aReturn;
}

Poly::Interval Poly::FindInterval(Point *thePoints, int thePointCount, Point theAxis, Point theRelativeVelocity)
{
	Interval aReturn;
	float aDot = aReturn.mMin = aReturn.mMax = thePoints[0].Dot(theAxis);
	for(int i=1;i<thePointCount;i++)
	{
		aDot=thePoints[i].Dot(theAxis);
		if(aDot<aReturn.mMin)aReturn.mMin=aDot;
		else if(aDot>aReturn.mMax)aReturn.mMax=aDot;
	}

	float aVelocityAdjustment = theRelativeVelocity.Dot(theAxis);
	if (aVelocityAdjustment < 0)aReturn.mMin += aVelocityAdjustment;
	else aReturn.mMax += aVelocityAdjustment;
	return aReturn;
}

Poly::Interval Poly::FindInterval(Point *thePoints, int thePointCount, Point theAxis)
{
	Interval aReturn;
	//Project each point from poly to the axis using dot product!
	//Keep track of minimum value and maximum value, these represent
	//the interval that the Poly spans on this axis in 1D.
	float aDot = aReturn.mMin = aReturn.mMax = thePoints[0].Dot(theAxis);
	for(int i=1;i<thePointCount;i++)
	{
		aDot=thePoints[i].Dot(theAxis);
		if(aDot<aReturn.mMin)aReturn.mMin=aDot;
		else if(aDot>aReturn.mMax)aReturn.mMax=aDot;
	}
	return aReturn;
}

bool Poly::Intersects(Poly *thePoly)
{
	if(mCount<2 || thePoly->GetVertexCount()<2)return false;

	if(mSub)
	{
		if (thePoly->mSub)return mSub->Intersects(thePoly->mSub);
		else return thePoly->Intersects(mSub);
	}
	else if (thePoly->mSub)return Intersects(thePoly->mSub);

	for(int aStart=0,aEnd=mCount-1;aStart<mCount;aEnd=aStart++)
	{
		Point aEdge(mPoint[aEnd]-mPoint[aStart]);
		Point aAxis(Point(-aEdge.mY,aEdge.mX));
		if(!FindInterval(aAxis).
			Intersects(thePoly->FindInterval(aAxis)))return false;
	}
	for(int aStart=0,aEnd=thePoly->mCount-1;aStart<thePoly->mCount;aEnd=aStart++)
	{
		Point aEdge(thePoly->mPoint[aEnd]-thePoly->mPoint[aStart]);
		Point aAxis(-aEdge.mY,aEdge.mX);
		if(!FindInterval(aAxis).
			Intersects(thePoly->FindInterval(thePoly->mPoint,thePoly->mCount,aAxis)))return false;
	}
	return true;
}

bool Poly::Intersects(SubPolyList *theList)
{
	for(int i=0;i<theList->mCount;i++)
	{
		if(Intersects(theList->mSub[i]))return true;
	}
	return false;
}

bool Poly::SubPolyList::Intersects(SubPolyList *theList)
{
	for(int i=0;i<mCount;i++)
	{
		for(int n=0;n<theList->mCount;n++)
		{
			if(mSub[i]->Intersects(theList->mSub[n]))return true;
		}
	}
	return false;
}

bool Poly::SubPolyList::WillIntersect(SubPolyList*theList,Point theRelativeVelocity)
{
	for(int i=0;i<mCount;i++)
	{
		for(int n=0;n<theList->mCount;n++)
		{
			if(mSub[i]->WillIntersect(theList->mSub[n], theRelativeVelocity))return true;
		}
	}
	return false;
}

bool Poly::SubPoly::Intersects(SubPoly *theSub)
{
	for(int aStart=0,aEnd=mCount-1;aStart<mCount;aEnd=aStart++)
	{
		Point aEdge(mPoint[mIndex[aEnd]]-mPoint[mIndex[aStart]]);
		Point aAxis(Point(-aEdge.mY,aEdge.mX));
		if(!Poly::FindInterval(this,aAxis).
			Intersects(Poly::FindInterval(theSub,aAxis)))return false;
	}
	for(int aStart=0,aEnd=theSub->mCount-1;aStart<theSub->mCount;aEnd=aStart++)
	{
		Point aEdge(theSub->mPoint[theSub->mIndex[aEnd]]-theSub->mPoint[theSub->mIndex[aStart]]);
		Point aAxis(Point(-aEdge.mY,aEdge.mX));
		if(!Poly::FindInterval(this,aAxis).
			Intersects(Poly::FindInterval(theSub,aAxis)))return false;
	}
	return true;
}

bool Poly::SubPoly::WillIntersect(SubPoly *theSub, Point theRelativeVelocity)
{
	for(int aStart=0,aEnd=mCount-1;aStart<mCount;aEnd=aStart++)
	{
		Point aEdge(mPoint[mIndex[aEnd]]-mPoint[mIndex[aStart]]);
		Point aAxis(Point(-aEdge.mY,aEdge.mX));
		if(!Poly::FindInterval(this,aAxis,theRelativeVelocity).
			Intersects(Poly::FindInterval(theSub,aAxis)))return false;
	}
	for(int aStart=0,aEnd=theSub->mCount-1;aStart<theSub->mCount;aEnd=aStart++)
	{
		Point aEdge(theSub->mPoint[theSub->mIndex[aEnd]]-theSub->mPoint[theSub->mIndex[aStart]]);
		Point aAxis(Point(-aEdge.mY,aEdge.mX));
		if(!Poly::FindInterval(this,aAxis,theRelativeVelocity).
			Intersects(Poly::FindInterval(theSub,aAxis)))return false;
	}
	return true;
}

bool Poly::Intersects(SubPoly *theSub)
{
	for(int aStart=0,aEnd=mCount-1;aStart<mCount;aEnd=aStart++)
	{
		Point aEdge(mPoint[aEnd]-mPoint[aStart]);
		Point aAxis(Point(-aEdge.mY,aEdge.mX));
		if(!FindInterval(aAxis).
			Intersects(FindInterval(theSub,aAxis)))return false;
	}

	for(int aStart=0,aEnd=theSub->mCount-1;aStart<theSub->mCount;aEnd=aStart++)
	{
		Point aEdge(theSub->mPoint[theSub->mIndex[aEnd]]-theSub->mPoint[theSub->mIndex[aStart]]);
		Point aAxis(Point(-aEdge.mY,aEdge.mX));
		if(!FindInterval(aAxis).
			Intersects(FindInterval(theSub,aAxis)))return false;
	}
	return true;
}

bool Poly::WillIntersect(Poly*thePoly,Point *theTranslationVector,Point theVelocity1,Point theVelocity2)
{
	if(mCount<2 || thePoly->GetVertexCount()<2)return false;

	//Get relative velocity.
	Point aRelativeVelocity = theVelocity1 - theVelocity2;

	float aMinDistance=5000;
	Point aTranslationAxis;

	//For each edge in the first Poly...
	for(int aStart=0,aEnd=mCount-1;aStart<mCount;aEnd=aStart++)
	{
		//Pick separation axis perpindicular to edge!
		Point aEdge(mPoint[aEnd]-mPoint[aStart]);
		Point aAxis(-aEdge.mY,aEdge.mX);
		aAxis.Normalize();

		Interval aInt1=FindInterval(aRelativeVelocity);
		Interval aInt2=thePoly->FindInterval(aAxis);

		//Difference between each Poly along the current axis.
		//If this is <=0, they overlap on this axis, so we must keep
		//on checkin'!
		float aDistanceBetween=aInt1.DistanceBetween(aInt2);
		if(aDistanceBetween>0)
		{
			return false;
		}
		else
		{
			aDistanceBetween=(float)fabs(aDistanceBetween);
			if(aDistanceBetween < aMinDistance)
			{
				aMinDistance=aDistanceBetween;
				aTranslationAxis=aAxis;
				Point aDist(mCenter - thePoly->mCenter);
				if(aDist.Dot(aTranslationAxis)<0)aTranslationAxis=-aTranslationAxis;
			}
		}
	}

	//Repeat all this malarkey for the second Poly!
	for(int aStart=0,aEnd=thePoly->mCount-1;aStart<thePoly->mCount;aEnd=aStart++)
	{
		//Pick separation axis perpindicular to edge!
		Point aEdge(thePoly->mPoint[aEnd]-thePoly->mPoint[aStart]);
		Point aAxis(-aEdge.mY,aEdge.mX);
		aAxis.Normalize();

		Interval aInt1=FindInterval(aAxis,aRelativeVelocity);
		Interval aInt2=thePoly->FindInterval(aAxis);

		//Difference between each Poly along the current axis.
		//If this is <=0, they overlap on this axis, so we must keep
		//on checkin'!
		float aDistanceBetween=aInt1.DistanceBetween(aInt2);
		if(aDistanceBetween>0)
		{
			return false;
		}
		else
		{
			aDistanceBetween=(float)fabs(aDistanceBetween);
			if(aDistanceBetween < aMinDistance)
			{
				aMinDistance=aDistanceBetween;
				aTranslationAxis=aAxis;
				Point aDist(mCenter - thePoly->mCenter);
				if(aDist.Dot(aTranslationAxis)<0)
				{
					aTranslationAxis=-aTranslationAxis;
				}
			}
		}
	}

	if(theTranslationVector)
	{
		theTranslationVector->mX=aTranslationAxis.mX*aMinDistance;
		theTranslationVector->mY=aTranslationAxis.mY*aMinDistance;
	}
	return true;
}

bool Poly::WillIntersect(Poly*thePoly,Point theVelocity1,Point theVelocity2)
{
	if(mCount<2 || thePoly->GetVertexCount()<2)return false;

	Point aRelativeVelocity = theVelocity1 - theVelocity2;

	if(mSub)
	{
		if (thePoly->mSub)return mSub->WillIntersect(thePoly->mSub,aRelativeVelocity);
		else return thePoly->WillIntersect(mSub,aRelativeVelocity);
	}
	else if (thePoly->mSub)return WillIntersect(thePoly->mSub,aRelativeVelocity);


	for(int aStart=0,aEnd=mCount-1;aStart<mCount;aEnd=aStart++)
	{
		Point aEdge(mPoint[aEnd]-mPoint[aStart]);
		Point aAxis(-aEdge.mY,aEdge.mX);
		aAxis.Normalize();

		if(!FindInterval(aAxis, aRelativeVelocity).Intersects(
			thePoly->FindInterval(aAxis)))return false;
	}
	for(int aStart=0,aEnd=thePoly->mCount-1;aStart<thePoly->mCount;aEnd=aStart++)
	{
		Point aEdge(thePoly->mPoint[aEnd]-thePoly->mPoint[aStart]);
		Point aAxis(-aEdge.mY,aEdge.mX);
		aAxis.Normalize();

		if(!FindInterval(aAxis, aRelativeVelocity).Intersects(
			thePoly->FindInterval(aAxis)))return false;
	}
	return true;
}

bool Poly::WillIntersect(SubPoly*theSub, Point theRelativeVelocity)
{
	for(int aStart=0,aEnd=mCount-1;aStart<mCount;aEnd=aStart++)
	{
		Point aEdge(mPoint[aEnd]-mPoint[aStart]);
		Point aAxis(Point(-aEdge.mY,aEdge.mX));
		if(!FindInterval(aAxis, theRelativeVelocity).
			Intersects(FindInterval(theSub,aAxis)))return false;
	}

	for(int aStart=0,aEnd=theSub->mCount-1;aStart<theSub->mCount;aEnd=aStart++)
	{
		Point aEdge(theSub->mPoint[theSub->mIndex[aEnd]]-theSub->mPoint[theSub->mIndex[aStart]]);
		Point aAxis(Point(-aEdge.mY,aEdge.mX));
		if(!FindInterval(aAxis,theRelativeVelocity).
			Intersects(FindInterval(theSub,aAxis)))return false;
	}
	return true;
}

bool Poly::WillIntersect(SubPolyList*theList, Point theRelativeVelocity)
{
	for(int i=0;i<theList->mCount;i++)
	{
		if(WillIntersect(theList->mSub[i],theRelativeVelocity))return true;
	}
	return false;
}

void Poly::Transform(Matrix theMatrix)
{
	mBoundingRect=Rect(-1,-1);
	mCenter=mCenterBase+Point(theMatrix.mData.m[3][0],theMatrix.mData.m[3][1]);
	for(int i=0; i<mCount; i++)
	{
		Point aPoint(mPointBase[i]-mCenterBase);
		mPoint[i]=Point(
		aPoint.mX*theMatrix.mData.m[0][0]+
		aPoint.mY*theMatrix.mData.m[1][0]+
		mCenter.mX,

		aPoint.mX*theMatrix.mData.m[0][1]+
		aPoint.mY*theMatrix.mData.m[1][1]+
		mCenter.mY);
	}
}

void Poly::Translate(float theX, float theY)
{
	mBoundingRect=Rect(-1,-1);
	for(int i=0; i<mCount; i++)
	{
		mPointBase[i].mX+=theX;
		mPointBase[i].mY+=theY;
		mPoint[i].mX+=theX;
		mPoint[i].mY+=theY;
	}
}

void Poly::Inflate(Point theCenter, float theAmount)
{
	mBoundingRect=Rect(-1,-1);
	for (int aCount=0;aCount<mCount;aCount++) 
	{
		Point aVec=mPointBase[aCount]-theCenter;
		aVec.Normalize();
		mPointBase[aCount]+=aVec*theAmount;

		aVec=mPoint[aCount]-theCenter;
		aVec.Normalize();
		mPoint[aCount]+=aVec*theAmount;

	}

}

void Poly::Transform(Point theTranslate, float theRotation, float theScale)
{
	mBoundingRect=Rect(-1,-1);
	mTranslate=theTranslate;
	mCenter=mCenterBase+mTranslate;

	float m00=gMath.Cos(theRotation) * theScale;
	float m01=-gMath.Sin(theRotation) * theScale;
	float m10=gMath.Sin(theRotation) * theScale;
	float m11=gMath.Cos(theRotation) * theScale;

	for(int i=0; i<mCount; i++)
	{
		Point aPoint(mCenterBase-mPointBase[i]);
		mPoint[i]=Point(
			aPoint.mX*m00+
			aPoint.mY*m10+
			mCenter.mX,
			aPoint.mX*m01+
			aPoint.mY*m11+
			mCenter.mY);
	}
}

float Poly::GetBoundingRadius()
{
	if(mCount>0)
	{
		if (!mPoint) mPoint=mPointBase;
		float aMaxDist=gMath.Distance(mPoint[0],mCenterBase);
		for(int i=1; i<mCount; i++)
		{
			float aDist=gMath.Distance(mPoint[i],mCenterBase);
			if(aDist>aMaxDist)aMaxDist=aDist;
		}
		if (mPoint==mPointBase) mPoint=NULL;
		return aMaxDist;
	}
	else return 0;
}

Rect Poly::GetBoundingRect()
{
	if (mBoundingRect.mWidth<0)
	{
		if(mCount>0)
		{
			if (!mPoint) mPoint=mPointBase;

			float aLeft,aRight,aTop,aBottom;
			aLeft=aRight=mPoint[0].mX;
			aTop=aBottom=mPoint[0].mY;
			for(int i=1;i<mCount;i++)
			{
				if(mPoint[i].mX<aLeft)aLeft=mPoint[i].mX;
				else if(mPoint[i].mX>aRight)aRight=mPoint[i].mX;

				if(mPoint[i].mY<aTop)aTop=mPoint[i].mY;
				else if(mPoint[i].mY>aBottom)aBottom=mPoint[i].mY;
			}
			if (mPoint==mPointBase) mPoint=NULL;
			mBoundingRect=Rect(aLeft,aTop,_max(0,aRight-aLeft),_max(0,aBottom-aTop));
		}
		else mBoundingRect=Rect(-1,-1);
	}

	return mBoundingRect;
}

Rect Poly::GetBoundingBox()
{
	if(mCount>0)
	{
		float aRadius=GetBoundingRadius();
		float aRadius2=aRadius+aRadius;
		return Rect(mCenterBase.mX-aRadius,mCenterBase.mY-aRadius,aRadius2,aRadius2);
	}
	else return Rect(0,0,0,0);
}

void Poly::Tidy()
{
	Point *aPointBase = mPointBase;
	Point *aPoint = mPoint;

	mPointBase=new Point[mCount];
	mPoint=new Point[mCount];

	for(int i=0; i<mCount; i++)
	{
		mPointBase[i]=aPointBase[i];
		mPoint[i]=aPoint[i];
	}

	if(aPointBase)delete[]aPointBase;
	if(aPoint)delete[]aPoint;

	mSize = mCount;
}

void Poly::Size(int theSize)
{
	Point *aPointBase = mPointBase;
	Point *aPoint = mPoint;
	mPointBase = new Point[theSize];
	mPoint = new Point[theSize];
	for(int i=0; i<mSize; i++)
	{
		mPointBase[i]=aPointBase[i];
		mPoint[i]=aPoint[i];
	}
	if(aPointBase)delete[]aPointBase;
	if(aPoint)delete[]aPoint;
	mSize = theSize;
}

void Poly::Draw()
{
	for(int aStart=0;aStart<mCount;aStart++)
	{
		gG.FillRect(mPoint[aStart].mX-2,mPoint[aStart].mY-2,5,5);
	}
	if(mSub)
	{
		for(int i=0;i<mSub->mCount;i++)
		{
			for(int aStart=0,aEnd=mSub->mSub[i]->mCount-1;aStart<mSub->mSub[i]->mCount;aEnd=aStart++)
			{
				gG.DrawLine(mPoint[mSub->mSub[i]->mIndex[aStart]], mPoint[mSub->mSub[i]->mIndex[aEnd]]);
			}
		}
	}
	else
	{
		for(int aStart=0,aEnd=mCount-1;aStart<mCount;aEnd=aStart++)
		{
			gG.DrawLine(mPoint[aStart], mPoint[aEnd]);
		}
	}
}

Point Poly::GetCentroid()
{
	Point aCentroid;
	float aAreaSum=0;
	for(int aStart=0,aEnd=mCount-1;aStart<mCount;aEnd=aStart++)
	{
		float aCross=(float)mPointBase[aStart].Cross(mPointBase[aEnd]);
		aAreaSum+=aCross;
		aCentroid+=Point((mPointBase[aStart].mX+mPointBase[aEnd].mX)*aCross,(mPointBase[aStart].mY+mPointBase[aEnd].mY)*aCross);
	}
	return aCentroid/(aAreaSum*3)+mTranslate;
}

float Poly::GetInertia()
{
	float aNumerator=0;
	for(int aStart=0,aEnd=mCount-1;aStart<mCount;aEnd=aStart++)
	{
		float aCross = (float)fabs(mPoint[aEnd].Cross(mPoint[aStart]));
		float aDotSum = (mPoint[aStart].Dot(mPoint[aStart]) + mPoint[aStart].Dot(mPoint[aEnd]) + mPoint[aEnd].Dot(mPoint[aEnd]));
		aNumerator += (aCross * aDotSum);
	}
	return aNumerator/12;
}

float Poly::GetArea()
{
	float aAreaSum=0;
	for(int aStart=0,aEnd=mCount-1;aStart<mCount;aEnd=aStart++)
	{
		aAreaSum+=(float)fabs(mPoint[aStart].Cross(mPoint[aEnd]));
	}
	return aAreaSum/2;
}

bool Poly::Intersects(float theX, float theY)
{
	if(mCount < 2)return false;

	bool aWinding=false;
	for (int aI=0,aJ=mCount-1;aI<mCount;aJ=aI++) if (((mPoint[aI].mY>theY)!=(mPoint[aJ].mY>theY)) && (theX<(mPoint[aJ].mX-mPoint[aI].mX)*(theY-mPoint[aI].mY)/(mPoint[aJ].mY-mPoint[aI].mY)+mPoint[aI].mX)) aWinding=!aWinding;
	return aWinding;
/*
	bool aReturn = false;
	for(int aStart=0,aEnd=mCount-1;aStart<mCount;aEnd=aStart++)
	{
		if ((((mPoint[aStart].mY<=theY) && (theY<mPoint[aEnd].mY))||
			((mPoint[aEnd].mY<=theY) && (theY<mPoint[aStart].mY)))&&
			(theX < (mPoint[aEnd].mX - mPoint[aStart].mX)*(theY - mPoint[aStart].mY)
			/(mPoint[aEnd].mY - mPoint[aStart].mY) + mPoint[aStart].mX))
			aReturn=!aReturn;
	}
	return aReturn;
*/
}

void Poly::Clear()
{
	delete[]mPoint;
	delete[]mPointBase;
	if(mSub)
	{
		mSub->Clear();
		delete mSub;
	}
	mSub=0;
	mSize=0;
	mCount=0;
	mTranslate=Point(0,0);
	mPoint=0;
	mPointBase=0;
}

void Poly::Untriangulate()
{
	if(mSub)
	{
		mSub->Clear();
		delete mSub;
	}
	mSub=0;
}


Point Poly::Center()
{
	mCenterBase=GetCentroid();
	mCenter=mCenterBase;
	return mCenter;
}

void Poly::CenterAt(Point thePoint)
{
	Center();
	Point aSubtract=mCenterBase-thePoint;
	for(int i=0; i<mCount; i++)
	{
		mPointBase[i]-=aSubtract;
		mPoint[i]-=aSubtract;
	}
	mCenterBase=mCenter=thePoint;
}

void Poly::MakeRegular(Point theCenter, int theVertices, float theRadii)
{
	Clear();
	Size(theVertices);
	float aAdd=360.0f/(float)theVertices;
	float aAngle=0;
	for(float i=0;i<theVertices;i++)
	{
		AddPoint(gMath.AngleToVector(aAngle)*theRadii+theCenter);
		aAngle+=aAdd;
	}
}
bool Poly::Intersects(float theX, float theY, float theWidth, float theHeight)
{
	Point aRectPoint[4]={Point(theX,theY),Point(theX+theWidth,theY),Point(theX+theWidth,theY+theHeight),Point(theX,theY+theHeight)};
	for(int aStart=0,aEnd=mCount-1;aStart<mCount;aEnd=aStart++)
	{
		Point aEdge(mPoint[aEnd]-mPoint[aStart]);
		Point aAxis(Point(-aEdge.mY,aEdge.mX));
		if(!FindInterval(mPoint,mCount,aAxis).
			Intersects(FindInterval(aRectPoint,4,aAxis)))return false;
	}
	if(!FindInterval(mPoint,mCount,Point(0,1)).Intersects(Interval(theY,aRectPoint[2].mY)))return false;
	if(!FindInterval(mPoint,mCount,Point(1,0)).Intersects(Interval(theX,aRectPoint[2].mX)))return false;
	return true;
}
bool Poly::Intersects(Line theLine)
{
	for(int aStart=0,aEnd=mCount-1;aStart<mCount;aEnd=aStart++)
	{
		if(SegmentsIntersect(theLine.mPos[0],theLine.mPos[1],mPoint[aStart],mPoint[aEnd]))
			return true;
	}
	return false;
}

bool Poly::Intersects(Point theCenter, Point theRadii)
{
	for(int aStart=0,aEnd=mCount-1;aStart<mCount;aEnd=aStart++)
	{
		if(gMath.EllipseIntersectLine(theCenter,theRadii,mPoint[aStart],mPoint[aEnd])) return true;
	}
	return false;
}

bool Poly::IsClockwise(Point aStart, Point aMiddle, Point aEnd)
{
	return (aMiddle.mX-aStart.mX)*(aEnd.mY-aMiddle.mY)-(aEnd.mX-aMiddle.mX)*(aMiddle.mY-aStart.mY) >= 0;
}

bool Poly::IsClockwise()
{
	float aAreaSum=0;
	for(int aStart=0,aEnd=mCount-1;aStart<mCount;aEnd=aStart++)aAreaSum+=mPoint[aStart].Cross(mPoint[aEnd]);
	return aAreaSum < 0;
}

bool Poly::IsConvex()
{
	if(mCount<=3)return true;
	bool aCW=IsClockwise(mPointBase[0],mPointBase[1],mPointBase[2]);
	for(int aEnd=0,aMiddle=mCount-1,aStart=mCount-2;aEnd<mCount;aEnd++)
	{
		if(aCW != IsClockwise(mPointBase[aStart],mPointBase[aMiddle],mPointBase[aEnd]))return false;
		aStart=aMiddle;
		aMiddle=aEnd;
	}
	return true;
}

void Poly::ReverseDirection()
{
	int aCeil=mCount-1;
	Point *aTemp=mPointBase;
	mPointBase=new Point[mCount];
	for(int i=0;i<=aCeil;i++)
	{
		mPointBase[i]=aTemp[aCeil-i];
	}
	delete[]aTemp;
}

void Poly::GetTriangle(int theIndex, Point *thePoint1, Point *thePoint2, Point *thePoint3)
{
	thePoint1->mX=mPoint[mSub->mSub[theIndex]->mIndex[0]].mX;
	thePoint1->mY=mPoint[mSub->mSub[theIndex]->mIndex[0]].mY;

	thePoint2->mX=mPoint[mSub->mSub[theIndex]->mIndex[1]].mX;
	thePoint2->mY=mPoint[mSub->mSub[theIndex]->mIndex[1]].mY;

	thePoint3->mX=mPoint[mSub->mSub[theIndex]->mIndex[2]].mX;
	thePoint3->mY=mPoint[mSub->mSub[theIndex]->mIndex[2]].mY;
}

void Poly::GetTriangleIndexes(int theIndex, int& theIndex1, int& theIndex2, int& theIndex3)
{
	theIndex1=mSub->mSub[theIndex]->mIndex[0];
	theIndex2=mSub->mSub[theIndex]->mIndex[1];
	theIndex3=mSub->mSub[theIndex]->mIndex[2];
}


void Poly::Triangulate()
{
	if(mSub)
	{
		mSub->Clear();
		delete mSub;
		mSub=0;
	}

	if(mCount < 3)
	{
		return;
	}

	mSub=new SubPolyList(this);

	int aCount=mCount;
	//List of indeces that are still to be considered.
	int *aIndex=new int[aCount];

	//If it's counterclockwise,
	//put the index list in reverse order!
	if(IsCounterClockwise())
	{
		int aCeil=aCount-1;
		for(int i=0;i<=aCeil;i++)aIndex[i]=aCeil-i;
	}
	else for(int i=0;i<aCount;i++)aIndex[i]=i;

	mSub->Size(mCount);

	SubPoly *aSub;
	bool aContinue=true;
	while(aCount>3 && aContinue)
	{
		aContinue=false;
		for(int i=0;i<aCount&&aCount>3;i++)
		{
			//If it's an ear, clip it off!
			if(IsEar(aIndex,aCount,i))
			{
				aContinue=true;

				int aNext=i+1;
				if(aNext==aCount)aNext=0;
				int aPrev=i-1;
				if(aPrev==-1)aPrev=aCount-1;

				//Triangle consists of this ear,
				//and the points before and after it.
				aSub=new SubPoly(mPoint);
				aSub->mCount=3;
				aSub->mIndex=new int[3];
				aSub->mIndex[0]=aIndex[aPrev];
				aSub->mIndex[1]=aIndex[i];
				aSub->mIndex[2]=aIndex[aNext];
				*mSub+=aSub;

				//Remove the ear from the list.
				aCount--;
				for(int n=i;n<aCount;n++)aIndex[n]=aIndex[n+1];
			}
		}
	}

	//Add all remaining points, if all went
	//according to plan, this should be
	//3 points, the final triangle...

	aSub=new SubPoly(mPoint);
	aSub->mCount=aCount;
	aSub->mIndex=new int[aCount];
	for(int i=0;i<aCount;i++)
	{
		aSub->mIndex[i]=aIndex[i];
	}
	*mSub+=aSub;

	mSub->Tidy();

	delete [] aIndex;
}

void Poly::Triangulate(Array<vbindex>& theResult)
{
	Triangulate();
	int aPCount=GetSubPolyCount();
	theResult.ForceSize(aPCount*3);

	int aPut=0;
	for (int aCount=0;aCount<aPCount;aCount++)
	{
		int aI1,aI2,aI3;
		GetTriangleIndexes(aCount,aI1,aI2,aI3);
		theResult[aPut++]=aI1;
		theResult[aPut++]=aI2;
		theResult[aPut++]=aI3;
		//gGX.AddTri3D(aP.mPoint[aI1],aP.mPoint[aI2],aP.mPoint[aI3]);
	}
}



//If the point is clockwise or counterclockwise from all 3 pairs,
//it must be in the triangle! Hurray!
bool Poly::TriangleContainsPoint(Point theT1, Point theT2, Point theT3, Point thePoint)
{
	bool aDirection=(thePoint.mX-theT1.mX)*(theT2.mY-thePoint.mY)-(theT2.mX-thePoint.mX)*(thePoint.mY-theT1.mY) >=0;
	if(aDirection!=((thePoint.mX-theT2.mX)*(theT3.mY-thePoint.mY)-(theT3.mX-thePoint.mX)*(thePoint.mY-theT2.mY) >=0))return false;
	return aDirection==((thePoint.mX-theT3.mX)*(theT1.mY-thePoint.mY)-(theT1.mX-thePoint.mX)*(thePoint.mY-theT3.mY) >=0);
}

bool Poly::IsEar(int *theIndexList, int theVertexCount, int theIndex)
{
	int aNext=theIndex+1;
	if(aNext==theVertexCount)aNext=0;
	int aPrev=theIndex-1;
	if(aPrev==-1)aPrev=theVertexCount-1;

	//The triangle with second point at the middle vertex.
	Point aPrevPoint(mPointBase[theIndexList[aPrev]]);
	Point aMiddlePoint(mPointBase[theIndexList[theIndex]]);
	Point aNextPoint(mPointBase[theIndexList[aNext]]);

	//If it's a concave point, there's no way it's an ear!
	if(IsCounterClockwise(aPrevPoint,aMiddlePoint,aNextPoint))return false;

	//If the potential ear's triangle contains any other points from the
	//Poly, it's not an ear at all! Nuke the imposter!
	for(int i=0;i<theIndex-1;i++)
	{
		if(TriangleContainsPoint(aPrevPoint,aMiddlePoint,aNextPoint,mPointBase[theIndexList[i]]))
		{
			return false;
		}
	}
	for(int i=theIndex+2;i<theVertexCount;i++)
	{
		if(TriangleContainsPoint(aPrevPoint,aMiddlePoint,aNextPoint,mPointBase[theIndexList[i]]))
		{
			return false;
		}
	}
	return true;
}

void Poly::SubPolyList::Size(int theSize)
{
	SubPoly **aSub=mSub;
	mSub=new SubPoly*[theSize];
	for(int i=0; i<mSize; i++)
	{
		mSub[i]=aSub[i];
	}
	delete[]aSub;
	mSize=theSize;
}

void Poly::SubPolyList::operator+=(SubPoly *theSubPoly)
{
	mCount++;
	if(mCount > mSize)Size(mCount * 2 + 1);
	mSub[mCount-1]=theSubPoly;
}

void Poly::SubPolyList::Tidy()
{
	if(mCount == mSize)return;
	SubPoly **aSub=mSub;
	mSub=new SubPoly*[mCount];
	for(int i=0; i<mCount; i++)
	{
		mSub[i]=aSub[i];
	}
	delete[]aSub;
	mSize=mCount;
}

void Poly::SubPolyList::Clear()
{
	for(int i=0;i<mCount;i++)delete mSub[i];
	delete[]mSub;
	mSub=0;
	mCount=0;
	mSize=0;
}

inline float Poly::TriangleArea(float x1, float y1, float x2, float y2, float x3, float y3) 
{
	return (x2 - x1) * (y3 - y1) - (x3 - x1) * (y2 - y1);
}

inline bool Poly::Between(float x1, float y1, float x2, float y2, float x3, float y3) 
{
	if (x1 != x2)return (x1 <= x3 && x3 <= x2) || (x1 >= x3 && x3 >= x2);   
	else return (y1 <= y3 && y3 <= y2) || (y1 >= y3 && y3 >= y2);   
}

bool Poly::SegmentsIntersect(Point theStart1, Point theEnd1, Point theStart2, Point theEnd2)
{
	float aArea1, aArea2, aArea3, aArea4;
	if((aArea1 = TriangleArea(theStart1.mX, theStart1.mY, theEnd1.mX, theEnd1.mY, theStart2.mX, theStart2.mY)) == 0) 
	{
		if (Between(theStart1.mX, theStart1.mY, theEnd1.mX, theEnd1.mY, theStart2.mX, theStart2.mY)) 
		{
			return true;
		}
		else 
		{
			if (TriangleArea(theStart1.mX, theStart1.mY, theEnd1.mX, theEnd1.mY, theEnd2.mX, theEnd2.mY) == 0) 
			{
				if(Between(theStart2.mX, theStart2.mY, theEnd2.mX, theEnd2.mY, theStart1.mX, theStart1.mY))
				{
					return true;
				}
				if(Between (theStart2.mX, theStart2.mY, theEnd2.mX, theEnd2.mY, theEnd1.mX, theEnd1.mY))
				{
					return true;
				}
				return false;
			}
			else return false;
		}
	}
	if ((aArea2 = TriangleArea(theStart1.mX, theStart1.mY, theEnd1.mX, theEnd1.mY, theEnd2.mX, theEnd2.mY)) == 0) 
	{
		return Between(theStart1.mX, theStart1.mY, theEnd1.mX, theEnd1.mY, theEnd2.mX, theEnd2.mY);
	}
	if ((aArea3 = TriangleArea(theStart2.mX, theStart2.mY, theEnd2.mX, theEnd2.mY, theStart1.mX, theStart1.mY)) == 0)
	{
		if (Between(theStart2.mX, theStart2.mY, theEnd2.mX, theEnd2.mY, theStart1.mX, theStart1.mY))
		{
			return true;
		}
		else
		{
			if (TriangleArea(theStart2.mX, theStart2.mY, theEnd2.mX, theEnd2.mY, theEnd1.mX, theEnd1.mY) == 0)
			{
				if(Between(theStart1.mX, theStart1.mY, theEnd1.mX, theEnd1.mY, theStart2.mX, theStart2.mY))
				{
					return true;
				}
				if(Between (theStart1.mX, theStart1.mY, theEnd1.mX, theEnd1.mY, theEnd2.mX, theEnd2.mY))
				{
					return true;
				}
				return false;
			}
			return false;
		}
	}
	if ((aArea4 = TriangleArea(theStart2.mX, theStart2.mY, theEnd2.mX, theEnd2.mY, theEnd1.mX, theEnd1.mY)) == 0)
	{
		return Between(theStart2.mX, theStart2.mY, theEnd2.mX, theEnd2.mY, theEnd1.mX, theEnd1.mY);
	}
	return (((aArea1 > 0) ^ (aArea2 > 0)) && ((aArea3 > 0) ^ (aArea4 > 0)));
}

bool Poly::Diagonal(int theIndex1, int theIndex2)
{
	if(abs(theIndex1 - theIndex2) <= 1)return true;
	if(theIndex1 > theIndex2)
	{
		int aHold=theIndex1;
		theIndex1=theIndex2;
		theIndex2=aHold;
	}
	for(int i=1;i<theIndex1;i++)
	{
		if(SegmentsIntersect(mPointBase[theIndex1], mPointBase[theIndex2], mPointBase[i-1], mPointBase[i]))return false;
	}
	for(int i=theIndex1+2;i<theIndex2;i++)
	{
		if(SegmentsIntersect(mPointBase[theIndex1], mPointBase[theIndex2], mPointBase[i-1], mPointBase[i]))return false;
	}
	for(int i=theIndex2+2;i<mCount;i++)
	{
		if(SegmentsIntersect(mPointBase[theIndex1], mPointBase[theIndex2], mPointBase[i-1], mPointBase[i]))return false;
	}
	if(theIndex1 != 0 && theIndex2 != mCount-1)
	{
		if(SegmentsIntersect(mPointBase[theIndex1], mPointBase[theIndex2], mPointBase[0], mPointBase[mCount-1]))return false;
	}

	Point aMidpoint=(mPointBase[theIndex1] + mPointBase[theIndex2]) * 0.5f;

	bool aReturn = false;
	for(int aStart=0,aEnd=mCount-1;aStart<mCount;aEnd=aStart++)
	{
		if ((((mPointBase[aStart].mY<=aMidpoint.mY) && (aMidpoint.mY<mPointBase[aEnd].mY))||
			((mPointBase[aEnd].mY<=aMidpoint.mY) && (aMidpoint.mY<mPointBase[aStart].mY)))&&
			(aMidpoint.mX < (mPointBase[aEnd].mX - mPointBase[aStart].mX)*(aMidpoint.mY - mPointBase[aStart].mY)
			/(mPointBase[aEnd].mY - mPointBase[aStart].mY) + mPointBase[aStart].mX))
			aReturn=!aReturn;
	}
	return aReturn;
}

bool Poly::IsSimple()
{
	for(int i=1;i<mCount-2;i++)
	{
		for(int n=i+2; n<mCount; n++)
		{
			if(SegmentsIntersect(mPointBase[i], mPointBase[i-1],
				mPointBase[n], mPointBase[n-1]))
			{
				return false;
			}
		}
	}
	for(int i=2; i<mCount-1; i++)
	{
		if(SegmentsIntersect(mPointBase[mCount-1],mPointBase[0],
			mPointBase[i],mPointBase[i-1]))
		{
			return false;
		}
	}
	return true;
}

void Poly::Decompose()
{
	if(mSub)
	{
		mSub->Clear();
		delete mSub;
		mSub=0;
	}

	if(mCount <= 3)
	{
		return;
	}

	mSub=new SubPolyList(this);
	mSub->Size(mCount);

	//Stack of unprocecced sub-Polys that may or may
	//not be convex.
	List aStack;
	aStack.GuaranteeSize(mCount);

	//List of indeces that are on the
	//convex partition of the Poly we
	//will be considering.
	int *aOnPolyList=new int[mCount];
	//Every time we find a point that's off of
	//the concex sub Poly, it goes here.
	int *aOffPolyList=new int[mCount];

	//First sub-Poly to consider is the
	//whole fucking thing.
	SubPoly *aWholePoly = new SubPoly(mPoint);
	aWholePoly->mCount=mCount;
	aWholePoly->mIndex=new int[mCount];
	for(int i=0;i<mCount;i++)
	{
		aWholePoly->mIndex[i]=i;
	}
	aStack+=aWholePoly;

	//Until the stack is empty...!
	while(aStack.mCount > 0)
	{
		//Pop the top sub Poly off the stack and rape it.
		SubPoly *aSub=((SubPoly*)aStack.Last());
		aStack.mCount--;

		//Triangles require no extra work.
		if(aSub->mCount<=3)
		{
			*mSub+=aSub;
			continue;
		}

		//This counts the number of clockwise and counter-
		//clockwise kinks on the current sub poly.
		int aStart=aSub->mCount-2;
		int aMiddle=aSub->mCount-1;
		int aEnd=0;
		int aCWCount=0;
		int aCCWCount=0;
		while(aEnd<aSub->mCount)
		{
			if(IsClockwise(mPointBase[aSub->mIndex[aStart]],mPointBase[aSub->mIndex[aMiddle]],mPointBase[aSub->mIndex[aEnd]]))
			{
				aCWCount++;
			}
			else
			{
				aCCWCount++;
			}
			aStart=aMiddle;
			aMiddle=aEnd;
			aEnd++;
		}

		//If it's convex after this inspection, add it!
		if(aCCWCount <= 0 || aCWCount <= 0)
		{
			*mSub+=aSub;
			continue;
		}

		//If it's counter-clockwise, reverse it!
		if(aCCWCount > aCWCount)
		{
			int *aTemp=aSub->mIndex;
			aSub->mIndex=new int[aSub->mCount];
			int aCeil=aSub->mCount-1;
			for(int i=0;i<=aCeil;i++)aSub->mIndex[i]=aTemp[aCeil-i];
			delete[]aTemp;
		}

		//We arbitrarily pick a starting index for our convex sub-Poly.
		aOnPolyList[0]=aSub->mIndex[0];
		//And the next index is part of it..
		aOnPolyList[1]=aSub->mIndex[1];

		aStart=0;
		aMiddle=1;
		aEnd=2;

		int aOnPolyIndex=2;
		int aOffPolyIndex=0;

		//For each vertex after the first two,
		//check if they're part of the convex Poly we're
		//constructing.
		while(aEnd<aSub->mCount-1)
		{
			bool aFound=false;

			/*What we figured out earlier from testing is that
			the next point will be part of the convex subolygon if
			1.) The kink [last point] [this] [first point] is CW
			2.) The kink [second last point] [last point] [this] is CW
			3.)A valid diagonal exists between the two points. A valid
			diagonal does not cross any edges and is inside of the Poly!*/

			if(IsClockwise(mPointBase[aSub->mIndex[aEnd]],
				mPointBase[aSub->mIndex[0]],
				mPointBase[aSub->mIndex[1]]))
			{
				if(IsClockwise(mPointBase[aSub->mIndex[aStart]],
					mPointBase[aSub->mIndex[aMiddle]],
					mPointBase[aSub->mIndex[aEnd]]))
				{
					if(Diagonal(aSub->mIndex[0], aSub->mIndex[aEnd]))
					{
						//Add it to the list!
						aOnPolyList[aOnPolyIndex]=aSub->mIndex[aEnd];
						aOnPolyIndex++;
						aFound=true;
						aStart=aMiddle;
						aMiddle=aEnd;

						//If we have some indeces from another sub poly that
						//is not part of this, we end that sub poly and
						//push it onto the stack.
						if(aOffPolyIndex)
						{
							//This stuff figured out through trial and error.
							SubPoly *aNewSub = new SubPoly(mPoint);
							aNewSub->mCount=aOffPolyIndex+2;
							aNewSub->mIndex=new int[aNewSub->mCount];
							int aLastIndex=aOffPolyList[aOffPolyIndex-1];
							int aFirstIndex=aOffPolyList[0];

							for(int i=0;i<aOffPolyIndex;i++)
							{
								aNewSub->mIndex[i+1]=aOffPolyList[i];
							}
							if(aFirstIndex > aLastIndex)
							{
								aNewSub->mIndex[aOffPolyIndex+1]=aLastIndex-1;
								aNewSub->mIndex[0]=aFirstIndex+1;
							}
							else
							{
								aNewSub->mIndex[0]=aFirstIndex-1;
								aNewSub->mIndex[aOffPolyIndex+1]=aLastIndex+1;
							}
							aStack+=aNewSub;
							aOffPolyIndex=0;
						}
					}
				}
			}
			//If this point is not on the convex sub poly, we add it to the
			//list of indeces off of the "good" list.
			if(!aFound)
			{
				aOffPolyList[aOffPolyIndex]=aSub->mIndex[aEnd];
				aOffPolyIndex++;
			}
			aEnd++;
		}

		/*This is a special case for the last point...
		We follow the first two criterea still:

		1.) The kink [last point] [this] [first point] is CW
		2.) The kink [second last point] [last point] [this] is CW

		but since the last point is right next to the first, we already
		know the valid diagonal exists... it is part of the main Poly!*/

		bool aFound=false;
		if(IsClockwise(mPointBase[aSub->mIndex[aEnd]],
			mPointBase[aSub->mIndex[0]],
			mPointBase[aSub->mIndex[1]]))
		{
			if(IsClockwise(mPointBase[aSub->mIndex[aStart]],
				mPointBase[aSub->mIndex[aMiddle]],
				mPointBase[aSub->mIndex[aEnd]]))
			{
				aOnPolyList[aOnPolyIndex]=aSub->mIndex[aEnd];
				aOnPolyIndex++;
				aFound=true;
			}
		}
		if(!aFound)
		{
			aOffPolyList[aOffPolyIndex]=aSub->mIndex[aEnd];
			aOffPolyIndex++;
		}

		//If there are any lingering indeces on the "bad" Poly list,
		//we have about 50,000 cases to consider because the universe
		//is just not user friendly.

		//I don't really remember why each of these cases is as it is,
		//I figured most of them out through trial and error with educated guessing.
		if(aOffPolyIndex && aOnPolyIndex > 2)
		{
			SubPoly *aNewSub=new SubPoly(mPoint);
			//The only thing consistent is the size of the new bad sub Poly.
			aNewSub->mCount=aOffPolyIndex+2;
			aNewSub->mIndex=new int[aNewSub->mCount];

			int aLastIndex=aOffPolyList[aOffPolyIndex-1];
			int aFirstIndex=aOffPolyList[0];

			//It's a backwards Poly!
			if(aFirstIndex >= aLastIndex)
			{
				//The last index is on the bad Poly.
				if(aFound)
				{
					for(int i=0;i<aOffPolyIndex;i++)
					{
						aNewSub->mIndex[i+1]=aOffPolyList[i];
					}
					aNewSub->mIndex[0]=aFirstIndex+1;
					aNewSub->mIndex[aOffPolyIndex+1]=aLastIndex-1;
				}
				//Last index not on the bad Poly.
				else
				{
					for(int i=0;i<aOffPolyIndex;i++)
					{
						aNewSub->mIndex[i+2]=aOffPolyList[i];
					}
					//If first index == last index... *sigh*
					if(aSub->mIndex[aSub->mCount-1] > aSub->mIndex[0])
					{
						aNewSub->mIndex[1]=aSub->mIndex[aSub->mCount-1]-1;
						aNewSub->mIndex[0]=aSub->mIndex[0];
					}
					else
					{
						aNewSub->mIndex[1]=aFirstIndex+1;
						aNewSub->mIndex[0]=aSub->mIndex[0];
					}

				}
			}
			//It's a nice normal clockwise subPoly!
			else
			{
				//The last index is on our bad poly.
				if(aFound)
				{
					for(int i=0;i<aOffPolyIndex;i++)
					{
						aNewSub->mIndex[i+1]=aOffPolyList[i];
					}
					aNewSub->mIndex[0]=aFirstIndex-1;
					aNewSub->mIndex[aOffPolyIndex+1]=aLastIndex+1;
				}
				//The last index hain't on our bad poly.
				else
				{
					for(int i=0;i<aOffPolyIndex;i++)
					{
						aNewSub->mIndex[i+2]=aOffPolyList[i];
					}
					aNewSub->mIndex[0]=aSub->mIndex[0];
					aNewSub->mIndex[1]=aFirstIndex-1;
				}
			}
			//Push the bad Poly onto the stack to be
			//checked out.
			aStack+=aNewSub;
		}

		//Ahh, the fruit of our labour! A convex subPoly
		//that is ripped out of a nonvex pile of poop.
		SubPoly *aNewConvexSub = new SubPoly(mPoint);
		aNewConvexSub->mIndex=new int[aOnPolyIndex];
		aNewConvexSub->mCount=aOnPolyIndex;
		for(int i=0;i<aOnPolyIndex;i++)
		{
			aNewConvexSub->mIndex[i]=aOnPolyList[i];
		}
		*mSub+=aNewConvexSub;

		//Delete the Poly that we broke down since all its info exists in
		//small subPolys now!
		delete aSub;
	}

	delete[]aOnPolyList;
	delete[]aOffPolyList;

	//This should always be true, but just as a precaution, let's
	//make sure we have at least two sub-Polys.

	if(mSub->mCount >= 2)
	{
		mSub->Tidy();
	}
	else
	{
		mSub->Clear();
		delete mSub;
		mSub=0;
	}
}

bool Triangle::IsSharedVertex(Triangle* theT)
{
	if (mPos[0]==theT->mPos[0]) return true;
	if (mPos[0]==theT->mPos[1]) return true;
	if (mPos[0]==theT->mPos[2]) return true;

	if (mPos[1]==theT->mPos[0]) return true;
	if (mPos[1]==theT->mPos[1]) return true;
	if (mPos[1]==theT->mPos[2]) return true;

	if (mPos[2]==theT->mPos[0]) return true;
	if (mPos[2]==theT->mPos[1]) return true;
	if (mPos[2]==theT->mPos[2]) return true;

	return false;
}

bool Triangle::IsSharedEdge(Triangle* theT, Line* theSharedLine)
{
	int aShared=0;
	int aSharedPoint[2];

	for (int aTestV1=0;aTestV1<3;aTestV1++)
	{
		for (int aTestV2=0;aTestV2<3;aTestV2++)
		{
			if (mPos[aTestV1]==theT->mPos[aTestV2])
			{
				aSharedPoint[aShared++]=aTestV1;
				if (aShared>1)
				{
					aTestV1=9999;
					aTestV2=9999;
					break;
				}
			}
		}
	}

	if (aShared<2) return false;

	if (theSharedLine)
	{
		theSharedLine->mPos[0]=mPos[aSharedPoint[0]];
		theSharedLine->mPos[1]=mPos[aSharedPoint[1]];
	}
	return true;
}

bool Triangle::ContainsPoint(Point thePos)
{
	return gMath.IsPointInTriangle(thePos,this);
}

Rect Triangle::GetBoundingRect()
{
	float aLeft,aRight,aTop,aBottom;
	aLeft=aRight=mPos[0].mX;
	aTop=aBottom=mPos[0].mY;
	for(int i=1;i<3;i++)
	{
		if(mPos[i].mX<aLeft)aLeft=mPos[i].mX;
		else if(mPos[i].mX>aRight)aRight=mPos[i].mX;
			if(mPos[i].mY<aTop)aTop=mPos[i].mY;
		else if(mPos[i].mY>aBottom)aBottom=mPos[i].mY;
	}
	return Rect(aLeft,aTop,_max(0,aRight-aLeft),_max(0,aBottom-aTop));
}

Rect Triangle3D::GetBoundingXYRect()
{
	float aLeft,aRight,aTop,aBottom;
	aLeft=aRight=mPos[0].mX;
	aTop=aBottom=mPos[0].mY;
	for(int i=1;i<3;i++)
	{
		if(mPos[i].mX<aLeft)aLeft=mPos[i].mX;
		else if(mPos[i].mX>aRight)aRight=mPos[i].mX;
		if(mPos[i].mY<aTop)aTop=mPos[i].mY;
		else if(mPos[i].mY>aBottom)aBottom=mPos[i].mY;
	}
	return Rect(aLeft,aTop,_max(0,aRight-aLeft),_max(0,aBottom-aTop));
}

AABBRect Triangle3D::GetBoundingXYRectAABB()
{
	float aLeft,aRight,aTop,aBottom;
	aLeft=aRight=mPos[0].mX;
	aTop=aBottom=mPos[0].mY;
	for(int i=1;i<3;i++)
	{
		if(mPos[i].mX<aLeft)aLeft=mPos[i].mX;
		else if(mPos[i].mX>aRight)aRight=mPos[i].mX;
		if(mPos[i].mY<aTop)aTop=mPos[i].mY;
		else if(mPos[i].mY>aBottom)aBottom=mPos[i].mY;
	}
	return AABBRect(aLeft,aTop,aRight,aBottom);
}

Cube Triangle3D::GetBoundingCube()
{
	Vector aP1=mPos[0];
	Vector aP2=mPos[0];
	for (int aCount=1;aCount<3;aCount++)
	{
		aP1.mX=_min(aP1.mX,mPos[aCount].mX);
		aP1.mY=_min(aP1.mY,mPos[aCount].mY);
		aP1.mZ=_min(aP1.mZ,mPos[aCount].mZ);
		aP2.mX=_max(aP2.mX,mPos[aCount].mX);
		aP2.mY=_max(aP2.mY,mPos[aCount].mY);
		aP2.mZ=_max(aP2.mZ,mPos[aCount].mZ);
	}
	return Cube(aP1,aP2);
}


RectComplex::RectComplex(void)
{
	mPerimeterData=NULL;
}

RectComplex::RectComplex(float theX, float theY, float theWidth, float theHeight)
{
	if (theWidth && theHeight) AppendRect(theX,theY,theWidth,theHeight);
	mPerimeterData=NULL;
}

RectComplex::RectComplex(Rect theRect)
{
	if (theRect.mWidth && theRect.mHeight) AppendRect(theRect.mX,theRect.mY,theRect.mWidth,theRect.mHeight);
	mPerimeterData=NULL;
}

RectComplex::RectComplex(RectComplex &theRect)
{
	EnumList(Rect,aRect,theRect.mRectList) Add(*aRect);
	mPerimeterData=NULL;
}


RectComplex::~RectComplex(void)
{
	Reset();
}

float RectComplex::GetArea()
{
	float aArea=0;
	EnumList(Rect,aR,GetRects()) aArea+=(aR->mWidth*aR->mHeight);
	return aArea;
}

void RectComplex::AppendRect(float theX, float theY, float theWidth, float theHeight)
{
	Rect *aRect=new Rect(theX, theY, theWidth, theHeight);
	mRectList+=aRect;
}


void RectComplex::Add(float theX, float theY, float theWidth, float theHeight)
{
	if (!theWidth || !theHeight) return;
	if (mRectList.GetCount()==0) AppendRect(theX,theY,theWidth,theHeight);
	else
	{
		Rect aRect=Rect(theX,theY,theWidth,theHeight);
		RectComplex aR(theX,theY,theWidth,theHeight);
		for (int aCount=GetRectCount();--aCount>=0;)
		{
			Rect aOurRect(GetRect(aCount));
			if (aRect.Intersects(aOurRect))
			{
				aR.Subtract(aOurRect.mX,aOurRect.mY,aOurRect.mWidth,aOurRect.mHeight);
				if (aR.mRectList.GetCount()==0) return;
			}
		}
		for (int aCount=aR.GetRectCount();--aCount>=0;) AppendRect(aR.GetRect(aCount));
	}
}

void RectComplex::Subtract(float theX, float theY, float theWidth, float theHeight)
{
	if (!theWidth || !theHeight) return;

	//
	// Why are these INTs?  Because if they're not, then rounding error in the FPU
	// can cause an infinite loop here.  This means a loss of precision, but
	// since this is primarily used to render with, it should be okay.
	//
	int aX1=(int)theX;
	int aY1=(int)theY;
	int aX2=(int)(theX+theWidth);
	int aY2=(int)(theY+theHeight);

	int aFudge=0;
	for (int aCount=GetRectCount();--aCount>=0;)
	{
		if (++aFudge>5000) 
		{
			gOut.Out("Too many rectangles? (5000+)");
			exit(0);
		}

		Rect *aRect=(Rect*)mRectList[aCount];
		int aRX1=(int)aRect->mX;
		int aRY1=(int)aRect->mY;
		int aRX2=(int)(aRect->mX+aRect->mWidth);
		int aRY2=(int)(aRect->mY+aRect->mHeight);

        if (!(aX2 <= aRX1 || aX1 >= aRX2 || aY2 <= aRY1 || aY1 >= aRY2))
		{
			mRectList-=aRect;
			delete aRect;

			if (aX1>aRX1 && aX1<aRX2)
			{
 				Rect *aRect=new Rect((float)aX1,(float)aRY1,(float)aRX2-(float)aX1,(float)aRY2-(float)aRY1);

				mRectList.Insert(aRect,aCount);
				aRect=new Rect((float)aRX1,(float)aRY1,(float)aX1-(float)aRX1,(float)aRY2-(float)aRY1);

				mRectList.Insert(aRect,aCount);
				aCount+=2;
			}
            else if (aX2>aRX1 && aX2<aRX2)
			{
				Rect *aRect=new Rect((float)aX2,(float)aRY1,(float)aRX2-(float)aX2,(float)aRY2-(float)aRY1);
				mRectList.Insert(aRect,aCount);
				aRect=new Rect((float)aRX1,(float)aRY1,(float)aX2-(float)aRX1,(float)aRY2-(float)aRY1);
				mRectList.Insert(aRect,aCount);
				aCount+=2;
			}
            else if (aY1>aRY1 && aY1<aRY2)
			{
				Rect *aRect=new Rect((float)aRX1,(float)aY1,(float)aRX2-(float)aRX1,(float)aRY2-(float)aY1);
				mRectList.Insert(aRect,aCount);
				aRect=new Rect((float)aRX1,(float)aRY1,(float)aRX2-(float)aRX1,(float)aY1-(float)aRY1);
				mRectList.Insert(aRect,aCount);
				aCount+=2;
			}
            else if (aY2>aRY1 && aY2<aRY2)
			{
				Rect *aRect=new Rect((float)aRX1,(float)aY2,(float)aRX2-(float)aRX1,(float)aRY2-(float)aY2);
				mRectList.Insert(aRect,aCount);
				aRect=new Rect((float)aRX1,(float)aRY1,(float)aRX2-(float)aRX1,(float)aY2-(float)aRY1);
				mRectList.Insert(aRect,aCount);
				aCount+=2;
			}
		}
	}
}

bool RectComplex::ContainsPoint(float theX, float theY)
{
	EnumList(Rect,aRect,mRectList) if (aRect->ContainsPoint(theX,theY)) return true;
	return false;
}

float RectComplex::GetMaxY()
{
	float aBottom=0;
	bool aFirst=true;
	EnumList(Rect,aRect,mRectList) 
	{
		if (aFirst) {aBottom=aRect->LowerRight().mY;aFirst=false;}
		aBottom=_max(aBottom,aRect->LowerRight().mY);
	}
	return aBottom;
}

float RectComplex::GetMinY()
{
	float aTop=0;
	bool aFirst=true;
	EnumList(Rect,aRect,mRectList) 
	{
		if (aFirst) {aTop=aRect->UpperLeft().mY;aFirst=false;}
		aTop=_min(aTop,aRect->UpperLeft().mY);
	}
	return aTop;
}

float RectComplex::GetMinX()
{
	float aLeft=0;
	bool aFirst=true;
	EnumList(Rect,aRect,mRectList)
	{
		if (aFirst) {aLeft=aRect->UpperLeft().mX;aFirst=false;}
		aLeft=_min(aLeft,aRect->UpperLeft().mX);
	}
	return aLeft;
}

float RectComplex::GetMaxX()
{
	float aRight=0;
	bool aFirst=true;
	EnumList(Rect,aRect,mRectList) 
	{
		if (aFirst) {aRight=aRect->LowerRight().mX;aFirst=false;}
		aRight=_max(aRight,aRect->LowerRight().mX);
	}
	return aRight;
}

void RectComplex::Add(RectComplex &theRect)
{
	EnumList(Rect,aRect,theRect.mRectList) Add(*aRect);
}

void RectComplex::operator=(RectComplex &theRect)
{
	Reset();
	EnumList(Rect,aRect,theRect.mRectList) Add(*aRect);
}

void RectComplex::Move(float theX, float theY)
{
	EnumList(Rect,aRect,mRectList)
	{
		aRect->mX+=theX;
		aRect->mY+=theY;
	}
}

void RectComplex::BuildPerimeterCorner(List *theCorners, List *theCornerPoints, List *theCornerNormal, Point thePotentialCorner, float theFudge)
{
	Point aCorner[4];

	aCorner[0]=thePotentialCorner+Point(theFudge,-theFudge);
	aCorner[1]=thePotentialCorner+Point(theFudge,theFudge);
	aCorner[2]=thePotentialCorner+Point(-theFudge,theFudge);
	aCorner[3]=thePotentialCorner+Point(-theFudge,-theFudge);


	bool aFound[4];for(int i=0;i<4;i++)aFound[i]=true;
	
	int aCount=0;
	
	for(int i=0;i<mRectList.mCount&&aCount<4;i++)
	{
		Rect *aRect=(Rect*)mRectList.mData[i];
		for(int i=0;i<4;i++)
		{
			if(aFound[i])
			{
				if(aRect->ContainsPoint(aCorner[i].mX,aCorner[i].mY))
				{
					aFound[i]=false;
					aCount++;
				}
			}
		}
	}
	if(aCount==1 || aCount==3)
	{
		for(int i=0;i<theCorners->mCount;i++)
		{
			RectComplex::Corner *aCorn=(RectComplex::Corner*)theCorners->mData[i];
			if(aCorn->mPos == thePotentialCorner)
			{
				return;
			}
		}

		RectComplex::Corner *aCorner=new RectComplex::Corner(thePotentialCorner);
		Point *aNorm = new Point(0.707107f,0.707107f);

		if(aCount==1)
		{
			if(!aFound[0])
			{
				aCorner->mD=true;
				aCorner->mL=true;
			}
			else if(!aFound[1])
			{
				aCorner->mU=true;
				aCorner->mL=true;
			}
			else if(!aFound[2])
			{
				aCorner->mU=true;
				aCorner->mR=true;
			}
			else
			{
				aCorner->mD=true;
				aCorner->mR=true;
			}

			if(aCorner->mU)aNorm->mY=-0.707107f;
			if(aCorner->mL)aNorm->mX=-0.707107f;
		}
		else
		{
			aCorner->mInner=true;

			if(aFound[0])
			{
				aCorner->mD=true;
				aCorner->mL=true;
			}
			else if(aFound[1])
			{
				aCorner->mU=true;
				aCorner->mL=true;
			}
			else if(aFound[2])
			{
				aCorner->mU=true;
				aCorner->mR=true;
			}
			else
			{
				aCorner->mD=true;
				aCorner->mR=true;
			}

			if(aCorner->mD)aNorm->mY=-0.707107f;
			if(aCorner->mR)aNorm->mX=-0.707107f;
		}
		theCorners->Insert(aCorner);
		theCornerNormal->Insert(aNorm);
		theCornerPoints->Insert(new Point(thePotentialCorner));
	}
}

void RectComplex::CalculatePerimeter(float theFudge)
{

	if(mPerimeterData)delete mPerimeterData;
	mPerimeterData=new PerimeterData();

	for(int i=0;i<mRectList.mCount;i++) 
	{
		Rect *aRect1=(Rect*)mRectList.mData[i];

		Point aUL1(aRect1->mX,aRect1->mY);
		Point aUR1(aRect1->mX+aRect1->mWidth,aRect1->mY);
		Point aLL1(aRect1->mX,aRect1->mY+aRect1->mHeight);
		Point aLR1(aRect1->mX+aRect1->mWidth,aRect1->mY+aRect1->mHeight);

		BuildPerimeterCorner(&mPerimeterData->mCornerData,&mPerimeterData->mCorners,&mPerimeterData->mCornerNormals,aUL1,theFudge);
		BuildPerimeterCorner(&mPerimeterData->mCornerData,&mPerimeterData->mCorners,&mPerimeterData->mCornerNormals,aUR1,theFudge);
		BuildPerimeterCorner(&mPerimeterData->mCornerData,&mPerimeterData->mCorners,&mPerimeterData->mCornerNormals,aLR1,theFudge);
		BuildPerimeterCorner(&mPerimeterData->mCornerData,&mPerimeterData->mCorners,&mPerimeterData->mCornerNormals,aLL1,theFudge);

		for(int n=0;n<mRectList.mCount; n++)
		{
			if(i!=n)
			{
				Rect *aRect2=(Rect*)mRectList.mData[n];

				Point aUL2(aRect2->mX,aRect2->mY);
				Point aUR2(aRect2->mX+aRect2->mWidth,aRect2->mY);
				Point aLL2(aRect2->mX,aRect2->mY+aRect2->mHeight);
				Point aLR2(aRect2->mX+aRect2->mWidth,aRect2->mY+aRect2->mHeight);

				if(aUL1.mX >= aUL2.mX && aUL1.mX <= aUR2.mX)
				{
					if(aLL2.mY > aUL1.mY && aLL2.mY < aLL1.mY)
					{
						BuildPerimeterCorner(&mPerimeterData->mCornerData,&mPerimeterData->mCorners, &mPerimeterData->mCornerNormals,Point(aUL1.mX,aLL2.mY),theFudge);
					}
					if(aUL2.mY > aUL1.mY && aUL2.mY < aLL1.mY)
					{
						BuildPerimeterCorner(&mPerimeterData->mCornerData,&mPerimeterData->mCorners, &mPerimeterData->mCornerNormals,Point(aUL1.mX,aUL2.mY),theFudge);
					}
				}
				if(aUR1.mX > aUL2.mX && aUR1.mX < aUR2.mX)
				{
					if(aLL2.mY > aUL1.mY && aLL2.mY < aLL1.mY)
					{
						BuildPerimeterCorner(&mPerimeterData->mCornerData,&mPerimeterData->mCorners, &mPerimeterData->mCornerNormals,Point(aUR1.mX,aLL2.mY),theFudge);
					}
					if(aUL2.mY > aUL1.mY && aUL2.mY < aLL1.mY)
					{
						BuildPerimeterCorner(&mPerimeterData->mCornerData,&mPerimeterData->mCorners, &mPerimeterData->mCornerNormals,Point(aUR1.mX,aUL2.mY),theFudge);
					}
				}
			}
		}
	}

	mPerimeterData->mLines.GuaranteeSize(mPerimeterData->mCornerData.mCount);
	mPerimeterData->mCornerNormalLines.GuaranteeSize(mPerimeterData->mCornerData.mCount);

	for(int i=mPerimeterData->mCornerData.mCount-1;i>=0;i--)
	{
		Corner *aCorn1=(Corner*)mPerimeterData->mCornerData.mData[i];

		float aH=9999999999999999.0f;
		float aV=9999999999999999.0f;

		Point *aHPoint=0;
		Point *aVPoint=0;

		Corner *aHCorn;
		Corner *aVCorn;

		int aHoldNH;
		int aHoldNV;

		for(int n=0;n<mPerimeterData->mCornerData.mCount;n++)
		{
			if(i!=n)
			{
				Corner *aCorn2=(Corner*)mPerimeterData->mCornerData.mData[n];
				if(aCorn1->mPos.mY==aCorn2->mPos.mY)
				{
					if(aCorn1->mL && aCorn2->mR)
					{
						if(aCorn2->mPos.mX > aCorn1->mPos.mX)
						{
							float aDist=aCorn2->mPos.mX-aCorn1->mPos.mX;
							if(aDist<aH)
							{
								aHoldNH=n;
								aH=aDist;
								aHPoint=&aCorn2->mPos;
								aHCorn=aCorn2;
							}
						}
					}
				}
				else if(aCorn1->mPos.mX==aCorn2->mPos.mX)
				{
					if(aCorn1->mD && aCorn2->mU)
					{
						if(aCorn1->mPos.mY > aCorn2->mPos.mY)
						{
							float aDist=aCorn1->mPos.mY-aCorn2->mPos.mY;
							if(aDist<aV)
							{
								aHoldNV=n;
								aV=aDist;
								aVPoint=&aCorn2->mPos;
								aVCorn=aCorn2;
							}
						}
					}
				}
			}
		}
		if(aHPoint)
		{
			aCorn1->mL=false;
			aHCorn->mR=false;
			mPerimeterData->mLines.Insert(new Line(aCorn1->mPos, *aHPoint));
			mPerimeterData->mCornerNormalLines.Insert(new Line(*((Point*)mPerimeterData->mCornerNormals[i]),*((Point*)mPerimeterData->mCornerNormals[aHoldNH])));
		}
		if(aVPoint)
		{
			aCorn1->mD=false;
			aVCorn->mU=false;
			mPerimeterData->mLines.Insert(new Line(aCorn1->mPos, *aVPoint));
			mPerimeterData->mCornerNormalLines.Insert(new Line(*((Point*)mPerimeterData->mCornerNormals[i]),*((Point*)mPerimeterData->mCornerNormals[aHoldNV])));
		}
	}

	mPerimeterData->mLineNormals.GuaranteeSize(mPerimeterData->mLines.mCount);

	for(int i=mPerimeterData->mLines.mCount-1;i>=0;i--)
	{
		Line *aLine=(Line*)mPerimeterData->mLines.mData[i];
		Point aMid=(aLine->mPos[0]+aLine->mPos[1]) * 0.5f;
		if(aLine->mPos[0].mX != aLine->mPos[1].mX)
		{
			aMid.mY+=theFudge;
			if(ContainsPoint(aMid.mX,aMid.mY))mPerimeterData->mLineNormals.Insert(new Point(0,-1));
			else mPerimeterData->mLineNormals.Insert(new Point(0,1));
		}
		else
		{
			aMid.mX+=theFudge;
			if(ContainsPoint(aMid.mX,aMid.mY))mPerimeterData->mLineNormals.Insert(new Point(-1,0));
			else mPerimeterData->mLineNormals.Insert(new Point(1,0));
		}
	}
}

bool RectComplex::Intersects(RectComplex &theRectComplex)
{
	EnumList(Rect,aRect,mRectList)
	{
		EnumList(Rect,aRect2,theRectComplex.mRectList)
		{
			if (aRect->Intersects(*aRect2)) return true;
		}
	}
	return false;
}

bool RectComplex::Intersects(Rect theRect)
{
	EnumList(Rect,aRect,mRectList)
	{
		if (aRect->Intersects(theRect)) return true;
	}
	return false;
}

void RectComplex::Subtract(RectComplex &theRect)
{
	EnumList(Rect,aRect,theRect.mRectList)
	{
		Subtract(*aRect);
	}
}

void RectComplex::Consolidate()
{
	//
	// Inflate rects first... this means attempting to merge only if we can do so
	// horizontally, and vertically, and then with a diagonal one.
	//
	bool aIsChanged=true;

	aIsChanged=true;
	while (aIsChanged)
	{
		aIsChanged=false;
		bool aBreakAll=false;
		EnumList(Rect,aRect,mRectList)
		{
			EnumList(Rect,aRect2,mRectList)
			{
				if (aIsChanged) break;
				if (aRect2==aRect) continue;
				bool aMerge=false;
				if (aRect->mX==aRect2->mX && aRect->mWidth==aRect2->mWidth)
				{
					if (aRect2->mY>=aRect->mY && aRect2->mY<=(aRect->mY+aRect->mHeight)) aMerge=true;
					if (aRect->mY>=aRect2->mY && aRect->mY<=(aRect2->mY+aRect2->mHeight)) aMerge=true;
				}
				if (aMerge)
				{
					EnumList(Rect,aRect3,mRectList)
					{
						if (aIsChanged) break;
						if (aRect3==aRect || aRect3==aRect2) continue;
						bool aMerge2=false;

						if (aRect->mY==aRect3->mY && aRect->mHeight==aRect3->mHeight)
						{
							if (aRect3->mX>=aRect->mX && aRect3->mX<=(aRect->mX+aRect->mWidth)) aMerge2=true;
							if (aRect->mX>=aRect3->mX && aRect->mX<=(aRect3->mX+aRect3->mWidth)) aMerge2=true;
						}
						if (aMerge2)
						{
							EnumList(Rect,aRect4,mRectList)
							{
								if (aIsChanged) break;
								if (aRect4==aRect || aRect4==aRect2 || aRect4==aRect3) continue;

								//
								// Okay, if this fourth rect is within the boundaries of a big rectangle
								// created by the other three, then we're in like flynn...
								//
								Rect aJoinRect=*aRect;
								aJoinRect=aJoinRect.Union(*aRect2);
								aJoinRect=aJoinRect.Union(*aRect3);

								if (aRect4->mX+aRect4->mWidth==aJoinRect.mX+aJoinRect.mWidth)
								{
									if (aRect4->mY+aRect4->mHeight==aJoinRect.mY+aJoinRect.mHeight)
									{
										*aRect=aJoinRect;
										mRectList-=aRect2;
										mRectList-=aRect3;
										mRectList-=aRect4;
										delete aRect2;
										delete aRect3;
										delete aRect4;
										//aIsChanged=true;
									}
								}
							}
						}
					}
				}


			}
		}
	}

	//
	// Merge adjacent rects
	//
	aIsChanged=true;
	while (aIsChanged)
	{
		aIsChanged=false;
		EnumList(Rect,aRect,mRectList)
		{
			EnumList(Rect,aRect2,mRectList)
			{
				if (aRect2==aRect) continue;
				bool aMerge=false;
				if (aRect->mX==aRect2->mX && aRect->mWidth==aRect2->mWidth)
				{
					if (aRect2->mY>=aRect->mY && aRect2->mY<=(aRect->mY+aRect->mHeight)) aMerge=true;
					if (aRect->mY>=aRect2->mY && aRect->mY<=(aRect2->mY+aRect2->mHeight)) aMerge=true;
				}

				if (aRect->mY==aRect2->mY && aRect->mHeight==aRect2->mHeight)
				{
					if (aRect2->mX>=aRect->mX && aRect2->mX<=(aRect->mX+aRect->mWidth)) aMerge=true;
					if (aRect->mX>=aRect2->mX && aRect->mX<=(aRect2->mX+aRect2->mWidth)) aMerge=true;
				}

				if (aMerge) 
				{
					Rect aHold=aRect->Union(*aRect2);
					*aRect=aHold;

					mRectList-=aRect2;
					delete aRect2;

					aIsChanged=true;
				}
			}
		}
	}

	//
	// Merge embedded rects
	//
	EnumList(Rect,aRect,mRectList)
	{
		EnumList(Rect,aRect2,mRectList)
		{
			if (aRect2==aRect) continue;
			Rect aFix=*aRect2;
			aFix.ClipInto(*aRect);
			if (aFix.mX==aRect2->mX && aFix.mY==aRect2->mY && aFix.mWidth==aRect2->mWidth && aFix.mHeight==aRect2->mHeight)
			{
				mRectList-=aRect2;
				delete aRect2;
				aIsChanged=true;
			}
		}
	}


}

bool RectComplex::IsInnerCorner(int theIndex)
{
	if(mPerimeterData)
		if(theIndex>=0&&theIndex<mPerimeterData->mCorners.mCount)
			return ((Corner*)mPerimeterData->mCornerData[theIndex])->mInner;
	return false;
}

bool RectComplex::IsInnerLine(int theIndex)
{
	if(mPerimeterData)
	{
		Corner *aC1=0;
		Corner *aC2=0;

		Line aLine=GetPerimeterLine(theIndex);

		EnumList(Corner, aCorner, mPerimeterData->mCornerData)
		{
			if(aCorner->mPos==aLine.mPos[0] || aCorner->mPos==aLine.mPos[1])
			{
				if(!aC1)
				{
					aC1=aCorner;
				}
				else if(!aC2)
				{
					aC2=aCorner;
				}
			}
		}

		if(aC1&&aC2)
		{
			return !(aC1->mInner && aC2->mInner);
		}
	}


	return false;
}

Rect Line::GetBound() {return Rect(mPos[0],mPos[0]).Union(Rect(mPos[1],mPos[1]));}
AABBRect Line::GetAABB() {return AABBRect(mPos[0],mPos[1]);}

Line Line::GetPart(float theStartPercent, float theEndPercent)
{
	Point aVec=Vector();
	Line aResult;

	aResult.mPos[0]=mPos[0]+(aVec*theStartPercent);
	aResult.mPos[1]=mPos[0]+(aVec*theEndPercent);

	return aResult;
}

bool Line::IsConnectedTo(Line theLine)
{
	if (gMath.DistanceSquared(theLine.mPos[0],mPos[0])<1) return true;
	if (gMath.DistanceSquared(theLine.mPos[0],mPos[1])<1) return true;
	if (gMath.DistanceSquared(theLine.mPos[1],mPos[0])<1) return true;
	if (gMath.DistanceSquared(theLine.mPos[1],mPos[1])<1) return true;
	return false;
}

bool Line::IsConnectedTo(Point thePoint)
{
	if (gMath.DistanceSquared(thePoint,mPos[0])<1) return true;
	if (gMath.DistanceSquared(thePoint,mPos[1])<1) return true;
	return false;
}

Point Rect::ClosestPointTo(Point theCamera)
{
	if (ContainsPoint(theCamera)) return theCamera;

	if (theCamera.mX<mX) theCamera.mX=mX;else if (theCamera.mX>mX+mWidth) theCamera.mX=mX+mWidth;
	if (theCamera.mY<mY) theCamera.mY=mY;else if (theCamera.mY>mY+mHeight) theCamera.mY=mY+mHeight;

	return theCamera;
    
/*
    //
    // Verbose for Mac compatibility.
    //
    Line aTop=Top();
    Line aBottom=Bottom();
    Line aLeft=Left();
    Line aRight=Right();
    
	Point aP1=gMath.ClosestPointOnLine(theCamera,aTop);
	Point aP2=gMath.ClosestPointOnLine(theCamera,aBottom);
	Point aP3=gMath.ClosestPointOnLine(theCamera,aLeft);
	Point aP4=gMath.ClosestPointOnLine(theCamera,aRight);

	float aDist1=gMath.DistanceSquared(theCamera,aP1);
	float aDist2=gMath.DistanceSquared(theCamera,aP2);
	float aDist3=gMath.DistanceSquared(theCamera,aP3);
	float aDist4=gMath.DistanceSquared(theCamera,aP4);

	Point aResult=aP1;
	float aBestDist=aDist1;

	if (aDist2<aBestDist) {aResult=aP2;aBestDist=aDist2;}
	if (aDist3<aBestDist) {aResult=aP3;aBestDist=aDist3;}
	if (aDist4<aBestDist) {aResult=aP4;aBestDist=aDist4;}

	return aResult;
*/
}

//
// Cube
//



void ViewFrustum::FromMatrix(Matrix& theMatrix, float theMult)
{
	Plane* aP;
	Vector aCenter;

	aP=&mPlane[FRUSTRUM_RIGHT];
	aP->mPos.mX=theMatrix.mData.mm[3]-theMatrix.mData.mm[0];
	aP->mPos.mY=theMatrix.mData.mm[7]-theMatrix.mData.mm[4];
	aP->mPos.mZ=theMatrix.mData.mm[11]-theMatrix.mData.mm[8];
	aP->mD=theMatrix.mData.mm[15]-theMatrix.mData.mm[12];
	aCenter+=aP->mPos;

	aP=&mPlane[FRUSTRUM_LEFT];
	aP->mPos.mX=theMatrix.mData.mm[3]+theMatrix.mData.mm[0];
	aP->mPos.mY=theMatrix.mData.mm[7]+theMatrix.mData.mm[4];
	aP->mPos.mZ=theMatrix.mData.mm[11]+theMatrix.mData.mm[8];
	aP->mD=theMatrix.mData.mm[15]+theMatrix.mData.mm[12];
	aCenter+=aP->mPos;

	aP=&mPlane[FRUSTRUM_BOTTOM];
	aP->mPos.mX=theMatrix.mData.mm[3]+theMatrix.mData.mm[1];
	aP->mPos.mY=theMatrix.mData.mm[7]+theMatrix.mData.mm[5];
	aP->mPos.mZ=theMatrix.mData.mm[11]+theMatrix.mData.mm[9];
	aP->mD=theMatrix.mData.mm[15]+theMatrix.mData.mm[13];
	aCenter+=aP->mPos;

	aP=&mPlane[FRUSTRUM_TOP];
	aP->mPos.mX=theMatrix.mData.mm[3]-theMatrix.mData.mm[1];
	aP->mPos.mY=theMatrix.mData.mm[7]-theMatrix.mData.mm[5];
	aP->mPos.mZ=theMatrix.mData.mm[11]-theMatrix.mData.mm[9];
	aP->mD=theMatrix.mData.mm[15]-theMatrix.mData.mm[13];
	aCenter+=aP->mPos;

	aP=&mPlane[FRUSTRUM_FAR];
	aP->mPos.mX=theMatrix.mData.mm[3]-theMatrix.mData.mm[2];
	aP->mPos.mY=theMatrix.mData.mm[7]-theMatrix.mData.mm[6];
	aP->mPos.mZ=theMatrix.mData.mm[11]-theMatrix.mData.mm[10];
	aP->mD=theMatrix.mData.mm[15]-theMatrix.mData.mm[14];
	aCenter+=aP->mPos;

	aP=&mPlane[FRUSTRUM_NEAR];
	aP->mPos.mX=theMatrix.mData.mm[3]+theMatrix.mData.mm[2];
	aP->mPos.mY=theMatrix.mData.mm[7]+theMatrix.mData.mm[6];
	aP->mPos.mZ=theMatrix.mData.mm[11]+theMatrix.mData.mm[10];
	aP->mD=theMatrix.mData.mm[15]+theMatrix.mData.mm[14];
	aCenter+=aP->mPos;
	aCenter/=6;

	for (int aCount=0;aCount<6;aCount++) {mPlane[aCount].mNormal=Normalize(aCenter-mPlane[aCount].mPos);}
	for (int aCount=0;aCount<6;aCount++) mPlane[aCount].NormalizePlus(theMult);
}

/*
Vector ViewFrustum::CastOut(Vector theStart, Vector theRay)
{
	float aBestDist=FLT_MAX;
	Vector aResult;aResult.SetNAN();
	for (int aCount=0;aCount<6;aCount++)
	{
		Vector aHit;
		if (gMath.DoesRayIntersectPlane(theStart,theRay,mPlane[aCount],&aHit))
		{
			float aDist=gMath.DistanceSquared(theStart,aHit);
			if (aDist<aBestDist)
			{
				aBestDist=aDist;
				aResult=aHit;
			}
		}
	}
	return aResult;
}
*/


bool ViewFrustum::ContainsPoint(Vector thePos)
{
	if (mPlane[FRUSTRUM_RIGHT].Dot(thePos)<0.0f) return false;
	if (mPlane[FRUSTRUM_LEFT].Dot(thePos)<0.0f) return false;
	if (mPlane[FRUSTRUM_TOP].Dot(thePos)<0.0f) return false;
	if (mPlane[FRUSTRUM_BOTTOM].Dot(thePos)<0.0f) return false;
	if (mPlane[FRUSTRUM_NEAR].Dot(thePos)<0.0f) return false;
	if (mPlane[FRUSTRUM_FAR].Dot(thePos)<0.0f) return false;
	return true;
}

bool ViewFrustum::ContainsCube(Cube theCube)
{
	for(int aCount=0;aCount<6;aCount++) 
	{
		bool aOK=false;
		for (int aCorner=0;aCorner<8;aCorner++) if (mPlane[aCount].Dot(theCube.Corner(aCorner))>=0.0f) {aOK=true;break;}
		if (!aOK) return false;
	}
	return true;
}

bool ViewFrustum::ContainsCube(Cube theCube, Matrix theMatrix)
{
	for(int aCount=0;aCount<6;aCount++) 
	{
		bool aOK=false;
		for (int aCorner=0;aCorner<8;aCorner++) if (mPlane[aCount].Dot(theMatrix.ProcessPoint3D(theCube.Corner(aCorner)))>=0.0f) {aOK=true;break;}
		if (!aOK) return false;
	}
	return true;
}


bool ViewFrustum::ContainsSphere(Vector thePos, float theRadius)
{
	if (mPlane[FRUSTRUM_RIGHT].Dot(thePos)<-theRadius) return false;
	if (mPlane[FRUSTRUM_LEFT].Dot(thePos)<-theRadius) return false;
	if (mPlane[FRUSTRUM_TOP].Dot(thePos)<-theRadius) return false;
	if (mPlane[FRUSTRUM_BOTTOM].Dot(thePos)<-theRadius) return false;
	if (mPlane[FRUSTRUM_NEAR].Dot(thePos)<-theRadius) return false;
	if (mPlane[FRUSTRUM_FAR].Dot(thePos)<-theRadius) return false;
	return true;
}

bool ViewFrustum::IsSphereBehind(char thePlane, Vector thePos, float theRadius)
{
	return (mPlane[thePlane].Dot(thePos)<-theRadius);
}

bool ViewFrustum::ContainsPointISO(Vector thePos)
{
	if (mPlane[FRUSTRUM_RIGHT].Dot(thePos)<0.0f) return false;
	if (mPlane[FRUSTRUM_LEFT].Dot(thePos)<0.0f) return false;
	if (mPlane[FRUSTRUM_NEAR].Dot(thePos)<0.0f) return false;
	if (mPlane[FRUSTRUM_FAR].Dot(thePos)<0.0f) return false;
	return true;
}

bool ViewFrustum::ContainsCubeISO(Cube theCube)
{
	for(int aCount=0;aCount<4;aCount++) 
	{
		bool aOK=false;
		for (int aCorner=0;aCorner<8;aCorner++) if (mPlane[aCount].Dot(theCube.Corner(aCorner))>=0.0f) {aOK=true;break;}
		if (!aOK) return false;
	}
	return true;
}

bool ViewFrustum::ContainsSphereISO(Vector thePos, float theRadius)
{
	if (mPlane[FRUSTRUM_RIGHT].Dot(thePos)<-theRadius) return false;
	if (mPlane[FRUSTRUM_LEFT].Dot(thePos)<-theRadius) return false;
	if (mPlane[FRUSTRUM_NEAR].Dot(thePos)<-theRadius) return false;
	if (mPlane[FRUSTRUM_FAR].Dot(thePos)<-theRadius) return false;
	return true;
}




Vector Volume::ClampInsideToEdge(Vector thePos)
{
	float aBestLen=FLT_MAX;
	Vector aBest=thePos;
	foreach(aP,mPlane)
	{
		float aLen=aP.DistanceTo(thePos);
		if (aLen<aBestLen)
		{
			aBest=aP.ClosestPoint(thePos);
			aBestLen=aLen;
		}
	}
	return aBest;
}

Vector Volume::ClampInsideToEdge(Vector thePos, Array<char>& theEdges)
{
	//
	// Created this to handle square collision "holes" in a Hamsterball world.  If the collision point
	// is inside the volume, it gets moved to the edge.
	//
	float aBestLen=FLT_MAX;
	Vector aBest=thePos;
	foreach(aC,theEdges)	
	{
		Plane& aP=mPlane[aC];
		float aLen=aP.DistanceTo(thePos);
		if (aLen<aBestLen)
		{
			aBest=aP.ClosestPoint(thePos);
			aBestLen=aLen;
		}
	}
	return aBest;

}


void Volume::FromCube(Cube theCube, Matrix theMatrix)
{
	Vector aV[12];

	//
	// Cube points
	//
	aV[FRUSTRUM_TOP]=theCube.GetMid(0,1,2,3); // Top
	aV[FRUSTRUM_BOTTOM]=theCube.GetMid(4,5,6,7); // Bottom
	aV[FRUSTRUM_LEFT]=theCube.GetMid(0,2,4,6); // Left
	aV[FRUSTRUM_RIGHT]=theCube.GetMid(1,3,5,7); // Right
	aV[FRUSTRUM_FAR]=theCube.GetMid(0,1,4,5); // Back
	aV[FRUSTRUM_NEAR]=theCube.GetMid(2,3,6,7); // Front
	aV[FRUSTRUM_TOP+6]=Vector(0,0,1); // Top of cube
	aV[FRUSTRUM_BOTTOM+6]=Vector(0,0,-1); // Bottom of cube
	aV[FRUSTRUM_LEFT+6]=Vector(1,0,0); // Left side
	aV[FRUSTRUM_RIGHT+6]=Vector(-1,0,0); // Right side
	aV[FRUSTRUM_FAR+6]=Vector(0,1,0); // Back side
	aV[FRUSTRUM_NEAR+6]=Vector(0,-1,0); // Front side

	for (int aCount=0;aCount<6;aCount++) theMatrix.Process(aV[aCount]);
	theMatrix.Normalize();
	for (int aCount=0;aCount<6;aCount++) theMatrix.Process(aV[aCount+6]);

	mPlane.GuaranteeSize(6);
	for (int aCount=0;aCount<6;aCount++) 
	{
		mPlane[aCount].Create(aV[aCount],aV[aCount+6].Normal());
		//mPlane[aCount].Normalize();
	}
}

void Volume::FromInfiniteCylinder(Vector theOrigin, Vector theAxis, float theRadius, int theSides)
{
	float aStep=360.0f/(float)theSides;

	Array<Vector> aPNorm;
	aPNorm.GuaranteeSize(theSides);

	int aFill=0;
	for (float aAngle=0;aAngle<360;aAngle+=aStep)
	{
		aPNorm[aFill]=gMath.AngleToVector(aAngle);
		aFill++;
	}
	aPNorm.Clip(aFill);

	Matrix aMat;
	aMat.VectorToVector(Vector(0,0,1),theAxis);
	aMat.ProcessPoints(aPNorm);

	mPlane.GuaranteeSize(aFill);
	for (int aCount=0;aCount<aFill;aCount++) mPlane[aCount].Create(theOrigin+aPNorm[aCount]*theRadius,-aPNorm[aCount]);
}


void Volume::FromTriangle(Triangle3D aT)
{
	Vector aOrthoCenter=aT.GetCentroid();

	Vector aMid01=gMath.GetClosestPointOnInfiniteLine(aOrthoCenter,aT.mPos[0],aT.mPos[1]);
	Vector aMid12=gMath.GetClosestPointOnInfiniteLine(aOrthoCenter,aT.mPos[1],aT.mPos[2]);
	Vector aMid20=gMath.GetClosestPointOnInfiniteLine(aOrthoCenter,aT.mPos[2],aT.mPos[0]);
	Vector aNorm01=aOrthoCenter-aMid01;
	Vector aNorm12=aOrthoCenter-aMid12;
	Vector aNorm20=aOrthoCenter-aMid20;

	/*
	if (aOrthoCenter.PrettyClose(aMid01)) aNorm01=aT.mPos[2]-aT.mPos[0];
	if (aOrthoCenter.PrettyClose(aMid12)) aNorm12=aT.mPos[0]-aT.mPos[1];
	if (aOrthoCenter.PrettyClose(aMid20)) aNorm20=aT.mPos[1]-aT.mPos[0];
	aNorm01*=0;
	aNorm12*=0;
	*/


	aNorm01.Normalize();
	aNorm12.Normalize();
	aNorm20.Normalize();

	mPlane.GuaranteeSize(3);
	mPlane[0].Create(aMid01,aNorm01);
	mPlane[1].Create(aMid12,aNorm12);
	mPlane[2].Create(aMid20,aNorm20);
}

void Volume::FromTriangle(Triangle3D aT, Triangle3D aTN)
{
	mPlane.GuaranteeSize(3);

	mPlane[0].Create(aT.mPos[1]+aTN.mPos[1],aT.mPos[0],aT.mPos[0]+aTN.mPos[0]);
	mPlane[1].Create(aT.mPos[2]+aTN.mPos[2],aT.mPos[1],aT.mPos[1]+aTN.mPos[1]);
	mPlane[2].Create(aT.mPos[0]+aTN.mPos[0],aT.mPos[2],aT.mPos[2]+aTN.mPos[2]);

	/*
	mPlane[0].Create(aT.mPos[0],aT.mPos[0]+aTN.mPos[0],aT.mPos[1]);
	mPlane[1].Create(aT.mPos[1],aT.mPos[1]+aTN.mPos[1],aT.mPos[2]);
	mPlane[2].Create(aT.mPos[2],aT.mPos[2]+aTN.mPos[2],aT.mPos[0]);
	*/


	/*
	Vector aOrthoCenter=aT.GetCentroid();

	Vector aMid01=gMath.GetClosestPointOnInfiniteLine(aOrthoCenter,aT.mPos[0],aT.mPos[1]);
	Vector aMid12=gMath.GetClosestPointOnInfiniteLine(aOrthoCenter,aT.mPos[1],aT.mPos[2]);
	Vector aMid20=gMath.GetClosestPointOnInfiniteLine(aOrthoCenter,aT.mPos[2],aT.mPos[0]);
	Vector aNorm01=aOrthoCenter-aMid01;
	Vector aNorm12=aOrthoCenter-aMid12;
	Vector aNorm20=aOrthoCenter-aMid20;

	aNorm01.Normalize();
	aNorm12.Normalize();
	aNorm20.Normalize();

	mPlane.GuaranteeSize(3);
	mPlane[0].Create(aMid01,aNorm01);
	mPlane[1].Create(aMid12,aNorm12);
	mPlane[2].Create(aMid20,aNorm20);
	*/
}


Vector Triangle3D::GetOrthoCenter()
{
	Vector aPerp1=gMath.GetClosestPointOnInfiniteLine(mPos[0],mPos[1],mPos[2]);
	Vector aPerp2=gMath.GetClosestPointOnInfiniteLine(mPos[1],mPos[2],mPos[0]);

	Vector aResult;
	gMath.GetLineIntersection(mPos[0],aPerp1,mPos[1],aPerp2,&aResult);
	return aResult;
	//Vector aPerp3=gMath.GetClosestPointOnInfiniteLine(mPos[2],mPos[0],mPos[1]);
}

Vector Triangle3DPtr::GetOrthoCenter()
{
	Vector aPerp1=gMath.GetClosestPointOnInfiniteLine(*mPos[0],*mPos[1],*mPos[2]);
	Vector aPerp2=gMath.GetClosestPointOnInfiniteLine(*mPos[1],*mPos[2],*mPos[0]);

	Vector aResult;
	gMath.GetLineIntersection(*mPos[0],aPerp1,*mPos[1],aPerp2,&aResult);
	return aResult;
	//Vector aPerp3=gMath.GetClosestPointOnInfiniteLine(mPos[2],mPos[0],mPos[1]);
}


Line3D Line3D::Translate(float theX, float theY, float theZ) {return Translate(Vector(theX,theY,theZ));}
float Line3D::LengthSquared() {return Vector(mPos[0]-mPos[1]).LengthSquared();}
float Line3D::LenSquared() {return LengthSquared();}
Vector Line3D::Direction() {return Vector(mPos[1]-mPos[0]).Normal();}
Vector Line3D::Dir() {return Direction();}
Vector Line3D::GetVectorAt(float thePercent) {return mPos[0]+(Span()*thePercent);}
Vector Line3D::Span() {return mPos[1]-mPos[0];}
void Line3D::Expand(float theAmount) {Vector aMove=Direction()*theAmount;mPos[0]-=aMove;mPos[1]+=aMove;}
void Line3D::Reverse() {Vector aHold=mPos[0];mPos[0]=mPos[1];mPos[1]=aHold;}
float Line3D::GetPercentAt(Vector thePos)
{
	Vector aSpan=Span();
	Vector aD1=thePos-mPos[0];

	/*
	gGX.AddPoint3D(aD1,Color(1,1,0));
	gGX.AddPoint3D(mPos[0],Color(1,0,0),10);
	gGX.AddPoint3D(mPos[1],Color(0,1,0),10);
	*/

	if (aD1.Dot(aSpan)<0) return 0;
	Vector aD2=thePos-mPos[1];
	if (aD2.Dot(aSpan)>0) return 1.0f;

	return aD1.Length()/Length();
}
float Line::GetPercentAt(Point thePos)
{
	Point aSpan=Span();
	Point aD1=thePos-mPos[0];
	if (aD1.Dot(aSpan)<0) return 0;
	Point aD2=thePos-mPos[1];
	if (aD2.Dot(aSpan)>0) return 1.0f;
	return aD1.Length()/Length();
}
void Line::Clip(Plane thePlane, char frontOrBack)
{
	Line3D aL;
	aL.mPos[0]=Vector(mPos[0],thePlane.mPos.mZ);
	aL.mPos[1]=Vector(mPos[1],thePlane.mPos.mZ);
	Vector aHit;
	if (gMath.DoesInfiniteLineIntersectPlane(aL,thePlane,&aHit)) 
	{
		if (gMath.Sign(thePlane.ClassifyPoint(mPos[0]))==frontOrBack) mPos[0]=aHit;
		if (gMath.Sign(thePlane.ClassifyPoint(mPos[1]))==frontOrBack) mPos[1]=aHit;
	}
}


Line3D Line3D::GetPart(float theStartPercent, float theEndPercent)
{
	Vector aVec=Span();
	Line3D aResult;

	aResult.mPos[0]=mPos[0]+(aVec*theStartPercent);
	aResult.mPos[1]=mPos[0]+(aVec*theEndPercent);

	return aResult;
}

void Line::SetLength(float theLength)
{
	Point aMid=Mid();
	Point aDir=Dir();
	mPos[0]=aMid-aDir*(theLength/2);
	mPos[1]=aMid+aDir*(theLength/2);
}

void Line3D::SetLength(float theLength)
{
	Vector aMid=Mid();
	Vector aDir=Dir();
	mPos[0]=aMid-aDir*(theLength/2);
	mPos[1]=aMid+aDir*(theLength/2);
}

void Line3D::SetLength(float theLength, char fromEnd)
{
	if (fromEnd<=0) fromEnd=0;
	else if (fromEnd>0) fromEnd=1;

	int aToEnd=1-fromEnd;
	Vector aDir=mPos[aToEnd]-mPos[fromEnd];
	aDir.SetLength(theLength);
	mPos[aToEnd]=mPos[fromEnd]+aDir;
}

Rect Rect::Interpolate(float theX, float theY, float theWidth, float theHeight, float thePercent)
{
	Point aWantUL=Point(theX,theY);
	Point aWantLR=aWantUL+Point(theWidth,theHeight);

	Point aVec1=aWantUL-UpperLeft();
	Point aVec2=aWantLR-LowerRight();

	aVec1*=thePercent;
	aVec2*=thePercent;

	return Rect(UpperLeft()+aVec1,LowerRight()+aVec2);
}


//
// XYRect...
//
XYRect::XYRect(void)
{
	mX1=0;
	mY1=0;
	mX2=0;
	mY2=0;
}

XYRect::XYRect(int theX1, int theY1, int theX2,int theY2)
{
	Resize(theX1,theY1,theX2,theY2);
}

void XYRect::Resize(int theX1, int theY1, int theX2, int theY2)
{
	mX1=theX1;
	mY1=theY1;
	mX2=theX2;
	mY2=theY2;
}


bool XYRect::Intersects(int theX1, int theY1, int theX2, int theY2)
{
	return !((theX2<mX1) || (theY2<mY1) || (theX1>mX2) || (theY1>mY2));
}

XYRect XYRect::GetIntersection(XYRect &theRect)
{
	int x1=_max(mX1,theRect.mX1);
	int x2=_min(mX2,theRect.mX2);
	int y1=_max(mY1,theRect.mY1);
	int y2=_min(mY2,theRect.mY2);

	if (((x2-x1)<0) || ((y2-y1)<0)) return XYRect(0,0,-1,-1);
	else return XYRect(x1,y1,x2,y2);

/*
	float x1=_max(mX,theRect.mX);
	float x2=_min(mX+mWidth,theRect.mX+theRect.mWidth);
	float y1=_max(mY,theRect.mY);
	float y2=_min(mY+mHeight,theRect.mY+theRect.mHeight);

	if (((x2-x1)<0) || ((y2-y1)<0)) return Rect(0,0,0,0);
	else return Rect(x1,y1,x2-x1,y2-y1);
*/
}

XYRect XYRect::Union(XYRect theRect)	
{
	if (mX1==0 && mY1==0 && mX2==0 && mY2==0)
	{
		//
		// Our rect is null right now, so just return the union'd rect.
		//
		return theRect;
	}
	else
	{
		int x1 = _min(mX1,theRect.mX1);
		int x2 = _max(mX2,theRect.mX2);
		int y1 = _min(mY1,theRect.mY1);
		int y2 = _max(mY2,theRect.mY2);
		return XYRect(x1,y1,x2,y2);
	}
}

bool XYRect::IsTouching(int theX1, int theY1, int theX2, int theY2)
{
	return Expand(1).Intersects(theX1,theY1,theX2,theY2);
}


//
// AABBRect...
//
AABBRect::AABBRect(void)
{
	mX1=0;
	mY1=0;
	mX2=0;
	mY2=0;
}

AABBRect::AABBRect(float theX1, float theY1, float theX2,float theY2)
{
	Resize(theX1,theY1,theX2,theY2);
}

void AABBRect::Resize(float theX1, float theY1, float theX2, float theY2)
{
	mX1=theX1;
	mY1=theY1;
	mX2=theX2;
	mY2=theY2;
}

void AABBCube::Resize(float theX1, float theY1, float theZ1, float theX2, float theY2, float theZ2)
{
	mX1=theX1;
	mY1=theY1;
	mZ1=theZ1;
	mX2=theX2;
	mY2=theY2;
	mZ2=theZ2;
}

AABBRect AABBRect::GetIntersection(AABBRect &theRect)
{
	float x1=_max(mX1,theRect.mX1);
	float x2=_min(mX2,theRect.mX2);
	float y1=_max(mY1,theRect.mY1);
	float y2=_min(mY2,theRect.mY2);

	if (((x2-x1)<0) || ((y2-y1)<0)) return AABBRect(0,0,-1,-1);
	else return AABBRect(x1,y1,x2,y2);
}

AABBCube AABBCube::GetIntersection(AABBCube &theCube)
{
	float x1=_max(mX1,theCube.mX1);
	float x2=_min(mX2,theCube.mX2);
	float y1=_max(mY1,theCube.mY1);
	float y2=_min(mY2,theCube.mY2);
	float z1=_max(mZ1,theCube.mZ1);
	float z2=_min(mZ2,theCube.mZ2);

	if (((x2-x1)<0) || ((y2-y1)<0)) return AABBCube(0,0,0,-1,-1,-1);
	else return AABBCube(x1,y1,z1,x2,y2,z2);
}

AABBRect AABBRect::Union(AABBRect theRect)	
{
	if (mX1==0 && mY1==0 && mX2==0 && mY2==0)
	{
		//
		// Our rect is null right now, so just return the union'd rect.
		//
		return theRect;
	}
	else
	{
		float x1 = _min(mX1,theRect.mX1);
		float x2 = _max(mX2,theRect.mX2);
		float y1 = _min(mY1,theRect.mY1);
		float y2 = _max(mY2,theRect.mY2);
		return AABBRect(x1,y1,x2,y2);
	}
}

AABBCube AABBCube::Union(AABBCube theCube)	
{
	if (mX1==0 && mY1==0 && mZ1==0 && mX2==0 && mY2==0 && mZ2==0) 
	{
		if (theCube.Width()==0 && theCube.Height()==0 && theCube.Depth()==0) theCube.mX2+=.0000001f; // To ensure we're not null...
		return theCube;
	}
	else
	{
		float x1 = _min(mX1,theCube.mX1);
		float x2 = _max(mX2,theCube.mX2);
		float y1 = _min(mY1,theCube.mY1);
		float y2 = _max(mY2,theCube.mY2);
		float z1 = _min(mZ1,theCube.mZ1);
		float z2 = _max(mZ2,theCube.mZ2);
		return AABBCube(x1,y1,z1,x2,y2,z2);
	}
}

bool AABBRect::IsTouching(float theX1, float theY1, float theX2, float theY2)
{
	return Expand(.001f).Intersects(theX1,theY1,theX2,theY2);
}

bool AABBCube::IsTouching(float theX1, float theY1, float theZ1, float theX2, float theY2, float theZ2)
{
	return Expand(.001f).Intersects(theX1,theY1,theZ1,theX2,theY2,theZ2);
}

Rect Rect::GetScaled(float theScaleX, float theScaleY)
{
	Rect aResult=Rect(mWidth*theScaleX,mHeight*theScaleY);
	aResult.CenterAt(Center());
	return aResult;
}

template <typename var_type>
char* CubeT<var_type>::ToString() {String& aResult=GetDiscardableString();aResult=Sprintf("%.2f,%.2f,%.2f - %.2f,%.2f,%.2f",(float)mX,(float)mY,(float)mZ,(float)mXSize,(float)mYSize,(float)mZSize);return aResult.c();}

template <typename var_type>
char* CubeT<var_type>::ToStringI() {String& aResult=GetDiscardableString();aResult=Sprintf("%d,%d,%d - %d,%d,%d",(int)mX,(int)mY,(int)mZ,(int)mXSize,(int)mYSize,(int)mZSize);return aResult.c();}

template class CubeT<int>;
template class CubeT<float>;

/*
bool Triangle::IsDegenerate() {return (gMath.DistanceSquared(mPos[0],mPos[1])<gMath.mMachineEpsilon)|(gMath.DistanceSquared(mPos[1],mPos[2])<gMath.mMachineEpsilon)|(gMath.DistanceSquared(mPos[0],mPos[2])<gMath.mMachineEpsilon);}
bool Triangle3D::IsDegenerate() {return (gMath.DistanceSquared(mPos[0],mPos[1])<gMath.mMachineEpsilon)|(gMath.DistanceSquared(mPos[1],mPos[2])<gMath.mMachineEpsilon)|(gMath.DistanceSquared(mPos[0],mPos[2])<gMath.mMachineEpsilon);}
bool Triangle3DPtr::IsDegenerate() {return (gMath.DistanceSquared(*mPos[0],*mPos[1])<gMath.mMachineEpsilon)|(gMath.DistanceSquared(*mPos[1],*mPos[2])<gMath.mMachineEpsilon)|(gMath.DistanceSquared(*mPos[0],*mPos[2])<gMath.mMachineEpsilon);}
*/

bool Triangle::IsDegenerate() {return gMath.IsTriangleDegenerate(mPos[0],mPos[1],mPos[2]);}
bool Triangle3D::IsDegenerate() {return gMath.IsTriangleDegenerate(mPos[0],mPos[1],mPos[2]);}
bool Triangle3DPtr::IsDegenerate() {return gMath.IsTriangleDegenerate(*mPos[0],*mPos[1],*mPos[2]);}

Vector Line3D::GetSize()
{
	Cube aCube;
	aCube=aCube.Union(mPos[0]);
	aCube=aCube.Union(mPos[1]);
	return Vector(aCube.mXSize,aCube.mYSize,aCube.mZSize);
}

/*
bool Volume::ContainsLine(Vector theLine1, Vector theLine2)
{
	if (ContainsPoint(theLine1) || ContainsPoint(theLine2)) return true;
	Vector aIPos;
	for (int aPlane=0;aPlane<6;aPlane++) if (gMath.DoesLineIntersectPlane(theLine1,theLine2,mPlane[aPlane],&aIPos)) if (ContainsPoint(aIPos)) return true;
	return false;
}
*/


Line Triangle::GetHypotenuse()
{
	float aD1=gMath.DistanceSquared(mPos[0],mPos[1]);
	float aD2=gMath.DistanceSquared(mPos[1],mPos[2]);
	float aD3=gMath.DistanceSquared(mPos[2],mPos[0]);
	if (aD1>aD2 && aD1>aD3) return Line(mPos[0],mPos[1]);
	if (aD2>aD1 && aD2>aD3) return Line(mPos[1],mPos[2]);
	return Line(mPos[2],mPos[0]);
};

Line3D Triangle3D::GetHypotenuse()
{
	float aD1=gMath.DistanceSquared(mPos[0],mPos[1]);
	float aD2=gMath.DistanceSquared(mPos[1],mPos[2]);
	float aD3=gMath.DistanceSquared(mPos[2],mPos[0]);
	if (aD1>aD2 && aD1>aD3) return Line3D(mPos[0],mPos[1]);
	if (aD2>aD1 && aD2>aD3) return Line3D(mPos[1],mPos[2]);
	return Line3D(mPos[2],mPos[0]);
};

void Triangle3D::GetLegs(Array<Line3D>& theLegArray, bool isCW)
{
	Vector aD1=mPos[1]-mPos[0];
	Vector aD2=mPos[2]-mPos[1];
	Vector aD3=mPos[2]-mPos[0];
	float aD1Len=aD1.LengthSquared();
	float aD2Len=aD2.LengthSquared();
	float aD3Len=aD3.LengthSquared();
	if (aD1Len>aD2Len && aD1Len>aD3Len) {theLegArray[1]=Line3D(mPos[1],mPos[2]);theLegArray[0]=Line3D(mPos[2],mPos[0]);}
	if (aD2Len>aD1Len && aD2Len>aD3Len) {theLegArray[1]=Line3D(mPos[0],mPos[1]);theLegArray[0]=Line3D(mPos[2],mPos[0]);}
	if (aD3Len>aD1Len && aD3Len>aD2Len) {theLegArray[1]=Line3D(mPos[0],mPos[1]);theLegArray[0]=Line3D(mPos[1],mPos[2]);}

	if (isCW) {if (theLegArray[0].mPos[1].PrettyClose(theLegArray[1].mPos[0])) Swap<Line3D>(theLegArray[0],theLegArray[1]);}
	else {if (theLegArray[1].mPos[1].PrettyClose(theLegArray[0].mPos[0])) Swap<Line3D>(theLegArray[0],theLegArray[1]);}
	// For CW, whichever leg ENDS on the other leg should be reported last.

}


Rect Triangle3DPtr::GetBoundingXYRect()
{
	float aLeft,aRight,aTop,aBottom;
	aLeft=aRight=mPos[0]->mX;
	aTop=aBottom=mPos[0]->mY;
	for(int i=1;i<3;i++)
	{
		if(mPos[i]->mX<aLeft)aLeft=mPos[i]->mX;
		else if(mPos[i]->mX>aRight)aRight=mPos[i]->mX;
		if(mPos[i]->mY<aTop)aTop=mPos[i]->mY;
		else if(mPos[i]->mY>aBottom)aBottom=mPos[i]->mY;
	}
	return Rect(aLeft,aTop,_max(0,aRight-aLeft),_max(0,aBottom-aTop));
}

AABBRect Triangle3DPtr::GetBoundingXYRectAABB()
{
	float aLeft,aRight,aTop,aBottom;
	aLeft=aRight=mPos[0]->mX;
	aTop=aBottom=mPos[0]->mY;
	for(int i=1;i<3;i++)
	{
		if(mPos[i]->mX<aLeft)aLeft=mPos[i]->mX;
		else if(mPos[i]->mX>aRight)aRight=mPos[i]->mX;
		if(mPos[i]->mY<aTop)aTop=mPos[i]->mY;
		else if(mPos[i]->mY>aBottom)aBottom=mPos[i]->mY;
	}
	return AABBRect(aLeft,aTop,aRight,aBottom);
}

Cube Triangle3DPtr::GetBoundingCube()
{
	Vector aP1=*mPos[0];
	Vector aP2=*mPos[0];
	for (int aCount=1;aCount<3;aCount++)
	{
		aP1.mX=_min(aP1.mX,mPos[aCount]->mX);
		aP1.mY=_min(aP1.mY,mPos[aCount]->mY);
		aP1.mZ=_min(aP1.mZ,mPos[aCount]->mZ);
		aP2.mX=_max(aP2.mX,mPos[aCount]->mX);
		aP2.mY=_max(aP2.mY,mPos[aCount]->mY);
		aP2.mZ=_max(aP2.mZ,mPos[aCount]->mZ);
	}
	return Cube(aP1,aP2);
}

Line3D Triangle3DPtr::GetHypotenuse()
{
	float aD1=gMath.DistanceSquared(*mPos[0],*mPos[1]);
	float aD2=gMath.DistanceSquared(*mPos[1],*mPos[2]);
	float aD3=gMath.DistanceSquared(*mPos[2],*mPos[0]);
	if (aD1>aD2 && aD1>aD3) return Line3D(*mPos[0],*mPos[1]);
	if (aD2>aD1 && aD2>aD3) return Line3D(*mPos[1],*mPos[2]);
	return Line3D(*mPos[2],*mPos[0]);
};

void Triangle3DPtr::GetLegs(Array<Line3D>& theLegArray, bool isCW)
{
	Vector aD1=*mPos[1]-*mPos[0];
	Vector aD2=*mPos[2]-*mPos[1];
	Vector aD3=*mPos[2]-*mPos[0];
	float aD1Len=aD1.LengthSquared();
	float aD2Len=aD2.LengthSquared();
	float aD3Len=aD3.LengthSquared();
	if (aD1Len>aD2Len && aD1Len>aD3Len) {theLegArray[1]=Line3D(*mPos[1],*mPos[2]);theLegArray[0]=Line3D(*mPos[2],*mPos[0]);}
	if (aD2Len>aD1Len && aD2Len>aD3Len) {theLegArray[1]=Line3D(*mPos[0],*mPos[1]);theLegArray[0]=Line3D(*mPos[2],*mPos[0]);}
	if (aD3Len>aD1Len && aD3Len>aD2Len) {theLegArray[1]=Line3D(*mPos[0],*mPos[1]);theLegArray[0]=Line3D(*mPos[1],*mPos[2]);}

	if (isCW) {if (theLegArray[0].mPos[1].PrettyClose(theLegArray[1].mPos[0])) Swap<Line3D>(theLegArray[0],theLegArray[1]);}
	else {if (theLegArray[1].mPos[1].PrettyClose(theLegArray[0].mPos[0])) Swap<Line3D>(theLegArray[0],theLegArray[1]);}
	// For CW, whichever leg ENDS on the other leg should be reported last.

}

void Triangle3D::Expand(float theAmount)
{
	/*
	//Vector aCenter=GetCentroid();
	Vector aCenter=GetIncenter();
	Vector a0toCenter=mPos[0]-aCenter;
	Vector a1toCenter=mPos[1]-aCenter;
	Vector a2toCenter=mPos[2]-aCenter;
	Vector a01toCenter=(gMath.GetClosestPointOnInfiniteLine(aCenter,mPos[0],mPos[1]))-aCenter;
	Vector a12toCenter=(gMath.GetClosestPointOnInfiniteLine(aCenter,mPos[1],mPos[2]))-aCenter;
	Vector a20toCenter=(gMath.GetClosestPointOnInfiniteLine(aCenter,mPos[2],mPos[0]))-aCenter;

	a0toCenter.SetLength(theAmount);
	a1toCenter.SetLength(theAmount);
	a2toCenter.SetLength(theAmount);
	a01toCenter.SetLength(theAmount);
	a12toCenter.SetLength(theAmount);
	a20toCenter.SetLength(theAmount);

	mPos[0]+=a0toCenter+a01toCenter+a20toCenter;
	mPos[1]+=a1toCenter+a01toCenter+a12toCenter;
	mPos[2]+=a2toCenter+a20toCenter+a12toCenter;
	/**/


	/*
	Vector aMid01=(mPos[0]+mPos[1])/2;
	Vector aMid12=(mPos[1]+mPos[2])/2;
	Vector aMid20=(mPos[2]+mPos[0])/2;

	Vector aDir0=mPos[0]-aMid12;
	Vector aDir1=mPos[1]-aMid20;
	Vector aDir2=mPos[2]-aMid01;
	aDir0.SetLength(theAmount);
	aDir1.SetLength(theAmount);
	aDir2.SetLength(theAmount);

	mPos[0]+=aDir0;
	mPos[1]+=aDir1;
	mPos[2]+=aDir2;
	/**/

	/*
	Vector aPos[3];
	aPos[0]=mPos[0];aPos[1]=mPos[1];aPos[2]=mPos[2];

	Vector a01=aPos[0]-aPos[1];a01.SetLength(theAmount);
	Vector a12=aPos[1]-aPos[2];a12.SetLength(theAmount);
	Vector a02=aPos[0]-aPos[2];a02.SetLength(theAmount);
	Vector a10=-a01;
	Vector a21=-a12;
	Vector a20=-a02;

	mPos[0]+=a01;mPos[0]+=a02;
	mPos[1]+=a10;mPos[1]+=a12;
	mPos[2]+=a20;mPos[2]+=a21;
	/**/

	//*
	Vector aCenter=GetCentroid();
//	Vector aCenter=GetIncenter();
	Vector aL1=mPos[0]-aCenter;aL1.SetLength(theAmount);
	Vector aL2=mPos[1]-aCenter;aL2.SetLength(theAmount);
	Vector aL3=mPos[2]-aCenter;aL3.SetLength(theAmount);
	mPos[0]+=aL1;
	mPos[1]+=aL2;
	mPos[2]+=aL3;

	/**/
}

bool Triangle3D::SamePoints(Triangle3D& theOther)
{
	char aSameCount=0;
	if (mPos[0].PrettyClose(theOther.mPos[0]) || mPos[0].PrettyClose(theOther.mPos[1]) || mPos[0].PrettyClose(theOther.mPos[2])) aSameCount++;
	if (mPos[1].PrettyClose(theOther.mPos[0]) || mPos[1].PrettyClose(theOther.mPos[1]) || mPos[1].PrettyClose(theOther.mPos[2])) aSameCount++;
	if (mPos[2].PrettyClose(theOther.mPos[0]) || mPos[2].PrettyClose(theOther.mPos[1]) || mPos[2].PrettyClose(theOther.mPos[2])) aSameCount++;
	return (aSameCount==3);
}

bool Triangle3DPtr::SamePoints(Triangle3DPtr& theOther)
{
	char aSameCount=0;
	if (mPos[0]->PrettyClose(*theOther.mPos[0]) || mPos[0]->PrettyClose(*theOther.mPos[1]) || mPos[0]->PrettyClose(*theOther.mPos[2])) aSameCount++;
	if (mPos[1]->PrettyClose(*theOther.mPos[0]) || mPos[1]->PrettyClose(*theOther.mPos[1]) || mPos[1]->PrettyClose(*theOther.mPos[2])) aSameCount++;
	if (mPos[2]->PrettyClose(*theOther.mPos[0]) || mPos[2]->PrettyClose(*theOther.mPos[1]) || mPos[2]->PrettyClose(*theOther.mPos[2])) aSameCount++;
	return (aSameCount==3);
}

bool DoesLineIntersectAABB(AABBRect theRect, Point theLine1, Point theLine2) {return gMath.DoesLineIntersectAABB(theRect,theLine1,theLine2);}
bool DoesLineIntersectAABB(AABBCube theCube, Vector theLine1, Vector theLine2) {return gMath.DoesLineIntersectAABB(theCube,theLine1,theLine2);}
bool DoesRayIntersectAABB(AABBCube theCube, Vector theLine1, Vector theLine2) {return gMath.DoesRayIntersectAABB(theCube,theLine1,theLine2);}

Point Rect::MapInto(float x, float y) {return Point(gMath.ReverseInterpolate(mX,x,mX+mWidth),gMath.ReverseInterpolate(mY,y,mY+mHeight));}
Point Rect::MapOutof(float x, float y) {return Point(mX+(x*mWidth),mY+(y*mHeight));}

bool Circle::ContainsPoint(Point thePos) {return gMath.EllipseIntersectPoint(mPos,mRadius,thePos);}

#ifdef _HASGX
void GXDrawLine(Vector theStart, Vector theEnd,int theColor) {Color aC;aC.Primary(theColor);gGX.AddLine3D(theStart,theEnd,aC,2);}
void GXDrawCube(Cube theCube,int theColor) {Color aC;aC.Primary(theColor);gGX.AddCube(theCube,aC);}
#endif

