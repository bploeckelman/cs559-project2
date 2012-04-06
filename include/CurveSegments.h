#pragma once
/*
 * CurveSegments.h
 */
#include "CtrlPoint.h"
#include "Vec3f.h"

enum CurveType {
	lines = 0,
	catmull,
	hermite,
	bspline
};

extern std::string CurveTypeNames[];


/* ==================================================================
 * CurveSegment base class
 * ==================================================================
 */
class CurveSegment
{
protected:
	int number;
	CurveType curveType;
	CtrlPoint startPoint, endPoint;
	CtrlPoint control1, control2;

public:
	CurveSegment(const int number, const CurveType& curveType, 
				 const CtrlPoint& startPoint, const CtrlPoint& endPoint, 
				 const CtrlPoint& control1,   const CtrlPoint& control2)
		: number(number)
		, curveType(curveType)
		, startPoint(startPoint)
		, endPoint(endPoint)
		, control1(control1)
		, control2(control2)
	{ }

	virtual void draw(bool drawPoints=false, bool isShadowed=false) = 0;

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
	LineSegment(const int number, 
				const CtrlPoint& startPoint, const CtrlPoint& endPoint,
				const CtrlPoint& control1=CtrlPoint(),
				const CtrlPoint& control2=CtrlPoint())
		: CurveSegment(number, lines, startPoint, endPoint, control1, control2) 
	{ }

	void draw(bool drawPoints=false, bool isShadowed=false);

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
	CatmullRomSegment(const int number, 
					  const CtrlPoint& startPoint, const CtrlPoint& endPoint,
					  const CtrlPoint& control1,   const CtrlPoint& control2)
		: CurveSegment(number, catmull, startPoint, endPoint, control1, control2) 
	{ }

	void draw(bool drawPoints=false, bool isShadowed=false);

	Vec3f getPosition(float t);
	Vec3f getDirection(float t);
};
