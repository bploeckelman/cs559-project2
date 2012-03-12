// CS559 - Programming Assignment 3 - Trains and Roller Coasters
// Sample Code - written by Michael Gleicher, October 2008
//
// Note: this code is meant to serve as both the example solution, as well
// as the starting point for student projects. Therefore, some (questionable)
// design tradeoffs have been made - please see the ReadMe-Initial.txt file
//

// file Train.cpp - the "main file" and entry point for the train project
// this basically opens up the window and starts up the world

#include "stdio.h"

#include "TrainWindow.H"

#pragma warning(push)
#pragma warning(disable:4312)
#pragma warning(disable:4311)
#include <Fl/Fl.h>
#pragma warning(pop)

// Note : we don't want to use this entry point for phase-2
/*
int main(int, char**)
{
	printf("CS559 Train Assignment\n");

	TrainWindow tw;
	tw.show();

	Fl::run();
}
*/
// $Header: /p/course/cs559-gleicher/private/CVS/TrainFiles/Train.cpp,v 1.2 2008/10/14 02:52:12 gleicher Exp $