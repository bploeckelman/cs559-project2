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

	void drawPoints()  const;

	void regenerateSegments();
	void regenerateLineSegments();
	void regenerateCatmullSegments();
	void regenerateBSplineSegments();

public:
	Curve(const CurveType& type=points);

	void draw();
	void drawSegment(const int number);

	Vec3f getPosition(const float t);
	Vec3f getDirection(const float t);
	CurveSegment* getSegment(const int number);

	int  addControlPoint(const CtrlPoint& point);
	void delControlPoint(const int id);

	void setCurveType(const CurveType& curveType);
	CurveType getCurveType() const;

	int numControlPoints() const; 
	int numSegments() const;

	CtrlPoint& getPoint(int id);
	class NoSuchPoint : public std::runtime_error { public: NoSuchPoint(const std::string& what_arg) : std::runtime_error(what_arg) { } };
};
