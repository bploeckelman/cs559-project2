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


void idleCallback(void *pData)
{
	static const unsigned long interval = CLOCKS_PER_SEC / 30;
	static unsigned long lastRedraw = 0;

	assert(pData != nullptr);

	MainWindow *window = reinterpret_cast<MainWindow*>(pData);
	int segment = 0;

	const unsigned long delta = clock() - lastRedraw;
	if( delta > interval ) 
	{
		lastRedraw = clock();

		//the lower code here was put into mainwindow to bring it mor in line with the sample project

		//MAIN DIFFERENCE BETWEEN PERRY's CODE AND THE TRAIN DEMO CODE:
		//in the sample project they use arclength param to figure out the step applied to the current rotation value

		//in perry's code he uses arclength param to directly translate the thing being arclength param'd

		/*const float rotation = window->getRotation();
		const float rotationStep = window->getRotationStep();
		const float speed = window->getSpeed();

		if( window->isAnimating() )
		{
			if(window->isArcParam())
			{
				float newRotationStep = window->getView().arcLengthInterpolation(rotation +speed*0.5f, segment);    //.arcLengthInterpolation((rotation + speed*0.5f), segment);
				window->setRotation(rotation+ newRotationStep);
			}
			else
			{
				window->setRotation(rotation + (rotationStep*speed*0.5f));
			}
			
			if( window->getRotation() > window->getCurve().numSegments() )
				window->setRotation(0.f);
		}*/
		const int direction = 1;//new code, may be used for a future update where we can toggle a direction change

		window->advanceTrain(direction);//new code, may be used for a future update where we can toggle a direction change
		window->damageMe();
	}
}

void animateButtonCallback(Fl_Widget *widget, MainWindow *window)
{
	assert(window != nullptr && widget != nullptr);
	window->toggleAnimating();
	window->damageMe();
}

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

void curveTypeChoiceCallback( Fl_Widget *widget, MainWindow *window )
{
	assert(window != nullptr && widget != nullptr);

	Fl_Choice *curveTypeChoice = dynamic_cast<Fl_Choice*>(widget);
	CurveType  curveType = static_cast<CurveType>(curveTypeChoice->value());

	window->getCurve().setCurveType(curveType);

	window->damageMe();
}

void viewTypeChoiceCallback( Fl_Widget *widget, MainWindow *window )
{
	assert(window != nullptr && widget != nullptr);

	Fl_Choice *viewTypeChoice = dynamic_cast<Fl_Choice*>(widget);
	ViewType   viewType = static_cast<ViewType>(viewTypeChoice->value());

	window->getView().viewType = viewType;

	window->damageMe();
}

void shadowButtonCallback( Fl_Widget *widget, MainWindow *window )
{
	assert(window != nullptr && widget != nullptr);
	window->toggleShadows();
	window->damageMe();
}

void paramButtonCallback( Fl_Widget *widget, MainWindow *window )
{
	assert(window != nullptr && widget != nullptr);
	window->toggleArcParam();
	window->damageMe();
}

void forwardButtonCallback(Fl_Widget *widget, MainWindow *window)
{
	assert(window != nullptr && widget != nullptr);

	const float rotation = window->getRotation();
	const float rotationStep = window->getRotationStep();

	window->setRotation(rotation + rotationStep);
	if( window->getRotation() >= window->getCurve().numSegments() )
	{
		window->setRotation(0.f);
	}

	window->damageMe();
}

void backwardButtonCallback(Fl_Widget *widget, MainWindow *window)
{
	assert(window != nullptr && widget != nullptr);

	const float rotation = window->getRotation();
	const float rotationStep = window->getRotationStep();

	window->setRotation(rotation - rotationStep);
	if( window->getRotation() < 0.f )
	{
		window->setRotation(window->getCurve().numSegments()- rotationStep);
	}

	window->damageMe();
}

void speedSliderCallback(Fl_Widget *widget, MainWindow *window)
{
	assert(window != nullptr && widget != nullptr);

	Fl_Value_Slider *speedSlider = dynamic_cast<Fl_Value_Slider*>(widget);
	window->setSpeed((float)speedSlider->value());

	window->damageMe();
}

void highlightButtonCallback(Fl_Widget *widget, MainWindow *window)
{
	assert(window != nullptr && widget != nullptr);
	window->toggleHighlightSegPts();
	window->damageMe();
}
