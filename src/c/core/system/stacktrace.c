/*
 * stacktrace.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard system-stacktrace function library.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#define _CORE_SYSTEM_COMPONENT
#include "core/system/stacktrace.h"
#define _CORE_SYSTEM_PLUGIN_COMPONENT
#include "core/system/plugin/stacktrace_android.h"
#include "core/system/plugin/stacktrace_linux.h"
#include "core/system/plugin/stacktrace_windows.h"

// define stacktrace public functions

int stacktrace_getStack(char **stackString, int *stackStringLength)
{
#if (defined(__linux__) || defined(__APPLE__)) && !defined(__ANDROID__)
	return stacktrace_getStackLinux(stackString, stackStringLength);
#elif defined(WIN32)
	return stacktrace_getStackWindows(stackString, stackStringLength);
#elif defined(__ANDROID__)
	return stacktrace_getStackAndroid(stackString, stackStringLength);
#else // - no plugin available -
	return -1;
#endif // - plugins -
}

