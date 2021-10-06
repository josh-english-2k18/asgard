/*
 * osapi.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard Game Engine operating system interface wrapper, header file.
 *
 * Written by Josh English.
 */

#if !defined(_ASGARD_CORE_OSAPI_H)

#define _ASGARD_CORE_OSAPI_H

#if !defined(_ASGARD_H) && !defined(_ASGARD_CORE_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _ASGARD_H || _ASGARD_CORE_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

// define osapi public constants

#define OSAPI_THREAD_TYPE_RENDER						1
#define OSAPI_THREAD_TYPE_GAMEPLAY						2


// declare osapi public functions

int osapi_createWindow(char *name, aboolean isFullscreenMode, int screenWidth,
		int screenHeight, int colorBits, Log *log, void **window);

int osapi_initWithinWindow(void *handle, char *name, aboolean isFullscreenMode,
		int screenWidth, int screenHeight, int colorBits, Log *log,
		void **window);

int osapi_destroyWindow(void *window);

void *osapi_enableOpenGL(aboolean isMultiThreaded, int colorBits, void *window);

void osapi_makeOpenGLCurrent(void *window, int threadType);

void osapi_disableOpenGL(void *window);

void *osapi_getWindowHandle(void *window);

int osapi_checkState(void *window, aboolean *exitApplication);

int osapi_swapScreenBuffer(void *screenHandle);

void osapi_setMousePosition(void *window, int mouseX, int mouseY);

void osapi_enableMouseDisplay(void *window);

void osapi_disableMouseDisplay(void *window);

AsgardEvent *osapi_popEvent(void *window);

#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _ASGARD_CORE_OSAPI_H

