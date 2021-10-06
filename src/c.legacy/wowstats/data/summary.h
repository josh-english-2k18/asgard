/*
 * summary.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard-WoW statistical analysis summary definitions, header file.
 *
 * Written by Josh English.
 */

#if !defined(_WOWSTATS_DATA_SUMMARY_H)

#define _WOWSTATS_DATA_SUMMARY_H

#if !defined(_WOWSTATS_DATA_H) && !defined(_WOWSTATS_DATA_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _WOWSTATS_DATA_H || _WOWSTATS_DATA_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

// define wowstats summary public constants

static WowSummaryList WOW_SUMMARY_LIST[] = {
	{	WOW_SUMMARY_HEALING,			"Healing",
			"The sum of player healing.",								},
	{	WOW_SUMMARY_DAMAGE,				"Damage",
			"The sum of player-dealt damage.",							},
	{	WOW_SUMMARY_END_OF_LIST,		NULL,
			NULL														}
};

#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _WOWSTATS_DATA_SUMMARY_H

