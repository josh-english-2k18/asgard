/*
 * eventspellss.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard-WoW statistical analysis events-parsing spell data, header file.
 *
 * Written by Josh English.
 */

#if !defined(_WOWSTATS_DATA_EVENTSPELLS_H)

#define _WOWSTATS_DATA_EVENTSPELLS_H

#if !defined(_WOWSTATS_DATA_H) && !defined(_WOWSTATS_DATA_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _WOWSTATS_DATA_H || _WOWSTATS_DATA_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

// define wowstats event-spells static variables

static WowSpellSchool WOW_SPELL_SCHOOL[] = {
	{		WOW_SPELL_SCHOOL_PHYSICAL,
		"0x0",			3,			"Physical",		"Physical"				},
	{		WOW_SPELL_SCHOOL_PHYSICAL,
		"0x1",			3,			"Physical",		"Physical"				},
	{		WOW_SPELL_SCHOOL_HOLY,
		"0x2",			3,			"Holy",			"Holy"					},
	{		WOW_SPELL_SCHOOL_FIRE,
		"0x4",			3,			"Fire",			"Fire"					},
	{		WOW_SPELL_SCHOOL_PHYSICAL,
		"0x5",			3,			"Physical",		"Physical"				},
	{		WOW_SPELL_SCHOOL_NATURE,
		"0x8",			3,			"Nature",		"Nature"				},
	{		WOW_SPELL_SCHOOL_PHYSICAL,
		"0x9",			3,			"Physical",		"Physical"				},
	{		WOW_SPELL_SCHOOL_FROST,
		"0x10",			4,			"Frost",		"Frost"					},
	{		WOW_SPELL_SCHOOL_FROSTFIRE,
		"0x14",			4,			"Frostfire",	"Frost plus Fire"		},
	{		WOW_SPELL_SCHOOL_FROSTSTORM,
		"0x18",			4,			"Froststorm",	"Frost plus Nature"		},
	{		WOW_SPELL_SCHOOL_SHADOW,
		"0x20",			4,			"Shadow",		"Shadow"				},
	{		WOW_SPELL_SCHOOL_SHADOWSTORM,
		"0x28",			4,			"Shadowstorm",	"Shadow plus Nature"	},
	{		WOW_SPELL_SCHOOL_ARCANE,
		"0x40",			4,			"Arcane",		"Arcane"				},
	{		WOW_SPELL_SCHOOL_HOLY,
		"0x42",			4,			"Holy",			"Holy"					},
	{		WOW_SPELL_SCHOOL_FIRE,
		"0x44",			4,			"Fire",			"Fire"					},
	{		WOW_SPELL_SCHOOL_ARCANE,
		"0x50",			4,			"Arcane",		"Arcane"				},
	{		WOW_SPELL_SCHOOL_END_OF_LIST,
		NULL,			0,			NULL,			NULL					}
};

#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _WOWSTATS_DATA_EVENTSPELLS_H

