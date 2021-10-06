/*
 * dl.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard dynamic-library interface.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#define _CORE_OS_COMPONENT
#include "core/os/dl.h"
#define _CORE_OS_PLUGIN_COMPONENT
#include "core/os/plugin/dl_linux.h"
#include "core/os/plugin/dl_windows.h"

// define dl public functions

int dl_open(char *filename, void **handle)
{
#if defined(__linux__) || defined(__APPLE__)
	return dl_openLinux(filename, handle);
#elif defined(WIN32)
	return dl_openWindows(filename, handle);
#else // - no plugin available -
	return -1;
#endif // - plugins -
}

int dl_close(void *handle)
{
#if defined(__linux__) || defined(__APPLE__)
	return dl_closeLinux(handle);
#elif defined(WIN32)
	return dl_closeWindows(handle);
#else // - no plugin available -
	return -1;
#endif // - plugins -
}

int dl_lookup(void *handle, char *functionName, void **function)
{
#if defined(__linux__) || defined(__APPLE__)
	return dl_lookupLinux(handle, functionName, function);
#elif defined(WIN32)
	return dl_lookupWindows(handle, functionName, function);
#else // - no plugin available -
	return -1;
#endif // - plugins -
}

