/*
 * managed_index.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard core library background thread managed indexing system, using a
 * B+Tree index, type library.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#define _CORE_LIBRARY_TYPES_COMPONENT
#include "core/library/types/managed_index.h"


// define managed index private constants

#define MANAGED_INDEX_VERBOSE_MODE						afalse


// define managed index private data types

typedef struct _EntryKey {
	int keyLength;
	char *key;
} EntryKey;


// declare managed index private functions

static void managedIndexDefaultFree(void *argument, void *memory);

static void managedIndexFree(void *argument, void *memory);

static void *managedIndexWorkerThread(void *threadContext, void *argument);

static void freeEntryKey(void *memory);


// define managed index private functions

static void managedIndexDefaultFree(void *argument, void *memory)
{
	if(memory == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	free(memory);
}

static void managedIndexFree(void *argument, void *memory)
{
	int memorySize = 0;

	ManagedIndex *index = NULL;
	ManagedIndexEntry *entry = NULL;
	ManagedIndexSimpleEntry *simpleEntry = NULL;

	if((argument == NULL) || (memory == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	index = (ManagedIndex *)argument;
	entry = (ManagedIndexEntry *)memory;

	memorySize = entry->memorySize;

	if(entry->key != NULL) {
		free(entry->key);
	}

	if(entry->value != NULL) {
		if(entry->isSimpleEntry) {
			simpleEntry = (ManagedIndexSimpleEntry *)entry->value;

			if(simpleEntry->value != NULL) {
				free(simpleEntry->value);
			}

			free(simpleEntry);
		}
		else {
			index->freeFunction(index->freeArgument, entry->value);
		}
	}

	free(entry);

	mutex_lock(&index->mutex);
	index->cachedItemCount -= (alint)1;
	index->cachedMemoryLength -= (alint)memorySize;
	mutex_unlock(&index->mutex);
}

static void freeEntryKey(void *memory)
{
	EntryKey *key = NULL;

	if(memory == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	key = (EntryKey *)memory;

	if(key->key != NULL) {
		free(key->key);
	}

	free(key);
}

static void *managedIndexWorkerThread(void *threadContext, void *argument)
{
	aboolean isEmpty = afalse;
	aboolean isDeleted = afalse;
	int counter = 0;
	int keyLength = 0;
	alint stackItemCount = 0;
	alint cachedItemCount = 0;
	alint cachedItemTicks = 0;
	alint cachedMemoryLength = 0;
	double cachedTimeout = 0.0;
	double elapsedTime = 0.0;
	char *key = NULL;

	FifoStack stack;

	EntryKey *entryKey = NULL;
	TmfThread *thread = NULL;
	ManagedIndex *index = NULL;
	ManagedIndexEntry *entry = NULL;

	if(MANAGED_INDEX_VERBOSE_MODE) {
		printf("%s() executed with arguments 0x%lx, 0x%lx\n", __FUNCTION__,
				(aptrcast)threadContext, (aptrcast)argument);
	}

	thread = (TmfThread *)threadContext;
	index = (ManagedIndex *)argument;

	if(MANAGED_INDEX_VERBOSE_MODE) {
		printf("%s() (thread #%i {%i}) running\n", __FUNCTION__, thread->uid,
				thread->systemId);
	}

	while((thread->state == THREAD_STATE_RUNNING) &&
			(thread->signal == THREAD_SIGNAL_OK)) {
		mutex_lock(thread->mutex);
		thread->status = THREAD_STATUS_BUSY;
		mutex_unlock(thread->mutex);

		if((index->cachedMemoryLength >= index->maxCachedMemoryLength) ||
				(index->cachedItemCount >= index->maxCachedItemCount)) {
			counter = 0;
			stackItemCount = 0;
			cachedItemCount = 0;
			cachedItemTicks = 0;
			cachedMemoryLength = 0;
			cachedTimeout = 0.0;

			fifostack_init(&stack);
			fifostack_setFreeFunction(&stack, freeEntryKey);

			keyLength = 1;
			key = (char *)malloc(sizeof(char) * (keyLength + 1));

			memset(key, 0, (int)(sizeof(char) * (keyLength + 1)));

			if(index->isSpinlockMode) {
				spinlock_readLock(&index->spinlock);
			}
			else {
				mutex_lock(&index->dataMutex);
			}

			while((thread->state == THREAD_STATE_RUNNING) &&
					(thread->signal == THREAD_SIGNAL_OK) &&
					(stackItemCount < 1024) &&
					(bptree_getNext(&index->index, key, keyLength,
									((void *)&entry)) == 0)) {
				free(key);

				keyLength = entry->keyLength;
				key = strndup(entry->key, entry->keyLength);

				isDeleted = afalse;

				if(index->timeoutSeconds > 0.0) {
					elapsedTime = time_getElapsedMusInSeconds(entry->timestamp);
					if(elapsedTime >= index->timeoutSeconds) {
						entryKey = (EntryKey *)malloc(sizeof(EntryKey));
						entryKey->keyLength = entry->keyLength;
						entryKey->key = strndup(entry->key, entry->keyLength);

						if(fifostack_push(&stack, entryKey) == 0) {
							stackItemCount++;
							isDeleted = atrue;
						}
					}
				}

				if(!isDeleted) {
					cachedItemTicks += entry->ticks;
					cachedMemoryLength += (alint)entry->memorySize;
					cachedTimeout += elapsedTime;
					cachedItemCount++;
				}

				counter++;

				if((counter % 8) == 0) {
					if(index->isSpinlockMode) {
						spinlock_readUnlock(&index->spinlock);
					}
					else {
						mutex_unlock(&index->dataMutex);
					}

					time_usleep(1024);

					if(index->isSpinlockMode) {
						spinlock_readLock(&index->spinlock);
					}
					else {
						mutex_lock(&index->dataMutex);
					}
				}
			}

			free(key);

			if(index->isSpinlockMode) {
				spinlock_readUnlock(&index->spinlock);
			}
			else {
				mutex_unlock(&index->dataMutex);
			}

			fifostack_isEmpty(&stack, &isEmpty);

			if(!isEmpty) {
				while((thread->state == THREAD_STATE_RUNNING) &&
						(thread->signal == THREAD_SIGNAL_OK) &&
						(fifostack_pop(&stack, ((void *)&entryKey)) == 0) &&
						(entryKey != NULL)) {
					managedIndex_remove(index, entryKey->key,
							entryKey->keyLength);

					free(entryKey->key);
					free(entryKey);
				}
			}

			if((thread->state == THREAD_STATE_RUNNING) &&
					(thread->signal == THREAD_SIGNAL_OK) &&
					(cachedItemCount > 0) &&
					(stackItemCount < 1024) &&
					((index->cachedMemoryLength >=
					  index->maxCachedMemoryLength) ||
					 (index->cachedItemCount >=
					  index->maxCachedItemCount))) {
				cachedItemTicks /= cachedItemCount;
				cachedMemoryLength /= cachedItemCount;
				cachedTimeout /= (double)cachedItemCount;

				stackItemCount = 0;
				cachedItemCount = 0;

				keyLength = 1;
				key = (char *)malloc(sizeof(char) * (keyLength + 1));

				memset(key, 0, (int)(sizeof(char) * (keyLength + 1)));

				if(index->isSpinlockMode) {
					spinlock_readLock(&index->spinlock);
				}
				else {
					mutex_lock(&index->dataMutex);
				}

				while((thread->state == THREAD_STATE_RUNNING) &&
						(thread->signal == THREAD_SIGNAL_OK) &&
						(stackItemCount < 1024) &&
						(bptree_getNext(&index->index, key, keyLength,
										((void *)&entry)) == 0)) {
					free(key);

					keyLength = entry->keyLength;
					key = strndup(entry->key, entry->keyLength);

					if(index->timeoutSeconds > 0.0) {
						elapsedTime = time_getElapsedMusInSeconds(
								entry->timestamp);
						if((elapsedTime >= index->timeoutSeconds) ||
								((entry->memorySize >= cachedMemoryLength) &&
								 (entry->ticks <= cachedItemTicks)) ||
								((elapsedTime >= cachedTimeout) &&
								 (entry->ticks <= cachedItemTicks))) {
							entryKey = (EntryKey *)malloc(sizeof(EntryKey));
							entryKey->keyLength = entry->keyLength;
							entryKey->key = strndup(entry->key,
									entry->keyLength);

							stackItemCount++;
							fifostack_push(&stack, entryKey);
						}
					}

					cachedItemCount++;

					if((cachedItemCount % 8) == 0) {
						if(index->isSpinlockMode) {
							spinlock_readUnlock(&index->spinlock);
						}
						else {
							mutex_unlock(&index->dataMutex);
						}

						time_usleep(1024);

						if(index->isSpinlockMode) {
							spinlock_readLock(&index->spinlock);
						}
						else {
							mutex_lock(&index->dataMutex);
						}
					}
				}

				free(key);

				if(index->isSpinlockMode) {
					spinlock_readUnlock(&index->spinlock);
				}
				else {
					mutex_unlock(&index->dataMutex);
				}
			}

			while((thread->state == THREAD_STATE_RUNNING) &&
					(thread->signal == THREAD_SIGNAL_OK) &&
					(fifostack_pop(&stack, ((void *)&entryKey)) == 0) &&
					(entryKey != NULL) &&
					((index->cachedMemoryLength >=
					  index->maxCachedMemoryLength) ||
					 (index->cachedItemCount >=
					  index->maxCachedItemCount))) {
				managedIndex_remove(index, entryKey->key,
						entryKey->keyLength);

				free(entryKey->key);
				free(entryKey);
			}

			fifostack_free(&stack);
		}

		mutex_lock(thread->mutex);
		thread->status = THREAD_STATUS_NAPPING;
		mutex_unlock(thread->mutex);

		// take a nap

		counter = 0;

		while((thread->state == THREAD_STATE_RUNNING) &&
				(thread->signal == THREAD_SIGNAL_OK) &&
				(counter < 1024) &&
				(index->cachedMemoryLength < index->maxCachedMemoryLength) &&
				(index->cachedItemCount < index->maxCachedItemCount)) {
			time_usleep(1024);
			counter++;
		}
	}

	if(MANAGED_INDEX_VERBOSE_MODE) {
		printf("%s() (thread #%i {%i}) halting\n", __FUNCTION__, thread->uid,
				thread->systemId);
	}

	return NULL;
}


// define managed index public functions

int managedIndex_init(ManagedIndex *index, aboolean isSpinlockMode)
{
	int threadId = 0;

	if(index == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	memset(index, 0, (int)(sizeof(ManagedIndex)));

	index->isSpinlockMode = isSpinlockMode;
	index->cachedItemCount = 0;
	index->maxCachedItemCount = MANAGED_INDEX_DEFAULT_MAX_CACHED_ITEM_COUNT;
	index->cachedMemoryLength = 0;
	index->maxCachedMemoryLength =
		MANAGED_INDEX_DEFAULT_MAX_CACHED_MEMORY_LENGTH;
	index->timeoutSeconds = MANAGED_INDEX_DEFAULT_TIMEOUT_SECONDS;
	index->freeArgument = NULL;
	index->freeFunction = (ManagedIndexFreeFunction)managedIndexDefaultFree;

	bptree_init(&index->index);
	bptree_setComplexFreeFunction(&index->index, index,
			(BptreeComplexFreeFunction)managedIndexFree);

	tmf_init(&index->tmf);

	spinlock_init(&index->spinlock);

	mutex_init(&index->dataMutex);
	mutex_init(&index->mutex);

	mutex_lock(&index->mutex);

	if(tmf_spawnThread(&index->tmf, managedIndexWorkerThread, index,
				&threadId) < 0) {
		mutex_unlock(&index->mutex);
		return -1;
	}

	mutex_unlock(&index->mutex);

	return 0;
}

int managedIndex_free(ManagedIndex *index)
{
	int result = 0;

	if(index == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&index->mutex);

	if(tmf_free(&index->tmf) < 0) {
		result = -1;
	}

	if(index->isSpinlockMode) {
		if(spinlock_writeLock(&index->spinlock) < 0) {
			mutex_unlock(&index->mutex);
			return -1;
		}
	}
	else {
		mutex_lock(&index->dataMutex);
	}

	mutex_unlock(&index->mutex);

	if(bptree_free(&index->index) < 0) {
		result = -1;
	}

	mutex_lock(&index->mutex);

	if(index->isSpinlockMode) {
		if(spinlock_writeUnlock(&index->spinlock) < 0) {
			mutex_unlock(&index->mutex);
			return -1;
		}
	}
	else {
		mutex_unlock(&index->dataMutex);
	}

	spinlock_free(&index->spinlock);
	mutex_free(&index->dataMutex);

	mutex_unlock(&index->mutex);

	mutex_free(&index->mutex);

	memset(index, 0, (int)(sizeof(ManagedIndex)));

	return result;
}

int managedIndex_setFreeFunction(ManagedIndex *index, void *argument,
		ManagedIndexFreeFunction freeFunction)
{
	if((index == NULL) || (freeFunction == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&index->mutex);

	index->freeArgument = argument;
	index->freeFunction = freeFunction;

	mutex_unlock(&index->mutex);

	return 0;
}

int managedIndex_setMaxCachedItemCount(ManagedIndex *index,
		alint maxCachedItemCount)
{
	if((index == NULL) || (maxCachedItemCount < 0)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&index->mutex);

	index->maxCachedItemCount = maxCachedItemCount;

	mutex_unlock(&index->mutex);

	return 0;
}

int managedIndex_setMaxCachedMemoryLength(ManagedIndex *index,
		alint maxCachedMemoryLength)
{
	if((index == NULL) || (maxCachedMemoryLength < 0)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&index->mutex);

	index->maxCachedMemoryLength = maxCachedMemoryLength;

	mutex_unlock(&index->mutex);

	return 0;
}

int managedIndex_setTimeout(ManagedIndex *index, double timeoutSeconds)
{
	if((index == NULL) || (timeoutSeconds < 0.0)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&index->mutex);

	index->timeoutSeconds = timeoutSeconds;

	mutex_unlock(&index->mutex);

	return 0;
}

int managedIndex_getCacheStatus(ManagedIndex *index, alint *cachedItemCount,
		alint *cachedMemoryLength)
{
	if((index == NULL) || (cachedItemCount == NULL) ||
			(cachedMemoryLength == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&index->mutex);

	*cachedItemCount = index->cachedItemCount;
	*cachedMemoryLength = index->cachedMemoryLength;

	mutex_unlock(&index->mutex);

	return 0;
}

int managedIndex_getLeafCount(ManagedIndex *index, int *leafCount)
{
	int result = 0;

	if((index == NULL) || (leafCount == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(index->isSpinlockMode) {
		if(spinlock_readLock(&index->spinlock) < 0) {
			return -1;
		}
	}
	else {
		mutex_lock(&index->dataMutex);
	}

	result = bptree_getLeafCount(&index->index, leafCount);

	if(index->isSpinlockMode) {
		if(spinlock_readUnlock(&index->spinlock) < 0) {
			return -1;
		}
	}
	else {
		mutex_unlock(&index->dataMutex);
	}

	return 0;
}

int managedIndex_clear(ManagedIndex *index)
{
	int result = 0;

	if(index == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&index->mutex);

	if(index->isSpinlockMode) {
		if(spinlock_writeLock(&index->spinlock) < 0) {
			mutex_unlock(&index->mutex);
			return -1;
		}
	}
	else {
		mutex_lock(&index->dataMutex);
	}

	if(bptree_clear(&index->index) < 0) {
		result = -1;
	}

	if(index->isSpinlockMode) {
		if(spinlock_writeUnlock(&index->spinlock) < 0) {
			mutex_unlock(&index->mutex);
			return -1;
		}
	}
	else {
		mutex_unlock(&index->dataMutex);
	}

	mutex_unlock(&index->mutex);

	return result;
}

int managedIndex_get(ManagedIndex *index, char *key, int keyLength,
		void **value)
{
	int result = 0;

	ManagedIndexEntry *entry = NULL;
	ManagedIndexSimpleEntry *simpleEntry = NULL;

	if((index == NULL) || (key == NULL) || (keyLength < 1) ||
			(value == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	*value = NULL;

	if(index->isSpinlockMode) {
		if(spinlock_readLock(&index->spinlock) < 0) {
			return -1;
		}
	}
	else {
		mutex_lock(&index->dataMutex);
	}

	if((result = bptree_get(&index->index, key, keyLength,
					((void *)&entry))) == 0) {
		entry->ticks += 1;
		entry->timestamp = time_getTimeMus();
		if(entry->isSimpleEntry) {
			simpleEntry = (ManagedIndexSimpleEntry *)entry->value;
			*value = simpleEntry->value;
		}
		else {
			*value = entry->value;
		}
	}

	if(index->isSpinlockMode) {
		if(spinlock_readUnlock(&index->spinlock) < 0) {
			return -1;
		}
	}
	else {
		mutex_unlock(&index->dataMutex);
	}

	return result;
}

int managedIndex_getSimple(ManagedIndex *index, char *key, int keyLength,
		char **value, int *valueLength)
{
	int result = 0;

	ManagedIndexEntry *entry = NULL;
	ManagedIndexSimpleEntry *simpleEntry = NULL;

	if((index == NULL) || (key == NULL) || (keyLength < 1) ||
			(value == NULL) || (valueLength == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	*value = NULL;
	*valueLength = 0;

	if(index->isSpinlockMode) {
		if(spinlock_readLock(&index->spinlock) < 0) {
			return -1;
		}
	}
	else {
		mutex_lock(&index->dataMutex);
	}

	if((result = bptree_get(&index->index, key, keyLength,
					((void *)&entry))) == 0) {
		entry->ticks += 1;
		entry->timestamp = time_getTimeMus();
		if(entry->isSimpleEntry) {
			simpleEntry = (ManagedIndexSimpleEntry *)entry->value;
			*value = simpleEntry->value;
			*valueLength = simpleEntry->length;
		}
		else {
			result = -1;
		}
	}

	if(index->isSpinlockMode) {
		if(spinlock_readUnlock(&index->spinlock) < 0) {
			return -1;
		}
	}
	else {
		mutex_unlock(&index->dataMutex);
	}

	return result;
}

int managedIndex_getNext(ManagedIndex *index, char *key, int keyLength,
		void **value)
{
	int result = 0;

	ManagedIndexEntry *entry = NULL;
	ManagedIndexSimpleEntry *simpleEntry = NULL;

	if((index == NULL) || (key == NULL) || (keyLength < 1) ||
			(value == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	*value = NULL;

	if(index->isSpinlockMode) {
		if(spinlock_readLock(&index->spinlock) < 0) {
			return -1;
		}
	}
	else {
		mutex_lock(&index->dataMutex);
	}

	if((result = bptree_getNext(&index->index, key, keyLength,
					((void *)&entry))) == 0) {
		entry->ticks += 1;
		entry->timestamp = time_getTimeMus();
		if(entry->isSimpleEntry) {
			simpleEntry = (ManagedIndexSimpleEntry *)entry->value;
			*value = simpleEntry->value;
		}
		else {
			*value = entry->value;
		}
	}

	if(index->isSpinlockMode) {
		if(spinlock_readUnlock(&index->spinlock) < 0) {
			return -1;
		}
	}
	else {
		mutex_unlock(&index->dataMutex);
	}

	return result;
}

int managedIndex_getNextSimple(ManagedIndex *index, char *key, int keyLength,
		char **value, int *valueLength)
{
	int result = 0;

	ManagedIndexEntry *entry = NULL;
	ManagedIndexSimpleEntry *simpleEntry = NULL;

	if((index == NULL) || (key == NULL) || (keyLength < 1) ||
			(value == NULL) || (valueLength == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	*value = NULL;
	*valueLength = 0;

	if(index->isSpinlockMode) {
		if(spinlock_readLock(&index->spinlock) < 0) {
			return -1;
		}
	}
	else {
		mutex_lock(&index->dataMutex);
	}

	if((result = bptree_getNext(&index->index, key, keyLength,
					((void *)&entry))) == 0) {
		entry->ticks += 1;
		entry->timestamp = time_getTimeMus();
		if(entry->isSimpleEntry) {
			simpleEntry = (ManagedIndexSimpleEntry *)entry->value;
			*value = simpleEntry->value;
			*valueLength = simpleEntry->length;
		}
		else {
			result = -1;
		}
	}

	if(index->isSpinlockMode) {
		if(spinlock_readUnlock(&index->spinlock) < 0) {
			return -1;
		}
	}
	else {
		mutex_unlock(&index->dataMutex);
	}

	return result;
}

int managedIndex_getPrevious(ManagedIndex *index, char *key, int keyLength,
		void **value)
{
	int result = 0;

	ManagedIndexEntry *entry = NULL;
	ManagedIndexSimpleEntry *simpleEntry = NULL;

	if((index == NULL) || (key == NULL) || (keyLength < 1) ||
			(value == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	*value = NULL;

	if(index->isSpinlockMode) {
		if(spinlock_readLock(&index->spinlock) < 0) {
			return -1;
		}
	}
	else {
		mutex_lock(&index->dataMutex);
	}

	if((result = bptree_getPrevious(&index->index, key, keyLength,
					((void *)&entry))) == 0) {
		entry->ticks += 1;
		entry->timestamp = time_getTimeMus();
		if(entry->isSimpleEntry) {
			simpleEntry = (ManagedIndexSimpleEntry *)entry->value;
			*value = simpleEntry->value;
		}
		else {
			*value = entry->value;
		}
	}

	if(index->isSpinlockMode) {
		if(spinlock_readUnlock(&index->spinlock) < 0) {
			return -1;
		}
	}
	else {
		mutex_unlock(&index->dataMutex);
	}

	return result;
}

int managedIndex_getPreviousSimple(ManagedIndex *index, char *key,
		int keyLength, char **value, int *valueLength)
{
	int result = 0;

	ManagedIndexEntry *entry = NULL;
	ManagedIndexSimpleEntry *simpleEntry = NULL;

	if((index == NULL) || (key == NULL) || (keyLength < 1) ||
			(value == NULL) || (valueLength == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	*value = NULL;
	*valueLength = 0;

	if(index->isSpinlockMode) {
		if(spinlock_readLock(&index->spinlock) < 0) {
			return -1;
		}
	}
	else {
		mutex_lock(&index->dataMutex);
	}

	if((result = bptree_getPrevious(&index->index, key, keyLength,
					((void *)&entry))) == 0) {
		entry->ticks += 1;
		entry->timestamp = time_getTimeMus();
		if(entry->isSimpleEntry) {
			simpleEntry = (ManagedIndexSimpleEntry *)entry->value;
			*value = simpleEntry->value;
			*valueLength = simpleEntry->length;
		}
		else {
			result = -1;
		}
	}

	if(index->isSpinlockMode) {
		if(spinlock_readUnlock(&index->spinlock) < 0) {
			return -1;
		}
	}
	else {
		mutex_unlock(&index->dataMutex);
	}

	return result;
}

int managedIndex_put(ManagedIndex *index, char *key, int keyLength,
		void *value, int memorySizeOfValue)
{
	int result = 0;

	ManagedIndexEntry *entry = NULL;

	if((index == NULL) || (key == NULL) || (keyLength < 1) ||
			(value == NULL) || (memorySizeOfValue < 1)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	// determine if key already exists

	if(index->isSpinlockMode) {
		if(spinlock_readLock(&index->spinlock) < 0) {
			return -1;
		}
	}
	else {
		mutex_lock(&index->dataMutex);
	}

	if((result = bptree_get(&index->index, key, keyLength,
					((void *)&entry))) == 0) {
		result = -1;
	}
	else {
		result = 0;
	}

	if(index->isSpinlockMode) {
		if(spinlock_readUnlock(&index->spinlock) < 0) {
			return -1;
		}
	}
	else {
		mutex_unlock(&index->dataMutex);
	}

	if(result < 0) {
		return result;
	}

	// add entry to index

	entry = (ManagedIndexEntry *)malloc(sizeof(ManagedIndexEntry));

	entry->isSimpleEntry = afalse;
	entry->keyLength = keyLength;
	entry->memorySize = (keyLength + memorySizeOfValue);
	entry->ticks = 0;
	entry->timestamp = time_getTimeMus();
	entry->key = strndup(key, keyLength);
	entry->value = value;

	if(index->isSpinlockMode) {
		if(spinlock_writeLock(&index->spinlock) < 0) {
			managedIndexFree(index, entry);
			return -1;
		}
	}
	else {
		mutex_lock(&index->dataMutex);
	}

	result = bptree_put(&index->index, key, keyLength, (void *)entry);

	if(index->isSpinlockMode) {
		if(spinlock_writeUnlock(&index->spinlock) < 0) {
			managedIndexFree(index, entry);
			return -1;
		}
	}
	else {
		mutex_unlock(&index->dataMutex);
	}

	if(result < 0) {
		managedIndexFree(index, entry);
	}
	else {
		mutex_lock(&index->mutex);
		index->cachedItemCount += (alint)1;
		index->cachedMemoryLength += (alint)entry->memorySize;
		mutex_unlock(&index->mutex);
	}

	return result;
}

int managedIndex_putSimple(ManagedIndex *index, char *key, int keyLength,
		char *value, int valueLength)
{
	int result = 0;

	ManagedIndexEntry *entry = NULL;
	ManagedIndexSimpleEntry *simpleEntry = NULL;

	if((index == NULL) || (key == NULL) || (keyLength < 1) ||
			(value == NULL) || (valueLength < 1)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	// determine if key already exists

	if(index->isSpinlockMode) {
		if(spinlock_readLock(&index->spinlock) < 0) {
			return -1;
		}
	}
	else {
		mutex_lock(&index->dataMutex);
	}

	if((result = bptree_get(&index->index, key, keyLength,
					((void *)&entry))) == 0) {
		result = -1;
	}
	else {
		result = 0;
	}

	if(index->isSpinlockMode) {
		if(spinlock_readUnlock(&index->spinlock) < 0) {
			return -1;
		}
	}
	else {
		mutex_unlock(&index->dataMutex);
	}

	if(result < 0) {
		return result;
	}

	// add simple entry to index

	simpleEntry = (ManagedIndexSimpleEntry *)malloc(
			sizeof(ManagedIndexSimpleEntry));

	simpleEntry->length = valueLength;
	simpleEntry->value = value;

	entry = (ManagedIndexEntry *)malloc(sizeof(ManagedIndexEntry));

	entry->isSimpleEntry = atrue;
	entry->keyLength = keyLength;
	entry->memorySize = (keyLength +
			sizeof(ManagedIndexSimpleEntry) +
			simpleEntry->length);
	entry->ticks = 0;
	entry->timestamp = time_getTimeMus();
	entry->key = strndup(key, keyLength);
	entry->value = (void *)simpleEntry;

	if(index->isSpinlockMode) {
		if(spinlock_writeLock(&index->spinlock) < 0) {
			managedIndexFree(index, entry);
			return -1;
		}
	}
	else {
		mutex_lock(&index->dataMutex);
	}

	result = bptree_put(&index->index, key, keyLength, (void *)entry);

	if(index->isSpinlockMode) {
		if(spinlock_writeUnlock(&index->spinlock) < 0) {
			managedIndexFree(index, entry);
			return -1;
		}
	}
	else {
		mutex_unlock(&index->dataMutex);
	}

	if(result < 0) {
		managedIndexFree(index, entry);
	}
	else {
		mutex_lock(&index->mutex);
		index->cachedItemCount += (alint)1;
		index->cachedMemoryLength += (alint)entry->memorySize;
		mutex_unlock(&index->mutex);
	}

	return result;
}

int managedIndex_remove(ManagedIndex *index, char *key, int keyLength)
{
	int result = 0;

	if((index == NULL) || (key == NULL) || (keyLength < 1)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(index->isSpinlockMode) {
		if(spinlock_writeLock(&index->spinlock) < 0) {
			return -1;
		}
	}
	else {
		mutex_lock(&index->dataMutex);
	}

	result = bptree_remove(&index->index, key, keyLength);

	if(index->isSpinlockMode) {
		if(spinlock_writeUnlock(&index->spinlock) < 0) {
			return -1;
		}
	}
	else {
		mutex_unlock(&index->dataMutex);
	}

	return result;
}

