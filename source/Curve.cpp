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

	if(controlPoints.size() > 1)
	{
		BuildParameterTable(100); //TODO: set to 25 samples (same as the catmull rom drawing) but we should get a sample variable here
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
	//printf("control points size in addControlPoint is %d \n", controlPoints.size());
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

/* parameter table building- this table keeps track of values as defined in the parameter_table struct for every point on the curve (ctrl and sample points)*/
/*the size of the paramer_table once fully built should be 1+num_segments()*number_of_samples*/
void Curve::BuildParameterTable(int number_of_samples)
{
	//clear any previous table as we are going to build a new one
	parameter_table.clear();

	//put in the first element of the new table which will always start with control pt 0's position
	std::vector<CtrlPoint> ctrlpts = getControlPoints();
	Vec3f previous_point = ctrlpts.at(0).pos();
	ParameterTable p;
	p.segment_number = 0;
	p.accumulated_length = 0;
	p.fraction_of_accumulated_length = 0;
	p.local_t = 0;
	parameter_table.push_back(p);

	for (int i = 0; i < numSegments(); i++)
	{
		for (int j = 1; j < number_of_samples; j++)
		{
			
			p.segment_number = i;
			p.local_t = ((double) j) / ((double) number_of_samples - 1);
			//printf(" number_of_samples is %d, i is %d, j is %d, local_t is %f \n", number_of_samples, i, j, p.local_t);
			Vec3f v;

			//if we are at the end of a segment, use the ctrl points position
			if(std::floor(p.local_t-1) == 0)
			{
				//printf(" in if \n");
				//printf(" ctrlpts.size() is %d \n", ctrlpts.size());
				v= ctrlpts.at(i).pos();

			}
			else//use local position based on the number of samples we want for the segment
			{
				//printf(" in else \n");
				v= getPosition((float)p.local_t+i); //may not work
			}
			p.accumulated_length = (double)((previous_point - v).Magnitude()) + parameter_table[parameter_table.size() - 1].accumulated_length;
			parameter_table.push_back(p);
			previous_point = v;
		}
	}

	double total_length = parameter_table[parameter_table.size() - 1].accumulated_length;
	//set each elements fraction_of_accum_length field based off of each elements accum_length and the total_length
	for (size_t i = 0; i < parameter_table.size(); i++)
	{
		parameter_table[i].fraction_of_accumulated_length = parameter_table[i].accumulated_length / total_length;
	}
}

