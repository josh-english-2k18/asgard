/*
 * canvas.c
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
#include "asgard/core/plugin/opengl.h"
#include "asgard/core/events.h"
#include "asgard/core/osapi.h"
#include "asgard/core/canvas.h"


// define canvas private functions

static void initOpenglState(Canvas *canvas);

static void setupOpenglScreen(Canvas *canvas);

static void reportOpenGLErrors(Canvas *canvas, const char *function);


// define canvas private macros

#define reportError() \
	reportOpenGLErrors(canvas, __FUNCTION__)


// define canvas private functions

static void initOpenglState(Canvas *canvas)
{
	// clear the screen

	cgi_glClearDepth(1.0);
	cgi_glClearColor(0.0, 0.0, 0.0, 1.0);
	cgi_glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// set the shading model

	cgi_glShadeModel(GL_SMOOTH);

	// alpha testing

	cgi_glDisable(GL_ALPHA_TEST);
//	cgi_glEnable(GL_ALPHA_TEST);
//	glAlphaFunc(GL_GEQUAL, 0.1);

	// generate normal vectors

#if !defined(__ANDROID__)
	cgi_glEnable(GL_AUTO_NORMAL);
#endif // !__ANDROID__

	// blending

	cgi_glDisable(GL_BLEND);

	// color material

	cgi_glEnable(GL_COLOR_MATERIAL);
	cgi_glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
//	cgi_glMateriali(GL_FRONT_AND_BACK, GL_SHININESS, 0);
	cgi_glMateriali(GL_FRONT_AND_BACK, GL_SHININESS, 8);

	// face culling

	cgi_glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	// depth testing

	cgi_glEnable(GL_DEPTH_TEST);
//	cgi_glDepthFunc(GL_LESS);
	cgi_glDepthFunc(GL_LEQUAL);

	// color dithering

	cgi_glEnable(GL_DITHER);

	// fog

	cgi_glDisable(GL_FOG);

	// histogram

#if !defined(__ANDROID__)
//	cgi_glDisable(GL_HISTOGRAM);
	cgi_glEnable(GL_HISTOGRAM);
//	glHistogram(GL_HISTOGRAM, 2, GL_RGBA, (GLboolean)afalse);
#endif // !__ANDROID__

	// lighting

	cgi_glDisable(GL_LIGHTING);

	// line smoothing

	cgi_glEnable(GL_LINE_SMOOTH);

	// line width

	cgi_glLineWidth(1.0);

	// line stippling

#if !defined(__ANDROID__)
	cgi_glDisable(GL_LINE_STIPPLE);
#endif // !__ANDROID__

	// rgba min/max

/*
 * Note: the GL_MINMAX does not seem to work correctly on modern ATI cards.
 *
	cgi_glEnable(GL_MINMAX);
 */

	// multisampling

	cgi_glEnable(GL_MULTISAMPLE);

	// normalization

	cgi_glDisable(GL_NORMALIZE);

	// point smoothing

	cgi_glEnable(GL_POINT_SMOOTH);

	// polygon smoothing

#if !defined(__ANDROID__)
	cgi_glEnable(GL_POLYGON_SMOOTH);
#endif // !__ANDROID__

	// normal rescaling

	cgi_glEnable(GL_RESCALE_NORMAL);

	// sampling

	cgi_glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
	cgi_glDisable(GL_SAMPLE_ALPHA_TO_ONE);
	cgi_glEnable(GL_SAMPLE_COVERAGE);

	// scissoring

	cgi_glEnable(GL_SCISSOR_TEST);
	glScissor(0, 0, canvas->screenWidth, canvas->screenHeight);

	// stencil testing

	cgi_glDisable(GL_STENCIL_TEST);

	// texture coordinate direction

	cgi_glFrontFace(GL_CCW);

	// hints

	cgi_glHint(GL_GENERATE_MIPMAP_HINT, GL_NICEST);
	cgi_glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	cgi_glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	cgi_glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
#if !defined(__ANDROID__)
	cgi_glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
	cgi_glHint(GL_TEXTURE_COMPRESSION_HINT, GL_NICEST);
#endif // !__ANDROID__
}

static void setupOpenglScreen(Canvas *canvas)
{
	// setup vieport

	cgi_glViewport(0, 0, canvas->screenWidth, canvas->screenHeight);

	// perspective frustum

	cgi_glMatrixMode(GL_PROJECTION);
	cgi_glLoadIdentity();

	cgi_gluPerspective(
			CANVAS_Y_VIEW_ANGLE,						// y view angle
			(GLfloat)((double)canvas->screenWidth /		// aspact ratio
				(double)canvas->screenHeight),
			CANVAS_NEAR_Z_PLANE,						// near z plane
			CANVAS_FAR_Z_PLANE							// far z plane
			);

	// default rendering mode (3D)

	cgi_glMatrixMode(GL_MODELVIEW);
	cgi_glLoadIdentity();

	// init state

	initOpenglState(canvas);
}

static void reportOpenGLErrors(Canvas *canvas, const char *function)
{
	int rc = 0;
	char *errorString = NULL;

	if((rc = cgi_glGetError()) != GL_NO_ERROR) {
		switch(rc) {
			case GL_INVALID_ENUM:
				errorString = "invalid enumerator";
				break;

			case GL_INVALID_VALUE:
				errorString = "invalid numeric argument";
				break;

			case GL_INVALID_OPERATION:
				errorString = "specified operation not allowed";
				break;

			case GL_STACK_OVERFLOW:
				errorString = "stack overflow detected";
				break;

			case GL_STACK_UNDERFLOW:
				errorString = "stack underflow detected";
				break;

			case GL_OUT_OF_MEMORY:
				errorString = "out of memory";
				break;

#if !defined(__ANDROID__)
			case GL_TABLE_TOO_LARGE:
				errorString = "table exceeds maximum supported size";
				break;
#endif // !__ANDROID__

			default:
				errorString = "unknown";
		}
		log_logf(canvas->log, LOG_LEVEL_ERROR,
				"detected OpenGL error '%s' from %s() on canvas '%s'",
				errorString, (char *)function, canvas->canvasName);
	}
}


// define canvas public functions

int canvas_init(Canvas *canvas, char *name, aboolean isFullscreenMode,
		int screenWidth, int screenHeight, void *screenHandle, Log *log)
{
	if((canvas == NULL) || (screenHandle == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	// check arguments for sanity

	if(name == NULL) {
		name = "Asgard Game Engine";
	}
	if((screenWidth < 320) || (screenWidth > 4096)) {
		screenWidth = CANVAS_DEFAULT_SCREEN_WIDTH;
	}
	if((screenHeight < 240) || (screenHeight > 3072)) {
		screenHeight = CANVAS_DEFAULT_SCREEN_HEIGHT;
	}

	// initialize canvas

	memset(canvas, 0, sizeof(Canvas));

	canvas->isFullscreenMode = isFullscreenMode;
	canvas->isRenderBegan = afalse;
	canvas->status = CANVAS_STATUS_INIT;
	canvas->renderMode = CANVAS_RENDER_MODE_3D;
	canvas->screenWidth = screenWidth;
	canvas->screenHeight = screenHeight;
	canvas->frameCounter = 0;
	canvas->framerateSampleThreshold =
		CANVAS_DEFAULT_FRAMERATE_SAMPLE_THRESHOLD;
	canvas->renderTicks = 0;
	canvas->framerate = 0.0;
	canvas->timestamp = 0.0;
	canvas->canvasName = strdup(name);
	canvas->videoCardVendor = NULL;
	canvas->videoCardRenderer = NULL;
	canvas->videoCardVersion = NULL;
	canvas->videoCardExtensions = NULL;
	canvas->screenHandle = screenHandle;
	canvas->log = log;

	// initialize opengl

	log_logf(canvas->log, LOG_LEVEL_INFO,
			"initializing canvas '%s' at %ix%i pixels",
			canvas->canvasName,
			canvas->screenWidth,
			canvas->screenHeight);

	canvas->videoCardVendor = strdup((char *)cgi_glGetString(GL_VENDOR));
	if(canvas->videoCardVendor == NULL) {
		canvas->videoCardVendor = strdup("unknown");
	}

	log_logf(canvas->log, LOG_LEVEL_INFO, "video card vendor: %s",
			canvas->videoCardVendor);

	canvas->videoCardRenderer = strdup((char *)cgi_glGetString(GL_RENDERER));
	if(canvas->videoCardRenderer == NULL) {
		canvas->videoCardRenderer = strdup("unknown");
	}

	log_logf(canvas->log, LOG_LEVEL_INFO, "video card renderer: %s",
			canvas->videoCardRenderer);

	canvas->videoCardVersion = strdup((char *)cgi_glGetString(GL_VERSION));
	if(canvas->videoCardVersion == NULL) {
		canvas->videoCardVersion = strdup("unknown");
	}

	log_logf(canvas->log, LOG_LEVEL_INFO, "video card version: %s",
			canvas->videoCardVersion);

	canvas->videoCardExtensions = strdup(
			(char *)cgi_glGetString(GL_EXTENSIONS));
	if(canvas->videoCardExtensions == NULL) {
		canvas->videoCardExtensions = strdup("unknown");
	}

	log_logf(canvas->log, LOG_LEVEL_INFO, "video card extensions: %s",
			canvas->videoCardExtensions);

	setupOpenglScreen(canvas);

	// update canvas status

	log_logf(canvas->log, LOG_LEVEL_INFO, "canvas '%s' initialized",
			canvas->canvasName);

	canvas->status = CANVAS_STATUS_OK;

	return 0;
}

int canvas_free(Canvas *canvas)
{
	if(canvas == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(canvas->status != CANVAS_STATUS_OK) {
		log_logf(canvas->log, LOG_LEVEL_ERROR,
				"unable to %s() on canvas '%s' in state %i",
				__FUNCTION__, canvas->canvasName, canvas->status);
		return -1;
	}

	canvas->status = CANVAS_STATUS_FREED;

	if(canvas->videoCardVendor != NULL) {
		free(canvas->videoCardVendor);
	}

	if(canvas->videoCardRenderer != NULL) {
		free(canvas->videoCardRenderer);
	}

	if(canvas->videoCardVersion != NULL) {
		free(canvas->videoCardVersion);
	}

	if(canvas->videoCardExtensions != NULL) {
		free(canvas->videoCardExtensions);
	}

	log_logf(canvas->log, LOG_LEVEL_INFO, "canvas '%s' freed",
			canvas->canvasName);

	if(canvas->canvasName != NULL) {
		free(canvas->canvasName);
	}

	memset(canvas, 0, sizeof(Canvas));

	return 0;
}

int canvas_getFullscreenMode(Canvas *canvas, aboolean *isFullscreenMode)
{
	if((canvas == NULL) || (isFullscreenMode == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(canvas->status != CANVAS_STATUS_OK) {
		log_logf(canvas->log, LOG_LEVEL_ERROR,
				"unable to %s() on canvas '%s' in state %i",
				__FUNCTION__, canvas->canvasName, canvas->status);
		return -1;
	}

	*isFullscreenMode = canvas->isFullscreenMode;

	return 0;
}

int canvas_toggleFullscreen(Canvas *canvas)
{
	if(canvas == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(canvas->status != CANVAS_STATUS_OK) {
		log_logf(canvas->log, LOG_LEVEL_ERROR,
				"unable to %s() on canvas '%s' in state %i",
				__FUNCTION__, canvas->canvasName, canvas->status);
		return -1;
	}

	/*
	 * TODO: this function, probably requires plugin-function
	 */

	return -1; // not implemented
}

int canvas_toggleWindowed(Canvas *canvas)
{
	if(canvas == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(canvas->status != CANVAS_STATUS_OK) {
		log_logf(canvas->log, LOG_LEVEL_ERROR,
				"unable to %s() on canvas '%s' in state %i",
				__FUNCTION__, canvas->canvasName, canvas->status);
		return -1;
	}

	/*
	 * TODO: this function, probably requires plugin-function
	 */

	return -1; // not implemented
}

int canvas_getStatus(Canvas *canvas)
{
	if(canvas == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(canvas->status != CANVAS_STATUS_OK) {
		log_logf(canvas->log, LOG_LEVEL_ERROR,
				"unable to %s() on canvas '%s' in state %i",
				__FUNCTION__, canvas->canvasName, canvas->status);
		return -1;
	}

	return canvas->status;
}

int canvas_getRenderMode(Canvas *canvas)
{
	if(canvas == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(canvas->status != CANVAS_STATUS_OK) {
		log_logf(canvas->log, LOG_LEVEL_ERROR,
				"unable to %s() on canvas '%s' in state %i",
				__FUNCTION__, canvas->canvasName, canvas->status);
		return -1;
	}

	return canvas->renderMode;
}

int canvas_toggleRenderMode2d(Canvas *canvas)
{
	if(canvas == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(canvas->status != CANVAS_STATUS_OK) {
		log_logf(canvas->log, LOG_LEVEL_ERROR,
				"unable to %s() on canvas '%s' in state %i",
				__FUNCTION__, canvas->canvasName, canvas->status);
		return -1;
	}

	if(canvas->renderMode == CANVAS_RENDER_MODE_2D) {
		return 0;
	}

	// flat perspective

	cgi_glMatrixMode(GL_PROJECTION);
	cgi_glLoadIdentity();

	cgi_glOrtho(0,
			canvas->screenWidth,
			canvas->screenHeight,
			0,
			-1,
			1
			);

	// rendering mode

	cgi_glMatrixMode(GL_MODELVIEW);
	cgi_glLoadIdentity();

	// setup vieport

	cgi_glViewport(0, 0, canvas->screenWidth, canvas->screenHeight);

	// turn off lighting

#if !defined(__ANDROID__)
	if(cgi_glIsEnabled(GL_LIGHTING)) {
		cgi_glDisable(GL_LIGHTING);
	}
#else // __ANDROID__
	cgi_glDisable(GL_LIGHTING);
#endif // !__ANDROID__

	canvas->renderMode = CANVAS_RENDER_MODE_2D;

	return 0;
}

int canvas_toggleRenderMode3d(Canvas *canvas)
{
	if(canvas == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(canvas->status != CANVAS_STATUS_OK) {
		log_logf(canvas->log, LOG_LEVEL_ERROR,
				"unable to %s() on canvas '%s' in state %i",
				__FUNCTION__, canvas->canvasName, canvas->status);
		return -1;
	}

	if(canvas->renderMode == CANVAS_RENDER_MODE_3D) {
		return 0;
	}

	// perspective frustum

	cgi_glMatrixMode(GL_PROJECTION);
	cgi_glLoadIdentity();

	cgi_gluPerspective(
			CANVAS_Y_VIEW_ANGLE,						// y view angle
			(GLfloat)((double)canvas->screenWidth /		// aspact ratio
				(double)canvas->screenHeight),
			CANVAS_NEAR_Z_PLANE,						// near z plane
			CANVAS_FAR_Z_PLANE							// far z plane
			);

	// rendering mode

	cgi_glMatrixMode(GL_MODELVIEW);
	cgi_glLoadIdentity();

	canvas->renderMode = CANVAS_RENDER_MODE_3D;

	return 0;
}

int canvas_getScreenDimensions(Canvas *canvas, int *width, int *height)
{
	if((canvas == NULL) || (width == NULL) || (height == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(canvas->status != CANVAS_STATUS_OK) {
		log_logf(canvas->log, LOG_LEVEL_ERROR,
				"unable to %s() on canvas '%s' in state %i",
				__FUNCTION__, canvas->canvasName, canvas->status);
		return -1;
	}

	*width = canvas->screenWidth;
	*height = canvas->screenHeight;

	return 0;
}

int canvas_setScreenDimensions(Canvas *canvas, int width, int height)
{
	if(canvas == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(canvas->status != CANVAS_STATUS_OK) {
		log_logf(canvas->log, LOG_LEVEL_ERROR,
				"unable to %s() on canvas '%s' in state %i",
				__FUNCTION__, canvas->canvasName, canvas->status);
		return -1;
	}

	if((width < 320) || (width > 4096)) {
		width = CANVAS_DEFAULT_SCREEN_WIDTH;
	}
	if((height < 240) || (height > 3072)) {
		height = CANVAS_DEFAULT_SCREEN_HEIGHT;
	}

	canvas->screenWidth = width;
	canvas->screenHeight = height;

	setupOpenglScreen(canvas);

	return 0;
}

int canvas_setFramerateSampleThreshold(Canvas *canvas, int threshold)
{
	if(canvas == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(canvas->status != CANVAS_STATUS_OK) {
		log_logf(canvas->log, LOG_LEVEL_ERROR,
				"unable to %s() on canvas '%s' in state %i",
				__FUNCTION__, canvas->canvasName, canvas->status);
		return -1;
	}

	canvas->framerateSampleThreshold = threshold;

	return 0;
}

double canvas_getFramerate(Canvas *canvas)
{
	if(canvas == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(canvas->status != CANVAS_STATUS_OK) {
		log_logf(canvas->log, LOG_LEVEL_ERROR,
				"unable to %s() on canvas '%s' in state %i",
				__FUNCTION__, canvas->canvasName, canvas->status);
		return -1;
	}

	return canvas->framerate;
}

alint canvas_getRenderTicks(Canvas *canvas)
{
	if(canvas == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(canvas->status != CANVAS_STATUS_OK) {
		log_logf(canvas->log, LOG_LEVEL_ERROR,
				"unable to %s() on canvas '%s' in state %i",
				__FUNCTION__, canvas->canvasName, canvas->status);
		return -1;
	}

	return canvas->renderTicks;
}

int canvas_getCanvasName(Canvas *canvas, char **name)
{
	if((canvas == NULL) || (name == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(canvas->status != CANVAS_STATUS_OK) {
		log_logf(canvas->log, LOG_LEVEL_ERROR,
				"unable to %s() on canvas '%s' in state %i",
				__FUNCTION__, canvas->canvasName, canvas->status);
		return -1;
	}

	*name = canvas->canvasName;

	return 0;
}

int canvas_getVideoCardVendor(Canvas *canvas, char **vendor)
{
	if((canvas == NULL) || (vendor == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(canvas->status != CANVAS_STATUS_OK) {
		log_logf(canvas->log, LOG_LEVEL_ERROR,
				"unable to %s() on canvas '%s' in state %i",
				__FUNCTION__, canvas->canvasName, canvas->status);
		return -1;
	}

	*vendor = canvas->videoCardVendor;

	return 0;
}

int canvas_getVideoCardRenderer(Canvas *canvas, char **renderer)
{
	if((canvas == NULL) || (renderer == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(canvas->status != CANVAS_STATUS_OK) {
		log_logf(canvas->log, LOG_LEVEL_ERROR,
				"unable to %s() on canvas '%s' in state %i",
				__FUNCTION__, canvas->canvasName, canvas->status);
		return -1;
	}

	*renderer = canvas->videoCardRenderer;

	return 0;
}

int canvas_getVideoCardVersion(Canvas *canvas, char **version)
{
	if((canvas == NULL) || (version == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(canvas->status != CANVAS_STATUS_OK) {
		log_logf(canvas->log, LOG_LEVEL_ERROR,
				"unable to %s() on canvas '%s' in state %i",
				__FUNCTION__, canvas->canvasName, canvas->status);
		return -1;
	}

	*version = canvas->videoCardVersion;

	return 0;
}

int canvas_getVideoCardExtensions(Canvas *canvas, char **extensions)
{
	if((canvas == NULL) || (extensions == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(canvas->status != CANVAS_STATUS_OK) {
		log_logf(canvas->log, LOG_LEVEL_ERROR,
				"unable to %s() on canvas '%s' in state %i",
				__FUNCTION__, canvas->canvasName, canvas->status);
		return -1;
	}

	*extensions = canvas->videoCardExtensions;

	return 0;
}

int canvas_getScreenHandle(Canvas *canvas, void **handle)
{
	if((canvas == NULL) || (handle == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(canvas->status != CANVAS_STATUS_OK) {
		log_logf(canvas->log, LOG_LEVEL_ERROR,
				"unable to %s() on canvas '%s' in state %i",
				__FUNCTION__, canvas->canvasName, canvas->status);
		return -1;
	}

	*handle = canvas->screenHandle;

	return 0;
}

int canvas_beginRender(Canvas *canvas)
{
	if(canvas == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(canvas->status != CANVAS_STATUS_OK) {
		log_logf(canvas->log, LOG_LEVEL_ERROR,
				"unable to %s() on canvas '%s' in state %i",
				__FUNCTION__, canvas->canvasName, canvas->status);
		return -1;
	}

	if(canvas->isRenderBegan) {
		log_logf(canvas->log, LOG_LEVEL_ERROR,
				"unable to begin render on canvas '%s', already in progress",
				canvas->canvasName);
		return -1;
	}

	if(canvas->timestamp == 0.0) {
		canvas->timestamp = time_getTimeMus();
	}

	initOpenglState(canvas);

	canvas->isRenderBegan = atrue;

	return 0;
}

int canvas_endRender(Canvas *canvas)
{
	double delta = 0.0;

	if(canvas == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(canvas->status != CANVAS_STATUS_OK) {
		log_logf(canvas->log, LOG_LEVEL_ERROR,
				"unable to %s() on canvas '%s' in state %i",
				__FUNCTION__, canvas->canvasName, canvas->status);
		return -1;
	}

	if(!canvas->isRenderBegan) {
		log_logf(canvas->log, LOG_LEVEL_ERROR,
				"unable to ender render on canvas '%s', not started",
				canvas->canvasName);
		return -1;
	}

	// swap buffers

	if(osapi_swapScreenBuffer(canvas->screenHandle) < 0) {
		log_logf(canvas->log, LOG_LEVEL_ERROR,
				"failed to swap screen buffer on canvas '%s'",
				canvas->canvasName);
		return -1;
	}

	// report OpenGL errors

	reportError();

	// calculate framerate

	canvas->frameCounter += 1;
	if(canvas->frameCounter >= canvas->framerateSampleThreshold) {
		delta = time_getElapsedMusInSeconds(canvas->timestamp);
		canvas->timestamp = time_getTimeMus();
		canvas->framerate = ((double)canvas->framerateSampleThreshold / delta);
		canvas->frameCounter = 0;

		log_logf(canvas->log, LOG_LEVEL_DEBUG,
				"canvas '%s' framerate: %0.2f fps",
				canvas->canvasName, canvas->framerate);
	}

	canvas->renderTicks += 1;

	// end render

	canvas->isRenderBegan = afalse;

	return 0;
}

void canvas_reportOpenGLErrors(Canvas *canvas, const char *function)
{
	if((canvas == NULL) || (function == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	reportOpenGLErrors(canvas, function);
}

