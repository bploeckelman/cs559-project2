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

	CtrlPoint();

	CtrlPoint(const Vec3f& pos);

	CtrlPoint(const Vec3f& pos, 
			  const Vec3f& orient);

	void draw(bool isShadowed) const;

	inline CtrlPoint& pos   (const Vec3f& p) { _pos = p;    return *this; }
	inline CtrlPoint& orient(const Vec3f& o) { _orient = o; return *this; }

	inline const Vec3f& pos()    const { return _pos; }
	inline const Vec3f& orient() const { return _orient; }

	inline Vec3f& pos()    { return _pos; }
	inline Vec3f& orient() { return _orient; }

	bool operator==(const CtrlPoint& other);
};
