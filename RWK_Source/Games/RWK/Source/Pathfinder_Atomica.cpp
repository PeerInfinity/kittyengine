// Pathfinder.cpp: implementation of the Pathfinder class.
//
//////////////////////////////////////////////////////////////////////

#include "Pathfinder_Atomica.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Pathfinder::Pathfinder()
{
	mPrevGrid=NULL;
	Reset();
}


Pathfinder::~Pathfinder()
{
	Reset();
}

int Pathfinder::GetColX(int theCol)
{
	return 9 + theCol*32;	
}
	
int Pathfinder::GetRowY(int theRow)
{
	return 25 + theRow*32;	
}		


bool Pathfinder::DoTraceHelper(int theStartCol, int theStartRow, int theEndCol, int theEndRow, int theNumTurns, int theMaxTurns, int theCurLen, Turn *theTurns)
{
	List aTracers;
	IPoint aTraceDir[4];
	aTraceDir[0]=gMath.GetCardinal8Direction(1);
	aTraceDir[1]=gMath.GetCardinal8Direction(3);
	aTraceDir[2]=gMath.GetCardinal8Direction(4);
	aTraceDir[3]=gMath.GetCardinal8Direction(6);
	aTracers+=&aTraceDir[0];
	aTracers+=&aTraceDir[1];
	aTracers+=&aTraceDir[2];
	aTracers+=&aTraceDir[3];
//	aTracers.Shuffle();

	//int aTraceDirs[4][2] = {{1, 0}, {0, 1}, {-1, 0}, {0, -1}};
	bool aHaveOne = false;
	
	for (int d = 0; d < 4; d++)
	{
		IPoint *aGoDir=(IPoint*)aTracers[d];
		int col = theStartCol + aGoDir->mX;
		int row = theStartRow + aGoDir->mY;
		int aThisLen = 2; // Punish extra for making turns

		while ((col >= 0) && (row >= 0) && (col<mGridWidth) && (row<mGridHeight))
		{						
			++mItrCount;
				
			// Did we find it??

			if ((col == theEndCol) && (row == theEndRow))
			{
				if (theTurns == NULL) return true;
				int aTotalLen = aThisLen + theCurLen;
				if (aTotalLen < mBestLen)
				{
					mBestNumTurns = theNumTurns;
								
					for (int i = 0; i < theMaxTurns; i++)
					{
						mBestTurns.x[i][0] = theTurns->x[i][0];
						mBestTurns.x[i][1] = theTurns->x[i][1];									
					}
							
					mBestLen = aTotalLen;
				}
				return true;
			}					
			else if ((col>=0) && (row>=0) && (col<mGridWidth) && (row<mGridHeight)) if (IsBlocked(col,row)) break;
			if (theNumTurns<theMaxTurns)
			{
				if (theTurns != NULL)
				{
					theTurns->x[theNumTurns][0] = col;
					theTurns->x[theNumTurns][1] = row;
				}
					
				unsigned int aTotalLen = theCurLen + aThisLen;
				if (GetPrevGrid(col,row)>aTotalLen)
				{
					SetPrevGrid(col,row,aTotalLen);
					if (DoTraceHelper(col, row, theEndCol, theEndRow, theNumTurns + 1, theMaxTurns, aTotalLen, theTurns))
					{
						if (theTurns == NULL) return true;
						aHaveOne = true;
					}
				}
			}				
				
			col += aGoDir->mX;
			row += aGoDir->mY;
			aThisLen++;
		}	
	}
		
	return aHaveOne;
}

bool Pathfinder::Go(int theStartCol, int theStartRow, int theEndCol, int theEndRow, Turn *theTurns, int theMaxTurns)
{
	memset(mPrevGrid,255,mGridWidth*mGridHeight*4);

	mBestLen=100000;
	mBestNumTurns=-1;
	mItrCount=0;
	if (DoTraceHelper(theStartCol, theStartRow, theEndCol, theEndRow, 0, theMaxTurns, 0, theTurns))
	{	
		if (theTurns==NULL) return true;
		if (mBestNumTurns==-1) return false;

		mTrailLength = 0;
		return true;	
	}
	else
		return false;

}

bool Pathfinder::Go(int theStartCol, int theStartRow, int theEndCol, int theEndRow)
{
	mTrailCol = -1;
	mTrailRow = -1;
	mValidMove = false;
	_FreeList(PathInfo,mTurnBlocks);

	Turn aTurns;
				
	bool aValid = Go(theStartCol,theStartRow, theEndCol, theEndRow, &aTurns, 50);
	if (mBestNumTurns<0) return false;
				
	mBestTurns.x[mBestNumTurns][0] = theEndCol;
	mBestTurns.x[mBestNumTurns][1] = theEndRow;
		
	int aLastXDir=0;
	int aLastYDir=0;		
	int aCurCol=theStartCol;
	int aCurRow=theStartRow;
	int aTargetCol=aCurCol;
	int aTargetRow=aCurRow;
	int aCurNumTurns=0;	
	bool aDone=false;
	int aXDir=0;
	int aYDir=0;
	int aLastArrowType=0;
	int aLastArrowRot=0;
		
	bool aBadMove=((theEndCol<0) || (theEndCol>=mGridWidth) || (theEndRow<0) || (theEndRow>=mGridHeight));
	if (!aBadMove) if (IsBlocked(theEndCol,theEndRow)) 
	{
		aBadMove=true;
	}
	while ((!aDone) && (!aBadMove))
	{
		if ((aCurCol == theEndCol) && (aCurRow == theEndRow)) aDone = true;
		else
		{										
			if ((aCurCol == aTargetCol) && (aCurRow == aTargetRow))
			{
				aTargetCol = mBestTurns.x[aCurNumTurns][0];
				aTargetRow = mBestTurns.x[aCurNumTurns][1];						
				++aCurNumTurns;
			}
			aXDir = 0;
			aYDir = 0;
			if (aCurCol < aTargetCol)					
				aXDir = 1;					
			else if (aCurCol > aTargetCol)					
				aXDir = -1;					
					
			if (aCurRow < aTargetRow)											
				aYDir = 1;					
			else if (aCurRow > aTargetRow)					
				aYDir = -1;					
		}

		PathInfo *aBlock=new PathInfo;
		aBlock->mPos.mX=aCurCol;
		aBlock->mPos.mY=aCurRow;

		aCurCol += aXDir;
		aCurRow += aYDir;
					
		bool turn = false;
		int aType = 0;
		int aRot = 0;
		if ((aXDir != 0) && (aLastXDir != 0)) {aType = STRAIGHT_HORIZONTAL;}
		else if ((aYDir != 0) && (aLastYDir != 0)) {aType = STRAIGHT_VERTICAL;}
		else 
		{
			turn = true;
			if (((aXDir > 0) && (aLastYDir < 0)) || ((aLastXDir < 0) && (aYDir > 0)))
			{
				aType = CORNER_UPRIGHT;
				aRot = 0;
			}
			else if (((aXDir > 0) && (aLastYDir > 0)) || ((aLastXDir < 0) && (aYDir < 0)))
			{
				aType = CORNER_DOWNRIGHT;
				aRot = 1;
			}
			else if (((aXDir < 0) && (aLastYDir < 0)) || ((aLastXDir > 0) && (aYDir > 0)))
			{
				aType = CORNER_UPLEFT;
				aRot = 3;
			}
			else if (((aXDir < 0) && (aLastYDir > 0)) || ((aLastXDir > 0) && (aYDir < 0)))
			{
				aType = CORNER_DOWNLEFT;
				aRot = 2;
			}				
		}
					
		int aArrowType=0;
		int aArrowRot=0;
		if (aXDir<0) aArrowType=ARROW_LEFT;
		else if (aXDir > 0) aArrowType=ARROW_RIGHT;
		else if (aYDir < 0)	aArrowType=ARROW_UP;
		else if (aYDir > 0)	aArrowType=ARROW_DOWN;
							
		if (aDone)
		{
			aBlock->mArrowType=aArrowType;
			aBlock->mLineType=0;
		}
		else
		{													
			aBlock->mLineType=aType;								
			aBlock->mArrowType=0;
		}
				
		if (aType!=0) mTurnBlocks.Add(aBlock);
						   
		aLastXDir=aXDir;
		aLastYDir=aYDir;
		aLastArrowType=aArrowType;
		aLastArrowRot=aArrowRot;
	}
		
	return !aBadMove;
}

void Pathfinder::Reset()
{
	mTrailCol = -1;
	mTrailRow = -1;
	mValidMove = false;
	_FreeList(PathInfo,mTurnBlocks);

	delete [] mPrevGrid;
}

bool Pathfinder::IsBlocked(int x, int y)
{
	if (GetGrid(x,y)==1) return true;
	return false;
}

