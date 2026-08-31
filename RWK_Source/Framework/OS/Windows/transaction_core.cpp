#include "network_core.h"
#include "transaction_core.h"
#include "os_core.h"
#include "os_headers.h"
#include <shellapi.h>

#include "rapt_string.h"
#include "rapt_list.h"
#include "rapt_smartpointer.h"
#include "util_core.h"

#define MAX_URL 512

namespace Transaction_Core
{
	bool gFakeBuyStatus=false;

	int gBuyDelay=0;
	Array<char*> gFakePurchaseList;
	bool Purchase(char* theData)
	{
		OS_Core::Printf("Purchasing [%s]",theData);
	
		char* aFake=new char[strlen(theData)+1];
		strcpy(aFake,theData);

		gBuyDelay=100;

		//if (!1)
		if (QuickRand(OS_Core::Tick())%2)
		{
			gFakeBuyStatus=true;
			char* aFake=new char[strlen(theData)+1];
			strcpy(aFake,theData);
			gFakePurchaseList+=aFake;

			OS_Core::Printf("Fake Buy...");
		}
		else
		{
			gFakeBuyStatus=false;
			OS_Core::Printf("Fake Buy Fail...");
		}

		//OS_Core::Execute(theData);
		return false;	// Means, don't wait for a response, fire and forget!
	}

	bool IsPurchaseComplete()
	{
		if (--gBuyDelay<=0) return true;
		return false;
	}

	bool IsPurchased(char* theData)
	{
		OS_Core::Printf("IsPurchased [%s]",theData);
	
		for (int aCount=0;aCount<gFakePurchaseList.Size();aCount++)
		{
			//OS_Core::Printf("Compare: [%s] == [%s]",theData,gFakePurchaseList[aCount]);
			if (_strcmpi(theData,gFakePurchaseList[aCount])==0) return true;
		}
		return false;
	}

	bool GetPurchaseResult(char* theData)
	{
		if (gBuyDelay<=0) return gFakeBuyStatus;
		return false;
	}

	char* GetPurchaseResultText()
	{
		return "Fake Purchase Result Text String...";
	}


	bool ConsumePurchase(char* theData)
	{
		OS_Core::Printf("ConsumePurchase [%s]",theData);
	
		for (int aCount=0;aCount<gFakePurchaseList.Size();aCount++)
		{
			if (_strcmpi(theData,gFakePurchaseList[aCount])==0)
			{
				char* aHold=gFakePurchaseList[aCount];
				gFakePurchaseList.DeleteElement(aCount);
				delete [] aHold;
				return true;
			}
		}
		return false;
	}

	bool AcknowledgePurchase(char* theData)
	{
		OS_Core::Printf("AcknowledgePurchase [%s]",theData);
		return false;
	}


	int gRestoreDelay=0;
	void RestorePurchases()
	{
		gRestoreDelay=100;
		OS_Core::Printf("Fake Restoring Purchases...");
	}

	bool IsRestoreComplete()
	{
		if (--gRestoreDelay<=0) return true;
		return false;
	}


	class AdKey
	{
	public:

		AdKey() {mType=0;mFlags=0;}

		enum
		{
			ADKEY_LOADED=0x01,
			ADKEY_DISMISSED=0x02,

			ADKEY_INTERSTITIAL=0,
			ADKEY_REWARD=1,
		};

		String mName;
		String mID;
		String mResult;

		int mType;
		int mFlags;

		//
		// Add any variables you need for specific ad systems here...
		//
	};

	class AdProvider
	{
	public:
		String mName;
		SmartList(AdKey) mKeyList;
		String GetKey(String theKey) {EnumSmartList(AdKey,aKey,mKeyList) if (aKey->mName==theKey) return aKey->mID;return "";}
	};

	SmartList(AdProvider) gAdProviderList;

	void StartAds(char* theAdData)
	{
		OS_Core::Printf("************************************");
		OS_Core::Printf("Ad Parsing: %s",theAdData);
		OS_Core::Printf("************************************");
		/*
		String aAdData=theAdData;
		String aToken=aAdData.GetToken(';');

		Smart(AdProvider) aCurrentAdProvider;
		while (!aToken.IsNull())
		{
			String aTag=aToken.GetSegmentBefore('=');
			String aValue=aToken.GetSegmentAfter('=');

			if (aTag=="service")
			{
				aCurrentAdProvider=new AdProvider;
				gAdProviderList+=aCurrentAdProvider;
				aCurrentAdProvider->mName=aValue;
			}
			else
			{
				if (aCurrentAdProvider.IsNotNull()) 
				{
					Smart(AdKey) aKey=new AdKey();

					String aAdType;

					if (aValue.Find(':')>0)
					{
						aAdType=aValue.GetSegmentBefore(':');
						aValue=aValue.GetSegmentAfter(':');

						if (aAdType=="interstitial") aKey->mType=AdKey::ADKEY_INTERSTITIAL;
						else if (aAdType=="reward") aKey->mType=AdKey::ADKEY_REWARD;
						else OS_Core::Printf("Unknown Ad type: %s",aAdType.c());
					}

					aKey->mName=aTag;
					aKey->mID=aValue;
					aCurrentAdProvider->mKeyList+=aKey;
				}
			}
			aToken=aAdData.GetNextToken(';');
		}

		EnumSmartList(AdProvider,aAP,gAdProviderList)
		{
			OS_Core::Printf("Initialize Ad Provider: %s (%s)",aAP->mName.c(),aAP->GetKey("appid").c());
			EnumSmartList(AdKey,aKey,aAP->mKeyList)
			{
				OS_Core::Printf("   Key: %s = %s",aKey->mName.c(),aKey->mID.c());
			}
			//if (aProvider.mName=="admob") MobileAds.initialize(mSelfPointer,aProvider.GetKey("appid"));
			//if (aProvider.mName=="chartboost") {}
			// NEW AD PROVIDER
		}
		*/

	}

	int gFakeAdCounter=0;
	void LoadAd(char* theAd)
	{
		gFakeAdCounter=0;
	}

	bool IsAdReady(char* theAd)
	{
#ifdef NO_THREADS
		// For testing purposes... if we're going threadless, we allow this in order to let ad testing go through...
		return true;
#endif
		if (_strcmpi(theAd,"reward")==0) 
		{
			gFakeAdCounter++;
			if (gFakeAdCounter>100) return true;
		}

		return false;
	}

	bool IsAdDismissed()
	{
		return true;
	}
	char* GetAdResult()
	{
		return "reward:10";
	}

	
	struct AdStubHelper {AdStubHelper(HOOKPTR theHook) {mHook=theHook;} HOOKPTR mHook;};
	void ShowAdStub(void* theArg)
	{
		AdStubHelper* aHelper=(AdStubHelper*)theArg;
		while (!IsAdDismissed()) {OS_Core::Sleep(10);}
		if (!OS_Core::gWantShutdown && aHelper && aHelper->mHook) {aHelper->mHook();}

	}

	bool ShowAd(char* theAd,HOOKPTR theHook)
	{
		if (theHook) 
		{
			OS_Core::Thread(&ShowAdStub,new AdStubHelper(theHook));
		}
		return true;
	}
	
	void* Query(char *theInfo, void* extraInfo)
	{
		OS_Core::Printf("!TCORE QUERY: [%s]",theInfo);
		return NULL;
	}
	

	/*
	void LoadAd(Array<char*>& theAdInfo, void* extraData) {}
	bool IsAdReady() {return false;}
	void ShowAd(bool waitForAd) {static int aAdCount=1;OS_Core::Printf("SHOW AD #%d!!!!",aAdCount++);if (waitForAd) OS_Core::Sleep(200);}
	bool IsAdDismissed() {return false;}


	//
	// Reward video stuff, along with some kludges to make it "act" real, even if it's not...
	//
	int gKludgeRewardVideoLoad=0;
	void		    LoadRewardVideo(Array<char*>& theAdInfo, void* extraData)
	{
		gKludgeRewardVideoLoad=0;
	}

	bool			IsRewardVideoReady()
	{
		if (++gKludgeRewardVideoLoad>50) return true;
		return false;
	}

	bool            IsRewardVideoFailed()
	{
		return false;
		//return true;
	}


	void			ShowRewardVideo()
	{
		OS_Core::Printf("Show Reward Video!!!!");
	}

	bool            IsRewardVideoDismissed()
	{
		if (++gKludgeRewardVideoLoad>150) return true;
		return false;
	}

	const char*		GetRewardResults()
	{
		return "WASTETIMEGETGOLD=100,500:1";
	}
	*/
}


