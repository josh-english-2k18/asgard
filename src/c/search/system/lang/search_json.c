/*
 * search_json.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The JSON language component for the search-engine query compiler.
 *
 * Written by Josh English.
 */

// preprocessor directives

#define _CORE_COMMON_BYPASS_STD_STREAM

#include "core/core.h"
#include "search/search.h"
#define _SEARCH_SYSTEM_LANG_COMPONENT
#include "search/system/lang/search_json.h"


// delcare search compiler json language private functions

static aboolean checkJsonParameter(SearchCompiler *compiler, Json *object,
		JsonValueType valueType, char *elementName,
		SearchJsonValidationType validationType, const char *TEMPLATE);

static void buildIndexSummary(SearchEngine *engine, char *domainKey,
		char *requestAttribute, Json *object);

static void buildDisplayFacets(SearchEngine *engine, char *domainKey,
		char *indexKey, Json *object);

static void buildSearchResultFacets(SearchActionSearch *action,
		char *attribute, Json *object);

static aboolean compileSearchCondition(SearchCompiler *compiler,
		SearchActionSearch *action, Json *condition,
		SearchJsonValidationType validationType,
		SearchActionSearchCondition *searchCondition,
		aboolean isRootCondition);


// define search compiler json language private functions

static aboolean checkJsonParameter(SearchCompiler *compiler, Json *object,
		JsonValueType valueType, char *elementName,
		SearchJsonValidationType validationType, const char *TEMPLATE)
{
	JsonValueType rcValueType = JSON_VALUE_TYPE_UNKNOWN;
	char buffer[1024];

	if((object == NULL) || (elementName == NULL)) {
		return afalse;
	}

	if(!json_elementExists(object, elementName)) {
		snprintf(buffer, (int)(sizeof(buffer) - 1),
				"failed to locate '%s' parameter", elementName);

		searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1, buffer);

		return afalse;
	}

	if(validationType == SEARCH_JSON_VALIDATION_TYPE_IGNORE) {
		return atrue;
	}

	if((rcValueType = json_getElementType(object, elementName)) != valueType) {
		snprintf(buffer, (int)(sizeof(buffer) - 1),
				"json '%s' parameter invalid type, found '%s' expecting '%s'",
				elementName,
				json_valueTypeToString(rcValueType),
				json_valueTypeToString(valueType));

		if(validationType == SEARCH_JSON_VALIDATION_TYPE_STRICT) {
			searchCompiler_buildErrorMessage(compiler, TEMPLATE,
					SEARCH_COMPILER_ERROR_JSON_COMPILE_INVALID_PARAMETER,
					buffer);
			return afalse;
		}

		searchCompiler_buildErrorMessage(compiler, TEMPLATE, 0, buffer);
	}

	return atrue;
}

static void buildIndexSummary(SearchEngine *engine, char *domainKey,
		char *requestAttribute, Json *object)
{
	Json *entry = NULL;

	entry = json_new();

	json_addString(entry, "domainName", domainKey);

	json_addString(entry, "indexKey", requestAttribute);

	json_addNumber(entry, "indexUniqueKeysCount",
			searchEngine_getIndexCount(engine, domainKey, requestAttribute));

	json_addNumber(entry, "indexUniqueValuesCount",
			searchEngine_getIndexUniqueValueCount(engine, domainKey,
				requestAttribute));

	json_addNumber(entry, "indexBptreeDepth",
			searchEngine_getIndexDepth(engine, domainKey, requestAttribute));

	json_addObjectToArray(object, "indexSummary", entry);
}

static void buildDisplayFacets(SearchEngine *engine, char *domainKey,
		char *indexKey, Json *object)
{
	aboolean isFirstTime = atrue;
	int ii = 0;

	Json *entry = NULL;
	SearchIndexSummary summary;

	if(searchEngine_getIndexUniqueValueSummary(engine, domainKey,
				indexKey, &summary) < 0) {
		return;
	}

	for(ii = 0; ii < summary.length; ii++) {
		if(summary.keys[ii] == NULL) {
			continue;
		}

		if(isFirstTime) {
			json_addArray(object, "facets");
			isFirstTime = afalse;
		}

		entry = json_new();

		json_addString(entry, "value", summary.keys[ii]);
		json_addNumber(entry, "count", summary.valueCounts[ii]);

		json_addObjectToArray(object, "facets", entry);

		free(summary.keys[ii]);
	}

	free(summary.valueCounts);
	free(summary.type);
	free(summary.keys);
}

static void buildSearchResultFacets(SearchActionSearch *action,
		char *attribute, Json *object)
{
	aboolean isFirstTime = atrue;
	int ii = 0;
	int facetCount = 0;

	Json *entry = NULL;
	SearchSortFacetEntry **facets = NULL;

	if((facetCount = searchSort_calculateFacets(action->engine,
					&action->intersect,
					attribute,
					&facets)) < 0) {
		return;
	}

	if(facets == NULL) {
		return;
	}

	for(ii = 0; ii < facetCount; ii++) {
		if(facets[ii] == NULL) {
			continue;
		}
		else if(facets[ii]->value == NULL) {
			free(facets[ii]);
			continue;
		}

		if(isFirstTime) {
			json_addArray(object, "facets");
			isFirstTime = afalse;
		}

		entry = json_new();

		json_addString(entry, "attribute", attribute);
		json_addString(entry, "value", facets[ii]->value);
		json_addNumber(entry, "facetCount", facets[ii]->count);

		json_addObjectToArray(object, "facets", entry);

		free(facets[ii]->value);
		free(facets[ii]);
	}

	free(facets);
}

static aboolean compileSearchCondition(SearchCompiler *compiler,
		SearchActionSearch *action, Json *condition,
		SearchJsonValidationType validationType,
		SearchActionSearchCondition *searchCondition,
		aboolean isRootCondition)
{
	char *string = NULL;

	const char *TEMPLATE = "failed to create search (condition) with '%s'";

	if(!checkJsonParameter(compiler, condition, JSON_VALUE_TYPE_STRING,
				"type", validationType, TEMPLATE)) {
		return afalse;
	}
	else if((string = json_getString(condition, "type")) == NULL) {
		searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
				"failed to obtain (condition) 'type' parameter");
		return afalse;
	}

	if(!strcasecmp(string, "wildcard")) {
		searchCondition->type =
			SEARCH_COMPILER_SEARCH_CONDITION_TYPE_WILDCARD;
	}
	else if(!strcasecmp(string, "exact")) {
		searchCondition->type =
			SEARCH_COMPILER_SEARCH_CONDITION_TYPE_EXACT;
	}
	else if(!strcasecmp(string, "user")) {
		searchCondition->type =
			SEARCH_COMPILER_SEARCH_CONDITION_TYPE_USER;
	}
	else if(!strcasecmp(string, "range")) {
		searchCondition->type =
			SEARCH_COMPILER_SEARCH_CONDITION_TYPE_RANGE;

		if(!checkJsonParameter(compiler, condition, JSON_VALUE_TYPE_STRING,
					"rangeType", validationType, TEMPLATE)) {
			return afalse;
		}
		else if((string = json_getString(condition, "rangeType")) == NULL) {
			searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
					"failed to obtain (condition) 'rangeType' parameter");
			return afalse;
		}

		if(!strcasecmp(string, "greater-than")) {
			searchCondition->rangeType =
				RANGE_INDEX_SEARCH_GREATER_THAN;
		}
		else if(!strcasecmp(string, "less-than")) {
			searchCondition->rangeType =
				RANGE_INDEX_SEARCH_LESS_THAN;
		}
		else if(!strcasecmp(string, "between")) {
			searchCondition->rangeType =
				RANGE_INDEX_SEARCH_BETWEEN;
		}
		else {
			searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
					"unknown (condition) 'rangeType' parameter");
			return afalse;
		}
	}
	else if(!strcasecmp(string, "geocoord")) {
		searchCondition->type =
			SEARCH_COMPILER_SEARCH_CONDITION_TYPE_GEO_COORD;

		if(!checkJsonParameter(compiler, condition, JSON_VALUE_TYPE_STRING,
					"geoCoordType", validationType, TEMPLATE)) {
			return afalse;
		}
		else if((string = json_getString(condition, "geoCoordType")) == NULL) {
			searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
					"failed to obtain (condition) 'geoCoordType' "
					"parameter");
			return afalse;
		}

		if(!strcasecmp(string, "miles")) {
			searchCondition->geoCoordType =
				SEARCH_ENGINE_GEO_COORD_TYPE_MILES;
		}
		else if(!strcasecmp(string, "yards")) {
			searchCondition->geoCoordType =
				SEARCH_ENGINE_GEO_COORD_TYPE_YARDS;
		}
		else if(!strcasecmp(string, "feet")) {
			searchCondition->geoCoordType =
				SEARCH_ENGINE_GEO_COORD_TYPE_FEET;
		}
		else if(!strcasecmp(string, "kilometers")) {
			searchCondition->geoCoordType =
				SEARCH_ENGINE_GEO_COORD_TYPE_KILOMETERS;
		}
		else if(!strcasecmp(string, "meters")) {
			searchCondition->geoCoordType =
				SEARCH_ENGINE_GEO_COORD_TYPE_METERS;
		}
		else {
			searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
					"unknown (condition) 'geoCoordType' parameter");
			return afalse;
		}
	}
	else {
		searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
				"unknown (condition) 'type' parameter");
		return afalse;
	}

	switch(searchCondition->type) {
		case SEARCH_COMPILER_SEARCH_CONDITION_TYPE_WILDCARD:
		case SEARCH_COMPILER_SEARCH_CONDITION_TYPE_EXACT:
		case SEARCH_COMPILER_SEARCH_CONDITION_TYPE_USER:
			if(!checkJsonParameter(compiler, condition, JSON_VALUE_TYPE_STRING,
						"attribute", validationType, TEMPLATE)) {
				return afalse;
			}
			else if((string = json_getString(condition, "attribute")) == NULL) {
				searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
						"failed to obtain (condition) 'attribute' "
						"parameter");
				return afalse;
			}

			searchCondition->attribute = strdup(string);

			if(!checkJsonParameter(compiler, condition, JSON_VALUE_TYPE_STRING,
						"value", validationType, TEMPLATE)) {
				return afalse;
			}
			else if((string = json_getString(condition, "value")) == NULL) {
				searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
						"failed to obtain (condition) 'value' parameter");
				return afalse;
			}

			searchCondition->value = strdup(string);
			break;

		case SEARCH_COMPILER_SEARCH_CONDITION_TYPE_RANGE:
			if(!checkJsonParameter(compiler, condition, JSON_VALUE_TYPE_STRING,
						"attribute", validationType, TEMPLATE)) {
				return afalse;
			}
			else if((string = json_getString(condition, "attribute")) == NULL) {
				searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
						"failed to obtain (condition) 'attribute' "
						"parameter");
				return afalse;
			}

			searchCondition->attribute = strdup(string);

			if(!checkJsonParameter(compiler, condition, JSON_VALUE_TYPE_NUMBER,
						"minValue", validationType, TEMPLATE)) {
				return afalse;
			}

			searchCondition->minValue = (int)json_getNumber(
					condition, "minValue");

			if(!checkJsonParameter(compiler, condition, JSON_VALUE_TYPE_NUMBER,
						"maxValue", validationType, TEMPLATE)) {
				return afalse;
			}

			searchCondition->maxValue = (int)json_getNumber(
					condition, "maxValue");
			break;

		case SEARCH_COMPILER_SEARCH_CONDITION_TYPE_GEO_COORD:
			if(!checkJsonParameter(compiler, condition, JSON_VALUE_TYPE_NUMBER,
						"latitude", validationType, TEMPLATE)) {
				return afalse;
			}

			searchCondition->latitude = json_getNumber(condition,
					"latitude");

			if(!checkJsonParameter(compiler, condition, JSON_VALUE_TYPE_NUMBER,
						"longitude", validationType, TEMPLATE)) {
				return afalse;
			}

			searchCondition->longitude = json_getNumber(condition,
					"longitude");

			if(!checkJsonParameter(compiler, condition, JSON_VALUE_TYPE_NUMBER,
						"distance", validationType, TEMPLATE)) {
				return afalse;
			}

			searchCondition->distance = json_getNumber(condition,
					"distance");
			break;

		case SEARCH_COMPILER_SEARCH_CONDITION_TYPE_ERROR:
		default:
				searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
						"detected invalid condition type");
				return afalse;
	}

	if((isRootCondition) &&
			((string = json_getString(condition, "intersect")) != NULL) &&
			(!strcasecmp(string, "or"))) {
		action->settings.isAndIntersection = afalse;
	}

	if(json_elementExists(condition, "isNotEqualTo")) {
		if(!checkJsonParameter(compiler, condition, JSON_VALUE_TYPE_BOOLEAN,
					"isNotEqualTo", validationType, TEMPLATE)) {
			return afalse;
		}

		searchCondition->isNotEqualTo =
			json_getBoolean(condition, "isNotEqualTo");
	}
	else {
		searchCondition->isNotEqualTo = afalse;
	}

	return atrue;
}


// define search compiler json language public functions

char *searchJson_validationTypeToString(SearchJsonValidationType type)
{
	char *result = "unknown";

	switch(type) {
		case SEARCH_JSON_VALIDATION_TYPE_IGNORE:
			result = "ignore";
			break;

		case SEARCH_JSON_VALIDATION_TYPE_WARNING:
			result = "warning";
			break;

		case SEARCH_JSON_VALIDATION_TYPE_STRICT:
			result = "strict";
			break;

		case SEARCH_JSON_VALIDATION_TYPE_UNKNOWN:
		default:
			result = "unknown";
	}

	return result;
}

SearchJsonValidationType searchJson_stringToValidationType(char *string)
{
	SearchJsonValidationType result = SEARCH_JSON_VALIDATION_TYPE_UNKNOWN;

	if(string == NULL) {
		DISPLAY_INVALID_ARGS;
		return SEARCH_JSON_VALIDATION_TYPE_UNKNOWN;
	}

	if(!strcasecmp(string, "ignore")) {
		result = SEARCH_JSON_VALIDATION_TYPE_IGNORE;
	}
	else if(!strcasecmp(string, "warning")) {
		result = SEARCH_JSON_VALIDATION_TYPE_WARNING;
	}
	else if(!strcasecmp(string, "strict")) {
		result = SEARCH_JSON_VALIDATION_TYPE_STRICT;
	}

	return result;
}

aboolean searchJson_compileNewDomain(SearchCompiler *compiler,
		SearchJsonValidationType validationType, Json *message)
{
	char *domainKey = NULL;
	char *domainName = NULL;

	SearchActionNewDomain *action = NULL;

	const char *TEMPLATE = "failed to create new domain with '%s'";

	if((compiler == NULL) || (message == NULL)) {
		DISPLAY_INVALID_ARGS;
		if(compiler != NULL) {
			searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
					"invalid or missing argument(s)");
		}
		return afalse;
	}

	if(!checkJsonParameter(compiler, message, JSON_VALUE_TYPE_STRING,
				"domainKey", validationType, TEMPLATE)) {
		return afalse;
	}
	else if((domainKey = json_getString(message, "domainKey")) == NULL) {
		searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
				"failed to obtain 'domainKey' parameter");
		return afalse;
	}

	if(!checkJsonParameter(compiler, message, JSON_VALUE_TYPE_STRING,
				"domainName", validationType, TEMPLATE)) {
		return afalse;
	}
	else if((domainName = json_getString(message, "domainName")) == NULL) {
		searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
				"failed to obtain 'domainName' parameter");
		return afalse;
	}

	action = (SearchActionNewDomain *)malloc(sizeof(SearchActionNewDomain));

	action->key = strdup(domainKey);
	action->name = strdup(domainName);

	compiler->type = SEARCH_COMPILER_ACTION_NEW_DOMAIN;
	compiler->action = (void *)action;

	return atrue;
}

Json *searchJson_buildNewDomainResult(SearchCompiler *compiler)
{
	char *string = NULL;

	Json *result = NULL;

	if((compiler == NULL) ||
			(compiler->type != SEARCH_COMPILER_ACTION_NEW_DOMAIN)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	result = json_new();

	json_addString(result, "messageName", "newDomainResponse");

	if(compiler->errorCode != 0) {
		string = itoa(compiler->errorCode);

		json_addString(result, "resultCode", string);

		free(string);

		if(compiler->errorMessage != NULL) {
			json_addString(result, "errorMessage", compiler->errorMessage);
		}
		else if(!compiler->isCompileComplete) {
			json_addString(result, "errorMessage",
					"compile failed to complete");
		}
		else if(!compiler->isExecutionComplete) {
			json_addString(result, "errorMessage",
					"execution failed to complete");
		}
	}
	else {
		json_addString(result, "resultCode", "0");
	}

	return result;
}

aboolean searchJson_compileNewIndex(SearchCompiler *compiler,
		SearchJsonValidationType validationType, Json *message)
{
	int indexType = 0;
	char *string = NULL;
	char *indexKey = NULL;
	char *indexName = NULL;

	SearchActionNewIndex *action = NULL;

	const char *TEMPLATE = "failed to create new index with '%s'";

	if((compiler == NULL) || (message == NULL)) {
		DISPLAY_INVALID_ARGS;
		if(compiler != NULL) {
			searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
					"invalid or missing argument(s)");
		}
		return afalse;
	}

	if(!json_elementExists(message, "indexType")) {
		searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
				"failed to locate 'indexType' parameter");
		return afalse;
	}
	else {
		if((string = json_getString(message, "indexType")) == NULL) {
			indexType = (int)json_getNumber(message, "indexType");

			if((indexType != INDEX_REGISTRY_TYPE_EXACT) &&
					(indexType != INDEX_REGISTRY_TYPE_WILDCARD) &&
					(indexType != INDEX_REGISTRY_TYPE_RANGE) &&
					(indexType != INDEX_REGISTRY_TYPE_USER_KEY)) {
				searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
						"unrecognized 'indexType' parameter");
				return afalse;
			}
		}
		else if(!strcasecmp(string, "exact")) {
			indexType = INDEX_REGISTRY_TYPE_EXACT;
		}
		else if(!strcasecmp(string, "wildcard")) {
			indexType = INDEX_REGISTRY_TYPE_WILDCARD;
		}
		else if(!strcasecmp(string, "range")) {
			indexType = INDEX_REGISTRY_TYPE_RANGE;
		}
		else if(!strcasecmp(string, "user")) {
			indexType = INDEX_REGISTRY_TYPE_USER_KEY;
		}
		else {
			indexType = (int)json_getNumber(message, "indexType");

			if((indexType != INDEX_REGISTRY_TYPE_EXACT) &&
					(indexType != INDEX_REGISTRY_TYPE_WILDCARD) &&
					(indexType != INDEX_REGISTRY_TYPE_RANGE) &&
					(indexType != INDEX_REGISTRY_TYPE_USER_KEY)) {
				searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
						"unrecognized 'indexType' parameter");
				return afalse;
			}
		}
	}

	if(!checkJsonParameter(compiler, message, JSON_VALUE_TYPE_STRING,
				"indexKey", validationType, TEMPLATE)) {
		return afalse;
	}
	else if((indexKey = json_getString(message, "indexKey")) == NULL) {
		searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
				"failed to obtain 'indexKey' parameter");
		return afalse;
	}

	if(!checkJsonParameter(compiler, message, JSON_VALUE_TYPE_STRING,
				"indexName", validationType, TEMPLATE)) {
		return afalse;
	}
	else if((indexName = json_getString(message, "indexName")) == NULL) {
		searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
				"failed to obtain 'indexName' parameter");
		return afalse;
	}

	action = (SearchActionNewIndex *)malloc(sizeof(SearchActionNewIndex));

	action->type = indexType;
	action->key = strdup(indexKey);
	action->name = strdup(indexName);

	compiler->type = SEARCH_COMPILER_ACTION_NEW_INDEX;
	compiler->action = (void *)action;

	return atrue;
}

Json *searchJson_buildNewIndexResult(SearchCompiler *compiler)
{
	char *string = NULL;

	Json *result = NULL;

	if((compiler == NULL) ||
			(compiler->type != SEARCH_COMPILER_ACTION_NEW_INDEX)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	result = json_new();

	json_addString(result, "messageName", "newIndexResponse");

	if(compiler->errorCode != 0) {
		string = itoa(compiler->errorCode);

		json_addString(result, "resultCode", string);

		free(string);

		if(compiler->errorMessage != NULL) {
			json_addString(result, "errorMessage", compiler->errorMessage);
		}
		else if(!compiler->isCompileComplete) {
			json_addString(result, "errorMessage",
					"compile failed to complete");
		}
		else if(!compiler->isExecutionComplete) {
			json_addString(result, "errorMessage",
					"execution failed to complete");
		}
	}
	else {
		json_addString(result, "resultCode", "0");
	}

	return result;
}

aboolean searchJson_compileGet(SearchCompiler *compiler,
		SearchJsonValidationType validationType, Json *message)
{
	int uid = 0;

	SearchActionGet *action = NULL;

	const char *TEMPLATE = "failed to create get with '%s'";

	if((compiler == NULL) || (message == NULL)) {
		DISPLAY_INVALID_ARGS;
		if(compiler != NULL) {
			searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
					"invalid or missing argument(s)");
		}
		return afalse;
	}

	if(!json_elementExists(message, "uid")) {
		if(!searchJson_compileGetByAttribute(compiler, validationType,
					message)) {
			searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
					"failed to locate 'uid' parameter");
			return afalse;
		}
		return atrue;
	}

	if(!checkJsonParameter(compiler, message, JSON_VALUE_TYPE_NUMBER,
				"uid", validationType, TEMPLATE)) {
		return afalse;
	}

	uid = (int)json_getNumber(message, "uid");

	action = (SearchActionGet *)malloc(sizeof(SearchActionGet));

	action->uid = uid;

	compiler->type = SEARCH_COMPILER_ACTION_GET;
	compiler->action = (void *)action;

	return atrue;
}

Json *searchJson_buildGetResult(SearchCompiler *compiler)
{
	char *string = NULL;

	Json *result = NULL;
	Json *containerObject = NULL;
	SearchActionGet *action = NULL;

	if((compiler == NULL) ||
			(compiler->type != SEARCH_COMPILER_ACTION_GET)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	action = (SearchActionGet *)compiler->action;

	result = json_new();

	json_addString(result, "messageName", "getResponse");

	if(compiler->errorCode != 0) {
		string = itoa(compiler->errorCode);

		json_addString(result, "resultCode", string);

		free(string);

		if(compiler->errorMessage != NULL) {
			json_addString(result, "errorMessage", compiler->errorMessage);
		}
		else if(!compiler->isCompileComplete) {
			json_addString(result, "errorMessage",
					"compile failed to complete");
		}
		else if(!compiler->isExecutionComplete) {
			json_addString(result, "errorMessage",
					"execution failed to complete");
		}
	}
	else {
		json_addString(result, "resultCode", "0");

		if((containerObject = container_containerToJson(
						action->container)) == NULL) {
			containerObject = json_new();
		}

		json_addObject(result, "payload", containerObject);
	}

	return result;
}

aboolean searchJson_compileGetByAttribute(SearchCompiler *compiler,
		SearchJsonValidationType validationType, Json *message)
{
	char *attribute = NULL;
	char *value = NULL;

	SearchActionGet *getAction = NULL;
	SearchActionGetByAttribute *action = NULL;

	const char *TEMPLATE = "failed to create get by attribute with '%s'";

	if((compiler == NULL) || (message == NULL)) {
		DISPLAY_INVALID_ARGS;
		if(compiler != NULL) {
			searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
					"invalid or missing argument(s)");
		}
		return afalse;
	}

	if(!checkJsonParameter(compiler, message, JSON_VALUE_TYPE_STRING,
				"attribute", validationType, TEMPLATE)) {
		return afalse;
	}
	else if((attribute = json_getString(message, "attribute")) == NULL) {
		searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
				"failed to obtain 'attribute' parameter");
		return afalse;
	}

	if(!checkJsonParameter(compiler, message, JSON_VALUE_TYPE_STRING,
				"value", validationType, TEMPLATE)) {
		return afalse;
	}
	else if((value = json_getString(message, "value")) == NULL) {
		searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
				"failed to obtain 'value' parameter");
		return afalse;
	}

	if(!strcasecmp(attribute, "uid")) {
		getAction = (SearchActionGet *)malloc(sizeof(SearchActionGet));

		getAction->uid = atoi(value);

		compiler->type = SEARCH_COMPILER_ACTION_GET;
		compiler->action = (void *)getAction;

		return atrue;
	}

	action = (SearchActionGetByAttribute *)malloc(
			sizeof(SearchActionGetByAttribute));

	action->attribute = strdup(attribute);
	action->value = strdup(value);

	compiler->type = SEARCH_COMPILER_ACTION_GET_BY_ATTRIBUTE;
	compiler->action = (void *)action;

	return atrue;
}

Json *searchJson_buildGetByAttributeResult(SearchCompiler *compiler)
{
	char *string = NULL;

	Json *result = NULL;
	Json *containerObject = NULL;
	SearchActionGetByAttribute *action = NULL;

	if((compiler == NULL) ||
			(compiler->type != SEARCH_COMPILER_ACTION_GET_BY_ATTRIBUTE)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	action = (SearchActionGetByAttribute *)compiler->action;

	result = json_new();

	json_addString(result, "messageName", "getResponse");

	if(compiler->errorCode != 0) {
		string = itoa(compiler->errorCode);

		json_addString(result, "resultCode", string);

		free(string);

		if(compiler->errorMessage != NULL) {
			json_addString(result, "errorMessage", compiler->errorMessage);
		}
		else if(!compiler->isCompileComplete) {
			json_addString(result, "errorMessage",
					"compile failed to complete");
		}
		else if(!compiler->isExecutionComplete) {
			json_addString(result, "errorMessage",
					"execution failed to complete");
		}
	}
	else {
		json_addString(result, "resultCode", "0");

		if((containerObject = container_containerToJson(
						action->container)) == NULL) {
			containerObject = json_new();
		}

		json_addObject(result, "payload", containerObject);
	}

	return result;
}

aboolean searchJson_compilePut(SearchCompiler *compiler,
		SearchJsonValidationType validationType, Json *message)
{
	aboolean isImmediate = atrue;
	char *domainKey = NULL;

	Json *payload = NULL;
	Container *container = NULL;
	SearchActionPut *action = NULL;

	const char *TEMPLATE = "failed to create put with '%s'";

	if((compiler == NULL) || (message == NULL)) {
		DISPLAY_INVALID_ARGS;
		if(compiler != NULL) {
			searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
					"invalid or missing argument(s)");
		}
		return afalse;
	}

	if(json_elementExists(message, "isImmediate")) {
		if(!checkJsonParameter(compiler, message, JSON_VALUE_TYPE_BOOLEAN,
					"isImmediate", validationType, TEMPLATE)) {
			return afalse;
		}
		isImmediate = json_getBoolean(message, "isImmediate");
	}

	if(!checkJsonParameter(compiler, message, JSON_VALUE_TYPE_STRING,
				"domainKey", validationType, TEMPLATE)) {
		return afalse;
	}
	else if((domainKey = json_getString(message, "domainKey")) == NULL) {
		searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
				"failed to obtain 'domainKey' parameter");
		return afalse;
	}

	if(!checkJsonParameter(compiler, message, JSON_VALUE_TYPE_OBJECT,
				"payload", validationType, TEMPLATE)) {
		return afalse;
	}
	else if((payload = json_getObject(message, "payload")) == NULL) {
		searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
				"failed to obtain 'payload' parameter");
		return afalse;
	}
	else if((container = container_jsonToContainer(payload)) == NULL) {
		searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
				"failed to decode 'payload' object");
		return afalse;
	}

	action = (SearchActionPut *)malloc(sizeof(SearchActionPut));

	action->isImmediate = isImmediate;
	action->domainKey = strdup(domainKey);
	action->container = container;

	compiler->type = SEARCH_COMPILER_ACTION_PUT;
	compiler->action = (void *)action;

	return atrue;
}

Json *searchJson_buildPutResult(SearchCompiler *compiler)
{
	char *string = NULL;

	Json *result = NULL;
	SearchActionPut *action = NULL;

	if((compiler == NULL) ||
			(compiler->type != SEARCH_COMPILER_ACTION_PUT)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	action = (SearchActionPut *)compiler->action;

	result = json_new();

	json_addString(result, "messageName", "putResponse");

	if(compiler->errorCode != 0) {
		string = itoa(compiler->errorCode);

		json_addString(result, "resultCode", string);

		free(string);

		if(compiler->errorMessage != NULL) {
			json_addString(result, "errorMessage", compiler->errorMessage);
		}
		else if(!compiler->isCompileComplete) {
			json_addString(result, "errorMessage",
					"compile failed to complete");
		}
		else if(!compiler->isExecutionComplete) {
			json_addString(result, "errorMessage",
					"execution failed to complete");
		}
	}
	else {
		json_addString(result, "resultCode", "0");
		json_addNumber(result, "uid", (double)action->uid);
	}

	return result;
}

aboolean searchJson_compileUpdate(SearchCompiler *compiler,
		SearchJsonValidationType validationType, Json *message)
{
	aboolean isImmediate = atrue;
	char flags = (char)0;
	char *domainKey = NULL;
	char *messageName = NULL;

	Json *payload = NULL;
	Container *container = NULL;
	SearchActionUpdate *action = NULL;

	const char *TEMPLATE = "failed to create update with '%s'";

	if((compiler == NULL) || (message == NULL)) {
		DISPLAY_INVALID_ARGS;
		if(compiler != NULL) {
			searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
					"invalid or missing argument(s)");
		}
		return afalse;
	}

	if(json_elementExists(message, "attribute")) {
		return searchJson_compileUpdateByAttribute(compiler, validationType,
				message);
	}

	if((messageName = json_getString(message, "messageName")) == NULL) {
		searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
				"failed to obtain 'messageName' parameter");
		return afalse;
	}

	if(!strcasecmp(messageName, "update")) {
		flags = CONTAINER_FLAG_UPDATE;
	}
	else if(!strcasecmp(messageName, "append")) {
		flags = CONTAINER_FLAG_APPEND;
	}
	else if(!strcasecmp(messageName, "uppend")) {
		flags = CONTAINER_FLAG_UPPEND;
	}
	else if(!strcasecmp(messageName, "replace")) {
		flags = CONTAINER_FLAG_REPLACE;
	}
	else {
		flags = CONTAINER_FLAG_UPDATE;
	}

	if(json_elementExists(message, "isImmediate")) {
		if(!checkJsonParameter(compiler, message, JSON_VALUE_TYPE_BOOLEAN,
					"isImmediate", validationType, TEMPLATE)) {
			return afalse;
		}
		isImmediate = json_getBoolean(message, "isImmediate");
	}

	if(!checkJsonParameter(compiler, message, JSON_VALUE_TYPE_STRING,
				"domainKey", validationType, TEMPLATE)) {
		return afalse;
	}
	else if((domainKey = json_getString(message, "domainKey")) == NULL) {
		searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
				"failed to obtain 'domainKey' parameter");
		return afalse;
	}

	if(!checkJsonParameter(compiler, message, JSON_VALUE_TYPE_OBJECT,
				"payload", validationType, TEMPLATE)) {
		return afalse;
	}
	else if((payload = json_getObject(message, "payload")) == NULL) {
		searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
				"failed to obtain 'payload' parameter");
		return afalse;
	}
	else if((container = container_jsonToContainer(payload)) == NULL) {
		searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
				"failed to decode 'payload' object");
		return afalse;
	}

	action = (SearchActionUpdate *)malloc(sizeof(SearchActionUpdate));

	action->isImmediate = isImmediate;
	action->flags = flags;
	action->domainKey = strdup(domainKey);
	action->container = container;

	compiler->type = SEARCH_COMPILER_ACTION_UPDATE;
	compiler->action = (void *)action;

	return atrue;
}

Json *searchJson_buildUpdateResult(SearchCompiler *compiler)
{
	char *string = NULL;

	Json *result = NULL;
	SearchActionUpdate *action = NULL;

	if((compiler == NULL) ||
			(compiler->type != SEARCH_COMPILER_ACTION_UPDATE)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	action = (SearchActionUpdate *)compiler->action;

	result = json_new();

	switch(action->flags) {
		case CONTAINER_FLAG_UPDATE:
			json_addString(result, "messageName", "updateResponse");
			break;

		case CONTAINER_FLAG_APPEND:
			json_addString(result, "messageName", "appendResponse");
			break;

		case CONTAINER_FLAG_UPPEND:
			json_addString(result, "messageName", "uppendResponse");
			break;

		case CONTAINER_FLAG_REPLACE:
			json_addString(result, "messageName", "replaceResponse");
			break;

		default:
			json_addString(result, "messageName", "updateResponse");
	}

	if(compiler->errorCode != 0) {
		string = itoa(compiler->errorCode);

		json_addString(result, "resultCode", string);

		free(string);

		if(compiler->errorMessage != NULL) {
			json_addString(result, "errorMessage", compiler->errorMessage);
		}
		else if(!compiler->isCompileComplete) {
			json_addString(result, "errorMessage",
					"compile failed to complete");
		}
		else if(!compiler->isExecutionComplete) {
			json_addString(result, "errorMessage",
					"execution failed to complete");
		}
	}
	else {
		json_addString(result, "resultCode", "0");
		json_addNumber(result, "uid", (double)action->uid);
	}

	return result;
}

aboolean searchJson_compileUpdateByAttribute(SearchCompiler *compiler,
		SearchJsonValidationType validationType, Json *message)
{
	aboolean isImmediate = atrue;
	char flags = (char)0;
	int attributeLength = 0;
	char *value = NULL;
	char *attribute = NULL;
	char *domainKey = NULL;
	char *messageName = NULL;

	Json *payload = NULL;
	Container *container = NULL;
	SearchActionUpdate *updateAction = NULL;
	SearchActionUpdateByAttribute *action = NULL;

	const char *TEMPLATE = "failed to create update by attribute with '%s'";

	if((compiler == NULL) || (message == NULL)) {
		DISPLAY_INVALID_ARGS;
		if(compiler != NULL) {
			searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
					"invalid or missing argument(s)");
		}
		return afalse;
	}

	if((messageName = json_getString(message, "messageName")) == NULL) {
		searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
				"failed to obtain 'messageName' parameter");
		return afalse;
	}

	if(strcasestr(messageName, "update") != NULL) {
		flags = CONTAINER_FLAG_UPDATE;
	}
	else if(strcasestr(messageName, "append") != NULL) {
		flags = CONTAINER_FLAG_APPEND;
	}
	else if(strcasestr(messageName, "uppend") != NULL) {
		flags = CONTAINER_FLAG_UPPEND;
	}
	else if(strcasestr(messageName, "replace") != NULL) {
		flags = CONTAINER_FLAG_REPLACE;
	}
	else {
		flags = CONTAINER_FLAG_UPDATE;
	}

	if(json_elementExists(message, "isImmediate")) {
		if(!checkJsonParameter(compiler, message, JSON_VALUE_TYPE_BOOLEAN,
					"isImmediate", validationType, TEMPLATE)) {
			return afalse;
		}
		isImmediate = json_getBoolean(message, "isImmediate");
	}

	if(!checkJsonParameter(compiler, message, JSON_VALUE_TYPE_STRING,
				"attribute", validationType, TEMPLATE)) {
		return afalse;
	}
	else if((attribute = json_getString(message, "attribute")) == NULL) {
		searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
				"failed to obtain 'attribute' parameter");
		return afalse;
	}

	if(!checkJsonParameter(compiler, message, JSON_VALUE_TYPE_STRING,
				"domainKey", validationType, TEMPLATE)) {
		return afalse;
	}
	else if((domainKey = json_getString(message, "domainKey")) == NULL) {
		searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
				"failed to obtain 'domainKey' parameter");
		return afalse;
	}

	if(!checkJsonParameter(compiler, message, JSON_VALUE_TYPE_OBJECT,
				"payload", validationType, TEMPLATE)) {
		return afalse;
	}
	else if((payload = json_getObject(message, "payload")) == NULL) {
		searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
				"failed to obtain 'payload' parameter");
		return afalse;
	}
	else if((container = container_jsonToContainer(payload)) == NULL) {
		searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
				"failed to decode 'payload' object");
		return afalse;
	}

	if(json_elementExists(message, "value")) {
		if(!checkJsonParameter(compiler, message, JSON_VALUE_TYPE_STRING,
					"value", validationType, TEMPLATE)) {
			return afalse;
		}
		value = json_getString(message, "value");
	}

	attributeLength = strlen(attribute);

	if((value != NULL) && (!strcasecmp(attribute, "uid"))) {
		container_setUid(container, atoi(value));

		updateAction = (SearchActionUpdate *)malloc(sizeof(SearchActionUpdate));

		updateAction->isImmediate = isImmediate;
		updateAction->flags = flags;
		updateAction->domainKey = strdup(domainKey);
		updateAction->container = container;

		compiler->type = SEARCH_COMPILER_ACTION_UPDATE;
		compiler->action = (void *)updateAction;

		return atrue;
	}

	if(value != NULL) {
		if(!container_exists(container, attribute, attributeLength)) {
			container_putString(container, attribute, attributeLength, value,
					strlen(value));
		}
		else {
			searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
					"attribute already exists in payload");
		}
	}

	action = (SearchActionUpdateByAttribute *)malloc(
			sizeof(SearchActionUpdateByAttribute));

	action->isImmediate = isImmediate;
	action->flags = flags;
	action->attribute = strndup(attribute, attributeLength);
	action->domainKey = strdup(domainKey);
	action->container = container;

	compiler->type = SEARCH_COMPILER_ACTION_UPDATE_BY_ATTRIBUTE;
	compiler->action = (void *)action;

	return atrue;
}

Json *searchJson_buildUpdateByAttributeResult(SearchCompiler *compiler)
{
	char *string = NULL;

	Json *result = NULL;
	SearchActionUpdateByAttribute *action = NULL;

	if((compiler == NULL) ||
			(compiler->type != SEARCH_COMPILER_ACTION_UPDATE_BY_ATTRIBUTE)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	action = (SearchActionUpdateByAttribute *)compiler->action;

	result = json_new();

	switch(action->flags) {
		case CONTAINER_FLAG_UPDATE:
			json_addString(result, "messageName", "updateResponse");
			break;

		case CONTAINER_FLAG_APPEND:
			json_addString(result, "messageName", "appendResponse");
			break;

		case CONTAINER_FLAG_UPPEND:
			json_addString(result, "messageName", "uppendResponse");
			break;

		case CONTAINER_FLAG_REPLACE:
			json_addString(result, "messageName", "replaceResponse");
			break;

		default:
			json_addString(result, "messageName", "updateResponse");
	}

	if(compiler->errorCode != 0) {
		string = itoa(compiler->errorCode);

		json_addString(result, "resultCode", string);

		free(string);

		if(compiler->errorMessage != NULL) {
			json_addString(result, "errorMessage", compiler->errorMessage);
		}
		else if(!compiler->isCompileComplete) {
			json_addString(result, "errorMessage",
					"compile failed to complete");
		}
		else if(!compiler->isExecutionComplete) {
			json_addString(result, "errorMessage",
					"execution failed to complete");
		}
	}
	else {
		json_addString(result, "resultCode", "0");
		json_addNumber(result, "uid", (double)action->uid);
	}

	return result;
}

aboolean searchJson_compileDelete(SearchCompiler *compiler,
		SearchJsonValidationType validationType, Json *message)
{
	int uid = 0;

	SearchActionDelete *action = NULL;

	const char *TEMPLATE = "failed to create delete with '%s'";

	if((compiler == NULL) || (message == NULL)) {
		DISPLAY_INVALID_ARGS;
		if(compiler != NULL) {
			searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
					"invalid or missing argument(s)");
		}
		return afalse;
	}

	if(!json_elementExists(message, "uid")) {
		if(!searchJson_compileDeleteByAttribute(compiler, validationType,
					message)) {
			searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
					"failed to locate 'uid' parameter");
			return afalse;
		}
		return atrue;
	}

	if(!checkJsonParameter(compiler, message, JSON_VALUE_TYPE_NUMBER,
				"uid", validationType, TEMPLATE)) {
		return afalse;
	}

	uid = (int)json_getNumber(message, "uid");

	action = (SearchActionDelete *)malloc(sizeof(SearchActionDelete));

	action->uid = uid;

	compiler->type = SEARCH_COMPILER_ACTION_DELETE;
	compiler->action = (void *)action;

	return atrue;
}

Json *searchJson_buildDeleteResult(SearchCompiler *compiler)
{
	char *string = NULL;

	Json *result = NULL;

	if((compiler == NULL) ||
			(compiler->type != SEARCH_COMPILER_ACTION_DELETE)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	result = json_new();

	json_addString(result, "messageName", "deleteResponse");

	if(compiler->errorCode != 0) {
		string = itoa(compiler->errorCode);

		json_addString(result, "resultCode", string);

		free(string);

		if(compiler->errorMessage != NULL) {
			json_addString(result, "errorMessage", compiler->errorMessage);
		}
		else if(!compiler->isCompileComplete) {
			json_addString(result, "errorMessage",
					"compile failed to complete");
		}
		else if(!compiler->isExecutionComplete) {
			json_addString(result, "errorMessage",
					"execution failed to complete");
		}
	}
	else {
		json_addString(result, "resultCode", "0");
	}

	return result;
}

aboolean searchJson_compileDeleteByAttribute(SearchCompiler *compiler,
		SearchJsonValidationType validationType, Json *message)
{
	char *value = NULL;
	char *attribute = NULL;

	SearchActionDelete *deleteAction = NULL;
	SearchActionDeleteByAttribute *action = NULL;

	const char *TEMPLATE = "failed to create delete by attribute with '%s'";

	if((compiler == NULL) || (message == NULL)) {
		DISPLAY_INVALID_ARGS;
		if(compiler != NULL) {
			searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
					"invalid or missing argument(s)");
		}
		return afalse;
	}

	if(!checkJsonParameter(compiler, message, JSON_VALUE_TYPE_STRING,
				"attribute", validationType, TEMPLATE)) {
		return afalse;
	}
	else if((attribute = json_getString(message, "attribute")) == NULL) {
		searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
				"failed to obtain 'attribute' parameter");
		return afalse;
	}

	if(!checkJsonParameter(compiler, message, JSON_VALUE_TYPE_STRING,
				"value", validationType, TEMPLATE)) {
		return afalse;
	}
	else if((value = json_getString(message, "value")) == NULL) {
		searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
				"failed to obtain 'value' parameter");
		return afalse;
	}

	if(!strcasecmp(attribute, "uid")) {
		deleteAction = (SearchActionDelete *)malloc(sizeof(SearchActionDelete));

		deleteAction->uid = atoi(value);

		compiler->type = SEARCH_COMPILER_ACTION_DELETE;
		compiler->action = (void *)deleteAction;

		return atrue;
	}

	action = (SearchActionDeleteByAttribute *)malloc(
			sizeof(SearchActionDeleteByAttribute));

	action->uid = 0;
	action->attribute = strdup(attribute);
	action->value = strdup(value);

	compiler->type = SEARCH_COMPILER_ACTION_DELETE_BY_ATTRIBUTE;
	compiler->action = (void *)action;

	return atrue;
}

Json *searchJson_buildDeleteByAttributeResult(SearchCompiler *compiler)
{
	char *string = NULL;

	Json *result = NULL;
	SearchActionDeleteByAttribute *action = NULL;

	if((compiler == NULL) ||
			(compiler->type != SEARCH_COMPILER_ACTION_DELETE_BY_ATTRIBUTE)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	action = (SearchActionDeleteByAttribute *)compiler->action;

	result = json_new();

	json_addString(result, "messageName", "deleteResponse");

	if(compiler->errorCode != 0) {
		string = itoa(compiler->errorCode);

		json_addString(result, "resultCode", string);

		free(string);

		if(compiler->errorMessage != NULL) {
			json_addString(result, "errorMessage", compiler->errorMessage);
		}
		else if(!compiler->isCompileComplete) {
			json_addString(result, "errorMessage",
					"compile failed to complete");
		}
		else if(!compiler->isExecutionComplete) {
			json_addString(result, "errorMessage",
					"execution failed to complete");
		}
	}
	else {
		json_addString(result, "resultCode", "0");
		json_addNumber(result, "uid", (double)action->uid);
	}

	return result;
}

aboolean searchJson_compileSearch(SearchCompiler *compiler,
		SearchJsonValidationType validationType, Json *message)
{
	aboolean hasGroups = afalse;
	aboolean isIncludeDistances = afalse;
	int ii = 0;
	int nn = 0;
	int jj = 0;
	int groupId = 0;
	int rootConditionsLength = 0;
	char *string = NULL;

	Json *condition = NULL;
	Json *groupCondition = NULL;
	Json *sortOptions = NULL;
	Json *sortDirective = NULL;
	Json *resultOptions = NULL;
	Json *facetOptions = NULL;
	SearchActionSearch *action = NULL;

	const char *TEMPLATE = "failed to create search with '%s'";

	if((compiler == NULL) || (message == NULL)) {
		DISPLAY_INVALID_ARGS;
		if(compiler != NULL) {
			searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
					"invalid or missing argument(s)");
		}
		return afalse;
	}

	action = (SearchActionSearch *)malloc(sizeof(SearchActionSearch));

	compiler->action = (void *)action;

	// determine if primary required attributes exist, and check them

	if(json_elementExists(message, "requestAttributes")) {
		if(!checkJsonParameter(compiler, message, JSON_VALUE_TYPE_ARRAY,
					"requestAttributes", validationType, TEMPLATE)) {
			return afalse;
		}

		action->requestAttributesLength = json_getArrayLength(message,
				"requestAttributes");

		if(action->requestAttributesLength < 0) {
			searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
					"failed to obtain valid value for 'requestAttributes' "
					"array length");
			return afalse;
		}
		else if(action->requestAttributesLength < 1) {
			action->requestAttributesLength = 0;
		}
	}

	if(!checkJsonParameter(compiler, message, JSON_VALUE_TYPE_ARRAY,
				"domainKeys", validationType, TEMPLATE)) {
		return afalse;
	}
	else if((action->domainKeysLength = json_getArrayLength(message,
					"domainKeys")) < 1) {
		searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
				"failed to obtain value 'domainKeys' array length");
		return afalse;
	}

	if(!checkJsonParameter(compiler, message, JSON_VALUE_TYPE_ARRAY,
				"conditions", validationType, TEMPLATE)) {
		return afalse;
	}
	else if((rootConditionsLength = json_getArrayLength(message,
					"conditions")) < 1) {
		searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
				"failed to obtain value 'conditions' array length");
		return afalse;
	}

	// allocate & initialize request attributes

	if(action->requestAttributesLength > 0) {
		action->requestAttributes = (char **)malloc(sizeof(char *) *
				action->requestAttributesLength);

		for(ii = 0; ii < action->requestAttributesLength; ii++) {
			if((string = json_getStringFromArray(message,
							"requestAttributes", ii)) == NULL) {
				searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
						"failed to locate value in 'requestAttributes' array");
				return afalse;
			}

			action->requestAttributes[ii] = strdup(string);

			if((!action->settings.isEntireContainer) &&
					(!strcasecmp(action->requestAttributes[ii], "*"))) {
				action->settings.isEntireContainer = atrue;
			}
		}
	}

	// allocate & initialize domain keys

	action->domainKeys = (char **)malloc(sizeof(char *) *
			action->domainKeysLength);

	for(ii = 0; ii < action->domainKeysLength; ii++) {
		if((string = json_getStringFromArray(message,
						"domainKeys", ii)) == NULL) {
			searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
					"failed to locate value in 'domainKeys' array");
			return afalse;
		}

		action->domainKeys[ii] = strdup(string);

		if((!action->settings.isGlobalSearch) &&
				(!strcasecmp(action->domainKeys[ii], "*"))) {
			action->settings.isGlobalSearch = atrue;
		}
	}

	// allocate & initialize search conditions

	action->settings.isAndIntersection = atrue;
	action->groupsLength = 0;

	for(ii = 0; ii < rootConditionsLength; ii++) {
		if((condition = json_getObjectFromArray(message,
						"conditions", ii)) == NULL) {
			searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
					"failed to locate value in 'conditions' array");
			return afalse;
		}

		if(!checkJsonParameter(compiler, condition, JSON_VALUE_TYPE_STRING,
					"type", validationType, TEMPLATE)) {
			return afalse;
		}

		if((string = json_getString(condition, "type")) == NULL) {
			searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
					"failed to obtain (condition) 'type' parameter");
			return afalse;
		}

		if(!strcasecmp(string, "group")) {
			hasGroups = atrue;
			break;
		}
	}

	if(hasGroups) {
		action->groupsLength = rootConditionsLength;
	}
	else {
		action->groupsLength = 1;
	}

	action->groups = (SearchActionSearchConditionGroup *)malloc(
			sizeof(SearchActionSearchConditionGroup) * action->groupsLength);

	if(!hasGroups) {
		action->groups[0].isAndIntersection = atrue;
		action->groups[0].conditionsLength = rootConditionsLength;
		action->groups[0].conditions = (SearchActionSearchCondition *)malloc(
			sizeof(SearchActionSearchCondition) *
			action->groups[0].conditionsLength);
	}

	for(ii = 0, groupId = 0; ii < rootConditionsLength; ii++) {
		if((condition = json_getObjectFromArray(message,
						"conditions", ii)) == NULL) {
			searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
					"failed to locate value in 'conditions' array");
			return afalse;
		}

		if(hasGroups) {
			if(groupId >= action->groupsLength) {
				searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
						"aborting on detection of invalid group length");
				return afalse;
			}

			action->groups[groupId].isAndIntersection = atrue;

			if(!checkJsonParameter(compiler, condition, JSON_VALUE_TYPE_STRING,
						"type", validationType, TEMPLATE)) {
				return afalse;
			}

			if((string = json_getString(condition, "type")) == NULL) {
				searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
						"failed to obtain (condition) 'type' parameter");
				return afalse;
			}

			if(!strcasecmp(string, "groupOptions")) {
				if(((string = json_getString(condition,
									"intersect")) != NULL) &&
						(!strcasecmp(string, "or"))) {
					action->settings.isAndIntersection = afalse;
				}
				else {
					action->settings.isAndIntersection = atrue;
				}
				continue;
			}
			else if(strcasecmp(string, "group")) {
				action->groups[groupId].conditionsLength = 1;
				action->groups[groupId].conditions =
					(SearchActionSearchCondition *)malloc(
							sizeof(SearchActionSearchCondition) *
							action->groups[groupId].conditionsLength);

				if(!compileSearchCondition(compiler, action, condition,
							validationType,
							&action->groups[groupId].conditions[0], atrue)) {
					return afalse;
				}

				groupId++;
				continue;
			}

			if(((string = json_getString(condition, "intersect")) != NULL) &&
					(!strcasecmp(string, "or"))) {
				action->groups[groupId].isAndIntersection = afalse;
			}

			if(!checkJsonParameter(compiler, condition, JSON_VALUE_TYPE_ARRAY,
						"conditions", validationType, TEMPLATE)) {
				return afalse;
			}

			if((action->groups[groupId].conditionsLength = json_getArrayLength(
							condition,
							"conditions")) < 1) {
				searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
						"failed to obtain value 'conditions' array length");
				return afalse;
			}

			action->groups[groupId].conditions =
				(SearchActionSearchCondition *)malloc(
						sizeof(SearchActionSearchCondition) *
						action->groups[groupId].conditionsLength);

			for(nn = 0; nn < action->groups[groupId].conditionsLength; nn++) {
				if((groupCondition = json_getObjectFromArray(condition,
								"conditions", nn)) == NULL) {
					searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
							"failed to locate value in 'conditions' array");
					return afalse;
				}

				if(!compileSearchCondition(compiler, action, groupCondition,
							validationType,
							&action->groups[groupId].conditions[nn], afalse)) {
					return afalse;
				}
			}

			groupId++;
		}
		else {
			if(!compileSearchCondition(compiler, action, condition,
						validationType,
						&action->groups[0].conditions[ii], atrue)) {
				return afalse;
			}

			if(((string = json_getString(condition, "intersect")) != NULL) &&
					(!strcasecmp(string, "or"))) {
				action->groups[0].isAndIntersection = afalse;
			}
		}
	}

	// determine if sorting options were specified

	if((sortOptions = json_getObject(message, "sortOptions")) != NULL) {
		action->sort.isSorted = atrue;

		if(((string = json_getString(sortOptions, "type")) != NULL) &&
				(!strcasecmp(string, "multivalue"))) {
			if(!checkJsonParameter(compiler, sortOptions, JSON_VALUE_TYPE_ARRAY,
						"directives", validationType, TEMPLATE)) {
				return afalse;
			}

			action->sort.directiveLength = json_getArrayLength(sortOptions,
					"directives");

			sortDirective = json_getObjectFromArray(sortOptions,
					"directives", 0);
		}
		else {
			action->sort.directiveLength = 1;
			sortDirective = sortOptions;
		}

		action->sort.directives = (SearchActionSearchSortDirective *)malloc(
				sizeof(SearchActionSearchSortDirective) *
				action->sort.directiveLength);

		ii = 0;

		do {
			if(sortDirective == NULL) {
				break;
			}

			if(json_elementExists(sortDirective, "direction")) {
				if(!checkJsonParameter(compiler, sortDirective,
							JSON_VALUE_TYPE_STRING, "direction",
							validationType, TEMPLATE)) {
					return afalse;
				}
			}

			if(((string = json_getString(sortDirective,
								"direction")) != NULL) &&
					(!strcasecmp(string, "asc"))) {
				action->sort.directives[ii].isStandardSortOrder = afalse;
			}
			else {
				action->sort.directives[ii].isStandardSortOrder = atrue;
			}
	
			action->sort.directives[ii].type =
				SEARCH_COMPILER_SEARCH_SORT_TYPE_UNKNOWN;

			if(json_elementExists(sortDirective, "type")) {
				if(!checkJsonParameter(compiler, sortDirective,
							JSON_VALUE_TYPE_STRING, "type",
							validationType, TEMPLATE)) {
					return afalse;
				}
			}
	
			if((string = json_getString(sortDirective, "type")) != NULL) {
				if(!strcasecmp(string, "attribute")) {
					if(!checkJsonParameter(compiler, sortDirective,
								JSON_VALUE_TYPE_STRING, "attribute",
								validationType, TEMPLATE)) {
						return afalse;
					}

					if((string = json_getString(sortDirective,
									"attribute")) != NULL) {
						action->sort.directives[ii].type =
							SEARCH_COMPILER_SEARCH_SORT_TYPE_ATTRIBUTE;
						action->sort.directives[ii].attribute = strdup(string);
					}
					else {
						action->sort.directives[ii].type =
							SEARCH_COMPILER_SEARCH_SORT_TYPE_ERROR;
					}
				}
				else if(!strcasecmp(string, "geocoord")) {
					if(!checkJsonParameter(compiler, sortDirective,
								JSON_VALUE_TYPE_NUMBER, "latitude",
								validationType, TEMPLATE)) {
						return afalse;
					}

					if(!checkJsonParameter(compiler, sortDirective,
								JSON_VALUE_TYPE_NUMBER, "longitude",
								validationType, TEMPLATE)) {
						return afalse;
					}

					if((json_elementExists(sortDirective, "latitude")) &&
							(json_elementExists(sortDirective, "longitude"))) {
						action->sort.directives[ii].type =
							SEARCH_COMPILER_SEARCH_SORT_TYPE_GEO_COORD;
						action->sort.directives[ii].latitude =
							json_getNumber(sortDirective,
								"latitude");
						action->sort.directives[ii].longitude =
							json_getNumber(sortDirective,
								"longitude");
					}
					else {
						action->sort.directives[ii].type =
							SEARCH_COMPILER_SEARCH_SORT_TYPE_ERROR;
					}
				}
				else if(!strcasecmp(string, "relevancy")) {
					action->sort.directives[ii].type =
						SEARCH_COMPILER_SEARCH_SORT_TYPE_RELEVANCY;
				}
			}

			if((action->sort.directives[ii].type ==
					 SEARCH_COMPILER_SEARCH_SORT_TYPE_RELEVANCY) ||
					(action->sort.directives[ii].type ==
					 SEARCH_COMPILER_SEARCH_SORT_TYPE_UNKNOWN)) {
				action->sort.directives[ii].type =
					SEARCH_COMPILER_SEARCH_SORT_TYPE_UNKNOWN;

				for(nn = 0; nn < action->groupsLength; nn++) {
					for(jj = 0;
							jj < action->groups[nn].conditionsLength;
							jj++) {
						if((action->groups[nn].conditions[jj].type ==
							 SEARCH_COMPILER_SEARCH_CONDITION_TYPE_WILDCARD) ||
							(action->groups[nn].conditions[jj].type ==
							 SEARCH_COMPILER_SEARCH_CONDITION_TYPE_WILDCARD) ||
							(action->groups[nn].conditions[jj].type ==
							 SEARCH_COMPILER_SEARCH_CONDITION_TYPE_WILDCARD)) {
							action->sort.directives[ii].type =
								SEARCH_COMPILER_SEARCH_SORT_TYPE_RELEVANCY;
							break;
						}
					}
				}
			}
	
			if(action->sort.directives[ii].type ==
					SEARCH_COMPILER_SEARCH_SORT_TYPE_UNKNOWN) {
				action->sort.isSorted = afalse;
				break;
			}

			ii++;
			if(ii >= action->sort.directiveLength) {
				break;
			}

			sortDirective = json_getObjectFromArray(sortOptions,
					"directives", ii);
		} while(sortDirective != NULL);
	}
	else {
		for(ii = 0; ii < action->groupsLength; ii++) {
			for(nn = 0; nn < action->groups[ii].conditionsLength; nn++) {
				if((action->groups[ii].conditions[nn].type ==
						 SEARCH_COMPILER_SEARCH_CONDITION_TYPE_WILDCARD) ||
						(action->groups[ii].conditions[nn].type ==
						 SEARCH_COMPILER_SEARCH_CONDITION_TYPE_WILDCARD) ||
						(action->groups[ii].conditions[nn].type ==
						 SEARCH_COMPILER_SEARCH_CONDITION_TYPE_WILDCARD)) {
					action->sort.isSorted = atrue;
					action->sort.directiveLength = 1;
					action->sort.directives =
						(SearchActionSearchSortDirective *)malloc(
								sizeof(SearchActionSearchSortDirective) *
								action->sort.directiveLength);
					action->sort.directives[0].type =
						SEARCH_COMPILER_SEARCH_SORT_TYPE_RELEVANCY;
					break;
				}
			}
		}
	}

	// determine if result options were specified

	if((resultOptions = json_getObject(message, "resultOptions")) != NULL) {
		if(json_elementExists(resultOptions, "offset")) {
			if(!checkJsonParameter(compiler, resultOptions,
						JSON_VALUE_TYPE_NUMBER, "offset",
						validationType, TEMPLATE)) {
				return afalse;
			}

			if((action->options.offset = (int)json_getNumber(resultOptions,
							"offset")) < 0) {
				action->options.offset = 0;
			}
		}

		if(json_elementExists(resultOptions, "limit")) {
			if(!checkJsonParameter(compiler, resultOptions,
						JSON_VALUE_TYPE_NUMBER, "limit",
						validationType, TEMPLATE)) {
				return afalse;
			}

			if((action->options.limit = (int)json_getNumber(resultOptions,
							"limit")) < 0) {
				action->options.limit = 0;
			}
		}

		if(json_elementExists(resultOptions, "includeDistances")) {
			if(!checkJsonParameter(compiler, resultOptions,
						JSON_VALUE_TYPE_BOOLEAN, "includeDistances",
						validationType, TEMPLATE)) {
				return afalse;
			}

			isIncludeDistances = json_getBoolean(resultOptions,
					"includeDistances");

			if(isIncludeDistances) {
				if(!checkJsonParameter(compiler, resultOptions,
							JSON_VALUE_TYPE_NUMBER, "latitude",
							validationType, TEMPLATE)) {
					return afalse;
				}

				if(!checkJsonParameter(compiler, resultOptions,
							JSON_VALUE_TYPE_NUMBER, "longitude",
							validationType, TEMPLATE)) {
					return afalse;
				}
			}
		}

		if(isIncludeDistances) {
			action->options.isIncludeDistances = isIncludeDistances;

			action->options.latitude = json_getNumber(resultOptions,
					"latitude");

			action->options.longitude = json_getNumber(resultOptions,
					"longitude");
		}

		action->options.hasResultOptions = atrue;
	}

	// determine if facet options were specified

	if((facetOptions = json_getObject(message, "facetOptions")) != NULL) {
		if(json_elementExists(facetOptions, "displayIndexSummary")) {
			if(!checkJsonParameter(compiler, facetOptions,
						JSON_VALUE_TYPE_BOOLEAN, "displayIndexSummary",
						validationType, TEMPLATE)) {
				return afalse;
			}

			action->facets.isDisplayIndexSummary = json_getBoolean(
					facetOptions, "displayIndexSummary");
		}

		if(json_elementExists(facetOptions, "displayFacets")) {
			if(!checkJsonParameter(compiler, facetOptions,
						JSON_VALUE_TYPE_BOOLEAN, "displayFacets",
						validationType, TEMPLATE)) {
				return afalse;
			}

			action->facets.isDisplayFacets =
				json_getBoolean(facetOptions, "displayFacets");
		}

		// allocate & initialize facet attributes

		if((action->facets.isDisplayIndexSummary) ||
				(action->facets.isDisplayFacets)) {
			if(!checkJsonParameter(compiler, facetOptions,
						JSON_VALUE_TYPE_ARRAY, "attributes",
						validationType, TEMPLATE)) {
				return afalse;
			}
			else if((action->facets.attributesLength = json_getArrayLength(
							facetOptions, "attributes")) < 1) {
				searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
						"failed to obtain value 'facetOptions->attributes' "
						"array length");
				return afalse;
			}

			action->facets.attributes = (char **)malloc(sizeof(char *) *
					action->facets.attributesLength);

			for(ii = 0; ii < action->facets.attributesLength; ii++) {
				if((string = json_getStringFromArray(facetOptions,
								"attributes", ii)) == NULL) {
					searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
							"failed to locate value in "
							"'facetOptions->attributes' array");
					return afalse;
				}

				action->facets.attributes[ii] = strdup(string);
			}
		}

		action->facets.hasResultFacets = atrue;
	}

	// finish initialization

	intersect_init(&action->intersect);

	compiler->type = SEARCH_COMPILER_ACTION_SEARCH;

	return atrue;
}

Json *searchJson_buildSearchResult(SearchCompiler *compiler)
{
	aboolean isAddLatitude = afalse;
	aboolean isAddLongitude = afalse;
	int rc = 0;
	int ii = 0;
	int nn = 0;
	int limit = 0;
	int offset = 0;
	int counter = 0;
	int stringLength = 0;
	int domainListLength = 0;
	double timer = 0.0;
	double elapsedTime = 0.0;
	double latitude = 0.0;
	double longitude = 0.0;
	double distance = 0.0;
	char *string = NULL;
	char **domainList = NULL;

	Json *entry = NULL;
	Json *result = NULL;
	Container *container = NULL;
	SearchActionSearch *action = NULL;

	if((compiler == NULL) ||
			(compiler->type != SEARCH_COMPILER_ACTION_SEARCH)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	action = (SearchActionSearch *)compiler->action;

	result = json_new();

	json_addString(result, "messageName", "searchResponse");

	if(compiler->errorCode != 0) {
		string = itoa(compiler->errorCode);

		json_addString(result, "resultCode", string);

		free(string);

		if(compiler->errorMessage != NULL) {
			json_addString(result, "errorMessage", compiler->errorMessage);
		}
		else if(!compiler->isCompileComplete) {
			json_addString(result, "errorMessage",
					"compile failed to complete");
		}
		else if(!compiler->isExecutionComplete) {
			json_addString(result, "errorMessage",
					"execution failed to complete");
		}

		return result;
	}

	json_addString(result, "resultCode", "0");

	// build the index summary or facets

	if((action->facets.hasResultFacets) &&
			((action->facets.isDisplayIndexSummary) ||
			 (action->facets.isDisplayFacets))) {
		timer = time_getTimeMus();

		if(action->facets.isDisplayIndexSummary) {
			json_addArray(result, "indexSummary");
		}

		counter = 0;

		if(!action->settings.isGlobalSearch) {
			for(ii = 0; ii < action->facets.attributesLength; ii++) {
				for(nn = 0; nn < action->domainKeysLength; nn++) {
					if(action->facets.isDisplayIndexSummary) {
						buildIndexSummary(action->engine,
								action->domainKeys[nn],
								action->facets.attributes[ii],
								result);
						counter++;
					}
				}

				if(action->facets.isDisplayFacets) {
					buildSearchResultFacets(action,
							action->facets.attributes[ii], result);
				}
			}
		}
		else if((domainList = searchEngine_listDomains(action->engine,
						&domainListLength, &rc)) != NULL) {
			for(ii = 0; ii < action->facets.attributesLength; ii++) {
				for(nn = 0; nn < domainListLength; nn++) {
					if(domainList[nn] == NULL) {
						continue;
					}

					if(action->facets.isDisplayIndexSummary) {
						buildIndexSummary(action->engine,
								domainList[nn],
								action->facets.attributes[ii],
								result);
						counter++;
					}
				}

				if(action->facets.isDisplayFacets) {
					buildSearchResultFacets(action,
							action->facets.attributes[ii], result);
				}
			}

			for(ii = 0; ii < domainListLength; ii++) {
				if(domainList[ii] != NULL) {
					free(domainList[ii]);
				}
			}

			free(domainList);
		}

		elapsedTime = time_getElapsedMusInSeconds(timer);

		log_logf(compiler->log, LOG_LEVEL_DEBUG,
				"{SEARCH} constructed %i index summary items in "
				"%0.6f (%0.6f avg) seconds",
				counter, elapsedTime, (elapsedTime / (double)counter));
	}

	// build a set of response objects resulting from the search

	timer = time_getTimeMus();

	offset = 0;
	limit = action->intersect.result.length;

	if(action->options.hasResultOptions) {
		if(action->options.offset > 0) {
			offset = action->options.offset;
		}

		if(action->options.limit > 0) {
			limit = action->options.limit;
		}
	}

	for(ii = offset, counter = 0;
			((ii < action->intersect.result.length) && (counter < limit));
			ii++) {
		if(action->intersect.result.array[ii] == 0) {
			continue;
		}

		if((container = searchEngine_get(action->engine,
						action->intersect.result.array[ii],
						&rc)) == NULL) {
			log_logf(compiler->log, LOG_LEVEL_WARNING,
					"{SEARCH} failed to locate container #%i from search "
					"intersection %i with '%s'",
					action->intersect.result.array[ii], ii,
					searchEngine_errorCodeToString(rc));

			continue;
		}
		else if(action->settings.isEntireContainer) {
			entry = container_containerToJson(container);

			searchEngine_unlockGet(action->engine, container);

			if(action->options.isIncludeDistances) {
				searchSort_determineGeoCoordsOnEntity(action->engine,
						action->options.latitude,
						action->options.longitude,
						ii,
						action->intersect.result.array,
						action->intersect.result.length,
						&latitude,
						&longitude,
						&distance);
			}
		}
		else {
			isAddLatitude = afalse;
			isAddLongitude = afalse;

			entry = json_new();

			json_addNumber(entry, "uid", (double)container->uid);

			for(nn = 0; nn < action->requestAttributesLength; nn++) {
				if(!strcmp(action->requestAttributes[nn], "latitude")) {
					isAddLatitude = atrue;
					continue;
				}
				else if(!strcmp(action->requestAttributes[nn], "longitude")) {
					isAddLongitude = atrue;
					continue;
				}
				else if((string = container_getString(container,
								action->requestAttributes[nn],
								strlen(action->requestAttributes[nn]),
								&stringLength)) == NULL) {
					continue;
				}

				json_addString(entry, action->requestAttributes[nn], string);

				free(string);
			}

			searchEngine_unlockGet(action->engine, container);

			if((isAddLatitude) ||
					(isAddLongitude) ||
					(action->options.isIncludeDistances)) {
				searchSort_determineGeoCoordsOnEntity(action->engine,
						action->options.latitude,
						action->options.longitude,
						ii,
						action->intersect.result.array,
						action->intersect.result.length,
						&latitude,
						&longitude,
						&distance);

				if(isAddLatitude) {
					string = (char *)malloc(sizeof(char) * 128);
					snprintf(string, ((int)(sizeof(char) * 128) - 1), "%0.6f",
							latitude);

					json_addString(entry, "latitude", string);

					free(string);
				}

				if(isAddLongitude) {
					string = (char *)malloc(sizeof(char) * 128);
					snprintf(string, ((int)(sizeof(char) * 128) - 1), "%0.6f",
							longitude);

					json_addString(entry, "longitude", string);

					free(string);
				}
			}
		}

		if(entry == NULL) {
			continue;
		}

		if(action->options.isIncludeDistances) {
			distance = searchSort_calculateGeoCoordDistanceMiles(
					action->options.latitude, action->options.longitude,
					latitude, longitude);

			json_addNumber(entry, "distanceInMiles", distance);
		}

		if(counter == 0) {
			json_addArray(result, "resultSet");
		}

		json_addObjectToArray(result, "resultSet", entry);

		counter++;
	}

	if(counter > 0) {
		elapsedTime = time_getElapsedMusInSeconds(timer);

		log_logf(compiler->log, LOG_LEVEL_DEBUG,
				"{SEARCH} constructed %i search-result containers in "
				"%0.6f (%0.6f avg) seconds",
				counter, elapsedTime, (elapsedTime / (double)counter));
	}

	json_addNumber(result, "resultSetLength", counter);

	json_addNumber(result, "resultSetTotalLength",
			action->intersect.result.length);

	return result;
}

aboolean searchJson_compileList(SearchCompiler *compiler,
		SearchJsonValidationType validationType, Json *message)
{
	aboolean isListDomains = afalse;
	aboolean isListIndexes = afalse;
	aboolean isListIndexDetails = afalse;
	char *domainKey = NULL;
	char *indexKey = NULL;

	SearchActionList *action = NULL;

	const char *TEMPLATE = "failed to compile list with '%s'";

	if((compiler == NULL) || (message == NULL)) {
		DISPLAY_INVALID_ARGS;
		if(compiler != NULL) {
			searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
					"invalid or missing argument(s)");
		}
		return afalse;
	}

	if(json_elementExists(message, "domains")) {
		if(!checkJsonParameter(compiler, message, JSON_VALUE_TYPE_BOOLEAN,
					"domains", validationType, TEMPLATE)) {
			return afalse;
		}

		isListDomains = json_getBoolean(message, "domains");
	}

	if(json_elementExists(message, "indexes")) {
		if(!checkJsonParameter(compiler, message, JSON_VALUE_TYPE_BOOLEAN,
					"indexes", validationType, TEMPLATE)) {
			return afalse;
		}

		isListIndexes = json_getBoolean(message, "indexes");
	}

	if(json_elementExists(message, "indexDetails")) {
		if(!checkJsonParameter(compiler, message, JSON_VALUE_TYPE_BOOLEAN,
					"indexDetails", validationType, TEMPLATE)) {
			return afalse;
		}

		isListIndexDetails = json_getBoolean(message, "indexDetails");
	}

	if(isListIndexDetails) {
		if(!checkJsonParameter(compiler, message, JSON_VALUE_TYPE_STRING,
					"domainKey", validationType, TEMPLATE)) {
			return afalse;
		}
		else if((domainKey = json_getString(message, "domainKey")) == NULL) {
			searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
					"failed to obtain 'domainKey' parameter");
			return afalse;
		}

		if(!checkJsonParameter(compiler, message, JSON_VALUE_TYPE_STRING,
					"indexKey", validationType, TEMPLATE)) {
			return afalse;
		}
		else if((indexKey = json_getString(message, "indexKey")) == NULL) {
			searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
					"failed to obtain 'indexKey' parameter");
			return afalse;
		}
	}

	action = (SearchActionList *)malloc(sizeof(SearchActionList));

	action->isListDomains = isListDomains;
	action->isListIndexes = isListIndexes;
	action->isListIndexDetails = isListIndexDetails;

	if(isListIndexDetails) {
		action->domainKey = strdup(domainKey);
		action->indexKey = strdup(indexKey);
	}

	compiler->type = SEARCH_COMPILER_ACTION_LIST;
	compiler->action = (void *)action;

	return atrue;
}

Json *searchJson_buildListResult(SearchCompiler *compiler)
{
	int ii = 0;
	char *string = NULL;

	Json *entry = NULL;
	Json *result = NULL;
	SearchActionList *action = NULL;

	if((compiler == NULL) ||
			(compiler->type != SEARCH_COMPILER_ACTION_LIST)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	action = (SearchActionList *)compiler->action;

	result = json_new();

	json_addString(result, "messageName", "listResponse");

	if(compiler->errorCode != 0) {
		string = itoa(compiler->errorCode);

		json_addString(result, "resultCode", string);

		free(string);

		if(compiler->errorMessage != NULL) {
			json_addString(result, "errorMessage", compiler->errorMessage);
		}
		else if(!compiler->isCompileComplete) {
			json_addString(result, "errorMessage",
					"compile failed to complete");
		}
		else if(!compiler->isExecutionComplete) {
			json_addString(result, "errorMessage",
					"execution failed to complete");
		}

		return result;
	}

	json_addString(result, "resultCode", "0");

	if((action->domainList != NULL) && (action->domainListLength > 0)) {
		json_addArray(result, "domainList");

		for(ii = 0; ii < action->domainListLength; ii++) {
			json_addStringToArray(result, "domainList", action->domainList[ii]);
		}
	}

	if((action->indexList != NULL) && (action->indexListLength > 0)) {
		json_addArray(result, "indexList");

		for(ii = 0; ii < action->indexListLength; ii++) {
			json_addStringToArray(result, "indexList", action->indexList[ii]);
		}
	}

	if((action->isListIndexDetails) &&
			(action->indexCount > 0) &&
			(action->indexUniqueValueCount > 0)) {
		entry = json_new();

		json_addString(entry, "indexKey", action->indexKey);
		json_addString(entry, "indexType", action->indexType);
		json_addString(entry, "domainKey", action->domainKey);
		json_addNumber(entry, "indexUniqueKeysCount", action->indexCount);
		json_addNumber(entry, "indexUniqueValuesCount",
				action->indexUniqueValueCount);
		json_addNumber(entry, "indexBptreeDepth", action->indexDepth);

		buildDisplayFacets(action->engine,
				action->domainKey,
				action->indexKey,
				entry);

		json_addObject(result, "indexDetails", entry);
	}

	return result;
}

aboolean searchJson_compileCommand(SearchCompiler *compiler,
		SearchJsonValidationType validationType, Json *message)
{
	aboolean isValidCommand = afalse;
	int indexType = 0;
	char *string = NULL;

	SearchCompilerCommandTypes type = SEARCH_COMPILER_COMMAND_TYPE_ERROR;
	SearchCompilerCommandActions action = SEARCH_COMPILER_COMMAND_ACTION_ERROR;
	SearchActionCommand *command = NULL;

	const char *TEMPLATE = "failed to compile command with '%s'";

	if((compiler == NULL) || (message == NULL)) {
		DISPLAY_INVALID_ARGS;
		if(compiler != NULL) {
			searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
					"invalid or missing argument(s)");
		}
		return afalse;
	}

	if(!checkJsonParameter(compiler, message, JSON_VALUE_TYPE_STRING,
				"type", validationType, TEMPLATE)) {
		return afalse;
	}
	else if((string = json_getString(message, "type")) == NULL) {
		searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
				"failed to obtain 'type' parameter");
		return afalse;
	}

	if(!strcasecmp(string, "index")) {
		type = SEARCH_COMPILER_COMMAND_TYPE_INDEX;
	}
	else if(!strcasecmp(string, "domain")) {
		type = SEARCH_COMPILER_COMMAND_TYPE_DOMAIN;
	}
	else if(!strcasecmp(string, "server")) {
		type = SEARCH_COMPILER_COMMAND_TYPE_SERVER;
	}
	else {
		searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
				"invalid 'type' parameter");
		return afalse;
	}

	if(!checkJsonParameter(compiler, message, JSON_VALUE_TYPE_STRING,
				"action", validationType, TEMPLATE)) {
		return afalse;
	}
	else if((string = json_getString(message, "action")) == NULL) {
		searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
				"failed to obtain 'type' parameter");
		return afalse;
	}

	if(!strcasecmp(string, "update")) {
		action = SEARCH_COMPILER_COMMAND_ACTION_UPDATE;
	}
	else if(!strcasecmp(string, "rename")) {
		action = SEARCH_COMPILER_COMMAND_ACTION_RENAME;
	}
	else if(!strcasecmp(string, "delete")) {
		action = SEARCH_COMPILER_COMMAND_ACTION_DELETE;
	}
	else if(!strcasecmp(string, "changeType")) {
		action = SEARCH_COMPILER_COMMAND_ACTION_CHANGE_TYPE;
	}
	else if(!strcasecmp(string, "copy")) {
		action = SEARCH_COMPILER_COMMAND_ACTION_COPY;
	}
	else if(!strcasecmp(string, "stop")) {
		action = SEARCH_COMPILER_COMMAND_ACTION_STOP;
	}
	else if(!strcasecmp(string, "backup")) {
		action = SEARCH_COMPILER_COMMAND_ACTION_BACKUP;
	}
	else {
		searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
				"invalid 'action' parameter");
		return afalse;
	}

	command = (SearchActionCommand *)malloc(sizeof(SearchActionCommand));

	command->type = SEARCH_COMPILER_COMMAND_TYPE_ERROR;
	command->action = SEARCH_COMPILER_COMMAND_ACTION_ERROR;

	compiler->type = SEARCH_COMPILER_ACTION_COMMAND;
	compiler->action = (void *)command;

	switch(type) {
		case SEARCH_COMPILER_COMMAND_TYPE_INDEX:
			switch(action) {
				case SEARCH_COMPILER_COMMAND_ACTION_UPDATE:
					if(!checkJsonParameter(compiler, message,
								JSON_VALUE_TYPE_STRING, "indexKey",
								validationType, TEMPLATE)) {
						return afalse;
					}
					else if((string = json_getString(message,
									"indexKey")) == NULL) {
						searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
								"failed to obtain 'indexKey' parameter");
						return afalse;
					}

					command->indexKey = strdup(string);

					if((json_elementExists(message, "indexType")) &&
							((string = json_getString(message,
									"indexType")) != NULL)) {
						if(!strcasecmp(string, "exact")) {
							indexType = INDEX_REGISTRY_TYPE_EXACT;
						}
						else if(!strcasecmp(string, "wildcard")) {
							indexType = INDEX_REGISTRY_TYPE_WILDCARD;
						}
						else if(!strcasecmp(string, "range")) {
							indexType = INDEX_REGISTRY_TYPE_RANGE;
						}
						else if(!strcasecmp(string, "user")) {
							indexType = INDEX_REGISTRY_TYPE_USER_KEY;
						}
						else {
							indexType = (int)json_getNumber(message,
									"indexType");

							if((indexType !=
										INDEX_REGISTRY_TYPE_EXACT) &&
									(indexType !=
									 INDEX_REGISTRY_TYPE_WILDCARD) &&
									(indexType !=
									 INDEX_REGISTRY_TYPE_RANGE) &&
									(indexType !=
									 INDEX_REGISTRY_TYPE_USER_KEY)) {
								searchCompiler_buildErrorMessage(compiler,
										TEMPLATE, -1,
										"unrecognized 'indexType' parameter");
								return afalse;
							}
						}

						command->indexType = strdup(
								indexRegistry_typeToString(indexType));
					}

					if(json_elementExists(message, "isIndexFullString")) {
						if(!checkJsonParameter(compiler, message,
									JSON_VALUE_TYPE_BOOLEAN,
									"isIndexFullString",
									validationType, TEMPLATE)) {
							return afalse;
						}
						command->isIndexFullString = json_getBoolean(message,
								"isIndexFullString");
						command->isOverrideIndexing = atrue;
					}

					if(json_elementExists(message, "isIndexTokenizedString")) {
						if(!checkJsonParameter(compiler, message,
									JSON_VALUE_TYPE_BOOLEAN,
									"isIndexTokenizedString",
									validationType, TEMPLATE)) {
							return afalse;
						}
						command->isIndexTokenizedString = json_getBoolean(
								message, "isIndexTokenizedString");
						command->isOverrideIndexing = atrue;
					}

					if(json_elementExists(message, "minStringLength")) {
						if(!checkJsonParameter(compiler, message,
									JSON_VALUE_TYPE_NUMBER,
									"minStringLength",
									validationType, TEMPLATE)) {
							return afalse;
						}
						command->minStringLength = (int)json_getNumber(message,
								"minStringLength");
					}

					if(json_elementExists(message, "maxStringLength")) {
						if(!checkJsonParameter(compiler, message,
									JSON_VALUE_TYPE_NUMBER,
									"maxStringLength",
									validationType, TEMPLATE)) {
							return afalse;
						}
						command->maxStringLength = (int)json_getNumber(message,
								"maxStringLength");
					}

					if((string = json_getString(message,
									"delimiters")) != NULL) {
						if(!checkJsonParameter(compiler, message,
									JSON_VALUE_TYPE_STRING,
									"delimiters",
									validationType, TEMPLATE)) {
							return afalse;
						}
						command->delimiters = strdup(string);
					}

					if((string = json_getString(message,
									"excludedWordFilename")) != NULL) {
						if(!checkJsonParameter(compiler, message,
									JSON_VALUE_TYPE_STRING,
									"excludedWordFilename",
									validationType, TEMPLATE)) {
							return afalse;
						}
						command->excludedWordFilename = strdup(string);
					}

					isValidCommand = atrue;
					break;

				case SEARCH_COMPILER_COMMAND_ACTION_RENAME:
					if(!checkJsonParameter(compiler, message,
								JSON_VALUE_TYPE_STRING, "indexKey",
								validationType, TEMPLATE)) {
						return afalse;
					}
					else if((string = json_getString(message,
									"indexKey")) == NULL) {
						searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
								"failed to obtain 'indexKey' parameter");
						return afalse;
					}

					command->indexKey = strdup(string);

					if(!checkJsonParameter(compiler, message,
								JSON_VALUE_TYPE_STRING, "newIndexKey",
								validationType, TEMPLATE)) {
						return afalse;
					}
					else if((string = json_getString(message,
									"newIndexKey")) == NULL) {
						searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
								"failed to obtain 'newIndexKey' parameter");
						return afalse;
					}

					command->newIndexKey = strdup(string);

					if(!checkJsonParameter(compiler, message,
								JSON_VALUE_TYPE_STRING, "newIndexName",
								validationType, TEMPLATE)) {
						return afalse;
					}
					else if((string = json_getString(message,
									"newIndexName")) == NULL) {
						searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
								"failed to obtain 'newIndexName' parameter");
						return afalse;
					}

					command->newIndexName = strdup(string);

					isValidCommand = atrue;
					break;

				case SEARCH_COMPILER_COMMAND_ACTION_DELETE:
					if(!checkJsonParameter(compiler, message,
								JSON_VALUE_TYPE_STRING, "indexKey",
								validationType, TEMPLATE)) {
						return afalse;
					}
					else if((string = json_getString(message,
									"indexKey")) == NULL) {
						searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
								"failed to obtain 'indexKey' parameter");
						return afalse;
					}

					command->indexKey = strdup(string);

					isValidCommand = atrue;
					break;

				case SEARCH_COMPILER_COMMAND_ACTION_CHANGE_TYPE:
					if(!checkJsonParameter(compiler, message,
								JSON_VALUE_TYPE_STRING, "indexKey",
								validationType, TEMPLATE)) {
						return afalse;
					}
					else if((string = json_getString(message,
									"indexKey")) == NULL) {
						searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
								"failed to obtain 'indexKey' parameter");
						return afalse;
					}

					command->indexKey = strdup(string);

					if(!checkJsonParameter(compiler, message,
								JSON_VALUE_TYPE_STRING, "indexType",
								validationType, TEMPLATE)) {
						return afalse;
					}
					else if((string = json_getString(message,
									"indexType")) == NULL) {
						searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
								"failed to obtain 'indexType' parameter");
						return afalse;
					}

					command->indexType = strdup(string);

					isValidCommand = atrue;
					break;

				case SEARCH_COMPILER_COMMAND_ACTION_COPY:
					if(!checkJsonParameter(compiler, message,
								JSON_VALUE_TYPE_STRING, "sourceIndexKey",
								validationType, TEMPLATE)) {
						return afalse;
					}
					else if((string = json_getString(message,
									"sourceIndexKey")) == NULL) {
						searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
								"failed to obtain 'sourceIndexKey' parameter");
						return afalse;
					}

					command->indexKey = strdup(string);

					if(!checkJsonParameter(compiler, message,
								JSON_VALUE_TYPE_STRING, "destIndexKey",
								validationType, TEMPLATE)) {
						return afalse;
					}
					else if((string = json_getString(message,
									"destIndexKey")) == NULL) {
						searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
								"failed to obtain 'destIndexKey' parameter");
						return afalse;
					}

					command->newIndexKey = strdup(string);

					if(!checkJsonParameter(compiler, message,
								JSON_VALUE_TYPE_STRING, "destIndexName",
								validationType, TEMPLATE)) {
						return afalse;
					}
					else if((string = json_getString(message,
									"destIndexName")) == NULL) {
						searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
								"failed to obtain 'destIndexName' parameter");
						return afalse;
					}

					command->newIndexName = strdup(string);

					isValidCommand = atrue;
					break;

				case SEARCH_COMPILER_COMMAND_ACTION_STOP:
				case SEARCH_COMPILER_COMMAND_ACTION_BACKUP:
					searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
							"invalid 'action' parameter for specified 'type'");
					return afalse;

				case SEARCH_COMPILER_COMMAND_ACTION_ERROR:
				default:
					searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
							"failed to resolve 'action' parameter");
					return afalse;
			}
			break;

		case SEARCH_COMPILER_COMMAND_TYPE_DOMAIN:
			switch(action) {
				case SEARCH_COMPILER_COMMAND_ACTION_RENAME:
					if(!checkJsonParameter(compiler, message,
								JSON_VALUE_TYPE_STRING, "domainKey",
								validationType, TEMPLATE)) {
						return afalse;
					}
					else if((string = json_getString(message,
									"domainKey")) == NULL) {
						searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
								"failed to obtain 'domainKey' parameter");
						return afalse;
					}

					command->domainKey = strdup(string);

					if(!checkJsonParameter(compiler, message,
								JSON_VALUE_TYPE_STRING, "newDomainKey",
								validationType, TEMPLATE)) {
						return afalse;
					}
					else if((string = json_getString(message,
									"newDomainKey")) == NULL) {
						searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
								"failed to obtain 'newDomainKey' parameter");
						return afalse;
					}

					command->newDomainKey = strdup(string);

					if(!checkJsonParameter(compiler, message,
								JSON_VALUE_TYPE_STRING, "newDomainName",
								validationType, TEMPLATE)) {
						return afalse;
					}
					else if((string = json_getString(message,
									"newDomainName")) == NULL) {
						searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
								"failed to obtain 'newDomainName' parameter");
						return afalse;
					}

					command->newDomainName = strdup(string);

					isValidCommand = atrue;
					break;

				case SEARCH_COMPILER_COMMAND_ACTION_DELETE:
					if(!checkJsonParameter(compiler, message,
								JSON_VALUE_TYPE_STRING, "domainKey",
								validationType, TEMPLATE)) {
						return afalse;
					}
					else if((string = json_getString(message,
									"domainKey")) == NULL) {
						searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
								"failed to obtain 'domainKey' parameter");
						return afalse;
					}

					command->domainKey = strdup(string);

					isValidCommand = atrue;
					break;

				case SEARCH_COMPILER_COMMAND_ACTION_COPY:
					if(!checkJsonParameter(compiler, message,
								JSON_VALUE_TYPE_STRING, "sourceDomainKey",
								validationType, TEMPLATE)) {
						return afalse;
					}
					else if((string = json_getString(message,
									"sourceDomainKey")) == NULL) {
						searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
								"failed to obtain 'sourceDdomainKey' "
								"parameter");
						return afalse;
					}

					command->domainKey = strdup(string);

					if(!checkJsonParameter(compiler, message,
								JSON_VALUE_TYPE_STRING, "destDomainKey",
								validationType, TEMPLATE)) {
						return afalse;
					}
					else if((string = json_getString(message,
									"destDomainKey")) == NULL) {
						searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
								"failed to obtain 'destDomainKey' parameter");
						return afalse;
					}

					command->newDomainKey = strdup(string);

					if(!checkJsonParameter(compiler, message,
								JSON_VALUE_TYPE_STRING, "destDomainName",
								validationType, TEMPLATE)) {
						return afalse;
					}
					else if((string = json_getString(message,
									"destDomainName")) == NULL) {
						searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
								"failed to obtain 'destDomainName' parameter");
						return afalse;
					}

					command->newDomainName = strdup(string);

					isValidCommand = atrue;
					break;

				case SEARCH_COMPILER_COMMAND_ACTION_UPDATE:
				case SEARCH_COMPILER_COMMAND_ACTION_CHANGE_TYPE:
				case SEARCH_COMPILER_COMMAND_ACTION_STOP:
				case SEARCH_COMPILER_COMMAND_ACTION_BACKUP:
					searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
							"invalid 'action' parameter for specified 'type'");
					return afalse;

				case SEARCH_COMPILER_COMMAND_ACTION_ERROR:
				default:
					searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
							"failed to resolve 'action' parameter");
					return afalse;
			}
			break;

		case SEARCH_COMPILER_COMMAND_TYPE_SERVER:
			switch(action) {
				case SEARCH_COMPILER_COMMAND_ACTION_STOP:
				case SEARCH_COMPILER_COMMAND_ACTION_BACKUP:
					isValidCommand = atrue;
					break;

				case SEARCH_COMPILER_COMMAND_ACTION_UPDATE:
				case SEARCH_COMPILER_COMMAND_ACTION_RENAME:
				case SEARCH_COMPILER_COMMAND_ACTION_DELETE:
				case SEARCH_COMPILER_COMMAND_ACTION_CHANGE_TYPE:
				case SEARCH_COMPILER_COMMAND_ACTION_COPY:
					searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
							"invalid 'action' parameter for specified 'type'");
					return afalse;

				case SEARCH_COMPILER_COMMAND_ACTION_ERROR:
				default:
					searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
							"failed to resolve 'action' parameter");
					return afalse;
			}
			break;

		case SEARCH_COMPILER_COMMAND_TYPE_ERROR:
		default:
			searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
					"incompatible type/action parameters");
			return afalse;
	}

	if(!isValidCommand) {
		searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
				"invalid command specified");
		return afalse;
	}

	command->type = type;
	command->action = action;

	return atrue;
}

Json *searchJson_buildCommandResult(SearchCompiler *compiler)
{
	char *string = NULL;

	Json *result = NULL;
	SearchActionCommand *action = NULL;

	if((compiler == NULL) ||
			(compiler->type != SEARCH_COMPILER_ACTION_COMMAND)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	action = (SearchActionCommand *)compiler->action;

	result = json_new();

	json_addString(result, "messageName", "commandResponse");
	json_addString(result, "type", searchCompiler_commandTypeToString(
				action->type));
	json_addString(result, "action", searchCompiler_commandActionToString(
				action->action));

	if(compiler->errorCode != 0) {
		string = itoa(compiler->errorCode);

		json_addString(result, "resultCode", string);

		free(string);

		if(compiler->errorMessage != NULL) {
			json_addString(result, "errorMessage", compiler->errorMessage);
		}
		else if(!compiler->isCompileComplete) {
			json_addString(result, "errorMessage",
					"compile failed to complete");
		}
		else if(!compiler->isExecutionComplete) {
			json_addString(result, "errorMessage",
					"execution failed to complete");
		}

		return result;
	}

	json_addString(result, "resultCode", "0");

	return result;
}

aboolean searchJson_compileLogin(SearchCompiler *compiler,
		SearchJsonValidationType validationType, Json *message)
{
	char *username = NULL;
	char *password = NULL;

	SearchActionLogin *action = NULL;

	const char *TEMPLATE = "failed to compile login with '%s'";

	if((compiler == NULL) || (message == NULL)) {
		DISPLAY_INVALID_ARGS;
		if(compiler != NULL) {
			searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
					"invalid or missing argument(s)");
		}
		return afalse;
	}

	if(!checkJsonParameter(compiler, message, JSON_VALUE_TYPE_STRING,
				"username", validationType, TEMPLATE)) {
		return afalse;
	}
	else if((username = json_getString(message, "username")) == NULL) {
		searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
				"failed to obtain 'username' parameter");
		return afalse;
	}

	if(!checkJsonParameter(compiler, message, JSON_VALUE_TYPE_STRING,
				"password", validationType, TEMPLATE)) {
		return afalse;
	}
	else if((password = json_getString(message, "password")) == NULL) {
		searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
				"failed to obtain 'password' parameter");
		return afalse;
	}

	action = (SearchActionLogin *)malloc(sizeof(SearchActionLogin));

	action->username = strdup(username);
	action->password = strdup(password);

	compiler->type = SEARCH_COMPILER_ACTION_LOGIN;
	compiler->action = (void *)action;

	return atrue;
}

Json *searchJson_buildLoginResult(SearchCompiler *compiler)
{
	char *string = NULL;

	Json *result = NULL;
	SearchActionLogin *action = NULL;

	if((compiler == NULL) ||
			(compiler->type != SEARCH_COMPILER_ACTION_LOGIN)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	action = (SearchActionLogin *)compiler->action;

	result = json_new();

	json_addString(result, "messageName", "loginResponse");

	if(compiler->errorCode != 0) {
		string = itoa(compiler->errorCode);

		json_addString(result, "resultCode", string);

		free(string);

		if(compiler->errorMessage != NULL) {
			json_addString(result, "errorMessage", compiler->errorMessage);
		}
		else if(!compiler->isCompileComplete) {
			json_addString(result, "errorMessage",
					"compile failed to complete");
		}
		else if(!compiler->isExecutionComplete) {
			json_addString(result, "errorMessage",
					"execution failed to complete");
		}

		return result;
	}

	json_addString(result, "resultCode", "0");

	if(action->token != NULL) {
		json_addString(result, "token", action->token);
	}

	return result;
}

aboolean searchJson_compileLogout(SearchCompiler *compiler,
		SearchJsonValidationType validationType, Json *message)
{
	char *token = NULL;

	SearchActionLogout *action = NULL;

	const char *TEMPLATE = "failed to compile logout with '%s'";

	if((compiler == NULL) || (message == NULL)) {
		DISPLAY_INVALID_ARGS;
		if(compiler != NULL) {
			searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
					"invalid or missing argument(s)");
		}
		return afalse;
	}

	if(!checkJsonParameter(compiler, message, JSON_VALUE_TYPE_STRING,
				"token", validationType, TEMPLATE)) {
		return afalse;
	}
	else if((token = json_getString(message, "token")) == NULL) {
		searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
				"failed to obtain 'token' parameter");
		return afalse;
	}

	action = (SearchActionLogout *)malloc(sizeof(SearchActionLogout));

	action->token = strdup(token);

	compiler->type = SEARCH_COMPILER_ACTION_LOGOUT;
	compiler->action = (void *)action;

	return atrue;
}

Json *searchJson_buildLogoutResult(SearchCompiler *compiler)
{
	char *string = NULL;

	Json *result = NULL;
	SearchActionLogout *action = NULL;

	if((compiler == NULL) ||
			(compiler->type != SEARCH_COMPILER_ACTION_LOGOUT)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	action = (SearchActionLogout *)compiler->action;

	result = json_new();

	json_addString(result, "messageName", "logoutResponse");

	if(compiler->errorCode != 0) {
		string = itoa(compiler->errorCode);

		json_addString(result, "resultCode", string);

		free(string);

		if(compiler->errorMessage != NULL) {
			json_addString(result, "errorMessage", compiler->errorMessage);
		}
		else if(!compiler->isCompileComplete) {
			json_addString(result, "errorMessage",
					"compile failed to complete");
		}
		else if(!compiler->isExecutionComplete) {
			json_addString(result, "errorMessage",
					"execution failed to complete");
		}

		return result;
	}

	json_addString(result, "resultCode", "0");

	return result;
}

