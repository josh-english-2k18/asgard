/*
 * osapi.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard Game Engine interface to an OpenGL rendering screen.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#define _ASGARD_CORE_COMPONENT
#include "asgard/core/common.h"
#define _ASGARD_CORE_PLUGIN_COMPONENT
#include "asgard/core/plugin/keys_android.h"
#include "asgard/core/plugin/keys_linux.h"
#include "asgard/core/plugin/keys_windows.h"
#define _ASGARD_CORE_COMPONENT
#include "asgard/core/keys.h"
#include "asgard/core/events.h"
#define _ASGARD_CORE_PLUGIN_COMPONENT
#include "asgard/core/plugin/opengl.h"
#include "asgard/core/plugin/androidapi.h"
#include "asgard/core/plugin/linuxapi.h"
#include "asgard/core/plugin/win32api.h"
#define _ASGARD_CORE_COMPONENT
#include "asgard/core/osapi.h"

// define osapi public functions

int osapi_createWindow(char *name, aboolean isFullscreenMode, int screenWidth,
		int screenHeight, int colorBits, Log *log, void **window)
{
	if((screenWidth < 320) || (screenHeight < 240) ||
			((colorBits != 8) &&
			 (colorBits != 16) &&
			 (colorBits != 24) &&
			 (colorBits != 32)) ||
			(window == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

#if defined(WIN32)
	*window = (void *)malloc(sizeof(Win32ApiWindow));

	if(win32api_createWindow(*window, name, isFullscreenMode, screenWidth,
				screenHeight, colorBits, log) < 0) {
		log_logf(log, LOG_LEVEL_PANIC, "osapi - failed to initialize window");
		return -1;
	}
#elif defined(__linux__) && !defined(__ANDROID__)
	*window = (void *)malloc(sizeof(LinuxApiWindow));

	if(linuxapi_createWindow(*window, name, isFullscreenMode, screenWidth,
				screenHeight, colorBits, log) < 0) {
		log_logf(log, LOG_LEVEL_PANIC, "osapi - failed to initialize window");
		return -1;
	}
#elif defined(__ANDROID__)
	*window = (void *)malloc(sizeof(AndroidApiWindow));

	if(androidapi_createWindow(*window, name, isFullscreenMode, screenWidth,
				screenHeight, colorBits, log) < 0) {
		log_logf(log, LOG_LEVEL_PANIC, "osapi - failed to initialize window");
		return -1;
	}
#else // - no plugins available -
	return -1;
#endif // - plugins -

	return 0;
}

int osapi_initWithinWindow(void *handle, char *name, aboolean isFullscreenMode,
		int screenWidth, int screenHeight, int colorBits, Log *log,
		void **window)
{
	if((handle == NULL) ||
			(screenWidth < 320) || (screenHeight < 240) ||
			((colorBits != 8) &&
			 (colorBits != 16) &&
			 (colorBits != 24) &&
			 (colorBits != 32)) ||
			(window == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

#if defined(WIN32)
	*window = (void *)malloc(sizeof(Win32ApiWindow));

	if(win32api_initWithinWindow(*window, handle, name, isFullscreenMode,
				screenWidth, screenHeight, colorBits, log) < 0) {
		log_logf(log, LOG_LEVEL_PANIC, "osapi - failed to initialize window");
		return -1;
	}
#elif defined(__linux__) && !defined(__ANDROID__)
	/*
	 * TODO: this
	 */
#elif defined(__ANDROID__)
	/*
	 * TODO: this
	 */
#else // - no plugins available -
	return -1;
#endif // - plugins -

	return 0;
}

int osapi_destroyWindow(void *window)
{
	if(window == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

#if defined(WIN32)
	win32api_destroyWindow(window);
#elif defined(__linux__) && !defined(__ANDROID__)
	linuxapi_destroyWindow(window);
#elif defined(__ANDROID__)
	androidapi_destroyWindow(window);
#else // - no plugins available -
	return -1;
#endif // - plugins -

	free(window);

	return 0;
}

void osapi_makeOpenGLCurrent(void *window, int threadType)
{
	if((window == NULL) ||
			((threadType != OSAPI_THREAD_TYPE_RENDER) &&
			 (threadType != OSAPI_THREAD_TYPE_GAMEPLAY))) {
		DISPLAY_INVALID_ARGS;
		return;
	}

#if defined(WIN32)
	win32api_makeOpenGLCurrent(window, threadType);
#elif defined(__linux__) && !defined(__ANDROID__)
	linuxapi_makeOpenGLCurrent(window, threadType);
#elif defined(__ANDROID__)
	androidapi_makeOpenGLCurrent(window, threadType);
#else // - no plugins available -
	return;
#endif // - plugins -
}

void *osapi_enableOpenGL(aboolean isMultiThreaded, int colorBits, void *window)
{
	void *result = NULL;

	if(((colorBits != 8) &&
				(colorBits != 16) &&
				(colorBits != 24) &&
				(colorBits != 32)) ||
			(window == NULL)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

#if defined(WIN32)
	result = win32api_enableOpenGL(isMultiThreaded, colorBits, window);
#elif defined(__linux__) && !defined(__ANDROID__)
	result = linuxapi_enableOpenGL(isMultiThreaded, colorBits, window);
#elif defined(__ANDROID__)
	result = androidapi_enableOpenGL(isMultiThreaded, colorBits, window);
#else // - no plugins available -
	result = NULL;
#endif // - plugins -

	return result;
}

void osapi_disableOpenGL(void *window)
{
	if(window == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

#if defined(WIN32)
	win32api_disableOpenGL(window);
#elif defined(__linux__) && !defined(__ANDROID__)
	linuxapi_disableOpenGL(window);
#elif defined(__ANDROID__)
	androidapi_disableOpenGL(window);
#else // - no plugins available -
	return;
#endif // - plugins -
}

void *osapi_getWindowHandle(void *window)
{
	void *result = NULL;

	if(window == NULL) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

#if defined(WIN32)
	result = (void *)((Win32ApiWindow *)window)->hdc;
#elif defined(__linux__) && !defined(__ANDROID__)
	result = (void *)window;
#elif defined(__ANDROID__)
	result = (void *)window;
#else // - no plugins available -
	return NULL;
#endif // - plugins -

	return result;
}

int osapi_checkState(void *window, aboolean *exitApplication)
{
	int resultState = 0;

	if((window == NULL) || (exitApplication == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

#if defined(WIN32)
	resultState = win32api_checkState(window, exitApplication);
#elif defined(__linux__) && !defined(__ANDROID__)
	resultState = linuxapi_checkState(window, exitApplication);
#elif defined(__ANDROID__)
	resultState = androidapi_checkState(window, exitApplication);
#else // - no plugins available -
	resultState = -1;
#endif // - plugins -

	return resultState;
}

int osapi_swapScreenBuffer(void *screenHandle)
{
	if(screenHandle == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

#if defined(WIN32)
	win32api_swapScreenBuffer(screenHandle);
#elif defined(__linux__) && !defined(__ANDROID__)
	linuxapi_swapScreenBuffer(screenHandle);
#elif defined(__ANDROID__)
	androidapi_swapScreenBuffer(screenHandle);
#else // - no plugin available -
	return -1;
#endif //  - plugins -

	return 0;
}

void osapi_setMousePosition(void *window, int mouseX, int mouseY)
{
#if defined(WIN32)
	win32api_setMousePosition(window, mouseX, mouseY);
#elif defined(__linux__) && !defined(__ANDROID__)
	linuxapi_setMousePosition(window, mouseX, mouseY);
#elif defined(__ANDROID__)
	androidapi_setMousePosition(window, mouseX, mouseY);
#else // - no plugin available -
	return;
#endif //  - plugins -
}

void osapi_enableMouseDisplay(void *window)
{
#if defined(WIN32)
	win32api_enableMouseDisplay(window);
#elif defined(__linux__) && !defined(__ANDROID__)
	linuxapi_enableMouseDisplay(window);
#elif defined(__ANDROID__)
	androidapi_enableMouseDisplay(window);
#else // - no plugin available -
	return;
#endif //  - plugins -
}

void osapi_disableMouseDisplay(void *window)
{
#if defined(WIN32)
	win32api_disableMouseDisplay(window);
#elif defined(__linux__) && !defined(__ANDROID__)
	linuxapi_disableMouseDisplay(window);
#elif defined(__ANDROID__)
	androidapi_disableMouseDisplay(window);
#else // - no plugin available -
	return;
#endif //  - plugins -
}

AsgardEvent *osapi_popEvent(void *window)
{
	AsgardEvent *result = NULL;

	if(window == NULL) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

#if defined(WIN32)
	result = win32api_popEvent(window);
#elif defined(__linux__) && !defined(__ANDROID__)
	result = linuxapi_popEvent(window);
#elif defined(__ANDROID__)
	result = androidapi_popEvent(window);
#else // - no plugin available -
	return NULL;
#endif //  - plugins -

	return result;
}

