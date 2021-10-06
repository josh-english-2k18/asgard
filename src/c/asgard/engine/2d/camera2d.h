/*
 * camera2d.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The 2D camera system, header file.
 *
 * Written by Josh English.
 */

#if !defined(_ASGARD_ENGINE_2D_CAMERA2D_H)

#define _ASGARD_ENGINE_2D_CAMERA2D_H

#if !defined(_ASGARD_H) && !defined(_ASGARD_ENGINE_2D_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _ASGARD_H || _ASGARD_ENGINE_2D_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// define 2d camera public constants

typedef enum _Camera2dDirection {
	CAMERA2D_DIRECTION_NORTH = 1,
	CAMERA2D_DIRECTION_NORTH_EAST,
	CAMERA2D_DIRECTION_EAST,
	CAMERA2D_DIRECTION_SOUTH_EAST,
	CAMERA2D_DIRECTION_SOUTH,
	CAMERA2D_DIRECTION_SOUTH_WEST,
	CAMERA2D_DIRECTION_WEST,
	CAMERA2D_DIRECTION_NORTH_WEST,
	CAMERA2D_DIRECTION_ERROR = -1
} Camera2dDirection;

typedef enum _Camera2dMode {
	CAMERA2D_MODE_NORMAL = 1,
	CAMERA2D_MODE_INVERTED,
	CAMERA2D_MODE_ERROR = -1
} Camera2dMode;


// define 2d camera public data types

typedef struct _Camera2d {
	aboolean isDragging;
	aboolean hasMoved;
	int xMousePosition;
	int yMousePosition;
	char *name;
	Position2d position;
	Position2d windowPosition;
} Camera2d;


// declare 2d camera public functions

void camera2d_init(Camera2d *camera, char *name, int windowX, int windowY,
		int windowWidth, int windowHeight);

Camera2d *camera2d_new(char *name, int windowX, int windowY, int windowWidth,
		int windowHeight);

Camera2d *camera2d_load(Log *log, char *assetFilename);

void camera2d_free(Camera2d *camera);

Position2d *camera2d_getPosition(Camera2d *camera);

void camera2d_setPosition(Camera2d *camera, int x, int y);

int camera2d_getX(Camera2d *camera);

int camera2d_getY(Camera2d *camera);

int camera2d_getWidth(Camera2d *camera);

int camera2d_getHeight(Camera2d *camera);

void camera2d_resize(Camera2d *camera, int width, int height);

Position2d *camera2d_getWindowPosition(Camera2d *camera);

void camera2d_setWindowPosition(Camera2d *camera, int x, int y);

int camera2d_getWindowX(Camera2d *camera);

int camera2d_getWindowY(Camera2d *camera);

int camera2d_getWindowWidth(Camera2d *camera);

int camera2d_getWindowHeight(Camera2d *camera);

void camera2d_resizeWindow(Camera2d *camera, int width, int height);

aboolean camera2d_hasMoved(Camera2d *camera);

int camera2d_getOffsetX(Camera2d *camera);

int camera2d_getOffsetY(Camera2d *camera);

void camera2d_setCameraCoords(Camera2d *camera, int x, int y);

void camera2d_applyCameraDiff(Camera2d *camera, int diffX, int diffY);

void camera2d_moveCamera(Camera2d *camera, int direction, int distance);

void camera2d_processMouseEvent(Camera2d *camera, Camera2dMode mode,
		AsgardMouseEvent *event);


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _ASGARD_ENGINE_2D_CAMERA2D_H

