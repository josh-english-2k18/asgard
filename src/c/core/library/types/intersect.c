/*
 * intersect.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * A library providing an API for performing optimized 32-bit integer
 * intersections.
 *
 * Written by Josh English.
 */

// preprocessor directives

//#define _CORE_COMMON_BYPASS_STD_STREAM

#include "core/core.h"
#define _CORE_LIBRARY_TYPES_COMPONENT
#include "core/library/types/intersect.h"


// declare intersect private functions

static int ascendingIntegerComparison(const void *left, const void *right);

static void ascendingSort(IntersectIntegerBuffer *buffer);


// define intersect private functions

static int ascendingIntegerComparison(const void *left, const void *right)
{
	int iLeft = 0;
	int iRight = 0;

	if((left == NULL) || (right == NULL)) {
		return 0;
	}

	iLeft = *((int *)left);
	iRight = *((int *)right);

	return (iLeft - iRight);
}

static void ascendingSort(IntersectIntegerBuffer *buffer)
{
	if((buffer == NULL) || (buffer->isSorted) || (buffer->length < 2) ||
			(buffer->array == NULL)) {
		return;
	}

	qsort((void *)buffer->array, buffer->length, sizeof(int),
			ascendingIntegerComparison);
	buffer->isSorted = atrue;
}


// define intersect public functions

void intersect_init(Intersect *intersect)
{
	if(intersect == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	intersect->state = INTERSECT_STATE_INIT;
	intersect->length = 0;
	intersect->buffers = NULL;
	intersect->result.isSorted = afalse;
	intersect->result.length = 0;
	intersect->result.array = NULL;
}

void intersect_free(Intersect *intersect)
{
	int ii = 0;

	if(intersect == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	if(intersect->buffers != NULL) {
		for(ii = 0; ii < intersect->length; ii++) {
			if(intersect->buffers[ii].array != NULL) {
				free(intersect->buffers[ii].array);
			}
		}

		free(intersect->buffers);
	}

	if(intersect->result.array != NULL) {
		free(intersect->result.array);
	}
}

char *intersect_stateToString(IntersectStates state)
{
	char *result = NULL;

	switch(state) {
		case INTERSECT_STATE_INIT:
			result = "init";
			break;

		case INTERSECT_STATE_READY:
			result = "ready";
			break;

		case INTERSECT_STATE_DONE:
			result = "done";
			break;

		case INTERSECT_STATE_UNKNOWN:
			result = "unknown";
			break;

		default:
			result = "error";
	}

	return result;
}

void intersect_display(void *stream, Intersect *intersect)
{
	int ii = 0;
	int nn = 0;

	if(intersect == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	if(stream == NULL) {
		return;
	}

	fprintf(stream, "Intersect 0x%lx ::\n", (aptrcast)intersect);
	fprintf(stream, "\t state     : '%s'\n",
			intersect_stateToString(intersect->state));
	fprintf(stream, "\t length    : %i\n", intersect->length);

	for(ii = 0; ii < intersect->length; ii++) {
		fprintf(stream, "\t buffer    : %i\n", ii);
		fprintf(stream, "\t\t is sorted  : %i\n",
				intersect->buffers[ii].isSorted);
		fprintf(stream, "\t\t length     : %i\n",
				intersect->buffers[ii].length);

		for(nn = 0; nn < intersect->buffers[ii].length; nn++) {
			fprintf(stream, "\t\t\t %03i : %i\n",
					nn, intersect->buffers[ii].array[nn]);
		}
	}

	fprintf(stream, "\t result       ::\n");
	fprintf(stream, "\t\t is sorted  : %i\n",
			intersect->result.isSorted);
	fprintf(stream, "\t\t length     : %i\n",
			intersect->result.length);

	for(ii = 0; ii < intersect->result.length; ii++) {
		fprintf(stream, "\t\t\t %03i : %i\n",
				ii, intersect->result.array[ii]);
	}
}

int intersect_getLength(Intersect *intersect, int id)
{
	if((intersect == NULL) || (id < 0) || (id >= intersect->length)) {
		DISPLAY_INVALID_ARGS;
		return 0;
	}

	return intersect->buffers[id].length;
}

int intersect_get(Intersect *intersect, int id, int position)
{
	if((intersect == NULL) || (id < 0) || (id >= intersect->length) ||
			(position < 0) || (position >= intersect->buffers[id].length)) {
		DISPLAY_INVALID_ARGS;
		return 0;
	}

	return intersect->buffers[id].array[position];
}

int *intersect_getArray(Intersect *intersect, int id, int *length)
{
	if((intersect == NULL) || (id < 0) || (id >= intersect->length) ||
			(length == NULL)) {
		DISPLAY_INVALID_ARGS;

		if(length != NULL) {
			*length = 0;
		}

		return NULL;
	}

	*length = intersect->buffers[id].length;

	return intersect->buffers[id].array;
}

int intersect_put(Intersect *intersect, int id, int value)
{
	int ii = 0;
	int length = 0;
	int bufferId = 0;

	if((intersect == NULL) || (id < 0)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(id >= intersect->length) {
		length = intersect->length;
		intersect->length = (id + 1);

		if(intersect->buffers == NULL) {
			intersect->buffers = (IntersectIntegerBuffer *)malloc(
					sizeof(IntersectIntegerBuffer) * intersect->length);
		}
		else {
			intersect->buffers = (IntersectIntegerBuffer *)realloc(
					intersect->buffers,
					(sizeof(IntersectIntegerBuffer) * intersect->length));
		}

		for(ii = length; ii < intersect->length; ii++) {
			intersect->buffers[ii].isSorted = afalse;
			intersect->buffers[ii].length = 0;
			intersect->buffers[ii].array = NULL;
		}
	}

	if(intersect->buffers[id].array == NULL) {
		bufferId = 0;
		intersect->buffers[id].length = 1;
		intersect->buffers[id].array = (int *)malloc(sizeof(int) *
				intersect->buffers[id].length);
	}
	else {
		bufferId = intersect->buffers[id].length;
		intersect->buffers[id].length += 1;
		intersect->buffers[id].array = (int *)realloc(
				intersect->buffers[id].array,
				(sizeof(int) * intersect->buffers[id].length));
	}

	intersect->buffers[id].isSorted = afalse;
	intersect->buffers[id].array[bufferId] = value;

	return 0;
}

int intersect_putArray(Intersect *intersect, aboolean isSorted, int *array,
		int length)
{
	int id = 0;

	if((intersect == NULL) || (array == NULL) || (length < 1)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(intersect->buffers == NULL) {
		id = 0;
		intersect->length = 1;
		intersect->buffers = (IntersectIntegerBuffer *)malloc(
				sizeof(IntersectIntegerBuffer) * intersect->length);
	}
	else {
		id = intersect->length;
		intersect->length += 1;
		intersect->buffers = (IntersectIntegerBuffer *)realloc(
				intersect->buffers,
				(sizeof(IntersectIntegerBuffer) * intersect->length));
	}

	intersect->buffers[id].isSorted = isSorted;
	intersect->buffers[id].length = length;
	intersect->buffers[id].array = (int *)malloc(sizeof(int) * length);

	memcpy(intersect->buffers[id].array, array, (sizeof(int) * length));

	return 0;
}

int intersect_putArrayStatic(Intersect *intersect, aboolean isSorted,
		int *array, int length)
{
	int id = 0;

	if((intersect == NULL) || (array == NULL) || (length < 1)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(intersect->buffers == NULL) {
		id = 0;
		intersect->length = 1;
		intersect->buffers = (IntersectIntegerBuffer *)malloc(
				sizeof(IntersectIntegerBuffer) * intersect->length);
	}
	else {
		id = intersect->length;
		intersect->length += 1;
		intersect->buffers = (IntersectIntegerBuffer *)realloc(
				intersect->buffers,
				(sizeof(IntersectIntegerBuffer) * intersect->length));
	}

	intersect->buffers[id].isSorted = isSorted;
	intersect->buffers[id].length = length;
	intersect->buffers[id].array = array;

	return 0;
}

aboolean intersect_isReady(Intersect *intersect)
{
	aboolean isReady = atrue;
	int ii = 0;

	if(intersect == NULL) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	if(intersect->state == INTERSECT_STATE_READY) {
		return atrue;
	}

	if(intersect->result.array != NULL) {
		free(intersect->result.array);
	}

	intersect->result.isSorted = afalse;
	intersect->result.length = 0;
	intersect->result.array = NULL;

	for(ii = 0; ii < intersect->length; ii++) {
		if(!intersect->buffers[ii].isSorted) {
			isReady = afalse;
			break;
		}
	}

	if(!isReady) {
		intersect->state = INTERSECT_STATE_INIT;
		return afalse;
	}

	intersect->state = INTERSECT_STATE_READY;

	return atrue;
}

void intersect_setIsSorted(Intersect *intersect)
{
	int ii = 0;

	if(intersect == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	if(intersect->state != INTERSECT_STATE_INIT) {
		return;
	}

	for(ii = 0; ii < intersect->length; ii++) {
		intersect->buffers[ii].isSorted = atrue;
	}

	intersect->state = INTERSECT_STATE_READY;
}

void intersect_sort(Intersect *intersect)
{
	int ii = 0;

	if(intersect == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	if(intersect->state != INTERSECT_STATE_INIT) {
		return;
	}

	for(ii = 0; ii < intersect->length; ii++) {
		if(!intersect->buffers[ii].isSorted) {
			ascendingSort(&intersect->buffers[ii]);
			intersect->buffers[ii].isSorted = atrue;
		}
	}

	intersect->state = INTERSECT_STATE_READY;
}

void intersect_execAnd(Intersect *intersect, aboolean isCleanMode)
{
	aboolean isFound = afalse;
	int ii = 0;
	int nn = 0;
	int jj = 0;
	int ref = 0;
	int maxLength = 0;
	int *thresholds = NULL;

	if(intersect == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	// determine readiness

	if(!intersect_isReady(intersect)) {
		intersect_sort(intersect);
		if(!intersect_isReady(intersect)) {
			return;
		}
	}

	// cleanup any old results

	if(intersect->result.array != NULL) {
		free(intersect->result.array);
	}

	intersect->result.isSorted = afalse;
	intersect->result.length = 0;
	intersect->result.array = NULL;

	// determine if an intersection is even possible

	if(intersect->length < 1) {
		return;
	}

	// execute a Boolean 'and' intersection

	thresholds = (int *)malloc(sizeof(int) * intersect->length);

	for(ii = 0; ii < intersect->length; ii++) {
		thresholds[ii] = 0;

		if(intersect->buffers[ii].length > maxLength) {
			maxLength = intersect->buffers[ii].length;
		}
	}

	intersect->result.length = (maxLength + 1);
	intersect->result.array = (int *)malloc(sizeof(int) *
			intersect->result.length);

	for(ii = 0; ii < intersect->length; ii++) {
		for(nn = 0; nn < intersect->buffers[ii].length; nn++) {
			isFound = atrue;

			for(jj = 0; jj < intersect->length; jj++) {
				if(jj == ii) {
					continue;
				}

				if(intersect->buffers[jj].length <= 0) {
					isFound = afalse;
					break;
				}

				if(integer_binarySearch(intersect->buffers[jj].array,
							thresholds[jj],
							intersect->buffers[jj].length,
							intersect->buffers[ii].array[nn]) < 0) {
					isFound = afalse;
					break;
				}
				else {
					thresholds[ii] = (nn + 1);
				}
			}

			if(isFound) {
				intersect->result.array[ref] = intersect->buffers[ii].array[nn];

				if((isCleanMode) &&
						(((maxLength > 1) && (ref > 0)) ||
						 ((maxLength == 1) && (ref == 0))) &&
						(intersect->result.array[ref] == 0)) {
					break;
				}

				ref++;

				if(ref >= intersect->result.length) {
					fprintf(stderr, "[%s():%i] error - exceeded max result "
							"size %i at %i of %i, aborting.\n", __FUNCTION__,
							__LINE__, intersect->result.length, ii, nn);
					break;
				}
			}
		}
	}

	intersect->result.length = ref;
	intersect->state = INTERSECT_STATE_DONE;

	free(thresholds);
}

void intersect_execOr(Intersect *intersect)
{
	int ii = 0;
	int low = 0;
	int resultRef = 0;
	int completed = 0;
	int *index = NULL;

	if(intersect == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	// determine readiness

	if(!intersect_isReady(intersect)) {
		intersect_sort(intersect);
		if(!intersect_isReady(intersect)) {
			return;
		}
	}

	// cleanup any old results

	if(intersect->result.array != NULL) {
		free(intersect->result.array);
	}

	intersect->result.isSorted = afalse;
	intersect->result.length = 0;
	intersect->result.array = NULL;

	// determine if an intersection is even possible

	if(intersect->length < 1) {
		return;
	}

	// execute a Boolean 'or' intersection

	index = (int *)malloc(sizeof(int) * intersect->length);

	// calculate maximum length of and initialize result

	intersect->result.length = 0;

	for(ii = 0; ii < intersect->length; ii++) {
		intersect->result.length += intersect->buffers[ii].length;
	}

	intersect->result.array = (int *)malloc(
			sizeof(int) * intersect->result.length);

	// execute 'or' intersection logic

	while(completed < intersect->length) {
		// locate the lowest of the next value in the available or'd arrays

		low = MAX_SIGNED_INT;
		for(ii = 0; ii < intersect->length; ii++) {
			if(index[ii] >= intersect->buffers[ii].length) {
				continue;
			}

			if(intersect->buffers[ii].array[(index[ii])] < low) {
				low = intersect->buffers[ii].array[(index[ii])];
			}
		}

		// increment the array index for each matching low value

		for(ii = 0; ii < intersect->length; ii++) {
			if(index[ii] >= intersect->buffers[ii].length) {
				continue;
			}

			if(intersect->buffers[ii].array[(index[ii])] == low) {
				index[ii] += 1;
				if(index[ii] >= intersect->buffers[ii].length) {
					completed++;
				}
			}
		}

		// assign the result

		intersect->result.array[resultRef] = low;
		resultRef++;

		if(resultRef >= intersect->result.length) {
			if((completed + 1) < intersect->length) {
				fprintf(stderr, "[%s():%i] error - exceeded max result "
						"length %i at %i of %i, aborting.\n",
						__FUNCTION__, __LINE__,
						intersect->result.length, completed, intersect->length);
			}
			break;
		}
	}

	intersect->result.length = resultRef;
	intersect->state = INTERSECT_STATE_DONE;

	free(index);
}

void intersect_execNot(Intersect *intersect, aboolean isCleanMode)
{
	aboolean isFound = afalse;
	int ii = 0;
	int nn = 0;
	int jj = 0;
	int ref = 0;
	int maxLength = 0;

	if(intersect == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	// determine readiness

	if(!intersect_isReady(intersect)) {
		intersect_sort(intersect);
		if(!intersect_isReady(intersect)) {
			return;
		}
	}

	// cleanup any old results

	if(intersect->result.array != NULL) {
		free(intersect->result.array);
	}

	intersect->result.isSorted = afalse;
	intersect->result.length = 0;
	intersect->result.array = NULL;

	// execute a Boolean 'and' intersection

	for(ii = 0; ii < intersect->length; ii++) {
		if(intersect->buffers[ii].length > maxLength) {
			maxLength = intersect->buffers[ii].length;
		}
	}

	intersect->result.length = (maxLength + 1);
	intersect->result.array = (int *)malloc(sizeof(int) *
			intersect->result.length);

	for(ii = 0; ii < intersect->length; ii++) {
		for(nn = 0; nn < intersect->buffers[ii].length; nn++) {
			isFound = atrue;

			for(jj = 0; jj < intersect->length; jj++) {
				if(jj == ii) {
					continue;
				}

				if(intersect->buffers[jj].length <= 0) {
					isFound = afalse;
					break;
				}

				if(integer_binarySearch(intersect->buffers[jj].array,
							0,
							intersect->buffers[jj].length,
							intersect->buffers[ii].array[nn]) < 0) {
					isFound = afalse;
					break;
				}
			}

			if(!isFound) {
				intersect->result.array[ref] = intersect->buffers[ii].array[nn];

				if((isCleanMode) && (ref > 0) &&
						(intersect->result.array[ref] == 0)) {
					break;
				}

				ref++;

				if(ref >= intersect->result.length) {
					fprintf(stderr, "[%s():%i] error - exceeded max result "
							"size %i at %i of %i, aborting.\n", __FUNCTION__,
							__LINE__, intersect->result.length, ii, nn);
					break;
				}
			}
		}
	}

	intersect->result.length = ref;
	intersect->state = INTERSECT_STATE_DONE;
}

int *intersect_getResult(Intersect *intersect, int *length)
{
	if((intersect == NULL) || (length == NULL)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	*length = intersect->result.length;

	return intersect->result.array;
}

int *intersect_getResultCopy(Intersect *intersect, int *length)
{
	int *result = NULL;

	if((intersect == NULL) || (length == NULL)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	*length = 0;

	if((intersect->result.length > 0) && (intersect->result.array != NULL)) {
		result = (int *)malloc(sizeof(int) * intersect->result.length);

		memcpy(result, intersect->result.array,
				(sizeof(int) * intersect->result.length));

		*length = intersect->result.length;
	}

	return result;
}

int intersect_binarySearch(int mode, int *array, int length, int item)
{
	int result = 0;
	int *local = NULL;

	if(((mode != INTERSECT_MODE_SORTED) &&
				(mode != INTERSECT_MODE_MANIPULATE) &&
				(mode != INTERSECT_MODE_TRANSPARENT)) ||
			(array == NULL) || (length < 1)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if((mode == INTERSECT_MODE_SORTED) ||
			(mode == INTERSECT_MODE_MANIPULATE)) {
		local = array;
	}
	else {
		local = (int *)malloc(sizeof(int) * length);
		memcpy(local, array, (sizeof(int) * length));
	}

	if(mode != INTERSECT_MODE_SORTED) {
		qsort((void *)local, length, sizeof(int), ascendingIntegerComparison);
	}

	result = integer_binarySearch(local, 0, length, item);

	if(mode == INTERSECT_MODE_TRANSPARENT) {
		free(local);
	}

	return result;
}

