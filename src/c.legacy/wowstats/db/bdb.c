/*
 * bdb.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard-WoW statistical analysis BerkeleyDB database interface.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#include "wowstats/db.h"
#define _WOWSTATS_DB_COMPONENT
#include "wowstats/db/bdb.h"

// define berkeley-db public functions

int bdb_init(Bdb *bdb, char *databaseName)
{
#if defined(DBFLAG_BERKELEY)
	DB *db = NULL;

	if(bdb == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	memset(bdb, 0, sizeof(Bdb));

	bdb->isOpen = afalse;
	bdb->databaseName = strdup(databaseName);
	bdb->db = NULL;

	if(db_create(&db, NULL, 0) != 0) {
		return -1;
	}

	if(db->set_cachesize(db, 0, 67108864, 1) != 0) {
		return -1;
	}

	if(db->open(db, NULL, databaseName, NULL, DB_BTREE, DB_CREATE,
				0644) != 0) {
		return -1;
	}

	bdb->isOpen = atrue;
	bdb->db = (void *)db;

	return 0;
#else // !DBFLAG_BERKELEY
	DISPLAY_INVALID_ARGS;
	return -1;
#endif // DBFLAG_BERKELEY
}

int bdb_free(Bdb *bdb)
{
#if defined(DBFLAG_BERKELEY)
	DB *db = NULL;

	if(bdb == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(bdb->isOpen) {
		db = (DB *)bdb->db;

		if(db->close(db, 0) != 0) {
			return -1;
		}
	}

	if(bdb->databaseName != NULL) {
		free(bdb->databaseName);
	}

	memset(bdb, 0, sizeof(Bdb));

	return 0;
#else // !DBFLAG_BERKELEY
	DISPLAY_INVALID_ARGS;
	return -1;
#endif // DBFLAG_BERKELEY
}

aboolean bdb_isOpen(Bdb *bdb)
{
#if defined(DBFLAG_BERKELEY)
	if(bdb == NULL) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	return bdb->isOpen;
#else // !DBFLAG_BERKELEY
	DISPLAY_INVALID_ARGS;
	return afalse;
#endif // DBFLAG_BERKELEY
}

int bdb_put(Bdb *bdb, char *key, int keyLength, char *value, int valueLength)
{
#if defined(DBFLAG_BERKELEY)
	DB *db = NULL;
	DBT dbKey;
	DBT dbValue;

	if((bdb == NULL) || (key == NULL) || (keyLength < 1) || (value == NULL) ||
			(valueLength < 1)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(!bdb->isOpen) {
		return -1;
	}

	memset((void *)&dbKey, 0, sizeof(DBT));

	dbKey.data = key;
	dbKey.size = keyLength;

	memset((void *)&dbValue, 0, sizeof(DBT));

	dbValue.data = value;
	dbValue.size = valueLength;

	db = (DB *)bdb->db;

	if(db->put(db, NULL, &dbKey, &dbValue, 0) != 0) {
		return -1;
	}

	return 0;
#else // !DBFLAG_BERKELEY
	DISPLAY_INVALID_ARGS;
	return -1;
#endif // DBFLAG_BERKELEY
}

int bdb_get(Bdb *bdb, char *key, int keyLength, char **value, int *valueLength)
{
#if defined(DBFLAG_BERKELEY)
	DB *db = NULL;
	DBT dbKey;
	DBT dbValue;

	if((bdb == NULL) || (key == NULL) || (keyLength < 1) || (value == NULL) ||
			(valueLength == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(!bdb->isOpen) {
		return -1;
	}

	memset((void *)&dbKey, 0, sizeof(DBT));

	dbKey.data = key;
	dbKey.size = keyLength;

	memset((void *)&dbValue, 0, sizeof(DBT));

	db = (DB *)bdb->db;

	if(db->get(db, NULL, &dbKey, &dbValue, 0) != 0) {
		return -1;
	}

	*value = dbValue.data;
	*valueLength = dbValue.size;

	return 0;
#else // !DBFLAG_BERKELEY
	DISPLAY_INVALID_ARGS;
	return -1;
#endif // DBFLAG_BERKELEY
}

int bdb_del(Bdb *bdb, char *key, int keyLength)
{
#if defined(DBFLAG_BERKELEY)
	DB *db = NULL;
	DBT dbKey;

	if((bdb == NULL) || (key == NULL) || (keyLength < 1)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(!bdb->isOpen) {
		return -1;
	}

	memset((void *)&dbKey, 0, sizeof(DBT));

	dbKey.data = key;
	dbKey.size = keyLength;

	db = (DB *)bdb->db;

	if(db->del(db, NULL, &dbKey, 0) != 0) {
		return -1;
	}

	return 0;
#else // !DBFLAG_BERKELEY
	DISPLAY_INVALID_ARGS;
	return -1;
#endif // DBFLAG_BERKELEY
}

