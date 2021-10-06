/*
 * playerindex.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard-WoW statistical analysis player-index system, header file.
 *
 * Written by Josh English.
 */

#if !defined(_WOWSTATS_SYSTEM_PLAYERINDEX_H)

#define _WOWSTATS_SYSTEM_PLAYERINDEX_H

#if !defined(_WOWSTATS_SYSTEM_H) && !defined(_WOWSTATS_SYSTEM_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _WOWSTATS_SYSTEM_H || _WOWSTATS_SYSTEM_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

// define wowstats player-index public data types

typedef struct _WowPlayerIndex {
	int currentPlayerCount;
	int alivePlayerCount;
	WowSpellindex *spellIndex;
	Bptree index;
} WowPlayerIndex;

// declare wowstats player-index public functions

int wowstats_playerIndexInit(WowPlayerIndex *index, WowSpellindex *spellIndex);

int wowstats_playerIndexFree(WowPlayerIndex *index);

int wowstats_playerIndexNoteEvent(WowPlayerIndex *index, WowEvent *event);

int wowstats_playerIndexGetSummaries(WowPlayerIndex *index,
		WowPlayerSummary ***summaries, int *summaryLength);

#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _WOWSTATS_SYSTEM_PLAYERINDEX_H

