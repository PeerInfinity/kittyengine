function Startup()
{
	console.log("Raptisoft Game Startup...");
	HitCounter();
	if (typeof OnStartup === "function") OnStartup(); // Local startup...
}

if (typeof window === "object") {
  Module['arguments'] = window.location.search.substr(1).trim().split('&');
  if (!Module['arguments'][0]) 
 {
    Module['arguments'] = [];
  }
}

//
// Something SDL needs to resume audio...
//
document.getElementById('canvas').addEventListener('click', resumePlaying);
function resumePlaying(e) 
{
	if (document.getElementById("canvas")) document.getElementById("canvas").focus();	
	if (typeof Module === 'undefined' || typeof Module.SDL2 == 'undefined' || typeof Module.SDL2.audioContext == 'undefined') return;
	if (Module.SDL2.audioContext.state == 'suspended') 
	{
			Module.SDL2.audioContext.resume();
	}
	/*
	//
	// Originally this removed the listener after one time.
	// But if in an iFrame, this listener has to hang around so if
	// they click off the frame, they can click to recover things.
	//
	if (Module.SDL2.audioContext.state == 'running')
	{
		document.getElementById('canvas').removeEventListener('click', resumePlaying);
		document.removeEventListener('keydown', resumePlaying);
	}
	*/
}

//
// Fullscreen handler...
//
function requestFullScreen(element) 
{
	// Supports most browsers and their versions.
	var requestMethod = element.requestFullScreen || element.webkitRequestFullScreen || element.mozRequestFullScreen || element.msRequestFullScreen;
	if (requestMethod) 
	{ // Native full screen.
		requestMethod.call(element);
	} 
	else if (typeof window.ActiveXObject !== "undefined") 
	{ // Older IE.
		var wscript = new ActiveXObject("WScript.Shell");
		if (wscript !== null) 
		{
			wscript.SendKeys("{F11}");
		}
	}
}
function getCanvasWidth() {return parseInt(document.getElementById('canvas').style.width,10);}
function getCanvasHeight() {return parseInt(document.getElementById('canvas').style.height,10);}

document.getElementById('canvas').addEventListener("keydown", event => 
{
	if (event.keyCode === 120) 
	{
		requestFullScreen(document.getElementById('canvas'));
		return;
	}
});

//
// For ads...
// Implement a custom _ShowInterstitial function for different ad providers.
//
function ShowInterstitial() 
{
	//console.log("Show Interstitial...");
	if (typeof _ShowInterstitial === "function") {_ShowInterstitial();}
	else 
	{
		console.log("No _ShowInterstitial function implemented!");
		
		var aResult="blocked";
		var aStringPtr=allocate(intArrayFromString(aResult), 'i8', ALLOC_NORMAL);				
		_AdComplete(aStringPtr);
		_free(aStringPtr);
	}
}

function HideInterstitial()
{
	document.getElementById('canvas').focus();
}

function RefreshBanner()
{
	//console.log("Refresh banner...");
	if (typeof _RefreshBanner === "function") _RefreshBanner();
}


function IsAdBlock() {return 0;}

function CopyToClipboard(theText)
{
	var copyText = document.getElementById("cutpaste_textbox");
	if (copyText==null)
	{
		document.body.insertAdjacentHTML('beforeend','<div id="cutpaste_div"><input type="text" value="" id="cutpaste_textbox"></div>');
	}
	copyText = document.getElementById("cutpaste_textbox");
	if (copyText!=null)
	{
		document.getElementById("cutpaste_div").style.visibility = "visible";
		copyText.value=theText;
		copyText.select();
		copyText.setSelectionRange(0, 99999);
		document.execCommand("copy");
		document.getElementById("cutpaste_div").style.visibility = "hidden";
	}
}

function PasteListener(e) 
{
    var clipboardData, pastedData;
    e.stopPropagation();
    e.preventDefault();
    clipboardData = e.clipboardData || window.clipboardData;
    var aPasteData=clipboardData.getData('Text');
	
	var aStringPtr=allocate(intArrayFromString(aPasteData), 'i8', ALLOC_NORMAL);				
	_SetPasteData(aStringPtr);
	_free(aStringPtr);
	
	alert("You can now use the clipboad data in the game!");
    //alert(gPasteData);
}

function ShowPasteHelper(theShow)
{
	var aHelper = document.getElementById("paste_helper");
	if (aHelper==null)
	{
		document.body.insertAdjacentHTML('beforeend','<center><div id="paste_helper" contenteditable="true">The Application is expecting clipboard data.  Please right click here and select "paste" to make it available!</div>');
	}
	aHelper = document.getElementById("paste_helper");
	if (aHelper!=null) 
	{
		if (theShow==0) 
		{
			aHelper.style.display="none";
			document.getElementById('paste_helper').removeEventListener('paste', PasteListener);
		}
		if (theShow==1) 
		{
			aHelper.style.display="block";
			document.getElementById('paste_helper').addEventListener('paste', PasteListener);
		}
	}
}

function HitCounter()
{
	//
	// Whatever you need to count page hits
	//
}
