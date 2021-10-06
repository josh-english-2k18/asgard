/*
 * camera2d.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The 2D camera system.
 *
 * Written by Josh English.
 */


// preprocessor directives

#include "core/core.h"
#include "asgard/asgard.h"
#define _ASGARD_ENGINE_2D_COMPONENT
#include "asgard/engine/2d/camera2d.h"


// define 2d camera public functions

void camera2d_init(Camera2d *camera, char *name, int windowX, int windowY,
		int windowWidth, int windowHeight)
{
	if(camera == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	memset(camera, 0, (sizeof(Camera2d)));

	camera->isDragging = atrue;
	camera->hasMoved = afalse;
	camera->xMousePosition = 0;
	camera->yMousePosition = 0;
	camera->name = strdup(name);
	camera->position.x = windowX;
	camera->position.y = windowY;
	camera->position.width = windowWidth;
	camera->position.height = windowHeight;
	camera->windowPosition.x = windowX;
	camera->windowPosition.y = windowY;
	camera->windowPosition.width = windowWidth;
	camera->windowPosition.height = windowHeight;
}

Camera2d *camera2d_new(char *name, int windowX, int windowY, int windowWidth,
		int windowHeight)
{
	Camera2d *result = NULL;

	result = (Camera2d *)malloc(sizeof(Camera2d));

	camera2d_init(result, name, windowX, windowY, windowWidth, windowHeight);

	return result;
}

Camera2d *camera2d_load(Log *log, char *assetFilename)
{
	if((log == NULL) || (assetFilename == NULL)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	/*
	 * TODO: this function
	 */

	return NULL;
}

void camera2d_free(Camera2d *camera)
{
	if(camera == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	/*
	 * TODO: this function
	 */

	return;
}

Position2d *camera2d_getPosition(Camera2d *camera)
{
	if(camera == NULL) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	return &(camera->position);
}

void camera2d_setPosition(Camera2d *camera, int x, int y)
{
	if(camera == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	camera->position.x = x;
	camera->position.y = y;
}

int camera2d_getX(Camera2d *camera)
{
	if(camera == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return camera->position.x;
}

int camera2d_getY(Camera2d *camera)
{
	if(camera == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return camera->position.y;
}

int camera2d_getWidth(Camera2d *camera)
{
	if(camera == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return camera->position.width;
}

int camera2d_getHeight(Camera2d *camera)
{
	if(camera == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return camera->position.height;
}

void camera2d_resize(Camera2d *camera, int width, int height)
{
	if(camera == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	camera->position.width = width;
	camera->position.height = height;
}

Position2d *camera2d_getWindowPosition(Camera2d *camera)
{
	if(camera == NULL) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	return &(camera->windowPosition);
}

void camera2d_setWindowPosition(Camera2d *camera, int x, int y)
{
	if(camera == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	camera->windowPosition.x = x;
	camera->windowPosition.y = y;
}

int camera2d_getWindowX(Camera2d *camera)
{
	if(camera == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return camera->windowPosition.x;
}

int camera2d_getWindowY(Camera2d *camera)
{
	if(camera == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return camera->windowPosition.y;
}

int camera2d_getWindowWidth(Camera2d *camera)
{
	if(camera == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return camera->windowPosition.width;
}

int camera2d_getWindowHeight(Camera2d *camera)
{
	if(camera == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return camera->windowPosition.height;
}

void camera2d_resizeWindow(Camera2d *camera, int width, int height)
{
	if(camera == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	camera->windowPosition.width = width;
	camera->windowPosition.height = height;
}

aboolean camera2d_hasMoved(Camera2d *camera)
{
	if(camera == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return camera->hasMoved;
}

int camera2d_getOffsetX(Camera2d *camera)
{
	if(camera == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return (int)(0.0 - (double)camera->position.x);
}

int camera2d_getOffsetY(Camera2d *camera)
{
	if(camera == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return (int)(0.0 - (double)camera->position.y);
}

void camera2d_setCameraCoords(Camera2d *camera, int x, int y)
{
	if(camera == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	camera->hasMoved = atrue;
	camera->position.x = x;
	camera->position.y = y;
}

void camera2d_applyCameraDiff(Camera2d *camera, int diffX, int diffY)
{
	if(camera == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	camera->hasMoved = atrue;
	camera->position.x += diffX;
	camera->position.y += diffY;
}

void camera2d_moveCamera(Camera2d *camera, int direction, int distance)
{
	if((camera == NULL) ||
			(direction < 0) || (direction > CAMERA2D_DIRECTION_NORTH_WEST) ||
			(distance < 0)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	switch(direction) {
		case CAMERA2D_DIRECTION_NORTH:
			camera->position.y -= distance;
			break;

		case CAMERA2D_DIRECTION_NORTH_EAST:
			camera->position.x += (int)((double)distance * 0.707106);
			camera->position.y -= (int)((double)distance * 0.707106);
			break;

		case CAMERA2D_DIRECTION_EAST:
			camera->position.x += distance;
			break;

		case CAMERA2D_DIRECTION_SOUTH_EAST:
			camera->position.x += (int)((double)distance * 0.707106);
			camera->position.y += (int)((double)distance * 0.707106);
			break;

		case CAMERA2D_DIRECTION_SOUTH:
			camera->position.y += distance;
			break;

		case CAMERA2D_DIRECTION_SOUTH_WEST:
			camera->position.x -= (int)((double)distance * 0.707106);
			camera->position.y += (int)((double)distance * 0.707106);
			break;

		case CAMERA2D_DIRECTION_WEST:
			camera->position.x -= distance;
			break;

		case CAMERA2D_DIRECTION_NORTH_WEST:
			camera->position.x -= (int)((double)distance * 0.707106);
			camera->position.y -= (int)((double)distance * 0.707106);
			break;
	}

	camera->hasMoved = atrue;
}

void camera2d_processMouseEvent(Camera2d *camera, Camera2dMode mode,
		AsgardMouseEvent *event)
{
	if((camera == NULL) ||
			((mode != CAMERA2D_MODE_NORMAL) &&
			 (mode != CAMERA2D_MODE_INVERTED)) ||
			(event == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	if((event->xPosition < camera->windowPosition.x) ||
			(event->xPosition > camera->windowPosition.width) ||
			(event->yPosition < camera->windowPosition.y) ||
			(event->yPosition > camera->windowPosition.height)) {
		return;
	}

	if(event->eventType == EVENT_MOUSE_DRAGGED) {
		if(camera->isDragging) {
			if(event->xPosition < camera->xMousePosition) {
				if(mode == CAMERA2D_MODE_NORMAL) {
					camera->position.x += (camera->xMousePosition -
							event->xPosition);
				}
				else {
					camera->position.x -= (camera->xMousePosition -
							event->xPosition);
				}
				camera->hasMoved = atrue;
			}
			else if(event->xPosition > camera->xMousePosition) {
				if(mode == CAMERA2D_MODE_NORMAL) {
					camera->position.x -= (event->xPosition -
							camera->xMousePosition);
				}
				else {
					camera->position.x += (event->xPosition -
							camera->xMousePosition);
				}
				camera->hasMoved = atrue;
			}
			if(event->yPosition < camera->yMousePosition) {
				if(mode == CAMERA2D_MODE_NORMAL) {
					camera->position.y += (camera->yMousePosition -
							event->yPosition);
				}
				else {
					camera->position.y -= (camera->yMousePosition -
							event->yPosition);
				}
				camera->hasMoved = atrue;
			}
			else if(event->yPosition > camera->yMousePosition) {
				if(mode == CAMERA2D_MODE_NORMAL) {
					camera->position.y -= (event->yPosition -
							camera->yMousePosition);
				}
				else {
					camera->position.y += (event->yPosition -
							camera->yMousePosition);
				}
				camera->hasMoved = atrue;
			}
		}
		else {
			camera->isDragging = atrue;
		}
	}
	else {
		camera->isDragging = afalse;
	}

	camera->xMousePosition = event->xPosition;
	camera->yMousePosition = event->yPosition;
}

