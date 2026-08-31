#pragma once
#include "rapt.h"

#define RMLINDICATOR "->TEXTRESPONSE<-"
#define RMLINDICATORLEN 16

#define EnumRQueryList(variable,list) EnumSmartList(RComm::RaptisoftQuery,variable,list)
#define RCOMMHOOK(func) [=](RComm::RQuery theQuery) func
#define RCOMMHOOKPTR std::function<void(RComm::RQuery theQuery)>
#define RCOMMVERIFYHOOK(func) [=](RComm::RQuery theQuery)->bool func
#define RCOMMVERIFYHOOKPTR std::function<bool(RComm::RQuery theQuery)>

namespace RComm
{

class RaptisoftQuery;
class QueryBuffer : public IOBuffer {public: Smart(RaptisoftQuery) mQueryPtr=NULL;};

typedef Smart(RaptisoftQuery) RQuery;

class RaptisoftQuery
{
public:
	RaptisoftQuery()
	{
		mExtraData=NULL;
		mFunctionPointer=NULL;
		mProgress=0;
		mNextQuery=NULL;
		mNextQueryCallback=NULL;
		mDone=false;
	}

	inline IOBuffer&		GetResult() {return mResult;}
	inline bool				IsDone() {return mDone;}
	bool					IsRML();
	float					Progress();

public:
	String					mName;		// Name of this query.  For user to identify with.
	String					mQuery;		// URL we're sending to
	QueryBuffer				mUpload;	// Any possible upload
	QueryBuffer				mResult;	// Result that comes down
	bool					mDone=false;		// If we're all done

	void*					mExtraData=NULL;
	union
	{
		void*					mFunctionPointer=NULL;	// Points to the function that called it... 
		void*					mCaller;				// Why?  if you throw all your queries into a list, you can scan this to
														// process them to see what a result should do.  It's here so the automated functions
														// can provide it.  It's sort of like an ID, without reserving numbers!
	};
	float					mProgress;

	RCOMMHOOKPTR			mCore_Hook=NULL;		// Core callback, controlled by calling function... don't modify this except in emergencies!
	RCOMMHOOKPTR			mCallback=NULL;			// User specified callback
	RCOMMVERIFYHOOKPTR		mVerifyCallback=NULL;	// This function will be called to verify if the query succeeded or failed.  For example, when uploading, the query will need to check
													// the results for the proper code, and can fail it.

	//
	// If we're calling a local function (i.e. we sent it a threaded function that we want to treat exactly like any web callback, by
	// watching the query)
	//
	// Depreciated in favor of mCallback...
	// Left in because RWK's Makermall uses it to build local levels (smart!)
	//
	RCOMMHOOKPTR			mLocalFunction=NULL;

	//
	// You can stack queries and do stuff with them and make them act like "one query."
	// This was put in to handle the file uploads-- because we have to send the level AND the preview, we want them to go together.
	// It'll invoke the NextQuery automatically, and call the NextQueryFunction beforehand (for file uploads, what we are doing is
	// making it so that we change the preview query to have the ID number)
	//
	Smart(RaptisoftQuery)	mNextQuery;
	bool (*mNextQueryCallback)(RaptisoftQuery* thePreviousQuery, RaptisoftQuery* theNextQuery); // Maybe depreciated...

};
typedef SmartList(RaptisoftQuery) RQueryList;

class RaptisoftExecute
{
public:
	String					mEXE;
};
typedef Smart(RaptisoftExecute) RExecute;



void RCThreadLock();
void RCThreadUnlock();
RExecute GetNextExecute();
void PushExecute(String theString);
RQuery SendSecretCode(String theCode, void* theExtraData=NULL);
RQuery GetGUID(String extraParams="", void* theExtraData=NULL);
RQuery Log(String theLogString, void* theExtraData=NULL);
RQuery Custom(RCOMMHOOKPTR theHook, void* theExtraData=NULL);
RQuery SendData(IOBuffer& theBuffer, String theDescription, String theFileExtension="", void* theExtraData=NULL);
RQuery ReportDeviceID(String extraParams="", void* theExtraData=NULL); // Send the DEVICE id (i.e. the Mac address or unique identifier) to the server for logging.
void RQuery_Log(char* theFormat, ...);
RQuery LevelSharing_GetTags(void* theExtraData=NULL);
RQuery LevelSharing_Login(String theName, String thePassword, void* theExtraData=NULL);
RQuery LevelSharing_Hello(String theName, String thePassword, String theVersion, void* theExtraData=NULL);
RQuery LevelSharing_GetLevels(int theStartCursor, int theCount, longlong theFilter=0, void* theExtraData=NULL);
RQuery LevelSharing_GetNewLevels(String theSince, int theStartCursor, int theCount, longlong theFilter=0, void* theExtraData=NULL);
RQuery LevelSharing_GetHotLevels(int theStartCursor, int theCount, longlong theFilter=0, void* theExtraData=NULL);
RQuery LevelSharing_GetStarLevels(int theStartCursor, int theCount, longlong theFilter=0, void* theExtraData=NULL);
RQuery LevelSharing_GetRandomLevels(int theStartCursor, int theCount, longlong theFilter=0, void* theExtraData=NULL);
RQuery LevelSharing_SearchLevels(String theNameFilter, String theAuthorFilter, bool newestFirst, int theStartCursor, int theCount, longlong theFilter=0, void* theExtraData=NULL);
RQuery LevelSharing_DownloadLevel(int theLevelID, void* theExtraData=NULL);
RQuery LevelSharing_DownloadLevelInfo(int theLevelID, void* theExtraData=NULL);	// Gets name and author... this was put in for playing instant-levels...
RQuery LevelSharing_DownloadLevelPreview(int theLevelID, void* theExtraData=NULL);
RQuery LevelSharing_UploadLevel(String theUsername, String thePassword, String theLevelName, longlong theTags, IOBuffer& theLevelFile, IOBuffer& thePreviewFile, String theClientVersion, void* theExtraData=NULL);
RQuery LevelSharing_UploadLevelNoPreview(String theUsername, String thePassword, String theLevelName, longlong theTags, IOBuffer& theLevelFile, String theClientVersion, void* theExtraData=NULL);
RQuery LevelSharing_GetComments(int theLevelID, void* theExtraData=NULL);
RQuery LevelSharing_PlayedLevel(int theLevelID, void* theExtraData=NULL);
RQuery LevelSharing_WonLevel(int theLevelID, void* theExtraData=NULL);
RQuery LevelSharing_GetLocalStats(String theUsername, String thePassword, Array<int>& theLevelIDList, void* theExtraData=NULL); // Somewhat Depreciated... GetLevelStats does the same thing, doesn't require password.
RQuery LevelSharing_GetLevelStats(Array<int>& theLevelIDList, void* theExtraData=NULL);
RQuery LevelSharing_Vote(int theLevelID, int theVote, void* theExtraData=NULL);
RQuery LevelSharing_GiveStar(String theUsername, String thePassword, int theLevelID, void* theExtraData=NULL);
RQuery LevelSharing_ReportAbuse(int theLevelID, String theReason, void* theExtraData=NULL);
RQuery LevelSharing_Comment(String theUsername, String thePassword, int theLevelID, String theComment, void* theExtraData=NULL);
RQuery LevelSharing_DeleteLevel(String theUsername, String thePassword, int theLevelID, void* theExtraData=NULL);
RQuery LevelSharing_Recover(String theName, void* theExtraData=NULL);
RQuery LevelSharing_SendActivation(String theName, void* theExtraData=NULL);
RQuery LevelSharing_CreateAccount(String theName, String thePassword, String theEmail, void* theExtraData=NULL);
RQuery LevelSharing_Serve(String theUsername, String thePassword, String theRequest, String extraParams="", void* theExtraData=NULL);
RQuery LevelSharing_Boost(String theUsername, String thePassword, int theLevelID, char theBoostDir, String extraParams="", void* theExtraData=NULL);

//
// Housekeeping...
//
void SetGameID(int theGameID, String theVersionID);	// Sets gameID for repeat ops...
void SetURL(String theURL);							// Sets the URL we'll be connecting to
String GetURL();									// Just tells us what the URL is
String GetURLPrefix(String theAppend="://");		// Just tells us what the URL Prefix is
void SetExtraParams(String theParams);				// Extra params that will go with every URL sent...
void SetIgnoreFilter(longlong theIgnore);			// Sets global filter bits to ignore for level retrieval (put in so that "Glitchy" filter can be ignored normally)
void SetRCommPage(String thePage="");
//
// Connecting...
//
RQuery Ping(RCOMMHOOKPTR theCallback=NULL);
RQuery Connect(String theURL="https://www.yourserver.com/");// Quick connect... usually runs this
bool IsConnecting();
bool IsConnected();
unsigned int GetGameGUID();
//
// News
//
bool IsNews();
String GetNews();
void NewsAlert(String theAlertString);
int GetFlagIcon(String theCode); // Returns a flag icon based on a country code
void SetNewsRead(); 
void FreshenNews(); 
inline void MarkNewsRead() {SetNewsRead();}
inline void RefreshNews() {FreshenNews();}
RQuery ClearCommand(String theName, String thePassword, void* theExtraData=NULL);

class Pending : public CPU
{
public:
	Pending(RQuery theQuery, RCOMMHOOKPTR onCompleteHook, CPUHOOKPTR drawHook=NULL, CPUHOOKPTR initializeHook=NULL);
	void			Core_Draw() {if (mAge>4) CPU::Core_Draw();}
	void			Update();

	RQuery			mQuery;
	RCOMMHOOKPTR	mOnComplete=NULL;
};

// Simple fire of an RComm query with an oncompletehook.
void FireAndForget(RQuery theQuery, RCOMMHOOKPTR onCompleteHook);
};

typedef Smart(RComm::RaptisoftQuery) RQuery;

