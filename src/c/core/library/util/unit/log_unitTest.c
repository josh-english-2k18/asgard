/*
 * log_unitTest.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Java log4j-style application logging system, unit test.
 *
 * Written by Josh English.
 */

// preprocessor directives

#define _CORE_COMMON_BYPASS_STD_STREAM
#include "core/core.h"


// declare log unit test constants

#define LOG_BASIC_TEST_FILENAME					"assets/data/test/testfile.log"


// declare log unit tests

static int log_basicFunctionalityTest();


// main function

int main()
{
	signal_registerDefault();

	printf("[unit] log unit test (Asgard Ver %s on %s)\n",
			ASGARD_VERSION, ASGARD_DATE);

	if(log_basicFunctionalityTest() < 0) {
		printf("[unit]\t ...FAILED\n");
		return 1;
	}

	printf("[unit] ...PASSED\n");

	return 0;
}


// define log unit tests

static int log_basicFunctionalityTest()
{
	aboolean fileExists = afalse;
	int ii = 0;
	int logLevel = 0;
	int logOutput = 0;

	Log log;

	printf("[unit]\t log basic functionality test...\n");
	printf("[unit]\t\t log test filename: %s\n",
			LOG_BASIC_TEST_FILENAME);

	system_fileExists(LOG_BASIC_TEST_FILENAME, &fileExists);
	if(fileExists) {
		if(system_fileDelete(LOG_BASIC_TEST_FILENAME) < 0) {
			return -1;
		}
		printf("[unit]\t\t ...found and deleted test file '%s'\n",
				LOG_BASIC_TEST_FILENAME);
	}

	if(log_init(&log, LOG_OUTPUT_NULL, NULL, LOG_LEVEL_DEBUG) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(log_enablePeriodicFlush(&log, 8) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	system_pickRandomSeed();

	for(logLevel = LOG_LEVEL_DEBUG; logLevel <= LOG_LEVEL_PANIC; logLevel++) {
		printf("[unit]\t\t log level: %i\n", logLevel);

		for(logOutput = LOG_OUTPUT_NULL; logOutput <= LOG_OUTPUT_FILE;
				logOutput++) {
			printf("[unit]\t\t log output: %i\n", logOutput);

			if(log_setLevel(&log, logLevel) < 0) {
				printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
						__LINE__);
				return -1;
			}

			if(log_setOutput(&log, logOutput, LOG_BASIC_TEST_FILENAME) < 0) {
				printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
						__LINE__);
				return -1;
			}

			for(ii = 0; ii < 128; ii++) {
				if(log_logf(&log, logLevel, "i %i, s %i, f %f, d %f, s '%s'",
							(int)rand(),
							(short)rand(),
							(float)rand(),
							(double)rand(),
							"this is a test") < 0) {
					printf("[unit]\t\t ...ERROR, failed at line %i, "
							"aborting.\n", __LINE__);
					return -1;
				}
			}
		}
	}

	for(ii = 0; ii < 128; ii++) {
		if(log_logf(NULL, ((rand() % 7) + 1), "i %i, s %i, f %f, d %f, s '%s'",
					(int)rand(),
					(short)rand(),
					(float)rand(),
					(double)rand(),
					"this is a test") < 0) {
			printf("[unit]\t\t ...ERROR, failed at line %i, "
					"aborting.\n", __LINE__);
			return -1;
		}
	}

	if(log_flush(&log) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(log_flush(NULL) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	// cleanup

	if(log_free(&log) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t ...PASSED\n");

	return 0;
}

