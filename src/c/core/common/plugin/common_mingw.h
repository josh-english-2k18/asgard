/*
 * common_mingw.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The common header-file for the MinGW compiler.
 *
 * Written by Josh English.
 */

#if !defined(_CORE_COMMON_PLUGIN_COMMON_MINGW_H)

#define _CORE_COMMON_PLUGIN_COMMON_MINGW_H

#if !defined(_CORE_H) && !defined(_CORE_COMMON_PLUGIN_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _CORE_H || _CORE_COMMON_PLUGIN_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


#if defined(MINGW)

// include header files

#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>

#include <direct.h>

// define macros

#define fseeko(file, offset, whence) \
	fseeko64(file, offset, whence)

#define ftello(file) \
	ftello64(file)

#endif // MINGW


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _CORE_COMMON_PLUGIN_COMMON_MINGW_H

