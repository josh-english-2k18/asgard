/*
 * index_registry.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * A library component for the search-engine to define the set of indexes
 * maintained by the search-engine system, header file.
 *
 * Written by Josh English.
 */

#if !defined(_SEARCH_CORE_INDEX_REGISTRY_H)

#define _SEARCH_CORE_INDEX_REGISTRY_H

#if !defined(_SEARCH_H) && !defined(_SEARCH_CORE_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _SEARCH_H || _SEARCH_CORE_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// define index registry public constants

typedef enum _IndexRegistryTypes {
	INDEX_REGISTRY_TYPE_EXACT = 1,
	INDEX_REGISTRY_TYPE_WILDCARD,
	INDEX_REGISTRY_TYPE_RANGE,
	INDEX_REGISTRY_TYPE_USER_KEY,
	INDEX_REGISTRY_TYPE_END_OF_LIST = -1
} IndexRegistryTypes;


// define index registry public data types

typedef struct _IndexRegistryIndexSetting {
	aboolean isOverrideStringLengths;
	aboolean isOverrideDelimiters;
	aboolean isOverrideExcludedWords;
	aboolean isOverrideIndexing;
	aboolean isIndexFullString;
	aboolean isIndexTokenizedString;
	int minStringLength;
	int maxStringLength;
	int delimitersLength;
	char *delimiters;
	char *excludedWordConfigFilename;
	Bptree *excludedWords;
	Mutex mutex;
} IndexRegistryIndexSetting;

typedef struct _IndexRegistryIndex {
	IndexRegistryTypes type;
	int uid;
	int keyLength;
	int nameLength;
	char *key;
	char *name;
	IndexRegistryIndexSetting settings;
} IndexRegistryIndex;

typedef struct _IndexRegistry {
	int uidCounter;
	Bptree index;
	Spinlock lock;
} IndexRegistry;


// delcare index registry public functions

int indexRegistry_init(IndexRegistry *registry);

IndexRegistry *indexRegistry_new();

int indexRegistry_free(IndexRegistry *registry);

int indexRegistry_freePtr(IndexRegistry *registry);

char *indexRegistry_typeToString(IndexRegistryTypes type);

IndexRegistryTypes indexRegistry_stringToType(char *string);

aboolean indexRegistry_isLocked(IndexRegistry *registry);

aboolean indexRegistry_isReadLocked(IndexRegistry *registry);

int indexRegistry_lock(IndexRegistry *registry);

int indexRegistry_unlock(IndexRegistry *registry);

int indexRegistry_put(IndexRegistry *registry, IndexRegistryTypes type,
		char *key, char *name);

IndexRegistryIndex *indexRegistry_get(IndexRegistry *registry, char *key);

int indexRegistry_remove(IndexRegistry *registry, char *key);

int indexRegistry_reset(IndexRegistry *registry, char *key,
		IndexRegistryTypes newType, char *newKey, char *newName);

// settings functions

int indexRegistry_setStringIndexThresholds(IndexRegistry *registry, char *key,
		int minStringLength, int maxStringLength);

int indexRegistry_setStringDelimiters(IndexRegistry *registry, char *key,
		char *delimiters, int delimitersLength);

int indexRegistry_loadExcludedWords(IndexRegistry *registry, char *key,
		Log *log, Esa *esa, char *filename);

int indexRegistry_setStringIndexing(IndexRegistry *registry, char *key,
		aboolean isIndexFullString, aboolean isIndexTokenizedString);

// helper functions

int indexRegistry_updateConfigContext(IndexRegistry *registry, char *key,
		Config *config, char *section);

int indexRegistry_updateSettingsFromConfig(IndexRegistry *registry, char *key,
		Config *config, char *section, Log *log, Esa *esa);


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _SEARCH_CORE_INDEX_REGISTRY_H

