/*
 * wsdal.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard-WoW statistical analysis data access layer (DAL).
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#include "wowstats/common.h"
#define _WOWSTATS_DATA_COMPONENT
#include "wowstats/data/summary.h"
#include "wowstats/core.h"
#include "wowstats/db.h"
#define _WOWSTATS_SYSTEM_COMPONENT
#include "wowstats/system/summary.h"
#include "wowstats/system/wsdal.h"

// declare wowstats data access layer private functions

static void cleanSqlString(char buffer[8192], char *string);

static int addSummaryRecord(Pgdb *pgdb, int logHistoryId, int realmId,
		WowSummary *summary);

static int getSummaryRecord(Pgdb *pgdb, int logHistoryId, int realmId,
		WowSummary *summary);

static int getRowInteger(Pgdb *pgdb, void *handle, int rowNumber,
		int columnNumber);


// define wowstats data access layer private functions

static void cleanSqlString(char buffer[8192], char *string)
{
	int ii = 0;
	int nn = 0;
	int cId = 0;
	int length = 0;
	int bufferLength = 0;

	bufferLength = ((int)sizeof(char) * 8192);
	memset(buffer, 0, bufferLength);

	length = strlen(string);
	for(ii = 0, nn = 0; ((ii < length) && (nn < bufferLength)); ii++) {
		cId = (int)string[ii];
		if(cId < 0) {
			cId += 256;
		}
		if((cId < 32) || (cId > 126)) {
//			if((cId < 192) || (cId > 254)) {
				continue;
//			}
		}

		buffer[nn] = string[ii];
		nn++;
		if(string[ii] == '\'') {
			buffer[nn] = '\'';
			nn++;
		}
	}
}

static int addSummaryRecord(Pgdb *pgdb, int logHistoryId, int realmId,
		WowSummary *summary)
{
	int summaryTypeId = 0;
	int sourceId = 0;
	int targetId = 0;
	int seconds = 0;
	int resultLength = 0;
	int resultCode = 0;
	char sql[8192];
	char *result = NULL;
	void *handle = NULL;

	const char *addTemplate =
		"insert into "
		"	cprSummary ("
		"		logHistoryId, "
		"		realmId, "
		"		type, "
		"		sourceId, "
		"		targetId, "
		"		elapsedTimeSeconds"
		"	) "
		"	values ("
		"		%i, "
		"		%i, "
		"		%i, "
		"		%i, "
		"		%i, "
		"		%i"
		"	)"
		";";

	const char *getTemplate =
		"select "
		"	currval('cprsummary_uid_seq') "
		"from "
		"	cprSummary "
		"limit "
		"	1"
		";";

	if(wowstats_timestampCalculateDifferenceInSeconds(
				&summary->lastTimestamp,
				&summary->firstTimestamp,
				&seconds) < 0) {
		return -1;
	}

	if(wowstats_wsdalGetSummaryTypeId(pgdb,
				WOW_SUMMARY_LIST[(summary->summaryId - 1)].name,
				&summaryTypeId) < 0) {
		return -1;
	}

	if(wowstats_wsdalGetEntityId(pgdb, realmId, summary->sourceUid,
				&sourceId) < 0) {
		return -1;
	}

	if(wowstats_wsdalGetEntityId(pgdb, realmId, summary->targetUid,
				&targetId) < 0) {
		return -1;
	}

	snprintf(sql, (sizeof(sql) - 1), addTemplate,
			logHistoryId,
			realmId,
			summaryTypeId,
			sourceId,
			targetId,
			seconds);

	if(pgdb_executeSql(pgdb, sql, PGRES_COMMAND_OK, &handle) < 0) {
		return -1;
	}

	if(pgdb_freeHandle(pgdb, handle) < 0) {
		return -1;
	}

	resultCode = -1;

	snprintf(sql, (sizeof(sql) - 1), getTemplate);

	if(pgdb_executeSql(pgdb, sql, PGRES_TUPLES_OK, &handle) < 0) {
		return -1;
	}

	result = pgdb_getRowResult(pgdb, handle, 0, 0, &resultLength);
	if((result == NULL) || (resultLength < 1)) {
		if(result != NULL) {
			free(result);
		}
		pgdb_freeHandle(pgdb, handle);
		return -1;
	}

	resultCode = atoi(result);

	free(result);

	if(pgdb_freeHandle(pgdb, handle) < 0) {
		return -1;
	}

	return resultCode;
}

static int getSummaryRecord(Pgdb *pgdb, int logHistoryId, int realmId,
		WowSummary *summary)
{
	int summaryTypeId = 0;
	int sourceId = 0;
	int targetId = 0;
	int resultLength = 0;
	int resultCode = 0;
	char sql[8192];
	char *tableName = "Unknown";
	char *result = NULL;
	void *handle = NULL;

	WowSummaryHealing *record = NULL;

	const char *template =
		"select "
		"	cs.uid "
		"from "
		"	cprSummary cs, "
		"	%s st "
		"where "
		"	cs.logHistoryId = %i and "
		"	cs.realmId = %i and "
		"	cs.type = %i and "
		"	cs.sourceId = %i and "
		"	cs.targetId = %i and "
		"	st.spellId = %i and "
		"	st.spellRank = %i and "
		"	cs.uid = st.summaryId"
		";";

	if(wowstats_wsdalGetSummaryTypeId(pgdb,
				WOW_SUMMARY_LIST[(summary->summaryId - 1)].name,
				&summaryTypeId) < 0) {
		return -1;
	}

	if(wowstats_wsdalGetEntityId(pgdb, realmId, summary->sourceUid,
				&sourceId) < 0) {
		return -1;
	}

	if(wowstats_wsdalGetEntityId(pgdb, realmId, summary->targetUid,
				&targetId) < 0) {
		return -1;
	}

	resultCode = -1;

	record = (WowSummaryHealing *)summary->record;

	switch(summary->summaryId) {
		case WOW_SUMMARY_HEALING:
			tableName = "cprSummaryHealing";
			break;
		case WOW_SUMMARY_DAMAGE:
			tableName = "cprSummaryDamage";
			break;
	}

	snprintf(sql, (sizeof(sql) - 1), template,
			tableName,
			logHistoryId,
			realmId,
			summaryTypeId,
			sourceId,
			targetId,
			record->spellId,
			record->spellRank);

	if(pgdb_executeSql(pgdb, sql, PGRES_TUPLES_OK, &handle) < 0) {
		return -1;
	}

	result = pgdb_getRowResult(pgdb, handle, 0, 0, &resultLength);
	if((result == NULL) || (resultLength < 1)) {
		if(result != NULL) {
			free(result);
		}
		pgdb_freeHandle(pgdb, handle);
		return -1;
	}

	resultCode = atoi(result);

	free(result);

	if(pgdb_freeHandle(pgdb, handle) < 0) {
		return -1;
	}

	return resultCode;
}

static int updateSummaryRecord(Pgdb *pgdb, int logHistoryId, int realmId,
		int summaryId, WowSummary *summary)
{
	int elapsedTimeSeconds = 0;
	char sql[8192];
	void *handle = NULL;

	const char *template =
		"update "
		"	cprSummary "
		"set "
		"	elapsedTimeSeconds = %i "
		"where "
		"	uid = %i"
		";";

	if(wowstats_timestampCalculateDifferenceInSeconds(
				&summary->lastTimestamp,
				&summary->firstTimestamp,
				&elapsedTimeSeconds) < 0) {
		return -1;
	}

	if(summary->extraElapsedTimeSeconds > 0) {
		elapsedTimeSeconds += summary->extraElapsedTimeSeconds;
	}

	snprintf(sql, (sizeof(sql) - 1), template,
			elapsedTimeSeconds,
			summaryId);

	if(pgdb_executeSql(pgdb, sql, PGRES_COMMAND_OK, &handle) < 0) {
		return -1;
	}

	if(pgdb_freeHandle(pgdb, handle) < 0) {
		return -1;
	}

	return 0;
}

static int getRowInteger(Pgdb *pgdb, void *handle, int rowNumber,
		int columnNumber)
{
	int result = 0;
	int stringLength = 0;
	char *string = NULL;

	string = pgdb_getRowResult(pgdb, handle, rowNumber, columnNumber,
			&stringLength);
	if((string == NULL) || (stringLength < 1)) {
		if(string != NULL) {
			free(string);
		}
		return -1;
	}

	result = atoi(string);

	free(string);

	return result;
}


// define wowstats data access layer public functions

// user functions

int wowstats_wsdalGetUserId(Pgdb *pgdb, char *emailAddress, int *userId)
{
	int resultLength = 0;
	char emailAddressBuffer[8192];
	char sql[8192];
	char *result = NULL;
	void *handle = NULL;

	const char *template =
		"select "
		"	uid "
		"from "
		"	cprUser "
		"where "
		"	emailAddress = '%s'"
		";";

	if((pgdb == NULL) || (emailAddress == NULL) || (userId == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	*userId = 0;

	cleanSqlString(emailAddressBuffer, emailAddress);

	snprintf(sql, (sizeof(sql) - 1), template,
			emailAddressBuffer);

	if(pgdb_executeSql(pgdb, sql, PGRES_TUPLES_OK, &handle) < 0) {
		return -1;
	}

	result = pgdb_getRowResult(pgdb, handle, 0, 0, &resultLength);
	if((result == NULL) || (resultLength < 1)) {
		if(result != NULL) {
			free(result);
		}
		pgdb_freeHandle(pgdb, handle);
		return -1;
	}

	*userId = atoi(result);

	free(result);

	if(pgdb_freeHandle(pgdb, handle) < 0) {
		return -1;
	}

	return 0;
}

// log history functions

int wowstats_wsdalGetLastLogHistoryId(Pgdb *pgdb, int *logHistoryId)
{
	int resultLength = 0;
	char sql[8192];
	char *result = NULL;
	void *handle = NULL;

	const char *template =
		"select "
		"	currval('cprloghistory_uid_seq') "
		"from "
		"	cprLogHistory "
		"limit "
		"	1"
		";";

	if((pgdb == NULL) || (logHistoryId == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	*logHistoryId = 0;

	snprintf(sql, (sizeof(sql) - 1), template);

	if(pgdb_executeSql(pgdb, sql, PGRES_TUPLES_OK, &handle) < 0) {
		return -1;
	}

	result = pgdb_getRowResult(pgdb, handle, 0, 0, &resultLength);
	if((result == NULL) || (resultLength < 1)) {
		if(result != NULL) {
			free(result);
		}
		pgdb_freeHandle(pgdb, handle);
		return -1;
	}

	*logHistoryId = atoi(result);

	free(result);

	if(pgdb_freeHandle(pgdb, handle) < 0) {
		return -1;
	}

	return 0;
}

int wowstats_wsdalAddLogHistory(Pgdb *pgdb, int userId, int realmId,
		char *logType, char *logFilename, char *logDescription,
		int playSeconds, char *logTimestamp)
{
	char logTypeBuffer[8192];
	char logFilenameBuffer[8192];
	char logDescriptionBuffer[8192];
	char logTimestampBuffer[8192];
	char dateCreated[32];
	char sql[8192];
	void *handle = NULL;

	const char *template =
		"insert into "
		"	cprLogHistory ("
		"		userId, "
		"		realmId, "
		"		type, "
		"		filename, "
		"		description, "
		"		playSeconds, "
		"		dateUploaded, "
		"		dateCreated, "
		"		dateModified"
		"	) "
		"	values ("
		"		%i, "
		"		%i, "
		"		'%s', "
		"		'%s', "
		"		'%s', "
		"		%i, "
		"		'%s', "
		"		'%s', "
		"		'%s'"
		"	)"
		";";

	if((pgdb == NULL) || (userId < 1) || (realmId < 1) || (logType == NULL) ||
			(logFilename == NULL) || (logDescription == NULL) ||
			(playSeconds < 0) || (logTimestamp == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	wowstats_timestampNowToString(dateCreated);

	cleanSqlString(logTypeBuffer, logType);
	cleanSqlString(logFilenameBuffer, logFilename);
	cleanSqlString(logDescriptionBuffer, logDescription);
	cleanSqlString(logTimestampBuffer, logTimestamp);

	snprintf(sql, (sizeof(sql) - 1), template,
			userId,
			realmId,
			logTypeBuffer,
			logFilenameBuffer,
			logDescriptionBuffer,
			playSeconds,
			dateCreated,
			logTimestamp,
			logTimestamp);

	if(pgdb_executeSql(pgdb, sql, PGRES_COMMAND_OK, &handle) < 0) {
		return -1;
	}

	if(pgdb_freeHandle(pgdb, handle) < 0) {
		return -1;
	}

	return 0;
}

int wowstats_wsdalSetLogHistoryPlaySeconds(Pgdb *pgdb, int logHistoryId,
		int playSeconds)
{
	char sql[8192];
	void *handle = NULL;

	const char *template =
		"update "
		"	cprLogHistory "
		"set "
		"	playSeconds = %i "
		"where "
		"	uid = %i "
		";";

	if((pgdb == NULL) || (logHistoryId < 1) || (playSeconds < 0)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	snprintf(sql, (sizeof(sql) - 1), template,
			playSeconds,
			logHistoryId);

	if(pgdb_executeSql(pgdb, sql, PGRES_COMMAND_OK, &handle) < 0) {
		return -1;
	}

	if(pgdb_freeHandle(pgdb, handle) < 0) {
		return -1;
	}

	return 0;
}

int wowstats_wsdalAddContributingLog(Pgdb *pgdb, int logHistoryId, int userId,
		int realmId, char *logType, char *logFilename, char *logDescription,
		char *logTimestamp)
{
	char logTypeBuffer[8192];
	char logFilenameBuffer[8192];
	char logDescriptionBuffer[8192];
	char dateCreated[32];
	char sql[8192];
	void *handle = NULL;

	const char *template =
		"insert into "
		"	cprContributingLog ("
		"		logHistoryId, "
		"		userId, "
		"		realmId, "
		"		type, "
		"		filename, "
		"		description, "
		"		dateUploaded, "
		"		dateCreated, "
		"		dateModified"
		"	) "
		"	values ("
		"		%i, "
		"		%i, "
		"		%i, "
		"		'%s', "
		"		'%s', "
		"		'%s', "
		"		'%s', "
		"		'%s', "
		"		'%s'"
		"	)"
		";";

	if((pgdb == NULL) || (logHistoryId < 1) || (userId < 1) ||
			(realmId < 1) || (logType == NULL) || (logFilename == NULL) ||
			(logDescription == NULL) || (logTimestamp == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	wowstats_timestampNowToString(dateCreated);

	cleanSqlString(logTypeBuffer, logType);
	cleanSqlString(logFilenameBuffer, logFilename);
	cleanSqlString(logDescriptionBuffer, logDescription);

	snprintf(sql, (sizeof(sql) - 1), template,
			logHistoryId,
			userId,
			realmId,
			logTypeBuffer,
			logFilenameBuffer,
			logDescriptionBuffer,
			dateCreated,
			logTimestamp,
			logTimestamp);

	if(pgdb_executeSql(pgdb, sql, PGRES_COMMAND_OK, &handle) < 0) {
		return -1;
	}

	if(pgdb_freeHandle(pgdb, handle) < 0) {
		return -1;
	}

	return 0;
}

int wowstats_wsdalGetLastMobLogId(Pgdb *pgdb, int *mobLogId)
{
	int resultLength = 0;
	char sql[8192];
	char *result = NULL;
	void *handle = NULL;

	const char *template =
		"select "
		"	currval('cprloghistory_moblogid_seq') "
		"from "
		"	cprLogHistory "
		"limit "
		"	1"
		";";

	if((pgdb == NULL) || (mobLogId == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	*mobLogId = 0;

	snprintf(sql, (sizeof(sql) - 1), template);

	if(pgdb_executeSql(pgdb, sql, PGRES_TUPLES_OK, &handle) < 0) {
		return -1;
	}

	result = pgdb_getRowResult(pgdb, handle, 0, 0, &resultLength);
	if((result == NULL) || (resultLength < 1)) {
		if(result != NULL) {
			free(result);
		}
		pgdb_freeHandle(pgdb, handle);
		return -1;
	}

	*mobLogId = atoi(result);

	free(result);

	if(pgdb_freeHandle(pgdb, handle) < 0) {
		return -1;
	}

	return 0;
}

int wowstats_wsdalAddMobLog(Pgdb *pgdb, int logHistoryId, int userId,
		int realmId, char *logType, char *logFilename, char *logDescription,
		int playSeconds, char *logTimestamp)
{
	char logTypeBuffer[8192];
	char logFilenameBuffer[8192];
	char logDescriptionBuffer[8192];
	char logTimestampBuffer[8192];
	char dateCreated[32];
	char sql[8192];
	void *handle = NULL;

	const char *template =
		"insert into "
		"	cprLogHistory ("
		"		parentLogId, "
		"		userId, "
		"		realmId, "
		"		type, "
		"		filename, "
		"		description, "
		"		playSeconds, "
		"		dateUploaded, "
		"		dateCreated, "
		"		dateModified"
		"	) "
		"	values ("
		"		%i, "
		"		%i, "
		"		%i, "
		"		'%s', "
		"		'%s', "
		"		'%s', "
		"		%i, "
		"		'%s', "
		"		'%s', "
		"		'%s'"
		"	)"
		";";

	if((pgdb == NULL) || (logHistoryId < 0) || (userId < 1) || (realmId < 1) ||
			(logType == NULL) || (logFilename == NULL) ||
			(logDescription == NULL) || (playSeconds < 0) ||
			(logTimestamp == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	wowstats_timestampNowToString(dateCreated);

	cleanSqlString(logTypeBuffer, logType);
	cleanSqlString(logFilenameBuffer, logFilename);
	cleanSqlString(logDescriptionBuffer, logDescription);
	cleanSqlString(logTimestampBuffer, logTimestamp);

	snprintf(sql, (sizeof(sql) - 1), template,
			logHistoryId,
			userId,
			realmId,
			logTypeBuffer,
			logFilenameBuffer,
			logDescriptionBuffer,
			playSeconds,
			dateCreated,
			logTimestamp,
			logTimestamp);

	if(pgdb_executeSql(pgdb, sql, PGRES_COMMAND_OK, &handle) < 0) {
		return -1;
	}

	if(pgdb_freeHandle(pgdb, handle) < 0) {
		return -1;
	}

	return 0;
}


// realm functions

int wowstats_wsdalGetRealmId(Pgdb *pgdb, char *regionCode, char *realmName,
		int *realmId)
{
	int resultLength = 0;
	char regionCodeBuffer[8192];
	char realmNameBuffer[8192];
	char sql[8192];
	char *result = NULL;
	void *handle = NULL;

	const char *template =
		"select "
		"	uid "
		"from "
		"	cprRealm "
		"where "
		"	region = '%s' and "
		"	name = '%s'"
		";";

	if((pgdb == NULL) || (regionCode == NULL) || (realmName == NULL) ||
			(realmId == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	*realmId = 0;

	cleanSqlString(regionCodeBuffer, regionCode);
	cleanSqlString(realmNameBuffer, realmName);

	snprintf(sql, (sizeof(sql) - 1), template,
			regionCodeBuffer,
			realmNameBuffer);

	if(pgdb_executeSql(pgdb, sql, PGRES_TUPLES_OK, &handle) < 0) {
		return -1;
	}

	result = pgdb_getRowResult(pgdb, handle, 0, 0, &resultLength);
	if((result == NULL) || (resultLength < 1)) {
		if(result != NULL) {
			free(result);
		}
		pgdb_freeHandle(pgdb, handle);
		return -1;
	}

	*realmId = atoi(result);

	free(result);

	if(pgdb_freeHandle(pgdb, handle) < 0) {
		return -1;
	}

	return 0;
}

// entity functions

int wowstats_wsdalGetEntityId(Pgdb *pgdb, int realmId, char *uid,
		int *entityId)
{
	int resultLength = 0;
	char uidBuffer[8192];
	char sql[8192];
	char *result = NULL;
	void *handle = NULL;

	const char *template =
		"select "
		"	uid "
		"from "
		"	cprEntity "
		"where "
		"	realmId = %i and "
		"	guid = '%s'"
		";";

	if((pgdb == NULL) || (realmId < 1) || (uid == NULL) ||
			(entityId == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	*entityId = 0;

	cleanSqlString(uidBuffer, uid);

	snprintf(sql, (sizeof(sql) - 1), template,
			realmId,
			uidBuffer);

	if(pgdb_executeSql(pgdb, sql, PGRES_TUPLES_OK, &handle) < 0) {
		return -1;
	}

	if(pgdb_getRowCount(pgdb, handle) == 0) {
		*entityId = -1;
	}
	else {
		result = pgdb_getRowResult(pgdb, handle, 0, 0, &resultLength);
		if((result == NULL) || (resultLength < 1)) {
			if(result != NULL) {
				free(result);
			}
			pgdb_freeHandle(pgdb, handle);
			return -1;
		}
	
		*entityId = atoi(result);

		free(result);
	}

	if(pgdb_freeHandle(pgdb, handle) < 0) {
		return -1;
	}

	return 0;
}

int wowstats_wsdalAddEntity(Pgdb *pgdb, int realmId, char *className,
		char *name, char *uid, int totalPlayedSeconds)
{
	char classNameBuffer[8192];
	char nameBuffer[8192];
	char uidBuffer[8192];
	char sql[8192];
	void *handle = NULL;

	const char *template =
		"insert into "
		"	cprEntity ("
		"		realmId, "
		"		className, "
		"		name, "
		"		guid, "
		"		totalPlayedSeconds"
		"	) "
		"	values ("
		"		%i, "
		"		'%s', "
		"		'%s', "
		"		'%s', "
		"		%i"
		"	)"
		";";

	if((pgdb == NULL) || (realmId < 1) || (className == NULL) ||
			(name == NULL) || (uid == NULL) || (totalPlayedSeconds < 0)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	cleanSqlString(classNameBuffer, className);
	cleanSqlString(nameBuffer, name);
	cleanSqlString(uidBuffer, uid);

	snprintf(sql, (sizeof(sql) - 1), template,
			realmId,
			classNameBuffer,
			nameBuffer,
			uidBuffer,
			totalPlayedSeconds);

	if(pgdb_executeSql(pgdb, sql, PGRES_COMMAND_OK, &handle) < 0) {
		return -1;
	}

	if(pgdb_freeHandle(pgdb, handle) < 0) {
		return -1;
	}

	return 0;
}

int wowstats_wsdalUpdateEntity(Pgdb *pgdb, int entityId, char *className,
		int totalPlayedSeconds)
{
	int stringLength = 0;
	int recordTotalPlayedSeconds = 0;
	char *string = NULL;
	char classNameBuffer[8192];
	char sql[8192];
	void *handle = NULL;

	const char *selectTemplate =
		"select "
		"	className, "
		"	totalPlayedSeconds "
		"from "
		"	cprEntity "
		"where "
		"	uid = %i"
		";";

	const char *updateTemplate =
		"update "
		"	cprEntity "
		"set "
		"	className = '%s', "
		"	totalPlayedSeconds = %i "
		"where "
		"	uid = %i"
		";";

	if((pgdb == NULL) || (entityId < 1) || (totalPlayedSeconds < 0)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	// obtain the current meta-data from the record

	snprintf(sql, (sizeof(sql) - 1), selectTemplate,
			entityId);

	if(pgdb_executeSql(pgdb, sql, PGRES_TUPLES_OK, &handle) < 0) {
		return -1;
	}

	if((pgdb_getRowCount(pgdb, handle) != 1) &&
			(pgdb_getColumnCount(pgdb, handle) != 2)) {
		pgdb_freeHandle(pgdb, handle);
		return -1;
	}

	string = pgdb_getRowResult(pgdb, handle, 0, 0, &stringLength);
	if((string == NULL) || (stringLength < 1)) {
		if(string != NULL) {
			free(string);
		}
		pgdb_freeHandle(pgdb, handle);
		return -1;
	}

	if((className != NULL) && (strcmp(className, string))) {
		cleanSqlString(classNameBuffer, string);
	}
	else {
		cleanSqlString(classNameBuffer, className);
	}

	free(string);

	string = pgdb_getRowResult(pgdb, handle, 0, 1, &stringLength);
	if((string == NULL) || (stringLength < 1)) {
		if(string != NULL) {
			free(string);
		}
		pgdb_freeHandle(pgdb, handle);
		return -1;
	}

	recordTotalPlayedSeconds = atoi(string);

	free(string);

	if(pgdb_freeHandle(pgdb, handle) < 0) {
		return -1;
	}

	// update the record

	recordTotalPlayedSeconds += totalPlayedSeconds;

	snprintf(sql, (sizeof(sql) - 1), updateTemplate,
			classNameBuffer,
			recordTotalPlayedSeconds,
			entityId);

	if(pgdb_executeSql(pgdb, sql, PGRES_COMMAND_OK, &handle) < 0) {
		return -1;
	}

	if(pgdb_freeHandle(pgdb, handle) < 0) {
		return -1;
	}

	return 0;
}

int wowstats_wsdalAddEntityLog(Pgdb *pgdb, int entityId, int logHistoryId,
		int totalPlayedSeconds)
{
	char sql[8192];
	void *handle = NULL;

	const char *template =
		"insert into "
		"	cprEntityLog ("
		"		entityId, "
		"		logHistoryId, "
		"		totalPlayedSeconds"
		"	) "
		"	values ("
		"		%i, "
		"		%i, "
		"		%i"
		"	)"
		";";

	if((pgdb == NULL) || (entityId < 1) || (logHistoryId < 1) ||
			(totalPlayedSeconds < 0)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	snprintf(sql, (sizeof(sql) - 1), template,
			entityId,
			logHistoryId,
			totalPlayedSeconds);

	if(pgdb_executeSql(pgdb, sql, PGRES_COMMAND_OK, &handle) < 0) {
		return -1;
	}

	if(pgdb_freeHandle(pgdb, handle) < 0) {
		return -1;
	}

	return 0;
}

// summary functions

int wowstats_wsdalGetSummaryTypeId(Pgdb *pgdb, char *summaryName,
		int *summaryTypeId)
{
	int resultLength = 0;
	char summaryNameBuffer[8192];
	char sql[8192];
	char *result = NULL;
	void *handle = NULL;

	const char *template =
		"select "
		"	uid "
		"from "
		"	cprSummaryType "
		"where "
		"	name = '%s'"
		";";

	if((pgdb == NULL) || (summaryName == NULL) || (summaryTypeId == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	*summaryTypeId = 0;

	cleanSqlString(summaryNameBuffer, summaryName);

	snprintf(sql, (sizeof(sql) - 1), template,
			summaryNameBuffer);

	if(pgdb_executeSql(pgdb, sql, PGRES_TUPLES_OK, &handle) < 0) {
		return -1;
	}

	result = pgdb_getRowResult(pgdb, handle, 0, 0, &resultLength);
	if((result == NULL) || (resultLength < 1)) {
		if(result != NULL) {
			free(result);
		}
		pgdb_freeHandle(pgdb, handle);
		return -1;
	}

	*summaryTypeId = atoi(result);

	free(result);

	if(pgdb_freeHandle(pgdb, handle) < 0) {
		return -1;
	}

	return 0;
}

// 'Healing' summary

int wowstats_wsdalAddSummaryHealing(Pgdb *pgdb, int logHistoryId,
		int realmId, WowSummary *summary)
{
	int summaryId = 0;
	char spellNameBuffer[8192];
	char spellSchoolBuffer[8192];
	char sql[8192];
	void *handle = NULL;

	WowSummaryHealing *record = NULL;

	const char *template =
		"insert into "
		"	cprSummaryHealing ("
		"		summaryId, "
		"		spellId, "
		"		spellName, "
		"		spellSchool, "
		"		spellRank, "
		"		directCount, "
		"		periodicCount, "
		"		healAmount, "
		"		overhealAmount, "
		"		criticalHealAmount, "
		"		criticalOverhealAmount, "
		"		criticalCount"
		"	) "
		"	values ("
		"		%i, "
		"		%i, "
		"		'%s', "
		"		'%s', "
		"		%i, "
		"		%i, "
		"		%i, "
		"		%i, "
		"		%i, "
		"		%i, "
		"		%i, "
		"		%i"
		"	)"
		";";

	if((pgdb == NULL) || (realmId < 1) || (summary == NULL) ||
			(summary->summaryId != WOW_SUMMARY_HEALING)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	summaryId = addSummaryRecord(pgdb, logHistoryId, realmId, summary);
	if(summaryId < 0) {
		return -1;
	}

	record = (WowSummaryHealing *)summary->record;

	cleanSqlString(spellNameBuffer, record->spellName);
	cleanSqlString(spellSchoolBuffer, record->spellSchool);

	snprintf(sql, (sizeof(sql) - 1), template,
			summaryId,
			record->spellId,
			spellNameBuffer,
			spellSchoolBuffer,
			record->spellRank,
			record->directCount,
			record->periodicCount,
			record->healAmount,
			record->overhealAmount,
			record->criticalHealAmount,
			record->criticalOverhealAmount,
			record->criticalCount);

	if(pgdb_executeSql(pgdb, sql, PGRES_COMMAND_OK, &handle) < 0) {
		return -1;
	}

	if(pgdb_freeHandle(pgdb, handle) < 0) {
		return -1;
	}

	return 0;
}

int wowstats_wsdalUpdateSummaryHealing(Pgdb *pgdb, int logHistoryId,
		int realmId, WowSummary *summary)
{
	int summaryId = 0;
	char sql[8192];
	void *handle = NULL;

	WowSummaryHealing *record = NULL;

	const char *template =
		"update "
		"	cprSummaryHealing "
		"set "
		"	directCount = %i, "
		"	periodicCount = %i, "
		"	healAmount = %i, "
		"	overhealAmount = %i, "
		"	criticalHealAmount = %i, "
		"	criticalOverhealAmount = %i, "
		"	criticalCount = %i "
		"where "
		"	summaryId = %i"
		";";

	if((pgdb == NULL) || (realmId < 1) || (summary == NULL) ||
			(summary->summaryId != WOW_SUMMARY_HEALING)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	summaryId = getSummaryRecord(pgdb, logHistoryId, realmId, summary);
	if(summaryId < 0) {
		return -1;
	}

	if(updateSummaryRecord(pgdb, logHistoryId, realmId, summaryId,
				summary) < 0) {
		return -1;
	}

	record = (WowSummaryHealing *)summary->record;

	snprintf(sql, (sizeof(sql) - 1), template,
			record->directCount,
			record->periodicCount,
			record->healAmount,
			record->overhealAmount,
			record->criticalHealAmount,
			record->criticalOverhealAmount,
			record->criticalCount,
			summaryId);

	if(pgdb_executeSql(pgdb, sql, PGRES_COMMAND_OK, &handle) < 0) {
		return -1;
	}

	if(pgdb_freeHandle(pgdb, handle) < 0) {
		return -1;
	}

	return 0;
}

int wowstats_wsdalGetSummaryHealing(Pgdb *pgdb, int logHistoryId,
		int realmId, WowSummary *summary)
{
	int iValue = 0;
	int sourceId = 0;
	int targetId = 0;
	int summaryTypeId = 0;
	int resultLength = 0;
	char sql[8192];
	char *result = NULL;
	void *handle = NULL;

	WowSummaryHealing *record = NULL;

	const char *template =
		"select "
		"	sh.spellId, "
		"	sh.spellName, "
		"	sh.spellSchool, "
		"	sh.spellRank, "
		"	sh.directCount, "
		"	sh.periodicCount, "
		"	sh.healAmount, "
		"	sh.overhealAmount, "
		"	sh.criticalHealAmount, "
		"	sh.criticalOverhealAmount, "
		"	sh.criticalCount, "
		"	cs.elapsedTimeSeconds "
		"from "
		"	cprSummary cs, "
		"	cprSummaryHealing sh "
		"where "
		"	cs.uid = sh.summaryId and "
		"	cs.logHistoryId = %i and "
		"	cs.realmId = %i and "
		"	cs.type = %i and "
		"	cs.sourceId = %i and "
		"	cs.targetId = %i and "
		"	sh.spellId = %i and "
		"	sh.spellRank = %i"
		";";

	if((pgdb == NULL) || (logHistoryId < 1) || (realmId < 1) ||
			(summary == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(wowstats_wsdalGetSummaryTypeId(pgdb,
				WOW_SUMMARY_LIST[(summary->summaryId - 1)].name,
				&summaryTypeId) < 0) {
		return -1;
	}

	if(wowstats_wsdalGetEntityId(pgdb, realmId, summary->sourceUid,
				&sourceId) < 0) {
		return -1;
	}

	if(wowstats_wsdalGetEntityId(pgdb, realmId, summary->targetUid,
				&targetId) < 0) {
		return -1;
	}

	record = (WowSummaryHealing *)summary->record;

	snprintf(sql, (sizeof(sql) - 1), template,
			logHistoryId,
			realmId,
			summaryTypeId,
			sourceId,
			targetId,
			record->spellId,
			record->spellRank);

	if(pgdb_executeSql(pgdb, sql, PGRES_TUPLES_OK, &handle) < 0) {
		return -1;
	}

	if(pgdb_getRowCount(pgdb, handle) != 1) {
		pgdb_freeHandle(pgdb, handle);
		return -1;
	}

	iValue = getRowInteger(pgdb, handle, 0, 0);
	if(record->spellId != iValue) {
		pgdb_freeHandle(pgdb, handle);
		return -1;
	}

	result = pgdb_getRowResult(pgdb, handle, 0, 1, &resultLength);
	if((result == NULL) || (resultLength < 1)) {
		if(result != NULL) {
			free(result);
		}
		pgdb_freeHandle(pgdb, handle);
		return -1;
	}

	if(strcmp(record->spellName, result)) {
		free(result);
		pgdb_freeHandle(pgdb, handle);
		return -1;
	}

	free(result);

	result = pgdb_getRowResult(pgdb, handle, 0, 2, &resultLength);
	if((result == NULL) || (resultLength < 1)) {
		if(result != NULL) {
			free(result);
		}
		pgdb_freeHandle(pgdb, handle);
		return -1;
	}

	if(strcmp(record->spellSchool, result)) {
		free(result);
		pgdb_freeHandle(pgdb, handle);
		return -1;
	}

	free(result);

	iValue = getRowInteger(pgdb, handle, 0, 3);
	if(record->spellRank != iValue) {
		pgdb_freeHandle(pgdb, handle);
		return -1;
	}

	record->directCount = getRowInteger(pgdb, handle, 0, 4);
	record->periodicCount = getRowInteger(pgdb, handle, 0, 5);
	record->healAmount = getRowInteger(pgdb, handle, 0, 6);
	record->overhealAmount = getRowInteger(pgdb, handle, 0, 7);
	record->criticalHealAmount = getRowInteger(pgdb, handle, 0, 8);
	record->criticalOverhealAmount = getRowInteger(pgdb, handle, 0, 9);
	record->criticalCount = getRowInteger(pgdb, handle, 0, 10);

	summary->extraElapsedTimeSeconds = getRowInteger(pgdb, handle, 0, 11);

	if(pgdb_freeHandle(pgdb, handle) < 0) {
		return -1;
	}

	return 0;
}

// 'Damage' summary

int wowstats_wsdalAddSummaryDamage(Pgdb *pgdb, int logHistoryId,
		int realmId, WowSummary *summary)
{
	int summaryId = 0;
	char spellNameBuffer[8192];
	char spellSchoolBuffer[8192];
	char damageTypeBuffer[8192];
	char sql[8192];
	void *handle = NULL;

	WowSummaryDamage *record = NULL;

	const char *template =
		"insert into "
		"	cprSummaryDamage ("
		"		summaryId, "
		"		spellId, "
		"		spellName, "
		"		spellSchool, "
		"		spellRank, "
		"		damageType, "
		"		directCount, "
		"		periodicCount, "
		"		damageAmount, "
		"		overkillAmount, "
		"		resistAmount, "
		"		blockAmount, "
		"		missedCount, "
		"		missedAmount, "
		"		absorbedCount, "
		"		absorbedDamageAmount, "
		"		absorbedOverkillAmount, "
		"		absorbedResistAmount, "
		"		absorbedBlockAmount, "
		"		criticalCount, "
		"		criticalDamageAmount, "
		"		criticalOverkillAmount, "
		"		criticalResistAmount, "
		"		criticalBlockAmount, "
		"		glancingCount, "
		"		glancingDamageAmount, "
		"		glancingOverkillAmount, "
		"		glancingResistAmount, "
		"		glancingBlockAmount, "
		"		crushingCount, "
		"		crushingDamageAmount, "
		"		crushingOverkillAmount, "
		"		crushingResistAmount, "
		"		crushingBlockAmount "
		"	) "
		"	values ("
		"		%i, "		// summaryId
		"		%i, "		// spellId
		"		'%s', "		// spellName
		"		'%s', "		// spellSchool
		"		%i, "		// spellRank
		"		'%s', "		// damageType
		"		%i, "		// directCount
		"		%i, "		// periodicCount
		"		%i, "		// damageAmount
		"		%i, "		// overkillAmount
		"		%i, "		// resistAmount
		"		%i, "		// blockAmount
		"		%i, "		// missedCount
		"		%i, "		// missedAmount
		"		%i, "		// absorbedCount
		"		%i, "		// absorbedDamageAmount
		"		%i, "		// absorbedOverkillAmount
		"		%i, "		// absorbedResistAmount
		"		%i, "		// absorbedBlockAmount
		"		%i, "		// criticalCount
		"		%i, "		// criticalDamageAmount
		"		%i, "		// criticalOverkillAmount
		"		%i, "		// criticalResistAmount
		"		%i, "		// criticalBlockAmount
		"		%i, "		// glancingCount
		"		%i, "		// glancingDamageAmount
		"		%i, "		// glancingOverkillAmount
		"		%i, "		// glancingResistAmount
		"		%i, "		// glancingBlockAmount
		"		%i, "		// crushingCount
		"		%i, "		// crushingDamageAmount
		"		%i, "		// crushingOverkillAmount
		"		%i, "		// crushingResistAmount
		"		%i "		// crushingBlockAmount
		"	)"
		";";

	if((pgdb == NULL) || (realmId < 1) || (summary == NULL) ||
			(summary->summaryId != WOW_SUMMARY_DAMAGE)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	summaryId = addSummaryRecord(pgdb, logHistoryId, realmId, summary);
	if(summaryId < 0) {
		return -1;
	}

	record = (WowSummaryDamage *)summary->record;

	cleanSqlString(spellNameBuffer, record->spellName);
	cleanSqlString(spellSchoolBuffer, record->spellSchool);
	cleanSqlString(damageTypeBuffer, record->damageType);

	snprintf(sql, (sizeof(sql) - 1), template,
			summaryId,
			record->spellId,
			spellNameBuffer,
			spellSchoolBuffer,
			record->spellRank,
			damageTypeBuffer,
			record->directCount,
			record->periodicCount,
			record->damageAmount,
			record->overkillAmount,
			record->resistAmount,
			record->blockAmount,
			record->missedCount,
			record->missedAmount,
			record->absorbedCount,
			record->absorbedDamageAmount,
			record->absorbedOverkillAmount,
			record->absorbedResistAmount,
			record->absorbedBlockAmount,
			record->criticalCount,
			record->criticalDamageAmount,
			record->criticalOverkillAmount,
			record->criticalResistAmount,
			record->criticalBlockAmount,
			record->glancingCount,
			record->glancingDamageAmount,
			record->glancingOverkillAmount,
			record->glancingResistAmount,
			record->glancingBlockAmount,
			record->crushingCount,
			record->crushingDamageAmount,
			record->crushingOverkillAmount,
			record->crushingResistAmount,
			record->crushingBlockAmount);

	if(pgdb_executeSql(pgdb, sql, PGRES_COMMAND_OK, &handle) < 0) {
		return -1;
	}

	if(pgdb_freeHandle(pgdb, handle) < 0) {
		return -1;
	}

	return 0;
}

int wowstats_wsdalUpdateSummaryDamage(Pgdb *pgdb, int logHistoryId,
		int realmId, WowSummary *summary)
{
	int summaryId = 0;
	char sql[8192];
	void *handle = NULL;

	WowSummaryDamage *record = NULL;

	const char *template =
		"update "
		"	cprSummaryDamage "
		"set "
		"	directCount = %i, "
		"	periodicCount = %i, "
		"	damageAmount = %i, "
		"	overkillAmount = %i, "
		"	resistAmount = %i, "
		"	blockAmount = %i, "
		"	missedCount = %i, "
		"	missedAmount = %i, "
		"	absorbedCount = %i, "
		"	absorbedDamageAmount = %i, "
		"	absorbedOverkillAmount = %i, "
		"	absorbedResistAmount = %i, "
		"	absorbedBlockAmount = %i, "
		"	criticalCount = %i, "
		"	criticalDamageAmount = %i, "
		"	criticalOverkillAmount = %i, "
		"	criticalResistAmount = %i, "
		"	criticalBlockAmount = %i, "
		"	glancingCount = %i, "
		"	glancingDamageAmount = %i, "
		"	glancingOverkillAmount = %i, "
		"	glancingResistAmount = %i, "
		"	glancingBlockAmount = %i, "
		"	crushingCount = %i, "
		"	crushingDamageAmount = %i, "
		"	crushingOverkillAmount = %i, "
		"	crushingResistAmount = %i, "
		"	crushingBlockAmount = %i "
		"where "
		"	summaryId = %i"
		";";

	if((pgdb == NULL) || (realmId < 1) || (summary == NULL) ||
			(summary->summaryId != WOW_SUMMARY_DAMAGE)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	summaryId = getSummaryRecord(pgdb, logHistoryId, realmId, summary);
	if(summaryId < 0) {
		return -1;
	}

	if(updateSummaryRecord(pgdb, logHistoryId, realmId, summaryId,
				summary) < 0) {
		return -1;
	}

	record = (WowSummaryDamage *)summary->record;

	snprintf(sql, (sizeof(sql) - 1), template,
			record->directCount,
			record->periodicCount,
			record->damageAmount,
			record->overkillAmount,
			record->resistAmount,
			record->blockAmount,
			record->missedCount,
			record->missedAmount,
			record->absorbedCount,
			record->absorbedDamageAmount,
			record->absorbedOverkillAmount,
			record->absorbedResistAmount,
			record->absorbedBlockAmount,
			record->criticalCount,
			record->criticalDamageAmount,
			record->criticalOverkillAmount,
			record->criticalResistAmount,
			record->criticalBlockAmount,
			record->glancingCount,
			record->glancingDamageAmount,
			record->glancingOverkillAmount,
			record->glancingResistAmount,
			record->glancingBlockAmount,
			record->crushingCount,
			record->crushingDamageAmount,
			record->crushingOverkillAmount,
			record->crushingResistAmount,
			record->crushingBlockAmount,
			summaryId);

	if(pgdb_executeSql(pgdb, sql, PGRES_COMMAND_OK, &handle) < 0) {
		return -1;
	}

	if(pgdb_freeHandle(pgdb, handle) < 0) {
		return -1;
	}

	return 0;
}

int wowstats_wsdalGetSummaryDamage(Pgdb *pgdb, int logHistoryId,
		int realmId, WowSummary *summary)
{
	int sourceId = 0;
	int targetId = 0;
	int summaryTypeId = 0;
	int resultLength = 0;
	char sql[8192];
	char *result = NULL;
	void *handle = NULL;

	WowSummaryDamage *record = NULL;

	const char *template =
		"select "
		"	sd.spellName, "
		"	sd.spellSchool, "
		"	sd.directCount, "
		"	sd.periodicCount, "
		"	sd.damageAmount, "
		"	sd.overkillAmount, "
		"	sd.resistAmount, "
		"	sd.blockAmount, "
		"	sd.missedCount, "
		"	sd.missedAmount, "
		"	sd.absorbedCount, "
		"	sd.absorbedDamageAmount, "
		"	sd.absorbedOverkillAmount, "
		"	sd.absorbedResistAmount, "
		"	sd.absorbedBlockAmount, "
		"	sd.criticalCount, "
		"	sd.criticalDamageAmount, "
		"	sd.criticalOverkillAmount, "
		"	sd.criticalResistAmount, "
		"	sd.criticalBlockAmount, "
		"	sd.glancingCount, "
		"	sd.glancingDamageAmount, "
		"	sd.glancingOverkillAmount, "
		"	sd.glancingResistAmount, "
		"	sd.glancingBlockAmount, "
		"	sd.crushingCount, "
		"	sd.crushingDamageAmount, "
		"	sd.crushingOverkillAmount, "
		"	sd.crushingResistAmount, "
		"	sd.crushingBlockAmount, "
		"	cs.elapsedTimeSeconds "
		"from "
		"	cprSummary cs, "
		"	cprSummaryDamage sd "
		"where "
		"	cs.uid = sd.summaryId and "
		"	cs.logHistoryId = %i and "
		"	cs.realmId = %i and "
		"	cs.type = %i and "
		"	cs.sourceId = %i and "
		"	cs.targetId = %i and "
		"	sd.spellId = %i and "
		"	sd.spellRank = %i and "
		"	sd.damageType = '%s'"
		";";

	if((pgdb == NULL) || (logHistoryId < 1) || (realmId < 1) ||
			(summary == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(wowstats_wsdalGetSummaryTypeId(pgdb,
				WOW_SUMMARY_LIST[(summary->summaryId - 1)].name,
				&summaryTypeId) < 0) {
		return -1;
	}

	if(wowstats_wsdalGetEntityId(pgdb, realmId, summary->sourceUid,
				&sourceId) < 0) {
		return -1;
	}

	if(wowstats_wsdalGetEntityId(pgdb, realmId, summary->targetUid,
				&targetId) < 0) {
		return -1;
	}

	record = (WowSummaryDamage *)summary->record;

	snprintf(sql, (sizeof(sql) - 1), template,
			logHistoryId,
			realmId,
			summaryTypeId,
			sourceId,
			targetId,
			record->spellId,
			record->spellRank,
			record->damageType);

	if(pgdb_executeSql(pgdb, sql, PGRES_TUPLES_OK, &handle) < 0) {
		return -1;
	}

	if(pgdb_getRowCount(pgdb, handle) != 1) {
		pgdb_freeHandle(pgdb, handle);
		return -1;
	}

	// check spell name

	result = pgdb_getRowResult(pgdb, handle, 0, 0, &resultLength);
	if((result == NULL) || (resultLength < 1)) {
		if(result != NULL) {
			free(result);
		}
		pgdb_freeHandle(pgdb, handle);
		return -1;
	}

	if(strcmp(record->spellName, result)) {
		free(result);
		pgdb_freeHandle(pgdb, handle);
		return -1;
	}

	free(result);

	// check spell school

	result = pgdb_getRowResult(pgdb, handle, 0, 1, &resultLength);
	if((result == NULL) || (resultLength < 1)) {
		if(result != NULL) {
			free(result);
		}
		pgdb_freeHandle(pgdb, handle);
		return -1;
	}

	if(strcmp(record->spellSchool, result)) {
		free(result);
		pgdb_freeHandle(pgdb, handle);
		return -1;
	}

	free(result);

	// obtain summary record values

	record->directCount = getRowInteger(pgdb, handle, 0, 2);
	record->periodicCount = getRowInteger(pgdb, handle, 0, 3);
	record->damageAmount = getRowInteger(pgdb, handle, 0, 4);
	record->overkillAmount = getRowInteger(pgdb, handle, 0, 5);
	record->resistAmount = getRowInteger(pgdb, handle, 0, 6);
	record->blockAmount = getRowInteger(pgdb, handle, 0, 7);
	record->missedCount = getRowInteger(pgdb, handle, 0, 8);
	record->missedAmount = getRowInteger(pgdb, handle, 0, 9);
	record->absorbedCount = getRowInteger(pgdb, handle, 0, 10);
	record->absorbedDamageAmount = getRowInteger(pgdb, handle, 0, 11);
	record->absorbedOverkillAmount = getRowInteger(pgdb, handle, 0, 12);
	record->absorbedResistAmount = getRowInteger(pgdb, handle, 0, 13);
	record->absorbedBlockAmount = getRowInteger(pgdb, handle, 0, 14);
	record->criticalCount = getRowInteger(pgdb, handle, 0, 15);
	record->criticalDamageAmount = getRowInteger(pgdb, handle, 0, 16);
	record->criticalOverkillAmount = getRowInteger(pgdb, handle, 0, 17);
	record->criticalResistAmount = getRowInteger(pgdb, handle, 0, 18);
	record->criticalBlockAmount = getRowInteger(pgdb, handle, 0, 19);
	record->glancingCount = getRowInteger(pgdb, handle, 0, 20);
	record->glancingDamageAmount = getRowInteger(pgdb, handle, 0, 21);
	record->glancingOverkillAmount = getRowInteger(pgdb, handle, 0, 22);
	record->glancingResistAmount = getRowInteger(pgdb, handle, 0, 23);
	record->glancingBlockAmount = getRowInteger(pgdb, handle, 0, 24);
	record->crushingCount = getRowInteger(pgdb, handle, 0, 25);
	record->crushingDamageAmount = getRowInteger(pgdb, handle, 0, 26);
	record->crushingOverkillAmount = getRowInteger(pgdb, handle, 0, 27);
	record->crushingResistAmount = getRowInteger(pgdb, handle, 0, 28);
	record->crushingBlockAmount = getRowInteger(pgdb, handle, 0, 29);

	summary->extraElapsedTimeSeconds = getRowInteger(pgdb, handle, 0, 30);

	if(pgdb_freeHandle(pgdb, handle) < 0) {
		return -1;
	}

	return 0;
}

