/*
 * index_registry_unitTest.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * A library component for the search-engine to define the set of indexes
 * maintained by the search-engine system, unit test.
 *
 * Written by Josh English.
 */

// preprocessor directives

#define _CORE_COMMON_BYPASS_STD_STREAM
#include "core/core.h"
#include "search/search.h"


// define index registry unit test constants

#define BASIC_UNIT_TEST_LENGTH							128

#define BASIC_UNIT_TEST_STRING_SEED						32


// declare index registry unit test private functions

static int indexRegistry_basicUnitTest();


// main

int main()
{
	signal_registerDefault();

	system_pickRandomSeed();

	printf("[unit] Index Registry unit test, using Ver %s on %s.\n",
			ASGARD_VERSION, ASGARD_DATE);

	if(indexRegistry_basicUnitTest() < 0) {
		printf("[unit]\t ...FAILED\n");
		return 1;
	}

	printf("[unit] ...PASSED\n");

	return 0;
}

// define index registry unit test private functions

static char *generateRandomString(int seed)
{
	int ii = 0;
	int pick = 0;
	int length = 0;
	char *result = NULL;

	length = ((rand() % seed) + (rand() % (seed / 4)) + 8);
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
			pick += 97 - 1;
		}
		result[ii] = (char)pick;
	}

	return result;
}

static int indexRegistry_basicUnitTest()
{
	int ii = 0;
	int nn = 0;
	int type = 0;
	int length = 0;
	char *key = NULL;
	char *newKey = NULL;
	char *name = NULL;

	ArrayList list;
	ArrayList updatedList;
	IndexRegistry registry;
	IndexRegistryIndex *index = NULL;

	printf("[unit]\t index registry basic unit test...\n");

	system_pickRandomSeed();

	for(ii = 0; ii < BASIC_UNIT_TEST_LENGTH; ii++) {
		if(indexRegistry_init(&registry) < 0) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		// check registry status

		if(indexRegistry_isLocked(&registry)) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		if(indexRegistry_isReadLocked(&registry)) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		if(indexRegistry_lock(&registry) < 0) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		if(!indexRegistry_isReadLocked(&registry)) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		if(indexRegistry_unlock(&registry) < 0) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		if(indexRegistry_isLocked(&registry)) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		if(indexRegistry_isReadLocked(&registry)) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		// build & test indexes

		length = ((rand() % 128) + 32);

		arraylist_init(&list);
		arraylist_init(&updatedList);

		for(nn = 0; nn < length; nn++) {
			type = ((rand() % 3) + 1);

			key = generateRandomString(BASIC_UNIT_TEST_STRING_SEED);

			name = generateRandomString(BASIC_UNIT_TEST_STRING_SEED);

			if(indexRegistry_put(&registry,
						(IndexRegistryTypes)type,
						key,
						name) < 0) {
				fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
						__FUNCTION__, __LINE__);
				return -1;
			}

			if((index = indexRegistry_get(&registry, key)) == NULL) {
				fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
						__FUNCTION__, __LINE__);
				return -1;
			}

			if(index->uid != nn) {
				fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
						__FUNCTION__, __LINE__);
				return -1;
			}

			if(index->type != (IndexRegistryTypes)type) {
				fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
						__FUNCTION__, __LINE__);
				return -1;
			}

			if(strcmp(index->key, key)) {
				fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
						__FUNCTION__, __LINE__);
				return -1;
			}

			if(strcmp(index->name, name)) {
				fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
						__FUNCTION__, __LINE__);
				return -1;
			}

			arraylist_pushFifo(&list, key);

			free(name);
		}

		// update indexes

		while((key = arraylist_pop(&list)) != NULL) {
			type = ((rand() % 3) + 1);

			newKey = generateRandomString(BASIC_UNIT_TEST_STRING_SEED);

			name = generateRandomString(BASIC_UNIT_TEST_STRING_SEED);

			if(indexRegistry_reset(&registry,
						key,
						(IndexRegistryTypes)type,
						newKey,
						name) < 0) {
				fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
						__FUNCTION__, __LINE__);
				return -1;
			}

			if((index = indexRegistry_get(&registry, newKey)) == NULL) {
				fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
						__FUNCTION__, __LINE__);
				return -1;
			}

			if(index->type != (IndexRegistryTypes)type) {
				fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
						__FUNCTION__, __LINE__);
				return -1;
			}

			if(strcmp(index->key, newKey)) {
				fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
						__FUNCTION__, __LINE__);
				return -1;
			}

			if(strcmp(index->name, name)) {
				fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
						__FUNCTION__, __LINE__);
				return -1;
			}

			arraylist_pushFifo(&updatedList, newKey);

			free(key);
			free(name);
		}

		// delete indexes

		while((key = arraylist_pop(&updatedList)) != NULL) {
			if(indexRegistry_remove(&registry, key) < 0) {
				fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
						__FUNCTION__, __LINE__);
				return -1;
			}

			free(key);
		}

		arraylist_free(&list);
		arraylist_free(&updatedList);

		// cleanup

		if(indexRegistry_free(&registry) < 0) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}
	}

	printf("[unit]\t ...PASSED\n");

	return 0;
}

