/*
 * search_engine_unitTest.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The engine component for the search-engine.
 *
 * Written by Josh English.
 */

// preprocessor directives

#define _CORE_COMMON_BYPASS_STD_STREAM
#include "core/core.h"
#include "search/search.h"


// define search engine unit test constants

#define STRING_COMPARISON_TEST_FILENAME					\
	"assets/data/test/lt.leary.commanding.txt"

#define STRING_COMPARISON_TEST_DISTANCE					8

#define BASIC_UNIT_LOG_LEVEL							LOG_LEVEL_DEBUG
//#define BASIC_UNIT_LOG_LEVEL							LOG_LEVEL_INFO
//#define BASIC_UNIT_LOG_LEVEL							LOG_LEVEL_ERROR

//#define BASIC_UNIT_LOG_OUTPUT							NULL
#define BASIC_UNIT_LOG_OUTPUT							\
	"log/search.engine.unit.test.log"

#define BASIC_UNIT_TEST_FILENAME						\
	"assets/data/test/command.txt.orig"
//	"assets/data/test/command.txt"

#define BASIC_UNIT_TEST_LENGTH							1024

#define BASIC_UNIT_TEST_STRING_SEED						32

#define STRESS_TEST_LENGTH								8//128


// declare search engine unit test private functions

// general functions

static void cleanString(char *string, int stringLength);

static void tokenizeAndIndexString(SearchEngine *engine, Bptree *index,
		char *string, int stringLength, aboolean willIndex,
		char ***resultTokenList, int **resultTokenLengths,
		int *resultTokenLength);

// unit test functions

static int searchEngine_stringComparisonTest();

static int searchEngine_basicUnitTest(aboolean isNonStdInitOrder);

static int searchEngine_stressTest();

static int searchEngine_geocodeTest();

static int searchEngine_delimitedExactIndexUnitTest();


// main

int main()
{
	int rc = 0;

	signal_registerDefault();

	system_pickRandomSeed();

	printf("[unit] Search Engine unit test, using Ver %s on %s.\n",
			ASGARD_VERSION, ASGARD_DATE);

if(0) {
	if(searchEngine_stringComparisonTest() < 0) {
		printf("[unit]\t ...FAILED\n");
		rc = 1;
	}
}

	if(searchEngine_basicUnitTest(afalse) < 0) {
		printf("[unit]\t ...FAILED\n");
		rc = 1;
	}

	if(searchEngine_basicUnitTest(atrue) < 0) {
		printf("[unit]\t ...FAILED\n");
		rc = 1;
	}

	if(searchEngine_stressTest() < 0) {
		printf("[unit]\t ...FAILED\n");
		rc = 1;
	}

	if(searchEngine_geocodeTest() < 0) {
		printf("[unit]\t ...FAILED\n");
		rc = 1;
	}

	if(searchEngine_delimitedExactIndexUnitTest() < 0) {
		printf("[unit]\t ...FAILED\n");
		rc = 1;
	}

	if(rc != 0) {
		printf("[unit] ...FAILED\n");
	}
	else {
		printf("[unit] ...PASSED\n");
	}

	return rc;
}


// define search engine unit test private functions

// general functions

static void cleanString(char *string, int stringLength)
{
	int ii = 0;
	int nn = 0;
	int iValue = 0;

	for(ii = 0, nn = 0; ii < stringLength; ii++) {
		iValue = ctype_ctoi(string[ii]);

		if(iValue == 9) {
			string[nn] = ' ';
			nn++;
		}
		else if((iValue >= 32) && (iValue <= 126)) {
			string[nn] = string[ii];
			nn++;
		}
	}

	string[nn] = '\0';
}

static void tokenizeAndIndexString(SearchEngine *engine, Bptree *index,
		char *string, int stringLength, aboolean willIndex,
		char ***resultTokenList, int **resultTokenLengths,
		int *resultTokenLength)
{
	int ii = 0;
	int nn = 0;
	int tokenCount = 0;
	int tokenStringLength = 0;
	int localStringLength = 0;
	int localStringResultLength = 0;
	int *tokenLengths = NULL;
	char *token = NULL;
	char *localString = NULL;
	char *localStringValue = NULL;
	char **tokenList = NULL;

	if(!willIndex) {
		*resultTokenList = NULL;
		*resultTokenLengths = NULL;
		*resultTokenLength = 0;
	}

	localString = strndup(string, stringLength);
	localStringLength = stringLength;

	if(searchEngineCommon_normalizeString(localString,
				localStringLength,
				&localStringResultLength) < 0) {
		free(localString);
		return;
	}

	localStringLength = localStringResultLength;

	if(localStringLength < SEARCH_ENGINE_DEFAULT_MIN_STRING_LENGTH) {
		free(localString);
		return;
	}

	if(strtotokens(localString,
				localStringLength,
				SEARCH_ENGINE_DEFAULT_DELIMITERS,
				strlen(SEARCH_ENGINE_DEFAULT_DELIMITERS),
				SEARCH_ENGINE_DEFAULT_MIN_STRING_LENGTH,
				&tokenList,
				&tokenLengths,
				&tokenCount) < 0) {
		free(localString);
		return;
	}

	if((tokenList == NULL) || (tokenLengths == NULL) || (tokenCount < 1)) {
		if(tokenLengths != NULL) {
			free(tokenLengths);
		}

		if(tokenList != NULL) {
			for(ii = 0; ii < tokenCount; ii++) {
				if(tokenList[ii] != NULL) {
					free(tokenList[ii]);
				}
			}

			free(tokenList);
		}

		free(localString);

		return;
	}

	free(localString);

	if(!willIndex) {
		*resultTokenList = (char **)malloc(sizeof(char *) * tokenCount);
		*resultTokenLengths = (int *)malloc(sizeof(int) * tokenCount);
		*resultTokenLength = 0;
	}

	for(ii = 0, nn = 0; ii < tokenCount; ii++) {
		if(tokenList[ii] == NULL) {
			continue;
		}

		localString = tokenList[ii];
		localStringLength = tokenLengths[ii];
		localStringResultLength = 0;
		token = NULL;

		if(localStringLength < SEARCH_ENGINE_DEFAULT_MIN_STRING_LENGTH) {
			free(localString);
			continue;
		}

		if(searchEngineCommon_normalizeString(localString, localStringLength,
					&localStringResultLength) < 0) {
			free(localString);
			continue;
		}

		if(strstr(localString, " ") != NULL) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			exit(1);
		}

		localStringLength = localStringResultLength;

		if(localStringLength < SEARCH_ENGINE_DEFAULT_MIN_STRING_LENGTH) {
			free(localString);
			continue;
		}

		if((!cstring_containsVowel(localString, localStringLength)) ||
				(!cstring_containsAlphabetic(localString, localStringLength))) {
			free(localString);
			continue;
		}

		searchEngine_lock(engine);

		if(bptree_get(&engine->excludedWords, localString, localStringLength,
					((void *)&localStringValue)) == 0) {
			free(localString);
			searchEngine_unlock(engine);
			continue;
		}

		if((token = esa_stemToken(&engine->esa, localString,
						localStringLength, &tokenStringLength)) != NULL) {
			if(bptree_get(&engine->excludedWords, token, tokenStringLength,
						((void *)&localStringValue)) == 0) {
				free(localString);
				free(token);
				searchEngine_unlock(engine);
				continue;
			}
		}

		searchEngine_unlock(engine);

		if(willIndex) {
			if(bptree_get(index, localString, localStringLength,
						((void *)&localStringValue)) < 0) {
				bptree_put(index, localString, localStringLength,
						strndup(localString, localStringLength));
			}

			free(tokenList[ii]);

			free(token);
		}
		else {
			free(tokenList[ii]);

			(*resultTokenList)[nn] = token;
			(*resultTokenLengths)[nn] = strlen(token);
			(*resultTokenLength) += 1;

			nn++;
		}
	}

	free(tokenList);
	free(tokenLengths);
}

// unit test functions

static int searchEngine_stringComparisonTest()
{
	int ii = 0;
	int nn = 0;
	int pick = 0;
	int masterPickOne = 0;
	int masterPickTwo = 0;
	int counter = 0;
	int distance = 0;
	int lineLength = 0;
	int tokenLength = 0;
	int masterTokenLength = 0;
	int serialStringLength = 0;
	int *tokenListLengths = NULL;
	int *masterTokenLengths = NULL;
	double timer = 0.0;
	double highestComparison = 0.0;
	double masterHighestComparison = 0.0;
	double totalArrayComparisonTime = 0.0;
	double *listComparisons = NULL;
	char *serialString = NULL;
	char **serialStringArray = NULL;
	char **lines = NULL;
	char **tokenList = NULL;
	char ***masterTokenList = NULL;
	char line[16384];

	Log log;
	FileHandle fh;
	SearchEngine engine;

	printf("[unit]\t search engine string comparison test...\n");

	if(BASIC_UNIT_LOG_OUTPUT != NULL) {
		log_init(&log, LOG_OUTPUT_FILE, BASIC_UNIT_LOG_OUTPUT,
				BASIC_UNIT_LOG_LEVEL);
	}
	else {
		log_init(&log, LOG_OUTPUT_STDOUT, BASIC_UNIT_LOG_OUTPUT,
				BASIC_UNIT_LOG_LEVEL);
	}

	if(searchEngine_init(&engine, &log) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	// check engine status

	if(searchEngine_isLocked(&engine)) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchEngine_isReadLocked(&engine)) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchEngine_lock(&engine) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(!searchEngine_isReadLocked(&engine)) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchEngine_unlock(&engine) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchEngine_isLocked(&engine)) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchEngine_isReadLocked(&engine)) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	// update default settings

	if(searchEngine_setMaxContainerCount(&engine, 128) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchEngine_setMaxContainerMemoryLength(&engine, 16384) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchEngine_setContainerTimeout(&engine, 2.0) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchEngine_setStringIndexThresholds(&engine,
				SEARCH_ENGINE_DEFAULT_MIN_STRING_LENGTH,
				SEARCH_ENGINE_DEFAULT_MAX_STRING_LENGTH) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchEngine_setStringDelimiters(&engine,
				SEARCH_ENGINE_DEFAULT_DELIMITERS,
				strlen(SEARCH_ENGINE_DEFAULT_DELIMITERS)) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchEngine_setStateBackupAttributes(&engine,
				256.0, // backup seconds
				"assets/data/test") < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchEngine_setIndexBackupAttributes(&engine,
				256.0, // backup seconds
				"assets/data/test") < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchEngine_loadExcludedWords(&engine,
				"conf/searchd.excluded.words.default.config") < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	// setup domain

	if(searchEngine_newDomain(&engine,
				"default",
				"Default Domain") < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	// setup indexes

	if(searchEngine_newIndex(&engine,
				INDEX_REGISTRY_TYPE_EXACT,
				"exactIndex",
				"Exact Index") < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchEngine_newIndex(&engine,
				INDEX_REGISTRY_TYPE_WILDCARD,
				"wildcardIndex",
				"Wildcard Index") < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchEngine_newIndex(&engine,
				INDEX_REGISTRY_TYPE_RANGE,
				"rangeIndex",
				"Range Index") < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	// count the number of valid lines in the test-file

	if(file_init(&fh, STRING_COMPARISON_TEST_FILENAME, "r", 8192) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	counter = 0;

	while(file_readLine(&fh, line, ((int)sizeof(line) - 1),
				&lineLength) == 0) {
		if(lineLength < 32) {
			continue;
		}

		cleanString(line, lineLength);

		if((lineLength = strlen(line)) < 256) {
			continue;
		}

		counter++;
	}

	// free file handle

	file_free(&fh);

	// allocate the master list

	masterTokenLength = counter;

	lines = (char **)malloc(sizeof(char *) * masterTokenLength);

	masterTokenLengths = (int *)malloc(sizeof(int) * masterTokenLength);
	masterTokenList = (char ***)malloc(sizeof(char **) * masterTokenLength);

	listComparisons = (double *)malloc(sizeof(double) * masterTokenLength);

	// build string arrays from test-file

	if(file_init(&fh, STRING_COMPARISON_TEST_FILENAME, "r", 8192) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	ii = 0;

	while(file_readLine(&fh, line, ((int)sizeof(line) - 1),
				&lineLength) == 0) {
		if(lineLength < 32) {
			continue;
		}

		cleanString(line, lineLength);

		if((lineLength = strlen(line)) < 256) {
			continue;
		}

		tokenizeAndIndexString(&engine, NULL, line, lineLength, afalse,
				&tokenList, &tokenListLengths, &tokenLength);

		// validate string array serialization

		if((serialString = serializeArray(tokenList, tokenListLengths,
						tokenLength, &serialStringLength)) == NULL) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		serialStringArray = ((char **)serialString);

		for(nn = 0; nn < tokenLength; nn++) {
			if(strcmp(serialStringArray[nn], tokenList[nn])) {
				fprintf(stderr, "[%s():%i] error - failed here, "
						"aborting (%03i => '%s' != '%s').\n",
						__FUNCTION__, __LINE__,
						nn, serialStringArray[nn], tokenList[nn]);
				return -1;
			}
		}

		free(serialString);

		// cleanup tokenization variables

		free(tokenListLengths);

		// setup unit test variables

		lines[ii] = strndup(line, lineLength);

		masterTokenList[ii] = tokenList;
		masterTokenLengths[ii] = tokenLength;
		ii++;
	}

	if(ii != masterTokenLength) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	// free file handle

	file_free(&fh);

	// perform token list comparisons

	counter = 0;
	totalArrayComparisonTime = 0.0;

	for(ii = 0; ii < masterTokenLength; ii++) {
		for(nn = (ii + 1); nn < masterTokenLength; nn++) {
			if(ii == nn) {
				listComparisons[nn] = 0.0;
			}
			else {
				timer = time_getTimeMus();

				if(masterTokenLengths[ii] > masterTokenLengths[nn]) {
					distance = masterTokenLengths[ii];
				}
				else if(masterTokenLengths[ii] < masterTokenLengths[nn]) {
					distance = masterTokenLengths[nn];
				}
				else {
					distance = STRING_COMPARISON_TEST_DISTANCE;
				}

				listComparisons[nn] = compareArrays(
						masterTokenList[ii],
						masterTokenLengths[ii],
						masterTokenList[nn],
						masterTokenLengths[nn],
						distance);

				totalArrayComparisonTime += time_getElapsedMusInSeconds(timer);
				counter++;
			}
		}

		// find the best comparison

		highestComparison = 0.0;
		pick = 0;

		for(nn = (ii + 1); nn < masterTokenLength; nn++) {
			if(listComparisons[nn] > highestComparison) {
				highestComparison = listComparisons[nn];
				pick = nn;
			}
		}

		if(highestComparison > masterHighestComparison) {
			masterHighestComparison = highestComparison;
			masterPickOne = ii;
			masterPickTwo = pick;
		}
	}

	printf("[unit]\t\t total string arrays compared        : %i\n",
			counter);
	printf("[unit]\t\t average comparison time             : %0.6f\n",
			(totalArrayComparisonTime / (double)counter));
	printf("[unit]\t\t total comparison time               : %0.6f\n",
			totalArrayComparisonTime);

	printf("[unit]\t\t highest comparison value found      : %0.6f\n",
			masterHighestComparison);

	printf("[unit]\t\t first line #%03i  => '%s', \n",
			masterPickOne, lines[masterPickOne]);

	for(ii = 0; ii < masterTokenLengths[masterPickOne]; ii++) {
		printf("[unit]\t\t\t %03i => '%s'\n", ii,
				masterTokenList[masterPickOne][ii]);
	}

	printf("[unit]\t\t second line #%03i => '%s', \n", masterPickTwo,
			lines[masterPickTwo]);

	for(ii = 0; ii < masterTokenLengths[masterPickTwo]; ii++) {
		printf("[unit]\t\t\t %03i => '%s'\n", ii,
				masterTokenList[masterPickTwo][ii]);
	}

	// cleanup

	if(searchEngine_free(&engine) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	for(ii = 0; ii < masterTokenLength; ii++) {
		if(masterTokenList[ii] != NULL) {
			for(nn = 0; nn < masterTokenLengths[ii]; nn++) {
				if(masterTokenList[ii][nn] != NULL) {
					free(masterTokenList[ii][nn]);
				}
			}
			free(masterTokenList[ii]);
		}

		if(lines[ii] != NULL) {
			free(lines[ii]);
		}
	}

	free(masterTokenList);
	free(masterTokenLengths);

	free(lines);

	free(listComparisons);

	log_free(&log);

	printf("[unit]\t ...PASSED\n");

	return 0;
}

static int searchEngine_basicUnitTest(aboolean isNonStdInitOrder)
{
	int ii = 0;
	int nn = 0;
	int rc = 0;
	int counter = 0;
	int sortCounter = 0;
	int searchCounter = 0;
	int intersectionCounter = 0;
	int intersectionVerifyCounter = 0;
	int intersectionResultsCounter = 0;
	int containersBuilt = 0;
	int lineLength = 0;
	int containerLength = 0;
	int arrayLength = 0;
	int resultSetLength = 0;
	int stringLength = 0;
	int *resultSet = NULL;
	int *keyLengths = NULL;
	double timer = 0.0;
	double totalContainerBuildTime = 0.0;
	double totalContainerImmediatePutTime = 0.0;
	double totalContainerImmediateUpdateTime = 0.0;
	double totalSearchTime = 0.0;
	double totalIntersectionTime = 0.0;
	double totalSortTime = 0.0;
	double totalDeleteTime = 0.0;
	char *string = NULL;
	char **keys = NULL;
	char **values = NULL;
	char name[128];
	char line[16384];
	char buffer[16384];

	Log log;
	Bptree index;
	FileHandle fh;
	Intersect intersect;
	Container *container = NULL;
	SearchEngine engine;

	printf("[unit]\t search engine basic unit test...\n");

	if(BASIC_UNIT_LOG_OUTPUT != NULL) {
		log_init(&log, LOG_OUTPUT_FILE, BASIC_UNIT_LOG_OUTPUT,
				BASIC_UNIT_LOG_LEVEL);
	}
	else {
		log_init(&log, LOG_OUTPUT_STDOUT, BASIC_UNIT_LOG_OUTPUT,
				BASIC_UNIT_LOG_LEVEL);
	}

	if(searchEngine_init(&engine, &log) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	// check engine status

	if(searchEngine_isLocked(&engine)) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchEngine_isReadLocked(&engine)) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchEngine_lock(&engine) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(!searchEngine_isReadLocked(&engine)) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchEngine_unlock(&engine) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchEngine_isLocked(&engine)) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchEngine_isReadLocked(&engine)) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	// update default settings

	if(searchEngine_setMaxContainerCount(&engine, 65536) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchEngine_setMaxContainerMemoryLength(&engine, 16777216) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchEngine_setContainerTimeout(&engine, 8192.0) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchEngine_setStringIndexThresholds(&engine,
				SEARCH_ENGINE_DEFAULT_MIN_STRING_LENGTH,
				SEARCH_ENGINE_DEFAULT_MAX_STRING_LENGTH) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchEngine_setStringDelimiters(&engine,
				SEARCH_ENGINE_DEFAULT_DELIMITERS,
				strlen(SEARCH_ENGINE_DEFAULT_DELIMITERS)) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchEngine_setStateBackupAttributes(&engine,
				128.0, // backup seconds
				"assets/data/test") < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchEngine_setIndexBackupAttributes(&engine,
				128.0, // backup seconds
				"assets/data/test") < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchEngine_loadExcludedWords(&engine,
				"conf/searchd.excluded.words.default.config") < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(isNonStdInitOrder) {
		// setup indexes

		if(searchEngine_newIndex(&engine,
					INDEX_REGISTRY_TYPE_EXACT,
					"exactIndex",
					"Exact Index") < 0) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		if(searchEngine_newIndex(&engine,
					INDEX_REGISTRY_TYPE_WILDCARD,
					"wildcardIndex",
					"Wildcard Index") < 0) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		if(searchEngine_newIndex(&engine,
					INDEX_REGISTRY_TYPE_RANGE,
					"rangeIndex",
					"Range Index") < 0) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		// setup domain

		if(searchEngine_newDomain(&engine,
					"default",
					"Default Domain") < 0) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}
	}
	else {
		// setup domain

		if(searchEngine_newDomain(&engine,
					"default",
					"Default Domain") < 0) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		// setup indexes

		if(searchEngine_newIndex(&engine,
					INDEX_REGISTRY_TYPE_EXACT,
					"exactIndex",
					"Exact Index") < 0) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		if(searchEngine_newIndex(&engine,
					INDEX_REGISTRY_TYPE_WILDCARD,
					"wildcardIndex",
					"Wildcard Index") < 0) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		if(searchEngine_newIndex(&engine,
					INDEX_REGISTRY_TYPE_RANGE,
					"rangeIndex",
					"Range Index") < 0) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}
	}

	// perform a simple unit-test to verify that exact-match indexes work

	container = container_newWithName("Josh's Test Container",
			strlen("Josh's Test Container"));

	container_putBoolean(container, "isUnitTest", strlen("isUnitTest"), atrue);
	container_putInteger(container, "unitTestCounter",
			strlen("unitTestCounter"), 1234);
	container_putString(container, "exactIndex", strlen("exactIndex"),
			"Pain Management Specialist",
			strlen("Pain Management Specialist"));
	container_putString(container, "wildcardIndex", strlen("wildcardIndex"),
			"Pain Management Specialist",
			strlen("Pain Management Specialist"));
	container_putInteger(container, "rangeIndex", strlen("rangeIndex"), 2134);

	if(searchEngine_put(&engine, "default", container, atrue) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	intersect_init(&intersect);

	if(searchEngine_search(&engine,
				"default",
				"exactIndex",
				"Pain Management Specialist",
				&intersect) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	intersect_execAnd(&intersect, atrue);

	resultSet = intersect_getResult(&intersect, &resultSetLength);

	if((resultSet == NULL) || (resultSetLength != 1)) {
		fprintf(stderr, "[%s():%i] error - failed to execute exact-match "
				"search, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if((resultSet != NULL) && (resultSetLength > 0)) {
		for(nn = 0; nn < resultSetLength; nn++) {
			if((container = searchEngine_get(&engine,
							resultSet[nn],
							&rc)) == NULL) {
				fprintf(stderr, "[%s():%i] error - failed to locate "
						"search-result container #%i with '%s', aborting.\n",
						__FUNCTION__, __LINE__, resultSet[nn],
						searchEngine_errorCodeToString(rc));
				return -1;
			}

			if(container->uid != 1) {
				fprintf(stderr, "[%s():%i] error - failed to locate "
						"search-result container #%i, aborting.\n",
						__FUNCTION__, __LINE__, 1);
				return -1;
			}

			if(searchEngine_unlockGet(&engine, container) < 0) {
				fprintf(stderr, "[%s():%i] error - failed here, "
						"aborting.\n", __FUNCTION__, __LINE__);
				return -1;
			}
		}
	}

	intersect_free(&intersect);

	intersect_init(&intersect);

	if(searchEngine_searchGlobal(&engine,
				"exactIndex",
				"Pain Management Specialist",
				&intersect) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	intersect_execAnd(&intersect, atrue);

	resultSet = intersect_getResult(&intersect, &resultSetLength);

	if((resultSet == NULL) || (resultSetLength != 1)) {
		fprintf(stderr, "[%s():%i] error - failed to execute exact-match "
				"global search, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if((resultSet != NULL) && (resultSetLength > 0)) {
		for(nn = 0; nn < resultSetLength; nn++) {
			if((container = searchEngine_get(&engine,
							resultSet[nn],
							&rc)) == NULL) {
				fprintf(stderr, "[%s():%i] error - failed to locate "
						"search-result container #%i with '%s', aborting.\n",
						__FUNCTION__, __LINE__, resultSet[nn],
						searchEngine_errorCodeToString(rc));
				return -1;
			}

			if(container->uid != 1) {
				fprintf(stderr, "[%s():%i] error - failed to locate "
						"search-result container #%i, aborting.\n",
						__FUNCTION__, __LINE__, 1);
				return -1;
			}

			if(searchEngine_unlockGet(&engine, container) < 0) {
				fprintf(stderr, "[%s():%i] error - failed here, "
						"aborting.\n", __FUNCTION__, __LINE__);
				return -1;
			}
		}
	}

	intersect_free(&intersect);

	// build containers from test-file

	bptree_init(&index);

	if(file_init(&fh, BASIC_UNIT_TEST_FILENAME, "r", 8192) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	counter = 0;

	while(file_readLine(&fh, line, ((int)sizeof(line) - 1),
				&lineLength) == 0) {
		if(lineLength < 32) {
			continue;
		}

		cleanString(line, lineLength);

		if((lineLength = strlen(line)) < 32) {
			continue;
		}

		snprintf(name, ((int)sizeof(name) - 1), "Container%06i", counter);

		timer = time_getTimeMus();

		container = container_newWithName(name, strlen(name));

		container_putBoolean(container, "isUnitTest", strlen("isUnitTest"),
				atrue);
		container_putInteger(container, "unitTestCounter",
				strlen("unitTestCounter"), counter);
		container_putString(container, "exactIndex", strlen("exactIndex"),
				name, strlen(name));
		container_putString(container, "wildcardIndex", strlen("wildcardIndex"),
				line, lineLength);
		container_putInteger(container, "rangeIndex", strlen("rangeIndex"),
				(counter % 128));

		totalContainerBuildTime += time_getElapsedMusInSeconds(timer);

		containerLength += container_calculateMemoryLength(container);

		timer = time_getTimeMus();

		if(searchEngine_put(&engine, "default", container, atrue) < 0) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		totalContainerImmediatePutTime += time_getElapsedMusInSeconds(timer);

		container = container_newWithName(name, strlen(name));

		container_setUid(container, (counter + 1));
		container_putBoolean(container, "isUnitTest", strlen("isUnitTest"),
				atrue);
		container_putInteger(container, "unitTestCounter",
				strlen("unitTestCounter"), counter);
		container_putInteger(container, "someOtherValue",
				strlen("someOtherValue"), rand());
		container_putString(container, "exactIndex", strlen("exactIndex"),
				name, strlen(name));
		container_putString(container, "wildcardIndex",
				strlen("wildcardIndex"), line, lineLength);
		container_putString(container, "wildcardIndexToo",
				strlen("wildcardIndexToo"), line, lineLength);
		container_putInteger(container, "rangeIndex", strlen("rangeIndex"),
				(counter % 128));
		container_putInteger(container, "rangeIndexToo",
				strlen("rangeIndexToo"), (counter % 128));

		if(spinlock_isLocked(&container->lock)) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}
		if(spinlock_getSpinThreadCount(&container->lock, &ii) < 0) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}
		if(ii != 0) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		timer = time_getTimeMus();

		if(searchEngine_update(&engine, "default", container, atrue,
					CONTAINER_FLAG_UPPEND) < 0) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		totalContainerImmediateUpdateTime +=
			time_getElapsedMusInSeconds(timer);

		tokenizeAndIndexString(&engine, &index, line, lineLength, atrue,
				NULL, NULL, NULL);

		counter++;
	}

	printf("[unit]\t\t total containers built              : %i\n", counter);
	printf("[unit]\t\t average container build time        : %0.6f\n",
			(totalContainerBuildTime / (double)counter));
	printf("[unit]\t\t average container memory length     : %i\n",
			(containerLength / counter));
	printf("[unit]\t\t total container memory length       : %i\n",
			containerLength);
	printf("[unit]\t\t average immediate put time          : %0.6f\n",
			(totalContainerImmediatePutTime / (double)counter));
	printf("[unit]\t\t average immediate update time       : %0.6f\n",
			(totalContainerImmediateUpdateTime / (double)counter));

	// free file handle

	file_free(&fh);

	// reexecute build-containers to verify index-cleaning

	if(file_init(&fh, BASIC_UNIT_TEST_FILENAME, "r", 8192) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	counter = 0;
	containerLength = 0;
	totalContainerBuildTime = 0.0;
	totalContainerImmediatePutTime = 0.0;

	while(file_readLine(&fh, line, ((int)sizeof(line) - 1),
				&lineLength) == 0) {
		if(lineLength < 32) {
			continue;
		}

		cleanString(line, lineLength);

		if((lineLength = strlen(line)) < 32) {
			continue;
		}

		snprintf(name, ((int)sizeof(name) - 1), "Container%06i", counter);

		timer = time_getTimeMus();

		container = container_newWithName(name, strlen(name));

		container_setUid(container, (counter + 1));

		container_putBoolean(container, "isUnitTest", strlen("isUnitTest"),
				atrue);
		container_putInteger(container, "unitTestCounter",
				strlen("unitTestCounter"), counter);
		container_putString(container, "exactIndex", strlen("exactIndex"),
				name, strlen(name));
		container_putString(container, "wildcardIndex",
				strlen("wildcardIndex"), line, lineLength);
		container_putInteger(container, "rangeIndex", strlen("rangeIndex"),
				(counter % 128));

		totalContainerBuildTime += time_getElapsedMusInSeconds(timer);

		containerLength += container_calculateMemoryLength(container);

		timer = time_getTimeMus();

		if(searchEngine_put(&engine, "default", container, atrue) < 0) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		totalContainerImmediatePutTime += time_getElapsedMusInSeconds(timer);

		counter++;
	}

	printf("[unit]\t\t total containers built              : %i\n", counter);
	printf("[unit]\t\t average container build time        : %0.6f\n",
			(totalContainerBuildTime / (double)counter));
	printf("[unit]\t\t average container memory length     : %i\n",
			(containerLength / counter));
	printf("[unit]\t\t total container memory length       : %i\n",
			containerLength);
	printf("[unit]\t\t average immediate put time          : %0.6f\n",
			(totalContainerImmediatePutTime / (double)counter));

	containersBuilt = counter;

	// free file handle

	file_free(&fh);

	// execute search tests

	if(bptree_toArray(&index,
				&arrayLength,
				&keyLengths,
				&keys,
				((void *)&values)) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	counter = 0;

	for(ii = 0; ii < arrayLength; ii++) {
		if(counter == 0) {
			intersect_init(&intersect);
			memset(buffer, 0, (int)(sizeof(buffer)));
		}

		strcat(buffer, keys[ii]);
		if(counter < 2) {
			strcat(buffer, " ");
		}

		timer = time_getTimeMus();

		if(counter == 2) {
			if(searchEngine_searchGlobal(&engine,
						"wildcardIndex",
						keys[ii],
						&intersect) < 0) {
				fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
						__FUNCTION__, __LINE__);
				return -1;
			}
		}
		else {
			if(searchEngine_search(&engine,
						"default",
						"wildcardIndex",
						keys[ii],
						&intersect) < 0) {
				fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
						__FUNCTION__, __LINE__);
				return -1;
			}
		}

		totalSearchTime += time_getElapsedMusInSeconds(timer);
		searchCounter++;

		counter++;

		if(counter >= 3) {
			timer = time_getTimeMus();

			if(searchEngine_searchRange(&engine,
						"default",
						"rangeIndex",
						RANGE_INDEX_SEARCH_LESS_THAN,
						128,
						0,
						&intersect) < 0) {
				fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
						__FUNCTION__, __LINE__);
				return -1;
			}

			totalSearchTime += time_getElapsedMusInSeconds(timer);
			searchCounter++;

			counter++;

			timer = time_getTimeMus();

			intersect_execAnd(&intersect, atrue);

			totalIntersectionTime += time_getElapsedMusInSeconds(timer);
			intersectionCounter++;

			timer = time_getTimeMus();

			if(searchSort_sortByRelevancy(&engine, "wildcardIndex", buffer,
						&intersect, atrue) < 0) {
				fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
						__FUNCTION__, __LINE__);
				return -1;
			}

			totalSortTime += time_getElapsedMusInSeconds(timer);
			sortCounter++;

			resultSet = intersect_getResult(&intersect, &resultSetLength);

			if((resultSet != NULL) && (resultSetLength > 0)) {
				counter = 0;

				for(nn = 0; nn < resultSetLength; nn++) {
					if((container = searchEngine_get(&engine,
									resultSet[nn], &rc)) == NULL) {
						fprintf(stderr, "[%s():%i] error - failed to locate "
								"search-result container #%i with '%s', "
								"aborting.\n",
								__FUNCTION__, __LINE__, resultSet[nn],
								searchEngine_errorCodeToString(rc));
						return -1;
					}

					string = container_getString(container, "wildcardIndex",
							strlen("wildcardIndex"), &stringLength);
					if((string == NULL) || (stringLength < 1)) {
						fprintf(stderr, "[%s():%i] error - failed here, "
								"aborting.\n", __FUNCTION__, __LINE__);
						return -1;
					}

					if((strcasestr(string, keys[(ii - 0)])) ||
							(strcasestr(string, keys[(ii - 1)])) ||
							(strcasestr(string, keys[(ii - 2)]))) {
						counter++;
					}

					if(searchEngine_unlockGet(&engine, container) < 0) {
						fprintf(stderr, "[%s():%i] error - failed here, "
								"aborting.\n", __FUNCTION__, __LINE__);
						return -1;
					}

					free(string);
				}

				if(counter == 0) {
					fprintf(stderr, "[%s():%i] error - failed to locate "
							"search-strings '%s', '%s' and '%s' in "
							"result-set of %i items, aborting.\n",
							__FUNCTION__, __LINE__,
							keys[(ii - 0)], keys[(ii - 1)], keys[(ii - 2)],
							resultSetLength);
					return -1;
				}

				intersectionVerifyCounter += counter;
				intersectionResultsCounter += resultSetLength;
			}

			intersect_free(&intersect);

			counter = 0;
		}
	}

	if(counter > 0) {
		intersect_free(&intersect);
	}

	for(ii = 0; ii < arrayLength; ii++) {
		free(keys[ii]);
	}

	free(keyLengths);
	free(keys);
	free(values);

	printf("[unit]\t\t total searches executed             : %i\n",
			searchCounter);
	printf("[unit]\t\t average search time                 : %0.6f\n",
			(totalSearchTime / (double)searchCounter));
	printf("[unit]\t\t total search intersections          : %i\n",
			intersectionCounter);
	printf("[unit]\t\t total search intersections results  : %i\n",
			intersectionResultsCounter);
	printf("[unit]\t\t total search intersections verified : %i\n",
			intersectionVerifyCounter);
	printf("[unit]\t\t average search intersection time    : %0.6f\n",
			(totalIntersectionTime / (double)intersectionCounter));
	printf("[unit]\t\t average relevancy sort time         : %0.6f\n",
			(totalSortTime / (double)sortCounter));

	// test state & data backup & restore

	timer = time_getTimeMus();

	if(searchEngine_backupState(&engine) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	printf("[unit]\t\t backup state time                   : %0.6f seconds\n",
			time_getElapsedMusInSeconds(timer));

	time_sleep(1);

	timer = time_getTimeMus();

	if(searchEngine_restoreState(&engine,
				"assets/data/test/searchd.state.00.config") < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	printf("[unit]\t\t restore state time                  : %0.6f seconds\n",
			time_getElapsedMusInSeconds(timer));

	time_sleep(1);

	timer = time_getTimeMus();

	if(searchEngine_backupData(&engine) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	printf("[unit]\t\t backup data time                    : %0.6f seconds\n",
			time_getElapsedMusInSeconds(timer));

	time_sleep(1);

	timer = time_getTimeMus();

	if(searchEngine_restoreData(&engine,
				"assets/data/test/searchd.00.containers") < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	printf("[unit]\t\t restore data time                   : %0.6f seconds\n",
			time_getElapsedMusInSeconds(timer));

	time_sleep(4);

	// rename indexes

	if(searchEngine_renameIndex(&engine,
				"exactIndex",
				"exactIndexII",
				"Exact Index #2") < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchEngine_renameIndex(&engine,
				"wildcardIndex",
				"wildcardIndexII",
				"Wildcard Index #2") < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchEngine_renameIndex(&engine,
				"rangeIndex",
				"rangeIndexII",
				"Range Index #2") < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	// resetup indexes

	if(searchEngine_resetIndex(&engine,
				"exactIndexII",
				INDEX_REGISTRY_TYPE_RANGE,
				"exactIndex",
				"Exact Index") < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchEngine_resetIndex(&engine,
				"wildcardIndexII",
				INDEX_REGISTRY_TYPE_EXACT,
				"wildcardIndex",
				"Wildcard Index") < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchEngine_resetIndex(&engine,
				"rangeIndexII",
				INDEX_REGISTRY_TYPE_WILDCARD,
				"rangeIndex",
				"Range Index") < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	// remove indexes

	if(searchEngine_removeIndex(&engine, "exactIndex") < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchEngine_removeIndex(&engine, "wildcardIndex") < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchEngine_removeIndex(&engine, "rangeIndex") < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	// test container deletion

	for(ii = 0; ii < containersBuilt; ii++) {
		timer = time_getTimeMus();

		if(searchEngine_delete(&engine, (ii + 1)) < 0) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		totalDeleteTime += time_getElapsedMusInSeconds(timer);
	}

	printf("[unit]\t\t total containers deleted            : %i\n",
			containersBuilt);
	printf("[unit]\t\t average container deletion time     : %0.6f\n",
			(totalDeleteTime / (double)containersBuilt));

	// cleanup

	bptree_free(&index);

	if(searchEngine_free(&engine) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	log_free(&log);

	printf("[unit]\t ...PASSED\n");

	return 0;
}

static int searchEngine_stressTest()
{
	int ii = 0;
	int nn = 0;
	int counter = 0;
	int lineLength = 0;
	int bufferLength = 0;
	int containerLength = 0;
	double timer = 0.0;
	double totalContainerBuildTime = 0.0;
	double totalContainerImmediatePutTime = 0.0;
	char name[128];
	char line[16384];
	char buffer[1024];

	Log log;
	FileHandle fh;
	Container *container = NULL;
	SearchEngine engine;

	printf("[unit]\t search engine stress test...\n");

	if(BASIC_UNIT_LOG_OUTPUT != NULL) {
		log_init(&log, LOG_OUTPUT_FILE, BASIC_UNIT_LOG_OUTPUT,
				BASIC_UNIT_LOG_LEVEL);
	}
	else {
		log_init(&log, LOG_OUTPUT_STDOUT, BASIC_UNIT_LOG_OUTPUT,
				BASIC_UNIT_LOG_LEVEL);
	}

	if(searchEngine_init(&engine, &log) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	// check engine status

	if(searchEngine_isLocked(&engine)) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchEngine_isReadLocked(&engine)) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchEngine_lock(&engine) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(!searchEngine_isReadLocked(&engine)) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchEngine_unlock(&engine) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchEngine_isLocked(&engine)) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchEngine_isReadLocked(&engine)) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	// update default settings

	if(searchEngine_setMaxContainerCount(&engine, 128) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchEngine_setMaxContainerMemoryLength(&engine, 16384) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchEngine_setContainerTimeout(&engine, 2.0) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchEngine_setStringIndexThresholds(&engine,
				SEARCH_ENGINE_DEFAULT_MIN_STRING_LENGTH,
				SEARCH_ENGINE_DEFAULT_MAX_STRING_LENGTH) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchEngine_setStringDelimiters(&engine,
				SEARCH_ENGINE_DEFAULT_DELIMITERS,
				strlen(SEARCH_ENGINE_DEFAULT_DELIMITERS)) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchEngine_setStateBackupAttributes(&engine,
				1.0, // backup seconds
				"assets/data/test") < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchEngine_setIndexBackupAttributes(&engine,
				1.0, // backup seconds
				"assets/data/test") < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchEngine_loadExcludedWords(&engine,
				"conf/searchd.excluded.words.default.config") < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	// setup domain

	if(searchEngine_newDomain(&engine,
				"default",
				"Default Domain") < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	// setup indexes

	if(searchEngine_newIndex(&engine,
				INDEX_REGISTRY_TYPE_EXACT,
				"exactIndex",
				"Exact Index") < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchEngine_newIndex(&engine,
				INDEX_REGISTRY_TYPE_WILDCARD,
				"wildcardIndex",
				"Wildcard Index") < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchEngine_newIndex(&engine,
				INDEX_REGISTRY_TYPE_RANGE,
				"rangeIndex",
				"Range Index") < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	// build containers from test-file

	counter = 0;

	printf("[unit]\t\t building initial container-set...");
	fflush(stdout);

	bufferLength = snprintf(buffer, ((int)sizeof(buffer) - 1),
			"%0.2f %%    ", 0.0);

	printf("%s", buffer);
	fflush(stdout);

	for(ii = 0; ii < STRESS_TEST_LENGTH; ii++) {
		if(ii > 0) {
			for(nn = 0; nn < bufferLength; nn++) {
				printf("\b");
			}

			bufferLength = snprintf(buffer, ((int)sizeof(buffer) - 1),
					"%0.2f %%    ",
					(((double)ii / (double)STRESS_TEST_LENGTH) * 100.0));

			printf("%s", buffer);
			fflush(stdout);
		}

		if(file_init(&fh, BASIC_UNIT_TEST_FILENAME, "r", 8192) < 0) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		while(file_readLine(&fh, line, ((int)sizeof(line) - 1),
					&lineLength) == 0) {
			if(lineLength < 32) {
				continue;
			}

			cleanString(line, lineLength);

			if((lineLength = strlen(line)) < 32) {
				continue;
			}

			snprintf(name, ((int)sizeof(name) - 1), "Container%06i", counter);

			timer = time_getTimeMus();

			container = container_newWithName(name, strlen(name));

			container_putBoolean(container, "isUnitTest", atrue);
			container_putInteger(container, "unitTestCounter", counter);
			container_putString(container, "exactIndex", name, strlen(name));
			container_putString(container, "wildcardIndex", line, lineLength);
			container_putInteger(container, "rangeIndex", (counter % 128));

			totalContainerBuildTime += time_getElapsedMusInSeconds(timer);

			containerLength += container_calculateMemoryLength(container);

			timer = time_getTimeMus();

			if(searchEngine_put(&engine, "default", container, atrue) < 0) {
				fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
						__FUNCTION__, __LINE__);
				return -1;
			}

			totalContainerImmediatePutTime +=
				time_getElapsedMusInSeconds(timer);

			counter++;
		}

		// free file handle

		file_free(&fh);
	}

	for(ii = 0; ii < bufferLength; ii++) {
		printf("\b");
	}

	printf("DONE.    \n");

	printf("[unit]\t\t total containers built              : %i\n", counter);
	printf("[unit]\t\t average container build time        : %0.6f\n",
			(totalContainerBuildTime / (double)counter));
	printf("[unit]\t\t average container memory length     : %i\n",
			(containerLength / counter));
	printf("[unit]\t\t total container memory length       : %i\n",
			containerLength);
	printf("[unit]\t\t average immediate put time          : %0.6f\n",
			(totalContainerImmediatePutTime / (double)counter));

	// update default settings

	if(searchEngine_setMaxContainerCount(&engine, 65536) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchEngine_setMaxContainerMemoryLength(&engine, 16777216) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchEngine_setContainerTimeout(&engine, 8192.0) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchEngine_setStringIndexThresholds(&engine,
				SEARCH_ENGINE_DEFAULT_MIN_STRING_LENGTH,
				SEARCH_ENGINE_DEFAULT_MAX_STRING_LENGTH) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchEngine_setStringDelimiters(&engine,
				SEARCH_ENGINE_DEFAULT_DELIMITERS,
				strlen(SEARCH_ENGINE_DEFAULT_DELIMITERS)) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchEngine_setStateBackupAttributes(&engine,
				8192.0, // backup seconds
				"assets/data/test") < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchEngine_setIndexBackupAttributes(&engine,
				8192.0, // backup seconds
				"assets/data/test") < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchEngine_loadExcludedWords(&engine,
				"conf/searchd.excluded.words.default.config") < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	// reexecute build-containers to verify index-cleaning

	counter = 0;
	containerLength = 0;
	totalContainerBuildTime = 0.0;
	totalContainerImmediatePutTime = 0.0;

	printf("[unit]\t\t building 2nd-pass container-set...");
	fflush(stdout);

	bufferLength = snprintf(buffer, ((int)sizeof(buffer) - 1),
			"%0.2f %%    ", 0.0);

	printf("%s", buffer);
	fflush(stdout);

	for(ii = 0; ii < STRESS_TEST_LENGTH; ii++) {
		if(ii > 0) {
			for(nn = 0; nn < bufferLength; nn++) {
				printf("\b");
			}

			bufferLength = snprintf(buffer, ((int)sizeof(buffer) - 1),
					"%0.2f %%    ",
					(((double)ii / (double)STRESS_TEST_LENGTH) * 100.0));

			printf("%s", buffer);
			fflush(stdout);
		}

		if(file_init(&fh, BASIC_UNIT_TEST_FILENAME, "r", 8192) < 0) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		while(file_readLine(&fh, line, ((int)sizeof(line) - 1),
					&lineLength) == 0) {
			if(lineLength < 32) {
				continue;
			}

			cleanString(line, lineLength);

			if((lineLength = strlen(line)) < 32) {
				continue;
			}

			snprintf(name, ((int)sizeof(name) - 1), "Container%06i", counter);

			timer = time_getTimeMus();

			container = container_newWithName(name);

			container_setUid(container, (counter + 1));

			container_putBoolean(container, "isUnitTest", atrue);
			container_putInteger(container, "unitTestCounter", counter);
			container_putString(container, "exactIndex", name, strlen(name));
			container_putString(container, "wildcardIndex", line, lineLength);
			container_putInteger(container, "rangeIndex", (counter % 128));

			totalContainerBuildTime += time_getElapsedMusInSeconds(timer);

			containerLength += container_calculateMemoryLength(container);

			timer = time_getTimeMus();

			if(searchEngine_put(&engine, "default", container, atrue) < 0) {
				fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
						__FUNCTION__, __LINE__);
				return -1;
			}

			totalContainerImmediatePutTime +=
				time_getElapsedMusInSeconds(timer);

			counter++;
		}

		// free file handle

		file_free(&fh);
	}

	for(ii = 0; ii < bufferLength; ii++) {
		printf("\b");
	}

	printf("DONE.    \n");

	printf("[unit]\t\t total containers built              : %i\n", counter);
	printf("[unit]\t\t average container build time        : %0.6f\n",
			(totalContainerBuildTime / (double)counter));
	printf("[unit]\t\t average container memory length     : %i\n",
			(containerLength / counter));
	printf("[unit]\t\t total container memory length       : %i\n",
			containerLength);
	printf("[unit]\t\t average immediate put time          : %0.6f\n",
			(totalContainerImmediatePutTime / (double)counter));

	// test state & data backup & restore

	if(searchEngine_backupState(&engine) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	time_sleep(2);

	if(searchEngine_restoreState(&engine,
				"assets/data/test/searchd.state.00.config") < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	time_sleep(2);

	if(searchEngine_backupData(&engine) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	time_sleep(2);

	if(searchEngine_restoreData(&engine,
				"assets/data/test/searchd.00.containers") < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	time_sleep(4);

	// cleanup

	if(searchEngine_free(&engine) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	printf("[unit]\t ...PASSED\n");

	return 0;
}

static int searchEngine_geocodeTest()
{
	int ii = 0;
	int rc = 0;
	int counter = 0;
	int containerLength = 0;
	double timer = 0.0;
	double timing = 0.0;
	double miles = 0.0;
	double latitude = 0.0;
	double longitude = 0.0;
	double totalContainerImmediatePutTime = 0.0;

	Log log;
	Intersect intersect;
	Container *container = NULL;
	SearchEngine engine;

	printf("[unit]\t search engine geocode test...\n");

	if(BASIC_UNIT_LOG_OUTPUT != NULL) {
		log_init(&log, LOG_OUTPUT_FILE, BASIC_UNIT_LOG_OUTPUT,
				BASIC_UNIT_LOG_LEVEL);
	}
	else {
		log_init(&log, LOG_OUTPUT_STDOUT, BASIC_UNIT_LOG_OUTPUT,
				BASIC_UNIT_LOG_LEVEL);
	}

	if(searchEngine_init(&engine, &log) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	// setup domain

	if(searchEngine_newDomain(&engine,
				"default",
				"Default Domain") < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	// setup indexes

	if(searchEngine_newIndex(&engine,
				INDEX_REGISTRY_TYPE_WILDCARD,
				"name",
				"Name Index") < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchEngine_newIndex(&engine,
				INDEX_REGISTRY_TYPE_WILDCARD,
				"description",
				"Description") < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchEngine_newIndex(&engine,
				INDEX_REGISTRY_TYPE_WILDCARD,
				"address",
				"Address") < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchEngine_newIndex(&engine,
				INDEX_REGISTRY_TYPE_WILDCARD,
				"category",
				"Category") < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchEngine_newIndex(&engine,
				INDEX_REGISTRY_TYPE_WILDCARD,
				"menu",
				"Menu") < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchEngine_newIndex(&engine,
				INDEX_REGISTRY_TYPE_WILDCARD,
				"at-a-glance",
				"At a Glance") < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchEngine_newIndex(&engine,
				INDEX_REGISTRY_TYPE_WILDCARD,
				"review",
				"Review") < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchEngine_newIndex(&engine,
				INDEX_REGISTRY_TYPE_RANGE,
				"rangeIndex",
				"Range Index") < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchEngine_newIndex(&engine,
				INDEX_REGISTRY_TYPE_RANGE,
				"latitude",
				"Latitude") < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchEngine_newIndex(&engine,
				INDEX_REGISTRY_TYPE_RANGE,
				"longitude",
				"Longitude") < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	// build containers

	container = container_newWithName("Bandits Grill & Bar");

	container_putString(container, "name",
			"Bandits Grill & Bar",
			strlen("Bandits Grill & Bar"));
	container_putString(container, "description",
			"Bar & Grill",
			strlen("Bar & Grill"));
	container_putString(container, "address",
			"589 North Moorpark Road, Thousand Oaks, CA 91360-3707",
			strlen("589 North Moorpark Road, Thousand Oaks, CA 91360-3707"));
	container_putString(container, "category",
			"Bar & Grill, Caterer, Barbecue Restaurant",
			strlen("Bar & Grill, Caterer, Barbecue Restaurant"));
	container_putString(container, "menu",
			"allmenus.com - gayot.com",
			strlen("allmenus.com - gayot.com"));
	container_putString(container, "at-a-glance",
			"baby back ribs � garlic toast � beef ribs � restaurants in los angeles � hot wings",
			strlen("baby back ribs � garlic toast � beef ribs � restaurants in los angeles � hot wings"));
	container_putString(container, "review",
			"I've had business lunches there twice now and found the food and atmosphere good. Prices are a bit high but the amount of food was more than enough to justify that. I'd go back.",
			strlen("I've had business lunches there twice now and found the food and atmosphere good. Prices are a bit high but the amount of food was more than enough to justify that. I'd go back."));
	container_putDouble(container, "latitude", 34.186193);
	container_putDouble(container, "longitude", -118.876516);

	containerLength += container_calculateMemoryLength(container);

	timer = time_getTimeMus();

	if(searchEngine_put(&engine, "default", container, atrue) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	totalContainerImmediatePutTime +=
		time_getElapsedMusInSeconds(timer);

	counter++;

	container = container_newWithName("Fiamme Restaurant and Bar");

	container_putString(container, "name",
			"Fiamme Restaurant and Bar",
			strlen("Fiamme Restaurant and Bar"));
	container_putString(container, "description",
			"Fiamme Retaurant is the home of BlazingGrands(sm) Dueling Pianos delivering the most unique and interactive live entertainment in Ventura county Great for large group events and parties",
			strlen("Fiamme Retaurant is the home of BlazingGrands(sm) Dueling Pianos delivering the most unique and interactive live entertainment in Ventura county Great for large group events and parties"));
	container_putString(container, "address",
			"3731 E. Thousand Oaks Blvd, Thousand Oaks, CA 91362",
			strlen("3731 E. Thousand Oaks Blvd, Thousand Oaks, CA 91362"));
	container_putString(container, "category",
			"Italian Restaurant, Restaurant, Cocktail Bar",
			strlen("Italian Restaurant, Restaurant, Cocktail Bar"));
	container_putString(container, "menu",
			"fiammerestaurant.com",
			strlen("fiammerestaurant.com"));
	container_putString(container, "at-a-glance",
			"the rack of lamb � dueling pianos � home made pasta � onion soup � chicken parm",
			strlen("the rack of lamb � dueling pianos � home made pasta � onion soup � chicken parm"));
	container_putString(container, "review",
			"I made a reservation and went there for a birthday dinner, when we arrived a 7pm on Saturday night and there was no one else in the restaurant and only 2 people in the bar. That should have been our first clue but we sat down, now keep in mind we where the only customers in the dining area. about 10 minutes later a waiter stopped by to drop menus and get drink orders then we didn't see him again for about 15 minutes. Should have left here but some other customers showed up so we thought maybe... The food was mediocre at best and the service was even worse. Think twice before you eat here, I've had better food and service at taco carts in Tijuana.",
			strlen("I made a reservation and went there for a birthday dinner, when we arrived a 7pm on Saturday night and there was no one else in the restaurant and only 2 people in the bar. That should have been our first clue but we sat down, now keep in mind we where the only customers in the dining area. about 10 minutes later a waiter stopped by to drop menus and get drink orders then we didn't see him again for about 15 minutes. Should have left here but some other customers showed up so we thought maybe... The food was mediocre at best and the service was even worse. Think twice before you eat here, I've had better food and service at taco carts in Tijuana."));
	container_putDouble(container, "latitude", 34.16402);
	container_putDouble(container, "longitude", -118.828175);

	containerLength += container_calculateMemoryLength(container);

	timer = time_getTimeMus();

	if(searchEngine_put(&engine, "default", container, atrue) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	totalContainerImmediatePutTime +=
		time_getElapsedMusInSeconds(timer);

	counter++;

	container = container_newWithName("Boccaccio's Restaurant");

	container_putString(container, "name",
			"Boccaccio's Restaurant",
			strlen("Boccaccio's Restaurant"));
	container_putString(container, "description",
			"Restaurant",
			strlen("Restaurant"));
	container_putString(container, "address",
			"32123 Lindero Canyon Rd # 104, Westlake Village, CA 91362",
			strlen("32123 Lindero Canyon Rd # 104, Westlake Village, CA 91362"));
	container_putString(container, "category",
			"Restaurant",
			strlen("Restaurant"));
	container_putString(container, "menu",
			"zagat.com - boccacciosonthelake.com",
			strlen("zagat.com - boccacciosonthelake.com"));
	container_putString(container, "at-a-glance",
			"bridal shower � sea bass � baked alaska � rehearsal dinner � continental cuisine",
			strlen("bridal shower � sea bass � baked alaska � rehearsal dinner � continental cuisine"));
	container_putString(container, "review",
			"Boccaccio's has been my favorite restaurant for years for many reasons but obviously the food is the main reason. The Chilean Sea Bass is the best I've ever had & I've ordered it around the world. For you meat eaters, the Filet Mignon is always cooked perfectly & the Caesar salad from scratch is out of this world. Theres a reason they have 5 star reviews. A healthy addiction!",
			strlen("Boccaccio's has been my favorite restaurant for years for many reasons but obviously the food is the main reason. The Chilean Sea Bass is the best I've ever had & I've ordered it around the world. For you meat eaters, the Filet Mignon is always cooked perfectly & the Caesar salad from scratch is out of this world. Theres a reason they have 5 star reviews. A healthy addiction!"));
	container_putDouble(container, "latitude", 34.138769);
	container_putDouble(container, "longitude", -118.823111);

	containerLength += container_calculateMemoryLength(container);

	timer = time_getTimeMus();

	if(searchEngine_put(&engine, "default", container, atrue) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	totalContainerImmediatePutTime +=
		time_getElapsedMusInSeconds(timer);

	counter++;

	container = container_newWithName("Ladyface Ale Companie");

	container_putString(container, "name",
			"Ladyface Ale Companie",
			strlen("Ladyface Ale Companie"));
	container_putString(container, "description",
			"Ladyface Alehouse & Brasserie combines the only microbrewery between Hollywood and Ventura with a gastropub inspired by the great brasseries from France and Belgium.",
			strlen("Ladyface Alehouse & Brasserie combines the only microbrewery between Hollywood and Ventura with a gastropub inspired by the great brasseries from France and Belgium."));
	container_putString(container, "address",
			"29281 Agoura Road, Agoura Hills, CA 91301",
			strlen("29281 Agoura Road, Agoura Hills, CA 91301"));
	container_putString(container, "category",
			"French Restaurant, Brewery, Pub",
			strlen("French Restaurant, Brewery, Pub"));
	container_putString(container, "menu",
			"ladyfaceale.com",
			strlen("ladyfaceale.com"));
	container_putString(container, "at-a-glance",
			"fish and chips � craft beer � mac and cheese � moules frites � blind ambition",
			strlen("fish and chips � craft beer � mac and cheese � moules frites � blind ambition"));
	container_putString(container, "review",
			"This place was amazing when it first opened. The food isn't as good, but it's still okay. The brews are always amazing! Great atmosphere too!",
			strlen("This place was amazing when it first opened. The food isn't as good, but it's still okay. The brews are always amazing! Great atmosphere too!"));
	container_putDouble(container, "latitude", 34.14431);
	container_putDouble(container, "longitude", -118.763026);

	containerLength += container_calculateMemoryLength(container);

	timer = time_getTimeMus();

	if(searchEngine_put(&engine, "default", container, atrue) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	totalContainerImmediatePutTime +=
		time_getElapsedMusInSeconds(timer);

	counter++;

	printf("[unit]\t\t total containers built              : %i\n", counter);
	printf("[unit]\t\t average container memory length     : %i\n",
			(containerLength / counter));
	printf("[unit]\t\t total container memory length       : %i\n",
			containerLength);
	printf("[unit]\t\t average immediate put time          : %0.6f\n",
			(totalContainerImmediatePutTime / (double)counter));

	// perform search tests

	// search within 1.5 miles of 34.187897,-118.892618

	latitude = 34.187897;
	longitude = -118.892618;
	miles = 1.5;

	printf("[unit]\t\t executing search for %0.2f miles from (%0.6f,%0.6f):\n",
			miles, latitude, longitude);

	timer = time_getTimeMus();

	intersect_init(&intersect);

	if(searchEngine_searchRangeGeocoord(&engine,
				"default",
				SEARCH_ENGINE_GEO_COORD_TYPE_MILES,
				latitude,
				longitude,
				miles,
				&intersect) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	intersect_execAnd(&intersect, atrue);

	timing = time_getElapsedMusInSeconds(timer);

	printf("[unit]\t\t\t search executed in: %0.6f seconds\n", timing);

	if(intersect.result.length != 1) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	for(ii = 0; ii < intersect.result.length; ii++) {
		if((container = searchEngine_get(&engine,
						intersect.result.array[ii],
						&rc)) == NULL) {
			fprintf(stderr, "[%s():%i] error - failed here with '%s', "
					"aborting.\n",
					__FUNCTION__, __LINE__,
					searchEngine_errorCodeToString(rc));
			return -1;
		}

		printf("[unit]\t\t\t search result #%03i->%03i: '%s'\n", ii,
				intersect.result.array[ii], container_getName(container));

		if(searchEngine_unlockGet(&engine, container) < 0) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}
	}

	intersect_free(&intersect);

	// search within 5 miles of 34.187897,-118.892618

	latitude = 34.187897;
	longitude = -118.892618;
	miles = 5.0;

	printf("[unit]\t\t executing search for %0.2f miles from (%0.6f,%0.6f):\n",
			miles, latitude, longitude);

	timer = time_getTimeMus();

	intersect_init(&intersect);

	if(searchEngine_searchRangeGeocoord(&engine,
				"default",
				SEARCH_ENGINE_GEO_COORD_TYPE_MILES,
				latitude,
				longitude,
				miles,
				&intersect) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	intersect_execAnd(&intersect, atrue);

	timing = time_getElapsedMusInSeconds(timer);

	printf("[unit]\t\t\t search executed in: %0.6f seconds\n", timing);

	if(intersect.result.length != 3) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	for(ii = 0; ii < intersect.result.length; ii++) {
		if((container = searchEngine_get(&engine,
						intersect.result.array[ii], &rc)) == NULL) {
			fprintf(stderr, "[%s():%i] error - failed here with '%s', "
					"aborting.\n",
					__FUNCTION__, __LINE__,
					searchEngine_errorCodeToString(rc));
			return -1;
		}

		printf("[unit]\t\t\t search result #%03i->%03i: '%s'\n", ii,
				intersect.result.array[ii], container_getName(container));

		if(searchEngine_unlockGet(&engine, container) < 0) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}
	}

	intersect_free(&intersect);

	// search within 10 miles of 34.187897,-118.892618

	latitude = 34.187897;
	longitude = -118.892618;
	miles = 10.0;

	printf("[unit]\t\t executing search for %0.2f miles from (%0.6f,%0.6f):\n",
			miles, latitude, longitude);

	timer = time_getTimeMus();

	intersect_init(&intersect);

	if(searchEngine_searchRangeGeocoord(&engine,
				"default",
				SEARCH_ENGINE_GEO_COORD_TYPE_MILES,
				latitude,
				longitude,
				miles,
				&intersect) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	intersect_execAnd(&intersect, atrue);

	timing = time_getElapsedMusInSeconds(timer);

	printf("[unit]\t\t\t search executed in: %0.6f seconds\n", timing);

	if(intersect.result.length != 4) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	timer = time_getTimeMus();

	if(searchSort_sortByDistance(&engine, latitude, longitude, &intersect,
				afalse) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	timing = time_getElapsedMusInSeconds(timer);

	printf("[unit]\t\t\t distance sort executed in: %0.6f seconds\n", timing);

	for(ii = 0; ii < intersect.result.length; ii++) {
		if((container = searchEngine_get(&engine,
						intersect.result.array[ii],
						&rc)) == NULL) {
			fprintf(stderr, "[%s():%i] error - failed here with '%s', "
					"aborting.\n",
					__FUNCTION__, __LINE__,
					searchEngine_errorCodeToString(rc));
			return -1;
		}

		printf("[unit]\t\t\t search result #%03i->%03i: '%s'\n", ii,
				intersect.result.array[ii], container_getName(container));

		if(searchEngine_unlockGet(&engine, container) < 0) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}
	}

	intersect_free(&intersect);

	// search within 14000 kilometers of 48.862004,2.350674 (paris)

	latitude = 48.862004;
	longitude = 2.350674;
	miles = 14000.0;

	printf("[unit]\t\t executing search for %0.2f km from (%0.6f,%0.6f):\n",
			miles, latitude, longitude);

	timer = time_getTimeMus();

	intersect_init(&intersect);

	if(searchEngine_searchRangeGeocoord(&engine,
				"default",
				SEARCH_ENGINE_GEO_COORD_TYPE_KILOMETERS,
				latitude,
				longitude,
				miles,
				&intersect) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	intersect_execAnd(&intersect, atrue);

	timing = time_getElapsedMusInSeconds(timer);

	printf("[unit]\t\t\t search executed in: %0.6f seconds\n", timing);

	if(intersect.result.length != 4) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	timer = time_getTimeMus();

	if(searchSort_sortByDistance(&engine, latitude, longitude, &intersect,
				atrue) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	timing = time_getElapsedMusInSeconds(timer);

	printf("[unit]\t\t\t distance sort executed in: %0.6f seconds\n", timing);

	for(ii = 0; ii < intersect.result.length; ii++) {
		if((container = searchEngine_get(&engine,
						intersect.result.array[ii],
						&rc)) == NULL) {
			fprintf(stderr, "[%s():%i] error - failed here with '%s', "
					"aborting.\n",
					__FUNCTION__, __LINE__,
					searchEngine_errorCodeToString(rc));
			return -1;
		}

		printf("[unit]\t\t\t search result #%03i->%03i: '%s'\n", ii,
				intersect.result.array[ii], container_getName(container));

		if(searchEngine_unlockGet(&engine, container) < 0) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}
	}

	intersect_free(&intersect);

	// combinged search within 10 miles of 34.187897,-118.892618

	latitude = 34.187897;
	longitude = -118.892618;
	miles = 10.0;

	printf("[unit]\t\t executing search for %0.2f miles from (%0.6f,%0.6f):\n",
			miles, latitude, longitude);

	intersect_init(&intersect);

	timer = time_getTimeMus();

	if(searchEngine_searchRangeGeocoord(&engine,
				"default",
				SEARCH_ENGINE_GEO_COORD_TYPE_MILES,
				latitude,
				longitude,
				miles,
				&intersect) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	timing = time_getElapsedMusInSeconds(timer);

	printf("[unit]\t\t\t search executed in: %0.6f seconds\n", timing);

	printf("[unit]\t\t executing additional string searches:\n");

	timer = time_getTimeMus();

	if(searchEngine_searchGlobal(&engine,
				"name",
				"bar",
				&intersect) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	timing = time_getElapsedMusInSeconds(timer);

	printf("[unit]\t\t\t search executed in: %0.6f seconds\n", timing);

	intersect_execAnd(&intersect, atrue);

	if(intersect.result.length != 2) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	timer = time_getTimeMus();

	if(searchSort_sortByDistance(&engine, latitude, longitude, &intersect,
				atrue) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	timing = time_getElapsedMusInSeconds(timer);

	printf("[unit]\t\t\t distance sort executed in: %0.6f seconds\n", timing);

	for(ii = 0; ii < intersect.result.length; ii++) {
		if((container = searchEngine_get(&engine,
						intersect.result.array[ii],
						&rc)) == NULL) {
			fprintf(stderr, "[%s():%i] error - failed here with '%s', "
					"aborting.\n",
					__FUNCTION__, __LINE__,
					searchEngine_errorCodeToString(rc));
			return -1;
		}

		printf("[unit]\t\t\t search result #%03i->%03i: '%s'\n", ii,
				intersect.result.array[ii], container_getName(container));

		if(searchEngine_unlockGet(&engine, container) < 0) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}
	}

	intersect_free(&intersect);

	// cleanup

	if(searchEngine_free(&engine) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	log_free(&log);

	printf("[unit]\t ...PASSED\n");

	return 0;
}

static int searchEngine_delimitedExactIndexUnitTest()
{
	int ii = 0;
	int rc = 0;
	int resultSetLength = 0;
	int *resultSet = NULL;

	Log log;
	Intersect intersect;
	Container *container = NULL;
	SearchEngine engine;

	printf("[unit]\t search engine delimited exact-index unit test...\n");

	if(BASIC_UNIT_LOG_OUTPUT != NULL) {
		log_init(&log, LOG_OUTPUT_FILE, BASIC_UNIT_LOG_OUTPUT,
				BASIC_UNIT_LOG_LEVEL);
	}
	else {
		log_init(&log, LOG_OUTPUT_STDOUT, BASIC_UNIT_LOG_OUTPUT,
				BASIC_UNIT_LOG_LEVEL);
	}

	if(searchEngine_init(&engine, &log) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	// update default settings

	if(searchEngine_setStringIndexThresholds(&engine,
				3,
				32) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchEngine_setStringDelimiters(&engine,
				"|",
				strlen("|")) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchEngine_loadExcludedWords(&engine,
				"conf/searchd.excluded.words.default.config") < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	// setup indexes

	if(searchEngine_newIndex(&engine,
				INDEX_REGISTRY_TYPE_EXACT,
				"exactIndex",
				"Exact Index") < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchEngine_newIndex(&engine,
				INDEX_REGISTRY_TYPE_WILDCARD,
				"wildcardIndex",
				"Wildcard Index") < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchEngine_newIndex(&engine,
				INDEX_REGISTRY_TYPE_RANGE,
				"rangeIndex",
				"Range Index") < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	// setup domain

	if(searchEngine_newDomain(&engine,
				"default",
				"Default Domain") < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	// setup containers to verify that delimited exact-match indexes work

	container = container_newWithName("Josh's Test Container");
	container_putBoolean(container, "isUnitTest", atrue);
	container_putInteger(container, "unitTestCounter", 1234);
	container_putString(container, "exactIndex",
			"Pain Management Specialist|Internal Medicine",
			strlen("Pain Management Specialist|Internal Medicine"));
	container_putString(container, "wildcardIndex",
			"this and there for some other test",
			strlen("this and there for some other test"));
	container_putInteger(container, "rangeIndex", 2134);

	if(searchEngine_put(&engine, "default", container, atrue) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	container = container_newWithName("Josh's Test Container II");
	container_putBoolean(container, "isUnitTest", atrue);
	container_putInteger(container, "unitTestCounter", 1235);
	container_putString(container, "exactIndex",
			"Pain Management Specialist Doctor|Internal Medicine Doctor",
			strlen(
				"Pain Management Specialist Doctor|Internal Medicine Doctor"));
	container_putString(container, "wildcardIndex",
			"this and there for some other test",
			strlen("this and there for some other test"));
	container_putInteger(container, "rangeIndex", 2135);

	if(searchEngine_put(&engine, "default", container, atrue) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	// domain-specific search on first token set

	intersect_init(&intersect);

	if(searchEngine_search(&engine,
				"default",
				"exactIndex",
				"Pain Management Specialist",
				&intersect) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	intersect_execAnd(&intersect, atrue);

	resultSet = intersect_getResult(&intersect, &resultSetLength);

	if((resultSet == NULL) || (resultSetLength != 1)) {
		fprintf(stderr, "[%s():%i] error - failed to execute exact-match "
				"search, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if((resultSet != NULL) && (resultSetLength > 0)) {
		for(ii = 0; ii < resultSetLength; ii++) {
			if((container = searchEngine_get(&engine,
							resultSet[ii],
							&rc)) == NULL) {
				fprintf(stderr, "[%s():%i] error - failed to locate "
						"search-result container #%i with '%s', aborting.\n",
						__FUNCTION__, __LINE__, resultSet[ii],
						searchEngine_errorCodeToString(rc));
				return -1;
			}

			if(container->uid != 1) {
				fprintf(stderr, "[%s():%i] error - failed to locate "
						"search-result container #%i, aborting.\n",
						__FUNCTION__, __LINE__, 1);
				return -1;
			}

			if(searchEngine_unlockGet(&engine, container) < 0) {
				fprintf(stderr, "[%s():%i] error - failed here, "
						"aborting.\n", __FUNCTION__, __LINE__);
				return -1;
			}
		}
	}

	intersect_free(&intersect);

	// global search on first token set

	intersect_init(&intersect);

	if(searchEngine_searchGlobal(&engine,
				"exactIndex",
				"Pain Management Specialist",
				&intersect) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	intersect_execAnd(&intersect, atrue);

	resultSet = intersect_getResult(&intersect, &resultSetLength);

	if((resultSet == NULL) || (resultSetLength != 1)) {
		fprintf(stderr, "[%s():%i] error - failed to execute exact-match "
				"global search, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if((resultSet != NULL) && (resultSetLength > 0)) {
		for(ii = 0; ii < resultSetLength; ii++) {
			if((container = searchEngine_get(&engine,
							resultSet[ii],
							&rc)) == NULL) {
				fprintf(stderr, "[%s():%i] error - failed to locate "
						"search-result container #%i with '%s', aborting.\n",
						__FUNCTION__, __LINE__, resultSet[ii],
						searchEngine_errorCodeToString(rc));
				return -1;
			}

			if(container->uid != 1) {
				fprintf(stderr, "[%s():%i] error - failed to locate "
						"search-result container #%i, aborting.\n",
						__FUNCTION__, __LINE__, 1);
				return -1;
			}

			if(searchEngine_unlockGet(&engine, container) < 0) {
				fprintf(stderr, "[%s():%i] error - failed here, "
						"aborting.\n", __FUNCTION__, __LINE__);
				return -1;
			}
		}
	}

	intersect_free(&intersect);

	// domain-specific search on second token set

	intersect_init(&intersect);

	if(searchEngine_search(&engine,
				"default",
				"exactIndex",
				"Internal Medicine",
				&intersect) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	intersect_execAnd(&intersect, atrue);

	resultSet = intersect_getResult(&intersect, &resultSetLength);

	if((resultSet == NULL) || (resultSetLength != 1)) {
		fprintf(stderr, "[%s():%i] error - failed to execute exact-match "
				"search, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if((resultSet != NULL) && (resultSetLength > 0)) {
		for(ii = 0; ii < resultSetLength; ii++) {
			if((container = searchEngine_get(&engine,
							resultSet[ii],
							&rc)) == NULL) {
				fprintf(stderr, "[%s():%i] error - failed to locate "
						"search-result container #%i with '%s', aborting.\n",
						__FUNCTION__, __LINE__, resultSet[ii],
						searchEngine_errorCodeToString(rc));
				return -1;
			}

			if(container->uid != 1) {
				fprintf(stderr, "[%s():%i] error - failed to locate "
						"search-result container #%i, aborting.\n",
						__FUNCTION__, __LINE__, 1);
				return -1;
			}

			if(searchEngine_unlockGet(&engine, container) < 0) {
				fprintf(stderr, "[%s():%i] error - failed here, "
						"aborting.\n", __FUNCTION__, __LINE__);
				return -1;
			}
		}
	}

	intersect_free(&intersect);

	// global search on second token set

	intersect_init(&intersect);

	if(searchEngine_searchGlobal(&engine,
				"exactIndex",
				"Internal Medicine",
				&intersect) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	intersect_execAnd(&intersect, atrue);

	resultSet = intersect_getResult(&intersect, &resultSetLength);

	if((resultSet == NULL) || (resultSetLength != 1)) {
		fprintf(stderr, "[%s():%i] error - failed to execute exact-match "
				"global search, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if((resultSet != NULL) && (resultSetLength > 0)) {
		for(ii = 0; ii < resultSetLength; ii++) {
			if((container = searchEngine_get(&engine,
							resultSet[ii],
							&rc)) == NULL) {
				fprintf(stderr, "[%s():%i] error - failed to locate "
						"search-result container #%i with '%s', aborting.\n",
						__FUNCTION__, __LINE__, resultSet[ii],
						searchEngine_errorCodeToString(rc));
				return -1;
			}

			if(container->uid != 1) {
				fprintf(stderr, "[%s():%i] error - failed to locate "
						"search-result container #%i, aborting.\n",
						__FUNCTION__, __LINE__, 1);
				return -1;
			}

			if(searchEngine_unlockGet(&engine, container) < 0) {
				fprintf(stderr, "[%s():%i] error - failed here, "
						"aborting.\n", __FUNCTION__, __LINE__);
				return -1;
			}
		}
	}

	intersect_free(&intersect);

	// cleanup

	if(searchEngine_free(&engine) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	log_free(&log);

	printf("[unit]\t ...PASSED\n");

	return 0;
}

