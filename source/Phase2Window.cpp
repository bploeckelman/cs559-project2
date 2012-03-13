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

#include <windows.h>
#define WIN32_LEAN_AND_MEAN

#include <GL/GL.h>
#include <GL/GLU.h>

#pragma warning(push)
#pragma warning(disable:4312)
#pragma warning(disable:4311)
#include <Fl/fl.h>
#include <Fl/Fl_Double_Window.h>
#include <Fl/Fl_Gl_Window.h>
#include <Fl/Fl_Box.h>
#include <Fl/Fl_Group.h>
#include <Fl/Fl_Button.h>
#pragma warning(pop)

// TODO - remove these and use constants in some header
#define M_PI       3.141592653f
#define M_2PI      2.f * M_PI

#include <iostream>
#include <vector>

using std::vector;
using std::cout;
using std::endl;


Phase2View::Phase2View(int x, int y, int w, int h, const char *l)
	: Fl_Gl_Window(x,y,w,h,l)
{
	mode( FL_RGB | FL_ALPHA | FL_DEPTH | FL_DOUBLE );
}

int Phase2View::handle(int event)
{
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
		/* Note : use this format to process keyboard input
		int k  = Fl::event_key();
		int ks = Fl::event_state();
		if( k == 'a' ) ; // ...
		*/
		break;
	}

	return Fl_Gl_Window::handle(event);
}

void Phase2View::draw()
{
	glClearColor(0.f, 0.f, 0.3f, 0.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	setupProjection();

	// Draw the points
	vector<CtrlPoint>::iterator it  = window->getPoints().begin();
	vector<CtrlPoint>::iterator end = window->getPoints().end();
	for(int i = 0; it != end; ++it, ++i)
	{
		if( i == selectedPoint )
			glColor3ub(240, 240, 30);
		else 
			glColor3ub(240, 60, 60);

		it->draw();
	}
}

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
			cout << "Warning: point index (" 
				 << selectedPoint 
				 << ") out of range" 
				 << endl;
		}
	} else {
		cout << "Nothing selected..." << endl;
	}
}

void Phase2View::setupProjection()
{
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


// --------------------------------------------------------------


Phase2Window::Phase2Window(const int x, const int y) 
	: Fl_Double_Window(x,y,800,600,"Train Project - Phase 2")
	, view(nullptr)
	, widgets(nullptr)
	, animateButton(nullptr)
	, points()
{
	// Make all the widgets
	begin();
	{
		// Create the opengl view
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
		animateButton->value(0); //initially off
		animateButton->selection_color((Fl_Color)3); // yellow when pressed
		animateButton->callback((Fl_Callback*)damageCB, this);

		// Create a phantom widget to help resize things
		Fl_Box *resizeBox = new Fl_Box(600, 595, 200, 5);
		widgets->resizable(resizeBox);

		widgets->end();
	}
	end();

	// Add initial points
	const float step = M_2PI / 10.f;
	const float radius = 60.f;
	for(float i = 0.f; i < M_2PI; i += step)
	{
		const Vec3f     pos(cosf(i), 0.f, sinf(i));
		const CtrlPoint point(pos * radius);
		points.push_back(point);
	}

	// Setup idle callback
	Fl::add_idle((void (*)(void*))idleCB, this);
}


void Phase2Window::damageMe()
{
	view->damage(1);
}
