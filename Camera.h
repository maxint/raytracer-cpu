#pragma once

#include "common.h"

namespace RayTracer
{

class CCamera
{
public:
	CCamera(void);
	~CCamera(void);
	
	/// Return camera position
	const Vec3& pos() { return mCameraPos; }

	/// Camera related
	void lookAt(const Vec3& eye, const Vec3& at, const Vec3& up);

	void perspective(Real fovy, Real aspect, Real zNear);
	void frustum(Real left, Real right, Real bottom, Real top, Real zNear);

	/**	Get screen position at screen position (x, y)
	\param
		x, y	0~1, relative position
	\return
		screen position in world coordinates.
	 */
	Vec3 getScreenPos(Real x, Real y);

private:
	Matrix mInvViewMatrix;

	Vec3 mCameraPos;
	Vec3 mP1, mP2, mP3, mP4, mDx, mDy;

	bool mNeedUpdate; // is it needed to update @mFinalMatrix
};

} // namespace RayTracer
