/********************************************************************
	created:	2010/04/22
	file name:	material.h
	author:		maxint lnychina@gmail.com
*********************************************************************/

#ifndef _RT_MATERIAL_H_
#define _RT_MATERIAL_H_

#include "common.h"
#include <map>

#pragma warning(disable:4800) // int to bool

namespace RayTracer {

// ------------------------------------------------------------------------------
// Custom Texture class
// ------------------------------------------------------------------------------

class Texture
{
public:
	Texture(const String& aFilename);
	Texture(Color* data, int w, int h);
	~Texture()					{ SAFE_DELETE_ARRAY(mBitmap); }
	Color* getBitmap()			{ return mBitmap; }
	Color getTexel(Real u, Real v) const;
	int getWidth() const		{ return mWidth; }
	int getHeight() const		{ return mHeight; }
private:
	Color* mBitmap;
	int mWidth, mHeight;
};

// ------------------------------------------------------------------------------
// Simple Texture Manager
// ------------------------------------------------------------------------------

class TextureManager
{
public:	
	static TextureManager& getInstance();
	Texture* createFromFile(const String& filename, const String& texName = "");
	Texture* createFromData(Color* data, int w, int h, const String& texName = "");
	Texture* getTexture(const String& texName);
private:
	TextureManager();
	TextureManager(const TextureManager&);
	TextureManager& operator=(const TextureManager&);
	~TextureManager();
private:
	typedef std::map<String, Texture*>	TextureList;
	typedef TextureList::iterator		TexListItor;
	typedef std::pair<String, Texture*>	TexListPair;
private:
	TextureList mTexturePool;
	int mIDCounter;
};

// ------------------------------------------------------------------------------
// Material class definition
// ------------------------------------------------------------------------------

class Material
{
public:
	enum StateToggler
	{
		AMBIENT = 1,
		DIFFUSE = 1<<1,
		SPECULAR = 1<<2,
		EMISSION = 1<<3,
		REFLECTION = 1<<4,
		REFRACTION = 1<<5,
		DIFFUSEREFL = 1<<6
	};
	Material()
		: mAmbient(0.2f,0.2f,0.2f)
		, mDiffuse(0.8f,0.8f,0.8f)
		, mSpecular(0,0,0)
		, mEmission(0,0,0)
		, mShininess(0)
		, mReflection(0)
		, mDiffuseRefl(0)
		, mRefraction(0)
		, mRefrIndex(1.5f)
		, mTexture(0)
		, mUScale(1)
		, mVScale(1)
		, mState(AMBIENT | DIFFUSE)
	{}
	~Material()	{}

	// ambient
	void setAmbient(Real _r, Real _g, Real _b)
	{
		mAmbient.r = _r;
		mAmbient.g = _g;
		mAmbient.b = _b;
		_setColorState(mAmbient, AMBIENT);
	}
	const Color& getAmbient() const { return mAmbient; }
	bool isAmbient() const { return (mState & AMBIENT); }
	void setAmbient(const Color& val) { setAmbient(val.r, val.g, val.b);  }

	// diffuse
	void setDiffuse(Real _r, Real _g, Real _b)
	{
		mDiffuse.r = _r;
		mDiffuse.g = _g;
		mDiffuse.b = _b;
		_setColorState(mDiffuse, DIFFUSE);
	}
	const Color& getDiffuse() const { return mDiffuse; }
	bool isDiffuse() const { return (mState & DIFFUSE); }
	void setDiffuse(const Color& val) { setDiffuse(val.r, val.g, val.b); }

	// specular
	void setSpecular(Real _r, Real _g, Real _b)
	{
		mSpecular.r = _r;
		mSpecular.g = _g;
		mSpecular.b = _b;
		_setColorState(mSpecular, SPECULAR);
	}
	const Color& getSpecular() const { return mSpecular; }
	bool isSpecular() const { return (mState & SPECULAR); }
	void setSpecular(const Color& val) { setSpecular(val.r, val.b, val.g); }

	// shininess
	Real getShininess() const { return mShininess; }
	void setShininess(Real val) { mShininess = val; }

	// emission
	void setEmission(Real _r, Real _g, Real _b)
	{
		mEmission.r = _r;
		mEmission.g = _g;
		mEmission.b = _b;
		_setColorState(mEmission, EMISSION);
	}
	const Color& getEmission() const { return mEmission; }
	bool isEmission() const { return (mState & EMISSION); }
	void setEmission(const Color& val) { setEmission(val.r, val.g, val.b); }

	// reflection
	Real getReflection() const { return mReflection; }
	bool isReflection() const { return (mState & REFLECTION); }
	void setReflection(Real val) { mReflection = val;  _setValueState(val, REFLECTION); }

	Real getDiffuseRefl() const { return mDiffuseRefl; }
	bool isDiffuseRefl() const { return (mState & DIFFUSEREFL); }
	void setDiffuseRefl(Real val) { mDiffuseRefl = val;  _setValueState(val, DIFFUSEREFL); }

	// refraction
	Real getRefraction() const { return mRefraction; }
	bool isRefraction() const { return (mState & REFRACTION); }
	void setRefraction(Real val) { mRefraction = val;  _setValueState(val, REFRACTION); }

	Real getRefrIndex() const { return mRefrIndex; }
	void setRefrIndex(Real val) { mRefrIndex = val; }

	// texture
	Texture* getTexture() const { return mTexture; }
	bool isTexture() const { return mTexture!=NULL; }
	void setTexture(Texture* val) { mTexture = val; }
	void setTexture(const String& texName);

	void setUVScale(Real uScale, Real vScale)
	{ 
		mUScale = uScale; 
		mVScale = vScale; 
	}
	Real getUScale() const { return mUScale; }
	Real getVScale() const { return mVScale; }
private:
	void _setColorState(const Color& clr, StateToggler tog)
	{
		if (clr.Length() > RT_EPSILON)
			mState |= tog;
		else
			mState &= ~tog;
	}
	void _setValueState(Real val, StateToggler tog)
	{
		if (abs(val) > RT_EPSILON)
			mState |= tog;
		else
			mState &= ~tog;
	}
private:
	Color mAmbient, mDiffuse, mSpecular, mEmission;
	Real mShininess;
	Real mReflection, mDiffuseRefl;
	Real mRefraction, mRefrIndex;
	Texture* mTexture;
	Real mUScale, mVScale;
	int mState;
};

// ------------------------------------------------------------------------------
// Custom Material Manager
// ------------------------------------------------------------------------------

class MaterialManager
{
public:
	static MaterialManager& getInstance();
	Material* createManual(const String& matName = "");
	Material* getMaterial(const String& matName);
private:
	MaterialManager();
	MaterialManager(const MaterialManager&);
	MaterialManager& operator=(const MaterialManager&);
	~MaterialManager();
private:
	typedef std::map<String, Material*> MaterialMap;
	typedef MaterialMap::iterator		MatMapItor;
private:
	MaterialMap mMaterialPool;
	int mIDCounter;
};

}; // namespace RayTracer

#endif // _RT_MATERIAL_H_