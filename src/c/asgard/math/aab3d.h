/*
 * aab3d.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * A mathematical representation of a 3D axis-aligned box, header file.
 *
 * Written by Josh English.
 */

#if !defined(_ASGARD_MATH_AAB3D_H)

#define _ASGARD_MATH_AAB3D_H

#if !defined(_ASGARD_H) && !defined(_ASGARD_MATH_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _ASGARD_H || _ASGARD_MATH_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// define 3d axis aligned box data types

typedef struct _AxisAlignedBox {
	double x;
	double y;
	double z;
	Vector3d corner;
} AxisAlignedBox;


// define 3d axis aligned box public functions

void aab3d_initSet(AxisAlignedBox *aab, double x, double y, double z,
		Vector3d *corner);

void aab3d_getVertexP(Vector3d *result, AxisAlignedBox *aab, Vector3d *normal);

void aab3d_getVertexN(Vector3d *result, AxisAlignedBox *aab, Vector3d *normal);


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _ASGARD_MATH_AAB3D_H

