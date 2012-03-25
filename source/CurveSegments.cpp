/*
 * CurveSegments.cpp
 */
#include "CurveSegments.h"

#include <Windows.h>
#define WIN32_LEAN_AND_MEAN

#include <GL/GL.h>

#include <stdexcept>

// Map CurveType enum value to string representation
std::string CurveTypeNames[] = {
	"Points", 
	"Lines",
	"Catmull-Rom",
	"Hermite",
	"B-Spline"
};


int       CurveSegment::getNumber    () const { return number; }
CurveType CurveSegment::getCurveType () const { return curveType; }

Vec3f CurveSegment::getStartPoint () const { return startPoint; }
Vec3f CurveSegment::getEndPoint   () const { return endPoint; }
Vec3f CurveSegment::getControl1   () const { return control1; }
Vec3f CurveSegment::getControl2   () const { return control2; }



void LineSegment::draw() 
{
	glBegin(GL_LINES);

	glVertex3f(startPoint.x(), startPoint.y(), startPoint.z());
	glVertex3f(endPoint.x(), endPoint.y(), endPoint.z());

	glEnd();
}

Vec3f LineSegment::getPosition( float t )
{
	return lerp(t, startPoint, endPoint);
}

Vec3f LineSegment::getDirection( float t )
{
	return (endPoint - startPoint); //.normalize();
}


void CatmullRomSegment::draw() 
{
	glBegin(GL_LINE_STRIP);

	float t = 0.f;
	for(int i = 0; i <= 25; ++i, t += 0.04f)
	{
		const Vec3f p(getPosition(t));
		glVertex3f(p.x(), p.y(), p.z());	
	}

	glEnd();
}

Vec3f CatmullRomSegment::getPosition( float t )
{
	const Vec3f p0(startPoint);
	const Vec3f p1(endPoint);
	const Vec3f m0(control1);
	const Vec3f m1(control2);

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
{	const Vec3f p0(startPoint);
	const Vec3f p1(endPoint);
	const Vec3f m0(control1);
	const Vec3f m1(control2);

	const float tt  = t * t;

	Vec3f dir( 0.5f * (
		(-3.f * tt + 4.f  * t - 1.f) * m0
	  + ( 9.f * tt - 10.f * t)       * p0 
	  + (-9.f * tt + 8.f  * t + 1.f) * p1
	  + ( 2.f * tt -        t)       * m1 ) 
	);

	return dir.normalize();
}
