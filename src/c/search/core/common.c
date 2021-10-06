/*
 * common.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * A library component for the search-engine to provide an API of common
 * functions.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#include "search/search.h"
#define _SEARCH_CORE_COMPONENT
#include "search/core/common.h"


// define common public functions

int searchEngineCommon_buildIndexString(char *string, int stringLength,
		int *stringResultLength)
{
	int ii = 0;
	int nn = 0;

	if((string == NULL) || (stringLength < 1) || (stringResultLength == NULL)) {
		DISPLAY_INVALID_ARGS;

		if(stringResultLength != NULL) {
			*stringResultLength = 0;
		}

		return -1;
	}

	for(ii = 0, nn = 0; ii < stringLength; ii++) {
		if(!ctype_isPlainText(string[ii])) {
			continue;
		}
		else if(ctype_isWhitespace(string[ii])) {
			continue;
		}
		else if(ctype_isUpper(string[ii])) {
			string[nn] = ctype_toLower(string[ii]);
			nn++;
		}
		else {
			string[nn] = string[ii];
			nn++;
		}
	}

	string[nn] = '\0';

	*stringResultLength = nn;

	return 0;
}

int searchEngineCommon_normalizeString(char *string, int stringLength,
		int *stringResultLength)
{
	int ii = 0;
	int nn = 0;

	if((string == NULL) || (stringLength < 1) || (stringResultLength == NULL)) {
		DISPLAY_INVALID_ARGS;

		if(stringResultLength != NULL) {
			*stringResultLength = 0;
		}

		return -1;
	}

	for(ii = 0, nn = 0; ii < stringLength; ii++) {
		if(!ctype_isPlainText(string[ii])) {
			if((nn > 0) && (!ctype_isWhitespace(string[(nn - 1)]))) {
				string[nn] = ' ';
				nn++;
			}
			continue;
		}

		if(ctype_isLower(string[ii])) {
			string[nn] = string[ii];
			nn++;
		}
		else if(ctype_isUpper(string[ii])) {
			string[nn] = ctype_toLower(string[ii]);
			nn++;
		}
		else if(ctype_isNumeric(string[ii])) {
			string[nn] = string[ii];
			nn++;
		}
		else if((nn > 0) && (!ctype_isWhitespace(string[(nn - 1)]))) {
			string[nn] = ' ';
			nn++;
		}
	}

	if((nn > 0) && (ctype_isWhitespace(string[(nn - 1)]))) {
		nn--;
	}

	string[nn] = '\0';

	*stringResultLength = nn;

	return 0;
}

int searchEngineCommon_buildIndexKey(char *indexName, int indexNameLength,
		char *containerName, int containerNameLength, char key[128],
		int *keyLength)
{
	int ref = 0;
	int partOneLength = 0;
	int partTwoLength = 0;

	if((indexName == NULL) || (indexNameLength < 1) ||
			(containerName == NULL) || (containerNameLength < 1) ||
			(keyLength == NULL)) {
		DISPLAY_INVALID_ARGS;

		if(keyLength != NULL) {
			*keyLength = 0;
		}

		return -1;
	}

	if(indexNameLength > 59) {
		partOneLength = 60;
	}
	else {
		partOneLength = indexNameLength;
	}

	if(containerNameLength > 59) {
		partTwoLength = 60;
	}
	else {
		partTwoLength = containerNameLength;
	}

	ref = 0;

	memcpy((key + ref), "//", 2);

	ref += 2;

	memcpy((key + ref), indexName, partOneLength);

	ref += partOneLength;

	memcpy((key + ref), "/", 1);

	ref += 1;

	memcpy((key + ref), containerName, partTwoLength);

	ref += partTwoLength;

	key[ref] = '\0';

	*keyLength = ref;

	return 0;
}

int searchEngineCommon_buildUidKey(int uid, char key[128], int *keyLength)
{
	if(keyLength == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	*keyLength = snprintf(key, (((int)sizeof(char) * 128) - 1), "%i", uid);

	return 0;
}

char *searchEngineCommon_buildRelevancyKey(char *typeKey, int typeKeyLength)
{
	int length = 0;
	char *result = NULL;

	if((typeKey == NULL) || (typeKeyLength < 1)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	length = (typeKeyLength + 15 + 1);
	result = (char *)malloc(sizeof(char) * (length + 1));

	snprintf(result, length, "%s_relevancyIndex", typeKey);

	return result;
}

char *searchEngineCommon_buildRelevancyLengthKey(char *typeKey,
		int typeKeyLength)
{
	int length = 0;
	char *result = NULL;

	if((typeKey == NULL) || (typeKeyLength < 1)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	length = (typeKeyLength + 21 + 1);
	result = (char *)malloc(sizeof(char) * (length + 1));

	snprintf(result, length, "%s_relevancyIndexLength", typeKey);

	return result;
}

SearchEngineDataTypes searchEngineCommon_determineDataType(char *string,
		int stringLength)
{
	aboolean isNumeric = atrue;
	int ii = 0;
	int decimalCounter = 0;

	if((string == NULL) || (stringLength < 1)) {
		DISPLAY_INVALID_ARGS;
		return SEARCH_ENGINE_DATA_TYPE_ERROR;
	}

	if((!strcasecmp(string, "true")) ||
			(!strcasecmp(string, "false"))) {
		return SEARCH_ENGINE_DATA_TYPE_BOOLEAN;
	}

	for(ii = 0; ii < stringLength; ii++) {
		if(!ctype_isPlainText(string[ii])) {
			return SEARCH_ENGINE_DATA_TYPE_BINARY;
		}

		if((isNumeric) &&
				((ctype_isNumeric(string[ii])) || (string[ii] == '.'))) {
			if(string[ii] == '.') {
				decimalCounter++;
			}
		}
		else {
			isNumeric = afalse;
		}
	}

	if(isNumeric) {
		if(decimalCounter == 0) {
			if(stringLength <= 10) {
				return SEARCH_ENGINE_DATA_TYPE_INTEGER;
			}
			else {
				return SEARCH_ENGINE_DATA_TYPE_BIG_INTEGER;
			}
		}
		else if(decimalCounter == 1) {
			return SEARCH_ENGINE_DATA_TYPE_DOUBLE;
		}
	}

	return SEARCH_ENGINE_DATA_TYPE_STRING;
}

int searchEngineCommong_padString(char *string, int stringLength,
		int stringAllocatedLength, int minStringLength,
		int *stringResultLength)
{
	int ii = 0;

	if((string == NULL) ||
			(stringLength < 0) ||
			(stringLength >= stringAllocatedLength) ||
			(stringAllocatedLength < 1) ||
			(stringAllocatedLength < minStringLength) ||
			(minStringLength < 1) ||
			(stringResultLength == NULL)) {
		DISPLAY_INVALID_ARGS;

		if(stringResultLength != NULL) {
			*stringResultLength = 0;
		}

		return -1;
	}

	*stringResultLength = 0;

	if(stringLength >= minStringLength) {
		return 0;
	}

	for(ii = stringLength;
			((ii < stringAllocatedLength) && (ii < minStringLength));
			ii++) {
		string[ii] = '0';
	}

	string[ii] = '\0';

	*stringResultLength = ii;

	return 0;
}

char **searchEngineCommon_stringToTokens(int minStringLength, char *delimiters,
		int delimitersLength, Esa *esa, char *string, int stringLength,
		int **resultLength, int *resultCount)
{
	int ii = 0;
	int resultRef = 0;
	int tokenCount = 0;
	int tokenResultLength = 0;
	int localStringLength = 0;
	int localStringResultLength = 0;
	int *tokenLengths = NULL;
	int *resultStringLength = NULL;
	char *token = NULL;
	char *localString = NULL;
	char **tokenList = NULL;
	char **result = NULL;

	if((minStringLength < 1) || (delimiters == NULL) ||
			(delimitersLength < 1) || (esa == NULL) || (string == NULL) ||
			(stringLength < 1) || (stringLength < minStringLength) ||
			(resultLength == NULL) || (resultCount == NULL)) {
		DISPLAY_INVALID_ARGS;

		if(resultLength != NULL) {
			*resultLength = NULL;
		}
		if(resultCount != NULL) {
			*resultCount = 0;
		}

		return NULL;
	}

	*resultLength = NULL;
	*resultCount = 0;

	localString = strndup(string, stringLength);
	localStringLength = stringLength;

	if(searchEngineCommon_normalizeString(localString,
				localStringLength,
				&localStringResultLength) < 0) {
		free(localString);
		return NULL;
	}

	localStringLength = localStringResultLength;

	if(localStringLength < minStringLength) {
		free(localString);
		return NULL;
	}

	if(strtotokens(localString,
				localStringLength,
				delimiters,
				delimitersLength,
				minStringLength,
				&tokenList,
				&tokenLengths,
				&tokenCount) < 0) {
		free(localString);
		return NULL;
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

		free(localString);

		return NULL;
	}

	free(localString);

	result = (char **)malloc(sizeof(char *) * tokenCount);
	resultStringLength = (int *)malloc(sizeof(int) * tokenCount);

	for(ii = 0; ii < tokenCount; ii++) {
		if(tokenList[ii] == NULL) {
			continue;
		}

		localString = tokenList[ii];
		localStringLength = tokenLengths[ii];
		localStringResultLength = 0;

		if(localStringLength < minStringLength) {
			free(localString);
			continue;
		}

		if(searchEngineCommon_normalizeString(localString, localStringLength,
					&localStringResultLength) < 0) {
			free(localString);
			continue;
		}

		localStringLength = localStringResultLength;

		if(localStringLength < minStringLength) {
			free(localString);
			continue;
		}

		if((!cstring_containsVowel(localString, localStringLength)) ||
				(!cstring_containsAlphabetic(localString, localStringLength))) {
			free(localString);
			continue;
		}

		if((token = esa_stemToken(esa, localString,
						localStringLength, &tokenResultLength)) == NULL) {
			free(localString);
			continue;
		}

		result[resultRef] = token;
		resultStringLength[resultRef] = tokenResultLength;
		resultRef++;

		free(localString);
	}

	free(tokenList);
	free(tokenLengths);

	*resultLength = resultStringLength;
	*resultCount = resultRef;

	return result;
}

int searchEngineCommon_loadExcludedWords(Log *log, Esa *esa, char *filename,
		Bptree *index)
{
	int ii = 0;
	int counter = 0;
	int wordCount = 0;
	int tokenLength = 0;
	int stringLength = 0;
	char property[128];
	char *token = NULL;
	char *value = NULL;
	char *string = NULL;

	Config config;

	if((esa == NULL) || (filename == NULL) || (index == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(config_init(&config, filename) < 0) {
		log_logf(log, LOG_LEVEL_ERROR,
				"failed to initialize configuration file '%s'",
				filename);
		return -1;
	}

	config_getInteger(&config, "excluded.words", "wordCount", 0, &wordCount);

	if(wordCount < 1) {
		config_free(&config);
		return 0;
	}

	for(ii = 0; ii < wordCount; ii++) {
		snprintf(property, ((int)sizeof(property) - 1), "word%03i", ii);

		if(config_getString(&config, "excluded.words", property, "",
					&string) < 0) {
			continue;
		}

		if((stringLength = strlen(string)) < 1) {
			continue;
		}

		value = strdup(string);

		if(bptree_put(index, string, stringLength, (void *)value) < 0) {
			free(value);
		}
		else {
			counter++;
		}

		if((token = esa_stemToken(esa, string, stringLength,
						&tokenLength)) == NULL) {
			continue;
		}

		if(tokenLength < 1) {
			free(token);
			continue;
		}

		value = strdup(token);

		if(bptree_put(index, token, tokenLength, (void *)value) < 0) {
			free(value);
		}
		else {
			counter++;
		}

		free(token);
	}

	log_logf(log, LOG_LEVEL_INFO,
			"loaded %i excluded search terms into the exclusion index",
			counter);

	config_free(&config);

	return 0;
}

