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
#include <Fl/fl.h>
#include <Fl/Fl_Double_Window.h>
#include <Fl/Fl_Gl_Window.h>
#include <Fl/Fl_Box.h>
#include <Fl/Fl_Group.h>
#include <Fl/Fl_Button.h>
#include <Fl/Fl_Output.h>
#include <Fl/Fl_Choice.h>
#include <Fl/glut.h>		// for primitive drawing
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
	float t = window->getRotation();
	Curve& curve(window->getCurve());

	updateTextWidget(t);

	openglFrameSetup();

	drawScenery();
	drawCurve(t);
	drawPathObject(t+0.3f); // need to have +.3f to draw the 2 cars behind it

	drawSelectedControlPoint();
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
		if( k == ' ' ) toggleUseArcball(); // ...
		//*/
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
void MainView::setupProjection()
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
	// TODO: call these once only, not every frame
	glEnable(GL_LIGHTING);
	GLfloat gAmbient[] = { 0.2f, 0.2f, 0.2f, 1.f };
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, gAmbient);

	GLfloat ambient[] = { 0.5f, 0.5f, 0.5f, 1.f };
	GLfloat diffuse[] = { 0.7f, 0.7f, 0.7f, 1.f };
	GLfloat specular[] = { 1.f, 1.f, 1.f, 1.f };
	GLfloat position[] = { 0.f, 200.f, 0.f, 1.f };

	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
	glLightfv(GL_LIGHT0, GL_POSITION, position);
	glEnable(GL_LIGHT0);

	glShadeModel(GL_SMOOTH);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glLineWidth(5.f);
	// -------------------------------------------

	glClearColor(0.f, 0.f, 0.2f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	setupProjection();
}

/* drawFloor() - Draws the floor plane and assorted scenery ------ */ 
void MainView::drawScenery()
{
	glDisable(GL_BLEND);

	glPushMatrix();
		glColor4ub(255, 255, 255, 255);
		glTranslatef(0.f, -20.f, 0.f);
		drawFloor(150.f);

		glTranslatef(0.f, 0.1f, 0.f);
		drawBasis(Vec3f(20.f, 0.f, 0.f), 
				  Vec3f(0.f, 20.f, 0.f), 
				  Vec3f(0.f, 0.f, 20.f));

		glLineWidth(1.f);
		glPushMatrix();
			glColor4ub(0, 0, 255, 255);
			glTranslatef(-50.f, 10.f, 50.f);
			glRotatef(-90.f, 1.f, 0.f, 0.f);
			static const GLushort pattern = 0x00ff;
			glLineStipple(1, pattern);
			glEnable(GL_LINE_STIPPLE);
			glutWireSphere(10.f, 20, 20);
			glDisable(GL_LINE_STIPPLE);
		glPopMatrix();
		glPushMatrix();
			glColor4ub(200, 50, 200, 255);
			glTranslatef(-50.f, 6.f, -50.f);
			glRotatef(-45.f, 0.f, 1.f, 0.f);
			glutSolidTeapot(8.f);
		glPopMatrix();
		glLineWidth(5.f);

		glPushMatrix();
			glColor4ub(0, 255, 0, 255);
			glPushMatrix();
				glTranslatef(50.f, 5.f, 50.f);
				glRotatef(-90.f, 1.f, 0.f, 0.f);
				glutSolidCone(10.f, 20.f, 10, 10);
			glPopMatrix();
			
			glColor4ub(139, 69, 19, 255);
			glPushMatrix();
				glTranslatef(50.f, 2.5f, 50.f);
				glutSolidCube(5.f);
			glPopMatrix();
		glPopMatrix();

		glPushMatrix();
			glColor4ub(255, 255, 10, 255);
			glTranslatef(50.f, 8.1f, -50.f);
			glScalef(5.f, 5.f, 5.f);
			glutWireDodecahedron();
		glPopMatrix();

	glPopMatrix();

	glEnable(GL_BLEND);
}

/* drawCurve() - Draws the window's curve object ----------------- */
void MainView::drawCurve( const float t )
{
	Curve& curve(window->getCurve());
	curve.selectedSegment = static_cast<int>(std::floor(t));

	glColor4ub(255, 255, 255, 255);
	curve.draw();

	glColor4ub(255, 0, 0, 255);
	curve.drawSelectedSegment();
}

/* drawPathObject() - Draws the object that travels along the path */
void MainView::drawPathObject( const float t )
{
	Curve& curve(window->getCurve());

	const Vec3f wup(0.f, 0.f, 1.f);         // world up vector

	vector<Vec3f> train_vectors(15); //create 3 pairs of 5 vectors for each train car

	//printf("t is %f \n", t);
	float tcar1;
	float tcar2;
	float tcar3;

	//the following can probably be called recursively to make this method customizable
	if(t > curve.numSegments())
	{
		tcar1 = t-(float)curve.numSegments();
		//printf("tcar1 at t:%f is %f \n",t , tcar1);

		if((t-0.1f) > curve.numSegments())
		{
			tcar2 = t- 0.1f-(float)curve.numSegments();
			//printf("tcar2 at t:%f is %f \n",t , tcar2);

			if((t-0.2f) > curve.numSegments())
			{
				tcar3 = t- 0.2f-(float)curve.numSegments();
				//printf("tcar3 at t:%f is %f \n",t , tcar3);
			}
			else
			{
				tcar3 = t- 0.2f;
				//printf("tcar3 (in else) at t:%f is %f \n",t , tcar3);
			}
		}
		else
		{
			tcar2 = t- 0.1f;
			//printf("tcar2 (in else) at t:%f is %f \n",t , tcar2);

			if((t-0.2f) > curve.numSegments())
			{
				tcar3 = t- 0.2f-(float)curve.numSegments();
				//printf("tcar3 at t:%f is %f \n",t , tcar3);
			}
			else
			{
				tcar3 = t- 0.2f;
				//printf("tcar3 (in else) at t:%f is %f \n",t , tcar3);
			}
		}
	}
	else
	{
		//printf("t is %f \n", t);
		tcar1 = t;
		tcar2 = t- 0.1f;
		tcar3 = t- 0.2f;
	}
	//end recursiveness


	for(int x = 0; x<15; x=x+5) //this can be made customizable
	{
		if(x == 0)
		{
			train_vectors.at(x)= curve.getPosition(tcar1); // position  @ t
			train_vectors.at(x+1) = curve.getDirection(tcar1); // direction @ t (non-normalized)
		}
		else if(x == 5)
		{
			train_vectors.at(x)= curve.getPosition(tcar2); // position  @ t- 1.f
			train_vectors.at(x+1) = curve.getDirection(tcar2); // direction @ t - 1.f (non-normalized)
		}
		else if(x == 10)
		{
			train_vectors.at(x)= curve.getPosition(tcar3); // position  @ t- 2.f
			train_vectors.at(x+1) = curve.getDirection(tcar3); // direction @ t - 2.f (non-normalized)
		}

		train_vectors.at(x+2)= normalize(train_vectors.at(x+1)); //tangent
		train_vectors.at(x+3)= cross(train_vectors.at(x+2), wup).normalize(); // local right vector
		train_vectors.at(x+4)= cross(train_vectors.at(x+3), train_vectors.at(x+2)).normalize(); // local up vector

		if(x == 0)
		{
			glColor4ub(20, 250, 20, 255); //lime green
		}
		else if(x == 5)
		{
			glColor4ub(255, 255, 0, 255); //yellow
		}
		else if(x == 10)
		{
			glColor4ub(0, 0, 255, 255); //blue
		}

		glPushMatrix();
		glTranslatef(0.f, 1.f, 0.f);
		glTranslatef(train_vectors.at(x).x(), train_vectors.at(x).y(), train_vectors.at(x).z());

		GLfloat m[] = {
			train_vectors.at(x+3).x(), train_vectors.at(x+3).y(), train_vectors.at(x+3).z(), 0.f,
			train_vectors.at(x+4).x(), train_vectors.at(x+4).y(), train_vectors.at(x+4).z(), 0.f,
			train_vectors.at(x+2).x(), train_vectors.at(x+2).y(), train_vectors.at(x+2).z(), 0.f,
			0.f,     0.f,     0.f,     1.f
		};
		glMultMatrixf(m);

		drawCube(0.f, 0.f, 0.f, 2.5f);
		drawVector(Vec3f(0.f, 0.f, 0.f), 
		Vec3f(0.f, 0.f, 5.f),
		Vec3f(1.f, 0.f, 1.f));
		glPopMatrix();
	}

}

/* drawSelectedControlPoint() - Draws the selected point highlighted */
void MainView::drawSelectedControlPoint()
{
	glColor4ub(250, 20, 20, 255);
	try {
		window->getPoints().at(selectedPoint).draw();
	} catch(std::out_of_range&) {}
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
	, curve(catmull)
	, animating(false)
	, rotation(0.f)
{
	createWidgets();
	createPoints();

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
//		this->resizable(view);

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

		// Create text display
		textOutput = new Fl_Output(605, 55, 90, 20);

		// Create a phantom widget to help resize things
//		Fl_Box *resizeBox = new Fl_Box(600, 595, 200, 5);
//		widgets->resizable(resizeBox);

		widgets->end();
	}
	end();
}

/* createPoints() - Called on construction to create initial points */
void MainWindow::createPoints()
{
	// Add initial points
	const float step = TWO_PI / 5.f;
	const float radius = 30.f;
	for(float i = 0.f; i < TWO_PI; i += step)
	{
		const Vec3f     pos(cosf(i), 0.f, sinf(i));
		const CtrlPoint point(pos * radius);
		curve.addControlPoint(point);
	}
}
