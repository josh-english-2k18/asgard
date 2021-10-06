/*
 * draw3d.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard Game Engine 3D rendering system, header file.
 *
 * Written by Josh English.
 */

#if !defined(_ASGARD_ENGINE_DRAW3D_H)

#define _ASGARD_ENGINE_DRAW3D_H

#if !defined(_ASGARD_H) && !defined(_ASGARD_ENGINE_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _ASGARD_H || _ASGARD_ENGINE_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

// define texture public constants

typedef enum _Draw3dObjectType {
	DRAW3D_TYPE_SHAPE_SPHERE = 1,
	DRAW3D_TYPE_SHAPE_CONE,
	DRAW3D_TYPE_UNKNOWN = -1
} Draw3dObjectType;


// define draw3d public data types

typedef struct _Draw3dSphere {
	Sphere3d *sphere;
} Draw3dSphere;

typedef struct _Draw3dCone {
	Cone3d *cone;
} Draw3dCone;

typedef struct _Draw3dObject {
	int type;
	int renderMode;
	void *payload;
	Point3d location;
	Vertex3d rotation;
} Draw3dObject;


// declare draw3d public functions

// rendering functions

void draw3d_sphere(Point3d *location, Vertex3d *rotation, Sphere3d *sphere,
		int renderMode);

void draw3d_cone(Point3d *location, Vertex3d *rotation, Cone3d *cone,
		int renderMode);

void draw3d_frustum(Point3d *location, Vertex3d *rotation, Frustum3d *frustum,
		int renderMode);

void draw3d_draw3dObject(Draw3dObject *object);

// object functions

Draw3dObject *draw3d_newObjectSphere(Point3d *location, Vertex3d *rotation,
		int sliceCount, int stackCount, double radius, double centerX,
		double centerY, double centerZ, int renderMode);

Draw3dObject *draw3d_newObjectSphereFromObject(Point3d *location,
		Vertex3d *rotation, Sphere3d *sphere);

Draw3dObject *draw3d_newObjectCone(Point3d *location, Vertex3d *rotation,
		int sliceCount, int stackCount, double base, double height,
		double centerX, double centerY, double centerZ, int renderMode);

Draw3dObject *draw3d_newObjectConeFromObject(Point3d *location,
		Vertex3d *rotation, Cone3d *cone);

int draw3d_freeObject(Draw3dObject *object);


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _ASGARD_ENGINE_DRAW3D_H

