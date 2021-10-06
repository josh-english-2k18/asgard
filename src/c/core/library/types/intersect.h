/*
 * intersect.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * A library providing an API for performing optimized 32-bit integer
 * intersections, header file.
 *
 * Written by Josh English.
 */

#if !defined(_CORE_LIBRARY_TYPES_INTERSECT_H)

#define _CORE_LIBRARY_TYPES_INTERSECT_H

#if !defined(_CORE_H) && !defined(_CORE_LIBRARY_H) && \
		!defined(_CORE_LIBRARY_TYPES_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _CORE_H || _CORE_LIBRARY_H || _CORE_LIBRARY_TYPES_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// define intersect public constants

typedef enum _IntersectStates {
	INTERSECT_STATE_INIT = 1,
	INTERSECT_STATE_READY,
	INTERSECT_STATE_DONE,
	INTERSECT_STATE_UNKNOWN = -1
} IntersectStates;

typedef enum _IntersectModes {
	INTERSECT_MODE_SORTED = 1,
	INTERSECT_MODE_MANIPULATE,
	INTERSECT_MODE_TRANSPARENT,
	INTERSECT_MODE_UNKNOWN = -1
} IntersectModes;


// define intersect public data types

typedef struct _IntersectIntegerBuffer {
	aboolean isSorted;
	int length;
	int *array;
} IntersectIntegerBuffer;

typedef struct _Intersect {
	int state;
	int length;
	IntersectIntegerBuffer *buffers;
	IntersectIntegerBuffer result;
} Intersect;


// declare intersect public functions

void intersect_init(Intersect *intersect);

void intersect_free(Intersect *intersect);

char *intersect_stateToString(IntersectStates state);

void intersect_display(void *stream, Intersect *intersect);

int intersect_getLength(Intersect *intersect, int id);

int intersect_get(Intersect *intersect, int id, int position);

int *intersect_getArray(Intersect *intersect, int id, int *length);

int intersect_put(Intersect *intersect, int id, int value);

int intersect_putArray(Intersect *intersect, aboolean isSorted, int *array,
		int length);

int intersect_putArrayStatic(Intersect *intersect, aboolean isSorted,
		int *array, int length);

aboolean intersect_isReady(Intersect *intersect);

void intersect_setIsSorted(Intersect *intersect);

void intersect_sort(Intersect *intersect);

void intersect_execAnd(Intersect *intersect, aboolean isCleanMode);

void intersect_execOr(Intersect *intersect);

void intersect_execNot(Intersect *intersect, aboolean isCleanMode);

int *intersect_getResult(Intersect *intersect, int *length);

int *intersect_getResultCopy(Intersect *intersect, int *length);

int intersect_binarySearch(int mode, int *array, int length, int item);


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _CORE_LIBRARY_TYPES_INTERSECT_H

