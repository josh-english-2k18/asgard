/*
 * trigger.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard-WoW statistical analysis trigger system.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#include "wowstats/common.h"
#include "wowstats/core.h"
#define _WOWSTATS_SYSTEM_COMPONENT
#include "wowstats/system/player.h"
#include "wowstats/system/playerindex.h"
#include "wowstats/system/trigger.h"

// define trigger private constants

static WowSpellList WOW_TRIGGER_IGNORE_SPELL_LIST[] = {
	{	1130,		"Hunter",		"Hunter",			1		},
	{	14323,		"Hunter",		"Hunter",			2		},
	{	14324,		"Hunter",		"Hunter",			3		},
	{	14325,		"Hunter",		"Hunter",			4		},
	{	53338,		"Hunter",		"Hunter",			5		},
	{	-1,			NULL,			NULL,				0		}
};

static WowSpellList WOW_TRIGGER_OUT_OF_COMBAT_AURA_SPELL_LIST[] = {
	{	52911,		"Unknown",		"Drink",			0	},
	{	43183,		"Unknown",		"Drink",			0	},
	{	43182,		"Unknown",		"Drink",			0	},
	{	43706,		"Unknown",		"Drink",			0	},
	{	27089,		"Unknown",		"Drink",			0	},
	{	-1,			NULL,			NULL,				0	}
};

static WowSpellList WOW_TRIGGER_OUT_OF_COMBAT_RESURRECT_SPELL_LIST[] = {
	{	48171,		"Priest",		"Resurrection",		7	},
	{	25435,		"Priest",		"Resurrection",		6	},
	{	20770,		"Priest",		"Resurrection",		5	},
	{	10881,		"Priest",		"Resurrection",		4	},
	{	10880,		"Priest",		"Resurrection",		3	},
	{	2010,		"Priest",		"Resurrection",		2	},
	{	2006,		"Priest",		"Resurrection",		1	},
	{	48950,		"Paladin",		"Redemption",		7	},
	{	48949,		"Paladin",		"Redemption",		6	},
	{	20773,		"Paladin",		"Redemption",		5	},
	{	20772,		"Paladin",		"Redemption",		4	},
	{	10324,		"Paladin",		"Redemption",		3	},
	{	10322,		"Paladin",		"Redemption",		2	},
	{	7328,		"Paladin",		"Redemption",		1	},
	{	50763,		"Druid",		"Revive",			7	},
	{	50764,		"Druid",		"Revive",			6	},
	{	50765,		"Druid",		"Revive",			5	},
	{	50766,		"Druid",		"Revive",			4	},
	{	50767,		"Druid",		"Revive",			3	},
	{	50768,		"Druid",		"Revive",			2	},
	{	50769,		"Druid",		"Revive",			1	},
	{	49277,		"Shaman",		"Ancestral Spirit",	7	},
	{	25590,		"Shaman",		"Ancestral Spirit",	6	},
	{	20777,		"Shaman",		"Ancestral Spirit",	5	},
	{	20776,		"Shaman",		"Ancestral Spirit",	4	},
	{	20610,		"Shaman",		"Ancestral Spirit",	3	},
	{	20609,		"Shaman",		"Ancestral Spirit",	2	},
	{	2008,		"Shaman",		"Ancestral Spirit",	1	},
	{	-1,			NULL,			NULL,				0	}
};

static WowTriggerList WOW_TRIGGERS[] = {
	/*
	 * The Obsidian Sanctum
	 */

	{
		WOW_TRIGGER_BOSS_SARTHARION,
		"The Obsidian Sanctum",
		28860,
		"Sartharion",
		{
			-1 // end of list
		},
		{
			28860, // Sartharion
			-1 // end of list
		},
		{
			30449, // Vesperon
			30452, // Tenebron
			30451, // Shadron
			-1 // end of list
		},
		{
			28860, // Sartharion
			-1 // end of list
		},
		45
	},
	{
		WOW_TRIGGER_BOSS_VESPERON,
		"The Obsidian Sanctum",
		30449,
		"Vesperon",
		{
			-1 // end of list
		},
		{
			30449, // Vesperon
			-1 // end of list
		},
		{
			-1 // end of list
		},
		{
			30449, // Vesperon
			-1 // end of list
		},
		10
	},
	{
		WOW_TRIGGER_BOSS_TENEBRON,
		"The Obsidian Sanctum",
		30452,
		"Tenebron",
		{
			-1 // end of list
		},
		{
			30452, // Tenebron
			-1 // end of list
		},
		{
			-1 // end of list
		},
		{
			30452, // Tenebron
			-1 // end of list
		},
		10
	},
	{
		WOW_TRIGGER_BOSS_SHADRON,
		"The Obsidian Sanctum",
		30451,
		"Shadron",
		{
			-1 // end of list
		},
		{
			30451, // Shadron
			-1 // end of list
		},
		{
			-1 // end of list
		},
		{
			30451, // Shadron
			-1 // end of list
		},
		10
	},

	/*
	 * Naxxramas
	 */

	{
		WOW_TRIGGER_BOSS_ANUBREKHAN,
		"Naxxramas",
		15956,
		"Anub'Rekhan",
		{
			-1 // end of list
		},
		{
			15956, // Anub'Rekhan
			-1 // end of list
		},
		{
			-1 // end of list
		},
		{
			15956, // Anub'Rekhan
			-1 // end of list
		},
		30
	},
	{
		WOW_TRIGGER_BOSS_FAERLINA,
		"Naxxramas",
		15953,
		"Grand Widow Faerlina",
		{
			-1 // end of list
		},
		{
			15953, // Grand Widow Faerlina
			-1 // end of list
		},
		{
			-1 // end of list
		},
		{
			15953, // Grand Widow Faerlina
			-1 // end of list
		},
		30
	},
	{
		WOW_TRIGGER_BOSS_MAEXXNA,
		"Naxxramas",
		15952,
		"Maexxna",
		{
			-1 // end of list
		},
		{
			15952, // Maexxna
			-1 // end of list
		},
		{
			-1 // end of list
		},
		{
			15952, // Maexxna
			-1 // end of list
		},
		30
	},
	{
		WOW_TRIGGER_BOSS_RAZUVIOUS,
		"Naxxramas",
		16061,
		"Instructor Razuvious",
		{
			16803, // Death Knight Understudy
			-1 // end of list
		},
		{
			16061, // Instructor Razuvious
			-1 // end of list
		},
		{
			16803, // Death Knight Understudy
			-1 // end of list
		},
		{
			16061, // Instructor Razuvious
			-1 // end of list
		},
		30
	},
	{
		WOW_TRIGGER_BOSS_GOTHIK,
		"Naxxramas",
		16060,
		"Gothik the Harvester",
		{
			16124, // Unrelenting Trainee
			16125, // Unrelenting Death Knight
			16126, // Unrelenting Rider
			16127, // Spectral Trainee
			16148, // Spectral Death Knight
			16149, // Spectral Horse
			16150, // Spectral Rider
			-1 // end of list
		},
		{
			16060, // Gothik the Harvester
			-1 // end of list
		},
		{
			-1 // end of list
		},
		{
			16060, // 
			-1 // end of list
		},
		30
	},
	{
		WOW_TRIGGER_BOSS_FOURHORSEMEN,
		"Naxxramas",
		16063,
		"The Four Horsemen", // NOTE: Encounter ends when all 4 have died; all 4 are active when encounter starts.
		{
			-1 // end of list
		},
		{
			16063, // Sir Zeliek
			16064, // Thane Korth'azz
			16065, // Lady Blaumeux
			30549, // Baron Rivendare
			-1 // end of list
		},
		{
			-1 // end of list
		},
		{
			16063, // Sir Zeliek
			16064, // Thane Korth'azz
			16065, // Lady Blaumeux
			30549, // Baron Rivendare
			-1 // end of list
		},
		30
	},
	{
		WOW_TRIGGER_BOSS_PATCHWERK,
		"Naxxramas",
		16028,
		"Patchwerk",
		{
			-1 // end of list
		},
		{
			16028, // Patchwerk
			-1 // end of list
		},
		{
			-1 // end of list
		},
		{
			16028, // Patchwerk
			-1 // end of list
		},
		30
	},
	{
		WOW_TRIGGER_BOSS_GROBBULUS,
		"Naxxramas",
		15931,
		"Grobbulus",
		{
			-1 // end of list
		},
		{
			15931, // Grobbulus
			-1 // end of list
		},
		{
			-1 // end of list
		},
		{
			15931, // Grobbulus
			-1 // end of list
		},
		30
	},
	{
		WOW_TRIGGER_BOSS_GLUTH,
		"Naxxramas",
		15933,
		"Gluth",
		{
			-1 // end of list
		},
		{
			15933, // Gluth
			-1 // end of list
		},
		{
			-1 // end of list
		},
		{
			15933, // Gluth
			-1 // end of list
		},
		30
	},
	{
		WOW_TRIGGER_BOSS_THADDIUS,
		"Naxxramas",
		15928,
		"Thaddius", // NOTE: Encounter starts with Stalagg & Feugen; once they die Thaddius awakens.
		{
			-1 // end of list
		},
		{
			15928, // Thaddius
			15929, // Stalagg
			15930, // Feugen
			-1 // end of list
		},
		{
			-1 // end of list
		},
		{
			15928, // Thaddius
			-1 // end of list
		},
		30
	},
	{
		WOW_TRIGGER_BOSS_NOTH,
		"Naxxramas",
		15954,
		"Noth the Plaguebringer",
		{
			-1 // end of list
		},
		{
			15954, // Noth the Plaguebringer
			-1 // end of list
		},
		{
			-1 // end of list
		},
		{
			15954, // Noth the Plaguebringer
			-1 // end of list
		},
		30
	},
	{
		WOW_TRIGGER_BOSS_HEIGAN,
		"Naxxramas",
		15936,
		"Heigan the Unclean",
		{
			-1 // end of list
		},
		{
			15936, // Heigan the Unclean
			-1 // end of list
		},
		{
			-1 // end of list
		},
		{
			15936, // Heigan the Unclean
			-1 // end of list
		},
		30
	},
	{
		WOW_TRIGGER_BOSS_LOATHEB,
		"Naxxramas",
		16011,
		"Loatheb",
		{
			-1 // end of list
		},
		{
			16011, // Loatheb
			-1 // end of list
		},
		{
			-1 // end of list
		},
		{
			16011, // Loatheb
			-1 // end of list
		},
		30
	},
	{
		WOW_TRIGGER_BOSS_SAPPHIRON,
		"Naxxramas",
		15989,
		"Sapphiron",
		{
			-1 // end of list
		},
		{
			15989, // Sapphiron
			-1 // end of list
		},
		{
			-1 // end of list
		},
		{
			15989, // Sapphiron
			-1 // end of list
		},
		45
	},
	{
		WOW_TRIGGER_BOSS_KELTHUZAD,
		"Naxxramas",
		15990,
		"Kel'Thuzad",
		{
			16427, // Soldier of the Frozen Wastes
			16428, // Unstoppable Abomination
			16429, // Soul Weaver
			-1 // end of list
		},
		{
			15990, // Kel'Thuzad
			-1 // end of list
		},
		{
			-1 // end of list
		},
		{
			15990, // Kel'Thuzad
			-1 // end of list
		},
		30
	},

	/*
	 * The Eye of Eternity
	 */

	{
		WOW_TRIGGER_BOSS_MALYGOS,
		"The Eye of Eternity",
		28859,
		"Malygos",
		{
			-1 // end of list
		},
		{
			28859, // Malygos
			-1 // end of list
		},
		{
			-1 // end of list
		},
		{
			28859, // Malygos
			-1 // end of list
		},
		30
	},

	/*
	 * The Vault of Archavon
	 */

	{
		WOW_TRIGGER_BOSS_ARCHAVON,
		"The Vault of Archavon",
		31125,
		"Archavon the Stone Watcher",
		{
			-1 // end of list
		},
		{
			31125, // Archavon the Stone Watcher
			-1 // end of list
		},
		{
			-1 // end of list
		},
		{
			31125, // Archavon the Stone Watcher
			-1 // end of list
		},
		30
	},

	/*
	 * Ulduar 
	 */

    {
        WOW_TRIGGER_BOSS_FLAMELEVIATHAN,		// 1 of 14
        "Ulduar",
		33113, // Vehicle encounter 
        "Flame Leviathan",
		{
			-1 // end of list
		},
        {
            33113, // Flame Leviathan (?) 
			-1 // end of list
        },
        {
            -1 // end of list
        },
        {
            33113, // Flame Leviathan (?) 
            -1 // end of list
        },
		15
    },
    {
        WOW_TRIGGER_BOSS_IGNIS,					// 2 of 14
        "Ulduar",
		33118, // Ignis the Furnace Master
        "Ignis the Furnace Master",
		{
			-1 // end of list
		},
        {
            33118, // Ignis the Furnace Master (?) 
			-1 // end of list
        },
        {
            -1 // end of list
        },
        {
            33118, // Ignis the Furnace Master (?) 
            -1 // end of list
        },
		15
    },
	{
        WOW_TRIGGER_BOSS_RAZORSCALE,			// 3 of 14 (placeholder)
        "Ulduar",
		33186, // Razorscale
        "Razorscale",
		{
			33388, // Dark Rune Gaurdian
			33846, // Dark Rune Sentinel
			33453, // Dark Rune Watcher
			-1 // end of list
		},
        {
            33186, // Razorscale
			-1 // end of list
        },
        {
            -1 // end of list
        },
        {
            33186, // Razorscale
            -1 // end of list
        },
		15
    },
    {
        WOW_TRIGGER_BOSS_XT002,					// 4 of 14
        "Ulduar",
		33293, 
        "XT-002 Deconstructor",
		{
			33343, // XT-002 Minion (?) 
			33344, // XT-002 Minion (?) 
			33346, // XT-002 Minion (?) 
			-1 // end of list
		},
        {
            33293, // XT-002 Deconstructor (?) 
			-1 // end of list
        },
        {
            -1 // end of list
        },
        {
            33293, // XT-002 Deconstructor (?) 
            -1 // end of list
        },
		8
    },
    {
        WOW_TRIGGER_BOSS_IRONCOUNCIL,			// 5 of 14
        "Ulduar",
		32927, // 3 Mob encounter 
        "The Iron Council",
		{
			-1 // end of list
		},
        {
            32927, // IronCouncil 1 (?) 
			32857, // IronCouncil 2 (?)
			32867, // IronCouncil 3 (?)
			-1 // end of list
        },
        {
            -1 // end of list
        },
        {
            32927, // IronCouncil 1 (?) 
			32857, // IronCouncil 2 (?)
			32867, // IronCouncil 3 (?)
            -1 // end of list
        },
		20
    },
    {
        WOW_TRIGGER_BOSS_KOLOGARN,				// 6 of 14
        "Ulduar",
		32930,  
        "Kologarn",
		{
			32933, // Arm 1 (?)
			32934, // Arm 2 (?)
			-1 // end of list
		},
        {
            32930, // Kologarn (?) 
			-1 // end of list
        },
        {
            -1 // end of list
        },
        {
            32930, // Kologarn (?) 
            -1 // end of list
        },
		20
    },
/*	{
        WOW_TRIGGER_BOSS_AURIAYA,				// 7 of 14 (placeholder)
        "Ulduar",
		, 
        "Auriaya",
		{
			-1 // end of list
		},
        {
            , 
			-1 // end of list
        },
        {
            -1 // end of list
        },
        {
            , 
            -1 // end of list
        }
		15
    },*/
    {
        WOW_TRIGGER_BOSS_HODIR,					// 8 of 14
        "Ulduar",
        32845,
        "Hodir",
		{
			32938, // Unknown
			-1 // end of list
		},
        {
            32845, // Hodir (?) 
			-1 // end of list
        },
        {
//          ?????, // Hodir-minion?
//          ?????, // Hodir-minion?
            -1 // end of list
        },
        {
            32845, // Hodir (?)
            -1 // end of list
        },
		20
    },
    {
        WOW_TRIGGER_BOSS_THORIM,				// 9 of 14
        "Ulduar",
        32865,
        "Thorim",
		{
			32876, // Unknown
			32904, // Unknown
			32878, // Unknown
			32877, // Unknown
			-1 // end of list
		},
        {
            32865, // Thorim (?) 
			-1 // end of list
        },
        {
            -1 // end of list
        },
        {
            32865, // Thorim (?)
            -1 // end of list
        },
		20
    },
    {
        WOW_TRIGGER_BOSS_FREYA,					// 10 of 14
        "Ulduar",
        32906,
        "Freya",
		{
			33202, // Freya minion?
			33203, // Freya minion?
			32918, // Freya minion?
			33228, // Freya minion?
			33215, // Freya minion?
			32916, // Freya minion?
			32919, // Freya minion?
			-1 // end of list
		},
        {
            32906, // Freya (?) 
			-1 // end of list
        },
        {
            -1 // end of list
        },
        {
            32906, // Freya (?)
            -1 // end of list
        },
		15
    },
/*	{
        WOW_TRIGGER_BOSS_MIMIRON,				// 11 of 14 (placeholder)
        "Ulduar",
		, 
        "Mimiron",
		{
			-1 // end of list
		},
        {
            , 
			-1 // end of list
        },
        {
            -1 // end of list
        },
        {
            , 
            -1 // end of list
        },
		15
    },*/
    {
        WOW_TRIGGER_BOSS_VEZAX,					// 12 of 14
        "Ulduar",
        33271,
        "General Vezax",
		{
			33524, // Vezax minion?
			-1 // end of list
		},
        {
            33271, // General Vezax (?) 
			-1 // end of list
        },
        {
            -1 // end of list
        },
        {
            33271, // General Vezax (?) 
            -1 // end of list
        },
		25
    },
/*	{
        WOW_TRIGGER_BOSS_YOGGSARON,				// 13 of 14 (placeholder)
        "Ulduar",
		, 
        "Yogg-Saron",
		{
			-1 // end of list
		},
        {
            , 
			-1 // end of list
        },
        {
            -1 // end of list
        },
        {
            , 
            -1 // end of list
        },
		15
    },
	{
        WOW_TRIGGER_BOSS_ALGALON,				// 14 of 14 (placeholder)
        "Ulduar",
		, 
        "Algalon the Observer",
		{
			-1 // end of list
		},
        {
            , 
			-1 // end of list
        },
        {
            -1 // end of list
        },
        {
            , 
            -1 // end of list
        },
		15
    },*/

	/*
	 * end of trigger list
	 */

	{
		WOW_TRIGGER_END_OF_LIST,
		NULL,
		-1,
		NULL,
		{
			-1 // end of list
		},
		{
			-1 // end of list
		},
		{
			-1 // end of list
		},
		{
			-1 // end of list
		},
		-1
	}
};

// define trigger private functions

static WowTriggerList *newTriggerBoss(int listId)
{
	WowTriggerList *result = NULL;

	result = (WowTriggerList *)malloc(sizeof(WowTriggerList));

	result->id = WOW_TRIGGERS[listId].id;
	result->zone = strdup(WOW_TRIGGERS[listId].zone);
	result->bossId = WOW_TRIGGERS[listId].bossId;
	result->bossName = strdup(WOW_TRIGGERS[listId].bossName);

	memcpy((void *)&result->startTrashList,
			(void *)&WOW_TRIGGERS[listId].startTrashList,
			(sizeof(int) * TRIGGER_MAX_BOSS_LENGTH));
	memcpy((void *)&result->startList,
			(void *)&WOW_TRIGGERS[listId].startList,
			(sizeof(int) * TRIGGER_MAX_BOSS_LENGTH));
	memcpy((void *)&result->childBossList,
			(void *)&WOW_TRIGGERS[listId].childBossList,
			(sizeof(int) * TRIGGER_MAX_BOSS_LENGTH));
	memcpy((void *)&result->endList,
			(void *)&WOW_TRIGGERS[listId].endList,
			(sizeof(int) * TRIGGER_MAX_BOSS_LENGTH));

	result->timeout = WOW_TRIGGERS[listId].timeout;

	return result;
}

static void freeTriggerBoss(void *memory)
{
	WowTriggerList *boss = NULL;

	if(memory == NULL) {
		DISPLAY_INVALID_ARGS;
	}

	boss = (WowTriggerList *)memory;

	free(boss->zone);
	free(boss->bossName);
	free(boss);
}

static int buildBossIndex(WowTrigger *trigger)
{
	int ii = 0;
	int nn = 0;

	WowTriggerList *boss = NULL;

	if(bptree_init(&trigger->bossIndex) < 0) {
		return -1;
	}

	if(bptree_setFreeFunction(&trigger->bossIndex, freeTriggerBoss) < 0) {
		return -1;
	}

	for(ii = 0; WOW_TRIGGERS[ii].id != WOW_TRIGGER_END_OF_LIST; ii++) {
		for(nn = 0; WOW_TRIGGERS[ii].startTrashList[nn] != -1; nn++) {
			boss = newTriggerBoss(ii);
			if(bptree_put(&trigger->bossIndex,
						((char *)&(boss->startTrashList[nn])),
						SIZEOF_INT,
						(void *)boss) < 0) {
				return -1;
			}
		}

		for(nn = 0; WOW_TRIGGERS[ii].startList[nn] != -1; nn++) {
			boss = newTriggerBoss(ii);
			if(bptree_put(&trigger->bossIndex,
						((char *)&(boss->startList[nn])),
						SIZEOF_INT,
						(void *)boss) < 0) {
				return -1;
			}
		}
	}

	return 0;
}

static int uidToUnitId(char *uid)
{
	int ii = 0;
	int nn = 0;
	unsigned int unitId = 0;
	char buffer[32];

	buffer[0] = '0';
	buffer[1] = 'x';

	for(ii = 5, nn = 2; ii < 12; ii++, nn++) {
		buffer[nn] = uid[ii];
	}
	buffer[nn] = '\0';

	sscanf(buffer, "%x", &unitId);

	return unitId;
}

static WowTriggerList *checkForBossTrigger(WowTrigger *trigger,
		WowEvent *event, aboolean *isSource, int *bossId, char **uid)
{
	int unitId = 0;
	WowTriggerList *result = NULL;

	*isSource = afalse;

	unitId = uidToUnitId(event->sourceUid);
	if(bptree_get(&trigger->bossIndex, ((char *)&unitId), SIZEOF_INT,
				(void *)&result) == 0) {
		*isSource = atrue;
		*bossId = unitId;
		*uid = event->sourceUid;
		return result;
	}

	unitId = uidToUnitId(event->targetUid);
	if(bptree_get(&trigger->bossIndex, ((char *)&unitId), SIZEOF_INT,
				(void *)&result) == 0) {
		*bossId = unitId;
		*uid = event->targetUid;
		return result;
	}

	return result;
}

static void resetBossUidList(WowTrigger *trigger)
{
	int ii = 0;

	for(ii = 0; ii < TRIGGER_MAX_BOSS_LENGTH; ii++) {
		if(trigger->bossUidList[ii] != NULL) {
			free(trigger->bossUidList[ii]);
		}
		trigger->bossUidList[ii] = NULL;
	}
}

static aboolean isSpellInIngoreList(WowEvent *event)
{
	int ii = 0;

	WowEventSpellCastSuccess *value = NULL;

	if(event->eventId != WOW_EVENT_SPELL_CAST_SUCCESS) {
		return afalse;
	}

	value = (WowEventSpellCastSuccess *)event->eventValue;

	for(ii = 0; WOW_TRIGGER_IGNORE_SPELL_LIST[ii].id != -1; ii++) {
		if(value->spellId == WOW_TRIGGER_IGNORE_SPELL_LIST[ii].id) {
			return atrue;
		}
	}

	return afalse;
}

static aboolean isSpellInOutOfCombatList(WowEvent *event)
{
	int ii = 0;

	WowEventSpellAuraApplied *auraValue = NULL;
	WowEventSpellResurrect *resurrectValue = NULL;

	if((event->eventId != WOW_EVENT_SPELL_AURA_APPLIED) &&
			(event->eventId != WOW_EVENT_SPELL_RESURRECT)) {
		return afalse;
	}

	auraValue = (WowEventSpellAuraApplied *)event->eventValue;

	for(ii = 0; WOW_TRIGGER_OUT_OF_COMBAT_AURA_SPELL_LIST[ii].id != -1; ii++) {
		if(auraValue->spellId ==
				WOW_TRIGGER_OUT_OF_COMBAT_AURA_SPELL_LIST[ii].id) {
			return atrue;
		}
	}

	resurrectValue = (WowEventSpellResurrect *)event->eventValue;

	for(ii = 0;
			WOW_TRIGGER_OUT_OF_COMBAT_RESURRECT_SPELL_LIST[ii].id != -1;
			ii++) {
		if(resurrectValue->spellId ==
				WOW_TRIGGER_OUT_OF_COMBAT_RESURRECT_SPELL_LIST[ii].id) {
			return atrue;
		}
	}


	return afalse;
}

// define wowstats trigger system public functions

int wowstats_triggerInit(WowTrigger *trigger)
{
	int ii = 0;

	if(trigger == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	memset(trigger, 0, sizeof(WowTrigger));

	trigger->analyzeMode = afalse;
	trigger->hasNotedTimestamp = afalse;
	trigger->hasBossTrigger = afalse;
	trigger->bossDeathCount = 0;

	for(ii = 0; ii < TRIGGER_MAX_BOSS_LENGTH; ii++) {
		trigger->bossUidList[ii] = NULL;
	}

	trigger->currentBoss = NULL;

	if(wowstats_timestampInit(&trigger->startTimestamp, 0) < 0) {
		return -1;
	}

	if(wowstats_timestampInit(&trigger->endTimestamp, 0) < 0) {
		return -1;
	}

	if(wowstats_timestampInit(&trigger->lastTriggerTimestamp, 0) < 0) {
		return -1;
	}

	if(buildBossIndex(trigger) < 0) {
		return -1;
	}

	return 0;
}

int wowstats_triggerFree(WowTrigger *trigger)
{
	int ii = 0;

	if(trigger == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	for(ii = 0; ii < TRIGGER_MAX_BOSS_LENGTH; ii++) {
		if(trigger->bossUidList[ii] != NULL) {
			free(trigger->bossUidList[ii]);
		}
	}

	if(bptree_free(&trigger->bossIndex) < 0) {
		return -1;
	}

	memset(trigger, 0, sizeof(WowTrigger));

	return 0;
}

int wowstats_triggerNoteEvent(WowTrigger *trigger,
		WowPlayerIndex *playerIndex, WowEvent *event,
		WowTriggerResult *triggerResult)
{
	aboolean foundNewBoss = afalse;
	aboolean foundCurrentBoss = afalse;
	aboolean isTrashBoss = afalse;
	aboolean isFoundBossSource = afalse;
	aboolean isEndOfBoss = afalse;
	int ii = 0;
	int bossId = 0;
	int deathCounter = 0;
	int elapsedSeconds = 0;
	char timestamp[32];
	char *bossUid = NULL;

	WowTriggerList *foundBoss = NULL;

	if((trigger == NULL) || (playerIndex == NULL) || (event == NULL) ||
			(triggerResult == NULL)) {
		DISPLAY_INVALID_ARGS;
		return WOW_TRIGGER_RESULT_ERROR;
	}

	switch(event->eventId) {
		case WOW_EVENT_ENCHANT_REMOVED:
		case WOW_EVENT_SPELL_AURA_BROKEN:
		case WOW_EVENT_SPELL_AURA_BROKEN_SPELL:
		case WOW_EVENT_SPELL_AURA_REMOVED:
		case WOW_EVENT_SPELL_AURA_REMOVED_DOSE:
			return WOW_TRIGGER_RESULT_OK;
	}

	if(!trigger->hasNotedTimestamp) {
		if(wowstats_timestampClone(&trigger->startTimestamp,
					&event->timestamp) < 0) {
			return WOW_TRIGGER_RESULT_ERROR;
		}
		if(wowstats_timestampClone(&trigger->lastTriggerTimestamp,
					&event->timestamp) < 0) {
			return WOW_TRIGGER_RESULT_ERROR;
		}
		trigger->hasNotedTimestamp = atrue;
	}

	if(wowstats_timestampCalculateDifferenceInSeconds(
				&event->timestamp,
				&trigger->lastTriggerTimestamp,
				&elapsedSeconds) < 0) {
		return WOW_TRIGGER_RESULT_ERROR;
	}

	if(elapsedSeconds < 60) {
		return WOW_TRIGGER_RESULT_OK;
	}

	foundBoss = checkForBossTrigger(trigger, event, &isFoundBossSource,
			&bossId, &bossUid);
	if(foundBoss != NULL) {
		if(isSpellInIngoreList(event)) {
			return WOW_TRIGGER_RESULT_OK;
		}

		if(trigger->hasBossTrigger) {
			foundCurrentBoss = afalse;
			for(ii = 0; trigger->currentBoss->startList[ii] != -1; ii++) {
				if(bossId == trigger->currentBoss->startList[ii]) {
					if(trigger->bossUidList[ii] == NULL) {
						foundCurrentBoss = atrue;
						trigger->bossUidList[ii] = strdup(bossUid);
						if(trigger->analyzeMode) {
							printf("[AnalyzeTrigger] added GUID[%i] '%s' for "
									"ID #%i\n", ii, trigger->bossUidList[ii],
									trigger->currentBoss->startList[ii]);
						}
					}
					else if(!strcmp(trigger->bossUidList[ii], bossUid)) {
						foundCurrentBoss = atrue;
					}
					else if(trigger->analyzeMode) {
						printf("[AnalyzeTrigger] detected GUID[%i] change "
								"from '%s' to '%s' for ID #%i\n",
								ii,
								trigger->bossUidList[ii],
								bossUid,
								bossId);
					}
					break;
				}
			}

			if(!foundCurrentBoss) {
				for(ii = 0;
						trigger->currentBoss->startTrashList[ii] != -1; ii++) {
					if(bossId == trigger->currentBoss->startTrashList[ii]) {
						foundCurrentBoss = atrue;
						isTrashBoss = atrue;
					}
				}
			}

			if(!foundCurrentBoss) {
				foundNewBoss = atrue;

				for(ii = 0;
						trigger->currentBoss->childBossList[ii] != -1;
						ii++) {
					if(bossId ==
							trigger->currentBoss->childBossList[ii]) {
						foundNewBoss = afalse;
						break;
					}
				}

				if(foundNewBoss) {
					// end boss

					if(wowstats_timestampClone(&trigger->endTimestamp,
								&event->timestamp) < 0) {
						return WOW_TRIGGER_RESULT_ERROR;
					}

					if(wowstats_timestampClone(&trigger->lastTriggerTimestamp,
								&event->timestamp) < 0) {
						return WOW_TRIGGER_RESULT_ERROR;
					}

					triggerResult->zone = trigger->currentBoss->zone;
					triggerResult->bossName = trigger->currentBoss->bossName;

					if(wowstats_timestampClone(&triggerResult->startTimestamp,
								&trigger->startTimestamp) < 0) {
						return WOW_TRIGGER_RESULT_ERROR;
					}
					if(wowstats_timestampClone(&triggerResult->endTimestamp,
								&trigger->endTimestamp) < 0) {
						return WOW_TRIGGER_RESULT_ERROR;
					}

					if(trigger->analyzeMode) {
						wowstats_timestampToString(&event->timestamp,
								timestamp);
						printf("[AnalyzeTrigger] found new/end boss #%i, uid "
								"'%s', name '%s' with %i of %i living "
								"players at '%s'\n",
								bossId,
								bossUid,
								foundBoss->bossName,
								playerIndex->alivePlayerCount,
								playerIndex->currentPlayerCount,
								timestamp);
					}

					resetBossUidList(trigger);

					// new boss

					trigger->hasBossTrigger = atrue;
					trigger->bossDeathCount = 0;
					trigger->currentBoss = foundBoss;

					for(ii = 0;
							trigger->currentBoss->startList[ii] != -1; ii++) {
						if(bossId == trigger->currentBoss->startList[ii]) {
							trigger->bossUidList[ii] = strdup(bossUid);
							if(trigger->analyzeMode) {
								printf("[AnalyzeTrigger] added GUID[%i] '%s' "
										"for ID #%i\n", ii,
										trigger->bossUidList[ii],
										trigger->currentBoss->startList[ii]);
							}
							break;
						}
					}

					if(wowstats_timestampClone(&trigger->startTimestamp,
								&event->timestamp) < 0) {
						return WOW_TRIGGER_RESULT_ERROR;
					}
					if(wowstats_timestampInit(&trigger->endTimestamp, 0) < 0) {
						return WOW_TRIGGER_RESULT_ERROR;
					}

					return WOW_TRIGGER_RESULT_NEW_AND_END_BOSS;
				}
			}
		}
		else {
			if(trigger->analyzeMode) {
				wowstats_timestampToString(&event->timestamp, timestamp);
				printf("[AnalyzeTrigger] found new boss #%i, uid '%s', "
						"name '%s' at '%s'\n", bossId, bossUid,
						foundBoss->bossName, timestamp);
			}

			// new boss

			trigger->hasBossTrigger = atrue;
			trigger->bossDeathCount = 0;
			trigger->currentBoss = foundBoss;

			for(ii = 0; trigger->currentBoss->startList[ii] != -1; ii++) {
				if(bossId == trigger->currentBoss->startList[ii]) {
					trigger->bossUidList[ii] = strdup(bossUid);
					if(trigger->analyzeMode) {
						printf("[AnalyzeTrigger] added GUID[%i] '%s' for "
								"ID #%i\n", ii, trigger->bossUidList[ii],
								trigger->currentBoss->startList[ii]);
					}
					break;
				}
			}

			if(wowstats_timestampClone(&trigger->startTimestamp,
						&event->timestamp) < 0) {
				return WOW_TRIGGER_RESULT_ERROR;
			}
			if(wowstats_timestampInit(&trigger->endTimestamp, 0) < 0) {
				return WOW_TRIGGER_RESULT_ERROR;
			}

			triggerResult->zone = foundBoss->zone;
			triggerResult->bossName = foundBoss->bossName;

			return WOW_TRIGGER_RESULT_NEW_BOSS;
		}
	}

	// boss-related state checks

	if(!trigger->hasBossTrigger) {
		return WOW_TRIGGER_RESULT_OK;
	}

	// did the boss die

	if((foundCurrentBoss) &&
			(event->eventId == WOW_EVENT_UNIT_DIED) &&
			(!isFoundBossSource)) {
		deathCounter = 0;
		for(ii = 0; trigger->currentBoss->endList[ii] != -1; ii++) {
			if(bossId == trigger->currentBoss->endList[ii]) {
				trigger->bossDeathCount += 1;
				if(trigger->analyzeMode) {
					wowstats_timestampToString(&event->timestamp, timestamp);
					printf("[AnalyzeTrigger] found DEATH { boss #%i, "
							"uid '%s', name '%s' with +%i } at %s\n",
							bossId,
							bossUid,
							trigger->currentBoss->bossName,
							trigger->bossDeathCount,
							timestamp);
				}
			}
			deathCounter++;
		}

		if(trigger->bossDeathCount >= deathCounter) {
			if(trigger->analyzeMode) {
				wowstats_timestampToString(&event->timestamp, timestamp);
				printf("[AnalyzeTrigger] state change [DIED] { boss #%i, "
						"uid '%s', name '%s' with %i vs %i } at %s\n",
						bossId,
						bossUid,
						trigger->currentBoss->bossName,
						trigger->bossDeathCount,
						deathCounter,
						timestamp);
			}

			// end boss

			if(wowstats_timestampClone(&trigger->endTimestamp,
						&event->timestamp) < 0) {
				return WOW_TRIGGER_RESULT_ERROR;
			}

			if(wowstats_timestampClone(&trigger->lastTriggerTimestamp,
						&event->timestamp) < 0) {
				return WOW_TRIGGER_RESULT_ERROR;
			}

			triggerResult->zone = trigger->currentBoss->zone;
			triggerResult->bossName = trigger->currentBoss->bossName;

			if(wowstats_timestampClone(&triggerResult->startTimestamp,
						&trigger->startTimestamp) < 0) {
				return WOW_TRIGGER_RESULT_ERROR;
			}
			if(wowstats_timestampClone(&triggerResult->endTimestamp,
						&trigger->endTimestamp) < 0) {
				return WOW_TRIGGER_RESULT_ERROR;
			}

			trigger->hasNotedTimestamp = afalse;
			trigger->hasBossTrigger = afalse;
			trigger->bossDeathCount = 0;
			trigger->currentBoss = NULL;

			if(wowstats_timestampInit(&trigger->startTimestamp, 0) < 0) {
				return WOW_TRIGGER_RESULT_ERROR;
			}
			if(wowstats_timestampInit(&trigger->endTimestamp, 0) < 0) {
				return WOW_TRIGGER_RESULT_ERROR;
			}

			resetBossUidList(trigger);

			return WOW_TRIGGER_RESULT_END_BOSS;
		}
	}

	// did all players die

	if(!isEndOfBoss) {
		if((playerIndex->currentPlayerCount > 0) &&
				(playerIndex->alivePlayerCount < 1)) {
			isEndOfBoss = atrue;

			if(wowstats_timestampClone(&trigger->endTimestamp,
						&event->timestamp) < 0) {
				return WOW_TRIGGER_RESULT_ERROR;
			}

			if(trigger->analyzeMode) {
				wowstats_timestampToString(&event->timestamp, timestamp);
				printf("[AnalyzeTrigger] state change [ALL PLAYERS DIED] { "
						"boss #%i, uid '%s', name '%s' with %i of %i } "
						"at %s\n",
						bossId,
						bossUid,
						trigger->currentBoss->bossName,
						playerIndex->alivePlayerCount,
						playerIndex->currentPlayerCount,
						timestamp);
			}
		}
	}

	// players in non-combat event

	if(isSpellInOutOfCombatList(event)) {
		isEndOfBoss = atrue;

		if(wowstats_timestampClone(&trigger->endTimestamp,
					&event->timestamp) < 0) {
			return WOW_TRIGGER_RESULT_ERROR;
		}

		if(trigger->analyzeMode) {
			wowstats_timestampToString(&event->timestamp, timestamp);
			printf("[AnalyzeTrigger] state change [NOT IN COMBAT] { "
					"boss #%i, uid '%s', name '%s' } at %s\n",
					bossId,
					bossUid,
					trigger->currentBoss->bossName,
					timestamp);
		}
	}

	// too much time has elapsed

	if(!isEndOfBoss) {
		if(wowstats_timestampCalculateDifferenceInSeconds(
					&event->timestamp,
					&trigger->startTimestamp,
					&elapsedSeconds) < 0) {
			return WOW_TRIGGER_RESULT_ERROR;
		}

		if((elapsedSeconds / 60) >= trigger->currentBoss->timeout) {
			if(trigger->analyzeMode) {
				wowstats_timestampToString(&event->timestamp, timestamp);
				printf("[AnalyzeTrigger] state change [OUT OF TIME] { "
						"boss #%i, uid '%s', name '%s'  with %i vs %i } "
						"at %s\n",
						bossId,
						bossUid,
						trigger->currentBoss->bossName,
						(elapsedSeconds / 60),
						trigger->currentBoss->timeout,
						timestamp);
			}

			isEndOfBoss = atrue;
		}
		else {
			if(wowstats_timestampClone(&trigger->endTimestamp,
						&event->timestamp) < 0) {
				return WOW_TRIGGER_RESULT_ERROR;
			}
		}
	}

	// end boss

	if(isEndOfBoss) {
		if(wowstats_timestampClone(&trigger->lastTriggerTimestamp,
					&event->timestamp) < 0) {
			return WOW_TRIGGER_RESULT_ERROR;
		}

		triggerResult->zone = trigger->currentBoss->zone;
		triggerResult->bossName = trigger->currentBoss->bossName;

		if(wowstats_timestampClone(&triggerResult->startTimestamp,
					&trigger->startTimestamp) < 0) {
			return WOW_TRIGGER_RESULT_ERROR;
		}
		if(wowstats_timestampClone(&triggerResult->endTimestamp,
					&trigger->endTimestamp) < 0) {
			return WOW_TRIGGER_RESULT_ERROR;
		}

		trigger->hasNotedTimestamp = afalse;
		trigger->hasBossTrigger = afalse;
		trigger->bossDeathCount = 0;
		trigger->currentBoss = NULL;

		if(wowstats_timestampInit(&trigger->startTimestamp, 0) < 0) {
			return WOW_TRIGGER_RESULT_ERROR;
		}
		if(wowstats_timestampInit(&trigger->endTimestamp, 0) < 0) {
			return WOW_TRIGGER_RESULT_ERROR;
		}

		resetBossUidList(trigger);

		return WOW_TRIGGER_RESULT_END_BOSS;
	}

	wowstats_playerIndexNoteEvent(playerIndex, event);

	return WOW_TRIGGER_RESULT_OK;
}

