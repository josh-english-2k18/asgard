/*
 * camera.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * A quaternion-based camera system.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#include "asgard/asgard.h"
#define _ASGARD_ENGINE_COMPONENT
#include "asgard/engine/camera.h"

// define camera private functions

static void rotateCamera(Camera *camera, double x, double y, double z,
		double amount)
{
	double xDirection = 0.0;
	double yDirection = 0.0;
	double zDirection = 0.0;
	double cosAngle = 0.0;
	double sinAngle = 0.0;

	Vector3d viewAngle;

	// determine view angle

	cosAngle = cos(amount);
	sinAngle = sin(amount);

	viewAngle.x = (camera->view.x - camera->location.x);
	viewAngle.y = (camera->view.y - camera->location.y);
	viewAngle.z = (camera->view.z - camera->location.z);

	vector3d_normalize(&viewAngle);

	// determine directions based upon angle & velocity

	xDirection = ((cosAngle + ((1.0 - cosAngle) * x)) * viewAngle.x);
	xDirection += (((1.0 - cosAngle) * x * y) -
			((z * sinAngle) * viewAngle.y));
	xDirection += (((1.0 - cosAngle) * x * z) +
			((y * sinAngle) * viewAngle.z));

	yDirection = (((1.0 - cosAngle) * x * y) +
			((z * sinAngle) * viewAngle.x));
	yDirection += ((cosAngle + (1.0 - cosAngle) * y) * viewAngle.y);
	yDirection += (((1.0 - cosAngle) * y * z) -
			((x * sinAngle) * viewAngle.z));

	zDirection = ((((1.0 - cosAngle) * x * z) - (y *
					sinAngle)) * viewAngle.x);
	zDirection += (((1.0 - cosAngle) * y * z) +
			((x * sinAngle) * viewAngle.y));
	zDirection += ((cosAngle + ((1.0 - cosAngle) * z)) * viewAngle.z);

	// apply rotation

	camera->view.x = (camera->location.x + xDirection);
	camera->view.y = (camera->location.y + yDirection);
	camera->view.z = (camera->location.z + zDirection);
}


// define camera public functions

void camera_init(Camera *camera)
{
	if(camera == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	camera->mode = CAMERA_MODE_ALL_AXIS;
	camera->mouseX = -1;
	camera->mouseY = -1;
	camera->scale = CAMERA_DEFAULT_SCALE;

	camera->location.x = 0.0;
	camera->location.y = 0.0;
	camera->location.z = 0.0;

	camera->view.x = 0.0;
	camera->view.y = 0.0;
	camera->view.z = 0.0;

	camera->rotation.x = 0.0;
	camera->rotation.y = 0.0;
	camera->rotation.z = 0.0;
}

void camera_setMode(Camera *camera, int mode)
{
	if((camera == NULL) ||
			((mode != CAMERA_MODE_ALL_AXIS) &&
			 (mode != CAMERA_MODE_NO_Y_AXIS))
	  ) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	camera->mode = mode;
}

int camera_getMode(Camera *camera)
{
	if(camera == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return camera->mode;
}

void camera_setScale(Camera *camera, double scale)
{
	camera->scale = scale;
}

double camera_getScale(Camera *camera)
{
	return camera->scale;
}

void camera_setLocation(Camera *camera, double x, double y, double z)
{
	camera->location.x = x;
	camera->location.y = y;
	camera->location.z = z;
}

Vector3d *camera_getLocation(Camera *camera)
{
	return &(camera->location);
}

void camera_setView(Camera *camera, double x, double y, double z)
{
	camera->view.x = x;
	camera->view.y = y;
	camera->view.z = z;
}

Vector3d *camera_getViewAngle(Camera *camera)
{
	return &(camera->view);
}

void camera_setRotation(Camera *camera, double x, double y, double z)
{
	camera->rotation.x = x;
	camera->rotation.y = y;
	camera->rotation.z = z;
}

Vector3d *camera_getRotation(Camera *camera)
{
	return &(camera->rotation);
}

void camera_move(Camera *camera, double distance)
{
	Vector3d direction;

	// calculate direction of movement

	direction.x = (camera->view.x - camera->location.x);
	direction.y = (camera->view.y - camera->location.y);
	direction.z = (camera->view.z - camera->location.z);

	vector3d_normalize(&direction);
	vector3d_multiplyEqualsValue(&direction, distance);

	// apply movement

	camera->location.x = (camera->location.x + direction.x);
	camera->view.x = (camera->view.x + direction.x);
	if(camera->mode == CAMERA_MODE_ALL_AXIS) {
		camera->location.y = (camera->location.y + direction.y);
		camera->view.y = (camera->view.y + direction.y);
	}
	camera->location.z = (camera->location.z + direction.z);
	camera->view.z = (camera->view.z + direction.z);
}

void camera_strafe(Camera *camera, double distance)
{
	Vector3d angle;
	Vector3d direction;

	// calculate current view angle

	angle.x = (camera->view.x - camera->location.x);
	angle.y = (camera->view.y - camera->location.y);
	angle.z = (camera->view.z - camera->location.z);

	vector3d_normalize(&angle);

	// calculate direction of movement

	direction.x = ((angle.y * 0.0) - (angle.z * 1.0));
	direction.y = ((angle.z * 0.0) - (angle.x * 0.0));
	direction.z = ((angle.x * 1.0) - (angle.y * 0.0));

	vector3d_multiplyEqualsValue(&direction, distance);

	// apply movement

	camera->location.x = (camera->location.x + direction.x);
	camera->view.x = (camera->view.x + direction.x);
	if(camera->mode == CAMERA_MODE_ALL_AXIS) {
		camera->location.y = (camera->location.y + direction.y);
		camera->view.y = (camera->view.y + direction.y);
	}
	camera->location.z = (camera->location.z + direction.z);
	camera->view.z = (camera->view.z + direction.z);
}

void camera_climb(Camera *camera, double distance)
{
	camera->location.y = (camera->location.y + distance);
	camera->view.y = (camera->view.y + distance);
}

void camera_resetMouse(Camera *camera, int x, int y)
{
	camera->mouseX = x;
	camera->mouseY = y;
}

void camera_applyMouse(Camera *camera, int x, int y)
{
	double xDiff = 0.0;
	double yDiff = 0.0;
	double normal = 0.0;

	Vector3d angle;
	Vector3d direction;

	// calculate mouse diffs

	if(camera->mouseX == -1) {
		camera->mouseX = x;
	}
	if(camera->mouseY == -1) {
		camera->mouseY = y;
	}

	// check for mouse movement

	if((camera->mouseX == x) && (camera->mouseY == y)) {
		return;
	}

	// calculate x & y change

	xDiff = (((double)camera->mouseX - fabs(x)) * camera->scale);
	yDiff = (((double)camera->mouseY - fabs(y)) * camera->scale);

	camera->mouseX = x;
	camera->mouseY = y;

	if((xDiff == 0.0) && (yDiff == 0.0)) {
		return;
	}

	// calculate current view angle

	angle.x = (camera->view.x - camera->location.x);
	angle.y = (camera->view.y - camera->location.y);
	angle.z = (camera->view.z - camera->location.z);

	// calculate direction of movement

	direction.x = ((angle.y * 0.0) - (angle.z * 1.0));
	direction.y = ((angle.z * 0.0) - (angle.x * 0.0));
	direction.z = ((angle.x * 1.0) - (angle.y * 0.0));

	// calculate the normal of the view angle & apply to direction

	normal = (1.0 / vector3d_length(&angle));
	vector3d_multiplyEqualsValue(&direction, normal);

	rotateCamera(camera, direction.x, direction.y, direction.z, yDiff);
	rotateCamera(camera, 0.0, 1.0, 0.0, xDiff);
}

void camera_apply(Camera *camera)
{
	double xRotation = 0.0;
	double yRotation = 0.0;
	double zRotation = 0.0;

	GLdouble matrix[16];

	cgi_glLoadIdentity();
	cgi_gluLookAt(
			camera->location.x, camera->location.y, camera->location.z,
			camera->view.x, camera->view.y, camera->view.z,
			0.0, 1.0, 0.0);

	cgi_glGetDoublev(GL_MODELVIEW_MATRIX, matrix);

	yRotation = (asin(matrix[2]));
	if((fabs(matrix[2]) >= 1.0) || (yRotation == 0.0)) {
		xRotation = 0.0;
		yRotation = 0.0;
		zRotation = 0.0;
	}
	else {
		if(fabs(cos(yRotation)) > 0.005) {
			xRotation = atan2((-matrix[6] / cos(yRotation)),
					(matrix[10] / cos(yRotation)));
			zRotation = atan2((-matrix[1] / cos(yRotation)),
					(matrix[0] / cos(yRotation)));
		}
		else {
			xRotation = 0.0;
			zRotation = atan2(matrix[4], matrix[5]);
		}
	}

	if(math_RadiansToDegrees(xRotation) < 0.0) {
		xRotation += math_DegreesToRadians(360.0);
	}
	if(math_RadiansToDegrees(yRotation) < 0.0) {
		yRotation += math_DegreesToRadians(360.0);
	}
	if(math_RadiansToDegrees(zRotation) < 0.0) {
		zRotation += math_DegreesToRadians(360.0);
	}

	camera->rotation.x = xRotation;
	camera->rotation.y = yRotation;
	camera->rotation.z = zRotation;
}

