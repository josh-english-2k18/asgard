/*
 * wsdal.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard-WoW statistical analysis data access layer (DAL), header file.
 *
 * Written by Josh English.
 */

#if !defined(_WOWSTATS_SYSTEM_DAL_H)

#define _WOWSTATS_SYSTEM_DAL_H

#if !defined(_WOWSTATS_SYSTEM_H) && !defined(_WOWSTATS_SYSTEM_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _WOWSTATS_SYSTEM_H || _WOWSTATS_SYSTEM_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

// declare wowstats data access layer public functions

// user functions

int wowstats_wsdalGetUserId(Pgdb *pgdb, char *emailAddress, int *userId);

// log history functions

int wowstats_wsdalGetLastLogHistoryId(Pgdb *pgdb, int *logHistoryId);

int wowstats_wsdalAddLogHistory(Pgdb *pgdb, int userId, int realmId,
		char *logType, char *logFilename, char *logDescription,
		int playSeconds, char *logTimestamp);

int wowstats_wsdalSetLogHistoryPlaySeconds(Pgdb *pgdb, int logHistoryId,
		int playSeconds);

int wowstats_wsdalAddContributingLog(Pgdb *pgdb, int logHistoryId, int userId,
		int realmId, char *logType, char *logFilename, char *logDescription,
		char *logTimestamp);

int wowstats_wsdalGetLastMobLogId(Pgdb *pgdb, int *mobLogId);

int wowstats_wsdalAddMobLog(Pgdb *pgdb, int logHistoryId, int userId,
		int realmId, char *logType, char *logFilename, char *logDescription,
		int playSeconds, char *logTimestamp);

// realm functions

int wowstats_wsdalGetRealmId(Pgdb *pgdb, char *regionCode, char *realmName,
		int *realmId);

// entity functions

int wowstats_wsdalGetEntityId(Pgdb *pgdb, int realmId, char *uid,
		int *entityId);

int wowstats_wsdalAddEntity(Pgdb *pgdb, int realmId, char *className,
		char *name, char *uid, int totalPlayedSeconds);

int wowstats_wsdalUpdateEntity(Pgdb *pgdb, int entityId, char *className,
		int totalPlayedSeconds);

int wowstats_wsdalAddEntityLog(Pgdb *pgdb, int entityId, int logHistoryId,
		int totalPlayedSeconds);

// summary functions

int wowstats_wsdalGetSummaryTypeId(Pgdb *pgdb, char *summaryName,
		int *summaryTypeId);

// 'Healing' summary

int wowstats_wsdalAddSummaryHealing(Pgdb *pgdb, int logHistoryId,
		int realmId, WowSummary *summary);

int wowstats_wsdalUpdateSummaryHealing(Pgdb *pgdb, int logHistoryId,
		int realmId, WowSummary *summary);

int wowstats_wsdalGetSummaryHealing(Pgdb *pgdb, int logHistoryId,
		int realmId, WowSummary *summary);

// 'Damage' summary

int wowstats_wsdalAddSummaryDamage(Pgdb *pgdb, int logHistoryId,
		int realmId, WowSummary *summary);

int wowstats_wsdalUpdateSummaryDamage(Pgdb *pgdb, int logHistoryId,
		int realmId, WowSummary *summary);

int wowstats_wsdalGetSummaryDamage(Pgdb *pgdb, int logHistoryId,
		int realmId, WowSummary *summary);

#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _WOWSTATS_SYSTEM_DAL_H

