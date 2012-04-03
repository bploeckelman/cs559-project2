/*
 * CS559   - Train Project 
 * Phase 2 - OpenGL Programming Signs of Life
 *
 * The MainWindow class is a basic FlTk window.
 *
 * Authors: Brian Ploeckelman
 *          Matthew Bayer
 */
#include "MainWindow.h"
#include "Callback.h"
#include "CtrlPoint.h"
#include "MathUtils.h"
#include "GLUtils.h"
#include "highPrecisionTime.h"

#include "TrainFiles/Utilities/ArcBallCam.H"
#include "TrainFiles/Utilities/3DUtils.h"

#include <windows.h>
#define WIN32_LEAN_AND_MEAN

#include <GL/GL.h>
#include <GL/GLU.h>

#pragma warning(push)
#pragma warning(disable:4312)
#pragma warning(disable:4311)
#pragma warning(disable:4099)
#pragma warning(disable:4996)
#include <Fl/fl.h>
#include <Fl/Fl_Double_Window.h>
#include <Fl/Fl_Gl_Window.h>
#include <Fl/Fl_Box.h>
#include <Fl/Fl_Group.h>
#include <Fl/Fl_Button.h>
#include <Fl/Fl_Output.h>
#include <Fl/Fl_Choice.h>
#include <Fl/Fl_Slider.h>
#include <Fl/Fl_Value_Slider.h>
#include <Fl/Fl_ask.h>
#include <Fl/glut.h>		// for primitive drawing
#pragma warning(pop)

#include <iostream>
#include <fstream>
#include <cassert>
#include <sstream>
#include <string>
#include <vector>

using std::ifstream;
using std::ofstream;
using std::stringstream;
using std::string;
using std::vector;
using std::cout;
using std::endl;


/* ==================================================================
 * MainView class
 * ==================================================================
 */

MainView::MainView(int x, int y, int w, int h, const char *l)
	: Fl_Gl_Window(x,y,w,h,l)
	, arcballCam()
{
	mode( FL_RGB | FL_ALPHA | FL_DEPTH | FL_DOUBLE );
	resetArcball();
}

/* draw() - Draws to the screen ---------------------------------- */
void MainView::draw()
{
	const float t = window->getRotation();

	updateTextWidget(t);
	openglFrameSetup();

	// Draw everything once without shadows
	drawScenery();
	drawCurve(t);
	drawPathObjects(t);
	drawSelectedControlPoint();

	// Draw everything again with shadows if they are enabled
	if(window->isShadowed())
	{
		glPushMatrix();
			// Translate down to the ground plane
			glTranslatef(0.f, -20.f, 0.f); 

			setupShadows();
		
			drawScenery(true);
			drawCurve(t, true);
			drawPathObjects(t, true);
			drawSelectedControlPoint(true);

			unsetupShadows();
		glPopMatrix();
	}
}

/* handle() - Handles user input --------------------------------- */
int MainView::handle(int event)
{
	if( useArcball ) 
	{
		if( arcballCam.handle(event) )
			return 1;
	}

	// Remember what button was used
	static int lastPush;

	switch(event)
	{
	case FL_PUSH:
		lastPush = Fl::event_button();
		if( lastPush == 1 )
		{
			pick();
			damage(1);
			return 1;
		}
		break;
	case FL_RELEASE:
		damage(1);
		lastPush = 0;
		return 1;
	case FL_DRAG:
		if ((lastPush == 1) && (selectedPoint >=0)) 
		{
			try {
				CtrlPoint& cp = window->getPoints().at(selectedPoint);

				double r1x, r1y, r1z, r2x, r2y, r2z;
				getMouseLine(r1x,r1y,r1z, r2x,r2y,r2z);

				double rx, ry, rz;
				mousePoleGo(r1x,r1y,r1z, r2x,r2y,r2z, 
					static_cast<double>(cp.pos().x()), 
					static_cast<double>(cp.pos().y()),
					static_cast<double>(cp.pos().z()),
					rx, ry, rz,
					(Fl::event_state() & FL_CTRL) != 0);

				cp.pos().x(static_cast<float>(rx));
				cp.pos().y(static_cast<float>(ry));
				cp.pos().z(static_cast<float>(rz));

				window->getCurve().regenerateSegments();

				damage(1);
			} catch(std::out_of_range&) {}
		}
		break;
	case FL_FOCUS:
		return 1;
	case FL_ENTER: // take focus anytime mouse enters window
		focus(this);
		break;
	case FL_KEYBOARD : 
		//* Note : use this format to process keyboard input
		int k  = Fl::event_key();
		int ks = Fl::event_state();
		if( k == ' ' )
		{
			toggleUseArcball();
		}
		break;
	}

	return Fl_Gl_Window::handle(event);
}

/* pick() - Performs OpenGL picking ------------------------------ */
void MainView::pick()
{
	// Make sure we're current so we can use OpenGL
	make_current();

	// Get the mouse position
	int mx = Fl::event_x();
	int my = Fl::event_y();

	// Get the viewport
	int viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	// Setup the pick matrix on the stack
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPickMatrix((double)mx, (double)(viewport[3] - my), 5, 5, viewport);

	setupProjection();

	// Draw objects (but only what has been 'hit')
	GLuint buf[100];
	glSelectBuffer(100, buf);
	glRenderMode(GL_SELECT);
	glInitNames();
	glPushName(0);

	vector<CtrlPoint>::iterator it  = window->getPoints().begin();
	vector<CtrlPoint>::iterator end = window->getPoints().end();
	for(int i = 0; it != end; ++it, ++i)
	{
		glLoadName((GLuint)(i + 1));
		it->draw(false); //dont draw shadows for the hit object, let mainView->Draw() do that
	}

	// See how picking did back in drawing mode
	int hits = glRenderMode(GL_RENDER);
	selectedPoint = hits ? buf[3] - 1 : -1;

	if( selectedPoint != -1 )
	{
		try { 
			const CtrlPoint p(window->getPoints().at(selectedPoint));
			cout << "Selected: "   << selectedPoint 
				 << " at "         << p.pos()  
				 << "oriented to " << p.orient() 
				 << endl; 
		} catch(std::out_of_range&) { 
			cout << "Warning: index "<<selectedPoint<<" out of range" << endl;
		}
	} else {
		cout << "Nothing selected..." << endl;
	}
}

/* setupProject() - Sets up projection & modelview matrices ------ */
void MainView::setupProjection()
{

	if( useArcball )
	{
		arcballCam.setProjection(false);
		return;
	}

	//the rest of this code is for birds eye view right?
	/*if( window->isShadowed() )
	{
		window->toggleShadows();
		//disable shadow lights
	}*/
	const float aspect = static_cast<float>(w()) / static_cast<float>(h());
	const float width  = (aspect >= 1) ? 110 : 110 * aspect;
	const float height = (aspect >= 1) ? 110 / aspect : 110;

	glMatrixMode(GL_PROJECTION);
	// Note: no glLoadIdentity() here, caller clears projection (for picking)
	glOrtho(-width, width, -height, height, 200, -200);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glRotatef(-90.f, 1.f, 0.f, 0.f);

	glViewport(0, 0, w(), h());
}

/* resetArcball() - Resets the arcball camera orientation -------- */
void MainView::resetArcball()
{
	arcballCam.setup(this, 40.f, 250.f, 0.2f, 0.4f, 0.f);
}

/* updateTextWidget() - Prints rotation amount to text widget ---- */
void MainView::updateTextWidget( const float t )
{
	stringstream ss; ss << "rot = " << t;
	window->setDebugText(ss.str());
}

/* openglFrameSetup() - Clears framebuffers and sets projection -- */
void MainView::openglFrameSetup()
{
	glClearColor(0.f, 0.f, 0.2f, 1.f);
	glClearStencil(0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glEnable(GL_DEPTH);

	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	setupProjection();
	// TODO: call these once only, not every frame

	glEnable(GL_COLOR_MATERIAL);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	if (!useArcball) {
		glDisable(GL_LIGHT1);
		glDisable(GL_LIGHT2);
	} else {
		glEnable(GL_LIGHT1);
		glEnable(GL_LIGHT2);
	}

	glShadeModel(GL_SMOOTH);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	
	glLineWidth(5.f);

	GLfloat gAmbient[] = { 0.2f, 0.2f, 0.2f, 1.f };
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, gAmbient);

	GLfloat ambient[] = { 0.5f, 0.5f, 0.5f, 1.f };
	GLfloat diffuse[] = {.3f, .3f, .3f, 1.f }; //diffuse is gray
	GLfloat specular[] = { 1.f, 1.f, 1.f, 1.f };
	GLfloat position1[] = { 0.f, 200.f, 0.f, 1.f }; //{0, 1, 1, 0};

	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
	glLightfv(GL_LIGHT0, GL_POSITION, position1);

	GLfloat lightPosition2[] = {50.f, 200.f, 50.f, 1.f}; // {0, 1, 1, 0};
	GLfloat lightPosition3[] = {200.f, 0.f, 0.f, 1.f}; // {1, 0, 0, 0}
	GLfloat yellowLight[] = {0.5f, 0.5f, .1f, 1.f};
	GLfloat whiteLight[] = {1.0f, 1.0f, 1.0f, 1.f};

	glLightfv(GL_LIGHT1, GL_POSITION, lightPosition2);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, whiteLight);

	glLightfv(GL_LIGHT2, GL_POSITION, lightPosition3);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, yellowLight);

	// -------------------------------------------

	glPushMatrix();
		//glColor4ub(255, 255, 255, 255); //this color setting originally did nothing
		glTranslatef(0.f, -20.f, 0.f);
		setupFloor();
		glDisable(GL_LIGHTING);
		drawGroundPlane(400.f);
		glEnable(GL_LIGHTING);
		setupObjects();
	glPopMatrix();
}

/* drawFloor() - Draws the floor plane and assorted scenery ------ */ 
void MainView::drawScenery(bool doShadows)
{
	if(!doShadows)
	{
		glDisable(GL_BLEND);
	}

	glPushMatrix();
		glTranslatef(0.f, -20.f, 0.f);

		if(!doShadows)
		{
			glColor4ub(255, 255, 255, 255);
			glTranslatef(0.f, 0.2f, 0.f);
			drawBasis(Vec3f(20.f, 0.f, 0.f),
			          Vec3f(0.f, 20.f, 0.f),
			          Vec3f(0.f, 0.f, 20.f));
		}

		glLineWidth(1.f);
		glPushMatrix();
			if(!doShadows)
			{
				glColor4ub(0, 0, 255, 255);
			}
			glTranslatef(-50.f, 10.f, 50.f);
			glRotatef(-90.f, 1.f, 0.f, 0.f);
			static const GLushort pattern = 0x00ff;
			glLineStipple(1, pattern);
			glEnable(GL_LINE_STIPPLE);
			glutWireSphere(10.f, 20, 20);
			glDisable(GL_LINE_STIPPLE);
		glPopMatrix();

		glPushMatrix();
			if(!doShadows)
			{
				glColor4ub(200, 50, 200, 255);
			}
			glTranslatef(-50.f, 6.f, -50.f);
			glRotatef(-45.f, 0.f, 1.f, 0.f);
			glutSolidTeapot(8.f);
		glPopMatrix();
		
		glLineWidth(5.f);

		glPushMatrix();
			if(!doShadows)
			{
				glColor4ub(0, 255, 0, 255);
			}
			glPushMatrix();
				glTranslatef(50.f, 5.f, 50.f);
				glRotatef(-90.f, 1.f, 0.f, 0.f);
				glutSolidCone(10.f, 20.f, 10, 10);
			glPopMatrix();
			
			if(!doShadows)
			{
				glColor4ub(139, 69, 19, 255);
			}
			glPushMatrix();
				glTranslatef(50.f, 2.5f, 50.f);
				glutSolidCube(5.f);
			glPopMatrix();
		glPopMatrix();

		glPushMatrix();
			if(!doShadows)
			{
				glColor4ub(255, 255, 10, 255);
			}
			glTranslatef(50.f, 8.1f, -50.f);
			glScalef(5.f, 5.f, 5.f);
			glutWireDodecahedron();
		glPopMatrix();

	glPopMatrix();

	if(!doShadows)
	{
		glEnable(GL_BLEND);
	}
}

/* drawCurve() - Draws the window's curve object ----------------- */
void MainView::drawCurve(const float t, bool doShadows)
{
	Curve& curve(window->getCurve());
	curve.selectedSegment = static_cast<int>(std::floor(t));

	if(!doShadows)
	{
		glColor4ub(255, 255, 255, 255);
	}
	curve.draw(doShadows);

	if(!doShadows)
	{
		glColor4ub(255, 0, 0, 255);
	}
	curve.drawSelectedSegment(doShadows);
}

/* drawPathObject() - Draws the object that travels along the path */
void MainView::drawPathObject(const float t, bool doShadows)
{
	Curve& curve(window->getCurve());
	const Vec3f p(curve.getPosition(t));	// position  @ t
	const Vec3f d(curve.getDirection(t));	// direction @ t (non-normalized)

	if(!doShadows)
	{
		glColor4ub(20, 250, 20, 255);
	}
	/*else
	{
		//glPushMatrix();
		//glTranslatef(0.f, -20.f, 0.f);
	}*/

	glPushMatrix();
		glTranslatef(p.x(), p.y() + 1.5f, p.z());
		applyBasisFromTangent(normalize(d));

		drawCube(0.f, 0.f, 0.f, 2.5f);
		/* TODO: drawing the basis shows that sometimes the local 
		coordinate system flips over, this will need to be 
		fixed before the final version so the train doesn't 
		go upside down (FIXED)
		drawBasis(Vec3f(5.f, 0.f, 0.f),
				  Vec3f(0.f, 5.f, 0.f),
				  Vec3f(0.f, 0.f, 5.f));
		*/
		if(!doShadows)
		{	
			drawVector(Vec3f(0.f, 0.f, 0.f), 
			           Vec3f(0.f, 0.f, 5.f),
			           Vec3f(1.f, 0.f, 1.f));
		}

	glPopMatrix();

	/*if(!window->isShadowed())
	{
		//glPopMatrix();
	}*/
}

void MainView::drawPathObjects(const float t, const bool doShadows)
{
	const Curve& curve(window->getCurve());

	float tt = t + 0.2f;
	if( tt >= curve.numSegments() )
	{
		tt -= curve.numSegments();
	}
	else if(tt < 0)
	{
		tt += curve.numSegments();
	}
	drawPathObject(tt, doShadows);

	tt = t + 0.1f;
	if( tt >= curve.numSegments() )
	{
		tt -= curve.numSegments();
	}
	else if(tt < 0)
	{
		tt += curve.numSegments();
	}
	drawPathObject(tt, doShadows);

	tt = t;
	if(tt < 0)
	{
		tt += curve.numSegments();
	}
	drawPathObject(tt, doShadows);
}

/* drawSelectedControlPoint() - Draws the selected point highlighted */
void MainView::drawSelectedControlPoint(bool doShadows)
{
	auto points = window->getPoints();
	if( selectedPoint >= 0 && selectedPoint < (signed)points.size() )
	{
		if(!doShadows)
		{
			glColor4ub(250, 20, 20, 255);
		}
		points[selectedPoint].draw(doShadows);
	}
}

//TODO if not used please delete, MAB did not use as of 3/28/12, not used with current arclength param attempt
/*void MainView::reparameterizing(Curve& curve, float big_t, bool doShadows)
{
	float seconds_per_sample = 1;
	float flight_time = seconds_per_sample * curve.numSegments(); // This should actually be decoupled.
	float time_now = (float)hpTime.TotalTime() - time_mode_started;

	//AnimateEqualSamples(time_now, seconds_per_sample);
	//DrawControlPointsAndTangents(true);
	float dmod = fmod(time_now, flight_time);
	float big_t = dmod / flight_time;
	int segment;
	float lil_t = arcLengthInterpolation(big_t, segment);
	//printf("dmod %f segment %d big_t %f lil_t %f time_now %f\n", dmod, segment, big_t, lil_t, time_now);

//bezier_segments[segment].EvaluateAt(lil_t);
	CurveSegment* curr_segment = curve.getSegment(segment);
	Vec3f p = curr_segment->getPosition(lil_t);//fix this using lil_t
	glPushMatrix();
	glTranslatef(p.x(), p.y(), p.z());

	glPopMatrix();
}*/

//float MainView::arcLengthInterpolation(float big_t, int& segment_number)
float MainView::arcLengthInterpolation(double big_t, int& segment_number)
	{
		Curve curve= window->getCurve();
		double epsilon = 0.0000001;
		printf("big_t in the function call is %f \n", big_t);
		//double epsilon = 0.000;
		if (abs(big_t) < epsilon)
			return 0;
		if (abs(big_t - 1) < epsilon)
			return 1;
	
		size_t index;
		for (index = 1; index < curve.parameter_table.size(); index++)
		{
			if (big_t < curve.parameter_table[index].fraction_of_accumulated_length)
				break;
		}
		if (index >= curve.parameter_table.size())
			throw "big_t not found.";

		printf("the index found is %d \n", index);
		/*the size of the paramer_table once fully built should be 1+num_segments()*number_of_samples*/
		printf("curve.parameter_table.size() is %d \n", curve.parameter_table.size());

		double lower_fraction = curve.parameter_table[index - 1].fraction_of_accumulated_length;
		double upper_fraction = curve.parameter_table[index].fraction_of_accumulated_length;
		double lower_local_t = ((curve.parameter_table[index - 1].segment_number != curve.parameter_table[index].segment_number) ? 0 : curve.parameter_table[index - 1].local_t);
		double upper_local_t = curve.parameter_table[index].local_t;
		float result = (float)(lower_local_t + (big_t - lower_fraction) / (upper_fraction - lower_fraction) * (upper_local_t - lower_local_t));
		//segment_number = curve.parameter_table[index].segment_number;
		printf("big_t: %f lil_t: %f lower fraction: %f upper_fraction: %f lower_t: %f upper_t: %f\n", big_t, result, lower_fraction, upper_fraction, lower_local_t, upper_local_t);
		return result;
	}

/* ==================================================================
 * MainWindow class
 * ==================================================================
 */

MainWindow::MainWindow(const int x, const int y) 
	: Fl_Double_Window(x, y, 800, 600, "Train Project - Phase 2")
	, view(nullptr)
	, widgets(nullptr)
	, animateButton(nullptr)
	, addPointButton(nullptr)
	, delPointButton(nullptr)
	, textOutput(nullptr)
	, curveTypeChoice(nullptr)
	, shadowButton(nullptr)
	, paramButton(nullptr)
	, forwardButton(nullptr)
	, backwardButton(nullptr)
	, speedSlider(nullptr)
	, curve(catmull)
	, animating(false)
	, rotation(0.f)
	, rotationStep(0.01f)
	, speed(2.f)
	, shadows(true)
	, isArcLengthParam(false)
	//, time_mode_started(hpTime.TotalTime()) //only use if using the HpTime to set up the big_t for arclength param
{
	createWidgets();
	resetPoints();

	Fl::add_idle(idleCallback, this); 
}

/* damageMe() - Called to force an update of the window ---------- */
void MainWindow::damageMe()
{
	view->damage(1);
}

/* setDebugText() - Called to update fltk multiline output text -- */
void MainWindow::setDebugText(const string& text)
{
	assert(textOutput != nullptr);
	textOutput->value("");
	textOutput->value(text.c_str());
}

/* createWidgets() - Called on construction to build fltk widgets  */
void MainWindow::createWidgets()
{
	// Make all the widgets
	begin();
	{
		// Create the OpenGL view
		view = new MainView(5, 5, 590, 590);
		view->setWindow(this);
		view->setSelectedPoint(0);
		//this->resizable(view);

		// Group widgets to help ease resizing
		widgets = new Fl_Group(600, 5, 190, 590); // x,y,w,h
		widgets->begin();

		// Create the animate button
		animateButton = new Fl_Button(605, 5, 60, 20, "Animate");
		animateButton->type(FL_TOGGLE_BUTTON);
		animateButton->value(0);                     // initially off
		animateButton->selection_color((Fl_Color)3); // yellow when pressed
		animateButton->callback((Fl_Callback*)animateButtonCallback, this);

		// Create the add control point button
		addPointButton = new Fl_Button(670, 5, 60, 20, "Add");
		addPointButton->type(FL_NORMAL_BUTTON);
		addPointButton->selection_color((Fl_Color)3); // yellow when pressed
		addPointButton->callback((Fl_Callback*)addPointButtonCallback, this);

		// Create the delete control point button
		delPointButton = new Fl_Button(735, 5, 60, 20, "Delete");
		delPointButton->type(FL_NORMAL_BUTTON);
		delPointButton->selection_color((Fl_Color)3); // yellow when pressed
		delPointButton->callback((Fl_Callback*)delPointButtonCallback, this);

		// Create curve type browser (drop down)
		curveTypeChoice = new Fl_Choice(605, 30, 90, 20);
		curveTypeChoice->clear();
		curveTypeChoice->add((CurveTypeNames[lines]).c_str());
		curveTypeChoice->add((CurveTypeNames[catmull]).c_str());
//		curveTypeChoice->add((CurveTypeNames[hermite]).c_str());
//		curveTypeChoice->add((CurveTypeNames[bspline]).c_str());
		curveTypeChoice->value(1);
		curveTypeChoice->callback((Fl_Callback*)curveTypeChoiceCallback, this);

		//create arc length parameterization button
		paramButton = new Fl_Button(605, 55, 120, 20, "Arclength Param");
		paramButton->type(FL_TOGGLE_BUTTON);
		paramButton->value(0);
		paramButton->selection_color((Fl_Color)3); // yellow when pressed
		paramButton->callback((Fl_Callback*)paramButtonCallback, this);

		//create arc length parameterization button
		shadowButton = new Fl_Button(700, 30, 90, 20, "Shadows");
		shadowButton->type(FL_TOGGLE_BUTTON);
		shadowButton->value(1);
		shadowButton->selection_color((Fl_Color)3); // yellow when pressed
		shadowButton->callback((Fl_Callback*)shadowButtonCallback, this);

		// Create the manual backwards button
		backwardButton = new Fl_Button(730, 55, 30, 20, "<<");
		backwardButton->type(FL_NORMAL_BUTTON);
		backwardButton->selection_color((Fl_Color)3); // yellow when pressed
		backwardButton->callback((Fl_Callback*)backwardButtonCallback, this);

		// Create the manual forwards button
		forwardButton = new Fl_Button(765, 55, 30, 20, ">>");
		forwardButton->type(FL_NORMAL_BUTTON);
		forwardButton->selection_color((Fl_Color)3); // yellow when pressed
		forwardButton->callback((Fl_Callback*)forwardButtonCallback, this);

		//create a speed slider
		speedSlider = new Fl_Value_Slider(645,80,140,20,"Speed");
		speedSlider->range(-10,10);
		speedSlider->value(2);
		speedSlider->align(FL_ALIGN_LEFT);
		speedSlider->type(FL_HORIZONTAL);
		speedSlider->callback((Fl_Callback*)speedSliderCallback, this);


		// Create text display
		textOutput = new Fl_Output(605, 105, 90, 20);

		// Create file write button
		/*writeButton = new Fl_Button(700, 105, 70, 20, "File Write");
		writeButton->type(FL_TOGGLE_BUTTON);
		writeButton->selection_color((Fl_Color)3); // yellow when pressed
		writeButton->callback((Fl_Callback*)writeButtonCallback, this);*/

		// Create a phantom widget to help resize things
		//this works with the train project because they have a World view as well as our trainview(mainview) view
		//TODO: fix?
		//Fl_Box *resizeBox = new Fl_Box(600, 595, 200, 5);
		//widgets->resizable(resizeBox);

		widgets->end();
	}
	end();
}

/* resetPoints() - Called to reset control points to a standard configuration */
void MainWindow::resetPoints()
{
	curve.clearPoints();

	const float step = TWO_PI / 5.f;
	const float radius = 30.f;
	for(float i = 0.f; i < TWO_PI; i += step)
	{
		const Vec3f     pos(cosf(i), 0.f, sinf(i));
		const CtrlPoint point(pos * radius);
		curve.addControlPoint(point);
	}
}

// the file format is simple
// first line: an integer with the number of control points
// other lines: one line per control point
// either 3 (X,Y,Z) numbers on the line, or 6 numbers (X,Y,Z, orientation)
void MainWindow::loadPoints(const string& filename)
{
	ifstream file(filename);
	if( file.is_open() )
	{
		// Get the number of points but don't worry about validating it,
		// since its not used and could be a lie anyways...
		int numPoints = 0;
		file >> numPoints;

		// Get the points
		curve.clearPoints();
		float px, py, pz;	// position
		float ox, oy, oz;	// orientation
		while( file >> px >> py >> pz >> ox >> oy >> oz )
		{
			const Vec3f position(px, py, pz);
			const Vec3f orientation(ox, oy, oz);
			curve.addControlPoint(CtrlPoint(position, orientation));
		}
	}
	else // file didn't open...
	{
		stringstream ss;
		ss << "Error - failed to open file: " << filename << endl
			<< "Using default control points instead." << endl;
		fl_alert(ss.str().c_str());
		resetPoints();
	}
}

// write the control points to our simple format
void MainWindow::savePoints(const string& filename)
{
	ofstream file(filename);
	if( file.is_open() )
	{
		// Save number of points
		file << curve.numControlPoints();

		// Save each point's position and orientation
		for each(const auto& point in curve.getControlPoints())
		{
			const Vec3f& p(point.pos());
			const Vec3f& o(point.orient());

			file << endl;
			file << p.x() << " " << p.y() << " " << p.z() << " ";
			file << o.x() << " " << o.y() << " " << o.z() << " ";
		}
	}
	else // file didn't open...
	{
		stringstream ss;
		ss << "Error - failed to open file \"" << filename
			<< "\" for writing." << endl;
		fl_alert(ss.str().c_str());
	}
}

void MainWindow::advanceTrain(int dir)
{
	int segment = 0;
	float newRotationStep = 0.0;

	if( isAnimating() )
	{
		if(isArcParam())
		{
			//MAIN DIFFERENCE BETWEEN PERRY's CODE AND THE TRAIN DEMO CODE:
			//in the sample project they use arclength param to figure out the step applied to the current rotation value

			//in perry's code he uses arclength param to directly translate the thing being arclength param'd

			//the following code computes the step amount (big_t) to be interpolated with in regards to a high precision timer: hpTime
			/*double seconds_per_sample = 1;
			double flight_time = seconds_per_sample * curve.numSegments();
			double time_now = hpTime.TotalTime() - time_mode_started;
			printf("time_now is %f and time_mode started is %f \n", time_now, time_mode_started);
			double dmod = fmod(time_now, flight_time);
			printf("dmod is %f \n", dmod);
			double big_t = dmod / (flight_time);
			printf("big_t is %f \n", big_t);*/
			//newRotationStep = view->arcLengthInterpolation(big_t, segment); //actual arclengthparam function call here set up like in TrainWindow.cpp
			//newRotationStep = rotationStep*(speed*0.5f); //my attempt at fixes
			//newRotationStep = newRotationStep/curve.numSegments(); //my attempt at fixes
			//newRotationStep = newRotationStep/rotation;  //my attempt at fixes
			newRotationStep = rotationStep*speed*0.5f;

			if(dir > 0)
				rotation = rotation + newRotationStep;
			else if(dir < 0)
				rotation = rotation - newRotationStep; //eventually allow for going in the opposite direction
			else
				rotation = rotation;

			//TODO: would need to add more conditionals here to eventually allow for going in the opposite direction
			if( rotation >= getCurve().numSegments() )
				rotation = 0.f;
			else if(rotation < 0.f)//assumes this condition is reached when we are still going in a pos dir (only reached when speed is negative)
				rotation= getCurve().numSegments()+ newRotationStep;
		}
		else
		{
			newRotationStep = rotationStep*speed*0.5f;
			if(dir > 0)
				rotation = rotation + newRotationStep;
			else if(dir < 0)
				rotation = rotation - newRotationStep; //eventually allow for going in the opposite direction
			else
				rotation = rotation;

			//TODO: would need to add more conditionals here to eventually allow for going in the opposite direction
			if( rotation >= getCurve().numSegments() )
				rotation = 0.f;
			else if(rotation < 0.f)//assumes this condition is reached when we are still going in a pos dir (only reached when speed is negative)
				rotation= getCurve().numSegments()+ newRotationStep;
		}
	}
}
