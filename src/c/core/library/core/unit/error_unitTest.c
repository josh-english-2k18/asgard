/*
 * error_unitTest.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard core library error management functions, unit test.
 *
 * Written by Josh English.
 */

// preprocessor directives

#define _CORE_COMMON_BYPASS_STD_STREAM
#include "core/core.h"

// declare error unit test private functions

static int error_basicFunctionalityTest();

// main function

int main()
{
	signal_registerDefault();

	printf("[unit] error unit test (Asgard Ver %s on %s)\n", ASGARD_VERSION,
			ASGARD_DATE);

	if(error_basicFunctionalityTest() < 0) {
		printf("[unit]\t ...FAILED\n");
		return 1;
	}

	printf("[unit] ...PASSED\n");

	return 0;
}

// define error unit test private functions

static int error_basicFunctionalityTest()
{
	int ii = 0;
	int uid = 0;
	int tempInt = 0;
	int lineNumber = 0;
	int uidList[ERROR_DEFAULT_ENTRY_LENGTH];
	char errorBuffer[1024];
	char buffer[1024];
	char *errorMessage = NULL;

	Error error;

	printf("[unit]\t error basic functionality test...\n");

	if(error_init(&error) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	for(ii = 0; ii < ERROR_DEFAULT_ENTRY_LENGTH; ii++) {
		lineNumber = ((int)__LINE__ + 2);
		if((uidList[ii] = errorf(&error, "%s() this is error message test #%i",
						__FUNCTION__, (ii + 1))) == 0) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}
	}

	snprintf(errorBuffer, sizeof(errorBuffer), "[%s->%s():%i]", __FILE__,
			__FUNCTION__, lineNumber);

	for(ii = 0; ii < ERROR_DEFAULT_ENTRY_LENGTH; ii++) {
		if(error_getError(&error, uidList[ii], &errorMessage) < 0) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		snprintf(buffer, sizeof(buffer),
				"%s %s() this is error message test #%i",
				errorBuffer, __FUNCTION__, (ii + 1));

		if(strcmp(errorMessage, buffer)) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		printf("[unit]\t\t error message '%s'\n", errorMessage);

		free(errorMessage);
	}

	if(error_getEntryCount(&error, &tempInt) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(tempInt != 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t\t error entry count   : %i\n", tempInt);

	if(error_getEntryLength(&error, &tempInt) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(tempInt != ERROR_DEFAULT_ENTRY_LENGTH) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t\t error entry length  : %i\n", tempInt);

	// now that the error system's buffer is full, determine if error-handling
	// still works correctly

	for(ii = 0; ii < ERROR_DEFAULT_ENTRY_LENGTH; ii++) {
		if((uidList[ii] = errorf(&error, "%s() this is error message test #%i",
						__FUNCTION__, (ii + 1))) == 0) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}
	}

	if(error_getEntryCount(&error, &tempInt) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(tempInt != ERROR_DEFAULT_ENTRY_LENGTH) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t\t error entry count   : %i\n", tempInt);

	if((uid = errorf(&error,
					"this is yet another test of the error system")) == 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(error_getError(&error, uid, &errorMessage) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t\t error message '%s'\n", errorMessage);

	free(errorMessage);

	errorf(NULL, "this is a %s() test at %i", __FUNCTION__, __LINE__);
	errorf(stdout, "this is a %s() test at %i", __FUNCTION__, __LINE__);
	errorf(stderr, "this is a %s() test at %i", __FUNCTION__, __LINE__);

	if(error_free(&error) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t ...PASSED\n");

	return 0;
}

