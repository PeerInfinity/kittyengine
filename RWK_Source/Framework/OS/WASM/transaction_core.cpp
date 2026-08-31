#include "transaction_core.h"
#include "sound_core.h"
#include "rapt_string.h"
#include <emscripten.h>

#define __HEADER
#include "common.h"
#undef __HEADER



namespace Transaction_Core
{
	bool Purchase(char* theData)
	{
		return false;
	}

	bool IsPurchaseComplete()
	{
		return false;
	}

	bool IsPurchased(char* theData)
	{
		return false;
	}

	bool GetPurchaseResult(char* theData)
	{
		return false;
	}
	
	char* GetPurchaseResultText()
	{
		return "Fake Purchase Result Text String...";
	}


	void ConsumePurchase(char* theData)
	{
	}

	void RestorePurchases()
	{
	}

	bool IsRestoreComplete()
	{
		return false;
	}
	
	bool gVideoAdComplete=false;

	void StartAds(char* theAdData)
	{
	}

	void LoadAd(char* theAd)
	{
	}

	bool IsAdReady(char* theAd)
	{
		return true; // WASM, looks like they're always ready?
	}

	bool IsAdDismissed()
	{
		return gVideoAdComplete;
	}
	
	String gAdResult;
	char* GetAdResult()
	{
		return gAdResult.c();
	}

	HOOKPTR gAdReturnHook=NULL;
	bool ShowAd(char* theAd, HOOKPTR theHook)
	{
		return false;
	}
	
	void* Query(char *theInfo, void* extraInfo)
	{
		return NULL;
	}
}

extern "C" {
	void AdComplete(char* theResult) 
	{
	}
}


