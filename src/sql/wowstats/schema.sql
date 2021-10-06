/*
 * schema.sql
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The database schema for the World of Warcraft (WoW) Combat Performance
 * Reports (CPR) website.
 *
 * Written by Josh English.
 */

---
--- foundation tables
---

drop table if exists cprUser;
create table cprUser
(
	uid						serial,
	name					varchar(32) not null,
	password				varchar(32) not null,
	emailAddress			varchar(64) not null,
	dateCreated				date not null default now(),
	dateModified			date not null default now()
);

create index cprUserNameIndex on cprUser(name);
create index cprUserEmailAddressIndex on cprUser(emailAddress);

---
--- combat log tables
---

drop table if exists cprLogHistory;
create table cprLogHistory
(
	uid						serial,
	parentLogId				integer default 0,
	mobLogId				serial,
	userId					integer default 0,
	realmId					integer default 0,
	type					varchar(32) not null default '',
	filename				varchar(64) not null default '',
	description				varchar(256) not null default '',
	playSeconds				integer default 0,
	dateUploaded			timestamp not null default now(),
	dateCreated				timestamp not null default now(),
	dateModified			timestamp not null default now()
);

create index cprLogHistoryUidIndex on cprLogHistory(uid);
create index cprLogHistoryUserIdIndex on cprLogHistory(userId);

drop table if exists cprContributingLog;
create table cprContributingLog
(
	uid						serial,
	logHistoryId			integer default 0,
	userId					integer default 0,
	realmId					integer default 0,
	type					varchar(32) not null default '',
	filename				varchar(64) not null default '',
	description				varchar(256) not null default '',
	playSeconds				integer default 0,
	dateUploaded			timestamp not null default now(),
	dateCreated				timestamp not null default now(),
	dateModified			timestamp not null default now()
);

create index cprContributingLogUidIndex on cprContributingLog(uid);
create index cprContributingLogLogHistoryIdIndex on cprContributingLog(logHistoryId);
create index cprContributingLogUserIdIndex on cprContributingLog(userId);

---
--- realm tables
---

drop table if exists cprRealm;
create table cprRealm
(
	uid						serial,
	region					char(2) not null,
	name					varchar(64) not null,
	language				varchar(32) not null,
	type					varchar(16) not null,
	dateCreated				date not null default now(),
	dateModified			date not null default now()
);

create index cprRealmNameIndex on cprRealm(name);

---
--- guild tables
---

drop table if exists cprGuild;
create table cprGuild
(
	uid						serial,
	realmId					integer default 0,
	userId					integer default 0,
	entityId				integer default 0,
	name					varchar(64) not null default '',
	dateCreated				date not null default now(),
	dateModified			date not null default now()
);

create index cprGuildUidIndex on cprGuild(uid);
create index cprGuildUserIdIndex on cprGuild(userId);
create index cprGuildRealmIdIndex on cprGuild(realmId);
create index cprGuildNameIndex on cprGuild(name);

---
--- entity tables (player, pet, or mobile-object)
---

drop table if exists cprEntity;
create table cprEntity
(
	uid						serial,
	userId					integer default 0,
	realmId					integer default 0,
	guildId					integer default 0,
	className				varchar(32) not null default 'Unknown',
	name					varchar(64) not null default '',
	guid					varchar(64) not null default 'unknown',
	totalPlayedSeconds		integer default 0,
	dateCreated				timestamp not null default now(),
	dateModified			timestamp not null default now()
);

create index cprEntityUidIndex on cprEntity(uid);
create index cprEntityUserIdIndex on cprEntity(userId);
create index cprEntityRealmIdIndex on cprEntity(realmId);
create index cprEntityNameIndex on cprEntity(name);
create index cprEntityGuidIndex on cprEntity(guid);

drop table if exists cprEntityLog;
create table cprEntityLog
(
	uid						serial,
	entityId				integer default 0,
	logHistoryId			integer default 0,
	totalPlayedSeconds		integer default 0,
	dateCreated				timestamp not null default now(),
	dateModified			timestamp not null default now()
);

create index cprEntityLogUidIndex on cprEntityLog(uid);
create index cprEntityLogEntityIdIndex on cprEntityLog(entityId);
create index cprEntityLogLogHistoryIdIndex on cprEntityLog(logHistoryId);

---
--- summary tables
---

drop table if exists cprSummaryType;
create table cprSummaryType
(
	uid						serial,
	name					varchar(64) not null,
	description				varchar(128) not null,
	dateCreated				date not null default now(),
	dateModified			date not null default now()
);

create index cprSummaryTypeNameIndex on cprSummaryType(name);

drop table if exists cprSummary;
create table cprSummary
(
	uid						serial,
	logHistoryId			integer default 0,
	realmId					integer default 0,
	type					integer default 0,
	sourceId				integer default 0,
	targetId				integer default 0,
	elapsedTimeSeconds		integer default 0,
	dateCreated				timestamp not null default now(),
	dateModified			timestamp not null default now()
);

create index cprSummaryLogHistoryIdIndex on cprSummary(logHistoryId);
create index cprSummaryRealmIdIndex on cprSummary(realmId);
create index cprSummaryTypeIndex on cprSummary(type);
create index cprSummarySourceIdIndex on cprSummary(sourceId);
create index cprSummaryTargetIdIndex on cprSummary(targetId);

drop table if exists cprSummaryHealing;
create table cprSummaryHealing
(
	uid						serial,
	summaryId				integer default 0,
	spellId					integer default 0,
	spellName				varchar(64) not null default '',
	spellSchool				varchar(64) not null default '',
	spellRank				integer default 0,
	directCount				integer default 0,
	periodicCount			integer default 0,
	healAmount				integer default 0,
	overhealAmount			integer default 0,
	criticalHealAmount		integer default 0,
	criticalOverhealAmount	integer default 0,
	criticalCount			integer default 0
);

create index cprSummaryHealingSummaryIdIndex on cprSummaryHealing(summaryId);

drop table if exists cprSummaryDamage;
create table cprSummaryDamage
(
	uid						serial,
	summaryId				integer default 0,
	spellId					integer default 0,
	spellName				varchar(64) not null default '',
	spellSchool				varchar(64) not null default '',
	spellRank				integer default 0,
	damageType				varchar(64) not null default '',
	directCount				integer default 0,
	periodicCount			integer default 0,
	damageAmount			integer default 0,
	overkillAmount			integer default 0,
	resistAmount			integer default 0,
	blockAmount				integer default 0,
	missedCount				integer default 0,
	missedAmount			integer default 0,
	absorbedCount			integer default 0,
	absorbedDamageAmount	integer default 0,
	absorbedOverkillAmount	integer default 0,
	absorbedResistAmount	integer default 0,
	absorbedBlockAmount		integer default 0,
	criticalCount			integer default 0,
	criticalDamageAmount	integer default 0,
	criticalOverkillAmount	integer default 0,
	criticalResistAmount	integer default 0,
	criticalBlockAmount		integer default 0,
	glancingCount			integer default 0,
	glancingDamageAmount	integer default 0,
	glancingOverkillAmount	integer default 0,
	glancingResistAmount	integer default 0,
	glancingBlockAmount		integer default 0,
	crushingCount			integer default 0,
	crushingDamageAmount	integer default 0,
	crushingOverkillAmount	integer default 0,
	crushingResistAmount	integer default 0,
	crushingBlockAmount		integer default 0
);

create index cprSummaryDamageSummaryIdIndex on cprSummaryDamage(summaryId);

