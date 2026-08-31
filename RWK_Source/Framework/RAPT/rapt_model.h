#pragma once
#include "rapt.h"

#if !defined(LEGACY_GL) && !defined(LEGACY_MODEL)

#include "util_core.h"
#include "rapt_string.h"
#include "rapt_point.h"
#include "rapt_color.h"
#include "rapt_rect.h"
#include "rapt_matrix.h"
#include "rapt_object.h"
#include "rapt_shaders.h"

class Model
{
public:
	void Load(String theFN, Sprite* theSprite=NULL);

	//void Load(Array<Vertex2DN>& theV, Array<vbindex>& theI);

	//
	// All MODLs have vertex format
	// 	   Vector mPos;
	// 	   Vector mNormal;
	// 	   Point mUV;
	//

	inline void Draw(Vector thePos) {Matrix aMat;aMat.Translate(thePos);Draw(aMat);}
	inline void Draw(Matrix theMat) {gGlobalGraphics->SetWorldMatrix(theMat);Draw();}
	void Draw();

	void ConnectToShader(Shader* theShader, bool wipeData=true);

	static void ForceShader(Shader* theShader=NULL);
	static Shader* mForceShader;

#ifdef _DEBUG
	String			mFilename;
#endif

	int				mShape=-1;
	Shader*			mShader=NULL;
	Sprite*			mSprite=NULL;

	Cube			mBounds;
	Array<Key3D>	mKey;

	VertexKit			mData;
	inline VertexKit&	GetData() {return mData;}
};

template<typename vtype>
class ModelLite
{
public:
	Array<vtype> mV;
	Array<vbindex> mI;
};

#endif