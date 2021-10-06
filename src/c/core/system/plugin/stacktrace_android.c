/*
 * stacktrace_android.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard system-stacktrace function library, Android plugin.
 *
 * Written by Josh English.
 */

#if defined(__ANDROID__)

// preprocessor directives

#include "core/core.h"
#define _CORE_SYSTEM_PLUGIN_COMPONENT
#include "core/system/plugin/stacktrace_android.h"


// define stacktrace (android plugin) data types

#define STACKTRACE_STACK_DEPTH						128


// declare stacktrace (android plugin) private functions

static int getAndroidBacktrace(void **array, int size);


// define stacktrace (android plugin) private functions

static int getAndroidBacktrace(void **array, int size)
{
	array[0] = (void *)"(unknown)";
	return 1;
}


// define stacktrace (android plugin) public functions

int stacktrace_getStackAndroid(char **stackString, int *stackStringLength)
{
	int ii = 0;
	int traceStackDepth = 0;
	int resultLength = 0;
	char buffer[8192];
	void *traceStackEntries[STACKTRACE_STACK_DEPTH];
	char *result = NULL;

	if((stackString == NULL) || (stackStringLength == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	*stackString = NULL;
	*stackStringLength = 0;

	// initialize stack entries

	for(ii = 0; ii < STACKTRACE_STACK_DEPTH; ii++) {
		traceStackEntries[ii] = NULL;
	}

	// obtain the stacktrace pointers

	traceStackDepth = getAndroidBacktrace((void **)traceStackEntries,
			STACKTRACE_STACK_DEPTH);

	result = (char *)malloc(sizeof(char) * traceStackDepth * 128);

	for(ii = 0; ii < traceStackDepth; ii++) {
		snprintf(buffer, (sizeof(buffer) - 1),
				"[stack %03i] (0x%12llx) %s", (traceStackDepth - ii),
				(aulint)traceStackEntries[ii],
				"addr");
		strcat(result, buffer);
		strcat(result, "\n");
	}

	// assign results

	*stackString = result;
	*stackStringLength = (int)strlen(result);

	return 0;
}

#endif // __ANDROID__

