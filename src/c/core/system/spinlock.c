/*
 * spinlock.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard spin-lock system for providing enhanced availability to shared
 * resources in a multithreaded environment.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#define _CORE_SYSTEM_COMPONENT
#include "core/system/spinlock.h"


// define spinlock private constants

typedef enum _SpinlockState {
	SPINLOCK_STATE_INIT = 1,
	SPINLOCK_STATE_WAITING,
	SPINLOCK_STATE_ACQUIRED,
	SPINLOCK_STATE_ERROR = -1
} SpinlockState;

#define SPIN_SLEEP_MICROSECONDS							32


// declare spinlock private functions

static char *statusToString(SpinlockStatus status);

static char *stateToString(SpinlockState state);


// define spinlock private functions

static char *statusToString(SpinlockStatus status)
{
	char *result = "Unknown";

	switch(status) {
		case SPINLOCK_STATUS_AVAILABLE:
			result = "Available";
			break;

		case SPINLOCK_STATUS_WAITING:
			result = "Waiting";
			break;

		case SPINLOCK_STATUS_LOCKED:
			result = "Locked";
			break;

		case SPINLOCK_STATUS_ERROR:
			result = "Error";
			break;

		default:
			result = "Unknown";
	}

	return result;
}

static char *stateToString(SpinlockState state)
{
	char *result = "Unknown";

	switch(state) {
		case SPINLOCK_STATE_INIT:
			result = "Init";
			break;

		case SPINLOCK_STATE_WAITING:
			result = "Waiting";
			break;

		case SPINLOCK_STATE_ACQUIRED:
			result = "Acquired";
			break;

		case SPINLOCK_STATE_ERROR:
			result = "Error";
			break;

		default:
			result = "Unknown";
	}

	return result;
}


// define spinlock public functions

int spinlock_init(Spinlock *spinlock)
{
	if(spinlock == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	memset(spinlock, 0, (int)(sizeof(Spinlock)));

	if(mutex_init(&spinlock->mutex) < 0) {
		return -1;
	}

	mutex_lock(&spinlock->mutex);

	spinlock->status = SPINLOCK_STATUS_AVAILABLE;
	spinlock->threadCount = 0;
	spinlock->spinThreshold = 0;
	spinlock->timeoutMicroseconds = SPINLOCK_DEFAULT_TIMEOUT_MICROSECONDS;
	spinlock->owner.line = 0;
	spinlock->owner.threadId = 0;
	spinlock->owner.file = NULL;
	spinlock->owner.function = NULL;
	spinlock->owner.timestamp = 0.0;

	spinlock->spinThreshold = (spinlock->timeoutMicroseconds /
			1024); // corrected value for CPU clock
//			SPIN_SLEEP_MICROSECONDS);

	if(spinlock->spinThreshold < 1024) {
		spinlock->spinThreshold = 1024;
	}

	mutex_unlock(&spinlock->mutex);

	return 0;
}

int spinlock_free(Spinlock *spinlock)
{
	if(spinlock == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&spinlock->mutex);

	if((spinlock->status != SPINLOCK_STATUS_AVAILABLE) ||
			(spinlock->threadCount != 0) ||
			(spinlock->owner.threadId != 0)) {
		fprintf(stderr, "[%s():%i] error - shutting down spinlock in "
				"status '%s', %i spinning threads, and locked owner #%i.\n",
				__FUNCTION__, __LINE__, statusToString(spinlock->status),
				spinlock->threadCount, spinlock->owner.threadId);

		spinlock->status = SPINLOCK_STATUS_ERROR;
	}

	mutex_unlock(&spinlock->mutex);

	if(mutex_free(&spinlock->mutex) < 0) {
		return -1;
	}

	memset(spinlock, 0, (int)(sizeof(Spinlock)));

	return 0;
}

int spinlock_setTimeout(Spinlock *spinlock, int timeoutMicroseconds)
{
	if((spinlock == NULL) || (timeoutMicroseconds < 1)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&spinlock->mutex);

	spinlock->timeoutMicroseconds = timeoutMicroseconds;

	spinlock->spinThreshold = (spinlock->timeoutMicroseconds /
			1024); // corrected value for CPU clock
//			SPIN_SLEEP_MICROSECONDS);

	if(spinlock->spinThreshold < 1024) {
		spinlock->spinThreshold = 1024;
	}

	mutex_unlock(&spinlock->mutex);

	return 0;
}

int __spinlock_writeLock(Spinlock *spinlock, const char *file,
		const char *function, size_t line)
{
	int state = 0;
	int counter = 0;
	int threadCount = 0;
	int lockerThreadId = 0;
	double timestamp = 0.0;

	if((spinlock == NULL) || (file == NULL) || (function == NULL) ||
			(line < 0)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	// spin until the spinlock is available

	state = SPINLOCK_STATE_INIT;

	if(EXPLICIT_ERRORS) {
		timestamp = time_getTimeMus();
	}

	while(counter < spinlock->spinThreshold) {
		mutex_lock(&spinlock->mutex);

		threadCount = spinlock->threadCount;

		switch(spinlock->status) {
			case SPINLOCK_STATUS_AVAILABLE:
				if(threadCount > 0) {
					spinlock->status = SPINLOCK_STATUS_WAITING;
					state = SPINLOCK_STATE_WAITING;
				}
				else if(threadCount == 0) {
					spinlock->status = SPINLOCK_STATUS_LOCKED;
					state = SPINLOCK_STATE_ACQUIRED;
				}
				else {
					state = SPINLOCK_STATE_ERROR;
				}
				break;

			case SPINLOCK_STATUS_WAITING:
			case SPINLOCK_STATUS_LOCKED:
				lockerThreadId = spinlock->owner.threadId;
				break;

			case SPINLOCK_STATUS_ERROR:
			default:
				state = SPINLOCK_STATE_ERROR;
				break;
		}

		mutex_unlock(&spinlock->mutex);

		if(state != SPINLOCK_STATE_INIT) {
			break;
		}

		counter++;

		time_usleep(SPIN_SLEEP_MICROSECONDS);
	}

	if(state == SPINLOCK_STATE_INIT) {
		fprintf(stderr, "{%s}[%s():%i]->[%s():%i] error - failed to "
				"request lock (%s) from {%s}[%s():%i] thread #%i after %0.6f "
				"seconds in status '%s' with %i spinners.\n",
				(char *)file, (char *)function, (int)line,
				__FUNCTION__, __LINE__,
				stateToString(state),
				spinlock->owner.file,
				spinlock->owner.function,
				spinlock->owner.line,
				spinlock->owner.threadId,
				time_getElapsedMusInSeconds(timestamp),
				statusToString(spinlock->status),
				threadCount);

		return -1;
	}
	else if(state == SPINLOCK_STATE_ACQUIRED) {
		mutex_lock(&spinlock->mutex);

		spinlock->owner.file = (char *)file;
		spinlock->owner.function = (char *)function;
		spinlock->owner.line = (int)line;

		thread_self(&spinlock->owner.threadId);

		spinlock->owner.timestamp = time_getTimeMus();

		mutex_unlock(&spinlock->mutex);

		return 0;
	}
	else if(state == SPINLOCK_STATE_ERROR) {
		return -1;
	}

	// wait for all threads to cease reading, and set to locked

	counter = 0;

	if(EXPLICIT_ERRORS) {
		timestamp = time_getTimeMus();
	}

	while(counter < spinlock->spinThreshold) {
		mutex_lock(&spinlock->mutex);

		threadCount = spinlock->threadCount;

		if(spinlock->status == SPINLOCK_STATUS_WAITING) {
			if(threadCount > 0) {
				state = SPINLOCK_STATE_WAITING;
			}
			else if(threadCount == 0) {
				spinlock->status = SPINLOCK_STATUS_LOCKED;
				state = SPINLOCK_STATE_ACQUIRED;
			}
			else {
				state = SPINLOCK_STATE_ERROR;
			}
		}
		else {
			state = SPINLOCK_STATE_ERROR;
		}

		mutex_unlock(&spinlock->mutex);

		if((state != SPINLOCK_STATE_INIT) &&
				(state != SPINLOCK_STATE_WAITING)) {
			break;
		}

		counter++;

		time_usleep(SPIN_SLEEP_MICROSECONDS);
	}

	if((state == SPINLOCK_STATE_INIT) || (state == SPINLOCK_STATE_WAITING)) {
		fprintf(stderr, "{%s}[%s():%i]->[%s():%i] error - failed to "
				"complete lock (%s) after %0.6f seconds with %i spinners.\n",
				(char *)file, (char *)function, (int)line,
				__FUNCTION__, __LINE__,
				stateToString(state),
				time_getElapsedMusInSeconds(timestamp),
				threadCount);

		mutex_lock(&spinlock->mutex);
		spinlock->status = SPINLOCK_STATUS_AVAILABLE;
		mutex_unlock(&spinlock->mutex);

		return -1;
	}
	else if(state == SPINLOCK_STATE_ERROR) {
		return -1;
	}

	mutex_lock(&spinlock->mutex);

	spinlock->owner.file = (char *)file;
	spinlock->owner.function = (char *)function;
	spinlock->owner.line = (int)line;

	thread_self(&spinlock->owner.threadId);

	spinlock->owner.timestamp = time_getTimeMus();

	mutex_unlock(&spinlock->mutex);

	return 0;
}

int __spinlock_writeUnlock(Spinlock *spinlock, const char *file,
		const char *function, size_t line)
{
	int result = 0;
	int threadId = 0;

	if((spinlock == NULL) || (file == NULL) || (function == NULL) ||
			(line < 0)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&spinlock->mutex);

	if(spinlock->status != SPINLOCK_STATUS_LOCKED) {
		fprintf(stderr, "{%s}[%s():%i]->[%s():%i] error - unable to "
				"unlock in mode '%s'.\n",
				(char *)file, (char *)function, (int)line,
				__FUNCTION__, __LINE__,
				statusToString(spinlock->status));

		result = -1;
	}
	else {
		thread_self(&threadId);

		if(spinlock->threadCount != 0) {
			fprintf(stderr, "{%s}[%s():%i]->[%s():%i] error - detected %i "
					"spinning threads in mode '%s'.\n",
					(char *)file, (char *)function, (int)line,
					__FUNCTION__, __LINE__,
					spinlock->threadCount,
					statusToString(spinlock->status));

			spinlock->status = SPINLOCK_STATUS_ERROR;

			result = -1;
		}
		else if(spinlock->owner.threadId != threadId) {
			fprintf(stderr, "{%s}[%s():%i]->[%s():%i] error - thread %i "
					"requested unlock held by thread %i.\n",
					(char *)file, (char *)function, (int)line,
					__FUNCTION__, __LINE__,
					threadId,
					spinlock->owner.threadId);

			result = -1;
		}
		else {
			spinlock->owner.line = 0;
			spinlock->owner.threadId = 0;
			spinlock->owner.file = NULL;
			spinlock->owner.function = NULL;
			spinlock->owner.timestamp = 0.0;
			spinlock->status = SPINLOCK_STATUS_AVAILABLE;
		}
	}

	mutex_unlock(&spinlock->mutex);

	return result;
}

int __spinlock_readLock(Spinlock *spinlock, const char *file,
		const char *function, size_t line)
{
	int status = 0;
	int counter = 0;
	int threadCount = 0;
	double timestamp = 0.0;

	if((spinlock == NULL) || (file == NULL) || (function == NULL) ||
			(line < 0)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(EXPLICIT_ERRORS) {
		timestamp = time_getTimeMus();
	}

	while(counter < spinlock->spinThreshold) {
		mutex_lock(&spinlock->mutex);

		status = spinlock->status;
		threadCount = spinlock->threadCount;

		if(status == SPINLOCK_STATUS_AVAILABLE) {
			spinlock->threadCount += 1;
			mutex_unlock(&spinlock->mutex);
			return 0;
		}

		mutex_unlock(&spinlock->mutex);

		counter++;

		time_usleep(SPIN_SLEEP_MICROSECONDS);
	}

	fprintf(stderr, "{%s}[%s():%i]->[%s():%i] error - failed to "
			"request read-lock (%s) from {%s}[%s():%i] thread #%i after %0.6f "
			"seconds with %i spinners.\n",
			(char *)file, (char *)function, (int)line,
			__FUNCTION__, __LINE__,
			statusToString(status),
			spinlock->owner.file,
			spinlock->owner.function,
			spinlock->owner.line,
			spinlock->owner.threadId,
			time_getElapsedMusInSeconds(timestamp),
			threadCount);

	return -1;
}

int __spinlock_readUnlock(Spinlock *spinlock, const char *file,
		const char *function, size_t line)
{
	int result = 0;

	if((spinlock == NULL) || (file == NULL) || (function == NULL) ||
			(line < 0)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&spinlock->mutex);

	if((spinlock->status != SPINLOCK_STATUS_AVAILABLE) &&
			(spinlock->status != SPINLOCK_STATUS_WAITING)) {
		fprintf(stderr, "{%s}[%s():%i]->[%s():%i] error - unable to "
				"unlock readlock in mode '%s'.\n",
				(char *)file, (char *)function, (int)line,
				__FUNCTION__, __LINE__,
				statusToString(spinlock->status));

		result = -1;
	}
	else {
		spinlock->threadCount -= 1;

		if(spinlock->threadCount < 0) {
			fprintf(stderr, "{%s}[%s():%i]->[%s():%i] error - detected "
					"invalid spinner thread count (%i).\n",
					(char *)file, (char *)function, (int)line,
					__FUNCTION__, __LINE__,
					spinlock->threadCount);

			spinlock->threadCount = 0;
		}
	}

	mutex_unlock(&spinlock->mutex);

	return result;
}

int spinlock_getSpinThreadCount(Spinlock *spinlock, int *threadCount)
{
	if((spinlock == NULL) || (threadCount == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	*threadCount = 0;

	mutex_lock(&spinlock->mutex);

	*threadCount = spinlock->threadCount;

	mutex_unlock(&spinlock->mutex);

	return 0;
}

aboolean spinlock_isLocked(Spinlock *spinlock)
{
	aboolean result = afalse;

	if(spinlock == NULL) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	mutex_lock(&spinlock->mutex);

	if(spinlock->status == SPINLOCK_STATUS_LOCKED) {
		result = atrue;
	}

	mutex_unlock(&spinlock->mutex);

	return result;
}

int spinlock_getLockerInfo(Spinlock *spinlock, int *threadId, char **file,
		char **function, int *line, double *lockedSeconds)
{
	if((spinlock == NULL) || (threadId == NULL) || (file == NULL) ||
			(function == NULL) || (line == NULL) || (lockedSeconds == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	*threadId = 0;
	*file = NULL;
	*function = NULL;
	*line = 0;
	*lockedSeconds = 0.0;

	mutex_lock(&spinlock->mutex);

	*threadId = spinlock->owner.threadId;
	*file = spinlock->owner.file;
	*function = spinlock->owner.function;
	*line = spinlock->owner.line;
	*lockedSeconds = time_getElapsedMusInSeconds(spinlock->owner.timestamp);

	mutex_unlock(&spinlock->mutex);

	return 0;
}

