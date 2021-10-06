/*
 * opengl.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The OpenGL wrapper library for the Asgard Game Engine.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#define _ASGARD_CORE_COMPONENT
#include "asgard/core/common.h"
#define _ASGARD_CORE_PLUGIN_COMPONENT
#include "asgard/core/plugin/opengl.h"
#define _ASGARD_MATH_COMPONENT
#include "asgard/math/common.h"


// define opengl-wrapper private data types

typedef struct _AndroidBuffer {
	aboolean is2d;
	unsigned int type;
	int vertexLength;
	int normalLength;
	int texCoordLength;
	float *vertices;
	float *normals;
	float *texCoords;
} AndroidBuffer;


// define opengl-wrapper private global variables

Pfs *pfs = NULL;

AndroidBuffer android;


// delcare opengl-wrapper private functions

static void initPfs();

static void add2dVertexToAndroid(GLdouble x, GLdouble y);

static void add3dVertexToAndroid(GLdouble x, GLdouble y, GLdouble z);

static void add3dNormalToAndroid(GLdouble x, GLdouble y, GLdouble z);

static void add2dTexCoordToAndroid(GLdouble x, GLdouble y);



// define opengl-wrapper private functions

static void initPfs()
{
	if(!OPENGL_USE_PFS) {
		return;
	}

	pfs = (Pfs *)malloc(sizeof(Pfs));

	pfs_init(pfs, OPENGL_PFS_WRITE_LOG, OPENGL_PFS_LOG_FILENAME);
}

static void add2dVertexToAndroid(GLdouble x, GLdouble y)
{
	int ref = 0;

	if((!android.is2d) && (android.vertices != NULL)) {
		return;
	}

	android.is2d = atrue;

	if(android.vertices == NULL) {
		ref = 0;
		android.vertexLength = 2;
		android.vertices = (float *)malloc(sizeof(float) *
				android.vertexLength);
	}
	else {
		ref = android.vertexLength;
		android.vertexLength += 2;
		android.vertices = (float *)realloc(android.vertices,
				(sizeof(float) * android.vertexLength));
	}

	android.vertices[(ref + 0)] = (float)x;
	android.vertices[(ref + 1)] = (float)y;
}

static void add3dVertexToAndroid(GLdouble x, GLdouble y, GLdouble z)
{
	int ref = 0;

	if((android.is2d) && (android.vertices != NULL)) {
		return;
	}

	android.is2d = afalse;

	if(android.vertices == NULL) {
		ref = 0;
		android.vertexLength = 3;
		android.vertices = (float *)malloc(sizeof(float) *
				android.vertexLength);
	}
	else {
		ref = android.vertexLength;
		android.vertexLength += 3;
		android.vertices = (float *)realloc(android.vertices,
				(sizeof(float) * android.vertexLength));
	}

	android.vertices[(ref + 0)] = (float)x;
	android.vertices[(ref + 1)] = (float)y;
	android.vertices[(ref + 2)] = (float)z;
}

static void add3dNormalToAndroid(GLdouble x, GLdouble y, GLdouble z)
{
	int ref = 0;

	if(android.is2d) {
		return;
	}

	if(android.normals == NULL) {
		ref = 0;
		android.normalLength = 3;
		android.normals = (float *)malloc(sizeof(float) *
				android.normalLength);
	}
	else {
		ref = android.normalLength;
		android.normalLength += 3;
		android.normals = (float *)realloc(android.normals,
				(sizeof(float) * android.vertexLength));
	}

	android.normals[(ref + 0)] = (float)x;
	android.normals[(ref + 1)] = (float)y;
	android.normals[(ref + 2)] = (float)z;
}

static void add2dTexCoordToAndroid(GLdouble x, GLdouble y)
{
	int ref = 0;

	if(android.texCoords == NULL) {
		ref = 0;
		android.texCoordLength = 2;
		android.texCoords = (float *)malloc(sizeof(float) *
				android.texCoordLength);
	}
	else {
		ref = android.texCoordLength;
		android.texCoordLength += 2;
		android.texCoords = (float *)realloc(android.texCoords,
				(sizeof(float) * android.texCoordLength));
	}

	android.texCoords[(ref + 0)] = (float)x;
	android.texCoords[(ref + 1)] = (float)y;
}


// define opengl-wrapper public functions

Pfs *asgard_getPfs()
{
	if(OPENGL_USE_PFS) {
		if(pfs == NULL) {
			initPfs();
		}
		return pfs;
	}
	return NULL;
}

// gl functions

void android_gluPerspective(double yFov, double aspect, double zNear,
		double zFar)
{
	double xMin = 0.0;
	double xMax = 0.0;
	double yMin = 0.0;
	double yMax = 0.0;

	yMax = (zNear * tan(yFov * math_PI / 360.0));

	yMin = -yMax;

	xMin = (yMin * aspect);

	xMax = (yMax * aspect);

#if defined(__ANDROID__)
	glFrustumf(xMin, xMax, yMin, yMax, zNear, zFar);
#endif // __ANDROID__
}

int asgard_glDoNothing()
{
	return 0;
}

void asgard_glBegin(GLenum mode, const char *file, const char *function)
{
	if(OPENGL_USE_PFS) {
		if(pfs == NULL) {
			initPfs();
		}
		pfs_profile(pfs, file, __FUNCTION__);
	}
	if(OPENGL_REPORT_USAGE) {
		printf("%s:%s()->%s()\n", (char *)file, (char *)function,
				__FUNCTION__);
	}
#if !defined(__ANDROID__)
	glBegin(mode);
#else // __ANDROID__
	if(android.vertices != NULL) {
		free(android.vertices);
	}

	if(android.normals != NULL) {
		free(android.normals);
	}

	if(android.texCoords != NULL) {
		free(android.texCoords);
	}

	memset((void *)&android, 0, (int)(sizeof(AndroidBuffer)));

	android.is2d = afalse;
	android.type = (unsigned int)mode;
	android.vertexLength = 0;
	android.normalLength = 0;
	android.texCoordLength = 0;
	android.vertices = NULL;
	android.normals = NULL;
	android.texCoords = NULL;
#endif // !__ANDROID__
}

void asgard_glBindTexture(GLenum target, GLuint texture, const char *file,
		const char *function)
{
	if(OPENGL_USE_PFS) {
		if(pfs == NULL) {
			initPfs();
		}
		pfs_profile(pfs, file, __FUNCTION__);
	}
	if(OPENGL_REPORT_USAGE) {
		printf("%s:%s()->%s()\n", (char *)file, (char *)function,
				__FUNCTION__);
	}
	glBindTexture(target, texture);
}

void asgard_glBlendFunc(GLenum sfactor, GLenum dfactor, const char *file,
		const char *function)
{
	if(OPENGL_USE_PFS) {
		if(pfs == NULL) {
			initPfs();
		}
		pfs_profile(pfs, file, __FUNCTION__);
	}
	if(OPENGL_REPORT_USAGE) {
		printf("%s:%s()->%s()\n", (char *)file, (char *)function,
				__FUNCTION__);
	}
	glBlendFunc(sfactor, dfactor);
}

void asgard_glCallList(GLuint list, const char *file, const char *function)
{
	if(OPENGL_USE_PFS) {
		if(pfs == NULL) {
			initPfs();
		}
		pfs_profile(pfs, file, __FUNCTION__);
	}
	if(OPENGL_REPORT_USAGE) {
		printf("%s:%s()->%s()\n", (char *)file, (char *)function,
				__FUNCTION__);
	}
	glCallList(list);
}

void asgard_glCallLists(GLsizei n, GLenum type, const GLvoid *lists,
		const char *file, const char *function)
{
	if(OPENGL_USE_PFS) {
		if(pfs == NULL) {
			initPfs();
		}
		pfs_profile(pfs, file, __FUNCTION__);
	}
	if(OPENGL_REPORT_USAGE) {
		printf("%s:%s()->%s()\n", (char *)file, (char *)function,
				__FUNCTION__);
	}
	glCallLists(n, type, lists);
}

void asgard_glClear(GLbitfield mask, const char *file, const char *function)
{
	if(OPENGL_USE_PFS) {
		if(pfs == NULL) {
			initPfs();
		}
		pfs_profile(pfs, file, __FUNCTION__);
	}
	if(OPENGL_REPORT_USAGE) {
		printf("%s:%s()->%s()\n", (char *)file, (char *)function,
				__FUNCTION__);
	}
	glClear(mask);
}

void asgard_glClearColor(GLclampf red, GLclampf green, GLclampf blue,
		GLclampf alpha, const char *file, const char *function)
{
	if(OPENGL_USE_PFS) {
		if(pfs == NULL) {
			initPfs();
		}
		pfs_profile(pfs, file, __FUNCTION__);
	}
	if(OPENGL_REPORT_USAGE) {
		printf("%s:%s()->%s()\n", (char *)file, (char *)function,
				__FUNCTION__);
	}
	glClearColor(red, green, blue, alpha);
}

void asgard_glClearDepth(GLclampd depth, const char *file,
		const char *function)
{
	if(OPENGL_USE_PFS) {
		if(pfs == NULL) {
			initPfs();
		}
		pfs_profile(pfs, file, __FUNCTION__);
	}
	if(OPENGL_REPORT_USAGE) {
		printf("%s:%s()->%s()\n", (char *)file, (char *)function,
				__FUNCTION__);
	}
	glClearDepth(depth);
}

void asgard_glColor4d(GLdouble red, GLdouble green, GLdouble blue,
		GLdouble alpha, const char *file, const char *function)
{
	if(OPENGL_USE_PFS) {
		if(pfs == NULL) {
			initPfs();
		}
		pfs_profile(pfs, file, __FUNCTION__);
	}
	if(OPENGL_REPORT_USAGE) {
		printf("%s:%s()->%s()\n", (char *)file, (char *)function,
				__FUNCTION__);
	}
	glColor4d(red, green, blue, alpha);
}

void asgard_glColorMaterial(GLenum face, GLenum mode, const char *file,
		const char *function)
{
	if(OPENGL_USE_PFS) {
		if(pfs == NULL) {
			initPfs();
		}
		pfs_profile(pfs, file, __FUNCTION__);
	}
	if(OPENGL_REPORT_USAGE) {
		printf("%s:%s()->%s()\n", (char *)file, (char *)function,
				__FUNCTION__);
	}
	glColorMaterial(face, mode);
}

void asgard_glDeleteLists(GLuint list, GLsizei range, const char *file,
		const char *function)
{
	if(OPENGL_USE_PFS) {
		if(pfs == NULL) {
			initPfs();
		}
		pfs_profile(pfs, file, __FUNCTION__);
	}
	if(OPENGL_REPORT_USAGE) {
		printf("%s:%s()->%s()\n", (char *)file, (char *)function,
				__FUNCTION__);
	}
	glDeleteLists(list, range);
}

void asgard_glDeleteTextures(GLsizei n, const GLuint *textures,
		const char *file, const char *function)
{
	if(OPENGL_USE_PFS) {
		if(pfs == NULL) {
			initPfs();
		}
		pfs_profile(pfs, file, __FUNCTION__);
	}
	if(OPENGL_REPORT_USAGE) {
		printf("%s:%s()->%s()\n", (char *)file, (char *)function,
				__FUNCTION__);
	}
	glDeleteTextures(n, textures);
}

void asgard_glDepthFunc(GLenum func, const char *file, const char *function)
{
	if(OPENGL_USE_PFS) {
		if(pfs == NULL) {
			initPfs();
		}
		pfs_profile(pfs, file, __FUNCTION__);
	}
	if(OPENGL_REPORT_USAGE) {
		printf("%s:%s()->%s()\n", (char *)file, (char *)function,
				__FUNCTION__);
	}
	glDepthFunc(func);
}

void asgard_glDisable(GLenum cap, const char *file, const char *function)
{
	if(OPENGL_USE_PFS) {
		if(pfs == NULL) {
			initPfs();
		}
		pfs_profile(pfs, file, __FUNCTION__);
	}
	if(OPENGL_REPORT_USAGE) {
		printf("%s:%s()->%s()\n", (char *)file, (char *)function,
				__FUNCTION__);
	}
	glDisable(cap);
}

void asgard_glEnable(GLenum cap, const char *file, const char *function)
{
	if(OPENGL_USE_PFS) {
		if(pfs == NULL) {
			initPfs();
		}
		pfs_profile(pfs, file, __FUNCTION__);
	}
	if(OPENGL_REPORT_USAGE) {
		printf("%s:%s()->%s()\n", (char *)file, (char *)function,
				__FUNCTION__);
	}
	glEnable(cap);
}

void asgard_glEnd(const char *file, const char *function)
{
#if defined(__ANDROID__)
	int ii = 0;
	int nn = 0;
	int length = 0;
	int indexLength = 0;
	unsigned char *indices = NULL;
#endif // __ANDROID__

	if(OPENGL_USE_PFS) {
		if(pfs == NULL) {
			initPfs();
		}
		pfs_profile(pfs, file, __FUNCTION__);
	}
	if(OPENGL_REPORT_USAGE) {
		printf("%s:%s()->%s()\n", (char *)file, (char *)function,
				__FUNCTION__);
	}
#if !defined(__ANDROID__)
	glEnd();
#else // __ANDROID__
	if((android.vertexLength <= 0) || (android.vertices == NULL)) {
		return;
	}

	glEnableClientState(GL_VERTEX_ARRAY);

	if(android.normals != NULL) {
		glEnableClientState(GL_NORMAL_ARRAY);
	}

	if(android.texCoords != NULL) {
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	}

	if((android.type == GL_QUADS) || (android.type == GL_QUAD_STRIP)) {
		if(android.is2d) {
			indexLength = (((android.vertexLength / 2) / 4) * 6);
		}
		else {
			indexLength = (((android.vertexLength / 3) / 4) * 6);
		}

		indices = (unsigned char *)malloc(sizeof(unsigned char) *
				(indexLength + 1));

		for(ii = 0, nn = 0; ii < indexLength; ii += 6, nn += 4) {
			// first triangle

			indices[(ii + 0)] = (nn + 0);
			indices[(ii + 1)] = (nn + 1);
			indices[(ii + 2)] = (nn + 3);

			// second triangle

/*			indices[(ii + 3)] = (nn + 0);
			indices[(ii + 4)] = (nn + 3);
			indices[(ii + 5)] = (nn + 2);*/
			indices[(ii + 3)] = (nn + 1);
			indices[(ii + 4)] = (nn + 2);
			indices[(ii + 5)] = (nn + 3);
		}
	}

/*	switch(android.type) {
		case GL_LINES:
		case GL_LINE_LOOP:
		case GL_LINE_STRIP:
		case GL_POINTS:
			length = (android.vertexLength / 2);
			break;

		case GL_POLYGON:
			length = 0;
			break;

		case GL_QUADS:
		case GL_QUAD_STRIP:
			length = (android.vertexLength / 4);
			break;

		case GL_TRIANGLES:
		case GL_TRIANGLE_FAN:
		case GL_TRIANGLE_STRIP:
			length = (android.vertexLength / 3);
			break;

		default:
			length = 0;
	}*/

	if(android.is2d) {
		glVertexPointer(2,
				GL_FLOAT,
				0,
				android.vertices);
		length = (android.vertexLength / 2);
	}
	else {
		glVertexPointer(3,
				GL_FLOAT,
				0,
				android.vertices);
		length = (android.vertexLength / 3);
	}

	if(android.normals != NULL) {
		glNormalPointer(GL_FLOAT, 0, android.normals);
	}

	if(android.texCoords != NULL) {
		glTexCoordPointer(2, GL_FLOAT, 0, android.texCoords);
	}

	if(indices == NULL) {
		glDrawArrays(android.type,
				0,
				length);
	}
	else {
		glDrawElements(GL_TRIANGLES, indexLength, GL_UNSIGNED_BYTE, indices);
	}

	glDisableClientState(GL_VERTEX_ARRAY);

	if(android.normals != NULL) {
		glDisableClientState(GL_NORMAL_ARRAY);
	}

	if(android.texCoords != NULL) {
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}

	if(indices != NULL) {
		free(indices);
	}

/*{
	int ii = 0;

	printf("%s() :: DRAW { is 2D %i, type %i, length %i, array %i }\n", __FUNCTION__, (int)android.is2d, (int)android.type, length, android.vertexLength);

	for(ii = 0; ii < android.vertexLength; ii++) {
		printf("%s() :: DRAW { #%03i => %0.2f }\n", __FUNCTION__, ii, android.vertices[ii]);
	}
}*/
#endif // !__ANDROID__
}

void asgard_glEndList(const char *file, const char *function)
{
	if(OPENGL_USE_PFS) {
		if(pfs == NULL) {
			initPfs();
		}
		pfs_profile(pfs, file, __FUNCTION__);
	}
	if(OPENGL_REPORT_USAGE) {
		printf("%s:%s()->%s()\n", (char *)file, (char *)function,
				__FUNCTION__);
	}
	glEndList();
}

GLenum asgard_glGetError(const char *file, const char *function)
{
	if(OPENGL_USE_PFS) {
		if(pfs == NULL) {
			initPfs();
		}
		pfs_profile(pfs, file, __FUNCTION__);
	}
	if(OPENGL_REPORT_USAGE) {
		printf("%s:%s()->%s()\n", (char *)file, (char *)function,
				__FUNCTION__);
	}
	return glGetError();
}

void asgard_glGetDoublev(GLenum pname, GLdouble *params, const char *file,
		const char *function)
{
	if(OPENGL_USE_PFS) {
		if(pfs == NULL) {
			initPfs();
		}
		pfs_profile(pfs, file, __FUNCTION__);
	}
	if(OPENGL_REPORT_USAGE) {
		printf("%s:%s()->%s()\n", (char *)file, (char *)function,
				__FUNCTION__);
	}
	glGetDoublev(pname, params);
}

const GLubyte *asgard_glGetString(GLenum name, const char *file,
		const char *function)
{
	if(OPENGL_USE_PFS) {
		if(pfs == NULL) {
			initPfs();
		}
		pfs_profile(pfs, file, __FUNCTION__);
	}
	if(OPENGL_REPORT_USAGE) {
		printf("%s:%s()->%s()\n", (char *)file, (char *)function,
				__FUNCTION__);
	}
	return glGetString(name);
}

GLuint asgard_glGenLists(GLsizei range, const char *file,
		const char *function)
{
	if(OPENGL_USE_PFS) {
		if(pfs == NULL) {
			initPfs();
		}
		pfs_profile(pfs, file, __FUNCTION__);
	}
	if(OPENGL_REPORT_USAGE) {
		printf("%s:%s()->%s()\n", (char *)file, (char *)function,
				__FUNCTION__);
	}
	return glGenLists(range);
}

void asgard_glGenTextures(GLsizei n, GLuint *textures, const char *file,
		const char *function)
{
	if(OPENGL_USE_PFS) {
		if(pfs == NULL) {
			initPfs();
		}
		pfs_profile(pfs, file, __FUNCTION__);
	}
	if(OPENGL_REPORT_USAGE) {
		printf("%s:%s()->%s()\n", (char *)file, (char *)function,
				__FUNCTION__);
	}
	glGenTextures(n, textures);
}

void asgard_glFrontFace(GLenum mode, const char *file, const char *function)
{
	if(OPENGL_USE_PFS) {
		if(pfs == NULL) {
			initPfs();
		}
		pfs_profile(pfs, file, __FUNCTION__);
	}
	if(OPENGL_REPORT_USAGE) {
		printf("%s:%s()->%s()\n", (char *)file, (char *)function,
				__FUNCTION__);
	}
	glFrontFace(mode);
}

void asgard_glHint(GLenum target, GLenum mode, const char *file,
		const char *function)
{
	if(OPENGL_USE_PFS) {
		if(pfs == NULL) {
			initPfs();
		}
		pfs_profile(pfs, file, __FUNCTION__);
	}
	if(OPENGL_REPORT_USAGE) {
		printf("%s:%s()->%s()\n", (char *)file, (char *)function,
				__FUNCTION__);
	}
	glHint(target, mode);
}

GLboolean asgard_glIsEnabled(GLenum cap, const char *file,
		const char *function)
{
	if(OPENGL_USE_PFS) {
		if(pfs == NULL) {
			initPfs();
		}
		pfs_profile(pfs, file, __FUNCTION__);
	}
	if(OPENGL_REPORT_USAGE) {
		printf("%s:%s()->%s()\n", (char *)file, (char *)function,
				__FUNCTION__);
	}
	return glIsEnabled(cap);
}

void asgard_glLightf(GLenum light, GLenum pname, GLfloat param,
		const char *file, const char *function)
{
	if(OPENGL_USE_PFS) {
		if(pfs == NULL) {
			initPfs();
		}
		pfs_profile(pfs, file, __FUNCTION__);
	}
	if(OPENGL_REPORT_USAGE) {
		printf("%s:%s()->%s()\n", (char *)file, (char *)function,
				__FUNCTION__);
	}
	glLightf(light, pname, param);
}

void asgard_glLightfv(GLenum light, GLenum pname, const GLfloat *params,
		const char *file, const char *function)
{
	if(OPENGL_USE_PFS) {
		if(pfs == NULL) {
			initPfs();
		}
		pfs_profile(pfs, file, __FUNCTION__);
	}
	if(OPENGL_REPORT_USAGE) {
		printf("%s:%s()->%s()\n", (char *)file, (char *)function,
				__FUNCTION__);
	}
	glLightfv(light, pname, params);
}

void asgard_glLineWidth(GLfloat width, const char *file, const char *function)
{
	if(OPENGL_USE_PFS) {
		if(pfs == NULL) {
			initPfs();
		}
		pfs_profile(pfs, file, __FUNCTION__);
	}
	if(OPENGL_REPORT_USAGE) {
		printf("%s:%s()->%s()\n", (char *)file, (char *)function,
				__FUNCTION__);
	}
	glLineWidth(width);
}

void asgard_glListBase(GLuint base, const char *file, const char *function)
{
	if(OPENGL_USE_PFS) {
		if(pfs == NULL) {
			initPfs();
		}
		pfs_profile(pfs, file, __FUNCTION__);
	}
	if(OPENGL_REPORT_USAGE) {
		printf("%s:%s()->%s()\n", (char *)file, (char *)function,
				__FUNCTION__);
	}
	glListBase(base);
}

void asgard_glLoadIdentity(const char *file, const char *function)
{
	if(OPENGL_USE_PFS) {
		if(pfs == NULL) {
			initPfs();
		}
		pfs_profile(pfs, file, __FUNCTION__);
	}
	if(OPENGL_REPORT_USAGE) {
		printf("%s:%s()->%s()\n", (char *)file, (char *)function,
				__FUNCTION__);
	}
	glLoadIdentity();
}

void asgard_glMateriali(GLenum face, GLenum pname, GLint param,
		const char *file, const char *function)
{
	if(OPENGL_USE_PFS) {
		if(pfs == NULL) {
			initPfs();
		}
		pfs_profile(pfs, file, __FUNCTION__);
	}
	if(OPENGL_REPORT_USAGE) {
		printf("%s:%s()->%s()\n", (char *)file, (char *)function,
				__FUNCTION__);
	}
	glMateriali(face, pname, param);
}

void asgard_glMatrixMode(GLenum mode, const char *file, const char *function)
{
	if(OPENGL_USE_PFS) {
		if(pfs == NULL) {
			initPfs();
		}
		pfs_profile(pfs, file, __FUNCTION__);
	}
	if(OPENGL_REPORT_USAGE) {
		printf("%s:%s()->%s()\n", (char *)file, (char *)function,
				__FUNCTION__);
	}
	glMatrixMode(mode);
}

void asgard_glNewList(GLuint list, GLenum mode, const char *file,
		const char *function)
{
	if(OPENGL_USE_PFS) {
		if(pfs == NULL) {
			initPfs();
		}
		pfs_profile(pfs, file, __FUNCTION__);
	}
	if(OPENGL_REPORT_USAGE) {
		printf("%s:%s()->%s()\n", (char *)file, (char *)function,
				__FUNCTION__);
	}
	glNewList(list, mode);
}

void asgard_glNormal3d(GLdouble nx, GLdouble ny, GLdouble nz,
		const char *file, const char *function)
{
	if(OPENGL_USE_PFS) {
		if(pfs == NULL) {
			initPfs();
		}
		pfs_profile(pfs, file, __FUNCTION__);
	}
	if(OPENGL_REPORT_USAGE) {
		printf("%s:%s()->%s()\n", (char *)file, (char *)function,
				__FUNCTION__);
	}
#if !defined(__ANDROID__)
	glNormal3d(nx, ny, nz);
#else // __ANDROID__
	add3dNormalToAndroid(nx, ny, nz);
#endif // !__ANDROID__
}

void asgard_glOrtho(GLdouble left, GLdouble right, GLdouble bottom,
		GLdouble top, GLdouble nearVal, GLdouble farVal, const char *file,
		const char *function)
{
	if(OPENGL_USE_PFS) {
		if(pfs == NULL) {
			initPfs();
		}
		pfs_profile(pfs, file, __FUNCTION__);
	}
	if(OPENGL_REPORT_USAGE) {
		printf("%s:%s()->%s()\n", (char *)file, (char *)function,
				__FUNCTION__);
	}
	glOrtho(left, right, bottom, top, nearVal, farVal);
}

void asgard_glPopAttrib(const char *file, const char *function)
{
	if(OPENGL_USE_PFS) {
		if(pfs == NULL) {
			initPfs();
		}
		pfs_profile(pfs, file, __FUNCTION__);
	}
	if(OPENGL_REPORT_USAGE) {
		printf("%s:%s()->%s()\n", (char *)file, (char *)function,
				__FUNCTION__);
	}
	glPopAttrib();
}

void asgard_glPopMatrix(const char *file, const char *function)
{
	if(OPENGL_USE_PFS) {
		if(pfs == NULL) {
			initPfs();
		}
		pfs_profile(pfs, file, __FUNCTION__);
	}
	if(OPENGL_REPORT_USAGE) {
		printf("%s:%s()->%s()\n", (char *)file, (char *)function,
				__FUNCTION__);
	}
	glPopMatrix();
}

void asgard_glPushAttrib(GLbitfield mask, const char *file,
		const char *function)
{
	if(OPENGL_USE_PFS) {
		if(pfs == NULL) {
			initPfs();
		}
		pfs_profile(pfs, file, __FUNCTION__);
	}
	if(OPENGL_REPORT_USAGE) {
		printf("%s:%s()->%s()\n", (char *)file, (char *)function,
				__FUNCTION__);
	}
	glPushAttrib(mask);
}

void asgard_glPushMatrix(const char *file, const char *function)
{
	if(OPENGL_USE_PFS) {
		if(pfs == NULL) {
			initPfs();
		}
		pfs_profile(pfs, file, __FUNCTION__);
	}
	if(OPENGL_REPORT_USAGE) {
		printf("%s:%s()->%s()\n", (char *)file, (char *)function,
				__FUNCTION__);
	}
	glPushMatrix();
}

void asgard_glRasterPos4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w,
		const char *file, const char *function)
{
	if(OPENGL_USE_PFS) {
		if(pfs == NULL) {
			initPfs();
		}
		pfs_profile(pfs, file, __FUNCTION__);
	}
	if(OPENGL_REPORT_USAGE) {
		printf("%s:%s()->%s()\n", (char *)file, (char *)function,
				__FUNCTION__);
	}
	glRasterPos4d(x, y, z, w);
}

void asgard_glRotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z,
		const char *file, const char *function)
{
	if(OPENGL_USE_PFS) {
		if(pfs == NULL) {
			initPfs();
		}
		pfs_profile(pfs, file, __FUNCTION__);
	}
	if(OPENGL_REPORT_USAGE) {
		printf("%s:%s()->%s()\n", (char *)file, (char *)function,
				__FUNCTION__);
	}
	glRotated(angle, x, y, z);
}

void asgard_glShadeModel(GLenum mode, const char *file, const char *function)
{
	if(OPENGL_USE_PFS) {
		if(pfs == NULL) {
			initPfs();
		}
		pfs_profile(pfs, file, __FUNCTION__);
	}
	if(OPENGL_REPORT_USAGE) {
		printf("%s:%s()->%s()\n", (char *)file, (char *)function,
				__FUNCTION__);
	}
	glShadeModel(mode);
}

void asgard_glTexCoord2d(GLdouble s, GLdouble t, const char *file,
		const char *function)
{
	if(OPENGL_USE_PFS) {
		if(pfs == NULL) {
			initPfs();
		}
		pfs_profile(pfs, file, __FUNCTION__);
	}
	if(OPENGL_REPORT_USAGE) {
		printf("%s:%s()->%s()\n", (char *)file, (char *)function,
				__FUNCTION__);
	}
#if !defined(__ANDROID__)
	glTexCoord2d(s, t);
#else // __ANDROID__
	add2dTexCoordToAndroid(s, t);
#endif // !__ANDROID__
}

void asgard_glTexEnvi(GLenum target, GLenum pname, GLint param,
		const char *file, const char *function)
{
	if(OPENGL_USE_PFS) {
		if(pfs == NULL) {
			initPfs();
		}
		pfs_profile(pfs, file, __FUNCTION__);
	}
	if(OPENGL_REPORT_USAGE) {
		printf("%s:%s()->%s()\n", (char *)file, (char *)function,
				__FUNCTION__);
	}
	glTexEnvi(target, pname, param);
}

void asgard_glTexImage2D(GLenum target, GLint level, GLint internalFormat,
		GLsizei width, GLsizei height, GLint border, GLenum format,
		GLenum type, const GLvoid *data, const char *file,
		const char *function)
{
	if(OPENGL_USE_PFS) {
		if(pfs == NULL) {
			initPfs();
		}
		pfs_profile(pfs, file, __FUNCTION__);
	}
	if(OPENGL_REPORT_USAGE) {
		printf("%s:%s()->%s()\n", (char *)file, (char *)function,
				__FUNCTION__);
	}
	glTexImage2D(target, level, internalFormat, width, height, border, format,
			type, data);
}

void asgard_glTexParameteri(GLenum target, GLenum pname, GLint param,
		const char *file, const char *function)
{
	if(OPENGL_USE_PFS) {
		if(pfs == NULL) {
			initPfs();
		}
		pfs_profile(pfs, file, __FUNCTION__);
	}
	if(OPENGL_REPORT_USAGE) {
		printf("%s:%s()->%s()\n", (char *)file, (char *)function,
				__FUNCTION__);
	}
	glTexParameteri(target, pname, param);
}

void asgard_glTexParameterf(GLenum target, GLenum pname, GLfloat param,
		const char *file, const char *function)
{
	if(OPENGL_USE_PFS) {
		if(pfs == NULL) {
			initPfs();
		}
		pfs_profile(pfs, file, __FUNCTION__);
	}
	if(OPENGL_REPORT_USAGE) {
		printf("%s:%s()->%s()\n", (char *)file, (char *)function,
				__FUNCTION__);
	}
	glTexParameterf(target, pname, param);
}

void asgard_glTexParameterfv(GLenum target, GLenum pname, GLfloat *params,
		const char *file, const char *function)
{
	if(OPENGL_USE_PFS) {
		if(pfs == NULL) {
			initPfs();
		}
		pfs_profile(pfs, file, __FUNCTION__);
	}
	if(OPENGL_REPORT_USAGE) {
		printf("%s:%s()->%s()\n", (char *)file, (char *)function,
				__FUNCTION__);
	}
	glTexParameterfv(target, pname, params);
}

void asgard_glTranslated(GLdouble x, GLdouble y, GLdouble z, const char *file,
		const char *function)
{
	if(OPENGL_USE_PFS) {
		if(pfs == NULL) {
			initPfs();
		}
		pfs_profile(pfs, file, __FUNCTION__);
	}
	if(OPENGL_REPORT_USAGE) {
		printf("%s:%s()->%s()\n", (char *)file, (char *)function,
				__FUNCTION__);
	}
	glTranslated(x, y, z);
}

void asgard_glViewport(GLint x, GLint y, GLsizei width, GLsizei height,
		const char *file, const char *function)
{
	if(OPENGL_USE_PFS) {
		if(pfs == NULL) {
			initPfs();
		}
		pfs_profile(pfs, file, __FUNCTION__);
	}
	if(OPENGL_REPORT_USAGE) {
		printf("%s:%s()->%s()\n", (char *)file, (char *)function,
				__FUNCTION__);
	}
	glViewport(x, y, width, height);
}

void asgard_glVertex2d(GLdouble x, GLdouble y, const char *file,
		const char *function)
{
	if(OPENGL_USE_PFS) {
		if(pfs == NULL) {
			initPfs();
		}
		pfs_profile(pfs, file, __FUNCTION__);
	}
	if(OPENGL_REPORT_USAGE) {
		printf("%s:%s()->%s()\n", (char *)file, (char *)function,
				__FUNCTION__);
	}
#if !defined(__ANDROID__)
	glVertex2d(x, y);
#else // __ANDROID__
	add2dVertexToAndroid(x, y);
#endif // !__ANDROID__
}

void asgard_glVertex3d(GLdouble x, GLdouble y, GLdouble z,
		const char *file, const char *function)
{
	if(OPENGL_USE_PFS) {
		if(pfs == NULL) {
			initPfs();
		}
		pfs_profile(pfs, file, __FUNCTION__);
	}
	if(OPENGL_REPORT_USAGE) {
		printf("%s:%s()->%s()\n", (char *)file, (char *)function,
				__FUNCTION__);
	}
#if !defined(__ANDROID__)
	glVertex3d(x, y, z);
#else // __ANDROID__
	add3dVertexToAndroid(x, y, z);
#endif // !__ANDROID__
}

// glu functions

GLint asgard_gluBuild2DMipmaps(GLenum target, GLint internalFormat,
		GLsizei width, GLsizei height, GLenum format, GLenum type,
		const void *data, const char *file, const char *function)
{
	if(OPENGL_USE_PFS) {
		if(pfs == NULL) {
			initPfs();
		}
		pfs_profile(pfs, file, __FUNCTION__);
	}
	if(OPENGL_REPORT_USAGE) {
		printf("%s:%s()->%s()\n", (char *)file, (char *)function,
				__FUNCTION__);
	}
	return gluBuild2DMipmaps(target, internalFormat, width, height, format,
			type, data);
}

void asgard_gluLookAt(GLdouble eyeX, GLdouble eyeY, GLdouble eyeZ,
		GLdouble centerX, GLdouble centerY, GLdouble centerZ,
		GLdouble upX, GLdouble upY, GLdouble upZ,
		const char *file, const char *function)
{
	if(OPENGL_USE_PFS) {
		if(pfs == NULL) {
			initPfs();
		}
		pfs_profile(pfs, file, __FUNCTION__);
	}
	if(OPENGL_REPORT_USAGE) {
		printf("%s:%s()->%s()\n", (char *)file, (char *)function,
				__FUNCTION__);
	}
	gluLookAt(eyeX, eyeY, eyeZ, centerX, centerY, centerZ, upX, upY, upZ);
}

void asgard_gluPerspective(GLdouble fovy, GLdouble aspect, GLdouble znear,
		GLdouble zfar, const char *file, const char *function)
{
	if(OPENGL_USE_PFS) {
		if(pfs == NULL) {
			initPfs();
		}
		pfs_profile(pfs, file, __FUNCTION__);
	}
	if(OPENGL_REPORT_USAGE) {
		printf("%s:%s()->%s()\n", (char *)file, (char *)function,
				__FUNCTION__);
	}
	gluPerspective(fovy, aspect, znear, zfar);
}

