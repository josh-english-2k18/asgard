/*
 * wildcard_index.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * A library to provide an API for building and searching against wildcard
 * indexes from input strings.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#define _CORE_LIBRARY_TYPES_COMPONENT
#include "core/library/types/wildcard_index.h"


// declare wildcard index private functions

static int updateWorkspace(WildcardIndex *wildcard, char *string);

static void freeWildcardEntry(void *memory);

static int updateWildcardEntry(WildcardIndex *wildcard, int index,
		aboolean isAddition);


// define wildcard index private functions

static int updateWorkspace(WildcardIndex *wildcard, char *string)
{
	int ii = 0;
	int nn = 0;
	int cValue = 0;

	for(ii = 0, nn = 0;
			((string[ii] != '\0') && (nn < wildcard->maxStringLength));
			ii++) {
		cValue = ctype_ctoi(string[ii]);

		if((cValue < 32) || (cValue > 126)) { // non-printable
			wildcard->workspace[nn] = ' ';
			nn++;
		}
		else if(cValue == 32) { // space
			wildcard->workspace[nn] = ' ';
			nn++;
		}
		else if((cValue > 64) && (cValue < 91)) { // uppercase letters
			wildcard->workspace[nn] = (char)(cValue + 32);
			nn++;
		}
		else if(((cValue > 47) && (cValue < 58)) || // numbers
				((cValue > 96) && (cValue < 123)) // lower-case
				) {
			wildcard->workspace[nn] = string[ii];
			nn++;
		}
	}

	if(nn < wildcard->minStringLength) {
		return -1;
	}

	wildcard->workspace[nn] = '\0';
	wildcard->workspaceLength = nn;

	return wildcard->workspaceLength;
}

static void freeWildcardEntry(void *memory)
{
	IntegerArray *entry = NULL;

	if(memory == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	entry = (IntegerArray *)memory;

	integer_freePtr(entry);
}

static int updateWildcardEntry(WildcardIndex *wildcard, int index,
		aboolean isAddition)
{
	int ii = 0;
	int length = 0;
	int result = 0;
	char *ptr = NULL;

	IntegerArray *entry = NULL;

	// create sub-strings from beginning-of-string & take action

	for(ii = 1; ii < wildcard->workspaceLength; ii++) {
		if((length = (wildcard->workspaceLength - ii)) <
				wildcard->minStringLength) {
			break;
		}

		ptr = (wildcard->workspace + ii);

		if(bptree_get(&wildcard->index, ptr, length, ((void *)&entry)) < 0) {
			if(isAddition) {
				entry = integer_new();

				integer_put(entry, index);

				if(bptree_put(&wildcard->index, ptr, length,
							(void *)entry) < 0) {
					result = -1;
				}
			}
			else {
				continue;
			}
		}
		else {
			if(isAddition) {
				if(integer_put(entry, index) < 0) {
					result = -1;
				}
			}
			else {
				if(integer_remove(entry, index) == 0) {
					if(entry->length == 0) {
						if(bptree_remove(&wildcard->index, ptr, length) < 0) {
							result = -1;
						}
					}
				}
			}
		}
	}

	// create sub-strings from end-of-string & take action

	for(ii = (wildcard->workspaceLength - 1);
			ii >= wildcard->minStringLength;
			ii--) {
		ptr = wildcard->workspace;

		ptr[ii] = '\0';
		length = ii;

		if(bptree_get(&wildcard->index, ptr, length, ((void *)&entry)) < 0) {
			if(isAddition) {
				entry = integer_new();

				integer_put(entry, index);

				if(bptree_put(&wildcard->index, ptr, length,
							(void *)entry) < 0) {
					result = -1;
				}
			}
			else {
				continue;
			}
		}
		else {
			if(isAddition) {
				if(integer_put(entry, index) < 0) {
					result = -1;
				}
			}
			else {
				if(integer_remove(entry, index) == 0) {
					if(entry->length == 0) {
						if(bptree_remove(&wildcard->index, ptr, length) < 0) {
							result = -1;
						}
					}
				}
			}
		}
	}

	return result;
}


// define wildcard index public functions

int wildcardIndex_init(WildcardIndex *wildcard, int minStringLength,
		int maxStringLength)
{
	if((wildcard == NULL) || (minStringLength < 0) || (maxStringLength < 0) ||
			(minStringLength > maxStringLength)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	memset(wildcard, 0, (int)(sizeof(WildcardIndex)));

	wildcard->minStringLength = minStringLength;
	wildcard->maxStringLength = maxStringLength;
	wildcard->ticks = 0;
	wildcard->workspaceLength = 0;
	wildcard->workspace = (char *)malloc(sizeof(char) * (maxStringLength + 8));

	bptree_init(&wildcard->index);
	bptree_setFreeFunction(&wildcard->index, freeWildcardEntry);

	return 0;
}

int wildcardIndex_free(WildcardIndex *wildcard)
{
	if(wildcard == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(wildcard->workspace != NULL) {
		free(wildcard->workspace);
	}

	bptree_free(&wildcard->index);

	return 0;
}

alint wildcardIndex_getTicks(WildcardIndex *wildcard)
{
	if(wildcard == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return wildcard->ticks;
}

int wildcardIndex_getIndexCount(WildcardIndex *wildcard)
{
	int result = 0;

	if(wildcard == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(bptree_getLeafCount(&wildcard->index, &result) < 0) {
		return -2;
	}

	return result;
}

int wildcardIndex_getIndexDepth(WildcardIndex *wildcard)
{
	int result = 0;

	if(wildcard == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(bptree_getTreeDepth(&wildcard->index, &result) < 0) {
		return -2;
	}

	return result;
}

int wildcardIndex_setStringThresholds(WildcardIndex *wildcard,
		int minStringLength, int maxStringLength)
{
	if((wildcard == NULL) || (minStringLength < 0) || (maxStringLength < 0) ||
			(minStringLength > maxStringLength)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	wildcard->minStringLength = minStringLength;
	wildcard->maxStringLength = maxStringLength;

	wildcard->workspaceLength = 0;

	if(wildcard->workspace != NULL) {
		free(wildcard->workspace);
	}

	wildcard->workspace = (char *)malloc(sizeof(char) * (maxStringLength + 8));

	return 0;
}

int *wildcardIndex_get(WildcardIndex *wildcard, char *string, int *length)
{
	int resultLength = 0;
	int *result = 0;

	IntegerArray *entry = NULL;

	if((wildcard == NULL) || (string == NULL) || (length == NULL)) {
		DISPLAY_INVALID_ARGS;

		if(length != NULL) {
			*length = 0;
		}

		return NULL;
	}

	*length = 0;

	wildcard->ticks += 1;

	if((updateWorkspace(wildcard, string) < 0) ||
			(wildcard->workspaceLength < wildcard->minStringLength) ||
			(wildcard->workspaceLength > wildcard->maxStringLength)) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s():%i] error - normalized string length %i is "
					"out-of-bounds.\n", __FUNCTION__, __LINE__,
					wildcard->workspaceLength);
		}
		return NULL;
	}

	if((bptree_get(&wildcard->index, wildcard->workspace,
					wildcard->workspaceLength, ((void *)&entry)) < 0) ||
			(entry == NULL)) {
		return NULL;
	}

	resultLength = entry->length;

	result = (int *)malloc(sizeof(int) * resultLength);

	memcpy(result, entry->array, (int)(sizeof(int) * resultLength));

	*length = resultLength;

	return result;
}

int wildcardIndex_put(WildcardIndex *wildcard, int type, char *string,
		int index)
{
	int result = 0;

	IntegerArray *entry = NULL;

	if((wildcard == NULL) ||
			((type != WILDCARD_INDEX_TYPE_STANDARD) &&
			 (type != WILDCARD_INDEX_TYPE_EXACT)) ||
			(string == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	wildcard->ticks += 1;

	if((updateWorkspace(wildcard, string) < 0) ||
			(wildcard->workspaceLength < wildcard->minStringLength) ||
			(wildcard->workspaceLength > wildcard->maxStringLength)) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s():%i] error - normalized string length %i is "
					"out-of-bounds.\n", __FUNCTION__, __LINE__,
					wildcard->workspaceLength);
		}
		return -1;
	}

	if(bptree_get(&wildcard->index, wildcard->workspace,
				wildcard->workspaceLength, ((void *)&entry)) < 0) {
		if((entry = integer_new()) == NULL) {
			return -1;
		}

		if(integer_put(entry, index) == 0) {
			if(bptree_put(&wildcard->index, wildcard->workspace,
						wildcard->workspaceLength, (void *)entry) < 0) {
				return -1;
			}
		}
		else {
			integer_freePtr(entry);
			return -1;
		}
	}
	else {
		if(integer_put(entry, index) < 0) {
			result = -1;
		}
	}

	if(type == WILDCARD_INDEX_TYPE_EXACT) {
		return 0;
	}

	if(updateWildcardEntry(wildcard, index, atrue) < 0) {
		return -1;
	}

	return result;
}

int wildcardIndex_remove(WildcardIndex *wildcard, char *string, int index)
{
	IntegerArray *entry = NULL;

	if((wildcard == NULL) || (string == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	wildcard->ticks += 1;

	if((updateWorkspace(wildcard, string) < 0) ||
			(wildcard->workspaceLength < wildcard->minStringLength) ||
			(wildcard->workspaceLength > wildcard->maxStringLength)) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s():%i] error - normalized string length %i is "
					"out-of-bounds.\n", __FUNCTION__, __LINE__,
					wildcard->workspaceLength);
		}
		return -1;
	}

	if(bptree_get(&wildcard->index, wildcard->workspace,
				wildcard->workspaceLength, ((void *)&entry)) == 0) {
		if(integer_remove(entry, index) < 0) {
			return -1;
		}
		else if(entry->length == 0) {
			if(bptree_remove(&wildcard->index, wildcard->workspace,
						wildcard->workspaceLength) < 0) {
				return -1;
			}
		}
	}

	return updateWildcardEntry(wildcard, index, afalse);
}

