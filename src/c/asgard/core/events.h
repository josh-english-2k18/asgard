/*
 * events.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The set of events that can be obtained from the operating system for the
 * Asgard Game Engine, header file.
 *
 * Written by Josh English.
 */

#if !defined(_ASGARD_CORE_EVENTS_H)

#define _ASGARD_CORE_EVENTS_H

#if !defined(_ASGARD_H) && !defined(_ASGARD_CORE_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _ASGARD_H || _ASGARD_CORE_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// define events public constants

typedef enum _EventTypes {
	EVENT_TYPE_OS = 1,
	EVENT_TYPE_KEYBOARD,
	EVENT_TYPE_MOUSE,
	EVENT_TYPE_END_OF_LIST = -1
} EventTypes;

typedef enum _OsEventTypes {
	EVENT_OS_ACTIVE = 1,
	EVENT_OS_CLOSE,
	EVENT_OS_LOW_MEMORY,
	EVENT_OS_SCREEN_FULLSCREEN,
	EVENT_OS_SCREEN_WINDOWED,
	EVENT_OS_SCREEN_RESIZED,
	EVENT_OS_QUIT,
	EVENT_OS_END_OF_LIST = -1
} OsEventTypes;

typedef enum _KeyboardEventTypes {
	EVENT_KEYBOARD_PRESSED = 1,
	EVENT_KEYBOARD_RELEASED,
	EVENT_KEYBOARD_TYPED,
	EVENT_KEYBOARD_END_OF_LIST = -1
} KeyboardEventTypes;

typedef enum _MouseEventTypes {
	EVENT_MOUSE_PRESSED = 1,
	EVENT_MOUSE_RELEASED,
	EVENT_MOUSE_CLICKED,
	EVENT_MOUSE_DRAGGED,
	EVENT_MOUSE_MOVED,
	EVENT_MOUSE_WHEEL_UP,
	EVENT_MOUSE_WHEEL_DOWN,
	EVENT_MOUSE_END_OF_LIST = -1
} MouseEventTypes;

typedef enum _MouseButtonTypes {
	MOUSE_BUTTON_LEFT = 1,
	MOUSE_BUTTON_MIDDLE,
	MOUSE_BUTTON_RIGHT,
	EVENT_BUTTON_END_OF_LIST = -1
} MouseButtonTypes;


// define events public data types

typedef struct _AsgardOsEvent {
	aboolean isActive;
	aboolean exitApplication;
	int screenX;
	int screenY;
	int screenWidth;
	int screenHeight;
	int eventType;
} AsgardOsEvent;

typedef struct _AsgardKeyboardEvent {
	char key;
	int keyCode;
	int eventType;
} AsgardKeyboardEvent;

typedef struct _AsgardMouseEvent {
	int buttonType;
	int clickCount;
	int xPosition;
	int yPosition;
	int eventType;
} AsgardMouseEvent;

typedef struct _AsgardEvent {
	int eventType;
	double timestamp;
	void *event;
} AsgardEvent;


// declare event public functions

AsgardEvent *events_newEvent(int eventType);

int events_freeEvent(AsgardEvent *event);


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _ASGARD_CORE_EVENTS_H

