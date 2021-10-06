/*
 * pgdb.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard-WoW statistical analysis PostgreSQL database interface, header
 * file.
 *
 * Written by Josh English.
 */

#if !defined(_WOWSTATS_DB_PGDB_H)

#define _WOWSTATS_DB_PGDB_H

#if !defined(_WOWSTATS_DB_H) && !defined(_WOWSTATS_DB_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _WOWSTATS_DB_H || _WOWSTATS_DB_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

// define postgres-db public data types

typedef struct _Pgdb {
	aboolean isConnected;
	char *hostname;
	int port;
	char *username;
	char *password;
	char *connectionFlags;
	char *databaseName;
	void *connection;
} Pgdb;

// declare postgres-db public functions

int pgdb_init(Pgdb *pgdb);

int pgdb_free(Pgdb *pgdb);

int pgdb_setDataSourceProperties(Pgdb *pgdb, char *hostname, int port,
		char *username, char *password, char *connectionFlags,
		char *databaseName);

int pgdb_connect(Pgdb *pgdb);

int pgdb_disconnect(Pgdb *pgdb);

aboolean pgdb_isConnected(Pgdb *pgdb);

int pgdb_executeSql(Pgdb *pgdb, char *sql, int successCode, void **handle);

int pgdb_getColumnCount(Pgdb *pgdb, void *handle);

int pgdb_getRowCount(Pgdb *pgdb, void *handle);

char *pgdb_getColumnName(Pgdb *pgdb, void *handle, int columnNumber);

char *pgdb_getRowResult(Pgdb *pgdb, void *handle, int rowNumber,
		int columnNumber, int *resultLength);

int pgdb_freeHandle(Pgdb *pgdb, void *handle);

#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _WOWSTATS_DB_PGDB_H

