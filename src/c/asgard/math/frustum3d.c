/*
 * frustum3d.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * A math API for manipulating a 3D geometric-based frustum.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#include "asgard/asgard.h"
#define _ASGARD_MATH_COMPONENT
#include "asgard/math/frustum3d.h"


// define frustum public functions

void frustum3d_init(Frustum3d *frustum)
{
	int ii = 0;

	if(frustum == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	memset(frustum, 0, (sizeof(Frustum3d)));

	frustum->nearDistance = 0.0;
	frustum->farDistance = 0.0;
	frustum->ratio = 0.0;
	frustum->angle = 0.0;
	frustum->tang = 0.0;
	frustum->nearWidth = 0.0;
	frustum->nearHeight = 0.0;
	frustum->farWidth = 0.0;
	frustum->farHeight = 0.0;

	vector3d_init(&frustum->nearTopLeft);
	vector3d_init(&frustum->nearTopRight);
	vector3d_init(&frustum->nearBottomLeft);
	vector3d_init(&frustum->nearBottomRight);
	vector3d_init(&frustum->farTopLeft);
	vector3d_init(&frustum->farTopRight);
	vector3d_init(&frustum->farBottomLeft);
	vector3d_init(&frustum->farBottomRight);

	for(ii = 0; ii < 6; ii++) {
		plane3d_init(&frustum->plane[ii]);
	}
}

void frustum3d_setSceneState(Frustum3d *frustum, double angle, double ratio,
		double nearDistance, double farDistance)
{
	if(frustum == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	frustum->angle = angle;

	frustum->ratio = ratio;

	frustum->nearDistance = nearDistance;

	frustum->farDistance = farDistance;

	frustum->tang = (double)tan(angle * (math_PI / 180.0) * 0.5);

	frustum->nearHeight = (frustum->nearDistance * frustum->tang);

	frustum->nearWidth = (frustum->nearHeight * frustum->ratio);

	frustum->farHeight = (frustum->farDistance * frustum->tang);

	frustum->farWidth = (frustum->farHeight * frustum->ratio);
}

void frustum3d_setCameraState(Frustum3d *frustum, Vector3d *pp, Vector3d *ll,
		Vector3d *uu)
{
	Vector3d xx;
	Vector3d yy;
	Vector3d zz;
	Vector3d ref;
	Vector3d nearCoord;
	Vector3d farCoord;
	Vector3d tempOne;
	Vector3d tempTwo;

	if((frustum == NULL) || (pp == NULL) || (ll == NULL) || (uu == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	// normalize(z = (p - l))

	vector3d_subtract(&zz, pp, ll);
	vector3d_normalize(&zz);

	// normalize(x = (u * z))

	vector3d_crossProduct(&xx, uu, &zz);
	vector3d_normalize(&xx);

	// y = z * x

	vector3d_crossProduct(&yy, &zz, &xx);

	// nc = p - (z * near-distance);
	// fc = p - (z * far-distance);

	vector3d_initSet(&tempOne, zz.x, zz.y, zz.z);
	vector3d_multiplyEqualsValue(&tempOne, frustum->nearDistance);
	vector3d_subtract(&nearCoord, pp, &tempOne);
	vector3d_initSet(&tempOne, zz.x, zz.y, zz.z);
	vector3d_multiplyEqualsValue(&tempOne, frustum->farDistance);
	vector3d_subtract(&farCoord, pp, &tempOne);

	// ntl = nc + (y * nh) - (x * nw);

	vector3d_initSet(&tempOne, yy.x, yy.y, yy.z);
	vector3d_multiplyEqualsValue(&tempOne, frustum->nearHeight);
	vector3d_initSet(&tempTwo, xx.x, xx.y, xx.z);
	vector3d_multiplyEqualsValue(&tempTwo, frustum->nearWidth);
	vector3d_add(&ref, &nearCoord, &tempOne);
	vector3d_subtractEquals(&ref, &tempTwo);
	vector3d_initSet(&frustum->nearTopLeft, ref.x, ref.y, ref.z);

	// ntr = nc + (y * nh) + (x * nw);

	vector3d_initSet(&tempOne, yy.x, yy.y, yy.z);
	vector3d_multiplyEqualsValue(&tempOne, frustum->nearHeight);
	vector3d_initSet(&tempTwo, xx.x, xx.y, xx.z);
	vector3d_multiplyEqualsValue(&tempTwo, frustum->nearWidth);
	vector3d_add(&ref, &nearCoord, &tempOne);
	vector3d_addEquals(&ref, &tempTwo);
	vector3d_initSet(&frustum->nearTopRight, ref.x, ref.y, ref.z);

	// nbl = ((nc - (y * nh)) - (x * nw));

	vector3d_initSet(&tempOne, yy.x, yy.y, yy.z);
	vector3d_multiplyEqualsValue(&tempOne, frustum->nearHeight);
	vector3d_initSet(&tempTwo, xx.x, xx.y, xx.z);
	vector3d_multiplyEqualsValue(&tempTwo, frustum->nearWidth);
	vector3d_subtract(&ref, &nearCoord, &tempOne);
	vector3d_subtractEquals(&ref, &tempTwo);
	vector3d_initSet(&frustum->nearBottomLeft, ref.x, ref.y, ref.z);

	// nbr = nc - (y * nh) + (x * nw);

	vector3d_initSet(&tempOne, yy.x, yy.y, yy.z);
	vector3d_multiplyEqualsValue(&tempOne, frustum->nearHeight);
	vector3d_initSet(&tempTwo, xx.x, xx.y, xx.z);
	vector3d_multiplyEqualsValue(&tempTwo, frustum->nearWidth);
	vector3d_subtract(&ref, &nearCoord, &tempOne);
	vector3d_addEquals(&ref, &tempTwo);
	vector3d_initSet(&frustum->nearBottomRight, ref.x, ref.y, ref.z);

	// ftl = fc + (y * fh) - (x * fw);

	vector3d_initSet(&tempOne, yy.x, yy.y, yy.z);
	vector3d_multiplyEqualsValue(&tempOne, frustum->farHeight);
	vector3d_initSet(&tempTwo, xx.x, xx.y, xx.z);
	vector3d_multiplyEqualsValue(&tempTwo, frustum->farWidth);
	vector3d_add(&ref, &farCoord, &tempOne);
	vector3d_subtractEquals(&ref, &tempTwo);
	vector3d_initSet(&frustum->farTopLeft, ref.x, ref.y, ref.z);

	// ftr = fc + (y * fh) + (x * fw);

	vector3d_initSet(&tempOne, yy.x, yy.y, yy.z);
	vector3d_multiplyEqualsValue(&tempOne, frustum->farHeight);
	vector3d_initSet(&tempTwo, xx.x, xx.y, xx.z);
	vector3d_multiplyEqualsValue(&tempTwo, frustum->farWidth);
	vector3d_add(&ref, &farCoord, &tempOne);
	vector3d_addEquals(&ref, &tempTwo);
	vector3d_initSet(&frustum->farTopRight, ref.x, ref.y, ref.z);

	// fbl = fc - (y * fh) - (x * fw);

	vector3d_initSet(&tempOne, yy.x, yy.y, yy.z);
	vector3d_multiplyEqualsValue(&tempOne, frustum->farHeight);
	vector3d_initSet(&tempTwo, xx.x, xx.y, xx.z);
	vector3d_multiplyEqualsValue(&tempTwo, frustum->farWidth);
	vector3d_subtract(&ref, &farCoord, &tempOne);
	vector3d_subtractEquals(&ref, &tempTwo);
	vector3d_initSet(&frustum->farBottomLeft, ref.x, ref.y, ref.z);

	// fbr = fc - (y * fh) + (x * fw);

	vector3d_initSet(&tempOne, yy.x, yy.y, yy.z);
	vector3d_multiplyEqualsValue(&tempOne, frustum->farHeight);
	vector3d_initSet(&tempTwo, xx.x, xx.y, xx.z);
	vector3d_multiplyEqualsValue(&tempTwo, frustum->farWidth);
	vector3d_subtract(&ref, &farCoord, &tempOne);
	vector3d_addEquals(&ref, &tempTwo);
	vector3d_initSet(&frustum->farBottomRight, ref.x, ref.y, ref.z);

	// set frustum planes

	plane3d_setByVectors(&frustum->plane[FRUSTUM_PLANE_TOP],
			&frustum->nearTopRight,
			&frustum->nearTopLeft,
			&frustum->farTopLeft);

	plane3d_setByVectors(&frustum->plane[FRUSTUM_PLANE_BOTTOM],
			&frustum->nearBottomLeft,
			&frustum->nearBottomRight,
			&frustum->farBottomRight);

	plane3d_setByVectors(&frustum->plane[FRUSTUM_PLANE_LEFT],
			&frustum->nearTopLeft,
			&frustum->nearBottomLeft,
			&frustum->farBottomLeft);

	plane3d_setByVectors(&frustum->plane[FRUSTUM_PLANE_RIGHT],
			&frustum->nearBottomRight,
			&frustum->nearTopRight,
			&frustum->farBottomRight);

	plane3d_setByVectors(&frustum->plane[FRUSTUM_PLANE_NEAR],
			&frustum->nearTopLeft,
			&frustum->nearTopRight,
			&frustum->nearBottomRight);

	plane3d_setByVectors(&frustum->plane[FRUSTUM_PLANE_FAR],
			&frustum->farTopRight,
			&frustum->farTopLeft,
			&frustum->farBottomLeft);
}

int frustum3d_containsPoint(Frustum3d *frustum, Point3d *point)
{
	int ii = 0;
	int result = FRUSTUM_LOCATION_INSIDE;
	double distance = 0.0;

	Vector3d vector;

	if((frustum == NULL) || (point == NULL)) {
		DISPLAY_INVALID_ARGS;
		return FRUSTUM_LOCATION_UNKNOWN;
	}

	vector.x = point->x;
	vector.y = point->y;
	vector.z = point->z;

	for(ii = 0; ii < 6; ii++) {
		distance = plane3d_getDistance(&frustum->plane[ii], &vector);
		if(((ii == 0) && (distance < -2.0)) ||
				((ii > 0) && (distance < 0.0))) {
			result = FRUSTUM_LOCATION_OUTSIDE;
			break;
		}
	}

	return result;
}

int frustum3d_containsSphere(Frustum3d *frustum, Point3d *point, double radius)
{
	int ii = 0;
	int result = FRUSTUM_LOCATION_INSIDE;
	double distance = 0.0;

	Vector3d vector;

	if((frustum == NULL) || (point == NULL)) {
		DISPLAY_INVALID_ARGS;
		return FRUSTUM_LOCATION_UNKNOWN;
	}

	vector.x = point->x;
	vector.y = point->y;
	vector.z = point->z;

	for(ii = 0; ii < 6; ii++) {
		distance = plane3d_getDistance(&frustum->plane[ii], &vector);
		if(distance < -radius) {
			result = FRUSTUM_LOCATION_OUTSIDE;
			break;
		}
		else if(distance < radius) {
			result = FRUSTUM_LOCATION_INTERSECT;
		}
	}

	return result;
}

int frustum3d_containsBox(Frustum3d *frustum, AxisAlignedBox *aab)
{
	int ii = 0;
	int result = FRUSTUM_LOCATION_INSIDE;

	Vector3d vector;

	if((frustum == NULL) || (aab == NULL)) {
		DISPLAY_INVALID_ARGS;
		return FRUSTUM_LOCATION_UNKNOWN;
	}

	for(ii = 0; ii < 6; ii++) {
		aab3d_getVertexP(&vector, aab, &frustum->plane[ii].normal);
		if(plane3d_getDistance(&frustum->plane[ii], &vector) < 0.0) {
			result = FRUSTUM_LOCATION_OUTSIDE;
			break;
		}

		aab3d_getVertexN(&vector, aab, &frustum->plane[ii].normal);
		if(plane3d_getDistance(&frustum->plane[ii], &vector) < 0.0) {
			result = FRUSTUM_LOCATION_INTERSECT;
		}
	}

	return result;
}

