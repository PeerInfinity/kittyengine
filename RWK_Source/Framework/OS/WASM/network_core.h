#pragma once
#include "os_headers.h"
#include "util_core.h"

namespace Network_Core
{
	void Startup();
	void Shutdown();

	//
	// File transfers.  Note that these BLOCK, so it's best to use them in a thread.
	// Note that you don't allocate the download buffer, but you DO need to free it when you're done with it.
	// theDownloadBuffer in Upload is where the server responds to you.
	//
	bool				Download(char* theURL, char** theDownloadBuffer, int* theDownloadBufferSize, float* theProgressIndicator);
	void				DownloadA(char* theURL, char** theDownloadBuffer, int* theDownloadBufferSize, float* theProgressIndicator,std::function<void(void* theArg)> theFunction,void* theArg);
	bool				Upload(char* theURL, char** theUploadBuffer, int* theUploadBufferSize, float* theProgressIndicator, char** theDownloadBuffer, int* theDownloadBufferSize);
	void				UploadA(char* theURL, char** theUploadBuffer, int* theUploadBufferSize, float* theProgressIndicator, char** theDownloadBuffer, int* theDownloadBufferSize,std::function<void(void* theArg)> theFunction,void* theArg);

	//
	// Allows you to set the param/value containing the upload data chunk, so that it can be
	// tailored to any system.  
	//
	void				SetUploadForms(char* theParam, char* theValue);

	//
	// Cancels all current transfers and queries
	//
	void				Cancel();

	//
	// Tells how many processes are still out
	//
	int					GetPendingCount();
}

namespace Socket_Core
{
/*
	bool			Startup();
	void			Shutdown();

	//
	// Server Functions
	//
	int				Host(int thePort, int theClients=4);

	//
	// Client Functions
	//
	int				Connect(const char* url, int port);
	void			CloseConnection(int theConnection, bool doFast=false);
	bool			Send(int theConnection, void* theData, int theDataLen);
	bool			SendGuaranteed(int theConnection, void* theData, int theDataLen);
	bool			Pump(int theConnection, void** theReceivedData, int* theReceivedDataLen);
	bool			IsConnected(int theConnection);	// Connection doesn't happen immediately, so we have to wait here...

	// returns last error as a string value
	char*			GetLastError();
*/

	bool			Startup();
	void			Shutdown();
	char*			GetError();

	//
	// Pump results:
	// -1 = The connection is no longer valid (client only)
	// 0 = no processing needed
	// 1 = theReceivedData/theReceivedDataLen contain a message
	// 2 = The number of connections changed
	//

	//
	// Server calls
	//
	bool			StartServer(int thePort, int theAllowedClients=4);
	char			PumpServer(char** theReceivedData, int* theReceivedDataLen);
	void			StopServer();
	bool			IsServerConnected();
	//
	// Special note:
	// When clients connect/disconnect, they can be re-arranged.
	// But, GUID and Slot will always be the same for the same client.
	// However, you must use the client number in order to send,
	// so you will likely want to store all three of these values
	// on the client.
	//
	int				GetConnectedClientCount();
	void*			GetConnectedClientPeer(int theClient);
	int				GetConnectedClientGUID(int theClient);
	int				GetConnectedClientSlot(int theClient);
	int				GetCurrentClientGUID(); // Gets the GUID from most recent client who sent message
	int				GetCurrentClientSlot(); // Gets the GUID from most recent client who sent message

	void			Reply(void* theData, int theDataLen, bool doGuaranteed=true);
	void			Broadcast(void* theData, int theDataLen, bool doGuaranteed=true);
	void			Send(int toClient, void* theData, int theDataLen, bool doGuaranteed=true);
	void			Send(void* thePeer, void* theData, int theDataLen, bool doGuaranteed=true);
	void			Send(Array<int>& toClients, void* theData, int theDataLen, bool doGuaranteed=true);
	void			Send(Array<void*>& thePeer, void* theData, int theDataLen, bool doGuaranteed=true);

	//
	// Client calls
	//
	bool			StartClient(const char* theURL, int thePort);
	char			PumpClient(char** theReceivedData, int* theReceivedDataLen);
	void			StopClient();
	bool			IsClientConnected();
	void			Send(void* theData, int theDataLen, bool doGuaranteed=true);

	//
	// Gets some extended info from peers, if we need it...
	//
	unsigned int	GetPeerPing(void* thePeer);
	unsigned int	GetServerPing();

	//
	// Gets the local IP address of this computer...
	//
	char*			GetLocalIP();
};