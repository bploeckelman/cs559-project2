/*
 * Callback functions for FlTk
 */
#include "Callback.h"
#include "Phase2Window.h"
#include "Curve.h"

#pragma warning(push)
#pragma warning(disable:4312)
#pragma warning(disable:4311)
#include <Fl/Fl_Widget.h>
#pragma warning(pop)

#include <iostream>
#include <time.h>

using std::cout;
using std::endl;


void idleCallback(void *pData)
{
	static const unsigned long interval = CLOCKS_PER_SEC / 30;
	static unsigned long lastRedraw = 0;

	if( pData == nullptr ) 
	{ 
		cout << "Error: idleCallback - null pointer passed." << endl;
		return;
	}
	Phase2Window *window = reinterpret_cast<Phase2Window*>(pData);

	const unsigned long delta = clock() - lastRedraw;
	if( delta > interval ) 
	{
		lastRedraw = clock();

		static const float rotationStep = 0.01f;
		const float rotation = window->getRotation();

		if( window->isAnimating() )
		{
			window->setRotation(rotation + rotationStep);
			if( window->getRotation() > window->getCurve().numSegments() )
				window->setRotation(0.f);
		}

		window->damageMe();
	}
}

void animateButtonCallback(Fl_Widget *widget, Phase2Window *window)
{
	if( window == nullptr || widget == nullptr )
	{
		cout << "Error: damageCallback - null pointer passed." << endl;
		return;
	}

	window->toggleAnimating();

	window->damageMe();
}

void curveTypeChoiceCallback( Fl_Widget *widget, Phase2Window *window )
{
	if( window == nullptr || widget == nullptr )
	{
		cout << "Error: curveTypeBrowserCallback - null pointer passed." << endl;
		return;
	}

	Fl_Choice *curveTypeChoice = dynamic_cast<Fl_Choice*>(widget);
	CurveType curveType = static_cast<CurveType>(curveTypeChoice->value());

	window->getCurve().setCurveType(curveType);

	window->damageMe();
}
