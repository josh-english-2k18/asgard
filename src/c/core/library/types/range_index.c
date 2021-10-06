/*
 * range_index.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * A library to provide an API for building and range-searching against a
 * numeric index.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#define _CORE_LIBRARY_TYPES_COMPONENT
#include "core/library/types/range_index.h"


// declare range index private functions

static RangeIndexEntry *entryNew(int value, char *key, int keyLength);

static void entryFree(void *memory);

static int compareIndexKeys(char *keyAlpha, int keyAlphaLength, char *keyBeta,
		int keyBetaLength);

static void buildResultSet(int **index, int *indexLength, int *list,
		int listLength);

static int ascendingIntegerComparison(const void *left, const void *right);

static int *searchGreaterThan(RangeIndex *range, int value, int *length);

static int *searchLessThan(RangeIndex *range, int value, int *length);

static int *searchBetween(RangeIndex *range, int start, int end, int *length);


// define range index private functions

static RangeIndexEntry *entryNew(int value, char *key, int keyLength)
{
	RangeIndexEntry *result = NULL;

	result = (RangeIndexEntry *)malloc(sizeof(RangeIndexEntry));

	result->value = value;
	result->key = strndup(key, keyLength);
	integer_init(&result->list);

	return result;
}

static void entryFree(void *memory)
{
	RangeIndexEntry *entry = NULL;

	if(memory == NULL) {
		fprintf(stderr, "[%s():%i] error - unable to free NULL memory.\n",
				__FUNCTION__, __LINE__);
		return;
	}

	entry = (RangeIndexEntry *)memory;

	if(entry->key == NULL) {
		fprintf(stderr, "[%s():%i] error - unable to free entry 0x%lx with "
				"NULL key value.\n", __FUNCTION__, __LINE__,
				(unsigned long int)entry);
		return;
	}

	free(entry->key);

	integer_free(&entry->list);

	free(entry);
}

static int compareIndexKeys(char *keyAlpha, int keyAlphaLength, char *keyBeta,
		int keyBetaLength)
{
	int alphaInteger = 0;
	int betaInteger = 0;

	if(keyAlpha == NULL) {
		if(keyBeta == NULL) {
			return 0;
		}
		return -1;
	}
	if(keyBeta == NULL) {
		return 1;
	}

	if(serialize_decodeInt(keyAlpha, keyAlphaLength, &alphaInteger) < 0) {
		alphaInteger = 0;
	}

	if(serialize_decodeInt(keyBeta, keyBetaLength, &betaInteger) < 0) {
		betaInteger = 0;
	}

	if(alphaInteger < betaInteger) {
		return -1;
	}
	else if(alphaInteger > betaInteger) {
		return 1;
	}

	return 0;
}

static void buildResultSet(int **index, int *indexLength, int *list,
		int listLength)
{
	int localLength = 0;
	int *local = NULL;
	void *ptr = NULL;

	if(*index == NULL) {
		local = (int *)malloc(sizeof(int) * listLength);
		memcpy(local, list, (sizeof(int) * listLength));
		localLength = listLength;
	}
	else {
		local = *index;
		localLength = *indexLength;

		local = (int *)realloc(local,
				(sizeof(int) * (localLength + listLength)));

		ptr = (void *)((aptrcast *)local + (localLength * sizeof(int)));

		memcpy(ptr, list, (sizeof(int) * listLength));

		localLength += listLength;
	}

	*indexLength = localLength;
	*index = local;
}

static int ascendingIntegerComparison(const void *left, const void *right)
{
	int iLeft = 0;
	int iRight = 0;

	if((left == NULL) || (right == NULL)) {
		return 0;
	}

	iLeft = *((int *)left);
	iRight = *((int *)right);

	return (iLeft - iRight);
}

static int *searchGreaterThan(RangeIndex *range, int value, int *length)
{
	int keyLength = SIZEOF_INT;
	int resultLength = 0;
	int *result = NULL;
	char key[SIZEOF_INT];

	RangeIndexEntry *entry = NULL;

	if(serialize_encodeInt(value, key) < 0) {
		return NULL;
	}

	while(bptree_getNext(&range->index, key, keyLength,
				((void *)&entry)) == 0) {
		memcpy(key, entry->key, keyLength);

		buildResultSet(&result, &resultLength, entry->list.array,
				entry->list.length);
	}

	if((result != NULL) && (resultLength > 1)) {
		qsort((void *)result, resultLength, sizeof(int),
				ascendingIntegerComparison);
	}
	else if((result == NULL) || (resultLength < 1)) {
		if(result != NULL) {
			free(result);
		}

		result = NULL;
		resultLength = 0;
	}

	*length = resultLength;

	return result;
}

static int *searchLessThan(RangeIndex *range, int value, int *length)
{
	int keyLength = SIZEOF_INT;
	int resultLength = 0;
	int *result = NULL;
	char key[SIZEOF_INT];

	RangeIndexEntry *entry = NULL;

	if(serialize_encodeInt(value, key) < 0) {
		return NULL;
	}

	while(bptree_getPrevious(&range->index, key, keyLength,
				((void *)&entry)) == 0) {
		memcpy(key, entry->key, keyLength);

		buildResultSet(&result, &resultLength, entry->list.array,
				entry->list.length);
	}

	if((result != NULL) && (resultLength > 1)) {
		qsort((void *)result, resultLength, sizeof(int),
				ascendingIntegerComparison);
	}
	else if((result == NULL) || (resultLength < 1)) {
		if(result != NULL) {
			free(result);
		}

		result = NULL;
		resultLength = 0;
	}

	*length = resultLength;

	return result;
}

static int *searchBetween(RangeIndex *range, int start, int end, int *length)
{
	int keyLength = SIZEOF_INT;
	int resultLength = 0;
	int *result = NULL;
	char key[SIZEOF_INT];

	RangeIndexEntry *entry = NULL;

	if(serialize_encodeInt(start, key) < 0) {
		return NULL;
	}

	if(bptree_get(&range->index, key, keyLength, ((void *)&entry)) == 0) {
		buildResultSet(&result, &resultLength, entry->list.array,
				entry->list.length);
	}

	while(bptree_getNext(&range->index, key, keyLength,
				((void *)&entry)) == 0) {
		if(entry->value > end) {
			break;
		}

		memcpy(key, entry->key, keyLength);

		buildResultSet(&result, &resultLength, entry->list.array,
				entry->list.length);
	}

	if((result != NULL) && (resultLength > 1)) {
		qsort((void *)result, resultLength, sizeof(int),
				ascendingIntegerComparison);
	}
	else if((result == NULL) || (resultLength < 1)) {
		if(result != NULL) {
			free(result);
		}

		result = NULL;
		resultLength = 0;
	}

	*length = resultLength;

	return result;
}


// define range index public functions

int rangeIndex_init(RangeIndex *range, char *name)
{
	if((range == NULL) || (name == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	memset(range, 0, (int)(sizeof(RangeIndex)));

	range->name = strdup(name);

	bptree_init(&range->index);
	bptree_setFreeFunction(&range->index, entryFree);
	bptree_setCompareKeysFunction(&range->index, compareIndexKeys);

	return 0;
}

int rangeIndex_free(RangeIndex *range)
{
	if(range == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(range->name != NULL) {
		free(range->name);
	}

	bptree_free(&range->index);

	return 0;
}

char *rangeIndex_typeToString(RangeIndexSearchTypes type)
{
	char *result = NULL;

	switch(type) {
		case RANGE_INDEX_SEARCH_GREATER_THAN:
			result = "Greater Than";
			break;

		case RANGE_INDEX_SEARCH_LESS_THAN:
			result = "Less Than";
			break;

		case RANGE_INDEX_SEARCH_BETWEEN:
			result = "Between";
			break;

		case RANGE_INDEX_SEARCH_UNKNOWN:
		default:
			result = "Unknown";
	}

	return result;
}

char *rangeIndex_getName(RangeIndex *range)
{
	if(range == NULL) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	return range->name;
}

int rangeIndex_setName(RangeIndex *range, char *name)
{
	if((range == NULL) || (name == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(range->name != NULL) {
		free(range->name);
	}

	range->name = strdup(name);

	return 0;
}

int *range_get(RangeIndex *range, int value, int *length)
{
	int keyLength = SIZEOF_INT;
	int *result = NULL;
	char key[SIZEOF_INT];

	RangeIndexEntry *entry = NULL;

	if((range == NULL) || (length == NULL)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	if(serialize_encodeInt(value, key) < 0) {
		return NULL;
	}

	if(bptree_get(&range->index, key, keyLength, ((void *)&entry)) < 0) {
		return NULL;
	}

	buildResultSet(&result, length, entry->list.array, entry->list.length);

	return result;
}

int rangeIndex_put(RangeIndex *range, int value, int uid)
{
	int keyLength = SIZEOF_INT;
	char key[SIZEOF_INT];

	RangeIndexEntry *entry = NULL;

	if(range == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(serialize_encodeInt(value, key) < 0) {
		return -1;
	}

	if(bptree_get(&range->index, key, keyLength, ((void *)&entry)) < 0) {
		entry = entryNew(value, key, keyLength);

		if(bptree_put(&range->index, key, keyLength, (void *)entry) < 0) {
			fprintf(stderr, "[%s():%i] error - failed to add index '%i'.\n",
					__FUNCTION__, __LINE__, value);
			return -1;
		}
	}

	if(integer_put(&entry->list, uid) < 0) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s():%i] error - failed to add UID %i to index "
					"'%i'.\n", __FUNCTION__, __LINE__, uid, value);
		}
	}

	return 0;
}

int rangeIndex_remove(RangeIndex *range, int value, int uid)
{
	int keyLength = SIZEOF_INT;
	char key[SIZEOF_INT];

	RangeIndexEntry *entry = NULL;

	if(range == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(serialize_encodeInt(value, key) < 0) {
		return -1;
	}

	if(bptree_get(&range->index, key, keyLength, ((void *)&entry)) < 0) {
		return -1;
	}

	if(entry->list.length <= 1) {
		if(bptree_remove(&range->index, key, keyLength) < 0) {
			fprintf(stderr, "[%s():%i] error - failed to removeindex '%s'.\n",
					__FUNCTION__, __LINE__, key);
			return -1;
		}
	}
	else {
		if(integer_remove(&entry->list, uid) < 0) {
			if(EXPLICIT_ERRORS) {
				fprintf(stderr, "[%s():%i] error - unable to remove UID %i "
						"from index '%i'.\n", __FUNCTION__, __LINE__,
						uid, value);
			}
		}
	}

	return 0;
}

int *rangeIndex_search(RangeIndex *range, int type, int minValue, int maxValue,
		int *length)
{
	int *result = NULL;

	if((range == NULL) ||
			((type != RANGE_INDEX_SEARCH_GREATER_THAN) &&
			 (type != RANGE_INDEX_SEARCH_LESS_THAN) &&
			 (type != RANGE_INDEX_SEARCH_BETWEEN)) ||
			((type == RANGE_INDEX_SEARCH_BETWEEN) && (minValue > maxValue)) ||
			(length == NULL)) {
		fprintf(stderr, "[%s():%i] error - invalid or missing argument(s)",
				__FUNCTION__, __LINE__);

		if(range == NULL) {
			fprintf(stderr, ", range is NULL");
		}

		if((type != RANGE_INDEX_SEARCH_GREATER_THAN) &&
				(type != RANGE_INDEX_SEARCH_LESS_THAN) &&
				(type != RANGE_INDEX_SEARCH_BETWEEN)) {
			fprintf(stderr, ", type is %i", type);
		}

		if((type == RANGE_INDEX_SEARCH_BETWEEN) && (minValue > maxValue)) {
			fprintf(stderr, ", in mode %i, min value %i > max value %i", type,
					minValue, maxValue);
		}

		if(length == NULL) {
			fprintf(stderr, ", length is NULL");
		}
		else {
			*length = 0;
		}

		fprintf(stderr, ".\n");

		return NULL;
	}

	*length = 0;

	switch(type) {
		case RANGE_INDEX_SEARCH_GREATER_THAN:
			result = searchGreaterThan(range, minValue, length);
			break;

		case RANGE_INDEX_SEARCH_LESS_THAN:
			result = searchLessThan(range, minValue, length);
			break;

		case RANGE_INDEX_SEARCH_BETWEEN:
			result = searchBetween(range, minValue, maxValue, length);
			break;

		case RANGE_INDEX_SEARCH_UNKNOWN:
		default:
			result = NULL;
	}

	return result;
}

