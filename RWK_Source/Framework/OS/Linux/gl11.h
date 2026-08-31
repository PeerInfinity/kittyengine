
#ifdef _WIN32
#define XCOMPARE strcmpi
#else
#define XCOMPARE strcasecmp
#endif

#define GL_RENDERMODE GL_TRIANGLES

#define IDENTITYMATRIX(theMatrix) {theMatrix[0][0]=1.0f;theMatrix[0][1]=0.0f;theMatrix[0][2]=0.0f;theMatrix[0][3]=0.0f;theMatrix[1][0]=0.0f;theMatrix[1][1]=1.0f;theMatrix[1][2]=0.0f;theMatrix[1][3]=0.0f;theMatrix[2][0]=0.0f;theMatrix[2][1]=0.0f;theMatrix[2][2]=1.0f;theMatrix[2][3]=0.0f;theMatrix[3][0]=0.0f;theMatrix[3][1]=0.0f;theMatrix[3][2]=0.0f;theMatrix[3][3]=1.0f;}
#define TRANSLATEMATRIX(theMatrix,theX,theY,theZ) {theMatrix[3][0]+=theX;theMatrix[3][1]+=theY;theMatrix[3][2]+=theZ;}
#define MULTIPLYMATRIX(theMatrix, theMultiplyBy) {float a_TempMultiply[4][4];memcpy(&a_TempMultiply,&theMatrix,sizeof(a_TempMultiply));theMatrix[0][0]=a_TempMultiply[0][0]*theMultiplyBy[0][0]+a_TempMultiply[0][1]*theMultiplyBy[1][0]+a_TempMultiply[0][2]*theMultiplyBy[2][0]+a_TempMultiply[0][3]*theMultiplyBy[3][0];theMatrix[0][1]=a_TempMultiply[0][0]*theMultiplyBy[0][1]+a_TempMultiply[0][1]*theMultiplyBy[1][1]+a_TempMultiply[0][2]*theMultiplyBy[2][1]+a_TempMultiply[0][3]*theMultiplyBy[3][1];theMatrix[0][2]=a_TempMultiply[0][0]*theMultiplyBy[0][2]+a_TempMultiply[0][1]*theMultiplyBy[1][2]+a_TempMultiply[0][2]*theMultiplyBy[2][2]+a_TempMultiply[0][3]*theMultiplyBy[3][2];theMatrix[0][3]=a_TempMultiply[0][0]*theMultiplyBy[0][3]+a_TempMultiply[0][1]*theMultiplyBy[1][3]+a_TempMultiply[0][2]*theMultiplyBy[2][3]+a_TempMultiply[0][3]*theMultiplyBy[3][3];theMatrix[1][0]=a_TempMultiply[1][0]*theMultiplyBy[0][0]+a_TempMultiply[1][1]*theMultiplyBy[1][0]+a_TempMultiply[1][2]*theMultiplyBy[2][0]+a_TempMultiply[1][3]*theMultiplyBy[3][0];theMatrix[1][1]=a_TempMultiply[1][0]*theMultiplyBy[0][1]+a_TempMultiply[1][1]*theMultiplyBy[1][1]+a_TempMultiply[1][2]*theMultiplyBy[2][1]+a_TempMultiply[1][3]*theMultiplyBy[3][1];theMatrix[1][2]=a_TempMultiply[1][0]*theMultiplyBy[0][2]+a_TempMultiply[1][1]*theMultiplyBy[1][2]+a_TempMultiply[1][2]*theMultiplyBy[2][2]+a_TempMultiply[1][3]*theMultiplyBy[3][2];theMatrix[1][3]=a_TempMultiply[1][0]*theMultiplyBy[0][3]+a_TempMultiply[1][1]*theMultiplyBy[1][3]+a_TempMultiply[1][2]*theMultiplyBy[2][3]+a_TempMultiply[1][3]*theMultiplyBy[3][3];theMatrix[2][0]=a_TempMultiply[2][0]*theMultiplyBy[0][0]+a_TempMultiply[2][1]*theMultiplyBy[1][0]+a_TempMultiply[2][2]*theMultiplyBy[2][0]+a_TempMultiply[2][3]*theMultiplyBy[3][0];theMatrix[2][1]=a_TempMultiply[2][0]*theMultiplyBy[0][1]+a_TempMultiply[2][1]*theMultiplyBy[1][1]+a_TempMultiply[2][2]*theMultiplyBy[2][1]+a_TempMultiply[2][3]*theMultiplyBy[3][1];theMatrix[2][2]=a_TempMultiply[2][0]*theMultiplyBy[0][2]+a_TempMultiply[2][1]*theMultiplyBy[1][2]+a_TempMultiply[2][2]*theMultiplyBy[2][2]+a_TempMultiply[2][3]*theMultiplyBy[3][2];theMatrix[2][3]=a_TempMultiply[2][0]*theMultiplyBy[0][3]+a_TempMultiply[2][1]*theMultiplyBy[1][3]+a_TempMultiply[2][2]*theMultiplyBy[2][3]+a_TempMultiply[2][3]*theMultiplyBy[3][3];theMatrix[3][0]=a_TempMultiply[3][0]*theMultiplyBy[0][0]+a_TempMultiply[3][1]*theMultiplyBy[1][0]+a_TempMultiply[3][2]*theMultiplyBy[2][0]+a_TempMultiply[3][3]*theMultiplyBy[3][0];theMatrix[3][1]=a_TempMultiply[3][0]*theMultiplyBy[0][1]+a_TempMultiply[3][1]*theMultiplyBy[1][1]+a_TempMultiply[3][2]*theMultiplyBy[2][1]+a_TempMultiply[3][3]*theMultiplyBy[3][1];theMatrix[3][2]=a_TempMultiply[3][0]*theMultiplyBy[0][2]+a_TempMultiply[3][1]*theMultiplyBy[1][2]+a_TempMultiply[3][2]*theMultiplyBy[2][2]+a_TempMultiply[3][3]*theMultiplyBy[3][2];theMatrix[3][3]=a_TempMultiply[3][0]*theMultiplyBy[0][3]+a_TempMultiply[3][1]*theMultiplyBy[1][3]+a_TempMultiply[3][2]*theMultiplyBy[2][3]+a_TempMultiply[3][3]*theMultiplyBy[3][3];}
#define SCALEMATRIX(theMatrix, theX,theY,theZ) {float a_TempScale[4][4];IDENTITYMATRIX(a_TempScale);a_TempScale[0][0]=theX;a_TempScale[1][1]=theY;a_TempScale[2][2]=theZ;MULTIPLYMATRIX(theMatrix,a_TempScale);}
#define RAD(theAngle) (theAngle*3.14159f/180.0f)
#define SIN(theAngle) ((float)sin(RAD(theAngle)))
#define COS(theAngle) (-(float)cos(RAD(theAngle)))


#define XGL(x) {x;CheckGLError();}
#define XGLLog(...) 
#define ClearGLError() {GLenum err ( glGetError() );while ( err != GL_NO_ERROR )err = glGetError();}
#define CheckGLError() _CheckGLError(__FILE__, __LINE__)

//*
#undef XGL
#define XGL(x) x
/**/

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
		OS_Core::Printf("!GL_%s - %s : %d",error,file,line);
		err = glGetError();
	}

	return;
}

namespace Graphics_Core
{
	GLint                       gSaturationShader=0;
	GLint                       gSaturation_UniformLocation;
	GLint						gNormalFrameBuffer;

	GLint						gCurrentTexture[2]={-1,-1};
	GLint						gFilter=GL_LINEAR;
	bool						gRecoverGL=false;
	bool						gPaused=false;
	
	float						gWorldMatrix[4][4];
	float						gViewMatrix[4][4];
	float						gWorkMatrix[4][4];
	
	bool						gBoundBuffer=false; // If we're drawing from a shape buffer on the video card, it's "bound" and needs to be unbound in order to draw manually... this tells us.
	
	bool						gIsRenderToTexture=false;
	int                         gRenderToTextureNumber=0;
	


	//
	// Pending is here because mFlags==0 is the marker for the space being blank.
	//
	#define GFXFLAG_PENDING 0x01
	#define GFXFLAG_RENDERER 0x02
	#define GFXFLAG_LIVE 0x04

	struct TextureStruct
	{
		int 					mWidth=0;
		int 					mHeight=0;
		int						mGLTexture=-1;
		int						mFrameBuffer=-1;
		int                     mFormat=0;
		char					mFlags=0;
		unsigned char*			mBits=NULL;
		void*                   mRendererBits=NULL;

		inline bool IsOpen() { return (mFlags == 0); }
		inline bool IsRenderer() { return (mFlags & GFXFLAG_RENDERER); }
		int						GetMyID();
		bool					MakeLive()
		{
			{
				if (mFlags&GFXFLAG_LIVE) return true;
				if (!mBits) return false;
				

				if (gRecoverGL)
				{
					if (mGLTexture!=-1) {GLuint aTexture=mGLTexture;glDeleteTextures(1, &aTexture);mGLTexture=-1;}
#ifdef _OES
					if (mFrameBuffer!=-1) {GLuint aFrameBuffer=mFrameBuffer;glDeleteFramebuffersOES(1,&aFrameBuffer);mFrameBuffer=-1;}
#else
#ifdef _EXT
					if (mFrameBuffer!=-1) {GLuint aFrameBuffer=mFrameBuffer;glDeleteFramebuffersEXT(1,&aFrameBuffer);mFrameBuffer=-1;}
#else
					if (mFrameBuffer!=-1) {GLuint aFrameBuffer=mFrameBuffer;glDeleteFramebuffers(1,&aFrameBuffer);mFrameBuffer=-1;}
#endif
#endif
					mFlags|=GFXFLAG_LIVE;mFlags^=GFXFLAG_LIVE;
				}

				GLuint aTexture;
				XGL(glGenTextures(1,&aTexture));
				mGLTexture = aTexture;
				XGL(glEnable(GL_TEXTURE_2D));
				XGL(glBindTexture(GL_TEXTURE_2D, mGLTexture));
				XGL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));

				/*
				{
				static int aLiveCount = 0;
				IOBuffer aBuffer;
				aBuffer.WriteInt(mDimensionsX);
				aBuffer.WriteInt(mDimensionsY);
				aBuffer.WriteRaw(mLoadedBits, mDimensionsX * mDimensionsY * 4);
				s    aBuffer.CommitFile(PointAtSandbox(Sprintf("Tex%d.rapt", aLiveCount++)));
				}
				/**/



				switch (mFormat)
				{
					case 0:
					case 8888:
						XGL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mWidth,mHeight, 0, GL_RGBA,GL_UNSIGNED_BYTE, mBits));
						break;
					case 1:
					case 565:
						XGL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, mWidth,mHeight, 0, GL_RGB,GL_UNSIGNED_SHORT_5_6_5, mBits));
						break;
					case 2:
					case 4444:
						XGL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mWidth,mHeight, 0, GL_RGBA,GL_UNSIGNED_SHORT_4_4_4_4, mBits));
						break;
				}

				//glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,mDimensionsX,mDimensionsY,0,GL_RGBA,GL_UNSIGNED_BYTE,mLoadedBits);

				glFlush();

				if (mFlags & GFXFLAG_RENDERER)
				{
					GLuint aTexture = mGLTexture;
					GLuint aFrameBuffer;
#ifdef _OES
					XGL(glGenFramebuffersOES(1, &aFrameBuffer));
					XGL(glBindFramebufferOES(GL_FRAMEBUFFER_OES, aFrameBuffer));
					XGL(glFramebufferTexture2DOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES,GL_TEXTURE_2D, aTexture, 0));
					XGL(glBindFramebufferOES(GL_FRAMEBUFFER_OES, gNormalFrameBuffer));
#else
#ifdef _EXT
					XGL(glGenFramebuffersEXT(1, &aFrameBuffer));
					XGL(glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, aFrameBuffer));
					XGL(glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,GL_TEXTURE_2D, aTexture, 0));
					XGL(glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, gNormalFrameBuffer));
#else
					XGL(glGenFramebuffers(1, &aFrameBuffer));
					XGL(glBindFramebuffer(GL_FRAMEBUFFER, aFrameBuffer));
					XGL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D, aTexture, 0));
					XGL(glBindFramebuffer(GL_FRAMEBUFFER, gNormalFrameBuffer));
#endif
#endif
					XGL(glFlush());

					mFrameBuffer = (int) aFrameBuffer;
				}

				mFlags |= GFXFLAG_PENDING;
				mFlags ^= GFXFLAG_PENDING;

				bool aOK=true;

				if (!glIsTexture(mGLTexture)) aOK=false;
				
				//
				// Not a reliable indicator of error...
				// On WASM GL1.1, at least, errors get left-over, so if you have any kind of
				// funky thing earlier, it's still hanging around!
				//
				//if (glGetError()!=GL_NO_ERROR) aOK=false;
#ifdef _OES
				if (mFlags&GFXFLAG_RENDERER) if (!glIsFramebufferOES(mFrameBuffer)) 
#else
#ifdef _EXT
				if (mFlags&GFXFLAG_RENDERER) if (!glIsFramebufferEXT(mFrameBuffer)) 
#else
				if (mFlags&GFXFLAG_RENDERER) if (!glIsFramebuffer(mFrameBuffer)) 
#endif
#endif
				{
					OS_Core::Printf(">>>>> [Tex %d] FRAMEBUFFER ERROR",GetMyID());
					aOK=false;
				}

				if (aOK) 
				{
					mFlags|=GFXFLAG_LIVE;
					if (!gRecoverGL)
					{
						delete[] mBits;
						mBits=NULL;
					}	
				}
				return aOK;
			}
		}
	};
	Array<struct TextureStruct> gTextureList;
	int TextureStruct::GetMyID()
	{
		for (int aCount=0;aCount<gTextureList.Size();aCount++) if (&gTextureList[aCount]==this) return aCount;
		return -1;
	}


	struct ShapeStruct
	{
		void Clean() {delete [] mV;mV=NULL;delete [] mI;mI=NULL;}

		GLuint                  mVertexBuffer=-1;
		GLuint                  mIndexBuffer=-1;
		int						mVertexCount=0;
		int						mIndexCount=0;
		int						mVertexStride=0;

		void*                   mV=NULL;
		short*                  mI=NULL;
		bool                    mOwnBackups=false;
		char					mFlags=0;

		bool MakeLive()
		{
			if (mFlags&GFXFLAG_LIVE) return true;
			if (!mV) return false;

			int aVSize=mVertexCount*mVertexStride;
			int aISize=mIndexCount*(sizeof(short));

			XGL(glGenBuffers(1,&mVertexBuffer));
			XGL(glBindBuffer(GL_ARRAY_BUFFER,mVertexBuffer));
			XGL(glBufferData(GL_ARRAY_BUFFER,aVSize,mV,GL_STATIC_DRAW));

			XGL(glGenBuffers(1,&mIndexBuffer));
			XGL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,mIndexBuffer));
			XGL(glBufferData(GL_ELEMENT_ARRAY_BUFFER,aISize,mI,GL_STATIC_DRAW));

			XGL(glBindBuffer(GL_ARRAY_BUFFER,NULL));
			XGL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,NULL));

			mFlags|=GFXFLAG_LIVE;
			mFlags|=GFXFLAG_PENDING;mFlags^=GFXFLAG_PENDING;

			if (!gRecoverGL) Clean();
			return true;
		}
	};
	Array <ShapeStruct>			gShapeList;
};

void Graphics_Core::Initialize()
{
}

void Graphics_Core::BeginRender()
{
	if (gIsRenderToTexture)
	{
	}
	else
	{
		glViewport(0, 0, gResolutionWidth, gResolutionHeight);
	}
	gCurrentTexture[0]=-2;
	gCurrentTexture[1]=-2;
}

void Graphics_Core::EndRender()
{
}




void MakePixelShaders()
{
	/*
	//
	// Load Shaders...
	//
	const char *aSaturation=
		"uniform sampler2D tex;"
		"uniform float thesaturation;"
		"void main()"
		"{"
		"   vec4 aC=texture2D(tex,gl_TexCoord[0].st);"
		"   aC*=gl_Color;"
		"   float aGrey=(aC.r+aC.g+aC.b)/3.0;"
		"   gl_FragColor=mix(vec4(aGrey,aGrey,aGrey,aC.a),aC,thesaturation);"
		"}"
		"";


	int aSaturationLen=strlen(aSaturation);
	GLint aSaturationShader=glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(aSaturationShader,1,&aSaturation,&aSaturationLen);
	glCompileShader(aSaturationShader);
	Graphics_Core::gSaturationShader=glCreateProgram();
	glAttachShader(Graphics_Core::gSaturationShader,aSaturationShader);
	glLinkProgram(Graphics_Core::gSaturationShader);

	//    aError=glGetError();
	//    printf("glLinkProgram ERROR: %s\n",glErrorToString(aError));

	Graphics_Core::gSaturation_UniformLocation=glGetUniformLocation(Graphics_Core::gSaturationShader,"thesaturation");
	*/
}

void StartGL()
{
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE,8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE,8);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,8);

	if (Graphics_Core::gZDepth>0)
	{
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,16);
		glDepthFunc(GL_LEQUAL);
	}

#ifdef _OES
	XGL(glGetIntegerv(GL_FRAMEBUFFER_BINDING_OES, &Graphics_Core::gNormalFrameBuffer));
#else
#ifdef _EXT
	XGL(glGetIntegerv(GL_FRAMEBUFFER_BINDING_EXT, &Graphics_Core::gNormalFrameBuffer));
	//Graphics_Core::gNormalFrameBuffer=0;
#else
	//XGL(glGetIntegerv(GL_FRAMEBUFFER_BINDING, &Graphics_Core::gNormalFrameBuffer));
	Graphics_Core::gNormalFrameBuffer=0;
#endif
#endif

}

void Graphics_Core::Clear(float theRed, float theGreen, float theBlue, float theAlpha)
{
	//
	// Clears the page with just a solid color...
	//
	// #ifdef _PORT_STARTUPGFX
	// !ERROR
	// #endif
	//
	EndRender();
	if (gZDepth>0)
	{
		XGL(glClearDepth(1.0f));
		XGL(glClearColor(theRed,theGreen,theBlue,theAlpha)); 
		XGL(glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT)); 
	}
	else 
	{
		XGL(glClearColor(theRed,theGreen,theBlue,theAlpha)); 
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
	if(gPaused) return;
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
	// #ifdef _PORT_TESTTRIANGLE
	// !ERROR
	// #endif
	//
	XGL(glBegin(GL_TRIANGLES));
	XGL(glColor3f(1,1,1));
	XGL(glVertex2d(50,0));
	XGL(glVertex2d(0,100));
	XGL(glVertex2d(100,100));
	XGL(glEnd());
}

void Graphics_Core::SetMatrix(int theType, void *theMatrixPtr)
{
	//
	// Set one of the system matrices (theType -> 0=world,1=view,2=perspective) with data...
	//
	// #ifdef _PORT_VIEWPORTS
	// !ERROR
	// #endif
	//
	// Set the texture matrixes too... (theType -> 3,4) for texture matrix (Multitexture)
	//
	// #ifdef _PORT_TEXMATRIX
	// !ERROR
	// #endif
	//

	switch (theType)
	{
		case 0:
			memcpy(gWorldMatrix,theMatrixPtr,4*4*sizeof(float));

			memcpy(gWorkMatrix,gWorldMatrix,4*4*sizeof(float));
			MULTIPLYMATRIX(gWorkMatrix,gViewMatrix);
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			glLoadMatrixf((float*)gWorkMatrix);
			break;
		case 1:
			memcpy(gViewMatrix,theMatrixPtr,4*4*sizeof(float));
			memcpy(gWorkMatrix,gWorldMatrix,4*4*sizeof(float));
			MULTIPLYMATRIX(gWorkMatrix,gViewMatrix);
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			glLoadMatrixf((float*)gWorkMatrix);
			break;
		case 2:
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			glLoadMatrixf((float*)theMatrixPtr);
			break;
		case 3:
			glActiveTexture(GL_TEXTURE0);
			glMatrixMode(GL_TEXTURE);
			glLoadIdentity();
			if (theMatrixPtr) glLoadMatrixf((float*)theMatrixPtr);
			break;
		case 4:
			glActiveTexture(GL_TEXTURE1);
			glMatrixMode(GL_TEXTURE);
			glLoadIdentity();
			if (theMatrixPtr) glLoadMatrixf((float*)theMatrixPtr);
			break;
	}
}

void Graphics_Core::GetMatrix(int theType, void *theMatrixPtr)
{
	//
	// #ifdef _PORT_VIEWPORTS
	// !ERROR
	// #endif
	//
	switch (theType)
	{
		case 0:memcpy(theMatrixPtr,gWorldMatrix,4*4*sizeof(float));break;
		case 1:memcpy(theMatrixPtr,gViewMatrix,4*4*sizeof(float));break;
		case 2:glGetFloatv(GL_PROJECTION_MATRIX,(float*)theMatrixPtr);break;
		#ifndef _ARB
		case 3:glActiveTexture(GL_TEXTURE0);glGetFloatv(GL_TEXTURE_MATRIX,(float*)theMatrixPtr);break;
		case 4:glActiveTexture(GL_TEXTURE1);glGetFloatv(GL_TEXTURE_MATRIX,(float*)theMatrixPtr);break;
		#else
		case 3:glActiveTextureARB(GL_TEXTURE0_ARB);glGetFloatv(GL_TEXTURE_MATRIX,(float*)theMatrixPtr);break;
		case 4:glActiveTextureARB(GL_TEXTURE1_ARB);glGetFloatv(GL_TEXTURE_MATRIX,(float*)theMatrixPtr);break;
		#endif
	}
}


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
	// #ifdef _PORT_VIEWPORTS
	// !ERROR
	// #endif
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
	// #ifdef _PORT_PRIMITIVES1
	// !ERROR
	// #endif
	//
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
	glDisable(GL_LIGHTING);
	glShadeModel(GL_SMOOTH);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

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
	// #ifdef _PORT_PRIMITIVES1
	// !ERROR
	// #endif
	//
	glDisable(GL_CULL_FACE);
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
	// #ifdef _PORT_PRIMITIVES1
	// !ERROR
	// #endif
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
		glBindBuffer(GL_ARRAY_BUFFER,NULL);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,NULL);
		gBoundBuffer=false;
	}

	Vertex2DRef *aRef=(Vertex2DRef*)theBuffer;
	glVertexPointer(3,GL_FLOAT,theVertexStride,&aRef->mX);
	glColorPointer(4,GL_UNSIGNED_BYTE,theVertexStride,&aRef->mDiffuse);
	glTexCoordPointer(2,GL_FLOAT,theVertexStride,&aRef->mTextureU);
	glDrawArrays(GL_TRIANGLES,0,theTriangleCount*3);

}


void Graphics_Core::SetAlphaThreshold(float theValue)
{
#ifdef _PORT_3DEX
	!ERROR
#endif
}

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
	// #ifdef _PORT_PRIMITIVES2
	// !ERROR
	// #endif
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
		glBindBuffer(GL_ARRAY_BUFFER,NULL);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,NULL);
		gBoundBuffer=false;
	}


	Vertex2DRef *aRef=(Vertex2DRef*)theVertexBuffer;
	XGL(glVertexPointer(3,GL_FLOAT,theVertexStride,&aRef->mX));
	XGL(glColorPointer(4,GL_UNSIGNED_BYTE,theVertexStride,&aRef->mDiffuse));
	XGL(glTexCoordPointer(2,GL_FLOAT,theVertexStride,&aRef->mTextureU));
	glDrawElements(GL_TRIANGLES,theTriangleCount*3,GL_UNSIGNED_SHORT,theIndexBuffer);
}

void Graphics_Core::Clip()
{
	glDisable(GL_SCISSOR_TEST);
}

void Graphics_Core::Clip(float theX1, float theY1, float theX2, float theY2)
{
	//
	// Clip drawing to the specified screen area...
	//
	// #ifdef _PORT_CLIPPING
	// !ERROR
	// #endif
	//

	//
	// #ifdef _PORT_RENDERTO
	// !ERROR - Clipping usually needs to branch if you're rendering to texture...
	// #endif
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

	glEnable(GL_SCISSOR_TEST);
	glScissor(aLeft,aTop,aWidth,aHeight);
}


void Graphics_Core::UnloadTexture(int theNumber)
{
	//
	// Unloads a texture
	//
	// #ifdef _PORT_LOADTEX
	// !ERROR
	// #endif
	//
	GLuint aTexture=gTextureList[theNumber].mGLTexture;
	if (aTexture!=0) glDeleteTextures(1,&aTexture);
	GLuint aFrameBuffer=gTextureList[theNumber].mFrameBuffer;
#ifdef _OES
	if (aFrameBuffer!=0) glDeleteFramebuffersOES(1,&aFrameBuffer);
#else
#ifdef _EXT
	if (aFrameBuffer!=0) glDeleteFramebuffersEXT(1,&aFrameBuffer);
#else
	if (aFrameBuffer!=0) glDeleteFramebuffers(1,&aFrameBuffer);
#endif
#endif
	gTextureList[theNumber].mGLTexture=0;
	gTextureList[theNumber].mFrameBuffer=0;
	delete [] gTextureList[theNumber].mBits;
	delete [] gTextureList[theNumber].mRendererBits;
	gTextureList[theNumber].mBits=NULL;
	gTextureList[theNumber].mRendererBits=NULL;

	if (gCurrentTexture[0]==theNumber) SetTexture(-1);
	if (gCurrentTexture[1]==theNumber) SetMultiTexture(-1);
}

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
#ifdef _OEM		
		glBindFramebufferOEM(GL_FRAMEBUFFER_OEM,0);
#else
#ifdef _EXT
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);
#else
		glBindFramebuffer(GL_FRAMEBUFFER,0);
#endif
#endif
		if (gIsRenderToTexture) glViewport(aHoldViewport[0],aHoldViewport[1],aHoldViewport[2],aHoldViewport[3]);
		gIsRenderToTexture=false;
	}
	else 
	{
		if (!gIsRenderToTexture) glGetIntegerv(GL_VIEWPORT,aHoldViewport);
#ifdef _OEM		
		glBindFramebufferOEM(GL_FRAMEBUFFER_OEM,gTextureList[theTexture].mFrameBuffer);
#else
#ifdef _EXT
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,gTextureList[theTexture].mFrameBuffer);
#else
		glBindFramebuffer(GL_FRAMEBUFFER,gTextureList[theTexture].mFrameBuffer);
#endif
#endif
		glViewport(0,0,gTextureList[theTexture].mWidth,gTextureList[theTexture].mHeight);
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
	// #ifdef _PORT_RENDERTO_READ
	// !ERROR
	// #endif
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

		// Crash here?  Was your texture readable?
		glReadPixels(0,0,gTextureList[gRenderToTextureNumber].mWidth,gTextureList[gRenderToTextureNumber].mHeight,thePixelFormat,thePixelType,gTextureList[gRenderToTextureNumber].mRendererBits);
		return gTextureList[gRenderToTextureNumber].mRendererBits;
	}
	return NULL;
}


void Graphics_Core::GetTextureDimensions(int theNumber, int *theWidth, int *theHeight)
{
	*theWidth=gTextureList[theNumber].mWidth;
	*theHeight=gTextureList[theNumber].mHeight;
}

void Graphics_Core::Filter(bool theState)
{
	if (theState) gFilter=GL_LINEAR;
	else gFilter=GL_NEAREST;
	if (gCurrentTexture[0]>=0) SetTexture(gCurrentTexture[0]);
	if (gCurrentTexture[1]>=0) SetMultiTexture(gCurrentTexture[1]);
}

void Graphics_Core::SetTexture(int theNumber,char* specialOp)
{
	//
	// Sets the current texture for the system to use.
	// It is wise to remember what texture is currently selected so that you can optimize drawing with no 
	// texture (for fills, etc).  Also, be prepared to set the filter for textures, since that is coming later.
	//
	// #ifdef _PORT_LOADTEX
	// !ERROR
	// #endif
	//
	XGL(glActiveTexture(GL_TEXTURE0));
	XGL(glClientActiveTexture(GL_TEXTURE0));

	gCurrentTexture[0]=theNumber;
	if (theNumber==-1)
	{
		XGL(glBindTexture(GL_TEXTURE_2D,0));	
		XGL(glDisable(GL_TEXTURE_2D));
	}
	else 
	{
		XGL(glEnable(GL_TEXTURE_2D));
		if (!(gTextureList[theNumber].mFlags&GFXFLAG_LIVE)) if (!gTextureList[theNumber].MakeLive()) return;

		XGL(glBindTexture(GL_TEXTURE_2D,gTextureList[theNumber].mGLTexture));	
		XGL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,gFilter));
		XGL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,gFilter));
	}
}

void Graphics_Core::SetMultiTexture(int theStage, int theNumber, char* specialOp)
{
	XGL(glActiveTexture(GL_TEXTURE1));
	XGL(glClientActiveTexture(GL_TEXTURE1));

	gCurrentTexture[1]=theNumber;
	if (theNumber==-1)
	{
		XGL(glBindTexture(GL_TEXTURE_2D,0));	
		XGL(glDisable(GL_TEXTURE_2D));
	}
	else 
	{
		XGL(glEnable(GL_TEXTURE_2D));
		if (!(gTextureList[theNumber].mFlags&GFXFLAG_LIVE)) if (!gTextureList[theNumber].MakeLive()) return;
		XGL(glBindTexture(GL_TEXTURE_2D,gTextureList[theNumber].mGLTexture));	
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		XGL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,gFilter));
		XGL(glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,gFilter));
	}
	glActiveTexture(GL_TEXTURE0);
}

int	Graphics_Core::CreateShape(void* theVertexList, int theVertexCount, short* theIndexList, int theIndexCount, int theVertexStride)
{
	if(gPaused) return 0;
	//OS_Core::Printf("Graphics_Core::CreateShape() called");

	int aShapeSpot=0;
	for (aShapeSpot=0;aShapeSpot<=gShapeList.Size();aShapeSpot++) if (!gShapeList[aShapeSpot].mFlags) break;

	ShapeStruct& aShape=gShapeList[aShapeSpot];

	int aVSize=theVertexCount*theVertexStride;
	int aISize=theIndexCount*(sizeof(short));

	aShape.mV=malloc(aVSize);memcpy(aShape.mV,theVertexList,aVSize);
	aShape.mI=(short*)malloc(aISize);memcpy(aShape.mI,theIndexList,aISize);

	aShape.mVertexCount=theVertexCount;
	aShape.mIndexCount=theIndexCount;
	aShape.mVertexStride=theVertexStride;
	aShape.mFlags=GFXFLAG_PENDING;

	return aShapeSpot;
}

void Graphics_Core::UnloadShape(int theNumber)
{
	//
	// Destroy a vertexbuffer
	//
	// #ifdef _PORT_PRIMITIVES3
	// !ERROR
	// #endif
	//
	if (theNumber<0 || theNumber>=gShapeList.Size()) return;
	ShapeStruct& aShape=gShapeList[theNumber];

	XGL(glDeleteBuffers(1,&aShape.mVertexBuffer));
	XGL(glDeleteBuffers(1,&aShape.mIndexBuffer));

	aShape.mVertexBuffer=0;
	aShape.mIndexBuffer=0;
	aShape.Clean();
	aShape.mFlags=0;
}

void Graphics_Core::DrawShape(int theNumber)
{
	//
	// Draw a vertexbuffer...
	//
	// #ifdef _PORT_PRIMITIVES3
	// !ERROR
	// #endif
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
	if (!(aShape.mFlags&GFXFLAG_LIVE)) aShape.MakeLive();

	XGL(glBindBuffer(GL_ARRAY_BUFFER,aShape.mVertexBuffer));
	XGL(glVertexPointer(3,GL_FLOAT,aShape.mVertexStride,(void*)offsetof(Vertex2DRef,mX)));
	XGL(glColorPointer(4,GL_UNSIGNED_BYTE,aShape.mVertexStride,(void*)offsetof(Vertex2DRef,mDiffuse)));
	XGL(glTexCoordPointer(2,GL_FLOAT,aShape.mVertexStride,(void*)offsetof(Vertex2DRef,mTextureU)));
	XGL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,aShape.mIndexBuffer));
	gBoundBuffer=true;

	glDrawElements(GL_RENDERMODE,aShape.mIndexCount,GL_UNSIGNED_SHORT,NULL);
}

void Graphics_Core::CullCCW()
{
	XGL(glEnable(GL_CULL_FACE));
	XGL(glFrontFace(GL_CW));
}

void Graphics_Core::CullCW()
{
	XGL(glEnable(GL_CULL_FACE));
	XGL(glFrontFace(GL_CCW));
}

void Graphics_Core::RenderNormal()
{
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

void Graphics_Core::Clamp()
{
	XGL(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	XGL(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
}

void Graphics_Core::Wrap()
{
	XGL(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
	XGL(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
}

void Graphics_Core::RenderAdditive()
{
	XGL(glBlendFunc(GL_SRC_ALPHA,GL_ONE));
}

void Graphics_Core::RenderMultiply()
{
	XGL(glBlendFunc(GL_ZERO,GL_SRC_COLOR));
}

void Graphics_Core::RenderWhite(bool theState)
{
	if (theState)
	{
		XGL(glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_BLEND));
	}
	else 
	{
		XGL(glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE));
	}
}

void Graphics_Core::SetRenderWhiteColor(float *theRGBA)
{
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
	if (theSaturation<1.0f)
	{
		XGL(glUseProgram(gSaturationShader));
		XGL(glUniform1f(gSaturation_UniformLocation,theSaturation));
	}
	else 
	{
		XGL(glUseProgram(0));
	}
}

void Graphics_Core::RenderBlurred(float theBlurLevel)
{
}


void ReloadEverything()
{
	if (Graphics_Core::gRecoverGL)
	{
		XGL(glFlush());
		for (int aCount=0;aCount<Graphics_Core::gTextureList.Size();aCount++) if (Graphics_Core::gTextureList[aCount].mFlags&GFXFLAG_LIVE) {Graphics_Core::gTextureList[aCount].mFlags^=GFXFLAG_LIVE;Graphics_Core::gTextureList[aCount].mFlags^=GFXFLAG_PENDING;}
		for (int aCount=0;aCount<Graphics_Core::gShapeList.Size();aCount++) if (Graphics_Core::gShapeList[aCount].mFlags&GFXFLAG_LIVE) {Graphics_Core::gShapeList[aCount].mFlags^=GFXFLAG_LIVE;Graphics_Core::gShapeList[aCount].mFlags^=GFXFLAG_PENDING;}
	}
}


void Graphics_Core::EnableZBuffer()
{
	//
	// #ifdef _PORT_ZBUFFER
	// !ERROR
	// #endif
	//
	if (gZDepth>0)
	{
		XGL(glEnable(GL_DEPTH_TEST));
		gZBuffer=true;
		XGL(glDepthFunc(GL_LEQUAL));
	}
}

void Graphics_Core::DisableZBuffer()
{
	//
	// #ifdef _PORT_ZBUFFER
	// !ERROR
	// #endif
	//
	glDisable(GL_DEPTH_TEST);
	gZBuffer=false;
}

void Graphics_Core::EnableFog(bool isLinear, int theColor, float theStart, float theEnd)
{
	if(gGraphicsPaused) return;
	//OS_Core::Printf("Graphics_Core::EnableFog() called");
	theStart = GetZDepth() * theStart;
	theEnd = GetZDepth() * theEnd;

	int aC1 = theColor & 0x0000FF;
	int aC2 = (theColor & 0x00FF00) >> 8;
	int aC3 = (theColor & 0xFF0000) >> 16;

	float aColor[4];
	aColor[0] = (float) aC1 / 255;
	aColor[1] = (float) aC2 / 255;
	aColor[2] = (float) aC3 / 255;
	aColor[3] = 1;

	XGL(glEnable(GL_FOG));
	XGL(glFogf(GL_FOG_MODE, GL_LINEAR));
	XGL(glFogfv(GL_FOG_COLOR, aColor));
	XGL(glFogf(GL_FOG_DENSITY, .33f));
	XGL(glHint(GL_FOG_HINT, GL_FASTEST));
	XGL(glFogf(GL_FOG_START, theStart));
	XGL(glFogf(GL_FOG_END, theEnd));
}

void Graphics_Core::DisableFog()
{
	if(gPaused) return;
	XGL(glDisable(GL_FOG));
}

void Graphics_Core::EnableZWrites()
{
	if(gPaused) return;
	XGL(glDepthMask(GL_TRUE));
}

void Graphics_Core::DisableZWrites()
{
	if(gPaused) return;
	XGL(glDepthMask(GL_FALSE));
}

void Graphics_Core::SetZOrder(char theOrder)
{
	if(gPaused) return;
	//OS_Core::Printf("Graphics_Core::SetZOrder() called");
	if (theOrder==0) XGL(glDepthFunc(GL_LEQUAL));
	if (theOrder==1) XGL(glDepthFunc(GL_GREATER));
	if (theOrder==2) XGL(glDepthFunc(GL_EQUAL));
	if (theOrder==3) XGL(glDepthFunc(GL_LESS));
	if (theOrder==4) XGL(glDepthFunc(GL_GEQUAL));
}

void Graphics_Core::SetZBias(float theValue)
{
	if(gPaused) return;
	//OS_Core::Printf("SetZBias() called");
	XGL(glPolygonOffset(1.0f, theValue));
}

void Graphics_Core::Wireframe(bool theState)
{
	if (theState) 
	{
		XGL(glPolygonMode( GL_FRONT_AND_BACK,GL_LINE));
	}
	else 
	{
		XGL(glPolygonMode( GL_FRONT_AND_BACK,GL_FILL));
	}
}

void Graphics_Core::DrawShapeN(void *theBuffer, int theTriangleCount, int theVertexStride)
{
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

	Vertex2DNRef* aRef=(Vertex2DNRef*)theBuffer;

	XGL(glDisableClientState(GL_COLOR_ARRAY));
	XGL(glEnableClientState(GL_NORMAL_ARRAY));

	XGL(glVertexPointer(3,GL_FLOAT,theVertexStride,&aRef->mX));
	XGL(glNormalPointer(GL_FLOAT,theVertexStride,&aRef->mNX));

	XGL(glClientActiveTexture(GL_TEXTURE0));
	XGL(glActiveTexture(GL_TEXTURE0));
	XGL(glEnableClientState(GL_TEXTURE_COORD_ARRAY));
	XGL(glTexCoordPointer(2,GL_FLOAT,theVertexStride,&aRef->mTextureU));

	XGL(glDrawArrays(GL_TRIANGLES,0,theTriangleCount*3));

	XGL(glDisableClientState(GL_NORMAL_ARRAY));
	XGL(glEnableClientState(GL_COLOR_ARRAY));

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

/*
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

void Graphics_Core::DrawLine(void *theBuffer, int theLineCount, int theVertexStride)
{
}















