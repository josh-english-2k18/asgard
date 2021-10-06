/*
 * wowcpr.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard-WoW statistical analysis system application.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#include "wowstats/common.h"
#define _WOWSTATS_DATA_COMPONENT
#include "wowstats/data/playerclass.h"
#include "wowstats/core.h"
#include "wowstats/db.h"
#include "wowstats/system.h"

// define wowcpr constants

#define WOWCPR_ANALYZE_MODE					atrue

#define WOWCPR_CONFIG_FILENAME				"conf/wowcpr.config"

#define MERGE_INDEX_KEY_HASH_LENGTH			40

// define wowcpr data types

typedef struct _WowCprDbProperties {
	char *hostname;
	int port;
	char *username;
	char *password;
	char *connectionFlags;
	char *name;
} WowCprDbProperties;

typedef struct _WowCprConfig {
	char *fileSearchPath;
	char *duplicateIndexPath;
	char *metaFileExtension;
	int defaultYear;
	WowCprDbProperties db;
} WowCprConfig;

typedef struct _WowMetaFile {
	char *ownerEmailAddress;
	char *regionCode;
	char *realmName;
	int fileYear;
	char *fileType;
	char *filename;
	char *description;
} WowMetaFile;

typedef struct _WowCprStats {
	/*
	 * file stats
	 */

	alint fileLineCount;
	alint fileLineErrorCount;
	alint fileLength;
	double fileProcessingTime;

	/*
	 * events stats
	 */

	alint eventValidCount;
	alint eventInvalidCount;
	alint eventUnknownCount;
	alint eventDuplicateCount;
	double eventParsingTime;
	double eventProcessingTime;
	double eventKeyHashTime;
	double eventMergeIndexTime;

	/*
	 * player stats
	 */

	alint playerValidEventsCount;
	alint playerInvalidEventsCount;
	int playerSummaryCount;
	int playerSummaryDatabaseExistsCount;
	int playerSummaryDatabaseUpdateCount;
	double linkIndexUpdateTime;
	double playerIndexUpdateTime;
	double playerDatabaseAccessTime;

	/*
	 * summary stats
	 */

	int summaryRecordCount;
	int summaryDatabaseMergeCount;
	int summaryDatabaseUpdateCount;
	alint summaryValidEventsCount;
	alint summaryInvalidEventsCount;
	alint summaryNotApplicableEventsCount;
	alint summaryIndexUpdateCount;
	alint summaryIndexUpdateFailureCount;
	double summaryConstructTime;
	double summaryIndexUpdateTime;
	double summaryDatabaseUpdateTime;
} WowCprStats;

typedef struct _WowMetaData {
	int userId;
	int logHistoryId;
	int realmId;
	WowTimestamp startLogTimestamp;
	WowTimestamp endLogTimestamp;
} WowMetaData;

typedef struct _ContributingLogEntry {
	int realmId;
	int logHistoryId;
} ContributingLogEntry;

typedef struct _WowTriggerData {
	aboolean hasTrigger;
	int ref;
	int length;
	WowTrigger trigger;
	WowTriggerResult *triggerResults;
	WowPlayerIndex *playerIndexes;
	WowSummaryIndex *summaryIndexes;
} WowTriggerData;

typedef struct _WowCpr {
	WowSpellindex spellIndex;
	WowLinkIndex linkIndex;
	WowPlayerIndex playerIndex;
	WowSummaryIndex summaryIndex;
	Bdb bdb;
	Pgdb pgdb;
} WowCpr;


// define wowcpr global variables

static WowCprConfig cprConfig;
static WowMetaFile metaFile;
static WowCprStats cprStats;
static WowMetaData metaData;
static WowTriggerData triggerData;
static WowCpr wowcpr;


// delcare wowcpr functions

// configuration functions

static int loadConfigFile();

static char *buildMetaFilename(char *filename);

static int loadMetaFile(char *filename);

// database functions

static int initDatabase();

static int freeDatabase();

static int getMetaData();

static int updateLogHistory();

static int updatePlayers();

static void buildContributingLogKey(int realmId, int logHistoryId,
		char key[32]);

static ContributingLogEntry *newContributingLogEntry(int realmId,
		int logHistoryId);

static int updateContributingLogs(Bptree *contributingLogs);

static int updateSummaries();

// index functions

static char *buildIndexFilename();

static int initIndex();

static int freeIndex();

static void buildKeyHash(char *key, int keyLength,
		char keyHash[MERGE_INDEX_KEY_HASH_LENGTH]);

static int popIndexValue(char *value, int valueLength, int id);

static aboolean isKeyInIndex(char *key, int keyLength, int *logHistoryId,
		int *realmId);

static int addKeyToIndex(char *key, int keyLength);

// player functions

static char *classIdToName(int classId);

// trigger functions

static void initTrigger();

static void freeTrigger();

static int newTrigger();

static int updateTriggerSummaries();

// summary functions

static int initContext();

static int freeContext();

static void displayProcessingReport();

static int summarizeWowLogfile(char *filename);


// main function

int main(int argc, char *argv[])
{
	aboolean fileExists = afalse;
	double timer = 0.0;
	char *filename = NULL;

	signal_registerDefault();
	timer = time_getTimeMus();

	printf("WoW Combat Performance Report Application Ver %s on %s.\n\n",
			ASGARD_VERSION, ASGARD_DATE);

	if(argc >= 2) {
		filename = argv[1];
	}
	else {
		fprintf(stderr, "error - invalid or missing argument(s).\n");
		fprintf(stderr, "usage: wowcpr [filename]\n");
		return 1;
	}

	printf("File '%s' exists...", filename);
	fflush(NULL);

	system_fileExists(filename, &fileExists);
	if(!fileExists) {
		printf("FAILED\n");
		fprintf(stderr, "error - unable to locate file '%s'.\n", filename);
		return 1;
	}

	printf("OK\n");
	printf("Loading configuration file...");
	fflush(NULL);

	if(loadConfigFile() < 0) {
		printf("FAILED\n");
		return 1;
	}

	printf("OK\n");
	printf("Loading meta-data file...");
	fflush(NULL);

	if(loadMetaFile(filename) < 0) {
		printf("FAILED\n");
		return 1;
	}

	printf("OK\n");

	if(summarizeWowLogfile(filename) < 0) {
		return 1;
	}

	printf("\nProcessing complete, total run time: %0.6f seconds\n",
			time_getElapsedMusInSeconds(timer));

	return 0;
}


// define wowcpr functions

// configuration functions

static int loadConfigFile()
{
	aboolean fileExists = afalse;
	int iValue = 0;
	char *sValue = NULL;

	Config config;

	system_fileExists(WOWCPR_CONFIG_FILENAME, &fileExists);
	if(!fileExists) {
		fprintf(stderr, "error - unable to locate configuration file '%s'.\n",
				WOWCPR_CONFIG_FILENAME);
		return -1;
	}

	if(config_init(&config, WOWCPR_CONFIG_FILENAME) < 0) {
		return -1;
	}

	memset((void *)&cprConfig, 0, sizeof(WowCprConfig));

	// get general properties

	if(config_getString(&config, "general", "fileSearchPath",
				"assets/data/wowcprInput", &sValue) < 0) {
		fprintf(stderr, "error - failed to locate property "
				"'general.fileSearchPath'.\n");
		return -1;
	}

	cprConfig.fileSearchPath = strdup(sValue);

	if(config_getString(&config, "general", "duplicateIndexPath",
				"assets/data/wowcprIndex", &sValue) < 0) {
		fprintf(stderr, "error - failed to locate property "
				"'general.duplicateIndexPath'.\n");
		return -1;
	}

	cprConfig.duplicateIndexPath = strdup(sValue);

	if(config_getString(&config, "general", "metaFileExtension",
				"meta", &sValue) < 0) {
		fprintf(stderr, "error - failed to locate property "
				"'general.metaFileExtension'.\n");
		return -1;
	}

	cprConfig.metaFileExtension = strdup(sValue);

	if(config_getInteger(&config, "general", "defaultYear", 2009,
				&iValue) < 0) {
		fprintf(stderr, "error - failed to locate property "
				"'general.defaultYear'.\n");
		return -1;
	}

	cprConfig.defaultYear = iValue;

	// get data source properties

	if(config_getString(&config, "dataSourceProperties", "dbHostname",
				"assets/data/wowcprInput", &sValue) < 0) {
		fprintf(stderr, "error - failed to locate property "
				"'dataSourceProperties.dbHostname'.\n");
		return -1;
	}

	cprConfig.db.hostname = strdup(sValue);

	if(config_getInteger(&config, "dataSourceProperties", "dbPort",
				0, &iValue) < 0) {
		fprintf(stderr, "error - failed to locate property "
				"'dataSourceProperties.dbPort'.\n");
		return -1;
	}

	cprConfig.db.port = iValue;

	if(config_getString(&config, "dataSourceProperties", "dbUsername",
				"assets/data/wowcprInput", &sValue) < 0) {
		fprintf(stderr, "error - failed to locate property "
				"'dataSourceProperties.dbUsername'.\n");
		return -1;
	}

	cprConfig.db.username = strdup(sValue);

	if(config_getString(&config, "dataSourceProperties", "dbPassword",
				"assets/data/wowcprInput", &sValue) < 0) {
		fprintf(stderr, "error - failed to locate property "
				"'dataSourceProperties.dbPassword'.\n");
		return -1;
	}

	cprConfig.db.password = strdup(sValue);

	if(config_getString(&config, "dataSourceProperties", "dbConnectionFlags",
				"", &sValue) < 0) {
		fprintf(stderr, "error - failed to locate property "
				"'dataSourceProperties.dbConnectionFlags'.\n");
		return -1;
	}

	if(strcmp(sValue, "")) {
		cprConfig.db.connectionFlags = NULL;
	}
	else {
		cprConfig.db.connectionFlags = strdup(sValue);
	}

	if(config_getString(&config, "dataSourceProperties", "dbName",
				"assets/data/wowcprInput", &sValue) < 0) {
		fprintf(stderr, "error - failed to locate property "
				"'dataSourceProperties.dbName'.\n");
		return -1;
	}

	cprConfig.db.name = strdup(sValue);

	// cleanup

	if(config_free(&config) < 0) {
		return -1;
	}

	return 0;
}

static char *buildMetaFilename(char *filename)
{
	int length = 0;
	char *result = NULL;

	length = (strlen(filename) + strlen(cprConfig.metaFileExtension) + 8);
	result = (char *)malloc(sizeof(char) * (length + 1));

	snprintf(result, length, "%s.%s", filename, cprConfig.metaFileExtension);

	return result;
}

static int loadMetaFile(char *filename)
{
	aboolean fileExists = afalse;
	int iValue = 0;
	char buffer[8192];
	char *sValue = NULL;
	char *metaFilename = NULL;

	Config config;

	metaFilename = buildMetaFilename(filename);

	system_fileExists(metaFilename, &fileExists);
	if(!fileExists) {
		fprintf(stderr, "error - unable to locate configuration file '%s'.\n",
				metaFilename);
		free(metaFilename);
		return -1;
	}

	if(config_init(&config, metaFilename) < 0) {
		free(metaFilename);
		return -1;
	}

	free(metaFilename);

	memset((void *)&metaFile, 0, sizeof(WowMetaFile));

	// get general properties

	if(config_getString(&config, "general", "ownerEmailAddress",
				"unknown@unknown.com", &sValue) < 0) {
		fprintf(stderr, "error - failed to locate property "
				"'general.ownerEmailAddress'.\n");
		return -1;
	}

	metaFile.ownerEmailAddress = strdup(sValue);

	if(config_getString(&config, "general", "regionCode",
				"US", &sValue) < 0) {
		fprintf(stderr, "error - failed to locate property "
				"'general.regionCode'.\n");
		return -1;
	}

	metaFile.regionCode = strdup(sValue);

	if(config_getString(&config, "general", "realmName",
				"unknown", &sValue) < 0) {
		fprintf(stderr, "error - failed to locate property "
				"'general.realmName'.\n");
		return -1;
	}

	metaFile.realmName = strdup(sValue);

	if(config_getInteger(&config, "general", "fileYear",
				cprConfig.defaultYear, &iValue) < 0) {
		fprintf(stderr, "error - failed to locate property "
				"'general.fileYear'.\n");
		return -1;
	}

	metaFile.fileYear = iValue;

	if(config_getString(&config, "general", "fileType",
				"Raid", &sValue) < 0) {
		fprintf(stderr, "error - failed to locate property "
				"'general.fileType'.\n");
		return -1;
	}

	metaFile.fileType = strdup(sValue);

	if(config_getString(&config, "general", "filename",
				"unknown", &sValue) < 0) {
		fprintf(stderr, "error - failed to locate property "
				"'general.filename'.\n");
		return -1;
	}

	metaFile.filename = strdup(sValue);

	if(config_getString(&config, "general", "fileDescription",
				"unknown", &sValue) < 0) {
		fprintf(stderr, "error - failed to locate property "
				"'general.fileDescription'.\n");
		return -1;
	}

	metaFile.description = strdup(sValue);

	// cleanup

	if(config_free(&config) < 0) {
		return -1;
	}

	// test configuration attributes

	snprintf(buffer, (sizeof(buffer) - 1), "%s/%s", cprConfig.fileSearchPath,
			metaFile.filename);

	if(strcmp(filename, buffer)) {
		fprintf(stderr, "error - meta-data filename mismatch with '%s' vs "
				"'%s'.\n", filename, buffer);
		return -1;
	}

	return 0;
}

// database functions

static int initDatabase()
{
	if(pgdb_init(&wowcpr.pgdb) < 0) {
		return -1;
	}

	if(pgdb_setDataSourceProperties(&wowcpr.pgdb,
				cprConfig.db.hostname,
				cprConfig.db.port,
				cprConfig.db.username,
				cprConfig.db.password,
				cprConfig.db.connectionFlags,
				cprConfig.db.name) < 0) {
		return -1;
	}

	if(pgdb_connect(&wowcpr.pgdb) < 0) {
		return -1;
	}

	return 0;
}

static int freeDatabase()
{
	if(pgdb_disconnect(&wowcpr.pgdb) < 0) {
		return -1;
	}

	if(pgdb_free(&wowcpr.pgdb) < 0) {
		return -1;
	}

	return 0;
}

static int getMetaData()
{
	memset((void *)&metaData, 0, sizeof(WowMetaData));

	if(wowstats_timestampInit(&metaData.startLogTimestamp,
				metaFile.fileYear) < 0) {
		return -1;
	}

	if(wowstats_timestampInit(&metaData.endLogTimestamp,
				metaFile.fileYear) < 0) {
		return -1;
	}

	if(WOWCPR_ANALYZE_MODE) {
		printf("\n\n");
		printf("[Analyze] obtained user and realm IDs\n");
		printf("\n");
		metaData.userId = 0;
		metaData.realmId = 0;
		return 0;
	}

	if(wowstats_wsdalGetUserId(&wowcpr.pgdb, metaFile.ownerEmailAddress,
				&metaData.userId) < 0) {
		return -1;
	}

	if(wowstats_wsdalGetRealmId(&wowcpr.pgdb, metaFile.regionCode,
				metaFile.realmName, &metaData.realmId) < 0) {
		return -1;
	}

	return 0;
}

static int updateLogHistory()
{
	char timestampString[32];

	if(wowstats_timestampToString(&metaData.startLogTimestamp,
				timestampString) < 0) {
		return -1;
	}

	if(WOWCPR_ANALYZE_MODE) {
		printf("[Analyze] created main log history ID\n");
		printf("\n");
		metaData.logHistoryId = 0;
		return 0;
	}

	if(wowstats_wsdalAddLogHistory(&wowcpr.pgdb, metaData.userId,
				metaData.realmId, metaFile.fileType, metaFile.filename,
				metaFile.description, 1, timestampString) < 0) {
		return -1;
	}

	if(wowstats_wsdalGetLastLogHistoryId(&wowcpr.pgdb,
				&metaData.logHistoryId) < 0) {
		return -1;
	}

	return 0;
}

static int updatePlayers()
{
	int ii = 0;
	int entityId = 0;
	int secondsOnline = 0;
	double timer = 0.0;

	WowPlayerSummary **playerSummaries = NULL;

	if(wowstats_playerIndexGetSummaries(&wowcpr.playerIndex, &playerSummaries,
				&cprStats.playerSummaryCount) < 0) {
		return -1;
	}

	if(WOWCPR_ANALYZE_MODE) {
		printf("\n\n");
		printf("[Analyze] main player index entries: %i\n",
				cprStats.playerSummaryCount);
		printf("\n");
		return 0;
	}

	for(ii = 0; ii < cprStats.playerSummaryCount; ii++) {
		timer = time_getTimeMus();

		if(wowstats_wsdalGetEntityId(&wowcpr.pgdb,
					metaData.realmId, // realm id
					playerSummaries[ii]->uid, // uid
					&entityId // entity id
					) < 0) {
			return -1;
		}

		if(wowstats_timestampCalculateDifferenceInSeconds(
					&playerSummaries[ii]->lastTimestamp,
					&playerSummaries[ii]->firstTimestamp,
					&secondsOnline) < 0) {
			return -1;
		}

		if(entityId == -1) {
			if(wowstats_wsdalAddEntity(&wowcpr.pgdb,
						metaData.realmId, // realm id
						classIdToName(playerSummaries[ii]->classId), // class
						playerSummaries[ii]->name, // name
						playerSummaries[ii]->uid, // uid
						secondsOnline // seconds online
						) < 0) {
				return -1;
			}

			if(wowstats_wsdalGetEntityId(&wowcpr.pgdb,
						metaData.realmId, // realm id
						playerSummaries[ii]->uid, // uid
						&entityId // entity id
						) < 0) {
				return -1;
			}

			cprStats.playerSummaryDatabaseUpdateCount += 1;
		}
		else {
			if(wowstats_wsdalUpdateEntity(&wowcpr.pgdb,
						entityId, // entity id
						classIdToName(playerSummaries[ii]->classId), // class
						secondsOnline // seconds online
						) < 0) {
				return -1;
			}

			cprStats.playerSummaryDatabaseExistsCount += 1;
		}

		if(wowstats_wsdalAddEntityLog(&wowcpr.pgdb,
					entityId, // entity id
					metaData.logHistoryId, // log history id
					secondsOnline // total played seconds
					) < 0) {
			return -1;
		}

		cprStats.playerDatabaseAccessTime += time_getElapsedMusInSeconds(timer);
	}

	return 0;
}

static void buildContributingLogKey(int realmId, int logHistoryId,
		char key[32])
{
	snprintf(key, 32, "/%i/%i", realmId, logHistoryId);
}

static ContributingLogEntry *newContributingLogEntry(int realmId,
		int logHistoryId)
{
	ContributingLogEntry *result = NULL;

	result = (ContributingLogEntry *)malloc(sizeof(ContributingLogEntry));

	result->realmId = realmId;
	result->logHistoryId = logHistoryId;

	return result;
}

static int updateContributingLogs(Bptree *contributingLogs)
{
	int ii = 0;
	int leafCount = 0;
	int arrayLength = 0;
	int *keyLengths = 0;
	char timestampString[32];
	char **keys = NULL;
	void **values = NULL;

	ContributingLogEntry *entry = NULL;

	if(bptree_getLeafCount(contributingLogs, &leafCount) < 0) {
		return -1;
	}

	if(leafCount < 1) {
		return 0;
	}

	if(bptree_toArray(contributingLogs, &arrayLength, &keyLengths, &keys,
				&values) < 0) {
		return -1;
	}

	if(wowstats_timestampToString(&metaData.startLogTimestamp,
				timestampString) < 0) {
		return -1;
	}

	for(ii = 0; ii < arrayLength; ii++) {
		entry = (ContributingLogEntry *)values[ii];

		if(WOWCPR_ANALYZE_MODE) {
			printf("[Analyze] added contributing log\n");
			continue;
		}

		if(wowstats_wsdalAddContributingLog(&wowcpr.pgdb,
					metaData.logHistoryId, metaData.userId, entry->realmId,
					metaFile.fileType, metaFile.filename,
					metaFile.description, timestampString) < 0) {
			return -1;
		}
	}

	return 0;
}

static int updateSummaries()
{
	int ii = 0;
	int nn = 0;
	int playSeconds = 0;
	int summaryListLength = 0;
	int summaryRecordListLength = 0;
	double timer = 0.0;
	char contributingLogKey[32];
	char **summaryList = NULL;
	char **summaryRecordList = NULL;

	ContributingLogEntry *contributingLogEntry = NULL;
	Bptree contributingLogs;

	WowSummaryIndexEntry *entry = NULL;
	WowSummary *summary = NULL;

	if(wowstats_timestampCalculateDifferenceInSeconds(
				&metaData.endLogTimestamp,
				&metaData.startLogTimestamp,
				&playSeconds) < 0) {
		return -1;
	}

	if(WOWCPR_ANALYZE_MODE) {
		printf("\n\n");
		printf("[Analyze] log history play seconds: %i\n", playSeconds);
	}
	else {
		if(wowstats_wsdalSetLogHistoryPlaySeconds(&wowcpr.pgdb,
					metaData.logHistoryId,
					playSeconds) < 0) {
			return -1;
		}
	}

	if(bptree_init(&contributingLogs) < 0) {
		return -1;
	}

	if(wowstats_summaryIndexListSummaries(&wowcpr.summaryIndex, &summaryList,
				&summaryListLength) < 0) {
		return -1;
	}

	if(WOWCPR_ANALYZE_MODE) {
		printf("[Analyze] summary list length: %i\n", summaryListLength);
		printf("\n");
	}

	for(ii = 0; ((ii < summaryListLength) && (!WOWCPR_ANALYZE_MODE)); ii++) {
		if(wowstats_summaryIndexListSummaryRecords(&wowcpr.summaryIndex,
					summaryList[ii], &summaryRecordList,
					&summaryRecordListLength) < 0) {
			return -1;
		}

		for(nn = 0; nn < summaryRecordListLength; nn++) {
			if(wowstats_summaryIndexGetEntry(&wowcpr.summaryIndex,
						summaryList[ii], summaryRecordList[nn], &entry) < 0) {
				return -1;
			}

			cprStats.summaryRecordCount += 1;

			summary = entry->summary;

//			wowstats_summaryDisplay(summary);

			if((entry->isParentRecord) && (entry->wasUpdated)) {
				buildContributingLogKey(entry->realmId, entry->logHistoryId,
						contributingLogKey);

				if(bptree_get(&contributingLogs, contributingLogKey,
							strlen(contributingLogKey),
							(void *)&contributingLogEntry) < 0) {
					contributingLogEntry = newContributingLogEntry(
							entry->realmId, entry->logHistoryId);
					if(bptree_put(&contributingLogs, contributingLogKey,
								strlen(contributingLogKey),
								(void *)contributingLogEntry) < 0) {
						return -1;
					}
				}

				timer = time_getTimeMus();

				switch(summary->summaryId) {
					case WOW_SUMMARY_HEALING:
						if(wowstats_wsdalUpdateSummaryHealing(
									&wowcpr.pgdb,
									entry->logHistoryId, // log history id
									entry->realmId, // realm id
									summary) < 0) {
							return -1;
						}
						break;
					case WOW_SUMMARY_DAMAGE:
						if(wowstats_wsdalUpdateSummaryDamage(
									&wowcpr.pgdb,
									entry->logHistoryId, // log history id
									entry->realmId, // realm id
									summary) < 0) {
							return -1;
						}
						break;
					default:
						break;
				}

				cprStats.summaryDatabaseMergeCount += 1;
				cprStats.summaryDatabaseUpdateCount += 1;
				cprStats.summaryDatabaseUpdateTime +=
					time_getElapsedMusInSeconds(timer);
			}
			else if(!entry->isParentRecord) {
				timer = time_getTimeMus();

				switch(summary->summaryId) {
					case WOW_SUMMARY_HEALING:
						if(wowstats_wsdalAddSummaryHealing(&wowcpr.pgdb,
									metaData.logHistoryId, // log history id
									metaData.realmId, // realm id
									summary) < 0) {
							return -1;
						}
						break;
					case WOW_SUMMARY_DAMAGE:
						if(wowstats_wsdalAddSummaryDamage(&wowcpr.pgdb,
									metaData.logHistoryId, // log history id
									metaData.realmId, // realm id
									summary) < 0) {
							return -1;
						}
						break;
					default:
						break;
				}

				cprStats.summaryDatabaseUpdateCount += 1;
				cprStats.summaryDatabaseUpdateTime +=
					time_getElapsedMusInSeconds(timer);
			}

			free(summaryRecordList[nn]);
		}

		if(summaryRecordList != NULL) {
			free(summaryRecordList);
		}

		free(summaryList[ii]);
	}

	free(summaryList);

	if(updateContributingLogs(&contributingLogs) < 0) {
		return -1;
	}

	bptree_free(&contributingLogs);

	return 0;
}

// index functions

static char *buildIndexFilename()
{
	int ii = 0;
	int length = 0;
	char *result = NULL;

	length = (strlen(cprConfig.duplicateIndexPath) +
			strlen(metaFile.regionCode) +
			strlen(metaFile.realmName) +
			32);
	result = (char *)malloc(sizeof(char) * (length + 1));

	snprintf(result, length, "%s/wci.%s.%s.db", cprConfig.duplicateIndexPath,
			metaFile.regionCode, metaFile.realmName);

	length = strlen(result);
	for(ii = 0; ii < length; ii++) {
		if(result[ii] == ' ') {
			result[ii] = '_';
		}
	}

	return result;
}

static int initIndex()
{
	char *databaseName = NULL;

	databaseName = buildIndexFilename();

	if(bdb_init(&wowcpr.bdb, databaseName) < 0) {
		return -1;
	}

	free(databaseName);

	return 0;
}

static int freeIndex()
{
	if(bdb_free(&wowcpr.bdb) < 0) {
		return -1;
	}

	return 0;
}

static void buildKeyHash(char *key, int keyLength,
		char keyHash[MERGE_INDEX_KEY_HASH_LENGTH])
{
/*
 * SHA-1 hashing of the key
 *
	Sha1 hash;

	sha1_init(&hash);
	sha1_hashData(&hash, (unsigned char *)key, keyLength);
	sha1_calculateHash(&hash);
	sha1_constructSignatureStatic(&hash, keyHash);
 */
/*
 * CRC-64 hashing of the key
 *
	int ii = 0;
	int middle = 0;
	char buffer[8];
	char *ptr = NULL;
	unsigned char digest[8];

	union {
		unsigned int i_value;
		unsigned char c_value[(sizeof(unsigned int))];
	} iu;

	union {
		unsigned long long int lli_value;
		unsigned char c_value[(sizeof(unsigned long long int))];
	} lliu;

	iu.i_value = 0;
	lliu.lli_value = 0;

	middle = (keyLength / 2);
	ptr = key;

	iu.i_value = crc32_calculateHash(ptr, middle);
	digest[0] = iu.c_value[0];
	digest[2] = iu.c_value[1];
	digest[4] = iu.c_value[2];
	digest[6] = iu.c_value[3];

	ptr += middle;
	iu.i_value = crc32_calculateHash(ptr, (keyLength - (int)(ptr - key)));
	digest[1] = iu.c_value[0];
	digest[3] = iu.c_value[1];
	digest[5] = iu.c_value[2];
	digest[7] = iu.c_value[3];

	memset(keyHash, 0, 40);

	for(ii = 0; ii < 8; ii++) {
		snprintf(buffer, sizeof(buffer), "%02x", digest[ii]);
		keyHash[((ii * 2) + 0)] = buffer[0];
		keyHash[((ii * 2) + 1)] = buffer[1];
	}
 */
}

static int popIndexValue(char *value, int valueLength, int id)
{
	int ii = 0;
	int nn = 0;
	int ref = 0;
	char buffer[1024];

	memset(buffer, 0, sizeof(buffer));

	for(ii = 0, nn = 0; ii < valueLength; ii++) {
		if(value[ii] == '|') {
			ref++;
		}
		else {
			if(ref > id) {
				break;
			}
			if(ref == id) {
				buffer[nn] = value[ii];
				nn++;
			}
		}
	}

	return atoi(buffer);
}

static aboolean isKeyInIndex(char *key, int keyLength, int *logHistoryId,
		int *realmId)
{
	int valueLength = 0;
	char *value = NULL;

	if(bdb_get(&wowcpr.bdb, key, keyLength, &value, &valueLength) == 0) {
		*logHistoryId = popIndexValue(value, valueLength, 0);
		*realmId = popIndexValue(value, valueLength, 1);
		return atrue;
	}

	return afalse;
}

static int addKeyToIndex(char *key, int keyLength)
{
	int valueLength = 0;
	char value[1024];

	snprintf(value, (sizeof(value) - 1), "%i|%i|", metaData.logHistoryId,
			metaData.realmId);
	valueLength = strlen(value);

	if(bdb_put(&wowcpr.bdb, key, keyLength, value, valueLength) < 0) {
		return -1;
	}

	return 0;
}

// player functions

static char *classIdToName(int classId)
{
	int ii = 0;

	for(ii = 0;
			WOW_PLAYER_CLASS_LIST[ii].id != WOW_PLAYER_CLASS_END_OF_LIST;
			ii++) {
		if(WOW_PLAYER_CLASS_LIST[ii].id == classId) {
			return WOW_PLAYER_CLASS_LIST[ii].name;
		}
	}

	return "Unknown";
}

// trigger functions

static void initTrigger()
{
	memset((void *)&triggerData, 0, sizeof(WowTriggerData));

	triggerData.hasTrigger = afalse;
	triggerData.ref = 0;
	triggerData.length = 0;
	triggerData.triggerResults = NULL;
	triggerData.playerIndexes = NULL;
	triggerData.summaryIndexes = NULL;

	wowstats_triggerInit(&triggerData.trigger);

	if(WOWCPR_ANALYZE_MODE) {
		triggerData.trigger.analyzeMode = atrue;
	}
}

static void freeTrigger()
{
	/*
	 * TODO: this
	 */

	wowstats_triggerFree(&triggerData.trigger);
}

static int newTrigger()
{
	if(triggerData.length == 0) {
		triggerData.ref = 0;
		triggerData.length = 1;
		triggerData.triggerResults = (WowTriggerResult *)malloc(
				sizeof(WowTriggerResult) * triggerData.length);
		triggerData.playerIndexes = (WowPlayerIndex *)malloc(
				sizeof(WowPlayerIndex) * triggerData.length);
		triggerData.summaryIndexes = (WowSummaryIndex *)malloc(
				sizeof(WowSummaryIndex) * triggerData.length);
	}
	else {
		triggerData.ref = triggerData.length;
		triggerData.length += 1;
		triggerData.triggerResults = (WowTriggerResult *)realloc(
				triggerData.triggerResults,
				(sizeof(WowTriggerResult) * triggerData.length));
		triggerData.playerIndexes = (WowPlayerIndex *)realloc(
				triggerData.playerIndexes,
				(sizeof(WowPlayerIndex) * triggerData.length));
		triggerData.summaryIndexes = (WowSummaryIndex *)realloc(
				triggerData.summaryIndexes,
				(sizeof(WowSummaryIndex) * triggerData.length));
	}

	triggerData.triggerResults[triggerData.ref].zone = NULL;
	triggerData.triggerResults[triggerData.ref].bossName = NULL;

	if(wowstats_playerIndexInit(&triggerData.playerIndexes[triggerData.ref],
				&wowcpr.spellIndex) < 0) {
		return -1;
	}

	if(wowstats_summaryIndexInit(
				&triggerData.summaryIndexes[triggerData.ref],
				&wowcpr.linkIndex) < 0) {
		return -1;
	}

	return 0;
}

static int updateTriggerSummaries()
{
	int ii = 0;
	int nn = 0;
	int jj = 0;
	int mobLogId = 0;
	int entityId = 0;
	int playSeconds = 0;
	int secondsOnline = 0;
	int playerSummaryCount = 0;
	int summaryListLength = 0;
	int summaryRecordListLength = 0;
	double timer = 0.0;
	char timestampString[32];
	char **summaryList = NULL;
	char **summaryRecordList = NULL;

	WowPlayerSummary **playerSummaries = NULL;
	WowSummaryIndexEntry *entry = NULL;
	WowSummary *summary = NULL;

	if(WOWCPR_ANALYZE_MODE) {
		printf("\n\n");
	}

	for(ii = 0; ii < triggerData.length; ii++) {
		if((triggerData.triggerResults[ii].zone == NULL) ||
				(triggerData.triggerResults[ii].bossName == NULL)) {
			continue;
		}

		if(wowstats_timestampCalculateDifferenceInSeconds(
					&triggerData.triggerResults[ii].endTimestamp,
					&triggerData.triggerResults[ii].startTimestamp,
					&playSeconds) < 0) {
			return -1;
		}

		if(playSeconds < 30) {
			continue;
		}

		if(bptree_getLeafCount(&triggerData.summaryIndexes[ii].index,
					&summaryListLength) < 0) {
			return -1;
		}

		if(summaryListLength < 1) {
			continue;
		}

		if(wowstats_timestampToString(
					&triggerData.triggerResults[ii].startTimestamp,
					timestampString) < 0) {
			return -1;
		}

		if(WOWCPR_ANALYZE_MODE) {
			printf("[Analyze] created mob-log entry { boss '%s', zone '%s', "
					"play seconds %i, timestamp '%s' }\n",
					triggerData.triggerResults[ii].bossName,
					triggerData.triggerResults[ii].zone,
					playSeconds,
					timestampString);
		}
		else {
			if(wowstats_wsdalAddMobLog(&wowcpr.pgdb,
					metaData.logHistoryId,
					metaData.userId,
					metaData.realmId,
					"Mob Fight",
					triggerData.triggerResults[ii].bossName,
					triggerData.triggerResults[ii].zone,
					playSeconds,
					timestampString
					) < 0) {
				return -1;
			}

			if(wowstats_wsdalGetLastMobLogId(&wowcpr.pgdb,
						&mobLogId) < 0) {
				return -1;
			}
		}

		if(wowstats_playerIndexGetSummaries(
					&triggerData.playerIndexes[ii],
					&playerSummaries,
					&playerSummaryCount) < 0) {
			return -1;
		}

		cprStats.playerSummaryCount += playerSummaryCount;

		if(WOWCPR_ANALYZE_MODE) {
			printf("[Analyze] mob-log player index entries: %i\n",
					playerSummaryCount);
			printf("[Analyze] updated global player index entries: %i\n",
					cprStats.playerSummaryCount);
		}

		for(nn = 0; nn < playerSummaryCount; nn++) {
			if(WOWCPR_ANALYZE_MODE) {
				if(
						(
						 (playerSummaries[nn]->classId !=
						  WOW_PLAYER_CLASS_UNKNOWN) &&
						 (playerSummaries[nn]->classId !=
						  WOW_PLAYER_CLASS_NPC) &&
						 (playerSummaries[nn]->classId !=
						  WOW_PLAYER_CLASS_PET) &&
						 (playerSummaries[nn]->classId !=
						  WOW_PLAYER_CLASS_ENVIRONMENT)
						) ||
						(!strcmp(playerSummaries[nn]->name,
								 triggerData.triggerResults[ii].bossName))
					  ) {
					printf("[Analyze] player #%06i => '%s', name: %s\n", nn,
							playerSummaries[nn]->uid,
							playerSummaries[nn]->name);
				}
				continue;
			}

			timer = time_getTimeMus();

			if(wowstats_wsdalGetEntityId(&wowcpr.pgdb,
						metaData.realmId, // realm id
						playerSummaries[nn]->uid, // uid
						&entityId // entity id
						) < 0) {
				return -1;
			}

			if(entityId == -1) {
				continue;
			}

			if(wowstats_timestampCalculateDifferenceInSeconds(
						&playerSummaries[nn]->lastTimestamp,
						&playerSummaries[nn]->firstTimestamp,
						&secondsOnline) < 0) {
				return -1;
			}

			if(wowstats_wsdalAddEntityLog(&wowcpr.pgdb,
						entityId, // entity id
						mobLogId, // log history id
						secondsOnline // total played seconds
						) < 0) {
				return -1;
			}

			cprStats.playerDatabaseAccessTime +=
				time_getElapsedMusInSeconds(timer);
		}

		if(wowstats_summaryIndexListSummaries(
					&triggerData.summaryIndexes[ii],
					&summaryList,
					&summaryListLength) < 0) {
			return -1;
		}

		if(WOWCPR_ANALYZE_MODE) {
			printf("[Analyze] mob-log summary list length: %i\n",
					summaryListLength);
			continue;
		}

		if((summaryList == NULL) || (summaryListLength < 1)) {
			continue;
		}

		for(nn = 0; nn < summaryListLength; nn++) {
			if(wowstats_summaryIndexListSummaryRecords(
						&triggerData.summaryIndexes[ii],
						summaryList[nn],
						&summaryRecordList,
						&summaryRecordListLength) < 0) {
				return -1;
			}

			for(jj = 0; jj < summaryRecordListLength; jj++) {
				if(wowstats_summaryIndexGetEntry(
							&triggerData.summaryIndexes[ii],
							summaryList[nn],
							summaryRecordList[jj],
							&entry) < 0) {
					return -1;
				}

				cprStats.summaryRecordCount += 1;

				summary = entry->summary;

//				wowstats_summaryDisplay(summary);

				if(!entry->isParentRecord) {
					timer = time_getTimeMus();

					switch(summary->summaryId) {
						case WOW_SUMMARY_HEALING:
							if(wowstats_wsdalAddSummaryHealing(&wowcpr.pgdb,
										mobLogId, // log history id
										metaData.realmId, // realm id
										summary) < 0) {
								return -1;
							}
							break;
						case WOW_SUMMARY_DAMAGE:
							if(wowstats_wsdalAddSummaryDamage(&wowcpr.pgdb,
										mobLogId, // log history id
										metaData.realmId, // realm id
										summary) < 0) {
								return -1;
							}
							break;
						default:
							break;
					}

					cprStats.summaryDatabaseUpdateCount += 1;
					cprStats.summaryDatabaseUpdateTime +=
						time_getElapsedMusInSeconds(timer);
				}

				free(summaryRecordList[jj]);
			}

			if(summaryRecordList != NULL) {
				free(summaryRecordList);
			}

			free(summaryList[nn]);
		}

		free(summaryList);
	}

	if(WOWCPR_ANALYZE_MODE) {
		printf("\n");
	}

	return 0;
}

// summary functions

static int initContext()
{
	if(wowstats_spellindexInit(&wowcpr.spellIndex) < 0) {
		return -1;
	}

	if(wowstats_linkIndexInit(&wowcpr.linkIndex) < 0) {
		return -1;
	}

	if(wowstats_playerIndexInit(&wowcpr.playerIndex, &wowcpr.spellIndex) < 0) {
		return -1;
	}

	if(wowstats_summaryIndexInit(&wowcpr.summaryIndex,
				&wowcpr.linkIndex) < 0) {
		return -1;
	}

	memset((void *)&cprStats, 0, sizeof(WowCprStats));

	initTrigger();

	return 0;
}

static int freeContext()
{
	if(wowstats_spellindexFree(&wowcpr.spellIndex) < 0) {
		return -1;
	}

	if(wowstats_linkIndexFree(&wowcpr.linkIndex) < 0) {
		return -1;
	}

	if(wowstats_playerIndexFree(&wowcpr.playerIndex) < 0) {
		return -1;
	}

	if(wowstats_summaryIndexFree(&wowcpr.summaryIndex) < 0) {
		return -1;
	}

	freeTrigger();

	return 0;
}

static void displayProcessingReport()
{
	printf("\n");
	printf("-=-=-=-=-=-=-=-=- WoW CPR Processing Report -=-=-=-=-=-=-=-=-\n");

	printf("[CprReport] file statistics:\n");
	printf("[CprReport]\t name                   : %s\n",
			metaFile.filename);
	printf("[CprReport]\t owner                  : %s\n",
			metaFile.ownerEmailAddress);
	printf("[CprReport]\t region code            : %s\n",
			metaFile.regionCode);
	printf("[CprReport]\t realm name             : %s\n",
			metaFile.realmName);
	printf("[CprReport]\t realm ID               : %i\n",
			metaData.realmId);
	printf("[CprReport]\t event year             : %i\n",
			metaFile.fileYear);
	printf("[CprReport]\t event type             : %s\n",
			metaFile.fileType);
	printf("[CprReport]\t byte length            : %lli\n",
			cprStats.fileLength);
	printf("[CprReport]\t line count             : %lli\n",
			cprStats.fileLineCount);
	printf("[CprReport]\t invalid lines          : %lli (%0.2f %%)\n",
			cprStats.fileLineErrorCount,
			(((double)cprStats.fileLineErrorCount /
			  (double)cprStats.fileLineCount) * 100.0));
	printf("[CprReport]\t avg bytes per line     : %i\n",
			(int)((double)cprStats.fileLength /
				(double)cprStats.fileLineCount));
	printf("[CprReport]\t processing time        : %0.6f\n",
			cprStats.fileProcessingTime);

	printf("[CprReport] event statistics:\n");
	printf("[CprReport]\t valid count            : %lli (%0.2f %%)\n",
			cprStats.eventValidCount,
			(((double)cprStats.eventValidCount /
			  (double)cprStats.fileLineCount) * 100.0));
	printf("[CprReport]\t invalid count          : %lli (%0.2f %%)\n",
			cprStats.eventInvalidCount,
			(((double)cprStats.eventInvalidCount /
			  (double)cprStats.fileLineCount) * 100.0));
	printf("[CprReport]\t unknown count          : %lli (%0.2f %%)\n",
			cprStats.eventUnknownCount,
			(((double)cprStats.eventUnknownCount /
			  (double)cprStats.fileLineCount) * 100.0));
	printf("[CprReport]\t duplicate count        : %lli (%0.2f %%)\n",
			cprStats.eventDuplicateCount,
			(((double)cprStats.eventDuplicateCount /
			  (double)cprStats.eventValidCount) * 100.0));
	printf("[CprReport]\t total parsing time     : %0.6f\n",
			cprStats.eventParsingTime);
	printf("[CprReport]\t avg parsing time       : %0.6f\n",
			(cprStats.eventParsingTime /
			 (double)(cprStats.eventValidCount +
				 cprStats.eventInvalidCount +
				 cprStats.eventUnknownCount)));
	printf("[CprReport]\t total processing time  : %0.6f\n",
			cprStats.eventProcessingTime);
	printf("[CprReport]\t avg processing time    : %0.6f\n",
			(cprStats.eventProcessingTime /
			 (double)(cprStats.eventValidCount +
				 cprStats.eventInvalidCount +
				 cprStats.eventUnknownCount)));
	printf("[CprReport]\t key hash time          : %0.6f\n",
			cprStats.eventKeyHashTime);
	printf("[CprReport]\t avg key hash time      : %0.6f\n",
			(cprStats.eventKeyHashTime /
			 (double)(cprStats.eventValidCount +
				 cprStats.eventInvalidCount +
				 cprStats.eventUnknownCount)));
	printf("[CprReport]\t merge index time       : %0.6f\n",
			cprStats.eventMergeIndexTime);
	printf("[CprReport]\t avg merge index time   : %0.6f\n",
			(cprStats.eventMergeIndexTime /
			 (double)(cprStats.eventValidCount +
				 cprStats.eventInvalidCount +
				 cprStats.eventUnknownCount)));

	printf("[CprReport] player statistics:\n");
	printf("[CprReport]\t valid count            : %lli (%0.2f %%)\n",
			cprStats.playerValidEventsCount,
			(((double)cprStats.playerValidEventsCount /
			  (double)cprStats.eventValidCount) * 100.0));
	printf("[CprReport]\t invalid count          : %lli (%0.2f %%)\n",
			cprStats.playerInvalidEventsCount,
			(((double)cprStats.playerInvalidEventsCount /
			  (double)cprStats.eventValidCount) * 100.0));
	printf("[CprReport]\t summary records        : %i\n",
			cprStats.playerSummaryCount);
	printf("[CprReport]\t player exists count    : %i (%0.2f %%)\n",
			cprStats.playerSummaryDatabaseExistsCount,
			(((double)cprStats.playerSummaryDatabaseExistsCount /
			  (double)cprStats.playerSummaryCount) * 100.0));
	printf("[CprReport]\t new player count       : %i (%0.2f %%)\n",
			cprStats.playerSummaryDatabaseUpdateCount,
			(((double)cprStats.playerSummaryDatabaseUpdateCount /
			  (double)cprStats.playerSummaryCount) * 100.0));
	printf("[CprReport]\t database access time   : %0.6f\n",
			cprStats.playerDatabaseAccessTime);
	printf("[CprReport]\t avg access time        : %0.6f\n",
			(cprStats.playerDatabaseAccessTime /
			 (double)cprStats.playerSummaryCount));
	printf("[CprReport]\t link index update time : %0.6f\n",
			cprStats.linkIndexUpdateTime);
	printf("[CprReport]\t index update time      : %0.6f\n",
			cprStats.playerIndexUpdateTime);
	printf("[CprReport]\t avg index update time  : %0.6f\n",
			(cprStats.playerIndexUpdateTime /
			 (double)cprStats.eventValidCount));

	printf("[CprReport] summarization statistics:\n");
	printf("[CprReport]\t valid events count     : %lli (%0.2f %%)\n",
			cprStats.summaryValidEventsCount,
			(((double)cprStats.summaryValidEventsCount /
			  (double)cprStats.eventValidCount) * 100.0));
	printf("[CprReport]\t invalid events count   : %lli (%0.2f %%)\n",
			cprStats.summaryInvalidEventsCount,
			(((double)cprStats.summaryInvalidEventsCount /
			  (double)cprStats.eventValidCount) * 100.0));
	printf("[CprReport]\t not-applicable count   : %lli (%0.2f %%)\n",
			cprStats.summaryNotApplicableEventsCount,
			(((double)cprStats.summaryNotApplicableEventsCount /
			  (double)cprStats.eventValidCount) * 100.0));
	printf("[CprReport]\t index updates          : %lli (%0.2f %%)\n",
			cprStats.summaryIndexUpdateCount,
			(((double)cprStats.summaryIndexUpdateCount /
			  (double)cprStats.summaryValidEventsCount) * 100.0));
	printf("[CprReport]\t index update failures  : %lli (%0.2f %%)\n",
			cprStats.summaryIndexUpdateFailureCount,
			(((double)cprStats.summaryIndexUpdateFailureCount /
			  (double)cprStats.summaryValidEventsCount) * 100.0));
	printf("[CprReport]\t construction time      : %0.6f\n",
			cprStats.summaryConstructTime);
	printf("[CprReport]\t avg construction time  : %0.6f\n",
			(cprStats.summaryConstructTime /
			 (double)cprStats.eventValidCount));
	printf("[CprReport]\t index update time      : %0.6f\n",
			cprStats.summaryIndexUpdateTime);
	printf("[CprReport]\t avg index update time  : %0.6f\n",
			(cprStats.summaryIndexUpdateTime /
			 (double)cprStats.eventValidCount));
	printf("[CprReport]\t summary records        : %i\n",
			cprStats.summaryRecordCount);
	printf("[CprReport]\t database merges        : %i (%0.2f %%)\n",
			cprStats.summaryDatabaseMergeCount,
			(((double)cprStats.summaryDatabaseMergeCount /
			  (double)cprStats.summaryDatabaseUpdateCount) * 100.0));
	printf("[CprReport]\t database updates       : %i (%0.2f %%)\n",
			cprStats.summaryDatabaseUpdateCount,
			(((double)cprStats.summaryDatabaseUpdateCount /
			  (double)cprStats.summaryRecordCount) * 100.0));
	printf("[CprReport]\t database update time   : %0.6f\n",
			cprStats.summaryDatabaseUpdateTime);
	printf("[CprReport]\t avg update time        : %0.6f\n",
			(cprStats.summaryDatabaseUpdateTime /
			 (double)cprStats.summaryDatabaseUpdateCount));
}

static int summarizeWowLogfile(char *filename)
{
	aboolean isFirstEvent = atrue;
	aboolean isFirstTrigger = atrue;
	aboolean isDuplicateEvent = afalse;
	int rc = 0;
	int keyLength = 0;
	int readLength = 0;
	int parseResult = 0;
	int summaryResult = 0;
	int recordHistoryId = 0;
	int recordRealmId = 0;
	double timer = 0.0;
	double detailTimer = 0.0;
	char *key = NULL;
	char keyHash[MERGE_INDEX_KEY_HASH_LENGTH];
	char buffer[8192];

	FileHandle fh;

	WowParser parser;
	WowEvent event;
	WowSummary *summary = NULL;

	// initialize database connectivity

	printf("Initializing databases...");
	fflush(NULL);

	if(initDatabase() < 0) {
		printf("FAILED at line %i, aborting.\n", __LINE__);
		return -1;
	}

	if(initIndex() < 0) {
		printf("FAILED at line %i, aborting.\n", __LINE__);
		return -1;
	}

	printf("OK\n");

	// obtain meta-data

	printf("Looking up WoW CPR meta-data...");
	fflush(NULL);

	if(getMetaData() < 0) {
		printf("FAILED at line %i, aborting.\n", __LINE__);
		return -1;
	}

	printf("OK.\n");

	// initialize context

	printf("Initializing context...");
	fflush(NULL);

	if(initContext() < 0) {
		printf("FAILED at line %i, aborting.\n", __LINE__);
		return -1;
	}

	if(newTrigger() < 0) {
		printf("FAILED at line %i, aborting.\n", __LINE__);
		return -1;
	}

	printf("OK\n");

	// initialize file

	printf("Initializing combat log file...");
	fflush(NULL);

	if((rc = file_init(&fh, filename, "r", 8192)) < 0) {
		printf("FAILED at line %i, aborting.\n", __LINE__);
		return -1;
	}

	if((rc = file_getFileLength(&fh, &cprStats.fileLength)) < 0) {
		printf("FAILED at line %i, aborting.\n", __LINE__);
		return -1;
	}

	printf("OK\n");

	// process the file

	printf("Processing combat log file...");
	fflush(NULL);

	if(WOWCPR_ANALYZE_MODE) {
		printf("\n\n");
	}

	timer = time_getTimeMus();

	rc = file_readLine(&fh, buffer, sizeof(buffer), &readLength);
	while(rc == 0) {
		cprStats.fileLineCount += 1;

		detailTimer = time_getTimeMus();

		if(wowstats_parserInit(&parser) < 0) {
			printf("FAILED at line %i, aborting.\n", __LINE__);
			return -1;
		}

		if(wowstats_parserParse(&parser, buffer, readLength) < 0) {
			cprStats.fileLineErrorCount += 1;
			rc = file_readLine(&fh, buffer, sizeof(buffer), &readLength);
			continue;
		}

		cprStats.eventParsingTime += time_getElapsedMusInSeconds(detailTimer);
		detailTimer = time_getTimeMus();

		parseResult = wowstats_eventInit(&event, metaFile.fileYear, &parser);

		cprStats.eventProcessingTime +=
			time_getElapsedMusInSeconds(detailTimer);

		if(parseResult == 0) {
			isDuplicateEvent = afalse;
			cprStats.eventValidCount += 1;

			// update the log history

			if(isFirstEvent) {
				if(wowstats_timestampClone(&metaData.startLogTimestamp,
							&event.timestamp) < 0) {
					printf("FAILED at line %i, aborting.\n", __LINE__);
					return -1;
				}
				if(WOWCPR_ANALYZE_MODE) {
					char timestamp[32];
					wowstats_timestampToString(&metaData.startLogTimestamp,
							timestamp);
					printf("[Analyze] log started at '%s'\n", timestamp);
				}
				if(updateLogHistory() < 0) {
					printf("FAILED at line %i, aborting.\n", __LINE__);
					return -1;
				}
				isFirstEvent = afalse;
			}
			else {
				if(wowstats_timestampClone(&metaData.endLogTimestamp,
							&event.timestamp) < 0) {
					printf("FAILED at line %i, aborting.\n", __LINE__);
					return -1;
				}
			}

			// update the link index

			detailTimer = time_getTimeMus();

			if(wowstats_linkIndexNoteEvent(&wowcpr.linkIndex, &event) < 0) {
				cprStats.playerInvalidEventsCount += 1;
			}
			else {
				cprStats.playerValidEventsCount += 1;
			}

			cprStats.linkIndexUpdateTime +=
				time_getElapsedMusInSeconds(detailTimer);

			// update the player index

			detailTimer = time_getTimeMus();

			if(wowstats_playerIndexNoteEvent(&wowcpr.playerIndex,
						&event) < 0) {
				cprStats.playerInvalidEventsCount += 1;
			}
			else {
				cprStats.playerValidEventsCount += 1;
			}

			cprStats.playerIndexUpdateTime +=
				time_getElapsedMusInSeconds(detailTimer);

			// handle trigger system

			if((rc = wowstats_triggerNoteEvent(&triggerData.trigger,
							&triggerData.playerIndexes[triggerData.ref],
							&event,
							&triggerData.triggerResults[triggerData.ref]
							)
						) < 0) {
				printf("FAILED at line %i, aborting.\n", __LINE__);
				return -1;
			}

			if((WOWCPR_ANALYZE_MODE) &&
					((rc == WOW_TRIGGER_RESULT_NEW_BOSS) ||
					 (rc == WOW_TRIGGER_RESULT_END_BOSS) ||
					 (rc == WOW_TRIGGER_RESULT_NEW_AND_END_BOSS) ||
					 (rc == WOW_TRIGGER_RESULT_END_LOG))) {
				int logTimeSeconds = 0;
				int triggerSeconds = 0;
				static int lastLogTimeSeconds = 0;

				printf("[Analyze] trigger #%03i event [", triggerData.ref);
				if(rc == WOW_TRIGGER_RESULT_NEW_BOSS) {
					printf("NEW BOSS");
				}
				else if(rc == WOW_TRIGGER_RESULT_END_BOSS) {
					printf("END BOSS");
				}
				else if(rc == WOW_TRIGGER_RESULT_NEW_AND_END_BOSS) {
					printf("NEW & END BOSS");
				}
				else if(rc == WOW_TRIGGER_RESULT_END_LOG) {
					printf("END LOG");
				}
				printf("] { ");
				if(triggerData.triggerResults[triggerData.ref].zone != NULL) {
					printf("zone '%s', boss '%s' ",
							triggerData.triggerResults[triggerData.ref].zone,
							triggerData.triggerResults[
								triggerData.ref].bossName);
				}

				wowstats_timestampCalculateDifferenceInSeconds(
						&triggerData.triggerResults[
							triggerData.ref].endTimestamp,
						&triggerData.triggerResults[
							triggerData.ref].startTimestamp,
						&triggerSeconds);
				wowstats_timestampCalculateDifferenceInSeconds(
						&event.timestamp,
						&metaData.startLogTimestamp,
						&logTimeSeconds);

				printf(" } for %i seconds at %i log seconds, %i since\n",
						triggerSeconds, logTimeSeconds,
						(logTimeSeconds - lastLogTimeSeconds));

				lastLogTimeSeconds = logTimeSeconds;
			}

			switch(rc) {
				case WOW_TRIGGER_RESULT_NEW_BOSS:
					if(isFirstTrigger) {
						isFirstTrigger = afalse;
					}
					triggerData.hasTrigger = atrue;
					if(WOWCPR_ANALYZE_MODE) {
						printf("[Analyze] set has trigger to TRUE\n");
					}
					break;
				case WOW_TRIGGER_RESULT_END_BOSS:
					if(isFirstTrigger) {
						printf("FAILED at line %i, aborting.\n", __LINE__);
						return -1;
					}
					triggerData.hasTrigger = afalse;
					if(newTrigger() < 0) {
						printf("FAILED at line %i, aborting.\n", __LINE__);
						return -1;
					}
					if(WOWCPR_ANALYZE_MODE) {
						printf("[Analyze] created new trigger (#%i), set has "
								"trigger to FALSE\n", triggerData.length);
					}
					break;
				case WOW_TRIGGER_RESULT_NEW_AND_END_BOSS:
					if(isFirstTrigger) {
						printf("FAILED at line %i, aborting.\n", __LINE__);
						return -1;
					}
					triggerData.hasTrigger = atrue;
					if(newTrigger() < 0) {
						printf("FAILED at line %i, aborting.\n", __LINE__);
						return -1;
					}
					if(WOWCPR_ANALYZE_MODE) {
						printf("[Analyze] created new trigger (#%i), set has "
								"trigger to TRUE\n", triggerData.length);
					}
					break;
			}

			// handle duplicate event

			detailTimer = time_getTimeMus();

			if(wowstats_eventBuildKey(&event, &key, &keyLength) < 0) {
				printf("FAILED at line %i, aborting.\n", __LINE__);
				return -1;
			}

			buildKeyHash(key, keyLength, keyHash);

			cprStats.eventKeyHashTime +=
				time_getElapsedMusInSeconds(detailTimer);

			if(!WOWCPR_ANALYZE_MODE) {
				detailTimer = time_getTimeMus();

				if(isKeyInIndex(key, keyLength, &recordHistoryId,
							&recordRealmId)) {
//				if(isKeyInIndex(keyHash, 16,
//				if(isKeyInIndex(keyHash, MERGE_INDEX_KEY_HASH_LENGTH,
					isDuplicateEvent = atrue;
				}
				else if(addKeyToIndex(key, keyLength) < 0) {
//				else if(addKeyToIndex(keyHash,
//						MERGE_INDEX_KEY_HASH_LENGTH) < 0) {
					printf("FAILED at line %i, aborting.\n", __LINE__);
					return -1;
				}

				cprStats.eventMergeIndexTime +=
					time_getElapsedMusInSeconds(detailTimer);
			}

			free(key);

			// construct the summary record

			detailTimer = time_getTimeMus();

			summaryResult = wowstats_summaryNew(&summary, &wowcpr.spellIndex,
					&event);

			cprStats.summaryConstructTime +=
				time_getElapsedMusInSeconds(detailTimer);

			// update the summary index

			if(summaryResult == 0) {
				cprStats.summaryValidEventsCount += 1;

				if(isDuplicateEvent) {
					cprStats.eventDuplicateCount += 1;

					detailTimer = time_getTimeMus();

					if(wowstats_summaryIndexUpdateDuplicateSummary(
								&wowcpr.summaryIndex,
								&wowcpr.pgdb,
								recordHistoryId,
								recordRealmId,
								summary) < 0) {
						cprStats.summaryIndexUpdateFailureCount += 1;
					}
					else {
						cprStats.summaryIndexUpdateCount += 1;
					}

					cprStats.summaryIndexUpdateTime +=
						time_getElapsedMusInSeconds(detailTimer);
				}
				else {
					detailTimer = time_getTimeMus();

					if(wowstats_summaryIndexUpdateSummary(
								&wowcpr.summaryIndex,
								summary) < 0) {
						cprStats.summaryIndexUpdateFailureCount += 1;
					}
					else {
						cprStats.summaryIndexUpdateCount += 1;
					}

					cprStats.summaryIndexUpdateTime +=
						time_getElapsedMusInSeconds(detailTimer);

					// update the trigger system summary index

					if((triggerData.length > 0) && (triggerData.hasTrigger)) {
						wowstats_summaryIndexUpdateSummary(
								&triggerData.summaryIndexes[triggerData.ref],
								summary);
					}
				}

				wowstats_summaryFreePtr(summary);
			}
			else if(summaryResult == WOW_SUMMARY_EVENT_NOT_APPLICABLE) {
				wowstats_summaryFreePtr(summary);
				cprStats.summaryNotApplicableEventsCount += 1;
			}
			else if(summaryResult == WOW_SUMMARY_BAD_EVENT) {
				wowstats_summaryFreePtr(summary);
				cprStats.summaryInvalidEventsCount += 1;
			}

			// cleanup event

			if(wowstats_eventFree(&event) < 0) {
				printf("FAILED at line %i, aborting.\n", __LINE__);
				return -1;
			}
		}
		else if(parseResult == WOWSTATS_EVENT_INVALID_EVENT) {
			cprStats.eventInvalidCount += 1;
		}
		else if(parseResult == WOWSTATS_EVENT_UNKNOWN_EVENT) {
			cprStats.eventUnknownCount += 1;
		}

/*
 * Note: for debugging only
 *
		if(time_getElapsedMusInSeconds(timer) > 10.0) {
			break;
		}
*/

		rc = file_readLine(&fh, buffer, sizeof(buffer), &readLength);
	}

	if(rc < 0) {
		printf("FAILED at line %i, aborting.\n", __LINE__);
		return -1;
	}

	if((rc = file_free(&fh)) < 0) {
		printf("FAILED at line %i, aborting.\n", __LINE__);
		return -1;
	}

	cprStats.fileProcessingTime = time_getElapsedMusInSeconds(timer);

	printf("OK\n");

	// update the database

	printf("Updating database player records...");
	fflush(NULL);

	if(updatePlayers() < 0) {
		printf("FAILED at line %i, aborting.\n", __LINE__);
		return -1;
	}

	printf("OK\n");

	printf("Updating database summary records...");
	fflush(NULL);

	if(updateSummaries() < 0) {
		printf("FAILED at line %i, aborting.\n", __LINE__);
		return -1;
	}

	printf("OK\n");
	printf("Updating trigger summary records...");
	fflush(NULL);

	if(updateTriggerSummaries() < 0) {
		printf("FAILED at line %i, aborting.\n", __LINE__);
		return -1;
	}

	printf("OK\n");

	// free database connectivity

	printf("Closing databases...");
	fflush(NULL);

	if(freeDatabase() < 0) {
		printf("FAILED at line %i, aborting.\n", __LINE__);
		return -1;
	}

	if(freeIndex() < 0) {
		printf("FAILED at line %i, aborting.\n", __LINE__);
		return -1;
	}

	printf("OK\n");

	// free context

	printf("Free'ing context...");
	fflush(NULL);

	if(freeContext() < 0) {
		printf("FAILED at line %i, aborting.\n", __LINE__);
		return -1;
	}

	printf("OK\n");

	// display processing report

	displayProcessingReport();

	return 0;
}

