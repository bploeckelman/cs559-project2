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
#include "highPrecisionTime.h"

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

	//GLuint arc_length_display_list;
	HighPrecisionTime hpTime;
	float time_mode_started;

	struct ParameterTable
	{
		float local_t;
		float accumulated_length;
		float fraction_of_accumulated_length;
		int segment_number;
	};

	std::vector<ParameterTable> parameter_table;

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
	float arcLengthInterpolation(float big_t, int& segment_number);
	void BuildParameterTable(int number_of_samples, Curve& curve);

private:
	void resetArcball();
	void setupProjection();

	void updateTextWidget( const float t );
	void openglFrameSetup();
	void drawScenery(bool doShadows);
	void drawCurve( const float t, bool doShadows);
	void drawPathObject( const float t, bool doShadows );
	void drawSelectedControlPoint(bool doShadows);
	void reparameterizing(Curve& curve);
	

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
	Fl_Button  *shadowButton;
	Fl_Button  *paramButton;
	Fl_Button  *forwardButton;
	Fl_Button  *backwardButton;
	Fl_Value_Slider  *speedSlider;


	Curve curve;

	bool animating;
	bool isArcLengthParam;
	bool shadows;
	float rotationStep;
	float speed;
	float rotation;

	void createWidgets();
	void readPoints(const char* filename);

public:
	MainWindow(const int x=100, const int y=50);

	void damageMe();
	void setDebugText(const std::string& text);

	inline bool isAnimating() const { return animating; }
	inline void toggleAnimating()   { animating = !animating; }

	inline float getRotation() const  { return rotation; }
	inline void  setRotation(float r) { rotation = r; }
	inline float getRotationStep() { return rotationStep;}

	inline const MainView& getView() const { return *view; }

	inline Curve& getCurve() { return curve; }
	inline std::vector<CtrlPoint>& getPoints() { return curve.getControlPoints(); }

	inline void toggleArcParam() { isArcLengthParam = !isArcLengthParam;}
	inline bool isArcParam() { return isArcLengthParam;}

	inline void toggleShadows() { shadows = !shadows;}
	inline bool isShadowed() { return shadows; }

	inline void setSpeed(float spdAmt) { speed=spdAmt;}
	inline float getSpeed() {return speed;}

	void createPoints(const char* filename);
	void writePoints(const char* filename);

	inline void breakString(char* str, std::vector<const char*>& words) {
	// start with no words
	words.clear();

	// scan through the string, starting at the beginning
	char* p = str;

	// stop when we hit the end of the string
	while(*p) {
		// skip over leading whitespace - stop at the first character or end of string
		while (*p && *p<=' ') p++;

		// now we're pointing at the first thing after the spaces
		// make sure its not a comment, and that we're not at the end of the string
		// (that's actually the same thing)
		if (! (*p) || *p == '#')
		break;

		// so we're pointing at a word! add it to the word list
		words.push_back(p);

		// now find the end of the word
		while(*p > ' ') p++;	// stop at space or end of string

		// if its the end of the string, we're done
		if (! *p) break;

		// otherwise, turn this space into and end of string (to end the word)
		// and keep going
		*p = 0;
		p++;
	}
}

};
