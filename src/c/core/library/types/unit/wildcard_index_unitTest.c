/*
 * wildcard_index_unitTest.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * A library to provide an API for building and searching against wildcard
 * indexes from input strings, unit test.
 *
 * Written by Josh English.
 */

// preprocessor directives

#define _CORE_COMMON_BYPASS_STD_STREAM
#include "core/core.h"


// define wildcard index unit test private constants

#define MIN_WILDCARD_SIZE								3
#define MAX_WILDCARD_SIZE								128

#define BASIC_UNIT_TEST_LENGTH							4096

#define STRESS_TEST_LENGTH								512


// delcare wildcard index unit test private functions

// helper functions

static char *generate_random_string();

// unit test functions

static int wildcardIndex_basicUnitTest();

static int wildcardIndex_strictUnitTest();

static int wildcardIndex_stressTest();


// main function

int main()
{
	signal_registerDefault();

	printf("[unit] Wildcard unit test, using Ver %s on %s.\n",
			ASGARD_VERSION, ASGARD_DATE);

	if(wildcardIndex_basicUnitTest() < 0) {
		printf("[unit]\t ...FAILED\n");
		return 1;
	}

	if(wildcardIndex_strictUnitTest() < 0) {
		printf("[unit]\t ...FAILED\n");
		return 1;
	}

	if(wildcardIndex_stressTest() < 0) {
		printf("[unit]\t ...FAILED\n");
		return 1;
	}

	printf("[unit] ...PASSED\n");

	return 0;
}


// define wildcard index unit test private functions

// helper functions

static char *generate_random_string()
{
	int ii = 0;
	int pick = 0;
	int length = 0;
	int counter = 0;
	char *result = NULL;

	system_pickRandomSeed();

	length = ((rand() % 32) + (rand() % 32) + 5);
	result = (char *)malloc(sizeof(char) * (length + 8));

	for(ii = 0; ii < length; ii++) {
		if((rand() % 100) > 90) {
			result[ii] = (char)((rand() % 255) + 1);
		}
		else if((rand() % 100) > 90) {
			result[ii] = ' ';
		}
		else {
			pick = (rand() % 46);
			if((pick >= 0) && (pick <= 19)) {
				if(pick > 9) {
					pick -= 10;
				}
				pick += 48;
			}
			else {
				pick += 77;
			}
			result[ii] = (char)pick;
			counter++;
		}
	}

	if(counter < MIN_WILDCARD_SIZE) {
		for(ii = length; ii < (length + MIN_WILDCARD_SIZE); ii++) {
			pick = (rand() % 46);
			if((pick >= 0) && (pick <= 19)) {
				if(pick > 9) {
					pick -= 10;
				}
				pick += 48;
			}
			else {
				pick += 77;
			}
			result[ii] = (char)pick;
		}
	}

	return result;
}

// unit test functions

static int wildcardIndex_basicUnitTest()
{
	int ii = 0;
	int nn = 0;
	int uid = 0;
	int length = 0;
	int removeCounter = 0;
	int *result = NULL;
	double timer = 0.0;
	double totalPutTime = 0.0;
	double totalGetTime = 0.0;
	double totalRemoveTime = 0.0;
	char *string = NULL;
	char value[128];

	WildcardIndex wildcard;

	printf("[unit]\t wildcard basic unit test...\n");

	if(wildcardIndex_init(&wildcard,
				MIN_WILDCARD_SIZE,
				MAX_WILDCARD_SIZE) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	printf("[unit]\t\t building initial indexes...\n");

	for(ii = 0; ii < BASIC_UNIT_TEST_LENGTH; ii++) {
		uid = ii;
		snprintf(value, sizeof(value), "test%06i", ii);

		timer = time_getTimeMus();

		if(wildcardIndex_put(&wildcard, WILDCARD_INDEX_TYPE_STANDARD,
					value, uid) < 0) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		totalPutTime += time_getElapsedMusInSeconds(timer);

		uid = (BASIC_UNIT_TEST_LENGTH + ii);
		string = generate_random_string();

		timer = time_getTimeMus();

		if(wildcardIndex_put(&wildcard, WILDCARD_INDEX_TYPE_STANDARD,
					string, uid) < 0) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		totalPutTime += time_getElapsedMusInSeconds(timer);

		if((rand() % 100) > 49) {
			timer = time_getTimeMus();

			if(wildcardIndex_remove(&wildcard, string, uid) < 0) {
				fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
						__FUNCTION__, __LINE__);
				return -1;
			}

			totalRemoveTime += time_getElapsedMusInSeconds(timer);
			removeCounter++;
		}

		free(string);
	}

	printf("[unit]\t\t wildcard ticks               : %lli\n",
			wildcardIndex_getTicks(&wildcard));
	printf("[unit]\t\t wildcard indexes             : %i\n",
			wildcardIndex_getIndexCount(&wildcard));
	printf("[unit]\t\t wildcard depth               : %i\n",
			wildcardIndex_getIndexDepth(&wildcard));
	printf("[unit]\t\t total wildcard put time      : %0.6f (%0.6f avg)\n",
			totalPutTime,
			(totalPutTime / (double)(BASIC_UNIT_TEST_LENGTH * 2)));
	printf("[unit]\t\t total wildcard remove time   : %0.6f (%0.6f avg)\n",
			totalRemoveTime,
			(totalRemoveTime / (double)removeCounter));
	printf("[unit]\t\t checking index integrity...\n");

	for(ii = 0; ii < BASIC_UNIT_TEST_LENGTH; ii++) {
		if((rand() % 100) > 49) {
			strcpy(value, "test");
		}
		else {
			snprintf(value, 100, "test%06i", ii);
		}

		timer = time_getTimeMus();

		result = wildcardIndex_get(&wildcard, value, &length);

		totalGetTime += time_getElapsedMusInSeconds(timer);

		if((result == NULL) || (length < 1)) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		if(strlen(value) == 4) {
			if(length != BASIC_UNIT_TEST_LENGTH) {
				fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
						__FUNCTION__, __LINE__);
				return -1;
			}

			for(nn = 0; nn < length; nn++) {
				if(result[nn] != nn) {
					fprintf(stderr, "[%s():%i] error - failed here, "
							"aborting.\n", __FUNCTION__, __LINE__);
					return -1;
				}
			}
		}

		free(result);
	}

	printf("[unit]\t\t total wildcard get time      : %0.6f (%0.6f avg)\n",
			totalGetTime,
			(totalGetTime / (double)BASIC_UNIT_TEST_LENGTH));

	if(wildcardIndex_free(&wildcard) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	printf("[unit]\t ...PASSED\n");

	return 0;
}

static int wildcardIndex_strictUnitTest()
{
	int length = 0;
	int *result = NULL;

	WildcardIndex wildcard;

	printf("[unit]\t wildcard strict unit test...\n");

	if(wildcardIndex_init(&wildcard, 3, 15) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(wildcardIndex_put(&wildcard, WILDCARD_INDEX_TYPE_STANDARD,
				"porcupine", 0) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(wildcardIndex_getIndexCount(&wildcard) != 13) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(wildcardIndex_put(&wildcard, WILDCARD_INDEX_TYPE_STANDARD,
				"pinetree", 1) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(wildcardIndex_put(&wildcard, WILDCARD_INDEX_TYPE_STANDARD,
			"porridge", 2) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(wildcardIndex_put(&wildcard, WILDCARD_INDEX_TYPE_EXACT,
			"pineridge", 3) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	result = wildcardIndex_get(&wildcard, "porridge", &length);
	if((result == NULL) || (length != 1) || (result[0] != 2)) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	free(result);

	result = wildcardIndex_get(&wildcard, "pine", &length);
	if((result == NULL) || (length != 2) || (result[0] != 0) ||
			(result[1] != 1)) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	free(result);

	result = wildcardIndex_get(&wildcard, "por", &length);
	if((result == NULL) || (length != 2) || (result[0] != 0) ||
			(result[1] != 2)) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	free(result);

	result = wildcardIndex_get(&wildcard, "pineridge", &length);
	if((result == NULL) || (length != 1) || (result[0] != 3)) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	free(result);

	if(wildcardIndex_remove(&wildcard, "pinetree", 1) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	result = wildcardIndex_get(&wildcard, "pinetree", &length);
	if(result != NULL) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	result = wildcardIndex_get(&wildcard, "pine", &length);
	if((result == NULL) || (length != 1) || (result[0] != 0)) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	free(result);

	result = wildcardIndex_get(&wildcard, "por", &length);
	if((result == NULL) || (length != 2) || (result[0] != 0) ||
			(result[1] != 2)) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	free(result);

	if(wildcardIndex_remove(&wildcard, "pineridge", 3) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	result = wildcardIndex_get(&wildcard, "pineridge", &length);
	if(result != NULL) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(wildcardIndex_remove(&wildcard, "porridge", 2) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	result = wildcardIndex_get(&wildcard, "ridge", &length);
	if(result != NULL) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(wildcardIndex_getIndexCount(&wildcard) != 13) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(wildcardIndex_put(&wildcard, WILDCARD_INDEX_TYPE_EXACT,
			"This is a test.", 1234) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	result = wildcardIndex_get(&wildcard, "This is a test.", &length);
	if((result == NULL) || (length != 1) || (result[0] != 1234)) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	free(result);

	// cleanup

	if(wildcardIndex_free(&wildcard) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	printf("[unit]\t ...PASSED\n");

	return 0;
}

static int wildcardIndex_stressTest()
{
	int ii = 0;
	int jj = 0;
	int kk = 0;
	int min = 0;
	int max = 0;
	int length = 0;
	int bufferLength = 0;
	int *result = NULL;
	char **strings = NULL;
	char buffer[1024];

	WildcardIndex wildcard;

	printf("[unit]\t wildcard stress test...\n");

	if(wildcardIndex_init(&wildcard,
				MIN_WILDCARD_SIZE,
				MAX_WILDCARD_SIZE) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	printf("[unit]\t\t building random strings...\n");

	strings = (char **)malloc(sizeof(char *) * STRESS_TEST_LENGTH);

	for(ii = 0; ii < STRESS_TEST_LENGTH; ii++) {
		strings[ii] = generate_random_string();
	}

	printf("[unit]\t\t executing stress test...");
	fflush(stdout);

	bufferLength = snprintf(buffer, ((int)sizeof(buffer) - 1),
			"%0.2f %%    ", 0.0);

	printf("%s", buffer);
	fflush(stdout);

	for(jj = 0; jj < STRESS_TEST_LENGTH; jj++) {
		if((jj > 0) && ((jj % 8) == 0)) {
			for(ii = 0; ii < bufferLength; ii++) {
				printf("\b");
			}

			bufferLength = snprintf(buffer, ((int)sizeof(buffer) - 1),
					"%0.2f %%    ",
					(((double)jj / (double)STRESS_TEST_LENGTH) * 100.0));

			printf("%s", buffer);
			fflush(stdout);

			system_pickRandomSeed();
		}

		min = (rand() % (STRESS_TEST_LENGTH - 1));
		max = (rand() % (STRESS_TEST_LENGTH - min - 1) + min + 1);

		for(ii = min; ii < max; ii++) {
			if(wildcardIndex_put(&wildcard, WILDCARD_INDEX_TYPE_EXACT,
						strings[ii], ii) < 0) {
				fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
						__FUNCTION__, __LINE__);
				return -1;
			}
		}

		for(ii = min; ii < max; ii++) {
			result = wildcardIndex_get(&wildcard, strings[ii], &length);
			if((result == NULL) || (length == 0)) {
				fprintf(stderr, " failed to retrieve inserted exact string\n");
				return -1;
			}

			for(kk = 0; kk < length; kk++) {
				if(result[kk] == ii) {
					break;
				}
			}
			free(result);

			if(kk == length) {
				fprintf(stderr, "failed to retrieve inserted exact string\n");
				return -1;
			}
		}

		min = rand() % (STRESS_TEST_LENGTH - 1);
		max = rand() % (STRESS_TEST_LENGTH - min - 1) + min + 1;

		for(ii = min; ii < max; ii++) {
			if(wildcardIndex_remove(&wildcard, strings[ii], ii) < 0) {
				fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
						__FUNCTION__, __LINE__);
				return -1;
			}
		}

		for(ii = min; ii < max; ii++) {
			result = wildcardIndex_get(&wildcard, strings[ii], &length);
			if((result != NULL) || (length != 0)) {
				fprintf(stderr, " failed to delete inserted exact string\n");
				free(result);
				return -1;
			}
		}
	}

	printf("\n");

	if(wildcardIndex_free(&wildcard) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	for(ii = 0; ii < STRESS_TEST_LENGTH; ii++) {
		free(strings[ii]);
	}

	free(strings);

	printf("[unit]\t ...PASSED\n");

	return 0;
}

