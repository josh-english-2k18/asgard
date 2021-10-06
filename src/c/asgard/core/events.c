/*
 * events.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The set of events that can be obtained from the operating system for the
 * Asgard Game Engine.
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

// declare event private functions

static AsgardOsEvent *newOsEvent();

static AsgardKeyboardEvent *newKeyboardEvent();

static AsgardMouseEvent *newMouseEvent();

// define event private functions

static AsgardOsEvent *newOsEvent()
{
	AsgardOsEvent *result = NULL;

	result = (AsgardOsEvent *)malloc(sizeof(AsgardOsEvent));

	result->isActive = afalse;
	result->exitApplication = afalse;
	result->screenX = 0;
	result->screenY = 0;
	result->screenWidth = 0;
	result->screenHeight = 0;
	result->eventType = EVENT_OS_END_OF_LIST;

	return result;
}

static AsgardKeyboardEvent *newKeyboardEvent()
{
	AsgardKeyboardEvent *result = NULL;

	result = (AsgardKeyboardEvent *)malloc(sizeof(AsgardKeyboardEvent));

	result->key = (char)0;
	result->keyCode = ASGARD_KEY_END_OF_LIST;
	result->eventType = EVENT_KEYBOARD_END_OF_LIST;

	return result;
}

static AsgardMouseEvent *newMouseEvent()
{
	AsgardMouseEvent *result = NULL;

	result = (AsgardMouseEvent *)malloc(sizeof(AsgardMouseEvent));

	result->buttonType = EVENT_BUTTON_END_OF_LIST;
	result->clickCount = 0;
	result->xPosition = 0;
	result->yPosition = 0;
	result->eventType = EVENT_MOUSE_END_OF_LIST;

	return result;
}

// define event public functions

/**
 * events_newEvent()
 *
 * This function does not follow the normal integer-return type for a specific
 * convenience, namely that the calling application will not need to locally
 * allocate a new event structure, given that it is anticipated that they will
 * be utilized primarily on message queues.
 */

AsgardEvent *events_newEvent(int eventType)
{
	AsgardEvent *result = NULL;

	if((eventType != EVENT_TYPE_OS) &&
			(eventType != EVENT_TYPE_KEYBOARD) &&
			(eventType != EVENT_TYPE_MOUSE)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	result = (AsgardEvent *)malloc(sizeof(AsgardEvent));

	result->eventType = eventType;
	result->timestamp = time_getTimeMus();

	switch(result->eventType) {
		case EVENT_TYPE_OS:
			result->event = newOsEvent();
			break;
		case EVENT_TYPE_KEYBOARD:
			result->event = newKeyboardEvent();
			break;
		case EVENT_TYPE_MOUSE:
			result->event = newMouseEvent();
			break;
	}

	return result;
}

int events_freeEvent(AsgardEvent *event)
{
	if(event == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(event->event != NULL) {
		free(event->event);
	}

	free(event);

	return 0;
}

