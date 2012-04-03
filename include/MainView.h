#pragma once
/*
 * CS559    - Train Project
 *
 * The MainView class is a fltk OpenGL window
 *
 * Authors: Brian Ploeckelman
 *          Matthew Bayer
 */
#include "TrainFiles/Utilities/ArcBallCam.H"

#pragma warning(push)
#pragma warning(disable:4312)
#pragma warning(disable:4311)
#include <Fl/Fl_Gl_Window.h>
#pragma warning(pop)

#include <string>

class MainWindow;

enum ViewType { arcball, train, overhead };
extern std::string ViewTypeNames[];


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
	MainWindow    *window;
	ArcBallCam    arcballCam;

	int selectedPoint;	// TODO: only use the value in window->curve

public:
	ViewType viewType;

	MainView(int x, int y, int w, int h, const char *l=0);

	virtual void draw();
	virtual int handle(int event);

	void pick();

	void setWindow(MainWindow *w);
	void setSelectedPoint(const int p);

	MainWindow* getWindow() const;
	int  getSelectedPoint() const;

	float arcLengthInterpolation(double big_t, int& segment_number);

private:
	void resetArcball();
	void setupProjection();

	void updateTextWidget( const float t );
	void openglFrameSetup();

	void drawScenery(bool doShadows=false);
	void drawCurve(const float t, bool drawPoints=false, bool doShadows=false);
	void drawPathObject(const float t, bool doShadows=false);
	void drawPathObjects(const float t, const bool doShadows=false);
	void drawSelectedControlPoint(bool doShadows=false);

	//void reparameterizing(Curve& curve, float big_t, bool doShadows);
};

inline void MainView::setWindow(MainWindow *w) { window = w; }
inline void MainView::setSelectedPoint(int p)  { selectedPoint = p; }
inline MainWindow* MainView::getWindow() const { return window; }
inline int  MainView::getSelectedPoint() const { return selectedPoint; }
