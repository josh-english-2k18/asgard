/*
 * thread_windows.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard thread interface, Windows plugin, header file.
 *
 * Written by Josh English.
 */

#if !defined(_CORE_SYSTEM_PLUGIN_THREAD_WINDOWS_H)

#define _CORE_SYSTEM_PLUGIN_THREAD_WINDOWS_H

#if !defined(_CORE_H) && !defined(_CORE_SYSTEM_PLUGIN_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _CORE_H || _CORE_SYSTEM_PLUGIN_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

#if defined(WIN32)

// delcare thread (windows plugin) public functions

int thread_initWindows(Thread *thread);

int thread_createWindows(Thread *thread, ThreadFunction function,
		void *argument);

int thread_joinWindows(Thread *thread, void **exitValue);

int thread_selfWindows(int *threadId);

#endif // WIN32

#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _CORE_SYSTEM_PLUGIN_THREAD_WINDOWS_H

