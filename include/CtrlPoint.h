#pragma once
/*
 * CtrlPoint.h
 */
#include "Vec3f.h"


class CtrlPoint
{
private:
	Vec3f _pos;
	Vec3f _orient;

public:

	CtrlPoint(const Vec3f& pos=Vec3f(), 
			  const Vec3f& orient=Vec3f(0.f, 1.f, 0.f));

	void draw() const;

	inline CtrlPoint& pos   (const Vec3f& p) { _pos = p;    return *this; }
	inline CtrlPoint& orient(const Vec3f& o) { _orient = o; return *this; }

	inline const Vec3f& pos()    const { return _pos; }
	inline const Vec3f& orient() const { return _orient; }

	bool operator==(const CtrlPoint& other);
};
