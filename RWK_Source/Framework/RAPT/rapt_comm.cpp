#include "rapt_comm.h"

//#define _RQUERYLOGGING

namespace RComm
{
///////////////////////////////////////////////////////////////////////////
//
// For connecting...
//
///////////////////////////////////////////////////////////////////////////
bool gMM_IsConnecting=false;
bool gMM_IsConnected=false;
bool gMM_News=false;
unsigned int gMM_GameGUID=0;
String gMM_StubURL;

//
// A way to send queries into the thread...
//
struct ThreadedRQuery {ThreadedRQuery(RQuery theQuery) {mQuery=theQuery;}RQuery mQuery;};

void RCCompatibleSend(RQuery theQuery, bool sendEvenIfNotConnected=false);

///////////////////////////////////////////////////////////////////////////
//
// Generalized Queries, for use by everything.
//
///////////////////////////////////////////////////////////////////////////
int gRCThreadLock=-1;
int gEXEThreadLock=-1;
SmartList(RaptisoftExecute) gPendingExecuteList;
RQueryList gPendingRQueryList;	// Queries that haven't gone out yet because we're not connected

float RaptisoftQuery::Progress()
{
	if (mNextQuery.IsNotNull())
	{
		float aProgress=mProgress;
		float aQCount=1;

		RQuery aQ=mNextQuery;

		while (aQ.IsNotNull())
		{
			aQCount++;
			aProgress+=aQ->mProgress;
			aQ=aQ->mNextQuery;
		}
		return aProgress/aQCount;
	}
	return mProgress;
}

bool RaptisoftQuery::IsRML()
{
	if (!mResult.mData) return false;
	if (mResult.Len()<RMLINDICATORLEN) return false;
	if (memcmp(mResult.mData,RMLINDICATOR,_min(RMLINDICATORLEN,mResult.Len()))==0) return true;
	return false;
}

void RCThreadLock()
{
	if (gRCThreadLock==-1) gRCThreadLock=CreateThreadLock();
	ThreadLock(gRCThreadLock);
}
void RCThreadUnlock()
{
	ThreadUnlock(gRCThreadLock);
}

void EXEThreadLock()
{
	if (gEXEThreadLock==-1) gEXEThreadLock=CreateThreadLock();
	ThreadLock(gEXEThreadLock);
}
void EXEThreadUnlock()
{
	ThreadUnlock(gEXEThreadLock);
}

#ifdef _RQUERYLOGGING
void RQuery_Log_Reset()
{
	String aFN="desktop://RQuery.log";
	IOBuffer aBuffer;
	aBuffer.Reset();
	aBuffer.CommitFile(aFN);
}
void RQuery_Log(char* theFormat, ...)
{
	String aString;
	va_list argp;
	va_start(argp, theFormat);

	//
	// Have a problem here!  A URLEncoded string here will blow up.
	// But we can't URLDecode everything because % gets decoded.
	//
	aString.Format(theFormat,argp); // If crashing here, read above
	//
	//
	va_end(argp);

	String aFN="desktop://RQuery.log";
	IOBuffer aBuffer;
	if (DoesFileExist(aFN)) aBuffer.Load(aFN);
	aBuffer.SeekEnd();
	aBuffer.WriteLine(aString);
	aBuffer.CommitFile(aFN);
}
#else
#define RQuery_Log_Reset()
#define RQuery_Log(...)
#endif
//
// Processes a query... just sends it out, receives the result, and
// that's it!
//
void RQuery_Stub(void* theArg)
{
	RaptisoftQuery* aRQ=(RaptisoftQuery*)theArg;

	while (aRQ)
	{

#ifdef _RQUERYLOGGING
		RQuery_Log("Sending: \"%s\"",aRQ->mQuery.c());
		int aTime=gAppPtr->GetAppTime();
#endif
        
		aRQ->mProgress=0;
		if (aRQ->mLocalFunction) aRQ->mLocalFunction(aRQ);
		else if (aRQ->mUpload.Len()) aRQ->mUpload.Upload(aRQ->mQuery,&aRQ->mResult,&aRQ->mProgress);
		else aRQ->mResult.Download(aRQ->mQuery,&aRQ->mProgress);
		aRQ->mProgress=1.0f;

		if (aRQ->mNextQueryCallback) if (!aRQ->mNextQueryCallback(aRQ,aRQ->mNextQuery.GetPointer())) 
		{
			//
			// Getting false from the callback means it was tagged as failed... so we will fail all subsequent queries too.
			//
			while (aRQ)
			{
				aRQ->mProgress=1.0f;
				aRQ=aRQ->mNextQuery.GetPointer();
			}
			break;
		}
		aRQ=aRQ->mNextQuery.GetPointer();
	}

	//
	// Mark everything done (we do this at the end of all sub-queries so that Done() will work call on any section
	//
	aRQ=(RaptisoftQuery*)theArg;
	while (aRQ)
	{
		aRQ->mDone=true;
		aRQ=aRQ->mNextQuery.GetPointer();
	}
}

RExecute GetNextExecute()
{
	if (gPendingExecuteList.GetCount()==0) return NULL;
	RExecute aRE=gPendingExecuteList[0];
	gPendingExecuteList-=aRE;
	return aRE;
}

void PushExecute(String theString)
{
	RExecute aRE=new RaptisoftExecute;
	aRE->mEXE=theString;
	gPendingExecuteList+=aRE;
}

///////////////////////////////////////////////////////////////////////////
//
// Makermall Specific
//
///////////////////////////////////////////////////////////////////////////
int gGameID=-1;
String gVersionID="";
String gExtraParams="";
String gURL="";
longlong gIgnoreFilter=0;
void MakerMall_SetGameID(int theGameID, String theVersionID)
{
	RCThreadLock();
	gGameID=theGameID;
	gVersionID=theVersionID;
	RQuery_Log_Reset();
	RCThreadUnlock();
}
void MakerMall_SetExtraParams(String theParams) {RCThreadLock();gExtraParams=theParams;RCThreadUnlock();}
void MakerMall_SetURL(String theString) {RCThreadLock();gURL=theString;RCThreadUnlock();}
void MakerMall_SetIgnoreFilter(longlong theIgnore) {RCThreadLock();gIgnoreFilter=theIgnore;RCThreadUnlock();}

String CreateMMURL(String thePHP,String theParams)
{
	return "";
}

RQuery LevelSharing_GetTags(void* theExtraData)
{
	RQuery aQ=new RaptisoftQuery;
	aQ->mExtraData=theExtraData;
	aQ->mFunctionPointer=(void*)&LevelSharing_GetTags;
	aQ->mQuery="http://www.yourserver.com/gettags.jsp";
	RCCompatibleSend(aQ);

	return aQ;
}

RQuery LevelSharing_Login(String theName, String thePassword, void* theExtraData)
{
	RQuery aQ=new RaptisoftQuery;
	aQ->mExtraData=theExtraData;
	aQ->mFunctionPointer=(void*)&LevelSharing_Login;
	aQ->mQuery="http://www.yourserver.com/login.jsp";
	RCCompatibleSend(aQ);

	return aQ;
}

RQuery LevelSharing_Hello(String theName, String thePassword, String theVersion, void* theExtraData)
{
	RQuery aQ=new RaptisoftQuery;
	aQ->mExtraData=theExtraData;
	aQ->mFunctionPointer=(void*)&LevelSharing_Hello;
	aQ->mQuery="http://www.yourserver.com/hello.jsp";
	RCCompatibleSend(aQ);
	return aQ;
}

RQuery ClearCommand(String theName, String thePassword, void* theExtraData)
{
	RQuery aQ=new RaptisoftQuery;
	aQ->mExtraData=theExtraData;
	aQ->mFunctionPointer=(void*)&ClearCommand;
	aQ->mQuery="http://www.yourserver.com/clearcommand.jsp";
	RCCompatibleSend(aQ);
	return aQ;
}


RQuery LevelSharing_GetLevels(int theStartCursor, int theCount, longlong theFilter, void* theExtraData)
{
	RQuery aQ=new RaptisoftQuery;
	aQ->mExtraData=theExtraData;
	aQ->mFunctionPointer=(void*)&LevelSharing_GetLevels;
	aQ->mQuery="http://www.yourserver.com/getlevels.jsp";
	RCCompatibleSend(aQ);
	return aQ;
}

RQuery LevelSharing_GetNewLevels(String theSince, int theStartCursor, int theCount, longlong theFilter, void* theExtraData)
{
	RQuery aQ=new RaptisoftQuery;
	aQ->mExtraData=theExtraData;
	aQ->mFunctionPointer=(void*)&LevelSharing_GetNewLevels;
	aQ->mQuery="http://www.yourserver.com/getnewlevels.jsp";
	RCCompatibleSend(aQ);
	return aQ;
}

RQuery LevelSharing_GetHotLevels(int theStartCursor, int theCount, longlong theFilter, void* theExtraData)
{
	RQuery aQ=new RaptisoftQuery;
	aQ->mExtraData=theExtraData;
	aQ->mFunctionPointer=(void*)&LevelSharing_GetHotLevels;
	aQ->mQuery="http://www.yourserver.com/gethotlevels.jsp";
	RCCompatibleSend(aQ);
	return aQ;
}

RQuery LevelSharing_GetStarLevels(int theStartCursor, int theCount, longlong theFilter, void* theExtraData)
{
	RQuery aQ=new RaptisoftQuery;
	aQ->mExtraData=theExtraData;
	aQ->mFunctionPointer=(void*)&LevelSharing_GetStarLevels;
	aQ->mQuery="http://www.yourserver.com/getstarlevels.jsp";
	RCCompatibleSend(aQ);

	return aQ;
}

RQuery LevelSharing_GetRandomLevels(int theStartCursor, int theCount, longlong theFilter, void* theExtraData)
{
	RQuery aQ=new RaptisoftQuery;
	aQ->mExtraData=theExtraData;
	aQ->mFunctionPointer=(void*)&LevelSharing_GetRandomLevels;
	aQ->mQuery="http://www.yourserver.com/getrandomlevels.jsp";
	RCCompatibleSend(aQ);
	return aQ;
}

RQuery LevelSharing_SearchLevels(String theNameFilter, String theAuthorFilter, bool newestFirst, int theStartCursor, int theCount, longlong theFilter, void* theExtraData)
{
	RQuery aQ=new RaptisoftQuery;
	aQ->mExtraData=theExtraData;
	aQ->mFunctionPointer=(void*)&LevelSharing_SearchLevels;
	aQ->mQuery="http://www.yourserver.com/searchlevels.jsp";
	RCCompatibleSend(aQ);
	return aQ;
}

RQuery LevelSharing_GetComments(int theLevelID, void* theExtraData)
{
	RQuery aQ=new RaptisoftQuery;
	aQ->mExtraData=theExtraData;
	aQ->mFunctionPointer=(void*)&LevelSharing_GetComments;
	aQ->mQuery="http://www.yourserver.com/getcomments.jsp";
	RCCompatibleSend(aQ);
	return aQ;
}

RQuery LevelSharing_DownloadLevel(int theLevelID, void* theExtraData)
{
	RQuery aQ=new RaptisoftQuery;
	aQ->mExtraData=theExtraData;
	aQ->mFunctionPointer=(void*)&LevelSharing_DownloadLevel;
	aQ->mQuery="http://www.yourserver.com/downloadlevel.jsp";
	RCCompatibleSend(aQ);
	return aQ;
}

RQuery LevelSharing_DownloadLevelInfo(int theLevelID, void* theExtraData)
{
	RQuery aQ=new RaptisoftQuery;
	aQ->mExtraData=theExtraData;
	aQ->mFunctionPointer=(void*)&LevelSharing_DownloadLevel;
	aQ->mQuery="http://www.yourserver.com/downloadlevelinfo.jsp";
	RCCompatibleSend(aQ);
	return aQ;
}


RQuery LevelSharing_DownloadLevelPreview(int theLevelID, void* theExtraData)
{
	RQuery aQ=new RaptisoftQuery;
	aQ->mExtraData=theExtraData;
	aQ->mFunctionPointer=(void*)&LevelSharing_DownloadLevelPreview;
	aQ->mQuery="http://www.yourserver.com/downloadlevelpreview.jsp";
	RCCompatibleSend(aQ);
	return aQ;
}

bool MMUploadCallback(RaptisoftQuery* thePreviousQuery, RaptisoftQuery* theNextQuery)
{
	String aResult=thePreviousQuery->GetResult().ToString();
	if (aResult=="OK") return true;
	return false;
}

RQuery LevelSharing_UploadLevel(String theUsername, String thePassword, String theLevelName, longlong theTags, IOBuffer& theLevelFile, IOBuffer& thePreviewFile, String theClientVersion, void* theExtraData)
{
	RQuery aQ=new RaptisoftQuery;
	aQ->mExtraData=theExtraData;
	aQ->mFunctionPointer=(void*)&LevelSharing_UploadLevel;
	aQ->mQuery="http://www.yourserver.com/uploadlevel.jsp";
	aQ->mUpload.Copy(&theLevelFile);

	RQuery aQQ=new RaptisoftQuery;
	aQQ->mExtraData=theExtraData;
	aQQ->mFunctionPointer=(void*)&LevelSharing_UploadLevel;
	aQ->mQuery="http://www.yourserver.com/uploadlevelpreview.jsp";
	aQQ->mUpload.Copy(&thePreviewFile);

	aQ->mNextQuery=aQQ;
	aQ->mNextQueryCallback=&MMUploadCallback;
	RCCompatibleSend(aQ);
	return aQ;
}

RQuery LevelSharing_UploadLevelNoPreview(String theUsername, String thePassword, String theLevelName, longlong theTags, IOBuffer& theLevelFile, String theClientVersion, void* theExtraData)
{
	RQuery aQ=new RaptisoftQuery;
	aQ->mExtraData=theExtraData;
	aQ->mFunctionPointer=(void*)&LevelSharing_UploadLevel;
	aQ->mQuery="http://www.yourserver.com/uploadlevelnopreview.jsp";
	aQ->mUpload.Copy(&theLevelFile);
	RCCompatibleSend(aQ);
	return aQ;
}

RQuery LevelSharing_PlayedLevel(int theLevelID, void* theExtraData)
{
	RQuery aQ=new RaptisoftQuery;
	aQ->mExtraData=theExtraData;
	aQ->mFunctionPointer=(void*)&LevelSharing_PlayedLevel;
	aQ->mQuery="http://www.yourserver.com/playedlevel.jsp";
	RCCompatibleSend(aQ);
	return aQ;
}

RQuery LevelSharing_WonLevel(int theLevelID, void* theExtraData)
{
	RQuery aQ=new RaptisoftQuery;
	aQ->mExtraData=theExtraData;
	aQ->mFunctionPointer=(void*)&LevelSharing_WonLevel;
	aQ->mQuery="http://www.yourserver.com/wonlevel.jsp";
	RCCompatibleSend(aQ);
	return aQ;
}

RQuery LevelSharing_GetLocalStats(String theUsername, String thePassword, Array<int>& theLevelIDList, void* theExtraData)
{
	String aLevelList;
	for (int aCount=0;aCount<theLevelIDList.Size();aCount++) {if (aCount>0) aLevelList+=",";aLevelList+=theLevelIDList[aCount];}

	RQuery aQ=new RaptisoftQuery;
	aQ->mExtraData=theExtraData;
	aQ->mFunctionPointer=(void*)&LevelSharing_GetLocalStats;
	aQ->mQuery="http://www.yourserver.com/getlocalstats.jsp";
	RCCompatibleSend(aQ);

	return aQ;
}

RQuery LevelSharing_GetLevelStats(Array<int>& theLevelIDList, void* theExtraData)
{
	String aLevelList;
	for (int aCount=0;aCount<theLevelIDList.Size();aCount++) {if (aCount>0) aLevelList+=",";aLevelList+=theLevelIDList[aCount];}

	RQuery aQ=new RaptisoftQuery;
	aQ->mExtraData=theExtraData;
	aQ->mFunctionPointer=(void*)&LevelSharing_GetLevelStats;
	aQ->mQuery="http://www.yourserver.com/getlevelstats.jsp";
	RCCompatibleSend(aQ);
	return aQ;
}

RQuery LevelSharing_Vote(int theLevelID, int theVote, void* theExtraData)
{
	if (!GetGameGUID()) return NULL;

	RQuery aQ=new RaptisoftQuery;
	aQ->mExtraData=theExtraData;
	aQ->mFunctionPointer=(void*)&LevelSharing_Vote;
	aQ->mQuery="http://www.yourserver.com/vote.jsp";
	RCCompatibleSend(aQ);
	return aQ;
}


RQuery LevelSharing_Comment(String theUsername, String thePassword, int theLevelID, String theComment, void* theExtraData)
{
	RQuery aQ=new RaptisoftQuery;
	aQ->mExtraData=theExtraData;
	aQ->mFunctionPointer=(void*)&LevelSharing_Comment;
	aQ->mQuery="http://www.yourserver.com/comment.jsp";
	RCCompatibleSend(aQ);
	return aQ;
}

RQuery LevelSharing_DeleteLevel(String theUsername, String thePassword, int theLevelID, void* theExtraData)
{
	RQuery aQ=new RaptisoftQuery;
	aQ->mExtraData=theExtraData;
	aQ->mFunctionPointer=(void*)&LevelSharing_DeleteLevel;
	aQ->mQuery="http://www.yourserver.com/deletelevel.jsp";
	RCCompatibleSend(aQ);

	return aQ;
}

RQuery LevelSharing_Recover(String theName, void* theExtraData)
{
	RQuery aQ=new RaptisoftQuery;
	aQ->mExtraData=theExtraData;
	aQ->mFunctionPointer=(void*)&LevelSharing_Recover;
	aQ->mQuery="http://www.yourserver.com/recoveraccount.jsp";
	RCCompatibleSend(aQ);
	return aQ;
}

RQuery LevelSharing_SendActivation(String theName, void* theExtraData)
{
	RQuery aQ=new RaptisoftQuery;
	aQ->mExtraData=theExtraData;
	aQ->mFunctionPointer=(void*)&LevelSharing_SendActivation;
	aQ->mQuery="http://www.yourserver.com/sendactivation.jsp";
	RCCompatibleSend(aQ);

	return aQ;
}

RQuery LevelSharing_CreateAccount(String theName, String thePassword, String theEmail, void* theExtraData)
{
	RQuery aQ=new RaptisoftQuery;
	aQ->mExtraData=theExtraData;
	aQ->mFunctionPointer=(void*)&LevelSharing_CreateAccount;
	aQ->mQuery="http://www.yourserver.com/createaccount.jsp";
	RCCompatibleSend(aQ);
	return aQ;
}

RQuery LevelSharing_Serve(String theUsername, String thePassword, String theRequest, String extraParams, void* theExtraData)
{
	RQuery aQ=new RaptisoftQuery;
	aQ->mExtraData=theExtraData;
	aQ->mFunctionPointer=(void*)&LevelSharing_Serve;
	aQ->mQuery="http://www.yourserver.com/serve.jsp";
	RCCompatibleSend(aQ);
	return aQ;
}

RQuery SendSecretCode(String theCode, void* theExtraData)
{
	RQuery aQ=new RaptisoftQuery;
	aQ->mExtraData=theExtraData;
	aQ->mFunctionPointer=(void*)&SendSecretCode;
	aQ->mQuery="http://www.yourserver.com/sendsecretcode.jsp";
	RCCompatibleSend(aQ);
	return aQ;
}

RQuery LevelSharing_Boost(String theUsername, String thePassword, int theLevelID, char theBoostDir, String extraParams, void* theExtraData)
{
	RQuery aQ=new RaptisoftQuery;
	aQ->mExtraData=theExtraData;
	aQ->mFunctionPointer=(void*)&LevelSharing_Boost;
	aQ->mQuery="http://www.yourserver.com/boostlevel.jsp";
	RCCompatibleSend(aQ);
	return aQ;
}

RQuery GetGUID(String extraParams, void* theExtraData)
{
	RQuery aQ=new RaptisoftQuery;
	aQ->mExtraData=theExtraData;
	aQ->mFunctionPointer=(void*)&GetGUID;
	aQ->mQuery="http://www.yourserver.com/getguid.jsp";
	RCCompatibleSend(aQ);

	return aQ;
}

RQuery LevelSharing_GiveStar(String theUsername, String thePassword, int theLevelID, void* theExtraData)
{
	RQuery aQ=new RaptisoftQuery;
	aQ->mExtraData=theExtraData;
	aQ->mFunctionPointer=(void*)&LevelSharing_GiveStar;
	aQ->mQuery="http://www.yourserver.com/givestar.jsp";
	RCCompatibleSend(aQ);

	return aQ;
}

RQuery LevelSharing_ReportAbuse(int theLevelID, String theReason, void* theExtraData)
{
	if (!GetGameGUID()) return NULL;

	RQuery aQ=new RaptisoftQuery;
	aQ->mExtraData=theExtraData;
	aQ->mFunctionPointer=(void*)&LevelSharing_ReportAbuse;
	aQ->mQuery="http://www.yourserver.com/reportabuse.jsp";
	RCCompatibleSend(aQ);
	return aQ;
}

RQuery Log(String theLogString, void* theExtraData)
{
	RQuery aQ=new RaptisoftQuery;
	aQ->mExtraData=theExtraData;
	aQ->mFunctionPointer=(void*)&Log;
	aQ->mQuery="http://www.yourserver.com/log.jsp";
	RCCompatibleSend(aQ);
	return aQ;
}

RQuery Custom(RCOMMHOOKPTR theHook, void* theExtraData)
{
	// Just runs a local function but acts like it's a server query for seamlessness
	RQuery aQ=new RaptisoftQuery;

	RunAfterNextDraw(HOOKLOCAL(
		{
		aQ->mExtraData=theExtraData;
		aQ->mFunctionPointer=(void*)&Custom;
		aQ->mLocalFunction=theHook;
		RCCompatibleSend(aQ);
		}));

	return aQ;
}

RQuery SendData(IOBuffer& theBuffer, String theDescription, String theFileExtension, void* theExtraData)
{
	RQuery aQ=new RaptisoftQuery;
	aQ->mExtraData=theExtraData;
	aQ->mFunctionPointer=(void*)&SendData;
	aQ->mQuery="http://www.yourserver.com/senddata.jsp";
	aQ->mUpload.Copy(&theBuffer);
	RCCompatibleSend(aQ);
	return aQ;
}

RQuery ReportDeviceID(String extraParams, void* theExtraData)
{
	RQuery aQ=new RaptisoftQuery;
	aQ->mExtraData=theExtraData;
	aQ->mFunctionPointer=(void*)&ReportDeviceID;
	aQ->mQuery="http://www.yourserver.com/reportdeviceid.jsp";
	RCCompatibleSend(aQ);
	return aQ;
}

inline bool IsRQueryLogging()
{
#ifdef _RQUERYLOGGING
	return true;
#endif
	return false;
}

void LogRQuery(RQuery theQuery)
{
	#ifdef _RQUERYLOGGING
	{
		RQuery_Log("Sent: \"%s\"",theQuery->mQuery.c());

		if (theQuery->IsRML())
		{
			RQuery_Log("Result: %s",theQuery->mResult.ToString().c());
			RQuery_Log("");
			String aRString=theQuery->mResult.ToString();
			String aTag;
			String aValue;
			while (aRString.ParseRML(aTag,aValue)) {RQuery_Log("   [%s] = %s",aTag.c(),aValue.c());}
		}
		else RQuery_Log("Result: (Downloaded File, Length=%d) %s",theQuery->mResult.Len(),theQuery->mResult.ToString().c());
		RQuery_Log("");
	}
	#endif
}

void RCCompatibleSend(RQuery theQuery,bool sendEvenIfNotConnected) // ::RCCompatibleSend
{
	if (!sendEvenIfNotConnected) if (!IsConnected()) {gPendingRQueryList+=theQuery;return;}

	theQuery->mResult.mQueryPtr=theQuery;
	if (theQuery->mLocalFunction) 
	{
		//
		// "Custom" query... just invoke and leave...
		//
		theQuery->mLocalFunction(theQuery);
		theQuery->mProgress=1.0f;
		return;
	}
	if (theQuery->mUpload.Len()) theQuery->mUpload.mQueryPtr=theQuery;

	IOBUFFERHOOKPTR aHook=IOBUFFERHOOK(
		{
			QueryBuffer* theQBuffer=(QueryBuffer*)theArg;
			RQuery aRQ=theQBuffer->mQueryPtr;
			theQBuffer->mQueryPtr=NULL;

			aRQ->mProgress=1.0f;
			if (IsRQueryLogging()) LogRQuery(aRQ);

			aRQ->mResult.SeekBeginning();
			if (aRQ->mCallback!=NULL) aRQ->mCallback(aRQ);
			
			aRQ->mDone=true;
			bool aSuccess=true;
			if (aRQ->mVerifyCallback) aSuccess=aRQ->mVerifyCallback(aRQ);

			if (aRQ->mNextQuery.IsNotNull())
			{
				if (aRQ->mNextQueryCallback) if (!aRQ->mNextQueryCallback(aRQ,aRQ->mNextQuery.GetPointer())) aSuccess=false;
				if (!aSuccess)
				{
					while (aRQ.IsNotNull())
					{
						aRQ->mProgress=1.0f;
						aRQ->mDone=true;
						aRQ=aRQ->mNextQuery;
					}
				}
				else RCCompatibleSend(aRQ->mNextQuery);
			}
		}
	);


	if (!theQuery->mQuery.StartsWith(gURL.c())) theQuery->mQuery.Insert(gURL.c());

#ifdef _RQUERYLOGGING
	RQuery_Log("SENDING REQUEST:");
	RQuery_Log(theQuery->mQuery);
	RQuery_Log("");
#endif


	if (theQuery->mUpload.Len()) 
	{
		theQuery->mUpload.UploadA(theQuery->mQuery,&theQuery->mResult,&theQuery->mProgress,aHook);
	}
	else 
	{
		theQuery->mResult.DownloadA(theQuery->mQuery,&theQuery->mProgress,aHook);
	}
}

void SetGameID(int theGameID, String theVersionID) {RCThreadLock();gGameID=theGameID;gVersionID=theVersionID;RQuery_Log_Reset();RCThreadUnlock();}
void SetURL(String theString) {RCThreadLock();gURL=theString;RCThreadUnlock();}
String GetURL() {return gURL;}
String GetURLPrefix(String theAppend) {String aResult=gURL.GetSegmentBefore(':');aResult+=theAppend;return aResult;}
void SetExtraParams(String theParams) {RCThreadLock();gExtraParams=theParams;RCThreadUnlock();}
void SetIgnoreFilter(longlong theIgnore) {RCThreadLock();gIgnoreFilter=theIgnore;RCThreadUnlock();}

bool IsConnecting() {return false;}
bool IsConnected() {return true;}
unsigned int GetGameGUID() {return gMM_GameGUID;}
bool IsNews() {return false;}
String GetNews() {return "";}
void NewsAlert(String theAlertString) {}


RQuery Ping(RCOMMHOOKPTR theCallback)
{
	RQuery aQ=new RaptisoftQuery;
	aQ->mCaller=(void*)&Ping;
	aQ->mQuery="http://www.yourserver.com/ping.jsp";
	aQ->mCallback=theCallback;
	RCCompatibleSend(aQ,true);
	return aQ;
}


void ConnectAndHandshake(String theURL);
void RetrieveNews();
void RequestGameID();
void SetConnected() {gMM_IsConnecting=false;gMM_IsConnected=true;}

RQuery Connect(String theURL) //::Connect
{
	if (IsConnected() || IsConnecting()) return NULL;

	// Do whatever you need to do to connect to the server, bound up in an RQuery.
	// Return immediately.
	return NULL;
}


void ConnectAndHandshake(String theURL)
{
	// Do whatever you need to do to connect to your server and verify before returning
}

void RequestGameID()
{
	// Do whatever you need to do to try to request a unique ID for this installation.  Mac address, server hello,
	// ip address, whatevs.
}

void RetrieveNews()
{
	// Ask your server for news
}

void SetNewsRead()
{
	// Mark your news as read however you want
}

void FreshenNews()
{
	// Just re-retrieves the news... in case it changed and app hasn't ever stopped running.
	RetrieveNews();
}

int GetFlagIcon(String theCode)
{
	// Whatever flag icon set you're using, send back the number of the flag icon.
	return 0;
}

Pending::Pending(RQuery theQuery, RCOMMHOOKPTR onCompleteHook, CPUHOOKPTR drawHook, CPUHOOKPTR initializeHook)
{
	mQuery=theQuery;
	mDrawHook=drawHook;
	mInitializeHook=initializeHook;
	mOnComplete=onCompleteHook;
}

void Pending::Update()
{
	if (mQuery->IsDone())
	{
		Kill();
		if (mOnComplete) mOnComplete(mQuery);
	}
}

void FireAndForget(RQuery theQuery, RCOMMHOOKPTR onCompleteHook) { gAppPtr->mBackgroundProcesses+=new Pending(theQuery,onCompleteHook); }

} // Namespace RComm

