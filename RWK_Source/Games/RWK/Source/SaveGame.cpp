#include "SaveGame.h"


#define OUTSYNC {if (!mChunk) {gOut.Out("SaveGame::Sync() - Trying to sync from outside a chunk!");return;}}

SaveGame::SaveGame(void)
{
	mSaving=false;
	mMainChunk=NULL;
	mVersion=0;
	Reset();
}

SaveGame::~SaveGame(void)
{
	Clear();
}

void SaveGame::Reset()
{
	if (mMainChunk) Clear();
	mMainChunk=new Chunk;
	mChunk=mMainChunk;
}

void SaveGame::Clear()
{
	_FreeList(Chunk,mNestedSaveGameList);
	_FreeList(Chunk,mCleanupList);
	delete mMainChunk;
	mMainChunk=NULL;
}

void SaveGame::Saving(String theFilename)
{
	mSaving=true;
	mFilename=theFilename;

	Reset();
}

bool SaveGame::Loading(String theFilename)
{
	mSaving=false;
	mFilename=theFilename;

	Clear();

	IOBuffer aBuffer;
	aBuffer.Load(theFilename.c());

	if (aBuffer.mDataLen<=0) return false;
	int aVersion=aBuffer.ReadInt();
	if (aVersion!=mVersion) return false;
	Load(aBuffer);
	return true;
}

void SaveGame::Load(IOBuffer &theBuffer)
{
	Reset();

	LoadChunk(theBuffer,mMainChunk);

	if (theBuffer.IsEnd()) return;
	//
	// Bring in the nested savegames...
	//
	int aReadChunks=theBuffer.ReadInt();
	for (int aCount=0;aCount<aReadChunks;aCount++)
	{
		Chunk *aChunk=new Chunk;
		aChunk->mSaveGame=new SaveGame;
		aChunk->mSaveGame->mSaving=mSaving;
		aChunk->mName=theBuffer.ReadString();
		aChunk->mSaveGame->Load(theBuffer);
		mNestedSaveGameList+=aChunk;
	}

}

void SaveGame::Rewind()
{
	if (mChunk) mChunk->mChunkID=0;
	else
	{
		gOut.Out("Called SaveGame::Rewind outside of a chunk!");
	}
}




SaveGame &SaveGame::GetNestedSaveGame(String theName)
{
	//
	// It's a named chunk, which means it is actually its own savegame...
	// Its name has to be unique, and it can be accessed by name only!
	// But it can be accessed out of order!
	//
	if (mSaving)
	{
		Chunk *aChunk=new Chunk;
		aChunk->mName=theName;
		aChunk->mSaveGame=new SaveGame;
		aChunk->mSaveGame->mSaving=mSaving;
		mNestedSaveGameList+=aChunk;

		return *aChunk->mSaveGame;
	}
	else
	{
		//
		// Find the named chunk!
		//
		EnumList(Chunk,aChunk,mNestedSaveGameList)
		{
			if (theName==aChunk->mName) 
			{
				return *aChunk->mSaveGame;
			}
		}

		//
		// Couldn't find the chunk... give them a blank one to 
		// work with, so we don't get crashes.
		//
		Chunk *aChunk=new Chunk;
		aChunk->mName=theName;
		aChunk->mSaveGame=new SaveGame;
		aChunk->mSaveGame->mSaving=mSaving;
		mNestedSaveGameList+=aChunk;
		return *aChunk->mSaveGame;
	}
}

void SaveGame::StartChunk()
{
	if (mSaving)
	{

		Chunk *aChunk=new Chunk;
		mChunk->mChunkList+=aChunk;
		mChunk->mChunkID++;
		aChunk->mParent=mChunk;
		mChunk=aChunk;

	}
	else	
	{
		Chunk *aChunk=(Chunk*)mChunk->mChunkList[mChunk->mChunkID++];
		if (!aChunk)
		{
			//
			// We're loading something that doesn't exist...
			// So we make a blank chunk.
			//
			aChunk=new Chunk;
			mCleanupList+=aChunk;
			aChunk->mParent=mChunk;
		}
		mChunk=aChunk;
	}
}


void SaveGame::EndChunk()
{
	if (mChunk==mMainChunk || !mChunk) 
	{
		gOut.Out("ERROR: SaveGame::EndChunk called on Main Chunk!");
		return;
	}


	Chunk *aParent=mChunk->mParent;
	mChunk=aParent;
}

void SaveGame::Commit()
{
	Commit(mFilename.c());
/*
	if (mSaving)
	{
		IOBuffer aWorkBuffer;
		aWorkBuffer.WriteInt(mVersion);
		Commit(aWorkBuffer);
		aWorkBuffer.CommitFile(mFilename.c());
	}
*/
}

void SaveGame::Commit(String theFilename)
{
	if (mSaving)
	{
		IOBuffer aWorkBuffer;
		aWorkBuffer.WriteInt(mVersion);
		Commit(aWorkBuffer);
		aWorkBuffer.CommitFile(theFilename.c());
	}
}

void SaveGame::Commit(IOBuffer &theBuffer)
{
	if (mChunk!=mMainChunk)
	{
		gOut.Out("SaveGame::Commit() - Trying to commit with an open Chunk!");
		return;
	}

	if (mSaving)
	{
		CommitChunk(theBuffer,mMainChunk);
		//
		// Commit any named chunks too!
		//
		// Hey, are you having a problem with Nested Savegames not saving?
		// Don't forget, it's a REFERENCE!  So do SaveGame &aNewSaveGame=whatever.GetNestedSavegame();
		// Don't forge the '&'!!!
		// 
		theBuffer.WriteInt(mNestedSaveGameList.GetCount());
		EnumList(Chunk,aChunk,mNestedSaveGameList) 
		{
			theBuffer.WriteString(aChunk->mName);
			aChunk->mSaveGame->Commit(theBuffer);
		}
	}

}

void SaveGame::CommitChunk(IOBuffer &theBuffer, Chunk *theChunk)
{
	theBuffer.WriteBuffer(theChunk->mBuffer);
	theBuffer.WriteInt(theChunk->mChunkList.GetCount());
	EnumList(Chunk,aChunk,theChunk->mChunkList)
	{
		CommitChunk(theBuffer,aChunk);
	}
}

void SaveGame::LoadChunk(IOBuffer &theBuffer, Chunk *theChunk)
{
	theBuffer.ReadBuffer(theChunk->mBuffer);
	int aChunkCount=theBuffer.ReadInt();

	for (int aCount=0;aCount<aChunkCount;aCount++)
	{
		Chunk *aChunk=new Chunk;
		aChunk->mParent=theChunk;
		theChunk->mChunkList+=aChunk;
		LoadChunk(theBuffer,aChunk);
	}
}


void SaveGame::Sync(char *theChar)
{
	OUTSYNC;
	if (mSaving) mChunk->mBuffer.WriteChar(*theChar);
	else *theChar=mChunk->mBuffer.ReadChar();
}

void SaveGame::Sync(int *theInt)
{
	OUTSYNC;
	if (mSaving) mChunk->mBuffer.WriteInt(*theInt);
	else *theInt=mChunk->mBuffer.ReadInt();
}

void SaveGame::Sync(longlong *theInt)
{
	OUTSYNC;
	if (mSaving) mChunk->mBuffer.WriteLongLong(*theInt);
	else *theInt=mChunk->mBuffer.ReadLongLong();
}

void SaveGame::Sync(unsigned int *theInt)
{
	OUTSYNC;
	if (mSaving) mChunk->mBuffer.WriteInt(*theInt);
	else *theInt=mChunk->mBuffer.ReadInt();
}

void SaveGame::Sync(bool *theBool)
{
	OUTSYNC;
	if (mSaving) mChunk->mBuffer.WriteBool(*theBool);
	else *theBool=mChunk->mBuffer.ReadBool();
}

void SaveGame::Sync(Point *thePoint)
{
	OUTSYNC;
	if (mSaving) mChunk->mBuffer.WritePoint(*thePoint);
	else *thePoint=mChunk->mBuffer.ReadPoint();
}

void SaveGame::Sync(IPoint *theIPoint)
{
	OUTSYNC;
	if (mSaving) mChunk->mBuffer.WriteIPoint(*theIPoint);
	else *theIPoint=mChunk->mBuffer.ReadIPoint();
}

void SaveGame::Sync(String *theString)
{
	OUTSYNC;
	if (mSaving) mChunk->mBuffer.WriteString(*theString);
	else *theString=mChunk->mBuffer.ReadString();
}

void SaveGame::Sync(float *theFloat)
{
	OUTSYNC;
	if (mSaving) mChunk->mBuffer.WriteFloat(*theFloat);
	else *theFloat=mChunk->mBuffer.ReadFloat();
}

void SaveGame::Sync(double *theDouble)
{
	OUTSYNC;
	if (mSaving) mChunk->mBuffer.WriteDouble(*theDouble);
	else *theDouble=mChunk->mBuffer.ReadDouble();
}

void SaveGame::Sync(IOBuffer *theBuffer)
{
	OUTSYNC;
	if (mSaving) mChunk->mBuffer.WriteBuffer(*theBuffer);
	else mChunk->mBuffer.ReadBuffer(*theBuffer);
}

void SaveGame::Sync(void *theMemory, int theLength)
{
	OUTSYNC;
	if (mSaving) mChunk->mBuffer.WriteRaw(theMemory,theLength);
	else mChunk->mBuffer.ReadRaw(theMemory,theLength);
}

void SaveGame::Sync(Color *theColor)
{
	Sync(&theColor->mR);
	Sync(&theColor->mG);
	Sync(&theColor->mB);
	Sync(&theColor->mA);
}

void SaveGame::Sync(Rect *theRect)
{
	Sync(&theRect->mX);
	Sync(&theRect->mY);
	Sync(&theRect->mWidth);
	Sync(&theRect->mHeight);
}

bool SaveGame::Checksum(int theValue)
{
	int aTestValue=-1;
	if (mSaving) aTestValue=theValue;
	Sync(&aTestValue);
	if (!mSaving) if (theValue!=aTestValue) return false;
	return true;
}

int SaveGame::Sync(int theValue)
{
	if (mSaving)
	{
		Sync(&theValue);
		return theValue;
	}
	else
	{
		int aResult;
		Sync(&aResult);
		return aResult;
	}
}


bool SaveGame::Sync(bool theValue)
{
	if (mSaving)
	{
		Sync(&theValue);
		return theValue;
	}
	else
	{
		bool aResult;
		Sync(&aResult);
		return aResult;
	}
}

bool SaveGame::SyncNotNull(void *thePointer)
{
	if (mSaving)
	{
		bool aResult=(thePointer!=NULL);
		Sync(aResult);
		return aResult;
	}
	else
	{
		bool aResult;
		Sync(&aResult);
		return aResult;
	}
}

