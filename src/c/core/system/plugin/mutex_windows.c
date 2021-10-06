/*
 * mutex_windows.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard mutex interface, Windows plugin.
 *
 * Written by Josh English.
 */

#if defined(WIN32)

// preprocessor directives

#include "core/core.h"
#define _CORE_SYSTEM_PLUGIN_COMPONENT
#include "core/system/plugin/mutex_windows.h"

// define mutex (windows plugin) private functions

static int lockMutex(void *mutex)
{
	int rc = 0;

	rc = WaitForSingleObject((HANDLE)mutex, INFINITE);
	if((rc != WAIT_OBJECT_0) && (rc != WAIT_ABANDONED)) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s():%i] error - failed to lock mutex 0x%llx "
					"with '%i'.\n", __FUNCTION__, __LINE__,
					(aulint)mutex, rc);
		}
		return -1;
	}

	return 0;
}

static int unlockMutex(void *mutex)
{
	int rc = 0;

	if((rc = ReleaseMutex((HANDLE)mutex)) != 1) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s():%i] error - failed to unlock mutex 0x%llx "
					"with '%s'.\n", __FUNCTION__, __LINE__,
					(aulint)mutex, rc);
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
			fprintf(stderr, "{%s}[%s():%i] error - detected mutex 0x%llx was "
					"misused, lock count is %i.\n", (char *)file,
					(char *)function, (int)line,
					(aulint)mutex->lock, mutex->lockCount);
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

// define mutex (windows plugin) public functions

int mutex_initWindows(Mutex *mutex)
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

	if((mutex->lock = (void *)CreateMutexA(NULL, 0, NULL)) == NULL) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s():%i] error - failed to initialize mutex "
					"0x%llx.\n", __FUNCTION__, __LINE__,
					(aulint)mutex);
		}
		return -1;
	}

	if((mutex->dataLock = (void *)CreateMutexA(NULL, 0, NULL)) == NULL) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s():%i] error - failed to initialize mutex "
					"0x%llx.\n", __FUNCTION__, __LINE__,
					(aulint)mutex);
		}
		return -1;
	}

	return 0;
}

int mutex_freeWindows(Mutex *mutex)
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

	if((rc = CloseHandle((HANDLE)mutex->lock) != 1)) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s():%i] error - failed to free mutex 0x%llx "
					"with %i.\n", __FUNCTION__, __LINE__,
					(aulint)mutex->lock, rc);
		}
		return -1;
	}

	if((rc = CloseHandle((HANDLE)mutex->dataLock) != 1)) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s():%i] error - failed to free mutex 0x%llx "
					"with %i.\n", __FUNCTION__, __LINE__,
					(aulint)mutex->lock, rc);
		}
		return -1;
	}

	memset(mutex, 0, (sizeof(Mutex)));

	return 0;
}

int mutex_lockWindows(Mutex *mutex, const char *file, const char *function,
		size_t line)
{
	if((mutex == NULL) || (mutex->lock == NULL) ||
			(mutex->dataLock == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	// check and update mutex lock status

	if(lockMutex(mutex->dataLock) < 0) {
		return -1;
	}

	if(mutex->lockCount < 0) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "{%s}[%s():%i] error - detected mutex 0x%llx was "
					"misused, lock count is %i.\n", (char *)file,
					(char *)function, (int)line,
					(aulint)mutex->lock, mutex->lockCount);
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

int mutex_unlockWindows(Mutex *mutex, const char *file, const char *function,
		size_t line)
{
	double elapsedTime = 0.0;

	if((mutex == NULL) || (mutex->lock == NULL) ||
			(mutex->dataLock == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	// check the lock status

	if(lockMutex(mutex->dataLock) < 0) {
		return -1;
	}

	mutex->lockCount -= 1;

	if(mutex->lockCount < 0) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "{%s}[%s():%i] error - detected mutex 0x%llx was "
					"misused, lock count is %i.\n", (char *)file,
					(char *)function, (int)line,
					(aulint)mutex->lock, mutex->lockCount);
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
				fprintf(stderr, "{%s}[%s():%i] note - mutex 0x%llx locked "
						"from {%s}[%s():%i] locked for %0.2f seconds with %i "
						"threads waiting.\n", (char *)file, (char *)function,
						(int)line, (aulint)mutex->lock,
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

int mutex_getLockCountWindows(Mutex *mutex, int *lockCount)
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

int mutex_getLockerInfoWindows(Mutex *mutex, char **file, char **function,
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

#endif // WIN32

