/*
 * fifostack.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard core library FIFO stack type library.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#define _CORE_LIBRARY_TYPES_COMPONENT
#include "core/library/types/fifostack.h"

// declare fifo stack private functions

static void freeLink(FifoStack *stack, FifoStackLink *link);

static FifoStackLink *newLink(void *item);

static void freeLinkPtr(FifoStack *stack, FifoStackLink *link);


// define fifo stack private functions

static void freeLink(FifoStack *stack, FifoStackLink *link)
{
	if(link->item != NULL) {
		if(stack->freeFunc == NULL) {
			free(link->item);
		}
		else {
			stack->freeFunc(link->item);
		}
	}

	memset(link, 0, sizeof(FifoStackLink));
}

static FifoStackLink *newLink(void *item)
{
	FifoStackLink *result = NULL;

	result = (FifoStackLink *)malloc(sizeof(FifoStackLink));

	result->next = NULL;
	result->item = item;

	return result;
}

static void freeLinkPtr(FifoStack *stack, FifoStackLink *link)
{
	freeLink(stack, link);
	free(link);
}


// define fifo stack public functions

int fifostack_init(FifoStack *stack)
{
	if(stack == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	memset(stack, 0, sizeof(FifoStack));

	stack->itemCount = 0;
	stack->rootLink = NULL;
	stack->lastLink = NULL;
	stack->freeFunc = NULL;

	return 0;
}

int fifostack_free(FifoStack *stack)
{
	alint linkCounter = 0;

	FifoStackLink *link = NULL;
	FifoStackLink *next = NULL;

	if(stack == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	for(link = stack->rootLink; link != NULL; link = next) {
		next = link->next;
		freeLinkPtr(stack, link);
		linkCounter++;
	}

	if(linkCounter != stack->itemCount) {
		return -1;
	}

	memset(stack, 0, sizeof(FifoStack));

	return 0;
}

int fifostack_setFreeFunction(FifoStack *stack, FifoStackFreeFunction freeFunc)
{
	if((stack == NULL) || (free == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	stack->freeFunc = freeFunc;

	return 0;
}

int fifostack_isEmpty(FifoStack *stack, aboolean *isEmpty)
{
	if((stack == NULL) || (isEmpty == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(stack->itemCount < 1) {
		*isEmpty = atrue;
	}
	else {
		*isEmpty = afalse;
	}

	return 0;
}

int fifostack_getItemCount(FifoStack *stack, alint *itemCount)
{
	if((stack == NULL) || (itemCount == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	*itemCount = stack->itemCount;

	return 0;
}

int fifostack_peek(FifoStack *stack, void **item)
{
	if((stack == NULL) || (item == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(stack->rootLink != NULL) {
		*item = stack->rootLink->item;
	}
	else {
		*item = NULL;
	}

	return 0;
}

int fifostack_pop(FifoStack *stack, void **item)
{
	FifoStackLink *link = NULL;

	if((stack == NULL) || (item == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(stack->rootLink != NULL) {
		*item = stack->rootLink->item;

		link = stack->rootLink;

		stack->rootLink = stack->rootLink->next;
		if(stack->rootLink == NULL) {
			stack->lastLink = NULL;
		}

		free(link);

		stack->itemCount -= 1;
	}
	else {
		*item = NULL;
	}

	return 0;
}

int fifostack_push(FifoStack *stack, void *item)
{
	FifoStackLink *link = NULL;

	if((stack == NULL) || (item == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	link = newLink(item);

	if(stack->rootLink == NULL) {
		stack->rootLink = link;
	}
	else {
		stack->lastLink->next = link;
	}

	stack->lastLink = link;

	stack->itemCount += 1;

	return 0;
}

