#pragma once
/*
 * MathUtils.h
 */
#include "Vec3f.h"
#include "Curve.h"

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
 * arcLengthStep()
 * Calculates an arc-length parameterized step for the specified curve
 * based on the specified step 't' and velocity
 *
 * Right now this is only used in CurveSegments for drawing rail ties
 *
 * TODO: this is duplicated code, also found in MainWindow
 * had to leave the MainWindow version for now to prevent
 * segment lookup errors when using this method to get
 * debug output for MainView::updateDebugText()
 */
inline float arcLengthStep(const Curve& curve, const float t, const float vel=1.f)
{
	float next_t = t + 0.1f;
	if( next_t >= curve.numSegments() )
		next_t -= curve.numSegments();

	const Vec3f thisPoint(curve.getPosition(t));
	const Vec3f nextPoint(curve.getPosition(next_t));

	const Vec3f distance(nextPoint - thisPoint);

	return (vel / distance.magnitude());
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

	normal = normalize(cross(binormal, tangent));
}
