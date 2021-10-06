/*
 * range_index.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * A library to provide an API for building and range-searching against a
 * numeric index, header file.
 *
 * Written by Josh English.
 */

#if !defined(_CORE_LIBRARY_TYPES_RANGE_INDEX_H)

#define _CORE_LIBRARY_TYPES_RANGE_INDEX_H

#if !defined(_CORE_H) && !defined(_CORE_LIBRARY_H) && \
		!defined(_CORE_LIBRARY_TYPES_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _CORE_H || _CORE_LIBRARY_H || _CORE_LIBRARY_TYPES_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// define range index public constants

typedef enum _RangeIndexSearchTypes {
	RANGE_INDEX_SEARCH_GREATER_THAN = 1,
	RANGE_INDEX_SEARCH_LESS_THAN,
	RANGE_INDEX_SEARCH_BETWEEN,
	RANGE_INDEX_SEARCH_UNKNOWN = -1
} RangeIndexSearchTypes;


// define range index public data types

typedef struct _RangeIndexEntry {
	int value;
	char *key;
	IntegerArray list;
} RangeIndexEntry;

typedef struct _RangeIndex {
	char *name;
	Bptree index;
} RangeIndex;


// declare range index public functions

int rangeIndex_init(RangeIndex *range, char *name);

int rangeIndex_free(RangeIndex *range);

char *rangeIndex_typeToString(RangeIndexSearchTypes type);

char *rangeIndex_getName(RangeIndex *range);

int rangeIndex_setName(RangeIndex *range, char *name);

int *range_get(RangeIndex *range, int value, int *length);

int rangeIndex_put(RangeIndex *range, int value, int uid);

int rangeIndex_remove(RangeIndex *range, int value, int uid);

int *rangeIndex_search(RangeIndex *range, int type, int minValue, int maxValue,
		int *length);


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _CORE_LIBRARY_TYPES_RANGE_INDEX_H

