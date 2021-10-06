/*
 * plane3d.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * A mathematical representation of a 3D geometric plane.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#include "asgard/asgard.h"
#define _ASGARD_MATH_COMPONENT
#include "asgard/math/plane3d.h"


// define 3d plane public functions

void plane3d_init(Plane3d *plane)
{
	if(plane == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	plane->d = 0.0;
	vector3d_init(&plane->normal);
	vector3d_init(&plane->point);
}

void plane3d_setByVectors(Plane3d *plane, Vector3d *one, Vector3d *two,
		Vector3d *three)
{
	Vector3d temp_one;
	Vector3d temp_two;
	Vector3d temp_three;

	if((plane == NULL) || (one == NULL) || (two == NULL) || (three == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	vector3d_subtract(&temp_one, one, two);
	vector3d_subtract(&temp_two, three, two);
	vector3d_crossProduct(&temp_three, &temp_two, &temp_one);

	vector3d_initSetByVector(&plane->normal, &temp_three);
	vector3d_normalize(&plane->normal);

	vector3d_initSetByVector(&plane->point, two);

	plane->d = -(vector3d_dotProduct(&plane->normal, &plane->point));
}

void plane3d_setByNormalAndPoint(Plane3d *plane, Vector3d *normal,
		Vector3d *point)
{
	if((plane == NULL) || (normal == NULL) || (point == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	vector3d_initSetByVector(&plane->normal, normal);

	vector3d_initSetByVector(&plane->point, point);

	plane->d = -(vector3d_dotProduct(&plane->normal, &plane->point));
}

void plane3d_setCoefficients(Plane3d *plane, double a, double b, double c,
		double d)
{
	double length = 0.0;

	if(plane == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	vector3d_initSet(&plane->normal, a, b, c);

	length = vector3d_length(&plane->normal);

	vector3d_initSet(&plane->normal, (a / length), (b / length), (c / length));

	plane->d = (d / length);
}

double plane3d_getDistance(Plane3d *plane, Vector3d *point)
{
	if((plane == NULL) || (point == NULL)) {
		DISPLAY_INVALID_ARGS;
		return 0.0;
	}

	return (plane->d + vector3d_dotProduct(&plane->normal, point));
}

