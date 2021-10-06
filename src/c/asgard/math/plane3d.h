/*
 * plane3d.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * A mathematical representation of a 3D geometric plane, header file.
 *
 * Written by Josh English.
 */

#if !defined(_ASGARD_MATH_PLANE3D_H)

#define _ASGARD_MATH_PLANE3D_H

#if !defined(_ASGARD_H) && !defined(_ASGARD_MATH_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _ASGARD_H || _ASGARD_MATH_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// define 3d plane data types

typedef struct _Plane3d {
	double d;
	Vector3d normal;
	Vector3d point;
} Plane3d;


// define 3d plane public functions

void plane3d_init(Plane3d *plane);

void plane3d_setByVectors(Plane3d *plane, Vector3d *one, Vector3d *two,
		Vector3d *three);

void plane3d_setByNormalAndPoint(Plane3d *plane, Vector3d *normal,
		Vector3d *point);

void plane3d_setCoefficients(Plane3d *plane, double a, double b, double c,
		double d);

double plane3d_getDistance(Plane3d *plane, Vector3d *point);


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _ASGARD_MATH_PLANE3D_H

