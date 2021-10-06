/*
 * dl_linux.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard dynamic-library interface, Linux plugin.
 *
 * Written by Josh English.
 */


#if defined(__linux__) || defined(__APPLE__)


// preprocessor directives

#include "core/core.h"
#define _CORE_OS_PLUGIN_COMPONENT
#include "core/os/plugin/dl_linux.h"

// define dl (linux plugin) public functions

int dl_openLinux(char *filename, void **handle)
{
	if((filename == NULL) || (handle == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	*handle = dlopen((const char *)filename, RTLD_LAZY | RTLD_GLOBAL);
	if(*handle == NULL) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s():%i] error - failed to dl open '%s' with "
					"'%s'.\n", __FUNCTION__, __LINE__, filename, dlerror());
		}
		return -1;
	}

	return 0;
}

int dl_closeLinux(void *handle)
{
	if(handle == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(dlclose(handle) != 0) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s():%i] error - failed to dl close 0x%lx with "
					"'%s'.\n", __FUNCTION__, __LINE__,
					(unsigned long int)handle, dlerror());
		}
		return -1;
	}

	return 0;
}

int dl_lookupLinux(void *handle, char *functionName, void **function)
{
	if((handle == NULL) || (functionName == NULL) || (function == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	*function = dlsym(handle, (const char *)functionName);
	if(*function == NULL) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s():%i] error - failed to dl lookup '%s' "
					"from 0x%lx with '%s'.\n", __FUNCTION__, __LINE__,
					functionName, (unsigned long int)handle, dlerror());
		}
		return -1;
	}

	return 0;
}


#endif // __linux__ || __APPLE__

