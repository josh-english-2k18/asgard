/*
 * spellindex.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard-WoW statistical analysis spell index.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#include "wowstats/common.h"
#define _WOWSTATS_DATA_COMPONENT
#include "wowstats/data/spells.h"
#define _WOWSTATS_CORE_COMPONENT
#include "wowstats/core/spellindex.h"

// declare wowstats spell index private functions

static WowSpellList *newList(int listRef);

static void freeList(void *memory);


// define wowstats spell index private functions

static WowSpellList *newList(int listRef)
{
	WowSpellList *result = NULL;

	result = (WowSpellList *)malloc(sizeof(WowSpellList));

	result->id = WOW_SPELL_LIST[listRef].id;
	result->playerClass = strdup(WOW_SPELL_LIST[listRef].playerClass);
	result->name = strdup(WOW_SPELL_LIST[listRef].name);
	result->rank = WOW_SPELL_LIST[listRef].rank;

	return result;
}

static void freeList(void *memory)
{
	WowSpellList *list = NULL;

	if(memory == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	list = (WowSpellList *)memory;

	free(list->playerClass);
	free(list->name);
	free(list);
}


// define wowstats spell index public functions

int wowstats_spellindexInit(WowSpellindex *index)
{
	int ii = 0;
	char key[128];

	WowSpellList *list = NULL;

	if(index == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(bptree_init(&index->index) < 0) {
		return -1;
	}

	if(bptree_setFreeFunction(&index->index, freeList) < 0) {
		return -1;
	}

	for(ii = 0; WOW_SPELL_LIST[ii].id != -1; ii++) {
		list = newList(ii);

		snprintf(key, (sizeof(key) - 1), "%i", list->id);

		if(bptree_put(&index->index, key, strlen(key), list) < 0) {
			freeList(list);
			return -1;
		}
	}

	return 0;
}

int wowstats_spellindexFree(WowSpellindex *index)
{
	if(index == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return bptree_free(&index->index);
}

int wowstats_spellindexLookup(WowSpellindex *index, int spellId,
		WowSpellList **spell)
{
	char key[128];

	if(index == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	snprintf(key, (sizeof(key) - 1), "%i", spellId);

	if(bptree_get(&index->index, key, strlen(key), (void **)spell) < 0) {
//		fprintf(stderr, "%i\n", spellId);
		return -1;
	}

	return 0;
}

