/*
 * Curve.cpp
 */
#include "Curve.h"

#include <Windows.h>
#define WIN32_LEAN_AND_MEAN

#include <GL/GL.h>

#include <stdexcept>
#include <iostream>
#include <sstream>
#include <vector>

using std::stringstream;
using std::vector;
using std::cout;
using std::endl;


Curve::Curve(const CurveType& type)
	: type(type)
	, controlPoints()
	, segments()
	, selectedPoint(-1)
	, selectedSegment(-1)
{ }

void Curve::draw(bool isShadowed) 
{
	if( controlPoints.empty() )
		return;

	if( segments.empty() )
		regenerateSegments();

	for each(auto segment in segments)
	{
		if( segment != nullptr )
			segment->draw(false,isShadowed);

			
	}
}

void Curve::drawPoint( int index, bool isShadowed )
{
	if( index < 0 || index >= (int)controlPoints.size() )
		return;

	controlPoints.at(index).draw(isShadowed);
}

void Curve::drawPoints(bool isShadowed) const
{
	// Note: visual studio 2010 implemented c++0x range-based for loops 
	// in this non-standard way apparently, see: 
	// http://blogs.msdn.com/b/vcblog/archive/2009/07/13/intellisense-and-browsing-with-c-0x.aspx

	for each(const auto& p in controlPoints)
	{
		p.draw(isShadowed);			
	}
}

void Curve::drawSelectedSegment(bool isShadowed)
{
	if( selectedSegment < 0 || selectedSegment >= (int)segments.size() )
		return;

		segments.at(selectedSegment)->draw(true, isShadowed);

}

Vec3f Curve::getPosition( const float t )
{
	const int segmentNumber = static_cast<int>(std::floor(t));
	try {
		CurveSegment *segment = segments.at(segmentNumber);
		const float tUnit = t - segmentNumber; 
		return segment->getPosition(tUnit); 
	}
	catch(std::out_of_range&) {
		stringstream ss;
		ss  << "Warning: Curve::getPosition out of range: "
			<< "t=" << t << "  seg#=" << segmentNumber << "  " 
			<< "#segs=" << segments.size() << endl;
		cout << ss.str();
		throw NoSuchPoint(ss.str());
	}
}

Vec3f Curve::getDirection( const float t )
{
	const int segmentNumber = static_cast<int>(std::floor(t));
	try	{
		CurveSegment *segment = segments.at(segmentNumber);
		const float tUnit = t - segmentNumber; 
		return segment->getDirection(tUnit);
	} catch(std::out_of_range&) {
		stringstream ss;
		ss  << "Warning: Curve::getDirection out of range: "
			<< "t=" << t << "  seg#=" << segmentNumber << "  " 
			<< "#segs=" << segments.size() << endl;
		cout << ss.str();
		throw NoSuchPoint(ss.str());		
	}
}

CurveSegment* Curve::getSegment( const int number )
{
	try {
		return segments.at(number);
	} catch(std::out_of_range&) {
		cout << "Warning: Curve::getSegment out of range: " << number << endl;
		return nullptr;
	}
}

void Curve::regenerateSegments()
{
	// Clean up old segments
	for each(auto segment in segments)
	{
		delete segment;
	}
	segments.clear();

	//this may need to take in a t value? for arc length parametrization

	// Can't create segments without control points
	if( controlPoints.empty() )
		return;

	// Create new segments using control points and curve type
	switch(type)
	{
	case lines:   regenerateLineSegments();    break;
	case catmull: regenerateCatmullSegments(); break;
	case bspline: regenerateBSplineSegments(); break;
	}

}

void Curve::regenerateLineSegments()
{
	auto it  = controlPoints.begin();
	auto end = controlPoints.end();
	for(int i = 0; it != end; ++i, ++it)
	{
		const bool last = ((it + 1) == end);

		const CtrlPoint& p0(*it);
		const CtrlPoint& p1(last ? *(controlPoints.begin()) : *(it + 1));

		segments.push_back(new LineSegment(i, p0, p1));
	}
}

void Curve::regenerateCatmullSegments()
{
	auto it  = controlPoints.begin();
	auto end = controlPoints.end();
	for(int i = 0; it != end; ++i, ++it)
	{
		if( i == 0 && controlPoints.size() >= 4 )
		{
			const CtrlPoint& c1(*(end - 1));
			const CtrlPoint& p0(*(it));
			const CtrlPoint& p1(*(it + 1));
			const CtrlPoint& c2(*(it + 2));
			segments.push_back(new CatmullRomSegment(i, p0, p1, c1, c2));
		} else {
			auto next1 = it + 1;
			if( next1 >= end )
				next1 = controlPoints.begin();

			auto next2 = next1 + 1; 
			if( next2 >= end )
				next2 = controlPoints.begin();

			auto prev = it;
			if( prev != controlPoints.begin() )
				prev -= 1;

			const CtrlPoint& c1(*prev);
			const CtrlPoint& p0(*(it));
			const CtrlPoint& p1(*next1);
			const CtrlPoint& c2(*next2);

			segments.push_back(new CatmullRomSegment(i, p0, p1, c1, c2));
		}
	}
}

void Curve::regenerateHermiteSegments()
{
	throw std::exception("The method or operation is not implemented.");
}

void Curve::regenerateBSplineSegments()
{
	throw std::exception("The method or operation is not implemented.");
}

int Curve::addControlPoint( const CtrlPoint& point )
{
	// Add the new point
	controlPoints.push_back(point);
	// Rebuild segments
	regenerateSegments();
	// Return the index of the newly added point
	return controlPoints.size() - 1;
}

void Curve::delControlPoint( const int id ) 
{
	try {
		// Make sure it is a good index,
		// throwing an exception if it isn't
		controlPoints.at(id);
		// Erase the point at that index
		controlPoints.erase(controlPoints.begin() + id);
		// Rebuild segments
		regenerateSegments();
	}
	catch(std::out_of_range&) {
		stringstream ss;
		ss << "Warning: no such point on curve with id=" << id;
		throw NoSuchPoint(ss.str());
	}
}

void Curve::setCurveType( const CurveType& curveType ) 
{ 
	type = curveType; 
	regenerateSegments();
}

CurveType Curve::getCurveType() const { return type; }
int Curve::numControlPoints  () const { return controlPoints.size(); }
int Curve::numSegments       () const { return segments.size(); }

vector<CtrlPoint>& Curve::getControlPoints() { return controlPoints; }

CtrlPoint& Curve::getPoint( int id )
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

void Curve::drawSegment( const int number, bool isShadowed )
{
	try {
		segments.at(number)->draw(false, isShadowed);
	} catch(std::out_of_range&) {
		stringstream ss;
		ss << "Warning: no point on curve with index=" << number;
		cout << ss.str() << endl;
//		throw NoSuchPoint(ss.str());
	}
}

//ONLY CALL THIS IF YOU ARE IMMEDIATELY going to fill up the control points of the curve!
void Curve::clearPoints()
{
	controlPoints.clear();
	regenerateSegments();

}

