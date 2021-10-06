/*
 * trigger_unitTest.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard-WoW statistical analysis trigger system, unit test.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#include "wowstats/common.h"
#define _WOWSTATS_DATA_COMPONENT
#include "wowstats/core.h"
#include "wowstats/db.h"
#include "wowstats/system.h"

// define wowstats trigger system unit test constants

#define BASIC_TEST_FILENAME						\
	"assets/data/test/wowstats.testfile"

#define BASIC_TEST_REALM_ID						313

#define BASIC_TEST_YEAR							2009

// declare wowstats trigger system unit tests

static int trigger_basicFunctionalityTest(char *filename);

// main function

int main(int argc, char *argv[])
{
	char *filename = NULL;

	signal_registerDefault();

	printf("[unit] wowstats trigger system unit test (Asgard Ver %s on %s)\n",
			ASGARD_VERSION, ASGARD_DATE);

	if(argc >= 2) {
		filename = argv[1];
	}
	else {
		filename = BASIC_TEST_FILENAME;
	}

	if(trigger_basicFunctionalityTest(filename) < 0) {
		printf("[unit]\t ...FAILED\n");
		return 1;
	}

	printf("[unit] ...PASSED\n");

	return 0;
}

// define wowstats trigger system unit tests

static int trigger_basicFunctionalityTest(char *filename)
{
	aboolean fileExists = afalse;
	int rc = 0;
	int readLength = 0;
	int lineCounter = 0;
	int parseResult = 0;
	int elapsedSeconds = 0;
	int eventValidCounter = 0;
	int eventInvalidCounter = 0;
	int eventUnknownCounter = 0;
	int playersValidCounter = 0;
	int playersInvalidCounter = 0;
	alint fileLength = 0;
	double timer = 0.0;
	double elapsedTime = 0.0;
	char buffer[8192];

	FileHandle fh;

	WowParser parser;
	WowEvent event;
	WowSpellindex spellIndex;
	WowPlayerIndex playerIndex;
	WowTriggerResult triggerResult;
	WowTrigger trigger;

	printf("[unit]\t wowstats trigger system basic functionality test...\n");
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

	if(wowstats_triggerInit(&trigger) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	timer = time_getTimeMus();

	rc = file_readLine(&fh, buffer, sizeof(buffer), &readLength);
	while(rc == 0) {
		lineCounter++;

		if(wowstats_parserParse(&parser, buffer, readLength) < 0) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		parseResult = wowstats_eventInit(&event, BASIC_TEST_YEAR, &parser);
		if(parseResult == 0) {
			eventValidCounter++;

			if(wowstats_playerIndexNoteEvent(&playerIndex, &event) < 0) {
				playersInvalidCounter++;
			}
			else {
				playersValidCounter++;
			}

			if((rc = wowstats_triggerNoteEvent(&trigger, &playerIndex, &event,
							&triggerResult)) < 0) {
				printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
						__LINE__);
				return -1;
			}

			switch(rc) {
				case WOW_TRIGGER_RESULT_NEW_BOSS:
				case WOW_TRIGGER_RESULT_END_BOSS:
				case WOW_TRIGGER_RESULT_NEW_AND_END_BOSS:
				case WOW_TRIGGER_RESULT_END_LOG:
					printf("[unit]\t\t\t trigger event [ ");
					if(rc == WOW_TRIGGER_RESULT_NEW_BOSS) {
						printf("NEW BOSS");
					}
					else if(rc == WOW_TRIGGER_RESULT_END_BOSS) {
						printf("END BOSS");
					}
					else if(rc == WOW_TRIGGER_RESULT_NEW_AND_END_BOSS) {
						printf("NEW & END BOSS");
					}
					else if(rc == WOW_TRIGGER_RESULT_END_LOG) {
						printf("END LOG");
					}
					printf(" ]");
					if(triggerResult.zone != NULL) {
						printf(" { zone '%s', boss '%s' }",
								triggerResult.zone,
								triggerResult.bossName);
					}
					if(wowstats_timestampCalculateDifferenceInSeconds(
								&triggerResult.endTimestamp,
								&triggerResult.startTimestamp,
								&elapsedSeconds) < 0) {
						printf("[unit]\t\t ...ERROR, failed at line %i, "
								"aborting.\n", __LINE__);
						return -1;
					}
					printf(" %i seconds", elapsedSeconds);

					printf("\n");
					break;
			}

			if((rc == WOW_TRIGGER_RESULT_NEW_BOSS) ||
					(rc == WOW_TRIGGER_RESULT_END_BOSS) ||
					(rc == WOW_TRIGGER_RESULT_NEW_AND_END_BOSS)) {
				if(wowstats_playerIndexFree(&playerIndex) < 0) {
					printf("[unit]\t\t ...ERROR, failed at line %i, "
							"aborting.\n", __LINE__);
					return -1;
				}
				if(wowstats_playerIndexInit(&playerIndex, &spellIndex) < 0) {
					printf("[unit]\t\t ...ERROR, failed at line %i, "
							"aborting.\n", __LINE__);
					return -1;
				}
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

	if((rc = file_free(&fh)) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	elapsedTime = time_getElapsedMusInSeconds(timer);

	if(wowstats_playerIndexFree(&playerIndex) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(wowstats_spellindexFree(&spellIndex) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(wowstats_triggerFree(&trigger) < 0) {
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
	printf("[unit]\t\t invalid events        : %i\n", eventInvalidCounter);
	printf("[unit]\t\t unknown events        : %i\n", eventUnknownCounter);
	printf("[unit]\t\t valid player events   : %i\n", playersValidCounter);
	printf("[unit]\t\t invalid player events : %i\n", playersInvalidCounter);

	printf("[unit]\t ...PASSED\n");

	return 0;
}

