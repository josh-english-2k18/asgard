/*
 * canvas.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard Game Engine interface to an OpenGL rendering screen, header file.
 *
 * Written by Josh English.
 */

#if !defined(_ASGARD_CORE_CANVAS_H)

#define _ASGARD_CORE_CANVAS_H

#if !defined(_ASGARD_H) && !defined(_ASGARD_CORE_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _ASGARD_H || _ASGARD_CORE_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

// define canvas public constants

#define CANVAS_Y_VIEW_ANGLE									45.0

#define CANVAS_NEAR_Z_PLANE									0.01
#define CANVAS_FAR_Z_PLANE									1024.0

#define CANVAS_STATUS_INIT									1001
#define CANVAS_STATUS_OK									1002
#define CANVAS_STATUS_FREED									1003
#define CANVAS_STATUS_ERROR									-1001

#define CANVAS_RENDER_MODE_2D								2001
#define CANVAS_RENDER_MODE_3D								2002

#define CANVAS_DEFAULT_SCREEN_WIDTH							1024

#define CANVAS_DEFAULT_SCREEN_HEIGHT						768

#define CANVAS_DEFAULT_FRAMERATE_SAMPLE_THRESHOLD			60


// define canvas public data types

typedef struct _Canvas {
	aboolean isFullscreenMode;
	aboolean isRenderBegan;
	int status;
	int renderMode;
	int screenWidth;
	int screenHeight;
	int frameCounter;
	int framerateSampleThreshold;
	alint renderTicks;
	double framerate;
	double timestamp;
	char *canvasName;
	char *videoCardVendor;
	char *videoCardRenderer;
	char *videoCardVersion;
	char *videoCardExtensions;
	void *screenHandle;
	Log *log;
} Canvas;


// declare canvas public functions

int canvas_init(Canvas *canvas, char *name, aboolean isFullscreenMode,
		int screenWidth, int screenHeight, void *screenHandle, Log *log);

int canvas_free(Canvas *canvas);

int canvas_getFullscreenMode(Canvas *canvas, aboolean *isFullscreenMode);

int canvas_toggleFullscreen(Canvas *canvas);

int canvas_toggleWindowed(Canvas *canvas);

int canvas_getStatus(Canvas *canvas);

int canvas_getRenderMode(Canvas *canvas);

int canvas_toggleRenderMode2d(Canvas *canvas);

int canvas_toggleRenderMode3d(Canvas *canvas);

int canvas_getScreenDimensions(Canvas *canvas, int *width, int *height);

int canvas_setScreenDimensions(Canvas *canvas, int width, int height);

int canvas_setFramerateSampleThreshold(Canvas *canvas, int threshold);

double canvas_getFramerate(Canvas *canvas);

alint canvas_getRenderTicks(Canvas *canvas);

int canvas_getCanvasName(Canvas *canvas, char **name);

int canvas_getVideoCardVendor(Canvas *canvas, char **vendor);

int canvas_getVideoCardRenderer(Canvas *canvas, char **renderer);

int canvas_getVideoCardVersion(Canvas *canvas, char **version);

int canvas_getVideoCardExtensions(Canvas *canvas, char **extensions);

int canvas_getScreenHandle(Canvas *canvas, void **handle);

int canvas_beginRender(Canvas *canvas);

int canvas_endRender(Canvas *canvas);

void canvas_reportOpenGLErrors(Canvas *canvas, const char *function);


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _ASGARD_CORE_CANVAS_H

