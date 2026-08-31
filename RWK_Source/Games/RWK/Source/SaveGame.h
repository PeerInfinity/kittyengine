#pragma once
#include "rapt.h"


class SaveGame
{
public:
	SaveGame(SaveGame &theSaveGame) {gOut.Out("Nested Savegame Error: Use a reference, not a local variable for nested save games!");}
	SaveGame(void);
	virtual ~SaveGame(void);

public:

	//
	// Saving to file...
	//
	void			Saving(String theFilename);
	void			Commit();
	//
	// This allows you to commit manually to another filename,
	// even if you've specified Saving elsewhere.  I put this in 
	// so that in Solomon's Keep, I could back up the system without
	// having to copy a file.
	//
	void			Commit(String theFilename);

	//
	// Savegame Version... lets us check to see if we need
	// to reject this savegame.
	//
	int				mVersion;
	bool			IsVersionOK(int theVersion) {return (mVersion==theVersion);}

	//
	// Loads from file
	//
	bool			Loading(String theFilename);	// Returns false if the file doesn't exist
	void			Load(IOBuffer &theBuffer);

	//
	// Rewind goes back to the first chunk, so you can read a little, then restart if you need to.
	// I put this explicitely in to allow me to read a game file version before load, then rewind so it can be
	// synced again in the normal syn.
	//
	void			Rewind();

	//
	// Starts a data chunk.  These can be handle lengthened or shortened
	// reads (for instance, adding a new variable to the mix), but they are always
	// read in order.
	//
	void			StartChunk();
	void			EndChunk();

	//
	// Gets a nested Savegame... these sit within the main one.  
	// They are stored by name, and so can be retrieve out of order.
	// Note: They are a REFERENCE... use a REFERENCE variable
	// to hold them!
	//
	SaveGame		&GetNestedSaveGame(String theName);

	//
	// Helper functions
	//
	bool			IsSaving() {return mSaving;}
	bool			IsLoading() {return !mSaving;}

	//
	// Sync functions...
	//
	void			Sync(char *theChar);
	void			Sync(int *theInt);
	void			Sync(longlong *theInt);
	void			Sync(unsigned int *theInt);
	void			Sync(bool *theBool);
	void			Sync(Point *thePoint);
	void			Sync(IPoint *theIPoint);
	void			Sync(String *theString);
	void			Sync(float *theFloat);
	void			Sync(double *theDouble);
	void			Sync(IOBuffer *theBuffer);
	void			Sync(void *theMemory, int theLength);
	void			Sync(Color *theColor);
	void			Sync(Rect *theRect);

	//
	// Special sync functions that give us added
	// ability and shortcuts
	//
	int				Sync(int theValue=0);			// Syncs an integer
	bool			Sync(bool theValue);			// Syncs a bool
	bool			SyncNotNull(void *thePointer);	// Syncs whether a pointer is null or not, so we can use it load something, or not

	//
	// Checksum just lets you put a value in to check.
	// If it's correct, returns true, if not, returns false.
	//
	bool			Checksum(int theValue=0xA0A0A0A0);

	void			Clear();
	void			Reset();

public:

	bool			mSaving;
	String			mFilename;

	struct Chunk
	{
		Chunk() {mSaveGame=NULL;mParent=NULL;mChunkID=0;}
		~Chunk() 
		{
			delete mSaveGame;
			mSaveGame=NULL;
			_FreeList(Chunk,mChunkList);
		}

		String		mName;
		IOBuffer	mBuffer;
		SaveGame	*mSaveGame;

		Chunk		*mParent;			// Parent Chunk
		List		mChunkList;			// Chunks embedded in this chunk
		int			mChunkID;			// ID of chunk we're reading
	};

	Chunk			*mMainChunk;

	List			mNestedSaveGameList;	// List of nested Savegames.  These don't nest, they're savegames in their own right.
	List			mCleanupList;			// If we try to load chunks that don't exist, we make a blank one and add it here for later deleting...

	Chunk			*mChunk;				// The current chunk we're working with


public:
	void			Commit(IOBuffer &theBuffer);
	void			CommitChunk(IOBuffer &theBuffer, Chunk *theChunk);
	void			LoadChunk(IOBuffer &theBuffer,Chunk *theChunk);

};

//
// Helpful defines for lists...
//
//
// Be carefuly with SyncListStart... if you re-cast the refItem, remember to remove it from
// your list before recasting, and re-add it later!
//
#define SyncListStart(refSaveGame,refObjectType,refList)								\
	{																					\
		int refCount=refSaveGame.Sync(refList.GetCount());								\
		for (int refCountUp=0;refCountUp<refCount;refCountUp++)							\
		{																				\
			refObjectType *refItem;														\
			if (refSaveGame.IsSaving()) refItem=(refObjectType*)refList[refCountUp];	\
			else {refItem=new refObjectType;refList+=refItem;}						

#define SyncListEnd()												\
}																	\
}																	

//
// Synchronize an object that might be NULL.
// Handles it so that if it is NULL, it loads in as NULL, but if it existed
// at save time, it loads in normally.
//
#define SyncNotNullObject(refSaveGame,refObjectType,refObject)						\
	if (refSaveGame.SyncNotNull(refObject))											\
	{																				\
		if (refSaveGame.IsLoading()) refObject=new refObjectType();					\
		refObject->Sync(refSaveGame);												\
	}

//
// Synchronize a list of items that have only one type in them...
//
#define SyncList(refSaveGame,refObjectType,refList)										\
	{																					\
		int refCount=refSaveGame.Sync(refList.GetCount());								\
		for (int refCountUp=0;refCountUp<refCount;refCountUp++)							\
		{																				\
			refObjectType *refItem;														\
			if (refSaveGame.IsSaving()) refItem=(refObjectType*)refList[refCountUp];	\
			else {refItem=new refObjectType;refList+=refItem;}							\
			refItem->Sync(refSaveGame);													\
		}																				\
	}																				
		