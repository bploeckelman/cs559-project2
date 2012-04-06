/*
 * CurveSegments.cpp
 */
#include "CurveSegments.h"
#include "CtrlPoint.h"
#include "Vec3f.h"

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
	static const float radius = 2.9f;	  // TODO: extract this to base class

	const Vec3f& start(startPoint.pos());
	const Vec3f& end(endPoint.pos());

	const Vec3f dir(normalize(start - end));
	const Vec3f up(0.f, 1.f, 0.f); //normalize((start + end) * 0.5f));
	const Vec3f side(normalize(cross(up, dir)));

	glBegin(GL_LINES);
	{
		const Vec3f v1(start + radius * side);
		glVertex3fv(v1.v());
		const Vec3f v2(end + radius * side);
		glVertex3fv(v2.v());
	}
	glEnd();

	glBegin(GL_LINES);
	{
		const Vec3f v1(start.x() - radius * side.x()
					,  start.y() + radius * side.y()
					,  start.z() - radius * side.z());
		glVertex3fv(v1.v());
		const Vec3f v2(end.x() - radius * side.x()
					,  end.y() + radius * side.y()
					,  end.z() - radius * side.z());
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
void CatmullRomSegment::draw(bool drawPoints, bool isShadowed) 
{
	static const int   lines = 25;
	static const float step  = 1.f / lines;
	static const float radius = 2.9f;	  // TODO: extract this to base class

	float t = 0.f;
	glBegin(GL_LINE_STRIP);
		for(int i = 0; i <= lines; ++i, t += step)
		{
			const Vec3f p(getPosition(t));
			const Vec3f dir(normalize(getDirection(t)));
			const Vec3f up(normalize(getOrientation(t)));
			const Vec3f side(normalize(cross(up, dir)));
			const Vec3f v(p + radius * side);
			glVertex3fv(v.v());
		}
	glEnd();

	t = 0.f;
	glBegin(GL_LINE_STRIP);
		for(int i = 0; i <= lines; ++i, t += step)
		{
			const Vec3f p(getPosition(t));
			const Vec3f dir(normalize(getDirection(t)));
			const Vec3f up(normalize(getOrientation(t)));
			const Vec3f side(normalize(cross(up, dir)));
			const Vec3f v(p.x() - radius * side.x()
						, p.y() + radius * side.y()
						, p.z() - radius * side.z());
			glVertex3fv(v.v());
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
// TODO


/* ==================================================================
 * BSplineSegment class
 * ==================================================================
 */
// TODO
