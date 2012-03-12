#pragma once
/*
 * CtrlPoint.h
 */


class CtrlPoint
{
private:

public:

	CtrlPoint() { }
	// TODO...
//  CtrlPoint(const Vec3f& pos);
//  CtrlPoint(const Vec3f& pos, const Vec3f& orient);

	void draw() const;

	bool operator==(const CtrlPoint& other);

};
