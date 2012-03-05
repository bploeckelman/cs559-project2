#pragma once
// CS559 Train Project - 
// define the callbacks for the TrainWindow
//
// these are little functions that get called when the various widgets
// get accessed (or the fltk timer ticks). these functions are used 
// when TrainWindow sets itself up.
//
// written by Mike Gleicher, October 2008
//

#include "TrainWindow.H"
#include "TrainView.H"
#include "CallBacks.H"
#include <time.h>
#include <math.h>

#pragma warning(push)
#pragma warning(disable:4312)
#pragma warning(disable:4311)
#include <Fl/Fl_File_Chooser.H>
#include <Fl/math.h>
#pragma warning(pop)

// reset the control points back to their base setup
void resetCB(Fl_Widget*, TrainWindow* tw)
{
	tw->world.resetPoints();
	tw->trainView->selectedCube = -1;
	tw->world.trainU = 0;
	tw->damageMe();
}

// any time something changes, you need to force a redraw
void damageCB(Fl_Widget*, TrainWindow* tw)
{
	tw->damageMe();
}

// Callback that adds a new point to the spline
// idea: add the point AFTER the selected point
void addPointCB(Fl_Widget*, TrainWindow* tw)
{
	// get the number of points
	size_t npts = tw->world.points.size();
	// the number for the new point
	size_t newidx = (tw->trainView->selectedCube>=0) ? tw->trainView->selectedCube : 0;

	// pick a reasonable location
	size_t previdx = (newidx + npts -1) % npts;
	Pnt3f npos = (tw->world.points[previdx].pos + tw->world.points[newidx].pos) * .5f;

	tw->world.points.insert(tw->world.points.begin() + newidx,npos);

	// make it so that the train doesn't move - unless its affected by this control point
	// it should stay between the same points
	if (ceil(tw->world.trainU) > ((float)newidx)) {
		tw->world.trainU += 1;
		if (tw->world.trainU >= npts) tw->world.trainU -= npts;
	}

	tw->damageMe();
}

// Callback that deletes a point from the spline
void deletePointCB(Fl_Widget*, TrainWindow* tw)
{
	if (tw->world.points.size() > 4) {
		if (tw->trainView->selectedCube >= 0) {
			tw->world.points.erase(tw->world.points.begin() + tw->trainView->selectedCube);
		} else
			tw->world.points.pop_back();
	}
	tw->damageMe();
}
// Callbacks for advancing/pulling back train
void forwCB(Fl_Widget*, TrainWindow* tw)
{
	tw->advanceTrain(2);
	tw->damageMe();
}
void backCB(Fl_Widget*, TrainWindow* tw)
{
	tw->advanceTrain(-2);
	tw->damageMe();
}




// Callback for idling - if things are sitting, this gets called
// if the run button is pushed, then we need to make the train go.
// This is taken from the old "RunButton" demo.
// another nice problem to have - most likely, we'll be too fast
// don't draw more than 30 times per second
static unsigned long lastRedraw = 0;
void runButtonCB(TrainWindow* tw)
{
	if (tw->runButton->value()) {	// only advance time if appropriate
		if (clock() - lastRedraw > CLOCKS_PER_SEC/30) {
			lastRedraw = clock();
			tw->advanceTrain();
			tw->damageMe();
		}
	}
}

// Load and save files
void loadCB(Fl_Widget*, TrainWindow* tw)
{
	const char* fname = fl_file_chooser("Pick a Track File","*.txt","TrackFiles/track.txt");
	if (fname) {
		tw->world.readPoints(fname);
		tw->damageMe();
	}
}
void saveCB(Fl_Widget*, TrainWindow* tw)
{
	const char* fname = fl_input("File name for save (should be *.txt)","TrackFiles/");
	if (fname)
		tw->world.writePoints(fname);
}

// roll control points
void rollx(TrainWindow* tw, float dir)
{
	int s = tw->trainView->selectedCube;
	if (s >= 0) {
		Pnt3f old = tw->world.points[s].orient;
		float si = sin(((float)M_PI_4) * dir);
		float co = cos(((float)M_PI_4) * dir);
		tw->world.points[s].orient.y = co * old.y - si * old.z;
		tw->world.points[s].orient.z = si * old.y + co * old.z;
	}
	tw->damageMe();
} 

void rpxCB(Fl_Widget*, TrainWindow* tw)
{
	rollx(tw,1);
}
void rmxCB(Fl_Widget*, TrainWindow* tw)
{
	rollx(tw,-1);
}

void rollz(TrainWindow* tw, float dir)
{
	int s = tw->trainView->selectedCube;
	if (s >= 0) {
		Pnt3f old = tw->world.points[s].orient;
		float si = sin(((float)M_PI_4) * dir);
		float co = cos(((float)M_PI_4) * dir);
		tw->world.points[s].orient.y = co * old.y - si * old.x;
		tw->world.points[s].orient.x = si * old.y + co * old.x;
	}
	tw->damageMe();
}

void rpzCB(Fl_Widget*, TrainWindow* tw)
{
	rollz(tw,1);
}
void rmzCB(Fl_Widget*, TrainWindow* tw)
{
	rollz(tw,-1);
}

// CVS Header - if you don't know what this is, don't worry about it
// This code tells us where the original came from in CVS
// Its a good idea to leave it as-is so we know what version of
// things you started with
// $Header: /p/course/cs559-gleicher/private/CVS/TrainFiles/CallBacks.cpp,v 1.5 2008/10/19 01:54:28 gleicher Exp $
