/*
 * cone3d.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The 3D cone shape, header file.
 *
 * Written by Josh English.
 */

#if !defined(_ASGARD_ENGINE_SHAPES_CONE3D_H)

#define _ASGARD_ENGINE_SHAPES_CONE3D_H

#if !defined(_ASGARD_H) && !defined(_ASGARD_ENGINE_SHAPES_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _ASGARD_H || _ASGARD_ENGINE_SHAPES_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// define cone public constants


// define cone public data types

typedef struct _Cone3d {
	aboolean isListed;
	aboolean hasColor;
	aboolean isMulticolor;
	int glId;
	int sliceCount;
	int stackCount;
	int renderMode;
	double base;
	double height;
	double *stackSinList;
	double *stackCosList;
	Point3d center;
	Color colors[3];
} Cone3d;


// declare cone public functions

Cone3d *cone3d_new(int sliceCount, int stackCount, double base, double height,
		double centerX, double centerY, double centerZ);

Cone3d *cone3d_load(Log *log, char *assetFilename);

void cone3d_free(Cone3d *cone);

void cone3d_setColor(Cone3d *cone, Color *color);

void cone3d_setMultiColor(Cone3d *cone, Color *colorOne, Color *colorTwo,
		Color *colorThree);

void cone3d_render3d(Cone3d *cone, int renderMode);


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _ASGARD_ENGINE_SHAPES_CONE3D_H

