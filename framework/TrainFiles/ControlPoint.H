#pragma once
// CS559 Train Project
//	begun Oct 2008, Mike Gleicher

// Data structure for control points
// These are not just points in space (see Point3D), because they may have
// an orientation associated with them. For most people, the orientation will
// just be ignored. However, if you try some advanced features, you might
// find the orientation vectors handy.
//
// I assume the orientation points UP (the positive Y axis), so that's the default.
// When things get drawn, the point "points" in that direction

#include "Utilities/Pnt3f.H"


class ControlPoint {
public:
	Pnt3f pos;
	Pnt3f orient;

	// constructors
	ControlPoint();					// need a default constructor for making arrays
	ControlPoint(const Pnt3f& pos);	// create in a position
	ControlPoint(const Pnt3f& pos, const Pnt3f& orient);

	// draw the control point - assumes the color is correct
	void draw();
};
// $Header: /p/course/cs559-gleicher/private/CVS/TrainFiles/ControlPoint.H,v 1.3 2008/10/16 14:34:54 gleicher Exp $