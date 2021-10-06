/*
 * search_compiler.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The component for the search-engine to compile a given input syntax into
 * commands for execution within the search environment.
 *
 * Written by Josh English.
 */

// preprocessor directives

#define _CORE_COMMON_BYPASS_STD_STREAM

#include "core/core.h"
#include "search/search.h"
#define _SEARCH_SYSTEM_COMPONENT
#include "search/system/search_compiler.h"


// define search compiler private data types

typedef struct _PruneByDistance {
	int uid;
	double distance;
} PruneByDistance;


// define search compiler private functions

// general functions

static char *actionToString(SearchCompilerActions action)
{
	char *result = NULL;

	switch(action) {
		case SEARCH_COMPILER_ACTION_NEW_DOMAIN:
			result = "new domain";
			break;

		case SEARCH_COMPILER_ACTION_NEW_INDEX:
			result = "new index";
			break;

		case SEARCH_COMPILER_ACTION_GET:
			result = "get";
			break;

		case SEARCH_COMPILER_ACTION_GET_BY_ATTRIBUTE:
			result = "get by attribute";
			break;

		case SEARCH_COMPILER_ACTION_PUT:
			result = "put";
			break;

		case SEARCH_COMPILER_ACTION_UPDATE:
			result = "update";
			break;

		case SEARCH_COMPILER_ACTION_UPDATE_BY_ATTRIBUTE:
			result = "update by attribute";
			break;

		case SEARCH_COMPILER_ACTION_DELETE:
			result = "delete";
			break;

		case SEARCH_COMPILER_ACTION_DELETE_BY_ATTRIBUTE:
			result = "delete by attribute";
			break;

		case SEARCH_COMPILER_ACTION_SEARCH:
			result = "search";
			break;

		case SEARCH_COMPILER_ACTION_LIST:
			result = "list";
			break;

		case SEARCH_COMPILER_ACTION_COMMAND:
			result = "command";
			break;

		case SEARCH_COMPILER_ACTION_LOGIN:
			result = "login";
			break;

		case SEARCH_COMPILER_ACTION_LOGOUT:
			result = "logout";
			break;

		case SEARCH_COMPILER_ACTION_ERROR:
			result = "error";
			break;

		default:
			result = "unknown";
	}

	return result;
}

static char *commandTypeToString(SearchCompilerCommandTypes type)
{
	char *result = NULL;

	switch(type) {
		case SEARCH_COMPILER_COMMAND_TYPE_INDEX:
			result = "index";
			break;

		case SEARCH_COMPILER_COMMAND_TYPE_DOMAIN:
			result = "domain";
			break;

		case SEARCH_COMPILER_COMMAND_TYPE_SERVER:
			result = "server";
			break;

		case SEARCH_COMPILER_COMMAND_TYPE_ERROR:
			result = "error";
			break;

		default:
			result = "unknown";
	}

	return result;
}

static char *commandActionToString(SearchCompilerCommandActions action)
{
	char *result = NULL;

	switch(action) {
		case SEARCH_COMPILER_COMMAND_ACTION_UPDATE:
			result = "update";
			break;

		case SEARCH_COMPILER_COMMAND_ACTION_RENAME:
			result = "rename";
			break;

		case SEARCH_COMPILER_COMMAND_ACTION_DELETE:
			result = "delete";
			break;

		case SEARCH_COMPILER_COMMAND_ACTION_CHANGE_TYPE:
			result = "change type";
			break;

		case SEARCH_COMPILER_COMMAND_ACTION_COPY:
			result = "copy";
			break;

		case SEARCH_COMPILER_COMMAND_ACTION_STOP:
			result = "stop";
			break;

		case SEARCH_COMPILER_COMMAND_ACTION_BACKUP:
			result = "backup";
			break;

		case SEARCH_COMPILER_COMMAND_ACTION_ERROR:
			result = "error";
			break;

		default:
			result = "unknown";
	}

	return result;
}

static int returnError(SearchCompiler *compiler, int errorCode)
{
	char *string = NULL;

	const char *TEMPLATE = "search query compiler failed with '%s'";

	switch(errorCode) {
		case SEARCH_COMPILER_ERROR_DEFAULT:
			string = "default error";
			break;

		case SEARCH_COMPILER_ERROR_JSON_MISSING_MESSAGE_NAME:
			string = "json object missing message name";
			break;

		case SEARCH_COMPILER_ERROR_JSON_INVALID_MESSAGE_NAME:
			string = "json object invalid message name";
			break;

		case SEARCH_COMPILER_ERROR_JSON_COMPILE_FAILED:
			string = "json compile failed";
			break;

		case SEARCH_COMPILER_ERROR_JSON_COMPILE_INVALID_PARAMETER:
			string = "json compile found invalid parameter";
			break;

		case SEARCH_COMPILER_ERROR_SQL_TOKENIZE_FAILED:
			string = "sql query string tokenization failed";
			break;

		case SEARCH_COMPILER_ERROR_SQL_UNRECOGNIZED_COMMAND:
			string = "unrecognized sql query command";
			break;

		case SEARCH_COMPILER_ERROR_SQL_COMPILE_FAILED:
			string = NULL;
			break;

		case SEARCH_COMPILER_ERROR_EXEC_COMPILE_INCOMPLETE:
			string = "execution failed, compile incomplete";
			break;

		case SEARCH_COMPILER_ERROR_EXEC_UNKNOWN_ACTION:
			string = "unable to execute unknown action";
			break;

		case SEARCH_COMPILER_ERROR_EXEC_FAILED:
			string = "failed to execute search engine";
			break;

		case SEARCH_COMPILER_ERROR_EXEC_CONTAINER_DOES_NOT_EXIST:
			string = "failed to locate requested container";
			break;

		case SEARCH_COMPILER_ERROR_EXEC_AUTHENTICATION_OFFLINE:
			string = "authentication system disabled";
			break;

		case SEARCH_COMPILER_ERROR_EXEC_PERMISSION_DENIED:
			string = "permission denied";
			break;

		case SEARCH_COMPILER_ERROR_EXEC_SORT_OUT_OF_MEMORY:
			string = "sort operation exceeded memory limits";
			break;

		case SEARCH_COMPILER_ERROR_UNKNOWN:
		default:
			string = "uknown error";
	}

	if(string != NULL) {
		searchCompiler_buildErrorMessage(compiler, TEMPLATE, errorCode, string);
	}

	return errorCode;
}

static int returnSearchEngineError(SearchCompiler *compiler,
		SearchEngine *engine, int errorCode)
{
	char *string = NULL;

	const char *TEMPLATE = "search engine execution failed with '%s'";

	if((errorCode < SEARCH_COMPILER_ERROR_DEFAULT) &&
			(errorCode <= SEARCH_COMPILER_ERROR_JSON_MISSING_MESSAGE_NAME)) {
		return returnError(compiler, errorCode);
	}
	else if((string = searchEngine_errorCodeToString(errorCode)) != NULL) {
		searchCompiler_buildErrorMessage(compiler, TEMPLATE, errorCode, string);
	}
	else {
		return returnError(compiler, SEARCH_COMPILER_ERROR_EXEC_FAILED);
	}

	return errorCode;
}

static char *searchConditionTypeToString(SearchCompilerSearchConditionType type)
{
	char *result = NULL;

	switch(type) {
		case SEARCH_COMPILER_SEARCH_CONDITION_TYPE_WILDCARD:
			result = "Wildcard";
			break;

		case SEARCH_COMPILER_SEARCH_CONDITION_TYPE_EXACT:
			result = "Exact Match";
			break;

		case SEARCH_COMPILER_SEARCH_CONDITION_TYPE_USER:
			result = "User Index";
			break;

		case SEARCH_COMPILER_SEARCH_CONDITION_TYPE_RANGE:
			result = "Range";
			break;

		case SEARCH_COMPILER_SEARCH_CONDITION_TYPE_GEO_COORD:
			result = "Geo Coord";
			break;

		case SEARCH_COMPILER_SEARCH_CONDITION_TYPE_ERROR:
			result = "Error";
			break;

		default:
			result = "Unknown";
	}

	return result;
}

static char *searchSortTypeToString(SearchCompilerSearchSortType type)
{
	char *result = NULL;

	switch(type) {
		case SEARCH_COMPILER_SEARCH_SORT_TYPE_RELEVANCY:
			result = "Relevancy";
			break;

		case SEARCH_COMPILER_SEARCH_SORT_TYPE_ATTRIBUTE:
			result = "Attribute";
			break;

		case SEARCH_COMPILER_SEARCH_SORT_TYPE_GEO_COORD:
			result = "Geo Coord";
			break;

		case SEARCH_COMPILER_SEARCH_SORT_TYPE_UNKNOWN:
			result = "Unknown";
			break;

		case SEARCH_COMPILER_SEARCH_SORT_TYPE_ERROR:
			result = "Error";
			break;

		default:
			result = "Unknown";
	}

	return result;
}


// string functions

static char *string_locateEndOfUrl(char *string, int length, int *resultLength)
{
	aboolean hasBegun = afalse;
	int ii = 0;

	*resultLength = length;

	for(ii = 0; ii < length; ii++) {
		if(((chartoint)string[ii] <= 32) || ((chartoint)string[ii] > 126)) {
			continue;
		}

		if((!hasBegun) && (string[ii] != '/')) {
			return string;
		}
		else if((hasBegun) &&
				((string[ii] == '=') ||
				 (string[ii] == '?'))) {
			*resultLength = (length - (ii + 1));
			return (string + (ii + 1));
		}
		else if((hasBegun) && (ctype_isWhitespace(string[ii]))) {
			return string;
		}

		hasBegun = atrue;
	}

	return string;
}

static void string_normalizeString(char *string, int length, int *resultLength)
{
	aboolean isVisible = atrue;
	int ii = 0;
	int nn = 0;

	*resultLength = 0;

	for(ii = 0, nn = 0; ii < length; ii++) {
		if(!ctype_isPlainText(string[ii])) {
			continue;
		}

		if(ctype_isWhitespace(string[ii])) {
			string[ii] = ' ';
		}

		if(string[ii] == '\'') {
			if((ii > 0) && (string[(ii - 1)] == '\\')) {
				// do nothing
			}
			else {
				if(isVisible) {
					isVisible = afalse;
				}
				else {
					isVisible = atrue;
				}
			}
		}

		if(isVisible) {
			if((ii > 0) && (string[ii] == ' ') && (string[(ii - 1)] == ' ')) {
				continue;
			}
		}

		string[nn] = string[ii];
		nn++;
	}

	string[nn] = '\0';

	*resultLength = nn;
}

static char *string_parseStringToken(char *string, int start, int length,
		int *parseLength, int *resultLength)
{
	aboolean isVisible = atrue;
	int ii = 0;
	int nn = 0;
	char *result = NULL;

	*parseLength = 0;
	*resultLength = 0;

	result = (char *)malloc(sizeof(char) * length);

	for(ii = start, nn = 0; ii < length; ii++) {
		if(string[ii] == '\'') {
			if((ii > 0) && (string[(ii - 1)] == '\\')) {
				if(nn > 0) {
					result[(nn - 1)] = '\'';
				}
				else {
					result[nn] = '\'';
					nn++;
				}
				continue;
			}
			else {
				if(isVisible) {
					isVisible = afalse;
				}
				else {
					isVisible = atrue;
				}
			}
		}

		if(isVisible) {
			if(string[ii] == ' ') {
				if(nn == 0) {
					continue;
				}
				break;
			}
			else if(string[ii] == ',') {
				if(nn == 0) {
					result[nn] = string[ii];
					nn++;
				}
				else {
					ii--;
				}
				break;
			}
		}

		result[nn] = string[ii];
		nn++;
	}

	*parseLength = ii;

	if(nn < 1) {
		free(result);
		return NULL;
	}

	*resultLength = nn;

	return result;
}

static char **string_tokenizeQuery(char *query, int queryLength,
		int **tokenLengths, int *tokenListLength)
{
	int ref = 0;
	int parseLength = 0;
	int resultRef = 0;
	int resultLength = 0;
	int tokenLength = 0;
	int stringLength = 0;
	int stringNormalizeLength = 0;
	int *resultLengths = NULL;
	char *ptr = NULL;
	char *token = NULL;
	char *string = NULL;
	char **result = NULL;

	*tokenLengths = NULL;
	*tokenListLength = 0;

	if(((ptr = string_locateEndOfUrl(query, queryLength,
						&stringLength)) == NULL) ||
			(stringLength < 1)) {
		return NULL;
	}

	string = strndup(ptr, stringLength);

	string_normalizeString(string, stringLength, &stringNormalizeLength);

	if(stringNormalizeLength < 1) {
		free(string);
		return NULL;
	}

	stringLength = stringNormalizeLength;

	resultRef = 0;
	resultLength = 32;
	resultLengths = (int *)malloc(sizeof(int) * resultLength);
	result = (char **)malloc(sizeof(char *) * resultLength);

	while((ref < stringLength) &&
			((token = string_parseStringToken(string, ref, stringLength,
					&parseLength, &tokenLength)) != NULL)) {
		resultLengths[resultRef] = tokenLength;
		result[resultRef] = token;
		resultRef++;

		if(resultRef >= resultLength) {
			resultLength *= 2;
			resultLengths = (int *)realloc(resultLengths,
					(sizeof(int) * resultLength));
			result = (char **)realloc(result,
					(sizeof(char *) * resultLength));
		}

		ref = (parseLength + 1);
	}

	*tokenListLength = resultRef;
	*tokenLengths = resultLengths;

	free(string);

	return result;
}

// search helper functions

static void search_pruneByDistance(SearchCompiler *compiler,
		SearchEngine *engine, double latitude, double longitude,
		double distance, Intersect *intersect)
{
	int ii = 0;
	int nn = 0;
	int length = 0;
	double dValueOne = 0.0;
	double dValueTwo = 0.0;

	PruneByDistance *prune = NULL;

	log_logf(compiler->log, LOG_LEVEL_DEBUG, "{SEARCH} executing "
			"distance-pruning from (%0.6f, %0.6f) "
			"distance %0.2f miles, length %i",
			latitude, longitude, distance, intersect->result.length);

	if(intersect->result.length < 0) {
		return;
	}

	length = intersect->result.length;

	prune = (PruneByDistance *)malloc(sizeof(PruneByDistance) *
			intersect->result.length);

	for(ii = 0; ii < intersect->result.length; ii++) {
		prune[ii].uid = intersect->result.array[ii];

		searchSort_determineGeoCoordsOnEntity(engine,
				latitude,
				longitude,
				ii,
				intersect->result.array,
				intersect->result.length,
				&dValueOne,
				&dValueTwo,
				&(prune[ii].distance));
	}

	for(ii = 0, nn = 0; ii < intersect->result.length; ii++) {
		if((prune[ii].distance <= 0.0) || (prune[ii].distance > distance)) {
			continue;
		}

		intersect->result.array[nn] = prune[ii].uid;
		nn++;
	}

	free(prune);

	intersect->result.length = nn;

	// remove any duplicates

/*	if(intersect->result.length > 1) {
		for(ii = 0, nn = 0; ii < (intersect->result.length - 1); ii++) {
			if(intersect->result.array[ii] ==
					intersect->result.array[(ii + 1)]) {
				if((ii + 1) >= (intersect->result.length - 1)) {
					intersect->result.array[nn] = intersect->result.array[ii];
					nn++;
					break;
				}
				continue;
			}

			intersect->result.array[nn] = intersect->result.array[ii];
			nn++;
		}

		intersect->result.length = nn;
	}*/

	log_logf(compiler->log, LOG_LEVEL_DEBUG, "{SEARCH} distance-prune "
			"complete from (%0.6f, %0.6f) distance %0.2f miles, "
			"with %i pruned items",
			latitude, longitude, distance, (length - intersect->result.length));
}

static void search_executeSearchCondition(SearchCompiler *compiler,
		SearchEngine *engine, SearchActionSearch *action,
		SearchActionSearchCondition *condition, Intersect *intersect)
{
	int ii = 0;

	Intersect geoIntersect;
	Intersect notIntersect;
	Intersect localIntersect;

	if((condition->type == SEARCH_COMPILER_SEARCH_CONDITION_TYPE_WILDCARD) ||
			(condition->type == SEARCH_COMPILER_SEARCH_CONDITION_TYPE_EXACT) ||
			(condition->type == SEARCH_COMPILER_SEARCH_CONDITION_TYPE_USER)) {
		// perform global string search

		if(action->settings.isGlobalSearch) {
			if(condition->isNotEqualTo) {
				log_logf(compiler->log, LOG_LEVEL_DEBUG, "{SEARCH} "
						"building UID global composite for '%s' "
						"from NOT search",
						condition->attribute);

				intersect_init(&notIntersect);

				searchEngine_searchNotListGlobal(engine,
						condition->attribute,
						&notIntersect);
			}

			log_logf(compiler->log, LOG_LEVEL_DEBUG, "{SEARCH} "
					"executing global search '%s' for '%s'/'%s'",
					searchConditionTypeToString(condition->type),
					condition->attribute,
					condition->value);

			if(condition->isNotEqualTo) {
				searchEngine_searchGlobal(engine,
						condition->attribute,
						condition->value,
						&notIntersect);

				intersect_execNot(&notIntersect, atrue);

				intersect_putArray(intersect,
						notIntersect.result.isSorted,
						notIntersect.result.array,
						notIntersect.result.length);

				intersect_free(&notIntersect);
			}
			else {
				searchEngine_searchGlobal(engine,
						condition->attribute,
						condition->value,
						intersect);
			}

			return;
		}

		// perform domain-specific string search

		if(action->domainKeysLength > 1) {
			intersect_init(&localIntersect);
		}

		for(ii = 0; ii < action->domainKeysLength; ii++) {
			if(condition->isNotEqualTo) {
				log_logf(compiler->log, LOG_LEVEL_DEBUG, "{SEARCH} "
						"building UID composite on domain '%s' for '%s' "
						"from NOT search",
						action->domainKeys[ii],
						condition->attribute);

				intersect_init(&notIntersect);

				searchEngine_searchNotList(engine,
						action->domainKeys[ii],
						condition->attribute,
						&notIntersect);
			}

			log_logf(compiler->log, LOG_LEVEL_DEBUG, "{SEARCH} "
					"executing search '%s' on domain '%s' for "
					"'%s'/'%s'",
					searchConditionTypeToString(condition->type),
					action->domainKeys[ii],
					condition->attribute,
					condition->value);

			if(condition->isNotEqualTo) {
				searchEngine_search(engine,
						action->domainKeys[ii],
						condition->attribute,
						condition->value,
						&notIntersect);

				intersect_execNot(&notIntersect, atrue);

				if(action->domainKeysLength > 1) {
					intersect_putArray(&localIntersect,
							notIntersect.result.isSorted,
							notIntersect.result.array,
							notIntersect.result.length);
				}
				else {
					intersect_putArray(intersect,
							notIntersect.result.isSorted,
							notIntersect.result.array,
							notIntersect.result.length);
				}

				intersect_free(&notIntersect);
			}
			else if(action->domainKeysLength > 1) {
				searchEngine_search(engine,
						action->domainKeys[ii],
						condition->attribute,
						condition->value,
						&localIntersect);
			}
			else {
				searchEngine_search(engine,
						action->domainKeys[ii],
						condition->attribute,
						condition->value,
						intersect);
			}
		}

		if(action->domainKeysLength > 1) {
			intersect_execOr(&localIntersect);

			intersect_putArray(intersect,
					localIntersect.result.isSorted,
					localIntersect.result.array,
					localIntersect.result.length);

			intersect_free(&localIntersect);
		}
	}
	else if(condition->type ==
			SEARCH_COMPILER_SEARCH_CONDITION_TYPE_RANGE) {
		// perform global range search

		if(action->settings.isGlobalSearch) {
			if(condition->isNotEqualTo) {
				log_logf(compiler->log, LOG_LEVEL_DEBUG, "{SEARCH} "
						"building UID global composite for '%s' "
						"from NOT search",
						condition->attribute);

				intersect_init(&notIntersect);

				searchEngine_searchNotListGlobal(engine,
						condition->attribute,
						&notIntersect);
			}

			log_logf(compiler->log, LOG_LEVEL_DEBUG, "{SEARCH} "
					"executing global search '%s' for '%s' search "
					"'%s' of %i (min), %i (max)",
					searchConditionTypeToString(condition->type),
					condition->attribute,
					rangeIndex_typeToString(condition->rangeType),
					condition->minValue,
					condition->maxValue);

			if(condition->isNotEqualTo) {
				searchEngine_searchRangeGlobal(engine,
						condition->attribute,
						condition->rangeType,
						condition->minValue,
						condition->maxValue,
						&notIntersect);

				intersect_execNot(&notIntersect, atrue);

				intersect_putArray(intersect,
						notIntersect.result.isSorted,
						notIntersect.result.array,
						notIntersect.result.length);

				intersect_free(&notIntersect);
			}
			else {
				searchEngine_searchRangeGlobal(engine,
						condition->attribute,
						condition->rangeType,
						condition->minValue,
						condition->maxValue,
						intersect);
			}

			return;
		}

		// perform domain-specific range search

		if(action->domainKeysLength > 1) {
			intersect_init(&localIntersect);
		}

		for(ii = 0; ii < action->domainKeysLength; ii++) {
			if(condition->isNotEqualTo) {
				log_logf(compiler->log, LOG_LEVEL_DEBUG, "{SEARCH} "
						"building UID composite on domain '%s' for '%s' "
						"from NOT search",
						action->domainKeys[ii],
						condition->attribute);

				intersect_init(&notIntersect);

				searchEngine_searchNotList(engine,
						action->domainKeys[ii],
						condition->attribute,
						&notIntersect);
			}

			log_logf(compiler->log, LOG_LEVEL_DEBUG, "{SEARCH} "
					"executing search '%s' on domain '%s' for '%s' "
					"search '%s' of %i (min), %i (max)",
					searchConditionTypeToString(condition->type),
					action->domainKeys[ii],
					condition->attribute,
					rangeIndex_typeToString(condition->rangeType),
					condition->minValue,
					condition->maxValue);

			if(condition->isNotEqualTo) {
				searchEngine_searchRange(engine,
						action->domainKeys[ii],
						condition->attribute,
						condition->rangeType,
						condition->minValue,
						condition->maxValue,
						&notIntersect);

				intersect_execNot(&notIntersect, atrue);

				if(action->domainKeysLength > 1) {
					intersect_putArray(&localIntersect,
							notIntersect.result.isSorted,
							notIntersect.result.array,
							notIntersect.result.length);
				}
				else {
					intersect_putArray(intersect,
							notIntersect.result.isSorted,
							notIntersect.result.array,
							notIntersect.result.length);
				}

				intersect_free(&notIntersect);
			}
			else if(action->domainKeysLength > 1) {
				searchEngine_searchRange(engine,
						action->domainKeys[ii],
						condition->attribute,
						condition->rangeType,
						condition->minValue,
						condition->maxValue,
						&localIntersect);
			}
			else {
				searchEngine_searchRange(engine,
						action->domainKeys[ii],
						condition->attribute,
						condition->rangeType,
						condition->minValue,
						condition->maxValue,
						intersect);
			}
		}

		if(action->domainKeysLength > 1) {
			intersect_execOr(&localIntersect);

			intersect_putArray(intersect,
					localIntersect.result.isSorted,
					localIntersect.result.array,
					localIntersect.result.length);

			intersect_free(&localIntersect);
		}
	}
	else if(condition->type ==
			SEARCH_COMPILER_SEARCH_CONDITION_TYPE_GEO_COORD) {
		// perform global geo-coord search

		if(action->settings.isGlobalSearch) {
			if(condition->isNotEqualTo) {
				log_logf(compiler->log, LOG_LEVEL_DEBUG, "{SEARCH} "
						"building UID global composite for 'geocoord' "
						"from NOT search");

				intersect_init(&geoIntersect);
				intersect_init(&notIntersect);

				searchEngine_searchNotListGlobal(engine, "latitude",
						&geoIntersect);
				searchEngine_searchNotListGlobal(engine, "longitude",
						&geoIntersect);

				intersect_execOr(&geoIntersect);

				intersect_putArray(&notIntersect,
						geoIntersect.result.isSorted,
						geoIntersect.result.array,
						geoIntersect.result.length);

				intersect_free(&geoIntersect);
			}

			log_logf(compiler->log, LOG_LEVEL_DEBUG, "{SEARCH} "
					"executing global search '%s', "
					"search distance %0.2f (%s) from (%0.6f,%0.6f)",
					searchConditionTypeToString(condition->type),
					condition->distance,
					searchEngine_geoCoordTypeToString(condition->geoCoordType),
					condition->latitude,
					condition->longitude);

			if(condition->isNotEqualTo) {
				searchEngine_searchRangeGeocoordGlobal(engine,
						condition->geoCoordType,
						condition->latitude,
						condition->longitude,
						condition->distance,
						&notIntersect);

				intersect_execNot(&notIntersect, atrue);

				intersect_putArray(intersect,
						notIntersect.result.isSorted,
						notIntersect.result.array,
						notIntersect.result.length);

				intersect_free(&notIntersect);
			}
			else {
				searchEngine_searchRangeGeocoordGlobal(engine,
						condition->geoCoordType,
						condition->latitude,
						condition->longitude,
						condition->distance,
						intersect);
			}

			return;
		}

		// perform domain-specific geo-cord search

		if(action->domainKeysLength > 1) {
			intersect_init(&localIntersect);
		}

		for(ii = 0; ii < action->domainKeysLength; ii++) {
			if(condition->isNotEqualTo) {
				log_logf(compiler->log, LOG_LEVEL_DEBUG, "{SEARCH} "
						"building UID composite on domain '%s' for '%s' "
						"from NOT search",
						action->domainKeys[ii],
						condition->attribute);

				intersect_init(&geoIntersect);
				intersect_init(&notIntersect);

				searchEngine_searchNotList(engine, action->domainKeys[ii],
						"latitude", &geoIntersect);
				searchEngine_searchNotList(engine, action->domainKeys[ii],
						"longitude", &geoIntersect);

				intersect_execOr(&geoIntersect);

				intersect_putArray(&notIntersect,
						geoIntersect.result.isSorted,
						geoIntersect.result.array,
						geoIntersect.result.length);

				intersect_free(&geoIntersect);
			}

			log_logf(compiler->log, LOG_LEVEL_DEBUG, "{SEARCH} "
					"executing search '%s' on domain '%s', "
					"search distance %0.2f (%s) from (%0.6f,%0.6f)",
					searchConditionTypeToString(condition->type),
					action->domainKeys[ii],
					condition->distance,
					searchEngine_geoCoordTypeToString(condition->geoCoordType),
					condition->latitude,
					condition->longitude);

			if(condition->isNotEqualTo) {
				searchEngine_searchRangeGeocoord(engine,
						action->domainKeys[ii],
						condition->geoCoordType,
						condition->latitude,
						condition->longitude,
						condition->distance,
						&notIntersect);

				intersect_execNot(&notIntersect, atrue);

				if(action->domainKeysLength > 1) {
					intersect_putArray(&localIntersect,
							notIntersect.result.isSorted,
							notIntersect.result.array,
							notIntersect.result.length);
				}
				else {
					intersect_putArray(intersect,
							notIntersect.result.isSorted,
							notIntersect.result.array,
							notIntersect.result.length);
				}

				intersect_free(&notIntersect);
			}
			else if(action->domainKeysLength > 1) {
				searchEngine_searchRangeGeocoord(engine,
						action->domainKeys[ii],
						condition->geoCoordType,
						condition->latitude,
						condition->longitude,
						condition->distance,
						&localIntersect);
			}
			else {
				searchEngine_searchRangeGeocoord(engine,
						action->domainKeys[ii],
						condition->geoCoordType,
						condition->latitude,
						condition->longitude,
						condition->distance,
						intersect);
			}
		}

		if(action->domainKeysLength > 1) {
			intersect_execOr(&localIntersect);

			intersect_putArray(intersect,
					localIntersect.result.isSorted,
					localIntersect.result.array,
					localIntersect.result.length);

			intersect_free(&localIntersect);
		}
	}
}

static void search_executeSearchRelevancySort(SearchActionSearch *action,
		SearchSort *sort)
{
	int ii = 0;
	int nn = 0;

	for(ii = 0; ii < action->groupsLength; ii++) {
		for(nn = 0; nn < action->groups[ii].conditionsLength; nn++) {
			if((action->groups[ii].conditions[nn].type !=
						SEARCH_COMPILER_SEARCH_CONDITION_TYPE_WILDCARD) &&
					(action->groups[ii].conditions[nn].type !=
					 SEARCH_COMPILER_SEARCH_CONDITION_TYPE_EXACT) &&
					(action->groups[ii].conditions[nn].type !=
					 SEARCH_COMPILER_SEARCH_CONDITION_TYPE_USER)) {
				continue;
			}

			searchSort_mvAddSortByRelevancy(sort,
					action->groups[ii].conditions[nn].attribute,
					action->groups[ii].conditions[nn].value,
					action->sort.directives[ii].isStandardSortOrder);
		}
	}
}

// action functions

// new domain

static int action_executeNewDomain(SearchCompiler *compiler,
		SearchEngine *engine)
{
	SearchActionNewDomain *action = NULL;

	if(compiler->action == NULL) {
		return -1;
	}

	action = (SearchActionNewDomain *)compiler->action;

	if((authSystem_isEnabled(&engine->auth)) &&
			(!authSystem_isPermitted(&engine->auth,
					compiler->token,
					NULL,
					compiler->ipAddress,
					AUTHENTICATION_PERMISSIONS_DOMAIN))) {
		return SEARCH_COMPILER_ERROR_EXEC_PERMISSION_DENIED;
	}

	return searchEngine_newDomain(engine, action->key, action->name);
}

static void action_freeNewDomain(SearchCompiler *compiler)
{
	SearchActionNewDomain *action = NULL;

	if(compiler->action == NULL) {
		return;
	}

	action = (SearchActionNewDomain *)compiler->action;

	if(action->key != NULL) {
		free(action->key);
	}

	if(action->name != NULL) {
		free(action->name);
	}

	free(action);
}

// new index

static int action_executeNewIndex(SearchCompiler *compiler,
		SearchEngine *engine)
{
	SearchActionNewIndex *action = NULL;

	if(compiler->action == NULL) {
		return -1;
	}

	action = (SearchActionNewIndex *)compiler->action;

	if((authSystem_isEnabled(&engine->auth)) &&
			(!authSystem_isPermitted(&engine->auth,
					compiler->token,
					NULL,
					compiler->ipAddress,
					AUTHENTICATION_PERMISSIONS_INDEX))) {
		return SEARCH_COMPILER_ERROR_EXEC_PERMISSION_DENIED;
	}

	return searchEngine_newIndex(engine, action->type, action->key,
			action->name);
}

static void action_freeNewIndex(SearchCompiler *compiler)
{
	SearchActionNewIndex *action = NULL;

	if(compiler->action == NULL) {
		return;
	}

	action = (SearchActionNewIndex *)compiler->action;

	if(action->key != NULL) {
		free(action->key);
	}

	if(action->name != NULL) {
		free(action->name);
	}

	free(action);
}

// get

static int action_executeGet(SearchCompiler *compiler,
		SearchEngine *engine)
{
	int rc = 0;

	SearchActionGet *action = NULL;

	if(compiler->action == NULL) {
		return -1;
	}

	action = (SearchActionGet *)compiler->action;

	action->engine = engine;

	if((authSystem_isEnabled(&engine->auth)) &&
			(!authSystem_isPermitted(&engine->auth,
					compiler->token,
					NULL,
					compiler->ipAddress,
					AUTHENTICATION_PERMISSIONS_GET))) {
		return SEARCH_COMPILER_ERROR_EXEC_PERMISSION_DENIED;
	}

	if((action->container = searchEngine_get(engine, action->uid,
					&rc)) == NULL) {
		if(rc < 0) {
			return rc;
		}
		return SEARCH_COMPILER_ERROR_EXEC_CONTAINER_DOES_NOT_EXIST;
	}

	return 0;
}

static void action_freeGet(SearchCompiler *compiler)
{
	SearchActionGet *action = NULL;

	if(compiler->action == NULL) {
		return;
	}

	action = (SearchActionGet *)compiler->action;

	if(action->container != NULL) {
		searchEngine_unlockGet(action->engine, action->container);
	}

	free(action);
}

// get by attribute

static int action_executeGetByAttribute(SearchCompiler *compiler,
		SearchEngine *engine)
{
	int rc = 0;

	SearchActionGetByAttribute *action = NULL;

	if(compiler->action == NULL) {
		return -1;
	}

	action = (SearchActionGetByAttribute *)compiler->action;

	action->engine = engine;

	if((authSystem_isEnabled(&engine->auth)) &&
			(!authSystem_isPermitted(&engine->auth,
					compiler->token,
					NULL,
					compiler->ipAddress,
					AUTHENTICATION_PERMISSIONS_GET))) {
		return SEARCH_COMPILER_ERROR_EXEC_PERMISSION_DENIED;
	}

	if((action->container = searchEngine_getByAttribute(engine,
					action->attribute, action->value, &rc)) == NULL) {
		if(rc < 0) {
			return rc;
		}
		return SEARCH_COMPILER_ERROR_EXEC_CONTAINER_DOES_NOT_EXIST;
	}

	return 0;
}

static void action_freeGetByAttribute(SearchCompiler *compiler)
{
	SearchActionGetByAttribute *action = NULL;

	if(compiler->action == NULL) {
		return;
	}

	action = (SearchActionGetByAttribute *)compiler->action;

	if(action->attribute != NULL) {
		free(action->attribute);
	}

	if(action->value != NULL) {
		free(action->value);
	}

	if(action->container != NULL) {
		searchEngine_unlockGet(action->engine, action->container);
	}

	free(action);
}

// put

static int action_executePut(SearchCompiler *compiler,
		SearchEngine *engine)
{
	SearchActionPut *action = NULL;

	if(compiler->action == NULL) {
		return -1;
	}

	action = (SearchActionPut *)compiler->action;

	if((authSystem_isEnabled(&engine->auth)) &&
			(!authSystem_isPermitted(&engine->auth,
					compiler->token,
					action->domainKey,
					compiler->ipAddress,
					AUTHENTICATION_PERMISSIONS_PUT))) {
		return SEARCH_COMPILER_ERROR_EXEC_PERMISSION_DENIED;
	}

	if((action->uid = searchEngine_put(engine, action->domainKey,
					action->container, action->isImmediate)) < 0) {
		return action->uid;
	}

	return 0;
}

static void action_freePut(SearchCompiler *compiler)
{
	SearchActionPut *action = NULL;

	if(compiler->action == NULL) {
		return;
	}

	action = (SearchActionPut *)compiler->action;

	if(action->domainKey != NULL) {
		free(action->domainKey);
	}

	free(action);
}

// update

static int action_executeUpdate(SearchCompiler *compiler,
		SearchEngine *engine)
{
	SearchActionUpdate *action = NULL;

	if(compiler->action == NULL) {
		return -1;
	}

	action = (SearchActionUpdate *)compiler->action;

	if((authSystem_isEnabled(&engine->auth)) &&
			(!authSystem_isPermitted(&engine->auth,
					compiler->token,
					action->domainKey,
					compiler->ipAddress,
					AUTHENTICATION_PERMISSIONS_UPDATE))) {
		return SEARCH_COMPILER_ERROR_EXEC_PERMISSION_DENIED;
	}

	if((action->uid = searchEngine_update(engine,
					action->domainKey,
					action->container,
					action->isImmediate,
					action->flags)) < 0) {
		return action->uid;
	}

	return 0;
}

static void action_freeUpdate(SearchCompiler *compiler)
{
	SearchActionUpdate *action = NULL;

	if(compiler->action == NULL) {
		return;
	}

	action = (SearchActionUpdate *)compiler->action;

	if(action->domainKey != NULL) {
		free(action->domainKey);
	}

	free(action);
}

// update by attribute

static int action_executeUpdateByAttribute(SearchCompiler *compiler,
		SearchEngine *engine)
{
	SearchActionUpdateByAttribute *action = NULL;

	if(compiler->action == NULL) {
		return -1;
	}

	action = (SearchActionUpdateByAttribute *)compiler->action;

	if((authSystem_isEnabled(&engine->auth)) &&
			(!authSystem_isPermitted(&engine->auth,
					compiler->token,
					action->domainKey,
					compiler->ipAddress,
					AUTHENTICATION_PERMISSIONS_UPDATE))) {
		return SEARCH_COMPILER_ERROR_EXEC_PERMISSION_DENIED;
	}

	if((action->uid = searchEngine_updateByAttribute(engine,
					action->domainKey,
					action->attribute,
					action->container,
					action->isImmediate,
					action->flags)) < 0) {
		return action->uid;
	}

	return 0;
}

static void action_freeUpdateByAttribute(SearchCompiler *compiler)
{
	SearchActionUpdateByAttribute *action = NULL;

	if(compiler->action == NULL) {
		return;
	}

	action = (SearchActionUpdateByAttribute *)compiler->action;

	if(action->attribute != NULL) {
		free(action->attribute);
	}

	if(action->domainKey != NULL) {
		free(action->domainKey);
	}

	free(action);
}

// delete

static int action_executeDelete(SearchCompiler *compiler,
		SearchEngine *engine)
{
	int rc = 0;

	SearchActionDelete *action = NULL;

	if(compiler->action == NULL) {
		return -1;
	}

	action = (SearchActionDelete *)compiler->action;

	if((authSystem_isEnabled(&engine->auth)) &&
			(!authSystem_isPermitted(&engine->auth,
					compiler->token,
					NULL,
					compiler->ipAddress,
					AUTHENTICATION_PERMISSIONS_DELETE))) {
		return SEARCH_COMPILER_ERROR_EXEC_PERMISSION_DENIED;
	}

	if((rc = searchEngine_delete(engine, action->uid)) < 0) {
		return rc;
	}

	return 0;
}

static void action_freeDelete(SearchCompiler *compiler)
{
	SearchActionDelete *action = NULL;

	if(compiler->action == NULL) {
		return;
	}

	action = (SearchActionDelete *)compiler->action;

	free(action);
}

// delete by attribute

static int action_executeDeleteByAttribute(SearchCompiler *compiler,
		SearchEngine *engine)
{
	int rc = 0;

	SearchActionDeleteByAttribute *action = NULL;

	if(compiler->action == NULL) {
		return -1;
	}

	action = (SearchActionDeleteByAttribute *)compiler->action;

	if((authSystem_isEnabled(&engine->auth)) &&
			(!authSystem_isPermitted(&engine->auth,
					compiler->token,
					NULL,
					compiler->ipAddress,
					AUTHENTICATION_PERMISSIONS_DELETE))) {
		return SEARCH_COMPILER_ERROR_EXEC_PERMISSION_DENIED;
	}

	if((rc = searchEngine_deleteByAttribute(engine, action->attribute,
					action->value)) < 0) {
		return rc;
	}

	return 0;
}

static void action_freeDeleteByAttribute(SearchCompiler *compiler)
{
	SearchActionDeleteByAttribute *action = NULL;

	if(compiler->action == NULL) {
		return;
	}

	action = (SearchActionDeleteByAttribute *)compiler->action;

	if(action->attribute != NULL) {
		free(action->attribute);
	}

	if(action->value != NULL) {
		free(action->value);
	}

	free(action);
}

// search

static int action_executeSearch(SearchCompiler *compiler,
		SearchEngine *engine)
{
	int ii = 0;
	int nn = 0;
	int result = 0;
	alint sortMemoryLength = 0;
	double timer = 0.0;
	char *domainKey = NULL;

	SearchSort sort;
	SearchActionSearch *action = NULL;

	if(compiler->action == NULL) {
		return -1;
	}

	action = (SearchActionSearch *)compiler->action;

	action->engine = engine;

	if(authSystem_isEnabled(&engine->auth)) {
		for(ii = 0; ii < action->domainKeysLength; ii++) {
			if(!strcmp(action->domainKeys[ii], "*")) {
				domainKey = NULL;
			}
			else {
				domainKey = action->domainKeys[ii];
			}

			if(!authSystem_isPermitted(&engine->auth,
						compiler->token,
						domainKey,
						compiler->ipAddress,
						AUTHENTICATION_PERMISSIONS_SEARCH)) {
				return SEARCH_COMPILER_ERROR_EXEC_PERMISSION_DENIED;
			}
		}
	}

	// perform search

	for(ii = 0; ii < action->groupsLength; ii++) {
		if(action->groups[ii].conditionsLength < 1) {
			intersect_init(&action->groups[ii].intersect);
			continue;
		}

		timer = time_getTimeMus();

		intersect_init(&action->groups[ii].intersect);

		for(nn = 0; nn < action->groups[ii].conditionsLength; nn++) {
			search_executeSearchCondition(compiler,
					engine,
					action,
					&action->groups[ii].conditions[nn],
					&action->groups[ii].intersect);
		}

		log_logf(compiler->log, LOG_LEVEL_DEBUG,
				"{SEARCH} performed group %i search in %0.6f seconds",
				ii, time_getElapsedMusInSeconds(timer));

		timer = time_getTimeMus();

		if(action->groups[ii].isAndIntersection) {
			intersect_execAnd(&action->groups[ii].intersect, atrue);

			log_logf(compiler->log, LOG_LEVEL_DEBUG,
					"{SEARCH} obtained %i results from AND intersection "
					"on group %i in %0.6f seconds",
					action->groups[ii].intersect.result.length, ii,
					time_getElapsedMusInSeconds(timer));
		}
		else {
			intersect_execOr(&action->groups[ii].intersect);

			log_logf(compiler->log, LOG_LEVEL_DEBUG,
					"{SEARCH} obtained %i results from OR intersection "
					"on group %i in %0.6f seconds",
					action->groups[ii].intersect.result.length, ii,
					time_getElapsedMusInSeconds(timer));
		}

/*
		intersect_display(stdout, &action->groups[ii].intersect);
 */
	}

	// perform intersection

	timer = time_getTimeMus();

	intersect_init(&action->intersect);

	for(ii = 0; ii < action->groupsLength; ii++) {
		if(action->groups[ii].conditionsLength < 1) {
			continue;
		}

		intersect_putArray(&action->intersect,
				action->groups[ii].intersect.result.isSorted,
				action->groups[ii].intersect.result.array,
				action->groups[ii].intersect.result.length);
	}

	if(action->settings.isAndIntersection) {
		intersect_execAnd(&action->intersect, atrue);

		log_logf(compiler->log, LOG_LEVEL_DEBUG,
				"{SEARCH} obtained %i results from AND intersection "
				"in %0.6f seconds",
				action->intersect.result.length,
				time_getElapsedMusInSeconds(timer));
	}
	else {
		intersect_execOr(&action->intersect);

		log_logf(compiler->log, LOG_LEVEL_DEBUG,
				"{SEARCH} obtained %i results from OR intersection "
				"in %0.6f seconds",
				action->intersect.result.length,
				time_getElapsedMusInSeconds(timer));
	}

/*
	intersect_display(stdout, &action->intersect);
*/

	// determine if any geocoord search pruning is required

	timer = time_getTimeMus();

	for(ii = 0; ii < action->groupsLength; ii++) {
		for(nn = 0; nn < action->groups[ii].conditionsLength; nn++) {
			if(action->groups[ii].conditions[nn].type ==
					SEARCH_COMPILER_SEARCH_CONDITION_TYPE_GEO_COORD) {
				search_pruneByDistance(compiler,
						engine,
						action->groups[ii].conditions[ii].latitude,
						action->groups[ii].conditions[ii].longitude,
						action->groups[ii].conditions[ii].distance,
						&action->intersect);
			}
		}
	}

	log_logf(compiler->log, LOG_LEVEL_DEBUG,
			"{SEARCH} distance-prune check completed in %0.6f seconds",
			time_getElapsedMusInSeconds(timer));

	// perform sorting

	if((action->sort.isSorted) &&
			(action->intersect.result.length > 1) &&
			(action->sort.directiveLength > 0) &&
			(action->sort.directives != NULL)) {
		timer = time_getTimeMus();

		if(engine->settings.maxSortOperationMemoryLength > 0) {
			sortMemoryLength = searchSort_calculateMvInitMemoryLength(
					&action->intersect);
		}

		searchSort_mvInit(&sort, engine, &action->intersect);

		for(ii = 0; ii < action->sort.directiveLength; ii++) {
			switch(action->sort.directives[ii].type) {
				case SEARCH_COMPILER_SEARCH_SORT_TYPE_RELEVANCY:
					if(engine->settings.maxSortOperationMemoryLength > 0) {
						sortMemoryLength +=
						searchSort_calculateMvAddSortByRelevancyMemoryLength(
								&sort);
					}

					if((engine->settings.maxSortOperationMemoryLength == 0) ||
							((engine->settings.maxSortOperationMemoryLength >
							  0) &&
							 (sortMemoryLength <=
							  engine->settings.maxSortOperationMemoryLength))) {
						search_executeSearchRelevancySort(action, &sort);
					}
					else {
						log_logf(compiler->log, LOG_LEVEL_WARNING,
								"{SEARCHD} aborted sort operation due to "
								"required memory higher than current limits "
								"(%lli vs %lli)",
								sortMemoryLength,
							  	engine->settings.maxSortOperationMemoryLength);

						result = SEARCH_COMPILER_ERROR_EXEC_SORT_OUT_OF_MEMORY;
					}
					break;

				case SEARCH_COMPILER_SEARCH_SORT_TYPE_ATTRIBUTE:
					if(engine->settings.maxSortOperationMemoryLength > 0) {
						sortMemoryLength +=
					searchSort_calculateMvAddSortByAttributeNameMemoryLength(
							&sort, action->sort.directives[ii].attribute,
							engine->settings.maxSortOperationMemoryLength);
					}

					if((engine->settings.maxSortOperationMemoryLength == 0) ||
							((engine->settings.maxSortOperationMemoryLength >
							  0) &&
							 (sortMemoryLength <=
							  engine->settings.maxSortOperationMemoryLength))) {
						searchSort_mvAddSortByAttributeName(&sort,
							action->sort.directives[ii].attribute,
							action->sort.directives[ii].isStandardSortOrder);
					}
					else {
						log_logf(compiler->log, LOG_LEVEL_WARNING,
								"{SEARCHD} aborted sort operation due to "
								"required memory higher than current limits "
								"(%lli vs %lli)",
								sortMemoryLength,
							  	engine->settings.maxSortOperationMemoryLength);

						result = SEARCH_COMPILER_ERROR_EXEC_SORT_OUT_OF_MEMORY;
					}
					break;

				case SEARCH_COMPILER_SEARCH_SORT_TYPE_GEO_COORD:
					if(engine->settings.maxSortOperationMemoryLength > 0) {
						sortMemoryLength +=
							searchSort_calculateMvAddSortByDistanceMemoryLength(
									&sort);
					}

					if((engine->settings.maxSortOperationMemoryLength == 0) ||
							((engine->settings.maxSortOperationMemoryLength >
							  0) &&
							 (sortMemoryLength <=
							  engine->settings.maxSortOperationMemoryLength))) {
						searchSort_mvAddSortByDistance(engine,
							&sort,
							action->sort.directives[ii].latitude,
							action->sort.directives[ii].longitude,
							action->sort.directives[ii].isStandardSortOrder);
					}
					else {
						log_logf(compiler->log, LOG_LEVEL_WARNING,
								"{SEARCHD} aborted sort operation due to "
								"required memory higher than current limits "
								"(%lli vs %lli)",
								sortMemoryLength,
							  	engine->settings.maxSortOperationMemoryLength);

						result = SEARCH_COMPILER_ERROR_EXEC_SORT_OUT_OF_MEMORY;
					}
					break;

				case SEARCH_COMPILER_SEARCH_SORT_TYPE_UNKNOWN:
				case SEARCH_COMPILER_SEARCH_SORT_TYPE_ERROR:
				default:
					log_logf(compiler->log, LOG_LEVEL_WARNING,
							"unknown sort type %i, aborting sort",
							action->sort.directives[ii].type);
			}
		}

		searchSort_mvExecute(&sort);
		searchSort_mvFree(&sort);

		log_logf(compiler->log, LOG_LEVEL_DEBUG,
				"{SEARCH} performed %ix sort on %i results in %0.6f seconds",
				action->sort.directiveLength,
				action->intersect.result.length,
				time_getElapsedMusInSeconds(timer));
	}

	return result;
}

static void action_freeSearchContext(SearchActionSearch *action)
{
	int ii = 0;
	int nn = 0;

	if(action->requestAttributes != NULL) {
		for(ii = 0; ii < action->requestAttributesLength; ii++) {
			if(action->requestAttributes[ii] != NULL) {
				free(action->requestAttributes[ii]);
			}
		}

		free(action->requestAttributes);
	}

	if(action->domainKeys != NULL) {
		for(ii = 0; ii < action->domainKeysLength; ii++) {
			if(action->domainKeys[ii] != NULL) {
				free(action->domainKeys[ii]);
			}
		}

		free(action->domainKeys);
	}

	intersect_free(&action->intersect);

	if(action->groups != NULL) {
		for(ii = 0; ii < action->groupsLength; ii++) {
			intersect_free(&action->groups[ii].intersect);

			if(action->groups[ii].conditions != NULL) {
				for(nn = 0; nn < action->groups[ii].conditionsLength; nn++) {
					if(action->groups[ii].conditions[nn].attribute != NULL) {
						free(action->groups[ii].conditions[nn].attribute);
					}
					if(action->groups[ii].conditions[nn].value != NULL) {
						free(action->groups[ii].conditions[nn].value);
					}
				}
	
				free(action->groups[ii].conditions);
			}
		}

		free(action->groups);
	}

	if(action->sort.directives != NULL) {
		for(ii = 0; ii < action->sort.directiveLength; ii++) {
			if(action->sort.directives[ii].attribute != NULL) {
				free(action->sort.directives[ii].attribute);
			}
		}

		free(action->sort.directives);
	}

	if(action->facets.attributes != NULL) {
		for(ii = 0; ii < action->facets.attributesLength; ii++) {
			if(action->facets.attributes[ii] != NULL) {
				free(action->facets.attributes[ii]);
			}
		}

		free(action->facets.attributes);
	}

	free(action);
}

static void action_freeSearch(SearchCompiler *compiler)
{
	if(compiler->action == NULL) {
		return;
	}

	action_freeSearchContext((SearchActionSearch *)compiler->action);
}

static void action_displaySearch(void *stream, SearchCompiler *compiler)
{
	int ii = 0;
	int nn = 0;

	SearchActionSearch *action = NULL;

	if(compiler->action == NULL) {
		return;
	}

	action = (SearchActionSearch *)compiler->action;

	fprintf(stream, "Search 0x%lx ::\n", (aptrcast)action);

	fprintf(stream, "\t settings:\n");

	fprintf(stream, "\t\t is global search     : %i\n",
			action->settings.isGlobalSearch);
	fprintf(stream, "\t\t is entire container  : %i\n",
			action->settings.isEntireContainer);
	fprintf(stream, "\t\t is AND intersection  : %i\n",
			action->settings.isAndIntersection);

	fprintf(stream, "\t attributes (%i):\n", action->requestAttributesLength);

	for(ii = 0; ii < action->requestAttributesLength; ii++) {
		fprintf(stream, "\t\t attribute %03i       : '%s'\n",
			ii, action->requestAttributes[ii]);
	}

	fprintf(stream, "\t domain keys (%i):\n", action->domainKeysLength);

	for(ii = 0; ii < action->domainKeysLength; ii++) {
		fprintf(stream, "\t\t domain key %03i      : '%s'\n",
			ii, action->domainKeys[ii]);
	}

	fprintf(stream, "\t groups (%i):\n", action->groupsLength);

	for(ii = 0; ii < action->groupsLength; ii++) {
		fprintf(stream, "\t\t group %03i ::\n", ii);
		fprintf(stream, "\t\t is AND intersection : %i\n",
				action->groups[ii].isAndIntersection);
		fprintf(stream, "\t\t conditions          : %i\n",
				action->groups[ii].conditionsLength);

		for(nn = 0; nn < action->groups[ii].conditionsLength; nn++) {
			fprintf(stream, "\t\t\t condition %03i ::\n", nn);

			fprintf(stream, "\t\t\t\t type            : '%s'\n",
					searchConditionTypeToString(
						action->groups[ii].conditions[nn].type));
			fprintf(stream, "\t\t\t\t range type      : '%s'\n",
					rangeIndex_typeToString(
						action->groups[ii].conditions[nn].rangeType));
			fprintf(stream, "\t\t\t\t geo coord type  : '%s'\n",
					searchEngine_geoCoordTypeToString(
						action->groups[ii].conditions[nn].geoCoordType));
			fprintf(stream, "\t\t\t\t is NOT equal to : %i\n",
					action->groups[ii].conditions[nn].isNotEqualTo);
			fprintf(stream, "\t\t\t\t min value       : %i\n",
					action->groups[ii].conditions[nn].minValue);
			fprintf(stream, "\t\t\t\t max value       : %i\n",
					action->groups[ii].conditions[nn].maxValue);
			fprintf(stream, "\t\t\t\t distance        : %0.2f\n",
					action->groups[ii].conditions[nn].distance);
			fprintf(stream, "\t\t\t\t latitude        : %0.6f\n",
					action->groups[ii].conditions[nn].latitude);
			fprintf(stream, "\t\t\t\t longitude       : %0.6f\n",
					action->groups[ii].conditions[nn].longitude);
			fprintf(stream, "\t\t\t\t attribute       : '%s'\n",
					action->groups[ii].conditions[nn].attribute);
			fprintf(stream, "\t\t\t\t value           : '%s'\n",
					action->groups[ii].conditions[nn].value);
		}
	}

	fprintf(stream, "\t sort:\n");

	fprintf(stream, "\t\t is sorted            : %i\n",
			action->sort.isSorted);
	fprintf(stream, "\t\t directives           : %i\n",
			action->sort.directiveLength);

	for(ii = 0; ii < action->sort.directiveLength; ii++) {
		fprintf(stream, "\t\t directive #%03i ::\n", ii);
		fprintf(stream, "\t\t\t is standard order    : %i\n",
				action->sort.directives[ii].isStandardSortOrder);
		fprintf(stream, "\t\t\t type                 : '%s'\n",
				searchSortTypeToString(action->sort.directives[ii].type));
		fprintf(stream, "\t\t\t latitude             : %0.2f\n",
				action->sort.directives[ii].latitude);
		fprintf(stream, "\t\t\t longitude            : %0.2f\n",
				action->sort.directives[ii].longitude);
		fprintf(stream, "\t\t\t attribute            : '%s'\n",
				action->sort.directives[ii].attribute);
	}

	fprintf(stream, "\t options:\n");

	fprintf(stream, "\t\t has options          : %i\n",
			action->options.hasResultOptions);
	fprintf(stream, "\t\t offset               : %i\n",
			action->options.offset);
	fprintf(stream, "\t\t limit                : %i\n",
			action->options.limit);
	fprintf(stream, "\t\t include distances    : %i\n",
			action->options.isIncludeDistances);
	fprintf(stream, "\t\t latitude             : %0.2f\n",
			action->options.latitude);
	fprintf(stream, "\t\t longitude            : %0.2f\n",
			action->options.longitude);

	fprintf(stream, "\t facets:\n");

	fprintf(stream, "\t\t display index summary: %i\n",
			action->facets.isDisplayIndexSummary);
	fprintf(stream, "\t\t display index facets : %i\n",
			action->facets.isDisplayFacets);

	fprintf(stream, "\t\t attributes (%i):\n",
			action->facets.attributesLength);

	for(ii = 0; ii < action->facets.attributesLength; ii++) {
		fprintf(stream, "\t\t\t attribute %03i       : '%s'\n",
			ii, action->facets.attributes[ii]);
	}
}

// list

static int action_executeList(SearchCompiler *compiler,
		SearchEngine *engine)
{
	int rc = 0;

	SearchActionList *action = NULL;

	if(compiler->action == NULL) {
		return -1;
	}

	action = (SearchActionList *)compiler->action;

	if((authSystem_isEnabled(&engine->auth)) &&
			(!authSystem_isPermitted(&engine->auth,
					compiler->token,
					NULL,
					compiler->ipAddress,
					AUTHENTICATION_PERMISSIONS_LIST))) {
		return SEARCH_COMPILER_ERROR_EXEC_PERMISSION_DENIED;
	}

	if(action->isListDomains) {
		action->domainList = searchEngine_listDomains(engine,
				&action->domainListLength, &rc);
	}

	if(action->isListIndexes) {
		action->indexList = searchEngine_listIndexes(engine,
				&action->indexListLength, &rc);
	}

	if(action->isListIndexDetails) {
		action->indexType = strdup(indexRegistry_typeToString(
					searchEngine_getIndexType(engine, action->indexKey)));
		action->indexCount = searchEngine_getIndexCount(
				engine, action->domainKey, action->indexKey);
		action->indexUniqueValueCount = searchEngine_getIndexUniqueValueCount(
				engine, action->domainKey, action->indexKey);
		action->indexDepth = searchEngine_getIndexDepth(
				engine, action->domainKey, action->indexKey);
	}

	action->engine = engine;

	return 0;
}

static void action_freeList(SearchCompiler *compiler)
{
	int ii = 0;

	SearchActionList *action = NULL;

	if(compiler->action == NULL) {
		return;
	}

	action = (SearchActionList *)compiler->action;

	if(action->indexKey != NULL) {
		free(action->indexKey);
	}

	if(action->indexType != NULL) {
		free(action->indexType);
	}

	if(action->domainKey != NULL) {
		free(action->domainKey);
	}

	if(action->domainList != NULL) {
		for(ii = 0; ii < action->domainListLength; ii++) {
			if(action->domainList[ii] != NULL) {
				free(action->domainList[ii]);
			}
		}
		free(action->domainList);
	}

	if(action->indexList != NULL) {
		for(ii = 0; ii < action->indexListLength; ii++) {
			if(action->indexList[ii] != NULL) {
				free(action->indexList[ii]);
			}
		}
		free(action->indexList);
	}

	free(action);
}

// command

static int action_executeCommand(SearchCompiler *compiler,
		SearchEngine *engine)
{
	int rc = 0;
	int stringLength = 0;
	int stringResultLength = 0;
	int delimitersLength = 0;
	char *string = NULL;
	char *indexName = NULL;

	IndexRegistryTypes type = INDEX_REGISTRY_TYPE_END_OF_LIST;
	IndexRegistryIndex *indexType = NULL;
	SearchActionCommand *action = NULL;
	AuthSystemPermissions permission = AUTHENTICATION_PERMISSIONS_END_OF_LIST;

	if(compiler->action == NULL) {
		return -1;
	}

	action = (SearchActionCommand *)compiler->action;

	if(authSystem_isEnabled(&engine->auth)) {
		switch(action->type) {
			case SEARCH_COMPILER_COMMAND_TYPE_INDEX:
				permission = AUTHENTICATION_PERMISSIONS_COMMAND_INDEX;
				break;

			case SEARCH_COMPILER_COMMAND_TYPE_DOMAIN:
				permission = AUTHENTICATION_PERMISSIONS_COMMAND_DOMAIN;
				break;

			case SEARCH_COMPILER_COMMAND_TYPE_SERVER:
				permission = AUTHENTICATION_PERMISSIONS_COMMAND_SERVER;
				break;

			case SEARCH_COMPILER_COMMAND_TYPE_ERROR:
			default:
				permission = AUTHENTICATION_PERMISSIONS_END_OF_LIST;
		}

		if(!authSystem_isPermitted(&engine->auth,
					compiler->token,
					NULL,
					compiler->ipAddress,
					permission)) {
			return SEARCH_COMPILER_ERROR_EXEC_PERMISSION_DENIED;
		}
	}

	switch(action->type) {
		case SEARCH_COMPILER_COMMAND_TYPE_INDEX:
			switch(action->action) {
				case SEARCH_COMPILER_COMMAND_ACTION_UPDATE:
					if((indexType = searchEngine_getIndex(engine,
									action->indexKey, &rc)) == NULL) {
						return rc;
					}

					stringLength = strlen(action->indexKey);
					string = strndup(action->indexKey, stringLength);

					searchEngineCommon_buildIndexString(string, stringLength,
							&stringResultLength);

					if(action->indexType != NULL) {
						type = indexRegistry_stringToType(action->indexType);
					}
					else {
						type = indexType->type;
					}

					if(type != indexType->type) {
						log_logf(engine->log, LOG_LEVEL_INFO,
								"{COMPILER} calling reset index type from "
								"'%s' to '%s' for { key '%s', name '%s' }",
								indexRegistry_typeToString(indexType->type),
								indexRegistry_typeToString(type),
								indexType->key,
								indexType->name);

						indexName = strdup(indexType->name);

						if((rc = searchEngine_resetIndex(engine,
										action->indexKey,
										type,
										action->indexKey,
										indexName)) < 0) {
							free(string);
							free(indexName);
							return rc;
						}

						free(indexName);
					}

					if(action->isOverrideIndexing) {
						log_logf(engine->log, LOG_LEVEL_INFO,
								"{COMPILER} calling set string indexing "
								"for '%s' to "
								"{ full-string %i, tokenized-string %i }",
								action->indexKey,
								(int)action->isIndexFullString,
								(int)action->isIndexTokenizedString);

						if((rc = indexRegistry_setStringIndexing(
										&engine->registry,
										string,
										action->isIndexFullString,
										action->isIndexTokenizedString)) < 0) {
							free(string);
							return rc;
						}
					}

					if((action->minStringLength > 0) &&
							(action->maxStringLength > 0) &&
							(action->minStringLength <=
							 action->maxStringLength) &&
							(action->maxStringLength <
							 SEARCH_ENGINE_CORE_MAX_STRING_INDEX_LENGTH)) {
						log_logf(engine->log, LOG_LEVEL_INFO,
								"{COMPILER} calling set string index "
								"thresholds for '%s' to { min %i, max %i }",
								action->indexKey,
								action->minStringLength,
								action->maxStringLength);

						if(indexType->type == INDEX_REGISTRY_TYPE_WILDCARD) {
							if((rc = searchEngine_setIndexStringThresholds(
											engine,
											string,
											action->minStringLength,
											action->maxStringLength)) < 0) {
								free(string);
								return rc;
							}
						}
						else {
							if((rc = indexRegistry_setStringIndexThresholds(
											&engine->registry,
											string,
											action->minStringLength,
											action->maxStringLength)) < 0) {
								free(string);
								return rc;
							}
						}
					}

					if((action->delimiters != NULL) &&
							((delimitersLength = strlen(
									action->delimiters)) > 0)) {
						log_logf(engine->log, LOG_LEVEL_INFO,
								"{COMPILER} calling set string delimiters "
								"for '%s' to { delimiters '%s' }",
								action->indexKey,
								action->delimiters);

						if((rc = indexRegistry_setStringDelimiters(
										&engine->registry,
										string,
										action->delimiters,
										delimitersLength)) < 0) {
							free(string);
							return rc;
						}
					}

					if(action->excludedWordFilename != NULL) {
						log_logf(engine->log, LOG_LEVEL_INFO,
								"{COMPILER} calling set excluded words "
								"for '%s' to { excluded words filename '%s' }",
								action->indexKey,
								action->excludedWordFilename);

						if((rc = indexRegistry_loadExcludedWords(
										&engine->registry,
										string,
										engine->log,
										&engine->esa,
										action->excludedWordFilename)) < 0) {
							free(string);
							return rc;
						}
					}

					free(string);
					break;

				case SEARCH_COMPILER_COMMAND_ACTION_RENAME:
					log_logf(engine->log, LOG_LEVEL_INFO,
							"{COMPILER} calling rename index on '%s' to "
							"{ key '%s', name '%s' }",
							action->indexKey,
							action->newIndexKey,
							action->newIndexName);

					if((rc = searchEngine_renameIndex(engine,
									action->indexKey,
									action->newIndexKey,
									action->newIndexName)) < 0) {
						return rc;
					}
					break;

				case SEARCH_COMPILER_COMMAND_ACTION_DELETE:
					log_logf(engine->log, LOG_LEVEL_INFO,
							"{COMPILER} calling delete index on '%s'",
							action->indexKey);

					if((rc = searchEngine_removeIndex(engine,
									action->indexKey)) < 0) {
						return rc;
					}
					break;

				case SEARCH_COMPILER_COMMAND_ACTION_COPY:
					/*
					 * TODO: this
					 */
					return SEARCH_COMPILER_ERROR_EXEC_FAILED;

				case SEARCH_COMPILER_COMMAND_ACTION_CHANGE_TYPE:
					if((indexType = searchEngine_getIndex(engine,
									action->indexKey, &rc)) == NULL) {
						return rc;
					}

					if(action->indexType != NULL) {
						type = indexRegistry_stringToType(action->indexType);
					}
					else {
						return SEARCH_COMPILER_ERROR_EXEC_FAILED;
					}

					if(type == indexType->type) {
						return SEARCH_COMPILER_ERROR_EXEC_FAILED;
					}

					log_logf(engine->log, LOG_LEVEL_INFO,
							"{COMPILER} calling reset index type from "
							"'%s' to '%s' for { key '%s', name '%s' }",
							indexRegistry_typeToString(indexType->type),
							indexRegistry_typeToString(type),
							indexType->key,
							indexType->name);

					indexName = strdup(indexType->name);

					if((rc = searchEngine_resetIndex(engine,
									action->indexKey,
									type,
									action->indexKey,
									indexName)) < 0) {
						free(indexName);
						return rc;
					}

					free(indexName);
					break;

				case SEARCH_COMPILER_COMMAND_ACTION_STOP:
				case SEARCH_COMPILER_COMMAND_ACTION_BACKUP:
				case SEARCH_COMPILER_COMMAND_ACTION_ERROR:
				default:
					return SEARCH_COMPILER_ERROR_EXEC_FAILED;
			}
			break;

		case SEARCH_COMPILER_COMMAND_TYPE_DOMAIN:
			switch(action->action) {
				case SEARCH_COMPILER_COMMAND_ACTION_RENAME:
					log_logf(engine->log, LOG_LEVEL_INFO,
							"{COMPILER} calling rename domain '%s' to "
							"{ key '%s', name '%s' }",
							action->domainKey,
							action->newDomainKey,
							action->newDomainName);

					if((rc = searchEngine_renameDomain(engine,
									action->domainKey,
									action->newDomainKey,
									action->newDomainName)) < 0) {
						return rc;
					}
					break;

				case SEARCH_COMPILER_COMMAND_ACTION_DELETE:
					log_logf(engine->log, LOG_LEVEL_INFO,
							"{COMPILER} calling remove domain '%s'",
							action->domainKey);

					if((rc = searchEngine_removeDomain(engine,
									action->domainKey)) < 0) {
						return rc;
					}
					break;

				case SEARCH_COMPILER_COMMAND_ACTION_COPY:
					/*
					 * TODO: this
					 */
					return SEARCH_COMPILER_ERROR_EXEC_FAILED;

				case SEARCH_COMPILER_COMMAND_ACTION_UPDATE:
				case SEARCH_COMPILER_COMMAND_ACTION_CHANGE_TYPE:
				case SEARCH_COMPILER_COMMAND_ACTION_STOP:
				case SEARCH_COMPILER_COMMAND_ACTION_BACKUP:
				case SEARCH_COMPILER_COMMAND_ACTION_ERROR:
				default:
					return SEARCH_COMPILER_ERROR_EXEC_FAILED;
			}
			break;

		case SEARCH_COMPILER_COMMAND_TYPE_SERVER:
			switch(action->action) {
				case SEARCH_COMPILER_COMMAND_ACTION_STOP:
					/*
					 * TODO: this
					 */
					return SEARCH_COMPILER_ERROR_EXEC_FAILED;

				case SEARCH_COMPILER_COMMAND_ACTION_BACKUP:
					/*
					 * TODO: this
					 */
					return SEARCH_COMPILER_ERROR_EXEC_FAILED;

				case SEARCH_COMPILER_COMMAND_ACTION_UPDATE:
				case SEARCH_COMPILER_COMMAND_ACTION_RENAME:
				case SEARCH_COMPILER_COMMAND_ACTION_DELETE:
				case SEARCH_COMPILER_COMMAND_ACTION_CHANGE_TYPE:
				case SEARCH_COMPILER_COMMAND_ACTION_COPY:
				case SEARCH_COMPILER_COMMAND_ACTION_ERROR:
				default:
					return SEARCH_COMPILER_ERROR_EXEC_FAILED;
			}
			break;

		case SEARCH_COMPILER_COMMAND_TYPE_ERROR:
		default:
			return SEARCH_COMPILER_ERROR_EXEC_FAILED;
	}

	return 0;
}

static void action_freeCommand(SearchCompiler *compiler)
{
	SearchActionCommand *action = NULL;

	if(compiler->action == NULL) {
		return;
	}

	action = (SearchActionCommand *)compiler->action;

	if(action->indexKey != NULL) {
		free(action->indexKey);
	}

	if(action->indexName != NULL) {
		free(action->indexName);
	}

	if(action->indexType != NULL) {
		free(action->indexType);
	}

	if(action->newIndexKey != NULL) {
		free(action->newIndexKey);
	}

	if(action->newIndexName != NULL) {
		free(action->newIndexName);
	}

	if(action->domainKey != NULL) {
		free(action->domainKey);
	}

	if(action->newDomainKey != NULL) {
		free(action->newDomainKey);
	}

	if(action->newDomainName != NULL) {
		free(action->newDomainName);
	}

	if(action->delimiters != NULL) {
		free(action->delimiters);
	}

	if(action->excludedWordFilename != NULL) {
		free(action->excludedWordFilename);
	}

	free(action);
}

// login

static int action_executeLogin(SearchCompiler *compiler,
		SearchEngine *engine)
{
	AuthSystemUser *user = NULL;
	SearchActionLogin *action = NULL;

	if(compiler->action == NULL) {
		return -1;
	}

	action = (SearchActionLogin *)compiler->action;

	if(!authSystem_isEnabled(&engine->auth)) {
		return SEARCH_COMPILER_ERROR_EXEC_AUTHENTICATION_OFFLINE;
	}

	if((user = authSystem_userLogin(&engine->auth, action->username,
					action->password)) == NULL) {
		return -1;
	}

	action->token = strndup(user->token, user->tokenLength);

	return 0;
}

static void action_freeLogin(SearchCompiler *compiler)
{
	SearchActionLogin *action = NULL;

	if(compiler->action == NULL) {
		return;
	}

	action = (SearchActionLogin *)compiler->action;

	if(action->username != NULL) {
		free(action->username);
	}

	if(action->password != NULL) {
		free(action->password);
	}

	if(action->token != NULL) {
		free(action->token);
	}

	free(action);
}

// logout

static int action_executeLogout(SearchCompiler *compiler,
		SearchEngine *engine)
{
	SearchActionLogout *action = NULL;

	if(compiler->action == NULL) {
		return -1;
	}

	action = (SearchActionLogout *)compiler->action;

	if(!authSystem_isEnabled(&engine->auth)) {
		return SEARCH_COMPILER_ERROR_EXEC_AUTHENTICATION_OFFLINE;
	}

	if(!authSystem_userLogout(&engine->auth, action->token)) {
		return -1;
	}

	return 0;
}

static void action_freeLogout(SearchCompiler *compiler)
{
	SearchActionLogout *action = NULL;

	if(compiler->action == NULL) {
		return;
	}

	action = (SearchActionLogout *)compiler->action;

	if(action->token != NULL) {
		free(action->token);
	}

	free(action);
}


// define search compiler public functions

int searchCompiler_compileJson(SearchCompiler *compiler, Log *log,
		char *ipAddress, SearchJsonValidationType validationType,
		Json *message)
{
	char *string = NULL;
	char *messageName = NULL;

	if((compiler == NULL) || (message == NULL)) {
		DISPLAY_INVALID_ARGS;
		return SEARCH_COMPILER_ERROR_DEFAULT;
	}

	memset(compiler, 0, (int)(sizeof(SearchCompiler)));

	if(ipAddress == NULL) {
		compiler->ipAddress = strndup("0.0.0.0", 7);
	}
	else {
		compiler->ipAddress = strdup(ipAddress);
	}

	if((json_elementExists(message, "token")) &&
			((string = json_getString(message, "token")) != NULL)) {
		compiler->token = strdup(string);
	}

	compiler->log = log;

	if((messageName = json_getString(message, "messageName")) == NULL) {
		return returnError(compiler,
				SEARCH_COMPILER_ERROR_JSON_MISSING_MESSAGE_NAME);
	}

	if(!strcmp(messageName, "newDomain")) {
		if(!searchJson_compileNewDomain(compiler, validationType, message)) {
			return returnError(compiler,
					SEARCH_COMPILER_ERROR_JSON_COMPILE_FAILED);
		}
	}
	else if(!strcmp(messageName, "newIndex")) {
		if(!searchJson_compileNewIndex(compiler, validationType, message)) {
			return returnError(compiler,
					SEARCH_COMPILER_ERROR_JSON_COMPILE_FAILED);
		}
	}
	else if(!strcmp(messageName, "get")) {
		if(!searchJson_compileGet(compiler, validationType, message)) {
			return returnError(compiler,
					SEARCH_COMPILER_ERROR_JSON_COMPILE_FAILED);
		}
	}
	else if(!strcmp(messageName, "getByAttribute")) {
		if(!searchJson_compileGetByAttribute(compiler, validationType,
					message)) {
			return returnError(compiler,
					SEARCH_COMPILER_ERROR_JSON_COMPILE_FAILED);
		}
	}
	else if(!strcmp(messageName, "put")) {
		if(!searchJson_compilePut(compiler, validationType, message)) {
			return returnError(compiler,
					SEARCH_COMPILER_ERROR_JSON_COMPILE_FAILED);
		}
	}
	else if((!strcmp(messageName, "update")) ||
			(!strcmp(messageName, "append")) ||
			(!strcmp(messageName, "uppend")) ||
			(!strcmp(messageName, "replace"))) {
		if(!searchJson_compileUpdate(compiler, validationType, message)) {
			return returnError(compiler,
					SEARCH_COMPILER_ERROR_JSON_COMPILE_FAILED);
		}
	}
	else if((!strcmp(messageName, "updateByAttribute")) ||
			(!strcmp(messageName, "appendByAttribute")) ||
			(!strcmp(messageName, "uppendByAttribute")) ||
			(!strcmp(messageName, "replaceByAttribute"))) {
		if(!searchJson_compileUpdateByAttribute(compiler, validationType,
					message)) {
			return returnError(compiler,
					SEARCH_COMPILER_ERROR_JSON_COMPILE_FAILED);
		}
	}
	else if(!strcmp(messageName, "delete")) {
		if(!searchJson_compileDelete(compiler, validationType, message)) {
			return returnError(compiler,
					SEARCH_COMPILER_ERROR_JSON_COMPILE_FAILED);
		}
	}
	else if(!strcmp(messageName, "deleteByAttribute")) {
		if(!searchJson_compileDeleteByAttribute(compiler, validationType,
					message)) {
			return returnError(compiler,
					SEARCH_COMPILER_ERROR_JSON_COMPILE_FAILED);
		}
	}
	else if(!strcmp(messageName, "search")) {
		if(!searchJson_compileSearch(compiler, validationType, message)) {
			if(compiler->action != NULL) {
				action_freeSearchContext(compiler->action);
				compiler->action = NULL;
			}
			return returnError(compiler,
					SEARCH_COMPILER_ERROR_JSON_COMPILE_FAILED);
		}

		if(SEARCHD_DEVELOPER_MODE) {
			action_displaySearch(stdout, compiler);
		}
	}
	else if(!strcmp(messageName, "list")) {
		if(!searchJson_compileList(compiler, validationType, message)) {
			return returnError(compiler,
					SEARCH_COMPILER_ERROR_JSON_COMPILE_FAILED);
		}
	}
	else if(!strcmp(messageName, "command")) {
		if(!searchJson_compileCommand(compiler, validationType, message)) {
			return returnError(compiler,
					SEARCH_COMPILER_ERROR_JSON_COMPILE_FAILED);
		}
	}
	else if(!strcmp(messageName, "login")) {
		if(!searchJson_compileLogin(compiler, validationType, message)) {
			return returnError(compiler,
					SEARCH_COMPILER_ERROR_JSON_COMPILE_FAILED);
		}
	}
	else if(!strcmp(messageName, "logout")) {
		if(!searchJson_compileLogout(compiler, validationType, message)) {
			return returnError(compiler,
					SEARCH_COMPILER_ERROR_JSON_COMPILE_FAILED);
		}
	}
	else {
		return returnError(compiler,
				SEARCH_COMPILER_ERROR_JSON_INVALID_MESSAGE_NAME);
	}

	compiler->isCompileComplete = atrue;

	return 0;
}

int searchCompiler_compileSql(SearchCompiler *compiler, Log *log, char *query)
{
	int ii = 0;
	int result = 0;
	int queryLength = 0;
	int tokenListLength = 0;
	int *tokenLengths = NULL;
	char **tokens = NULL;

	if((compiler == NULL) || (query == NULL) ||
			((queryLength = strlen(query)) < 1)) {
		DISPLAY_INVALID_ARGS;
		return SEARCH_COMPILER_ERROR_DEFAULT;
	}

	memset(compiler, 0, (int)(sizeof(SearchCompiler)));

	compiler->log = log;

	if(((tokens = string_tokenizeQuery(query, queryLength, &tokenLengths,
						&tokenListLength)) == NULL) ||
			(tokenLengths == NULL) ||
			(tokenListLength < 1)) {
		if(tokens != NULL) {
			for(ii = 0; ii < tokenListLength; ii++) {
				if(tokens[ii] != NULL) {
					free(tokens[ii]);
				}
			}
			free(tokens);
		}

		if(tokenLengths != NULL) {
			free(tokenLengths);
		}

		return SEARCH_COMPILER_ERROR_SQL_TOKENIZE_FAILED;
	}

/*	{
		for(ii = 0; ii < tokenListLength; ii++) {
			printf("\t TOKEN %03i => '%s'\n", ii, tokens[ii]);
		}
	}*/

	if(!strcasecmp(tokens[0], "create")) {
		if(!strcasecmp(tokens[1], "domain")) {
			if(!searchSql_compileNewDomain(compiler, tokens, tokenLengths,
						tokenListLength)) {
				result = returnError(compiler,
						SEARCH_COMPILER_ERROR_SQL_COMPILE_FAILED);
			}
		}
		else if(!strcasecmp(tokens[1], "index")) {
		}
		else {
			result = returnError(compiler,
					SEARCH_COMPILER_ERROR_SQL_UNRECOGNIZED_COMMAND);
		}
	}
	else if(!strcasecmp(tokens[0], "select")) {
	}
	else if(!strcasecmp(tokens[0], "insert")) {
	}
	else if(!strcasecmp(tokens[0], "update")) {
	}
	else if(!strcasecmp(tokens[0], "delete")) {
	}
	else if(!strcasecmp(tokens[0], "list")) {
	}
	else {
		result = returnError(compiler,
				SEARCH_COMPILER_ERROR_SQL_UNRECOGNIZED_COMMAND);
	}

	compiler->isCompileComplete = atrue;

	// cleanup

	for(ii = 0; ii < tokenListLength; ii++) {
		if(tokens[ii] != NULL) {
			free(tokens[ii]);
		}
	}

	free(tokens);
	free(tokenLengths);

	return result;
}

void searchCompiler_free(SearchCompiler *compiler)
{
	if(compiler == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	switch(compiler->type) {
		case SEARCH_COMPILER_ACTION_NEW_DOMAIN:
			action_freeNewDomain(compiler);
			break;

		case SEARCH_COMPILER_ACTION_NEW_INDEX:
			action_freeNewIndex(compiler);
			break;

		case SEARCH_COMPILER_ACTION_GET:
			action_freeGet(compiler);
			break;

		case SEARCH_COMPILER_ACTION_GET_BY_ATTRIBUTE:
			action_freeGetByAttribute(compiler);
			break;

		case SEARCH_COMPILER_ACTION_PUT:
			action_freePut(compiler);
			break;

		case SEARCH_COMPILER_ACTION_UPDATE:
			action_freeUpdate(compiler);
			break;

		case SEARCH_COMPILER_ACTION_UPDATE_BY_ATTRIBUTE:
			action_freeUpdateByAttribute(compiler);
			break;

		case SEARCH_COMPILER_ACTION_DELETE:
			action_freeDelete(compiler);
			break;

		case SEARCH_COMPILER_ACTION_DELETE_BY_ATTRIBUTE:
			action_freeDeleteByAttribute(compiler);
			break;

		case SEARCH_COMPILER_ACTION_SEARCH:
			action_freeSearch(compiler);
			break;

		case SEARCH_COMPILER_ACTION_LIST:
			action_freeList(compiler);
			break;

		case SEARCH_COMPILER_ACTION_COMMAND:
			action_freeCommand(compiler);
			break;

		case SEARCH_COMPILER_ACTION_LOGIN:
			action_freeLogin(compiler);
			break;

		case SEARCH_COMPILER_ACTION_LOGOUT:
			action_freeLogout(compiler);
			break;

		case SEARCH_COMPILER_ACTION_ERROR:
		default:
			return;
	}

	if(compiler->token != NULL) {
		free(compiler->token);
	}

	if(compiler->ipAddress != NULL) {
		free(compiler->ipAddress);
	}

	if(compiler->errorMessage != NULL) {
		free(compiler->errorMessage);
	}

	memset(compiler, 0, (int)(sizeof(SearchCompiler)));
}

int searchCompiler_execute(SearchCompiler *compiler, SearchEngine *engine)
{
	int rc = 0;
	int stringLength = 0;
	char *string = NULL;

	if((compiler == NULL) || (engine == NULL)) {
		DISPLAY_INVALID_ARGS;
		return SEARCH_COMPILER_ERROR_DEFAULT;
	}

	if(!compiler->isCompileComplete) {
		return returnError(compiler,
				SEARCH_COMPILER_ERROR_EXEC_COMPILE_INCOMPLETE);
	}

	switch(compiler->type) {
		case SEARCH_COMPILER_ACTION_NEW_DOMAIN:
			log_logf(compiler->log, LOG_LEVEL_INFO,
					"{COMPILER} action '%s' executing for "
					"{ key '%s', name '%s' }",
					actionToString(compiler->type),
					((SearchActionNewDomain *)compiler->action)->key,
					((SearchActionNewDomain *)compiler->action)->name);

			rc = action_executeNewDomain(compiler, engine);
			break;

		case SEARCH_COMPILER_ACTION_NEW_INDEX:
			log_logf(compiler->log, LOG_LEVEL_INFO,
					"{COMPILER} action '%s' executing for "
					"{ type '%s', key '%s', name '%s' }",
					actionToString(compiler->type),
					indexRegistry_typeToString(
						((SearchActionNewIndex *)compiler->action)->type),
					((SearchActionNewIndex *)compiler->action)->key,
					((SearchActionNewIndex *)compiler->action)->name);

			rc = action_executeNewIndex(compiler, engine);
			break;

		case SEARCH_COMPILER_ACTION_GET:
			log_logf(compiler->log, LOG_LEVEL_INFO,
					"{COMPILER} action '%s' executing for "
					"{ uid %i }",
					actionToString(compiler->type),
					((SearchActionGet *)compiler->action)->uid);

			rc = action_executeGet(compiler, engine);
			break;

		case SEARCH_COMPILER_ACTION_GET_BY_ATTRIBUTE:
			log_logf(compiler->log, LOG_LEVEL_INFO,
					"{COMPILER} action '%s' executing for "
					"{ attr '%s', value '%s' }",
					actionToString(compiler->type),
					((SearchActionGetByAttribute *)compiler->action)->attribute,
					((SearchActionGetByAttribute *)compiler->action)->value);

			rc = action_executeGetByAttribute(compiler, engine);
			break;

		case SEARCH_COMPILER_ACTION_PUT:
			log_logf(compiler->log, LOG_LEVEL_INFO,
					"{COMPILER} action '%s' executing for "
					"{ domain '%s' }",
					actionToString(compiler->type),
					((SearchActionPut *)compiler->action)->domainKey);

			rc = action_executePut(compiler, engine);
			break;

		case SEARCH_COMPILER_ACTION_UPDATE:
			log_logf(compiler->log, LOG_LEVEL_INFO,
					"{COMPILER} action '%s' executing for "
					"{ uid %i }",
					actionToString(compiler->type),
					((SearchActionUpdate *)compiler->action)->uid);

			rc = action_executeUpdate(compiler, engine);
			break;

		case SEARCH_COMPILER_ACTION_UPDATE_BY_ATTRIBUTE:
			string = container_getString(
					((SearchActionUpdateByAttribute *)
					 compiler->action)->container,
					((SearchActionUpdateByAttribute *)
					 compiler->action)->attribute,
					strlen(((SearchActionUpdateByAttribute *)
					 compiler->action)->attribute),
					&stringLength);

			log_logf(compiler->log, LOG_LEVEL_INFO,
					"{COMPILER} action '%s' executing for "
					"{ domain '%s', attribute '%s', value '%s' }",
					actionToString(compiler->type),
					((SearchActionUpdateByAttribute *)
					 compiler->action)->domainKey,
					((SearchActionUpdateByAttribute *)
					 compiler->action)->attribute,
					string);

			free(string);

			rc = action_executeUpdateByAttribute(compiler, engine);
			break;

		case SEARCH_COMPILER_ACTION_DELETE:
			log_logf(compiler->log, LOG_LEVEL_INFO,
					"{COMPILER} action '%s' executing for "
					"{ uid %i }",
					actionToString(compiler->type),
					((SearchActionDelete *)compiler->action)->uid);

			rc = action_executeDelete(compiler, engine);
			break;

		case SEARCH_COMPILER_ACTION_DELETE_BY_ATTRIBUTE:
			log_logf(compiler->log, LOG_LEVEL_INFO,
					"{COMPILER} action '%s' executing for "
					"{ attribute '%s', value '%s' }",
					actionToString(compiler->type),
					((SearchActionDeleteByAttribute *)
					 compiler->action)->attribute,
					((SearchActionDeleteByAttribute *)
					 compiler->action)->value);

			rc = action_executeDeleteByAttribute(compiler, engine);
			break;

		case SEARCH_COMPILER_ACTION_SEARCH:
			rc = action_executeSearch(compiler, engine);
			break;

		case SEARCH_COMPILER_ACTION_LIST:
			log_logf(compiler->log, LOG_LEVEL_INFO,
					"{COMPILER} action '%s' executing",
					actionToString(compiler->type));

			rc = action_executeList(compiler, engine);
			break;

		case SEARCH_COMPILER_ACTION_COMMAND:
			log_logf(compiler->log, LOG_LEVEL_INFO,
					"{COMPILER} action '%s' type '%s'->'%s' executing",
					actionToString(compiler->type),
					commandTypeToString(
						((SearchActionCommand *)compiler->action)->type),
					commandActionToString(
						((SearchActionCommand *)compiler->action)->action));

			rc = action_executeCommand(compiler, engine);
			break;

		case SEARCH_COMPILER_ACTION_LOGIN:
			log_logf(compiler->log, LOG_LEVEL_INFO,
					"{COMPILER} action '%s' executing",
					actionToString(compiler->type));

			rc = action_executeLogin(compiler, engine);
			break;

		case SEARCH_COMPILER_ACTION_LOGOUT:
			log_logf(compiler->log, LOG_LEVEL_INFO,
					"{COMPILER} action '%s' executing",
					actionToString(compiler->type));

			rc = action_executeLogout(compiler, engine);
			break;

		case SEARCH_COMPILER_ACTION_ERROR:
		default:
			return returnError(compiler,
					SEARCH_COMPILER_ERROR_EXEC_UNKNOWN_ACTION);
	}

	if(rc < 0) {
		return returnSearchEngineError(compiler, engine, rc);
	}

	compiler->isExecutionComplete = atrue;

	return 0;
}

Json *searchCompiler_buildResponse(SearchCompiler *compiler)
{
	char *string = NULL;
	char *errorMessage = NULL;

	Json *result = NULL;

	if(compiler == NULL) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	switch(compiler->type) {
		case SEARCH_COMPILER_ACTION_NEW_DOMAIN:
			if((result = searchJson_buildNewDomainResult(compiler)) == NULL) {
				errorMessage = "failed to build new domain result";
			}
			break;

		case SEARCH_COMPILER_ACTION_NEW_INDEX:
			if((result = searchJson_buildNewIndexResult(compiler)) == NULL) {
				errorMessage = "failed to build new index result";
			}
			break;

		case SEARCH_COMPILER_ACTION_GET:
			if((result = searchJson_buildGetResult(compiler)) == NULL) {
				errorMessage = "failed to build get result";
			}
			break;

		case SEARCH_COMPILER_ACTION_GET_BY_ATTRIBUTE:
			if((result = searchJson_buildGetByAttributeResult(
							compiler)) == NULL) {
				errorMessage = "failed to build get-by-attribute result";
			}
			break;

		case SEARCH_COMPILER_ACTION_PUT:
			if((result = searchJson_buildPutResult(compiler)) == NULL) {
				errorMessage = "failed to build put result";
			}
			break;

		case SEARCH_COMPILER_ACTION_UPDATE:
			if((result = searchJson_buildUpdateResult(compiler)) == NULL) {
				errorMessage = "failed to build update result";
			}
			break;

		case SEARCH_COMPILER_ACTION_UPDATE_BY_ATTRIBUTE:
			if((result = searchJson_buildUpdateByAttributeResult(
							compiler)) == NULL) {
				errorMessage = "failed to build update-by-attribute result";
			}
			break;

		case SEARCH_COMPILER_ACTION_DELETE:
			if((result = searchJson_buildDeleteResult(compiler)) == NULL) {
				errorMessage = "failed to build delete result";
			}
			break;

		case SEARCH_COMPILER_ACTION_DELETE_BY_ATTRIBUTE:
			if((result = searchJson_buildDeleteByAttributeResult(
							compiler)) == NULL) {
				errorMessage = "failed to build delete-by-attribute result";
			}
			break;

		case SEARCH_COMPILER_ACTION_SEARCH:
			if((result = searchJson_buildSearchResult(compiler)) == NULL) {
				errorMessage = "failed to build search result";
			}
			break;

		case SEARCH_COMPILER_ACTION_LIST:
			if((result = searchJson_buildListResult(compiler)) == NULL) {
				errorMessage = "failed to build list result";
			}
			break;

		case SEARCH_COMPILER_ACTION_COMMAND:
			if((result = searchJson_buildCommandResult(compiler)) == NULL) {
				errorMessage = "failed to build command result";
			}
			break;

		case SEARCH_COMPILER_ACTION_LOGIN:
			if((result = searchJson_buildLoginResult(compiler)) == NULL) {
				errorMessage = "failed to build login result";
			}
			break;

		case SEARCH_COMPILER_ACTION_LOGOUT:
			if((result = searchJson_buildLogoutResult(compiler)) == NULL) {
				errorMessage = "failed to build logout result";
			}
			break;

		case SEARCH_COMPILER_ACTION_ERROR:
		default:
			result = NULL;
			errorMessage = "unknown action";
	}

	if(result == NULL) {
		result = json_new();

		json_addString(result, "messageName", "errorMessage");

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
			json_addString(result, "resultCode", "-1");

			if(compiler->errorMessage != NULL) {
				json_addString(result, "errorMessage", compiler->errorMessage);
			}
			else if(errorMessage != NULL) {
				json_addString(result, "errorMessage", errorMessage);
			}
			else {
				json_addString(result, "errorMessage", "unknown error");
			}
		}
	}
	else {
		if((compiler->errorMessage != NULL) &&
				(!json_elementExists(result, "errorMessage"))) {
			json_addString(result, "warningMessage", compiler->errorMessage);
		}
	}

	return result;
}

// helper functions for compiler extensions

void searchCompiler_buildErrorMessage(SearchCompiler *compiler,
		const char *template, int errorCode, char *errorMessage)
{
	int length = 0;
	char *string = NULL;
	char *buffer = NULL;

	length = (strlen((char *)template) + strlen(errorMessage) + 8);

	string = (char *)malloc(sizeof(char) * (length + 1));

	snprintf(string, length, template, errorMessage);

	if(compiler->errorMessage != NULL) {
		length = (strlen(string) + strlen(compiler->errorMessage) + 8);

		buffer = (char *)malloc(sizeof(char) * (length + 1));

		snprintf(buffer, length, "%s (%s)", string, compiler->errorMessage);

		free(compiler->errorMessage);
		free(string);

		compiler->errorMessage = buffer;
	}
	else {
		compiler->errorMessage = string;
	}

	compiler->errorCode = errorCode;

	log_logf(compiler->log, LOG_LEVEL_ERROR, "{COMPILER} {%s} (%i) %s",
			actionToString(compiler->type),
			compiler->errorCode,
			compiler->errorMessage);
}

char *searchCompiler_actionToString(SearchCompilerActions action)
{
	return actionToString(action);
}

char *searchCompiler_commandTypeToString(SearchCompilerCommandTypes type)
{
	return commandTypeToString(type);
}

char *searchCompiler_commandActionToString(SearchCompilerCommandActions action)
{
	return commandActionToString(action);
}

