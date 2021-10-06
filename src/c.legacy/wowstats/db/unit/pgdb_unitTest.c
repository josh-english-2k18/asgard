/*
 * pgdb_unitTest.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard-WoW statistical analysis PostgreSQL database interface, unit test.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#include "wowstats/db.h"

// define postgres-db unit test constants

#define BASIC_TEST_HOSTNAME							"localhost"
#define BASIC_TEST_PORT								0
#define BASIC_TEST_USERNAME							"wowcpr"
#define BASIC_TEST_PASSWORD							"wowcpr"
#define BASIC_TEST_FLAGS							NULL
#define BASIC_TEST_DB								"wowcpr"

// declare postgres-db unit tests

static int pgdb_basicFunctionalityTest();

// main function

int main()
{
	signal_registerDefault();

	printf("[unit] wowstats postgres-db unit test (Asgard Ver %s on %s)\n",
			ASGARD_VERSION, ASGARD_DATE);

	if(pgdb_basicFunctionalityTest() < 0) {
		printf("[unit]\t ...FAILED\n");
		return 1;
	}

	printf("[unit] ...PASSED\n");

	return 0;
}

// define pgdb unit tests

static int pgdb_basicFunctionalityTest()
{
	int ii = 0;
	int nn = 0;
	int resultLength = 0;
	double timer = 0.0;
	char *rowResult = NULL;
	void *handle = NULL;

	Pgdb pgdb;

	printf("[unit]\t wowstats postgres-db basic functionality test...\n");
	printf("[unit]\t\t hostname          : %s\n", BASIC_TEST_HOSTNAME);
	printf("[unit]\t\t port              : %i\n", BASIC_TEST_PORT);
	printf("[unit]\t\t username          : %s\n", BASIC_TEST_USERNAME);
	printf("[unit]\t\t database          : %s\n", BASIC_TEST_DB);

	timer = time_getTimeMus();

	if(pgdb_init(&pgdb) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(pgdb_setDataSourceProperties(&pgdb,
				BASIC_TEST_HOSTNAME,
				BASIC_TEST_PORT,
				BASIC_TEST_USERNAME,
				BASIC_TEST_PASSWORD,
				BASIC_TEST_FLAGS,
				BASIC_TEST_DB) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(pgdb_connect(&pgdb) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(!pgdb_isConnected(&pgdb)) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t\t connection time   : %0.6f\n",
			time_getElapsedMusInSeconds(timer));
	timer = time_getTimeMus();

	if(pgdb_executeSql(&pgdb, "select * from cprRealm limit 10;",
				PGRES_TUPLES_OK, &handle) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t\t execution time    : %0.6f\n",
			time_getElapsedMusInSeconds(timer));

	printf("[unit]\t\t\t ");

	for(ii = 0; ii < pgdb_getColumnCount(&pgdb, handle); ii++) {
		printf("| %s\t", pgdb_getColumnName(&pgdb, handle, ii));
	}

	printf("\n");

	for(ii = 0; ii < pgdb_getRowCount(&pgdb, handle); ii++) {
		printf("[unit]\t\t\t ");
		for(nn = 0; nn < pgdb_getColumnCount(&pgdb, handle); nn++) {
			rowResult = pgdb_getRowResult(&pgdb, handle, ii, nn, &resultLength);
			printf("| %s\t", rowResult);
			free(rowResult);
		}
		printf("\n");
	}

	timer = time_getTimeMus();

	if(pgdb_freeHandle(&pgdb, handle) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t\t free handle time  : %0.6f\n",
			time_getElapsedMusInSeconds(timer));
	timer = time_getTimeMus();

	if(pgdb_disconnect(&pgdb) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(pgdb_free(&pgdb) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t\t disconnect time   : %0.6f\n",
			time_getElapsedMusInSeconds(timer));

	printf("[unit]\t ...PASSED\n");

	return 0;
}

