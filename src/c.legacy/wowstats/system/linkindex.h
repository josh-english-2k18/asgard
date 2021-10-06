/*
 * linkindex.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard-WoW statistical analysis link-index system, header file.
 *
 * Written by Josh English.
 */

#if !defined(_WOWSTATS_SYSTEM_LINKINDEX_H)

#define _WOWSTATS_SYSTEM_LINKINDEX_H

#if !defined(_WOWSTATS_SYSTEM_H) && !defined(_WOWSTATS_SYSTEM_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _WOWSTATS_SYSTEM_H || _WOWSTATS_SYSTEM_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

// define wowstats link-index public data types

typedef struct _WowLinkIndex {
	Bptree index;
} WowLinkIndex;

// declare wowstats link-index public functions

int wowstats_linkIndexInit(WowLinkIndex *index);

int wowstats_linkIndexFree(WowLinkIndex *index);

int wowstats_linkIndexNoteEvent(WowLinkIndex *index, WowEvent *event);

int wowstats_linkIndexGet(WowLinkIndex *index, char *uid,
		WowLinkSystem **link);

int wowstats_linkIndexGetLinks(WowLinkIndex *index, WowLinkSystem ***links,
		int *linkLength);

#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _WOWSTATS_SYSTEM_LINKINDEX_H

