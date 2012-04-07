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
#include "MainView.h"
#include "Curve.h"

#pragma warning(push)
#pragma warning(disable:4312)
#pragma warning(disable:4311)
#include <Fl/Fl_Double_Window.h>
#include <Fl/Fl_Group.h>
#include <Fl/Fl_Button.h>
#include <Fl/Fl_Output.h>
#include <Fl/Fl_Choice.h>
#include <Fl/Fl_Slider.h>
#include <Fl/Fl_Value_Slider.h>
#include <FL/fl_ask.h>
#pragma warning(pop)


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
	Fl_Output  *textOutput1;
	Fl_Choice  *curveTypeChoice;
	Fl_Choice  *viewTypeChoice;
	Fl_Button  *shadowButton;
	Fl_Button  *paramButton;
	Fl_Button  *forwardButton;
	Fl_Button  *backwardButton;
	Fl_Button  *highlightButton;
	Fl_Button  *resetPointsButton;
	Fl_Button  *loadPointsButton;
	Fl_Button  *savePointsButton;
	Fl_Button  *pointResetButton;
	Fl_Button  *pointPitchMoreButton;
	Fl_Button  *pointRollMoreButton;
	Fl_Button  *pointPitchLessButton;
	Fl_Button  *pointRollLessButton;
	Fl_Value_Slider *speedSlider;
	Fl_Value_Slider *tensionSlider;

	Curve curve;

	bool animating;
	bool isArcLengthParam;
	bool shadows;
	bool highlightSegPts;

	float speed;
	float rotation;
	float rotationStep;

	void createWidgets();
	float arcLengthStep(const float vel=1.f);

public:
	friend class MainView;

	MainWindow(const int x=100, const int y=50);

	MainView& getView();
	const MainView& getView() const;

	float getSpeed()          const;
	float getRotation()       const;
	float& getRotation();
	float getRotationStep()   const;
	Curve& getCurve();
	ControlPointVector& getPoints();

	void setSpeed(float spdAmt);
	void setRotation(float r);
	void setViewType(int view);

	bool isAnimating() const;
	bool isArcParam()  const;
	bool isShadowed()  const;
	bool isHighlightedSegPts() const;

	void toggleAnimating();
	void toggleArcParam();
	void toggleShadows();
	void toggleHighlightSegPts();

	void resetPoints();
	void loadPoints(const std::string& filename);
	void savePoints(const std::string& filename);

	void advanceTrain(int dir=1);

	void damageMe();

	void setDebugText(const std::string& text, const std::string& text1="");
};

inline MainView& MainWindow::getView()             { return *view; }
inline const MainView& MainWindow::getView() const { return *view; }
inline float MainWindow::getSpeed()          const { return speed ; }
inline float MainWindow::getRotation()       const { return rotation; }
inline float& MainWindow::getRotation()            { return rotation; }
inline float MainWindow::getRotationStep()   const { return rotationStep; }
inline Curve& MainWindow::getCurve()               { return curve; }
inline ControlPointVector& MainWindow::getPoints() { return curve.getControlPoints(); }
inline void MainWindow::setSpeed(float s)    { speed = s; }
inline void MainWindow::setRotation(float r) { rotation = r; }
inline bool MainWindow::isAnimating() const  { return animating; }
inline bool MainWindow::isArcParam()  const  { return isArcLengthParam; }
inline bool MainWindow::isShadowed()  const  { return shadows; }
inline bool MainWindow::isHighlightedSegPts() const { return highlightSegPts; }
inline void MainWindow::toggleAnimating()    { animating = !animating; }
inline void MainWindow::toggleArcParam()     { isArcLengthParam = !isArcLengthParam; }
inline void MainWindow::toggleShadows()      { shadows = !shadows; }
inline void MainWindow::toggleHighlightSegPts()     { highlightSegPts = !highlightSegPts; }
inline void MainWindow::setViewType(int view) {viewTypeChoice->value(view);}
