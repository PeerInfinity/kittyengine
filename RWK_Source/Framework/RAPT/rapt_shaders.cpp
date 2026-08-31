#include "rapt_shaders.h"
#ifndef LEGACY_GL

GlobalShaders gSh;

void Vertex::ManualTransform(Vertex* theVectors, int theVectorCount, Matrix& theMatrix, Viewport& theViewport) {for (int aCount=0;aCount<theVectorCount;aCount++) {Vector aResult=theMatrix.ProcessPointNormalized(theVectors->mPos);theVectors->mPos.mX=(aResult.mX*0.5f+0.5f)*theViewport.mWidth;theVectors->mPos.mY=(aResult.mY*-0.5f+0.5f)*theViewport.mHeight;theVectors->mPos.mZ=gG.FixZ(aResult.mZ);theVectors++;}}
void VertexMT::ManualTransform(VertexMT* theVectors, int theVectorCount, Matrix& theMatrix, Viewport& theViewport) {for (int aCount=0;aCount<theVectorCount;aCount++) {Vector aResult=theMatrix.ProcessPointNormalized(theVectors->mPos);theVectors->mPos.mX=(aResult.mX*0.5f+0.5f)*theViewport.mWidth;theVectors->mPos.mY=(aResult.mY*-0.5f+0.5f)*theViewport.mHeight;theVectors->mPos.mZ=gG.FixZ(aResult.mZ);theVectors++;}}

void ShaderN::Load()
{
	mFogLimit=Point(gG.mZRange.mY-(gG.mZRange.mY/4),gG.mZRange.mY);
	String aSH=
		"info {ShaderN}"
		"global"
		"{"
		"	combomatrix		gComboMatrix;"
		"	worldmatrixn	gWorldMatrixN;"
		"	worldviewmatrix	gWorldViewMatrix;"
		"	sampler			gTexture;"
		"	vector			gLightDir;"
		"	float2			gFogLimit;"
		"}"			
		"input"		
		"{"			
		"	xyz 			mXYZ;"
		"	normal			mNormal;"
		"	uv				mUV;"			
		"}"			
		"vertex"	
		"{"			
		"	xyz				mXYZ;"
		"	uv				mUV;"
		"	float4			mNormal;"
		"	float			mFogPos;"
		"	float			mDotLight;"
		"}"			
		"vsmain()"
		"{"		
		"	vertex.mXYZ=mul(input.mXYZ,gComboMatrix);"
		"	vertex.mNormal=mul(input.mNormal,gWorldMatrixN);"
		"	vertex.mUV=input.mUV;"
		"	vertex.mFogPos=mul(input.mXYZ,gWorldViewMatrix).z;"
		"	vertex.mDotLight=dot(global.gLightDir,-vertex.mNormal.xyz);"
		"}"
		"psmain()"
		"{"
		"	pixel=tex2D(gTexture,vertex.mUV);"
		//"	pixel=float4(1,0,0,1);"
		"	pixel.rgb*=clamp(vertex.mDotLight*2.0f,.25f,1.5f);"
		"	if (vertex.mFogPos>gFogLimit.x) pixel.a*=smoothstep(gFogLimit.y,gFogLimit.x,vertex.mFogPos);"
		//"	pixel.rgb*=dot(global.gLightDir,-vertex.mNormal.xyz);"
		"}";

	Shader::Load(aSH);
	mLightDirRef=GetGlobalRef("gLightDir");
	mFogLimitRef=GetGlobalRef("gFogLimit");
	SetShaderHook(SHADERHOOK({theShader->SetGlobal(mLightDirRef,(float*)&mLightDir,3);theShader->SetGlobal(mFogLimitRef,(float*)&mFogLimit,2);}));
	//SetShaderHook(SHADERHOOK({theShader->SetGlobal(mLightDirRef,(float*)&mLightDir,3);}));
}

void ShaderNColored::Load()
{
	mFogLimit=Point(gG.mZRange.mY-(gG.mZRange.mY/4),gG.mZRange.mY);
	String aSH=
		"info {ShaderNColored}"
		"global"
		"{"
		"	combomatrix		gComboMatrix;"
		"	worldmatrixn	gWorldMatrixN;"
		"	worldviewmatrix	gWorldViewMatrix;"
		"	sampler			gTexture;"
		"	vector			gLightDir;"
		"	float2			gFogLimit;"
		"	gG.color		gColor;"
		"}"			
		"input"		
		"{"			
		"	xyz 			mXYZ;"
		"	normal			mNormal;"
		"	uv				mUV;"			
		"}"			
		"vertex"	
		"{"			
		"	xyz				mXYZ;"
		"	uv				mUV;"
		"	float4			mNormal;"
		"	float			mFogPos;"
		"	float			mDotLight;"
		"}"			
		"vsmain()"
		"{"		
		"	vertex.mXYZ=mul(input.mXYZ,gComboMatrix);"
		"	vertex.mNormal=mul(input.mNormal,gWorldMatrixN);"
		"	vertex.mUV=input.mUV;"
		"	vertex.mFogPos=mul(input.mXYZ,gWorldViewMatrix).z;"
		"	vertex.mDotLight=dot(global.gLightDir,-vertex.mNormal.xyz);"
		"}"
		"psmain()"
		"{"
		"	pixel=tex2D(gTexture,vertex.mUV);"
		"	pixel*=gColor;"
		"	pixel.rgb*=clamp(vertex.mDotLight*2.0f,.25f,1.5f);"
		"	if (vertex.mFogPos>gFogLimit.x) pixel.a*=smoothstep(gFogLimit.y,gFogLimit.x,vertex.mFogPos);"
		//"	pixel.rgb*=dot(global.gLightDir,-vertex.mNormal.xyz);"
		"}";

	Shader::Load(aSH);
	mLightDirRef=GetGlobalRef("gLightDir");
	mFogLimitRef=GetGlobalRef("gFogLimit");
	SetShaderHook(SHADERHOOK({theShader->SetGlobal(mLightDirRef,(float*)&mLightDir,3);theShader->SetGlobal(mFogLimitRef,(float*)&mFogLimit,2);}));
	//SetShaderHook(SHADERHOOK({theShader->SetGlobal(mLightDirRef,(float*)&mLightDir,3);}));
}

void ShaderGizmo::Load()
{
	String aSH=
		"info {ShaderGizmo}"
		"global"
		"{"
		"	combomatrix		gComboMatrix;"
		"	vector			gCenter;"
		"	point			gRange;"
		"}"
		"input"
		"{"
		"	xyz 			mXYZ;"
		"	rgba			mColor;"
		"	uv				mUV;"
		"}"
		"vertex"
		"{"
		"	xyz				mXYZ;"
		"	color			mColor;"
		"	uv				mUV;"
		"	float3			mVPos;"
		"}"
		"vsmain()"
		"{"
		"	vertex.mXYZ=mul(input.mXYZ,gComboMatrix);"
		"	vertex.mColor=input.mColor;"
		"	vertex.mUV=input.mUV;"
		"	vertex.mVPos=input.mXYZ.xyz;"
		"}"
		"psmain()"
		"{"
		"	float aX=vertex.mVPos.x-gCenter.x;"
		"	float aY=vertex.mVPos.y-gCenter.y;"
		"	float aZ=vertex.mVPos.z-gCenter.z;"
		"	float aDist=aX*aX+aY*aY+aZ*aZ;"
		"	if (aDist>=gRange.x && aDist<=gRange.y) pixel=vertex.mColor;"
		"	else pixel=float4(1,1,1,.1);"
		"}";

	Shader::Load(aSH);
	mCenterRef=GetGlobalRef("gCenter");
	mRangeRef=GetGlobalRef("gRange");
	SetShaderHook(SHADERHOOK(
		{
			theShader->SetGlobal(mCenterRef,(float*)&mCenter,3);
			theShader->SetGlobal(mRangeRef,(float*)&mRange,2);
		}
	));
}

void ShaderS::Load()
{
	String aSH=
		"info {ShaderS}"
		"global"
		"{"
		"	combomatrix		gComboMatrix;"
		"	gG.color		gColor;"
		"	gG.zbias		gZBias;"
		"}"			
		"input"		
		"{"			
		"	xyz 			mXYZ;"			
		"}"			
		"vertex"	
		"{"			
		"	xyz				mXYZ;"
		"}"			
		"vsmain()"
		"{"		
		"	vertex.mXYZ=mul(input.mXYZ,gComboMatrix);"
		"}"
		"psmain()"
		"{"
		"	pixel=gColor;"
		"	zdepth=pixel.zdepth/gZBias;"
		"}";

	Shader::Load(aSH);
}

void ShaderV::Load()
{
	String aSH=
		"info {ShaderV}"
		"global"
		"{"
		"	combomatrix		gComboMatrix;"
		"	sampler			gTexture;"
		"	gG.zbias		gZBias;"
		"}"			
		"input"		
		"{"			
		"	xyz 			mXYZ;"
		"	rgba			mColor;"
		"	uv				mUV;"
		"}"			
		"vertex"	
		"{"			
		"	xyz					mXYZ;"
		"	color				mColor;"
		"	uv					mUV;"
		"}"			
		"vsmain()"
		"{"		
		"	vertex.mXYZ=mul(input.mXYZ,gComboMatrix);"
		"	vertex.mColor=input.mColor;"
		"	vertex.mUV=input.mUV;"
		"}"


		"float LD(float d,float zNear,float zFar)"
		"{"
		"	float z_n=2.0*d-1.0;"
		"	return 2.0*zNear*zFar/(zFar+zNear-z_n*(zFar-zNear));"
		"}"

	"psmain()"
		"{"
		"	pixel=tex2D(gTexture,vertex.mUV);"
		"	pixel*=vertex.mColor;"
		"	zdepth=pixel.zdepth/gZBias;"
		//"	pixel.rgb=float3(vertex.mXYZ.w);"//LD(pixel.zdepth,0.,1.));"
		//"	zdepth=LinearizeDepth(pixel.zdepth);"
		//"	if (pixel.a<.001f) {discard;}"
		"}";

	Shader::Load(aSH);
}

void ShaderC::Load()
{
	String aSH=
		"info {ShaderC}"
		"global"
		"{"
		"	combomatrix			gComboMatrix;"
		"	sampler				gTexture;"
		"	gG.zbias			gZBias;"
		"	gG.color			gColor;"
		"	gG.alphathreshold	gAlpha;"
		"}"			
		"input"		
		"{"			
		"	xyz 			mXYZ;"
		"	uv				mUV;"
		"}"			
		"vertex"	
		"{"			
		"	xyz					mXYZ;"
		"	uv					mUV;"
		"}"			
		"vsmain()"
		"{"		
		"	vertex.mXYZ=mul(input.mXYZ,gComboMatrix);"
		"	vertex.mUV=input.mUV;"
		"}"
		"psmain()"
		"{"
		"	pixel=tex2D(gTexture,vertex.mUV);"
		"	if (pixel.a<global.gAlpha) {discard;}"
		"	pixel*=gColor;"
		"	zdepth=pixel.zdepth/gZBias;"
		"}";

	Shader::Load(aSH);
}


void ShaderVMT::Load()
{
	String aSH=
		"info {ShaderVMT}"
		"global"
		"{"
		"	combomatrix		gComboMatrix;"
		"	sampler			gTexture;"
		"	sampler			gTexture2;"
		"}"			
		"input"		
		"{"			
		"	xyz 			mXYZ;"			
		"	rgba			mColor;"
		"	uv				mUV;"		
		"	uv				mUV2;"		
		"}"			
		"vertex"	
		"{"			
		"	xyz				mXYZ;"
		"	color			mColor;"
		"	uv				mUV;"
		"	uv				mUV2;"
		"}"			
		"vsmain()"
		"{"		
		"	vertex.mXYZ=mul(input.mXYZ,gComboMatrix);"
		"	vertex.mColor=input.mColor;"
		"	vertex.mUV=input.mUV;"
		"	vertex.mUV2=input.mUV2;"
		"}"
		"psmain()"
		"{"
		"	pixel=tex2D(gTexture,vertex.mUV);"
		"	float4 aPixel2=tex2D(gTexture2,vertex.mUV2);"
		"	if (aPixel2.a>0.0f) {"
		"		pixel.rgb=(pixel.rgb*(1.0f-aPixel2.a))+(aPixel2.rgb*aPixel2.a);"
		"		pixel.a=max(pixel.a,aPixel2.a);"
		"	}"
		"	pixel*=vertex.mColor;"

		//return underlying + (1 - underlying.a) * overlying;
		//"	if (pixel.a<global.gAlphaThreshold) {discard;}"
		"}";

	Shader::Load(aSH);
}

/*
void ShaderRoundedRect::Load()
{
	String aSH=
		"global"
		"{"
		"	combomatrix			gMatrix;"
		"	float2				gDimensions;"
		"	float				gRadius;"
		"}"			
		"input"		
		"{"			
		"	xyz 			mXYZ;"
		"	rgba			mColor;"
		"	uv				mUV;"		
		"}"			
		"vertex"	
		"{"			
		"	xyz				mXYZ;"
		"	color			mColor;"
		"	uv				mUV;"
		"}"
		"float CircleDist(float2 coord, float2 corner) {return length(coord-corner);}"
		"float GetA(float2 coords)"
		"{"
		"	if ((coords.x>gRadius && coords.x<gDimensions.x-gRadius) || (coords.y>gRadius && coords.y<gDimensions.y-gRadius)) return 1.0f;"
		"	else"
		"	{"
		"		float aA=CircleDist(coords,float2(gRadius, gRadius));"
		"		aA=min(aA,CircleDist(coords,float2(gDimensions.x-gRadius-1.0f, gRadius)));"
		"		aA=min(aA,CircleDist(coords,float2(gRadius, gDimensions.y-gRadius-1.0f)));"
		"		aA=min(aA,CircleDist(coords,float2(gDimensions.x-gRadius-1.0f, gDimensions.y-gRadius-1.0f)));"
		"		if (aA>=gRadius+1.0f) return 0.0f;"
		"		else if (aA<gRadius) return 1.0f;"
		"		else return 1.0f-(aA-gRadius);"
		"	}"
		"}"
		""
		"vsmain()"
		"{"
		"	vertex.mXYZ=mul(input.mXYZ,gMatrix);"
		"	vertex.mColor=input.mColor;"
		"	vertex.mUV=input.mUV;"
		"}"
		"psmain()"
		"{"
		"	pixel=vertex.mColor;"
		"	pixel.a=GetA(vertex.mUV*gDimensions)*vertex.mColor.a;"
		"	if (pixel.a<=0.0f) discard;"
		//"	if (ShouldDiscard(vertex.mUV*gDimensions)) {discard;}"
		"}";

	Shader::Load(aSH);
	mDimensionRef=GetGlobalRef("gDimensions");
	mRadiusRef=GetGlobalRef("gRadius");
}

void ShaderRoundedRect::Draw(Rect theRect, float theRadius)
{
	theRect=theRect.Translate(gG.mTranslate);
	Vertex aV[4];
	aV[0]=Vertex(theRect.mX,theRect.mY,0,gG.mColorInt,Point(0,0));
	aV[1]=Vertex(theRect.mX+theRect.mWidth,theRect.mY,0,gG.mColorInt,Point(1,0));
	aV[2]=Vertex(theRect.mX,theRect.mY+theRect.mHeight,0,gG.mColorInt,Point(0,1));
	aV[3]=Vertex(theRect.mX+theRect.mWidth,theRect.mY+theRect.mHeight,0,gG.mColorInt,Point(1,1));
	Draw(aV,Point(theRect.mWidth,theRect.mHeight),theRadius);
}

void ShaderRoundedRect::Draw(Quad theRect, float theRadius)
{
	theRect.Translate(gG.mTranslate);
	Vertex aV[4];
	aV[0]=Vertex(Vector(theRect.mCorner[0]),gG.mColorInt,Point(0,0));
	aV[1]=Vertex(Vector(theRect.mCorner[1]),gG.mColorInt,Point(1,0));
	aV[2]=Vertex(Vector(theRect.mCorner[2]),gG.mColorInt,Point(0,1));
	aV[3]=Vertex(Vector(theRect.mCorner[3]),gG.mColorInt,Point(1,1));
	Draw(aV,Point(theRect.GetLine(0,1).Len(),theRect.GetLine(0,2).Len()),theRadius);
}

void ShaderRoundedRect::Draw(Vertex* theV, float theRadius)
{
	Point aDims;
	aDims.mX=gMath.Distance(theV[0].ToVector(),theV[1].ToVector());
	aDims.mY=gMath.Distance(theV[0].ToVector(),theV[2].ToVector());

	gG.SetShader(this);
	SetGlobal(mDimensionRef,(float*)&aDims,2);
	SetGlobal(mRadiusRef,(float*)&theRadius,1);
	Shader::DrawShape(theV,4,Index4Helper(),6);
}

void ShaderRoundedRect::Draw(Vertex* theV, Point theDims, float theRadius)
{
	gG.SetShader(this);
	SetGlobal(mDimensionRef,(float*)&theDims,2);
	SetGlobal(mRadiusRef,(float*)&theRadius,1);
	Shader::DrawShape(theV,4,Index4Helper(),6);
}

void ShaderRoundedRect::DrawShape(void* theV, int theVCount, vbindex* theI, int theICount)
{
	Vertex* aV=(Vertex*)theV;
	Point aDims;
	aDims.mX=gMath.Distance(aV[0].ToVector(),aV[1].ToVector());
	aDims.mY=gMath.Distance(aV[0].ToVector(),aV[2].ToVector());
	float aRadius=_min(aDims.mX,aDims.mY)/2;

	gG.SetShader(this);
	SetGlobal(mDimensionRef,(float*)&aDims,2);
	SetGlobal(mRadiusRef,(float*)&aRadius,1);
	Shader::DrawShape(theV,theVCount,theI,theICount);
}

void ShaderRoundedRectInvert::Load()
{
	String aSH=
		"global"
		"{"
		"	combomatrix			gMatrix;"
		"	float2				gDimensions;"
		"	float				gRadius;"
		"}"			
		"input"		
		"{"			
		"	xyz 			mXYZ;"
		"	rgba			mColor;"
		"	uv				mUV;"		
		"}"			
		"vertex"	
		"{"			
		"	xyz				mXYZ;"
		"	color			mColor;"
		"	uv				mUV;"
		"}"
		"float CircleDist(float2 coord, float2 corner) {return length(coord-corner);}"
		"float GetA(float2 coords)"
		"{"
		"	if ((coords.x>gRadius && coords.x<gDimensions.x-gRadius) || (coords.y>gRadius && coords.y<gDimensions.y-gRadius)) return 1.0f;"
		"	else"
		"	{"
		"		float aA=CircleDist(coords,float2(gRadius, gRadius));"
		"		aA=min(aA,CircleDist(coords,float2(gDimensions.x-gRadius-1.0f, gRadius)));"
		"		aA=min(aA,CircleDist(coords,float2(gRadius, gDimensions.y-gRadius-1.0f)));"
		"		aA=min(aA,CircleDist(coords,float2(gDimensions.x-gRadius-1.0f, gDimensions.y-gRadius-1.0f)));"
		"		if (aA>=gRadius+1.0f) return 0.0f;"
		"		else if (aA<gRadius) return 1.0f;"
		"		else return 1.0f-(aA-gRadius);"
		"	}"
		"}"
		""
		"vsmain()"
		"{"
		"	vertex.mXYZ=mul(input.mXYZ,gMatrix);"
		"	vertex.mColor=input.mColor;"
		"	vertex.mUV=input.mUV;"
		"}"
		"psmain()"
		"{"
		"	pixel=vertex.mColor;"
		"	pixel.a=(1.0f-GetA(vertex.mUV*gDimensions))*vertex.mColor.a;"
		"	if (pixel.a<=0.0f) discard;"
		//"	if (ShouldDiscard(vertex.mUV*gDimensions)) {discard;}"
		"}";

	Shader::Load(aSH);
	mDimensionRef=GetGlobalRef("gDimensions");
	mRadiusRef=GetGlobalRef("gRadius");
}
*/

void ShaderEllipse::Load()
{
	String aSH=
		"info {ShaderEllipse}"
		"global"
		"{"
		"	combomatrix			gMatrix;"
		"	float				gBorder;"
		"	float2				gDims;"
		"}"			
		"input"		
		"{"			
		"	xyz 			mXYZ;"
		"	rgba			mColor;"
		"	uv				mUV;"		
		"}"			
		"vertex"	
		"{"			
		"	xyz				mXYZ;"
		"	color			mColor;"
		"	float2			mUV;"
		"}"
		""
		"vsmain()"
		"{"
		"	vertex.mXYZ=mul(input.mXYZ,gMatrix);"
		"	vertex.mColor=input.mColor;"
		"	vertex.mUV=input.mUV;"
		"}"
		"psmain()"
		"{"
		"	pixel=vertex.mColor;"
		"	float x=vertex.mUV.x;"
		"	float y=vertex.mUV.y;"
		"	float x2 = x * x;"
		"	float y2 = y * y;"
		"	float w2 = gDims[0]*gDims[0];"
		"	float h2 = gDims[1]*gDims[1];"
		"	float f = sqrt(x2+y2);"
		"	float sd = (f - 1.0f) * f / (2.0 * sqrt(x2 / w2 + y2 / h2));"
		"	if (sd > 0.5) {discard;}"
		"	else if (sd > -0.5) {pixel.a=0.5f-sd;}"
		"	else if (sd > -gBorder + 0.5) {pixel.a=1.0f;}"
		"	else if (sd > -gBorder - 0.5) {pixel.a=sd+gBorder+0.5;}"
		"	else discard;"
		/*
		"	pixel=vertex.mColor;"
		"	float aDist=(vertex.mUV.x-.5f)*(vertex.mUV.x-.5f)+(vertex.mUV.y-.5f)*(vertex.mUV.y-.5f);"
		"	if (aDist>(.5f*.5f) || aDist<gBorder) {discard;}"
		*/
		"}";

	/*
	void main() {

		float f = sqrt(x * x + y * y);

		float sd = (f - 1.0) * f / (2.0 * sqrt(x2 / w2 + y2 / h2));

		float alpha;
		if (sd > 0.5) {
			discard;
		} else if (sd > -0.5) {
			alpha = 0.5 - sd;
		} else if (sd > -BORDER + 0.5) {
			alpha = 1.0;
		} else if (sd > -BORDER - 0.5) {
			alpha = sd + BORDER + 0.5;
		} else {
			discard;
		}
		gl_FragColor = vec4(1, .5, 0, alpha);
	}
	*/

	Shader::Load(aSH);
	mBorderRef=GetGlobalRef("gBorder");
	mDimsRef=GetGlobalRef("gDims");
}

void ShaderEllipse::Draw(Vertex* theV, float theBorder)
{
	mBorder=theBorder;
	gG.SetShader(this);
	this->DrawShape(theV,4,Index4Helper(),6);
}

void ShaderEllipse::Draw(Point thePos, Point theRadius, float theBorder)
{
	mBorder=theBorder;

	Vertex aV[4];
	aV[0]=Vertex(thePos-theRadius,gG.mColorInt,Point(-1,-1));
	aV[1]=Vertex(thePos+(theRadius*Point(1,-1)),gG.mColorInt,Point(1,-1));
	aV[2]=Vertex(thePos+(theRadius*Point(-1,1)),gG.mColorInt,Point(-1,1));
	aV[3]=Vertex(thePos+theRadius,gG.mColorInt,Point(1,1));
	Draw(aV,mBorder);
}

void ShaderEllipse::DrawShape(void* theVerts, int theVCount, vbindex* theI, int theICount)
{
	Vertex* aV=(Vertex*)theVerts;
	Vector aW=aV[1].mPos-aV[0].mPos;
	Vector aH=aV[2].mPos-aV[0].mPos;
	mDims[0]=aW.Length();
	mDims[1]=aH.Length();

	gG.SetShader(this);
	SetGlobal(mBorderRef,(float*)&mBorder,1);
	SetGlobal(mDimsRef,(float*)&mDims,2);
	Shader::DrawShape(theVerts,theVCount,theI,theICount);
}

void ShaderNEX::Load()
{
	mFogLimit=Point(gG.mZRange.mY-(gG.mZRange.mY/4),gG.mZRange.mY);
	String aSH=
		"info {ShaderNEX}"
		"global"
		"{"
		"	combomatrix		gComboMatrix;"
		"	worldmatrixn	gWorldMatrixN;"
		"	worldviewmatrix	gWorldViewMatrix;"
		"	sampler			gTexture1;"	// Multitex 0
		"	vector			gLightDir;"
		"	float2			gFogLimit;"
		"}"			
		"input"		
		"{"			
		"	xyz 			mXYZ;"			
		"	normal			mNormal;"
		"	uv				mUV;"	
		"	rgba			mFlag;"	// Ignored
		"}"			
		"vertex"	
		"{"			
		"	xyz				mXYZ;"
		"	uv				mUV;"
		"	float4			mNormal;"
		"	float			mFogPos;"
		"	float			mDotLight;"
		"}"			
		"vsmain()"
		"{"		
		"	vertex.mXYZ=mul(input.mXYZ,gComboMatrix);"
		"	vertex.mNormal=mul(input.mNormal,gWorldMatrixN);"
		"	vertex.mUV=input.mUV;"
		"	vertex.mFogPos=mul(input.mXYZ,gWorldViewMatrix).z;"
		"	vertex.mDotLight=dot(global.gLightDir,-vertex.mNormal.xyz);"
		"}"
		"psmain()"
		"{"
		"	pixel=tex2D(gTexture1,vertex.mUV);"
		//"	pixel=float4(1,0,0,1);"
		"	pixel.rgb*=clamp(vertex.mDotLight*2.0f,.25f,1.5f);"
		"	if (vertex.mFogPos>gFogLimit.x) pixel.a*=smoothstep(gFogLimit.y,gFogLimit.x,vertex.mFogPos);"
		//"	pixel.a*=.5f;"
		//"	pixel.rgb*=dot(global.gLightDir,-vertex.mNormal.xyz);"
		"}";

	Shader::Load(aSH);
	
	SetShaderHook(SHADERHOOK({
		if (mLightDirRef==-1) {mLightDirRef=GetGlobalRef("gLightDir");mFogLimitRef=GetGlobalRef("gFogLimit");}
		theShader->SetGlobal(mLightDirRef,(float*)&mLightDir,3);theShader->SetGlobal(mFogLimitRef,(float*)&mFogLimit,2);}));
}

void ShaderDecalNEX::Load()
{
	mFogLimit=Point(gG.mZRange.mY-(gG.mZRange.mY/4),gG.mZRange.mY);
	String aSH=
		"info {ShaderDecalNEX}"
		"global"
		"{"
		"	combomatrix		gComboMatrix;"
		"	worldmatrixn	gWorldMatrixN;"
		"	worldviewmatrix	gWorldViewMatrix;"
		"	sampler			gTexture1;"	// Multitex 0
		"	vector			gLightDir;"
		"	float2			gFogLimit;"
		"	float			gBias;"
		"}"			
		"input"		
		"{"			
		"	xyz 			mXYZ;"			
		"	normal			mNormal;"
		"	uv				mUV;"	
		"	rgba			mFlag;"	// Ignored
		"}"			
		"vertex"	
		"{"			
		"	xyz				mXYZ;"
		"	uv				mUV;"
		"	float4			mNormal;"
		"	float			mFogPos;"
		"	float			mDotLight;"
		"}"			
		"vsmain()"
		"{"
		"	vertex.mXYZ=mul(input.mXYZ,gComboMatrix);"
		//"	vertex.mXYZ.z-=.0001f;"
		"	vertex.mNormal=mul(input.mNormal,gWorldMatrixN);"
		//"	vertex.mXYZ+=vertex.mNormal*gBias;"
		"	vertex.mXYZ.w*=gBias;"
		"	vertex.mUV=input.mUV;"
		"	vertex.mDotLight=dot(global.gLightDir,-vertex.mNormal.xyz);"
		"	vertex.mFogPos=mul(input.mXYZ,gWorldViewMatrix).z;"
		"}"
		"psmain()"
		"{"
		"	pixel=tex2D(gTexture1,vertex.mUV);"
		//"	pixel=float4(1,0,0,1);"
		"	pixel.rgb*=clamp(vertex.mDotLight*2.0f,.25f,1.5f);"
		"	if (vertex.mFogPos>gFogLimit.x) pixel.a*=smoothstep(gFogLimit.y,gFogLimit.x,vertex.mFogPos);"
		//"	pixel.rgb*=dot(global.gLightDir,-vertex.mNormal.xyz);"
		"}";

	Shader::Load(aSH);
	mLightDirRef=GetGlobalRef("gLightDir");
	mFogLimitRef=GetGlobalRef("gFogLimit");
	mBiasRef=GetGlobalRef("gBias");
	SetShaderHook(SHADERHOOK({theShader->SetGlobal(mLightDirRef,(float*)&mLightDir,3);theShader->SetGlobal(mFogLimitRef,(float*)&mFogLimit,2);theShader->SetGlobal(mBiasRef,(float*)&mBias,1);}));
}

void ShaderDecalN::Load()
{
	mFogLimit=Point(gG.mZRange.mY-(gG.mZRange.mY/4),gG.mZRange.mY);
	String aSH=
		"info {ShaderDecalN}"
		"global"
		"{"
		"	combomatrix		gComboMatrix;"
		"	worldmatrixn	gWorldMatrixN;"
		"	worldviewmatrix	gWorldViewMatrix;"
		"	sampler			gTexture1;"	// Multitex 0
		"	vector			gLightDir;"
		"	float2			gFogLimit;"
		"	float			gBias;"
		"}"			
		"input"		
		"{"			
		"	xyz 			mXYZ;"			
		"	normal			mNormal;"
		"	uv				mUV;"	
		"}"			
		"vertex"	
		"{"			
		"	xyz				mXYZ;"
		"	uv				mUV;"
		"	float4			mNormal;"
		"	float			mFogPos;"
		"	float			mDotLight;"
		"}"			
		"vsmain()"
		"{"
		"	vertex.mXYZ=mul(input.mXYZ,gComboMatrix);"
		//"	vertex.mXYZ.z-=.0001f;"
		"	vertex.mNormal=mul(input.mNormal,gWorldMatrixN);"
		//"	vertex.mXYZ+=vertex.mNormal*gBias;"
		"	vertex.mXYZ.w*=gBias;"
		"	vertex.mUV=input.mUV;"
		"	vertex.mDotLight=dot(global.gLightDir,-vertex.mNormal.xyz);"
		"	vertex.mFogPos=mul(input.mXYZ,gWorldViewMatrix).z;"
		"}"
		"psmain()"
		"{"
		"	pixel=tex2D(gTexture1,vertex.mUV);"
		//"	pixel=float4(1,0,0,1);"
		"	pixel.rgb*=clamp(vertex.mDotLight*2.0f,.25f,1.5f);"
		"	if (vertex.mFogPos>gFogLimit.x) pixel.a*=smoothstep(gFogLimit.y,gFogLimit.x,vertex.mFogPos);"
		//"	pixel.rgb*=dot(global.gLightDir,-vertex.mNormal.xyz);"
		"}";

	Shader::Load(aSH);
	mLightDirRef=GetGlobalRef("gLightDir");
	mFogLimitRef=GetGlobalRef("gFogLimit");
	mBiasRef=GetGlobalRef("gBias");
	SetShaderHook(SHADERHOOK({theShader->SetGlobal(mLightDirRef,(float*)&mLightDir,3);theShader->SetGlobal(mFogLimitRef,(float*)&mFogLimit,2);theShader->SetGlobal(mBiasRef,(float*)&mBias,1);}));
}

/*
// 
// Keeping this around as a reference.  This doesn't work because my int values interpolate (and become "flat").
// So it fails with a shared vert.  BUT!  It would be okay to use flag for things like walls, etc, since they won't share
// verts with another texture.
// 
void ShaderNEXStart::Load()
{
	mFogLimit=Point(gG.mZRange.mY-(gG.mZRange.mY/4),gG.mZRange.mY);
	String aSH=
		"global"
		"{"
		"	combomatrix		gComboMatrix;"
		"	worldmatrixn	gWorldMatrixN;"
		"	worldviewmatrix	gWorldViewMatrix;"
		"	sampler			theTexture1;"	// Multitex 0
		"	sampler			theTexture2;"	// Multitex 1
		"	vector			theLightDir;"
		"	float2			theFogLimit;"
		"}"			
		"input"		
		"{"			
		"	xyz 			mXYZ;"			
		"	normal			mNormal;"
		"	uv				mUV;"	
		"	rgba			mFlag;"	// Ignored
		"}"			
		"vertex"	
		"{"			
		"	xyz				mXYZ;"
		"	uv				mUV;"
		"	float4			mNormal;"
		"	float			mFogPos;"
		"	int				mFlag;"
		"}"			
		"vsmain()"
		"{"		
		"	vertex.mXYZ=mul(input.mXYZ,gComboMatrix);"
		"	vertex.mNormal=mul(input.mNormal,gWorldMatrixN);"
		"	vertex.mUV=input.mUV;"
		"	vertex.mFogPos=mul(input.mXYZ,gWorldViewMatrix).z;"
		"	vertex.mFlag=0;"
		"	if (input.mFlag.a>.25f) vertex.mFlag=1;"
		"}"
		"psmain()"
		"{"
		//"	pixel=tex2D(theTexture1,vertex.mUV);"
		//"	pixel.rgb*=clamp(dot(global.gLightDir,-vertex.mNormal.xyz)*2.0f,.25f,1.5f);"
		//"	if (vertex.mFlag>600000000.0f) {"	// Special flag indicates special deal...
		//"		float4 aPixel2=float4(1,0,0,1);"
		//"		if (aPixel2.a>0.0f) {"
		//"			pixel.rgb=(pixel.rgb*(1.0f-aPixel2.a))+(aPixel2.rgb*aPixel2.a);"
		//"			pixel.a=max(pixel.a,aPixel2.a);"
		//"		}"
		//"	}"
		//"	if (vertex.mFogPos>theFogLimit.x) pixel.a*=smoothstep(theFogLimit.y,theFogLimit.x,vertex.mFogPos);"
		"	pixel=float4(1,0,0,1);"
		"	if (vertex.mFlag>.01f) pixel=float4(0,1,0,1);"
		//"	pixel.rgb*=dot(global.gLightDir,-vertex.mNormal.xyz);"
		"}";

	Shader::Load(aSH);
	mLightDirRef=GetGlobalRef("gLightDir");
	mFogLimitRef=GetGlobalRef("gFogLimit");
	SetShaderHook(SHADERHOOK({theShader->SetGlobal(mLightDirRef,(float*)&mLightDir,3);theShader->SetGlobal(mFogLimitRef,(float*)&mFogLimit,2);}));
}
*/

void ShaderRoundLine::Load()
{
	String aSH=
		"info {ShaderRoundLine}"
		"global"
		"{"
		"	combomatrix			gComboMatrix;"
		//"	projectionmatrix	gProj;"
		"   float4				gInfo;"
		"	gG.color			gColor;"
		"	gG.zbias			gZBias;"

		"}"
		"input"		
		"{"			
		"	xyz 			mXYZ;"			
		"	xyz 			mNormal;"			
		"	uv	 			mUV;"
		"	uv	 			mUV2;"
		"}"			
		"vertex"	
		"{"			
		"	xyz				mXYZ;"
		"	uv				mUV;"
		"}"			
		"vsmain()"
		"{"
		"	vertex.mXYZ=input.mXYZ*gComboMatrix;"
		"	vertex.mUV=input.mUV2;"
		//"	vertex.mUV=input.mUV2*vertex.mXYZ.w*gInfo.z;"
		//"	vertex.mXYZ.w*=gZBias;"
		"	vertex.mXYZ.xy+=(input.mUV*gInfo.zw)*vertex.mXYZ.w;"
		"}"

		"float RectSDF(vec2 p, float r)"
		"{"
		"	vec2 d = abs(p) - (gInfo.xy/2.) + vec2(r);"
		"	return min(max(d.x, d.y), 0.0) + length(max(d, 0.0)) - r;"
		"}"

		"psmain()"
		"{"
		"	float aDist = RectSDF((vertex.mUV-gInfo.xy / 2.0), gInfo.y/2.);"
		"	if (aDist>-1.) discard;"
		//"	pixel=float4(gColor.xyz,-aDist*gColor.w);"
		"	pixel=float4(gColor.xyz,gColor.w);"
		"	zdepth=pixel.zdepth/gZBias;"

		/*
		"	float aYoink=vertex.mUV.x/gInfo.x;"
		"	if (vertex.mUV.x<gInfo.y/2.) pixel=float4(1,0,0,1);"
		"	else if (vertex.mUV.x>gInfo.x-gInfo.y/2.) pixel=float4(1,0,0,1);"
		"	else pixel=float4(0,1,0,1);"
		"	pixel.a=-fDist;"
		/**/

		"}";

	mInfo[3]=gG.WidthF()/gG.HeightF();	// Aspect in 3....

	Shader::Load(aSH);
	mInfoRef=GetGlobalRef("gInfo");
}

void ShaderRoundLine::Draw(Vector theStart, Vector theEnd, float theThickness)
{
	DrawEX(theStart,theEnd,theThickness,0xFF);
}

void ShaderRoundLine::DrawEX(Vector theStart, Vector theEnd, float theThickness,unsigned char theEnds)
{
	Vector aDir=gG.ManualTransform(theEnd)-gG.ManualTransform(theStart);
	float aLen=aDir.Length();
	Point aPerp=aDir.GetNormal().ToPoint().Perp();
	aPerp.mY*=-1;

	float aCurve0=0.0f;
	float aCurve1=aLen;
	if (!(theEnds&0x01)) aCurve0=aLen/2;
	if (!(theEnds&0x02)) aCurve1=aLen/2;

	VertexNMT aV[4];
	aV[0].mPos=theStart;
	aV[0].mNormal=aDir;
	aV[0].mUV=aPerp*theThickness;
	aV[0].mUV2=Point(aCurve0,0.0f);

	aV[1].mPos=theEnd;
	aV[1].mNormal=aDir;
	aV[1].mUV=aPerp*theThickness;
	aV[1].mUV2=Point(aCurve1,0.0f);

	aV[2].mPos=theStart;
	aV[2].mNormal=aDir;
	aV[2].mUV=-aPerp*theThickness;
	aV[2].mUV2=Point(aCurve0,theThickness);

	aV[3].mPos=theEnd;
	aV[3].mNormal=aDir;
	aV[3].mUV=-aPerp*theThickness;
	aV[3].mUV2=Point(aCurve1,theThickness);

	mInfo[0]=aLen;
	mInfo[1]=theThickness;
	mInfo[2]=1.0f/gG.GetViewport().mWidth;
	mInfo[3]=1.0f/gG.GetViewport().mHeight;

	//
	// If len<thickness, just draw a circle at center...
	//

	gG.SetShader(this);
	SetGlobal(mInfoRef,(float*)&mInfo,4);
	Shader::DrawShape(aV,4,Index4Helper(),6);
	//Shader::DrawShape(aV,3);
}


void ShaderLine::Load()
{
	//
	// Possibly use geometry shader?
	// Here's a clue, link:
	// https://stackoverflow.com/questions/54686818/glsl-geometry-shader-to-replace-gllinewidth
	//
	String aSH=
		"info {ShaderLine}"
		"global"
		"{"
		"	combomatrix			gComboMatrix;"
		"   float4				gInfo;"
		"	gG.color			gColor;"
		"	gG.zbias			gZBias;"
		"}"
		"input"		
		"{"			
		"	xyz 			mXYZ;"			
		"	xyz 			mNormal;"			
		"	uv	 			mUV;"			
		"}"			
		"vertex"	
		"{"			
		"	xyz				mXYZ;"
		"}"			
		"vsmain()"
		"{"
		"	vertex.mXYZ=input.mXYZ*gComboMatrix;"
		"	vertex.mXYZ.xy+=(input.mNormal.xy*gInfo.xy)*vertex.mXYZ.w;"
		"}"
		"psmain()"
		"{"
		"	pixel=gColor;"
		"	zdepth=pixel.zdepth/gZBias;"
		"}";

	Shader::Load(aSH);
	mInfoRef=GetGlobalRef("gInfo");
}

void ShaderLine::Draw(Vector theStart, Vector theEnd, float theThickness,float theThickness2)
{
	if (theThickness2==-1) theThickness2=theThickness;

	Vector aDir=gG.ManualTransform(theEnd)-gG.ManualTransform(theStart);
	float aLen=aDir.Length();
	Point aPerp=aDir.GetNormal().ToPoint().Perp();
	aPerp.mY*=-1;

	VertexN aV[4];
	aV[0].mPos=theStart;
	aV[0].mNormal=aPerp*theThickness;
	aV[0].mUV=Point(0,0);

	aV[1].mPos=theEnd;
	aV[1].mNormal=aPerp*theThickness2;
	aV[1].mUV=Point(aLen,0.0f);

	aV[2].mPos=theStart;
	aV[2].mNormal=-aPerp*theThickness;
	aV[2].mUV=Point(0,1);

	aV[3].mPos=theEnd;
	aV[3].mNormal=-aPerp*theThickness2;
	aV[3].mUV=Point(aLen,1.0f);

	mInfo[0]=1.0f/gG.GetViewport().mWidth;
	mInfo[1]=1.0f/gG.GetViewport().mHeight;
	mInfo[2]=aLen; // This IS needed in other shaders, do not remove it.

	gG.SetShader(this);
	SetGlobal(mInfoRef,(float*)&mInfo,4);
	Shader::DrawShape(aV,4,Index4Helper(),6);
}

Smart(ShaderLine::Kit) ShaderLine::MakeKit(Array<Line3D>& theList, float theThickness, float theThickness2, bool isConnected, bool perpType, Smart(Kit) theKit)
{
	//
	// REMEMBER:
	// Your 3D camera has to be set up here (including world matrix) for it to work.
	//
	if (theKit.IsNull()) {theKit=new Kit;}
	if (theThickness2==-1) theThickness2=theThickness;

	struct LInfo
	{
		Point mDir;
		float mLen;
		Point mPerp;
	};

	Array<LInfo> aInfoList;
	aInfoList.GuaranteeSize(theList.Size());

	Matrix aR90;aR90.RotateZ(90);
	for (int aCount=0;aCount<theList.Size();aCount++)
	{
		aInfoList[aCount].mDir=(theList[aCount].mPos[1])-(theList[aCount].mPos[0]);
		aInfoList[aCount].mLen=aInfoList[aCount].mDir.Length();
		aInfoList[aCount].mPerp=(aInfoList[aCount].mDir/aInfoList[aCount].mLen);
		//aInfoList[aCount].mPerp=aR90.ProcessPoint(aInfoList[aCount].mPerp);
		aInfoList[aCount].mPerp=aInfoList[aCount].mPerp.Perp();
		aInfoList[aCount].mPerp.mY*=-1;
	}

	theKit->mV.GuaranteeSize(theList.Size()*4);
	theKit->mI.GuaranteeSize(theList.Size()*6);

	int aVPos=0;
	int aIPos=0;
	float aPreStep=0;
	for (int aCount=0;aCount<theList.Size();aCount++)
	{
		theKit->mV[aVPos].mPos=theList[aCount].mPos[0];
		theKit->mV[aVPos].mNormal=aInfoList[aCount].mPerp*theThickness;
		theKit->mV[aVPos].mUV=Point(aPreStep,0.0f);

		theKit->mV[aVPos+1].mPos=theList[aCount].mPos[1];
		theKit->mV[aVPos+1].mNormal=aInfoList[aCount].mPerp*theThickness2;
		theKit->mV[aVPos+1].mUV=Point(aPreStep+aInfoList[aCount].mLen,0.0f);

		theKit->mV[aVPos+2].mPos=theList[aCount].mPos[0];
		theKit->mV[aVPos+2].mNormal=-aInfoList[aCount].mPerp*theThickness;
		theKit->mV[aVPos+2].mUV=Point(aPreStep,1.0f);

		theKit->mV[aVPos+3].mPos=theList[aCount].mPos[1];
		theKit->mV[aVPos+3].mNormal=-aInfoList[aCount].mPerp*theThickness2;
		theKit->mV[aVPos+3].mUV=Point(aPreStep+aInfoList[aCount].mLen,1.0f);

		theKit->mI[aIPos++]=aVPos+0;
		theKit->mI[aIPos++]=aVPos+1;
		theKit->mI[aIPos++]=aVPos+2;
		theKit->mI[aIPos++]=aVPos+2;
		theKit->mI[aIPos++]=aVPos+1;
		theKit->mI[aIPos++]=aVPos+3;

		aVPos+=4;

		//
		// Between each segment, we need another, short line that just adjusts the normals (if the dot is big!)
		//
		if (isConnected && aCount>0)// && aCount<theList.Size()-1)
		{
			int aPrev=aCount-1;
			if (aPrev<0) aPrev+=theList.Size();

			if (gMath.Abs(aInfoList[aPrev].mPerp.Dot(aInfoList[aCount].mPerp))<1.0f-gMath.mMachineEpsilon)
			{
				theKit->mV[aVPos].mPos=theList[aCount].mPos[0];
				theKit->mV[aVPos].mNormal=aInfoList[aPrev].mPerp*theThickness;
				theKit->mV[aVPos].mUV=Point(aPreStep+aInfoList[aCount].mLen,0.0f);

				theKit->mV[aVPos+1].mPos=theList[aCount].mPos[0];
				theKit->mV[aVPos+1].mNormal=aInfoList[aCount].mPerp*theThickness;
				theKit->mV[aVPos+1].mUV=Point(aPreStep+aInfoList[aCount].mLen,0.0f);

				theKit->mV[aVPos+2].mPos=theList[aCount].mPos[0];
				theKit->mV[aVPos+2].mNormal=-aInfoList[aPrev].mPerp*theThickness;
				theKit->mV[aVPos+2].mUV=Point(aPreStep+aInfoList[aCount].mLen,1.0f);

				theKit->mV[aVPos+3].mPos=theList[aCount].mPos[0];
				theKit->mV[aVPos+3].mNormal=-aInfoList[aCount].mPerp*theThickness;
				theKit->mV[aVPos+3].mUV=Point(aPreStep+aInfoList[aCount].mLen,1.0f);

				//gGX.AddPoint(aV[aVPos].mPos);

				theKit->mI[aIPos++]=aVPos+0;
				theKit->mI[aIPos++]=aVPos+1;
				theKit->mI[aIPos++]=aVPos+2;
				theKit->mI[aIPos++]=aVPos+2;
				theKit->mI[aIPos++]=aVPos+1;
				theKit->mI[aIPos++]=aVPos+3;

				aVPos+=4;
			}
		}

		aPreStep+=aInfoList[aCount].mLen;

	}


	theKit->mVCount=aVPos;
	theKit->mICount=aIPos;
	theKit->mThickness=theThickness;

	/*
	gGX.Reset();
	for (int aCount=0;aCount<theKit->mICount;aCount+=3)
	{
		Vector aV1=theKit->mV[theKit->mI[aCount+0]].mPos+theKit->mV[theKit->mI[aCount+0]].mNormal;
		Vector aV2=theKit->mV[theKit->mI[aCount+1]].mPos+theKit->mV[theKit->mI[aCount+1]].mNormal;
		Vector aV3=theKit->mV[theKit->mI[aCount+2]].mPos+theKit->mV[theKit->mI[aCount+2]].mNormal;
		gGX.AddTri(aV1.ToPoint(),aV2.ToPoint(),aV3.ToPoint(),Color(0,0,1),2);
	}
	/**/


	return theKit;
}


Smart(ShaderLine::Kit) ShaderLine::MakeKit(Line3D& theLine, float theThickness, float theThickness2, bool isConnected, Smart(Kit) theKit)
{
	//
	// REMEMBER:
	// Your 3D camera has to be set up here (including world matrix) for it to work.
	//
	if (theKit.IsNull()) {theKit=new Kit;}
	if (theThickness2==-1) theThickness2=theThickness;

	struct LInfo
	{
		Vector mDir;
		float mLen;
		Point mPerp;
	};

	LInfo aInfoList;
	{
		aInfoList.mDir=gG.ManualTransform(theLine.mPos[1])-gG.ManualTransform(theLine.mPos[0]);
		aInfoList.mLen=aInfoList.mDir.Length();
		aInfoList.mPerp=(aInfoList.mDir.ToPoint()/aInfoList.mLen);
		aInfoList.mPerp=aInfoList.mPerp.Perp();
		aInfoList.mPerp.mY*=-1;
	}
	theKit->mV.GuaranteeSize(4);
	theKit->mI.GuaranteeSize(6);

	int aVPos=0;
	int aIPos=0;
	{
		theKit->mV[aVPos].mPos=theLine.mPos[0];
		theKit->mV[aVPos].mNormal=aInfoList.mPerp*theThickness;
		//aV[aVPos].mUV=Point(0,0);

		theKit->mV[aVPos+1].mPos=theLine.mPos[1];
		theKit->mV[aVPos+1].mNormal=aInfoList.mPerp*theThickness2;
		//aV[aVPos+1].mUV=Point(aInfoList[aCount].mLen,0.0f);

		theKit->mV[aVPos+2].mPos=theLine.mPos[0];
		theKit->mV[aVPos+2].mNormal=-aInfoList.mPerp*theThickness;
		//aV[aVPos+2].mUV=Point(0,1);

		theKit->mV[aVPos+3].mPos=theLine.mPos[1];
		theKit->mV[aVPos+3].mNormal=-aInfoList.mPerp*theThickness2;
		//aV[aVPos+3].mUV=Point(aInfoList[aCount].mLen,1.0f);

		theKit->mI[aIPos++]=aVPos+0;
		theKit->mI[aIPos++]=aVPos+1;
		theKit->mI[aIPos++]=aVPos+2;
		theKit->mI[aIPos++]=aVPos+2;
		theKit->mI[aIPos++]=aVPos+1;
		theKit->mI[aIPos++]=aVPos+3;

		aVPos+=4;
	}

	theKit->mVCount=aVPos;
	theKit->mICount=aIPos;
	theKit->mThickness=theThickness;
	return theKit;
}

void ShaderLine::Draw(Kit* theKit)
{
	mInfo[0]=1.0f/gG.GetViewport().mWidth;
	mInfo[1]=1.0f/gG.GetViewport().mHeight;

	gG.SetShader(this);
	SetGlobal(mInfoRef,(float*)&mInfo,4);
	ShaderLine::DrawShape(theKit->mV.mArray,theKit->mVCount,theKit->mI.mArray,theKit->mICount);
}

void ShaderLine::Draw(Array<Line3D>& theList, float theThickness)
{
	if (mBaseKit.IsNull()) mBaseKit=new Kit;
	MakeKit(theList,theThickness,mBaseKit);
	Draw(mBaseKit);
}

/****/
void ShaderLineGradient::Load()
{
	//
	// Possibly use geometry shader?
	// Here's a clue, link:
	// https://stackoverflow.com/questions/54686818/glsl-geometry-shader-to-replace-gllinewidth
	//
	String aSH=
		"info {ShaderLine}"
		"global"
		"{"
		"	combomatrix			gComboMatrix;"
		"   float2				gInfo;"
		"	gG.color			gColor;"
		"	gG.zbias			gZBias;"
		"}"
		"input"		
		"{"			
		"	xyz 			mXYZ;"			
		"	xyz 			mNormal;"			
		"	uv	 			mUV;"
		"	rgba			mColor;"
		"}"			
		"vertex"	
		"{"			
		"	xyz				mXYZ;"
		"	color			mColor;"
		"}"			
		"vsmain()"
		"{"
		"	vertex.mXYZ=input.mXYZ*gComboMatrix;"
		"	vertex.mXYZ.xy+=(input.mNormal.xy*gInfo.xy)*vertex.mXYZ.w;"
		"	vertex.mColor=input.mColor;"
		"}"
		"psmain()"
		"{"
		"	pixel=vertex.mColor;"
		"	zdepth=pixel.zdepth/gZBias;"
		"}";

	Shader::Load(aSH);
	mInfoRef=GetGlobalRef("gInfo");
}

void ShaderLineGradient::Draw(Vector theStart, Vector theEnd, float theThickness,float theThickness2, Color theC1, Color theC2)
{
	if (theThickness2==-1) theThickness2=theThickness;

	theC1*=gG.mColor;
	theC2*=gG.mColor;

	Vector aDir=gG.ManualTransform(theEnd)-gG.ManualTransform(theStart);
	float aLen=aDir.Length();
	Point aPerp=aDir.GetNormal().ToPoint().Perp();
	aPerp.mY*=-1;

	VertexNEX aV[4];
	aV[0].mPos=theStart;
	aV[0].mNormal=aPerp*theThickness;
	aV[0].mUV=Point(0,0);
	aV[0].mFlag=theC1.ToInt();


	aV[1].mPos=theEnd;
	aV[1].mNormal=aPerp*theThickness2;
	aV[1].mUV=Point(aLen,0.0f);
	aV[1].mFlag=theC2.ToInt();

	aV[2].mPos=theStart;
	aV[2].mNormal=-aPerp*theThickness;
	aV[2].mUV=Point(0,1);
	aV[2].mFlag=theC1.ToInt();

	aV[3].mPos=theEnd;
	aV[3].mNormal=-aPerp*theThickness2;
	aV[3].mUV=Point(aLen,1.0f);
	aV[3].mFlag=theC2.ToInt();

	mInfo[0]=1.0f/gG.GetViewport().mWidth;
	mInfo[1]=1.0f/gG.GetViewport().mHeight;

	gG.SetShader(this);
	SetGlobal(mInfoRef,(float*)&mInfo,2);
	Shader::DrawShape(aV,4,Index4Helper(),6);
}

/****/



void ShaderBillboard::Load()
{
	String aSH=
		"info {ShaderBillboard}"
		"global"
		"{"
		"	combomatrix			gComboMatrix;"
		"	sampler				gTexture;"
		"   float4				gInfo;"
		"	gG.zbias			gZBias;"
		"	gG.alphathreshold	gAlpha;"
		"}"			
		"input"		
		"{"			
		"	xyz 			mXYZ;"
		"	rgba			mColor;"
		"	uv				mUV;"	
		"	uv				mUV2;"
		"}"			
		"vertex"	
		"{"			
		"	xyz				mXYZ;"
		"	color			mColor;"
		"	uv				mUV;"
		"}"			
		"vsmain()"
		"{"
		"	vertex.mXYZ=mul(input.mXYZ,gComboMatrix);"
		"	vertex.mColor=input.mColor;"
		"	vertex.mUV=input.mUV;"
		"	if (gInfo[2]!=0.) vertex.mXYZ.xy+=(input.mUV2*gInfo.xy)*vertex.mXYZ.w;"
		"	else vertex.mXYZ.xy+=(input.mUV2*gInfo.xy);"
		"}"
		"psmain()"
		"{"
		"	pixel=tex2D(gTexture,vertex.mUV);"
		"	pixel*=vertex.mColor;"
		"	if (pixel.a<gAlpha) discard;"
		"	zdepth=pixel.zdepth/gZBias;"
		"}";

	Shader::Load(aSH);
	mInfoRef=GetGlobalRef("gInfo");
}

void ShaderBillboard::Draw(Sprite& theSprite, Vector thePos, float theScale, bool oneSize)
{
	static VertexMT aV[4];
	int aColor=gG.mColorInt;
	aV[0].mPos=thePos;//theSprite.mDrawQuad.mCorner[0];
	aV[1].mPos=thePos;//theSprite.mDrawQuad.mCorner[1];
	aV[2].mPos=thePos;//theSprite.mDrawQuad.mCorner[2];
	aV[3].mPos=thePos;//theSprite.mDrawQuad.mCorner[3];
	aV[0].mUV=theSprite.mTextureQuad.mCorner[0];
	aV[1].mUV=theSprite.mTextureQuad.mCorner[1];
	aV[2].mUV=theSprite.mTextureQuad.mCorner[2];
	aV[3].mUV=theSprite.mTextureQuad.mCorner[3];
	aV[0].mColor=aColor;
	aV[1].mColor=aColor;
	aV[2].mColor=aColor;
	aV[3].mColor=aColor;
	aV[0].mUV2=theSprite.mDrawQuad.mCorner[0]*mGlobalScalar*theScale;
	aV[1].mUV2=theSprite.mDrawQuad.mCorner[1]*mGlobalScalar*theScale;
	aV[2].mUV2=theSprite.mDrawQuad.mCorner[2]*mGlobalScalar*theScale;
	aV[3].mUV2=theSprite.mDrawQuad.mCorner[3]*mGlobalScalar*theScale;
	gG.SetTexture(theSprite.mTexture);

	mInfo[0]=1.0f/gG.GetViewport().mWidth;
	mInfo[1]=1.0f/gG.GetViewport().mHeight;
	mInfo[2]=oneSize;

	/*
	Matrix aMat;
	Vector theScale=1.0f;
	aMat.Scale(theScale.mX*gG.mBillboardScale,theScale.mY*gG.mBillboardScale,1);
	aMat*=gG.GetBillboardMatrix();
	aMat.Translate(thePos);
	aMat*=gG.GetWorldMatrix();

	gG.PushWorldMatrix();
	gG.SetWorldMatrix(aMat);
	/**/
	gG.SetShader(this);
	SetGlobal(mInfoRef,(float*)&mInfo,4);
	DrawShape((void*)&aV,4,Index4Helper(),6);

	//gG.PopWorldMatrix();
}


void ShaderCircle::Load()
{
	String aSH=
		"info {ShaderCircle}"
		"global"
		"{"
		"	combomatrix			gComboMatrix;"
		"   float4				gInfo;"
		"	gG.zbias			gZBias;"
		"}"			
		"input"		
		"{"			
		"	xyz 			mXYZ;"
		"	rgba			mColor;"
		"	uv				mUV;"	
		"}"			
		"vertex"	
		"{"			
		"	xyz				mXYZ;"
		"	color			mColor;"
		"	uv				mUV;"
		"}"			
		"vsmain()"
		"{"
		"	vertex.mXYZ=mul(input.mXYZ,gComboMatrix);"
		"	vertex.mColor=input.mColor;"
		"	vertex.mUV=input.mUV;"
		"	vertex.mXYZ.xy+=gInfo[3]*(input.mUV*gInfo.xy)*vertex.mXYZ.w;" // Always using "onesize" for this case...
		"}"
		"psmain()"
		"{"
		"	pixel=vertex.mColor;"
		"	float aDot=dot(vertex.mUV,vertex.mUV);"
		"	if (aDot>.5f*.5f) discard;"
		// Antialias?
		"	zdepth=pixel.zdepth/gZBias;"
		"}";

	Shader::Load(aSH);
	mInfoRef=GetGlobalRef("gInfo");
}

void ShaderCircle::Draw(Vector thePos, float theSize, bool oneSize)
{
	static Vertex aV[4];
	int aColor=gG.mColorInt;
	aV[0].mPos=thePos;
	aV[1].mPos=thePos;
	aV[2].mPos=thePos;
	aV[3].mPos=thePos;
	aV[0].mUV=Point(-.5f,-.5f);
	aV[1].mUV=Point(.5f,-.5f);
	aV[2].mUV=Point(-.5f,.5f);
	aV[3].mUV=Point(.5f,.5f);
	aV[0].mColor=aColor;
	aV[1].mColor=aColor;
	aV[2].mColor=aColor;
	aV[3].mColor=aColor;

	float aBorder=1;
	float aBorderUV=aBorder/theSize;

	mInfo[0]=1.0f/gG.GetViewport().mWidth;
	mInfo[1]=1.0f/gG.GetViewport().mHeight;
	//mInfo[2]=aBorderUV*aBorderUV;
	mInfo[3]=theSize*mGlobalScalar*2;

	gG.SetShader(this);
	SetGlobal(mInfoRef,(float*)&mInfo,4);
	DrawShape((void*)&aV,4,Index4Helper(),6);
}

void ShaderCircle3D::Load()
{
	String aSH=
		"info {ShaderCircle}"
		"global"
		"{"
		"	combomatrix			gComboMatrix;"
		"	gG.zbias			gZBias;"
		"}"			
		"input"		
		"{"			
		"	xyz 			mXYZ;"
		"	rgba			mColor;"
		"	uv				mUV;"	
		"}"			
		"vertex"	
		"{"			
		"	xyz				mXYZ;"
		"	color			mColor;"
		"	uv				mUV;"
		"}"			
		"vsmain()"
		"{"
		"	vertex.mXYZ=mul(input.mXYZ,gComboMatrix);"
		"	vertex.mColor=input.mColor;"
		"	vertex.mUV=input.mUV;"
		"}"
		"psmain()"
		"{"
		"	pixel=vertex.mColor;"
		"	float aDot=dot(vertex.mUV,vertex.mUV);"
		"	if (aDot>.5f*.5f) discard;"
		// Antialias?
		"	zdepth=pixel.zdepth/gZBias;"
		"}";

	Shader::Load(aSH);
}

void ShaderCircle3D::Draw(Quad3D theQuad)
{
	static Vertex aV[4];
	int aColor=gG.mColorInt;
	aV[0].mPos=theQuad.mCorner[0];
	aV[1].mPos=theQuad.mCorner[1];
	aV[2].mPos=theQuad.mCorner[2];
	aV[3].mPos=theQuad.mCorner[3];
	aV[0].mUV=Point(-.5f,-.5f);
	aV[1].mUV=Point(.5f,-.5f);
	aV[2].mUV=Point(-.5f,.5f);
	aV[3].mUV=Point(.5f,.5f);
	aV[0].mColor=aColor;
	aV[1].mColor=aColor;
	aV[2].mColor=aColor;
	aV[3].mColor=aColor;

	gG.SetShader(this);
	DrawShape((void*)&aV,4,Index4Helper(),6);
}


void ShaderGlowCircle::Load()
{
	String aSH=
		"info {ShaderGlowCircle}"
		"global"
		"{"
		"	combomatrix			gComboMatrix;"
		"   float4				gInfo;"
		"	gG.zbias			gZBias;"
		"}"			
		"input"		
		"{"			
		"	xyz 			mXYZ;"
		"	rgba			mColor;"
		"	uv				mUV;"	
		"}"			
		"vertex"	
		"{"			
		"	xyz				mXYZ;"
		"	color			mColor;"
		"	uv				mUV;"
		"}"			
		"vsmain()"
		"{"
		"	vertex.mXYZ=mul(input.mXYZ,gComboMatrix);"
		"	vertex.mColor=input.mColor;"
		"	vertex.mUV=input.mUV;"
		"	vertex.mXYZ.xy+=gInfo[3]*(input.mUV*gInfo.xy)*vertex.mXYZ.w;" // Always using "onesize" for this case...
		"}"
		"psmain()"
		"{"
		"	pixel=vertex.mColor;"
		"	float aDot=length(vertex.mUV);"
		"	if (aDot>.5f || aDot<gInfo[2]) discard;"
		"	float aMod=(aDot-gInfo[2])/gInfo[2];"
		"	pixel.a*=(aMod*aMod);"
		// Antialias?
		"	zdepth=pixel.zdepth/gZBias;"
		"}";

	Shader::Load(aSH);
	mInfoRef=GetGlobalRef("gInfo");
	mInfo[2]=.25f;
}


void ShaderDottedLine::Load()
{
	String aSH=
		"info {ShaderDottedLine}"
		"global"
		"{"
		"	combomatrix			gComboMatrix;"
		"   float4				gInfo;"
		"	gG.color			gColor;"
		"	gG.zbias			gZBias;"
		"	gG.shaderdata		gData;"
		"}"
		"input"		
		"{"			
		"	xyz 			mXYZ;"
		"	xyz 			mNormal;"
		"	uv	 			mUV;"	
		"}"			
		"vertex"	
		"{"			
		"	xyz				mXYZ;"
		"	uv				mUV;"
		"}"			
		"vsmain()"
		"{"
		"	vertex.mXYZ=input.mXYZ*gComboMatrix;"
		"	vertex.mXYZ.xy+=(input.mNormal.xy*gInfo.xy)*vertex.mXYZ.w;"
		"	vertex.mUV=input.mUV;"
		"}"
		"psmain()"
		"{"
		"	if (mod(vertex.mUV.x+gInfo[3],gData.x*2.)>gData.x) discard;" // gData is the width of the bars...
		"	pixel=gColor;"
		"	zdepth=pixel.zdepth/gZBias;"
		"}";

	Shader::Load(aSH);
	mInfoRef=GetGlobalRef("gInfo");
}

void ShaderDottedLine::Draw(Vector theStart, Vector theEnd, float theThickness, float theDotSize) {if (theDotSize<=0) theDotSize=theThickness;gG.ShaderData()[0]=theDotSize;ShaderLine::Draw(theStart,theEnd,theThickness,theThickness);}
void ShaderDottedLine::Draw(Array<Line3D>& theList, float theThickness, float theDotSize) {if (theDotSize<=0) theDotSize=theThickness;gG.ShaderData()[0]=theDotSize;ShaderLine::Draw(theList,theThickness);}
void ShaderDottedLine::Draw(Kit* theKit, float theDotSize) {if (theDotSize<=0) theDotSize=1.0f;gG.ShaderData()[0]=theDotSize;ShaderLine::Draw(theKit);}

void ShaderShowZ::Load()
{
	String aSH=
		"info {ShaderShowZ}"
		"global"
		"{"
		"	combomatrix			gComboMatrix;"
		"	sampler				gZBuffer;"
		"}"
		"input"
		"{"
		"	xyz 			mXYZ;"
		"	rgba			mColor;"
		"	uv				mUV;"
		"}"
		"vertex"
		"{"
		"	xyz				mXYZ;"
		"	uv				mUV;"
		"}"
		"vsmain()"
		"{"
		"	vertex.mXYZ=input.mXYZ*gComboMatrix;"
		"	vertex.mUV=input.mUV;"
		"}"
		"psmain()"
		"{"
		"	float aValue=texture(gZBuffer,vertex.mUV).r;"
		"	pixel=float4(aValue,aValue,aValue,1.0f);"
		"}";

	Shader::Load(aSH);
}

void ShaderShowZ::Draw()
{
	gG.SetShader(this);

	Vertex2D aV[4];
	aV[0]=Vertex2D(0,0,0,0xFFFFFFFF,0,0);
	aV[1]=Vertex2D(gG.WidthF(),0.0f,0.0f,0xFFFFFFFF,1,0);
	aV[2]=Vertex2D(0.0f,gG.HeightF(),0.0f,0xFFFFFFFF,0,1);
	aV[3]=Vertex2D(gG.WidthF(),gG.HeightF(),0.0f,0xFFFFFFFF,1,1);

	Shader::DrawShape(aV,4,Index4Array());
	gG.SetTexture();
}


void ShaderShowZ::Draw(int theZBuffer)
{
	gG.SetShader(this);
	gG.SetTexture(theZBuffer);

	Vertex2D aV[4];
	aV[0]=Vertex2D(0,0,0,0xFFFFFFFF,0,0);
	aV[1]=Vertex2D(gG.WidthF(),0.0f,0.0f,0xFFFFFFFF,1,0);
	aV[2]=Vertex2D(0.0f,gG.HeightF(),0.0f,0xFFFFFFFF,0,1);
	aV[3]=Vertex2D(gG.WidthF(),gG.HeightF(),0.0f,0xFFFFFFFF,1,1);

	/*
	aV[0].mX+=10;aV[0].mY+=10;
	aV[1].mX-=10;aV[1].mY+=10;
	aV[2].mX+=10;aV[2].mY-=10;
	aV[3].mX-=10;aV[3].mY-=10;
	*/

	Shader::DrawShape(aV,4,Index4Array());
	gG.SetTexture();
}

void Shader2D::Load()
{
	String aSH=
		"info {Shader2D}"
		"global"
		"{"
		"	sampler			gTexture;"
		"	gG.resolution	gRez;"
		"	gG.flags		gFlags;"
		"}"			
		"input"		
		"{"			
		"	xy				mXY;"
		"	rgba			mColor;"
		"	uv				mUV;"
		"}"			
		"vertex"	
		"{"			
		"	xyz				mXYZ;"
		"	color			mColor;"
		"	uv				mUV;"
		"}"			
		"vsmain()"
		"{"
		"	if ((gFlags&FLAG_RENDERTO)!=0) vertex.mXYZ=float4(((input.mXY.x/gRez.z)-1.0f),(((input.mXY.y)/gRez.w))-1.0f,0.,1.);" //RenderTo (no flip GL)
		"	else vertex.mXYZ=float4(((input.mXY.x/gRez.z)-1.0f),(1.0f-((input.mXY.y)/gRez.w)),0.,1.);" //Normal (flipGL)
		"	vertex.mColor=input.mColor;"
		"	vertex.mUV=input.mUV;"
		"}"
		"psmain()"
		"{"
		"	pixel=tex2D(gTexture,vertex.mUV)*vertex.mColor;"
		"}";

	Shader::Load(aSH);
}

void Shader2DWhite::Load()
{
	String aSH=
		"info {Shader2DWhite}"
		"global"
		"{"
		"	sampler			gTexture;"
		"	gG.resolution	gRez;"
		"	gG.flags		gFlags;"
		"}"			
		"input"		
		"{"			
		"	xy				mXY;"
		"	rgba			mColor;"
		"	uv				mUV;"
		"}"			
		"vertex"	
		"{"			
		"	xyz				mXYZ;"
		"	color			mColor;"
		"	uv				mUV;"
		"}"			
		"vsmain()"
		"{"
		"	if ((gFlags&FLAG_RENDERTO)!=0) vertex.mXYZ=float4(((input.mXY.x/gRez.z)-1.0f),(((input.mXY.y)/gRez.w))-1.0f,0.,1.);" //RenderTo (no flip GL)
		"	else vertex.mXYZ=float4(((input.mXY.x/gRez.z)-1.0f),(1.0f-((input.mXY.y)/gRez.w)),0.,1.);" //Normal (flipGL)
		"	vertex.mColor=input.mColor;"
		"	vertex.mUV=input.mUV;"
		"}"
		"psmain()"
		"{"
		"	pixel=vertex.mColor;"
		"	pixel.a*=tex2D(gTexture,vertex.mUV).a;"
		"}";

	Shader::Load(aSH);
}

void Shader2DGrey::Load()
{
	String aSH=
		"info {Shader2DGrey}"
		"global"
		"{"
		"	sampler			gTexture;"
		"	gG.resolution	gRez;"
		"	gG.flags		gFlags;"
		"	gG.saturation	gSaturation;"
		"}"			
		"input"		
		"{"			
		"	xy				mXY;"
		"	rgba			mColor;"
		"	uv				mUV;"
		"}"			
		"vertex"	
		"{"			
		"	xyz				mXYZ;"
		"	color			mColor;"
		"	uv				mUV;"
		"}"			
		"vsmain()"
		"{"
		"	if ((gFlags&FLAG_RENDERTO)!=0) vertex.mXYZ=float4(((input.mXY.x/gRez.z)-1.0f),(((input.mXY.y)/gRez.w))-1.0f,0.,1.);" //RenderTo (no flip GL)
		"	else vertex.mXYZ=float4(((input.mXY.x/gRez.z)-1.0f),(1.0f-((input.mXY.y)/gRez.w)),0.,1.);" //Normal (flipGL)
		"	vertex.mColor=input.mColor;"
		"	vertex.mUV=input.mUV;"
		"}"
		"psmain()"
		"{"
		"	pixel=tex2D(gTexture,vertex.mUV);"
		"	float aGrey=(pixel.r+pixel.g+pixel.b)/3.;"
		"	pixel=mix(float4(aGrey,aGrey,aGrey,pixel.a),pixel,gSaturation);"
		"}";

	Shader::Load(aSH);
}

void Shader2DColorMix::Load()
{
	String aSH=
		"info {Shader2DColorMix}"
		"global"
		"{"
		"	sampler				gTexture;"
		"	gG.resolution		gRez;"
		"	gG.flags			gFlags;"
		"	gG.shaderdata0		gColor1;"
		"	gG.shaderdata1		gColor2;"
		"	gG.shaderdata2		gColor3;"
		"}"			
		"input"		
		"{"			
		"	xy				mXY;"
		"	rgba			mColor;"
		"	uv				mUV;"
		"}"			
		"vertex"	
		"{"			
		"	xyz				mXYZ;"
		"	color			mColor;"
		"	uv				mUV;"
		"}"			
		"vsmain()"
		"{"		
		"	if ((gFlags&FLAG_RENDERTO)!=0) vertex.mXYZ=float4(((input.mXY.x/gRez.z)-1.0f),(((input.mXY.y)/gRez.w))-1.0f,0.,1.);" //RenderTo (no flip GL)
		"	else vertex.mXYZ=float4(((input.mXY.x/gRez.z)-1.0f),(1.0f-((input.mXY.y)/gRez.w)),0.,1.);" //Normal (flipGL)
		"	vertex.mColor=input.mColor;"
		"	vertex.mUV=input.mUV;"
		"}"
		"psmain()"
		"{"
		"	float4 aSource=tex2D(gTexture,vertex.mUV);"
		"	pixel=float4(0,0,0,0);"
		//*
		"	if (aSource.r>0.) pixel.rgba=float4(gColor1.xyz,aSource.r);"
		"	if (aSource.g>0.) pixel=mix(pixel,float4(gColor2.xyz,aSource.g),aSource.g);"
		"	if (aSource.b>0.) pixel=mix(pixel,float4(gColor3.xyz,aSource.b),aSource.b);"
		/**/

		/*
		"	if (aSource.r>0.) pixel.rgba=float4(gColor1.xyz*aSource.r,aSource.r);"
		"	if (aSource.g>0.) pixel=mix(pixel,float4(gColor2.xyz*aSource.g,aSource.g),aSource.g);"
		"	if (aSource.b>0.) pixel=mix(pixel,float4(gColor3.xyz*aSource.b,aSource.b),aSource.b);"
		/**/
		"	pixel.a=aSource.a;"

		//"	pixel*=vertex.mColor;"
		"}";

	Shader::Load(aSH);
}


#define RRCORNER \
	"float CircleDist(float2 coord, float2 corner) {return length(coord-corner);}" \
	"float GetA(float2 coords)" \
	"{" \
	"	if ((coords.x>vertex.mExtra.z && coords.x<vertex.mExtra.x-vertex.mExtra.z) || (coords.y>vertex.mExtra.z && coords.y<vertex.mExtra.y-vertex.mExtra.z)) return 1.0f;" \
	"	else" \
	"	{" \
	"		float aA=CircleDist(coords,float2(vertex.mExtra.z+1.,vertex.mExtra.z+1.0f));" \
	"		aA=min(aA,CircleDist(coords,float2(vertex.mExtra.x-vertex.mExtra.z-1.0f, vertex.mExtra.z+1.0f)));" \
	"		aA=min(aA,CircleDist(coords,float2(vertex.mExtra.z+1., vertex.mExtra.y-vertex.mExtra.z-1.0f)));" \
	"		aA=min(aA,CircleDist(coords,float2(vertex.mExtra.x-vertex.mExtra.z-1.0f, vertex.mExtra.y-vertex.mExtra.z-1.0f)));" \
	"		if (aA>=vertex.mExtra.z+1.0f) return 0.0f;" \
	"		else if (aA<vertex.mExtra.z) return 1.0f;" \
	"		else return 1.0f-(aA-vertex.mExtra.z);" \
	"	}" \
	"}"

void ShaderRRPipeline::Load()
{
	String aSH=
		"info {ShaderRRPipeline}"
		"global"
		"{"
		"	gG.resolution		gRez;"
		"	gG.flags			gFlags;"
		"}"			
		"input"		
		"{"			
		"	xy				mXY;"
		"	rgba			mColor;"
		"	uv				mUV;"
		"	vector			mExtra;"
		"}"			
		"vertex"	
		"{"			
		"	xyz				mXYZ;"
		"	color			mColor;"
		"	uv				mUV;"
		"	vector			mExtra;"
		
		"}"
		RRCORNER
		""
		"vsmain()"
		"{"
		"	if ((gFlags&FLAG_RENDERTO)!=0) vertex.mXYZ=float4(((input.mXY.x/gRez.z)-1.0f),(((input.mXY.y)/gRez.w))-1.0f,0.,1.);" //RenderTo (no flip GL)
		"	else vertex.mXYZ=float4(((input.mXY.x/gRez.z)-1.0f),(1.0f-((input.mXY.y)/gRez.w)),0.,1.);" //Normal (flipGL)
		"	vertex.mColor=input.mColor;"
		"	vertex.mUV=input.mUV;"
		"	vertex.mExtra=input.mExtra;"
		"}"
		"psmain()"
		"{"
		"	pixel=vertex.mColor;"
		"	pixel.a=GetA(vertex.mUV)*vertex.mColor.a;"
		"	if (pixel.a<=0.0f) discard;"
		"}";

	Shader::Load(aSH);
}

void ShaderRRPipelineI::Load()
{
	String aSH=
		"info {ShaderRRPipelineI}"
		"global"
		"{"
		"	gG.resolution		gRez;"
		"	gG.flags			gFlags;"
		"}"			
		"input"		
		"{"			
		"	xy				mXY;"
		"	rgba			mColor;"
		"	uv				mUV;"
		"	vector			mExtra;"
		"}"			
		"vertex"	
		"{"			
		"	xyz				mXYZ;"
		"	color			mColor;"
		"	uv				mUV;"
		"	vector			mExtra;"

		"}"
		RRCORNER
		""
		"vsmain()"
		"{"
		"	if ((gFlags&FLAG_RENDERTO)!=0) vertex.mXYZ=float4(((input.mXY.x/gRez.z)-1.0f),(((input.mXY.y)/gRez.w))-1.0f,0.,1.);" //RenderTo (no flip GL)
		"	else vertex.mXYZ=float4(((input.mXY.x/gRez.z)-1.0f),(1.0f-((input.mXY.y)/gRez.w)),0.,1.);" //Normal (flipGL)
		"	vertex.mColor=input.mColor;"
		"	vertex.mUV=input.mUV;"
		"	vertex.mExtra=input.mExtra;"
		"}"
		"psmain()"
		"{"
		"	pixel=vertex.mColor;"
		"	pixel.a=1.0f-GetA(vertex.mUV)*vertex.mColor.a;"
		"	if (pixel.a<=0.0f) discard;"
		"}";

	Shader::Load(aSH);
}

void ShaderRRPipelineT::Load()
{
	String aSH=
		"info {ShaderRRPipelineT}"
		"global"
		"{"
		"	gG.resolution		gRez;"
		"	gG.flags			gFlags;"
		"	sampler				gTexture;"
		"}"			
		"input"		
		"{"			
		"	xy				mXY;"
		"	rgba			mColor;"
		"	uv				mUV;"
		"	vector			mExtra;"
		"}"			
		"vertex"	
		"{"			
		"	xyz				mXYZ;"
		"	color			mColor;"
		"	uv				mUV;"
		"	uv				mCorner;"
		"	vector			mExtra;"

		"}"
		RRCORNER
		""
		"vsmain()"
		"{"
		"	if ((gFlags&FLAG_RENDERTO)!=0) vertex.mXYZ=float4(((input.mXY.x/gRez.z)-1.0f),(((input.mXY.y)/gRez.w))-1.0f,0.,1.);" //RenderTo (no flip GL)
		"	else vertex.mXYZ=float4(((input.mXY.x/gRez.z)-1.0f),(1.0f-((input.mXY.y)/gRez.w)),0.,1.);" //Normal (flipGL)
		"	vertex.mColor=input.mColor;"
		"	vertex.mUV=input.mUV*input.mExtra.xy/float2(textureSize(gTexture,0));"
		"	vertex.mExtra=input.mExtra;"
		"	vertex.mCorner=input.mUV*vertex.mExtra.xy;"
		"}"
		"psmain()"
		"{"
		"	pixel=tex2D(gTexture,vertex.mUV)*vertex.mColor;"
		"	pixel.a=(GetA(vertex.mCorner))*vertex.mColor.a;"
		"	if (pixel.a<=0.0f) discard;"
		"}";

	Shader::Load(aSH);
}

void ShaderRRPipelineTI::Load()
{
	String aSH=
		"info {ShaderRRPipelineTI}"
		"global"
		"{"
		"	gG.resolution		gRez;"
		"	gG.flags			gFlags;"
		"	sampler				gTexture;"
		"}"			
		"input"		
		"{"			
		"	xy				mXY;"
		"	rgba			mColor;"
		"	uv				mUV;"
		"	vector			mExtra;"
		"}"			
		"vertex"	
		"{"			
		"	xyz				mXYZ;"
		"	color			mColor;"
		"	uv				mUV;"
		"	uv				mCorner;"
		"	vector			mExtra;"

		"}"
		RRCORNER
		""
		"vsmain()"
		"{"
		"	if ((gFlags&FLAG_RENDERTO)!=0) vertex.mXYZ=float4(((input.mXY.x/gRez.z)-1.0f),(((input.mXY.y)/gRez.w))-1.0f,0.,1.);" //RenderTo (no flip GL)
		"	else vertex.mXYZ=float4(((input.mXY.x/gRez.z)-1.0f),(1.0f-((input.mXY.y)/gRez.w)),0.,1.);" //Normal (flipGL)
		"	vertex.mColor=input.mColor;"
		"	vertex.mUV=input.mUV*input.mExtra.xy/float2(textureSize(gTexture,0));"
		"	vertex.mExtra=input.mExtra;"
		"	vertex.mCorner=input.mUV*vertex.mExtra.xy;"
		"}"
		"psmain()"
		"{"
		"	pixel=tex2D(gTexture,vertex.mUV)*vertex.mColor;"
		"	pixel.a=(1.0f-GetA(vertex.mCorner))*vertex.mColor.a;"
		"	if (pixel.a<=0.0f) discard;"
		"}";

	Shader::Load(aSH);
}

void ShaderCirclePipeline::Load()
{
	String aSH=
		"info {ShaderCirclePipeline}"
		"global"
		"{"
		"	gG.resolution		gRez;"
		"	gG.flags			gFlags;"
		"}"			
		"input"		
		"{"			
		"	xy				mXY;"
		"	rgba			mColor;"
		"	uv				mUV;"
		"	vector			mExtra;"
		"}"			
		"vertex"	
		"{"			
		"	xyz				mXYZ;"
		"	color			mColor;"
		"	uv				mUV;"
		"	float4			mExtra;"

		"}"
		"vsmain()"
		"{"
		//"	vertex.mOriginal=input.mXY;"
		"	if ((gFlags&FLAG_RENDERTO)!=0) vertex.mXYZ=float4(((input.mXY.x/gRez.z)-1.0f),(((input.mXY.y)/gRez.w))-1.0f,0.,1.);" //RenderTo (no flip GL)
		"	else vertex.mXYZ=float4(((input.mXY.x/gRez.z)-1.0f),(1.0f-((input.mXY.y)/gRez.w)),0.,1.);" //Normal (flipGL)
		"	vertex.mColor=input.mColor;"
		"	vertex.mUV=input.mUV-float2(.5f,.5f);"
		"	vertex.mExtra.xyz=input.mExtra;"
		"	vertex.mExtra.w=.5f-(.5f/vertex.mExtra.z);"
		"	vertex.mExtra.w*=vertex.mExtra.w;"
		"}"
		"psmain()"
		"{"
		"	pixel=vertex.mColor;"
		"	float aDot=dot(vertex.mUV,vertex.mUV);"
		"	if (aDot>.5f*.5f) discard;"
		"	else if (aDot>=vertex.mExtra.w) {pixel.a=1.-smoothstep(vertex.mExtra.z-1.,vertex.mExtra.z,sqrt(aDot)*vertex.mExtra.z*2.);}"//pixel.rgb=float3(1.,0.,0.);}"
		"}";


	Shader::Load(aSH);
}

void ShaderSpline3D::Load()
{
	String aSH=
		"info {ShaderLine}"
		"global"
		"{"
		"	combomatrix			gComboMatrix;"
		"	gG.shaderdata		gData;"
		"	gG.color			gColor;"
		"	gG.zbias			gZBias;"
		"}"
		"input"		
		"{"			
		"	xyz				mXYZ;"
		"	rgba			mColor;"
		"	uv				mUV;"
		"}"
		"vertex"	
		"{"			
		"	xyz				mXYZ;"
		"	uv				mUV;"
		"}"			
		"vsmain()"
		"{"
		"	vertex.mXYZ=input.mXYZ*gComboMatrix;"
		"}"
		"psmain()"
		"{"
		"	pixel=gColor;"
		"	zdepth=pixel.zdepth/gZBias;"
		"}";

	Shader::Load(aSH);
}

void ShaderSpline3D::Draw(Kit* theKit, float rounded)
{
	gG.SetShader(this);
	gG.SetShaderData(rounded);
	Shader::DrawShape(theKit->mV.mArray,theKit->mV.Size(),theKit->mI.mArray,theKit->mI.Size());
}

Smart(ShaderSpline3D::Kit) ShaderSpline3D::MakeKit(Array<Line3D>& theList, Array<Vector>& theNormals, float theThickness)
{
	Smart(Kit) aKit=new Kit;
	aKit->mV.GuaranteeSize((theList.Size()+1)*2);
	aKit->mI.GuaranteeSize((theList.Size())*6);
	int aPutV=0;

	Vector aPos;
	for (int aCount=0;aCount<=theList.Size();aCount++)
	{
		int aGet=_min(aCount,theList.Size()-1);
		if (aCount==theList.Size()) aPos=theList[aGet].mPos[1];
		else aPos=theList[aGet].mPos[0];
		Vector aNorm=theNormals[aGet];

		aKit->mV[aPutV].mPos=aPos-aNorm*theThickness;
		aKit->mV[aPutV].mUV=Point(0,0);
		aPutV++;

		aKit->mV[aPutV].mPos=aPos+aNorm*theThickness;
		aKit->mV[aPutV].mUV=Point(0,0);
		aPutV++;
	}

	int aPutI=0;
	for (int aCount=0;aCount<theList.Size();aCount++)
	{
		int aBase=aCount*2;
		aKit->mI[aPutI++]=aBase;
		aKit->mI[aPutI++]=aBase+2;
		aKit->mI[aPutI++]=aBase+1;
		aKit->mI[aPutI++]=aBase+1;
		aKit->mI[aPutI++]=aBase+2;
		aKit->mI[aPutI++]=aBase+3;
	}

	aKit->mV.Clip(aPutV);
	aKit->mI.Clip(aPutI);

	return aKit;
}

#endif