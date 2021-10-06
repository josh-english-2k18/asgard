/*
 * win32api.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Windows application programming interface to work with the OpenGL
 * library, header file.
 *
 * Written by Josh English.
 */

#if !defined(_ASGARD_CORE_PLUGIN_WIN32API_H)

#define _ASGARD_CORE_PLUGIN_WIN32API_H

#if !defined(_ASGARD_H) && !defined(_ASGARD_CORE_PLUGIN_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _ASGARD_H || _ASGARD_CORE_PLUGIN_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


#if defined(WIN32)

// define win32api public data types

typedef struct _Win32ApiWindow {
	aboolean wasCreated;
	aboolean isOpenGLEnabled;
	aboolean isSecondOpenGLEnabled;
	HDC hdc;
	HGLRC hglrcOne;
	HGLRC hglrcTwo;
	HWND hwnd;
	WNDPROC subclassHandle;
	HINSTANCE hinstance;
	Log *log;
	Mutex mutex;
} Win32ApiWindow;

// delcare win32api public functions

int win32api_createWindow(Win32ApiWindow *window, char *name,
		aboolean isFullscreenMode, int screenWidth, int screenHeight,
		int colorBits, Log *log);

int win32api_initWithinWindow(Win32ApiWindow *window, void *handle, char *name,
		aboolean isFullscreenMode, int screenWidth, int screenHeight,
		int colorBits, Log *log);

void win32api_destroyWindow(Win32ApiWindow *window);

void *win32api_enableOpenGL(aboolean isMultiThreaded, int colorBits,
		Win32ApiWindow *window);

void win32api_makeOpenGLCurrent(Win32ApiWindow *window, int threadType);

void win32api_disableOpenGL(Win32ApiWindow *window);

int win32api_checkState(Win32ApiWindow *window, aboolean *exitApplication);

void win32api_swapScreenBuffer(HDC hdc);

void win32api_setMousePosition(Win32ApiWindow *window, int mouseX, int mouseY);

void win32api_enableMouseDisplay(Win32ApiWindow *window);

void win32api_disableMouseDisplay(Win32ApiWindow *window);

AsgardEvent *win32api_popEvent(Win32ApiWindow *window);

#endif // WIN32


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _ASGARD_CORE_PLUGIN_WIN32API_H

