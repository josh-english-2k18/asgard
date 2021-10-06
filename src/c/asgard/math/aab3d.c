/*
 * aab3d.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * A mathematical representation of a 3D axis-aligned box.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#include "asgard/asgard.h"
#define _ASGARD_MATH_COMPONENT
#include "asgard/math/aab3d.h"


// define 3d axis aligned box public functions

void aab3d_initSet(AxisAlignedBox *aab, double x, double y, double z,
		Vector3d *corner)
{
	if((aab == NULL) || (corner == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	vector3d_initSet(&aab->corner, corner->x, corner->y, corner->z);

	if(x < 0.0) {
		x *= -1.0;
		aab->corner.x -= x;
	}
	if(y < 0.0) {
		y *= -1.0;
		aab->corner.y -= y;
	}
	if(z < 0.0) {
		z *= -1.0;
		aab->corner.z -= z;
	}

	aab->x = x;
	aab->y = y;
	aab->z = z;
}

void aab3d_getVertexP(Vector3d *result, AxisAlignedBox *aab, Vector3d *normal)
{
	if((result == NULL) || (aab == NULL) || (normal == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	vector3d_initSet(result, aab->corner.x, aab->corner.y, aab->corner.z);

	if(normal->x > 0.0) {
		result->x += aab->x;
	}
	if(normal->y > 0.0) {
		result->y += aab->y;
	}
	if(normal->z > 0.0) {
		result->z += aab->z;
	}
}

void aab3d_getVertexN(Vector3d *result, AxisAlignedBox *aab, Vector3d *normal)
{
	if((result == NULL) || (aab == NULL) || (normal == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	vector3d_initSet(result, aab->corner.x, aab->corner.y, aab->corner.z);

	if(normal->x < 0.0) {
		result->x += aab->x;
	}
	if(normal->y < 0.0) {
		result->y += aab->y;
	}
	if(normal->z < 0.0) {
		result->z += aab->z;
	}
}

