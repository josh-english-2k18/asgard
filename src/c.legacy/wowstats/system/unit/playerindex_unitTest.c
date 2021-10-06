/*
 * playerindex_unitTest.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard-WoW statistical analysis player-index system, unit test.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#include "wowstats/common.h"
#define _WOWSTATS_DATA_COMPONENT
#include "wowstats/data/playerclass.h"
#include "wowstats/core.h"
#include "wowstats/db.h"
#include "wowstats/system.h"

// define wowstats player-index unit test constants

#define BASIC_TEST_HOSTNAME						"localhost"
#define BASIC_TEST_PORT							0
#define BASIC_TEST_USERNAME						"wowcpr"
#define BASIC_TEST_PASSWORD						"wowcpr"
#define BASIC_TEST_FLAGS						NULL
#define BASIC_TEST_DB							"wowcpr"

#define BASIC_TEST_FILENAME						\
	"assets/data/test/wowstats.testfile"

#define BASIC_TEST_REALM_ID						313

#define BASIC_TEST_YEAR							2009

// declare wowstats player-index unit tests

static int playerindex_basicFunctionalityTest(char *filename);

// main function

int main(int argc, char *argv[])
{
	char *filename = NULL;

	signal_registerDefault();

	printf("[unit] wowstats player-index unit test (Asgard Ver %s on %s)\n",
			ASGARD_VERSION, ASGARD_DATE);

	if(argc >= 2) {
		filename = argv[1];
	}
	else {
		filename = BASIC_TEST_FILENAME;
	}

	if(playerindex_basicFunctionalityTest(filename) < 0) {
		printf("[unit]\t ...FAILED\n");
		return 1;
	}

	printf("[unit] ...PASSED\n");

	return 0;
}

// define wowstats player-index unit tests

static char *classIdToName(int classId)
{
	int ii = 0;

	for(ii = 0;
			WOW_PLAYER_CLASS_LIST[ii].id != WOW_PLAYER_CLASS_END_OF_LIST;
			ii++) {
		if(WOW_PLAYER_CLASS_LIST[ii].id == classId) {
			return WOW_PLAYER_CLASS_LIST[ii].name;
		}
	}

	return "Error";
}

static int playerindex_basicFunctionalityTest(char *filename)
{
	aboolean fileExists = afalse;
	int ii = 0;
	int rc = 0;
	int entityId = 0;
	int keyLength = 0;
	int readLength = 0;
	int lineCounter = 0;
	int parseResult = 0;
	int eventValidCounter = 0;
	int eventDuplicateCounter = 0;
	int eventInvalidCounter = 0;
	int eventUnknownCounter = 0;
	int playersValidCounter = 0;
	int playersInvalidCounter = 0;
	int playerSummaryCount = 0;
	int databaseUpdateCount = 0;
	alint fileLength = 0;
	double timer = 0.0;
	double elapsedTime = 0.0;
	double databaseUpdateTime = 0.0;
	char buffer[8192];
	char *ptr = NULL;
	char *key = NULL;

	FileHandle fh;
	Bptree index;

	WowParser parser;
	WowEvent event;
	WowSpellindex spellIndex;
	WowPlayerIndex playerIndex;
	WowPlayerSummary **playerSummaries = NULL;

	Pgdb pgdb;

	printf("[unit]\t wowstats player-index basic functionality test...\n");
	printf("[unit]\t\t filename            : %s\n", filename);

	system_fileExists(filename, &fileExists);
	if(!fileExists) {
		printf("[unit]\t\t ...unable to execute when file '%s' does not "
				"exist\n", filename);
		return -1;
	}

	if((rc = file_init(&fh, filename, "r", 65536)) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if((rc = file_getFileLength(&fh, &fileLength)) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t\t file length         : %lli\n", fileLength);

	bptree_init(&index);

	if(wowstats_spellindexInit(&spellIndex) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(wowstats_playerIndexInit(&playerIndex, &spellIndex) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	timer = time_getTimeMus();

	rc = file_readLine(&fh, buffer, sizeof(buffer), &readLength);
	while(rc == 0) {
		lineCounter++;

		if(wowstats_parserInit(&parser) < 0) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		if(wowstats_parserParse(&parser, buffer, readLength) < 0) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		parseResult = wowstats_eventInit(&event, BASIC_TEST_YEAR, &parser);
		if(parseResult == 0) {
			if(wowstats_eventBuildKey(&event, &key, &keyLength) < 0) {
				printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
						__LINE__);
				return -1;
			}

			ptr = strndup(key, keyLength);
			if(bptree_put(&index, key, keyLength, ptr) < 0) {
				free(ptr);
				eventDuplicateCounter++;
			}
			else {
				eventValidCounter++;
			}

			free(key);

			if(wowstats_playerIndexNoteEvent(&playerIndex, &event) < 0) {
				playersInvalidCounter++;
			}
			else {
				playersValidCounter++;
			}

			if(wowstats_eventFree(&event) < 0) {
				printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
						__LINE__);
				return -1;
			}
		}
		else if(parseResult == WOWSTATS_EVENT_INVALID_EVENT) {
			eventInvalidCounter++;
		}
		else if(parseResult == WOWSTATS_EVENT_UNKNOWN_EVENT) {
			eventUnknownCounter++;
		}

		rc = file_readLine(&fh, buffer, sizeof(buffer), &readLength);
	}

	if(rc < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(wowstats_playerIndexGetSummaries(&playerIndex, &playerSummaries,
				&playerSummaryCount) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if((rc = file_free(&fh)) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(bptree_free(&index) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	elapsedTime = time_getElapsedMusInSeconds(timer);

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

	for(ii = 0; ii < playerSummaryCount; ii++) {
		printf("[unit]\t\t player summary for '%s' (%s):\n",
				playerSummaries[ii]->uid, playerSummaries[ii]->name);
		printf("[unit]\t\t\t class id        : %i (%s)\n",
				playerSummaries[ii]->classId,
				classIdToName(playerSummaries[ii]->classId));
		printf("[unit]\t\t\t first timestamp : %02i/%02i/%04i - "
				"%02i:%02i:%02i.%04i\n",
				playerSummaries[ii]->firstTimestamp.month,
				playerSummaries[ii]->firstTimestamp.day,
				playerSummaries[ii]->firstTimestamp.year,
				playerSummaries[ii]->firstTimestamp.hour,
				playerSummaries[ii]->firstTimestamp.minute,
				playerSummaries[ii]->firstTimestamp.second,
				playerSummaries[ii]->firstTimestamp.millis);
		printf("[unit]\t\t\t last timestamp  : %02i/%02i/%04i - "
				"%02i:%02i:%02i.%04i\n",
				playerSummaries[ii]->lastTimestamp.month,
				playerSummaries[ii]->lastTimestamp.day,
				playerSummaries[ii]->lastTimestamp.year,
				playerSummaries[ii]->lastTimestamp.hour,
				playerSummaries[ii]->lastTimestamp.minute,
				playerSummaries[ii]->lastTimestamp.second,
				playerSummaries[ii]->lastTimestamp.millis);

		if(wowstats_timestampCalculateDifferenceInSeconds(
					&playerSummaries[ii]->lastTimestamp,
					&playerSummaries[ii]->firstTimestamp,
					&rc) < 0) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		printf("[unit]\t\t\t play time       : %i seconds\n", rc);
		printf("[unit]\t\t\t is alive        : %i\n",
				playerSummaries[ii]->isPlayerAlive);

		timer = time_getTimeMus();

		if(wowstats_wsdalGetEntityId(&pgdb,
					BASIC_TEST_REALM_ID, // realm id
					playerSummaries[ii]->uid, // uid
					&entityId // entity id
					) < 0) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		if(entityId == -1) {
			if(wowstats_wsdalAddEntity(&pgdb,
						BASIC_TEST_REALM_ID, // realm id
						classIdToName(playerSummaries[ii]->classId), // class
						playerSummaries[ii]->name, // name
						playerSummaries[ii]->uid, // uid
						rc // seconds online
						) < 0) {
				printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
						__LINE__);
				return -1;
			}
			databaseUpdateCount++;
		}

		databaseUpdateTime += time_getElapsedMusInSeconds(timer);
	}

	free(playerSummaries);

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

	if(wowstats_spellindexFree(&spellIndex) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(wowstats_playerIndexFree(&playerIndex) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t\t file parsed lines     : %i\n", lineCounter);
	printf("[unit]\t\t file parse time       : %0.6f\n", elapsedTime);
	printf("[unit]\t\t avg line parse time   : %0.6f\n",
			(elapsedTime / (double)lineCounter));
	printf("[unit]\t\t file parse rate       : %0.2f mBps\n",
			((((double)fileLength / elapsedTime) / 1024.0) / 1024.0));
	printf("[unit]\t\t valid events          : %i\n", eventValidCounter);
	printf("[unit]\t\t duplicate events      : %i\n", eventDuplicateCounter);
	printf("[unit]\t\t invalid events        : %i\n", eventInvalidCounter);
	printf("[unit]\t\t unknown events        : %i\n", eventUnknownCounter);
	printf("[unit]\t\t valid player events   : %i\n", playersValidCounter);
	printf("[unit]\t\t invalid player events : %i\n", playersInvalidCounter);
	printf("[unit]\t\t player summaries      : %i\n", playerSummaryCount);
	printf("[unit]\t\t database updates      : %i\n", databaseUpdateCount);
	printf("[unit]\t\t database update time  : %0.6f\n", databaseUpdateTime);
	printf("[unit]\t\t avg database update   : %0.6f\n",
			(databaseUpdateTime / (double)databaseUpdateCount));

	printf("[unit]\t ...PASSED\n");

	return 0;
}

