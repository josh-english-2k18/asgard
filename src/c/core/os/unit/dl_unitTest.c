/*
 * dl_unitTest.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard dynamic-library interface, unit test.
 *
 * Written by Josh English.
 */

// preprocessor directives

#define _CORE_COMMON_BYPASS_STD_STREAM
#include "core/core.h"

// define dl unit test constants

#if defined(__linux__)
#	define LIBRARY						"lib/libasgardcore_unitTest.so"
#	define FUNCTION_NAME				"dl_open"
#elif defined(__APPLE__)
#	define LIBRARY						"lib/libasgardcore_unitTest.dylib"
#	define FUNCTION_NAME				"dl_open"
#elif defined(WIN32)
#	define LIBRARY						"vorbisfile.dll"
#	define FUNCTION_NAME				"ov_clear"
#endif // plugins

// declare dl unit test private functions

static int dl_basicFunctionalityTest();

// main function

int main()
{
	signal_registerDefault();

	printf("[unit] dl unit test (Asgard Ver %s on %s)\n", ASGARD_VERSION,
			ASGARD_DATE);

	if(dl_basicFunctionalityTest() < 0) {
		printf("[unit]\t ...FAILED\n");
		return 1;
	}

	printf("[unit] ...PASSED\n");

	return 0;
}

// define dl unit test private functions

static int dl_basicFunctionalityTest()
{
	void *handle = NULL;
	void *function = NULL;

	printf("[unit]\t dl basic functionality test...\n");

	if(dl_open(LIBRARY, &handle) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(handle == NULL) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(dl_lookup(handle, FUNCTION_NAME, &function) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(function == NULL) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(dl_close(handle) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t ...PASSED\n");

	return 0;
}

