/*
 * opengl.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The OpenGL wrapper library for the Asgard Game Engine, header file.
 *
 * Written by Josh English.
 */

#if !defined(_ASGARD_CORE_PLUGIN_OPENGL_H)

#define _ASGARD_CORE_PLUGIN_OPENGL_H

#if !defined(_ASGARD_H) && !defined(_ASGARD_CORE_PLUGIN_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _ASGARD_H || _ASGARD_CORE_PLUGIN_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// define opengl-wrapper public constants

//#define OPENGL_USE_PFS								atrue
#define OPENGL_USE_PFS								afalse

//#define OPENGL_PFS_WRITE_LOG						atrue
#define OPENGL_PFS_WRITE_LOG						afalse

#define OPENGL_PFS_LOG_FILENAME						"log/opengl.pfs.log"

//#define OPENGL_REPORT_USAGE							atrue
#define OPENGL_REPORT_USAGE							afalse


// define opengl-wrapper ES edition helper constants & macros

#if defined(__ANDROID__)

#	define GLclampd									double

#	define GLdouble									double

#	define GL_QUADS									1001

#	define GL_LIST_BIT								1002

#	define GL_COMPILE								1003

#	define GL_POLYGON								1004

#	define GL_QUAD_STRIP							1005

#	define glCallList(...) \
		asgard_glDoNothing()

#	define glCallLists(...) \
		asgard_glDoNothing()

#	define glClearDepth(...) \
		asgard_glDoNothing()

#	define glColor4d(red, green, blue, alpha) \
		glColor4f(red, green, blue, alpha)

#	define glColorMaterial(...) \
		asgard_glDoNothing()

#	define glDeleteLists(...) \
		asgard_glDoNothing()

#	define glEndList(...) \
		asgard_glDoNothing()

#	define glGetDoublev(...) \
		asgard_glDoNothing()

#	define glGenLists(...) \
		asgard_glDoNothing()

#	define glListBase(...) \
		asgard_glDoNothing()

#	define glMateriali(...) \
		asgard_glDoNothing()

#	define glNewList(...) \
		asgard_glDoNothing()

/*#	define glNormal3d(nx, ny, nz) \
		glNormal3f(nx, ny, nz)*/

#	define glOrtho(left, right, bottom, top, nearVal, farVal) \
		glOrthof(left, right, bottom, top, nearVal, farVal)

#	define glPopAttrib(...) \
		asgard_glDoNothing()

#	define glPushAttrib(...) \
		asgard_glDoNothing()

#	define glRasterPos4d(...) \
		asgard_glDoNothing()

#	define glRotated(angle, x, y, z) \
		glRotatef(angle, x, y, z)

/*#	define glTexCoord2d(...) \
		asgard_glDoNothing()*/

#	define glMultiTexCoord2d(...) \
		asgard_glDoNothing()

#	define glTranslated(x, y, z) \
		glTranslatef(x, y, z)

#	define gluBuild2DMipmaps(...) \
		asgard_glDoNothing()

#	define gluLookAt(...) \
		asgard_glDoNothing()

#	define gluPerspective(yFov, aspect, zNear, zFar) \
		android_gluPerspective(yFov, aspect, zNear, zFar)
/*
#	define gluPerspective(...) \
		asgard_glDoNothing()
*/

#endif // __ANDROID__


// declare opengl-wrapper public functions

Pfs *asgard_getPfs();

// gl functions

void android_gluPerspective(double yFov, double aspect, double zNear,
		double zFar);

int asgard_glDoNothing();

void asgard_glBegin(GLenum mode, const char *file, const char *function);

void asgard_glBindTexture(GLenum target, GLuint texture, const char *file,
		const char *function);

void asgard_glBlendFunc(GLenum sfactor, GLenum dfactor, const char *file,
		const char *function);

void asgard_glCallList(GLuint list, const char *file, const char *function);

void asgard_glCallLists(GLsizei n, GLenum type, const GLvoid *lists,
		const char *file, const char *function);

void asgard_glClear(GLbitfield mask, const char *file, const char *function);

void asgard_glClearColor(GLclampf red, GLclampf green, GLclampf blue,
		GLclampf alpha, const char *file, const char *function);

void asgard_glClearDepth(GLclampd depth, const char *file,
		const char *function);

void asgard_glColor4d(GLdouble red, GLdouble green, GLdouble blue,
		GLdouble alpha, const char *file, const char *function);

void asgard_glColorMaterial(GLenum face, GLenum mode, const char *file,
		const char *function);

void asgard_glDeleteLists(GLuint list, GLsizei range, const char *file,
		const char *function);

void asgard_glDeleteTextures(GLsizei n, const GLuint *textures,
		const char *file, const char *function);

void asgard_glDepthFunc(GLenum func, const char *file, const char *function);

void asgard_glDisable(GLenum cap, const char *file, const char *function);

void asgard_glEnable(GLenum cap, const char *file, const char *function);

void asgard_glEnd(const char *file, const char *function);

void asgard_glEndList(const char *file, const char *function);

GLenum asgard_glGetError(const char *file, const char *function);

void asgard_glGetDoublev(GLenum pname, GLdouble *params, const char *file,
		const char *function);

const GLubyte *asgard_glGetString(GLenum name, const char *file,
		const char *function);

GLuint asgard_glGenLists(GLsizei range, const char *file,
		const char *function);

void asgard_glGenTextures(GLsizei n, GLuint *textures, const char *file,
		const char *function);

void asgard_glFrontFace(GLenum mode, const char *file, const char *function);

void asgard_glHint(GLenum target, GLenum mode, const char *file,
		const char *function);

GLboolean asgard_glIsEnabled(GLenum cap, const char *file,
		const char *function);

void asgard_glLightf(GLenum light, GLenum pname, GLfloat param,
		const char *file, const char *function);

void asgard_glLightfv(GLenum light, GLenum pname, const GLfloat *params,
		const char *file, const char *function);

void asgard_glLineWidth(GLfloat width, const char *file, const char *function);

void asgard_glListBase(GLuint base, const char *file, const char *function);

void asgard_glLoadIdentity(const char *file, const char *function);

void asgard_glMateriali(GLenum face, GLenum pname, GLint param,
		const char *file, const char *function);

void asgard_glMatrixMode(GLenum mode, const char *file, const char *function);

void asgard_glNewList(GLuint list, GLenum mode, const char *file,
		const char *function);

void asgard_glNormal3d(GLdouble nx, GLdouble ny, GLdouble nz,
		const char *file, const char *function);

void asgard_glOrtho(GLdouble left, GLdouble right, GLdouble bottom,
		GLdouble top, GLdouble nearVal, GLdouble farVal, const char *file,
		const char *function);

void asgard_glPopAttrib(const char *file, const char *function);

void asgard_glPopMatrix(const char *file, const char *function);

void asgard_glPushAttrib(GLbitfield mask, const char *file,
		const char *function);

void asgard_glPushMatrix(const char *file, const char *function);

void asgard_glRasterPos4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w,
		const char *file, const char *function);

void asgard_glRotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z,
		const char *file, const char *function);

void asgard_glShadeModel(GLenum mode, const char *file, const char *function);

void asgard_glTexCoord2d(GLdouble s, GLdouble t, const char *file,
		const char *function);

void asgard_glTexEnvi(GLenum target, GLenum pname, GLint param,
		const char *file, const char *function);

void asgard_glTexImage2D(GLenum target, GLint level, GLint internalFormat,
		GLsizei width, GLsizei height, GLint border, GLenum format,
		GLenum type, const GLvoid *data, const char *file,
		const char *function);

void asgard_glTexParameteri(GLenum target, GLenum pname, GLint param,
		const char *file, const char *function);

void asgard_glTexParameterf(GLenum target, GLenum pname, GLfloat param,
		const char *file, const char *function);

void asgard_glTexParameterfv(GLenum target, GLenum pname, GLfloat *params,
		const char *file, const char *function);

void asgard_glTranslated(GLdouble x, GLdouble y, GLdouble z, const char *file,
		const char *function);

void asgard_glViewport(GLint x, GLint y, GLsizei width, GLsizei height,
		const char *file, const char *function);

void asgard_glVertex2d(GLdouble x, GLdouble y, const char *file,
		const char *function);

void asgard_glVertex3d(GLdouble x, GLdouble y, GLdouble z,
		const char *file, const char *function);

// glu functions

GLint asgard_gluBuild2DMipmaps(GLenum target, GLint internalFormat,
		GLsizei width, GLsizei height, GLenum format, GLenum type,
		const void *data, const char *file, const char *function);

void asgard_gluLookAt(GLdouble eyeX, GLdouble eyeY, GLdouble eyeZ,
		GLdouble centerX, GLdouble centerY, GLdouble centerZ,
		GLdouble upX, GLdouble upY, GLdouble upZ,
		const char *file, const char *function);

void asgard_gluPerspective(GLdouble fovy, GLdouble aspect, GLdouble znear,
		GLdouble zfar, const char *file, const char *function);


// define opengl-wrapper public macros

// gl functions

#define cgi_glBegin(mode) \
	asgard_glBegin(mode, __FILE__, __FUNCTION__)

#define cgi_glBindTexture(target, texture) \
	asgard_glBindTexture(target, texture, __FILE__, __FUNCTION__)

#define cgi_glCallList(list) \
	asgard_glCallList(list, __FILE__, __FUNCTION__)

#define cgi_glCallLists(n, type, lists) \
	asgard_glCallLists(n, type, lists, __FILE__, __FUNCTION__)

#define cgi_glBlendFunc(sfactor, dfactor) \
	asgard_glBlendFunc(sfactor, dfactor, __FILE__, __FUNCTION__)

#define cgi_glClear(mask) \
	asgard_glClear(mask, __FILE__, __FUNCTION__)

#define cgi_glClearColor(red, green, blue, alpha) \
	asgard_glClearColor(red, green, blue, alpha, __FILE__, __FUNCTION__)

#define cgi_glClearDepth(depth) \
	asgard_glClearDepth(depth, __FILE__, __FUNCTION__)

#define cgi_glColor4d(red, green, blue, alpha) \
	asgard_glColor4d(red, green, blue, alpha, __FILE__, __FUNCTION__)

#define cgi_glColorMaterial(face, mode) \
	asgard_glColorMaterial(face, mode, __FILE__, __FUNCTION__)

#define cgi_glDeleteLists(list, range) \
	asgard_glDeleteLists(list, range, __FILE__, __FUNCTION__)

#define cgi_glDeleteTextures(n, textures) \
	asgard_glDeleteTextures(n, textures, __FILE__, __FUNCTION__)

#define cgi_glDepthFunc(func) \
	asgard_glDepthFunc(func, __FILE__, __FUNCTION__)

#define cgi_glDisable(cap) \
	asgard_glDisable(cap, __FILE__, __FUNCTION__)

#define cgi_glEnable(cap) \
	asgard_glEnable(cap, __FILE__, __FUNCTION__)

#define cgi_glEnd() \
	asgard_glEnd(__FILE__, __FUNCTION__)

#define cgi_glEndList() \
	asgard_glEndList(__FILE__, __FUNCTION__)

#define cgi_glGetError() \
	asgard_glGetError(__FILE__, __FUNCTION__)

#define cgi_glGetDoublev(pname, params) \
	asgard_glGetDoublev(pname, params, __FILE__, __FUNCTION__)

#define cgi_glGetString(name) \
	asgard_glGetString(name, __FILE__, __FUNCTION__)

#define cgi_glGenLists(range) \
	asgard_glGenLists(range, __FILE__, __FUNCTION__)

#define cgi_glGenTextures(n, textures) \
	asgard_glGenTextures(n, textures, __FILE__, __FUNCTION__)

#define cgi_glFrontFace(mode) \
	asgard_glFrontFace(mode, __FILE__, __FUNCTION__)

#define cgi_glHint(target, mode) \
	asgard_glHint(target, mode, __FILE__, __FUNCTION__)

#define cgi_glIsEnabled(cap) \
	asgard_glIsEnabled(cap, __FILE__, __FUNCTION__)

#define cgi_glLightf(light, pname, param) \
	asgard_glLightf(light, pname, param, __FILE__, __FUNCTION__)

#define cgi_glLightfv(light, pname, params) \
	asgard_glLightfv(light, pname, params, __FILE__, __FUNCTION__)

#define cgi_glLineWidth(width) \
	asgard_glLineWidth(width, __FILE__, __FUNCTION__)

#define cgi_glListBase(base) \
	asgard_glListBase(base, __FILE__, __FUNCTION__)

#define cgi_glLoadIdentity() \
	asgard_glLoadIdentity(__FILE__, __FUNCTION__)

#define cgi_glMateriali(face, pname, param) \
	asgard_glMateriali(face, pname, param, __FILE__, __FUNCTION__)

#define cgi_glMatrixMode(mode) \
	asgard_glMatrixMode(mode, __FILE__, __FUNCTION__)

#define cgi_glNewList(list, mode) \
	asgard_glNewList(list, mode, __FILE__, __FUNCTION__)

#define cgi_glNormal3d(nx, ny, nz) \
	asgard_glNormal3d(nx, ny, nz, __FILE__, __FUNCTION__)

#define cgi_glOrtho(left, right, bottom, top, nearVal, farVal) \
	asgard_glOrtho(left, right, bottom, top, nearVal, farVal, __FILE__, \
			__FUNCTION__)

#define cgi_glPopAttrib() \
	asgard_glPopAttrib(__FILE__, __FUNCTION__)

#define cgi_glPopMatrix() \
	asgard_glPopMatrix(__FILE__, __FUNCTION__)

#define cgi_glPushAttrib(mask) \
	asgard_glPushAttrib(mask, __FILE__, __FUNCTION__)

#define cgi_glPushMatrix() \
	asgard_glPushMatrix(__FILE__, __FUNCTION__)

#define cgi_glRasterPos4d(x, y, z, w) \
	asgard_glRasterPos4d(x, y, z, w, __FILE__, __FUNCTION__)

#define cgi_glRotated(angle, x, y, z) \
	asgard_glRotated(angle, x, y, z, __FILE__, __FUNCTION__)

#define cgi_glShadeModel(mode) \
	asgard_glShadeModel(mode, __FILE__, __FUNCTION__)

#define cgi_glTexCoord2d(s, t) \
	asgard_glTexCoord2d(s, t, __FILE__, __FUNCTION__)

#define cgi_glTexEnvi(target, pname, param) \
	asgard_glTexEnvi(target, pname, param, __FILE__, __FUNCTION__)

#define cgi_glTexImage2D(target, level, internalFormat, width, height, \
		border, format, type, data) \
	asgard_glTexImage2D(target, level, internalFormat, width, height, \
		border, format, type, data, __FILE__, __FUNCTION__)

#define cgi_glTexParameteri(target, pname, param) \
	asgard_glTexParameteri(target, pname, param, __FILE__, __FUNCTION__)

#define cgi_glTexParameterf(target, pname, param) \
	asgard_glTexParameterf(target, pname, param, __FILE__, __FUNCTION__)

#define cgi_glTexParameterfv(target, pname, params) \
	asgard_glTexParameterfv(target, pname, params, __FILE__, __FUNCTION__)

#define cgi_glTranslated(x, y, z) \
	asgard_glTranslated(x, y, z, __FILE__, __FUNCTION__)

#define cgi_glViewport(x, y, width, height) \
	asgard_glViewport(x, y, width, height, __FILE__, __FUNCTION__)

#define cgi_glVertex2d(x, y) \
	asgard_glVertex2d(x, y, __FILE__, __FUNCTION__)

#define cgi_glVertex3d(x, y, z) \
	asgard_glVertex3d(x, y, z, __FILE__, __FUNCTION__)

// glu functions

#define cgi_gluBuild2DMipmaps(target, internalFormat, width, height, format, \
		type, data) \
	asgard_gluBuild2DMipmaps(target, internalFormat, width, height, format, \
			type, data, __FILE__, __FUNCTION__)

#define cgi_gluLookAt(eyeX, eyeY, eyeZ, \
		centerX, centerY, centerZ, \
		upX, upY, upZ) \
	asgard_gluLookAt(eyeX, eyeY, eyeZ, \
			centerX, centerY, centerZ, \
			upX, upY, upZ, \
			__FILE__, __FUNCTION__);

#define cgi_gluPerspective(fovy, aspect, znear, zfar) \
	asgard_gluPerspective(fovy, aspect, znear, zfar, __FILE__, __FUNCTION__);



#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _ASGARD_CORE_PLUGIN_OPENGL_H

