/*
 * sphere3d.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The 3D sphere shape, header file.
 *
 * Written by Josh English.
 */

#if !defined(_ASGARD_ENGINE_SHAPES_SPHERE3D_H)

#define _ASGARD_ENGINE_SHAPES_SPHERE3D_H

#if !defined(_ASGARD_H) && !defined(_ASGARD_ENGINE_SHAPES_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _ASGARD_H || _ASGARD_ENGINE_SHAPES_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// define sphere public constants


// define sphere public data types

typedef struct _Sphere3d {
	aboolean isListed;
	aboolean hasColor;
	aboolean isMulticolor;
	int glId;
	int sliceCount;
	int stackCount;
	int renderMode;
	double radius;
	double *stackSinList;
	double *stackCosList;
	double *sliceSinList;
	double *sliceCosList;
	Point3d center;
	Color colors[3];
} Sphere3d;


// declare sphere public functions

Sphere3d *sphere3d_new(int sliceCount, int stackCount, double radius,
		double centerX, double centerY, double centerZ);

Sphere3d *sphere3d_load(Log *log, char *assetFilename);

void sphere3d_free(Sphere3d *sphere);

void sphere3d_setColor(Sphere3d *sphere, Color *color);

void sphere3d_setMultiColor(Sphere3d *sphere, Color *colorOne, Color *colorTwo,
		Color *colorThree);

void sphere3d_render3d(Sphere3d *sphere, int renderMode);


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _ASGARD_ENGINE_SHAPES_SPHERE3D_H

