/*
 * dir_unitTest.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard core library dir-handling functions, unit test.
 *
 * Written by Josh English.
 */

// preprocessor directives

#define _CORE_COMMON_BYPASS_STD_STREAM
#include "core/core.h"

// define dir-handling unit test constants

#define BASICTEST_DIRNAME							"assets/data/test"

// declare dir-handling unit tests

static int dir_basicFunctionalityTest();

// main function

int main()
{
	signal_registerDefault();

	printf("[unit] dir-handling unit test (Asgard Ver %s on %s)\n",
			ASGARD_VERSION, ASGARD_DATE);

	if(dir_basicFunctionalityTest() < 0) {
		printf("[unit]\t ...FAILED\n");
		return 1;
	}

	printf("[unit] ...PASSED\n");

	return 0;
}

// define dir-handling unit tests

static int dir_basicFunctionalityTest()
{
	int rc = 0;
	char *filename = NULL;
	char *errorMessage = NULL;

	DirHandle handle;

	printf("[unit]\t dir-handling basic functionality test...\n");

	if(dir_init(&handle, BASICTEST_DIRNAME) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t\t directory '%s' file list:\n", BASICTEST_DIRNAME);

	rc = dir_read(&handle, &filename);
	while(rc == 0) {
		printf("[unit]\t\t\t filename: '%s'\n", filename);
		free(filename);
		rc = dir_read(&handle, &filename);
	}

	if(rc < 0) {
		if(dir_getError(&handle, rc, &errorMessage) < 0) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}
		printf("[unit]\t\t finished reading directory with '%s'\n",
				errorMessage);
		free(errorMessage);
	}

	if(dir_free(&handle) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t ...PASSED\n");

	return 0;
}

