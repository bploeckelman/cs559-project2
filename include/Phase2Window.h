#pragma once
/*
 * CS559   - Train Project 
 * Phase 2 - OpenGL Programming Signs of Life
 *
 * The Phase2Window class is a basic FlTk window.
 *
 * Authors: Brian Ploeckelman
 *          Matthew Bayer
 */
#include "CtrlPoint.h"

#pragma warning(push)
#pragma warning(disable:4312)
#pragma warning(disable:4311)
#include <Fl/Fl_Double_Window.h>
#include <Fl/Fl_Gl_Window.h>
#include <Fl/Fl_Group.h>
#include <Fl/Fl_Button.h>
#pragma warning(pop)

#include <vector>

class Phase2Window;

class Phase2View : public Fl_Gl_Window
{
private:

	Phase2Window *window;
	int selectedPoint;

	void setupProjection();

public:

	Phase2View(int x, int y, int w, int h, const char *l=0);

	virtual int handle(int event);
	virtual void draw();

	void pick();

	inline void setWindow(Phase2Window *w)    { window = w; }
	inline void setSelectedPoint(const int p) { selectedPoint = p; }
};


class Phase2Window : public Fl_Double_Window 
{
private:

	// The opengl view
	Phase2View *view;

	// FlTk Widgets
	Fl_Group  *widgets;
	Fl_Button *animateButton;

	std::vector<CtrlPoint> points;

public:

	Phase2Window(const int x=50, const int y=50);

	void damageMe();

	inline std::vector<CtrlPoint>& getPoints() { return points; }

};
