/*
 * bptree.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard core library B+Tree type library, header file.
 *
 * Written by Josh English.
 */

#if !defined(_CORE_LIBRARY_TYPES_BPTREE_H)

#define _CORE_LIBRARY_TYPES_BPTREE_H

#if !defined(_CORE_H) && !defined(_CORE_LIBRARY_H) && \
		!defined(_CORE_LIBRARY_TYPES_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _CORE_H || _CORE_LIBRARY_H || _CORE_LIBRARY_TYPES_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// define b+tree public constants

#define BPTREE_NODE_ELEMENTS							8


// declare b+tree public data types

typedef void (*BptreeFreeFunction)(void *memory);

typedef void (*BptreeComplexFreeFunction)(void *argument, void *memory);

typedef int (*BptreeCompareKeysFunction)(char *keyAlpha, int keyAlphaLength,
		char *keyBeta, int keyBetaLength);

typedef struct _BptreeLink {
	char *key;
	int keyLength;
	void *value;
	void *next;
} BptreeLink;

typedef struct _BptreeNode {
	int length;
	BptreeLink links[BPTREE_NODE_ELEMENTS];
} BptreeNode;

typedef struct _Bptree {
	aboolean debugMode;
	aboolean isComplexFree;
	int treeDepth;
	int leafCount;
	int depthLength;
	int *depthCounts;
	void *complexFreeArgument;
	BptreeNode *root;
	BptreeFreeFunction freeFunc;
	BptreeComplexFreeFunction complexFreeFunc;
	BptreeCompareKeysFunction compareFunc;
} Bptree;


// declare b+tree public functions

int bptree_init(Bptree *tree);

int bptree_free(Bptree *tree);

int bptree_setFreeFunction(Bptree *tree, BptreeFreeFunction freeFunction);

int bptree_setComplexFreeFunction(Bptree *tree, void *argument,
		BptreeComplexFreeFunction complexFreeFunction);

int bptree_setCompareKeysFunction(Bptree *tree,
		BptreeCompareKeysFunction compareFunc);

int bptree_setDebugMode(Bptree *tree, aboolean debugMode);

int bptree_getLeafCount(Bptree *tree, int *leafCount);

int bptree_getTreeDepth(Bptree *tree, int *treeDepth);

int bptree_clear(Bptree *tree);

int bptree_get(Bptree *tree, char *key, int keyLength, void **value);

int bptree_getNext(Bptree *tree, char *key, int keyLength, void **value);

int bptree_getPrevious(Bptree *tree, char *key, int keyLength, void **value);

int bptree_put(Bptree *tree, char *key, int keyLength, void *value);

int bptree_remove(Bptree *tree, char *key, int keyLength);

int bptree_toArray(Bptree *tree, int *arrayLength, int **keyLengths,
		char ***keys, void ***values);

int bptree_performInternalTest(Bptree *tree);


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _CORE_LIBRARY_TYPES_BPTREE_H

