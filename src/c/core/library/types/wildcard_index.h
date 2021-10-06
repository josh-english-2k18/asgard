/*
 * wildcard_index.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * A library to provide an API for building and searching against wildcard
 * indexes from input strings, header file.
 *
 * Written by Josh English.
 */

#if !defined(_CORE_LIBRARY_TYPES_WILDCARD_INDEX_H)

#define _CORE_LIBRARY_TYPES_WILDCARD_INDEX_H

#if !defined(_CORE_H) && !defined(_CORE_LIBRARY_H) && \
		!defined(_CORE_LIBRARY_TYPES_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _CORE_H || _CORE_LIBRARY_H || _CORE_LIBRARY_TYPES_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// define wildcard index public constants

typedef enum _WildcardIndexTypes {
	WILDCARD_INDEX_TYPE_STANDARD = 1,
	WILDCARD_INDEX_TYPE_EXACT,
	WILDCARD_INDEX_TYPE_UNKNOWN = -1
} WildcardIndexTypes;


// define wildcard index public data types

typedef struct _WildcardIndex {
	int minStringLength;
	int maxStringLength;
	alint ticks;
	int workspaceLength;
	char *workspace;
	Bptree index;
} WildcardIndex;


// declare wildcard index public functions

int wildcardIndex_init(WildcardIndex *wildcard, int minStringLength,
		int maxStringLength);

int wildcardIndex_free(WildcardIndex *wildcard);

alint wildcardIndex_getTicks(WildcardIndex *wildcard);

int wildcardIndex_getIndexCount(WildcardIndex *wildcard);

int wildcardIndex_getIndexDepth(WildcardIndex *wildcard);

int wildcardIndex_setStringThresholds(WildcardIndex *wildcard,
		int minStringLength, int maxStringLength);

int *wildcardIndex_get(WildcardIndex *wildcard, char *string, int *length);

int wildcardIndex_put(WildcardIndex *wildcard, int type, char *string,
		int index);

int wildcardIndex_remove(WildcardIndex *wildcard, char *string, int index);


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _CORE_LIBRARY_TYPES_WILDCARD_INDEX_H

