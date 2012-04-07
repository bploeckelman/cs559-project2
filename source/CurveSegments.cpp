/*
 * CurveSegments.cpp
 */
#include "CurveSegments.h"
#include "CtrlPoint.h"
#include "MathUtils.h"
#include "Vec3f.h"
#include "Curve.h"

#include <Windows.h>
#define WIN32_LEAN_AND_MEAN

#include <GL/GL.h>

#include <stdexcept>

// Map CurveType enum value to string representation
std::string CurveTypeNames[] = {
	"Lines",
	"Catmull-Rom",
	"Hermite",
	"B-Spline"
};


/* ==================================================================
 * CurveSegment base class
 * ==================================================================
 */
const int   CurveSegment::numLines = 25;
const float CurveSegment::step     = 1.f / CurveSegment::numLines;
const float CurveSegment::radius   = 2.9f;

void CurveSegment::draw(bool drawPoints, bool isShadowed)
{
	float t = 0.f;
	glBegin(GL_LINE_STRIP);
		for(int i = 0; i <= numLines; ++i, t += step)
		{
			const Vec3f pos (getPosition(t));
			const Vec3f dir (normalize(getDirection(t)));
			const Vec3f up  (normalize(getOrientation(t)));
			const Vec3f side(normalize(cross(dir, up)));

			const Vec3f v(pos + radius * side);

			glVertex3fv(v.v());
		}
	glEnd();

	t = 0.f;
	glBegin(GL_LINE_STRIP);
		for(int i = 0; i <= numLines; ++i, t += step)
		{
			const Vec3f pos (getPosition(t));
			const Vec3f dir (normalize(getDirection(t)));
			const Vec3f up  (normalize(getOrientation(t)));
			const Vec3f side(normalize(cross(dir, up)));

			const Vec3f v(pos + -radius * side);
			// Note:     (pos -  radius * side) doesn't work as expected

			glVertex3fv(v.v());
		}
	glEnd();

	// Draw ties
	if( !isShadowed ) glColor4ub(139, 69, 19, 255); // brown
	glBegin(GL_LINES);
		for(float t = 0.f, arc_t = 0.f; arc_t <= 1.f; t += step)
		{
			const Vec3f pos (getPosition(arc_t));
			const Vec3f dir (normalize(getDirection(arc_t)));
			const Vec3f up  (normalize(getOrientation(arc_t)));
			const Vec3f side(normalize(cross(dir,up)));

			const Vec3f v1(pos +  radius * side);
			const Vec3f v2(pos + -radius * side);
			// Note:      (pos -  radius * side) doesn't work as expected

			glVertex3fv(v1.v());
			glVertex3fv(v2.v());

			arc_t += arcLengthStep(parentCurve, t);
		}
	glEnd();

	if( drawPoints )
	{
		if( !isShadowed ) glColor4ub(20, 20, 255, 255);
		startPoint.draw(isShadowed);
		if( !isShadowed ) glColor4ub(20, 20, 255, 255);
		endPoint.draw(isShadowed);

		if( !isShadowed ) glColor4ub(128, 0, 128, 255);
		control1.draw(isShadowed);
		if( !isShadowed ) glColor4ub(128, 0, 128, 255);
		control2.draw(isShadowed);
	}
}

Vec3f CurveSegment::getOrientation(float t)
{
	return lerp(-t, startPoint.orient(), endPoint.orient());
}

int       CurveSegment::getNumber    () const { return number; }
CurveType CurveSegment::getCurveType () const { return curveType; }

CtrlPoint& CurveSegment::getStartPoint () { return startPoint; }
CtrlPoint& CurveSegment::getEndPoint   () { return endPoint; }
CtrlPoint& CurveSegment::getControl1   () { return control1; }
CtrlPoint& CurveSegment::getControl2   () { return control2; }


/* ==================================================================
 * LineSegment class
 * ==================================================================
 */
void LineSegment::draw(bool drawPoints, bool isShadowed) 
{
	const Vec3f& start(startPoint.pos());
	const Vec3f& end  (endPoint.pos());

	const Vec3f dir (normalize(start - end));
	const Vec3f up  (normalize(startPoint.orient()));
	const Vec3f side(normalize(cross(dir, up)));

	glBegin(GL_LINE_STRIP);
	{
		const Vec3f v1(start + radius * side);
		glVertex3fv(v1.v());
		const Vec3f v2(end + radius * side);
		glVertex3fv(v2.v());
	}
	glEnd();

	glBegin(GL_LINE_STRIP);
	{
		const Vec3f v1(start + -radius * side);
		glVertex3fv(v1.v());
		const Vec3f v2(end + -radius * side);
		glVertex3fv(v2.v());
	}
	glEnd();

	if( drawPoints )
	{
		if( !isShadowed ) glColor4ub(20, 20, 255, 255);
		startPoint.draw(isShadowed);
		if( !isShadowed ) glColor4ub(20, 20, 255, 255);
		endPoint.draw(isShadowed);
	
		if( !isShadowed ) glColor4ub(128, 0, 128, 255);
		control1.draw(isShadowed);
		if( !isShadowed ) glColor4ub(128, 0, 128, 255);
		control2.draw(isShadowed);
	}
}

Vec3f LineSegment::getPosition( float t )
{
	return lerp(-t, startPoint.pos(), endPoint.pos());
}

Vec3f LineSegment::getDirection( float t )
{
	return (startPoint.pos() - endPoint.pos());
}


/* ==================================================================
 * CatmullRomSegment class
 * ==================================================================
 */
Vec3f CatmullRomSegment::getPosition( float t )
{
	const Vec3f p0(startPoint.pos());
	const Vec3f p1(endPoint.pos());
	const Vec3f m0(control1.pos());
	const Vec3f m1(control2.pos());

	const float tt  = t * t;
	const float ttt = t * tt;

	Vec3f pos( 0.5f * (
		(-1.f * ttt + 2.f * tt - t)   * m0
	  + ( 3.f * ttt - 5.f * tt + 2.f) * p0 
	  + (-3.f * ttt + 4.f * tt + t)   * p1
	  + (       ttt -       tt)       * m1 ) 
	);

	return pos;
}

Vec3f CatmullRomSegment::getDirection( float t )
{
	const Vec3f p0(startPoint.pos());
	const Vec3f p1(endPoint.pos());
	const Vec3f m0(control1.pos());
	const Vec3f m1(control2.pos());

	const float tt  = t * t;

	Vec3f dir( 0.5f * (
		(-3.f * tt + 4.f  * t - 1.f) * m0
	  + ( 9.f * tt - 10.f * t)       * p0 
	  + (-9.f * tt + 8.f  * t + 1.f) * p1
	  + ( 2.f * tt -        t)       * m1 )
	);

	return dir.normalize();
}


/* ==================================================================
 * HermiteSegment class
 * ==================================================================
 */
Vec3f HermiteSegment::getPosition( float t )
{
	const Vec3f p0(startPoint.pos());
	const Vec3f p1(endPoint.pos());
	const Vec3f m0(control1.pos());
	const Vec3f m1(control2.pos());

	const float tt  = t * t;
	const float ttt = t * tt;

	Vec3f pos(
		( 2.f * ttt - 3.f * tt + 1.f) * p0
	  + (-2.f * ttt + 3.f * tt      ) * p1
	  + (       ttt - 2.f * tt + t)   * m0
	  + (       ttt -       tt)       * m1
	);

	return pos;
}

Vec3f HermiteSegment::getDirection( float t )
{
	const Vec3f p0(startPoint.pos());
	const Vec3f p1(endPoint.pos());
	const Vec3f m0(control1.pos());
	const Vec3f m1(control2.pos());

	const float tt  = t * t;

	Vec3f dir(
		( 6.f * tt - 6.f * t      ) * p0
	  + (-6.f * tt + 6.f * t      ) * p1
	  + ( 3.f * tt - 4.f * t + 1.f) * m0
	  + ( 3.f * tt - 2.f * t      ) * m1
	);

	return dir.normalize();
}

/* ==================================================================
 * BSplineSegment class
 * ==================================================================
 */
Vec3f BSplineSegment::getPosition( float t )
{
	const Vec3f p0(startPoint.pos());
	const Vec3f p1(endPoint.pos());
	const Vec3f m0(control1.pos());
	const Vec3f m1(control2.pos());

	const float tt  = t * t;
	const float ttt = t * tt;

	Vec3f pos( (1.f / 6.f) * (
		(-1.f * ttt + 3.f * tt - 3.f * t + 1.f) * m0
	  + ( 3.f * ttt - 6.f * tt           + 4.f) * p0
	  + (-3.f * ttt + 3.f * tt + 3.f * t + 1.f) * p1
	  + ( 1.f * ttt                           ) * m1 )
	);

	return pos;
}

Vec3f BSplineSegment::getDirection( float t )
{
	const Vec3f p0(startPoint.pos());
	const Vec3f p1(endPoint.pos());
	const Vec3f m0(control1.pos());
	const Vec3f m1(control2.pos());

	const float tt  = t * t;

	Vec3f dir( (1.f / 6.f) * (
		(-3.f * tt +  6.f * t - 3.f) * m0
	  + ( 9.f * tt - 12.f * t      ) * p0
	  + (-9.f * tt +  6.f * t + 3.f) * p1
	  + ( 3.f * tt                 ) * m1 )
	);

	return dir.normalize();
}
