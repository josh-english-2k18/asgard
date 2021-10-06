/*
 * summaryindex_unitTest.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard-WoW statistical analysis summary-index, unit test.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#include "wowstats/common.h"
#include "wowstats/core.h"
#include "wowstats/db.h"
#include "wowstats/system.h"

// define wowstats summary-index unit test constants

#define BASIC_TEST_HOSTNAME						"localhost"
#define BASIC_TEST_PORT							0
#define BASIC_TEST_USERNAME						"wowcpr"
#define BASIC_TEST_PASSWORD						"wowcpr"
#define BASIC_TEST_FLAGS						NULL
#define BASIC_TEST_DB							"wowcpr"

#define BASIC_TEST_FILENAME						\
	"assets/data/test/wowstats.testfile"

#define BASIC_TEST_LOG_HISTORY_ID				1

#define BASIC_TEST_REALM_ID						313

#define BASIC_TEST_YEAR							2009

// declare wowstats summary-index unit tests

static int summaryindex_basicFunctionalityTest(char *filename);

// main function

int main(int argc, char *argv[])
{
	char *filename = NULL;

	signal_registerDefault();

	printf("[unit] wowstats summary-index unit test (Asgard Ver %s on %s)\n",
			ASGARD_VERSION, ASGARD_DATE);

	if(argc >= 2) {
		filename = argv[1];
	}
	else {
		filename = BASIC_TEST_FILENAME;
	}

	if(summaryindex_basicFunctionalityTest(filename) < 0) {
		printf("[unit]\t ...FAILED\n");
		return 1;
	}

	printf("[unit] ...PASSED\n");

	return 0;
}

// define wowstats summary-index unit tests

static int summaryindex_basicFunctionalityTest(char *filename)
{
	aboolean fileExists = afalse;
	int ii = 0;
	int nn = 0;
	int rc = 0;
	int keyLength = 0;
	int readLength = 0;
	int lineCounter = 0;
	int parseResult = 0;
	int summaryResult = 0;
	int eventValidCounter = 0;
	int eventDuplicateCounter = 0;
	int eventInvalidCounter = 0;
	int eventUnknownCounter = 0;
	int summaryCounter = 0;
	int summaryNotApplicableCounter = 0;
	int summaryBadEventCounter = 0;
	int summaryIndexCounter = 0;
	int summaryIndexErrorCounter = 0;
	int summaryRecordCounter = 0;
	int summaryListLength = 0;
	int summaryRecordListLength = 0;
	int databaseUpdateCount = 0;
	alint fileLength = 0;
	double timer = 0.0;
	double elapsedTime = 0.0;
	double databaseUpdateTime = 0.0;
	char buffer[8192];
	char *ptr = NULL;
	char *key = NULL;
	char **summaryList = NULL;
	char **summaryRecordList = NULL;

	FileHandle fh;
	Bptree index;

	WowParser parser;
	WowEvent event;
	WowSpellindex spellIndex;
	WowLinkIndex linkIndex;
	WowSummary *summary = NULL;
	WowSummaryIndex summaryIndex;

	Pgdb pgdb;

	printf("[unit]\t wowstats summary-index basic functionality test...\n");
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

	if(wowstats_linkIndexInit(&linkIndex) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(wowstats_summaryIndexInit(&summaryIndex, &linkIndex) < 0) {
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
			wowstats_linkIndexNoteEvent(&linkIndex, &event);

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

				summaryResult = wowstats_summaryNew(&summary, &spellIndex,
						&event);
				if(summaryResult == 0) {
					if(wowstats_summaryIndexUpdateSummary(&summaryIndex,
								summary) < 0) {
						summaryIndexErrorCounter++;
					}
					else {
						summaryIndexCounter++;
					}
					summaryCounter++;
				}
				else if(summaryResult == WOW_SUMMARY_EVENT_NOT_APPLICABLE) {
					summaryNotApplicableCounter++;
				}
				else if(summaryResult == WOW_SUMMARY_BAD_EVENT) {
					summaryBadEventCounter++;
				}

				wowstats_summaryFreePtr(summary);
			}

			free(key);

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

	elapsedTime = time_getElapsedMusInSeconds(timer);

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

	if(wowstats_linkIndexFree(&linkIndex) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

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

	if(wowstats_summaryIndexListSummaries(&summaryIndex, &summaryList,
				&summaryListLength) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	for(ii = 0; ii < summaryListLength; ii++) {
		printf("[unit]\t\t summary [%8i]: '%s'\n", ii, summaryList[ii]);

		if(wowstats_summaryIndexListSummaryRecords(&summaryIndex,
					summaryList[ii], &summaryRecordList,
					&summaryRecordListLength) < 0) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		for(nn = 0; nn < summaryRecordListLength; nn++) {
			printf("[unit]\t\t\t record [%8i]: '%s'\n", nn,
					summaryRecordList[nn]);

			if(wowstats_summaryIndexGetSummary(&summaryIndex, summaryList[ii],
						summaryRecordList[nn], &summary) < 0) {
				printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
						__LINE__);
				return -1;
			}

			summaryRecordCounter++;

//			wowstats_summaryDisplay(summary);

			if(summary->summaryId == WOW_SUMMARY_HEALING) {
				timer = time_getTimeMus();

				if(wowstats_wsdalAddSummaryHealing(&pgdb,
							BASIC_TEST_LOG_HISTORY_ID, // log history id
							BASIC_TEST_REALM_ID, // realm id
							summary) < 0) {
					printf("[unit]\t\t ...ERROR, failed at line %i, "
							"aborting.\n", __LINE__);
					return -1;
				}

				databaseUpdateCount++;
				databaseUpdateTime += time_getElapsedMusInSeconds(timer);
			}
			else if(summary->summaryId == WOW_SUMMARY_DAMAGE) {
				timer = time_getTimeMus();

				if(wowstats_wsdalAddSummaryDamage(&pgdb,
							BASIC_TEST_LOG_HISTORY_ID, // log history id
							BASIC_TEST_REALM_ID, // realm id
							summary) < 0) {
					printf("[unit]\t\t ...ERROR, failed at line %i, "
							"aborting.\n", __LINE__);
					return -1;
				}

				databaseUpdateCount++;
				databaseUpdateTime += time_getElapsedMusInSeconds(timer);
			}

			free(summaryRecordList[nn]);
		}

		free(summaryRecordList);

		free(summaryList[ii]);
	}

	free(summaryList);

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

	if(wowstats_summaryIndexFree(&summaryIndex) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t\t file parsed lines        : %i\n", lineCounter);
	printf("[unit]\t\t file parse time          : %0.6f\n", elapsedTime);
	printf("[unit]\t\t avg line parse time      : %0.6f\n",
			(elapsedTime / (double)lineCounter));
	printf("[unit]\t\t file parse rate          : %0.2f mBps\n",
			((((double)fileLength / elapsedTime) / 1024.0) / 1024.0));
	printf("[unit]\t\t valid events             : %i\n", eventValidCounter);
	printf("[unit]\t\t duplicate events         : %i\n",
			eventDuplicateCounter);
	printf("[unit]\t\t invalid events           : %i\n", eventInvalidCounter);
	printf("[unit]\t\t unknown events           : %i\n", eventUnknownCounter);
	printf("[unit]\t\t good summaries           : %i\n", summaryCounter);
	printf("[unit]\t\t not-applicable summaries : %i\n",
			summaryNotApplicableCounter);
	printf("[unit]\t\t bad summary events       : %i\n",
			summaryBadEventCounter);
	printf("[unit]\t\t summary index updates    : %i\n", summaryIndexCounter);
	printf("[unit]\t\t summary index errors     : %i\n",
			summaryIndexErrorCounter);
	printf("[unit]\t\t summary records          : %i\n", summaryRecordCounter);
	printf("[unit]\t\t database updates         : %i\n", databaseUpdateCount);
	printf("[unit]\t\t database update time     : %0.6f\n", databaseUpdateTime);
	printf("[unit]\t\t avg database update time : %0.6f\n",
			(databaseUpdateTime / (double)databaseUpdateCount));

	printf("[unit]\t ...PASSED\n");

	return 0;
}

