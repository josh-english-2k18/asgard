/*
 * managed_index.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard core library background thread managed indexing system, using a
 * B+Tree index, type library, header file.
 *
 * Written by Josh English.
 */

#if !defined(_CORE_LIBRARY_TYPES_MANAGED_INDEX_H)

#define _CORE_LIBRARY_TYPES_MANAGED_INDEX_H

#if !defined(_CORE_H) && !defined(_CORE_LIBRARY_H) && \
		!defined(_CORE_LIBRARY_TYPES_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _CORE_H || _CORE_LIBRARY_H || _CORE_LIBRARY_TYPES_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// define managed index public constants

#define MANAGED_INDEX_DEFAULT_MAX_CACHED_ITEM_COUNT				2048

#define MANAGED_INDEX_DEFAULT_MAX_CACHED_MEMORY_LENGTH			2097152

#define MANAGED_INDEX_DEFAULT_TIMEOUT_SECONDS					300.0



// declare managed index public data types

typedef void (*ManagedIndexFreeFunction)(void *argument, void *memory);

typedef struct _ManagedIndexSimpleEntry {
	int length;
	char *value;
} ManagedIndexSimpleEntry;

typedef struct _ManagedIndexEntry {
	aboolean isSimpleEntry;
	int keyLength;
	int memorySize;
	alint ticks;
	double timestamp;
	char *key;
	void *value;
} ManagedIndexEntry;

typedef struct _ManagedIndex {
	aboolean isSpinlockMode;
	alint cachedItemCount;
	alint maxCachedItemCount;
	alint cachedMemoryLength;
	alint maxCachedMemoryLength;
	double timeoutSeconds;
	void *freeArgument;
	ManagedIndexFreeFunction freeFunction;
	Bptree index;
	TmfContext tmf;
	Spinlock spinlock;
	Mutex dataMutex;
	Mutex mutex;
} ManagedIndex;


// declare managed index public functions

int managedIndex_init(ManagedIndex *index, aboolean isSpinlockMode);

int managedIndex_free(ManagedIndex *index);

int managedIndex_setFreeFunction(ManagedIndex *index, void *argument,
		ManagedIndexFreeFunction freeFunction);

int managedIndex_setMaxCachedItemCount(ManagedIndex *index,
		alint maxCachedItemCount);

int managedIndex_setMaxCachedMemoryLength(ManagedIndex *index,
		alint maxCachedMemoryLength);

int managedIndex_setTimeout(ManagedIndex *index, double timeoutSeconds);

int managedIndex_getCacheStatus(ManagedIndex *index, alint *cachedItemCount,
		alint *cachedMemoryLength);

int managedIndex_getLeafCount(ManagedIndex *index, int *leafCount);

int managedIndex_clear(ManagedIndex *index);

int managedIndex_get(ManagedIndex *index, char *key, int keyLength,
		void **value);

int managedIndex_getSimple(ManagedIndex *index, char *key, int keyLength,
		char **value, int *valueLength);

int managedIndex_getNext(ManagedIndex *index, char *key, int keyLength,
		void **value);

int managedIndex_getNextSimple(ManagedIndex *index, char *key, int keyLength,
		char **value, int *valueLength);

int managedIndex_getPrevious(ManagedIndex *index, char *key, int keyLength,
		void **value);

int managedIndex_getPreviousSimple(ManagedIndex *index, char *key,
		int keyLength, char **value, int *valueLength);

int managedIndex_put(ManagedIndex *index, char *key, int keyLength,
		void *value, int memorySizeOfValue);

int managedIndex_putSimple(ManagedIndex *index, char *key, int keyLength,
		char *value, int valueLength);

int managedIndex_remove(ManagedIndex *index, char *key, int keyLength);


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _CORE_LIBRARY_TYPES_MANAGED_INDEX_H

