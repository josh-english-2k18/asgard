/*
 * arraylist.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * A generic array-list implementation modeled after Java's ArrayList.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#define _CORE_LIBRARY_TYPES_COMPONENT
#include "core/library/types/arraylist.h"


// declare arraylist private functions

static void defaultFreeFunction(void *memory);

static void increaseArrayLength(ArrayList *list, int position);

static void removeArrayListEntry(ArrayList *list, int begin, int end);


// define arraylist private functions

static void defaultFreeFunction(void *memory)
{
	if(memory == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	free(memory);
}

static void increaseArrayLength(ArrayList *list, int position)
{
	int ii = 0;
	int length = 0;

	if(position >= list->length) {
		while(position >= (list->length + length)) {
			length += list->currentIncrement;
		}

		if(length > list->currentIncrement) {
			list->currentIncrement *= 2;
		}

		list->length += length;

		list->list = (void *)realloc(list->list,
				(sizeof(void *) * list->length));

		for(ii = list->position; ii < list->length; ii++) {
			list->list[ii] = NULL;
		}
	}
}

static void removeArrayListEntry(ArrayList *list, int begin, int end)
{
	int ii = 0;
	int nn = 0;

	for(ii = 0, nn = 0; ii < list->position; ii++) {
		if((ii >= begin) && (ii <= end)) {
			if(list->list[ii] != NULL) {
				if(list->free != NULL) {
					(*list->free)(list->list[ii]);
				}
				list->list[ii] = NULL;
			}
		}
		else {
			list->list[nn] = list->list[ii];
			nn++;
		}
	}

	list->position = nn;
}


// define arraylist public functions

void arraylist_init(ArrayList *list)
{
	if(list == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	memset(list, 0, (sizeof(ArrayList)));

	list->length = ARRAYLIST_DEFAULT_LENGTH;
	list->position = 0;
	list->currentIncrement = ARRAYLIST_DEFAULT_LIST_INCREMENT;
	list->list = (void *)malloc(sizeof(void *) * list->length);
	list->free = defaultFreeFunction;
}

void arraylist_free(ArrayList *list)
{
	int ii = 0;

	if(list == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	if(list->list != NULL) {
		for(ii = 0; ii < list->position; ii++) {
			if(list->list[ii] != NULL) {
				if(list->free != NULL) {
					(*list->free)(list->list[ii]);
				}
			}
		}
		free(list->list);
	}

	memset(list, 0, (sizeof(ArrayList)));
}

void arraylist_setFreeFunction(ArrayList *list, ArrayListFreeFunction function)
{
	if((list == NULL) || (function == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	list->free = function;
}

void arraylist_ensureCurrentCapacity(ArrayList *list, int length)
{
	int ii = 0;

	if((list == NULL) || (length < 1)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	if(length > list->length) {
		list->length = length;
		list->list = (void *)realloc(list->list,
				(sizeof(void *) * list->length));
		for(ii = list->position; ii < list->length; ii++) {
			list->list[ii] = NULL;
		}
	}
}

void arraylist_trimCurrentCapacity(ArrayList *list, int length)
{
	int ii = 0;

	if((list == NULL) || (length < 1)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	if(length > list->position) {
		list->length = length;
		list->list = (void *)realloc(list->list,
				(sizeof(void *) * list->length));
		for(ii = list->position; ii < list->length; ii++) {
			list->list[ii] = NULL;
		}
	}
}

int arraylist_add(ArrayList *list, void *entry)
{
	int position = 0;

	if((list == NULL) || (entry == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	position = list->position;

	list->position += 1;

	increaseArrayLength(list, list->position);

	list->list[position] = entry;

	return 0;
}

int arraylist_addFirst(ArrayList *list, void *entry)
{
	int ii = 0;
	int nn = 0;

	if((list == NULL) || (entry == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	list->position += 1;

	increaseArrayLength(list, list->position);

	for(ii = (list->position - 2), nn = (list->position - 1);
			ii >= 0;
			ii--, nn--) {
		list->list[nn] = list->list[ii];
	}

	list->list[0] = entry;

	return 0;
}

int arraylist_addPosition(ArrayList *list, int position, void *entry)
{
	int ii = 0;
	int nn = 0;

	if((list == NULL) || (position < 0) || (entry == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(position > list->position) {
		fprintf(stderr, "[%s():%i] error - position %i is out-of-bounds "
				"(length %i).\n", __FUNCTION__, __LINE__, position,
				list->position);
		return -1;
	}

	list->position += 1;

	increaseArrayLength(list, list->position);

	for(ii = 0, nn = 0; ii < list->position; ii++) {
		if(ii == position) {
			list->list[nn] = entry;
			nn++;
		}
		list->list[nn] = list->list[ii];
		nn++;
	}

	return 0;
}

void arraylist_clear(ArrayList *list)
{
	int ii = 0;

	if(list == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	for(ii = 0; ii < list->length; ii++) {
		if(ii < list->position) {
			if(list->list[ii] != NULL) {
				if(list->free != NULL) {
					(*list->free)(list->list[ii]);
				}
			}
		}
		list->list[ii] = NULL;
	}

	list->position = 0;
}

aboolean arraylist_containsEntry(ArrayList *list, void *entry)
{
	aboolean result = afalse;
	int ii = 0;

	if((list == NULL) || (entry == NULL)) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	for(ii = 0; ii < list->position; ii++) {
		if(list->list[ii] == entry) {
			result = atrue;
			break;
		}
	}

	return result;
}

int arraylist_positionOfEntry(ArrayList *list, void *entry)
{
	int ii = 0;
	int result = -1;

	if((list == NULL) || (entry == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	for(ii = 0; ii < list->position; ii++) {
		if(list->list[ii] == entry) {
			result = ii;
			break;
		}
	}

	return result;
}

void *arraylist_get(ArrayList *list, int position)
{
	void *result = NULL;

	if((list == NULL) || (position < 0)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	if(position < list->position) {
		result = list->list[position];
	}

	return result;
}

int arraylist_del(ArrayList *list, void *entry)
{
	int ii = 0;
	int position = 0;

	if((list == NULL) || (entry == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	position = -1;

	for(ii = 0; ii < list->position; ii++) {
		if(list->list[ii] == entry) {
			position = ii;
			break;
		}
	}

	if(position == -1) {
		return -1;
	}

	removeArrayListEntry(list, position, position);

	return 0;
}

int arraylist_delPosition(ArrayList *list, int position)
{
	if((list == NULL) || (position < 0) || (position >= list->position)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	removeArrayListEntry(list, position, position);

	return 0;
}

int arraylist_delRange(ArrayList *list, int begin, int end)
{
	if((list == NULL) || (begin < 0) || (begin > end) ||
			(end > list->position)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	removeArrayListEntry(list, begin, end);

	return 0;
}

int arraylist_set(ArrayList *list, int position, void *entry)
{
	if((list == NULL) || (position < 0) || (entry == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(position >= list->position) {
		fprintf(stderr, "[%s():%i] error - position %i is out-of-bounds "
				"(length %i).\n", __FUNCTION__, __LINE__, position,
				list->position);
		return -1;
	}

	if((list->list[position] != NULL) && (list->free != NULL)) {
		(*list->free)(list->list[position]);
	}

	list->list[position] = entry;

	return 0;
}

int arraylist_length(ArrayList *list)
{
	if(list == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return list->position;
}

void **arraylist_array(ArrayList *list, int *length)
{
	void **result = NULL;

	if((list == NULL) || (length == NULL)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	*length = list->position;
	result = list->list;

	return result;
}

int arraylist_pushFifo(ArrayList *list, void *entry)
{
	if((list == NULL) || (entry == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return arraylist_addFirst(list, entry);
}

int arraylist_pushFilo(ArrayList *list, void *entry)
{
	if((list == NULL) || (entry == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return arraylist_add(list, entry);
}

void *arraylist_pop(ArrayList *list)
{
	int ii = 0;
	int nn = 0;
	void *result = NULL;

	if(list == NULL) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	if(list->position > 0) {
		result = list->list[0];

		for(ii = 1, nn = 0; ii < list->position; ii++) {
			list->list[nn] = list->list[ii];
			nn++;
		}

		list->position = nn;
	}

	return result;
}

void *arraylist_popLast(ArrayList *list)
{
	void *result = NULL;

	if(list == NULL) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	if(list->position > 0) {
		result = list->list[(list->position - 1)];
		list->list[(list->position - 1)] = NULL;
		list->position -= 1;
	}

	return result;
}

