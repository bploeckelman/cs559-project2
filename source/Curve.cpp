/*
 * Curve.cpp
 */
#include "Curve.h"

#include <Windows.h>
#define WIN32_LEAN_AND_MEAN

#include <GL/GL.h>

#include <stdexcept>
#include <sstream>
#include <string>
#include <vector>

using std::stringstream;
using std::vector;

// Map CurveType enum value to string representation
std::string CurveTypeNames[] = {
	"Points", 
	"Lines",
	"Bezier",
	"Catmull-Rom",
	"B-Spline"
};


Curve::Curve(const CurveType& type)
	: type(type)
	, controlPoints()
{ }

int Curve::addControlPoint( const CtrlPoint& point )
{
	controlPoints.push_back(point);
	return controlPoints.size() - 1;
}

inline int Curve::numControlPoints() const { return controlPoints.size(); }

CtrlPoint& Curve::point( int id )
{
	try {
		auto& p = controlPoints.at(id);
		return p;
	} catch(std::out_of_range&) {
		stringstream ss;
		ss << "Warning: no point on curve with id=" << id;
		throw NoSuchPoint(ss.str());
	}
}

void Curve::draw() const
{
	switch(type)
	{
	case points:  drawPoints();  break;
	case lines:   drawLines();   break;
	case bezier:  drawBezier();  break;
	case catmull: drawCatmull(); break;
	case bspline: drawBSpline(); break;
	}
}

void Curve::drawPoints() const
{
	// Note: visual studio 2010 implemented c++0x range-based for loops 
	// in this non-standard way apparently, see: 
	// http://blogs.msdn.com/b/vcblog/archive/2009/07/13/intellisense-and-browsing-with-c-0x.aspx

	for each(const auto& p in controlPoints)
	{
		p.draw();			
	}
}

void Curve::drawLines() const
{
	glPushMatrix();
	glBegin(GL_LINES);

	auto pit  = controlPoints.begin();
	auto pend = controlPoints.end();
	for(; pit != pend; ++pit)
	{
		Vec3f p1((*(pit + 0)).pos());
		Vec3f p2; // either the next point, or wrap to the first

		auto pit2 = (pit + 1);
		if( pit2 != pend )
			p2 = (*pit2).pos();
		else
			p2 = (*controlPoints.begin()).pos();

		glVertex3f(p1.x(), p1.y(), p1.z());
		glVertex3f(p2.x(), p2.y(), p2.z());
	}

	glEnd();
	glPopMatrix();

	drawPoints();
}

void Curve::drawBezier() const
{
	throw std::runtime_error("Curve::drawBezier() not implemented");
}

void Curve::drawCatmull() const 
{
	throw std::runtime_error("Curve::drawCatmull() not implemented");
}

void Curve::drawBSpline() const
{
	throw std::runtime_error("Curve::drawBSpline() not implemented");
}
