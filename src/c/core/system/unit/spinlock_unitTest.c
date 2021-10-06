/*
 * spinlock_unitTest.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard spin-lock system for providing enhanced availability to shared
 * resources in a multithreaded environment, unit test.
 *
 * Written by Josh English.
 */

// preprocessor directives

#define _CORE_COMMON_BYPASS_STD_STREAM
#include "core/core.h"


// define spinlock unit test private constants

#define PERFORMANCE_TEST_ITERATIONS						8192

#define MULTITHREADED_TEST_VERBOSE_MODE					afalse

#define MULTITHREADED_TEST_WORKERS						32

#define MULTITHREADED_TEST_ITERATIONS					16384
//#define MULTITHREADED_TEST_ITERATIONS					65536
//#define MULTITHREADED_TEST_ITERATIONS					262144


// define spinlock unit test private data types

typedef struct _SpinlockUnitTest {
	aboolean isSpinlockMode;
	int dataElement;
	int writes;
	int reads;
	double writeLockTime;
	double writeUnlockTime;
	double readLockTime;
	double readUnlockTime;
	Spinlock spinlock;
	Mutex mutex;
	Mutex dataMutex;
} SpinlockUnitTest;


// declare spinlock unit test private functions

static int spinlock_basicFunctionalityTest();

static int spinlock_basicPerformanceTest();

static void *spinlockWorkerThread(void *threadContext, void *argument);

static int spinlock_basicMultithreadedTest();


// main function

int main()
{
	signal_registerDefault();

	printf("[unit] spinlock unit test (Asgard Ver %s on %s)\n", ASGARD_VERSION,
			ASGARD_DATE);

	if(spinlock_basicFunctionalityTest() < 0) {
		printf("[unit]\t ...FAILED\n");
		return 1;
	}

	if(spinlock_basicPerformanceTest() < 0) {
		printf("[unit]\t ...FAILED\n");
		return 1;
	}

	if(spinlock_basicMultithreadedTest() < 0) {
		printf("[unit]\t ...FAILED\n");
		return 1;
	}

	printf("[unit] ...PASSED\n");

	return 0;
}


// define spinlock unit test private functions

static int spinlock_basicFunctionalityTest()
{
	int lockThreadId = 0;
	int lockLine = 0;
	int lockCount = 0;
	double lockSeconds = 0.0;
	char *lockFile = NULL;
	char *lockFunction = NULL;

	Spinlock spinlock;

	printf("[unit]\t spinlock basic functionality test...\n");

	if(spinlock_init(&spinlock) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(spinlock_setTimeout(&spinlock, 2097152) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(spinlock_free(&spinlock) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(spinlock_init(&spinlock) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(spinlock_writeLock(&spinlock) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(!spinlock_isLocked(&spinlock)) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(spinlock_getSpinThreadCount(&spinlock, &lockCount) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t\t ...spin thread count  : %i\n", lockCount);

	if(lockCount != 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(spinlock_getLockerInfo(&spinlock,
				&lockThreadId,
				&lockFile,
				&lockFunction,
				&lockLine,
				&lockSeconds) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t\t ...lock thread id     : %i\n", lockThreadId);
	printf("[unit]\t\t ...lock file          : %s\n", lockFile);
	printf("[unit]\t\t ...lock function      : %s\n", lockFunction);
	printf("[unit]\t\t ...lock line          : %i\n", lockLine);
	printf("[unit]\t\t ...lock seconds       : %0.6f\n", lockSeconds);

	if(spinlock_writeUnlock(&spinlock) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(spinlock_readLock(&spinlock) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(spinlock_getSpinThreadCount(&spinlock, &lockCount) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t\t ...spin thread count  : %i\n", lockCount);

	if(lockCount != 1) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(spinlock_readUnlock(&spinlock) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(spinlock_writeLock(&spinlock) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(spinlock_readLock(&spinlock) == 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(spinlock_readUnlock(&spinlock) == 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(spinlock_writeUnlock(&spinlock) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(spinlock_readLock(&spinlock) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(spinlock_writeLock(&spinlock) == 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(spinlock_writeUnlock(&spinlock) == 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(spinlock_readUnlock(&spinlock) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(spinlock_writeLock(&spinlock) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(spinlock_writeUnlock(&spinlock) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(spinlock_readLock(&spinlock) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(spinlock_readUnlock(&spinlock) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(spinlock_free(&spinlock) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t ...PASSED\n");

	return 0;
}

static int spinlock_basicPerformanceTest()
{
	int ii = 0;
	double timestamp = 0.0;
	double writeLockTime = 0.0;
	double writeUnlockTime = 0.0;
	double readLockTime = 0.0;
	double readUnlockTime = 0.0;

	Spinlock spinlock;

	printf("[unit]\t spinlock basic performance test...\n");

	if(spinlock_init(&spinlock) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	for(ii = 0; ii < PERFORMANCE_TEST_ITERATIONS; ii++) {
		timestamp = time_getTimeMus();

		spinlock_writeLock(&spinlock);

		writeLockTime += time_getElapsedMusInSeconds(timestamp);
		timestamp = time_getTimeMus();

		spinlock_writeUnlock(&spinlock);

		writeUnlockTime += time_getElapsedMusInSeconds(timestamp);
		timestamp = time_getTimeMus();

		spinlock_readLock(&spinlock);

		readLockTime += time_getElapsedMusInSeconds(timestamp);
		timestamp = time_getTimeMus();

		spinlock_readUnlock(&spinlock);

		readUnlockTime += time_getElapsedMusInSeconds(timestamp);
	}

	if(spinlock_free(&spinlock) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t\t write lock average time   : %0.6f seconds\n",
			(writeLockTime / (double)PERFORMANCE_TEST_ITERATIONS));

	printf("[unit]\t\t write unlock average time : %0.6f seconds\n",
			(writeUnlockTime / (double)PERFORMANCE_TEST_ITERATIONS));

	printf("[unit]\t\t read lock average time    : %0.6f seconds\n",
			(readLockTime / (double)PERFORMANCE_TEST_ITERATIONS));

	printf("[unit]\t\t read unlock average time  : %0.6f seconds\n",
			(readUnlockTime / (double)PERFORMANCE_TEST_ITERATIONS));

	printf("[unit]\t ...PASSED\n");

	return 0;
}

static void *spinlockWorkerThread(void *threadContext, void *argument)
{
	int dataElement = 0;
	double timestamp = 0.0;
	double elapsedTime = 0.0;

	TmfThread *thread = NULL;
	SpinlockUnitTest *context = NULL;

	if(MULTITHREADED_TEST_VERBOSE_MODE) {
		printf("[unit]\t\t %s() executed with arguments 0x%lx, 0x%lx\n",
				__FUNCTION__, (aptrcast)threadContext,
				(aptrcast)argument);
	}

	thread = (TmfThread *)threadContext;
	context = (SpinlockUnitTest *)argument;

	if(MULTITHREADED_TEST_VERBOSE_MODE) {
		printf("[unit]\t\t %s() (thread #%i {%i}) running\n", __FUNCTION__,
				thread->uid, thread->systemId);
	}

	while((thread->state == THREAD_STATE_RUNNING) &&
			(thread->signal == THREAD_SIGNAL_OK)) {
		mutex_lock(thread->mutex);
		thread->status = THREAD_STATUS_BUSY;
		mutex_unlock(thread->mutex);

		if((rand() % 100) > 49) {
			timestamp = time_getTimeMus();

			if(context->isSpinlockMode) {
				if(spinlock_writeLock(&context->spinlock) < 0) {
					printf("[unit]\t\t ...ERROR, failed at line %i, "
							"aborting.\n", __LINE__);

					mutex_lock(thread->mutex);
					thread->status = THREAD_STATUS_ERROR;
					mutex_unlock(thread->mutex);

					return NULL;
				}
			}
			else {
				if(mutex_lock(&context->dataMutex) < 0) {
					printf("[unit]\t\t ...ERROR, failed at line %i, "
							"aborting.\n", __LINE__);

					mutex_lock(thread->mutex);
					thread->status = THREAD_STATUS_ERROR;
					mutex_unlock(thread->mutex);

					return NULL;
				}
			}

			elapsedTime = time_getElapsedMusInSeconds(timestamp);

			mutex_lock(&context->mutex);
			context->writeLockTime += elapsedTime;
			mutex_unlock(&context->mutex);

			context->dataElement += 1;

			timestamp = time_getTimeMus();

			if(context->isSpinlockMode) {
				if(spinlock_writeUnlock(&context->spinlock) < 0) {
					printf("[unit]\t\t ...ERROR, failed at line %i, "
							"aborting.\n", __LINE__);

					mutex_lock(thread->mutex);
					thread->status = THREAD_STATUS_ERROR;
					mutex_unlock(thread->mutex);

					return NULL;
				}
			}
			else {
				if(mutex_unlock(&context->dataMutex) < 0) {
					printf("[unit]\t\t ...ERROR, failed at line %i, "
							"aborting.\n", __LINE__);

					mutex_lock(thread->mutex);
					thread->status = THREAD_STATUS_ERROR;
					mutex_unlock(thread->mutex);

					return NULL;
				}
			}

			elapsedTime = time_getElapsedMusInSeconds(timestamp);

			mutex_lock(&context->mutex);
			context->writes += 1;
			context->writeUnlockTime += elapsedTime;
			mutex_unlock(&context->mutex);
		}
		else {
			timestamp = time_getTimeMus();

			if(context->isSpinlockMode) {
				if(spinlock_readLock(&context->spinlock) < 0) {
					printf("[unit]\t\t ...ERROR, failed at line %i, "
							"aborting.\n", __LINE__);

					mutex_lock(thread->mutex);
					thread->status = THREAD_STATUS_ERROR;
					mutex_unlock(thread->mutex);

					return NULL;
				}
			}
			else {
				if(mutex_lock(&context->dataMutex) < 0) {
					printf("[unit]\t\t ...ERROR, failed at line %i, "
							"aborting.\n", __LINE__);

					mutex_lock(thread->mutex);
					thread->status = THREAD_STATUS_ERROR;
					mutex_unlock(thread->mutex);

					return NULL;
				}
			}

			elapsedTime = time_getElapsedMusInSeconds(timestamp);

			mutex_lock(&context->mutex);
			context->readLockTime += elapsedTime;
			mutex_unlock(&context->mutex);

			dataElement = context->dataElement;

			timestamp = time_getTimeMus();

			if(context->isSpinlockMode) {
				if(spinlock_readUnlock(&context->spinlock) < 0) {
					printf("[unit]\t\t ...ERROR, failed at line %i, "
							"aborting.\n", __LINE__);

					mutex_lock(thread->mutex);
					thread->status = THREAD_STATUS_ERROR;
					mutex_unlock(thread->mutex);

					return NULL;
				}
			}
			else {
				if(mutex_unlock(&context->dataMutex) < 0) {
					printf("[unit]\t\t ...ERROR, failed at line %i, "
							"aborting.\n", __LINE__);

					mutex_lock(thread->mutex);
					thread->status = THREAD_STATUS_ERROR;
					mutex_unlock(thread->mutex);

					return NULL;
				}
			}

			elapsedTime = time_getElapsedMusInSeconds(timestamp);

			mutex_lock(&context->mutex);
			context->reads += 1;
			context->readUnlockTime += elapsedTime;
			mutex_unlock(&context->mutex);
		}

		mutex_lock(thread->mutex);
		thread->status = THREAD_STATUS_NAPPING;
		mutex_unlock(thread->mutex);

		time_usleep(1024);
	}

	if(MULTITHREADED_TEST_VERBOSE_MODE) {
		printf("[unit]\t\t %s() current data element: %i\n", __FUNCTION__,
				dataElement);
		printf("[unit]\t\t %s() (thread #%i {%i}) halting\n", __FUNCTION__,
				thread->uid, thread->systemId);
	}

	return (void *)"PASSED";
}

static int spinlock_basicMultithreadedTest()
{
	int ii = 0;
	int length = 0;
	int threadId = 0;
	int threadPoolSize = 0;
	int currentThreadCount = 0;
	int activeThreadCount = 0;
	char buffer[128];

	TmfContext tmf;
	SpinlockUnitTest context;

	printf("[unit]\t spinlock basic multithreaded test...\n");
	printf("[unit]\t\t mode: spinlock\n");

	context.isSpinlockMode = atrue;
	context.dataElement = 0;
	context.writes = 0;
	context.reads = 0;
	context.writeLockTime = 0.0;
	context.writeUnlockTime = 0.0;
	context.readLockTime = 0.0;
	context.readUnlockTime = 0.0;

	mutex_init(&context.mutex);
	mutex_init(&context.dataMutex);

	if(spinlock_init(&context.spinlock) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(tmf_init(&tmf) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	for(ii = 0; ii < MULTITHREADED_TEST_WORKERS; ii++) {
		if(tmf_spawnThread(&tmf, spinlockWorkerThread, (void *)&context,
					&threadId) < 0) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		if(MULTITHREADED_TEST_VERBOSE_MODE) {
			printf("[unit]\t\t started unit test thread #%i\n", threadId);
		}
	}

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

	memset(buffer, 0, (int)(sizeof(buffer)));

	length = snprintf(buffer, (int)(sizeof(buffer) - 1), "%i (%0.2f %%)",
			context.dataElement,
			(((double)context.dataElement /
			  (double)MULTITHREADED_TEST_ITERATIONS) * 100.0));

	printf("[unit]\t\t current data element :: ");

	while(context.dataElement < MULTITHREADED_TEST_ITERATIONS) {
		printf("%s", buffer);
		fflush(stdout);

		time_usleep(1000);

		for(ii = 0; ii < length; ii++) {
			printf("\b");
		}

		length = snprintf(buffer, (int)(sizeof(buffer) - 1), "%i (%0.2f %%)",
				context.dataElement,
				(((double)context.dataElement /
				  (double)MULTITHREADED_TEST_ITERATIONS) * 100.0));
	}

	printf("%s\n", buffer);

	if(tmf_free(&tmf) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(spinlock_free(&context.spinlock) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t\t write lock average time   : %0.6f seconds\n",
			(context.writeLockTime / (double)context.writes));

	printf("[unit]\t\t write unlock average time : %0.6f seconds\n",
			(context.writeUnlockTime / (double)context.writes));

	printf("[unit]\t\t read lock average time    : %0.6f seconds\n",
			(context.readLockTime / (double)context.reads));

	printf("[unit]\t\t read unlock average time  : %0.6f seconds\n",
			(context.readUnlockTime / (double)context.reads));

	mutex_free(&context.mutex);
	mutex_free(&context.dataMutex);

	printf("[unit]\t\t mode: mutex\n");

	context.isSpinlockMode = afalse;
	context.dataElement = 0;
	context.writes = 0;
	context.reads = 0;
	context.writeLockTime = 0.0;
	context.writeUnlockTime = 0.0;
	context.readLockTime = 0.0;
	context.readUnlockTime = 0.0;

	mutex_init(&context.mutex);
	mutex_init(&context.dataMutex);

	if(spinlock_init(&context.spinlock) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(tmf_init(&tmf) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	for(ii = 0; ii < MULTITHREADED_TEST_WORKERS; ii++) {
		if(tmf_spawnThread(&tmf, spinlockWorkerThread, (void *)&context,
					&threadId) < 0) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		if(MULTITHREADED_TEST_VERBOSE_MODE) {
			printf("[unit]\t\t started unit test thread #%i\n", threadId);
		}
	}

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

	memset(buffer, 0, (int)(sizeof(buffer)));

	length = snprintf(buffer, (int)(sizeof(buffer) - 1), "%i (%0.2f %%)",
			context.dataElement,
			(((double)context.dataElement /
			  (double)MULTITHREADED_TEST_ITERATIONS) * 100.0));

	printf("[unit]\t\t current data element :: ");

	while(context.dataElement < MULTITHREADED_TEST_ITERATIONS) {
		printf("%s", buffer);
		fflush(stdout);

		time_usleep(1000);

		for(ii = 0; ii < length; ii++) {
			printf("\b");
		}

		length = snprintf(buffer, (int)(sizeof(buffer) - 1), "%i (%0.2f %%)",
				context.dataElement,
				(((double)context.dataElement /
				  (double)MULTITHREADED_TEST_ITERATIONS) * 100.0));
	}

	printf("%s\n", buffer);

	if(tmf_free(&tmf) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(spinlock_free(&context.spinlock) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t\t write lock average time   : %0.6f seconds\n",
			(context.writeLockTime / (double)context.writes));

	printf("[unit]\t\t write unlock average time : %0.6f seconds\n",
			(context.writeUnlockTime / (double)context.writes));

	printf("[unit]\t\t read lock average time    : %0.6f seconds\n",
			(context.readLockTime / (double)context.reads));

	printf("[unit]\t\t read unlock average time  : %0.6f seconds\n",
			(context.readUnlockTime / (double)context.reads));

	mutex_free(&context.mutex);
	mutex_free(&context.dataMutex);

	printf("[unit]\t ...PASSED\n");

	return 0;
}

