/*
 * dl_windows.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard dynamic-library interface, Windows plugin.
 *
 * Written by Josh English.
 */

#if defined(WIN32)

// preprocessor directives

#include "core/core.h"
#define _CORE_OS_PLUGIN_COMPONENT
#include "core/os/plugin/dl_windows.h"

// define dl (windows plugin) public functions

int dl_openWindows(char *filename, void **handle)
{
	HMODULE libraryHandle = (HMODULE)NULL;

	if((filename == NULL) || (handle == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	*handle = NULL;

	libraryHandle = LoadLibrary((LPCSTR)filename);
	if(libraryHandle == NULL) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s():%i] error - failed to dl open '%s' with "
					"%i.\n", __FUNCTION__, __LINE__, filename,
					(int)WSAGetLastError());
		}
		return -1;
	}

	*handle = (void *)libraryHandle;

	return 0;
}

int dl_closeWindows(void *handle)
{
	if(handle == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if((int)FreeLibrary((HMODULE)handle) == 0) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s():%i] error - failed to dl close 0x%lx with "
					"%i.\n", __FUNCTION__, __LINE__,
					(unsigned long long)handle, (int)WSAGetLastError());
		}
		return -1;
	}

	return 0;
}

int dl_lookupWindows(void *handle, char *functionName, void **function)
{
	if((handle == NULL) || (functionName == NULL) || (function == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	*function = GetProcAddress((HMODULE)handle, functionName);
	if(*function == NULL) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s():%i] error - failed to dl lookup '%s' "
					"from 0x%lx with %i.\n", __FUNCTION__, __LINE__,
					functionName, (unsigned long long)handle,
					(int)WSAGetLastError());
		}
		return -1;
	}

	return 0;
}

#endif // WIN32

