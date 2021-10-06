/*
 * insertTestData.sql
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The test data for the World of Warcraft (WoW) Combat Performance Reports
 * (CPR) applications.
 *
 * Written by Josh English.
 */

---
--- test users
---

insert into cprUser (name, password, emailAddress) values ('TestUserName', 'TestUserPassword', 'TestUserEmailAddress');
insert into cprUser (name, password, emailAddress) values ('josh', '', 'josh@wowcpr.com');
insert into cprUser (name, password, emailAddress) values ('greg', '', 'greg@wowcpr.com');
insert into cprUser (name, password, emailAddress) values ('andass', '', 'andaas@hossguild.net');

---
--- test guilds
---

insert into cprGuild (realmId, userId, entityId, name) values (313, 4, 1, 'Hoss');

---
--- test entities
---

insert into cprEntity (userId, realmId, guildId, className, name, guid) values (4, 313, 1, 'Priest', 'Firanja', '0x0000000000099DAF');

