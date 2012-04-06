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
	, textOutput1     (nullptr)
	, curveTypeChoice (nullptr)
	, viewTypeChoice  (nullptr)
	, shadowButton    (nullptr)
	, paramButton     (nullptr)
	, forwardButton   (nullptr)
	, backwardButton  (nullptr)
	, highlightButton (nullptr)
	, resetPointButton(nullptr)
	, loadPointsButton(nullptr)
	, savePointsButton(nullptr)
	, speedSlider     (nullptr)
	, curve           (catmull)
	, animating       (false)
	, isArcLengthParam(true)
	, highlightSegPts (false)
	, shadows         (true)
	, speed           (2.f)
	, rotation        (0.f)
	, rotationStep    (0.01f)
{
	createWidgets();
	resetPoints();

	Fl::add_idle(idleCallback, this); 
}

/* setDebugText() - Called to update fltk multiline output text -- */
void MainWindow::setDebugText( const string& text, const string& text1 )
{
	assert(textOutput != nullptr);
	textOutput->value("");
	textOutput->value(text.c_str());

	if( !text1.empty() )
	{
		assert(textOutput1 != nullptr);
		textOutput1->value("");
		textOutput1->value(text1.c_str());
	}
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
		paramButton->value(1);
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

		// Create text displays
		textOutput  = new Fl_Output(605, 130, 90, 20);
		textOutput1 = new Fl_Output(700, 130, 90, 20);

		//Create a button for highlighting the curve's current segment pts being used
		highlightButton = new Fl_Button(605, 155, 180, 20, "Highlight Current Segment");
		highlightButton->type(FL_TOGGLE_BUTTON);
		highlightButton->value(0);                     // initially off
		highlightButton->selection_color((Fl_Color)3); // yellow when pressed
		highlightButton->callback((Fl_Callback*)highlightButtonCallback, this);

		// Create a button to reset the curve's control points
		resetPointButton = new Fl_Button(605, 180, 50, 20, "Reset");
		resetPointButton->type(FL_NORMAL_BUTTON);
		resetPointButton->selection_color((Fl_Color)3);
		resetPointButton->callback((Fl_Callback*)resetPointButtonCallback, this);

		// Create a button to load control points from a file
		loadPointsButton = new Fl_Button(660, 180, 50, 20, "Load");
		loadPointsButton->type(FL_NORMAL_BUTTON);
		loadPointsButton->selection_color((Fl_Color)3);
		loadPointsButton->callback((Fl_Callback*)loadPointsButtonCallback, this);

		// Create a button to save control points to a file
		savePointsButton = new Fl_Button(715, 180, 50, 20, "Save");
		savePointsButton->type(FL_NORMAL_BUTTON);
		savePointsButton->selection_color((Fl_Color)3);
		savePointsButton->callback((Fl_Callback*)savePointsButtonCallback, this);

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

	curve.regenerateSegments();
}

/* loadPoints() - Loads control points from a text file ---------- */
void MainWindow::loadPoints(const string& filename)
{
	/* File Format:
	 * ------------
	 * [1]   - number of control points (integer)
	 * [2..] - control point position and orientation
	 *         (3 floats each, separated by spaces)
	 */
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

/* savePoints() - Saves the control points to a text file -------- */
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

/* advanceTrain() - Moves the train in the specified direction --- */
void MainWindow::advanceTrain(int dir)
{
	if( isArcLengthParam )
	{
		const float vel = dir * speed * 0.07f;
		rotation += arcLengthStep(vel);
	}
	else
	{
		rotation += dir * speed * 0.01f;
	}

	if( rotation >= curve.numControlPoints() )
		rotation = 0.f;
	if( rotation < 0 )
		rotation += curve.numControlPoints();
}

/* damageMe() - Called to force an update of the window ---------- */
void MainWindow::damageMe()
{
	view->damage(1);
}

/* arcLengthStep() - Calculates the arc-length step value for animation */
float MainWindow::arcLengthStep(const float vel)
{
	float next_t = rotation + 0.1f;
	if( next_t >= curve.numSegments() )
		next_t -= curve.numSegments();

	const Vec3f thisPoint(curve.getPosition(rotation));
	const Vec3f nextPoint(curve.getPosition(next_t));

	const Vec3f distance(nextPoint - thisPoint);

	return (vel / distance.magnitude());
}
