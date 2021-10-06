/*
 * log.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Java log4j-style application logging system.
 *
 * Written by Josh English.
 */

// preprocessor directives

#define _CORE_COMMON_BYPASS_STD_STREAM
#include "core/core.h"
#define _CORE_LIBRARY_TYPES_COMPONENT
#include "core/library/util/log.h"


// define log public functions

int log_init(Log *log, int logOutput, char *filename, int logLevel)
{
	int rc = 0;
	char *errorMessage = NULL;

	if((log == NULL) ||
			((logOutput != LOG_OUTPUT_NULL) &&
			 (logOutput != LOG_OUTPUT_STDOUT) &&
			 (logOutput != LOG_OUTPUT_STDERR) &&
			 (logOutput != LOG_OUTPUT_FILE)) ||
			((logOutput == LOG_OUTPUT_FILE) &&
			 (filename == NULL)) ||
			((logLevel != LOG_LEVEL_DEBUG) &&
			 (logLevel != LOG_LEVEL_INFO) &&
			 (logLevel != LOG_LEVEL_FINE) &&
			 (logLevel != LOG_LEVEL_WARNING) &&
			 (logLevel != LOG_LEVEL_ERROR) &&
			 (logLevel != LOG_LEVEL_SEVERE) &&
			 (logLevel != LOG_LEVEL_PANIC))
	  ) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	memset(log, 0, (int)(sizeof(Log)));

	log->isFlushOutput = afalse;
	log->isPeriodicFlush = afalse;
	log->periodicFlushCounter = 0;
	log->periodicFlushThreshold = 0;
	log->level = logLevel;
	log->output = logOutput;

	if(log->output == LOG_OUTPUT_FILE) {
		if((rc = file_init(&log->fh, filename, "ab", 524288)) < 0) {
			if(EXPLICIT_ERRORS) {
				file_getError(&log->fh, rc, &errorMessage);
				fprintf(stderr, "[%s():%i] error - failed to initialze file "
						"handle on '%s' with '%s'.\n", __FUNCTION__, __LINE__,
						filename, errorMessage);
			}
			file_free(&log->fh);
			log->output = LOG_OUTPUT_STDOUT;
			return -1;
		}
	}

	return 0;
}

int log_free(Log *log)
{
	if(log == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	switch(log->output) {
		case LOG_OUTPUT_NULL:
		case LOG_OUTPUT_UNKNOWN:
			break;

		case LOG_OUTPUT_STDOUT:
			fflush(stdout);
			break;

		case LOG_OUTPUT_STDERR:
			fflush(stderr);
			break;

		case LOG_OUTPUT_FILE:
			if(file_sync(&log->fh) < 0) {
				return -1;
			}
			if(file_free(&log->fh) < 0) {
				return -1;
			}
			break;
	}

	memset(log, 0, (int)(sizeof(Log)));

	return 0;
}

char *log_levelToString(LogLevel logLevel)
{
	char *result = NULL;

	switch(logLevel) {
		case LOG_LEVEL_DEBUG:
			result = "Debug";
			break;

		case LOG_LEVEL_INFO:
			result = "Info";
			break;

		case LOG_LEVEL_FINE:
			result = "Fine";
			break;

		case LOG_LEVEL_WARNING:
			result = "Warning";
			break;

		case LOG_LEVEL_ERROR:
			result = "Error";
			break;

		case LOG_LEVEL_SEVERE:
			result = "Severe";
			break;

		case LOG_LEVEL_PANIC:
			result = "Panic";
			break;

		case LOG_LEVEL_UNKNOWN:
		default:
			result = "Unknown";
	}

	return result;
}

char *log_outputToString(LogOutput logOutput)
{
	char *result = NULL;

	switch(logOutput) {
		case LOG_OUTPUT_NULL:
			result = "Null";
			break;

		case LOG_OUTPUT_STDOUT:
			result = "Stdout";
			break;

		case LOG_OUTPUT_STDERR:
			result = "Stderr";
			break;

		case LOG_OUTPUT_FILE:
			result = "File";
			break;

		case LOG_OUTPUT_UNKNOWN:
		default:
			result = "Unknown";
	}

	return result;
}

aboolean log_isFlushOutput(Log *log)
{
	if(log == NULL) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	return log->isFlushOutput;
}

int log_setFlushOutput(Log *log, aboolean isFlushOutput)
{
	if(log == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	log->isFlushOutput = isFlushOutput;

	return 0;
}

aboolean log_isPeriodicFlush(Log *log)
{
	if(log == NULL) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	return log->isPeriodicFlush;
}

int log_setPeriodicFlush(Log *log, aboolean isPeriodicFlush,
		int flushThreshold)
{
	if((log == NULL) || (flushThreshold < 1)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	log->isPeriodicFlush = isPeriodicFlush;
	log->periodicFlushCounter = 0;
	log->periodicFlushThreshold = flushThreshold;

	return 0;
}

int log_enablePeriodicFlush(Log *log, int flushThreshold)
{
	if((log == NULL) || (flushThreshold < 1)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return log_setPeriodicFlush(log, atrue, flushThreshold);
}

int log_disablePeriodicFlush(Log *log)
{
	if(log == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	log->isPeriodicFlush = afalse;

	return 0;
}

int log_getLevel(Log *log)
{
	if(log == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return (int)log->level;
}

int log_setLevel(Log *log, int logLevel)
{
	if((log == NULL) ||
			((logLevel != LOG_LEVEL_DEBUG) &&
			 (logLevel != LOG_LEVEL_INFO) &&
			 (logLevel != LOG_LEVEL_FINE) &&
			 (logLevel != LOG_LEVEL_WARNING) &&
			 (logLevel != LOG_LEVEL_ERROR) &&
			 (logLevel != LOG_LEVEL_SEVERE) &&
			 (logLevel != LOG_LEVEL_PANIC))
	  ) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	log->level = logLevel;

	return 0;
}

int log_getOutput(Log *log)
{
	if(log == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return (int)log->output;
}

int log_setOutput(Log *log, int logOutput, char *filename)
{
	int rc = 0;
	char *errorMessage = NULL;

	if((log == NULL) ||
			((logOutput != LOG_OUTPUT_NULL) &&
			 (logOutput != LOG_OUTPUT_STDOUT) &&
			 (logOutput != LOG_OUTPUT_STDERR) &&
			 (logOutput != LOG_OUTPUT_FILE)) ||
			((logOutput == LOG_OUTPUT_FILE) &&
			 (filename == NULL))
	  ) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(log->output == LOG_OUTPUT_FILE) {
		file_free(&log->fh);
	}

	log->output = logOutput;

	if(log->output == LOG_OUTPUT_FILE) {
		rc = file_init(&log->fh, filename, "ab", 524288);
		if(rc < 0) { 
			if(EXPLICIT_ERRORS) {
				file_getError(&log->fh, rc, &errorMessage);
				fprintf(stderr, "[%s():%i] error - failed to initialze file "
						"handle on '%s' with '%s'.\n", __FUNCTION__, __LINE__,
						filename, errorMessage);
			}
			file_free(&log->fh);
			log->output = LOG_OUTPUT_STDOUT;
			return -1;
		}
	}

	return 0;
}

int log_logf(Log *log, int logLevel, const char *format, ...)
{
	aboolean isFlushOutput = afalse;
	aboolean isPeriodicFlush = afalse;
	int threadId = 0;
	int bufferLength = 0;
	char *buffer = NULL;
	char *logLevelString = NULL;
	char timestamp[32];
	void *stream = NULL;

	LogOutput output = LOG_OUTPUT_STDOUT;

	va_list vaArgs;

	if(
			((logLevel != LOG_LEVEL_DEBUG) &&
			 (logLevel != LOG_LEVEL_INFO) &&
			 (logLevel != LOG_LEVEL_FINE) &&
			 (logLevel != LOG_LEVEL_WARNING) &&
			 (logLevel != LOG_LEVEL_ERROR) &&
			 (logLevel != LOG_LEVEL_SEVERE) &&
			 (logLevel != LOG_LEVEL_PANIC))
	  ) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(log != NULL) {
		if((log->output == LOG_OUTPUT_NULL) || (logLevel < log->level)) {
			return 0;
		}

		output = log->output;
		isFlushOutput = log->isFlushOutput;
		isPeriodicFlush = log->isPeriodicFlush;
	}

	va_start(vaArgs, format);
	buffer = vaprintf(format, vaArgs, &bufferLength);
	va_end(vaArgs);

	switch(output) {
		case LOG_OUTPUT_STDOUT:
			stream = stdout;
			break;

		case LOG_OUTPUT_STDERR:
			stream = stderr;
			break;

		case LOG_OUTPUT_FILE:
			if(log != NULL) {
				stream = log->fh.fd;
			}
			else {
				stream = stdout;
			}
			break;

		default:
			stream = stdout;
	}

	switch(logLevel) {
		case LOG_LEVEL_DEBUG:
			logLevelString = "DEBUG";
			break;

		case LOG_LEVEL_INFO:
			logLevelString = "INFO";
			break;

		case LOG_LEVEL_FINE:
			logLevelString = "FINE";
			break;

		case LOG_LEVEL_WARNING:
			logLevelString = "WARNING";
			break;

		case LOG_LEVEL_ERROR:
			logLevelString = "ERROR";
			break;

		case LOG_LEVEL_SEVERE:
			logLevelString = "SEVERE";
			break;

		case LOG_LEVEL_PANIC:
			logLevelString = "PANIC";
			break;

		default:
			logLevelString = "UNKNOWN";
	}

	time_getTimestamp(timestamp);
	thread_self(&threadId);

#if !defined(__ANDROID__)
	fprintf(stream, "[%s] (%u) {%s} %s\n", timestamp, (unsigned int)threadId,
			logLevelString, buffer);

	if(isPeriodicFlush) {
		log->periodicFlushCounter += 1;
	}

	if((isFlushOutput) ||
			((isPeriodicFlush) &&
			 (log->periodicFlushCounter >= log->periodicFlushThreshold))) {
		fflush(stream);
	}

#else // __ANDROID__
	__android_log_print(ANDROID_LOG_INFO,
			"asgard",
			"[%s] (%u) {%s} %s\n",
			timestamp,
			(unsigned int)threadId,
			logLevelString,
			buffer);
#endif // !__ANDROID__

	free(buffer);

	return 0;
}

int log_flush(Log *log)
{
	void *stream = NULL;

	LogOutput output = LOG_OUTPUT_STDOUT;

	if(log != NULL) {
		if(log->output == LOG_OUTPUT_NULL) {
			return 0;
		}

		output = log->output;
	}

	switch(output) {
		case LOG_OUTPUT_STDOUT:
			stream = stdout;
			break;

		case LOG_OUTPUT_STDERR:
			stream = stderr;
			break;

		case LOG_OUTPUT_FILE:
			if(log != NULL) {
				stream = log->fh.fd;
			}
			else {
				stream = stdout;
			}
			break;

		default:
			stream = stdout;
	}

#if !defined(__ANDROID__)
	fflush(stream);
#endif // !__ANDROID__

	return 0;
}

