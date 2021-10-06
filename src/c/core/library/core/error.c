/*
 * error.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard core library error management functions.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#define _CORE_LIBRARY_CORE_COMPONENT
#include "core/library/core/error.h"

// declare error private functions

static void initEntry(ErrorEntry *entry);
static void freeEntry(ErrorEntry *entry);
static void copyEntry(ErrorEntry *entryDest, ErrorEntry *entrySrc);
static void removeEntry(Error *error, int listId);
static char *buildUnknownError(int uid);
static char *formatMessage(ErrorEntry *entry);

// define error private functions

static void initEntry(ErrorEntry *entry)
{
	memset(entry, 0, (sizeof(ErrorEntry)));

	entry->uid = 0;
	entry->line = 0;
	entry->function = NULL;
	entry->file = NULL;
	entry->message = NULL;
}

static void freeEntry(ErrorEntry *entry)
{
	if(entry->function != NULL) {
		free(entry->function);
	}
	if(entry->file != NULL) {
		free(entry->file);
	}
	if(entry->message != NULL) {
		free(entry->message);
	}

	memset(entry, 0, (sizeof(ErrorEntry)));
}

static void copyEntry(ErrorEntry *entryDest, ErrorEntry *entrySrc)
{
	entryDest->uid = entrySrc->uid;
	entryDest->line = entrySrc->line;
	entryDest->function = entrySrc->function;
	entryDest->file = entrySrc->file;
	entryDest->message = entrySrc->message;
}

static void removeEntry(Error *error, int listId)
{
	int ii = 0;
	int nn = 0;

	for(ii = 0, nn = 0; ii < error->entryCount; ii++) {
		if(ii == listId) {
			freeEntry(&error->entries[ii]);
			continue;
		}
		if(ii > nn) {
			copyEntry(&error->entries[nn], &error->entries[ii]);
		}
		nn++;
	}
}

static char *buildUnknownError(int uid)
{
	int length = 0;
	char *result = NULL;

	const char *template = "unknown error code %i";

	length = (int)(strlen(template) + 32);
	result = (char *)malloc(sizeof(char) * length);

	snprintf(result, length, template, uid);

	return result;
}

static char *formatMessage(ErrorEntry *entry)
{
	int length = 0;
	char *result = NULL;

	length = (int)(strlen(entry->function) +
			strlen(entry->file) +
			strlen(entry->message) + 32);
	result = (char *)malloc(sizeof(char) * length);

	snprintf(result, length, "[%s->%s():%i] %s", entry->file, entry->function,
			entry->line, entry->message);

	return result;
}

// define error public functions

int error_init(Error *error)
{
	int ii = 0;

	if(error == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	memset(error, 0, (sizeof(Error)));

	error->displayErrors = ERROR_DEFAULT_DISPLAY_ERRORS;
	error->uidCount = 128;
	error->entryCount = 0;
	error->entryLength = ERROR_DEFAULT_ENTRY_LENGTH;
	error->entries = (ErrorEntry *)malloc(sizeof(ErrorEntry) *
			error->entryLength);

	for(ii = 0; ii < error->entryLength; ii++) {
		initEntry(&error->entries[ii]);
	}

	mutex_init(&error->mutex);

	return 0;
}

int error_free(Error *error)
{
	int ii = 0;

	if(error == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&error->mutex);

	if(error->entries != NULL) {
		for(ii = 0; ii < error->entryCount; ii++) {
			freeEntry(&error->entries[ii]);
		}
		free(error->entries);
	}

	mutex_unlock(&error->mutex);
	mutex_free(&error->mutex);

	memset(error, 0, (sizeof(Error)));

	return 0;
}

int error_clearEntries(Error *error)
{
	int ii = 0;

	if(error == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&error->mutex);

	for(ii = 0; ii < error->entryCount; ii++) {
		freeEntry(&error->entries[ii]);
	}

	error->entryCount = 0;

	mutex_unlock(&error->mutex);

	return 0;
}

int error_setDisplayErrors(Error *error, aboolean displayErrors)
{
	if(error == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&error->mutex);

	error->displayErrors = displayErrors;

	mutex_unlock(&error->mutex);

	return 0;
}

int error_getDisplayErrors(Error *error, aboolean *displayErrors)
{
	if((error == NULL) || (displayErrors == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&error->mutex);

	*displayErrors = error->displayErrors;

	mutex_unlock(&error->mutex);

	return 0;
}

int error_getEntryCount(Error *error, int *entryCount)
{
	if((error == NULL) || (entryCount == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&error->mutex);

	*entryCount = error->entryCount;

	mutex_unlock(&error->mutex);

	return 0;
}

int error_setEntryLength(Error *error, int entryLength)
{
	int ii = 0;

	if(error == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&error->mutex);

	for(ii = 0; ii < error->entryCount; ii++) {
		freeEntry(&error->entries[ii]);
	}

	error->entryCount = 0;
	error->entryLength = entryLength;
	error->entries = (ErrorEntry *)malloc(sizeof(ErrorEntry) *
			error->entryLength);

	for(ii = 0; ii < error->entryLength; ii++) {
		initEntry(&error->entries[ii]);
	}

	mutex_unlock(&error->mutex);

	return 0;
}

int error_getEntryLength(Error *error, int *entryLength)
{
	if((error == NULL) || (entryLength == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&error->mutex);

	*entryLength = error->entryLength;

	mutex_unlock(&error->mutex);

	return 0;
}

int error_getError(Error *error, int uid, char **errorMessage)
{
	int ii = 0;

	if((error == NULL) || (errorMessage == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	*errorMessage = NULL;

	mutex_lock(&error->mutex);

	for(ii = 0; ii < error->entryCount; ii++) {
		if(uid == error->entries[ii].uid) {
			*errorMessage = formatMessage(&error->entries[ii]);
			removeEntry(error, ii);
			error->entryCount -= 1;
			break;
		}
	}

	mutex_unlock(&error->mutex);

	if(*errorMessage == NULL) {
		*errorMessage = buildUnknownError(uid);
	}

	return 0;
}

int error_addError(Error *error, const char *file, const char *function,
		size_t line, int *uid, char *errorMessage, aboolean isStaticMessage)
{
	int id = 0;
	aboolean displayError = afalse;

	if((error == NULL) || (file == NULL) || (function == NULL) ||
			(line < 0) || (uid == NULL) || (errorMessage == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	*uid = 0;

	mutex_lock(&error->mutex);

	if(error->entryCount >= error->entryLength) {
		removeEntry(error, 0);
		error->entryCount -= 1;
	}

	id = error->entryCount;

	error->entries[id].uid = (error->uidCount * -1);
	error->entries[id].line = (int)line;
	error->entries[id].function = strdup((char *)function);
	error->entries[id].file = strdup((char *)file);
	if(isStaticMessage) {
		error->entries[id].message = strdup(errorMessage);
	}
	else {
		error->entries[id].message = errorMessage;
	}

	error->uidCount += 1;
	error->entryCount += 1;

	*uid = error->entries[id].uid;

	displayError = error->displayErrors;

	mutex_unlock(&error->mutex);

	if((EXPLICIT_ERRORS) && (displayError)) {
		fprintf(stderr, "[%s->%s():%i] error - %s\n", (char *)file,
				(char *)function, (int)line, errorMessage);
	}

	return 0;
}

int error_handleError(void *error, const char *file, const char *function,
		size_t line, const char *format, ...)
{
	int result = 0;
	int bufferLength = 0;
	char *buffer = NULL;
	void *stream = NULL;

	va_list vaArgs;

	if(file == NULL) {
		file = "unknown.c";
	}
	if(function == NULL) {
		function = "unknown";
	}
	if(line < 0) {
		line = 0;
	}

	if((error == NULL) || (error == (void *)stdout) ||
			(error == (void *)stderr)) {
		if(error == (void *)stdout) {
			stream = stdout;
		}
		else {
			stream = stderr;
		}
	}

	if(format == NULL) {
		if(stream != NULL) {
			if(EXPLICIT_ERRORS) {
				fprintf(stream, "[%s->%s():%i] error - %s\n", (char *)file,
						(char *)function, (int)line, "unknown");
			}
			result = -1;
		}
		else {
			if(error_addError(error, file, function, line, &result, "unknown",
						atrue) < 0) {
				return 0;
			}
		}
	}
	else {
		va_start(vaArgs, format);
		buffer = vaprintf(format, vaArgs, &bufferLength);
		va_end(vaArgs);

		if(stream != NULL) {
			fprintf(stream, "[%s->%s():%i] error - %s\n", (char *)file,
					(char *)function, (int)line, buffer);
			free(buffer);
			result = -1;
		}
		else {
			if(error_addError(error, file, function, line, &result, buffer,
						afalse) < 0) {
				free(buffer);
				return 0;
			}
		}
	}

	return result;
}

