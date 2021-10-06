/*
 * mutex_unitTest.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard mutex interface, unit test.
 *
 * Written by Josh English.
 */

// preprocessor directives

#define _CORE_COMMON_BYPASS_STD_STREAM
#include "core/core.h"

// declare mutex unit test private functions

static int mutex_basicFunctionalityTest();

// main function

int main()
{
	signal_registerDefault();

	printf("[unit] mutex unit test (Asgard Ver %s on %s)\n", ASGARD_VERSION,
			ASGARD_DATE);

	if(mutex_basicFunctionalityTest() < 0) {
		printf("[unit]\t ...FAILED\n");
		return 1;
	}

	printf("[unit] ...PASSED\n");

	return 0;
}

// define mutex unit test private functions

static int mutex_basicFunctionalityTest()
{
	int lockLine = 0;
	int lockCount = 0;
	char *lockFile = NULL;
	char *lockFunction = NULL;

	Mutex mutex;

	printf("[unit]\t mutex basic functionality test...\n");

	if(mutex_init(&mutex) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(mutex_lock(&mutex) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(mutex_getLockCount(&mutex, &lockCount) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t\t ...lock count    : %i\n", lockCount);

	if(lockCount != 1) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(mutex_getLockerInfo(&mutex, &lockFile, &lockFunction, &lockLine) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t\t ...lock file     : %s\n", lockFile);
	printf("[unit]\t\t ...lock function : %s\n", lockFunction);
	printf("[unit]\t\t ...lock line     : %i\n", lockLine);

	if(mutex_unlock(&mutex) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(mutex_getLockCount(&mutex, &lockCount) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t\t ...lock count    : %i\n", lockCount);

	if(lockCount != 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(mutex_free(&mutex) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t ...PASSED\n");

	return 0;
}

