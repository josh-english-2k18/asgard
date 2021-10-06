/*
 * thread_windows.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard thread interface, Windows plugin.
 *
 * Written by Josh English.
 */

#if defined(WIN32)

// preprocessor directives

#include "core/core.h"
#define _CORE_SYSTEM_PLUGIN_COMPONENT
#include "core/system/plugin/thread_windows.h"

// declare thread (windows plugin) private data types

typedef struct _ThreadWindows {
	int handleLength;
	void **handles;
} ThreadWindows;


// declare thread (windows plugin) private functions

static char *iToString(int value);


// define thread (windows plugin) private functions

static char *iToString(int value)
{
	char *result = NULL;

	result = (char *)malloc(sizeof(char) * (16 + 1));

	snprintf(result, 16, "%i", value);

	return result;
}


// define thread (windows plugin) public functions

int thread_initWindows(Thread *thread)
{
	ThreadWindows *context = NULL;

	if(thread == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	memset(thread, 0, sizeof(Thread));

	if(mutex_init(&thread->mutex) < 0) {
		return -1;
	}

	context = (ThreadWindows *)malloc(sizeof(ThreadWindows));
	context->handleLength = 0;
	context->handles = NULL;

	thread->handle = (void *)context;

	return 0;
}

int thread_createWindows(Thread *thread, ThreadFunction function,
		void *argument)
{
	int ref = 0;
	void *handle = NULL;

	ThreadWindows *context = NULL;

	DWORD threadId = (DWORD)0;

	if((thread == NULL) || (thread->handle == NULL) || (function == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&thread->mutex);

	context = (ThreadWindows *)thread->handle;

	handle = CreateThread(
			(LPSECURITY_ATTRIBUTES)NULL,
			(SIZE_T)0,
			(LPTHREAD_START_ROUTINE)function,
			(LPVOID)argument,
			(DWORD)0,
			&threadId);
	if(handle == NULL) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s():%i] error - failed to create thread 0x%llx "
					"with %i.\n", __FUNCTION__, __LINE__,
					(aulint)handle, (int)GetLastError());
		}
		mutex_unlock(&thread->mutex);
		return -1;
	}

	if(context->handles == NULL) {
		ref = 0;
		context->handleLength = 1;
		context->handles = (void **)malloc(sizeof(void *) *
				context->handleLength);
	}
	else {
		ref = context->handleLength;
		context->handleLength += 1;
		context->handles = (void **)realloc(context->handles,
				(sizeof(void *) * context->handleLength));
	}

	context->handles[ref] = handle;

	mutex_unlock(&thread->mutex);

	return 0;
}

int thread_joinWindows(Thread *thread, void **exitValue)
{
	int ii = 0;
	char *string = NULL;
	char *buffer = NULL;

	ThreadWindows *context = NULL;

	DWORD result = (DWORD)0;
	DWORD exitCode = (DWORD)0;

	if(thread == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&thread->mutex);

	context = (ThreadWindows *)thread->handle;

	if(context->handles != NULL) {
		for(ii = 0; ii < context->handleLength; ii++) {
			result = WaitForSingleObject(context->handles[ii], INFINITE);
			if(result != WAIT_OBJECT_0) {
				fprintf(stderr, "[%s():%i] error - WaitForSingleObject() on "
						"0x%llx returned %i (expecting %i).\n",
						__FUNCTION__, __LINE__,
						(aulint)thread, (int)result, (int)WAIT_OBJECT_0);
				exitCode = -1;
			}

			if(GetExitCodeThread(context->handles[ii], &exitCode) != TRUE) {
				fprintf(stderr, "[%s():%i] error - GetExitCodeThread() on "
						"0x%llx failed.\n",
						__FUNCTION__, __LINE__, (aulint)thread);
				exitCode = -2;
			}

			if(CloseHandle(context->handles[ii]) == (BOOL)0) {
				fprintf(stderr, "[%s():%i] error - CloseHandle() on 0x%llx "
						"failed.\n", __FUNCTION__, __LINE__, (aulint)thread);
				return -1;
			}

			if(exitValue != NULL) {
				string = iToString(exitCode);
				if(buffer == NULL) {
					buffer = string;
				}
				else {
					strcat(buffer, ",");
					strcat(buffer, string);
					free(string);
				}
			}
		}

		free(context->handles);
	}

	if(exitValue != NULL) {
		*exitValue = (void *)buffer;
	}

	mutex_unlock(&thread->mutex);

	if(mutex_free(&thread->mutex) < 0) {
		result = -1;
	}

	memset(thread, 0, sizeof(Thread));

	return result;
}

int thread_selfWindows(int *threadId)
{
	if(threadId == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	*threadId = (int)GetCurrentThreadId();

	return 0;
}

#endif // WIN32

