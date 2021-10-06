/*
 * common_windows.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The common header-file for Windows systems.
 *
 * Written by Josh English.
 */

#if !defined(_CORE_COMMON_PLUGIN_COMMON_WINDOWS_H)

#define _CORE_COMMON_PLUGIN_COMMON_WINDOWS_H

#if !defined(_CORE_H) && !defined(_CORE_COMMON_PLUGIN_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _CORE_H || _CORE_COMMON_PLUGIN_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


#if defined(WIN32) && !defined(MINGW)

// include header files

#include <windows.h>
#include <direct.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include "core/common/plugin/dirent_windows.h"


// define constants

#define VERSION							0.9.34
#define VERSION_DATE					2012.04.30

#define MEMCACHE_BYPASS					1


// define macros

#define snprintf(value, length, ...) \
	_snprintf_s((char *)value, (size_t)length, (size_t)length, ##__VA_ARGS__)

#define fseeko(file, offset, whence) \
	_fseeki64(file, offset, whence)

#define ftello(file) _ftelli64(file)

#endif // WIN32 && !MINGW


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _CORE_COMMON_PLUGIN_COMMON_WINDOWS_H

