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


void damageCB(Fl_Widget *widget, Phase2Window *window)
{
	if( window == nullptr || widget == nullptr )
		std::cout << "Error in damage callback, null pointer passed." << std::endl;

	window->damageMe();
}