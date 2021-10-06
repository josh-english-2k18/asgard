/*
 * common.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The master common function library for Asgard.
 *
 * Written by Josh English.
 */

// preprocessor directives

#define _CORE_COMMON_COMPONENT
#define _CORE_COMMON_BYPASS_STD_STREAM
#include "core/common/common.h"
#include "core/common/macros.h"
#include "core/common/version.h"
#include "core/common/string.h"
#define _CORE_OS_COMPONENT
#include "core/os/types.h"
#define _CORE_MEMORY_COMPONENT
#define MEMORY_NATIVE
#include "core/memory/memory.h"


// define common function library private global variables

StdStream *stdStream = NULL;


// declare common function library private functions

static void cleanStdString(char *string);


// define common function library private functions

static void cleanStdString(char *string)
{
	int ii = 0;
	int length = 0;

	if((string == NULL) || ((length = strlen(string)) < 1)) {
		return;
	}

	for(ii = (length - 1); ii >= 0; ii--) {
		if(((int)((unsigned char)string[ii]) == 10) ||
				((int)((unsigned char)string[ii]) == 13)) {
			string[ii] = '\0';
		}
		else {
			break;
		}
	}
}


// define common function library public functions

void common_initStdStream()
{
	int ii = 0;

	stdStream = (StdStream *)MEMORY_MALLOC_FUNCTION((int)(sizeof(StdStream)),
			__FILE__, __FUNCTION__, __LINE__);

	stdStream->errRef = 0;
	stdStream->outRef = 0;

	for(ii = 0; ii < COMMON_STD_STREAM_ENTRY_LENGTH; ii++) {
		stdStream->errEntry[ii] = NULL;
		stdStream->outEntry[ii] = NULL;
	}

	atexit(common_freeStdStream);
}

void common_freeStdStream(void)
{
	int ii = 0;

	if(stdStream == NULL) {
		return;
	}

	for(ii = 0; ii < stdStream->errRef; ii++) {
		if(stdStream->errEntry[ii] != NULL) {
#if !defined(__ANDROID__)
			fprintf(stderr, "[%s()->stderr] %s\n", __FUNCTION__,
					stdStream->errEntry[ii]);
#else // __ANDROID__
			__android_log_print(ANDROID_LOG_ERROR,
					"asgard",
					stdStream->errEntry[ii]);
#endif // !__ANDROID__
			MEMORY_FREE_FUNCTION(stdStream->errEntry[ii],
					__FILE__, __FUNCTION__, __LINE__);
		}
	}

	for(ii = 0; ii < stdStream->outRef; ii++) {
		if(stdStream->outEntry[ii] != NULL) {
#if !defined(__ANDROID__)
			fprintf(stdout, "[%s()->stdout] %s\n", __FUNCTION__,
					stdStream->outEntry[ii]);
#else // __ANDROID__
			__android_log_print(ANDROID_LOG_INFO,
					"asgard",
					stdStream->errEntry[ii]);
#endif // !__ANDROID__
			MEMORY_FREE_FUNCTION(stdStream->outEntry[ii],
					__FILE__, __FUNCTION__, __LINE__);
		}
	}

	MEMORY_FREE_FUNCTION(stdStream, __FILE__, __FUNCTION__, __LINE__);

	stdStream = NULL;
}

StdStream *common_getStdStream()
{
	if(stdStream == NULL) {
		common_initStdStream();
	}

	return stdStream;
}

void common_logStdStream(void *stream, const char *format, ...)
{
	int bufferLength = 0;
	char *buffer = NULL;

	va_list vaArgs;

	if(stdStream == NULL) {
		common_initStdStream();
	}

	va_start(vaArgs, format);
	buffer = vaprintf(format, vaArgs, &bufferLength);
	va_end(vaArgs);

	if(stream == stderr) {
		common_logStdStreamError(buffer);
	}
	else {
		common_logStdStreamOutput(buffer);
	}
}

void common_logStdStreamError(char *buffer)
{
	int ii = 0;

	if(stdStream == NULL) {
		common_initStdStream();
	}

	cleanStdString(buffer);

	stdStream->errEntry[(stdStream->errRef)] = buffer;
	stdStream->errRef += 1;

	if(stdStream->errRef >= COMMON_STD_STREAM_ENTRY_LENGTH) {
		MEMORY_FREE_FUNCTION(stdStream->errEntry[0],
				__FILE__, __FUNCTION__, __LINE__);

		for(ii = 0; ii < (COMMON_STD_STREAM_ENTRY_LENGTH - 1); ii++) {
			stdStream->errEntry[ii] = stdStream->errEntry[(ii + 1)];
		}

		stdStream->errRef = (COMMON_STD_STREAM_ENTRY_LENGTH - 1);
	}
}

void common_logStdStreamOutput(char *buffer)
{
	int ii = 0;

	if(stdStream == NULL) {
		common_initStdStream();
	}

	cleanStdString(buffer);

	stdStream->outEntry[(stdStream->outRef)] = buffer;
	stdStream->outRef += 1;

	if(stdStream->outRef >= COMMON_STD_STREAM_ENTRY_LENGTH) {
		MEMORY_FREE_FUNCTION(stdStream->outEntry[0],
				__FILE__, __FUNCTION__, __LINE__);

		for(ii = 0; ii < (COMMON_STD_STREAM_ENTRY_LENGTH - 1); ii++) {
			stdStream->outEntry[ii] = stdStream->outEntry[(ii + 1)];
		}

		stdStream->outRef = (COMMON_STD_STREAM_ENTRY_LENGTH - 1);
	}
}

char *common_popStdStreamError()
{
	int position = 0;
	char *result = NULL;

	if(stdStream == NULL) {
		common_initStdStream();
		return NULL;
	}

	position = (stdStream->errRef - 1);
	if((position < 0) || (position >= COMMON_STD_STREAM_ENTRY_LENGTH)) {
		return NULL;
	}

	result = stdStream->errEntry[position];

	stdStream->errEntry[position] = NULL;
	stdStream->errRef -= 1;

	if(stdStream->errRef < 0) {
		stdStream->errRef = 0;
	}

	return result;
}

char *common_popStdStreamOutput()
{
	int position = 0;
	char *result = NULL;

	if(stdStream == NULL) {
		common_initStdStream();
		return NULL;
	}

	position = (stdStream->outRef - 1);
	if((position < 0) || (position >= COMMON_STD_STREAM_ENTRY_LENGTH)) {
		return NULL;
	}

	result = stdStream->outEntry[position];

	stdStream->outEntry[position] = NULL;
	stdStream->outRef -= 1;

	if(stdStream->outRef < 0) {
		stdStream->outRef = 0;
	}

	return result;
}

void common_display(void *stream, char *string, int stringLength)
{
	int ii = 0;
	int intValue = 0;

	if(stream == NULL) {
		stream = stdout;
	}

	fprintf(stream, "string(%i)=>'", stringLength);

	if((string != NULL) && (stringLength > 0)) {
		for(ii = 0; ii < stringLength; ii++) {
			intValue = (int)((unsigned char)string[ii]);

			if((intValue < 32) || (intValue > 126)) {
				fprintf(stream, "0x%02x", intValue);
			}
			else {
				fprintf(stream, "%c", string[ii]);
			}
		}
	}

	fprintf(stream, "'.\n");
}

