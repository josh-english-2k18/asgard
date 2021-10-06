/*
 * time.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard time library.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#define _CORE_OS_COMPONENT
#include "core/os/time.h"
#define _CORE_OS_PLUGIN_COMPONENT
#include "core/os/plugin/time_linux.h"
#include "core/os/plugin/time_windows.h"

// define time public functions

int time_getTimeSeconds()
{
#if defined(__linux__) || defined(__APPLE__) || defined(MINGW)
	return time_getTimeSecondsLinux();
#elif defined(WIN32)
	return time_getTimeSecondsWindows();
#else // - no plugin available -
	return -1;
#endif // - plugins -
}

double time_getTimeMus()
{
#if defined(__linux__) || defined(__APPLE__) || defined(MINGW)
	return time_getTimeMusLinux();
#elif defined(WIN32)
	return time_getTimeMusWindows();
#else // - no plugin available -
	return -1.0;
#endif // - plugins -
}

int time_getElapsedSeconds(int seconds)
{
#if defined(__linux__) || defined(__APPLE__) || defined(MINGW)
	return time_getElapsedSecondsLinux(seconds);
#elif defined(WIN32)
	return time_getElapsedSecondsWindows(seconds);
#else // - no plugin available -
	return -1;
#endif // - plugins -
}

double time_getElapsedMus(double microseconds)
{
#if defined(__linux__) || defined(__APPLE__) || defined(MINGW)
	return time_getElapsedMusLinux(microseconds);
#elif defined(WIN32)
	return time_getElapsedMusWindows(microseconds);
#else // - no plugin available -
	return -1.0;
#endif // - plugins -
}

void time_getTimestamp(char timestamp[32])
{
#if defined(__linux__) || defined(__APPLE__) || defined(MINGW)
	time_getTimestampLinux(timestamp);
#elif defined(WIN32)
	time_getTimestampWindows(timestamp);
#else // - no plugin available -
	return;
#endif // - plugins -
}

void time_sleep(int seconds)
{
#if defined(__linux__) || defined(__APPLE__) || defined(MINGW)
	time_sleepLinux(seconds);
#elif defined(WIN32)
	time_sleepWindows(seconds);
#else // - no plugin available -
	return;
#endif // - plugins -
}

void time_usleep(int microseconds)
{
#if defined(__linux__) || defined(__APPLE__) || defined(MINGW)
	time_usleepLinux(microseconds);
#elif defined(WIN32)
	time_usleepWindows(microseconds);
#else // - no plugin available -
	return;
#endif // - plugins -
}

void time_nanosleep(int nanoseconds)
{
#if defined(__linux__) || defined(__APPLE__) || defined(MINGW)
	time_nanosleepLinux(nanoseconds);
#elif defined(WIN32)
	time_nanosleepWindows(nanoseconds);
#else // - no plugin available -
	return;
#endif // - plugins -
}

