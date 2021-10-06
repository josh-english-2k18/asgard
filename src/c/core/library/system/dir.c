/*
 * dir.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard core library dir-handling functions.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#define _CORE_LIBRARY_SYSTEM_COMPONENT
#include "core/library/system/dir.h"

// define dir-handling public functions

int dir_init(DirHandle *handle, char *directoryName)
{
	int result = 0;

	if((handle == NULL) || (directoryName == NULL)) {
		result = errorf(stderr, "invalid or missing argument(s) { handle "
				"0x%lx, directoryName 0x%lx }",
				(aptrcast)handle, (aptrcast)directoryName);
		return result;
	}

	memset(handle, 0, (sizeof(DirHandle)));

	handle->directoryName = strdup(directoryName);
	handle->dir = NULL;

	error_init(&handle->errors);

	if(system_fileSetNative(handle->directoryName) < 0) {
		result = errorf(&handle->errors,
				"failed to set dir '%s' to native directory name",
				handle->directoryName);
		return result;
	}

	handle->dir = opendir(handle->directoryName);
	if(handle->dir == NULL) {
		result = errorf(&handle->errors,
				"failed to open directory '%s' with '%s'",
				handle->directoryName, strerror(errno));
		return result;
	}

	return result;
}

int dir_free(DirHandle *handle)
{
	int result = 0;

	if(handle == NULL) {
		result = errorf(stderr, "invalid or missing argument(s) { handle "
				"0x%lx }", (aptrcast)handle);
		return result;
	}

	result = closedir(handle->dir);
	if(result != 0) {
		result = errorf(stderr, "failed to close directory '%s' with '%s'",
				handle->directoryName, strerror(errno));
	}

	if(handle->directoryName != NULL) {
		free(handle->directoryName);
	}

	error_free(&handle->errors);

	memset(handle, 0, (sizeof(DirHandle)));

	return result;
}

int dir_read(DirHandle *handle, char **filename)
{
	int result = 0;

	struct dirent *entry = NULL;

	if((handle == NULL) || (filename == NULL)) {
		result = errorf(stderr, "invalid or missing argument(s) { handle "
				"0x%lx, filename 0x%lx }", (aptrcast)handle,
				(aptrcast)filename);
		return result;
	}

	entry = readdir(handle->dir);
	if(entry == NULL) {
		result = errorf(&handle->errors,
				"failed to read directory '%s' with '%s'",
				handle->directoryName, strerror(errno));
		return result;
	}

	*filename = strdup(entry->d_name);

	return result;
}

int dir_getError(DirHandle *handle, int errorCode, char **errorMessage)
{
	int result = 0;

	if((handle == NULL) || (errorMessage == NULL)) {
		result = errorf(stderr, "invalid or missing argument(s) { handle "
				"0x%lx, errorMessage 0x%lx }", (aptrcast)handle,
				(aptrcast)errorMessage);
		return result;
	}

	return error_getError(&handle->errors, errorCode, errorMessage);
}

