/*
 * managed_index_unitTest.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard core library background thread managed indexing system, using a
 * B+Tree index, type library, unit test.
 *
 * Written by Josh English.
 */


// preprocessor directives

#define _CORE_COMMON_BYPASS_STD_STREAM
#include "core/core.h"


// declare managed index unit test constants

//#define MANAGED_INDEX_TEST_IS_SPINLOCK_MODE					afalse
#define MANAGED_INDEX_TEST_IS_SPINLOCK_MODE					atrue

#define MANAGED_INDEX_BASIC_TEST_LENGTH						65536

#define MANAGED_INDEX_PERFORMANCE_TEST_LENGTH				262144

#define MANAGED_INDEX_MANAGEMENT_TEST_LENGTH				262144


// define managed index unit test data types

typedef struct _BasicTest {
	int keyLength;
	int valueLength;
	char *key;
	char *value;
} BasicTest;


// declare managed index unit test functions

static void freeBasicTest(void *argument, void *memory);

static void complexFreeBasicTest(void *argument, void *memory);

static int managedIndex_basicFunctionalityTest();

static char *generateRandomString(int minLength, int maxLength,
		int *stringLength);

static int managedIndex_basicPerformanceTest();

static int managedIndex_basicManagementTest();


// define managed index unit test global variables

static void *globalArgument = NULL;


// main function

int main()
{
	signal_registerDefault();

	printf("[unit] managed index unit test (Asgard Ver %s on %s)\n",
			ASGARD_VERSION, ASGARD_DATE);

	globalArgument = malloc(128);

	if(managedIndex_basicFunctionalityTest() < 0) {
		printf("[unit]\t ...FAILED\n");
		return 1;
	}

	free(globalArgument);

	if(managedIndex_basicPerformanceTest() < 0) {
		printf("[unit]\t ...FAILED\n");
		return 1;
	}

	if(managedIndex_basicManagementTest() < 0) {
		printf("[unit]\t ...FAILED\n");
		return 1;
	}

	printf("[unit] ...PASSED\n");

	return 0;
}


// define managed index unit tests

static void freeBasicTest(void *argument, void *memory)
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

static int managedIndex_basicFunctionalityTest()
{
	int ii = 0;
	int leafCount = 0;
	char *key = NULL;
	char *value = NULL;
	char buffer[1024];

	ManagedIndex index;
	BasicTest *entry = NULL;
	BasicTest *getEntry = NULL;

	printf("[unit]\t managed index basic functionality test...\n");

	if(managedIndex_init(&index, MANAGED_INDEX_TEST_IS_SPINLOCK_MODE) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	for(ii = 0; ii < MANAGED_INDEX_BASIC_TEST_LENGTH; ii++) {
		snprintf(buffer, sizeof(buffer), "bptree test key %08i", ii);

		key = strdup(buffer);

		snprintf(buffer, sizeof(buffer), "bptree test value %08i", ii);

		value = strdup(buffer);

		if(managedIndex_put(&index, key, (int)strlen(key), (void *)value,
					strlen(value)) < 0) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		if(managedIndex_get(&index, key, (int)strlen(key),
					((void *)&value)) < 0) {
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

		if(managedIndex_getLeafCount(&index, &leafCount) < 0) {
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

	for(ii = 0; ii < MANAGED_INDEX_BASIC_TEST_LENGTH; ii++) {
		snprintf(buffer, sizeof(buffer), "bptree test key %08i", ii);

		key = strdup(buffer);

		if(managedIndex_get(&index, key, (int)strlen(key),
					((void *)&value)) < 0) {
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

		if(managedIndex_remove(&index, key, (int)strlen(key)) < 0) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		free(key);

		if(managedIndex_getLeafCount(&index, &leafCount) < 0) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		if(leafCount != (MANAGED_INDEX_BASIC_TEST_LENGTH - (ii + 1))) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}
	}

	if(managedIndex_free(&index) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(managedIndex_init(&index, MANAGED_INDEX_TEST_IS_SPINLOCK_MODE) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(managedIndex_setFreeFunction(&index, NULL, freeBasicTest) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	for(ii = 0; ii < MANAGED_INDEX_BASIC_TEST_LENGTH; ii++) {
		entry = (BasicTest *)malloc(sizeof(BasicTest));

		entry->key = (char *)malloc(sizeof(char) * 128);
		entry->value = (char *)malloc(sizeof(char) * 128);

		entry->keyLength = snprintf(entry->key, (sizeof(char) * 128),
				"bptree/basic/key/%i", ii);

		entry->valueLength = snprintf(entry->value, (sizeof(char) * 128),
				"bptree/basic/value/%i", ii);

		if(managedIndex_put(&index, entry->key, entry->keyLength,
					(void *)entry,
					(sizeof(BasicTest) +
					 entry->keyLength +
					 entry->valueLength)) < 0) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		if(managedIndex_get(&index, entry->key, entry->keyLength,
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

		if(managedIndex_getLeafCount(&index, &leafCount) < 0) {
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

	for(ii = 0; ii < MANAGED_INDEX_BASIC_TEST_LENGTH; ii++) {
		snprintf(buffer, sizeof(buffer), "bptree/basic/key/%i", ii);

		key = strdup(buffer);

		if(managedIndex_get(&index, key, (int)strlen(key),
					((void *)&getEntry)) < 0) {
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

		if(managedIndex_remove(&index, key, (int)strlen(key)) < 0) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		free(key);

		if(managedIndex_getLeafCount(&index, &leafCount) < 0) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		if(leafCount != (MANAGED_INDEX_BASIC_TEST_LENGTH - (ii + 1))) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}
	}

	if(managedIndex_free(&index) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(managedIndex_init(&index, MANAGED_INDEX_TEST_IS_SPINLOCK_MODE) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(managedIndex_setFreeFunction(&index, globalArgument,
				complexFreeBasicTest) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	for(ii = 0; ii < MANAGED_INDEX_BASIC_TEST_LENGTH; ii++) {
		entry = (BasicTest *)malloc(sizeof(BasicTest));

		entry->key = (char *)malloc(sizeof(char) * 128);
		entry->value = (char *)malloc(sizeof(char) * 128);

		entry->keyLength = snprintf(entry->key, (sizeof(char) * 128),
				"bptree/basic/key/%i", ii);

		entry->valueLength = snprintf(entry->value, (sizeof(char) * 128),
				"bptree/basic/value/%i", ii);

		if(managedIndex_put(&index, entry->key, entry->keyLength,
					(void *)entry,
					(sizeof(BasicTest) +
					 entry->keyLength +
					 entry->valueLength)) < 0) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		if(managedIndex_get(&index, entry->key, entry->keyLength,
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

		if(managedIndex_getLeafCount(&index, &leafCount) < 0) {
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

	for(ii = 0; ii < MANAGED_INDEX_BASIC_TEST_LENGTH; ii++) {
		snprintf(buffer, sizeof(buffer), "bptree/basic/key/%i", ii);

		key = strdup(buffer);

		if(managedIndex_get(&index, key, (int)strlen(key),
					((void *)&getEntry)) < 0) {
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

		if(managedIndex_remove(&index, key, (int)strlen(key)) < 0) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		free(key);

		if(managedIndex_getLeafCount(&index, &leafCount) < 0) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		if(leafCount != (MANAGED_INDEX_BASIC_TEST_LENGTH - (ii + 1))) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}
	}

	if(managedIndex_free(&index) < 0) {
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

static int managedIndex_basicPerformanceTest()
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

	ManagedIndex index;

	printf("[unit]\t managed index basic performance test (%i iterations)...\n",
			MANAGED_INDEX_PERFORMANCE_TEST_LENGTH);

	if(managedIndex_init(&index, MANAGED_INDEX_TEST_IS_SPINLOCK_MODE) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	for(ii = 0; ii < MANAGED_INDEX_PERFORMANCE_TEST_LENGTH; ii++) {
		key = generateRandomString(8, 64, &keyLength);
		value = generateRandomString(8, 1024, &valueLength);

		timestamp = time_getTimeMus();

		if(managedIndex_put(&index, key, keyLength, (void *)value,
					valueLength) < 0) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		putTime += time_getElapsedMusInSeconds(timestamp);
		timestamp = time_getTimeMus();

		if(managedIndex_get(&index, key, keyLength, ((void *)&getValue)) < 0) {
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

		if(managedIndex_remove(&index, key, keyLength) < 0) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		removeTime += time_getElapsedMusInSeconds(timestamp);

		free(key);
	}

	if(managedIndex_free(&index) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t\t put total time       : %0.6f seconds\n", putTime);
	printf("[unit]\t\t put average time     : %0.6f seconds (%0.3f nanos)\n",
			(putTime / (double)MANAGED_INDEX_PERFORMANCE_TEST_LENGTH),
			((putTime / (double)MANAGED_INDEX_PERFORMANCE_TEST_LENGTH) *
			 REAL_ONE_BILLION));

	printf("[unit]\t\t get total time       : %0.6f seconds\n", getTime);
	printf("[unit]\t\t get average time     : %0.6f seconds (%0.3f nanos)\n",
			(getTime / (double)MANAGED_INDEX_PERFORMANCE_TEST_LENGTH),
			((getTime / (double)MANAGED_INDEX_PERFORMANCE_TEST_LENGTH) *
			 REAL_ONE_BILLION));

	printf("[unit]\t\t remove total time    : %0.6f seconds\n", removeTime);
	printf("[unit]\t\t remove average time  : %0.6f seconds (%0.3f nanos)\n",
			(removeTime / (double)MANAGED_INDEX_PERFORMANCE_TEST_LENGTH),
			((removeTime / (double)MANAGED_INDEX_PERFORMANCE_TEST_LENGTH) *
			 REAL_ONE_BILLION));

	printf("[unit]\t ...PASSED\n");

	return 0;
}

static int managedIndex_basicManagementTest()
{
	int ii = 0;
	int keyLength = 0;
	int valueLength = 0;
	alint cachedItemCount = 0;
	alint cachedMemoryLength = 0;
	double timestamp = 0.0;
	double putTime = 0.0;
	double getTime = 0.0;
	char *key = NULL;
	char *value = NULL;
	char *getValue = NULL;

	ManagedIndex index;

	printf("[unit]\t managed index basic management test (%i iterations)...\n",
			MANAGED_INDEX_MANAGEMENT_TEST_LENGTH);

	if(managedIndex_init(&index, MANAGED_INDEX_TEST_IS_SPINLOCK_MODE) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(managedIndex_setMaxCachedItemCount(&index, 128) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(managedIndex_setMaxCachedMemoryLength(&index, 16384) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(managedIndex_setTimeout(&index, 8.0) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	for(ii = 0; ii < MANAGED_INDEX_MANAGEMENT_TEST_LENGTH; ii++) {
		key = NULL;
		value = NULL;

		do {
			if(key != NULL) {
				free(key);
			}

			key = generateRandomString(8, 64, &keyLength);
		} while(managedIndex_get(&index, key, keyLength,
					((void *)&getValue)) == 0);

		value = generateRandomString(8, 1024, &valueLength);

		timestamp = time_getTimeMus();

		if(managedIndex_put(&index, key, keyLength, (void *)value,
					valueLength) < 0) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		putTime += time_getElapsedMusInSeconds(timestamp);
		timestamp = time_getTimeMus();

		if(managedIndex_get(&index, key, keyLength, ((void *)&getValue)) < 0) {
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

		free(key);
	}

	if(managedIndex_getCacheStatus(&index, &cachedItemCount,
				&cachedMemoryLength) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	while(cachedItemCount > (MANAGED_INDEX_MANAGEMENT_TEST_LENGTH / 2)) {
		printf("[unit]\t\t cached item count    : %lli\n",
				cachedItemCount);
		printf("[unit]\t\t cached memory length : %lli bytes\n",
				cachedMemoryLength);

		time_sleep(2);

		if(managedIndex_getCacheStatus(&index, &cachedItemCount,
					&cachedMemoryLength) < 0) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}
	}

	printf("[unit]\t\t cached item count    : %lli\n",
			cachedItemCount);
	printf("[unit]\t\t cached memory length : %lli bytes\n",
			cachedMemoryLength);

	if(managedIndex_free(&index) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t\t put total time       : %0.6f seconds\n", putTime);
	printf("[unit]\t\t put average time     : %0.6f seconds (%0.3f nanos)\n",
			(putTime / (double)MANAGED_INDEX_MANAGEMENT_TEST_LENGTH),
			((putTime / (double)MANAGED_INDEX_MANAGEMENT_TEST_LENGTH) *
			 REAL_ONE_BILLION));

	printf("[unit]\t\t get total time       : %0.6f seconds\n", getTime);
	printf("[unit]\t\t get average time     : %0.6f seconds (%0.3f nanos)\n",
			(getTime / (double)MANAGED_INDEX_MANAGEMENT_TEST_LENGTH),
			((getTime / (double)MANAGED_INDEX_MANAGEMENT_TEST_LENGTH) *
			 REAL_ONE_BILLION));

	printf("[unit]\t ...PASSED\n");

	return 0;
}

