#include "rapt_iobuffer.h"
#include "os_core.h"
#include "network_core.h" // For Download

#include "rapt_app.h"

#define WADCODE "::WadFile_Starts_Here"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// IOBuffer Class
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

List gDownloadingBuffers;

//
// Helpers for autoclouding...
//
bool IsAutoCloudLocation(char* theFN)
{
	if (!gAppPtr) return false;
	if (!gAppPtr->mAutoCloud || gAppPtr->mIsShutdown) return false;	// If we're not autoclouding, or we're shutting down, we don't autocloud.

	String aFN=gAppPtr->GetStorageLocation(theFN);
	if (aFN.ContainsI(".nocloud")) return false;
	if (aFN.StartsWith("sandbox://")) return true;
	if (aFN.StartsWith("cloud://")) return true;
	return false;
}

IOBuffer::IOBuffer(void)
{
	mData=NULL;
	mCursor=0;
	mDataLen=0;
	mAllocLen=0;
	mError=false;
	mStatus=0;
}

IOBuffer::~IOBuffer(void)
{
	if (!IsDataIsPointer()) _DeleteArray(mData);
	SetDownloading(false);

	mStatus=0;
	mData=NULL;
}

void IOBuffer::Reset(bool freeMemory)
{
	if (freeMemory)
	{
		if (!IsDataIsPointer()) {_DeleteArray(mData);}
		mData=NULL;
		mAllocLen=0;
	}

	mDataLen=0;
	mCursor=0;
	mError=false;
	mStatus=0;
}

void IOBuffer::LoadFromString(String theString)
{
	if (!IsDataIsPointer()) {_DeleteArray(mData);}
	SetDataIsPointer(false);
	SetDownloading(false);
	mDataLen=0;
	mAllocLen=0;
	mCursor=0;
	mError=true;

	if (theString.Len())
	{
		mData=new char[theString.Len()];
		memcpy(mData,theString.c(),theString.Len());
		mAllocLen=theString.Len();
		mDataLen=mAllocLen;
		if (!mData)  return;
		mError=false;
	}
}

void IOBuffer::SetFileSource(char *theFilename)
{
	if (!IsDataIsPointer()) {_DeleteArray(mData);}
	SetDataIsPointer(false);
	SetDownloading(false);
	mDataLen=0;
	mAllocLen=0;
	mCursor=0;
	mError=true;

	mData=OS_Core::ReadFile(theFilename,&mDataLen);
	mAllocLen=mDataLen;
	if (!mData)  return;
	mError=false;
}

void IOBuffer::SetFileSource(char *theFilename, int theBytesToRead)
{
	if (!IsDataIsPointer()) {_DeleteArray(mData);}
	SetDataIsPointer(false);
	SetDownloading(false);
	mDataLen=theBytesToRead;
	mCursor=0;
	mError=true;

	mData=OS_Core::ReadPartialFile(theFilename,theBytesToRead);
	mAllocLen=mDataLen;
	if (!mData)  return;
	mError=false;
}

void IOBuffer::Download(String theURL, float* theProgressIndicator)
{
	Reset();
    
    //
    // Added for Apple... Apple's download calls literally require http to be there, or they
    // just fail.  Cuz you know, that added layer of security will DETER THE H@XX0RZZZZ!!!!!
    //
    if (!theURL.StartsWith("http")) theURL.Insert("http://");

	mError=true;
	SetDownloading(true);
	Network_Core::Download(theURL,&mData,&mDataLen,theProgressIndicator);
	IsDownloadOK();

	//mAllocLen=mDataLen;
	//if (!mData) return;
	//mError=false;
}

bool IOBuffer::IsDownloadOK()
{
	if (!gAppPtr || gAppPtr->IsQuit()) return false;
	if (!gDownloadingBuffers.Exists(this)) return false;
	SetDownloading(false);
	mAllocLen=mDataLen;
	if (mData) mError=false;
	return true;
}

void IOBuffer::SetDownloading(bool theState)
{
	if (theState==false && IsDownloading()) gDownloadingBuffers-=this;
	if (theState==true) {gDownloadingBuffers.Unique();gDownloadingBuffers+=this;}

	mStatus|=IOBUFFER_STATUS_DOWNLOADING;
	if (!theState) mStatus^=IOBUFFER_STATUS_DOWNLOADING;
}

void IOBuffer::DownloadA(String theURL, float *theProgressIndicator, IOBUFFERHOOKPTR runWhenComplete)
{
	if (theURL.Len()==0)
	{
		//
		// If no URL is provided, we simply complete immediately (without resetting the contents of the buffer).
		// This can be used as a kludge to invoke the runWhenComplete on pre-loaded contents and keep the pipeline the same
		// whether there's anything to download or not.
		//
		SetDownloading(true);
		if (theProgressIndicator) *theProgressIndicator=1.0f;
		if (runWhenComplete==NULL) runWhenComplete=IOBUFFERHOOK({});
		if (runWhenComplete) runWhenComplete(this);
	}
	else
	{
		Reset();
		String aURL=theURL;

		//gOut.Out("Download: %s",theURL.c());

		if (!aURL.StartsWith("http")) aURL.Insert("http://");

		//gOut.Out("Download (Fixed): %s",theURL.c());
		mError=true;

		SetDownloading(true);
		if (runWhenComplete==NULL) runWhenComplete=IOBUFFERHOOK({});
		Network_Core::DownloadA(aURL,&mData,&mDataLen,theProgressIndicator,runWhenComplete,this);
	}
}

void IOBuffer::UploadA(String theURL, IOBuffer* theResultBuffer, float *theProgressIndicator, IOBUFFERHOOKPTR runWhenComplete)
{
	if (theURL.Len()==0 || mDataLen==0)
	{
		if (mDataLen==0) gOut.Debug("!!!! TRIED UPLOADING A ZERO LENGTH BUFFER -- SKIPPING !!!!");
		//
		// If no URL is provided, we simply complete immediately (without resetting the contents of the buffer).
		// This can be used as a kludge to invoke the runWhenComplete on pre-loaded contents and keep the pipeline the same
		// whether there's anything to download or not.
		//
		SetDownloading(true);
		if (theProgressIndicator) *theProgressIndicator=1.0f;
		if (runWhenComplete==NULL) runWhenComplete=IOBUFFERHOOK({});
		if (runWhenComplete) runWhenComplete(this);
	}
	else
	{
		String aURL=theURL;
		if (!aURL.StartsWith("http")) aURL.Insert("http://");
		mError=true;
		SetDownloading(true);
		if (runWhenComplete==NULL) runWhenComplete=IOBUFFERHOOK({});

		Network_Core::UploadA(aURL,&mData,&mDataLen,theProgressIndicator,&theResultBuffer->mData,&theResultBuffer->mDataLen,runWhenComplete,this);
	}
}





void IOBuffer::Upload(String theURL, IOBuffer* theResultBuffer, float* theProgressIndicator)
{
	if (!mData) return;
	if (theResultBuffer==NULL) theResultBuffer=this;

	mError=true;
	char* aResultBuffer;
	int aResultBufferSize;

	Network_Core::Upload(theURL,&mData,&mDataLen,theProgressIndicator,&aResultBuffer,&aResultBufferSize);
	theResultBuffer->SetMemoryPointerSource(aResultBuffer,aResultBufferSize,true);

	mError=false;
}

void EnumCloud(Array<String> &theArray)
{
	if (!OS_Core::HasCloud()) return;

	Array<char*> aResult;
	OS_Core::EnumCloud(aResult);
	for (int aCount=0;aCount<aResult.Size();aCount++) 
	{
		theArray+=aResult[aCount];
		delete [] aResult[aCount];
	}
}

bool IOBuffer::CloudLoad(String theKeyName)
{
	if (!gAppPtr->HasCloud()) return false;

	if (!IsDataIsPointer()) {_DeleteArray(mData);}
	SetDataIsPointer(false);
	SetDownloading(false);
	mDataLen=0;
	mAllocLen=0;
	mCursor=0;
	mError=true;

	if (!OS_Core::GetCloud(theKeyName,(void**)&mData,&mDataLen)) return false;
	mAllocLen=mDataLen;
	if (!mData) return false;
	mError=false;
	return true;
}

bool IOBuffer::CloudSave(String theKeyName, bool doImmediate)
{
	if (!gAppPtr->HasCloud()) return false;

	//
	// This shouldn't actually cloud...it should put everything into a cloud queue for later (threaded) writing.
	//
#ifdef NO_THREADS
	return OS_Core::PutCloud(theKeyName,mData,mDataLen,true);
#else
	if (doImmediate) 
	{
		return OS_Core::PutCloud(theKeyName,mData,mDataLen,true);
	}
	CloudQueue::Put(theKeyName,this);
	return true;
#endif
}

void IOBuffer::SetMemorySource(void *theData, int theDataLen)
{
	if (!IsDataIsPointer()) {_DeleteArray(mData);}
	SetDataIsPointer(false);
	SetDownloading(false);
	mDataLen=0;
	mAllocLen=0;

	if (theDataLen && theData)
	{
		mData=new char[theDataLen];
		memcpy(mData,theData,theDataLen);
		mDataLen=theDataLen;
		mAllocLen=theDataLen;
	}
	mCursor=0;
	mError=false;
}

void IOBuffer::SetMemoryPointerSource(void *theData, int theDataLen, bool takeControlofData)
{
	if (!IsDataIsPointer()) {_DeleteArray(mData);}
	if (theDataLen==0)
	{
		Reset();
		return;
	}

	SetDataIsPointer(!takeControlofData);
	SetDownloading(false);

	mData=(char*)theData;
	mDataLen=theDataLen;
	mAllocLen=theDataLen;
	mCursor=0;
	mError=false;
}

bool IOBuffer::IsBinary()
{
	return memchr(mData, '\0', _min(1000000,Len())) != NULL;
/*
	char* aPtr=mData;
	int aFudge=0;
	for (int aCount=0;aCount<Len();aCount++)
	{
		if (*aPtr<1 || *aPtr>127) return true;
		aPtr++;
	}
	return false;
*/
}


void IOBuffer::CommitFile(char *theFilename)
{
	if (theFilename!=NULL) 
	{
		OS_Core::WriteFile(theFilename,mData,mDataLen);
		if (IsAutoCloudLocation(theFilename)) this->CloudSave(FilenameToCloudKey(theFilename));
	}
}

void IOBuffer::Dump(String theMessage)
{
	gOut.Out("IOBuffer::Dump(%s)",theMessage.c());
	for (int aCount=0;aCount<mDataLen;aCount++) gOut.Out("[%d] = %d (%c)",aCount,mData[aCount],mData[aCount]);
}


char IOBuffer::ReadChar()
{
	if (IsEnd()) return 0;
	if (mCursor+1>mDataLen) 
	{
		mError=true;
		return 0;
	}


	int aCursor=mCursor;
	mCursor++;

	return *(char*)(mData+aCursor);
}

char IOBuffer::PeekChar(int stepForward)
{
	if (IsEnd()) return 0;
	if (mCursor+stepForward>=mDataLen) 
	{
		mError=true;
		return 0;
	}

	return *(char*)(mData+mCursor+stepForward);
}

bool IOBuffer::IsPeekString(int stepForward, char* theString)
{
	if (IsEnd()) return 0;
	for (unsigned int aCount=0;aCount<strlen(theString);aCount++)
	{
		if (mCursor+stepForward>=mDataLen) 
		{
			mError=true;
			return false;
		}
		char aC=*(char*)(mData+mCursor+stepForward+aCount);
		if (aC!=theString[aCount]) return false;
	}
	return true;
}


unsigned char IOBuffer::ReadUChar()
{
	if (IsEnd()) return 0;
	if (mCursor+1>mDataLen) 
	{
		mError=true;
		return 0;
	}


	int aCursor=mCursor;
	mCursor++;

	return *(unsigned char*)(mData+aCursor);
}

short IOBuffer::ReadShort()
{
	if (IsEnd()) return 0;
	if (mCursor+2>mDataLen) 
	{
		mError=true;
		return 0;
	}

	int aCursor=mCursor;
	mCursor+=2;

	short aResult;memcpy(&aResult,(mData+aCursor),sizeof(short));
	return aResult;
//	return *(short*)(mData+aCursor);
}

vbindex IOBuffer::ReadVBIndex()
{
	if (IsEnd()) return 0;
	if ((int)(mCursor+sizeof(vbindex))>mDataLen) 
	{
		mError=true;
		return 0;
	}

	int aCursor=mCursor;
	mCursor+=sizeof(vbindex);

	vbindex aResult;memcpy(&aResult,(mData+aCursor),sizeof(vbindex));
	return aResult;
}


unsigned short IOBuffer::ReadUShort()
{
	if (IsEnd()) return 0;
	if (mCursor+2>mDataLen) 
	{
		mError=true;
		return 0;
	}

	int aCursor=mCursor;
	mCursor+=2;

	short aResult;memcpy(&aResult,(mData+aCursor),sizeof(unsigned short));
	return aResult;
//	return *(unsigned short*)(mData+aCursor);
}


int IOBuffer::ReadInt()
{
	if (IsEnd()) return 0;
	if (mCursor+4>mDataLen) 
	{
		mError=true;
		return 0;
	}

	int aCursor=mCursor;
	mCursor+=4;

	int aResult;memcpy(&aResult,(mData+aCursor),sizeof(aResult));
	return aResult;
	//return *(int*)(mData+aCursor);
}

longlong IOBuffer::ReadLongLong()
{
	if (IsEnd()) return 0;
	if ((int)(mCursor+sizeof(longlong))>mDataLen) 
	{
		mError=true;
		return 0;
	}

	int aCursor=mCursor;
	mCursor+=sizeof(longlong);

	longlong aResult;memcpy(&aResult,(mData+aCursor),sizeof(aResult));
	return aResult;

//	return *(longlong*)(mData+aCursor);
}

unsigned int IOBuffer::ReadUInt()
{
	if (IsEnd()) return 0;
	if (mCursor+4>mDataLen) 
	{
		mError=true;
		return 0;
	}

	int aCursor=mCursor;
	mCursor+=4;

	unsigned int aResult;memcpy(&aResult,(mData+aCursor),sizeof(aResult));
	return aResult;
	//return *(unsigned int*)(mData+aCursor);
}

String IOBuffer::ReadString()
{
	//
	// HAVING TROUBLE?
	// Did you know that sprintf commands apparently process
	// the parameters BACKWARDS?  So, if you store a buffer
	// with:
	//			WriteString("Hello");
	//			WriteInt(50);
	//
	// And do this:
	//          gOut.Out("%s (%d)",ReadString(),ReadInt());
	//
	// You'll get incorrect results.  You need to do
	// THIS in that situation:
	//          gOut.Out("%s (%d)",ReadInt(),ReadString());
	//
	// However, this seems to be SO dangerous that you
	// need to completely avoid accessing multiple buffer
	// reads from a sprintf type statement.
	//
	int aLen=ReadInt();

	if (mCursor+aLen>mDataLen || aLen<0) return ""; // ADD THIS!  Whole IOBuffer is invalid now, tho!
	if (IsEnd()) 
	{
		String aNullString;
		return aNullString;
	}

	int aCursor=mCursor;
	mCursor+=aLen;
	return String((char*)(mData+aCursor));
}

int IOBuffer::ReadLine(char *theDestination)
{
	if (IsEnd()) return 0;

	int aHoldCursor=mCursor;
	int aLineLength=0;

	bool aGotActualData=false;

	for(;;)
	{
		if (IsEnd()) break;
		int aCursor=mCursor;
		mCursor++;
		//gOut.Out("PData: %c [%d]",mData[aCursor],mData[aCursor]);

		if (mData[aCursor]>=32) aGotActualData=true;
		if (aGotActualData)
		{
			if (mData[aCursor]==13) {if (aCursor<mDataLen-1) if (mData[aCursor+1]==10) mCursor++;break;}
			if (aCursor>0) if ((mData[aCursor]==10 && mData[aCursor-1]>=32)) break;
			theDestination[aLineLength]=mData[aCursor];
			aLineLength++;
		}
		if (aCursor>0) if ((mData[aCursor]==10 && mData[aCursor-1]>=32)) break;
		if (aCursor<mDataLen-1) if (mData[aCursor]==10 && mData[aCursor+1]>=32) break;
	}
	return aLineLength;
}

String IOBuffer::ReadINILine(char theStartBrace, char theEndBrace)
{
	if (IsEnd()) 
	{
		String aNullString;
		return aNullString;
	}

	int aHoldCursor=mCursor;
	int aLineLength=0;

	bool aGotActualData=false;

	int aBraceCount=0;

	String aResult;
	//gOut.Out("");
	for(;;)
	{
		if (IsEnd()) break;
		int aCursor=mCursor;
		mCursor++;
		//gOut.Out("PData: %c [%d]",mData[aCursor],mData[aCursor]);

		//
		// Wow, LOTS of ifs necessary to parse CRLF's vs plain LF's in files!
		// Note: if this doesn't work, you should set up a flag in the buffer so that
		// you look for a CRLF and mark that as a file type, vs. a type that has only LF
		//
		if (mData[aCursor]==theStartBrace) aBraceCount++;
		if (mData[aCursor]==theEndBrace) aBraceCount--;

		if (mData[aCursor]>=32 || aBraceCount>0) aGotActualData=true;
		if (aGotActualData)
		{
			if (aBraceCount<=0) 
			{
				if (mData[aCursor]==13) {if (aCursor<mDataLen-1) if (mData[aCursor+1]==10) mCursor++;break;}
				if (aCursor>0) if ((mData[aCursor]==10 && mData[aCursor-1]>=32)) break;
			}
			aResult+=mData[aCursor];
		}
		if (aBraceCount<=0) 
		{
			if (aCursor>0) if ((mData[aCursor]==10 && mData[aCursor-1]>=32)) break;
			if (aCursor<mDataLen-1) if (mData[aCursor]==10 && mData[aCursor+1]>=32) break;
		}
	}
	return aResult;
}


String IOBuffer::ReadLine()
{
	if (IsEnd()) 
	{
		String aNullString;
		return aNullString;
	}

	int aHoldCursor=mCursor;
	int aLineLength=0;

	bool aGotActualData=false;

	String aResult;
	//gOut.Out("");
	for(;;)
	{
		if (IsEnd()) break;
		int aCursor=mCursor;
		mCursor++;
		//gOut.Out("PData: %c [%d]",mData[aCursor],mData[aCursor]);

		//
		// Wow, LOTS of ifs necessary to parse CRLF's vs plain LF's in files!
		// Note: if this doesn't work, you should set up a flag in the buffer so that
		// you look for a CRLF and mark that as a file type, vs. a type that has only LF
		//

		if (mData[aCursor]>=32) aGotActualData=true;
		if (aGotActualData)
		{
			if (mData[aCursor]==13) {if (aCursor<mDataLen-1) if (mData[aCursor+1]==10) mCursor++;break;}
			if (aCursor>0) if ((mData[aCursor]==10 && mData[aCursor-1]>=32)) break;
			aResult+=mData[aCursor];
		}
		if (aCursor>0) if ((mData[aCursor]==10 && mData[aCursor-1]>=32)) break;
		if (aCursor<mDataLen-1) if (mData[aCursor]==10 && mData[aCursor+1]>=32) break;
	}
	return aResult;
}

String IOBuffer::ReadRawLine()
{
	if (IsEnd()) 
	{
		String aNullString;
		return aNullString;
	}

	int aHoldCursor=mCursor;
	int aLineLength=0;

	bool aGotActualData=false;

	String aResult;
	//gOut.Out("");
	for(;;)
	{
		if (IsEnd()) break;
		int aCursor=mCursor;
		mCursor++;
		//gOut.Out("PData: %c [%d]",mData[aCursor],mData[aCursor]);

		//
		// Wow, LOTS of ifs necessary to parse CRLF's vs plain LF's in files!
		// Note: if this doesn't work, you should set up a flag in the buffer so that
		// you look for a CRLF and mark that as a file type, vs. a type that has only LF
		//

		if (mData[aCursor]!=13 || mData[aCursor]!=10) aGotActualData=true;
		if (aGotActualData)
		{
			if (mData[aCursor]==13) {if (aCursor<mDataLen-1) if (mData[aCursor+1]==10) mCursor++;break;}
			if (aCursor>0) if ((mData[aCursor]==10 && mData[aCursor-1]>=32)) break;
			aResult+=mData[aCursor];
		}
		if (aCursor>0) if ((mData[aCursor]==10 && mData[aCursor-1]>=32)) break;
		if (aCursor<mDataLen-1) if (mData[aCursor]==10 && mData[aCursor+1]>=32) break;
	}
	return aResult;
}

float IOBuffer::ReadFloat()
{
	if (IsEnd()) return 0;
	if (mCursor+4>mDataLen) 
	{
		mError=true;
		return 0;
	}

	int aCursor=mCursor;
	mCursor+=4;
    
    float aFloat;
    memcpy(&aFloat,(mData+aCursor),sizeof(float));
    return aFloat;
//	return *(float*)(mData+aCursor);
}

double IOBuffer::ReadDouble()
{
	if (IsEnd()) return 0;
	if ((int)(mCursor+sizeof(double))>mDataLen) 
	{
		mError=true;
		return 0;
	}

	int aCursor=mCursor;
	mCursor+=sizeof(double);
    
    double aDouble;
    memcpy(&aDouble,(mData+aCursor),sizeof(double));
    return aDouble;

//	return *(double*)(mData+aCursor);
}

bool IOBuffer::ReadBool()
{
	if (IsEnd()) return 0;
	if (mCursor+1>mDataLen) 
	{
		mError=true;
		return 0;
	}

	int aCursor=mCursor;
	mCursor++;

	bool aResult;memcpy(&aResult,(mData+aCursor),sizeof(aResult));
	return aResult;
	//return (*(char*)(mData+aCursor)==1);
}

Matrix IOBuffer::ReadMatrix() 
{
	Matrix aResult;
	for (int aCount=0;aCount<16;aCount++) aResult.mData.mm[aCount]=ReadFloat();
	return aResult;
}


void IOBuffer::ReadRaw(void *theDestination, int theSize)
{
	if (mCursor+theSize<=mDataLen)
	{
		memcpy(theDestination,(mData+mCursor),theSize);
		mCursor+=theSize;
	}
	else
	{
		memset(theDestination,0,theSize);
		mError=true;
	}

}

void IOBuffer::WriteRaw(void *theMemory, int theSize)
{
	if (!EnsureDataFit(theSize)) return;
	memcpy((mData+mCursor),theMemory,theSize);
	mCursor+=theSize;
}

void IOBuffer::WriteChar(char theChar)
{
	if (!EnsureDataFit(1)) return;

	memcpy(mData+mCursor,&theChar,sizeof(char));
	//*(char*)(mData+mCursor)=theChar;
	mCursor++;
}

void IOBuffer::WriteUChar(unsigned char theChar)
{
	if (!EnsureDataFit(1)) return;

	memcpy(mData+mCursor,&theChar,sizeof(unsigned char));
	//*(char*)(mData+mCursor)=theChar;
	mCursor++;
}

void IOBuffer::WriteShort(short theShort)
{
	if (!EnsureDataFit(2)) return;

	memcpy(mData+mCursor,&theShort,sizeof(short));
	//*(short*)(mData+mCursor)=theShort;
	mCursor+=2;
}

void IOBuffer::WriteVBIndex(vbindex theShort)
{
	if (!EnsureDataFit(sizeof(vbindex))) return;

	memcpy(mData+mCursor,&theShort,sizeof(vbindex));
	//*(short*)(mData+mCursor)=theShort;
	mCursor+=sizeof(vbindex);
}

void IOBuffer::WriteUShort(unsigned short theShort)
{
	if (!EnsureDataFit(2)) return;

	memcpy(mData+mCursor,&theShort,sizeof(unsigned short));
	//*(unsigned short*)(mData+mCursor)=theShort;
	mCursor+=2;
}


void IOBuffer::WriteInt(int theInt)
{
	if (!EnsureDataFit(4)) return;

	memcpy(mData+mCursor,&theInt,sizeof(int));
	//*(int*)(mData+mCursor)=theInt;
	mCursor+=4;
}

void IOBuffer::WriteLongLong(longlong theInt)
{

	if (!EnsureDataFit(sizeof(longlong))) return;

	memcpy(mData+mCursor,&theInt,sizeof(longlong));
	//*(longlong*)(mData+mCursor)=theInt;
	mCursor+=sizeof(longlong);
}

void IOBuffer::WriteUInt(unsigned int theInt)
{

	if (!EnsureDataFit(4)) return;

	memcpy(mData+mCursor,&theInt,sizeof(unsigned int));
	//*(unsigned int*)(mData+mCursor)=theInt;
	mCursor+=4;
}

void IOBuffer::WriteString(String theString)
{
	WriteString(theString.c());
}

void IOBuffer::WriteString(char *theString)
{
	if (!EnsureDataFit(1)) return;
	WriteInt((int)strlen(theString)+1);
	EnsureDataFit((int)strlen(theString)+1);
	strcpy((char*)(mData+mCursor),theString);

	mCursor+=(int)strlen(theString)+1;
}

void IOBuffer::WriteBytes(void* theBytes, int theSize)
{
	if (!EnsureDataFit(1)) return;
	WriteInt(theSize);
	WriteRaw(theBytes,theSize);
}

void IOBuffer::ReadBytes(void* thePtr)
{
	int aBytes=ReadInt();
	ReadRaw(thePtr,aBytes);
}

int IOBuffer::PeekReadBytes()
{
	if (IsEnd()) return 0;
	if (mCursor+3>=mDataLen) 
	{
		mError=true;
		return 0;
	}

	return *(int*)(mData+mCursor);
}



void IOBuffer::WriteFloat(float theFloat)
{
	if (!EnsureDataFit(sizeof(float))) return;
    memcpy(mData+mCursor,&theFloat,sizeof(float));
//	*(float*)(mData+mCursor)=theFloat;
	mCursor+=sizeof(float);
}

#define PRECISION 100000
float IOBuffer::ReadSafeFloat()
{
	int aWhole=ReadInt();
	int aDecimal=ReadInt();
	float aFloat=(float)aWhole+(float)aDecimal/PRECISION;
	return aFloat;
}

void IOBuffer::WriteSafeFloat(float theFloat)
{
	int aWhole=(int)theFloat;
	int aDecimal=(int)((theFloat-aWhole)*PRECISION);
	WriteInt(aWhole);
	WriteInt(aDecimal);
}


float IOBuffer::ReadFraction()
{
	unsigned char aVal=ReadUChar();
	return ((float)aVal)/255.0f;
}

void IOBuffer::WriteFraction(float theFraction)
{
	unsigned char aVal=(unsigned char)(theFraction*255);
	WriteUChar(aVal);
}

//
// Values to indicate 0, but negative... since our algo won't write the whole as a negative zero...
//
#define SMALLNEGVALUE 32760
#define TINYNEGVALUE 127

float IOBuffer::ReadSmallFloat()
{
	short aWhole=ReadShort();
	unsigned char aDecimal=ReadUChar();

	float aFloat;
	if (aWhole==SMALLNEGVALUE)
	{
		aWhole=0;
		aFloat=(float)aWhole-(1.0f-((float)aDecimal/255.0f));
	}
	else
	{
		if (aWhole>=0) aFloat=(float)aWhole+(float)aDecimal/255.0f;
		else aFloat=(float)aWhole-(1.0f-((float)aDecimal/255.0f));
	}

	return aFloat;
}

void IOBuffer::WriteSmallFloat(float theFloat)
{
	short aWhole=(int)_clamp(-32700,theFloat,32700);
	unsigned char aDecimal;
	if (theFloat>=0) aDecimal=(unsigned char)(_min(1.0f,((theFloat-floorf(theFloat))+.005f))*255.0f);
	else aDecimal=(unsigned char)(((theFloat-ceilf(theFloat))-.005f)*255.0f);

	if (aWhole==0 && theFloat<0) aWhole=SMALLNEGVALUE;	// Tag value, to indicate -0

	WriteShort(aWhole);
	WriteUChar(aDecimal);
}


float IOBuffer::ReadTinyFloat()
{
	char aWhole=ReadChar();
	unsigned char aDecimal=ReadUChar();
	float aFloat;
	if (aWhole==TINYNEGVALUE)
	{
		aWhole=0;
		aFloat=(float)aWhole-(1.0f-((float)aDecimal/255.0f));
	}
	else
	{
		if (aWhole>=0) aFloat=(float)aWhole+(float)aDecimal/255.0f;
		else aFloat=(float)aWhole-(1.0f-((float)aDecimal/255.0f));
	}
	return aFloat;
}

float IOBuffer::ReadTinyUFloat()
{
	unsigned char aWhole=ReadUChar();
	unsigned char aDecimal=ReadUChar();
	float aFloat;
	if (aWhole>=0) aFloat=(float)aWhole+(float)aDecimal/255.0f;
	else aFloat=(float)aWhole-(1.0f-((float)aDecimal/255.0f));
	return aFloat;
}

void IOBuffer::WriteTinyFloat(float theFloat)
{
	char aWhole=(int)_clamp(-125,theFloat,125);
	unsigned char aDecimal;
	if (aWhole>=0) aDecimal=(unsigned char)(_min(1.0f,((theFloat-floorf(theFloat))+.005f))*255.0f);
	else aDecimal=(unsigned char)(((theFloat-ceilf(theFloat))-.005f)*255.0f);

	if (aWhole==0 && theFloat<0) aWhole=TINYNEGVALUE;	// Tag value, to indicate -0

	WriteChar(aWhole);
	WriteUChar(aDecimal);
}

void IOBuffer::WriteTinyUFloat(float theFloat)
{
	unsigned char aWhole=(int)_clamp(0,theFloat,255);
	unsigned char aDecimal;
	if (aWhole>=0) aDecimal=(unsigned char)(_min(1.0f,((theFloat-floorf(theFloat))+.005f))*255.0f);
	else aDecimal=(unsigned char)(((theFloat-ceilf(theFloat))-.005f)*255.0f);
	WriteUChar(aWhole);
	WriteUChar(aDecimal);
}

GlobalID IOBuffer::ReadGlobalID()
{
	GlobalID aID;
	aID.mData[0]=(unsigned int)ReadUInt();
	aID.mData[1]=(unsigned int)ReadUInt();
	aID.mData[2]=(unsigned int)ReadUInt();
	aID.mData[3]=(unsigned int)ReadUInt();

	if (mError) aID.Null();
	return aID;
}

void IOBuffer::WriteGlobalID(GlobalID theID)
{
	WriteUInt((int)theID.mData[0]);
	WriteUInt((int)theID.mData[1]);
	WriteUInt((int)theID.mData[2]);
	WriteUInt((int)theID.mData[3]);
}

void IOBuffer::WriteDouble(double theDouble)
{
	if (!EnsureDataFit(sizeof(double))) return;
    memcpy(mData+mCursor,&theDouble,sizeof(double));
//	*(double*)(mData+mCursor)=theDouble;
	mCursor+=sizeof(double);
}

void IOBuffer::WriteBool(bool theBool)
{
	if (!EnsureDataFit(1)) return;

	memcpy(mData+mCursor,&theBool,sizeof(bool));
	//*(char*)(mData+mCursor)=(theBool==1);
	mCursor++;
}

void IOBuffer::WriteMatrix(Matrix& theMatrix) 
{
	if (!EnsureDataFit(16*4)) return;
	for (int aCount=0;aCount<16;aCount++) WriteFloat(theMatrix.mData.mm[aCount]);
}

void IOBuffer::WriteLine(char *theLine)
{
	if (!EnsureDataFit((int)strlen(theLine)+1)) return;

	strcpy((char*)(mData+mCursor),theLine);
	mCursor+=((int)strlen(theLine));

	EnsureDataFit(2);
	mData[mCursor]=13;mCursor++;
	mData[mCursor]=10;mCursor++;
}

bool IOBuffer::EnsureDataFit(int theDataSize)
{
	if (IsReadOnly()) return false;

	int aDataLen=mCursor+theDataSize;
	if (aDataLen>mDataLen)
	{
		if (aDataLen>mAllocLen)
		{
			char *aNewData=new char[aDataLen];
			memcpy(aNewData,mData,mDataLen);
			_DeleteArray(mData);
			mData=aNewData;

			mAllocLen=aDataLen;
		}

		mDataLen=aDataLen;
	}

	return true;
}


void IOBuffer::Seek(int thePos)
{
	mCursor=thePos;
}

void IOBuffer::Rewind(int howFar)
{
	mCursor-=howFar;
	if (mCursor<0) mCursor=0;
}

void IOBuffer::FastForward(int howFar)
{
	mCursor+=howFar;
	if (mCursor>mDataLen) mCursor=mDataLen;
}


void IOBuffer::SeekEnd()
{
	mCursor=mDataLen;
	if (mCursor<0) mCursor=0;
}

void IOBuffer::SeekBeginning()
{
	mCursor=0;
}

String IOBuffer::ToString(int theLen)
{
	int aLen=_min(theLen,mDataLen);
	if (aLen<=0) aLen=mDataLen;
	if (mDataLen<=0) return "";

    String aStr;
    aStr.strncpy(mData,aLen);
    return aStr;
}

bool DoesFileExist(String theFilename)
{
	return OS_Core::DoesFileExist(theFilename.c());
}

bool IsDirectory(String thePath)
{
	return OS_Core::IsDirectory(thePath.c());
}

bool DownloadWad(String theURL, String theVersion, bool forceDownload)
{
	String aTail=theURL.GetTail('.');
	String aName=URLToFN(theURL.RemoveTail('.'));
	String aFN;

	String aWCFN="cache://";
	aWCFN+=aName;aWCFN+=".version";
	if (!forceDownload)
	{
		if (DoesFileExist(aWCFN)) 
		{
			IOBuffer aBuffer;
			aBuffer.Load(aWCFN);
			if (aBuffer.ReadString()==theVersion) return true;
		}
	}

#ifdef _DEBUG
	gOut.Out(">>>> Downloading WAD: [%s]",theURL.c());
#endif
	aFN="cache://";
	aFN+=aName;
	aFN+=".";
	aFN+=aTail;

	IOBuffer aBuffer;
	aBuffer.Download(theURL);

	ExtractWAD(aBuffer,GetPathFromPath(aFN));
	aBuffer.Reset();
	aBuffer.WriteString(theVersion);
	aBuffer.CommitFile(aWCFN);
	return true;
}

void RemoveFile(String theFilename)
{
#ifdef _WIN32
#pragma warning (disable:4995)
#endif
	OS_Core::DeleteFile(theFilename);
#ifdef _WIN32
#pragma warning (default:4995)
#endif
	if (IsAutoCloudLocation(theFilename)) OS_Core::DeleteCloud(FilenameToCloudKey(theFilename).c());
}

void RemoveFileFromCloud(String theFilename)
{
	OS_Core::DeleteCloud(FilenameToCloudKey(theFilename).c());
}

void EnumDirectory_Legacy(String theDirectoryName, Array<String>& theArray, bool includeSubdirs)
{
	Array<char*> aResult;
	OS_Core::EnumDirectory(theDirectoryName.c(),aResult,includeSubdirs);

	for (int aCount=0;aCount<aResult.Size();aCount++)
	{
		theArray+=aResult[aCount];
		delete [] aResult[aCount];
	}

}


void EnumDirectory(String theDirectoryName, Array<String>& theFileList, bool includeSubDirs)
{
	Array<String> aFiles;
	EnumDirectoryFilesAndFolders(theDirectoryName,aFiles);

	for (int aCount=0;aCount<aFiles.Size();aCount++)
	{
		if (aFiles[aCount].LastChar()=='\\' || aFiles[aCount].LastChar()=='/')
		{
			if (includeSubDirs)
			{
				String aDir=Sprintf("%s%s",theDirectoryName.c(),aFiles[aCount].c());
				EnumDirectory(aDir,theFileList,includeSubDirs);
			}
		}
		else 
		{
			theFileList+=Sprintf("%s%s",theDirectoryName.c(),aFiles[aCount].c());
		}
	}
}

bool RenameFile(String theOldName, String theNewName)
{
	String aFN=GetFilenameFromPath(theOldName);
	bool aResult=true;
	if (IsAutoCloudLocation(theOldName))
	{
		aResult=DoesFileExist(theOldName);
		CopyFile(theOldName,theNewName);
		RemoveFile(theOldName);
	}
	else aResult=OS_Core::RenameFile(theOldName,theNewName);
	return aResult;
}

bool CopyFile(String theOldName, String theNewName)
{
	if (!DoesFileExist(theOldName)) return false;

	IOBuffer aBuffer;
	aBuffer.Load(theOldName);
	aBuffer.CommitFile(theNewName);
	return true;
}

void MakeDirectory(String theFilename)
{
	OS_Core::MakeDirectory(theFilename);
}

void EnumDirectoryFiles(String theDirectoryName, Array <String>&theArray)
{
	Array<char*> aResult;
	OS_Core::EnumDirectory(theDirectoryName.c(),aResult,false);
	for (int aCount=0;aCount<aResult.Size();aCount++) 
	{
		String aFN=aResult[aCount];
		if (!aFN.StartsWith("::")) theArray+=aFN;
		delete [] aResult[aCount];
	}
}

void EnumDirectoryFolders(String theDirectoryName, Array <String>&theArray)
{
	Array<char*> aResult;
	OS_Core::EnumDirectory(theDirectoryName.c(),aResult,true);
	for (int aCount=0;aCount<aResult.Size();aCount++) 
	{
		String aFN=aResult[aCount];
		if (aFN.StartsWith("::")) theArray+=aFN.GetSegment(2,aFN.Len());
		delete [] aResult[aCount];
	}
}

void EnumDirectoryFilesAndFolders(String theDirectoryName, Array <String>&theArray)
{
	Array<char*> aResult;
	OS_Core::EnumDirectory(theDirectoryName.c(),aResult,true);
	for (int aCount=0;aCount<aResult.Size();aCount++) 
	{
		String aFN=aResult[aCount];
		if (aFN.StartsWith("::"))
		{
			aFN=aFN.GetSegment(2,aFN.Len());
			aFN+="/";
		}
		theArray+=aFN;
		delete [] aResult[aCount];
	}
}


void EnumFiles(String theDirectoryName, Array<String> &theArray)
{
	Array<char*> aResult;
	OS_Core::EnumDirectory(theDirectoryName.c(),aResult,true);

	for (int aCount=0;aCount<aResult.Size();aCount++)
	{
		if (aResult[aCount][0]==':')
		{
			String aStr=aResult[aCount];
			String aSearch=Sprintf("%s\\%s",theDirectoryName.c(),aStr.GetSegment(2,MAX_PATH).c());
			EnumFiles(aSearch,theArray);
		}
		else
		{
			theArray[theArray.Size()]=Sprintf("%s\\%s",theDirectoryName.c(),aResult[aCount]);
		}
	}

	for (int aCount=0;aCount<aResult.Size();aCount++) delete [] aResult[aCount];
}

void DeleteCacheFiles(String theFolder)
{
	Array<String> aFiles;
	EnumDirectory(theFolder,aFiles);

	for (int aCount=0;aCount<aFiles.Size();aCount++)
	{
		//if (aFiles[aCount][0]==':') DeleteCacheFiles(Sprintf("%s%s\\",theFolder.c(),aFiles[aCount].GetSegment(2,9999).c()));
		//else if (aFiles[aCount].Contains("._cache")) DeleteFile(Sprintf("%s%s",theFolder.c(),aFiles[aCount].c()));
		if (aFiles[aCount].Contains("._cache")) RemoveFile(aFiles[aCount].c());
	}
}

void ClearTemp(String theSubDir)
{
	String aDir="temp://";aDir+=theSubDir;
	DeleteDirectory(aDir);
}


void DeleteDirectory(String theDirectoryName)
{
	if (IsAutoCloudLocation(theDirectoryName))
	{
		DeleteDirectoryContents(theDirectoryName);
		OS_Core::DeleteCloud(FilenameToCloudKey(theDirectoryName).c());
	}
	OS_Core::DeleteDirectory(theDirectoryName);
}

void DeleteDirectoryContents(String theDirectoryName)
{
	Array<String> aFiles;
	EnumDirectoryFilesAndFolders(theDirectoryName,aFiles);
	for (int aCount=0;aCount<aFiles.Size();aCount++)
	{
		if (aFiles[aCount].LastChar()=='\\') 
		{
			String aDir=Sprintf("%s\\%s",theDirectoryName.c(),aFiles[aCount].c());
			DeleteDirectoryContents(aDir);
			DeleteDirectory(aDir);
		}
		else RemoveFile(Sprintf("%s\\%s",theDirectoryName.c(),aFiles[aCount].c()));
	}
}

void IOBuffer::Copy(IOBuffer *theBuffer)
{
	if (mAllocLen>=theBuffer->mAllocLen)
	{
		//
		// Quick escape... size matches, copy and get out!
		//
		mDataLen=theBuffer->mDataLen;
		if (mDataLen>0) memcpy(mData,theBuffer->mData,mDataLen);
		return;
	}

	if (!IsDataIsPointer()) {_DeleteArray(mData);}
	SetDataIsPointer(false);
	SetDownloading(false);

	mDataLen=theBuffer->mDataLen;
	mAllocLen=mDataLen;
	mCursor=theBuffer->mCursor;

	mData=new char[mDataLen];
	memcpy(mData,theBuffer->mData,mDataLen);
}

void IOBuffer::Copy(IOBuffer *theBuffer, int theStartPos, int theLength)
{
	if (!IsDataIsPointer()) {_DeleteArray(mData);}
	SetDataIsPointer(false);
	SetDownloading(false);

	mDataLen=theLength;
	mAllocLen=mDataLen;
	mCursor=0;

	mData=new char[mDataLen];
	memcpy(mData,theBuffer->mData+theStartPos,mDataLen);
}

void IOBuffer::Copy(char* theData, int theLen)
{
	if (mAllocLen>=theLen)
	{
		//
		// Quick escape... size matches, copy and get out!
		//
		mDataLen=theLen;
		if (mDataLen>0) memcpy(mData,theData,mDataLen);
		return;
	}

	if (!IsDataIsPointer()) {_DeleteArray(mData);}
	SetDataIsPointer(false);
	SetDownloading(false);

	mDataLen=theLen;
	mAllocLen=mDataLen;
	mData=new char[mDataLen];
	memcpy(mData,theData,mDataLen);
	mCursor=0;
}


void IOBuffer::Append(IOBuffer* theBuffer)
{
	int aSize=mDataLen;
	if (GuaranteeSize(aSize+theBuffer->mDataLen)) memcpy(&mData[aSize],theBuffer->mData,theBuffer->mDataLen);
}


String IOBuffer::ReadCLine(String theTerminators, bool includeBreaker)
{
	if (IsEnd()) 
	{
		String aNullString;
		return aNullString;
	}

	static Array<char> aResultArray;
	aResultArray[MAX_PATH]=0;
	int aPlotPos=0;
	aResultArray[aPlotPos]=0;

	bool aPreRead=true;
	bool aInShortComment=false;
	bool aInLongComment=false;
	bool aInQuote=false;
	bool aInDoubleQuote=false;
	bool aInExactRead=false;
	char aIgnoreDouble=0;
		
	for (;;)
	{
		aResultArray[aPlotPos]=0;
		if (IsEnd()) break;
		char aChar=mData[mCursor++];

		if (aChar=='@')
		{
			if (mCursor<mDataLen-2)
			{
				if (mData[mCursor]=='-' && mData[mCursor+1]=='-')
				if (!aInShortComment && !aInLongComment && !aInQuote && !aInDoubleQuote && !aInExactRead)
				{
					mCursor++;
					mCursor++;
					aInExactRead=true;
					continue;
				}
			}
		}

		if (aInExactRead)
		{
			if (aChar=='-') 
			{
				if (mCursor<mDataLen-2)
				{
					if (mData[mCursor]=='-' && mData[mCursor+1]=='@')
					{
						aInExactRead=false;
						mCursor+=2;
					}
				}
			}
			if (aInExactRead) aResultArray[aPlotPos++]=aChar;
			continue;
		}


		if (aChar==10) 
		{
			aPreRead=true;
			aInShortComment=false;
		}
		if (aPreRead) if (aChar<=32) continue;

		if (aChar==aIgnoreDouble)
		{
			if (aChar==34) aInDoubleQuote=!aInDoubleQuote;
			if (aChar==39) aInQuote=!aInQuote;
			aIgnoreDouble=0;
			aPlotPos--;
			continue;
		}



		if (aInLongComment)
		{
			if (aChar=='*') if (mCursor<mDataLen-1) if (mData[mCursor]=='/') {aInLongComment=false;mCursor++;}
			continue;
		}

		if (!aInShortComment && !aInLongComment)
		{
			if (aInDoubleQuote) if (aChar==34) {aIgnoreDouble=aChar;}
			if (aInQuote) if (aChar==39) {aIgnoreDouble=aChar;}

			if (aChar==34 && !aInQuote) aInDoubleQuote=!aInDoubleQuote;
			if (aChar==39 && !aInDoubleQuote) aInQuote=!aInQuote;
		}

		if (!aInQuote && !aInDoubleQuote)
		{
			if (aChar=='/') if (mCursor<mDataLen-1) if (mData[mCursor]=='/') aInShortComment=true;
			if (aChar=='/') if (mCursor<mDataLen-1) if (mData[mCursor]=='*') aInLongComment=true;
		}
		if (aInShortComment || aInLongComment) aChar=0;

		if (theTerminators.Contains(aChar) && (!aInQuote && !aInDoubleQuote))
		{
			if (includeBreaker) {aResultArray[aPlotPos++]=aChar;aResultArray[aPlotPos++]=0;}
			break;
		}
		if (aChar>=32) 
		{
			if (aPlotPos==0) if (aChar=='#') {theTerminators.AppendCRLF();}
			aResultArray[aPlotPos++]=aChar;
			aPreRead=false;
		}
		else
		{
			if (!aPreRead) if (aResultArray[aPlotPos-1]!=32) aResultArray[aPlotPos++]=32;
		}
	}

	String aResult=aResultArray.mArray;
	return aResult;
}

void IOBuffer::ReadBuffer(IOBuffer &theBuffer)
{
	theBuffer.Reset();
	int aLength=ReadInt();
	if (!theBuffer.EnsureDataFit(aLength)) return;
	ReadRaw(theBuffer.mData,aLength);
}

void IOBuffer::WriteBuffer(IOBuffer &theBuffer)
{
	WriteInt(theBuffer.mDataLen);
	WriteRaw(theBuffer.mData,theBuffer.mDataLen);
}

void IOBuffer::ExportAsCCharList(String theFilename)
{
	 String aString=Sprintf("char aCharList[%d]={",mDataLen);

	 char *aPtr=mData;
	 for (int aCount=0;aCount<mDataLen;aCount++)
	 {
		 if (aCount==0) aString+=Sprintf("%d",*aPtr);
		 else aString+=Sprintf(",%d",*aPtr);
		 aPtr++;
	 }

	 aString+="};";

	 IOBuffer aBuffer;
	 aBuffer.WriteLine(aString.c());
	 aBuffer.CommitFile(theFilename);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Misc helper functions...
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
String FixFN(String theFN)
{
	String aFix;
	aFix="desktop://";if (theFN.StartsWith(aFix)) {return PointAtDesktop(theFN.GetSegment(aFix.Len(),theFN.Len()));}
	aFix="sandbox://";if (theFN.StartsWith(aFix)) {return PointAtSandbox(theFN.GetSegment(aFix.Len(),theFN.Len()));}
	aFix="cloud://";if (theFN.StartsWith(aFix)) {return PointAtCloud(theFN.GetSegment(aFix.Len(),theFN.Len()));}
	aFix="cache://";if (theFN.StartsWith(aFix)) {return PointAtCache(theFN.GetSegment(aFix.Len(),theFN.Len()));}
	aFix="temp://";if (theFN.StartsWith(aFix)) {return PointAtTemp(theFN.GetSegment(aFix.Len(),theFN.Len()));}
	aFix="package://";if (theFN.StartsWith(aFix)) {return PointAtPackage(theFN.GetSegment(aFix.Len(),theFN.Len()));}
	aFix="local://";if (theFN.StartsWith(aFix)) {return PointAtPackage(theFN.GetSegment(aFix.Len(),theFN.Len()));}
	aFix="images://";if (theFN.StartsWith(aFix)) {return PointAtImages(Sprintf("%s",theFN.GetSegment(aFix.Len(),theFN.Len()).c()));}
	aFix="data://";if (theFN.StartsWith(aFix)) {return PointAtPackage(Sprintf("data\\%s",theFN.GetSegment(aFix.Len(),theFN.Len()).c()));}
	aFix="sounds://";if (theFN.StartsWith(aFix)) {return PointAtPackage(Sprintf("sounds\\%s",theFN.GetSegment(aFix.Len(),theFN.Len()).c()));}
	aFix="music://";if (theFN.StartsWith(aFix)) {return PointAtPackage(Sprintf("music\\%s",theFN.GetSegment(aFix.Len(),theFN.Len()).c()));}
	aFix="profile://";if (theFN.StartsWith(aFix)) {return gAppPtr->PointAtProfile(theFN.GetSegment(aFix.Len(),theFN.Len()));}
	return theFN;
}
*/

String URLToFN(String theURL)
{
	theURL.DeleteI("http://");
	theURL.DeleteI("https://");
	theURL.Replace("mm://","makermall_");
	theURL.Replace('.','_');
	theURL.Replace('/','_');
	theURL.Replace('\\','_');
	theURL.Replace('?','_');
	theURL.Replace('&','_');
	theURL.Replace('=','_');
	theURL.Replace('"','_');
	theURL.Replace('+','_');
	theURL.Replace('%','_');
	theURL.Replace(' ','_');
	return theURL;
}

String URLtoWebCache(String theURL, String theVersion)
{
	String aTail=theURL.GetTail('.');
	String aFN="cache://";
	aFN+=URLToFN(theURL);//.RemoveTail('.'));
	if (theVersion.Len()) {aFN+='.';aFN+=theVersion;}
	return aFN;
}


/*
String PointAtDesktop(String theString)
{
	String aStr="desktop:\\";aStr+=theString;
	return aStr;
}
*/

/*
String gRouteSandbox;
String gSandboxDir;

String RouteSandbox(String theNewSandbox) 
{
	gSandboxDir="";
	gRouteSandbox=theNewSandbox;
	#ifdef _DEBUG
	gOut.Out("$New Sandbox Route: [%s]",gRouteSandbox.c());
	#endif
	MakeDirectory(gRouteSandbox);
	return gRouteSandbox;
}

String gRouteCloud;
String gCloudDir;
String RouteCloud(String theNewCloud) 
{
	gCloudDir="";
	gRouteCloud=theNewCloud;
	#ifdef _DEBUG
	gOut.Out("$New Cloud Route: [%s]",gRouteCloud.c());
	#endif
	MakeDirectory(gRouteCloud);
	return gRouteCloud;
}
*/

/*
String PointAtSandbox(String theString)
{
	String aResult="sandbox://";aResult+=theString;
	return aResult;
}
*/

/*
String PointAtCloud(String theString)
{
	String aResult="cloud://";aResult+=theString;
	return aResult;
}
*/

/*
String PointAtCache(String theString)
{
	String aResult="cache://";aResult+=theString;
	return aResult;
}
*/

/*
String PointAtPackage(String theString)
{
	String aResult="package://";aResult+=theString;
	return aResult;
}
*/

/*
String PointAtImages(String theString)
{
	String aStr="images://";aStr+=theString;
	return aStr;
}
*/
/*
String PointAtModels(String theString)
{
	String aStr="models:\\";aStr+=theString;
	return aStr;
}
*/

/*
String PointAtTemp(String theString)
{
	String aStr="temp:\\";aStr+=theString;
	return aStr;
	//return PointAtCache(Sprintf("temp\\%s",theString.c()));
}
*/

void Execute(String theCommand)
{
	OS_Core::Execute(theCommand);
}

void OpenURL(String theURL)
{
	Execute(theURL);
}

String GetTruePath(String thePath)
{
	char aResult[MAX_PATH];
	Common::FixPath(thePath.c(),aResult);
	return aResult;
}


String InsertFilePrefix(String thePath, String thePrefix)
{
	int aCount;
	for (aCount=thePath.Len()-1;aCount>0;aCount--)
	{
		if (thePath[aCount]=='\\' || thePath[aCount]=='/')
		{
			aCount++;
			break;
		}
	}

	String aResult=thePath;
	aResult.Insert(thePrefix,aCount);
	return aResult;
}

String GetFilenameFromPath(String thePath)
{
	int aCount;
	for (aCount=thePath.Len()-1;aCount>0;aCount--)
	{
		if (thePath[aCount]=='\\' || thePath[aCount]=='/')
		{
			aCount++;
			break;
		}
	}

	return thePath.GetSegment(aCount,thePath.Len());
}

String GetLastFolderFromPath(String thePath)
{
	int aCount;
	bool aFoundSlash=false;
	for (aCount=thePath.Len()-1;aCount>0;aCount--)
	{
		if (thePath[aCount]=='\\' || thePath[aCount]=='/') aFoundSlash=true;
		else if (aFoundSlash) break;
	}

	return GetFilenameFromPath(thePath.GetSegment(0,aCount+1));
}

String GetPathFromPath(String thePath)
{
	int aCount;
	for (aCount=thePath.Len()-1;aCount>0;aCount--)
	{
		if (thePath[aCount]=='\\' || thePath[aCount]=='/')
		{
			aCount++;
			break;
		}
	}

	return thePath.GetSegment(0,aCount);
}

String GetUpPathFromPath(String thePath)
{
	int aStartAt=thePath.Len()-1;
	if (thePath[thePath.Len()-1]=='\\' || thePath[thePath.Len()-1]=='/') aStartAt--;
	int aCount;
	for (aCount=aStartAt;aCount>0;aCount--)
	{
		if (thePath[aCount]=='\\' || thePath[aCount]=='/')
		{
//			aCount++;
			break;
		}
	}

	return thePath.GetSegment(0,aCount);
}

QuickStack gStack;
void QuickStack::Push(void *theMemory, int theLength)
{
	WriteRaw(theMemory,theLength);
}

void QuickStack::Pop(void *theMemory, int theLength)
{
	mCursor-=theLength;
	ReadRaw(theMemory,theLength);
	mCursor-=theLength;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Written for Android, but can be used anywhere...
// Binds the contents of a folder into a single file which can then be extracted later.
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void MakeWAD(String theFolder, IOBuffer& theBuffer, String ignoreExtensions, String ignoreFilenames,bool isRecursive)
{
	ignoreExtensions+=";";
	ignoreFilenames+=";";
	static String aWadBase;
	if (!isRecursive) 
	{
		String aStr=WADCODE;
		theBuffer.WriteRaw(aStr.mData,aStr.Len());
		aWadBase=theFolder;
	}
	//
	// ignoreExtensions should be a string like this:
	// exe;dll;obj;
	//
	String aMyFolder=theFolder;
	aMyFolder.Delete(aWadBase);

	Array<String> aFilenameArray;
	EnumDirectoryFilesAndFolders(theFolder,aFilenameArray);
	for (int aCount=0;aCount<aFilenameArray.Size();aCount++)
	{
		if (aFilenameArray[aCount].LastChar()=='\\' || aFilenameArray[aCount].LastChar()=='/')
		{
			bool aIgnore=false;
			if (ignoreFilenames.Len())
			{
				String aFN=aFilenameArray[aCount].GetSegment(0,aFilenameArray[aCount].Len()-1);aFN+=";";
				if (ignoreFilenames.FindI(aFN)>=0) aIgnore=true;
			}

			if (!aIgnore)
			{
				String aFolder=theFolder;
				if (aFolder.LastChar()!='/' && aFolder.LastChar()!='\\') aFolder+="/";
				aFolder+=aFilenameArray[aCount];
				if (aFolder.LastChar()!='/' && aFolder.LastChar()!='\\') aFolder+="/";
				MakeWAD(aFolder,theBuffer,ignoreExtensions,ignoreFilenames,true);
			}
		}
		else 
		{
			bool aIgnore=false;

			if (ignoreFilenames.Len())
			{
				String aFN=aFilenameArray[aCount];aFN+=";";
				if (ignoreFilenames.FindI(aFN)>=0) aIgnore=true;
			}

			if (!aIgnore) if (ignoreExtensions.Len())
			{
				String aExtension=aFilenameArray[aCount].GetTail('.');
				if (aExtension.Len())
				{
					aExtension+=';';
					if (ignoreExtensions.FindI(aExtension)>=0) aIgnore=true;
				}
			}
			if (!aIgnore)
			{
				String aFN=aMyFolder;
				aFN+="/";
				aFN+=aFilenameArray[aCount];

				IOBuffer aImport;

#ifdef _DEBUG
				gOut.Out("Wadding...%s\\%s",theFolder.c(),aFilenameArray[aCount].c());
#endif

				aImport.Load(Sprintf("%s/%s",theFolder.c(),aFilenameArray[aCount].c()));

				theBuffer.WriteString(aFN);
				theBuffer.WriteBuffer(aImport);
			}
		}

	}

	if (!isRecursive) aWadBase="";

/*
	//
	// Ye olde version... doesn't do subdirs...
	//

	EnumFiles(theFolder,aFilenameArray);
	for (int aCount=0;aCount<aFilenameArray.Size();aCount++)
	{
		//
		// Bring each file into the WAD...
		//
		String aLocalString=aFilenameArray[aCount];
		aLocalString.Delete(0,theFolder.Len()+1);

		bool aIgnore=false;
		if (ignoreExtensions.Len())
		{

			String aExtension;
			for (int aGet=aLocalString.Len();aGet>=0;aGet--)
			{
				if (aLocalString[aGet]=='.')
				{
					aExtension=aLocalString.GetSegment(aGet+1,MAX_PATH);
					break;
				}
			}

			if (aExtension.Len())
			{
				aExtension+=';';
				if (ignoreExtensions.FindI(aExtension)>=0) aIgnore=true;
			}
		}


		if (!aIgnore)
		{
			IOBuffer aBuffer;
			aBuffer.Load(aFilenameArray[aCount]);
			theBuffer.WriteString(aLocalString);
			theBuffer.WriteBuffer(aBuffer);
		}
	}
*/
}

void MakeWAD(String theFolder, String theOutFile, String ignoreExtensions, String ignoreFilenames)
{
	IOBuffer aWADBuffer;
	MakeWAD(theFolder,aWADBuffer,ignoreExtensions,ignoreFilenames);
	aWADBuffer.CommitFile(theOutFile);
}

void ExtractWAD(IOBuffer& theBuffer, String theOutFolder, Array<String>* theCreatedFiles)
{
	if (!IsWAD(theBuffer)) return;
/*
	bool aBad=false;
	String aWadCode=WADCODE;
	for (int aCount=0;aCount<aWadCode.Len();aCount++) if (aWadCode[aCount]!=theBuffer.mData[aCount]) {aBad=true;break;}

	if (aBad)
	//if (!memcmp(WADCODE,theBuffer.mData,strlen(WADCODE)))
	{
		theBuffer.mData[strlen(WADCODE)]=0;
		gOut.Out("Invalid WAD file [%s]!!!!",theBuffer.ToString().c());

		return;
	}
*/
	theBuffer.Seek(strlen(WADCODE));
	while (!theBuffer.IsEnd())
	{
		IOBuffer aLocal;

		String aLocalString=theBuffer.ReadString();
		theBuffer.ReadBuffer(aLocal);

		String aFN=Sprintf("%s\\%s",theOutFolder.c(),aLocalString.c());

		MakeDirectory(aFN);
		aLocal.CommitFile(aFN);
		if (theCreatedFiles) theCreatedFiles->Add(aFN);
	}
}

void ExtractWAD(String theInFile, String theOutFolder, Array<String>* theCreatedFiles)
{
	IOBuffer aBuffer;
	aBuffer.Load(theInFile);
	ExtractWAD(aBuffer,theOutFolder,theCreatedFiles);
}

bool IOBuffer::IsWAD()
{
	return ::IsWAD(*this);
}

bool IsWAD(IOBuffer& theBuffer)
{
	bool aBad=false;
	String aWadCode=WADCODE;
	if (theBuffer.Len()<=aWadCode.Len()) return false;

	for (int aCount=0;aCount<aWadCode.Len();aCount++) if (aWadCode[aCount]!=theBuffer.mData[aCount]) {aBad=true;break;}
	return !aBad;
}

bool IsWAD(String theInFile)
{
	IOBuffer aBuffer;
	aBuffer.Load(theInFile);
	return IsWAD(aBuffer);
}

void FileToArray(String theFilename,Array<String>& theArray)
{
	IOBuffer aBuffer;
	aBuffer.Load(theFilename);
	while (!aBuffer.IsEnd()) theArray+=aBuffer.ReadLine();
}

String FileToString(String theFilename)
{
	IOBuffer aBuffer;
	aBuffer.Load(theFilename);
	return aBuffer.ToString();
}


void ArrayToFile(String theFilename,Array<String>& theArray)
{
	IOBuffer aBuffer;
	for (int aCount=0;aCount<theArray.Size();aCount++) aBuffer.WriteLine(theArray[aCount]);
	aBuffer.CommitFile(theFilename);
}

longlong IOBuffer::Hash()
{
	longlong aHash = 5381;

	char *aPtr=mData;
	char *aEnd=mData+mDataLen;

	while (aPtr<aEnd) {aHash=((aHash<<5)+aHash)+(*aPtr);aPtr++;}
	return aHash;
}

void IOBuffer::WriteSuperbits(SuperBits& theBits)
{
	WriteInt(theBits.mBits.Size());
	for (int aCount=0;aCount<theBits.mBits.Size();aCount++) WriteInt(theBits.mBits[aCount]);
}

void IOBuffer::ReadSuperbits(SuperBits& theBits)
{
	int aBCount=ReadInt();
	theBits.mBits.Reset();
	theBits.mBits.GuaranteeSize(aBCount);
	for (int aCount=0;aCount<aBCount;aCount++) theBits.mBits[aCount]=ReadInt();
}

bool IOBuffer::IsRML()
{
	if (mData) if (memcmp(mData,RMLINDICATOR,_min(RMLINDICATORLEN,Len()))==0) return true;
	return false;
}

void IOBuffer::ReadRandom(Random& theRandom)
{
	for (int aCount=0;aCount<theRandom.GetStateDataSize();aCount++) theRandom.GetStateData(aCount)=ReadInt();
}

void IOBuffer::WriteRandom(Random& theRandom)
{
	for (int aCount=0;aCount<theRandom.GetStateDataSize();aCount++) WriteInt(theRandom.GetStateData(aCount));
}

int IOBuffer::GetCRC()
{
	int aTotal=0;
	char* aPtr=mData;
	int aPoly=0x82f63b78;

	for (int aCount=0;aCount<Len();aCount++) {aTotal^=*aPtr;for (int aBit=0;aBit<8;aBit++) aTotal=aTotal&1?(aTotal>>1)^aPoly : aTotal>>1;aPtr++;}
	return aTotal;
}

longlong IOBuffer::GetLongCRC()
{
	longlong aTotal=0;
	char* aPtr=mData;
	longlong aPoly=0x82f63b7887b36f28;
	for (int aCount=0;aCount<Len();aCount++) {aTotal^=*aPtr;for (int aBit=0;aBit<8;aBit++) aTotal=aTotal&1?(aTotal>>1)^aPoly : aTotal>>1;aPtr++;}
	return aTotal;
}



void CopyDirectory(String theSource, String theDestination, float* theProgress)
{
	Array<String> aFiles;
	EnumDirectory(theSource,aFiles,true);

	if (theDestination.LastChar()!='\\' && theDestination.LastChar()!='/') theDestination+='/';
	if (theProgress) *theProgress=0;

	float aMax=(float)aFiles.Size();

	for (int aCount=0;aCount<aFiles.Size();aCount++)
	{
		String aFile=aFiles[aCount].GetSegment(theSource.Len(),9999);
		String aDest=theDestination;
		aDest+=aFile;

		MakeDirectory(aDest);
		//gOut.Out("! Calling Copyfile: %s",aFiles[aCount].c());
		CopyFile(aFiles[aCount],aDest);

		if (theProgress) *theProgress=((float)aCount)/aMax;
	}
	if (theProgress) *theProgress=1;
}

int gPendingDownloadCount=0;
class BufferDownloader : public Object
{
public:
	Smart(IOBuffer) mBuffer;
	String mURL;
	String mDestination;
	bool mWaiting=true;
	float* mProgress=NULL;

	HOOKARGPTR mHook=NULL;
	void* mHookParam=NULL;

	void Update()
	{
		if (mWaiting)
		{
			if (gPendingDownloadCount<3)
			{
				mBuffer->DownloadA(mURL,mProgress);
				gPendingDownloadCount++;
				mWaiting=false;
			}

		}
		else if (!mBuffer->IsDownloading())
		{
			gPendingDownloadCount=_max(0,gPendingDownloadCount-1);
			if (mDestination.Len()) mBuffer->CommitFile(mDestination);
			if (mHook) mHook(mHookParam);
			Kill();
		}
	}
};

bool DoesFileExist(String theFilename, String theVersion)
{
	String theVersionFN=theFilename;
	theVersionFN+=".";
	theVersionFN+=theVersion;
	return (DoesFileExist(theFilename) && DoesFileExist(theVersionFN));
}


Smart(IOBuffer) DownloadFile(String theURL, String theDestination, String theVersion,float* theProgressIndicator, HOOKARGPTR runWhenComplete, void* theArg)
{
	String theVersionFN=theDestination;
	theVersionFN+=".";
	theVersionFN+=theVersion;

	if (DoesFileExist(theDestination,theVersion))
	{
		Smart(IOBuffer) aBuffer=new IOBuffer;
		aBuffer->Load(theDestination);
		if (theProgressIndicator) *theProgressIndicator=1.0f;
		return aBuffer;
	}

	IOBuffer aBuffer;
	aBuffer.WriteRaw(theVersion,theVersion.Len());
	aBuffer.CommitFile(theVersionFN);

	BufferDownloader* aB=new BufferDownloader;
	aB->mURL=theURL;
	aB->mDestination=theDestination;
	aB->mHook=runWhenComplete;
	aB->mHookParam=theArg;
	aB->mBuffer=new IOBuffer;
	aB->mProgress=theProgressIndicator;
	aB->mBuffer->mStatus&=IOBUFFER_STATUS_DOWNLOADING; // So that we see it as downloading before we even start...
	gAppPtr->AddBackgroundProcess(aB);

	return aB->mBuffer;
}

void MakeArchive(String theArchiveName, String theFolder, float* theProgress, String ignoreExtensions, String ignoreFilenames)
{
	ignoreExtensions+=";";
	ignoreFilenames+=";";
	Array<String> aFileArray;
	Array<char*> aCArray;
	Array<String> aIgnoreFN;
	Utils::CSVToArray(ignoreFilenames,aIgnoreFN,';');

	String aGameIDName=Sprintf("%s\\__Archive_Archive_Archive_Archive__",theFolder.c());
	IOBuffer aGameID;aGameID.WriteRaw(gAppPtr->mAppName.c(),gAppPtr->mAppName.Len());aGameID.CommitFile(aGameIDName);

	EnumDirectory(theFolder,aFileArray,true);
	for (int aCount=0;aCount<aFileArray.Size();aCount++)
	{
		bool aIgnore=false;
		if (ignoreExtensions.Len())
		{
			String aExtension=aFileArray[aCount].GetTail('.');
			if (aExtension.Len())
			{
				aExtension+=';';
				if (ignoreExtensions.FindI(aExtension)>=0) 
				{
					gOut.Out("      Ignore because of extension = %s",aExtension.c());
					aIgnore=true;
				}
			}
		}
		if (!aIgnore) if (aIgnoreFN.Size())
		{
			String aTest;
			for (int aCC=0;aCC<aIgnoreFN.Size();aCC++)
			{
				aTest=aIgnoreFN[aCC];if (aTest.LastChar()!='.') aTest+=".";if (aFileArray[aCount].FindI(aTest)>=0) {aIgnore=true;break;}
				aTest=aIgnoreFN[aCC];if (aTest.LastChar()!='\\') aTest+="\\";if (aFileArray[aCount].FindI(aTest)>=0) {aIgnore=true;break;}
				aTest=aIgnoreFN[aCC];if (aTest.LastChar()!='/') aTest+="/";if (aFileArray[aCount].FindI(aTest)>=0) {aIgnore=true;break;}
			}
		}

		if (!aIgnore) aCArray+=aFileArray[aCount].c();
	}

	if (aCArray.Size()) OS_Core::Archive(theFolder.c(),aCArray,theArchiveName.c(),theProgress);
	RemoveFile(aGameIDName);
}

void ExtractArchive(String theArchiveName, String theFolder, float* theProgress)
{
	if (DoesFileExist(theArchiveName)) OS_Core::UnArchive(theFolder.c(),theArchiveName.c(),theProgress);
	String aGameIDName=Sprintf("%s\\__Archive_Archive_Archive__",theFolder.c());RemoveFile(aGameIDName);
}

