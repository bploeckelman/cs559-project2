/*
 * CS559   - Train Project 
 * Phase 2 - OpenGL Programming Signs of Life
 *
 * The Phase2Window class is a basic FlTk window.
 *
 * Authors: Brian Ploeckelman
 *          Matthew Bayer
 */
#include "Phase2Window.h"
#include "Callback.h"
#include "CtrlPoint.h"
#include "MathUtils.h"
#include "GLUtils.h"

#include "TrainFiles/Utilities/ArcBallCam.H"
#include "TrainFiles/Utilities/3DUtils.h"	// for drawCube()

#include <windows.h>
#define WIN32_LEAN_AND_MEAN

#include <GL/GL.h>
#include <GL/GLU.h>

#pragma warning(push)
#pragma warning(disable:4312)
#pragma warning(disable:4311)
#pragma warning(disable:4099)
#include <Fl/fl.h>
#include <Fl/Fl_Double_Window.h>
#include <Fl/Fl_Gl_Window.h>
#include <Fl/Fl_Box.h>
#include <Fl/Fl_Group.h>
#include <Fl/Fl_Button.h>
#include <Fl/Fl_Output.h>
#include <Fl/Fl_Choice.h>
#pragma warning(pop)

#include <iostream>
#include <cassert>
#include <sstream>
#include <string>
#include <vector>

using std::stringstream;
using std::string;
using std::vector;
using std::cout;
using std::endl;


/* ==================================================================
 * Phase2View class
 * ==================================================================
 */

Phase2View::Phase2View(int x, int y, int w, int h, const char *l)
	: Fl_Gl_Window(x,y,w,h,l)
	, arcballCam()
{
	mode( FL_RGB | FL_ALPHA | FL_DEPTH | FL_DOUBLE );
	resetArcball();
}

/* draw() - Draws to the screen ---------------------------------- */
void Phase2View::draw()
{
	const float t = window->getRotation();

	// Update window's textOutput widget
	stringstream ss; ss << "rot = " << t;
	window->setDebugText(ss.str());

/* TODO: remove this, it checks allowable line width values
	GLfloat sizes[2]; 
	GLfloat step;
	glGetFloatv(GL_LINE_WIDTH_RANGE, sizes);
	glGetFloatv(GL_LINE_WIDTH_GRANULARITY, &step);
*/
	// TODO: call these once only
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glLineWidth(5.f);

	glClearColor(0.f, 0.f, 0.2f, 0.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	setupProjection();

	// Draw the floor and a basis --------------------------------
	glColor4ub(255, 255, 255, 200);
	glPushMatrix();
		glTranslatef(0.f, -20.f, 0.f);
		drawFloor(150.f);

		glTranslatef(0.f, 0.1f, 0.f);
		drawBasis(Vec3f(20.f, 0.f, 0.f), 
				  Vec3f(0.f, 20.f, 0.f), 
				  Vec3f(0.f, 0.f, 20.f));
	glPopMatrix();

	// Draw the curve, highlighting the current segment ----------
	Curve& curve(window->getCurve());
	curve.selectedSegment = static_cast<int>(std::floor(t));

	glColor4ub(255, 255, 255, 255);
	curve.draw();
	glColor4ub(255, 0, 0, 255);
	curve.drawSelectedSegment();

	// Draw the moving cube --------------------------------------
	const Vec3f p(curve.getPosition(t));	// position  @ t
	const Vec3f d(curve.getDirection(t));	// direction @ t (non-normalized)

	const Vec3f wup(0.f, 0.f, 1.f);		// world up vector

	const Vec3f dir(normalize(d));                  // tangent
	const Vec3f rit(cross(dir, wup).normalize());   // local right vector
	const Vec3f lup(cross(rit, dir).normalize());   // local up vector

	glColor4ub(20, 250, 20, 255);
	glPushMatrix();
		glTranslatef(p.x(), p.y(), p.z());

		GLfloat m[] = {
			rit.x(), rit.y(), rit.z(), 0.f,
			lup.x(), lup.y(), lup.z(), 0.f,
			dir.x(), dir.y(), dir.z(), 0.f,
			0.f,     0.f,     0.f,     1.f
		};
		glMultMatrixf(m);

		drawCube(0.f, 0.f, 0.f, 2.5f);
	glPopMatrix();
}

/* handle() - Handles user input --------------------------------- */
int Phase2View::handle(int event)
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
	case FL_FOCUS:
		return 1;
	case FL_ENTER: // take focus anytime mouse enters window
		focus(this);
		break;
	case FL_KEYBOARD : 
		//* Note : use this format to process keyboard input
		int k  = Fl::event_key();
		int ks = Fl::event_state();
		if( k == ' ' ) toggleUseArcball(); // ...
		if( k == 'c' ) 
		{
			Curve& curve = window->getCurve();
			++curve.selectedSegment;
			if( curve.selectedSegment >= curve.numSegments() )
				curve.selectedSegment = 0;
		}
		if( k == 'z' )
		{
			Curve& curve = window->getCurve();
			--curve.selectedSegment;
			if( curve.selectedSegment < 0 )
				curve.selectedSegment = curve.numSegments() - 1;
		}
		//*/
		break;
	}

	return Fl_Gl_Window::handle(event);
}

/* pick() - Performs OpenGL picking ------------------------------ */
void Phase2View::pick()
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
		it->draw();
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
void Phase2View::setupProjection()
{
	if( useArcball )
	{
		arcballCam.setProjection(false);
		return;
	}

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

void Phase2View::resetArcball()
{
	arcballCam.setup(this, 40.f, 250.f, 0.2f, 0.4f, 0.f);
}


/* ==================================================================
 * Phase2Window class
 * ==================================================================
 */

Phase2Window::Phase2Window(const int x, const int y) 
	: Fl_Double_Window(x, y, 800, 600, "Train Project - Phase 2")
	, view(nullptr)
	, widgets(nullptr)
	, animateButton(nullptr)
	, textOutput(nullptr)
	, curveTypeChoice(nullptr)
	, points()
	, curve(lines)
	, animating(false)
	, rotation(0.f)
{
	createWidgets();
	createPoints();

	Fl::add_idle(idleCallback, this); 
}

/* damageMe() - Called to force an update of the window ---------- */
void Phase2Window::damageMe()
{
	view->damage(1);
}

/* setDebugText() - Called to update fltk multiline output text -- */
void Phase2Window::setDebugText(const string& text)
{
	assert(textOutput != nullptr);
	textOutput->value("");
	textOutput->value(text.c_str());
}

/* createWidgets() - Called on construction to build fltk widgets  */
void Phase2Window::createWidgets()
{
	// Make all the widgets
	begin();
	{
		// Create the OpenGL view
		view = new Phase2View(5, 5, 590, 590);
		view->setWindow(this);
		view->setSelectedPoint(0);
		this->resizable(view);

		// Group widgets to help ease resizing
		widgets = new Fl_Group(600, 5, 190, 590); // x,y,w,h
		widgets->begin();

		// Create the animate button
		animateButton = new Fl_Button(605, 5, 60, 20, "Animate");
		animateButton->type(FL_TOGGLE_BUTTON);
		animateButton->value(0);                     // initially off
		animateButton->selection_color((Fl_Color)3); // yellow when pressed
		animateButton->callback((Fl_Callback*)animateButtonCallback, this);

		// Create curve type browser (drop down)
		curveTypeChoice = new Fl_Choice(605, 30, 90, 20);
		curveTypeChoice->clear();
		curveTypeChoice->add((CurveTypeNames[lines]).c_str());
		curveTypeChoice->add((CurveTypeNames[catmull]).c_str());
//		curveTypeChoice->add((CurveTypeNames[hermite]).c_str());
//		curveTypeChoice->add((CurveTypeNames[bspline]).c_str());
		curveTypeChoice->value(0);
		curveTypeChoice->callback((Fl_Callback*)curveTypeChoiceCallback, this);

		// Create text display
		textOutput = new Fl_Output(605, 55, 90, 20);

		// Create a phantom widget to help resize things
		Fl_Box *resizeBox = new Fl_Box(600, 595, 200, 5);
		widgets->resizable(resizeBox);

		widgets->end();
	}
	end();
}

/* createPoints() - Called on construction to create initial points */
void Phase2Window::createPoints()
{
	// Add initial points
	const float step = TWO_PI / 5.f;
	const float radius = 30.f;
	for(float i = 0.f; i < TWO_PI; i += step)
	{
		const Vec3f     pos(cosf(i), 0.f, sinf(i));
		const CtrlPoint point(pos * radius);
		points.push_back(point);
		curve.addControlPoint(point);
	}
}
