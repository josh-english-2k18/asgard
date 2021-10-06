/*
 * summaryindex.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard-WoW statistical analysis summary-index.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#include "wowstats/common.h"
#include "wowstats/core.h"
#include "wowstats/db.h"
#define _WOWSTATS_SYSTEM_COMPONENT
#include "wowstats/system/linkindex.h"
#include "wowstats/system/summary.h"
#include "wowstats/system/summaryindex.h"

// declare wowstats summary-index private functions

// summary functions

static void freeWowSummary(void *memory);

// index-entry functions

static WowSummaryIndexEntry *newIndexEntry(aboolean cloneSummary,
		WowSummary *summary);

static void freeIndexEntry(void *memory);

// summary-record functions

static WowSummaryRecord *newSummaryRecord(char *entityUid);

static void freeSummaryRecord(void *memory);


// define wowstats summary-index private functions

// summary functions

static void freeWowSummary(void *memory)
{
	WowSummary *summary = NULL;

	if(memory == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	summary = (WowSummary *)memory;

	wowstats_summaryFreePtr(summary);
}

// index-entry functions

static WowSummaryIndexEntry *newIndexEntry(aboolean cloneSummary,
		WowSummary *summary)
{
	WowSummaryIndexEntry *result = NULL;

	result = (WowSummaryIndexEntry *)malloc(sizeof(WowSummaryIndexEntry));

	result->wasUpdated = afalse;
	result->isParentRecord = afalse;
	result->logHistoryId = 0;
	result->realmId = 0;

	if(cloneSummary) {
		if(wowstats_summaryClone(summary, &result->summary) < 0) {
			fprintf(stderr, "[%s():%i] error - failed to clone summary.\n",
					__FUNCTION__, __LINE__);
			exit(1);
		}
	}
	else {
		result->summary = summary;
	}

	return result;
}

static void freeIndexEntry(void *memory)
{
	WowSummaryIndexEntry *entry = NULL;

	if(memory == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	entry = (WowSummaryIndexEntry *)memory;

	freeWowSummary(entry->summary);
	free(entry);
}

// summary-record functions

static WowSummaryRecord *newSummaryRecord(char *entityUid)
{
	WowSummaryRecord *result = NULL;

	result = (WowSummaryRecord *)malloc(sizeof(WowSummaryRecord));

	result->entityUid = strdup(entityUid);

	if(bptree_init(&result->index) < 0) {
		freeSummaryRecord(result);
		return NULL;
	}

	if(bptree_setFreeFunction(&result->index, freeIndexEntry) < 0) {
		freeSummaryRecord(result);
		return NULL;
	}

	return result;
}

static void freeSummaryRecord(void *memory)
{
	WowSummaryRecord *summaryRecord = NULL;

	if(memory == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	summaryRecord = (WowSummaryRecord *)memory;

	if(summaryRecord->entityUid != NULL) {
		free(summaryRecord->entityUid);
	}

	bptree_free(&summaryRecord->index);

	free(summaryRecord);
}


// define wowstats summary-index public functions

int wowstats_summaryIndexInit(WowSummaryIndex *summaryIndex,
		WowLinkIndex *linkIndex)
{
	if((summaryIndex == NULL) || (linkIndex == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	memset(summaryIndex, 0, sizeof(WowSummaryIndex));

	if(bptree_init(&summaryIndex->index) < 0) {
		return -1;
	}

	if(bptree_setFreeFunction(&summaryIndex->index, freeSummaryRecord) < 0) {
		return -1;
	}

	summaryIndex->linkIndex = linkIndex;

	return 0;
}

int wowstats_summaryIndexFree(WowSummaryIndex *summaryIndex)
{
	if(summaryIndex == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(bptree_free(&summaryIndex->index) < 0) {
		return -1;
	}

	memset(summaryIndex, 0, sizeof(WowSummaryIndex));

	return 0;
}

int wowstats_summaryIndexUpdateSummary(WowSummaryIndex *summaryIndex,
		WowSummary *summary)
{
	int keyLength = 0;
	char *key = NULL;

	WowLinkSystem *link = NULL;
	WowSummary *linkSummary = NULL;
	WowSummaryRecord *record = NULL;
	WowSummaryIndexEntry *entry = NULL;

	if((summaryIndex == NULL) || (summary == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(wowstats_summaryBuildEntityKey(summary, &key, &keyLength) < 0) {
		return -1;
	}

	if(bptree_get(&summaryIndex->index, key, keyLength,
				((void *)&record)) < 0) {
		record = newSummaryRecord(summary->sourceUid);
		if(bptree_put(&summaryIndex->index, key, keyLength, record) < 0) {
			freeSummaryRecord(record);
			return -1;
		}
	}

	if(wowstats_summaryBuildKey(summary, &key, &keyLength) < 0) {
		return -1;
	}

	if(bptree_get(&record->index, key, keyLength,
				((void *)&entry)) < 0) {
		entry = newIndexEntry(atrue, summary);
		if(bptree_put(&record->index, key, keyLength, entry) < 0) {
			free(key);
			return -1;
		}
	}
	else {
		if(wowstats_summaryUpdateRecord(entry->summary, summary) < 0) {
			free(key);
			return -1;
		}
		if(entry->isParentRecord) {
			entry->wasUpdated = atrue;
		}
	}

	free(key);

	if(wowstats_linkIndexGet(summaryIndex->linkIndex, summary->sourceUid,
				&link) == 0) {
		key = link->playerUid;
		keyLength = strlen(link->playerUid);

		if(bptree_get(&summaryIndex->index, key, keyLength,
					((void *)&record)) < 0) {
			record = newSummaryRecord(summary->sourceUid);
			if(bptree_put(&summaryIndex->index, key, keyLength, record) < 0) {
				freeSummaryRecord(record);
				return -1;
			}
		}

		if(wowstats_summaryBuildOverrideKey(summary, link->playerUid, &key,
					&keyLength) < 0) {
			return -1;
		}

		if(wowstats_summaryClone(summary, &linkSummary) < 0) {
			free(key);
			return -1;
		}

		free(linkSummary->source);
		free(linkSummary->sourceUid);

		linkSummary->source = strdup(link->playerName);
		linkSummary->sourceUid = strdup(link->playerUid);

		if(wowstats_summaryPrependName(linkSummary, link->objectName) < 0) {
			free(key);
			return -1;
		}

		if(bptree_get(&record->index, key, keyLength,
					((void *)&entry)) < 0) {
			entry = newIndexEntry(atrue, linkSummary);
			if(bptree_put(&record->index, key, keyLength, entry) < 0) {
				free(key);
				wowstats_summaryFreePtr(linkSummary);
				return -1;
			}
		}
		else {
			if(wowstats_summaryUpdateRecord(entry->summary, linkSummary) < 0) {
				free(key);
				wowstats_summaryFreePtr(linkSummary);
				return -1;
			}
			if(entry->isParentRecord) {
				entry->wasUpdated = atrue;
			}
		}

		free(key);
		wowstats_summaryFreePtr(linkSummary);
	}

	return 0;
}

int wowstats_summaryIndexUpdateDuplicateSummary(WowSummaryIndex *summaryIndex,
		Pgdb *pgdb, int logHistoryId, int realmId, WowSummary *summary)
{
	int keyLength = 0;
	char *key = NULL;

	WowSummary *parentSummary = NULL;
	WowSummaryRecord *record = NULL;
	WowSummaryIndexEntry *entry = NULL;

	if((summaryIndex == NULL) || (pgdb == NULL) || (logHistoryId < 1) ||
			(realmId < 1) || (summary == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(wowstats_summaryBuildEntityKey(summary, &key, &keyLength) < 0) {
		return -1;
	}

	if(bptree_get(&summaryIndex->index, key, keyLength,
				((void *)&record)) < 0) {
		record = newSummaryRecord(summary->sourceUid);
		if(bptree_put(&summaryIndex->index, key, keyLength, record) < 0) {
			freeSummaryRecord(record);
			return -1;
		}
	}

	if(wowstats_summaryBuildKey(summary, &key, &keyLength) < 0) {
		return -1;
	}

	if(bptree_get(&record->index, key, keyLength,
				((void *)&entry)) < 0) {
		if(wowstats_summaryLookup(summary, pgdb, logHistoryId, realmId,
					&parentSummary) < 0) {
			free(key);
			return wowstats_summaryIndexUpdateSummary(summaryIndex, summary);
		}

		entry = newIndexEntry(afalse, parentSummary);

		entry->isParentRecord = atrue;
		entry->logHistoryId = logHistoryId;
		entry->realmId = realmId;

		if(bptree_put(&record->index, key, keyLength, entry) < 0) {
			free(key);
			return -1;
		}
	}
	else {
		if(!entry->isParentRecord) {
			free(key);
			return -1;
		}
	}

	free(key);

	return 0;
}

int wowstats_summaryIndexListSummaries(WowSummaryIndex *summaryIndex,
		char ***summaryList, int *summaryListLength)
{
	int *keyLengths = NULL;
	void **values = NULL;

	if((summaryIndex == NULL) || (summaryList == NULL) ||
			(summaryListLength == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(bptree_toArray(&summaryIndex->index, summaryListLength, &keyLengths,
				summaryList, &values) < 0) {
		return -1;
	}

	free(keyLengths);
	free(values);

	return 0;
}

int wowstats_summaryIndexListSummaryRecords(WowSummaryIndex *summaryIndex,
		char *summaryName, char ***summaryRecordList,
		int *summaryRecordListLength)
{
	int *keyLengths = NULL;
	void **values = NULL;

	WowSummaryRecord *record = NULL;

	if((summaryIndex == NULL) || (summaryName == NULL) ||
			(summaryRecordList == NULL) || (summaryRecordListLength == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(bptree_get(&summaryIndex->index, summaryName, strlen(summaryName),
				((void *)&record)) < 0) {
		return -1;
	}

	if(bptree_toArray(&record->index, summaryRecordListLength, &keyLengths,
				summaryRecordList, &values) < 0) {
		return -1;
	}

	free(keyLengths);
	free(values);

	return 0;
}

int wowstats_summaryIndexGetEntry(WowSummaryIndex *summaryIndex,
		char *summaryName, char *summaryRecordName,
		WowSummaryIndexEntry **indexEntry)
{
	WowSummaryRecord *record = NULL;
	WowSummaryIndexEntry *entry = NULL;

	if((summaryIndex == NULL) || (summaryName == NULL) ||
			(summaryRecordName == NULL) || (indexEntry == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(bptree_get(&summaryIndex->index, summaryName, strlen(summaryName),
				((void *)&record)) < 0) {
		return -1;
	}

	if(bptree_get(&record->index, summaryRecordName, strlen(summaryRecordName),
				((void *)&entry)) < 0) {
		return -1;
	}

	*indexEntry = entry;

	return 0;
}

int wowstats_summaryIndexGetSummary(WowSummaryIndex *summaryIndex,
		char *summaryName, char *summaryRecordName, WowSummary **summary)
{
	WowSummaryRecord *record = NULL;
	WowSummaryIndexEntry *entry = NULL;

	if((summaryIndex == NULL) || (summaryName == NULL) ||
			(summaryRecordName == NULL) || (summary == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(bptree_get(&summaryIndex->index, summaryName, strlen(summaryName),
				((void *)&record)) < 0) {
		return -1;
	}

	if(bptree_get(&record->index, summaryRecordName, strlen(summaryRecordName),
				((void *)&entry)) < 0) {
		return -1;
	}

	*summary = entry->summary;

	return 0;
}

