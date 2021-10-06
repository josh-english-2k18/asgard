/*
 * bdb_unitTest.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard-WoW statistical analysis BerkeleyDB database interface, unit test.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#include "wowstats/db.h"

// define berkeley-db unit test constants

#define BASIC_TEST_DBNAME							\
	"assets/data/test/berkeley.testfile.bdb"

#define BASIC_TEST_LENGTH							8192

// declare berkeley-db unit tests

static int bdb_basicFunctionalityTest();

// main function

int main()
{
	signal_registerDefault();

	printf("[unit] wowstats berkeley-db unit test (Asgard Ver %s on %s)\n",
			ASGARD_VERSION, ASGARD_DATE);

	if(bdb_basicFunctionalityTest() < 0) {
		printf("[unit]\t ...FAILED\n");
		return 1;
	}

	printf("[unit] ...PASSED\n");

	return 0;
}

// define berkeley-db unit test private functions

static char *buildRandomString(int *length)
{
	int ii = 0;
	char *result = NULL;

	system_pickRandomSeed();

	*length = ((rand() % 1024) + 8);

	result = (char *)malloc(sizeof(char) * (*length + 1));

	for(ii = 0; ii < *length; ii++) {
		if((rand() % 100) > 49) {
			result[ii] = (char)((rand() % 26) + 97); // lower-case letter
		}
		else {
			result[ii] = (char)((rand() % 10) + 48); // number
		}
	}

	return result;
}

// define berkeley-db unit tests

static int bdb_basicFunctionalityTest()
{
	aboolean fileExists = afalse;
	int ii = 0;
	int keyLength = 0;
	int valueLength = 0;
	alint fileLength = 0;
	double timer = 0.0;
	double timing = 0.0;
	double totalTimer = 0.0;
	char *key = NULL;
	char *value = NULL;
	char *keyList[BASIC_TEST_LENGTH];
	char *valueList[BASIC_TEST_LENGTH];

	Bdb bdb;

	printf("[unit]\t wowstats berkeley-db basic functionality test...\n");
	printf("[unit]\t\t filename            : %s\n", BASIC_TEST_DBNAME);

	system_fileExists(BASIC_TEST_DBNAME, &fileExists);
	if(fileExists) {
		if(system_fileDelete(BASIC_TEST_DBNAME) < 0) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}
		printf("[unit]\t\t deleted test file   : %s\n", BASIC_TEST_DBNAME);
	}

	totalTimer = time_getTimeMus();

	if(bdb_init(&bdb, BASIC_TEST_DBNAME) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	// put tests

	timing = 0.0;

	for(ii = 0; ii < BASIC_TEST_LENGTH; ii++) {
		key = buildRandomString(&keyLength);
		value = buildRandomString(&valueLength);

		timer = time_getTimeMus();

		if(bdb_put(&bdb, key, keyLength, value, valueLength) < 0) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		timing += time_getElapsedMusInSeconds(timer);

		keyList[ii] = key;
		valueList[ii] = value;
	}

	printf("[unit]\t\t avg put time        : %0.6f\n",
			(timing / (double)BASIC_TEST_LENGTH));

	// get tests

	timing = 0.0;

	for(ii = 0; ii < BASIC_TEST_LENGTH; ii++) {
		key = keyList[ii];
		keyLength = strlen(key);

		timer = time_getTimeMus();

		if(bdb_get(&bdb, key, keyLength, &value, &valueLength) < 0) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		timing += time_getElapsedMusInSeconds(timer);

		if(strncmp(value, valueList[ii], valueLength)) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}
	}

	printf("[unit]\t\t avg get time        : %0.6f\n",
			(timing / (double)BASIC_TEST_LENGTH));

	// del tests

	timing = 0.0;

	for(ii = 0; ii < BASIC_TEST_LENGTH; ii++) {
		key = keyList[ii];
		keyLength = strlen(key);

		timer = time_getTimeMus();

		if(bdb_del(&bdb, key, keyLength) < 0) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		timing += time_getElapsedMusInSeconds(timer);

		if(bdb_get(&bdb, key, keyLength, &value, &valueLength) == 0) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}
	}

	printf("[unit]\t\t avg del time        : %0.6f\n",
			(timing / (double)BASIC_TEST_LENGTH));

	// cleanup

	for(ii = 0; ii < BASIC_TEST_LENGTH; ii++) {
		free(keyList[ii]);
		free(valueList[ii]);
	}

	if(bdb_free(&bdb) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t\t total test time     : %0.6f\n",
			time_getElapsedMusInSeconds(totalTimer));

	if(system_fileLength(BASIC_TEST_DBNAME, &fileLength) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t\t db file length      : %lli\n", fileLength);

	if(system_fileDelete(BASIC_TEST_DBNAME) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t ...PASSED\n");

	return 0;
}

