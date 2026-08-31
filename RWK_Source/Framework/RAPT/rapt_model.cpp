#include "rapt_model.h"

#ifndef LEGACY_GL

void Model::ForceShader(Shader* theShader) {mForceShader=theShader;}
Shader* Model::mForceShader=NULL;

void Model::Load(String theFN, Sprite* theSprite)
{
	mSprite=theSprite;

	//
	// ID Block...?
	//
	// P = has Pos
	// N = has Normal
	// U = has UV
	// D = has Diffuse
	//

	IOBuffer aBuffer;
	aBuffer.Load(theFN);
	mData.Reset();

	short aVersion=aBuffer.ReadShort();

	if (aVersion==1)
	{
		mBounds=aBuffer.ReadCube();

		//
		// Keys...
		//
		int aKeys=aBuffer.ReadInt();
		if (aKeys>0) {mKey.GuaranteeSize(aBuffer.ReadInt());for (int aCount=0;aCount<mKey.Size();aCount++) {mKey[aCount].mPos=aBuffer.ReadVector();mKey[aCount].mRotation=aBuffer.ReadVector();}}

		//
		// Vertices...
		//
		String aVertID="PNU"; // Eventually embed this in the file?

		int aVCount=aBuffer.ReadInt();
		mData.mPos.GuaranteeSize(aVCount);
		mData.mNormal.GuaranteeSize(aVCount);
		mData.mUV.GuaranteeSize(aVCount);

		for (int aCount=0;aCount<aVCount;aCount++)
		{
			for (int aFCount=0;aFCount<aVertID.Len();aFCount++)
			{
				switch (aVertID[aFCount])
				{
				case 'P': mData.mPos[aCount]=aBuffer.ReadVector();break;
				case 'N': mData.mNormal[aCount]=aBuffer.ReadVector();break;
				case 'U': mData.mUV[aCount]=aBuffer.ReadPoint();break;
				case 'D': mData.mDiffuse[aCount]=aBuffer.ReadInt();break;
				}
			}
		}

		mData.mI.GuaranteeSize(aBuffer.ReadInt());
		for (int aCount=0;aCount<mData.mI.Size();aCount++) {mData.mI[aCount]=aBuffer.ReadVBIndex();}

		//
		// Map UVs to our sprite...
		//
		if (mSprite) mData.FixUV(&mSprite->GetTextureQuad());
	}

#ifdef _DEBUG
	mFilename=theFN;
#endif
}


void Model::ConnectToShader(Shader* theShader, bool wipeData)
{
	if (mShape>=0) return;
	//
	// Remember, the shader has to override ::StreamShape for this.
	// ModL contains pos,normal,uv ... but your shader might not need all that!
	//	When I was working on Hamsterball, my Tarboi objects weren't showing up, but it turns out the working shader was not set up for
	//	pos,normal,uv ... so nothing appeared.
	//
	if (mData.mPos.Size()>0)
	{
		Quad aUV;
		if (mSprite) aUV=mSprite->GetTextureQuad();
		mShader=theShader;
		if (mShape!=-1) 
		{
			#ifdef _DEBUG
			gOut.Out("*Warning: Re-casting Model to another shader [%s]!",mFilename.c());
			#endif		
			gG.UnloadShape(mShape);
		}
		//
		// REMEMBER, mShader->StreamShape() is going to get called!
		// (LoadModel is a helper so that if it has a sprite, the UVs get mapped)
		//
		mShape=mShader->StreamShape(&mData);
		if (wipeData) mData.Reset();
	}
	#ifdef _DEBUG
	else 
	{
		gOut.Out("! ERROR Model::ConnectToShader ... mData.Len()==0 !!!");
	}
	#endif
}

void Model::Draw()
{
	if (mSprite) gG.SetTexture(mSprite->mTexture);
	if (mForceShader) 
	{
		gG.SetShader(mForceShader);
		mForceShader->DrawShape(mShape);
	}
	else 
	{
		gG.SetShader(mShader);
		mShader->DrawShape(mShape);
	}
}

/*
void Model::Load(Array<Vertex2DN>& theV, Array<vbindex>& theI)
{
	if (!mShader) mShader=&gSh.mShaderN;
	mShader->CreateShape(theV.mArray,theV.Size(),theI);
}
*/


#endif