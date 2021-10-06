/*
 * mutex.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard mutex interface.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#define _CORE_SYSTEM_COMPONENT
#include "core/system/mutex.h"
#define _CORE_SYSTEM_PLUGIN_COMPONENT
#include "core/system/plugin/mutex_linux.h"
#include "core/system/plugin/mutex_windows.h"

// define mutex public functions

int mutex_init(Mutex *mutex)
{
#if defined(__linux__) || defined(__APPLE__)
	return mutex_initLinux(mutex);
#elif defined(WIN32)
	return mutex_initWindows(mutex);
#else // - no plugin available -
	return -1;
#endif // - plugins -
}

int mutex_free(Mutex *mutex)
{
#if defined(__linux__) || defined(__APPLE__)
	return mutex_freeLinux(mutex);
#elif defined(WIN32)
	return mutex_freeWindows(mutex);
#else // - no plugin available -
	return -1;
#endif // - plugins -
}

int __mutex_lock(Mutex *mutex, const char *file, const char *function,
		size_t line)
{
#if defined(__linux__) || defined(__APPLE__)
	return mutex_lockLinux(mutex, file, function, line);
#elif defined(WIN32)
	return mutex_lockWindows(mutex, file, function, line);
#else // - no plugin available -
	return -1;
#endif // - plugins -
}

int __mutex_unlock(Mutex *mutex, const char *file, const char *function,
		size_t line)
{
#if defined(__linux__) || defined(__APPLE__)
	return mutex_unlockLinux(mutex, file, function, line);
#elif defined(WIN32)
	return mutex_unlockWindows(mutex, file, function, line);
#else // - no plugin available -
	return -1;
#endif // - plugins -
}

int mutex_getLockCount(Mutex *mutex, int *lockCount)
{
#if defined(__linux__) || defined(__APPLE__)
	return mutex_getLockCountLinux(mutex, lockCount);
#elif defined(WIN32)
	return mutex_getLockCountWindows(mutex, lockCount);
#else // - no plugin available -
	return -1;
#endif // - plugins -
}

int mutex_getLockerInfo(Mutex *mutex, char **file, char **function, int *line)
{
#if defined(__linux__) || defined(__APPLE__)
	return mutex_getLockerInfoLinux(mutex, file, function, line);
#elif defined(WIN32)
	return mutex_getLockerInfoWindows(mutex, file, function, line);
#else // - no plugin available -
	return -1;
#endif // - plugins -
}

