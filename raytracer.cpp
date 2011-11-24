/********************************************************************
	created:	2010/04/13
	file name:	raytracer.cpp
	author:		maxint lnychina@gmail.com
*********************************************************************/

#include "raytracer.h"
#include "scene.h"
#include "primitive.h"
#include "material.h"
#include "twister.h"
#include "Camera.h"

#include <QImage>
#include <ctime>

#define SATURATE(x) ( ((x)>255) ? 255 : (((x)<0) ? 0 : (x)) )
#define ROUND(x) int((x)+0.5)
#define FAR_DISTANCE 1000000.0f
#define MAX_RENDER_TIME 100

#define REFRACTION_SHADE 0.3f

namespace RayTracer {

static const Vec3 EYE_POS(0, 0, -5);
static const Color DEFAULT_COLOR(1.0f, 1.0f, 1.0f);

#ifdef _DEBUG
using std::cout;
using std::endl;
#endif

Engine::Engine()
: mScene(new Scene())
, mCreated(false)
, mTraceDepth(4)
, mRegularSampleSize(3)
, mTwister(new Twister())
, mCamera(new CCamera())
{
	// initialize scene
	mScene->initScene();
#ifdef _DEBUG
	cout << mTwister->Rand() << endl;
#endif
}

Engine::~Engine()
{
	SAFE_DELETE(mScene);
	SAFE_DELETE(mTwister);
	SAFE_DELETE(mCamera);
}

void Engine::setRenderTarget(int _w, int _h, QImage *_img)
{
	mWidth = _w;
	mHeight = _h;
	mRatio = mWidth * 1.0f  / mHeight;
	mImage = _img;

	mCreated = true;
}

RTResult Engine::findNearest(const Ray& aRay, Real& aDist, Primitive*& aPrim)
{
	int i, gidx;
	RTResult retval = MISS, ret;
	// 在世界坐标系下计算
	Vec3 curPos = aRay.getOrigin();
	Vec3 rayDir = aRay.getDir();
	/*	NOTE1: 经调试，Cell坐标系下的步进要小心，因为Cell一般不是正方体，所以要
		规一化@vDelta步进。
		NOTE2：如果直接在世界坐标系下计算，停止条件判断可以更简单
	*/
	Vec3 curCell, vStep, vOut;	// Cell坐标系下，整数步进
	Vec3 vMax, vDelta;			// 在世界坐标系下，小数步进
	Real tMinStep;
	int tMinAxis;
	Vec3 tIntPos;
	bool tIsIntersected;

	curCell = (curPos - mScene->getExtends().getMin()) * mRCS;
	if ( !(curCell > 0.0f) || !(curCell < RT_GRIDSIZE) ) 
		return MISS;

	// DDA algorithm initialization
	for (i=0; i<3; ++i)
	{
		if (rayDir[i] > 0)
		{
			vStep[i] = 1; 
			vOut[i] = RT_GRIDSIZE;
			vDelta[i] = mCS[i] / rayDir[i];
			vMax[i] = (static_cast<int>(curCell[i]) + 1 - curCell[i]) * vDelta[i];
		}
		else if(rayDir[i] < 0)
		{
			vStep[i] = -1;
			vOut[i] = -1;
			vDelta[i] = -mCS[i] / rayDir[i];
			vMax[i] = (curCell[i] - static_cast<int>(curCell[i])) * vDelta[i];
		}
		else 
		{
			vStep[i] = 0;
			vOut[i] = 0;
			vDelta[i] = 0;
			vMax[i] = 1000000;
		}
		curCell[i] = floor(curCell[i]);
	}
	Scene::ObjectList *list = 0;
	Scene::GridMap &grid = mScene->mGird;
	Scene::ObjectItor oit, oit_end;
	Primitive *prim;
	int X, Y, Z;
	// trace primary ray
	while (1)
	{
		X = static_cast<int>(curCell.x);
		Y = static_cast<int>(curCell.y);
		Z = static_cast<int>(curCell.z);
		gidx = X + (Y << RT_GRIDSHIFT) + (Z << (RT_GRIDSHIFT * 2));
		list = grid[gidx];
		tIsIntersected = false;
		if (list)
		{
			oit = list->begin();
			oit_end = list->end();
			for (; oit!=oit_end; ++oit)
			{
				prim = *oit;
				if (prim->getLastRayID() == aRay.getID() && aPrim == prim)
				{
					tIsIntersected = true;
				}
				else if ((ret = prim->intersect(aRay, aDist)) != MISS)
				{
					retval = ret;
					aPrim = prim;
					tIsIntersected = true;
				}
			}
		}

		// 得到步进最小轴，并进入下一个Cell
		tMinStep = vMax[0];
		tMinAxis = 0;
		for (i=1; i<3; ++i) if (vMax[i] < tMinStep)
		{
			tMinStep = vMax[i];
			tMinAxis = i;
		}

		// 判断交点是否在当前Cell中，如果是，找到交点，退出
		if (tIsIntersected && aDist < vMax[tMinAxis])
			break;

		curCell[tMinAxis] += vStep[tMinAxis];
		if (curCell[tMinAxis] == vOut[tMinAxis])
			return MISS;
		vMax[tMinAxis] += vDelta[tMinAxis];
	}
	return retval;
}

Real Engine::calcShade(const Light* aLight, const Vec3& aIP, Vec3& aDir)
{
	//return 1.0f;

	Real retval, tDist, tAtt;
	Primitive *prim = 0;
	int x, y;
	Vec3 dim;
	int tShadowed = 0;

	// handle point light source
	/*	1 for a visible lightPrim source
		0 for an occluded lightPrim
		*/
	switch (aLight->mType)
	{
	case Light::LT_DIRECTIONAL:
		aDir = aLight->getDirection();
		// NOTE: 如果offset不大，容易出现黑点
		if (findNearest(Ray(aIP + aDir * RT_EPSILON, aDir, ++mCurRayID), tDist, prim) != MISS && 
			!prim->isLight())
		{
			if (prim->getMaterial()->isRefraction())
				retval = REFRACTION_SHADE; // 可透射物体
			else
				retval = 0.0f;
		}
		else
			retval = 1.0f;
		break;

	case Light::LT_POINT:
		retval = 1.0f;
		aDir = aLight->mPosition - aIP;
		tDist = aDir.Length();
		aDir *= (1.0f / tDist);
		// NOTE: 如果offset不大，容易出现黑点
		if (findNearest(Ray(aIP + aDir * RT_EPSILON, aDir, ++mCurRayID), tDist, prim) != MISS && 
			!prim->isLight())
		{
			if (prim->getMaterial()->isRefraction())
				retval = REFRACTION_SHADE; // 可透射物体
			else
				retval = 0.0f;
		}
		else
		{
			tAtt = 1.0 / (aLight->mAttenuation0 + aLight->mAttenuation1 * tDist +
				aLight->mAttenuation2 * tDist * tDist);
			retval *= tAtt;
		}
		break;
	
	case Light::LT_AREA:
		retval = 0;
		dim = aLight->mAABB.getDim();
		aDir = aLight->mAABB.getMin() - aIP;
		// NOTE: 加速，大致判断是否在阴影中，如果不在就不采样了
		for (x=0; x<2; ++x) for (y=0; y<2; ++y)
		{
			Vec3 dir( aDir + dim * Vec3(x,y,y) );
			tDist = dir.Length();
			dir *= 1.0f / tDist;
			if ( findNearest(Ray(aIP + dir * RT_EPSILON, dir, ++mCurRayID), tDist, prim) != MISS)
			{
				++tShadowed;
				break;
			}
		}

		if (tShadowed == 4) // fully in shadow
			retval = 0;
		else if (tShadowed == 0) // fully in light
			retval = 1;
		else
		{ // partially in shadow
			retval = 0;
			for (x=0; x<mRegularSampleSize; ++x) for (y=0; y<mRegularSampleSize; ++y)
			{
				Vec3 dir( aDir + dim * (
					Vec3(x+mTwister->Rand(),y+mTwister->Rand(),y+mTwister->Rand()) 
					* mSampleScale) );
				tDist = dir.Length();
				dir *= 1.0f / tDist;
				if (findNearest(Ray(aIP + dir * RT_EPSILON, dir, ++mCurRayID), tDist, prim) == MISS ||
					prim->isLight())
					retval += mSampleScale2;
				else if (prim->getMaterial()->isRefraction()) // 可透射物体
					retval += mSampleScale2 * REFRACTION_SHADE;
			}
		}
		if (retval != 0)
		{
			tAtt = 1.0 / (aLight->mAttenuation0 + aLight->mAttenuation1 * tDist +
				aLight->mAttenuation2 * tDist * tDist);
			retval *= tAtt;
		}

		aDir += 0.5f * dim;
		aDir.Normalize();
		break;

	default:
		break;
	}

	return retval;
}

Primitive* Engine::rayTrace(const RayTracer::Ray &aRay, 
							Color &aAccClr, 
							Real& aDist,
							int aDepth, 
							Real aRIndex)
{
	if (!mCreated || aDepth > mTraceDepth) 
		return 0;

	// trace primary ray
	aDist = FAR_DISTANCE;
	Vec3 pi, normDir, viewDir, lightDir, reflDir, transDir;
	Ray shadowRay;
	Scene::LightItor lit, lit_end;
	Primitive *prim = 0;
	Light *lightPrim = 0;
	RTResult result = MISS;

	viewDir = aRay.getDir();

	// find the nearest intersection
	result = findNearest(aRay, aDist, prim);
	if (result == MISS) return 0;

	Material *primMat = prim->getMaterial();

	// handle intersection
	if (prim->isLight())
	{// we hit a lightPrim, stop tracing	
		// NOTE: 应该是累加，不然反走样时可能出现黑点
		aAccClr += DEFAULT_COLOR;
	}
	else
	{// determine color at point of intersection
		// intersection position
		pi = aRay.getOrigin() + viewDir * aDist;
		normDir = prim->getNormal(pi);
		reflDir = viewDir - (2.0f * viewDir.Dot(normDir) * normDir);
		Color color = prim->getColor(pi);

		// trace lights
		lit = mScene->mLights.begin();
		lit_end = mScene->mLights.end();
		for (; lit!=lit_end; ++lit)
		{
			lightPrim = *lit;

			// 1. add ambient light
			if (lightPrim->isAmbient() && primMat->isAmbient())
				aAccClr += primMat->getAmbient() * lightPrim->getAmbient() * color;

			/*	1 for a visible lightPrim source
				0 for an occluded lightPrim
				*/
			Real shade = calcShade(lightPrim, pi, lightDir);

			if (shade <=0 )
				continue;

			// 2. calculate diffuse shading
			if (lightPrim->isDiffuse() && primMat->isDiffuse())
			{
				Real diffDot = lightDir.Dot(normDir);
				if (diffDot > 0)
					aAccClr += diffDot * shade * lightPrim->getDiffuse() * color;
			}

			// 3. calculate specular shading
			if (lightPrim->isSpecular() && primMat->isSpecular())
			{
				// point lightPrim source: sample once for specular highlight
				// viewDir.Dot(lightReflDir) == lightDir.Dot(reflDir)
				Real specDot = lightDir.Dot(reflDir);
				if (specDot > 0)
					aAccClr += powf(specDot, primMat->getShininess()) * primMat->getSpecular()
					* shade * lightPrim->getSpecular();
			}
		}//end for lights

		// 4. calculate diffuse reflection
		if (primMat->isReflection() && aDepth < mTraceDepth)
		{
			if (primMat->isDiffuseRefl() && aDepth < 2)
			{
				Real drefl = primMat->getDiffuseRefl();
				Vec3 tRN1(reflDir.z, reflDir.y, -reflDir.x);
				Vec3 tRN2 = reflDir.Cross(tRN1);
				Real xoffs, yoffs;
				Vec3 refl = primMat->getReflection() * mSampleScale2 * color;
				for (int i=0; i<mRegularSampleSize*mRegularSampleSize; ++i)
				{
					do 
					{
						xoffs = (mTwister->Rand() - 0.5f) * 0.8f;
						yoffs = (mTwister->Rand() - 0.5f) * 0.8f;
					} while (xoffs * xoffs + yoffs * yoffs > 1.0f);
					Vec3 tReflDir = reflDir + tRN1 * xoffs * drefl + tRN2 * yoffs * drefl;
					tReflDir.Normalize();
					Color rcol(0,0,0);
					Real dist = 0;
					if (rayTrace(Ray(pi + (tReflDir * RT_EPSILON), tReflDir, ++mCurRayID),
						rcol, dist, aDepth+1, aRIndex) != 0)
						aAccClr += refl * rcol;
				}
			}
			else
			{
				Color rcol(0,0,0);
				Real dist = 0;
				if (rayTrace(Ray(pi + (reflDir * RT_EPSILON), reflDir, ++mCurRayID),
					rcol, dist, aDepth+1, aRIndex) != 0)
					aAccClr += rcol * primMat->getReflection();
			}
		}

		// 5. calculate refraction
		if (primMat->isRefraction() && aDepth < mTraceDepth)
		{
			Real rindex = primMat->getRefrIndex();
			Real n = aRIndex / rindex; //  折射率
			if (result == INPRIM)
				normDir *= -1.0f; // 根据是否在物体内调整法向
			Real cosI = -normDir.Dot(viewDir); // 入射角余弦
			Real cosT2 = 1.0f - n * n * (1.0f - cosI * cosI); // 出射角余弦平方
			if (cosT2 > 0.0f)
			{
				transDir = (n * viewDir) + (n * cosI - sqrtf(cosT2)) * normDir;
				Color rcol(0,0,0);
				Real dist = 0;
				rayTrace(Ray(pi + transDir * RT_EPSILON, transDir, ++mCurRayID), rcol, dist, aDepth+1, rindex);
				// apply Beer's law
				if (n < 1.0f)
				{ // 只有当在光线从低折射率介质进入高折射率介质时才计算，避免重复计算
					Real absorbance = primMat->getRefraction() * 0.15f * -aDist;
					aAccClr += rcol * expf(absorbance);
				}
				else
					aAccClr += rcol;
			}
		}
	}// end if it is not a lightPrim
	return prim;
}

void Engine::initEngine(const Vec3& aEyePos, const Vec3& aTarget)
{
	if (!mCreated)
		return;

	// set first line to draw to
	mCurrLine = 0;

	// update camera
	mCamera->lookAt(aEyePos, aTarget, Vec3::UNIT_Y);
	mCamera->frustum(-mRatio, mRatio, -1, 1, 1);
	//mCamera->perspective(RT_PI * 0.25f, mWidth * 1.0f / mHeight, );

	// calculate data for regular grid stepping
	mRCS = RT_GRIDSIZE / mScene->getExtends().getDim();
	mCS = mScene->getExtends().getDim() / RT_GRIDSIZE;

	// reset ray id counter
	mCurRayID = 0;

	// calculate deltas for interpolation
	mDx = 1.0f / mWidth;
	mDy = 1.0f / mHeight;
	mSx = 0.0f;
	mSy = mCurrLine * mDy;

	// regular sampling
	mSampleScale = 1.0f / mRegularSampleSize;
	mSampleOffset = 0.5f  * mSampleScale;
	mSampleScale2 = mSampleScale * mSampleScale;

	// last line primitives recorder
	mLastLinePrims.resize(mWidth, NULL);
}

Primitive* Engine::renderRay(Real x, Real y, Color& aAccClr)
{
	static Box extends(mScene->getExtends());
	
	Vec3 camPos = mCamera->pos();
	Vec3 screenPos = mCamera->getScreenPos(x, y);
	Vec3 dir = screenPos - camPos;
	dir.Normalize();
	Ray ray(camPos, dir, ++mCurRayID);

	// advance ray to scene bounding box boundary
	if (!extends.contains(camPos))
	{
		Real bdist = 10000.0f;
		if (extends.intersect(ray, bdist))
			ray.setOrigin(camPos + (bdist + RT_EPSILON) * dir);
	}

	Real dist;
	return rayTrace(ray, aAccClr, dist, 1, 1.0f);
}

bool Engine::render()
{
	if (!mCreated)
		return true;

	clock_t tt = clock();

	Primitive *lastPrim = 0, *currPrim;
	Ray ray;
	Real aaScale = 1.0f / 4.0f;

	// render remaining lines
	for (int y=mCurrLine; y<mHeight; ++y)
	{
		mSx = 0.0f;
		// render pixels for current line
		for (int x=0; x<mWidth; ++x)
		{
			// fire primary ray
			Color finalClr(0,0,0);
			currPrim = renderRay(mSx, mSy, finalClr);
			// upsampling TOP LEFT 2 x 2
			if (currPrim != lastPrim || 
				mLastLinePrims[x] != currPrim ||
				finalClr.Length() < RT_EPSILON) // NOTE: 防止在这个平面相交处的遮挡误判断
			{
				lastPrim = currPrim;
				mLastLinePrims[x] = currPrim;
				// left
				currPrim = renderRay(mSx - 0.5f*mDx, mSy, finalClr);
				// top left
				currPrim = renderRay(mSx - 0.5f*mDx, mSy + 0.5f*mDy, finalClr);
				// top
				currPrim = renderRay(mSx, mSy - 0.5f*mDy, finalClr);

				finalClr *= aaScale;
			}
			_setFrameBuffer(y, x, finalClr);
			mSx += mDx;
		}
		mSy += mDy;

		// see if we've been working too long already
		if (clock() - tt > MAX_RENDER_TIME)
		{
			mCurrLine = y+1;
			if (mCurrLine != mHeight)
			{
				for (int x=0; x<mWidth; ++x)
					_setFrameBuffer(mCurrLine, x, Color(1,1,1));
			}
			return false;
		}
	}
	// all done
	return true;
}

void Engine::_setFrameBuffer(int _y, int _x, const Color& _clr)
{
	Color color = _clr * 255.0f;
	mImage->setPixel(_x, _y,
		qRgb(SATURATE(color.r), SATURATE(color.g), SATURATE(color.b)) );
}

int Engine::getNumOfPrimitives() const
{
	return mScene->getNumOfPrimitives();
}

void Engine::loadObjModel(const trimeshVec::CAccessObj* accessObj)
{
	mScene->loadObjModel(accessObj);
}

}; // namespace RayTracer