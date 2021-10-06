/*
 * mutex_windows.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard mutex interface, Windows plugin, header file.
 *
 * Written by Josh English.
 */

#if !defined(_CORE_SYSTEM_PLUGIN_MUTEX_WINDOWS_H)

#define _CORE_SYSTEM_PLUGIN_MUTEX_WINDOWS_H

#if !defined(_CORE_H) && !defined(_CORE_SYSTEM_PLUGIN_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _CORE_H || _CORE_SYSTEM_PLUGIN_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

#if defined(WIN32)

// delcare mutex (windows plugin) public functions

int mutex_initWindows(Mutex *mutex);

int mutex_freeWindows(Mutex *mutex);

int mutex_lockWindows(Mutex *mutex, const char *file, const char *function,
		size_t line);

int mutex_unlockWindows(Mutex *mutex, const char *file, const char *function,
		size_t line);

int mutex_getLockCountWindows(Mutex *mutex, int *lockCount);

int mutex_getLockerInfoWindows(Mutex *mutex, char **file, char **function,
		int *line);

#endif // WIN32

#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _CORE_SYSTEM_PLUGIN_MUTEX_WINDOWS_H

