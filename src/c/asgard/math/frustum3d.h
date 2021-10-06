/*
 * frustum3d.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * A math API for manipulating a 3D geometric-based frustum, header file.
 *
 * Written by Josh English.
 */

#if !defined(_ASGARD_MATH_FRUSTUM3D_H)

#define _ASGARD_MATH_FRUSTUM3D_H

#if !defined(_ASGARD_H) && !defined(_ASGARD_MATH_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _ASGARD_H || _ASGARD_MATH_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// define 3d frustum public constants

typedef enum _FrustumPlanes {
	FRUSTUM_PLANE_TOP = 0,
	FRUSTUM_PLANE_BOTTOM,
	FRUSTUM_PLANE_LEFT,
	FRUSTUM_PLANE_RIGHT,
	FRUSTUM_PLANE_NEAR,
	FRUSTUM_PLANE_FAR,
	FRUSTUM_PLANE_UNKNOWN = -1
} FrustumPlanes;

typedef enum _FrustumLocations {
	FRUSTUM_LOCATION_OUTSIDE = 1,
	FRUSTUM_LOCATION_INTERSECT,
	FRUSTUM_LOCATION_INSIDE,
	FRUSTUM_LOCATION_UNKNOWN = -1
} FrustumLocations;


// define 3d frustum public data types

typedef struct _Frustum3d {
	double nearDistance;
	double farDistance;
	double ratio;
	double angle;
	double tang;
	double nearWidth;
	double nearHeight;
	double farWidth;
	double farHeight;
	Vector3d nearTopLeft;
	Vector3d nearTopRight;
	Vector3d nearBottomLeft;
	Vector3d nearBottomRight;
	Vector3d farTopLeft;
	Vector3d farTopRight;
	Vector3d farBottomLeft;
	Vector3d farBottomRight;
	Plane3d plane[6];
} Frustum3d;


// declare 3d frustum public functions

void frustum3d_init(Frustum3d *frustum);

void frustum3d_setSceneState(Frustum3d *frustum, double angle, double ratio,
		double nearDistance, double farDistance);

void frustum3d_setCameraState(Frustum3d *frustum, Vector3d *pp, Vector3d *ll,
		Vector3d *uu);

int frustum3d_containsPoint(Frustum3d *frustum, Point3d *point);

int frustum3d_containsSphere(Frustum3d *frustum, Point3d *point, double radius);

int frustum3d_containsBox(Frustum3d *frustum, AxisAlignedBox *aab);


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _ASGARD_MATH_FRUSTUM3D_H

