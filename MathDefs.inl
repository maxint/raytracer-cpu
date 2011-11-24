/********************************************************************
	created:	2010/04/22
	file name:	MathDefs.inl
	author:		maxint lnychina@gmail.com
*********************************************************************/

#ifndef _RT_MATHDEFS_INL_
#define _RT_MATHDEFS_INL_

namespace RayTracer {

// ------------------------------------------------------------------------------
// Vector3 class implementation
// ------------------------------------------------------------------------------

template <typename _Tp> const Vector3<_Tp> Vector3<_Tp>::ZERO		= Vector3<_Tp>(0,0,0);
template <typename _Tp> const Vector3<_Tp> Vector3<_Tp>::ONE		= Vector3<_Tp>(1,1,1);
template <typename _Tp> const Vector3<_Tp> Vector3<_Tp>::UNIT_X		= Vector3<_Tp>(1,0,0);
template <typename _Tp> const Vector3<_Tp> Vector3<_Tp>::UNIT_Y		= Vector3<_Tp>(0,1,0);
template <typename _Tp> const Vector3<_Tp> Vector3<_Tp>::UNIT_Z		= Vector3<_Tp>(0,0,1);

template <typename _Tp>
const Vector3<_Tp>& Vector3<_Tp>::Min(const Vector3<_Tp>& b)
{
	for (int i=0; i<3; ++i)
		cell[i] = std::min(cell[i], b.cell[i]);

	return *this;
}

template <typename _Tp>
const Vector3<_Tp>& Vector3<_Tp>::Max(const Vector3<_Tp>& b)
{
	for (int i=0; i<3; ++i)
		cell[i] = std::max(cell[i], b.cell[i]);
	
	return *this;
}

template <typename _Tp>
const Vector3<_Tp>& Vector3<_Tp>::Abs()
{
	for (int i=0; i<3; ++i)
		cell[i] = std::abs(cell[i]);
	
	return *this;
}

template <typename _Tp>
bool AABB_<_Tp>::interset(const AABB_<_Tp>& aB2) const
{
	return (
		(mMin.x < aB2.mMax.x) && (aB2.mMin.x < mMax.x) && // x-axis overlap
		(mMin.y < aB2.mMax.y) && (aB2.mMin.y < mMax.y) && // y-axis overlap
		(mMin.z < aB2.mMax.z) && (aB2.mMin.z < mMax.z) // z-axis overlap
		);
}

template <typename _Tp>
bool AABB_<_Tp>::contains(const Vector3<_Tp>& aPos) const
{
	return (
		(aPos.x > mMin.x - RT_EPSILON) && (aPos.x < mMax.x + RT_EPSILON) && 
		(aPos.y > mMin.y - RT_EPSILON) && (aPos.y < mMax.y + RT_EPSILON) && 
		(aPos.z > mMin.z - RT_EPSILON) && (aPos.z < mMax.z + RT_EPSILON) 
		);
}

}

#endif // _RT_MATHDEFS_INL_