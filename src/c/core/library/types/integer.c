/*
 * integer.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * A simple API for managing an optimized array of 32-bit integers.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#define _CORE_LIBRARY_TYPES_COMPONENT
#include "core/library/types/integer.h"


// define integer private constants

#define INTEGER_ARRAY_SEARCH_THRESHOLD						32

//#define INTEGER_VERBOSE_MODE								atrue
#define INTEGER_VERBOSE_MODE								afalse


// declare integer private functions

static int iSearch(int *array, int start, int length, int item);

static int findArrayPosition(int *array, int length, int item);


// define integer private functions

static int iSearch(int *array, int start, int length, int item)
{
	int minBinary = 0;
	int maxBinary = 0;
	int searchLocation = 0;

	if((array == NULL) || (length < 0)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	minBinary = start;
	maxBinary = (length - 1);

	if(minBinary == maxBinary) {
		if(array[minBinary] == item) {
			return minBinary;
		}
		return -1;
	}

	while(minBinary < maxBinary) {
		if((maxBinary - minBinary) <= INTEGER_ARRAY_SEARCH_THRESHOLD) {
			searchLocation = minBinary;

			while((searchLocation <= maxBinary) &&
					(array[searchLocation] < item)) {
				if(array[searchLocation] == item) {
					return searchLocation;
				}
				searchLocation++;
			}

			if((searchLocation < length) && (array[searchLocation] == item)) {
				return searchLocation;
			}

			break;
		}

		searchLocation = ((minBinary + maxBinary) / 2);

		if(array[searchLocation] < item) {
			minBinary = searchLocation;
		}
		else if(array[searchLocation] > item) {
			maxBinary = searchLocation;
		}
		else {
			return searchLocation;
		}
	}

	return -1;
}

static int findArrayPosition(int *array, int length, int item)
{
	int minBinary = 0;
	int maxBinary = 0;
	int searchLocation = 0;

	if((array == NULL) || (length < 0)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	minBinary = 0;
	maxBinary = (length - 1);

	if(minBinary == maxBinary) {
		if(array[minBinary] < item) {
			return length;
		}
		else if(array[minBinary] > item) {
			return minBinary;
		}
		return -1;
	}

	while(minBinary < maxBinary) {
		if((maxBinary - minBinary) <= INTEGER_ARRAY_SEARCH_THRESHOLD) {
			searchLocation = minBinary;

			while((searchLocation <= maxBinary) &&
					(array[searchLocation] < item)) {
				if(array[searchLocation] == item) {
					return -1;
				}
				searchLocation++;
				if((searchLocation < length) &&
						(array[searchLocation] > item)) {
					return searchLocation;
				}
			}

			if((searchLocation < length) && (array[searchLocation] == item)) {
				return -1;
			}

			break;
		}

		searchLocation = ((minBinary + maxBinary) / 2);

		if(array[searchLocation] < item) {
			minBinary = searchLocation;
		}
		else if(array[searchLocation] > item) {
			maxBinary = searchLocation;
		}
		else {
			return -1;
		}
	}

	return searchLocation;
}


// define integer public functions

void integer_init(IntegerArray *integer)
{
	if(integer == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	memset(integer, 0, (int)(sizeof(IntegerArray)));

	integer->length = 0;
	integer->allocated = INTEGER_ALLOCATION_BLOCK_SIZE;
	integer->array = (int *)malloc(sizeof(int) * integer->allocated);
}

IntegerArray *integer_new()
{
	IntegerArray *result = NULL;

	result = (IntegerArray *)malloc(sizeof(IntegerArray));

	integer_init(result);

	return result;
}

void integer_free(IntegerArray *integer)
{
	if(integer == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	if(integer->array != NULL) {
		free(integer->array);
	}

	memset(integer, 0, (int)(sizeof(IntegerArray)));
}

void integer_freePtr(IntegerArray *integer)
{
	if(integer == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	integer_free(integer);
	free(integer);
}

aboolean integer_isSane(IntegerArray *integer)
{
	int ii = 0;

	if(integer == NULL) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	if(integer->length < 0) {
		fprintf(stderr, "[%s():%i] error - found length %i is invalid.\n",
				__FUNCTION__, __LINE__, integer->length);
		return afalse;
	}

	if(integer->allocated < 0) {
		fprintf(stderr, "[%s():%i] error - found allocated length %i is "
				"invalid.\n", __FUNCTION__, __LINE__, integer->allocated);
		return afalse;
	}

	if(integer->length >= integer->allocated) {
		fprintf(stderr, "[%s():%i] error - found length %i >= allocated length "
				"%i.\n", __FUNCTION__, __LINE__, integer->length,
				integer->allocated);
		return afalse;
	}

	if((integer->allocated - integer->length) > INTEGER_ALLOCATION_BLOCK_SIZE) {
		fprintf(stderr, "[%s():%i] error - found available block size %i > "
				"allocation block size %i.\n", __FUNCTION__, __LINE__,
				(integer->allocated - integer->length),
				INTEGER_ALLOCATION_BLOCK_SIZE);
		return afalse;
	}

	if(integer->array == NULL) {
		fprintf(stderr, "[%s():%i] error - found array is NULL.\n",
				__FUNCTION__, __LINE__);
		return afalse;
	}

	for(ii = 0; ii < (integer->length - 1); ii++) {
		if(integer->array[ii] >= integer->array[(ii + 1)]) {
			fprintf(stderr, "[%s():%i] error - found array element %i (%i) is "
					">= element %i (%i).\n", __FUNCTION__, __LINE__, ii,
					integer->array[ii], (ii + 1), integer->array[(ii + 1)]);
			return afalse;
		}
	}

	return atrue;
}

int integer_contains(IntegerArray *integer, int value)
{
	int result = 0;

	if(integer == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	result = iSearch(integer->array, 0, integer->length, value);

	return result;
}

int integer_get(IntegerArray *integer, int position)
{
	if((integer == NULL) || (position < 0) || (position >= integer->length)) {
		DISPLAY_INVALID_ARGS;
		return 0;
	}

	return integer->array[position];
}

int integer_put(IntegerArray *integer, int value)
{
	int position = 0;
	int copyLength = 0;
	void *destination = NULL;
	void *source = NULL;

	if(integer == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	position = findArrayPosition(integer->array, integer->length, value);
	if(position < 0) {
		if(INTEGER_VERBOSE_MODE) {
			fprintf(stderr, "[%s():%i] error - unable to locate valid "
					"position for %i in 0x%lx->0x%lx, length %i, "
					"allocated %i.\n", __FUNCTION__, __LINE__, value,
					(aptrcast)integer, (aptrcast)integer->array,
					integer->length, integer->allocated);
		}
		return -1;
	}

	integer->length += 1;
	if(integer->length >= integer->allocated) {
		integer->allocated += INTEGER_ALLOCATION_BLOCK_SIZE;
		integer->array = (int *)realloc(integer->array,
				(sizeof(int) * integer->allocated));
	}

	if(integer->length > 1) {
		source = (void *)((aptrcast *)integer->array +
				(position * sizeof(int)));
		destination = (void *)((aptrcast *)source + sizeof(int));
		copyLength = ((integer->length - position) - 1);

		if(copyLength > 0) {
			memmove(destination, source, (copyLength * sizeof(int)));
		}
	}

	integer->array[position] = value;

	return position;
}

int integer_putArray(IntegerArray *integer, int *array, int length)
{
	int ii = 0;

	if((integer == NULL) || (array == NULL) || (length < 1)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	for(ii = 0; ii < length; ii++) {
		integer_put(integer, array[ii]);
	}

	return 0;
}

int integer_remove(IntegerArray *integer, int value)
{
	int position = 0;
	int copyLength = 0;
	void *destination = NULL;
	void *source = NULL;

	if(integer == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	position = iSearch(integer->array, 0, integer->length, value);
	if(position < 0) {
		if(INTEGER_VERBOSE_MODE) {
			fprintf(stderr, "[%s():%i] error - failed to locate position for "
					"%i in 0x%lx->0x%lx, length %i, allocated %i.\n",
					__FUNCTION__, __LINE__, value, (aptrcast)integer,
					(aptrcast)integer->array, integer->length,
					integer->allocated);
		}
		return -1;
	}

	destination = (void *)((aptrcast *)integer->array +
			(position * sizeof(int)));
	source = (void *)((aptrcast *)destination + sizeof(int));
	copyLength = (integer->length -
			(int)(((aptrcast *)source -
					((aptrcast *)integer->array)) / sizeof(int)));

	if(copyLength > 0) {
		memmove(destination, source, (copyLength * sizeof(int)));
	}

	integer->length -= 1;
	if((integer->length > 0) &&
			((integer->allocated - integer->length) >
			 INTEGER_ALLOCATION_BLOCK_SIZE)) {
		integer->allocated -= INTEGER_ALLOCATION_BLOCK_SIZE;
		integer->array = (int *)realloc(integer->array,
				(sizeof(int) * integer->allocated));
	}

	return 0;
}

int integer_binarySearch(int *array, int start, int length, int item)
{
	if((array == NULL) || (start < 0) || (length < 0) || (start > length)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return iSearch(array, start, length, item);
}

