/*
 * eventpowers.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard-WoW statistical analysis events-parsing powers data, header file.
 *
 * Written by Josh English.
 */

#if !defined(_WOWSTATS_DATA_EVENTPOWERS_H)

#define _WOWSTATS_DATA_EVENTPOWERS_H

#if !defined(_WOWSTATS_DATA_H) && !defined(_WOWSTATS_DATA_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _WOWSTATS_DATA_H || _WOWSTATS_DATA_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

// define wowstats event-powers static variables

static WowPowerType WOW_POWER_TYPE[] = {
	{		WOW_POWER_TYPE_HEALTH,
		-2,		"Health",				"Health"							},
	{		WOW_POWER_TYPE_MANA,
		0,		"Mana",					"Mana"								},
	{		WOW_POWER_TYPE_RAGE,
		1,		"Rage",					"Rage"								},
	{		WOW_POWER_TYPE_FOCUS,
		2,		"Focus",				"Focus"								},
	{		WOW_POWER_TYPE_ENERGY,
		3,		"Energy",				"Energy"							},
	{		WOW_POWER_TYPE_PET_HAPPINESS,
		4,		"Pet Happiness",		"Pet Happiness"						},
	{		WOW_POWER_TYPE_RUNES,
		5,		"Runes",				"Runes"								},
	{		WOW_POWER_TYPE_RUNIC_POWER,
		6,		"Runic Power",			"Runic Power"						},
	{		WOW_POWER_TYPE_END_OF_LIST,
		-1,		NULL,					NULL								}
};

#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _WOWSTATS_DATA_EVENTPOWERS_H

