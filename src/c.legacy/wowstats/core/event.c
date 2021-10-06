/*
 * event.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard-WoW statistical analysis event-parsing library.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#include "wowstats/common.h"
#define _WOWSTATS_DATA_COMPONENT
#include "wowstats/data/events.h"
#include "wowstats/data/eventspells.h"
#include "wowstats/data/eventpowers.h"
#define _WOWSTATS_CORE_COMPONENT
#include "wowstats/core/parser.h"
#include "wowstats/core/timestamp.h"
#include "wowstats/core/event.h"

// define wowstats event private constants

#define KEY_PORTION_LENGTH								1024

#define BUFFER_LENGTH									8192
#define BUFFER_SNPRINTF_LENGTH							8191

#define EVENT_TYPE_ARRAY_ELEMENT						2
#define SOURCE_UID_ARRAY_ELEMENT						3
#define SOURCE_NAME_ARRAY_ELEMENT						4
#define SOURCE_FLAGS_ARRAY_ELEMENT						5
#define TARGET_UID_ARRAY_ELEMENT						6
#define TARGET_NAME_ARRAY_ELEMENT						7
#define TARGET_FLAGS_ARRAY_ELEMENT						8


// declare wowstats event private functions

/*
 * TODO: this
 */


// define wowstats event private functions

// general functions

static aboolean isValidEvent(WowParser *parser)
{
	if(parser->tokenCount < 9) {
		return afalse;
	}
	return atrue;
}

static int determineEventId(WowParser *parser)
{
	int ii = 0;

	for(ii = 0; WOW_EVENT_LIST[ii].id != WOW_EVENT_END_OF_LIST; ii++) {
		if(WOW_EVENT_LIST[ii].nameLength !=
				parser->tokenLengths[EVENT_TYPE_ARRAY_ELEMENT]) {
			continue;
		}
		if(!strncmp(parser->tokens[EVENT_TYPE_ARRAY_ELEMENT],
					WOW_EVENT_LIST[ii].name,
					WOW_EVENT_LIST[ii].nameLength)) {
			return WOW_EVENT_LIST[ii].id;
		}
	}

	return WOWSTATS_EVENT_UNKNOWN_EVENT;
}

// spell-releated parsing

static int determineSpellSchool(WowParser *parser, int tokenId)
{
	int ii = 0;

	for(ii = 0; WOW_SPELL_SCHOOL[ii].id != WOW_SPELL_SCHOOL_END_OF_LIST;
			ii++) {
		if(WOW_SPELL_SCHOOL[ii].codeLength != parser->tokenLengths[tokenId]) {
			continue;
		}
		if(!strncmp(parser->tokens[tokenId], WOW_SPELL_SCHOOL[ii].code,
					WOW_SPELL_SCHOOL[ii].codeLength)) {
			return WOW_SPELL_SCHOOL[ii].id;
		}
	}

	return -1;
}

static char *determinePowerType(WowParser *parser, int tokenId)
{
	int ii = 0;
	int powerCode = 0;
	char *result = "Unknown";

	powerCode = atoi(parser->tokens[tokenId]);

	for(ii = 0; WOW_POWER_TYPE[ii].id != WOW_POWER_TYPE_END_OF_LIST; ii++) {
		if(WOW_POWER_TYPE[ii].code == powerCode) {
			result = WOW_POWER_TYPE[ii].name;
			break;
		}
	}

	return result;
}

// DAMAGE_SHIELD event parsing

static void *parseDamageShield(WowParser *parser)
{
	int spellSchool = 0;

	WowEventDamageShield *result = NULL;

	if(parser->tokenCount < 21) {
		return NULL;
	}

	if((spellSchool = determineSpellSchool(parser, 11)) < 0) {
		return NULL;
	}

	result = (WowEventDamageShield *)malloc(sizeof(WowEventDamageShield));

	result->spellId = atoi(parser->tokens[9]);
	result->spellName = strndup(parser->tokens[10], parser->tokenLengths[10]);
	result->spellSchool = spellSchool;
	result->damageAmount = atoi(parser->tokens[12]);
	result->overkillAmount = atoi(parser->tokens[13]);
	result->school = spellSchool; // TODO: verify this
	result->resistAmount = atoi(parser->tokens[15]);
	result->blockAmount = atoi(parser->tokens[16]);

	if(!strncmp(parser->tokens[17], "0", 1)) {
		result->absorbed = afalse;
	}
	else {
		result->absorbed = atrue;
	}

	if(!strncmp(parser->tokens[18], "1", 1)) {
		result->wasCritical = atrue;
	}
	else {
		result->wasCritical = afalse;
	}

	if(!strncmp(parser->tokens[19], "1", 1)) {
		result->glancing = atrue;
	}
	else {
		result->glancing = afalse;
	}

	if(!strncmp(parser->tokens[20], "1", 1)) {
		result->crushing = atrue;
	}
	else {
		result->crushing = afalse;
	}

/*
	printf("\t "
			"DETAIL[%s]{%i, '%s', '%s', %i, %i, %i, %i, %i, %i, %i, %i, %i}\n",
			WOW_EVENT_LIST[(WOW_EVENT_DAMAGE_SHIELD - 1)].name, result->spellId,
			result->spellName,
			WOW_SPELL_SCHOOL[(result->spellSchool - 1)].name,
			result->damageAmount, result->overkillAmount, result->school,
			result->resistAmount, result->blockAmount, result->absorbed,
			result->wasCritical, result->glancing, result->crushing);
*/
	return result;
}

static void freeDamageShield(WowEventDamageShield *event)
{
	if(event->spellName != NULL) {
		free(event->spellName);
	}

	free(event);
}

static void buildKeyPortionDamageShield(WowEventDamageShield *event,
		char keyPortion[KEY_PORTION_LENGTH])
{
	unsigned int hashValue = 0;
	char buffer[BUFFER_LENGTH];

	snprintf(buffer, BUFFER_SNPRINTF_LENGTH,
			"%i/%s/%i/%i/%i/%i/%i/%i/%i/%i/%i/%i",
			event->spellId, event->spellName, event->spellSchool,
			event->damageAmount, event->overkillAmount, event->school,
			event->resistAmount, event->blockAmount, event->absorbed,
			event->wasCritical, event->glancing, event->crushing);

	hashValue = crc32_calculateHash(buffer, strlen(buffer));

	snprintf(keyPortion, (KEY_PORTION_LENGTH - 1), "%u", hashValue);
}

// DAMAGE_SHIELD_MISSED event parsing

static void *parseDamageShieldMissed(WowParser *parser)
{
	int spellSchool = 0;

	WowEventDamageShieldMissed *result = NULL;

	if(parser->tokenCount < 13) {
		return NULL;
	}

	if((spellSchool = determineSpellSchool(parser, 11)) < 0) {
		return NULL;
	}

	result = (WowEventDamageShieldMissed *)malloc(
			sizeof(WowEventDamageShieldMissed));

	result->spellId = atoi(parser->tokens[9]);
	result->spellName = strndup(parser->tokens[10], parser->tokenLengths[10]);
	result->spellSchool = spellSchool;
	result->missType = strndup(parser->tokens[12], parser->tokenLengths[12]);

	if(parser->tokenCount < 14) {
		result->missAmount = 0;
	}
	else {
		result->missAmount = atoi(parser->tokens[13]);
	}

/*
	printf("\t "
			"DETAIL[%s]{%i, '%s', '%s', '%s', %i}\n",
			WOW_EVENT_LIST[(WOW_EVENT_DAMAGE_SHIELD_MISSED - 1)].name,
			result->spellId, result->spellName,
			WOW_SPELL_SCHOOL[(result->spellSchool - 1)].name,
			result->missType, result->missAmount);
*/

	return result;
}

static void freeDamageShieldMissed(WowEventDamageShieldMissed *event)
{
	if(event->spellName != NULL) {
		free(event->spellName);
	}

	if(event->missType != NULL) {
		free(event->missType);
	}

	free(event);
}

static void buildKeyPortionDamageShieldMissed(
		WowEventDamageShieldMissed *event, char keyPortion[KEY_PORTION_LENGTH])
{
	unsigned int hashValue = 0;
	char buffer[BUFFER_LENGTH];

	snprintf(buffer, BUFFER_SNPRINTF_LENGTH,
			"%i/%s/%i/%s/%i",
			event->spellId, event->spellName, event->spellSchool,
			event->missType, event->missAmount);

	hashValue = crc32_calculateHash(buffer, strlen(buffer));

	snprintf(keyPortion, (KEY_PORTION_LENGTH - 1), "%u", hashValue);
}

// DAMAGE_SPLIT event parsing

static void *parseDamageSplit(WowParser *parser)
{
	int spellSchool = 0;

	WowEventDamageSplit *result = NULL;

	if(parser->tokenCount < 21) {
		return NULL;
	}

	if((spellSchool = determineSpellSchool(parser, 11)) < 0) {
		return NULL;
	}

	result = (WowEventDamageSplit *)malloc(sizeof(WowEventDamageSplit));

	result->spellId = atoi(parser->tokens[9]);
	result->spellName = strndup(parser->tokens[10], parser->tokenLengths[10]);
	result->spellSchool = spellSchool;
	result->damageAmount = atoi(parser->tokens[12]);
	result->overkillAmount = atoi(parser->tokens[13]);
	result->school = spellSchool; // TODO: verify this
	result->resistAmount = atoi(parser->tokens[15]);
	result->blockAmount = atoi(parser->tokens[16]);

	if(!strncmp(parser->tokens[17], "0", 1)) {
		result->absorbed = afalse;
	}
	else {
		result->absorbed = atrue;
	}

	if(!strncmp(parser->tokens[18], "1", 1)) {
		result->wasCritical = atrue;
	}
	else {
		result->wasCritical = afalse;
	}

	if(!strncmp(parser->tokens[19], "1", 1)) {
		result->glancing = atrue;
	}
	else {
		result->glancing = afalse;
	}

	if(!strncmp(parser->tokens[20], "1", 1)) {
		result->crushing = atrue;
	}
	else {
		result->crushing = afalse;
	}

/*
	printf("\t "
			"DETAIL[%s]{%i, '%s', '%s', %i, %i, %i, %i, %i, %i, %i, %i, %i}\n",
			WOW_EVENT_LIST[(WOW_EVENT_DAMAGE_SPLIT - 1)].name, result->spellId,
			result->spellName,
			WOW_SPELL_SCHOOL[(result->spellSchool - 1)].name,
			result->damageAmount, result->overkillAmount, result->school,
			result->resistAmount, result->blockAmount, result->absorbed,
			result->wasCritical, result->glancing, result->crushing);
*/

	return result;
}

static void freeDamageSplit(WowEventDamageSplit *event)
{
	if(event->spellName != NULL) {
		free(event->spellName);
	}

	free(event);
}

static void buildKeyPortionDamageSplit(WowEventDamageSplit *event,
		char keyPortion[KEY_PORTION_LENGTH])
{
	unsigned int hashValue = 0;
	char buffer[BUFFER_LENGTH];

	snprintf(buffer, BUFFER_SNPRINTF_LENGTH,
			"%i/%s/%i/%i/%i/%i/%i/%i/%i/%i/%i/%i",
			event->spellId, event->spellName, event->spellSchool,
			event->damageAmount, event->overkillAmount, event->school,
			event->resistAmount, event->blockAmount, event->absorbed,
			event->wasCritical, event->glancing, event->crushing);

	hashValue = crc32_calculateHash(buffer, strlen(buffer));

	snprintf(keyPortion, (KEY_PORTION_LENGTH - 1), "%u", hashValue);
}

// ENCHANT_APPLIED event parsing

static void *parseEnchantApplied(WowParser *parser)
{
	WowEventEnchantApplied *result = NULL;

	if(parser->tokenCount < 12) {
		return NULL;
	}

	result = (WowEventEnchantApplied *)malloc(sizeof(WowEventEnchantApplied));

	result->spellName = strndup(parser->tokens[9], parser->tokenLengths[9]);
	result->itemId = atoi(parser->tokens[10]);
	result->itemName = strndup(parser->tokens[11], parser->tokenLengths[11]);

/*
	printf("\t "
			"DETAIL[%s]{'%s', %i, '%s'}\n",
			WOW_EVENT_LIST[(WOW_EVENT_ENCHANT_APPLIED - 1)].name,
			result->spellName, result->itemId, result->itemName);
*/

	return result;
}

static void freeEnchantApplied(WowEventEnchantApplied *event)
{
	if(event->spellName != NULL) {
		free(event->spellName);
	}

	if(event->itemName != NULL) {
		free(event->itemName);
	}

	free(event);
}

static void buildKeyPortionEnchantApplied(WowEventEnchantApplied *event,
		char keyPortion[KEY_PORTION_LENGTH])
{
	unsigned int hashValue = 0;
	char buffer[BUFFER_LENGTH];

	snprintf(buffer, BUFFER_SNPRINTF_LENGTH,
			"%s/%i/%s",
			event->spellName, event->itemId, event->itemName);

	hashValue = crc32_calculateHash(buffer, strlen(buffer));

	snprintf(keyPortion, (KEY_PORTION_LENGTH - 1), "%u", hashValue);
}

// ENCHANT_REMOVED event parsing

static void *parseEnchantRemoved(WowParser *parser)
{
	WowEventEnchantRemoved *result = NULL;

	if(parser->tokenCount < 12) {
		return NULL;
	}

	result = (WowEventEnchantRemoved *)malloc(sizeof(WowEventEnchantRemoved));

	result->spellName = strndup(parser->tokens[9], parser->tokenLengths[9]);
	result->itemId = atoi(parser->tokens[10]);
	result->itemName = strndup(parser->tokens[11], parser->tokenLengths[11]);

/*
	printf("\t "
			"DETAIL[%s]{'%s', %i, '%s'}\n",
			WOW_EVENT_LIST[(WOW_EVENT_ENCHANT_REMOVED - 1)].name,
			result->spellName, result->itemId, result->itemName);
*/

	return result;
}

static void freeEnchantRemoved(WowEventEnchantRemoved *event)
{
	if(event->spellName != NULL) {
		free(event->spellName);
	}

	if(event->itemName != NULL) {
		free(event->itemName);
	}

	free(event);
}

static void buildKeyPortionEnchantRemoved(WowEventEnchantRemoved *event,
		char keyPortion[KEY_PORTION_LENGTH])
{
	unsigned int hashValue = 0;
	char buffer[BUFFER_LENGTH];

	snprintf(buffer, BUFFER_SNPRINTF_LENGTH,
			"%s/%i/%s",
			event->spellName, event->itemId, event->itemName);

	hashValue = crc32_calculateHash(buffer, strlen(buffer));

	snprintf(keyPortion, (KEY_PORTION_LENGTH - 1), "%u", hashValue);
}

// PARTY_KILL event parsing

static void *parsePartyKill(WowParser *parser)
{
	WowEventPartyKill *result = NULL;

	if(parser->tokenCount < 9) {
		return NULL;
	}

	result = (WowEventPartyKill *)malloc(sizeof(WowEventPartyKill));

	result->placeholder = atrue;

/*
	printf("\t "
			"DETAIL[%s]{%i}\n",
			WOW_EVENT_LIST[(WOW_EVENT_PARTY_KILL - 1)].name,
			result->placeholder);
*/

	return result;
}

static void freePartyKill(WowEventPartyKill *event)
{
	free(event);
}

static void buildKeyPortionPartyKill(WowEventPartyKill *event,
		char keyPortion[KEY_PORTION_LENGTH])
{
	snprintf(keyPortion, (KEY_PORTION_LENGTH - 1), "%u", (unsigned int)0);
}

// UNIT_DIED event parsing

static void *parseUnitDied(WowParser *parser)
{
	WowEventUnitDied *result = NULL;

	if(parser->tokenCount < 9) {
		return NULL;
	}

	result = (WowEventUnitDied *)malloc(sizeof(WowEventUnitDied));

	result->placeholder = atrue;

/*
	printf("\t "
			"DETAIL[%s]{%i}\n",
			WOW_EVENT_LIST[(WOW_EVENT_UNIT_DIED - 1)].name,
			result->placeholder);
*/

	return result;
}

static void freeUnitDied(WowEventUnitDied *event)
{
	free(event);
}

static void buildKeyPortionUnitDied(WowEventUnitDied *event,
		char keyPortion[KEY_PORTION_LENGTH])
{
	snprintf(keyPortion, (KEY_PORTION_LENGTH - 1), "%u", (unsigned int)0);
}

// UNIT_DESTROYED event parsing

static void *parseUnitDestroyed(WowParser *parser)
{
	WowEventUnitDestroyed *result = NULL;

	if(parser->tokenCount < 9) {
		return NULL;
	}

	result = (WowEventUnitDestroyed *)malloc(sizeof(WowEventUnitDestroyed));

	result->placeholder = atrue;

/*
	printf("\t "
			"DETAIL[%s]{%i}\n",
			WOW_EVENT_LIST[(WOW_EVENT_UNIT_DESTROYED - 1)].name,
			result->placeholder);
*/

	return result;
}

static void freeUnitDestroyed(WowEventUnitDestroyed *event)
{
	free(event);
}

static void buildKeyPortionUnitDestroyed(WowEventUnitDestroyed *event,
		char keyPortion[KEY_PORTION_LENGTH])
{
	snprintf(keyPortion, (KEY_PORTION_LENGTH - 1), "%u", (unsigned int)0);
}

// ENVIRONMENTAL_DAMAGE event parsing

static void *parseEnvironmentalDamage(WowParser *parser)
{
	WowEventEnvironmentalDamage *result = NULL;

	if(parser->tokenCount < 9) {
		return NULL;
	}

	result = (WowEventEnvironmentalDamage *)malloc(
			sizeof(WowEventEnvironmentalDamage));


	result->damageType = strndup(parser->tokens[9], parser->tokenLengths[9]);
	result->damageAmount = atoi(parser->tokens[10]);
	result->overkillAmount = atoi(parser->tokens[11]);
	result->school = atoi(parser->tokens[12]);
	result->resistAmount = atoi(parser->tokens[13]);
	result->blockAmount = atoi(parser->tokens[14]);

	if(!strncmp(parser->tokens[15], "0", 1)) {
		result->absorbed = afalse;
	}
	else {
		result->absorbed = atrue;
	}

	if(!strncmp(parser->tokens[16], "1", 1)) {
		result->wasCritical = atrue;
	}
	else {
		result->wasCritical = afalse;
	}

	if(!strncmp(parser->tokens[17], "1", 1)) {
		result->glancing = atrue;
	}
	else {
		result->glancing = afalse;
	}

	if(!strncmp(parser->tokens[18], "1", 1)) {
		result->crushing = atrue;
	}
	else {
		result->crushing = afalse;
	}

/*
	printf("\t "
			"DETAIL[%s]{'%s', %i, %i, %i, %i, %i, %i, %i, %i, %i}\n",
			WOW_EVENT_LIST[(WOW_EVENT_ENVIRONMENTAL_DAMAGE - 1)].name,
			result->damageType, result->damageAmount, result->overkillAmount,
			result->school, result->resistAmount, result->blockAmount,
			result->absorbed, result->wasCritical, result->glancing,
			result->crushing);
*/

	return result;
}

static void freeEnvironmentalDamage(WowEventEnvironmentalDamage *event)
{
	if(event->damageType != NULL) {
		free(event->damageType);
	}

	free(event);
}

static void buildKeyPortionEnvironmentalDamage(
		WowEventEnvironmentalDamage *event,
		char keyPortion[KEY_PORTION_LENGTH])
{
	unsigned int hashValue = 0;
	char buffer[BUFFER_LENGTH];

	snprintf(buffer, BUFFER_SNPRINTF_LENGTH,
			"%s/%i/%i/%i/%i/%i/%i/%i/%i/%i",
			event->damageType,
			event->damageAmount, event->overkillAmount, event->school,
			event->resistAmount, event->blockAmount, event->absorbed,
			event->wasCritical, event->glancing, event->crushing);

	hashValue = crc32_calculateHash(buffer, strlen(buffer));

	snprintf(keyPortion, (KEY_PORTION_LENGTH - 1), "%u", hashValue);
}

// RANGE_DAMAGE event parsing

static void *parseRangeDamage(WowParser *parser)
{
	int spellSchool = 0;

	WowEventRangeDamage *result = NULL;

	if(parser->tokenCount < 21) {
		return NULL;
	}

	if((spellSchool = determineSpellSchool(parser, 11)) < 0) {
		return NULL;
	}

	result = (WowEventRangeDamage *)malloc(sizeof(WowEventRangeDamage));

	result->spellId = atoi(parser->tokens[9]);
	result->spellName = strndup(parser->tokens[10], parser->tokenLengths[10]);
	result->spellSchool = spellSchool;
	result->damageAmount = atoi(parser->tokens[12]);
	result->overkillAmount = atoi(parser->tokens[13]);
	result->school = spellSchool; // TODO: verify this
	result->resistAmount = atoi(parser->tokens[15]);
	result->blockAmount = atoi(parser->tokens[16]);

	if(!strncmp(parser->tokens[17], "0", 1)) {
		result->absorbed = afalse;
	}
	else {
		result->absorbed = atrue;
	}

	if(!strncmp(parser->tokens[18], "1", 1)) {
		result->wasCritical = atrue;
	}
	else {
		result->wasCritical = afalse;
	}

	if(!strncmp(parser->tokens[19], "1", 1)) {
		result->glancing = atrue;
	}
	else {
		result->glancing = afalse;
	}

	if(!strncmp(parser->tokens[20], "1", 1)) {
		result->crushing = atrue;
	}
	else {
		result->crushing = afalse;
	}

/*
	printf("\t "
			"DETAIL[%s]{%i, '%s', '%s', %i, %i, %i, %i, %i, %i, %i, %i, %i}\n",
			WOW_EVENT_LIST[(WOW_EVENT_RANGE_DAMAGE - 1)].name, result->spellId,
			result->spellName,
			WOW_SPELL_SCHOOL[(result->spellSchool - 1)].name,
			result->damageAmount, result->overkillAmount, result->school,
			result->resistAmount, result->blockAmount, result->absorbed,
			result->wasCritical, result->glancing, result->crushing);
*/
	return result;
}

static void freeRangeDamage(WowEventRangeDamage *event)
{
	if(event->spellName != NULL) {
		free(event->spellName);
	}

	free(event);
}

static void buildKeyPortionRangeDamage(WowEventRangeDamage *event,
		char keyPortion[KEY_PORTION_LENGTH])
{
	unsigned int hashValue = 0;
	char buffer[BUFFER_LENGTH];

	snprintf(buffer, BUFFER_SNPRINTF_LENGTH,
			"%i/%s/%i/%i/%i/%i/%i/%i/%i/%i/%i/%i",
			event->spellId, event->spellName, event->spellSchool,
			event->damageAmount, event->overkillAmount, event->school,
			event->resistAmount, event->blockAmount, event->absorbed,
			event->wasCritical, event->glancing, event->crushing);

	hashValue = crc32_calculateHash(buffer, strlen(buffer));

	snprintf(keyPortion, (KEY_PORTION_LENGTH - 1), "%u", hashValue);
}

// RANGE_MISSED event parsing

static void *parseRangeMissed(WowParser *parser)
{
	int spellSchool = 0;

	WowEventRangeMissed *result = NULL;

	if(parser->tokenCount < 13) {
		return NULL;
	}

	if((spellSchool = determineSpellSchool(parser, 11)) < 0) {
		return NULL;
	}

	result = (WowEventRangeMissed *)malloc(sizeof(WowEventRangeMissed));

	result->spellId = atoi(parser->tokens[9]);
	result->spellName = strndup(parser->tokens[10], parser->tokenLengths[10]);
	result->spellSchool = spellSchool;
	result->missType = strndup(parser->tokens[12], parser->tokenLengths[12]);

	if(parser->tokenCount > 13) {
		result->missAmount = atoi(parser->tokens[13]);
	}
	else {
		result->missAmount = 0;
	}

/*
	printf("\t "
			"DETAIL[%s]{%i, '%s', '%s', '%s', %i}\n",
			WOW_EVENT_LIST[(WOW_EVENT_RANGE_DAMAGE - 1)].name, result->spellId,
			result->spellName,
			WOW_SPELL_SCHOOL[(result->spellSchool - 1)].name,
			result->missType, result->missAmount);
*/

	return result;
}

static void freeRangeMissed(WowEventRangeMissed *event)
{
	if(event->spellName != NULL) {
		free(event->spellName);
	}

	if(event->missType != NULL) {
		free(event->missType);
	}

	free(event);
}

static void buildKeyPortionRangeMissed(WowEventRangeMissed *event,
		char keyPortion[KEY_PORTION_LENGTH])
{
	unsigned int hashValue = 0;
	char buffer[BUFFER_LENGTH];

	snprintf(buffer, BUFFER_SNPRINTF_LENGTH,
			"%i/%s/%i/%s/%i",
			event->spellId, event->spellName, event->spellSchool,
			event->missType, event->missAmount);

	hashValue = crc32_calculateHash(buffer, strlen(buffer));

	snprintf(keyPortion, (KEY_PORTION_LENGTH - 1), "%u", hashValue);
}

// SPELL_AURA_APPLIED event parsing

static void *parseSpellAuraApplied(WowParser *parser)
{
	int spellSchool = 0;

	WowEventSpellAuraApplied *result = NULL;

	if(parser->tokenCount < 13) {
		return NULL;
	}

	if((spellSchool = determineSpellSchool(parser, 11)) < 0) {
		return NULL;
	}

	result = (WowEventSpellAuraApplied *)malloc(
			sizeof(WowEventSpellAuraApplied));

	result->spellId = atoi(parser->tokens[9]);
	result->spellName = strndup(parser->tokens[10], parser->tokenLengths[10]);
	result->spellSchool = spellSchool;
	result->auraType = strndup(parser->tokens[12], parser->tokenLengths[12]);

/*
	printf("\t "
			"DETAIL[%s]{%i, '%s', '%s', '%s'}\n",
			WOW_EVENT_LIST[(WOW_EVENT_SPELL_AURA_APPLIED - 1)].name,
			result->spellId, result->spellName,
			WOW_SPELL_SCHOOL[(result->spellSchool - 1)].name,
			result->auraType);
*/

	return result;
}

static void freeSpellAuraApplied(WowEventSpellAuraApplied *event)
{
	if(event->spellName != NULL) {
		free(event->spellName);
	}

	if(event->auraType != NULL) {
		free(event->auraType);
	}

	free(event);
}

static void buildKeyPortionSpellAuraApplied(WowEventSpellAuraApplied *event,
		char keyPortion[KEY_PORTION_LENGTH])
{
	unsigned int hashValue = 0;
	char buffer[BUFFER_LENGTH];

	snprintf(buffer, BUFFER_SNPRINTF_LENGTH,
			"%i/%s/%i/%s",
			event->spellId, event->spellName, event->spellSchool,
			event->auraType);

	hashValue = crc32_calculateHash(buffer, strlen(buffer));

	snprintf(keyPortion, (KEY_PORTION_LENGTH - 1), "%u", hashValue);
}

// SPELL_AURA_APPLIED_DOSE event parsing

static void *parseSpellAuraAppliedDose(WowParser *parser)
{
	int spellSchool = 0;

	WowEventSpellAuraAppliedDose *result = NULL;

	if(parser->tokenCount < 14) {
		return NULL;
	}

	if((spellSchool = determineSpellSchool(parser, 11)) < 0) {
		return NULL;
	}

	result = (WowEventSpellAuraAppliedDose *)malloc(
			sizeof(WowEventSpellAuraAppliedDose));

	result->spellId = atoi(parser->tokens[9]);
	result->spellName = strndup(parser->tokens[10], parser->tokenLengths[10]);
	result->spellSchool = spellSchool;
	result->auraType = strndup(parser->tokens[12], parser->tokenLengths[12]);
	result->auraAmount = atoi(parser->tokens[13]);

/*
	printf("\t "
			"DETAIL[%s]{%i, '%s', '%s', '%s', %i}\n",
			WOW_EVENT_LIST[(WOW_EVENT_SPELL_AURA_APPLIED_DOSE - 1)].name,
			result->spellId, result->spellName,
			WOW_SPELL_SCHOOL[(result->spellSchool - 1)].name,
			result->auraType, result->auraAmount);
*/

	return result;
}

static void freeSpellAuraAppliedDose(WowEventSpellAuraAppliedDose *event)
{
	if(event->spellName != NULL) {
		free(event->spellName);
	}

	if(event->auraType != NULL) {
		free(event->auraType);
	}

	free(event);
}

static void buildKeyPortionSpellAuraAppliedDose(
		WowEventSpellAuraAppliedDose *event,
		char keyPortion[KEY_PORTION_LENGTH])
{
	unsigned int hashValue = 0;
	char buffer[BUFFER_LENGTH];

	snprintf(buffer, BUFFER_SNPRINTF_LENGTH,
			"%i/%s/%i/%s/%i",
			event->spellId, event->spellName, event->spellSchool,
			event->auraType, event->auraAmount);

	hashValue = crc32_calculateHash(buffer, strlen(buffer));

	snprintf(keyPortion, (KEY_PORTION_LENGTH - 1), "%u", hashValue);
}

// SPELL_AURA_BROKEN event parsing

static void *parseSpellAuraBroken(WowParser *parser)
{
	int spellSchool = 0;

	WowEventSpellAuraBroken *result = NULL;

	if(parser->tokenCount < 13) {
		return NULL;
	}

	if((spellSchool = determineSpellSchool(parser, 11)) < 0) {
		return NULL;
	}

	result = (WowEventSpellAuraBroken *)malloc(
			sizeof(WowEventSpellAuraBroken));

	result->spellId = atoi(parser->tokens[9]);
	result->spellName = strndup(parser->tokens[10], parser->tokenLengths[10]);
	result->spellSchool = spellSchool;
	result->auraType = strndup(parser->tokens[12], parser->tokenLengths[12]);

/*
	printf("\t "
			"DETAIL[%s]{%i, '%s', '%s', '%s'}\n",
			WOW_EVENT_LIST[(WOW_EVENT_SPELL_AURA_BROKEN - 1)].name,
			result->spellId, result->spellName,
			WOW_SPELL_SCHOOL[(result->spellSchool - 1)].name,
			result->auraType);
*/

	return result;
}

static void freeSpellAuraBroken(WowEventSpellAuraBroken *event)
{
	if(event->spellName != NULL) {
		free(event->spellName);
	}

	if(event->auraType != NULL) {
		free(event->auraType);
	}

	free(event);
}

static void buildKeyPortionSpellAuraBroken(WowEventSpellAuraBroken *event,
		char keyPortion[KEY_PORTION_LENGTH])
{
	unsigned int hashValue = 0;
	char buffer[BUFFER_LENGTH];

	snprintf(buffer, BUFFER_SNPRINTF_LENGTH,
			"%i/%s/%i/%s",
			event->spellId, event->spellName, event->spellSchool,
			event->auraType);

	hashValue = crc32_calculateHash(buffer, strlen(buffer));

	snprintf(keyPortion, (KEY_PORTION_LENGTH - 1), "%u", hashValue);
}

// SPELL_AURA_BROKEN_SPELL event parsing

static void *parseSpellAuraBrokenSpell(WowParser *parser)
{
	int spellSchool = 0;

	WowEventSpellAuraBrokenSpell *result = NULL;

	if(parser->tokenCount < 16) {
		return NULL;
	}

	if((spellSchool = determineSpellSchool(parser, 11)) < 0) {
		return NULL;
	}

	result = (WowEventSpellAuraBrokenSpell *)malloc(
			sizeof(WowEventSpellAuraBrokenSpell));

	result->spellId = atoi(parser->tokens[9]);
	result->spellName = strndup(parser->tokens[10], parser->tokenLengths[10]);
	result->spellSchool = spellSchool;
	result->breakingSpellId = atoi(parser->tokens[12]);
	result->breakingSpellName = strndup(parser->tokens[13],
			parser->tokenLengths[13]);
	result->breakingSpellSchool = atoi(parser->tokens[14]);
	result->auraType = strndup(parser->tokens[15], parser->tokenLengths[15]);

/*
	printf("\t "
			"DETAIL[%s]{%i, '%s', '%s', %i, '%s', %i, '%s'}\n",
			WOW_EVENT_LIST[(WOW_EVENT_SPELL_AURA_BROKEN_SPELL - 1)].name,
			result->spellId, result->spellName,
			WOW_SPELL_SCHOOL[(result->spellSchool - 1)].name,
			result->breakingSpellId, result->breakingSpellName,
			result->breakingSpellSchool, result->auraType);
*/

	return result;
}

static void freeSpellAuraBrokenSpell(WowEventSpellAuraBrokenSpell *event)
{
	if(event->spellName != NULL) {
		free(event->spellName);
	}

	if(event->breakingSpellName != NULL) {
		free(event->breakingSpellName);
	}

	if(event->auraType != NULL) {
		free(event->auraType);
	}

	free(event);
}

static void buildKeyPortionSpellAuraBrokenSpell(
		WowEventSpellAuraBrokenSpell *event,
		char keyPortion[KEY_PORTION_LENGTH])
{
	unsigned int hashValue = 0;
	char buffer[BUFFER_LENGTH];

	snprintf(buffer, BUFFER_SNPRINTF_LENGTH,
			"%i/%s/%i/%i/%s/%i/%s",
			event->spellId, event->spellName, event->spellSchool,
			event->breakingSpellId, event->breakingSpellName,
			event->breakingSpellSchool, event->auraType);

	hashValue = crc32_calculateHash(buffer, strlen(buffer));

	snprintf(keyPortion, (KEY_PORTION_LENGTH - 1), "%u", hashValue);
}

// SPELL_AURA_REFRESH event parsing

static void *parseSpellAuraRefresh(WowParser *parser)
{
	int spellSchool = 0;

	WowEventSpellAuraRefresh *result = NULL;

	if(parser->tokenCount < 13) {
		return NULL;
	}

	if((spellSchool = determineSpellSchool(parser, 11)) < 0) {
		return NULL;
	}

	result = (WowEventSpellAuraRefresh *)malloc(
			sizeof(WowEventSpellAuraRefresh));

	result->spellId = atoi(parser->tokens[9]);
	result->spellName = strndup(parser->tokens[10], parser->tokenLengths[10]);
	result->spellSchool = spellSchool;
	result->auraType = strndup(parser->tokens[12], parser->tokenLengths[12]);

/*
	printf("\t "
			"DETAIL[%s]{%i, '%s', '%s', '%s'}\n",
			WOW_EVENT_LIST[(WOW_EVENT_SPELL_AURA_REFRESH - 1)].name,
			result->spellId, result->spellName,
			WOW_SPELL_SCHOOL[(result->spellSchool - 1)].name,
			result->auraType);
*/

	return result;
}

static void freeSpellAuraRefresh(WowEventSpellAuraRefresh *event)
{
	if(event->spellName != NULL) {
		free(event->spellName);
	}

	if(event->auraType != NULL) {
		free(event->auraType);
	}

	free(event);
}

static void buildKeyPortionSpellAuraRefresh(WowEventSpellAuraRefresh *event,
		char keyPortion[KEY_PORTION_LENGTH])
{
	unsigned int hashValue = 0;
	char buffer[BUFFER_LENGTH];

	snprintf(buffer, BUFFER_SNPRINTF_LENGTH,
			"%i/%s/%i/%s",
			event->spellId, event->spellName, event->spellSchool,
			event->auraType);

	hashValue = crc32_calculateHash(buffer, strlen(buffer));

	snprintf(keyPortion, (KEY_PORTION_LENGTH - 1), "%u", hashValue);
}

// SPELL_AURA_REMOVED event parsing

static void *parseSpellAuraRemoved(WowParser *parser)
{
	int spellSchool = 0;

	WowEventSpellAuraRemoved *result = NULL;

	if(parser->tokenCount < 13) {
		return NULL;
	}

	if((spellSchool = determineSpellSchool(parser, 11)) < 0) {
		return NULL;
	}

	result = (WowEventSpellAuraRemoved *)malloc(
			sizeof(WowEventSpellAuraRemoved));

	result->spellId = atoi(parser->tokens[9]);
	result->spellName = strndup(parser->tokens[10], parser->tokenLengths[10]);
	result->spellSchool = spellSchool;
	result->auraType = strndup(parser->tokens[12], parser->tokenLengths[12]);

/*
	printf("\t "
			"DETAIL[%s]{%i, '%s', '%s', '%s'}\n",
			WOW_EVENT_LIST[(WOW_EVENT_SPELL_AURA_REMOVED - 1)].name,
			result->spellId, result->spellName,
			WOW_SPELL_SCHOOL[(result->spellSchool - 1)].name,
			result->auraType);
*/

	return result;
}

static void freeSpellAuraRemoved(WowEventSpellAuraRemoved *event)
{
	if(event->spellName != NULL) {
		free(event->spellName);
	}

	if(event->auraType != NULL) {
		free(event->auraType);
	}

	free(event);
}

static void buildKeyPortionSpellAuraRemoved(WowEventSpellAuraRemoved *event,
		char keyPortion[KEY_PORTION_LENGTH])
{
	unsigned int hashValue = 0;
	char buffer[BUFFER_LENGTH];

	snprintf(buffer, BUFFER_SNPRINTF_LENGTH,
			"%i/%s/%i/%s",
			event->spellId, event->spellName, event->spellSchool,
			event->auraType);

	hashValue = crc32_calculateHash(buffer, strlen(buffer));

	snprintf(keyPortion, (KEY_PORTION_LENGTH - 1), "%u", hashValue);
}

// SPELL_AURA_REMOVED_DOSE event parsing

static void *parseSpellAuraRemovedDose(WowParser *parser)
{
	int spellSchool = 0;

	WowEventSpellAuraRemovedDose *result = NULL;

	if(parser->tokenCount < 14) {
		return NULL;
	}

	if((spellSchool = determineSpellSchool(parser, 11)) < 0) {
		return NULL;
	}

	result = (WowEventSpellAuraRemovedDose *)malloc(
			sizeof(WowEventSpellAuraRemovedDose));

	result->spellId = atoi(parser->tokens[9]);
	result->spellName = strndup(parser->tokens[10], parser->tokenLengths[10]);
	result->spellSchool = spellSchool;
	result->auraType = strndup(parser->tokens[12], parser->tokenLengths[12]);
	result->auraAmount = atoi(parser->tokens[13]);

/*
	printf("\t "
			"DETAIL[%s]{%i, '%s', '%s', '%s', %i}\n",
			WOW_EVENT_LIST[(WOW_EVENT_SPELL_AURA_REMOVED_DOSE - 1)].name,
			result->spellId, result->spellName,
			WOW_SPELL_SCHOOL[(result->spellSchool - 1)].name,
			result->auraType, result->auraAmount);
*/

	return result;
}

static void freeSpellAuraRemovedDose(WowEventSpellAuraRemovedDose *event)
{
	if(event->spellName != NULL) {
		free(event->spellName);
	}

	if(event->auraType != NULL) {
		free(event->auraType);
	}

	free(event);
}

static void buildKeyPortionSpellAuraRemovedDose(
		WowEventSpellAuraRemovedDose *event,
		char keyPortion[KEY_PORTION_LENGTH])
{
	unsigned int hashValue = 0;
	char buffer[BUFFER_LENGTH];

	snprintf(buffer, BUFFER_SNPRINTF_LENGTH,
			"%i/%s/%i/%s/%i",
			event->spellId, event->spellName, event->spellSchool,
			event->auraType, event->auraAmount);

	hashValue = crc32_calculateHash(buffer, strlen(buffer));

	snprintf(keyPortion, (KEY_PORTION_LENGTH - 1), "%u", hashValue);
}

// SPELL_CAST_FAILED event parsing

static void *parseSpellCastFailed(WowParser *parser)
{
	int spellSchool = 0;

	WowEventSpellCastFailed *result = NULL;

	if(parser->tokenCount < 13) {
		return NULL;
	}

	if((spellSchool = determineSpellSchool(parser, 11)) < 0) {
		return NULL;
	}

	result = (WowEventSpellCastFailed *)malloc(
			sizeof(WowEventSpellCastFailed));

	result->spellId = atoi(parser->tokens[9]);
	result->spellName = strndup(parser->tokens[10], parser->tokenLengths[10]);
	result->spellSchool = spellSchool;
	result->reason = strndup(parser->tokens[12], parser->tokenLengths[12]);

/*
	printf("\t "
			"DETAIL[%s]{%i, '%s', '%s', '%s'}\n",
			WOW_EVENT_LIST[(WOW_EVENT_SPELL_CAST_FAILED - 1)].name,
			result->spellId, result->spellName,
			WOW_SPELL_SCHOOL[(result->spellSchool - 1)].name,
			result->reason);
*/

	return result;
}

static void freeSpellCastFailed(WowEventSpellCastFailed *event)
{
	if(event->spellName != NULL) {
		free(event->spellName);
	}

	if(event->reason != NULL) {
		free(event->reason);
	}

	free(event);
}

static void buildKeyPortionSpellCastFailed(WowEventSpellCastFailed *event,
		char keyPortion[KEY_PORTION_LENGTH])
{
	unsigned int hashValue = 0;
	char buffer[BUFFER_LENGTH];

	snprintf(buffer, BUFFER_SNPRINTF_LENGTH,
			"%i/%s/%i/%s",
			event->spellId, event->spellName, event->spellSchool,
			event->reason);

	hashValue = crc32_calculateHash(buffer, strlen(buffer));

	snprintf(keyPortion, (KEY_PORTION_LENGTH - 1), "%u", hashValue);
}

// SPELL_CAST_START event parsing

static void *parseSpellCastStart(WowParser *parser)
{
	int spellSchool = 0;

	WowEventSpellCastStart *result = NULL;

	if(parser->tokenCount < 12) {
		return NULL;
	}

	if((spellSchool = determineSpellSchool(parser, 11)) < 0) {
		return NULL;
	}

	result = (WowEventSpellCastStart *)malloc(sizeof(WowEventSpellCastStart));

	result->spellId = atoi(parser->tokens[9]);
	result->spellName = strndup(parser->tokens[10], parser->tokenLengths[10]);
	result->spellSchool = spellSchool;

/*
	printf("\t "
			"DETAIL[%s]{%i, '%s', '%s'}\n",
			WOW_EVENT_LIST[(WOW_EVENT_SPELL_CAST_START - 1)].name,
			result->spellId, result->spellName,
			WOW_SPELL_SCHOOL[(result->spellSchool - 1)].name);
*/

	return result;
}

static void freeSpellCastStart(WowEventSpellCastStart *event)
{
	if(event->spellName != NULL) {
		free(event->spellName);
	}

	free(event);
}

static void buildKeyPortionSpellCastStart(WowEventSpellCastStart *event,
		char keyPortion[KEY_PORTION_LENGTH])
{
	unsigned int hashValue = 0;
	char buffer[BUFFER_LENGTH];

	snprintf(buffer, BUFFER_SNPRINTF_LENGTH,
			"%i/%s/%i",
			event->spellId, event->spellName, event->spellSchool);

	hashValue = crc32_calculateHash(buffer, strlen(buffer));

	snprintf(keyPortion, (KEY_PORTION_LENGTH - 1), "%u", hashValue);
}

// SPELL_CAST_SUCCESS event parsing

static void *parseSpellCastSuccess(WowParser *parser)
{
	int spellSchool = 0;

	WowEventSpellCastSuccess *result = NULL;

	if(parser->tokenCount < 12) {
		return NULL;
	}

	if((spellSchool = determineSpellSchool(parser, 11)) < 0) {
		return NULL;
	}

	result = (WowEventSpellCastSuccess *)malloc(
			sizeof(WowEventSpellCastSuccess));

	result->spellId = atoi(parser->tokens[9]);
	result->spellName = strndup(parser->tokens[10], parser->tokenLengths[10]);
	result->spellSchool = spellSchool;

/*
	printf("\t "
			"DETAIL[%s]{%i, '%s', '%s'}\n",
			WOW_EVENT_LIST[(WOW_EVENT_SPELL_CAST_SUCCESS - 1)].name,
			result->spellId, result->spellName,
			WOW_SPELL_SCHOOL[(result->spellSchool - 1)].name);
*/

	return result;
}

static void freeSpellCastSuccess(WowEventSpellCastSuccess *event)
{
	if(event->spellName != NULL) {
		free(event->spellName);
	}

	free(event);
}

static void buildKeyPortionSpellCastSuccess(WowEventSpellCastSuccess *event,
		char keyPortion[KEY_PORTION_LENGTH])
{
	unsigned int hashValue = 0;
	char buffer[BUFFER_LENGTH];

	snprintf(buffer, BUFFER_SNPRINTF_LENGTH,
			"%i/%s/%i",
			event->spellId, event->spellName, event->spellSchool);

	hashValue = crc32_calculateHash(buffer, strlen(buffer));

	snprintf(keyPortion, (KEY_PORTION_LENGTH - 1), "%u", hashValue);
}

// SPELL_CREATE event parsing

static void *parseSpellCreate(WowParser *parser)
{
	int spellSchool = 0;

	WowEventSpellCreate *result = NULL;

	if(parser->tokenCount < 12) {
		return NULL;
	}

	if((spellSchool = determineSpellSchool(parser, 11)) < 0) {
		return NULL;
	}

	result = (WowEventSpellCreate *)malloc(sizeof(WowEventSpellCreate));

	result->spellId = atoi(parser->tokens[9]);
	result->spellName = strndup(parser->tokens[10], parser->tokenLengths[10]);
	result->spellSchool = spellSchool;

/*
	printf("\t "
			"DETAIL[%s]{%i, '%s', '%s'}\n",
			WOW_EVENT_LIST[(WOW_EVENT_SPELL_CREATE - 1)].name,
			result->spellId, result->spellName,
			WOW_SPELL_SCHOOL[(result->spellSchool - 1)].name);
*/

	return result;
}

static void freeSpellCreate(WowEventSpellCreate *event)
{
	if(event->spellName != NULL) {
		free(event->spellName);
	}

	free(event);
}

static void buildKeyPortionSpellCreate(WowEventSpellCreate *event,
		char keyPortion[KEY_PORTION_LENGTH])
{
	unsigned int hashValue = 0;
	char buffer[BUFFER_LENGTH];

	snprintf(buffer, BUFFER_SNPRINTF_LENGTH,
			"%i/%s/%i",
			event->spellId, event->spellName, event->spellSchool);

	hashValue = crc32_calculateHash(buffer, strlen(buffer));

	snprintf(keyPortion, (KEY_PORTION_LENGTH - 1), "%u", hashValue);
}

// SPELL_DAMAGE event parsing

static void *parseSpellDamage(WowParser *parser)
{
	int spellSchool = 0;

	WowEventSpellDamage *result = NULL;

	if(parser->tokenCount < 21) {
		return NULL;
	}

	if((spellSchool = determineSpellSchool(parser, 11)) < 0) {
		return NULL;
	}

	result = (WowEventSpellDamage *)malloc(sizeof(WowEventSpellDamage));

	result->spellId = atoi(parser->tokens[9]);
	result->spellName = strndup(parser->tokens[10], parser->tokenLengths[10]);
	result->spellSchool = spellSchool;
	result->damageAmount = atoi(parser->tokens[12]);
	result->overkillAmount = atoi(parser->tokens[13]);
	result->school = spellSchool; // TODO: verify this
	result->resistAmount = atoi(parser->tokens[15]);
	result->blockAmount = atoi(parser->tokens[16]);

	if(!strncmp(parser->tokens[17], "0", 1)) {
		result->absorbed = afalse;
	}
	else {
		result->absorbed = atrue;
	}

	if(!strncmp(parser->tokens[18], "1", 1)) {
		result->wasCritical = atrue;
	}
	else {
		result->wasCritical = afalse;
	}

	if(!strncmp(parser->tokens[19], "1", 1)) {
		result->glancing = atrue;
	}
	else {
		result->glancing = afalse;
	}

	if(!strncmp(parser->tokens[20], "1", 1)) {
		result->crushing = atrue;
	}
	else {
		result->crushing = afalse;
	}

/*
	printf("\t "
			"DETAIL[%s]{%i, '%s', '%s', %i, %i, %i, %i, %i, %i, %i, %i, %i}\n",
			WOW_EVENT_LIST[(WOW_EVENT_SPELL_DAMAGE - 1)].name, result->spellId,
			result->spellName,
			WOW_SPELL_SCHOOL[(result->spellSchool - 1)].name,
			result->damageAmount, result->overkillAmount, result->school,
			result->resistAmount, result->blockAmount, result->absorbed,
			result->wasCritical, result->glancing, result->crushing);
*/

	return result;
}

static void freeSpellDamage(WowEventSpellDamage *event)
{
	if(event->spellName != NULL) {
		free(event->spellName);
	}

	free(event);
}

static void buildKeyPortionSpellDamage(WowEventSpellDamage *event,
		char keyPortion[KEY_PORTION_LENGTH])
{
	unsigned int hashValue = 0;
	char buffer[BUFFER_LENGTH];

	snprintf(buffer, BUFFER_SNPRINTF_LENGTH,
			"%i/%s/%i/%i/%i/%i/%i/%i/%i/%i/%i/%i",
			event->spellId, event->spellName, event->spellSchool,
			event->damageAmount, event->overkillAmount, event->school,
			event->resistAmount, event->blockAmount, event->absorbed,
			event->wasCritical, event->glancing, event->crushing);

	hashValue = crc32_calculateHash(buffer, strlen(buffer));

	snprintf(keyPortion, (KEY_PORTION_LENGTH - 1), "%u", hashValue);
}

// SPELL_DISPEL event parsing

static void *parseSpellDispel(WowParser *parser)
{
	int spellSchool = 0;

	WowEventSpellDispel *result = NULL;

	if(parser->tokenCount < 16) {
		return NULL;
	}

	if((spellSchool = determineSpellSchool(parser, 11)) < 0) {
		return NULL;
	}

	result = (WowEventSpellDispel *)malloc(sizeof(WowEventSpellDispel));

	result->spellId = atoi(parser->tokens[9]);
	result->spellName = strndup(parser->tokens[10], parser->tokenLengths[10]);
	result->spellSchool = spellSchool;
	result->dispelSpellId = atoi(parser->tokens[12]);
	result->dispelSpellName = strndup(parser->tokens[13],
			parser->tokenLengths[13]);
	result->dispelSpellSchool = atoi(parser->tokens[14]);
	result->auraType = strndup(parser->tokens[15], parser->tokenLengths[15]);

/*
	printf("\t "
			"DETAIL[%s]{%i, '%s', '%s', %i, '%s', %i, '%s'}\n",
			WOW_EVENT_LIST[(WOW_EVENT_SPELL_DISPEL - 1)].name,
			result->spellId, result->spellName,
			WOW_SPELL_SCHOOL[(result->spellSchool - 1)].name,
			result->dispelSpellId, result->dispelSpellName,
			result->dispelSpellSchool, result->auraType);
*/

	return result;
}

static void freeSpellDispel(WowEventSpellDispel *event)
{
	if(event->spellName != NULL) {
		free(event->spellName);
	}

	if(event->dispelSpellName != NULL) {
		free(event->dispelSpellName);
	}

	if(event->auraType != NULL) {
		free(event->auraType);
	}

	free(event);
}

static void buildKeyPortionSpellDispel(WowEventSpellDispel *event,
		char keyPortion[KEY_PORTION_LENGTH])
{
	unsigned int hashValue = 0;
	char buffer[BUFFER_LENGTH];

	snprintf(buffer, BUFFER_SNPRINTF_LENGTH,
			"%i/%s/%i/%i/%s/%i/%s",
			event->spellId, event->spellName, event->spellSchool,
			event->dispelSpellId, event->dispelSpellName,
			event->dispelSpellSchool, event->auraType);

	hashValue = crc32_calculateHash(buffer, strlen(buffer));

	snprintf(keyPortion, (KEY_PORTION_LENGTH - 1), "%u", hashValue);
}

// SPELL_DISPEL_FAILED event parsing

static void *parseSpellDispelFailed(WowParser *parser)
{
	int spellSchool = 0;

	WowEventSpellDispelFailed *result = NULL;

	if(parser->tokenCount < 15) {
		return NULL;
	}

	if((spellSchool = determineSpellSchool(parser, 11)) < 0) {
		return NULL;
	}

	result = (WowEventSpellDispelFailed *)malloc(
			sizeof(WowEventSpellDispelFailed));

	result->spellId = atoi(parser->tokens[9]);
	result->spellName = strndup(parser->tokens[10], parser->tokenLengths[10]);
	result->spellSchool = spellSchool;
	result->dispelSpellId = atoi(parser->tokens[12]);
	result->dispelSpellName = strndup(parser->tokens[13],
			parser->tokenLengths[13]);
	result->dispelSpellSchool = atoi(parser->tokens[14]);

/*
	printf("\t "
			"DETAIL[%s]{%i, '%s', '%s', %i, '%s', %i}\n",
			WOW_EVENT_LIST[(WOW_EVENT_SPELL_DISPEL_FAILED - 1)].name,
			result->spellId, result->spellName,
			WOW_SPELL_SCHOOL[(result->spellSchool - 1)].name,
			result->dispelSpellId, result->dispelSpellName,
			result->dispelSpellSchool);
*/

	return result;
}

static void freeSpellDispelFailed(WowEventSpellDispelFailed *event)
{
	if(event->spellName != NULL) {
		free(event->spellName);
	}

	if(event->dispelSpellName != NULL) {
		free(event->dispelSpellName);
	}

	free(event);
}

static void buildKeyPortionSpellDispelFailed(WowEventSpellDispelFailed *event,
		char keyPortion[KEY_PORTION_LENGTH])
{
	unsigned int hashValue = 0;
	char buffer[BUFFER_LENGTH];

	snprintf(buffer, BUFFER_SNPRINTF_LENGTH,
			"%i/%s/%i/%i/%s/%i",
			event->spellId, event->spellName, event->spellSchool,
			event->dispelSpellId, event->dispelSpellName,
			event->dispelSpellSchool);

	hashValue = crc32_calculateHash(buffer, strlen(buffer));

	snprintf(keyPortion, (KEY_PORTION_LENGTH - 1), "%u", hashValue);
}

// SPELL_ENERGIZE event parsing

static void *parseSpellEnergize(WowParser *parser)
{
	int spellSchool = 0;

	WowEventSpellEnergize *result = NULL;

	if(parser->tokenCount < 14) {
		return NULL;
	}

	if((spellSchool = determineSpellSchool(parser, 11)) < 0) {
		return NULL;
	}

	result = (WowEventSpellEnergize *)malloc(sizeof(WowEventSpellEnergize));

	result->spellId = atoi(parser->tokens[9]);
	result->spellName = strndup(parser->tokens[10], parser->tokenLengths[10]);
	result->spellSchool = spellSchool;
	result->energizeAmount = atoi(parser->tokens[12]);
	result->powerType = strdup(determinePowerType(parser, 13));

	if(parser->tokenCount > 14) {
		result->extraEnergizeAmount = atoi(parser->tokens[14]);
	}
	else {
		result->extraEnergizeAmount = 0;
	}

/*
	printf("\t "
			"DETAIL[%s]{%i, '%s', '%s', %i, '%s', %i}\n",
			WOW_EVENT_LIST[(WOW_EVENT_SPELL_ENERGIZE - 1)].name,
			result->spellId, result->spellName,
			WOW_SPELL_SCHOOL[(result->spellSchool - 1)].name,
			result->energizeAmount, result->powerType,
			result->extraEnergizeAmount);
*/

	return result;
}

static void freeSpellEnergize(WowEventSpellEnergize *event)
{
	if(event->spellName != NULL) {
		free(event->spellName);
	}

	if(event->powerType != NULL) {
		free(event->powerType);
	}

	free(event);
}

static void buildKeyPortionSpellEnergize(WowEventSpellEnergize *event,
		char keyPortion[KEY_PORTION_LENGTH])
{
	unsigned int hashValue = 0;
	char buffer[BUFFER_LENGTH];

	snprintf(buffer, BUFFER_SNPRINTF_LENGTH,
			"%i/%s/%i/%i/%s/%i",
			event->spellId, event->spellName, event->spellSchool,
			event->energizeAmount, event->powerType,
			event->extraEnergizeAmount);

	hashValue = crc32_calculateHash(buffer, strlen(buffer));

	snprintf(keyPortion, (KEY_PORTION_LENGTH - 1), "%u", hashValue);
}

// SPELL_EXTRA_ATTACKS event parsing

static void *parseSpellExtraAttacks(WowParser *parser)
{
	int spellSchool = 0;

	WowEventSpellExtraAttacks *result = NULL;

	if(parser->tokenCount < 13) {
		return NULL;
	}

	if((spellSchool = determineSpellSchool(parser, 11)) < 0) {
		return NULL;
	}

	result = (WowEventSpellExtraAttacks *)malloc(
			sizeof(WowEventSpellExtraAttacks));

	result->spellId = atoi(parser->tokens[9]);
	result->spellName = strndup(parser->tokens[10], parser->tokenLengths[10]);
	result->spellSchool = spellSchool;
	result->extraAttackAmount = atoi(parser->tokens[12]);

/*
	printf("\t "
			"DETAIL[%s]{%i, '%s', '%s', %i}\n",
			WOW_EVENT_LIST[(WOW_EVENT_SPELL_EXTRA_ATTACKS - 1)].name,
			result->spellId, result->spellName,
			WOW_SPELL_SCHOOL[(result->spellSchool - 1)].name,
			result->extraAttackAmount);
*/

	return result;
}

static void freeSpellExtraAttacks(WowEventSpellExtraAttacks *event)
{
	if(event->spellName != NULL) {
		free(event->spellName);
	}

	free(event);
}

static void buildKeyPortionSpellExtraAttacks(WowEventSpellExtraAttacks *event,
		char keyPortion[KEY_PORTION_LENGTH])
{
	unsigned int hashValue = 0;
	char buffer[BUFFER_LENGTH];

	snprintf(buffer, BUFFER_SNPRINTF_LENGTH,
			"%i/%s/%i/%i",
			event->spellId, event->spellName, event->spellSchool,
			event->extraAttackAmount);

	hashValue = crc32_calculateHash(buffer, strlen(buffer));

	snprintf(keyPortion, (KEY_PORTION_LENGTH - 1), "%u", hashValue);
}

// SPELL_HEAL event parsing

static void *parseSpellHealed(WowParser *parser)
{
	int spellSchool = 0;

	WowEventSpellHealed *result = NULL;

	if(parser->tokenCount < 15) {
		return NULL;
	}

	if((spellSchool = determineSpellSchool(parser, 11)) < 0) {
		return NULL;
	}

	result = (WowEventSpellHealed *)malloc(sizeof(WowEventSpellHealed));

	result->spellId = atoi(parser->tokens[9]);
	result->spellName = strndup(parser->tokens[10], parser->tokenLengths[10]);
	result->spellSchool = spellSchool;
	result->healAmount = atoi(parser->tokens[12]);
	result->overHealingAmount = atoi(parser->tokens[13]);

	if(!strncmp(parser->tokens[14], "1", 1)) {
		result->wasCritical = atrue;
	}
	else {
		result->wasCritical = afalse;
	}
/*
	printf("\t DETAIL[%s]{%i, '%s', '%s', %i, %i, %i}\n",
			WOW_EVENT_LIST[(WOW_EVENT_SPELL_HEAL - 1)].name, result->spellId,
			result->spellName,
			WOW_SPELL_SCHOOL[(result->spellSchool - 1)].name,
			result->healAmount, result->overHealingAmount,
			result->wasCritical);
*/
	return result;
}

static void freeSpellHealed(WowEventSpellHealed *event)
{
	if(event->spellName != NULL) {
		free(event->spellName);
	}

	free(event);
}

static void buildKeyPortionSpellHealed(WowEventSpellHealed *event,
		char keyPortion[KEY_PORTION_LENGTH])
{
	unsigned int hashValue = 0;
	char buffer[BUFFER_LENGTH];

	snprintf(buffer, BUFFER_SNPRINTF_LENGTH,
			"%i/%s/%i/%i/%i/%i",
			event->spellId, event->spellName, event->spellSchool,
			event->healAmount, event->overHealingAmount, event->wasCritical);

	hashValue = crc32_calculateHash(buffer, strlen(buffer));

	snprintf(keyPortion, (KEY_PORTION_LENGTH - 1), "%u", hashValue);
}

// SPELL_INSTAKILL event parsing

static void *parseSpellInstakill(WowParser *parser)
{
	int spellSchool = 0;

	WowEventSpellInstakill *result = NULL;

	if(parser->tokenCount < 12) {
		return NULL;
	}

	if((spellSchool = determineSpellSchool(parser, 11)) < 0) {
		return NULL;
	}

	result = (WowEventSpellInstakill *)malloc(sizeof(WowEventSpellInstakill));

	result->spellId = atoi(parser->tokens[9]);
	result->spellName = strndup(parser->tokens[10], parser->tokenLengths[10]);
	result->spellSchool = spellSchool;

/*
	printf("\t "
			"DETAIL[%s]{%i, '%s', '%s'}\n",
			WOW_EVENT_LIST[(WOW_EVENT_SPELL_INSTAKILL - 1)].name,
			result->spellId, result->spellName,
			WOW_SPELL_SCHOOL[(result->spellSchool - 1)].name);
*/

	return result;
}

static void freeSpellInstakill(WowEventSpellInstakill *event)
{
	if(event->spellName != NULL) {
		free(event->spellName);
	}

	free(event);
}

static void buildKeyPortionSpellInstakill(WowEventSpellInstakill *event,
		char keyPortion[KEY_PORTION_LENGTH])
{
	unsigned int hashValue = 0;
	char buffer[BUFFER_LENGTH];

	snprintf(buffer, BUFFER_SNPRINTF_LENGTH,
			"%i/%s/%i",
			event->spellId, event->spellName, event->spellSchool);

	hashValue = crc32_calculateHash(buffer, strlen(buffer));

	snprintf(keyPortion, (KEY_PORTION_LENGTH - 1), "%u", hashValue);
}

// SPELL_INTERRUPT event parsing

static void *parseSpellInterrupt(WowParser *parser)
{
	int spellSchool = 0;

	WowEventSpellInterrupt *result = NULL;

	if(parser->tokenCount < 15) {
		return NULL;
	}

	if((spellSchool = determineSpellSchool(parser, 11)) < 0) {
		return NULL;
	}

	result = (WowEventSpellInterrupt *)malloc(sizeof(WowEventSpellInterrupt));

	result->spellId = atoi(parser->tokens[9]);
	result->spellName = strndup(parser->tokens[10], parser->tokenLengths[10]);
	result->spellSchool = spellSchool;
	result->interruptSpellId = atoi(parser->tokens[12]);
	result->interruptSpellName = strndup(parser->tokens[13],
			parser->tokenLengths[13]);
	result->interruptSpellSchool = atoi(parser->tokens[14]);

/*
	printf("\t "
			"DETAIL[%s]{%i, '%s', '%s', %i, '%s', %i}\n",
			WOW_EVENT_LIST[(WOW_EVENT_SPELL_INTERRUPT - 1)].name,
			result->spellId, result->spellName,
			WOW_SPELL_SCHOOL[(result->spellSchool - 1)].name,
			result->interruptSpellId, result->interruptSpellName,
			result->interruptSpellSchool);
*/

	return result;
}

static void freeSpellInterrupt(WowEventSpellInterrupt *event)
{
	if(event->spellName != NULL) {
		free(event->spellName);
	}

	if(event->interruptSpellName != NULL) {
		free(event->interruptSpellName);
	}

	free(event);
}

static void buildKeyPortionSpellInterrupt(WowEventSpellInterrupt *event,
		char keyPortion[KEY_PORTION_LENGTH])
{
	unsigned int hashValue = 0;
	char buffer[BUFFER_LENGTH];

	snprintf(buffer, BUFFER_SNPRINTF_LENGTH,
			"%i/%s/%i/%i/%s/%i",
			event->spellId, event->spellName, event->spellSchool,
			event->interruptSpellId, event->interruptSpellName,
			event->interruptSpellSchool);

	hashValue = crc32_calculateHash(buffer, strlen(buffer));

	snprintf(keyPortion, (KEY_PORTION_LENGTH - 1), "%u", hashValue);
}

// SPELL_LEECH event parsing

static void *parseSpellLeech(WowParser *parser)
{
	int spellSchool = 0;

	WowEventSpellLeech *result = NULL;

	if(parser->tokenCount < 14) {
		return NULL;
	}

	if((spellSchool = determineSpellSchool(parser, 11)) < 0) {
		return NULL;
	}

	result = (WowEventSpellLeech *)malloc(sizeof(WowEventSpellLeech));

	result->spellId = atoi(parser->tokens[9]);
	result->spellName = strndup(parser->tokens[10], parser->tokenLengths[10]);
	result->spellSchool = spellSchool;
	result->leechAmount = atoi(parser->tokens[12]);
	result->powerType = strdup(determinePowerType(parser, 13));

	if(parser->tokenCount > 14) {
		result->extraLeechAmount = atoi(parser->tokens[14]);
	}
	else {
		result->extraLeechAmount = 0;
	}

/*
	printf("\t "
			"DETAIL[%s]{%i, '%s', '%s', %i, '%s', %i}\n",
			WOW_EVENT_LIST[(WOW_EVENT_SPELL_LEECH - 1)].name,
			result->spellId, result->spellName,
			WOW_SPELL_SCHOOL[(result->spellSchool - 1)].name,
			result->leechAmount, result->powerType,
			result->extraLeechAmount);
*/

	return result;
}

static void freeSpellLeech(WowEventSpellLeech *event)
{
	if(event->spellName != NULL) {
		free(event->spellName);
	}

	if(event->powerType != NULL) {
		free(event->powerType);
	}

	free(event);
}

static void buildKeyPortionSpellLeech(WowEventSpellLeech *event,
		char keyPortion[KEY_PORTION_LENGTH])
{
	unsigned int hashValue = 0;
	char buffer[BUFFER_LENGTH];

	snprintf(buffer, BUFFER_SNPRINTF_LENGTH,
			"%i/%s/%i/%i/%s/%i",
			event->spellId, event->spellName, event->spellSchool,
			event->leechAmount, event->powerType,
			event->extraLeechAmount);

	hashValue = crc32_calculateHash(buffer, strlen(buffer));

	snprintf(keyPortion, (KEY_PORTION_LENGTH - 1), "%u", hashValue);
}

// SPELL_MISSED event parsing

static void *parseSpellMissed(WowParser *parser)
{
	int spellSchool = 0;

	WowEventSpellMissed *result = NULL;

	if(parser->tokenCount < 13) {
		return NULL;
	}

	if((spellSchool = determineSpellSchool(parser, 11)) < 0) {
		return NULL;
	}

	result = (WowEventSpellMissed *)malloc(sizeof(WowEventSpellMissed));

	result->spellId = atoi(parser->tokens[9]);
	result->spellName = strndup(parser->tokens[10], parser->tokenLengths[10]);
	result->spellSchool = spellSchool;
	result->missType = strndup(parser->tokens[12],
			parser->tokenLengths[12]);

	if(parser->tokenCount > 13) {
		result->missAmount = atoi(parser->tokens[13]);
	}
	else {
		result->missAmount = 0;
	}

/*
	printf("\t "
			"DETAIL[%s]{%i, '%s', '%s', '%s', %i}\n",
			WOW_EVENT_LIST[(WOW_EVENT_SPELL_MISSED - 1)].name,
			result->spellId, result->spellName,
			WOW_SPELL_SCHOOL[(result->spellSchool - 1)].name,
			result->missType, result->missAmount);
*/

	return result;
}

static void freeSpellMissed(WowEventSpellMissed *event)
{
	if(event->spellName != NULL) {
		free(event->spellName);
	}

	if(event->missType != NULL) {
		free(event->missType);
	}

	free(event);
}

static void buildKeyPortionSpellMissed(WowEventSpellMissed *event,
		char keyPortion[KEY_PORTION_LENGTH])
{
	unsigned int hashValue = 0;
	char buffer[BUFFER_LENGTH];

	snprintf(buffer, BUFFER_SNPRINTF_LENGTH,
			"%i/%s/%i/%s/%i",
			event->spellId, event->spellName, event->spellSchool,
			event->missType, event->missAmount);

	hashValue = crc32_calculateHash(buffer, strlen(buffer));

	snprintf(keyPortion, (KEY_PORTION_LENGTH - 1), "%u", hashValue);
}

// SPELL_RESURRECT event parsing

static void *parseSpellResurrect(WowParser *parser)
{
	int spellSchool = 0;

	WowEventSpellResurrect *result = NULL;

	if(parser->tokenCount < 12) {
		return NULL;
	}

	if((spellSchool = determineSpellSchool(parser, 11)) < 0) {
		return NULL;
	}

	result = (WowEventSpellResurrect *)malloc(sizeof(WowEventSpellResurrect));

	result->spellId = atoi(parser->tokens[9]);
	result->spellName = strndup(parser->tokens[10], parser->tokenLengths[10]);
	result->spellSchool = spellSchool;

/*
	printf("\t "
			"DETAIL[%s]{%i, '%s', '%s'}\n",
			WOW_EVENT_LIST[(WOW_EVENT_SPELL_RESURRECT - 1)].name,
			result->spellId, result->spellName,
			WOW_SPELL_SCHOOL[(result->spellSchool - 1)].name);
*/

	return result;
}

static void freeSpellResurrect(WowEventSpellResurrect *event)
{
	if(event->spellName != NULL) {
		free(event->spellName);
	}

	free(event);
}

static void buildKeyPortionSpellResurrect(WowEventSpellResurrect *event,
		char keyPortion[KEY_PORTION_LENGTH])
{
	unsigned int hashValue = 0;
	char buffer[BUFFER_LENGTH];

	snprintf(buffer, BUFFER_SNPRINTF_LENGTH,
			"%i/%s/%i",
			event->spellId, event->spellName, event->spellSchool);

	hashValue = crc32_calculateHash(buffer, strlen(buffer));

	snprintf(keyPortion, (KEY_PORTION_LENGTH - 1), "%u", hashValue);
}

// SPELL_STOLEN event parsing

static void *parseSpellStolen(WowParser *parser)
{
	int spellSchool = 0;

	WowEventSpellStolen *result = NULL;

	if(parser->tokenCount < 16) {
		return NULL;
	}

	if((spellSchool = determineSpellSchool(parser, 11)) < 0) {
		return NULL;
	}

	result = (WowEventSpellStolen *)malloc(sizeof(WowEventSpellStolen));

	result->spellId = atoi(parser->tokens[9]);
	result->spellName = strndup(parser->tokens[10], parser->tokenLengths[10]);
	result->spellSchool = spellSchool;
	result->stolenSpellId = atoi(parser->tokens[12]);
	result->stolenSpellName = strndup(parser->tokens[13],
			parser->tokenLengths[13]);
	result->stolenSpellSchool = atoi(parser->tokens[14]);
	result->auraType = strndup(parser->tokens[15], parser->tokenLengths[15]);

/*
	printf("\t "
			"DETAIL[%s]{%i, '%s', '%s', %i, '%s', %i, '%s'}\n",
			WOW_EVENT_LIST[(WOW_EVENT_SPELL_STOLEN - 1)].name,
			result->spellId, result->spellName,
			WOW_SPELL_SCHOOL[(result->spellSchool - 1)].name,
			result->stolenSpellId, result->stolenSpellName,
			result->stolenSpellSchool, result->auraType);
*/

	return result;
}

static void freeSpellStolen(WowEventSpellStolen *event)
{
	if(event->spellName != NULL) {
		free(event->spellName);
	}

	if(event->stolenSpellName != NULL) {
		free(event->stolenSpellName);
	}

	if(event->auraType != NULL) {
		free(event->auraType);
	}

	free(event);
}

static void buildKeyPortionSpellStolen(WowEventSpellStolen *event,
		char keyPortion[KEY_PORTION_LENGTH])
{
	unsigned int hashValue = 0;
	char buffer[BUFFER_LENGTH];

	snprintf(buffer, BUFFER_SNPRINTF_LENGTH,
			"%i/%s/%i/%i/%s/%i/%s",
			event->spellId, event->spellName, event->spellSchool,
			event->stolenSpellId, event->stolenSpellName,
			event->stolenSpellSchool, event->auraType);

	hashValue = crc32_calculateHash(buffer, strlen(buffer));

	snprintf(keyPortion, (KEY_PORTION_LENGTH - 1), "%u", hashValue);
}

// SPELL_SUMMON event parsing

static void *parseSpellSummon(WowParser *parser)
{
	int spellSchool = 0;

	WowEventSpellSummon *result = NULL;

	if(parser->tokenCount < 12) {
		return NULL;
	}

	if((spellSchool = determineSpellSchool(parser, 11)) < 0) {
		return NULL;
	}

	result = (WowEventSpellSummon *)malloc(sizeof(WowEventSpellSummon));

	result->spellId = atoi(parser->tokens[9]);
	result->spellName = strndup(parser->tokens[10], parser->tokenLengths[10]);
	result->spellSchool = spellSchool;

/*
	printf("\t "
			"DETAIL[%s]{%i, '%s', '%s'}\n",
			WOW_EVENT_LIST[(WOW_EVENT_SPELL_SUMMON - 1)].name,
			result->spellId, result->spellName,
			WOW_SPELL_SCHOOL[(result->spellSchool - 1)].name);
*/

	return result;
}

static void freeSpellSummon(WowEventSpellSummon *event)
{
	if(event->spellName != NULL) {
		free(event->spellName);
	}

	free(event);
}

static void buildKeyPortionSpellSummon(WowEventSpellSummon *event,
		char keyPortion[KEY_PORTION_LENGTH])
{
	unsigned int hashValue = 0;
	char buffer[BUFFER_LENGTH];

	snprintf(buffer, BUFFER_SNPRINTF_LENGTH,
			"%i/%s/%i",
			event->spellId, event->spellName, event->spellSchool);

	hashValue = crc32_calculateHash(buffer, strlen(buffer));

	snprintf(keyPortion, (KEY_PORTION_LENGTH - 1), "%u", hashValue);
}

// SPELL_PERIODIC_DAMAGE event parsing

static void *parseSpellPeriodicDamage(WowParser *parser)
{
	int spellSchool = 0;

	WowEventSpellPeriodicDamage *result = NULL;

	if(parser->tokenCount < 21) {
		return NULL;
	}

	if((spellSchool = determineSpellSchool(parser, 11)) < 0) {
		return NULL;
	}

	result = (WowEventSpellPeriodicDamage *)malloc(
			sizeof(WowEventSpellPeriodicDamage));

	result->spellId = atoi(parser->tokens[9]);
	result->spellName = strndup(parser->tokens[10], parser->tokenLengths[10]);
	result->spellSchool = spellSchool;
	result->damageAmount = atoi(parser->tokens[12]);
	result->overkillAmount = atoi(parser->tokens[13]);
	result->school = spellSchool; // TODO: verify this
	result->resistAmount = atoi(parser->tokens[15]);
	result->blockAmount = atoi(parser->tokens[16]);

	if(!strncmp(parser->tokens[17], "0", 1)) {
		result->absorbed = afalse;
	}
	else {
		result->absorbed = atrue;
	}

	if(!strncmp(parser->tokens[18], "1", 1)) {
		result->wasCritical = atrue;
	}
	else {
		result->wasCritical = afalse;
	}

	if(!strncmp(parser->tokens[19], "1", 1)) {
		result->glancing = atrue;
	}
	else {
		result->glancing = afalse;
	}

	if(!strncmp(parser->tokens[20], "1", 1)) {
		result->crushing = atrue;
	}
	else {
		result->crushing = afalse;
	}

/*
	printf("\t "
			"DETAIL[%s]{%i, '%s', '%s', %i, %i, %i, %i, %i, %i, %i, %i, %i}\n",
			WOW_EVENT_LIST[(WOW_EVENT_SPELL_PERIODIC_DAMAGE - 1)].name,
			result->spellId, result->spellName,
			WOW_SPELL_SCHOOL[(result->spellSchool - 1)].name,
			result->damageAmount, result->overkillAmount, result->school,
			result->resistAmount, result->blockAmount, result->absorbed,
			result->wasCritical, result->glancing, result->crushing);
*/

	return result;
}

static void freeSpellPeriodicDamage(WowEventSpellPeriodicDamage *event)
{
	if(event->spellName != NULL) {
		free(event->spellName);
	}

	free(event);
}

static void buildKeyPortionSpellPeriodicDamage(
		WowEventSpellPeriodicDamage *event,
		char keyPortion[KEY_PORTION_LENGTH])
{
	unsigned int hashValue = 0;
	char buffer[BUFFER_LENGTH];

	snprintf(buffer, BUFFER_SNPRINTF_LENGTH,
			"%i/%s/%i/%i/%i/%i/%i/%i/%i/%i/%i/%i",
			event->spellId, event->spellName, event->spellSchool,
			event->damageAmount, event->overkillAmount, event->school,
			event->resistAmount, event->blockAmount, event->absorbed,
			event->wasCritical, event->glancing, event->crushing);

	hashValue = crc32_calculateHash(buffer, strlen(buffer));

	snprintf(keyPortion, (KEY_PORTION_LENGTH - 1), "%u", hashValue);
}

// SPELL_PERIODIC_ENERGIZE event parsing

static void *parseSpellPeriodicEnergize(WowParser *parser)
{
	int spellSchool = 0;

	WowEventSpellPeriodicEnergize *result = NULL;

	if(parser->tokenCount < 14) {
		return NULL;
	}

	if((spellSchool = determineSpellSchool(parser, 11)) < 0) {
		return NULL;
	}

	result = (WowEventSpellPeriodicEnergize *)malloc(
			sizeof(WowEventSpellPeriodicEnergize));

	result->spellId = atoi(parser->tokens[9]);
	result->spellName = strndup(parser->tokens[10], parser->tokenLengths[10]);
	result->spellSchool = spellSchool;
	result->energizeAmount = atoi(parser->tokens[12]);
	result->powerType = strdup(determinePowerType(parser, 13));

	if(parser->tokenCount > 14) {
		result->extraEnergizeAmount = atoi(parser->tokens[14]);
	}
	else {
		result->extraEnergizeAmount = 0;
	}

/*
	printf("\t "
			"DETAIL[%s]{%i, '%s', '%s', %i, '%s', %i}\n",
			WOW_EVENT_LIST[(WOW_EVENT_SPELL_PERIODIC_ENERGIZE - 1)].name,
			result->spellId, result->spellName,
			WOW_SPELL_SCHOOL[(result->spellSchool - 1)].name,
			result->energizeAmount, result->powerType,
			result->extraEnergizeAmount);
*/

	return result;
}

static void freeSpellPeriodicEnergize(WowEventSpellPeriodicEnergize *event)
{
	if(event->spellName != NULL) {
		free(event->spellName);
	}

	if(event->powerType != NULL) {
		free(event->powerType);
	}

	free(event);
}

static void buildKeyPortionSpellPeriodicEnergize(
		WowEventSpellPeriodicEnergize *event,
		char keyPortion[KEY_PORTION_LENGTH])
{
	unsigned int hashValue = 0;
	char buffer[BUFFER_LENGTH];

	snprintf(buffer, BUFFER_SNPRINTF_LENGTH,
			"%i/%s/%i/%i/%s/%i",
			event->spellId, event->spellName, event->spellSchool,
			event->energizeAmount, event->powerType,
			event->extraEnergizeAmount);

	hashValue = crc32_calculateHash(buffer, strlen(buffer));

	snprintf(keyPortion, (KEY_PORTION_LENGTH - 1), "%u", hashValue);
}

// SPELL_PERIODIC_HEAL event parsing

static void *parseSpellPeriodicHealed(WowParser *parser)
{
	int spellSchool = 0;

	WowEventSpellPeriodicHealed *result = NULL;

	if(parser->tokenCount < 15) {
		return NULL;
	}

	if((spellSchool = determineSpellSchool(parser, 11)) < 0) {
		return NULL;
	}

	result = (WowEventSpellPeriodicHealed *)malloc(
			sizeof(WowEventSpellPeriodicHealed));

	result->spellId = atoi(parser->tokens[9]);
	result->spellName = strndup(parser->tokens[10], parser->tokenLengths[10]);
	result->spellSchool = spellSchool;
	result->healAmount = atoi(parser->tokens[12]);
	result->overHealingAmount = atoi(parser->tokens[13]);

	if(!strncmp(parser->tokens[14], "1", 1)) {
		result->wasCritical = atrue;
	}
	else {
		result->wasCritical = afalse;
	}
/*
	printf("\t DETAIL[%s]{%i, '%s', '%s', %i, %i, %i}\n",
			WOW_EVENT_LIST[(WOW_EVENT_SPELL_PERIODIC_HEAL - 1)].name,
			result->spellId, result->spellName,
			WOW_SPELL_SCHOOL[(result->spellSchool - 1)].name,
			result->healAmount, result->overHealingAmount,
			result->wasCritical);
*/
	return result;
}

static void freeSpellPeriodicHealed(WowEventSpellPeriodicHealed *event)
{
	if(event->spellName != NULL) {
		free(event->spellName);
	}

	free(event);
}

static void buildKeyPortionSpellPeriodicHealed(
		WowEventSpellPeriodicHealed *event,
		char keyPortion[KEY_PORTION_LENGTH])
{
	unsigned int hashValue = 0;
	char buffer[BUFFER_LENGTH];

	snprintf(buffer, BUFFER_SNPRINTF_LENGTH,
			"%i/%s/%i/%i/%i/%i",
			event->spellId, event->spellName, event->spellSchool,
			event->healAmount, event->overHealingAmount, event->wasCritical);

	hashValue = crc32_calculateHash(buffer, strlen(buffer));

	snprintf(keyPortion, (KEY_PORTION_LENGTH - 1), "%u", hashValue);
}

// SPELL_PERIODIC_MISSED event parsing

static void *parseSpellPeriodicMissed(WowParser *parser)
{
	int spellSchool = 0;

	WowEventSpellPeriodicMissed *result = NULL;

	if(parser->tokenCount < 12) {
		return NULL;
	}

	if((spellSchool = determineSpellSchool(parser, 11)) < 0) {
		return NULL;
	}

	result = (WowEventSpellPeriodicMissed *)malloc(
			sizeof(WowEventSpellPeriodicMissed));

	result->spellId = atoi(parser->tokens[9]);
	result->spellName = strndup(parser->tokens[10], parser->tokenLengths[10]);
	result->spellSchool = spellSchool;
	result->missType = strndup(parser->tokens[12],
			parser->tokenLengths[12]);

	if(parser->tokenCount > 13) {
		result->missAmount = atoi(parser->tokens[13]);
	}
	else {
		result->missAmount = 0;
	}

/*
	printf("\t "
			"DETAIL[%s]{%i, '%s', '%s', '%s', %i}\n",
			WOW_EVENT_LIST[(WOW_EVENT_SPELL_PERIODIC_MISSED - 1)].name,
			result->spellId, result->spellName,
			WOW_SPELL_SCHOOL[(result->spellSchool - 1)].name,
			result->missType, result->missAmount);
*/

	return result;
}

static void freeSpellPeriodicMissed(WowEventSpellPeriodicMissed *event)
{
	if(event->spellName != NULL) {
		free(event->spellName);
	}

	if(event->missType != NULL) {
		free(event->missType);
	}

	free(event);
}

static void buildKeyPortionSpellPeriodicMissed(
		WowEventSpellPeriodicMissed *event,
		char keyPortion[KEY_PORTION_LENGTH])
{
	unsigned int hashValue = 0;
	char buffer[BUFFER_LENGTH];

	snprintf(buffer, BUFFER_SNPRINTF_LENGTH,
			"%i/%s/%i/%s/%i",
			event->spellId, event->spellName, event->spellSchool,
			event->missType, event->missAmount);

	hashValue = crc32_calculateHash(buffer, strlen(buffer));

	snprintf(keyPortion, (KEY_PORTION_LENGTH - 1), "%u", hashValue);
}

// SWING_DAMAGE event parsing

static void *parseSwingDamage(WowParser *parser)
{
	WowEventSwingDamage *result = NULL;

	if(parser->tokenCount < 18) {
		return NULL;
	}

	result = (WowEventSwingDamage *)malloc(sizeof(WowEventSwingDamage));

	result->damageAmount = atoi(parser->tokens[9]);
	result->overkillAmount = atoi(parser->tokens[10]);
	result->school = atoi(parser->tokens[11]);
	result->resistAmount = atoi(parser->tokens[12]);
	result->blockAmount = atoi(parser->tokens[13]);

	if(!strncmp(parser->tokens[14], "0", 1)) {
		result->absorbed = afalse;
	}
	else {
		result->absorbed = atrue;
	}

	if(!strncmp(parser->tokens[15], "1", 1)) {
		result->wasCritical = atrue;
	}
	else {
		result->wasCritical = afalse;
	}

	if(!strncmp(parser->tokens[16], "1", 1)) {
		result->glancing = atrue;
	}
	else {
		result->glancing = afalse;
	}

	if(!strncmp(parser->tokens[17], "1", 1)) {
		result->crushing = atrue;
	}
	else {
		result->crushing = afalse;
	}

/*
	printf("\t "
			"DETAIL[%s]{%i, %i, %i, %i, %i, %i, %i, %i, %i}\n",
			WOW_EVENT_LIST[(WOW_EVENT_SWING_DAMAGE - 1)].name,
			result->damageAmount, result->overkillAmount, result->school,
			result->resistAmount, result->blockAmount, result->absorbed,
			result->wasCritical, result->glancing, result->crushing);
*/

	return result;
}

static void freeSwingDamage(WowEventSwingDamage *event)
{
	free(event);
}

static void buildKeyPortionSwingDamage(WowEventSwingDamage *event,
		char keyPortion[KEY_PORTION_LENGTH])
{
	unsigned int hashValue = 0;
	char buffer[BUFFER_LENGTH];

	snprintf(buffer, BUFFER_SNPRINTF_LENGTH,
			"%i/%i/%i/%i/%i/%i/%i/%i/%i",
			event->damageAmount, event->overkillAmount, event->school,
			event->resistAmount, event->blockAmount, event->absorbed,
			event->wasCritical, event->glancing, event->crushing);

	hashValue = crc32_calculateHash(buffer, strlen(buffer));

	snprintf(keyPortion, (KEY_PORTION_LENGTH - 1), "%u", hashValue);
}

// SWING_MISSED event parsing

static void *parseSwingMissed(WowParser *parser)
{
	WowEventSwingMissed *result = NULL;

/*
	{
		int ii = 0;
		for(ii = 0; ii < parser->tokenCount; ii++) {
			printf("parser[%4i]->'%s'\n", ii, parser->tokens[ii]);
		}
		exit(1);
	}
*/

	if(parser->tokenCount < 10) {
		return NULL;
	}

	result = (WowEventSwingMissed *)malloc(sizeof(WowEventSwingMissed));

	result->missType = strndup(parser->tokens[9], parser->tokenLengths[9]);

	if(parser->tokenCount > 10) {
		result->missAmount = atoi(parser->tokens[10]);
	}
	else {
		result->missAmount = 0;
	}

/*
	printf("\t "
			"DETAIL[%s]{'%s', %i}\n",
			WOW_EVENT_LIST[(WOW_EVENT_SWING_MISSED - 1)].name,
			result->missType, result->missAmount);
*/

	return result;
}

static void freeSwingMissed(WowEventSwingMissed *event)
{
	if(event->missType != NULL) {
		free(event->missType);
	}

	free(event);
}

static void buildKeyPortionSwingMissed(WowEventSwingMissed *event,
		char keyPortion[KEY_PORTION_LENGTH])
{
	unsigned int hashValue = 0;
	char buffer[BUFFER_LENGTH];

	snprintf(buffer, BUFFER_SNPRINTF_LENGTH,
			"%s/%i",
			event->missType, event->missAmount);

	hashValue = crc32_calculateHash(buffer, strlen(buffer));

	snprintf(keyPortion, (KEY_PORTION_LENGTH - 1), "%u", hashValue);
}

// general event parsing

static void cleanUlduarUid(char *uid, int uidLength)
{
	if(uidLength < 17) {
		return;
	}

	if(strncmp(uid, "0x0180", 6)) {
		return;
	}

	uid[3] = '0';
	uid[4] = '0';
}

static unsigned int parseSourceFlags(char *sourceFlags, int sourceFlagsLength)
{
	unsigned int sourceFlagId = 0;

	sscanf(sourceFlags, "%x", &sourceFlagId);

	return sourceFlagId;
}

static unsigned int parseTargetFlags(char *targetFlags, int targetFlagsLength)
{
	unsigned int targetFlagId = 0;

	sscanf(targetFlags, "%x", &targetFlagId);

	return targetFlagId;
}

static unsigned long long int parseUnitType(char *unitType, int unitTypeLength)
{
	unsigned long long int unitTypeId = 0;

	sscanf(unitType, "%llx", &unitTypeId);

	return unitTypeId;
}

static int parseEvent(WowEvent *event, WowParser *parser)
{
	aboolean parsedEvent = afalse;
	int ii = 0;
	void *eventParseResult = NULL;

	// parse the event

	switch(event->eventId) {
		case WOW_EVENT_DAMAGE_SHIELD:
			parsedEvent = atrue;
			eventParseResult = parseDamageShield(parser);
			break;
		case WOW_EVENT_DAMAGE_SHIELD_MISSED:
			parsedEvent = atrue;
			eventParseResult = parseDamageShieldMissed(parser);
			break;
		case WOW_EVENT_DAMAGE_SPLIT:
			parsedEvent = atrue;
			eventParseResult = parseDamageSplit(parser);
			break;
		case WOW_EVENT_ENCHANT_APPLIED:
			parsedEvent = atrue;
			eventParseResult = parseEnchantApplied(parser);
			break;
		case WOW_EVENT_ENCHANT_REMOVED:
			parsedEvent = atrue;
			eventParseResult = parseEnchantRemoved(parser);
			break;
		case WOW_EVENT_PARTY_KILL:
			parsedEvent = atrue;
			eventParseResult = parsePartyKill(parser);
			break;
		case WOW_EVENT_UNIT_DIED:
			parsedEvent = atrue;
			eventParseResult = parseUnitDied(parser);
			break;
		case WOW_EVENT_UNIT_DESTROYED:
			parsedEvent = atrue;
			eventParseResult = parseUnitDestroyed(parser);
			break;
		case WOW_EVENT_ENVIRONMENTAL_DAMAGE:
			parsedEvent = atrue;
			eventParseResult = parseEnvironmentalDamage(parser);
			break;
		case WOW_EVENT_RANGE_DAMAGE:
			parsedEvent = atrue;
			eventParseResult = parseRangeDamage(parser);
			break;
		case WOW_EVENT_RANGE_MISSED:
			parsedEvent = atrue;
			eventParseResult = parseRangeMissed(parser);
			break;
		case WOW_EVENT_SPELL_AURA_APPLIED:
			parsedEvent = atrue;
			eventParseResult = parseSpellAuraApplied(parser);
			break;
		case WOW_EVENT_SPELL_AURA_APPLIED_DOSE:
			parsedEvent = atrue;
			eventParseResult = parseSpellAuraAppliedDose(parser);
			break;
		case WOW_EVENT_SPELL_AURA_BROKEN:
			parsedEvent = atrue;
			eventParseResult = parseSpellAuraBroken(parser);
			break;
		case WOW_EVENT_SPELL_AURA_BROKEN_SPELL:
			parsedEvent = atrue;
			eventParseResult = parseSpellAuraBrokenSpell(parser);
			break;
		case WOW_EVENT_SPELL_AURA_REFRESH:
			parsedEvent = atrue;
			eventParseResult = parseSpellAuraRefresh(parser);
			break;
		case WOW_EVENT_SPELL_AURA_REMOVED:
			parsedEvent = atrue;
			eventParseResult = parseSpellAuraRemoved(parser);
			break;
		case WOW_EVENT_SPELL_AURA_REMOVED_DOSE:
			parsedEvent = atrue;
			eventParseResult = parseSpellAuraRemovedDose(parser);
			break;
		case WOW_EVENT_SPELL_CAST_FAILED:
			parsedEvent = atrue;
			eventParseResult = parseSpellCastFailed(parser);
			break;
		case WOW_EVENT_SPELL_CAST_START:
			parsedEvent = atrue;
			eventParseResult = parseSpellCastStart(parser);
			break;
		case WOW_EVENT_SPELL_CAST_SUCCESS:
			parsedEvent = atrue;
			eventParseResult = parseSpellCastSuccess(parser);
			break;
		case WOW_EVENT_SPELL_CREATE:
			parsedEvent = atrue;
			eventParseResult = parseSpellCreate(parser);
			break;
		case WOW_EVENT_SPELL_DAMAGE:
			parsedEvent = atrue;
			eventParseResult = parseSpellDamage(parser);
			break;
		case WOW_EVENT_SPELL_DISPEL:
			parsedEvent = atrue;
			eventParseResult = parseSpellDispel(parser);
			break;
		case WOW_EVENT_SPELL_DISPEL_FAILED:
			parsedEvent = atrue;
			eventParseResult = parseSpellDispelFailed(parser);
			break;
		case WOW_EVENT_SPELL_ENERGIZE:
			parsedEvent = atrue;
			eventParseResult = parseSpellEnergize(parser);
			break;
		case WOW_EVENT_SPELL_EXTRA_ATTACKS:
			parsedEvent = atrue;
			eventParseResult = parseSpellExtraAttacks(parser);
			break;
		case WOW_EVENT_SPELL_HEAL:
			parsedEvent = atrue;
			eventParseResult = parseSpellHealed(parser);
			break;
		case WOW_EVENT_SPELL_INSTAKILL:
			parsedEvent = atrue;
			eventParseResult = parseSpellInstakill(parser);
			break;
		case WOW_EVENT_SPELL_INTERRUPT:
			parsedEvent = atrue;
			eventParseResult = parseSpellInterrupt(parser);
			break;
		case WOW_EVENT_SPELL_LEECH:
			parsedEvent = atrue;
			eventParseResult = parseSpellLeech(parser);
			break;
		case WOW_EVENT_SPELL_MISSED:
			parsedEvent = atrue;
			eventParseResult = parseSpellMissed(parser);
			break;
		case WOW_EVENT_SPELL_RESURRECT:
			parsedEvent = atrue;
			eventParseResult = parseSpellResurrect(parser);
			break;
		case WOW_EVENT_SPELL_STOLEN:
			parsedEvent = atrue;
			eventParseResult = parseSpellStolen(parser);
			break;
		case WOW_EVENT_SPELL_SUMMON:
			parsedEvent = atrue;
			eventParseResult = parseSpellSummon(parser);
			break;
		case WOW_EVENT_SPELL_PERIODIC_DAMAGE:
			parsedEvent = atrue;
			eventParseResult = parseSpellPeriodicDamage(parser);
			break;
		case WOW_EVENT_SPELL_PERIODIC_ENERGIZE:
			parsedEvent = atrue;
			eventParseResult = parseSpellPeriodicEnergize(parser);
			break;
		case WOW_EVENT_SPELL_PERIODIC_HEAL:
			parsedEvent = atrue;
			eventParseResult = parseSpellPeriodicHealed(parser);
			break;
		case WOW_EVENT_SPELL_PERIODIC_MISSED:
			parsedEvent = atrue;
			eventParseResult = parseSpellPeriodicMissed(parser);
			break;
		case WOW_EVENT_SWING_DAMAGE:
			parsedEvent = atrue;
			eventParseResult = parseSwingDamage(parser);
			break;
		case WOW_EVENT_SWING_MISSED:
			parsedEvent = atrue;
			eventParseResult = parseSwingMissed(parser);
			break;
	}

	if(eventParseResult == NULL) {
		if(parsedEvent) {
			printf("[%s():%i] error - failed to parse event { ", __FUNCTION__,
					__LINE__);
			for(ii = 0; ii < parser->tokenCount; ii++) {
				printf("'%s'", parser->tokens[ii]);
				if((ii + 1) < parser->tokenCount) {
					printf(", ");
				}
			}
			printf(" } %i tokens\n", parser->tokenCount);
		}
		return WOWSTATS_EVENT_INVALID_EVENT;
	}

	event->eventValue = eventParseResult;

	// parse the source information

	cleanUlduarUid(parser->tokens[SOURCE_UID_ARRAY_ELEMENT],
			parser->tokenLengths[SOURCE_UID_ARRAY_ELEMENT]);

	event->sourceUid = strndup(parser->tokens[SOURCE_UID_ARRAY_ELEMENT],
			parser->tokenLengths[SOURCE_UID_ARRAY_ELEMENT]);
	event->sourceName = strndup(parser->tokens[SOURCE_NAME_ARRAY_ELEMENT],
			parser->tokenLengths[SOURCE_NAME_ARRAY_ELEMENT]);
	event->sourceFlags = parseSourceFlags(
			parser->tokens[SOURCE_FLAGS_ARRAY_ELEMENT],
			parser->tokenLengths[SOURCE_FLAGS_ARRAY_ELEMENT]);
	event->sourceUnitType = parseUnitType(
			parser->tokens[SOURCE_UID_ARRAY_ELEMENT],
			parser->tokenLengths[SOURCE_UID_ARRAY_ELEMENT]);

	// parse the target information

	cleanUlduarUid(parser->tokens[TARGET_UID_ARRAY_ELEMENT],
			parser->tokenLengths[TARGET_UID_ARRAY_ELEMENT]);

	event->targetUid = strndup(parser->tokens[TARGET_UID_ARRAY_ELEMENT],
			parser->tokenLengths[TARGET_UID_ARRAY_ELEMENT]);
	event->targetName = strndup(parser->tokens[TARGET_NAME_ARRAY_ELEMENT],
			parser->tokenLengths[TARGET_NAME_ARRAY_ELEMENT]);
	event->targetFlags = parseTargetFlags(
			parser->tokens[TARGET_FLAGS_ARRAY_ELEMENT],
			parser->tokenLengths[TARGET_FLAGS_ARRAY_ELEMENT]);
	event->targetUnitType = parseUnitType(
			parser->tokens[TARGET_UID_ARRAY_ELEMENT],
			parser->tokenLengths[TARGET_UID_ARRAY_ELEMENT]);

/*
	printf("EVENT[%s]{'%s', '%s', %i, %i, '%s', '%s', %i, %i, "
			"[%02i/%02i/%02i at %02i:%02i:%02i.%04i] has 0x%lx\n",
			WOW_EVENT_LIST[(event->eventId - 1)].name,
			event->sourceUid, event->sourceName, event->sourceFlags,
			event->sourceUnitType, event->targetUid, event->targetName,
			event->targetFlags, event->targetUnitType,
			event->timestamp.month, event->timestamp.day,
			event->timestamp.year, event->timestamp.hour,
			event->timestamp.minute, event->timestamp.second,
			event->timestamp.millis, (unsigned long int)event->eventValue);
*/

	return 0;
}


// define wowstats event public functions

int wowstats_eventInit(WowEvent *event, int year, WowParser *parser)
{
	int rc = 0;

	if((event == NULL) || ((year < 2000) && (year > 2020)) ||
			(parser == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	memset(event, 0, sizeof(WowEvent));

	event->eventId = 0;
	event->sourceUid = NULL;
	event->sourceName = NULL;
	event->sourceFlags = 0;
	event->sourceUnitType = 0;
	event->targetUid = NULL;
	event->targetName = NULL;
	event->targetFlags = 0;
	event->targetUnitType = 0;
	event->eventValue = NULL;

	if(!isValidEvent(parser)) {
		return WOWSTATS_EVENT_INVALID_EVENT;
	}

	if((event->eventId = determineEventId(parser)) ==
			WOWSTATS_EVENT_UNKNOWN_EVENT) {
		return WOWSTATS_EVENT_UNKNOWN_EVENT;
	}

	if((rc = parseEvent(event, parser)) != 0) {
		return rc;
	}

	if(wowstats_timestampInit(&event->timestamp, year) < 0) {
		return -1;
	}

	if(wowstats_timestampParseDate(&event->timestamp, parser->tokens[0],
				parser->tokenLengths[0]) < 0) {
		return -1;
	}

	if(wowstats_timestampParseTime(&event->timestamp, parser->tokens[1],
				parser->tokenLengths[1]) < 0) {
		return -1;
	}

	return 0;
}

int wowstats_eventFree(WowEvent *event)
{
	if(event == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(event->sourceUid != NULL) {
		free(event->sourceUid);
	}

	if(event->sourceName != NULL) {
		free(event->sourceName);
	}

	if(event->targetUid != NULL) {
		free(event->targetUid);
	}

	if(event->targetName != NULL) {
		free(event->targetName);
	}

	if(event->eventValue != NULL) {
		switch(event->eventId) {
			case WOW_EVENT_DAMAGE_SHIELD:
				freeDamageShield(event->eventValue);
				break;
			case WOW_EVENT_DAMAGE_SHIELD_MISSED:
				freeDamageShieldMissed(event->eventValue);
				break;
			case WOW_EVENT_DAMAGE_SPLIT:
				freeDamageSplit(event->eventValue);
				break;
			case WOW_EVENT_ENCHANT_APPLIED:
				freeEnchantApplied(event->eventValue);
				break;
			case WOW_EVENT_ENCHANT_REMOVED:
				freeEnchantRemoved(event->eventValue);
				break;
			case WOW_EVENT_PARTY_KILL:
				freePartyKill(event->eventValue);
				break;
			case WOW_EVENT_UNIT_DIED:
				freeUnitDied(event->eventValue);
				break;
			case WOW_EVENT_UNIT_DESTROYED:
				freeUnitDestroyed(event->eventValue);
				break;
			case WOW_EVENT_ENVIRONMENTAL_DAMAGE:
				freeEnvironmentalDamage(event->eventValue);
				break;
			case WOW_EVENT_RANGE_DAMAGE:
				freeRangeDamage(event->eventValue);
				break;
			case WOW_EVENT_RANGE_MISSED:
				freeRangeMissed(event->eventValue);
				break;
			case WOW_EVENT_SPELL_AURA_APPLIED:
				freeSpellAuraApplied(event->eventValue);
				break;
			case WOW_EVENT_SPELL_AURA_APPLIED_DOSE:
				freeSpellAuraAppliedDose(event->eventValue);
				break;
			case WOW_EVENT_SPELL_AURA_BROKEN:
				freeSpellAuraBroken(event->eventValue);
				break;
			case WOW_EVENT_SPELL_AURA_BROKEN_SPELL:
				freeSpellAuraBrokenSpell(event->eventValue);
				break;
			case WOW_EVENT_SPELL_AURA_REFRESH:
				freeSpellAuraRefresh(event->eventValue);
				break;
			case WOW_EVENT_SPELL_AURA_REMOVED:
				freeSpellAuraRemoved(event->eventValue);
				break;
			case WOW_EVENT_SPELL_AURA_REMOVED_DOSE:
				freeSpellAuraRemovedDose(event->eventValue);
				break;
			case WOW_EVENT_SPELL_CAST_FAILED:
				freeSpellCastFailed(event->eventValue);
				break;
			case WOW_EVENT_SPELL_CAST_START:
				freeSpellCastStart(event->eventValue);
				break;
			case WOW_EVENT_SPELL_CAST_SUCCESS:
				freeSpellCastSuccess(event->eventValue);
				break;
			case WOW_EVENT_SPELL_CREATE:
				freeSpellCreate(event->eventValue);
				break;
			case WOW_EVENT_SPELL_DAMAGE:
				freeSpellDamage(event->eventValue);
				break;
			case WOW_EVENT_SPELL_DISPEL:
				freeSpellDispel(event->eventValue);
				break;
			case WOW_EVENT_SPELL_DISPEL_FAILED:
				freeSpellDispelFailed(event->eventValue);
				break;
			case WOW_EVENT_SPELL_ENERGIZE:
				freeSpellEnergize(event->eventValue);
				break;
			case WOW_EVENT_SPELL_EXTRA_ATTACKS:
				freeSpellExtraAttacks(event->eventValue);
				break;
			case WOW_EVENT_SPELL_HEAL:
				freeSpellHealed(event->eventValue);
				break;
			case WOW_EVENT_SPELL_INSTAKILL:
				freeSpellInstakill(event->eventValue);
				break;
			case WOW_EVENT_SPELL_INTERRUPT:
				freeSpellInterrupt(event->eventValue);
				break;
			case WOW_EVENT_SPELL_LEECH:
				freeSpellLeech(event->eventValue);
				break;
			case WOW_EVENT_SPELL_MISSED:
				freeSpellMissed(event->eventValue);
				break;
			case WOW_EVENT_SPELL_RESURRECT:
				freeSpellResurrect(event->eventValue);
				break;
			case WOW_EVENT_SPELL_STOLEN:
				freeSpellStolen(event->eventValue);
				break;
			case WOW_EVENT_SPELL_SUMMON:
				freeSpellSummon(event->eventValue);
				break;
			case WOW_EVENT_SPELL_PERIODIC_DAMAGE:
				freeSpellPeriodicDamage(event->eventValue);
				break;
			case WOW_EVENT_SPELL_PERIODIC_ENERGIZE:
				freeSpellPeriodicEnergize(event->eventValue);
				break;
			case WOW_EVENT_SPELL_PERIODIC_HEAL:
				freeSpellPeriodicHealed(event->eventValue);
				break;
			case WOW_EVENT_SPELL_PERIODIC_MISSED:
				freeSpellPeriodicMissed(event->eventValue);
				break;
			case WOW_EVENT_SWING_DAMAGE:
				freeSwingDamage(event->eventValue);
				break;
			case WOW_EVENT_SWING_MISSED:
				freeSwingMissed(event->eventValue);
				break;
		}
	}

	return 0;
}

int wowstats_eventBuildKey(WowEvent *event, char **key, int *keyLength)
{
	char keyPortion[KEY_PORTION_LENGTH];
	char buffer[BUFFER_LENGTH];

	if((event == NULL) || (key == NULL) || (keyLength == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	switch(event->eventId) {
		case WOW_EVENT_DAMAGE_SHIELD:
			buildKeyPortionDamageShield(event->eventValue, keyPortion);
			break;
		case WOW_EVENT_DAMAGE_SHIELD_MISSED:
			buildKeyPortionDamageShieldMissed(event->eventValue, keyPortion);
			break;
		case WOW_EVENT_DAMAGE_SPLIT:
			buildKeyPortionDamageSplit(event->eventValue, keyPortion);
			break;
		case WOW_EVENT_ENCHANT_APPLIED:
			buildKeyPortionEnchantApplied(event->eventValue, keyPortion);
			break;
		case WOW_EVENT_ENCHANT_REMOVED:
			buildKeyPortionEnchantRemoved(event->eventValue, keyPortion);
			break;
		case WOW_EVENT_PARTY_KILL:
			buildKeyPortionPartyKill(event->eventValue, keyPortion);
			break;
		case WOW_EVENT_UNIT_DIED:
			buildKeyPortionUnitDied(event->eventValue, keyPortion);
			break;
		case WOW_EVENT_UNIT_DESTROYED:
			buildKeyPortionUnitDestroyed(event->eventValue, keyPortion);
			break;
		case WOW_EVENT_ENVIRONMENTAL_DAMAGE:
			buildKeyPortionEnvironmentalDamage(event->eventValue, keyPortion);
			break;
		case WOW_EVENT_RANGE_DAMAGE:
			buildKeyPortionRangeDamage(event->eventValue, keyPortion);
			break;
		case WOW_EVENT_RANGE_MISSED:
			buildKeyPortionRangeMissed(event->eventValue, keyPortion);
			break;
		case WOW_EVENT_SPELL_AURA_APPLIED:
			buildKeyPortionSpellAuraApplied(event->eventValue, keyPortion);
			break;
		case WOW_EVENT_SPELL_AURA_APPLIED_DOSE:
			buildKeyPortionSpellAuraAppliedDose(event->eventValue, keyPortion);
			break;
		case WOW_EVENT_SPELL_AURA_BROKEN:
			buildKeyPortionSpellAuraBroken(event->eventValue, keyPortion);
			break;
		case WOW_EVENT_SPELL_AURA_BROKEN_SPELL:
			buildKeyPortionSpellAuraBrokenSpell(event->eventValue, keyPortion);
			break;
		case WOW_EVENT_SPELL_AURA_REFRESH:
			buildKeyPortionSpellAuraRefresh(event->eventValue, keyPortion);
			break;
		case WOW_EVENT_SPELL_AURA_REMOVED:
			buildKeyPortionSpellAuraRemoved(event->eventValue, keyPortion);
			break;
		case WOW_EVENT_SPELL_AURA_REMOVED_DOSE:
			buildKeyPortionSpellAuraRemovedDose(event->eventValue, keyPortion);
			break;
		case WOW_EVENT_SPELL_CAST_FAILED:
			buildKeyPortionSpellCastFailed(event->eventValue, keyPortion);
			break;
		case WOW_EVENT_SPELL_CAST_START:
			buildKeyPortionSpellCastStart(event->eventValue, keyPortion);
			break;
		case WOW_EVENT_SPELL_CAST_SUCCESS:
			buildKeyPortionSpellCastSuccess(event->eventValue, keyPortion);
			break;
		case WOW_EVENT_SPELL_CREATE:
			buildKeyPortionSpellCreate(event->eventValue, keyPortion);
			break;
		case WOW_EVENT_SPELL_DAMAGE:
			buildKeyPortionSpellDamage(event->eventValue, keyPortion);
			break;
		case WOW_EVENT_SPELL_DISPEL:
			buildKeyPortionSpellDispel(event->eventValue, keyPortion);
			break;
		case WOW_EVENT_SPELL_DISPEL_FAILED:
			buildKeyPortionSpellDispelFailed(event->eventValue, keyPortion);
			break;
		case WOW_EVENT_SPELL_ENERGIZE:
			buildKeyPortionSpellEnergize(event->eventValue, keyPortion);
			break;
		case WOW_EVENT_SPELL_EXTRA_ATTACKS:
			buildKeyPortionSpellExtraAttacks(event->eventValue, keyPortion);
			break;
		case WOW_EVENT_SPELL_HEAL:
			buildKeyPortionSpellHealed(event->eventValue, keyPortion);
			break;
		case WOW_EVENT_SPELL_INSTAKILL:
			buildKeyPortionSpellInstakill(event->eventValue, keyPortion);
			break;
		case WOW_EVENT_SPELL_INTERRUPT:
			buildKeyPortionSpellInterrupt(event->eventValue, keyPortion);
			break;
		case WOW_EVENT_SPELL_LEECH:
			buildKeyPortionSpellLeech(event->eventValue, keyPortion);
			break;
		case WOW_EVENT_SPELL_MISSED:
			buildKeyPortionSpellMissed(event->eventValue, keyPortion);
			break;
		case WOW_EVENT_SPELL_RESURRECT:
			buildKeyPortionSpellResurrect(event->eventValue, keyPortion);
			break;
		case WOW_EVENT_SPELL_STOLEN:
			buildKeyPortionSpellStolen(event->eventValue, keyPortion);
			break;
		case WOW_EVENT_SPELL_SUMMON:
			buildKeyPortionSpellSummon(event->eventValue, keyPortion);
			break;
		case WOW_EVENT_SPELL_PERIODIC_DAMAGE:
			buildKeyPortionSpellPeriodicDamage(event->eventValue, keyPortion);
			break;
		case WOW_EVENT_SPELL_PERIODIC_ENERGIZE:
			buildKeyPortionSpellPeriodicEnergize(event->eventValue, keyPortion);
			break;
		case WOW_EVENT_SPELL_PERIODIC_HEAL:
			buildKeyPortionSpellPeriodicHealed(event->eventValue, keyPortion);
			break;
		case WOW_EVENT_SPELL_PERIODIC_MISSED:
			buildKeyPortionSpellPeriodicMissed(event->eventValue, keyPortion);
			break;
		case WOW_EVENT_SWING_DAMAGE:
			buildKeyPortionSwingDamage(event->eventValue, keyPortion);
			break;
		case WOW_EVENT_SWING_MISSED:
			buildKeyPortionSwingMissed(event->eventValue, keyPortion);
			break;
		default:
			strcpy(keyPortion, "(unknown)");
	}

	snprintf(buffer, BUFFER_SNPRINTF_LENGTH,
			"/%04i/%02i/%02i/%02i/%02i/%02i/%04i/%02i/%s/%s/%s/%s/%s",
			event->timestamp.year, event->timestamp.month,
			event->timestamp.day, event->timestamp.hour,
			event->timestamp.minute, event->timestamp.second,
			event->timestamp.millis, event->eventId, event->sourceUid,
			event->sourceName, event->targetUid, event->targetName,
			keyPortion);

	*keyLength = strlen(buffer);
	*key = strndup(buffer, *keyLength);

	return 0;
}

