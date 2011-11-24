#include "Camera.h"
#include <cassert>

namespace RayTracer
{

CCamera::CCamera(void)
: mNeedUpdate(true)
, mCameraPos(0, 0, 1)
{
}

CCamera::~CCamera(void)
{
}

//------------------------------------------------------------------------------
// Camera Related
//------------------------------------------------------------------------------
void CCamera::lookAt(const Vec3& eye, const Vec3& at, const Vec3& up)
{
	mCameraPos = eye;

	// calculate camera matrix
	Vec3 zAxis = at - eye;
	zAxis.Normalize();

	// if up is near @t_dir, calculation below may be fail, adjust it
	Vec3 tUp(up);
	tUp.Normalize();
	if ( tUp.Dot(zAxis) > 0.99f )
	{
		tUp = Vec3::UNIT_Z;
	}
	Vec3 xAxis = tUp.Cross(zAxis);
	Vec3 yAxis = zAxis.Cross(xAxis);

	// update inverse view matrix
	// NOTE: 从相机空坐标到世界坐标
	mInvViewMatrix.Identity();
	mInvViewMatrix.cell[0] = xAxis.x;
	mInvViewMatrix.cell[4] = xAxis.y; 
	mInvViewMatrix.cell[8] = xAxis.z;
	mInvViewMatrix.cell[1] = yAxis.x; 
	mInvViewMatrix.cell[5] = yAxis.y; 
	mInvViewMatrix.cell[9] = yAxis.z;
	mInvViewMatrix.cell[2] = zAxis.x; 
	mInvViewMatrix.cell[6] = zAxis.y; 
	mInvViewMatrix.cell[10] = zAxis.z;
	mInvViewMatrix.cell[3] = eye.x;
	mInvViewMatrix.cell[7] = eye.y; 
	mInvViewMatrix.cell[11] = eye.z;
	
	mNeedUpdate = true;
}

void CCamera::perspective(Real fovy, Real aspect, Real zNear)
{
	assert(fovy>0 && aspect>0);
	Real top = tan(fovy/2)*zNear;
	Real right = top * aspect;
	frustum(-right, right, -top, top, zNear);
}

void CCamera::frustum(Real left, Real right, Real bottom, Real top, Real zNear)
{
	mP1 = Vec3(left, top, zNear);
	mP2 = Vec3(right, top, zNear);
	mP3 = Vec3(right, bottom, zNear);
	mP4 = Vec3(left, bottom, zNear);

	mNeedUpdate = true;
}

Vec3 CCamera::getScreenPos(Real x, Real y)
{
	if (mNeedUpdate)
	{
		mInvViewMatrix.Transform(mP1);
		mInvViewMatrix.Transform(mP2);
		mInvViewMatrix.Transform(mP3);
		mInvViewMatrix.Transform(mP4);
		// calculate screen plane interpolation vectors
		mDx = (mP2 - mP1);
		mDy = (mP4 - mP1);
		mNeedUpdate = false;
	}
	return (mP1 + x * mDx + y * mDy);
}

} // namespace RayTracer