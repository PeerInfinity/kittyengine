#include "network_core.h"
#include "os_core.h"


#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>

#define __HEADER
#include "common.h"
#undef __HEADER


#include <emscripten/fetch.h>

namespace Network_Core
{
	bool				gIsStarted=false;
	char*				gAgent="Mozilla/5.0";
	int					gTimeOut=10*60;
	char				gUploadFormParam[MAX_PATH];
	char				gUploadFormValue[MAX_PATH];
	bool				gCancel=false;
	int					gProcessCount=0;

	void SetUploadForms(const char* theParam, const char* theValue)
	{
		strcpy(gUploadFormParam,theParam);
		strcpy(gUploadFormValue,theValue);
	}

	struct TransferStruct 
	{
		TransferStruct()
		{
			mMemory=(char*)malloc(1);
			mSize=0;
			mTransferSize=0;
			mProgress=NULL;
		}

		char *mMemory;
		size_t mSize;
		size_t mTransferSize;
		float* mProgress;
	};

	void Startup()
	{
		if (!gIsStarted) 
		{
			strcpy(gUploadFormParam,"upload_file");
			strcpy(gUploadFormValue,"file.dat");
		}
		gIsStarted=true;
	}

	void Shutdown()
	{
		while (gProcessCount>0) {}	// Wait for any processes to end...
		gIsStarted=false;
	}

	void Cancel()
	{
		gCancel=true;
	}

	bool Download(char* theURL, char** theDownloadBuffer, int* theDownloadBufferSize, float* theProgressIndicator)
	{
		if (!gIsStarted) return false;
		/*
		gCancel=false;
		gProcessCount++;
		bool aResult=false;
		// Download the file (synchronous!)
		
		if (!gCancel)
		{
			if (theProgressIndicator) *theProgressIndicator=1.0f;
			if (theDownloadBuffer) *theDownloadBuffer=NULL;
			if (theDownloadBufferSize) *theDownloadBufferSize=0;
			//if (theDownloadBuffer) *theDownloadBuffer=aResultStruct.mMemory;
			//if (theDownloadBufferSize) *theDownloadBufferSize=aResultStruct.mSize;
		}
		

		gProcessCount--;
		return aResult;
		*/
		OS_Core::Printf("FAIL!  Can't down Network_Core::Download in WASM... use DownloadA instead!");
		exit(0);
		return false;
	}
	
	//
	// Helpers for Emscriptem Fetch...
	//
	struct FetchStruct
	{
		char** mDownloadBuffer=NULL;
		int* mDownloadBufferSize=NULL;
		float* mProgressIndicator=NULL;
		
		std::function<void(void* theArg)> mFunction;
		void* mArg=NULL;
	};
	
	void _downloadProgress(emscripten_fetch_t* theData)
	{
		FetchStruct* aFS=(FetchStruct*)theData->userData;
		if (aFS->mProgressIndicator)
		{
			if (theData->totalBytes) *aFS->mProgressIndicator=(float)theData->dataOffset/(float)theData->totalBytes;
			else *aFS->mProgressIndicator=0.0f;// If we can't tell them the progress, just lock it at zero... //(float)(theData->dataOffset + theData->numBytes);
			//OS_Core::Printf("DOWNLOADPROGRESS: %s -> %f",theData->url,*aFS->mProgressIndicator);
		}
	}
	
	void _downloadSucceeded(emscripten_fetch_t* theData)
	{
		FetchStruct* aFS=(FetchStruct*)theData->userData;
		char* aDownloadData = new char[theData->numBytes];
		memcpy((void*)(aDownloadData),theData->data,theData->numBytes);

		//OS_Core::Printf("DOWNLOADSUCCEEDED: %s",theData->url);
		//OS_Core::Printf("Size = %d",theData->numBytes);
		
		*aFS->mDownloadBuffer=aDownloadData;
		*aFS->mDownloadBufferSize=theData->numBytes;
		if (aFS->mProgressIndicator) *aFS->mProgressIndicator=1.0f;
		emscripten_fetch_close(theData);
		if (aFS->mFunction!=NULL) 
		{
			//OS_Core::Printf("Run Post Function [%d]",aFS->mArg);
			aFS->mFunction(aFS->mArg);
			//OS_Core::Printf("Run Post Function complete!");
		}
		delete aFS;
	}

	void _downloadFailed(emscripten_fetch_t* theData)
	{
		//OS_Core::Printf("Download FAILED in!");
		
		FetchStruct* aFS=(FetchStruct*)theData->userData;
		*aFS->mDownloadBufferSize=0;
		*aFS->mDownloadBuffer=NULL;
		if (aFS->mProgressIndicator) *aFS->mProgressIndicator=1.0f;
		
		emscripten_fetch_close(theData);
		if (aFS->mFunction!=NULL) aFS->mFunction(aFS->mArg);
		
		//OS_Core::Printf("DOWNLOADFAILED: %s : %d",theData->url,theData->status);
		delete aFS;
	}

	void DownloadA(char* theURL, char** theDownloadBuffer, int* theDownloadBufferSize, float* theProgressIndicator,std::function<void(void* theArg)> theFunction,void* theArg)
	{
		FetchStruct* aFS=new FetchStruct;
		aFS->mDownloadBuffer=theDownloadBuffer;
		aFS->mDownloadBufferSize=theDownloadBufferSize;
		aFS->mProgressIndicator=theProgressIndicator;
		aFS->mFunction=theFunction;
		aFS->mArg=theArg;
		if (aFS->mProgressIndicator) *aFS->mProgressIndicator=0.0f;
		
		emscripten_fetch_attr_t attr;
		emscripten_fetch_attr_init(&attr);
		strcpy(attr.requestMethod, "GET");
		attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
		attr.onsuccess = _downloadSucceeded;
		if (theProgressIndicator) attr.onprogress=_downloadProgress;
		attr.onerror = _downloadFailed;
		//attr.timeoutMSecs = 2*60;
		attr.userData=aFS;
		emscripten_fetch(&attr,theURL);
		
		//OS_Core::Printf("Launched DOWNLOADA: %s",theURL);
	}
	

	bool Upload(char* theURL, char** theUploadBuffer, int* theUploadBufferSize,float* theProgressIndicator, char** theDownloadBuffer, int* theDownloadBufferSize)
	{
		if (!gIsStarted) return false;
		
		OS_Core::Printf("FAIL!  Can't down Network_Core::Upload in WASM... use UploadA instead!");
		exit(0);
		
		return false;

		
	}
	
	const char *gHeaders[] = {"Content-Type", "application/x-www-form-urlencoded", 0};
	void UploadA(char* theURL, char** theUploadBuffer, int* theUploadBufferSize, float* theProgressIndicator, char** theDownloadBuffer, int* theDownloadBufferSize,std::function<void(void* theArg)> theFunction,void* theArg)
	{
		//OS_Core::Printf("UPLOAD: %s",theURL);
		
		FetchStruct* aFS=new FetchStruct;
		aFS->mDownloadBuffer=theDownloadBuffer;
		aFS->mDownloadBufferSize=theDownloadBufferSize;
		aFS->mProgressIndicator=theProgressIndicator;
		aFS->mFunction=theFunction;
		aFS->mArg=theArg;
		if (aFS->mProgressIndicator) *aFS->mProgressIndicator=0.0f;
		
		emscripten_fetch_attr_t attr;
		emscripten_fetch_attr_init(&attr);
		strcpy(attr.requestMethod, "POST");
		attr.attributes=EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
		attr.onsuccess=_downloadSucceeded;
		
		attr.requestHeaders=gHeaders;
		
		if (theProgressIndicator) attr.onprogress=_downloadProgress;
		attr.onerror=_downloadFailed;
		attr.requestData=*theUploadBuffer;
		attr.requestDataSize=*theUploadBufferSize;
		attr.userData=aFS;
		emscripten_fetch(&attr,theURL);
		
		//OS_Core::Printf("UPLOAD OUT? %s",theURL);
	}


	int GetPendingCount()
	{
		return gProcessCount;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Socket core
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Socket_Core
{
	bool Startup()
	{
		return false;
	}

	void Shutdown()
	{
	}

	bool StartServer(int thePort, int theAllowedClients)
	{
		return false;
	}

	bool IsServerConnected() {return false;}
	char PumpServer(char** theReceivedData, int* theReceivedDataLen)
	{
		return 0;
	}

	void StopServer()
	{
	}

	int GetConnectedClientCount()
	{
		return 0;
	}

	void* GetConnectedClientPeer(int theClient)
	{
		return NULL;
	}

	int GetConnectedClientGUID(int theClient)
	{
		return 0;
	}

	int GetCurrentClientGUID()
	{
		return -1;
	}

	int GetConnectedClientSlot(int theClient)
	{
		return 0;
	}

	int GetCurrentClientSlot()
	{
		return -1;
	}


	bool StartClient(const char* theURL, int thePort)
	{
		return false;
	}

	bool IsClientConnected() {return false;}

	char PumpClient(char** theReceivedData, int* theReceivedDataLen)
	{
		return 0;
	}

	void StopClient()
	{
	}

	void Send(void* theData, int theDataLen, bool doGuaranteed)
	{
	}

	void Reply(void* theData, int theDataLen, bool doGuaranteed)
	{
	}

	void Broadcast(void* theData, int theDataLen, bool doGuaranteed)
	{
	}

	void Send(int toClient, void* theData, int theDataLen, bool doGuaranteed)
	{
	}

	void Send(Array<int>& toClients, void* theData, int theDataLen, bool doGuaranteed)
	{
	}


	void Send(void* thePeer, void* theData, int theDataLen, bool doGuaranteed)
	{
	}

	void Send(Array<void*>& thePeers, void* theData, int theDataLen, bool doGuaranteed)
	{
	}


	unsigned int GetPeerPing(void* thePeer)
	{
		return 0;
	}

	unsigned int GetServerPing()
	{
		return 0;
	}

}
