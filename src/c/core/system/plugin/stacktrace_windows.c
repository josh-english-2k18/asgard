/*
 * stacktrace_windows.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard system-stacktrace function library, Windows plugin.
 *
 * Written by Josh English.
 */

#if defined(WIN32)

// preprocessor directives

#include "core/core.h"
#define _CORE_SYSTEM_PLUGIN_COMPONENT
#include "core/system/plugin/stacktrace_windows.h"


// define stacktrace (windows plugin) data types

#define STACKTRACE_STACK_DEPTH						128


// declare stacktrace (windows plugin) private functions

static int getWin32Backtrace(void **array, int size);


// define stacktrace (windows plugin) private functions

static int getWin32Backtrace(void **array, int size)
{
	array[0] = (void *)GetCurrentProcess();
	return 1;
}

/*
 * TODO: reexamine how to do this
 *

#include "DbgHelp.h"

int getWin32Backtrace(void **array, int size)
{
	HANDLE hProcess = (HANDLE)NULL;
	STACKFRAME64 stackFrame64;
	CONTEXT contextRecord;

	hProcess = GetCurrentProcess();

	if(StackWalk64(IMAGE_FILE_MACHINE_I386,
				hProcess,
				hProcess,
				&stackFrame64,
				&contextRecord,
				NULL,
				NULL,
				NULL,
				NULL)) {
		printf("worked!\n");
	}

	return 0;
}
*/

/*
int getWin32Backtrace(void **array, int size)
{
	int ii = 0;
	int diff = 0;
	unsigned int *rfp = NULL;
	register void *_ebp __asm__ ("ebp");
	register void *_esp __asm__ ("esp");

	for(rfp = *(unsigned int **)_ebp; ((rfp != NULL) && (ii < size));
			rfp = *(unsigned int **)rfp) {
		diff = *rfp - (unsigned int)rfp;
		if(((void *)rfp < _esp) || (diff > (4 * 1024)) || (diff < 0)) {
			break;
		}
		array[ii] = (void *)(rfp[1] - 4);
		ii++;
	}

	return ii;
}
*/

// define stacktrace (windows plugin) public functions

int stacktrace_getStackWindows(char **stackString, int *stackStringLength)
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

	traceStackDepth = getWin32Backtrace((void **)traceStackEntries,
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

#endif // WIN32

