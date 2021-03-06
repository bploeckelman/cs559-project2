/*
 * Callback functions for FlTk
 */
#include "Callback.h"
#include "MainWindow.h"
#include "Curve.h"
#include "CtrlPoint.h"
#include "MathUtils.h"
#include "Vec3f.h"

#pragma warning(push)
#pragma warning(disable:4312)
#pragma warning(disable:4311)
#include <Fl/Fl_Widget.h>
#include <Fl/Fl_File_Chooser.h>
#include <Fl/fl_ask.h>
#pragma warning(pop)

#include <iostream>
#include <cassert>
#include <cstdlib>
#include <ctime>

using std::rand;
using std::cout;
using std::endl;


/* idleCallback() - Repeatedly called by fltk while idle --------- */
void idleCallback(void *pData)
{
	static const unsigned long interval = CLOCKS_PER_SEC / 30;
	static unsigned long lastRedraw = 0;

	assert(pData != nullptr);
	MainWindow *window = reinterpret_cast<MainWindow*>(pData);

	const unsigned long delta = clock() - lastRedraw;
	if( delta > interval ) 
	{
		if( window->isAnimating() )
		{
			lastRedraw = clock();
			window->advanceTrain();
			window->damageMe();
		}
	}
}

/* animateButtonCallback() - Called by fltk when the animate button is pressed */
void animateButtonCallback(Fl_Widget *widget, MainWindow *window)
{
	assert(window != nullptr && widget != nullptr);
	window->toggleAnimating();
	window->damageMe();
}

/* addPointButtonCallback() - Called by fltk when the add point button is pressed */
void addPointButtonCallback( Fl_Widget *widget, MainWindow *window )
{
	assert(window != nullptr && widget != nullptr);

	Curve&              curve  = window->getCurve();
	ControlPointVector& points = curve.getControlPoints();

	const int numPoints     = curve.numControlPoints();
	const int selectedIndex = window->getView().getSelectedPoint();

	const int addIndex  = (selectedIndex >= 0) ? selectedIndex : 0;
	const int prevIndex = (addIndex + numPoints - 1) % numPoints;

	const Vec3f prevPos = points[prevIndex].pos();
	const Vec3f addPos  = points[addIndex].pos();
	const Vec3f newPos  = 0.5f * (prevPos + addPos);

	// TODO: modify Curve::addControlPoint() to take an index to insert at
	points.insert(points.begin() + addIndex, CtrlPoint(newPos));

	curve.regenerateSegments();

	// Don't move the train unless it is affected by the new point
	float& t = window->getRotation();
	if( std::ceil(t) > static_cast<float>(addIndex) )
	{
		t += 1.f;
		if( t >= numPoints )
			t -= numPoints;
	}

	window->damageMe();
}

/* delPointButtonCallback() - Called by fltk when the delete point button is pressed */
void delPointButtonCallback( Fl_Widget *widget, MainWindow *window )
{
	assert(window != nullptr && widget != nullptr);

	// Don't allow deletion of points if we are down to just 4
	if( window->getCurve().numControlPoints() <= 4 )
		return;

	const int selected = window->getView().getSelectedPoint();
	try {
		window->getCurve().delControlPoint(selected);

		// Reset t so we don't try to access out of bounds
		window->setRotation(0.f);
	} catch(Curve::NoSuchPoint&) { }

	window->damageMe();
}

/* curveTypeChoiceCallback() - Called by fltk when the curve type drop box is changed */
void curveTypeChoiceCallback( Fl_Widget *widget, MainWindow *window )
{
	assert(window != nullptr && widget != nullptr);

	Fl_Choice *curveTypeChoice = dynamic_cast<Fl_Choice*>(widget);
	CurveType  curveType = static_cast<CurveType>(curveTypeChoice->value());

	window->getCurve().setCurveType(curveType);

	window->damageMe();
}

/* viewTypeChoiceCallback() - Called by fltk when the view type drop box is changed */
void viewTypeChoiceCallback( Fl_Widget *widget, MainWindow *window )
{
	assert(window != nullptr && widget != nullptr);

	Fl_Choice *viewTypeChoice = dynamic_cast<Fl_Choice*>(widget);
	ViewType   viewType = static_cast<ViewType>(viewTypeChoice->value());

	window->getView().viewType = viewType;

	window->damageMe();
}

/* shadowButtonCallback() - Called by fltk when the toggle shadow button is pressed */
void shadowButtonCallback( Fl_Widget *widget, MainWindow *window )
{
	assert(window != nullptr && widget != nullptr);
	window->toggleShadows();
	window->damageMe();
}

/* paramButtonCallback() - Called by fltk when the toggle arc-length param button is pressed */
void paramButtonCallback( Fl_Widget *widget, MainWindow *window )
{
	assert(window != nullptr && widget != nullptr);
	window->toggleArcParam();
	window->damageMe();
}

/* forwardButtonCallback() - Called by fltk when the move forward button is pressed */
void forwardButtonCallback(Fl_Widget *widget, MainWindow *window)
{
	assert(window != nullptr && widget != nullptr);
	window->advanceTrain(2);
	window->damageMe();
}

/* backwardButtonCallback() - Called by fltk when the move backward button is pressed */
void backwardButtonCallback(Fl_Widget *widget, MainWindow *window)
{
	assert(window != nullptr && widget != nullptr);
	window->advanceTrain(-2);
	window->damageMe();
}

/* speedSliderCallback() - Called by fltk when the speed slider is moved */
void speedSliderCallback(Fl_Widget *widget, MainWindow *window)
{
	assert(window != nullptr && widget != nullptr);

	Fl_Value_Slider *speedSlider = dynamic_cast<Fl_Value_Slider*>(widget);
	window->setSpeed((float)speedSlider->value());

	window->damageMe();
}

/* highlightButtonCallback() - Called by fltk when the toggle highlights button is pressed */
void highlightButtonCallback(Fl_Widget *widget, MainWindow *window)
{
	assert(window != nullptr && widget != nullptr);
	window->toggleHighlightSegPts();
	window->damageMe();
}

/* resetPointsButtonCallback() - Called by fltk when the reset button is pressed */
void resetPointsButtonCallback( Fl_Widget *widget, MainWindow *window )
{
	assert(window != nullptr && widget != nullptr);
	window->resetPoints();
	window->setRotation(0.f);
	window->damageMe();
}

/* loadPointsButtonCallback() - Called by fltk when the load button is pressed */
void loadPointsButtonCallback( Fl_Widget *widget, MainWindow *window )
{
	assert(window != nullptr && widget != nullptr);
	const char *filename = fl_file_chooser("Pick a track file", "*.txt", "tracks/reset.txt");
	if( filename != nullptr )
	{
		window->loadPoints(filename);
		window->getCurve().regenerateSegments();
		window->setRotation(0.f);
		window->damageMe();
	}
}

/* savePointsButtonCallback() - Called by fltk when the save button is pressed */
void savePointsButtonCallback( Fl_Widget *widget, MainWindow *window )
{
	assert(window != nullptr && widget != nullptr);
	const char *filename = fl_input("File name for save [*.txt]", "tracks/");
	if( filename != nullptr )
	{
		window->savePoints(filename);
		window->getCurve().regenerateSegments();
		window->setRotation(0.f);
		window->damageMe();
	}
}

// ---------------------------------------------------------------

/* pitch() - A helper function for pitching a control point (around x) */
void pitch(MainWindow *window, const float dir=1)
{
	Curve& curve = window->getCurve();

	const int selected = window->getView().getSelectedPoint(); //curve.selectedPoint;
	if( selected < 0 || selected >= curve.numControlPoints() )
		return;

	const Vec3f oldOrient(curve.getPoint(selected).orient());
	const float s = sin(QUAR_PI * dir);
	const float c = cos(QUAR_PI * dir);

	curve.getPoint(selected)
		 .orient(Vec3f(oldOrient.x()
					,  c * oldOrient.y() - s * oldOrient.z()
					,  s * oldOrient.y() + c * oldOrient.z()));

	curve.regenerateSegments();

	window->damageMe();
}

/* roll() - A helper function for rolling a control point (around z) */
void roll(MainWindow *window, const float dir=1)
{
	Curve& curve = window->getCurve();

	const int selected = window->getView().getSelectedPoint(); //curve.selectedPoint;
	if( selected < 0 || selected >= curve.numControlPoints() )
		return;

	const Vec3f oldOrient(curve.getPoint(selected).orient());
	const float s = sin(QUAR_PI * dir);
	const float c = cos(QUAR_PI * dir);

	curve.getPoint(selected)
		 .orient(Vec3f(s * oldOrient.y() + c * oldOrient.x()
					,  c * oldOrient.y() - s * oldOrient.x()
					,  oldOrient.z()));

	curve.regenerateSegments();

	window->damageMe();
}

/* pointResetButtonCallback() - Called by fltk when the reset orientation button is pressed */
void pointResetButtonCallback( Fl_Widget *widget, MainWindow *window )
{
	Curve& curve = window->getCurve();

	if( window->getView().viewType == train )
		return;

	const int selected = window->getView().getSelectedPoint(); //curve.selectedPoint;
	if( selected < 0 || selected >= curve.numControlPoints() )
		return;

	curve.getPoint(selected).orient(Vec3f(0.f, 1.f, 0.f));

	curve.regenerateSegments();

	window->damageMe();
}

/* pointPitchMoreButtonCallback() - Called by fltk when the pitch+ button is pressed */
void pointPitchMoreButtonCallback( Fl_Widget *widget, MainWindow *window )
{
	if( window->getView().viewType != train )
		pitch(window, 1.f);
}

/* pointPitchLessButtonCallback() - Called by fltk when the pitch- button is pressed */
void pointPitchLessButtonCallback( Fl_Widget *widget, MainWindow *window )
{
	if( window->getView().viewType != train )
		pitch(window, -1.f);
}

/* pointRollMoreButtonCallback() - Called by fltk when the roll+ button is pressed */
void pointRollMoreButtonCallback( Fl_Widget *widget, MainWindow *window )
{
	if( window->getView().viewType != train )
		roll(window, 1.f);
}

/* pointRollLessButtonCallback() - Called by fltk when the roll- button is pressed */
void pointRollLessButtonCallback( Fl_Widget *widget, MainWindow *window )
{
	if( window->getView().viewType != train )
		roll(window, -1.f);
}

/* tensionSliderCallback() - Called by fltk when the tension slider is moved */
void tensionSliderCallback( Fl_Widget *widget, MainWindow *window )
{
	Fl_Value_Slider *tensionSlider = dynamic_cast<Fl_Value_Slider*>(widget);
	window->getCurve().tension = static_cast<float>(tensionSlider->value());

	window->damageMe();
}
