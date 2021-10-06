/*
 * stacktrace_android.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard system-stacktrace function library, Android plugin, header file.
 *
 * Written by Josh English.
 */

#if !defined(_CORE_SYSTEM_PLUGIN_STACKTRACE_ANDROID_H)

#define _CORE_SYSTEM_PLUGIN_STACKTRACE_ANDROID_H

#if !defined(_CORE_H) && !defined(_CORE_SYSTEM_PLUGIN_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _CORE_H || _CORE_SYSTEM_PLUGIN_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


#if defined(__ANDROID__)


// delcare stacktrace (android plugin) public functions

int stacktrace_getStackAndroid(char **stackString, int *stackStringLength);


#endif // __ANDROID__


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _CORE_SYSTEM_PLUGIN_STACKTRACE_ANDROID_H

