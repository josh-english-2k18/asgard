/*
 * stacktrace.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard system-stacktrace function library, header file.
 *
 * Written by Josh English.
 */

#if !defined(_CORE_SYSTEM_STACKTRACE_H)

#define _CORE_SYSTEM_STACKTRACE_H

#if !defined(_CORE_H) && !defined(_CORE_SYSTEM_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _CORE_H || _CORE_SYSTEM_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

// delcare stacktrace public functions

int stacktrace_getStack(char **stackString, int *stackStringLength);

#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _CORE_SYSTEM_STACKTRACE_H

