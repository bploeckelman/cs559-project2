/*
 * CS559   - Train Project 
 * Phase 2 - OpenGL Programming Signs of Life
 *
 * The MainWindow class is a basic FlTk window.
 *
 * Authors: Brian Ploeckelman
 *          Matthew Bayer
 */
#include "MainWindow.h"
#include "MainView.h"

#include "Curve.h"
#include "Callback.h"
#include "MathUtils.h"

#pragma warning(push)
#pragma warning(disable:4312)
#pragma warning(disable:4311)
#pragma warning(disable:4099)
#pragma warning(disable:4996)
#include <Fl/fl.h>
#include <Fl/Fl_Double_Window.h>
#include <Fl/Fl_Box.h>
#include <Fl/Fl_Group.h>
#include <Fl/Fl_Button.h>
#include <Fl/Fl_Output.h>
#include <Fl/Fl_Choice.h>
#include <Fl/Fl_Slider.h>
#include <Fl/Fl_Value_Slider.h>
#include <Fl/Fl_ask.h>
#pragma warning(pop)

#include <iostream>
#include <fstream>
#include <cassert>
#include <sstream>
#include <string>
#include <vector>

using std::ifstream;
using std::ofstream;
using std::stringstream;
using std::string;
using std::vector;
using std::endl;


/* ==================================================================
 * MainWindow class
 * ==================================================================
 */

MainWindow::MainWindow(const int x, const int y) 
	: Fl_Double_Window(x, y, 800, 600, "Train Project - Phase 2")
	, view            (nullptr)
	, widgets         (nullptr)
	, animateButton   (nullptr)
	, addPointButton  (nullptr)
	, delPointButton  (nullptr)
	, textOutput      (nullptr)
	, curveTypeChoice (nullptr)
	, viewTypeChoice  (nullptr)
	, shadowButton    (nullptr)
	, paramButton     (nullptr)
	, forwardButton   (nullptr)
	, backwardButton  (nullptr)
	, speedSlider     (nullptr)
	, curve           (catmull)
	, animating       (false)
	, isArcLengthParam(false)
	, shadows         (true)
	, speed           (2.f)
	, rotation        (0.f)
	, rotationStep    (0.01f)
//  , time_mode_started(hpTime.TotalTime()) //only use if using the HpTime to set up the big_t for arclength param
{
	createWidgets();
	resetPoints();

	Fl::add_idle(idleCallback, this); 
}

/* setDebugText() - Called to update fltk multiline output text -- */
void MainWindow::setDebugText(const string& text)
{
	assert(textOutput != nullptr);
	textOutput->value("");
	textOutput->value(text.c_str());
}

/* createWidgets() - Called on construction to build fltk widgets  */
void MainWindow::createWidgets()
{
	// Make all the widgets
	begin();
	{
		// Create the OpenGL view
		view = new MainView(5, 5, 590, 590);
		view->setWindow(this);
		view->setSelectedPoint(0);
		//this->resizable(view);

		// Group widgets to help ease resizing
		widgets = new Fl_Group(600, 5, 190, 590); // x,y,w,h
		widgets->begin();

		// Create the animate button
		animateButton = new Fl_Button(605, 5, 60, 20, "Animate");
		animateButton->type(FL_TOGGLE_BUTTON);
		animateButton->value(0);                     // initially off
		animateButton->selection_color((Fl_Color)3); // yellow when pressed
		animateButton->callback((Fl_Callback*)animateButtonCallback, this);

		// Create the add control point button
		addPointButton = new Fl_Button(670, 5, 60, 20, "Add");
		addPointButton->type(FL_NORMAL_BUTTON);
		addPointButton->selection_color((Fl_Color)3); // yellow when pressed
		addPointButton->callback((Fl_Callback*)addPointButtonCallback, this);

		// Create the delete control point button
		delPointButton = new Fl_Button(735, 5, 60, 20, "Delete");
		delPointButton->type(FL_NORMAL_BUTTON);
		delPointButton->selection_color((Fl_Color)3); // yellow when pressed
		delPointButton->callback((Fl_Callback*)delPointButtonCallback, this);

		// Create curve type browser (drop down)
		curveTypeChoice = new Fl_Choice(605, 30, 90, 20);
		curveTypeChoice->clear();
		curveTypeChoice->add((CurveTypeNames[lines]).c_str());
		curveTypeChoice->add((CurveTypeNames[catmull]).c_str());
//		curveTypeChoice->add((CurveTypeNames[hermite]).c_str());
//		curveTypeChoice->add((CurveTypeNames[bspline]).c_str());
		curveTypeChoice->value(1);
		curveTypeChoice->callback((Fl_Callback*)curveTypeChoiceCallback, this);

		// Create view type browser (drop down)
		viewTypeChoice = new Fl_Choice(645, 55, 90, 20, "View");
		viewTypeChoice->clear();
		viewTypeChoice->add((ViewTypeNames[arcball]).c_str());
		viewTypeChoice->add((ViewTypeNames[train]).c_str());
		viewTypeChoice->add((ViewTypeNames[overhead]).c_str());
		viewTypeChoice->value(0);
		viewTypeChoice->callback((Fl_Callback*)viewTypeChoiceCallback, this);

		//create arc length parameterization button
		paramButton = new Fl_Button(605, 105, 120, 20, "Arclength Param");
		paramButton->type(FL_TOGGLE_BUTTON);
		paramButton->value(0);
		paramButton->selection_color((Fl_Color)3); // yellow when pressed
		paramButton->callback((Fl_Callback*)paramButtonCallback, this);

		//create arc length parameterization button
		shadowButton = new Fl_Button(700, 30, 90, 20, "Shadows");
		shadowButton->type(FL_TOGGLE_BUTTON);
		shadowButton->value(1);
		shadowButton->selection_color((Fl_Color)3); // yellow when pressed
		shadowButton->callback((Fl_Callback*)shadowButtonCallback, this);

		// Create the manual backwards button
		backwardButton = new Fl_Button(730, 105, 30, 20, "<<");
		backwardButton->type(FL_NORMAL_BUTTON);
		backwardButton->selection_color((Fl_Color)3); // yellow when pressed
		backwardButton->callback((Fl_Callback*)backwardButtonCallback, this);

		// Create the manual forwards button
		forwardButton = new Fl_Button(765, 105, 30, 20, ">>");
		forwardButton->type(FL_NORMAL_BUTTON);
		forwardButton->selection_color((Fl_Color)3); // yellow when pressed
		forwardButton->callback((Fl_Callback*)forwardButtonCallback, this);

		//create a speed slider
		speedSlider = new Fl_Value_Slider(645,80,140,20,"Speed");
		speedSlider->range(-10,10);
		speedSlider->value(2);
		speedSlider->align(FL_ALIGN_LEFT);
		speedSlider->type(FL_HORIZONTAL);
		speedSlider->callback((Fl_Callback*)speedSliderCallback, this);


		// Create text display
		textOutput = new Fl_Output(605, 130, 80, 20);

		// Create file write button
		/*writeButton = new Fl_Button(700, 105, 70, 20, "File Write");
		writeButton->type(FL_TOGGLE_BUTTON);
		writeButton->selection_color((Fl_Color)3); // yellow when pressed
		writeButton->callback((Fl_Callback*)writeButtonCallback, this);*/

		// Create a phantom widget to help resize things
		//this works with the train project because they have a World view as well as our trainview(mainview) view
		//TODO: fix?
		//Fl_Box *resizeBox = new Fl_Box(600, 595, 200, 5);
		//widgets->resizable(resizeBox);

		widgets->end();
	}
	end();
}

/* resetPoints() - Called to reset control points to a standard configuration */
void MainWindow::resetPoints()
{
	curve.clearPoints();

	const float step = TWO_PI / 5.f;
	const float radius = 30.f;
	for(float i = 0.f; i < TWO_PI; i += step)
	{
		const Vec3f     pos(cosf(i), 0.f, sinf(i));
		const CtrlPoint point(pos * radius);
		curve.addControlPoint(point);
	}
}

// the file format is simple
// first line: an integer with the number of control points
// other lines: one line per control point
// either 3 (X,Y,Z) numbers on the line, or 6 numbers (X,Y,Z, orientation)
void MainWindow::loadPoints(const string& filename)
{
	ifstream file(filename);
	if( file.is_open() )
	{
		// Get the number of points but don't worry about validating it,
		// since its not used and could be a lie anyways...
		int numPoints = 0;
		file >> numPoints;

		// Get the points
		curve.clearPoints();
		float px, py, pz;	// position
		float ox, oy, oz;	// orientation
		while( file >> px >> py >> pz >> ox >> oy >> oz )
		{
			const Vec3f position(px, py, pz);
			const Vec3f orientation(ox, oy, oz);
			curve.addControlPoint(CtrlPoint(position, orientation));
		}
	}
	else // file didn't open...
	{
		stringstream ss;
		ss << "Error - failed to open file: " << filename << endl
			<< "Using default control points instead." << endl;
		fl_alert(ss.str().c_str());
		resetPoints();
	}
}

void MainWindow::savePoints(const string& filename)
{
	ofstream file(filename);
	if( file.is_open() )
	{
		// Save number of points
		file << curve.numControlPoints();

		// Save each point's position and orientation
		for each(const auto& point in curve.getControlPoints())
		{
			const Vec3f& p(point.pos());
			const Vec3f& o(point.orient());

			file << endl;
			file << p.x() << " " << p.y() << " " << p.z() << " ";
			file << o.x() << " " << o.y() << " " << o.z() << " ";
		}
	}
	else // file didn't open...
	{
		stringstream ss;
		ss << "Error - failed to open file \"" << filename
			<< "\" for writing." << endl;
		fl_alert(ss.str().c_str());
	}
}

void MainWindow::advanceTrain(int dir)
{
	int segment = 0;
	float newRotationStep = 0.0;

	if( isAnimating() )
	{
		// Both branches of this if do the same thing right now?
		if(isArcParam())
		{
			//MAIN DIFFERENCE BETWEEN PERRY's CODE AND THE TRAIN DEMO CODE:
			//in the sample project they use arclength param to figure out the step applied to the current rotation value

			//in perry's code he uses arclength param to directly translate the thing being arclength param'd

			//the following code computes the step amount (big_t) to be interpolated with in regards to a high precision timer: hpTime
			/*double seconds_per_sample = 1;
			double flight_time = seconds_per_sample * curve.numSegments();
			double time_now = hpTime.TotalTime() - time_mode_started;
			printf("time_now is %f and time_mode started is %f \n", time_now, time_mode_started);
			double dmod = fmod(time_now, flight_time);
			printf("dmod is %f \n", dmod);
			double big_t = dmod / (flight_time);
			printf("big_t is %f \n", big_t);*/
			//newRotationStep = view->arcLengthInterpolation(big_t, segment); //actual arclengthparam function call here set up like in TrainWindow.cpp
			//newRotationStep = rotationStep*(speed*0.5f); //my attempt at fixes
			//newRotationStep = newRotationStep/curve.numSegments(); //my attempt at fixes
			//newRotationStep = newRotationStep/rotation;  //my attempt at fixes

			newRotationStep = rotationStep * speed * 0.5f;

			if(dir > 0)
				rotation += newRotationStep;
			else if(dir < 0)
				rotation -= newRotationStep; //eventually allow for going in the opposite direction

			//TODO: would need to add more conditionals here to eventually allow for going in the opposite direction
			if( rotation >= getCurve().numSegments() )
				rotation = 0.f;
			else if(rotation < 0.f)//assumes this condition is reached when we are still going in a pos dir (only reached when speed is negative)
				rotation= getCurve().numSegments() + newRotationStep;
		}
		else
		{
			newRotationStep = rotationStep * speed * 0.5f;

			if(dir > 0)
				rotation += newRotationStep;
			else if(dir < 0)
				rotation -= newRotationStep; //eventually allow for going in the opposite direction

			//TODO: would need to add more conditionals here to eventually allow for going in the opposite direction
			if( rotation >= getCurve().numSegments() )
				rotation = 0.f;
			else if(rotation < 0.f)//assumes this condition is reached when we are still going in a pos dir (only reached when speed is negative)
				rotation = getCurve().numSegments() + newRotationStep;
		}
	}
}

/* damageMe() - Called to force an update of the window ---------- */
void MainWindow::damageMe()
{
	view->damage(1);
}
