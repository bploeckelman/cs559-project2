// CS559 Train Project
//	begun Oct 2008, Mike Gleicher
// Implementation of ControlPoint class
#include "ControlPoint.H"

#include <windows.h>
#include <GL/gl.h>

#include "Utilities/3dUtils.h"
#include <math.h>

ControlPoint::ControlPoint() : pos(0,0,0), orient(0,1,0)
{
}
ControlPoint::ControlPoint(const Pnt3f &_pos) : pos(_pos), orient(0,1,0)
{
}
ControlPoint::ControlPoint(const Pnt3f &_pos, const Pnt3f &_orient) : pos(_pos), orient(_orient)
{
	orient.normalize();
}

void ControlPoint::draw()
{
	float size=2.0;

	glPushMatrix();
	glTranslatef(pos.x,pos.y,pos.z);
	float theta1 = -radiansToDegrees(atan2(orient.z,orient.x));
	glRotatef(theta1,0,1,0);
	float theta2 = -radiansToDegrees(acos(orient.y));
	glRotatef(theta2,0,0,1);

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

			// no top - it will be the point

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
		glBegin(GL_TRIANGLE_FAN);
			glNormal3f(0,1.0f,0);
			glVertex3f(0,3.0f*size,0);
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
	glPopMatrix();
}


// $Header: /p/course/cs559-gleicher/private/CVS/TrainFiles/ControlPoint.cpp,v 1.3 2008/10/16 14:34:54 gleicher Exp $