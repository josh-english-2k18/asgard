/*
 * player.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard-WoW statistical analysis player-summary system, header file.
 *
 * Written by Josh English.
 */

#if !defined(_WOWSTATS_SYSTEM_PLAYER_H)

#define _WOWSTATS_SYSTEM_PLAYER_H

#if !defined(_WOWSTATS_SYSTEM_H) && !defined(_WOWSTATS_SYSTEM_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _WOWSTATS_SYSTEM_H || _WOWSTATS_SYSTEM_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

// define wowstats player-summary public data types

typedef struct _WowPlayerSummary {
	aboolean hasNotedTimestamp;
	aboolean isPlayerAlive;
	int classId;
	char *name;
	char *uid;
	WowTimestamp firstTimestamp;
	WowTimestamp lastTimestamp;
	WowTimestamp diedTimestamp;
} WowPlayerSummary;

// declare wowstats player-summary public functions

int wowstats_playerInit(WowPlayerSummary *player, char *name, char *uid);

int wowstats_playerFree(WowPlayerSummary *player);

int wowstats_playerSetClassId(WowPlayerSummary *player, int classId);

int wowstats_playerNoteTimestamp(WowPlayerSummary *player,
		WowTimestamp *timestamp);

#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _WOWSTATS_SYSTEM_PLAYER_H

