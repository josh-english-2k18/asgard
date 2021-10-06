/*
 * range_index_unitTest.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * A library to provide an API for building and range-searching against a
 * numeric index, unit test.
 *
 * Written by Josh English.
 */

// preprocessor directives

#define _CORE_COMMON_BYPASS_STD_STREAM
#include "core/core.h"


// declare range index unit test constants

#define BASIC_UNIT_TEST_INDEX_LENGTH				1024
#define BASIC_UNIT_TEST_LENGTH						16384

#define ACCURACY_TEST_LENGTH						6


// define range index unit test private types

typedef struct _VALUE_UID_DEF {
	int value;
	int length;
	int *uids;
} VALUE_UID_DEF;


// declare range index unit test private functions

static int rangeIndex_basicUnitTest();

static int rangeIndex_accuracyTest();


// main function

int main()
{
	signal_registerDefault();

	printf("[unit] Range-Index Unit Test, using Ver %s on %s.\n",
			ASGARD_VERSION, ASGARD_DATE);

	if(rangeIndex_basicUnitTest() < 0) {
		printf("[unit]\t ...FAILED\n");
		return 1;
	}

	if(rangeIndex_accuracyTest() < 0) {
		printf("[unit]\t ...FAILED\n");
		return 1;
	}

	printf("[unit] ...PASSED\n");

	return 0;
}


// define range index unit test private functions

static int lrand()
{
	int ii = 0;
	int length = 0;
	int result = 0;

	length = ((rand() % 128) + 1);

	for(ii = 0; ii < length; ii++) {
		result += rand();
	}

	if(result < 0) {
		result *= -1;
	}

	return result;
}

static int rangeIndex_basicUnitTest()
{
	int ii = 0;
	int nn = 0;
	int pick = 0;
	int bufferLength = 0;
	int searchResultCount = 0;
	int greaterThanSearchCount = 0;
	int greaterThanSearchLength = 0;
	int lessThanSearchCount = 0;
	int lessThanSearchLength = 0;
	int betweenSearchCount = 0;
	int betweenSearchLength = 0;
	int listLength = 0;
	int *list = NULL;
	double timer = 0.0;
	double totalAddTime = 0.0;
	double greaterThanSearchTime = 0.0;
	double lessThanSearchTime = 0.0;
	double betweenSearchTime = 0.0;
	char *name = NULL;
	char buffer[1024];

	RangeIndex range;

	printf("[unit]\t range-index basic unit test...\n");

	system_pickRandomSeed();

	if(rangeIndex_init(&range, "unit test range") < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if((name = rangeIndex_getName(&range)) == NULL) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(strcmp(name, "unit test range")) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(rangeIndex_setName(&range, "this is yet another test") < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if((name = rangeIndex_getName(&range)) == NULL) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(strcmp(name, "this is yet another test")) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	printf("[unit]\t\t executing tests...");
	fflush(stdout);

	bufferLength = snprintf(buffer, ((int)sizeof(buffer) - 1),
			"%i of %i (%0.2f %%)", ii, BASIC_UNIT_TEST_LENGTH,
			(((double)ii / (double)BASIC_UNIT_TEST_LENGTH) * 100.0));

	printf("%s", buffer);
	fflush(stdout);

	for(ii = 0; ii < BASIC_UNIT_TEST_LENGTH; ii++) {
		if((ii > 0) && ((ii % 128) == 0)) {
			for(nn = 0; nn < bufferLength; nn++) {
				printf("\b");
			}

			bufferLength = snprintf(buffer, ((int)sizeof(buffer) - 1),
					"%i of %i (%0.2f %%)", ii, BASIC_UNIT_TEST_LENGTH,
					(((double)ii / (double)BASIC_UNIT_TEST_LENGTH) * 100.0));

			printf("%s", buffer);
			fflush(stdout);

			system_pickRandomSeed();
		}

		pick = (lrand() % BASIC_UNIT_TEST_INDEX_LENGTH);

		timer = time_getTimeMus();

		if(rangeIndex_put(&range, pick, ii) < 0) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		totalAddTime += time_getElapsedMusInSeconds(timer);

		pick = (lrand() % BASIC_UNIT_TEST_INDEX_LENGTH);

		timer = time_getTimeMus();

		if((lrand() % 100) > 65) {
			list = rangeIndex_search(&range, RANGE_INDEX_SEARCH_GREATER_THAN,
					pick, 0, &listLength);
			greaterThanSearchTime += time_getElapsedMusInSeconds(timer);
			greaterThanSearchCount++;
			greaterThanSearchLength += listLength;
		}
		else if((lrand() % 100) > 65) {
			list = rangeIndex_search(&range, RANGE_INDEX_SEARCH_LESS_THAN,
					pick, 0, &listLength);
			lessThanSearchTime += time_getElapsedMusInSeconds(timer);
			lessThanSearchCount++;
			lessThanSearchLength += listLength;
		}
		else {
			list = rangeIndex_search(&range, RANGE_INDEX_SEARCH_BETWEEN,
					pick,
					(pick +
					 ((lrand() % (BASIC_UNIT_TEST_INDEX_LENGTH / 8)) * 2)),
					&listLength);
			betweenSearchTime += time_getElapsedMusInSeconds(timer);
			betweenSearchCount++;
			betweenSearchLength += listLength;
		}

		if(list != NULL) {
			free(list);
		}

		searchResultCount += listLength;
	}

	printf("\n");

	if(rangeIndex_free(&range) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	printf("[unit]\t\t total adds performed      : %i\n",
			BASIC_UNIT_TEST_LENGTH);
	printf("[unit]\t\t total add time            : %0.6f\n",
			totalAddTime);
	printf("[unit]\t\t average add time          : %0.6f\n",
			(totalAddTime / (double)BASIC_UNIT_TEST_LENGTH));
	printf("[unit]\t\t total search results      : %i\n",
			searchResultCount);
	printf("[unit]\t\t average search results    : %i\n",
			(searchResultCount / BASIC_UNIT_TEST_LENGTH));
	printf("[unit]\t\t total (gt) searches       : %i\n",
			greaterThanSearchCount);
	printf("[unit]\t\t total (gt) search length  : %i\n",
			greaterThanSearchLength);
	printf("[unit]\t\t average (gt) search length: %0.0f\n",
			(greaterThanSearchLength / (double)greaterThanSearchCount));
	printf("[unit]\t\t total (gt) search time    : %0.6f\n",
			greaterThanSearchTime);
	printf("[unit]\t\t average (gt) search time  : %0.6f\n",
			(greaterThanSearchTime / (double)greaterThanSearchCount));
	printf("[unit]\t\t total (lt) searches       : %i\n",
			lessThanSearchCount);
	printf("[unit]\t\t total (lt) search length  : %i\n",
			lessThanSearchLength);
	printf("[unit]\t\t average (lt) search length: %0.0f\n",
			(lessThanSearchLength / (double)lessThanSearchCount));
	printf("[unit]\t\t total (lt) search time    : %0.6f\n",
			lessThanSearchTime);
	printf("[unit]\t\t average (lt) search time  : %0.6f\n",
			(lessThanSearchTime / (double)lessThanSearchCount));
	printf("[unit]\t\t total (b) searches        : %i\n",
			betweenSearchCount);
	printf("[unit]\t\t total (b) search length   : %i\n",
			betweenSearchLength);
	printf("[unit]\t\t average (b) search length : %0.0f\n",
			(betweenSearchLength / (double)betweenSearchCount));
	printf("[unit]\t\t total (b) search time     : %0.6f\n",
			betweenSearchTime);
	printf("[unit]\t\t average (b) search time   : %0.6f\n",
			(betweenSearchTime / (double)betweenSearchCount));

	printf("[unit]\t ...PASSED\n");

	return 0;
}

static int rangeIndex_accuracyTest()
{
	int ii = 0;
	int jj = 0;
	int kk = 0;
	int count = 0;
	int uids[ACCURACY_TEST_LENGTH]  = { 105, 110, 115, 215, 120, 125 };
	int values[ACCURACY_TEST_LENGTH] = { 5, 10, 15, 15, 20, 25 };

	int greaterThanTestList[] = { 4, 15, 25, -1};
	int lessThanTestList[] = { 4, 15, 25, -1 };

	int resultLength = 0;
	int *result = NULL;

	RangeIndex range;

	printf("[unit]\t range-index accuracy test...\n");

	rangeIndex_init(&range, "range index accuracy test");
	
	for(ii = 0; ii < ACCURACY_TEST_LENGTH; ii++) {
		if(rangeIndex_put(&range, values[ii], uids[ii]) < 0) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
						__FUNCTION__, __LINE__);
				return -1;
		}
	}

	// greater than tests

	for(kk = 0; greaterThanTestList[kk] != -1; kk++) {
		result = rangeIndex_search(&range, RANGE_INDEX_SEARCH_GREATER_THAN,
				greaterThanTestList[kk], -1, &resultLength);

		if(result == NULL) {
			if(kk < 2) {
				fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
						__FUNCTION__, __LINE__);
				return -1;
			}
			continue;
		}

		count = 0;

		for(ii = 0; ii < ACCURACY_TEST_LENGTH; ii++) {
			if(values[ii] > greaterThanTestList[kk]) {
				count++;
				for(jj = 0; jj < resultLength; jj++) {
					if(result[jj] == uids[ii]) {
						break;
					}
				}

				if(jj == resultLength) {
					fprintf(stderr, "[%s():%i] error - failed here, "
							"aborting.\n", __FUNCTION__, __LINE__);
					return -1;
				}
			}
		}

		if(count != resultLength) {
			fprintf(stderr, "expected %d results; received %d\n", count,
					resultLength);
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		free(result);
	}

	// less than tests

	for(kk = 0; lessThanTestList[kk] != -1; kk++) {
		result = rangeIndex_search(&range, RANGE_INDEX_SEARCH_LESS_THAN,
				lessThanTestList[kk], -1, &resultLength);

		if(result == NULL) {
			if(kk != 0) {
				fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
						__FUNCTION__, __LINE__);
				return -1;
			}
			continue;
		}

		count = 0;

		for(ii = 0; ii < ACCURACY_TEST_LENGTH; ii++) {
			if(values[ii] < lessThanTestList[kk]) {
				count++;
				for(jj = 0; jj < resultLength; jj++) {
					if(result[jj] == uids[ii]) {
						break;
					}
				}

				if(jj == resultLength) {
					fprintf(stderr, "[%s():%i] error - failed here, "
							"aborting.\n", __FUNCTION__, __LINE__);
					return -1;
				}
			}
		}

		if(count != resultLength) {
			fprintf(stderr, "expected %d results; received %d\n", count,
					resultLength);
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		free(result);
	}

	rangeIndex_free(&range);

	printf("[unit]\t ...PASSED\n");

	return 0;
}

