/********************************************************************
	created:	2010/04/22
	file name:	common.h
	author:		maxint lnychina@gmail.com
*********************************************************************/

#ifndef _RT_COMMON_H_
#define _RT_COMMON_H_

#include <string>
#include <iostream>

#define HIGH_PRECISION

#include "MathDefs.h"

// ------------------------------------------------------------------------------
// Useful Macros
// ------------------------------------------------------------------------------

#ifndef RT_PI
#define RT_PI				3.141592653589793239462f
#endif

#ifndef RT_CLAMP
#define RT_CLAMP(x,x1,x2) std::max(Real(x1), std::min(Real(x2), Real(x)))
#endif

#define RT_TRACEDEPTH		6
#define RT_GRIDSIZE			32
#define RT_GRIDSHIFT		5

#ifndef SAFE_DELETE
#define SAFE_DELETE(p) if(p) { delete (p); (p)=0; }
#endif

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) if(p) { delete[] (p); (p)=0; }
#endif

namespace RayTracer {

#ifdef HIGH_PRECISION
	typedef double Real;
#else
	typedef float Real;
#endif

inline Real rtRand( Real a_Range ) { return ((Real)rand() / RAND_MAX) * a_Range; }

/// Intersection method return values
enum RTResult
{
	INPRIM	= -1,	// Ray started inside primitive
	MISS	= 0,	// Ray missed primitive
	HIT		= 1		// Ray hit primitive
};

// ------------------------------------------------------------------------------
// Useful type definitions
// ------------------------------------------------------------------------------

typedef Vector3<Real>	Color;
typedef Vector3<Real>	Vec3;
typedef Plane_<Real>	Plane;
typedef AABB_<Real>		AABB;
typedef Matrix_<Real>	Matrix;
typedef std::string		String;

}; // namespace RayTracer

#endif // _RT_COMMON_H_