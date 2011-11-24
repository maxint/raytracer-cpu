/********************************************************************
	created:	2010/04/13
	file name:	raytracer.h
	author:		maxint lnychina@gmail.com
*********************************************************************/

#ifndef _RT_RAYTRACER_H_
#define _RT_RAYTRACER_H_

#include "common.h"
#include <vector>

class QImage;

#define RT_SAMPLES			128
#define RT_REGULAR_SAMPLES	8

namespace trimeshVec{
	class CAccessObj;
}

namespace RayTracer {

// ------------------------------------------------------------------------------
// Ray class definition
// ------------------------------------------------------------------------------

class Ray
{
public:
	Ray()
		: mOrigin(0,0,0)
		, mDirection(0,0,0)
		, mID(0)
	{}
	
	Ray(const Vec3& aOrig, const Vec3& aDir, int aID)
		: mOrigin(aOrig)
		, mDirection(aDir)
		, mID(aID)
	{}

	void setOrigin(const Vec3& aOrig)
	{
		mOrigin = aOrig;
	}
	void setDirection(const Vec3& aDir)
	{
		mDirection = aDir;
	}
	const Vec3& getOrigin() const
	{
		return mOrigin;
	}
	const Vec3& getDir() const
	{
		return mDirection;
	}
	int getID() const { return mID; }
	void setID(int val) { mID = val; }

private:
	Vec3 mOrigin;
	Vec3 mDirection;
	int mID;
};

// ------------------------------------------------------------------------------
// Ray tracer Engine Core
// ------------------------------------------------------------------------------
class Scene;
class Primitive;
class Twister;
class CCamera;
class Light;

class Engine
{
public:
	Engine();
	~Engine();

	/**	Set the render target canvas
	\param
		_w		width of the canvas
		_h		height of the canvas
		_img	which image it is rendered to?
	 */
	void setRenderTarget(int _w, int _h, QImage *_img);
	Scene* getScene()
	{
		return mScene;
	}
	int getNumOfPrimitives() const;

	/**	Naive ray tracing
		Intersects the ray with every primitives in the scene to determine the 
		closest intersection.
	\param
		_ray	light ray
		_acc	final accumulated color
		_dist	the closest distance
		_depth	maximum recurse depth
		_rIndex	refraction index
	\return
		if no intersection return 0, otherwise the intersected primitive
	 */
	Primitive* rayTrace(const Ray& _ray, Color& _acc, Real& _dist, 
		int _depth, Real _rIndex);

	/**	Initializes the engine renderer
		Reset the line / tile counters and precalculate some values
	\param
		aPos	the camera position
		aTarget	the looking at position 
	 */
	void initEngine(const Vec3& aPos, const Vec3& aTarget);

	/**	Fires rays in the scene one scan line at a time, from left to right
	\return
		true	render completed
		false	over time, continue to render next time
	 */
	bool render();

	/**	Get and set tracing depth
	 */
	int getTraceDepth() const { return mTraceDepth; }
	void setTraceDepth(int val) { mTraceDepth = std::min(val, RT_TRACEDEPTH); }

	/**	Get current progress
	 */
	int getCurrProgree() const { return static_cast<int>(mCurrLine * 100.0f / mHeight); }

	/**	Find the nearest intersection in a regular grid for a ray
	\param
		aRay	light ray
		aDist	the nearest distance
		aPrim	the nearest primitive
	 */
	RTResult findNearest(const Ray& aRay, Real& aDist, Primitive*& aPrim);

	/**	Helper function, fire one ray in the regular grid
	\param
		aScreenPos	position of the screen to trace from
		aAccClr		final color return
	\return
		the nearest primitive intersected
	 */
	Primitive* renderRay(Real x, Real y, Color& aAccClr);

	/**	Determine the light intensity received from a point light (in case of 
		a SHPERE primitive) or an area light (in case of an AABB primitive)
	\param
		aLight	the light
		aIP		the intersected position
		aDir	return light direction
	\return 
		shade parameter, 0~1. 
		When it's point light, 0 indicates in shadow, 1 indicates in light.
		When it's area light, return the proportion of light region.
	 */
	Real calcShade(const Light* aLight, const Vec3& aIP, Vec3& aDir);

	/**	Get and set regular sample size of light
	 */
	int getRegularSampleSize() const { return mRegularSampleSize; }
	void setRegularSampleSize(int val) { mRegularSampleSize = std::min(std::max(1, val), RT_REGULAR_SAMPLES); }

	/**	Load obj model file
	 */
	void loadObjModel(const trimeshVec::CAccessObj* accessObj);

private:
	/**	Set the color of frame buffer
	 */
	void _setFrameBuffer(int _y, int _x, const Color& _clr);

private:
	typedef std::vector<Primitive*> PrimitiveList;

private:
	bool mCreated;
	Scene* mScene;
	int mWidth, mHeight;
	Real mRatio;
	QImage* mImage;
	int mCurrLine;
	Real mDx, mDy, mSx, mSy;
	Vec3 mRCS;	// 1 / size of a cell
	Vec3 mCS;	// size of a cell
	int mCurRayID;

	/// last line primitives
	PrimitiveList mLastLinePrims;

	/// benchmark related
	int mTraceDepth;
	int mRegularSampleSize;
	Real mSampleScale;
	Real mSampleOffset;
	Real mSampleScale2;

	/// random number generator
	Twister* mTwister;
	CCamera* mCamera;
};

}; // namespace RayTracer

#endif // _RT_RAYTRACER_H_