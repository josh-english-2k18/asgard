/*
 * events.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard-WoW statistical analysis events-parsing data, header file.
 *
 * Written by Josh English.
 */

#if !defined(_WOWSTATS_DATA_EVENTS_H)

#define _WOWSTATS_DATA_EVENTS_H

#if !defined(_WOWSTATS_DATA_H) && !defined(_WOWSTATS_DATA_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _WOWSTATS_DATA_H || _WOWSTATS_DATA_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

// define wowstats events static variables

static WowEventList WOW_EVENT_LIST[] = {
	{		WOW_EVENT_DAMAGE_SHIELD,
		"DAMAGE_SHIELD",			13,		"Damaged a shield"			},
	{		WOW_EVENT_DAMAGE_SHIELD_MISSED,
		"DAMAGE_SHIELD_MISSED",		20,		"Damage to shield missed"	},
	{		WOW_EVENT_DAMAGE_SPLIT,
		"DAMAGE_SPLIT",				12,		"Damage split"				},
	{		WOW_EVENT_ENCHANT_APPLIED,
		"ENCHANT_APPLIED",			15,		"Enchantment applied"		},
	{		WOW_EVENT_ENCHANT_REMOVED,
		"ENCHANT_REMOVED",			15,		"Enchantment removed"		},
	{		WOW_EVENT_PARTY_KILL,
		"PARTY_KILL",				10,		"Party was killed"			},
	{		WOW_EVENT_UNIT_DIED,
		"UNIT_DIED",				9,		"Unit died"					},
	{		WOW_EVENT_UNIT_DESTROYED,
		"UNIT_DESTROYED",			14,		"Unit destroyed"			},
	{		WOW_EVENT_ENVIRONMENTAL_DAMAGE,
		"ENVIRONMENTAL_DAMAGE",		20,		"Environmental damage"		},
	{		WOW_EVENT_RANGE_DAMAGE,
		"RANGE_DAMAGE",				12,		"Range damage"				},
	{		WOW_EVENT_RANGE_MISSED,
		"RANGE_MISSED",				12,		"Range damage missed"		},
	{		WOW_EVENT_SPELL_AURA_APPLIED,
		"SPELL_AURA_APPLIED",		18,		"Spell aura applied"		},
	{		WOW_EVENT_SPELL_AURA_APPLIED_DOSE,
		"SPELL_AURA_APPLIED_DOSE",	23,		"Spell aura applied dose"	},
	{		WOW_EVENT_SPELL_AURA_BROKEN,
		"SPELL_AURA_BROKEN",		17,		"Spell aura broken"			},
	{		WOW_EVENT_SPELL_AURA_BROKEN_SPELL,
		"SPELL_AURA_BROKEN_SPELL",	23,		"Spell aura broken spell"	},
	{		WOW_EVENT_SPELL_AURA_REFRESH,
		"SPELL_AURA_REFRESH",		18,		"Spell aura refreshed"		},
	{		WOW_EVENT_SPELL_AURA_REMOVED,
		"SPELL_AURA_REMOVED",		18,		"Spell aura removed"		},
	{		WOW_EVENT_SPELL_AURA_REMOVED_DOSE,
		"SPELL_AURA_REMOVED_DOSE",	23,		"Spell aura removed dose"	},
	{		WOW_EVENT_SPELL_CAST_FAILED,
		"SPELL_CAST_FAILED",		17,		"Spell cast failed"			},
	{		WOW_EVENT_SPELL_CAST_START,
		"SPELL_CAST_START",			16,		"Spell cast started"		},
	{		WOW_EVENT_SPELL_CAST_SUCCESS,
		"SPELL_CAST_SUCCESS",		18,		"Spell cast successfully"	},
	{		WOW_EVENT_SPELL_CREATE,
		"SPELL_CREATE",				12,		"Spell created"				},
	{		WOW_EVENT_SPELL_DAMAGE,
		"SPELL_DAMAGE",				12,		"Spell damage"				},
	{		WOW_EVENT_SPELL_DISPEL,
		"SPELL_DISPEL",				12,		"Spell dispel"				},
	{		WOW_EVENT_SPELL_DISPEL_FAILED,
		"SPELL_DISPEL_FAILED",		19,		"Spell dispel failed"		},
	{		WOW_EVENT_SPELL_ENERGIZE,
		"SPELL_ENERGIZE",			14,		"Spell energized"			},
	{		WOW_EVENT_SPELL_EXTRA_ATTACKS,
		"SPELL_EXTRA_ATTACKS",		19,		"Spell extra attacks"		},
	{		WOW_EVENT_SPELL_HEAL,
		"SPELL_HEAL",				10,		"Spell heal"				},
	{		WOW_EVENT_SPELL_INSTAKILL,
		"SPELL_INSTAKILL",			15,		"Spell instant kill"		},
	{		WOW_EVENT_SPELL_INTERRUPT,
		"SPELL_INTERRUPT",			15,		"Spell interrupt"			},
	{		WOW_EVENT_SPELL_LEECH,
		"SPELL_LEECH",				11,		"Spell leech"				},
	{		WOW_EVENT_SPELL_MISSED,
		"SPELL_MISSED",				12,		"Spell missed"				},
	{		WOW_EVENT_SPELL_RESURRECT,
		"SPELL_RESURRECT",			15,		"Spell resurrected"			},
	{		WOW_EVENT_SPELL_STOLEN,
		"SPELL_STOLEN",				12,		"Spell stolen"				},
	{		WOW_EVENT_SPELL_SUMMON,
		"SPELL_SUMMON",				12,		"Spell summoned"			},
	{		WOW_EVENT_SPELL_PERIODIC_DAMAGE,
		"SPELL_PERIODIC_DAMAGE",	21,		"Spell periodic damage"		},
	{		WOW_EVENT_SPELL_PERIODIC_ENERGIZE,
		"SPELL_PERIODIC_ENERGIZE",	23,		"Spell periodic energize"	},
	{		WOW_EVENT_SPELL_PERIODIC_HEAL,
		"SPELL_PERIODIC_HEAL",		19,		"Spell periodic heal"		},
	{		WOW_EVENT_SPELL_PERIODIC_MISSED,
		"SPELL_PERIODIC_MISSED",	21,		"Spell periodic missed"		},
	{		WOW_EVENT_SWING_DAMAGE,
		"SWING_DAMAGE",				12,		"Swing damage"				},
	{		WOW_EVENT_SWING_MISSED,
		"SWING_MISSED",				12,		"Swing missed"				},
	{		WOW_EVENT_END_OF_LIST,
		NULL,						0,		NULL						}
};

#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _WOWSTATS_DATA_EVENTS_H

