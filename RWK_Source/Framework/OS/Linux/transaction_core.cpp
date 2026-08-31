#include "network_core.h"
#include "transaction_core.h"
#include "os_core.h"
#include "os_headers.h"

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
			if (strcasecmp(theData,gFakePurchaseList[aCount])==0) return true;
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
	}

	int gFakeAdCounter=0;
	void LoadAd(char* theAd)
	{
		gFakeAdCounter=0;
	}

	bool IsAdReady(char* theAd)
	{
		return false;
	}

	bool IsAdDismissed()
	{
		return true;
	}
	char* GetAdResult()
	{
		return "";
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

}


