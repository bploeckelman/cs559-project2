/*
 * Callback functions for FlTk
 */
#include "Callback.h"
#include "Phase2Window.h"

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

		static const float rotationStep = 2.f;
		if( window->isAnimating() )
			window->setRotation(window->getRotation() + rotationStep);

		window->damageMe();
	}
}

void animateButtonCallback(Fl_Widget *widget, Phase2Window *window)
{
	if( window == nullptr || widget == nullptr )
		cout << "Error: damageCallback - null pointer passed." << endl;

	window->toggleAnimating();

	window->damageMe();
}
