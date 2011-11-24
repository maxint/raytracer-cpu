/********************************************************************
	created:	2010/04/22
	file name:	material.cpp
	author:		maxint lnychina@gmail.com
*********************************************************************/

#include "material.h"

#include <QImage>
#include <sstream>

namespace RayTracer {

using std::min;
using std::max;
using std::abs;

static const String _DEFAULT_NAME = "_default_";

// ------------------------------------------------------------------------------
// Texture class implementation
// ------------------------------------------------------------------------------

Texture::Texture(const String& aFilename)
: mBitmap(0)
, mWidth(0)
, mHeight(0)
{
	QImage img(aFilename.c_str());
	if (img.isNull())
	{
		return;
	}
	mWidth = img.width();
	mHeight = img.height();
	mBitmap = new Color[mWidth * mHeight];
	Real reci  = 1.0f / 256;
	for (int x=0; x<mWidth; ++x) for (int y=0; y<mHeight; ++y)
	{
		QRgb clr = img.pixel(x, y);
		mBitmap[x + y*mWidth] = Color(qRed(clr), qGreen(clr), qBlue(clr)) * reci;
	}
}

Texture::Texture(RayTracer::Color *data, int w, int h)
: mBitmap(data)
, mWidth(w)
, mHeight(h)
{}

Color Texture::getTexel(Real u, Real v) const
{
	u = fmod(u, Real(1.0));
	v = fmod(v, Real(1.0));
	u = (u < 0.0f) ? (u + 1.0f) : u;
	v = (v < 0.0f) ? (v + 1.0f) : v;
	v = 1.0f - v;
	// fetch a bilinearly filtered texel
	Real fu = u * mWidth;
	Real fv = v * mHeight;
	int u1 = static_cast<int>(fu) % mWidth;
	int u2 = static_cast<int>(fu +1) % mWidth;
	int v1 = static_cast<int>(fv) % mHeight;
	int v2 = static_cast<int>(fv +1) % mHeight;
	
	// calculate fractional parts of u and v
	Real fracu = fu - floorf(fu);
	Real fracv = fv - floorf(fv);

	// calculate weight factors
	Real w1 = (1 - fracu) * (1 - fracv);
	Real w2 = fracu * (1 - fracv);
	Real w3 = (1 - fracu) * fracv;
	Real w4 = fracu *  fracv;

	// fetch four texels
	Color c1 = mBitmap[u1 + v1 * mWidth];
	Color c2 = mBitmap[u2 + v1 * mWidth];
	Color c3 = mBitmap[u1 + v2 * mWidth];
	Color c4 = mBitmap[u2 + v2 * mWidth];

	// scale and sum the four colors
	return c1 * w1 + c2 * w2 + c3 * w3 + c4 * w4;
}

// ------------------------------------------------------------------------------
// Texture Manager class implementation
// ------------------------------------------------------------------------------

TextureManager::TextureManager()
: mIDCounter(0)
{}

TextureManager::~TextureManager()
{
	TexListItor it = mTexturePool.begin();
	TexListItor it_end = mTexturePool.end();
	for (; it!=it_end; ++it)
	{
		SAFE_DELETE(it->second);
	}
	mTexturePool.clear();
}

TextureManager& TextureManager::getInstance()
{
	static TextureManager mInstance;
	return mInstance;
}

Texture* TextureManager::createFromFile(const String& filename, const String& texName /* = */ )
{
	if (texName.length() == 0)
	{
		std::ostringstream oss;
		oss << "_Tex" << ++mIDCounter;
		Texture *tex = new Texture(filename);
		std::cout << "INFO: Texture " << oss.str() << " is created!" << std::endl;
		mTexturePool.insert(TexListPair(oss.str(), tex));
		return tex;
	}

	TexListItor it = mTexturePool.find(texName);
	if (it != mTexturePool.end())
	{ // found
		std::cout << "WARNING: Texture " << texName << " has existed!" << std::endl;
		return it->second;
	}
	else
	{
		Texture *tex = new Texture(filename);
		mTexturePool.insert(TexListPair(texName, tex));
		return tex;
	}
}

Texture* TextureManager::createFromData(Color* data, int w, int h, const String& texName /* = */ )
{
	if (texName.length() == 0)
	{
		std::ostringstream oss;
		oss << "_Tex" << ++mIDCounter;
		Texture *tex = new Texture(data, w, h);
		std::cout << "INFO: Texture " << oss.str() << " is created!" << std::endl;
		mTexturePool.insert(TexListPair(oss.str(), tex));
		return tex;
	}

	TexListItor it = mTexturePool.find(texName);
	if (it != mTexturePool.end())
	{ // found
		std::cout << "WARNING: Texture " << texName << " has existed!" << std::endl;
		return it->second;
	}
	else
	{
		Texture *tex = new Texture(data, w, h);
		mTexturePool.insert(TexListPair(texName, tex));
		return tex;
	}
}

Texture* TextureManager::getTexture(const String& texName)
{
	TexListItor it = mTexturePool.find(texName);
	if (it != mTexturePool.end())
	{ // found
		return it->second;
	}
	else
	{
		std::cout << "WARNING: Texture " << texName << " did not existed!" << std::endl;
		return NULL;
	}
}

// ------------------------------------------------------------------------------
// Material class implementation
// ------------------------------------------------------------------------------

void Material::setTexture(const String& texName)
{
	mTexture = TextureManager::getInstance().getTexture(texName);
}

// ------------------------------------------------------------------------------
// Material Manager class implementation
// ------------------------------------------------------------------------------

MaterialManager::MaterialManager()
: mIDCounter(0)
{
	Material *mat = new Material();
	mMaterialPool.insert(std::make_pair(_DEFAULT_NAME, mat));
}

MaterialManager::~MaterialManager()
{
	MatMapItor it = mMaterialPool.begin();
	MatMapItor it_end = mMaterialPool.end();
	for (; it!=it_end; ++it)
	{
		SAFE_DELETE(it->second);
	}
	mMaterialPool.clear();
}

MaterialManager& MaterialManager::getInstance()
{
	static MaterialManager mInstance;
	return mInstance;
}

Material* MaterialManager::createManual(const String& matName /* = "" */)
{
	if (matName.length() == 0)
	{
		std::ostringstream oss;
		oss << "_Mat" << ++mIDCounter;
		Material* mat = new Material();
		std::cout << "INFO: Material " << oss.str() << " is created!" << std::endl;

		mMaterialPool.insert(std::make_pair(oss.str(), mat));
		return mat;
	}

	MatMapItor it = mMaterialPool.find(matName);
	if (it!=mMaterialPool.end())
	{// found
		std::cout << "WARNING: Material " << matName << " has existed!" << std::endl;
		return it->second;
	}
	else
	{
		Material* mat = new Material();
		mMaterialPool.insert(std::make_pair(matName, mat));
		return mat;
	}
}

Material* MaterialManager::getMaterial(const String& matName)
{
	MatMapItor it = mMaterialPool.find(matName);
	if (it!=mMaterialPool.end())
	{
		return it->second;
	}
	else
	{
		std::cout << "WARNING: Material " << matName << " did not existed!" << std::endl;
		return mMaterialPool[_DEFAULT_NAME];
	}
}

}; // namespace RayTracer