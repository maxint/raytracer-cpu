/********************************************************************
	created:	2010/04/22
	file name:	primitive.cpp
	author:		maxint lnychina@gmail.com
*********************************************************************/

#include "primitive.h"
#include "material.h"
#include "raytracer.h"

namespace RayTracer {

using std::min;
using std::max;
using std::abs;

static const int MODULO3[] = { 0, 1, 2, 0, 1 };

// ------------------------------------------------------------------------------
// Primitive class implementation
// ------------------------------------------------------------------------------

Primitive::Primitive()
: mName("")
, mIsLight(false)
, mAABB(Vec3::ZERO, Vec3::ZERO)
, mRayID(-1)
, mMaterial(NULL)
{
	mMaterial = MaterialManager::getInstance().getMaterial("_default_");
}


Primitive::~Primitive() 
{ 
}

void Primitive::setMaterial(const String& matName)
{
	mMaterial = MaterialManager::getInstance().getMaterial(matName);
}

Color Primitive::getColor(const Vec3& aIP) const
{
	if (!mMaterial->isTexture())
	{
		return mMaterial->getDiffuse();
	}
	else
	{
		Real u, v;
		getTextureCoord(u, v, aIP);
		u *= mMaterial->getUScale();
		v *= mMaterial->getVScale();
		return mMaterial->getTexture()->getTexel(u, v) * mMaterial->getDiffuse();
	}
}

// ------------------------------------------------------------------------------
// Sphere primitive methods
// ------------------------------------------------------------------------------

RTResult Sphere::intersect(const Ray& aRay, Real& aDist)
{
	mRayID = aRay.getID();

	Vec3 v = mCentre - aRay.getOrigin();
	Real b = v.Dot(aRay.getDir());
	Real a2 = v.SqrLength() - (b * b); // 直线与球心的距离
	Real det = mSqrRadius - a2; // 内截径
	RTResult retval = MISS;
	if (det > 0)
	{ // 相交
		det = std::sqrt(det);
		Real i1 = b - det; /*  near intersection
								positive if outside sphere
								negative if inside sphere
								*/
		Real i2 = b + det; /*	far intersection
								positive if ray is toward or inside sphere
								negative if ray is backward sphere
								*/
		if (i2 > 0)
		{
			if (i1 < 0)
			{ // inner
				if (i2 < aDist)
				{ // 小于预设值
					aDist = i2;
					retval = INPRIM;
				}
			}
			else
			{
				if (i1 < aDist)
				{
					aDist = i1;
					retval = HIT;
				}
			}
		}
	}
	return retval;
}

bool Sphere::intersetBox(const AABB& aBox) const
{
	const Vec3 &v1 = aBox.getMin(), &v2 = aBox.getMax();
	Real dmin = 0;

	for (int i=0; i<3; ++i)
	{
		if (mCentre[i] < v1[i])
		{
			dmin += (mCentre[i] - v1[i]) * (mCentre[i] - v1[i]);
		}
		else if (mCentre[i] > v2[i])
		{
			dmin += (mCentre[i] - v2[i]) * (mCentre[i] - v2[i]);
		}
	}
	return (dmin <= mSqrRadius);
}

void Sphere::getTextureCoord(Real& u, Real& v, const Vec3& aIP) const
{
	Vec3 vp = (aIP - mCentre) * mRRadius;
	Real phi = acos( static_cast<Real>(RT_CLAMP(vp.Dot(Vec3::UNIT_Y), -1.0f, 1.0f)) );
	v = phi * (1.0f / RT_PI);
	Real theta = acos( static_cast<Real>(RT_CLAMP(vp.Dot(Vec3::UNIT_X) / sin(phi), -1.0f, 1.0f)) ) * 0.5f / RT_PI;
	if (vp.Dot(Vec3::UNIT_Z) >= 0)
		u = (1.0f - theta);
	else 
		u = theta;
}

// ------------------------------------------------------------------------------
// Plane primitive class implementation
// ------------------------------------------------------------------------------

PlanePrim::PlanePrim(const Vec3& aNormal, Real aDist)
: mPlane(aNormal, aDist)
, mUAxis(aNormal.y, aNormal.z, -aNormal.x)
{
	mAABB = AABB(-10000 * Vec3::ONE, 10000 * Vec3::ONE);
	mVAxis = mUAxis.Cross(aNormal);
}

RTResult PlanePrim::intersect(const Ray& aRay, Real& aDist)
{
	mRayID = aRay.getID();

	Real d = mPlane.N.Dot(aRay.getDir()); // negative
	if (d < 0)
	{ // 不平行且符合朝向
		Real dist = -( mPlane.N.Dot(aRay.getOrigin()) + mPlane.D) / d;
		if (dist > 0 && dist < aDist)
		{
			aDist = dist;
			return HIT;
		}
	}
	return MISS;
}

bool PlanePrim::intersetBox(const AABB& aBox) const
{
	int side1 = 0, side2 = 0;
	const Vec3 *v[2];
	v[0] = &(aBox.getMin()); v[1] = &(aBox.getMax());
	// 如果8个顶点都在平面同一侧则不相交，否则相交
	for (int i=0; i<8; ++i)
	{
		if (Vec3(v[i&1]->x, v[(i>>1)&1]->y, v[(i>>2)&1]->z).Dot(mPlane.N) + mPlane.D < 0)
			++side1;
		else
			++side2;
	}
	return !(side1 == 0 || side2 == 0);
}

void PlanePrim::getTextureCoord(Real &u, Real &v, const RayTracer::Vec3 &aIP) const
{
	u = aIP.Dot(mUAxis);
	v = aIP.Dot(mVAxis);
}

// ------------------------------------------------------------------------------
// Axis aligned box class implementation
// ------------------------------------------------------------------------------

RTResult Box::intersect(const Ray& aRay, Real& aDist)
{
	mRayID = aRay.getID();

	Real dist[6]; // 到各平面的距离
	RTResult retval = MISS;
	int i;
	for (i = 0; i < 6; ++i ) dist[i] = -1;
	Vec3 v1 = mAABB.getMin(), v2 = mAABB.getMax();
	Vec3 d = aRay.getDir(), o = aRay.getOrigin();
	if (d.x) 
	{
		Real rc = 1.0f / d.x;
		dist[0] = (v1.x - o.x) * rc;
		dist[1] = (v2.x - o.x) * rc;
	}
	if (d.y) 
	{
		Real rc = 1.0f / d.y;
		dist[2] = (v1.y - o.y) * rc;
		dist[3] = (v2.y - o.y) * rc;
	}
	if (d.z) 
	{
		Real rc = 1.0f / d.z;
		dist[4] = (v1.z - o.z) * rc;
		dist[5] = (v2.z - o.z) * rc;
	}
	Vec3 ip;
	for (i = 0; i < 6; i++ ) if (dist[i] > 0)
	{
		ip = o + dist[i] * d; // 在各平面上的交点
		if (dist[i] < aDist &&
			ip > (v1 - RT_EPSILON) &&
			ip < (v2 + RT_EPSILON)
			)
		{
			aDist = dist[i];
			retval = HIT;
		}
	}
	if (retval == HIT && mAABB.contains(o))
	{
		retval = INPRIM;
	}
	return retval;
}

const Vec3& Box::getNormal(const Vec3& aPos)
{
	Real tDist[6];
	tDist[0] = abs(aPos.x - mAABB.getMin().x);
	tDist[1] = abs(aPos.y - mAABB.getMin().y);
	tDist[2] = abs(aPos.z - mAABB.getMin().z);
	tDist[3] = abs(aPos.x - mAABB.getMax().x);
	tDist[4] = abs(aPos.y - mAABB.getMax().y);
	tDist[5] = abs(aPos.z - mAABB.getMax().z);

	int best = 0, i;
	Real bdist = tDist[0];
	for (i=1; i<6; ++i) if (tDist[i] < bdist)
	{
		bdist = tDist[i];
		best = i;
	}
	mNormal = Vec3::ZERO;
	if (best == 0) mNormal.x = -1;
	if (best == 1) mNormal.y = -1;
	if (best == 2) mNormal.z = -1;
	if (best == 3) mNormal.x = 1;
	if (best == 4) mNormal.y = 1;
	if (best == 5) mNormal.z = 1;

	return mNormal;
}

// ------------------------------------------------------------------------------
// Triangle primitive class implementation
// ------------------------------------------------------------------------------

TrianglePrim::TrianglePrim(Vertex* v1, Vertex* v2, Vertex* v3, bool genNorm)
{
	mVertices[0] = v1;
	mVertices[1] = v2;
	mVertices[2] = v3;

	// precalculate
	Vec3 b = v2->mPos - v1->mPos;
	Vec3 c = v3->mPos - v1->mPos;
	mN = b.Cross(c);
	mN.Normalize();
	Vec3 Nabs = mN;
	Nabs.Abs();
	if (Nabs.x > Nabs.y && Nabs.x > Nabs.z)
		mMajorAxis = 0;
	else if (Nabs.y > Nabs.x && Nabs.y > Nabs.z)
		mMajorAxis = 1;
	else
		mMajorAxis = 2;
	
	int u = MODULO3[mMajorAxis + 1];
	int v = MODULO3[mMajorAxis + 2];
	Real krec = 1.0f / (b.cell[u] * c.cell[v] - b.cell[v] * c.cell[u]);
	mBx = c.cell[v] * krec;
	mBy = -c.cell[u] * krec;
	mCx = -b.cell[v] * krec;
	mCy = b.cell[u] * krec;

	// AABB
	Vec3 vMin(v1->mPos), vMax(v1->mPos);
	vMin.Min(v2->mPos);
	vMin.Min(v3->mPos);
	vMax.Max(v2->mPos);
	vMax.Max(v3->mPos);
	mAABB.setMin(vMin);
	mAABB.setMax(vMax);

	// set normals
	if (genNorm)
	{
		v1->mNormal = mN;
		v2->mNormal = mN;
		v3->mNormal = mN;
	}
}

RTResult TrianglePrim::intersect(const Ray& aRay, Real& aDist)
{
	mRayID = aRay.getID();

	const Vec3& O = aRay.getOrigin();
	const Vec3& D = aRay.getDir();
	Real dist = mN.Dot(D);
	if (dist < 0)
	{ // 不平行且符合朝向
		dist = mN.Dot(mVertices[0]->mPos-O) / dist;
		if (!(dist > 0 && dist < aDist))
			return MISS;
		
		// 质心坐标判断交点是否在三角形中
		Vec3 hit = O + D * dist - mVertices[0]->mPos;
		int u = MODULO3[mMajorAxis + 1];
		int v = MODULO3[mMajorAxis + 2];
		mBaryCoord[1] = hit.cell[u] * mBx + hit.cell[v] * mBy;
		if (mBaryCoord[1] < 0) return MISS;
		mBaryCoord[2] = hit.cell[u] * mCx + hit.cell[v] * mCy;
		if (mBaryCoord[2] < 0) return MISS;
		mBaryCoord[0] = 1 - mBaryCoord[1] -mBaryCoord[2];
		if (mBaryCoord[0] < 0) return MISS;
		//Vec3 test = Vec3::ZERO;
		//for (int i=0; i<3; ++i)
		//{
		//	test += mBaryCoord[i] * mVertices[i]->mPos;
		//}
		//test -= mVertices[0]->mPos;
		aDist = dist;
		return HIT;
	}
	return MISS;
}

bool TrianglePrim::intersetBox(const AABB &aBox) const
{
	Vec3 halfDim = 0.5f * aBox.getDim();
	Vec3 centre = aBox.getMin() + halfDim;
	Vec3 v[3], e[3], sv;
	int i, j;
	int side;
	for (i=0; i<3; ++i)
	{
		v[i] = mVertices[i]->mPos - centre;
		e[i] = mVertices[MODULO3[i]]->mPos - mVertices[MODULO3[i+1]]->mPos;
		e[i].Normalize();

	}
	// 基于分离轴定理的三角形与AABB求交算法，共测试3+1+9＝13次
	// AABB 的主轴
	// AABB 的3个主轴
	sv = Vec3::ZERO;
	for (i=0; i<3; ++i)
	{ // each axis
		side = 0;
		for (j=0; j<3; ++j)
		{ // each vertex
			if (v[j].cell[i] > halfDim[i])
				++side;
			else if (v[j].cell[i] < -halfDim[i])
				--side;
			else
				++sv[i];
		}
		if (side == -3 || side == 3) return false;
	}
	for (i=0; i<3; ++i) if (sv[i] == 3)
		return true;

	// 三角形法向
	Vec3 norm = e[0].Cross(e[1]);
	Real d2tri = abs(norm.Dot(v[0]));
	Real d2r = 0;
	for (i=0; i<3; ++i)
	{
		d2r += ( (norm.cell[i]<0) ? -1 : 1 ) * norm.cell[i] * halfDim.cell[i];
	}
	if (d2tri > d2r) return false;
	
	// 9种法向
	Real d2tri2;
	for (i=0; i<3; ++i)
	{// each axis
		for (j=0; j<3; ++j)
		{// each edge
			norm = Vec3::ZERO;
			norm[i] = 1;
			if (abs(norm.Dot(e[j])) > 1.0 - RT_EPSILON)
			{
				continue;
			}
			norm = norm.Cross(e[j]);
			d2tri = abs(norm.Dot(v[i]));
			d2tri2 = abs(norm.Dot(v[MODULO3[i+2]]));
			d2tri = min(d2tri, d2tri2);
			Real d2r = 0;
			for (i=0; i<0; ++i)
				d2r += ( (norm.cell[i]<0) ? -1 : 1 ) * norm.cell[i] * halfDim.cell[i];
			if (d2tri > d2r) return false;
		}
	}

	return true;
}

const Vec3& TrianglePrim::getNormal(const Vec3 &)
{
	mNormal = Vec3::ZERO;
	for (int i=0; i<3; ++i)
	{
		mNormal += mBaryCoord[i] * mVertices[i]->mNormal;
	}
	mNormal.Normalize();
	return mNormal;
}

void TrianglePrim::getTextureCoord(Real &u, Real &v, const Vec3&) const
{
	u = v = 0;
	for (int i=0; i<3; ++i)
	{
		u += mBaryCoord[i] * mVertices[i]->mU;
		v += mBaryCoord[i] * mVertices[i]->mV;
	}
}

// ------------------------------------------------------------------------------
// Light class implementation
// ------------------------------------------------------------------------------


}; // namespace RayTracer