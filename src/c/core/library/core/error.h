/*
 * error.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard core library error management functions, header file.
 *
 * Written by Josh English.
 */

#if !defined(_CORE_LIBRARY_CORE_ERROR_H)

#define _CORE_LIBRARY_CORE_ERROR_H

#if !defined(_CORE_H) && !defined(_CORE_LIBRARY_H) && \
		!defined(_CORE_LIBRARY_CORE_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _CORE_H || _CORE_LIBRARY_H || _CORE_LIBRARY_CORE_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

// define error public constants

#define ERROR_DEFAULT_DISPLAY_ERRORS					afalse
#define ERROR_DEFAULT_ENTRY_LENGTH						16

// define error public data types

typedef struct _ErrorEntry {
	int uid;
	int line;
	char *function;
	char *file;
	char *message;
} ErrorEntry;

typedef struct _Error {
	aboolean displayErrors;
	int uidCount;
	int entryCount;
	int entryLength;
	ErrorEntry *entries;
	Mutex mutex;
} Error;

// delcare error public functions

int error_init(Error *error);

int error_free(Error *error);

int error_clearEntries(Error *error);

int error_setDisplayErrors(Error *error, aboolean displayErrors);

int error_getDisplayErrors(Error *error, aboolean *displayErrors);

int error_getEntryCount(Error *error, int *entryCount);

int error_setEntryLength(Error *error, int entryLength);

int error_getEntryLength(Error *error, int *entryLength);

int error_getError(Error *error, int uid, char **errorMessage);

int error_addError(Error *error, const char *file, const char *function,
		size_t line, int *uid, char *errorMessage, aboolean isStaticMessage);

/**
 * error_handleError()
 *
 * This function differs in its API calling convention in that it returns a
 * negative number indicating the error UID, or a zero if it fails.
 */
int error_handleError(void *error, const char *file, const char *function,
		size_t line, const char *format, ...);

// define error public macros

#define errorf(context, format, ...) \
	error_handleError(context, __FILE__, __FUNCTION__, __LINE__, format, \
		##__VA_ARGS__)

#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _CORE_LIBRARY_CORE_ERROR_H

