/*
 * mutex_linux.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard mutex interface, Linux plugin.
 *
 * Written by Josh English.
 */


#if defined(__linux__) || defined(__APPLE__)


// preprocessor directives

#define _CORE_COMMON_COMPONENT
#include "core/common/common.h"
#include "core/common/macros.h"
#include "core/common/version.h"
#include "core/common/string.h"
#define _CORE_OS_COMPONENT
#include "core/os/dl.h"
#include "core/os/time.h"
#include "core/os/types.h"
#define _CORE_SYSTEM_COMPONENT
#include "core/system/mutex.h"
#define _CORE_SYSTEM_PLUGIN_COMPONENT
#include "core/system/plugin/mutex_linux.h"


// define mutex (linux plugin) private functions

static int lockMutex(void *mutex)
{
	int rc = 0;

	if((rc = pthread_mutex_lock((pthread_mutex_t *)mutex)) != 0) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s():%i] error - failed to lock mutex 0x%lx "
					"with '%s'.\n", __FUNCTION__, __LINE__,
					(unsigned long int)mutex, strerror(rc));
		}
		return -1;
	}

	return 0;
}

static int unlockMutex(void *mutex)
{
	int rc = 0;

	if((rc = pthread_mutex_unlock((pthread_mutex_t *)mutex)) != 0) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s():%i] error - failed to unlock mutex 0x%lx "
					"with '%s'.\n", __FUNCTION__, __LINE__,
					(unsigned long int)mutex, strerror(rc));
		}
		return -1;
	}

	return 0;
}

static int checkMutexLock(Mutex *mutex, const char *file,
		const char *function, size_t line)
{
	if(lockMutex(mutex->dataLock) < 0) {
		return -1;
	}

	if(mutex->lockCount < 0) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "{%s}[%s():%i] error - detected mutex 0x%lx was "
					"misused, lock count is %i.\n", (char *)file,
					(char *)function, (int)line,
					(unsigned long int)mutex->lock, mutex->lockCount);
		}
		if(unlockMutex(mutex->dataLock) < 0) {
			return -1;
		}
		return -1;
	}

	if(unlockMutex(mutex->dataLock) < 0) {
		return -1;
	}

	return 0;
}

// define mutex (linux plugin) public functions

int mutex_initLinux(Mutex *mutex)
{
	int rc = 0;

	if(mutex == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	memset(mutex, 0, (sizeof(Mutex)));

	mutex->lockCount = 0;
	mutex->lockerLine = 0;
	mutex->lockerFile = "(none - unlocked)";
	mutex->lockerFunction = "(none - unlocked)";
	mutex->lock = NULL;
	mutex->dataLock = NULL;

	mutex->lock = (void *)malloc(sizeof(pthread_mutex_t));

	if((rc = pthread_mutex_init((pthread_mutex_t *)mutex->lock, NULL)) != 0) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s():%i] error - failed to initialize mutex "
					"0x%lx with '%s'.\n", __FUNCTION__, __LINE__,
					(unsigned long int)mutex->lock, strerror(rc));
		}
		return -1;
	}

	mutex->dataLock = (void *)malloc(sizeof(pthread_mutex_t));

	if((rc = pthread_mutex_init((pthread_mutex_t *)mutex->dataLock,
					NULL)) != 0) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s():%i] error - failed to initialize mutex "
					"0x%lx with '%s'.\n", __FUNCTION__, __LINE__,
					(unsigned long int)mutex->dataLock, strerror(rc));
		}
		return -1;
	}

	return 0;
}

int mutex_freeLinux(Mutex *mutex)
{
	int rc = 0;
	int counter = 0;

	if((mutex == NULL) || (mutex->lock == NULL) ||
			(mutex->dataLock == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(checkMutexLock(mutex, __FILE__, __FUNCTION__, __LINE__) < 0) {
		return -1;
	}

	// wait for a while if the mutex is locked to protect against the undefined
	// behavior where destroy() is called on a locked mutex

	rc = pthread_mutex_trylock((pthread_mutex_t *)mutex->lock);
	while((rc == EBUSY) && (counter < 1024)) {
		time_usleep(1024);
		rc = pthread_mutex_trylock((pthread_mutex_t *)mutex->lock);
	}

	if(rc == EBUSY) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s():%i] error - unable to free mutex 0x%lx "
					"when in locked state.\n", __FUNCTION__, __LINE__,
					(unsigned long int)mutex->lock);
		}
		return -1;
	}

	if(unlockMutex(mutex->lock) < 0) {
		return -1;
	}

	if((rc = pthread_mutex_destroy((pthread_mutex_t *)mutex->lock) != 0)) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s():%i] error - failed to free mutex 0x%lx "
					"with '%s'.\n", __FUNCTION__, __LINE__,
					(unsigned long int)mutex->lock, strerror(rc));
		}
		return -1;
	}

	if((rc = pthread_mutex_destroy((pthread_mutex_t *)mutex->dataLock) != 0)) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s():%i] error - failed to free mutex 0x%lx "
					"with '%s'.\n", __FUNCTION__, __LINE__,
					(unsigned long int)mutex->dataLock, strerror(rc));
		}
		return -1;
	}

	free(mutex->lock);
	free(mutex->dataLock);

	memset(mutex, 0, (sizeof(Mutex)));

	return 0;
}

int mutex_lockLinux(Mutex *mutex, const char *file, const char *function,
		size_t line)
{
	if((mutex == NULL) || (mutex->lock == NULL) ||
			(mutex->dataLock == NULL)) {
		fprintf(stderr, "[%s():%i] error - invalid or missing argument(s) "
				"from {%s}[%s():%i].\n", __FUNCTION__, __LINE__, (char *)file,
				(char *)function, (int)line);
		return -1;
	}

	// check and update mutex lock status

	if(lockMutex(mutex->dataLock) < 0) {
		return -1;
	}

	if(mutex->lockCount < 0) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "{%s}[%s():%i] error - detected mutex 0x%lx was "
					"misused, lock count is %i.\n", (char *)file,
					(char *)function, (int)line,
					(unsigned long int)mutex->lock, mutex->lockCount);
		}
		if(unlockMutex(mutex->dataLock) < 0) {
			return -1;
		}
		return -1;
	}

	mutex->lockCount += 1;

	if(unlockMutex(mutex->dataLock) < 0) {
		return -1;
	}

	// lock the mutex

	if(lockMutex(mutex->lock) < 0) {
		return -1;
	}

	// update the mutex usage information

	mutex->lockerLine = (int)line;
	mutex->lockerFile = (char *)file;
	mutex->lockerFunction = (char *)function;

	if(EXPLICIT_ERRORS) {
		mutex->lockerTime = time_getTimeMus();
	}

	return 0;
}

int mutex_unlockLinux(Mutex *mutex, const char *file, const char *function,
		size_t line)
{
	double elapsedTime = 0.0;

	if((mutex == NULL) || (mutex->lock == NULL) ||
			(mutex->dataLock == NULL)) {
		fprintf(stderr, "[%s():%i] error - invalid or missing argument(s) "
				"from {%s}[%s():%i].\n", __FUNCTION__, __LINE__, (char *)file,
				(char *)function, (int)line);
		return -1;
	}

	// check the lock status

	if(lockMutex(mutex->dataLock) < 0) {
		return -1;
	}

	mutex->lockCount -= 1;

	if(mutex->lockCount < 0) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "{%s}[%s():%i] error - detected mutex 0x%lx was "
					"misused, lock count is %i.\n", (char *)file,
					(char *)function, (int)line,
					(unsigned long int)mutex->lock, mutex->lockCount);
		}
		if(unlockMutex(mutex->dataLock) < 0) {
			return -1;
		}
		return -1;
	}

	if(EXPLICIT_ERRORS) {
		if((mutex->lockCount > 0) &&
				(strcmp(mutex->lockerFunction, "socket_accept"))) {
			elapsedTime = time_getElapsedMusInSeconds(mutex->lockerTime);
			if(elapsedTime > 0.1) {
				fprintf(stderr, "{%s}[%s():%i] note - mutex 0x%lx locked "
						"from {%s}[%s():%i] locked for %0.2f seconds with %i "
						"threads waiting.\n", (char *)file, (char *)function,
						(int)line, (unsigned long int)mutex->lock,
						mutex->lockerFile, mutex->lockerFunction,
						mutex->lockerLine, elapsedTime, mutex->lockCount);
			}
		}
	}

	if(unlockMutex(mutex->dataLock) < 0) {
		return -1;
	}

	// unlock the mutex

	if(unlockMutex(mutex->lock) < 0) {
		return -1;
	}

	return 0;
}

int mutex_getLockCountLinux(Mutex *mutex, int *lockCount)
{
	if((mutex == NULL) || (mutex->lock == NULL) ||
			(mutex->dataLock == NULL) || (lockCount == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(checkMutexLock(mutex, __FILE__, __FUNCTION__, __LINE__) < 0) {
		return -1;
	}

	*lockCount = mutex->lockCount;

	return 0;
}

int mutex_getLockerInfoLinux(Mutex *mutex, char **file, char **function,
		int *line)
{
	if((mutex == NULL) || (mutex->lock == NULL) ||
			(mutex->dataLock == NULL) || (file == NULL) ||
			(function == NULL) || (line == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(checkMutexLock(mutex, __FILE__, __FUNCTION__, __LINE__) < 0) {
		return -1;
	}

	*file = mutex->lockerFile;
	*function = mutex->lockerFunction;
	*line = mutex->lockerLine;

	return 0;
}


#endif // __linux__ || __APPLE__

