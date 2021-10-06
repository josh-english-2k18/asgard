/*
 * lfbc.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Low-Fragmentation Block-Cache (LFBC) is a block-based file cache
 * including a background-thread management system.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#define _CORE_LIBRARY_TYPES_COMPONENT
#include "core/library/types/lfbc.h"


// define low-fragmentation block-cache private constants

#define FUDGE_FACTOR_LENGTH							32


// declare low-fragmentation block-cache private functions

// general functions

static void freeNothing(void *memory);

static alint calculateBlockPosition(Lfbc *cache, alint position);

static int openFile(Lfbc *cache, LfbcEntry *entry);

static int handleFileError(Lfbc *cache, FileHandle *fh, char *message, int rc,
		const char *function, size_t line);

// cache entry functions

static int initCacheEntry(Lfbc *cache, LfbcEntry *entry);

static LfbcEntry *newCacheEntry(Lfbc *cache, char *filename);

static void freeCacheEntry(void *memory);

static char *getCacheEntryBLock(Lfbc *cache, LfbcEntry *entry,
		alint position, int *blockLength, alint *blockId);

// cache functions

static char *allocateCacheBlock(Lfbc *cache);

static void freeCacheBlock(Lfbc *cache, char *block);

// cache management functions

static void *manageCacheThread(void *threadContext, void *argument);


// define low-fragmentation block-cache private macros

#define fileError(cache, fh, rc, message) \
	handleFileError(cache, fh, rc, message, __FUNCTION__, __LINE__)


// define low-fragmentation block-cache private functions

// general functions

static void freeNothing(void *memory)
{
	// do nothing
}

static alint calculateBlockPosition(Lfbc *cache, alint position)
{
	alint result = 0;

	result = (position / (alint)cache->blockLength);

	return result;
}

static int openFile(Lfbc *cache, LfbcEntry *entry)
{
	int rc = 0;
	int ii = 0;
	int length = 0;

	LfbcEntry *listEntry = NULL;
	LfbcEntry *oldestAccessEntry = NULL;

	mutex_lock(&cache->mutex);

	if(cache->currentOpenFileHandles >= cache->maxOpenFileHandles) {
		length = arraylist_length(&cache->entries);

		for(ii = 0; ii < length; ii++) {
			listEntry = (LfbcEntry *)arraylist_get(&cache->entries, ii);

			if((listEntry == NULL) ||
					(!listEntry->isFileHandleOpen) ||
					(listEntry->blockCount == 0) ||
					(listEntry->mostRecentTimestamp == 0.0)) {
				continue;
			}

			if((oldestAccessEntry == NULL) ||
					(oldestAccessEntry->mostRecentTimestamp >
					 listEntry->mostRecentTimestamp)) {
				oldestAccessEntry = listEntry;
			}
		}

		if(oldestAccessEntry != NULL) {
			mutex_lock(&oldestAccessEntry->mutex);

			if(oldestAccessEntry->isFileHandleOpen) {
				log_logf(cache->log, LOG_LEVEL_INFO,
						"released file handle for oldest-access entry '%s'",
						oldestAccessEntry->filename);

				file_free(&oldestAccessEntry->fh);
				oldestAccessEntry->isFileHandleOpen = afalse;

				cache->currentOpenFileHandles -= 1;
			}

			mutex_unlock(&oldestAccessEntry->mutex);
		}
	}

	mutex_unlock(&cache->mutex);		

	if((rc = file_init(&entry->fh, entry->filename, "rb",
					LFBC_STATE_DEFAULT_BLOCK_LENGTH)) == 0) {
		log_logf(cache->log, LOG_LEVEL_INFO,
				"opened file-handle for entry '%s'", entry->filename);

		entry->isFileHandleOpen = atrue;

		mutex_lock(&cache->mutex);
		cache->currentOpenFileHandles += 1;
		mutex_unlock(&cache->mutex);
	}

	return rc;
}

static int handleFileError(Lfbc *cache, FileHandle *fh, char *message, int rc,
		const char *function, size_t line)
{
	char *errorMessage = NULL;

	errorMessage = file_getErrorMessage(fh, rc);

	log_logf(cache->log, LOG_LEVEL_ERROR,
			"[%s():%i] error - failed to %s file '%s' with '%s'",
			(char *)function, (int)line, message, fh->filename, errorMessage);

	free(errorMessage);

	return rc;
}


// cache entry functions

static int initCacheEntry(Lfbc *cache, LfbcEntry *entry)
{
	int ii = 0;
	int rc = 0;
	alint fileLength = 0;

	if((rc = openFile(cache, entry)) < 0) {
		return fileError(cache, &entry->fh, "open", rc);
	}

	if(((rc = file_getFileLength(&entry->fh, &entry->fileLength)) < 0) ||
			(entry->fileLength < 0)) {
		return fileError(cache, &entry->fh, "get length on", rc);
	}

	entry->blockCount = (entry->fileLength / (alint)cache->blockLength);
	if((entry->fileLength % (alint)cache->blockLength) != 0) {
		entry->blockCount += 1;
	}

	entry->blocks = (LfbcBlock *)malloc(sizeof(LfbcBlock) * (int)entry->blockCount);

	fileLength = entry->fileLength;

	for(ii = 0; ii < entry->blockCount; ii++) {
		entry->blocks[ii].lock = 0;

		entry->blocks[ii].length = cache->blockLength;
		if((fileLength - (alint)entry->blocks[ii].length) < 0) {
			entry->blocks[ii].length = (int)fileLength;
		}

		entry->blocks[ii].timestamp = 0.0;

		entry->blocks[ii].block = NULL;

		fileLength -= entry->blocks[ii].length;
	}

	system_fileModifiedTime(entry->filename, &entry->lastModifiedTime);

	mutex_lock(&cache->mutex);

	if(bptree_put(&cache->index, entry->filename, strlen(entry->filename),
				(void *)entry) < 0) {
		mutex_unlock(&cache->mutex);
		return -1;
	}

	if(arraylist_pushFilo(&cache->entries, entry)) {
		mutex_unlock(&cache->mutex);
		return -1;
	}

	mutex_unlock(&cache->mutex);

	log_logf(cache->log, LOG_LEVEL_DEBUG,
			"cache entry '%s' is %lli bytes in %lli blocks modified at %0.0f",
			entry->filename,
			entry->fileLength,
			entry->blockCount,
			entry->lastModifiedTime);

	return 0;
}

static LfbcEntry *newCacheEntry(Lfbc *cache, char *filename)
{
	LfbcEntry *entry = NULL;

	entry = (LfbcEntry *)malloc(sizeof(LfbcEntry));

	entry->isFileHandleOpen = afalse;
	entry->isLockedForReload = afalse;
	entry->fileLength = 0;
	entry->blockCount = 0;
	entry->mostRecentTimestamp = 0.0;
	entry->lastModifiedTime = 0.0;
	entry->filename = strdup(filename);
	entry->blocks = NULL;

	mutex_init(&entry->mutex);
	mutex_lock(&entry->mutex);

	if(initCacheEntry(cache, entry) < 0) {
		mutex_unlock(&entry->mutex);
		freeCacheEntry(entry);
		return NULL;
	}

	mutex_unlock(&entry->mutex);

	return entry;
}

static void freeCacheEntry(void *memory)
{
	int ii = 0;

	LfbcEntry *entry = NULL;

	if(memory == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	entry = (LfbcEntry *)memory;

	mutex_lock(&entry->mutex);

	if(entry->isFileHandleOpen) {
		file_free(&entry->fh);
		entry->isFileHandleOpen = afalse;
	}

	if(entry->blocks != NULL) {
		for(ii = 0; ii < entry->blockCount; ii++) {
			if(entry->blocks[ii].lock > 0) {
				fprintf(stderr, "[%s():%i] error - found entry '%s' block %i "
						"still in-use with %i users.\n", __FUNCTION__,
						__LINE__, entry->filename, ii, entry->blocks[ii].lock);
				continue;
			}

			if(entry->blocks[ii].block != NULL) {
				free(entry->blocks[ii].block);
			}
		}

		free(entry->blocks);
	}

	if(entry->filename != NULL) {
		free(entry->filename);
	}

	mutex_unlock(&entry->mutex);
	mutex_free(&entry->mutex);

	memset(entry, 0, sizeof(LfbcEntry));

	free(entry);
}

static char *getCacheEntryBLock(Lfbc *cache, LfbcEntry *entry,
		alint position, int *blockLength, alint *blockId)
{
	int rc = 0;
	alint blockPos = 0;
	char *result = NULL;

	if(position > entry->fileLength) {
		fprintf(stderr, "[%s():%i] error - invalid file postion %lli.\n",
				__FUNCTION__, __LINE__, position);
		return NULL;
	}

	blockPos = calculateBlockPosition(cache, position);
	if((blockPos < 0) || (blockPos >= entry->blockCount)) {
		fprintf(stderr, "[%s():%i] error - invalid block postion %lli.\n",
				__FUNCTION__, __LINE__, blockPos);
		return NULL;
	}

	mutex_lock(&entry->mutex);

	if(entry->isLockedForReload) {
		mutex_unlock(&entry->mutex);
		return NULL;
	}

	if(entry->blocks[blockPos].block == NULL) {
		if(!entry->isFileHandleOpen) {
			if((rc = openFile(cache, entry)) < 0) {
				fileError(cache, &entry->fh, "open", rc);
				mutex_unlock(&entry->mutex);
				return NULL;
			}
		}

		if((rc = file_seek(&entry->fh, position)) < 0) {
			fileError(cache, &entry->fh, "seek on", rc);
			mutex_unlock(&entry->mutex);
			return NULL;
		}

		entry->blocks[blockPos].lock = 0;
		entry->blocks[blockPos].block = allocateCacheBlock(cache);

		rc = file_read(&entry->fh, entry->blocks[blockPos].block,
				entry->blocks[blockPos].length);
		if((rc < 0) || (rc != entry->blocks[blockPos].length)) {
			fileError(cache, &entry->fh, "read from", rc);

			freeCacheBlock(cache, entry->blocks[blockPos].block);
			entry->blocks[blockPos].block = NULL;

			mutex_unlock(&entry->mutex);

			return NULL;
		}
	}

	*blockLength = entry->blocks[blockPos].length;
	result = entry->blocks[blockPos].block;

	entry->blocks[blockPos].lock += 1;
	entry->blocks[blockPos].timestamp = time_getTimeMus();
	entry->mostRecentTimestamp = time_getTimeMus();

	mutex_unlock(&entry->mutex);

	*blockId = blockPos;

	return result;
}

// cache functions

static char *allocateCacheBlock(Lfbc *cache)
{
	char *result = NULL;

	mutex_lock(&cache->mutex);
	result = arraylist_popLast(&cache->cache);
	mutex_unlock(&cache->mutex);

	if(result == NULL) {
		result = (char *)malloc(sizeof(char) * (cache->blockLength + 1));

		mutex_lock(&cache->mutex);
		cache->allocatedBlocks += 1;
		mutex_unlock(&cache->mutex);

		log_logf(cache->log, LOG_LEVEL_DEBUG,
				"allocated new cache block with %i allocated of %i",
				cache->allocatedBlocks, cache->maxBlocks);
	}
	else {
		log_logf(cache->log, LOG_LEVEL_DEBUG,
				"reused cache block from FILO list "
				"with %i allocated of %i with %i cached",
				cache->allocatedBlocks, cache->maxBlocks,
				arraylist_length(&cache->cache));
	}

	return result;
}

static void freeCacheBlock(Lfbc *cache, char *block)
{
	if(cache->allocatedBlocks > (cache->maxBlocks + FUDGE_FACTOR_LENGTH)) {
		free(block);

		mutex_lock(&cache->mutex);
		cache->allocatedBlocks -= 1;
		mutex_unlock(&cache->mutex);

		log_logf(cache->log, LOG_LEVEL_DEBUG,
				"freed cache block with %i allocated of %i",
				cache->allocatedBlocks, cache->maxBlocks);
	}
	else {
		memset(block, 0, (sizeof(char) * (cache->blockLength + 1)));

		mutex_lock(&cache->mutex);
		arraylist_pushFilo(&cache->cache, block);
		mutex_unlock(&cache->mutex);

		log_logf(cache->log, LOG_LEVEL_DEBUG,
				"retained cache block in FILO list "
				"with %i allocated of %i with %i cached",
				cache->allocatedBlocks, cache->maxBlocks,
				arraylist_length(&cache->cache));
	}
}

// cache management functions

static void *manageCacheThread(void *threadContext, void *argument)
{
	aboolean hasInUseBlocks = afalse;
	aboolean isFileHandleOpen = afalse;
	int ii = 0;
	int nn = 0;
	int length = 0;
	alint counter = 0;
	double modTime = 0;
	double currentTime = 0.0;
	double avgBlockAge = 0.0;
	double blockAgeSeconds = 0.0;

	TmfThread *thread = NULL;

	Lfbc *cache = NULL;
	LfbcEntry *entry = NULL;

	if((threadContext == NULL) || (argument == NULL)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	thread = (TmfThread *)threadContext;
	cache = (Lfbc *)argument;

	mutex_lock(&cache->mutex);
	cache->state = LFBC_STATE_RUNNING;
	mutex_unlock(&cache->mutex);

	while((thread->state == THREAD_STATE_RUNNING) &&
			(thread->signal == THREAD_SIGNAL_OK) &&
			(cache->state == LFBC_STATE_RUNNING)) {
		// determine if any cache entries changed

		mutex_lock(&cache->mutex);
		length = arraylist_length(&cache->entries);
		mutex_unlock(&cache->mutex);

		for(ii = 0; ii < length; ii++) {
			mutex_lock(&cache->mutex);
			entry = (LfbcEntry *)arraylist_get(&cache->entries, ii);
			mutex_unlock(&cache->mutex);

			if(entry == NULL) {
				continue;
			}

			if(entry->lastModifiedTime == 0.0) {
				continue;
			}

			system_fileModifiedTime(entry->filename, &modTime);

			if(modTime > entry->lastModifiedTime) {
				log_logf(cache->log, LOG_LEVEL_INFO,
						"noted that file '%s' was modified",
						entry->filename);

				mutex_lock(&entry->mutex);
				entry->isLockedForReload = atrue;
				mutex_unlock(&entry->mutex);

				counter = 0;
				hasInUseBlocks = atrue;

				do {
					hasInUseBlocks = afalse;

					mutex_lock(&entry->mutex);

					for(nn = 0; nn < entry->blockCount; nn++) {

						if((entry->blocks[nn].block != NULL) &&
								(entry->blocks[nn].lock > 0)) {
							hasInUseBlocks = atrue;
						}

						if(hasInUseBlocks) {
							break;
						}
					}

					mutex_unlock(&entry->mutex);

					if(!hasInUseBlocks) {
						break;
					}

					if((thread->state != THREAD_STATE_RUNNING) ||
							(thread->signal != THREAD_SIGNAL_OK)) {
						break;
					}

					time_usleep(1024);

					counter++;
				} while((hasInUseBlocks) && (counter < 8192));

				if(!hasInUseBlocks) {
					log_logf(cache->log, LOG_LEVEL_INFO,
							"removing entry for '%s' from cache",
							entry->filename);

					mutex_lock(&cache->mutex);

					isFileHandleOpen = entry->isFileHandleOpen;

					arraylist_delPosition(&cache->entries, ii);
					bptree_remove(&cache->index, entry->filename,
							strlen(entry->filename));

					mutex_unlock(&cache->mutex);

					if(isFileHandleOpen) {
						mutex_lock(&cache->mutex);
						cache->currentOpenFileHandles -= 1;
						mutex_unlock(&cache->mutex);
					}
				}
				else {
					mutex_lock(&entry->mutex);
					entry->isLockedForReload = afalse;
					mutex_unlock(&entry->mutex);
				}
			}

			if((thread->state != THREAD_STATE_RUNNING) ||
					(thread->signal != THREAD_SIGNAL_OK)) {
				break;
			}
		}

		if((thread->state != THREAD_STATE_RUNNING) ||
				(thread->signal != THREAD_SIGNAL_OK)) {
			break;
		}

		if(cache->allocatedBlocks > (cache->maxBlocks + FUDGE_FACTOR_LENGTH)) {
			counter = 0;
			avgBlockAge = 0.0;
			currentTime = time_getTimeMus();

			mutex_lock(&cache->mutex);
			length = arraylist_length(&cache->entries);
			mutex_unlock(&cache->mutex);

			for(ii = 0; ii < length; ii++) {
				mutex_lock(&cache->mutex);
				entry = (LfbcEntry *)arraylist_get(&cache->entries, ii);
				mutex_unlock(&cache->mutex);

				if(entry == NULL) {
					continue;
				}

				for(nn = 0; nn < entry->blockCount; nn++) {
					mutex_lock(&entry->mutex);

					if(entry->blocks[nn].block != NULL) {
						counter++;
						avgBlockAge += (currentTime -
								entry->blocks[nn].timestamp);
					}

					mutex_unlock(&entry->mutex);
				}

				if((thread->state != THREAD_STATE_RUNNING) ||
						(thread->signal != THREAD_SIGNAL_OK)) {
					break;
				}
			}

			if((thread->state != THREAD_STATE_RUNNING) ||
					(thread->signal != THREAD_SIGNAL_OK)) {
				break;
			}

			avgBlockAge /= (double)counter;

			for(ii = 0; ii < length; ii++) {
				mutex_lock(&cache->mutex);
				entry = (LfbcEntry *)arraylist_get(&cache->entries, ii);
				mutex_unlock(&cache->mutex);

				if(entry == NULL) {
					continue;
				}

				for(nn = 0; nn < entry->blockCount; nn++) {
					mutex_lock(&entry->mutex);

					if((entry->blocks[nn].block != NULL) &&
							(entry->blocks[nn].lock == 0)) {
						blockAgeSeconds = (currentTime -
								entry->blocks[nn].timestamp);
						if(blockAgeSeconds > avgBlockAge) {
							freeCacheBlock(cache, entry->blocks[nn].block);
							entry->blocks[nn].lock = 0;
							entry->blocks[nn].block = NULL;
							entry->blocks[nn].timestamp = 0.0;
						}
					}

					mutex_unlock(&entry->mutex);

					if(cache->allocatedBlocks <= cache->maxBlocks) {
						break;
					}
				}

				if(cache->allocatedBlocks <= cache->maxBlocks) {
					break;
				}

				if((thread->state != THREAD_STATE_RUNNING) ||
						(thread->signal != THREAD_SIGNAL_OK)) {
					break;
				}
			}
		}
		else {
			mutex_lock(&cache->mutex);
			length = arraylist_length(&cache->entries);
			mutex_unlock(&cache->mutex);

			if(length < 1) {
				length = FUDGE_FACTOR_LENGTH;
				if((cache->allocatedBlocks + length) < cache->maxBlocks) {
					for(ii = 0; ii < length; ii++) {
						freeCacheBlock(cache, allocateCacheBlock(cache));

						if((thread->state != THREAD_STATE_RUNNING) ||
								(thread->signal != THREAD_SIGNAL_OK)) {
							break;
						}
					}
				}
			}
		}

		if((thread->state != THREAD_STATE_RUNNING) ||
				(thread->signal != THREAD_SIGNAL_OK)) {
			break;
		}

		// take a nap

		counter = 0;

		while((thread->state == THREAD_STATE_RUNNING) &&
				(thread->signal == THREAD_SIGNAL_OK) &&
				(cache->state == LFBC_STATE_RUNNING) &&
				(counter < 128) &&
				(cache->allocatedBlocks <=
				 (cache->maxBlocks + FUDGE_FACTOR_LENGTH))) {
			time_usleep(1024);
			counter++;
		}
	}

	mutex_lock(&cache->mutex);

	if(cache->state == LFBC_STATE_SHUTDOWN) {
		cache->state = LFBC_STATE_STOPPED;
	}
	else {
		cache->state = LFBC_STATE_ERROR;
	}

	mutex_unlock(&cache->mutex);

	return NULL;
}


// define low-fragmentation block-cache public functions

void lfbc_init(Lfbc *cache, void *log)
{
	if(cache == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	memset(cache, 0, sizeof(Lfbc));

	cache->state = LFBC_STATE_INIT;
	cache->blockLength = LFBC_STATE_DEFAULT_BLOCK_LENGTH;
	cache->maxBlocks = LFBC_STATE_DEFAULT_MAX_BLOCKS;
	cache->allocatedBlocks = 0;
	cache->managerThreadId = 0;
	cache->maxOpenFileHandles = LFBC_STATE_DEFAULT_MAX_OPEN_FILE_HANDLES;
	cache->currentOpenFileHandles = 0;

	bptree_init(&cache->index);
	bptree_setFreeFunction(&cache->index, freeCacheEntry);

	arraylist_init(&cache->entries);
	arraylist_setFreeFunction(&cache->entries, freeNothing);

	arraylist_init(&cache->cache);

	tmf_init(&cache->tmf);

	if(log == NULL) {
		cache->isLogInternallyAllocated = atrue;
		cache->log = (Log *)malloc(sizeof(Log));

		log_init(cache->log, LOG_OUTPUT_STDOUT, NULL, LOG_LEVEL_INFO);
	}
	else {
		cache->isLogInternallyAllocated = afalse;
		cache->log = log;
	}

	mutex_init(&cache->mutex);
}

void lfbc_free(Lfbc *cache)
{
	if(cache == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	mutex_lock(&cache->mutex);

	if(cache->state == LFBC_STATE_RUNNING) {
		cache->state = LFBC_STATE_SHUTDOWN;

		mutex_unlock(&cache->mutex);

		if(tmf_free(&cache->tmf) < 0) {
			mutex_lock(&cache->mutex);
			cache->state = LFBC_STATE_ERROR;
			mutex_unlock(&cache->mutex);

			log_logf(cache->log, LOG_LEVEL_ERROR, 
					"failed to shutdown cache management thread");

			return;
		}

		mutex_lock(&cache->mutex);

		cache->state = LFBC_STATE_STOPPED;
	}
	else {
		tmf_free(&cache->tmf);
	}

	bptree_free(&cache->index);

	arraylist_free(&cache->entries);

	arraylist_free(&cache->cache);

	if(cache->isLogInternallyAllocated) {
		log_free(cache->log);
		free(cache->log);
	}

	mutex_unlock(&cache->mutex);
	mutex_free(&cache->mutex);

	memset(cache, 0, sizeof(Lfbc));
}

int lfbc_getState(Lfbc *cache)
{
	int result = 0;

	if(cache == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&cache->mutex);

	result = cache->state;

	mutex_unlock(&cache->mutex);

	return result;
}

int lfbc_getBlockLength(Lfbc *cache)
{
	int result = 0;

	if(cache == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&cache->mutex);

	result = cache->blockLength;

	mutex_unlock(&cache->mutex);

	return result;
}

void lfbc_setBlockLength(Lfbc *cache, int blockLength)
{
	if((cache == NULL) || (blockLength < 32)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	mutex_lock(&cache->mutex);

	if(cache->state != LFBC_STATE_INIT) {
		log_logf(cache->log, LOG_LEVEL_ERROR, 
				"unable to set block-length in state %i",
				cache->state);
	}
	else {
		cache->blockLength = blockLength;
	}

	mutex_unlock(&cache->mutex);
}

int lfbc_getMaxBlocks(Lfbc *cache)
{
	int result = 0;

	if(cache == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&cache->mutex);

	result = cache->maxBlocks;

	mutex_unlock(&cache->mutex);

	return result;
}

void lfbc_setMaxBlocks(Lfbc *cache, int maxBlocks)
{
	if((cache == NULL) || (maxBlocks < 128)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	mutex_lock(&cache->mutex);

	cache->maxBlocks = maxBlocks;

	mutex_unlock(&cache->mutex);
}

int lfbc_getMaxOpenFileHandles(Lfbc *cache)
{
	int result = 0;

	if(cache == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&cache->mutex);

	result = cache->maxOpenFileHandles;

	mutex_unlock(&cache->mutex);

	return result;
}

void lfbc_setMaxOpenFileHandles(Lfbc *cache, int maxOpenFileHandles)
{
	if((cache == NULL) || (maxOpenFileHandles < 32)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	mutex_lock(&cache->mutex);

	cache->maxOpenFileHandles = maxOpenFileHandles;

	mutex_unlock(&cache->mutex);
}

alint lfbc_getAllocatedMemory(Lfbc *cache)
{
	alint result = 0;

	if(cache == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&cache->mutex);

	result = ((alint)cache->blockLength * (alint)cache->allocatedBlocks);

	mutex_unlock(&cache->mutex);

	return result;
}

void *cache_getLog(Lfbc *cache)
{
	void *result = NULL;

	if(cache == NULL) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	mutex_lock(&cache->mutex);

	result = (void *)cache->log;

	mutex_unlock(&cache->mutex);

	return result;
}

int lfbc_start(Lfbc *cache)
{
	int ii = 0;
	int length = 0;

	if(cache == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	// spawn the cache-management thread

	mutex_lock(&cache->mutex);

	if(cache->state != LFBC_STATE_INIT) {
		log_logf(cache->log, LOG_LEVEL_ERROR, "unable to start in state %i",
				cache->state);
		mutex_unlock(&cache->mutex);
		return -1;
	}

	mutex_unlock(&cache->mutex);

	if(tmf_spawnThread(&cache->tmf,
				manageCacheThread,
				(void *)cache,
				&cache->managerThreadId) < 0) {
		mutex_lock(&cache->mutex);
		cache->state = LFBC_STATE_ERROR;
		mutex_unlock(&cache->mutex);

		log_logf(cache->log, LOG_LEVEL_ERROR,
				"failed to spawn cache management thread");

		return -1;
	}

	if(cache->state != LFBC_STATE_RUNNING) {
		log_logf(cache->log, LOG_LEVEL_ERROR,
				"failed to start cache management thread");
		return -1;
	}

	// create an initial set of cached memory blocks

	length = (cache->maxBlocks / 64);
	if(length < 1) {
		length = 128;
	}
	if(length > cache->maxBlocks) {
		length = cache->maxBlocks;
	}

	for(ii = 0; ii < length; ii++) {
		freeCacheBlock(cache, allocateCacheBlock(cache));
	}

	return 0;
}

aboolean lfbc_hasCache(Lfbc *cache, char *filename, alint *fileLength,
		alint *blockCount)
{
	aboolean exists = afalse;

	LfbcEntry *entry = NULL;

	if((cache == NULL) || (filename == NULL) || (fileLength == NULL) ||
			(blockCount == NULL)) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	*fileLength = 0;
	*blockCount = 0;

	if(cache->state != LFBC_STATE_RUNNING) {
		log_logf(cache->log, LOG_LEVEL_ERROR,
				"unable to check cache in state %i", cache->state);
		return afalse;
	}

	mutex_lock(&cache->mutex);

	if((bptree_get(&cache->index, filename, strlen(filename),
					((void *)&entry)) < 0) ||
			(entry == NULL)) {
		mutex_unlock(&cache->mutex);

		if((system_fileExists(filename, &exists) < 0) || (!exists)) {
			return afalse;
		}

		if((entry = newCacheEntry(cache, filename)) == NULL) {
			return afalse;
		}

		mutex_lock(&cache->mutex);
	}

	mutex_unlock(&cache->mutex);

	*fileLength = entry->fileLength;
	*blockCount = entry->blockCount;

	return atrue;
}

char *lfbc_get(Lfbc *cache, char *filename, alint position,
		int *blockLength, alint *blockId)
{
	aboolean exists = afalse;

	LfbcEntry *entry = NULL;

	if((cache == NULL) || (filename == NULL) || (position < 0) ||
			(blockLength == NULL) || (blockId == NULL)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	*blockId = 0;
	*blockLength = 0;

	if(cache->state != LFBC_STATE_RUNNING) {
		log_logf(cache->log, LOG_LEVEL_ERROR,
				"unable to get block in state %i", cache->state);
		return NULL;
	}

	if((position % (alint)cache->blockLength) != 0) {
		log_logf(cache->log, LOG_LEVEL_ERROR, "invalid file postion %lli",
				position);
		return NULL;
	}

	mutex_lock(&cache->mutex);

	if((bptree_get(&cache->index, filename, strlen(filename),
					((void *)&entry)) == 0) &&
			(entry != NULL)) {
		mutex_unlock(&cache->mutex);

		return getCacheEntryBLock(cache, entry, position, blockLength, blockId);
	}

	mutex_unlock(&cache->mutex);

	if((system_fileExists(filename, &exists) < 0) || (!exists)) {
		log_logf(cache->log, LOG_LEVEL_ERROR, "file '%s' does not exist",
				filename);
		return NULL;
	}

	if((entry = newCacheEntry(cache, filename)) == NULL) {
		return NULL;
	}

	return getCacheEntryBLock(cache, entry, position, blockLength,
			blockId);
}

int lfbc_releaseBlock(Lfbc *cache, char *filename, alint blockId)
{
	LfbcEntry *entry = NULL;

	if((cache == NULL) || (blockId < 0)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(cache->state != LFBC_STATE_RUNNING) {
		log_logf(cache->log, LOG_LEVEL_ERROR,
				"unable to release block in state %i", cache->state);
		return -2;
	}

	mutex_lock(&cache->mutex);

	if((bptree_get(&cache->index, filename, strlen(filename),
					((void *)&entry)) < 0) ||
			(entry == NULL)) {
		mutex_unlock(&cache->mutex);
		log_logf(cache->log, LOG_LEVEL_ERROR,
				"entry for '%s' does not exist", filename);
		return -3;
	}

	mutex_unlock(&cache->mutex);

	mutex_lock(&entry->mutex);

	if(blockId >= entry->blockCount) {
		mutex_unlock(&entry->mutex);
		log_logf(cache->log, LOG_LEVEL_ERROR,
				"block #%lli for '%s' is invalid", blockId, filename);
		return -4;
	}

	entry->blocks[blockId].lock -= 1;

	if(entry->blocks[blockId].lock < 0) {
		log_logf(cache->log, LOG_LEVEL_WARNING,
				"block #%lli locks set to %i, resetting",
				blockId, entry->blocks[blockId].lock);
		entry->blocks[blockId].lock = 0;
	}

	mutex_unlock(&entry->mutex);

	return 0;
}

