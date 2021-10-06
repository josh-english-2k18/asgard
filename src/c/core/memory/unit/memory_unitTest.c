/*
 * memory_unitTest.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard memory-wrapping functions and macros, unit test.
 *
 * Written by Josh English.
 */

// preprocessor directives

#define _CORE_COMMON_BYPASS_STD_STREAM
#include "core/core.h"


// define memory unit test private constants

#define PERFORMANCE_TEST_ITERATIONS						131072
//#define PERFORMANCE_TEST_ITERATIONS						1048576


// declare memory unit test private functions

static int memory_basicFunctionalityTest();

static int memory_basicPerformanceTest();


// main function

int main()
{
	signal_registerDefault();

	printf("[unit] memory unit test (Asgard Ver %s on %s)\n", ASGARD_VERSION,
			ASGARD_DATE);

	memory_init();

	if(memory_basicFunctionalityTest() < 0) {
		printf("[unit]\t ...FAILED\n");
		return 1;
	}

	if(memory_basicPerformanceTest() < 0) {
		printf("[unit]\t ...FAILED\n");
		return 1;
	}

	memory_displayStatus();
	memory_displaySlotProfile();

	printf("[unit] ...PASSED\n");

	return 0;
}


// define memory unit test private functions

static int memory_basicFunctionalityTest()
{
	int ii = 0;
	int length = 0;
	char *string = NULL;
	char *duplicate = NULL;
	void *memory = NULL;
	void **array = NULL;

	printf("[unit]\t memory basic functionality test...\n");

	memory = malloc(1024);
	if(memory == NULL) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	memory = realloc(memory, 2048);
	if(memory == NULL) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	memory = realloc(memory, 512);
	if(memory == NULL) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	free(memory);

	string = strdup("this is a test string");
	if((string == NULL) || (strcmp(string, "this is a test string"))) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	length = strlen(string);
	duplicate = strndup(string, length);
	if((duplicate == NULL) || (strcmp(duplicate, "this is a test string"))) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	free(string);
	free(duplicate);

	system_pickRandomSeed();

	array = (void **)malloc(sizeof(void *) * 1024);

	for(ii = 0; ii < 1024; ii++) {
		array[ii] = malloc((rand() % 16384) + 1);

		if((rand() % 100) > 49) {
			free(array[ii]);
			array[ii] = NULL;
		}
		else {
			array[ii] = realloc(array[ii], ((rand() % 16384) + 1));
		}
	}

	for(ii = 0; ii < 1024; ii++) {
		if(array[ii] != NULL) {
			free(array[ii]);
		}
	}

	free(array);

	printf("[unit]\t ...PASSED\n");

	return 0;
}

static int memory_basicPerformanceTest()
{
	int ii = 0;
	double timestamp = 0.0;
	double mallocTime = 0.0;
	double reallocTime = 0.0;
	double freeTime = 0.0;
	void *value = NULL;

	printf("[unit]\t memory basic performance test...\n");

	for(ii = 0; ii < PERFORMANCE_TEST_ITERATIONS; ii++) {
		timestamp = time_getTimeMus();

		value = (void *)malloc(sizeof(char) * (ii + 8192));

		mallocTime += time_getElapsedMusInSeconds(timestamp);
		timestamp = time_getTimeMus();

		value = (void *)realloc(value, (sizeof(char) * (ii + 8192 + 128)));

		reallocTime += time_getElapsedMusInSeconds(timestamp);
		timestamp = time_getTimeMus();

		free(value);

		freeTime += time_getElapsedMusInSeconds(timestamp);
	}

	printf("[unit]\t\t malloc total time      : %0.6f seconds\n", mallocTime);
	printf("[unit]\t\t malloc average time    : %0.6f seconds\n",
			(mallocTime / (double)PERFORMANCE_TEST_ITERATIONS));

	printf("[unit]\t\t realloc total time     : %0.6f seconds\n", reallocTime);
	printf("[unit]\t\t realloc average time   : %0.6f seconds\n",
			(reallocTime / (double)PERFORMANCE_TEST_ITERATIONS));

	printf("[unit]\t\t free total time        : %0.6f seconds\n", freeTime);
	printf("[unit]\t\t free average time      : %0.6f seconds\n",
			(freeTime / (double)PERFORMANCE_TEST_ITERATIONS));

	printf("[unit]\t ...PASSED\n");

	return 0;
}

