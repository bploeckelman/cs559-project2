/*
 * CS559     - Train Project 
 * - Phase 2 - OpenGL Programming Signs of Life
 *
 * This program consists of:
 * - a basic OpenGL window 
 * - a collection of points 
 * - an animate mode, that when turned on 
 *   has another shape orbit a selected point
 * 
 * Authors: Brian Ploeckelman
 *          Matthew Bayer
 */
#include "Phase2Window.h"

#pragma warning(push)
#pragma warning(disable:4312)
#pragma warning(disable:4311)
#include <Fl/Fl.h>
#pragma warning(pop)

#include <iostream>


int main(int argc, char* argv[])
{
	std::cout << "CS559 - Project 2 - Phase 2" << std::endl;

	Phase2Window window;
	window.show();

	Fl::run();

	return 0;
}
