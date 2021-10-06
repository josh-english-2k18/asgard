/*
 * timestamp.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard-WoW statistical analysis time & date stamp library.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#define _WOWSTATS_CORE_COMPONENT
#include "wowstats/core/timestamp.h"

// define wowstats timestamp private constants

#define TOKEN_BUFFER_LENGTH									32

// delcare wowstats timestamp private functions

static int parseElement(char *token, int tokenRef, int tokenLength,
		char delimiter, int *parseLength);


// define wowstats timestamp private functions

static int parseElement(char *token, int tokenRef, int tokenLength,
		char delimiter, int *parseLength)
{
	int ii = 0;
	int nn = 0;
	char buffer[TOKEN_BUFFER_LENGTH];

	*parseLength = 0;

	memset(buffer, 0, TOKEN_BUFFER_LENGTH);

	for(ii = tokenRef, nn = 0;
			((ii < tokenLength) && (nn < tokenLength)); ii++) {
		*parseLength += 1;

		if(((unsigned int)token[ii] < 32) || ((unsigned int)token[ii] > 126)) {
			continue;
		}

		if(token[ii] == delimiter) {
			break;
		}

		buffer[nn] = token[ii];
		nn++;

		if((nn + 1) >= TOKEN_BUFFER_LENGTH) {
			if(((ii + 1) < tokenLength) && (token[(ii + 1)] == delimiter)) {
				break;
			}
			return -1;
		}
	}

	return atoi(buffer);
}


// define wowstats timestamp public functions

int wowstats_timestampInit(WowTimestamp *timestamp, int year)
{
	if(timestamp == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	memset(timestamp, 0, sizeof(WowTimestamp));

	timestamp->month = 0;
	timestamp->day = 0;
	timestamp->year = year;
	timestamp->hour = 0;
	timestamp->minute = 0;
	timestamp->second = 0;
	timestamp->millis = 0;

	return 0;
}

int wowstats_timestampParseDate(WowTimestamp *timestamp, char *token,
		int tokenLength)
{
	int tokenRef = 0;
	int parsedNumeric = 0;
	int parseLength = 0;

	if((timestamp == NULL) || (token == NULL) || (tokenLength < 1)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	// parse the month

	parsedNumeric = parseElement(token, tokenRef, tokenLength, '/',
			&parseLength);
	if(parsedNumeric < 0) {
		return -1;
	}

	timestamp->month = parsedNumeric;

	tokenRef += parseLength;
	if(tokenRef >= tokenLength) {
		return -1;
	}

	// parse the day

	parsedNumeric = parseElement(token, tokenRef, tokenLength, '\0',
			&parseLength);
	if(parsedNumeric < 0) {
		return -1;
	}

	timestamp->day = parsedNumeric;

	return 0;
}

int wowstats_timestampParseTime(WowTimestamp *timestamp, char *token,
		int tokenLength)
{
	int tokenRef = 0;
	int parsedNumeric = 0;
	int parseLength = 0;

	if((timestamp == NULL) || (token == NULL) || (tokenLength < 1)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	// parse the hour

	parsedNumeric = parseElement(token, tokenRef, tokenLength, ':',
			&parseLength);
	if(parsedNumeric < 0) {
		return -1;
	}

	timestamp->hour = parsedNumeric;

	tokenRef += parseLength;
	if(tokenRef >= tokenLength) {
		return -1;
	}

	// parse the minute

	parsedNumeric = parseElement(token, tokenRef, tokenLength, ':',
			&parseLength);
	if(parsedNumeric < 0) {
		return -1;
	}

	timestamp->minute = parsedNumeric;

	tokenRef += parseLength;
	if(tokenRef >= tokenLength) {
		return -1;
	}

	// parse the second

	parsedNumeric = parseElement(token, tokenRef, tokenLength, '.',
			&parseLength);
	if(parsedNumeric < 0) {
		return -1;
	}

	timestamp->second = parsedNumeric;

	tokenRef += parseLength;
	if(tokenRef >= tokenLength) {
		return -1;
	}

	// parse the millis

	parsedNumeric = parseElement(token, tokenRef, tokenLength, '\0',
			&parseLength);
	if(parsedNumeric < 0) {
		return -1;
	}

	timestamp->millis = parsedNumeric;

	return 0;
}

int wowstats_timestampClone(WowTimestamp *alpha, WowTimestamp *beta)
{
	if((alpha == NULL) || (beta == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	alpha->month = beta->month;
	alpha->day = beta->day;
	alpha->year = beta->year;
	alpha->hour = beta->hour;
	alpha->minute = beta->minute;
	alpha->second = beta->second;
	alpha->millis = beta->millis;

	return 0;
}

int wowstats_timestampCompare(WowTimestamp *alpha, WowTimestamp *beta,
		int *result)
{
	if((alpha == NULL) || (beta == NULL) || (result == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(alpha->year > beta->year) {
		*result = WOW_TIMESTAMP_LESS_THAN;
	}
	else if(alpha->year < beta->year) {
		*result = WOW_TIMESTAMP_GREATER_THAN;
	}
	else if(alpha->month > beta->month) {
		*result = WOW_TIMESTAMP_LESS_THAN;
	}
	else if(alpha->month < beta->month) {
		*result = WOW_TIMESTAMP_GREATER_THAN;
	}
	else if(alpha->day > beta->day) {
		*result = WOW_TIMESTAMP_LESS_THAN;
	}
	else if(alpha->day < beta->day) {
		*result = WOW_TIMESTAMP_GREATER_THAN;
	}
	else if(alpha->hour > beta->hour) {
		*result = WOW_TIMESTAMP_LESS_THAN;
	}
	else if(alpha->hour < beta->hour) {
		*result = WOW_TIMESTAMP_GREATER_THAN;
	}
	else if(alpha->minute > beta->minute) {
		*result = WOW_TIMESTAMP_LESS_THAN;
	}
	else if(alpha->minute < beta->minute) {
		*result = WOW_TIMESTAMP_GREATER_THAN;
	}
	else if(alpha->second > beta->second) {
		*result = WOW_TIMESTAMP_LESS_THAN;
	}
	else if(alpha->second < beta->second) {
		*result = WOW_TIMESTAMP_GREATER_THAN;
	}
	else if(alpha->millis > beta->millis) {
		*result = WOW_TIMESTAMP_LESS_THAN;
	}
	else if(alpha->millis < beta->millis) {
		*result = WOW_TIMESTAMP_GREATER_THAN;
	}
	else {
		*result = WOW_TIMESTAMP_EQUAL;
	}

	return 0;
}

int wowstats_timestampCalculateDifferenceInSeconds(WowTimestamp *startStamp,
		WowTimestamp *endStamp, int *seconds)
{
	time_t startTime = (time_t)0;
	time_t endTime = (time_t)0;

	struct tm startTimeData;
	struct tm endTimeData;

	if((startStamp == NULL) || (endStamp == NULL) || (seconds == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	*seconds = 0;

	// convert the start timestamp into seconds from epoch

	memset((void *)&startTimeData, 0, sizeof(struct tm));

	startTimeData.tm_sec = startStamp->second;
	startTimeData.tm_min = startStamp->minute;
	startTimeData.tm_hour = startStamp->hour;
	startTimeData.tm_mday = startStamp->day;
	startTimeData.tm_mon = startStamp->month;
	startTimeData.tm_year = startStamp->year;

	startTime = mktime(&startTimeData);

	// convert the end timestamp into seconds from epoch

	memset((void *)&endTimeData, 0, sizeof(struct tm));

	endTimeData.tm_sec = endStamp->second;
	endTimeData.tm_min = endStamp->minute;
	endTimeData.tm_hour = endStamp->hour;
	endTimeData.tm_mday = endStamp->day;
	endTimeData.tm_mon = endStamp->month;
	endTimeData.tm_year = endStamp->year;

	endTime = mktime(&endTimeData);

	// calculate the difference

	*seconds = (int)difftime(startTime, endTime);

	return 0;
}

int wowstats_timestampToString(WowTimestamp *timestamp, char string[32])
{
	if(timestamp == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	memset(string, 0, 32);

	snprintf(string, 32, "%4i-%02i-%02i %02i:%02i:%02i",
			timestamp->year, timestamp->month, timestamp->day,
			timestamp->hour, timestamp->minute, timestamp->second);

	return 0;
}

void wowstats_timestampNowToString(char string[32])
{
	memset(string, 0, 32);

#if defined(__linux__)
	struct tm time;
	struct timeval local_time;

	gettimeofday(&local_time, (struct timezone *)NULL);
	localtime_r(&local_time.tv_sec, &time);

	snprintf(string, 32, "%4i-%02i-%02i %02i:%02i:%02i",
        (time.tm_year + 1900), (time.tm_mon + 1), time.tm_mday,
		time.tm_hour, time.tm_min, time.tm_sec);
#endif // __linux__
}

