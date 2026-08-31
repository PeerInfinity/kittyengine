#include "network_core.h"
#include "os_core.h"

#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include "rapt_string.h"

#define USE_LIBCURL
#define USE_SOCKETS


#ifdef USE_LIBCURL
#include <curl/curl.h>
#endif 

#ifdef USE_SOCKETS
#include "ENET/enet.cpp"
#endif

namespace Network_Core
{
	bool				gIsStarted=false;
	bool				gCancel=false;
	int					gProcessCount=0;
	
#ifdef USE_LIBCURL
	char*				gAgent="Mozilla/5.0";
	int					gTimeOut=10*60;
	char				gUploadFormParam[MAX_PATH];
	char				gUploadFormValue[MAX_PATH];

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

	static size_t DownloadMemoryCallback(void *theContents, size_t theSize, size_t theUnitSize, void *theUserPointer)
	{
		if (gCancel) return -1;

		size_t aRealSize=theSize*theUnitSize;
		struct TransferStruct *aMem=(struct TransferStruct *)theUserPointer;
		aMem->mMemory=(char*)realloc(aMem->mMemory,aMem->mSize+aRealSize+1);
		if(aMem->mMemory == NULL)
		{
			OS_Core::Printf("Severe Error: Out of memory while downloading!");
			return 0;
		}
		memcpy(&(aMem->mMemory[aMem->mSize]),theContents,aRealSize);
		aMem->mSize+=aRealSize;
		aMem->mMemory[aMem->mSize]=0;

		return aRealSize;
	}

	static size_t UploadMemoryCallback( void *theContents, size_t theSize, size_t theUnitSize, void* theUserPointer)
	{
		if (gCancel) return -1;

		size_t aTransferSize=theSize*theUnitSize;
		struct TransferStruct *aMem=(struct TransferStruct *)theUserPointer;

		size_t aDataLeftToTransfer=aMem->mSize-aMem->mTransferSize;
		if (aTransferSize>aDataLeftToTransfer) aTransferSize=aDataLeftToTransfer;

		memcpy(theContents,aMem->mMemory+aMem->mTransferSize,aTransferSize);
		aMem->mTransferSize+=aTransferSize;

		if (aMem->mProgress) *aMem->mProgress=(float)aMem->mTransferSize/(float)aMem->mSize;
		return aTransferSize;
	}


	int DownloadProgressCallback(float* theVariable, double downloadTotal, double downloadCurrent, double uploadTotal, double uploadNow)
	{
		if (gCancel) return -1;

		if (downloadTotal>0) *theVariable=(float)(downloadCurrent/downloadTotal);
		return 0;
	}
	
#endif	

	void Startup()
	{
		if (!gIsStarted)
		{
#ifdef USE_LIBCURL
			curl_global_init(CURL_GLOBAL_ALL);
			strcpy(gUploadFormParam,"upload_file");
			strcpy(gUploadFormValue,"file.dat");
#else
#endif			
		}
		gIsStarted=true;
	}

	void Shutdown()
	{
		while (gProcessCount>0) {}	// Wait for any processes to end...
		if (gIsStarted) 
		{
#ifdef USE_LIBCURL
			curl_global_cleanup();
#else
#endif			
			
		}
		gIsStarted=false;
	}

	void Cancel()
	{
		gCancel=true;
	}

	bool Download(char* theURL, char** theDownloadBuffer, int* theDownloadBufferSize, float* theProgressIndicator)
	{
		if (!gIsStarted) return false;
		gCancel=false;
#ifdef USE_LIBCURL
		gProcessCount++;

		CURL *aHandle;
		CURLcode aCurlResult;
		struct TransferStruct aChunk;
		aChunk.mProgress=theProgressIndicator;
		aChunk.mTransferSize=*theDownloadBufferSize;
		bool aResult=true;

		aHandle=curl_easy_init();
		curl_easy_setopt(aHandle,CURLOPT_URL,theURL);
		curl_easy_setopt(aHandle,CURLOPT_WRITEFUNCTION,DownloadMemoryCallback);
		curl_easy_setopt(aHandle,CURLOPT_WRITEDATA,(void *)&aChunk);
		curl_easy_setopt(aHandle,CURLOPT_USERAGENT,gAgent);
		curl_easy_setopt(aHandle, CURLOPT_NOSIGNAL, 1);
		if (theProgressIndicator)
		{
			*theProgressIndicator=0;
			curl_easy_setopt(aHandle,CURLOPT_NOPROGRESS,0L);
			curl_easy_setopt(aHandle,CURLOPT_PROGRESSFUNCTION,DownloadProgressCallback);
			curl_easy_setopt(aHandle,CURLOPT_PROGRESSDATA,theProgressIndicator);
		}

		aCurlResult=curl_easy_perform(aHandle);
		if (aCurlResult!=CURLE_OK || gCancel)
		{
			aResult=false;
			free(aChunk.mMemory);
			aChunk.mSize=0;
            OS_Core::Printf("CURL FAILED TO DOWNLOAD\n");

			*theDownloadBuffer=NULL;
			*theDownloadBufferSize=0;
		}
		else if (!gCancel)
		{
			if (theProgressIndicator) *theProgressIndicator=1.0f;
			if (aChunk.mSize>0)
			{
				if (theDownloadBuffer) *theDownloadBuffer=aChunk.mMemory;
				if (theDownloadBufferSize) *theDownloadBufferSize=aChunk.mSize;
			}
			else
			{
				*theDownloadBuffer=NULL;
				*theDownloadBufferSize=0;
			}
		}

		curl_easy_cleanup(aHandle);
		gProcessCount--;

		return aResult;
#else
		return false;
#endif
	}


	bool Upload(char* theURL, char** theUploadBuffer, int* theUploadBufferSize,float* theProgressIndicator, char** theDownloadBuffer, int* theDownloadBufferSize)
	{
		if (!gIsStarted) return false;
		gCancel=false;
		
#ifdef USE_LIBCURL	

		gProcessCount++;


		CURL *aHandle;
		CURLcode aCurlResult;
		struct TransferStruct aChunk;
		bool aResult=true;

		aHandle=curl_easy_init();


		TransferStruct aMem;
		aMem.mMemory=new char[*theUploadBufferSize];
		aMem.mSize=*theUploadBufferSize;
		aMem.mProgress=theProgressIndicator;
		memcpy(aMem.mMemory,*theUploadBuffer,aMem.mSize);

		struct curl_httppost* aFormPost=NULL;
		struct curl_httppost* aLastPointer=NULL;

		//
		// Make the form data for the actual upload...
		// Note that the param/value here is flexible so it can
		// be tailored to the upload system.  Currently just hardcoded for
		// Rapt.
		//
		curl_formadd(&aFormPost, &aLastPointer,
			CURLFORM_COPYNAME,gUploadFormParam,
			CURLFORM_FILENAME,gUploadFormValue,
			CURLFORM_STREAM,&aMem,
			CURLFORM_CONTENTSLENGTH,aMem.mSize,
			CURLFORM_CONTENTTYPE,"application/octet-stream",
			CURLFORM_END);

		//
		// Parse URL until we hit ?
		// We have to break up all the params to add them to the form, that's the
		// only way to pass params for an httppost upload.
		//
		for (unsigned int aQCount=0;aQCount<strlen(theURL);aQCount++)
		{
			if (theURL[aQCount]=='?')
			{
				char aParamName[MAX_PATH];
				char aParamValue[MAX_PATH*8]; //MIKE this wasn't long enough and caused stack corruption.  Maybe do it dynamic?
				int aParamNameSlot=0;
				int aParamValueSlot=0;
				bool aGettingParam=true;

				for (unsigned int aAndCount=0;aAndCount<=strlen(theURL);aAndCount++)
				{
					if (theURL[aAndCount]=='=') {aGettingParam=false;aParamValueSlot=0;}
					else if (theURL[aAndCount]=='&' || aAndCount==strlen(theURL))
					{
						aGettingParam=true;
						aParamNameSlot=0;
						if(curl_formadd(&aFormPost, &aLastPointer,
							CURLFORM_COPYNAME, aParamName,
							CURLFORM_COPYCONTENTS, aParamValue,
							CURLFORM_CONTENTTYPE, "application/x-www-form-urlencoded",
							CURLFORM_END)!=0)
						{
							aResult=false;
							break;
						}
					}
					else
					{
						if (aGettingParam) {aParamName[aParamNameSlot++]=theURL[aAndCount];aParamName[aParamNameSlot]=0;}
						else {aParamValue[aParamValueSlot++]=theURL[aAndCount];aParamValue[aParamValueSlot]=0;}
					}
				}
				break;
			}
		}

		TransferStruct aResultStruct;
		if(curl_easy_setopt(aHandle,CURLOPT_HTTPPOST, aFormPost)!=CURLE_OK) aResult=false;
		if(curl_easy_setopt(aHandle,CURLOPT_URL, theURL)!=CURLE_OK) aResult=false;
		if(curl_easy_setopt(aHandle,CURLOPT_WRITEFUNCTION, DownloadMemoryCallback)!=CURLE_OK) aResult=false;
		if(curl_easy_setopt(aHandle,CURLOPT_CONNECTTIMEOUT,gTimeOut)!=CURLE_OK) aResult=false;
		if(curl_easy_setopt(aHandle,CURLOPT_TIMEOUT,gTimeOut)!=CURLE_OK) aResult=false;
		if(curl_easy_setopt(aHandle,CURLOPT_WRITEDATA,&aResultStruct)!=CURLE_OK) aResult=false;
		if(curl_easy_setopt(aHandle, CURLOPT_READFUNCTION, UploadMemoryCallback)!=CURLE_OK) aResult=false;
		if(curl_easy_setopt(aHandle, CURLOPT_READDATA, (void*)&aMem)!=CURLE_OK) aResult=false;

		if (aResult)
		{
			if (theProgressIndicator) *theProgressIndicator=0;

			aCurlResult=curl_easy_perform(aHandle);
			if (aCurlResult!=CURLE_OK || gCancel) aResult=false;
		}

		if (!aResult || gCancel)
		{
			aResult=false;
			free(aChunk.mMemory);
			aChunk.mSize=0;
		}

		if (!gCancel)
		{
			if (theProgressIndicator) *theProgressIndicator=1.0f;
			if (theDownloadBuffer) *theDownloadBuffer=aResultStruct.mMemory;
			if (theDownloadBufferSize) *theDownloadBufferSize=aResultStruct.mSize;
		}

		curl_easy_cleanup(aHandle);

		delete [] aMem.mMemory;
		gProcessCount--;

		return aResult;
#else
		return false;
#endif
	}
	
	struct TransferData
	{
		std::string							mURL;
		char**								mUploadBuffer;
		int*								mUploadBufferSize;
		char**								mDownloadBuffer;
		int*								mDownloadBufferSize;
		float*								mProgressIndicator;
		std::function<void(void* theArg)>	mFunction;
		void*								mArg;
	};

	void DownloadAStub(void* theArg)
	{
		TransferData* aData=(TransferData*)theArg;
		Download((char*)aData->mURL.c_str(),aData->mDownloadBuffer,aData->mDownloadBufferSize,aData->mProgressIndicator);
		if (aData->mFunction) aData->mFunction(aData->mArg);
		delete aData;
	}

	void DownloadA(char* theURL, char** theDownloadBuffer, int* theDownloadBufferSize, float* theProgressIndicator,std::function<void(void* theArg)> theFunction,void* theArg)
	{
		TransferData* aData=new TransferData;
		aData->mURL=theURL;
		aData->mDownloadBuffer=theDownloadBuffer;
		aData->mDownloadBufferSize=theDownloadBufferSize;
		aData->mProgressIndicator=theProgressIndicator;
		aData->mFunction=theFunction;
		aData->mArg=theArg;

		OS_Core::Thread(&DownloadAStub,aData);
	}

	void UploadAStub(void* theArg)
	{
		TransferData* aData=(TransferData*)theArg;
		Upload((char*)aData->mURL.c_str(),aData->mUploadBuffer,aData->mUploadBufferSize,aData->mProgressIndicator,aData->mDownloadBuffer,aData->mDownloadBufferSize);
		if (aData->mFunction) aData->mFunction(aData->mArg);
		delete aData;
	}


	bool UploadA(char* theURL, char** theUploadBuffer, int* theUploadBufferSize, float* theProgressIndicator, char** theDownloadBuffer, int* theDownloadBufferSize,std::function<void(void* theArg)> theFunction,void* theArg)
	{
		TransferData* aData=new TransferData;
		aData->mURL=theURL;
		aData->mUploadBuffer=theUploadBuffer;
		aData->mUploadBufferSize=theUploadBufferSize;
		aData->mDownloadBuffer=theDownloadBuffer;
		aData->mDownloadBufferSize=theDownloadBufferSize;
		aData->mProgressIndicator=theProgressIndicator;
		aData->mFunction=theFunction;
		aData->mArg=theArg;

		OS_Core::Thread(&UploadAStub,aData);
		return true;
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

#ifdef USE_SOCKETS
namespace Socket_Core
{
	int gSocketThreadLock=0;
	char gLocalIP[32]={0};//,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	
	char* GetLocalIP()
	{
		if (gLocalIP[0]==0)
		{
			char aBuffer[1024];
			if(gethostname(aBuffer, sizeof(aBuffer)) == SOCKET_ERROR) return (char*)"(Failed to get IP)";
	
			struct hostent *host = gethostbyname(aBuffer);
			if(host == NULL) return (char*)"(Failed to get IP)";
			struct in_addr* aAddress=(in_addr*)host->h_addr;
			strcpy(gLocalIP,inet_ntoa(*aAddress) );

		}
		return gLocalIP;
	}

	ENetHost* gServer=NULL;
	ENetHost* gClient=NULL;
	ENetPeer* gClientPeer=NULL;
	ENetPeer* gReplyPeer=NULL;	// Peer that just came in (so we can reply to it)
	ENetEvent gServerEvent;	// Global so we can clean it up...
	ENetEvent gClientEvent;	// Global so we can clean it up...
	bool gIsStarted=false;

	static char gError[256];
	enum CHANNELS
	{
		CHANNEL_RELIABLE=0,
		CHANNEL_FAST,
		MAX_CHANNELS
	};

	char* GetError() {return gError;}
	int gPeerGUID=1;
	Array<ENetPeer*> gPeerList;
	ENetPeer* gCurrentPeer=NULL;

	struct PeerData
	{
		int			mGUID;	// Global ID -- always unique for each connection
		int			mSlot;  // Slot -- which slot we're shoved into.
	};

	bool Startup()
	{
		if (enet_initialize () != 0) {strcpy(gError,"Could not initialize Enet");return false;}
		gIsStarted=true;

		memset(&gClientEvent,0,sizeof(gClientEvent));
		memset(&gServerEvent,0,sizeof(gServerEvent));

		gSocketThreadLock=OS_Core::CreateThreadLock();
		return true;
	}

	void Shutdown()
	{
		StopClient();
		StopServer();
		enet_deinitialize();
		OS_Core::ReleaseThreadLock(gSocketThreadLock);
		gIsStarted=false;

		memset(&gClientEvent,0,sizeof(gClientEvent));
		memset(&gServerEvent,0,sizeof(gServerEvent));
	}

	int gAllowedClients=0;
	bool StartServer(int thePort, int theAllowedClients)
	{
		if (!gIsStarted) Startup();
		if (gServer) StopServer();

		ENetHost* aServer=NULL;
		ENetAddress aAddress;
		aAddress.host=ENET_HOST_ANY;
		aAddress.port=thePort;

		gAllowedClients=theAllowedClients;
		gPeerList.GuaranteeSize(gAllowedClients);
		for (int aCount=0;aCount<gPeerList.Size();aCount++) gPeerList[aCount]=NULL;

		aServer=enet_host_create(&aAddress,theAllowedClients,2,0,0);
		if (!aServer) {strcpy(gError,"An error occurred while trying to create a server!");return false;}
		gServer=aServer;

		return true;
	}

	bool IsServerConnected() {return (gServer!=NULL);}
	char PumpServer(char** theReceivedData, int* theReceivedDataLen)
	{
		if (!gServer) return 0;
		char aResult=0;

		OS_Core::ThreadLock(gSocketThreadLock);
		enet_packet_destroy (gServerEvent.packet);
		if (enet_host_service(gServer,&gServerEvent,0)>0)
		{
			gReplyPeer=gServerEvent.peer;
			gCurrentPeer=gServerEvent.peer;
			switch (gServerEvent.type)
			{
			case ENET_EVENT_TYPE_NONE:break;
			case ENET_EVENT_TYPE_CONNECT:
				{
					*theReceivedData=NULL;
					*theReceivedDataLen=0;

					aResult=2;

					PeerData* aPD=new PeerData;
					aPD->mGUID=gPeerGUID++;
					aPD->mSlot=-1;

					for (int aPeerSlot=0;aPeerSlot<gAllowedClients;aPeerSlot++)
					{
						bool aOK=true;
						for (int aCount=0;aCount<gPeerList.Size();aCount++)
						{
							ENetPeer* aPeer=gPeerList[aCount];
							if (aPeer) 
							{
								PeerData* aPDTest=(PeerData*)aPeer->data;
								if (aPDTest->mSlot==aPeerSlot) {aOK=false;break;}
							}
						}
						if (aOK)
						{
							aPD->mSlot=aPeerSlot;
							break;
						}
					}
					if (aPD->mSlot<0) 
					{
						OS_Core::Printf("!");
						OS_Core::Printf("!");
						OS_Core::Printf("SEVERE ERROR: Could not get a Peer Slot!");
						OS_Core::Printf("!");
						OS_Core::Printf("!");
					}
					else
					{
#ifdef _DEBUG
						OS_Core::Printf("Connect Client... [%d]",aPD->mSlot);
#endif				
					}


					gServerEvent.peer->data=aPD;
					gPeerList[aPD->mSlot]=gServerEvent.peer;
				}
				break;
			case ENET_EVENT_TYPE_RECEIVE:
				aResult=1;
				if (theReceivedData && theReceivedDataLen)
				{
					*theReceivedData=(char*)gServerEvent.packet->data;
					*theReceivedDataLen=(int)gServerEvent.packet->dataLength;
				}
				break;
			case ENET_EVENT_TYPE_DISCONNECT:
				*theReceivedData=NULL;
				*theReceivedDataLen=0;

				if (gReplyPeer==gServerEvent.peer) gReplyPeer=NULL;

				for (int aCount=0;aCount<gPeerList.Size();aCount++)
				{
					int aLocation=gPeerList.FindValue(gServerEvent.peer);
					if (aLocation>=0) 
					{
						gPeerList[aLocation]=NULL;//.DeleteElement(aLocation);

#ifdef _DEBUG
						OS_Core::Printf("Disconnect Client... [%d]",aLocation);
#endif

					}
				}

				struct PeerData* aPD=(PeerData*)gServerEvent.peer->data;
				delete aPD;
				gServerEvent.peer->data=NULL;

				aResult=2;
			}
		}
		OS_Core::ThreadUnlock(gSocketThreadLock);
		return aResult;
	}

	void StopServer()
	{
		OS_Core::ThreadLock(gSocketThreadLock);
		enet_packet_destroy(gServerEvent.packet);

		if (gServer)
		{
			PumpServer(NULL,NULL);
			for (int aCount=0;aCount<gPeerList.Size();aCount++)
			{
				if (gPeerList[aCount]) enet_peer_disconnect(gPeerList[aCount],0);
				gPeerList[aCount]=NULL;
				PumpServer(NULL,NULL);
			}
			gPeerList.Reset();
			enet_host_destroy(gServer);
		}
		OS_Core::ThreadUnlock(gSocketThreadLock);
		gServer=NULL;
	}

	int GetConnectedClientCount()
	{
		return gPeerList.Size();
	}

	void* GetConnectedClientPeer(int theClient)
	{
		if (theClient<0 || theClient>=gPeerList.Size()) return NULL;
		return (void*)gPeerList[theClient];
	}

	int GetConnectedClientGUID(int theClient)
	{
		if (theClient<0 || theClient>=gPeerList.Size()) return 0;
		if (!gPeerList[theClient]) return 0;

		PeerData* aPD=(PeerData*)gPeerList[theClient]->data;
		return aPD->mGUID;
	}

	int GetCurrentClientGUID()
	{
		if (!gCurrentPeer) return -1;

		PeerData* aPD=(PeerData*)gCurrentPeer->data;
		return aPD->mGUID;
	}

	int GetConnectedClientSlot(int theClient)
	{
		if (theClient<0 || theClient>=gPeerList.Size()) return 0;
		if (!gPeerList[theClient]) return 0;

		PeerData* aPD=(PeerData*)gPeerList[theClient]->data;
		return aPD->mSlot;
	}

	int GetCurrentClientSlot()
	{
		if (!gCurrentPeer) return -1;

		PeerData* aPD=(PeerData*)gCurrentPeer->data;
		return aPD->mSlot;
	}


	bool StartClient(const char* theURL, int thePort)
	{
		if (!gIsStarted) Startup();
		if (gClient) StopClient();
		ENetHost* aClient=NULL;

		aClient=enet_host_create(NULL,1,2,0,0);
		if (!aClient) {strcpy(gError,"An error occurred while trying to create a client!");return false;}

		ENetAddress aAddress;
		ENetEvent aEvent;
		ENetPeer *aPeer;

		enet_address_set_host(&aAddress,theURL);
		aAddress.port=thePort;
		aPeer=enet_host_connect(aClient,&aAddress,2,0);
		if (!aPeer) {strcpy(gError,"An error occurred while trying to create a connection peer!");return false;}

		if (enet_host_service(aClient,&aEvent,5000)>0 && aEvent.type==ENET_EVENT_TYPE_CONNECT)
		{
#ifdef _DEBUG
			OS_Core::Printf("Connection success!");
#endif
		}
		else
		{
			enet_peer_reset(aPeer);
			sprintf(gError,"Connection to %s:%d failed!",theURL,thePort);
			return false;
		}
		gClient=aClient;
		gClientPeer=aPeer;

		return true;
	}

	bool IsClientConnected() {return (gClient!=NULL);}

	char PumpClient(char** theReceivedData, int* theReceivedDataLen)
	{
		if (!gClient) return 0;
		char aResult=0;

		OS_Core::ThreadLock(gSocketThreadLock);
		enet_packet_destroy(gClientEvent.packet);
		if (enet_host_service(gClient,&gClientEvent,0)>0)
		{
			switch (gClientEvent.type)
			{
			case ENET_EVENT_TYPE_NONE:break;
			case ENET_EVENT_TYPE_CONNECT:
				*theReceivedData=NULL;
				*theReceivedDataLen=0;
				aResult=2;
				break;
			case ENET_EVENT_TYPE_RECEIVE:
				aResult=1;
				if (theReceivedData && theReceivedDataLen)
				{
					*theReceivedData=(char*)gClientEvent.packet->data;
					*theReceivedDataLen=(int)gClientEvent.packet->dataLength;
				}
				break;
			case ENET_EVENT_TYPE_DISCONNECT:
#ifdef _DEBUG
				OS_Core::Printf("Disconnect Event?");
#endif
				*theReceivedData=NULL;
				*theReceivedDataLen=0;
				aResult=-1;
				break;
			}
		}
		OS_Core::ThreadUnlock(gSocketThreadLock);
		return aResult;
	}

	void StopClient()
	{
		OS_Core::ThreadLock(gSocketThreadLock);
		enet_packet_destroy(gClientEvent.packet);
		if (gClient)
		{
			//if (doFast) enet_peer_disconnect_now(aConnection.mPeer,0);
			if (gClientPeer) enet_peer_disconnect(gClientPeer,0);

			//
			// Final pump cleanup...
			// We make the client disconnect elegantly, if possible...
			//
			ENetEvent aEvent;
			bool aGotDisconnect=false;
			while (enet_host_service(gClient,&aEvent,100)>0)
			{
				if (aEvent.type==ENET_EVENT_TYPE_RECEIVE) enet_packet_destroy(aEvent.packet);
				if (aEvent.type==ENET_EVENT_TYPE_DISCONNECT) {aGotDisconnect=true;break;}
			}
			if (!aGotDisconnect) enet_peer_reset(gClientPeer); // Forceful disconnect

			//
			// Destroy the client...
			//
			if (gClient) enet_host_destroy(gClient);
		}
		OS_Core::ThreadUnlock(gSocketThreadLock);
		gClient=NULL;
	}

	void Send(void* theData, int theDataLen, bool doGuaranteed)
	{
		if (!gClient || !gClientPeer)
		{
			if (gServer) Broadcast(theData,theDataLen,doGuaranteed);
			return;
		}
		OS_Core::ThreadLock(gSocketThreadLock);
		ENetPacket* aPacket = enet_packet_create (theData,theDataLen,doGuaranteed?ENET_PACKET_FLAG_RELIABLE:0);

		int aResult=0;
		if (doGuaranteed) aResult=enet_peer_send(gClientPeer,CHANNEL_RELIABLE,aPacket)<0;	
		else aResult=enet_peer_send(gClientPeer,CHANNEL_FAST,aPacket);	
		if (aResult<0) OS_Core::Printf("Send(1) Failed!");
		OS_Core::ThreadUnlock(gSocketThreadLock);

		// enet_host_broadcast (host, 0, packet);         
	}

	void Reply(void* theData, int theDataLen, bool doGuaranteed)
	{
		if (!gReplyPeer) return;

		OS_Core::ThreadLock(gSocketThreadLock);
		ENetPacket* aPacket = enet_packet_create (theData,theDataLen,doGuaranteed?ENET_PACKET_FLAG_RELIABLE:0);

		int aResult=0;
		if (doGuaranteed) aResult=enet_peer_send(gReplyPeer,CHANNEL_RELIABLE,aPacket);	
		else aResult=enet_peer_send(gReplyPeer,CHANNEL_FAST,aPacket);	
		if (aResult<0) OS_Core::Printf("Send(2) Failed!");
		OS_Core::ThreadUnlock(gSocketThreadLock);
	}

	void Broadcast(void* theData, int theDataLen, bool doGuaranteed)
	{
		if (!gServer) return;

		OS_Core::ThreadLock(gSocketThreadLock);
		ENetPacket* aPacket = enet_packet_create (theData,theDataLen,doGuaranteed?ENET_PACKET_FLAG_RELIABLE:0);
		if (doGuaranteed) enet_host_broadcast(gServer,CHANNEL_RELIABLE,aPacket);	
		else enet_host_broadcast(gServer,CHANNEL_FAST,aPacket);	
		OS_Core::ThreadUnlock(gSocketThreadLock);
	}

	void Send(int toClient, void* theData, int theDataLen, bool doGuaranteed)
	{
		if (!gServer) return;
		if (toClient<0 || toClient>=gPeerList.Size()) return;
		if (!gPeerList[toClient]) return;

		OS_Core::ThreadLock(gSocketThreadLock);
		ENetPacket* aPacket = enet_packet_create (theData,theDataLen,doGuaranteed?ENET_PACKET_FLAG_RELIABLE:0);
		int aResult=0;
		if (doGuaranteed) aResult=enet_peer_send(gPeerList[toClient],CHANNEL_RELIABLE,aPacket);	
		else aResult=enet_peer_send(gPeerList[toClient],CHANNEL_FAST,aPacket);	
		if (aResult<0) OS_Core::Printf("Send(4) Failed!");
		OS_Core::ThreadUnlock(gSocketThreadLock);
	}

	void Send(Array<int>& toClients, void* theData, int theDataLen, bool doGuaranteed)
	{
		if (!gServer) return;
		if (toClients.Size()==0) return;

		OS_Core::ThreadLock(gSocketThreadLock);
		ENetPacket* aPacket = enet_packet_create (theData,theDataLen,doGuaranteed?ENET_PACKET_FLAG_RELIABLE:0);

		int aResult=0;
		int aSendHow=CHANNEL_FAST;
		if (doGuaranteed)  aSendHow=CHANNEL_RELIABLE;

		for (int aCount=0;aCount<toClients.Size();aCount++)
		{
			int aToClient=toClients[aCount];
			if (aToClient<0 || aToClient>=gPeerList.Size()) continue;
			if (!gPeerList[aToClient]) continue;
			aResult=enet_peer_send(gPeerList[aToClient],aSendHow,aPacket);	
		}
		if (aResult<0) OS_Core::Printf("Send(6) Failed!");
		OS_Core::ThreadUnlock(gSocketThreadLock);
	}


	void Send(void* thePeer, void* theData, int theDataLen, bool doGuaranteed)
	{
		if (!gServer) return;
		if (!thePeer) return;

		ENetPeer* aPeer=(ENetPeer*)thePeer;
		//if (gPeerList.FindValue(aPeer)>=0)
		{
			OS_Core::ThreadLock(gSocketThreadLock);
			ENetPacket* aPacket = enet_packet_create (theData,theDataLen,doGuaranteed?ENET_PACKET_FLAG_RELIABLE:0);
			int aResult=0;
			if (doGuaranteed) aResult=enet_peer_send(aPeer,CHANNEL_RELIABLE,aPacket);	
			else aResult=enet_peer_send(aPeer,CHANNEL_FAST,aPacket);	
			if (aResult<0) OS_Core::Printf("Send(5) Failed!");
			OS_Core::ThreadUnlock(gSocketThreadLock);
		}
	}

	void Send(Array<void*>& thePeers, void* theData, int theDataLen, bool doGuaranteed)
	{
		if (!gServer) return;
		if (thePeers.Size()==0) return;
		//if (!thePeer) return;

		//ENetPeer* aPeer=(ENetPeer*)thePeer;
		//if (gPeerList.FindValue(aPeer)>=0)
		{
			OS_Core::ThreadLock(gSocketThreadLock);
			ENetPacket* aPacket = enet_packet_create (theData,theDataLen,doGuaranteed?ENET_PACKET_FLAG_RELIABLE:0);

			int aResult=0;
			int aSendHow=CHANNEL_FAST;
			if (doGuaranteed)  aSendHow=CHANNEL_RELIABLE;
			for (int aCount=0;aCount<thePeers.Size();aCount++)
			{
				ENetPeer* aPeer=(ENetPeer*)thePeers[aCount];
				if (aPeer) aResult=enet_peer_send(aPeer,aSendHow,aPacket);	
			}
			if (aResult<0) OS_Core::Printf("Send(7) Failed!");
			OS_Core::ThreadUnlock(gSocketThreadLock);
		}
	}


	unsigned int GetPeerPing(void* thePeer)
	{
		if (!gServer) return 0;
		if (!thePeer) return 0;

		ENetPeer* aPeer=(ENetPeer*)thePeer;
		return aPeer->roundTripTime;
	}

	unsigned int GetServerPing()
	{
		if (gClientPeer) return gClientPeer->lastRoundTripTime;
		return 0;
	}
}
#endif

