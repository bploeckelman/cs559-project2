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
#include <cassert>
#include <vector>

using std::stringstream;
using std::vector;
using std::cout;
using std::endl;


/************************************************************************/
/* Curve class                                                          */
/************************************************************************/

Curve::Curve(const CurveType& type)
	: type(type)
	, controlPoints()
	, segments()
	, selectedPoint(-1)
	, selectedSegment(-1)
{ }

/* draw() - Draws all the segments of this curve ----------------- */
void Curve::draw(bool drawPoints, bool isShadowed)
{
	if( controlPoints.empty() )
		return;

	if( segments.empty() )
		regenerateSegments();

	for each(auto segment in segments)
	{
		assert(segment != nullptr);
		if( !isShadowed ) glColor4ub(164, 164, 164, 255); 
		segment->draw(false, isShadowed);
	}

	if( drawPoints ) Curve::drawPoints(isShadowed);
}

/* drawPoint() - Draws the control point at the specified index, if it exists */
void Curve::drawPoint( int index, bool isShadowed )
{
	if( index < 0 || index >= (int)controlPoints.size() )
		return;

	controlPoints[index].draw(isShadowed);
}

/* drawPoints() - Draws all the control points for this curve ---- */
void Curve::drawPoints(bool isShadowed) const
{
	for each(const auto& p in controlPoints)
	{
		if( !isShadowed ) glColor4ub(255, 255, 255, 255);
		p.draw(isShadowed);			
	}
}

/* drawSelectedSegment() - Draws the selected segment ------------ */
void Curve::drawSelectedSegment(bool drawPoints, bool isShadowed)
{
	if( selectedSegment < 0 || selectedSegment >= numSegments() ) 
		return;

	if( !isShadowed ) glColor4ub(255, 20, 20, 255);
	segments[selectedSegment]->draw(drawPoints, isShadowed);
}

/* setCurveType() - Sets the type of curve used to evaluate the control points */
void Curve::setCurveType( const CurveType& curveType ) 
{ 
	type = curveType; 
	regenerateSegments();
}

/* addControlPoint() - Add the specified control point to the curve */
int Curve::addControlPoint( const CtrlPoint& point )
{
	controlPoints.push_back(point);
	regenerateSegments();
	return controlPoints.size() - 1;
}

/* clearPoints() - Clears all the control points ----------------- */
//ONLY CALL THIS IF YOU ARE IMMEDIATELY going to fill up the control points of the curve!
void Curve::clearPoints()
{
	controlPoints.clear();
	regenerateSegments();
}

/* delControlPoint() - Tries to delete the point at the specified index */
/* Throws NoSuchPoint exception on bad point index --------------- */
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

/* getPosition() - Lerp across curve segments to find position --- */
/* Throws NoSuchPoint exception on bad segment index ------------- */
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

/* getDirection() - Lerp across curve segments to find direction - */
/* Throws NoSuchPoint exception on bad segment index ------------- */
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

/* getOrientation() - Returns the interpolated orientation ------- */
Vec3f Curve::getOrientation( const float t )
{
	const int segmentNumber = static_cast<int>(std::floor(t));
	try {
		CurveSegment *segment = segments.at(segmentNumber);
		const float tUnit = t - segmentNumber;
		return segment->getOrientation(tUnit);
	} catch(std::out_of_range&) {
		stringstream ss;
		ss  << "Warning: Curve::getOrientation out of range: "
			<< "t=" << t << "  seg#=" << segmentNumber << "  "
			<< "#segs=" << segments.size() << endl;
		cout << ss.str();
		throw NoSuchPoint(ss.str());
	}
}

/* getPoint() - Returns the specified point, if it exists -------- */
/* Throws NoSuchPoint exception on bad point index --------------- */
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

/* getSegment() - Returns the specified segment, if it exists ---- */
/* Returns nullptr if segment doesn't exist ---------------------- */
CurveSegment* Curve::getSegment( const int number )
{
	if( number < 0 || number >= numSegments() )
		return nullptr;

	return segments[number];
}

/* regenerateSegments() - Regenerates segments based on control points and curve type */
void Curve::regenerateSegments()
{
	// Clean up old segments
	for each(auto segment in segments)
	{
		delete segment;
	}
	segments.clear();

	// Can't create segments without control points or with 1 control point
	if( controlPoints.empty())
		return;

	// Create new segments using control points and curve type
	switch(type)
	{
	case lines:   regenerateLineSegments();    break;
	case catmull: regenerateCatmullSegments(); break;
	case bspline: regenerateBSplineSegments(); break;
	}
}

/* regenerateLineSegments() - Regenerates segments as lines ------ */
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

/* regenerateCatmullSegments() - Regenerates segments as catmull-rom splines */
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

/* regenerateHermiteSegments() - Regenerates segments as hermite splines */
void Curve::regenerateHermiteSegments()
{
	throw std::exception("The method or operation is not implemented.");
}

/* regenerateBSplineSegments() - Regenerates segments as bsplines */
void Curve::regenerateBSplineSegments()
{
	throw std::exception("The method or operation is not implemented.");
}

/* drawSegment() - Draws the specified segment ------------------- */
void Curve::drawSegment( const int number, bool isShadowed )
{
	if( number < 0 || number >= numSegments() )
		return;

	if( !isShadowed ) glColor4ub(255, 255, 255, 255);
	segments[number]->draw(false, isShadowed);
}
