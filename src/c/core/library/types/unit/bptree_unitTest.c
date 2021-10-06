/*
 * bptree_unitTest.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard core library B+Tree type library, unit test.
 *
 * Written by Josh English.
 */

// preprocessor directives

#define _CORE_COMMON_BYPASS_STD_STREAM
#include "core/core.h"


// declare b+tree unit test constants

#define BPTREE_BASIC_TEST_LENGTH					65536

#define BPTREE_PERFORMANCE_TEST_LENGTH				262144


// define b+tree unit test data types

typedef struct _BasicTest {
	int keyLength;
	int valueLength;
	char *key;
	char *value;
} BasicTest;


// declare b+tree unit test functions

static void freeBasicTest(void *memory);

static void complexFreeBasicTest(void *argument, void *memory);

static int bptree_basicFunctionalityTest();

static char *generateRandomString(int minLength, int maxLength,
		int *stringLength);

static int bptree_basicPerformanceTest();


// define b+tree unit test global variables

static void *globalArgument = NULL;


// main function

int main()
{
	signal_registerDefault();

	printf("[unit] b+tree unit test (Asgard Ver %s on %s)\n",
			ASGARD_VERSION, ASGARD_DATE);

	globalArgument = malloc(128);

	if(bptree_basicFunctionalityTest() < 0) {
		printf("[unit]\t ...FAILED\n");
		return 1;
	}

	free(globalArgument);

	if(bptree_basicPerformanceTest() < 0) {
		printf("[unit]\t ...FAILED\n");
		return 1;
	}

	printf("[unit] ...PASSED\n");

	return 0;
}


// define b+tree unit tests

static void freeBasicTest(void *memory)
{
	BasicTest *basicTest = NULL;

	if(memory == NULL) {
		DISPLAY_INVALID_ARGS;
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		exit(1);
	}

	basicTest = (BasicTest *)memory;

	if(basicTest->key != NULL) {
		free(basicTest->key);
	}

	if(basicTest->value != NULL) {
		free(basicTest->value);
	}

	free(basicTest);
}

static void complexFreeBasicTest(void *argument, void *memory)
{
	BasicTest *basicTest = NULL;

	if(argument != globalArgument) {
		DISPLAY_INVALID_ARGS;
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		exit(1);
	}

	if(memory == NULL) {
		DISPLAY_INVALID_ARGS;
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		exit(1);
	}

	basicTest = (BasicTest *)memory;

	if(basicTest->key != NULL) {
		free(basicTest->key);
	}

	if(basicTest->value != NULL) {
		free(basicTest->value);
	}

	free(basicTest);
}

static int bptree_basicFunctionalityTest()
{
	int ii = 0;
	int leafCount = 0;
	char *key = NULL;
	char *value = NULL;
	char buffer[1024];

	Bptree index;
	BasicTest *entry = NULL;
	BasicTest *getEntry = NULL;

	printf("[unit]\t b+tree basic functionality test...\n");

	if(bptree_init(&index) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	for(ii = 0; ii < BPTREE_BASIC_TEST_LENGTH; ii++) {
		snprintf(buffer, sizeof(buffer), "bptree test key %08i", ii);

		key = strdup(buffer);

		snprintf(buffer, sizeof(buffer), "bptree test value %08i", ii);

		value = strdup(buffer);

		if(bptree_put(&index, key, (int)strlen(key), (void *)value) < 0) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		if(bptree_get(&index, key, (int)strlen(key), ((void *)&value)) < 0) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		if(strcmp(buffer, value)) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		free(key);

		if(bptree_getLeafCount(&index, &leafCount) < 0) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		if(leafCount != (ii + 1)) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}
	}

	for(ii = 0; ii < BPTREE_BASIC_TEST_LENGTH; ii++) {
		snprintf(buffer, sizeof(buffer), "bptree test key %08i", ii);

		key = strdup(buffer);

		if(bptree_get(&index, key, (int)strlen(key), ((void *)&value)) < 0) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		snprintf(buffer, sizeof(buffer), "bptree test value %08i", ii);

		if(strcmp(buffer, value)) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		if(bptree_remove(&index, key, (int)strlen(key)) < 0) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		free(key);

		if(bptree_getLeafCount(&index, &leafCount) < 0) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		if(leafCount != (BPTREE_BASIC_TEST_LENGTH - (ii + 1))) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}
	}

	if(bptree_free(&index) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(bptree_init(&index) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(bptree_setFreeFunction(&index, freeBasicTest) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	for(ii = 0; ii < BPTREE_BASIC_TEST_LENGTH; ii++) {
		entry = (BasicTest *)malloc(sizeof(BasicTest));

		entry->key = (char *)malloc(sizeof(char) * 128);
		entry->value = (char *)malloc(sizeof(char) * 128);

		entry->keyLength = snprintf(entry->key, (sizeof(char) * 128),
				"bptree/basic/key/%i", ii);

		entry->valueLength = snprintf(entry->value, (sizeof(char) * 128),
				"bptree/basic/value/%i", ii);

		if(bptree_put(&index, entry->key, entry->keyLength,
					(void *)entry) < 0) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		if(bptree_get(&index, entry->key, entry->keyLength,
					((void *)&getEntry)) < 0) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		if(entry->keyLength != getEntry->keyLength) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		if(strcmp(entry->key, getEntry->key)) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		if(entry->valueLength != getEntry->valueLength) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		if(strcmp(entry->value, getEntry->value)) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		if(bptree_getLeafCount(&index, &leafCount) < 0) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		if(leafCount != (ii + 1)) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}
	}

	for(ii = 0; ii < BPTREE_BASIC_TEST_LENGTH; ii++) {
		snprintf(buffer, sizeof(buffer), "bptree/basic/key/%i", ii);

		key = strdup(buffer);

		if(bptree_get(&index, key, (int)strlen(key), ((void *)&getEntry)) < 0) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		snprintf(buffer, sizeof(buffer), "bptree/basic/value/%i", ii);

		if(strcmp(buffer, getEntry->value)) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		if(bptree_remove(&index, key, (int)strlen(key)) < 0) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		free(key);

		if(bptree_getLeafCount(&index, &leafCount) < 0) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		if(leafCount != (BPTREE_BASIC_TEST_LENGTH - (ii + 1))) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}
	}

	if(bptree_free(&index) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(bptree_init(&index) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(bptree_setComplexFreeFunction(&index, globalArgument,
				complexFreeBasicTest) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	for(ii = 0; ii < BPTREE_BASIC_TEST_LENGTH; ii++) {
		entry = (BasicTest *)malloc(sizeof(BasicTest));

		entry->key = (char *)malloc(sizeof(char) * 128);
		entry->value = (char *)malloc(sizeof(char) * 128);

		entry->keyLength = snprintf(entry->key, (sizeof(char) * 128),
				"bptree/basic/key/%i", ii);

		entry->valueLength = snprintf(entry->value, (sizeof(char) * 128),
				"bptree/basic/value/%i", ii);

		if(bptree_put(&index, entry->key, entry->keyLength,
					(void *)entry) < 0) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		if(bptree_get(&index, entry->key, entry->keyLength,
					((void *)&getEntry)) < 0) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		if(entry->keyLength != getEntry->keyLength) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		if(strcmp(entry->key, getEntry->key)) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		if(entry->valueLength != getEntry->valueLength) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		if(strcmp(entry->value, getEntry->value)) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		if(bptree_getLeafCount(&index, &leafCount) < 0) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		if(leafCount != (ii + 1)) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}
	}

	for(ii = 0; ii < BPTREE_BASIC_TEST_LENGTH; ii++) {
		snprintf(buffer, sizeof(buffer), "bptree/basic/key/%i", ii);

		key = strdup(buffer);

		if(bptree_get(&index, key, (int)strlen(key), ((void *)&getEntry)) < 0) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		snprintf(buffer, sizeof(buffer), "bptree/basic/value/%i", ii);

		if(strcmp(buffer, getEntry->value)) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		if(bptree_remove(&index, key, (int)strlen(key)) < 0) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		free(key);

		if(bptree_getLeafCount(&index, &leafCount) < 0) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		if(leafCount != (BPTREE_BASIC_TEST_LENGTH - (ii + 1))) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}
	}

	if(bptree_free(&index) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t ...PASSED\n");

	return 0;
}

static char *generateRandomString(int minLength, int maxLength,
		int *stringLength)
{
	int ii = 0;
	int pick = 0;
	int length = 0;
	char *result = NULL;

	*stringLength = 0;

	system_pickRandomSeed();

	length = ((rand() % (maxLength - minLength)) + minLength);

	result = (char *)malloc(sizeof(char) * (length + 1));

	for(ii = 0; ii < length; ii++) {
		pick = (rand() % 3);

		switch(pick) {
			case 0: // number
				result[ii] = (char)((rand() % 10) + 48);
				break;

			case 1: // upper case
				result[ii] = (char)((rand() % 26) + 65);
				break;

			case 2: // lower case
				result[ii] = (char)((rand() % 26) + 97);
				break;
		}
	}

	*stringLength = length;

	return result;
}

static int bptree_basicPerformanceTest()
{
	int ii = 0;
	int keyLength = 0;
	int valueLength = 0;
	double timestamp = 0.0;
	double putTime = 0.0;
	double getTime = 0.0;
	double removeTime = 0.0;
	char *key = NULL;
	char *value = NULL;
	char *getValue = NULL;

	Bptree index;

	printf("[unit]\t b+tree basic performance test (%i iterations)...\n",
			BPTREE_PERFORMANCE_TEST_LENGTH);

	if(bptree_init(&index) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	for(ii = 0; ii < BPTREE_PERFORMANCE_TEST_LENGTH; ii++) {
		key = generateRandomString(8, 64, &keyLength);
		value = generateRandomString(8, 1024, &valueLength);

		timestamp = time_getTimeMus();

		if(bptree_put(&index, key, keyLength, (void *)value) < 0) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		putTime += time_getElapsedMusInSeconds(timestamp);
		timestamp = time_getTimeMus();

		if(bptree_get(&index, key, keyLength, ((void *)&getValue)) < 0) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		getTime += time_getElapsedMusInSeconds(timestamp);

		if(strcmp(value, getValue)) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		timestamp = time_getTimeMus();

		if(bptree_remove(&index, key, keyLength) < 0) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		removeTime += time_getElapsedMusInSeconds(timestamp);

		free(key);
	}

	if(bptree_free(&index) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t\t put total time       : %0.6f seconds\n", putTime);
	printf("[unit]\t\t put average time     : %0.6f seconds (%0.3f nanos)\n",
			(putTime / (double)BPTREE_PERFORMANCE_TEST_LENGTH),
			((putTime / (double)BPTREE_PERFORMANCE_TEST_LENGTH) *
			 REAL_ONE_BILLION));

	printf("[unit]\t\t get total time       : %0.6f seconds\n", getTime);
	printf("[unit]\t\t get average time     : %0.6f seconds (%0.3f nanos)\n",
			(getTime / (double)BPTREE_PERFORMANCE_TEST_LENGTH),
			((getTime / (double)BPTREE_PERFORMANCE_TEST_LENGTH) *
			 REAL_ONE_BILLION));

	printf("[unit]\t\t remove total time    : %0.6f seconds\n", removeTime);
	printf("[unit]\t\t remove average time  : %0.6f seconds (%0.3f nanos)\n",
			(removeTime / (double)BPTREE_PERFORMANCE_TEST_LENGTH),
			((removeTime / (double)BPTREE_PERFORMANCE_TEST_LENGTH) *
			 REAL_ONE_BILLION));

	printf("[unit]\t ...PASSED\n");

	return 0;
}

