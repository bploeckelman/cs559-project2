/*
 * CS559    - Train Project
 *
 * The MainView class is a fltk OpenGL window
 *
 * Authors: Brian Ploeckelman
 *          Matthew Bayer
 */
#include "MainView.h"
#include "MainWindow.h"

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
#pragma warning(disable:4996)
#include <Fl/fl.h>
#include <Fl/Fl_Gl_Window.h>
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

string ViewTypeNames[] = { "Arcball", "Train", "Overhead" };


/* ==================================================================
 * MainView class
 * ==================================================================
 */

MainView::MainView(int x, int y, int w, int h, const char *l)
	: Fl_Gl_Window(x,y,w,h,l)
	, arcballCam()
	, selectedPoint(-1)
	, viewType(arcball)
{
	mode( FL_RGB | FL_ALPHA | FL_DEPTH | FL_DOUBLE );
	resetArcball();
}

/* draw() - Draws to the screen ---------------------------------- */
void MainView::draw()
{
	float t = window->getRotation();

	updateTextWidget(t);
	openglFrameSetup();

	// Draw everything once without shadows
	drawScenery();
	if( viewType == train )
		drawCurve(t, false, false);
	else
	{
		drawCurve(t, true, false);
		drawTrain(t);
		drawSelectedControlPoint(false);
	}

	// Draw everything again with shadows if they are enabled
	if( window->isShadowed() && viewType != overhead )
	{
		glPushMatrix();
			// Translate down to the ground plane
			glTranslatef(0.f, -20.f, 0.f);

			setupShadows();

			drawScenery(true);
			if( viewType == train )
				drawCurve(t, false, true);
			else
			{
				drawCurve(t, true, true);
				drawTrain(t, true);
				drawSelectedControlPoint(true);
			}

			unsetupShadows();
		glPopMatrix();
	}
}

/* handle() - Handles user input --------------------------------- */
int MainView::handle(int event)
{
	if( viewType == arcball )
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
		if ( lastPush == 1 && selectedPoint >=0 && viewType != train )
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
	case FL_KEYBOARD:
		//* Note : use this format to process keyboard input
		int k  = Fl::event_key();
		int ks = Fl::event_state();
		if( k == 'a' )
		{
			viewType = arcball;
			window->setViewType(0);
		}
		if( k == 't' )
		{
			viewType = train;
			window->setViewType(1);

		}
		if( k == 'o' )
		{
			viewType = overhead;
			window->setViewType(2);
		}
		break;
	}

	return Fl_Gl_Window::handle(event);
}

/* pick() - Performs OpenGL picking ------------------------------ */
void MainView::pick()
{
	// Don't pick if in train view
	if( viewType == train )
		return;

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

	ControlPointVectorConstIter it  = window->getPoints().begin();
	ControlPointVectorConstIter end = window->getPoints().end();
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

/* resetArcball() - Resets the arcball camera orientation -------- */
void MainView::resetArcball()
{
	arcballCam.setup(this, 40.f, 250.f, 0.2f, 0.4f, 0.f);
}

/* setupProject() - Sets up projection & modelview matrices ------ */
void MainView::setupProjection()
{
	const float aspect = static_cast<float>(w()) / static_cast<float>(h());
	const float width  = (aspect >= 1) ? 110 : 110 * aspect;
	const float height = (aspect >= 1) ? 110 / aspect : 110;

	switch(viewType)
	{
		case arcball: arcballCam.setProjection(false); break;
		case train:
		{
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			gluPerspective(90.0, aspect, 0.1, 1000.0);

			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();

			Curve& curve(window->getCurve());
			const float t = window->getRotation();

			const Vec3f& p(-1.f * curve.getPosition(t));
			const Vec3f& d(-1.f * curve.getDirection(t));

			// Calculate and apply the orientation matrix
			Vec3f normal(curve.getOrientation(t));
			Vec3f binormal(normalize(cross(normal, normalize(d))));

			normal = normalize(cross(normalize(d), binormal));

			const Vec3f& z(normalize(d)), y(normal), x(binormal);
			GLfloat m[] = {
				x.x(), y.x(), z.x(), 0.f,
				x.y(), y.y(), z.y(), 0.f,
				x.z(), y.z(), z.z(), 0.f,
				0.f,   0.f,   0.f,   1.f
			};
			glMultMatrixf(m);

			// Move to the correct position, offset on y to be on top of track
			glTranslatef(p.x(), p.y() - 3.f * y.y(), p.z());
		}
		break;
		case overhead:
		{
			glMatrixMode(GL_PROJECTION);
			// Note: no glLoadIdentity() here, caller clears projection (for picking)
			glOrtho(-width, width, -height, height, 200, -200);

			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			glRotatef(-90.f, 1.f, 0.f, 0.f);
		}
		break;
	};

	glViewport(0, 0, w(), h());
}

/* updateTextWidget() - Prints rotation amount to text widget ---- */
void MainView::updateTextWidget( const float t )
{
	stringstream ss, ss1;
	ss  << "t = " << t;
	ss1 << "s = " << (window->isArcParam() ?
		window->arcLengthStep(window->speed * 0.07f) : 0.f);

	window->setDebugText(ss.str(), ss1.str());
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

	if (viewType != arcball) {
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
	//glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
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
}

/* drawScenery() - Draws the floor plane and assorted scenery ------ */
void MainView::drawScenery(bool doShadows)
{
	if( !doShadows ) glDisable(GL_BLEND);

	if( !doShadows )
	{
		glPushMatrix();
			glTranslatef(0.f, -20.f, 0.f);
			setupFloor();
			glDisable(GL_LIGHTING);
			drawGroundPlane(400.f);
			glEnable(GL_LIGHTING);
			setupObjects();
		glPopMatrix();
	}

	glPushMatrix();
		glTranslatef(0.f, -20.f, 0.f);

		if( !doShadows )
		{
			glColor4ub(255, 255, 255, 255);
			glTranslatef(0.f, 0.2f, 0.f);
			drawBasis(Vec3f(20.f, 0.f, 0.f),
			          Vec3f(0.f, 20.f, 0.f),
			          Vec3f(0.f, 0.f, 20.f));
		}

		glLineWidth(1.f);
		glPushMatrix();
			if( !doShadows ) glColor4ub(0, 0, 255, 255);
			glTranslatef(-50.f, 10.f, 50.f);
			glRotatef(-90.f, 1.f, 0.f, 0.f);
			glutSolidSphere(10.f, 20, 20);
		glPopMatrix();

		glPushMatrix();
			if( !doShadows ) glColor4ub(200, 50, 200, 255);
			glTranslatef(-50.f, 6.f, -50.f);
			glRotatef(-45.f, 0.f, 1.f, 0.f);
			glutSolidTeapot(8.f);
		glPopMatrix();

		glLineWidth(5.f);

		glPushMatrix();
			if( !doShadows ) glColor4ub(0, 255, 0, 255);
			glPushMatrix();
				glTranslatef(50.f, 5.f, 50.f);
				glRotatef(-90.f, 1.f, 0.f, 0.f);
				glutSolidCone(10.f, 20.f, 10, 10);
			glPopMatrix();

			if( !doShadows ) glColor4ub(139, 69, 19, 255);
			glPushMatrix();
				glTranslatef(50.f, 2.5f, 50.f);
				glutSolidCube(5.f);
			glPopMatrix();
		glPopMatrix();

		glPushMatrix();
			if( !doShadows ) glColor4ub(255, 255, 10, 255);
			glTranslatef(50.f, 8.1f, -50.f);
			glScalef(5.f, 5.f, 5.f);
			glutWireDodecahedron();
		glPopMatrix();

	glPopMatrix();

	if( !doShadows ) glEnable(GL_BLEND);
}

/* drawCurve() - Draws the window's curve object ----------------- */
void MainView::drawCurve(const float t, bool drawPoints, bool doShadows)
{
	Curve& curve(window->getCurve());
	curve.selectedSegment = static_cast<int>(std::floor(t));

	curve.draw(drawPoints, doShadows);

	if(window->isHighlightedSegPts())
	{
		curve.drawSelectedSegment(drawPoints, doShadows);
	}
}

/* drawTrain() - Draws the train at the specified parameter ------ */
void MainView::drawTrain( const float t, bool doingShadows )
{
	const Vec3f& p(window->getCurve().getPosition(t));
	const Vec3f& d(window->getCurve().getDirection(t));

	Vec3f normal(window->getCurve().getOrientation(t));
	Vec3f binormal;

	glPushMatrix();

	glTranslatef(p.x(), p.y(), p.z());

	// Calculate and apply orientation matrix
	binormal = normalize(cross(normal, normalize(d)));
	normal   = normalize(cross(normalize(d), binormal));

	const Vec3f& z(normalize(d)), y(normal), x(binormal);
	GLfloat m[] = {
		x.x(), x.y(), x.z(), 0.f,
		y.x(), y.y(), y.z(), 0.f,
		z.x(), z.y(), z.z(), 0.f,
		0.f,   0.f,   0.f,   1.f
	};
	glMultMatrixf(m);

	// Face forward
	glRotatef(-90.f, 0.f, 1.f, 0.f);

	if(!doingShadows) glColor3d(0.3, 0.5, 1.0);

	// inside face
	glBegin(GL_POLYGON);
		glNormal3d(0.0, 0.0, 1.0);
		glVertex3f( 6.f, 1.f,  2.5f);
		glVertex3f(-3.f, 1.f,  2.5f);
		glVertex3f(-6.f, 2.f,  2.5f);
		glVertex3f(-6.f, 5.5f, 2.5f);
		glVertex3f( 3.f, 6.f,  2.5f);
	glEnd();

	glBegin(GL_TRIANGLES);
		glNormal3d(-0.29, -0.18, 0.94);
		glVertex3f(6.f, 1.f, 2.5f);
		glVertex3f(8.f, 3.f, 1.5f);
		glVertex3f(3.f, 6.f, 2.5f);
	glEnd();

	// outside face
	glBegin(GL_POLYGON);
		glNormal3d(0.0, 0.0, -1.0);
		glVertex3f( 6.f, 1.f,  -2.5);
		glVertex3f(-3.f, 1.f,  -2.5);
		glVertex3f(-6.f, 2.f,  -2.5);
		glVertex3f(-6.f, 5.5f, -2.5);
		glVertex3f( 3.f, 6.f,  -2.5);
	glEnd();

	glBegin(GL_TRIANGLES);
		glNormal3d(0.29, 0.18, -0.94);
		glVertex3f(6.f, 1.f, -2.5f);
		glVertex3f(8.f, 3.f, -1.5f);
		glVertex3f(3.f, 6.f, -2.5f);
	glEnd();

	// panels
	glBegin(GL_QUADS);
		glNormal3d(0.51, 0.86, 0.0);
		glVertex3f(3.f, 6.f, 2.5f);
		glVertex3f(3.f, 6.f, -2.5f);
		glVertex3f(8.f, 3.f, -1.5f);
		glVertex3f(8.f, 3.f, 1.5f);

		glNormal3d(0.71, -0.71, 0.0);
		glVertex3f(8.f, 3.f,  1.5f);
		glVertex3f(8.f, 3.f, -1.5f);
		glVertex3f(6.f, 1.f, -2.5f);
		glVertex3f(6.f, 1.f,  2.5f);

		glNormal3d(0.0, -1.0, 0.0);
		glVertex3f( 6.f, 1.f,  2.5f);
		glVertex3f( 6.f, 1.f, -2.5f);
		glVertex3f(-3.f, 1.f, -2.5f);
		glVertex3f(-3.f, 1.f,  2.5f);

		glNormal3d(-0.32, -0.95, 0.0);
		glVertex3f(-3.f, 1.f,  2.5f);
		glVertex3f(-3.f, 1.f, -2.5f);
		glVertex3f(-6.f, 2.f, -2.5f);
		glVertex3f(-6.f, 2.f,  2.5f);

		glNormal3d(-1.0, 0.0, 0.0);
		glVertex3f(-6.f, 2.f,   2.5f);
		glVertex3f(-6.f, 2.f,  -2.5f);
		glVertex3f(-6.f, 5.5f, -2.5f);
		glVertex3f(-6.f, 5.5f,  2.5f);

		if(!doingShadows) glColor3d(0.2, 0.2, 0.2);

		glNormal3d(-0.06, 0.998, 0.0);
		glVertex3f(-6.f, 5.5f,  2.5f);
		glVertex3f(-6.f, 5.5f, -2.5f);
		glVertex3f( 3.f, 6.f,  -2.5f);
		glVertex3f( 3.f, 6.f,   2.5f);
	glEnd();

	// wheels
	if(!doingShadows) glColor3d(0.0, 0.0, 0.0);

	glTranslatef(5.f, 0.f, 2.55f);
	glBegin(GL_POLYGON);
		glNormal3d(0.0, 0.0, -1.0);
		glVertex3f(0.f, 2., 0);
		glVertex3f(-0.8f, 1.4f, 0.f);
		glVertex3f(-0.8f, 0.6f, 0.f);
		glVertex3f( 0.f,  0.f,  0.f);
		glVertex3f( 0.8f, 0.6f, 0.f);
		glVertex3f( 0.8f, 1.4f, 0.f);
	glEnd();

	glTranslatef(0.f, 0.f, -5.1f);
	glBegin(GL_POLYGON);
		glNormal3d(0.0, 0.0, 1.0);
		glVertex3f( 0.f,  2.f,  0.f);
		glVertex3f(-0.8f, 1.4f, 0.f);
		glVertex3f(-0.8f, 0.6f, 0.f);
		glVertex3f( 0.f,  0.f,  0.f);
		glVertex3f( 0.8f, 0.6f, 0.f);
		glVertex3f( 0.8f, 1.4f, 0.f);
	glEnd();

	glTranslatef(-8.f, 0.f, 0.f);
	glBegin(GL_POLYGON);
		glNormal3d( 0.0, 0.0, 1.0);
		glVertex3f( 0.f,   2.f,  0.f);
		glVertex3f(-0.8f,  1.4f, 0.f);
		glVertex3f(-0.8f,  0.6f, 0.f);
		glVertex3f( 0.f,   0.0f, 0.f);
		glVertex3f( 0.8f,  0.6f, 0.f);
		glVertex3f( 0.8f,  1.4f, 0.f);
	glEnd();

	glTranslatef(0.f, 0.f, 5.1f);
	glBegin(GL_POLYGON);
		glNormal3d(0.0, 0.0, -1.0);
		glVertex3f( 0.f,  2.f,  0.f);
		glVertex3f(-0.8f, 1.4f, 0.f);
		glVertex3f(-0.8f, 0.6f, 0.f);
		glVertex3f( 0.f,  0.f,  0.f);
		glVertex3f( 0.8f, 0.6f, 0.f);
		glVertex3f( 0.8f, 1.4f, 0.f);
	glEnd();

	glPopMatrix();
}

/* drawSelectedControlPoint() - Draws the selected point highlighted */
void MainView::drawSelectedControlPoint(bool doShadows)
{
	auto points = window->getPoints();
	if( selectedPoint >= 0 && selectedPoint < (signed)points.size() )
	{
		if( !doShadows ) glColor4ub(250, 20, 20, 255);
		points[selectedPoint].draw(doShadows);
	}
}
