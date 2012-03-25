#pragma once
/*
 * CS559   - Train Project 
 * Phase 4 - OpenGL Programming Signs of Life
 *
 * The MainWindow class is a basic FlTk window.
 *
 * Authors: Brian Ploeckelman
 *          Matthew Bayer
 */
#include "Curve.h"
#include "CtrlPoint.h"

#include "TrainFiles/Utilities/ArcBallCam.H"

#pragma warning(push)
#pragma warning(disable:4312)
#pragma warning(disable:4311)
#include <Fl/Fl_Double_Window.h>
#include <Fl/Fl_Gl_Window.h>
#include <Fl/Fl_Group.h>
#include <Fl/Fl_Button.h>
#include <Fl/Fl_Output.h>
#include <Fl/Fl_Choice.h>
#pragma warning(pop)

#include <vector>

class MainWindow;


/* ==================================================================
 * MainView class
 *
 * This class is the main fltk OpenGL view for the program, 
 * it consists of a pointer to its fltk parent window
 * and it tracks which of a group of control points is selected
 * ==================================================================
 */
class MainView : public Fl_Gl_Window
{
private:
	MainWindow *window;
	int selectedPoint;

	ArcBallCam arcballCam;
	bool useArcball;

public:
	MainView(int x, int y, int w, int h, const char *l=0);

	virtual void draw();
	virtual int handle(int event);

	void pick();

	inline void setSelectedPoint(const int p) { selectedPoint = p; }
	inline int  getSelectedPoint() const      { return selectedPoint; }

	inline void setWindow(MainWindow *w) { window = w; }
	inline MainWindow* getWindow() const { return window; }

	inline void toggleUseArcball() { useArcball = !useArcball; }

private:
	void resetArcball();
	void setupProjection();

	void updateTextWidget( const float t );
	void openglFrameSetup();
	void drawFloor();
	void drawCurve( const float t );
	void drawPathObject( const float t );
	void drawSelectedControlPoint();
};


/* ==================================================================
 * MainWindow class
 *
 * This class is the main fltk window for the program, 
 * it consists of an fltk OpenGL view, fltk ui widgets, 
 * and a cubic curve.
 * ==================================================================
 */
class MainWindow : public Fl_Double_Window 
{
private:

	MainView *view;

	Fl_Group   *widgets;
	Fl_Button  *animateButton;
	Fl_Button  *addPointButton;
	Fl_Button  *delPointButton;
	Fl_Output  *textOutput;
	Fl_Choice  *curveTypeChoice;

	Curve curve;

	bool animating;
	float rotation;

	void createWidgets();
	void createPoints();

public:
	MainWindow(const int x=100, const int y=50);

	void damageMe();
	void setDebugText(const std::string& text);

	inline bool isAnimating() const { return animating; }
	inline void toggleAnimating()   { animating = !animating; }

	inline float getRotation() const  { return rotation; }
	inline void  setRotation(float r) { rotation = r; }

	inline const MainView& getView() const { return *view; }

	inline Curve& getCurve() { return curve; }
	inline std::vector<CtrlPoint>& getPoints() { return curve.getControlPoints(); }
};
