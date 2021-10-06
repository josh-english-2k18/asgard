/*
 * bdb.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard-WoW statistical analysis BerkeleyDB database interface, header
 * file.
 *
 * Written by Josh English.
 */

#if !defined(_WOWSTATS_DB_BDB_H)

#define _WOWSTATS_DB_BDB_H

#if !defined(_WOWSTATS_DB_H) && !defined(_WOWSTATS_DB_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _WOWSTATS_DB_H || _WOWSTATS_DB_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

// define berkeley-db public data types

typedef struct _Bdb {
	aboolean isOpen;
	char *databaseName;
	void *db;
} Bdb;

// declare berkeley-db public functions

int bdb_init(Bdb *bdb, char *databaseName);

int bdb_free(Bdb *bdb);

aboolean bdb_isOpen(Bdb *bdb);

int bdb_put(Bdb *bdb, char *key, int keyLength, char *value, int valueLength);

int bdb_get(Bdb *bdb, char *key, int keyLength, char **value, int *valueLength);

int bdb_del(Bdb *bdb, char *key, int keyLength);

#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _WOWSTATS_DB_BDB_H

