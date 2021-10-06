/*
 * thread_linux.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard thread interface, Linux plugin.
 *
 * Written by Josh English.
 */


#if defined(__linux__) || defined(__APPLE__)


// preprocessor directives

#include "core/core.h"
#define _CORE_SYSTEM_PLUGIN_COMPONENT
#include "core/system/plugin/thread_linux.h"


// define thread (linux plugin) public functions

int thread_initLinux(Thread *thread)
{
	if(thread == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	memset(thread, 0, sizeof(Thread));

	if(mutex_init(&thread->mutex) < 0) {
		return -1;
	}

	thread->handle = NULL;

	return 0;
}

int thread_createLinux(Thread *thread, ThreadFunction function, void *argument)
{
	int rc = 0;

	if((thread == NULL) || (function == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&thread->mutex);

	if(thread->handle == NULL) {
		thread->handle = (pthread_t *)malloc(sizeof(pthread_t));
	}

	if((rc = pthread_create((pthread_t *)thread->handle, NULL, function,
					argument)) != 0) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s():%i] error - failed to create thread 0x%lx "
					"with '%s'.\n", __FUNCTION__, __LINE__,
					(unsigned long int)thread->handle, strerror(rc));
		}
		mutex_unlock(&thread->mutex);
		return -1;
	}

	mutex_unlock(&thread->mutex);

	return 0;
}

int thread_joinLinux(Thread *thread, void **exitValue)
{
	int rc = 0;
	int result = 0;

	if(thread == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&thread->mutex);

	if(thread->handle != NULL) {
		if((rc = pthread_join(*((pthread_t *)thread->handle),
						exitValue)) != 0) {
			if(EXPLICIT_ERRORS) {
				fprintf(stderr, "[%s():%i] error - failed to join thread 0x%lx "
						"with '%s'.\n", __FUNCTION__, __LINE__,
						(unsigned long int)thread->handle, strerror(rc));
			}
			result = -1;
		}

		free(thread->handle);
	}

	mutex_unlock(&thread->mutex);

	if(mutex_free(&thread->mutex) < 0) {
		result = -1;
	}

	memset(thread, 0, sizeof(Thread));

	return result;
}

int thread_selfLinux(int *threadId)
{
	if(threadId == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

#if defined(__APPLE__)
	*threadId = (int)((unsigned long int)pthread_self());
#else // !__APPLE__
	*threadId = (int)pthread_self();
#endif // __APPLE__

	return 0;
}


#endif // __linux__ || __APPLE__

