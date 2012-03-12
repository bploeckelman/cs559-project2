/*
 * CtrlPoint.cpp
 */
#include "CtrlPoint.h"


void CtrlPoint::draw() const
{
	// TODO
}

bool CtrlPoint::operator==(const CtrlPoint& other)
{
	// TODO : would a compare by address work?
	// or should we compare by pos and orient? (once they are in place)
	return (this == &other) ? true : false;
}