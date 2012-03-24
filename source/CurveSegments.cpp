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
	"B-Spline"
};


int       CurveSegment::getNumber    () const { return number; }
CurveType CurveSegment::getCurveType () const { return curveType; }

Vec3f CurveSegment::getStartPoint () const { return startPoint; }
Vec3f CurveSegment::getEndPoint   () const { return endPoint; }
Vec3f CurveSegment::getControl1   () const { return control1; }
Vec3f CurveSegment::getControl2   () const { return control2; }



void LineSegment::draw() const
{
	glBegin(GL_LINES);

	glVertex3f(startPoint.x(), startPoint.y(), startPoint.z());
	glVertex3f(endPoint.x(), endPoint.y(), endPoint.z());

	glEnd();
}

Vec3f LineSegment::getPosition( const float t )
{
	return lerp(t, startPoint, endPoint);
}

Vec3f LineSegment::getDirection( const float t )
{
//	return (endPoint - startPoint).normalize();
	return (endPoint - startPoint);
}


void CatmullRomSegment::draw() const
{
	throw std::runtime_error("CatmullRomSegment::draw() not implemented");
}

Vec3f CatmullRomSegment::getPosition( const float t )
{
	throw std::runtime_error("CatmullRomSegment::() not implemented");
}

Vec3f CatmullRomSegment::getDirection( const float t )
{
	throw std::runtime_error("CatmullRomSegment::getDirection() not implemented");
}
