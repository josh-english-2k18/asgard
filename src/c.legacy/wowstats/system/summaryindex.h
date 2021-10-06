/*
 * summaryindex.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard-WoW statistical analysis summary-index, header file.
 *
 * Written by Josh English.
 */

#if !defined(_WOWSTATS_SYSTEM_SUMMARYINDEX_H)

#define _WOWSTATS_SYSTEM_SUMMARYINDEX_H

#if !defined(_WOWSTATS_SYSTEM_H) && !defined(_WOWSTATS_SYSTEM_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _WOWSTATS_SYSTEM_H || _WOWSTATS_SYSTEM_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

// define wowstats summary-index public data types

typedef struct _WowSummaryIndexEntry {
	aboolean wasUpdated;
	aboolean isParentRecord;
	int logHistoryId;
	int realmId;
	WowSummary *summary;
} WowSummaryIndexEntry;

typedef struct _WowSummaryRecord {
	char *entityUid;
	Bptree index;
} WowSummaryRecord;

typedef struct _WowSummaryIndex {
	WowLinkIndex *linkIndex;
	Bptree index;
} WowSummaryIndex;

// declare wowstats summary-index public functions

int wowstats_summaryIndexInit(WowSummaryIndex *summaryIndex,
		WowLinkIndex *linkIndex);

int wowstats_summaryIndexFree(WowSummaryIndex *summaryIndex);

int wowstats_summaryIndexUpdateSummary(WowSummaryIndex *summaryIndex,
		WowSummary *summary);

int wowstats_summaryIndexUpdateDuplicateSummary(WowSummaryIndex *summaryIndex,
		Pgdb *pgdb, int logHistoryId, int realmId, WowSummary *summary);

int wowstats_summaryIndexListSummaries(WowSummaryIndex *summaryIndex,
		char ***summaryList, int *summaryListLength);

int wowstats_summaryIndexListSummaryRecords(WowSummaryIndex *summaryIndex,
		char *summaryName, char ***summaryRecordList,
		int *summaryRecordListLength);

int wowstats_summaryIndexGetEntry(WowSummaryIndex *summaryIndex,
		char *summaryName, char *summaryRecordName,
		WowSummaryIndexEntry **indexEntry);

int wowstats_summaryIndexGetSummary(WowSummaryIndex *summaryIndex,
		char *summaryName, char *summaryRecordName, WowSummary **summary);

#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _WOWSTATS_SYSTEM_SUMMARYINDEX_H

