/*
 * camera.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * A quaternion-based camera system, header file.
 *
 * Written by Josh English.
 */

#if !defined(_ASGARD_ENGINE_CAMERA_H)

#define _ASGARD_ENGINE_CAMERA_H

#if !defined(_ASGARD_H) && !defined(_ASGARD_ENGINE_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _ASGARD_H || _ASGARD_ENGINE_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// define camera public constants

#define CAMERA_MODE_ALL_AXIS						1
#define CAMERA_MODE_NO_Y_AXIS						2

#define CAMERA_DEFAULT_SCALE						0.001


// define camera public data types

typedef struct _Camera {
	int mode;
	int mouseX;
	int mouseY;
	double scale;
	Vector3d location;
	Vector3d view;
	Vector3d rotation;
} Camera;


// declare camera public functions

void camera_init(Camera *camera);

void camera_setMode(Camera *camera, int mode);

int camera_getMode(Camera *camera);

void camera_setScale(Camera *camera, double scale);

double camera_getScale(Camera *camera);

void camera_setLocation(Camera *camera, double x, double y, double z);

Vector3d *camera_getLocation(Camera *camera);

void camera_setView(Camera *camera, double x, double y, double z);

Vector3d *camera_getViewAngle(Camera *camera);

void camera_setRotation(Camera *camera, double x, double y, double z);

Vector3d *camera_getRotation(Camera *camera);

void camera_move(Camera *camera, double distance);

void camera_strafe(Camera *camera, double distance);

void camera_climb(Camera *camera, double distance);

void camera_resetMouse(Camera *camera, int x, int y);

void camera_applyMouse(Camera *camera, int x, int y);

void camera_apply(Camera *camera);


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _ASGARD_ENGINE_CAMERA_H

