/*
 * player.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard-WoW statistical analysis player-summary system.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#include "wowstats/common.h"
#include "wowstats/core.h"
#define _WOWSTATS_SYSTEM_COMPONENT
#include "wowstats/system/player.h"

// define wowstats player-summary public functions

int wowstats_playerInit(WowPlayerSummary *player, char *name, char *uid)
{
	if((player == NULL) || (name == NULL) || (uid == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	memset(player, 0, sizeof(WowPlayerSummary));

	player->hasNotedTimestamp = afalse;
	player->isPlayerAlive = atrue;
	player->classId = WOW_PLAYER_CLASS_UNKNOWN;
	player->name = strdup(name);
	player->uid = strdup(uid);

	if(wowstats_timestampInit(&player->firstTimestamp, 0) < 0) {
		return -1;
	}

	if(wowstats_timestampInit(&player->lastTimestamp, 0) < 0) {
		return -1;
	}

	return 0;
}

int wowstats_playerFree(WowPlayerSummary *player)
{
	if(player == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	free(player->name);
	free(player->uid);

	return 0;
}

int wowstats_playerSetClassId(WowPlayerSummary *player, int classId)
{
	if(player == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	player->classId = classId;

	return 0;
}

int wowstats_playerNoteTimestamp(WowPlayerSummary *player,
		WowTimestamp *timestamp)
{
	int rc = 0;

	if((player == NULL) || (timestamp == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(!player->hasNotedTimestamp) {
		if(wowstats_timestampClone(&player->firstTimestamp, timestamp) < 0) {
			return -1;
		}

		if(wowstats_timestampClone(&player->lastTimestamp, timestamp) < 0) {
			return -1;
		}

		player->hasNotedTimestamp = atrue;
	}
	else {
		if(wowstats_timestampCompare(&player->firstTimestamp,
					timestamp, &rc) < 0) {
			return -1;
		}

		if(rc == WOW_TIMESTAMP_LESS_THAN) {
			if(wowstats_timestampClone(&player->firstTimestamp,
						timestamp) < 0) {
				return -1;
			}
		}

		if(wowstats_timestampCompare(&player->lastTimestamp,
					timestamp, &rc) < 0) {
			return -1;
		}

		if(rc == WOW_TIMESTAMP_GREATER_THAN) {
			if(wowstats_timestampClone(&player->lastTimestamp,
						timestamp) < 0) {
				return -1;
			}
		}
	}

	return 0;
}

