/*
 * integer.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * A simple API for managing an optimized array of 32-bit integers, header file.
 *
 * Written by Josh English.
 */

#if !defined(_CORE_LIBRARY_TYPES_INTEGER_H)

#define _CORE_LIBRARY_TYPES_INTEGER_H

#if !defined(_CORE_H) && !defined(_CORE_LIBRARY_H) && \
		!defined(_CORE_LIBRARY_TYPES_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _CORE_H || _CORE_LIBRARY_H || _CORE_LIBRARY_TYPES_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// define integer public constants

#define INTEGER_ALLOCATION_BLOCK_SIZE				8


// define integer public data types

typedef struct _IntegerArray {
	int length;
	int allocated;
	int *array;
} IntegerArray;


// declare integer public functions

void integer_init(IntegerArray *integer);

IntegerArray *integer_new();

void integer_free(IntegerArray *integer);

void integer_freePtr(IntegerArray *integer);

aboolean integer_isSane(IntegerArray *integer);

int integer_contains(IntegerArray *integer, int value);

int integer_get(IntegerArray *integer, int position);

int integer_put(IntegerArray *integer, int value);

int integer_putArray(IntegerArray *integer, int *array, int length);

int integer_remove(IntegerArray *integer, int value);

int integer_binarySearch(int *array, int start, int length, int item);


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _CORE_LIBRARY_TYPES_INTEGER_H

