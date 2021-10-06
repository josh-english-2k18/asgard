/*
 * common.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * A library component for the search-engine to provide an API of common
 * functions, header file.
 *
 * Written by Josh English.
 */

#if !defined(_SEARCH_CORE_COMMON_H)

#define _SEARCH_CORE_COMMON_H

#if !defined(_SEARCH_H) && !defined(_SEARCH_CORE_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _SEARCH_H || _SEARCH_CORE_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// define common public constants

typedef enum _SearchEngineDataTypes {
	SEARCH_ENGINE_DATA_TYPE_BOOLEAN = 1,
	SEARCH_ENGINE_DATA_TYPE_INTEGER,
	SEARCH_ENGINE_DATA_TYPE_BIG_INTEGER,
	SEARCH_ENGINE_DATA_TYPE_DOUBLE,
	SEARCH_ENGINE_DATA_TYPE_STRING,
	SEARCH_ENGINE_DATA_TYPE_BINARY,
	SEARCH_ENGINE_DATA_TYPE_ERROR = -1
} SearchEngineDataTypes;


// delcare common public functions

int searchEngineCommon_buildIndexString(char *string, int stringLength,
		int *stringResultLength);

int searchEngineCommon_normalizeString(char *string, int stringLength,
		int *stringResultLength);

int searchEngineCommon_buildIndexKey(char *indexName, int indexNameLength,
		char *containerName, int containerNameLength, char key[128],
		int *keyLength);

int searchEngineCommon_buildUidKey(int uid, char key[128], int *keyLength);

char *searchEngineCommon_buildRelevancyKey(char *typeKey, int typeKeyLength);

char *searchEngineCommon_buildRelevancyLengthKey(char *typeKey,
		int typeKeyLength);

SearchEngineDataTypes searchEngineCommon_determineDataType(char *string,
		int stringLength);

int searchEngineCommong_padString(char *string, int stringLength,
		int stringAllocatedLength, int minStringLength,
		int *stringResultLength);

char **searchEngineCommon_stringToTokens(int minStringLength, char *delimiters,
		int delimitersLength, Esa *esa, char *string, int stringLength,
		int **resultLength, int *resultCount);

int searchEngineCommon_loadExcludedWords(Log *log, Esa *esa, char *filename,
		Bptree *index);


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _SEARCH_CORE_COMMON_H

