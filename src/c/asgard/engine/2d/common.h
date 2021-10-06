/*
 * common.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The common header-file for the Asgard Game Engine 2d library.
 *
 * Written by Josh English.
 */

#if !defined(_ASGARD_ENGINE_2D_COMMON_H)

#define _ASGARD_ENGINE_2D_COMMON_H

#if !defined(_ASGARD_H) && !defined(_ASGARD_ENGINE_2D_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _ASGARD_H || _ASGARD_ENGINE_2D_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// define common 2d public constants

/*
 * TODO: this
 */


// define common 2d public data types

typedef struct _Position2d {
	int x;
	int y;
	int width;
	int height;
} Position2d;


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _ASGARD_ENGINE_2D_COMMON_H

