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
	const float t = window->getRotation();

	updateTextWidget(t);
	openglFrameSetup();

	// Draw everything once without shadows
	drawScenery();
	if( viewType == train )
		drawCurve(t, false, false);
	else
	{
		drawCurve(t, true, false);
		drawPathObjects(t);
		drawSelectedControlPoint(false);
	}

	// Draw everything again with shadows if they are enabled
	if(window->isShadowed() && viewType != overhead)
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
				drawPathObjects(t, true);
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
			window->setViewType(0); //update widget value
		}
		if( k == 't' )
		{
			viewType = train;
			window->setViewType(1); //update widget value

		}
		if( k == 'o' )
		{
			viewType = overhead;
			window->setViewType(2); //update widget value
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

			const float t = window->getRotation();
			Curve& curve(window->getCurve());
			const Vec3f& p(-1.f * curve.getPosition(t));
			const Vec3f& d(-1.f * curve.getDirection(t));

			applyBasisFromTangent(normalize(d));
			glTranslatef(p.x(), p.y() - 3.f, p.z());
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
	stringstream ss; ss << "t = " << t;
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
	if( !doShadows ) glDisable(GL_BLEND);

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

void MainView::drawTrainHeadCar(const float t, bool doShadows)
{
	Curve& curve(window->getCurve());
	const Vec3f p(curve.getPosition(t));	// position  @ t
	const Vec3f d(curve.getDirection(t));	// direction @ t (non-normalized)

	if(!doShadows)
	{
		glColor4ub(20, 250, 20, 255);
	}

	glPushMatrix();
		glTranslatef(p.x(), p.y() + 2.5f, p.z());
		applyBasisFromTangent(normalize(d));

		float size = 1.5f;
		//drawCube(0.f, 0.f, 0.f, 3.f);
		//should be drawn as a rectangle
		glBegin(GL_QUADS);
			//front
			/*glNormal3f( 0,0,1);
			glVertex3f( size, size, size);
			glVertex3f(-size, size, size);
			glVertex3f(-size,-size, size);
			glVertex3f( size,-size, size);*/

			//back
			glNormal3f( 0, 0, -1);
			glVertex3f( size, size, -size);
			glVertex3f( size,-size, -size);
			glVertex3f(-size,-size, -size);
			glVertex3f(-size, size, -size);

			//top
			glNormal3f( 0, 1, 0);
			glVertex3f( size, size,  size);
			glVertex3f(-size, size,  size);
			glVertex3f(-size, size,  -size);
			glVertex3f( size, size,  -size);

			//bottom
			glNormal3f( 0,-1,0);
			glVertex3f( size,-size, size);
			glVertex3f(-size,-size, size);
			glVertex3f(-size,-size,-size);
			glVertex3f( size,-size,-size);

			//right side facing with x vector
			glNormal3f( 1,0,0);
			glVertex3f( size, size, size);
			glVertex3f( size,-size, size);
			glVertex3f( size,-size,-size);
			glVertex3f( size, size,-size);

			//left side
			glNormal3f(-1,0,0);
			glVertex3f(-size, size, size);
			glVertex3f(-size, size,-size);
			glVertex3f(-size,-size,-size);
			glVertex3f(-size,-size, size);
		glEnd();

		//draw cow catcher on the front, needs to be fixed
		//glPushMatrix();
		//glRotatef(90.f, 1.f, 0.f, 0.f);
		glBegin(GL_TRIANGLE_FAN);

			/*glNormal3f( 0,0,1);
			glVertex3f( size, size, size);
			glVertex3f(-size, size, size);
			glVertex3f(-size,-size, size);
			glVertex3f( size,-size, size);*/

			glNormal3f(0,0.f,1.f);
			glVertex3f(0,0.f,3.0f*size);
			glNormal3f( 1.0f, 0.0f , 1.0f);
			glVertex3f( size, size , size);
			glNormal3f(-1.0f, 0.0f , 1.0f);
			glVertex3f(-size, size , size);
			glNormal3f(-1.0f, 0.0f ,-1.0f);
			glVertex3f(-size, size ,-size);
			glNormal3f( 1.0f, 0.0f ,-1.0f);
			glVertex3f( size, size ,-size);
			glNormal3f( 1.0f, 0.0f , 1.0f);
			glVertex3f( size, size , size);
		glEnd();
		//glPopMatrix();

		glPushMatrix();
			if(!doShadows)
			{
				glColor4ub(0, 0, 0, 255);
			}
			//void glRotatef(GLfloat  angle,  GLfloat  x,  GLfloat  y,  GLfloat  z);
			//drawCube(1.8f, -1.5f, 1.8f, 1.0f);
			glTranslatef(1.8f, -1.5f, 1.8f);
			glRotatef(90.f,  0.f,  1.f,  0.f);
			//glutSolidTorus(GLdouble innerRadius, GLdouble outerRadius, GLint sides, GLint rings)
			glutSolidTorus(0.4f, 0.6f, 10, 12);
			
		glPopMatrix();

		glPushMatrix();
			if(!doShadows)
			{
				glColor4ub(0, 0, 0, 255);
			}
			//drawCube(-1.8f, -1.5f, 1.8f, 1.0f);
			glTranslatef(-1.8f, -1.5f, 1.8f);
			glRotatef(90.f,  0.f,  1.f,  0.f);
			glutSolidTorus(0.4f, 0.6f, 10, 12);
			
		glPopMatrix();

		glPushMatrix();
			if(!doShadows)
			{
				glColor4ub(0, 0, 0, 255);
			}
			//drawCube(1.8f, -1.5f, -1.8f, 1.0f);
			glTranslatef(1.8f, -1.5f, -1.8f);
			glRotatef(90.f,  0.f,  1.f,  0.f);
			glutSolidTorus(0.4f, 0.6f, 10, 12);
			
		glPopMatrix();

		glPushMatrix();
			if(!doShadows)
			{
				glColor4ub(0, 0, 0, 255);
			}
			//drawCube(-1.8f, -1.5f, -1.8f, 1.0f);
			glTranslatef(-1.8f, -1.5f, -1.8f);
			glRotatef(90.f,  0.f,  1.f,  0.f);
			glutSolidTorus(0.4f, 0.6f, 10, 12);
			
		glPopMatrix();

		if(window->isHighlightedSegPts()&&!doShadows)
		{
			drawBasis(Vec3f(5.f, 0.f, 0.f),
					  Vec3f(0.f, 5.f, 0.f),
					  Vec3f(0.f, 0.f, 5.f));
		}
	glPopMatrix();

}

/* drawPathObject() - Draws the object that travels along the path */
void MainView::drawTrainCar(const float t, bool doShadows)
{
	Curve& curve(window->getCurve());
	const Vec3f p(curve.getPosition(t));	// position  @ t
	const Vec3f d(curve.getDirection(t));	// direction @ t (non-normalized)

	if(!doShadows)
	{
		glColor4ub(20, 250, 20, 255);
	}

	glPushMatrix();
		glTranslatef(p.x(), p.y() + 2.5f, p.z());
		applyBasisFromTangent(normalize(d));

		//drawCube(0.f, 0.f, 0.f, 3.f);
		float size= 1.5f;
		//float rand_coal_size = size- ((1.5f/99.f) * (float)(rand() % 100));
		float rand_coal_size = size- 1.1f;
		glBegin(GL_QUADS);
			glNormal3f( 0,0,1);
			glVertex3f( size, size, size);
			glVertex3f(-size, size, size);
			glVertex3f(-size,-size, size);
			glVertex3f( size,-size, size);

			glNormal3f( 0, 0, -1);
			glVertex3f( size, size, -size);
			glVertex3f( size,-size, -size);
			glVertex3f(-size,-size, -size);
			glVertex3f(-size, size, -size);

			//draw coal for top of car
			if(!doShadows)
			{
				glColor4ub(0, 0, 0, 255);
			}

			glNormal3f( 0, 1, 0);
			glVertex3f( size, rand_coal_size,  size);
			glVertex3f(-size, rand_coal_size,  size);
			glVertex3f(-size, rand_coal_size,  -size);
			glVertex3f( size, rand_coal_size,  -size);

			if(!doShadows)
			{
				glColor4ub(20, 250, 20, 255);
			}

			glNormal3f( 0,-1,0);
			glVertex3f( size,-size, size);
			glVertex3f(-size,-size, size);
			glVertex3f(-size,-size,-size);
			glVertex3f( size,-size,-size);

			glNormal3f( 1,0,0);
			glVertex3f( size, size, size);
			glVertex3f( size,-size, size);
			glVertex3f( size,-size,-size);
			glVertex3f( size, size,-size);

			glNormal3f(-1,0,0);
			glVertex3f(-size, size, size);
			glVertex3f(-size, size,-size);
			glVertex3f(-size,-size,-size);
			glVertex3f(-size,-size, size);
		glEnd();

		glPushMatrix();
			if(!doShadows)
			{
				glColor4ub(0, 0, 0, 255);
			}
			//void glRotatef(GLfloat  angle,  GLfloat  x,  GLfloat  y,  GLfloat  z);
			//drawCube(1.8f, -1.5f, 1.8f, 1.0f);
			glTranslatef(1.8f, -1.5f, 1.8f);
			glRotatef(90.f,  0.f,  1.f,  0.f);
			//glutSolidTorus(GLdouble innerRadius, GLdouble outerRadius, GLint sides, GLint rings)
			glutSolidTorus(0.4f, 0.6f, 10, 12);
			
		glPopMatrix();

		glPushMatrix();
			if(!doShadows)
			{
				glColor4ub(0, 0, 0, 255);
			}
			//drawCube(-1.8f, -1.5f, 1.8f, 1.0f);
			glTranslatef(-1.8f, -1.5f, 1.8f);
			glRotatef(90.f,  0.f,  1.f,  0.f);
			glutSolidTorus(0.4f, 0.6f, 10, 12);
			
		glPopMatrix();

		glPushMatrix();
			if(!doShadows)
			{
				glColor4ub(0, 0, 0, 255);
			}
			//drawCube(1.8f, -1.5f, -1.8f, 1.0f);
			glTranslatef(1.8f, -1.5f, -1.8f);
			glRotatef(90.f,  0.f,  1.f,  0.f);
			glutSolidTorus(0.4f, 0.6f, 10, 12);
			
		glPopMatrix();

		glPushMatrix();
			if(!doShadows)
			{
				glColor4ub(0, 0, 0, 255);
			}
			//drawCube(-1.8f, -1.5f, -1.8f, 1.0f);
			glTranslatef(-1.8f, -1.5f, -1.8f);
			glRotatef(90.f,  0.f,  1.f,  0.f);
			glutSolidTorus(0.4f, 0.6f, 10, 12);
			
		glPopMatrix();

		if(window->isHighlightedSegPts()&&!doShadows)
		{
			drawBasis(Vec3f(5.f, 0.f, 0.f),
					  Vec3f(0.f, 5.f, 0.f),
					  Vec3f(0.f, 0.f, 5.f));
		}
	glPopMatrix();

}

/* drawPathObjects() - Draws several objects along the path ------ */
void MainView::drawPathObjects(const float t, const bool doShadows)
{
	const Curve& curve(window->getCurve());

	float tt = t + 0.4f;
	if( tt >= curve.numSegments() )
	{
		tt -= curve.numSegments();
	}
	else if(tt < 0)
	{
		tt += curve.numSegments();
	}
	drawTrainHeadCar(tt, doShadows);

	tt = t + 0.15f;
	if( tt >= curve.numSegments() )
	{
		tt -= curve.numSegments();
	}
	else if(tt < 0)
	{
		tt += curve.numSegments();
	}
	drawTrainCar(tt, doShadows);

	tt = t;
	if(tt < 0)
	{
		tt += curve.numSegments();
	}
	drawTrainCar(tt, doShadows);
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
