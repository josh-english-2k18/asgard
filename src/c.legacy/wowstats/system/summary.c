/*
 * summary.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard-WoW statistical analysis summarization engine.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#include "wowstats/common.h"
#define _WOWSTATS_DATA_COMPONENT
#include "wowstats/data/eventspells.h"
#include "wowstats/data/summary.h"
#include "wowstats/core.h"
#include "wowstats/db.h"
#define _WOWSTATS_SYSTEM_COMPONENT
#include "wowstats/system/summary.h"
#include "wowstats/system/wsdal.h"

// define wowstats summarization engine private functions

// general summary functions

static char *getSpellSchool(int spellSchool)
{
	int ii = 0;
	char *result = "Unknown";

	for(ii = 0; WOW_SPELL_SCHOOL[ii].id != WOW_SPELL_SCHOOL_END_OF_LIST;
			ii++) {
		if(WOW_SPELL_SCHOOL[ii].id == spellSchool) {
			result = WOW_SPELL_SCHOOL[ii].name;
			break;
		}
	}

	return result;
}

// 'Healing' summary functions

static void initSummaryHealingFromHealed(WowSummaryHealing *record,
		WowEvent *event)
{
	WowEventSpellHealed *spellHealedEvent = NULL;

	spellHealedEvent = (WowEventSpellHealed *)event->eventValue;

	record->spellName = strdup(spellHealedEvent->spellName);
	record->spellSchool = strdup(getSpellSchool(
				spellHealedEvent->spellSchool));
	record->directCount = 1;
	record->periodicCount = 0;
	record->healAmount = spellHealedEvent->healAmount;
	record->overhealAmount = spellHealedEvent->overHealingAmount;

	if(spellHealedEvent->wasCritical) {
		record->criticalHealAmount = spellHealedEvent->healAmount;
		record->criticalOverhealAmount = spellHealedEvent->overHealingAmount;
		record->criticalCount = 1;
	}
	else {
		record->criticalHealAmount = 0;
		record->criticalOverhealAmount = 0;
		record->criticalCount = 0;
	}
}

static void initSummaryHealingFromPeriodicHealed(WowSummaryHealing *record,
		WowEvent *event)
{
	WowEventSpellPeriodicHealed *periodicHealedEvent = NULL;

	periodicHealedEvent = (WowEventSpellPeriodicHealed *)event->eventValue;

	record->spellName = strdup(periodicHealedEvent->spellName);
	record->spellSchool = strdup(getSpellSchool(
				periodicHealedEvent->spellSchool));
	record->directCount = 0;
	record->periodicCount = 1;
	record->healAmount = periodicHealedEvent->healAmount;
	record->overhealAmount = periodicHealedEvent->overHealingAmount;

	if(periodicHealedEvent->wasCritical) {
		record->criticalHealAmount = periodicHealedEvent->healAmount;
		record->criticalOverhealAmount = periodicHealedEvent->overHealingAmount;
		record->criticalCount = 1;
	}
	else {
		record->criticalHealAmount = 0;
		record->criticalOverhealAmount = 0;
		record->criticalCount = 0;
	}
}

static void initSummaryHealingFromLeech(WowSummaryHealing *record,
		WowEvent *event)
{
	WowEventSpellLeech *leechEvent = NULL;

	leechEvent = (WowEventSpellLeech *)event->eventValue;

	record->spellName = strdup(leechEvent->spellName);
	record->spellSchool = strdup(getSpellSchool(
				leechEvent->spellSchool));
	record->directCount = 1;
	record->periodicCount = 0;
	record->healAmount = leechEvent->leechAmount;
	record->overhealAmount = 0;
	record->criticalHealAmount = 0;
	record->criticalOverhealAmount = 0;
	record->criticalCount = 0;
}

static WowSummaryHealing *newSummaryHealing(WowSpellindex *spellIndex,
		WowEvent *event)
{
	int spellId = 0;
	int spellRank = 0;

	WowSpellList *spell = NULL;
	WowSummaryHealing *result = NULL;

	if((event->eventId != WOW_EVENT_SPELL_HEAL) &&
			(event->eventId != WOW_EVENT_SPELL_PERIODIC_HEAL) &&
			(event->eventId != WOW_EVENT_SPELL_LEECH)) {
		return NULL;
	}

	switch(event->eventId) {
		case WOW_EVENT_SPELL_HEAL:
			spellId = ((WowEventSpellHealed *)event->eventValue)->spellId;
			break;
		case WOW_EVENT_SPELL_PERIODIC_HEAL:
			spellId =
				((WowEventSpellPeriodicHealed *)event->eventValue)->spellId;
			break;
		case WOW_EVENT_SPELL_LEECH:
			spellId = ((WowEventSpellLeech *)event->eventValue)->spellId;
			break;
	}

	if(wowstats_spellindexLookup(spellIndex, spellId, &spell) < 0) {
		spellRank = 0;
	}
	else {
		spellRank = spell->rank;
	}

	result = (WowSummaryHealing *)malloc(sizeof(WowSummaryHealing));

	result->spellId = spellId;
	result->spellRank = spellRank;

	switch(event->eventId) {
		case WOW_EVENT_SPELL_HEAL:
			initSummaryHealingFromHealed(result, event);
			break;
		case WOW_EVENT_SPELL_PERIODIC_HEAL:
			initSummaryHealingFromPeriodicHealed(result, event);
			break;
		case WOW_EVENT_SPELL_LEECH:
			initSummaryHealingFromLeech(result, event);
			break;
	}

	return result;
}

static WowSummaryHealing *newSummaryHealingFromRecord(
		WowSummaryHealing *record)
{
	WowSummaryHealing *result = NULL;

	result = (WowSummaryHealing *)malloc(
			sizeof(WowSummaryHealing));

	result->spellId = record->spellId;
	result->spellName = strdup(record->spellName);
	result->spellSchool = strdup(record->spellSchool);
	result->spellRank = record->spellRank;
	result->directCount = record->directCount;
	result->periodicCount = record->periodicCount;
	result->healAmount = record->healAmount;
	result->overhealAmount = record->overhealAmount;
	result->criticalHealAmount = record->criticalHealAmount;
	result->criticalOverhealAmount = record->criticalOverhealAmount;
	result->criticalCount = record->criticalCount;

	return result;
}

static void prependSummaryHealingName(WowSummaryHealing *record,
		char *sourceName)
{
	int length = 0;
	char *spellName = NULL;

	if(record->spellName == NULL) {
		record->spellName = strdup("Unknown");
	}

	length = (strlen(sourceName) + strlen(record->spellName) + 8);
	spellName = (char *)malloc(sizeof(char) * (length + 1));

	snprintf(spellName, length, "%s: %s", sourceName, record->spellName);

	free(record->spellName);

	record->spellName = spellName;
}

static void freeSummaryHealing(WowSummary *summary)
{
	WowSummaryHealing *record = NULL;

	record = (WowSummaryHealing *)summary->record;

	free(record->spellName);
	free(record->spellSchool);
	free(record);
}

static char *buildSummaryHealingKey(WowSummary *summary)
{
	char buffer[8192];

	WowSummaryHealing *record = NULL;

	record = (WowSummaryHealing *)summary->record;

	snprintf(buffer, (sizeof(buffer) - 1),
			"/i%i/%s/%s/%i",
			summary->summaryId,
			summary->sourceUid,
			summary->targetUid,
			record->spellId);

	return strdup(buffer);
}

static char *buildSummaryHealingOverrideKey(WowSummary *summary,
		char *sourceUid)
{
	char buffer[8192];

	WowSummaryHealing *record = NULL;

	record = (WowSummaryHealing *)summary->record;

	snprintf(buffer, (sizeof(buffer) - 1),
			"/p%i/%s/%s/%i",
			summary->summaryId,
			sourceUid,
			summary->targetUid,
			record->spellId);

	return strdup(buffer);
}

static int updateSummaryHealing(WowSummary *summary, WowSummary *newSummary)
{
	WowSummaryHealing *record = NULL;
	WowSummaryHealing *newRecord = NULL;

	if(summary->summaryId != newSummary->summaryId) {
		return WOW_SUMMARY_RECORD_MISMATCH;
	}

	record = (WowSummaryHealing *)summary->record;
	newRecord = (WowSummaryHealing *)newSummary->record;

	if(record->spellId != newRecord->spellId) {
		return WOW_SUMMARY_RECORD_MISMATCH;
	}

	if(newRecord->directCount > 0) {
		record->directCount += newRecord->directCount;
	}

	if(newRecord->periodicCount > 0) {
		record->periodicCount += newRecord->periodicCount;
	}

	if(newRecord->healAmount > 0) {
		record->healAmount += newRecord->healAmount;
	}

	if(newRecord->overhealAmount > 0) {
		record->overhealAmount += newRecord->overhealAmount;
	}

	if(newRecord->criticalCount > 0) {
		record->criticalHealAmount += newRecord->criticalHealAmount;
		record->criticalOverhealAmount += newRecord->criticalOverhealAmount;
		record->criticalCount += newRecord->criticalCount;
	}

	return 0;
}

static void displaySummaryHealing(WowSummary *summary)
{
	int seconds = 0;

	WowSummaryHealing *record = NULL;

	if(wowstats_timestampCalculateDifferenceInSeconds(
				&summary->lastTimestamp,
				&summary->firstTimestamp,
				&seconds) < 0) {
		return;
	}

	record = (WowSummaryHealing *)summary->record;

	printf("[%s]{'%s'->'%s'} cast '%s' school '%s' rank %i, direct %i, "
			"periodic %i for %i healing, %i over-healing, %i critical over "
			"%i seconds\n",
			WOW_SUMMARY_LIST[(summary->summaryId - 1)].name, summary->source,
			summary->target, record->spellName, record->spellSchool,
			record->spellRank, record->directCount, record->periodicCount,
			record->healAmount, record->overhealAmount, record->criticalCount,
			seconds);
}

// 'Damage' summary functions

static void setSummaryDamageAbsorbed(WowSummaryDamage *record,
		aboolean absorbed, int damageAmount, int overkillAmount,
		int resistAmount, int blockAmount)
{
	if(absorbed) {
		record->absorbedCount = 1;
		record->absorbedDamageAmount = damageAmount;
		record->absorbedOverkillAmount = overkillAmount;
		record->absorbedResistAmount = resistAmount;
		record->absorbedBlockAmount = blockAmount;
	}
	else {
		record->absorbedCount = 0;
		record->absorbedDamageAmount = 0;
		record->absorbedOverkillAmount = 0;
		record->absorbedResistAmount = 0;
		record->absorbedBlockAmount = 0;
	}
}

static void setSummaryDamageCritical(WowSummaryDamage *record,
		aboolean wasCritical, int damageAmount, int overkillAmount,
		int resistAmount, int blockAmount)
{
	if(wasCritical) {
		record->criticalCount = 1;
		record->criticalDamageAmount = damageAmount;
		record->criticalOverkillAmount = overkillAmount;
		record->criticalResistAmount = resistAmount;
		record->criticalBlockAmount = blockAmount;
	}
	else {
		record->criticalCount = 0;
		record->criticalDamageAmount = 0;
		record->criticalOverkillAmount = 0;
		record->criticalResistAmount = 0;
		record->criticalBlockAmount = 0;
	}
}

static void setSummaryDamageGlancing(WowSummaryDamage *record,
		aboolean glancing, int damageAmount, int overkillAmount,
		int resistAmount, int blockAmount)
{
	if(glancing) {
		record->glancingCount = 1;
		record->glancingDamageAmount = damageAmount;
		record->glancingOverkillAmount = overkillAmount;
		record->glancingResistAmount = resistAmount;
		record->glancingBlockAmount = blockAmount;
	}
	else {
		record->glancingCount = 0;
		record->glancingDamageAmount = 0;
		record->glancingOverkillAmount = 0;
		record->glancingResistAmount = 0;
		record->glancingBlockAmount = 0;
	}
}

static void setSummaryDamageCrushing(WowSummaryDamage *record,
		aboolean crushing, int damageAmount, int overkillAmount,
		int resistAmount, int blockAmount)
{
	if(crushing) {
		record->crushingCount = 1;
		record->crushingDamageAmount = damageAmount;
		record->crushingOverkillAmount = overkillAmount;
		record->crushingResistAmount = resistAmount;
		record->crushingBlockAmount = blockAmount;
	}
	else {
		record->crushingCount = 0;
		record->crushingDamageAmount = 0;
		record->crushingOverkillAmount = 0;
		record->crushingResistAmount = 0;
		record->crushingBlockAmount = 0;
	}
}

static void initSummaryDamageFromDamageShield(WowSummaryDamage *record,
		WowEvent *event)
{
	WowEventDamageShield *damageShieldEvent = NULL;

	damageShieldEvent = (WowEventDamageShield *)event->eventValue;

	record->spellId = damageShieldEvent->spellId;
	record->spellName = strdup(damageShieldEvent->spellName);
	record->spellSchool = strdup(getSpellSchool(
				damageShieldEvent->spellSchool));
	record->directCount = 1;
	record->periodicCount = 0;
	record->damageAmount = damageShieldEvent->damageAmount;
	record->overkillAmount = damageShieldEvent->overkillAmount;
	record->resistAmount = damageShieldEvent->resistAmount;
	record->blockAmount = damageShieldEvent->blockAmount;
	record->missedCount = 0;
	record->missedAmount = 0;

	setSummaryDamageAbsorbed(record, damageShieldEvent->absorbed,
			damageShieldEvent->damageAmount,
			damageShieldEvent->overkillAmount,
			damageShieldEvent->resistAmount,
			damageShieldEvent->blockAmount);

	setSummaryDamageCritical(record, damageShieldEvent->wasCritical,
			damageShieldEvent->damageAmount,
			damageShieldEvent->overkillAmount,
			damageShieldEvent->resistAmount,
			damageShieldEvent->blockAmount);

	setSummaryDamageGlancing(record, damageShieldEvent->glancing,
			damageShieldEvent->damageAmount,
			damageShieldEvent->overkillAmount,
			damageShieldEvent->resistAmount,
			damageShieldEvent->blockAmount);

	setSummaryDamageCrushing(record, damageShieldEvent->crushing,
			damageShieldEvent->damageAmount,
			damageShieldEvent->overkillAmount,
			damageShieldEvent->resistAmount,
			damageShieldEvent->blockAmount);
}

static void initSummaryDamageFromDamageShieldMissed(WowSummaryDamage *record,
		WowEvent *event)
{
	WowEventDamageShieldMissed *damageShieldMissedEvent = NULL;

	damageShieldMissedEvent = (WowEventDamageShieldMissed *)event->eventValue;

	record->spellId = damageShieldMissedEvent->spellId;
	record->spellName = strdup(damageShieldMissedEvent->spellName);
	record->spellSchool = strdup(getSpellSchool(
				damageShieldMissedEvent->spellSchool));
	record->directCount = 0;
	record->periodicCount = 0;
	record->damageAmount = 0;
	record->overkillAmount = 0;
	record->resistAmount = 0;
	record->blockAmount = 0;
	record->missedCount = 1;
	record->missedAmount = damageShieldMissedEvent->missAmount;

	setSummaryDamageAbsorbed(record, afalse, 0, 0, 0, 0);
	setSummaryDamageCritical(record, afalse, 0, 0, 0, 0);
	setSummaryDamageGlancing(record, afalse, 0, 0, 0, 0);
	setSummaryDamageCrushing(record, afalse, 0, 0, 0, 0);
}

static void initSummaryDamageFromDamageSplit(WowSummaryDamage *record,
		WowEvent *event)
{
	WowEventDamageSplit *damageSplitEvent = NULL;

	damageSplitEvent = (WowEventDamageSplit *)event->eventValue;

	record->spellId = damageSplitEvent->spellId;
	record->spellName = strdup(damageSplitEvent->spellName);
	record->spellSchool = strdup(getSpellSchool(
				damageSplitEvent->spellSchool));
	record->directCount = 1;
	record->periodicCount = 0;
	record->damageAmount = damageSplitEvent->damageAmount;
	record->overkillAmount = damageSplitEvent->overkillAmount;
	record->resistAmount = damageSplitEvent->resistAmount;
	record->blockAmount = damageSplitEvent->blockAmount;
	record->missedCount = 0;
	record->missedAmount = 0;

	setSummaryDamageAbsorbed(record, damageSplitEvent->absorbed,
			damageSplitEvent->damageAmount,
			damageSplitEvent->overkillAmount,
			damageSplitEvent->resistAmount,
			damageSplitEvent->blockAmount);

	setSummaryDamageCritical(record, damageSplitEvent->wasCritical,
			damageSplitEvent->damageAmount,
			damageSplitEvent->overkillAmount,
			damageSplitEvent->resistAmount,
			damageSplitEvent->blockAmount);

	setSummaryDamageGlancing(record, damageSplitEvent->glancing,
			damageSplitEvent->damageAmount,
			damageSplitEvent->overkillAmount,
			damageSplitEvent->resistAmount,
			damageSplitEvent->blockAmount);

	setSummaryDamageCrushing(record, damageSplitEvent->crushing,
			damageSplitEvent->damageAmount,
			damageSplitEvent->overkillAmount,
			damageSplitEvent->resistAmount,
			damageSplitEvent->blockAmount);
}

static void initSummaryDamageFromEnvironmentalDamage(WowSummaryDamage *record,
		WowEvent *event)
{
	WowEventEnvironmentalDamage *envDamageEvent = NULL;

	envDamageEvent = (WowEventEnvironmentalDamage *)event->eventValue;

	record->damageType = strdup(envDamageEvent->damageType);
	record->directCount = 1;
	record->periodicCount = 0;
	record->damageAmount = envDamageEvent->damageAmount;
	record->overkillAmount = envDamageEvent->overkillAmount;
	record->resistAmount = envDamageEvent->resistAmount;
	record->blockAmount = envDamageEvent->blockAmount;
	record->missedCount = 0;
	record->missedAmount = 0;

	setSummaryDamageAbsorbed(record, envDamageEvent->absorbed,
			envDamageEvent->damageAmount,
			envDamageEvent->overkillAmount,
			envDamageEvent->resistAmount,
			envDamageEvent->blockAmount);

	setSummaryDamageCritical(record, envDamageEvent->wasCritical,
			envDamageEvent->damageAmount,
			envDamageEvent->overkillAmount,
			envDamageEvent->resistAmount,
			envDamageEvent->blockAmount);

	setSummaryDamageGlancing(record, envDamageEvent->glancing,
			envDamageEvent->damageAmount,
			envDamageEvent->overkillAmount,
			envDamageEvent->resistAmount,
			envDamageEvent->blockAmount);

	setSummaryDamageCrushing(record, envDamageEvent->crushing,
			envDamageEvent->damageAmount,
			envDamageEvent->overkillAmount,
			envDamageEvent->resistAmount,
			envDamageEvent->blockAmount);
}

static void initSummaryDamageFromRangeDamage(WowSummaryDamage *record,
		WowEvent *event)
{
	WowEventRangeDamage *rangeDamageEvent = NULL;

	rangeDamageEvent = (WowEventRangeDamage *)event->eventValue;

	record->spellId = rangeDamageEvent->spellId;
	record->spellName = strdup(rangeDamageEvent->spellName);
	record->spellSchool = strdup(getSpellSchool(
				rangeDamageEvent->spellSchool));
	record->directCount = 1;
	record->periodicCount = 0;
	record->damageAmount = rangeDamageEvent->damageAmount;
	record->overkillAmount = rangeDamageEvent->overkillAmount;
	record->resistAmount = rangeDamageEvent->resistAmount;
	record->blockAmount = rangeDamageEvent->blockAmount;
	record->missedCount = 0;
	record->missedAmount = 0;

	setSummaryDamageAbsorbed(record, rangeDamageEvent->absorbed,
			rangeDamageEvent->damageAmount,
			rangeDamageEvent->overkillAmount,
			rangeDamageEvent->resistAmount,
			rangeDamageEvent->blockAmount);

	setSummaryDamageCritical(record, rangeDamageEvent->wasCritical,
			rangeDamageEvent->damageAmount,
			rangeDamageEvent->overkillAmount,
			rangeDamageEvent->resistAmount,
			rangeDamageEvent->blockAmount);

	setSummaryDamageGlancing(record, rangeDamageEvent->glancing,
			rangeDamageEvent->damageAmount,
			rangeDamageEvent->overkillAmount,
			rangeDamageEvent->resistAmount,
			rangeDamageEvent->blockAmount);

	setSummaryDamageCrushing(record, rangeDamageEvent->crushing,
			rangeDamageEvent->damageAmount,
			rangeDamageEvent->overkillAmount,
			rangeDamageEvent->resistAmount,
			rangeDamageEvent->blockAmount);
}

static void initSummaryDamageFromRangeMissed(WowSummaryDamage *record,
		WowEvent *event)
{
	WowEventRangeMissed *rangeMissedEvent = NULL;

	rangeMissedEvent = (WowEventRangeMissed *)event->eventValue;

	record->spellId = rangeMissedEvent->spellId;
	record->spellName = strdup(rangeMissedEvent->spellName);
	record->spellSchool = strdup(getSpellSchool(
				rangeMissedEvent->spellSchool));
	record->directCount = 0;
	record->periodicCount = 0;
	record->damageAmount = 0;
	record->overkillAmount = 0;
	record->resistAmount = 0;
	record->blockAmount = 0;
	record->missedCount = 1;
	record->missedAmount = rangeMissedEvent->missAmount;

	setSummaryDamageAbsorbed(record, afalse, 0, 0, 0, 0);
	setSummaryDamageCritical(record, afalse, 0, 0, 0, 0);
	setSummaryDamageGlancing(record, afalse, 0, 0, 0, 0);
	setSummaryDamageCrushing(record, afalse, 0, 0, 0, 0);
}

static void initSummaryDamageFromSpellDamage(WowSummaryDamage *record,
		WowEvent *event)
{
	WowEventSpellDamage *spellDamageEvent = NULL;

	spellDamageEvent = (WowEventSpellDamage *)event->eventValue;

	record->spellId = spellDamageEvent->spellId;
	record->spellName = strdup(spellDamageEvent->spellName);
	record->spellSchool = strdup(getSpellSchool(
				spellDamageEvent->spellSchool));
	record->directCount = 1;
	record->periodicCount = 0;
	record->damageAmount = spellDamageEvent->damageAmount;
	record->overkillAmount = spellDamageEvent->overkillAmount;
	record->resistAmount = spellDamageEvent->resistAmount;
	record->blockAmount = spellDamageEvent->blockAmount;
	record->missedCount = 0;
	record->missedAmount = 0;

	setSummaryDamageAbsorbed(record, spellDamageEvent->absorbed,
			spellDamageEvent->damageAmount,
			spellDamageEvent->overkillAmount,
			spellDamageEvent->resistAmount,
			spellDamageEvent->blockAmount);

	setSummaryDamageCritical(record, spellDamageEvent->wasCritical,
			spellDamageEvent->damageAmount,
			spellDamageEvent->overkillAmount,
			spellDamageEvent->resistAmount,
			spellDamageEvent->blockAmount);

	setSummaryDamageGlancing(record, spellDamageEvent->glancing,
			spellDamageEvent->damageAmount,
			spellDamageEvent->overkillAmount,
			spellDamageEvent->resistAmount,
			spellDamageEvent->blockAmount);

	setSummaryDamageCrushing(record, spellDamageEvent->crushing,
			spellDamageEvent->damageAmount,
			spellDamageEvent->overkillAmount,
			spellDamageEvent->resistAmount,
			spellDamageEvent->blockAmount);
}

static void initSummaryDamageFromSpellPeriodicDamage(WowSummaryDamage *record,
		WowEvent *event)
{
	WowEventSpellPeriodicDamage *spellPeriodicDamageEvent = NULL;

	spellPeriodicDamageEvent =
		(WowEventSpellPeriodicDamage *)event->eventValue;

	record->spellId = spellPeriodicDamageEvent->spellId;
	record->spellName = strdup(spellPeriodicDamageEvent->spellName);
	record->spellSchool = strdup(getSpellSchool(
				spellPeriodicDamageEvent->spellSchool));
	record->directCount = 0;
	record->periodicCount = 1;
	record->damageAmount = spellPeriodicDamageEvent->damageAmount;
	record->overkillAmount = spellPeriodicDamageEvent->overkillAmount;
	record->resistAmount = spellPeriodicDamageEvent->resistAmount;
	record->blockAmount = spellPeriodicDamageEvent->blockAmount;
	record->missedCount = 0;
	record->missedAmount = 0;

	setSummaryDamageAbsorbed(record, spellPeriodicDamageEvent->absorbed,
			spellPeriodicDamageEvent->damageAmount,
			spellPeriodicDamageEvent->overkillAmount,
			spellPeriodicDamageEvent->resistAmount,
			spellPeriodicDamageEvent->blockAmount);

	setSummaryDamageCritical(record, spellPeriodicDamageEvent->wasCritical,
			spellPeriodicDamageEvent->damageAmount,
			spellPeriodicDamageEvent->overkillAmount,
			spellPeriodicDamageEvent->resistAmount,
			spellPeriodicDamageEvent->blockAmount);

	setSummaryDamageGlancing(record, spellPeriodicDamageEvent->glancing,
			spellPeriodicDamageEvent->damageAmount,
			spellPeriodicDamageEvent->overkillAmount,
			spellPeriodicDamageEvent->resistAmount,
			spellPeriodicDamageEvent->blockAmount);

	setSummaryDamageCrushing(record, spellPeriodicDamageEvent->crushing,
			spellPeriodicDamageEvent->damageAmount,
			spellPeriodicDamageEvent->overkillAmount,
			spellPeriodicDamageEvent->resistAmount,
			spellPeriodicDamageEvent->blockAmount);
}

static void initSummaryDamageFromSpellExtraAttacks(WowSummaryDamage *record,
		WowEvent *event)
{
	WowEventSpellExtraAttacks *spellExtraAttacksEvent = NULL;

	spellExtraAttacksEvent = (WowEventSpellExtraAttacks *)event->eventValue;

	record->spellId = spellExtraAttacksEvent->spellId;
	record->spellName = strdup(spellExtraAttacksEvent->spellName);
	record->spellSchool = strdup(getSpellSchool(
				spellExtraAttacksEvent->spellSchool));
	record->directCount = 1;
	record->periodicCount = 0;
	record->damageAmount = spellExtraAttacksEvent->extraAttackAmount;
	record->overkillAmount = 0;
	record->resistAmount = 0;
	record->blockAmount = 0;
	record->missedCount = 0;
	record->missedAmount = 0;

	setSummaryDamageAbsorbed(record, afalse, 0, 0, 0, 0);
	setSummaryDamageCritical(record, afalse, 0, 0, 0, 0);
	setSummaryDamageGlancing(record, afalse, 0, 0, 0, 0);
	setSummaryDamageCrushing(record, afalse, 0, 0, 0, 0);
}

static void initSummaryDamageFromSpellMissed(WowSummaryDamage *record,
		WowEvent *event)
{
	WowEventSpellMissed *spellMissedEvent = NULL;

	spellMissedEvent = (WowEventSpellMissed *)event->eventValue;

	record->spellId = spellMissedEvent->spellId;
	record->spellName = strdup(spellMissedEvent->spellName);
	record->spellSchool = strdup(getSpellSchool(
				spellMissedEvent->spellSchool));
	record->directCount = 0;
	record->periodicCount = 0;
	record->damageAmount = 0;
	record->overkillAmount = 0;
	record->resistAmount = 0;
	record->blockAmount = 0;
	record->missedCount = 1;
	record->missedAmount = spellMissedEvent->missAmount;

	setSummaryDamageAbsorbed(record, afalse, 0, 0, 0, 0);
	setSummaryDamageCritical(record, afalse, 0, 0, 0, 0);
	setSummaryDamageGlancing(record, afalse, 0, 0, 0, 0);
	setSummaryDamageCrushing(record, afalse, 0, 0, 0, 0);
}

static void initSummaryDamageFromSpellPeriodicMissed(WowSummaryDamage *record,
		WowEvent *event)
{
	WowEventSpellPeriodicMissed *spellPeriodicMissedEvent = NULL;

	spellPeriodicMissedEvent =
		(WowEventSpellPeriodicMissed *)event->eventValue;

	record->spellId = spellPeriodicMissedEvent->spellId;
	record->spellName = strdup(spellPeriodicMissedEvent->spellName);
	record->spellSchool = strdup(getSpellSchool(
				spellPeriodicMissedEvent->spellSchool));
	record->directCount = 0;
	record->periodicCount = 0;
	record->damageAmount = 0;
	record->overkillAmount = 0;
	record->resistAmount = 0;
	record->blockAmount = 0;
	record->missedCount = 1;
	record->missedAmount = spellPeriodicMissedEvent->missAmount;

	setSummaryDamageAbsorbed(record, afalse, 0, 0, 0, 0);
	setSummaryDamageCritical(record, afalse, 0, 0, 0, 0);
	setSummaryDamageGlancing(record, afalse, 0, 0, 0, 0);
	setSummaryDamageCrushing(record, afalse, 0, 0, 0, 0);
}

static void initSummaryDamageFromSwingDamage(WowSummaryDamage *record,
		WowEvent *event)
{
	WowEventSwingDamage *swingDamageEvent = NULL;

	swingDamageEvent = (WowEventSwingDamage *)event->eventValue;

	record->directCount = 1;
	record->periodicCount = 0;
	record->damageAmount = swingDamageEvent->damageAmount;
	record->overkillAmount = swingDamageEvent->overkillAmount;
	record->resistAmount = swingDamageEvent->resistAmount;
	record->blockAmount = swingDamageEvent->blockAmount;
	record->missedCount = 0;
	record->missedAmount = 0;

	setSummaryDamageAbsorbed(record, swingDamageEvent->absorbed,
			swingDamageEvent->damageAmount,
			swingDamageEvent->overkillAmount,
			swingDamageEvent->resistAmount,
			swingDamageEvent->blockAmount);

	setSummaryDamageCritical(record, swingDamageEvent->wasCritical,
			swingDamageEvent->damageAmount,
			swingDamageEvent->overkillAmount,
			swingDamageEvent->resistAmount,
			swingDamageEvent->blockAmount);

	setSummaryDamageGlancing(record, swingDamageEvent->glancing,
			swingDamageEvent->damageAmount,
			swingDamageEvent->overkillAmount,
			swingDamageEvent->resistAmount,
			swingDamageEvent->blockAmount);

	setSummaryDamageCrushing(record, swingDamageEvent->crushing,
			swingDamageEvent->damageAmount,
			swingDamageEvent->overkillAmount,
			swingDamageEvent->resistAmount,
			swingDamageEvent->blockAmount);
}

static void initSummaryDamageFromSwingMissed(WowSummaryDamage *record,
		WowEvent *event)
{
	WowEventSwingMissed *swingMissedEvent = NULL;

	swingMissedEvent = (WowEventSwingMissed *)event->eventValue;

	record->directCount = 0;
	record->periodicCount = 0;
	record->damageAmount = 0;
	record->overkillAmount = 0;
	record->resistAmount = 0;
	record->blockAmount = 0;
	record->missedCount = 1;
	record->missedAmount = swingMissedEvent->missAmount;

	setSummaryDamageAbsorbed(record, afalse, 0, 0, 0, 0);
	setSummaryDamageCritical(record, afalse, 0, 0, 0, 0);
	setSummaryDamageGlancing(record, afalse, 0, 0, 0, 0);
	setSummaryDamageCrushing(record, afalse, 0, 0, 0, 0);
}

static WowSummaryDamage *newSummaryDamage(WowSpellindex *spellIndex,
		WowEvent *event)
{
	int spellId = 0;
	int spellRank = 0;

	WowSpellList *spell = NULL;
	WowSummaryDamage *result = NULL;

	if((event->eventId != WOW_EVENT_DAMAGE_SHIELD) &&
			(event->eventId != WOW_EVENT_DAMAGE_SHIELD_MISSED) &&
			(event->eventId != WOW_EVENT_DAMAGE_SPLIT) &&
			(event->eventId != WOW_EVENT_ENVIRONMENTAL_DAMAGE) &&
			(event->eventId != WOW_EVENT_RANGE_DAMAGE) &&
			(event->eventId != WOW_EVENT_RANGE_MISSED) &&
			(event->eventId != WOW_EVENT_SPELL_DAMAGE) &&
			(event->eventId != WOW_EVENT_SPELL_PERIODIC_DAMAGE) &&
			(event->eventId != WOW_EVENT_SPELL_EXTRA_ATTACKS) &&
			(event->eventId != WOW_EVENT_SPELL_MISSED) &&
			(event->eventId != WOW_EVENT_SPELL_PERIODIC_MISSED) &&
			(event->eventId != WOW_EVENT_SWING_DAMAGE) &&
			(event->eventId != WOW_EVENT_SWING_MISSED)) {
		return NULL;
	}

	switch(event->eventId) {
		case WOW_EVENT_DAMAGE_SHIELD:
			spellId = ((WowEventDamageShield *)event->eventValue)->spellId;
			break;
		case WOW_EVENT_DAMAGE_SHIELD_MISSED:
			spellId =
				((WowEventDamageShieldMissed *)event->eventValue)->spellId;
			break;
		case WOW_EVENT_DAMAGE_SPLIT:
			spellId = ((WowEventDamageSplit *)event->eventValue)->spellId;
			break;
		case WOW_EVENT_RANGE_DAMAGE:
			spellId = ((WowEventRangeDamage *)event->eventValue)->spellId;
			break;
		case WOW_EVENT_RANGE_MISSED:
			spellId = ((WowEventRangeMissed *)event->eventValue)->spellId;
			break;
		case WOW_EVENT_SPELL_DAMAGE:
			spellId = ((WowEventSpellDamage *)event->eventValue)->spellId;
			break;
		case WOW_EVENT_SPELL_PERIODIC_DAMAGE:
			spellId =
				((WowEventSpellPeriodicDamage *)event->eventValue)->spellId;
			break;
		case WOW_EVENT_SPELL_EXTRA_ATTACKS:
			spellId =
				((WowEventSpellExtraAttacks *)event->eventValue)->spellId;
			break;
		case WOW_EVENT_SPELL_MISSED:
			spellId = ((WowEventSpellMissed *)event->eventValue)->spellId;
			break;
		case WOW_EVENT_SPELL_PERIODIC_MISSED:
			spellId =
				((WowEventSpellPeriodicMissed *)event->eventValue)->spellId;
			break;
		default:
			spellId = -1;
	}

	if(spellId != -1) {
		if(wowstats_spellindexLookup(spellIndex, spellId, &spell) < 0) {
			spellRank = 0;
		}
		else {
			spellRank = spell->rank;
		}
	}

	result = (WowSummaryDamage *)malloc(sizeof(WowSummaryDamage));

	if(spellId != -1) {
		result->spellId = spellId;
		result->spellRank = spellRank;
	}
	else {
		result->spellId = 0;
		result->spellRank = 0;

		switch(event->eventId) {
			case WOW_EVENT_ENVIRONMENTAL_DAMAGE:
				result->spellName = strdup("Damage");
				result->spellSchool = strdup("Environmental");
				break;
			case WOW_EVENT_SWING_DAMAGE:
			case WOW_EVENT_SWING_MISSED:
				result->spellName = strdup("Melee");
				result->spellSchool = strdup("Physical");
				break;
		}
	}

	switch(event->eventId) {
		case WOW_EVENT_DAMAGE_SHIELD:
			result->damageType = strdup("Shield");
			initSummaryDamageFromDamageShield(result, event);
			break;
		case WOW_EVENT_DAMAGE_SHIELD_MISSED:
			result->damageType = strdup("Shield");
			initSummaryDamageFromDamageShieldMissed(result, event);
			break;
		case WOW_EVENT_DAMAGE_SPLIT:
			result->damageType = strdup("Split");
			initSummaryDamageFromDamageSplit(result, event);
			break;
		case WOW_EVENT_ENVIRONMENTAL_DAMAGE:
			initSummaryDamageFromEnvironmentalDamage(result, event);
			break;
		case WOW_EVENT_RANGE_DAMAGE:
			result->damageType = strdup("Physical");
			initSummaryDamageFromRangeDamage(result, event);
			break;
		case WOW_EVENT_RANGE_MISSED:
			result->damageType = strdup("Physical");
			initSummaryDamageFromRangeMissed(result, event);
			break;
		case WOW_EVENT_SPELL_DAMAGE:
			result->damageType = strdup("Spell");
			initSummaryDamageFromSpellDamage(result, event);
			break;
		case WOW_EVENT_SPELL_PERIODIC_DAMAGE:
			result->damageType = strdup("Spell");
			initSummaryDamageFromSpellPeriodicDamage(result, event);
			break;
		case WOW_EVENT_SPELL_EXTRA_ATTACKS:
			result->damageType = strdup("Spell");
			initSummaryDamageFromSpellExtraAttacks(result, event);
			break;
		case WOW_EVENT_SPELL_MISSED:
			result->damageType = strdup("Spell");
			initSummaryDamageFromSpellMissed(result, event);
			break;
		case WOW_EVENT_SPELL_PERIODIC_MISSED:
			result->damageType = strdup("Spell");
			initSummaryDamageFromSpellPeriodicMissed(result, event);
			break;
		case WOW_EVENT_SWING_DAMAGE:
			result->damageType = strdup("Physical");
			initSummaryDamageFromSwingDamage(result, event);
			break;
		case WOW_EVENT_SWING_MISSED:
			result->damageType = strdup("Physical");
			initSummaryDamageFromSwingMissed(result, event);
			break;
	}

	if(result->damageType == NULL) {
		result->damageType = strdup("n/a");
	}

	return result;
}

static WowSummaryDamage *newSummaryDamageFromRecord(
		WowSummaryDamage *record)
{
	WowSummaryDamage *result = NULL;

	result = (WowSummaryDamage *)malloc(sizeof(WowSummaryDamage));

	result->spellId = record->spellId;
	result->spellName = strdup(record->spellName);
	result->spellSchool = strdup(record->spellSchool);
	result->spellRank = record->spellRank;
	result->damageType = strdup(record->damageType);
	result->directCount = record->directCount;
	result->periodicCount = record->periodicCount;
	result->damageAmount = record->damageAmount;
	result->overkillAmount = record->overkillAmount;
	result->resistAmount = record->resistAmount;
	result->blockAmount = record->blockAmount;
	result->missedCount = record->missedCount;
	result->missedAmount = record->missedAmount;

	result->absorbedCount = record->absorbedCount;
	result->absorbedDamageAmount = record->absorbedDamageAmount;
	result->absorbedOverkillAmount = record->absorbedOverkillAmount;
	result->absorbedResistAmount = record->absorbedResistAmount;
	result->absorbedBlockAmount = record->absorbedBlockAmount;

	result->criticalCount = record->criticalCount;
	result->criticalDamageAmount = record->criticalDamageAmount;
	result->criticalOverkillAmount = record->criticalOverkillAmount;
	result->criticalResistAmount = record->criticalResistAmount;
	result->criticalBlockAmount = record->criticalBlockAmount;

	result->glancingCount = record->glancingCount;
	result->glancingDamageAmount = record->glancingDamageAmount;
	result->glancingOverkillAmount = record->glancingOverkillAmount;
	result->glancingResistAmount = record->glancingResistAmount;
	result->glancingBlockAmount = record->glancingBlockAmount;

	result->crushingCount = record->crushingCount;
	result->crushingDamageAmount = record->crushingDamageAmount;
	result->crushingOverkillAmount = record->crushingOverkillAmount;
	result->crushingResistAmount = record->crushingResistAmount;
	result->crushingBlockAmount = record->crushingBlockAmount;

	return result;
}

static void prependSummaryDamageName(WowSummaryDamage *record,
		char *sourceName)
{
	int length = 0;
	char *spellName = NULL;

	if(record->spellName == NULL) {
		record->spellName = strdup("Unknown");
	}

	length = (strlen(sourceName) + strlen(record->spellName) + 8);
	spellName = (char *)malloc(sizeof(char) * (length + 1));

	snprintf(spellName, length, "%s: %s", sourceName, record->spellName);

	free(record->spellName);

	record->spellName = spellName;
}

static void freeSummaryDamage(WowSummary *summary)
{
	WowSummaryDamage *record = NULL;

	record = (WowSummaryDamage *)summary->record;

	free(record->spellName);
	free(record->spellSchool);
	free(record->damageType);
	free(record);
}

static char *buildSummaryDamageKey(WowSummary *summary)
{
	char buffer[8192];

	WowSummaryDamage *record = NULL;

	record = (WowSummaryDamage *)summary->record;

	snprintf(buffer, (sizeof(buffer) - 1),
			"/i%i/%s/%s/%i/%s/%s",
			summary->summaryId,
			summary->sourceUid,
			summary->targetUid,
			record->spellId,
			record->spellName,
			record->damageType);

	return strdup(buffer);
}

static char *buildSummaryDamageOverrideKey(WowSummary *summary,
		char *sourceUid)
{
	char buffer[8192];

	WowSummaryDamage *record = NULL;

	record = (WowSummaryDamage *)summary->record;

	snprintf(buffer, (sizeof(buffer) - 1),
			"/p%i/%s/%s/%i/%s/%s",
			summary->summaryId,
			sourceUid,
			summary->targetUid,
			record->spellId,
			record->spellName,
			record->damageType);

	return strdup(buffer);
}

static int updateSummaryDamage(WowSummary *summary, WowSummary *newSummary)
{
	WowSummaryDamage *record = NULL;
	WowSummaryDamage *newRecord = NULL;

	if(summary->summaryId != newSummary->summaryId) {
		return WOW_SUMMARY_RECORD_MISMATCH;
	}

	record = (WowSummaryDamage *)summary->record;
	newRecord = (WowSummaryDamage *)newSummary->record;

	if(record->spellId != newRecord->spellId) {
		return WOW_SUMMARY_RECORD_MISMATCH;
	}

	if(newRecord->directCount > 0) {
		record->directCount += newRecord->directCount;
	}

	if(newRecord->periodicCount > 0) {
		record->periodicCount += newRecord->periodicCount;
	}

	if(newRecord->damageAmount > 0) {
		record->damageAmount += newRecord->damageAmount;
	}

	if(newRecord->overkillAmount > 0) {
		record->overkillAmount += newRecord->overkillAmount;
	}

	if(newRecord->resistAmount > 0) {
		record->resistAmount += newRecord->resistAmount;
	}

	if(newRecord->blockAmount > 0) {
		record->blockAmount += newRecord->blockAmount;
	}

	if(newRecord->missedCount > 0) {
		record->missedCount += newRecord->missedCount;
	}

	if(newRecord->missedAmount > 0) {
		record->missedAmount += newRecord->missedAmount;
	}

	if(newRecord->absorbedCount > 0) {
		record->absorbedCount += newRecord->absorbedCount;
		record->absorbedDamageAmount += newRecord->absorbedDamageAmount;
		record->absorbedOverkillAmount += newRecord->absorbedOverkillAmount;
		record->absorbedResistAmount += newRecord->absorbedResistAmount;
		record->absorbedBlockAmount += newRecord->absorbedBlockAmount;
	}

	if(newRecord->criticalCount > 0) {
		record->criticalCount += newRecord->criticalCount;
		record->criticalDamageAmount += newRecord->criticalDamageAmount;
		record->criticalOverkillAmount += newRecord->criticalOverkillAmount;
		record->criticalResistAmount += newRecord->criticalResistAmount;
		record->criticalBlockAmount += newRecord->criticalBlockAmount;
	}

	if(newRecord->glancingCount > 0) {
		record->glancingCount += newRecord->glancingCount;
		record->glancingDamageAmount += newRecord->glancingDamageAmount;
		record->glancingOverkillAmount += newRecord->glancingOverkillAmount;
		record->glancingResistAmount += newRecord->glancingResistAmount;
		record->glancingBlockAmount += newRecord->glancingBlockAmount;
	}

	if(newRecord->crushingCount > 0) {
		record->crushingCount += newRecord->crushingCount;
		record->crushingDamageAmount += newRecord->crushingDamageAmount;
		record->crushingOverkillAmount += newRecord->crushingOverkillAmount;
		record->crushingResistAmount += newRecord->crushingResistAmount;
		record->crushingBlockAmount += newRecord->crushingBlockAmount;
	}

	return 0;
}

static void displaySummaryDamage(WowSummary *summary)
{
	int seconds = 0;

	WowSummaryDamage *record = NULL;

	if(wowstats_timestampCalculateDifferenceInSeconds(
				&summary->lastTimestamp,
				&summary->firstTimestamp,
				&seconds) < 0) {
		return;
	}

	record = (WowSummaryDamage *)summary->record;

	printf("[%s]{'%s'->'%s'} cast '%s' rank %i school '%s' type '%s', "
			"%i direct %i periodic, for %i damage, %i over-kill, %i resist, "
			"%i block, %i misses, %i missed, %i absorbed, %i critical, "
			"%i glancing, %i crushing over %i seconds\n",
			WOW_SUMMARY_LIST[(summary->summaryId - 1)].name, summary->source,
			summary->target, record->spellName, record->spellRank,
			record->spellSchool, record->damageType, record->directCount,
			record->periodicCount, record->damageAmount,
			record->overkillAmount, record->resistAmount, record->blockAmount,
			record->missedCount, record->missedAmount, record->absorbedCount,
			record->criticalCount, record->glancingCount,
			record->crushingCount, seconds);
}

// define wowstats summarization engine public functions

int wowstats_summaryInit(WowSummary *summary, WowSpellindex *spellIndex,
		WowEvent *event)
{
	if((summary == NULL) || (spellIndex == NULL) || (event == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	memset(summary, 0, sizeof(WowSummary));

	summary->summaryId = WOW_SUMMARY_END_OF_LIST;

	switch(event->eventId) {
		case WOW_EVENT_SPELL_HEAL:
		case WOW_EVENT_SPELL_PERIODIC_HEAL:
		case WOW_EVENT_SPELL_LEECH:
			summary->summaryId = WOW_SUMMARY_HEALING;
			summary->record = newSummaryHealing(spellIndex, event);
			break;

		case WOW_EVENT_DAMAGE_SHIELD:
		case WOW_EVENT_DAMAGE_SHIELD_MISSED:
		case WOW_EVENT_DAMAGE_SPLIT:
		case WOW_EVENT_ENVIRONMENTAL_DAMAGE:
		case WOW_EVENT_RANGE_DAMAGE:
		case WOW_EVENT_RANGE_MISSED:
		case WOW_EVENT_SPELL_DAMAGE:
		case WOW_EVENT_SPELL_PERIODIC_DAMAGE:
		case WOW_EVENT_SPELL_EXTRA_ATTACKS:
		case WOW_EVENT_SPELL_MISSED:
		case WOW_EVENT_SPELL_PERIODIC_MISSED:
		case WOW_EVENT_SWING_DAMAGE:
		case WOW_EVENT_SWING_MISSED:
			summary->summaryId = WOW_SUMMARY_DAMAGE;
			summary->record = newSummaryDamage(spellIndex, event);
			break;

		default:
			wowstats_summaryFree(summary);
			return WOW_SUMMARY_EVENT_NOT_APPLICABLE;
	}

	if(summary->record == NULL) {
		wowstats_summaryFree(summary);
		return WOW_SUMMARY_BAD_EVENT;
	}

	if(wowstats_timestampClone(&summary->firstTimestamp,
				&event->timestamp) < 0) {
		wowstats_summaryFree(summary);
		return WOW_SUMMARY_BAD_EVENT;
	}

	if(wowstats_timestampClone(&summary->lastTimestamp,
				&event->timestamp) < 0) {
		wowstats_summaryFree(summary);
		return WOW_SUMMARY_BAD_EVENT;
	}

	summary->source = strdup(event->sourceName);
	summary->sourceUid = strdup(event->sourceUid);
	summary->target = strdup(event->targetName);
	summary->targetUid = strdup(event->targetUid);
	summary->extraElapsedTimeSeconds = 0;

	return 0;
}

int wowstats_summaryNew(WowSummary **summary, WowSpellindex *spellIndex,
		WowEvent *event)
{
	if((summary == NULL) || (spellIndex == NULL) || (event == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	*summary = (WowSummary *)malloc(sizeof(WowSummary));

	return wowstats_summaryInit(*summary, spellIndex, event);
}

int wowstats_summaryFree(WowSummary *summary)
{
	int result = 0;

	if(summary == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(summary->record != NULL) {
		switch(summary->summaryId) {
			case WOW_SUMMARY_HEALING:
				freeSummaryHealing(summary);
				break;
			case WOW_SUMMARY_DAMAGE:
				freeSummaryDamage(summary);
				break;
			default:
				result = WOW_SUMMARY_INVALID_RECORD;
		}
	}

	if(summary->source != NULL) {
		free(summary->source);
	}

	if(summary->sourceUid != NULL) {
		free(summary->sourceUid);
	}

	if(summary->target != NULL) {
		free(summary->target);
	}

	if(summary->targetUid != NULL) {
		free(summary->targetUid);
	}

	memset(summary, 0, sizeof(WowSummary));

	return result;
}

int wowstats_summaryFreePtr(WowSummary *summary)
{
	int rc = 0;

	if((rc = wowstats_summaryFree(summary)) < 0) {
		return rc;
	}

	free(summary);

	return 0;
}

int wowstats_summaryClone(WowSummary *summary, WowSummary **newSummary)
{
	WowSummary *localSummary = NULL;

	if((summary == NULL) || (summary->record == NULL) ||
			(newSummary == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	localSummary = (WowSummary *)malloc(sizeof(WowSummary));

	localSummary->summaryId = summary->summaryId;

	switch(localSummary->summaryId) {
		case WOW_SUMMARY_HEALING:
			localSummary->record = newSummaryHealingFromRecord(summary->record);
			break;
		case WOW_SUMMARY_DAMAGE:
			localSummary->record = newSummaryDamageFromRecord(summary->record);
			break;
		default:
			wowstats_summaryFree(localSummary);
			return WOW_SUMMARY_INVALID_RECORD;
	}

	if(localSummary->record == NULL) {
		wowstats_summaryFree(localSummary);
		return WOW_SUMMARY_INVALID_RECORD;
	}

	if(wowstats_timestampClone(&localSummary->firstTimestamp,
				&summary->firstTimestamp) < 0) {
		wowstats_summaryFree(localSummary);
		return WOW_SUMMARY_INVALID_SUMMARY;
	}

	if(wowstats_timestampClone(&localSummary->lastTimestamp,
				&summary->lastTimestamp) < 0) {
		wowstats_summaryFree(localSummary);
		return WOW_SUMMARY_INVALID_SUMMARY;
	}

	localSummary->source = strdup(summary->source);
	localSummary->sourceUid = strdup(summary->sourceUid);
	localSummary->target = strdup(summary->target);
	localSummary->targetUid = strdup(summary->targetUid);

	*newSummary = localSummary;

	return 0;
}

int wowstats_summaryPrependName(WowSummary *summary, char *sourceName)
{
	if((summary == NULL) || (summary->record == NULL) ||
			(sourceName == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	switch(summary->summaryId) {
		case WOW_SUMMARY_HEALING:
			prependSummaryHealingName(summary->record, sourceName);
			break;
		case WOW_SUMMARY_DAMAGE:
			prependSummaryDamageName(summary->record, sourceName);
			break;
		default:
			return WOW_SUMMARY_INVALID_RECORD;
	}

	return 0;
}

int wowstats_summaryLookup(WowSummary *summary, Pgdb *pgdb, int logHistoryId,
		int realmId, WowSummary **newSummary)
{
	WowSummary *localSummary = NULL;

	if((summary == NULL) || (summary->record == NULL) || (pgdb == NULL) ||
			(logHistoryId < 1) || (realmId < 1) || (newSummary == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	// clone the existing summary

	localSummary = (WowSummary *)malloc(sizeof(WowSummary));

	localSummary->summaryId = summary->summaryId;

	switch(localSummary->summaryId) {
		case WOW_SUMMARY_HEALING:
			localSummary->record = newSummaryHealingFromRecord(summary->record);
			break;
		case WOW_SUMMARY_DAMAGE:
			localSummary->record = newSummaryDamageFromRecord(summary->record);
			break;
		default:
			wowstats_summaryFree(localSummary);
			return WOW_SUMMARY_INVALID_RECORD;
	}

	if(localSummary->record == NULL) {
		wowstats_summaryFree(localSummary);
		return WOW_SUMMARY_INVALID_RECORD;
	}

	if(wowstats_timestampClone(&localSummary->firstTimestamp,
				&summary->firstTimestamp) < 0) {
		wowstats_summaryFree(localSummary);
		return WOW_SUMMARY_INVALID_SUMMARY;
	}

	if(wowstats_timestampClone(&localSummary->lastTimestamp,
				&summary->lastTimestamp) < 0) {
		wowstats_summaryFree(localSummary);
		return WOW_SUMMARY_INVALID_SUMMARY;
	}

	localSummary->source = strdup(summary->source);
	localSummary->sourceUid = strdup(summary->sourceUid);
	localSummary->target = strdup(summary->target);
	localSummary->targetUid = strdup(summary->targetUid);

	// lookup the parent summary from the database

	switch(localSummary->summaryId) {
		case WOW_SUMMARY_HEALING:
			if(wowstats_wsdalGetSummaryHealing(pgdb, logHistoryId,
						realmId, localSummary) < 0) {
				wowstats_summaryFree(localSummary);
				return WOW_SUMMARY_INVALID_SUMMARY;
			}
			break;
		case WOW_SUMMARY_DAMAGE:
			if(wowstats_wsdalGetSummaryDamage(pgdb, logHistoryId,
						realmId, localSummary) < 0) {
				wowstats_summaryFree(localSummary);
				return WOW_SUMMARY_INVALID_SUMMARY;
			}
			break;
		default:
			wowstats_summaryFree(localSummary);
			return WOW_SUMMARY_INVALID_SUMMARY;
	}

	*newSummary = localSummary;

	return 0;
}

int wowstats_summaryBuildEntityKey(WowSummary *summary, char **key,
		int *keyLength)
{
	if((summary == NULL) || (summary->record == NULL) || (key == NULL) ||
			(keyLength == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	*key = summary->sourceUid;
	*keyLength = strlen(*key);

	return 0;
}

int wowstats_summaryBuildKey(WowSummary *summary, char **key, int *keyLength)
{
	if((summary == NULL) || (summary->record == NULL) || (key == NULL) ||
			(keyLength == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	switch(summary->summaryId) {
		case WOW_SUMMARY_HEALING:
			*key = buildSummaryHealingKey(summary);
			break;
		case WOW_SUMMARY_DAMAGE:
			*key = buildSummaryDamageKey(summary);
			break;
		default:
			return WOW_SUMMARY_INVALID_SUMMARY;
	}

	if(*key == NULL) {
		return WOW_SUMMARY_INVALID_RECORD;
	}

	*keyLength = strlen(*key);

	return 0;
}

int wowstats_summaryBuildOverrideKey(WowSummary *summary, char *sourceUid,
		char **key, int *keyLength)
{
	if((summary == NULL) || (summary->record == NULL) ||
			(sourceUid == NULL) || (key == NULL) || (keyLength == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	switch(summary->summaryId) {
		case WOW_SUMMARY_HEALING:
			*key = buildSummaryHealingOverrideKey(summary, sourceUid);
			break;
		case WOW_SUMMARY_DAMAGE:
			*key = buildSummaryDamageOverrideKey(summary, sourceUid);
			break;
		default:
			return WOW_SUMMARY_INVALID_SUMMARY;
	}

	if(*key == NULL) {
		return WOW_SUMMARY_INVALID_RECORD;
	}

	*keyLength = strlen(*key);

	return 0;
}

int wowstats_summaryUpdateRecord(WowSummary *summary, WowSummary *newSummary)
{
	int rc = 0;
	int result = 0;

	if((summary == NULL) || (summary->record == NULL) || (newSummary == NULL) ||
			(newSummary->record == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	switch(summary->summaryId) {
		case WOW_SUMMARY_HEALING:
			result = updateSummaryHealing(summary, newSummary);
			break;
		case WOW_SUMMARY_DAMAGE:
			result = updateSummaryDamage(summary, newSummary);
			break;
		default:
			return WOW_SUMMARY_INVALID_SUMMARY;
	}

	if(result == 0) {
		if(newSummary->extraElapsedTimeSeconds > 0) {
			summary->extraElapsedTimeSeconds +=
				newSummary->extraElapsedTimeSeconds;
		}

		if(wowstats_timestampCompare(&summary->firstTimestamp,
					&newSummary->firstTimestamp, &rc) < 0) {
			return WOW_SUMMARY_INVALID_SUMMARY;
		}

		if(rc == WOW_TIMESTAMP_LESS_THAN) {
			if(wowstats_timestampClone(&summary->firstTimestamp,
						&newSummary->firstTimestamp) < 0) {
				return WOW_SUMMARY_INVALID_SUMMARY;
			}
		}

		if(wowstats_timestampCompare(&summary->lastTimestamp,
					&newSummary->lastTimestamp, &rc) < 0) {
			return WOW_SUMMARY_INVALID_SUMMARY;
		}

		if(rc == WOW_TIMESTAMP_GREATER_THAN) {
			if(wowstats_timestampClone(&summary->lastTimestamp,
						&newSummary->lastTimestamp) < 0) {
				return WOW_SUMMARY_INVALID_SUMMARY;
			}
		}
	}

	return result;
}

void wowstats_summaryDisplay(WowSummary *summary)
{
	if((summary == NULL) || (summary->record == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	switch(summary->summaryId) {
		case WOW_SUMMARY_HEALING:
			displaySummaryHealing(summary);
			break;
		case WOW_SUMMARY_DAMAGE:
			displaySummaryDamage(summary);
			break;
	}
}

