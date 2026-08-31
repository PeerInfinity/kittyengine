#pragma once
#include "os_headers.h"

template <class var_type> class Array;
namespace Graphics_Core
{

	//
	// Call this when you want to start and stop graphics operations
	//
	void			PreStartup(); // For SDL
	void 			Startup_SDL();

	void			Startup();
	void			Shutdown();
	void			Multitasking(bool inForeground);

	//
	// Called sometime after startup, but BEFORE the first draw...
	// (Put in because OpenGL can be a dick about when things can be initialized)
	//
	void			Initialize();

	//
	// Query anything out of Graphics_Core that is weirdly different between ports
	//
	void*			Query(char* theQuery);
	void*			Query(longlong theHash, ...);


	//
	// Call these before and after rendering... you can call them multiple times.
	// Must call BeginRender before drawing shapes, must call EndRender before Show.
	//
	void			BeginRender();
	void			EndRender();

	//
	// This actually displays what you drew
	//
	void			Show();

	//
	// This is to help with multiple resolutions.
	// PageSize is the actual drawing resolution.
	//
#ifdef LEGACY_GL
	void			SetMatrix(int theType, void *theMatrixPtr); // Depreciated by shaders...
	void			GetMatrix(int theType, void *theMatrixPtr); // Depreciated by shaders...
	void			Set2DCamera(int thePageWidth, int thePageHeight); // Depreciated by shaders...
#endif

	void			SetViewport(float x, float y, float theWidth, float theHeight);
	void            GetViewport(float* theViewport);
	void			SetRenderDefaults();

	//
	// Clears the whole display, regardless of resolution, clipping, etc.
	// Just wipes the whole of graphic memory clean.
	//
	void			Clear(float theRed, float theGreen, float theBlue, float theAlpha);
	void 			ClearZ(float theValue);
	void			ClearStencil(int theValue);

	//
	// Draws a polygon shape
	//
	void			DrawTestTriangle();
	void			DrawShape(void *theBuffer, int theTriangleCount, int theVertexStride);
	void			DrawIndexedShape(void *theVertexBuffer, void* theIndexBuffer, int theVertexCount, int theTriangleCount, int theVertexStride);
	//
	// Clips drawing areas.  Conforms to Page Size, not to actual
	// display resolution.
	//
	void			Clip(float theX1, float theY1, float theX2, float theY2);
	void			Clip();

	void 			Wireframe(bool theState);


	//
	// For loading raw images.  This is in OS because some OS's can do this
	// natively, and quickly, and we want to take advantage of that.
	//
	bool			LoadImage_PNG(char *theFilename, unsigned char **theBits, int *theWidth, int *theHeight);
	void			SaveImage_PNG(char *theFilename, unsigned char *theBits, int theWidth, int theHeight);
	bool			LoadImage_GIF(char *theFilename, unsigned char **theBits, int *theWidth, int *theHeight);
	bool			LoadImage_JPG(char *theFilename, unsigned char **theBits, int *theWidth, int *theHeight);

	bool			LoadImageMem_PNG(const unsigned char *theData, int theDataLen, unsigned char **theBits, int *theWidth, int *theHeight);

	//bool			LoadImage_RAW(char *theFilename, unsigned char **theBits, int *theWidth, int *theHeight);
	//void			SaveImage_RAW(char *theFilename, unsigned char *theBits, int theWidth, int theHeight);

	//
	// For dealing with textures
	//
	int				LoadTexture(int theWidth, int theHeight, unsigned char *theBits, int theFormat);	// This loads a texture using bits, so you can split out the alpha
#ifdef LEGACY_GL
	int				CreateTexture(int theWidth, int theHeight, int theFormat=0);			// Creates a blank texture (will always be a render target)
#else
	int				CreateTexture(int theWidth, int theHeight, longlong theFormat=0);			// Creates a blank texture (will always be a render target)
#endif

	void			UnloadTexture(int theNumber);											// Gets rid of a loaded texture
	void			GetTextureDimensions(int theNumber, int *theWidth, int *theHeight);		// Gets info about the texture

	//
	// For shapes stored on the card itself.
	// Lit shapes indicate that they are in format xyz diffuse uv
	//
#ifdef LEGACY_GL
	int				CreateShape(void* theVertexList, int theVertexCount, short* theIndexList, int theIndexCount, int theVertexStride);
	int				CreateShapeN(void* theVertexList, int theVertexCount, short* theIndexList, int theIndexCount, int theVertexStride);
	int				CreateShapeMT(void* theVertexList, int theVertexCount, short* theIndexList, int theIndexCount, int theVertexStride);
	void			DrawIndexedShapeN(void *theVertexBuffer, void* theIndexBuffer, int theIndexCount, int theTriangleCount, int theVertexStride);
	void			DrawIndexedShapeMT(void *theVertexBuffer, void* theIndexBuffer, int theIndexCount, int theTriangleCount, int theVertexStride);
#else
	int				CreateShape(void* theVertexList, int theVertexCount, vbindex* theIndexList, int theIndexCount, int theVertexStride);
#endif
	int				CreateShapeMultiTexture(void* theVertexList, int theVertexCount, vbindex* theIndexList, int theIndexCount, int theVertexStride);
	void			UnloadShape(int theNumber);
	void			DrawShape(int theNumber);
	void			GetShapeData(int theNumber, void** theVertexList, int* theVertexCount, vbindex** theIndexList, int* theIndexCount, int* theVertexStride);


	//
	// Sets the texture you're currently drawing with
	//
	void			SetTexture(int theNumber=-1, char* specialOp=0);					// Queues up the texture for drawing
	void			SetMultiTexture(int theStage, int theNumber=-1, char* specialOp=0);	// SpecialOp allows us to do special stuff.
																						// Currently: pass in 'z' to "set" the z buffer
	//
	// Culling if you want it...
	//
	void			CullNone();
	void			CullCCW();
	void			CullCW();
	//
	// Some rendering states for fancy-shmancy drawing
	//
	void            RenderNormal();
	void			RenderAdditive();                                           // Set additive rendering state
	void			RenderMultiply();                                           // Set multiply rendering state
	void			RenderWhite(bool theState);									// Set white rendering state
	void			RenderSaturation(float theSaturation);						// Set grayscale rendering
	void			RenderBlurred(float theBlurLevel);							// Set blurred rendering... 0 = no blur

	//
	// Couple special functions for coping with OpenGL's
	// treatment of RenderWhite... make NeedSetRenderWhiteColor
	// true or false...
	//
	// #ifdef _PORT_FX_WHITE
	// !ERROR
	// #endif
	//
	inline bool		NeedSetRenderWhiteColor() {return true;}
	void			SetRenderWhiteColor(float *theRGBA);						// OpenGL requires "fixing" of Renderwhite...
	//
	// Changes our filtering
	//
	void			Filter(bool theState);
	//
	// For clamping or wrapping textures...
	//
	void            Clamp();
	void            Wrap();



	//////////////////////////////////////////////////////////////////////////////////////////

	//
	// Graphics_SetResolution is only valid on devices capable of multiple resolutions,
	// like Windows or Mac.  It only affects the size of the memory page allocated for
	// fullscreen mode.
	//
	void			SetResolution(int theWidth, int theHeight, bool fullScreen);

	void            GetDesktopSize(int *theWidth, int *theHeight);
	bool            IsTooBigForWindowed(int theWidth, int theHeight);

	//
	// This tells us if our screen is zoom-adjusted at all... I'm using this for feedback on whether
	// to completely disable filtering or not.
	//
	bool			IsZoomed();

	//
	// Draws a simple line
	//
	void            DrawLine(void *theBuffer, int theLineCount, int theVertexStride);


	//
	// Alpha threshold...
	//
	void			SetAlphaThreshold(float theValue);

	//
	// For converting coordinates on systems where resolution and pagesize might
	// be different... (OS_Core uses this to transform mouse coordinates into page space)
	//
	void			ConvertResolutionToPage(int *theX, int *theY);
	void			ConvertTouchResolutionToPage(int *theX, int *theY);
	void			ConvertPageToResolution(int *theX, int *theY);

	//
	// Takes a screenshot of just the DX area and puts it on the clipboard,
	// or wherever is most OS appropriate.
	//
	void			Screenshot();
	void*			ScreenshotToMemory(int* theWidth, int* theHeight);

	//
	// Creates or Changes our render target
	// CreateRenderer allows for two return variables because OpenGL needs both a FrameBuffer and
	// a texture in these situations.
	//
#ifdef LEGACY_GL
	//int				CreateRenderer(int theWidth, int theHeight, int pixelFormat, bool canRead=false);
	int				CreateRenderer(int theWidth, int theHeights, bool wantAlpha, bool canRead=false);
#else
	int				CreateRenderer(int theWidth, int theHeight, longlong pixelFormat, bool canRead=false);
#endif
	void			SetRenderTarget(int theTexture=-1);
	void*           GetRenderTargetPixels();
	void			Set2DRenderToCamera(int thePageWidth, int thePageHeight, int theResolutionWidth, int theResolutionHeight);


	//
	// Pixel Kludging... in DirectX, drawing is to the center of
	// the pixel, but in OpenGL, evidently not.  This gives us
	// OS-free access to a number to kludge that by, so our
	// framework can query and offset appropriately.
	//
	// #ifdef _PORT_FINAL
	// !ERROR
	// #endif
	//
	inline float	KludgePixels() {return 0.0f;}
	inline float	KludgeTexels() {return 0.5f;}

	//
	// Tells us if we can do windowed or not
	//
	inline bool		CanDoWindowed() {return true;}

	//
	// Tells if we're retina
	//
	inline bool		IsRetinaDevice() {return false;}

	//
	// Tells if we're a smallscreen...
	//
	bool			IsSmallScreen();

	//
	// Must we pow2?
	//
	inline bool		MustPow2Textures() {return false;}
	//inline bool		MustPow2Textures() {return true;}

	//
	// Gets resolutions
	//
	Array< Array<int> >&	GetAvailableResolutions();

	//
	// ZBuffer stuff
	//
	void			WantZBuffer(float theZNear, float theZFar);
	float			GetZDepth();
	void			EnableZBuffer();
	void			DisableZBuffer();
	void            EnableZWrites();
	void            DisableZWrites();
	void			SetZOrder(char theOrder);
	void            SetZBias(float theValue);

	void			WantStencilBuffer(int theStencilBits=1);
	void			EnableStencil();
	void			DisableStencil();
	void			EnableStencilWrites();
	void			DisableStencilWrites();
	void			SetStencilMode(char theTest=-1, int theValue=1,char theFailOp=-1, char theStencilPassZFailOp=-1, char thePassOp=-1);

	void			EnableRGBAWrites();
	void			DisableRGBAWrites();


	//
	// Saving video memory... defaults false, we're on a Mac.
	//
	inline bool     SaveVideoMemory() {return false;}

	//
	// This gives us a 3D viewing matrix, so that we can draw things at xyz coordinates and
	// just have it all look magically right without thought.
	//

	//
	// PAULNETTLE:
	// If you need to add params to this, feel free to do so, I might not have included enough stuff.
	// NOTE: THESE ARE LEGACY NOW
	//
#ifdef LEGACY_GL
	void			Set3DCamera(float theCameraX, float theCameraY, float theCameraZ,float theLookatX, float theLookatY, float theLookatZ,float theUpVectorX, float theUpVectorY, float theUpVectorZ,float theFOV);
	void			Set3DCameraOffset(float theX, float theY);
	void			Set3DRenderToCamera(float theCameraX, float theCameraY, float theCameraZ,float theLookatX, float theLookatY, float theLookatZ,float theUpVectorX, float theUpVectorY, float theUpVectorZ,float theFOV,int thePageWidth, int thePageHeight, int theResolutionWidth, int theResolutionHeight);
#endif

	//
	// For Multithreading Graphics... this is necessary for OpenGL only, seemingly.
	//
	void            EnableThreadGraphics();
	void            ThreadLock();
	void            ThreadUnlock();

	void            EnableFog(bool isLinear, int theColor, float theStart, float theEnd);
	void            DisableFog();


	//
	// VSync helper... just returning true
	//
	bool            IsTimeBeforeVSync(unsigned int lastDrawTime, unsigned int lastDrawDuration);

	//
	// Just gives us the color direction
	// OpenGL is bgra, DirectX is rgba...
	//
	// #ifdef _PORT_STARTUPGFX
	// !ERROR
	// #endif
	//
	inline char*	GetRGBOrder() {return "bgra";}

	//
	// Newer stuff
	//
	inline void*			QueryFake(char* theQuery) {return NULL;}
	inline bool			IsFakePhone() {return false;}
	inline bool			IsFakeWeb() {return false;}
	inline bool			IsFakeDevice() {return false;}
	inline int				GetFakeDeviceResolutionX() {return 0;}
	inline int				GetFakeDeviceResolutionY() {return 0;}

	int				GetPageWidth();
	int				GetPageHeight();

	longlong		GetVRAM();

	//
	// New Additions, 2021,
	// For drawing with normals and lights for Hamsterball
	//

	void			DrawShapeN(void *theBuffer, int theTriangleCount, int theVertexStride); // For OpenGL
	void			DrawShapeMT(void *theBuffer, int theTriangleCount, int theVertexStride); // For OpenGL
	int				CreateShapeN(void* theVertexList, int theVertexCount, vbindex* theIndexList, int theIndexCount, int theVertexStride);
	int				CreateShapeMT(void* theVertexList, int theVertexCount, vbindex* theIndexList, int theIndexCount, int theVertexStride);
	void			DrawIndexedShapeN(void *theVertexBuffer, void* theIndexBuffer, int theVertexCount, int theTriangleCount, int theVertexStride);
	void			DrawIndexedShapeMT(void *theVertexBuffer, void* theIndexBuffer, int theVertexCount, int theTriangleCount, int theVertexStride);

	//
	// Shaders
	//
	inline char*	GetShaderStyle() {return "GLSL";}
	int				CreateShader(char* theVertexShader, char* thePixelShader, Array<char*>& theVertexFormat);
	void			UnloadShader(int theNumber);
	void			SetShader(int theShader);

	//
	// ShaderType is 0 = Vertex, 1 = Pixel... not needed in OpenGL, but DirectX differentiates
	// between the two...
	//
	int				GetShaderGlobalRef(int theShader, int vertexOrPixel, char* theVariableName);
	void			SetShaderGlobal_Matrix(int theRef, float* theData);
	void			SetShaderGlobal_Float(int theRef, float* theData, int theCount);
	void			SetShaderGlobal_Int(int theRef, int* theData, int theCount);
	void			SetShaderGlobal_UInt(int theRef, unsigned int* theData, int theCount);
	void			SetShaderGlobal_Float2(int theRef, float* theData, int theCount);
	void			SetShaderGlobal_Float3(int theRef, float* theData, int theCount);
	void			SetShaderGlobal_Float4(int theRef, float* theData, int theCount);


	//
	// Shader shape functions-- vertex format gets bound right into the shader, so these are functions that
	// let you pump custom shapes.
	//
	void			DrawShape(int theShader, void *theBuffer, int theVCount);
	void			DrawIndexedShape(int theShader, void *theVertexBuffer, int theVertexCount, void* theIndexBuffer, int theIndexCount);
	int				CreateShape(int theShader, void* theVertexList, int theVertexCount, vbindex* theIndexList, int theIndexCount);

	//
	// For counting samples
	//
	int				CountWrites(int theCMD); // 0 = Start Counting Pixels, 1 = End Counting Pixels, 2 = Get Count Pixels Result
	bool			WatchWrites(int theCMD); // 0 = Start watching Pixels, 1 = End watching Pixels, 2 = Get whether any pixels were written

	//
	// A function for development... called via gG.Dev
	//
#ifdef _DEBUG
	void			Dev(Array<void*>& theData);
	void			SetSubTexture(int theNumber=-1, int theSubTexture=0); // For debugging subtextures attached by setting them.
#endif

	//
	// Catch-all to run rare but still done graphics things that I don't want to define a function for.
	// Currently supported:
	// BLENDOFF - Turn GL_BLEND off (an SSAO thing)
	// BLENDOF - Turn GL_BLEND on
	//
	void			Exe(longlong theQuery, void* extraData=NULL);

#ifdef _DEBUG
	void			WatchShape(int theID);
	void			WatchTexture(int theID);
#endif


};
