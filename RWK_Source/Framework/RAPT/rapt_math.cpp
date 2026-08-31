#include "rapt.h"
#include "os_headers.h"

Math gMath;
Math3D gMath3D;
float gPrettyCloseNumber=0;
Math::Math(void)
{
}

void Math::Go()
{
	mPI=3.1415925511f;

	mMachineEpsilon=1.0f;
	//mTiny=1e-5f;
//	mTiny=.0001f;
	mTiny=.0001f;
	while ((1.0f+.05f*mMachineEpsilon)!=1.0f) mMachineEpsilon=.5f*mMachineEpsilon;
	gPrettyCloseNumber=gMath.mMachineEpsilon;

	mCardinalDirList[0]=IPoint(-1,-1);
	mCardinalDirList[1]=IPoint(0,-1);		// Up
	mCardinalDirList[2]=IPoint(1,-1);
	mCardinalDirList[3]=IPoint(-1,0);		// Left
	mCardinalDirList[4]=IPoint(0,0);		// Center
	mCardinalDirList[5]=IPoint(1,0);		// Right
	mCardinalDirList[6]=IPoint(-1,1);
	mCardinalDirList[7]=IPoint(0,1);		// Down
	mCardinalDirList[8]=IPoint(1,1);

	mCardinal8DirList[0]=IPoint(-1,-1);
	mCardinal8DirList[1]=IPoint(0,-1);		// Up
	mCardinal8DirList[2]=IPoint(1,-1);
	mCardinal8DirList[3]=IPoint(-1,0);		// Left
	mCardinal8DirList[4]=IPoint(1,0);		// Right
	mCardinal8DirList[5]=IPoint(-1,1);
	mCardinal8DirList[6]=IPoint(0,1);		// Down
	mCardinal8DirList[7]=IPoint(1,1);

	mCardinal4DirList[0]=IPoint(0,-1);
	mCardinal4DirList[1]=IPoint(-1,0);
	mCardinal4DirList[2]=IPoint(1,0);
	mCardinal4DirList[3]=IPoint(0,1);

	mCardinal4DirListCW[0]=IPoint(0,-1);
	mCardinal4DirListCW[1]=IPoint(1,0);
	mCardinal4DirListCW[2]=IPoint(0,1);
	mCardinal4DirListCW[3]=IPoint(-1,0);


	mCardinalDirList3D[0]=IVector(-1,-1,-1);
	mCardinalDirList3D[1]=IVector(0,-1,-1);
	mCardinalDirList3D[2]=IVector(1,-1,-1);
	mCardinalDirList3D[3]=IVector(-1,0,-1);
	mCardinalDirList3D[4]=IVector(0,0,-1);	
	mCardinalDirList3D[5]=IVector(1,0,-1);	
	mCardinalDirList3D[6]=IVector(-1,1,-1);
	mCardinalDirList3D[7]=IVector(0,1,-1);	
	mCardinalDirList3D[8]=IVector(1,1,-1);
	mCardinalDirList3D[9]=IVector(-1,-1,0);
	mCardinalDirList3D[10]=IVector(0,-1,0);
	mCardinalDirList3D[11]=IVector(1,-1,0);
	mCardinalDirList3D[12]=IVector(-1,0,0);
	mCardinalDirList3D[13]=IVector(0,0,0);	
	mCardinalDirList3D[14]=IVector(1,0,0);	
	mCardinalDirList3D[15]=IVector(-1,1,0);
	mCardinalDirList3D[16]=IVector(0,1,0);	
	mCardinalDirList3D[17]=IVector(1,1,0);
	mCardinalDirList3D[18]=IVector(-1,-1,1);
	mCardinalDirList3D[19]=IVector(0,-1,1);
	mCardinalDirList3D[20]=IVector(1,-1,1);
	mCardinalDirList3D[21]=IVector(-1,0,1);
	mCardinalDirList3D[22]=IVector(0,0,1);	
	mCardinalDirList3D[23]=IVector(1,0,1);	
	mCardinalDirList3D[24]=IVector(-1,1,1);
	mCardinalDirList3D[25]=IVector(0,1,1);	
	mCardinalDirList3D[26]=IVector(1,1,1);

	mCardinal4DirList3D[0]=IVector(0,-1,0);
	mCardinal4DirList3D[1]=IVector(-1,0,0);
	mCardinal4DirList3D[2]=IVector(1,0,0);
	mCardinal4DirList3D[3]=IVector(0,1,0);
	mCardinal4DirList3D[4]=IVector(0,1,-1);
	mCardinal4DirList3D[5]=IVector(0,1,1);

	
}

int Math::NearestPowerOfTwo(int theNumber)
{
	int aRound=1;
	for (;;)
	{
		if (theNumber<=aRound) return aRound;
		aRound*=2;
	}
}

bool Math::IsPowerOfTwo(int theNumber)
{
	int aNumberOfBits=0;
	while (theNumber>0)
	{
		aNumberOfBits+=theNumber&1;
		theNumber>>=1;
	}
	return aNumberOfBits==1;
}

int	Math::GetBitPos(unsigned int theBit)
{
	int aPos=0;
	while(!(theBit&(1<<aPos))) aPos++;
	return aPos;
}


int Math::Clip(int theNumber, int theLow, int theHigh)
{
	if (theNumber<theLow) return theLow;
	if (theNumber>theHigh) return theHigh;
	return theNumber;
}

Point Math::Rotate2D(Point thePoint, Point theCenter, float theAngle)
{
	Point aPoint=thePoint;
	aPoint-=theCenter;
	Point aNewPoint;


	theAngle=Rad(theAngle);
	//theAngle=-theAngle;
	aNewPoint.mX=aPoint.mX*(float)cos(theAngle)-aPoint.mY*(float)sin(theAngle);
	aNewPoint.mY=aPoint.mX*(float)sin(theAngle)+aPoint.mY*(float)cos(theAngle);

	aNewPoint+=theCenter;
	return aNewPoint;

}

float Math::Distance(float theX1, float theY1, float theX2, float theY2)
{
	float aDX=theX2-theX1;
	float aDY=theY2-theY1;
	return (float)sqrt(aDX*aDX+aDY*aDY);
}

float Math::Distance(float theX1, float theY1, float theZ1, float theX2, float theY2, float theZ2)
{
	float aD1=theX2-theX1;
	float aD2=theY2-theY1;
	float aD3=theZ2-theZ1;
	return (float)sqrt((aD1*aD1)+(aD2*aD2)+(aD3*aD3));
}

bool Math::CircleIntersectCircle(Point theCenter1, float theRadius1, Point theCenter2, float theRadius2)
{
	Point aDelta=theCenter1-theCenter2;
	aDelta*=1.0f/(theRadius1+theRadius2);
	return (aDelta.LengthSquared()<1.0f); 
}

bool Math::EllipseIntersect(Point theCenter1, Point theRadius1, Point theCenter2, Point theRadius2)
{
	Point aDelta=theCenter1-theCenter2;
	Point aRadii=theRadius1+theRadius2;

	aDelta*=Point(1/aRadii.mX,1/aRadii.mY);
	return (aDelta.LengthSquared()<1.0f); 
}

Point Math::AngleToVector(float theAngle)
{
	return Point(Sin(theAngle),Cos(theAngle));
}

#ifndef NORANDOM
Point Math::GetRandomPoint(float theRadius)
{
	Point aPoint=AngleToVector((float)gRand.Get(360));
	aPoint*=gRand.GetF(1.0f)*theRadius;

	return aPoint;
}
#endif

Point Math::ClosestPointOnLine(Point& thePoint, Point& theLineStart, Point& theLineEnd)
{
	Vector aC=thePoint-theLineStart;
	Vector aV=theLineEnd-theLineStart; 

	float aD = aV.mX*aV.mX+aV.mY*aV.mY;
	float aT = aV.mX*aC.mX+aV.mY*aC.mY;

	if (aT<0.0f) return (theLineStart);
	if (aT>aD) return (theLineEnd);

	aV*=(aT/aD);
	return (theLineStart+aV);  


	/*
	const Point LineDiffVect = theLineEnd-theLineStart;
	const float lineSegSqrLength = LineDiffVect.LengthSquared();

	const Point LineToPointVect=thePoint-theLineStart;
	const float dotProduct = LineDiffVect.Dot(LineToPointVect);

	const float percAlongLine = dotProduct / lineSegSqrLength;

	if (percAlongLine<0.0f) return theLineStart;
	if (percAlongLine>1.0f ) return theLineEnd;

	return (theLineStart+(percAlongLine*(LineDiffVect)));//theLineEnd - LinePointStart ));
	*/

/*
	//
	// Slow version
	//
	Point aFactor1=thePoint-theLine1;
	Point aFactor2=theLine2-theLine1;
	float aLength=aFactor2.Length();
	aFactor2.Normalize();

	float aScalar=aFactor2.Dot(aFactor1);
	if (aScalar<0) return theLine1;
	if (aScalar>aLength) return theLine2;

	return theLine1+aFactor2*aScalar;

*/
}

Point Math::ClosestPointOnInfiniteLine(Point& thePoint, Point& theLine1, Point& theLine2)
{
	float A1 = theLine2.mY - theLine1.mY; 
	float B1 = theLine1.mX - theLine2.mX; 
	float C1 = (theLine2.mY - theLine1.mY)*theLine1.mX + (theLine1.mX - theLine2.mX)*theLine1.mY; 
	float C2 = -B1*thePoint.mX + A1*thePoint.mY;
	float det = A1*A1 - -B1*B1; 
	//float cx = 0; 
	//float cy = 0; 
	if(det != 0) return Point((float)((A1*C1 - B1*C2)/det),(float)((A1*C2 - -B1*C1)/det));
	return thePoint;
}


//
// Removed: Use GetLineIntersection
// 
/*
bool Math::LineIntersectLine(Point theLine1Start, Point theLine1End, Point theLine2Start, Point theLine2End, Point *theIntersection)
{
	Point a0=theLine1Start;
	Point a1=theLine1End;
	Point b0=theLine2Start;
	Point b1=theLine2End;
	float aDenom = (b1.mY - b0.mY) * (a1.mX - a0.mX) - (b1.mX - b0.mX) * (a1.mY - a0.mY);

	if (aDenom==0) return false; // Parallel line
 
	float aNumer = (b1.mX - b0.mX) * (a0.mY - b0.mY) - (b1.mY - b0.mY) * (a0.mX - b0.mX);
	float aDelta = aNumer / aDenom;
	if (aDelta<0 || aDelta>1.0) return false;

	float aX=a0.mX+aDelta*(a1.mX-a0.mX);
	float aY=a0.mY+aDelta*(a1.mY-a0.mY);
	float aTopX=_min(theLine2Start.mX,theLine2End.mX);
	float aBottomX=_max(theLine2Start.mX,theLine2End.mX);
	float aTopY=_min(theLine2Start.mY,theLine2End.mY);
	float aBottomY=_max(theLine2Start.mY,theLine2End.mY);

	float aKludge=.0001f;	// Kludge is here to get around floating point imprecision
	if (aX>=aTopX-aKludge && aX<=aBottomX+aKludge && aY>=aTopY-aKludge && aY<=aBottomY+aKludge)
	{

		if (theIntersection)
		{
			theIntersection->mX=aX;
			theIntersection->mY=aY;
		}

		return true;
	}
	return false;
}
*/

bool Math::DoesLineIntersectRectangle(Rect theRect, Point theLine1, Point theLine2)
{
	float aRectangleMinX=theRect.mX;
	float aRectangleMinY=theRect.mY;
	float aRectangleMaxX=theRect.mX+theRect.mWidth;
	float aRectangleMaxY=theRect.mY+theRect.mHeight;
	float minX = _max(_min(theLine1.mX,theLine2.mX),aRectangleMinX);
	float maxX = _min(_max(theLine1.mX,theLine2.mX),aRectangleMaxX);
	if(minX > maxX) return false;

	float minY = theLine1.mY;
	float maxY = theLine2.mY;
	float dx = theLine2.mX - theLine1.mX;
	if(dx)
	{
		float a = (theLine2.mY - theLine1.mY) / dx;
		float b = theLine1.mY - a * theLine1.mX;
		minY = a * minX + b;
		maxY = a * maxX + b;
	}
	if(minY>maxY)
	{
		float tmp = maxY;
		maxY = minY;
		minY = tmp;
	}
	if(maxY>aRectangleMaxY) maxY=aRectangleMaxY;
	if(minY<aRectangleMinY) minY=aRectangleMinY;
	return (minY<=maxY);
}

bool Math::DoesLineIntersectAABB(AABBRect theRect, Point theLine1, Point theLine2)
{
	float aRectangleMinX=theRect.mX1;
	float aRectangleMinY=theRect.mY1;
	float aRectangleMaxX=theRect.mX2;
	float aRectangleMaxY=theRect.mY2;
	float minX = _max(_min(theLine1.mX,theLine2.mX),aRectangleMinX);
	float maxX = _min(_max(theLine1.mX,theLine2.mX),aRectangleMaxX);
	if(minX > maxX) return false;

	float minY = theLine1.mY;
	float maxY = theLine2.mY;
	float dx = theLine2.mX - theLine1.mX;
	if(dx)
	{
		float a = (theLine2.mY - theLine1.mY) / dx;
		float b = theLine1.mY - a * theLine1.mX;
		minY = a * minX + b;
		maxY = a * maxX + b;
	}
	if(minY>maxY)
	{
		float tmp = maxY;
		maxY = minY;
		minY = tmp;
	}
	if(maxY>aRectangleMaxY) maxY=aRectangleMaxY;
	if(minY<aRectangleMinY) minY=aRectangleMinY;
	return (minY<=maxY);
}

/*
bool Math::DoesLineIntersectAABB(AABBCube theCube, Vector theLine1, Vector theLine2) 
{
	if (theCube.ContainsPoint(theLine1)) return true;
	if (theCube.ContainsPoint(theLine2)) return true;
	return gMath3D.DoesLineIntersectCube(theLine1,theLine2,Vector(theCube.mX1,theCube.mY1,theCube.mZ1),Vector(theCube.mX2,theCube.mX2,theCube.mY2),NULL);
}
*/

bool Math::DoesLineIntersectAABB(AABBCube theCube, Vector theLine1, Vector theLine2) 
{
	return gMath3D.DoesLineIntersectCube(theLine1,theLine2,theCube.LowerBound(),theCube.UpperBound(),NULL);
	/*
	Vector aBoxExtents, aDiff, aDir;
	Vector aCenter=theCube.Center();
	float afAWdU[3];

	aDir.mX=0.5f*(theLine2.mX-theLine1.mX);
	aBoxExtents.mX=theCube.mX2-theCube.mX1;
	aDiff.mX=(0.5f*(theLine2.mX+theLine1.mX))-aCenter.mX;
	afAWdU[0]=fabsf(aDir.mX);
	if(fabsf(aDiff.mX)>aBoxExtents.mX+afAWdU[0]) return false;

	aDir.mY=0.5f*(theLine2.mY-theLine1.mY);
	aBoxExtents.mY=theCube.mY2-theCube.mY1;
	aDiff.mY=(0.5f*(theLine2.mY+theLine1.mY))-aCenter.mY;
	afAWdU[1]=fabsf(aDir.mY);
	if(fabsf(aDiff.mY)>aBoxExtents.mY+afAWdU[1]) return false;

	aDir.mZ=0.5f*(theLine2.mZ-theLine1.mZ);
	aBoxExtents.mZ=theCube.mZ2-theCube.mZ1;
	aDiff.mZ=(0.5f*(theLine2.mZ+theLine1.mZ))-aCenter.mZ;
	afAWdU[2]=fabsf(aDir.mZ);
	if(fabsf(aDiff.mZ)>aBoxExtents.mZ+afAWdU[2]) return false;

	float aF;
	aF = aDir.mY*aDiff.mZ-aDir.mZ*aDiff.mY;if(fabsf(aF)>aBoxExtents.mY*afAWdU[2]+aBoxExtents.mZ*afAWdU[1]) return false;
	aF = aDir.mZ*aDiff.mX-aDir.mX*aDiff.mZ;if(fabsf(aF)>aBoxExtents.mX*afAWdU[2]+aBoxExtents.mZ*afAWdU[0]) return false;
	aF = aDir.mX*aDiff.mY-aDir.mY*aDiff.mX;if(fabsf(aF)>aBoxExtents.mX*afAWdU[1]+aBoxExtents.mY*afAWdU[0]) return false;
	return true;
	*/
}

bool Math::DoesLineIntersectAABB(Cube theCube, Vector theLine1, Vector theLine2) 
{
	return gMath3D.DoesLineIntersectCube(theLine1,theLine2,theCube.Corner(0),theCube.Corner(7),NULL);
	/*
	Vector aCenter=theCube.Corner(0);//theCube.Center();
	//Vector aCenter=theCube.Center();
	theLine1-=aCenter;
	theLine2-=aCenter;

	Vector aLMid=(theLine1+theLine2) * 0.5f; 
	Vector aL=(theLine1-aLMid);
	Vector aLExt=Vector(Abs(aL.mX),Abs(aL.mY),Abs(aL.mZ));

	// Use Separating Axis Test
	// Separation vector from box center to line center is LMid, since the line is in box space
	if (Abs(aLMid.mX)>theCube.mXSize+aLExt.mX) return false;
	if (Abs(aLMid.mY)>theCube.mYSize+aLExt.mY) return false;
	if (Abs(aLMid.mZ)>theCube.mZSize+aLExt.mZ) return false;
	// Crossproducts of line and each axis
	if (Abs(aLMid.mY *aL.mZ -aLMid.mZ *aL.mY)>(theCube.mYSize* aLExt.mZ + theCube.mZSize * aLExt.mY) ) return false;
	if (Abs(aLMid.mX *aL.mZ -aLMid.mZ *aL.mX)>(theCube.mXSize* aLExt.mZ + theCube.mZSize * aLExt.mX) ) return false;
	if (Abs(aLMid.mX *aL.mY -aLMid.mY *aL.mX)>(theCube.mXSize* aLExt.mY + theCube.mYSize * aLExt.mX) ) return false;
	// No separating axis, the line intersects
	return true;
	/**/

	/*
	Vector aBoxExtents, aDiff, aDir;
	Vector aCenter=theCube.Center();
	float afAWdU[3];

	aDir.mX=0.5f*(theLine2.mX-theLine1.mX);
	aBoxExtents.mX=theCube.mXSize;
	aDiff.mX=(0.5f*(theLine2.mX+theLine1.mX))-aCenter.mX;
	afAWdU[0]=fabsf(aDir.mX);
	if(fabsf(aDiff.mX)>aBoxExtents.mX+afAWdU[0]) return false;

	aDir.mY=0.5f*(theLine2.mY-theLine1.mY);
	aBoxExtents.mY=theCube.mYSize;
	aDiff.mY=(0.5f*(theLine2.mY+theLine1.mY))-aCenter.mY;
	afAWdU[1]=fabsf(aDir.mY);
	if(fabsf(aDiff.mY)>aBoxExtents.mY+afAWdU[1]) return false;

	aDir.mZ=0.5f*(theLine2.mZ-theLine1.mZ);
	aBoxExtents.mZ=theCube.mZSize;
	aDiff.mZ=(0.5f*(theLine2.mZ+theLine1.mZ))-aCenter.mZ;
	afAWdU[2]=fabsf(aDir.mZ);
	if(fabsf(aDiff.mZ)>aBoxExtents.mZ+afAWdU[2]) return false;

	float aF;
	aF = aDir.mY*aDiff.mZ-aDir.mZ*aDiff.mY;if(fabsf(aF)>aBoxExtents.mY*afAWdU[2]+aBoxExtents.mZ*afAWdU[1]) return false;
	aF = aDir.mZ*aDiff.mX-aDir.mX*aDiff.mZ;if(fabsf(aF)>aBoxExtents.mX*afAWdU[2]+aBoxExtents.mZ*afAWdU[0]) return false;
	aF = aDir.mX*aDiff.mY-aDir.mY*aDiff.mX;if(fabsf(aF)>aBoxExtents.mX*afAWdU[1]+aBoxExtents.mY*afAWdU[0]) return false;
	return true;
	/**/
}

bool Math::DoesRayIntersectAABB(AABBCube theCube, Vector theOrigin, Vector theDir)
{
	Vector aBoxExtents, aDiff;
	Vector aCenter=theCube.Center();

	aDiff.mX=theOrigin.mX-aCenter.mX;
	aBoxExtents.mX=theCube.mX2-theCube.mX1;
	if(fabsf(aDiff.mX)>aBoxExtents.mX && aDiff.mX*theDir.mX>=0.0f) return false;

	aDiff.mY=theOrigin.mY-aCenter.mY;
	aBoxExtents.mY=theCube.mY2-theCube.mY1;
	if(fabsf(aDiff.mY)>aBoxExtents.mY && aDiff.mY*theDir.mY>=0.0f) return false;

	aDiff.mZ=theOrigin.mZ-aCenter.mZ;
	aBoxExtents.mZ=theCube.mZ2-theCube.mZ1;
	if(fabsf(aDiff.mZ)>aBoxExtents.mZ && aDiff.mZ*theDir.mZ>=0.0f) return false;

	float afAWdU[3];
	afAWdU[0]=fabsf(theDir.mX);
	afAWdU[1]=fabsf(theDir.mY);
	afAWdU[2]=fabsf(theDir.mZ);

	float aF;
	aF = theDir.mY*aDiff.mZ-theDir.mZ*aDiff.mY;if(fabsf(aF)>aBoxExtents.mY*afAWdU[2]+aBoxExtents.mZ*afAWdU[1]) return false;
	aF = theDir.mZ*aDiff.mX-theDir.mX*aDiff.mZ;if(fabsf(aF)>aBoxExtents.mX*afAWdU[2]+aBoxExtents.mZ*afAWdU[0]) return false;
	aF = theDir.mX*aDiff.mY-theDir.mY*aDiff.mX;if(fabsf(aF)>aBoxExtents.mX*afAWdU[1]+aBoxExtents.mY*afAWdU[0]) return false;
	return true;
}

bool Math::DoesRayIntersectAABB(Cube theCube, Vector theOrigin, Vector theDir)
{
	Vector aBoxExtents, aDiff;
	Vector aCenter=theCube.Center();

	aDiff.mX=theOrigin.mX-aCenter.mX;
	aBoxExtents.mX=theCube.mXSize;
	if(fabsf(aDiff.mX)>aBoxExtents.mX && aDiff.mX*theDir.mX>=0.0f) return false;

	aDiff.mY=theOrigin.mY-aCenter.mY;
	aBoxExtents.mY=theCube.mYSize;
	if(fabsf(aDiff.mY)>aBoxExtents.mY && aDiff.mY*theDir.mY>=0.0f) return false;

	aDiff.mZ=theOrigin.mZ-aCenter.mZ;
	aBoxExtents.mZ=theCube.mZSize;
	if(fabsf(aDiff.mZ)>aBoxExtents.mZ && aDiff.mZ*theDir.mZ>=0.0f) return false;

	float afAWdU[3];
	afAWdU[0]=fabsf(theDir.mX);
	afAWdU[1]=fabsf(theDir.mY);
	afAWdU[2]=fabsf(theDir.mZ);

	float aF;
	aF = theDir.mY*aDiff.mZ-theDir.mZ*aDiff.mY;if(fabsf(aF)>aBoxExtents.mY*afAWdU[2]+aBoxExtents.mZ*afAWdU[1]) return false;
	aF = theDir.mZ*aDiff.mX-theDir.mX*aDiff.mZ;if(fabsf(aF)>aBoxExtents.mX*afAWdU[2]+aBoxExtents.mZ*afAWdU[0]) return false;
	aF = theDir.mX*aDiff.mY-theDir.mY*aDiff.mX;if(fabsf(aF)>aBoxExtents.mX*afAWdU[1]+aBoxExtents.mY*afAWdU[0]) return false;
	return true;
}



bool Math::IsCCW(Point& theP1, Point& theP2, Point& theP3)
{
	return ((theP2.mX-theP1.mX)*(theP3.mY-theP1.mY)>(theP2.mY-theP1.mY)*(theP3.mX-theP1.mX));
	//return (theP2.mX - theP1.mX)*(theP3.mY-theP1.mY)-(theP3.mX-theP1.mX)*(theP2.mY-theP1.mY)>0;
}

bool Math::IsCCW(Vector theP1, Vector theP2, Vector theP3, Vector viewFrom)
{
	Vector aBA=theP2-theP1;
	Vector aCA=theP3-theP1;
	Vector aNorm=aBA.Cross(aCA).Normal();
	float aW=aNorm.Dot(theP1-viewFrom);
	return (aW>0);
}

bool Math::IsTriangleDegenerate(DVector theA, DVector theB, DVector theC) 
{
	DVector aBA=theB-theA;
	DVector aCB=theC-theB;
	DVector aAC=theA-theC;
	double aL[3];
	aL[0]=aBA.Length();
	aL[1]=aCB.Length();
	aL[2]=aAC.Length();

	if (aL[0]>aL[1]) Swap<double>(aL[0],aL[1]);
	if (aL[0]>aL[2]) Swap<double>(aL[0],aL[2]);
	if (aL[1]>aL[2]) Swap<double>(aL[1],aL[2]);
	if (aL[0]>aL[1]) Swap<double>(aL[0],aL[1]);
	if (aL[0]>aL[2]) Swap<double>(aL[0],aL[2]);
	if (aL[1]>aL[2]) Swap<double>(aL[1],aL[2]);

	//gOut.Out("Sort: %9.7f + %9.7f > %9.7f",aL[0],aL[1],aL[2]);
	//gOut.Out("Sort: %9.7f > %9.7f",aL[0]+aL[1],aL[2]);

//	return !((aL[0]+aL[1])>(aL[2]-gMath.mMachineEpsilon));

	return !((aL[0]+aL[1])>(aL[2]));

	//return !((aL[0]+aL[1])>(aL[2]));

	//return (gMath.DistanceSquared(theA,theB)<gMath.mMachineEpsilon)|(gMath.DistanceSquared(theB,theC)<gMath.mMachineEpsilon)|(gMath.DistanceSquared(theC,theA)<gMath.mMachineEpsilon);	
	/*
	Vector aBA=theB-theA;
	Vector aCB=theC-theB;
	Vector aAC=theA-theC;
	aBA.Normalize();
	aCB.Normalize();
	aAC.Normalize();
	return (gMath.Abs(aBA.Dot(aCB))>=1.0f) && (gMath.Abs(aCB.Dot(aAC))>=1.0f) && (gMath.Abs(aAC.Dot(aBA))>=1.0f);
	*/
}

bool Math::IsTriangleDegenerate(Point theA, Point theB, Point theC)
{
	/*
	float aThresh=.000001f;
	if (gMath.DistanceSquared(theA,theB)<aThresh || gMath.DistanceSquared(theB,theC)<aThresh || gMath.DistanceSquared(theC,theA)<aThresh) return true;
	return IsTriangleLinear(theA,theB,theC,aThresh);
	*/

	Point aBA=theB-theA;
	Point aCB=theC-theB;
	Point aAC=theA-theC;
	float aL[3];
	aL[0]=aBA.Length();
	aL[1]=aCB.Length();
	aL[2]=aAC.Length();

	if (aL[0]>aL[1]) Swap<float>(aL[0],aL[1]);
	if (aL[0]>aL[2]) Swap<float>(aL[0],aL[2]);
	if (aL[1]>aL[2]) Swap<float>(aL[1],aL[2]);
	if (aL[0]>aL[1]) Swap<float>(aL[0],aL[1]);
	if (aL[0]>aL[2]) Swap<float>(aL[0],aL[2]);
	if (aL[1]>aL[2]) Swap<float>(aL[1],aL[2]);

	//bool aResult=!((aL[0]+aL[1])>(aL[2]));
	//if (!aResult) aResult=IsTriangleLinear(theA,theB,theC,.00001f);
	//return aResult;

	return !((aL[0]+aL[1])>(aL[2]));
}


bool Math::IsTriangleDegenerate(Vector theA, Vector theB, Vector theC)
{
	/*
	float aThresh=.000001f;
	if (gMath.DistanceSquared(theA,theB)<aThresh || gMath.DistanceSquared(theB,theC)<aThresh || gMath.DistanceSquared(theC,theA)<aThresh) return true;
	return IsTriangleLinear(theA,theB,theC,aThresh);
	/**/

	//*
	Vector aBA=theB-theA;
	Vector aCB=theC-theB;
	Vector aAC=theA-theC;
	float aL[3];
	aL[0]=aBA.Length();
	aL[1]=aCB.Length();
	aL[2]=aAC.Length();

	if (aL[0]>aL[1]) Swap<float>(aL[0],aL[1]);
	if (aL[0]>aL[2]) Swap<float>(aL[0],aL[2]);
	if (aL[1]>aL[2]) Swap<float>(aL[1],aL[2]);
	if (aL[0]>aL[1]) Swap<float>(aL[0],aL[1]);
	if (aL[0]>aL[2]) Swap<float>(aL[0],aL[2]);
	if (aL[1]>aL[2]) Swap<float>(aL[1],aL[2]);

	//bool aResult=!((aL[0]+aL[1])>(aL[2]));
	//if (!aResult) aResult=IsTriangleLinear(theA,theB,theC,thePrettyCloseThreshold);
	//return aResult;

	return !((aL[0]+aL[1])>(aL[2]));
	/**/
}

bool Math::IsTriangleLinear(Vector theA, Vector theB, Vector theC, float thePrettyCloseThreshold)
{
	//
	// Note: You have to check TWO lines, in case point A is the one that's far away from the other two.
	//
	float aD=gMath3D.DistanceToInfiniteLine(theA,theB,theC);
	if (aD<thePrettyCloseThreshold) return true;
	aD=gMath3D.DistanceToInfiniteLine(theB,theC,theA);
	return (aD<thePrettyCloseThreshold*thePrettyCloseThreshold);
}

bool Math::IsTriangleLinear(Point theA, Point theB, Point theC, float thePrettyCloseThreshold)
{
	//
	// Note: You have to check TWO lines, in case point A is the one that's far away from the other two.
	//
	float aD=gMath3D.DistanceToInfiniteLineSquared(theA,theB,theC);
	if (aD<thePrettyCloseThreshold) return true;
	aD=gMath3D.DistanceToInfiniteLineSquared(theB,theC,theA);
	return (aD<thePrettyCloseThreshold*thePrettyCloseThreshold);
}

bool Math::DoesLineIntersectLine(Point theLine1Start, Point theLine1End, Point theLine2Start, Point theLine2End)
{
	AABBRect aR1;
	AABBRect aR2;
	aR1.mX1=_min(theLine1Start.mX,theLine1End.mX);
	aR1.mY1=_min(theLine1Start.mY,theLine1End.mY);
	aR1.mX2=_max(theLine1Start.mX,theLine1End.mX);
	aR1.mY2=_max(theLine1Start.mY,theLine1End.mY);
	aR2.mX1=_min(theLine2Start.mX,theLine2End.mX);
	aR2.mY1=_min(theLine2Start.mY,theLine2End.mY);
	aR2.mX2=_max(theLine2Start.mX,theLine2End.mX);
	aR2.mY2=_max(theLine2Start.mY,theLine2End.mY);
	if (!aR1.Intersects(aR2)) return false;

	return (IsCCW(theLine1Start, theLine2Start, theLine2End)!=IsCCW(theLine1End, theLine2Start, theLine2End) && IsCCW(theLine1Start, theLine1End, theLine2Start)!=IsCCW(theLine1Start, theLine1End, theLine2End));
}

bool Math::GetLineIntersection(Point theLine1Start, Point theLine1End, Point theLine2Start, Point theLine2End, Point *theIntersection)
{
	if (DoesLineIntersectLine(theLine1Start,theLine1End,theLine2Start,theLine2End))
	{
		float A[2];
		float B[2];
		float C[2];
		float aDenom;
		Point aPt;

		A[0]=theLine1End.mY-theLine1Start.mY;
		B[0]=theLine1Start.mX-theLine1End.mX;
		C[0]=A[0]*theLine1Start.mX+B[0]*theLine1Start.mY;

		A[1]=theLine2End.mY-theLine2Start.mY;
		B[1]=theLine2Start.mX-theLine2End.mX;
		C[1]=A[1]*theLine2Start.mX+B[1]*theLine2Start.mY;

		aDenom=A[0]*B[1]-A[1]*B[0];
		if (aDenom!=0)
		{
			aPt.mX=(B[1]*C[0]-B[0]*C[1])/aDenom;
			aPt.mY=(A[0]*C[1]-A[1]*C[0])/aDenom;
			*theIntersection=aPt;
			return true;
		}
	}

	*theIntersection=Point(0,0);
	return false;
}

bool Math::GetLineIntersection(Vector theLine1Start, Vector theLine1End, Vector theLine2Start, Vector theLine2End, Vector *theIntersection)
{
	Vector aDA=theLine1End-theLine1Start;//Coord da = a.second - a.first; 
	Vector aDB=theLine2End-theLine2Start;//Coord db = b.second - b.first;
	Vector aDC=theLine2Start-theLine1Start;//Coord dc = b.first - a.first;

	if (aDC.Dot(aDA.Cross(aDB))!=0) return false;//if (dot(dc, cross(da,db)) != 0.0) return false;// lines are not coplanar

	float aS=(Cross(aDC,aDB).Dot(aDA.Cross(aDB))) / (aDA.Cross(aDB).Norm2()); //Point s = dot(cross(dc,db),cross(da,db)) / norm2(cross(da,db));
	if (aS>=0.0f && aS<=1.0f)
	{
		if (theIntersection) *theIntersection=theLine1Start+(aDA*aS); //ip = a.first + da * Coord(s,s,s);
		return true;
	}
	return false;
}

bool Math::GetInfiniteLineIntersection(Vector theLine1Start, Vector theLine1End, Vector theLine2Start, Vector theLine2End, Vector *theIntersection)
{
	Vector aDA=theLine1End-theLine1Start;//Coord da = a.second - a.first; 
	Vector aDB=theLine2End-theLine2Start;//Coord db = b.second - b.first;
	Vector aDC=theLine2Start-theLine1Start;//Coord dc = b.first - a.first;

	if (aDC.Dot(aDA.Cross(aDB))!=0) return false;//if (dot(dc, cross(da,db)) != 0.0) return false;// lines are not coplanar

	float aS=(Cross(aDC,aDB).Dot(aDA.Cross(aDB))) / (aDA.Cross(aDB).Norm2()); //Point s = dot(cross(dc,db),cross(da,db)) / norm2(cross(da,db));
	if (theIntersection) *theIntersection=theLine1Start+(aDA*aS); //ip = a.first + da * Coord(s,s,s);
	return true;
}


bool Math::GetInfiniteLineIntersection(Point theLine1Start, Point theLine1End, Point theLine2Start, Point theLine2End, Point *theIntersection)
{
	float bx = theLine1End.mX - theLine1Start.mX;
	float by = theLine1End.mY - theLine1Start.mY;
	float dx = theLine2End.mX - theLine2Start.mX;
	float dy = theLine2End.mY - theLine2Start.mY; 
	float b_dot_d_perp = bx*dy - by*dx;
	if(b_dot_d_perp == 0) return false;

	float cx = theLine2Start.mX-theLine1Start.mX; 
	float cy = theLine2Start.mY-theLine1Start.mY;
	float t = (cx*dy - cy*dx) / b_dot_d_perp; 

	if (theIntersection)
	{
		theIntersection->mX=theLine1Start.mX+t*bx;
		theIntersection->mY=theLine1Start.mY+t*by;
	}
	return true;
}



bool Math::EllipseIntersectLine(Point theCenter, Point theRadius, Point theLine1, Point theLine2)
{
	/*
	//
	// Get the closest point on the line to the center of the ellipse.
	//
	Point aClosest=ClosestPointOnLine(theCenter,theLine1,theLine2);

	//
	// Move it into ellipse space
	//
	aClosest-=theCenter;

	//
	// Figure out some ellipse stuff
	//
	float aMajorAxis=_max(theRadius.mX,theRadius.mY);
	float aMinorAxis=_min(theRadius.mX,theRadius.mY);
	float aAxisRatio=aMinorAxis/aMajorAxis;

	//
	// Scale our point
	//
	if (aMajorAxis==theRadius.mX) aClosest.mX*=aAxisRatio;
	if (aMajorAxis==theRadius.mY) aClosest.mY*=aAxisRatio;

	//
	// Is it in the circle?
	//
	if (aClosest.Length()<=aMinorAxis) return true;
	return false;
	*/

	// Quick check...
	if (EllipseIntersectPoint(theCenter,theRadius,theLine1)) return true;
	if (EllipseIntersectPoint(theCenter,theRadius,theLine2)) return true;
	if (EllipseIntersectPoint(theCenter,theRadius,(theLine1+theLine2)/2.0f)) return true;

	theLine1-=theCenter;
	theLine2-=theCenter;

	float aA=powf(theLine2.mX-theLine1.mX,2.0f)/theRadius.mX/theRadius.mX+powf(theLine2.mY-theLine1.mY,2.0f)/theRadius.mY/theRadius.mY;
	float aB=2*theLine1.mX*(theLine2.mX-theLine1.mX)/theRadius.mX/theRadius.mX+2*theLine1.mY*(theLine2.mY-theLine1.mY)/theRadius.mY/theRadius.mY;
	float aC=theLine1.mX*theLine1.mX/theRadius.mX/theRadius.mX+theLine1.mY*theLine1.mY/theRadius.mY/theRadius.mY-1.0f;
	float aD=aB*aB-4*aA*aC;
	if (aD==0)
	{
		float aT=-aB/2.0f/aA;
		return (aT>=0 && aT<=1);
	}
	if (aD>0)
	{
		float aSqrt=sqrtf(aD);
		float aT1=(-aB+aSqrt)/2.0f/aA;
		float aT2=(-aB-aSqrt)/2.0f/aA;
		return (aT1>=0 && aT1<=1) || (aT2>=0 && aT2<=1);
	}
	return false;
}






#ifndef NORANDOM
Point Math::AngleToVector(Random *theRandom)
{
	if (theRandom==NULL) theRandom=&gRand;
	return AngleToVector(theRandom->GetF(360));
}
#endif

bool Math::CollideCircleCircle(Point theMovingCenter, float theMovingRadius, Point theMovementVector, Point theStationaryCenter, float theStationaryRadius, Point *theResultVector)
{
	//
	// In the event of no collision, we'll want our result filled
	// with a "normal" movement.
	//
	if (theResultVector) *theResultVector=theMovementVector;

	//
	// First off, if the movement doesn't even take us near eachother
	// then toss it.
	//
	float aDistance=Point(theMovingCenter-theStationaryCenter).Length();
	float aRadius2=theMovingRadius+theStationaryRadius;
	aDistance-=aRadius2;
	if (theMovementVector.Length()<aDistance) return false;

	//
	// If we're not even moving toward the circle, then lose it, too.
	//
	Point aN=theMovementVector;aN.Normalize();
	Point aC=theStationaryCenter-theMovingCenter;
	float aD=aN.Dot(aC);
	if (aD<=0) return false;

	//
	// If we've moving toward, but won't come close for
	// whatever reason, then bubbye!
	//
	float aLengthC=aC.Length();
	float aF=(aLengthC*aLengthC)-(aD*aD);
	float aSumRadius2=aRadius2*aRadius2;
	if (aF>aSumRadius2) return false;

	float aT=aSumRadius2-aF;
	if (aT<0) return false;

	//
	// Okay, if our resultant postulated movevment isn't
	// long enough, we didn't actually touch!
	//
	float aDistanceToTravel=aD-(float)sqrt(aT);
	float aMagnitude=theMovementVector.Length();
	if (aMagnitude<aDistanceToTravel) return false;

	theMovementVector.Normalize();
	theMovementVector.SetLength(aDistanceToTravel);

	if (theResultVector) *theResultVector=theMovementVector;
	return true;
}

float Math::NormalizeAngle(float theAngle)
{
	int a = (int)theAngle;
	theAngle=theAngle-(float)a;
	a%=360;

	float fret = (float)a + theAngle;
	if(fret<0.0f) fret +=360.0f;
	return fret;}

float Math::GetBestRotation(float theSourceAngle, float theDestAngle) 
{
	float start = NormalizeAngle(theSourceAngle);
	float target = NormalizeAngle(theDestAngle);


	if(fabs(start-target)<=1.0f || fabs(start-target)>=359.0f) return 0;

	if(target>start)
	{
		if(target-start>180.0f) return -1; else return 1;
	}
	if(start>target)
	{
		if(start-target>180.0f) return 1; else return -1;
	}

	// MUST BE ONE OF ABOVE CASES
	return(0);
/*
	float a1,a2;

	while (theSourceAngle>360) theSourceAngle-=360;
	while (theSourceAngle<0) theSourceAngle+=360;
	while (theDestAngle>360) theDestAngle-=360;
	while (theDestAngle<0) theDestAngle+=360;

	a1=theSourceAngle;
	a2=theSourceAngle;

	for (int aCount=0;aCount<360;aCount++) 
	{
		float aOldA1=a1;
		float aOldA2=a2;

		a1++;if (a1>360) {a1-=361;aOldA1-=361;}
		a2--;if (a2<0) {a2+=360;aOldA2+=360;}

		if (a1>=theDestAngle && aOldA1<theDestAngle) return 1;
		if (a2<=theDestAngle && aOldA2>theDestAngle) return -1;
	}

	return 0;
/**/
}

bool Math::CrossTheshold(float theFirstNumber, float theSecondNumber, float theThreshold)
{
	if (theFirstNumber<=theThreshold && theSecondNumber>theThreshold) return true;
	if (theFirstNumber<theThreshold && theSecondNumber>=theThreshold) return true;
	if (theFirstNumber>=theThreshold && theSecondNumber<theThreshold) return true;
	if (theFirstNumber>theThreshold && theSecondNumber<=theThreshold) return true;
	return false;
}


float Math::GetDistanceBetweenAngles(float theAngle1, float theAngle2)
{
	//return 180.0f-fabs(fmod(fabs(theAngle1-theAngle2),360.0f)-180.0f);

	float aCrossDiff=360-_max(theAngle1,theAngle2)+_min(theAngle1,theAngle2);
	float aDirectDiff=_max(theAngle1,theAngle2)-_min(theAngle1,theAngle2);
	return _min(aCrossDiff,aDirectDiff);
}

float Math::GetDifferenceBetweenAngles(float theAngle1, float theAngle2)
{
    float aResult=Deg((float)(atan2(Sin(theAngle2-theAngle1),Cos(theAngle2-theAngle1))));
	if (aResult>0) return 180-aResult;
	else return (-180)-aResult;
}

bool Math::IsPointInSector(Point theCenter, float theStartAngle, float theEndAngle, Point theTestPoint)
{
	Point aStart=gMath.AngleToVector(theStartAngle);
	Point aEnd=gMath.AngleToVector(theEndAngle);
	Point aPos=theTestPoint-theCenter;

	float aStartAngle=(float)atan2(aStart.mY,aStart.mX);
	float aEndAngle=(float)atan2(aEnd.mY,aEnd.mX);
	float aAngle=(float)atan2(aPos.mY,aPos.mX);

//	gOut.Out("Angle Shit: %f -> %f - %f",Deg(aAngle),Deg(aStartAngle),Deg(aEndAngle));

	if (aAngle>=aStartAngle && aAngle<=aEndAngle) return true;
	if (aEndAngle<aStartAngle && (aAngle>=aStartAngle || aAngle<=aEndAngle)) return true;
/*
	{
		if (aAngle>=aStartAngle) return true;
		if (aAngle<=aEndAngle) return true;
	}
*/
	return false;
}

bool Math::IsPointInCone(Point theConeCenter, Point theConeVector, float coneAngleHalfWidth, Point theTestPoint)
{
	Point aTowardVec=theTestPoint-theConeCenter;
	aTowardVec.Normalize();
	if (theConeVector.LengthSquared()!=1) theConeVector.Normalize();

	float aDot=aTowardVec.Dot(theConeVector);
	float aDeg=Deg((float)(acos(aDot)));
	return (aDeg<coneAngleHalfWidth);



/*
	float aStart=fmod(coneAngleStart,360);
	float aEnd=fmod(coneAngleEnd,360);
	Point aVec=theTestPoint-theConeCenter;
	int aAngle=(int)(fmod(VectorToAngle(aVec),360));

	float aTest11=aStart;
	float aTest12=aStart+coneWidth;
	float aTest21=aEnd-coneWidth;
	float aTest22=aEnd;

	if ((aAngle>=aTest11 && aAngle<aTest12) || (aAngle>=aTest21 && aAngle<=aTest22)) return true;
	return false;
*/
}

/*
float Math::FastSqrt(float number)
{
	unsigned int i;
	float x2, y;
	const float threehalfs=1.5F;

	x2=number*0.5F;
	y=number;
	i=*(unsigned int*)&y;       
	i=0x5f3759df-(i>>1);  
	y=*(float*)&i;
	y=y*(threehalfs-(x2*y*y));
	//y=y*(threehalfs-(x2*y*y)); // Adds accuracy
	return (1/y);
}
*/

bool Math::RangeIntersect(Point theRange1, Point theRange2)
{
	if (theRange1.mX>=theRange2.mX && theRange1.mX<=theRange2.mY) return true;
	if (theRange1.mY>=theRange2.mX && theRange1.mY<=theRange2.mY) return true;

	if (theRange2.mX>=theRange1.mX && theRange2.mX<=theRange1.mY) return true;
	if (theRange2.mY>=theRange1.mX && theRange2.mY<=theRange1.mY) return true;

	return false;

}


//
// Helper Functions and Constants for
// clipping a line to a rect.
//
const int OUTCODE_INSIDE = 0; // 0000
const int OUTCODE_LEFT = 1;   // 0001
const int OUTCODE_RIGHT = 2;  // 0010
const int OUTCODE_BOTTOM = 4; // 0100
const int OUTCODE_TOP = 8;    // 1000
inline int ComputeOutCode(float x, float y, float xmin, float ymin, float xmax, float ymax)
{
	int aCode;

	aCode=OUTCODE_INSIDE;
	if (x<xmin) aCode|=OUTCODE_LEFT;
	else if (x>xmax) aCode|=OUTCODE_RIGHT;
	if (y<ymin) aCode|=OUTCODE_BOTTOM;
	else if (y>ymax) aCode|=OUTCODE_TOP;

	return aCode;
}

Line Math::ClipLineToRectangle(Line theLine, Rect theRect)
{
	float x0=theLine.mPos[0].mX;
	float x1=theLine.mPos[1].mX;
	float y0=theLine.mPos[0].mY;
	float y1=theLine.mPos[1].mY;

	float xmin=theRect.mX;
	float ymin=theRect.mY;
	float xmax=theRect.mX+theRect.mWidth;
	float ymax=theRect.mY+theRect.mHeight;

	int aOutcode0=ComputeOutCode(x0,y0,xmin,ymin,xmax,ymax);
	int aOutcode1=ComputeOutCode(x1,y1,xmin,ymin,xmax,ymax);
	bool aAccept = false;

	while (true) 
	{
		if (!(aOutcode0|aOutcode1)) {aAccept = true;break;}
		else if (aOutcode0 & aOutcode1) break;
		else 
		{
			float x, y;
			int aOutcodeOut=aOutcode0?aOutcode0:aOutcode1;

			if (aOutcodeOut&OUTCODE_TOP) 
			{
				x = x0 + (x1 - x0) * (ymax - y0) / (y1 - y0);
				y = ymax;
			} 
			else if (aOutcodeOut&OUTCODE_BOTTOM) 
			{
				x = x0 + (x1 - x0) * (ymin - y0) / (y1 - y0);
				y = ymin;
			} 
			else if (aOutcodeOut&OUTCODE_RIGHT) 
			{
				y = y0 + (y1 - y0) * (xmax - x0) / (x1 - x0);
				x = xmax;
			} 
			else if (aOutcodeOut&OUTCODE_LEFT) 
			{
				y = y0 + (y1 - y0) * (xmin - x0) / (x1 - x0);
				x = xmin;
			}
			// Now we move outside point to intersection point to clip
			// and get ready for next pass.
			if (aOutcodeOut == aOutcode0) {
				x0 = x;
				y0 = y;
				aOutcode0=ComputeOutCode(x0, y0,xmin,ymin,xmax,ymax);
			} 
			else 
			{
				x1 = x;
				y1 = y;
				aOutcode1=ComputeOutCode(x1, y1,xmin,ymin,xmax,ymax);
			}

		}
	}

	if (aAccept) return Line(Point(x0,y0),Point(x1,y1));

	//
	// If the line is completely outside, just
	// return the line clipped to the line's start point.
	// It's our job to make sure the line isn't zero length outside.
	//
	return Line(Point(x0,y0),Point(x0,y0));
}

Line3D Math::ClipLineToCube(Line3D theLine, Cube theCube)
{
	if (theCube.ContainsPoint(theLine.mPos[0]) && theCube.ContainsPoint(theLine.mPos[1])) return theLine;
	if (!DoesLineIntersectAABB(theCube,theLine)) return Line3D(theLine.mPos[0],theLine.mPos[0]);

	Volume aV;
	aV.FromCube(theCube);

	float aBestDist[2]={FLT_MAX,FLT_MAX};
	Vector aBest[2];

	if (theCube.ContainsPoint(theLine.mPos[0])) {aBestDist[0]=0;aBest[0]=theLine.mPos[0];}
	if (theCube.ContainsPoint(theLine.mPos[1])) {aBestDist[1]=0;aBest[1]=theLine.mPos[1];}

	foreach(aP,aV.mPlane)
	{
		for (int aCount=0;aCount<2;aCount++)
		{
			Vector aHit;
			if (gMath.DoesInfiniteLineIntersectPlane(theLine,aP,&aHit))
			{
				float aDist=gMath.DistanceSquared(theLine.mPos[aCount],aHit);
				if (aDist<aBestDist[aCount])
				{
					if (aV.ContainsPoint(aHit))
					{
						aBestDist[aCount]=aDist;
						aBest[aCount]=aHit;
					}
				}
			}
		}
	}

	return Line3D(aBest[0],aBest[1]);

}

Line3D Math::ClipLineToCube(Line3D theLine, Cube theCube, Matrix theMatrix)
{
	Matrix aIMat=theMatrix.GetInvert();
	Line3D aWorkLine;
	aWorkLine.mPos[0]=aIMat.ProcessPoint(theLine.mPos[0]);
	aWorkLine.mPos[1]=aIMat.ProcessPoint(theLine.mPos[1]);

	if (theCube.ContainsPoint(aWorkLine.mPos[0]) && theCube.ContainsPoint(aWorkLine.mPos[1])) return theLine;
	if (!DoesLineIntersectAABB(theCube,aWorkLine)) return Line3D(theLine.mPos[0],theLine.mPos[0]);

	Volume aV;
	aV.FromCube(theCube);

	float aBestDist[2]={FLT_MAX,FLT_MAX};
	Vector aBest[2];

	if (theCube.ContainsPoint(aWorkLine.mPos[0])) {aBestDist[0]=0;aBest[0]=aWorkLine.mPos[0];}
	if (theCube.ContainsPoint(aWorkLine.mPos[1])) {aBestDist[1]=0;aBest[1]=aWorkLine.mPos[1];}

	foreach(aP,aV.mPlane)
	{
		for (int aCount=0;aCount<2;aCount++)
		{
			Vector aHit;
			if (gMath.DoesInfiniteLineIntersectPlane(aWorkLine,aP,&aHit))
			{
				float aDist=gMath.DistanceSquared(aWorkLine.mPos[aCount],aHit);
				if (aDist<aBestDist[aCount])
				{
					if (aV.ContainsPoint(aHit))
					{
						aBestDist[aCount]=aDist;
						aBest[aCount]=aHit;
					}
				}
			}
		}
	}

	return Line3D(theMatrix.ProcessPoint(aBest[0]),theMatrix.ProcessPoint(aBest[1]));

}


Point Math::RandomPointOnLine(Point& theLine1, Point& theLine2)
{
	float aT=gRand.GetF(1.0f);
	Point aResult;
	aResult.mX=(1-aT)*theLine1.mX+aT*theLine2.mX;
	aResult.mY=(1-aT)*theLine1.mY+aT*theLine2.mY;
	return aResult;
}

Math3D::Math3D(void)
{
}

void Math3D::Go()
{
}

float Math3D::DistanceSquared(Vector theVector1, Vector theVector2)
{
	return (theVector2-theVector1).LengthSquared();
}

void Math3D::MakeSphere(float thePoints, float theRadius, Array<Vector>& theArray)
{
	theArray.ForceSize((int)thePoints);
	float aInc=gMath.mPI*(3-(float)sqrt(5.0f));
	float aOff=2.0f/thePoints;

	int aCount=0;
	for (float aCounter=0;aCounter<thePoints;aCounter++)
	{
		//float aY=gMath.Sin(aCount/aNumberOfPoints)*aRadius;
		float aY=aCounter*aOff-1+(aOff/2);
		float aR=(float)sqrt(1-aY*aY);
		aY/=aR;

		float aPHI=aCounter*aInc;
		float aX=(float)cos(aPHI);//*aRadius;
		float aZ=(float)sin(aPHI);//*aRadius;

		theArray[aCount]=Vector(aX,aY,aZ);
		theArray[aCount].SetLength(theRadius);
		aCount++;
	}
}


//
// Edge axis seperation test... 
//
/*
bool DoAxisSeparationTest(Point x1, Point x2, Point x3, Point[] otherQuadPoints) {
	Vector vec = x2 - x1;
	Vector rotated = new Vector(-vec.Y, vec.X);

	bool refSide = (rotated.X * (x3.X - x1.X)
		+ rotated.Y * (x3.Y - x1.Y)) >= 0;

	foreach (Point pt in otherQuadPoints) {
		bool side = (rotated.X * (pt.X - x1.X) 
			+ rotated.Y * (pt.Y - x1.Y)) >= 0;
		if (side == refSide) {
			// At least one point of the other quad is one the same side as x3. Therefor the specified edge can't be a
			// separating axis anymore.
			return false;
		}
	}

	// All points of the other quad are on the other side of the edge. Therefor the edge is a separating axis and
	// the quads don't intersect.
	return true;
}
*/

float Math::PerlinNoise(float x, float y)
{
	float floorx=(float)((int)x);//This is kinda a cheap way to floor a float integer.
	float floory=(float)((int)y);
	float s,t,u,v;//Integer declaration
	s=Noise2D(floorx,floory);
	t=Noise2D(floorx+1,floory);
	u=Noise2D(floorx,floory+1);//Get the surrounding pixels to calculate the transition.
	v=Noise2D(floorx+1,floory+1);
	float int1=Interpolate(s,t,x-floorx);//Interpolate between the values.
	float int2=Interpolate(u,v,x-floorx);//Here we use x-floorx, to get 1st dimension. Don't mind the x-floorx thingie, it's part of the cosine formula.
	return Interpolate(int1,int2,y-floory);//Here we use y-floory, to get the 2nd dimension.
}

bool Math::IsCircleInsideCircle(Point theCircle, float theRadius, Point theInsideCircle, float theInsideRadius)
{
	if (theInsideRadius>theRadius) return false;
	Point aVec=theInsideCircle-theCircle;

	float aCheckLength=theRadius-theInsideRadius;
	aCheckLength*=aCheckLength;

	if (aVec.LengthSquared()<aCheckLength) return true;
	return false;
}

bool Math::IsPointInQuad(Point thePoint, Quad& theQuad)
{
	if (IsPointInTriangle(&thePoint,&theQuad.mCorner[0],&theQuad.mCorner[1],&theQuad.mCorner[2])) return true;
	if (IsPointInTriangle(&thePoint,&theQuad.mCorner[1],&theQuad.mCorner[3],&theQuad.mCorner[2])) return true;
	return false;
}

float GeoSign(Point* v1, Point* v2, Point* v3) {return (v1->mX - v3->mX) * (v2->mY - v3->mY) - (v2->mX - v3->mX) * (v1->mY - v3->mY);}
float GeoSign2D(Point* v1, Vector* v2, Vector* v3) {return (v1->mX - v3->mX) * (v2->mY - v3->mY) - (v2->mX - v3->mX) * (v1->mY - v3->mY);}

bool Math::IsPointInTrianglePrecise(Point thePoint, Point theTriangle1, Point theTriangle2, Point theTriangle3, float theEpsilon)
{
	if (IsTriangleDegenerate(theTriangle1,theTriangle2,theTriangle3)) return false;
	//if (gMath.DistanceSquared(theTriangle1,theTriangle2)<theEpsilon) return false;
	//if (gMath.DistanceSquared(theTriangle2,theTriangle3)<theEpsilon) return false;
	//if (gMath.DistanceSquared(theTriangle1,theTriangle3)<theEpsilon) return false;

	theEpsilon*=theEpsilon;
	if (gMath.DistanceSquared(thePoint,theTriangle1)<theEpsilon || gMath.DistanceSquared(thePoint,theTriangle2)<theEpsilon || gMath.DistanceSquared(thePoint,theTriangle3)<theEpsilon) return true;
	if (gMath.DistanceToLineSquared(thePoint,theTriangle1,theTriangle2)<theEpsilon) return true;
	if (gMath.DistanceToLineSquared(thePoint,theTriangle1,theTriangle3)<theEpsilon) return true;
	if (gMath.DistanceToLineSquared(thePoint,theTriangle2,theTriangle3)<theEpsilon) return true;

	return IsPointInTriangle(thePoint,theTriangle1,theTriangle2,theTriangle3);
}


bool Math::IsPointInTriangle(Point thePoint, Point theTriangle1, Point theTriangle2, Point theTriangle3)
{

	bool aTest1, aTest2, aTest3;

	aTest1 = GeoSign(&thePoint, &theTriangle1, &theTriangle2) < 0.0f;
	aTest2 = GeoSign(&thePoint, &theTriangle2, &theTriangle3) < 0.0f;
	aTest3 = GeoSign(&thePoint, &theTriangle3, &theTriangle1) < 0.0f;

	return ((aTest1 == aTest2) && (aTest2 == aTest3));
}

bool Math::IsPointInTriangle(Point thePoint, Vector *theTriangle1, Vector *theTriangle2, Vector *theTriangle3)
{
	bool aTest1, aTest2, aTest3;

	aTest1 = GeoSign2D(&thePoint, theTriangle1, theTriangle2) < 0.0f;
	aTest2 = GeoSign2D(&thePoint, theTriangle2, theTriangle3) < 0.0f;
	aTest3 = GeoSign2D(&thePoint, theTriangle3, theTriangle1) < 0.0f;

	return ((aTest1 == aTest2) && (aTest2 == aTest3));
}



bool Math::IsPointInTriangle(Point* thePoint, Point* theTriangle1, Point* theTriangle2, Point* theTriangle3)
{
	bool aTest1, aTest2, aTest3;

	aTest1 = GeoSign(thePoint, theTriangle1, theTriangle2) < 0.0f;
	aTest2 = GeoSign(thePoint, theTriangle2, theTriangle3) < 0.0f;
	aTest3 = GeoSign(thePoint, theTriangle3, theTriangle1) < 0.0f;

	return ((aTest1 == aTest2) && (aTest2 == aTest3));
}


bool Math::DoesLineIntersectTriangle(Point theLine1Start, Point theLine1End, Point aT1, Point aT2, Point aT3)
{
	if (IsPointInTriangle(&theLine1Start,&aT1,&aT2,&aT3)) return true;
	if (IsPointInTriangle(&theLine1End,&aT1,&aT2,&aT3)) return true;

	if (DoesLineIntersectLine(theLine1Start,theLine1End,aT1,aT2)) return true;
	if (DoesLineIntersectLine(theLine1Start,theLine1End,aT2,aT3)) return true;
	if (DoesLineIntersectLine(theLine1Start,theLine1End,aT1,aT3)) return true;

	return false;
}

bool Math::DoesLineIntersectQuad(Point theLine1Start, Point theLine1End, Quad theQuad)
{
	if (IsPointInQuad(theLine1Start,theQuad)) return true;
	if (IsPointInQuad(theLine1End,theQuad)) return true;

	if (DoesLineIntersectTriangle(theLine1Start,theLine1End,theQuad.UpperLeft(),theQuad.UpperRight(),theQuad.LowerLeft())) return true;
	if (DoesLineIntersectTriangle(theLine1Start,theLine1End,theQuad.UpperRight(),theQuad.LowerLeft(),theQuad.LowerRight())) return true;

	return false;
}

bool Math::RectangleIntersectQuad(Rect theRect, Quad& theQuad)
{
	if (!theRect.Intersects(theQuad.GetBounds())) return false;

	if (theRect.ContainsPoint(theQuad.mCorner[0])) return true;

	if (IsPointInTriangle(theRect.UpperLeft(),theQuad.mCorner[0],theQuad.mCorner[1],theQuad.mCorner[2])) return true;
	if (IsPointInTriangle(theRect.UpperLeft(),theQuad.mCorner[1],theQuad.mCorner[2],theQuad.mCorner[3])) return true;

	if (DoesLineIntersectRectangle(theRect,theQuad.mCorner[0],theQuad.mCorner[1])) return true;
	if (DoesLineIntersectRectangle(theRect,theQuad.mCorner[1],theQuad.mCorner[3])) return true;
	if (DoesLineIntersectRectangle(theRect,theQuad.mCorner[3],theQuad.mCorner[2])) return true;
	if (DoesLineIntersectRectangle(theRect,theQuad.mCorner[2],theQuad.mCorner[0])) return true;

	return false;
}

Point Math::RandomPointInRect(Rect theRect,Random& theRandom)
{
	return Point(theRect.mX+theRandom.GetF(theRect.mWidth),theRect.mY+theRandom.GetF(theRect.mHeight));
}


bool Math::EllipseIntersectQuad(Point theCenter, Point theRadius, Quad& theQuad)
{
	Rect aEllipseBound=Rect(theRadius.mX*2,theRadius.mY*2);
	aEllipseBound.CenterAt(theCenter);

	if (!aEllipseBound.Intersects(theQuad.GetBounds())) return false;

	if (IsPointInTriangle(theCenter,theQuad.mCorner[0],theQuad.mCorner[1],theQuad.mCorner[2])) return true;
	if (IsPointInTriangle(theCenter,theQuad.mCorner[1],theQuad.mCorner[2],theQuad.mCorner[3])) return true;

	for (int aCount=0;aCount<4;aCount++) if (IsPointInEllipse(theCenter,theRadius,theQuad.mCorner[aCount])) return true;

	if (EllipseIntersectLine(theCenter,theRadius,theQuad.mCorner[0],theQuad.mCorner[1])) return true;
	if (EllipseIntersectLine(theCenter,theRadius,theQuad.mCorner[1],theQuad.mCorner[3])) return true;
	if (EllipseIntersectLine(theCenter,theRadius,theQuad.mCorner[3],theQuad.mCorner[2])) return true;
	if (EllipseIntersectLine(theCenter,theRadius,theQuad.mCorner[2],theQuad.mCorner[0])) return true;

	return false;
}

Vector Math::GetFaceNormal(Vector v1, Vector v2, Vector v3)
{
	Vector aResult;
	Vector aVec1=v2-v1;
	Vector aVec2=v3-v1;
	aResult=-aVec1.Cross(aVec2);
	aResult.Normalize();

	return aResult;
}

Vector Math::GetFaceNormalUnNormalized(Vector v1, Vector v2, Vector v3)
{
	Vector aResult;
	Vector aVec1=v2-v1;
	Vector aVec2=v3-v1;
	aResult=-aVec1.Cross(aVec2);
	return aResult;
}


Vector Math::InterpolateTriangle(Vector thePoint, Vector theV1, Vector theV2, Vector theV3, Vector theInterpolate1,Vector theInterpolate2,Vector theInterpolate3)
{
	Vector aF1=theV1-thePoint;
	Vector aF2=theV2-thePoint;
	Vector aF3=theV3-thePoint;

	Vector aP1MinusP2=theV1-theV2;
	Vector aP1MinusP3=theV1-theV3;
	float aA=aP1MinusP2.Cross(aP1MinusP3).Length();
	float aA1=aF2.Cross(aF3).Length()/aA;
	float aA2=aF3.Cross(aF1).Length()/aA;
	float aA3=aF1.Cross(aF2).Length()/aA;
	return theInterpolate1*aA1+theInterpolate2*aA2+theInterpolate3*aA3;
}

/*
#define MAGICTRIANGLE(pos, a,b,c, tointerpolate, result) \
	Vector aF1=gCheckQuad[a]-pos;\
	Vector aF2=gCheckQuad[b]-pos;\
	Vector aF3=gCheckQuad[c]-pos;\
	Vector aP1MinusP2=gCheckQuad[a]-gCheckQuad[b];\
	Vector aP1MinusP3=gCheckQuad[a]-gCheckQuad[c];\
	float aA=aP1MinusP2.Cross(aP1MinusP3).Length();\
	float aA1=aF2.Cross(aF3).Length()/aA;\
	float aA2=aF3.Cross(aF1).Length()/aA;\
	float aA3=aF1.Cross(aF2).Length()/aA;\
	result=tointerpolate[a]*aA1+tointerpolate[b]*aA2+tointerpolate[c]*aA3;
*/

Vector Math::Cross(Vector theVA, Vector theVB)
{
	Vector aResult;
	aResult.mX=(theVA.mY*theVB.mZ)-(theVB.mY*theVA.mZ);
	aResult.mY=-(theVA.mX*theVB.mZ)+(theVB.mX*theVA.mZ);
	aResult.mZ=(theVA.mX*theVB.mY)-(theVA.mY*theVB.mX);
	return aResult;
}

float Math::Dot(Vector theVA, Vector theVB)
{
	return (theVA.mX * theVB.mX + theVA.mY * theVB.mY + theVA.mZ * theVB.mZ);
}

float Math::IntersectRayPlane(Vector theRayOrigin, Vector theRayDir, Vector thePlanePos, Vector thePlaneNormal)
{
	float aD=-Dot(thePlaneNormal,thePlanePos);

	float aNumer = Dot(thePlaneNormal,theRayOrigin)+aD;
	float aDenom = Dot(thePlaneNormal,theRayDir);

	if (aDenom==0) return -1.0f;
	return -(aNumer/aDenom);	
}

bool Math::IntersectRayPlane(Vector theRayOrigin, Vector theRayDir, Vector thePlanePos, Vector thePlaneNormal, float &result)
{
	float aD=-Dot(thePlaneNormal,thePlanePos);

	float aNumer = Dot(thePlaneNormal,theRayOrigin)+aD;
	float aDenom = Dot(thePlaneNormal,theRayDir);

	if (aDenom==0) return false;
	result = -(aNumer/aDenom);
	return true;
}


char Math::ClassifyPoint(Vector thePoint, Vector thePlanePos, Vector thePlaneNormal)
{
	Vector aDir=thePlanePos-thePoint;
	float aD=Dot(aDir,thePlaneNormal);

	// Set up for CullCCW
	if (aD<-0.0005f) return 1;
	if (aD>0.0005f) return -1;
	return 0;
}

/*
bool Math::IsPointInTriangle(Vector thePos, Vector theT1, Vector theT2, Vector theT3)
{
	theT1-=thePos;
	theT2-=thePos;
	theT3-=thePos;

	Vector aU=theT2.Cross(theT3);
	Vector aV=theT3.Cross(theT1);
	Vector aW=theT1.Cross(theT2);

	if (aU.Dot(aV)<0.0f) return false;
	if (aU.Dot(aW)<0.0f) return false;
	return true;
}
/**/


//typedef unsigned int uint32;
#define in(a) ((unsigned int&) a)
//*
bool Math::IsPointInTriangle(Vector thePos, Vector theT1, Vector theT2, Vector theT3)

//bool IsPointInTriangle(const VECTOR& point,
//	const VECTOR& pa,const VECTOR& pb, const VECTOR& pc)
{
	Vector e10=theT2-theT1;
	Vector e20=theT3-theT1;
	float a = e10.Dot(e10);
	float b = e10.Dot(e20);
	float c = e20.Dot(e20);
	float ac_bb=(a*c)-(b*b);
	Vector vp(thePos.mX-theT1.mX, thePos.mY-theT1.mY, thePos.mZ-theT1.mZ);
	float d = vp.Dot(e10);
	float e = vp.Dot(e20);
	float x = (d*c)-(e*b);
	float y = (e*a)-(d*b);
	float z = x+y-ac_bb;
	return (( in(z)& ~(in(x)|in(y)) ) & 0x80000000)!=0;
}
/**/

//*
bool Math::IsPointInTriangle(DVector thePos, DVector theT1, DVector theT2, DVector theT3)

//bool IsPointInTriangle(const VECTOR& point,
//	const VECTOR& pa,const VECTOR& pb, const VECTOR& pc)
{
	DVector e10=theT2-theT1;
	DVector e20=theT3-theT1;
	double a = e10.Dot(e10);
	double b = e10.Dot(e20);
	double c = e20.Dot(e20);
	double ac_bb=(a*c)-(b*b);
	DVector vp(thePos.mX-theT1.mX, thePos.mY-theT1.mY, thePos.mZ-theT1.mZ);
	double d = vp.Dot(e10);
	double e = vp.Dot(e20);
	double x = (d*c)-(e*b);
	double y = (e*a)-(d*b);
	double z = x+y-ac_bb;
	return (( in(z)& ~(in(x)|in(y)) ) & 0x80000000)!=0;
}

#undef in
/**/

/*
bool Math::IsPointInTriangle(Vector thePos, Vector theT1, Vector theT2, Vector theT3)
{
	float aTotalAngles=0.0f;

	Vector aV1 = thePos-theT1;
	Vector aV2 = thePos-theT2;
	Vector aV3 = thePos-theT3;

	aV1.Normalize();
	aV2.Normalize();
	aV3.Normalize();

	aTotalAngles += acos(Dot(aV1,aV2));   
	aTotalAngles += acos(Dot(aV2,aV3));
	aTotalAngles += acos(Dot(aV3,aV1)); 

	if (fabs(aTotalAngles-2*gMath.mPI)<=0.005) return true;
	return false;
}
/**/

//D3DVECTOR closestPointOnLine(D3DVECTOR& a, D3DVECTOR& b, D3DVECTOR& p) {

/*
Vector Math::GetClosestPointOnLine(Vector thePos, Vector theL1, Vector theL2)
{
	//
	// Do NOT try to optimize this like the 2D version... when I did it screwed up all the Hamsterball physics!
	//
	Vector aC=thePos-theL1;
	Vector aV=theL2-theL1; 

	float aD=aV.Length();
	//aV.Normalize();
	aV/=aD;
	float aT=Dot(aV,aC);

	if (aT<0.0f) return (theL1);
	if (aT>aD) return (theL2);


	// Return the point between ‘a’ and ‘b’
	//set length of V to t. V is normalized so this is easy
	aV*=aT;
	return (theL1+aV);	
}
*/

//*
Vector Math::GetClosestPointOnLine(Vector thePos, Vector theL1, Vector theL2)
{
	Vector aC=thePos-theL1;
	Vector aV=theL2-theL1; 

	float aD = aV.LengthSquared();
	float aT = Dot(aV,aC);

	if (aT<0.0f) return (theL1);
	if (aT>aD) return (theL2);

	aV*=(aT/aD);
	return (theL1+aV);  
}
/**/


DVector Math::GetClosestPointOnLine(DVector thePos, DVector theL1, DVector theL2)
{
	//
	// Do NOT try to optimize this like the 2D version... when I did it screwed up all the Hamsterball physics!
	//
	DVector aC=thePos-theL1;
	DVector aV=theL2-theL1; 

	double aD=aV.Length();
	aV.Normalize();
	double aT=aV.Dot(aC);
	//double aT=Dot(aV,aC);

	if (aT<0.0) return (theL1);
	if (aT>aD) return (theL2);


	// Return the point between ‘a’ and ‘b’
	//set length of V to t. V is normalized so this is easy
	aV*=aT;
	return (theL1+aV);	
}

bool Math::DoesLineIntersectSphere(Vector theLine1Start, Vector theLine1End, Vector theCircleCenter, float theCircleRadius)
{
	Vector aC=theCircleCenter-theLine1Start;
	Vector aV=theLine1End-theLine1Start; 
	float aD=aV.LengthSquared();
	aV.Normalize();
	float aT=Dot(aV,aC);

	Vector aClosest;
	if (aT<0.0f) aClosest=theLine1Start;
	else if (aT*aT>aD) aClosest=theLine1End;
	else {aV*=aT;aClosest=theLine1Start+aV;}

	return (DistanceSquared(theCircleCenter,aClosest)<=theCircleRadius*theCircleRadius);
}


Vector Math::ClosestPointOnInfiniteLine(Vector thePos, Vector theL1, Vector theL2)
{
	Vector aC=thePos-theL1;
	Vector aV=theL2-theL1; 

	//float aD=aV.Length();
	aV.Normalize();
	float aT=Dot(aV,aC);

	//if (aT<0.0f) return (theL1);
	//if (aT>aD) return (theL2);
	// Return the point between ‘a’ and ‘b’
	//set length of V to t. V is normalized so this is easy
	aV*=aT;
	return (theL1+aV);	
}

Vector Math::ClosestPointToAxis(Vector thePos, Vector theAxisPos, Vector theAxisDir)
{
	Vector aC=thePos-theAxisPos;
	float aT=Dot(theAxisDir,aC);
	theAxisDir*=aT;
	return (theAxisPos+theAxisDir);
}


float Math::GetClosestPointOnLinePercent(Vector thePos, Vector theL1, Vector theL2)
{
	Vector aC=thePos-theL1;
	Vector aV=theL2-theL1; 

	float aD=aV.Length();
	aV.Normalize();
	float aT=Dot(aV,aC);

	if (aT<0.0f) aT=0;
	if (aT>aD) aT=aD;

	return aT/aD;
}

float Math::GetClosestPointOnInfiniteLinePercent(Vector thePos, Vector theL1, Vector theL2)
{
	Vector aC=thePos-theL1;
	Vector aV=theL2-theL1; 

	float aD=aV.Length();
	if (aD==0) return 0.0f; // Line has no length, so just return us at 0.0f
	aV.Normalize();
	float aT=Dot(aV,aC);

	return aT/aD;
}

double Math::GetClosestPointOnInfiniteLinePercent(DVector thePos, DVector theL1, DVector theL2)
{
	DVector aC=thePos-theL1;
	DVector aV=theL2-theL1; 

	double aD=aV.Length();
	if (aD==0) return 0.0f; // Line has no length, so just return us at 0.0f
	aV.Normalize();
	double aT=aV.Dot(aC);

	return aT/aD;
}

double Math::GetClosestPointOnInfiniteLinePercentD(Vector thePos, Vector theL1, Vector theL2)
{
	DVector aC=thePos-theL1;
	DVector aV=theL2-theL1; 

	double aD=aV.Length();
	if (aD==0) return 0.0f; // Line has no length, so just return us at 0.0f
	aV.Normalize();
	double aT=aV.Dot(aC);//Dot(aV,aC);
	return aT/aD;
}

float Math::GetClosestPointOnLinePercent(Point thePos, Point theL1, Point theL2)
{
	Point aC=thePos-theL1;
	Point aV=theL2-theL1; 

	float aD=aV.Length();
	aV.Normalize();
	float aT=Dot(aV,aC);

	if (aT<0.0f) aT=0;
	if (aT>aD) aT=aD;

	return aT/aD;
}


Vector Math::GetClosestPointOnTriangle(Vector thePos, Vector theT1, Vector theT2, Vector theT3)
{
	if (IsPointInTriangle(thePos,theT1,theT2,theT3)) return thePos;

	Vector aP1=GetClosestPointOnLine(thePos,theT1,theT2);
	Vector aP2=GetClosestPointOnLine(thePos,theT2,theT3);
	Vector aP3=GetClosestPointOnLine(thePos,theT3,theT1);

	float aD1=(thePos-aP1).LengthSquared();
	float aD2=(thePos-aP2).LengthSquared();
	float aD3=(thePos-aP3).LengthSquared();

	float aMin=aD1;
	Vector aResult=aP1;
	if (aD2<aMin) 
	{
		aMin=aD2;
		aResult=aP2;
	}
	if (aD3<aMin) aResult=aP3;
	return aResult;
}

Point Math::GetClosestPointOnTriangle(Point thePos, Point theT1, Point theT2, Point theT3)
{
	if (IsPointInTriangle(thePos,theT1,theT2,theT3)) return thePos;

	Point aP1=GetClosestPointOnLine(thePos,theT1,theT2);
	Point aP2=GetClosestPointOnLine(thePos,theT2,theT3);
	Point aP3=GetClosestPointOnLine(thePos,theT3,theT1);

	float aD1=(thePos-aP1).LengthSquared();
	float aD2=(thePos-aP2).LengthSquared();
	float aD3=(thePos-aP3).LengthSquared();

	float aMin=aD1;
	Point aResult=aP1;
	if (aD2<aMin) 
	{
		aMin=aD2;
		aResult=aP2;
	}
	if (aD3<aMin) aResult=aP3;
	return aResult;
}


bool Math::IsPointInSphere(Vector thePoint, Vector theSphereCenter, float theRadius)
{
	float aLen=(thePoint-theSphereCenter).LengthSquared();
	if (aLen<theRadius*theRadius) return true;
	return false;
}

//double intersectRaySphere(D3DVECTOR rO, D3DVECTOR rV, D3DVECTOR sO, double sR) {
float Math::IntersectRaySphere(Vector theRayOrigin, Vector theRayDir, Vector theSphereCenter, float theRadius)
{
	Vector aQ=theSphereCenter-theRayOrigin;

	float aC=aQ.Length();
	float aV=Dot(aQ,theRayDir);
	float aD=theRadius*theRadius-(aC*aC-aV*aV);
	if (aD<0.0) return (-1.0f);
	return (aV-(float)sqrt(aD));
}

/*
bool Math::DoesLineIntersectPlane(Vector& theLine1, Vector& theLine2, Plane& thePlane, Vector* theIntersectPos)
{
	float aU=((thePlane.mX*theLine1.mX) + (thePlane.mY*theLine1.mY)+(thePlane.mZ*theLine1.mZ)+thePlane.mD)/(thePlane.mX * (theLine1.mX - theLine2.mX)) + (thePlane.mY * (theLine1.mY - theLine2.mY)) + (thePlane.mZ * (theLine1.mZ - theLine2.mY));
	if (aU<=0.0f || aU>1.0f) return false;

	if (theIntersectPos)
	{
		theIntersectPos->mX=aU*theLine1.mX+(1-aU)*theLine2.mX;
		theIntersectPos->mY=aU*theLine1.mY+(1-aU)*theLine2.mY;
		theIntersectPos->mZ=aU*theLine1.mZ+(1-aU)*theLine2.mZ;
	}
	return true;
}
*/

#define SMALL_NUM   0.00000001 // anything that avoids division overflow
bool Math::DoesLineIntersectPlane(Vector theLine1, Vector theLine2, Plane thePlane, Vector* theIntersectPos)
{
	Vector aU=theLine2-theLine1;
	Vector aW=theLine1-thePlane.mPos;

	float aD=Dot(thePlane.mNormal,aU);
	float aN=-Dot(thePlane.mNormal,aW);

	if (fabs(aD)<SMALL_NUM) 
	{           // segment is parallel to plane
		if (aN==0) 
		{
			if (theIntersectPos) *theIntersectPos=thePlane.mPos;
			return true;
		}
		return false;
	}

	float aSI = aN/aD;
	if (aSI<0 || aSI>1) return false;
	if (theIntersectPos) *theIntersectPos=(theLine1+aSI*aU);
	//*theIntersectPos-=thePlane.mPos;
	return true;
}

bool Math::DoesRayIntersectPlane(Vector theLine1, Vector theLine2, Plane thePlane, Vector* theIntersectPos)
{
	Vector aU=theLine2-theLine1;
	Vector aW=theLine1-thePlane.mPos;

	float aD=Dot(thePlane.mNormal,aU);
	float aN=-Dot(thePlane.mNormal,aW);

	if (fabs(aD)<SMALL_NUM) 
	{           // segment is parallel to plane
		if (aN==0) 
		{
			if (theIntersectPos) *theIntersectPos=thePlane.mPos;
			return true;
		}
		return false;
	}

	float aSI = aN/aD;
	if (aSI<0) return false;
	if (theIntersectPos) *theIntersectPos=(theLine1+aSI*aU);
	//*theIntersectPos-=thePlane.mPos;
	return true;
}


bool Math::DoesLineIntersectPlane(DVector theLine1, DVector theLine2, Plane thePlane, DVector* theIntersectPos)
{
	DVector aU=theLine2-theLine1;
	DVector aW=theLine1-thePlane.mPos;
	DVector aPNormal=thePlane.mNormal;

	double aD=aPNormal.Dot(aU);
	double aN=-aPNormal.Dot(aW);

	if (fabs(aD)<SMALL_NUM) 
	{           // segment is parallel to plane
		if (aN==0) 
		{
			if (theIntersectPos) *theIntersectPos=thePlane.mPos;
			return true;
		}
		return false;
	}

	double aSI = aN/aD;
	if (aSI<0 || aSI>1) return false;
	if (theIntersectPos) *theIntersectPos=(theLine1+aSI*aU);
	//*theIntersectPos-=thePlane.mPos;
	return true;
}

bool Math::DoesInfiniteLineIntersectPlane(Vector theLine1, Vector theLine2, Plane thePlane, Vector* theIntersectPos)
{
	Vector aU=theLine2-theLine1;
	Vector aW=theLine1-thePlane.mPos;

	float aD=Dot(thePlane.mNormal,aU);
	float aN=-Dot(thePlane.mNormal,aW);

	if (fabs(aD)<SMALL_NUM) 
	{           // segment is parallel to plane
		if (aN==0) 
		{
			if (theIntersectPos) *theIntersectPos=thePlane.mPos;
			return true;
		}
		return false;
	}

	float aSI = aN/aD;
	if (theIntersectPos) *theIntersectPos=(theLine1+aSI*aU);
	return true;
}


/*
// intersect3D_SegmentPlane(): find the 3D intersection of a segment and a plane
//    Input:  S = a segment, and Pn = a plane = {Point V0;  Vector n;}
//    Output: *I0 = the intersect point (when it exists)
//    Return: 0 = disjoint (no intersection)
//            1 =  intersection in the unique point *I0
//            2 = the  segment lies in the plane
int
	intersect3D_SegmentPlane( Segment S, Plane Pn, Point* I )
{
	Vector    u = S.P1 - S.P0;
	Vector    w = S.P0 - Pn.V0;

	float     D = dot(Pn.n, u);
	float     N = -dot(Pn.n, w);

	if (fabs(D) < SMALL_NUM) {           // segment is parallel to plane
		if (N == 0)                      // segment lies in plane
			return 2;
		else
			return 0;                    // no intersection
	}
	// they are not parallel
	// compute intersect param
	float sI = N / D;
	if (sI < 0 || sI > 1)
		return 0;                        // no intersection

	*I = S.P0 + sI * u;                  // compute segment intersect point
	return 1;
}
*/

Point Math::GetClosestPointOnLine(Point thePos, Point theL1, Point theL2)
{
	Vector aC=thePos-theL1;
	Vector aV=theL2-theL1; 

	float aD=aV.Length();
	aV.Normalize();
	float aT=Dot(aV,aC);

	if (aT<0.0f) return (theL1);
	if (aT>aD) return (theL2);


	// Return the point between ‘a’ and ‘b’
	//set length of V to t. V is normalized so this is easy
	aV*=aT;
	return (theL1+aV);	
}

float Math::DistanceToLine(Point thePos, Point theL1, Point theL2)
{
	#define NORM(v)     sqrt(gMath.Dot(v,v))     // norm = length of  vector
	#define NORMD(u,v)      NORM(u-v)          // distance = norm of difference	rn aArea/theL2.Normal();

	Point aV=theL2-theL1;
	Point aW=thePos-theL1;

	float aC1=Dot(aW,aV);
	if (aC1<=0) return (float)NORMD(thePos,theL1);

	float aC2=Dot(aV,aV);
	if (aC2<=aC1) return (float)NORMD(thePos,theL2);

	float aB=aC1/aC2;
	Point aPb=theL1+aB*aV;
	return (float)NORMD(thePos,aPb);
}

float Math3D::DistanceToLine(Vector thePos, Vector theL1, Vector theL2)
{
	Vector aV=theL2-theL1;
	Vector aW=thePos-theL1;

	float aC1=gMath.Dot(aW,aV);
	if (aC1<=0) return (float)NORMD(thePos,theL1);

	float aC2=gMath.Dot(aV,aV);
	if (aC2<=aC1) return (float)NORMD(thePos,theL2);

	float aB=aC1/aC2;
	Vector aPb=theL1+aB*aV;
	return (float)NORMD(thePos,aPb);
}

#undef NORM
#define NORM(v)     (gMath.Dot(v,v))     // norm = length of  vector
float Math3D::DistanceToInfiniteLineSquared(Vector thePos, Vector theL1, Vector theL2)
{
	/*
	Vector aPos=thePos-theL1;  
	Vector aDir=theL2-theL1;
	float aDL=aDir.GetNormal().Dot(aPos);
	return sqrt(gMath.Dot2(aPos)-aDL*aDL);
	*/
	Vector aV=theL2-theL1;
	Vector aW=thePos-theL1;

	float aC1=gMath.Dot(aW,aV);
	//if (aC1<=0) return (float)NORMD(thePos,theL1);

	float aC2=gMath.Dot(aV,aV);
	//if (aC2<=aC1) return (float)NORMD(thePos,theL2);

	float aB=aC1/aC2;
	Vector aPb=theL1+aB*aV;
	return (float)NORMD(thePos,aPb);
}


float Math::DistanceToLineSquared(Point thePos, Point theL1, Point theL2)
{
	Point aV=theL2-theL1;
	Point aW=thePos-theL1;

	float aC1=Dot(aW,aV);if (aC1<=0) return NORMD(thePos,theL1);
	float aC2=Dot(aV,aV);if (aC2<=aC1) return NORMD(thePos,theL2);

	float aB=aC1/aC2;
	Point aPb=theL1+aB*aV;
	return NORMD(thePos,aPb);
}

float Math3D::DistanceToLineSquared(Vector thePos, Vector theL1, Vector theL2)
{
	Vector aV=theL2-theL1;
	Vector aW=thePos-theL1;

	float aC1=gMath.Dot(aW,aV);if (aC1<=0) return NORMD(thePos,theL1);
	float aC2=gMath.Dot(aV,aV);if (aC2<=aC1) return NORMD(thePos,theL2);

	float aB=aC1/aC2;
	Vector aPb=theL1+aB*aV;
	return NORMD(thePos,aPb);
}

int Math::Wrap(int theValue, int theMin, int theMax)
{
	int aSpan=theMax-theMin;
	while (theValue<theMin) theValue+=aSpan;
	while (theValue>theMax) theValue-=aSpan;
	return theValue;
}

float Math::Wrap(float theValue, float theMin, float theMax)
{
	float aSpan=theMax-theMin;
	while (theValue<theMin) theValue+=aSpan;
	while (theValue>theMax) theValue-=aSpan;
	return theValue;
}


bool Math::GetLowestRoot(float theA, float theB, float theC, float maxR, float* theRoot) 
{
	float aDeterminant=theB*theB-4.0f*theA*theC;
	if (aDeterminant<0.0f) return false;

	float aSqrtD=(float)sqrt(aDeterminant);
	float aR1=(-theB-aSqrtD)/(2*theA);
	float aR2=(-theB+aSqrtD)/(2*theA);
	if (aR1>aR2) 
	{
		float aTemp=aR2;
		aR2=aR1;
		aR1=aTemp;
	}
	if (aR1>0 && aR1<maxR) {*theRoot=aR1;return true;}
	if (aR2>0 && aR2<maxR) {*theRoot=aR2;return true;}
	return false;
}

bool Math::CircleIntersectTriangle(Point theCenter, float theRadius, Point theT1, Point theT2, Point theT3)
{

	if (IsPointInTriangle(theCenter,theT1,theT2,theT3)) return true;
	if (CircleIntersectLine(theCenter,theRadius,theT1,theT2)) return true;
	if (CircleIntersectLine(theCenter,theRadius,theT2,theT3)) return true;
	if (CircleIntersectLine(theCenter,theRadius,theT3,theT1)) return true;
	return false;

}

inline bool GetIntersection(float fDst1, float fDst2, Vector P1, Vector P2, Vector& Hit)
{
	if ((fDst1 * fDst2) >= 0.0f) return false;
	if (fDst1 == fDst2) return false;
	Hit = P1 + (P2 - P1) * (-fDst1 / (fDst2 - fDst1));
	return true;
}

inline bool InBox(Vector Hit, Vector B1, Vector B2, int Axis)
{
	return ((Axis == 1 && Hit.mZ > B1.mZ && Hit.mZ < B2.mZ && Hit.mY > B1.mY && Hit.mY < B2.mY) ||
		(Axis == 2 && Hit.mZ > B1.mZ && Hit.mZ < B2.mZ && Hit.mX > B1.mX && Hit.mX < B2.mX) ||
		(Axis == 3 && Hit.mX > B1.mX && Hit.mX < B2.mX && Hit.mY > B1.mY && Hit.mY < B2.mY));
}

bool Math3D::DoesLineIntersectCube(Vector aLine1, Vector aLine2, Vector aBox1, Vector aBox2, Vector* theIntersection)
{
	Vector aHit;

	if (aLine2.mX < aBox1.mX && aLine1.mX < aBox1.mX) return false;
	if (aLine2.mX > aBox2.mX && aLine1.mX > aBox2.mX) return false;
	if (aLine2.mY < aBox1.mY && aLine1.mY < aBox1.mY) return false;
	if (aLine2.mY > aBox2.mY && aLine1.mY > aBox2.mY) return false;
	if (aLine2.mZ < aBox1.mZ && aLine1.mZ < aBox1.mZ) return false;
	if (aLine2.mZ > aBox2.mZ && aLine1.mZ > aBox2.mZ) return false;
	if (aLine1.mX > aBox1.mX && aLine1.mX < aBox2.mX &&
		aLine1.mY > aBox1.mY && aLine1.mY < aBox2.mY &&
		aLine1.mZ > aBox1.mZ && aLine1.mZ < aBox2.mZ)
	{
		if (theIntersection) *theIntersection=aLine1;
		return true;
	}
	if ((GetIntersection(aLine1.mX - aBox1.mX, aLine2.mX - aBox1.mX, aLine1, aLine2,aHit) && InBox(aHit, aBox1, aBox2, 1))
		|| (GetIntersection(aLine1.mY - aBox1.mY, aLine2.mY - aBox1.mY, aLine1, aLine2,aHit) && InBox(aHit, aBox1, aBox2, 2))
		|| (GetIntersection(aLine1.mZ - aBox1.mZ, aLine2.mZ - aBox1.mZ, aLine1, aLine2,aHit) && InBox(aHit, aBox1, aBox2, 3))
		|| (GetIntersection(aLine1.mX - aBox2.mX, aLine2.mX - aBox2.mX, aLine1, aLine2,aHit) && InBox(aHit, aBox1, aBox2, 1))
		|| (GetIntersection(aLine1.mY - aBox2.mY, aLine2.mY - aBox2.mY, aLine1, aLine2,aHit) && InBox(aHit, aBox1, aBox2, 2))
		|| (GetIntersection(aLine1.mZ - aBox2.mZ, aLine2.mZ - aBox2.mZ, aLine1, aLine2,aHit) && InBox(aHit, aBox1, aBox2, 3)))
	{
		if (theIntersection) *theIntersection=aHit;
		return true;
	}

	return false;
}

bool Math3D::DoesLineIntersectTriangle(Vector theLine1, Vector theLine2, Vector theT1, Vector theT2, Vector theT3, Vector* theIntersection)
{
	Plane aPlane(theT1,theT2,theT3);
	Vector aIntersection;
	if (gMath.DoesLineIntersectPlane(theLine1,theLine2,aPlane,&aIntersection))
	{
		bool aResult=gMath.IsPointInTriangle(aIntersection,theT1,theT2,theT3);
		if (aResult)
		{
			if (theIntersection) *theIntersection=aIntersection;
			return true;
		}
	}
	return false;
}

bool Math3D::DoesLineIntersectTriangle(DVector theLine1, DVector theLine2, DVector theT1, DVector theT2, DVector theT3, DVector* theIntersection)
{
	Plane aPlane(theT1,theT2,theT3);
	DVector aIntersection;
	if (gMath.DoesLineIntersectPlane(theLine1,theLine2,aPlane,&aIntersection))
	{
		bool aResult=gMath.IsPointInTriangle(aIntersection,theT1,theT2,theT3);
		if (aResult)
		{
			if (theIntersection) *theIntersection=aIntersection;
			return true;
		}
	}
	return false;
}


Vector Math3D::ClosestPointOnTriangle(Vector sourcePos, Vector theT0, Vector theT1, Vector theT2)
{
	Plane aP;aP.Create(theT0,theT1,theT2);
	Vector aTest;
	Vector aResult=aP.ClosestPoint(sourcePos);
	if (gMath3D.IsPointInTriangleVolume(aResult,theT0,theT1,theT2)) return aResult;

	aResult=gMath.GetClosestPointOnLine(sourcePos,theT0,theT1);
	float aBestDist=gMath.DistanceSquared(sourcePos,aResult);

	aTest=gMath.GetClosestPointOnLine(sourcePos,theT1,theT2);
	float aDist=gMath.DistanceSquared(sourcePos,aTest);
	if (aDist<aBestDist) {aBestDist=aDist;aResult=aTest;}

	aTest=gMath.GetClosestPointOnLine(sourcePos,theT2,theT0);
	aDist=gMath.DistanceSquared(sourcePos,aTest);
	if (aDist<aBestDist) {aBestDist=aDist;aResult=aTest;}

	return aResult;
}

DVector Math3D::ClosestPointOnTriangle(DVector sourcePos, DVector theT0, DVector theT1, DVector theT2)
{
	DPlane aP;aP.Create(theT0,theT1,theT2);
	DVector aTest;
	DVector aResult=aP.ClosestPoint(sourcePos);
	if (gMath3D.IsPointInTriangleVolume(aResult,theT0,theT1,theT2)) return aResult;

	aResult=gMath.GetClosestPointOnLine(sourcePos,theT0,theT1);
	double aBestDist=gMath.DistanceSquared(sourcePos,aResult);

	aTest=gMath.GetClosestPointOnLine(sourcePos,theT1,theT2);
	double aDist=gMath.DistanceSquared(sourcePos,aTest);
	if (aDist<aBestDist) {aBestDist=aDist;aResult=aTest;}

	aTest=gMath.GetClosestPointOnLine(sourcePos,theT2,theT0);
	aDist=gMath.DistanceSquared(sourcePos,aTest);
	if (aDist<aBestDist) {aBestDist=aDist;aResult=aTest;}

	return aResult;
}

/*
Vector Math3D::ClosestPointOnTriangle(Vector sourcePos, Vector theT0, Vector theT1, Vector theT2)
{
	Vector aEdge0=theT1-theT0;
	Vector aEdge1=theT2-theT0;
	Vector aV0=theT0-sourcePos;

	float aA=aEdge0.Dot(aEdge0);
	float aB=aEdge0.Dot(aEdge1);
	float aC=aEdge1.Dot(aEdge1);
	float aD=aEdge0.Dot(aV0);
	float aE=aEdge1.Dot(aV0);

	float aDet=aA*aC-aB*aB;
	float aS=aB*aE-aC*aD;
	float aT=aB*aD-aA*aE;

	if (aS+aT<aDet)
	{
		if (aS<0.0f)
		{
			if (aT<0.0f)
			{
				if (aD<0.0f)
				{
					aS=_clamp(0.0f,-aD/aA,1.0f);
					aT=0.0f;
				}
				else
				{
					aS=0.0f;
					aT=_clamp(0.0f,-aE/aC,1.0f);
				}
			}
			else
			{
				aS=0.0f;
				aT=_clamp(0.0f,-aE/aC,1.0f);
			}
		}
		else if (aT<0.0f)
		{
			aS=_clamp(0.0f,-aD/aA,1.0f);
			aT=0.0f;
		}
		else
		{
			float aInvDet=1.0f/aDet;
			aS*=aInvDet;
			aT*=aInvDet;
		}
	}
	else
	{
		if (aS<0.0f)
		{
			float aTmp0=aB+aD;
			float aTmp1=aC+aE;
			if (aTmp1>aTmp0)
			{
				float aNumer=aTmp1-aTmp0;
				float aDenom=aA-2*aB+aC;
				aS=_clamp(0.0f,aNumer/aDenom,1.0f);
				aT=1-aS;
			}
			else
			{
				aT=_clamp(0.0f,-aE/aC,1.0f);
				aS=0.0f;
			}
		}
		else if (aT<0.0f)
		{
			if (aA+aD>aB+aE)
			{
				float aNumer=aC+aE-aB-aD;
				float aDenom=aA-2*aB+aC;
				aS=_clamp(0.0f,aNumer/aDenom,1.0f);
				aT=1-aS;
			}
			else
			{
				aS=_clamp(0.0f,-aE/aC,1.0f);
				aT=0.0f;
			}
		}
		else
		{
			float aNumer=aC+aE-aB-aD;
			float aDenom=aA-2*aB+aC;
			aS=_clamp(0.0f,aNumer/aDenom,1.0f);
			aT=1.0f-aS;
		}
	}
	return theT0+aS*aEdge0+aT*aEdge1;
}
*/


namespace TinyEvaluator
{
	float Expression(String& theData);
	char Peek(String& theData){if (*theData.mCursor=='f' || *theData.mCursor==' ') theData.mCursor++;return *theData.mCursor;}
	char Get(String& theData) {if (*theData.mCursor=='f' || *theData.mCursor==' ') theData.mCursor++;return *theData.mCursor++;}
	bool IsKeyword(String& theData, char* theWord)
	{
		char* aPtrInString=theData.mCursor;
		char* aPtrInKeyword=theWord;
		for (;;)
		{
			if (*aPtrInKeyword==0) {for (size_t aCount=0;aCount<strlen(theWord);aCount++) Get(theData);return true;}
			if (*aPtrInString==0) return false;if (*aPtrInString!=*aPtrInKeyword) return false;
			aPtrInString++;aPtrInKeyword++;
		}
		return false;
	}
	String GetParameters(String &theData)
	{
		if (Peek(theData)!='(') return "";
		Get(theData); // Get parenthesis out of the queue

		String aResult;
		int aParenthesisStack=0;
		for (;;)
		{
			char aC=Get(theData);
			if (aC==0) {return aResult;}
			if (aC=='(') aParenthesisStack++;
			if (aC==')') if (--aParenthesisStack<0) {return aResult;}
			aResult+=aC;
		}
	}

	float Number(String& theData)
	{
		String aNumber;
		while ((Peek(theData)>='0' && Peek(theData)<='9') || Peek(theData)=='.') aNumber+=Get(theData);
		return aNumber.ToFloat();
	}
	float Factor(String& theData)
	{
		if (IsKeyword(theData,"SIN")) {String aStr=GetParameters(theData);return gMath.Sin(Expression(aStr));}
		// Insert additional keywords here if wanted.
		
		if ((Peek(theData)>='0' && Peek(theData)<='9') || Peek(theData)=='.') return Number(theData);
		else if (Peek(theData)=='(')
		{
			Get(theData); // '('
			float aResult=Expression(theData);
			Get(theData); // ')'
			return aResult;
		}
		else if (Peek(theData)=='-')
		{
			Get(theData);
			return -Expression(theData);
		}
		return 0; // error
	}
	float Term(String& theData)
	{
		float aResult=Factor(theData);
		while (Peek(theData)=='>' || Peek(theData)=='<' || Peek(theData)=='=' || Peek(theData)=='!')
		{
			char aGet=Get(theData);
			if (aGet=='=') {if (Peek(theData)==aGet) Get(theData);if (aGet=='=') {int aTemp=(int)aResult;aResult=(float)(aTemp==(int)Factor(theData));}}
			if (aGet=='!') {if (Peek(theData)=='=') {Get(theData);int aTemp=(int)aResult;aResult=(float)(aTemp!=(int)Factor(theData));}else aResult=(float)(!(int)Factor(theData));}
			if (aGet=='>') 
			{
				if (Peek(theData)=='=') {Get(theData);int aTemp=(int)aResult;aResult=(float)(aTemp>=(int)Factor(theData));}
				else if (Peek(theData)=='>') {Get(theData);int aTemp=(int)aResult;aResult=(float)(aTemp>>(int)Factor(theData));}
				else {int aTemp=(int)aResult;aResult=(float)(aTemp>(int)Factor(theData));}}
			if (aGet=='<') 
			{
				if (Peek(theData)=='=') {Get(theData);int aTemp=(int)aResult;aResult=(float)(aTemp<=(int)Factor(theData));}
				else if (Peek(theData)=='<') {Get(theData);int aTemp=(int)aResult;aResult=(float)(aTemp<<(int)Factor(theData));}
				else {int aTemp=(int)aResult;aResult=(float)(aTemp<(int)Factor(theData));}}
		}

		while (Peek(theData)=='&' || Peek(theData)=='|' || Peek(theData)=='^')
		{
			char aGet=Get(theData);
			if ((aGet=='&' || aGet=='|') && Peek(theData)==aGet) Get(theData);	// Handle double &&,== and || as if they were single

			if (aGet=='&') {int aTemp=(int)aResult;aTemp&=(int)Factor(theData);aResult=(float)aTemp;}
			else if (aGet=='|') {int aTemp=(int)aResult;aTemp|=(int)Factor(theData);aResult=(float)aTemp;}
			else if (aGet=='^') {int aTemp=(int)aResult;aTemp^=(int)Factor(theData);aResult=(float)aTemp;}
		}

		while (Peek(theData)=='*' || Peek(theData)=='/')
		{
			if (Get(theData)=='*') aResult*=Factor(theData);
			else aResult/=Factor(theData);
		}

		return aResult;
	}
	float Expression(String& theData)
	{
		if (!theData.mCursor) theData.mCursor=theData.mData;if (!theData.mCursor) return 0;
		float aResult=Term(theData);
		while (Peek(theData)=='+' || Peek(theData)=='-')
		{
			if (Get(theData)=='+') aResult+=Term(theData);
			else aResult-=Term(theData);
		}
		return aResult;
	}
}

float Math::Eval(String theEval)
{
	return TinyEvaluator::Expression(theEval);
}

Point Math3D::TriangleXYZToUV(Vector thePos, Vertex2D* theTriangle)
{
	Vector aTr1=theTriangle->ToVector()-(theTriangle+1)->ToVector();
	Vector aTr2=theTriangle->ToVector()-(theTriangle+2)->ToVector();

	Vector aF1 = theTriangle->ToVector()-thePos;
	Vector aF2 = (theTriangle+1)->ToVector()-thePos;
	Vector aF3 = (theTriangle+2)->ToVector()-thePos;

	float aA=aTr1.Cross(aTr2).Length();
	float aA1=aF2.Cross(aF3).Length()/aA;
	float aA2=aF3.Cross(aF1).Length()/aA;
	float aA3=aF1.Cross(aF2).Length()/aA;

	Point aUV=(theTriangle->ToUV()*aA1)+((theTriangle+1)->ToUV()*aA2)+((theTriangle+2)->ToUV()*aA3);
	return aUV;
}

Point Math3D::TriangleXYZToUV(Vector thePos, Vector theTPos0, Vector theTPos1, Vector theTPos2, Point theUVPos0, Point theUVPos1, Point theUVPos2)
{
	Vector aTr1=theTPos0-(theTPos1);
	Vector aTr2=theTPos0-(theTPos2);

	Vector aF1 = theTPos0-thePos;
	Vector aF2 = theTPos1-thePos;
	Vector aF3 = theTPos2-thePos;

	float aA=aTr1.Cross(aTr2).Length();
	float aA1=aF2.Cross(aF3).Length()/aA;
	float aA2=aF3.Cross(aF1).Length()/aA;
	float aA3=aF1.Cross(aF2).Length()/aA;

	Point aUV=(theUVPos0*aA1)+((theUVPos1)*aA2)+((theUVPos2)*aA3);
	return aUV;
}


Vector Math3D::TriangleUVToXYZ(Point theUV, Vertex2D* theTriangle)
{
	Point aTr1=theTriangle->ToUV()-(theTriangle+1)->ToUV();
	Point aTr2=theTriangle->ToUV()-(theTriangle+2)->ToUV();
	Point aF1 = theTriangle->ToUV()-theUV;
	Point aF2 = (theTriangle+1)->ToUV()-theUV;
	Point aF3 = (theTriangle+2)->ToUV()-theUV;
	float aA=aTr1.Cross(aTr2);
	float aA1=aF2.Cross(aF3)/aA;
	float aA2=aF3.Cross(aF1)/aA;
	float aA3=aF1.Cross(aF2)/aA;

	Vector aXYZ=(theTriangle->ToVector()*aA1)+((theTriangle+1)->ToVector()*aA2)+((theTriangle+2)->ToVector()*aA3);
	return aXYZ;
}

Vector Math3D::TriangleUVToXYZ(Point theUV, Vector* theTPos, Point *theTUV)
{
	Point aTr1=theTUV[0]-theTUV[1];
	Point aTr2=theTUV[0]-theTUV[2];

	Point aF1 = theTUV[0]-theUV;
	Point aF2 = theTUV[1]-theUV;
	Point aF3 = theTUV[2]-theUV;

	float aA=aTr1.Cross(aTr2);
	float aA1=aF2.Cross(aF3)/aA;
	float aA2=aF3.Cross(aF1)/aA;
	float aA3=aF1.Cross(aF2)/aA;

	return (theTPos[0]*aA1)+(theTPos[1]*aA2)+(theTPos[2]*aA3);
}

Vector Math3D::GetTriangleInterpolations(Vector thePos, Vector theT1, Vector theT2, Vector theT3)
{
	Vector aTr1=theT1-theT2;
	Vector aTr2=theT1-theT3;

	Vector aF1=theT1-thePos;
	Vector aF2=theT2-thePos;
	Vector aF3=theT3-thePos;

	// calculate the areas and factors (order of parameters doesn't matter):
	float aA=aTr1.Cross(aTr2).Length();
	if (aA==0) return Vector(0,0,0);
	float aA1=aF2.Cross(aF3).Length()/aA;
	float aA2=aF3.Cross(aF1).Length()/aA;
	float aA3=aF1.Cross(aF2).Length()/aA;

	return Vector(aA1,aA2,aA3);
}

void SetPrettyCloseThreshold(float theNumber)
{
	gPrettyCloseNumber=theNumber;
}

//
// Someone else's code to detect Triangle/Triangle overlap.
// My port of it didn't work!  I fucked something up!
// But this does work... so onward.
//
namespace SomeoneElse_TriangleTriangleCollision
{
	typedef float real;

	int tri_tri_overlap_test_3d(real p1[3], real q1[3], real r1[3], real p2[3], real q2[3], real r2[3]);
	int coplanar_tri_tri3d(real  p1[3], real  q1[3], real  r1[3], real  p2[3], real  q2[3], real  r2[3], real  N1[3], real  N2[3]);
	int tri_tri_overlap_test_2d(real p1[2], real q1[2], real r1[2], real p2[2], real q2[2], real r2[2]);
	int tri_tri_intersection_test_3d(real p1[3], real q1[3], real r1[3], real p2[3], real q2[3], real r2[3],int * coplanar, real source[3],real target[3]);

	#define ZERO_TEST(x)  (x == 0)
	#define CROSS(dest,v1,v2) dest[0]=v1[1]*v2[2]-v1[2]*v2[1];dest[1]=v1[2]*v2[0]-v1[0]*v2[2];dest[2]=v1[0]*v2[1]-v1[1]*v2[0];
	#define DOT(v1,v2) (v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2])
	#define SUB(dest,v1,v2) dest[0]=v1[0]-v2[0];dest[1]=v1[1]-v2[1];dest[2]=v1[2]-v2[2];
	#define SCALAR(dest,alpha,v) dest[0] = alpha * v[0];dest[1] = alpha * v[1];dest[2] = alpha * v[2];
	#define CHECK_MIN_MAX(p1,q1,r1,p2,q2,r2) {SUB(v1,p2,q1)SUB(v2,p1,q1)CROSS(N1,v1,v2)SUB(v1,q2,q1)if (DOT(v1,N1) > 0.0f) return 0;SUB(v1,p2,p1)SUB(v2,r1,p1)CROSS(N1,v1,v2)SUB(v1,r2,p1)if (DOT(v1,N1) > 0.0f) return 0;else return 1;}
	#define TRI_TRI_3D(p1,q1,r1,p2,q2,r2,dp2,dq2,dr2) {if (dp2 > 0.0f) { if (dq2 > 0.0f) CHECK_MIN_MAX(p1,r1,q1,r2,p2,q2) else if (dr2 > 0.0f) CHECK_MIN_MAX(p1,r1,q1,q2,r2,p2)else CHECK_MIN_MAX(p1,q1,r1,p2,q2,r2) }else if (dp2 < 0.0f) { if (dq2 < 0.0f) CHECK_MIN_MAX(p1,q1,r1,r2,p2,q2)else if (dr2 < 0.0f) CHECK_MIN_MAX(p1,q1,r1,q2,r2,p2)else CHECK_MIN_MAX(p1,r1,q1,p2,q2,r2)} else { if (dq2 < 0.0f) { if (dr2 >= 0.0f)  CHECK_MIN_MAX(p1,r1,q1,q2,r2,p2)else CHECK_MIN_MAX(p1,q1,r1,p2,q2,r2)} else if (dq2 > 0.0f) { if (dr2 > 0.0f) CHECK_MIN_MAX(p1,r1,q1,p2,q2,r2)else  CHECK_MIN_MAX(p1,q1,r1,q2,r2,p2)} else  {if (dr2 > 0.0f) CHECK_MIN_MAX(p1,q1,r1,r2,p2,q2)else if (dr2 < 0.0f) CHECK_MIN_MAX(p1,r1,q1,r2,p2,q2)else return coplanar_tri_tri3d(p1,q1,r1,p2,q2,r2,N1,N2);}}}

	int tri_tri_overlap_test_3d(real p1[3], real q1[3], real r1[3], real p2[3], real q2[3], real r2[3])
	{
		real dp1, dq1, dr1, dp2, dq2, dr2;
		real v1[3],v2[3];real N1[3], N2[3]; 
		SUB(v1,p2,r2);SUB(v2,q2,r2);CROSS(N2,v1,v2)
		SUB(v1,p1,r2)dp1 = DOT(v1,N2);SUB(v1,q1,r2)dq1 = DOT(v1,N2);SUB(v1,r1,r2);dr1 = DOT(v1,N2);
		if (((dp1 * dq1) > 0.0f) && ((dp1 * dr1) > 0.0f))  return 0; 
		SUB(v1,q1,p1);SUB(v2,r1,p1);CROSS(N1,v1,v2)
		SUB(v1,p2,r1);dp2 = DOT(v1,N1);SUB(v1,q2,r1);dq2 = DOT(v1,N1);SUB(v1,r2,r1);dr2 = DOT(v1,N1);
		if (((dp2 * dq2) > 0.0f) && ((dp2 * dr2) > 0.0f)) return 0;
		if (dp1 > 0.0f) {if (dq1 > 0.0f) TRI_TRI_3D(r1,p1,q1,p2,r2,q2,dp2,dr2,dq2) else if (dr1 > 0.0f) TRI_TRI_3D(q1,r1,p1,p2,r2,q2,dp2,dr2,dq2) else TRI_TRI_3D(p1,q1,r1,p2,q2,r2,dp2,dq2,dr2)} else if (dp1 < 0.0f) {if (dq1 < 0.0f) TRI_TRI_3D(r1,p1,q1,p2,q2,r2,dp2,dq2,dr2)else if (dr1 < 0.0f) TRI_TRI_3D(q1,r1,p1,p2,q2,r2,dp2,dq2,dr2)else TRI_TRI_3D(p1,q1,r1,p2,r2,q2,dp2,dr2,dq2)} else {if (dq1 < 0.0f) {if (dr1 >= 0.0f) TRI_TRI_3D(q1,r1,p1,p2,r2,q2,dp2,dr2,dq2)else TRI_TRI_3D(p1,q1,r1,p2,q2,r2,dp2,dq2,dr2)}else if (dq1 > 0.0f) {if (dr1 > 0.0f) TRI_TRI_3D(p1,q1,r1,p2,r2,q2,dp2,dr2,dq2)else TRI_TRI_3D(q1,r1,p1,p2,q2,r2,dp2,dq2,dr2)}else  {if (dr1 > 0.0f) TRI_TRI_3D(r1,p1,q1,p2,q2,r2,dp2,dq2,dr2)else if (dr1 < 0.0f) TRI_TRI_3D(r1,p1,q1,p2,r2,q2,dp2,dr2,dq2)else return coplanar_tri_tri3d(p1,q1,r1,p2,q2,r2,N1,N2);}}
	};

	int coplanar_tri_tri3d(real p1[3], real q1[3], real r1[3],
		real p2[3], real q2[3], real r2[3],
		real normal_1[3], real normal_2[3]){
		real P1[2],Q1[2],R1[2];real P2[2],Q2[2],R2[2];
		real n_x = ((normal_1[0]<0)?-normal_1[0]:normal_1[0]);
		real n_y = ((normal_1[1]<0)?-normal_1[1]:normal_1[1]);
		real n_z = ((normal_1[2]<0)?-normal_1[2]:normal_1[2]);

		if (( n_x > n_z ) && ( n_x >= n_y )) {P1[0] = q1[2]; P1[1] = q1[1];Q1[0] = p1[2]; Q1[1] = p1[1];R1[0] = r1[2]; R1[1] = r1[1]; P2[0] = q2[2]; P2[1] = q2[1];Q2[0] = p2[2]; Q2[1] = p2[1];R2[0] = r2[2]; R2[1] = r2[1];} 
		else if (( n_y > n_z ) && ( n_y >= n_x )) {P1[0] = q1[0]; P1[1] = q1[2];Q1[0] = p1[0]; Q1[1] = p1[2];R1[0] = r1[0]; R1[1] = r1[2]; P2[0] = q2[0]; P2[1] = q2[2];Q2[0] = p2[0]; Q2[1] = p2[2];R2[0] = r2[0]; R2[1] = r2[2];} 
		else {P1[0] = p1[0]; P1[1] = p1[1]; Q1[0] = q1[0]; Q1[1] = q1[1]; R1[0] = r1[0]; R1[1] = r1[1]; P2[0] = p2[0]; P2[1] = p2[1]; Q2[0] = q2[0]; Q2[1] = q2[1];R2[0] = r2[0]; R2[1] = r2[1];}
		return tri_tri_overlap_test_2d(P1,Q1,R1,P2,Q2,R2);
	};

	#define CONSTRUCT_INTERSECTION(p1,q1,r1,p2,q2,r2) {SUB(v1,q1,p1) SUB(v2,r2,p1) CROSS(N,v1,v2) SUB(v,p2,p1) if (DOT(v,N) > 0.0f) {SUB(v1,r1,p1) CROSS(N,v1,v2) if (DOT(v,N) <= 0.0f) { SUB(v2,q2,p1) CROSS(N,v1,v2) if (DOT(v,N) > 0.0f) { SUB(v1,p1,p2)SUB(v2,p1,r1)alpha = DOT(v1,N2) / DOT(v2,N2);SCALAR(v1,alpha,v2)SUB(source,p1,v1)SUB(v1,p2,p1)SUB(v2,p2,r2)alpha = DOT(v1,N1) / DOT(v2,N1);SCALAR(v1,alpha,v2)SUB(target,p2,v1)return 1;} else {SUB(v1,p2,p1)SUB(v2,p2,q2)alpha = DOT(v1,N1) / DOT(v2,N1);SCALAR(v1,alpha,v2)SUB(source,p2,v1)SUB(v1,p2,p1)SUB(v2,p2,r2)alpha = DOT(v1,N1) / DOT(v2,N1);SCALAR(v1,alpha,v2)SUB(target,p2,v1)return 1;}} else {return 0;}} else {SUB(v2,q2,p1)CROSS(N,v1,v2)if (DOT(v,N) < 0.0f) {return 0;} else {SUB(v1,r1,p1)CROSS(N,v1,v2)if (DOT(v,N) >= 0.0f) {SUB(v1,p1,p2)SUB(v2,p1,r1)alpha = DOT(v1,N2) / DOT(v2,N2);SCALAR(v1,alpha,v2)SUB(source,p1,v1)SUB(v1,p1,p2)SUB(v2,p1,q1)alpha = DOT(v1,N2) / DOT(v2,N2);SCALAR(v1,alpha,v2)SUB(target,p1,v1)return 1;} else {SUB(v1,p2,p1)SUB(v2,p2,q2)alpha = DOT(v1,N1) / DOT(v2,N1);SCALAR(v1,alpha,v2)SUB(source,p2,v1)SUB(v1,p1,p2)SUB(v2,p1,q1)alpha = DOT(v1,N2) / DOT(v2,N2);SCALAR(v1,alpha,v2)SUB(target,p1,v1)return 1;}}}} 
	#define TRI_TRI_INTER_3D(p1,q1,r1,p2,q2,r2,dp2,dq2,dr2) {if (dp2 > 0.0f) { if (dq2 > 0.0f) CONSTRUCT_INTERSECTION(p1,r1,q1,r2,p2,q2) else if (dr2 > 0.0f) CONSTRUCT_INTERSECTION(p1,r1,q1,q2,r2,p2)else CONSTRUCT_INTERSECTION(p1,q1,r1,p2,q2,r2) }else if (dp2 < 0.0f) { if (dq2 < 0.0f) CONSTRUCT_INTERSECTION(p1,q1,r1,r2,p2,q2)else if (dr2 < 0.0f) CONSTRUCT_INTERSECTION(p1,q1,r1,q2,r2,p2)else CONSTRUCT_INTERSECTION(p1,r1,q1,p2,q2,r2)} else { if (dq2 < 0.0f) { if (dr2 >= 0.0f)  CONSTRUCT_INTERSECTION(p1,r1,q1,q2,r2,p2)else CONSTRUCT_INTERSECTION(p1,q1,r1,p2,q2,r2)} else if (dq2 > 0.0f) { if (dr2 > 0.0f) CONSTRUCT_INTERSECTION(p1,r1,q1,p2,q2,r2)else  CONSTRUCT_INTERSECTION(p1,q1,r1,q2,r2,p2)} else  { if (dr2 > 0.0f) CONSTRUCT_INTERSECTION(p1,q1,r1,r2,p2,q2)else if (dr2 < 0.0f) CONSTRUCT_INTERSECTION(p1,r1,q1,r2,p2,q2)else { *coplanar = 1; return coplanar_tri_tri3d(p1,q1,r1,p2,q2,r2,N1,N2);} }}}

	int tri_tri_intersection_test_3d(real p1[3], real q1[3], real r1[3], real p2[3], real q2[3], real r2[3],int * coplanar,real source[3], real target[3] )
	{
		real dp1, dq1, dr1, dp2, dq2, dr2;
		real v1[3], v2[3], v[3];
		real N1[3], N2[3], N[3];
		real alpha;

		SUB(v1,p2,r2);SUB(v2,q2,r2);CROSS(N2,v1,v2);SUB(v1,p1,r2);dp1 = DOT(v1,N2);SUB(v1,q1,r2);dq1 = DOT(v1,N2);SUB(v1,r1,r2);dr1 = DOT(v1,N2);if (((dp1 * dq1) > 0.0f) && ((dp1 * dr1) > 0.0f))  return 0; 
		SUB(v1,q1,p1);SUB(v2,r1,p1);CROSS(N1,v1,v2);SUB(v1,p2,r1);dp2 = DOT(v1,N1);SUB(v1,q2,r1);dq2 = DOT(v1,N1);SUB(v1,r2,r1);dr2 = DOT(v1,N1);if (((dp2 * dq2) > 0.0f) && ((dp2 * dr2) > 0.0f)) return 0;
		if (dp1 > 0.0f) {if (dq1 > 0.0f) TRI_TRI_INTER_3D(r1,p1,q1,p2,r2,q2,dp2,dr2,dq2)else if (dr1 > 0.0f) TRI_TRI_INTER_3D(q1,r1,p1,p2,r2,q2,dp2,dr2,dq2)else TRI_TRI_INTER_3D(p1,q1,r1,p2,q2,r2,dp2,dq2,dr2)} else if (dp1 < 0.0f) {if (dq1 < 0.0f) TRI_TRI_INTER_3D(r1,p1,q1,p2,q2,r2,dp2,dq2,dr2)	else if (dr1 < 0.0f) TRI_TRI_INTER_3D(q1,r1,p1,p2,q2,r2,dp2,dq2,dr2)else TRI_TRI_INTER_3D(p1,q1,r1,p2,r2,q2,dp2,dr2,dq2)} else {if (dq1 < 0.0f) {if (dr1 >= 0.0f) TRI_TRI_INTER_3D(q1,r1,p1,p2,r2,q2,dp2,dr2,dq2)else TRI_TRI_INTER_3D(p1,q1,r1,p2,q2,r2,dp2,dq2,dr2)}else if (dq1 > 0.0f) {if (dr1 > 0.0f) TRI_TRI_INTER_3D(p1,q1,r1,p2,r2,q2,dp2,dr2,dq2)else TRI_TRI_INTER_3D(q1,r1,p1,p2,q2,r2,dp2,dq2,dr2)}else  {if (dr1 > 0.0f) TRI_TRI_INTER_3D(r1,p1,q1,p2,q2,r2,dp2,dq2,dr2)else if (dr1 < 0.0f) TRI_TRI_INTER_3D(r1,p1,q1,p2,r2,q2,dp2,dr2,dq2)else {*coplanar = 1;return coplanar_tri_tri3d(p1,q1,r1,p2,q2,r2,N1,N2);}}}
	};

	#define ORIENT_2D(a, b, c)  ((a[0]-c[0])*(b[1]-c[1])-(a[1]-c[1])*(b[0]-c[0]))
	#define INTERSECTION_TEST_VERTEXA(P1, Q1, R1, P2, Q2, R2) {if (ORIENT_2D(R2,P2,Q1) >= 0.0f)if (ORIENT_2D(R2,Q2,Q1) <= 0.0f)if (ORIENT_2D(P1,P2,Q1) > 0.0f) {if (ORIENT_2D(P1,Q2,Q1) <= 0.0f) return 1; else return 0;} else {if (ORIENT_2D(P1,P2,R1) >= 0.0f)if (ORIENT_2D(Q1,R1,P2) >= 0.0f) return 1; else return 0;else return 0;}else if (ORIENT_2D(P1,Q2,Q1) <= 0.0f)if (ORIENT_2D(R2,Q2,R1) <= 0.0f)if (ORIENT_2D(Q1,R1,Q2) >= 0.0f) return 1; else return 0;else return 0;else return 0;else if (ORIENT_2D(R2,P2,R1) >= 0.0f) if (ORIENT_2D(Q1,R1,R2) >= 0.0f)if (ORIENT_2D(P1,P2,R1) >= 0.0f) return 1;else return 0;else if (ORIENT_2D(Q1,R1,Q2) >= 0.0f) {if (ORIENT_2D(R2,R1,Q2) >= 0.0f) return 1; else return 0; }else return 0; else  return 0;};
	#define INTERSECTION_TEST_VERTEX(P1, Q1, R1, P2, Q2, R2) { if (ORIENT_2D(R2,P2,Q1) >= 0.0f) if (ORIENT_2D(R2,Q2,Q1) <= 0.0f) if (ORIENT_2D(P1,P2,Q1) > 0.0f) {if (ORIENT_2D(P1,Q2,Q1) <= 0.0f) return 1; else return 0;} else { if (ORIENT_2D(P1,P2,R1) >= 0.0f) if (ORIENT_2D(Q1,R1,P2) >= 0.0f) return 1; else return 0; else return 0;} else if (ORIENT_2D(P1,Q2,Q1) <= 0.0f) if (ORIENT_2D(R2,Q2,R1) <= 0.0f) if (ORIENT_2D(Q1,R1,Q2) >= 0.0f) return 1; else return 0;else return 0;else return 0;else if (ORIENT_2D(R2,P2,R1) >= 0.0f) if (ORIENT_2D(Q1,R1,R2) >= 0.0f) if (ORIENT_2D(P1,P2,R1) >= 0.0f) return 1;else return 0;else if (ORIENT_2D(Q1,R1,Q2) >= 0.0f) {if (ORIENT_2D(R2,R1,Q2) >= 0.0f) return 1; else return 0; }else return 0;else  return 0;};
	#define INTERSECTION_TEST_EDGE(P1, Q1, R1, P2, Q2, R2) { if (ORIENT_2D(R2,P2,Q1) >= 0.0f) {if (ORIENT_2D(P1,P2,Q1) >= 0.0f) { if (ORIENT_2D(P1,Q1,R2) >= 0.0f) return 1; else return 0;} else { if (ORIENT_2D(Q1,R1,P2) >= 0.0f){ if (ORIENT_2D(R1,P1,P2) >= 0.0f) return 1; else return 0;} else return 0; } } else {if (ORIENT_2D(R2,P2,R1) >= 0.0f) {if (ORIENT_2D(P1,P2,R1) >= 0.0f) {if (ORIENT_2D(P1,R1,R2) >= 0.0f) return 1; else {if (ORIENT_2D(Q1,R1,R2) >= 0.0f) return 1; else return 0;}}else  return 0; }else return 0; }}

	int ccw_tri_tri_intersection_2d(real p1[2], real q1[2], real r1[2], real p2[2], real q2[2], real r2[2]) {if ( ORIENT_2D(p2,q2,p1) >= 0.0f ) {if ( ORIENT_2D(q2,r2,p1) >= 0.0f ) {if ( ORIENT_2D(r2,p2,p1) >= 0.0f ) return 1;else INTERSECTION_TEST_EDGE(p1,q1,r1,p2,q2,r2)} else {  if ( ORIENT_2D(r2,p2,p1) >= 0.0f ) INTERSECTION_TEST_EDGE(p1,q1,r1,r2,p2,q2)else INTERSECTION_TEST_VERTEX(p1,q1,r1,p2,q2,r2)}}else {if ( ORIENT_2D(q2,r2,p1) >= 0.0f ) {if ( ORIENT_2D(r2,p2,p1) >= 0.0f )INTERSECTION_TEST_EDGE(p1,q1,r1,q2,r2,p2)else  INTERSECTION_TEST_VERTEX(p1,q1,r1,q2,r2,p2)}else INTERSECTION_TEST_VERTEX(p1,q1,r1,r2,p2,q2)}};
	int tri_tri_overlap_test_2d(real p1[2], real q1[2], real r1[2],real p2[2], real q2[2], real r2[2]) {if ( ORIENT_2D(p1,q1,r1) < 0.0f ) if ( ORIENT_2D(p2,q2,r2) < 0.0f ) return ccw_tri_tri_intersection_2d(p1,r1,q1,p2,r2,q2); else return ccw_tri_tri_intersection_2d(p1,r1,q1,p2,q2,r2); else if ( ORIENT_2D(p2,q2,r2) < 0.0f )return ccw_tri_tri_intersection_2d(p1,q1,r1,p2,r2,q2);else return ccw_tri_tri_intersection_2d(p1,q1,r1,p2,q2,r2);};
}

bool Math3D::DoesTriangleIntersectTriangle(Vector theP1,Vector theQ1,Vector theR1, Vector theP2,Vector theQ2,Vector theR2, float theFudgeFactor) 
{
	/*
	Cube aCube1;
	Cube aCube2;
	aCube1.FromTriangle(theP1,theQ1,theR1);
	aCube2.FromTriangle(theP2,theQ2,theR2);
	aCube1=aCube1.Expand(gMath.mMachineEpsilon);
	aCube2=aCube2.Expand(gMath.mMachineEpsilon);
	if (!aCube1.Intersects(aCube2)) return false;
	/**/
	if (theFudgeFactor!=0)
	{
		Triangle3D aT1(theP1,theQ1,theR1);
		Triangle3D aT2(theP2,theQ2,theR2);
		aT1.Expand(theFudgeFactor);
		aT2.Expand(theFudgeFactor);
		theP1=aT1.mPos[0];
		theQ1=aT1.mPos[1];
		theR1=aT1.mPos[2];
		theP2=aT2.mPos[0];
		theQ2=aT2.mPos[1];
		theR2=aT2.mPos[2];
	}

	return (SomeoneElse_TriangleTriangleCollision::tri_tri_overlap_test_3d(theP1.AsFloat(),theQ1.AsFloat(),theR1.AsFloat(),theP2.AsFloat(),theQ2.AsFloat(),theR2.AsFloat()))!=0;
}

bool Math3D::IsPointInTriangleVolume(Vector thePos, Vector theT0, Vector theT1, Vector theT2)
{
	Vector aU=theT1-theT0;
	Vector aV=theT2-theT0;
	Vector aN=aU.Cross(aV);
	Vector aW=thePos-theT0;

	float aGamma=aU.Cross(aW).Dot(aN)/aN.Dot(aN);
	float aBeta=aW.Cross(aV).Dot(aN)/aN.Dot(aN);
	float aAlpha=1.0f-aGamma-aBeta;

	return ((0<=aAlpha) && (aAlpha<=1) && (0<=aBeta)  && (aBeta<=1) && (0<=aGamma) && (aGamma<=1));
}

bool Math3D::IsPointInTriangleVolume(DVector thePos, DVector theT0, DVector theT1, DVector theT2)
{
	DVector aU=theT1-theT0;
	DVector aV=theT2-theT0;
	DVector aN=aU.Cross(aV);
	DVector aW=thePos-theT0;

	double aGamma=aU.Cross(aW).Dot(aN)/aN.Dot(aN);
	double aBeta=aW.Cross(aV).Dot(aN)/aN.Dot(aN);
	double aAlpha=1.0f-aGamma-aBeta;

	return ((0<=aAlpha) && (aAlpha<=1) && (0<=aBeta)  && (aBeta<=1) && (0<=aGamma) && (aGamma<=1));
}


bool Math::GetLineCircleIntersections(Point theCenter, float theRadius, Point theLineA, Point theLineB, Array<Point>& theResults)
{
	theResults.Reset();
	Point aBA=theLineB-theLineA;
	Point aCA=theCenter-theLineA;

	float aA=aBA.mX*aBA.mX+aBA.mY*aBA.mY;
	float aBBy2=aBA.mX*aCA.mX+aBA.mY*aCA.mY;
	float aC=aCA.mX*aCA.mX+aCA.mY*aCA.mY-theRadius*theRadius;

	float aPBy2=aBBy2/aA;
	float aQ=aC/aA;

	float aDisc=aPBy2*aPBy2-aQ;
	if (aDisc<0) return false;
	float aTmpSqrt=(float)sqrt(aDisc);
	float aABScalingFactor1=-aPBy2+aTmpSqrt;
	float aABScalingFactor2=-aPBy2-aTmpSqrt;

	int aRSpot=0;
	if (aABScalingFactor1<=0.0f && aABScalingFactor1>=-1.0f) theResults[aRSpot++]=Point(theLineA.mX-aBA.mX*aABScalingFactor1,theLineA.mY-aBA.mY*aABScalingFactor1);
	if (aDisc==0) return true;
	if (aABScalingFactor2<=0.0f && aABScalingFactor2>=-1.0f) theResults[aRSpot++]=Point(theLineA.mX-aBA.mX*aABScalingFactor2,theLineA.mY-aBA.mY*aABScalingFactor2);
	return true;
}

bool Math::GetInfiniteLineCircleIntersections(Point theCenter, float theRadius, Point theLineA, Point theLineB, Array<Point>& theResults)
{
	theResults.Reset();
	Point aBA=theLineB-theLineA;
	Point aCA=theCenter-theLineA;

	float aA=aBA.mX*aBA.mX+aBA.mY*aBA.mY;
	float aBBy2=aBA.mX*aCA.mX+aBA.mY*aCA.mY;
	float aC=aCA.mX*aCA.mX+aCA.mY*aCA.mY-theRadius*theRadius;

	float aPBy2=aBBy2/aA;
	float aQ=aC/aA;

	float aDisc=aPBy2*aPBy2-aQ;
	if (aDisc<0) return false;
	// if disc == 0 ... dealt with later
	float aTmpSqrt=(float)sqrt(aDisc);
	float aABScalingFactor1=-aPBy2+aTmpSqrt;
	float aABScalingFactor2=-aPBy2-aTmpSqrt;

	//gOut.Out("Factor: %f %f",aABScalingFactor1,aABScalingFactor2);

	//
	// Remove these ABScalingFactor<=0 checks to make it an infinite line
	//

	int aRSpot=0;
	theResults[aRSpot++]=Point(theLineA.mX-aBA.mX*aABScalingFactor1,theLineA.mY-aBA.mY*aABScalingFactor1);
	if (aDisc==0) return true;
	theResults[aRSpot++]=Point(theLineA.mX-aBA.mX*aABScalingFactor2,theLineA.mY-aBA.mY*aABScalingFactor2);
	return true;
}


bool Math3D::GetLineSphereIntersections(Vector theCenter, float theRadius, Vector theLineA, Vector theLineB, Array<Vector>& theResults)
{
	theResults.Reset();
	Vector aBA=theLineB-theLineA;
	Vector aCA=theCenter-theLineA;

	float aA=aBA.mX*aBA.mX+aBA.mY*aBA.mY+aBA.mZ*aBA.mZ;
	float aBBy2=aBA.mX*aCA.mX+aBA.mY*aCA.mY+aBA.mZ*aCA.mZ;
	float aC=aCA.mX*aCA.mX+aCA.mY*aCA.mY+aCA.mZ*aCA.mZ-theRadius*theRadius;

	float aPBy2=aBBy2/aA;
	float aQ=aC/aA;

	float aDisc=aPBy2*aPBy2-aQ;
	if (aDisc<0) return false;
	float aTmpSqrt=(float)sqrt(aDisc);
	float aABScalingFactor1=-aPBy2+aTmpSqrt;
	float aABScalingFactor2=-aPBy2-aTmpSqrt;

	int aRSpot=0;
	if (aABScalingFactor1<=0.0f && aABScalingFactor1>=-1.0f) theResults[aRSpot++]=theLineA-aBA*aABScalingFactor1;
	if (aDisc==0) return true;
	if (aABScalingFactor2<=0.0f && aABScalingFactor2>=-1.0f) theResults[aRSpot++]=theLineA-aBA*aABScalingFactor2;
	return true;
}

bool Math3D::GetLineCylinderIntersections(Vector theOrigin, Vector theAxis, float theRadius, Vector theLineA, Vector theLineB, Array<Vector>& theResults)
{
	if (theAxis.LengthSquared()!=1.0f) theAxis.Normalize();

	Matrix aMat;
	if (fabs(theAxis.Dot(Vector(0,0,-1))<1.0f))
	{
		aMat.VectorToVector(theAxis,Vector(0,0,-1));
		aMat.Translate(-theOrigin);
		theLineA=aMat.ProcessPoint(theLineA);
		theLineB=aMat.ProcessPoint(theLineB);
		theOrigin=aMat.ProcessPoint(theOrigin);
	}

	theResults.Reset();
	Vector aBA=theLineB-theLineA;
	Vector aCA=theOrigin-theLineA;

	float aA=aBA.mX*aBA.mX+aBA.mY*aBA.mY;
	float aBBy2=aBA.mX*aCA.mX+aBA.mY*aCA.mY;
	float aC=aCA.mX*aCA.mX+aCA.mY*aCA.mY-theRadius*theRadius;

	float aPBy2=aBBy2/aA;
	float aQ=aC/aA;

	float aDisc=aPBy2*aPBy2-aQ;
	if (aDisc<0) return false;

	float aTmpSqrt=(float)sqrt(aDisc);
	float aABScalingFactor1=-aPBy2+aTmpSqrt;
	float aABScalingFactor2=-aPBy2-aTmpSqrt;

	int aRSpot=0;
	if (aABScalingFactor1<=0.0f && aABScalingFactor1>=-1.0f) theResults[aRSpot++]=theLineA-aBA*aABScalingFactor1;
	if (aDisc!=0) if (aABScalingFactor2<=0.0f && aABScalingFactor2>=-1.0f) theResults[aRSpot++]=theLineA-aBA*aABScalingFactor2;

	Matrix aInvert=aMat.GetInvert();
	for (int aCount=0;aCount<theResults.Size();aCount++) theResults[aCount]=aInvert.ProcessPoint3D(theResults[aCount]);
	return true;
}


Line3D Math3D::ClosestLineLine(Vector theA, Vector theB, Vector theC, Vector theD, float* resultInterpolationA, float* resultInterpolationB)
{
	Vector aU = theB-theA;
	Vector aV = theD-theC;
	Vector aW = theA-theC;

	float a = aU.Dot(aU);
	float b = aU.Dot(aV);
	float c = aV.Dot(aV);
	float d = aU.Dot(aW);
	float e = aV.Dot(aW);
	float sc, sN, sD = a*c - b*b;
	float tc, tN, tD = a*c - b*b;
	float tol = (float)1e-15;
	if (sD < tol) {sN=0.0f;sD=1.0f;tN=e;tD=c;}
	else {sN = (b*e - c*d);tN = (a*e - b*d);if (sN < 0.0) {sN = 0.0f;tN = e;tD = c;} else if (sN > sD) {sN = sD;tN = e + b;tD = c;}}
	if (tN < 0.0f) {tN = 0.0f;if (-d < 0.0f)sN = 0.0f;else if (-d > a)sN = sD;else {sN = -d;sD = a;}}
	else if (tN > tD) {tN = tD;if ((-d + b) < 0.0f)sN = 0;else if ((-d + b) > a)sN = sD;else {sN = (-d +  b);sD = a;}}
	sc = (fabs(sN) < tol ? 0.0f : sN / sD);
	tc = (fabs(tN) < tol ? 0.0f : tN / tD);

	if (resultInterpolationA) *resultInterpolationA=sc;
	if (resultInterpolationB) *resultInterpolationB=tc;

	Line3D aResult;
	aResult.mPos[0]=theA+(sc*aU);
	aResult.mPos[1]=theC+(tc*aV);
	return aResult;
}

Vector Math3D::ClosestPointOnTriangleToLine(Vector theT0, Vector theT1, Vector theT2, Vector theL1, Vector theL2)
{
	Line3D aL1=ClosestLineLine(theT0,theT1,theL1,theL2);
	Line3D aL2=ClosestLineLine(theT1,theT2,theL1,theL2);
	Line3D aL3=ClosestLineLine(theT2,theT0,theL1,theL2);

	float aDist;
	float aBestDist=gMath.DistanceToLineSquared(aL1.mPos[0],theL1,theL2);
	Vector aBestResult=aL1.mPos[0];

	aDist=gMath.DistanceToLineSquared(aL2.mPos[0],theL1,theL2);
	if (aDist<aBestDist) {aBestDist=aDist;aBestResult=aL2.mPos[0];}

	aDist=gMath.DistanceToLineSquared(aL3.mPos[0],theL1,theL2);
	if (aDist<aBestDist) {aBestDist=aDist;aBestResult=aL3.mPos[0];}

	return aBestResult;
}


Vector Math::GetCircleCenter(Vector theP1, Vector theP2, Vector theP3)
{
	// triangle "edges"
	const Vector aT = theP2-theP1;
	const Vector aU = theP3-theP1;
	const Vector aV = theP3-theP2;

	// triangle normal
	const Vector aW=aT.Cross(aU);
	const float aWSL=aW.LengthSquared();
	if (aWSL<10e-14) return theP1;

								  // helpers
	const float aIWSL2=1.0f/(2.0f*aWSL);
	const float aTT=aT.Dot(aT);
	const float aUU=aU.Dot(aU);

	return theP1+(aU*aTT*(aU.Dot(aV))-aT*aUU*(aT.Dot(aV)))*aIWSL2;

	/*
	Vector3d circCenter = p1 + (u*tt*(u*v) - t*uu*(t*v)) * iwsl2;
	double   circRadius = sqrt(tt * uu * (v*v) * iwsl2*0.5);
	Vector3d circAxis   = w / sqrt(wsl);
	*/
}

float Math::GetCircleRadius(Vector theP1, Vector theP2, Vector theP3)
{
	// triangle "edges"
	const Vector aT = theP2-theP1;
	const Vector aU = theP3-theP1;
	const Vector aV = theP3-theP2;

	// triangle normal
	const Vector aW=aT.Cross(aU);
	const float aWSL=aW.LengthSquared();
	if (aWSL<10e-14) return 0;

	// helpers
	const float aIWSL2=1.0f/(2.0f*aWSL);
	const float aTT=aT.Dot(aT);
	const float aUU=aU.Dot(aU);

	return (float)sqrt(aTT*aUU*(aV.Dot(aV))*aIWSL2*.5f);
}

Vector Math::GetCircleNormal(Vector theP1, Vector theP2, Vector theP3)
{
	// triangle "edges"
	const Vector aT = theP2-theP1;
	const Vector aU = theP3-theP1;
	const Vector aV = theP3-theP2;

	// triangle normal
	const Vector aW=aT.Cross(aU);
	const float aWSL=aW.LengthSquared();
	if (aWSL<10e-14) return 0;

	// helpers
	const float aIWSL2=1.0f/(2.0f*aWSL);
	const float aTT=aT.Dot(aT);
	const float aUU=aU.Dot(aU);

	return aW/(float)sqrt(aWSL);
}

bool Math::GetCircleInfo(Vector theP1, Vector theP2, Vector theP3, Vector* theCenter, float* theRadius, Vector* theNormal)
{
	const Vector aT = theP2-theP1;
	const Vector aU = theP3-theP1;
	const Vector aV = theP3-theP2;

	const Vector aW=aT.Cross(aU);
	const float aWSL=aW.LengthSquared();
	if (aWSL<10e-14) return false;

	const float aIWSL2=1.0f/(2.0f*aWSL);
	const float aTT=aT.Dot(aT);
	const float aUU=aU.Dot(aU);

	if (theCenter) *theCenter=theP1+(aU*aTT*(aU.Dot(aV))-aT*aUU*(aT.Dot(aV)))*aIWSL2;
	if (theRadius) *theRadius=(float)sqrt(aTT*aUU*(aV.Dot(aV))*aIWSL2*.5f);
	if (theNormal) *theNormal=aW/(float)sqrt(aWSL);

	return true;
}

bool Math3D::DoesSphereIntersectRoundedCone(Vector spherePos, float sphereRadius, Vector coneStart, float coneStartRadius, Vector coneEnd, float coneEndRadius)
{
	/*
	sphereRadius=.001f;
	coneStartRadius=.1f;
	coneEndRadius=.5f;
	/**/

	Vector aV1=spherePos-coneStart;
	Vector aV2=coneEnd-coneStart;
	float aPercent=_clamp(0.0f,(aV1.mX*aV2.mX+aV1.mY*aV2.mY+aV1.mZ*aV2.mZ) / (aV2.mX*aV2.mX+aV2.mY*aV2.mY+aV2.mZ*aV2.mZ),1.0f);

	Vector aV3=aV1-(aPercent*aV2);
	float aConeRad=sphereRadius+(coneStartRadius+(coneEndRadius-coneStartRadius)*aPercent);
	return (aV3.LengthSquared()<SQUARED(aConeRad));

	/*
	Vector aConePos=coneStart+(aV2*aPercent);
	float aConeRad=coneStartRadius+(coneEndRadius-coneStartRadius)*aPercent;
	if (gMath.DistanceSquared(spherePos,aConePos)<=SQUARED(sphereRadius+aConeRad)) return true;
	*/
	return false;

	 
	//
	// These make for a faster decision... but they cause a little "bulge" also, so it's inexact.
	//
	/*
	if (gMath.DistanceSquared(spherePos,coneStart)<=SQUARED(sphereRadius+coneStartRadius)) return true;
	if (gMath.DistanceSquared(spherePos,coneEnd)<=SQUARED(sphereRadius+coneEndRadius)) return true;
	return false;

	Vector aV1=spherePos-coneStart;
	Vector aV2=coneEnd-coneStart;
	float aPercent=_clamp(0.0f,(aV1.mX*aV2.mX+aV1.mY*aV2.mY+aV1.mZ*aV2.mZ) / (aV2.mX*aV2.mX+aV2.mY*aV2.mY+aV2.mZ*aV2.mZ),1.0f);

	Vector aConePos=coneStart+(aV2*aPercent);
	float aConeRad=coneStartRadius-(coneEndRadius-coneStartRadius)*aPercent;

	if (gMath.DistanceSquared(spherePos,aConePos)<SQUARED(aConeRad+sphereRadius)) return true;



	/*
	Vector aV3=aV1-(aPercent*aV2);
	float aAt=coneStartRadius-(coneEndRadius-coneStartRadius)*aPercent;
	if (aV3.LengthSquared()<=SQUARED(sphereRadius+aAt)) return true;
	*/
	return false;
}


float Math3D::DistanceToRoundedCone(Vector thePos, Vector coneStart, float coneStartRadius, Vector coneEnd, float coneEndRadius)
{
	//*
	Vector aV1=thePos-coneStart;
	Vector aV2=coneEnd-coneStart;
	float aPercent=_clamp(0.0f,(aV1.mX*aV2.mX+aV1.mY*aV2.mY+aV1.mZ*aV2.mZ) / (aV2.mX*aV2.mX+aV2.mY*aV2.mY+aV2.mZ*aV2.mZ),1.0f);
	Vector aV3=aV1-(aPercent*aV2);
	return aV3.Length()-coneStartRadius-(coneEndRadius-coneStartRadius)*aPercent;

	/**/
	
	/*
	float aWork[10];

	aWork[0]=thePos.mX-coneStart.mX;
	aWork[1]=thePos.mY-coneStart.mY;
	aWork[2]=thePos.mZ-coneStart.mZ;
	aWork[3]=coneEnd.mX-coneStart.mX;
	aWork[4]=coneEnd.mY-coneStart.mY;
	aWork[5]=coneEnd.mZ-coneStart.mZ;
	aWork[6]=_clamp01((aWork[0]*aWork[3]+aWork[1]*aWork[4]+aWork[2]*aWork[5]) / (aWork[3]*aWork[3]+aWork[4]*aWork[4]+aWork[5]*aWork[5])); //h
	aWork[7]=aWork[0]-aWork[6]*aWork[3];
	aWork[8]=aWork[2]-aWork[6]*aWork[5];
	aWork[9]=aWork[1]-aWork[6]*aWork[4];
	return sqrt(aWork[7]*aWork[7]+aWork[9]*aWork[9]+aWork[8]*aWork[8])-coneStartRadius-(coneEndRadius-coneStartRadius)*aWork[6];
	/**/
}

//
// This appears to be a nonviable function.  You just don't get the right number when trying to subtract the squared radius of the sphere from
// the squared distance to the line.  I see no way to do it without sqrt.
//
/*
float Math3D::DistanceToRoundedConeSquared(Vector thePos, Vector coneStart, float coneStartRadius, Vector coneEnd, float coneEndRadius)
{
	Vector aV1=thePos-coneStart;
	Vector aV2=coneEnd-coneStart;
	float aPercent=_clamp(0.0f,(aV1.mX*aV2.mX+aV1.mY*aV2.mY+aV1.mZ*aV2.mZ) / (aV2.mX*aV2.mX+aV2.mY*aV2.mY+aV2.mZ*aV2.mZ),1.0f);
	Vector aV3=aV1-(aPercent*aV2);
	float aConeRad=(coneStartRadius+(coneEndRadius-coneStartRadius)*aPercent);
	return (aV3.LengthSquared()-SQUARED(aConeRad));
}
*/

float Math3D::DistanceToCubeSquared(Vector thePos, Cube theCube)
{
	if (theCube.ContainsPoint(thePos)) return 0;
	Vector aClosest=theCube.ClampPoint(thePos);
	return gMath.DistanceSquared(thePos,aClosest);

	/*
	float aD=0;

	if(thePos.mX<theCube.MinX()) {float aS=thePos.mX-theCube.MinX();aD+=aS*aS;}
	else if(thePos.mX>theCube.MaxX()) {float aS=thePos.mX-theCube.MaxX();aD+=aS*aS;}

	if(thePos.mY<theCube.MinY()) {float aS=thePos.mY-theCube.MinY();aD+=aS*aS;}
	else if(thePos.mY>theCube.MaxY()) {float aS=thePos.mY-theCube.MaxY();aD+=aS*aS;}

	if(thePos.mZ<theCube.MinZ()) {float aS=thePos.mZ-theCube.MinZ();aD+=aS*aS;}
	else if(thePos.mZ>theCube.MaxZ()) {float aS=thePos.mZ-theCube.MaxZ();aD+=aS*aS;}

	return aD;
	*/
}

float Math3D::GetAngleBetweenVectors(Vector theV1, Vector theV2, Vector thePlaneNormal)
{
	float aAngle=(float)atan2(theV1.Cross(theV2).Dot(thePlaneNormal),theV1.Dot(theV2));
	return gMath.Deg(aAngle);
}

Line3D Math3D::GetSharedSeam(Triangle3D* aT1, Triangle3D* aT2, float theCloseEnough)
{
	//gGX.AddTri3D(*aT1,Color(1,0,0),5);
	//gGX.AddTri3D(*aT2,Color(0,1,0),3);
	float aThresh=theCloseEnough*theCloseEnough;
	for (int aC1=0;aC1<3;aC1++)
	{
		Line3D aL1(aT1->mPos[aC1],aT1->mPos[(aC1+1)%3]);
		Vector aDir=aL1.Dir();
		for (int aC2=0;aC2<3;aC2++)
		{
			Line3D aL2(aT2->mPos[aC2],aT2->mPos[(aC2+1)%3]);
			if (gMath.Abs(aDir.Dot(aL2.Dir()))>.9999f)
			{
				Line3D aClosest=ClosestLineLine(aL1,aL2);
				if (aClosest.LengthSquared()<=aThresh) return aL1;

				//gGX.AddLine3D(aClosest,Color(1,1,0));
			}
		}
	}
	return Line3D();
}

bool Math::SolveQuadratic(float theA, float theB, float theC, Point& theResult)
{
	float aSqrtVal=(theB*theB)-(4*theA*theC);
	if (aSqrtVal>=0) {theResult=Point((1.0f/(2*theA))*(-theB+(float)sqrt(aSqrtVal)),(1.0f/(2*theA))*(-theB-(float)sqrt(aSqrtVal)));return true;}
	return false;
}

bool Math::GetLineEllipseIntersections(Point theCenter, Point theRadius, Line theLine, Array<Point>& theResults)
{
	//
	// Y-Intercept of zero means invalid!
	//
	if (gMath.Abs(theLine.mPos[0].mX-theLine.mPos[1].mX)<.001f)
	{
		Line aLine;
		aLine.mPos[0]=theLine.mPos[0].Transpose();
		aLine.mPos[1]=theLine.mPos[1].Transpose();
		bool aResult=GetLineEllipseIntersections(theCenter.Transpose(),theRadius.Transpose(),aLine,theResults);
		foreach(aR,theResults) {aR=aR.Transpose();}
		return aResult;
	}

	theResults.Reset();
	Point aP1=theLine.mPos[0]-theCenter;
	Point aP2=theLine.mPos[1]-theCenter;
	Point aP3;
	Point aP4;

	//gGX.AddLine3D(aP1+Vector(0,.75f,0),aP2+Vector(0,.75f,0),Color(1,0,0));

	Rect aRect;
	aRect=aRect.Union(aP1);
	aRect=aRect.Union(aP2);
	aRect=aRect.Expand(mMachineEpsilon);

	float aS=(aP2.mY-aP1.mY)/(aP2.mX-aP1.mX);
	float aSI=aP2.mY-(aS*aP2.mX);
	float aA=(theRadius.mY*theRadius.mY)+(theRadius.mX*theRadius.mX*aS*aS);
	float aB=2.0f*theRadius.mX*theRadius.mX*aSI*aS;
	float aC=theRadius.mX*theRadius.mX*aSI*aSI-theRadius.mX*theRadius.mX*theRadius.mY*theRadius.mY;

	float aRadicand_sqrt=(float)sqrt((aB*aB)-(4.0f*aA*aC));
	aP3.mX=(-aB-aRadicand_sqrt)/(2.0f*aA);
	aP4.mX=(-aB+aRadicand_sqrt)/(2.0f*aA);
	aP3.mY=aS*aP3.mX+aSI;
	aP4.mY=aS*aP4.mX+aSI;

	if(aRect.ContainsPoint(aP3)) theResults+=(aP3+theCenter);
	if(aRect.ContainsPoint(aP4)) theResults+=(aP4+theCenter);
	return (theResults.Size()>0);
}


Point Math::GetNextPointAroundEllipseCW(Point P, Point C, Point theRadius, float theDegrees) 
{
	// Convert n degrees to radians
	float aRadians=theDegrees*mPI/180.0f;

	// Calculate the sin and cos of the angle
	float aSinAngle=(float)sin(aRadians);
	float aCosAngle=(float)cos(aRadians);

	// Translate point P and center C so that C is at the origin
	float aX1=P.mX-C.mX;
	float aY1=P.mY-C.mY;
	aX1/=theRadius.mX;
	aY1/=theRadius.mY;

	// Apply rotation matrix to point P
	float aNewX=aX1*aCosAngle-aY1*aSinAngle;
	float aNewY=aX1*aSinAngle+aY1*aCosAngle;

	// Scale point P back to ellipse dimensions
	aNewX=aNewX*theRadius.mX+C.mX;
	aNewY=aNewY*theRadius.mY+C.mY;

	// Return the next point on the ellipse
	return Point(aNewX,aNewY);
}


bool Math3D::DoesTriangleIntersectCubeQD(Vector theV1,Vector theV2,Vector theV3, Cube theCube)
{
	Cube aCC;
	aCC=aCC.Union(theV1);
	aCC=aCC.Union(theV2);
	aCC=aCC.Union(theV3);
	return theCube.Intersects(aCC);
}

//
// Triangle-Cube intersection...
// Taken from:
// https://fileadmin.cs.lth.se/cs/Personal/Tomas_Akenine-Moller/code/tribox3.txt
//
/*
#define X 0
#define Y 1
#define Z 2

#define CROSS(dest,v1,v2) \
dest[0]=v1[1]*v2[2]-v1[2]*v2[1]; \
dest[1]=v1[2]*v2[0]-v1[0]*v2[2]; \
dest[2]=v1[0]*v2[1]-v1[1]*v2[0];

#define DOT(v1,v2) (v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2])

#define SUB(dest,v1,v2) \
dest[0]=v1[0]-v2[0]; \
dest[1]=v1[1]-v2[1]; \
dest[2]=v1[2]-v2[2];

#define FINDMINMAX(x0,x1,x2,min,max) min=max=x0;if(x1<min) min=x1;if(x1>max) max=x1;if(x2<min) min=x2;if(x2>max) max=x2;
int planeBoxOverlap(float normal[3], float vert[3], float maxbox[3])
{
	int q;
	float vmin[3],vmax[3],v;
	for(q=X;q<=Z;q++)
	{
		v=vert[q];					// -NJMP-
		if(normal[q]>0.0f)
		{
			vmin[q]=-maxbox[q] - v;	// -NJMP-
			vmax[q]= maxbox[q] - v;	// -NJMP-
		}
		else
		{
			vmin[q]= maxbox[q] - v;	// -NJMP-
			vmax[q]=-maxbox[q] - v;	// -NJMP-
		}
	}
	if(DOT(normal,vmin)>0.0f) return 0;	// -NJMP-
	if(DOT(normal,vmax)>=0.0f) return 1;	// -NJMP-
	return 0;
}

#define AXISTEST_X01(a, b, fa, fb) p0 = a*v0[Y] - b*v0[Z];p2 = a*v2[Y] - b*v2[Z];if(p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;}rad = fa * boxhalfsize[Y] + fb * boxhalfsize[Z];if(min>rad || max<-rad) return 0;
#define AXISTEST_X2(a, b, fa, fb) p0 = a*v0[Y] - b*v0[Z];p1 = a*v1[Y] - b*v1[Z];if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;}rad = fa * boxhalfsize[Y] + fb * boxhalfsize[Z];if(min>rad || max<-rad) return 0;
#define AXISTEST_Y02(a, b, fa, fb) p0 = -a*v0[X] + b*v0[Z];p2 = -a*v2[X] + b*v2[Z];if(p0<p2) {min=p0; max=p2;} else {min=p2; max=p0;}rad = fa * boxhalfsize[X] + fb * boxhalfsize[Z];if(min>rad || max<-rad) return 0;
#define AXISTEST_Y1(a, b, fa, fb) p0 = -a*v0[X] + b*v0[Z];p1 = -a*v1[X] + b*v1[Z];if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;}rad = fa * boxhalfsize[X] + fb * boxhalfsize[Z];if(min>rad || max<-rad) return 0;
#define AXISTEST_Z12(a, b, fa, fb) p1 = a*v1[X] - b*v1[Y];p2 = a*v2[X] - b*v2[Y];if(p2<p1) {min=p2; max=p1;} else {min=p1; max=p2;}rad = fa * boxhalfsize[X] + fb * boxhalfsize[Y];if(min>rad || max<-rad) return 0;
#define AXISTEST_Z0(a, b, fa, fb) p0 = a*v0[X] - b*v0[Y];p1 = a*v1[X] - b*v1[Y];if(p0<p1) {min=p0; max=p1;} else {min=p1; max=p0;}rad = fa * boxhalfsize[X] + fb * boxhalfsize[Y];if(min>rad || max<-rad) return 0;

int triBoxOverlap(float boxcenter[3],float boxhalfsize[3],float triverts[3][3])
{
	float v0[3],v1[3],v2[3];
	float min,max,p0,p1,p2,rad,fex,fey,fez;		// -NJMP- "d" local variable removed
	float normal[3],e0[3],e1[3],e2[3];

	SUB(v0,triverts[0],boxcenter);
	SUB(v1,triverts[1],boxcenter);
	SUB(v2,triverts[2],boxcenter);
	SUB(e0,v1,v0);      // tri edge 0
	SUB(e1,v2,v1);      // tri edge 1
	SUB(e2,v0,v2);      // tri edge 2

	fex = fabsf(e0[X]);
	fey = fabsf(e0[Y]);
	fez = fabsf(e0[Z]);
	AXISTEST_X01(e0[Z], e0[Y], fez, fey);
	AXISTEST_Y02(e0[Z], e0[X], fez, fex);
	AXISTEST_Z12(e0[Y], e0[X], fey, fex);

	fex = fabsf(e1[X]);
	fey = fabsf(e1[Y]);
	fez = fabsf(e1[Z]);

	AXISTEST_X01(e1[Z], e1[Y], fez, fey);
	AXISTEST_Y02(e1[Z], e1[X], fez, fex);
	AXISTEST_Z0(e1[Y], e1[X], fey, fex);

	fex = fabsf(e2[X]);
	fey = fabsf(e2[Y]);
	fez = fabsf(e2[Z]);

	AXISTEST_X2(e2[Z], e2[Y], fez, fey);
	AXISTEST_Y1(e2[Z], e2[X], fez, fex);
	AXISTEST_Z12(e2[Y], e2[X], fey, fex);

	FINDMINMAX(v0[X],v1[X],v2[X],min,max);
	if(min>boxhalfsize[X] || max<-boxhalfsize[X]) return 0;
	FINDMINMAX(v0[Y],v1[Y],v2[Y],min,max);
	if(min>boxhalfsize[Y] || max<-boxhalfsize[Y]) return 0;
	FINDMINMAX(v0[Z],v1[Z],v2[Z],min,max);
	if(min>boxhalfsize[Z] || max<-boxhalfsize[Z]) return 0;

	CROSS(normal,e0,e1);
	if(!planeBoxOverlap(normal,v0,boxhalfsize)) return 0;	// -NJMP-
	return 1;
}
*/

bool Math::IsPointInCircumCircle(Point theT1, Point theT2, Point theT3, Point thePos)
{
	if (!IsCCW(theT1,theT2,theT3)) Swap(theT1,theT2);

	double aAX=(double)theT1.mX-(double)thePos.mX;	
	double aAY=(double)theT1.mY-(double)thePos.mY;
	double aBX=(double)theT2.mX-(double)thePos.mX;
	double aBY=(double)theT2.mY-(double)thePos.mY;
	double aCX=(double)theT3.mX-(double)thePos.mX;
	double aCY=(double)theT3.mY-(double)thePos.mY;
	return ((aAX*aAX+aAY*aAY)*(aBX*aCY-aCX*aBY)-
		(aBX*aBX+aBY*aBY)*(aAX*aCY-aCX*aAY)+
		(aCX*aCX+aCY*aCY)*(aAX*aBY-aBX*aAY)
		)>0.0;

}

Point Math::GetCircumCenter(Point theT1, Point theT2, Point theT3)
{
	// Calculate the perpendicular bisectors of two sides
	float& aX1 = theT1.mX, &aY1 = theT1.mY;
	float& aX2 = theT2.mX, &aY2 = theT2.mY;
	float& aX3 = theT3.mX, &aY3 = theT3.mY;
	float aA = aX2 - aX1;
	float aB = aY2 - aY1;
	float aC = aX3 - aX2;
	float aD = aY3 - aY2;
	float aE = aA*(aX1+aX2)+aB*(aY1+aY2);
	float aF = aC*(aX2+aX3)+aD*(aY2+aY3);
	float aG = 2.0f*(aA*(aY3-aY2)-aB*(aX3-aX2));
	if (Abs(aG)<1e-6) return theT1; // Colinear
	return Point((aD*aE-aB*aF)/aG,(aA*aF-aC*aE)/aG);
}

// Returns the % along line1 where the intersection occurs... otherwise returns NAN
float Math::GetLineIntersectionPercent(Point theLine1Start, Point theLine1End, Point theLine2Start, Point theLine2End)
{
	Point aS1;
	Point aS2;
	aS1=theLine1End-theLine1Start;
	aS2=theLine2End-theLine2Start;

	// p0=theLine1Start
	// p1=theLine1End
	// p2=theLine2Start
	// p3=theLine2End

	float aS,aT;
	aS=(-aS1.mY * (theLine1Start.mX - theLine2Start.mX) + aS1.mX * (theLine1Start.mY - theLine2Start.mY)) / (-aS2.mX * aS1.mY + aS1.mX * aS2.mY);
	aT=( aS2.mX * (theLine1Start.mY - theLine2Start.mY) - aS2.mY * (theLine1Start.mX - theLine2Start.mX)) / (-aS2.mX * aS1.mY + aS1.mX * aS2.mY);

	aS=SNAPFPS(aS);
	aT=SNAPFPS(aT);

	if (aS>=0 && aS<=1.0f) return aT;// && aT>=0 && aT<=1) return aT;
	return NAN;
}

Vector Math3D::GetPlanarLineIntersection(Plane thePlane,Vector theL1Start,Vector theL1End, Vector theL2Start, Vector theL2End)
{
	Point aL1S=thePlane.PlanifyN(theL1Start);
	Point aL1E=thePlane.PlanifyN(theL1End);
	Point aL2S=thePlane.PlanifyN(theL2Start);
	Point aL2E=thePlane.PlanifyN(theL2End);

	float aHit=gMath.GetLineIntersectionPercent(aL1S,aL1E,aL2S,aL2E);
	if (aHit>=0 && aHit<=1.0f)
	{
		Vector aDir=theL1End-theL1Start;
		Vector aNewPos=theL1Start+aDir*aHit;
		return aNewPos;
	}

	/*
	gGX.AddPlane3D(thePlane,Color(1,1,0),5);
	gGX.AddLine3D(theL1Start,theL1End,Color(1,0,0),3);
	gGX.AddLine3D(theL2Start,theL2End,Color(0,1,0),3);

	if (gMath.Distance(theL2End,theL2Start)==0) 
	{
		gGX.AddPoint3D(theL2End,Color(1,0,0),25);
		gGX.AddPoint3D(theL2Start,Color(0,1,0),15);
	}

	gOut.Out("WTF: %f %f",gMath.Distance(theL1End,theL1Start),gMath.Distance(theL2End,theL2Start));
	*/

	return NAN;
}

Point Math::MapCircleToSquare(Point theCircle)
{
	float u=theCircle.mX;
	float v=theCircle.mY;
	float u2 = u * u;
	float v2 = v * v;
	float twosqrt2 = 2.0f * sqrtf(2.0);
	float subtermx = 2.0f + u2 - v2;
	float subtermy = 2.0f - u2 + v2;
	float termx1 = subtermx + u * twosqrt2;
	float termx2 = subtermx - u * twosqrt2;
	float termy1 = subtermy + v * twosqrt2;
	float termy2 = subtermy - v * twosqrt2;
	float x = 0.5f * sqrtf(termx1) - 0.5f * sqrtf(termx2);
	float y = 0.5f * sqrtf(termy1) - 0.5f * sqrtf(termy2);
	return Point(x,y);
}

bool Math3D::GetSpherePlaneIntersection(Vector sphereCenter, float sphereRadius, Plane thePlane, Vector* theIntersection, float* theRadius)
{
	Vector aClosest=thePlane.ClosestPoint(sphereCenter);
	float aD=gMath.DistanceSquared(sphereCenter,aClosest);
	if (aD>(sphereRadius*sphereRadius)) return false;
	if (theIntersection && theRadius)
	{
		*theIntersection=aClosest;
		*theRadius=sqrtf((sphereRadius*sphereRadius)-aD);
	}
	return true;
}

float Math::GetTriangleArea(Vector v1, Vector v2, Vector v3)
{
	float aA=gMath.Distance(v1,v2);
	float aB=gMath.Distance(v2,v3);
	float aC=gMath.Distance(v3,v1);
	float aS=(aA+aB+aC)/2;
	return sqrtf(aS*(aS-aA)*(aS-aB)*(aS-aC));
}

float Math::GetTriangleArea(Point v1, Point v2, Point v3)
{
	float aA=gMath.Distance(v1,v2);
	float aB=gMath.Distance(v2,v3);
	float aC=gMath.Distance(v3,v1);
	float aS=(aA+aB+aC)/2;
	return sqrtf(aS*(aS-aA)*(aS-aB)*(aS-aC));
}

bool Math3D::DoesCylinderIntersectTriangle(Vector cylinderBase, Vector cylinderTop, float cylinderRadius, Vector triangleVertex1, Vector triangleVertex2, Vector triangleVertex3)
{
	//
	// DOES NOT WORK
	// 	   
	// 	   Theoretical: If closest point on triangle is within cylinder radius, we collide.
	//
	if (DoesLineIntersectTriangle(cylinderBase,cylinderTop,triangleVertex1,triangleVertex2,triangleVertex3)) return true;

	float aR2=cylinderRadius*cylinderRadius;
	if (ClosestLineLine(cylinderBase,cylinderTop,triangleVertex1,triangleVertex2).LengthSquared()<=aR2) return true;
	if (ClosestLineLine(cylinderBase,cylinderTop,triangleVertex2,triangleVertex3).LengthSquared()<=aR2) return true;
	if (ClosestLineLine(cylinderBase,cylinderTop,triangleVertex3,triangleVertex1).LengthSquared()<=aR2) return true;
	return false;
}

bool Math3D::GetLineCubeIntersections(Cube theCube, Vector theLine1, Vector theLine2, Array<Vector>& theResults)
{
	theResults.Reset();
	Cube aIn=theCube.Expand(.0001f);

	// Define the six faces of the cube
	float xmin = theCube.MinX(), xmax = theCube.MaxX();
	float ymin = theCube.MinY(), ymax = theCube.MaxY();
	float zmin = theCube.MinZ(), zmax = theCube.MaxZ();

	// Calculate the direction vector of the line
	float dx = theLine2.mX - theLine1.mX;
	float dy = theLine2.mY - theLine1.mY;
	float dz = theLine2.mZ - theLine1.mZ;

	// Calculate the parameters for the line equation
	float txmin = (xmin - theLine1.mX) / dx;
	float txmax = (xmax - theLine1.mX) / dx;
	float tymin = (ymin - theLine1.mY) / dy;
	float tymax = (ymax - theLine1.mY) / dy;
	float tzmin = (zmin - theLine1.mZ) / dz;
	float tzmax = (zmax - theLine1.mZ) / dz;

	if (dx==dx)
	//if (txmin <= txmax) 
	{
		if (txmin >= 0 && txmin <= 1) 
		{
			Vector aV(xmin, theLine1.mY + txmin * dy, theLine1.mZ + txmin * dz);
			if (aIn.ContainsPoint(aV)) theResults+=aV;
		}
		if (txmax >= 0 && txmax <= 1) 
		{
			Vector aV(xmax, theLine1.mY + txmax * dy, theLine1.mZ + txmax * dz);
			if (aIn.ContainsPoint(aV)) theResults+=aV;
		}
	}

	if (dy==dy)
	//if (tymin <= tymax) 
	{
		if (tymin >= 0 && tymin <= 1) 
		{
			Vector aV(theLine1.mX + tymin * dx, ymin, theLine1.mZ + tymin * dz);
			if (aIn.ContainsPoint(aV)) theResults+=aV;
		}
		if (tymax >= 0 && tymax <= 1) 
		{
			Vector aV(theLine1.mX + tymax * dx, ymax, theLine1.mZ + tymax * dz);
			if (aIn.ContainsPoint(aV)) theResults+=aV;
		}
	}

	if (dz==dz)
	//if (tzmin <= tzmax) 
	{
		if (tzmin >= 0 && tzmin <= 1) 
		{
			Vector aV(theLine1.mX + tzmin * dx, theLine1.mY + tzmin * dy, zmin);
			if (aIn.ContainsPoint(aV)) theResults+=aV;
		}
		if (tzmax >= 0 && tzmax <= 1) 
		{
			Vector aV(theLine1.mX + tzmax * dx, theLine1.mY + tzmax * dy, zmax);
			if (aIn.ContainsPoint(aV)) theResults+=aV;
		}
	}

	return theResults.Size()>0;
}

// Fast Barycentric
/*
void Barycentric(Point p, Point a, Point b, Point c, float &u, float &v, float &w)
{
	Vector v0 = b - a, v1 = c - a, v2 = p - a;
	float d00 = Dot(v0, v0);
	float d01 = Dot(v0, v1);
	float d11 = Dot(v1, v1);
	float d20 = Dot(v2, v0);
	float d21 = Dot(v2, v1);
	float denom = d00 * d11 - d01 * d01;
	v = (d11 * d20 - d01 * d21) / denom;
	w = (d00 * d21 - d01 * d20) / denom;
	u = 1.0f - v - w;
}
*/


float Math::Ease(char theEasing, float x)
{
	switch(theEasing)
	{
	case EASE_NONE:return x;
	case EASE_SIN:return -(cosf(gMath.mPI*x)-1)/2;
	case EASE_QUAD:return x < 0.5f ? 2 * x * x : 1 - powf(-2 * x + 2, 2) / 2;
	case EASE_CUBE:return x < 0.5f ? 4 * x * x * x : 1 - powf(-2 * x + 2, 3) / 2;
	case EASE_QUART:return x < 0.5 ? 8 * x * x * x * x : 1 - powf(-2 * x + 2, 4) / 2;
	case EASE_QUINT:return x < 0.5 ? 16 * x * x * x * x * x : 1 - powf(-2 * x + 2, 5) / 2;
	case EASE_EXPO:return x == 0 ? 0 : x == 1 ? 1 : x < 0.5 ? powf(2, 20 * x - 10) / 2: (2 - powf(2, -20 * x + 10)) / 2;
	case EASE_CIRC:return x < 0.5 ? (1 - sqrtf(1 - powf(2 * x, 2))) / 2 : (sqrtf(1 - powf(-2 * x + 2, 2)) + 1) / 2;
	case EASE_REBOUND:
	{
		const float c1 = 1.70158f;
		const float c2 = c1 * 1.525f;
		return x < 0.5 ? (powf(2 * x, 2) * ((c2 + 1) * 2 * x - c2)) / 2 : (powf(2 * x - 2, 2) * ((c2 + 1) * (x * 2 - 2) + c2) + 2) / 2;	
	}
	case EASE_ELASTIC:
	{
		const float c5 = (2 * gMath.mPI) / 4.5f;
		return x == 0 ? 0 : x == 1 ? 1 : x < 0.5f ? -(powf(2, 20 * x - 10) * sinf((20 * x - 11.125f) * c5)) / 2 : (powf(2, -20 * x + 10) * sinf((20 * x - 11.125f) * c5)) / 2 + 1;
	}
	case EASE_BOUNCE: return x < 0.5 ? (1 - Ease(EASE_BOUNCEOUT,1 - 2 * x)) / 2 : (1 + Ease(EASE_BOUNCEOUT,(2 * x - 1))) / 2.0f;
	case EASE_BOUNCEIN: return 1 - Ease(EASE_BOUNCEOUT,1 - x);
	case EASE_BOUNCEOUT:
	{
		const float n1 = 7.5625f;
		const float d1 = 2.75f;

		if (x < 1 / d1) {return n1 * x * x;} else if (x < 2 / d1) {return n1 * (x -= 1.5f / d1) * x + 0.75f;} else if (x < 2.5f / d1) {return n1 * (x -= 2.25f / d1) * x + 0.9375f;} else {return n1 * (x -= 2.625f / d1) * x + 0.984375f;}
	}
	}

	return x;
}

/*
float Math::GetBestSmoothCircleSides(float theRadius, float startTestAngle, float theDot)
{
	gOut.Out("---- For radius %f",theRadius);
	float aAngleStep=startTestAngle;
	for (;;)
	{
		Point aV1=(gMath.AngleToVector(aAngleStep)*theRadius)-(gMath.AngleToVector(0.0f)*theRadius);
		Point aV2=(gMath.AngleToVector(aAngleStep+aAngleStep+aAngleStep)*theRadius)-(gMath.AngleToVector(aAngleStep+aAngleStep)*theRadius);
		aV1.Normalize();
		aV2.Normalize();

		gOut.Out(" ...angle: %f",aAngleStep);

		if (aV1.Dot(aV2)>theDot) break;
		aAngleStep*=.9f;
		if (aAngleStep<.01f) break;
	}

	return (float)((int)(360.0f/aAngleStep));

}
*/

