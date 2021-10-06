/*
 * thread.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard thread interface.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#define _CORE_SYSTEM_COMPONENT
#include "core/system/thread.h"
#define _CORE_SYSTEM_PLUGIN_COMPONENT
#include "core/system/plugin/thread_linux.h"
#include "core/system/plugin/thread_windows.h"

// define thread public functions

int thread_init(Thread *thread)
{
#if defined(__linux__) || defined(__APPLE__)
	return thread_initLinux(thread);
#elif defined(WIN32)
	return thread_initWindows(thread);
#else // - no plugin available -
	return -1;
#endif // - plugins -
}

int thread_create(Thread *thread, ThreadFunction function, void *argument)
{
#if defined(__linux__) || defined(__APPLE__)
	return thread_createLinux(thread, function, argument);
#elif defined(WIN32)
	return thread_createWindows(thread, function, argument);
#else // - no plugin available -
	return -1;
#endif // - plugins -
}

int thread_join(Thread *thread, void **exitValue)
{
#if defined(__linux__) || defined(__APPLE__)
	return thread_joinLinux(thread, exitValue);
#elif defined(WIN32)
	return thread_joinWindows(thread, exitValue);
#else // - no plugin available -
	return -1;
#endif // - plugins -
}

int thread_self(int *threadId)
{
#if defined(__linux__) || defined(__APPLE__)
	return thread_selfLinux(threadId);
#elif defined(WIN32)
	return thread_selfWindows(threadId);
#else // - no plugin available -
	return -1;
#endif // - plugins -
}

