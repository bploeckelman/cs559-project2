/*
 * CtrlPoint.cpp
 */
#include "CtrlPoint.h"
#include "MathUtils.h"
#include "GLUtils.h"

#include <Windows.h>
#define WIN32_LEAN_AND_MEAN

#include <GL/GL.h>


CtrlPoint::CtrlPoint(const Vec3f& pos, const Vec3f& orient)
	: _pos(pos)
	, _orient(normalize(orient))
{ }

void CtrlPoint::draw(bool isShadowed) const
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

		//error with shadows!
		if(!isShadowed)
		{
			drawVector(Vec3f(0.f, 0.f, 0.f),
					   Vec3f(0.f, 8.f, 0.f),
					   Vec3f(0.f, 1.f, 0.f));
		}
	glPopMatrix();
}

bool CtrlPoint::operator==(const CtrlPoint& other)
{
	return (_pos    == other._pos
		 && _orient == other._orient);
}
