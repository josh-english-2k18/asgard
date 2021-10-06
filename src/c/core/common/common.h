/*
 * common.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The master common header-file for Asgard.
 *
 * Written by Josh English.
 */

#if !defined(_CORE_COMMON_COMMON_H)

#define _CORE_COMMON_COMMON_H

#if !defined(_CORE_H) && !defined(_CORE_COMMON_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _CORE_H || _CORE_COMMON_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// gcc library directives

#if defined(__GNUC__)

#	if !defined(_REENTRANT)
#		define _REENTRANT
#	endif // _REENTRANT

#	if !defined(_THREAD_SAFE)
#		define _THREAD_SAFE
#	endif // _THREAD_SAFE

#endif // __GNUC__


// setup standard includes

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include "zlib/zlib.h"

#define _CORE_COMMON_PLUGIN_COMPONENT
#include "core/common/plugin/common_android.h"
#include "core/common/plugin/common_apple.h"
#include "core/common/plugin/common_linux.h"
#include "core/common/plugin/common_mingw.h"
#include "core/common/plugin/common_windows.h"


// compiler-specific directives

// msvc directives

#if defined(_MSC_VER)

#pragma once

#if !defined(WINVER)
#	define WINVER			0x0501			// windows XP or later
#endif

#if !defined(_WIN32_WINNT)
#	define _WIN32_WINNT		0x0501			// windows XP or later
#endif						

#if !defined(_WIN32_WINDOWS)
#	define _WIN32_WINDOWS	0x0410			// windows 98 or later
#endif

#if !defined(_WIN32_IE)
#	define _WIN32_IE		0x0600			// IE 6.0 or later
#endif

#define WIN32_LEAN_AND_MEAN

#endif // _MSC_VER


// define common function library constants

#define COMMON_STD_STREAM_ENTRY_LENGTH				1024


// define common function library data types

typedef struct _StdStream {
	int errRef;
	int outRef;
	char *errEntry[COMMON_STD_STREAM_ENTRY_LENGTH];
	char *outEntry[COMMON_STD_STREAM_ENTRY_LENGTH];
} StdStream;


// define common function library macros

#if !defined(_CORE_COMMON_BYPASS_STD_STREAM)

#define printf(format, ...) \
	common_logStdStream(stdout, format, ##__VA_ARGS__)

#define fprintf(stream, format, ...) \
	common_logStdStream(stream, format, ##__VA_ARGS__)

#endif // _CORE_COMMON_BYPASS_STD_STREAM


// declare common function library public functions

void common_initStdStream();

void common_freeStdStream(void);

StdStream *common_getStdStream();

void common_logStdStream(void *stream, const char *format, ...);

void common_logStdStreamError(char *buffer);

void common_logStdStreamOutput(char *buffer);

char *common_popStdStreamError();

char *common_popStdStreamOutput();

void common_display(void *stream, char *string, int stringLength);


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _CORE_COMMON_COMMON_H

