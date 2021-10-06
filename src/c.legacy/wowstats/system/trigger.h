/*
 * trigger.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard-WoW statistical analysis trigger system, header file.
 *
 * Written by Josh English.
 */

#if !defined(_WOWSTATS_SYSTEM_TRIGGER_H)

#define _WOWSTATS_SYSTEM_TRIGGER_H

#if !defined(_WOWSTATS_SYSTEM_H) && !defined(_WOWSTATS_SYSTEM_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _WOWSTATS_SYSTEM_H || _WOWSTATS_SYSTEM_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

// define wowstats trigger system public data types

typedef struct _WowTriggerResult {
	char *zone;
	char *bossName;
	WowTimestamp startTimestamp;
	WowTimestamp endTimestamp;
} WowTriggerResult;

typedef struct _WowTrigger {
	aboolean analyzeMode;
	aboolean hasNotedTimestamp;
	aboolean hasBossTrigger;
	int bossDeathCount;
	char *bossUidList[TRIGGER_MAX_BOSS_LENGTH];
	WowTriggerList *currentBoss;
	WowTimestamp startTimestamp;
	WowTimestamp endTimestamp;
	WowTimestamp lastTriggerTimestamp;
	Bptree bossIndex;
} WowTrigger;

// declare wowstats trigger system public functions

int wowstats_triggerInit(WowTrigger *trigger);

int wowstats_triggerFree(WowTrigger *trigger);

int wowstats_triggerNoteEvent(WowTrigger *trigger,
		WowPlayerIndex *playerIndex, WowEvent *event,
		WowTriggerResult *triggerResult);

#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _WOWSTATS_SYSTEM_TRIGGER_H

