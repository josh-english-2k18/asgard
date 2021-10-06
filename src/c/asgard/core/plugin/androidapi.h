/*
 * androidapi.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Android application programming interface to work with the OpenGL
 * library, header file.
 *
 * Written by Josh English.
 */

#if !defined(_ASGARD_CORE_PLUGIN_ANDROIDAPI_H)

#define _ASGARD_CORE_PLUGIN_ANDROIDAPI_H

#if !defined(_ASGARD_H) && !defined(_ASGARD_CORE_PLUGIN_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _ASGARD_H || _ASGARD_CORE_PLUGIN_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


#if defined(__ANDROID__)


// define androidapi public data types

typedef struct _AndroidApiWindow {
	aboolean wasCreated;
	aboolean isOpenGLEnabled;
	aboolean isSecondOpenGLEnabled;
	aboolean exitApplication;
	aboolean isFullscreenMode;
	int xPosition;
	int yPosition;
	unsigned int windowWidth;
	unsigned int windowHeight;
	unsigned int bitsPerPixel;
	FifoStack eventQueue;
	Mutex mutex;
	Log *log;
} AndroidApiWindow;


// delcare androidapi public functions

int androidapi_createWindow(AndroidApiWindow *window, char *name,
		aboolean isFullscreenMode, int screenWidth, int screenHeight,
		int colorBits, Log *log);

void androidapi_destroyWindow(AndroidApiWindow *window);

void *androidapi_enableOpenGL(aboolean isMultiThreaded, int colorBits,
		AndroidApiWindow *window);

void androidapi_makeOpenGLCurrent(AndroidApiWindow *window, int threadType);

void androidapi_disableOpenGL(AndroidApiWindow *window);

int androidapi_checkState(AndroidApiWindow *window, aboolean *exitApplication);

void androidapi_swapScreenBuffer(AndroidApiWindow *window);

void androidapi_setMousePosition(AndroidApiWindow *window, int mouseX,
		int mouseY);

void androidapi_enableMouseDisplay(AndroidApiWindow *window);

void androidapi_disableMouseDisplay(AndroidApiWindow *window);

AsgardEvent *androidapi_popEvent(AndroidApiWindow *window);

aboolean androidapi_pushEvent(AndroidApiWindow *window, AsgardEvent *event);

// helper functions

void *androidapi_zipUtil_openApkArchive(char *filename);

char *androidapi_zipUtil_readFileToBuffer(void *context, char *filename,
		int *bufferLength);

int androidapi_textureUtil_loadTexture(void *context, Log *log,
		void *texture, char *filename);


#endif // __ANDROID__


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _ASGARD_CORE_PLUGIN_ANDROIDAPI_H

