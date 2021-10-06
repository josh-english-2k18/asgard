/*
 * container.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * A library component for the search-engine to contain key-value attributes
 * in a highly-memory-optmized serial buffer, header file.
 *
 * Written by Josh English.
 */

#if !defined(_SEARCH_CORE_CONTAINER_H)

#define _SEARCH_CORE_CONTAINER_H

#if !defined(_SEARCH_H) && !defined(_SEARCH_CORE_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _SEARCH_H || _SEARCH_CORE_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// define container public constants

typedef enum _ContainerValueTypes {
	CONTAINER_TYPE_BOOLEAN = 1,
	CONTAINER_TYPE_INTEGER,
	CONTAINER_TYPE_DOUBLE,
	CONTAINER_TYPE_STRING,
	CONTAINER_TYPE_END_OF_LIST = -1
} ContainerValueTypes;

typedef enum _ContainerIteratorResults {
	CONTAINER_ITERATOR_OK = 1,
	CONTAINER_ITERATOR_END_OF_LIST,
	CONTAINER_ITERATOR_ERROR = -1
} ContainerIteratorResults;

#define CONTAINER_REFRESH_TIMESTAMP_THRESHOLD_SECONDS		1200 // 20 minutes

#define CONTAINER_HASH_MAP_ELEMENTS							8

#define CONTAINER_MEMORY_BLOCK_LENGTH						128

#define CONTAINER_FLAG_UPDATE								0x00000001
#define CONTAINER_FLAG_APPEND								0x00000002
#define CONTAINER_FLAG_UPPEND								0x00000004
#define CONTAINER_FLAG_REPLACE								0x00000008


// define container public data types

typedef struct _ContainerIndex {
	ContainerValueTypes type;
	int blockRef;
	int blockLength;
	int nameLength;
	char *name;
	void *next;
} ContainerIndex;

typedef struct _Container {
	int uid;
	int blockRef;
	int blockLength;
	int entityCount;
	int nameLength;
	double timestamp;
	char *name;
	char *block;
	ContainerIndex *mapRoot[CONTAINER_HASH_MAP_ELEMENTS];
	ContainerIndex *mapEnd[CONTAINER_HASH_MAP_ELEMENTS];
	Spinlock lock;
} Container;

typedef struct _ContainerIteratorEntity {
	ContainerValueTypes type;
	aboolean bValue;
	int iValue;
	double dValue;
	int nameLength;
	int valueLength;
	char *name;
	char *sValue;
} ContainerIteratorEntity;

typedef struct _ContainerIterator {
	int mapReference;
	ContainerIteratorEntity entity;
	ContainerIndex *index;
	Container *container;
	Mutex mutex;
} ContainerIterator;


// define container public functions

// container functions

int container_init(Container *container);

int container_initWithName(Container *container, char *name, int nameLength);

Container *container_new();

Container *container_newWithName(char *name, int nameLength);

int container_free(Container *container);

int container_freePtr(Container *container);

char *container_typeToString(ContainerValueTypes type);

char *container_iteratorToString(ContainerIteratorResults result);

char *container_flagToString(char flag);

aboolean container_isLocked(Container *container);

aboolean container_isReadLocked(Container *container);

int container_lock(Container *container);

int container_unlock(Container *container);

int container_getUid(Container *container);

int container_setUid(Container *container, int uid);

char *container_getName(Container *container, int *nameLength);

int container_setName(Container *container, char *name, int nameLength);

int container_peg(Container *container);

double container_getAgeInSeconds(Container *container);

aboolean container_exists(Container *container, char *name, int nameLength);

ContainerValueTypes container_getType(Container *container, char *name,
		int nameLength);

int container_putBoolean(Container *container, char *name, int nameLength,
		aboolean value);

aboolean container_getBoolean(Container *container, char *name, int nameLength);

int container_putInteger(Container *container, char *name, int nameLength,
		int value);

int container_getInteger(Container *container, char *name, int nameLength);

int container_putDouble(Container *container, char *name, int nameLength,
		double value);

double container_getDouble(Container *container, char *name, int nameLength);

int container_putString(Container *container, char *name, int nameLength,
		char *value, int length);

char *container_getString(Container *container, char *name, int nameLength,
		int *length);

int container_calculateMemoryLength(Container *container);

char *container_serialize(Container *container, int *length);

Container *container_deserialize(char *container, int length);

// iterator functions

int containerIterator_init(Container *container, ContainerIterator *iterator);

ContainerIterator *containerIterator_new(Container *container);

int containerIterator_free(ContainerIterator *iterator);

int containerIterator_freePtr(ContainerIterator *iterator);

int containerIterator_next(ContainerIterator *iterator);

int containerIterator_rewind(ContainerIterator *iterator);

// helper functions

Container *container_jsonToContainer(Json *object);

Json *container_containerToJson(Container *container);

Container *container_combine(Container *oldContainer, Container *newContainer,
		char flags, char **errorMessage);

void container_display(void *stream, Container *container);


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _SEARCH_CORE_CONTAINER_H

