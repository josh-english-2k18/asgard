/*
 * common_apple.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The common header-file for Mac OS X systems.
 *
 * Written by Josh English.
 */

#if !defined(_CORE_COMMON_PLUGIN_COMMON_APPLE_H)

#define _CORE_COMMON_PLUGIN_COMMON_APPLE_H

#if !defined(_CORE_H) && !defined(_CORE_COMMON_PLUGIN_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _CORE_H || _CORE_COMMON_PLUGIN_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


#if defined(__APPLE__)

#include <dirent.h>
#include <dlfcn.h>
#include <errno.h>
#include <execinfo.h>
//#include <fcntl.h>
#include <limits.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
//#include <sys/utsname.h>

#include <arpa/inet.h>

#include <netinet/in.h>

#endif // __APPLE__


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _CORE_COMMON_PLUGIN_COMMON_APPLE_H

