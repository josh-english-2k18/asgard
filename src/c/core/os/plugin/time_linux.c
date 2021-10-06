/*
 * time_linux.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard time library, Linux plugin.
 *
 * Written by Josh English.
 */


#if defined(__linux__) || defined(__APPLE__) || defined(MINGW)


// preprocessor directives

#include "core/core.h"
#define _CORE_OS_PLUGIN_COMPONENT
#include "core/os/plugin/time_linux.h"


// define time (linux plugin) private constants

#define SIZEOF_TIMESTAMP							(sizeof(char) * 32)


// define time (linux plugin) public functions

int time_getTimeSecondsLinux()
{
	int result = 0;

	result = (int)(time_getTimeMusLinux() / REAL_ONE_MILLION);

	return result;
}

double time_getTimeMusLinux()
{
	double result = 0.0;

	struct timeval timer;

	gettimeofday(&timer, (struct timezone *)NULL);

	result = ((REAL_ONE_MILLION * (double)timer.tv_sec) +
			(double)timer.tv_usec);

	return result;
}

int time_getElapsedSecondsLinux(int seconds)
{
	return (time_getTimeSecondsLinux() - seconds);
}

double time_getElapsedMusLinux(double microseconds)
{
	if(microseconds < 0.0) {
		DISPLAY_INVALID_ARGS;
		return 0.0;
	}

	return (time_getTimeMusLinux() - microseconds);
}

void time_getTimestampLinux(char timestamp[32])
{
	struct tm time;
	struct timeval localTime;

	gettimeofday(&localTime, (struct timezone *)NULL);
	localtime_r(&localTime.tv_sec, &time);

	memset(timestamp, 0, SIZEOF_TIMESTAMP);
	snprintf(timestamp, SIZEOF_TIMESTAMP,
		"%04i-%02i-%02i.%02i:%02i:%02i",
		(time.tm_year + 1900),
		(time.tm_mon + 1),
		time.tm_mday,
		time.tm_hour,
		time.tm_min,
		time.tm_sec);
}

void time_sleepLinux(int seconds)
{
	if(seconds < 0) {
		DISPLAY_INVALID_ARGS;
		return;
	}

#if !defined(MINGW)
	sleep(seconds);
#else // MINGW
	Sleep(seconds * 1000);
#endif // !MINGW
}

void time_usleepLinux(int microseconds)
{
	if(microseconds < 0) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	usleep(microseconds);
}

void time_nanosleepLinux(int nanoseconds)
{
	struct timespec spec;

	if(nanoseconds < 0) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	if(nanoseconds > 999999999) {
		spec.tv_sec = (time_t)(nanoseconds / 999999999);
		spec.tv_nsec = (long)(nanoseconds % 999999999);
	}
	else {
		spec.tv_sec = 0;
		spec.tv_nsec = (long)nanoseconds;
	}

#if !defined(MINGW)
	nanosleep(&spec, NULL);
#else // MINGW
	time_usleepLinux(nanoseconds / 1000);
#endif // !MINGW
}


#endif // __linux__ || __APPLE__ || MINGW

