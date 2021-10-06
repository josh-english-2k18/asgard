/*
 * search_index.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * A system component for the search-engine to maintain the set of exact,
 * wildcard, and integer indexes for the search-engine system, header file.
 *
 * Written by Josh English.
 */

#if !defined(_SEARCH_SYSTEM_SEARCH_INDEX_H)

#define _SEARCH_SYSTEM_SEARCH_INDEX_H

#if !defined(_SEARCH_H) && !defined(_SEARCH_SYSTEM_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _SEARCH_H || _SEARCH_SYSTEM_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// define search index public data types

typedef struct _SearchIndexSummary {
	int length;
	int *valueCounts;
	char *type;
	char **keys;
} SearchIndexSummary;

typedef struct _SearchIndexEntry {
	IndexRegistryTypes type;
	char *key;
	void *index;
	Mutex mutex;
} SearchIndexEntry;

typedef struct _SearchIndex {
	int minStringLength;
	int maxStringLength;
	char *key;
	char *name;
	Bptree index;
	Spinlock lock;
} SearchIndex;


// delcare search index public functions

int searchIndex_init(SearchIndex *index, int minStringLength,
		int maxStringLength, char *key, char *name);

SearchIndex *searchIndex_new(int minStringLength, int maxStringLength,
		char *key, char *name);

int searchIndex_free(SearchIndex *index);

int searchIndex_freePtr(SearchIndex *index);

aboolean searchIndex_isLocked(SearchIndex *index);

aboolean searchIndex_isReadLocked(SearchIndex *index);

int searchIndex_lock(SearchIndex *index);

int searchIndex_unlock(SearchIndex *index);

int searchIndex_rename(SearchIndex *index, char *key, char *name);

int searchIndex_setStringThresholdsindexNew(SearchIndex *index,
		 IndexRegistryIndex *type, int minStringLength, int maxStringLength);

int searchIndex_getIndexCount(SearchIndex *index, IndexRegistryIndex *type);

int searchIndex_getIndexDepth(SearchIndex *index, IndexRegistryIndex *type);

int searchIndex_getIndexValueCount(SearchIndex *index,
		IndexRegistryIndex *type);

int searchIndex_buildIndexValueSummary(SearchIndex *index,
		IndexRegistryIndex *type, SearchIndexSummary *summary);

int searchIndex_buildAllIndexValueUids(SearchIndex *index,
		IndexRegistryIndex *type, Intersect *intersect);

int searchIndex_newIndex(SearchIndex *index, IndexRegistryIndex *type);

int searchIndex_removeIndex(SearchIndex *index, IndexRegistryIndex *type);

int searchIndex_renameIndex(SearchIndex *index, char *key,
		IndexRegistryIndex *type);

int *searchIndex_getString(SearchIndex *index, IndexRegistryIndex *type,
		char *value, int *length);

int *searchIndex_getRange(SearchIndex *index, IndexRegistryIndex *type,
		int searchType, int minValue, int maxValue, int *length);

int searchIndex_putString(SearchIndex *index, IndexRegistryIndex *type,
		char *value, int uid);

int searchIndex_putInteger(SearchIndex *index, IndexRegistryIndex *type,
		int value, int uid);

int searchIndex_removeString(SearchIndex *index, IndexRegistryIndex *type,
		char *value, int uid);

int searchIndex_removeInteger(SearchIndex *index, IndexRegistryIndex *type,
		int value, int uid);


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _SEARCH_SYSTEM_SEARCH_INDEX_H

