/*
 * parser_unitTest.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard-WoW statistical analysis file-parsing library, unit test.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#include "wowstats/common.h"
#include "wowstats/core.h"

// define wowstats parser unit test constants

#define BASIC_TEST_FILENAME						\
	"assets/data/test/wowstats.testfile"

// declare wowstats parser unit tests

static int parser_basicFunctionalityTest();

// main function

int main()
{
	signal_registerDefault();

	printf("[unit] wowstats parser unit test (Asgard Ver %s on %s)\n",
			ASGARD_VERSION, ASGARD_DATE);

	if(parser_basicFunctionalityTest() < 0) {
		printf("[unit]\t ...FAILED\n");
		return 1;
	}

	printf("[unit] ...PASSED\n");

	return 0;
}

// define wowstats parser unit tests

static int parser_basicFunctionalityTest()
{
	aboolean fileExists = afalse;
	int rc = 0;
	int readLength = 0;
	int lineCounter = 0;
	alint fileLength = 0;
	double timer = 0.0;
	double elapsedTime = 0.0;
	char buffer[8192];

	FileHandle fh;
	WowParser parser;

	printf("[unit]\t wowstats parser basic functionality test...\n");
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
/*
 * Note: for use when debugging parsed-lines
 *
		{
			int ii = 0;

			printf("[unit]\t\t line #%08i tokens:\n", (lineCounter + 1));

			for(ii = 0; ii < parser.tokenCount; ii++) {
				printf("[unit]\t\t\t token[%03i]: (%3i)'%s'\n", ii,
						parser.tokenLengths[ii], parser.tokens[ii]);
			}

			if(lineCounter > 3) {
				break;
			}
		}
 */

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

	printf("[unit]\t\t file parsed lines   : %i\n", lineCounter);
	printf("[unit]\t\t file parse time     : %0.6f\n", elapsedTime);
	printf("[unit]\t\t avg line parse time : %0.6f\n",
			(elapsedTime / (double)lineCounter));

	printf("[unit]\t ...PASSED\n");

	return 0;
}

