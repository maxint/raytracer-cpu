// -----------------------------------------------------------
// common.h
// 2004 - Jacco Bikker - jacco@bik5.com - www.bik5.com -   <><
// -----------------------------------------------------------

#ifndef _RT_MATH_DEFS_H_
#define _RT_MATH_DEFS_H_

#include <cmath>
#include <algorithm>

namespace RayTracer {

#ifndef RT_PI
#define RT_PI				3.141592653589793239462f
#endif

#ifdef HIGH_PRECISION
#define RT_EPSILON			1E-6
#else
#define RT_EPSILON			0.0001f
#endif

// ------------------------------------------------------------------------------
// Basic vector class
// ------------------------------------------------------------------------------

template <typename _Tp>
class Vector3
{
public:
	Vector3() : x( 0.0f ), y( 0.0f ), z( 0.0f ) {};
	Vector3( _Tp a_X, _Tp a_Y, _Tp a_Z ) : x( a_X ), y( a_Y ), z( a_Z ) {};
	void Set( _Tp a_X, _Tp a_Y, _Tp a_Z ) { x = a_X; y = a_Y; z = a_Z; }
	void Normalize() { _Tp l = 1.0f / Length(); x *= l; y *= l; z *= l; }
	_Tp Length() const { return (_Tp)sqrt( x * x + y * y + z * z ); }
	_Tp SqrLength() const { return x * x + y * y + z * z; }
	_Tp Dot(const Vector3& a_V ) const { return x * a_V.x + y * a_V.y + z * a_V.z; }
	Vector3 Cross(const Vector3& b ) const 
		{ return Vector3( y * b.z - z * b.y, z * b.x - x * b.z, x * b.y - y * b.x ); }

	// Accessor
	const _Tp operator[] (int i) const	{ return cell[i]; }
	_Tp& operator[] (int i)				{ return cell[i]; }

	void operator += ( const Vector3& a_V ) { x += a_V.x; y += a_V.y; z += a_V.z; }
	void operator += ( _Tp f ) { for (int i=0; i<3; ++i) cell[i] += f; }
	void operator -= ( const Vector3& a_V ) { x -= a_V.x; y -= a_V.y; z -= a_V.z; }
	void operator -= ( _Tp f ) { for (int i=0; i<3; ++i) cell[i] -= f; }
	void operator *= ( const Vector3& a_V ) { x *= a_V.x; y *= a_V.y; z *= a_V.z; }
	void operator *= ( _Tp f ) { for (int i=0; i<3; ++i) cell[i] *= f; }
	void operator /= ( const Vector3& a_V ) { x /= a_V.x; y /= a_V.y; z /= a_V.z; }
	void operator /= ( _Tp f ) { for (int i=0; i<3; ++i) cell[i] /= f; }
	Vector3 operator- () const { return Vector3( -x, -y, -z ); }

	/// Operations between vectors
	friend Vector3 operator + ( const Vector3& v1, const Vector3& v2 )
		{ return Vector3( v1.x + v2.x, v1.y + v2.y, v1.z + v2.z ); }
	friend Vector3 operator - ( const Vector3& v1, const Vector3& v2 ) 
		{ return Vector3( v1.x - v2.x, v1.y - v2.y, v1.z - v2.z ); }
	friend Vector3 operator * ( const Vector3& v1, const Vector3& v2 )
		{ return Vector3( v1.x * v2.x, v1.y * v2.y, v1.z * v2.z ); }
	friend Vector3 operator / ( const Vector3& v1, const Vector3& v2 )
		{ return Vector3( v1.x / v2.x, v1.y / v2.y, v1.z / v2.z ); }

	/// Operations between vector and scale
	friend Vector3 operator + ( const Vector3& v, _Tp f)
		{ return Vector3( v.x + f, v.y + f, v.z + f ); }
	friend Vector3 operator + ( _Tp f, const Vector3& v )
		{ return Vector3( v.x + f, v.y + f, v.z + f ); }

	friend Vector3 operator - ( const Vector3& v, _Tp f)
		{ return Vector3( v.x - f, v.y - f, v.z - f ); }
	friend Vector3 operator - ( _Tp f, const Vector3& v )
		{ return Vector3( f - v.x, f - v.y, f - v.z ); }

	friend Vector3 operator * ( const Vector3& v, _Tp f)
		{ return Vector3( v.x * f, v.y * f, v.z * f ); }
	friend Vector3 operator * ( _Tp f, const Vector3& v )
		{ return Vector3( v.x * f, v.y * f, v.z * f ); }

	friend Vector3 operator / ( _Tp f, const Vector3& v )
		{ return Vector3( f / v.x, f / v.y, f / v.z ); }
	friend Vector3 operator / ( const Vector3& v, _Tp f)
		{ return Vector3( v.x / f, v.y / f, v.z / f ); }

	/// Other operations
	const Vector3& Max(const Vector3& b);
	const Vector3& Min(const Vector3& b);
	const Vector3& Abs();

	friend Vector3 Min(const Vector3& a, const Vector3& b)
		{ return Vector3(a).Min(b); }
	friend Vector3 Max(const Vector3& a, const Vector3& b)
		{ return Vector3(a).Max(b); }

	friend bool operator < (const Vector3& a, const Vector3& b)
		{ return (a.cell[0] < b.cell[0]) && (a.cell[1] < b.cell[1]) && (a.cell[2] < b.cell[2]); }
	friend bool operator > (const Vector3& a, const Vector3& b)
		{ return b < a; }
	bool operator < (_Tp f) const
		{ return (cell[0] < f &&  cell[1] < f && cell[2] < f); }
	bool operator > (_Tp f) const
		{ return (cell[0] > f &&  cell[1] > f && cell[2] > f); }

	union
	{
		struct { _Tp x, y, z; };
		struct { _Tp r, g, b; };
		struct { _Tp cell[3]; };
	};
	static const Vector3 ZERO;
	static const Vector3 ONE;
	static const Vector3 UNIT_X;
	static const Vector3 UNIT_Y;
	static const Vector3 UNIT_Z;
};

// ------------------------------------------------------------------------------
// 4x4 Matrix_ class
// ------------------------------------------------------------------------------

template <typename _Tp>
class Matrix_
{
public:
	_Tp cell[16];
	enum 
	{ 
		TX=3, 
		TY=7, 
		TZ=11, 
		D0=0, D1=5, D2=10, D3=15, 
		SX=D0, SY=D1, SZ=D2, 
		W=D3 
	};
	Matrix_() { Identity(); }
	void ToZero()
	{
		memset(cell, 0, sizeof(_Tp)*16);
	}
	void Identity()
	{
		ToZero();
		cell[D0] = cell[D1] = cell[D2] = cell[W] = 1;
	}
	void Rotate( Vector3<_Tp> a_Pos, _Tp a_RX, _Tp a_RY, _Tp a_RZ )
	{
		Matrix_ t;
		t.RotateX( a_RZ );
		RotateY( a_RY );
		Concatenate( t );
		t.RotateZ( a_RX );
		Concatenate( t );
		Translate( a_Pos );
	}
	void RotateX( _Tp a_RX )
	{
		_Tp sx = (_Tp)sin( a_RX * RT_PI / 180 );
		_Tp cx = (_Tp)cos( a_RX * RT_PI / 180 );
		Identity();
		cell[5] = cx, cell[6] = sx, cell[9] = -sx, cell[10] = cx;
	}
	void RotateY( _Tp a_RY )
	{
		_Tp sy = (_Tp)sin( a_RY * RT_PI / 180 );
		_Tp cy = (_Tp)cos( a_RY * RT_PI / 180 );
		Identity ();
		cell[0] = cy, cell[2] = -sy, cell[8] = sy, cell[10] = cy;
	}
	void RotateZ( _Tp a_RZ )
	{
		_Tp sz = (_Tp)sin( a_RZ * RT_PI / 180 );
		_Tp cz = (_Tp)cos( a_RZ * RT_PI / 180 );
		Identity ();
		cell[0] = cz, cell[1] = sz, cell[4] = -sz, cell[5] = cz;
	}
	void Translate( Vector3<_Tp> a_Pos ) { cell[TX] += a_Pos.x; cell[TY] += a_Pos.y; cell[TZ] += a_Pos.z; }
	void Concatenate( Matrix_& m2 )
	{
		Matrix_ res;
		int c;
		for ( c = 0; c < 4; c++ ) for ( int r = 0; r < 4; r++ )
			res.cell[r * 4 + c] = cell[r * 4] * m2.cell[c] +
			cell[r * 4 + 1] * m2.cell[c + 4] +
			cell[r * 4 + 2] * m2.cell[c + 8] +
			cell[r * 4 + 3] * m2.cell[c + 12];
		memcpy(cell, res.cell, sizeof(_Tp)*16);
	}
	Vector3<_Tp> Transformed( const Vector3<_Tp>& v ) const
	{
		_Tp x  = cell[0] * v.x + cell[1] * v.y + cell[2] * v.z + cell[3];
		_Tp y  = cell[4] * v.x + cell[5] * v.y + cell[6] * v.z + cell[7];
		_Tp z  = cell[8] * v.x + cell[9] * v.y + cell[10] * v.z + cell[11];
		return Vector3( x, y, z );
	}
	void Transform( Vector3<_Tp>& v ) const
	{
		v.x  = cell[0] * v.x + cell[1] * v.y + cell[2] * v.z + cell[3];
		v.y  = cell[4] * v.x + cell[5] * v.y + cell[6] * v.z + cell[7];
		v.z  = cell[8] * v.x + cell[9] * v.y + cell[10] * v.z + cell[11];
	}
	void Invert()
	{
		Matrix_ t;
		_Tp tx = -cell[3], ty = -cell[7], tz = -cell[11];
		for ( int h = 0; h < 3; h++ ) for ( int v = 0; v < 3; v++ ) t.cell[h + v * 4] = cell[v + h * 4];
		for ( int i = 0; i < 11; i++ ) cell[i] = t.cell[i];
		cell[3] = tx * cell[0] + ty * cell[1] + tz * cell[2];
		cell[7] = tx * cell[4] + ty * cell[5] + tz * cell[6];
		cell[11] = tx * cell[8] + ty * cell[9] + tz * cell[10];
	}
};


// ------------------------------------------------------------------------------
// A simple plane definition in math
// ------------------------------------------------------------------------------

template <typename _Tp>
class Plane_
{
public:
	union
	{
		struct
		{
			Vector3<_Tp> N;
			_Tp D;
		};
		_Tp cell[4];
	};
	Plane_()
		: N(0,0,0)
		, D(0)
	{}
	Plane_(const Vector3<_Tp>& aNormal, _Tp aDist)
		: N(aNormal)
		, D(aDist)
	{}
};

// ------------------------------------------------------------------------------
// Axis Aligned Bounding Box
// ------------------------------------------------------------------------------

template <typename _Tp>
class AABB_
{
public:
	AABB_()
		: mMin(0,0,0)
		, mMax(0,0,0)
	{}
	AABB_(const Vector3<_Tp>& aMin, const Vector3<_Tp>& aMax)
		: mMin(aMin)
		, mMax(aMax)
	{}
	const Vector3<_Tp>& getMin() const		{ return mMin; }
	void setMin(const Vector3<_Tp>& val)	{ mMin = val; }
	const Vector3<_Tp>& getMax() const		{ return mMax; }
	void setMax(const Vector3<_Tp>& val)	{ mMax = val; }
	const Vector3<_Tp> getDim() const		{ return mMax - mMin; }
	
	/**	Detect intersection of two AABBs
	\param
		aB2		the other AABB_
	\return
		true	intersected
		false	not intersected
	 */
	bool interset(const AABB_& aB2) const;

	/**	Whether a point is inside the AABB_
	\param 
		aPos	the point to be detected
	 */
	bool contains(const Vector3<_Tp>& aPos) const ;

private:
	Vector3<_Tp> mMin, mMax;
};

}; // namespace RayTracer

#include "MathDefs.inl"

#endif // _RT_MATH_DEFS_H_