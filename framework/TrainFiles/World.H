#pragma once
// CS559 Train Project
//	begun Oct 2008, Mike Gleicher
//
// Container for the "World"
//
// This provides a container for all of the "stuff" in the world.
//
// It could have been all global variables, or it could have just been
// contained in the window. The advantage of doing it this way is that
// we might have multiple windows looking at the same world. But, I don't
// think we'll actually do that.
//
// See the readme for commentary on code style
#include <vector>

// avoid having to say std::vector all of the time
using std::vector;

// make use of other data structures from this project
#include "ControlPoint.H"

class World {
public:		// we don't make anything private - for no good reason
	World();

	// rather than have generic objects, we make a special case for these few
	// objects that we know that all implementations are going to need and that
	// we're going to have to handle specially
	vector<ControlPoint> points;

	// when we want to clear the control points, we really "reset" them 
	// to have 4 default positions (since we should never have fewer
	// than 4 points)
	void resetPoints();

	// the state of the train - basically, all I need to remember is where
	// it is in parameter space
	// TODO: you might want to do this differently
	float trainU;

	// read and write to files
	void readPoints(const char* filename);
	void writePoints(const char* filename);
};

// $Header: /p/course/cs559-gleicher/private/CVS/TrainFiles/World.H,v 1.5 2008/10/16 21:15:16 gleicher Exp $