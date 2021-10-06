/*
 * summary.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard-WoW statistical analysis summarization engine, header file.
 *
 * Written by Josh English.
 */

#if !defined(_WOWSTATS_SYSTEM_SUMMARY_H)

#define _WOWSTATS_SYSTEM_SUMMARY_H

#if !defined(_WOWSTATS_SYSTEM_H) && !defined(_WOWSTATS_SYSTEM_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _WOWSTATS_SYSTEM_H || _WOWSTATS_SYSTEM_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

// define wowstats summarization engine public constants

#define WOW_SUMMARY_EVENT_NOT_APPLICABLE				-2
#define WOW_SUMMARY_BAD_EVENT							-3
#define WOW_SUMMARY_INVALID_SUMMARY						-4
#define WOW_SUMMARY_INVALID_RECORD						-5
#define WOW_SUMMARY_RECORD_MISMATCH						-6

// define wowstats summarization engine public data types

typedef struct _WowSummaryHealing {
	int spellId;
	char *spellName;
	char *spellSchool;
	int spellRank;
	int directCount;
	int periodicCount;
	int healAmount;
	int overhealAmount;
	int criticalHealAmount;
	int criticalOverhealAmount;
	int criticalCount;
} WowSummaryHealing;

typedef struct _WowSummaryDamage {
	int spellId;
	char *spellName;
	char *spellSchool;
	int spellRank;
	char *damageType;
	int directCount;
	int periodicCount;
	int damageAmount;
	int overkillAmount;
	int resistAmount;
	int blockAmount;
	int missedCount;
	int missedAmount;
	int absorbedCount;
	int absorbedDamageAmount;
	int absorbedOverkillAmount;
	int absorbedResistAmount;
	int absorbedBlockAmount;
	int criticalCount;
	int criticalDamageAmount;
	int criticalOverkillAmount;
	int criticalResistAmount;
	int criticalBlockAmount;
	int glancingCount;
	int glancingDamageAmount;
	int glancingOverkillAmount;
	int glancingResistAmount;
	int glancingBlockAmount;
	int crushingCount;
	int crushingDamageAmount;
	int crushingOverkillAmount;
	int crushingResistAmount;
	int crushingBlockAmount;
} WowSummaryDamage;

typedef struct _WowSummary {
	int summaryId;
	char *source;
	char *sourceUid;
	char *target;
	char *targetUid;
	int extraElapsedTimeSeconds;
	WowTimestamp firstTimestamp;
	WowTimestamp lastTimestamp;
	void *record;
} WowSummary;

// declare wowstats summarization engine public functions

int wowstats_summaryInit(WowSummary *summary, WowSpellindex *spellIndex,
		WowEvent *event);

int wowstats_summaryNew(WowSummary **summary, WowSpellindex *spellIndex,
		WowEvent *event);

int wowstats_summaryFree(WowSummary *summary);

int wowstats_summaryFreePtr(WowSummary *summary);

int wowstats_summaryClone(WowSummary *summary, WowSummary **newSummary);

int wowstats_summaryPrependName(WowSummary *summary, char *sourceName);

int wowstats_summaryLookup(WowSummary *summary, Pgdb *pgdb, int logHistoryId,
		int realmId, WowSummary **newSummary);

int wowstats_summaryBuildEntityKey(WowSummary *summary, char **key,
		int *keyLength);

int wowstats_summaryBuildKey(WowSummary *summary, char **key, int *keyLength);

int wowstats_summaryBuildOverrideKey(WowSummary *summary, char *sourceUid,
		char **key, int *keyLength);

int wowstats_summaryUpdateRecord(WowSummary *summary, WowSummary *newSummary);

void wowstats_summaryDisplay(WowSummary *summary);

#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _WOWSTATS_SYSTEM_SUMMARY_H

