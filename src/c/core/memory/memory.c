/*
 * memory.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard memory-wrapping functions.
 *
 * Written by Josh English.
 */

// preprocessor directives

#define _CORE_COMMON_COMPONENT
#include "core/common/common.h"
#include "core/common/macros.h"
#include "core/common/version.h"
#include "core/common/string.h"
#define _CORE_OS_COMPONENT
#include "core/os/dl.h"
#include "core/os/time.h"
#include "core/os/types.h"
#define _CORE_SYSTEM_COMPONENT
#include "core/system/mutex.h"


// detect compiler environment


#if defined(MINGW) || \
		defined(MINGW32) || \
		defined(__MINGW32) || \
		defined(__MINGW32__) || \
		defined(MINGW64) || \
		defined(__MINGW64) || \
		defined(__MINGW64__) || \
		(defined(__GNUC__) && \
		 defined(WIN32))

#	define ENV_MINGW

#	include <_mingw.h>

#endif // MinGW


// define memory private constants

#define FREE_ALLOW_CACHE_BYPASS						1

#define PRUNE_MEMORY_CACHE							0

#define MEMLIST_DEFAULT_LENGTH						8

#define MEMLIST_LIST_INCREMENT						8

#define MAX_MEMORY_LENGTH							1048576

#define MEMORY_BLOCK_LENGTH							8

#define MAX_CACHE_MEMORY							67108864

#define TARGET_MEMORY_FOOTPRINT						1073741824

#define MEMCACHE_PREFIX								53164597

#define MEMCACHE_POSTFIX							86451325

#define DISPLAY_SLOT_PROFILE						0


// define memory private data types

typedef struct _Memlist {
	int ref;
	int length;
	void **list;
	Mutex mutex;
} Memlist;

typedef struct _MemcacheEntry {
	int length;
	int inUse;
	int peak;
	alint ticks;
	Memlist bucket;
} MemcacheEntry;

typedef struct _memcache {
	int cacheLength;
	int maxMemoryLength;
	int memoryBlockLength;
	alint maxCacheMemory;
	alint targetMemoryFootprint;
	alint allocatedMemory;
	alint cachedMemory;
	alint totalCachedItems;
	alint totalSlotTicks;
	MemcacheEntry *cache;
	Mutex mutex;
} MEMCACHE_STATE;


// define memory private global variables

MEMCACHE_STATE *memcache = NULL;


// declare memory private functions

// memory functions

static void *local_malloc(int length, const char *file, const char *function,
		size_t line);

static void *local_realloc(void *value, int length, const char *file,
		const char *function, size_t line);

static void local_free(void *value, const char *file, const char *function,
		size_t line);

// memory list functions

void memlist_init(Memlist *list);

void memlist_free(Memlist *list);

int memlist_length(Memlist *list);

int memlist_push(Memlist *list, void *entry);

void *memlist_pop(Memlist *list);

// memory caching helper functions

static int memcache_calculateSlotId(int length);

static void memcache_pruneCache();

static void memcache_freeCache(void);


// declare public memory functions (for compiler heirarchy)

void memory_displaySlotProfile();



// define memory private functions

// memory functions

static void *local_malloc(int length, const char *file, const char *function,
		size_t line)
{
	void *result = NULL;

	if(length < 1) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s():%i] warning - unable to allocate %i bytes "
					"from [%s->%s():%i].\n", __FUNCTION__, __LINE__, length,
					(char *)file, (char *)function, (int)line);
		}
		length = 1;
	}

	result = malloc(length);

	if(result == NULL) {
		fprintf(stderr, "[%s():%i] PANIC - failed to allocate %i bytes of "
				"memory, aborting.\n", __FUNCTION__, __LINE__, length);
		fflush(NULL);
		exit(1);
	}

	memset(result, 0, length);

	return result;
}

static void *local_realloc(void *value, int length, const char *file,
		const char *function, size_t line)
{
	void *result = NULL;

	if(value == NULL) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s():%i] warning - unable to reallocate NULL "
					"memory from [%s->%s():%i].\n", __FUNCTION__, __LINE__,
					(char *)file, (char *)function, (int)line);
		}
	}
	if(length < 1) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s():%i] warning - unable to reallocate %i bytes "
					"from [%s->%s():%i].\n", __FUNCTION__, __LINE__, length,
					(char *)file, (char *)function, (int)line);
		}
		length = 1;
	}

	if(value == NULL) {
		result = malloc(length);
	}
	else {
		result = realloc(value, length);
	}

	if(result == NULL) {
		fprintf(stderr, "[%s():%i] PANIC - failed to reallocate %i bytes of "
				"memory, aborting.\n", __FUNCTION__, __LINE__, length);
		fflush(NULL);
		exit(1);
	}

	if(value == NULL) {
		memset(result, 0, length);
	}

	return result;
}

static void local_free(void *value, const char *file, const char *function,
		size_t line)
{
	if(value == NULL) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s():%i] error - unable to free NULL memory "
					"from [%s->%s():%i].\n", __FUNCTION__, __LINE__,
					(char *)file, (char *)function, (int)line);
		}
		return;
	}

	free(value);
}

// memory list functions

void memlist_init(Memlist *list)
{
	if(list == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	memset(list, 0, (sizeof(Memlist)));

	list->ref = 0;
	list->length = MEMLIST_DEFAULT_LENGTH;
	list->list = (void *)local_malloc((sizeof(void *) * list->length),
				__FILE__, __FUNCTION__, __LINE__);

	mutex_init(&list->mutex);
}

void memlist_free(Memlist *list)
{
	int ii = 0;

	if(list == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	mutex_lock(&list->mutex);

	if(list->list != NULL) {
		for(ii = 0; ii < list->ref; ii++) {
			if(list->list[ii] != NULL) {
				local_free(list->list[ii], __FILE__, __FUNCTION__, __LINE__);
			}
		}
		local_free(list->list, __FILE__, __FUNCTION__, __LINE__);
	}

	mutex_unlock(&list->mutex);
	mutex_free(&list->mutex);

	memset(list, 0, (sizeof(Memlist)));
}

int memlist_length(Memlist *list)
{
	int result = 0;

	if(list == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&list->mutex);

	result = list->ref;

	mutex_unlock(&list->mutex);

	return result;
}

int memlist_push(Memlist *list, void *entry)
{
	int ii = 0;
	int ref = 0;

	if((list == NULL) || (entry == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&list->mutex);

	ref = list->ref;

	list->ref += 1;
	if(list->ref >= list->length) {
		while(list->ref >= list->length) {
			list->length += MEMLIST_LIST_INCREMENT;
		}
		list->list = (void *)local_realloc(list->list,
				(sizeof(void *) * list->length),
				__FILE__, __FUNCTION__, __LINE__);
		for(ii = list->ref; ii < list->length; ii++) {
			list->list[ii] = NULL;
		}
	}

	list->list[ref] = entry;

	mutex_unlock(&list->mutex);

	return 0;
}

void *memlist_pop(Memlist *list)
{
	void *result = NULL;

	if(list == NULL) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	mutex_lock(&list->mutex);

	if(list->ref > 0) {
		result = list->list[(list->ref - 1)];
		list->list[(list->ref - 1)] = NULL;
		list->ref -= 1;
	}

	mutex_unlock(&list->mutex);

	return result;
}

// memory caching helper functions

static int memcache_calculateSlotId(int length)
{
	int slot_id = 0;

	if(length <= 128) {
		slot_id = (length / 9);
	}
	else if(length <= 2048) {
		slot_id = (length / 32) + 10;
	}
	else if(length <= memcache->maxMemoryLength) {
		slot_id = (length / 5000) + 74;
	}
	else {
		slot_id = -1;
	}

	return slot_id;
}

static void memcache_pruneCache()
{
	int ii = 0;
	int counter = 0;
	int bucket_length = 0;
	alint cache_target = 0;
	double bucket_percent = 0.0;
	double tick_percent = 0.0;
	double totalCachedItems = 0.0;
	void *entry = NULL;

	if((memcache->cachedMemory < memcache->maxCacheMemory) &&
			(memcache->allocatedMemory < memcache->targetMemoryFootprint)) {
		return;
	}

	cache_target = (alint)((double)memcache->cachedMemory * 0.9);

	while((memcache->cachedMemory > cache_target) && (counter < 3)) {
		totalCachedItems = (double)memcache->totalCachedItems;

		for(ii = 0; ii < memcache->cacheLength; ii++) {
			if(memlist_length(&memcache->cache[ii].bucket) < 2) {
				continue;
			}

			bucket_length = (memcache->cache[ii].length + (SIZEOF_INT * 3));

			bucket_percent = (
					(double)memlist_length(
						&memcache->cache[ii].bucket) /
					totalCachedItems);
			tick_percent = (
					(double)memcache->cache[ii].ticks /
					(double)memcache->totalSlotTicks);
			if(bucket_percent > tick_percent) {
				entry = memlist_pop(&memcache->cache[ii].bucket);
				if(entry == NULL) {
					break;
				}

				local_free(entry, __FILE__, __FUNCTION__, __LINE__);

				mutex_lock(&memcache->mutex);
				memcache->totalCachedItems -= 1;
				memcache->totalSlotTicks += 1;
				memcache->cache[ii].ticks += 1;
				memcache->cachedMemory -= bucket_length;
				memcache->allocatedMemory -= bucket_length;
				mutex_unlock(&memcache->mutex);

				bucket_percent = (
						(double)memlist_length(
							&memcache->cache[ii].bucket) /
						totalCachedItems);
				tick_percent = (
						(double)memcache->cache[ii].ticks /
						(double)memcache->totalSlotTicks);
			}
		}

		counter++;
	}
}

static void memcache_freeCache(void)
{
	int ii = 0;

	if(memcache == NULL) {
		return;
	}

	mutex_lock(&memcache->mutex);

	if(memcache->cache != NULL) {
		for(ii = 0; ii < memcache->cacheLength; ii++) {
			memlist_free(&memcache->cache[ii].bucket);
		}

		local_free(memcache->cache, __FILE__, __FUNCTION__, __LINE__);
	}

	mutex_unlock(&memcache->mutex);
	mutex_free(&memcache->mutex);

	memset(memcache, 0, sizeof(MEMCACHE_STATE));

	local_free(memcache, __FILE__, __FUNCTION__, __LINE__);
}


// define memory public functions

void memory_init()
{
	int ii = 0;
	int nn = 0;
	int length = 0;
	void *memory = NULL;

#if defined(MEMCACHE_BYPASS)
	return;
#endif // MEMCACHE_BYPASS

	if(memcache != NULL) {
		return;
	}

	memcache = (MEMCACHE_STATE *)local_malloc(sizeof(MEMCACHE_STATE), __FILE__,
			__FUNCTION__, __LINE__);

	memcache->cacheLength = 0;
	memcache->maxMemoryLength = MAX_MEMORY_LENGTH;
	memcache->memoryBlockLength = MEMORY_BLOCK_LENGTH;
	memcache->maxCacheMemory = MAX_CACHE_MEMORY;
	memcache->targetMemoryFootprint = TARGET_MEMORY_FOOTPRINT;
	memcache->allocatedMemory = 0;
	memcache->cachedMemory = 0;
	memcache->totalCachedItems = 0;
	memcache->totalSlotTicks = 0;

	memcache->cacheLength = (memcache_calculateSlotId(MAX_MEMORY_LENGTH) + 1);

	memcache->cache = (MemcacheEntry *)local_malloc(
			(sizeof(MemcacheEntry) * memcache->cacheLength),
			__FILE__, __FUNCTION__, __LINE__);

	mutex_init(&memcache->mutex);

	ii = 0;
	length = 9;
	while(length <= 128) {
		memcache->cache[ii].length = (length - 1);
		ii++;
		length += 9;
	}

	length = (((length + 32) / 32) * 32);
	while(length <= 2048) {
		memcache->cache[ii].length = (length - 1);
		ii++;
		length += 32;
	}

	length = (((length + 5000) / 5000) * 5000);
	while(length <= (MAX_MEMORY_LENGTH + 5000)) {
		memcache->cache[ii].length = (length - 1);
		ii++;
		length += 5000;
	}

	memcache->cache[(memcache->cacheLength - 1)].length = MAX_MEMORY_LENGTH;

	for(ii = 0; ii < memcache->cacheLength; ii++) {
		memcache->cache[ii].ticks = 0;
		memcache->cache[ii].inUse = 0;
		memcache->cache[ii].peak = 0;

		memlist_init(&memcache->cache[ii].bucket);

		if(memcache->cache[ii].length > 8192) {
			continue;
		}

		for(nn = 0; nn < ((memcache->cacheLength - ii) / 32); nn++) {
			length = memcache->cache[ii].length;

			memory = local_malloc(
					(sizeof(char) * ((length + (SIZEOF_INT * 3)) + 1)),
					__FILE__, __FUNCTION__, __LINE__);

			memlist_push(&memcache->cache[ii].bucket, memory);
			memcache->allocatedMemory += (length + (SIZEOF_INT * 3));
			memcache->cachedMemory += (length + (SIZEOF_INT * 3));
			memcache->totalCachedItems += 1;
			memcache->totalSlotTicks += 1;
			memcache->cache[ii].ticks += 1;
			memcache->cache[ii].inUse += 1;
			memcache->cache[ii].peak = memcache->cache[ii].inUse;
		}
	}

	atexit(memcache_freeCache);
}

void *memory_allocate(int length, const char *file, const char *function,
		size_t line)
{
	aboolean obtainedFromCache = afalse;
	int ii = 0;
	int slot = 0;
	int localLength = 0;
	int allocationLength = 0;
	void *ptr = NULL;
	void *result = NULL;

	if(length < 1) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s():%i] warning - unable to allocate %i bytes "
					"from [%s->%s():%i].\n", __FUNCTION__, __LINE__, length,
					(char *)file, (char *)function, (int)line);
		}
		length = 1;
	}

#if defined(MEMCACHE_BYPASS)
	return (void *)local_malloc(
			(sizeof(char) * (length + 1)),
			file, function, line);
#endif // MEMCACHE_BYPASS

	if(memcache == NULL) {
		memory_init();
	}

	if(length <= memcache->maxMemoryLength) {
		slot = memcache_calculateSlotId(length);
		if((slot < 0) || (slot >= memcache->cacheLength)) {
			fprintf(stderr, "[%s():%i] error - calculated invalid slot %i "
					"for %i bytes of memory from {%s}[%s():%i].\n",
					__FUNCTION__, __LINE__, slot, length, (char *)file,
					(char *)function, (int)line);
			return NULL;
		}
	}
	else {
		slot = -1;
		localLength = length;
	}

	if(slot != -1) {
		// pick either the slot for the requested length, or the next one
		// larger

		for(ii = 0;
				((ii < 2) &&
				 ((slot + ii) < memcache->cacheLength) &&
				 (result == NULL));
				ii++) {
			result = memlist_pop(&memcache->cache[(slot + ii)].bucket);
			if(result != NULL) {
				break;
			}
		}

		if(result == NULL) {
			ii = 0;
		}
		localLength = memcache->cache[(slot + ii)].length;
	}

	allocationLength = (localLength + (SIZEOF_INT * 3));

	if(result == NULL) {
		result = (void *)local_malloc(
				(sizeof(char) * (allocationLength + 1)),
				file, function, line);
		if(result == NULL) {
			fprintf(stderr, "[%s():%i] PANIC - failed to allocate memory "
					"length %i from {%s}[%s():%i].\n", __FUNCTION__, __LINE__,
					length, (char *)file, (char *)function, (int)line);
			fflush(stderr);
			exit(1);
		}
	}
	else {
		obtainedFromCache = atrue;
	}

	memset(result, 0, (sizeof(char) * (allocationLength + 1)));

	ptr = result;

	*((int *)ptr) = localLength;
#if defined(WIN32) && !defined(ENV_MINGW)
	((char *)ptr) += SIZEOF_INT;
#else // !WIN32 || ENV_MINGW
	ptr += SIZEOF_INT;
#endif // WIN32 && !ENV_MINGW

	*((int *)ptr) = MEMCACHE_PREFIX;
#if defined(WIN32) && !defined(ENV_MINGW)
	((char *)ptr) += (SIZEOF_INT + localLength);
#else // !WIN32 || ENV_MINGW
	ptr += (SIZEOF_INT + localLength);
#endif // WIN32 && !ENV_MINGW

	*((int *)ptr) = MEMCACHE_POSTFIX;

#if defined(WIN32) && !defined(ENV_MINGW)
	ptr = (((char *)result) + (SIZEOF_INT * 2));
#else // !WIN32 || ENV_MINGW
	ptr = (result + (SIZEOF_INT * 2));
#endif // WIN32 && !ENV_MINGW

	mutex_lock(&memcache->mutex);

	if(obtainedFromCache) {
		memcache->totalCachedItems -= 1;
		memcache->totalSlotTicks += 1;
		memcache->cache[(slot + ii)].ticks += 1;
		memcache->cachedMemory -= allocationLength;
	}
	else {
		memcache->allocatedMemory += allocationLength;
	}

	if(slot != -1) {
		memcache->cache[(slot + ii)].inUse += 1;
		if(memcache->cache[(slot + ii)].inUse >
				memcache->cache[(slot + ii)].peak) {
			memcache->cache[(slot + ii)].peak =
				memcache->cache[(slot + ii)].inUse;
		}
	}

	mutex_unlock(&memcache->mutex);

	return ptr;
}

void *memory_reallocate(void *value, int length, const char *file,
		const char *function, size_t line)
{
	int slot = 0;
	int oldSlotId = 0;
	int newSlotId = 0;
	int localLength = 0;
	int currentLength = 0;
	int allocationLength = 0;
	void *ptr = NULL;
	void *result = NULL;

	if(value == NULL) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s():%i] warning - unable to reallocate NULL "
					"memory from [%s->%s():%i].\n", __FUNCTION__, __LINE__,
					(char *)file, (char *)function, (int)line);
		}
	}

	if(length < 1) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s():%i] warning - unable to reallocate %i bytes "
					"from [%s->%s():%i].\n", __FUNCTION__, __LINE__, length,
					(char *)file, (char *)function, (int)line);
		}
		length = 1;
	}

	if(value == NULL) {
		return local_malloc(length, file, function, line);
	}

#if defined(MEMCACHE_BYPASS)
	return (void *)local_realloc(value, (sizeof(char) * (length + 1)),
			file, function, line);
#endif // MEMCACHE_BYPASS

	if(memcache == NULL) {
		memory_init();
	}

#if defined(WIN32) && !defined(ENV_MINGW)
	ptr = (((char *)value) - (SIZEOF_INT * 2));
#else // !WIN32 || ENV_MINGW
	ptr = (value - (SIZEOF_INT * 2));
#endif // WIN32 && !ENV_MINGW

	currentLength = *((int *)ptr);
	if(currentLength < 1) {
		fprintf(stderr, "[%s():%i] error - invalid memory 0x%llx with "
				"length %i from {%s}[%s():%i].\n", __FUNCTION__, __LINE__,
				(alint)(aptrcast)value, currentLength, (char *)file,
				(char *)function, (int)line);
		return NULL;
	}

#if defined(WIN32) && !defined(ENV_MINGW)
	((char *)ptr) += SIZEOF_INT;
#else // !WIN32 || ENV_MINGW
	ptr += SIZEOF_INT;
#endif // WIN32 && !ENV_MINGW

	if(*((int *)ptr) != MEMCACHE_PREFIX) {
		fprintf(stderr, "[%s():%i] error - invalid or unrecognized memory "
				"0x%llx with prefix mismatch %i vs %i from {%s}[%s():%i].\n",
				__FUNCTION__, __LINE__, (alint)(aptrcast)value,
				MEMCACHE_PREFIX, *((int *)ptr), (char *)file, (char *)function,
				(int)line);
		return NULL;
	}

#if defined(WIN32) && !defined(ENV_MINGW)
	((char *)ptr) += (SIZEOF_INT + currentLength);
#else // !WIN32 || ENV_MINGW
	ptr += (SIZEOF_INT + currentLength);
#endif // WIN32 && !ENV_MINGW

	if(*((int *)ptr) != MEMCACHE_POSTFIX) {
		fprintf(stderr, "[%s():%i] error - invalid or unrecognized memory "
				"0x%llx with postfix mismatch %i vs %i from {%s}[%s():%i].\n",
				__FUNCTION__, __LINE__, (alint)(aptrcast)value,
				MEMCACHE_POSTFIX, *((int *)ptr), (char *)file,
				(char *)function, (int)line);
		return NULL;
	}

	if(length <= memcache->maxMemoryLength) {
		slot = memcache_calculateSlotId(length);
		if((slot < 0) || (slot >= memcache->cacheLength)) {
			fprintf(stderr, "[%s():%i] error - calculated invalid slot %i "
					"for %i bytes of memory from {%s}[%s():%i].\n",
					__FUNCTION__, __LINE__, slot, length, (char *)file,
					(char *)function, (int)line);
			return NULL;
		}
		localLength = memcache->cache[slot].length;
	}
	else {
		slot = -1;
		localLength = length;
	}

#if defined(WIN32) && !defined(ENV_MINGW)
	ptr = (((char *)value) - (SIZEOF_INT * 2));
#else // !WIN32 || ENV_MINGW
	ptr = (value - (SIZEOF_INT * 2));
#endif // WIN32 && !ENV_MINGW

	allocationLength = (localLength + (SIZEOF_INT * 3));

	ptr = (void *)local_realloc(ptr,
			(sizeof(char) * (allocationLength + 1)),
			file, function, line);
	if(ptr == NULL) {
		fprintf(stderr, "[%s():%i] PANIC - failed to reallocate memory "
				"length %i from {%s}[%s():%i].\n", __FUNCTION__, __LINE__,
				length, (char *)file, (char *)function, (int)line);
		fflush(stderr);
		exit(1);
	}

	result = ptr;

	*((int *)ptr) = localLength;
#if defined(WIN32) && !defined(ENV_MINGW)
	((char *)ptr) += SIZEOF_INT;
#else // !WIN32 || ENV_MINGW
	ptr += SIZEOF_INT;
#endif // WIN32 && !ENV_MINGW

	*((int *)ptr) = MEMCACHE_PREFIX;
#if defined(WIN32) && !defined(ENV_MINGW)
	((char *)ptr) += (SIZEOF_INT + localLength);
#else // !WIN32 || ENV_MINGW
	ptr += (SIZEOF_INT + localLength);
#endif // WIN32 && !ENV_MINGW

	*((int *)ptr) = MEMCACHE_POSTFIX;

#if defined(WIN32) && !defined(ENV_MINGW)
	ptr = (((char *)result) + (SIZEOF_INT * 2));
#else // !WIN32 || ENV_MINGW
	ptr = (result + (SIZEOF_INT * 2));
#endif // WIN32 && !ENV_MINGW

	oldSlotId = memcache_calculateSlotId(currentLength);
	newSlotId = memcache_calculateSlotId(localLength);

	mutex_lock(&memcache->mutex);

	memcache->allocatedMemory -= currentLength;
	memcache->allocatedMemory += allocationLength;

	if(oldSlotId != -1) {
		memcache->cache[oldSlotId].inUse -= 1;
	}
	if(newSlotId != -1) {
		memcache->cache[newSlotId].inUse += 1;
		if(memcache->cache[newSlotId].inUse > memcache->cache[newSlotId].peak) {
			memcache->cache[newSlotId].peak = memcache->cache[newSlotId].inUse;
		}
	}

	mutex_unlock(&memcache->mutex);

	return ptr;
}

void memory_free(void *value, const char *file, const char *function,
		size_t line)
{
	int slot = 0;
	int length = 0;
	int allocationLength = 0;
	void *ptr = NULL;

	if(value == NULL) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s():%i] error - unable to free NULL memory "
					"from [%s->%s():%i].\n", __FUNCTION__, __LINE__,
					(char *)file, (char *)function, (int)line);
		}
		return;
	}

#if defined(MEMCACHE_BYPASS)
	local_free(value, file, function, line);
	return;
#endif // MEMCACHE_BYPASS

	if(memcache == NULL) {
		memory_init();
	}

#if defined(WIN32) && !defined(ENV_MINGW)
	ptr = (((char *)value) - (SIZEOF_INT * 2));
#else // !WIN32 || ENV_MINGW
	ptr = (value - (SIZEOF_INT * 2));
#endif // WIN32 && !ENV_MINGW

	length = *((int *)ptr);
	if(length < 1) {
		fprintf(stderr, "[%s():%i] error - invalid memory 0x%llx with "
				"length %i from {%s}[%s():%i].\n", __FUNCTION__, __LINE__,
				(alint)(aptrcast)value, length, (char *)file,
				(char *)function, (int)line);
		return;
	}

#if defined(WIN32) && !defined(ENV_MINGW)
	((char *)ptr) += SIZEOF_INT;
#else // !WIN32 || ENV_MINGW
	ptr += SIZEOF_INT;
#endif // WIN32 && !ENV_MINGW

	if(*((int *)ptr) != MEMCACHE_PREFIX) {
		if(FREE_ALLOW_CACHE_BYPASS) {
			local_free(value, file, function, line);
		}
		else {
			fprintf(stderr, "[%s():%i] error - invalid or unrecognized memory "
					"0x%llx with prefix mismatch %i vs %i from {%s}[%s():%i].\n",
					__FUNCTION__, __LINE__, (alint)(aptrcast)value,
					MEMCACHE_PREFIX, *((int *)ptr), (char *)file,
					(char *)function, (int)line);
		}
		return;
	}

#if defined(WIN32) && !defined(ENV_MINGW)
	((char *)ptr) += (SIZEOF_INT + length);
#else // !WIN32 || ENV_MINGW
	ptr += (SIZEOF_INT + length);
#endif // WIN32 && !ENV_MINGW

	if(*((int *)ptr) != MEMCACHE_POSTFIX) {
		fprintf(stderr, "[%s():%i] error - invalid or unrecognized memory "
				"0x%llx with postfix mismatch %i vs %i from {%s}[%s():%i].\n",
				__FUNCTION__, __LINE__, (alint)(aptrcast)value,
				MEMCACHE_POSTFIX, *((int *)ptr), (char *)file,
				(char *)function, (int)line);
		return;
	}

	if(length <= memcache->maxMemoryLength) {
		slot = memcache_calculateSlotId(length);
		if((slot < 0) || (slot >= memcache->cacheLength)) {
			fprintf(stderr, "[%s():%i] error - calculated invalid slot %i "
					"for %i bytes of memory from {%s}[%s():%i].\n",
					__FUNCTION__, __LINE__, slot, length, (char *)file,
					(char *)function, (int)line);
			return;
		}
	}
	else {
		slot = -1;
	}

#if defined(WIN32) && !defined(ENV_MINGW)
	ptr = (((char *)value) - (SIZEOF_INT * 2));
#else // !WIN32 || ENV_MINGW
	ptr = (value - (SIZEOF_INT * 2));
#endif // WIN32 && !ENV_MINGW
	allocationLength = (length + (SIZEOF_INT * 3));

	if((PRUNE_MEMORY_CACHE) && (slot != -1)) {
		memlist_push(&memcache->cache[slot].bucket, ptr);

		mutex_lock(&memcache->mutex);
		memcache->totalCachedItems += 1;
		memcache->totalSlotTicks += 1;
		memcache->cache[slot].ticks += 1;
		memcache->cache[slot].inUse -= 1;
		memcache->cachedMemory += allocationLength;
		mutex_unlock(&memcache->mutex);

		memcache_pruneCache();
	}
	else if((slot != -1) &&
			(memcache->cachedMemory < memcache->maxCacheMemory) &&
			(memcache->allocatedMemory < memcache->targetMemoryFootprint)) {
		memlist_push(&memcache->cache[slot].bucket, ptr);

		mutex_lock(&memcache->mutex);
		memcache->totalCachedItems += 1;
		memcache->totalSlotTicks += 1;
		memcache->cache[slot].ticks += 1;
		memcache->cache[slot].inUse -= 1;
		memcache->cachedMemory += allocationLength;
		mutex_unlock(&memcache->mutex);
	}
	else {
		local_free(ptr, __FILE__, __FUNCTION__, __LINE__);

		mutex_lock(&memcache->mutex);
		memcache->allocatedMemory -= allocationLength;
		mutex_unlock(&memcache->mutex);
	}
}

void memory_systemFree(void *value, const char *file, const char *function,
		size_t line)
{
	local_free(value, file, function, line);
}

char *memory_strdup(char *string, const char *file, const char *function,
		size_t line)
{
	int length = 0;
	char *result = NULL;

	if(string == NULL) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s():%i] error - unable to duplicate NULL "
					"string from [%s->%s():%i].\n", __FUNCTION__, __LINE__,
					(char *)file, (char *)function, (int)line);
		}
		string = "";
	}

	length = (int)strlen(string);
	result = (char *)memory_allocate((sizeof(char) * (length + 1)), file,
			function, line);

	if(length > 0) {
		memcpy(result, string, length);
	}

	return result;
}

char *memory_strndup(char *string, int length, const char *file,
		const char *function, size_t line)
{
	char *result = NULL;

	if(string == NULL) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s():%i] error - unable to duplicate NULL "
					"string from [%s->%s():%i].\n", __FUNCTION__, __LINE__,
					(char *)file, (char *)function, (int)line);
		}
		string = "";
	}
	if(length < 1) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s():%i] error - unable to duplicate string "
					"with length %i from [%s->%s():%i].\n", __FUNCTION__,
					__LINE__, length, (char *)file, (char *)function,
					(int)line);
		}
		length = (int)strlen(string);
	}

	result = (char *)memory_allocate((sizeof(char) * (length + 1)), file,
			function, line);
	memcpy(result, string, length);

	return result;
}

char *memory_strxpndcat(char *string, int *stringRef, int *stringLength,
		char *source, int sourceLength, const char *file, const char *function,
		size_t line)
{
	char *ptr = NULL;
	char *result = NULL;

	if((string == NULL) || (stringRef == NULL) || (stringLength == NULL) ||
			(source == NULL) || (sourceLength < 1)) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s->%s():%i] error - invalid or missing "
					"arguments to %s()", (char *)file, (char *)function,
					(int)line, __FUNCTION__);
			if(string == NULL) {
				fprintf(stderr, ", string is NULL");
			}
			if(stringRef == NULL) {
				fprintf(stderr, ", stringRef is NULL");
			}
			if(stringLength == NULL) {
				fprintf(stderr, ", stringLength is NULL");
			}
			if(source == NULL) {
				fprintf(stderr, ", source is NULL");
			}
			if(sourceLength < 1) {
				fprintf(stderr, ", sourceLength is %i", sourceLength);
			}
			fprintf(stderr, "\n");
		}
		return string;
	}

	result = string;

	if((sourceLength + (*stringRef) + 1) >= (*stringLength)) {
		*stringLength += sourceLength;
		*stringLength *= 2;
		result = (char *)memory_reallocate(result,
				(sizeof(char) * (*stringLength)),
				file,
				function,
				line);
	}

	ptr = (result + (*stringRef));

	memcpy(ptr, source, sourceLength);

	*stringRef += sourceLength;

	return result;
}

char *memory_itoa(int value, const char *file, const char *function,
		size_t line)
{
	char *result = NULL;

	result = (char *)memory_allocate((sizeof(char) * 16), file, function, line);

	snprintf(result, (int)((sizeof(char) * 16) - 1), "%i", value);

	return result;
}

char *memory_bitoa(alint value, const char *file, const char *function,
		size_t line)
{
	char *result = NULL;

	result = (char *)memory_allocate((sizeof(char) * 32), file, function, line);

	snprintf(result, (int)((sizeof(char) * 32) - 1), "%lli", value);

	return result;
}

char *memory_dtoa(double value, const char *file, const char *function,
		size_t line)
{
	char *result = NULL;

	result = (char *)memory_allocate((sizeof(char) * 64), file, function, line);

	snprintf(result, (int)((sizeof(char) * 64) - 1), "%f", value);

	return result;
}

void memory_displayStatus()
{
#if defined(MEMCACHE_BYPASS)
	return;
#endif // MEMCACHE_BYPASS

	if(memcache == NULL) {
		memory_init();
	}

	printf("memcache status:\n");

	printf("    cache length             : %i\n", memcache->cacheLength);
	printf("    max memory length        : %i\n", memcache->maxMemoryLength);
	printf("    memory block length      : %i\n",
			memcache->memoryBlockLength);
	printf("    max cache memory         : %lli\n", memcache->maxCacheMemory);
	printf("    target memory footprint  : %lli\n",
			memcache->targetMemoryFootprint);
	printf("    allocated memory         : %lli\n", memcache->allocatedMemory);
	printf("    cached memory            : %lli\n", memcache->cachedMemory);
	printf("    in-use memory            : %lli\n",
			(memcache->allocatedMemory - memcache->cachedMemory));
	printf("    total cached items       : %lli\n",
			memcache->totalCachedItems);
	printf("    total slot ticks         : %lli\n", memcache->totalSlotTicks);

	if(DISPLAY_SLOT_PROFILE) {
		memory_displaySlotProfile();
	}
}

void memory_displaySlotProfile()
{
	int ii = 0;

#if defined(MEMCACHE_BYPASS)
	return;
#endif // MEMCACHE_BYPASS

	if(memcache == NULL) {
		memory_init();
	}

	printf("memcache slot profile:\n");

	printf("    slot usage:\n");
	printf("        slot    length   current                    ticks"
		   "                     peak\n");
	for(ii = 0; ii < memcache->cacheLength; ii++) {
		printf("        %4i  %8i  %8i (%6.2f %%)  %12lli (%6.2f %%)  %12i\n",
				ii,
				memcache->cache[ii].length,
				memlist_length(&memcache->cache[ii].bucket),
				(((double)memlist_length(&memcache->cache[ii].bucket) /
				  (double)memcache->totalCachedItems) * 100.0),
				memcache->cache[ii].ticks,
				(((double)memcache->cache[ii].ticks /
				  (double)memcache->totalSlotTicks) * 100.0),
			   memcache->cache[ii].peak);
	}
}

void *memory_getState()
{
#if defined(MEMCACHE_BYPASS)
	return NULL;
#endif // MEMCACHE_BYPASS

	if(memcache == NULL) {
		memory_init();
	}

	return (void *)memcache;
}

int memory_setState(void *state)
{
#if defined(MEMCACHE_BYPASS)
	return 0;
#endif // MEMCACHE_BYPASS

	if(state == NULL) {
		fprintf(stderr, "[%s():%i] error - invalid or missing argument(s).\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if((memcache != NULL) && (state == memcache)) {
		return 0;
	}

	if(memcache != NULL) {
		memcache_freeCache();
	}

	memcache = (MEMCACHE_STATE *)state;

	return 0;
}

