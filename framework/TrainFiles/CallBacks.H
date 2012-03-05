#pragma once
// CS559 Train Project - 
// Header file to define callback functions.
// define the callbacks for the TrainWindow
//
// these are little functions that get called when the various widgets
// get accessed (or the fltk timer ticks). these functions are used 
// when TrainWindow sets itself up.
//
// written by Mike Gleicher, October 2008
//

class TrainWindow;
class Fl_Widget;

void resetCB(Fl_Widget*, TrainWindow* tw);
void damageCB(Fl_Widget*, TrainWindow* tw);

// Callback that adds a new point to the spline
// idea: add the point AFTER the selected point
void addPointCB(Fl_Widget*, TrainWindow* tw);
// Callback that deletes a point from the spline
void deletePointCB(Fl_Widget*, TrainWindow* tw);

// Callbacks for advancing/pulling back train
void forwCB(Fl_Widget*, TrainWindow* tw);
void backCB(Fl_Widget*, TrainWindow* tw);

// idle callback
void runButtonCB(TrainWindow* tw);

// for load and save buttons
void loadCB(Fl_Widget*, TrainWindow* tw);
void saveCB(Fl_Widget*, TrainWindow* tw);

// roll the control points
void rpxCB(Fl_Widget*, TrainWindow* tw);
void rmxCB(Fl_Widget*, TrainWindow* tw);
void rpzCB(Fl_Widget*, TrainWindow* tw);
void rmzCB(Fl_Widget*, TrainWindow* tw);

// CVS Header - if you don't know what this is, don't worry about it
// This code tells us where the original came from in CVS
// Its a good idea to leave it as-is so we know what version of
// things you started with
// $Header: /p/course/cs559-gleicher/private/CVS/TrainFiles/CallBacks.H,v 1.4 2008/10/19 01:54:28 gleicher Exp $
