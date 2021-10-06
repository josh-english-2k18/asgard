/*
 * event.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard-WoW statistical analysis event-parsing library, header file.
 *
 * Written by Josh English.
 */

#if !defined(_WOWSTATS_CORE_EVENT_H)

#define _WOWSTATS_CORE_EVENT_H

#if !defined(_WOWSTATS_CORE_H) && !defined(_WOWSTATS_CORE_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _WOWSTATS_CORE_H || _WOWSTATS_CORE_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

// define wowstats event public constants

#define WOWSTATS_EVENT_INVALID_EVENT					-2
#define WOWSTATS_EVENT_UNKNOWN_EVENT					-3


// define wowstats event public data types

typedef struct _WowEventDamageShield {
	int spellId;
	char *spellName;
	int spellSchool;
	int damageAmount;
	int overkillAmount;
	int school;
	int resistAmount;
	int blockAmount;
	aboolean absorbed;
	aboolean wasCritical;
	aboolean glancing;
	aboolean crushing;
} WowEventDamageShield;

typedef struct _WowEventDamageShieldMissed {
	int spellId;
	char *spellName;
	int spellSchool;
	char *missType;
	int missAmount;
} WowEventDamageShieldMissed;

typedef struct _WowEventDamageSplit {
	int spellId;
	char *spellName;
	int spellSchool;
	int damageAmount;
	int overkillAmount;
	int school;
	int resistAmount;
	int blockAmount;
	aboolean absorbed;
	aboolean wasCritical;
	aboolean glancing;
	aboolean crushing;
} WowEventDamageSplit;

typedef struct _WowEventEnchantApplied {
	char *spellName;
	int itemId;
	char *itemName;
} WowEventEnchantApplied;

typedef struct _WowEventEnchantRemoved {
	char *spellName;
	int itemId;
	char *itemName;
} WowEventEnchantRemoved;

typedef struct _WowEventPartyKill {
	aboolean placeholder;
} WowEventPartyKill;

typedef struct _WowEventUnitDied {
	aboolean placeholder;
} WowEventUnitDied;

typedef struct _WowEventUnitDestroyed {
	aboolean placeholder;
} WowEventUnitDestroyed;

typedef struct _WowEventEnvironmentalDamage {
	char *damageType;
	int damageAmount;
	int overkillAmount;
	int school;
	int resistAmount;
	int blockAmount;
	aboolean absorbed;
	aboolean wasCritical;
	aboolean glancing;
	aboolean crushing;
} WowEventEnvironmentalDamage;

typedef struct _WowEventRangeDamage {
	int spellId;
	char *spellName;
	int spellSchool;
	int damageAmount;
	int overkillAmount;
	int school;
	int resistAmount;
	int blockAmount;
	aboolean absorbed;
	aboolean wasCritical;
	aboolean glancing;
	aboolean crushing;
} WowEventRangeDamage;

typedef struct _WowEventRangeMissed {
	int spellId;
	char *spellName;
	int spellSchool;
	char *missType;
	int missAmount;
} WowEventRangeMissed;

typedef struct _WowEventSpellAuraApplied {
	int spellId;
	char *spellName;
	int spellSchool;
	char *auraType;
} WowEventSpellAuraApplied;

typedef struct _WowEventSpellAuraAppliedDose {
	int spellId;
	char *spellName;
	int spellSchool;
	char *auraType;
	int auraAmount;
} WowEventSpellAuraAppliedDose;

typedef struct _WowEventSpellAuraBroken {
	int spellId;
	char *spellName;
	int spellSchool;
	char *auraType;
} WowEventSpellAuraBroken;

typedef struct _WowEventSpellAuraBrokenSpell {
	int spellId;
	char *spellName;
	int spellSchool;
	int breakingSpellId;
	char *breakingSpellName;
	int breakingSpellSchool;
	char *auraType;
} WowEventSpellAuraBrokenSpell;

typedef struct _WowEventSpellAuraRefresh {
	int spellId;
	char *spellName;
	int spellSchool;
	char *auraType;
} WowEventSpellAuraRefresh;

typedef struct _WowEventSpellAuraRemoved {
	int spellId;
	char *spellName;
	int spellSchool;
	char *auraType;
} WowEventSpellAuraRemoved;

typedef struct _WowEventSpellAuraRemovedDose {
	int spellId;
	char *spellName;
	int spellSchool;
	char *auraType;
	int auraAmount;
} WowEventSpellAuraRemovedDose;

typedef struct _WowEventSpellCastFailed {
	int spellId;
	char *spellName;
	int spellSchool;
	char *reason;
} WowEventSpellCastFailed;

typedef struct _WowEventSpellCastStart {
	int spellId;
	char *spellName;
	int spellSchool;
} WowEventSpellCastStart;

typedef struct _WowEventSpellCastSuccess {
	int spellId;
	char *spellName;
	int spellSchool;
} WowEventSpellCastSuccess;

typedef struct _WowEventSpellCreate {
	int spellId;
	char *spellName;
	int spellSchool;
} WowEventSpellCreate;

typedef struct _WowEventSpellDamage {
	int spellId;
	char *spellName;
	int spellSchool;
	int damageAmount;
	int overkillAmount;
	int school;
	int resistAmount;
	int blockAmount;
	aboolean absorbed;
	aboolean wasCritical;
	aboolean glancing;
	aboolean crushing;
} WowEventSpellDamage;

typedef struct _WowEventSpellDispel {
	int spellId;
	char *spellName;
	int spellSchool;
	int dispelSpellId;
	char *dispelSpellName;
	int dispelSpellSchool;
	char *auraType;
} WowEventSpellDispel;

typedef struct _WowEventSpellDispelFailed {
	int spellId;
	char *spellName;
	int spellSchool;
	int dispelSpellId;
	char *dispelSpellName;
	int dispelSpellSchool;
} WowEventSpellDispelFailed;

typedef struct _WowEventSpellEnergize {
	int spellId;
	char *spellName;
	int spellSchool;
	int energizeAmount;
	char *powerType;
	int extraEnergizeAmount;
} WowEventSpellEnergize;

typedef struct _WowEventSpellExtraAttacks {
	int spellId;
	char *spellName;
	int spellSchool;
	int extraAttackAmount;
} WowEventSpellExtraAttacks;

typedef struct _WowEventSpellHealed {
	int spellId;
	char *spellName;
	int spellSchool;
	int healAmount;
	int overHealingAmount;
	aboolean wasCritical;
} WowEventSpellHealed;

typedef struct _WowEventSpellInstakill {
	int spellId;
	char *spellName;
	int spellSchool;
} WowEventSpellInstakill;

typedef struct _WowEventSpellInterrupt {
	int spellId;
	char *spellName;
	int spellSchool;
	int interruptSpellId;
	char *interruptSpellName;
	int interruptSpellSchool;
} WowEventSpellInterrupt;

typedef struct _WowEventSpellLeech {
	int spellId;
	char *spellName;
	int spellSchool;
	int leechAmount;
	char *powerType;
	int extraLeechAmount;
} WowEventSpellLeech;

typedef struct _WowEventSpellMissed {
	int spellId;
	char *spellName;
	int spellSchool;
	char *missType;
	int missAmount;
} WowEventSpellMissed;

typedef struct _WowEventSpellResurrect {
	int spellId;
	char *spellName;
	int spellSchool;
} WowEventSpellResurrect;

typedef struct _WowEventSpellStolen {
	int spellId;
	char *spellName;
	int spellSchool;
	int stolenSpellId;
	char *stolenSpellName;
	int stolenSpellSchool;
	char *auraType;
} WowEventSpellStolen;

typedef struct _WowEventSpellSummon {
	int spellId;
	char *spellName;
	int spellSchool;
} WowEventSpellSummon;

typedef struct _WowEventSpellPeriodicDamage {
	int spellId;
	char *spellName;
	int spellSchool;
	int damageAmount;
	int overkillAmount;
	int school;
	int resistAmount;
	int blockAmount;
	aboolean absorbed;
	aboolean wasCritical;
	aboolean glancing;
	aboolean crushing;
} WowEventSpellPeriodicDamage;

typedef struct _WowEventSpellPeriodicEnergize {
	int spellId;
	char *spellName;
	int spellSchool;
	int energizeAmount;
	char *powerType;
	int extraEnergizeAmount;
} WowEventSpellPeriodicEnergize;

typedef struct _WowEventSpellPeriodicHealed {
	int spellId;
	char *spellName;
	int spellSchool;
	int healAmount;
	int overHealingAmount;
	aboolean wasCritical;
} WowEventSpellPeriodicHealed;

typedef struct _WowEventSpellPeriodicMissed {
	int spellId;
	char *spellName;
	int spellSchool;
	char *missType;
	int missAmount;
} WowEventSpellPeriodicMissed;

typedef struct _WowEventSwingDamage {
	int damageAmount;
	int overkillAmount;
	int school;
	int resistAmount;
	int blockAmount;
	aboolean absorbed;
	aboolean wasCritical;
	aboolean glancing;
	aboolean crushing;
} WowEventSwingDamage;

typedef struct _WowEventSwingMissed {
	char *missType;
	int missAmount;
} WowEventSwingMissed;

typedef struct _WowEvent {
	int eventId;
	char *sourceUid;
	char *sourceName;
	unsigned int sourceFlags;
	unsigned long long int sourceUnitType;
	char *targetUid;
	char *targetName;
	unsigned int targetFlags;
	unsigned long long int targetUnitType;
	WowTimestamp timestamp;
	void *eventValue;
} WowEvent;


// declare wowstats event public functions

int wowstats_eventInit(WowEvent *event, int year, WowParser *parser);

int wowstats_eventFree(WowEvent *event);

int wowstats_eventBuildKey(WowEvent *event, char **key, int *keyLength);

#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _WOWSTATS_CORE_EVENT_H

