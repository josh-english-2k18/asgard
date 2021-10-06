/*
 * time_windows.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard time library, Windows plugin.
 *
 * Written by Josh English.
 */

#if defined(WIN32) && !defined(MINGW)

// preprocessor directives

#include "core/core.h"
#define _CORE_OS_PLUGIN_COMPONENT
#include "core/os/plugin/time_windows.h"

// define time (windows plugin) private constants

#define SIZEOF_TIMESTAMP			(sizeof(char) * 32)

#define YEAR0						1900

#define EPOCH_YR					1970

#define SECS_DAY					(24L * 60L * 60L)

#define LEAPYEAR(year)				\
	(!((year) % 4) && (((year) % 100) || !((year) % 400)))

#define YEARSIZE(year)				(LEAPYEAR(year) ? 366 : 365)

#define CONSTANT_64BIT(x)			x##i64

//#define TIME_BASE_DIFF				CONSTANT_64BIT(116444736000000000)
#define TIME_BASE_DIFF				116444736000000000ull

const unsigned long YEAR_TABLE[2][12] = {
	{	31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31	},
	{	31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31	}
};

// define time (windows plugin) private data types

struct timezone {
	int tz_minuteswest;
	int tz_dsttime;
};

struct tm
{
	int tm_sec;
	int tm_min;
	int tm_hour;
	int tm_mday;
	int tm_mon;
	int tm_year;
	int tm_wday;
	int tm_yday;
	int tm_isdst;
};

// define time (windows plugin) private static global variables

static SOCKET socketHandle = (SOCKET)NULL;

// define time (windows plugin) private functions

static int gettimeofday(struct timeval *tv, struct timezone *tz)
{
	double quadCalc = 0.0;

	LARGE_INTEGER frequency;
	LARGE_INTEGER counts;

	if(QueryPerformanceFrequency(&frequency) == 0) {
		return -1;
	}
	else {
		if(QueryPerformanceCounter(&counts) == 0) {
			return -1;
		}
		else {
			quadCalc = ((double)counts.QuadPart / (double)frequency.QuadPart);
			tv->tv_sec = (long)quadCalc;
			tv->tv_usec = (long)((quadCalc - (int)quadCalc) * 1000000.0);
		}
	}

	return 0;
}

static struct tm *localtime_r(const time_t *timer, struct tm *tmbuf)
{
	int year = 0;
	unsigned long secondsIntoDay = 0;
	unsigned long dayNumber = 0;

	ULONGLONG localFiletime;

	time_t time = (time_t)0;

	union {
		struct {
			DWORD LowPart;
			DWORD HighPart;
		} ft;
		ULONGLONG ul;
	} UI;

	year = EPOCH_YR;

	GetSystemTimeAsFileTime((FILETIME *)&UI.ft);	
	localFiletime = (UI.ul - TIME_BASE_DIFF);
	//time = (time_t)(localFiletime / CONSTANT_64BIT(10000000));
	time = (time_t)(localFiletime / 10000000ull);

	secondsIntoDay = (unsigned long)(time % SECS_DAY);
	dayNumber = (unsigned long)(time / SECS_DAY);

	tmbuf->tm_sec = (secondsIntoDay % 60);
	tmbuf->tm_min = ((secondsIntoDay % 3600) / 60);
	tmbuf->tm_hour = (secondsIntoDay / 3600);
	tmbuf->tm_wday = ((dayNumber + 4) % 7); // day 0 was a thursday

	while(dayNumber >= (unsigned long)YEARSIZE(year)) {
		dayNumber -= YEARSIZE(year);
		year++;
	}

	tmbuf->tm_year = (year - YEAR0);
	tmbuf->tm_yday = dayNumber;
	tmbuf->tm_mon = 0;

	while(dayNumber >= YEAR_TABLE[LEAPYEAR(year)][tmbuf->tm_mon]) {
		dayNumber -= YEAR_TABLE[LEAPYEAR(year)][tmbuf->tm_mon];
		tmbuf->tm_mon++;
	}

	tmbuf->tm_mday = dayNumber + 1;
	tmbuf->tm_isdst = 0;

	return tmbuf;
}

// define time (windows plugin) public functions

int time_getTimeSecondsWindows()
{
	int result = 0;

	struct timeval timer;

	gettimeofday(&timer, (struct timezone *)NULL);

	result = (int)timer.tv_sec;

	return result;
}

double time_getTimeMusWindows()
{
	double result = 0.0;

	struct timeval timer;

	gettimeofday(&timer, (struct timezone *)NULL);

	result = ((REAL_ONE_MILLION * (double)timer.tv_sec) +
			(double)timer.tv_usec);

	return result;
}

int time_getElapsedSecondsWindows(int seconds)
{
	if(seconds < 0) {
		DISPLAY_INVALID_ARGS;
		return 0;
	}

	return (time_getTimeSecondsWindows() - seconds);
}

double time_getElapsedMusWindows(double microseconds)
{
	if(microseconds < 0.0) {
		DISPLAY_INVALID_ARGS;
		return 0.0;
	}

	return (time_getTimeMusWindows() - microseconds);
}

void time_getTimestampWindows(char timestamp[32])
{
	struct tm time;
	struct timeval localTime;

	gettimeofday(&localTime, (struct timezone *)NULL);
	localtime_r((const time_t *)&localTime.tv_sec, &time);

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

void time_sleepWindows(int seconds)
{
	if(seconds < 0) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	Sleep(seconds * 1000);
}

void time_usleepWindows(int microseconds)
{
	int sleepTime = 0;

	fd_set fd;

	struct timeval timeout;

	WORD versionRequest;
	WSADATA wsaData;

	if(microseconds < 0) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	if(socketHandle == (SOCKET)NULL) {
		versionRequest = MAKEWORD(1, 0);
		WSAStartup(versionRequest, &wsaData);
		socketHandle = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	}

	FD_ZERO(&fd);
	FD_SET(socketHandle, &fd);

	timeout.tv_sec = (long)(microseconds / INT_ONE_MILLION);
	timeout.tv_usec = (long)(microseconds % INT_ONE_MILLION);

	select(0, 0, 0, &fd, &timeout);
}

void time_nanosleepWindows(int nanoseconds)
{
	time_usleepWindows(nanoseconds / 1000);
}

#endif // WIN32 && !MINGW

