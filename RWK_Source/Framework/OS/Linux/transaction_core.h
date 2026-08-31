#include "os_headers.h"
#include "util_core.h"

#define HOOKPTR std::function<void(void)>


//
// This is a "local system" transaction core, which means it gets handled completely by the user.
// It's targetted at Paypal, but expects an intermediate step where you hit your own url to redirect to paypal.
// It works like this:
// 
// 1. Call Startup(downloadSKUs_URL)
// 2. Then do Purchase(SKUIndex, buyURL)
//
// The buyURL should have all the info you need baked into it.  In most cases, you'll probably want to put the buy url
// into the extradata of the sku, and just tack whatever you need onto the end of that.
//
namespace Transaction_Core
{

	//
	// For paypal, these basically do nothing.
	//
	inline void		Startup() {}
	inline void		Shutdown() {}

	inline char*	GetStoreName() {return "paypal";}

	//
	// Purchasing returns TRUE if you should wait for a response,
	// or false if you shouldn't.  For example, Paypal does not wait for a response, it's
	// just "fire URL and forget"
	//
	bool			Purchase(char* theData);
	bool			IsPurchased(char* theData);
	bool			IsPurchaseComplete();
	bool			ConsumePurchase(char* theData);
	bool			AcknowledgePurchase(char* theData);

	//
	// True if purchase was successful, false if not.  Any extra data goes into "theData" so make sure theData is a char[256] that can hold stuff.
	// This function does nothing on Paypal, and you should not be watching for it because
	// Purchase will have return false.
	//
	bool			GetPurchaseResult(char* theData=NULL);
	char*			GetPurchaseResultText();

	void			RestorePurchases();
	bool			IsRestoreComplete();

	//
	// For Ads... (does nothign on PC)
	//
	void			StartAds(char* theAdData);
	void			LoadAd(char* theAd);
	bool			IsAdReady(char* theAd);
	bool			ShowAd(char* theAd,HOOKPTR theHook=NULL);
	inline bool		IsAdBlocked() {return false;}
	bool			IsAdDismissed();
	char*			GetAdResult();

	/*
	void		    LoadAd(Array<char*>& theAdInfo, void* extraData=NULL);
	bool			IsAdReady();
	void			ShowAd(bool waitForAd=true);
	bool			IsAdDismissed();

	inline bool		IsAdBlocked() {return false;}


	void            LoadRewardVideo(Array<char*>& theAdInfo, void* extraData=NULL);
	bool            IsRewardVideoFailed();
	bool            IsRewardVideoReady();
	void            ShowRewardVideo();
	bool            IsRewardVideoDismissed();
	const char*     GetRewardResults();
	*/
	
	void*			Query(char *theInfo, void* extraInfo=NULL);	
}


