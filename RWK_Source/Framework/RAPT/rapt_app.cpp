#include "rapt_app.h"
#include "os_core.h"
#include "graphics_core.h"
#include "sound_core.h"
#include "network_core.h"
#include "social_core.h"
#include "transaction_core.h"

App *gAppPtr=NULL;

void gApp_SaveEverything();
void LoadingBackgroundThread(void *theArg) {gG.Threading();gAppPtr->LoadBackground();gAppPtr->mRunLoadBackgroundComplete=true;}

//
// For quickly/easily faking no threading...
//
#ifndef NO_THREADS
void Thread(void (*theFunction)(void *theArg), void* theArg) {;OS_Core::Thread(theFunction,theArg);}
#endif
void SetThreadPriority(float thePriority) {OS_Core::SetThreadPriority(thePriority);}
int CreateThreadLock() {return OS_Core::CreateThreadLock();}
void KillThreadLock(int theLock) {OS_Core::ReleaseThreadLock(theLock);}
void DeleteThreadLock(int theLock) {OS_Core::ReleaseThreadLock(theLock);}
void ThreadLock(int theLock) {OS_Core::ThreadLock(theLock);}
void ThreadUnlock(int theLock) {OS_Core::ThreadUnlock(theLock);}

void CopyToClipboard(String theText) {OS_Core::CopyToClipboard(theText.c());}
String PasteFromClipboard() {String aResult=OS_Core::PasteFromClipboard();return aResult;}

int gThrottleThreadlock=0;
void ThrottleLock() {if (!gThrottleThreadlock) gThrottleThreadlock=CreateThreadLock();ThreadLock(gThrottleThreadlock);}
void ThrottleUnlock() {if (gThrottleThreadlock) ThreadUnlock(gThrottleThreadlock);}

bool gIgnoreNextTouchEnd=false;	// Ignores the next touch-end if we forced a touchend.

CleanArray<int> gCurrentTouchIDs;

#ifdef _DEBUG
bool gWatching=false;
#endif

//
// Attempted "recording" of all app messages...
//
#ifdef _RECORDER
struct Recorder
{
	struct Record
	{
		int					mAppTime;
		int					mMessage;
		int					mID;
		int					mData1;
		int					mData2;
		int					mData3;
		void*				mDataPtr; // Won't actually be valid!
	};

	List				mRecordList;

	void				Save()
	{
		IOBuffer aBuffer;
		EnumList(Record,aR,mRecordList)
		{
			aBuffer.WriteInt(aR->mAppTime);
			aBuffer.WriteInt(aR->mMessage);
			aBuffer.WriteInt(aR->mID);
			aBuffer.WriteInt(aR->mData1);
			aBuffer.WriteInt(aR->mData2);
			aBuffer.WriteInt(aR->mData3);
		}
		aBuffer.CommitFile("sandbox://recorder.dat");
	}
};

struct Playbacker
{
	struct Record
	{
		int					mAppTime;
		int					mMessage;
		int					mID;
		int					mData1;
		int					mData2;
		int					mData3;
		void*				mDataPtr; // Won't actually be valid!
	};

	List				mRecordList;
	int					mPos;

	void				Load(String theFilename)
	{
		mPos=0;
		IOBuffer aBuffer;
		aBuffer.Load(theFilename);
		while (!aBuffer.IsEnd())
		{
			Record* aR=new Record;
			mRecordList+=aR;

			aR->mAppTime=aBuffer.ReadInt();
			aR->mMessage=aBuffer.ReadInt();
			aR->mID=aBuffer.ReadInt();
			aR->mData1=aBuffer.ReadInt();
			aR->mData2=aBuffer.ReadInt();
			aR->mData3=aBuffer.ReadInt();
			aR->mDataPtr=NULL;
		}
	}
};

Recorder*				gRecorder=NULL;
Playbacker*				gPlaybacker=NULL;
void Playback(String theFilename)
{
	if (!gAppPtr) return;

	gPlaybacker=new Playbacker;
	gPlaybacker->Load(theFilename);
	Playbacker::Record* aR=(Playbacker::Record*)gPlaybacker->mRecordList[0];
	if (!aR) {delete gPlaybacker;gPlaybacker=NULL;}
	else gAppPtr->mAge=aR->mAppTime-1;
}
void ThrottlePlayback()
{
	WARNING("Remember: ThrottlePlayback does NOT correctly grab random numbers!");
	for (;;)
	{
		if (!gAppPtr) break;
		Playbacker::Record* aR=(Playbacker::Record*)gPlaybacker->mRecordList[gPlaybacker->mPos];
		if (!aR) 
		{
			gOut.Out("!!!! Finished Playback!");
			delete gPlaybacker;
			gPlaybacker=NULL;
			break;
		}
		else if (gAppPtr->mAge==aR->mAppTime) 
		{
			gAppPtr->System_ProcessMessage(aR->mMessage,aR->mID,aR->mData1,aR->mData2,aR->mData3,aR->mDataPtr);
			gPlaybacker->mPos++;
		}
		else break;
	}
}
#endif

App::App()
{
	gAppPtr=this;

	mAppStartTick=Tick();
	gObjectUID=mAppStartTick;

	mLoader=NULL;
	mSystemState=SYSTEMSTATE_STARTUP;


	mAppName="Unnamed App";
	mPublisher="Raptisoft";
	mAppVersion="0.01a";


	mAppDrawNumber=0;
    mRunLoadBackgroundComplete=false;
	mLoadComplete=false;
    mOnscreenKeyboardY=0;
    mOnscreenKeyboardChanged=false;
	EnableMultitasking();
	mToolVersion=0;
	//mCloudLoadPending=false;
	//mCloudStatus=CLOUDSTATUS_NULL;

	SetUpdatesPerSecond(100);
	RecoverTime();

	mFocusKeyboardCPU=NULL;
	mFocusTouchesCPU=NULL;
	mFocusSpinsCPU.Reset();
	mFocusAccelerometerCPU=NULL;
	mHoverCPU=this;
	mFocusBackButtonCPUList.Unique();

	mFPS=60;

	mMessageData_TouchID=0;
	mMessageData_TouchTime=-1;
	mMessageData_LastTouchTime=-9999;
	mMessageData_TouchPosition=Point(400,300);
	mMessageData_LastTouchPosition=Point(400,300);
	mMessageData_TouchStartPosition=Point(400,300);
	mMessageData_IsTouching=0;
	mMessageData_TouchStatus=0;
	mMessageData_MouseButton=-1;
	mIsShutdown=false;
	mIsolatedCPUManager=NULL;

	mPauseWhenInBackground=true;
	mPauseSoundWhenInBackground=true;
	mInBackground=false;
	mNeedBackgroundDraw=false;
	mVisibilityScroll=0;
	mVisibilityY=0;
	mVisibilityYExtraSpace=10;
	mGarbageCollect=false;

	mUpdateMultiplier=1;

	mSkipDraw=0;
	mBackgroundProcesses.Unique();
}

App::~App()
{
#ifdef _RECORDER
	if (gRecorder) gRecorder->Save();
#endif

	_FreeList(FocusTouchID,mFocusTouchesByIDList);
	mBackgroundProcesses.Free();
	if (mCPUManager) mCPUManager->Free();

	if (gAppPtr==this) gAppPtr=NULL;
	Network_Core::Shutdown();

}

void App::SetUserName(String theName)
{
	mUserName=theName;
	gAppPtr->mSettings.Changed();
}

void App::SetUserPassword(String theName)
{
	mUserPassword=theName;
	gAppPtr->mSettings.Changed();
}

void App::SetSessionToken(String theToken)
{
	mSessionToken=theToken;
	gAppPtr->mSettings.Changed();
}

void App::SetComputerName(String theName)
{
	mComputerName=theName;
	gAppPtr->mSettings.Changed();
}

void App::SetIPAddress(String theAddress)
{
	mIPAddress=theAddress;
	gAppPtr->mSettings.Changed();
}

bool App::HasCloud() {return OS_Core::HasCloud();}

void App::CatchCrashes(String theURL)
{
#ifdef WIN32
	OS_Core::CatchCrashes(theURL.c());
#endif
}


void App::Go(bool runMainLoop)
{
	mCrashInfoThreadlock=OS_Core::CreateThreadLock();

	//
	// This is basically OS startup...
	//
	Network_Core::Startup();
	Transaction_Core::Startup();

	OS_Core::SetAppName(mAppName.c());
	{static char aWindowTitle[]="kittyengine";OS_Core::SetWindowName(aWindowTitle);}	// kittyengine: the window carries this project's name
	OS_Core::SetPublisher(mPublisher.c());
	OS_Core::SetAppVersion(mAppVersion.c());

	//
	// If a WAD file exists in resources, unWAD it!
	//

	//
	// This is an artifact from the Nightmare Android Days... although the Asset Loader seems to deploy wrong?
	// We might consider re-wadding.
	//
	/*
	{
		String aWADFile=PointAtPackage(OS_Core::GetWADName());
		if (GetCommandLine().ContainsI("-makewad"))
		{
			IOBuffer aWAD;
			MakeWAD(PointAtPackage(""),aWAD,"exe;dll;exp;lib;map;pdb;suo;");
			aWAD.CommitFile(aWADFile);
		}
		else if (DoesFileExist(aWADFile)) 
		{
			IOBuffer aWAD;
			aWAD.Load(aWADFile);
			ExtractWAD(aWAD,PointAtPackage(""));
			DeleteFile(aWADFile);
		}
	}
	*/

	//
	// Grab auto-clouding before other stuff, since it's needed to make our load work...
	//
	Settings aPreCheck;
	aPreCheck.SetReadOnly(true);
	aPreCheck.Load("sandbox://settings.txt");

	gSocial.Go();	// Social must start before cloud, so that we're actually "signing in"

	if (!IsMinimalistApp())
    {
		//WARNING("ALWAYS Grabbing from the Cloud!!!!");
		//if (DoesFileExist(PointAtSandbox("settings.txt")) || !OS_Core::HasCloud() || !mAutoCloud) LoadSettings();

		LoadSettings();
		
		/*
		if (!OS_Core::HasCloud() || !mAutoCloud) LoadSettings();
		else
		{
			mCloudLoadPending=true;
			Thread(&PullFromCloud);
		}
		*/
    }


/*
	if (!gAppPtr->mSettings.Exists("System.Username")) gAppPtr->mSettings.SetString("System.Username","");
	if (!gAppPtr->mSettings.Exists("System.Password")) gAppPtr->mSettings.SetString("System.Password","");
	if (!gAppPtr->mSettings.Exists("System.ComputerName")) gAppPtr->mSettings.SetString("System.ComputerName",OS_Core::GetLocalName());

	mUserName=gAppPtr->mSettings.GetString("System.Username");
	mUserPassword=gAppPtr->mSettings.GetString("System.Password");
	mUserToken=gAppPtr->mSettings.GetString("System.Token");
	mComputerName=gAppPtr->mSettings.GetString("System.ComputerName");
*/

	gRand.Go();
	gMath.Go();
	gMath3D.Go();
	gConsole.Go();
	mGraphics.Go();
	mInput.Go();
	mAudio.Go();

#ifdef _DEBUG
	gOut.Out("$Raptisoft [RAPT] Framework ... startup");
	gOut.Out("$gObjectUID = %d",gObjectUID);
#endif
   
	//
	// Remove any legacy folders...
	//
	mCommandLine=OS_Core::GetCommandLine();

	//
	// Little kludge to make sure the cache folder exists... on Android, it does NOT like to do queries into Java after app stop, which is when
	// it writes crap out...
	//
	DoesFileExist("cache://nothing.here");

	Initialize();
	Size(gG.GetPageRect());



/*
	IOBuffer aUnWad;
	aUnWad.Load(PointAtDesktop("cubic.wad"));
	ExtractWAD(aUnWad,PointAtDesktop("wtest\\"));
*/	


	//
	// Fire off the loading thread...
	//
//*
    mGraphics.Threading();
	mLoader=new AssetLoader;

	Load();
	RecoverTime();

	*this+=mLoader;

	mInitComplete=true;

	if (runMainLoop)
	{
		//
		// Process until the game quits...
		// On quit, we give it a few updates to clean things up (kill off throttled CPUs and suchlike)
		// On some systems (WASM) we set up the main loop and Stop() elsewhere...
		//
		short aQuitTicker=10;
		while (!IsQuit() || aQuitTicker>0) 
		{
			Throttle();
			if (IsQuit()) aQuitTicker--;
		}
		Stop();
	}
}

bool App::DoesCloudDataExist()
{
	/*
	static char aIsCloudEmpty=-1;
	if (aIsCloudEmpty==-1) aIsCloudEmpty=OS_Core::IsCloudEmpty();
	return (aIsCloudEmpty==0);
	*/

	//
	// No caching this... caching it in the core...
	//

	//gOut.Out("IsCloudEmpty: %d",OS_Core::IsCloudEmpty());
	return !(OS_Core::IsCloudEmpty());
}


void App::LoadSettings()
{
	mSettings.Load("sandbox://settings.txt");

	mSettings.ThreadLock();

	gAppPtr->mSettings.SetString("Game.OS",OS_Core::GetOSName());
	gAppPtr->mSettings.RegisterVariable("Game.Username",mUserName,"");
	gAppPtr->mSettings.RegisterVariable("Game.Userpassword",mUserPassword,"");
	gAppPtr->mSettings.RegisterVariable("Game.Startups",mStartups,0);
	gAppPtr->mSettings.RegisterVariable("Game.Token",mSessionToken,"");
	gAppPtr->mSettings.RegisterVariable("AutoCloud",mAutoCloud,false);
	gAppPtr->mSettings.RegisterVariable("AskedAutoCloud",mAskedAutoCloud,false);
	gAppPtr->mSettings.RegisterVariable("Game.PlayCount",mPlayCounter,0);
	gAppPtr->mSettings.RegisterVariable("Game.ToolVersion",mToolVersion,0);
	gAppPtr->mSettings.mIsChanged=false; // So we don't do a save just because we set the OS name...

	//
	// App settings now get clouding turned on, because we've had a successful load (we don't want to pause things unless we had a successful load!)
	//
	mSettings.ThreadUnlock();

}


/*
void App::PullFromCloud(void* theArg)
{
	gOut.Out("!!!! Fetching settings from Cloud... starting!");
	while (gSocial.IsConnecting()) {Sleep(100);}

	//
	// This happens automatically in the core-- the manual sign-in.
	//
	//if (!gSocial.IsConnected())
	//{
	//	gSocial.Go(); // Trying manual sign-in...
	//	while (gSocial.IsConnecting()) Sleep(100);
	//}
	//

	bool aReloadSettings=false;
	if (gSocial.IsConnected())
	{

		//#endif
		//
		// Get Cloud List...
		// Anyone with "Cloud." or "Sandbox." or "App." gets pulled down and put in the proper place.
		//
		Array<String> aTempArray;
		#ifdef _DEBUG
		gOut.Out("...getting cloud keys...");
		#endif
		EnumCloud(aTempArray);
		#ifdef _DEBUG
		gOut.Out("...getting cloud keys...done");
		#endif

		//
		// Count up how many will be loaded...
		//
		Array<String> aArray;
		bool aGrabEverything=false;
		if (!DoesFileExist(PointAtSandbox("cloudinfo.nocloud"))) aGrabEverything=true;
		gOut.Out("Cloud Fetch Status Flag = %d,%d",aGrabEverything,aTempArray.Size());
		for (int aCount=0;aCount<aTempArray.Size();aCount++) 
		{
			if (aTempArray[aCount].StartsWith("APP.") || aTempArray[aCount].StartsWith("CLOUD.") || aTempArray[aCount].StartsWith("SANDBOX.")) 
			{
				String aFN=KeyNameToFN(aTempArray[aCount]);
				if (aGrabEverything || !DoesFileExist(aFN)) 
				{
					gOut.Out("Preparing to Cloudfetch: %s",aFN.c());
					aArray+=aTempArray[aCount];
				}
			}
		}

		//if (gAppPtr && gAppPtr->mLoader) 
		//{
		//	gAppPtr->mLoader->mCloudLoadMax=aArray.Size();
		//	gAppPtr->mLoader->mCloudLoadCounter=0;
		//}

		//
		// Grab "APP" critical files first...
		//
		for (int aCount=0;aCount<aArray.Size();aCount++)
		{
			if (gAppPtr) if (gAppPtr->DidCloudFail()) break;
			if (aArray[aCount].StartsWith("APP."))
			{
				String aKey=aArray[aCount].GetSegmentAfter("APP.");

				#ifdef _DEBUG
				gOut.Out("...getting APP file %s",aKey.c());
				#endif
				IOBuffer aBuffer;
				int aFudge=0;
				gOut.Out("...CloudFetching: %s",aKey.c());
				while (aBuffer.Len()==0 && ++aFudge<3)
				{
					aBuffer.CloudLoad(aArray[aCount].c());
					if (aBuffer.Len()==0) Sleep(500);
				}
				if (aFudge>=3)
				{
					// Warn them somehow?  They don't have their game back...
					gOut.Out("... *** Failed to get cloud data! ***");
					if (gAppPtr) gAppPtr->mCloudStatus=CLOUDSTATUS_FAIL;
				}
				else
				{
					ThrottleLock();
					if (gAppPtr)
					{
						if (gAppPtr->mLoader) 
						{
							gAppPtr->mLoader->mCloudLoadMax=aArray.Size();
							gAppPtr->mLoader->mCloudLoadCounter++;
						}

						String aFN;
						if (aKey=="settings") aFN=PointAtSandbox("settings.txt");
						if (aKey=="cfg") aFN=PointAtSandbox(Sprintf("%s.cfg",gAppPtr->mAppName.ToLower().c()));
						if (aFN.Len()) 
						{
							MakeDirectory(GetPathFromPath(aFN));
							aBuffer.CommitFile(aFN);
							aReloadSettings=true;	// Game needs to reload settings...
						}
					}
					ThrottleUnlock();
				}
				#ifdef _DEBUG
				gOut.Out("...getting APP file %s...done [%f]",aKey.c(),gAppPtr->GetCloudLoadComplete());
				#endif
			}
		}
		for (int aCount=0;aCount<aArray.Size();aCount++)
		{
			if (gAppPtr) if (gAppPtr->DidCloudFail()) break;
			if (aArray[aCount].StartsWith("CLOUD.") || aArray[aCount].StartsWith("SANDBOX."))
			{
				String aLocation=aArray[aCount].GetSegmentBefore('.');
				String aKey=aArray[aCount].GetSegmentAfter('.');

				IOBuffer aBuffer;
				int aFudge=0;
				gOut.Out("...CloudFetching: %s",aKey.c());
				while (aBuffer.Len()==0 && ++aFudge<3) 
				{
					aBuffer.CloudLoad(aArray[aCount].c());
					if (aBuffer.Len()==0) Sleep(500);
				}
				if (aFudge>=3)
				{
					// Warn them somehow?  They don't have their game back...
					gOut.Out("... *** Failed to get cloud data! ***");
					if (gAppPtr) gAppPtr->mCloudStatus=CLOUDSTATUS_FAIL;
				}
				else
				{
					ThrottleLock();
					if (gAppPtr)
					{
						if (gAppPtr->mLoader) 
						{
							gAppPtr->mLoader->mCloudLoadMax=aArray.Size();
							gAppPtr->mLoader->mCloudLoadCounter++;
						}

						String aFN;
						if (aLocation=="SANDBOX") aFN=PointAtSandbox(aKey);
						if (aLocation=="CLOUD") aFN=PointAtCloud(aKey);
						if (aFN.Len()) 
						{
							MakeDirectory(GetPathFromPath(aFN));
							aBuffer.CommitFile(aFN);
						}
					}
					ThrottleUnlock();
					#ifdef _DEBUG
					gOut.Out("...getting %s file %s...done [%f]",aLocation.c(),aKey.c(),gAppPtr->GetCloudLoadComplete());
					#endif
				}
			}
		}
	}
	#ifdef _DEBUG
	else gOut.Out("!!!! Failed to pull settings from the cloud!");
	#endif

	gOut.Out("!!!! Done with cloud grab...");
	if (gAppPtr) 
	{
		gAppPtr->LoadSettings();
		Sleep(500); // Gives time to draw completed loading bar...
		gAppPtr->mCloudLoadPending=false;
		if (gAppPtr->mCloudStatus!=CLOUDSTATUS_FAIL) 
		{
			gAppPtr->mCloudStatus=CLOUDSTATUS_SUCCESS;
			IOBuffer aBuffer;
			aBuffer.WriteBool(true);
			aBuffer.CommitFile(PointAtSandbox("cloudinfo.nocloud"));
			if (aReloadSettings) gAppPtr->LoadSettings();
		}
	}

	#ifdef _DEBUG
	gOut.Out("!!!! Fetching settings from Cloud... done!");
	#endif
}
*/

void App::Stop()
{
	if (!IsQuit()) {Quit();}

	gApp_SaveEverything();
	ThrottleShutdown();
	Core_Multitasking(false);
	//
	// Clean up all our CPUs, etc...
	//
	if (mCPUManager) mCPUManager->Free();


	//
	// Stop our core services...
	//
	gSocial.Stop();
	mAudio.Stop();
	mInput.Stop();
	mGraphics.Stop();

	int aFudge=0;
	while (mBackgroundProcesses.GetCount() && ++aFudge<100) mBackgroundProcesses.Update();
	mBackgroundProcesses.Free();

	//
	// For multithreaded apps, let the watcher thread know
	// that we're completely done...
	//
	_GarbageCollect();
	mIsShutdown=true;

	//
	// And, finally, make sure any settings save in case we adjusted them
	// during shutdown... we'll save regardless, because with RegisterVariables,
	// we can't reply on IsChanged.
	//
	//if (mSettings.IsChanged()) 
	mSettings.Save();
}

void gApp_PreClean()
{
	//
	// Makes sure any working stuff from previous run gets wiped out...
	//
	//if (DoesFileExist("scratch://")) DeleteDirectory("scratch://");
}

// Helper functions for mobile callbacks out of the core to finish processing everything...
void App_Finishup()
{
	if (!gAppPtr) return;
	gOut.Out("Finishing up...");
	OS_Core::OS_MessageData aMessageData;
	OS_Core::GetFirstOSMessage(&aMessageData);

#ifdef AppRunsInThread
	OS_Core::LockSystemThread(true);
#endif
	while (aMessageData.mMessage)
	{
		gAppPtr->System_ProcessMessage(aMessageData.mMessage,aMessageData.mID,aMessageData.mData[0],aMessageData.mData[1],aMessageData.mData[2],aMessageData.mDataPtr);
		OS_Core::GetNextOSMessage(&aMessageData);
	}
#ifdef AppRunsInThread
	OS_Core::LockSystemThread(false);
#endif
	gOut.Out("Finishing up done!...");
}

void gApp_SaveEverything() 
{
	//
	// If battery is critical, we won't save.
	// (The android core actually will not call into this if battery is low, but leaving it in here as a failsafe)
	//
	if (OS_Core::Query("core:outofbattery")) 
	{
		gOut.Out("Not saving, out of battery, too risky!");
		return;
	}

	if (gAppPtr)
	{
		//
		// For data safety, everything here gets saved somewhere TEMPORARY.  Then copied over.
		// This is possibly rendered obsolete by checking for outofbattery.
		//
#ifndef GL30
		//
		// Compatibility for Chuzzle, etc.  I was dumping everything into a scratch location on the assumption
		// that file saving screws up on battery loss.  HOWEVER, this causes a problem because DoesFileExist won't work correctly, which Hamsterball
		// uses to save.
		//
		OS_Core::LocationAlias("sandbox://","scratch://__sandbox/");
		OS_Core::LocationAlias("cloud://","scratch://__cloud/");

		gAppPtr->SaveEverything();
		if (gAppPtr->mSaveSettingsWhenMultitasking) gAppPtr->SaveSettings();	// I should not need this here, because settings saves whenever something changes...
																				// Normally this would catch registered variables, but those are automatic.

		OS_Core::LocationAlias("sandbox://","");
		OS_Core::LocationAlias("cloud://","");	// This is depreciated... all of Sandbox is "cloud" (as of 2021)

		//
		// Okay, so, we're doing this crazy routing because it seems that on phones, if the battery dies, everything gets a shutdown.  Then machine quits before data can be written.
		// Anyway, this sleep tries to give the machine time to die before copying files over, in the hopes that that doesn't kill anything.
		// Note that only anything that actually SAVES gets into the new directory, so it's only clouding what's changed anyway.
		//
		OS_Core::Sleep(100);
		CopyDirectory("scratch://__sandbox/","sandbox://");
		CopyDirectory("scratch://__cloud/","cloud://"); // This is depreciated... all of Sandbox is "cloud" (as of 2021)
		DeleteDirectory("scratch://");
#else
		gAppPtr->SaveEverything();
		if (gAppPtr->mSaveSettingsWhenMultitasking) gAppPtr->SaveSettings();	// I should not need this here, because settings saves whenever something changes...
																				// Normally this would catch registered variables, but those are automatic.
		OS_Core::Sleep(100);
#endif
		
		CloudQueue::Dispatch(false);

#ifdef _DEBUG
		gOut.Out("_Save Everything Done?");
		gOut.Out("");
#endif

	}
}


void App::Throttle()
{
	//
	// Throttle calls the FrameController and anything else that needs to run the system.
	//
	ThrottleLock();
	FrameController();
	ThrottleUnlock();
    mOnscreenKeyboardChanged=false;

    if (mRunLoadBackgroundComplete)
    {
		mStartups++;
		mLoadComplete=true;
        LoadBackgroundComplete();
        mRunLoadBackgroundComplete=false;
    }
}

unsigned int App::Tick()
{
	return OS_Core::Tick();
}

unsigned int App::TickUpdates()
{
	return (unsigned int)((OS_Core::Tick()/1000)*mUpdatesPerSecond);
}

void App::ThrottleUpdate()
{
#ifdef _RECORDER
	if (IsKeyPressed(KB_R) && IsKeyDown(KB_LEFTCONTROL)) 
	{
		if (!gRecorder) 
		{
			gRecorder=new Recorder;
			gOut.Out("!!!! Beginning recording!");
		}
	}
	if (IsKeyPressed(KB_P) && IsKeyDown(KB_LEFTCONTROL)) 
	{
		if (gRecorder) {delete gRecorder;gRecorder=NULL;}
		Playback("sandbox://recorder.dat");
		gOut.Out("!!!! Beginning Playback!");
	}
#endif

	mSystemState=SYSTEMSTATE_UPDATING;
	if (mGarbageCollect)
	{
		mGarbageCollect=false;
		_GarbageCollect();
	}

	Social_Core::Pump();

	mBackgroundProcesses.Update();
	if (mIsolatedCPUManager && !mIsolateTouchesOnly) mIsolatedCPUManager->Update();
	else Core_Update();

	if (mIsolatedCPUManager)
	{
		if (mIsolatedCPUManager->GetCount()<=0)
		{
			delete mIsolatedCPUManager;
			mIsolatedCPUManager=NULL;
		}
	}
	mSystemState=SYSTEMSTATE_NULL;

#ifdef _DEBUG
	// CTRL-ALT-INSERT ... our "reboot"
	if (IsKeyDown(KB_LEFTCONTROL) && IsKeyDown(KB_LEFTALT) && IsKeyDown(KB_INSERT))
	{
		OS_Core::PushOSMessage(OS_Core::OS_REBOOT);
	}
#endif
}

void App::CustomDraw(bool startOrFinish, bool drawMainPipeline)
{
	if (startOrFinish)
	{
		mAppDrawNumber++;
		gG.ThreadLock(true);
		mSystemState=SYSTEMSTATE_DRAWING;
		PreRender();
		if (drawMainPipeline) Render();
	}
	else
	{
		PostRender();
		gG.Show();
		gG.ThreadLock(false);
		mSystemState=SYSTEMSTATE_NULL;
	}
}

void App::ThrottleDraw()
{
	if (mFeatures&CPU_DRAW)
	{
		mAppDrawNumber++;
		if (!WantShutdown())
		{
			//
			// Process time slices before we draw...
			//
			if (mTimeSliceProcesses.GetCount())
			{
				int aNow=Tick();
				if (mLastTimeSlice==-1) mLastTimeSlice=aNow-(int)UpdatesToTicks(1);
				float aTime=TicksToUpdatesF(aNow-mLastTimeSlice);
				EnumList(CPU,aCPU,mTimeSliceProcesses)
				{
					if (aCPU->mKill) continue;
					aCPU->UpdateTimeslice(aCPU->mFirstDraw?0:aTime);
				}
				mLastTimeSlice=aNow;
			}

            gG.ThreadLock(true);
			mSystemState=SYSTEMSTATE_DRAWING;
			PreRender();
			Render();
			PostRender();



#ifdef _WIN32
#ifdef _DEBUG
			if (Graphics_Core::IsFakePhone())
			{
				// Testing for keyboard...
				if (mOnscreenKeyboardY>0)
				{
					gG.BeginRender();
					gG.SetColor(0,0,0,.75f);
					gG.FillRect(0,mOnscreenKeyboardY,9999,9999);
					gG.FillRect(0,mOnscreenKeyboardY,9999,10);
					gG.EndRender();
				}
			}
#endif
#endif

            gG.Show();
            gG.ThreadLock(false);
			mSystemState=SYSTEMSTATE_NULL;
		}
	}
}

//void App::TranslateForVisibility() {gG.Translate(0.0f,mVisibilityScroll);}
//void App::UnTranslateForVisibility() {gG.Translate(0.0f,-mVisibilityScroll);}
void App::TranslateForVisibility() {gG.mBaseTranslate=Point(0.0f,mVisibilityScroll);gG.Translate(gG.mBaseTranslate);}
void App::UnTranslateForVisibility() {gG.Translate(-gG.mBaseTranslate);gG.mBaseTranslate*=0;}

void App::PreRender()
{
	gG.BeginRender();
    TranslateForVisibility();
//	gOut.Out("Vis: %f -> %s",mVisibilityScroll,gG.mTranslate.ToString());
}

void App::Render()
{
	Draw();
	if (mIsolatedCPUManager && !mIsolateTouchesOnly) mIsolatedCPUManager->Draw();
	else if (mCPUManager) mCPUManager->Draw();
	DrawOverlay();

	if (mToolTipCPU)
	{
		gG.PushTranslate();
		gG.PushClip();

		gG.Clip(mToolTipClip);
		gG.Translate(mToolTipTranslate);

		mToolTipCPU->DrawToolTip();
		mToolTipCPU=NULL;
		gG.PopTranslate();
		gG.PopClip();
	}
}

void App::ToolTip(CPU* theCPU) {mToolTipCPU=theCPU;mToolTipClip=gG.mClipRect;mToolTipTranslate=gG.mTranslate;}


void App::PostRender()
{
    UnTranslateForVisibility();
	gG.EndRender();
}


void App::RecoverTime()
{
	mFrame.mBaseUpdateTime=Tick();
	mFrame.mCurrentUpdateNumber=0;
	mFrame.mBreakUpdate=true;
	mFrame.mDesiredUpdate=0;
	mLastTimeSlice=-1;
}


void App::FrameController()
{
	static unsigned int aLastDrawTime=0;
	//
	// If the App is minimized, we don't do anything here but sleep and Pump and SystemProcess
	//
    if (mInBackground)
    {
        if (OS_Core::WillAppProcessInBackground())
        {
            if ((mPauseWhenInBackground))
            {

                #ifndef AppRunsInThread
				#ifdef _RECORDER
				if (gPlaybacker) ThrottlePlayback();else
				#endif
				OS_Core::Pump();

                #endif
                System_Process();
                OS_Core::Sleep(20);
                
                if (mNeedBackgroundDraw)
                {
                    ThrottleDraw();
                    mNeedBackgroundDraw=false;
                }
                return;
            }
			if (mIdleWhenInBackground) 
			{
				#ifndef AppRunsInThread
				#ifdef _RECORDER
				if (gPlaybacker) ThrottlePlayback();else
				#endif
				OS_Core::Pump();
				#endif
				System_Process();
				if (mNeedBackgroundDraw) 
				{
					ThrottleDraw();
					mNeedBackgroundDraw=false;
				}
				else OS_Core::Sleep(200);
			}
        }
        else
        {
            System_Process();
            OS_Core::Sleep(20);
            return;
        }
    }

	//
	// Frame controller for static number of updates/sec
	//
	{

		/////////////////////////////////////////////////////////////////
		//
		// Main control loop... calls Update() and Draw() appropriately
		//
		/////////////////////////////////////////////////////////////////

		mFrame.mDesiredUpdate=(float)(Tick()-mFrame.mBaseUpdateTime);
        mFrame.mDesiredUpdate/=10;
        mFrame.mDesiredUpdate*=(float)mUpdatesPerSecond/100.0f;
        
		//
		// A glitch that can happen on Macs and iOS is that suddenly you'll get
		// a tick that's BEFORE your current time.  So if that happens, we just
		// recover and reset.
		//
		//if (mFrame.mDesiredUpdate<mFrame.mCurrentUpdateNumber || (mUpdateMultiplier==1 && mFrame.mCurrentUpdateNumber<(int)(mFrame.mDesiredUpdate-25))) CatchUp();
		if (mFrame.mDesiredUpdate<mFrame.mCurrentUpdateNumber) CatchUp();

		mFrame.mBreakUpdate=false;
		bool aShouldDraw=false;
        int aUpdateCheck=(int)mFrame.mDesiredUpdate-mFrame.mCurrentUpdateNumber;
        unsigned int aFrameStart=Tick();

		if (aUpdateCheck>0)
		{
			int aAccum=0;
			//
			// Update until we've reached our desired update number...
			//
			while (mFrame.mCurrentUpdateNumber<(int)mFrame.mDesiredUpdate && !mFrame.mBreakUpdate)
			{
				aShouldDraw=true;
			
				#ifndef AppRunsInThread
				#ifdef _RECORDER
				if (gPlaybacker) ThrottlePlayback();else
				#endif
				OS_Core::Pump();
				#endif

				System_Process(); // Here?
				if (WantShutdown()) break;
				//
				// Process the actual update
				//
				mFrame.mCurrentUpdateNumber++;
				ThrottleUpdate();
				mOnscreenKeyboardChanged=false;

				if (mUpdateMultiplier>1)
				{
					for (int aCount=1;aCount<mUpdateMultiplier;aCount++)
					{
						mFrame.mCurrentUpdateNumber++;
						ThrottleUpdate();
					}
				}
				else
				{
					//
					// If we're updating too much, we want to force drawing at 4FPS.
					// This will affect game framerate, but the alternative is to leave 
					// the program unresponsive.  We do a catchup so we don't end up in
                    // super speed mode.
					//
					if (++aAccum>(mUpdatesPerSecond/4)) {CatchUp();break;}
				}

				if (mInBackground && mIdleWhenInBackground) break;
                OS_Core::Sleep(0);
			}
		}

		//
		// If ANYONE is timeslicing, we "should draw"
		//

		if (mTimeSliceProcesses.GetCount()) aShouldDraw=true;
		if (aShouldDraw)
		{
            //*
			{

				//
				// If we've finished our updates, draw!
				// (But only draw if something happened, or
				// the draw would be completely redundant)
				//
				static unsigned int aLastDraw=0;

//                if (Tick()<aLastDraw+8) aShouldDraw=false;
				if (mLoadComplete) if (!Graphics_Core::IsTimeBeforeVSync(aLastDraw,aLastDrawTime)) aShouldDraw=false;
				if (mSkipDraw>0) {mSkipDraw--;aShouldDraw=false;}
				if (aShouldDraw) 
				{
					unsigned int aBeforeDraw=Tick();

					ThrottleDraw();
                    
					aLastDraw=Tick();
					aLastDrawTime=_max(0,aLastDraw-aBeforeDraw);	// Time we completed last draw... the _max is in there because Apple can screw up!


					//
					// Calculate FPS...
					// This gets clipped down to 60, the vertical refresh, because
					// when we lower the app's update time, the FPS goes through the
					// roof.  I am not sure why this happens at this point... I would
					// think it was from skipped ThrottleUpdate calls, but if we skip
					// those, then ShouldDraw shouldn't be happening at all...
					//
					unsigned int aFrameTime=_max(0,Tick()-aFrameStart);
					if (aFrameTime>0) mFPS=_min(60,1000/aFrameTime);

					//
					// If we're loading, draw less frequently.
					//
					if (!mLoadComplete) mSkipDraw=5;
				}
			}
            /**/
        }
        
        //
        // Formerly this was in the UpdateCheck section... but it seems we get improvement by drawing all
        // the time instead of ONLY when we know an update has happened.  So, if we are in a position of
        // skipping an update, we let it draw anyway now.
		//
		// UPDATE:	I put it back into the UpdateCheck section.  It seems to make Hoggy smoother.
		//			I might eventually make this optional.

        //
        /*
        {

			//
			// If we've finished our updates, draw!
			// (But only draw if something happened, or
			// the draw would be completely redundant)
			//
			static int aLastDraw=-1;

			//if (Tick()<aLastDraw+16) aShouldDraw=false;
            
            if (mLoadComplete) if (!Graphics_Core::IsTimeBeforeVSync(aLastDraw,aLastDrawTime)) aShouldDraw=false;
			if (mSkipDraw>0) {mSkipDraw--;aShouldDraw=false;}
			if (aShouldDraw) 
			{
				unsigned int aBeforeDraw=Tick();
                
                
				ThrottleDraw();
				aLastDraw=Tick();
				aLastDrawTime=_max(0,aLastDraw-aBeforeDraw);	// Time we completed last draw... the _max is in there because Apple can screw up!


				//
				// Calculate FPS...
				// This gets clipped down to 60, the vertical refresh, because
				// when we lower the app's update time, the FPS goes through the
				// roof.  I am not sure why this happens at this point... I would
				// think it was from skipped ThrottleUpdate calls, but if we skip
				// those, then ShouldDraw shouldn't be happening at all...
				//
				int aFrameTime=_max(0,Tick()-aFrameStart);
				if (aFrameTime>0) mFPS=_min(60,1000/aFrameTime);

				//
				// If we're loading, draw less frequently.
				//
				if (!mLoadComplete) mSkipDraw=5;
			}
		}
		/**/
        
        //
        // This seems to improve things?  Why?  But it does.  Gain of like 1-2 FPS on iOS.
        // It would be fascinating to understand why this is so.
        //
        OS_Core::Sleep(mThrottleSleep);
        
/*
		else
		{
			//
			// We're inactive
			//
			OS_Core::Sleep(1);
		}
*/
	}

	if (mInBackground && mIdleWhenInBackground) RecoverTime();
}

void App::FrameControllerTimeSlice()
{
	//
	// Frame tied to VSync for smoother performance
	// (This one requires that you modify all motion by a time slice thing)
	//
}

void App::System_Process()
{
	if (IsQuit()) return;

	mCanRefreshTouches=true;

#ifdef AppRunsInThread
	OS_Core::LockSystemThread(true);
#endif

	OS_Core::OS_MessageData aMessageData;
	OS_Core::GetFirstOSMessage(&aMessageData);

#ifdef AppRunsInThread
	OS_Core::LockSystemThread(false);
#endif
	while (aMessageData.mMessage)
	{
		if (IsQuit()) break;

//		gOut.Out("In data: %d %d %d",aMessageData.mData[0],aMessageData.mData[1],aMessageData.mData[2]);
		System_ProcessMessage(aMessageData.mMessage,aMessageData.mID,aMessageData.mData[0],aMessageData.mData[1],aMessageData.mData[2],aMessageData.mDataPtr);
#ifdef AppRunsInThread
        OS_Core::LockSystemThread(true);
#endif
		OS_Core::GetNextOSMessage(&aMessageData);
#ifdef AppRunsInThread
        OS_Core::LockSystemThread(false);
#endif
	}
}

void App::System_Clear()
{
	//
	// Finishes processing system messages.
	// We need this to Throttle gadgets, we need to clear the system
	// out or everything messes up!
	// 
	// Note that it simply acts as the second half of System_Process,
	// so you won't lock the thread here, only unlock it.  We can only be
	// here from within System_Process, theoretically.
	//
	OS_Core::OS_MessageData aMessageData;
	for (;;)
	{
#ifdef AppRunsInThread
        OS_Core::LockSystemThread(true);
#endif
		OS_Core::GetNextOSMessage(&aMessageData);
#ifdef AppRunsInThread
        OS_Core::LockSystemThread(false);
#endif
        
		if (!aMessageData.mMessage) break;
		System_ProcessMessage(aMessageData.mMessage,aMessageData.mID,aMessageData.mData[0],aMessageData.mData[1],aMessageData.mData[2],aMessageData.mDataPtr);
	}

}

void App::System_PostMessage(int theMessage, int theID, int theData1, int theData2, int theData3, void* theDataPtr)
{
    //
    // We are IN the system thread right now, so no posting threaded messages!
    //
	OS_Core::PushOSMessageNoThread(theMessage,theID,theData1,theData2,theData3,theDataPtr);
}

void App::System_PostTouch(int theX, int theY)
{
    //
    // We are IN the system thread right now, so no posting threaded messages!
    //
	OS_Core::PushOSMessageNoThread(OS_Core::OS_TOUCHMOVE,-1,theX,theY,-1);
	OS_Core::PushOSMessageNoThread(OS_Core::OS_TOUCHSTART,-1,theX,theY,-1);
	OS_Core::PushOSMessageNoThread(OS_Core::OS_TOUCHEND,-1,theX,theY,-1);
}

void App::RefreshTouches()
{
    //
    // We are IN the system thread right now, so no posting threaded messages!
    //
	OS_Core::PushOSMessageNoThread(OS_Core::OS_REFRESHTOUCHES);
}

void App::ForceTouchEnd(bool ignoreNextTouchEnd,Point thePos)
{
	if (mMessageData_IsTouching==0) return;

	if (thePos.mX!=FLT_MAX && thePos.mY!=FLT_MAX) {mMessageData_TouchStartPosition=mMessageData_TouchPosition=mMessageData_LastTouchPosition=thePos;mMessageData_MaxMove=0;}
//*
	bool aDidIt=false;
	for (int aCount=0;aCount<gCurrentTouchIDs.Size();aCount++)
	{
		if (gCurrentTouchIDs[aCount]!=-99999)
		{
			mMessageData_TouchID=gCurrentTouchIDs[aCount];
			System_TouchEnd(mMessageData_TouchPosition.mX,mMessageData_TouchPosition.mY);
			gCurrentTouchIDs[aCount]=-99999;
			aDidIt=true;
		}
	}
	mMessageData_IsTouching=0;
	if (!aDidIt)
	{
		mMessageData_TouchID=-1;
		System_TouchEnd(mMessageData_TouchPosition.mX,mMessageData_TouchPosition.mY);
	}

	if (ignoreNextTouchEnd) gIgnoreNextTouchEnd=true;
/**/
//	mMessageData_TouchID=-1;
//	System_TouchEnd(mMessageData_TouchPosition.mX,mMessageData_TouchPosition.mY);
}
void App::ForceTouchStart(Point thePos)
{
	if (thePos.mX==FLT_MAX && thePos.mY==FLT_MAX) thePos=mMessageData_TouchPosition;
	System_TouchStart((int)thePos.mX,(int)thePos.mY);
}

void App::System_ProcessMessage(int theMessage, int theID, int theData1, int theData2, int theData3, void* theDataPtr)
{
#ifdef _RECORDER
	if (gRecorder)
	{
		Recorder::Record* aRec=new Recorder::Record;
		aRec->mAppTime=AppTime();
		aRec->mMessage=theMessage;
		aRec->mID=theID;
		aRec->mData1=theData1;
		aRec->mData2=theData2;
		aRec->mData3=theData3;
		aRec->mDataPtr=NULL;
		gRecorder->mRecordList+=aRec;
	}
#endif
	//
	// Process system messages (mousemoves, etc)
	//
	switch (theMessage)
	{
	case OS_Core::OS_OBSCURESCREENBOTTOM:
		{
			mOnscreenKeyboardY=((float)theData1);

			mOnscreenKeyboardChanged=true;
			RehupVisibilityScroll();
			break;
		}
	case OS_Core::OS_REFRESHTOUCHES:
		{
			//
			// This refreshes touches if something happened that might require a re-evaluation of the touch
			// position.  The clearest example is, if you make a new CPU right under the touch position.
			//
			if (mCanRefreshTouches) Core_TouchMove(mMessageData_TouchPosition.mX,mMessageData_TouchPosition.mY);
			break;
		}
	case OS_Core::OS_DROPFILE:
		{
			char *aData=(char*)theDataPtr;
			DroppedFile(aData);
			_DeleteArray(aData);
			CatchUp();
			break;
		}
	case OS_Core::OS_TOUCHMOVE:
		{
			if (mFPSCursorCPU) {mFPSCursorCPU->TouchMove(theData1,theData2);break;}

			theData1-=(int)(gG.mLetterboxSize.mX/2);
			theData2-=(int)(gG.mLetterboxSize.mY/2);
			mCanRefreshTouches=false;
			mMessageData_TouchID=theID;
			mMessageData_LastTouchPosition=mMessageData_TouchPosition;
			mMessageData_TouchPosition=IPoint(theData1,theData2-(int)mVisibilityScroll);
			float aDist=gMath.Distance(mMessageData_TouchPosition,mMessageData_TouchStartPosition);
			mMessageData_MaxMove=_max(aDist,mMessageData_MaxMove);
			System_TouchMove(mMessageData_TouchPosition.mX,mMessageData_TouchPosition.mY);

			break;
		}
	case OS_Core::OS_TOUCHSTART:
		{
			theData1-=(int)(gG.mLetterboxSize.mX/2);
			theData2-=(int)(gG.mLetterboxSize.mY/2);

			mCanRefreshTouches=false;
			mMessageData_LastTouchTime=mMessageData_TouchTime;
			mMessageData_TouchTime=AppTime();

			mMessageData_TouchID=theID;
			mMessageData_MouseButton=theData3;
			mMessageData_MaxMove=0;

			if (theData3==-1) mMessageData_TouchStatus|=TOUCH_LEFT;
			if (theData3==1) mMessageData_TouchStatus|=TOUCH_RIGHT;
			if (theData3==0) mMessageData_TouchStatus|=TOUCH_MIDDLE;

			mMessageData_TouchPosition=mMessageData_TouchStartPosition=mMessageData_LastTouchPosition=IPoint(theData1,theData2-(int)mVisibilityScroll);

			//gOut.Out("Touch start ID: %d = %d,%d",theID,mMessageData_TouchPosition.mX,mMessageData_TouchPosition.mY);


			bool aAdded=false;
			for (int aCount=0;aCount<gCurrentTouchIDs.Size();aCount++) if (gCurrentTouchIDs[aCount]==-99999) 
			{
				gCurrentTouchIDs[aCount]=theID;
				aAdded=true;
				break;
			}
			if (!aAdded) gCurrentTouchIDs+=theID;

			//
			// Sending a Touchmove before a Touchstart helps alleviate some of the stress on the programmer.  Programming on Windows will end up sending TouchMoves
			// before each touchstart anyway, but a touch device will NOT-- which has caused me some grief in the pass trying to keep things synchronized.
			//
			if (IsTouchDevice()) System_TouchMove(mMessageData_TouchPosition.mX,mMessageData_TouchPosition.mY);
			System_TouchStart(mMessageData_TouchPosition.mX,mMessageData_TouchPosition.mY);
			break;
		}
	case OS_Core::OS_TOUCHEND:
		{
			if (gIgnoreNextTouchEnd)
			{
				gIgnoreNextTouchEnd=false;
				break;
			}

			theData1-=(int)(gG.mLetterboxSize.mX/2);
			theData2-=(int)(gG.mLetterboxSize.mY/2);

			mCanRefreshTouches=false;
			mMessageData_TouchID=theID;
			mMessageData_MouseButton=theData3;
			for (int aCount=0;aCount<gCurrentTouchIDs.Size();aCount++) if (gCurrentTouchIDs[aCount]==theID) gCurrentTouchIDs[aCount]=-99999;

			if (theData3==-1) {mMessageData_TouchStatus|=TOUCH_LEFT;mMessageData_TouchStatus^=TOUCH_LEFT;}
			if (theData3==1) {mMessageData_TouchStatus|=TOUCH_RIGHT;mMessageData_TouchStatus^=TOUCH_RIGHT;}
			if (theData3==0) {mMessageData_TouchStatus|=TOUCH_MIDDLE;mMessageData_TouchStatus^=TOUCH_MIDDLE;}

			mMessageData_LastTouchPosition=mMessageData_TouchPosition;
			mMessageData_TouchPosition=IPoint(theData1,theData2-(int)mVisibilityScroll);
			System_TouchEnd(mMessageData_TouchPosition.mX,mMessageData_TouchPosition.mY);

//			gOut.Out("Touch end ID: %d = %d,%d",theID,mMessageData_TouchPosition.mX,mMessageData_TouchPosition.mY);

			break;
		}
	case OS_Core::OS_SPIN:
		{
			System_Spin(theData1);
			break;
		}
	case OS_Core::OS_KEYDOWN:
		{
			System_KeyDown(theData1);
			break;
		}
	case OS_Core::OS_KEYUP:
		{
			System_KeyUp(theData1);
			break;
		}
	case OS_Core::OS_CHAR:
		{
			System_Char(theData1);
			break;
		}
	case OS_Core::OS_BACK:
		{
			System_Back();
			break;
		}
	case OS_Core::OS_MULTITASKING:
		{
			CatchUp();
			bool aWantBackground=(theData1==0);

			//
			// Handle multitasking for all system devices...
			//
			if (ShouldPauseSoundWhenInBackground()) gAudio.Multitasking(!aWantBackground);
			gG.Multitasking(!aWantBackground);
			gInput.Multitasking(!aWantBackground);

			if (mDisableMultitasking==0 && mInBackground!=aWantBackground) 
			{
				Core_Multitasking(!aWantBackground);

				//
				// For mobile, we save everything when we go away from the app... because it might get shut down.
				//
				#ifndef _WIN32
				if (aWantBackground) gApp_SaveEverything();
				#endif
			}
			if (mDisableMultitasking>0) mDisableMultitasking--;
			CatchUp();
			mInBackground=aWantBackground;
			if (mInBackground) mNeedBackgroundDraw=OS_Core::CanBackgroundDraw();
			//
			// iOS could possibly terminate us BEFORE we save... so iOS starts a background task.
			// Then this core query lets the core know that we finished it up.
			//
			OS_Core::Query("FINISH_MULTITASKING");
			break;
		}
	case OS_Core::OS_SYSTEMNOTIFY:
		{
			char *aStringPtr=(char*)theDataPtr;
			String aNotify=aStringPtr;

			gAudio.SystemNotify(aNotify);
			gInput.SystemNotify(aNotify);
			gG.SystemNotify(aNotify);

			Core_SystemNotify(aNotify);
			_DeleteArray(aStringPtr);
			break;
		}
	case OS_Core::OS_SCREENSHOT:
		{
		/*
			int aWidth,aHeight;
			RGBA* aPix=(RGBA*)Graphics_Core::ScreenshotToMemory(&aWidth,&aHeight);
			if (aPix)
			{
				Image aImage;
				aImage.mWidth=aWidth;
				aImage.mHeight=aHeight;
				aImage.mData=aPix;
				aImage.ExportPNG(PointAtDesktop("Screenshot.png"));
			}
			*/

			//
			// We now dump screenshots to desktop because Win32 isn't creating a compatible bitmap for
			// whatever my newest NVIDIA card is doing.
			//
			Graphics_Core::Screenshot();
			CatchUp();
			break;
		}
	case OS_Core::OS_SWAPFULLSCREEN:
		{
			static bool aIsSwapping=false;
			if (!aIsSwapping)
			{
				aIsSwapping=true;
				gG.SetResolution(gG.mResolutionWidth,gG.mResolutionHeight,!gG.mFullscreen);
				aIsSwapping=false;
			}
			CatchUp();
			break;
		}
	case OS_Core::OS_REBOOT:
		{
			Reboot();
			break;
		}
	case OS_Core::OS_MISC:
		{
			switch (theID) 
			{
				case 0:CatchUp();break;	// Misc 0 is something that took system time and needs a time slice rehup (i.e. windows moving the window around)
			}
			break;
		}

	}
}

void App::Quit()
{
	OS_Core::RequestShutdown();
}

inline bool App::IsQuit()
{
	if (!gAppPtr) return true;
	return (WantShutdown());
}

bool App::IsTouchDevice()
{
	return OS_Core::IsTouchDevice();
}

bool App::IsRetinaDevice()
{
	return Graphics_Core::IsRetinaDevice();
}

String App::GetOSName()
{
	return String(OS_Core::GetOSName());
}

String App::GetOSVersion()
{
	char aVersion[256];
	OS_Core::GetOSVersion(aVersion);
	return String(aVersion);
}


CPU* App::GetCPUAtPos(int x, int y)
{
	CPU *aCPU=NULL;
	if (mIsolatedCPUManager) aCPU=mIsolatedCPUManager->Pick((float)x,(float)y);
	else if (mCPUManager) aCPU=mCPUManager->Pick((float)x,(float)y);
	if (!aCPU) aCPU=this;
	return aCPU;
}

void App::System_TouchMove(int x, int y)
{
	CPU *aCPU=GetFocusTouches(GetTouchID());
	if (!aCPU) aCPU=GetCPUAtPos(x,y);

	SetHoverCPU(aCPU);
	Point aPos=aCPU->ConvertScreenToLocal(Point(x,y));
	aCPU->Core_TouchMove((int)aPos.mX,(int)aPos.mY);
}

void App::System_TouchStart(int x, int y)
{
	StartTimeLog();

	mMessageData_IsTouching++;
	mMessageData_TouchNumber++;
	CPU *aCPU=GetFocusTouches(GetTouchID());
	if (!aCPU) aCPU=GetCPUAtPos(x,y);

	//gOut.Out("Touching: %s",aCPU->mName.c());

	SetHoverCPU(aCPU);
	TouchedCPU(aCPU);

	Point aPos=aCPU->ConvertScreenToLocal(Point(x,y));
	aCPU->Core_TouchStart((int)aPos.mX,(int)aPos.mY);
}

void App::UniquifyTouchID(CPU* theCPU)
{
	EnumList(FocusTouchID,aFT,mFocusTouchesByIDList)
	{
		if (aFT->mCPU!=theCPU)
		{
			mFocusTouchesByIDList-=aFT;
			delete aFT;
			EnumListRewind(FocusTouchID);
		}
	}
}

void App::System_TouchEnd(int x, int y)
{
	mMessageData_IsTouching=_max(0,mMessageData_IsTouching-1);

	CPU *aCPU=GetFocusTouches(GetTouchID());
	if (!aCPU) aCPU=GetCPUAtPos(x,y);

	SetHoverCPU(aCPU);
	Point aPos=aCPU->ConvertScreenToLocal(Point(x,y));
	aCPU->Core_TouchEnd((int)aPos.mX,(int)aPos.mY);
}

void App::System_Spin(int theDir)
{
	CPU *aCPU=GetFocusSpins();
	if (!aCPU) aCPU=GetHoverCPU();
	if (aCPU) aCPU->Core_Spin(theDir);
}

void App::System_KeyDown(int theKey)
{
	CPU *aCPU=GetFocusKeyboard();
	if (aCPU) aCPU->Core_KeyDown(theKey);
}

void App::System_KeyUp(int theKey)
{
	CPU *aCPU=GetFocusKeyboard();
	if (aCPU) aCPU->Core_KeyUp(theKey);
}

void App::System_Char(int theKey)
{
	CPU *aCPU=GetFocusKeyboard();
	if (aCPU) aCPU->Core_Char(theKey);
}

void App::System_Back()
{
	CPU *aCPU;
	aCPU=GetFocusBackButton();

	CPU* aKBCPU=GetFocusKeyboard();			// If we have keyboard focus, and it's not the current "back" CPU, then don't process
	if (aKBCPU && aCPU!=aKBCPU) aCPU=NULL;  // this, because a keyboard focused CPU should handle everything.
	if (aCPU) 
	{
		aCPU->Back();
	}
}

void App::System_Accelerometer(int theX, int theY, int theZ)
{
	CPU *aCPU=GetFocusAccelerometer();
	if (aCPU) aCPU->Core_Accelerometer(theX,theY,theZ);
}

void App::FocusKeyboard(CPU *theCPU) 
{
	CPU *aOld=mFocusKeyboardCPU;
	mFocusKeyboardCPU=theCPU;

	if (aOld!=theCPU)
	{
		if (aOld) aOld->Core_LostKeyboardFocus();
		if (theCPU) theCPU->Core_GotKeyboardFocus();
	}

	//
	// (For touch devices)
	// Show the keyboard, since we're asking for input.
	// If our focus'd CPU is under the keyboard, we'll scroll up, too.
	//
	if (!theCPU) 
	{
		OS_Core::PopupKeyboard(false);
	}
	else
	{
		//mVisibilityY=_min(gG.mHeight,theCPU->ConvertLocalToScreen(Point(0.0f,theCPU->mHeight)).mY);
		mVisibilityY=_min(gG.mHeight,theCPU->ConvertLocalToScreen(Point(0.0f,theCPU->mHeight)).mY);
		mVisibilityY+=mVisibilityYExtraSpace;

		//gOut.Out("With  extra Space = %f",mVisibilityY);

		RehupVisibilityScroll();
		OS_Core::PopupKeyboard(true);
	}
}

void App::ThrottleShutdown()
{
	if (mIsolatedCPUManager)
	{
		//
		// We clear it because our CPU's are duplicated in CPUManager...
		// So we don't want them deleted when this CPUManager destroys.
		//
		mIsolatedCPUManager->Clear();
		delete mIsolatedCPUManager;
		mIsolatedCPUManager=NULL;
	}
/*
	//
	// Don't remove CPUManagers here...
	// We might want to fiddle with any remaining CPUs in App::Shutdown(),
	// so make sure they're available.
	//
	if (mCPUManager)
	{
		_Destroy(mCPUManager);
		mCPUManager=NULL;
	}
*/
	Shutdown();
}

void App::FocusTouchesByID(int theID, CPU *theCPU)
{
	FocusTouchID *aMyFT=NULL;
	EnumList(FocusTouchID,aFT,mFocusTouchesByIDList) if (aFT->mID==theID) {aMyFT=aFT;break;}

	if (!aMyFT)
	{
		aMyFT=new FocusTouchID;
		aMyFT->mID=theID;
		mFocusTouchesByIDList+=aMyFT;
	}
	aMyFT->mCPU=theCPU;
}

void App::UnfocusTouchesByID(int theID)
{
	EnumList(FocusTouchID,aFT,mFocusTouchesByIDList)
	{
		if (aFT->mID==theID)
		{
			mFocusTouchesByIDList-=aFT;
			delete aFT;
			EnumListRewind(FocusTouchID);
		}
	}
}

CPU* App::GetFocusTouches(int theID) 
{
	//
	// If we've manually focused touches, then go with
	// that one, no matter what!
	//
	if (mFocusTouchesCPU) return mFocusTouchesCPU;
	//
	// See if this ID is focused onto a CPU...
	// If it is, go with that CPU.
	//
	EnumList(FocusTouchID,aFT,mFocusTouchesByIDList) if (aFT->mID==theID) return aFT->mCPU;
	return NULL;
}

void App::SetHoverCPU(CPU *theCPU) 
{
	if (GetHoverCPU()!=theCPU) 
	{
		if (GetHoverCPU()) GetHoverCPU()->Core_Leave();
		mHoverCPU=theCPU;
		if (theCPU) 
		{
			theCPU->Core_Enter();
			OS_Core::SetCursor(theCPU->mCursor);
		}
		else OS_Core::SetCursor(0);
	}
}

void App::RefreshCursor()
{
	CPU *aCPU=GetHoverCPU();
	if (aCPU) OS_Core::SetCursor(aCPU->mCursor);
}

void App::UnfocusTouchesByCPU(CPU *theCPU)
{
	if (mFocusTouchesCPU==theCPU) FocusTouches(NULL);
	EnumList(FocusTouchID,aFT,mFocusTouchesByIDList) 
	{
		if (aFT->mCPU==theCPU)
		{
			mFocusTouchesByIDList-=aFT;
			delete aFT;
			EnumListRewind(FocusTouchID);
		}
	}
}

void App::IsolateCPU(CPU* theCPU, bool touchesOnly)
{
	if (!mIsolatedCPUManager) mIsolatedCPUManager=new CPUManager;
	*mIsolatedCPUManager+=theCPU;
	mIsolateTouchesOnly=touchesOnly;
}

void App::UnIsolateCPU(CPU* theCPU)
{
	if (!mIsolatedCPUManager) return;
	*mIsolatedCPUManager-=theCPU;
}

bool App::IsCPUTouchFocused(CPU *theCPU)
{
	if (mFocusTouchesCPU==theCPU) return true;
	EnumList(FocusTouchID,aFT,mFocusTouchesByIDList) if (aFT->mCPU==theCPU) return true;
	return false;

}


String ClickOrTouch(String theString, bool forceTouchDevice)
{
	String aString=theString;
	if (gAppPtr->IsTouchDevice() || forceTouchDevice)
	{
		aString.Replace("[cl]","touch");
		aString.Replace("[Cl]","Touch");
		aString.Replace("[CL]","TOUCH");
	}
	else
	{
		aString.Replace("[cl]","click");
		aString.Replace("[Cl]","Click");
		aString.Replace("[CL]","CLICK");
	}
	return aString;
}

String ClickOrTap(String theString, bool forceTouchDevice)
{
	String aString=theString;
	if (gAppPtr->IsTouchDevice() || forceTouchDevice)
	{
		aString.Replace("[cl]","tap");
		aString.Replace("[Cl]","Tap");
		aString.Replace("[CL]","TAP");
	}
	else
	{
		aString.Replace("[cl]","click");
		aString.Replace("[Cl]","Click");
		aString.Replace("[CL]","CLICK");
	}
	return aString;
}


String App::GetLoadFilename(String thePath, String theExtensions)
{
	return OS_Core::GetFilename(thePath.c(),theExtensions.c(),true);
}

String App::GetSaveFilename(String thePath, String theExtensions)
{
	return OS_Core::GetFilename(thePath.c(),theExtensions.c(),false);
}

void Sleep(int theAmount)
{
	OS_Core::Sleep(theAmount);
}


String App::GetAppID()
{
	char aResult[MAX_PATH];
	OS_Core::GetAppID(aResult);
	return aResult;
}

void App::SetCrashInfo(String theInfo)
{
	ThreadLock(mCrashInfoThreadlock);
	mCrashInfo=theInfo;
	CrashHandler::SetExtendedInfo(mCrashInfo.mData);
	ThreadUnlock(mCrashInfoThreadlock);
}

void App::Minimize()
{
	OS_Core::Minimize();
}

#ifdef _DEBUG
int gTimeLog=-1;
void StartTimeLog()
{
	gTimeLog=OS_Core::Tick();
}

void ShowTimeLog(String theString)
{
	int aNow=OS_Core::Tick()-gTimeLog;
	int aNowUpdates=TicksToUpdates(aNow);
	gOut.Out("*TIME: [%s] %d (Updates: %d)",theString.c(),aNow,aNowUpdates);
	gTimeLog=OS_Core::Tick();
}
#endif

bool App::IsSmallScreen()
{
	return gG.IsSmallScreen();
}

void App::SaveSettings()
{
	gAudio.Save();
	mSettings.Save();
}

void App::OverrideBackButton(bool theState)
{
	if (theState) OS_Core::Query("backbutton_override");
	else OS_Core::Query("backbutton_normal");
}

void App::RehupVisibilityScroll()
{
	// VisibilityY = Y we want to see
	// OnscreenKeyboard Y = level Keyboard comes up to
	if (mOnscreenKeyboardY==0) mVisibilityScroll=0;
	else mVisibilityScroll=_min(0,((gG.mHeight-mOnscreenKeyboardY)-mVisibilityY));

	/*
	gOut.Out("--------------------------------------");
	gOut.Out("Must See: %f",mVisibilityY);
	gOut.Out("Page Height: %d",gG.mHeight);
	gOut.Out("Keyboard Size: %f",mOnscreenKeyboardY);
	gOut.Out("Scroll it: %f",mVisibilityScroll);
	gOut.Out("--------------------------------------");
	*/

	//if (mOnscreenKeyboardY==0) mVisibilityScroll=0;
	//else mVisibilityScroll=_min(0,((mOnscreenKeyboardY)-mVisibilityY));

	
	/*
	if (mOnscreenKeyboardY==0) mVisibilityScroll=0;
	else mVisibilityScroll=-(mVisibilityY-(gG.mHeight-mOnscreenKeyboardY));
	gOut.Out("Want to see Y = %f",mVisibilityY);
	gOut.Out("Keyboard Height = %f",mOnscreenKeyboardY);
	gOut.Out("Keyboard screenpos = %f",(float)gG.mHeight-mOnscreenKeyboardY);
	gOut.Out("Diff: %f",mVisibilityScroll);
	/**/
}

void CallSoonObject::Update()
{
	Kill();
	if (gAppPtr && !gAppPtr->IsQuit() && mFunction) mFunction(mArg);
}

char* _TT(char* theText) 
{
	return theText;
}

float App::GetLoadComplete() 
{
	if (mLoadComplete) return 1.0f;
	if (mLoader) return mLoader->GetLoadComplete();
	return 0.0f;
}

String App::GetLoadStatus()
{
	if (mLoadComplete) return "Done!";
	if (mLoader) return mLoader->GetStatus();
	return "";
}

/*
float App::GetCloudLoadComplete() 
{
	if (mLoader) return mLoader->GetCloudLoadComplete();
	return 0.0f;
}
*/

#ifdef MULTICORE_LOAD
AssetLoader* gLoader=NULL;
struct AssetThreader
{
	Smart(AssetLoader::Asset) mAsset;
};

void AssetThread(void* theArg)
{
	AssetThreader* aA=(AssetThreader*)theArg;

	//
	// Wait for the loader asset tier to rise...
	//
	for (;;) {if (!gLoader) return;if (gLoader->mAssetStage>=aA->mAsset->mStage) break;Sleep(10);}
	gG.Threading();

	gOut.Debug("_Load Asset %s (%d) [Asset Stage: %d]",aA->mAsset->mDesc.c(),aA->mAsset->mStage,gLoader->mAssetStage);
	
	if (gLoader) 
	switch (aA->mAsset->mType)
	{
		case AssetLoader::ASSET_NULL:break;
		case AssetLoader::ASSET_SOUND:{Sound* aSound=(Sound*)aA->mAsset->mPointer;if (!gLoader->mReloading || !aSound->IsLoaded()) aSound->Load(aA->mAsset->mText);}break;
		case AssetLoader::ASSET_SOUNDSTREAM:{SoundStream* aSound=(SoundStream*)aA->mAsset->mPointer;if (!gLoader->mReloading || !aSound->IsLoaded()) aSound->Load(aA->mAsset->mText);}break;
		case AssetLoader::ASSET_SOUNDLOOP:{SoundLoop* aSound=(SoundLoop*)aA->mAsset->mPointer;if (!gLoader->mReloading || !aSound->IsLoaded()) aSound->Load(aA->mAsset->mText);}break;
		case AssetLoader::ASSET_SOUNDLOOPREF:{SoundLoop* aSound=(SoundLoop*)aA->mAsset->mPointer;if (!gLoader->mReloading || !aSound->IsLoaded()) aSound->Load((Sound*)aA->mAsset->mRefPointer);}break;
		case AssetLoader::ASSET_MUSIC:{Music* aMusic=(Music*)aA->mAsset->mPointer;if (!gLoader->mReloading || !aMusic->IsLoaded()) aMusic->Load(aA->mAsset->mText);}break;
		case AssetLoader::ASSET_SPRITEBUNDLE:{SpriteBundle* aBundle=(SpriteBundle*)aA->mAsset->mPointer;if (!gLoader->mReloading || !aBundle->IsLoaded()) aBundle->Load();}break;
		case AssetLoader::ASSET_SPRITE:{Sprite* aSprite=(Sprite*)aA->mAsset->mPointer;if (!gLoader->mReloading || aSprite->mTexture==-1) aSprite->Load(aA->mAsset->mText);}break;
		case AssetLoader::ASSET_IMAGE:{Image* aImage=(Image*)aA->mAsset->mPointer;if (!gLoader->mReloading || aImage->GetBits()==NULL) aImage->Load(aA->mAsset->mText);}break;
		case AssetLoader::ASSET_TEXTURE:{int* aTexInt=(int*)aA->mAsset->mPointer;if (!gLoader->mReloading || *aTexInt==-1) *aTexInt=gG.LoadTexture(aA->mAsset->mText);}break;
		case AssetLoader::ASSET_RQUERY:{if (aA->mAsset->mQuery.IsNotNull()) while (!aA->mAsset->mQuery->IsDone()) {}break;}
		case AssetLoader::ASSET_CUSTOM:
			{
				//
				// The option could exist to run a custom threaded...
				// If it ever comes up, you can put it in this line...
				//
				if (aA->mAsset->mFlag&ASSETLOADER_FORCETHREADED) {gOut.Out("_   Custom threaded...");if (gAppPtr) gAppPtr->CustomLoad(aA->mAsset->mText);}
				else if (gLoader)
				{
					//
					// Wait until we're open...
					//
					bool aWatch=true;
					while (aWatch)
					{
						gLoader->Lock();
						if (gLoader->mRunMe.IsNull()) {gLoader->mRunMe=aA->mAsset;break;}
						gLoader->Unlock();
						Sleep(25);
					}
					gLoader->Unlock();
					aWatch=true;
					while (aWatch)
					{
						Sleep(25);
						gLoader->Lock();
						if (gLoader->mRunMe!=aA->mAsset) aWatch=false;
						gLoader->Unlock();
					}
				}
			}
			break;
			//if (!gLoader->mReloading) gAppPtr->CustomLoad(aA->mAsset->mText);break;
	}

	if (gLoader)
	{
		gLoader->Lock();
		//gLoader->mAssetList-=aA->mAsset;
		aA->mAsset->mFlag|=ASSETLOADER_KILL;
		gLoader->mThreadCount--;
		gLoader->Unlock();
	}
	delete aA;
}

void AssetLoader::Initialize() 
{
	HookTo(&gLoader);

	AddNull()->SetDesc("Let's go!")->SetStage(INT_MAX);

	EnumSmartList(Asset,aA,mAssetList)
	{
		AssetThreader* aAT=new AssetThreader;
		aAT->mAsset=aA;
		Thread(AssetThread,aAT);
		mThreadCount++;
		/*
		//if (aA->mStage<255 || (aA->mType!=ASSET_CUSTOM && aA->mType!=ASSET_NULL))
		if (aA->mStage<255) // Stage 255 runs sequentially at the end.  Everything else is threaded.
		{
		AssetThreader* aAT=new AssetThreader;
		aAT->mAsset=aA;
		Thread(AssetThread,aAT);
		mThreadCount++;
		}
		*/
	}

	mLoadMax=(float)mAssetList.GetCount();

}
void AssetLoader::Update()
{
	if (!gAppPtr || gAppPtr->IsQuit() || gAppPtr->mLoadComplete) return;

	//
	// Run customs on the main thread always... we'll avoid problems that way.
	//
	Lock();
	if (mRunMe.IsNotNull())	{gAppPtr->CustomLoad(mRunMe->mText);mRunMe=NULL;}
	Unlock();

	Lock();
	int aAssetCount=mAssetList.GetCount();
	if (mThreadCount!=mOldThreadCount)
	{
		EnumSmartList(Asset,aA,mAssetList) {if (aA->mFlag&ASSETLOADER_KILL) {mAssetList-=aA;EnumSmartListRewind(Asset);}}
		mAssetStage=-1;EnumSmartList(Asset,aA,mAssetList) mAssetStage=_min(mAssetStage,aA->mStage);

		mOldThreadCount=mThreadCount;
		if (mThreadCount==0)
		{
			//
			// Done loading... finish up with the customs and the nulls...
			// Depreciated... we staged customs so they'll run in the thread...
			//
			/*
			EnumSmartList(Asset,aA,mAssetList) {if (aA->mType!=ASSET_CUSTOM) {mAssetList-=aA;EnumSmartListRewind(Asset);}}
			if (!mReloading) while (mAssetList.GetCount())
			{
				gAppPtr->PreRender();
				gAppPtr->Render();
				gAppPtr->PostRender();
				gG.Show();

				gAppPtr->CustomLoad(mAssetList[0]->mText);
				mAssetList.Remove(0);
			}
			*/
			aAssetCount=0;
		}
	}
	Unlock();
	if (aAssetCount<=0)
	{
		gAppPtr->mLoadComplete=true;
		gAppPtr->mLoader=NULL;
		gAppPtr->DrawLoadingScreen(App::LOADINGSCREEN_UNLOADGRAPHICS);
		Kill();

		if (gAppPtr->HasCloud()) CloudQueue::Enable(); // Enable clouding (when appropriate)
		gAppPtr->LoadComplete();
		Thread(&LoadingBackgroundThread,gAppPtr);
		gAppPtr->RecoverTime();

		return;
	}

}

void AssetLoader::Draw() 
{
	//
	// Did you trace a lockup to here?  Remember to PACK ANDROID in RWizard!  I had a hang because data was out of date!
	//
	gAppPtr->DrawLoadingScreen(mFlag);
	mFlag=0;

	/*
	
	//
	// This causes flickering on Android, whenever it doesn't draw...
	//
	float aLC=GetLoadComplete();
	if (aLC!=mOldLoadComplete)
	{
		mOldLoadComplete=aLC;

		// Only if changed...
		gAppPtr->DrawLoadingScreen(mFlag);
		mFlag=0;
	}
	*/
}

#else
void AssetLoader::Initialize() 
{
	for (int aCount=0;aCount<15;aCount++) AddNull()->SetDesc("Finishing up..."); // Just some padding nulls to make the load bar look nicer...
	AddNull()->SetDesc("Let's go!")->SetStage(INT_MAX);
	mLoadMax=(float)mAssetList.GetCount();

	bool aChanged=true;
	while (aChanged)
	{
		aChanged=false;
		for (int aCount=1;aCount<mAssetList.GetCount();aCount++) {if (mAssetList[aCount]->mStage<mAssetList[aCount-1]->mStage) {mAssetList.Swap(aCount,aCount-1);aChanged=true;}}
	}
}

void AssetLoader::Update()
{
	if (!gAppPtr || gAppPtr->IsQuit() || gAppPtr->mLoadComplete) return;
	/*
	if (mLoadMax==-1) 
	{
		for (int aCount=0;aCount<15;aCount++) AddNull()->SetDesc("Finishing up..."); // Just some padding nulls to make the load bar look nicer...
		AddNull()->SetDesc("Let's go!")->SetStage(255);
		mLoadMax=(float)mAssetList.GetCount();
	}
	*/

	if (mAssetList.GetCount()<=0)
	{
		gAppPtr->mLoadComplete=true;
		gAppPtr->mLoader=NULL;
		gAppPtr->DrawLoadingScreen(App::LOADINGSCREEN_UNLOADGRAPHICS);
		Kill();

		gApp_PreClean();	// Clean off any pre-folders that might have junk in them...
		gAppPtr->LoadComplete();
#ifndef NO_THREADS
		Thread(&LoadingBackgroundThread,gAppPtr);
#else
		LoadingBackgroundThread(gAppPtr);
#endif
		gAppPtr->RecoverTime();

		return;
	}


	//
	// Forces a draw so we can see status updates...
	//
	gAppPtr->PreRender();
	gAppPtr->Render();
	gAppPtr->PostRender();
	gG.Show();

	switch (mAssetList[0]->mType)
	{
	case ASSET_NULL:break;
	case ASSET_CUSTOM:if (!mReloading) gAppPtr->CustomLoad(mAssetList[0]->mText);break;
	case ASSET_SOUND:{Sound* aSound=(Sound*)mAssetList[0]->mPointer;if (!mReloading || !aSound->IsLoaded()) aSound->Load(mAssetList[0]->mText);}break;
	case ASSET_SOUNDSTREAM:{SoundStream* aSound=(SoundStream*)mAssetList[0]->mPointer;if (!mReloading || !aSound->IsLoaded()) aSound->Load(mAssetList[0]->mText);}break;
	case ASSET_SOUNDLOOP:{SoundLoop* aSound=(SoundLoop*)mAssetList[0]->mPointer;if (!mReloading || !aSound->IsLoaded()) aSound->Load(mAssetList[0]->mText);}break;
	case ASSET_SOUNDLOOPREF:{SoundLoop* aSound=(SoundLoop*)mAssetList[0]->mPointer;if (!mReloading || !aSound->IsLoaded()) aSound->Load((Sound*)mAssetList[0]->mRefPointer);}break;
	case ASSET_MUSIC:{Music* aMusic=(Music*)mAssetList[0]->mPointer;if (!mReloading || !aMusic->IsLoaded()) aMusic->Load(mAssetList[0]->mText);}break;
	case ASSET_SPRITEBUNDLE:{SpriteBundle* aBundle=(SpriteBundle*)mAssetList[0]->mPointer;if (!mReloading || !aBundle->IsLoaded()) aBundle->Load();}break;
	case ASSET_SPRITE:{Sprite* aSprite=(Sprite*)mAssetList[0]->mPointer;if (!mReloading || aSprite->mTexture==-1) aSprite->Load(mAssetList[0]->mText);}break;
	case ASSET_IMAGE:{Image* aImage=(Image*)mAssetList[0]->mPointer;if (!mReloading || aImage->GetBits()==NULL) aImage->Load(mAssetList[0]->mText);}break;
	case ASSET_TEXTURE:{int* aTexInt=(int*)mAssetList[0]->mPointer;if (!mReloading || *aTexInt==-1) *aTexInt=gG.LoadTexture(mAssetList[0]->mText);}break;
	case ASSET_RQUERY:{if (mAssetList[0]->mQuery.IsNotNull()) if (!mAssetList[0]->mQuery->IsDone()) return;}break;
		//case ASSET_CLOUDDOUBLECHECK:{if (!gSocial.IsConnected()) {gSocial.Go();gAppPtr->mCloudLoadPending=true;Thread(&App::PullFromCloud);}}break;
	//case ASSET_CLOUD:{mCloudLoadStarted=true;gAppPtr->RecoverTime();if (gAppPtr->mCloudLoadPending) return;}break;
	}

	if (mAssetList[0]->mQuery.IsNotNull()) mRQueryList-=mAssetList[0]->mQuery;
	mAssetList.Remove(0);
	gAppPtr->RecoverTime();
}

void AssetLoader::Draw() 
{
	//gOut.Out("Huh: %d / %d  [%f == %f]",mCloudLoadCounter,mCloudLoadMax,(float)mCloudLoadCounter/(float)mCloudLoadMax,GetCloudLoadComplete());
	gAppPtr->DrawLoadingScreen(mFlag);
	mFlag=0;
}
#endif

float AssetLoader::GetLoadComplete() 
{
	Lock();
	float aResult=1.0f-_clamp(0,(float)_max(0,(mAssetList.GetCount()-10))/mLoadMax,1);
	if (mRQueryList.GetCount()) {EnumSmartList(RComm::RaptisoftQuery,aRQ,mRQueryList) aResult=_min(aResult,aRQ->mProgress);}
	Unlock();
	return aResult;
}

String AssetLoader::GetStatus()
{
	#ifndef MULTICORE_LOAD
	if (mAssetList.GetCount()) return mAssetList[0]->mDesc;
	#endif
	return "";
}



#ifdef _DEBUG
bool gRandomizeDailySeed=false;
void RandomizeDailySeed() {gRandomizeDailySeed=true;}
#endif
int App::DailySeed(int addDays) 
{
	#ifdef _DEBUG
	if (gRandomizeDailySeed) return gRand.Get(999999999);
	#endif
	int aHour;int aMinute;int aSecond;int aMon;int aDay;int aYear;OS_Core::GetTime(addDays*24*60*60,&aHour,&aMinute,&aSecond,&aMon,&aDay,&aYear);

	if (mDailySeedEncoding==0) return Sprintf("%.4d%.2d%.2d",aYear,aMon,aDay).ToInt();
	else return Sprintf("%.4d%.2d%.2d",aYear,aDay,aMon).ToInt();
	//
	// Note to self:
	//		I made the seed useless as a counter because I put day before month, meaning you can't delay on Seed(tomorrow)>Seed(today)...
	//		However, I believe I did this for a reason... it might be connected to some kind of monthly thing.  Anyway, Chuzzle Snap needs the
	// 	    seed in the original format (to not invalidate the score lists) so now it branches here.
	//
}
int App::MonthlySeed(int addDays) 
{
	#ifdef _DEBUG
	if (gRandomizeDailySeed) return gRand.Get(999999999);
	#endif
	int aHour;int aMinute;int aSecond;int aMon;int aDay;int aYear;OS_Core::GetTime(addDays*24*60*60,&aHour,&aMinute,&aSecond,&aMon,&aDay,&aYear);return Sprintf("%.4d%.2d",aYear,aMon).ToInt();
}

int App::NextWeekdaySeed(int theWeekday, bool allowTodayAsNext)
{
	int aAddDays=0;
	//allowTodayAsNext=false;
	if (!allowTodayAsNext) aAddDays=1;

	while (aAddDays<14) // We only need to count to 7 but I believe in overkill
	{
		int aWDay=GetNowWeekday(aAddDays);
		if (aWDay==theWeekday) return DailySeed(aAddDays);
		aAddDays++;
	}
	return 0;
}


void App::DailySeedToMDY(int theSeed, int& theMonth, int& theDay, int& theYear)
{
	String aSeed=Sprintf("%d",theSeed);

	/*
	gOut.Out("!Seed: %s",aSeed.c());
	gOut.Out("! Year: %s",aSeed.GetSegment(0,4).c());
	gOut.Out("! Day: %s",aSeed.GetSegment(4,2).c());
	gOut.Out("! Mon: %s",aSeed.GetSegment(6,2).c());
	/**/

#ifdef SEEDYMD
	theYear=aSeed.GetSegment(0,4).ToInt();
	theMonth=aSeed.GetSegment(4,2).ToInt();
	theDay=aSeed.GetSegment(6,2).ToInt();
#else
	theYear=aSeed.GetSegment(0,4).ToInt();
	theDay=aSeed.GetSegment(4,2).ToInt();
	theMonth=aSeed.GetSegment(6,2).ToInt();
#endif
}

int App::DailySeedToInt(int theSeed)
{
	int aM,aD,aY;
	DailySeedToMDY(theSeed,aM,aD,aY);
	return DateToInt(aM,aD,aY);
}


struct WordFilter
{
	Dictionary mBadWords;
	Dictionary mBadWholeWords;
	Dictionary mGoodWords;
};
WordFilter* gWordFilter=NULL;

bool gWasProfanity=false;
void LoadFilter()
{
	if (!gWordFilter) gWordFilter=new WordFilter;
	gWordFilter->mBadWords.Load("data://filter/badwords.txt");
	gWordFilter->mBadWholeWords.Load("data://filter/badwholewords.txt");
	gWordFilter->mGoodWords.Load("data://filter/goodwords.txt");
}

bool WasProfanity() {return gWasProfanity;}
String ProfanityFilter(String theText, String replaceWith, bool replaceWholeWord)
{
	if (!gWordFilter) LoadFilter();
	//if (!gBadWords.IsLoaded()) LoadFilter();

	gWasProfanity=false;

	{
		int aPos=0;
		while (aPos<theText.Len())
		{
			int aStart,aLen;
			if (theText.GetWordStartingAtPosition(aPos,aStart,aLen))
			{
				//String aWord=theText.GetSegment(aStart,aLen);
				//if (int aBadWordLen=gBadWholeWords.Exists(aWord))
				if (int aBadWordLen=gWordFilter->mBadWholeWords.Exists(theText.c()+aStart,aLen))
				{
					gWasProfanity=true;
					if (replaceWholeWord) theText.Delete(aStart,aLen);
					else theText.Delete(aStart,aBadWordLen);
					theText.Insert(replaceWith,aStart);
					aLen=replaceWith.Len();
				}
				aPos+=aLen+1;
			}
			else aPos++;
		}
	}

	{
		int aPos=0;
		while (aPos<theText.Len())
		{
			if (int aBadWordLen=gWordFilter->mBadWords.Exists(theText.c()+aPos))
			{
				int aStart,aLen;
				if (theText.GetWordAtPosition(aPos,aStart,aLen))
				{
					//String aWord=theText.GetSegment(aStart,aLen);
					//if (!gGoodWords.Exists(aWord))
					if (!gWordFilter->mGoodWords.Exists(theText.c()+aStart,aLen))
					{
						gWasProfanity=true;
						if (replaceWholeWord) theText.Delete(aStart,aLen);
						else 
						{
							theText.GetWordStartingAtPosition(aPos,aStart,aLen);
							theText.Delete(aStart,aBadWordLen);
						}
						theText.Insert(replaceWith,aStart);
					}
				}
				aPos+=aBadWordLen;
			}
			aPos++;
		}
	}
	return theText;
}

bool IsProfanity(String theText)
{
	if (!gWordFilter) LoadFilter();

	gWasProfanity=false;
	if (!gWordFilter->mBadWords.IsLoaded()) LoadFilter();

	{
		int aPos=0;
		while (aPos<theText.Len())
		{
			int aStart,aLen;
			if (theText.GetWordStartingAtPosition(aPos,aStart,aLen))
			{
				if (gWordFilter->mBadWholeWords.Exists(theText.c()+aStart,aLen))
				{
					gWasProfanity=true;
					return true;
				}
				aPos+=aLen+1;
			}
			else aPos++;
		}
	}

	{
		int aPos=0;
		while (aPos<theText.Len())
		{
			if (int aBadWordLen=gWordFilter->mBadWords.Exists(theText.c()+aPos))
			{
				int aStart,aLen;
				if (theText.GetWordStartingAtPosition(aPos,aStart,aLen))
				{
					String aWord=theText.GetSegment(aStart,aLen);
					if (!gWordFilter->mGoodWords.Exists(aWord))
					{
						gWasProfanity=true;
						return true;
					}
				}
				aPos+=aBadWordLen;
			}
			aPos++;
		}
	}
	return false;
}

int GetDaysInMonth(int theMonth, int theYear, bool ignoreLeapYears=false)
{
	if(theMonth==2)	{if(!ignoreLeapYears && ((theYear%400==0) || (theYear%4==0 && theYear%100!=0))) return 29;else return 28;}
	else if(theMonth == 1 || theMonth == 3 || theMonth == 5 || theMonth == 7 || theMonth == 8 || theMonth == 10 || theMonth==12) return 31;
	return 30;
} 

int CountLeapYears(int theMonth, int theDay, int theYear) 
{ 
	int aYears=theYear; 

	if (theMonth<=2) aYears--; 
	return aYears/4-aYears/100+aYears/400; 
} 

int DateToInt(int theMonth, int theDay, int theYear)
{
	if (theMonth==-1 || theDay==-1 || theYear==-1)
	{
		int aHour;int aMinute;int aSecond;int aMon;int aDay;int aYear;OS_Core::GetTime(&aHour,&aMinute,&aSecond,&aMon,&aDay,&aYear);
		theMonth=aMon;theDay=aDay;theYear=aYear;
	}
	int aValue=theYear*365+theDay; 
	for (int i=0; i<theMonth-1; i++) aValue+=GetDaysInMonth(i,theYear,true);
	aValue+=CountLeapYears(theMonth,theDay,theYear);
	return aValue;
}

bool IsWithinDaysBefore(int theMon, int theDay, int theYear, int theDaysBefore)
{
	int aHour;int aMinute;int aSecond;int aMon;int aDay;int aYear;
	OS_Core::GetTime(&aHour,&aMinute,&aSecond,&aMon,&aDay,&aYear);

	int aStartMon=theMon;
	int aStartDay=theDay;
	int aStartYear=theYear;
	aStartDay-=theDaysBefore;
	if (aStartDay<=0)
	{
		aStartMon--;
		if (aStartMon<=0) {aStartMon+=12;aStartYear--;}
		aStartDay+=GetDaysInMonth(aStartMon,aStartYear);
	}


	/*
	gOut.Out("Start: %d-%d-%d   End: %d-%d-%d  Now: %d-%d-%d",
		aStartMon,aStartDay,aStartYear,theMon,theDay,theYear,aMon,aDay,aYear);
	gOut.Out("Check1: %d>=%d && %d>=%d && %d>=%d",aYear,aStartYear,aMon,aStartMon,aDay,aStartDay);
	gOut.Out("Check2: %d<=%d && %d<=%d && %d<=%d",aYear,theYear,aMon,theMon,aDay,theDay);
	*/

	if (aYear>=aStartYear && (aMon>aStartMon || (aMon==aStartMon && aDay>=aStartDay))) if (aYear<=theYear && (aMon<theMon || (aMon==theMon && aDay<=theDay))) return true;
	return false;
}

bool IsToday(int theMon, int theDay, int theYear)
{
	int aHour;int aMinute;int aSecond;int aMon;int aDay;int aYear;
	OS_Core::GetTime(&aHour,&aMinute,&aSecond,&aMon,&aDay,&aYear);
	return (theMon==aMon && theDay==aDay && theYear==aYear);
}

#ifdef _DEBUG
String gFakeSeason="";
void FakeSeason(String theSeason) {gFakeSeason=theSeason;}
#endif

// Gregorian Calculation
void CalculateEaster(int year, int &month, int &day) 
{
	int a = year % 19;
	int b = year / 100;
	int c = year % 100;
	int d = b / 4;
	int e = b % 4;
	int f = (b + 8) / 25;
	int g = (b - f + 1) / 3;
	int h = (19 * a + b - d - g + 15) % 30;
	int i = c / 4;
	int k = c % 4;
	int l = (32 + 2 * e + 2 * i - h - k) % 7;
	int m = (a + 11 * h + 22 * l) / 451;

	month = (h + l - 7 * m + 114) / 31;
	day = ((h + l - 7 * m + 114) % 31) + 1;
}

bool IsSeason(String theSeason) {return (GetSeason().FindI(theSeason)>=0);}
String GetSeason()
{
	String aSeason;

#ifdef _DEBUG
	if (gFakeSeason.Len()) aSeason.AppendToken(gFakeSeason);
#endif


	int aHour;int aMinute;int aSecond;int aMon;int aDay;int aYear;
	OS_Core::GetTime(&aHour,&aMinute,&aSecond,&aMon,&aDay,&aYear);
	if ((aMon>=12) || (aMon<=1 && aDay<=6)) aSeason.AppendToken("Christmas");
	if ((aMon==2) && (aDay>=7 && aDay<=14)) aSeason.AppendToken("Valentine");
	if ((aMon==9 && aDay>=10) || (aMon==10 && aDay<=10)) aSeason.AppendToken("Autumn");
	if ((aMon==10) && (aDay>=15)) aSeason.AppendToken("Halloween");
	if ((aMon==3) && (aDay>=14 && aDay<=17)) aSeason.AppendToken("St. Patrick's Day");
	if ((aMon==11) && (aDay>=18)) aSeason.AppendToken("Thanksgiving");

	//Easter
	bool aEaster=false;
	int aEMon,aEDay;CalculateEaster(aYear,aEMon,aEDay);
	if (IsWithinDaysBefore(aEMon,aEDay,aYear,14)) aEaster=true;
	/*
	switch (aYear)
	{
	case 2020:if (IsWithinDaysBefore(4,12+14,2020,14+14)) aEaster=true;break;
	case 2021:if (IsWithinDaysBefore(4,4,2021,14)) aEaster=true;break;
	case 2022:if (IsWithinDaysBefore(4,17,2022,14)) aEaster=true;break;
	case 2023:if (IsWithinDaysBefore(4,9,2023,14)) aEaster=true;break;
	case 2024:if (IsWithinDaysBefore(3,31,2024,14)) aEaster=true;break;
	case 2025:if (IsWithinDaysBefore(4,20,2025,14)) aEaster=true;break;
	case 2026:if (IsWithinDaysBefore(4,5,2026,14)) aEaster=true;break;
	case 2027:if (IsWithinDaysBefore(3,28,2027,14)) aEaster=true;break;
	case 2028:if (IsWithinDaysBefore(4,6,2028,14)) aEaster=true;break;
	case 2029:if (IsWithinDaysBefore(4,1,2029,14)) aEaster=true;break;
	case 2030:if (IsWithinDaysBefore(4,21,2030,14)) aEaster=true;break;
	case 2031:if (IsWithinDaysBefore(4,13,2031,14)) aEaster=true;break;
	case 2032:if (IsWithinDaysBefore(3,28,2032,14)) aEaster=true;break;
	case 2033:if (IsWithinDaysBefore(4,17,2033,14)) aEaster=true;break;
	case 2034:if (IsWithinDaysBefore(4,9,2034,14)) aEaster=true;break;
	case 2035:if (IsWithinDaysBefore(3,25,2035,14)) aEaster=true;break;
	case 2036:if (IsWithinDaysBefore(4,13,2036,14)) aEaster=true;break;
	case 2037:if (IsWithinDaysBefore(4,5,2037,14)) aEaster=true;break;
	case 2038:if (IsWithinDaysBefore(4,25,2038,14)) aEaster=true;break;
	case 2039:if (IsWithinDaysBefore(4,10,2039,14)) aEaster=true;break;
	case 2040:if (IsWithinDaysBefore(4,1,2040,14)) aEaster=true;break;
	}
	*/
	if (aEaster) aSeason.AppendToken("Easter");

	bool aMothers=false;
	switch (aYear)
	{
	case 2023:if (IsWithinDaysBefore(5,14,2023,30)) aMothers=true;break; // So that we can test the beta, this just allows the day before to be a long time.
	case 2024:if (IsWithinDaysBefore(5,12,2024,7)) aMothers=true;break;
	case 2025:if (IsWithinDaysBefore(5,11,2025,7)) aMothers=true;break;
	case 2026:if (IsWithinDaysBefore(5,10,2026,7)) aMothers=true;break;
	case 2027:if (IsWithinDaysBefore(5,9,2027,7)) aMothers=true;break;
	case 2028:if (IsWithinDaysBefore(5,14,2028,7)) aMothers=true;break;
	case 2029:if (IsWithinDaysBefore(5,13,2029,7)) aMothers=true;break;
	case 2030:if (IsWithinDaysBefore(5,12,2030,7)) aMothers=true;break;
	case 2031:if (IsWithinDaysBefore(5,11,2031,7)) aMothers=true;break;
	case 2032:if (IsWithinDaysBefore(5,9,2032,7)) aMothers=true;break;
	case 2033:if (IsWithinDaysBefore(5,8,2033,7)) aMothers=true;break;
	case 2034:if (IsWithinDaysBefore(5,14,2034,7)) aMothers=true;break;
	case 2035:if (IsWithinDaysBefore(5,13,2035,7)) aMothers=true;break;
	case 2036:if (IsWithinDaysBefore(5,11,2036,7)) aMothers=true;break;
	case 2037:if (IsWithinDaysBefore(5,10,2037,7)) aMothers=true;break;
	case 2038:if (IsWithinDaysBefore(5,9,2038,7)) aMothers=true;break;
	case 2039:if (IsWithinDaysBefore(5,8,2039,7)) aMothers=true;break;
	case 2040:if (IsWithinDaysBefore(5,13,2040,7)) aMothers=true;break;
	}
	if (aMothers) aSeason.AppendToken("Mother's Day");

	return aSeason;
}

bool IsHoliday(String theHoliday)
{
	int aHour;int aMinute;int aSecond;int aMon;int aDay;int aYear;
	OS_Core::GetTime(&aHour,&aMinute,&aSecond,&aMon,&aDay,&aYear);

	if (theHoliday=="christmas") {if (aMon==12 && aDay==25) return true;return false;}
	if (theHoliday=="halloween") {if (aMon==10 && aDay==31) return true;return false;}
	if (theHoliday.ContainsI("valentine")) {if (aMon==2 && aDay==14) return true;return false;}
	if (theHoliday.ContainsI("new year")) {if (aMon==1 && aDay==1) return true;return false;}
	if (theHoliday.ContainsI("patrick")) {if (aMon==3 && aDay==17) return true;return false;}
	if (theHoliday=="easter")
	{
		int aEMon,aEDay;CalculateEaster(aYear,aEMon,aEDay);
		if (IsToday(aEMon,aEDay,aYear)) return true;

		/*
		switch (aYear)
		{
		case 2023:if (IsToday(4,9,2023)) return true;break;
		case 2024:if (IsToday(3,31,2024)) return true;break;
		case 2025:if (IsToday(4,20,2025)) return true;break;
		case 2026:if (IsToday(4,5,2026)) return true;break;
		case 2027:if (IsToday(3,28,2027)) return true;break;
		case 2028:if (IsToday(4,6,2028)) return true;break;
		case 2029:if (IsToday(4,1,2029)) return true;break;
		case 2030:if (IsToday(4,21,2030)) return true;break;
		case 2031:if (IsToday(4,13,2031)) return true;break;
		case 2032:if (IsToday(3,28,2032)) return true;break;
		case 2033:if (IsToday(4,17,2033)) return true;break;
		case 2034:if (IsToday(4,9,2034)) return true;break;
		case 2035:if (IsToday(3,25,2035)) return true;break;
		case 2036:if (IsToday(4,13,2036)) return true;break;
		case 2037:if (IsToday(4,5,2037)) return true;break;
		case 2038:if (IsToday(4,25,2038)) return true;break;
		case 2039:if (IsToday(4,10,2039)) return true;break;
		case 2040:if (IsToday(4,1,2040)) return true;break;
		}
		*/
	}
	if (theHoliday.ContainsI("mother"))
	{
		switch (aYear)
		{
		case 2023:if (IsToday(5,14,2023)) return true;break;
		case 2024:if (IsToday(5,12,2024)) return true;break;
		case 2025:if (IsToday(5,11,2025)) return true;break;
		case 2026:if (IsToday(5,10,2026)) return true;break;
		case 2027:if (IsToday(5,9,2027)) return true;break;
		case 2028:if (IsToday(5,14,2028)) return true;break;
		case 2029:if (IsToday(5,13,2029)) return true;break;
		case 2030:if (IsToday(5,12,2030)) return true;break;
		case 2031:if (IsToday(5,11,2031)) return true;break;
		case 2032:if (IsToday(5,9,2032)) return true;break;
		case 2033:if (IsToday(5,8,2033)) return true;break;
		case 2034:if (IsToday(5,14,2034)) return true;break;
		case 2035:if (IsToday(5,13,2035)) return true;break;
		case 2036:if (IsToday(5,11,2036)) return true;break;
		case 2037:if (IsToday(5,10,2037)) return true;break;
		case 2038:if (IsToday(5,9,2038)) return true;break;
		case 2039:if (IsToday(5,8,2039)) return true;break;
		case 2040:if (IsToday(5,13,2040)) return true;break;
		}

	}

	return false;
}

int App::GetNowMonth(int thePlusDay)
{
	int aHour;int aMinute;int aSecond;int aMon;int aDay;int aYear;
	OS_Core::GetTime(thePlusDay*24*60*60,&aHour,&aMinute,&aSecond,&aMon,&aDay,&aYear);
	return aMon;
}

int App::GetNowDay(int thePlusDay)
{
	int aHour;int aMinute;int aSecond;int aMon;int aDay;int aYear;
	OS_Core::GetTime(thePlusDay*24*60*60,&aHour,&aMinute,&aSecond,&aMon,&aDay,&aYear);
	return aDay;
}

int App::GetNowYear(int thePlusDay)
{
	int aHour;int aMinute;int aSecond;int aMon;int aDay;int aYear;
	OS_Core::GetTime(thePlusDay*24*60*60,&aHour,&aMinute,&aSecond,&aMon,&aDay,&aYear);
	return aYear;
}

int App::GetNowHour(int thePlusDay)
{
	int aHour;int aMinute;int aSecond;int aMon;int aDay;int aYear;
	OS_Core::GetTime(thePlusDay*24*60*60,&aHour,&aMinute,&aSecond,&aMon,&aDay,&aYear);
	return aHour;
}

int App::GetNowMinute(int thePlusDay)
{
	int aHour;int aMinute;int aSecond;int aMon;int aDay;int aYear;
	OS_Core::GetTime(thePlusDay*24*60*60,&aHour,&aMinute,&aSecond,&aMon,&aDay,&aYear);
	return aMinute;
}

int App::GetNowWeekday(int thePlusDay)
{
	int aHour;int aMinute;int aSecond;int aMon;int aDay;int aYear;int aWeekday;
	OS_Core::GetTime(thePlusDay*24*60*60,&aHour,&aMinute,&aSecond,&aMon,&aDay,&aYear,&aWeekday);
	return aWeekday;
}


void SecureLog(char* theText, ...)
{
	String aString;

	va_list argp;
	va_start(argp, theText);
	aString.Format(theText,argp);
	va_end(argp);

	OS_Core::SecureLog("sandbox://securelog.txt",aString.c());
}

#ifdef LEGACY_GL
#ifdef _ANDROID
String RestoreLegacyFiles(bool evenIfMoved)
{
	String aLegacy=OS_Core::QueryString("core:getlegacysandbox");
	if (aLegacy.Len())
	{
		aLegacy+="/";

		char aSbox[MAX_PATH];OS_Core::GetSandboxFolder(aSbox);
		String aSandbox=aSbox;
		aSandbox.DeleteI("sandbox/");

		if (aLegacy!=aSandbox)
		{
			if (evenIfMoved)
			{
				//
				// Remake our old folders...
				//
				String aCloud=aLegacy;aCloud+="cloud/";
				String aNewCloud=aLegacy;aNewCloud+="_cloud/";
				RenameDirectory(aNewCloud,aCloud);

				String aOldSB=aLegacy;aOldSB+="sandbox/";
				String aNewSB=aLegacy;aNewSB+="_sandbox/";
				RenameDirectory(aNewSB,aOldSB);
			}

			String aCloud=aLegacy;aCloud+="cloud/";
			Array<String> aDirList;
			EnumDirectory(aCloud,aDirList,true);

			bool aShouldMove=false;
			if (aDirList.Size()) for (int aCount=0;aCount<aDirList.Size();aCount++) if (aDirList[aCount].ContainsI("profile.cfg")) {aShouldMove=true;break;}
			if (aShouldMove)
			{
				//
				// If we already have a chuzzarium file somewhere current, we do NOT move anything.
				// Put this in under the assumption that Google's file stuff could end up restoring the legacy data back to its old filenames.
				//
				if (!evenIfMoved)
				{
					Array<String> aMyList;
					EnumDirectory("cloud://",aMyList,true);
					for (int aCount=0;aCount<aMyList.Size();aCount++) if (aMyList[aCount].ContainsI("chuzzarium.cfg")) return "[NOLEGACY] A current game exists and wll not be overwritten!";
				}

				CopyDirectory(aLegacy,aSandbox);

				String aNewCloud=aLegacy;aNewCloud+="_cloud/";
				RenameDirectory(aCloud,aNewCloud);

				String aOldSB=aLegacy;aOldSB+="sandbox/";
				String aNewSB=aLegacy;aNewSB+="_sandbox/";
				RenameDirectory(aOldSB,aNewSB);

				gAppPtr->mSettings.Reload();

				return "Legacy game data has been restored!";
			}
			else 
			{
				if (evenIfMoved)
				{
					String aNewCloud=aLegacy;aNewCloud+="_cloud/";
					RenameDirectory(aCloud,aNewCloud);
					String aOldSB=aLegacy;aOldSB+="sandbox/";
					String aNewSB=aLegacy;aNewSB+="_sandbox/";
					RenameDirectory(aOldSB,aNewSB);
				}
				return "[NOLEGACY] No user profiles were found in the legacy location!";
			}
		}
	}
	return "[NOLEGACY] Android is not reporting a location for legacy data!";
}
#else
String RestoreLegacyFiles(bool evenIfMoved)
{
	return "";
}
#endif
#endif


void App::Reboot()
{
	SaveSettings();

	if (mIsolatedCPUManager)
	{
		//
		// We clear it because our CPU's are duplicated in CPUManager...
		// So we don't want them deleted when this CPUManager destroys.
		//
		mIsolatedCPUManager->Clear();
		delete mIsolatedCPUManager;
		mIsolatedCPUManager=NULL;
	}

	if (mCPUManager)
	{
		EnumList(CPU,aCPU,*mCPUManager) aCPU->Kill();
	}

	gAppPtr->mLoadComplete=false;
	mAppDrawNumber=0;
	mRunLoadBackgroundComplete=false;

	mAudio.StopAllSound();
	mAudio.StopAllMusic();


	mLoader=new AssetLoader;
	mLoader->mReloading=true;
	Load();
	RecoverTime();
	*this+=mLoader;
}

void* OSCore_Query(String theQuery) {return OS_Core::Query(theQuery.c());}
void* TransactionCore_Query(String theQuery) {return Transaction_Core::Query(theQuery.c());}
void* GraphicsCore_Query(String theQuery) {return Graphics_Core::Query(theQuery.c());}

void App::SetPortableApp(bool theState) 
{
	if (mPortableApp && !theState)
	{
		OS_Core::LocationAlias("sandbox://","");
		OS_Core::LocationAlias("cloud://","");
		OS_Core::LocationAlias("cache://","");
		OS_Core::LocationAlias("temp://","");
		OS_Core::LocationAlias("social://","");
		OS_Core::LocationAlias("scratch://","");
	}
	mPortableApp=theState;
	if (theState)
	{
		OS_Core::LocationAlias("sandbox://","local://userdata/_sandbox/");
		OS_Core::LocationAlias("cloud://","local://userdata/_cloud/");
		OS_Core::LocationAlias("cache://","local://userdata/_cache/");
		OS_Core::LocationAlias("temp://","local://userdata/_cache/temp/");
		OS_Core::LocationAlias("social://","local://userdata/_sandbox/_social/");
		OS_Core::LocationAlias("scratch://","local://userdata/_cache/_scratch/");
	}
}

//void App::SetCursorPos(int x, int y) {OS_Core::SetCursorPos(x,y);}
void App::SetCursorPos(int x, int y) {}
void App::SetFPSCursor(CPU* theCPU)
{
	mFPSCursorCPU=theCPU;
	if (theCPU) OS_Core::Query("FPSMouse:on");
	else OS_Core::Query("FPSMouse:off");
}


namespace Common
{
	void GetCustomPath(char* thePrefix, char* theResult) {String aPrefix=thePrefix;if (gAppPtr) strcpy(theResult,gAppPtr->GetStorageLocation(aPrefix));}
}

String PeekPath(String thePath) {char aFix[MAX_PATH];Common::FixPath(thePath,aFix);return aFix;}
String ResolvePath(String thePath) {char aFix[MAX_PATH];Common::FixPath(thePath,aFix);return aFix;}


