/*
 * timestamp.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard-WoW statistical analysis time & date stamp library, header file.
 *
 * Written by Josh English.
 */

#if !defined(_WOWSTATS_CORE_TIMESTAMP_H)

#define _WOWSTATS_CORE_TIMESTAMP_H

#if !defined(_WOWSTATS_CORE_H) && !defined(_WOWSTATS_CORE_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _WOWSTATS_CORE_H || _WOWSTATS_CORE_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

// define wowstats timestamp public constants

#define WOW_TIMESTAMP_EQUAL								0
#define WOW_TIMESTAMP_GREATER_THAN						1
#define WOW_TIMESTAMP_LESS_THAN							2

// define wowstats timestamp public data types

typedef struct _WowTimestamp {
	int month;
	int day;
	int year;
	int hour;
	int minute;
	int second;
	int millis;
} WowTimestamp;

// declare wowstats timestamp public functions

int wowstats_timestampInit(WowTimestamp *timestamp, int year);

int wowstats_timestampParseDate(WowTimestamp *timestamp, char *token,
		int tokenLength);

int wowstats_timestampParseTime(WowTimestamp *timestamp, char *token,
		int tokenLength);

int wowstats_timestampClone(WowTimestamp *alpha, WowTimestamp *beta);

int wowstats_timestampCompare(WowTimestamp *alpha, WowTimestamp *beta,
		int *result);

int wowstats_timestampCalculateDifferenceInSeconds(WowTimestamp *startStamp,
		WowTimestamp *endStamp, int *seconds);

int wowstats_timestampToString(WowTimestamp *timestamp, char string[32]);

void wowstats_timestampNowToString(char string[32]);

#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _WOWSTATS_CORE_TIMESTAMP_H

