/*
 * linuxapi.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Linux application programming interface to work with the OpenGL
 * library, header file.
 *
 * Written by Josh English.
 */

#if !defined(_ASGARD_CORE_PLUGIN_LINUXAPI_H)

#define _ASGARD_CORE_PLUGIN_LINUXAPI_H

#if !defined(_ASGARD_H) && !defined(_ASGARD_CORE_PLUGIN_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _ASGARD_H || _ASGARD_CORE_PLUGIN_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


#if defined(__linux__) && !defined(__ANDROID__)


// define linuxapi public data types

typedef struct _LinuxApiWindow {
	aboolean wasCreated;
	aboolean isOpenGLEnabled;
	aboolean isSecondOpenGLEnabled;
	aboolean exitApplication;
	aboolean isFullscreenMode;
	int screenId;
	int threadId;
	int xPosition;
	int yPosition;
	unsigned int windowWidth;
	unsigned int windowHeight;
	unsigned int bitsPerPixel;
	Display *display;
	Window handle;
	GLXContext contextOne;
	GLXContext contextTwo;
	XVisualInfo *visualInfo;
	XF86VidModeModeInfo videoMode;
	XSetWindowAttributes attributes;
	FifoStack eventQueue;
	TmfContext tmf;
	Mutex mutex;
	Log *log;
} LinuxApiWindow;


// delcare linuxapi public functions

int linuxapi_createWindow(LinuxApiWindow *window, char *name,
		aboolean isFullscreenMode, int screenWidth, int screenHeight,
		int colorBits, Log *log);

void linuxapi_destroyWindow(LinuxApiWindow *window);

void *linuxapi_enableOpenGL(aboolean isMultiThreaded, int colorBits,
		LinuxApiWindow *window);

void linuxapi_makeOpenGLCurrent(LinuxApiWindow *window, int threadType);

void linuxapi_disableOpenGL(LinuxApiWindow *window);

int linuxapi_checkState(LinuxApiWindow *window, aboolean *exitApplication);

void linuxapi_swapScreenBuffer(LinuxApiWindow *window);

void linuxapi_setMousePosition(LinuxApiWindow *window, int mouseX, int mouseY);

void linuxapi_enableMouseDisplay(LinuxApiWindow *window);

void linuxapi_disableMouseDisplay(LinuxApiWindow *window);

AsgardEvent *linuxapi_popEvent(LinuxApiWindow *window);


#endif // __linux__ && !__ANDROID__


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _ASGARD_CORE_PLUGIN_LINUXAPI_H

