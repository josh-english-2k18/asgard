/*
 * timestamp_unitTest.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard-WoW statistical analysis time & date stamp library, unit test.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#include "wowstats/common.h"
#include "wowstats/core.h"

// define wowstats timestamp unit test constants

#define BASIC_TEST_FILENAME						\
	"assets/data/test/wowstats.testfile"

#define BASIC_TEST_YEAR							2009

// declare wowstats timestamp unit tests

static int timestamp_basicFunctionalityTest();

// main function

int main()
{
	signal_registerDefault();

	printf("[unit] wowstats timestamp unit test (Asgard Ver %s on %s)\n",
			ASGARD_VERSION, ASGARD_DATE);

	if(timestamp_basicFunctionalityTest() < 0) {
		printf("[unit]\t ...FAILED\n");
		return 1;
	}

	printf("[unit] ...PASSED\n");

	return 0;
}

// define wowstats timestamp unit tests

static int timestamp_basicFunctionalityTest()
{
	aboolean fileExists = afalse;
	int ii = 0;
	int rc = 0;
	int readLength = 0;
	int lineCounter = 0;
	alint fileLength = 0;
	double timer = 0.0;
	double elapsedTime = 0.0;
	char buffer[8192];

	FileHandle fh;
	WowParser parser;
	WowTimestamp alphaTime;
	WowTimestamp betaTime;
	WowTimestamp timestamp;

	printf("[unit]\t wowstats timestamp basic functionality test...\n");
	printf("[unit]\t\t filename      : %s\n", BASIC_TEST_FILENAME);

	system_fileExists(BASIC_TEST_FILENAME, &fileExists);
	if(!fileExists) {
		printf("[unit]\t\t ...unable to execute when file '%s' does not "
				"exist\n", BASIC_TEST_FILENAME);
		return -1;
	}

	if((rc = file_init(&fh, BASIC_TEST_FILENAME, "r", 65536)) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if((rc = file_getFileLength(&fh, &fileLength)) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t\t file length   : %lli\n", fileLength);

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

		if(wowstats_timestampInit(&timestamp, BASIC_TEST_YEAR) < 0) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		if((lineCounter % 1024) == 0) {
			printf("[unit]\t\t line #%08i tokens:\n", (lineCounter + 1));
		}

		for(ii = 0; ((ii < parser.tokenCount) && (ii < 2)); ii++) {
			if((lineCounter % 1024) == 0) {
				printf("[unit]\t\t\t token[%03i]: (%3i)'%s'\n", ii,
						parser.tokenLengths[ii], parser.tokens[ii]);
			}

			if(ii == 0) {
				if(wowstats_timestampParseDate(&timestamp,
							parser.tokens[ii], parser.tokenLengths[ii]) < 0) {
					printf("[unit]\t\t ...ERROR, failed at line %i, "
							"aborting.\n", __LINE__);
					return -1;
				}
			}
			else if(ii == 1) {
				if(wowstats_timestampParseTime(&timestamp,
							parser.tokens[ii], parser.tokenLengths[ii]) < 0) {
					printf("[unit]\t\t ...ERROR, failed at line %i, "
							"aborting.\n", __LINE__);
					return -1;
				}
			}
		}

		if((lineCounter % 1024) == 0) {
			printf("[unit]\t\t\t timestamp: %02i/%02i/%04i - "
					"%02i:%02i:%02i.%04i\n", timestamp.month, timestamp.day,
					timestamp.year, timestamp.hour, timestamp.minute,
					timestamp.second, timestamp.millis);
		}

		if(lineCounter == 1) {
			if(wowstats_timestampClone(&betaTime, &timestamp) < 0) {
				printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
						__LINE__);
				return -1;
			}
		}

		rc = file_readLine(&fh, buffer, sizeof(buffer), &readLength);
	}

	if(rc < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(wowstats_timestampClone(&alphaTime, &timestamp) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(wowstats_timestampCompare(&alphaTime, &betaTime, &rc) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t\t timestamp comparison is : %i\n", rc);

	if(wowstats_timestampCalculateDifferenceInSeconds(&alphaTime,
				&betaTime, &rc) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t\t timestamp differnece is : %i seconds\n", rc);

	if((rc = file_free(&fh)) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	elapsedTime = time_getElapsedMusInSeconds(timer);

	printf("[unit]\t\t file parsed lines   : %i\n", lineCounter);
	printf("[unit]\t\t file parse time     : %0.6f\n", elapsedTime);
	printf("[unit]\t\t avg line parse time : %0.6f\n",
			(elapsedTime / (double)lineCounter));

	printf("[unit]\t ...PASSED\n");

	return 0;
}

