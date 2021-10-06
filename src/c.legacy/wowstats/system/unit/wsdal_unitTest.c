/*
 * wsdal_unitTest.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard-WoW statistical analysis data access layer (DAL), unit test.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#include "wowstats/common.h"
#include "wowstats/core.h"
#include "wowstats/db.h"
#include "wowstats/system.h"

// define data access layer unit test constants

#define BASIC_TEST_HOSTNAME							"localhost"
#define BASIC_TEST_PORT								0
#define BASIC_TEST_USERNAME							"wowcpr"
#define BASIC_TEST_PASSWORD							"wowcpr"
#define BASIC_TEST_FLAGS							NULL
#define BASIC_TEST_DB								"wowcpr"

// declare data access layer unit tests

static int wsdal_basicFunctionalityTest();

// main function

int main()
{
	signal_registerDefault();

	printf("[unit] wowstats DAL unit test (Asgard Ver %s on %s)\n",
			ASGARD_VERSION, ASGARD_DATE);

	if(wsdal_basicFunctionalityTest() < 0) {
		printf("[unit]\t ...FAILED\n");
		return 1;
	}

	printf("[unit] ...PASSED\n");

	return 0;
}

// define data access layer unit tests

static char *buildRandomString(int length)
{
	int ii = 0;
	char *result = NULL;

	system_pickRandomSeed();

	result = (char *)malloc(sizeof(char) * (length + 1));

	for(ii = 0; ii < length; ii++) {
		if((rand() % 100) > 49) {
			result[ii] = (char)((rand() % 26) + 97); // lower-case letter
		}
		else {
			result[ii] = (char)((rand() % 10) + 48); // number
		}
	}

	return result;
}

static int wsdal_basicFunctionalityTest()
{
	int userId = 0;
	int logHistoryId = 0;
	int realmId = 0;
	int entityId = 0;
	int summaryTypeId = 0;
	double timer = 0.0;
	char *entityName = NULL;
	char *entityUid = NULL;
	char *entityClassName = NULL;

	Pgdb pgdb;

	printf("[unit]\t wowstats data access layer basic functionality test...\n");
	printf("[unit]\t\t hostname                          : %s\n",
			BASIC_TEST_HOSTNAME);
	printf("[unit]\t\t port                              : %i\n",
			BASIC_TEST_PORT);
	printf("[unit]\t\t username                          : %s\n",
			BASIC_TEST_USERNAME);
	printf("[unit]\t\t database                          : %s\n",
			BASIC_TEST_DB);

	// connect to the database

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

	printf("[unit]\t\t connection time                   : %0.6f\n",
			time_getElapsedMusInSeconds(timer));

	// wowstats_wsdalGetUserId() test

	timer = time_getTimeMus();

	if(wowstats_wsdalGetUserId(&pgdb, "TestUserEmailAddress", &userId) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t\t wowstats_wsdalGetUserId() time    : %0.6f\n",
			time_getElapsedMusInSeconds(timer));
	printf("[unit]\t\t user ID is                        : %i\n", userId);

	// wowstats_wsdalAddMobLog() test

	timer = time_getTimeMus();

	if(wowstats_wsdalAddMobLog(&pgdb, 1, 1, 1, "Sartharion",
				"somelog.filename", "a Sartharion fight", 5678,
				"2009-03-19 12:59:27") < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t\t wowstats_wsdalAddMobLog() time: %0.6f\n",
			time_getElapsedMusInSeconds(timer));

	// wowstats_wsdalGetLastMobLogId() test

	timer = time_getTimeMus();

	if(wowstats_wsdalGetLastMobLogId(&pgdb, &logHistoryId) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t\t wowstats_wsdalGetLastLogHistoryId()\n");
	printf("[unit]\t\t                            time   : %0.6f\n",
			time_getElapsedMusInSeconds(timer));
	printf("[unit]\t\t mob log ID is                     : %i\n",
		logHistoryId);

	// wowstats_wsdalAddLogHistory() test

	timer = time_getTimeMus();

	if(wowstats_wsdalAddLogHistory(&pgdb, 1, 1, "Raid",
				"some.log.filename", "log file description", 1234,
				"2009-03-19 12:59:27") < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t\t wowstats_wsdalAddLogHistory() time: %0.6f\n",
			time_getElapsedMusInSeconds(timer));

	// wowstats_wsdalGetLastLogHistoryId() test

	timer = time_getTimeMus();

	if(wowstats_wsdalGetLastLogHistoryId(&pgdb, &logHistoryId) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t\t wowstats_wsdalGetLastLogHistoryId()\n");
	printf("[unit]\t\t                            time   : %0.6f\n",
			time_getElapsedMusInSeconds(timer));
	printf("[unit]\t\t log history ID is                 : %i\n",
		logHistoryId);

	// wowstats_wsdalSetLogHistoryPlaySeconds() test

	timer = time_getTimeMus();

	if(wowstats_wsdalSetLogHistoryPlaySeconds(&pgdb, 1, 45678) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t\t wowstats_wsdalSetLogHistoryPlaySeconds()\n");
	printf("[unit]\t\t                            time   : %0.6f\n",
			time_getElapsedMusInSeconds(timer));

	// wowstats_wsdalAddContributingLog() test

	timer = time_getTimeMus();

	if(wowstats_wsdalAddContributingLog(&pgdb, logHistoryId, 1, 1, "Raid",
				"some.other.log.filename", "some other log description",
				"2009-03-19 12:59:27") < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t\t wowstats_wsdalAddContributingLog()\n");
	printf("[unit]\t\t                            time   : %0.6f\n",
			time_getElapsedMusInSeconds(timer));

	// wowstats_wsdalGetRealmId() test

	timer = time_getTimeMus();

	if(wowstats_wsdalGetRealmId(&pgdb, "US", "Aegwynn", &realmId) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t\t wowstats_wsdalGetRealmId() time   : %0.6f\n",
			time_getElapsedMusInSeconds(timer));
	printf("[unit]\t\t realm ID is                       : %i\n", realmId);

	// wowstats_wsdalGetEntityId() test

	timer = time_getTimeMus();

	if(wowstats_wsdalGetEntityId(&pgdb, realmId, "UID", &entityId) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t\t wowstats_wsdalGetEntityId() time  : %0.6f\n",
			time_getElapsedMusInSeconds(timer));
	printf("[unit]\t\t entity ID is                      : %i\n", entityId);

	// wowstats_wsdalAddEntity() test

	entityName = buildRandomString(32);
	entityUid = buildRandomString(32);

	timer = time_getTimeMus();

	if(wowstats_wsdalAddEntity(&pgdb, realmId, "TestClass", entityName,
				entityUid, ((rand() % 1024) + 8)) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t\t wowstats_wsdalAddEntity() time    : %0.6f\n",
			time_getElapsedMusInSeconds(timer));
	printf("[unit]\t\t entity name is                    : %s\n", entityName);
	printf("[unit]\t\t entity UID is                     : %s\n", entityUid);

	free(entityName);

	// wowstats_wsdalGetEntityId() re-test

	timer = time_getTimeMus();

	if(wowstats_wsdalGetEntityId(&pgdb, realmId, entityUid, &entityId) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t\t wowstats_wsdalGetEntityId() time  : %0.6f\n",
			time_getElapsedMusInSeconds(timer));
	printf("[unit]\t\t entity ID is                      : %i\n", entityId);

	free(entityUid);

	// wowstats_wsdalUpdateEntity() test

	entityClassName = buildRandomString(16);

	timer = time_getTimeMus();

	if(wowstats_wsdalUpdateEntity(&pgdb, entityId, entityClassName,
				((rand() % 1024) + 8)) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t\t wowstats_wsdalUpdateEntity() time : %0.6f\n",
			time_getElapsedMusInSeconds(timer));
	printf("[unit]\t\t entity class name is              : %s\n",
			entityClassName);

	free(entityClassName);

	// wowstats_wsdalAddEntityLog() test

	timer = time_getTimeMus();

	if(wowstats_wsdalAddEntityLog(&pgdb, entityId, 1, 9876) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t\t wowstats_wsdalAddEntityLog() time : %0.6f\n",
			time_getElapsedMusInSeconds(timer));

	// wowstats_wsdalGetSummaryTypeId() test

	timer = time_getTimeMus();

	if(wowstats_wsdalGetSummaryTypeId(&pgdb, "Healing", &summaryTypeId) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t\t wowstats_wsdalGetSummaryTypeId()\n");
	printf("[unit]\t\t                           time    : %0.6f\n",
			time_getElapsedMusInSeconds(timer));
	printf("[unit]\t\t summary type ID is                : %i\n",
			summaryTypeId);

	// disconnect from the database

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

	printf("[unit]\t\t disconnect time                   : %0.6f\n",
			time_getElapsedMusInSeconds(timer));

	printf("[unit]\t ...PASSED\n");

	return 0;
}

