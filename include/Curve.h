#pragma once
/*
 * Curve.h
 */
#include "CtrlPoint.h"
#include "CurveSegments.h"

#include <stdexcept>
#include <string>
#include <vector>


class Curve
{
private:
	CurveType type;
	std::vector<CtrlPoint> controlPoints;
	std::vector<CurveSegment*> segments;

	void drawSegment(const int number, bool isShadowed);

	void regenerateLineSegments();
	void regenerateCatmullSegments();
	void regenerateHermiteSegments();
	void regenerateBSplineSegments();

public:
	//new structure stored per curve, made it public so I didnt need to add all the get/set methods when it's called in mainwindow.cpp
	//I kept the double types to allow for more precision when calculating length values for the table, float types also work (but with less precsion)
	struct ParameterTable
	{
		double local_t;
		double accumulated_length;
		double fraction_of_accumulated_length;
		int segment_number;
	};
	//new vector for storing everything about a curve
	/*the size of the paramer_table once fully built should be 1+num_segments()*number_of_samples*/
	std::vector<ParameterTable> parameter_table;

	int selectedPoint;
	int selectedSegment;

	Curve(const CurveType& type=lines);

	void regenerateSegments();
	void draw(bool isShadowed);
	void drawPoint(int index, bool isShadowed);
	void drawPoints(bool isShadowed) const;
	void drawSelectedSegment(bool isShadowed);

	Vec3f getPosition(const float t);
	Vec3f getDirection(const float t);
	CurveSegment* getSegment(const int number);

	int  addControlPoint(const CtrlPoint& point);
	void delControlPoint(const int id);

	void setCurveType(const CurveType& curveType);
	CurveType getCurveType() const;

	int numControlPoints() const; 
	int numSegments() const;

	std::vector<CtrlPoint>& getControlPoints(); 

	CtrlPoint& getPoint(int id);
	void clearPoints();

	class NoSuchPoint : public std::runtime_error { public: NoSuchPoint(const std::string& what_arg) : std::runtime_error(what_arg) { } };

	void BuildParameterTable(int number_of_samples);
};
