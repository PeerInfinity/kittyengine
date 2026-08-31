#include "rapt_graphics.h"
#include "rapt_utils.h"
#include "rapt.h"

Any NullAny;

Utils::Utils(void)
{
}

void Utils::AddUniquePointToList(List &theList, Point thePoint)
{
	//
	// This makes sure that when you add a point to the list, it is not repeated
	// anywhere.
	//

	EnumList(Point,aPoint,theList) if (aPoint->mX==thePoint.mX && aPoint->mY==thePoint.mY) return;
	theList+=new Point(thePoint);
}

void Utils::AddUniquePointToArray(Array<Point>& theList, Point thePoint, float theThreshold)
{
	float aT2=theThreshold*theThreshold;
	for (int aCount=0;aCount<theList.Size();aCount++) if (gMath.DistanceSquared(theList[aCount],thePoint)<aT2) return;
	theList+=thePoint;
}


void Utils::RemoveIPointFromArray(Array<IPoint>& theList, IPoint thePoint)
{
	for (int aCount=0;aCount<theList.Size();aCount++) if (theList[aCount].mX==thePoint.mX && theList[aCount].mY==thePoint.mY) 
	{
		theList.DeleteElement(aCount);
		aCount--;
	}
}

void Utils::RemovePointFromArray(Array<Point>& theList, Point thePoint)
{
	for (int aCount=0;aCount<theList.Size();aCount++) if (theList[aCount].mX==thePoint.mX && theList[aCount].mY==thePoint.mY) 
	{
		theList.DeleteElement(aCount);
		aCount--;
	}
}


bool Utils::IsPointInArray(Array<Point>& theList, Point thePoint)
{
	for (int aCount=0;aCount<theList.Size();aCount++) if (theList[aCount].mX==thePoint.mX && theList[aCount].mY==thePoint.mY) return true;
	return false;
}

bool Utils::IsIPointInArray(Array<IPoint>& theList, IPoint thePoint)
{
	for (int aCount=0;aCount<theList.Size();aCount++) if (theList[aCount].mX==thePoint.mX && theList[aCount].mY==thePoint.mY) return true;
	return false;
}


void Utils::AddUniqueIPointToArray(Array<IPoint>& theList, IPoint thePoint)
{
	for (int aCount=0;aCount<theList.Size();aCount++) if (theList[aCount].mX==thePoint.mX && theList[aCount].mY==thePoint.mY) return;
	theList+=thePoint;
}


void Utils::SortPointListByClosest(List &theList, Point theFirstPoint)
{
	//
	// This sorts points in the list so that their order will be by proximity, so that
	// you can do effects like the star match hiliting in Chuzzle
	//
	Point aFocalPoint=theFirstPoint;
	List aSortedList;
	bool aFoundOne=true;

	while (aFoundOne)
	{
		float aDistance=999999;
		Point *aBestPoint=NULL;

		aFoundOne=false;
		EnumList(Point,aPoint,theList)
		{
			bool aSkip=false;
			EnumList(Point, aCheckPoint,aSortedList)
			{
				if (aCheckPoint->mX==aPoint->mX && aCheckPoint->mY==aPoint->mY)
				{
					aSkip=true;
					break;
				}
			}
			if (aSkip) continue;

			Point aDist=*aPoint-aFocalPoint;
			if (aDist.Length()<aDistance)
			{
				aBestPoint=aPoint;
				aDistance=aDist.Length();
			}
		}

		if (aBestPoint)
		{
			aSortedList+=aBestPoint;
			aFoundOne=true;
			aFocalPoint=*aBestPoint;
		}
	}

	theList.Clear();
	theList+=aSortedList;

}

void Utils::SortPointListByClosest(List &theList, void *theObject,Point *thePointPosition, Point theFirstPoint)
{
	//
	// This sorts points in the list so that their order will be by proximity, so that
	// you can do effects like the star match hiliting in Chuzzle
	//
	Point aFocalPoint=theFirstPoint;
	List aSortedList;
	bool aFoundOne=true;

	unsigned int aPointPosition=PtrToLong(thePointPosition)-PtrToLong(theObject);

	while (aFoundOne)
	{
		float aDistance=999999;
		char *aBestObject=NULL;

		aFoundOne=false;
		EnumList(char, aObject,theList)
		{
			Point *aPoint=(Point*)(aObject+aPointPosition);

			bool aSkip=false;
			EnumList(char, aCheckObject,aSortedList)
			{
				Point *aCheckPoint=(Point*)(aCheckObject+aPointPosition);
				if (aCheckPoint->mX==aPoint->mX && aCheckPoint->mY==aPoint->mY)
				{
					aSkip=true;
					break;
				}
			}
			if (aSkip) continue;

			Point aDist=*aPoint-aFocalPoint;
			if (aDist.Length()<aDistance)
			{
				aBestObject=aObject;
				aDistance=aDist.Length();
			}
		}

		if (aBestObject)
		{
			aSortedList+=aBestObject;
			aFoundOne=true;

			Point *aPPtr=(Point*)(aBestObject+aPointPosition);
			aFocalPoint=*aPPtr;
		}
	}

	theList.Clear();
	theList+=aSortedList;

}

void Utils::SortPointListByY(List &theList, void *theObject, Point *thePointPosition)
{
	List aSortedList;

	unsigned int aOffset=PtrToLong(thePointPosition)-PtrToLong(theObject);

	EnumList(char,aPoint,theList)
	{
		char *aFindPoint=aPoint+aOffset;
		Point *aOriginalPoint=(Point*)aFindPoint;

		int aIPoint=0;
		bool aFound=false;
		EnumList(char,aCheckPoint,aSortedList)
		{
			char *aFindPoint2=aCheckPoint+aOffset;
			Point *aComparePoint=(Point*)aFindPoint2;

			if (aComparePoint->mY>aOriginalPoint->mY)
			{
				aSortedList.Insert(aPoint,aIPoint);
				aFound=true;
				break;
			}
			aIPoint++;
		}

		if (!aFound) aSortedList+=aPoint;
	}

	theList.Clear();
	theList+=aSortedList;
}

void Utils::SortPointListByX(List &theList, void *theObject, Point *thePointPosition)
{
	List aSortedList;

	unsigned int aOffset=PtrToLong(thePointPosition)-PtrToLong(theObject);

	EnumList(char,aPoint,theList)
	{
		char *aFindPoint=aPoint+aOffset;
		Point *aOriginalPoint=(Point*)aFindPoint;

		int aIPoint=0;
		bool aFound=false;
		EnumList(char,aCheckPoint,aSortedList)
		{
			char *aFindPoint2=aCheckPoint+aOffset;
			Point *aComparePoint=(Point*)aFindPoint2;

			if (aComparePoint->mX>aOriginalPoint->mX)
			{
				aSortedList.Insert(aPoint,aIPoint);
				aFound=true;
				break;
			}
			aIPoint++;
		}

		if (!aFound) aSortedList+=aPoint;
	}

	theList.Clear();
	theList+=aSortedList;
}

void Utils::SortListByZ(List &theList, void *theObject, float *theZPosition)
{
	List aSortedList;

	unsigned int aOffset=PtrToLong(theZPosition)-PtrToLong(theObject);

	EnumList(char,aZ,theList)
	{
		char *aFindZ=aZ+aOffset;
		float *aOriginalZ=(float*)aFindZ;

		int aIPoint=0;
		bool aFound=false;
		EnumList(char,aCheckZ,aSortedList)
		{
			char *aFindZ2=aCheckZ+aOffset;
			float *aCompareZ=(float*)aFindZ2;

			if (*aCompareZ>*aOriginalZ)
			{
				aSortedList.Insert(aZ,aIPoint);
				aFound=true;
				break;
			}
			aIPoint++;
		}

		if (!aFound) aSortedList+=aZ;
	}

	theList.Clear();
	theList+=aSortedList;
}

void Utils::SortListByZ(List &theList, void *theObject, int *theZPosition)
{
	List aSortedList;

	unsigned int aOffset=PtrToLong(theZPosition)-PtrToLong(theObject);

	EnumList(char,aZ,theList)
	{
		char *aFindZ=aZ+aOffset;
		float *aOriginalZ=(float*)aFindZ;

		int aIPoint=0;
		bool aFound=false;
		EnumList(char,aCheckZ,aSortedList)
		{
			char *aFindZ2=aCheckZ+aOffset;
			float *aCompareZ=(float*)aFindZ2;

			if (*aCompareZ>*aOriginalZ)
			{
				aSortedList.Insert(aZ,aIPoint);
				aFound=true;
				break;
			}
			aIPoint++;
		}

		if (!aFound) aSortedList+=aZ;
	}

	theList.Clear();
	theList+=aSortedList;
}

void Utils::SortListOfObjects(List &theList)
{
	List aSortedList;
	EnumList(Object,aO,theList)
	{
		int aIPoint=0;
		bool aFound=false;
		EnumList(Object,aCheckO,aSortedList)
		{
			if (aO->mSort>aCheckO->mSort)
			{
				aSortedList.Insert(aO,aIPoint);
				aFound=true;
				break;
			}
			aIPoint++;
		}
		if (!aFound) aSortedList+=aO;
	}

	theList.Clear();
	theList+=aSortedList;
}

unsigned int Utils::GetBitShift(unsigned int theBits)
{
	unsigned int aShift=0;
	while (!(theBits&(1<<aShift))) aShift++;
	return aShift;
}

unsigned int Utils::GetBitLength(unsigned int theBits, int theMax)
{
	unsigned int aLength=theMax;
	while (theBits)
	{
		theBits=theBits&(theBits-1);
		aLength--;
	}
	return aLength;
}

void SmartVariable::FromInt(int theValue)
{
	mInt=theValue;
	mFloat=(float)theValue;
	mBool=(theValue!=0);
	mString=Sprintf("%d",theValue);
}

void SmartVariable::FromBool(bool theValue)
{
	mInt=(int)theValue;
	mFloat=(float)theValue;
	mBool=theValue;
	mString=theValue ? "TRUE" : "FALSE";
}

void SmartVariable::FromFloat(float theValue)
{
	mInt=(int)theValue;
	mFloat=theValue;
	mBool=(theValue!=0);
	mString=Sprintf("%.2f",theValue);
}

void SmartVariable::FromString(String theValue)
{
	mInt=theValue.ToInt();
	mFloat=theValue.ToFloat();
	mBool=theValue.ToBool();
	mString=theValue;
}

void Utils::QuadsToVerts(Quad& theDrawQuad, Quad& theUVQuad, Array<Vertex2D>& theVerts)
{
	if (theVerts.Size()!=6) theVerts.Reset();
	theVerts[5].mX=0;

	theVerts[0].mX=theDrawQuad.UpperLeft().mX;
	theVerts[0].mY=theDrawQuad.UpperLeft().mY;
	theVerts[0].mZ=1;
	theVerts[0].mDiffuse=0xFFFFFFFF;
	theVerts[0].mTextureU=theUVQuad.UpperLeft().mX;
	theVerts[0].mTextureV=theUVQuad.UpperLeft().mY;

	theVerts[1].mX=theDrawQuad.UpperRight().mX;
	theVerts[1].mY=theDrawQuad.UpperRight().mY;
	theVerts[1].mZ=1;
	theVerts[1].mDiffuse=0xFFFFFFFF;
	theVerts[1].mTextureU=theUVQuad.UpperRight().mX;
	theVerts[1].mTextureV=theUVQuad.UpperRight().mY;

	theVerts[2].mX=theDrawQuad.LowerLeft().mX;
	theVerts[2].mY=theDrawQuad.LowerLeft().mY;
	theVerts[2].mZ=1;
	theVerts[2].mDiffuse=0xFFFFFFFF;
	theVerts[2].mTextureU=theUVQuad.LowerLeft().mX;
	theVerts[2].mTextureV=theUVQuad.LowerLeft().mY;

	theVerts[3].mX=theDrawQuad.UpperRight().mX;
	theVerts[3].mY=theDrawQuad.UpperRight().mY;
	theVerts[3].mZ=1;
	theVerts[3].mDiffuse=0xFFFFFFFF;
	theVerts[3].mTextureU=theUVQuad.UpperRight().mX;
	theVerts[3].mTextureV=theUVQuad.UpperRight().mY;

	theVerts[4].mX=theDrawQuad.LowerLeft().mX;
	theVerts[4].mY=theDrawQuad.LowerLeft().mY;
	theVerts[4].mZ=1;
	theVerts[4].mDiffuse=0xFFFFFFFF;
	theVerts[4].mTextureU=theUVQuad.LowerLeft().mX;
	theVerts[4].mTextureV=theUVQuad.LowerLeft().mY;

	theVerts[5].mX=theDrawQuad.LowerRight().mX;
	theVerts[5].mY=theDrawQuad.LowerRight().mY;
	theVerts[5].mZ=1;
	theVerts[5].mDiffuse=0xFFFFFFFF;
	theVerts[5].mTextureU=theUVQuad.LowerRight().mX;
	theVerts[5].mTextureV=theUVQuad.LowerRight().mY;
}

void Utils::VertsToIndexedVerts(int theVCount,Vertex2D* theVertices,Array<Vertex2D>& theResultVerts, Array<vbindex>& theResultIndices)
{
	//
	// Find all unique vertices, put them in a list.
	//
	theResultIndices.Reset();
	theResultVerts.Reset();

	theResultIndices[theVCount]=0;
	theResultVerts[theVCount].mX=0;

	int aICount=0;
	int aVCount=0;
	Vertex2D* aVertex=(theVertices);
	for (int aCount=0;aCount<theVCount;aCount++)
	{
		bool aFound=false;

		for (int aCompare=0;aCompare<aVCount;aCompare++)
		{
			if (aVertex->PrettyClose(theResultVerts[aCompare]))
			{
				theResultIndices[aICount++]=aCompare;
				aFound=true;
			}
		}
		if (!aFound)
		{
			theResultVerts[aVCount++]=*aVertex;
			theResultIndices[aICount++]=aVCount-1;
		}
		aVertex++;
	}

	theResultIndices.Clip(aICount);
	theResultVerts.Clip(aVCount);
}


void Utils::VertsToIndexedVerts(int theVCount,Vertex2DN* theVertices,Array<Vertex2DN>& theResultVerts, Array<vbindex>& theResultIndices)
{
	//
	// Find all unique vertices, put them in a list.
	//
	theResultIndices.Reset();
	theResultVerts.Reset();

	theResultIndices[theVCount]=0;
	theResultVerts[theVCount].mX=0;

	int aICount=0;
	int aVCount=0;
	Vertex2DN* aVertex=(theVertices);
	for (int aCount=0;aCount<theVCount;aCount++)
	{
		bool aFound=false;

		for (int aCompare=0;aCompare<aVCount;aCompare++)
		{
			if (aVertex->PrettyClose(theResultVerts[aCompare]))
			{
				theResultIndices[aICount++]=aCompare;
				aFound=true;
			}
		}
		if (!aFound)
		{
			theResultVerts[aVCount++]=*aVertex;
			theResultIndices[aICount++]=aVCount-1;
		}
		aVertex++;
	}

	theResultIndices.Clip(aICount);
	theResultVerts.Clip(aVCount);
}


void Utils::VertsToIndexedVerts(int theVCount,Vertex2DMT* theVertices,Array<Vertex2DMT>& theResultVerts, Array<vbindex>& theResultIndices)
{
	//
	// Find all unique vertices, put them in a list.
	//
	theResultIndices.Reset();
	theResultVerts.Reset();

	theResultIndices[theVCount]=0;
	theResultVerts[theVCount].mX=0;

	int aICount=0;
	int aVCount=0;
	Vertex2DMT* aVertex=theVertices;
	for (int aCount=0;aCount<theVCount;aCount++)
	{
		bool aFound=false;

		//
		// Improve speed maybe: Insert verts into a grid, so we only need to check the grid in question?
		// Will need to associate the verts with an index position if so...
		//
		for (int aCompare=0;aCompare<aVCount;aCompare++)
		{
			if (aVertex->PrettyClose(theResultVerts[aCompare]))
			{
				theResultIndices[aICount++]=aCompare;
				aFound=true;
			}
		}
		if (!aFound)
		{
			theResultVerts[aVCount++]=*aVertex;
			theResultIndices[aICount++]=aVCount-1;
		}
		aVertex++;
	}

	theResultIndices.Clip(aICount);
	theResultVerts.Clip(aVCount);
}

void Utils::VertsToIndexedVertsInVolume(Cube theBound, int theVCount,Vertex2DMT* theVertices,Array<Vertex2DMT>& theResultVerts, Array<vbindex>& theResultIndices)
{

	PointerVolume aVertVolume;
	aVertVolume.Go(theBound,2);
	struct Vert
	{
		Vertex2DMT*		mV;
		int				mI;
	};
	Vert* aVertList=new Vert[theVCount];
	int aVertCount=0;

	theResultIndices.Reset();
	theResultVerts.Reset();

	theResultIndices[theVCount]=0;
	theResultVerts[theVCount].mX=0;

	int aICount=0;
	int aVCount=0;

	Vertex2DMT* aVertex=theVertices;

	for (int aCount=0;aCount<theVCount;aCount++)
	{
		bool aFound=false;

		List& aOldList=aVertVolume.GetList(aVertex->ToVector());
		EnumList(Vert,aVert,aOldList)
		{
			if (*aVertex==*aVert->mV)
			{
				theResultIndices[aICount++]=aVert->mI;
				aFound=true;
			}
		}
		if (!aFound)
		{
			theResultVerts[aVCount++]=*aVertex;
			theResultIndices[aICount++]=aVCount-1;

			aVertList[aVertCount].mV=aVertex;
			aVertList[aVertCount].mI=aVCount-1;
			aVertVolume.Add(aVertex->ToVector(),&aVertList[aVertCount]);

			aVertCount++;
		}

		aVertex++;
	}

	theResultIndices.Clip(aICount);
	theResultVerts.Clip(aVCount);

	//delete [] aVertList;
}

int Utils::PickStringToInt(String myString, String theList, int theDefault)
{
	int aResult=theDefault;
	int aCounter=0;

	String aToken=theList.GetToken('|');
	while (!aToken.IsNull())
	{
		String aBracket=aToken.GetBracedToken('[',']',true);
		if (aBracket.Len()) aToken.Delete(aBracket);
		if (myString==aToken) 
		{
			if (aBracket.Len()>0) {aResult=aBracket.GetSegment(1,aBracket.Len()-2).ToInt();break;}
			else {aResult=aCounter;break;}
		}

		aCounter++;
		aToken=theList.GetNextToken('|');
	}

	return aResult;
}

void Utils::NumberStringToIntArray(String myString, Array<int>& theArray)
{
	//1,3-5,7,9-12
	String aToken=myString.GetToken(',');
	while (!aToken.IsNull())
	{
		if (aToken[0]<=32) aToken.RemoveLeadingSpaces();
		if (aToken[0]!='-' && aToken.Contains('-'))
		{
			String aT1=aToken.GetToken('-');
			String aT2=aToken.GetNextToken('-');

			for (int aCount=aT1.ToInt();aCount<=aT2.ToInt();aCount++) theArray+=aCount;
		}
		else theArray+=aToken.ToInt();

		aToken=myString.CGetNextToken(',');
	}
}

void Utils::NumberStringToShortArray(String myString, Array<short>& theArray)
{
	//1,3-5,7,9-12
	String aToken=myString.GetToken(',');
	while (!aToken.IsNull())
	{
		if (aToken[0]<=32) aToken.RemoveLeadingSpaces();
		if (aToken[0]!='-' && aToken.Contains('-'))
		{
			String aT1=aToken.GetToken('-');
			String aT2=aToken.GetNextToken('-');

			for (int aCount=aT1.ToInt();aCount<=aT2.ToInt();aCount++) theArray+=(short)aCount;
		}
		else theArray+=(short)aToken.ToInt();

		aToken=myString.CGetNextToken(',');
	}
}

int Utils::NumberStringToRandomInt(String myString)
{
	Array<int> aArray;
	NumberStringToIntArray(myString,aArray);
	return aArray[gRand.Get(aArray.Size())];
}

GlobalID GetGlobalID(Random& theRand)
{
	GlobalID aGUID;
	for (int aCount=0;aCount<16;aCount++) aGUID.mDataBytes[aCount]=(unsigned char)theRand.Get(256);
	return aGUID;
}

GlobalID GetNullGlobalID()
{
	GlobalID aGUID;
	aGUID.Null();
	return aGUID;
}

GlobalID GetInvalidGlobalID()
{
	GlobalID aGUID;
	aGUID.Invalidate();
	return aGUID;
}


void GlobalID::FromString(String theString)
{
	unsigned int p0;
	int p1, p2, p3, p4, p5, p6, p7, p8, p9, p10;
	const int aRet=sscanf(theString.c(),"%8X-%4X-%4X-%2X%2X-%2X%2X%2X%2X%2X%2X",&p0, &p1, &p2, &p3, &p4, &p5, &p6, &p7, &p8, &p9, &p10);
	mGuid.mData1 = p0;
	mGuid.mData2 = p1;
	mGuid.mData3 = p2;
	mGuid.mData4[0] = p3;
	mGuid.mData4[1] = p4;
	mGuid.mData4[2] = p5;
	mGuid.mData4[3] = p6;
	mGuid.mData4[4] = p7;
	mGuid.mData4[5] = p8;
	mGuid.mData4[6] = p9;
	mGuid.mData4[7] = p10;

	if (aRet!=11)  gOut.Out("Invalid GUID, format should be {00000000-0000-0000-0000-000000000000}");
}

String GlobalID::ToString()
{
	char aResult[80];
	sprintf(aResult,"%08X-%04hX-%04hX-%02X%02X-%02X%02X%02X%02X%02X%02X", mGuid.mData1, mGuid.mData2, mGuid.mData3, mGuid.mData4[0], mGuid.mData4[1], mGuid.mData4[2], mGuid.mData4[3], mGuid.mData4[4], mGuid.mData4[5], mGuid.mData4[6], mGuid.mData4[7]);
	return String(aResult);
}


String GlobalID::ToHex()
{
	char aResult[40];
	unsigned char *aRunner = (unsigned char *)this->mData;
	for (int i=0;i<16;i++)  sprintf(&aResult[i*2],"%02x", aRunner[i]);
	return String(aResult);
}

void PointerVolume::Go(Cube theVolume, float theSubVolumeSize, bool useUnique)
{
	theVolume=theVolume.Expand(.1f);

	mVolume=theVolume;

	if (fmod(theVolume.mXSize,theSubVolumeSize)>0) theVolume.mXSize+=theSubVolumeSize;
	if (fmod(theVolume.mYSize,theSubVolumeSize)>0) theVolume.mYSize+=theSubVolumeSize;
	if (fmod(theVolume.mZSize,theSubVolumeSize)>0) theVolume.mZSize+=theSubVolumeSize;

	mSubVolumeSize=theSubVolumeSize;
	mXSize=(int)(theVolume.mXSize/theSubVolumeSize);
	mYSize=(int)(theVolume.mYSize/theSubVolumeSize);
	mZSize=(int)(theVolume.mZSize/theSubVolumeSize);

	mListGrid[mXSize*mYSize*mZSize].GetCount();
	if (useUnique) for (int aCount=0;aCount<mListGrid.Size();aCount++) mListGrid[aCount].Unique();
}

void PointerVolume::Add(Vector thePos, void* thePointer)
{
	if (!mVolume.ContainsPoint(thePos)) 
	{
		//gOut.Out("Out of Volume: %f,%f,%f",thePos.mX,thePos.mY,thePos.mZ);
		return;
	}
	thePos-=mVolume.Corner(0);
	thePos/=mSubVolumeSize;

	int aPos=PosToGrid(thePos);
	mListGrid[aPos]+=thePointer;
}

List& PointerVolume::GetList(Vector thePos)
{
	if (!mVolume.ContainsPoint(thePos)) return mNullList;
	thePos-=mVolume.Corner(0);
	thePos/=mSubVolumeSize;
	int aPos=PosToGrid(thePos);

	return mListGrid[aPos];
}

#include "graphics_core.h"
void Utils::ImageToMask(Image& theImage, String theOutFile)
{
	IOBuffer aOut;

	aOut.WriteLine("Image* aImagePtr;");
	for (int aSpanX=0;aSpanX<theImage.mWidth;aSpanX++)
	{
		for (int aSpanY=0;aSpanY<theImage.mHeight;aSpanY++)
		{
			RGBA* aPix=theImage.GetPixelPtr(aSpanX,aSpanY);
			if (aPix->mA<255) aOut.WriteLine(Sprintf("aImagePtr->GetPixelPtr(%d,%d)->mA=%d;",aSpanX,aSpanY,aPix->mA));
		}
	}




	aOut.CommitFile(theOutFile);
}

void Stopwatch::Start() {mTick=gAppPtr->Tick();}
int	 Stopwatch::Stop() {return gAppPtr->Tick()-mTick;}

void Utils::SortIPointArrayByX(Array<IPoint>& theArray)
{
	bool aChanged=true;
	while (aChanged)
	{
		aChanged=false;
		for (int aCount=0;aCount<theArray.Size()-1;aCount++)
		{
			if (theArray[aCount+1].mX<theArray[aCount].mX)
			{
				IPoint aHold=theArray[aCount+1];
				theArray[aCount+1]=theArray[aCount];
				theArray[aCount]=aHold;
				aChanged=true;
			}
		}
	}
}

void Utils::SortIPointArrayByY(Array<IPoint>& theArray)
{
	bool aChanged=true;
	while (aChanged)
	{
		aChanged=false;
		for (int aCount=0;aCount<theArray.Size()-1;aCount++)
		{
			if (theArray[aCount+1].mY<theArray[aCount].mY)
			{
				IPoint aHold=theArray[aCount+1];
				theArray[aCount+1]=theArray[aCount];
				theArray[aCount]=aHold;
				aChanged=true;
			}
		}
	}
}

void Utils::CSVToArray(String theString, Array<String>& theArray, char theSeperator)
{
	String aToken=theString.GetCodeToken(theSeperator);
	if (!aToken.IsNull() || !theString.IsCursorAtEnd())
	for(;;)
	{
		String aGot=aToken;
		theArray+=aGot;
		aToken=theString.GetNextCodeToken(theSeperator);

		if (aToken.IsNull() && theString.IsCursorAtEnd()) break;
	}
}

void Utils::CSVToArray(String theString, Array<int>& theArray, char theSeperator)
{
	String aToken=theString.GetToken(theSeperator);
	if (!aToken.IsNull() || !theString.IsCursorAtEnd())
	for(;;)
	{
		String aGot=aToken;
		theArray+=aGot.ToInt();
		aToken=theString.GetNextToken(theSeperator);

		if (aToken.IsNull() && theString.IsCursorAtEnd()) break;
	}
}

void Utils::CSVToArray(String theString, Array<char>& theArray, char theSeperator)
{
	String aToken=theString.GetToken(theSeperator);
	if (!aToken.IsNull() || !theString.IsCursorAtEnd())
	for(;;)
	{
		String aGot=aToken;
		theArray+=(char)aGot.ToInt();
		aToken=theString.GetNextToken(theSeperator);

		if (aToken.IsNull() && theString.IsCursorAtEnd()) break;
	}
}

void Utils::CSVToArray(String theString, Array<bool>& theArray, char theSeperator)
{
	String aToken=theString.GetToken(theSeperator);
	if (!aToken.IsNull() || !theString.IsCursorAtEnd())
		for(;;)
		{
			String aGot=aToken;
			theArray+=(aGot.ToInt()!=0);
			aToken=theString.GetNextToken(theSeperator);
			if (aToken.IsNull() && theString.IsCursorAtEnd()) break;
		}
}

String Utils::ArrayToCSV(Array<String>& theArray, char theSeperator)
{
	String aResult;
	for (int aCount=0;aCount<theArray.Size();aCount++)
	{
		if (aCount>0) aResult+=theSeperator;
		aResult+=theArray[aCount];
	}

	return aResult;
}

String Utils::ArrayToCSV(Array<int>& theArray, char theSeperator)
{
	String aResult;
	for (int aCount=0;aCount<theArray.Size();aCount++)
	{
		if (aCount>0) aResult+=theSeperator;
		aResult+=Sprintf("%d",theArray[aCount]);
	}

	return aResult;
}

String Utils::ArrayToCSV(Array<short>& theArray, char theSeperator)
{
	String aResult;
	for (int aCount=0;aCount<theArray.Size();aCount++)
	{
		if (aCount>0) aResult+=theSeperator;
		aResult+=Sprintf("%d",theArray[aCount]);
	}

	return aResult;
}

String Utils::ArrayToCSV(Array<char>& theArray, char theSeperator)
{
	String aResult;
	for (int aCount=0;aCount<theArray.Size();aCount++)
	{
		if (aCount>0) aResult+=theSeperator;
		aResult+=Sprintf("%d",theArray[aCount]);
	}

	return aResult;
}

String Utils::ArrayToCSV(Array<bool>& theArray, char theSeperator)
{
	String aResult;
	for (int aCount=0;aCount<theArray.Size();aCount++)
	{
		if (aCount>0) aResult+=theSeperator;
		aResult+=Sprintf("%d",theArray[aCount]);
	}

	return aResult;
}


void Utils::CSVToSmartList(String theString, SmartList(String)& theArray, char theSeperator)
{
	String aToken=theString.GetToken(theSeperator);
	for(;;)
	{
		String aGot=aToken;
		Smart(String) aStr=new String(aGot);
		theArray+=aStr;
		aToken=theString.GetNextToken(theSeperator);

		if (aToken.IsNull() && theString.IsCursorAtEnd()) break;
	}
}

String Utils::SmartListToCSV(SmartList(String)& theArray, char theSeperator)
{
	String aResult;
	bool aFirst=true;
	EnumSmartList(String,aStr,theArray)
	{
		if (!aFirst) {aResult+=theSeperator;}
		aResult+=aStr->c();
		aFirst=false;
	}

	return aResult;
}

int Utils::DateToInt(int theMonth, int theDay, int theYear)
{
	const int aMonthDays[12] = {31, 28, 31, 30, 31, 30,31, 31, 30, 31, 30, 31};

	long int aResult=theYear*365+theDay;
	for (int aCount=0;aCount<theMonth-1;aCount++) aResult+=aMonthDays[aCount];
	{
		int aYears=theYear;
		if (theMonth<=2) aYears--;
		aResult+=(aYears/4-aYears/100+aYears/400);
	}
	return aResult;
}

ColorCycler::ColorCycler(void)
{
	mSpanCount=0;
	mDidGenerate=0;
}

ColorCycler::~ColorCycler(void)
{
}

void ColorCycler::AddColor(Color theColor, float theSpan)
{
	if (mDidGenerate)
	{
		//
		// Remove the last color (since it's been added to be color #1)
		//
		mDidGenerate=false;
		mList-=mList.FetchLast();
	}

	struct SpanStruct *aSpan=new SpanStruct;
	aSpan->mColor=theColor;
	aSpan->mSpan=mSpanCount;
	mList+=aSpan;

	mSpanCount+=theSpan;
}

void ColorCycler::Generate()
{
	struct SpanStruct *aS=(struct SpanStruct *)mList[0];

	struct SpanStruct *aSpan=new SpanStruct;
	aSpan->mColor=aS->mColor;
	aSpan->mSpan=mSpanCount;
	mList+=aSpan;

	mDidGenerate=true;
}

Color ColorCycler::Get(float theValue)
{
	if (!mDidGenerate) Generate();
	if (mList.GetCount()==0) return Color(0,0,0,0);
	if (theValue>GetCount())
	{
		int aMod=(int)theValue/GetCount();
		aMod*=GetCount();
		theValue-=(float)aMod;
	}

	if (theValue<0)
	{
		struct SpanStruct *aSpan=(struct SpanStruct*)mList[0];
		return aSpan->mColor;
	}

	if (theValue>=mList.GetCount())
	{
		struct SpanStruct *aSpan=(struct SpanStruct*)mList.FetchLast();
		return aSpan->mColor;
	}

	//
	// Move forward until mSpan>=theValue
	// Then, figure out the difference between the two spans in question,
	// convert the value into that scale, and mix colors from that.
	//
	for (int aCount=mList.GetCount()-1;aCount>=0;aCount--)
	{
		struct SpanStruct *aFirstSpan=(struct SpanStruct *)mList[aCount];
		if (theValue>=aFirstSpan->mSpan)
		{
			struct SpanStruct *aNextSpan=(struct SpanStruct*)mList[aCount+1];
			if (!aNextSpan)
			{
				struct SpanStruct *aSpan=(struct SpanStruct*)mList.FetchLast();
				return aSpan->mColor;
			}

			//
			// Figure out the scale of our span
			//
			float aScale=aNextSpan->mSpan-aFirstSpan->mSpan;

			//
			// Here's the amount that our requested value is over 0
			//
			float aFloor=floorf(theValue);
			float aFraction=theValue-aFloor;

			//
			// Our adjusted fraction
			//
			aFraction/=aScale;

			float aColor1=1.0f-_min(aFraction,1.0f);
			float aColor2=_min(aFraction,1.0f);

			Color aC1=aFirstSpan->mColor;
			Color aC2=aNextSpan->mColor;

			aC1*=aColor1;
			aC2*=aColor2;

			Color aNewColor;
			aNewColor.mR=_min(1.0f,aC1.mR+aC2.mR);
			aNewColor.mG=_min(1.0f,aC1.mG+aC2.mG);
			aNewColor.mB=_min(1.0f,aC1.mB+aC2.mB);
			aNewColor.mA=_min(1.0f,aC1.mA+aC2.mA);

			return aNewColor;
		}
	}

	return Color(0,0,0,0);
}

int QuicksortPartitionObjects(List& theList, int theP, int theQ)
{
	Object* aO=(Object*)theList.Fetch(theP);int aX=aO->mSort;int aI=theP;
	for(int aJ=theP+1;aJ<theQ;aJ++) {aO=(Object*)theList.Fetch(aJ);if(aO->mSort<aX) {aI=aI+1;theList.Swap(aI,aJ);}}
	theList.Swap(aI,theP);return aI;
}

void Utils::QuicksortObjects(List& theList, int theStart, int theEnd)
{
	if (theEnd==-1) theEnd=theList.GetCount();
	if (theStart<theEnd)
	{
		int aR=QuicksortPartitionObjects(theList,theStart,theEnd);
		QuicksortObjects(theList,theStart,aR);
		QuicksortObjects(theList,aR+1,theEnd);
	}
}

int QuicksortPartitionObjectsReverse(List& theList, int theP, int theQ)
{
	Object* aO=(Object*)theList.Fetch(theP);int aX=aO->mSort;int aI=theP;
	for(int aJ=theP+1;aJ<theQ;aJ++) {aO=(Object*)theList.Fetch(aJ);if(aO->mSort>aX) {aI=aI+1;theList.Swap(aI,aJ);}}
	theList.Swap(aI,theP);return aI;
}

void Utils::QuicksortObjectsReverse(List& theList, int theStart, int theEnd)
{
	if (theEnd==-1) theEnd=theList.GetCount();
	if (theStart<theEnd)
	{
		int aR=QuicksortPartitionObjectsReverse(theList,theStart,theEnd);
		QuicksortObjectsReverse(theList,theStart,aR);
		QuicksortObjectsReverse(theList,aR+1,theEnd);
	}
}

int QuicksortPartitionObjects(SmartList(Object)* theList, int theP, int theQ)
{
	int aX=theList->Fetch(theP)->mSort;int aI=theP;
	for(int aJ=theP+1;aJ<theQ;aJ++) {if(theList->Fetch(aJ)->mSort<aX) {aI=aI+1;theList->Swap(aI,aJ);}}
	theList->Swap(aI,theP);return aI;
}

void Utils::QuicksortObjects(SmartList(Object)* theList, int theStart, int theEnd)
{
	if (theEnd==-1) theEnd=theList->GetCount();
	if (theStart<theEnd)
	{
		int aR=QuicksortPartitionObjects(theList,theStart,theEnd);
		QuicksortObjects(theList,theStart,aR);
		QuicksortObjects(theList,aR+1,theEnd);
	}
}

int QuicksortPartitionObjectsReverse(SmartList(Object)* theList, int theP, int theQ)
{
	int aX=theList->Fetch(theP)->mSort;int aI=theP;
	for(int aJ=theP+1;aJ<theQ;aJ++) {if(theList->Fetch(aJ)->mSort>aX) {aI=aI+1;theList->Swap(aI,aJ);}}
	theList->Swap(aI,theP);return aI;
}

void Utils::QuicksortObjectsReverse(SmartList(Object)* theList, int theStart, int theEnd)
{
	if (theEnd==-1) theEnd=theList->GetCount();
	if (theStart<theEnd)
	{
		int aR=QuicksortPartitionObjectsReverse(theList,theStart,theEnd);
		QuicksortObjectsReverse(theList,theStart,aR);
		QuicksortObjectsReverse(theList,aR+1,theEnd);
	}
}

int AlphabetizePartition(Array<char*>& theList,int theP,int theQ)
{
	char* aX=theList.Element(theP);
	int aI=theP;
	for(int aJ=theP+1;aJ<theQ;aJ++)
	{
#ifdef _WIN32
		if(strcmpi(theList.Element(aJ),aX)<0)
#else
		if(strcasecmp(theList.Element(aJ),aX)<0)
#endif
		{
			aI=aI+1;
			theList.Swap(aI,aJ);
		}
	}
	theList.Swap(aI,theP);
	return aI;
}

void Utils::Alphabetize(Array<char*>& theList,int theStart,int theEnd)
{
	if(theEnd==-1) theEnd=theList.Size();
	if(theStart<theEnd)
	{
		int aR=AlphabetizePartition(theList,theStart,theEnd);
		Alphabetize(theList,theStart,aR);
		Alphabetize(theList,aR+1,theEnd);
	}
}

int AlphabetizePartition(Array<String> &theList,int theP,int theQ)
{
	char* aX=theList.Element(theP).c();
	int aI=theP;
	for(int aJ=theP+1;aJ<theQ;aJ++)
	{
		#ifdef _WIN32
		if(strcmpi(theList.Element(aJ).c(),aX)<0)
		#else
		if(strcasecmp(theList.Element(aJ).c(),aX)<0)
		#endif
		{
			aI=aI+1;
			theList.Swap(aI,aJ);
		}
	}
	theList.Swap(aI,theP);
	return aI;
}

void Utils::Alphabetize(Array<String> &theList,int theStart,int theEnd)
{
	if(theEnd==-1) theEnd=theList.Size();
	if(theStart<theEnd)
	{
		int aR=AlphabetizePartition(theList,theStart,theEnd);
		Alphabetize(theList,theStart,aR);
		Alphabetize(theList,aR+1,theEnd);
	}
}


float Utils::GetFlicker(int theTimeBetween, int theFlickerLength, int theTick)
{
	if (theTick==-1) theTick=gAppPtr->GetAppTime();

	int aGT=((theTick)%(theTimeBetween+theFlickerLength));
	if (aGT>theTimeBetween)
	{
		aGT-=theTimeBetween;
		return (gMath.Sin(((float)aGT/(float)theFlickerLength)*180));
	}
	return 0;
}

void Dictionary::Load(String theFilename)
{
	IOBuffer aBuffer;
	aBuffer.Load(theFilename);
	int aLowest1=INT_MAX;
	int aLowest2=INT_MAX;
	int aHighest1=0;
	int aHighest2=0;
	while (!aBuffer.IsEnd()) 
	{
		String aLine=aBuffer.ReadLine();
		if (aLine.Len()>=2) 
		{
			aLine.Uppercase();
			mWords+=new String(aLine);
			aLowest1=_min(aLowest1,aLine[0]);
			aLowest2=_min(aLowest2,aLine[1]);
			aHighest1=_max(aHighest1,aLine[0]);
			aHighest2=_max(aHighest2,aLine[1]);
		}
	}

	int aRange1=(aHighest1-aLowest1)+2;
	int aRange2=(aHighest2-aLowest2)+2;
	mLookupBase[0]=aLowest1-1;
	mLookupBase[1]=aLowest2-1;
	
	mLookup.GuaranteeSize(aRange1);
	mLookup.mLockSize=true;
	for (int aCount=0;aCount<aRange1;aCount++)
	{
		mLookup[aCount].GuaranteeSize(aRange2);
		mLookup[aCount].mLockSize=true;
		for (int aCount2=0;aCount2<aRange2;aCount2++) mLookup[aCount][aCount2]=-1;
	}

	int aSlot=0;
	EnumSmartList(String,aS,mWords)
	{
		String aSS=*aS;
		int aSlot1=aSS[0]-mLookupBase[0];
		int aSlot2=aSS[1]-mLookupBase[1];
		if (mLookup[aSlot1][aSlot2].mX==-1) mLookup[aSlot1][aSlot2].mX=aSlot;
		mLookup[aSlot1][aSlot2].mY=aSlot;
		aSlot++;
	}

	mLoaded=true;

	if (mWords.GetCount()==0) gOut.Out("!!!! Failed to load dictionary [%s]",theFilename.c());
}

bool Dictionary::Exists(String theWord)
{
	if (!mLoaded || theWord.Len()<2) return false;

	int aSlot1=toupper(theWord[0])-mLookupBase[0];
	int aSlot2=toupper(theWord[1])-mLookupBase[1];
	if (aSlot1<0 || aSlot1>=mLookup.Size()) return false;
	if (aSlot2<0 || aSlot2>=mLookup[aSlot1].Size()) return false;

	IPoint& aRange=mLookup[aSlot1][aSlot2];
	if (aRange.mX>=0) for (int aCount=aRange.mX;aCount<=aRange.mY;aCount++) if (theWord==*mWords[aCount]) return true;
	return false;
}

int Dictionary::Exists(char* thePtr, int theLen)
{
	if (!mLoaded) return 0;

	int aSlot1=toupper(thePtr[0])-mLookupBase[0];
	int aSlot2=toupper(thePtr[1])-mLookupBase[1];
	if (aSlot1<0 || aSlot1>=mLookup.Size()) return 0;
	if (aSlot2<0 || aSlot2>=mLookup[aSlot1].Size()) return 0;

	IPoint& aRange=mLookup[aSlot1][aSlot2];
	if (aRange.mX<0) return 0;
	for (int aCount=aRange.mX;aCount<=aRange.mY;aCount++) 
	{
		int aLen=theLen;
		if (aLen==-1) aLen=mWords[aCount]->Len();
		if (mWords[aCount]->Len()==aLen && mWords[aCount]->strnicmp(thePtr,aLen)) return mWords[aCount]->Len();
	}
	return 0;
}

float GetGlint(int theTick, int theDelay, int theDuration)
{
	int aGT=(theTick%theDelay);
	if (aGT>(theDelay-theDuration))
	{
		aGT-=(theDelay-theDuration);
		return (gMath.Sin(((float)aGT/(float)theDuration)*180));
	}
	return 0;
}

int gDebugTimerTick=0;
String gDebugTimerString;

longlong Utils::Hash(void* theData, int theLen)
{
	int* aDPtr=(int*)theData;
	/*
	longlong aResult=0;
	for (int aCount=0;aCount<theLen;aCount+=4) {aResult=*aDPtr+(aResult<<6)+(aResult<<16)-aResult;aDPtr++;}
	//for (int aCount=0;aCount<theLen;aCount+=4) {aResult=(aResult<<5)+(aResult)+*aDPtr;aDPtr++;}
	return aResult;
	*/

	longlong aS1=1;
	longlong aS2=0;
	for (int aCount=0;aCount<theLen;aCount+=4)
	{
		aS1=(aS1+*aDPtr)%65521;
		aS2=(aS2+aS1)%65521;
		aDPtr++;
	}
	return (aS2<<16)|aS1;
}



//////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Cloud queuing stuff... to make sure we don't dump to the cloud every time we tweak settings
// or stuff like that.
//
//////////////////////////////////////////////////////////////////////////////////////////////////////
namespace CloudQueue
{
	bool gEnabled=false;

	bool gQueueSending=false;
	class CloudQueue : public CPU
	{
	public:
		struct QueuedItem
		{
			longlong mHash;
			String mKey;
			IOBuffer mBuffer;
		};
		List mItemList;
		int mCountdown=500;

		~CloudQueue() {_FreeList(QueuedItem,mItemList);}
		void Update() {if (gQueueSending) mCountdown=500;if (--mCountdown<=0) {Dispatch(true);}}
		QueuedItem* Push(String& theString,IOBuffer* theBuffer)
		{
			mCountdown=_max(100,mCountdown); // Always add some time to cloudqueue, so that we're always waiting a bit after files get pushed...

			longlong aHash=theString.GetHash();
			EnumList(QueuedItem,aQI,mItemList) {if (aQI->mHash==aHash) if (aQI->mKey==theString) {aQI->mBuffer.Copy(theBuffer);return aQI;}}

			QueuedItem* aNewQ=new QueuedItem;
			aNewQ->mKey=theString;
			aNewQ->mHash=theString.GetHash();
			aNewQ->mBuffer.Copy(theBuffer);
			mItemList+=aNewQ;
			return aNewQ;
		}

	};
	CloudQueue* gCloudQueue=NULL;

	void Disable() {gEnabled=false;if (gCloudQueue) gCloudQueue->Pause();}
	void Enable() {gEnabled=true;if (gCloudQueue) gCloudQueue->UnpauseAll();}


	void Put(String& theName,IOBuffer* theBuffer)
	{
		if (!gEnabled) return;
		if (!gAppPtr) return;
		if (!gCloudQueue) {gCloudQueue=new CloudQueue;gAppPtr->mBackgroundProcesses+=gCloudQueue;gOut.Debug("*CloudQueue...now pending!");}
		gCloudQueue->Push(theName,theBuffer);
	}

	void SendCloudQueue(void* theArg)
	{
		if (!gEnabled) return;
		if (!theArg) return;
		if (!gAppPtr) return;

		//
		// Was removed from BackgroundPorcesses in Dispatch.... theArg is now completely freestanding...
		//

		gQueueSending=true;
		CloudQueue* aQueue=(CloudQueue*)theArg;
		EnumList(CloudQueue::QueuedItem,aQ,aQueue->mItemList) {OS_Core::PutCloud(aQ->mKey.c(),aQ->mBuffer.mData,aQ->mBuffer.mDataLen);OS_Core::Sleep(2);}
		delete aQueue;

		gQueueSending=false;
		gOut.Debug("*CloudQueue...finished dispatching!");
	}

	void Dispatch(bool doThreaded)
	{
		if (!gEnabled) return;
		if (!gAppPtr) return;
		if (gCloudQueue)
		{
			gOut.Debug("*CloudQueue...dispatching!");

			CloudQueue* aQ=gCloudQueue;
			gAppPtr->mBackgroundProcesses-=gCloudQueue;
			gCloudQueue=NULL;

#ifndef NO_THREADS
			if (doThreaded) Thread(SendCloudQueue,(void*)aQ);
			else 
#endif
				SendCloudQueue(aQ);
		}
	}

	void Kill()
	{
		if (gCloudQueue)
		{
			gOut.Debug("*CloudQueue...killed!");
			gAppPtr->mBackgroundProcesses-=gCloudQueue;
			delete gCloudQueue;
			gCloudQueue=NULL;
		}
	}

}

void Utils::GetLineIntersections(Array<Line> &theLines, Array<Point>& theResults)
{
	int aPut=0;
	theResults.GuaranteeSize(theLines.Size()*2);

	Point aHit;
	foreach(aL1,theLines) foreach(aL2,theLines) {if (&aL1==&aL2) continue;if (gMath.GetLineIntersection(aL1,aL2,&aHit)) theResults[aPut++]=aHit;}
	theResults.Clip(aPut);
}

void Utils::GetLineIntersections(Line theKeyLine, Array<Line> &theLines, Array<Point>& theResults)
{
	int aPut=0;
	theResults.GuaranteeSize(theLines.Size()*2);

	Point aHit;
	foreach(aL1,theLines) if (gMath.GetLineIntersection(aL1,theKeyLine,&aHit)) theResults[aPut++]=aHit;
	theResults.Clip(aPut);
}

