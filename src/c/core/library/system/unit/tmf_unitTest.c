/*
 * tmf_unitTest.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard core library thread management framework, unit test.
 *
 * Written by Josh English.
 */

// preprocessor directives

#define _CORE_COMMON_BYPASS_STD_STREAM
#include "core/core.h"

// declare thread management framework unit test private functions

static void *tmfBasicTestWorker(void *threadContext, void *argument);

static int tmf_basicFunctionalityTest();

// main function

int main()
{
	signal_registerDefault();

	printf("[unit] thread management framework unit test "
			"(Asgard Ver %s on %s)\n", ASGARD_VERSION, ASGARD_DATE);

	if(tmf_basicFunctionalityTest() < 0) {
		printf("[unit]\t ...FAILED\n");
		return 1;
	}

	printf("[unit] ...PASSED\n");

	return 0;
}

// define thread management framework unit test private functions

static void *tmfBasicTestWorker(void *threadContext, void *argument)
{
	int threadNumber = 0;
	alint counter = 0;

	TmfThread *thread = NULL;

	printf("[unit]\t\t %s() executed with arguments 0x%lx, 0x%lx\n",
			__FUNCTION__, (aptrcast)threadContext,
			(aptrcast)argument);

	thread = (TmfThread *)threadContext;
	threadNumber = *((int *)argument);

	printf("[unit]\t\t %s() thread number (from argument) is %i\n",
			__FUNCTION__, threadNumber);

	printf("[unit]\t\t %s() (thread #%i {%i}) running\n", __FUNCTION__,
			thread->uid, thread->systemId);

	while((thread->state == THREAD_STATE_RUNNING) &&
			(thread->signal == THREAD_SIGNAL_OK)) {
		mutex_lock(thread->mutex);
		thread->status = THREAD_STATUS_BUSY;
		mutex_unlock(thread->mutex);

		counter++;

		mutex_lock(thread->mutex);
		thread->status = THREAD_STATUS_NAPPING;
		mutex_unlock(thread->mutex);

		time_usleep(1024);
	}

	printf("[unit]\t\t %s() counted to %lli\n", __FUNCTION__, counter);

	printf("[unit]\t\t %s() (thread #%i {%i}) halting\n", __FUNCTION__,
			thread->uid, thread->systemId);

	return (void *)"PASSED";
}

static int tmf_basicFunctionalityTest()
{
	int threadId = 0;
	int threadNumber = 0;
	int threadPoolSize = 0;
	int currentThreadCount = 0;
	int activeThreadCount = 0;

	TmfContext tmf;

	printf("[unit]\t thread management framework "
			"basic functionality test...\n");

	if(tmf_init(&tmf) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	threadNumber = 5678;

	printf("[unit]\t\t ...spawning thread (argument %i)\n", threadNumber);

	if(tmf_spawnThread(&tmf, tmfBasicTestWorker, (void *)&threadNumber,
				&threadId) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t\t started unit test thread #%i\n", threadId);

	if(tmf_getThreadPoolSize(&tmf, &threadPoolSize) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t\t thread pool size: %i\n", threadPoolSize);

	if(tmf_getCurrentThreadCount(&tmf, &currentThreadCount) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t\t current thread pool count: %i\n", currentThreadCount);

	if(tmf_getActiveThreadCount(&tmf, &activeThreadCount) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t\t active thread count: %i\n", activeThreadCount);

	time_sleep(1);

	if(tmf_stopThread(&tmf, threadId) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(tmf_free(&tmf) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t ...PASSED\n");

	return 0;
}

