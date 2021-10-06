/*
 * mutex_linux.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard mutex interface, Linux plugin, header file.
 *
 * Written by Josh English.
 */

#if !defined(_CORE_SYSTEM_PLUGIN_MUTEX_LINUX_H)

#define _CORE_SYSTEM_PLUGIN_MUTEX_LINUX_H

#if !defined(_CORE_H) && !defined(_CORE_SYSTEM_PLUGIN_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _CORE_H || _CORE_SYSTEM_PLUGIN_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


#if defined(__linux__) || defined(__APPLE__)


// delcare mutex (linux plugin) public functions

int mutex_initLinux(Mutex *mutex);

int mutex_freeLinux(Mutex *mutex);

int mutex_lockLinux(Mutex *mutex, const char *file, const char *function,
		size_t line);

int mutex_unlockLinux(Mutex *mutex, const char *file, const char *function,
		size_t line);

int mutex_getLockCountLinux(Mutex *mutex, int *lockCount);

int mutex_getLockerInfoLinux(Mutex *mutex, char **file, char **function,
		int *line);


#endif // __linux__ || __APPLE__


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _CORE_SYSTEM_PLUGIN_MUTEX_LINUX_H

