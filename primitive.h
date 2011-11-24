/********************************************************************
	created:	2010/04/22
	file name:	primitive.h
	author:		maxint lnychina@gmail.com
*********************************************************************/

#ifndef _RT_PRIMITIVE_H_
#define _RT_PRIMITIVE_H_

#include "common.h"

#pragma warning(disable:4800) // int to bool

namespace RayTracer {

// ------------------------------------------------------------------------------
// Primitive class definition
// ------------------------------------------------------------------------------

class Ray;
class Material;

class Primitive
{
public:
	enum PrimType
	{
		PT_SPHERE,
		PT_PLANE,
		PT_BOX,
		PT_TRIANGLE
	};
	Primitive();
	virtual ~Primitive();

	Material* getMaterial() const		{ return mMaterial; }
	void setMaterial(Material* aMat)	{ mMaterial = aMat; }
	void setMaterial(const String& matName);
	virtual Color getColor(const Vec3& aIP) const;

	virtual PrimType getType() const = 0;

	/**	Intersection detection
	\param
		aRay	light ray
		aDist	intersection distance
	\return
		Ray tracing result, hit, miss or inside
	 */
	virtual RTResult intersect(const Ray& aRay, Real& aDist) = 0;
	
	/**	Whether it intersects with a AABB
	 */
	virtual bool intersetBox(const AABB& aBox) const = 0;

	/**	Return normalized normal direction
	 */
	virtual const Vec3& getNormal(const Vec3& aPos) = 0;

	virtual const AABB& getAABB() const
	{
		return mAABB;
	}
	virtual void setLight(bool isLight)	{ mIsLight = isLight; }
	bool isLight() const				{ return mIsLight; }

	void setName(const String& aName)	{ mName = aName; }
	const String& getName() const		{ return mName; }
	int getLastRayID() const			{ return mRayID; }

protected:
	virtual void getTextureCoord(Real& u, Real& v, const Vec3& aIP) const = 0;

protected:
	Material* mMaterial;
	String mName;
	bool mIsLight;
	AABB mAABB;
	int mRayID;
};

// ------------------------------------------------------------------------------
// Sphere primitive class definition
// ------------------------------------------------------------------------------

class Sphere : public Primitive
{
public:
	Sphere(const Vec3& aCentre, Real aRadius)
		: mCentre(aCentre)
		, mRadius(aRadius)
		, mSqrRadius(aRadius * aRadius)
		, mRRadius(1.0f / aRadius)
	{
		mAABB = AABB(-aRadius * Vec3::ONE + aCentre, aRadius * Vec3::ONE + aCentre);
	}

	// override from Primitive
	PrimType getType() const
	{
		return PT_SPHERE;
	}
	RTResult intersect(const Ray& aRay, Real& aDist);
	bool intersetBox(const AABB& aBox) const;
	const Vec3& getNormal(const Vec3& aPos)
	{
		mNormal = (aPos - mCentre) *  mRRadius;
		return mNormal;
	}

	const Vec3& getCentre() const
	{
		return mCentre;
	}
	Real getSqrRadius()
	{
		mSqrRadius;
	}

private:
	// override from Primitive
	void getTextureCoord(Real& u, Real& v, const Vec3& aIP) const ;

private:
	Vec3 mCentre;
	Vec3 mNormal;
	Real mSqrRadius, mRadius, mRRadius;
};

// ------------------------------------------------------------------------------
// Plane primitive
// ------------------------------------------------------------------------------

class PlanePrim : public Primitive
{
public:
	PlanePrim(const Vec3& aNormal, Real aDist);

	// override from Primitive
	PrimType getType() const
	{
		return PT_PLANE;
	}
	const Vec3& getNormal(const Vec3& )
	{
		return mPlane.N;
	}
	RTResult intersect(const Ray& aRay, Real& aDist);
	bool intersetBox(const AABB& aBox) const;

	const Vec3& getNormal() const
	{
		return mPlane.N;
	}
	Real getD()
	{
		return mPlane.D;
	}

private:
	// override from Primitive
	void getTextureCoord(Real& u, Real& v, const Vec3& aIP) const;

private:
	Plane mPlane;
	Vec3 mUAxis, mVAxis;
};

// ------------------------------------------------------------------------------
// Box primitive class definition
// ------------------------------------------------------------------------------

class Box : public Primitive
{
public:
	Box()
	{}
	Box(const AABB& aBox)
	{
		mAABB = aBox;
	}

	// override from Primitive
	PrimType getType() const
	{
		return PT_BOX;
	}
	RTResult intersect(const Ray& aRay, Real& aDist);
	bool intersetBox(const AABB& aBox) const
	{
		return mAABB.interset(aBox);
	}
	const Vec3& getNormal(const Vec3& aPos);

	bool contains(const Vec3& aPos) const
	{
		return mAABB.contains(aPos);
	}
	const Vec3& getPos() const
	{
		return mAABB.getMin();
	}
	const Vec3& getDim() const
	{
		return mAABB.getMax();
	}

private:
	// override from Primitive
	void getTextureCoord(Real& , Real& , const Vec3& ) const {};

private:
	Vec3 mNormal;
};

// ------------------------------------------------------------------------------
// A Triangle Primitive
// ------------------------------------------------------------------------------

/// A vertex with position, normal and texcoords
class Vertex
{
public:
	Vec3 mPos, mNormal;
	Real mU, mV;
public:
	Vertex() {}
	Vertex(const Vec3& aPos, const Vec3& aNormal = Vec3::ZERO, Real u = 0, Real v = 0)
		: mPos(aPos), mNormal(aNormal)
		, mU(u), mV(v)
	{}
};

class TrianglePrim : public Primitive
{
public:
	TrianglePrim(Vertex* v1, Vertex* v2, Vertex* v3, bool genNorm = false);

	// override from Primitive
	PrimType getType() const	{ return PT_TRIANGLE; }
	RTResult intersect(const Ray& aRay, Real& aDist);
	bool intersetBox(const AABB& aBox) const;
	const Vec3& getNormal(const Vec3& aPos);

private:
	// override from Primitive
	void getTextureCoord(Real& u, Real& v, const Vec3& aIP) const;

private:
	Vertex* mVertices[3];
	Vec3 mN;
	int mMajorAxis; // 法向主轴
	Vec3 mBaryCoord;	// 求交计算结果，用于求法向
	Real mBx, mBy, mCx, mCy; // 求质心坐标的预计算结果
	Vec3 mNormal;
};

// ------------------------------------------------------------------------------
// Light class definition
// ------------------------------------------------------------------------------

class Light
{
public:
	enum LightType
	{
		LT_NONE,
		LT_POINT,
		LT_DIRECTIONAL,
		LT_SPOT,
		LT_AREA,
	};
	enum StateToggler
	{
		AMBIENT = 1,
		DIFFUSE = 1<<1,
		SPECULAR = 1<<2
	};
public:
	Light()
		: mAmbient(0.2f,0.2f,0.2f)
		, mDiffuse(0.8f,0.8f,0.8f)
		, mSpecular(0,0,0)
		, mPosition(0,0,1)
		, mDirection(0,0,-1)
		, mSpotFallOff(180)
		, mAttenuation0(1)
		, mAttenuation1(0)
		, mAttenuation2(0)
		, mSpotExponent(30)
		, mState(AMBIENT | DIFFUSE)
	{}

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

	// directional light setting
	const Vec3& getDirection() const { return mDirection; }
	void setDirection(const Vec3& val) 
	{ 
		mDirection = val;  
		mDirection.Normalize();
	}
	void setDirection(Real nx, Real ny, Real nz)
	{ 
		mDirection.x = nx;
		mDirection.y = ny;
		mDirection.z = nz;
		mDirection.Normalize();
	}

private:
	void _setColorState(const Color& clr, StateToggler tog)
	{
		if (clr.Length() > RT_EPSILON)
			mState |= tog;
		else
			mState &= ~tog;
	}
private:
	Color mAmbient, mDiffuse, mSpecular;	// 需要状态判断
	Vec3 mDirection;
public:
	LightType mType;
	Real mShininess;
	Vec3 mPosition;
	Real mSpotFallOff;
	Real mSpotExponent;
	Real mAttenuation0;
	Real mAttenuation1;
	Real mAttenuation2;
	AABB mAABB;

	int mState;
};

}; // namespace RayTracer

#endif // _RT_PRIMITIVE_H_