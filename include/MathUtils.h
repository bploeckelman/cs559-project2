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
