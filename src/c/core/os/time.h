/*
 * time.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard time library, header file.
 *
 * Written by Josh English.
 */

#if !defined(_CORE_OS_TIME_H)

#define _CORE_OS_TIME_H

#if !defined(_CORE_H) && !defined(_CORE_OS_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _CORE_H || _CORE_OS_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

// delcare time public functions

int time_getTimeSeconds();

double time_getTimeMus();

int time_getElapsedSeconds(int seconds);

double time_getElapsedMus(double microseconds);

void time_getTimestamp(char timestamp[32]);

void time_sleep(int seconds);

void time_usleep(int microseconds);

void time_nanosleep(int nanoseconds);

// define time public macros

#define time_getElapsedMusInMilliseconds(microseconds) \
	(time_getElapsedMus(microseconds) / REAL_ONE_THOUSAND)

#define time_getElapsedMusInSeconds(microseconds) \
	(time_getElapsedMus(microseconds) / REAL_ONE_MILLION)

#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _CORE_OS_TIME_H

