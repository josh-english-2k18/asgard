/*
 * container.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * A library component for the search-engine to contain key-value attributes
 * in a highly-memory-optmized serial buffer.
 *
 * Written by Josh English.
 */

// preprocessor directives

#define _CORE_COMMON_BYPASS_STD_STREAM

#include "core/core.h"
#include "search/search.h"
#define _SEARCH_CORE_COMPONENT
#include "search/core/container.h"


// define container private functions

// general functions

static int calculateHashMapHash(char *string, int stringLength)
{
	int result = 0;

	if((string == NULL) || (stringLength < 1)) {
		return 0;
	}

	if((result = (int)crc32_calculateHash(string, stringLength)) < 0) {
		result *= -1;
	}

	result %= CONTAINER_HASH_MAP_ELEMENTS;

	return result;
}

static char *buildErrorMessage(char flags, const char *format, ...)
{
	int resultLength = 0;
	int bufferLength = 0;
	char *result = NULL;
	char *buffer = NULL;

	va_list vaArgs;

	va_start(vaArgs, format);
	buffer = vaprintf(format, vaArgs, &bufferLength);
	va_end(vaArgs);

	resultLength = (bufferLength + 128);
	result = (char *)malloc(sizeof(char) * resultLength);

	snprintf(result, (resultLength - 1),
			"failed to %s on container with \"%s\"",
			container_flagToString(flags),
			buffer);

	free(buffer);

	return result;
}

// index functions

static ContainerIndex *indexNew(ContainerValueTypes type, char *name,
		int nameLength, int blockRef, int blockLength)
{
	ContainerIndex *result = NULL;

	result = (ContainerIndex *)malloc(sizeof(ContainerIndex));

	result->type = type;
	result->blockRef = blockRef;
	result->blockLength = blockLength;
	result->nameLength = nameLength;
	result->name = strndup(name, nameLength);
	result->next = NULL;

	return result;
}

static void indexFree(ContainerIndex *index)
{
	if(index == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	if(index->name != NULL) {
		free(index->name);
	}
	else {
		fprintf(stderr, "[%s():%i] error - detected index 0x%lx name is "
				"NULL.\n", __FUNCTION__, __LINE__, (unsigned long int)index);
	}

	memset(index, 0, (int)(sizeof(ContainerIndex)));

	free(index);
}

static ContainerIndex *indexGet(Container *container, char *key, int keyLength,
		int *hash)
{
	ContainerIndex *link = NULL;

	*hash = calculateHashMapHash(key, keyLength);

	for(link = container->mapRoot[(*hash)]; link != NULL; link = link->next) {
		if(keyLength != link->nameLength) {
			continue;
		}

		if(!strncmp(key, link->name, link->nameLength)) {
			return link;
		}
	}

	return NULL;
}

static int indexPut(Container *container, ContainerValueTypes type, char *key,
		int keyLength, int blockRef, int blockLength)
{
	int hash = 0;

	ContainerIndex *link = NULL;

	if(indexGet(container, key, keyLength, &hash) != NULL) {
		return -1;
	}

	link = indexNew(type, key, keyLength, blockRef, blockLength);

	if(container->mapRoot[hash] == NULL) {
		container->mapRoot[hash] = link;
	}
	else {
		container->mapEnd[hash]->next = link;
	}

	container->mapEnd[hash] = link;

	return 0;
}

static int indexPutInIndex(Container *container, int hash,
		ContainerValueTypes type, char *name, int nameLength, int blockRef,
		int blockLength)
{
	ContainerIndex *link = NULL;

	link = indexNew(type, name, nameLength, blockRef, blockLength);

	if(container->mapRoot[hash] == NULL) {
		container->mapRoot[hash] = link;
	}
	else {
		container->mapEnd[hash]->next = link;
	}

	container->mapEnd[hash] = link;

	return 0;
}

// container functions

static int addEntityToContainer(Container *container, ContainerValueTypes type,
		char *name, int nameLength, char *value, int valueLength)
{
	int addLength = 0;
	char *ptr = NULL;

	switch(type) {
		case CONTAINER_TYPE_BOOLEAN:
		case CONTAINER_TYPE_INTEGER:
		case CONTAINER_TYPE_DOUBLE:
		case CONTAINER_TYPE_STRING:
			if(valueLength < 1) {
				return -1;
			}
			break;

		case CONTAINER_TYPE_END_OF_LIST:
		default:
			return -1;
	}

	if(indexPut(container, type, name, nameLength, container->blockRef,
				valueLength) < 0) {
		return -1;
	}

	if(type == CONTAINER_TYPE_STRING) {
		valueLength += 1; // include the null byte
	}

//	nameLength = (strlen(name) + 1); // include the null byte
	nameLength = (nameLength + 1); // include the null byte

	addLength = (nameLength + valueLength);

	if((container->blockRef + addLength) > container->blockLength) {
		container->blockLength +=
			((int)(addLength / CONTAINER_MEMORY_BLOCK_LENGTH) + 1) *
			CONTAINER_MEMORY_BLOCK_LENGTH;

		if(container->block == NULL) {
			container->block = (char *)malloc(sizeof(char) *
					container->blockLength);
		}
		else {
			container->block = (char *)realloc(container->block,
					(sizeof(char) * container->blockLength));
		}
	}

	ptr = (container->block + container->blockRef);

	memcpy(ptr, name, nameLength);

	ptr += nameLength;

	memcpy(ptr, value, valueLength);

	ptr += valueLength;

	if((int)(ptr - container->block) != (container->blockRef + addLength)) {
		fprintf(stderr, "[%s():%i] error - detected internal block copy "
				"mismatch with %i vs %i.\n", __FUNCTION__, __LINE__,
				(int)(container->block - ptr),
				(container->blockRef + addLength));
		return -1;
	}

	container->blockRef += addLength;
	container->entityCount += 1;

	return 0;
}


// define container public functions

// container functions

int container_init(Container *container)
{
	int ii = 0;

	if(container == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	memset(container, 0, (int)(sizeof(Container)));

	container->uid = 0;
	container->blockRef = 0;
	container->blockLength = 0;
	container->entityCount = 0;
	container->nameLength = 0;
	container->name = NULL;
	container->block = NULL;
	container->timestamp = time_getTimeMus();

	for(ii = 0; ii < CONTAINER_HASH_MAP_ELEMENTS; ii++) {
		container->mapRoot[ii] = NULL;
		container->mapEnd[ii] = NULL;
	}

	if(spinlock_init(&container->lock) < 0) {
		return -1;
	}

	return 0;
}

int container_initWithName(Container *container, char *name, int nameLength)
{
	if((container == NULL) || (name == NULL) || (nameLength < 1)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	container_init(container);

	if(spinlock_writeLock(&container->lock) < 0) {
		return -1;
	}

	container->nameLength = nameLength;
	container->name = strndup(name, nameLength);

	spinlock_writeUnlock(&container->lock);

	return 0;
}

Container *container_new()
{
	Container *result = NULL;

	result = (Container *)malloc(sizeof(Container));

	if(container_init(result) < 0) {
		container_freePtr(result);
		return NULL;
	}

	return result;
}

Container *container_newWithName(char *name, int nameLength)
{
	Container *result = NULL;

	if((name == NULL) || (nameLength < 1)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	result = (Container *)malloc(sizeof(Container));

	if(container_initWithName(result, name, nameLength) < 0) {
		container_freePtr(result);
		return NULL;
	}

	return result;
}

int container_free(Container *container)
{
	int ii = 0;

	ContainerIndex *link = NULL;
	ContainerIndex *next = NULL;

	if(container == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(spinlock_writeLock(&container->lock) < 0) {
		return -1;
	}

	if(container->name != NULL) {
		free(container->name);
	}

	for(ii = 0; ii < CONTAINER_HASH_MAP_ELEMENTS; ii++) {
		for(link = container->mapRoot[ii]; link != NULL; link = next) {
			next = link->next;
			indexFree(link);
		}
	}

	if(container->block != NULL) {
		free(container->block);
	}

	spinlock_writeUnlock(&container->lock);

	if(spinlock_free(&container->lock) < 0) {
		return -1;
	}

	memset(container, 0, (int)(sizeof(Container)));

	return 0;
}

int container_freePtr(Container *container)
{
	if(container == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(container_free(container) < 0) {
		return -1;
	}

	free(container);

	return 0;
}

char *container_typeToString(ContainerValueTypes type)
{
	char *result = NULL;

	switch(type) {
		case CONTAINER_TYPE_BOOLEAN:
			result = "Boolean";
			break;

		case CONTAINER_TYPE_INTEGER:
			result = "Integer";
			break;

		case CONTAINER_TYPE_DOUBLE:
			result = "Double";
			break;

		case CONTAINER_TYPE_STRING:
			result = "String";
			break;

		case CONTAINER_TYPE_END_OF_LIST:
		default:
			result = "Unknown";
	}

	return result;
}

char *container_iteratorToString(ContainerIteratorResults iResult)
{
	char *result = NULL;

	switch(iResult) {
		case CONTAINER_ITERATOR_OK:
			result = "Ok";
			break;

		case CONTAINER_ITERATOR_END_OF_LIST:
			result = "End of List";
			break;

		case CONTAINER_ITERATOR_ERROR:
			result = "Error";
			break;

		default:
			result = "Unknown";
	}

	return result;
}

char *container_flagToString(char flag)
{
	aboolean hasData = afalse;
	char *result = NULL;
	char buffer[1024];

	memset(buffer, 0, (int)(sizeof(buffer)));

	if(flag & CONTAINER_FLAG_UPDATE) {
		strcat(buffer, "Update");
		hasData = atrue;
	}

	if(flag & CONTAINER_FLAG_APPEND) {
		if(hasData) {
			strcat(buffer, "|Append");
		}
		else {
			strcat(buffer, "Append");
			hasData = atrue;
		}
	}

	if(flag & CONTAINER_FLAG_UPPEND) {
		if(hasData) {
			strcat(buffer, "|Uppend");
		}
		else {
			strcat(buffer, "Uppend");
			hasData = atrue;
		}
	}

	if(flag & CONTAINER_FLAG_REPLACE) {
		if(hasData) {
			strcat(buffer, "|Replace");
		}
		else {
			strcat(buffer, "Replace");
			hasData = atrue;
		}
	}

	result = buffer;

	return result;
}

aboolean container_isLocked(Container *container)
{
	if(container == NULL) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	return spinlock_isLocked(&container->lock);
}

aboolean container_isReadLocked(Container *container)
{
	int threadCount = 0;

	if(container == NULL) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	if(spinlock_getSpinThreadCount(&container->lock, &threadCount) < 0) {
		return afalse;
	}

	if(threadCount > 0) {
		return atrue;
	}

	return afalse;
}

int container_lock(Container *container)
{
	if(container == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return spinlock_readLock(&container->lock);
}

int container_unlock(Container *container)
{
	if(container == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return spinlock_readUnlock(&container->lock);
}

int container_getUid(Container *container)
{
	int result = 0;

	if(container == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(spinlock_readLock(&container->lock) < 0) {
		return -1;
	}

	result = container->uid;

	spinlock_readUnlock(&container->lock);

	return result;
}

int container_setUid(Container *container, int uid)
{
	if(container == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(spinlock_writeLock(&container->lock) < 0) {
		return -1;
	}

	container->uid = uid;

	spinlock_writeUnlock(&container->lock);

	return 0;
}

char *container_getName(Container *container, int *nameLength)
{
	char *result = NULL;

	if((container == NULL) || (nameLength == NULL)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	*nameLength = 0;

	if(spinlock_readLock(&container->lock) < 0) {
		return NULL;
	}

	*nameLength = container->nameLength;
	result = container->name;

	spinlock_readUnlock(&container->lock);

	return result;
}

int container_setName(Container *container, char *name, int nameLength)
{
	char *oldName = NULL;
	char *newName = NULL;

	if((container == NULL) || (nameLength < 1)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	newName = strndup(name, nameLength);

	if(spinlock_writeLock(&container->lock) < 0) {
		free(newName);
		return -1;
	}

	oldName = container->name;

	container->nameLength = nameLength;
	container->name = newName;

	spinlock_writeUnlock(&container->lock);

	if(oldName != NULL) {
		free(oldName);
	}

	return 0;
}

int container_peg(Container *container)
{
	if(container == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(spinlock_writeLock(&container->lock) < 0) {
		return -1;
	}

	container->timestamp = time_getTimeMus();

	spinlock_writeUnlock(&container->lock);

	return 0;
}

double container_getAgeInSeconds(Container *container)
{
	double result = 0.0;

	if(container == NULL) {
		DISPLAY_INVALID_ARGS;
		return 0.0;
	}

	if(spinlock_readLock(&container->lock) < 0) {
		return -1;
	}

	result = time_getElapsedMusInSeconds(container->timestamp);

	spinlock_readUnlock(&container->lock);

	return result;
}

aboolean container_exists(Container *container, char *name, int nameLength)
{
	int hash = 0;

	ContainerIndex *index = NULL;

	if((container == NULL) || (name == NULL) || (nameLength < 1)) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	if(spinlock_readLock(&container->lock) < 0) {
		return -1;
	}

	if((index = indexGet(container, name, nameLength, &hash)) == NULL) {
		spinlock_readUnlock(&container->lock);
		return afalse;
	}

	spinlock_readUnlock(&container->lock);

	return atrue;
}

ContainerValueTypes container_getType(Container *container, char *name,
		int nameLength)
{
	int hash = 0;

	ContainerIndex *index = NULL;
	ContainerValueTypes result = CONTAINER_TYPE_END_OF_LIST;

	if((container == NULL) || (name == NULL) || (nameLength < 1)) {
		DISPLAY_INVALID_ARGS;
		return CONTAINER_TYPE_END_OF_LIST;
	}

	if(spinlock_readLock(&container->lock) < 0) {
		return -1;
	}

	if((index = indexGet(container, name, nameLength, &hash)) == NULL) {
		spinlock_readUnlock(&container->lock);
		return CONTAINER_TYPE_END_OF_LIST;
	}

	result = index->type;

	spinlock_readUnlock(&container->lock);

	return result;
}

int container_putBoolean(Container *container, char *name, int nameLength,
		aboolean value)
{
	char cValue[SIZEOF_BOOLEAN];

	if((container == NULL) || (name == NULL) || (nameLength < 1)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	cValue[0] = (char)value;

	if(spinlock_writeLock(&container->lock) < 0) {
		return -1;
	}

	if(addEntityToContainer(container, CONTAINER_TYPE_BOOLEAN, name,
				nameLength, cValue, SIZEOF_BOOLEAN) < 0) {
		spinlock_writeUnlock(&container->lock);
		return -1;
	}

	spinlock_writeUnlock(&container->lock);

	return 0;
}

aboolean container_getBoolean(Container *container, char *name, int nameLength)
{
	aboolean result = afalse;
	int ref = 0;
	int hash = 0;
	int intResult = 0;
	double doubleResult = 0.0;
	char *stringResult = NULL;
	char iValue[SIZEOF_INT];
	char dValue[SIZEOF_DOUBLE];

	ContainerIndex *index = NULL;

	if((container == NULL) || (name == NULL) || (nameLength < 1)) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	if(spinlock_readLock(&container->lock) < 0) {
		return -1;
	}

	if((index = indexGet(container, name, nameLength, &hash)) == NULL) {
		spinlock_readUnlock(&container->lock);
		return -1;
	}

	ref = index->blockRef;

	if((nameLength = (strlen(container->block + ref) + 1)) < 1) {
		spinlock_readUnlock(&container->lock);
		return -1;
	}

	switch(index->type) {
		case CONTAINER_TYPE_BOOLEAN:
			if((ref + nameLength + SIZEOF_BOOLEAN) > container->blockRef) {
				spinlock_readUnlock(&container->lock);
				return -1;
			}

			result = (aboolean)(*(container->block + ref + nameLength));
			break;

		case CONTAINER_TYPE_INTEGER:
			if((ref + nameLength + SIZEOF_INT) > container->blockRef) {
				spinlock_readUnlock(&container->lock);
				return -1;
			}

			memcpy(iValue, (container->block + ref + nameLength), SIZEOF_INT);

			if(serialize_decodeInt(iValue, SIZEOF_INT, &intResult) < 0) {
				spinlock_readUnlock(&container->lock);
				return -1;
			}

			if(intResult == 0) {
				result = afalse;
			}
			else {
				result = atrue;
			}
			break;

		case CONTAINER_TYPE_DOUBLE:
			if((ref + nameLength + SIZEOF_DOUBLE) > container->blockRef) {
				spinlock_readUnlock(&container->lock);
				return -1;
			}

			memcpy(dValue, (container->block + ref + nameLength),
					SIZEOF_DOUBLE);

			if(serialize_decodeDouble(dValue, SIZEOF_DOUBLE,
						&doubleResult) < 0) {
				spinlock_readUnlock(&container->lock);
				return 0.0;
			}

			if(doubleResult == 0.0) {
				result = afalse;
			}
			else {
				result = atrue;
			}
			break;

		case CONTAINER_TYPE_STRING:
			if((ref + nameLength + 1) > container->blockRef) {
				spinlock_readUnlock(&container->lock);
				return -1;
			}

			stringResult = (container->block + ref + nameLength);

			if((!strcasecmp(stringResult, "true")) ||
					(!strcasecmp(stringResult, "1")) ||
					(atoi(stringResult) != 0)) {
				result = atrue;
			}
			else {
				result = afalse;
			}
			break;

		case CONTAINER_TYPE_END_OF_LIST:
		default:
			spinlock_readUnlock(&container->lock);
			return -1;
	}

	spinlock_readUnlock(&container->lock);

	return result;
}

int container_putInteger(Container *container, char *name, int nameLength,
		int value)
{
	char cValue[SIZEOF_INT];

	if((container == NULL) || (name == NULL) || (nameLength < 1)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(serialize_encodeInt(value, cValue) < 0) {
		return -1;
	}

	if(spinlock_writeLock(&container->lock) < 0) {
		return -1;
	}

	if(addEntityToContainer(container, CONTAINER_TYPE_INTEGER, name,
				nameLength, cValue, SIZEOF_INT) < 0) {
		spinlock_writeUnlock(&container->lock);
		return -1;
	}

	spinlock_writeUnlock(&container->lock);

	return 0;
}

int container_getInteger(Container *container, char *name, int nameLength)
{
	int ref = 0;
	int hash = 0;
	int result = 0;
	double doubleResult = 0.0;
	char iValue[SIZEOF_INT];
	char dValue[SIZEOF_DOUBLE];

	ContainerIndex *index = NULL;

	if((container == NULL) || (name == NULL) || (nameLength < 1)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(spinlock_readLock(&container->lock) < 0) {
		return -1;
	}

	if((index = indexGet(container, name, nameLength, &hash)) == NULL) {
		spinlock_readUnlock(&container->lock);
		return -1;
	}

	ref = index->blockRef;

	nameLength += 1;
/*	if((nameLength = (strlen(container->block + ref) + 1)) < 1) {
		spinlock_readUnlock(&container->lock);
		return -1;
	}*/

	switch(index->type) {
		case CONTAINER_TYPE_BOOLEAN:
			if((ref + nameLength + SIZEOF_BOOLEAN) > container->blockRef) {
				spinlock_readUnlock(&container->lock);
				return -1;
			}

			if((aboolean)(*(container->block + ref + nameLength))) {
				result = 1;
			}
			else {
				result = 0;
			}
			break;

		case CONTAINER_TYPE_INTEGER:
			if((ref + nameLength + SIZEOF_INT) > container->blockRef) {
				spinlock_readUnlock(&container->lock);
				return -1;
			}

			memcpy(iValue, (container->block + ref + nameLength), SIZEOF_INT);

			if(serialize_decodeInt(iValue, SIZEOF_INT, &result) < 0) {
				spinlock_readUnlock(&container->lock);
				return -1;
			}
			break;

		case CONTAINER_TYPE_DOUBLE:
			if((ref + nameLength + SIZEOF_DOUBLE) > container->blockRef) {
				spinlock_readUnlock(&container->lock);
				return -1;
			}

			memcpy(dValue, (container->block + ref + nameLength),
					SIZEOF_DOUBLE);

			if(serialize_decodeDouble(dValue, SIZEOF_DOUBLE,
						&doubleResult) < 0) {
				spinlock_readUnlock(&container->lock);
				return 0.0;
			}

			result = (int)doubleResult;
			break;

		case CONTAINER_TYPE_STRING:
			if((ref + nameLength + 1) > container->blockRef) {
				spinlock_readUnlock(&container->lock);
				return -1;
			}

			result = atoi(container->block + ref + nameLength);
			break;

		case CONTAINER_TYPE_END_OF_LIST:
		default:
			spinlock_readUnlock(&container->lock);
			return -1;
	}

	spinlock_readUnlock(&container->lock);

	return result;
}

int container_putDouble(Container *container, char *name, int nameLength,
		double value)
{
	char cValue[SIZEOF_DOUBLE];

	if((container == NULL) || (name == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(serialize_encodeDouble(value, cValue) < 0) {
		return -1;
	}

	if(spinlock_writeLock(&container->lock) < 0) {
		return -1;
	}

	if(addEntityToContainer(container, CONTAINER_TYPE_DOUBLE, name,
				nameLength, cValue, SIZEOF_DOUBLE) < 0) {
		spinlock_writeUnlock(&container->lock);
		return -1;
	}

	spinlock_writeUnlock(&container->lock);

	return 0;
}

double container_getDouble(Container *container, char *name, int nameLength)
{
	int ref = 0;
	int hash = 0;
	int intResult = 0;
	double result = 0.0;
	char iValue[SIZEOF_INT];
	char dValue[SIZEOF_DOUBLE];

	ContainerIndex *index = NULL;

	if((container == NULL) || (name == NULL) || (nameLength < 1)) {
		DISPLAY_INVALID_ARGS;
		return 0.0;
	}

	if(spinlock_readLock(&container->lock) < 0) {
		return 0.0;
	}

	if((index = indexGet(container, name, nameLength, &hash)) == NULL) {
		spinlock_readUnlock(&container->lock);
		return 0.0;
	}

	ref = index->blockRef;

	nameLength += 1;
/*	if((nameLength = (strlen(container->block + ref) + 1)) < 1) {
		spinlock_readUnlock(&container->lock);
		return 0.0;
	}*/

	switch(index->type) {
		case CONTAINER_TYPE_BOOLEAN:
			if((ref + nameLength + SIZEOF_BOOLEAN) > container->blockRef) {
				spinlock_readUnlock(&container->lock);
				return 0.0;
			}

			if((aboolean)(*(container->block + ref + nameLength))) {
				result = 1.0;
			}
			else {
				result = 0.0;
			}
			break;

		case CONTAINER_TYPE_INTEGER:
			if((ref + nameLength + SIZEOF_INT) > container->blockRef) {
				spinlock_readUnlock(&container->lock);
				return 0.0;
			}

			memcpy(iValue, (container->block + ref + nameLength), SIZEOF_INT);

			if(serialize_decodeInt(iValue, SIZEOF_INT, &intResult) < 0) {
				spinlock_readUnlock(&container->lock);
				return 0.0;
			}

			result = (double)intResult;
			break;

		case CONTAINER_TYPE_DOUBLE:
			if((ref + nameLength + SIZEOF_DOUBLE) > container->blockRef) {
				spinlock_readUnlock(&container->lock);
				return 0.0;
			}

			memcpy(dValue, (container->block + ref + nameLength),
					SIZEOF_DOUBLE);

			if(serialize_decodeDouble(dValue, SIZEOF_DOUBLE, &result) < 0) {
				spinlock_readUnlock(&container->lock);
				return 0.0;
			}
			break;

		case CONTAINER_TYPE_STRING:
			if((ref + nameLength + 1) > container->blockRef) {
				spinlock_readUnlock(&container->lock);
				return 0.0;
			}

			result = atod(container->block + ref + nameLength);
			break;

		case CONTAINER_TYPE_END_OF_LIST:
		default:
			spinlock_readUnlock(&container->lock);
			return 0.0;
	}

	spinlock_readUnlock(&container->lock);

	return result;
}

int container_putString(Container *container, char *name, int nameLength,
		char *value, int length)
{
	if((container == NULL) || (name == NULL) || (nameLength < 1) ||
			(value == NULL) || (length < 1)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(spinlock_writeLock(&container->lock) < 0) {
		return -1;
	}

	if(addEntityToContainer(container, CONTAINER_TYPE_STRING, name,
				nameLength, value, length) < 0) {
		spinlock_writeUnlock(&container->lock);
		return -1;
	}

	spinlock_writeUnlock(&container->lock);

	return 0;
}

char *container_getString(Container *container, char *name, int nameLength,
		int *length)
{
	int ref = 0;
	int hash = 0;
	int intResult = 0;
	double doubleResult = 0.0;
	char *result = NULL;
	char iValue[SIZEOF_INT];
	char dValue[SIZEOF_DOUBLE];
	char buffer[1024];

	ContainerIndex *index = NULL;

	if((container == NULL) || (name == NULL) || (nameLength < 1) ||
			(length == NULL)) {
		DISPLAY_INVALID_ARGS;

		if(length != NULL) {
			*length = 0;
		}

		return NULL;
	}

	*length = 0;

	if(spinlock_readLock(&container->lock) < 0) {
		return NULL;
	}

	if((index = indexGet(container, name, nameLength, &hash)) == NULL) {
		spinlock_readUnlock(&container->lock);
		return NULL;
	}

	ref = index->blockRef;

	nameLength += 1;
/*	if((nameLength = (strlen(container->block + ref) + 1)) < 1) {
		spinlock_readUnlock(&container->lock);
		return NULL;
	}*/

	switch(index->type) {
		case CONTAINER_TYPE_BOOLEAN:
			if((ref + nameLength + SIZEOF_BOOLEAN) > container->blockRef) {
				spinlock_readUnlock(&container->lock);
				return NULL;
			}

			if((aboolean)(*(container->block + ref + nameLength))) {
				result = strndup("true", 4);
				*length = 4;
			}
			else {
				result = strndup("false", 5);
				*length = 5;
			}
			break;

		case CONTAINER_TYPE_INTEGER:
			if((ref + nameLength + SIZEOF_INT) > container->blockRef) {
				spinlock_readUnlock(&container->lock);
				return NULL;
			}

			memcpy(iValue, (container->block + ref + nameLength), SIZEOF_INT);

			if(serialize_decodeInt(iValue, SIZEOF_INT, &intResult) < 0) {
				spinlock_readUnlock(&container->lock);
				return NULL;
			}

			*length = snprintf(buffer, ((int)sizeof(buffer) - 1), "%i",
					intResult);

			result = strndup(buffer, (*length));
			break;

		case CONTAINER_TYPE_DOUBLE:
			if((ref + nameLength + SIZEOF_DOUBLE) > container->blockRef) {
				spinlock_readUnlock(&container->lock);
				return NULL;
			}

			memcpy(dValue, (container->block + ref + nameLength),
					SIZEOF_DOUBLE);

			if(serialize_decodeDouble(dValue, SIZEOF_DOUBLE,
						&doubleResult) < 0) {
				spinlock_readUnlock(&container->lock);
				return NULL;
			}

			*length = snprintf(buffer, ((int)sizeof(buffer) - 1), "%f",
					doubleResult);

			result = strndup(buffer, (*length));
			break;

		case CONTAINER_TYPE_STRING:
			if((ref + nameLength + 1) > container->blockRef) {
				spinlock_readUnlock(&container->lock);
				return NULL;
			}

			result = strndup((container->block + ref + nameLength),
					index->blockLength);
			*length = index->blockLength;
			break;

		case CONTAINER_TYPE_END_OF_LIST:
		default:
			spinlock_readUnlock(&container->lock);
			return NULL;
	}

	spinlock_readUnlock(&container->lock);

	return result;
}

int container_calculateMemoryLength(Container *container)
{
	int ii = 0;
	int result = 0;

	ContainerIndex *link = NULL;

	if(container == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(spinlock_readLock(&container->lock) < 0) {
		return -1;
	}

	result += (int)sizeof(Container);

	if(container->name != NULL) {
		result += container->nameLength;
	}

	result += container->blockLength;

	for(ii = 0; ii < CONTAINER_HASH_MAP_ELEMENTS; ii++) {
		for(link = container->mapRoot[ii]; link != NULL; link = link->next) {
			result += ((int)sizeof(ContainerIndex) + link->nameLength);
		}
	}

	spinlock_readUnlock(&container->lock);

	return result;
}

char *container_serialize(Container *container, int *length)
{
	int ii = 0;
	int nameLength = 0;
	int indexLength = 0;
	int resultLength = 0;
	char *ptr = NULL;
	char *result = NULL;
	char iValue[SIZEOF_INT];
	char dValue[SIZEOF_DOUBLE];

	ContainerIndex *link = NULL;

	if((container == NULL) || (length == NULL)) {
		DISPLAY_INVALID_ARGS;

		if(length != NULL) {
			*length = 0;
		}

		return NULL;
	}

	if(spinlock_readLock(&container->lock) < 0) {
		return NULL;
	}

	// calculate serialized length

	if(container->name != NULL) {
		nameLength = container->nameLength;
	}

	for(ii = 0; ii < CONTAINER_HASH_MAP_ELEMENTS; ii++) {
		indexLength += SIZEOF_INT;			// index length

		for(link = container->mapRoot[ii]; link != NULL; link = link->next) {
			indexLength += (
					SIZEOF_INT +			// type
					SIZEOF_INT +			// block ref
					SIZEOF_INT +			// block length
					SIZEOF_INT +			// name length
					link->nameLength		// name
				);
		}
	}

	resultLength = (
			SIZEOF_INT +			// uid
			SIZEOF_INT +			// entity count
			SIZEOF_DOUBLE +			// timestamp
			SIZEOF_INT +			// name length
			nameLength +			// name
			SIZEOF_INT +			// block length
			indexLength +			// index length
			container->blockRef		// block
		);

	result = (char *)malloc(sizeof(char) * (resultLength + 1));
	ptr = result;

	// uid

	serialize_encodeInt(container->uid, iValue);
	memcpy(ptr, iValue, SIZEOF_INT);
	ptr += SIZEOF_INT;

	// entity count

	serialize_encodeInt(container->entityCount, iValue);
	memcpy(ptr, iValue, SIZEOF_INT);
	ptr += SIZEOF_INT;

	// timestamp

	serialize_encodeDouble(container->timestamp, dValue);
	memcpy(ptr, dValue, SIZEOF_DOUBLE);
	ptr += SIZEOF_DOUBLE;

	// name length

	serialize_encodeInt(nameLength, iValue);
	memcpy(ptr, iValue, SIZEOF_INT);
	ptr += SIZEOF_INT;

	// name

	if((container->name != NULL) && (nameLength > 0)) {
		memcpy(ptr, container->name, nameLength);
		ptr += nameLength;
	}

	// block length

	serialize_encodeInt(container->blockRef, iValue);
	memcpy(ptr, iValue, SIZEOF_INT);
	ptr += SIZEOF_INT;

	// index

	for(ii = 0; ii < CONTAINER_HASH_MAP_ELEMENTS; ii++) {
		// calculate index length

		indexLength = 0;

		for(link = container->mapRoot[ii]; link != NULL; link = link->next) {
			indexLength++;
		}

		// index length

		serialize_encodeInt(indexLength, iValue);
		memcpy(ptr, iValue, SIZEOF_INT);
		ptr += SIZEOF_INT;

		// indexes

		for(link = container->mapRoot[ii]; link != NULL; link = link->next) {
			// type

			serialize_encodeInt((int)link->type, iValue);
			memcpy(ptr, iValue, SIZEOF_INT);
			ptr += SIZEOF_INT;

			// block ref

			serialize_encodeInt(link->blockRef, iValue);
			memcpy(ptr, iValue, SIZEOF_INT);
			ptr += SIZEOF_INT;

			// block length

			serialize_encodeInt(link->blockLength, iValue);
			memcpy(ptr, iValue, SIZEOF_INT);
			ptr += SIZEOF_INT;

			// name length

			nameLength = link->nameLength;

			serialize_encodeInt(nameLength, iValue);
			memcpy(ptr, iValue, SIZEOF_INT);
			ptr += SIZEOF_INT;

			// name

			memcpy(ptr, link->name, nameLength);
			ptr += nameLength;
		}
	}

	// block

	memcpy(ptr, container->block, container->blockRef);
	ptr += container->blockRef;

	spinlock_readUnlock(&container->lock);

	// sanity check result

	if((int)((aptrcast)(ptr - result)) != resultLength) {
		free(result);
		return NULL;
	}

	*length = resultLength;

	return result;
}

Container *container_deserialize(char *container, int length)
{
	int ii = 0;
	int nn = 0;
	int indexType = 0;
	int indexLength = 0;
	int indexBlockRef = 0;
	int indexBlockLength = 0;
	int indexNameLength = 0;
	int nameLength = 0;
	char *ptr = NULL;
	char *indexName = NULL;

	Container *result = NULL;

	if((container == NULL) || (length < 1)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	result = container_new();

	ptr = container;

	// uid

	if(serialize_decodeInt(ptr, (length - (int)(ptr - container)),
				&result->uid) < 0) {
		container_freePtr(result);
		return NULL;
	}

	ptr += SIZEOF_INT;

	// entity count

	if(serialize_decodeInt(ptr, (length - (int)(ptr - container)),
				&result->entityCount) < 0) {
		container_freePtr(result);
		return NULL;
	}

	ptr += SIZEOF_INT;

	// timestamp

	if(serialize_decodeDouble(ptr, (length - (int)(ptr - container)),
				&result->timestamp) < 0) {
		container_freePtr(result);
		return NULL;
	}

	ptr += SIZEOF_DOUBLE;

	// name length

	if(serialize_decodeInt(ptr, (length - (int)(ptr - container)),
				&nameLength) < 0) {
		container_freePtr(result);
		return NULL;
	}

	ptr += SIZEOF_INT;

	if(nameLength >= ((length - (int)(ptr - container)) - SIZEOF_INT)) {
		container_freePtr(result);
		return NULL;
	}

	// name

	if(nameLength > 0) {
		result->name = strndup(ptr, nameLength);

		ptr += nameLength;
	}

	// block length

	if(serialize_decodeInt(ptr, (length - (int)(ptr - container)),
				&result->blockRef) < 0) {
		container_freePtr(result);
		return NULL;
	}

	ptr += SIZEOF_INT;

	if(result->blockRef > (length - (int)(ptr - container))) {
		container_freePtr(result);
		return NULL;
	}

	// index

	for(ii = 0; ii < CONTAINER_HASH_MAP_ELEMENTS; ii++) {
		// index length

		if(serialize_decodeInt(ptr, (length - (int)(ptr - container)),
					&indexLength) < 0) {
			container_freePtr(result);
			return NULL;
		}

		ptr += SIZEOF_INT;

		if(indexLength >= ((length - (int)(ptr - container)) - SIZEOF_INT)) {
			container_freePtr(result);
			return NULL;
		}

		// indexes

		for(nn = 0; nn < indexLength; nn++) {
			// type

			if(serialize_decodeInt(ptr, (length - (int)(ptr - container)),
						&indexType) < 0) {
				container_freePtr(result);
				return NULL;
			}

			ptr += SIZEOF_INT;

			if((indexType != CONTAINER_TYPE_BOOLEAN) &&
					(indexType != CONTAINER_TYPE_INTEGER) &&
					(indexType != CONTAINER_TYPE_DOUBLE) &&
					(indexType != CONTAINER_TYPE_STRING)) {
				container_freePtr(result);
				return NULL;
			}

			// block ref

			if(serialize_decodeInt(ptr, (length - (int)(ptr - container)),
						&indexBlockRef) < 0) {
				container_freePtr(result);
				return NULL;
			}

			ptr += SIZEOF_INT;

			if((indexBlockRef < 0) || (indexBlockRef >= result->blockRef)) {
				container_freePtr(result);
				return NULL;
			}

			// block length

			if(serialize_decodeInt(ptr, (length - (int)(ptr - container)),
						&indexBlockLength) < 0) {
				container_freePtr(result);
				return NULL;
			}

			ptr += SIZEOF_INT;

			if((indexBlockLength < 0) ||
					((indexBlockRef + indexBlockLength) >= result->blockRef)) {
				container_freePtr(result);
				return NULL;
			}

			// name length

			if(serialize_decodeInt(ptr, (length - (int)(ptr - container)),
						&indexNameLength) < 0) {
				container_freePtr(result);
				return NULL;
			}

			ptr += SIZEOF_INT;

			if((indexNameLength < 1) ||
					(indexNameLength >=
					 ((length - (int)(ptr - container)) - SIZEOF_INT))) {
				container_freePtr(result);
				return NULL;
			}

			// name

			indexName = strndup(ptr, indexNameLength);

			ptr += indexNameLength;

			// create index

			if(indexPutInIndex(result, ii, (ContainerValueTypes)indexType,
						indexName, indexNameLength, indexBlockRef,
						indexBlockLength) < 0) {
				free(indexName);
				container_freePtr(result);
				return NULL;
			}

			// cleanup

			free(indexName);
		}
	}

	// block

	result->blockLength +=
		((int)(result->blockRef / CONTAINER_MEMORY_BLOCK_LENGTH) + 1) *
		CONTAINER_MEMORY_BLOCK_LENGTH;

	result->block = (char *)malloc(sizeof(char) * result->blockLength);

	memcpy(result->block, ptr, result->blockRef);

	ptr += result->blockRef;

	// sanity-check result

	if((int)((aptrcast)(ptr - container)) != length) {
		container_freePtr(result);
		return NULL;
	}

	return result;
}

// iterator functions

int containerIterator_init(Container *container, ContainerIterator *iterator)
{
	if((container == NULL) || (iterator == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	memset(iterator, 0, (int)(sizeof(ContainerIterator)));

	if(container_lock(container) < 0) {
		return -1;
	}

	iterator->mapReference = 0;
	iterator->index = NULL;
	iterator->container = container;

	mutex_init(&iterator->mutex);

	return 0;
}

ContainerIterator *containerIterator_new(Container *container)
{
	ContainerIterator *result = NULL;

	if(container == NULL) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	result = (ContainerIterator *)malloc(sizeof(ContainerIterator));

	if(containerIterator_init(container, result) < 0) {
		containerIterator_freePtr(result);
		return NULL;
	}

	return result;
}

int containerIterator_free(ContainerIterator *iterator)
{
	if(iterator == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&iterator->mutex);

	if(container_unlock(iterator->container) < 0) {
		mutex_unlock(&iterator->mutex);
		return -1;
	}

	mutex_unlock(&iterator->mutex);

	mutex_free(&iterator->mutex);

	memset(iterator, 0, (int)(sizeof(ContainerIterator)));

	return 0;
}

int containerIterator_freePtr(ContainerIterator *iterator)
{
	if(iterator == NULL) {
		DISPLAY_INVALID_ARGS;
		return 0;
	}

	if(containerIterator_free(iterator) < 0) {
		return -1;
	}

	free(iterator);

	return 0;
}

int containerIterator_next(ContainerIterator *iterator)
{
	int ii = 0;
	int ref = 0;
	int nameLength = 0;
	char cValue[128];

	if(iterator == NULL) {
		DISPLAY_INVALID_ARGS;
		return CONTAINER_ITERATOR_ERROR;
	}

	mutex_lock(&iterator->mutex);

	memset(&(iterator->entity), 0, (int)(sizeof(ContainerIteratorEntity)));

	if(iterator->index == NULL) {
		if(iterator->mapReference >= CONTAINER_HASH_MAP_ELEMENTS) {
			mutex_unlock(&iterator->mutex);
			return CONTAINER_ITERATOR_END_OF_LIST;
		}

		for(ii = iterator->mapReference;
				ii < CONTAINER_HASH_MAP_ELEMENTS;
				ii++) {
			if(iterator->container->mapRoot[ii] != NULL) {
				iterator->index = iterator->container->mapRoot[ii];
				break;
			}
		}

		iterator->mapReference = ii;
	}

	if(iterator->index == NULL) {
		mutex_unlock(&iterator->mutex);
		return CONTAINER_ITERATOR_END_OF_LIST;
	}

	ref = iterator->index->blockRef;

//	if(((nameLength = (strlen(iterator->container->block + ref) + 1)) > 0) &&
	if(((nameLength = (iterator->index->nameLength + 1)) > 0) &&
			((ref + nameLength + 1) <= iterator->container->blockRef)) {
		iterator->entity.type = iterator->index->type;
		iterator->entity.nameLength = (nameLength - 1);
		iterator->entity.name =  (iterator->container->block + ref);
		iterator->entity.valueLength = iterator->index->blockLength;

		switch(iterator->entity.type) {
			case CONTAINER_TYPE_BOOLEAN:
				iterator->entity.bValue = (aboolean)(*(
							iterator->container->block +
							ref +
							nameLength));
				break;

			case CONTAINER_TYPE_INTEGER:
				memcpy(cValue, (iterator->container->block + ref + nameLength),
						SIZEOF_INT);

				if(serialize_decodeInt(cValue, SIZEOF_INT,
							&iterator->entity.iValue) < 0) {
					mutex_unlock(&iterator->mutex);
					return CONTAINER_ITERATOR_ERROR;
				}
				break;

			case CONTAINER_TYPE_DOUBLE:
				memcpy(cValue, (iterator->container->block + ref + nameLength),
						SIZEOF_DOUBLE);

				if(serialize_decodeDouble(cValue, SIZEOF_DOUBLE,
							&iterator->entity.dValue) < 0) {
					mutex_unlock(&iterator->mutex);
					return CONTAINER_ITERATOR_ERROR;
				}
				break;

			case CONTAINER_TYPE_STRING:
				iterator->entity.sValue = (iterator->container->block +
						ref +
						nameLength);
				break;

			case CONTAINER_TYPE_END_OF_LIST:
			default:
				mutex_unlock(&iterator->mutex);
				return CONTAINER_ITERATOR_ERROR;
		}
	}
	else {
		return CONTAINER_ITERATOR_ERROR;
	}

	iterator->index = iterator->index->next;

	if(iterator->index == NULL) {
		iterator->mapReference += 1;
	}

	mutex_unlock(&iterator->mutex);

	return CONTAINER_ITERATOR_OK;
}

int containerIterator_rewind(ContainerIterator *iterator)
{
	if(iterator == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&iterator->mutex);

	memset(&(iterator->entity), 0, (int)(sizeof(ContainerIteratorEntity)));

	iterator->mapReference = 0;
	iterator->index = NULL;

	mutex_unlock(&iterator->mutex);

	return 0;
}

// helper functions

Container *container_jsonToContainer(Json *object)
{
	int ii = 0;
	int nn = 0;
	int uid = 0;
	int arrayLength = 0;
	int sValueLength = 0;
	int tempNameLength = 0;
	double dValue = 0.0;
	double timestamp = 0.0;
	char *name = NULL;
	char *sValue = NULL;
	char tempName[128];

	Json *containerObject = NULL;
	Container *result = NULL;

	if(object == NULL) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	if((containerObject = json_getObject(object,
					"containerMetaData")) != NULL) {
		if(json_elementExists(containerObject, "uid")) {
			uid = (int)json_getNumber(containerObject, "uid");
		}

		if(json_elementExists(containerObject, "timestamp")) {
			timestamp = json_getNumber(containerObject, "timestamp");
		}

		if(json_elementExists(containerObject, "name")) {
			name = json_getString(containerObject, "name");
		}
	}

	if(name != NULL) {
		result = container_newWithName(name, strlen(name));
	}
	else {
		result = container_new();
	}

	if(uid > 0) {
		result->uid = uid;
	}

	if(timestamp > 0.0) {
		result->timestamp = timestamp;
	}
	else {
		result->timestamp = time_getTimeMus();
	}

	/*
	 * Note: this will only obtain the 1st-level JSON attributes
	 */

	for(ii = 0; ii < object->elementLength; ii++) {
		if((object->elements[ii].name == NULL) ||
				(!strcasecmp(object->elements[ii].name,
							 "containerMetaData")) ||
				(!strcasecmp(object->elements[ii].name,
							 "uid")) ||
				(!strcasecmp(object->elements[ii].name,
							 SEARCHD_DOMAIN_KEY)) ||
				(!strcasecmp(object->elements[ii].name,
							 SEARCHD_CONTAINER_UID)) ||
				(!strcasecmp(object->elements[ii].name,
							 SEARCHD_CONTAINER_NAME))) {
			continue;
		}

		switch(object->elements[ii].type) {
			case JSON_VALUE_TYPE_BOOLEAN:
				container_putBoolean(result,
						object->elements[ii].name,
						object->elements[ii].nameLength,
						json_getBoolean(object, object->elements[ii].name));
				break;

			case JSON_VALUE_TYPE_NUMBER:
				dValue = json_getNumber(object, object->elements[ii].name);

				if((dValue == (double)((int)dValue)) &&
						(dValue < MAX_SIGNED_INT)) {
					container_putInteger(result,
							object->elements[ii].name,
							object->elements[ii].nameLength,
							(int)dValue);
				}
				else {
					container_putDouble(result,
							object->elements[ii].name,
							object->elements[ii].nameLength,
							dValue);
				}
				break;

			case JSON_VALUE_TYPE_STRING:
				sValue = json_getString(object, object->elements[ii].name);

				if((sValue != NULL) && ((sValueLength = strlen(sValue)) > 0)) {
					container_putString(result,
							object->elements[ii].name,
							object->elements[ii].nameLength,
							sValue,
							sValueLength);
				}
				break;

			case JSON_VALUE_TYPE_ARRAY:
				if((!strcasecmp(object->elements[ii].name, "geocoords")) &&
						((arrayLength = json_getArrayLength(object,
								object->elements[ii].name)) > 0)) {
					for(nn = 0; ((nn < arrayLength) && (nn < 1000)); nn++) {
						if((containerObject = json_getObjectFromArray(object,
								object->elements[ii].name, nn)) == NULL) {
							continue;
						}

						if((!json_elementExists(containerObject,
										"latitude")) ||
								(!json_elementExists(containerObject,
										"longitude"))) {
							continue;
						}

						dValue = json_getNumber(containerObject, "latitude");

						tempNameLength = snprintf(tempName,
								((int)sizeof(tempName) - 1),
								"latitude%03i", nn);

						container_putDouble(result,
								tempName,
								tempNameLength,
								dValue);

						dValue = json_getNumber(containerObject, "longitude");

						tempNameLength = snprintf(tempName,
								((int)sizeof(tempName) - 1),
								"longitude%03i", nn);

						container_putDouble(result,
								tempName,
								tempNameLength,
								dValue);
					}
				}
				else {
					container_putString(result,
							object->elements[ii].name,
							object->elements[ii].nameLength,
							"(type 'array' not supported)", 28);
				}
				break;

			case JSON_VALUE_TYPE_OBJECT:
				container_putString(result,
						object->elements[ii].name,
						object->elements[ii].nameLength,
						"(type 'object' not supported)", 29);
				break;

			case JSON_VALUE_TYPE_NULL:
				container_putString(result,
						object->elements[ii].name,
						object->elements[ii].nameLength,
						"(null)", 6);
				break;

			case JSON_VALUE_TYPE_UNKNOWN:
			default:
				container_putString(result,
						object->elements[ii].name,
						object->elements[ii].nameLength,
						"(unkonwn)", 9);
		}
	}

	return result;
}

Json *container_containerToJson(Container *container)
{
	Json *result = NULL;
	Json *containerObject = NULL;
	ContainerIterator iterator;

	if(container == NULL) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	if(spinlock_readLock(&container->lock) < 0) {
		return NULL;
	}

	result = json_new();

	containerObject = json_new();

	json_addNumber(containerObject, "uid", (double)container->uid);
	json_addNumber(containerObject, "timestamp", container->timestamp);
	json_addNumber(containerObject, "age",
			time_getElapsedMusInSeconds(container->timestamp));

	if(container->name != NULL) {
		json_addString(containerObject, "name", container->name);
	}

	json_addObject(result, "containerMetaData", containerObject);

	if(containerIterator_init(container, &iterator) < 0) {
		json_freePtr(result);
		return NULL;
	}

	while(containerIterator_next(&iterator) == CONTAINER_ITERATOR_OK) {
		if((strstr(iterator.entity.name, "relevancyIndex") != NULL) ||
				(strstr(iterator.entity.name, "searchd_domainKey") != NULL)) {
			continue;
		}

		switch(iterator.entity.type) {
			case CONTAINER_TYPE_BOOLEAN:
				json_addBoolean(result,
						iterator.entity.name,
						iterator.entity.bValue);
				break;

			case CONTAINER_TYPE_INTEGER:
				json_addNumber(result,
						iterator.entity.name,
						(double)iterator.entity.iValue);
				break;

			case CONTAINER_TYPE_DOUBLE:
				json_addNumber(result,
						iterator.entity.name,
						iterator.entity.dValue);
				break;

			case CONTAINER_TYPE_STRING:
				json_addString(result,
						iterator.entity.name,
						iterator.entity.sValue);
				break;

			case CONTAINER_TYPE_END_OF_LIST:
			default:
				json_addString(result,
						iterator.entity.name,
						"(unknown)");
		}
	}

	containerIterator_free(&iterator);

	spinlock_readUnlock(&container->lock);

	return result;
}

Container *container_combine(Container *oldContainer, Container *newContainer,
		char flags, char **errorMessage)
{
	aboolean exists = afalse;
	int rc = 0;

	Container *result = NULL;
	ContainerIterator iterator;

	if((oldContainer == NULL) || (newContainer == NULL) ||
			(errorMessage == NULL)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	*errorMessage = NULL;

	// construct result

	result = container_new();

	// assign meta data

	container_lock(oldContainer);
	container_lock(newContainer);

	if(newContainer->uid > 0) {
		result->uid = newContainer->uid;
	}
	else if(oldContainer->uid > 0) {
		result->uid = oldContainer->uid;
	}

	if(newContainer->name != NULL) {
		result->nameLength = newContainer->nameLength;
		result->name = strndup(newContainer->name, newContainer->nameLength);
	}
	else if(oldContainer->name != NULL) {
		result->nameLength = oldContainer->nameLength;
		result->name = strndup(oldContainer->name, oldContainer->nameLength);
	}

	container_unlock(oldContainer);
	container_unlock(newContainer);

	// iterate old container and construct result baseline

	if((flags & CONTAINER_FLAG_UPDATE) ||
			(flags & CONTAINER_FLAG_APPEND) ||
			(flags & CONTAINER_FLAG_UPPEND)) {
		if(containerIterator_init(oldContainer, &iterator) < 0) {
			container_freePtr(result);
			return NULL;
		}

		while((rc = containerIterator_next(&iterator)) ==
				CONTAINER_ITERATOR_OK) {
			if(iterator.entity.nameLength < 1) {
				continue;
			}

			if(container_exists(newContainer, iterator.entity.name,
						iterator.entity.nameLength)) {
				continue;
			}

			switch(iterator.entity.type) {
				case CONTAINER_TYPE_BOOLEAN:
					container_putBoolean(result, iterator.entity.name,
							iterator.entity.nameLength,
							iterator.entity.bValue);
					break;

				case CONTAINER_TYPE_INTEGER:
					container_putInteger(result, iterator.entity.name,
							iterator.entity.nameLength,
							iterator.entity.iValue);
					break;

				case CONTAINER_TYPE_DOUBLE:
					container_putDouble(result, iterator.entity.name,
							iterator.entity.nameLength,
							iterator.entity.dValue);
					break;

				case CONTAINER_TYPE_STRING:
					container_putString(result, iterator.entity.name,
							iterator.entity.nameLength,
							iterator.entity.sValue,
							iterator.entity.valueLength);
					break;

				case CONTAINER_TYPE_END_OF_LIST:
				default:
					continue;
			}
		}

		containerIterator_free(&iterator);
	}

	// iterate new container and copy to result

	if(containerIterator_init(newContainer, &iterator) < 0) {
		container_freePtr(result);
		return NULL;
	}

	while((rc = containerIterator_next(&iterator)) == CONTAINER_ITERATOR_OK) {
		if(iterator.entity.nameLength < 1) {
			continue;
		}

		if((!(flags & CONTAINER_FLAG_UPPEND)) &&
				(!(flags & CONTAINER_FLAG_REPLACE))) {
			exists = container_exists(oldContainer, iterator.entity.name,
					iterator.entity.nameLength);

			if((flags & CONTAINER_FLAG_UPDATE) && (!exists)) {
				*errorMessage = buildErrorMessage(flags,
						"attribute '%s' does not exist",
						iterator.entity.name);
				container_freePtr(result);
				result = NULL;
				break;
			}
			else if((flags & CONTAINER_FLAG_APPEND) && (exists)) {
				*errorMessage = buildErrorMessage(flags,
						"attribute '%s' already exists",
						iterator.entity.name);
				container_freePtr(result);
				result = NULL;
				break;
			}
		}

		switch(iterator.entity.type) {
			case CONTAINER_TYPE_BOOLEAN:
				container_putBoolean(result, iterator.entity.name,
						iterator.entity.nameLength,
						iterator.entity.bValue);
				break;

			case CONTAINER_TYPE_INTEGER:
				container_putInteger(result, iterator.entity.name,
						iterator.entity.nameLength,
						iterator.entity.iValue);
				break;

			case CONTAINER_TYPE_DOUBLE:
				container_putDouble(result, iterator.entity.name,
						iterator.entity.nameLength,
						iterator.entity.dValue);
				break;

			case CONTAINER_TYPE_STRING:
				container_putString(result, iterator.entity.name,
						iterator.entity.nameLength,
						iterator.entity.sValue, iterator.entity.valueLength);
				break;

			case CONTAINER_TYPE_END_OF_LIST:
			default:
				continue;
		}
	}

	containerIterator_free(&iterator);

	return result;
}

void container_display(void *stream, Container *container)
{
	int counter = 0;

	ContainerIterator iterator;

	if(container == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	if(stream == NULL) {
		stream = stdout;
	}

	if(spinlock_readLock(&container->lock) < 0) {
		return;
	}

	if(containerIterator_init(container, &iterator) < 0) {
		return;
	}

	printf("Container 0x%lx #%i\n", (aptrcast)container, container->uid);
	printf("\t block ref     : %i\n", container->blockRef);
	printf("\t block length  : %i\n", container->blockLength);
	printf("\t entity count  : %i\n", container->entityCount);
	printf("\t timestamp     : %0.6f seconds\n",
			time_getElapsedMusInSeconds(container->timestamp));
	printf("\t block ref     : %i\n", container->blockRef);
	printf("\t name          : '%s'\n", container->name);
	printf("\t name length   : %i\n", container->nameLength);
	printf("\t block         : 0x%lx\n", (aptrcast)container->block);

	while(containerIterator_next(&iterator) == CONTAINER_ITERATOR_OK) {
		if(strstr(iterator.entity.name, "relevancyIndex") != NULL) {
			continue;
		}

		switch(iterator.entity.type) {
			case CONTAINER_TYPE_BOOLEAN:
				printf("\t\t {BOOLEAN}   entity [%03i] : '%s' is %i\n",
						counter, iterator.entity.name,
						(int)iterator.entity.bValue);
				break;

			case CONTAINER_TYPE_INTEGER:
				printf("\t\t {INTEGER}   entity [%03i] : '%s' is %i\n",
						counter, iterator.entity.name,
						iterator.entity.iValue);
				break;

			case CONTAINER_TYPE_DOUBLE:
				printf("\t\t {DOUBLE}    entity [%03i] : '%s' is %0.6f\n",
						counter, iterator.entity.name,
						iterator.entity.dValue);
				break;

			case CONTAINER_TYPE_STRING:
				printf("\t\t {STRING}    entity [%03i] : '%s' is '%s'\n",
						counter, iterator.entity.name,
						iterator.entity.sValue);
				break;

			case CONTAINER_TYPE_END_OF_LIST:
			default:
				printf("\t\t {UNKNOWN}   entity [%03i] : '%s' is '%s'\n",
						counter, iterator.entity.name, "(unknown)");
		}

		counter++;
	}

	containerIterator_free(&iterator);

	spinlock_readUnlock(&container->lock);
}

