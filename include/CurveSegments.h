#pragma once
/*
 * CurveSegments.h
 */
#include "Vec3f.h"

enum CurveType {
	points = 0, 
	lines,
	catmull,
	bspline
};

extern std::string CurveTypeNames[];


class CurveSegment
{
protected:
	int number;
	CurveType curveType;
	Vec3f startPoint, endPoint;
	Vec3f control1, control2;

public:
	CurveSegment(const int number, const CurveType& curveType, 
				 const Vec3f& startPoint, const Vec3f& endPoint, 
				 const Vec3f& control1,   const Vec3f& control2)
		: number(number)
		, curveType(curveType)
		, startPoint(startPoint)
		, endPoint(endPoint)
		, control1(control1)
		, control2(control2)
	{ }

	virtual void draw() const = 0;

	virtual Vec3f getPosition  (const float t) = 0;
	virtual Vec3f getDirection (const float t) = 0;;

	int	      getNumber    () const;
	CurveType getCurveType () const;

	Vec3f getStartPoint () const;
	Vec3f getEndPoint   () const;
	Vec3f getControl1   () const;
	Vec3f getControl2   () const;
};


class LineSegment : public CurveSegment
{
public:
	LineSegment(const int number, 
				const Vec3f& startPoint, const Vec3f& endPoint, 
				const Vec3f& control1=Vec3f(0.f, 0.f, 0.f), 
				const Vec3f& control2=Vec3f(0.f, 0.f, 0.f))
		: CurveSegment(number, lines, startPoint, endPoint, control1, control2) 
	{ }

	void draw() const;

	Vec3f getPosition(const float t);
	Vec3f getDirection(const float t);
};


class CatmullRomSegment : public CurveSegment
{
public:
	CatmullRomSegment(const int number, 
					  const Vec3f& startPoint, const Vec3f& endPoint, 
					  const Vec3f& control1, const Vec3f& control2)
		: CurveSegment(number, catmull, startPoint, endPoint, control1, control2) 
	{ }

	void draw() const;

	Vec3f getPosition(const float t);
	Vec3f getDirection(const float t);
};
