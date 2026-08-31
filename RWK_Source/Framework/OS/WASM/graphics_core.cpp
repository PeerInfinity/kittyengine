#ifndef GL30
#include "legacy_graphics_core.h"
#else
#include "graphics_core.h"
#include "os_core.h"

#define GL_GLEXT_PROTOTYPES
#define GL_FUNC_ADD                       0x8006

#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <SDL2/SDL.h>
//#include <sdl_image.h>
#include <SDL2/SDL_opengl.h>
//#include <sdl_image.h>
#include <GLES/gl.h>
#include <GLES/glext.h>
#include <emscripten.h>
#include <png.h>

#define __HEADER
#include "common.h"
#undef __HEADER


//#define _GLEW
//#include "GL/glew.h"


#define GL_RENDERMODE GL_TRIANGLES

#define IDENTITYMATRIX(theMatrix) {theMatrix[0][0]=1.0f;theMatrix[0][1]=0.0f;theMatrix[0][2]=0.0f;theMatrix[0][3]=0.0f;theMatrix[1][0]=0.0f;theMatrix[1][1]=1.0f;theMatrix[1][2]=0.0f;theMatrix[1][3]=0.0f;theMatrix[2][0]=0.0f;theMatrix[2][1]=0.0f;theMatrix[2][2]=1.0f;theMatrix[2][3]=0.0f;theMatrix[3][0]=0.0f;theMatrix[3][1]=0.0f;theMatrix[3][2]=0.0f;theMatrix[3][3]=1.0f;}
#define TRANSLATEMATRIX(theMatrix,theX,theY,theZ) {theMatrix[3][0]+=theX;theMatrix[3][1]+=theY;theMatrix[3][2]+=theZ;}
#define MULTIPLYMATRIX(theMatrix, theMultiplyBy) {float a_TempMultiply[4][4];memcpy(&a_TempMultiply,&theMatrix,sizeof(a_TempMultiply));theMatrix[0][0]=a_TempMultiply[0][0]*theMultiplyBy[0][0]+a_TempMultiply[0][1]*theMultiplyBy[1][0]+a_TempMultiply[0][2]*theMultiplyBy[2][0]+a_TempMultiply[0][3]*theMultiplyBy[3][0];theMatrix[0][1]=a_TempMultiply[0][0]*theMultiplyBy[0][1]+a_TempMultiply[0][1]*theMultiplyBy[1][1]+a_TempMultiply[0][2]*theMultiplyBy[2][1]+a_TempMultiply[0][3]*theMultiplyBy[3][1];theMatrix[0][2]=a_TempMultiply[0][0]*theMultiplyBy[0][2]+a_TempMultiply[0][1]*theMultiplyBy[1][2]+a_TempMultiply[0][2]*theMultiplyBy[2][2]+a_TempMultiply[0][3]*theMultiplyBy[3][2];theMatrix[0][3]=a_TempMultiply[0][0]*theMultiplyBy[0][3]+a_TempMultiply[0][1]*theMultiplyBy[1][3]+a_TempMultiply[0][2]*theMultiplyBy[2][3]+a_TempMultiply[0][3]*theMultiplyBy[3][3];theMatrix[1][0]=a_TempMultiply[1][0]*theMultiplyBy[0][0]+a_TempMultiply[1][1]*theMultiplyBy[1][0]+a_TempMultiply[1][2]*theMultiplyBy[2][0]+a_TempMultiply[1][3]*theMultiplyBy[3][0];theMatrix[1][1]=a_TempMultiply[1][0]*theMultiplyBy[0][1]+a_TempMultiply[1][1]*theMultiplyBy[1][1]+a_TempMultiply[1][2]*theMultiplyBy[2][1]+a_TempMultiply[1][3]*theMultiplyBy[3][1];theMatrix[1][2]=a_TempMultiply[1][0]*theMultiplyBy[0][2]+a_TempMultiply[1][1]*theMultiplyBy[1][2]+a_TempMultiply[1][2]*theMultiplyBy[2][2]+a_TempMultiply[1][3]*theMultiplyBy[3][2];theMatrix[1][3]=a_TempMultiply[1][0]*theMultiplyBy[0][3]+a_TempMultiply[1][1]*theMultiplyBy[1][3]+a_TempMultiply[1][2]*theMultiplyBy[2][3]+a_TempMultiply[1][3]*theMultiplyBy[3][3];theMatrix[2][0]=a_TempMultiply[2][0]*theMultiplyBy[0][0]+a_TempMultiply[2][1]*theMultiplyBy[1][0]+a_TempMultiply[2][2]*theMultiplyBy[2][0]+a_TempMultiply[2][3]*theMultiplyBy[3][0];theMatrix[2][1]=a_TempMultiply[2][0]*theMultiplyBy[0][1]+a_TempMultiply[2][1]*theMultiplyBy[1][1]+a_TempMultiply[2][2]*theMultiplyBy[2][1]+a_TempMultiply[2][3]*theMultiplyBy[3][1];theMatrix[2][2]=a_TempMultiply[2][0]*theMultiplyBy[0][2]+a_TempMultiply[2][1]*theMultiplyBy[1][2]+a_TempMultiply[2][2]*theMultiplyBy[2][2]+a_TempMultiply[2][3]*theMultiplyBy[3][2];theMatrix[2][3]=a_TempMultiply[2][0]*theMultiplyBy[0][3]+a_TempMultiply[2][1]*theMultiplyBy[1][3]+a_TempMultiply[2][2]*theMultiplyBy[2][3]+a_TempMultiply[2][3]*theMultiplyBy[3][3];theMatrix[3][0]=a_TempMultiply[3][0]*theMultiplyBy[0][0]+a_TempMultiply[3][1]*theMultiplyBy[1][0]+a_TempMultiply[3][2]*theMultiplyBy[2][0]+a_TempMultiply[3][3]*theMultiplyBy[3][0];theMatrix[3][1]=a_TempMultiply[3][0]*theMultiplyBy[0][1]+a_TempMultiply[3][1]*theMultiplyBy[1][1]+a_TempMultiply[3][2]*theMultiplyBy[2][1]+a_TempMultiply[3][3]*theMultiplyBy[3][1];theMatrix[3][2]=a_TempMultiply[3][0]*theMultiplyBy[0][2]+a_TempMultiply[3][1]*theMultiplyBy[1][2]+a_TempMultiply[3][2]*theMultiplyBy[2][2]+a_TempMultiply[3][3]*theMultiplyBy[3][2];theMatrix[3][3]=a_TempMultiply[3][0]*theMultiplyBy[0][3]+a_TempMultiply[3][1]*theMultiplyBy[1][3]+a_TempMultiply[3][2]*theMultiplyBy[2][3]+a_TempMultiply[3][3]*theMultiplyBy[3][3];}
#define SCALEMATRIX(theMatrix, theX,theY,theZ) {float a_TempScale[4][4];IDENTITYMATRIX(a_TempScale);a_TempScale[0][0]=theX;a_TempScale[1][1]=theY;a_TempScale[2][2]=theZ;MULTIPLYMATRIX(theMatrix,a_TempScale);}
#define RAD(theAngle) (theAngle*3.14159f/180.0f)
#define SIN(theAngle) ((float)sin(RAD(theAngle)))
#define COS(theAngle) (-(float)cos(RAD(theAngle)))

//#define XGL(x) x;CheckGLError();
//#define XGLLog(...) 

/*
#undef XGL
#undef XGLLog
int gErrorCount=0;
GLenum gLogError=GL_NO_ERROR;
#define XGL(x) {x;if (gErrorCount<100) {gLogError=glGetError();if (gLogError!=GL_NO_ERROR) {gErrorCount++;OS_Core::Printf(#x" error: %s",glErrorToString(gLogError));}}}
#define XGLLog(...) if (gErrorCount<100) OS_Core::Printf(__VA_ARGS__)
/**/

/*
#define ClearGLError() {GLenum err ( glGetError() );while ( err != GL_NO_ERROR )err = glGetError();}
#define CheckGLError() _CheckGLError(__FILE__, __LINE__)
//#define ClearGLError();
//#define CheckGLError()
void _CheckGLError(const char* file, int line)
{
    GLenum err ( glGetError() );

    while ( err != GL_NO_ERROR )
    {
        char* error;
        switch ( err )
        {
            case GL_INVALID_OPERATION:  error="INVALID_OPERATION";      break;
            case GL_INVALID_ENUM:       error="INVALID_ENUM";           break;
            case GL_INVALID_VALUE:      error="INVALID_VALUE";          break;
            case GL_OUT_OF_MEMORY:      error="OUT_OF_MEMORY";          break;
			default:					error="UNKNOWN";				break;
            //case GL_INVALID_FRAMEBUFFER_OPERATION:  error="INVALID_FRAMEBUFFER_OPERATION";  break;
        }
		OS_Core::Printf("GL_%s - %s : %d",error,file,line);
        err = glGetError();
    }

    return;
}
*/

namespace Graphics_Core
{
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
	
static SDL_Surface			*gSDLScreen=NULL;
SDL_Renderer*				gRenderer=NULL;
SDL_Window*					gWindow=NULL;
SDL_GLContext				gContext;
bool 						gGraphicsPaused = false;
//GLint 						gFilter=GL_LINEAR;

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
float 						g3DOffsetX=0.0f;
float 						g3DOffsetY=0.0f;


float                       gZNear=0.0f;
float						gZDepth=0.0f;
bool						gZBuffer=false;
bool						gReverseRGB=true;		// This will depend on platform! MACFIX

int							gStencilSize=0;
GLuint						gStencilValue=1;


//GLint                       gSaturationShader=0;
//GLint                       gSaturation_UniformLocation;
//bool						gIsRenderToTexture=false;
//int                         gRenderToTextureNumber=0;
};

/*
void MakePixelShaders()
{
}
*/


char * glErrorToString(int theError) 
{
	switch (theError) 
	{
		case GL_NO_ERROR:return "GL_NO_ERROR";
		case GL_INVALID_ENUM:return "GL_INVALID_ENUM";
		case GL_INVALID_VALUE:return "GL_INVALID_VALUE";
		case GL_INVALID_OPERATION:return "GL_INVALID_OPERATION";
		case GL_STACK_OVERFLOW:return "GL_STACK_OVERFLOW";
		case GL_STACK_UNDERFLOW:return "GL_STACK_UNDERFLOW";
		case GL_OUT_OF_MEMORY:return "GL_OUT_OF_MEMORY";
		default:return "Unknown Error";
	}
}

#ifdef GL30
#include "gl30.h"
#else
#include "gl11.h"
#endif

void* Graphics_Core::Query(longlong theHash, ...)
{
	va_list aArgs;
	va_start(aArgs,theHash);
	switch(theHash)
	{
		case HASH8("REHUPREZ"):
		{
			gPageWidth=va_arg(aArgs, int);
			gPageHeight=va_arg(aArgs, int);
			gResolutionWidth=va_arg(aArgs, int);
			gResolutionHeight=va_arg(aArgs, int);

			gPageWidthMultiplier=(float)gResolutionWidth/(float)gPageWidth;
			gPageHeightMultiplier=(float)gResolutionHeight/(float)gPageHeight;
			gTouchWidthMultiplier=(float)gResolutionWidth/(float)gPageWidth;
			gTouchHeightMultiplier=(float)gResolutionHeight/(float)gPageHeight;

			break;
		}
	}
	return NULL;
}

void Graphics_Core::PreStartup()
{
	// The following fails on WASM with "context attributes are not supported"
	/*
#ifndef LEGACY_GL
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 2 );
#else
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 1 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 2 );
#endif
	*/

	//
	// Everything commented here will cause SDL to fail to create the context
	//
	//SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
	//SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 2 );
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,SDL_GL_CONTEXT_PROFILE_CORE); // Fails it
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,1); //Fails it
	//SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1); // Fails it
	//
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE,8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,8);

	//
	// If I uncomment these, I crash.  I think these were put in for Hamsterball reasons, tho, when I was fiddling around with
	// the bad Z-Buffer.
	//
	//if (Graphics_Core::gZDepth>0) {SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,16);glDepthFunc(GL_LEQUAL);}
	//if (Graphics_Core::gStencilSize>0) SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE,gStencilSize);
}


void Graphics_Core::Startup_SDL()
{
	GetDesktopSize(&gDesktopWidth,&gDesktopHeight);
	
	//IMG_Init(IMG_INIT_PNG|IMG_INIT_JPG);
	gWindow=(SDL_Window*)OS_Core::Query("WINDOW");

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE,8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,8);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,1);
	if (gZDepth>0) SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,16);


	gContext=SDL_GL_CreateContext(gWindow);
	if (gContext==NULL) OS_Core::Error("Could not create OpenGL Context: %s",SDL_GetError());

	gRenderer=SDL_CreateRenderer(gWindow,-1,SDL_RENDERER_ACCELERATED|SDL_RENDERER_TARGETTEXTURE|SDL_RENDERER_PRESENTVSYNC);
	if (gRenderer==NULL) OS_Core::Error("Could not create OpenGL Renderer: %s",SDL_GetError());

	gSDLScreen=SDL_GetWindowSurface(gWindow);
	if (!gSDLScreen) OS_Core::Error("Could not create SDL Screen...");

	int aFlags=0;
	if (gFullscreen) aFlags=SDL_WINDOW_FULLSCREEN_DESKTOP;
	SDL_SetWindowFullscreen(gWindow,aFlags);

	const GLubyte*version=glGetString(GL_VERSION);if(version) {OS_Core::Printf("OpenGL Version: %s",version); } else {OS_Core::Printf("Failed to retrieve OpenGL version."); }

	//
	//#ifdef _PORT_ZBUFFER
	//!ERROR -- You will want to initialize z buffering if gZDepth>0 here...
	//          ...Remember, your OpenGL context must exist first (at least in WebAssembly)
	//#endif
	//

	if (gZDepth>0) glDepthFunc(GL_LEQUAL);



	SetRenderDefaults();
}


void Graphics_Core::Startup()
{
	//
	// Do the initialization you need to get graphics going...
	//
	//#ifdef _PORT_STARTUPGFX
	//!ERROR
	//#endif
	//
	
	// We used to have Startup_SDL in here... moved it because I was experimenting
	// with the SDL window stuff...
	Startup_SDL();

}

void Graphics_Core::Shutdown()
{
	//
	// Do what you need to do to shut down any graphics...
	//
	//#ifdef _PORT_STARTUPGFX
	//!ERROR
	//#endif
	//
	//IMG_Quit();
}

/*
void* Graphics_Core::Query(char* theQuery)
{
	//
	// Just fill out with anything that another core might need to query Graphics_Core for.
	// Resolution?  Page multipliers?  Anything like that.
	//
	//#ifdef _PORT_STARTUPGFX
	//!ERROR
	//#endif
	//
	return NULL;
}
*/

/*
void Graphics_Core::BeginRender()
{
	XGLLog("BeginRender");

	//
	// Do whatever needs to happen prior to a rendering run...
	//
	// #ifdef _PORT_STARTUPGFX
	// !ERROR
	// #endif
	//
	if (gIsRenderToTexture)
	{
	}
	else
	{
		XGL(glViewport(0, 0, gResolutionWidth, gResolutionHeight));
	}
	
	//
	// Strangely, for WebGL if we don't call this code block before the first time we load or set a texture,
	// then we can't ever set the texture matrix-- it just doesn't take.  Why is this?  Some initializer??
	//
	XGL(glMatrixMode(GL_TEXTURE));
	XGL(glLoadIdentity());
}

void Graphics_Core::EndRender()
{
	//
	// Do whatever needs to happen after a rendering run...
	//
	// #ifdef _PORT_STARTUPGFX
	// !ERROR
	// #endif
	//
	
	XGLLog("EndRender");
}
*/

void Graphics_Core::Show()
{
	//
	// Flip the page, or present surface, or whatever...
	//
	//#ifdef _PORT_STARTUPGFX
	//!ERROR
	//#endif
	//
	SDL_GL_SwapWindow(gWindow);
}

/*
void Graphics_Core::Clear(float theRed, float theGreen, float theBlue, float theAlpha)
{
	//
	// Clears the page with just a solid color...
	//
	//#ifdef _PORT_STARTUPGFX
	//!ERROR
	//#endif
	//
	EndRender();
	if (gZDepth > 0)
	{
		XGL(glClearDepthf(1.0f));
		XGL(glClearColor(theRed, theGreen, theBlue, theAlpha));
		XGL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
	}
	else
	{
		XGL(glClearColor(theRed, theGreen, theBlue, theAlpha));
		XGL(glClear(GL_COLOR_BUFFER_BIT));
	}
	BeginRender();

	// #ifdef _PORT_ZBUFFER
	// !! You'll need to clear the ZBuffer too!
	// #endif
	//

}

void Graphics_Core::ClearZ(float theValue)
{
	if(gGraphicsPaused) return;
	//OS_Core::Printf("Graphics_Core::Clear() called");
	EndRender();
	if (gZDepth>0)
	{
		XGL(glClearDepthf(theValue));
		XGL(glClear(GL_DEPTH_BUFFER_BIT));
	}
	BeginRender();
}


void Graphics_Core::DrawTestTriangle()
{
	//
	// This draws a very simple anything shape on-screen.  Just use it to make sure things are appearing at all.
	// This function is not a part of the framework, it's just a porting helper, and can be removed after
	// porting is completed.
	//
	//#ifdef _PORT_TESTTRIANGLE
	//!ERROR
	//#endif
	//

	#ifndef _PORT_PRIMITIVES1
	//
	// These render defaults are set when _PORT_PRIMITIVES1 is defined.
	// So if that's not defined, we go ahead and do them here, so that we can see the triangle.
	//
	XGL(glEnableClientState(GL_VERTEX_ARRAY));
	XGL(glEnableClientState(GL_COLOR_ARRAY));
	XGL(glEnableClientState(GL_TEXTURE_COORD_ARRAY));
	#endif
	
	
	struct Vertex2DRef
	{
		Vertex2DRef() {}
		Vertex2DRef(float theX, float theY, float theZ, long theDiffuse, float theTextureU, float theTextureV) {mX=theX;mY=theY;mZ=theZ;mDiffuse=theDiffuse;mTextureU=theTextureU;mTextureV=theTextureV;}
		float			mX;
		float			mY;
		float			mZ;
		unsigned int	mDiffuse;
		float			mTextureU;
		float			mTextureV;
	};
	
	Vertex2DRef aVArray[3];
	aVArray[0]=Vertex2DRef(0,0,0,0xFF0000FF,0,0);
	aVArray[1]=Vertex2DRef(0,1,0,0x00FF00FF,0,0);
	aVArray[2]=Vertex2DRef(1,1,0,0x0000FFFF,0,0);
	
	aVArray[0]=Vertex2DRef(0,0,0,0xFF0000FF,0,0);
	aVArray[1]=Vertex2DRef(0,512,0,0x00FF00FF,0,1);
	aVArray[2]=Vertex2DRef(512,512,0,0x0000FFFF,1,1);
	int aVertexStride=sizeof(Vertex2DRef);
	
	Vertex2DRef *aRef=(Vertex2DRef*)aVArray;
	XGL(glVertexPointer(3,GL_FLOAT,aVertexStride,&aRef->mX));
	XGL(glColorPointer(4,GL_UNSIGNED_BYTE,aVertexStride,&aRef->mDiffuse));
	XGL(glTexCoordPointer(2,GL_FLOAT,aVertexStride,&aRef->mTextureU));
	XGL(glDrawArrays(GL_TRIANGLES,0,3));
}

namespace Graphics_Core
{
	float gWorldMatrix[4][4];
	float gViewMatrix[4][4];
	float gWorkMatrix[4][4];
}

void Graphics_Core::SetMatrix(int theType, void *theMatrixPtr)
{
	//
	// Set one of the system matrices (theType -> 0=world,1=view,2=perspective) with data...
	//
	//#ifdef _PORT_VIEWPORTS
	//!ERROR
	//#endif
	//
	// Set the texture matrixes too... (theType -> 3,4) for texture matrix (Multitexture)
	//
	//#ifdef _PORT_TEXMATRIX
	//!ERROR
	//#endif
	//

	switch (theType)
	{
		case 0:
            memcpy(gWorldMatrix,theMatrixPtr,4*4*sizeof(float));
            
            memcpy(gWorkMatrix,gWorldMatrix,4*4*sizeof(float));
            MULTIPLYMATRIX(gWorkMatrix,gViewMatrix);
			XGL(glMatrixMode(GL_MODELVIEW));
			XGL(glLoadIdentity());
			XGL(glLoadMatrixf((float*)gWorkMatrix));
            break;
		case 1:
            memcpy(gViewMatrix,theMatrixPtr,4*4*sizeof(float));
            memcpy(gWorkMatrix,gWorldMatrix,4*4*sizeof(float));
            MULTIPLYMATRIX(gWorkMatrix,gViewMatrix);
			XGL(glMatrixMode(GL_MODELVIEW));
			XGL(glLoadIdentity());
			XGL(glLoadMatrixf((float*)gWorkMatrix));
			break;
		case 2:
			XGL(glMatrixMode(GL_PROJECTION));
			XGL(glLoadIdentity());
			XGL(glLoadMatrixf((float*)theMatrixPtr));
			break;
		case 3:
			XGL(glActiveTexture(GL_TEXTURE0));
			XGL(glMatrixMode(GL_TEXTURE));
			XGL(glLoadIdentity());
            if (theMatrixPtr) XGL(glLoadMatrixf((float*)theMatrixPtr));
			break;
		//
		// This is an INCORRECT and BAD implementation of multitexturing, because it makes the current active texture be Texture1,
		// which seems to turn OFF texture 0 I guess?
		//
		case 4:
			break;
			XGL(glActiveTexture(GL_TEXTURE1));
			XGL(glMatrixMode(GL_TEXTURE));
			XGL(glLoadIdentity());
			if (theMatrixPtr) XGL(glLoadMatrixf((float*)theMatrixPtr));
			break;
	}
}

void Graphics_Core::GetMatrix(int theType, void *theMatrixPtr)
{
	//
	//#ifdef _PORT_VIEWPORTS
	//!ERROR
	//#endif
	//
	switch (theType)
	{
	case 0:memcpy(theMatrixPtr,gWorldMatrix,4*4*sizeof(float));break;
	case 1:memcpy(theMatrixPtr,gViewMatrix,4*4*sizeof(float));break;
	case 2:glGetFloatv(GL_PROJECTION_MATRIX,(float*)theMatrixPtr);break;
	case 3:glActiveTexture(GL_TEXTURE0);glGetFloatv(GL_TEXTURE_MATRIX,(float*)theMatrixPtr);break;
	case 4:glActiveTexture(GL_TEXTURE1);glGetFloatv(GL_TEXTURE_MATRIX,(float*)theMatrixPtr);break;
	}
}
*/
/*
void Graphics_Core::Set2DCamera(int thePageWidth, int thePageHeight)
{
	//
	// Sets up the matrixes easily for 2d drawing...
	// In porting this, you will likely want to add global resolution variables, to track the "real" resolution
	// vs. the page resolution... you will also likely want to add ZDepth variables, if the camera calls for it.
	//
	//#ifdef _PORT_VIEWPORTS
	//!ERROR
	//#endif
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
}
*/
/*
void Graphics_Core::GetViewport(float* theViewport)
{
	//
	// Just gets a copy of the viewport into float[6],
	// which is interpreted into this struct:
	//		typedef struct Viewport
	//		{
	//			float			mX;
	//			float			mY;
	//			float			mWidth;
	//			float			mHeight;
	//			float			mMinZ;
	//			float			mMaxZ;
	//		} Viewport;
	//
	//#ifdef _PORT_VIEWPORTS
	//!ERROR
	//#endif
	//
	GLint aViewport[4];
	glGetIntegerv(GL_VIEWPORT,aViewport);

	*theViewport=(float)aViewport[0];theViewport++;
	*theViewport=(float)aViewport[1];theViewport++;
	*theViewport=(float)aViewport[2];theViewport++;
	*theViewport=(float)aViewport[3];theViewport++;

	GLfloat aDepth[2];
	glGetFloatv(GL_DEPTH_RANGE,aDepth);
	*theViewport=aDepth[0];theViewport++;
	*theViewport=aDepth[1];theViewport++;
}

void Graphics_Core::SetRenderDefaults()
{
	//
	//#ifdef _PORT_PRIMITIVES1
	//!ERROR
	//#endif
	//
	
	XGL(glEnable(GL_BLEND));
	XGL(glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA));
	XGL(glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE));
	XGL(glDisable(GL_LIGHTING));
	XGL(glShadeModel(GL_SMOOTH));
	XGL(glDisable(GL_DEPTH_TEST));
	XGL(glDisable(GL_CULL_FACE));
	XGL(glEnableClientState(GL_VERTEX_ARRAY));
	XGL(glEnableClientState(GL_COLOR_ARRAY));
	XGL(glEnableClientState(GL_TEXTURE_COORD_ARRAY));
	

	SetAlphaThreshold(.01f);
	DisableZBuffer();
	RenderNormal();
	RenderWhite(false);
	RenderSaturation(1.0f);
	SetTexture();
}

void Graphics_Core::CullNone() 
{
	//
	// Want to set things up for no culling, initially... so no porting "errors" are due to hidden faces.
	//
	//#ifdef _PORT_PRIMITIVES1
	//!ERROR
	//#endif
	//
	XGL(glDisable(GL_CULL_FACE));
}



namespace Graphics_Core
{
	bool gBoundBuffer=false; // If we're drawing from a shape buffer on the video card, it's "bound" and needs to be unbound in order to draw manually... this tells us.
}
void Graphics_Core::DrawShape(void *theBuffer, int theTriangleCount, int theVertexStride)
{
	//
	// Draw an indexed shape out of a buffer... the buffer should be in this format:
	//		struct Vertex2DRef
	//		{
	//			float			mX;
	//			float			mY;
	//			float			mZ;
	//			unsigned int 	mDiffuse;
	//			float			mTextureU;
	//			float			mTextureV;
	//		}
	//
	//#ifdef _PORT_PRIMITIVES1
	//!ERROR
	//#endif
	//
	struct Vertex2DRef
	{
		float			mX;
		float			mY;
		float			mZ;
		unsigned int	mDiffuse;
		float			mTextureU;
		float			mTextureV;
	};

	if (gBoundBuffer)
	{
		XGL(glBindBuffer(GL_ARRAY_BUFFER,NULL));
		XGL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,NULL));
		gBoundBuffer=false;
	}

	Vertex2DRef *aRef=(Vertex2DRef*)theBuffer;
	XGL(glVertexPointer(3,GL_FLOAT,theVertexStride,&aRef->mX));
	XGL(glColorPointer(4,GL_UNSIGNED_BYTE,theVertexStride,&aRef->mDiffuse));
	XGL(glTexCoordPointer(2,GL_FLOAT,theVertexStride,&aRef->mTextureU));
	XGL(glDrawArrays(GL_TRIANGLES,0,theTriangleCount*3));
}


void Graphics_Core::SetAlphaThreshold(float theValue)
{
	//#ifdef _PORT_3DEX
	//!ERROR
	//#endif
	
	if (theValue>0)
	{
		XGL(glAlphaFunc(GL_GREATER,theValue));
		XGL(glEnable(GL_ALPHA_TEST));
	}
	else
	{
		XGL(glDisable(GL_ALPHA_TEST));
	}
	

}
*/


bool Graphics_Core::IsZoomed()
{
	//
	// Simply return true if the page is stretched at all (i.e. if resolution!=page resolution).
	// This might become obsolete if I move all this out of the core...
	//
	//#ifdef _PORT_FINAL
	//!ERROR
	 //#endif
	//
	
    if (gPageHeightMultiplier!=1.0f || gPageWidthMultiplier!=1.0f) return true;
    return false;
	
}

/*
void Graphics_Core::DrawIndexedShape(void *theVertexBuffer, void* theIndexBuffer, int theVertexCount, int theTriangleCount, int theVertexStride)
{
	//
	// Draw an indexed shape out of a buffer... the buffer should be in this format:
	//		struct Vertex2DRef
	//		{
	//			float			mX;
	//			float			mY;
	//			float			mZ;
	//			unsigned int	mDiffuse;
	//			float			mTextureU;
	//			float			mTextureV;
	//		}
	// And the indicia will be an array of shorts.
	//
	//
	//#ifdef _PORT_PRIMITIVES2
	//!ERROR
	//#endif
	//
	struct Vertex2DRef
	{
		float			mX;
		float			mY;
		float			mZ;
		unsigned int	mDiffuse;
		float			mTextureU;
		float			mTextureV;
	};

	if (gBoundBuffer)
	{
		XGL(glBindBuffer(GL_ARRAY_BUFFER,NULL));
		XGL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,NULL));
		gBoundBuffer=false;
	}

	Vertex2DRef *aRef=(Vertex2DRef*)theVertexBuffer;
	XGL(glVertexPointer(3,GL_FLOAT,theVertexStride,&aRef->mX));
	XGL(glColorPointer(4,GL_UNSIGNED_BYTE,theVertexStride,&aRef->mDiffuse));
	XGL(glTexCoordPointer(2,GL_FLOAT,theVertexStride,&aRef->mTextureU));
	
	XGL(glDrawElements(GL_TRIANGLES,theTriangleCount*3,GL_UNSIGNED_SHORT,theIndexBuffer));
}

void Graphics_Core::Clip(float theX1, float theY1, float theX2, float theY2)
{
	//
	// Clip drawing to the specified screen area...
	//
	//#ifdef _PORT_CLIPPING
	//!ERROR
	//#endif
	//

	//
	//#ifdef _PORT_RENDERTO
	//!ERROR - Clipping usually needs to branch if you're rendering to texture...
	//#endif
	//

	
	GLint aLeft,aTop,aWidth,aHeight;

	if (!gIsRenderToTexture)
	{
		aLeft=(int)(theX1*gPageWidthMultiplier);
		aWidth=(int)(theX2*gPageWidthMultiplier)-aLeft;
		aTop=gResolutionHeight-(int)(theY2*gPageHeightMultiplier);
		aHeight=(gResolutionHeight-(int)(theY1*gPageHeightMultiplier))-aTop;
	}
	else
	{
		aLeft=(int)theX1;
		aWidth=((int)theX2)-aLeft;
		aTop=(int)theY1;
		aHeight=((int)theY2)-aTop;
	}

	XGL(glEnable(GL_SCISSOR_TEST));
	XGL(glScissor(aLeft,aTop,aWidth,aHeight));
	
}

void Graphics_Core::Clip()
{
	//
	// Don't clip anything (you can just disable all clipping here if that's an option)
	//
	//#ifdef _PORT_CLIPPING
	//!ERROR
	//#endif
	//
	XGL(glDisable(GL_SCISSOR_TEST));
}
*/

bool Graphics_Core::LoadImage_PNG(char *theFilename, unsigned char **theBits, int *theWidth, int *theHeight)
{
	_FixPath(theFilename);
	//
	// Load a PNG's stream into "thebits."  Most mobile OS's do this natively!
	//
	//#ifdef _PORT_LOADPNG
	//!ERROR
	//#endif
	//
	*theWidth=0;
	*theHeight=0;
	*theBits=NULL;
	
	unsigned char aHeader[8];    // 8 is the maximum size that can be checked
	FILE *aFP = fopen(theFilename, "rb");
	if (!aFP) {OS_Core::Printf("Could not find PNG file %s",theFilename);return false;}
	fread(aHeader,1,8,aFP);
	if (png_sig_cmp(aHeader, 0, 8)) {OS_Core::Printf("File %s is not a PNG file",theFilename);return false;}
	png_structp aPNGPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!aPNGPtr) {OS_Core::Printf("File %s: png_create_read_struct failed.",theFilename);return false;}

	png_infop aInfoPtr = png_create_info_struct(aPNGPtr);
	if (!aInfoPtr) {OS_Core::Printf("File %s: png_create_info_struct failed.",theFilename);return false;}
	if (setjmp(png_jmpbuf(aPNGPtr))) {OS_Core::Printf("File %s: setjmp failed.",theFilename);return false;}
	png_init_io(aPNGPtr,aFP);
	png_set_sig_bytes(aPNGPtr,8);
	png_read_info(aPNGPtr,aInfoPtr);
	int aWidth=png_get_image_width(aPNGPtr,aInfoPtr);
	int aHeight=png_get_image_height(aPNGPtr,aInfoPtr);
	int aColorType=png_get_color_type(aPNGPtr,aInfoPtr);
	int aBitDepth=png_get_bit_depth(aPNGPtr,aInfoPtr);
	int aNumberOfPasses=png_set_interlace_handling(aPNGPtr);
	png_read_update_info(aPNGPtr,aInfoPtr);
	if (setjmp(png_jmpbuf(aPNGPtr))) {OS_Core::Printf("File %s: error reading.",theFilename);return false;}
	int aNumBytes=png_get_rowbytes(aPNGPtr,aInfoPtr)*aHeight;
	unsigned int *aBits=new unsigned int[aNumBytes];
	for (unsigned int aCount=0;aCount<aHeight;aCount++) {png_bytep anAddr=(png_bytep)&aBits[aCount*aWidth];png_read_rows(aPNGPtr,(png_bytepp)&anAddr,NULL,1);}
	png_read_end(aPNGPtr,aInfoPtr);
	png_destroy_read_struct(&aPNGPtr,&aInfoPtr,(png_infopp)NULL);
	fclose(aFP);	
	*theWidth=aWidth;
	*theHeight=aHeight;
	*theBits=(unsigned char*)aBits;

	//
	// Have to swap B&A for compatibility here....
	//
	/*
    unsigned char *aBitPtr=*theBits;
    for (int aSpan=0;aSpan<*theWidth*(*theHeight);aSpan++)
    {
        unsigned char aHoldR=*(aBitPtr+0);
        unsigned char aHoldB=*(aBitPtr+2);
        *(aBitPtr+0)=(char)aHoldB;
        *(aBitPtr+2)=(char)aHoldR;
        aBitPtr+=4;
    }
	/**/
	
	
	return true;
	
	
	/*
	SDL_Surface *aSurface=IMG_Load(OS_Core::FixPath(theFilename));
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
	*/
}

bool Graphics_Core::LoadImage_GIF(char *theFilename, unsigned char **theBits, int *theWidth, int *theHeight)
{
	//
	// Load a GIF's stream into "thebits."  Most mobile OS's do this natively!
	//
	//#ifdef _PORT_LOADGIF
	//!ERROR
	//#endif
	//
	return LoadImage_PNG(theFilename,theBits,theWidth,theHeight);
}

extern "C"
{
#define XMD_H
#include <jpeglib.h>
}

bool Graphics_Core::LoadImage_JPG(char *theFilename, unsigned char **theBits, int *theWidth, int *theHeight)
{
	//
	// Load a JPG's stream into "thebits."  Most mobile OS's do this natively!
	//
	_FixPath(theFilename);
	unsigned long aWidth,aHeight;
	unsigned int texture_id;
	unsigned int type;
	unsigned char * rowptr[1];    // pointer to an array
	unsigned char * jdata;        // data for the image
	struct jpeg_decompress_struct aInfo; //for our jpeg info
	struct jpeg_error_mgr err;          //the error handler
	
	

	FILE* aFile = fopen(theFilename, "rb");  //open the file

	aInfo.err = jpeg_std_error(& err);     
	jpeg_create_decompress(&aInfo);   //fills info structure

	//if the jpeg file doesn't load
	if(!aFile) 
	{
		OS_Core::Printf("Error reading JPEG file %s!", theFilename);
		return false;
	}	

	jpeg_stdio_src(&aInfo, aFile);
	jpeg_read_header(&aInfo, TRUE);   // read jpeg file header
	jpeg_start_decompress(&aInfo);    // decompress the file

	int aRowStride=aInfo.output_width*aInfo.output_components;
	unsigned char**aBuffer=(*aInfo.mem->alloc_sarray)((j_common_ptr)&aInfo,JPOOL_IMAGE,aRowStride,1);
	unsigned int* aBits=new unsigned int[aInfo.output_width*aInfo.output_height];
	unsigned int* aQ=aBits;
	while (aInfo.output_scanline < aInfo.output_height)
	{
		jpeg_read_scanlines(&aInfo, aBuffer, 1);
		unsigned char* aPtr = *aBuffer;
		for (unsigned int aCount=0;aCount<aInfo.output_width;aCount++) {int aR = *aPtr++;int aG = *aPtr++;int aB = *aPtr++;*aQ++=0xFF000000|(aR<<16)|(aG<<8)|(aB);}
	}
	*theWidth=aInfo.output_width;
	*theHeight=aInfo.output_height;
	*theBits=(unsigned char*)aBits;

	jpeg_finish_decompress(&aInfo);
	jpeg_destroy_decompress(&aInfo);
	fclose(aFile);

	//
	// Not sure why I have to flip RB on jpgs but not on PNGs...
	//
    unsigned char *aBitPtr=*theBits;
    for (int aSpan=0;aSpan<*theWidth*(*theHeight);aSpan++)
    {
        unsigned char aHoldR=*(aBitPtr+0);
        unsigned char aHoldB=*(aBitPtr+2);
        *(aBitPtr+0)=(char)aHoldB;
        *(aBitPtr+2)=(char)aHoldR;
        aBitPtr+=4;
    }
	
	return true;
}

/*
bool Graphics_Core::LoadImage_RAW(char *theFilename, unsigned char **theBits, int *theWidth, int *theHeight)
{
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
*/

void Graphics_Core::SaveImage_PNG(char *theFilename, unsigned char *theBits, int theWidth, int theHeight)
{
	//
	// Save a PNG.  Implement if you can, but it's not hugely necessary unless you're making an editor or
	// something.
	//
	// #ifdef _PORT_EXPORTIMG
	// !ERROR
	// #endif
	//
}

/*
int	Graphics_Core::CreateTexture(int theWidth, int theHeight, int theFormat)
{
	unsigned char *aBitFix=NULL;
	#define CLEARCOLOR 0
	int aBitSize=0;
	switch (theFormat)
	{
		case 0:case 8888:aBitSize=theWidth*theHeight*4;break;
		case 1:case 565:aBitSize=theWidth*theHeight*2;break;
		case 2:case 4444:aBitSize=theWidth*theHeight*2;break;
		case 16:aBitSize=theWidth*theHeight*2;break;
	}
		
	if (aBitSize>0)
	{
		aBitFix=new unsigned char[aBitSize];
		memset(aBitFix,CLEARCOLOR,aBitSize);
	}

	unsigned int aNewTex=0;
	ThreadLock();
	for (aNewTex=0;aNewTex<gTextureList.size();aNewTex++) if (gTextureList[aNewTex]->IsOpen()) break;
	if (aNewTex>=gTextureList.size()) gTextureList.push_back(new TextureStruct());
	gTextureList[aNewTex]->mGLTexture=-1; //(int)aTexture;
	gTextureList[aNewTex]->mFlags|=GFXFLAG_PENDING;
	gTextureList[aNewTex]->mWidth=theWidth;
	gTextureList[aNewTex]->mHeight=theHeight;
	gTextureList[aNewTex]->mFormat=theFormat;
	gTextureList[aNewTex]->mBits=aBitFix;
	ThreadUnlock();
	//delete [] aBitFix;
	return aNewTex;
}
*/

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

	unsigned int aNewTex = 0;
	ThreadLock();
	for (aNewTex = 0; aNewTex < gTextureList.size(); aNewTex++) if (gTextureList[aNewTex]->IsOpen()) break;
	if (aNewTex>=gTextureList.size()) gTextureList.push_back(new TextureStruct());
	gTextureList[aNewTex]->mGLTexture=-1;
	gTextureList[aNewTex]->mFlags|=GFXFLAG_PENDING;
	gTextureList[aNewTex]->mWidth = theWidth;
	gTextureList[aNewTex]->mHeight = theHeight;
	gTextureList[aNewTex]->mBits = aBitFix;
	gTextureList[aNewTex]->mFormat=theFormat;
	ThreadUnlock();
	return (int) aNewTex;
}

/*
int	Graphics_Core::CreateRenderer(int theWidth, int theHeight, int pixelFormat, bool canRead)
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

/*
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

/*
namespace Graphics_Core
{
	struct TextureStruct
	{
		TextureStruct()
		{
			mGLTexture=0;
			mFrameBuffer=0;
			mRendererBits=0;
			mBits=0;
		}
		int 					mWidth;
		int 					mHeight;
		int						mGLTexture;
		int						mFrameBuffer;
		int                     mFormat;
		char*					mBits;
		char*                   mRendererBits;
	};
	Array<struct TextureStruct> gTextureList;

	void CreateGLTexture(int theWidth, int theHeight, int theFormat, char *theBits)
	{
		//
		// Wrapped here so if we add new texture formats, we can do it in one place.
		//
		
		switch (theFormat)
		{
		case 0:case 8888:XGL(glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,theWidth,theHeight,0,GL_RGBA,GL_UNSIGNED_BYTE,theBits));break;
		case 1:case 565:XGL(glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,theWidth,theHeight,0,GL_RGB,GL_UNSIGNED_SHORT_5_6_5,theBits));break;
		case 2:case 4444:XGL(glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,theWidth,theHeight,0,GL_RGBA,GL_UNSIGNED_SHORT_4_4_4_4,theBits));break;
		default:OS_Core::Printf("Graphics_Core::CreateGLTexture() : Bad Texture Format: %d",theFormat);break;
		}
	}
	
}

int	Graphics_Core::CreateTexture(int theWidth, int theHeight, int theFormat) {return LoadTexture(theWidth,theHeight,NULL,theFormat);}
int Graphics_Core::LoadTexture(int theWidth, int theHeight, unsigned char *theBits, int theFormat)
{
	//
	// Loads a texture onto the video card (or onto a surface or whatevs).
	// Textures should be placed in a list, and their index in that list returned.  That way,
	// all textures can be referenced by a single int, even if they're pointers or whatnot.
	//
	// Example OPENGL List:
	//		struct TextureStruct
	//		{
	//			TextureStruct()
	//			{
	//				mGLTexture=0;
	//				mFrameBuffer=0;
	//				mRendererBits=0;
	//				mBits=0;
	//			}
	//			int 					mWidth;
	//			int 					mHeight;
	//			int						mGLTexture;
	//			int						mFrameBuffer;
	//			int                     mFormat;
	//			char*					mBits;				// Keep around if you need them to recover textures with multitasking...
	//			char*                   mRendererBits;		// Keep around if you need to recover... most systems require a different initialization for renderer bits...
	//		};
	//		Array<struct TextureStruct> gTextureList;
	//
	// theFormat's values can be 8888, 565, 4444.
	// If theBits==NULL, just create a blank texture with thebits memset to zero.
	//
	//#ifdef _PORT_LOADTEX
	//!ERROR
	//#endif
	//
	GLuint aTexture;
	glGenTextures(1,&aTexture);

	int aFormatSize=4;
	if (theFormat==1 || theFormat==2 || theFormat==565 || theFormat==4444) aFormatSize=2;

	XGL(glEnable(GL_TEXTURE_2D));
	XGL(glBindTexture(GL_TEXTURE_2D,aTexture));
	//glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_STORAGE_HINT_APPLE,GL_STORAGE_CACHED_APPLE);
	//glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, GL_TRUE);		
	XGL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR));
	XGL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR));
	XGL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT));
	XGL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT));   

	char *aBitFix=new char[theWidth*theHeight*aFormatSize];
	char *aBitPtr=aBitFix;

	if (theBits)
	{
		if (theFormat==0 || theFormat==8888)
		{
			//
			// Format 8888
			//
			memcpy(aBitFix,theBits,theWidth*theHeight*4);
			if (gReverseRGB) for (int aSpan=0;aSpan<theWidth*theHeight;aSpan++)
			{
				char aHoldR=*aBitPtr;
				char aHoldB=*(aBitPtr+2);
				*aBitPtr=aHoldB;
				*(aBitPtr+2)=aHoldR;
				aBitPtr+=4;
			}
			CreateGLTexture(theWidth,theHeight,theFormat,aBitFix);
		}
		if (theFormat==1 || theFormat==565)
		{
			//
			// Convert to 565
			//
			{
				int aSize=theHeight*theWidth;

				if (gReverseRGB)
				{
					for (int aCount=0;aCount<aSize;aCount++) 
					{
						int aSrca=theBits[aCount*4+2]/8;
						int aSrcb=theBits[aCount*4+1]/4;
						int aSrcc=theBits[aCount*4+0]/8;
						unsigned short aColor=(aSrcb>>3)|((aSrca&0xFC)<<3)|((aSrcc&0xF8)<<8);
						*aBitPtr=aColor>>8;aBitPtr++;
						*aBitPtr=aColor&0x00FF;aBitPtr++;
					}
				}
				else
				{
					for (int aCount=0;aCount<aSize;aCount++) 
					{
						int aSrca=theBits[aCount*4+0]/8;
						int aSrcb=theBits[aCount*4+1]/4;
						int aSrcc=theBits[aCount*4+2]/8;
						unsigned short aColor=(aSrcb>>3)|((aSrca&0xFC)<<3)|((aSrcc&0xF8)<<8);
						*aBitPtr=aColor>>8;aBitPtr++;
						*aBitPtr=aColor&0x00FF;aBitPtr++;
					}
				}
				CreateGLTexture(theWidth,theHeight,theFormat,aBitFix);
			}
		}
		if (theFormat==2 || theFormat==4444) 
		{
			//
			// Convert to 4444
			//
			{
				int aSize=theHeight*theWidth;

				if (gReverseRGB)
				{
					for (int aCount=0;aCount<aSize;aCount++) 
					{
						int aSrcc=theBits[aCount*4]/16;
						int aSrcb=theBits[aCount*4+1]/16;
						int aSrca=theBits[aCount*4+2]/16;
						int aSrcd=theBits[aCount*4+3]/16;
	
						*aBitPtr=(aSrcc<<4|aSrcd);aBitPtr++;
						*aBitPtr=(aSrca<<4|aSrcb);aBitPtr++;
					}
				}
				else
				{
					for (int aCount=0;aCount<aSize;aCount++) 
					{
						int aSrcc=theBits[aCount*4+2]/16;
						int aSrcb=theBits[aCount*4+1]/16;
						int aSrca=theBits[aCount*4+0]/16;
						int aSrcd=theBits[aCount*4+3]/16;

						*aBitPtr=(aSrcc<<4|aSrcd);aBitPtr++;
						*aBitPtr=(aSrca<<4|aSrcb);aBitPtr++;
					}
				}
				CreateGLTexture(theWidth,theHeight,theFormat,aBitFix);
			}
		}
	}
	else
	{
		memset(aBitFix,0,theWidth*theHeight*aFormatSize);
		CreateGLTexture(theWidth,theHeight,theFormat,aBitFix);
	}


	XGL(glFlush());

	int aNewTex=0;
	for (aNewTex=0;aNewTex<gTextureList.Size();aNewTex++) if (gTextureList[aNewTex].mGLTexture==0) break;

	gTextureList[aNewTex].mGLTexture=(int)aTexture;
	gTextureList[aNewTex].mWidth=theWidth;
	gTextureList[aNewTex].mHeight=theHeight;
	gTextureList[aNewTex].mBits=aBitFix;
	gTextureList[aNewTex].mFormat=theFormat;
	
	return (int)aNewTex;
}

void Graphics_Core::UnloadTexture(int theNumber)
{
	//
	// Unloads a texture
	//
	//#ifdef _PORT_LOADTEX
	//!ERROR
	//#endif
	//
	GLuint aTexture=gTextureList[theNumber].mGLTexture;
	if (aTexture!=0) XGL(glDeleteTextures(1,&aTexture));
	GLuint aFrameBuffer=gTextureList[theNumber].mFrameBuffer;
	if (aFrameBuffer!=0) XGL(glDeleteFramebuffers(1, &aFrameBuffer));
	
	gTextureList[theNumber].mGLTexture=0;
	gTextureList[theNumber].mFrameBuffer=0;
	delete [] gTextureList[theNumber].mBits;
	delete [] gTextureList[theNumber].mRendererBits;
	gTextureList[theNumber].mBits=NULL;
	gTextureList[theNumber].mRendererBits=NULL;
}
 
void Graphics_Core::GetTextureDimensions(int theNumber, int *theWidth, int *theHeight)
{
	//
	// Return the dimensions of a texture that's already loaded.  In all systems ported so far,
	// I've had to keep a record of this and just look into the table.
	//
	//#ifdef _PORT_LOADTEX
	//!ERROR
	//#endif
	//
	*theWidth=gTextureList[theNumber].mWidth;
	*theHeight=gTextureList[theNumber].mHeight;
}

void Graphics_Core::SetTexture(int theNumber)
{
	//
	// Sets the current texture for the system to use.
	// It is wise to remember what texture is currently selected so that you can optimize drawing with no 
	// texture (for fills, etc).  Also, be prepared to set the filter for textures, since that is coming later.
	//
	//#ifdef _PORT_LOADTEX
	//!ERROR
	//#endif
	//
	glActiveTexture(GL_TEXTURE0);
	XGL(glActiveTexture(GL_TEXTURE0));
	XGL(glClientActiveTexture(GL_TEXTURE0));
	
	if (theNumber==-1)
	{
		glBindTexture(GL_TEXTURE_2D,0);	
		XGL(glDisable(GL_TEXTURE_2D));
	}
	else 
	{
		XGL(glEnable(GL_TEXTURE_2D));
		XGL(glBindTexture(GL_TEXTURE_2D,gTextureList[theNumber].mGLTexture));	
		XGL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,gFilter));
		XGL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,gFilter));
	}
}
*/

void Graphics_Core::Multitasking(bool inForeground)
{
	//
	// Do anything you need to here, for the implementation.  Some systems will need their
	// texture reloaded.  Others won't need to do anything!
	//
}

/*
namespace Graphics_Core
{
	struct ShapeStruct
	{
		ShapeStruct() {mVertexBuffer=0;mIndexBuffer=0;mVBackup=NULL;mIBackup=NULL;mOwnBackups=false;}
		void Clean() {delete [] mVBackup;mVBackup=NULL;delete [] mIBackup;mIBackup=NULL;}

		GLuint                  mVertexBuffer;
		GLuint                  mIndexBuffer;
		int						mVertexCount;
		int						mIndexCount;
		int						mVertexStride;

		char*                   mVBackup;
		short*                  mIBackup;
		bool                    mOwnBackups;
	};
	Array <ShapeStruct>			gShapeList;
}

int Graphics_Core::CreateShape(void* theVertexList, int theVertexCount, short* theIndexList, int theIndexCount, int theVertexStride)
{
	//
	// Create a vertexbuffer on the card.  Like textures, it's best to store these in a list so that they can be recovered,
	// And use the index into the list for all references.  theVertexList should be verts in this format:
	//		struct Vertex2DRef
	//		{
	//			float			mX;
	//			float			mY;
	//			float			mZ;
	//			unsigned int	mDiffuse;
	//			float			mTextureU;
	//			float			mTextureV;
	//		}
	//
	//#ifdef _PORT_PRIMITIVES3
	//!ERROR
	//#endif
	//
	int aShapeSpot=0;
	for (aShapeSpot=0;aShapeSpot<=gShapeList.Size();aShapeSpot++) if (!gShapeList[aShapeSpot].mVertexBuffer) break;
	ShapeStruct& aShape=gShapeList[aShapeSpot];

	int aVSize=theVertexCount*theVertexStride;
	int aISize=theIndexCount*(sizeof(short));

	XGL(glGenBuffers(1,&aShape.mVertexBuffer));
	XGL(glBindBuffer(GL_ARRAY_BUFFER,aShape.mVertexBuffer));
	XGL(glBufferData(GL_ARRAY_BUFFER,aVSize,theVertexList,GL_STATIC_DRAW));

	XGL(glGenBuffers(1,&aShape.mIndexBuffer));
	XGL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,aShape.mIndexBuffer));
	XGL(glBufferData(GL_ELEMENT_ARRAY_BUFFER,aISize,theIndexList,GL_STATIC_DRAW));

	aShape.mVertexCount=theVertexCount;
	aShape.mIndexCount=theIndexCount;
	aShape.mVertexStride=theVertexStride;
	aShape.mOwnBackups=false;

	{
		aShape.mOwnBackups=true;
		aShape.mVBackup=new char[aVSize];
		aShape.mIBackup=new short[theIndexCount];
		memcpy(aShape.mVBackup,theVertexList,aVSize);
		memcpy(aShape.mIBackup,theIndexList,aISize);
	}

	XGL(glBindBuffer(GL_ARRAY_BUFFER,NULL));
	XGL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,NULL));

	return aShapeSpot;
}

void Graphics_Core::UnloadShape(int theNumber)
{
	//
	// Destroy a vertexbuffer
	//
	//#ifdef _PORT_PRIMITIVES3
	//!ERROR
	//#endif
	//
	if (theNumber<0 || theNumber>=gShapeList.Size()) return;
	ShapeStruct& aShape=gShapeList[theNumber];

	XGL(glDeleteBuffers(1,&aShape.mVertexBuffer));
	XGL(glDeleteBuffers(1,&aShape.mIndexBuffer));

	aShape.mVertexBuffer=0;
	aShape.mIndexBuffer=0;

	if (aShape.mOwnBackups)
	{
		delete [] aShape.mVBackup;
		delete [] aShape.mIBackup;
	}
	aShape.mVBackup=NULL;
	aShape.mIBackup=NULL;
	aShape.mOwnBackups=false;
}

void Graphics_Core::DrawShape(int theNumber)
{
	//
	// Draw a vertexbuffer...
	//
	//#ifdef _PORT_PRIMITIVES3
	//!ERROR
	//#endif
	//
	struct Vertex2DRef
	{
		float			mX;
		float			mY;
		float			mZ;
		unsigned int	mDiffuse;
		float			mTextureU;
		float			mTextureV;
	};


	if (theNumber<0 || theNumber>=gShapeList.Size()) return;
	ShapeStruct& aShape=gShapeList[theNumber];

	XGL(glBindBuffer(GL_ARRAY_BUFFER,aShape.mVertexBuffer));
	XGL(glVertexPointer(3,GL_FLOAT,aShape.mVertexStride,(void*)offsetof(Vertex2DRef,mX)));
	XGL(glColorPointer(4,GL_UNSIGNED_BYTE,aShape.mVertexStride,(void*)offsetof(Vertex2DRef,mDiffuse)));
	XGL(glTexCoordPointer(2,GL_FLOAT,aShape.mVertexStride,(void*)offsetof(Vertex2DRef,mTextureU)));
	XGL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,aShape.mIndexBuffer));
	gBoundBuffer=true;

	XGL(glDrawElements(GL_RENDERMODE,aShape.mIndexCount,GL_UNSIGNED_SHORT,NULL));
}


void Graphics_Core::CullCCW()
{
	//
	// Cull CCW polygons...
	//
	//#ifdef _PORT_CULLING
	//!ERROR
	//#endif
	//
	XGL(glEnable(GL_CULL_FACE));
	XGL(glFrontFace(GL_CW));
}

void Graphics_Core::CullCW()
{
	// 
	// Cull CW polygons...
	//
	//#ifdef _PORT_CULLING
	//!ERROR
	//#endif
	//
	XGL(glEnable(GL_CULL_FACE));
	XGL(glFrontFace(GL_CCW));
}

void Graphics_Core::RenderNormal()
{
	//
	// Remove all rendering special effects and just render normally
	//
	//
	//#ifdef _PORT_DRAWTEX
	//!ERROR
	//#endif
	//
	if (!gIsRenderToTexture)
	{
		XGL(glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA));
		XGL(glBlendEquation(GL_FUNC_ADD));
	}
	else
	{
		XGL(glBlendFuncSeparate(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA,GL_ONE,GL_ONE_MINUS_SRC_ALPHA));
		XGL(glBlendEquationSeparate(GL_FUNC_ADD,GL_FUNC_ADD));
	}
}

void Graphics_Core::Filter(bool theState)
{
	//
	// Set whether textures will be filtered or not when drawn
	//
	//#ifdef _PORT_DRAWTEX
	//!ERROR
	//#endif
	//

	return;	// Turns out this looked bad in OpenGL... so just skipped it!
	//if (theState) gFilter=GL_LINEAR;
	//else gFilter=GL_NEAREST;
	//if (gCurrentTexture!=-1) SetTexture(gCurrentTexture);
}

void Graphics_Core::Clamp()
{
	//
	// Set texture mode as Clamped to the edge of the texture
	//
	//#ifdef _PORT_DRAWTEX
	//!ERROR
	//#endif
	//
	XGL(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	XGL(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
}

void Graphics_Core::Wrap()
{
	//
	// Set textures to wrap around the edges of the texture
	//
	//#ifdef _PORT_DRAWTEX
	//!ERROR
	//#endif
	//
	XGL(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
	XGL(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
	
}

void Graphics_Core::RenderAdditive()
{
	//
	// Render additively
	//
	//#ifdef _PORT_FX_ADDITIVE
	//!ERROR
	//#endif
	//
	XGL(glBlendFunc(GL_SRC_ALPHA,GL_ONE));
}

void Graphics_Core::RenderMultiply()
{
	//
	// Render with multiply effect
	//
	//#ifdef _PORT_FX_MULTIPLY
	//!ERROR
	//#endif
	//
	XGL(glBlendFunc(GL_ZERO,GL_SRC_COLOR));
}


void Graphics_Core::RenderWhite(bool theState)
{
	//
	// Render as pure white (See NeedSetRenderWhiteColor, if needed)
	//
	//
	//#ifdef _PORT_FX_WHITE
	//!ERROR
	//#endif
	//
	
	//
	// This is all the other OpenGLs... this is how it is in 
	// modern android/iOS/mac/PC
	//
	//if (theState) glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_BLEND);
	//else glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
	
	//
	// This is my archaic code that doesn't work on newer OpenGL-- but seems to work
	// on WebAssembly.... I believe this code came from my first framework (so iPhone1)
	//
	if (theState)
	{
		XGL(glColor4f(1.0f, 1.0f, 1.0f, 1.0f));
		XGL(glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE));
		XGL(glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE));
		XGL(glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_PREVIOUS));
		XGL(glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_TEXTURE));
		XGL(glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR));
		XGL(glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR));
	}
	else
	{
		XGL(glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE));
	}
}


void Graphics_Core::SetRenderWhiteColor(float *theRGBA)
{
	//
	// Set the color for rendering white
	//
	//#ifdef _PORT_FX_WHITE
	//!ERROR
	//#endif
	//
	if (!theRGBA) return;

	GLfloat aColorArray[4];
	aColorArray[0]=*theRGBA;theRGBA++;
	aColorArray[1]=*theRGBA;theRGBA++;
	aColorArray[2]=*theRGBA;theRGBA++;
	aColorArray[3]=*theRGBA;theRGBA++;
	XGL(glTexEnvfv(GL_TEXTURE_ENV,GL_TEXTURE_ENV_COLOR,aColorArray));
}

void Graphics_Core::RenderSaturation(float theSaturation)
{
	//
	// Render desaturated, if supported (needs shaders)
	//
	 #ifdef _PORT_FX_DESAT
	 !ERROR
	 #endif
	//
	if (theSaturation<1.0f)
	{
		//glUseProgram(gSaturationShader);
		//glUniform1f(gSaturation_UniformLocation,theSaturation);
	}
	else 
	{
		//glUseProgram(0);
	}
}

void ReloadEverything()
{
	//
	// Do anything you need to here, for the implementation.  Some systems will need their
	// texture reloaded.  Others won't need to do anything!
	//
	//#ifdef _PORT_MULTITASK
	//!ERROR
	//#endif
	//
	
	//
	// Looks like we don't need this for WebGL?
	//
}
*/

void Graphics_Core::SetResolution(int theWidth, int theHeight, bool fullScreen)
{
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
	//#ifdef _PORT_RESOLUTION
	//!ERROR
	//#endif
	//
	int aWidth=emscripten_run_script_int("getCanvasWidth();\n");
	int aHeight=emscripten_run_script_int("getCanvasHeight();\n");
	if (aWidth>0 && aHeight>0) {theWidth=aWidth;theHeight=aHeight;}

	
	gFullscreen=false; //Never fullscreen on web
	gResolutionWidth=theWidth;
	gResolutionHeight=theHeight;
	
	if (!1) // WASM not a thing...
	if (gRenderer)
	{
		
		SDL_DisplayMode aMode;
		SDL_GetWindowDisplayMode(gWindow,&aMode);
		aMode.w=theWidth;
		aMode.h=theHeight;
		SDL_SetWindowDisplayMode(gWindow,&aMode);
		int aFlags=0;
		if (fullScreen) aFlags=SDL_WINDOW_FULLSCREEN_DESKTOP;
		SDL_SetWindowFullscreen(gWindow,aFlags);

		if (fullScreen)
		{
			SDL_GetWindowDisplayMode(gWindow,&aMode);
			gResolutionWidth=aMode.w;
			gResolutionHeight=aMode.h;
		}

		gSDLScreen=SDL_GetWindowSurface(gWindow);
		if (!gSDLScreen) OS_Core::Error("Could not create SDL Screen...");


		SDL_DestroyRenderer(gRenderer);
		Startup();
		ReloadEverything();
	}
}

bool Graphics_Core::IsSmallScreen()
{
	if (((float)gResolutionWidth/(float)gResolutionHeight)>2.0f) return true; // Screen is super wide... assume small screen!
	static char aCheckSmallScreen=-1;
	if (aCheckSmallScreen==-1) aCheckSmallScreen=emscripten_run_script_int("WebQuery(\"FORCESMALLSCREEN\");\n");
	return (aCheckSmallScreen!=0);
}

void Graphics_Core::ConvertResolutionToPage(int *theX, int *theY)
{
	//
	// Conversion for visible resolution to logical resolution...
	// Probably can just uncomment this code!
	//
	//#ifdef _PORT_RESOLUTION
	//!ERROR
	//#endif
	//

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
	//#ifdef _PORT_RESOLUTION
	//!ERROR
	//#endif
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
	//#ifdef _PORT_RESOLUTION
	//!ERROR
	//#endif
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
	//#ifdef _PORT_RESOLUTION
	//!ERROR
	//#endif
	//
	
	//
	// For web assembly, this doesn't really do anything.
	// The point of this function is to help with fullscreening
	//
	*theWidth=gResolutionWidth;
	*theHeight=gResolutionHeight;
}

bool Graphics_Core::IsTooBigForWindowed(int theWidth, int theHeight)
{
	//
	// Simply tells us if the resolution requested is too big for windowed mode.
	//
	//#ifdef _PORT_FULLSCREEN
	//!ERROR
	//#endif

	/*
	GetDesktopSize(&gDesktopWidth,&gDesktopHeight);
	if (theWidth>gDesktopWidth) return true;
	if (theHeight>gDesktopHeight-100) return true;
	*/
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
	//#ifdef _PORT_RESOLUTION
	//!ERROR
	//#endif
	//
	
	//
	// For web assembly, only what the app was initialized with is available.
	//
	gResolutionList.Reset();
	gResolutionList[0][0]=gResolutionWidth;
	gResolutionList[0][1]=gResolutionHeight;

	return gResolutionList;
}

/*
int Graphics_Core::CreateRenderer(int theWidth, int theHeight, bool wantAlpha, bool canRead)
{
	//
	// Create a "render-to" texture, howsoever one does it on the current OS
	//
	//#ifdef _PORT_RENDERTO
	//!ERROR
	//#endif
	//

	int aNewTex;
	if (wantAlpha) aNewTex=CreateTexture(theWidth,theHeight);
	else aNewTex=CreateTexture(theWidth,theHeight,1);

	GLuint aTexture=gTextureList[aNewTex].mGLTexture;

	GLuint aFrameBuffer;
	XGL(glGenFramebuffers(1,&aFrameBuffer));
	XGL(glBindFramebuffer(GL_FRAMEBUFFER,aFrameBuffer));
	XGL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,aTexture, 0));
	XGL(glBindFramebuffer(GL_FRAMEBUFFER,0));
	XGL(glFlush());

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
	//#ifdef _PORT_RENDERTO
	//!ERROR
	//#endif
	//

	static int aHoldViewport[4];

	if (theTexture==-1)
	{
		XGL(glBindFramebuffer(GL_FRAMEBUFFER,0));
		if (gIsRenderToTexture) XGL(glViewport(aHoldViewport[0],aHoldViewport[1],aHoldViewport[2],aHoldViewport[3]));
		gIsRenderToTexture=false;
	}
	else 
	{
		if (!gIsRenderToTexture) XGL(glGetIntegerv(GL_VIEWPORT,aHoldViewport));
		XGL(glBindFramebuffer(GL_FRAMEBUFFER,gTextureList[theTexture].mFrameBuffer));
		XGL(glViewport(0,0,gTextureList[theTexture].mWidth,gTextureList[theTexture].mHeight));
		gIsRenderToTexture=true;
		gRenderToTextureNumber=theTexture;
	}
	RenderNormal();
}

void* Graphics_Core::GetRenderTargetPixels()
{
	//
	// Read back the pixels from the render target.  Width/Height are assumed known outside.
	//
	//#ifdef _PORT_RENDERTO_READ
	//!ERROR
	//#endif
	//
	if (gIsRenderToTexture)
	{
		int thePixelFormat;
		int thePixelType;

		switch (gTextureList[gRenderToTextureNumber].mFormat)
		{
		case 0:case 8888:thePixelFormat=GL_RGBA;thePixelType=GL_UNSIGNED_BYTE;break;
		case 1:case 565:thePixelFormat=GL_RGB;thePixelType=GL_UNSIGNED_SHORT_5_6_5;break;
		}
		XGL(glReadPixels(0,0,gTextureList[gRenderToTextureNumber].mWidth,gTextureList[gRenderToTextureNumber].mHeight,thePixelFormat,thePixelType,gTextureList[gRenderToTextureNumber].mRendererBits));
		
		RGBA* aPtr=(RGBA*)gTextureList[gRenderToTextureNumber].mRendererBits;
		for (int aCount=0;aCount<gTextureList[gRenderToTextureNumber].mWidth*gTextureList[gRenderToTextureNumber].mHeight;aCount++)
		{
			char aHold=aPtr->mB;
			aPtr->mB=aPtr->mR;
			aPtr->mR=aHold;
			aPtr++;
		}
		
		return gTextureList[gRenderToTextureNumber].mRendererBits;
	}
	return NULL;
}
*/
/*
void Graphics_Core::Set2DRenderToCamera(int thePageWidth, int thePageHeight, int theResolutionWidth, int theResolutionHeight)
{
	//
	// Set up matrixes, etc, so that things render in the texture just like they'd render on the screen.
	// The parameters are width/height of texture, then width/height of the app's resolution.  Those are in there
	// because OpenGL treats everything like the main screen (DirectX doesn't).
	//
	//#ifdef _PORT_RENDERTO
	//!ERROR
	//#endif
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

void Graphics_Core::Set3DCameraOffset(float theX, float theY)
{
	g3DOffsetX=theX;
	g3DOffsetY=theY;
}

void Graphics_Core::Set3DCamera(float theCameraX, float theCameraY, float theCameraZ, float theLookatX, float theLookatY, float theLookatZ, float theUpVectorX, float theUpVectorY, float theUpVectorZ, float theFOV)
{
	if(gGraphicsPaused) return;
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
	if (aAspect > 1.0)
	{
		aWidth /= aAspect;
	}
	else
	{
		aHeight *= aAspect;
	}
	float s = SIN(theFOV / 2.0f);
	float d = 1.0f - aNear / aFar;

	float aMatrix[4][4];
	aMatrix[0][0] = aWidth;
	aMatrix[1][0] = 0;
    aMatrix[2][0]=g3DOffsetX/gPageWidth/2;
	aMatrix[3][0] = 0;
	aMatrix[0][1] = 0;
	aMatrix[1][1] = aHeight;
    aMatrix[2][1]=g3DOffsetY/gPageHeight/2;
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
*/



void Graphics_Core::WantZBuffer(float theZNear, float theZFar)
{
	//
	// This should enable the SYSTEM to do z-buffer, but it doesn't turn it on.
	// This is purely here to set up the near and far... this is not "enable zbuffering"
	//

	//
	//#ifdef _PORT_ZBUFFER
	//!ERROR
	//#endif
	//
	gZNear=theZNear;
	gZDepth=theZFar;
}

float Graphics_Core::GetZDepth()
{
	return gZDepth;
}

/*
void Graphics_Core::EnableZBuffer()
{
	//
	//#ifdef _PORT_ZBUFFER
	//!ERROR
	//#endif
	//
	if (gZDepth>0)
	{
		XGL(glEnable(GL_DEPTH_TEST));
		gZBuffer=true;
		glDepthFunc(GL_LEQUAL);
	}
}

void Graphics_Core::DisableZBuffer()
{
	//
	//#ifdef _PORT_ZBUFFER
	//!ERROR
	//#endif
	//
	XGL(glDisable(GL_DEPTH_TEST));
	gZBuffer=false;
}

void Graphics_Core::EnableZWrites()
{
	//#ifdef _PORT_ZBUFFEREX
	//!ERROR
	//#endif
	XGL(glDepthMask(GL_TRUE));
}

void Graphics_Core::DisableZWrites()
{
	//#ifdef _PORT_ZBUFFEREX
	//!ERROR
	//#endif
	XGL(glDepthMask(GL_FALSE));
}



void Graphics_Core::SetZOrder(char theOrder)
{
//#ifdef _PORT_ZBUFFEREX
//	!ERROR
//#endif
    if (theOrder==0) XGL(glDepthFunc(GL_LEQUAL));
    if (theOrder==1) XGL(glDepthFunc(GL_GREATER));
    if (theOrder==2) XGL(glDepthFunc(GL_EQUAL));
    if (theOrder==3) XGL(glDepthFunc(GL_LESS));
    if (theOrder==4) XGL(glDepthFunc(GL_GEQUAL));
}

void Graphics_Core::SetZBias(float theValue)
{
	//#ifdef _PORT_ZBUFFEREX
	//	!ERROR
	//#endif
    if (theValue!=0)
    {
        XGL(glEnable(GL_POLYGON_OFFSET_FILL));
        glPolygonOffset(-1.0f,-theValue);
    }
    else XGL(glDisable(GL_POLYGON_OFFSET_FILL));

}

void Graphics_Core::EnableFog(bool isLinear, int theColor, float theStart, float theEnd)
{
	//#ifdef _PORT_FOG
	//!ERROR
	//#endif

    theStart=GetZDepth()*theStart;
    theEnd=GetZDepth()*theEnd;
    
    int aC1=theColor&0x0000FF;
    int aC2=(theColor&0x00FF00)>>8;
    int aC3=(theColor&0xFF0000)>>16;
   
    
    float aColor[4];
    aColor[0]=(float)aC1/255;
    aColor[1]=(float)aC2/255;
    aColor[2]=(float)aC3/255;
    aColor[3]=1;

    XGL(glEnable(GL_FOG));                   // Enables GL_FOG
    XGL(glFogi(GL_FOG_MODE,GL_LINEAR));        // Fog Mode
    XGL(glFogfv(GL_FOG_COLOR,aColor));//theColor);            // Set Fog Color
    XGL(glFogf(GL_FOG_DENSITY, .33f));              // How Dense Will The Fog Be
    XGL(glHint(GL_FOG_HINT, GL_NICEST));          // Fog Hint Value
    XGL(glFogf(GL_FOG_START, theStart));             // Fog Start Depth
    XGL(glFogf(GL_FOG_END, theEnd));               // Fog End Depth
}

void Graphics_Core::DisableFog()
{
	//#ifdef _PORT_FOG
	//!ERROR
	//#endif

	XGL(glDisable(GL_FOG));
}

void Graphics_Core::SetMultiTexture(int theNumber)
{
	XGL(glActiveTexture(GL_TEXTURE1));
	XGL(glClientActiveTexture(GL_TEXTURE1));
	
	if (theNumber==-1)
	{
		glBindTexture(GL_TEXTURE_2D,0);	
		XGL(glDisable(GL_TEXTURE_2D));
	}
	else 
	{
		XGL(glEnable(GL_TEXTURE_2D));
		XGL(glBindTexture(GL_TEXTURE_2D,gTextureList[theNumber].mGLTexture));	
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		XGL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,gFilter));
		XGL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,gFilter));
	}
}

void Graphics_Core::RenderBlurred(float theBlur)
{
}
*/

void Graphics_Core::EnableThreadGraphics()
{
	// No threading in WASM yet...
}

void Graphics_Core::ThreadLock()
{
	// No threading in WASM yet...
}

void Graphics_Core::ThreadUnlock()
{
	// No threading in WASM yet...
}

/*
void Graphics_Core::Wireframe(bool theState)
{
	if (theState) 
	{
		XGL(glPolygonMode( GL_FRONT_AND_BACK, GL_LINE ));
	}
	else 
	{
		XGL(glPolygonMode( GL_FRONT_AND_BACK, GL_FILL ));
	}
}

	//
	// New Additions, 2021, 
	// For drawing with normals and lights for Hamsterball
	//

void Graphics_Core::DrawShapeN(void *theBuffer, int theTriangleCount, int theVertexStride)
{
	//
	// CAN'T DO IT!
	// We have no way to set materials or lights in Emscripten!
	// Looks like only option will be shader'd GL here!
	//
	
	struct Vertex2DNRef
	{
		float			mX;
		float			mY;
		float			mZ;
		float			mNX;
		float			mNY;
		float			mNZ;
		float			mTextureU;
		float			mTextureV;
	};

	if (gBoundBuffer)
	{
		XGL(glBindBuffer(GL_ARRAY_BUFFER,NULL));
		XGL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,NULL));
		gBoundBuffer=false;
	}
	
   GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
   GLfloat mat_shininess[] = { 50.0 };
   GLfloat light_position[] = { 1.0, 1.0, 1.0, 0.0 };
   glClearColor (0.0, 0.0, 0.0, 0.0);
   glShadeModel (GL_SMOOTH);

   XGL(glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_specular));
   XGL(glMaterialfv(GL_FRONT, GL_AMBIENT, mat_specular));
   XGL(glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular));
   XGL(glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess));
   XGL(glLightfv(GL_LIGHT0, GL_POSITION, light_position));

   XGL(glEnable(GL_LIGHTING));
   XGL(glEnable(GL_LIGHT0));	

	XGL(glDisableClientState(GL_COLOR_ARRAY));
	XGL(glEnableClientState(GL_NORMAL_ARRAY));
	
	//OS_Core::Printf("Stride: %d - %d",sizeof(float),sizeof(GLfloat));

	Vertex2DNRef *aRef=(Vertex2DNRef*)theBuffer;
	XGL(glVertexPointer(3,GL_FLOAT,theVertexStride,&aRef->mX));
	XGL(glNormalPointer(GL_FLOAT,theVertexStride,&aRef->mNX));
	XGL(glTexCoordPointer(2,GL_FLOAT,theVertexStride,&aRef->mTextureU));
	XGL(glDrawArrays(GL_TRIANGLES,0,theTriangleCount*3));
	
	ClearGLError();
	XGL(glEnableClientState(GL_COLOR_ARRAY));CheckGLError();
	XGL(glDisableClientState(GL_NORMAL_ARRAY));
	
   XGL(glDisable(GL_LIGHTING));
   XGL(glDisable(GL_LIGHT0));	
	
}

void Graphics_Core::DrawShapeMT(void *theBuffer, int theTriangleCount, int theVertexStride)
{
	struct Vertex2DMTRef
	{
		float			mX;
		float			mY;
		float			mZ;
		unsigned int	mDiffuse;
		float			mTextureU;
		float			mTextureV;
		float			mTextureU2;
		float			mTextureV2;
	};

	if (gBoundBuffer)
	{
		XGL(glBindBuffer(GL_ARRAY_BUFFER,NULL));
		XGL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,NULL));
		gBoundBuffer=false;
	}
	
	Vertex2DMTRef* aRef=(Vertex2DMTRef*)theBuffer;

	XGL(glVertexPointer(3,GL_FLOAT,theVertexStride,&aRef->mX));
	XGL(glColorPointer(4,GL_UNSIGNED_BYTE,theVertexStride,&aRef->mDiffuse));
	
	XGL(glClientActiveTexture(GL_TEXTURE0));
	XGL(glActiveTexture(GL_TEXTURE0));
	XGL(glEnableClientState(GL_TEXTURE_COORD_ARRAY));
	XGL(glTexCoordPointer(2,GL_FLOAT,theVertexStride,&aRef->mTextureU));

	XGL(glClientActiveTexture(GL_TEXTURE1));
	XGL(glActiveTexture(GL_TEXTURE1));
	XGL(glEnableClientState(GL_TEXTURE_COORD_ARRAY));
	XGL(glTexCoordPointer(2,GL_FLOAT,theVertexStride,&aRef->mTextureU2));
	
	XGL(glDrawArrays(GL_TRIANGLES,0,theTriangleCount*3));


	XGL(glClientActiveTexture(GL_TEXTURE1));
	XGL(glActiveTexture(GL_TEXTURE1));
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	XGL(glClientActiveTexture(GL_TEXTURE0));
	XGL(glActiveTexture(GL_TEXTURE0));
	
}

int Graphics_Core::CreateShapeN(void* theVertexList, int theVertexCount, short* theIndexList, int theIndexCount, int theVertexStride)
{
	return -1;
}

int Graphics_Core::CreateShapeMT(void* theVertexList, int theVertexCount, short* theIndexList, int theIndexCount, int theVertexStride)
{
	return -1;
}

void Graphics_Core::DrawIndexedShapeN(void *theVertexBuffer, void* theIndexBuffer, int theIndexCount, int theTriangleCount, int theVertexStride)
{

}

void Graphics_Core::DrawIndexedShapeMT(void *theVertexBuffer, void* theIndexBuffer, int theIndexCount, int theTriangleCount, int theVertexStride)
{
}

void Graphics_Core::DisableLights()
{
}

void Graphics_Core::SetAmbientLight(float r, float g, float b)
{
}
	
void Graphics_Core::SetMaterial(float ambientR,float ambientG,float ambientB, float ambientA, float diffuseR,float diffuseG,float diffuseB,float diffuseA, float specR,float specG,float specB, float specA, float emR,float emG,float emB, float emA, float thePower)
{
}

void Graphics_Core::AddOmniLight(float x, float y, float z, float theRange,float r, float g, float b, float sr, float sg, float sb, float constAtten, float inverseAtten,float squareAtten, float fallOff)
{
}

void Graphics_Core::AddSkyLight(float xdir, float ydir, float zdir, float r, float g, float b, float sr, float sg, float sb)
{
}
*/

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

void Graphics_Core::Screenshot()
{
}



#endif
