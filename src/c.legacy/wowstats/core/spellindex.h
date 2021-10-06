/*
 * spellindex.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard-WoW statistical analysis spell index, header file.
 *
 * Written by Josh English.
 */

#if !defined(_WOWSTATS_CORE_SPELLINDEX_H)

#define _WOWSTATS_CORE_SPELLINDEX_H

#if !defined(_WOWSTATS_CORE_H) && !defined(_WOWSTATS_CORE_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _WOWSTATS_CORE_H || _WOWSTATS_CORE_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

// define wowstats spell index public data types

typedef struct _WowSpellindex {
	Bptree index;
} WowSpellindex;

// declare wowstats spell index public functions

int wowstats_spellindexInit(WowSpellindex *index);

int wowstats_spellindexFree(WowSpellindex *index);

int wowstats_spellindexLookup(WowSpellindex *index, int spellId,
		WowSpellList **spell);

#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _WOWSTATS_CORE_SPELLINDEX_H

