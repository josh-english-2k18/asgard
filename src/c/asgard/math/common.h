/*
 * common.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The common header-file for the Asgard Game Engine math library.
 *
 * Written by Josh English.
 */

#if !defined(_ASGARD_MATH_COMMON_H)

#define _ASGARD_MATH_COMMON_H

#if !defined(_ASGARD_H) && !defined(_ASGARD_MATH_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _ASGARD_H || _ASGARD_MATH_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// define common math constants

#if defined(math_PI)
#	undef math_PI
#endif // math_PI

#define math_PI								\
	3.141592653589793238462643383279502884


// define common math macros

#define math_AngleToRadiansMultiplier		\
	(math_PI / 180.0)

#define math_DegreesToRadians(degrees)		\
	((degrees) * (math_PI / 180.0))

#define math_RadiansToDegrees(radians)		\
	((radians) * (180.0 / math_PI))


// define common math data types

// points

typedef struct _Point2d {
	double x;
	double y;
} Point2d;

typedef struct _Point3d {
	double x;
	double y;
	double z;
} Point3d;

typedef struct _Point4d {
	double x;
	double y;
	double z;
	double w;
} Point4d;

// verticies

typedef struct _Vertex2d {
	double x;
	double y;
} Vertex2d;

typedef struct _Vertex3d {
	double x;
	double y;
	double z;
} Vertex3d;

typedef struct _Vertex4d {
	double x;
	double y;
	double z;
	double w;
} Vertex4d;

// normals

typedef struct _Normal3d {
	double x;
	double y;
	double z;
} Normal3d;


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _ASGARD_MATH_COMMON_H

