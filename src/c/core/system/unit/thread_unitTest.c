/*
 * thread_unitTest.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard thread interface, unit test.
 *
 * Written by Josh English.
 */

// preprocessor directives

#define _CORE_COMMON_BYPASS_STD_STREAM
#include "core/core.h"

// declare thread unit test private functions

static void *threadBasicTestWorker(void *argument);
static int thread_basicFunctionalityTest();

// main function

int main()
{
	signal_registerDefault();

	printf("[unit] thread unit test (Asgard Ver %s on %s)\n", ASGARD_VERSION,
			ASGARD_DATE);

	if(thread_basicFunctionalityTest() < 0) {
		printf("[unit]\t ...FAILED\n");
		return 1;
	}

	printf("[unit] ...PASSED\n");

	return 0;
}

// define thread unit test private functions

static void *threadBasicTestWorker(void *argument)
{
	int threadId = 0;
	int threadNumber = 0;

	printf("[unit]\t\t %s() spawned with argument 0x%lx\n", __FUNCTION__,
			(aptrcast)argument);

	threadNumber = *((int *)argument);

	printf("[unit]\t\t %s() thread number (from argument) is %i\n",
			__FUNCTION__, threadNumber);

	if(thread_self(&threadId)) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return (void *)"FAILED";
	}

	printf("[unit]\t\t %s() is thread #%i\n", __FUNCTION__, threadId);

	time_sleep(1);

	printf("[unit]\t\t %s() (thread #%i) shutting down\n", __FUNCTION__,
			threadId);

	return (void *)"PASSED";
}

static int thread_basicFunctionalityTest()
{
	int threadId = 0;
	int threadNumber = 0;
	void *exitValue = NULL;

	Thread thread;

	printf("[unit]\t thread basic functionality test...\n");

	if(thread_self(&threadId)) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t\t unit test process is thread #%i\n", threadId);

	threadNumber = 1234;

	printf("[unit]\t\t ...spawning thread (number %i)\n", threadNumber);

	if(thread_init(&thread) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(thread_create(&thread, threadBasicTestWorker,
				(void *)&threadNumber) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t\t ...joining thread\n");

	if(thread_join(&thread, &exitValue) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t\t thread exit value 0x%lx is '%s'\n",
			(aptrcast)exitValue, (char *)exitValue);

	printf("[unit]\t ...PASSED\n");

	return 0;
}

