/*
 * CtrlPoint.cpp
 */
#include "CtrlPoint.h"
#include "Vec3f.h"

#include <Windows.h>
#define WIN32_LEAN_AND_MEAN

#include <GL/GL.h>

// #include <math.h>
// Frowny face at Microsoft for not including constants from math.h
// without having to specify _USE_MATH_CONSTANTS... jerks...
// TODO - remove this and use constants in some header
#define M_PI       3.141592653f

// TODO - move this elsewhere and include it
float radToDeg(const float rad)
{
	static const float factor = 180.f / M_PI;
	return rad * factor;
}


CtrlPoint::CtrlPoint(const Vec3f& pos, const Vec3f& orient)
	: _pos(pos)
	, _orient(normalize(orient))
{ }

void CtrlPoint::draw() const
{
	float size=2.0;

	glPushMatrix();
	
		glTranslatef(_pos.x(), _pos.y(), _pos.z());

		float theta1 = -radToDeg(atan2(_orient.z(), _orient.x()));
		glRotatef(theta1,0,1,0);

		float theta2 = -radToDeg(acos(_orient.y()));
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

bool CtrlPoint::operator==(const CtrlPoint& other)
{
	// TODO : would a compare by address work?
	// or should we compare by pos and orient? (once they are in place)
	return (this == &other) ? true : false;
}