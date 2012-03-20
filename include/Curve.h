#pragma once
/*
 * Curve.h
 */
#include "CtrlPoint.h"

#include <stdexcept>
#include <string>
#include <vector>

enum CurveType {
	points = 0, 
	lines,
	bezier,
	catmull,
	bspline
};

extern std::string CurveTypeNames[];


class Curve
{
private:
	CurveType type;
	std::vector<CtrlPoint> controlPoints;

	void drawPoints()  const;
	void drawLines()   const;
	void drawBezier()  const;
	void drawCatmull() const;
	void drawBSpline() const;

public:
	Curve(const CurveType& type=points);

	virtual void draw() const;

	int addControlPoint(const CtrlPoint& point);
	int numControlPoints() const; 

	CtrlPoint& point(int id);
	
	class NoSuchPoint : public std::runtime_error { public: NoSuchPoint(const std::string& what_arg) : std::runtime_error(what_arg) { } };
};
