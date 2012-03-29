#pragma once
/*
 * MathUtils.h
 */
#include "Vec3f.h"

#include <cmath>


static const float PI      = 3.141593f;
static const float TWO_PI  = 6.283185f;
static const float HALF_PI = 1.570796f;
static const float E       = 2.718282f;


inline float radToDeg(const float rad)
{
	static const float factor = 180.f / PI;
	return rad * factor;
}

inline float degToRad(const float deg)
{
	static const float factor = PI / 180.f;
	return deg * factor;
}

/*
 * generateBasis() 
 * Takes a tangent vector and calculates orthonormal 'up' and 'right' vectors 
 * @normal - the 'up' vector (overwritten)
 * @binormal - the 'right' vector (overwritten)
 */
inline void generateBasis(const Vec3f& tangent, Vec3f& normal, Vec3f& binormal)
{
	static const Vec3f worldUp(0.f, 1.f, 0.f);
	static const Vec3f worldRight(1.f, 0.f, 0.f);

	binormal = normalize(cross(tangent,worldUp));

	// Protect from the case where tangent == worldup 
	if( binormal == Vec3f(0.f, 0.f, 0.f) )
		binormal = normalize(cross(tangent, worldRight));

	normal = normalize(cross(tangent, binormal));
}