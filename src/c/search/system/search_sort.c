/*
 * search_sort.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The sorting component for the search-engine.
 *
 * Written by Josh English.
 */

// preprocessor directives

#define _CORE_COMMON_BYPASS_STD_STREAM

#include "core/core.h"
#include "search/search.h"
#define _SEARCH_SYSTEM_COMPONENT
#include "search/system/search_sort.h"


// define search sort private data types

typedef struct _AttributeSort {
	int uid;
	int length;
	char *value;
} AttributeSort;

typedef struct _RelevancySort {
	int uid;
	double relevancy;
} RelevancySort;

typedef struct _DistanceSort {
	int uid;
	double distance;
} DistanceSort;

typedef struct _EntityGeoCoord {
	int uid;
	double distance;
	double latitude;
	double longitude;
} EntityGeoCoord;


// declare search sort private functions

// general functions

static char **castSerialStringArray(char *string, int stringLength,
		int arrayLength);

// attribute functions

static int attribute_compareValues(char *keyAlpha, int keyAlphaLength,
		char *keyBeta, int keyBetaLength);

static int attribute_compareAscending(const void *left, const void *right);

static int attribute_compareDescending(const void *left, const void *right);

static alint attribute_calculateBuildEntryMemoryLength(SearchEngine *engine,
		char *attribute, int uid);

static void attribute_buildEntry(SearchEngine *engine, char *attribute,
		int uid, AttributeSort *sortEntry);

// relevancy functions

static int relevancy_compareAscending(const void *left, const void *right);

static int relevancy_compareDescending(const void *left, const void *right);

static IndexRegistryIndex *relevancy_doesIndexExists(SearchEngine *engine,
		char *attribute);

static void relevancy_buildEntry(SearchEngine *engine, char **tokenList,
		int tokenCount, char *relevancyKey, char *relevancyLengthKey,
		int uid, RelevancySort *relevancySort);

// distance functions

static int distance_compareAscending(const void *left, const void *right);

static int distance_compareDescending(const void *left, const void *right);

// entity geocoord functions

static int entityGeoCoord_compareAscending(const void *left,
		const void *right);

// multi-value functions

static int multiValue_compare(const void *left, const void *right);

// facet functions

static SearchSortFacetEntry *facet_new(char *value, int valueLength);

static void facet_free(void *memory);

static void facet_freeNothing(void *memory);

static void facet_updateIndex(SearchEngine *engine,
		IndexRegistryIndexSetting *settings, IndexRegistryTypes type,
		char *attribute, int attributeLength, int uid, Bptree *index);


// define search sort private functions

// general functions

static char **castSerialStringArray(char *string, int stringLength,
		int arrayLength)
{
	aboolean found = afalse;
	int ii = 0;
	int headerLength = 0;
	char *stringPtr = NULL;
	char **result = NULL;

	result = (char **)string;

	headerLength = (int)(sizeof(char **) * arrayLength);

	stringPtr = (string + headerLength);

	found = atrue;

	do {
		if((*stringPtr) == '\0') {
			found = atrue;
		}
		else if(found) {
			result[ii] = stringPtr;

			ii++;

			if(ii >= arrayLength) {
				break;
			}

			found = afalse;
		}

		stringPtr++;
	} while((int)(stringPtr - string) < stringLength);

	return result;
}

// attribute functions

static int attribute_compareValues(char *keyAlpha, int keyAlphaLength,
		char *keyBeta, int keyBetaLength)
{
	int ii = 0;
	int iAlpha = 0;
	int iBeta = 0;
	double dAlpha = 0.0;
	double dBeta = 0.0;

	if(keyAlpha == NULL) {
		if(keyBeta == NULL) {
			return 0;
		}
		return -1;
	}
	else if(keyBeta == NULL) {
		return 1;
	}

	if((keyAlphaLength < 1) || (keyBetaLength < 1)) {
		if(keyAlphaLength < keyBetaLength) {
			return -1;
		}
		else if(keyAlphaLength > keyBetaLength) {
			return 1;
		}
		return 0;
	}

	if((cstring_isNumericInteger(keyAlpha, keyAlphaLength)) &&
			(cstring_isNumericInteger(keyBeta, keyBetaLength))) {
		iAlpha = atoi(keyAlpha);
		iBeta = atoi(keyBeta);

		if(iAlpha < iBeta) {
			return -1;
		}
		else if(iAlpha > iBeta) {
			return 1;
		}

		return 0;
	}
	else if((cstring_isNumericReal(keyAlpha, keyAlphaLength)) &&
			(cstring_isNumericReal(keyBeta, keyBetaLength))) {
		dAlpha = atod(keyAlpha);
		dBeta = atod(keyBeta);

		if(dAlpha < dBeta) {
			return -1;
		}
		else if(dAlpha > dBeta) {
			return 1;
		}

		return 0;
	}

	for(ii = 0; ((ii < keyAlphaLength) && (ii < keyBetaLength)); ii++) {
		if((ctype_isAlphabetic(keyAlpha[ii])) &&
				(ctype_isAlphabetic(keyBeta[ii]))) {
			iAlpha = (int)((unsigned char)ctype_toLower(keyAlpha[ii]));
			iBeta = (int)((unsigned char)ctype_toLower(keyBeta[ii]));
		}
		else {
			iAlpha = (int)((unsigned char)keyAlpha[ii]);
			iBeta = (int)((unsigned char)keyBeta[ii]);
		}

		if(iAlpha < iBeta) {
			return -1;
		}
		else if(iAlpha > iBeta) {
			return 1;
		}
	}

	if(keyAlphaLength < keyBetaLength) {
		return -1;
	}
	else if(keyAlphaLength > keyBetaLength) {
		return 1;
	}

	return 0;
}

static int attribute_compareAscending(const void *left, const void *right)
{
	AttributeSort *alpha = NULL;
	AttributeSort *beta = NULL;

	if(left == NULL) {
		if(right == NULL) {
			return 0;
		}
		return 1;
	}
	else if(right == NULL) {
		return -1;
	}

	alpha = (AttributeSort *)((aptrcast)left);
	beta = (AttributeSort *)((aptrcast)right);

	return attribute_compareValues(alpha->value, alpha->length, beta->value,
			beta->length);
}

static int attribute_compareDescending(const void *left, const void *right)
{
	int result = 0;

	AttributeSort *alpha = NULL;
	AttributeSort *beta = NULL;

	if(left == NULL) {
		if(right == NULL) {
			return 0;
		}
		return 1;
	}
	else if(right == NULL) {
		return -1;
	}

	alpha = (AttributeSort *)((aptrcast)left);
	beta = (AttributeSort *)((aptrcast)right);

	if((result = attribute_compareValues(alpha->value, alpha->length,
					beta->value, beta->length)) != 0) {
		result *= -1;
	}

	return result;
}

static alint attribute_calculateBuildEntryMemoryLength(SearchEngine *engine,
		char *attribute, int uid)
{
	int rc = 0;
	int stringLength = 0;
	alint result = 0;
	char *string = NULL;

	Container *container = NULL;

	if((uid <= 0) ||
			((container = searchEngine_get(engine, uid, &rc)) == NULL)) {
		return (8 + 1);
	}

	if(!strcasecmp(attribute, SEARCHD_CONTAINER_UID)) {
		string = itoa(container->uid);

		result += (alint)(strlen(string) + 1);

		free(string);
	}
	else if(!strcasecmp(attribute, SEARCHD_CONTAINER_NAME)) {
		if((string = container_getName(container, &stringLength)) == NULL) {
			result += (alint)(8 + 1);
		}
		else {
			result += (alint)(stringLength + 1);

			free(string);
		}
	}
	else if((string = container_getString(container, attribute,
					strlen(attribute), &stringLength)) == NULL) {
		result += (alint)(8 + 1);
	}
	else {
		result += (alint)(stringLength + 1);
		free(string);
	}

	searchEngine_unlockGet(engine, container);

	return result;
}

static void attribute_buildEntry(SearchEngine *engine, char *attribute,
		int uid, AttributeSort *sortEntry)
{
	int rc = 0;
	int stringLength = 0;
	char *string = NULL;

	Container *container = NULL;

	if((uid <= 0) ||
			((container = searchEngine_get(engine, uid, &rc)) == NULL)) {
		sortEntry->uid = uid;
		sortEntry->length = 8;
		sortEntry->value = strndup("ZZZZZZZZ", 8);
		return;
	}

	sortEntry->uid = container->uid;

	if(!strcasecmp(attribute, SEARCHD_CONTAINER_UID)) {
		sortEntry->value = itoa(container->uid);
		sortEntry->length = strlen(sortEntry->value);
	}
	else if(!strcasecmp(attribute, SEARCHD_CONTAINER_NAME)) {
		if((string = container_getName(container, &stringLength)) == NULL) {
			sortEntry->length = 8;
			sortEntry->value = strndup("ZZZZZZZZ", 8);
		}
		else {
			sortEntry->length = stringLength;
			sortEntry->value = strndup(string, stringLength);
		}
	}
	else if((string = container_getString(container, attribute,
					strlen(attribute), &stringLength)) == NULL) {
		sortEntry->length = 8;
		sortEntry->value = strndup("ZZZZZZZZ", 8);
	}
	else {
		sortEntry->length = stringLength;
		sortEntry->value = string;
	}

	searchEngine_unlockGet(engine, container);
}

// relevancy functions

static int relevancy_compareAscending(const void *left, const void *right)
{
	RelevancySort *alpha = NULL;
	RelevancySort *beta = NULL;

	if(left == NULL) {
		if(right == NULL) {
			return 0;
		}
		return 1;
	}
	else if(right == NULL) {
		return -1;
	}

	alpha = (RelevancySort *)((aptrcast)left);
	beta = (RelevancySort *)((aptrcast)right);

	if(alpha->relevancy < beta->relevancy) {
		return -1;
	}
	else if(alpha->relevancy > beta->relevancy) {
		return 1;
	}

	return 0;
}

static int relevancy_compareDescending(const void *left, const void *right)
{
	RelevancySort *alpha = NULL;
	RelevancySort *beta = NULL;

	if(left == NULL) {
		if(right == NULL) {
			return 0;
		}
		return 1;
	}
	else if(right == NULL) {
		return -1;
	}

	alpha = (RelevancySort *)((aptrcast)left);
	beta = (RelevancySort *)((aptrcast)right);

	if(alpha->relevancy < beta->relevancy) {
		return 1;
	}
	else if(alpha->relevancy > beta->relevancy) {
		return -1;
	}

	return 0;
}

static IndexRegistryIndex *relevancy_doesIndexExists(SearchEngine *engine,
		char *attribute)
{
	int stringLength = 0;
	int stringResultLength = 0;
	char *string = NULL;

	IndexRegistryIndex *result = NULL;

	stringLength = strlen(attribute);
	string = strndup(attribute, stringLength);

	if(searchEngineCommon_buildIndexString(string, stringLength,
				&stringResultLength) < 0) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"normalized index registry key '%s' is invalid",
				attribute);
		free(string);
		return NULL;
	}

	if((result = indexRegistry_get(&engine->registry, string)) == NULL) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"no index registry entry exists for '%s'",
				attribute);
		free(string);
		return NULL;
	}

	free(string);

	if((result->type != INDEX_REGISTRY_TYPE_EXACT) &&
			(result->type != INDEX_REGISTRY_TYPE_WILDCARD) &&
			(result->type != INDEX_REGISTRY_TYPE_USER_KEY)) {
		log_logf(engine->log, LOG_LEVEL_ERROR,
				"unable to execute sort on index '%s'", result->key);
		return NULL;
	}

	return result;
}

static void relevancy_buildEntry(SearchEngine *engine, char **tokenList,
		int tokenCount, char *relevancyKey, char *relevancyLengthKey,
		int uid, RelevancySort *relevancySort)
{
	int rc = 0;
	int stringLength = 0;
	int relevancyLength = 0;
	char *string = NULL;
	char **relevancyList = NULL;

	Container *container = NULL;

	if((uid <= 0) ||
			((container = searchEngine_get(engine, uid, &rc)) == NULL)) {
		relevancySort->uid = uid;
		relevancySort->relevancy = 0.0;
	}
	else {
		relevancySort->uid = container->uid;

		string = container_getString(container, relevancyKey,
				strlen(relevancyKey), &stringLength);

		relevancyLength = container_getInteger(container,
				relevancyLengthKey, strlen(relevancyLengthKey));

		if((string != NULL) && (stringLength > 0) && (relevancyLength > 0)) {
			relevancyList = castSerialStringArray(string,
					stringLength, relevancyLength);

			relevancySort->relevancy = compareArrays(relevancyList,
					relevancyLength, tokenList, tokenCount, 4);
		}
		else {
			relevancySort->relevancy = 0.0;
		}

		if(string != NULL) {
			free(string);
		}

		searchEngine_unlockGet(engine, container);
	}
}

// distance functions

static int distance_compareAscending(const void *left, const void *right)
{
	DistanceSort *alpha = NULL;
	DistanceSort *beta = NULL;

	if(left == NULL) {
		if(right == NULL) {
			return 0;
		}
		return 1;
	}
	else if(right == NULL) {
		return -1;
	}

	alpha = (DistanceSort *)((aptrcast)left);
	beta = (DistanceSort *)((aptrcast)right);

	if(alpha->distance < beta->distance) {
		return -1;
	}
	else if(alpha->distance > beta->distance) {
		return 1;
	}

	return 0;
}

static int distance_compareDescending(const void *left, const void *right)
{
	DistanceSort *alpha = NULL;
	DistanceSort *beta = NULL;

	if(left == NULL) {
		if(right == NULL) {
			return 0;
		}
		return 1;
	}
	else if(right == NULL) {
		return -1;
	}

	alpha = (DistanceSort *)((aptrcast)left);
	beta = (DistanceSort *)((aptrcast)right);

	if(alpha->distance < beta->distance) {
		return 1;
	}
	else if(alpha->distance > beta->distance) {
		return -1;
	}

	return 0;
}

// entity geocoord functions

static int entityGeoCoord_compareAscending(const void *left, const void *right)
{
	EntityGeoCoord *alpha = NULL;
	EntityGeoCoord *beta = NULL;

	if(left == NULL) {
		if(right == NULL) {
			return 0;
		}
		return 1;
	}
	else if(right == NULL) {
		return -1;
	}

	alpha = (EntityGeoCoord *)((aptrcast)left);
	beta = (EntityGeoCoord *)((aptrcast)right);

	if(alpha->distance < beta->distance) {
		return -1;
	}
	else if(alpha->distance > beta->distance) {
		return 1;
	}

	return 0;
}

// multi-value functions

static int multiValue_compare(const void *left, const void *right)
{
	int ii = 0;
	int rc = 0;

	SearchSortEntry *alpha = NULL;
	SearchSortEntry *beta = NULL;
	SearchSort *sort = NULL;

	if(left == NULL) {
		if(right == NULL) {
			return 0;
		}
		return 1;
	}
	else if(right == NULL) {
		return -1;
	}

	alpha = (SearchSortEntry *)((aptrcast)left);
	beta = (SearchSortEntry *)((aptrcast)right);

	sort = alpha->context;

	for(ii = 0; ii < sort->configLength; ii++) {
		switch(sort->config[ii].type) {
			case SEARCH_SORT_TYPE_ATTRIBUTE:
				if(sort->config[ii].isStandardOrder) {
					rc = attribute_compareDescending(
							(const void *)alpha->items[ii],
							(const void *)beta->items[ii]);
				}
				else {
					rc = attribute_compareAscending(
							(const void *)alpha->items[ii],
							(const void *)beta->items[ii]);
				}
				break;

			case SEARCH_SORT_TYPE_RELEVANCY:
				if(sort->config[ii].isStandardOrder) {
					rc = relevancy_compareDescending(
							(const void *)alpha->items[ii],
							(const void *)beta->items[ii]);
				}
				else {
					rc = relevancy_compareAscending(
							(const void *)alpha->items[ii],
							(const void *)beta->items[ii]);
				}
				break;

			case SEARCH_SORT_TYPE_DISTANCE:
				if(sort->config[ii].isStandardOrder) {
					rc = distance_compareDescending(
							(const void *)alpha->items[ii],
							(const void *)beta->items[ii]);
				}
				else {
					rc = distance_compareAscending(
							(const void *)alpha->items[ii],
							(const void *)beta->items[ii]);
				}
				break;

			case SEARCH_SORT_TYPE_UNKNOWN:
			default:
				log_logf(sort->engine->log, LOG_LEVEL_WARNING,
						"{SORT} unknown multi-value type %i",
						sort->config[ii].type);
				rc = 0;
				break;
		}

		if(rc != 0) {
			return rc;
		}
	}

	return 0;
}

// facet functions

static SearchSortFacetEntry *facet_new(char *value, int valueLength)
{
	SearchSortFacetEntry *result = NULL;

	result = (SearchSortFacetEntry *)malloc(sizeof(SearchSortFacetEntry));

	result->count = 1;
	result->value = strndup(value, valueLength);

	return result;
}

static void facet_free(void *memory)
{
	SearchSortFacetEntry *entry = NULL;

	if(memory == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	entry = (SearchSortFacetEntry *)memory;

	if(entry->value != NULL) {
		free(entry->value);
	}

	free(entry);
}

static void facet_freeNothing(void *memory)
{
	if(memory == NULL) {
		DISPLAY_INVALID_ARGS;
	}
}

static void facet_updateIndex(SearchEngine *engine,
		IndexRegistryIndexSetting *settings, IndexRegistryTypes type,
		char *attribute, int attributeLength, int uid, Bptree *index)
{
	int ii = 0;
	int rc = 0;
	int tokenCount = 0;
	int valueLength = 0;
	int stringLength = 0;
	int stringResultLength = 0;
	int esaStringLength = 0;
	int *tokenLengths = NULL;
	char *value = NULL;
	char *string = NULL;
	char *esaString = NULL;
	char *excludedWord = NULL;
	char **tokenList = NULL;

	SearchSortFacetEntry *entry = NULL;
	Container *container = NULL;

	if((uid <= 0) ||
			((container = searchEngine_get(engine, uid, &rc)) == NULL)) {
		return;
	}

	value = container_getString(container, attribute, strlen(attribute),
			&valueLength);

	searchEngine_unlockGet(engine, container);

	if((value == NULL) || (valueLength < 1)) {
		if(value != NULL) {
			free(value);
		}
		return;
	}

	if(settings->isIndexFullString) {
		do {
			if((valueLength < settings->minStringLength) ||
					(valueLength > settings->maxStringLength)) {
				break;
			}

			stringLength = valueLength;
			string = strndup(value, valueLength);

			if(searchEngineCommon_normalizeString(string, stringLength,
						&stringResultLength) < 0) {
				free(string);
				break;
			}

			stringLength = stringResultLength;

			if(stringLength < settings->minStringLength) {
				free(string);
				break;
			}

			if((cstring_containsVowel(string, stringLength)) ||
					(cstring_containsAlphabetic(string, stringLength))) {
				if(bptree_get(settings->excludedWords, string, stringLength,
							((void *)&excludedWord)) == 0) {
					free(string);
					break;
				}

				if(type == INDEX_REGISTRY_TYPE_WILDCARD) {
					if((esaString = esa_stemToken(&engine->esa, string,
									stringLength, &esaStringLength)) != NULL) {
						if((esaStringLength >= settings->minStringLength) &&
								(esaStringLength <=
								 settings->maxStringLength)) {
							if(bptree_get(settings->excludedWords, esaString,
										esaStringLength,
										((void *)&excludedWord)) == 0) {
								free(esaString);
								free(string);
								break;
							}
						}

						free(esaString);
					}
				}
			}

			free(string);

			if(bptree_get(index, value, valueLength, (void *)&entry) < 0) {
				entry = facet_new(value, valueLength);

				if(bptree_put(index, value, valueLength, (void *)entry) < 0) {
					facet_free(entry);
				}
			}
			else {
				entry->count += 1;
			}
		} while(afalse);
	}

	if(settings->isIndexTokenizedString) {
		// tokenize string

		if(strtotokens(value,
					valueLength,
					settings->delimiters,
					settings->delimitersLength,
					settings->minStringLength,
					&tokenList,
					&tokenLengths,
					&tokenCount) < 0) {
			free(value);
			return;
		}

		if((tokenList == NULL) || (tokenLengths == NULL) || (tokenCount < 1) ||
				((settings->isIndexFullString) &&
				 (tokenCount == 1) &&
				 (tokenList != NULL) &&
				 (!strcmp(tokenList[0], value)))) {
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

			free(value);

			return;
		}

		// iterate token list & update indexes

		for(ii = 0; ii < tokenCount; ii++) {
			if(tokenList[ii] == NULL) {
				continue;
			}

			if((tokenLengths[ii] < settings->minStringLength) ||
					(tokenLengths[ii] > settings->maxStringLength)) {
				break;
			}

			stringLength = tokenLengths[ii];
			string = strndup(tokenList[ii], tokenLengths[ii]);

			if(searchEngineCommon_normalizeString(string, stringLength,
						&stringResultLength) < 0) {
				free(string);
				break;
			}

			stringLength = stringResultLength;

			if(stringLength < settings->minStringLength) {
				free(string);
				break;
			}

			if((cstring_containsVowel(string, stringLength)) ||
					(cstring_containsAlphabetic(string, stringLength))) {
				if(bptree_get(settings->excludedWords, string, stringLength,
							((void *)&excludedWord)) == 0) {
					free(string);
					break;
				}

				if(type == INDEX_REGISTRY_TYPE_WILDCARD) {
					if((esaString = esa_stemToken(&engine->esa, string,
									stringLength, &esaStringLength)) != NULL) {
						if((esaStringLength >= settings->minStringLength) &&
								(esaStringLength <=
								 settings->maxStringLength)) {
							if(bptree_get(settings->excludedWords, esaString,
										esaStringLength,
										((void *)&excludedWord)) == 0) {
								free(esaString);
								free(string);
								break;
							}
						}

						free(esaString);
					}
				}
			}

			free(string);

			if(bptree_get(index, tokenList[ii], tokenLengths[ii],
						(void *)&entry) < 0) {
				entry = facet_new(tokenList[ii], tokenLengths[ii]);

				if(bptree_put(index, tokenList[ii], tokenLengths[ii],
							(void *)entry) < 0) {
					facet_free(entry);
				}
			}
			else {
				entry->count += 1;
			}

			free(tokenList[ii]);
		}

		free(tokenList);
		free(tokenLengths);
	}

	// cleanup

	free(value);
}


// define search sort public functions

// direct sort functions

int searchSort_sortByAttributeName(SearchEngine *engine, char *attribute,
		Intersect *intersect, aboolean isStandardOrder)
{
	int ii = 0;

	AttributeSort *sortArray = NULL;

	if((engine == NULL) || (intersect == NULL) || (attribute == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(intersect->result.length < 1) {
		return 0; // nothing to sort
	}

	sortArray = (AttributeSort *)malloc(sizeof(AttributeSort) *
			intersect->result.length);

	for(ii = 0; ii < intersect->result.length; ii++) {
		attribute_buildEntry(engine,
				attribute,
				intersect->result.array[ii],
				&(sortArray[ii]));
	}

	if(isStandardOrder) {
		qsort((void *)sortArray,
				intersect->result.length,
				(int)sizeof(AttributeSort),
				attribute_compareDescending);
	}
	else {
		qsort((void *)sortArray,
				intersect->result.length,
				(int)sizeof(AttributeSort),
				attribute_compareAscending);
	}

	for(ii = 0; ii < intersect->result.length; ii++) {
		intersect->result.array[ii] = sortArray[ii].uid;

		if(sortArray[ii].value != NULL) {
			free(sortArray[ii].value);
		}
	}

	free(sortArray);

	return 0;
}

int searchSort_sortByRelevancy(SearchEngine *engine, char *attribute,
		char *value, Intersect *intersect, aboolean isStandardOrder)
{
	int ii = 0;
	int tokenCount = 0;
	int *tokenLengths = NULL;
	char *relevancyKey = NULL;
	char *relevancyLengthKey = NULL;
	char **tokenList = NULL;

	IndexRegistryIndex *indexType = NULL;
	RelevancySort *relevancySort = NULL;

	if((engine == NULL) || (attribute == NULL) || (value == NULL) ||
			(intersect == NULL) || (intersect->result.array == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(intersect->result.length < 1) {
		return 0; // nothing to sort
	}

	// determine if an index exists for this attribute

	if((indexType = relevancy_doesIndexExists(engine, attribute)) == NULL) {
		return -1;
	}

	// tokenize search string

	tokenList = searchEngineCommon_stringToTokens(
			engine->settings.minStringLength,
			engine->settings.delimiters,
			engine->settings.delimitersLength,
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
				"failed to tokenize sort-string '%s'", value);

		return -1;
	}

	// construct sort context

	relevancyKey = searchEngineCommon_buildRelevancyKey(indexType->key,
			indexType->keyLength);

	relevancyLengthKey = searchEngineCommon_buildRelevancyLengthKey(
			indexType->key, indexType->keyLength);

	relevancySort = (RelevancySort *)malloc(sizeof(RelevancySort) *
			intersect->result.length);

	for(ii = 0; ii < intersect->result.length; ii++) {
		relevancy_buildEntry(engine,
				tokenList,
				tokenCount,
				relevancyKey,
				relevancyLengthKey,
				intersect->result.array[ii],
				&(relevancySort[ii]));
	}

	// execute sort

	if(isStandardOrder) {
		qsort((void *)relevancySort,
				intersect->result.length,
				(int)sizeof(RelevancySort),
				relevancy_compareDescending);
	}
	else {
		qsort((void *)relevancySort,
				intersect->result.length,
				(int)sizeof(RelevancySort),
				relevancy_compareAscending);
	}

	// assign sort to intersection results

	for(ii = 0; ii < intersect->result.length; ii++) {
		intersect->result.array[ii] = relevancySort[ii].uid;
	}

	// cleanup

	if(tokenList != NULL) {
		for(ii = 0; ii < tokenCount; ii++) {
			if(tokenList[ii] != NULL) {
				free(tokenList[ii]);
			}
		}

		free(tokenList);
	}

	if(tokenLengths != NULL) {
		free(tokenLengths);
	}

	free(relevancyKey);

	free(relevancyLengthKey);

	free(relevancySort);

	return 0;
}

int searchSort_sortByDistance(SearchEngine *engine, double latitude,
		double longitude, Intersect *intersect, aboolean isStandardOrder)
{
	int ii = 0;
	double dValueOne = 0.0;
	double dValueTwo = 0.0;

	DistanceSort *distanceSort = NULL;

	if((engine == NULL) ||
			(latitude < -90.0) || (latitude > 90.0) ||
			(longitude < -180.0) || (longitude > 180.0) ||
			(intersect == NULL) || (intersect->result.array == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(intersect->result.length < 1) {
		return 0; // nothing to sort
	}

	// construct sort context

	distanceSort = (DistanceSort *)malloc(
			sizeof(DistanceSort) * intersect->result.length);

	for(ii = 0; ii < intersect->result.length; ii++) {
		distanceSort[ii].uid = intersect->result.array[ii];

		searchSort_determineGeoCoordsOnEntity(engine,
				latitude,
				longitude,
				ii,
				intersect->result.array,
				intersect->result.length,
				&dValueOne,
				&dValueTwo,
				&(distanceSort[ii].distance));
	}

	// execute sort

	if(isStandardOrder) {
		qsort((void *)distanceSort,
				intersect->result.length,
				(int)sizeof(DistanceSort),
				distance_compareDescending);
	}
	else {
		qsort((void *)distanceSort,
				intersect->result.length,
				(int)sizeof(DistanceSort),
				distance_compareAscending);
	}

	// assign sort to intersection results

	for(ii = 0; ii < intersect->result.length; ii++) {
		intersect->result.array[ii] = distanceSort[ii].uid;
	}

	// cleanup

	free(distanceSort);

	return 0;
}

// multi-value sort functions

int searchSort_mvInit(SearchSort *sort, SearchEngine *engine,
		Intersect *intersect)
{
	int ii = 0;

	if((sort == NULL) || (engine == NULL) || (intersect == NULL) ||
			(intersect->result.array == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	memset(sort, 0, (int)(sizeof(SearchSort)));

	sort->length = 0;
	sort->configLength = 0;
	sort->config = NULL;
	sort->entries = NULL;
	sort->intersect = intersect;
	sort->engine = engine;

	if(intersect->result.length < 1) {
		return 0; // nothing to sort
	}

	sort->length = intersect->result.length;
	sort->entries = (SearchSortEntry *)malloc(sizeof(SearchSortEntry) *
			sort->length);

	for(ii = 0; ii < sort->length; ii++) {
		sort->entries[ii].uid = intersect->result.array[ii];
		sort->entries[ii].context = (void *)sort;
		sort->entries[ii].items = NULL;
	}

	return 0;
}

int searchSort_mvFree(SearchSort *sort)
{
	int ii = 0;
	int nn = 0;

	AttributeSort *item = NULL;

	if(sort == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(sort->entries != NULL) {
		for(ii = 0; ii < sort->length; ii++) {
			if(sort->entries[ii].items != NULL) {
				for(nn = 0; nn < sort->configLength; nn++) {
					if(sort->entries[ii].items[nn] != NULL) {
						if((sort->config != NULL) &&
								(sort->config[nn].type ==
								 SEARCH_SORT_TYPE_ATTRIBUTE)) {
							item = (AttributeSort *)sort->entries[ii].items[nn];

							if(item->value != NULL) {
								free(item->value);
							}
						}

						free(sort->entries[ii].items[nn]);
					}
				}

				free(sort->entries[ii].items);
			}
		}

		free(sort->entries);
	}

	if(sort->config != NULL) {
		free(sort->config);
	}

	memset(sort, 0, (int)(sizeof(SearchSort)));

	return 0;
}

int searchSort_mvAddSortByAttributeName(SearchSort *sort, char *attribute,
		aboolean isStandardOrder)
{
	int ii = 0;
	int id = 0;

	AttributeSort *sortItem = NULL;

	if((sort == NULL) || (attribute == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(sort->length < 1) {
		return 0; // nothing to sort
	}

	if(sort->config == NULL) {
		id = 0;
		sort->configLength = 1;
		sort->config = (SearchSortConfig *)malloc(sizeof(SearchSortConfig) *
				sort->configLength);
	}
	else {
		id = sort->configLength;
		sort->configLength += 1;
		sort->config = (SearchSortConfig *)realloc(sort->config,
				(sizeof(SearchSortConfig) * sort->configLength));
	}

	sort->config[id].isStandardOrder = isStandardOrder;
	sort->config[id].type = SEARCH_SORT_TYPE_ATTRIBUTE;

	for(ii = 0; ii < sort->length; ii++) {
		if(id == 0) {
			sort->entries[ii].items = (void **)malloc(sizeof(void *) *
					sort->configLength);
		}
		else {
			sort->entries[ii].items = (void **)realloc(sort->entries[ii].items,
					(sizeof(void *) * sort->configLength));
		}

		sortItem = (AttributeSort *)malloc(sizeof(AttributeSort));

		attribute_buildEntry(sort->engine,
				attribute,
				sort->entries[ii].uid,
				sortItem);

		sort->entries[ii].items[id] = (void *)sortItem;
	}

	return 0;
}

int searchSort_mvAddSortByRelevancy(SearchSort *sort, char *attribute,
		char *value, aboolean isStandardOrder)
{
	int ii = 0;
	int id = 0;
	int tokenCount = 0;
	int *tokenLengths = NULL;
	char *relevancyKey = NULL;
	char *relevancyLengthKey = NULL;
	char **tokenList = NULL;

	IndexRegistryIndex *indexType = NULL;
	RelevancySort *relevancyItem = NULL;

	if((sort == NULL) || (attribute == NULL) || (value == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(sort->length < 1) {
		return 0; // nothing to sort
	}

	// determine if an index exists for this attribute

	if((indexType = relevancy_doesIndexExists(sort->engine,
					attribute)) == NULL) {
		return -1;
	}

	// tokenize search string

	tokenList = searchEngineCommon_stringToTokens(
			sort->engine->settings.minStringLength,
			sort->engine->settings.delimiters,
			sort->engine->settings.delimitersLength,
			&sort->engine->esa,
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

		log_logf(sort->engine->log, LOG_LEVEL_ERROR,
				"failed to tokenize sort-string '%s'", value);

		return -1;
	}

	// update sort configuration

	if(sort->config == NULL) {
		id = 0;
		sort->configLength = 1;
		sort->config = (SearchSortConfig *)malloc(sizeof(SearchSortConfig) *
				sort->configLength);
	}
	else {
		id = sort->configLength;
		sort->configLength += 1;
		sort->config = (SearchSortConfig *)realloc(sort->config,
				(sizeof(SearchSortConfig) * sort->configLength));
	}

	sort->config[id].isStandardOrder = isStandardOrder;
	sort->config[id].type = SEARCH_SORT_TYPE_RELEVANCY;

	// construct sort context

	relevancyKey = searchEngineCommon_buildRelevancyKey(indexType->key,
			indexType->keyLength);

	relevancyLengthKey = searchEngineCommon_buildRelevancyLengthKey(
			indexType->key, indexType->keyLength);

	for(ii = 0; ii < sort->length; ii++) {
		if(id == 0) {
			sort->entries[ii].items = (void **)malloc(sizeof(void *) *
					sort->configLength);
		}
		else {
			sort->entries[ii].items = (void **)realloc(sort->entries[ii].items,
					(sizeof(void *) * sort->configLength));
		}

		relevancyItem = (RelevancySort *)malloc(sizeof(RelevancySort));

		relevancy_buildEntry(sort->engine,
				tokenList,
				tokenCount,
				relevancyKey,
				relevancyLengthKey,
				sort->entries[ii].uid,
				relevancyItem);

		sort->entries[ii].items[id] = (void *)relevancyItem;
	}

	// cleanup

	if(tokenList != NULL) {
		for(ii = 0; ii < tokenCount; ii++) {
			if(tokenList[ii] != NULL) {
				free(tokenList[ii]);
			}
		}

		free(tokenList);
	}

	if(tokenLengths != NULL) {
		free(tokenLengths);
	}

	free(relevancyKey);

	free(relevancyLengthKey);

	return 0;
}

int searchSort_mvAddSortByDistance(SearchEngine *engine, SearchSort *sort,
		double latitude, double longitude, aboolean isStandardOrder)
{
	int ii = 0;
	int id = 0;
	double dValueOne = 0.0;
	double dValueTwo = 0.0;

	DistanceSort *distanceItem = NULL;

	if((engine == NULL) || (sort == NULL) ||
			(latitude < -90.0) || (latitude > 90.0) ||
			(longitude < -180.0) || (longitude > 180.0)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(sort->length < 1) {
		return 0; // nothing to sort
	}

	// update sort configuration

	if(sort->config == NULL) {
		id = 0;
		sort->configLength = 1;
		sort->config = (SearchSortConfig *)malloc(sizeof(SearchSortConfig) *
				sort->configLength);
	}
	else {
		id = sort->configLength;
		sort->configLength += 1;
		sort->config = (SearchSortConfig *)realloc(sort->config,
				(sizeof(SearchSortConfig) * sort->configLength));
	}

	sort->config[id].isStandardOrder = isStandardOrder;
	sort->config[id].type = SEARCH_SORT_TYPE_DISTANCE;

	// construct sort context

	for(ii = 0; ii < sort->length; ii++) {
		if(id == 0) {
			sort->entries[ii].items = (void **)malloc(sizeof(void *) *
					sort->configLength);
		}
		else {
			sort->entries[ii].items = (void **)realloc(sort->entries[ii].items,
					(sizeof(void *) * sort->configLength));
		}

		distanceItem = (DistanceSort *)malloc(sizeof(DistanceSort));

		distanceItem->uid = sort->entries[ii].uid;

		searchSort_determineGeoCoordsOnEntity(engine,
				latitude,
				longitude,
				ii,
				sort->intersect->result.array,
				sort->intersect->result.length,
				&dValueOne,
				&dValueTwo,
				&(distanceItem->distance));

		sort->entries[ii].items[id] = (void *)distanceItem;
	}

	return 0;
}

int searchSort_mvExecute(SearchSort *sort)
{
	int ii = 0;

	if(sort == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(sort->length < 1) {
		return 0; // nothing to sort
	}

	// execute multi-value sort

	qsort((void *)sort->entries,
			sort->length,
			(int)sizeof(SearchSortEntry),
			multiValue_compare);

	// assign sort to intersection results

	for(ii = 0; ii < sort->intersect->result.length; ii++) {
		sort->intersect->result.array[ii] = sort->entries[ii].uid;
	}

	return 0;
}

// facet functions

int searchSort_calculateFacets(SearchEngine *engine, Intersect *intersect,
		char *attribute, SearchSortFacetEntry ***facets)
{
	int ii = 0;
	int rc = 0;
	int resultLength = 0;
	int attributeLength = 0;
	int *keyLengths = NULL;
	char **keys = NULL;
	void **entries = NULL;

	Bptree index;
	IndexRegistryIndex *indexType = NULL;
	IndexRegistryIndexSetting settings;

	if((engine == NULL) || (intersect == NULL) || (attribute == NULL) ||
			(facets == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	*facets = NULL;

	// check attribute length, and if the attribute is a reserved value

	if((attributeLength = strlen(attribute)) < 1) {
		return 0;
	}

	if((!strcasecmp(attribute, SEARCHD_DOMAIN_KEY)) ||
			(!strcasecmp(attribute, SEARCHD_CONTAINER_UID)) ||
			(!strcasecmp(attribute, SEARCHD_CONTAINER_NAME)) ||
			(!strcasecmp(attribute, "uid"))) {
		return 0;
	}

	// obtain the index type, and create a settings object for this facet

	if((indexType = searchEngine_getIndex(engine, attribute, &rc)) == NULL) {
		return rc;
	}

	memset(&settings, 0, (int)(sizeof(IndexRegistryIndexSetting)));

	if(indexType->settings.isOverrideStringLengths) {
		settings.minStringLength = indexType->settings.minStringLength;
		settings.maxStringLength = indexType->settings.maxStringLength;
	}
	else {
		settings.minStringLength = engine->settings.minStringLength;
		settings.maxStringLength = engine->settings.maxStringLength;
	}

	if(indexType->settings.isOverrideDelimiters) {
		settings.delimiters = indexType->settings.delimiters;
		settings.delimitersLength = indexType->settings.delimitersLength;
	}
	else {
		settings.delimiters = engine->settings.delimiters;
		settings.delimitersLength = engine->settings.delimitersLength;
	}

	if(indexType->settings.isOverrideExcludedWords) {
		settings.excludedWords = indexType->settings.excludedWords;
	}
	else {
		settings.excludedWords = &(engine->excludedWords);
	}

	if(indexType->settings.isOverrideIndexing) {
		settings.isIndexFullString = indexType->settings.isIndexFullString;
		settings.isIndexTokenizedString =
			indexType->settings.isIndexTokenizedString;
	}
	else {
		settings.isIndexFullString = atrue;

		if(indexType->type == INDEX_REGISTRY_TYPE_USER_KEY) {
			settings.isIndexTokenizedString = afalse;
		}
		else {
			settings.isIndexTokenizedString = atrue;
		}
	}

	// build index of facets

	bptree_init(&index);
	bptree_setFreeFunction(&index, facet_free);

	for(ii = 0; ii < intersect->result.length; ii++) {
		facet_updateIndex(engine, &settings, indexType->type, attribute,
				attributeLength, intersect->result.array[ii], &index);
	}

	// convert the facets in the index into an array of facets

	if(bptree_toArray(&index,
				&resultLength,
				&keyLengths,
				&keys,
				(void ***)&entries) < 0) {
		bptree_free(&index);
		return 0;
	}

	*facets = (SearchSortFacetEntry **)entries;

	// cleanup

	if(keys != NULL) {
		for(ii = 0; ii < resultLength; ii++) {
			if(keys[ii] != NULL) {
				free(keys[ii]);
			}
		}

		free(keys);
	}

	if(keyLengths != NULL) {
		free(keyLengths);
	}

	bptree_setFreeFunction(&index, facet_freeNothing);
	bptree_free(&index);

	return resultLength;
}

// helper functions

alint searchSort_calculateMvInitMemoryLength(Intersect *intersect)
{
	alint result = 0;

	if(intersect == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	result = ((alint)intersect->result.length *
			(alint)sizeof(SearchSortEntry));

	return result;
}

alint searchSort_calculateMvAddSortByAttributeNameMemoryLength(
		SearchSort *sort, char *attribute, alint memoryLengthLimit)
{
	int ii = 0;
	alint result = 0;

	if((sort == NULL) || (attribute == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	result += (alint)sizeof(SearchSortConfig);

	result += ((alint)sort->length * (alint)sizeof(void *));

	result += ((alint)sort->length * (alint)sizeof(AttributeSort));

	if(result >= memoryLengthLimit) {
		return result;
	}

	for(ii = 0; ii < sort->length; ii++) {
		result += attribute_calculateBuildEntryMemoryLength(sort->engine,
				attribute,
				sort->entries[ii].uid);
	}

	return result;
}

alint searchSort_calculateMvAddSortByRelevancyMemoryLength(SearchSort *sort)
{
	alint result = 0;

	if(sort == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	result += (alint)sizeof(SearchSortConfig);

	result += ((alint)sort->length * (alint)sizeof(void *));

	result += ((alint)sort->length * (alint)sizeof(RelevancySort));

	return result;
}

alint searchSort_calculateMvAddSortByDistanceMemoryLength(SearchSort *sort)
{
	alint result = 0;

	if(sort == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	result += (alint)sizeof(SearchSortConfig);

	result += ((alint)sort->length * (alint)sizeof(void *));

	result += ((alint)sort->length * (alint)sizeof(DistanceSort));

	return result;
}

double searchSort_calculateGeoCoordDistanceMiles(double latitudeOne,
		double longitudeOne, double latitudeTwo, double longitudeTwo)
{
	double result = 0.0;

	result = (acos(
				cos(geocoord_degreesToRadians(latitudeOne)) *
				cos(geocoord_degreesToRadians(longitudeOne)) *
				cos(geocoord_degreesToRadians(latitudeTwo)) *
				cos(geocoord_degreesToRadians(longitudeTwo)) +
				cos(geocoord_degreesToRadians(latitudeOne)) *
				sin(geocoord_degreesToRadians(longitudeOne)) *
				cos(geocoord_degreesToRadians(latitudeTwo)) *
				sin(geocoord_degreesToRadians(longitudeTwo)) +
				sin(geocoord_degreesToRadians(latitudeOne)) *
				sin(geocoord_degreesToRadians(latitudeTwo))) *
			GEOCOORD_RADIUS_OF_EARTH_IN_MILES);

	if((result == 0.0) && (latitudeOne == latitudeTwo) &&
			(longitudeOne == longitudeTwo)) {
		result = 0.000001;
	}

	return result;
}

aboolean searchSort_determineGeoCoordsOnEntity(SearchEngine *engine,
		double latitude, double longitude,
		int uidListId, int *uidList, int uidListLength,
		double *resultLatitude, double *resultLongitude,
		double *resultDistance)
{
	int ii = 0;
	int rc = 0;
	int uid = 0;
	int nameLength = 0;
	int failCounter = 0;
	int duplicateCounter = 0;
	int geoCoordListRef = 0;
	int geoCoordListLength = 0;
	double localDistance = 0.0;
	double localLatitude = 0.0;
	double localLongitude = 0.0;
	char name[128];

	Container *container = NULL;
	EntityGeoCoord *geoCoordList = NULL;

	if((engine == NULL) || (uidListId < 0) || (uidList == NULL) ||
			(uidListLength < 1) || (uidListId >= uidListLength) ||
			(resultLatitude == NULL) || (resultLongitude == NULL) ||
			(resultDistance == NULL)) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	*resultLatitude = 0.0;
	*resultLongitude = 0.0;
	*resultDistance = 0.0;

	// assign UID from list position

	uid = uidList[uidListId];

	// obtain container from search engine

	if((container = searchEngine_get(engine, uid, &rc)) == NULL) {
		return afalse;
	}

	// scan UID list for duplicates

	if(uidListId > 0) {
		for(ii = 0; ii < uidListId; ii++) {
			if(uidList[ii] == uid) {
				duplicateCounter++;
			}
		}
	}

	// determine if standard latitude/longtitude entries exist

	if((container_exists(container, "latitude", 8)) &&
			(container_exists(container, "longitude", 9))) {
		localLatitude = container_getDouble(container, "latitude", 8);
		localLongitude = container_getDouble(container, "longitude", 9);

		// calculate distance

		localDistance = searchSort_calculateGeoCoordDistanceMiles(
				latitude, longitude, localLatitude, localLongitude);

		// add entry to geocoord list

		if(geoCoordList == NULL) {
			geoCoordListRef = 0;
			geoCoordListLength = 1;
			geoCoordList = (EntityGeoCoord *)malloc(sizeof(EntityGeoCoord) *
					geoCoordListLength);
		}
		else {
			geoCoordListRef = geoCoordListLength;
			geoCoordListLength += 1;
			geoCoordList = (EntityGeoCoord *)realloc(geoCoordList,
					(sizeof(EntityGeoCoord) * geoCoordListLength));
		}

		geoCoordList[geoCoordListRef].uid = uid;
		geoCoordList[geoCoordListRef].distance = localDistance;
		geoCoordList[geoCoordListRef].latitude = localLatitude;
		geoCoordList[geoCoordListRef].longitude = localLongitude;
	}

	// check to see if additional long/lat exist

	for(ii = 0, failCounter = 0; ((ii < 1000) && (failCounter < 3)); ii++) {
		nameLength = snprintf(name, ((int)sizeof(name) - 1),
				"latitude%03i", ii);

		if(!container_exists(container, name, nameLength)) {
			failCounter++;
			continue;;
		}

		localLatitude = container_getDouble(container, name, nameLength);

		nameLength = snprintf(name, ((int)sizeof(name) - 1),
				"longitude%03i", ii);

		if(!container_exists(container, name, nameLength)) {
			failCounter++;
			continue;;
		}

		localLongitude = container_getDouble(container, name, nameLength);

		// calculate distance

		localDistance = searchSort_calculateGeoCoordDistanceMiles(latitude,
				longitude, localLatitude, localLongitude);

		// add entry to geocoord list

		if(geoCoordList == NULL) {
			geoCoordListRef = 0;
			geoCoordListLength = 1;
			geoCoordList = (EntityGeoCoord *)malloc(sizeof(EntityGeoCoord) *
					geoCoordListLength);
		}
		else {
			geoCoordListRef = geoCoordListLength;
			geoCoordListLength += 1;
			geoCoordList = (EntityGeoCoord *)realloc(geoCoordList,
					(sizeof(EntityGeoCoord) * geoCoordListLength));
		}

		geoCoordList[geoCoordListRef].uid = uid;
		geoCoordList[geoCoordListRef].distance = localDistance;
		geoCoordList[geoCoordListRef].latitude = localLatitude;
		geoCoordList[geoCoordListRef].longitude = localLongitude;
	}

	searchEngine_unlockGet(engine, container);

	// determine if there are any entries in the geocoord list

	if(geoCoordList == NULL) {
		return afalse;
	}

	// sort geocoord list

	qsort((void *)geoCoordList,
			geoCoordListLength,
			(int)(sizeof(EntityGeoCoord)),
			entityGeoCoord_compareAscending);

	// assign the appropriate result

	if(duplicateCounter >= geoCoordListLength) {
		*resultLatitude = geoCoordList[geoCoordListRef].latitude;
		*resultLongitude = geoCoordList[geoCoordListRef].longitude;
		*resultDistance = geoCoordList[geoCoordListRef].distance;
	}
	else {
		*resultLatitude = geoCoordList[duplicateCounter].latitude;
		*resultLongitude = geoCoordList[duplicateCounter].longitude;
		*resultDistance = geoCoordList[duplicateCounter].distance;
	}
//printf("%s) :: UID #%i from (%0.6f, %0.6f) { %i, 0x%lx, %i } results { dup count %i, dist %0.6f, lat/long (%0.6f, %0.6f) } from list { ref %i, length %i, ptr 0x%lx }\n", __FUNCTION__, uid, latitude, longitude, uidListId, (aptrcast)uidList, uidListLength, duplicateCounter, *resultDistance, *resultLatitude, *resultLongitude, geoCoordListRef, geoCoordListLength, (aptrcast)geoCoordList); fflush(NULL);

	// cleanup

	free(geoCoordList);

	return atrue;
}

