/*
 * linkindex.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard-WoW statistical analysis link-index system.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#include "wowstats/common.h"
#include "wowstats/core.h"
#define _WOWSTATS_SYSTEM_COMPONENT
#include "wowstats/system/linkindex.h"

// define wowstats link-index private constants

/*
 * TODO: this
 */


// define wowstats link-index private functions

/*
 * TODO: this
 */


// define wowstats link-index private functions

static void freeLink(void *memory)
{
	WowLinkSystem *link = NULL;

	if(memory == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	link = (WowLinkSystem *)memory;

	if(wowstats_linkFree(link) < 0) {
		return;
	}

	free(link);
}

// define wowstats link-index public functions

int wowstats_linkIndexInit(WowLinkIndex *index)
{
	if(index == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	memset(index, 0, sizeof(WowLinkIndex));

	if(bptree_init(&index->index) < 0) {
		return -1;
	}

	if(bptree_setFreeFunction(&index->index, freeLink) < 0) {
		return -1;
	}

	return 0;
}

int wowstats_linkIndexFree(WowLinkIndex *index)
{
	if(index == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(bptree_free(&index->index) < 0) {
		return -1;
	}

	memset(index, 0, sizeof(WowLinkIndex));

	return 0;
}

int wowstats_linkIndexNoteEvent(WowLinkIndex *index, WowEvent *event)
{
	WowLinkSystem *link = NULL;

	if((index == NULL) || (event == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(event->eventId != WOW_EVENT_SPELL_SUMMON) {
		return 0;
	}

	if(bptree_get(&index->index, event->targetUid, strlen(event->targetUid),
				(void *)&link) < 0) {
		if(wowstats_linkNew(&link, event) < 0) {
			return -1;
		}
		if(bptree_put(&index->index, link->objectUid, strlen(link->objectUid),
					link) < 0) {
			return -1;
		}
	}

	return 0;
}

int wowstats_linkIndexGet(WowLinkIndex *index, char *uid, WowLinkSystem **link)
{
	if((index == NULL) || (uid == NULL) || (link == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	*link = NULL;

	if(bptree_get(&index->index, uid, strlen(uid), (void *)link) < 0) {
		return -1;
	}

	return 0;
}

int wowstats_linkIndexGetLinks(WowLinkIndex *index, WowLinkSystem ***links,
		int *linkLength)
{
	int ii = 0;
	int *keyLengths = NULL;
	char **keys = NULL;

	if((index == NULL) || (links == NULL) || (linkLength == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	*linkLength = 0;

	if(bptree_toArray(&index->index, linkLength, &keyLengths, &keys,
				(void ***)links) < 0) {
		return -1;
	}

	free(keyLengths);

	for(ii = 0; ii < *linkLength; ii++) {
		free(keys[ii]);
	}
	free(keys);

	return 0;
}

