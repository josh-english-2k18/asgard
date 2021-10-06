/*
 * time_linux.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard time library, Linux plugin, header file.
 *
 * Written by Josh English.
 */

#if !defined(_CORE_OS_PLUGIN_TIME_LINUX_H)

#define _CORE_OS_PLUGIN_TIME_LINUX_H

#if !defined(_CORE_H) && !defined(_CORE_OS_PLUGIN_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _CORE_H || _CORE_OS_PLUGIN_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


#if defined(__linux__) || defined(__APPLE__) || defined(MINGW)


// delcare time (linux plugin) public functions

int time_getTimeSecondsLinux();

double time_getTimeMusLinux();

int time_getElapsedSecondsLinux(int seconds);

double time_getElapsedMusLinux(double microseconds);

void time_getTimestampLinux(char timestamp[32]);

void time_sleepLinux(int seconds);

void time_usleepLinux(int microseconds);

void time_nanosleepLinux(int nanoseconds);

#endif // __linux__ || __APPLE__ || MINGW


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _CORE_OS_PLUGIN_TIME_LINUX_H

