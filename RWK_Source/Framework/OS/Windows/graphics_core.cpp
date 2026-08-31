#include "graphics_core.h"
#include "os_core.h"

#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>

#define __HEADER
#include "..\common.h"
#undef __HEADER


#define _EXT
#define _ARB


class RGBA
{
public:
	RGBA() {}
	RGBA(int r, int g, int b, int a)
	{
		mB=b;
		mR=r;
		mG=g;
		mA=a;
	}
	RGBA(int a)
	{
		mB=255;
		mR=255;
		mG=255;
		mA=a;
	}

	inline bool operator==(RGBA &thePixel) {return ((mR==thePixel.mR)&&(mG==thePixel.mG)&&(mB==thePixel.mB)&&(mA==thePixel.mA));}
	inline bool operator!=(RGBA &thePixel) {return ((mR!=thePixel.mR)||(mG!=thePixel.mG)||(mB!=thePixel.mB)||(mA!=thePixel.mA));}

	unsigned char mB;
	unsigned char mG;
	unsigned char mR;
	unsigned char mA;
};

namespace OS_Core
{
	extern int gReserveTopScreen;
	extern int gReserveBottomScreen;
};

namespace Graphics_Core
{
static SDL_Surface			*gSDLScreen=NULL;
SDL_Renderer*				gRenderer=NULL;
SDL_Window*					gWindow=NULL;
SDL_GLContext				gContext;

int							gResolutionWidth=800;
int							gResolutionHeight=600;
bool						gFullscreen=false;
int							gPageWidth=800;
int							gPageHeight=600;
float						gPageWidthMultiplier=1.0f;
float						gPageHeightMultiplier=1.0f;
float						gTouchWidthMultiplier=1.0f;
float						gTouchHeightMultiplier=1.0f;
int							gDesktopWidth=-1;
int							gDesktopHeight=-1;
bool						gGraphicsPaused=false;

float                       gZNear=0.0f;
float						gZDepth=0.0f;
int							gStencilSize=0;
GLuint						gStencilValue=1;
bool						gZBuffer=false;
bool						gReverseRGB=false;		// This will depend on platform! MACFIX

float						g3DOffsetX=0.0f;
float						g3DOffsetY=0.0f;

//bool						gIsRenderToTexture=false;
//int                         gRenderToTextureNumber=0;

bool						gThreadsEnabled=false;
CRITICAL_SECTION			gThreadLock;

// Seems to be needed for SDL...
CRITICAL_SECTION			gImageLoadThreadLock;
void						ImageLoadThreadLock();
void						ImageLoadThreadUnlock();


//
// Dev stuff:
//
enum
{
	FAKE_PHONE=0x01,
	FAKE_WEB=0x02,
};
char						gFakeDevice=0;
bool						gFakeSmallScreen=false;
int							gFakeDeviceResolutionX=0;
int							gFakeDeviceResolutionY=0;


/*
void CreateGLTexture(int theWidth, int theHeight, int theFormat, char *theBits)
{
	//
	// Wrapped here so if we add new texture formats, we can do it in one place.
	//
	switch (theFormat)
	{
		case 0:case 8888:glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,theWidth,theHeight,0,GL_RGBA,GL_UNSIGNED_BYTE,theBits);break;
		case 1:case 565:glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,theWidth,theHeight,0,GL_RGB,GL_UNSIGNED_SHORT_5_6_5,theBits);break;
		case 2:case 4444:glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,theWidth,theHeight,0,GL_RGBA,GL_UNSIGNED_SHORT_4_4_4_4,theBits);break;
		default:OS_Core::Printf("Graphics_Core::CreateGLTexture() : Bad Texture Format: %d",theFormat);break;
	}
}
*/

};

//#define LEGACY_GL
#ifdef LEGACY_GL
#include "gl11.h"
#else
#include "gl30.h"
#endif

void Graphics_Core::PreStartup()
{
#ifndef LEGACY_GL
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 2 );
#endif

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE,8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,8);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,1);
	SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);
	if (Graphics_Core::gZDepth>0) {SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,16);glDepthFunc(GL_LEQUAL);}
	if (Graphics_Core::gStencilSize>0) SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE,gStencilSize);
}

void Graphics_Core::Startup()
{
	//
	// Do the initialization you need to get graphics going...
	//
	// #ifdef _PORT_STARTUPGFX
	// !ERROR
	// #endif
	//

	GetDesktopSize(&gDesktopWidth,&gDesktopHeight);
	gWindow=(SDL_Window*)OS_Core::Query("WINDOW");
	IMG_Init(IMG_INIT_PNG|IMG_INIT_JPG);


	StartGL();

	// #ifdef _PORT_ZBUFFER
	// !ERROR -- You will want to initialize z buffering if gZDepth>0 here...
	// #endif

	gSDLScreen=SDL_GetWindowSurface(gWindow);
	if (!gSDLScreen) OS_Core::Error("Could not create SDL Screen...");

	int aFlags=0;
	if (gFullscreen) aFlags=SDL_WINDOW_FULLSCREEN_DESKTOP;
	SDL_SetWindowFullscreen(gWindow,aFlags);
	
	gContext=SDL_GL_CreateContext(gWindow);
	if (gContext==NULL) OS_Core::Error("Could not create OpenGL Context...");

	gRenderer=SDL_CreateRenderer(gWindow,-1,SDL_RENDERER_ACCELERATED|SDL_RENDERER_TARGETTEXTURE);
	//gRenderer=SDL_CreateRenderer(gWindow,-1,SDL_RENDERER_ACCELERATED|SDL_RENDERER_TARGETTEXTURE|SDL_RENDERER_PRESENTVSYNC);
	if (gRenderer==NULL) OS_Core::Error("Could not create OpenGL Renderer...");

#ifdef _GLEW
	glewInit();
#endif

	SetRenderDefaults();

	/*
	int aSS;
	SDL_GL_GetAttribute(SDL_GL_STENCIL_SIZE,&aSS);
	OS_Core::Printf("Stencil Size: %d",aSS);
	exit(0);
	*/
}

void Graphics_Core::Shutdown()
{
	//
	// Do what you need to do to shut down any graphics...
	//
	// #ifdef _PORT_STARTUPGFX
	// !ERROR
	// #endif
	//
	IMG_Quit();

}

int Graphics_Core::GetPageWidth() {return gPageWidth;}
int	Graphics_Core::GetPageHeight() {return gPageHeight;}


void Graphics_Core::Show()
{
	//
	// Flip the page, or present surface, or whatever...
	//
	// #ifdef _PORT_STARTUPGFX
	// !ERROR
	// #endif
	//
	SDL_GL_SwapWindow(gWindow);
}





bool Graphics_Core::IsZoomed()
{
	//
	// Simply return true if the page is stretched at all (i.e. if resolution!=page resolution).
	// This might become obsolete if I move all this out of the core...
	//
	#ifdef _PORT_FINAL
	!ERROR
	 #endif
	//
	if (gPageHeightMultiplier!=1.0f || gPageWidthMultiplier!=1.0f) return true;
	return false;
}

bool Graphics_Core::LoadImage_PNG(char *theFilename, unsigned char **theBits, int *theWidth, int *theHeight)
{
	_FixPath(theFilename);
	//
	// Load a PNG's stream into "thebits."  Most mobile OS's do this natively!
	//
	ImageLoadThreadLock(); // Looks like SDL needs this, otherwise we'll crash if trying to load too many in too many threads?
	SDL_Surface *aSurface=IMG_Load(theFilename);
	ImageLoadThreadUnlock();
	if (aSurface)
	{
		if (aSurface->format->BitsPerPixel!=32)
		{
			//
			// Convert it to 32 bits per pixel...
			//
			unsigned int rmask,gmask,bmask,amask;

#if SDL_BYTEORDER==SDL_BIG_ENDIAN
			rmask=0xFF000000;
			gmask=0x00FF0000;
			bmask=0x0000FF00;
			amask=0x000000FF;	
#else
			rmask=0x000000FF;
			gmask=0x0000FF00;
			bmask=0x00FF0000;
			amask=0xFF000000;			
#endif

			SDL_Surface *aNewSurface=SDL_CreateRGBSurface(SDL_SWSURFACE,aSurface->w,aSurface->h,32,rmask,gmask,bmask,amask);
			SDL_BlitSurface(aSurface,NULL,aNewSurface,NULL);
			SDL_FreeSurface(aSurface);
			aSurface=aNewSurface;
		}

		if (SDL_MUSTLOCK(aSurface)) SDL_LockSurface(aSurface);
		char *aPtr=(char*)aSurface->pixels;
		char *aBits=new char[aSurface->w*aSurface->h*4];
		memcpy(aBits,aPtr,aSurface->w*aSurface->h*4);
		if (SDL_MUSTLOCK(aSurface)) SDL_UnlockSurface(aSurface);

		*theWidth=aSurface->w;
		*theHeight=aSurface->h;
		*theBits=(unsigned char*)aBits;

		SDL_FreeSurface(aSurface);
		return true;
	}
	return false;
}

bool Graphics_Core::LoadImageMem_PNG(const unsigned char *theData, int theDataLen, unsigned char **theBits, int *theWidth, int *theHeight)
{
	SDL_RWops* aOps=SDL_RWFromConstMem(theData,theDataLen);
	SDL_Surface *aSurface=IMG_Load_RW(aOps,1);
	if (aSurface)
	{
		if (aSurface->format->BitsPerPixel!=32)
		{
			//
			// Convert it to 32 bits per pixel...
			//
			unsigned int rmask,gmask,bmask,amask;

#if SDL_BYTEORDER==SDL_BIG_ENDIAN
			rmask=0xFF000000;
			gmask=0x00FF0000;
			bmask=0x0000FF00;
			amask=0x000000FF;	
#else
			rmask=0x000000FF;
			gmask=0x0000FF00;
			bmask=0x00FF0000;
			amask=0xFF000000;			
#endif

			SDL_Surface *aNewSurface=SDL_CreateRGBSurface(SDL_SWSURFACE,aSurface->w,aSurface->h,32,rmask,gmask,bmask,amask);
			SDL_BlitSurface(aSurface,NULL,aNewSurface,NULL);
			SDL_FreeSurface(aSurface);
			aSurface=aNewSurface;
		}

		if (SDL_MUSTLOCK(aSurface)) SDL_LockSurface(aSurface);
		char *aPtr=(char*)aSurface->pixels;
		char *aBits=new char[aSurface->w*aSurface->h*4];
		memcpy(aBits,aPtr,aSurface->w*aSurface->h*4);
		if (SDL_MUSTLOCK(aSurface)) SDL_UnlockSurface(aSurface);

		*theWidth=aSurface->w;
		*theHeight=aSurface->h;
		*theBits=(unsigned char*)aBits;

		SDL_FreeSurface(aSurface);
		return true;
	}
	return false;
}


bool Graphics_Core::LoadImage_GIF(char *theFilename, unsigned char **theBits, int *theWidth, int *theHeight)
{
	return LoadImage_PNG(theFilename,theBits,theWidth,theHeight); // Just works
}

bool Graphics_Core::LoadImage_JPG(char *theFilename, unsigned char **theBits, int *theWidth, int *theHeight)
{
	return LoadImage_PNG(theFilename,theBits,theWidth,theHeight); // Just works
}

/*
bool Graphics_Core::LoadImage_RAW(char *theFilename, unsigned char **theBits, int *theWidth, int *theHeight)
{
	theFilename=Common::FixPath(theFilename);
	//
	// Load our own "RAW" file format... 
	// Basically this, in whatever file reading format is best for the OS
	//
	//		int aWidth;
	//		int aHeight;
	//		read(aFile,&aWidth,4);
	//		read(aFile,&aHeight,4);
	//		unsigned char *aBits=new unsigned char[aWidth*aHeight*4];
	//		read(aFile,aBits,aWidth*aHeight*4);
	//		close(aFile);
	//		*theWidth=aWidth;
	//		*theHeight=aHeight;
	//		*theBits=(unsigned char*)aBits;
	//		return true;
	//
	// #ifdef _PORT_LOADRAW
	// !ERROR
	// #endif
	//
	return false;
}


void Graphics_Core::SaveImage_RAW(char *theFilename, unsigned char *theBits, int theWidth, int theHeight)
{
	//
	// Save a Raw Image.  More necessary, since we DO need a way to store images with the client.
	//
	// #ifdef _PORT_EXPORTIMG
	// !ERROR
	// #endif
	//
}

*/

void Graphics_Core::SaveImage_PNG(char *theFilename, unsigned char *theBits, int theWidth, int theHeight)
{
	_FixPath(theFilename);

	long aRMask;
	long aGMask;
	long aBMask;
	long aAMask;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	aRMask = 0xff000000;
	aGMask = 0x00ff0000;
	aBMask = 0x0000ff00;
	aAMask = 0x000000ff;
#else
	aRMask = 0x000000ff;
	aGMask = 0x0000ff00;
	aBMask = 0x00ff0000;
	aAMask = 0xff000000;
#endif

	SDL_Surface* aImage=SDL_CreateRGBSurface(0,theWidth,theHeight,32,aRMask,aGMask,aBMask,aAMask);
	SDL_LockSurface(aImage);
	memcpy(aImage->pixels,theBits,theWidth*theHeight*4);
	SDL_UnlockSurface(aImage);
	IMG_SavePNG(aImage,theFilename);
	SDL_FreeSurface(aImage);

	/*
	SDL_Surface * image = SDL_LoadBMP("image.bmp");
	IMG_SavePNG(image, "out.png");
	SDL_FreeSurface(image);
	*/

	//
	// Save a PNG.  Implement if you can, but it's not hugely necessary unless you're making an editor or
	// something.
	//
	// #ifdef _PORT_EXPORTIMG
	// !ERROR
	// #endif
	//
}



void Graphics_Core::Multitasking(bool inForeground)
{
	ReloadEverything();
}

#ifndef LEGACY_GL
/*

//
// Moved this into GL30 so that I can add texture styles without too much tweaking
//
int	Graphics_Core::CreateTexture(int theWidth, int theHeight, int theFormat) 
{
	unsigned char *aBitFix=NULL; // COPYME
	#define CLEARCOLOR 0 
	int aBitSize=0; // COPYME
	switch (theFormat)
	{
		case 0:case 8888:aBitSize=4;break;
		case 1:case 565:
		case 2:case 4444:
			aBitSize=2;break;
	}

	// COPYME
	if (aBitSize>0)
	{
		aBitFix=new unsigned char[theWidth*theHeight*aBitSize];
		memset(aBitFix,CLEARCOLOR,theWidth*theHeight*aBitSize);
	}

	ThreadLock();
	unsigned int aNewTex=0;
	for (aNewTex=0;aNewTex<gTextureList.size();aNewTex++) if (gTextureList[aNewTex]->IsOpen()) break;
	if (aNewTex>=gTextureList.size()) gTextureList.push_back(new TextureStruct());

	gTextureList[aNewTex]->mGLTexture=-1; //(int)aTexture;
	gTextureList[aNewTex]->mFlags|=GFXFLAG_PENDING;
	gTextureList[aNewTex]->mWidth=theWidth;
	gTextureList[aNewTex]->mHeight=theHeight;
	gTextureList[aNewTex]->mFormat=theFormat;
	gTextureList[aNewTex]->mBits=aBitFix;
	ThreadUnlock();

	return aNewTex;
}
*/

int	Graphics_Core::LoadTexture(int theWidth, int theHeight, unsigned char *theBits, int theFormat)
{
	int aFormatSize = 4;
	if (theFormat == 1 || theFormat==565) aFormatSize = 2;
	if (theFormat == 2 || theFormat==4444) aFormatSize = 2;
	//if (theFormat == 'C') aFormatSize = 2;

	unsigned char *aBitFix=new unsigned char[theWidth * (theHeight) * aFormatSize];
	unsigned char *aBitPtr = aBitFix;


	if (theFormat == 0 || theFormat==8888)
	{
		memcpy(aBitFix, theBits, theWidth * (theHeight) * 4);
	}
	if (theFormat == 1 || theFormat==565)
	{
		{
			int aSize = theHeight * theWidth;

			for (int aCount = 0; aCount < aSize; aCount++)
			{
				int aSrca = theBits[aCount * 4 + 2] / 8;
				int aSrcb = theBits[aCount * 4 + 1] / 4;
				int aSrcc = theBits[aCount * 4 + 0] / 8;

				unsigned int aCombined = ((aSrca << 11)&0xF800) | ((aSrcb << 5)&0x7E0) | ((aSrcc << 0)&0x1F);

				*aBitPtr=aCombined&0x00FF;aBitPtr++;
				*aBitPtr=aCombined>>8;aBitPtr++;
			}
		}
	}
	if (theFormat == 2 || theFormat==4444)
	{
		{
			int aSize = theHeight * theWidth;

			for (int aCount = 0; aCount < aSize; aCount++)
			{
				int aSrcd = theBits[aCount * 4 + 3] / 16;
				int aSrca = theBits[aCount * 4 + 2] / 16;
				int aSrcb = theBits[aCount * 4 + 1] / 16;
				int aSrcc = theBits[aCount * 4 + 0] / 16;

				*aBitPtr = (aSrcc << 4 | aSrcd);
				aBitPtr++;
				*aBitPtr = (aSrca << 4 | aSrcb);
				aBitPtr++;
			}
		}
	}

	ThreadLock();
	unsigned int aNewTex = 0;
	for (aNewTex = 0; aNewTex < gTextureList.size(); aNewTex++) {if (gTextureList[aNewTex]->IsOpen()) break;}
	if (aNewTex>=gTextureList.size()) gTextureList.push_back(new TextureStruct());

	gTextureList[aNewTex]->mGLTexture=-1;
	gTextureList[aNewTex]->mFlags|=GFXFLAG_PENDING;
	gTextureList[aNewTex]->mWidth = theWidth;
	gTextureList[aNewTex]->mHeight = theHeight;
	gTextureList[aNewTex]->mBits = aBitFix;
	gTextureList[aNewTex]->mFormat=theFormat;
	ThreadUnlock();

	/*
	TextureStruct aT;
	aT.mGLTexture=-1;
	aT.mFlags|=GFXFLAG_PENDING;
	aT.mWidth = theWidth;
	aT.mHeight = theHeight;
	aT.mBits = aBitFix;
	aT.mFormat=theFormat;

	if (aNewTex<gTextureList.size()) gTextureList[aNewTex]=aT;
	else gTextureList.push_back(aT);
	*/

	return (int) aNewTex;
}

/*
//
// Moved into GL30
//
int	Graphics_Core::CreateRenderer(int theWidth, int theHeight, longlong pixelFormat, bool canRead)
{
	int aNewTex;

	aNewTex=CreateTexture(theWidth, theHeight, pixelFormat);
	gTextureList[aNewTex]->mFlags|=GFXFLAG_RENDERER;

	if (canRead)
	{
		int aPixelSize = 0;
		switch (pixelFormat)
		{
			case 0:case 8888:aPixelSize = 4;break;
			case 1:case 565:aPixelSize = 2;break;
			case 16:aPixelSize = 2;break;
		}
		gTextureList[aNewTex]->mRendererBits=new char[theWidth*theHeight*aPixelSize];
	}
	return aNewTex;
}
*/
#else
//
// The difference?  LegacyGL uses our array, newer GL just uses std::vector
//
int	Graphics_Core::CreateTexture(int theWidth, int theHeight, int theFormat) 
{
	unsigned char *aBitFix;
	#define CLEARCOLOR 0
	int aBitSize=4;
	switch (theFormat)
	{
		case 0:case 8888:aBitSize=4;break;
		case 2:case 4444:
		case 1:case 565:aBitSize=2;break;
	}
	aBitFix=new unsigned char[theWidth*theHeight*aBitSize];
	memset(aBitFix,CLEARCOLOR,theWidth*theHeight*aBitSize);

	int aNewTex=0;
	for (aNewTex=0;aNewTex<gTextureList.Size();aNewTex++) if (gTextureList[aNewTex].IsOpen()) break;

	gTextureList[aNewTex].mGLTexture=-1; //(int)aTexture;

	gTextureList[aNewTex].mFlags|=GFXFLAG_PENDING;
	gTextureList[aNewTex].mWidth=theWidth;
	gTextureList[aNewTex].mHeight=theHeight;
	gTextureList[aNewTex].mFormat=theFormat;
	gTextureList[aNewTex].mBits=aBitFix;
	//delete [] aBitFix;
	return aNewTex;
}

int	Graphics_Core::LoadTexture(int theWidth, int theHeight, unsigned char *theBits, int theFormat)
{
	int aFormatSize = 4;
	if (theFormat == 1 || theFormat==565) aFormatSize = 2;
	if (theFormat == 2 || theFormat==4444) aFormatSize = 2;

	unsigned char *aBitFix=new unsigned char[theWidth * (theHeight) * aFormatSize];
	unsigned char *aBitPtr = aBitFix;


	if (theFormat == 0 || theFormat==8888)
	{
		memcpy(aBitFix, theBits, theWidth * (theHeight) * 4);
	}
	if (theFormat == 1 || theFormat==565)
	{
		{
			int aSize = theHeight * theWidth;

			for (int aCount = 0; aCount < aSize; aCount++)
			{
				int aSrca = theBits[aCount * 4 + 2] / 8;
				int aSrcb = theBits[aCount * 4 + 1] / 4;
				int aSrcc = theBits[aCount * 4 + 0] / 8;

				unsigned int aCombined = ((aSrca << 11)&0xF800) | ((aSrcb << 5)&0x7E0) | ((aSrcc << 0)&0x1F);

				*aBitPtr=aCombined&0x00FF;aBitPtr++;
				*aBitPtr=aCombined>>8;aBitPtr++;
			}
		}
	}
	if (theFormat == 2 || theFormat==4444)
	{
		{
			int aSize = theHeight * theWidth;

			for (int aCount = 0; aCount < aSize; aCount++)
			{
				int aSrcd = theBits[aCount * 4 + 3] / 16;
				int aSrca = theBits[aCount * 4 + 2] / 16;
				int aSrcb = theBits[aCount * 4 + 1] / 16;
				int aSrcc = theBits[aCount * 4 + 0] / 16;

				*aBitPtr = (aSrcc << 4 | aSrcd);
				aBitPtr++;
				*aBitPtr = (aSrca << 4 | aSrcb);
				aBitPtr++;
			}
		}
	}

	int aNewTex = 0;
	for (aNewTex = 0; aNewTex < gTextureList.Size(); aNewTex++) if (gTextureList[aNewTex].IsOpen()) break;
	gTextureList[aNewTex].mGLTexture=-1;
	gTextureList[aNewTex].mFlags|=GFXFLAG_PENDING;
	gTextureList[aNewTex].mWidth = theWidth;
	gTextureList[aNewTex].mHeight = theHeight;
	gTextureList[aNewTex].mBits = aBitFix;
	gTextureList[aNewTex].mFormat=theFormat;
	return (int) aNewTex;
}

#ifdef LEGACY_GL
int	Graphics_Core::CreateRenderer(int theWidth, int theHeight, bool wantAlpha, bool canRead)
{
	int aNewTex;

	int aFormat;
	if (wantAlpha) aFormat = 0;
	else aFormat = 1;

	aNewTex=CreateTexture(theWidth, theHeight, aFormat);
	gTextureList[aNewTex].mFlags|=GFXFLAG_RENDERER;

	if (canRead)
	{
		int aPixelSize = 0;
		switch (aFormat)
		{
		case 0:case 8888:aPixelSize = 4;break;
		case 1:case 565:aPixelSize = 2;break;
		}
		gTextureList[aNewTex].mRendererBits=new char[theWidth*theHeight*aPixelSize];
	}
	return aNewTex;
}
#else
int	Graphics_Core::CreateRenderer(int theWidth, int theHeight, int pixelFormat, bool canRead)
{
	int aNewTex;

	aNewTex=CreateTexture(theWidth, theHeight, pixelFormat);
	gTextureList[aNewTex].mFlags|=GFXFLAG_RENDERER;

	if (canRead)
	{
		int aPixelSize = 0;
		switch (pixelFormat)
		{
			case 0:case 8888:aPixelSize = 4;break;
			case 1:case 565:aPixelSize = 2;break;
			case 16:aPixelSize = 2;break;
		}
		gTextureList[aNewTex].mRendererBits=new char[theWidth*theHeight*aPixelSize];
	}
	return aNewTex;
}
#endif

//
// Dummy functions, to let us compile...
//
int Graphics_Core::GetShaderGlobalRef(int theShader, int vertexOrPixel, char* theVariableName) {return -1;}
void Graphics_Core::SetShaderGlobal_Matrix(int theRef, float* theData) {}
void Graphics_Core::SetShaderGlobal_Float(int theRef, float* theData, int theCount) {}
void Graphics_Core::SetShaderGlobal_Int(int theRef, int* theData, int theCount) {}
void Graphics_Core::SetShaderGlobal_UInt(int theRef, unsigned int* theData, int theCount) {}
int Graphics_Core::CreateShader(char* theVertexShader, char* thePixelShader, Array<char*>& theVertexFormat) {return -1;}
void Graphics_Core::UnloadShader(int theNumber) {}
void Graphics_Core::SetShader(int theShader) {}
void Graphics_Core::DrawShape(int theShader, void *theBuffer, int theVCount) {}
void Graphics_Core::DrawIndexedShape(int theShader, void *theVertexBuffer, int theVertexCount, void* theIndexBuffer, int theIndexCount) {}
int	Graphics_Core::CreateShape(int theShader, void* theVertexList, int theVertexCount, vbindex* theIndexList, int theIndexCount) {return -1;}
#endif


void Graphics_Core::SetResolution(int theWidth, int theHeight, bool fullScreen)
{
	//
	// If we're faking phone stuff...
	//
	if (IsFakeDevice())
	{
		theWidth=GetFakeDeviceResolutionX();
		theHeight=GetFakeDeviceResolutionY();
		fullScreen=false;
	}


	//
	// Sets the GRAPHICAL resolution.  This sets all the graphics card resolution info (like for fullscreen, etc).
	// Not to be confused with the WINDOW resolution, which is set in OS_Core.  This can be considered the "real" setresolution call,
	// the OS_Core one only exists to make the system window the right size.  An app will set resolution by doing this:
	//
	//				OS_Core::SetResolution(w,h,f);
	//				Graphics_Core::SetResolution(w,h,f);
	//
	// Don't forget to recover textures/shapes here if that's an issue.
	//
	// #ifdef _PORT_RESOLUTION
	// !ERROR
	// #endif
	//
	gFullscreen=fullScreen;
	gResolutionWidth=theWidth;
	gResolutionHeight=theHeight;

	if (gRenderer)
	{
		SDL_DisplayMode aMode;
		SDL_GetWindowDisplayMode(gWindow,&aMode);
		aMode.w=theWidth;
		aMode.h=theHeight;
		SDL_SetWindowDisplayMode(gWindow,&aMode);

		//
		// Have to change window size?
		// 

		int aFlags=0;
		if (fullScreen) aFlags=SDL_WINDOW_FULLSCREEN_DESKTOP;
		SDL_SetWindowFullscreen(gWindow,aFlags);

		if (fullScreen)
		{
			SDL_GetWindowDisplayMode(gWindow,&aMode);
			gResolutionWidth=aMode.w;
			gResolutionHeight=aMode.h;
		}
		else
		{
			//
			// Center the window...
			//
			SDL_SetWindowSize(gWindow,gResolutionWidth,gResolutionHeight);

			//SDL_DisplayMode aMode;
			//if (SDL_GetDesktopDisplayMode(0,&aMode)!=0) OS_Core::Error("Could not get desktop mode: [%s]",SDL_GetError());
			//SDL_SetWindowPosition(gWindow,(aMode.w/2)-(gResolutionWidth/2),(aMode.h/2)-(gResolutionHeight/2));

			HWND aWindow=GetDesktopWindow();
			RECT aRect;GetClientRect(aWindow,&aRect);
			SDL_SetWindowPosition(gWindow,aRect.left+((aRect.right-aRect.left)/2)-(gResolutionWidth/2),aRect.top+((aRect.bottom-aRect.top)/2)-(gResolutionHeight/2));
		}

		gSDLScreen=SDL_GetWindowSurface(gWindow);
		if (!gSDLScreen) OS_Core::Error("Could not create SDL Screen...");

		SDL_DestroyRenderer(gRenderer);
		Startup();
		ReloadEverything();
	}

	OS_Core::Printf("");

}

void Graphics_Core::ConvertResolutionToPage(int *theX, int *theY)
{
	//
	// Conversion for visible resolution to logical resolution...
	// Probably can just uncomment this code!
	//
	// #ifdef _PORT_RESOLUTION
	// !ERROR
	// #endif
	//

	return;
	float aX=(float)*theX;
	float aY=(float)*theY;
	aX/=gPageWidthMultiplier;
	aY/=gPageHeightMultiplier;
	*theX=(int)aX;
	*theY=(int)aY;
}

void Graphics_Core::ConvertTouchResolutionToPage(int *theX, int *theY)
{
	//
	// Conversion for visible resolution to logical resolution...
	// Probably can just uncomment this code!
	//
	// #ifdef _PORT_RESOLUTION
	// !ERROR
	// #endif
	//

	float aX=(float)*theX;
	float aY=(float)*theY;
	aX/=gTouchWidthMultiplier;
	aY/=gTouchHeightMultiplier;
	*theX=(int)aX;
	*theY=(int)aY;
}


void Graphics_Core::ConvertPageToResolution(int *theX, int *theY)
{
	//
	// Conversion for visible resolution to logical resolution...
	// Probably can just uncomment this code!
	//
	// #ifdef _PORT_RESOLUTION
	// !ERROR
	// #endif
	//

	float aX=(float)*theX;
	float aY=(float)*theY;
	aX*=gPageWidthMultiplier;
	aY*=gPageHeightMultiplier;
	*theX=(int)aX;
	*theY=(int)aY;
}

void Graphics_Core::GetDesktopSize(int *theWidth, int *theHeight)
{
	//
	// Gets the desktop resolution, if the system supports it.  If not, just return the resolution we've
	// got for the game.
	//
	// #ifdef _PORT_RESOLUTION
	// !ERROR
	// #endif
	//
	if (gDesktopWidth==-1)
	{
		SDL_DisplayMode aMode;
		if (SDL_GetDesktopDisplayMode(0,&aMode)!=0) OS_Core::Error("Could not get desktop mode: [%s]",SDL_GetError());
		gDesktopWidth=aMode.w;
		gDesktopHeight=aMode.h;
	}
	*theWidth=gDesktopWidth;
	*theHeight=gDesktopHeight;
}

bool Graphics_Core::IsTooBigForWindowed(int theWidth, int theHeight)
{
	//
	// Simply tells us if the resolution requested is too big for windowed mode.
	//
	// #ifdef _PORT_FULLSCREEN
	// !ERROR
	// #endif

	GetDesktopSize(&gDesktopWidth,&gDesktopHeight);
	if (theWidth>gDesktopWidth) return true;
	if (theHeight>gDesktopHeight-100) return true;

	return false;
}

namespace Graphics_Core
{
	Array<Array<int>> gResolutionList;
}
Array< Array<int> >& Graphics_Core::GetAvailableResolutions()
{
	//
	// Gets a list of available resolutions for the system, to present to the user.
	// If it's a rez-locked system, just return the one.
	//
	// #ifdef _PORT_RESOLUTION
	// !ERROR
	// #endif
	//
	if (gResolutionList.Size()==0)
	{
		int aModeCount=SDL_GetNumDisplayModes(0);
		int aRealCount=0;
		for (int aCount=0;aCount<aModeCount;aCount++)
		{
			SDL_DisplayMode aMode;
			if (SDL_GetDisplayMode(0,aCount,&aMode)==0)
			{
				bool aNewRez=true;
				for (int aCheckCount=0;aCheckCount<gResolutionList.Size();aCheckCount++) if (gResolutionList[aCheckCount][0]==aMode.w && gResolutionList[aCheckCount][1]==aMode.h) {aNewRez=false;break;}
				if (aNewRez)
				{
					gResolutionList[aRealCount][0]=aMode.w;
					gResolutionList[aRealCount][1]=aMode.h;
					aRealCount++;
				}
			}
		}
	}

	return gResolutionList;
}



/*
int Graphics_Core::CreateRenderer(int theWidth, int theHeight, bool wantAlpha, bool canRead)
{
	//
	// Create a "render-to" texture, howsoever one does it on the current OS
	//
	// #ifdef _PORT_RENDERTO
	// !ERROR
	// #endif
	//

	int aNewTex;
	if (wantAlpha) aNewTex=CreateTexture(theWidth,theHeight);
	else aNewTex=CreateTexture(theWidth,theHeight,1);

	GLuint aTexture=gTextureList[aNewTex].mGLTexture;

	GLuint aFrameBuffer;
	glGenFramebuffersEXT(1,&aFrameBuffer);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,aFrameBuffer);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D,aTexture, 0);	
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);
	glFlush();

	gTextureList[aNewTex].mFrameBuffer=(int)aFrameBuffer;
	gTextureList[aNewTex].mRendererBits=gTextureList[aNewTex].mBits;
	gTextureList[aNewTex].mBits=NULL;
	return aNewTex;

}
*/

/*
void Graphics_Core::SetRenderTarget(int theTexture)
{
	//
	// Command the OS to render to gTextureList[theTexture] instead of wherever
	//
	// #ifdef _PORT_RENDERTO
	// !ERROR
	// #endif
	//

	static int aHoldViewport[4];

	if (theTexture==-1)
	{
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);
		if (gIsRenderToTexture) glViewport(aHoldViewport[0],aHoldViewport[1],aHoldViewport[2],aHoldViewport[3]);
		gIsRenderToTexture=false;
	}
	else 
	{
		if (!gIsRenderToTexture) glGetIntegerv(GL_VIEWPORT,aHoldViewport);
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,gTextureList[theTexture].mFrameBuffer);
		glViewport(0,0,gTextureList[theTexture].mWidth,gTextureList[theTexture].mHeight);
		gIsRenderToTexture=true;
		gRenderToTextureNumber=theTexture;
	}
	RenderNormal();
}
*/

#ifdef LEGACY_GL
void Graphics_Core::Set2DCamera(int thePageWidth, int thePageHeight)
{
	//
	// Sets up the matrixes easily for 2d drawing...
	// In porting this, you will likely want to add global resolution variables, to track the "real" resolution
	// vs. the page resolution... you will also likely want to add ZDepth variables, if the camera calls for it.
	//
	// #ifdef _PORT_VIEWPORTS
	// !ERROR
	// #endif
	//
	gPageWidth=thePageWidth;
	gPageHeight=thePageHeight;

	gPageWidthMultiplier=(float)gResolutionWidth/(float)gPageWidth;
	gPageHeightMultiplier=(float)gResolutionHeight/(float)gPageHeight;
	gTouchWidthMultiplier=(float)gResolutionWidth/(float)gPageWidth;
	gTouchHeightMultiplier=(float)gResolutionHeight/(float)gPageHeight;

	if (gSDLScreen)
	{
		float aLeft=0;
		float aTop=0;
		float aRight=(float)gPageWidth-.5f;
		float aBottom=(float)gPageHeight-.5f;
		//aRight/=gPageWidthMultiplier;
		//aBottom/=gPageHeightMultiplier;
		float aFar=-max(1.0f,gZDepth);
		float aNear=0.0f;

		float aMatrix[4][4];
		aMatrix[0][0]=(2.0f)/(aRight-aLeft);
		aMatrix[1][0]=0;
		aMatrix[2][0]=0;
		aMatrix[3][0]=(-aRight-aLeft)/(aRight-aLeft);

		aMatrix[0][1]=0;
		aMatrix[1][1]=(2.0f)/(aTop-aBottom);
		aMatrix[2][1]=0;
		aMatrix[3][1]=(-aTop-aBottom)/(aTop-aBottom);
		aMatrix[0][2]=0;
		aMatrix[1][2]=0;
		aMatrix[2][2]=-2/(aFar-aNear);
		aMatrix[3][2]=(-aFar-aNear)/(aFar-aNear);
		aMatrix[0][3]=0;
		aMatrix[1][3]=0;
		aMatrix[2][3]=0;
		aMatrix[3][3]=1.0f;

		SetMatrix(2,&aMatrix);
	}
}
void Graphics_Core::Set2DRenderToCamera(int thePageWidth, int thePageHeight, int theResolutionWidth, int theResolutionHeight)
{
	//
	// Set up matrixes, etc, so that things render in the texture just like they'd render on the screen.
	// The parameters are width/height of texture, then width/height of the app's resolution.  Those are in there
	// because OpenGL treats everything like the main screen (DirectX doesn't).
	//
	// #ifdef _PORT_RENDERTO
	// !ERROR
	// #endif
	//
	if (gSDLScreen)
	{
		float aLeft=0;
		float aTop=(float)thePageHeight;///aMultY;
		float aRight=(float)thePageWidth;///aMultX;
		float aBottom=0;
		float aFar=-_max(1.0f,gZDepth);
		float aNear=0.0f;
        
        float aMatrix[4][4];
		aMatrix[0][0]=(2.0f)/(aRight-aLeft);
		aMatrix[1][0]=0;
		aMatrix[2][0]=0;
		aMatrix[3][0]=(-aRight-aLeft)/(aRight-aLeft);
		aMatrix[0][1]=0;
		aMatrix[1][1]=(2.0f)/(aTop-aBottom);
		aMatrix[2][1]=0;
		aMatrix[3][1]=(-aTop-aBottom)/(aTop-aBottom);
		aMatrix[0][2]=0;
		aMatrix[1][2]=0;
		aMatrix[2][2]=-2/(aFar-aNear);
		aMatrix[3][2]=(-aFar-aNear)/(aFar-aNear);
		aMatrix[0][3]=0;
		aMatrix[1][3]=0;
		aMatrix[2][3]=0;
		aMatrix[3][3]=1.0f;
		SetMatrix(2,&aMatrix);
	}
	gPageWidthMultiplier=(float)theResolutionWidth/(float)gPageWidth;
	gPageHeightMultiplier=(float)theResolutionHeight/(float)gPageHeight;
}

void Graphics_Core::Set3DCamera(float theCameraX, float theCameraY, float theCameraZ, float theLookatX, float theLookatY,float theLookatZ, float theUpVectorX, float theUpVectorY, float theUpVectorZ, float theFOV)
{
	if(gPaused) return;
	float aWorldMatrix[4][4];
	float aViewMatrix[4][4];
	IDENTITYMATRIX(aWorldMatrix);
	//
	// Added this line to make modelview usable...
	//
	//        SetMatrix(1,aWorldMatrix);
	SCALEMATRIX(aWorldMatrix, 1, 1, -1);

	theCameraZ = -theCameraZ;
	theLookatZ = -theLookatZ;
	theUpVectorZ = -theUpVectorZ;

	// We translate the world the opposite direction of the camera (relatively speakng, of course)
	TRANSLATEMATRIX(aWorldMatrix, -theCameraX, -theCameraY, -theCameraZ);

	//
	// Look-at matrix vectors
	//

	// Lookat vector
	float aLookatVectorX = theLookatX - theCameraX;
	float aLookatVectorY = theLookatY - theCameraY;
	float aLookatVectorZ = theLookatZ - theCameraZ;

	// Side vector (UP cross LOOKAT)
	float aSideVectorX = theUpVectorY * aLookatVectorZ - theUpVectorZ * aLookatVectorY;
	float aSideVectorY = theUpVectorZ * aLookatVectorX - theUpVectorX * aLookatVectorZ;
	float aSideVectorZ = theUpVectorX * aLookatVectorY - theUpVectorY * aLookatVectorX;

	// Correct the UP vector (LOOKAT cross SIDE)
	theUpVectorX = aLookatVectorY * aSideVectorZ - aLookatVectorZ * aSideVectorY;
	theUpVectorY = aLookatVectorZ * aSideVectorX - aLookatVectorX * aSideVectorZ;
	theUpVectorZ = aLookatVectorX * aSideVectorY - aLookatVectorY * aSideVectorX;

	// Normalize the lookat vector
	float len = (float) sqrt(aLookatVectorX * aLookatVectorX + aLookatVectorY * aLookatVectorY + aLookatVectorZ * aLookatVectorZ);
	aLookatVectorX /= len;
	aLookatVectorY /= len;
	aLookatVectorZ /= len;

	// Normalize side vector
	len = (float) sqrt(aSideVectorX * aSideVectorX + aSideVectorY * aSideVectorY + aSideVectorZ * aSideVectorZ);
	aSideVectorX /= len;
	aSideVectorY /= len;
	aSideVectorZ /= len;

	// Normalize the up vector
	len = (float) sqrt(theUpVectorX * theUpVectorX + theUpVectorY * theUpVectorY + theUpVectorZ * theUpVectorZ);
	theUpVectorX /= len;
	theUpVectorY /= len;
	theUpVectorZ /= len;

	//
	// The view matrix (look-at)
	//
	aViewMatrix[0][0] = -aSideVectorX;
	aViewMatrix[1][0] = -aSideVectorY;
	aViewMatrix[2][0] = -aSideVectorZ;
	aViewMatrix[3][0] = 0;

	aViewMatrix[0][1] = -theUpVectorX;
	aViewMatrix[1][1] = -theUpVectorY;
	aViewMatrix[2][1] = -theUpVectorZ;
	aViewMatrix[3][1] = 0;

	aViewMatrix[0][2] = aLookatVectorX;
	aViewMatrix[1][2] = aLookatVectorY;
	aViewMatrix[2][2] = aLookatVectorZ;
	aViewMatrix[3][2] = 0;

	aViewMatrix[0][3] = 0;
	aViewMatrix[1][3] = 0;
	aViewMatrix[2][3] = 0;
	aViewMatrix[3][3] = 1;

	// Combine the world and view (GL doesn't support View matrices)
	MULTIPLYMATRIX(aWorldMatrix, aViewMatrix);
	// Commented this out to make modelview usable...
	//SetMatrix(1,&aWorldMatrix);

	//
	// Perspective projection matrix (as per Blinn)
	//
	float aAspect = (float) gPageWidth / (float) gPageHeight;
	float aNear = gZNear;
	float aFar = GetZDepth();

	float aWidth = COS(theFOV / 2.0f);
	float aHeight = COS(theFOV / 2.0f);
	//OS_Core::Printf("!Aspect: %f --> %f,%f   [%d,%d]",aAspect,aWidth,aHeight,gPageWidth,gPageHeight);

	if (aAspect > 1.0) aWidth /= aAspect;
	else aHeight *= aAspect;

	float s = SIN(theFOV / 2.0f);
	float d = 1.0f - aNear / aFar;

	float aMatrix[4][4];
	aMatrix[0][0] = aWidth;
	aMatrix[1][0] = 0;
	aMatrix[2][0] = g3DOffsetX/gPageWidth/2;
	aMatrix[3][0] = 0;
	aMatrix[0][1] = 0;
	aMatrix[1][1] = aHeight;
	aMatrix[2][1] = g3DOffsetY/gPageHeight/2;
	aMatrix[3][1] = 0;
	aMatrix[0][2] = 0;
	aMatrix[1][2] = 0;
	aMatrix[2][2] = s / d;
	aMatrix[3][2] = -(s * aNear / d);
	aMatrix[0][3] = 0;
	aMatrix[1][3] = 0;
	aMatrix[2][3] = s;
	aMatrix[3][3] = 0;
	//
	// The next two lines combine the matrix to make modelview usable
	//
	SetMatrix(1, &aWorldMatrix);
	SetMatrix(2, &aMatrix);
}

void Graphics_Core::Set3DCameraOffset(float theX, float theY)
{
	g3DOffsetX=theX;
	g3DOffsetY=theY;
}

void Graphics_Core::Set3DRenderToCamera(float theCameraX, float theCameraY, float theCameraZ,float theLookatX, float theLookatY, float theLookatZ,float theUpVectorX, float theUpVectorY, float theUpVectorZ,float theFOV,int thePageWidth, int thePageHeight, int theResolutionWidth, int theResolutionHeight)
{
}

#endif

void Graphics_Core::WantZBuffer(float theZNear, float theZFar)
{
	gZNear=theZNear;
	gZDepth=theZFar;
}

void Graphics_Core::WantStencilBuffer(int theStencilBits)
{
	gStencilSize=theStencilBits;
}


float Graphics_Core::GetZDepth()
{
	return gZDepth;
}

void Graphics_Core::Screenshot()
{
	HWND aWindow=*(HWND*)OS_Core::Query("HWND");

	unsigned char * aPixels = new unsigned char[gResolutionWidth*gResolutionHeight*4]; // 4 bytes for RGBA
	unsigned char * aPixels2 = new unsigned char[gResolutionWidth*gResolutionHeight*4];
	glReadPixels(0,0,gResolutionWidth,gResolutionHeight, GL_BGRA, GL_UNSIGNED_BYTE, aPixels);

	unsigned char* aP1Ptr=&aPixels[(gResolutionHeight-1)*(gResolutionWidth*4)];
	unsigned char* aP2Ptr=aPixels2;
	for (int aCount=0;aCount<gResolutionHeight;aCount++)
	{
		memcpy(aP2Ptr,aP1Ptr,(gResolutionWidth*4));
		aP2Ptr+=(gResolutionWidth*4);
		aP1Ptr-=(gResolutionWidth*4);
	}
	
	HBITMAP aBitmap=CreateCompatibleBitmap(GetDC(aWindow),gResolutionWidth,gResolutionHeight);
	SetBitmapBits(aBitmap,gResolutionWidth*gResolutionHeight*4,aPixels2);

	if (!::OpenClipboard(NULL)) OS_Core::Printf("!OpenClipboard failed");
	if (!::EmptyClipboard()) OS_Core::Printf("!EmptyClipboard failed");
	HANDLE aPaste=::SetClipboardData(CF_BITMAP,aBitmap);
	if (aPaste==NULL) OS_Core::Printf("!SetClipboardData failed");
	::CloseClipboard();

	DeleteObject(aBitmap);
	delete [] aPixels;
	delete [] aPixels2;

}

void* Graphics_Core::ScreenshotToMemory(int* theWidth, int* theHeight)
{
#ifdef _PORT_FINAL
	!ERROR
#endif
	return NULL;
}

bool Graphics_Core::IsTimeBeforeVSync(unsigned int lastDrawFinished, unsigned int lastDrawDuration)
{
	//
	// On computers, we just go ahead and let this go...
	//
	return true;

	//
	// For iOS
	//
	//http://www.ananseproductions.com/game-loops-on-ios/
	//

}


void Graphics_Core::ThreadLock()
{
	if (!gThreadsEnabled) EnableThreadGraphics();
	EnterCriticalSection(&gThreadLock);
}

void Graphics_Core::ThreadUnlock()
{
	LeaveCriticalSection(&gThreadLock);
}

void Graphics_Core::ImageLoadThreadLock()
{
	if (!gThreadsEnabled) EnableThreadGraphics();
	EnterCriticalSection(&gImageLoadThreadLock);
}

void Graphics_Core::ImageLoadThreadUnlock()
{
	LeaveCriticalSection(&gImageLoadThreadLock);
}

void  Graphics_Core::EnableThreadGraphics()
{
	if (!gThreadsEnabled) 
	{
		InitializeCriticalSection(&gThreadLock);
		InitializeCriticalSection(&gImageLoadThreadLock);
		gThreadsEnabled=true;
	}

	//
	// This doesn't work.  We have the GL threading problem, but no apparent way on windows to share
	// the context?
	//
	/*
	ThreadLock();
	SDL_GL_MakeCurrent(gWindow,gContext);
	ThreadUnlock();
	*/
}

unsigned int Graphics_Core::GetVRAM()
{
	return (unsigned int)SDL_GetSystemRAM();
}

#define STARTSWITH(theString,compare) (_strnicmp(theQuery,compare,strlen(compare))==0)

void* Graphics_Core::QueryFake(char* theQuery)
{

	// 1420x800 is a good general purpose "phone aspect/widescreen"

	//1136x640
	if (STARTSWITH(theQuery,"fakephone:iPod")) 
	{
		gFakeDevice=FAKE_PHONE;
		gFakeSmallScreen=true;
		gFakeDeviceResolutionX=1136/2;
		gFakeDeviceResolutionY=640/2;
	}
	if (STARTSWITH(theQuery,"fakephone:iPhone")) 
	{
		gFakeDevice=FAKE_PHONE;
		gFakeSmallScreen=true;
		gFakeDeviceResolutionX=736;
		gFakeDeviceResolutionY=414;
	}
	if (STARTSWITH(theQuery,"fakephone:iPhoneX"))
	{
		gFakeDevice=FAKE_PHONE;
		gFakeSmallScreen=true;
		gFakeDeviceResolutionX=896;
		gFakeDeviceResolutionY=414;
	}
	if (STARTSWITH(theQuery,"fakephone:iPadPro")) 
	{
		gFakeDevice=FAKE_PHONE;
		gFakeSmallScreen=false;
		gFakeDeviceResolutionX=1366/2;
		gFakeDeviceResolutionY=1024/2;
	}
	if (STARTSWITH(theQuery,"fakeweb:crazygamesbig"))
	{
		gFakeDevice=FAKE_WEB;
		gFakeSmallScreen=true;
		gFakeDeviceResolutionX=1108;
		gFakeDeviceResolutionY=503;
	}
	if (STARTSWITH(theQuery,"fakeweb:crazygamessmall"))
	{
		gFakeDevice=FAKE_WEB;
		gFakeSmallScreen=true;
		gFakeDeviceResolutionX=922;
		gFakeDeviceResolutionY=487;
	}


	std::string aWorkStr=theQuery;
	if (aWorkStr.find("+notch")!=std::string::npos)
	{
		OS_Core::gReserveTopScreen=44;
		OS_Core::gReserveBottomScreen=34;
	}
	if (aWorkStr.find("+portrait")!=std::string::npos)
	{
		int aHold=gFakeDeviceResolutionX;
		gFakeDeviceResolutionX=gFakeDeviceResolutionY;
		gFakeDeviceResolutionY=aHold;
	}


	/*// iPhone 4
	#define PRETEND_PHONE IPoint(736,414)
	inline bool IsSmallScreen() {return true;}
	/**/
	/*// iPhone 6.5 in
	#define PRETEND_PHONE IPoint(1344,621)
	inline bool IsSmallScreen() {return true;}
	/**/
	/*// iPhone 5.5 in
	#define PRETEND_PHONE IPoint(1104,621)
	inline bool IsSmallScreen() {return true;}
	/**/
	/*// iPad Pro
	#define PRETEND_PHONE IPoint(1366,1024)
	inline bool	IsSmallScreen() {return false;}
	/**/

	return NULL;
}

bool Graphics_Core::IsSmallScreen() {return gFakeSmallScreen;}
bool Graphics_Core::IsFakePhone() {return ((gFakeDevice&FAKE_PHONE)!=0);}
bool Graphics_Core::IsFakeWeb() {return ((gFakeDevice&FAKE_WEB)!=0);}
bool Graphics_Core::IsFakeDevice() {return (gFakeDevice!=0);}
int	Graphics_Core::GetFakeDeviceResolutionX() {return gFakeDeviceResolutionX;}
int	Graphics_Core::GetFakeDeviceResolutionY() {return gFakeDeviceResolutionY;}

