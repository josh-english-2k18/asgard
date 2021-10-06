/*
 * link.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard-WoW statistical analysis link (correlation between a player and
 * an object, e.g. pet, etc.) system, header file.
 *
 * Written by Josh English.
 */

#if !defined(_WOWSTATS_CORE_LINK_H)

#define _WOWSTATS_CORE_LINK_H

#if !defined(_WOWSTATS_CORE_H) && !defined(_WOWSTATS_CORE_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _WOWSTATS_CORE_H || _WOWSTATS_CORE_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

// define wowstats link-system public data types

typedef struct _WowLinkSystem {
	char *playerUid;
	char *playerName;
	int objectId;
	char *objectUid;
	char *objectName;
} WowLinkSystem;

// declare wowstats link-system public functions

int wowstats_linkInit(WowLinkSystem *link, WowEvent *event);

int wowstats_linkNew(WowLinkSystem **link, WowEvent *event);

int wowstats_linkFree(WowLinkSystem *link);

#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _WOWSTATS_CORE_LINK_H

