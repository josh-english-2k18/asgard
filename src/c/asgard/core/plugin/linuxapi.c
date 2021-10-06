/*
 * linuxapi.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Linux application programming interface to work with the OpenGL
 * library.
 *
 * Written by Josh English.
 */

#if defined(__linux__) && !defined(__ANDROID__)

// preprocessor directives

#include "core/core.h"
#define _ASGARD_CORE_COMPONENT
#include "asgard/core/common.h"
#include "asgard/core/events.h"
#include "asgard/core/osapi.h"
#define _ASGARD_CORE_PLUGIN_COMPONENT
#include "asgard/core/plugin/linuxapi.h"


// define linuxapi private constants

#define LINUXAPI_VERBOSE_MESSAGING						atrue


// declare linuxapi private functions

static aboolean enableOpenGL(aboolean isMultiThreaded, LinuxApiWindow *window);

static void disableOpenGL(LinuxApiWindow *window);

static void pushEvent(LinuxApiWindow *window, AsgardEvent *event);

static void handleLinuxEvent(LinuxApiWindow *window, XEvent *xEvent);

static void *monitorWindowState(void *threadContext, void *argument);


// define linuxapi private functions

static aboolean enableOpenGL(aboolean isMultiThreaded, LinuxApiWindow *window)
{
	unsigned int apiBorder = 0;

	Window apiWindow;

	memset((void *)&apiWindow, 0, (sizeof(Window)));

	if(XGetGeometry(
				window->display,
				window->handle,
				&apiWindow,
				&window->xPosition,
				&window->yPosition,
				&window->windowWidth,
				&window->windowHeight,
				&apiBorder,
				&window->bitsPerPixel) == BadDrawable) {
		return afalse;
	}

	if((window->contextOne = glXCreateContext(
					window->display,
					window->visualInfo,
					0,
					GL_TRUE))== NULL) {
		return afalse;
	}

	if(!isMultiThreaded) {
		glXMakeCurrent(window->display, window->handle, window->contextOne);
	}
	else {
		window->contextTwo = glXCreateContext(
				window->display,
				window->visualInfo,
				window->contextOne,
				GL_TRUE);
		if(window->contextTwo != NULL) {
			window->isSecondOpenGLEnabled = atrue;
		}
		else {
			log_logf(window->log, LOG_LEVEL_ERROR,
					"failed to enable second OpenGL context");
			return afalse;
		}
	}

	return atrue;
}

static void disableOpenGL(LinuxApiWindow *window)
{
	if(!glXMakeCurrent(window->display, None, NULL)) {
		log_logf(window->log, LOG_LEVEL_ERROR,
				"failed to release drawing context");
	}

	if(window->contextOne != NULL) {
		glXDestroyContext(window->display, window->contextOne);
		window->contextOne = NULL;
	}

	if(window->contextTwo != NULL) {
		glXDestroyContext(window->display, window->contextTwo);
		window->contextTwo = NULL;
	}

	window->isOpenGLEnabled = afalse;
	window->isSecondOpenGLEnabled = afalse;
}

static void pushEvent(LinuxApiWindow *window, AsgardEvent *event)
{
	if(window == NULL) {
		return;
	}

	mutex_lock(&window->mutex);

	fifostack_push(&window->eventQueue, event);

	mutex_unlock(&window->mutex);
}

static void handleLinuxEvent(LinuxApiWindow *window, XEvent *xEvent)
{
	char key = (char)0;
	int keyCode = 0;

	AsgardEvent *event = NULL;

	switch(xEvent->type) {
		case Expose:
			if(xEvent->xexpose.count != 0) {
				mutex_lock(&window->mutex);
				window->exitApplication = atrue;
				mutex_unlock(&window->mutex);
			}
			break;

		case ConfigureNotify:
			if((xEvent->xconfigure.width != window->windowWidth) ||
					(xEvent->xconfigure.height != window->windowHeight)) {
				mutex_lock(&window->mutex);
				window->windowWidth = xEvent->xconfigure.width;
				window->windowHeight = xEvent->xconfigure.height;
				mutex_unlock(&window->mutex);

				event = events_newEvent(EVENT_TYPE_OS);
				((AsgardOsEvent *)event->event)->eventType =
					EVENT_OS_SCREEN_RESIZED;
				((AsgardOsEvent *)event->event)->screenWidth =
					window->windowWidth;
				((AsgardOsEvent *)event->event)->screenHeight =
					window->windowHeight;

				pushEvent(window, event);
			}
			break;

		/*
		 * keyboard events
		 */

		case KeyPress:
			keyCode = (int)XLookupKeysym(&xEvent->xkey, 0);

			if(xEvent->xkey.state & ShiftMask) {
				key = ctype_toUpper((char)keyCode);
			}
			else {
				key = (char)keyCode;
			}

			log_logf(window->log, LOG_LEVEL_DEBUG,
					"window-message: key #%i (%c) X-code %i down",
					keyCode, (char)keyCode, xEvent->xkey.keycode);

			event = events_newEvent(EVENT_TYPE_KEYBOARD);
			((AsgardKeyboardEvent *)event->event)->key = key;
			((AsgardKeyboardEvent *)event->event)->keyCode = keyCode;
			((AsgardKeyboardEvent *)event->event)->eventType =
				EVENT_KEYBOARD_TYPED;

			pushEvent(window, event);

			log_logf(window->log, LOG_LEVEL_DEBUG,
					"window-message: key #%i (%c) X-code %i pressed",
					keyCode, (char)keyCode, xEvent->xkey.keycode);

			event = events_newEvent(EVENT_TYPE_KEYBOARD);
			((AsgardKeyboardEvent *)event->event)->key = key;
			((AsgardKeyboardEvent *)event->event)->keyCode = keyCode;
			((AsgardKeyboardEvent *)event->event)->eventType =
				EVENT_KEYBOARD_PRESSED;

			pushEvent(window, event);
			break;

		case KeyRelease:
			keyCode = (int)XLookupKeysym(&xEvent->xkey, 0);

			if(xEvent->xkey.state & ShiftMask) {
				key = ctype_toUpper((char)keyCode);
			}
			else {
				key = (char)keyCode;
			}

			log_logf(window->log, LOG_LEVEL_DEBUG,
					"window-message: key #%i (%c) X-code %i up",
					keyCode, (char)keyCode, xEvent->xkey.keycode);

			event = events_newEvent(EVENT_TYPE_KEYBOARD);
			((AsgardKeyboardEvent *)event->event)->key = key;
			((AsgardKeyboardEvent *)event->event)->keyCode = keyCode;
			((AsgardKeyboardEvent *)event->event)->eventType =
				EVENT_KEYBOARD_RELEASED;

			pushEvent(window, event);
			break;

		/*
		 * mouse events
		 */

		case MotionNotify: // mouse moved
			log_logf(window->log, LOG_LEVEL_DEBUG,
					"window-message: mouse moved (%i, %i)",
					xEvent->xmotion.x, xEvent->xmotion.y);

			event = events_newEvent(EVENT_TYPE_MOUSE);
			((AsgardMouseEvent *)event->event)->eventType = EVENT_MOUSE_MOVED;
			((AsgardMouseEvent *)event->event)->xPosition = xEvent->xmotion.x;
			((AsgardMouseEvent *)event->event)->yPosition = xEvent->xmotion.y;

			pushEvent(window, event);
			break;

		case ClientMessage:
			if(*XGetAtomName(window->display,
						xEvent->xclient.message_type) == *"WM_PROTOCOLS") {
				mutex_lock(&window->mutex);
				window->exitApplication = atrue;
				mutex_unlock(&window->mutex);
			}
			break;

		default:
			if(LINUXAPI_VERBOSE_MESSAGING) {
				log_logf(window->log, LOG_LEVEL_DEBUG,
						"window-message: unhandled message #%i",
						(int)xEvent->type);
			}
			break;
	}
}

static void *monitorWindowState(void *threadContext, void *argument)
{
	int counter = 0;
	alint totalCounter = 0;
	double timestamp = 0.0;

	TmfThread *thread = NULL;
	LinuxApiWindow *window = NULL;

	XEvent xEvent;

	thread = (TmfThread *)threadContext;
	window = (LinuxApiWindow *)argument;

	log_logf(window->log, LOG_LEVEL_INFO,
			"window-state monitoring thread #%i (%i) running",
			thread->uid, thread->systemId);

	while((thread->state == THREAD_STATE_RUNNING) &&
			(thread->signal == THREAD_SIGNAL_OK)) {
		timestamp = time_getTimeMus();

		mutex_lock(thread->mutex);
		thread->status = THREAD_STATUS_BUSY;
		mutex_unlock(thread->mutex);

		counter = 0;
		while(XPending(window->display) > 0) {
			XNextEvent(window->display, &xEvent);
			handleLinuxEvent(window, &xEvent);
			counter++;
			totalCounter++;
		}

		if(counter > 0) {
			log_logf(window->log, LOG_LEVEL_DEBUG,
					"monitoring thread processed %i event(s) in %0.6f seconds",
					counter, time_getElapsedMusInSeconds(timestamp));
		}

		if((thread->state != THREAD_STATE_RUNNING) ||
				(thread->signal != THREAD_SIGNAL_OK)) {
			break;
		}

		mutex_lock(thread->mutex);
		thread->status = THREAD_STATUS_NAPPING;
		mutex_unlock(thread->mutex);

		counter = 0;
		while((time_getElapsedMusInSeconds(timestamp) < 0.01) &&
				(thread->state == THREAD_STATE_RUNNING) &&
				(thread->signal == THREAD_SIGNAL_OK) &&
				(counter < 1024)) {
			time_usleep(1024);
			counter++;
		}
	}

	log_logf(window->log, LOG_LEVEL_INFO,
			"window-state monitoring thread #%i (%i) halted (%lli events)",
			thread->uid, thread->systemId, totalCounter);

	return NULL;
}

// define linuxapi public functions

int linuxapi_createWindow(LinuxApiWindow *window, char *name,
		aboolean isFullscreenMode, int screenWidth, int screenHeight,
		int colorBits, Log *log)
{
	int ii = 0;
	int bestMode = 0;
	int modeCount = 0;
	int *screenAttributes = NULL;

	Atom deleteAtom;
	XF86VidModeModeInfo **modes = NULL;

	if((window == NULL) || (screenWidth < 320) || (screenHeight < 240) ||
			((colorBits != 8) &&
			 (colorBits != 16) &&
			 (colorBits != 24) &&
			 (colorBits != 32))) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(name == NULL) {
		name = "Project Asgard Game Engine";
	}

	if(colorBits == 32) {
		colorBits = 24; // highest bit-support on linux
	}

	log_logf(log, LOG_LEVEL_INFO,
			"using linux system, selected %i color bits", colorBits);

	memset((void *)window, 0, sizeof(LinuxApiWindow));

	window->wasCreated = afalse;
	window->isOpenGLEnabled = afalse;
	window->isSecondOpenGLEnabled = afalse;
	window->exitApplication = afalse;
	window->isFullscreenMode = isFullscreenMode;
	window->screenId = 0;
	window->threadId = 0;
	window->xPosition = 0;
	window->yPosition = 0;
	window->windowWidth = 0;
	window->windowHeight = 0;
	window->bitsPerPixel = 0;
	window->display = NULL;
	window->visualInfo = NULL;
	window->log = log;

	memset((void *)&window->handle, 0, (sizeof(Window)));
	memset((void *)&window->contextOne, 0, (sizeof(GLXContext)));
	memset((void *)&window->contextTwo, 0, (sizeof(GLXContext)));
	memset((void *)&window->videoMode, 0, (sizeof(XF86VidModeModeInfo)));
	memset((void *)&window->attributes, 0, (sizeof(XSetWindowAttributes)));

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

	if(tmf_init(&window->tmf) < 0) {
		log_logf(window->log, LOG_LEVEL_ERROR,
				"failed to initialize thread management framework");
		return -1;
	}

	if(mutex_init(&window->mutex) < 0) {
		log_logf(window->log, LOG_LEVEL_ERROR,
				"failed to initialize mutex");
		return -1;
	}

	window->display = XOpenDisplay(NULL);
	if(window->display == NULL) {
		log_logf(window->log, LOG_LEVEL_PANIC, "failed to open X display");
		return -1;
	}

	window->screenId = DefaultScreen(window->display);

	log_logf(window->log, LOG_LEVEL_INFO, "using X screen #%i",
			window->screenId);

	if(!XF86VidModeGetAllModeLines(window->display, window->screenId,
				&modeCount, &modes)) {
		log_logf(window->log, LOG_LEVEL_PANIC,
				"failed to obtain available video modes");
		return -1;
	}

	screenAttributes = (int *)malloc(sizeof(int) * 11);
	screenAttributes[0] = GLX_RGBA;
	screenAttributes[1] = GLX_DOUBLEBUFFER;
	screenAttributes[2] = GLX_RED_SIZE;
	screenAttributes[3] = 4;
	screenAttributes[4] = GLX_GREEN_SIZE;
	screenAttributes[5] = 4;
	screenAttributes[6] = GLX_BLUE_SIZE;
	screenAttributes[7] = 4;
	screenAttributes[8] = GLX_DEPTH_SIZE;
	screenAttributes[9] = colorBits;
	screenAttributes[10] = None;

	window->visualInfo = glXChooseVisual(window->display, window->screenId,
			(int *)screenAttributes);
	if(window->visualInfo == NULL) {
		screenAttributes[8] = None;
		window->visualInfo = glXChooseVisual(window->display, window->screenId,
				(int *)screenAttributes);
		if(window->visualInfo == NULL) {
			free(screenAttributes);
			log_logf(window->log, LOG_LEVEL_PANIC, "failed to choose visual");
			return -1;
		}
	}

	free(screenAttributes);

	window->attributes.colormap = XCreateColormap(
			window->display,
			RootWindow(window->display, window->visualInfo->screen),
			window->visualInfo->visual,
			AllocNone
			);
	if(window->attributes.colormap == BadColor) {
		log_logf(window->log, LOG_LEVEL_PANIC,
				"failed to obtain validate video colormap attributes");
		return -1;
	}

	window->attributes.border_pixel = 0;

	window->attributes.event_mask = (ExposureMask |
			KeyPressMask |
			KeyReleaseMask |
			ButtonPressMask |
			ButtonReleaseMask |
			PointerMotionMask |
			StructureNotifyMask);

	if(isFullscreenMode) {
		bestMode = -1;
		for(ii = 0; ii < modeCount; ii++) {
			if((modes[ii]->hdisplay == screenWidth) &&
					(modes[ii]->vdisplay == screenHeight)) {
				bestMode = ii;
				break;
			}
		}

		if(bestMode == -1) {
			log_logf(window->log, LOG_LEVEL_WARNING,
					"failed to find requested resolution, defaulting to %ix%i",
					(int)modes[bestMode]->hdisplay, (int)modes[ii]->vdisplay);
			bestMode = 0;
		}

		XF86VidModeSwitchToMode(window->display, window->screenId,
				modes[bestMode]);

		XF86VidModeSetViewPort(window->display, window->screenId, 0, 0);

		screenWidth = modes[bestMode]->hdisplay;
		screenHeight = modes[bestMode]->vdisplay;

		XFree(modes);

		window->attributes.override_redirect = True;

		window->handle = XCreateWindow(
				window->display,							// display
				RootWindow(window->display,
					window->visualInfo->screen),			// parent window
				0,											// x position
				0,											// y position
				screenWidth,								// width
				screenHeight,								// height
				0,											// border
				window->visualInfo->depth,					// depth
				InputOutput,								// class
				window->visualInfo->visual,					// visual type
				CWBorderPixel |
					CWColormap |
					CWEventMask |
					CWOverrideRedirect,						// attributes mask
				&window->attributes							// attributes
				);

		XWarpPointer(window->display, None, window->handle, 0, 0, 0, 0, 0, 0);

		XMapRaised(window->display, window->handle);

		XGrabKeyboard(window->display, window->handle, True, GrabModeAsync,
				GrabModeAsync, CurrentTime);

		XGrabPointer(window->display, window->handle, True, ButtonPressMask,
				GrabModeAsync, GrabModeAsync, window->handle, None,
				CurrentTime);
	}
	else {
		window->videoMode = *modes[0];

		window->handle = XCreateWindow(
				window->display,							// display
				RootWindow(window->display,
					window->visualInfo->screen),			// parent window
				0,											// x position
				0,											// y position
				screenWidth,								// width
				screenHeight,								// height
				0,											// border
				window->visualInfo->depth,					// depth
				InputOutput,								// class
				window->visualInfo->visual,					// visual type
				CWBorderPixel | CWColormap | CWEventMask,	// attributes mask
				&window->attributes							// attributes
				);

		deleteAtom = XInternAtom(window->display, "WM_DELETE_WINDOW", True);

		XSetWMProtocols(window->display, window->handle, &deleteAtom, 1);

		XSetStandardProperties(
				window->display,
				window->handle,
				name,
				name,
				None,
				NULL,
				0,
				NULL
				);

		XMapRaised(window->display, window->handle);
	}

	if(!enableOpenGL(afalse, window)) {
		log_logf(window->log, LOG_LEVEL_ERROR,
				"failed to enable OpenGL for window '%s'", name);
		return -1;
	}

	if(tmf_spawnThread(&window->tmf, monitorWindowState, (void *)window,
				&window->threadId) < 0) {
		log_logf(window->log, LOG_LEVEL_ERROR,
				"failed to spawn window-state monitoring thread");
		return -1;
	}

	log_logf(window->log, LOG_LEVEL_INFO,
			"created window '%s' from (%i, %i) at %ix%i with %i-bit depth",
			name, window->xPosition, window->yPosition, window->windowWidth,
			window->windowHeight, window->bitsPerPixel);

	window->wasCreated = atrue;

	return 0;
}

void linuxapi_destroyWindow(LinuxApiWindow *window)
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

	if(tmf_stopThread(&window->tmf, window->threadId) < 0) {
		log_logf(window->log, LOG_LEVEL_ERROR,
				"failed to stop window-state monitoring thread");
	}

	if(tmf_free(&window->tmf) < 0) {
		log_logf(window->log, LOG_LEVEL_ERROR,
				"failed to free thread management framework");
	}

	if(window->isOpenGLEnabled) {
		disableOpenGL(window);
	}

	if(window->isFullscreenMode) {
		XF86VidModeSwitchToMode(
				window->display,
				window->screenId,
				&window->videoMode
				);

		XF86VidModeSetViewPort(
				window->display,
				window->screenId,
				0, 
				0
				);
	}

	XCloseDisplay(window->display);

	fifostack_free(&window->eventQueue);

	log_logf(window->log, LOG_LEVEL_INFO, "destroyed window");
}

void *linuxapi_enableOpenGL(aboolean isMultiThreaded, int colorBits,
		LinuxApiWindow *window)
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

	if(window->contextOne != NULL) {
		window->isOpenGLEnabled = atrue;
	}

	mutex_unlock(&window->mutex);

	return (void *)window;
}

void linuxapi_makeOpenGLCurrent(LinuxApiWindow *window, int threadType)
{
	if((window == NULL) ||
			((threadType != OSAPI_THREAD_TYPE_RENDER) &&
			 (threadType != OSAPI_THREAD_TYPE_GAMEPLAY))) {
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

	if((!window->isOpenGLEnabled) || (!window->isSecondOpenGLEnabled)) {
		log_logf(window->log, LOG_LEVEL_ERROR,
				"unable to execute %s(), OpenGL not enabled",
				__FUNCTION__);
		mutex_unlock(&window->mutex);
		return;
	}

	if(threadType == OSAPI_THREAD_TYPE_RENDER) {
		glXMakeCurrent(window->display, window->handle, window->contextOne);
	}
	else if(threadType == OSAPI_THREAD_TYPE_GAMEPLAY) {
		glXMakeCurrent(window->display, window->handle, window->contextTwo);
	}

	mutex_unlock(&window->mutex);
}

void linuxapi_disableOpenGL(LinuxApiWindow *window)
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

int linuxapi_checkState(LinuxApiWindow *window, aboolean *exitApplication)
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

void linuxapi_swapScreenBuffer(LinuxApiWindow *window)
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

	glXSwapBuffers(window->display, window->handle);

	mutex_unlock(&window->mutex);
}

void linuxapi_setMousePosition(LinuxApiWindow *window, int mouseX, int mouseY)
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

	XWarpPointer(window->display,
			None,
//			window->handle,
			window->handle,
			0,
			0,
			0,
//			window->windowWidth,
			0,
//			window->windowHeight,
			mouseX,
			mouseY);

	mutex_unlock(&window->mutex);
}

void linuxapi_enableMouseDisplay(LinuxApiWindow *window)
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

	XDefineCursor(window->display, window->handle, None);

	mutex_unlock(&window->mutex);
}

void linuxapi_disableMouseDisplay(LinuxApiWindow *window)
{
	XColor xColor;
	Pixmap emptyBitmap;
	Cursor invisibleCursor;

	char emptySet[] = { 0, 0, 0, 0, 0, 0, 0, 0 };

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

	xColor.red = 0;
	xColor.green = 0;
	xColor.blue = 0;

	emptyBitmap = XCreateBitmapFromData(window->display, window->handle,
			emptySet, 8, 8);

	invisibleCursor = XCreatePixmapCursor(window->display, emptyBitmap,
			emptyBitmap, &xColor, &xColor, 0, 0);

	XDefineCursor(window->display,window->handle, invisibleCursor);

	XFreeCursor(window->display, invisibleCursor);

	mutex_unlock(&window->mutex);
}

AsgardEvent *linuxapi_popEvent(LinuxApiWindow *window)
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

#endif // __linux__ && !__ANDROID__

