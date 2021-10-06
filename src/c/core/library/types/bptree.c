/*
 * bptree.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard core library B+Tree type library.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#define _CORE_LIBRARY_TYPES_COMPONENT
#include "core/library/types/bptree.h"

// define b+tree private constants

#define SEARCH_DIRECTION_NEXT						1
#define SEARCH_DIRECTION_PREVIOUS					0


// declare b+tree private functions

// general functions

static char *copyKey(char *key, int keyLength);

static void bptreeInternalComplexFree(void *argument, void *memory);

static int bptreeInternalCompareKeys(char *keyAlpha, int keyAlphaLength,
		char *keyBeta, int keyBetaLength);

static void displayNode(Bptree *tree, const char *function, BptreeNode *node,
		int keyLength, int count);

// tree-node functions

static BptreeNode *newNode();

static BptreeNode *splitNode(BptreeNode *node);

static int validateNode(BptreeNode *node, int currentDepth);

// tree-depth functions

static void increaseTreeDepthMemory(Bptree *tree, int currentDepth);

static void subtractFromTreeDepth(Bptree *tree, int currentDepth);

static int calculateTreeDepth(Bptree *tree);

// tree functions

static void freeTree(Bptree *tree, BptreeNode *node, int currentDepth);

static BptreeNode *insertLeaf(Bptree *tree, BptreeNode *node, char *key,
		int keyLength, void *value, int currentDepth);

static aboolean searchTree(Bptree *tree, BptreeNode *node, char *key,
		int keyLength, int currentDepth, void **value);

static char *directionalTreeSearch(Bptree *tree, BptreeNode *node, char *key,
		int keyLength, int currentDepth, int direction);

static int exciseNode(Bptree *tree, BptreeNode *node, char *key,
		int keyLength, int currentDepth, int mode);

static int removeNode(Bptree *tree, BptreeNode *node, char *key,
		int keyLength, int currentDepth, int mode);

static void treeToArray(Bptree *tree, BptreeNode *node, int currentDepth,
		int *listRef, int *keyListLengths, char **keyList, void **valueList);

static int validateTree(Bptree *tree, BptreeNode *node, int count);


// define b+tree private functions

// general functions

static char *copyKey(char *key, int keyLength)
{
	char *result = NULL;

	if((key == NULL) || (keyLength < 1)) {
		DISPLAY_INVALID_ARGS;
		exit(1);
	}

	result = (char *)malloc(sizeof(char) * (keyLength + 1));
	memcpy(result, key, keyLength);

	return result;
}

static void bptreeInternalComplexFree(void *argument, void *memory)
{
	free(memory);
}

static int bptreeInternalCompareKeys(char *keyAlpha, int keyAlphaLength,
		char *keyBeta, int keyBetaLength)
{
	int ii = 0;

	if(keyAlpha == NULL) {
		if(keyBeta == NULL) {
			return 0;
		}
		return -1;
	}
	else if(keyBeta == NULL) {
		return 1;
	}

	for(ii = 0; ((ii < keyAlphaLength) && (ii < keyBetaLength)); ii++) {
		if((unsigned char)keyAlpha[ii] < (unsigned char)keyBeta[ii]) {
			return -1;
		}
		else if((unsigned char)keyAlpha[ii] > (unsigned char)keyBeta[ii]) {
			return 1;
		}
	}

	if(keyAlphaLength < keyBetaLength) {
		return -1;
	}
	else if(keyAlphaLength > keyBetaLength) {
		return 1;
	}

	return 0;
}

static void displayNode(Bptree *tree, const char *function, BptreeNode *node,
		int keyLength, int count)
{
	int ii = 0;
	int nn = 0;

	if((tree == NULL) || (function == NULL) || (node == NULL) || (count < 0)) {
		DISPLAY_INVALID_ARGS;
		exit(1);
	}

	printf("[debug] %s(), key size: %i, count: %i, order: %i, length: %i\n",
			function, keyLength, count, tree->treeDepth, node->length);
	for(ii = 0; ii < node->length; ii++) {
		printf("[debug] ");
		for(nn = 0; nn < count; nn++) {
			printf("-");
		}
//		printf(">%10i", node->links[ii].keyLength);
		printf(">(%2i/%2i):%i -> '", ii, node->length,
				node->links[ii].keyLength);
		for(nn = 0; ((nn < 45) && (nn < node->links[ii].keyLength)); nn++) {
			if(((int)node->links[ii].key[nn] < 32) ||
					((int)node->links[ii].key[nn] > 126)) {
				printf("(0x%x)", (unsigned char)node->links[ii].key[nn]);
			}
			else {
				printf("%c", node->links[ii].key[nn]);
			}
		}
		printf("', value: ");
		if(node->links[ii].value != NULL) {
			printf("0x%lx", (aptrcast)node->links[ii].value);
		}
		else {
			printf("NULL");
		}
		printf(", link: ");
		if(node->links[ii].next != NULL) {
			printf("0x%lx", (aptrcast)node->links[ii].next);
		}
		else {
			printf("NULL");
		}
		printf("\n");
	}

/*	printf("[debug]   tree depth: %i\n", tree->depthLength);
	for(ii = 0; ii < tree->depthLength; ii++) {
		printf("[debug]     %i: %i\n", ii, tree->depthCounts[ii]);
	}*/
}

// tree-node functions

static BptreeNode *newNode()
{
	int ii = 0;

	BptreeNode *node = NULL;

	node = (BptreeNode *)malloc(sizeof(BptreeNode));

	node->length = 0;
	for(ii = 0; ii < BPTREE_NODE_ELEMENTS; ii++) {
		node->links[ii].key = NULL;
		node->links[ii].keyLength = 0;
		node->links[ii].value = NULL;
		node->links[ii].next = NULL;
	}

	return node;
}

static BptreeNode *splitNode(BptreeNode *node)
{
	int ii = 0;
	int nodeLength = 0;

	BptreeNode *localNode = NULL;

	if(node == NULL) {
		DISPLAY_INVALID_ARGS;
		exit(1);
	}

	nodeLength = (BPTREE_NODE_ELEMENTS / 2);
	localNode = newNode();

	for(ii = 0; ii < BPTREE_NODE_ELEMENTS; ii++) {
		if(ii < nodeLength) {
			localNode->links[ii].key = node->links[(nodeLength + ii)].key;
			localNode->links[ii].keyLength =
				node->links[(nodeLength + ii)].keyLength;
			localNode->links[ii].value = node->links[(nodeLength + ii)].value;
			localNode->links[ii].next = node->links[(nodeLength + ii)].next;
		}
		else {
			localNode->links[ii].key = NULL;
			localNode->links[ii].keyLength = 0;
			localNode->links[ii].value = NULL;
			localNode->links[ii].next = NULL;
			node->links[ii].key = NULL;
			node->links[ii].keyLength = 0;
			node->links[ii].value = NULL;
			node->links[ii].next = NULL;
		}
	}

	node->length = nodeLength;
	localNode->length = nodeLength;

	return localNode;
}

static int validateNode(BptreeNode *node, int currentDepth)
{
	int ii = 0;

	if(node == NULL) {
		fprintf(stderr, "[%s():%i] error - node is NULL.\n", __FUNCTION__,
				__LINE__);
		return -1;
	}

	if(node->length < 1) {
		fprintf(stderr, "[%s():%i] error - detected node 0x%lx length of %i.\n",
				__FUNCTION__, __LINE__, (aptrcast)node, node->length);
		return -2;
	}

	for(ii = 0; ii < node->length; ii++) {
		if(node->links[ii].key == NULL) {
			fprintf(stderr, "[%s():%i] error - detected node 0x%lx has NULL "
					"key for link %i.\n", __FUNCTION__, __LINE__,
					(aptrcast)node, ii);
			return -3;
		}
		if(node->links[ii].keyLength < 1) {
			fprintf(stderr, "[%s():%i] error - detected node 0x%lx has key "
					"length of %i for link %i.\n", __FUNCTION__, __LINE__,
					(aptrcast)node, node->links[ii].keyLength, ii);
			return -4;
		}
		if((currentDepth == 0) && (node->links[ii].next != NULL)) {
			fprintf(stderr, "[%s():%i] error - detected node 0x%lx has NULL "
					"value for link %i.\n", __FUNCTION__, __LINE__,
					(aptrcast)node, ii);
			return -5;
		}
	}

	return 0;
}

// tree-depth functions

static void increaseTreeDepthMemory(Bptree *tree, int currentDepth)
{
	int ref = 0;

	if((tree == NULL) || (currentDepth < 0)) {
		DISPLAY_INVALID_ARGS;
		exit(1);
	}

	ref = tree->depthLength;
	tree->depthLength = (currentDepth + 1);
	tree->depthCounts = (int *)realloc(tree->depthCounts,
			(sizeof(int) * tree->depthLength));

	tree->depthCounts[ref] = 0;
}

static void addToTreeDepth(Bptree *tree, int currentDepth)
{
	if((tree == NULL) || (currentDepth < 0)) {
		DISPLAY_INVALID_ARGS;
		exit(1);
	}

	if(currentDepth >= tree->depthLength) {
		increaseTreeDepthMemory(tree, currentDepth);
	}

	tree->depthCounts[currentDepth] += 1;

	if(tree->debugMode) {
		printf("[debug] tree depth(%i)++\n", currentDepth);
	}
}

static void subtractFromTreeDepth(Bptree *tree, int currentDepth)
{
	if((tree == NULL) || (currentDepth < 0)) {
		DISPLAY_INVALID_ARGS;
		exit(1);
	}

	if(currentDepth >= tree->depthLength) {
		printf("error - 'sub' called on non-existant tree depth.\n");
		exit(1);
	}

	tree->depthCounts[currentDepth] -= 1;

	if(tree->debugMode) {
		printf("[debug] tree depth(%i)--\n", currentDepth);
	}
}

static int calculateTreeDepth(Bptree *tree)
{
	int ii = 0;

	if(tree == NULL) {
		DISPLAY_INVALID_ARGS;
		exit(1);
	}

	for(ii = 1; ii < tree->depthLength; ii++) {
		if(tree->depthCounts[ii] < 1) {
			if(tree->depthCounts[ii] < 0) {
				printf("error - tree depth is less than zero.\n");
				exit(1);
			}
			return (ii - 1);
		}
	}

	return (tree->depthLength - 1);
}

// tree functions

static void freeTree(Bptree *tree, BptreeNode *node, int currentDepth)
{
	int ii = 0;

	if((tree == NULL) || (node == NULL) || (currentDepth < 0)) {
		DISPLAY_INVALID_ARGS;
		exit(1);
	}

	if(tree->debugMode) {
		displayNode(tree, __FUNCTION__, node, -1, currentDepth);
	}

	for(ii = 0; ii < node->length; ii++) {
		free(node->links[ii].key);
		if(currentDepth == 0) {
			if(node->links[ii].value != NULL) {
				if(tree->isComplexFree) {
					tree->complexFreeFunc(tree->complexFreeArgument,
							node->links[ii].value);
				}
				else if(tree->freeFunc ==
						(BptreeFreeFunction)MEMORY_FREE_FUNCTION) {
					MEMORY_FREE_FUNCTION(node->links[ii].value, __FILE__,
							__FUNCTION__, __LINE__);
				}
				else {
					tree->freeFunc(node->links[ii].value);
				}
			}
		}

		if(currentDepth > 0) {
			freeTree(tree, (BptreeNode *)node->links[ii].next,
					(currentDepth - 1));
		}
	}

	free(node);
}

static BptreeNode *insertLeaf(Bptree *tree, BptreeNode *node, char *key,
		int keyLength, void *value, int currentDepth)
{
	int ii = 0;
	int nn = 0;

	BptreeLink link;
	BptreeNode *localNode = NULL;

	if((tree == NULL) || (node == NULL) || (key == NULL) || (keyLength < 0) ||
			(currentDepth < 0)) {
		DISPLAY_INVALID_ARGS;
		exit(1);
	}

	if(tree->debugMode) {
		displayNode(tree, __FUNCTION__, node, keyLength, currentDepth);
	}

	memset((void *)&link, 0, sizeof(BptreeLink));

	link.key = copyKey(key, keyLength);
	link.keyLength = keyLength;
	link.value = value;
	link.next = NULL;

	if(currentDepth == 0) {
		for(ii = 0; ii < node->length; ii++) {
			if(tree->compareFunc(key, keyLength, node->links[ii].key,
						node->links[ii].keyLength) == -1) {
				break;
			}
		}
	}
	else if(currentDepth > 0) {
		for(ii = 0; ii < node->length; ii++) {
			if(((ii + 1) == node->length) ||
					(tree->compareFunc(key, keyLength,
							node->links[(ii + 1)].key,
							node->links[(ii + 1)].keyLength) == -1)) {
				localNode = (BptreeNode *)insertLeaf(tree, node->links[ii].next,
						key, keyLength, value, (currentDepth - 1));
				free(link.key);
				if(localNode == NULL) {
					return NULL;
				}

				link.key = copyKey(localNode->links[0].key,
						localNode->links[0].keyLength);
				link.keyLength = localNode->links[0].keyLength;
				link.value = NULL;
				link.next = localNode;
				ii++;

				break;
			}
		}
	}

	addToTreeDepth(tree, currentDepth);

	for(nn = node->length; nn > ii; nn--) {
		node->links[nn] = node->links[(nn - 1)];
	}

	node->length++;
	node->links[ii].key = link.key;
	node->links[ii].keyLength = link.keyLength;
	node->links[ii].value = link.value;
	node->links[ii].next = link.next;

	if(node->length < BPTREE_NODE_ELEMENTS) {
		return NULL;
	}

	if((currentDepth + 1) >= tree->depthLength) {
		increaseTreeDepthMemory(tree, currentDepth + 1);
	}

	if(tree->depthCounts[(currentDepth + 1)] == 0) {
		addToTreeDepth(tree, currentDepth + 1);
		addToTreeDepth(tree, currentDepth + 1);
	}

	return splitNode(node);
}

static aboolean searchTree(Bptree *tree, BptreeNode *node, char *key,
		int keyLength, int currentDepth, void **value)
{
	int ii = 0;

	if((tree == NULL) || (node == NULL) || (key == NULL) || (keyLength < 0) ||
			(currentDepth < 0)) {
		DISPLAY_INVALID_ARGS;
		exit(1);
	}

	if(tree->debugMode) {
		displayNode(tree, __FUNCTION__, node, keyLength, currentDepth);
	}

	if(currentDepth == 0) {
		for(ii = 0; ii < node->length; ii++) {
			if(tree->compareFunc(key, keyLength, node->links[ii].key,
						node->links[ii].keyLength) == 0) {
				*value = node->links[ii].value;
				return atrue;
			}
		}
	}
	else if(currentDepth > 0) {
		for(ii = 0; ii < node->length; ii++) {
			if(((ii + 1) == node->length) ||
					(tree->compareFunc(key, keyLength,
							node->links[(ii + 1)].key,
							node->links[(ii + 1)].keyLength) == -1)) {
				return searchTree(tree, (BptreeNode *)node->links[ii].next,
						key, keyLength, (currentDepth - 1), value);
			}
		}
	}

	return afalse;
}

static char *directionalTreeSearch(Bptree *tree, BptreeNode *node, char *key,
		int keyLength, int currentDepth, int direction)
{
	int ii = 0;
	char *result = NULL;

	if((tree == NULL) || (node == NULL) || (key == NULL) || (keyLength < 0) ||
			(currentDepth < 0) ||
			((direction != SEARCH_DIRECTION_NEXT) &&
			 (direction != SEARCH_DIRECTION_PREVIOUS))) {
		DISPLAY_INVALID_ARGS;
		exit(1);
	}

	if(tree->debugMode) {
		displayNode(tree, __FUNCTION__, node, keyLength, currentDepth);
	}

	if(currentDepth == 0) {
		if(direction == SEARCH_DIRECTION_NEXT) {
			for(ii = 0; ii < node->length; ii++) {
				if(tree->compareFunc(node->links[ii].key,
							node->links[ii].keyLength, key, keyLength) == 1) {
					return node->links[ii].value;
				}
			}
		}
		else if(direction == SEARCH_DIRECTION_PREVIOUS) {
			for(ii = (node->length - 1); ii > -1; ii--) {
				if(tree->compareFunc(node->links[ii].key,
							node->links[ii].keyLength, key,
							keyLength) == -1) {
					return node->links[ii].value;
				}
			}
		}
	}
	else if(currentDepth > 0) {
		for(ii = 0; ii < node->length; ii++) {
			if(((ii + 1) == node->length) ||
					(tree->compareFunc(key, keyLength,
							node->links[(ii + 1)].key,
							node->links[(ii + 1)].keyLength) == -1)) {
				while((result = directionalTreeSearch(tree,
								node->links[ii].next, key, keyLength,
								(currentDepth - 1), direction)) == NULL) {
					if(direction == SEARCH_DIRECTION_NEXT) {
						ii++;
						if(ii >= node->length) {
							break;
						}
					}
					else if(direction == SEARCH_DIRECTION_PREVIOUS) {
						ii--;
						if(ii < 0) {
							break;
						}
					}
				}

				return result;
			}
		}
	}

	return NULL;
}

static int exciseNode(Bptree *tree, BptreeNode *node, char *key,
		int keyLength, int currentDepth, int mode)
{
	int ii = 0;
	int nn = 0;
	int jj = 0;
	int foundNode = 0;

	BptreeNode *localNode = NULL;

	if((tree == NULL) || (node == NULL) || (key == NULL) || (keyLength < 0) ||
			(currentDepth < 0) || ((mode != 0) && (mode != 1))) {
		DISPLAY_INVALID_ARGS;
		exit(1);
	}

	if(tree->debugMode) {
		displayNode(tree, __FUNCTION__, node, keyLength, currentDepth);
	}

	for(ii = 0; ii < node->length; ii++) {
		if(tree->compareFunc(key, keyLength, node->links[ii].key,
					node->links[ii].keyLength) == 0) {
			// validate that this is not simply a matching node on a
			// non-leaf node

			if((mode == 1) && (node->links[ii].next != NULL)) {
				continue;
			}

			// validate that the remove link contains less than 1 items

			if((mode == 1) && (node->links[ii].value != NULL)) {
				localNode = (BptreeNode *)node->links[ii].next;
				if(localNode != NULL) {
					if(localNode->length > 0) {
						fprintf(stderr, "error - node length is %i, "
								"aborting\n", localNode->length);
						continue;
					}
				}
			}

			// remove the node from the link array

			foundNode = 1;
			jj = 0;
			for(nn = 0; nn < node->length; nn++) {
				if(jj >= BPTREE_NODE_ELEMENTS) {
					fprintf(stderr, "PANIC - iterator exceeded tree "
							"order with %i of %i, aborting.\n", jj,
							BPTREE_NODE_ELEMENTS);
					fflush(NULL);
					exit(1);
				}
				if(nn != ii) {
					node->links[nn].key = node->links[jj].key;
					node->links[nn].keyLength =
						node->links[jj].keyLength;
					node->links[nn].value = node->links[jj].value;
					node->links[nn].next = node->links[jj].next;
					jj++;
				}
				else {
					free(node->links[jj].key);
					node->links[jj].key = NULL;
					node->links[jj].keyLength = 0;
					if((node->links[jj].value != NULL) &&
							(mode == 0)) {
						if(tree->isComplexFree) {
							tree->complexFreeFunc(tree->complexFreeArgument,
									node->links[jj].value);
						}
						else if(tree->freeFunc ==
								(BptreeFreeFunction)MEMORY_FREE_FUNCTION) {
							MEMORY_FREE_FUNCTION(node->links[jj].value,
									__FILE__, __FUNCTION__, __LINE__);
						}
						else {
							tree->freeFunc(node->links[jj].value);
						}
					}
					node->links[jj].value = NULL;
					node->links[jj].next = NULL;
					jj++;
					node->links[nn].key = node->links[jj].key;
					node->links[nn].keyLength =
						node->links[jj].keyLength;
					node->links[nn].value = node->links[jj].value;
					node->links[nn].next = node->links[jj].next;
					jj++;
				}
			}

			node->length -= 1;

			// initialize any remaining link array items

			for(nn = node->length; nn < BPTREE_NODE_ELEMENTS; nn++) {
				node->links[nn].key = NULL;
				node->links[nn].keyLength = 0;
				node->links[nn].value = NULL;
				node->links[nn].next = NULL;
			}

			break;
		}
	}

	return foundNode;
}


static int removeNode(Bptree *tree, BptreeNode *node, char *key,
		int keyLength, int currentDepth, int mode)
{
	int ii = 0;
	int found = 0;

	BptreeNode *localNode = NULL;

	if((tree == NULL) || (node == NULL) || (key == NULL) || (keyLength < 0) ||
			(currentDepth < 0) || ((mode != 0) && (mode != 1))) {
		DISPLAY_INVALID_ARGS;
		exit(1);
	}

	if(tree->debugMode) {
		displayNode(tree, __FUNCTION__, node, keyLength, currentDepth);
	}

	if((currentDepth == 0) || (mode == 1)) {
		if(node->length > 1) {
			found = exciseNode(tree, node, key, keyLength, currentDepth, mode);
		}
		else if(node->length == 1) {
			if(tree->compareFunc(key, keyLength, node->links[0].key,
						node->links[0].keyLength) == 0) {
				found = 1;
				node->length = 0;
				free(node->links[0].key);
				node->links[0].key = NULL;
				node->links[0].keyLength = 0;
				if((node->links[0].value != NULL) && (mode == 0)) {
					if(tree->isComplexFree) {
						tree->complexFreeFunc(tree->complexFreeArgument,
								node->links[0].value);
					}
					else if(tree->freeFunc ==
							(BptreeFreeFunction)MEMORY_FREE_FUNCTION) {
						MEMORY_FREE_FUNCTION(node->links[0].value, __FILE__,
								__FUNCTION__, __LINE__);
					}
					else {
						tree->freeFunc(node->links[0].value);
					}
				}
				node->links[0].value = NULL;
				node->links[0].next = NULL;
			}
		}

		if(found == 0) {
			return -1;
		}

		subtractFromTreeDepth(tree, currentDepth);

		if(mode == 0) {
			tree->leafCount -= 1;
		}
		else if(currentDepth == tree->treeDepth) {
			tree->treeDepth = calculateTreeDepth(tree);
			if(tree->leafCount == 0) {
				tree->treeDepth = 0;
			}
			if(tree->treeDepth == 0) {
				freeTree(tree, tree->root, tree->treeDepth);
				free(tree->depthCounts);

				tree->depthLength = 1;
				tree->depthCounts = (int *)malloc(
						sizeof(int) * tree->depthLength);
				tree->root = newNode();
			}
		}

		return 0;
	}
	else if(currentDepth > 0) {
		for(ii = 0; ii < node->length; ii++) {
			if(((ii + 1) == node->length) ||
					(tree->compareFunc(key, keyLength,
						node->links[(ii + 1)].key,
						node->links[(ii + 1)].keyLength) == -1)) {
				localNode = (BptreeNode *)node->links[ii].next;

				if(removeNode(tree, (BptreeNode *)node->links[ii].next,
							key, keyLength, (currentDepth - 1), 0) < 0) {
					return -1;
				}

				if(localNode->length == 0) {
					free(localNode);
					node->links[ii].next = NULL;
					if(removeNode(tree, node, node->links[ii].key,
								node->links[ii].keyLength,
								currentDepth, 1) < 0) {
						return -1;
					}
				}

				return 0;
			}
		}
	}

	return -1;
}

static void treeToArray(Bptree *tree, BptreeNode *node, int currentDepth,
		int *listRef, int *keyListLengths, char **keyList, void **valueList)
{
	int ii = 0;

	if((tree == NULL) || (node == NULL) || (currentDepth < 0)) {
		DISPLAY_INVALID_ARGS;
		exit(1);
	}

	if(tree->debugMode) {
		displayNode(tree, __FUNCTION__, node, -1, currentDepth);
	}

	for(ii = 0; ii < node->length; ii++) {
		if(currentDepth == 0) {
			if(*listRef >= tree->leafCount) {
				fprintf(stderr, "error - failed to allocate enough memory "
						"for lists.\n");
				return;
			}

			keyList[*listRef] = copyKey(node->links[ii].key,
					node->links[ii].keyLength);
			keyListLengths[*listRef] = node->links[ii].keyLength;
			valueList[*listRef] = node->links[ii].value;
			*listRef += 1;
		}

		if(currentDepth > 0) {
			treeToArray(tree, (BptreeNode *)node->links[ii].next,
					(currentDepth - 1), listRef, keyListLengths, keyList,
					valueList);
		}
	}
}

static int validateTree(Bptree *tree, BptreeNode *node, int count)
{
	int ii = 0;
	int result = 0;

	if(tree == NULL) {
		DISPLAY_INVALID_ARGS;
		exit(1);
	}

	if((count < 0) || (tree->leafCount == 0)) {
		return 0;
	}

	if((result = validateNode(node, count)) < 0) {
		if(result == -2) {
			displayNode(tree, __FUNCTION__, node, -1, count);
		}
		printf("error - failed to validate node content at %i\n", count);
		return -1;
	}

	for(ii = 0; ii < node->length; ii++) {
		if(validateTree(tree, (BptreeNode *)node->links[ii].next,
					(count - 1)) < 0) {
			return -1;
		}
	}

	return 0;
}


// define b+tree public functions

int bptree_init(Bptree *tree)
{
	if(tree == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	memset(tree, 0, sizeof(Bptree));

	tree->debugMode = afalse;
	tree->isComplexFree = afalse;
	tree->treeDepth = 0;
	tree->leafCount = 0;
	tree->depthLength = 1;
	tree->depthCounts = (int *)malloc(sizeof(int) * tree->depthLength);
	tree->complexFreeArgument = NULL;
	tree->root = newNode();
	tree->freeFunc = (BptreeFreeFunction)MEMORY_FREE_FUNCTION;
	tree->complexFreeFunc =
		(BptreeComplexFreeFunction)bptreeInternalComplexFree;
	tree->compareFunc = bptreeInternalCompareKeys;

	return 0;
}

int bptree_free(Bptree *tree)
{
	if(tree == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	freeTree(tree, tree->root, tree->treeDepth);
	free(tree->depthCounts);

	memset(tree, 0, sizeof(Bptree));

	return 0;
}

int bptree_setFreeFunction(Bptree *tree, BptreeFreeFunction freeFunction)
{
	if((tree == NULL) || (freeFunction == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	tree->freeFunc = freeFunction;

	return 0;
}

int bptree_setComplexFreeFunction(Bptree *tree, void *argument,
		BptreeComplexFreeFunction complexFreeFunction)
{
	if((tree == NULL) || (argument == NULL) || (complexFreeFunction == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	tree->isComplexFree = atrue;
	tree->complexFreeArgument = argument;
	tree->complexFreeFunc = complexFreeFunction;

	return 0;
}

int bptree_setCompareKeysFunction(Bptree *tree,
		BptreeCompareKeysFunction compareFunc)
{
	if((tree == NULL) || (compareFunc == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	tree->compareFunc = compareFunc;

	return 0;
}

int bptree_setDebugMode(Bptree *tree, aboolean debugMode)
{
	if(tree == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	tree->debugMode = debugMode;

	return 0;
}

int bptree_getLeafCount(Bptree *tree, int *leafCount)
{
	if((tree == NULL) || (leafCount == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	*leafCount = tree->leafCount;

	return 0;
}

int bptree_getTreeDepth(Bptree *tree, int *treeDepth)
{
	if((tree == NULL) || (treeDepth == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	*treeDepth = tree->treeDepth;

	return 0;
}

int bptree_clear(Bptree *tree)
{
	if(tree == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(tree->root == NULL) {
		return 0;
	}

	freeTree(tree, tree->root, tree->treeDepth);
	free(tree->depthCounts);

	tree->depthLength = 1;
	tree->depthCounts = (int *)malloc(sizeof(int) * tree->depthLength);
	tree->root = newNode();

	return 0;
}

int bptree_get(Bptree *tree, char *key, int keyLength, void **value)
{
	if((tree == NULL) || (key == NULL) || (keyLength < 0) || (value == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	*value = NULL;

	if(!searchTree(tree, tree->root, key, keyLength, tree->treeDepth,
				value)) {
		return -1;
	}

	return 0;
}

int bptree_getNext(Bptree *tree, char *key, int keyLength, void **value)
{
	if((tree == NULL) || (key == NULL) || (keyLength < 0) || (value == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	*value = directionalTreeSearch(tree, tree->root, key, keyLength,
			tree->treeDepth, SEARCH_DIRECTION_NEXT);
	if(*value == NULL) {
		return -1;
	}

	return 0;
}

int bptree_getPrevious(Bptree *tree, char *key, int keyLength, void **value)
{
	if((tree == NULL) || (key == NULL) || (keyLength < 0) || (value == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	*value = directionalTreeSearch(tree, tree->root, key, keyLength,
			tree->treeDepth, SEARCH_DIRECTION_PREVIOUS);
	if(*value == NULL) {
		return -1;
	}

	return 0;
}

int bptree_put(Bptree *tree, char *key, int keyLength, void *value)
{
	aboolean found = afalse;
	void *searchResult = NULL;

	BptreeNode *current = NULL;
	BptreeNode *newnode = NULL;

	if((tree == NULL) || (key == NULL) || (keyLength < 0)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	found = searchTree(tree, tree->root, key, keyLength, tree->treeDepth,
			&searchResult);

	if(found) {
		return -1;
	}

	tree->leafCount += 1;
	newnode = insertLeaf(tree, tree->root, key, keyLength, value,
			tree->treeDepth);

	if(newnode == NULL) {
		return 0;
	}

	current = newNode();
	current->length = 2;

	current->links[0].key = copyKey(tree->root->links[0].key,
			tree->root->links[0].keyLength);
	current->links[0].keyLength = tree->root->links[0].keyLength;
	current->links[0].value = NULL;
	current->links[0].next = tree->root;

	current->links[1].key = copyKey(newnode->links[0].key,
			newnode->links[0].keyLength);
	current->links[1].keyLength = newnode->links[0].keyLength;
	current->links[1].value = NULL;
	current->links[1].next = newnode;

	tree->root = current;
	tree->treeDepth += 1;

	return 1;
}

int bptree_remove(Bptree *tree, char *key, int keyLength)
{
	int result = 0;

	if((tree == NULL) || (key == NULL) || (keyLength < 0)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	result = removeNode(tree, tree->root, key, keyLength, tree->treeDepth, 0);

	return result;
}

int bptree_toArray(Bptree *tree, int *arrayLength, int **keyLengths,
		char ***keys, void ***values)
{
	int listRef = 0;

	if((tree == NULL) || (arrayLength == NULL) || (keyLengths == NULL) ||
			(keys == NULL) || (values == NULL)) {
		return -1;
	}

	*arrayLength = tree->leafCount;
	*keyLengths = NULL;
	*keys = NULL;
	*values = NULL;

	if(tree->leafCount <= 0) {
		return 0;
	}

	*keys = (char **)malloc(sizeof(char *) * tree->leafCount);
	*keyLengths = (int *)malloc(sizeof(int) * tree->leafCount);
	*values = (void **)malloc(sizeof(void *) * tree->leafCount);

	treeToArray(tree, tree->root, tree->treeDepth, &listRef, *keyLengths,
			*keys, *values);

	return 0;
}

int bptree_performInternalTest(Bptree *tree)
{
	int result = 0;

	if(tree == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	result = validateTree(tree, tree->root, tree->treeDepth);

	return result;
}

