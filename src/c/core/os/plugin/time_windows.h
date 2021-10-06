/*
 * time_windows.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard time library, Windows plugin, header file.
 *
 * Written by Josh English.
 */

#if !defined(_CORE_OS_PLUGIN_TIME_WINDOWS_H)

#define _CORE_OS_PLUGIN_TIME_WINDOWS_H

#if !defined(_CORE_H) && !defined(_CORE_OS_PLUGIN_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _CORE_H || _CORE_OS_PLUGIN_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

#if defined(WIN32) && !defined(MINGW)

// delcare time (windows plugin) public functions

int time_getTimeSecondsWindows();

double time_getTimeMusWindows();

int time_getElapsedSecondsWindows(int seconds);

double time_getElapsedMusWindows(double microseconds);

void time_getTimestampWindows(char timestamp[32]);

void time_sleepWindows(int seconds);

void time_usleepWindows(int microseconds);

void time_nanosleepWindows(int nanoseconds);

#endif // WIN32 && !MINGW

#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _CORE_OS_PLUGIN_TIME_WINDOWS_H

