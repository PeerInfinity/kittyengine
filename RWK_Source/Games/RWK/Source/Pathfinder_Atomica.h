// Pathfinder.h: interface for the Pathfinder class.
//
//////////////////////////////////////////////////////////////////////
#include "rapt.h"


#define STRAIGHT_HORIZONTAL					1
#define STRAIGHT_VERTICAL					2
#define CORNER_UPLEFT						3
#define CORNER_UPRIGHT						4
#define CORNER_DOWNLEFT						5
#define CORNER_DOWNRIGHT					6

#define ARROW_UP							1
#define ARROW_DOWN							2
#define ARROW_LEFT							3
#define ARROW_RIGHT							4

class Pathfinder
{
public:
	Pathfinder();
	virtual ~Pathfinder();

public:

	struct Turn
	{
	public:
		int x[50][2];
	};

	struct PathInfo
	{
	public:
		IPoint			mPos;
		int				mLineType;
		int				mArrowType;
	};


	bool			Go(int theStartCol, int theStartRow, int theEndCol, int theEndRow);
	bool			Go(int theStartCol, int theStartRow, int theEndCol, int theEndRow, Turn *theTurns, int theMaxTurns);
	void			Reset();


	bool			DoTraceHelper(int theStartCol, int theStartRow, int theEndCol, int theEndRow, int theNumTurns, int theMaxTurns, int theCurLen, Turn *theTurns);

	int				GetColX(int theCol);
	int				GetRowY(int theRow);

public:
	//
	// Grid info... grid is OUTSIDE the pathfinder, has
	// to be allocated elsewhere.
	//
	inline void				HookToGrid(char *theGrid, int theWidth, int theHeight)
	{
		Reset();
		mGrid=theGrid;
		mGridWidth=theWidth;
		mGridHeight=theHeight;

		mPrevGrid=new unsigned int[theWidth*theHeight];
	}
	inline char				GetGrid(int x, int y) {return mGrid[x+(y*mGridWidth)];}

	int						mGridWidth;
	int						mGridHeight;
	char					*mGrid;

	List					mTurnBlocks;	// Contains info for each block, like which way to turn, etc.

	List					&GetPath() {return mTurnBlocks;}

	int						mTrailCol;
	int						mTrailRow;
	bool					mValidMove;	
	int						mLastTrailTick;

	Turn					mBestTurns;
	unsigned int			*mPrevGrid;
	inline unsigned int		GetPrevGrid(int x, int y) {return mPrevGrid[x+(y*mGridWidth)];}
	inline void				SetPrevGrid(int x, int y, unsigned int theValue) {mPrevGrid[x+(y*mGridWidth)]=theValue;}

	int						mBestNumTurns;
	int						mBestLen;
	int						mItrCount;
	int						mTrailLength;

	//
	// Flesh this out yourself... per program.
	// Since we'll determine blockishness uniquely.
	//
	bool					IsBlocked(int x, int y);
};

