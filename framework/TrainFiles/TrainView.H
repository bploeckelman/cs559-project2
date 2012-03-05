#pragma once
// CS559 Train Project
// TrainView class definition
//
// The TrainView is the window that actually shows the train. Its a
// GL display canvas (Fl_Gl_Window).  It is held within a TrainWindow
// that is the outer window with all the widgets. The TrainView needs 
// to be aware of the window - since it might need to check the widgets 
// to see how to draw

// we need to have pointers to this, but maybe not know about it
// (beware circular references)
//
// written October 2008, by Mike Gleicher
//
class TrainWindow;
class World;

//
// You might change the TrainView in order to add different objects to
// be drawn, or adjust the lighting, or ...
//
// The TrainView also needs to handle its user interface events.
// since this is an FLTk Window, we need to include FlTk Headers
// the pragmas turn off the warnings from FlTk - our code should have no warnings,
// but their code doesn't meet that standard
#pragma warning(push)
#pragma warning(disable:4312)
#pragma warning(disable:4311)
#include <Fl/Fl_Gl_Window.h>
#pragma warning(pop)

// this uses the old ArcBall Code
#include "Utilities/ArcBallCam.H"

// note that this mainly takes care of the drawing and other window
// stuff
class TrainView : public Fl_Gl_Window
{
public:
	// note that we keep the "standard widget" constructor arguments
	TrainView(int x, int y, int w, int h, const char* l = 0);

	// overrides of important window things
	virtual int handle(int);
	virtual void draw();

	// all of the actual drawing happens in this routine
	// it has to be encapsulated, since we draw differently if
	// we're drawing shadows (no colors, for example)
	void drawStuff(bool doingShadows=false);

	// setup the projection - assuming that the projection stack has been
	// cleared for you
	void setProjection();

	// keep an ArcBall for the UI
	ArcBallCam arcball;
	void resetArcball();

	// pick a point (for when the mouse goes down)
	void doPick();
	// simple - just remember which cube is selected
	int selectedCube;

	// easy access to parent
	TrainWindow* tw;
	World* world;
};



// CVS Header - if you don't know what this is, don't worry about it
// This code tells us where the original came from in CVS
// Its a good idea to leave it as-is so we know what version of
// things you started with
// $Header: /p/course/cs559-gleicher/private/CVS/TrainFiles/TrainView.H,v 1.3 2008/10/19 01:54:28 gleicher Exp $
