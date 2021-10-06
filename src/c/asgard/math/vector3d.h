/*
 * vector3d.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * A math API for manipulating 3D vectors, header file.
 *
 * Written by Josh English.
 */

#if !defined(_ASGARD_MATH_VECTOR3D_H)

#define _ASGARD_MATH_VECTOR3D_H

#if !defined(_ASGARD_H) && !defined(_ASGARD_MATH_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _ASGARD_H || _ASGARD_MATH_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// define 3d vector public data types

typedef struct _Vector3d {
	double x;
	double y;
	double z;
} Vector3d;


// declare 3d vector public functions

void vector3d_init(Vector3d *vector);

void vector3d_initSet(Vector3d *vector, double x, double y, double z);

void vector3d_initSetByVector(Vector3d *vector, Vector3d *value);

void vector3d_add(Vector3d *result, Vector3d *first, Vector3d *second);

void vector3d_addEquals(Vector3d *vector, Vector3d *value);

void vector3d_addEqualsValue(Vector3d *vector, double value);

void vector3d_subtract(Vector3d *result, Vector3d *first, Vector3d *second);

void vector3d_subtractEquals(Vector3d *vector, Vector3d *value);

void vector3d_subtractEqualsValue(Vector3d *vector, double value);

void vector3d_multiply(Vector3d *result, Vector3d *first, Vector3d *second);

void vector3d_multiplyEquals(Vector3d *vector, Vector3d *value);

void vector3d_multiplyEqualsValue(Vector3d *vector, double value);

void vector3d_divide(Vector3d *result, Vector3d *first, Vector3d *second);

void vector3d_divideEquals(Vector3d *vector, Vector3d *value);

void vector3d_divideEqualsValue(Vector3d *vector, double value);

double vector3d_sqr(Vector3d *vector, Vector3d *value);

double vector3d_distance(Vector3d *vector, Vector3d *value);

double vector3d_length(Vector3d *vector);

double vector3d_dotProduct(Vector3d *first, Vector3d *second);

void vector3d_crossProduct(Vector3d *result, Vector3d *first, Vector3d *second);

void vector3d_normalize(Vector3d *vector);

void vector3d_normalizeByDistance(Vector3d *vector, Vector3d *value);

aboolean vector3d_equals(Vector3d *first, Vector3d *second);

aboolean vector3d_notEquals(Vector3d *first, Vector3d *second);

void vector3d_rotateX(Vector3d *vector, double amount);

void vector3d_rotateY(Vector3d *vector, double amount);

void vector3d_rotateZ(Vector3d *vector, double amount);

void vector3d_getRotationX(Vector3d *result, Vector3d *vector, double amount);

void vector3d_getRotationY(Vector3d *result, Vector3d *vector, double amount);

void vector3d_getRotationZ(Vector3d *result, Vector3d *vector, double amount);


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _ASGARD_MATH_VECTOR3D_H

