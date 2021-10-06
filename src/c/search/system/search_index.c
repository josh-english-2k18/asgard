/*
 * search_index.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * A system component for the search-engine to maintain the set of exact,
 * wildcard, and integer indexes for the search-engine system.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#include "search/search.h"
#define _SEARCH_SYSTEM_COMPONENT
#include "search/system/search_index.h"


// define search index private functions

static void freeUserKeyEntry(void *memory)
{
	IntegerArray *entry = NULL;

	if(memory == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	entry = (IntegerArray *)memory;

	integer_freePtr(entry);
}

static SearchIndexEntry *indexNew(SearchIndex *index, IndexRegistryIndex *type)
{
	int minStringLength = 0;
	int maxStringLength = 0;

	SearchIndexEntry *result = NULL;

	if(type->settings.isOverrideStringLengths) {
		minStringLength = type->settings.minStringLength;
		maxStringLength = type->settings.maxStringLength;
	}
	else {
		minStringLength = index->minStringLength;
		maxStringLength = index->maxStringLength;
	}

	result = (SearchIndexEntry *)malloc(sizeof(SearchIndexEntry));

	result->type = type->type;

	switch(result->type) {
		case INDEX_REGISTRY_TYPE_EXACT:
		case INDEX_REGISTRY_TYPE_WILDCARD:
			result->index = (void *)malloc(sizeof(WildcardIndex));
			wildcardIndex_init(result->index, minStringLength, maxStringLength);
			break;

		case INDEX_REGISTRY_TYPE_USER_KEY:
			result->index = (void *)malloc(sizeof(Bptree));
			bptree_init(result->index);
			bptree_setFreeFunction(result->index, freeUserKeyEntry);
			break;

		case INDEX_REGISTRY_TYPE_RANGE:
			result->index = (void *)malloc(sizeof(RangeIndex));
			rangeIndex_init(result->index, type->key);
			break;

		case INDEX_REGISTRY_TYPE_END_OF_LIST:
		default:
			DISPLAY_INVALID_ARGS;
			free(result);
			return NULL;
	}

	result->key = strdup(type->key);

	mutex_init(&result->mutex);

	return result;
}

static void indexFree(void *memory)
{
	SearchIndexEntry *entry = NULL;

	if(memory == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	entry = (SearchIndexEntry *)memory;

	mutex_lock(&entry->mutex);

	if(entry->index != NULL) {
		switch(entry->type) {
			case INDEX_REGISTRY_TYPE_EXACT:
			case INDEX_REGISTRY_TYPE_WILDCARD:
				wildcardIndex_free(entry->index);
				break;

			case INDEX_REGISTRY_TYPE_USER_KEY:
				bptree_free(entry->index);
				break;

			case INDEX_REGISTRY_TYPE_RANGE:
				rangeIndex_free(entry->index);
				break;

			case INDEX_REGISTRY_TYPE_END_OF_LIST:
			default:
				DISPLAY_INVALID_ARGS;
				mutex_lock(&entry->mutex);
				return;
		}

		free(entry->index);
	}

	if(entry->key != NULL) {
		free(entry->key);
	}

	mutex_unlock(&entry->mutex);

	mutex_free(&entry->mutex);

	memset(entry, 0, (int)(sizeof(SearchIndexEntry)));

	free(entry);
}

static void indexFreeNothing(void *memory)
{
	if(memory == NULL) {
		DISPLAY_INVALID_ARGS;
	}
}

// define search index public functions

int searchIndex_init(SearchIndex *index, int minStringLength,
		int maxStringLength, char *key, char *name)
{
	if((index == NULL) ||
			(minStringLength < 1) ||
			(maxStringLength < 1) ||
			(minStringLength > maxStringLength) ||
			(maxStringLength >= SEARCH_ENGINE_CORE_MAX_STRING_INDEX_LENGTH) ||
			(key == NULL) ||
			(name == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	memset(index, 0, (int)(sizeof(SearchIndex)));

	index->minStringLength = minStringLength;
	index->maxStringLength = maxStringLength;
	index->key = strdup(key);
	index->name = strdup(name);

	if(bptree_init(&index->index) < 0) {
		return -1;
	}

	if(bptree_setFreeFunction(&index->index, indexFree) < 0) {
		return -1;
	}

	if(spinlock_init(&index->lock) < 0) {
		return -1;
	}

	return 0;
}

SearchIndex *searchIndex_new(int minStringLength, int maxStringLength,
		char *key, char *name)
{
	SearchIndex *result = NULL;

	result = (SearchIndex *)malloc(sizeof(SearchIndex));

	if(searchIndex_init(result, minStringLength, maxStringLength, key,
				name) < 0) {
		searchIndex_freePtr(result);
		return NULL;
	}

	return result;
}

int searchIndex_free(SearchIndex *index)
{
	if(index == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(spinlock_writeLock(&index->lock) < 0) {
		return -1;
	}

	if(index->key != NULL) {
		free(index->key);
	}

	if(index->name != NULL) {
		free(index->name);
	}

	if(bptree_free(&index->index) < 0) {
		spinlock_writeUnlock(&index->lock);
		return -1;
	}

	spinlock_writeUnlock(&index->lock);

	if(spinlock_free(&index->lock) < 0) {
		return -1;
	}

	memset(index, 0, (int)(sizeof(SearchIndex)));

	return 0;
}

int searchIndex_freePtr(SearchIndex *index)
{
	if(index == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(searchIndex_free(index) < 0) {
		return -1;
	}

	free(index);

	return 0;
}

aboolean searchIndex_isLocked(SearchIndex *index)
{
	if(index == NULL) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	return spinlock_isLocked(&index->lock);
}

aboolean searchIndex_isReadLocked(SearchIndex *index)
{
	int threadCount = 0;

	if(index == NULL) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	if(spinlock_getSpinThreadCount(&index->lock, &threadCount) < 0) {
		return afalse;
	}

	if(threadCount > 0) {
		return atrue;
	}

	return afalse;
}

int searchIndex_lock(SearchIndex *index)
{
	if(index == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return spinlock_readLock(&index->lock);
}

int searchIndex_unlock(SearchIndex *index)
{
	if(index == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return spinlock_readUnlock(&index->lock);
}

int searchIndex_rename(SearchIndex *index, char *key, char *name)
{
	if((index == NULL) || (key == NULL) || (name == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(spinlock_writeLock(&index->lock) < 0) {
		return -1;
	}

	if(index->key != NULL) {
		free(index->key);
	}

	if(index->name != NULL) {
		free(index->name);
	}

	index->key = strdup(key);
	index->name = strdup(name);

	spinlock_writeUnlock(&index->lock);

	return 0;
}

int searchIndex_setStringThresholdsindexNew(SearchIndex *index,
		 IndexRegistryIndex *type, int minStringLength, int maxStringLength)
{
	int result = 0;

	WildcardIndex *wildcard = NULL;
	SearchIndexEntry *entry = NULL;

	if((index == NULL) || (type == NULL) || (minStringLength < 0) ||
			(maxStringLength < 0) || (minStringLength > maxStringLength)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(spinlock_readLock(&index->lock) < 0) {
		return -1;
	}

	if(bptree_get(&index->index, type->key, strlen(type->key),
				((void *)&entry)) < 0) {
		spinlock_readUnlock(&index->lock);
		return -1;
	}

	mutex_lock(&entry->mutex);

	switch(type->type) {
		case INDEX_REGISTRY_TYPE_WILDCARD:
			wildcard = (WildcardIndex *)entry->index;
			break;

		case INDEX_REGISTRY_TYPE_EXACT:
		case INDEX_REGISTRY_TYPE_USER_KEY:
		case INDEX_REGISTRY_TYPE_RANGE:
		case INDEX_REGISTRY_TYPE_END_OF_LIST:
		default:
			result = -1;
	}

	if(wildcard != NULL) {
		result = wildcardIndex_setStringThresholds(wildcard, minStringLength,
				maxStringLength);
	}

	mutex_unlock(&entry->mutex);

	spinlock_readUnlock(&index->lock);

	return result;
}

int searchIndex_getIndexCount(SearchIndex *index, IndexRegistryIndex *type)
{
	int result = 0;

	Bptree *tree = NULL;
	SearchIndexEntry *entry = NULL;

	if((index == NULL) || (type == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(spinlock_readLock(&index->lock) < 0) {
		return -1;
	}

	if(bptree_get(&index->index, type->key, strlen(type->key),
				((void *)&entry)) < 0) {
		spinlock_readUnlock(&index->lock);
		return -1;
	}

	mutex_lock(&entry->mutex);

	switch(type->type) {
		case INDEX_REGISTRY_TYPE_EXACT:
		case INDEX_REGISTRY_TYPE_WILDCARD:
			tree = &(((WildcardIndex *)entry->index)->index);
			break;

		case INDEX_REGISTRY_TYPE_USER_KEY:
			tree = ((Bptree *)entry->index);
			break;

		case INDEX_REGISTRY_TYPE_RANGE:
			tree = &(((RangeIndex *)entry->index)->index);
			break;

		case INDEX_REGISTRY_TYPE_END_OF_LIST:
		default:
			tree = NULL;
	}

	if(tree != NULL) {
		bptree_getLeafCount(tree, &result);
	}
	else {
		result = -1;
	}

	mutex_unlock(&entry->mutex);

	spinlock_readUnlock(&index->lock);

	return result;
}

int searchIndex_getIndexDepth(SearchIndex *index, IndexRegistryIndex *type)
{
	int result = 0;

	Bptree *tree = NULL;
	SearchIndexEntry *entry = NULL;

	if((index == NULL) || (type == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(spinlock_readLock(&index->lock) < 0) {
		return -1;
	}

	if(bptree_get(&index->index, type->key, strlen(type->key),
				((void *)&entry)) < 0) {
		spinlock_readUnlock(&index->lock);
		return -1;
	}

	mutex_lock(&entry->mutex);

	switch(type->type) {
		case INDEX_REGISTRY_TYPE_EXACT:
		case INDEX_REGISTRY_TYPE_WILDCARD:
			tree = &(((WildcardIndex *)entry->index)->index);
			break;

		case INDEX_REGISTRY_TYPE_USER_KEY:
			tree = ((Bptree *)entry->index);
			break;

		case INDEX_REGISTRY_TYPE_RANGE:
			tree = &(((RangeIndex *)entry->index)->index);
			break;

		case INDEX_REGISTRY_TYPE_END_OF_LIST:
		default:
			tree = NULL;
	}

	if(tree != NULL) {
		bptree_getTreeDepth(tree, &result);
	}
	else {
		result = -1;
	}

	mutex_unlock(&entry->mutex);

	spinlock_readUnlock(&index->lock);

	return result;
}

int searchIndex_getIndexValueCount(SearchIndex *index,
		IndexRegistryIndex *type)
{
	int ii = 0;
	int result = 0;
	int arrayLength = 0;
	int *keyLengths = NULL;
	char **keys = NULL;
	void **values = NULL;

	Bptree *tree = NULL;
	IntegerArray *array = NULL;
	RangeIndexEntry *rangeEntry = NULL;
	SearchIndexEntry *entry = NULL;

	Intersect intersect;

	if((index == NULL) || (type == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(spinlock_readLock(&index->lock) < 0) {
		return -1;
	}

	if(bptree_get(&index->index, type->key, strlen(type->key),
				((void *)&entry)) < 0) {
		spinlock_readUnlock(&index->lock);
		return -1;
	}

	mutex_lock(&entry->mutex);

	switch(type->type) {
		case INDEX_REGISTRY_TYPE_EXACT:
		case INDEX_REGISTRY_TYPE_WILDCARD:
			tree = &(((WildcardIndex *)entry->index)->index);
			break;

		case INDEX_REGISTRY_TYPE_USER_KEY:
			tree = ((Bptree *)entry->index);
			break;

		case INDEX_REGISTRY_TYPE_RANGE:
			tree = &(((RangeIndex *)entry->index)->index);
			break;

		case INDEX_REGISTRY_TYPE_END_OF_LIST:
		default:
			tree = NULL;
	}

	if(tree == NULL) {
		mutex_unlock(&entry->mutex);
		spinlock_readUnlock(&index->lock);
		return -1;
	}

	if(bptree_toArray(tree, &arrayLength, &keyLengths, &keys, &values) < 0) {
		mutex_unlock(&entry->mutex);
		spinlock_readUnlock(&index->lock);
		return -1;
	}

	intersect_init(&intersect);

	for(ii = 0; ii < arrayLength; ii++) {
		if(keys[ii] != NULL) {
			free(keys[ii]);
		}

		if((type->type == INDEX_REGISTRY_TYPE_EXACT) ||
				(type->type == INDEX_REGISTRY_TYPE_WILDCARD) ||
				(type->type == INDEX_REGISTRY_TYPE_USER_KEY)) {
			array = (IntegerArray *)values[ii];
		}
		else {
			rangeEntry = (RangeIndexEntry *)values[ii];
			array = &(rangeEntry->list);
		}

		intersect_putArray(&intersect, atrue, array->array, array->length);
	}

	intersect_execOr(&intersect);

	result = intersect.result.length;

	mutex_unlock(&entry->mutex);

	spinlock_readUnlock(&index->lock);

	// cleanup

	free(keys);
	free(keyLengths);
	free(values);

	intersect_free(&intersect);

	return result;
}

int searchIndex_buildIndexValueSummary(SearchIndex *index,
		IndexRegistryIndex *type, SearchIndexSummary *summary)
{
	int ii = 0;
	int arrayLength = 0;
	int *keyLengths = NULL;
	char **keys = NULL;
	void **values = NULL;

	Bptree *tree = NULL;
	IntegerArray *array = NULL;
	RangeIndexEntry *rangeEntry = NULL;
	SearchIndexEntry *entry = NULL;

	if((index == NULL) || (type == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(spinlock_readLock(&index->lock) < 0) {
		return -1;
	}

	if(bptree_get(&index->index, type->key, strlen(type->key),
				((void *)&entry)) < 0) {
		spinlock_readUnlock(&index->lock);
		return -1;
	}

	mutex_lock(&entry->mutex);

	switch(type->type) {
		case INDEX_REGISTRY_TYPE_EXACT:
		case INDEX_REGISTRY_TYPE_WILDCARD:
			tree = &(((WildcardIndex *)entry->index)->index);
			break;

		case INDEX_REGISTRY_TYPE_USER_KEY:
			tree = ((Bptree *)entry->index);
			break;

		case INDEX_REGISTRY_TYPE_RANGE:
			tree = &(((RangeIndex *)entry->index)->index);
			break;

		case INDEX_REGISTRY_TYPE_END_OF_LIST:
		default:
			tree = NULL;
	}

	if(tree == NULL) {
		mutex_unlock(&entry->mutex);
		spinlock_readUnlock(&index->lock);
		return -1;
	}

	if(bptree_toArray(tree, &arrayLength, &keyLengths, &keys, &values) < 0) {
		mutex_unlock(&entry->mutex);
		spinlock_readUnlock(&index->lock);
		return -1;
	}

	memset(summary, 0, (int)(sizeof(SearchIndexSummary)));

	summary->length = arrayLength;
	summary->valueCounts = (int *)malloc(sizeof(int) * summary->length);
	summary->type = strdup(indexRegistry_typeToString(type->type));
	summary->keys = keys;

	for(ii = 0; ii < arrayLength; ii++) {
		if((type->type == INDEX_REGISTRY_TYPE_EXACT) ||
				(type->type == INDEX_REGISTRY_TYPE_WILDCARD) ||
				(type->type == INDEX_REGISTRY_TYPE_USER_KEY)) {
			array = (IntegerArray *)values[ii];
		}
		else {
			rangeEntry = (RangeIndexEntry *)values[ii];
			array = &(rangeEntry->list);
		}

		summary->valueCounts[ii] = array->length;
	}

	mutex_unlock(&entry->mutex);

	spinlock_readUnlock(&index->lock);

	// cleanup

	free(keyLengths);
	free(values);

	return 0;
}

int searchIndex_buildAllIndexValueUids(SearchIndex *index,
		IndexRegistryIndex *type, Intersect *intersect)
{
	int ii = 0;
	int arrayLength = 0;
	int *keyLengths = NULL;
	char **keys = NULL;
	void **values = NULL;

	Bptree *tree = NULL;
	IntegerArray *array = NULL;
	RangeIndexEntry *rangeEntry = NULL;
	SearchIndexEntry *entry = NULL;

	Intersect localIntersect;

	if((index == NULL) || (type == NULL) || (intersect == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(spinlock_readLock(&index->lock) < 0) {
		return -1;
	}

	if(bptree_get(&index->index, type->key, strlen(type->key),
				((void *)&entry)) < 0) {
		spinlock_readUnlock(&index->lock);
		return -1;
	}

	mutex_lock(&entry->mutex);

	switch(type->type) {
		case INDEX_REGISTRY_TYPE_EXACT:
		case INDEX_REGISTRY_TYPE_WILDCARD:
			tree = &(((WildcardIndex *)entry->index)->index);
			break;

		case INDEX_REGISTRY_TYPE_USER_KEY:
			tree = ((Bptree *)entry->index);
			break;

		case INDEX_REGISTRY_TYPE_RANGE:
			tree = &(((RangeIndex *)entry->index)->index);
			break;

		case INDEX_REGISTRY_TYPE_END_OF_LIST:
		default:
			tree = NULL;
	}

	if(tree == NULL) {
		mutex_unlock(&entry->mutex);
		spinlock_readUnlock(&index->lock);
		return -1;
	}

	if(bptree_toArray(tree, &arrayLength, &keyLengths, &keys, &values) < 0) {
		mutex_unlock(&entry->mutex);
		spinlock_readUnlock(&index->lock);
		return -1;
	}

	intersect_init(&localIntersect);

	for(ii = 0; ii < arrayLength; ii++) {
		if(keys[ii] != NULL) {
			free(keys[ii]);
		}

		if(values[ii] == NULL) {
			continue;
		}

		if((type->type == INDEX_REGISTRY_TYPE_EXACT) ||
				(type->type == INDEX_REGISTRY_TYPE_WILDCARD) ||
				(type->type == INDEX_REGISTRY_TYPE_USER_KEY)) {
			array = (IntegerArray *)values[ii];
		}
		else {
			rangeEntry = (RangeIndexEntry *)values[ii];
			array = &(rangeEntry->list);
		}

		intersect_putArray(&localIntersect, atrue, array->array,
				array->length);
	}

	mutex_unlock(&entry->mutex);

	spinlock_readUnlock(&index->lock);

	// intersect all uid values and assign them to the master intersection

	intersect_execOr(&localIntersect);

	intersect_putArray(intersect, atrue, localIntersect.result.array,
			localIntersect.result.length);

	intersect_free(&localIntersect);

	// cleanup

	free(keys);
	free(keyLengths);
	free(values);

	return 0;
}

int searchIndex_newIndex(SearchIndex *index, IndexRegistryIndex *type)
{
	int keyLength = 0;

	SearchIndexEntry *entry = NULL;

	if((index == NULL) || (type == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(spinlock_writeLock(&index->lock) < 0) {
		return -1;
	}

	keyLength = strlen(type->key);

	if(bptree_get(&index->index, type->key, keyLength, ((void *)&entry)) < 0) {
		if((entry = indexNew(index, type)) == NULL) {
			spinlock_writeUnlock(&index->lock);
			return -1;
		}
		else if(bptree_put(&index->index, type->key, keyLength,
					(void *)entry) < 0) {
			spinlock_writeUnlock(&index->lock);
			return -1;
		}
	}
	else {
		spinlock_writeUnlock(&index->lock);
		return -1;
	}

	spinlock_writeUnlock(&index->lock);

	return 0;
}

int searchIndex_removeIndex(SearchIndex *index, IndexRegistryIndex *type)
{
	int result = 0;
	int keyLength = 0;

	SearchIndexEntry *entry = NULL;

	if((index == NULL) || (type == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(spinlock_writeLock(&index->lock) < 0) {
		return -1;
	}

	keyLength = strlen(type->key);

	if(bptree_get(&index->index, type->key, keyLength, ((void *)&entry)) < 0) {
		spinlock_writeUnlock(&index->lock);
		return -1;
	}

	result = bptree_remove(&index->index, type->key, keyLength);

	spinlock_writeUnlock(&index->lock);

	return result;
}

int searchIndex_renameIndex(SearchIndex *index, char *key,
		IndexRegistryIndex *type)
{
	int result = 0;
	int keyLength = 0;

	SearchIndexEntry *entry = NULL;

	if((index == NULL) || (key == NULL) || (type == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	keyLength = strlen(key);

	if(spinlock_writeLock(&index->lock) < 0) {
		return -1;
	}

	if(bptree_get(&index->index, key, keyLength, ((void *)&entry)) < 0) {
		spinlock_writeUnlock(&index->lock);
		return -1;
	}

	if(type->type != entry->type) {
		spinlock_writeUnlock(&index->lock);
		return -1;
	}

	if(bptree_setFreeFunction(&index->index, indexFreeNothing) < 0) {
		spinlock_writeUnlock(&index->lock);
		return -1;
	}

	if(bptree_remove(&index->index, key, keyLength) < 0) {
		spinlock_writeUnlock(&index->lock);
		return -1;
	}

	if(bptree_setFreeFunction(&index->index, indexFree) < 0) {
		spinlock_writeUnlock(&index->lock);
		return -1;
	}

	keyLength = strlen(type->key);

	mutex_lock(&entry->mutex);

	if(entry->key != NULL) {
		free(entry->key);
	}

	entry->key = strndup(type->key, keyLength);

	mutex_unlock(&entry->mutex);

	if(bptree_put(&index->index, type->key, keyLength, (void *)entry) < 0) {
		spinlock_writeUnlock(&index->lock);
		return -1;
	}

	spinlock_writeUnlock(&index->lock);

	return result;
}

int *searchIndex_getString(SearchIndex *index, IndexRegistryIndex *type,
		char *value, int *length)
{
	int resultLength = 0;
	int *result = NULL;

	IntegerArray *array = NULL;
	SearchIndexEntry *entry = NULL;

	if((index == NULL) || (type == NULL) ||
			((type->type != INDEX_REGISTRY_TYPE_EXACT) &&
			 (type->type != INDEX_REGISTRY_TYPE_WILDCARD) &&
			 (type->type != INDEX_REGISTRY_TYPE_USER_KEY)) ||
			(value == NULL) || (length == NULL)) {
		DISPLAY_INVALID_ARGS;

		if(length != NULL) {
			*length = 0;
		}

		return NULL;
	}

	*length = 0;

	if(spinlock_readLock(&index->lock) < 0) {
		return NULL;
	}

	if(bptree_get(&index->index, type->key, strlen(type->key),
				((void *)&entry)) < 0) {
		spinlock_readUnlock(&index->lock);
		return NULL;
	}

	mutex_lock(&entry->mutex);

	if(entry->type != type->type) {
		mutex_unlock(&entry->mutex);
		spinlock_readUnlock(&index->lock);
		return NULL;
	}

	if(type->type != INDEX_REGISTRY_TYPE_USER_KEY) {
		result = wildcardIndex_get((WildcardIndex *)entry->index, value,
				length);
	}
	else if(bptree_get((Bptree *)entry->index, value, strlen(value),
				((void *)&array)) == 0) {
		resultLength = array->length;

		result = (int *)malloc(sizeof(int) * resultLength);

		memcpy(result, array->array, (int)(sizeof(int) * resultLength));

		*length = resultLength;
	}

	mutex_unlock(&entry->mutex);

	spinlock_readUnlock(&index->lock);

	return result;
}

int *searchIndex_getRange(SearchIndex *index, IndexRegistryIndex *type,
		int searchType, int minValue, int maxValue, int *length)
{
	int *result = NULL;

	SearchIndexEntry *entry = NULL;

	if((index == NULL) || (type == NULL) ||
			(type->type != INDEX_REGISTRY_TYPE_RANGE) || (length == NULL)) {
		DISPLAY_INVALID_ARGS;

		if(length != NULL) {
			*length = 0;
		}

		return NULL;
	}

	*length = 0;

	if(spinlock_readLock(&index->lock) < 0) {
		return NULL;
	}

	if(bptree_get(&index->index, type->key, strlen(type->key),
				((void *)&entry)) < 0) {
		spinlock_readUnlock(&index->lock);
		return NULL;
	}

	mutex_lock(&entry->mutex);

	if(entry->type != type->type) {
		mutex_unlock(&entry->mutex);
		spinlock_readUnlock(&index->lock);
		return NULL;
	}

	result = rangeIndex_search((RangeIndex *)entry->index, searchType,
			minValue, maxValue, length);

	mutex_unlock(&entry->mutex);

	spinlock_readUnlock(&index->lock);

	return result;
}

int searchIndex_putString(SearchIndex *index, IndexRegistryIndex *type,
		char *value, int uid)
{
	int result = 0;

	IntegerArray *array = NULL;
	SearchIndexEntry *entry = NULL;

	if((index == NULL) || (type == NULL) ||
			((type->type != INDEX_REGISTRY_TYPE_EXACT) &&
			 (type->type != INDEX_REGISTRY_TYPE_WILDCARD) &&
			 (type->type != INDEX_REGISTRY_TYPE_USER_KEY)) ||
			(value == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(spinlock_writeLock(&index->lock) < 0) {
		return -1;
	}

	if(bptree_get(&index->index, type->key, strlen(type->key),
				((void *)&entry)) < 0) {
		spinlock_writeUnlock(&index->lock);
		return -1;
	}

	mutex_lock(&entry->mutex);

	if(entry->type != type->type) {
		mutex_unlock(&entry->mutex);
		spinlock_writeUnlock(&index->lock);
		return -1;
	}

	if(entry->type == INDEX_REGISTRY_TYPE_EXACT) {
		result = wildcardIndex_put((WildcardIndex *)entry->index,
				WILDCARD_INDEX_TYPE_EXACT,
				value,
				uid);
	}
	else if(type->type == INDEX_REGISTRY_TYPE_WILDCARD) {
		result = wildcardIndex_put((WildcardIndex *)entry->index,
				WILDCARD_INDEX_TYPE_STANDARD,
				value,
				uid);
	}
	else {
		if(bptree_get((Bptree *)entry->index, value, strlen(value),
					((void *)&array)) == 0) {
			mutex_unlock(&entry->mutex);
			spinlock_writeUnlock(&index->lock);
			return -1;
		}

		array = integer_new();
		integer_put(array, uid);

		bptree_put((Bptree *)entry->index, value, strlen(value), (void *)array);
	}

	mutex_unlock(&entry->mutex);

	spinlock_writeUnlock(&index->lock);

	return result;
}

int searchIndex_putInteger(SearchIndex *index, IndexRegistryIndex *type,
		int value, int uid)
{
	int result = 0;

	SearchIndexEntry *entry = NULL;

	if((index == NULL) || (type == NULL) ||
			(type->type != INDEX_REGISTRY_TYPE_RANGE)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(spinlock_writeLock(&index->lock) < 0) {
		return -1;
	}

	if(bptree_get(&index->index, type->key, strlen(type->key),
				((void *)&entry)) < 0) {
		spinlock_writeUnlock(&index->lock);
		return -1;
	}

	mutex_lock(&entry->mutex);

	if(entry->type != type->type) {
		mutex_unlock(&entry->mutex);
		spinlock_writeUnlock(&index->lock);
		return -1;
	}

	result = rangeIndex_put((RangeIndex *)entry->index, value, uid);

	mutex_unlock(&entry->mutex);

	spinlock_writeUnlock(&index->lock);

	return result;
}

int searchIndex_removeString(SearchIndex *index, IndexRegistryIndex *type,
		char *value, int uid)
{
	int result = 0;

	SearchIndexEntry *entry = NULL;

	if((index == NULL) || (type == NULL) ||
			((type->type != INDEX_REGISTRY_TYPE_EXACT) &&
			 (type->type != INDEX_REGISTRY_TYPE_WILDCARD) &&
			 (type->type != INDEX_REGISTRY_TYPE_USER_KEY)) ||
			(value == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(spinlock_writeLock(&index->lock) < 0) {
		return -1;
	}

	if(bptree_get(&index->index, type->key, strlen(type->key),
				((void *)&entry)) < 0) {
		spinlock_writeUnlock(&index->lock);
		return -1;
	}

	mutex_lock(&entry->mutex);

	if(entry->type != type->type) {
		mutex_unlock(&entry->mutex);
		spinlock_writeUnlock(&index->lock);
		return -1;
	}

	if(type->type != INDEX_REGISTRY_TYPE_USER_KEY) {
		result = wildcardIndex_remove((WildcardIndex *)entry->index, value,
				uid);
	}
	else {
		result = bptree_remove((Bptree *)entry->index, value, strlen(value));
	}

	mutex_unlock(&entry->mutex);

	spinlock_writeUnlock(&index->lock);

	return result;
}

int searchIndex_removeInteger(SearchIndex *index, IndexRegistryIndex *type,
		int value, int uid)
{
	int result = 0;

	SearchIndexEntry *entry = NULL;

	if((index == NULL) || (type == NULL) ||
			(type->type != INDEX_REGISTRY_TYPE_RANGE)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(spinlock_writeLock(&index->lock) < 0) {
		return -1;
	}

	if(bptree_get(&index->index, type->key, strlen(type->key),
				((void *)&entry)) < 0) {
		spinlock_writeUnlock(&index->lock);
		return -1;
	}

	mutex_lock(&entry->mutex);

	if(entry->type != type->type) {
		mutex_unlock(&entry->mutex);
		spinlock_writeUnlock(&index->lock);
		return -1;
	}

	result = rangeIndex_remove((RangeIndex *)entry->index, value, uid);

	mutex_unlock(&entry->mutex);

	spinlock_writeUnlock(&index->lock);

	return result;
}

