/*
 * index_registry.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * A library component for the search-engine to define the set of indexes
 * maintained by the search-engine system.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#include "search/search.h"
#define _SEARCH_CORE_COMPONENT
#include "search/core/index_registry.h"


// declare index registry private data types

typedef enum _IndexRegistryGeoCoordType {
	IR_GEOCOORD_TYPE_NONE = 1,
	IR_GEOCOORD_TYPE_LATITUDE,
	IR_GEOCOORD_TYPE_LONGITUDE,
	IR_GEOCOORD_TYPE_ERROR = -1
} IndexRegistryGeoCoordType;


// declare index registry private functions

static IndexRegistryGeoCoordType isKeyGeoCoordAttribute(char *key);

static IndexRegistryIndex *indexNew(IndexRegistryTypes type, int uid,
		char *key, char *name);

static void indexFree(void *memory);


// define index registry private functions

static IndexRegistryGeoCoordType isKeyGeoCoordAttribute(char *key)
{
	IndexRegistryGeoCoordType result = IR_GEOCOORD_TYPE_NONE;
	int ii = 0;
	int keyLength = 0;
	char *ptr = NULL;

	if((key == NULL) || ((keyLength = strlen(key)) < 8)) {
		return IR_GEOCOORD_TYPE_NONE;
	}

	if(((ptr = strstr(key, "latitude")) != NULL) && (ptr == key)) {
		result = IR_GEOCOORD_TYPE_LATITUDE;

		for(ii = 8; ii < keyLength; ii++) {
			if(!ctype_isNumeric(key[ii])) {
				result = IR_GEOCOORD_TYPE_NONE;
				break;
			}
		}
	}
	else if(((ptr = strstr(key, "longitude")) != NULL) && (ptr == key)) {
		result = IR_GEOCOORD_TYPE_LONGITUDE;

		for(ii = 9; ii < keyLength; ii++) {
			if(!ctype_isNumeric(key[ii])) {
				result = IR_GEOCOORD_TYPE_NONE;
				break;
			}
		}
	}

	return result;
}

static IndexRegistryIndex *indexNew(IndexRegistryTypes type, int uid,
		char *key, char *name)
{
	IndexRegistryIndex *result = NULL;

	result = (IndexRegistryIndex *)malloc(sizeof(IndexRegistryIndex));

	result->type = type;
	result->uid = uid;
	result->keyLength = strlen(key);
	result->key = strndup(key, result->keyLength);
	result->nameLength = strlen(name);
	result->name = strndup(name, result->nameLength);

	result->settings.isOverrideStringLengths = afalse;
	result->settings.isOverrideDelimiters = afalse;
	result->settings.isOverrideExcludedWords = afalse;
	result->settings.isOverrideIndexing = afalse;
	result->settings.isIndexFullString = afalse;
	result->settings.isIndexTokenizedString = afalse;
	result->settings.minStringLength = 0;
	result->settings.maxStringLength = 0;
	result->settings.delimitersLength = 0;
	result->settings.delimiters = NULL;
	result->settings.excludedWordConfigFilename = NULL;
	result->settings.excludedWords = NULL;

	mutex_init(&result->settings.mutex);

	return result;
}

static void indexFree(void *memory)
{
	IndexRegistryIndex *index = NULL;

	if(memory == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	index = (IndexRegistryIndex *)memory;

	if(index->key != NULL) {
		free(index->key);
	}

	if(index->name != NULL) {
		free(index->name);
	}

	mutex_lock(&index->settings.mutex);

	if(index->settings.delimiters != NULL) {
		free(index->settings.delimiters);
	}

	if(index->settings.excludedWordConfigFilename != NULL) {
		free(index->settings.excludedWordConfigFilename);
	}

	if(index->settings.excludedWords != NULL) {
		bptree_free(index->settings.excludedWords);
		free(index->settings.excludedWords);
	}

	mutex_unlock(&index->settings.mutex);
	mutex_free(&index->settings.mutex);

	memset(index, 0, (int)(sizeof(IndexRegistryIndex)));

	free(index);
}


// define index registry public functions

int indexRegistry_init(IndexRegistry *registry)
{
	if(registry == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	memset(registry, 0, (int)(sizeof(IndexRegistry)));

	registry->uidCounter = 0;

	if(bptree_init(&registry->index) < 0) {
		return -1;
	}

	if(bptree_setFreeFunction(&registry->index, indexFree) < 0) {
		return -1;
	}

	if(spinlock_init(&registry->lock) < 0) {
		return -1;
	}

	return 0;
}

IndexRegistry *indexRegistry_new()
{
	IndexRegistry *result = NULL;

	result = (IndexRegistry *)malloc(sizeof(IndexRegistry));

	if(indexRegistry_init(result) < 0) {
		indexRegistry_freePtr(result);
		return NULL;
	}

	return result;
}

int indexRegistry_free(IndexRegistry *registry)
{
	if(registry == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(spinlock_writeLock(&registry->lock) < 0) {
		return -1;
	}

	if(bptree_free(&registry->index) < 0) {
		spinlock_writeUnlock(&registry->lock);
		return -1;
	}

	spinlock_writeUnlock(&registry->lock);

	if(spinlock_free(&registry->lock) < 0) {
		return -1;
	}

	memset(registry, 0, (int)(sizeof(IndexRegistry)));

	return 0;
}

int indexRegistry_freePtr(IndexRegistry *registry)
{
	if(registry == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(indexRegistry_free(registry) < 0) {
		return -1;
	}

	free(registry);

	return 0;
}

char *indexRegistry_typeToString(IndexRegistryTypes type)
{
	char *result = NULL;

	switch(type) {
		case INDEX_REGISTRY_TYPE_EXACT:
			result = "Exact";
			break;

		case INDEX_REGISTRY_TYPE_WILDCARD:
			result = "Wildcard";
			break;

		case INDEX_REGISTRY_TYPE_RANGE:
			result = "Range";
			break;

		case INDEX_REGISTRY_TYPE_USER_KEY:
			result = "User Key";
			break;

		case INDEX_REGISTRY_TYPE_END_OF_LIST:
		default:
			result = "Unknown";
	}

	return result;
}

IndexRegistryTypes indexRegistry_stringToType(char *string)
{
	IndexRegistryTypes result = INDEX_REGISTRY_TYPE_END_OF_LIST;

	if(string == NULL) {
		DISPLAY_INVALID_ARGS;
		return INDEX_REGISTRY_TYPE_END_OF_LIST;
	}

	if(!strcasecmp(string, "Exact")) {
		result = INDEX_REGISTRY_TYPE_EXACT;
	}
	else if(!strcasecmp(string, "Wildcard")) {
		result = INDEX_REGISTRY_TYPE_WILDCARD;
	}
	else if(!strcasecmp(string, "Range")) {
		result = INDEX_REGISTRY_TYPE_RANGE;
	}
	else if(!strcasecmp(string, "User Key")) {
		result = INDEX_REGISTRY_TYPE_USER_KEY;
	}

	return result;
}

aboolean indexRegistry_isLocked(IndexRegistry *registry)
{
	if(registry == NULL) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	return spinlock_isLocked(&registry->lock);
}

aboolean indexRegistry_isReadLocked(IndexRegistry *registry)
{
	int threadCount = 0;

	if(registry == NULL) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	if(spinlock_getSpinThreadCount(&registry->lock, &threadCount) < 0) {
		return afalse;
	}

	if(threadCount > 0) {
		return atrue;
	}

	return afalse;
}

int indexRegistry_lock(IndexRegistry *registry)
{
	if(registry == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return spinlock_readLock(&registry->lock);
}

int indexRegistry_unlock(IndexRegistry *registry)
{
	if(registry == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return spinlock_readUnlock(&registry->lock);
}

int indexRegistry_put(IndexRegistry *registry, IndexRegistryTypes type,
		char *key, char *name)
{
	int keyLength = 0;
	int nameLength = 0;

	IndexRegistryIndex *index = NULL;

	if((registry == NULL) ||
			((type != INDEX_REGISTRY_TYPE_EXACT) &&
			 (type != INDEX_REGISTRY_TYPE_WILDCARD) &&
			 (type != INDEX_REGISTRY_TYPE_RANGE) &&
			 (type != INDEX_REGISTRY_TYPE_USER_KEY)) ||
			(key == NULL) ||
			((keyLength = strlen(key)) < 1) ||
			(name == NULL) ||
			((nameLength = strlen(name)) < 1)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(spinlock_writeLock(&registry->lock) < 0) {
		return -1;
	}

	if(bptree_get(&registry->index, key, keyLength, ((void *)&index)) == 0) {
		spinlock_writeUnlock(&registry->lock);
		return -1;
	}

	index = indexNew(type, registry->uidCounter, key, name);

	if(bptree_put(&registry->index, key, keyLength, (void *)index) < 0) {
		spinlock_writeUnlock(&registry->lock);
		return -1;
	}

	registry->uidCounter += 1;

	spinlock_writeUnlock(&registry->lock);

	return 0;
}

IndexRegistryIndex *indexRegistry_get(IndexRegistry *registry, char *key)
{
	IndexRegistryGeoCoordType geoCoordType = IR_GEOCOORD_TYPE_NONE;
	int keyLength = 0;

	IndexRegistryIndex *result = NULL;

	if((registry == NULL) || (key == NULL) || ((keyLength = strlen(key)) < 1)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	geoCoordType = isKeyGeoCoordAttribute(key);

	if(geoCoordType == IR_GEOCOORD_TYPE_LATITUDE) {
		key = "latitude";
		keyLength = 8;
	}
	else if(geoCoordType == IR_GEOCOORD_TYPE_LONGITUDE) {
		key = "longitude";
		keyLength = 9;
	}

	if(spinlock_readLock(&registry->lock) < 0) {
		return NULL;
	}

	bptree_get(&registry->index, key, keyLength, ((void *)&result));

	spinlock_readUnlock(&registry->lock);

	return result;
}

int indexRegistry_remove(IndexRegistry *registry, char *key)
{
	int result = 0;
	int keyLength = 0;

	if((registry == NULL) || (key == NULL) || ((keyLength = strlen(key)) < 1)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(spinlock_writeLock(&registry->lock) < 0) {
		return -1;
	}

	result = bptree_remove(&registry->index, key, keyLength);

	spinlock_writeUnlock(&registry->lock);

	return result;
}

int indexRegistry_reset(IndexRegistry *registry, char *key,
		IndexRegistryTypes newType, char *newKey, char *newName)
{
	int uid = 0;
	int keyLength = 0;
	int newKeyLength = 0;
	int newNameLength = 0;

	IndexRegistryIndex *index = NULL;

	if((registry == NULL) ||
			(key == NULL) ||
			((keyLength = strlen(key)) < 1) ||
			((newType != INDEX_REGISTRY_TYPE_EXACT) &&
			 (newType != INDEX_REGISTRY_TYPE_WILDCARD) &&
			 (newType != INDEX_REGISTRY_TYPE_RANGE) &&
			 (newType != INDEX_REGISTRY_TYPE_USER_KEY)) ||
			(newKey == NULL) ||
			((newKeyLength = strlen(newKey)) < 1) ||
			(newName == NULL) ||
			((newNameLength = strlen(newName)) < 1)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(spinlock_writeLock(&registry->lock) < 0) {
		return -1;
	}

	if(bptree_get(&registry->index, key, keyLength, ((void *)&index)) < 0) {
		spinlock_writeUnlock(&registry->lock);
		return -1;
	}

	uid = index->uid;

	if(bptree_remove(&registry->index, key, keyLength) < 0) {
		spinlock_writeUnlock(&registry->lock);
		return -1;
	}

	index = indexNew(newType, uid, newKey, newName);

	if(bptree_put(&registry->index, newKey, newKeyLength, (void *)index) < 0) {
		spinlock_writeUnlock(&registry->lock);
		return -1;
	}

	spinlock_writeUnlock(&registry->lock);

	return 0;
}

// settings functions

int indexRegistry_setStringIndexThresholds(IndexRegistry *registry, char *key,
		int minStringLength, int maxStringLength)
{
	int rc = 0;
	int keyLength = 0;

	IndexRegistryIndex *result = NULL;

	if((registry == NULL) || (key == NULL) ||
			((keyLength = strlen(key)) < 1) ||
			(minStringLength < 1) ||
			(maxStringLength < 1) ||
			(minStringLength > maxStringLength) ||
			(maxStringLength >= SEARCH_ENGINE_CORE_MAX_STRING_INDEX_LENGTH)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(spinlock_readLock(&registry->lock) < 0) {
		return -1;
	}

	rc = bptree_get(&registry->index, key, keyLength, ((void *)&result));

	spinlock_readUnlock(&registry->lock);

	if((rc == 0) && (result != NULL)) {
		mutex_lock(&result->settings.mutex);

		result->settings.isOverrideStringLengths = atrue;
		result->settings.minStringLength = minStringLength;
		result->settings.maxStringLength = maxStringLength;

		mutex_unlock(&result->settings.mutex);
	}
	else {
		return -1;
	}

	return 0;
}

int indexRegistry_setStringDelimiters(IndexRegistry *registry, char *key,
		char *delimiters, int delimitersLength)
{
	int rc = 0;
	int keyLength = 0;

	IndexRegistryIndex *result = NULL;

	if((registry == NULL) || (key == NULL) ||
			((keyLength = strlen(key)) < 1) || (delimiters == NULL) ||
			(delimitersLength < 1)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(spinlock_readLock(&registry->lock) < 0) {
		return -1;
	}

	rc = bptree_get(&registry->index, key, keyLength, ((void *)&result));

	spinlock_readUnlock(&registry->lock);

	if((rc == 0) && (result != NULL)) {
		mutex_lock(&result->settings.mutex);

		result->settings.isOverrideDelimiters = atrue;

		if(result->settings.delimiters != NULL) {
			free(result->settings.delimiters);
		}

		result->settings.delimiters = strndup(delimiters, delimitersLength);
		result->settings.delimitersLength = delimitersLength;

		mutex_unlock(&result->settings.mutex);
	}
	else {
		return -1;
	}

	return 0;
}

int indexRegistry_loadExcludedWords(IndexRegistry *registry, char *key,
		Log *log, Esa *esa, char *filename)
{
	int rc = 0;
	int keyLength = 0;

	IndexRegistryIndex *result = NULL;

	if((registry == NULL) || (key == NULL) ||
			((keyLength = strlen(key)) < 1) || (esa == NULL) ||
			(filename == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(spinlock_readLock(&registry->lock) < 0) {
		return -1;
	}

	rc = bptree_get(&registry->index, key, keyLength, ((void *)&result));

	spinlock_readUnlock(&registry->lock);

	if((rc == 0) && (result != NULL)) {
		mutex_lock(&result->settings.mutex);

		if(result->settings.excludedWordConfigFilename != NULL) {
			free(result->settings.excludedWordConfigFilename);
		}

		if(result->settings.excludedWords != NULL) {
			bptree_free(result->settings.excludedWords);
			free(result->settings.excludedWords);
		}

		result->settings.excludedWordConfigFilename = strdup(filename);
		result->settings.excludedWords = (Bptree *)malloc(sizeof(Bptree));

		bptree_init(result->settings.excludedWords);

		if(searchEngineCommon_loadExcludedWords(log, esa, filename,
					result->settings.excludedWords) == 0) {
			result->settings.isOverrideExcludedWords = atrue;
		}

		mutex_unlock(&result->settings.mutex);
	}
	else {
		return -1;
	}

	return 0;
}

int indexRegistry_setStringIndexing(IndexRegistry *registry, char *key,
		aboolean isIndexFullString, aboolean isIndexTokenizedString)
{
	int rc = 0;
	int keyLength = 0;

	IndexRegistryIndex *result = NULL;

	if((registry == NULL) || (key == NULL) || ((keyLength = strlen(key)) < 1)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(spinlock_readLock(&registry->lock) < 0) {
		return -1;
	}

	rc = bptree_get(&registry->index, key, keyLength, ((void *)&result));

	spinlock_readUnlock(&registry->lock);

	if((rc == 0) && (result != NULL)) {
		mutex_lock(&result->settings.mutex);

		result->settings.isOverrideIndexing = atrue;
		result->settings.isIndexFullString = isIndexFullString;
		result->settings.isIndexTokenizedString = isIndexTokenizedString;

		mutex_unlock(&result->settings.mutex);
	}
	else {
		return -1;
	}

	return 0;
}

// helper functions

int indexRegistry_updateConfigContext(IndexRegistry *registry, char *key,
		Config *config, char *section)
{
	int rc = 0;
	int keyLength = 0;

	IndexRegistryIndex *result = NULL;

	if((registry == NULL) || (key == NULL) ||
			((keyLength = strlen(key)) < 1) || (config == NULL) ||
			(section == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(spinlock_readLock(&registry->lock) < 0) {
		return -1;
	}

	rc = bptree_get(&registry->index, key, keyLength, ((void *)&result));

	spinlock_readUnlock(&registry->lock);

	if((rc == 0) || (result != NULL)) {
		config_putString(config, section, "type",
				indexRegistry_typeToString(result->type));
		config_putInteger(config, section, "uid", result->uid);
		config_putString(config, section, "key", result->key);
		config_putString(config, section, "name", result->name);

		mutex_lock(&result->settings.mutex);

		if(result->settings.isOverrideStringLengths) {
			config_putInteger(config, section, "minStringLength",
					result->settings.minStringLength);
			config_putInteger(config, section, "maxStringLength",
					result->settings.maxStringLength);
		}

		if(result->settings.isOverrideDelimiters) {
			config_putString(config, section, "stringDelimiters",
					result->settings.delimiters);
		}

		if(result->settings.isOverrideExcludedWords) {
			config_putString(config, section, "excludedWordConfigFilename",
					result->settings.excludedWordConfigFilename);
		}

		if(result->settings.isOverrideIndexing) {
			config_putBoolean(config, section, "isIndexFullString",
					result->settings.isIndexFullString);
			config_putBoolean(config, section, "isIndexTokenizedString",
					result->settings.isIndexTokenizedString);
		}

		mutex_unlock(&result->settings.mutex);
	}
	else {
		return -1;
	}

	return 0;
}

int indexRegistry_updateSettingsFromConfig(IndexRegistry *registry, char *key,
		Config *config, char *section, Log *log, Esa *esa)
{
	aboolean isIndexFullString = afalse;
	aboolean isIndexTokenizedString = afalse;
	int rc = 0;
	int iValue = 0;
	int iValueTwo = 0;
	int keyLength = 0;
	char *sValue = NULL;

	IndexRegistryIndex *result = NULL;

	if((registry == NULL) || (key == NULL) ||
			((keyLength = strlen(key)) < 1) || (config == NULL) ||
			(section == NULL) || (esa == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(spinlock_readLock(&registry->lock) < 0) {
		return -1;
	}

	rc = bptree_get(&registry->index, key, keyLength, ((void *)&result));

	spinlock_readUnlock(&registry->lock);

	if((rc == 0) || (result != NULL)) {
		if((config_exists(config, section, "minStringLength")) &&
				(config_exists(config, section, "maxStringLength"))) {
			config_getInteger(config, section, "minStringLength",
					SEARCH_ENGINE_DEFAULT_MIN_STRING_LENGTH,
					&iValue);

			config_getInteger(config, section, "maxStringLength",
					SEARCH_ENGINE_DEFAULT_MAX_STRING_LENGTH,
					&iValueTwo);

			indexRegistry_setStringIndexThresholds(registry, key, iValue,
					iValueTwo);
		}

		if(config_exists(config, section, "stringDelimiters")) {
			config_getString(config, section, "stringDelimiters",
					SEARCH_ENGINE_DEFAULT_DELIMITERS,
					&sValue);

			indexRegistry_setStringDelimiters(registry, key, sValue,
					strlen(sValue));
		}

		if(config_exists(config, section, "excludedWordConfigFilename")) {
			config_getString(config, section, "excludedWordConfigFilename",
					SEARCH_ENGINE_DEFAULT_EXCLUDED_WORDS_CONFIG,
					&sValue);

			indexRegistry_loadExcludedWords(registry, key, log, esa, sValue);
		}

		if((config_exists(config, section, "isIndexFullString")) &&
				(config_exists(config, section, "isIndexTokenizedString"))) {
			config_getBoolean(config, section, "isIndexFullString",
					afalse, &isIndexFullString);
			config_getBoolean(config, section, "isIndexTokenizedString",
					afalse, &isIndexTokenizedString);

			indexRegistry_setStringIndexing(registry, key, isIndexFullString,
					isIndexTokenizedString);
		}
	}
	else {
		return -1;
	}

	return 0;
}

