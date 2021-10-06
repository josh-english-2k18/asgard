/*
 * win32api.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Windows application programming interface to work with the OpenGL
 * library.
 *
 * Written by Josh English.
 */

#if defined(WIN32)

// preprocessor directives

#include "core/core.h"
#include <windowsx.h>
#define _ASGARD_CORE_COMPONENT
#include "asgard/core/common.h"
#include "asgard/core/events.h"
#include "asgard/core/osapi.h"
#define _ASGARD_CORE_PLUGIN_COMPONENT
#include "asgard/core/plugin/win32api.h"

// define win32api private constants

#define WIN32API_PIXEL_TYPE								PFD_TYPE_RGBA
#define WIN32API_LAYER_TYPE								PFD_MAIN_PLANE
#define	WIN32API_DEPTH_BITS								32
#define	WIN32API_ACCUM_BITS								0
#define	WIN32API_STENCIL_BITS							0

#define WIN32API_VERBOSE_MESSAGING						afalse


// define win32api private macros

#define WIN32_WIDTH(param)								\
	((int)(short)LOWORD(param))

#define WIN32_HEIGHT(param)								\
	((int)(short)HIWORD(param))

#define WIN32_XVALUE(param)								\
	((int)(short)LOWORD(param))

#define WIN32_YVALUE(param)								\
	((int)(short)HIWORD(param))

#define WIN32_WHEELVALUE(param)							\
	((int)(short)HIWORD(param))


// define win32api private data types

typedef struct _Win32Context {
	aboolean isActive;
	aboolean exitApplication;
	aboolean isFullscreenMode;
	int windowX;
	int windowY;
	int windowWidth;
	int windowHeight;
	int screenWidth;
	int screenHeight;
	int mouseX;
	int mouseY;
	int lastLeftMouseX;
	int lastLeftMouseY;
	int lastMiddleMouseX;
	int lastMiddleMouseY;
	int lastRightMouseX;
	int lastRightMouseY;
	FifoStack eventQueue;
	Log *log;
	Win32ApiWindow *window;
	Mutex dataMutex;
} Win32Context;


// define win32api private global variables

Win32Context *win32Context = NULL;


// declare win32api private functions

static void pushEvent(AsgardEvent *event);

static aboolean updateWindowCoordinates();

static LRESULT CALLBACK Win32MessageHandler(HWND hWnd, UINT message,
		WPARAM wParam, LPARAM lParam);

static void enableOpenGL(aboolean isMultiThreaded, int colorBits,
		Win32ApiWindow *window);

static void disableOpenGL(aboolean releaseHdc, HWND hwnd, HDC *hdc,
		HGLRC *hglrc);


// define win32api private functions

static void pushEvent(AsgardEvent *event)
{
	if(win32Context != NULL) {
		mutex_lock(&win32Context->dataMutex);

		fifostack_push(&win32Context->eventQueue, event);

		mutex_unlock(&win32Context->dataMutex);
	}
}

static aboolean updateWindowCoordinates()
{
	aboolean isUpdated = afalse;
	int windowWidth = 0;
	int windowHeight = 0;

	RECT windowDimensions;

	// obtain actual window dimensions

	GetWindowRect(win32Context->window->hwnd, &windowDimensions);

	if(win32Context->windowX != windowDimensions.left) {
		mutex_lock(&win32Context->dataMutex);
		win32Context->windowX = windowDimensions.left;
		mutex_unlock(&win32Context->dataMutex);
		isUpdated = atrue;
	}

	if(win32Context->windowY != windowDimensions.top) {
		mutex_lock(&win32Context->dataMutex);
		win32Context->windowY = windowDimensions.top;
		mutex_unlock(&win32Context->dataMutex);
		isUpdated = atrue;
	}

	windowWidth = (windowDimensions.right - windowDimensions.left);

	if(win32Context->windowWidth != windowWidth) {
		mutex_lock(&win32Context->dataMutex);
		win32Context->windowWidth = windowWidth;
		mutex_unlock(&win32Context->dataMutex);
		isUpdated = atrue;
	}

	windowHeight = (windowDimensions.bottom - windowDimensions.top);

	if(win32Context->windowHeight != windowHeight) {
		mutex_lock(&win32Context->dataMutex);
		win32Context->windowHeight = windowHeight;
		mutex_unlock(&win32Context->dataMutex);
		isUpdated = atrue;
	}

	if(isUpdated) {
		log_logf(win32Context->log, LOG_LEVEL_INFO,
				"updated window from (%i, %i) of %i x %i",
				win32Context->windowX, win32Context->windowY,
				win32Context->windowWidth, win32Context->windowHeight);
	}

	return isUpdated;
}

static LRESULT CALLBACK Win32MessageHandler(HWND hWnd, UINT message,
		WPARAM wParam, LPARAM lParam)
{
	int mouseX = 0;
	int mouseY = 0;
	int mouseWheelValue = 0;

	POINT cursorPos;

	AsgardEvent *event = NULL;

	switch(message) {
		/*
		 * operating system events
		 */

		case WM_ACTIVATE:
			if(!HIWORD(wParam)) {
				log_logf(win32Context->log, LOG_LEVEL_INFO,
						"window-message: application is active");

				event = events_newEvent(EVENT_TYPE_OS);
				((AsgardOsEvent *)event->event)->isActive = atrue;
				((AsgardOsEvent *)event->event)->eventType = EVENT_OS_ACTIVE;
				pushEvent(event);

				mutex_lock(&win32Context->dataMutex);
				win32Context->isActive = atrue;
				mutex_unlock(&win32Context->dataMutex);
			}
			else {
				log_logf(win32Context->log, LOG_LEVEL_INFO,
						"window-message: application is inactive");

				event = events_newEvent(EVENT_TYPE_OS);
				((AsgardOsEvent *)event->event)->isActive = afalse;
				((AsgardOsEvent *)event->event)->eventType = EVENT_OS_ACTIVE;
				pushEvent(event);

				mutex_lock(&win32Context->dataMutex);
				win32Context->isActive = afalse;
				mutex_unlock(&win32Context->dataMutex);
			}
			return 0;

		case WM_CLOSE:
			log_logf(win32Context->log, LOG_LEVEL_INFO,
					"window-message: close application");

			event = events_newEvent(EVENT_TYPE_OS);
			((AsgardOsEvent *)event->event)->exitApplication = atrue;
			((AsgardOsEvent *)event->event)->eventType = EVENT_OS_CLOSE;
			pushEvent(event);

			mutex_lock(&win32Context->dataMutex);
			win32Context->exitApplication = atrue;
			mutex_unlock(&win32Context->dataMutex);
			return 0;

		case WM_COMPACTING:
			log_logf(win32Context->log, LOG_LEVEL_INFO,
					"window-message: system is low on memory, "
					"aborting application");

			event = events_newEvent(EVENT_TYPE_OS);
			((AsgardOsEvent *)event->event)->exitApplication = atrue;
			((AsgardOsEvent *)event->event)->eventType = EVENT_OS_LOW_MEMORY;
			pushEvent(event);

			mutex_lock(&win32Context->dataMutex);
			win32Context->exitApplication = atrue;
			mutex_unlock(&win32Context->dataMutex);
			break;

		case WM_SIZE:
			log_logf(win32Context->log, LOG_LEVEL_INFO,
					"window-message: window resized to %ix%i",
					WIN32_WIDTH(lParam), WIN32_HEIGHT(lParam));

			mutex_lock(&win32Context->dataMutex);
			win32Context->screenWidth = WIN32_WIDTH(lParam);
			win32Context->screenHeight = WIN32_HEIGHT(lParam);
			mutex_unlock(&win32Context->dataMutex);

			event = events_newEvent(EVENT_TYPE_OS);
			((AsgardOsEvent *)event->event)->eventType =
				EVENT_OS_SCREEN_RESIZED;
			((AsgardOsEvent *)event->event)->screenWidth =
				win32Context->screenWidth;
			((AsgardOsEvent *)event->event)->screenHeight =
				win32Context->screenHeight;
			pushEvent(event);
			break;

		case WM_QUIT:
			log_logf(win32Context->log, LOG_LEVEL_INFO,
					"window-message: quit application");

			event = events_newEvent(EVENT_TYPE_OS);
			((AsgardOsEvent *)event->event)->exitApplication = atrue;
			((AsgardOsEvent *)event->event)->eventType = EVENT_OS_QUIT;
			pushEvent(event);
			return 0;

		/*
		 * keyboard events
		 */

		case WM_CHAR:
			log_logf(win32Context->log, LOG_LEVEL_DEBUG,
					"window-message: char #%i (%c) pressed", (int)wParam,
					(char)wParam);

			event = events_newEvent(EVENT_TYPE_KEYBOARD);
			((AsgardKeyboardEvent *)event->event)->key = (char)wParam;
			((AsgardKeyboardEvent *)event->event)->keyCode = (int)(wParam - 32);
			((AsgardKeyboardEvent *)event->event)->eventType =
				EVENT_KEYBOARD_PRESSED;
			pushEvent(event);

			event = events_newEvent(EVENT_TYPE_KEYBOARD);
			((AsgardKeyboardEvent *)event->event)->key = (char)wParam;
			((AsgardKeyboardEvent *)event->event)->keyCode = (int)(wParam - 32);
			((AsgardKeyboardEvent *)event->event)->eventType =
				EVENT_KEYBOARD_TYPED;
			pushEvent(event);
			break;

		case WM_KEYDOWN:
			log_logf(win32Context->log, LOG_LEVEL_DEBUG,
					"window-message: char #%i (%c) down", (int)wParam,
					(char)(wParam + 32));

			event = events_newEvent(EVENT_TYPE_KEYBOARD);
			((AsgardKeyboardEvent *)event->event)->key = (char)(wParam + 32);
			((AsgardKeyboardEvent *)event->event)->keyCode = (int)wParam;
			((AsgardKeyboardEvent *)event->event)->eventType =
				EVENT_KEYBOARD_PRESSED;
			pushEvent(event);
			break;

		case WM_KEYUP:
			log_logf(win32Context->log, LOG_LEVEL_DEBUG,
					"window-message: char #%i (%c) released", (int)wParam,
					(char)(wParam + 32));

			event = events_newEvent(EVENT_TYPE_KEYBOARD);
			((AsgardKeyboardEvent *)event->event)->key = (char)(wParam + 32);
			((AsgardKeyboardEvent *)event->event)->keyCode = (int)wParam;
			((AsgardKeyboardEvent *)event->event)->eventType =
				EVENT_KEYBOARD_RELEASED;
			pushEvent(event);
			break;

		/*
		 * mouse events
		 */

		case WM_LBUTTONDBLCLK: // left mouse button was double-clicked
			log_logf(win32Context->log, LOG_LEVEL_DEBUG,
					"window-message: mouse left-button double-clicked");

			event = events_newEvent(EVENT_TYPE_MOUSE);
			((AsgardMouseEvent *)event->event)->buttonType = MOUSE_BUTTON_LEFT;
			((AsgardMouseEvent *)event->event)->clickCount = 2;
			((AsgardMouseEvent *)event->event)->xPosition =
				win32Context->mouseX;
			((AsgardMouseEvent *)event->event)->yPosition =
				win32Context->mouseY;
			((AsgardMouseEvent *)event->event)->eventType = EVENT_MOUSE_CLICKED;
			pushEvent(event);
			break;

		case WM_LBUTTONDOWN: // left mouse button was pressed
			log_logf(win32Context->log, LOG_LEVEL_DEBUG,
					"window-message: mouse left-button pressed");

			mutex_lock(&win32Context->dataMutex);
			win32Context->lastLeftMouseX = win32Context->mouseX;
			win32Context->lastLeftMouseY = win32Context->mouseY;
			mutex_unlock(&win32Context->dataMutex);

			event = events_newEvent(EVENT_TYPE_MOUSE);
			((AsgardMouseEvent *)event->event)->buttonType = MOUSE_BUTTON_LEFT;
			((AsgardMouseEvent *)event->event)->xPosition =
				win32Context->mouseX;
			((AsgardMouseEvent *)event->event)->yPosition =
				win32Context->mouseY;
			((AsgardMouseEvent *)event->event)->eventType = EVENT_MOUSE_PRESSED;
			pushEvent(event);
			break;

		case WM_LBUTTONUP: // left mouse button was released
			log_logf(win32Context->log, LOG_LEVEL_DEBUG,
					"window-message: mouse left-button released");

			if((win32Context->lastLeftMouseX != win32Context->mouseX) ||
					(win32Context->lastLeftMouseY != win32Context->mouseY)) {
				event = events_newEvent(EVENT_TYPE_MOUSE);
				((AsgardMouseEvent *)event->event)->xPosition =
					win32Context->mouseX;
				((AsgardMouseEvent *)event->event)->yPosition =
					win32Context->mouseY;
				((AsgardMouseEvent *)event->event)->eventType =
					EVENT_MOUSE_DRAGGED;
				pushEvent(event);
			}

			event = events_newEvent(EVENT_TYPE_MOUSE);
			((AsgardMouseEvent *)event->event)->buttonType = MOUSE_BUTTON_LEFT;
			((AsgardMouseEvent *)event->event)->xPosition =
				win32Context->mouseX;
			((AsgardMouseEvent *)event->event)->yPosition =
				win32Context->mouseY;
			((AsgardMouseEvent *)event->event)->eventType =
				EVENT_MOUSE_RELEASED;
			pushEvent(event);

			event = events_newEvent(EVENT_TYPE_MOUSE);
			((AsgardMouseEvent *)event->event)->buttonType = MOUSE_BUTTON_LEFT;
			((AsgardMouseEvent *)event->event)->clickCount = 1;
			((AsgardMouseEvent *)event->event)->xPosition =
				win32Context->mouseX;
			((AsgardMouseEvent *)event->event)->yPosition =
				win32Context->mouseY;
			((AsgardMouseEvent *)event->event)->eventType =
				EVENT_MOUSE_CLICKED;
			pushEvent(event);
			break;

		case WM_MBUTTONDBLCLK: // middle mouse button was double-clicked
			log_logf(win32Context->log, LOG_LEVEL_DEBUG,
					"window-message: mouse middle-button double-clicked");

			event = events_newEvent(EVENT_TYPE_MOUSE);
			((AsgardMouseEvent *)event->event)->buttonType =
				MOUSE_BUTTON_MIDDLE;
			((AsgardMouseEvent *)event->event)->clickCount = 2;
			((AsgardMouseEvent *)event->event)->xPosition =
				win32Context->mouseX;
			((AsgardMouseEvent *)event->event)->yPosition =
				win32Context->mouseY;
			((AsgardMouseEvent *)event->event)->eventType = EVENT_MOUSE_CLICKED;
			pushEvent(event);
			break;

		case WM_MBUTTONDOWN: // middle mouse button was pressed
			log_logf(win32Context->log, LOG_LEVEL_DEBUG,
					"window-message: mouse middle-button pressed");

			mutex_lock(&win32Context->dataMutex);
			win32Context->lastMiddleMouseX = win32Context->mouseX;
			win32Context->lastMiddleMouseY = win32Context->mouseY;
			mutex_unlock(&win32Context->dataMutex);

			event = events_newEvent(EVENT_TYPE_MOUSE);
			((AsgardMouseEvent *)event->event)->buttonType =
				MOUSE_BUTTON_MIDDLE;
			((AsgardMouseEvent *)event->event)->xPosition =
				win32Context->mouseX;
			((AsgardMouseEvent *)event->event)->yPosition =
				win32Context->mouseY;
			((AsgardMouseEvent *)event->event)->eventType = EVENT_MOUSE_PRESSED;
			pushEvent(event);
			break;

		case WM_MBUTTONUP: // middle mouse button was released
			log_logf(win32Context->log, LOG_LEVEL_DEBUG,
					"window-message: mouse middle-button released");

			if((win32Context->lastMiddleMouseX != win32Context->mouseX) ||
					(win32Context->lastMiddleMouseY != win32Context->mouseY)) {
				event = events_newEvent(EVENT_TYPE_MOUSE);
				((AsgardMouseEvent *)event->event)->xPosition =
					win32Context->mouseX;
				((AsgardMouseEvent *)event->event)->yPosition =
					win32Context->mouseY;
				((AsgardMouseEvent *)event->event)->eventType =
					EVENT_MOUSE_DRAGGED;
				pushEvent(event);
			}

			event = events_newEvent(EVENT_TYPE_MOUSE);
			((AsgardMouseEvent *)event->event)->buttonType =
				MOUSE_BUTTON_MIDDLE;
			((AsgardMouseEvent *)event->event)->xPosition =
				win32Context->mouseX;
			((AsgardMouseEvent *)event->event)->yPosition =
				win32Context->mouseY;
			((AsgardMouseEvent *)event->event)->eventType =
				EVENT_MOUSE_RELEASED;
			pushEvent(event);

			event = events_newEvent(EVENT_TYPE_MOUSE);
			((AsgardMouseEvent *)event->event)->buttonType =
				MOUSE_BUTTON_MIDDLE;
			((AsgardMouseEvent *)event->event)->clickCount = 1;
			((AsgardMouseEvent *)event->event)->xPosition =
				win32Context->mouseX;
			((AsgardMouseEvent *)event->event)->yPosition =
				win32Context->mouseY;
			((AsgardMouseEvent *)event->event)->eventType =
				EVENT_MOUSE_CLICKED;
			pushEvent(event);
			break;

		case WM_RBUTTONDBLCLK: // right mouse button was double-clicked
			log_logf(win32Context->log, LOG_LEVEL_DEBUG,
					"window-message: mouse right-button double-clicked");

			event = events_newEvent(EVENT_TYPE_MOUSE);
			((AsgardMouseEvent *)event->event)->buttonType = MOUSE_BUTTON_RIGHT;
			((AsgardMouseEvent *)event->event)->clickCount = 2;
			((AsgardMouseEvent *)event->event)->xPosition =
				win32Context->mouseX;
			((AsgardMouseEvent *)event->event)->yPosition =
				win32Context->mouseY;
			((AsgardMouseEvent *)event->event)->eventType = EVENT_MOUSE_CLICKED;
			pushEvent(event);
			break;

		case WM_RBUTTONDOWN: // right mouse button was pressed
			log_logf(win32Context->log, LOG_LEVEL_DEBUG,
					"window-message: mouse right-button pressed");

			mutex_lock(&win32Context->dataMutex);
			win32Context->lastRightMouseX = win32Context->mouseX;
			win32Context->lastRightMouseY = win32Context->mouseY;
			mutex_unlock(&win32Context->dataMutex);

			event = events_newEvent(EVENT_TYPE_MOUSE);
			((AsgardMouseEvent *)event->event)->buttonType = MOUSE_BUTTON_RIGHT;
			((AsgardMouseEvent *)event->event)->xPosition =
				win32Context->mouseX;
			((AsgardMouseEvent *)event->event)->yPosition =
				win32Context->mouseY;
			((AsgardMouseEvent *)event->event)->eventType = EVENT_MOUSE_PRESSED;
			pushEvent(event);
			break;

		case WM_RBUTTONUP: // right mouse button was released
			log_logf(win32Context->log, LOG_LEVEL_DEBUG,
					"window-message: mouse right-button released");

			if((win32Context->lastRightMouseX != win32Context->mouseX) ||
					(win32Context->lastRightMouseY != win32Context->mouseY)) {
				event = events_newEvent(EVENT_TYPE_MOUSE);
				((AsgardMouseEvent *)event->event)->xPosition =
					win32Context->mouseX;
				((AsgardMouseEvent *)event->event)->yPosition =
					win32Context->mouseY;
				((AsgardMouseEvent *)event->event)->eventType =
					EVENT_MOUSE_DRAGGED;
				pushEvent(event);
			}

			event = events_newEvent(EVENT_TYPE_MOUSE);
			((AsgardMouseEvent *)event->event)->buttonType = MOUSE_BUTTON_RIGHT;
			((AsgardMouseEvent *)event->event)->xPosition =
				win32Context->mouseX;
			((AsgardMouseEvent *)event->event)->yPosition =
				win32Context->mouseY;
			((AsgardMouseEvent *)event->event)->eventType =
				EVENT_MOUSE_RELEASED;
			pushEvent(event);

			event = events_newEvent(EVENT_TYPE_MOUSE);
			((AsgardMouseEvent *)event->event)->buttonType = MOUSE_BUTTON_RIGHT;
			((AsgardMouseEvent *)event->event)->clickCount = 1;
			((AsgardMouseEvent *)event->event)->xPosition =
				win32Context->mouseX;
			((AsgardMouseEvent *)event->event)->yPosition =
				win32Context->mouseY;
			((AsgardMouseEvent *)event->event)->eventType =
				EVENT_MOUSE_CLICKED;
			pushEvent(event);
			break;

		case WM_MOUSEMOVE: // mouse moved
			if(!GetCursorPos(&cursorPos)) {
				break;
			}

			if(updateWindowCoordinates()) {
				event = events_newEvent(EVENT_TYPE_OS);

				((AsgardOsEvent *)event->event)->eventType =
					EVENT_OS_SCREEN_RESIZED;
				((AsgardOsEvent *)event->event)->screenWidth =
					win32Context->screenWidth;
				((AsgardOsEvent *)event->event)->screenHeight =
					win32Context->screenHeight;

				pushEvent(event);
			}

			mouseX = (cursorPos.x -
					(win32Context->windowX +
					 ((win32Context->windowWidth -
					   win32Context->screenWidth) / 2)));
			mouseY = (cursorPos.y -
					(win32Context->windowY +
					 ((win32Context->windowHeight -
					   win32Context->screenHeight) / 2)));

			if(!win32Context->isFullscreenMode) {
				mouseY -= 13;
			}

			if((win32Context->mouseX == mouseX) &&
					(win32Context->mouseY == mouseY)) {
				break;
			}

			log_logf(win32Context->log, LOG_LEVEL_DEBUG,
					"window-message: mouse moved from (%i, %i) to (%i, %i) "
					"actual (%i, %i)",
					win32Context->mouseX, win32Context->mouseY,
					mouseX, mouseY, cursorPos.x, cursorPos.y);

			event = events_newEvent(EVENT_TYPE_MOUSE);
			((AsgardMouseEvent *)event->event)->eventType = EVENT_MOUSE_MOVED;
			((AsgardMouseEvent *)event->event)->xPosition = mouseX;
			((AsgardMouseEvent *)event->event)->yPosition = mouseY;
			pushEvent(event);

			mutex_lock(&win32Context->dataMutex);
			win32Context->mouseX = mouseX;
			win32Context->mouseY = mouseY;
			mutex_unlock(&win32Context->dataMutex);
			break;

		case WM_MOUSEWHEEL: // mouse wheel moved
			mouseWheelValue = WIN32_WHEELVALUE(wParam);

			log_logf(win32Context->log, LOG_LEVEL_DEBUG,
					"window-message: mouse wheel moved %i", mouseWheelValue);

			event = events_newEvent(EVENT_TYPE_MOUSE);

			if(mouseWheelValue > 0) {
				((AsgardMouseEvent *)event->event)->eventType =
					EVENT_MOUSE_WHEEL_UP;
			}
			else {
				((AsgardMouseEvent *)event->event)->eventType =
					EVENT_MOUSE_WHEEL_DOWN;
			}

			((AsgardMouseEvent *)event->event)->xPosition =
				win32Context->mouseX;
			((AsgardMouseEvent *)event->event)->yPosition =
				win32Context->mouseY;
			pushEvent(event);
			break;

		/*
		 * non-event messages
		 */

		case WM_DESTROY:
			log_logf(win32Context->log, LOG_LEVEL_INFO,
					"window-message: destroy window");
			PostQuitMessage(0);
			return 0;

		case WM_SYSCOMMAND:
			switch(wParam) {
				case SC_SCREENSAVE:
					log_logf(win32Context->log, LOG_LEVEL_INFO,
							"window-message: activate screensaver");
				case SC_MONITORPOWER:
					log_logf(win32Context->log, LOG_LEVEL_INFO,
							"window-message: enter powersave mode");
					return 0;
			}
			break;

		case WM_PAINT: // TODO: do i need this?
			log_logf(win32Context->log, LOG_LEVEL_INFO,
					"window-message: paint window");
			break;

		case WM_TIMER: // TODO: do i need this? perhaps for the plugin?
			log_logf(win32Context->log, LOG_LEVEL_DEBUG,
					"window-message: timer executed");
			break;

		default:
			if(WIN32API_VERBOSE_MESSAGING) {
				log_logf(win32Context->log, LOG_LEVEL_DEBUG,
						"window-message: unhandled message #%i param: %i",
						(int)message, (int)wParam);
			}
			break;				
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

static void enableOpenGL(aboolean isMultiThreaded, int colorBits,
		Win32ApiWindow *window)
{
	int pixelFormat = 0;

	HDC localHdc;
	HGLRC localHglrc;
	PIXELFORMATDESCRIPTOR pfd;

	if((window->isOpenGLEnabled) && (window->isSecondOpenGLEnabled)) {
		return;
	}

	// get the device context

	localHdc = GetDC(window->hwnd);

	// set the pixel format for the device context

	memset((void *)&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));

	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;

	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.dwLayerMask = PFD_MAIN_PLANE;

	pfd.iPixelType = WIN32API_PIXEL_TYPE;
	pfd.iLayerType = WIN32API_LAYER_TYPE;
	pfd.cColorBits = colorBits;
	pfd.cDepthBits = WIN32API_DEPTH_BITS;
	pfd.cAccumBits = WIN32API_ACCUM_BITS;
	pfd.cStencilBits = WIN32API_STENCIL_BITS;

	if((pixelFormat = ChoosePixelFormat(localHdc, &pfd)) != FALSE) {
		SetPixelFormat(localHdc, pixelFormat, &pfd);
	}
	else {
		fprintf(stderr, "error - failed to set pixel format on this machine.");
	}

	// create and enable the render context (RC)

	localHglrc = wglCreateContext(localHdc);

	if(!isMultiThreaded) {
		wglMakeCurrent(localHdc, localHglrc);
	}

	window->hdc = localHdc;
	window->hglrcOne = localHglrc;

	if(isMultiThreaded) {
		window->hglrcTwo = wglCreateContext(localHdc);;
		wglShareLists(window->hglrcTwo, window->hglrcOne);
		window->isSecondOpenGLEnabled = atrue;
	}
}

static void disableOpenGL(aboolean releaseHdc, HWND hwnd, HDC *hdc,
		HGLRC *hglrc)
{
	wglMakeCurrent(NULL, NULL);

	wglDeleteContext(*hglrc);

	if(releaseHdc) {
		ReleaseDC(hwnd, *hdc);
	}
}

// define win32api public functions

int win32api_createWindow(Win32ApiWindow *window, char *name,
		aboolean isFullscreenMode, int screenWidth, int screenHeight,
		int colorBits, Log *log)
{
	DWORD windowStyle = (DWORD)0;
	DWORD windowExtendedStyle = (DWORD)0;
	RECT windowDimensions;
	DEVMODE deviceMode;
	WNDCLASS windowClass;

	if((window == NULL) || (screenWidth < 320) || (screenHeight < 240) ||
			((colorBits != 8) &&
			 (colorBits != 16) &&
			 (colorBits != 24) &&
			 (colorBits != 32))) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	// sanitize arguments

	if(name == NULL) {
		name = "Project Asgard Game Engine";
	}

	// initialize window

	memset((void *)window, 0, sizeof(Win32ApiWindow));

	window->wasCreated = afalse;
	window->isOpenGLEnabled = afalse;
	window->hdc = (HDC)NULL;
	window->hglrcOne = (HGLRC)0;
	window->hglrcTwo = (HGLRC)0;
	window->hwnd = (HWND)NULL;
	window->log = log;

	if(mutex_init(&window->mutex) < 0) {
		log_logf(window->log, LOG_LEVEL_ERROR,
				"failed to initialize the window mutex");
		return -1;
	}

	mutex_lock(&window->mutex);

	// initialize window context

	win32Context = (Win32Context *)malloc(sizeof(Win32Context));

	win32Context->isActive = atrue;
	win32Context->exitApplication = afalse;
	win32Context->isFullscreenMode = isFullscreenMode;
	win32Context->windowX = 0;
	win32Context->windowY = 0;
	win32Context->windowWidth = 0;
	win32Context->windowHeight = 0;
	win32Context->screenWidth = screenWidth;
	win32Context->screenHeight = screenHeight;
	win32Context->mouseX = 0;
	win32Context->mouseY = 0;
	win32Context->lastLeftMouseX = 0;
	win32Context->lastLeftMouseY = 0;
	win32Context->lastMiddleMouseX = 0;
	win32Context->lastMiddleMouseY = 0;
	win32Context->lastRightMouseX = 0;
	win32Context->lastRightMouseY = 0;

	fifostack_init(&win32Context->eventQueue);
	fifostack_setFreeFunction(&win32Context->eventQueue, events_freeEvent);

	win32Context->log = log;

	win32Context->window = window;

	if(mutex_init(&win32Context->dataMutex) < 0) {
		log_logf(window->log, LOG_LEVEL_ERROR,
				"failed to initialize the window-context data-mutex");
		return -1;
	}

	// setup window class

	window->hinstance = GetModuleHandle(NULL);

	memset((void *)&windowClass, 0, sizeof(windowClass));

	windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	windowClass.lpfnWndProc = Win32MessageHandler;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = window->hinstance;
	windowClass.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	windowClass.hbrBackground = NULL;
	windowClass.lpszMenuName = NULL;
	windowClass.lpszClassName = "AsgardGameEngine";

	if(!RegisterClass(&windowClass)) {
		log_logf(window->log, LOG_LEVEL_PANIC,
				"failed to register the window class");
		mutex_unlock(&window->mutex);
		return -1;
	}

	// setup window styling

	if(isFullscreenMode) {
		memset((void *)&deviceMode, 0, sizeof(deviceMode));

		deviceMode.dmSize = sizeof(deviceMode);
		deviceMode.dmPelsWidth = screenWidth;
		deviceMode.dmPelsHeight = screenHeight;
		deviceMode.dmBitsPerPel = colorBits;
		deviceMode.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		if(ChangeDisplaySettings(&deviceMode,
					CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL) {
			isFullscreenMode = afalse;
			windowStyle = WS_OVERLAPPEDWINDOW;
			windowExtendedStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		}
		else {
			windowStyle = WS_POPUP;
			windowExtendedStyle = WS_EX_APPWINDOW;
		}
	}
	else {
		windowStyle = WS_OVERLAPPEDWINDOW;
		windowExtendedStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
	}

	windowStyle |= WS_CLIPSIBLINGS;
	windowStyle |= WS_CLIPCHILDREN;

	// determine window dimensions

	windowDimensions.left = (long)0;
	windowDimensions.right = (long)screenWidth;
	windowDimensions.top = (long)0;
	windowDimensions.bottom = (long)screenHeight;

	AdjustWindowRectEx(&windowDimensions, windowStyle, FALSE,
			windowExtendedStyle);

	// create window

	window->hwnd = CreateWindowEx(
			windowExtendedStyle,
			"AsgardGameEngine",
			name,
			windowStyle,
			((GetSystemMetrics(SM_CXSCREEN) / 2) -
			 ((windowDimensions.right - windowDimensions.left) / 2)),
			((GetSystemMetrics(SM_CYSCREEN) / 2) -
			 ((windowDimensions.bottom - windowDimensions.top) / 2)),
			(windowDimensions.right - windowDimensions.left),
			(windowDimensions.bottom - windowDimensions.top),
			NULL,
			NULL,
			window->hinstance,
			NULL
			);

	if(window->hwnd == NULL) {
		log_logf(window->log, LOG_LEVEL_PANIC,
				"failed to create the window");
		mutex_unlock(&window->mutex);
		return -1;
	}

	// set the window as the user focus

	ShowWindow(window->hwnd, SW_SHOW);
	SetForegroundWindow(window->hwnd);
	SetFocus(window->hwnd);

	// obtain actual window dimensions

	GetWindowRect(window->hwnd, &windowDimensions);

	win32Context->windowX = windowDimensions.left;
	win32Context->windowY = windowDimensions.top;
	win32Context->windowWidth = (windowDimensions.right -
			windowDimensions.left);
	win32Context->windowHeight = (windowDimensions.bottom -
			windowDimensions.top);

	log_logf(window->log, LOG_LEVEL_INFO,
			"created window '%s' from (%i, %i) of %i x %i", name,
			win32Context->windowX, win32Context->windowY,
			win32Context->windowWidth, win32Context->windowHeight);

	window->wasCreated = atrue;

	mutex_unlock(&window->mutex);

	return 0;
}

int win32api_initWithinWindow(Win32ApiWindow *window, void *handle, char *name,
		aboolean isFullscreenMode, int screenWidth, int screenHeight,
		int colorBits, Log *log)
{
	DWORD windowStyle = (DWORD)0;
	DWORD windowExtendedStyle = (DWORD)0;
	RECT windowDimensions;

	if((window == NULL) || (handle == NULL) ||
			(screenWidth < 320) || (screenHeight < 240) ||
			((colorBits != 8) &&
			 (colorBits != 16) &&
			 (colorBits != 24) &&
			 (colorBits != 32))) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	// sanitize arguments

	if(name == NULL) {
		name = "Project Asgard Game Engine";
	}

	// initialize window

	memset((void *)window, 0, sizeof(Win32ApiWindow));

	window->wasCreated = atrue;
	window->isOpenGLEnabled = afalse;
	window->hdc = (HDC)NULL;
	window->hglrcOne = (HGLRC)0;
	window->hglrcTwo = (HGLRC)0;
	window->hwnd = (HWND)handle;
	window->log = log;

	if(mutex_init(&window->mutex) < 0) {
		log_logf(window->log, LOG_LEVEL_ERROR,
				"failed to initialize the window mutex");
		return -1;
	}

	mutex_lock(&window->mutex);

	// initialize window context

	win32Context = (Win32Context *)malloc(sizeof(Win32Context));

	win32Context->isActive = atrue;
	win32Context->exitApplication = afalse;
	win32Context->isFullscreenMode = isFullscreenMode;
	win32Context->windowX = 0;
	win32Context->windowY = 0;
	win32Context->windowWidth = 0;
	win32Context->windowHeight = 0;
	win32Context->screenWidth = screenWidth;
	win32Context->screenHeight = screenHeight;
	win32Context->mouseX = 0;
	win32Context->mouseY = 0;
	win32Context->lastLeftMouseX = 0;
	win32Context->lastLeftMouseY = 0;
	win32Context->lastMiddleMouseX = 0;
	win32Context->lastMiddleMouseY = 0;
	win32Context->lastRightMouseX = 0;
	win32Context->lastRightMouseY = 0;

	fifostack_init(&win32Context->eventQueue);
	fifostack_setFreeFunction(&win32Context->eventQueue, events_freeEvent);

	win32Context->log = log;

	win32Context->window = window;

	if(mutex_init(&win32Context->dataMutex) < 0) {
		log_logf(window->log, LOG_LEVEL_ERROR,
				"failed to initialize the window-context data-mutex");
		return -1;
	}

	// subclass the window

	window->subclassHandle = SubclassWindow(window->hwnd,
			(WNDPROC)Win32MessageHandler);

	// associate the window with this application instance

	SetWindowLong(window->hwnd, GWL_USERDATA,
			GetWindowLong(window->hwnd, GWL_USERDATA));

	// obtain actual window dimensions

	GetWindowRect(window->hwnd, &windowDimensions);

	win32Context->windowX = windowDimensions.left;
	win32Context->windowY = windowDimensions.top;
	win32Context->windowWidth = (windowDimensions.right -
			windowDimensions.left);
	win32Context->windowHeight = (windowDimensions.bottom -
			windowDimensions.top);

	log_logf(window->log, LOG_LEVEL_INFO,
			"initialized sub-window '%s' from (%i, %i) of %i x %i", name,
			win32Context->windowX, win32Context->windowY,
			win32Context->windowWidth, win32Context->windowHeight);

	window->wasCreated = atrue;

	mutex_unlock(&window->mutex);

	return 0;
}

void win32api_destroyWindow(Win32ApiWindow *window)
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

	if(window->isOpenGLEnabled) {
		if(window->isSecondOpenGLEnabled) {
			disableOpenGL(afalse, window->hwnd, &window->hdc,
					&window->hglrcTwo);
		}
		disableOpenGL(atrue, window->hwnd, &window->hdc,
				&window->hglrcOne);
	}

	DestroyWindow(window->hwnd);
	UnregisterClass("AsgardGameEngine", window->hinstance);

	if(win32Context != NULL) {
		mutex_lock(&win32Context->dataMutex);
		fifostack_free(&win32Context->eventQueue);
		mutex_unlock(&win32Context->dataMutex);
		mutex_free(&win32Context->dataMutex);

		memset(win32Context, 0, sizeof(Win32Context));

		free(win32Context);
	}

	log_logf(window->log, LOG_LEVEL_INFO, "destroyed window");

	mutex_unlock(&window->mutex);
	mutex_free(&window->mutex);

	memset(window, 0, sizeof(Win32ApiWindow));
}

void *win32api_enableOpenGL(aboolean isMultiThreaded, int colorBits,
		Win32ApiWindow *window)
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

	mutex_lock(&window->mutex);

	if(!window->wasCreated) {
		log_logf(window->log, LOG_LEVEL_ERROR,
				"unable to execute %s(), window not created",
				__FUNCTION__);
		mutex_unlock(&window->mutex);
		return NULL;
	}

	enableOpenGL(isMultiThreaded, colorBits, window);

	result = (void *)window->hdc;

	if(result != NULL) {
		window->isOpenGLEnabled = atrue;
	}

	mutex_unlock(&window->mutex);

	return result;
}

void win32api_makeOpenGLCurrent(Win32ApiWindow *window, int threadType)
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
		wglMakeCurrent(window->hdc, window->hglrcOne);
	}
	else if(threadType == OSAPI_THREAD_TYPE_GAMEPLAY) {
		wglMakeCurrent(window->hdc, window->hglrcTwo);
	}

	mutex_unlock(&window->mutex);
}

void win32api_disableOpenGL(Win32ApiWindow *window)
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

	if(window->isSecondOpenGLEnabled) {
		disableOpenGL(afalse, window->hwnd, &window->hdc, &window->hglrcTwo);
		window->isSecondOpenGLEnabled = afalse;
	}
	disableOpenGL(atrue, window->hwnd, &window->hdc, &window->hglrcOne);

	window->isOpenGLEnabled = afalse;

	mutex_unlock(&window->mutex);
}

int win32api_checkState(Win32ApiWindow *window, aboolean *exitApplication)
{
	int resultState = 0;

	MSG message;

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

	if(PeekMessage(&message, window->hwnd, 0, 0, PM_REMOVE)) {
		if(message.message == WM_QUIT) {
			*exitApplication = atrue;
			resultState = (int)message.wParam;
		}
		else {
			TranslateMessage(&message);
			DispatchMessage(&message);
		}
	}

	if(win32Context != NULL) {
		mutex_lock(&win32Context->dataMutex);

		if(win32Context->exitApplication) {
			*exitApplication = atrue;
		}

		mutex_unlock(&win32Context->dataMutex);
	}

	mutex_unlock(&window->mutex);

	return resultState;
}

void win32api_swapScreenBuffer(HDC hdc)
{
	if((hdc == (HDC)NULL) || (win32Context == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	mutex_lock(&win32Context->window->mutex);

	if(!win32Context->window->wasCreated) {
		log_logf(win32Context->window->log, LOG_LEVEL_ERROR,
				"unable to execute %s(), window not created",
				__FUNCTION__);
		mutex_unlock(&win32Context->window->mutex);
		return;
	}

	SwapBuffers(hdc);

	mutex_unlock(&win32Context->window->mutex);
}

void win32api_setMousePosition(Win32ApiWindow *window, int mouseX, int mouseY)
{
	int localMouseX = 0;
	int localMouseY = 0;

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

	localMouseX = (mouseX +
			(win32Context->windowX +
				((win32Context->windowWidth -
				  win32Context->screenWidth) / 2)));
	localMouseY = (mouseY +
			(win32Context->windowY +
				((win32Context->windowHeight -
				  win32Context->screenHeight) / 2)));

	if(!win32Context->isFullscreenMode) {
		localMouseY += 13;
	}

	if(SetCursorPos(localMouseX, localMouseY)) {
		log_logf(window->log, LOG_LEVEL_DEBUG,
				"window-notification: set mouse position to (%i, %i) "
				"actual (%i, %i)",
				mouseX, mouseY, localMouseX, localMouseY);

		win32Context->mouseX = mouseX;
		win32Context->mouseY = mouseY;
	}

	mutex_unlock(&window->mutex);
}

void win32api_enableMouseDisplay(Win32ApiWindow *window)
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

	ShowCursor(TRUE);

	mutex_unlock(&window->mutex);
}

void win32api_disableMouseDisplay(Win32ApiWindow *window)
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

	ShowCursor(FALSE);

	mutex_unlock(&window->mutex);
}

AsgardEvent *win32api_popEvent(Win32ApiWindow *window)
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

	if(win32Context != NULL) {
		mutex_lock(&win32Context->dataMutex);

		if(fifostack_pop(&win32Context->eventQueue, &result) < 0) {
			result = NULL;
		}

		mutex_unlock(&win32Context->dataMutex);
	}

	mutex_unlock(&window->mutex);

	return (AsgardEvent *)result;
}

#endif // WIN32

