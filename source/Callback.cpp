/*
 * Callback functions for FlTk
 */
#include "Callback.h"
#include "MainWindow.h"
#include "Curve.h"
#include "CtrlPoint.h"
#include "Vec3f.h"

#pragma warning(push)
#pragma warning(disable:4312)
#pragma warning(disable:4311)
#include <Fl/Fl_Widget.h>
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

	const float x = static_cast<float>((rand() % 150) - 75);
	const float y = static_cast<float>((rand() % 50));
	const float z = static_cast<float>((rand() % 150) - 75);

	window->getCurve().addControlPoint(CtrlPoint(Vec3f(x,y,z)));

	// Reset t so we don't try to access out of bounds
	window->setRotation(0.f);

	window->damageMe();
}

/* delPointButtonCallback() - Called by fltk when the delete point button is pressed */
void delPointButtonCallback( Fl_Widget *widget, MainWindow *window )
{
	assert(window != nullptr && widget != nullptr);

	// Don't allow deletion of points if we are down to just 2
	if( window->getCurve().numControlPoints() <= 2 )
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
