/*
 * fifostack.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard core library FIFO stack type library, header file.
 *
 * Written by Josh English.
 */

#if !defined(_CORE_LIBRARY_TYPES_FIFOSTACK_H)

#define _CORE_LIBRARY_TYPES_FIFOSTACK_H

#if !defined(_CORE_H) && !defined(_CORE_LIBRARY_H) && \
		!defined(_CORE_LIBRARY_TYPES_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _CORE_H || _CORE_LIBRARY_H || _CORE_LIBRARY_TYPES_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

// delcare fifo stack public data types

typedef void (*FifoStackFreeFunction)(void *memory);

typedef struct _FifoStackLink {
	void *next;
	void *item;
} FifoStackLink;

typedef struct _FifoStack {
	alint itemCount;
	FifoStackLink *rootLink;
	FifoStackLink *lastLink;
	FifoStackFreeFunction freeFunc;
} FifoStack;

// delcare fifo stack public functions

int fifostack_init(FifoStack *stack);

int fifostack_free(FifoStack *stack);

int fifostack_setFreeFunction(FifoStack *stack, FifoStackFreeFunction freeFunc);

int fifostack_isEmpty(FifoStack *stack, aboolean *isEmpty);

int fifostack_getItemCount(FifoStack *stack, alint *itemCount);

int fifostack_peek(FifoStack *stack, void **item);

int fifostack_pop(FifoStack *stack, void **item);

int fifostack_push(FifoStack *stack, void *item);

#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _CORE_LIBRARY_TYPES_FIFOSTACK_H

