/*
 * search_engine.c
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
#define _SEARCH_SYSTEM_COMPONENT
#include "search/system/search_engine.h"


// define search engine private constants

typedef enum _SearchEngineQueueCommands {
	SEARCH_ENGINE_QUEUE_COMMAND_FREE = 1,
	SEARCH_ENGINE_QUEUE_COMMAND_PUT,
	SEARCH_ENGINE_QUEUE_COMMAND_UPDATE,
	SEARCH_ENGINE_QUEUE_COMMAND_ERROR = -1
} SearchEngineQueueCommands;


// define search engine private data types

typedef struct _SearchEngineQueueEntry {
	SearchEngineQueueCommands type;
	char flags;
	void *payload;
} SearchEngineQueueEntry;


// declare search engine private functions

// general functions

static void freeNothing(void *memory);

static void freeIndex(void *memory);

static void freeContainer(void *argument, void *memory);

// queue functions

SearchEngineQueueEntry *queue_buildEntry(SearchEngineQueueCommands type,
		void *payload);

static void queue_freeEntry(void *memory);

static int queue_executeAddition(SearchEngine *engine,
		SearchEngineQueueEntry *entry);

// index functions

static int index_updateFullString(SearchEngine *engine, aboolean isPut,
		SearchIndex *searchIndex, IndexRegistryIndex *indexType, char *string,
		int stringLength, int uid);

static int index_updateTokenizedString(SearchEngine *engine, aboolean isPut,
		SearchIndex *searchIndex, IndexRegistryIndex *indexType, char *string,
		int stringLength, int uid);

static int index_updateString(SearchEngine *engine, aboolean isPut,
		SearchIndex *searchIndex, IndexRegistryIndex *indexType,
		ContainerIterator *iterator, int uid);

static int index_updateInteger(SearchEngine *engine, aboolean isPut,
		SearchIndex *searchIndex, IndexRegistryIndex *indexType,
		ContainerIterator *iterator, int uid);

static int index_reindexContainers(SearchEngine *engine,
		IndexRegistryIndex *indexType);

// container functions

static int container_reindex(SearchEngine *engine,
		IndexRegistryIndex *indexType, SearchIndex *searchIndex,
		Container *container);

static int container_updateIndexes(SearchEngine *engine, aboolean isPut,
		SearchIndex *searchIndex, Container *container);

static char *container_buildRelevancyStringArray(SearchEngine *engine,
		IndexRegistryIndex *indexType, ContainerIterator *iterator,
		int *resultStringLength, int *resultArrayLength);

static int container_calculateRelevancy(SearchEngine *engine,
		Container *container);

// thread functions

char *thread_buildStateFilename(SearchEngine *engine, int type);

char *thread_buildContainerFilename(SearchEngine *engine, int type);

static int thread_writeStateToFile(SearchEngine *engine);

static int thread_readStateFromFile(SearchEngine *engine, char *filename);

static int thread_writeContainersToFile(SearchEngine *engine);

static int thread_readContainersFromFile(SearchEngine *engine, char *filename);

static void *thread_searchEngineWorker(void *threadContext, void *argument);


// define search engine private functions

// general functions

static void freeNothing(void *memory)
{
	if(memory == NULL) {
		DISPLAY_INVALID_ARGS;
	}
}

static void freeIndex(void *memory)
{
	SearchIndex *index = NULL;

	if(memory == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	index = (SearchIndex *)memory;

	searchIndex_freePtr(index);
}

static void freeContainer(void *argument, void *memory)
{
	int domainKeyLength = 0;
	char *domainKey = NULL;

	Container *container = NULL;
	SearchIndex *searchIndex = NULL;
	SearchEngine *engine = NULL;
	SearchEngineQueueEntry *entry = NULL;

	if((argument == NULL) || (memory == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	engine = (SearchEngine *)argument;
	container = (Container *)memory;

	// remove the container indexes

	if(container_lock(container) < 0) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"failed to obtain container read lock for free");
		return;
	}

	domainKey = container_getString(container, SEARCHD_DOMAIN_KEY,
			SEARCHD_DOMAIN_KEY_LENGTH, &domainKeyLength);

	if((domainKey != NULL) && (domainKeyLength > 0)) {
		if(spinlock_readLock(&engine->lock) < 0) {
			log_logf(engine->log, LOG_LEVEL_ERROR,
					"failed to obtain spinlock for domain lookup from %s()",
					__FUNCTION__);
		}
		else {
			if((bptree_get(&engine->indexes, domainKey, domainKeyLength,
							((void *)&searchIndex)) == 0) &&
					(searchIndex != NULL)) {
				if(container_updateIndexes(engine, afalse, searchIndex,
							container) < 0) {
					log_logf(engine->log, LOG_LEVEL_ERROR,
							"failed to remove indexes for container #%i",
							container_getUid(container));
				}
			}
			else {
				log_logf(engine->log, LOG_LEVEL_ERROR,
						"unable to locate domain '%s' to remove container #%i",
						domainKey, container_getUid(container));
			}

			spinlock_readUnlock(&engine->lock);
		}
	}
	else {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"failed to locate '%s' in container #%i",
				SEARCHD_DOMAIN_KEY, container_getUid(container));
	}

	container_unlock(container);

	if(domainKey != NULL) {
		free(domainKey);
	}

	// construct the queue entry & add it to the queue

	entry = queue_buildEntry(SEARCH_ENGINE_QUEUE_COMMAND_FREE, container);

	mutex_lock(&engine->queueMutex);

	if(fifostack_push(&engine->queue, entry) < 0) {
		queue_freeEntry(entry);
	}

	mutex_unlock(&engine->queueMutex);
}

// queue functions

SearchEngineQueueEntry *queue_buildEntry(SearchEngineQueueCommands type,
		void *payload)
{
	SearchEngineQueueEntry *result = NULL;

	result = (SearchEngineQueueEntry *)malloc(sizeof(SearchEngineQueueEntry));
	result->type = type;
	result->payload = payload;

	return result;
}

static void queue_freeEntry(void *memory)
{
	SearchEngineQueueEntry *entry = NULL;

	if(memory == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	entry = (SearchEngineQueueEntry *)memory;

	switch(entry->type) {
		case SEARCH_ENGINE_QUEUE_COMMAND_FREE:
			container_freePtr(entry->payload);
			break;

		case SEARCH_ENGINE_QUEUE_COMMAND_PUT:
		case SEARCH_ENGINE_QUEUE_COMMAND_UPDATE:
			/*
			 * do nothing
			 */
			break;

		case SEARCH_ENGINE_QUEUE_COMMAND_ERROR:
		default:
			DISPLAY_INVALID_ARGS;
	}

	free(entry);
}

static int queue_executeAddition(SearchEngine *engine,
		SearchEngineQueueEntry *entry)
{
	int keyLength = 0;
	int domainKeyLength = 0;
	char key[128];
	char *domainKey = NULL;
	char *errorMessage = NULL;

	Container *container = NULL;
	Container *localContainer = NULL;
	Container *updatedContainer = NULL;
	SearchIndex *searchIndex = NULL;

	if((entry == NULL) ||
			((entry->type != SEARCH_ENGINE_QUEUE_COMMAND_PUT) &&
			 (entry->type != SEARCH_ENGINE_QUEUE_COMMAND_UPDATE))) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	container = (Container *)entry->payload;

	// calculate container relevancy

	container_calculateRelevancy(engine, container);

	// read-lock the container

	if(container_lock(container) < 0) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"failed to obtain container read lock for buffered put");
		container_freePtr(container);
		return -1;
	}

	// obtain the domain key from the container

	domainKey = container_getString(container, SEARCHD_DOMAIN_KEY,
			SEARCHD_DOMAIN_KEY_LENGTH, &domainKeyLength);

	if((domainKey == NULL) || (domainKeyLength < 1)) {
		if(domainKey != NULL) {
			free(domainKey);
		}

		log_logf(engine->log, LOG_LEVEL_ERROR,
				"failed to obtain 'domainKey' attribute from container");

		container_unlock(container);
		container_freePtr(container);
		return -1;
	}

	// build UID key

	if(searchEngineCommon_buildUidKey(container_getUid(container), key,
				&keyLength) < 0) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"failed to build index key for container #%i domain '%s'",
				container_getUid(container),
				domainKey);
		free(domainKey);
		container_unlock(container);
		container_freePtr(container);
		return SEARCH_ENGINE_ERROR_COMMON_FAILED_TO_BUILD_UID_KEY;
	}

	// determine if container already exists for this UID

	if(managedIndex_get(&engine->containers, key, keyLength,
				((void *)&localContainer)) == 0) {
		if(entry->type == SEARCH_ENGINE_QUEUE_COMMAND_PUT) {
			log_logf(engine->log, LOG_LEVEL_ERROR,
					"unable to put container container #%i domain '%s', "
					"already exists",
					container_getUid(container),
					domainKey);
			free(domainKey);
			container_unlock(container);
			container_freePtr(container);
			return -1;
		}

		// combine the containers

		if(container_lock(localContainer) < 0) {
			log_logf(engine->log, LOG_LEVEL_WARNING,
					"failed to obtain container read lock for removal");
		}

		if((updatedContainer = container_combine(localContainer,
						container, entry->flags, &errorMessage)) == NULL) {
			log_logf(engine->log, LOG_LEVEL_ERROR,
					"failed to perform container-combine #%i/#%i from "
					"domain '%s' with '%s'",
					container_getUid(localContainer),
					container_getUid(container),
					domainKey,
					errorMessage);
			free(domainKey);
			container_unlock(container);
			container_freePtr(container);
			container_unlock(localContainer);
			return -1;
		}

		// update the container with reserved entities

		container_putString(updatedContainer, SEARCHD_DOMAIN_KEY,
				SEARCHD_DOMAIN_KEY_LENGTH, domainKey, domainKeyLength);

		// read-unlock, free, and reassign the container

		container_unlock(container);
		container_freePtr(container);

		container = updatedContainer;
		container_lock(container);

		// remove the old container

		if(managedIndex_remove(&engine->containers, key, keyLength) < 0) {
			log_logf(engine->log, LOG_LEVEL_ERROR,
					"failed to remove container #%i "
					"domain '%s' from master index",
					container_getUid(localContainer),
					domainKey);
			free(domainKey);
			container_unlock(container);
			container_freePtr(container);
			container_unlock(localContainer);
			return -1;
		}

		container_unlock(localContainer);
	}

	// add container to managed index

	if(managedIndex_put(&engine->containers, key, keyLength, container,
				container_calculateMemoryLength(container)) < 0) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"failed to put container #%i in master index for domain '%s'",
				container_getUid(container), domainKey);
		container_unlock(container);
		container_freePtr(container);
		return -1;
	}

	// update the container indexes

	if(spinlock_readLock(&engine->lock) < 0) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"failed to obtain spinlock for domain "
				"lookup from %s()", __FUNCTION__);
	}
	else {
		if((bptree_get(&engine->indexes,
						domainKey,
						domainKeyLength,
						((void *)&searchIndex)) == 0) &&
				(searchIndex != NULL)) {
			if(container_updateIndexes(engine,
						atrue, // is put
						searchIndex,
						container) < 0) {
				log_logf(engine->log, LOG_LEVEL_ERROR,
						"failed to add indexes for container #%i",
						container_getUid(container));
			}
		}
		else {
			log_logf(engine->log, LOG_LEVEL_ERROR,
					"unable to locate domain '%s' to add container #%i",
					domainKey, container_getUid(container));
		}

		spinlock_readUnlock(&engine->lock);
	}

	container_unlock(container);

	// cleanup

	if(domainKey != NULL) {
		free(domainKey);
	}

	return 0;
}

// index functions

static int index_updateFullString(SearchEngine *engine, aboolean isPut,
		SearchIndex *searchIndex, IndexRegistryIndex *indexType, char *string,
		int stringLength, int uid)
{
	int length = 0;
	int resultLength = 0;
	int allocatedLength = 0;
	int minStringLength = 0;
	int maxStringLength = 0;
	char *buffer = NULL;
	char *localString = NULL;

	SearchEngineDataTypes dataType = SEARCH_ENGINE_DATA_TYPE_ERROR;

	// determine if this index allows this type of indexing

	if((indexType->settings.isOverrideIndexing) &&
			(!indexType->settings.isIndexFullString)) {
		return 0;
	}

	length = stringLength;
	allocatedLength = stringLength;
	localString = strndup(string, length);

	// determine the data type

	if((dataType = searchEngineCommon_determineDataType(localString,
					length)) == SEARCH_ENGINE_DATA_TYPE_ERROR) {
		log_logf(engine->log, LOG_LEVEL_DEBUG,
				"update-full-string { #%i->'%s' } unknown data type",
				uid, indexType->key);
		free(localString);
		return SEARCH_ENGINE_ERROR_COMMON_FAILED_TO_DETERMINE_DATA_TYPE;
	}

	// perform normalization

	if((dataType == SEARCH_ENGINE_DATA_TYPE_STRING) ||
			(dataType == SEARCH_ENGINE_DATA_TYPE_BINARY)) {
		if(searchEngineCommon_normalizeString(localString, length,
					&resultLength) < 0) {
			log_logf(engine->log, LOG_LEVEL_DEBUG,
					"update-full-string { #%i->'%s' } normalization failed",
					uid, indexType->key);
			free(localString);
			return SEARCH_ENGINE_ERROR_COMMON_FAILED_TO_NORMALIZE_STRING;
		}

		length = resultLength;
	}

	// determine string length thresholds

	if(indexType->settings.isOverrideStringLengths) {
		minStringLength = indexType->settings.minStringLength;
		maxStringLength = indexType->settings.maxStringLength;
	}
	else {
		minStringLength = engine->settings.minStringLength;
		maxStringLength = engine->settings.maxStringLength;
	}

	// pad a short string

	if(length < minStringLength) {
		if(indexType->type == INDEX_REGISTRY_TYPE_WILDCARD) {
			log_logf(engine->log, LOG_LEVEL_DEBUG,
					"update-full-string { #%i->'%s' } unable to pad "
					"string '%s' for wildcard index",
					uid, indexType->key, localString);
			free(localString);
			return 0;
		}

		if(allocatedLength < (minStringLength * 2)) {
			allocatedLength = (minStringLength * 2);

			localString = (char *)realloc(localString,
					(sizeof(char) * allocatedLength));
		}

		if(searchEngineCommong_padString(localString,
					length,
					allocatedLength,
					minStringLength,
					&resultLength) < 0) {
			log_logf(engine->log, LOG_LEVEL_DEBUG,
					"update-full-string { #%i->'%s' } pad string failed",
					uid, indexType->key);
			free(localString);
			return SEARCH_ENGINE_ERROR_COMMON_FAILED_TO_PAD_STRING;
		}

		length = resultLength;
	}

	if(length < minStringLength) {
		log_logf(engine->log, LOG_LEVEL_DEBUG,
				"update-full-string { #%i->'%s' } pad string fell short",
				uid, indexType->key);
		free(localString);
		return -1;
	}

	// update the string in the search index for exact & user-key indexes

	buffer = (char *)malloc(sizeof(char) * (maxStringLength + 1));

	if(length > maxStringLength) {
		strncpy(buffer, localString, maxStringLength);
	}
	else {
		strcpy(buffer, localString);
	}

	if(isPut) {
		searchIndex_putString(searchIndex, indexType, buffer, uid);
	}
	else {
		searchIndex_removeString(searchIndex, indexType, buffer, uid);
	}

	free(buffer);

	// cleanup

	free(localString);

	return 0;
}

static int index_updateTokenizedString(SearchEngine *engine, aboolean isPut,
		SearchIndex *searchIndex, IndexRegistryIndex *indexType, char *string,
		int stringLength, int uid)
{
	int ii = 0;
	int length = 0;
	int tokenCount = 0;
	int resultLength = 0;
	int minStringLength = 0;
	int maxStringLength = 0;
	int delimitersLength = 0;
	int tokenResultLength = 0;
	int *tokenLengths = NULL;
	char *token = NULL;
	char *delimiters = NULL;
	char *localString = NULL;
	char *excludedWord = NULL;
	char **tokenList = NULL;

	// determine if this index allows this type of indexing

	if((indexType->settings.isOverrideIndexing) &&
			(!indexType->settings.isIndexTokenizedString)) {
		return 0;
	}

	// check to see if the string has a vowel or a alphabetic character

	if((!cstring_containsVowel(string, stringLength)) ||
			(!cstring_containsAlphabetic(string, stringLength))) {
		log_logf(engine->log, LOG_LEVEL_DEBUG,
				"update-tokenized-string { #%i->'%s' } string contains no "
				"alphas or vowels", uid, indexType->key);
		return 0;
	}

	// determine string length thresholds

	if(indexType->settings.isOverrideStringLengths) {
		minStringLength = indexType->settings.minStringLength;
		maxStringLength = indexType->settings.maxStringLength;
	}
	else {
		minStringLength = engine->settings.minStringLength;
		maxStringLength = engine->settings.maxStringLength;
	}

	// determine string delimiters

	if(indexType->settings.isOverrideDelimiters) {
		delimiters = indexType->settings.delimiters;
		delimitersLength = indexType->settings.delimitersLength;
	}
	else {
		delimiters = engine->settings.delimiters;
		delimitersLength = engine->settings.delimitersLength;
	}

	// tokenize string

	if(strtotokens(string,
				stringLength,
				delimiters,
				delimitersLength,
				minStringLength,
				&tokenList,
				&tokenLengths,
				&tokenCount) < 0) {
		log_logf(engine->log, LOG_LEVEL_DEBUG,
				"update-tokenized-string { #%i->'%s' } tokenization failed",
				uid, indexType->key);
		return -1;
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

		log_logf(engine->log, LOG_LEVEL_DEBUG,
				"update-tokenized-string { #%i->'%s' } tokenization "
				"returned no results", uid, indexType->key);

		return -1;
	}

	// iterate token list & update indexes

	for(ii = 0; ii < tokenCount; ii++) {
		if(tokenList[ii] == NULL) {
			continue;
		}

		localString = tokenList[ii];
		length = tokenLengths[ii];
		resultLength = 0;

		if(length < minStringLength) {
			free(localString);
			continue;
		}

		if(searchEngineCommon_normalizeString(localString, length,
					&resultLength) < 0) {
			free(localString);
			continue;
		}

		length = resultLength;

		if(length < minStringLength) {
			free(localString);
			continue;
		}

		if((cstring_containsVowel(localString, length)) ||
				(cstring_containsAlphabetic(localString, length))) {
			if(indexType->type == INDEX_REGISTRY_TYPE_WILDCARD) {
				if((token = esa_stemToken(&engine->esa, localString,
								length, &tokenResultLength)) == NULL) {
					free(localString);
					continue;
				}

				if(tokenResultLength > maxStringLength) {
					free(token);
					free(localString);
					continue;
				}

				length = tokenResultLength;
			}
			else {
				token = strndup(localString, length);
			}

			if(indexType->settings.isOverrideExcludedWords) {
				if(bptree_get(indexType->settings.excludedWords, token,
							length, ((void *)&excludedWord)) == 0) {
					free(token);
					free(localString);
					continue;
				}
			}
			else {
				if(bptree_get(&engine->excludedWords, token, length,
							((void *)&excludedWord)) == 0) {
					free(token);
					free(localString);
					continue;
				}
			}
		}
		else {
			token = strndup(localString, length);
		}

		if(isPut) {
			searchIndex_putString(searchIndex, indexType, token, uid);
		}
		else {
			searchIndex_removeString(searchIndex, indexType, token, uid);
		}

		free(token);
		free(tokenList[ii]);
	}

	free(tokenList);
	free(tokenLengths);

	return 0;
}

static int index_updateString(SearchEngine *engine, aboolean isPut,
		SearchIndex *searchIndex, IndexRegistryIndex *indexType,
		ContainerIterator *iterator, int uid)
{
	aboolean isIndexFullString = afalse;
	aboolean isIndexTokenizedString = afalse;
	int stringLength = 0;
	int minStringLength = 0;
	char *string = NULL;

	// determine string length thresholds

	if(indexType->settings.isOverrideStringLengths) {
		minStringLength = indexType->settings.minStringLength;
	}
	else {
		minStringLength = engine->settings.minStringLength;
	}

	// setup local string value

	if(iterator->entity.type == CONTAINER_TYPE_STRING) {
		// copy the string from the iterator entity

		if((stringLength = iterator->entity.valueLength) < minStringLength) {
			log_logf(engine->log, LOG_LEVEL_DEBUG,
					"update-string { #%i->'%s' } length %i invalid",
					uid, indexType->key, stringLength);
			return -1;
		}

		string = strndup(iterator->entity.sValue, stringLength);
	}
	else {
		// perform type conversion using the container API

		if((string = container_getString(iterator->container,
						iterator->entity.name,
						iterator->entity.nameLength,
						&stringLength)) == NULL) {
			log_logf(engine->log, LOG_LEVEL_DEBUG,
					"update-string { #%i->'%s' } does not exist",
					uid, indexType->key);
			return -1;
		}

		if(stringLength < minStringLength) {
			log_logf(engine->log, LOG_LEVEL_DEBUG,
					"update-string { #%i->'%s' } length %i invalid",
					uid, indexType->key, stringLength);
			free(string);
			return -1;
		}
	}

	// determine indexing logic

	if(indexType->settings.isOverrideIndexing) {
		isIndexFullString = indexType->settings.isIndexFullString;
		isIndexTokenizedString = indexType->settings.isIndexTokenizedString;
	}
	else {
		isIndexFullString = atrue;

		if(indexType->type == INDEX_REGISTRY_TYPE_USER_KEY) {
			isIndexTokenizedString = afalse;
		}
		else {
			isIndexTokenizedString = atrue;
		}
	}

	// update the full-length string

	if(isIndexFullString) {
		if(index_updateFullString(engine, isPut, searchIndex, indexType, string,
					stringLength, uid) < 0) {
			free(string);
			return -1;
		}
	}

	// perform wildcard indexing of string tokens

	if(isIndexTokenizedString) {
		if(index_updateTokenizedString(engine, isPut, searchIndex, indexType,
					string, stringLength, uid) < 0) {
			free(string);
			return -1;
		}
	}

	// cleanup

	free(string);

	return 0;
}

static int index_updateInteger(SearchEngine *engine, aboolean isPut,
		SearchIndex *searchIndex, IndexRegistryIndex *indexType,
		ContainerIterator *iterator, int uid)
{
	int iValue = 0;

	// setup local integer value

	if(iterator->entity.type == CONTAINER_TYPE_INTEGER) {
		iValue = iterator->entity.iValue;
	}
	else {
		// perform latitude & longitude decimal conversion

		if(((!strcasecmp(indexType->key, "latitude")) ||
					(!strcasecmp(indexType->key, "longitude"))) &&
				(iterator->entity.type == CONTAINER_TYPE_DOUBLE)) {
			iValue = (iterator->entity.dValue * GEOCOORD_INTEGER_CONVERSION);
		}
		else {
			// perform type conversion using the container API

			iValue = container_getInteger(iterator->container,
					iterator->entity.name, iterator->entity.nameLength);
		}
	}

	// update the range index

	if(isPut) {
		searchIndex_putInteger(searchIndex, indexType, iValue, uid);
	}
	else {
		searchIndex_removeInteger(searchIndex, indexType, iValue, uid);
	}

	return 0;
}

static int index_reindexContainers(SearchEngine *engine,
		IndexRegistryIndex *indexType)
{
	int keyLength = 0;
	int counter = 0;
	int domainKeyLength = 0;
	double timer = 0.0;
	char key[128];
	char *domainKey = NULL;

	Container *container = NULL;
	SearchIndex *searchIndex = NULL;

	// start a timer

	timer = time_getTimeMus();

	// iterate the managed containers index & write them to a file

	log_logf(engine->log, LOG_LEVEL_INFO,
			"reindexing search engine containers for '%s'",
			indexType->key);

	keyLength = 1;
	memset(key, 0, (int)(sizeof(key)));

	while((managedIndex_getNext(&engine->containers, key, keyLength,
					((void *)&container)) == 0) &&
			(container != NULL)) {
		searchEngineCommon_buildUidKey(container_getUid(container), key,
				&keyLength);

		if(container_lock(container) < 0) {
			log_logf(engine->log, LOG_LEVEL_ERROR,
					"failed to obtain container read lock for reindex");
			continue;
		}

		if(!container_exists(container, indexType->key,
					indexType->keyLength)) {
			container_unlock(container);
			continue;
		}

		domainKey = container_getString(container, SEARCHD_DOMAIN_KEY,
				SEARCHD_DOMAIN_KEY_LENGTH, &domainKeyLength);

		if((domainKey == NULL) || (domainKeyLength < 1)) {
			if(domainKey != NULL) {
				free(domainKey);
			}

			log_logf(engine->log, LOG_LEVEL_ERROR,
					"failed to locate '%s' in container #%i",
					SEARCHD_DOMAIN_KEY, container_getUid(container));

			container_unlock(container);

			continue;
		}

		if(spinlock_readLock(&engine->lock) < 0) {
			log_logf(engine->log, LOG_LEVEL_ERROR,
					"failed to obtain spinlock for domain lookup from %s()",
					__FUNCTION__);
		}
		else {
			if((bptree_get(&engine->indexes, domainKey, domainKeyLength,
							((void *)&searchIndex)) == 0) &&
					(searchIndex != NULL)) {
				if(container_reindex(engine, indexType, searchIndex,
							container) < 0) {
					log_logf(engine->log, LOG_LEVEL_ERROR,
							"failed to reindex container #%i for '%s'",
							container_getUid(container), indexType->key);
				}
				else {
					log_logf(engine->log, LOG_LEVEL_DEBUG,
							"reindexed container #%i for '%s' on "
							"domain '%s' ('%s')",
							container_getUid(container), indexType->key,
							searchIndex->key, searchIndex->name);
				}
			}
			else {
				log_logf(engine->log, LOG_LEVEL_ERROR,
						"unable to locate domain '%s' to reindex container #%i",
						domainKey, container_getUid(container));
			}

			spinlock_readUnlock(&engine->lock);
		}

		container_unlock(container);

		free(domainKey);

		counter++;
	}

	log_logf(engine->log, LOG_LEVEL_INFO,
			"reindexed %i search engine containers for '%s' in %0.6f seconds",
			counter, indexType->key, time_getElapsedMusInSeconds(timer));

	return 0;
}

static int index_checkFullString(SearchEngine *engine,
		SearchIndex *searchIndex, IndexRegistryIndex *indexType, char *string,
		int stringLength, int uid)
{
	int rc = 0;
	int length = 0;
	int resultLength = 0;
	int resultSetLength = 0;
	int allocatedLength = 0;
	int minStringLength = 0;
	int maxStringLength = 0;
	int *resultSet = NULL;
	char *buffer = NULL;
	char *localString = NULL;

	SearchEngineDataTypes dataType = SEARCH_ENGINE_DATA_TYPE_ERROR;

	// determine if this index allows this type of indexing

	if((indexType->settings.isOverrideIndexing) &&
			(!indexType->settings.isIndexFullString)) {
		return 0;
	}

	length = stringLength;
	allocatedLength = stringLength;
	localString = strndup(string, length);

	// determine the data type

	if((dataType = searchEngineCommon_determineDataType(localString,
					length)) == SEARCH_ENGINE_DATA_TYPE_ERROR) {
		log_logf(engine->log, LOG_LEVEL_DEBUG,
				"check-full-string { #%i->'%s' } unknown data type",
				uid, indexType->key);
		free(localString);
		return SEARCH_ENGINE_ERROR_COMMON_FAILED_TO_DETERMINE_DATA_TYPE;
	}

	// perform normalization

	if((dataType == SEARCH_ENGINE_DATA_TYPE_STRING) ||
			(dataType == SEARCH_ENGINE_DATA_TYPE_BINARY)) {
		if(searchEngineCommon_normalizeString(localString, length,
					&resultLength) < 0) {
			log_logf(engine->log, LOG_LEVEL_DEBUG,
					"check-full-string { #%i->'%s' } normalization failed",
					uid, indexType->key);
			free(localString);
			return SEARCH_ENGINE_ERROR_COMMON_FAILED_TO_NORMALIZE_STRING;
		}

		length = resultLength;
	}

	// determine string length thresholds

	if(indexType->settings.isOverrideStringLengths) {
		minStringLength = indexType->settings.minStringLength;
		maxStringLength = indexType->settings.maxStringLength;
	}
	else {
		minStringLength = engine->settings.minStringLength;
		maxStringLength = engine->settings.maxStringLength;
	}

	// pad a short string

	if(length < minStringLength) {
		if(indexType->type == INDEX_REGISTRY_TYPE_WILDCARD) {
			log_logf(engine->log, LOG_LEVEL_DEBUG,
					"check-full-string { #%i->'%s' } unable to pad "
					"string '%s' for wildcard index",
					uid, indexType->key, localString);
			free(localString);
			return 0;
		}

		if(allocatedLength < (minStringLength * 2)) {
			allocatedLength = (minStringLength * 2);

			localString = (char *)realloc(localString,
					(sizeof(char) * allocatedLength));
		}

		if(searchEngineCommong_padString(localString,
					length,
					allocatedLength,
					minStringLength,
					&resultLength) < 0) {
			log_logf(engine->log, LOG_LEVEL_DEBUG,
					"check-full-string { #%i->'%s' } pad string failed",
					uid, indexType->key);
			free(localString);
			return SEARCH_ENGINE_ERROR_COMMON_FAILED_TO_PAD_STRING;
		}

		length = resultLength;
	}

	if(length < minStringLength) {
		log_logf(engine->log, LOG_LEVEL_DEBUG,
				"check-full-string { #%i->'%s' } pad string fell short",
				uid, indexType->key);
		free(localString);
		return -1;
	}

	// check the string in the search index for exact & user-key indexes

	buffer = (char *)malloc(sizeof(char) * (maxStringLength + 1));

	if(length > maxStringLength) {
		strncpy(buffer, localString, maxStringLength);
	}
	else {
		strcpy(buffer, localString);
	}

	if((resultSet = searchIndex_getString(searchIndex, indexType, buffer,
					&resultSetLength)) != NULL) {
		free(resultSet);
		rc = 1;
	}

	free(buffer);

	// cleanup

	free(localString);

	return rc;
}

static int index_checkIfStringExists(SearchEngine *engine,
		SearchIndex *searchIndex, IndexRegistryIndex *indexType,
		ContainerIterator *iterator, int uid)
{
	int rc = 0;
	int stringLength = 0;
	int minStringLength = 0;
	char *string = NULL;

	// determine string length thresholds

	if(indexType->settings.isOverrideStringLengths) {
		minStringLength = indexType->settings.minStringLength;
	}
	else {
		minStringLength = engine->settings.minStringLength;
	}

	// setup local string value

	if(iterator->entity.type == CONTAINER_TYPE_STRING) {
		// copy the string from the iterator entity

		if((stringLength = iterator->entity.valueLength) < minStringLength) {
			log_logf(engine->log, LOG_LEVEL_DEBUG,
					"check-string { #%i->'%s' } length %i invalid",
					uid, indexType->key, stringLength);
			return -1;
		}

		string = strndup(iterator->entity.sValue, stringLength);
	}
	else {
		// perform type conversion using the container API

		if((string = container_getString(iterator->container,
						iterator->entity.name,
						iterator->entity.nameLength,
						&stringLength)) == NULL) {
			log_logf(engine->log, LOG_LEVEL_DEBUG,
					"check-string { #%i->'%s' } does not exist",
					uid, indexType->key);
			return -1;
		}

		if(stringLength < minStringLength) {
			log_logf(engine->log, LOG_LEVEL_DEBUG,
					"check-string { #%i->'%s' } length %i invalid",
					uid, indexType->key, stringLength);
			free(string);
			return -1;
		}
	}

	// check the full-length string

	rc = index_checkFullString(engine, searchIndex, indexType,
			string, stringLength, uid);

	// cleanup

	free(string);

	return rc;
}

// container functions

static int container_reindex(SearchEngine *engine,
		IndexRegistryIndex *indexType, SearchIndex *searchIndex,
		Container *container)
{
	int iValue = 0;
	int stringLength = 0;
	int minStringLength = 0;
	char *string = NULL;

	if((indexType->type == INDEX_REGISTRY_TYPE_EXACT) ||
			(indexType->type == INDEX_REGISTRY_TYPE_WILDCARD) ||
			(indexType->type == INDEX_REGISTRY_TYPE_USER_KEY)) {
		// determine string length thresholds

		if(indexType->settings.isOverrideStringLengths) {
			minStringLength = indexType->settings.minStringLength;
		}
		else {
			minStringLength = engine->settings.minStringLength;
		}

		// perform type conversion using the container API

		if((string = container_getString(container, indexType->key,
						indexType->keyLength, &stringLength)) == NULL) {
			log_logf(engine->log, LOG_LEVEL_DEBUG,
					"update-string { #%i->'%s' } does not exist",
					container_getUid(container), indexType->key);
			return -1;
		}

		if(stringLength < minStringLength) {
			log_logf(engine->log, LOG_LEVEL_DEBUG,
					"update-string { #%i->'%s' } length %i invalid",
					container_getUid(container), indexType->key, stringLength);
			free(string);
			return -1;
		}

		// update the full-length string

		if(index_updateFullString(engine, atrue, searchIndex, indexType, string,
					stringLength, container_getUid(container)) < 0) {
			free(string);
			return -1;
		}

		// if a user-key index, update is complete

		if(indexType->type == INDEX_REGISTRY_TYPE_USER_KEY) {
			free(string);
			return 0;
		}

		// perform wildcard indexing of string tokens

		if(index_updateTokenizedString(engine, atrue, searchIndex, indexType,
					string, stringLength, container_getUid(container)) < 0) {
			free(string);
			return -1;
		}

		// cleanup

		free(string);
	}
	else if(indexType->type == INDEX_REGISTRY_TYPE_RANGE) {
		// perform latitude & longitude decimal conversion

		if(((!strcasecmp(indexType->key, "latitude")) ||
					(!strcasecmp(indexType->key, "longitude"))) &&
				(container_getType(container, indexType->key,
								   indexType->keyLength) ==
				 CONTAINER_TYPE_DOUBLE)) {
			iValue = (container_getDouble(container, indexType->key,
						indexType->keyLength) *
					GEOCOORD_INTEGER_CONVERSION);
		}
		else {
			// perform type conversion using the container API

			iValue = container_getInteger(container, indexType->key,
					indexType->keyLength);
		}

		// update the range index

		searchIndex_putInteger(searchIndex, indexType, iValue,
				container_getUid(container));
	}

	return 0;
}

static int container_checkUserKeyIndexes(SearchEngine *engine,
		SearchIndex *searchIndex, Container *container)
{
	int rc = 0;
	int result = 0;
	int indexNameLength = 0;
	int indexNameResultLength = 0;
	char *indexName = NULL;

	ContainerIterator iterator;
	IndexRegistryIndex *indexType = NULL;

	if(containerIterator_init(container, &iterator) < 0) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"failed to initialize iterator on container #%i",
				container_getUid(container));
		return -1;
	}

	while((rc = containerIterator_next(&iterator)) == CONTAINER_ITERATOR_OK) {
		if(iterator.entity.nameLength < 1) {
			continue;
		}

		indexName = strndup(iterator.entity.name, iterator.entity.nameLength);
		indexNameLength = iterator.entity.nameLength;

		if((searchEngineCommon_buildIndexString(indexName, indexNameLength,
						&indexNameResultLength) < 0) ||
				(indexNameResultLength < 1)) {
			free(indexName);
			continue;
		}

		if((indexType = indexRegistry_get(&engine->registry,
						indexName)) == NULL) {
			free(indexName);
			continue;
		}

		if(indexType->type == INDEX_REGISTRY_TYPE_USER_KEY) {
			if(index_checkIfStringExists(engine, searchIndex, indexType,
						&iterator, container_getUid(container)) == 1) {
				result = 1;
			}
		}

		free(indexName);
	}

	if(rc == CONTAINER_ITERATOR_ERROR) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"encountered error while iterating container #%i",
				container_getUid(container));
		return -1;
	}

	if(containerIterator_free(&iterator) < 0) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"failed to free iterator on container #%i",
				container_getUid(container));
		return -1;
	}

	return result;
}

static int container_updateIndexes(SearchEngine *engine, aboolean isPut,
		SearchIndex *searchIndex, Container *container)
{
	int rc = 0;
	int indexNameLength = 0;
	int indexNameResultLength = 0;
	char *indexName = NULL;

	ContainerIterator iterator;
	IndexRegistryIndex *indexType = NULL;

	if(containerIterator_init(container, &iterator) < 0) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"failed to initialize iterator on container #%i",
				container_getUid(container));
		return -1;
	}

	while((rc = containerIterator_next(&iterator)) == CONTAINER_ITERATOR_OK) {
		if(iterator.entity.nameLength < 1) {
			continue;
		}

		indexName = strndup(iterator.entity.name, iterator.entity.nameLength);
		indexNameLength = iterator.entity.nameLength;

		if((searchEngineCommon_buildIndexString(indexName, indexNameLength,
						&indexNameResultLength) < 0) ||
				(indexNameResultLength < 1)) {
			free(indexName);
			continue;
		}

		if((indexType = indexRegistry_get(&engine->registry,
						indexName)) == NULL) {
			free(indexName);
			continue;
		}

		if((indexType->type == INDEX_REGISTRY_TYPE_EXACT) ||
				(indexType->type == INDEX_REGISTRY_TYPE_WILDCARD) ||
				(indexType->type == INDEX_REGISTRY_TYPE_USER_KEY)) {
			if(index_updateString(engine, isPut, searchIndex, indexType,
						&iterator, container_getUid(container)) < 0) {
				log_logf(engine->log, LOG_LEVEL_DEBUG,
						"container update-index failed on '%s' type '%s' "
						"for '%s'",
						searchIndex->key,
						indexRegistry_typeToString(indexType->type),
						indexName);
			}
		}
		else if(indexType->type == INDEX_REGISTRY_TYPE_RANGE) {
			if(index_updateInteger(engine, isPut, searchIndex, indexType,
						&iterator, container_getUid(container)) < 0) {
				log_logf(engine->log, LOG_LEVEL_DEBUG,
						"container update-index failed on '%s' type '%s' "
						"for '%s'",
						searchIndex->key,
						indexRegistry_typeToString(indexType->type),
						indexName);
			}
		}

		free(indexName);
	}

	if(rc == CONTAINER_ITERATOR_ERROR) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"encountered error while iterating container #%i",
				container_getUid(container));
		return -1;
	}

	if(containerIterator_free(&iterator) < 0) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"failed to free iterator on container #%i",
				container_getUid(container));
		return -1;
	}

	return 0;
}

static char *container_buildRelevancyStringArray(SearchEngine *engine,
		IndexRegistryIndex *indexType, ContainerIterator *iterator,
		int *resultStringLength, int *resultArrayLength)
{
	int ii = 0;
	int tokenCount = 0;
	int stringLength = 0;
	int minStringLength = 0;
	int delimitersLength = 0;
	int *tokenLengths = NULL;
	char *result = NULL;
	char *string = NULL;
	char *delimiters = NULL;
	char **tokenList = NULL;

	*resultStringLength = 0;
	*resultArrayLength = 0;

	// determine string length thresholds

	if(indexType->settings.isOverrideStringLengths) {
		minStringLength = indexType->settings.minStringLength;
	}
	else {
		minStringLength = engine->settings.minStringLength;
	}

	// setup local string value

	if(iterator->entity.type == CONTAINER_TYPE_STRING) {
		// copy the string from the iterator entity

		if((stringLength = iterator->entity.valueLength) < minStringLength) {
			log_logf(engine->log, LOG_LEVEL_DEBUG,
					"relevancy-string '%s' from #%i length %i invalid",
					iterator->entity.name,
					container_getUid(iterator->container),
					stringLength);
			return NULL;
		}

		string = strndup(iterator->entity.sValue, stringLength);
	}
	else {
		// perform type conversion using the container API

		if((string = container_getString(iterator->container,
						iterator->entity.name,
						iterator->entity.nameLength,
						&stringLength)) == NULL) {
			log_logf(engine->log, LOG_LEVEL_DEBUG,
					"relevancy-string '%s' from #%i does not exist",
					iterator->entity.name,
					container_getUid(iterator->container));
			return NULL;
		}

		if(stringLength < minStringLength) {
			log_logf(engine->log, LOG_LEVEL_DEBUG,
					"relevancy-string '%s' from #%i length %i invalid",
					iterator->entity.name,
					container_getUid(iterator->container),
					stringLength);
			free(string);
			return NULL;
		}
	}

	// determine string delimiters

	if(indexType->settings.isOverrideDelimiters) {
		delimiters = indexType->settings.delimiters;
		delimitersLength = indexType->settings.delimitersLength;
	}
	else {
		delimiters = engine->settings.delimiters;
		delimitersLength = engine->settings.delimitersLength;
	}

	// tokenize relevancy string value

	tokenList = searchEngineCommon_stringToTokens(
			minStringLength,
			delimiters,
			delimitersLength,
			&engine->esa,
			string,
			stringLength,
			&tokenLengths,
			&tokenCount);

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

		log_logf(engine->log, LOG_LEVEL_DEBUG,
				"failed to tokenize relevancy-string '%s' value '%s'",
				iterator->entity.name,
				string);

		free(string);

		return NULL;
	}

	free(string);

	// serialize the array of tokens

	if((result = serializeArray(tokenList, tokenLengths, tokenCount,
					resultStringLength)) == NULL) {
		log_logf(engine->log, LOG_LEVEL_DEBUG,
				"failed to serialize relevancy-string '%s' array",
				iterator->entity.name);
	}
	else if((*resultStringLength) < 1) {
		log_logf(engine->log, LOG_LEVEL_DEBUG,
				"invalid serialized relevancy-string-array '%s' length %i",
				iterator->entity.name, (*resultStringLength));
		free(result);
		result = NULL;
	}
	else {
		*resultArrayLength = tokenCount;
	}

	// cleanup

	for(ii = 0; ii < tokenCount; ii++) {
		if(tokenList[ii] != NULL) {
			free(tokenList[ii]);
		}
	}

	free(tokenList);
	free(tokenLengths);

	return result;
}

static int container_calculateRelevancy(SearchEngine *engine,
		Container *container)
{
	aboolean calculatedRelevancy = afalse;
	int rc = 0;
	int relevancyArrayLength = 0;
	int relevancyStringLength = 0;
	int indexNameLength = 0;
	int indexNameResultLength = 0;
	double timer = 0.0;
	char *indexName = NULL;
	char *relevancy = NULL;
	char *relevancyKey = NULL;
	char *relevancyLengthKey = NULL;

	ContainerIterator iterator;
	IndexRegistryIndex *indexType = NULL;

	// start a timer

	timer = time_getTimeMus();

	// iterate container and check for relevancy arrays

	if(containerIterator_init(container, &iterator) < 0) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"failed to initialize iterator on container #%i",
				container_getUid(container));
		return -1;
	}

	calculatedRelevancy = afalse;

	while((rc = containerIterator_next(&iterator)) == CONTAINER_ITERATOR_OK) {
		if(iterator.entity.nameLength < 1) {
			continue;
		}

		indexName = strndup(iterator.entity.name, iterator.entity.nameLength);
		indexNameLength = iterator.entity.nameLength;

		if((searchEngineCommon_buildIndexString(indexName, indexNameLength,
						&indexNameResultLength) < 0) ||
				(indexNameResultLength < 1)) {
			free(indexName);
			continue;
		}

		if((indexType = indexRegistry_get(&engine->registry,
						indexName)) == NULL) {
			free(indexName);
			continue;
		}

		free(indexName);

		if((indexType->type != INDEX_REGISTRY_TYPE_EXACT) &&
				(indexType->type != INDEX_REGISTRY_TYPE_WILDCARD)) {
			continue;
		}

		relevancyKey = searchEngineCommon_buildRelevancyKey(indexType->key,
				indexType->keyLength);

		if(container_exists(container, relevancyKey, strlen(relevancyKey))) {
			free(relevancyKey);
			continue;
		}

		if((relevancy = container_buildRelevancyStringArray(engine,
						indexType,
						&iterator,
						&relevancyStringLength,
						&relevancyArrayLength)) != NULL) {
			relevancyLengthKey = searchEngineCommon_buildRelevancyLengthKey(
					indexType->key, indexType->keyLength);

			if(container_unlock(container) < 0) {
				log_logf(engine->log, LOG_LEVEL_ERROR,
						"failed to unlock container for relevancy-put");
				free(relevancy);
				free(relevancyLengthKey);
				free(relevancyKey);
				break;
			}

			if(container_putString(container,
						relevancyKey,
						strlen(relevancyKey),
						relevancy,
						relevancyStringLength) < 0) {
				log_logf(engine->log, LOG_LEVEL_ERROR,
						"failed to put relevancy attribute '%s' in container",
						relevancyKey);
				free(relevancy);
				free(relevancyLengthKey);
				free(relevancyKey);
				break;
			}

			if(container_putInteger(container,
						relevancyLengthKey,
						strlen(relevancyLengthKey),
						relevancyArrayLength) < 0) {
				log_logf(engine->log, LOG_LEVEL_ERROR,
						"failed to put relevancy attribute '%s' in container",
						relevancyLengthKey);
				free(relevancy);
				free(relevancyLengthKey);
				free(relevancyKey);
				break;
			}

			if(container_lock(container) < 0) {
				log_logf(engine->log, LOG_LEVEL_ERROR,
						"failed to lock container after relevancy-put");
				free(relevancy);
				free(relevancyLengthKey);
				free(relevancyKey);
				break;
			}

			free(relevancy);
			free(relevancyLengthKey);

			containerIterator_rewind(&iterator);
		}

		calculatedRelevancy = atrue;

		free(relevancyKey);
	}

	if(rc == CONTAINER_ITERATOR_ERROR) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"encountered error while iterating container #%i",
				container_getUid(container));
		return -1;
	}

	if(containerIterator_free(&iterator) < 0) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"failed to free iterator on container #%i",
				container_getUid(container));
		return -1;
	}

	if(calculatedRelevancy) {
		log_logf(engine->log, LOG_LEVEL_DEBUG,
				"examined container and calculated relevancy in %0.6f seconds",
				time_getElapsedMusInSeconds(timer));
	}

	return 0;
}

// thread functions

char *thread_buildStateFilename(SearchEngine *engine, int type)
{
	int length = 0;
	char *result = NULL;

	mutex_lock(&engine->settings.mutex);

	length = (strlen(engine->settings.statePath) +
			strlen("searchd.state..config") +
			32);

	result = (char *)malloc(sizeof(char) * (length + 1));

	length = snprintf(result, length, "%s/searchd.state.%02i.config",
			engine->settings.statePath,
			type);

	mutex_unlock(&engine->settings.mutex);

	return result;
}

char *thread_buildContainerFilename(SearchEngine *engine, int type)
{
	int length = 0;
	char *result = NULL;

	mutex_lock(&engine->settings.mutex);

	length = (strlen(engine->settings.containerPath) +
			strlen("searchd..containers") +
			32);

	result = (char *)malloc(sizeof(char) * (length + 1));

	length = snprintf(result, length, "%s/searchd.%02i.containers",
			engine->settings.containerPath,
			type);

	mutex_unlock(&engine->settings.mutex);

	return result;
}

static int thread_writeStateToFile(SearchEngine *engine)
{
	aboolean exists = afalse;
	int ii = 0;
	int counter = 0;
	int typeKeyLength = 0;
	char *typeKey = NULL;
	char *filename = NULL;
	char *backupFilename = NULL;
	char section[128];

	Config config;
	IndexRegistryIndex *indexType = NULL;

	// check on state-writing settings

	mutex_lock(&engine->settings.mutex);

	if(engine->settings.statePath == NULL) {
		mutex_unlock(&engine->settings.mutex);

		log_logf(engine->log, LOG_LEVEL_DEBUG,
				"unable to write state - no path specified");

		return -1;
	}

	mutex_unlock(&engine->settings.mutex);

	// backup old state files

	filename = thread_buildStateFilename(engine, 1);
	backupFilename = thread_buildStateFilename(engine, 2);

	if((system_fileExists(filename, &exists) == 0) && (exists)) {
		log_logf(engine->log, LOG_LEVEL_DEBUG,
				"removing search engine indexes state file '%s'",
				backupFilename);

		if((system_fileExists(backupFilename, &exists) == 0) && (exists)) {
			system_fileDelete(backupFilename);
		}

		system_fileMove(filename, backupFilename);
	}

	free(backupFilename);

	backupFilename = filename;
	filename = thread_buildStateFilename(engine, 0);

	if((system_fileExists(filename, &exists) == 0) && (exists)) {
		log_logf(engine->log, LOG_LEVEL_DEBUG,
				"removing search engine indexes state file '%s'",
				backupFilename);

		if((system_fileExists(backupFilename, &exists) == 0) && (exists)) {
			system_fileDelete(backupFilename);
		}

		system_fileMove(filename, backupFilename);
	}

	free(backupFilename);

	// initialize configuration

	config_init(&config, NULL);

	// update configuration with the index registry

	typeKeyLength = 1;
	typeKey = (char *)malloc(sizeof(char) * (typeKeyLength + 1));

	counter = 0;

	indexRegistry_lock(&engine->registry);

	while(bptree_getNext(&engine->registry.index, typeKey, typeKeyLength,
				((void *)&indexType)) == 0) {
		free(typeKey);

		typeKeyLength = strlen(indexType->key);
		typeKey = strndup(indexType->key, typeKeyLength);

		snprintf(section, (int)(sizeof(section) - 1), "indexRegistry.%03i",
				counter);

		indexRegistry_updateConfigContext(&engine->registry, indexType->key,
				&config, section);

		counter++;
	}

	indexRegistry_unlock(&engine->registry);

	if(typeKey != NULL) {
		free(typeKey);
	}

	config_putInteger(&config, "index.registry", "registers", counter);

	// update configuration with the domains

	config_putInteger(&config, "domains", "length", engine->domainLength);

	if(spinlock_readLock(&engine->lock) == 0) {
		for(ii = 0; ii < engine->domainLength; ii++) {
			if(engine->domains[ii] == NULL) {
				continue;
			}

			snprintf(section, (int)(sizeof(section) - 1), "domain.%03i", ii);

			config_putString(&config, section, "key",
					engine->domains[ii]->key);
			config_putString(&config, section, "name",
					engine->domains[ii]->name);
		}

		spinlock_readUnlock(&engine->lock);
	}

	// update configuration with settings

	config_putBigInteger(&config, "search.engine", "maxContainerCount",
			engine->containers.maxCachedItemCount);
	config_putBigInteger(&config, "search.engine", "maxContainerMemoryLength",
			engine->containers.maxCachedMemoryLength);
	config_putDouble(&config, "search.engine", "containerTimeout",
			engine->containers.timeoutSeconds);

	mutex_lock(&engine->settings.mutex);

	config_putInteger(&config, "search.engine", "minStringLength",
			engine->settings.minStringLength);
	config_putInteger(&config, "search.engine", "maxStringLength",
			engine->settings.maxStringLength);
	config_putInteger(&config, "search.engine", "maxSortOperationMemoryLength",
			engine->settings.maxSortOperationMemoryLength);
	config_putString(&config, "search.engine", "stringDelimiters",
			engine->settings.delimiters);

	config_putDouble(&config, "search.engine.backup",
			"stateWriteThresholdSeconds",
			engine->settings.stateWriteThresholdSeconds);
	config_putString(&config, "search.engine.backup",
			"statePath",
			engine->settings.statePath);
	config_putDouble(&config, "search.engine.backup",
			"containerWriteThresholdSeconds",
			engine->settings.containerWriteThresholdSeconds);
	config_putString(&config, "search.engine.backup",
			"containerPath",
			engine->settings.containerPath);

	config_putString(&config, "search.engine.exclusions",
			"excludedWordConfigFilename",
			engine->settings.excludedWordConfigFilename);

	config_putString(&config, "search.engine.authentication",
			"authenticationConfigFilename",
			engine->settings.authenticationConfigFilename);

	config_putString(&config, "search.engine.lang.json",
			"typeCheckLevel",
			searchJson_validationTypeToString(engine->settings.validationType));

	mutex_unlock(&engine->settings.mutex);

	// write state to file

	log_logf(engine->log, LOG_LEVEL_INFO,
			"writing search engine state to file '%s'",
			filename);

	if(config_write(&config, filename) < 0) {
		config_free(&config);
		return -1;
	}

	config_free(&config);

	free(filename);

	return 0;
}

static int thread_readStateFromFile(SearchEngine *engine, char *filename)
{
	int ii = 0;
	int iValue = 0;
	int iValueTwo = 0;
	int registers = 0;
	int domainLength = 0;
	int indexType = 0;
	int indexUid = 0;
	alint lValue = 0;
	double dValue = 0.0;
	char *indexKey = NULL;
	char *indexName = NULL;
	char *domainKey = NULL;
	char *domainName = NULL;
	char *sValue = NULL;
	char section[128];

	Config config;

	if(config_init(&config, filename) < 0) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"failed to initialize state configuration file '%s'",
				filename);
		return -1;
	}

	// read index registry settings from configuration

	config_getInteger(&config, "index.registry", "registers", 0, &registers);

	for(ii = 0; ii < registers; ii++) {
		snprintf(section, (int)(sizeof(section) - 1), "indexRegistry.%03i", ii);
		config_getString(&config, section, "type", "unknown", &sValue);

		if((indexType = indexRegistry_stringToType(sValue)) ==
				INDEX_REGISTRY_TYPE_END_OF_LIST) {
			continue;
		}

		config_getInteger(&config, section, "uid", -1, &indexUid);
		config_getString(&config, section, "key", "unknown", &indexKey);
		config_getString(&config, section, "name", "unknown", &indexName);

		if((indexUid == -1) ||
				(!strcmp(indexKey, "unknown")) ||
				(!strcmp(indexName, "unknown"))) {
			continue;
		}

		if(searchEngine_newIndex(engine, indexType, indexKey, indexName) == 0) {
			indexRegistry_updateSettingsFromConfig(&engine->registry,
					indexKey, &config, section, engine->log, &engine->esa);
		}
	}

	// read domains from configuration

	config_getInteger(&config, "domains", "length", 0, &domainLength);

	for(ii = 0; ii < domainLength; ii++) {
		snprintf(section, (int)(sizeof(section) - 1), "domain.%03i", ii);

		config_getString(&config, section, "key", "unknown", &domainKey);
		config_getString(&config, section, "name", "unknown", &domainName);

		if((!strcmp(domainKey, "unknown")) ||
				(!strcmp(domainName, "unknown"))) {
			continue;
		}

		searchEngine_newDomain(engine, domainKey, domainName);
	}

	// read search engine settings from configuration

	config_getBigInteger(&config, "search.engine", "maxContainerCount",
			SEARCH_ENGINE_DEFAULT_MAX_CONTAINERS, &lValue);

	searchEngine_setMaxContainerCount(engine, lValue);

	config_getBigInteger(&config, "search.engine", "maxContainerMemoryLength",
			SEARCH_ENGINE_DEFAULT_MAX_CONTAINER_MEMORY, &lValue);

	searchEngine_setMaxContainerMemoryLength(engine, lValue);

	config_getDouble(&config, "search.engine", "containerTimeout",
			SEARCH_ENGINE_DEFAULT_CONTAINER_TIMEOUT, &dValue);

	searchEngine_setContainerTimeout(engine, dValue);

	config_getInteger(&config, "search.engine", "minStringLength",
			SEARCH_ENGINE_DEFAULT_MIN_STRING_LENGTH,
			&iValue);

	config_getInteger(&config, "search.engine", "maxStringLength",
			SEARCH_ENGINE_DEFAULT_MAX_STRING_LENGTH,
			&iValueTwo);

	searchEngine_setStringIndexThresholds(engine, iValue, iValueTwo);

	config_getBigInteger(&config, "search.engine",
			"maxSortOperationMemoryLength",
			SEARCH_ENGINE_DEFAULT_MAX_SORT_OPERATION_LENGTH,
			&lValue);

	searchEngine_setMaxSortOperationMemoryLength(engine, lValue);

	config_getString(&config, "search.engine", "stringDelimiters",
			SEARCH_ENGINE_DEFAULT_DELIMITERS,
			&sValue);

	searchEngine_setStringDelimiters(engine, sValue, strlen(sValue));

	config_getDouble(&config, "search.engine.backup",
			"stateWriteThresholdSeconds",
			SEARCH_ENGINE_DEFAULT_STATE_BACKUP_SECONDS, &dValue);

	config_getString(&config, "search.engine.backup", "statePath",
			SEARCH_ENGINE_DEFAULT_STATE_BACKUP_PATH, &sValue);

	searchEngine_setStateBackupAttributes(engine, dValue, sValue);

	config_getDouble(&config, "search.engine.backup",
			"containerWriteThresholdSeconds",
			SEARCH_ENGINE_DEFAULT_DATA_BACKUP_SECONDS, &dValue);

	config_getString(&config, "search.engine.backup", "containerPath",
			SEARCH_ENGINE_DEFAULT_DATA_BACKUP_PATH, &sValue);

	searchEngine_setIndexBackupAttributes(engine, dValue, sValue);

	config_getString(&config, "search.engine.exclusions",
			"excludedWordConfigFilename",
			SEARCH_ENGINE_DEFAULT_EXCLUDED_WORDS_CONFIG,
			&sValue);

	searchEngine_loadExcludedWords(engine, sValue);

	config_getString(&config, "search.engine.authentication",
			"authenticationConfigFilename",
			SEARCH_ENGINE_DEFAULT_AUTHENTICATION_CONFIG,
			&sValue);

	searchEngine_loadAuthentication(engine, sValue);

	config_getString(&config, "search.engine.lang.json",
			"typeCheckLevel",
			SEARCH_ENGINE_DEFAULT_VALIDATION_TYPE_STRING,
			&sValue);

	engine->settings.validationType = searchJson_stringToValidationType(sValue);

	// cleanup

	config_free(&config);

	return 0;
}

static int thread_writeContainersToFile(SearchEngine *engine)
{
	aboolean exists = afalse;
	int hash = 0;
	int keyLength = 0;
	int bufferLength = 0;
	alint bytesWritten = 0;
	char key[128];
	char *buffer = NULL;
	char *filename = NULL;
	char *backupFilename = NULL;

	Container *container = NULL;
	FileHandle fh;

	// check on container-writing settings

	mutex_lock(&engine->settings.mutex);

	if(engine->settings.containerPath == NULL) {
		mutex_unlock(&engine->settings.mutex);

		log_logf(engine->log, LOG_LEVEL_DEBUG,
				"unable to write container - no path specified");

		return -1;
	}

	mutex_unlock(&engine->settings.mutex);

	// backup old container files

	filename = thread_buildContainerFilename(engine, 1);
	backupFilename = thread_buildContainerFilename(engine, 2);

	if((system_fileExists(filename, &exists) == 0) && (exists)) {
		log_logf(engine->log, LOG_LEVEL_DEBUG,
				"removing search engine indexes container file '%s'",
				backupFilename);

		if((system_fileExists(backupFilename, &exists) == 0) && (exists)) {
			system_fileDelete(backupFilename);
		}

		system_fileMove(filename, backupFilename);
	}

	free(backupFilename);

	backupFilename = filename;
	filename = thread_buildContainerFilename(engine, 0);

	if((system_fileExists(filename, &exists) == 0) && (exists)) {
		log_logf(engine->log, LOG_LEVEL_DEBUG,
				"removing search engine indexes container file '%s'",
				backupFilename);

		if((system_fileExists(backupFilename, &exists) == 0) && (exists)) {
			system_fileDelete(backupFilename);
		}

		system_fileMove(filename, backupFilename);
	}

	free(backupFilename);

	// iterate the managed containers index & write them to a file

	log_logf(engine->log, LOG_LEVEL_INFO,
			"writing search engine containers to file '%s'",
			filename);

	if(file_init(&fh, filename, "wb", 0) < 0) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"failed to open container file '%s'", filename);
		free(filename);
		return -1;
	}

	file_writeInt(&fh, SEARCH_ENGINE_CONTAINER_FILE_MAGIC_NUMBER);

	bytesWritten += SIZEOF_INT;

	keyLength = 1;
	memset(key, 0, (int)(sizeof(key)));

	while((managedIndex_getNext(&engine->containers, key, keyLength,
					((void *)&container)) == 0) &&
			(container != NULL)) {
		searchEngineCommon_buildUidKey(container_getUid(container), key,
				&keyLength);

		if(((buffer = container_serialize(container,
							&bufferLength)) == NULL) ||
				(bufferLength < 1)) {
			continue;
		}

		hash = (int)crc32_calculateHash(buffer, bufferLength);

		file_writeInt(&fh, hash);

		bytesWritten += SIZEOF_INT;

		file_writeInt(&fh, bufferLength);

		bytesWritten += SIZEOF_INT;

		file_write(&fh, buffer, bufferLength);

		bytesWritten += bufferLength;

		free(buffer);
	}

	file_writeInt(&fh, SEARCH_ENGINE_CONTAINER_FILE_MAGIC_NUMBER);

	bytesWritten += SIZEOF_INT;

	log_logf(engine->log, LOG_LEVEL_INFO,
			"wrote %lli bytes to search engine container file '%s'",
			bytesWritten, filename);

	// cleanup

	file_free(&fh);

	free(filename);

	return 0;
}

static int thread_readContainersFromFile(SearchEngine *engine, char *filename)
{
	aboolean hasError = afalse;
	int uid = 0;
	int hash = 0;
	int iValue = 0;
	int counter = 0;
	int calculatedHash = 0;
	int bufferRef = 0;
	int bufferLength = 0;
	int loadUidReference = 0;
	char *buffer = NULL;

	FileHandle fh;
	Container *container = NULL;
	SearchEngineQueueEntry *entry = NULL;

	log_logf(engine->log, LOG_LEVEL_INFO,
			"reading search engine containers from file '%s'",
			filename);

	if(file_init(&fh, filename, "rb", 0) < 0) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"failed to open container file '%s'", filename);
		free(filename);
		return -1;
	}

	// read magic number

	if(file_readInt(&fh, &iValue) < 0) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"failed to read prefix magic number from container file '%s'",
				filename);
		file_free(&fh);
		return -1;
	}

	if(iValue != SEARCH_ENGINE_CONTAINER_FILE_MAGIC_NUMBER) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"detected prefix magic number mismatch in container file '%s'",
				filename);
		file_free(&fh);
		return -1;
	}

	// read the containers from the file

	bufferRef = 0;
	bufferLength = 1024;
	buffer = (char *)malloc(sizeof(char) * (bufferLength + 1));

	while(atrue) {
		if(file_readInt(&fh, &iValue) < 0) {
			log_logf(engine->log, LOG_LEVEL_ERROR,
					"failed to read hash from container file '%s'",
					filename);
			hasError = atrue;
			break;
		}

		hash = iValue;

		if(hash == SEARCH_ENGINE_CONTAINER_FILE_MAGIC_NUMBER) { // end of file
			break;
		}

		if(file_readInt(&fh, &iValue) < 0) {
			log_logf(engine->log, LOG_LEVEL_ERROR,
					"failed to read serial length from container file '%s'",
					filename);
			hasError = atrue;
			break;
		}

		bufferRef = iValue;

		if(bufferRef >= bufferLength) {
			do {
				bufferLength *= 2;
			} while(bufferRef >= bufferLength);

			buffer = (char *)realloc(buffer,
					(sizeof(char) * (bufferLength + 1)));
		}

		if(file_read(&fh, buffer, bufferRef) < 0) {
			log_logf(engine->log, LOG_LEVEL_ERROR,
					"failed to read serialized value from container file '%s'",
					filename);
			hasError = atrue;
			break;
		}

		calculatedHash = (int)crc32_calculateHash(buffer, bufferRef);

		if(hash != calculatedHash) {
			log_logf(engine->log, LOG_LEVEL_ERROR,
					"detected hash mismatch in container file '%s'",
					filename);
			hasError = atrue;
			break;
		}

		if((container = container_deserialize(buffer, bufferRef)) == NULL) {
			log_logf(engine->log, LOG_LEVEL_ERROR,
					"failed to deserialize container from container file '%s'",
					filename);
			hasError = atrue;
			break;
		}

/*		{
			int jStringLength = 0;
			char *jString = NULL;
			Json *json = NULL;

			if((json = container_containerToJson(container)) != NULL) {
				if((jString = json_toString(json, &jStringLength)) != NULL) {
					printf("\n%s\n", jString);
					free(jString);
				}
				json_freePtr(json);
			}
		}*/

		if((uid = container_getUid(container)) > loadUidReference) {
			loadUidReference = uid;
		}

		entry = queue_buildEntry(SEARCH_ENGINE_QUEUE_COMMAND_PUT, container);

		mutex_lock(&engine->queueMutex);

		if(fifostack_push(&engine->queue, entry) < 0) {
			free(entry);
		}

		mutex_unlock(&engine->queueMutex);

		counter++;
	}

	// update search engine UID settings, if needed

	mutex_lock(&engine->settings.mutex);

	if(loadUidReference >= engine->settings.uidCounter) {
		engine->settings.uidCounter = (loadUidReference + 1);
	}

	mutex_unlock(&engine->settings.mutex);

	// cleanup

	file_free(&fh);

	free(buffer);

	// return result status

	if(hasError) {
		return -1;
	}

	log_logf(engine->log, LOG_LEVEL_INFO,
			"restored %i search engine containers from container file '%s'",
			counter, filename);

	return 0;
}

static void *thread_searchEngineWorker(void *threadContext, void *argument)
{
	int counter = 0;
	int putCounter = 0;
	int freeCounter = 0;
	int updateCounter = 0;
	alint queueLength = 0;
	double timer = 0.0;
	double elapsedTime = 0.0;
	double lastStateWriteTime = 0.0;
	double lastContainerWriteTime = 0.0;

	TmfThread *thread = NULL;
	SearchEngine *engine = NULL;

	SearchEngineQueueEntry *entry = NULL;

	if((threadContext == NULL) || (argument == NULL)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	thread = (TmfThread *)threadContext;
	engine = (SearchEngine *)argument;

	log_logf(engine->log, LOG_LEVEL_INFO,
			"search engine worker thread #%i (%i) started",
			thread->uid, thread->systemId);

	lastStateWriteTime = time_getTimeMus();
	lastContainerWriteTime = time_getTimeMus();

	while((thread->state == THREAD_STATE_RUNNING) &&
			(thread->signal == THREAD_SIGNAL_OK)) {
		mutex_lock(thread->mutex);
		thread->status = THREAD_STATUS_BUSY;
		mutex_unlock(thread->mutex);

		// determine if the state & containers should be backed-up to disk

		if((engine->settings.stateWriteThresholdSeconds > 0.0) &&
				(time_getElapsedMusInSeconds(lastStateWriteTime) >
				 engine->settings.stateWriteThresholdSeconds)) {
			thread_writeStateToFile(engine);
			lastStateWriteTime = time_getTimeMus();
		}

		if((engine->settings.containerWriteThresholdSeconds > 0.0) &&
				(time_getElapsedMusInSeconds(lastContainerWriteTime) >
				 engine->settings.containerWriteThresholdSeconds)) {
			thread_writeContainersToFile(engine);
			lastContainerWriteTime = time_getTimeMus();
		}

		if((thread->state != THREAD_STATE_RUNNING) ||
				(thread->signal != THREAD_SIGNAL_OK)) {
			break;
		}

		// iterate & execute the command queue

		counter = 0;
		putCounter = 0;
		freeCounter = 0;
		updateCounter = 0;

		timer = time_getTimeMus();

		while((thread->state == THREAD_STATE_RUNNING) &&
				(thread->signal == THREAD_SIGNAL_OK) &&
				(counter < 1024) &&
				(fifostack_pop(&engine->queue, ((void *)&entry)) == 0) &&
				(entry != NULL)) {
			switch(entry->type) {
				case SEARCH_ENGINE_QUEUE_COMMAND_FREE:
					container_freePtr(entry->payload);
					freeCounter++;
					break;

				case SEARCH_ENGINE_QUEUE_COMMAND_PUT:
					putCounter++;
					queue_executeAddition(engine, entry);
					break;

				case SEARCH_ENGINE_QUEUE_COMMAND_UPDATE:
					updateCounter++;
					queue_executeAddition(engine, entry);
					break;

				case SEARCH_ENGINE_QUEUE_COMMAND_ERROR:
				default:
					log_logf(engine->log, LOG_LEVEL_ERROR,
							"unknown queue command %i",
							entry->type);
			}

			free(entry);

			counter++;
		}

		elapsedTime = time_getElapsedMusInSeconds(timer);

		if(counter > 0) {
			log_logf(engine->log, LOG_LEVEL_DEBUG,
					"executed %i commands from the queue "
					"(%i free, %i put, %i update) "
					"in %0.6f seconds (%0.6f avg)",
					counter,
					freeCounter, putCounter, updateCounter,
					elapsedTime, (elapsedTime / (double)counter));
		}

		if((thread->state != THREAD_STATE_RUNNING) ||
				(thread->signal != THREAD_SIGNAL_OK)) {
			break;
		}

		// take a nap

		mutex_lock(thread->mutex);
		thread->status = THREAD_STATUS_NAPPING;
		mutex_unlock(thread->mutex);

		counter = 0;
		queueLength = 0;

		do {
			time_usleep(1024);

			if((counter % 8) == 0) {
				fifostack_getItemCount(&engine->queue, &queueLength);
			}

			counter++;
		} while((counter < 1024) &&
				(queueLength < 1) &&
				(thread->state == THREAD_STATE_RUNNING) &&
				(thread->signal == THREAD_SIGNAL_OK));
	}

	log_logf(engine->log, LOG_LEVEL_INFO,
			"search engine worker thread #%i (%i) shut down",
			thread->uid, thread->systemId);

	return NULL;
}

// define search engine public functions

int searchEngine_init(SearchEngine *engine, Log *log)
{
	if(engine == NULL) {
		DISPLAY_INVALID_ARGS;
		return SEARCH_ENGINE_ERROR_CORE_INVALID_ARGUMENTS;
	}

	memset(engine, 0, (int)(sizeof(SearchEngine)));

	if(indexRegistry_init(&engine->registry) < 0) {
		return -1;
	}

	if(bptree_init(&engine->indexes) < 0) {
		return -1;
	}

	if(bptree_setFreeFunction(&engine->indexes, freeIndex) < 0) {
		return -1;
	}

	if(bptree_init(&engine->excludedWords) < 0) {
		return -1;
	}

	engine->domains = NULL;

	if(managedIndex_init(&engine->containers, atrue) < 0) {
		return -1;
	}

	if(managedIndex_setFreeFunction(&engine->containers, engine,
				freeContainer) < 0) {
		return -1;
	}

	engine->settings.workerThreadId = 0;
	engine->settings.minStringLength = SEARCH_ENGINE_DEFAULT_MIN_STRING_LENGTH;
	engine->settings.maxStringLength = SEARCH_ENGINE_DEFAULT_MAX_STRING_LENGTH;
	engine->settings.maxSortOperationMemoryLength =
		SEARCH_ENGINE_DEFAULT_MAX_SORT_OPERATION_LENGTH;
	engine->settings.uidCounter = 0;
	engine->settings.delimiters = strdup(SEARCH_ENGINE_DEFAULT_DELIMITERS);
	engine->settings.delimitersLength = strlen(engine->settings.delimiters);
	engine->settings.stateWriteThresholdSeconds =
		SEARCH_ENGINE_DEFAULT_STATE_BACKUP_SECONDS;
	engine->settings.statePath = strdup(
			SEARCH_ENGINE_DEFAULT_STATE_BACKUP_PATH);
	engine->settings.containerWriteThresholdSeconds =
		SEARCH_ENGINE_DEFAULT_DATA_BACKUP_SECONDS;
	engine->settings.containerPath = strdup(
			SEARCH_ENGINE_DEFAULT_DATA_BACKUP_PATH);
	engine->settings.excludedWordConfigFilename = strdup(
			SEARCH_ENGINE_DEFAULT_EXCLUDED_WORDS_CONFIG);
	engine->settings.authenticationConfigFilename = strdup(
			SEARCH_ENGINE_DEFAULT_AUTHENTICATION_CONFIG);
	engine->settings.validationType = SEARCH_JSON_VALIDATION_TYPE_WARNING;

	mutex_init(&engine->settings.mutex);

	if(fifostack_init(&engine->queue) < 0) {
		return -1;
	}

	if(fifostack_setFreeFunction(&engine->queue, queue_freeEntry) < 0) {
		return -1;
	}

	esa_init(&engine->esa);
	esa_setDebugMode(&engine->esa, afalse);

	if(log == NULL) {
		engine->settings.isLogInternallyAllocated = atrue;
		engine->log = (Log *)malloc(sizeof(Log));

		log_init(engine->log, LOG_OUTPUT_STDOUT, NULL, LOG_LEVEL_INFO);
	}
	else {
		engine->settings.isLogInternallyAllocated = afalse;
		engine->log = log;
	}

	if(authSystem_init(&engine->auth, engine->log) < 0) {
		return -1;
	}

	mutex_init(&engine->queueMutex);

	if(spinlock_init(&engine->lock) < 0) {
		return -1;
	}

	if(tmf_init(&engine->tmf) < 0) {
		return -1;
	}

	if(tmf_spawnThread(&engine->tmf,
				thread_searchEngineWorker,
				(void *)engine,
				&engine->settings.workerThreadId) < 0) {
		return -1;
	}

	return SEARCH_ENGINE_ERROR_OK;
}

SearchEngine *searchEngine_new(Log *log)
{
	SearchEngine *result = NULL;

	result = (SearchEngine *)malloc(sizeof(SearchEngine));

	if(searchEngine_init(result, log) < 0) {
		searchEngine_freePtr(result);
		return NULL;
	}

	return result;
}

int searchEngine_free(SearchEngine *engine)
{
	if(engine == NULL) {
		DISPLAY_INVALID_ARGS;
		return SEARCH_ENGINE_ERROR_CORE_INVALID_ARGUMENTS;
	}

	if(tmf_free(&engine->tmf) < 0) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"failed to free the thread-management framework");
		return -1;
	}

	log_logf(engine->log, LOG_LEVEL_DEBUG,
			"search engine thread pool shutdown");

	if(spinlock_readLock(&engine->lock) < 0) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"failed to obtain read lock on engine spinlock");
		return -1;
	}

	log_logf(engine->log, LOG_LEVEL_DEBUG,
			"search engine readlock obtained");

	if(managedIndex_free(&engine->containers) < 0) {
		spinlock_readUnlock(&engine->lock);
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"failed to free the managed-index system");
		return -1;
	}

	log_logf(engine->log, LOG_LEVEL_DEBUG,
			"search engine container managed-index freed");

	if(authSystem_free(&engine->auth) < 0) {
		spinlock_readUnlock(&engine->lock);
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"failed to free the auth system");
		return -1;
	}

	log_logf(engine->log, LOG_LEVEL_DEBUG,
			"search engine authentication freed");

	if(indexRegistry_free(&engine->registry) < 0) {
		spinlock_readUnlock(&engine->lock);
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"failed to free the index-registry");
		return -1;
	}

	log_logf(engine->log, LOG_LEVEL_DEBUG,
			"search engine index registry freed");

	if(bptree_free(&engine->indexes) < 0) {
		spinlock_readUnlock(&engine->lock);
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"failed to free the indexes");
		return -1;
	}

	log_logf(engine->log, LOG_LEVEL_DEBUG,
			"search engine indexes freed");

	if(bptree_free(&engine->excludedWords) < 0) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"failed to free the excluded word index");
		return -1;
	}

	log_logf(engine->log, LOG_LEVEL_DEBUG,
			"search engine excluded words freed");

	if(engine->domains != NULL) {
		free(engine->domains);
	}

	spinlock_readUnlock(&engine->lock);

	log_logf(engine->log, LOG_LEVEL_DEBUG,
			"search engine readlock released");

	if(spinlock_writeLock(&engine->lock) < 0) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"failed to obtain a write-lock on the search engine");
		return -1;
	}

	log_logf(engine->log, LOG_LEVEL_DEBUG,
			"search engine writelock obtained");

	mutex_lock(&engine->settings.mutex);

	if(engine->settings.delimiters != NULL) {
		free(engine->settings.delimiters);
	}

	if(engine->settings.statePath != NULL) {
		free(engine->settings.statePath);
	}

	if(engine->settings.containerPath != NULL) {
		free(engine->settings.containerPath);
	}

	if(engine->settings.excludedWordConfigFilename != NULL) {
		free(engine->settings.excludedWordConfigFilename);
	}

	if(engine->settings.authenticationConfigFilename != NULL) {
		free(engine->settings.authenticationConfigFilename);
	}

	mutex_unlock(&engine->settings.mutex);
	mutex_free(&engine->settings.mutex);

	mutex_lock(&engine->queueMutex);

	if(fifostack_free(&engine->queue) < 0) {
		spinlock_writeUnlock(&engine->lock);
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"failed to free the FIFO stack command queue");
		return -1;
	}

	mutex_unlock(&engine->queueMutex);
	mutex_free(&engine->queueMutex);

	esa_free(&engine->esa);

	if(engine->settings.isLogInternallyAllocated) {
		log_free(engine->log);
		free(engine->log);
	}

	spinlock_writeUnlock(&engine->lock);

	if(spinlock_free(&engine->lock) < 0) {
		return -1;
	}

	memset(engine, 0, (int)(sizeof(SearchEngine)));

	return SEARCH_ENGINE_ERROR_OK;
}

int searchEngine_freePtr(SearchEngine *engine)
{
	if(engine == NULL) {
		DISPLAY_INVALID_ARGS;
		return SEARCH_ENGINE_ERROR_CORE_INVALID_ARGUMENTS;
	}

	if(searchEngine_free(engine) < 0) {
		return -1;
	}

	free(engine);

	return SEARCH_ENGINE_ERROR_OK;
}

char *searchEngine_errorCodeToString(SearchEngineErrorCodes code)
{
	char *result = NULL;

	switch(code) {
		case SEARCH_ENGINE_ERROR_OK:
			result = "ok";
			break;

		case SEARCH_ENGINE_ERROR_DEFAULT_ERROR:
			result = "default error";
			break;

		case SEARCH_ENGINE_ERROR_CORE_INVALID_ARGUMENTS:
			result = "invalid arguments";
			break;

		case SEARCH_ENGINE_ERROR_CORE_READ_LOCK_FAILURE:
			result = "read lock failure";
			break;

		case SEARCH_ENGINE_ERROR_CORE_READ_UNLOCK_FAILURE:
			result = "read unlock failure";
			break;

		case SEARCH_ENGINE_ERROR_CORE_WRITE_LOCK_FAILURE:
			result = "write lock failure";
			break;

		case SEARCH_ENGINE_ERROR_CORE_WRITE_UNLOCK_FAILURE:
			result = "write unlock failure";
			break;

		case SEARCH_ENGINE_ERROR_CORE_MISSING_UID:
			result = "missing uid";
			break;

		case SEARCH_ENGINE_ERROR_CORE_INVALID_UID:
			result = "invalid uid";
			break;

		case SEARCH_ENGINE_ERROR_CORE_INVALID_DIRECTORY:
			result = "invalid directory";
			break;

		case SEARCH_ENGINE_ERROR_CORE_FAILED_TO_INIT_CONFIG:
			result = "failed to initialize configuration context";
			break;

		case SEARCH_ENGINE_ERROR_CORE_STATE_WRITE_FAILURE:
			result = "state write failure";
			break;

		case SEARCH_ENGINE_ERROR_CORE_STATE_READ_FAILURE:
			result = "state read failure";
			break;

		case SEARCH_ENGINE_ERROR_CORE_DATA_WRITE_FAILURE:
			result = "data write failure";
			break;

		case SEARCH_ENGINE_ERROR_CORE_DATA_READ_FAILURE:
			result = "data read failure";
			break;

		case SEARCH_ENGINE_ERROR_CORE_INVALID_GEO_COORD_DISTANCE_TYPE:
			result = "invalid geographical coordinate distance type";
			break;

		case SEARCH_ENGINE_ERROR_CORE_CONTAINER_DOES_NOT_EXIST:
			result = "container does not exist";
			break;

		case SEARCH_ENGINE_ERROR_CORE_CONTAINER_ALREADY_EXISTS:
			result = "container already exists";
			break;

		case SEARCH_ENGINE_ERROR_CORE_CONTAINER_LOCK_FAILURE:
			result = "container lock failure";
			break;

		case SEARCH_ENGINE_ERROR_CORE_CONTAINER_HAS_RESERVED_KEY:
			result = "container contains a reservered key";
			break;

		case SEARCH_ENGINE_ERROR_CORE_CONTAINER_MISSING_ATTRIBUTE:
			result = "container missing specified attribute";
			break;

		case SEARCH_ENGINE_ERROR_CORE_FAILED_TO_PUT_CONTAINER:
			result = "failed to put container";
			break;

		case SEARCH_ENGINE_ERROR_CORE_FAILED_TO_LOCATE_CONTAINER:
			result = "failed to locate container";
			break;

		case SEARCH_ENGINE_ERROR_CORE_FAILED_TO_DELETE_CONTAINER:
			result = "failed to delete container";
			break;

		case SEARCH_ENGINE_ERROR_CORE_FAILED_TO_UPDATE_CONTAINER_INDEXES:
			result = "failed to update container indexes";
			break;

		case SEARCH_ENGINE_ERROR_CORE_FAILED_TO_COMBINE_CONTAINERS:
			result = "failed to combine containers";
			break;

		case SEARCH_ENGINE_ERROR_CORE_CONTAINER_HAS_DUPLICATE_USER_KEY_INDEX:
			result = "container has duplicate user-key index";
			break;

		case SEARCH_ENGINE_ERROR_COMMON_FAILED_TO_BUILD_INDEX_STRING:
			result = "failed to build index string";
			break;

		case SEARCH_ENGINE_ERROR_COMMON_FAILED_TO_NORMALIZE_STRING:
			result = "failed to normalize string";
			break;

		case SEARCH_ENGINE_ERROR_COMMON_FAILED_TO_BUILD_INDEX_KEY:
			result = "failed to build index key";
			break;

		case SEARCH_ENGINE_ERROR_COMMON_FAILED_TO_BUILD_UID_KEY:
			result = "failed to build uid key";
			break;

		case SEARCH_ENGINE_ERROR_COMMON_FAILED_TO_BUILD_RELEVANCY_KEY:
			result = "failed to build relevancy key";
			break;

		case SEARCH_ENGINE_ERROR_COMMON_FAILED_TO_BUILD_RELEVANCY_LENGTH_KEY:
			result = "failed to build relevancy-length key";
			break;

		case SEARCH_ENGINE_ERROR_COMMON_FAILED_TO_DETERMINE_DATA_TYPE:
			result = "failed to determine data type";
			break;

		case SEARCH_ENGINE_ERROR_COMMON_FAILED_TO_PAD_STRING:
			result = "failed to pad string";
			break;

		case SEARCH_ENGINE_ERROR_COMMON_FAILED_TO_TOKENIZE_STRING:
			result = "failed to tokenize string";
			break;

		case SEARCH_ENGINE_ERROR_INDEX_ALREADY_EXISTS:
			result = "index already exists";
			break;

		case SEARCH_ENGINE_ERROR_INDEX_INIT_FAILURE:
			result = "index initialization failure";
			break;

		case SEARCH_ENGINE_ERROR_INDEX_FAILED_TO_LOCATE:
			result = "failed to locate index";
			break;

		case SEARCH_ENGINE_ERROR_INDEX_FAILED_TO_CREATE:
			result = "failed to create index";
			break;

		case SEARCH_ENGINE_ERROR_INDEX_FAILED_TO_REINDEX:
			result = "failed to reindex on index";
			break;

		case SEARCH_ENGINE_ERROR_INDEX_FAILED_TO_REMOVE:
			result = "failed to remove index";
			break;

		case SEARCH_ENGINE_ERROR_INDEX_FAILED_TO_REMOVE_REGISTRY:
			result = "failed to remove index registry";
			break;

		case SEARCH_ENGINE_ERROR_INDEX_TYPE_NOT_USER_KEY:
			result = "index type is not a user-key";
			break;

		case SEARCH_ENGINE_ERROR_INDEX_TYPE_INVALID:
			result = "invalid index type";
			break;

		case SEARCH_ENGINE_ERROR_INDEX_RESET_FAILURE:
			result = "index registry reset failure";
			break;

		case SEARCH_ENGINE_ERROR_INDEX_RENAME_FAILURE:
			result = "index rename failure";
			break;

		case SEARCH_ENGINE_ERROR_DOMAIN_ALREADY_EXISTS:
			result = "domain already exists";
			break;

		case SEARCH_ENGINE_ERROR_DOMAIN_FAILED_TO_LOCATE:
			result = "failed to locate domain";
			break;

		case SEARCH_ENGINE_ERROR_DOMAIN_FAILED_TO_INIT:
			result = "domain initialization failure";
			break;

		case SEARCH_ENGINE_ERROR_DOMAIN_FAILED_TO_CREATE:
			result = "failed to create domain";
			break;

		case SEARCH_ENGINE_ERROR_DOMAIN_FAILED_TO_CREATE_INDEX:
			result = "failed to create index on domain";
			break;

		case SEARCH_ENGINE_ERROR_DOMAIN_RENAME_FAILURE:
			result = "failed to rename domain";
			break;

		case SEARCH_ENGINE_ERROR_DOMAIN_REMOVE_FAILURE:
			result = "failed to remove domain";
			break;

		case SEARCH_ENGINE_ERROR_UNKNOWN:
		default:
			result = "an unknown error has occured";
	}

	return result;
}

char *searchEngine_geoCoordTypeToString(SearchEngineGeoCoordTypes type)
{
	char *result = NULL;

	switch(type) {
		case SEARCH_ENGINE_GEO_COORD_TYPE_MILES:
			result = "Miles";
			break;

		case SEARCH_ENGINE_GEO_COORD_TYPE_YARDS:
			result = "Yards";
			break;

		case SEARCH_ENGINE_GEO_COORD_TYPE_FEET:
			result = "Feet";
			break;

		case SEARCH_ENGINE_GEO_COORD_TYPE_KILOMETERS:
			result = "Kilometers";
			break;

		case SEARCH_ENGINE_GEO_COORD_TYPE_METERS:
			result = "Meters";
			break;

		case SEARCH_ENGINE_GEO_COORD_TYPE_UNKNOWN:
		default:
			result = "Unknown";
	}

	return result;
}

aboolean searchEngine_isLocked(SearchEngine *engine)
{
	if(engine == NULL) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	return spinlock_isLocked(&engine->lock);
}

aboolean searchEngine_isReadLocked(SearchEngine *engine)
{
	int threadCount = 0;

	if(engine == NULL) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	if(spinlock_getSpinThreadCount(&engine->lock, &threadCount) < 0) {
		return afalse;
	}

	if(threadCount > 0) {
		return atrue;
	}

	return afalse;
}

int searchEngine_lock(SearchEngine *engine)
{
	if(engine == NULL) {
		DISPLAY_INVALID_ARGS;
		return SEARCH_ENGINE_ERROR_CORE_INVALID_ARGUMENTS;
	}

	return spinlock_readLock(&engine->lock);
}

int searchEngine_unlock(SearchEngine *engine)
{
	if(engine == NULL) {
		DISPLAY_INVALID_ARGS;
		return SEARCH_ENGINE_ERROR_CORE_INVALID_ARGUMENTS;
	}

	return spinlock_readUnlock(&engine->lock);
}

int searchEngine_setMaxContainerCount(SearchEngine *engine,
		alint maxCachedItemCount)
{
	if((engine == NULL) || (maxCachedItemCount < 1)) {
		DISPLAY_INVALID_ARGS;
		return SEARCH_ENGINE_ERROR_CORE_INVALID_ARGUMENTS;
	}

	return managedIndex_setMaxCachedItemCount(&engine->containers,
			maxCachedItemCount);
}

int searchEngine_setMaxContainerMemoryLength(SearchEngine *engine,
		alint maxCachedMemoryLength)
{
	if((engine == NULL) || (maxCachedMemoryLength < 1)) {
		DISPLAY_INVALID_ARGS;
		return SEARCH_ENGINE_ERROR_CORE_INVALID_ARGUMENTS;
	}

	return managedIndex_setMaxCachedMemoryLength(&engine->containers,
			maxCachedMemoryLength);
}

int searchEngine_setContainerTimeout(SearchEngine *engine,
		double timeoutSeconds)
{
	if((engine == NULL) || (timeoutSeconds < 0.0)) {
		DISPLAY_INVALID_ARGS;
		return SEARCH_ENGINE_ERROR_CORE_INVALID_ARGUMENTS;
	}

	return managedIndex_setTimeout(&engine->containers, timeoutSeconds);
}

int searchEngine_setStringIndexThresholds(SearchEngine *engine,
		int minStringLength, int maxStringLength)
{
	if((engine == NULL) ||
			(minStringLength < 1) ||
			(maxStringLength < 1) ||
			(minStringLength > maxStringLength) ||
			(maxStringLength >= SEARCH_ENGINE_CORE_MAX_STRING_INDEX_LENGTH)) {
		DISPLAY_INVALID_ARGS;
		return SEARCH_ENGINE_ERROR_CORE_INVALID_ARGUMENTS;
	}

	mutex_lock(&engine->settings.mutex);

	engine->settings.minStringLength = minStringLength;
	engine->settings.maxStringLength = maxStringLength;

	mutex_unlock(&engine->settings.mutex);

	return SEARCH_ENGINE_ERROR_OK;
}

int searchEngine_setMaxSortOperationMemoryLength(SearchEngine *engine,
		alint maxSortOperationMemoryLength)
{
	if((engine == NULL) || (maxSortOperationMemoryLength < 0)) {
		DISPLAY_INVALID_ARGS;
		return SEARCH_ENGINE_ERROR_CORE_INVALID_ARGUMENTS;
	}

	mutex_lock(&engine->settings.mutex);

	engine->settings.maxSortOperationMemoryLength =
		maxSortOperationMemoryLength;

	mutex_unlock(&engine->settings.mutex);

	return SEARCH_ENGINE_ERROR_OK;
}

int searchEngine_setStringDelimiters(SearchEngine *engine, char *delimiters,
		int delimitersLength)
{
	if((engine == NULL) || (delimiters == NULL) || (delimitersLength < 1)) {
		DISPLAY_INVALID_ARGS;
		return SEARCH_ENGINE_ERROR_CORE_INVALID_ARGUMENTS;
	}

	mutex_lock(&engine->settings.mutex);

	if(engine->settings.delimiters != NULL) {
		free(engine->settings.delimiters);
	}

	engine->settings.delimiters = strndup(delimiters, delimitersLength);
	engine->settings.delimitersLength = delimitersLength;

	mutex_unlock(&engine->settings.mutex);

	return SEARCH_ENGINE_ERROR_OK;
}

int searchEngine_setStateBackupAttributes(SearchEngine *engine,
		double stateWriteThresholdSeconds, char *statePath)
{
	aboolean exists = afalse;

	if((engine == NULL) || (stateWriteThresholdSeconds < 0.0) ||
			(statePath == NULL)) {
		DISPLAY_INVALID_ARGS;
		return SEARCH_ENGINE_ERROR_CORE_INVALID_ARGUMENTS;
	}

	if((system_dirExists(statePath, &exists) < 0) || (!exists)) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"unable to locate directory '%s'", statePath);
		return SEARCH_ENGINE_ERROR_CORE_INVALID_DIRECTORY;
	}

	mutex_lock(&engine->settings.mutex);

	if(engine->settings.statePath != NULL) {
		free(engine->settings.statePath);
	}

	engine->settings.stateWriteThresholdSeconds = stateWriteThresholdSeconds;
	engine->settings.statePath = strdup(statePath);

	mutex_unlock(&engine->settings.mutex);

	return SEARCH_ENGINE_ERROR_OK;
}

int searchEngine_setIndexBackupAttributes(SearchEngine *engine,
		double containerWriteThresholdSeconds, char *containerPath)
{
	aboolean exists = afalse;

	if((engine == NULL) || (containerWriteThresholdSeconds < 0.0) ||
			(containerPath == NULL)) {
		DISPLAY_INVALID_ARGS;
		return SEARCH_ENGINE_ERROR_CORE_INVALID_ARGUMENTS;
	}

	if((system_dirExists(containerPath, &exists) < 0) || (!exists)) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"unable to locate directory '%s'", containerPath);
		return SEARCH_ENGINE_ERROR_CORE_INVALID_DIRECTORY;
	}

	mutex_lock(&engine->settings.mutex);

	if(engine->settings.containerPath != NULL) {
		free(engine->settings.containerPath);
	}

	engine->settings.containerWriteThresholdSeconds =
		containerWriteThresholdSeconds;
	engine->settings.containerPath = strdup(containerPath);

	mutex_unlock(&engine->settings.mutex);

	return SEARCH_ENGINE_ERROR_OK;
}

int searchEngine_setJsonLangValidationType(SearchEngine *engine,
		SearchJsonValidationType validationType)
{
	if(engine == NULL) {
		DISPLAY_INVALID_ARGS;
		return SEARCH_ENGINE_ERROR_CORE_INVALID_ARGUMENTS;
	}

	mutex_lock(&engine->settings.mutex);

	engine->settings.validationType = validationType;

	mutex_unlock(&engine->settings.mutex);

	return SEARCH_ENGINE_ERROR_OK;
}

int searchEngine_loadExcludedWords(SearchEngine *engine, char *filename)
{
	int rc = 0;

	if((engine == NULL) || (filename == NULL)) {
		DISPLAY_INVALID_ARGS;
		return SEARCH_ENGINE_ERROR_CORE_INVALID_ARGUMENTS;
	}

	// lock the engine & load the excluded words from a config file

	if(spinlock_writeLock(&engine->lock) < 0) {
		return SEARCH_ENGINE_ERROR_CORE_WRITE_UNLOCK_FAILURE;
	}

	rc = searchEngineCommon_loadExcludedWords(engine->log, &engine->esa,
			filename, &engine->excludedWords);

	spinlock_writeUnlock(&engine->lock);

	if(rc < 0) {
		return SEARCH_ENGINE_ERROR_CORE_FAILED_TO_INIT_CONFIG;
	}

	// update the engine settings

	mutex_lock(&engine->settings.mutex);

	if(engine->settings.excludedWordConfigFilename != NULL) {
		free(engine->settings.excludedWordConfigFilename);
	}

	engine->settings.excludedWordConfigFilename = strdup(filename);

	mutex_unlock(&engine->settings.mutex);

	return SEARCH_ENGINE_ERROR_OK;
}

int searchEngine_loadAuthentication(SearchEngine *engine, char *filename)
{
	int rc = 0;

	if((engine == NULL) || (filename == NULL)) {
		DISPLAY_INVALID_ARGS;
		return SEARCH_ENGINE_ERROR_CORE_INVALID_ARGUMENTS;
	}

	// lock the engine & load the authentication state from a config file

	if(spinlock_writeLock(&engine->lock) < 0) {
		return SEARCH_ENGINE_ERROR_CORE_WRITE_UNLOCK_FAILURE;
	}

	rc = authSystem_loadConfiguration(&engine->auth, filename);

	spinlock_writeUnlock(&engine->lock);

	if(rc < 0) {
		return SEARCH_ENGINE_ERROR_CORE_FAILED_TO_INIT_CONFIG;
	}

	// update the engine settings

	mutex_lock(&engine->settings.mutex);

	if(engine->settings.authenticationConfigFilename != NULL) {
		free(engine->settings.authenticationConfigFilename);
	}

	engine->settings.authenticationConfigFilename = strdup(filename);

	mutex_unlock(&engine->settings.mutex);

	return SEARCH_ENGINE_ERROR_OK;
}

int searchEngine_saveAuthentication(SearchEngine *engine)
{
	int rc = 0;

	if(engine == NULL) {
		DISPLAY_INVALID_ARGS;
		return SEARCH_ENGINE_ERROR_CORE_INVALID_ARGUMENTS;
	}

	mutex_lock(&engine->settings.mutex);

	rc = authSystem_saveConfiguration(&engine->auth,
			engine->settings.authenticationConfigFilename);

	mutex_unlock(&engine->settings.mutex);

	if(rc < 0) {
		return SEARCH_ENGINE_ERROR_DEFAULT_ERROR;
	}

	return SEARCH_ENGINE_ERROR_OK;
}

int searchEngine_backupState(SearchEngine *engine)
{
	if(engine == NULL) {
		DISPLAY_INVALID_ARGS;
		return SEARCH_ENGINE_ERROR_CORE_INVALID_ARGUMENTS;
	}

	if(spinlock_readLock(&engine->lock) < 0) {
		return SEARCH_ENGINE_ERROR_CORE_READ_LOCK_FAILURE;
	}

	if(thread_writeStateToFile(engine) < 0) {
		spinlock_readUnlock(&engine->lock);
		return SEARCH_ENGINE_ERROR_CORE_STATE_WRITE_FAILURE;
	}

	spinlock_readUnlock(&engine->lock);

	return SEARCH_ENGINE_ERROR_OK;
}

int searchEngine_restoreState(SearchEngine *engine, char *filename)
{
	if((engine == NULL) || (filename == NULL)) {
		DISPLAY_INVALID_ARGS;
		return SEARCH_ENGINE_ERROR_CORE_INVALID_ARGUMENTS;
	}

	if(thread_readStateFromFile(engine, filename) < 0) {
		spinlock_readUnlock(&engine->lock);
		return SEARCH_ENGINE_ERROR_CORE_STATE_READ_FAILURE;
	}

	return SEARCH_ENGINE_ERROR_OK;
}

int searchEngine_backupData(SearchEngine *engine)
{
	if(engine == NULL) {
		DISPLAY_INVALID_ARGS;
		return SEARCH_ENGINE_ERROR_CORE_INVALID_ARGUMENTS;
	}

	if(spinlock_readLock(&engine->lock) < 0) {
		return SEARCH_ENGINE_ERROR_CORE_READ_LOCK_FAILURE;
	}

	if(thread_writeContainersToFile(engine) < 0) {
		spinlock_readUnlock(&engine->lock);
		return SEARCH_ENGINE_ERROR_CORE_DATA_WRITE_FAILURE;
	}

	spinlock_readUnlock(&engine->lock);

	return SEARCH_ENGINE_ERROR_OK;
}

int searchEngine_restoreData(SearchEngine *engine, char *filename)
{
	if((engine == NULL) || (filename == NULL)) {
		DISPLAY_INVALID_ARGS;
		return SEARCH_ENGINE_ERROR_CORE_INVALID_ARGUMENTS;
	}

	if(spinlock_readLock(&engine->lock) < 0) {
		return SEARCH_ENGINE_ERROR_CORE_READ_LOCK_FAILURE;
	}

	if(thread_readContainersFromFile(engine, filename) < 0) {
		spinlock_readUnlock(&engine->lock);
		return SEARCH_ENGINE_ERROR_CORE_DATA_READ_FAILURE;
	}

	spinlock_readUnlock(&engine->lock);

	return SEARCH_ENGINE_ERROR_OK;
}

int searchEngine_newIndex(SearchEngine *engine, IndexRegistryTypes type,
		char *key, char *name)
{
	int ii = 0;
	int keyLength = 0;
	int keyResultLength = 0;
	char *localKey = NULL;

	IndexRegistryIndex *indexType = NULL;

	if((engine == NULL) || (key == NULL) || ((keyLength = strlen(key)) < 1) ||
			(name == NULL)) {
		DISPLAY_INVALID_ARGS;
		return SEARCH_ENGINE_ERROR_CORE_INVALID_ARGUMENTS;
	}

	localKey = strndup(key, keyLength);

	if(searchEngineCommon_buildIndexString(localKey, keyLength,
				&keyResultLength) < 0) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"normalized index registry key '%s' is invalid",
				localKey);
		free(localKey);
		return SEARCH_ENGINE_ERROR_COMMON_FAILED_TO_BUILD_INDEX_STRING;
	}

	if(spinlock_readLock(&engine->lock) < 0) {
		return SEARCH_ENGINE_ERROR_CORE_READ_LOCK_FAILURE;
	}

	if(indexRegistry_put(&engine->registry, type, localKey, name) < 0) {
		spinlock_readUnlock(&engine->lock);

		log_logf(engine->log, LOG_LEVEL_ERROR,
				"failed to register new index "
				"{ type '%s', key '%s', name '%s' } - already exists",
				indexRegistry_typeToString(type), localKey, name);

		free(localKey);
		return SEARCH_ENGINE_ERROR_INDEX_ALREADY_EXISTS;
	}

	if((indexType = indexRegistry_get(&engine->registry, localKey)) == NULL) {
		spinlock_readUnlock(&engine->lock);

		log_logf(engine->log, LOG_LEVEL_ERROR,
				"failed to locate registry for new index "
				"{ type '%s', key '%s', name '%s' }",
				indexRegistry_typeToString(type), localKey, name);

		free(localKey);

		return SEARCH_ENGINE_ERROR_INDEX_FAILED_TO_LOCATE;
	}

	spinlock_readUnlock(&engine->lock);

	if(spinlock_writeLock(&engine->lock) < 0) {
		free(localKey);
		return SEARCH_ENGINE_ERROR_CORE_WRITE_LOCK_FAILURE;
	}

	for(ii = 0; ii < engine->domainLength; ii++) {
		if(engine->domains[ii] == NULL) {
			continue;
		}

		if(searchIndex_newIndex(engine->domains[ii], indexType) < 0) {
			spinlock_writeUnlock(&engine->lock);

			log_logf(engine->log, LOG_LEVEL_ERROR,
					"failed to create new index "
					"{ type '%s', key '%s', name '%s' } "
					"in domain #%03i '%s' ('%s')",
					indexRegistry_typeToString(type), localKey, name,
					ii, engine->domains[ii]->key, engine->domains[ii]->name);

			free(localKey);
			return SEARCH_ENGINE_ERROR_INDEX_FAILED_TO_CREATE;
		}
		else {
			log_logf(engine->log, LOG_LEVEL_INFO,
					"created new index { type '%s', key '%s', name '%s' } "
					"for domain #%03i '%s' ('%s')",
					indexRegistry_typeToString(type), localKey, name,
					ii, engine->domains[ii]->key, engine->domains[ii]->name);
		}
	}

	spinlock_writeUnlock(&engine->lock);

	free(localKey);

	if(index_reindexContainers(engine, indexType) < 0) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"failed to reindex containers for new index "
				"{ type '%s', key '%s', name '%s' } ",
				indexRegistry_typeToString(indexType->type),
				indexType->key, indexType->name);
		return SEARCH_ENGINE_ERROR_INDEX_FAILED_TO_REINDEX;
	}

	log_logf(engine->log, LOG_LEVEL_INFO,
			"created new index { type '%s', key '%s', name '%s' } ",
			indexRegistry_typeToString(indexType->type),
			indexType->key, indexType->name);

	return SEARCH_ENGINE_ERROR_OK;
}

int searchEngine_newUserIndex(SearchEngine *engine, char *key, char *name)
{
	if((engine == NULL) || (key == NULL) || (name == NULL)) {
		DISPLAY_INVALID_ARGS;
		return SEARCH_ENGINE_ERROR_CORE_INVALID_ARGUMENTS;
	}

	return searchEngine_newIndex(engine,
			INDEX_REGISTRY_TYPE_USER_KEY,
			key,
			name);
}

int searchEngine_removeIndex(SearchEngine *engine, char *key)
{
	int ii = 0;
	int keyLength = 0;
	int keyResultLength = 0;
	char *localKey = NULL;

	IndexRegistryIndex *indexType = NULL;

	if((engine == NULL) || (key == NULL) || ((keyLength = strlen(key)) < 1)) {
		DISPLAY_INVALID_ARGS;
		return SEARCH_ENGINE_ERROR_CORE_INVALID_ARGUMENTS;
	}

	localKey = strndup(key, keyLength);

	if(searchEngineCommon_buildIndexString(localKey, keyLength,
				&keyResultLength) < 0) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"normalized index registry key '%s' is invalid",
				localKey);
		free(localKey);
		return SEARCH_ENGINE_ERROR_COMMON_FAILED_TO_BUILD_INDEX_STRING;
	}

	if(spinlock_writeLock(&engine->lock) < 0) {
		return SEARCH_ENGINE_ERROR_CORE_WRITE_LOCK_FAILURE;
	}

	if((indexType = indexRegistry_get(&engine->registry, localKey)) == NULL) {
		spinlock_writeUnlock(&engine->lock);

		log_logf(engine->log, LOG_LEVEL_ERROR,
				"failed to locate registry for remove index { key '%s' }",
				localKey);

		free(localKey);

		return SEARCH_ENGINE_ERROR_INDEX_FAILED_TO_LOCATE;
	}

	for(ii = 0; ii < engine->domainLength; ii++) {
		if(engine->domains[ii] == NULL) {
			continue;
		}

		if(searchIndex_removeIndex(engine->domains[ii], indexType) < 0) {
			spinlock_writeUnlock(&engine->lock);

			log_logf(engine->log, LOG_LEVEL_ERROR,
					"failed to remove index "
					"{ type '%s', key '%s', name '%s' } "
					"from domain '%s' ('%s')",
					indexRegistry_typeToString(indexType->type),
					indexType->key, indexType->name,
					engine->domains[ii]->key, engine->domains[ii]->name);

			free(localKey);

			return SEARCH_ENGINE_ERROR_INDEX_FAILED_TO_REMOVE;
		}
		else {
			log_logf(engine->log, LOG_LEVEL_INFO,
					"removed index { type '%s', key '%s', name '%s' } "
					"from domain '%s' ('%s')",
					indexRegistry_typeToString(indexType->type),
					indexType->key, indexType->name,
					engine->domains[ii]->key, engine->domains[ii]->name);
		}
	}

	if(indexRegistry_remove(&engine->registry, localKey) < 0) {
		spinlock_writeUnlock(&engine->lock);

		log_logf(engine->log, LOG_LEVEL_ERROR,
				"failed to remove registry for remove index "
				"{ type '%s', key '%s', name '%s' }",
				indexRegistry_typeToString(indexType->type),
				indexType->key, indexType->name);

		free(localKey);

		return SEARCH_ENGINE_ERROR_INDEX_FAILED_TO_REMOVE_REGISTRY;
	}

	spinlock_writeUnlock(&engine->lock);

	log_logf(engine->log, LOG_LEVEL_INFO, "removed index '%s'", key);

	free(localKey);

	return SEARCH_ENGINE_ERROR_OK;
}

int searchEngine_renameIndex(SearchEngine *engine, char *key, char *newKey,
		char *newName)
{
	int ii = 0;
	int keyLength = 0;
	int newKeyLength = 0;
	int keyResultLength = 0;
	int newKeyResultLength = 0;
	char *localKey = NULL;
	char *newLocalKey = NULL;

	IndexRegistryIndex *indexType = NULL;

	if((engine == NULL) ||
			(key == NULL) ||
			((keyLength = strlen(key)) < 1) ||
			(newKey == NULL) ||
			((newKeyLength = strlen(newKey)) < 1) ||
			(newName == NULL)) {
		DISPLAY_INVALID_ARGS;
		return SEARCH_ENGINE_ERROR_CORE_INVALID_ARGUMENTS;
	}

	localKey = strndup(key, keyLength);

	if(searchEngineCommon_buildIndexString(localKey, keyLength,
				&keyResultLength) < 0) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"normalized index registry key (1) '%s' is invalid",
				localKey);
		free(localKey);
		return SEARCH_ENGINE_ERROR_COMMON_FAILED_TO_BUILD_INDEX_STRING;
	}

	newLocalKey = strndup(newKey, newKeyLength);

	if(searchEngineCommon_buildIndexString(newLocalKey, newKeyLength,
				&newKeyResultLength) < 0) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"normalized index registry key (2) '%s' is invalid",
				newLocalKey);
		free(localKey);
		free(newLocalKey);
		return SEARCH_ENGINE_ERROR_COMMON_FAILED_TO_BUILD_INDEX_STRING;
	}

	if(spinlock_writeLock(&engine->lock) < 0) {
		free(localKey);
		free(newLocalKey);
		return SEARCH_ENGINE_ERROR_CORE_WRITE_LOCK_FAILURE;
	}

	if((indexType = indexRegistry_get(&engine->registry,
					newLocalKey)) != NULL) {
		spinlock_writeUnlock(&engine->lock);

		log_logf(engine->log, LOG_LEVEL_ERROR,
				"unable to rename registry '%s' to '%s' - already exists",
				newLocalKey, localKey);

		free(localKey);
		free(newLocalKey);

		return SEARCH_ENGINE_ERROR_INDEX_ALREADY_EXISTS;
	}

	if((indexType = indexRegistry_get(&engine->registry, localKey)) == NULL) {
		spinlock_writeUnlock(&engine->lock);

		log_logf(engine->log, LOG_LEVEL_ERROR,
				"failed to locate registry (1) for rename index { key '%s' }",
				localKey);

		free(localKey);
		free(newLocalKey);

		return SEARCH_ENGINE_ERROR_INDEX_FAILED_TO_LOCATE;
	}

	if(indexRegistry_reset(&engine->registry, localKey, indexType->type,
				newLocalKey, newName) < 0) {
		spinlock_writeUnlock(&engine->lock);

		log_logf(engine->log, LOG_LEVEL_ERROR,
				"failed to rename registry for rename index "
				"{ type '%s', key '%s', name '%s' }",
				indexRegistry_typeToString(indexType->type),
				indexType->key, indexType->name);

		free(localKey);
		free(newLocalKey);

		return SEARCH_ENGINE_ERROR_INDEX_RESET_FAILURE;
	}

	if((indexType = indexRegistry_get(&engine->registry,
					newLocalKey)) == NULL) {
		spinlock_writeUnlock(&engine->lock);

		log_logf(engine->log, LOG_LEVEL_ERROR,
				"failed to locate registry (2) for rename index { key '%s' }",
				newLocalKey);

		free(localKey);
		free(newLocalKey);

		return SEARCH_ENGINE_ERROR_INDEX_FAILED_TO_LOCATE;
	}

	for(ii = 0; ii < engine->domainLength; ii++) {
		if(engine->domains[ii] == NULL) {
			continue;
		}

		if(searchIndex_renameIndex(engine->domains[ii], localKey,
					indexType) < 0) {
			spinlock_writeUnlock(&engine->lock);

			log_logf(engine->log, LOG_LEVEL_ERROR,
					"failed to rename index "
					"{ type '%s', key '%s', name '%s' } "
					"for domain '%s' ('%s')",
					indexRegistry_typeToString(indexType->type),
					indexType->key, indexType->name,
					engine->domains[ii]->key, engine->domains[ii]->name);

			free(localKey);
			free(newLocalKey);

			return SEARCH_ENGINE_ERROR_INDEX_RENAME_FAILURE;
		}
		else {
			log_logf(engine->log, LOG_LEVEL_INFO,
					"renamed index { type '%s', key '%s', name '%s' } "
					"for domain '%s' ('%s')",
					indexRegistry_typeToString(indexType->type),
					indexType->key, indexType->name,
					engine->domains[ii]->key, engine->domains[ii]->name);
		}
	}

	log_logf(engine->log, LOG_LEVEL_INFO,
			"renamed index '%s' to { type '%s', key '%s', name '%s' }",
			localKey, indexRegistry_typeToString(indexType->type),
			indexType->key, indexType->name);

	spinlock_writeUnlock(&engine->lock);

	free(localKey);
	free(newLocalKey);

	return SEARCH_ENGINE_ERROR_OK;
}

int searchEngine_setIndexStringThresholds(SearchEngine *engine, char *key,
		int minStringLength, int maxStringLength)
{
	int ii = 0;
	int keyLength = 0;
	int keyResultLength = 0;
	char *localKey = NULL;

	IndexRegistryIndex *indexType = NULL;

	if((engine == NULL) ||
			(key == NULL) ||
			((keyLength = strlen(key)) < 1) ||
			(minStringLength < 0) ||
			(maxStringLength < 0) ||
			(minStringLength > maxStringLength)) {
		DISPLAY_INVALID_ARGS;
		return SEARCH_ENGINE_ERROR_CORE_INVALID_ARGUMENTS;
	}

	localKey = strndup(key, keyLength);

	if(searchEngineCommon_buildIndexString(localKey, keyLength,
				&keyResultLength) < 0) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"normalized index registry key '%s' is invalid",
				localKey);
		free(localKey);
		return SEARCH_ENGINE_ERROR_COMMON_FAILED_TO_BUILD_INDEX_STRING;
	}

	if(spinlock_writeLock(&engine->lock) < 0) {
		free(localKey);
		return SEARCH_ENGINE_ERROR_CORE_WRITE_LOCK_FAILURE;
	}

	if((indexType = indexRegistry_get(&engine->registry, localKey)) == NULL) {
		spinlock_writeUnlock(&engine->lock);

		log_logf(engine->log, LOG_LEVEL_ERROR,
				"failed to locate registry for set index string thresholds "
				"{ key '%s' }",
				localKey);

		free(localKey);

		return SEARCH_ENGINE_ERROR_INDEX_FAILED_TO_LOCATE;
	}

	if(indexType->type != INDEX_REGISTRY_TYPE_WILDCARD) {
		spinlock_writeUnlock(&engine->lock);

		log_logf(engine->log, LOG_LEVEL_ERROR,
				"detected index registry type is invalid "
				"{ type '%s', key '%s' }",
				indexRegistry_typeToString(indexType->type),
				localKey);

		free(localKey);

		return SEARCH_ENGINE_ERROR_INDEX_TYPE_INVALID;
	}

	if(indexRegistry_setStringIndexThresholds(&engine->registry, localKey,
				minStringLength, maxStringLength) < 0) {
		spinlock_writeUnlock(&engine->lock);

		log_logf(engine->log, LOG_LEVEL_ERROR,
				"failed to set registry for index string threholds "
				"{ type '%s', key '%s', name '%s' }",
				indexRegistry_typeToString(indexType->type),
				indexType->key, indexType->name);

		free(localKey);

		return SEARCH_ENGINE_ERROR_INDEX_RESET_FAILURE;
	}

	for(ii = 0; ii < engine->domainLength; ii++) {
		if(engine->domains[ii] == NULL) {
			continue;
		}

		if(searchIndex_setStringThresholdsindexNew(engine->domains[ii],
					indexType, minStringLength, maxStringLength) < 0) {
			spinlock_writeUnlock(&engine->lock);

			log_logf(engine->log, LOG_LEVEL_ERROR,
					"failed to set index string thresholds "
					"{ type '%s', key '%s', name '%s' } "
					"for domain '%s' ('%s')",
					indexRegistry_typeToString(indexType->type),
					indexType->key, indexType->name,
					engine->domains[ii]->key, engine->domains[ii]->name);

			free(localKey);

			return SEARCH_ENGINE_ERROR_INDEX_RESET_FAILURE;
		}
		else {
			log_logf(engine->log, LOG_LEVEL_INFO,
					"set index { type '%s', key '%s', name '%s' } "
					"string thresholds (min %i, max %i) "
					"for domain '%s' ('%s')",
					indexRegistry_typeToString(indexType->type),
					indexType->key, indexType->name,
					minStringLength, maxStringLength,
					engine->domains[ii]->key, engine->domains[ii]->name);
		}
	}

	log_logf(engine->log, LOG_LEVEL_INFO,
			"set index '%s' string thresholds to (min %i, max %i) "
			"{ type '%s', key '%s', name '%s' }",
			localKey,
			minStringLength,
			maxStringLength,
			indexRegistry_typeToString(indexType->type),
			indexType->key,
			indexType->name);

	spinlock_writeUnlock(&engine->lock);

	free(localKey);

	return SEARCH_ENGINE_ERROR_OK;
}

int searchEngine_resetIndex(SearchEngine *engine, char *key,
		IndexRegistryTypes newType, char *newKey, char *newName)
{
	int rc = 0;

	if((engine == NULL) || (key == NULL) || (newKey == NULL) ||
			(newName == NULL)) {
		DISPLAY_INVALID_ARGS;
		return SEARCH_ENGINE_ERROR_CORE_INVALID_ARGUMENTS;
	}

	if((rc = searchEngine_removeIndex(engine, key)) < 0) {
		return rc;
	}

	if((rc = searchEngine_newIndex(engine, newType, newKey, newName)) < 0) {
		return rc;
	}

	log_logf(engine->log, LOG_LEVEL_INFO,
			"reset index '%s' to { type '%s', key '%s', name '%s' }",
			key, indexRegistry_typeToString(newType), newKey, newName);

	return SEARCH_ENGINE_ERROR_OK;
}

char **searchEngine_listIndexes(SearchEngine *engine, int *indexLength,
		int *errorCode)
{
	int ref = 0;
	int length = 0;
	int typeKeyLength = 0;
	char *typeKey = NULL;
	char **result = NULL;

	IndexRegistryIndex *indexType = NULL;

	if((engine == NULL) || (indexLength == NULL) || (errorCode == NULL)) {
		DISPLAY_INVALID_ARGS;
		if(errorCode != NULL) {
			*errorCode = SEARCH_ENGINE_ERROR_CORE_INVALID_ARGUMENTS;
		}
		return NULL;
	}

	*errorCode = SEARCH_ENGINE_ERROR_OK;

	if(spinlock_readLock(&engine->lock) < 0) {
		*errorCode = SEARCH_ENGINE_ERROR_CORE_READ_LOCK_FAILURE;
		return NULL;
	}

	ref = 0;
	length = 8;
	result = (char **)malloc(sizeof(char *) * length);

	typeKeyLength = 1;
	typeKey = (char *)malloc(sizeof(char) * (typeKeyLength + 1));

	indexRegistry_lock(&engine->registry);

	while(bptree_getNext(&engine->registry.index, typeKey, typeKeyLength,
				((void *)&indexType)) == 0) {
		free(typeKey);

		typeKeyLength = strlen(indexType->key);
		typeKey = strndup(indexType->key, typeKeyLength);

		result[ref] = strdup(indexType->key);
		ref++;

		if(ref >= length) {
			length *= 2;
			result = (char **)realloc(result, (sizeof(char *) * length));
		}
	}

	indexRegistry_unlock(&engine->registry);

	if(typeKey != NULL) {
		free(typeKey);
	}

	if(ref == 0) {
		free(result);
		result = NULL;
	}

	*indexLength = ref;

	spinlock_readUnlock(&engine->lock);

	return result;
}

IndexRegistryIndex *searchEngine_getIndex(SearchEngine *engine, char *indexKey,
		int *errorCode)
{
	int stringLength = 0;
	int stringResultLength = 0;
	char *string = NULL;

	IndexRegistryIndex *result = NULL;

	if((engine == NULL) || (indexKey == NULL) || (errorCode == NULL)) {
		DISPLAY_INVALID_ARGS;
		if(errorCode != NULL) {
			*errorCode = SEARCH_ENGINE_ERROR_CORE_INVALID_ARGUMENTS;
		}
		return NULL;
	}

	*errorCode = SEARCH_ENGINE_ERROR_OK;

	// determine if an index exists for this index key

	stringLength = strlen(indexKey);
	string = strndup(indexKey, stringLength);

	if(searchEngineCommon_buildIndexString(string, stringLength,
				&stringResultLength) < 0) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"normalized index registry key '%s' is invalid",
				indexKey);
		free(string);
		*errorCode = SEARCH_ENGINE_ERROR_COMMON_FAILED_TO_BUILD_INDEX_STRING;
		return NULL;
	}

	if((result = indexRegistry_get(&engine->registry, string)) == NULL) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"no index registry entry exists for '%s'",
				indexKey);
		free(string);
		*errorCode = SEARCH_ENGINE_ERROR_COMMON_FAILED_TO_BUILD_INDEX_STRING;
		return NULL;
	}

	free(string);

	return result;
}

int searchEngine_getIndexType(SearchEngine *engine, char *indexKey)
{
	int stringLength = 0;
	int stringResultLength = 0;
	char *string = NULL;

	IndexRegistryIndex *indexType = NULL;

	if((engine == NULL) || (indexKey == NULL)) {
		DISPLAY_INVALID_ARGS;
		return SEARCH_ENGINE_ERROR_CORE_INVALID_ARGUMENTS;
	}

	// determine if an index exists for this index key

	stringLength = strlen(indexKey);
	string = strndup(indexKey, stringLength);

	if(searchEngineCommon_buildIndexString(string, stringLength,
				&stringResultLength) < 0) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"normalized index registry key '%s' is invalid",
				indexKey);
		free(string);
		return SEARCH_ENGINE_ERROR_COMMON_FAILED_TO_BUILD_INDEX_STRING;
	}

	if((indexType = indexRegistry_get(&engine->registry, string)) == NULL) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"no index registry entry exists for '%s'",
				indexKey);
		free(string);
		return SEARCH_ENGINE_ERROR_INDEX_FAILED_TO_LOCATE;
	}

	free(string);

	return (int)indexType->type;
}

int searchEngine_getIndexCount(SearchEngine *engine, char *domainKey,
		char *indexKey)
{
	int keyLength = 0;
	int stringLength = 0;
	int stringResultLength = 0;
	char *string = NULL;

	SearchIndex *searchIndex = NULL;
	IndexRegistryIndex *indexType = NULL;

	if((engine == NULL) || (domainKey == NULL) ||
			((keyLength = strlen(domainKey)) < 1) || (indexKey == NULL)) {
		DISPLAY_INVALID_ARGS;
		return SEARCH_ENGINE_ERROR_CORE_INVALID_ARGUMENTS;
	}

	// lookup the specified domain

	if(spinlock_readLock(&engine->lock) < 0) {
		return SEARCH_ENGINE_ERROR_CORE_READ_LOCK_FAILURE;
	}

	if((bptree_get(&engine->indexes, domainKey, keyLength,
					((void *)&searchIndex)) < 0) ||
			(searchIndex == NULL)) {
		spinlock_readUnlock(&engine->lock);

		log_logf(engine->log, LOG_LEVEL_ERROR,
				"unable to locate domain '%s' for search execution",
				domainKey);

		return SEARCH_ENGINE_ERROR_DOMAIN_FAILED_TO_LOCATE;
	}

	spinlock_readUnlock(&engine->lock);

	// determine if an index exists for this index key

	stringLength = strlen(indexKey);
	string = strndup(indexKey, stringLength);

	if(searchEngineCommon_buildIndexString(string, stringLength,
				&stringResultLength) < 0) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"normalized index registry key '%s' is invalid",
				indexKey);
		free(string);
		return SEARCH_ENGINE_ERROR_COMMON_FAILED_TO_BUILD_INDEX_STRING;
	}

	if((indexType = indexRegistry_get(&engine->registry, string)) == NULL) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"no index registry entry exists for '%s'",
				indexKey);
		free(string);
		return SEARCH_ENGINE_ERROR_INDEX_FAILED_TO_LOCATE;
	}

	free(string);

	// obtain the index count for this domain & index

	return searchIndex_getIndexCount(searchIndex, indexType);
}

int searchEngine_getIndexDepth(SearchEngine *engine, char *domainKey,
		char *indexKey)
{
	int keyLength = 0;
	int stringLength = 0;
	int stringResultLength = 0;
	char *string = NULL;

	SearchIndex *searchIndex = NULL;
	IndexRegistryIndex *indexType = NULL;

	if((engine == NULL) || (domainKey == NULL) ||
			((keyLength = strlen(domainKey)) < 1) || (indexKey == NULL)) {
		DISPLAY_INVALID_ARGS;
		return SEARCH_ENGINE_ERROR_CORE_INVALID_ARGUMENTS;
	}

	// lookup the specified domain

	if(spinlock_readLock(&engine->lock) < 0) {
		return SEARCH_ENGINE_ERROR_CORE_READ_LOCK_FAILURE;
	}

	if((bptree_get(&engine->indexes, domainKey, keyLength,
					((void *)&searchIndex)) < 0) ||
			(searchIndex == NULL)) {
		spinlock_readUnlock(&engine->lock);

		log_logf(engine->log, LOG_LEVEL_ERROR,
				"unable to locate domain '%s' for search execution",
				domainKey);

		return SEARCH_ENGINE_ERROR_DOMAIN_FAILED_TO_LOCATE;
	}

	spinlock_readUnlock(&engine->lock);

	// determine if an index exists for this index key

	stringLength = strlen(indexKey);
	string = strndup(indexKey, stringLength);

	if(searchEngineCommon_buildIndexString(string, stringLength,
				&stringResultLength) < 0) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"normalized index registry key '%s' is invalid",
				indexKey);
		free(string);
		return SEARCH_ENGINE_ERROR_COMMON_FAILED_TO_BUILD_INDEX_STRING;
	}

	if((indexType = indexRegistry_get(&engine->registry, string)) == NULL) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"no index registry entry exists for '%s'",
				indexKey);
		free(string);
		return SEARCH_ENGINE_ERROR_INDEX_FAILED_TO_LOCATE;
	}

	free(string);

	// obtain the index depth for this domain & index

	return searchIndex_getIndexDepth(searchIndex, indexType);
}

int searchEngine_getIndexUniqueValueCount(SearchEngine *engine,
		char *domainKey, char *indexKey)
{
	int keyLength = 0;
	int stringLength = 0;
	int stringResultLength = 0;
	char *string = NULL;

	SearchIndex *searchIndex = NULL;
	IndexRegistryIndex *indexType = NULL;

	if((engine == NULL) || (domainKey == NULL) ||
			((keyLength = strlen(domainKey)) < 1) || (indexKey == NULL)) {
		DISPLAY_INVALID_ARGS;
		return SEARCH_ENGINE_ERROR_CORE_INVALID_ARGUMENTS;
	}

	// lookup the specified domain

	if(spinlock_readLock(&engine->lock) < 0) {
		return SEARCH_ENGINE_ERROR_CORE_READ_LOCK_FAILURE;
	}

	if((bptree_get(&engine->indexes, domainKey, keyLength,
					((void *)&searchIndex)) < 0) ||
			(searchIndex == NULL)) {
		spinlock_readUnlock(&engine->lock);

		log_logf(engine->log, LOG_LEVEL_ERROR,
				"unable to locate domain '%s' for search execution",
				domainKey);

		return SEARCH_ENGINE_ERROR_DOMAIN_FAILED_TO_LOCATE;
	}

	spinlock_readUnlock(&engine->lock);

	// determine if an index exists for this index key

	stringLength = strlen(indexKey);
	string = strndup(indexKey, stringLength);

	if(searchEngineCommon_buildIndexString(string, stringLength,
				&stringResultLength) < 0) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"normalized index registry key '%s' is invalid",
				indexKey);
		free(string);
		return SEARCH_ENGINE_ERROR_COMMON_FAILED_TO_BUILD_INDEX_STRING;
	}

	if((indexType = indexRegistry_get(&engine->registry, string)) == NULL) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"no index registry entry exists for '%s'",
				indexKey);
		free(string);
		return SEARCH_ENGINE_ERROR_INDEX_FAILED_TO_LOCATE;
	}

	free(string);

	// obtain the index depth for this domain & index

	return searchIndex_getIndexValueCount(searchIndex, indexType);
}

int searchEngine_getIndexUniqueValueSummary(SearchEngine *engine,
		char *domainKey, char *indexKey, SearchIndexSummary *summary)
{
	int keyLength = 0;
	int stringLength = 0;
	int stringResultLength = 0;
	char *string = NULL;

	SearchIndex *searchIndex = NULL;
	IndexRegistryIndex *indexType = NULL;

	if((engine == NULL) || (domainKey == NULL) ||
			((keyLength = strlen(domainKey)) < 1) || (indexKey == NULL) ||
			(summary == NULL)) {
		DISPLAY_INVALID_ARGS;
		return SEARCH_ENGINE_ERROR_CORE_INVALID_ARGUMENTS;
	}

	// lookup the specified domain

	if(spinlock_readLock(&engine->lock) < 0) {
		return SEARCH_ENGINE_ERROR_CORE_READ_LOCK_FAILURE;
	}

	if((bptree_get(&engine->indexes, domainKey, keyLength,
					((void *)&searchIndex)) < 0) ||
			(searchIndex == NULL)) {
		spinlock_readUnlock(&engine->lock);

		log_logf(engine->log, LOG_LEVEL_ERROR,
				"unable to locate domain '%s' for search execution",
				domainKey);

		return SEARCH_ENGINE_ERROR_DOMAIN_FAILED_TO_LOCATE;
	}

	spinlock_readUnlock(&engine->lock);

	// determine if an index exists for this index key

	stringLength = strlen(indexKey);
	string = strndup(indexKey, stringLength);

	if(searchEngineCommon_buildIndexString(string, stringLength,
				&stringResultLength) < 0) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"normalized index registry key '%s' is invalid",
				indexKey);
		free(string);
		return SEARCH_ENGINE_ERROR_COMMON_FAILED_TO_BUILD_INDEX_STRING;
	}

	if((indexType = indexRegistry_get(&engine->registry, string)) == NULL) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"no index registry entry exists for '%s'",
				indexKey);
		free(string);
		return SEARCH_ENGINE_ERROR_INDEX_FAILED_TO_LOCATE;
	}

	free(string);

	// obtain the index depth for this domain & index

	return searchIndex_buildIndexValueSummary(searchIndex, indexType, summary);
}

int searchEngine_newDomain(SearchEngine *engine, char *key, char *name)
{
	int ref = 0;
	int keyLength = 0;
	int typeKeyLength = 0;
	char *typeKey = NULL;

	SearchIndex *searchIndex = NULL;
	IndexRegistryIndex *indexType = NULL;

	if((engine == NULL) || (key == NULL) || ((keyLength = strlen(key)) < 1) ||
			(name == NULL)) {
		DISPLAY_INVALID_ARGS;
		return SEARCH_ENGINE_ERROR_CORE_INVALID_ARGUMENTS;
	}

	if(spinlock_writeLock(&engine->lock) < 0) {
		return SEARCH_ENGINE_ERROR_CORE_WRITE_LOCK_FAILURE;
	}

	if(bptree_get(&engine->indexes, key, keyLength,
				((void *)&searchIndex)) == 0) {
		spinlock_writeUnlock(&engine->lock);

		log_logf(engine->log, LOG_LEVEL_ERROR,
				"unable to create new domain { key '%s', name '%s' } - "
				"already exists",
				key, name);

		return SEARCH_ENGINE_ERROR_DOMAIN_ALREADY_EXISTS;
	}

	mutex_lock(&engine->settings.mutex);

	searchIndex = searchIndex_new(engine->settings.minStringLength,
			engine->settings.maxStringLength,
			key,
			name);

	mutex_unlock(&engine->settings.mutex);

	if(searchIndex == NULL) {
		spinlock_writeUnlock(&engine->lock);

		log_logf(engine->log, LOG_LEVEL_ERROR,
				"unable to create new domain { key '%s', name '%s' } - "
				"search index initialization failed",
				key, name);

		return SEARCH_ENGINE_ERROR_DOMAIN_FAILED_TO_INIT;
	}

	if(bptree_put(&engine->indexes, key, keyLength, (void *)searchIndex) < 0) {
		spinlock_writeUnlock(&engine->lock);

		log_logf(engine->log, LOG_LEVEL_ERROR,
				"unable to create new domain { key '%s', name '%s' } - "
				"failed to add to domain index",
				key, name);

		searchIndex_freePtr(searchIndex);

		return SEARCH_ENGINE_ERROR_DOMAIN_FAILED_TO_CREATE;
	}

	typeKeyLength = 1;
	typeKey = (char *)malloc(sizeof(char) * (typeKeyLength + 1));

	indexRegistry_lock(&engine->registry);

	while(bptree_getNext(&engine->registry.index, typeKey, typeKeyLength,
				((void *)&indexType)) == 0) {
		free(typeKey);

		typeKeyLength = strlen(indexType->key);
		typeKey = strndup(indexType->key, typeKeyLength);

		if(searchIndex_newIndex(searchIndex, indexType) < 0) {
			indexRegistry_unlock(&engine->registry);
			spinlock_writeUnlock(&engine->lock);

			log_logf(engine->log, LOG_LEVEL_ERROR,
					"failed to create new index "
					"{ type '%s', key '%s', name '%s' } "
					"in domain '%s' ('%s')",
					indexRegistry_typeToString(indexType->type),
					indexType->key, indexType->name,
					searchIndex->key, searchIndex->name);

			free(typeKey);
			return SEARCH_ENGINE_ERROR_DOMAIN_FAILED_TO_CREATE_INDEX;
		}
		else {
			log_logf(engine->log, LOG_LEVEL_DEBUG,
					"created new index "
					"{ type '%s', key '%s', name '%s' } "
					"on new domain '%s' ('%s')",
					indexRegistry_typeToString(indexType->type),
					indexType->key, indexType->name,
					searchIndex->key, searchIndex->name);
		}
	}

	indexRegistry_unlock(&engine->registry);

	if(typeKey != NULL) {
		free(typeKey);
	}

	if(engine->domains == NULL) {
		ref = 0;
		engine->domainLength = 1;
		engine->domains = (SearchIndex **)malloc(sizeof(SearchIndex *) *
				engine->domainLength);
	}
	else {
		ref = engine->domainLength;
		engine->domainLength += 1;
		engine->domains = (SearchIndex **)realloc(engine->domains,
				(sizeof(SearchIndex *) * engine->domainLength));
	}

	engine->domains[ref] = searchIndex;

	spinlock_writeUnlock(&engine->lock);

	return SEARCH_ENGINE_ERROR_OK;
}

int searchEngine_renameDomain(SearchEngine *engine, char *domainKey,
		char *newKey, char *newName)
{
	int newKeyLength = 0;
	int domainKeyLength = 0;

	SearchIndex *searchIndex = NULL;

	if((engine == NULL) ||
			(domainKey == NULL) ||
			((domainKeyLength = strlen(domainKey)) < 1) ||
			(newKey == NULL) ||
			((newKeyLength = strlen(newKey)) < 1) ||
			(newName == NULL)) {
		DISPLAY_INVALID_ARGS;
		return SEARCH_ENGINE_ERROR_CORE_INVALID_ARGUMENTS;
	}

	if(spinlock_writeLock(&engine->lock) < 0) {
		return SEARCH_ENGINE_ERROR_CORE_WRITE_LOCK_FAILURE;
	}

	if(bptree_get(&engine->indexes, newKey, newKeyLength,
				((void *)&searchIndex)) == 0) {
		spinlock_writeUnlock(&engine->lock);

		log_logf(engine->log, LOG_LEVEL_ERROR,
				"unable to rename domain '%s' to '%s', '%s' - already exists",
				domainKey, newKey, newKey);

		return SEARCH_ENGINE_ERROR_DOMAIN_ALREADY_EXISTS;
	}

	if((bptree_get(&engine->indexes, domainKey, domainKeyLength,
					((void *)&searchIndex)) < 0) ||
			(searchIndex == NULL)) {
		spinlock_writeUnlock(&engine->lock);

		log_logf(engine->log, LOG_LEVEL_ERROR,
				"unable to locate domain '%s' for rename",
				domainKey);

		return SEARCH_ENGINE_ERROR_DOMAIN_FAILED_TO_LOCATE;
	}

	if(searchIndex_rename(searchIndex, newKey, newName) < 0) {
		spinlock_writeUnlock(&engine->lock);

		log_logf(engine->log, LOG_LEVEL_ERROR,
				"failed to rename domain '%s' to { key '%s', name '%s' }",
				domainKey, newKey, newName);

		return SEARCH_ENGINE_ERROR_DOMAIN_RENAME_FAILURE;
	}

	bptree_setFreeFunction(&engine->indexes, freeNothing);

	if(bptree_remove(&engine->indexes, domainKey, domainKeyLength) < 0) {
		bptree_setFreeFunction(&engine->indexes, freeIndex);

		spinlock_writeUnlock(&engine->lock);

		log_logf(engine->log, LOG_LEVEL_ERROR,
				"failed to remove domain '%s' for rename", domainKey);

		return SEARCH_ENGINE_ERROR_DOMAIN_REMOVE_FAILURE;
	}

	bptree_setFreeFunction(&engine->indexes, freeIndex);

	if(bptree_put(&engine->indexes, newKey, newKeyLength,
				(void *)searchIndex) < 0) {
		spinlock_writeUnlock(&engine->lock);

		log_logf(engine->log, LOG_LEVEL_ERROR,
				"unable to rename domain '%s' to { key '%s', name '%s' } - "
				"failed to add to domain index",
				domainKey, newKey, newName);

		searchIndex_freePtr(searchIndex);

		return SEARCH_ENGINE_ERROR_DOMAIN_FAILED_TO_CREATE;
	}

	spinlock_writeUnlock(&engine->lock);

	return SEARCH_ENGINE_ERROR_OK;
}

int searchEngine_removeDomain(SearchEngine *engine, char *domainKey)
{
	aboolean isFound = afalse;
	int ii = 0;
	int domainKeyLength = 0;

	SearchIndex *searchIndex = NULL;

	if((engine == NULL) ||
			(domainKey == NULL) ||
			((domainKeyLength = strlen(domainKey)) < 1)) {
		DISPLAY_INVALID_ARGS;
		return SEARCH_ENGINE_ERROR_CORE_INVALID_ARGUMENTS;
	}

	if(spinlock_writeLock(&engine->lock) < 0) {
		return SEARCH_ENGINE_ERROR_CORE_WRITE_LOCK_FAILURE;
	}

	if((bptree_get(&engine->indexes, domainKey, domainKeyLength,
					((void *)&searchIndex)) < 0) ||
			(searchIndex == NULL)) {
		spinlock_writeUnlock(&engine->lock);

		log_logf(engine->log, LOG_LEVEL_ERROR,
				"unable to locate domain '%s' for remove",
				domainKey);

		return SEARCH_ENGINE_ERROR_DOMAIN_FAILED_TO_LOCATE;
	}

	for(ii = 0; ii < engine->domainLength; ii++) {
		if(engine->domains[ii] == NULL) {
			continue;
		}

		if(!strcmp(engine->domains[ii]->key, domainKey)) {
			engine->domains[ii] = NULL;
			isFound = atrue;
			break;
		}
	}

	if(!isFound) {
		spinlock_writeUnlock(&engine->lock);

		log_logf(engine->log, LOG_LEVEL_ERROR,
				"failed to locate domain '%s' in array for remove", domainKey);

		return SEARCH_ENGINE_ERROR_DOMAIN_REMOVE_FAILURE;
	}

	if(bptree_remove(&engine->indexes, domainKey, domainKeyLength) < 0) {
		spinlock_writeUnlock(&engine->lock);

		log_logf(engine->log, LOG_LEVEL_ERROR,
				"failed to remove domain '%s' for remove", domainKey);

		return SEARCH_ENGINE_ERROR_DOMAIN_REMOVE_FAILURE;
	}

	spinlock_writeUnlock(&engine->lock);

	return SEARCH_ENGINE_ERROR_OK;
}

char **searchEngine_listDomains(SearchEngine *engine, int *domainLength,
		int *errorCode)
{
	int ii = 0;
	char **result = NULL;

	if((engine == NULL) || (domainLength == NULL) || (errorCode == NULL)) {
		DISPLAY_INVALID_ARGS;
		if(errorCode != NULL) {
			*errorCode = SEARCH_ENGINE_ERROR_CORE_INVALID_ARGUMENTS;
		}
		return NULL;
	}

	*domainLength = 0;
	*errorCode = SEARCH_ENGINE_ERROR_OK;

	if(spinlock_readLock(&engine->lock) < 0) {
		*errorCode = SEARCH_ENGINE_ERROR_CORE_READ_LOCK_FAILURE;
		return NULL;
	}

	result = (char **)malloc(sizeof(char *) * engine->domainLength);

	for(ii = 0; ii < engine->domainLength; ii++) {
		if(engine->domains[ii] == NULL) {
			continue;
		}

		result[ii] = strdup(engine->domains[ii]->key);
	}

	*domainLength = ii;

	spinlock_readUnlock(&engine->lock);

	return result;
}

Container *searchEngine_get(SearchEngine *engine, int uid, int *errorCode)
{
	int keyLength = 0;
	char key[128];

	Container *result = NULL;

	if((engine == NULL) || (uid < 1) || (errorCode == NULL)) {
		DISPLAY_INVALID_ARGS;
		if(errorCode != NULL) {
			*errorCode = SEARCH_ENGINE_ERROR_CORE_INVALID_ARGUMENTS;
		}
		return NULL;
	}

	*errorCode = SEARCH_ENGINE_ERROR_OK;

	if(searchEngineCommon_buildUidKey(uid, key, &keyLength) < 0) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"failed to build index key for uid %i", uid);
		*errorCode = SEARCH_ENGINE_ERROR_COMMON_FAILED_TO_BUILD_UID_KEY;
		return NULL;
	}

	if(spinlock_readLock(&engine->lock) < 0) {
		*errorCode = SEARCH_ENGINE_ERROR_CORE_READ_LOCK_FAILURE;
		return NULL;
	}

	if(managedIndex_get(&engine->containers, key, keyLength,
				((void *)&result)) == 0) {
		container_peg(result);
		container_lock(result);
	}

	spinlock_readUnlock(&engine->lock);

	return result;
}

Container *searchEngine_getByAttribute(SearchEngine *engine, char *attribute,
		char *value, int *errorCode)
{
	int ii = 0;
	int uid = 0;
	int keyLength = 0;
	int stringLength = 0;
	int stringResultLength = 0;
	int resultSetLength = 0;
	int *resultSet = NULL;
	char *string = NULL;
	char key[128];

	Container *result = NULL;
	IndexRegistryIndex *indexType = NULL;

	if((engine == NULL) || (attribute == NULL) || (value == NULL) ||
			(errorCode == NULL)) {
		DISPLAY_INVALID_ARGS;
		if(errorCode != NULL) {
			*errorCode = SEARCH_ENGINE_ERROR_CORE_INVALID_ARGUMENTS;
		}
		return NULL;
	}

	*errorCode = SEARCH_ENGINE_ERROR_OK;

	// determine if an index exists for this attribute

	stringLength = strlen(attribute);
	string = strndup(attribute, stringLength);

	if(searchEngineCommon_buildIndexString(string, stringLength,
				&stringResultLength) < 0) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"normalized index registry key '%s' is invalid",
				attribute);
		free(string);
		*errorCode = SEARCH_ENGINE_ERROR_COMMON_FAILED_TO_BUILD_INDEX_STRING;
		return NULL;
	}

	if((indexType = indexRegistry_get(&engine->registry, string)) == NULL) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"no index registry entry exists for '%s'",
				attribute);
		free(string);
		*errorCode = SEARCH_ENGINE_ERROR_INDEX_FAILED_TO_LOCATE;
		return NULL;
	}

	free(string);

	if(indexType->type != INDEX_REGISTRY_TYPE_USER_KEY) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"unable to execute string-search on %s-index '%s'",
				indexRegistry_typeToString(indexType->type),
				indexType->key);
		*errorCode = SEARCH_ENGINE_ERROR_INDEX_TYPE_NOT_USER_KEY;
		return NULL;
	}

	// execute search across all domains

	if(spinlock_readLock(&engine->lock) < 0) {
		*errorCode = SEARCH_ENGINE_ERROR_CORE_READ_LOCK_FAILURE;
		return NULL;
	}

	for(ii = 0; ii < engine->domainLength; ii++) {
		if(engine->domains[ii] == NULL) {
			continue;
		}

		resultSet = searchIndex_getString(engine->domains[ii], indexType,
				value, &resultSetLength);

		log_logf(engine->log, LOG_LEVEL_DEBUG,
				"obtained %i results from domain '%s' for "
				"get-by-attribute search '%s' on attribute '%s'",
				resultSetLength, engine->domains[ii]->name,
				value, indexType->name);

		if((resultSet == NULL) || (resultSetLength < 1)) {
			if(resultSet != NULL) {
				free(resultSet);
			}
			continue;
		}

		if(resultSetLength > 1) {
			log_logf(engine->log, LOG_LEVEL_WARNING,
					"found multiple results (%i) from domain '%s' for "
					"get-by-attribute '%s' on attribute '%s'",
					resultSetLength, engine->domains[ii]->name,
					value, indexType->name);
		}

		uid = resultSet[0];

		free(resultSet);

		if(searchEngineCommon_buildUidKey(uid, key, &keyLength) < 0) {
			log_logf(engine->log, LOG_LEVEL_ERROR,
					"failed to build index key for uid %i", uid);
			break;
		}

		if(managedIndex_get(&engine->containers, key, keyLength,
					((void *)&result)) == 0) {
			container_peg(result);
			container_lock(result);
		}

		break;
	}

	spinlock_readUnlock(&engine->lock);

	return result;
}

int searchEngine_unlockGet(SearchEngine *engine, Container *container)
{
	int uid = 0;
	int keyLength = 0;
	char key[128];

	Container *localContainer = NULL;

	if((engine == NULL) || (container == NULL)) {
		DISPLAY_INVALID_ARGS;
		return SEARCH_ENGINE_ERROR_CORE_INVALID_ARGUMENTS;
	}

	if((uid = container_getUid(container)) < 1) {
		return SEARCH_ENGINE_ERROR_CORE_MISSING_UID;
	}

	if(searchEngineCommon_buildUidKey(uid, key, &keyLength) < 0) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"failed to build index key for uid %i", uid);
		return SEARCH_ENGINE_ERROR_COMMON_FAILED_TO_BUILD_UID_KEY;
	}

	if(spinlock_readLock(&engine->lock) < 0) {
		return SEARCH_ENGINE_ERROR_CORE_READ_LOCK_FAILURE;
	}

	if(managedIndex_get(&engine->containers, key, keyLength,
				((void *)&localContainer)) == 0) {
		container_unlock(container);
	}
	else {
		spinlock_readUnlock(&engine->lock);
		return SEARCH_ENGINE_ERROR_CORE_CONTAINER_DOES_NOT_EXIST;
	}

	spinlock_readUnlock(&engine->lock);

	return SEARCH_ENGINE_ERROR_OK;
}

int searchEngine_put(SearchEngine *engine, char *domainKey,
		Container *container, aboolean isImmediate)
{
	aboolean hasUid = afalse;
	int uid = 0;
	int keyLength = 0;
	int domainKeyLength = 0;
	char key[128];

	SearchIndex *searchIndex = NULL;
	Container *localContainer = NULL;
	SearchEngineQueueEntry *entry = NULL;

	if((engine == NULL) ||
			(domainKey == NULL) ||
			((domainKeyLength = strlen(domainKey)) < 1) ||
			(container == NULL)) {
		DISPLAY_INVALID_ARGS;
		return SEARCH_ENGINE_ERROR_CORE_INVALID_ARGUMENTS;
	}

	// check the container for reserved entities

	if(container_exists(container, SEARCHD_DOMAIN_KEY,
				SEARCHD_DOMAIN_KEY_LENGTH)) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"unable to use container #%i, contains '%s' entity",
				container_getUid(container),
				SEARCHD_DOMAIN_KEY);
		return SEARCH_ENGINE_ERROR_CORE_CONTAINER_HAS_RESERVED_KEY;
	}

	// determine if domain exists

	if(spinlock_readLock(&engine->lock) < 0) {
		return SEARCH_ENGINE_ERROR_CORE_READ_LOCK_FAILURE;
	}

	if((bptree_get(&engine->indexes, domainKey, domainKeyLength,
					((void *)&searchIndex)) < 0) ||
			(searchIndex == NULL)) {
		spinlock_readUnlock(&engine->lock);

		log_logf(engine->log, LOG_LEVEL_ERROR,
				"unable to locate domain '%s' to put container #%i",
				domainKey, container_getUid(container));

		return SEARCH_ENGINE_ERROR_DOMAIN_FAILED_TO_LOCATE;
	}

	spinlock_readUnlock(&engine->lock);

	// set UID on container, if required

	if((uid = container_getUid(container)) == 0) {
		mutex_lock(&engine->settings.mutex);

		engine->settings.uidCounter += 1;

		container_setUid(container, engine->settings.uidCounter);

		uid = engine->settings.uidCounter;

		mutex_unlock(&engine->settings.mutex);
	}
	else {
		hasUid = atrue;
	}

	// read-lock the container

	if(container_lock(container) < 0) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"failed to obtain container read lock for put");
		return SEARCH_ENGINE_ERROR_CORE_CONTAINER_LOCK_FAILURE;
	}

	// build the container UID key

	if((hasUid) || (isImmediate)) {
		if(searchEngineCommon_buildUidKey(container_getUid(container),
					key, &keyLength) < 0) {
			log_logf(engine->log, LOG_LEVEL_ERROR,
					"failed to build index key for container #%i domain '%s'",
					container_getUid(container), domainKey);
			container_unlock(container);
			return SEARCH_ENGINE_ERROR_COMMON_FAILED_TO_BUILD_UID_KEY;
		}
	}

	// determine if the container cannot be added due to duplicate user-key

	if(container_checkUserKeyIndexes(engine, searchIndex, container) == 1) {
		log_logf(engine->log, LOG_LEVEL_INFO,
				"container contains duplicate user-key index values, "
				"rejecting");
		container_unlock(container);
		return SEARCH_ENGINE_ERROR_CORE_CONTAINER_HAS_DUPLICATE_USER_KEY_INDEX;
	}

	// determine if the container already exists

	if(hasUid) {
		if(managedIndex_get(&engine->containers, key, keyLength,
					((void *)&localContainer)) == 0) {
			if(container_lock(localContainer) < 0) {
				log_logf(engine->log, LOG_LEVEL_WARNING,
						"failed to obtain container read lock for removal");
			}

			// remove the old container from the system (note that this
			// automatically places the container on the queue for deletion,
			// which in turn removes the existing indexes)

			if(managedIndex_remove(&engine->containers, key, keyLength) < 0) {
				log_logf(engine->log, LOG_LEVEL_ERROR,
						"failed to remove container #%i domain '%s' from "
						"master index",
						container_getUid(localContainer), domainKey);
				container_unlock(container);
				container_unlock(localContainer);
				return SEARCH_ENGINE_ERROR_CORE_FAILED_TO_DELETE_CONTAINER;
			}

			container_unlock(localContainer);
		}
	}

	// unlock the container

	container_unlock(container);

	// update the container with reserved entities

	container_putString(container, SEARCHD_DOMAIN_KEY,
			SEARCHD_DOMAIN_KEY_LENGTH, domainKey, domainKeyLength);

	// add the container to the system

	if(isImmediate) {
		container_calculateRelevancy(engine, container);

		if(managedIndex_put(&engine->containers, key, keyLength, container,
					container_calculateMemoryLength(container)) < 0) {
			log_logf(engine->log, LOG_LEVEL_ERROR,
					"failed to put container #%i in master index for "
					"domain '%s'",
					container_getUid(container), domainKey);
			return SEARCH_ENGINE_ERROR_CORE_FAILED_TO_PUT_CONTAINER;
		}

		if(container_updateIndexes(engine, atrue, searchIndex, container) < 0) {
			log_logf(engine->log, LOG_LEVEL_ERROR,
					"failed to put indexes for container #%i domain '%s'",
					container_getUid(container), domainKey);
			return SEARCH_ENGINE_ERROR_CORE_FAILED_TO_UPDATE_CONTAINER_INDEXES;
		}
	}
	else {
		entry = queue_buildEntry(SEARCH_ENGINE_QUEUE_COMMAND_PUT, container);

		mutex_lock(&engine->queueMutex);

		if(fifostack_push(&engine->queue, entry) < 0) {
			free(entry);
		}

		mutex_unlock(&engine->queueMutex);
	}

	return uid;
}

int searchEngine_update(SearchEngine *engine, char *domainKey,
		Container *container, aboolean isImmediate, char flags)
{
	int uid = 0;
	int keyLength = 0;
	int nameLength = 0;
	int domainKeyLength = 0;
	char *errorMessage = NULL;
	char key[128];

	SearchIndex *searchIndex = NULL;
	Container *localContainer = NULL;
	Container *updatedContainer = NULL;
	SearchEngineQueueEntry *entry = NULL;

	if((engine == NULL) ||
			(domainKey == NULL) ||
			((domainKeyLength = strlen(domainKey)) < 1) ||
			(container == NULL)) {
		DISPLAY_INVALID_ARGS;
		return SEARCH_ENGINE_ERROR_CORE_INVALID_ARGUMENTS;
	}

	// check the container for reserved entities

	if(container_exists(container, SEARCHD_DOMAIN_KEY,
				SEARCHD_DOMAIN_KEY_LENGTH)) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"unable to use container #%i, contains '%s' entity",
				container_getUid(container),
				SEARCHD_DOMAIN_KEY);
		return SEARCH_ENGINE_ERROR_CORE_CONTAINER_HAS_RESERVED_KEY;
	}

	// determine if domain exists

	if(spinlock_readLock(&engine->lock) < 0) {
		return SEARCH_ENGINE_ERROR_CORE_READ_LOCK_FAILURE;
	}

	if((bptree_get(&engine->indexes, domainKey, domainKeyLength,
					((void *)&searchIndex)) < 0) ||
			(searchIndex == NULL)) {
		spinlock_readUnlock(&engine->lock);

		log_logf(engine->log, LOG_LEVEL_ERROR,
				"unable to locate domain '%s' to put container #%i",
				domainKey, container_getUid(container));

		return SEARCH_ENGINE_ERROR_DOMAIN_FAILED_TO_LOCATE;
	}

	spinlock_readUnlock(&engine->lock);

	// determine if the container has a UID

	if((uid = container_getUid(container)) == 0) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"unable to update container '%s' without a UID",
				container_getName(container, &nameLength));
		return SEARCH_ENGINE_ERROR_CORE_MISSING_UID;
	}

	// read-lock the container

	if(container_lock(container) < 0) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"failed to obtain container read lock for update");
		return SEARCH_ENGINE_ERROR_CORE_CONTAINER_LOCK_FAILURE;
	}

	// build the container UID key

	if(searchEngineCommon_buildUidKey(container_getUid(container),
				key, &keyLength) < 0) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"failed to build index key for container #%i domain '%s'",
				container_getUid(container), domainKey);
		container_unlock(container);
		return SEARCH_ENGINE_ERROR_COMMON_FAILED_TO_BUILD_UID_KEY;
	}

	// determine if the container already exists

	if(managedIndex_get(&engine->containers, key, keyLength,
				((void *)&localContainer)) < 0) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"failed to locate container #%i",
				container_getUid(container));
		container_unlock(container);
		return SEARCH_ENGINE_ERROR_CORE_FAILED_TO_LOCATE_CONTAINER;
	}

	// combine the containers

	if(container_lock(localContainer) < 0) {
		log_logf(engine->log, LOG_LEVEL_WARNING,
				"failed to obtain container read lock for update");
	}

	if((updatedContainer = container_combine(localContainer,
					container, flags, &errorMessage)) == NULL) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"failed to perform container-combine #%i/#%i from "
				"domain '%s' with '%s'",
				container_getUid(localContainer),
				container_getUid(container),
				domainKey,
				errorMessage);
		container_unlock(container);
		container_unlock(localContainer);
		return SEARCH_ENGINE_ERROR_CORE_FAILED_TO_COMBINE_CONTAINERS;
	}

	// update the container with reserved entities

	container_putString(updatedContainer, SEARCHD_DOMAIN_KEY,
			SEARCHD_DOMAIN_KEY_LENGTH, domainKey, domainKeyLength);

	// read-unlock & free the container

	container_unlock(container);

	container_freePtr(container);

	// remove the old container from the system (note that this
	// automatically places the container on the queue for deletion,
	// which in turn removes the existing indexes)

	if(managedIndex_remove(&engine->containers, key, keyLength) < 0) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"failed to remove container #%i domain '%s' from "
				"master index",
				container_getUid(localContainer), domainKey);
		container_unlock(localContainer);
		return SEARCH_ENGINE_ERROR_CORE_FAILED_TO_DELETE_CONTAINER;
	}

	container_unlock(localContainer);

	// add the container to the system

	if(isImmediate) {
		container_calculateRelevancy(engine, updatedContainer);

		if(managedIndex_put(&engine->containers, key, keyLength,
					updatedContainer,
					container_calculateMemoryLength(updatedContainer)) < 0) {
			log_logf(engine->log, LOG_LEVEL_ERROR,
					"failed to put container #%i in master index for "
					"domain '%s'",
					container_getUid(updatedContainer), domainKey);
			container_freePtr(updatedContainer);
			return SEARCH_ENGINE_ERROR_CORE_FAILED_TO_PUT_CONTAINER;
		}

		if(container_updateIndexes(engine, atrue, searchIndex,
					updatedContainer) < 0) {
			log_logf(engine->log, LOG_LEVEL_ERROR,
					"failed to put indexes for container #%i domain '%s'",
					container_getUid(updatedContainer), domainKey);
			return SEARCH_ENGINE_ERROR_CORE_FAILED_TO_UPDATE_CONTAINER_INDEXES;
		}
	}
	else {
		entry = queue_buildEntry(SEARCH_ENGINE_QUEUE_COMMAND_UPDATE,
				updatedContainer);

		entry->flags = flags;

		mutex_lock(&engine->queueMutex);

		if(fifostack_push(&engine->queue, entry) < 0) {
			free(entry);
		}

		mutex_unlock(&engine->queueMutex);
	}

	return uid;
}

int searchEngine_updateByAttribute(SearchEngine *engine, char *domainKey,
		char *attribute, Container *container, aboolean isImmediate,
		char flags)
{
	int ii = 0;
	int uid = 0;
	int stringLength = 0;
	int attributeLength = 0;
	int resultSetLength = 0;
	int stringResultLength = 0;
	int *resultSet = NULL;
	char *string = NULL;

	IndexRegistryIndex *indexType = NULL;

	if((engine == NULL) ||
			(domainKey == NULL) ||
			(attribute == NULL) ||
			(container == NULL)) {
		DISPLAY_INVALID_ARGS;
		return SEARCH_ENGINE_ERROR_CORE_INVALID_ARGUMENTS;
	}

	// determine if the container has a UID

	if((uid = container_getUid(container)) > 0) {
		return searchEngine_update(engine, domainKey, container, isImmediate,
				flags);
	}

	// determine if an index exists for this attribute

	attributeLength = strlen(attribute);
	stringLength = attributeLength;
	string = strndup(attribute, stringLength);

	if(searchEngineCommon_buildIndexString(string, stringLength,
				&stringResultLength) < 0) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"normalized index registry key '%s' is invalid",
				attribute);
		free(string);
		return SEARCH_ENGINE_ERROR_COMMON_FAILED_TO_BUILD_INDEX_STRING;
	}

	if((indexType = indexRegistry_get(&engine->registry, string)) == NULL) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"no index registry entry exists for '%s'",
				attribute);
		free(string);
		return SEARCH_ENGINE_ERROR_INDEX_FAILED_TO_LOCATE;
	}

	free(string);

	if(indexType->type != INDEX_REGISTRY_TYPE_USER_KEY) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"unable to execute string-search on %s-index '%s'",
				indexRegistry_typeToString(indexType->type),
				indexType->key);
		return SEARCH_ENGINE_ERROR_INDEX_TYPE_NOT_USER_KEY;
	}

	// locate the container's UID by attribute-lookup

	if((string = container_getString(container, attribute, attributeLength,
					&stringLength)) == NULL) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"failed to obtain attribute '%s' value from "
				"container #%i domain '%s'", attribute,
				container_getUid(container), domainKey);
		return SEARCH_ENGINE_ERROR_CORE_CONTAINER_MISSING_ATTRIBUTE;
	}

	if(spinlock_readLock(&engine->lock) < 0) {
		free(string);
		return SEARCH_ENGINE_ERROR_CORE_READ_LOCK_FAILURE;
	}

	for(ii = 0; ii < engine->domainLength; ii++) {
		if(engine->domains[ii] == NULL) {
			continue;
		}

		resultSet = searchIndex_getString(engine->domains[ii], indexType,
				string, &resultSetLength);

		log_logf(engine->log, LOG_LEVEL_DEBUG,
				"obtained %i results from domain '%s' for "
				"get-by-attribute search '%s' on attribute '%s'",
				resultSetLength, engine->domains[ii]->name,
				string, indexType->name);

		if((resultSet == NULL) || (resultSetLength < 1)) {
			if(resultSet != NULL) {
				free(resultSet);
			}
			continue;
		}

		if(resultSetLength > 1) {
			log_logf(engine->log, LOG_LEVEL_WARNING,
					"found multiple results (%i) from domain '%s' for "
					"get-by-attribute '%s' on attribute '%s'",
					resultSetLength, engine->domains[ii]->name,
					string, indexType->name);
		}

		uid = resultSet[0];

		free(resultSet);

		break;
	}

	spinlock_readUnlock(&engine->lock);

	// check for valid UID

	if(uid <= 0) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"failed to locate valid UID for container #%i on "
				"attribute '%s' value '%s'",
				container_getUid(container),
				attribute, string);
		free(string);
		return SEARCH_ENGINE_ERROR_CORE_INVALID_UID;
	}

	free(string);

	// update container's UID

	container_setUid(container, uid);

	// perform update

	return searchEngine_update(engine, domainKey, container, isImmediate,
			flags);
}

int searchEngine_delete(SearchEngine *engine, int uid)
{
	int keyLength = 0;
	char key[128];

	Container *localContainer = NULL;

	if((engine == NULL) || (uid < 1)) {
		DISPLAY_INVALID_ARGS;
		return SEARCH_ENGINE_ERROR_CORE_INVALID_ARGUMENTS;
	}

	// build the container UID key

	if(searchEngineCommon_buildUidKey(uid, key, &keyLength) < 0) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"failed to build index key for container #%i", uid);
		return SEARCH_ENGINE_ERROR_COMMON_FAILED_TO_BUILD_UID_KEY;
	}

	// determine if the container already exists

	if(managedIndex_get(&engine->containers, key, keyLength,
				((void *)&localContainer)) < 0) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"failed to locate container #%i", uid);
		return SEARCH_ENGINE_ERROR_CORE_CONTAINER_DOES_NOT_EXIST;
	}

	// remove the old container from the system (note that this
	// automatically places the container on the queue for deletion,
	// which in turn removes the existing indexes)

	if(container_lock(localContainer) < 0) {
		log_logf(engine->log, LOG_LEVEL_WARNING,
				"failed to obtain container read lock for delete");
	}

	if(managedIndex_remove(&engine->containers, key, keyLength) < 0) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"failed to remove container #%i from master index",
				container_getUid(localContainer));
		container_unlock(localContainer);
		return SEARCH_ENGINE_ERROR_CORE_FAILED_TO_DELETE_CONTAINER;
	}

	container_unlock(localContainer);

	return uid;
}

int searchEngine_deleteByAttribute(SearchEngine *engine, char *attribute,
		char *value)
{
	int ii = 0;
	int uid = 0;
	int keyLength = 0;
	int stringLength = 0;
	int resultSetLength = 0;
	int stringResultLength = 0;
	int *resultSet = NULL;
	char *string = NULL;
	char key[128];

	Container *localContainer = NULL;
	IndexRegistryIndex *indexType = NULL;

	if((engine == NULL) || (attribute == NULL) || (value == NULL)) {
		DISPLAY_INVALID_ARGS;
		return SEARCH_ENGINE_ERROR_CORE_INVALID_ARGUMENTS;
	}

	// determine if an index exists for this attribute

	stringLength = strlen(attribute);
	string = strndup(attribute, stringLength);

	if(searchEngineCommon_buildIndexString(string, stringLength,
				&stringResultLength) < 0) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"normalized index registry key '%s' is invalid",
				attribute);
		free(string);
		return SEARCH_ENGINE_ERROR_COMMON_FAILED_TO_BUILD_INDEX_STRING;
	}

	if((indexType = indexRegistry_get(&engine->registry, string)) == NULL) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"no index registry entry exists for '%s'",
				attribute);
		free(string);
		return SEARCH_ENGINE_ERROR_INDEX_FAILED_TO_LOCATE;
	}

	free(string);

	if(indexType->type != INDEX_REGISTRY_TYPE_USER_KEY) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"unable to execute string-search on %s-index '%s'",
				indexRegistry_typeToString(indexType->type),
				indexType->key);
		return SEARCH_ENGINE_ERROR_INDEX_TYPE_NOT_USER_KEY;
	}

	// search the domains for a container that matches this value

	if(spinlock_readLock(&engine->lock) < 0) {
		return SEARCH_ENGINE_ERROR_CORE_READ_LOCK_FAILURE;
	}

	for(ii = 0; ii < engine->domainLength; ii++) {
		if(engine->domains[ii] == NULL) {
			continue;
		}

		resultSet = searchIndex_getString(engine->domains[ii], indexType,
				value, &resultSetLength);

		log_logf(engine->log, LOG_LEVEL_DEBUG,
				"obtained %i results from domain '%s' for "
				"get-by-attribute search '%s' on attribute '%s'",
				resultSetLength, engine->domains[ii]->name,
				value, indexType->name);

		if((resultSet == NULL) || (resultSetLength < 1)) {
			if(resultSet != NULL) {
				free(resultSet);
			}
			continue;
		}

		if(resultSetLength > 1) {
			log_logf(engine->log, LOG_LEVEL_WARNING,
					"found multiple results (%i) from domain '%s' for "
					"get-by-attribute '%s' on attribute '%s'",
					resultSetLength, engine->domains[ii]->name,
					value, indexType->name);
		}

		uid = resultSet[0];

		free(resultSet);

		break;
	}

	spinlock_readUnlock(&engine->lock);

	// check for valid UID

	if(uid <= 0) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"failed to locate UID for attribute '%s' value '%s'",
				attribute, value);
		return SEARCH_ENGINE_ERROR_CORE_INVALID_UID;
	}

	// build the container UID key

	if(searchEngineCommon_buildUidKey(uid, key, &keyLength) < 0) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"failed to build index key for container #%i", uid);
		return SEARCH_ENGINE_ERROR_COMMON_FAILED_TO_BUILD_UID_KEY;
	}

	// determine if the container already exists

	if(managedIndex_get(&engine->containers, key, keyLength,
				((void *)&localContainer)) < 0) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"failed to locate container #%i", uid);
		return SEARCH_ENGINE_ERROR_CORE_FAILED_TO_LOCATE_CONTAINER;
	}

	// remove the old container from the system (note that this
	// automatically places the container on the queue for deletion,
	// which in turn removes the existing indexes)

	if(container_lock(localContainer) < 0) {
		log_logf(engine->log, LOG_LEVEL_WARNING,
				"failed to obtain container read lock for delete");
	}

	if(managedIndex_remove(&engine->containers, key, keyLength) < 0) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"failed to remove container #%i from master index",
				container_getUid(localContainer));
		container_unlock(localContainer);
		return SEARCH_ENGINE_ERROR_CORE_FAILED_TO_DELETE_CONTAINER;
	}

	container_unlock(localContainer);

	return uid;
}

int searchEngine_searchNotList(SearchEngine *engine, char *domainKey,
		char *indexKey, Intersect *intersect)
{
	int keyLength = 0;
	int stringLength = 0;
	int stringResultLength = 0;
	char *string = NULL;

	SearchIndex *searchIndex = NULL;
	IndexRegistryIndex *indexType = NULL;

	if((engine == NULL) || (domainKey == NULL) ||
			((keyLength = strlen(domainKey)) < 1) || (indexKey == NULL) ||
			(intersect == NULL)) {
		DISPLAY_INVALID_ARGS;
		return SEARCH_ENGINE_ERROR_CORE_INVALID_ARGUMENTS;
	}

	// lookup the specified domain

	if(spinlock_readLock(&engine->lock) < 0) {
		return SEARCH_ENGINE_ERROR_CORE_READ_LOCK_FAILURE;
	}

	if((bptree_get(&engine->indexes, domainKey, keyLength,
					((void *)&searchIndex)) < 0) ||
			(searchIndex == NULL)) {
		spinlock_readUnlock(&engine->lock);

		log_logf(engine->log, LOG_LEVEL_ERROR,
				"unable to locate domain '%s' for search execution",
				domainKey);

		return SEARCH_ENGINE_ERROR_DOMAIN_FAILED_TO_LOCATE;
	}

	spinlock_readUnlock(&engine->lock);

	// determine if an index exists for this index key

	stringLength = strlen(indexKey);
	string = strndup(indexKey, stringLength);

	if(searchEngineCommon_buildIndexString(string, stringLength,
				&stringResultLength) < 0) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"normalized index registry key '%s' is invalid",
				indexKey);
		free(string);
		return SEARCH_ENGINE_ERROR_COMMON_FAILED_TO_BUILD_INDEX_STRING;
	}

	if((indexType = indexRegistry_get(&engine->registry, string)) == NULL) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"no index registry entry exists for '%s'",
				indexKey);
		free(string);
		return SEARCH_ENGINE_ERROR_INDEX_FAILED_TO_LOCATE;
	}

	free(string);

	// obtain all of the UIDs within this index & domain

	return searchIndex_buildAllIndexValueUids(searchIndex, indexType,
			intersect);
}

int searchEngine_searchNotListGlobal(SearchEngine *engine, char *indexKey,
		Intersect *intersect)
{
	int ii = 0;
	int stringLength = 0;
	int stringResultLength = 0;
	char *string = NULL;

	Intersect localIntersect;
	IndexRegistryIndex *indexType = NULL;

	if((engine == NULL) || (indexKey == NULL) || (intersect == NULL)) {
		DISPLAY_INVALID_ARGS;
		return SEARCH_ENGINE_ERROR_CORE_INVALID_ARGUMENTS;
	}

	// determine if an index exists for this index key

	stringLength = strlen(indexKey);
	string = strndup(indexKey, stringLength);

	if(searchEngineCommon_buildIndexString(string, stringLength,
				&stringResultLength) < 0) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"normalized index registry key '%s' is invalid",
				indexKey);
		free(string);
		return SEARCH_ENGINE_ERROR_COMMON_FAILED_TO_BUILD_INDEX_STRING;
	}

	if((indexType = indexRegistry_get(&engine->registry, string)) == NULL) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"no index registry entry exists for '%s'",
				indexKey);
		free(string);
		return SEARCH_ENGINE_ERROR_INDEX_FAILED_TO_LOCATE;
	}

	free(string);

	// iterate through the domains and build a list of all UIDs

	if(spinlock_readLock(&engine->lock) < 0) {
		return SEARCH_ENGINE_ERROR_CORE_READ_LOCK_FAILURE;
	}

	intersect_init(&localIntersect);

	for(ii = 0; ii < engine->domainLength; ii++) {
		if(engine->domains[ii] == NULL) {
			continue;
		}

		searchIndex_buildAllIndexValueUids(engine->domains[ii], indexType,
				&localIntersect);
	}

	spinlock_readUnlock(&engine->lock);

	intersect_execOr(&localIntersect);

	intersect_putArray(intersect,
			localIntersect.result.isSorted,
			localIntersect.result.array,
			localIntersect.result.length);

	// cleanup

	intersect_free(&localIntersect);

	return SEARCH_ENGINE_ERROR_OK;
}

int searchEngine_search(SearchEngine *engine, char *domainKey, char *attribute,
		char *value, Intersect *intersect)
{
	int ii = 0;
	int keyLength = 0;
	int tokenCount = 0;
	int stringLength = 0;
	int stringResultLength = 0;
	int resultSetLength = 0;
	int minStringLength = 0;
	int delimitersLength = 0;
	int *tokenLengths = NULL;
	int *resultSet = NULL;
	char *string = NULL;
	char *delimiters = NULL;
	char **tokenList = NULL;

	SearchIndex *searchIndex = NULL;
	IndexRegistryIndex *indexType = NULL;

	if((engine == NULL) || (domainKey == NULL) ||
			((keyLength = strlen(domainKey)) < 1) || (attribute == NULL) ||
			(value == NULL) || (intersect == NULL)) {
		DISPLAY_INVALID_ARGS;
		return SEARCH_ENGINE_ERROR_CORE_INVALID_ARGUMENTS;
	}

	// lookup the specified domain

	if(spinlock_readLock(&engine->lock) < 0) {
		return SEARCH_ENGINE_ERROR_CORE_READ_LOCK_FAILURE;
	}

	if((bptree_get(&engine->indexes, domainKey, keyLength,
					((void *)&searchIndex)) < 0) ||
			(searchIndex == NULL)) {
		spinlock_readUnlock(&engine->lock);

		log_logf(engine->log, LOG_LEVEL_ERROR,
				"unable to locate domain '%s' for search execution",
				domainKey);

		return SEARCH_ENGINE_ERROR_DOMAIN_FAILED_TO_LOCATE;
	}

	spinlock_readUnlock(&engine->lock);

	// determine if an index exists for this attribute

	stringLength = strlen(attribute);
	string = strndup(attribute, stringLength);

	if(searchEngineCommon_buildIndexString(string, stringLength,
				&stringResultLength) < 0) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"normalized index registry key '%s' is invalid",
				attribute);
		free(string);
		return SEARCH_ENGINE_ERROR_COMMON_FAILED_TO_BUILD_INDEX_STRING;
	}

	if((indexType = indexRegistry_get(&engine->registry, string)) == NULL) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"no index registry entry exists for '%s'",
				attribute);
		free(string);
		return SEARCH_ENGINE_ERROR_INDEX_FAILED_TO_LOCATE;
	}

	free(string);

	if((indexType->type != INDEX_REGISTRY_TYPE_EXACT) &&
			(indexType->type != INDEX_REGISTRY_TYPE_WILDCARD) &&
			(indexType->type != INDEX_REGISTRY_TYPE_USER_KEY)) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"unable to execute string-search on range-index '%s'",
				indexType->key);
		return SEARCH_ENGINE_ERROR_INDEX_TYPE_INVALID;
	}

	// determine if is exact or user-key search

	if((indexType->type == INDEX_REGISTRY_TYPE_EXACT) ||
			(indexType->type == INDEX_REGISTRY_TYPE_USER_KEY)) {
		stringLength = strlen(value);
		string = strndup(value, stringLength);

		if(searchEngineCommon_normalizeString(string, stringLength,
					&stringResultLength) < 0) {
			log_logf(engine->log, LOG_LEVEL_DEBUG,
					"exact-search string { '%s'->'%s' } normalization failed",
					indexType->key, value);
			free(string);
			return SEARCH_ENGINE_ERROR_COMMON_FAILED_TO_NORMALIZE_STRING;
		}

		stringLength = stringResultLength;

		resultSet = searchIndex_getString(searchIndex, indexType,
				string, &resultSetLength);

		log_logf(engine->log, LOG_LEVEL_DEBUG,
				"obtained %i results from domain '%s' for "
				"search '%s' on attribute '%s'",
				resultSetLength, searchIndex->name,
				string, indexType->name);

		if((resultSet == NULL) || (resultSetLength < 1)) {
			if(resultSet != NULL) {
				free(resultSet);
			}
			intersect_put(intersect, intersect->length, 0);
		}
		else {
			intersect_putArrayStatic(intersect, atrue, resultSet,
					resultSetLength);
		}

		free(string);

		return SEARCH_ENGINE_ERROR_OK;
	}

	// determine string length thresholds

	if(indexType->settings.isOverrideStringLengths) {
		minStringLength = indexType->settings.minStringLength;
	}
	else {
		minStringLength = engine->settings.minStringLength;
	}

	// determine string delimiters

	if(indexType->settings.isOverrideDelimiters) {
		delimiters = indexType->settings.delimiters;
		delimitersLength = indexType->settings.delimitersLength;
	}
	else {
		delimiters = engine->settings.delimiters;
		delimitersLength = engine->settings.delimitersLength;
	}

	// tokenize search string

	tokenList = searchEngineCommon_stringToTokens(
			minStringLength,
			delimiters,
			delimitersLength,
			&engine->esa,
			value,
			strlen(value),
			&tokenLengths,
			&tokenCount);

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

		log_logf(engine->log, LOG_LEVEL_ERROR,
				"failed to tokenize string-search '%s'", value);

		return SEARCH_ENGINE_ERROR_COMMON_FAILED_TO_TOKENIZE_STRING;
	}

	// execute search

	for(ii = 0; ii < tokenCount; ii++) {
		if(tokenList[ii] == NULL) {
			continue;
		}

		resultSet = searchIndex_getString(searchIndex, indexType,
				tokenList[ii], &resultSetLength);

		log_logf(engine->log, LOG_LEVEL_DEBUG,
				"obtained %i results from domain '%s' for "
				"search '%s' on attribute '%s'",
				resultSetLength, searchIndex->name,
				tokenList[ii], indexType->name);

		if((resultSet == NULL) || (resultSetLength < 1)) {
			if(resultSet != NULL) {
				free(resultSet);
			}
			free(tokenList[ii]);
			intersect_put(intersect, intersect->length, 0);
			continue;
		}

		intersect_putArrayStatic(intersect, atrue, resultSet, resultSetLength);

		free(tokenList[ii]);
	}

	free(tokenList);
	free(tokenLengths);

	return SEARCH_ENGINE_ERROR_OK;
}

int searchEngine_searchGlobal(SearchEngine *engine, char *attribute,
		char *value, Intersect *intersect)
{
	int ii = 0;
	int nn = 0;
	int tokenCount = 0;
	int stringLength = 0;
	int stringResultLength = 0;
	int resultSetLength = 0;
	int minStringLength = 0;
	int delimitersLength = 0;
	int *tokenLengths = NULL;
	int *resultSet = NULL;
	char *string = NULL;
	char *delimiters = NULL;
	char **tokenList = NULL;

	Intersect localIntersect;
	IndexRegistryIndex *indexType = NULL;

	if((engine == NULL) || (attribute == NULL) || (value == NULL) ||
			(intersect == NULL)) {
		DISPLAY_INVALID_ARGS;
		return SEARCH_ENGINE_ERROR_CORE_INVALID_ARGUMENTS;
	}

	// determine if an index exists for this attribute

	stringLength = strlen(attribute);
	string = strndup(attribute, stringLength);

	if(searchEngineCommon_buildIndexString(string, stringLength,
				&stringResultLength) < 0) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"normalized index registry key '%s' is invalid",
				attribute);
		free(string);
		return SEARCH_ENGINE_ERROR_COMMON_FAILED_TO_BUILD_INDEX_STRING;
	}

	if((indexType = indexRegistry_get(&engine->registry, string)) == NULL) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"no index registry entry exists for '%s'",
				attribute);
		free(string);
		return SEARCH_ENGINE_ERROR_INDEX_FAILED_TO_LOCATE;
	}

	free(string);

	if((indexType->type != INDEX_REGISTRY_TYPE_EXACT) &&
			(indexType->type != INDEX_REGISTRY_TYPE_WILDCARD) &&
			(indexType->type != INDEX_REGISTRY_TYPE_USER_KEY)) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"unable to execute string-search on range-index '%s'",
				indexType->key);
		return SEARCH_ENGINE_ERROR_INDEX_TYPE_INVALID;
	}

	// determine if is exact or user-key search

	if((indexType->type == INDEX_REGISTRY_TYPE_EXACT) ||
			(indexType->type == INDEX_REGISTRY_TYPE_USER_KEY)) {
		stringLength = strlen(value);
		string = strndup(value, stringLength);

		if(searchEngineCommon_normalizeString(string, stringLength,
					&stringResultLength) < 0) {
			log_logf(engine->log, LOG_LEVEL_DEBUG,
					"exact-search string { '%s'->'%s' } normalization failed",
					indexType->key, value);
			free(string);
			return SEARCH_ENGINE_ERROR_COMMON_FAILED_TO_NORMALIZE_STRING;
		}

		stringLength = stringResultLength;

		intersect_init(&localIntersect);

		if(spinlock_readLock(&engine->lock) < 0) {
			free(string);
			intersect_free(&localIntersect);
			return SEARCH_ENGINE_ERROR_CORE_READ_LOCK_FAILURE;
		}

		for(ii = 0; ii < engine->domainLength; ii++) {
			if(engine->domains[ii] == NULL) {
				continue;
			}

			resultSet = searchIndex_getString(engine->domains[ii],
					indexType, string, &resultSetLength);

			log_logf(engine->log, LOG_LEVEL_DEBUG,
					"obtained %i results from domain '%s' for "
					"search '%s' on attribute '%s'",
					resultSetLength, engine->domains[ii]->name,
					string, indexType->name);

			if((resultSet == NULL) || (resultSetLength < 1)) {
				if(resultSet != NULL) {
					free(resultSet);
				}
				intersect_put(&localIntersect, localIntersect.length, 0);
				continue;
			}
	
			intersect_putArrayStatic(&localIntersect, atrue, resultSet,
					resultSetLength);
		}

		spinlock_readUnlock(&engine->lock);

		intersect_execOr(&localIntersect);

		intersect_putArray(intersect,
				localIntersect.result.isSorted,
				localIntersect.result.array,
				localIntersect.result.length);

		// cleanup

		free(string);

		intersect_free(&localIntersect);

		return SEARCH_ENGINE_ERROR_OK;
	}

	// determine string length thresholds

	if(indexType->settings.isOverrideStringLengths) {
		minStringLength = indexType->settings.minStringLength;
	}
	else {
		minStringLength = engine->settings.minStringLength;
	}

	// determine string delimiters

	if(indexType->settings.isOverrideDelimiters) {
		delimiters = indexType->settings.delimiters;
		delimitersLength = indexType->settings.delimitersLength;
	}
	else {
		delimiters = engine->settings.delimiters;
		delimitersLength = engine->settings.delimitersLength;
	}

	// tokenize search string

	tokenList = searchEngineCommon_stringToTokens(
			minStringLength,
			delimiters,
			delimitersLength,
			&engine->esa,
			value,
			strlen(value),
			&tokenLengths,
			&tokenCount);

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

		log_logf(engine->log, LOG_LEVEL_ERROR,
				"failed to tokenize string-search '%s'", value);

		return SEARCH_ENGINE_ERROR_COMMON_FAILED_TO_TOKENIZE_STRING;
	}

	// execute search across all domains

	intersect_init(&localIntersect);

	if(spinlock_readLock(&engine->lock) < 0) {
		for(ii = 0; ii < tokenCount; ii++) {
			if(tokenList[ii] != NULL) {
				free(tokenList[ii]);
			}
		}

		free(tokenList);
		free(tokenLengths);

		intersect_free(&localIntersect);

		return SEARCH_ENGINE_ERROR_CORE_READ_LOCK_FAILURE;
	}

	for(ii = 0; ii < engine->domainLength; ii++) {
		if(engine->domains[ii] == NULL) {
			continue;
		}

		for(nn = 0; nn < tokenCount; nn++) {
			if(tokenList[nn] == NULL) {
				continue;
			}

			resultSet = searchIndex_getString(engine->domains[ii], indexType,
					tokenList[nn], &resultSetLength);

			log_logf(engine->log, LOG_LEVEL_DEBUG,
					"obtained %i results from domain '%s' for "
					"search '%s' on attribute '%s'",
					resultSetLength, engine->domains[ii]->name,
					tokenList[nn], indexType->name);

			if((resultSet == NULL) || (resultSetLength < 1)) {
				if(resultSet != NULL) {
					free(resultSet);
				}
				intersect_put(&localIntersect, localIntersect.length, 0);
				continue;
			}

			intersect_putArrayStatic(&localIntersect, atrue, resultSet,
					resultSetLength);
		}
	}

	spinlock_readUnlock(&engine->lock);

	intersect_execOr(&localIntersect);

	intersect_putArray(intersect,
			localIntersect.result.isSorted,
			localIntersect.result.array,
			localIntersect.result.length);

	// cleanup

	for(ii = 0; ii < tokenCount; ii++) {
		if(tokenList[ii] != NULL) {
			free(tokenList[ii]);
		}
	}

	free(tokenList);
	free(tokenLengths);

	intersect_free(&localIntersect);

	return SEARCH_ENGINE_ERROR_OK;
}

int searchEngine_searchRange(SearchEngine *engine, char *domainKey,
		char *attribute, int searchType, int minValue, int maxValue,
		Intersect *intersect)
{
	int keyLength = 0;
	int stringLength = 0;
	int stringResultLength = 0;
	int resultSetLength = 0;
	int *resultSet = NULL;
	char *string = NULL;

	SearchIndex *searchIndex = NULL;
	IndexRegistryIndex *indexType = NULL;

	if((engine == NULL) || (domainKey == NULL) ||
			((keyLength = strlen(domainKey)) < 1) || (attribute == NULL) ||
			(intersect == NULL)) {
		DISPLAY_INVALID_ARGS;
		return SEARCH_ENGINE_ERROR_CORE_INVALID_ARGUMENTS;
	}

	// lookup the specified domain

	if(spinlock_readLock(&engine->lock) < 0) {
		return SEARCH_ENGINE_ERROR_CORE_READ_LOCK_FAILURE;
	}

	if((bptree_get(&engine->indexes, domainKey, keyLength,
					((void *)&searchIndex)) < 0) ||
			(searchIndex == NULL)) {
		spinlock_readUnlock(&engine->lock);

		log_logf(engine->log, LOG_LEVEL_ERROR,
				"unable to locate domain '%s' for search execution",
				domainKey);

		return SEARCH_ENGINE_ERROR_DOMAIN_FAILED_TO_LOCATE;
	}

	spinlock_readUnlock(&engine->lock);

	// determine if an index exists for this attribute

	stringLength = strlen(attribute);
	string = strndup(attribute, stringLength);

	if(searchEngineCommon_buildIndexString(string, stringLength,
				&stringResultLength) < 0) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"normalized index registry key '%s' is invalid",
				attribute);
		free(string);
		return SEARCH_ENGINE_ERROR_COMMON_FAILED_TO_BUILD_INDEX_STRING;
	}

	if((indexType = indexRegistry_get(&engine->registry, string)) == NULL) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"no index registry entry exists for '%s'",
				attribute);
		free(string);
		return SEARCH_ENGINE_ERROR_INDEX_FAILED_TO_LOCATE;
	}

	free(string);

	if(indexType->type != INDEX_REGISTRY_TYPE_RANGE) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"unable to execute range-search on string-index '%s'",
				indexType->key);
		return SEARCH_ENGINE_ERROR_INDEX_TYPE_INVALID;
	}

	// execute search

	resultSet = searchIndex_getRange(searchIndex, indexType, searchType,
			minValue, maxValue, &resultSetLength);

	log_logf(engine->log, LOG_LEVEL_DEBUG,
			"obtained %i results from domain '%s' for "
			"range search '%s' for (%i, %i)",
			resultSetLength, searchIndex->name,
			rangeIndex_typeToString(searchType),
			minValue, maxValue);

	if((resultSet == NULL) || (resultSetLength < 1)) {
		if(resultSet != NULL) {
			free(resultSet);
		}
		intersect_put(intersect, intersect->length, 0);
	}
	else {
		intersect_putArrayStatic(intersect, atrue, resultSet, resultSetLength);
	}

	return SEARCH_ENGINE_ERROR_OK;
}

int searchEngine_searchRangeGlobal(SearchEngine *engine, char *attribute,
		int searchType, int minValue, int maxValue, Intersect *intersect)
{
	int ii = 0;
	int stringLength = 0;
	int stringResultLength = 0;
	int resultSetLength = 0;
	int *resultSet = NULL;
	char *string = NULL;

	Intersect localIntersect;
	IndexRegistryIndex *indexType = NULL;

	if((engine == NULL) || (attribute == NULL) || (intersect == NULL)) {
		DISPLAY_INVALID_ARGS;
		return SEARCH_ENGINE_ERROR_CORE_INVALID_ARGUMENTS;
	}

	// determine if an index exists for this attribute

	stringLength = strlen(attribute);
	string = strndup(attribute, stringLength);

	if(searchEngineCommon_buildIndexString(string, stringLength,
				&stringResultLength) < 0) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"normalized index registry key '%s' is invalid",
				attribute);
		free(string);
		return SEARCH_ENGINE_ERROR_COMMON_FAILED_TO_BUILD_INDEX_STRING;
	}

	if((indexType = indexRegistry_get(&engine->registry, string)) == NULL) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"no index registry entry exists for '%s'",
				attribute);
		free(string);
		return SEARCH_ENGINE_ERROR_INDEX_FAILED_TO_LOCATE;
	}

	free(string);

	if(indexType->type != INDEX_REGISTRY_TYPE_RANGE) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"unable to execute range-search on string-index '%s'",
				indexType->key);
		return SEARCH_ENGINE_ERROR_INDEX_TYPE_INVALID;
	}

	// execute search across all domains

	intersect_init(&localIntersect);

	if(spinlock_readLock(&engine->lock) < 0) {
		intersect_free(&localIntersect);
		return SEARCH_ENGINE_ERROR_CORE_READ_LOCK_FAILURE;
	}

	for(ii = 0; ii < engine->domainLength; ii++) {
		if(engine->domains[ii] == NULL) {
			continue;
		}

		resultSet = searchIndex_getRange(engine->domains[ii], indexType,
				searchType, minValue, maxValue, &resultSetLength);

		log_logf(engine->log, LOG_LEVEL_DEBUG,
				"obtained %i results from domain '%s' for "
				"range search '%s' for (%i, %i)",
				resultSetLength, engine->domains[ii]->name,
				rangeIndex_typeToString(searchType),
				minValue, maxValue);

		if((resultSet == NULL) || (resultSetLength < 1)) {
			if(resultSet != NULL) {
				free(resultSet);
			}
			intersect_put(&localIntersect, localIntersect.length, 0);
		}
		else {
			intersect_putArrayStatic(&localIntersect, atrue, resultSet,
					resultSetLength);
		}
	}

	spinlock_readUnlock(&engine->lock);

	intersect_execOr(&localIntersect);

	intersect_putArray(intersect,
			localIntersect.result.isSorted,
			localIntersect.result.array,
			localIntersect.result.length);

	// cleanup

	intersect_free(&localIntersect);

	return SEARCH_ENGINE_ERROR_OK;
}

int searchEngine_searchRangeGeocoord(SearchEngine *engine, char *domainKey,
		SearchEngineGeoCoordTypes type, double latitude, double longitude,
		double distance, Intersect *intersect)
{
	int rc = 0;
	int keyLength = 0;
	int stringLength = 0;
	int stringResultLength = 0;
	double miles = 0.0;
	double milesToDegrees = 0.0;
	double localLatitude = 0.0;
	double localLongitude = 0.0;
	double degreesPerMileLongitude = 0.0;
	char *string = NULL;
	char *attribute = NULL;

	Intersect localIntersect;
	SearchIndex *searchIndex = NULL;
	IndexRegistryIndex *indexType = NULL;

	if((engine == NULL) ||
			(domainKey == NULL) ||
			((keyLength = strlen(domainKey)) < 1) ||
			((type != SEARCH_ENGINE_GEO_COORD_TYPE_MILES) &&
			 (type != SEARCH_ENGINE_GEO_COORD_TYPE_YARDS) &&
			 (type != SEARCH_ENGINE_GEO_COORD_TYPE_FEET) &&
			 (type != SEARCH_ENGINE_GEO_COORD_TYPE_KILOMETERS) &&
			 (type != SEARCH_ENGINE_GEO_COORD_TYPE_METERS)) ||
			(latitude < -90.0) || (latitude > 90.0) ||
			(longitude < -180.0) || (longitude > 180.0) ||
			(distance <= 0.0) ||
			(intersect == NULL)) {
		DISPLAY_INVALID_ARGS;
		return SEARCH_ENGINE_ERROR_CORE_INVALID_ARGUMENTS;
	}

	// lookup the specified domain

	if(spinlock_readLock(&engine->lock) < 0) {
		return SEARCH_ENGINE_ERROR_CORE_READ_LOCK_FAILURE;
	}

	if((bptree_get(&engine->indexes, domainKey, keyLength,
					((void *)&searchIndex)) < 0) ||
			(searchIndex == NULL)) {
		spinlock_readUnlock(&engine->lock);

		log_logf(engine->log, LOG_LEVEL_ERROR,
				"unable to locate domain '%s' for search execution",
				domainKey);

		return SEARCH_ENGINE_ERROR_DOMAIN_FAILED_TO_LOCATE;
	}

	spinlock_readUnlock(&engine->lock);

	// determine if the latitude and longitude indexes exist

	attribute = "latitude";

	stringLength = strlen(attribute);
	string = strndup(attribute, stringLength);

	if(searchEngineCommon_buildIndexString(string, stringLength,
				&stringResultLength) < 0) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"normalized index registry key '%s' is invalid",
				attribute);
		free(string);
		return SEARCH_ENGINE_ERROR_COMMON_FAILED_TO_BUILD_INDEX_STRING;
	}

	if((indexType = indexRegistry_get(&engine->registry, string)) == NULL) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"no index registry entry exists for '%s'",
				attribute);
		free(string);
		return SEARCH_ENGINE_ERROR_INDEX_FAILED_TO_LOCATE;
	}

	free(string);

	if(indexType->type != INDEX_REGISTRY_TYPE_RANGE) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"unable to execute range-search on string-index '%s'",
				indexType->key);
		return SEARCH_ENGINE_ERROR_INDEX_TYPE_INVALID;
	}

	attribute = "longitude";

	stringLength = strlen(attribute);
	string = strndup(attribute, stringLength);

	if(searchEngineCommon_buildIndexString(string, stringLength,
				&stringResultLength) < 0) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"normalized index registry key '%s' is invalid",
				attribute);
		free(string);
		return SEARCH_ENGINE_ERROR_COMMON_FAILED_TO_BUILD_INDEX_STRING;
	}

	if((indexType = indexRegistry_get(&engine->registry, string)) == NULL) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"no index registry entry exists for '%s'",
				attribute);
		free(string);
		return SEARCH_ENGINE_ERROR_INDEX_FAILED_TO_LOCATE;
	}

	free(string);

	if(indexType->type != INDEX_REGISTRY_TYPE_RANGE) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"unable to execute range-search on string-index '%s'",
				indexType->key);
		return SEARCH_ENGINE_ERROR_INDEX_TYPE_INVALID;
	}

	// convert distance to miles

	switch(type) {
		case SEARCH_ENGINE_GEO_COORD_TYPE_MILES:
			miles = distance;
			break;

		case SEARCH_ENGINE_GEO_COORD_TYPE_YARDS:
			miles = (distance * 1760.0);
			break;

		case SEARCH_ENGINE_GEO_COORD_TYPE_FEET:
			miles = (distance * 5280.0);
			break;

		case SEARCH_ENGINE_GEO_COORD_TYPE_KILOMETERS:
			miles = (distance * 0.621371192);
			break;

		case SEARCH_ENGINE_GEO_COORD_TYPE_METERS:
			miles = ((distance * 0.621371192) * 1000.0);
			break;

		case SEARCH_ENGINE_GEO_COORD_TYPE_UNKNOWN:
		default:
			log_logf(engine->log, LOG_LEVEL_ERROR,
					"failed to convert distance units to miles");
			return SEARCH_ENGINE_ERROR_CORE_INVALID_GEO_COORD_DISTANCE_TYPE;
	}

	// execute search

	intersect_init(&localIntersect);

	milesToDegrees = (miles * GEOCOORD_DEGREES_PER_MILE);

	localLatitude = (latitude - milesToDegrees);
	if(localLatitude < -90.0) {
		localLatitude = -90.0;
	}

	if((rc = searchEngine_searchRange(engine,
					domainKey,
					"latitude",
					RANGE_INDEX_SEARCH_GREATER_THAN,
					(int)(localLatitude * GEOCOORD_INTEGER_CONVERSION),
					0.0,
					intersect)) < 0) {
		intersect_free(&localIntersect);
		return rc;
	}

	localLatitude = (latitude + milesToDegrees);
	if(localLatitude > 90.0) {
		localLatitude = 90.0;
	}

	if((rc = searchEngine_searchRange(engine,
					domainKey,
					"latitude",
					RANGE_INDEX_SEARCH_LESS_THAN,
					(int)(localLatitude * GEOCOORD_INTEGER_CONVERSION),
					0.0,
					intersect)) < 0) {
		intersect_free(&localIntersect);
		return rc;
	}

	degreesPerMileLongitude = (360.0 /
			(360.0 *
			 (GEOCOORD_MILES_PER_DEGREE *
			  cos(latitude * GEOCOORD_PI / 180.0))));

	milesToDegrees = (miles * degreesPerMileLongitude);

	localLongitude = (longitude - milesToDegrees);
	if(localLongitude < -180.0) {
		localLongitude = -180.0;
	}

	if((rc = searchEngine_searchRange(engine,
					domainKey,
					"longitude",
					RANGE_INDEX_SEARCH_GREATER_THAN,
					(int)(localLongitude * GEOCOORD_INTEGER_CONVERSION),
					0.0,
					intersect)) < 0) {
		intersect_free(&localIntersect);
		return rc;
	}

	localLongitude = (longitude + milesToDegrees);
	if(localLongitude > 180.0) {
		localLongitude = 180.0;
	}

	if((rc = searchEngine_searchRange(engine,
					domainKey,
					"longitude",
					RANGE_INDEX_SEARCH_LESS_THAN,
					(int)(localLongitude * GEOCOORD_INTEGER_CONVERSION),
					0.0,
					intersect)) < 0) {
		intersect_free(&localIntersect);
		return rc;
	}

	intersect_execAnd(&localIntersect, atrue);

	intersect_putArray(intersect,
			localIntersect.result.isSorted,
			localIntersect.result.array,
			localIntersect.result.length);

	// cleanup

	intersect_free(&localIntersect);

	return SEARCH_ENGINE_ERROR_OK;
}

int searchEngine_searchRangeGeocoordGlobal(SearchEngine *engine,
		SearchEngineGeoCoordTypes type, double latitude, double longitude,
		double distance, Intersect *intersect)
{
	int rc = 0;
	int stringLength = 0;
	int stringResultLength = 0;
	double miles = 0.0;
	double milesToDegrees = 0.0;
	double localLatitude = 0.0;
	double localLongitude = 0.0;
	double degreesPerMileLongitude = 0.0;
	char *string = NULL;
	char *attribute = NULL;

	Intersect localIntersect;
	IndexRegistryIndex *indexType = NULL;

	if((engine == NULL) ||
			((type != SEARCH_ENGINE_GEO_COORD_TYPE_MILES) &&
			 (type != SEARCH_ENGINE_GEO_COORD_TYPE_YARDS) &&
			 (type != SEARCH_ENGINE_GEO_COORD_TYPE_FEET) &&
			 (type != SEARCH_ENGINE_GEO_COORD_TYPE_KILOMETERS) &&
			 (type != SEARCH_ENGINE_GEO_COORD_TYPE_METERS)) ||
			(latitude < -90.0) || (latitude > 90.0) ||
			(longitude < -180.0) || (longitude > 180.0) ||
			(distance <= 0.0) ||
			(intersect == NULL)) {
		DISPLAY_INVALID_ARGS;
		return SEARCH_ENGINE_ERROR_CORE_INVALID_ARGUMENTS;
	}

	// determine if the latitude and longitude indexes exist

	attribute = "latitude";

	stringLength = strlen(attribute);
	string = strndup(attribute, stringLength);

	if(searchEngineCommon_buildIndexString(string, stringLength,
				&stringResultLength) < 0) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"normalized index registry key '%s' is invalid",
				attribute);
		free(string);
		return SEARCH_ENGINE_ERROR_COMMON_FAILED_TO_BUILD_INDEX_STRING;
	}

	if((indexType = indexRegistry_get(&engine->registry, string)) == NULL) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"no index registry entry exists for '%s'",
				attribute);
		free(string);
		return SEARCH_ENGINE_ERROR_INDEX_FAILED_TO_LOCATE;
	}

	free(string);

	if(indexType->type != INDEX_REGISTRY_TYPE_RANGE) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"unable to execute range-search on string-index '%s'",
				indexType->key);
		return SEARCH_ENGINE_ERROR_INDEX_TYPE_INVALID;
	}

	attribute = "longitude";

	stringLength = strlen(attribute);
	string = strndup(attribute, stringLength);

	if(searchEngineCommon_buildIndexString(string, stringLength,
				&stringResultLength) < 0) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"normalized index registry key '%s' is invalid",
				attribute);
		free(string);
		return SEARCH_ENGINE_ERROR_COMMON_FAILED_TO_BUILD_INDEX_STRING;
	}

	if((indexType = indexRegistry_get(&engine->registry, string)) == NULL) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"no index registry entry exists for '%s'",
				attribute);
		free(string);
		return SEARCH_ENGINE_ERROR_INDEX_FAILED_TO_LOCATE;
	}

	free(string);

	if(indexType->type != INDEX_REGISTRY_TYPE_RANGE) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"unable to execute range-search on string-index '%s'",
				indexType->key);
		return SEARCH_ENGINE_ERROR_INDEX_TYPE_INVALID;
	}

	// convert distance to miles

	switch(type) {
		case SEARCH_ENGINE_GEO_COORD_TYPE_MILES:
			miles = distance;
			break;

		case SEARCH_ENGINE_GEO_COORD_TYPE_YARDS:
			miles = (distance * 1760.0);
			break;

		case SEARCH_ENGINE_GEO_COORD_TYPE_FEET:
			miles = (distance * 5280.0);
			break;

		case SEARCH_ENGINE_GEO_COORD_TYPE_KILOMETERS:
			miles = (distance * 0.621371192);
			break;

		case SEARCH_ENGINE_GEO_COORD_TYPE_METERS:
			miles = ((distance * 0.621371192) * 1000.0);
			break;

		case SEARCH_ENGINE_GEO_COORD_TYPE_UNKNOWN:
		default:
			log_logf(engine->log, LOG_LEVEL_ERROR,
					"failed to convert distance units to miles");
			return SEARCH_ENGINE_ERROR_CORE_INVALID_GEO_COORD_DISTANCE_TYPE;
	}

	// execute search

	intersect_init(&localIntersect);

	milesToDegrees = (miles * GEOCOORD_DEGREES_PER_MILE);

	localLatitude = (latitude - milesToDegrees);
	if(localLatitude < -90.0) {
		localLatitude = -90.0;
	}

	if((rc = searchEngine_searchRangeGlobal(engine,
					"latitude",
					RANGE_INDEX_SEARCH_GREATER_THAN,
					(int)(localLatitude * GEOCOORD_INTEGER_CONVERSION),
					0.0,
					&localIntersect)) < 0) {
		intersect_free(&localIntersect);
		return rc;
	}

	localLatitude = (latitude + milesToDegrees);
	if(localLatitude > 90.0) {
		localLatitude = 90.0;
	}

	if((rc = searchEngine_searchRangeGlobal(engine,
					"latitude",
					RANGE_INDEX_SEARCH_LESS_THAN,
					(int)(localLatitude * GEOCOORD_INTEGER_CONVERSION),
					0.0,
					&localIntersect)) < 0) {
		intersect_free(&localIntersect);
		return rc;
	}

	degreesPerMileLongitude = (360.0 /
			(360.0 *
			 (GEOCOORD_MILES_PER_DEGREE *
			  cos(latitude * GEOCOORD_PI / 180.0))));

	milesToDegrees = (miles * degreesPerMileLongitude);

	localLongitude = (longitude - milesToDegrees);
	if(localLongitude < -180.0) {
		localLongitude = -180.0;
	}

	if((rc = searchEngine_searchRangeGlobal(engine,
					"longitude",
					RANGE_INDEX_SEARCH_GREATER_THAN,
					(int)(localLongitude * GEOCOORD_INTEGER_CONVERSION),
					0.0,
					&localIntersect)) < 0) {
		intersect_free(&localIntersect);
		return rc;
	}

	localLongitude = (longitude + milesToDegrees);
	if(localLongitude > 180.0) {
		localLongitude = 180.0;
	}

	if((rc = searchEngine_searchRangeGlobal(engine,
					"longitude",
					RANGE_INDEX_SEARCH_LESS_THAN,
					(int)(localLongitude * GEOCOORD_INTEGER_CONVERSION),
					0.0,
					&localIntersect)) < 0) {
		intersect_free(&localIntersect);
		return rc;
	}

	intersect_execAnd(&localIntersect, atrue);

	intersect_putArray(intersect,
			localIntersect.result.isSorted,
			localIntersect.result.array,
			localIntersect.result.length);

	// cleanup

	intersect_free(&localIntersect);

	return SEARCH_ENGINE_ERROR_OK;
}

