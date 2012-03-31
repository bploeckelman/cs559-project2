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
#include<stdio.h>
#include<string>
#include<sstream>


int main(int argc, char* argv[])
{
	std::cout << "CS559 - Project 2 - Phase 6" << std::endl;
	if(argc < 1 || argc > 3){
		std::cout << "INCORRECT NUMBER OF ARGUMENTS!" << std::endl;
		system ("PAUSE");
		exit(0);
	}

	std::string input;
	std::string output;

	if(argc == 3)
	{
		input = argv[1];
		output = argv[2];
	}
	else if (argc == 2)
	{
		input = argv[1];
	}

	MainWindow window;
	if(argc == 2 || argc == 3)
	{
		window.createPoints(input.c_str());
	}

	window.show();
	Fl::run();

	if(argc == 3)
	{
		window.writePoints(output.c_str());
	}

	return 0;
}
