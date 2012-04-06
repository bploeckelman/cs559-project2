#pragma once
/*
 * Curve.h
 */
#include "CtrlPoint.h"
#include "CurveSegments.h"

#include <stdexcept>
#include <string>
#include <vector>
#include <map>

typedef std::vector<CtrlPoint>             ControlPointVector;
typedef ControlPointVector::iterator       ControlPointVectorIter;
typedef ControlPointVector::const_iterator ControlPointVectorConstIter;

typedef std::vector<CurveSegment*>         CurveSegmentVector;
typedef CurveSegmentVector::iterator       CurveSegmentVectorIter;
typedef CurveSegmentVector::const_iterator CurveSegmentVectorConstIter;


/************************************************************************/
/* Curve class                                                          */
/* -----------                                                          */
/* Stores a vector of control points and a curve type specifier         */
/* and uses these to build a vector of corresponding segments           */
/************************************************************************/
class Curve
{
private:
	CurveType          type;
	ControlPointVector controlPoints;
	CurveSegmentVector segments;

public:
	// TODO: make private?
	int selectedPoint;
	int selectedSegment;

public:
	Curve(const CurveType& type=lines);

	void regenerateSegments();

	void draw(bool drawPoints, bool isShadowed);
	void drawPoint(int index, bool isShadowed);
	void drawPoints(bool isShadowed) const;
	void drawSelectedSegment(bool drawPoints, bool isShadowed);

	void setCurveType(const CurveType& curveType);

	int numSegments() const;
	int numControlPoints() const; 
	int addControlPoint(const CtrlPoint& point);
	void clearPoints();
	void delControlPoint(const int id);

	Vec3f getPosition (const float t);
	Vec3f getDirection(const float t);
	CtrlPoint& getPoint(int id);
	CurveType  getCurveType() const;
	CurveSegment* getSegment(const int number);
	ControlPointVector& getControlPoints(); 

	class NoSuchPoint : public std::runtime_error { public: NoSuchPoint(const std::string& what_arg) : std::runtime_error(what_arg) { } };

private:
	void drawSegment(const int number, bool isShadowed);

	void regenerateLineSegments();
	void regenerateCatmullSegments();
	void regenerateHermiteSegments();
	void regenerateBSplineSegments();
};

inline int Curve::numSegments()        const { return segments.size(); }
inline int Curve::numControlPoints()   const { return controlPoints.size(); }
inline CurveType Curve::getCurveType() const { return type; }
inline ControlPointVector& Curve::getControlPoints()   { return controlPoints; }
