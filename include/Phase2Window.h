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

#include "TrainFiles/Utilities/ArcBallCam.H"

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


/* ==================================================================
 * Phase2View class
 *
 * This class is the main fltk OpenGL view for the program, 
 * it consists of a pointer to its fltk parent window
 * and it tracks which of a group of control points is selected
 * ==================================================================
 */
class Phase2View : public Fl_Gl_Window
{
private:

	Phase2Window *window;
	int selectedPoint;

	ArcBallCam arcballCam;
	bool useArcball;

	void resetArcball();
	void setupProjection();

public:

	Phase2View(int x, int y, int w, int h, const char *l=0);

	virtual int handle(int event);
	virtual void draw();

	void pick();

	inline void toggleUseArcball() { useArcball = !useArcball; }

	inline void setWindow(Phase2Window *w)    { window = w; }
	inline void setSelectedPoint(const int p) { selectedPoint = p; }

	inline Phase2Window* getWindow() const { return window; }
	inline int getSelectedPoint()    const { return selectedPoint; }
};


/* ==================================================================
 * Phase2Window class
 *
 * This class is the main fltk window for the program, 
 * it consists of an fltk OpenGL view, fltk ui widgets, 
 * and a group of control points.
 * ==================================================================
 */
class Phase2Window : public Fl_Double_Window 
{
private:

	Phase2View *view;

	Fl_Group  *widgets;
	Fl_Button *animateButton;

	std::vector<CtrlPoint> points;

	bool animating;
	float rotation;

public:

	Phase2Window(const int x=100, const int y=50);

	void damageMe();

	inline bool isAnimating() const { return animating; }
	inline void toggleAnimating()   { animating = !animating; }

	inline float getRotation() const  { return rotation; }
	inline void  setRotation(float r) { rotation = r; }

	inline std::vector<CtrlPoint>& getPoints() { return points; }
};
