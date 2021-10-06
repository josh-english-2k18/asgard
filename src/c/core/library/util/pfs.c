/*
 * pfs.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * A simplified profile-function system (PFS).
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#define _CORE_LIBRARY_TYPES_COMPONENT
#include "core/library/util/pfs.h"

// define pfs private functions

static void buildKey(const char *file, const char *function, char key[1024],
		int *keyLength)
{
	*keyLength = snprintf(key, 1024, "p/%s/%s", (char *)file,
			(char *)function);
}

static PfsEntry *newEntry(const char *file, const char *function)
{
	PfsEntry *result = NULL;

	result = (PfsEntry *)malloc(sizeof(PfsEntry));

	result->ticks = 0;
	result->file = strdup((char *)file);
	result->function = strdup((char *)function);

	return result;
}

static void freeEntry(void *memory)
{
	PfsEntry *entry = NULL;

	if(memory == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	entry = (PfsEntry *)memory;

	if(entry->file != NULL) {
		free(entry->file);
	}

	if(entry->function != NULL) {
		free(entry->function);
	}

	memset(entry, 0, sizeof(PfsEntry));

	free(entry);
}

// define pfs public functions

int pfs_init(Pfs *pfs, aboolean writeLog, char *filename)
{
	if((pfs == NULL) || ((writeLog) && (filename == NULL))) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	memset(pfs, 0, sizeof(Pfs));

	if(bptree_init(&pfs->index) < 0) {
		return -1;
	}

	if(bptree_setFreeFunction(&pfs->index, freeEntry) < 0) {
		return -1;
	}

	if(writeLog) {
		if(log_init(&pfs->log, LOG_OUTPUT_FILE, filename,
					LOG_LEVEL_DEBUG) < 0) {
			return -1;
		}
	}
	else {
		if(log_init(&pfs->log, LOG_OUTPUT_STDOUT, NULL,
					LOG_LEVEL_WARNING) < 0) {
			return -1;
		}
	}

	if(mutex_init(&pfs->mutex) < 0) {
		return -1;
	}

	return 0;
}

int pfs_free(Pfs *pfs)
{
	if(pfs == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&pfs->mutex);

	if(bptree_free(&pfs->index) < 0) {
		return -1;
	}

	if(log_free(&pfs->log) < 0) {
		return -1;
	}

	mutex_unlock(&pfs->mutex);

	if(mutex_free(&pfs->mutex) < 0) {
		return -1;
	}

	return 0;
}

int pfs_profile(Pfs *pfs, const char *file, const char *function)
{
	int keyLength = 0;
	char key[1024];

	PfsEntry *entry = NULL;

	if(pfs == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(file == NULL) {
		file = "unknown";
	}
	if(function == NULL) {
		function = "unknown";
	}

	buildKey(file, function, key, &keyLength);

	mutex_lock(&pfs->mutex);

	if(bptree_get(&pfs->index, key, keyLength, ((void *)&entry)) < 0) {
		entry = newEntry(file, function);
		if(bptree_put(&pfs->index, key, keyLength, (void *)entry) < 0) {
			mutex_unlock(&pfs->mutex);
			freeEntry(entry);
			return -1;
		}
	}

	entry->ticks += 1;

	mutex_unlock(&pfs->mutex);

	log_logf(&pfs->log, LOG_LEVEL_DEBUG, "executed %s:%s()", (char *)file,
			(char *)function);

	return 0;
}

int pfs_buildReport(Pfs *pfs, char **report, int *reportLength)
{
	int ii = 0;
	int threadId = 0;
	int arrayLength = 0;
	int bufferLength = 0;
	int *keyLengths = NULL;
	char statement[1024];
	char *buffer = NULL;
	char **keys = NULL;
	void **values = NULL;

	PfsEntry *entry = NULL;

	const char *template = ""
		"Profile-Function System Report (pid %i):\n"
		"----------------------------------------------------------------\n"
		"%s"
		"----------------------------------------------------------------\n"
		"\n";

	if((pfs == NULL) || (report == NULL) || (reportLength == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&pfs->mutex);

	if(bptree_toArray(&pfs->index, &arrayLength, &keyLengths, &keys,
				&values) < 0) {
		mutex_unlock(&pfs->mutex);
		return -1;
	}

	for(ii = 0; ii < arrayLength; ii++) {
		entry = (PfsEntry *)values[ii];

		bufferLength += ((int)strlen(entry->file) +
				(int)strlen(entry->function) +
				128);

		free(keys[ii]);
	}

	bufferLength += 1024;
	buffer = (char *)malloc(sizeof(char) * bufferLength);

	for(ii = 0; ii < arrayLength; ii++) {
		entry = (PfsEntry *)values[ii];

		snprintf(statement, sizeof(statement),
				"%s:%s() => %lli executions\n",
				entry->file, entry->function, entry->ticks);

		strcat(buffer, statement);
	}

	free(keyLengths);
	free(keys);
	free(values);

	mutex_unlock(&pfs->mutex);

	*reportLength = (bufferLength + (int)strlen((char *)template));
	*report = (char *)malloc(sizeof(char) * (*reportLength));

	thread_self(&threadId);

	*reportLength = snprintf(*report, *reportLength, template, threadId,
			buffer);

	free(buffer);

	return 0;
}

