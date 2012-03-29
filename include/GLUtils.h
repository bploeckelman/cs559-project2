#pragma once
/*
 * GLUtils.h
 */
#include "Vec3f.h"
#include "MathUtils.h"

#include <Windows.h>
#define WIN32_LEAN_AND_MEAN

#include <GL/GL.h>
#include <GL/GLU.h>

#include <Fl/glut.h>


inline void applyBasisFromTangent(const Vec3f& tangent)
{
	const Vec3f z(tangent);
	Vec3f y, x;   
	generateBasis(z, y, x);
	GLfloat m[] = {
		x.x(), x.y(), x.z(), 0.f,
		y.x(), y.y(), y.z(), 0.f,
		z.x(), z.y(), z.z(), 0.f,
		0.f,   0.f,   0.f,   1.f // vec.x(), vec.y(), vec.z(), 1.f
	};
	glMultMatrixf(m);
}

inline void drawVector(const Vec3f& pos, const Vec3f& vec, const Vec3f& color)
{
	glColor4f(color.x(), color.y(), color.z(), 1.f);

	// Draw the line
	glBegin(GL_LINES);
		glVertex3f(pos.x(), pos.y(), pos.z());
		glVertex3f(vec.x(), vec.y(), vec.z());
	glEnd();

	// Draw the arrow head
	glPushMatrix();
		glTranslatef(vec.x(), vec.y(), vec.z());
		applyBasisFromTangent(normalize(pos - vec));
		glutSolidCone(0.5f, 1.5f, 8, 4);
	glPopMatrix();
}

inline void drawBasis(const Vec3f& x=Vec3f(1.f, 0.f, 0.f), 
					  const Vec3f& y=Vec3f(0.f, 1.f, 0.f),
					  const Vec3f& z=Vec3f(0.f, 0.f, 1.f))
{
	const Vec3f origin(0.f, 0.f, 0.f);

	drawVector(origin, x, Vec3f(1.f, 0.f, 0.f));
	drawVector(origin, y, Vec3f(0.f, 1.f, 0.f));
	drawVector(origin, z, Vec3f(0.f, 0.f, 1.f));
}

inline void lookAt(const Vec3f& position, 
				   const Vec3f& direction, 
				   const Vec3f& up)
{
	gluLookAt(position.x(), position.y(), position.z(), 
			  direction.x(), direction.y(), direction.z(),
			  up.x(), up.y(), up.z());
}
