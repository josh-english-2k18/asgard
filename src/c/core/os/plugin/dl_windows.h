/*
 * dl_windows.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard dynamic-library interface, Windows plugin, header file.
 *
 * Written by Josh English.
 */

#if !defined(_CORE_OS_PLUGIN_DL_WINDOWS_H)

#define _CORE_OS_PLUGIN_DL_WINDOWS_H

#if !defined(_CORE_H) && !defined(_CORE_OS_PLUGIN_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _CORE_H || _CORE_OS_PLUGIN_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

#if defined(WIN32)

// delcare dl (windows plugin) public functions

int dl_openWindows(char *filename, void **handle);

int dl_closeWindows(void *handle);

int dl_lookupWindows(void *handle, char *functionName, void **function);

#endif // WIN32

#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _CORE_OS_PLUGIN_DL_WINDOWS_H

