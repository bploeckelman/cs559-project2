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
#include <string>


int main(int argc, char* argv[])
{
	using std::cout;
	using std::endl;

	cout << "CS559 - Project 2 - Phase 6" << endl;
	if( argc > 3 )
	{
		cout << "Invalid number of arguments." << endl
			 << "usage: cs559-project2 [input-trackfile [output-trackfile]]"
			 << endl;
		system("PAUSE");
		return 0;
	}

	MainWindow window;

	if( argc > 1 )
	{
		const std::string inputTrackFilename(argv[1]);
		window.loadPoints(inputTrackFilename);
	}

	window.show();
	Fl::run();

	if( argc == 3 )
	{
		const std::string outputTrackFilename(argv[2]);
		window.savePoints(outputTrackFilename);
	}

	return 0;
}
