/*
 * common_unitTest.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * A library component for the search-engine to provide an API of common
 * functions, unit test.
 *
 * Written by Josh English.
 */

// preprocessor directives

#define _CORE_COMMON_BYPASS_STD_STREAM
#include "core/core.h"
#include "search/search.h"


// define search engine common unit test constants

#define BASIC_UNIT_TEST_STRING_SEED						64


// declare search engine common unit test private functions

static int searchEngineCommon_basicUnitTest();


// main

int main()
{
	signal_registerDefault();

	system_pickRandomSeed();

	printf("[unit] Search Engine Common API library unit test, "
			"using Ver %s on %s.\n", ASGARD_VERSION, ASGARD_DATE);

	if(searchEngineCommon_basicUnitTest() < 0) {
		printf("[unit]\t ...FAILED\n");
		return 1;
	}

	printf("[unit] ...PASSED\n");

	return 0;
}

// define search engine common unit test private functions

static char *generateRandomString(aboolean isBinary, int lengthSeed,
		int *resultLength)
{
	int ii = 0;
	int pick = 0;
	int length = 0;
	char *result = NULL;

	length = ((rand() % lengthSeed) + (rand() % (lengthSeed / 4)) + 8);

	result = (char *)malloc(sizeof(char) * (length + 1));

	for(ii = 0; ii < length; ii++) {
		if(isBinary) {
			pick = (int)(rand() % 256); // binary
		}
		else {
			pick = (int)((rand() % 94) + 32); // printable
		}

		result[ii] = (char)pick;
	}

	*resultLength = length;

	return result;
}

static int searchEngineCommon_basicUnitTest()
{
	int ii = 0;
	int keyLength = 0;
	int stringLength = 0;
	int stringResultLength = 0;
	int tokenCount = 0;
	int *tokenLengths = NULL;
	char *string = NULL;
	char **tokenList = NULL;
	char key[128];

	printf("[unit]\t search engine common basic unit test...\n");

	system_pickRandomSeed();

	// index-strings

	string = strdup("tHiS iS soMe tEst STrinG");
	stringLength = strlen(string);

	printf("[unit]\t\t index-string before(%3i) : '%s'\n",
			stringLength, string);

	if(searchEngineCommon_buildIndexString(string, stringLength,
				&stringResultLength) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	printf("[unit]\t\t index-string after(%3i)  : '%s'\n",
			stringResultLength, string);

	if(strcmp(string, "thisissometeststring")) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(stringResultLength != strlen(string)) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	free(string);

	string = generateRandomString(afalse, BASIC_UNIT_TEST_STRING_SEED,
			&stringLength);

	printf("[unit]\t\t index-string before(%3i) : '%s'\n",
			stringLength, string);

	if(searchEngineCommon_buildIndexString(string, stringLength,
				&stringResultLength) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	printf("[unit]\t\t index-string after(%3i)  : '%s'\n",
			stringResultLength, string);

	if(stringResultLength != strlen(string)) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	free(string);

	// normal strings

	string = strdup("tHiS iS soMe tEst STrinG");
	stringLength = strlen(string);

	printf("[unit]\t\t normal-string before(%3i): '%s'\n",
			stringLength, string);

	if(searchEngineCommon_normalizeString(string, stringLength,
				&stringResultLength) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	printf("[unit]\t\t normal-string after(%3i) : '%s'\n",
			stringResultLength, string);

	if(strcmp(string, "this is some test string")) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(stringResultLength != strlen(string)) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	free(string);

	string = generateRandomString(afalse, BASIC_UNIT_TEST_STRING_SEED,
			&stringLength);

	printf("[unit]\t\t normal-string before(%3i): '%s'\n",
			stringLength, string);

	if(searchEngineCommon_normalizeString(string, stringLength,
				&stringResultLength) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	printf("[unit]\t\t normal-string after(%3i) : '%s'\n",
			stringResultLength, string);

	if(stringResultLength != strlen(string)) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	free(string);

	string = generateRandomString(atrue, BASIC_UNIT_TEST_STRING_SEED,
			&stringLength);

	printf("[unit]\t\t normal-string before(%3i): binary- ", stringLength);
	common_display(stdout, string, (stringLength > 64) ? 64 : stringLength);

	if(searchEngineCommon_normalizeString(string, stringLength,
				&stringResultLength) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	printf("[unit]\t\t normal-string after(%3i) : '%s'\n",
			stringResultLength, string);

	if(stringResultLength != strlen(string)) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	free(string);

	// index keys

	if(searchEngineCommon_buildIndexKey("indexName", strlen("indexName"),
				"containerName", strlen("containerName"),
				key, &keyLength) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	printf("[unit]\t\t index-key (%3i)          : '%s'\n",
			keyLength, key);

	if(strcmp(key, "//indexName/containerName")) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(strlen(key) != keyLength) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	// uid keys

	if(searchEngineCommon_buildUidKey(12345, key, &keyLength) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	printf("[unit]\t\t uid-key (%3i)            : '%s'\n",
			keyLength, key);

	if(strcmp(key, "12345")) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(atoi(key) != 12345) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(strlen(key) != keyLength) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	// data types

	if(searchEngineCommon_determineDataType("TrUe", strlen("TrUe")) !=
			SEARCH_ENGINE_DATA_TYPE_BOOLEAN) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchEngineCommon_determineDataType("fAlsE", strlen("fAlsE")) !=
			SEARCH_ENGINE_DATA_TYPE_BOOLEAN) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchEngineCommon_determineDataType("12345", strlen("12345")) !=
			SEARCH_ENGINE_DATA_TYPE_INTEGER) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchEngineCommon_determineDataType("5438257", strlen("5438257")) !=
			SEARCH_ENGINE_DATA_TYPE_INTEGER) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchEngineCommon_determineDataType("1234567890123456",
				strlen("1234567890123456")) !=
			SEARCH_ENGINE_DATA_TYPE_BIG_INTEGER) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchEngineCommon_determineDataType("9876543219651987351687",
				strlen("9876543219651987351687")) !=
			SEARCH_ENGINE_DATA_TYPE_BIG_INTEGER) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchEngineCommon_determineDataType("123.456", strlen("123.456")) !=
			SEARCH_ENGINE_DATA_TYPE_DOUBLE) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchEngineCommon_determineDataType("987654.654654",
				strlen("987654.654654")) !=
			SEARCH_ENGINE_DATA_TYPE_DOUBLE) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	string = generateRandomString(afalse, BASIC_UNIT_TEST_STRING_SEED,
			&stringLength);

	if(searchEngineCommon_determineDataType(string, strlen(string)) !=
			SEARCH_ENGINE_DATA_TYPE_STRING) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	free(string);
	string = generateRandomString(afalse, BASIC_UNIT_TEST_STRING_SEED,
			&stringLength);

	if(searchEngineCommon_determineDataType(string, strlen(string)) !=
			SEARCH_ENGINE_DATA_TYPE_STRING) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	free(string);
	string = generateRandomString(atrue, BASIC_UNIT_TEST_STRING_SEED,
			&stringLength);

	if(searchEngineCommon_determineDataType(string, strlen(string)) !=
			SEARCH_ENGINE_DATA_TYPE_BINARY) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	free(string);
	string = generateRandomString(atrue, BASIC_UNIT_TEST_STRING_SEED,
			&stringLength);

	if(searchEngineCommon_determineDataType(string, strlen(string)) !=
			SEARCH_ENGINE_DATA_TYPE_BINARY) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	free(string);

	// pad-string

	string = (char *)malloc(sizeof(char) * 1024);
	strcpy(string, "asdf");
	stringLength = strlen(string);

	printf("[unit]\t\t pad-string before(%3i)   : '%s'\n",
			stringLength, string);

	if(searchEngineCommong_padString(string,
				stringLength,
				1024,
				8,
				&stringResultLength) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	printf("[unit]\t\t pad-string after(%3i)    : '%s'\n",
			stringResultLength, string);

	if(strcmp(string, "asdf0000")) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(stringResultLength != strlen(string)) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	free(string);

	string = generateRandomString(afalse, 8, &stringLength);
	string = (char *)realloc(string, (sizeof(char) * (stringLength * 2)));

	printf("[unit]\t\t pad-string before(%3i)   : '%s'\n",
			stringLength, string);

	if(searchEngineCommong_padString(string,
				stringLength,
				(stringLength * 2),
				(stringLength + 8),
				&stringResultLength) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	printf("[unit]\t\t pad-string after(%3i)    : '%s'\n",
			stringResultLength, string);

	if(stringResultLength != strlen(string)) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	free(string);

	// tokenization

	string = generateRandomString(atrue, 8192, &stringLength);

	printf("[unit]\t\t token-string before(%3i) : binary- ", stringLength);
	common_display(stdout, string, (stringLength > 64) ? 64 : stringLength);

	if(searchEngineCommon_normalizeString(string, stringLength,
				&stringResultLength) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	printf("[unit]\t\t normal-string after(%3i) : ", stringResultLength);
	common_display(stdout, string, 
			(stringResultLength > 64) ? 64 : stringResultLength);

	if(stringResultLength != strlen(string)) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(strtotokens(string, stringResultLength,
				" |~,;:.][)(}{*@!&-_", strlen(" |~,;:.][)(}{*@!&-_"),
				3, &tokenList, &tokenLengths, &tokenCount) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if((tokenList == NULL) || (tokenLengths == NULL) || (tokenCount < 1)) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	for(ii = 0; ii < tokenCount; ii++) {
		if((ii < 8) || (ii >= (tokenCount - 8))) {
			printf("[unit]\t\t token #%03i of %03i => '%s', length %i\n",
				(ii + 1), tokenCount, tokenList[ii], tokenLengths[ii]);
		}
		else if(ii == 8) {
			printf("[unit]\t\t --------/skip/--------\n");
		}

		if(strlen(tokenList[ii]) != tokenLengths[ii]) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		free(tokenList[ii]);
	}

	free(tokenList);
	free(tokenLengths);

	free(string);

	printf("[unit]\t ...PASSED\n");

	return 0;
}

