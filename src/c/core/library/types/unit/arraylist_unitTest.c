/*
 * arraylist.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * A generic array-list implementation modeled after Java's ArrayList.
 *
 * Written by Josh English.
 */

// preprocessor directives

#define _CORE_COMMON_BYPASS_STD_STREAM
#include "core/core.h"


// define arraylist unit test constants

#define UNIT_TEST_ONE_LENGTH							16384


// declare arraylist unit test functions

static char *generateRandomString(int seedLength, aboolean useExactLength);

static int arraylist_basicFunctionalityTest();


// main function

int main()
{
	signal_registerDefault();

	printf("[unit] array list unit test (Asgard Ver %s on %s)\n",
			ASGARD_VERSION, ASGARD_DATE);

	if(arraylist_basicFunctionalityTest() < 0) {
		printf("[unit]\t ...FAILED\n");
		return 1;
	}

	printf("[unit] ...PASSED\n");

	return 0;
}


// define arraylist unit test functions

static char *generateRandomString(int seedLength, aboolean useExactLength)
{
	int ii = 0;
	int pick = 0;
	int length = 0;
	char *result = NULL;

	if((rand() % 100) > 49) {
		system_pickRandomSeed();
	}

	if(useExactLength) {
		length = seedLength;
	}
	else {
		length = ((rand() % seedLength) + 1);
	}

	result = (char *)malloc(sizeof(char) * (length + 1));

	for(ii = 0; ii < length; ii++) {
		pick = (int)(rand() % 26);

		if(pick < 0) {
			pick *= -1;
		}

		pick += 97;

		result[ii] = (char)pick;
	}

	return result;
}

static int arraylist_basicFunctionalityTest()
{
	int ii = 0;
	int nn = 0;
	int length = 0;
	int position = 0;
	char *result = NULL;
	char **array = NULL;
	char **compareArray = NULL;

	ArrayList list;

	printf("[unit]\t arraylist unit test #1...\n");

	system_pickRandomSeed();

	array = (char **)malloc(sizeof(char *) * UNIT_TEST_ONE_LENGTH);
	memset(array, 0, (sizeof(char *) * UNIT_TEST_ONE_LENGTH));

	arraylist_init(&list);

	arraylist_ensureCurrentCapacity(&list, (UNIT_TEST_ONE_LENGTH / 4));

	for(ii = 0; ii < UNIT_TEST_ONE_LENGTH; ii++) {
		array[ii] = generateRandomString(32, afalse);

		if(arraylist_add(&list, array[ii]) < 0) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		if((result = (char *)arraylist_get(&list, ii)) == NULL) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		if(result != array[ii]) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}
	}

	for(ii = 0; ii < UNIT_TEST_ONE_LENGTH; ii++) {
		if(arraylist_del(&list, array[ii]) < 0) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}
	}

	arraylist_trimCurrentCapacity(&list, UNIT_TEST_ONE_LENGTH);

	arraylist_clear(&list);

	memset(array, 0, (sizeof(char *) * UNIT_TEST_ONE_LENGTH));

	for(ii = 0; ii < UNIT_TEST_ONE_LENGTH; ii++) {
		array[ii] = generateRandomString(32, afalse);

		if((rand() % 100) > 49) {
			if(arraylist_pushFilo(&list, array[ii]) < 0) {
				fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
						__FUNCTION__, __LINE__);
				return -1;
			}
		}
		else {
			if(arraylist_addFirst(&list, array[ii]) < 0) {
				fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
						__FUNCTION__, __LINE__);
				return -1;
			}
		}
	}

	for(ii = 0; ii < UNIT_TEST_ONE_LENGTH; ii++) {
		if((position = arraylist_positionOfEntry(&list, array[ii])) < 0) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		if((result = (char *)arraylist_get(&list, position)) == NULL) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		if(result != array[ii]) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		if(!arraylist_containsEntry(&list, array[ii])) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}
	}

	arraylist_clear(&list);

	arraylist_ensureCurrentCapacity(&list, (UNIT_TEST_ONE_LENGTH + 1));

	memset(array, 0, (sizeof(char *) * UNIT_TEST_ONE_LENGTH));

	for(ii = 0; ii < UNIT_TEST_ONE_LENGTH; ii++) {
		array[ii] = generateRandomString(32, afalse);

		if(arraylist_addPosition(&list, ii, array[ii]) < 0) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		if((result = (char *)arraylist_get(&list, ii)) == NULL) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		if(result != array[ii]) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		array[ii] = generateRandomString(32, afalse);

		if(arraylist_set(&list, ii, array[ii]) < 0) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		if((result = (char *)arraylist_get(&list, ii)) == NULL) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		if(result != array[ii]) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}
	}

	if((compareArray = (char **)arraylist_array(&list, &length)) == NULL) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(length != UNIT_TEST_ONE_LENGTH) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	for(ii = 0; ii < length; ii++) {
		if(compareArray[ii] != array[ii]) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}
	}

	if(arraylist_delRange(&list, 0, UNIT_TEST_ONE_LENGTH) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	arraylist_clear(&list);

	memset(array, 0, (sizeof(char *) * UNIT_TEST_ONE_LENGTH));

	for(ii = 0; ii < UNIT_TEST_ONE_LENGTH; ii++) {
		array[ii] = generateRandomString(32, afalse);

		if(arraylist_pushFilo(&list, array[ii]) < 0) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}
	}

	for(ii = 0; ii < UNIT_TEST_ONE_LENGTH; ii++) {
		result = arraylist_pop(&list);
		if(result == NULL) {
			fprintf(stderr, "[%s():%i] error - failed at %i, aborting.\n",
					__FUNCTION__, __LINE__, ii);
			return -1;
		}

		if(result != array[ii]) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		free(result);
	}

	arraylist_clear(&list);

	for(ii = 0; ii < UNIT_TEST_ONE_LENGTH; ii++) {
		array[((UNIT_TEST_ONE_LENGTH - 1) - ii)] =
			generateRandomString(32, afalse);

		if(arraylist_pushFifo(&list,
					array[((UNIT_TEST_ONE_LENGTH - 1) - ii)]) < 0) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}
	}

	for(ii = 0; ii < UNIT_TEST_ONE_LENGTH; ii++) {
		result = arraylist_pop(&list);
		if(result == NULL) {
			fprintf(stderr, "[%s():%i] error - failed at %i, aborting.\n",
					__FUNCTION__, __LINE__, ii);
			return -1;
		}

		if(result != array[ii]) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		free(result);
	}

	arraylist_clear(&list);

	for(ii = 0, nn = 0; ii < UNIT_TEST_ONE_LENGTH; ii++) {
		array[ii] = generateRandomString(32, afalse);

		if(arraylist_add(&list, array[ii]) < 0) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		result = (char *)arraylist_get(&list, nn);
		if(result == NULL) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		if(result != array[ii]) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		if((rand() % 100) > 49) {
			if((rand() % 100) > 49) {
				if(arraylist_del(&list, array[ii]) < 0) {
					fprintf(stderr, "[%s():%i] error - failed here, "
							"aborting.\n", __FUNCTION__, __LINE__);
					return -1;
				}
			}
			else {
				if(arraylist_delPosition(&list, nn) < 0) {
					fprintf(stderr, "[%s():%i] error - failed here, "
							"aborting.\n", __FUNCTION__, __LINE__);
					return -1;
				}
			}

			if(arraylist_get(&list, nn) == array[ii]) {
				fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
						__FUNCTION__, __LINE__);
				return -1;
			}

			array[ii] = NULL;
		}
		else {
			nn++;
		}
	}

	for(ii = 0, nn = 0; ii < UNIT_TEST_ONE_LENGTH; ii++) {
		if(array[ii] == NULL) {
			continue;
		}

		result = (char *)arraylist_get(&list, nn);
		if(result == NULL) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		if(result != array[ii]) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		nn++;
	}

	arraylist_clear(&list);

	for(ii = 0; ii < UNIT_TEST_ONE_LENGTH; ii++) {
		array[ii] = generateRandomString(32, afalse);

		if(arraylist_add(&list, array[ii]) < 0) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}
	}

	for(ii = 0; ii < UNIT_TEST_ONE_LENGTH; ii++) {
		if((result = (char *)arraylist_popLast(&list)) == NULL) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		if(result != array[((UNIT_TEST_ONE_LENGTH - 1) - ii)]) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}
	}

	for(ii = 0; ii < UNIT_TEST_ONE_LENGTH; ii++) {
		if(array[ii] != NULL) {
			free(array[ii]);
		}
	}

	arraylist_free(&list);

	free(array);

	printf("[unit]\t ...PASSED\n");

	return 0;
}

