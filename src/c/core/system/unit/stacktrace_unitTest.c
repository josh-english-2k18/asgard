/*
 * stacktrace_unitTest.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard system-stacktrace function library, unit test.
 *
 * Written by Josh English.
 */

// preprocessor directives

#define _CORE_COMMON_BYPASS_STD_STREAM
#include "core/core.h"

// declare stacktrace unit test private functions

static int stacktrace_basicFunctionalityTest();

// main function

int main()
{
	signal_registerDefault();

	printf("[unit] stacktrace unit test (Asgard Ver %s on %s)\n",
			ASGARD_VERSION, ASGARD_DATE);

	if(stacktrace_basicFunctionalityTest() < 0) {
		printf("[unit]\t ...FAILED\n");
		return 1;
	}

	printf("[unit] ...PASSED\n");

	return 0;
}

// define stacktrace unit test private functions

static int stacktrace_basicFunctionalityTest()
{
	int stackStringLength = 0;
	char *stackString = NULL;

	printf("[unit]\t stacktrace basic functionality test...\n");

	if(stacktrace_getStack(&stackString, &stackStringLength) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t\t unit test obtained stack string 0x%lx length %i\n",
			(aptrcast)stackString, stackStringLength);
	printf("%s", stackString);

	free(stackString);

	printf("[unit]\t ...PASSED\n");

	return 0;
}

