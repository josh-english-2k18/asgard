/*
 * search_index_unitTest.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * A system component for the search-engine to maintain the set of exact,
 * wildcard, and integer indexes for the search-engine system, unit test.
 *
 * Written by Josh English.
 */

// preprocessor directives

#define _CORE_COMMON_BYPASS_STD_STREAM
#include "core/core.h"
#include "search/search.h"


// define search index unit test constants

#define BASIC_UNIT_TEST_LENGTH							1024

#define BASIC_UNIT_TEST_STRING_SEED						32


// declare search index unit test private functions

static int searchIndex_basicUnitTest();


// main

int main()
{
	signal_registerDefault();

	system_pickRandomSeed();

	printf("[unit] Search Index unit test, using Ver %s on %s.\n",
			ASGARD_VERSION, ASGARD_DATE);

	if(searchIndex_basicUnitTest() < 0) {
		printf("[unit]\t ...FAILED\n");
		return 1;
	}

	printf("[unit] ...PASSED\n");

	return 0;
}

// define search index unit test private functions

static char *generateRandomString(int seed)
{
	int ii = 0;
	int pick = 0;
	int length = 0;
	char *result = NULL;

	length = ((rand() % seed) + (rand() % (seed / 4)) + 64);
	result = (char *)malloc(sizeof(char) * (length + 1));

	for(ii = 0; ii < length; ii++) {
		pick = (int)(rand() % 27);

		if(pick < 0) {
			pick *= -1;
		}

		if(pick == 0) {
			pick = 32;
		}
		else {
			pick += (97 - 1);
		}

		result[ii] = (char)pick;
	}

	return result;
}

static int searchIndex_basicUnitTest()
{
	int ii = 0;
	int nn = 0;
	int pick = 0;
	int number = 0;
	int length = 0;
	int uidListLength = 0;
	int stringCounter = 0;
	int intCounter = 0;
	int userCounter = 0;
	int *uidList = 0;
	double timer = 0.0;
	double totalPutStringTime = 0.0;
	double totalGetStringTime = 0.0;
	double totalPutIntTime = 0.0;
	double totalGetIntTime = 0.0;
	double totalPutUserTime = 0.0;
	double totalGetUserTime = 0.0;
	char *string = NULL;

	SearchIndex index;
	IndexRegistryIndex types[4];

	printf("[unit]\t search index basic unit test...\n");

	if(searchIndex_init(&index, 3, 18, "testKey", "testName") < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	// check registry status

	if(searchIndex_isLocked(&index)) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchIndex_isReadLocked(&index)) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchIndex_lock(&index) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(!searchIndex_isReadLocked(&index)) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchIndex_unlock(&index) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchIndex_isLocked(&index)) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(searchIndex_isReadLocked(&index)) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	// setup types

	memset(types, 0, (int)(sizeof(types)));

	types[0].type = INDEX_REGISTRY_TYPE_EXACT;
	types[0].uid = 0;
	types[0].key = "exactIndex";
	types[0].name = "Exact Index";

	if(searchIndex_newIndex(&index, &(types[0])) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	types[1].type = INDEX_REGISTRY_TYPE_WILDCARD;
	types[1].uid = 1;
	types[1].key = "wildcardIndex";
	types[1].name = "Wildcard Index";

	if(searchIndex_newIndex(&index, &(types[1])) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	types[2].type = INDEX_REGISTRY_TYPE_RANGE;
	types[2].uid = 2;
	types[2].key = "rangeIndex";
	types[2].name = "Range Index";

	if(searchIndex_newIndex(&index, &(types[2])) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	types[3].type = INDEX_REGISTRY_TYPE_USER_KEY;
	types[3].uid = 3;
	types[3].key = "myUid";
	types[3].name = "A user-defined UID index";

	if(searchIndex_newIndex(&index, &(types[3])) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	// build & test indexes

	for(ii = 0; ii < BASIC_UNIT_TEST_LENGTH; ii++) {
		system_pickRandomSeed();

		length = ((rand() % 8) + 4);

		for(nn = 0; nn < length; nn++) {
			pick = (int)(rand() % 4);

			if((pick == 0) || (pick == 1)) {
				string = generateRandomString(BASIC_UNIT_TEST_STRING_SEED);

				timer = time_getTimeMus();

				searchIndex_putString(&index,
						&(types[pick]),
						string,
						ii);

				totalPutStringTime += time_getElapsedMusInSeconds(timer);
				timer = time_getTimeMus();

				if((uidList = searchIndex_getString(&index,
								&(types[pick]),
								string,
								&uidListLength)) == NULL) {
					fprintf(stderr, "[%s():%i] error - failed here, "
							"aborting.\n", __FUNCTION__, __LINE__);
					return -1;
				}

				totalGetStringTime += time_getElapsedMusInSeconds(timer);

				stringCounter++;

				free(string);
			}
			else if(pick == 2) {
				number = (int)(rand() % 8192);

				timer = time_getTimeMus();

				searchIndex_putInteger(&index,
						&(types[pick]),
						number,
						ii);

				totalPutIntTime += time_getElapsedMusInSeconds(timer);
				timer = time_getTimeMus();

				if((uidList = searchIndex_getRange(&index,
								&(types[pick]),
								RANGE_INDEX_SEARCH_BETWEEN,
								(number - 128),
								(number + 128),
								&uidListLength)) == NULL) {
					fprintf(stderr, "[%s():%i] error - failed here, "
							"aborting.\n", __FUNCTION__, __LINE__);
					return -1;
				}

				totalGetIntTime += time_getElapsedMusInSeconds(timer);

				intCounter++;
			}
			else {
				string = generateRandomString(BASIC_UNIT_TEST_STRING_SEED);

				timer = time_getTimeMus();

				searchIndex_putString(&index,
						&(types[pick]),
						string,
						ii);

				totalPutUserTime += time_getElapsedMusInSeconds(timer);
				timer = time_getTimeMus();

				if((uidList = searchIndex_getString(&index,
								&(types[pick]),
								string,
								&uidListLength)) == NULL) {
					fprintf(stderr, "[%s():%i] error - failed here, "
							"aborting.\n", __FUNCTION__, __LINE__);
					return -1;
				}

				if(uidListLength != 1) {
					fprintf(stderr, "[%s():%i] error - failed here, "
							"aborting.\n", __FUNCTION__, __LINE__);
					return -1;
				}

				totalGetUserTime += time_getElapsedMusInSeconds(timer);

				userCounter++;

				free(string);
			}

			free(uidList);
		}
	}

	// rename indexes

	for(ii = 0; ii < 4; ii++) {
		switch(ii) {
			case 0:
				string = "exactIndex";
				types[ii].key = "newExactIndex";
				break;

			case 1:
				string = "wildcardIndex";
				types[ii].key = "newWildcardIndex";
				break;

			case 2:
				string = "rangeIndex";
				types[ii].key = "newRangeIndex";
				break;

			case 3:
				string = "myUid";
				types[ii].key = "newMyUid";
				break;
		}

		if(searchIndex_renameIndex(&index, string, &(types[ii])) < 0) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}
	}

	// remove indexes

	for(ii = 0; ii < 4; ii++) {
		if(searchIndex_removeIndex(&index, &(types[ii])) < 0) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}
	}

	// display unit test results

	printf("[unit]\t\t total string tests             : %i\n", stringCounter);
	printf("[unit]\t\t average put string time        : %0.6f\n",
			(totalPutStringTime / (double)stringCounter));
	printf("[unit]\t\t average get string time        : %0.6f\n",
			(totalGetStringTime / (double)stringCounter));

	printf("[unit]\t\t total int tests                : %i\n", intCounter);
	printf("[unit]\t\t average put int time           : %0.6f\n",
			(totalPutIntTime / (double)intCounter));
	printf("[unit]\t\t average get int time           : %0.6f\n",
			(totalGetIntTime / (double)intCounter));

	printf("[unit]\t\t total user tests               : %i\n", userCounter);
	printf("[unit]\t\t average put user time          : %0.6f\n",
			(totalPutUserTime / (double)userCounter));
	printf("[unit]\t\t average get user time          : %0.6f\n",
			(totalGetUserTime / (double)userCounter));

	// cleanup

	if(searchIndex_free(&index) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	printf("[unit]\t ...PASSED\n");

	return 0;
}

