/*
 * time_unitTest.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard time library, unit test.
 *
 * Written by Josh English.
 */

// preprocessor directives

#define _CORE_COMMON_BYPASS_STD_STREAM
#include "core/core.h"

// declare time unit test private functions

static int time_basicFunctionalityTest();

// main function

int main()
{
	signal_registerDefault();

	printf("[unit] time unit test (Asgard Ver %s on %s)\n", ASGARD_VERSION,
			ASGARD_DATE);

	if(time_basicFunctionalityTest() < 0) {
		printf("[unit]\t ...FAILED\n");
		return 1;
	}

	printf("[unit] ...PASSED\n");

	return 0;
}

// define time unit test private functions

static int time_basicFunctionalityTest()
{
	int markerSeconds = 0;
	double markerMus = 0.0;
	double elapsedMus = 0.0;
	char timestamp[32];

	printf("[unit]\t time basic functionality test...\n");

	if((markerSeconds = time_getTimeSeconds()) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if((markerMus = time_getTimeMus()) < 0.0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	time_getTimestamp(timestamp);

	printf("[unit]\t\t ...timestamp is '%s'\n", timestamp);

	time_sleep(1);

	printf("[unit]\t\t ...{sleep} elapsed seconds : %i\n",
			time_getElapsedSeconds(markerSeconds));
	printf("[unit]\t\t ...{sleep} elapsed micros  : %0.6f\n",
			time_getElapsedMus(markerMus));

	markerMus = time_getTimeMus();

	time_usleep(1234);

	elapsedMus = time_getElapsedMus(markerMus);
	printf("[unit]\t\t ...{usleep} elapsed seconds : %0.6f\n",
			(elapsedMus / REAL_ONE_MILLION));
	printf("[unit]\t\t ...{usleep} elapsed micros  : %0.6f\n",
			elapsedMus);

	time_nanosleep(8192);

	printf("[unit]\t\t ...{nanosleep} elapsed seconds : %i\n",
			time_getElapsedSeconds(markerSeconds));
	printf("[unit]\t\t ...{nanosleep} elapsed micros  : %0.6f\n",
			time_getElapsedMus(markerMus));

	printf("[unit]\t ...PASSED\n");

	return 0;
}

