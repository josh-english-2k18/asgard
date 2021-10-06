/*
 * log.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Java log4j-style application logging system, header file.
 *
 * Written by Josh English.
 */

#if !defined(_CORE_LIBRARY_UTIL_LOG_H)

#define _CORE_LIBRARY_UTIL_LOG_H

#if !defined(_CORE_H) && !defined(_CORE_LIBRARY_H) && \
		!defined(_CORE_LIBRARY_UTIL_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _CORE_H || _CORE_LIBRARY_H || _CORE_LIBRARY_UTIL_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// define log public constants

typedef enum _LogLevel {
	LOG_LEVEL_DEBUG = 1,
	LOG_LEVEL_INFO,
	LOG_LEVEL_FINE,
	LOG_LEVEL_WARNING,
	LOG_LEVEL_ERROR,
	LOG_LEVEL_SEVERE,
	LOG_LEVEL_PANIC,
	LOG_LEVEL_UNKNOWN = -1
} LogLevel;

typedef enum _LogOutput {
	LOG_OUTPUT_NULL = 1,
	LOG_OUTPUT_STDOUT,
	LOG_OUTPUT_STDERR,
	LOG_OUTPUT_FILE,
	LOG_OUTPUT_UNKNOWN = -1
} LogOutput;


// define log public data types

typedef struct _Log {
	aboolean isFlushOutput;
	aboolean isPeriodicFlush;
	int periodicFlushCounter;
	int periodicFlushThreshold;
	LogLevel level;
	LogOutput output;
	FileHandle fh;
} Log;


// declare log public functions

int log_init(Log *log, int logOutput, char *filename, int logLevel);

int log_free(Log *log);

char *log_levelToString(LogLevel logLevel);

char *log_outputToString(LogOutput logOutput);

aboolean log_isFlushOutput(Log *log);

int log_setFlushOutput(Log *log, aboolean isFlushOutput);

aboolean log_isPeriodicFlush(Log *log);

int log_setPeriodicFlush(Log *log, aboolean isPeriodicFlush,
		int flushThreshold);

int log_enablePeriodicFlush(Log *log, int flushThreshold);

int log_disablePeriodicFlush(Log *log);

int log_getLevel(Log *log);

int log_setLevel(Log *log, int logLevel);

int log_getOutput(Log *log);

int log_setOutput(Log *log, int logOutput, char *filename);

int log_logf(Log *log, int logLevel, const char *format, ...);

int log_flush(Log *log);


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _CORE_LIBRARY_UTIL_LOG_H

