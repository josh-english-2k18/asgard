/*
 * intersect_unitTest.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * A library providing an API for performing optimized 32-bit integer
 * intersections, unit test.
 *
 * Written by Josh English.
 */

// preprocessor directives

#define _CORE_COMMON_BYPASS_STD_STREAM
#include "core/core.h"


// define intersect unit test constants

#define UNIT_TEST_ONE_LENGTH							8

#define UNIT_TEST_TWO_LENGTH							16

#define RANDOM_ARRAY_LENGTH								16384

#define ENTRY_SEED										131072


// declare intersect unit test private functions

// general functions

static int *createRandomArray(aboolean mode, int size, int seed,
		int *resultLength);

static int ascendingIntegerComparison(const void *left, const void *right);

// unit test functions

static int intersect_basicUnitTest(aboolean mode);

static int intersect_binarySearchUnitTest();

static int intersect_binarySearchSanityTest();

static int intersect_strictUnitTest();


// main function

int main()
{
	signal_registerDefault();

	printf("[unit] Intersect Unit Test, using Ver %s on %s.\n",
			ASGARD_VERSION, ASGARD_DATE);

	if(intersect_basicUnitTest(afalse) < 0) {
		printf("[unit]\t ...FAILED\n");
		return 1;
	}

	if(intersect_basicUnitTest(atrue) < 0) {
		printf("[unit]\t ...FAILED\n");
		return 1;
	}

	if(intersect_binarySearchUnitTest() < 0) {
		printf("[unit]\t ...FAILED\n");
		return 1;
	}

	if(intersect_binarySearchSanityTest() < 0) {
		printf("[unit]\t ...FAILED\n");
		return 1;
	}

	if(intersect_strictUnitTest() < 0) {
		printf("[unit]\t ...FAILED\n");
		return 1;
	}

	printf("[unit] ...PASSED\n");

	return 0;
}


// define intersect unit test private functions

// general functions

static int *createRandomArray(aboolean mode, int size, int seed,
		int *resultLength)
{
	aboolean isFound = afalse;
	int ii = 0;
	int nn = 0;
	int length = 0;
	int pick = 0;
	int *result = NULL;

	if(!mode) {
		system_pickRandomSeed();
	}

	length = ((rand() % size) + (rand() % size) + 128);

	result = (int *)malloc(sizeof(int) * length);

	for(ii = 0; ii < length; ii++) {
		do {
			isFound = afalse;

			if(!mode) {
				pick = (rand() % seed);
			}
			else {
				pick = (rand() % (length * 2));
			}

			for(nn = 0; nn < ii; nn++) {
				if(result[nn] == pick) {
					isFound = atrue;
					break;
				}
			}
		} while(isFound);

		result[ii] = pick;
	}

	*resultLength = length;

	return result;
}

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

// unit test functions

static int intersect_basicUnitTest(aboolean mode)
{
	int ii = 0;
	int nn = 0;
	int value = 0;
	int length = 0;
	int totalElements = 0;
	int *array = NULL;
	double timer = 0.0;
	char *modeName = NULL;

	Intersect intersect;

	if(mode) {
		modeName = "boolean-and";
	}
	else {
		modeName = "boolean-or";
	}

	printf("[unit]\t intersect basic unit test [%s]...\n", modeName);

	timer = time_getTimeMus();

	intersect_init(&intersect);

	for(ii = 0; ii < UNIT_TEST_ONE_LENGTH; ii++) {
		array = createRandomArray(mode, RANDOM_ARRAY_LENGTH, ENTRY_SEED,
				&length);

		if(intersect_putArray(&intersect, afalse, array, length) < 0) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		value = intersect_getLength(&intersect, ii);
		if(value != length) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		for(nn = 0; nn < length; nn++) {
			value = intersect_get(&intersect, ii, nn);

			if(value != array[nn]) {
				fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
						__FUNCTION__, __LINE__);
				return -1;
			}
		}

		totalElements += length;

		free(array);

		if((array = intersect_getArray(&intersect, ii, &length)) == NULL) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		for(nn = 0; nn < length; nn++) {
			value = intersect_get(&intersect, ii, nn);

			if(value != array[nn]) {
				fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
						__FUNCTION__, __LINE__);
				return -1;
			}
		}
	}

	printf("[unit]\t\t constructed %i arrays of %i elements in %0.6f seconds\n",
			UNIT_TEST_ONE_LENGTH, totalElements,
			time_getElapsedMusInSeconds(timer));

	timer = time_getTimeMus();

	intersect_sort(&intersect);

	printf("[unit]\t\t sorted all array elements in %0.6f seconds\n",
			time_getElapsedMusInSeconds(timer));

	if(!intersect_isReady(&intersect)) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	timer = time_getTimeMus();

	if(mode) {
		intersect_execAnd(&intersect, atrue);
	}
	else {
		intersect_execOr(&intersect);
	}

	printf("[unit]\t\t executed '%s' intersection in %0.6f seconds\n",
			modeName, time_getElapsedMusInSeconds(timer));
	printf("[unit]\t\t obtained %i results\n", intersect.result.length);

	intersect_free(&intersect);

	printf("[unit]\t ...PASSED\n");

	return 0;
}

static int intersect_binarySearchUnitTest()
{
	aboolean mode = afalse;
	int ii = 0;
	int nn = 0;
	int pick = 0;
	int type = 0;
	int length = 0;
	int result = 0;
	int *array = NULL;
	double timer = 0.0;
	double linearTime = 0.0;
	double sortedBinaryTime = 0.0;
	double binaryTime = 0.0;

	printf("[unit]\t intersect binary-search unit test...\n");

	for(ii = 0; ii < UNIT_TEST_TWO_LENGTH; ii++) {
		array = createRandomArray(afalse, RANDOM_ARRAY_LENGTH, ENTRY_SEED,
				&length);

		if((rand() % 100) > 49) {
			pick = array[(rand() % length)];
			mode = atrue;
		}
		else {
			do {
				result = 0;
				pick = rand();
				for(nn = 0; nn < length; nn++) {
					if(array[nn] == pick) {
						result = 1;
						break;
					}
				}
			} while(result);
			mode = afalse;
		}

		if((rand() % 100) > 65) {
			type = INTERSECT_MODE_SORTED;
			qsort((void *)array, length, (int)sizeof(int),
					ascendingIntegerComparison);
		}
		else if((rand() % 100) > 65) {
			type = INTERSECT_MODE_MANIPULATE;
		}
		else {
			type = INTERSECT_MODE_TRANSPARENT;
		}

		timer = time_getTimeMus();

		result = -1;

		for(nn = 0; nn < length; nn++) {
			if(array[nn] == pick) {
				result = 1;
				break;
			}
		}

		linearTime += time_getElapsedMusInSeconds(timer);

		if((mode) && (result < 0)) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}
		else if((!mode) && (result >= 0)) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		timer = time_getTimeMus();

		result = intersect_binarySearch(type, array, length, pick);

		if(type == INTERSECT_MODE_SORTED) {
			sortedBinaryTime += time_getElapsedMusInSeconds(timer);
		}
		else {
			binaryTime += time_getElapsedMusInSeconds(timer);
		}

		if((mode) && (result < 0)) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}
		else if((!mode) && (result >= 0)) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		free(array);
	}

	printf("[unit]\t\t linear time         : %0.6f\n", linearTime);
	printf("[unit]\t\t binary(sorted) time : %0.6f\n", sortedBinaryTime);
	printf("[unit]\t\t binary(std) time    : %0.6f\n", binaryTime);
	printf("[unit]\t ...PASSED\n");

	return 0;
}

static int intersect_binarySearchSanityTest()
{
	int result = 0;
	int array[1] = { 5 };

	printf("[unit]\t intersect binary-search sanity-test unit test...\n");

	result = intersect_binarySearch(INTERSECT_MODE_SORTED, array, 1, 5);
	if(result != 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	result = intersect_binarySearch(INTERSECT_MODE_SORTED, array, 1, 0);
	if(result != -1) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	result = intersect_binarySearch(INTERSECT_MODE_MANIPULATE, array, 1, 5);
	if(result != 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	result = intersect_binarySearch(INTERSECT_MODE_MANIPULATE, array, 1, 0);
	if(result != -1) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	result = intersect_binarySearch(INTERSECT_MODE_TRANSPARENT, array, 1, 5);
	if(result != 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	result = intersect_binarySearch(INTERSECT_MODE_TRANSPARENT, array, 1, 0);
	if(result != -1) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	printf("[unit]\t ...PASSED\n");

	return 0;
}

static int intersect_strictUnitTest()
{
	int ii = 0;
	int nn = 0;
	int arrayOne[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
	int arrayTwo[5] = { 1, 3, 5, 7, 9 };
	double timer = 0.0;

	Intersect intersect;

	printf("[unit]\t intersect strict unit test...\n");

	intersect_init(&intersect);

	if(intersect_putArray(&intersect, atrue, arrayOne, 10) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(intersect_putArray(&intersect, atrue, arrayTwo, 5) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	timer = time_getTimeMus();

	intersect_execAnd(&intersect, atrue);

	printf("[unit]\t\t boolean-and intersect time: %0.6f seconds\n",
			time_getElapsedMusInSeconds(timer));

	for(ii = 0, nn = 1; ii < intersect.result.length; ii++, nn += 2) {
		if(intersect.result.array[ii] != arrayOne[nn]) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}
	}

	timer = time_getTimeMus();

	intersect_execOr(&intersect);

	printf("[unit]\t\t boolean-or intersect time : %0.6f seconds\n",
			time_getElapsedMusInSeconds(timer));

	for(ii = 0, nn = 0; ii < intersect.result.length; ii++, nn++) {
		if(intersect.result.array[ii] != arrayOne[nn]) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}
	}

	timer = time_getTimeMus();

	intersect_execNot(&intersect, atrue);

	printf("[unit]\t\t boolean-not intersect time: %0.6f seconds\n",
			time_getElapsedMusInSeconds(timer));

	for(ii = 0, nn = 0; ii < intersect.result.length; ii++, nn += 2) {
		if(intersect.result.array[ii] != arrayOne[nn]) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}
	}

	intersect_free(&intersect);

	printf("[unit]\t ...PASSED\n");

	return 0;
}

