/*
 * stacktrace_linux.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard system-stacktrace function library, Linux plugin, header file.
 *
 * Written by Josh English.
 */

#if !defined(_CORE_SYSTEM_PLUGIN_STACKTRACE_LINUX_H)

#define _CORE_SYSTEM_PLUGIN_STACKTRACE_LINUX_H

#if !defined(_CORE_H) && !defined(_CORE_SYSTEM_PLUGIN_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _CORE_H || _CORE_SYSTEM_PLUGIN_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


#if (defined(__linux__) || defined(__APPLE__)) && !defined(__ANDROID__)


// delcare stacktrace (linux plugin) public functions

int stacktrace_getStackLinux(char **stackString, int *stackStringLength);


#endif // (__linux__ || __APPLE__) && !__ANDROID__


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _CORE_SYSTEM_PLUGIN_STACKTRACE_LINUX_H

