/*
 * androidapi.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Android application programming interface to work with the OpenGL
 * library.
 *
 * Written by Josh English.
 */

#if defined(__ANDROID__)

// preprocessor directives

#include "core/core.h"
#define _ASGARD_CORE_COMPONENT
#include "asgard/core/common.h"
#include "asgard/core/colors.h"
#include "asgard/core/events.h"
#include "asgard/core/osapi.h"
#define _ASGARD_ENGINE_COMPONENT
#include "asgard/engine/texture.h"
#define _ASGARD_ENGINE_PLUGIN_COMPONENT
#include "asgard/engine/plugin/targa.h"
#define _ASGARD_CORE_PLUGIN_COMPONENT
#include "asgard/core/plugin/androidapi.h"
#include "zip.h"


// declare androidapi private functions

static aboolean enableOpenGL(aboolean isMultiThreaded,
		AndroidApiWindow *window);

static void disableOpenGL(AndroidApiWindow *window);


// define androidapi private functions

static aboolean enableOpenGL(aboolean isMultiThreaded,
		AndroidApiWindow *window)
{
	return atrue;
}

static void disableOpenGL(AndroidApiWindow *window)
{
	window->isOpenGLEnabled = afalse;
	window->isSecondOpenGLEnabled = afalse;
}

// define androidapi public functions

int androidapi_createWindow(AndroidApiWindow *window, char *name,
		aboolean isFullscreenMode, int screenWidth, int screenHeight,
		int colorBits, Log *log)
{
	if((window == NULL) || (screenWidth < 8) || (screenHeight < 8)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(name == NULL) {
		name = "Project Asgard Game Engine";
	}

	log_logf(log, LOG_LEVEL_INFO, "using android system");

	memset((void *)window, 0, sizeof(AndroidApiWindow));

	window->wasCreated = afalse;
	window->isOpenGLEnabled = afalse;
	window->isSecondOpenGLEnabled = afalse;
	window->exitApplication = afalse;
	window->isFullscreenMode = isFullscreenMode;
	window->xPosition = 0;
	window->yPosition = 0;
	window->windowWidth = screenWidth;
	window->windowHeight = screenHeight;
	window->bitsPerPixel = colorBits;
	window->log = log;

	if(fifostack_init(&window->eventQueue) < 0) {
		log_logf(window->log, LOG_LEVEL_ERROR,
				"failed to initialize event queue");
		return -1;
	}

	if(fifostack_setFreeFunction(&window->eventQueue,
				(FifoStackFreeFunction)events_freeEvent) < 0) {
		log_logf(window->log, LOG_LEVEL_ERROR,
				"failed to set event queue free function");
		return -1;
	}

	if(mutex_init(&window->mutex) < 0) {
		log_logf(window->log, LOG_LEVEL_ERROR,
				"failed to initialize mutex");
		return -1;
	}

	if(!enableOpenGL(afalse, window)) {
		log_logf(window->log, LOG_LEVEL_ERROR,
				"failed to enable OpenGL for window '%s'", name);
		return -1;
	}

	log_logf(window->log, LOG_LEVEL_INFO,
			"created window '%s' from (%i, %i) at %ix%i with %i-bit depth",
			name, window->xPosition, window->yPosition, window->windowWidth,
			window->windowHeight, window->bitsPerPixel);

	window->wasCreated = atrue;

	return 0;
}

void androidapi_destroyWindow(AndroidApiWindow *window)
{
	if(window == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	if(!window->wasCreated) {
		log_logf(window->log, LOG_LEVEL_ERROR,
				"unable to execute %s(), window not created",
				__FUNCTION__);
		return;
	}

	if(window->isOpenGLEnabled) {
		disableOpenGL(window);
	}

	fifostack_free(&window->eventQueue);

	log_logf(window->log, LOG_LEVEL_INFO, "destroyed window");
}

void *androidapi_enableOpenGL(aboolean isMultiThreaded, int colorBits,
		AndroidApiWindow *window)
{
	if(((colorBits != 8) &&
				(colorBits != 16) &&
				(colorBits != 24) &&
				(colorBits != 32)) ||
			(window == NULL)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	mutex_lock(&window->mutex);

	if(!window->wasCreated) {
		log_logf(window->log, LOG_LEVEL_ERROR,
				"unable to execute %s(), window not created",
				__FUNCTION__);
		mutex_unlock(&window->mutex);
		return NULL;
	}

	if(window->isOpenGLEnabled) {
		disableOpenGL(window);
	}

	if(!enableOpenGL(isMultiThreaded, window)) {
		log_logf(window->log, LOG_LEVEL_ERROR, "failed to enable OpenGL");
		mutex_unlock(&window->mutex);
		return NULL;
	}

	window->isOpenGLEnabled = atrue;

	mutex_unlock(&window->mutex);

	return (void *)window;
}

void androidapi_makeOpenGLCurrent(AndroidApiWindow *window, int threadType)
{
	if((window == NULL) ||
			((threadType != OSAPI_THREAD_TYPE_RENDER) &&
			 (threadType != OSAPI_THREAD_TYPE_GAMEPLAY))) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	return;
}

void androidapi_disableOpenGL(AndroidApiWindow *window)
{
	if(window == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	mutex_lock(&window->mutex);

	if(!window->wasCreated) {
		log_logf(window->log, LOG_LEVEL_ERROR,
				"unable to execute %s(), window not created",
				__FUNCTION__);
		mutex_unlock(&window->mutex);
		return;
	}

	if(!window->isOpenGLEnabled) {
		log_logf(window->log, LOG_LEVEL_ERROR,
				"unable to execute %s(), OpenGL not initialized",
				__FUNCTION__);
		mutex_unlock(&window->mutex);
		return;
	}

	disableOpenGL(window);

	mutex_unlock(&window->mutex);
}

int androidapi_checkState(AndroidApiWindow *window, aboolean *exitApplication)
{
	if((window == NULL) || (exitApplication == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&window->mutex);

	if(!window->wasCreated) {
		log_logf(window->log, LOG_LEVEL_ERROR,
				"unable to execute %s(), window not created",
				__FUNCTION__);
		mutex_unlock(&window->mutex);
		return -1;
	}

	*exitApplication = window->exitApplication;

	mutex_unlock(&window->mutex);

	return 0;
}

void androidapi_swapScreenBuffer(AndroidApiWindow *window)
{
	if(window == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	mutex_lock(&window->mutex);

	if(!window->wasCreated) {
		log_logf(window->log, LOG_LEVEL_ERROR,
				"unable to execute %s(), window not created",
				__FUNCTION__);
		mutex_unlock(&window->mutex);
		return;
	}

	if(!window->isOpenGLEnabled) {
		log_logf(window->log, LOG_LEVEL_ERROR,
				"unable to execute %s(), OpenGL not initialized",
				__FUNCTION__);
		mutex_unlock(&window->mutex);
		return;
	}

	mutex_unlock(&window->mutex);
}

void androidapi_setMousePosition(AndroidApiWindow *window,
		int mouseX, int mouseY)
{
	if(window == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	mutex_lock(&window->mutex);

	if(!window->wasCreated) {
		log_logf(window->log, LOG_LEVEL_ERROR,
				"unable to execute %s(), window not created",
				__FUNCTION__);
		mutex_unlock(&window->mutex);
		return;
	}

	mutex_unlock(&window->mutex);
}

void androidapi_enableMouseDisplay(AndroidApiWindow *window)
{
	if(window == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	mutex_lock(&window->mutex);

	if(!window->wasCreated) {
		log_logf(window->log, LOG_LEVEL_ERROR,
				"unable to execute %s(), window not created",
				__FUNCTION__);
		mutex_unlock(&window->mutex);
		return;
	}

	mutex_unlock(&window->mutex);
}

void androidapi_disableMouseDisplay(AndroidApiWindow *window)
{
	if(window == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	mutex_lock(&window->mutex);

	if(!window->wasCreated) {
		log_logf(window->log, LOG_LEVEL_ERROR,
				"unable to execute %s(), window not created",
				__FUNCTION__);
		mutex_unlock(&window->mutex);
		return;
	}

	mutex_unlock(&window->mutex);
}

AsgardEvent *androidapi_popEvent(AndroidApiWindow *window)
{
	void *result = NULL;

	if(window == NULL) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	mutex_lock(&window->mutex);

	if(!window->wasCreated) {
		log_logf(window->log, LOG_LEVEL_ERROR,
				"unable to execute %s(), window not created",
				__FUNCTION__);
		mutex_unlock(&window->mutex);
		return NULL;
	}

	if(fifostack_pop(&window->eventQueue, &result) < 0) {
		result = NULL;
	}

	mutex_unlock(&window->mutex);

	return (AsgardEvent *)result;
}

aboolean androidapi_pushEvent(AndroidApiWindow *window, AsgardEvent *event)
{
	aboolean result = afalse;

	if(window == NULL) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	mutex_lock(&window->mutex);

	if(!window->wasCreated) {
		log_logf(window->log, LOG_LEVEL_ERROR,
				"unable to execute %s(), window not created",
				__FUNCTION__);
		mutex_unlock(&window->mutex);
		return afalse;
	}

	if(fifostack_push(&window->eventQueue, event) < 0) {
		result = afalse;
	}
	else {
		result = atrue;
	}

	mutex_unlock(&window->mutex);

	return result;
}

// helper functions

void *androidapi_zipUtil_openApkArchive(char *filename)
{
	int ii = 0;
	int fileCount = 0;
	const char *archiveFilename = NULL;

	struct zip *result = NULL;

	if(filename == NULL) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	if((result = zip_open(filename, 0, NULL)) == NULL) {
		fprintf(stderr, "[%s():%i] error - failed to open zip "
				"archive '%s'.\n", __FUNCTION__, __LINE__, filename);
		return NULL;
	}

	if((fileCount = zip_get_num_files(result)) > 0) {
		for(ii = 0; ii < fileCount; ii++) {
			if((archiveFilename = zip_get_name(result, ii, 0)) == NULL) {
				fprintf(stderr, "[%s():%i] error - failed to read from zip "
						"archive '%s' at index #%i.\n",
						__FUNCTION__, __LINE__, filename, ii);
				return NULL;
			}
			else {
				printf("%s() :: zip archive file '%s'\n", __FUNCTION__,
						archiveFilename);
			}
		}
	}

	return result;
}

char *androidapi_zipUtil_readFileToBuffer(void *context, char *filename,
		int *bufferLength)
{
	int rc = 0;
	int ref = 0;
	int length = 0;
	char chunk[8];
	char *result = NULL;

	struct zip_file *fd = NULL;

	if((context == NULL) || (filename == NULL) || (bufferLength == NULL)) {
		fprintf(stderr, "[%s():%i] error - invalid or missing arguments",
				__FUNCTION__, __LINE__);
		if(context == NULL) {
			fprintf(stderr, ", context is NULL");
		}
		if(filename == NULL) {
			fprintf(stderr, ", filename is NULL");
		}
		if(bufferLength == NULL) {
			fprintf(stderr, ", bufferLength is NULL");
		}
		fprintf(stderr, ".\n");
		return NULL;
	}

	*bufferLength = 0;

	if((fd = zip_fopen(context, filename, 0)) == NULL) {
		fprintf(stderr, "[%s():%i] error - failed to open zip archive "
				"file '%s'.\n", __FUNCTION__, __LINE__, filename);
		return NULL;
	}

	ref = 0;
	length = 1024;
	result = (char *)malloc(sizeof(char) * (length + 1));

	while((rc = zip_fread(fd, chunk, 1)) == 1) {
		result[ref] = chunk[0];
		ref++;
		if(ref >= length) {
			length *= 2;
			result = (char *)realloc(result,
					(sizeof(char) * (length + 1)));
		}
	}

	zip_fclose(fd);

	if(ref <= 0) {
		free(result);
		return NULL;
	}

	*bufferLength = ref;

	return result;
}

int androidapi_textureUtil_loadTexture(void *context, Log *log,
		void *texture, char *filename)
{
	int bufferLength = 0;
	char *buffer = NULL;

	Targa targa;
	Texture *lTexture = NULL;

	if((context == NULL) || (texture == NULL) || (filename == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if((buffer = androidapi_zipUtil_readFileToBuffer(context,
					filename,
					&bufferLength)) == NULL) {
		log_logf(log, LOG_LEVEL_ERROR,
				"failed to read from texture '%s'", filename);
		return -1;
	}

	log_logf(log, LOG_LEVEL_DEBUG,
			"read %i bytes from texture '%s' into RGBA buffer",
			bufferLength, filename);

	targa_init(&targa);

	if(targa_loadFromData(&targa, (unsigned char *)buffer, bufferLength) < 0) {
		log_logf(log, LOG_LEVEL_ERROR,
				"failed to decode TGA from texture '%s'", filename);
		targa_free(&targa);
		return -1;
	}

	lTexture = texture;

	if(texture_initFromRgbaBuffer(lTexture,
				TEXTURE_QUALITY_MEDIUM,
				TEXTURE_RENDER_REPEAT,
				afalse,
				(char *)targa.image,
				targa.imageLength,
				targa.width,
				targa.height) < 0) {
		log_logf(log, LOG_LEVEL_ERROR,
				"failed to initialize TGA texture from '%s' "
				"decoded RGBA buffer", filename);
		targa_free(&targa);
		return -1;
	}

	log_logf(log, LOG_LEVEL_INFO,
			"loaded %ix%i TGA texture '%s' from RGBA buffer as #%i",
			targa.width, targa.height, filename, (int)lTexture->id);

	targa_free(&targa);

	return 0;
}

#endif // __ANDROID__

