/*
 * stacktrace_linux.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard system-stacktrace function library, Linux plugin.
 *
 * Written by Josh English.
 */


#if (defined(__linux__) || defined(__APPLE__)) && !defined(__ANDROID__)


// preprocessor directives

#include "core/core.h"
#define _CORE_SYSTEM_PLUGIN_COMPONENT
#include "core/system/plugin/stacktrace_linux.h"


// define stacktrace (linux plugin) data types

#define STACKTRACE_STACK_DEPTH						128


// define stacktrace (linux plugin) public functions

int stacktrace_getStackLinux(char **stackString, int *stackStringLength)
{
	int ii = 0;
	int traceStackDepth = 0;
	int resultLength = 0;
	char buffer[8192];
	void *traceStackEntries[STACKTRACE_STACK_DEPTH];
	char **traceStackStrings = NULL;
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

	traceStackDepth = backtrace((void **)traceStackEntries,
			STACKTRACE_STACK_DEPTH);

	// fill the stacktrace pointers with strings

	traceStackStrings = backtrace_symbols((void **)traceStackEntries,
			(size_t)traceStackDepth);

	// allocate and populate result string

	for(ii = 0; ii < traceStackDepth; ii++) {
		resultLength += (strlen(traceStackStrings[ii]) + 128);
	}

	result = (char *)malloc(sizeof(char) * resultLength);

	for(ii = 0; ii < traceStackDepth; ii++) {
		snprintf(buffer, (sizeof(buffer) - 1),
				"[stack %03i] (0x%12lx) %s", (traceStackDepth - ii),
				(unsigned long int)traceStackEntries[ii],
				traceStackStrings[ii]);
		strcat(result, buffer);
		strcat(result, "\n");
	}

	// assign results

	*stackString = result;
	*stackStringLength = strlen(result);

	// cleanup

	free(traceStackStrings);

	return 0;
}

#endif // _CORE_SYSTEM_PLUGIN_STACKTRACE_LINUX_H

