/*
 * vector.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * An math API for representing and manipulating vectors in 3D.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#include "asgard/asgard.h"
#define _ASGARD_MATH_COMPONENT
#include "asgard/math/vector3d.h"


// define 3d vector public functions

void vector3d_init(Vector3d *vector)
{
	if(vector == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	vector->x = 0.0;
	vector->y = 0.0;
	vector->z = 0.0;
}

void vector3d_initSet(Vector3d *vector, double x, double y, double z)
{
	if(vector == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	vector->x = x;
	vector->y = y;
	vector->z = z;
}

void vector3d_initSetByVector(Vector3d *vector, Vector3d *value)
{
	if((vector == NULL) || (value == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	vector->x = value->x;
	vector->y = value->y;
	vector->z = value->z;
}

void vector3d_add(Vector3d *result, Vector3d *first, Vector3d *second)
{
	if((result == NULL) || (first == NULL) || (second == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	result->x = (first->x + second->x);
	result->y = (first->y + second->y);
	result->z = (first->z + second->z);
}

void vector3d_addEquals(Vector3d *vector, Vector3d *value)
{
	if((vector == NULL) || (value == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	vector3d_add(vector, vector, value);
}

void vector3d_addEqualsValue(Vector3d *vector, double value)
{
	if(vector == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	vector->x += value;
	vector->y += value;
	vector->z += value;
}

void vector3d_subtract(Vector3d *result, Vector3d *first, Vector3d *second)
{
	if((result == NULL) || (first == NULL) || (second == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	result->x = (first->x - second->x);
	result->y = (first->y - second->y);
	result->z = (first->z - second->z);
}

void vector3d_subtractEquals(Vector3d *vector, Vector3d *value)
{
	if((vector == NULL) || (value == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	vector3d_subtract(vector, vector, value);
}

void vector3d_subtractEqualsValue(Vector3d *vector, double value)
{
	if(vector == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	vector->x -= value;
	vector->y -= value;
	vector->z -= value;
}

void vector3d_multiply(Vector3d *result, Vector3d *first, Vector3d *second)
{
	if((result == NULL) || (first == NULL) || (second == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	result->x = (first->x * second->x);
	result->y = (first->y * second->y);
	result->z = (first->z * second->z);
}

void vector3d_multiplyEquals(Vector3d *vector, Vector3d *value)
{
	if((vector == NULL) || (value == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	vector3d_multiply(vector, vector, value);
}

void vector3d_multiplyEqualsValue(Vector3d *vector, double value)
{
	if(vector == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	vector->x *= value;
	vector->y *= value;
	vector->z *= value;
}

void vector3d_divide(Vector3d *result, Vector3d *first, Vector3d *second)
{
	if((result == NULL) || (first == NULL) || (second == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	if(second->x != 0.0) {
		result->x = (first->x / second->x);
	}
	if(second->y != 0.0) {
		result->y = (first->y / second->y);
	}
	if(second->z != 0.0) {
		result->z = (first->z / second->z);
	}
}

void vector3d_divideEquals(Vector3d *vector, Vector3d *value)
{
	if((vector == NULL) || (value == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	vector3d_divide(vector, vector, value);
}

void vector3d_divideEqualsValue(Vector3d *vector, double value)
{
	if(vector == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	if(value != 0.0) {
		vector->x /= value;
		vector->y /= value;
		vector->z /= value;
	}
}

double vector3d_sqr(Vector3d *vector, Vector3d *value)
{
	double result = 0.0;

	if((vector == NULL) || (value == NULL)) {
		DISPLAY_INVALID_ARGS;
		return 0.0;
	}

	result = (
			((value->x - vector->x) * (value->x - vector->x)) +
			((value->y - vector->y) * (value->y - vector->y)) +
			((value->z - vector->z) * (value->z - vector->z))
			);

	return result;
}

double vector3d_distance(Vector3d *vector, Vector3d *value)
{
	double result = 0.0;

	if((vector == NULL) || (value == NULL)) {
		DISPLAY_INVALID_ARGS;
		return 0.0;
	}

	result = vector3d_sqr(vector, value);

	return sqrt(result);
}

double vector3d_length(Vector3d *vector)
{
	double result = 0.0;

	if(vector == NULL) {
		DISPLAY_INVALID_ARGS;
		return 0.0;
	}

	result = sqrt(vector3d_dotProduct(vector, vector));

	return result;
}

double vector3d_dotProduct(Vector3d *first, Vector3d *second)
{
	double result = 0.0;

	if((first == NULL) || (second == NULL)) {
		DISPLAY_INVALID_ARGS;
		return 0.0;
	}

	result = (
			(first->x * second->x) +
			(first->y * second->y) +
			(first->z * second->z)
			);

	return result;
}

void vector3d_crossProduct(Vector3d *result, Vector3d *first, Vector3d *second)
{
	if((result == NULL) || (first == NULL) || (second == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	result->x = ((first->y * second->z) - (first->z * second->y));
	result->y = ((first->z * second->x) - (first->x * second->z));
	result->z = ((first->x * second->y) - (first->y * second->x));
}

void vector3d_normalize(Vector3d *vector)
{
	double length = 0.0;

	if(vector == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	length = vector3d_length(vector);

	if(length != 0.0) {
		vector3d_divideEqualsValue(vector, length);
	}
}

void vector3d_normalizeByDistance(Vector3d *vector, Vector3d *value)
{
	double distance = 0.0;

	if((vector == NULL) || (value == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	distance = vector3d_distance(vector, value);

	vector3d_divideEqualsValue(vector, distance);
}

aboolean vector3d_equals(Vector3d *first, Vector3d *second)
{
	if((first == NULL) || (second == NULL)) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	if((first->x != second->x) || (first->y != second->y) ||
			(first->z != second->z)) {
		return afalse;
	}

	return atrue;
}

aboolean vector3d_notEquals(Vector3d *first, Vector3d *second)
{
	if((first == NULL) || (second == NULL)) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	if((first->x != second->x) || (first->y != second->y) ||
			(first->z != second->z)) {
		return atrue;
	}

	return afalse;
}

void vector3d_rotateX(Vector3d *vector, double amount)
{
	double x = 0.0;
	double y = 0.0;
	double z = 0.0;

	if(vector == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	x = vector->x;
	y = ((vector->y * cos(amount)) + (vector->z * sin(amount)));
	z = ((vector->z * cos(amount)) - (vector->y * sin(amount)));

	vector3d_initSet(vector, x, y, z);
}

void vector3d_rotateY(Vector3d *vector, double amount)
{
	double x = 0.0;
	double y = 0.0;
	double z = 0.0;

	if(vector == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	x = ((vector->x * cos(amount)) + (vector->z * sin(amount)));
	y = vector->y;
	z = ((vector->z * cos(amount)) - (vector->z * sin(amount)));

	vector3d_initSet(vector, x, y, z);
}

void vector3d_rotateZ(Vector3d *vector, double amount)
{
	double x = 0.0;
	double y = 0.0;
	double z = 0.0;

	if(vector == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	x = ((vector->x * cos(amount)) + (vector->y * sin(amount)));
	y = ((vector->y * cos(amount)) - (vector->x * sin(amount)));
	z = vector->z;

	vector3d_initSet(vector, x, y, z);
}

void vector3d_getRotationX(Vector3d *result, Vector3d *vector, double amount)
{
	if((result == NULL) || (vector == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	vector3d_initSetByVector(result, vector);
	vector3d_rotateX(result, amount);
}

void vector3d_getRotationY(Vector3d *result, Vector3d *vector, double amount)
{
	if((result == NULL) || (vector == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	vector3d_initSetByVector(result, vector);
	vector3d_rotateY(result, amount);
}

void vector3d_getRotationZ(Vector3d *result, Vector3d *vector, double amount)
{
	if((result == NULL) || (vector == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	vector3d_initSetByVector(result, vector);
	vector3d_rotateZ(result, amount);
}

