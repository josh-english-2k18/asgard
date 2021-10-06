/*
 * arraylist.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * A generic array-list implementation modeled after Java's ArrayList, header
 * file.
 *
 * Written by Josh English.
 */

#if !defined(_CORE_LIBRARY_TYPES_ARRAYLIST_H)

#define _CORE_LIBRARY_TYPES_ARRAYLIST_H

#if !defined(_CORE_H) && !defined(_CORE_LIBRARY_H) && \
		!defined(_CORE_LIBRARY_TYPES_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _CORE_H || _CORE_LIBRARY_H || _CORE_LIBRARY_TYPES_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// define arraylist constants

#define ARRAYLIST_DEFAULT_LENGTH							8

#define ARRAYLIST_DEFAULT_LIST_INCREMENT					8


// define arraylist data types

typedef void (*ArrayListFreeFunction)(void *memory);

typedef struct _ArrayList {
	int length;
	int position;
	int currentIncrement;
	void **list;
	ArrayListFreeFunction free;
} ArrayList;


// declare arraylist public functions

void arraylist_init(ArrayList *list);

void arraylist_free(ArrayList *list);

void arraylist_setFreeFunction(ArrayList *list, ArrayListFreeFunction function);

void arraylist_ensureCurrentCapacity(ArrayList *list, int length);

void arraylist_trimCurrentCapacity(ArrayList *list, int length);

int arraylist_add(ArrayList *list, void *entry);

int arraylist_addFirst(ArrayList *list, void *entry);

int arraylist_addPosition(ArrayList *list, int position, void *entry);

void arraylist_clear(ArrayList *list);

aboolean arraylist_containsEntry(ArrayList *list, void *entry);

int arraylist_positionOfEntry(ArrayList *list, void *entry);

void *arraylist_get(ArrayList *list, int position);

int arraylist_del(ArrayList *list, void *entry);

int arraylist_delPosition(ArrayList *list, int position);

int arraylist_delRange(ArrayList *list, int begin, int end);

int arraylist_set(ArrayList *list, int position, void *entry);

int arraylist_length(ArrayList *list);

void **arraylist_array(ArrayList *list, int *length);

int arraylist_pushFifo(ArrayList *list, void *entry);

int arraylist_pushFilo(ArrayList *list, void *entry);

void *arraylist_pop(ArrayList *list);

void *arraylist_popLast(ArrayList *list);


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _CORE_LIBRARY_TYPES_ARRAYLIST_H

