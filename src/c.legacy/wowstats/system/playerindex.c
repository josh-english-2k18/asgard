/*
 * playerindex.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard-WoW statistical analysis player-index system.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#include "wowstats/common.h"
#define _WOWSTATS_DATA_COMPONENT
#include "wowstats/data/playerclass.h"
#include "wowstats/core.h"
#define _WOWSTATS_SYSTEM_COMPONENT
#include "wowstats/system/player.h"
#include "wowstats/system/playerindex.h"

// define wowstats player-index private constants

static WowSpellList WOW_PLAYERINDEX_BUFF_SPELL_LIST[] = {
	{	26888,		"Rogue",		"Vanish",			0		},
	{	58984,		"Unknown",		"Shadowmeld",		0		},
	{	-1,			NULL,			NULL,				0		}
};


// define wowstats player-index private functions

static WowPlayerSummary *newPlayer(char *name, char *uid);

static void freePlayer(void *memory);

static int lookupClassId(char *playerClass);

static aboolean isSpellInBuffList(WowEvent *event, aboolean *isApplied);


// define wowstats player-index private functions

static WowPlayerSummary *newPlayer(char *name, char *uid)
{
	WowPlayerSummary *result = NULL;

	result = (WowPlayerSummary *)malloc(sizeof(WowPlayerSummary));

	if(wowstats_playerInit(result, name, uid) < 0) {
		wowstats_playerFree(result);
		free(result);
		return NULL;
	}

	return result;
}

static void freePlayer(void *memory)
{
	WowPlayerSummary *player = NULL;

	if(memory == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	player = (WowPlayerSummary *)memory;

	if(wowstats_playerFree(player) < 0) {
		return;
	}

	free(player);
}

static int lookupClassId(char *playerClass)
{
	int ii = 0;

	for(ii = 0;
			WOW_PLAYER_CLASS_LIST[ii].id != WOW_PLAYER_CLASS_END_OF_LIST;
			ii++) {
		if(!strcmp(WOW_PLAYER_CLASS_LIST[ii].name, playerClass)) {
			return WOW_PLAYER_CLASS_LIST[ii].id;
		}
	}

	return WOW_PLAYER_CLASS_UNKNOWN;
}

static void determinePlayerClass(WowPlayerIndex *index,
		WowPlayerSummary *player, WowEvent *event)
{
	aboolean setPlayerClassId = afalse;

	WowSpellList *spell = NULL;

	if((player->classId == WOW_PLAYER_CLASS_UNKNOWN) &&
			(!strcmp(player->name, "nil"))) {
		free(player->name);
		player->name = strdup("Environment");
		player->classId = WOW_PLAYER_CLASS_ENVIRONMENT;
		setPlayerClassId = atrue;
	}
	else if((player->classId == WOW_PLAYER_CLASS_UNKNOWN) &&
			((event->sourceUnitType & EVENT_UNIT_TYPE_BITMASK) ==
			 EVENT_UNIT_TYPE_PLAYER_MASK)) {
		switch(event->eventId) {
			case WOW_EVENT_DAMAGE_SHIELD:
				if(wowstats_spellindexLookup(index->spellIndex,
							((WowEventDamageShield *)
							 event->eventValue)->spellId, &spell) == 0) {
					player->classId = lookupClassId(spell->playerClass);
					setPlayerClassId = atrue;
				}
				break;
			case WOW_EVENT_DAMAGE_SHIELD_MISSED:
				if(wowstats_spellindexLookup(index->spellIndex,
							((WowEventDamageShieldMissed *)
							 event->eventValue)->spellId, &spell) == 0) {
					player->classId = lookupClassId(spell->playerClass);
					setPlayerClassId = atrue;
				}
				break;
			case WOW_EVENT_DAMAGE_SPLIT:
				if(wowstats_spellindexLookup(index->spellIndex,
							((WowEventDamageSplit *)
							 event->eventValue)->spellId, &spell) == 0) {
					player->classId = lookupClassId(spell->playerClass);
					setPlayerClassId = atrue;
				}
				break;
			case WOW_EVENT_RANGE_DAMAGE:
				if(wowstats_spellindexLookup(index->spellIndex,
							((WowEventRangeDamage *)
							 event->eventValue)->spellId, &spell) == 0) {
					player->classId = lookupClassId(spell->playerClass);
					setPlayerClassId = atrue;
				}
				break;
			case WOW_EVENT_RANGE_MISSED:
				if(wowstats_spellindexLookup(index->spellIndex,
							((WowEventRangeMissed *)
							 event->eventValue)->spellId, &spell) == 0) {
					player->classId = lookupClassId(spell->playerClass);
					setPlayerClassId = atrue;
				}
				break;
			case WOW_EVENT_SPELL_AURA_APPLIED:
				if(wowstats_spellindexLookup(index->spellIndex,
							((WowEventSpellAuraApplied *)
							 event->eventValue)->spellId, &spell) == 0) {
					player->classId = lookupClassId(spell->playerClass);
					setPlayerClassId = atrue;
				}
				break;
			case WOW_EVENT_SPELL_AURA_APPLIED_DOSE:
				if(wowstats_spellindexLookup(index->spellIndex,
							((WowEventSpellAuraAppliedDose *)
							 event->eventValue)->spellId, &spell) == 0) {
					player->classId = lookupClassId(spell->playerClass);
				}
				break;
			case WOW_EVENT_SPELL_AURA_BROKEN:
				if(wowstats_spellindexLookup(index->spellIndex,
							((WowEventSpellAuraBroken *)
							 event->eventValue)->spellId, &spell) == 0) {
					player->classId = lookupClassId(spell->playerClass);
					setPlayerClassId = atrue;
				}
				break;
			case WOW_EVENT_SPELL_AURA_BROKEN_SPELL:
				if(wowstats_spellindexLookup(index->spellIndex,
							((WowEventSpellAuraBrokenSpell *)
							 event->eventValue)->spellId, &spell) == 0) {
					player->classId = lookupClassId(spell->playerClass);
					setPlayerClassId = atrue;
				}
				break;
			case WOW_EVENT_SPELL_AURA_REFRESH:
				if(wowstats_spellindexLookup(index->spellIndex,
							((WowEventSpellAuraRefresh *)
							 event->eventValue)->spellId, &spell) == 0) {
					player->classId = lookupClassId(spell->playerClass);
					setPlayerClassId = atrue;
				}
				break;
			case WOW_EVENT_SPELL_AURA_REMOVED:
				if(wowstats_spellindexLookup(index->spellIndex,
							((WowEventSpellAuraRemoved *)
							 event->eventValue)->spellId, &spell) == 0) {
					player->classId = lookupClassId(spell->playerClass);
					setPlayerClassId = atrue;
				}
				break;
			case WOW_EVENT_SPELL_AURA_REMOVED_DOSE:
				if(wowstats_spellindexLookup(index->spellIndex,
							((WowEventSpellAuraRemovedDose *)
							 event->eventValue)->spellId, &spell) == 0) {
					player->classId = lookupClassId(spell->playerClass);
					setPlayerClassId = atrue;
				}
				break;
			case WOW_EVENT_SPELL_CAST_FAILED:
				if(wowstats_spellindexLookup(index->spellIndex,
							((WowEventSpellCastFailed *)
							 event->eventValue)->spellId, &spell) == 0) {
					player->classId = lookupClassId(spell->playerClass);
					setPlayerClassId = atrue;
				}
				break;
			case WOW_EVENT_SPELL_CAST_START:
				if(wowstats_spellindexLookup(index->spellIndex,
							((WowEventSpellCastStart *)
							 event->eventValue)->spellId, &spell) == 0) {
					player->classId = lookupClassId(spell->playerClass);
					setPlayerClassId = atrue;
				}
				break;
			case WOW_EVENT_SPELL_CAST_SUCCESS:
				if(wowstats_spellindexLookup(index->spellIndex,
							((WowEventSpellCastSuccess *)
							 event->eventValue)->spellId, &spell) == 0) {
					player->classId = lookupClassId(spell->playerClass);
					setPlayerClassId = atrue;
				}
				break;
			case WOW_EVENT_SPELL_CREATE:
				if(wowstats_spellindexLookup(index->spellIndex,
							((WowEventSpellCreate *)
							 event->eventValue)->spellId, &spell) == 0) {
					player->classId = lookupClassId(spell->playerClass);
					setPlayerClassId = atrue;
				}
				break;
			case WOW_EVENT_SPELL_DAMAGE:
				if(wowstats_spellindexLookup(index->spellIndex,
							((WowEventSpellDamage *)
							 event->eventValue)->spellId, &spell) == 0) {
					player->classId = lookupClassId(spell->playerClass);
					setPlayerClassId = atrue;
				}
				break;
			case WOW_EVENT_SPELL_DISPEL:
				if(wowstats_spellindexLookup(index->spellIndex,
							((WowEventSpellDispel *)
							 event->eventValue)->spellId, &spell) == 0) {
					player->classId = lookupClassId(spell->playerClass);
					setPlayerClassId = atrue;
				}
				break;
			case WOW_EVENT_SPELL_DISPEL_FAILED:
				if(wowstats_spellindexLookup(index->spellIndex,
							((WowEventSpellDispelFailed *)
							 event->eventValue)->spellId, &spell) == 0) {
					player->classId = lookupClassId(spell->playerClass);
					setPlayerClassId = atrue;
				}
				break;
			case WOW_EVENT_SPELL_ENERGIZE:
				if(wowstats_spellindexLookup(index->spellIndex,
							((WowEventSpellEnergize *)
							 event->eventValue)->spellId, &spell) == 0) {
					player->classId = lookupClassId(spell->playerClass);
					setPlayerClassId = atrue;
				}
				break;
			case WOW_EVENT_SPELL_EXTRA_ATTACKS:
				if(wowstats_spellindexLookup(index->spellIndex,
							((WowEventSpellExtraAttacks *)
							 event->eventValue)->spellId, &spell) == 0) {
					player->classId = lookupClassId(spell->playerClass);
					setPlayerClassId = atrue;
				}
				break;
			case WOW_EVENT_SPELL_HEAL:
				if(wowstats_spellindexLookup(index->spellIndex,
							((WowEventSpellHealed *)
							 event->eventValue)->spellId, &spell) == 0) {
					player->classId = lookupClassId(spell->playerClass);
					setPlayerClassId = atrue;
				}
				break;
			case WOW_EVENT_SPELL_INSTAKILL:
				if(wowstats_spellindexLookup(index->spellIndex,
							((WowEventSpellInstakill *)
							 event->eventValue)->spellId, &spell) == 0) {
					player->classId = lookupClassId(spell->playerClass);
					setPlayerClassId = atrue;
				}
				break;
			case WOW_EVENT_SPELL_INTERRUPT:
				if(wowstats_spellindexLookup(index->spellIndex,
							((WowEventSpellInterrupt *)
							 event->eventValue)->spellId, &spell) == 0) {
					player->classId = lookupClassId(spell->playerClass);
					setPlayerClassId = atrue;
				}
				break;
			case WOW_EVENT_SPELL_LEECH:
				if(wowstats_spellindexLookup(index->spellIndex,
							((WowEventSpellLeech *)
							 event->eventValue)->spellId, &spell) == 0) {
					player->classId = lookupClassId(spell->playerClass);
					setPlayerClassId = atrue;
				}
				break;
			case WOW_EVENT_SPELL_MISSED:
				if(wowstats_spellindexLookup(index->spellIndex,
							((WowEventSpellMissed *)
							 event->eventValue)->spellId, &spell) == 0) {
					player->classId = lookupClassId(spell->playerClass);
					setPlayerClassId = atrue;
				}
				break;
			case WOW_EVENT_SPELL_RESURRECT:
				if(wowstats_spellindexLookup(index->spellIndex,
							((WowEventSpellResurrect *)
							 event->eventValue)->spellId, &spell) == 0) {
					player->classId = lookupClassId(spell->playerClass);
					setPlayerClassId = atrue;
				}
				break;
			case WOW_EVENT_SPELL_STOLEN:
				if(wowstats_spellindexLookup(index->spellIndex,
							((WowEventSpellStolen *)
							 event->eventValue)->spellId, &spell) == 0) {
					player->classId = lookupClassId(spell->playerClass);
					setPlayerClassId = atrue;
				}
				break;
			case WOW_EVENT_SPELL_SUMMON:
				if(wowstats_spellindexLookup(index->spellIndex,
							((WowEventSpellSummon *)
							 event->eventValue)->spellId, &spell) == 0) {
					player->classId = lookupClassId(spell->playerClass);
					setPlayerClassId = atrue;
				}
				break;
			case WOW_EVENT_SPELL_PERIODIC_DAMAGE:
				if(wowstats_spellindexLookup(index->spellIndex,
							((WowEventSpellPeriodicDamage *)
							 event->eventValue)->spellId, &spell) == 0) {
					player->classId = lookupClassId(spell->playerClass);
					setPlayerClassId = atrue;
				}
				break;
			case WOW_EVENT_SPELL_PERIODIC_ENERGIZE:
				if(wowstats_spellindexLookup(index->spellIndex,
							((WowEventSpellPeriodicEnergize *)
							 event->eventValue)->spellId, &spell) == 0) {
					player->classId = lookupClassId(spell->playerClass);
					setPlayerClassId = atrue;
				}
				break;
			case WOW_EVENT_SPELL_PERIODIC_HEAL:
				if(wowstats_spellindexLookup(index->spellIndex,
							((WowEventSpellPeriodicHealed *)
							 event->eventValue)->spellId, &spell) == 0) {
					player->classId = lookupClassId(spell->playerClass);
					setPlayerClassId = atrue;
				}
				break;
			case WOW_EVENT_SPELL_PERIODIC_MISSED:
				if(wowstats_spellindexLookup(index->spellIndex,
							((WowEventSpellPeriodicMissed *)
							 event->eventValue)->spellId, &spell) == 0) {
					player->classId = lookupClassId(spell->playerClass);
					setPlayerClassId = atrue;
				}
				break;
		}
	}
	else if(player->classId == WOW_PLAYER_CLASS_UNKNOWN) {
		switch(event->sourceUnitType & EVENT_UNIT_TYPE_BITMASK) {
			case EVENT_UNIT_TYPE_NPC_MASK:
				player->classId = WOW_PLAYER_CLASS_NPC;
				setPlayerClassId = atrue;
				break;
			case EVENT_UNIT_TYPE_PET_MASK:
				player->classId = WOW_PLAYER_CLASS_PET;
				setPlayerClassId = atrue;
				break;
			case EVENT_UNIT_TYPE_VEHICLE_MASK:
				player->classId = WOW_PLAYER_CLASS_VEHICLE;
				setPlayerClassId = atrue;
				break;
		}
	}

	if((setPlayerClassId) &&
			(player->classId != WOW_PLAYER_CLASS_UNKNOWN) &&
			(player->classId != WOW_PLAYER_CLASS_NPC) &&
			(player->classId != WOW_PLAYER_CLASS_PET) &&
			(player->classId != WOW_PLAYER_CLASS_ENVIRONMENT) &&
			(player->classId != WOW_PLAYER_CLASS_END_OF_LIST)) {

//char timestamp[32];
//wowstats_timestampToString(&event->timestamp, timestamp);
//printf("(0x%lx) PLAYER {%s} '%s' => [%s] has SPAWNED (%i ov %i) at '%s'\n", (unsigned long int)index, player->uid, player->name, WOW_PLAYER_CLASS_LIST[player->classId].name, index->alivePlayerCount, index->currentPlayerCount, timestamp);

		index->currentPlayerCount += 1;
		index->alivePlayerCount += 1;
	}
}

static aboolean isSpellInBuffList(WowEvent *event, aboolean *isApplied)
{
	int ii = 0;

	WowEventSpellAuraApplied *appliedValue = NULL;
	WowEventSpellAuraRemoved *removedValue = NULL;

	*isApplied = afalse;

	if(event->eventId == WOW_EVENT_SPELL_AURA_APPLIED) {
		appliedValue = (WowEventSpellAuraApplied *)event->eventValue;

		for(ii = 0; WOW_PLAYERINDEX_BUFF_SPELL_LIST[ii].id != -1; ii++) {
			if(appliedValue->spellId ==
					WOW_PLAYERINDEX_BUFF_SPELL_LIST[ii].id) {
				*isApplied = atrue;
				return atrue;
			}
		}
	}
	else if(event->eventId == WOW_EVENT_SPELL_AURA_REMOVED) {
		removedValue = (WowEventSpellAuraRemoved *)event->eventValue;

		for(ii = 0; WOW_PLAYERINDEX_BUFF_SPELL_LIST[ii].id != -1; ii++) {
			if(removedValue->spellId ==
					WOW_PLAYERINDEX_BUFF_SPELL_LIST[ii].id) {
				return atrue;
			}
		}
	}

	return afalse;
}


// define wowstats player-index public functions

int wowstats_playerIndexInit(WowPlayerIndex *index, WowSpellindex *spellIndex)
{
	if((index == NULL) || (spellIndex == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	memset(index, 0, sizeof(WowPlayerIndex));

	index->currentPlayerCount = 0;
	index->alivePlayerCount = 0;

	if(bptree_init(&index->index) < 0) {
		return -1;
	}

	if(bptree_setFreeFunction(&index->index, freePlayer) < 0) {
		return -1;
	}

	index->spellIndex = spellIndex;

	return 0;
}

int wowstats_playerIndexFree(WowPlayerIndex *index)
{
	if(index == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(bptree_free(&index->index) < 0) {
		return -1;
	}

	memset(index, 0, sizeof(WowPlayerIndex));

	return 0;
}

int wowstats_playerIndexNoteEvent(WowPlayerIndex *index, WowEvent *event)
{
	aboolean isApplied = afalse;
	aboolean isBuff = afalse;
	int elapsedSeconds = 0;

	WowPlayerSummary *player = NULL;

	if((index == NULL) || (event == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(bptree_get(&index->index, event->sourceUid, strlen(event->sourceUid),
				(void *)&player) < 0) {
		player = newPlayer(event->sourceName, event->sourceUid);
		if(bptree_put(&index->index, player->uid, strlen(player->uid),
					player) < 0) {
			return -1;
		}
	}

	if(wowstats_playerNoteTimestamp(player, &event->timestamp) < 0) {
		return -1;
	}

	determinePlayerClass(index, player, event);

	if((event->eventId != WOW_EVENT_UNIT_DIED) &&
			(event->eventId != WOW_EVENT_ENCHANT_REMOVED) &&
			(event->eventId != WOW_EVENT_SPELL_AURA_BROKEN) &&
			(event->eventId != WOW_EVENT_SPELL_AURA_BROKEN_SPELL) &&
//			(event->eventId != WOW_EVENT_SPELL_AURA_REMOVED) &&
			(event->eventId != WOW_EVENT_SPELL_AURA_REMOVED_DOSE) &&
			(event->eventId != WOW_EVENT_SPELL_PERIODIC_DAMAGE) &&
			(event->eventId != WOW_EVENT_SPELL_PERIODIC_ENERGIZE) &&
			(event->eventId != WOW_EVENT_SPELL_PERIODIC_HEAL) &&
			(event->eventId != WOW_EVENT_SPELL_PERIODIC_MISSED)
			) {
		isBuff = isSpellInBuffList(event, &isApplied);

		if(((isBuff) && (!isApplied)) ||
				((event->eventId != WOW_EVENT_SPELL_AURA_REMOVED) &&
				 (!player->isPlayerAlive) &&
				 (player->classId != WOW_PLAYER_CLASS_UNKNOWN) &&
				 (player->classId != WOW_PLAYER_CLASS_NPC) &&
				 (player->classId != WOW_PLAYER_CLASS_PET) &&
				 (player->classId != WOW_PLAYER_CLASS_ENVIRONMENT) &&
				 (player->classId != WOW_PLAYER_CLASS_END_OF_LIST))
				) {
			if(wowstats_timestampCalculateDifferenceInSeconds(
						&event->timestamp,
						&player->diedTimestamp,
						&elapsedSeconds) < 0) {
				return -1;
			}

			if((player->classId == WOW_PLAYER_CLASS_HUNTER) &&
					(elapsedSeconds >= 45)) {
				index->alivePlayerCount += 1;
				player->isPlayerAlive = atrue;
//char timestamp[32];
//wowstats_timestampToString(&event->timestamp, timestamp);
//printf("(0x%lx) PLAYER {%s} '%s' => [%s] has ARISEN (%i ov %i) at '%s'\n", (unsigned long int)index, player->uid, player->name, WOW_PLAYER_CLASS_LIST[player->classId].name, index->alivePlayerCount, index->currentPlayerCount, timestamp);
			}
			else if(elapsedSeconds >= 60) {
				index->alivePlayerCount += 1;
				player->isPlayerAlive = atrue;
//char timestamp[32];
//wowstats_timestampToString(&event->timestamp, timestamp);
//printf("(0x%lx) PLAYER {%s} '%s' => [%s] has ARISEN (%i ov %i) at '%s'\n", (unsigned long int)index, player->uid, player->name, WOW_PLAYER_CLASS_LIST[player->classId].name, index->alivePlayerCount, index->currentPlayerCount, timestamp);
			}
//			else {
//printf("PLAYER {%s} '%s' => [%s] remains DEAD after %i seconds (%i ov %i)\n", player->uid, player->name, WOW_PLAYER_CLASS_LIST[player->classId].name, elapsedSeconds, index->alivePlayerCount, index->currentPlayerCount);
//			}
		}
	}

	if(bptree_get(&index->index, event->targetUid, strlen(event->targetUid),
				(void *)&player) < 0) {
		player = newPlayer(event->targetName, event->targetUid);
		if(bptree_put(&index->index, player->uid, strlen(player->uid),
					player) < 0) {
			return -1;
		}
	}

	if(wowstats_playerNoteTimestamp(player, &event->timestamp) < 0) {
		return -1;
	}

	/*
	 * Note: you cannot determine player class on the target due to the fact
	 *       that the target did not perform the action.
	 */

	isBuff = isSpellInBuffList(event, &isApplied);
	if(((isBuff) && (isApplied)) ||
			(event->eventId == WOW_EVENT_UNIT_DIED)) {
		if((player->isPlayerAlive) &&
				(player->classId != WOW_PLAYER_CLASS_UNKNOWN) &&
				(player->classId != WOW_PLAYER_CLASS_NPC) &&
				(player->classId != WOW_PLAYER_CLASS_PET) &&
				(player->classId != WOW_PLAYER_CLASS_ENVIRONMENT) &&
				(player->classId != WOW_PLAYER_CLASS_END_OF_LIST)) {
			if(wowstats_timestampClone(&player->diedTimestamp,
						&event->timestamp) < 0) {
				return WOW_TRIGGER_RESULT_ERROR;
			}

			index->alivePlayerCount -= 1;
			player->isPlayerAlive = afalse;

//char timestamp[32];
//wowstats_timestampToString(&event->timestamp, timestamp);
//printf("(0x%lx) PLAYER {%s} '%s' => [%s] has DIED (%i of %i) at '%s'\n", (unsigned long int)index, player->uid, player->name, WOW_PLAYER_CLASS_LIST[player->classId].name, index->alivePlayerCount, index->currentPlayerCount, timestamp);
		}
	}

	return 0;
}

int wowstats_playerIndexGetSummaries(WowPlayerIndex *index,
		WowPlayerSummary ***summaries, int *summaryLength)
{
	int ii = 0;
	int *keyLengths = NULL;
	char **keys = NULL;

	if((index == NULL) || (summaries == NULL) || (summaryLength == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	*summaryLength = 0;

	if(bptree_toArray(&index->index, summaryLength, &keyLengths, &keys,
				(void ***)summaries) < 0) {
		return -1;
	}

	free(keyLengths);

	for(ii = 0; ii < *summaryLength; ii++) {
		free(keys[ii]);
	}
	free(keys);

	return 0;
}

