/*
 * engine.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard Game Engine central game engine component, header file.
 *
 * Written by Josh English.
 */

#if !defined(_ASGARD_ENGINE_ENGINE_H)

#define _ASGARD_ENGINE_ENGINE_H

#if !defined(_ASGARD_H) && !defined(_ASGARD_ENGINE_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _ASGARD_H || _ASGARD_ENGINE_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

// define engine public constants

#define ASGARD_ENGINE_DEVELOPMENT_BUILD						1
#define ASGARD_ENGINE_RELEASE_BUILD							2

#define ASGARD_ENGINE_RENDER_STATE_INIT						1
#define ASGARD_ENGINE_RENDER_STATE_RUNNING					2
#define ASGARD_ENGINE_RENDER_STATE_PAUSED					3
#define ASGARD_ENGINE_RENDER_STATE_STOPPED					4
#define ASGARD_ENGINE_RENDER_STATE_SHUTDOWN					5
#define ASGARD_ENGINE_RENDER_STATE_ERROR					-1

#define ASGARD_ENGINE_KEY_STATE_LENGTH						256
#define ASGARD_ENGINE_KEY_BUFFER_LENGTH						1024

#define ASGARD_ENGINE_DEFAULT_FONT							FONT_LUCIDA
#define ASGARD_ENGINE_DEFAULT_FONT_SIZE						14


// define engine public data types

typedef int (*EngineFunction)(void *engineContext, void *gameContext,
		void *argument);

/*
 * TODO: add mouse button states
 */

typedef struct _MouseState {
	int clickCount;
	int buttonType;
	int x;
	int y;
	int lastX;
	int lastY;
	int eventType;
	double lastUpdatedTime;
	Mutex mutex;
} MouseState;

typedef struct _KeyboardState {
	aboolean shiftKey;
	aboolean controlKey;
	char key;
	char lastKey;
	int keyCode;
	int lastKeyCode;
	int eventType;
	int bufferRef;
	int keyState[ASGARD_ENGINE_KEY_STATE_LENGTH];
	char buffer[ASGARD_ENGINE_KEY_BUFFER_LENGTH];
	double lastUpdatedTime;
	Mutex mutex;
} KeyboardState;

typedef struct _ScreenState {
	aboolean requiresResize;
	int screenX;
	int screenY;
	int screenWidth;
	int screenHeight;
	double lastUpdatedTime;
	Mutex mutex;
} ScreenState;

typedef struct _EngineThread {
	int id;
	EngineFunction function;
} EngineThread;

typedef struct _Engine {
	aboolean isActive;
	aboolean isInitialized;
	aboolean isOpenglInitialized;
	aboolean exitApplication;
	int colorBits;
	int buildType;
	int renderState;
	char *name;
	MouseState mouse;
	KeyboardState keyboard;
	ScreenState screen;
	Font font;
	Canvas canvas;
	Log log;
	Mutex mutex;
	EngineThread ioThread;
	EngineThread renderThread;
	EngineThread physicsThread;
	EngineThread gameplayThread;
	TmfContext tmf;
	void *window;
	void *windowHandle;
	void *gameContext;
} Engine;


// declare engine public functions

int engine_init(Engine *engine, void *handle, char *name, void *gameContext,
		aboolean createWindow, aboolean isFullscreen, int buildType,
		int screenWidth, int screenHeight, int colorBits,
		EngineFunction ioFunction, EngineFunction renderFunction,
		EngineFunction physicsFunction, EngineFunction gameplayFunction);

int engine_free(Engine *engine, aboolean destroyWindow);


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _ASGARD_ENGINE_ENGINE_H

