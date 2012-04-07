#pragma once
/*
 * CurveSegments.h
 */
#include "CtrlPoint.h"
#include "Vec3f.h"

enum CurveType {
	lines = 0,
	catmull,
	cardinal,
	bspline
};

extern std::string CurveTypeNames[];

class Curve;


/* ==================================================================
 * CurveSegment base class
 * ==================================================================
 */
class CurveSegment
{
protected:
	static const int   numLines;
	static const float step;
	static const float radius;

	const Curve& parentCurve;
	int number;
	CurveType curveType;
	CtrlPoint startPoint, endPoint;
	CtrlPoint control1, control2;

public:
	CurveSegment(const Curve& parentCurve,
				 const int number, const CurveType& curveType,
				 const CtrlPoint& startPoint, const CtrlPoint& endPoint, 
				 const CtrlPoint& control1,   const CtrlPoint& control2)
		: parentCurve(parentCurve)
		, number(number)
		, curveType(curveType)
		, startPoint(startPoint)
		, endPoint(endPoint)
		, control1(control1)
		, control2(control2)
	{ }

	virtual void draw(bool drawPoints=false, bool isShadowed=false);

	virtual Vec3f getPosition  (float t) = 0;
	virtual Vec3f getDirection (float t) = 0;;
	virtual Vec3f getOrientation(float t);

	int	      getNumber    () const;
	CurveType getCurveType () const;

	CtrlPoint& getStartPoint ();
	CtrlPoint& getEndPoint   ();
	CtrlPoint& getControl1   ();
	CtrlPoint& getControl2   ();
};


/* ==================================================================
 * LineSegment class
 * ==================================================================
 */
class LineSegment : public CurveSegment
{
public:
	LineSegment(const Curve& parentCurve, const int number,
				const CtrlPoint& startPoint, const CtrlPoint& endPoint,
				const CtrlPoint& control1=CtrlPoint(),
				const CtrlPoint& control2=CtrlPoint())
		: CurveSegment(parentCurve, number, lines, startPoint, endPoint, control1, control2)
	{ }

	virtual void draw(bool drawPoints=false, bool isShadowed=false);

	Vec3f getPosition(float t);
	Vec3f getDirection(float t);
};


/* ==================================================================
 * CatmullRomSegment class
 * ==================================================================
 */
class CatmullRomSegment : public CurveSegment
{
public:
	CatmullRomSegment(const Curve& parentCurve, const int number,
					  const CtrlPoint& startPoint, const CtrlPoint& endPoint,
					  const CtrlPoint& control1,   const CtrlPoint& control2)
		: CurveSegment(parentCurve, number, catmull, startPoint, endPoint, control1, control2)
	{ }

	Vec3f getPosition(float t);
	Vec3f getDirection(float t);
};

/* ==================================================================
 * CardinalSegment class
 * ==================================================================
 */
class CardinalSegment : public CurveSegment
{
public:
	CardinalSegment(const Curve& parentCurve, const int number,
					  const CtrlPoint& startPoint, const CtrlPoint& endPoint,
					  const CtrlPoint& control1,   const CtrlPoint& control2)
		: CurveSegment(parentCurve, number, cardinal, startPoint, endPoint, control1, control2)
	{ }

	Vec3f getPosition(float t);
	Vec3f getDirection(float t);
};

/* ==================================================================
 * BSplineSegment class
 * ==================================================================
 */
class BSplineSegment : public CurveSegment
{
public:
	BSplineSegment(const Curve& parentCurve, const int number,
				   const CtrlPoint& startPoint, const CtrlPoint& endPoint,
				   const CtrlPoint& control1,   const CtrlPoint& control2)
		: CurveSegment(parentCurve, number, bspline, startPoint, endPoint, control1, control2)
	{ }

	Vec3f getPosition(float t);
	Vec3f getDirection(float t);
};
