/*
 * insertSummaryTypes.sql
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The list of summaries for the World of Warcraft (WoW) Combat Performance
 * Reports (CPR) website.
 *
 * Written by Josh English.
 */


insert into cprSummaryType (name, description) values ('Healing', 'The sum of player healing.');
insert into cprSummaryType (name, description) values ('Damage', 'The sum of player-dealt damage.');

