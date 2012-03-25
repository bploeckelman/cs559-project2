/*
 * CS559     - Train Project 
 * - Phase 4 - OpenGL Programming Signs of Life
 *
 * This program consists of:
 * - a basic OpenGL window 
 * - a collection of control points defining a cubic curve
 * - an animate mode, that when turned on 
 *   has a cube move along the curve
 * 
 * Authors: Brian Ploeckelman
 *          Matthew Bayer
 */
#include "MainWindow.h"

#pragma warning(push)
#pragma warning(disable:4312)
#pragma warning(disable:4311)
#pragma warning(disable:4099)
#include <Fl/Fl.h>
#pragma warning(pop)

#include <iostream>


int main(int argc, char* argv[])
{
	std::cout << "CS559 - Project 2 - Phase 4" << std::endl;

	MainWindow window;
	window.show();

	Fl::run();

	return 0;
}
