/*
 * event_unitTest.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard-WoW statistical analysis event-parsing library, unit test.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#include "wowstats/common.h"
#include "wowstats/core.h"

// define wowstats event unit test constants

#define BASIC_TEST_FILENAME						\
	"assets/data/test/wowstats.testfile"

#define BASIC_TEST_YEAR							2009

#define BASIC_TEST_PERFORM_INDEXING				atrue

// declare wowstats event unit tests

static int event_basicFunctionalityTest(char *filename);

// main function

int main(int argc, char *argv[])
{
	char *filename = NULL;

	signal_registerDefault();

	printf("[unit] wowstats event unit test (Asgard Ver %s on %s)\n",
			ASGARD_VERSION, ASGARD_DATE);

	if(argc >= 2) {
		filename = argv[1];
	}
	else {
		filename = BASIC_TEST_FILENAME;
	}

	if(event_basicFunctionalityTest(filename) < 0) {
		printf("[unit]\t ...FAILED\n");
		return 1;
	}

	printf("[unit] ...PASSED\n");

	return 0;
}

// define wowstats event unit tests

static int event_basicFunctionalityTest(char *filename)
{
	aboolean fileExists = afalse;
	int rc = 0;
	int keyLength = 0;
	int readLength = 0;
	int lineCounter = 0;
	int parseResult = 0;
	int eventValidCounter = 0;
	int eventDuplicateCounter = 0;
	int eventInvalidCounter = 0;
	int eventUnknownCounter = 0;
	int spellsValidCounter = 0;
	int spellsInvalidCounter = 0;
	alint fileLength = 0;
	double timer = 0.0;
	double detailTimer = 0.0;
	double elapsedTime = 0.0;
	double localTime = 0.0;
	double readTime = 0.0;
	double parseTime = 0.0;
	double parseTimeHigh = 0.0;
	double parseTimeLow = MAX_SIGNED_DOUBLE;
	double eventTime = 0.0;
	double eventKeyTime = 0.0;
	double indexTime = 0.0;
	double eventFreeTime = 0.0;
	char buffer[8192];
	char *ptr = NULL;
	char *key = NULL;

	FileHandle fh;
	WowParser parser;
	WowEvent event;
	WowSpellindex spellIndex;
	WowSpellList *spell = NULL;
	Bptree index;

	printf("[unit]\t wowstats event basic functionality test...\n");
	printf("[unit]\t\t filename            : %s\n", filename);

	system_fileExists(filename, &fileExists);
	if(!fileExists) {
		printf("[unit]\t\t ...unable to execute when file '%s' does not "
				"exist\n", filename);
		return -1;
	}

	if((rc = file_init(&fh, filename, "r", 8192)) < 0) {
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

	timer = time_getTimeMus();

	detailTimer = time_getTimeMus();

	rc = file_readLine(&fh, buffer, sizeof(buffer), &readLength);

	readTime += time_getElapsedMusInSeconds(detailTimer);

	while(rc == 0) {
		lineCounter++;

		detailTimer = time_getTimeMus();

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

		localTime = time_getElapsedMusInSeconds(detailTimer);

		parseTime += localTime;
		if(localTime < parseTimeLow) {
			parseTimeLow = localTime;
		}
		if(localTime > parseTimeHigh) {
			parseTimeHigh = localTime;
		}

		detailTimer = time_getTimeMus();

		parseResult = wowstats_eventInit(&event, BASIC_TEST_YEAR, &parser);

		eventTime += time_getElapsedMusInSeconds(detailTimer);

		if(parseResult == 0) {
			detailTimer = time_getTimeMus();

			if(wowstats_eventBuildKey(&event, &key, &keyLength) < 0) {
				printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
						__LINE__);
				return -1;
			}

			eventKeyTime += time_getElapsedMusInSeconds(detailTimer);

			if(BASIC_TEST_PERFORM_INDEXING) {
				detailTimer = time_getTimeMus();

				ptr = strndup(key, keyLength);
				if(bptree_put(&index, key, keyLength, ptr) < 0) {
					free(ptr);
					eventDuplicateCounter++;
				}
				else {
					eventValidCounter++;
				}

				indexTime += time_getElapsedMusInSeconds(detailTimer);
			}
			else {
				eventValidCounter++;
			}

			free(key);

			if(event.eventId == WOW_EVENT_SPELL_HEAL) {
				if(wowstats_spellindexLookup(&spellIndex,
							((WowEventSpellHealed *)
							 event.eventValue)->spellId, &spell) < 0) {
/*
					printf("[unit]\t\t\t noted unkown spell id: %i (%s)\n",
							((WowEventSpellHealed *)
							 event.eventValue)->spellId,
							((WowEventSpellHealed *)
							 event.eventValue)->spellName);
*/
					spellsInvalidCounter++;
				}
				else {
/*
					printf("[unit]\t\t\t noted valid spell id: %i (%s)\n",
							((WowEventSpellHealed *)
							 event.eventValue)->spellId,
							((WowEventSpellHealed *)
							 event.eventValue)->spellName);
*/
					spellsValidCounter++;
				}
			}
		}
		else if(parseResult == WOWSTATS_EVENT_INVALID_EVENT) {
			eventInvalidCounter++;
		}
		else if(parseResult == WOWSTATS_EVENT_UNKNOWN_EVENT) {
			eventUnknownCounter++;
		}

		detailTimer = time_getTimeMus();

		if(wowstats_eventFree(&event) < 0) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		eventFreeTime += time_getElapsedMusInSeconds(detailTimer);

		detailTimer = time_getTimeMus();

		rc = file_readLine(&fh, buffer, sizeof(buffer), &readLength);

		readTime += time_getElapsedMusInSeconds(detailTimer);
	}

	elapsedTime = time_getElapsedMusInSeconds(timer);

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

	if(wowstats_spellindexFree(&spellIndex) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(bptree_free(&index) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t\t file parsed lines   : %i\n", lineCounter);
	printf("[unit]\t\t total elapsed time  : %0.6f\n", elapsedTime);
	printf("[unit]\t\t line read time      : %0.6f (%0.2f %%)\n", readTime,
			((readTime / elapsedTime) * 100.0));
	printf("[unit]\t\t avg line read time  : %0.6f\n",
			(readTime / (double)lineCounter));
	printf("[unit]\t\t parse time          : %0.6f (%0.2f %%)\n", parseTime,
			((parseTime / elapsedTime) * 100.0));
	printf("[unit]\t\t parse time low      : %0.6f\n", parseTimeLow);
	printf("[unit]\t\t parse time high     : %0.6f\n", parseTimeHigh);
	printf("[unit]\t\t avg parse time      : %0.6f\n",
			(parseTime / (double)lineCounter));
	printf("[unit]\t\t event init time     : %0.6f (%0.2f %%)\n", eventTime,
			((eventTime / elapsedTime) * 100.0));
	printf("[unit]\t\t avg event init time : %0.6f\n",
			(eventTime / (double)lineCounter));
	printf("[unit]\t\t event key time      : %0.6f (%0.2f %%)\n", eventKeyTime,
			((eventKeyTime / elapsedTime) * 100.0));
	printf("[unit]\t\t avg event key time  : %0.6f\n",
			(eventKeyTime / (double)lineCounter));
	printf("[unit]\t\t event free time     : %0.6f (%0.2f %%)\n", eventTime,
			((eventTime / elapsedTime) * 100.0));
	printf("[unit]\t\t avg event free time : %0.6f\n",
			(eventTime / (double)lineCounter));
	printf("[unit]\t\t indexing time       : %0.6f (%0.2f %%)\n", indexTime,
			((indexTime / elapsedTime) * 100.0));
	printf("[unit]\t\t avg indexing time   : %0.6f\n",
			(indexTime / (double)lineCounter));
	printf("[unit]\t\t file parse rate     : %0.2f mBps\n",
			((((double)fileLength / elapsedTime) / 1024.0) / 1024.0));
	printf("[unit]\t\t valid events        : %i\n", eventValidCounter);
	printf("[unit]\t\t duplicate events    : %i\n", eventDuplicateCounter);
	printf("[unit]\t\t invalid events      : %i\n", eventInvalidCounter);
	printf("[unit]\t\t invalid event ratio : %0.2f %%\n",
			(((double)eventInvalidCounter / (double)lineCounter) * 100.0));
	printf("[unit]\t\t unknown events      : %i\n", eventUnknownCounter);
	printf("[unit]\t\t known spells        : %i\n", spellsValidCounter);
	printf("[unit]\t\t unknown spells      : %i\n", spellsInvalidCounter);

	printf("[unit]\t ...PASSED\n");

	return 0;
}

