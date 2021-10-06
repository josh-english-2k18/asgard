/*
 * development.sql
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The development-level SQL for the WoW-CPR website database.
 *
 * Written by Josh English.
 */

-------------------------------------------------------------------------------
--- user table related sql
-------------------------------------------------------------------------------

--- create a new user

insert into cprUser (name, password, emailAddress) values ('TestUserName', 'TestUserPassword', 'TestUserEmailAddress');

--- obtain a user's id from name/password

select uid from cprUser where name = 'UserName' and password = 'UserPassword';

--- obtain a user's id from email-address

select uid from cprUser where emailAddress = 'UserEmailAddress';


-------------------------------------------------------------------------------
--- log history table related sql
-------------------------------------------------------------------------------

--- create a new log history

insert into cprLogHistory (userId, realmId, type, filename, description) values (1, 1, 'Raid', 'FileName', 'This is a test entry');
insert into cprLogHistory (userId, realmId, type, filename, description, dateCreated) values (1, 1, 'Raid', 'FileName', 'This is a test entry', '2009-03-19 12:59:27');

--- create a new contributing log

insert into cprContributingLog (logHistoryId, userId, realmId, type, filename, description) values (1, 1, 1, 'Raid', 'SupportingFileName', 'This is a test entry of a supporting combat log');

--- obtain the uid of the last inserted log history record

select currval('cprloghistory_uid_seq') from cprLogHistory limit 1;

--- obtain the uid of the last inserted contributing log

select currval('cprcontributinglog_uid_seq') from cprContributingLog limit 1;


-------------------------------------------------------------------------------
--- realm table related sql
-------------------------------------------------------------------------------

--- create a new realm

insert into cprRealm (region, name, language, type) values ('US', 'Aegwynn', 'English', 'PvP');

--- lookup a realm

select uid from cprRealm where region = 'US' and name = 'Trollbane';


-------------------------------------------------------------------------------
--- guild table related sql
-------------------------------------------------------------------------------

--- create a new guild

insert into cprGuild (userId, realmId, name) values (2, 1, 'Josh''s Test Guild');

--- lookup a guild

select uid from cprGuild where userId = 1 and realmId = 1;

select uid from cprGuild where name = 'Josh''s Test Guild';

--- add an entity to a guild

update cprEntity set guildId = 1 where uid = 1;

update cprEntity set guildId = 1 where userId = 2;


-------------------------------------------------------------------------------
--- entity table related sql
-------------------------------------------------------------------------------

--- lookup an entity

select uid from cprEntity where realmId = 1 and guid = 'GUID';

--- create a new entity

insert into cprEntity (realmId, className, name, guid, totalPlayedSeconds) values (1, 'Warlock', 'Bob', 'GUID', 123);

--- update an entity's class-name and total-played-seconds

select className, totalPlayedSeconds from cprEntity where uid = 4;
update cprEntity set className = 'CLASS', totalPlayedSeconds = 123 where uid = 4;


-------------------------------------------------------------------------------
--- summary table related sql
-------------------------------------------------------------------------------

--- summary-type record creation

insert into cprSummaryType (name, description) values ('NAME', 'DESCRIPTION');

--- lookup a summary type id

select uid from cprSummaryType where name = 'ShieldDamageSummary';

--- create a summary record

insert into cprSummary (logHistoryId, realmId, type, sourceId, targetId, elapsedTimeSeconds) values (1, 1, 1, 1, 1, 123);

--- obtain the uid of the last inserted summary record

select currval('cprsummary_uid_seq') from cprSummary limit 1;

--- create an 'HealingSummary' record

insert into cprSummaryHealing (summaryId, spellId, spellName, spellRank, directCount, periodicCount, healAmount, overhealAmount, criticalHealAmount, criticalOverhealAmount, criticalCount) values (1, 1, 'SpellName', 1, 1, 1, 1, 1, 1, 1, 1);

--- update an 'HealingSummary' record

update cprSummaryHealing set directCount = 2, periodicCount = 2, healAmount = 2, overhealAmount = 2, criticalHealAmount = 2, criticalOverhealAmount = 2, criticalCount = 2 where summaryId = 1;

--- obtain a specific 'HealingSummary' record summary id

select
	cs.uid
from
	cprSummary cs,
	cprSummaryHealing ah
where
	cs.logHistoryId = 1 and
	cs.realmId = 313 and
	cs.type = 2 and
	cs.sourceId = 1 and
	cs.targetId = 1 and
	ah.spellId = 33110 and
	ah.spellRank = 0 and
	cs.uid = ah.summaryId;

--- obtain a specific 'HealingSummary' record

select
	ah.spellId,
	ah.spellName,
	ah.spellRank,
	ah.directCount,
	ah.periodicCount,
	ah.healAmount,
	ah.overhealAmount,
	ah.criticalHealAmount,
	ah.criticalOverhealAmount,
	ah.criticalCount,
	cs.elapsedTimeSeconds
from
	cprSummary cs,
	cprSummaryHealing ah
where
	cs.logHistoryId = 1 and
	cs.realmId = 313 and
	cs.type = 2 and
	cs.sourceId = 1 and
	cs.targetId = 1 and
	ah.spellId = 33110 and
	ah.spellRank = 0 and
	cs.uid = ah.summaryId;


-------------------------------------------------------------------------------
--- data-access query related sql
-------------------------------------------------------------------------------

--- lookup a user

select
	uid
from
	cprUser
where
	name = 'josh' and
	password = 'josh';

--- determine if a user is the master for a guild

select
	cg.uid,
	cg.name,
	cr.uid,
	cr.region,
	cr.name,
	cr.type
from
	cprGuild cg,
	cprRealm cr
where
	cg.realmId = cr.uid and
	cg.userId = 2;

--- determine if a character is in a guild

select
	cg.uid
from
	cprGuild cg,
	cprEntity ce
where
	cg.uid = ce.guildId and
	ce.userId = 3;

--- obtain the set of characters for a user

select
	ce.uid,
	ce.realmId,
	ce.className,
	ce.name,
	ce.guid,
	ce.totalPlayedSeconds
from
	cprEntity ce,
	cprUser cu
where
	ce.userId = cu.uid and
	cu.uid = 2;

--- obtain the set of log-history records for a guild

select
	clh.uid
from
	cprLogHistory clh,
	cprUser cu,
	cprEntity ce,
	cprGuild cg
where
	clh.userId = cu.uid and
	cu.uid = ce.userId and
	ce.guildId = cg.uid and
	cg.uid = 1;

--- obtain the set of log-history records for a user

select distinct
	cs.logHistoryId
from
	cprUser cu,
	cprEntity ce,
	cprSummary cs
where
	cu.uid = ce.userId and
	((ce.uid = cs.sourceId) or (ce.uid = cs.targetId)) and
	cu.uid = 2;

--- obtain information from the log history

select
	type,
	realmId,
	filename,
	dateCreated
from
	cprLogHistory
where
	uid = 11;

--- obtain the players who were acted upon from a log history

select distinct
	cs.targetId,
	ce.name,
	ce.guid
from
	cprSummary cs,
	cprEntity ce
where
	cs.logHistoryId = 15 and
	cs.targetId = ce.uid;

--- obtain the player who acted from a log history

select distinct
	cs.sourceId,
	ce.name,
	ce.guid
from
	cprSummary cs,
	cprEntity ce
where
	cs.logHistoryId = 15 and
	cs.sourceId = ce.uid;

--- the 'Healing' summary record summary for a given entity

select distinct
	ah.spellId,
	ah.spellName,
	ah.spellRank,
	sum(ah.directCount) as DirectCount,
	sum(ah.periodicCount) as PeriodicCount,
	sum(ah.healAmount) as HealAmount,
	sum(ah.overhealAmount) as OverhealAmount,
	sum(ah.criticalHealAmount) as CriticalHealAmount,
	sum(ah.criticalOverhealAmount) as CriticalOverhealAmount,
	sum(ah.criticalCount) as CriticalCount,
	sum(cs.elapsedTimeSeconds) as ElapsedSeconds
from
	cprEntity ce,
	cprSummaryHealing ah,
	cprSummary cs
where
	ce.uid = cs.targetId and
	cs.uid = ah.summaryId and
	cs.realmId = 239 and
	cs.type = 2 and
	cs.sourceId = 2
group by
	ah.spellId,
	ah.spellName,
	ah.spellRank;

--- the 'Damage' summary record summary for a given combat log

select distinct
	cs.sourceId,
	ce.name,
	sum(sd.damageAmount) as DamageAmount,
	sum(cs.elapsedTimeSeconds) as ElapsedSeconds
from
	cprEntity ce,
	cprSummaryDamage sd,
	cprSummary cs
where
	ce.uid = cs.sourceId and
	cs.type = 2 and
	sd.summaryId = ce.uid and
	cs.logHistoryId = 10
group by
	cs.sourceId,
	ce.name,
	DamageAmount
order by
	DamageAmount
desc
;


--- the 'Healing' summary records for a given entity

select
	ce.name,
	ce.guid,
	ah.spellId,
	ah.spellName,
	ah.spellRank,
	ah.directCount,
	ah.periodicCount,
	ah.healAmount,
	ah.overhealAmount,
	ah.criticalHealAmount,
	ah.criticalOverhealAmount,
	ah.criticalCount,
	cs.elapsedTimeSeconds
from
	cprEntity ce,
	cprSummaryHealing ah,
	cprSummary cs
where
	ce.uid = cs.targetId and
	cs.uid = ah.summaryId and
	cs.realmId = 239 and
	cs.type = 2 and
	cs.sourceId = 2
order by
	ce.name,
	ah.spellName,
	ah.spellRank;

--- the 'Healing' summary records for a given log history

select
	ce.name,
	ce.guid,
	ah.spellId,
	ah.spellName,
	ah.spellRank,
	ah.directCount,
	ah.periodicCount,
	ah.healAmount,
	ah.overhealAmount,
	ah.criticalHealAmount,
	ah.criticalOverhealAmount,
	ah.criticalCount,
	cs.elapsedTimeSeconds
from
	cprSummary cs,
	cprSummaryHealing ah,
	cprEntity ce
where
	cs.logHistoryId = 15 and
	cs.realmId = 239 and
	cs.type = 2 and
	cs.sourceId = 2 and
	cs.uid = ah.summaryId and
	ce.uid = cs.targetId
order by
	ce.name,
	ah.spellName,
	ah.spellRank;

