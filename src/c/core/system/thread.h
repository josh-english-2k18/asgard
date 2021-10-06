/*
 * thread.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard thread interface, header file.
 *
 * Written by Josh English.
 */

#if !defined(_CORE_SYSTEM_THREAD_H)

#define _CORE_SYSTEM_THREAD_H

#if !defined(_CORE_H) && !defined(_CORE_SYSTEM_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _CORE_H || _CORE_SYSTEM_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

// define thread public data types

typedef void *(ThreadFunction)(void *argument);

typedef struct _Thread {
	void *handle;
	Mutex mutex;
} Thread;

// delcare thread public functions

int thread_init(Thread *thread);

int thread_create(Thread *thread, ThreadFunction function, void *argument);

int thread_join(Thread *thread, void **exitValue);

int thread_self(int *threadId);

#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _CORE_SYSTEM_THREAD_H

