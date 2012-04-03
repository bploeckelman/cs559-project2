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
#include "highprecisiontime.h"

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
#include <Fl/Fl_Slider.h>
#include <Fl/Fl_Value_Slider.h>
#include <FL/fl_ask.h>
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

	void toggleUseArcball();
	void setSelectedPoint(const int p);
	void setWindow(MainWindow *w);

	int  getSelectedPoint() const;
	MainWindow* getWindow() const;

	float arcLengthInterpolation(double big_t, int& segment_number);

private:
	void resetArcball();
	void setupProjection();

	void updateTextWidget( const float t );
	void openglFrameSetup();

	void drawScenery(bool doShadows=false);
	void drawCurve(const float t, bool doShadows=false);
	void drawPathObject(const float t, bool doShadows=false);
	void drawPathObjects(const float t, const bool doShadows=false);
	void drawSelectedControlPoint(bool doShadows=false);

	//void reparameterizing(Curve& curve, float big_t, bool doShadows);
};

inline void MainView::toggleUseArcball()       { useArcball = !useArcball; }
inline void MainView::setSelectedPoint(int p)  { selectedPoint = p; }
inline void MainView::setWindow(MainWindow *w) { window = w; }
inline int  MainView::getSelectedPoint() const { return selectedPoint; }
inline MainWindow* MainView::getWindow() const { return window; }



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
	Fl_Button  *shadowButton;
	Fl_Button  *paramButton;
	Fl_Button  *forwardButton;
	Fl_Button  *backwardButton;
	Fl_Value_Slider  *speedSlider;

	Curve curve;

	bool animating;
	bool isArcLengthParam;
	bool shadows;

	float speed;
	float rotation;
	float rotationStep;

	//only use if using the HpTime to set up the big_t for arclength param
	//HighPrecisionTime hpTime;
	//double time_mode_started;

	void createWidgets();

public:
	MainWindow(const int x=100, const int y=50);

	const MainView& getView() const;

	float getSpeed()          const;
	float getRotation()       const;
	float getRotationStep()   const;
	Curve& getCurve();
	ControlPointVector& getPoints();

	void setSpeed(float spdAmt);
	void setRotation(float r);

	bool isAnimating() const;
	bool isArcParam()  const;
	bool isShadowed()  const;

	void toggleAnimating();
	void toggleArcParam();
	void toggleShadows();

	void resetPoints();
	void loadPoints(const std::string& filename);
	void savePoints(const std::string& filename);

	void advanceTrain(int dir);

	void damageMe();

	void setDebugText(const std::string& text);
};

inline const MainView& MainWindow::getView() const { return *view; }
inline float MainWindow::getSpeed()          const { return speed ; }
inline float MainWindow::getRotation()       const { return rotation; }
inline float MainWindow::getRotationStep()   const { return rotationStep; }
inline Curve& MainWindow::getCurve()               { return curve; }
inline ControlPointVector& MainWindow::getPoints() { return curve.getControlPoints(); }
inline void MainWindow::setSpeed(float s)    { speed = s; }
inline void MainWindow::setRotation(float r) { rotation = r; }
inline bool MainWindow::isAnimating() const  { return animating; }
inline bool MainWindow::isArcParam()  const  { return isArcLengthParam; }
inline bool MainWindow::isShadowed()  const  { return shadows; }
inline void MainWindow::toggleAnimating()    { animating = !animating; }
inline void MainWindow::toggleArcParam()     { isArcLengthParam = !isArcLengthParam; }
inline void MainWindow::toggleShadows()      { shadows = !shadows; }
