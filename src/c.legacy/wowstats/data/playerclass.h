/*
 * playerclass.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard-WoW statistical analysis player class definitions, header file.
 *
 * Written by Josh English.
 */

#if !defined(_WOWSTATS_DATA_PLAYERCLASS_H)

#define _WOWSTATS_DATA_PLAYERCLASS_H

#if !defined(_WOWSTATS_DATA_H) && !defined(_WOWSTATS_DATA_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _WOWSTATS_DATA_H || _WOWSTATS_DATA_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

// define wowstats player class public constants

static WowPlayerClassList WOW_PLAYER_CLASS_LIST[] = {
	{	WOW_PLAYER_CLASS_UNKNOWN,			"Unknown"			},
	{	WOW_PLAYER_CLASS_WARRIOR,			"Warrior"			},
	{	WOW_PLAYER_CLASS_DRUID,				"Druid"				},
	{	WOW_PLAYER_CLASS_WARLOCK,			"Warlock"			},
	{	WOW_PLAYER_CLASS_SHAMAN,			"Shaman"			},
	{	WOW_PLAYER_CLASS_PALADIN,			"Paladin"			},
	{	WOW_PLAYER_CLASS_PRIEST,			"Priest"			},
	{	WOW_PLAYER_CLASS_ROGUE,				"Rogue"				},
	{	WOW_PLAYER_CLASS_MAGE,				"Mage"				},
	{	WOW_PLAYER_CLASS_HUNTER,			"Hunter"			},
	{	WOW_PLAYER_CLASS_DEATHKNIGHT,		"Death Knight"		},
	{	WOW_PLAYER_CLASS_NPC,				"NPC"				},
	{	WOW_PLAYER_CLASS_PET,				"Pet"				},
	{	WOW_PLAYER_CLASS_ENVIRONMENT,		"Environment"		},
	{	WOW_PLAYER_CLASS_VEHICLE,			"Vehicle"			},
	{	WOW_PLAYER_CLASS_END_OF_LIST,		NULL				}
};

#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _WOWSTATS_DATA_PLAYERCLASS_H

