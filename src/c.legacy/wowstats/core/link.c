/*
 * link.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard-WoW statistical analysis link (correlation between a player and
 * an object, e.g. pet, etc.) system.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#include "wowstats/common.h"
#define _WOWSTATS_CORE_COMPONENT
#include "wowstats/core/parser.h"
#include "wowstats/core/timestamp.h"
#include "wowstats/core/event.h"
#include "wowstats/core/link.h"

// define wowstats link-system private functions

static int uidToUnitId(char *uid)
{
	int ii = 0;
	int nn = 0;
	unsigned int unitId = 0;
	char buffer[32];

	buffer[0] = '0';
	buffer[1] = 'x';

	for(ii = 5, nn = 2; ii < 12; ii++, nn++) {
		buffer[nn] = uid[ii];
	}
	buffer[nn] = '\0';

	sscanf(buffer, "%x", &unitId);

	return unitId;
}

// define wowstats link-system public functions

int wowstats_linkInit(WowLinkSystem *link, WowEvent *event)
{
	if((link == NULL) || (event == NULL) ||
			(event->eventId != WOW_EVENT_SPELL_SUMMON)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	memset(link, 0, sizeof(WowLinkSystem));

	link->playerUid = strdup(event->sourceUid);
	link->playerName = strdup(event->sourceName);
	link->objectId = uidToUnitId(event->targetUid);
	link->objectUid = strdup(event->targetUid);
	link->objectName = strdup(event->targetName);

	return 0;
}

int wowstats_linkNew(WowLinkSystem **link, WowEvent *event)
{
	if((link == NULL) || (event == NULL) ||
			(event->eventId != WOW_EVENT_SPELL_SUMMON)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	*link = (WowLinkSystem *)malloc(sizeof(WowLinkSystem));

	return wowstats_linkInit(*link, event);
}

int wowstats_linkFree(WowLinkSystem *link)
{
	if(link == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(link->playerUid != NULL) {
		free(link->playerUid);
	}

	if(link->playerName != NULL) {
		free(link->playerName);
	}

	if(link->objectUid != NULL) {
		free(link->objectUid);
	}

	if(link->objectName != NULL) {
		free(link->objectName);
	}

	memset(link, 0, sizeof(WowLinkSystem));

	return 0;
}

