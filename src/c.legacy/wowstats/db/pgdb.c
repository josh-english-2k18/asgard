/*
 * pgdb.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard-WoW statistical analysis PostgreSQL database interface.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#include "wowstats/db.h"
#define _WOWSTATS_DB_COMPONENT
#include "wowstats/db/pgdb.h"

// define postgres-db public functions

int pgdb_init(Pgdb *pgdb)
{
#if defined(DBFLAG_POSTGRES)
	if(pgdb == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	memset(pgdb, 0, sizeof(Pgdb));

	pgdb->isConnected = afalse;
	pgdb->hostname = NULL;
	pgdb->port = 0;
	pgdb->username = NULL;
	pgdb->password = NULL;
	pgdb->connectionFlags = NULL;
	pgdb->databaseName = NULL;
	pgdb->connection = NULL;

	return 0;
#else // !DBFLAG_POSTGRES
	DISPLAY_INVALID_ARGS;
	return -1;
#endif // DBFLAG_POSTGRES
}

int pgdb_free(Pgdb *pgdb)
{
#if defined(DBFLAG_POSTGRES)
	if(pgdb == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(pgdb->isConnected) {
		pgdb_disconnect(pgdb);
	}

	if(pgdb->hostname != NULL) {
		free(pgdb->hostname);
	}

	if(pgdb->username != NULL) {
		free(pgdb->username);
	}

	if(pgdb->password != NULL) {
		free(pgdb->password);
	}

	if(pgdb->connectionFlags != NULL) {
		free(pgdb->connectionFlags);
	}

	if(pgdb->databaseName != NULL) {
		free(pgdb->databaseName);
	}

	memset(pgdb, 0, sizeof(Pgdb));

	return 0;
#else // !DBFLAG_POSTGRES
	DISPLAY_INVALID_ARGS;
	return -1;
#endif // DBFLAG_POSTGRES
}

int pgdb_setDataSourceProperties(Pgdb *pgdb, char *hostname, int port,
		char *username, char *password, char *connectionFlags,
		char *databaseName)
{
#if defined(DBFLAG_POSTGRES)
	if(pgdb == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(pgdb->isConnected) {
		return -1;
	}

	if(hostname != NULL) {
		pgdb->hostname = strdup(hostname);
	}

	if((port > 0) && (port <= 65536)) {
		pgdb->port = port;
	}

	if(username != NULL) {
		pgdb->username = strdup(username);
	}

	if(password != NULL) {
		pgdb->password = strdup(password);
	}

	if(connectionFlags != NULL) {
		pgdb->connectionFlags = strdup(connectionFlags);
	}

	if(databaseName != NULL) {
		pgdb->databaseName = strdup(databaseName);
	}

	return 0;
#else // !DBFLAG_POSTGRES
	DISPLAY_INVALID_ARGS;
	return -1;
#endif // DBFLAG_POSTGRES
}

int pgdb_connect(Pgdb *pgdb)
{
#if defined(DBFLAG_POSTGRES)
	char *port = NULL;

	if(pgdb == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(pgdb->isConnected) {
		return -1;
	}

	if(pgdb->port != 0) {
		port = (char *)malloc(sizeof(char) * 32);
		snprintf(port, (sizeof(char) * 32), "%i", pgdb->port);
	}

	if((pgdb->username != NULL) && (pgdb->password != NULL)) {
		pgdb->connection = PQsetdbLogin(pgdb->hostname, port,
				pgdb->connectionFlags, NULL, pgdb->databaseName,
				pgdb->username, pgdb->password);
	}
	else {
		pgdb->connection = PQsetdb(pgdb->hostname, port,
				pgdb->connectionFlags, NULL, pgdb->databaseName);
	}

	if(PQstatus((PGconn *)pgdb->connection) == CONNECTION_BAD) {
		PQtrace((PGconn *)pgdb->connection, stderr);
		return -1;
	}

	pgdb->isConnected = atrue;

	return 0;
#else // !DBFLAG_POSTGRES
	DISPLAY_INVALID_ARGS;
	return -1;
#endif // DBFLAG_POSTGRES
}

int pgdb_disconnect(Pgdb *pgdb)
{
#if defined(DBFLAG_POSTGRES)
	if(pgdb == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(!pgdb->isConnected) {
		return -1;
	}

	PQfinish((PGconn *)pgdb->connection);
	pgdb->connection = NULL;

	pgdb->isConnected = afalse;

	return 0;
#else // !DBFLAG_POSTGRES
	DISPLAY_INVALID_ARGS;
	return -1;
#endif // DBFLAG_POSTGRES
}

aboolean pgdb_isConnected(Pgdb *pgdb)
{
#if defined(DBFLAG_POSTGRES)
	if(pgdb == NULL) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	return pgdb->isConnected;
#else // !DBFLAG_POSTGRES
	DISPLAY_INVALID_ARGS;
	return afalse;
#endif // DBFLAG_POSTGRES
}

int pgdb_executeSql(Pgdb *pgdb, char *sql, int successCode, void **handle)
{
#if defined(DBFLAG_POSTGRES)
	PGresult *pgResult = NULL;

	if((pgdb == NULL) || (sql == NULL) || (handle == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(!pgdb->isConnected) {
		return -1;
	}

	*handle = NULL;

	pgResult = PQexec((PGconn *)pgdb->connection, sql);
	if(PQresultStatus(pgResult) != successCode) {
		PQclear(pgResult);
		return -1;
	}

	*handle = pgResult;

	return 0;
#else // !DBFLAG_POSTGRES
	DISPLAY_INVALID_ARGS;
	return -1;
#endif // DBFLAG_POSTGRES
}

int pgdb_getColumnCount(Pgdb *pgdb, void *handle)
{
#if defined(DBFLAG_POSTGRES)
	if((pgdb == NULL) || (handle == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(!pgdb->isConnected) {
		return -1;
	}

	return (int)PQnfields((PGresult *)handle);
#else // !DBFLAG_POSTGRES
	DISPLAY_INVALID_ARGS;
	return -1;
#endif // DBFLAG_POSTGRES
}

int pgdb_getRowCount(Pgdb *pgdb, void *handle)
{
#if defined(DBFLAG_POSTGRES)
	if((pgdb == NULL) || (handle == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(!pgdb->isConnected) {
		return -1;
	}

	return (int)PQntuples((PGresult *)handle);
#else // !DBFLAG_POSTGRES
	DISPLAY_INVALID_ARGS;
	return -1;
#endif // DBFLAG_POSTGRES
}

char *pgdb_getColumnName(Pgdb *pgdb, void *handle, int columnNumber)
{
#if defined(DBFLAG_POSTGRES)
	if((pgdb == NULL) || (handle == NULL)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	if(!pgdb->isConnected) {
		return NULL;
	}

	return (char *)PQfname((PGresult *)handle, columnNumber);
#else // !DBFLAG_POSTGRES
	DISPLAY_INVALID_ARGS;
	return -1;
#endif // DBFLAG_POSTGRES
}

char *pgdb_getRowResult(Pgdb *pgdb, void *handle, int rowNumber,
		int columnNumber, int *resultLength)
{
#if defined(DBFLAG_POSTGRES)
	char *ptr = NULL;
	char *result = NULL;

	if((pgdb == NULL) || (handle == NULL) || (columnNumber < 0) ||
			(rowNumber < 0) || (resultLength == NULL)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	if(!pgdb->isConnected) {
		return NULL;
	}

	*resultLength = 0;

	ptr = (char *)PQgetvalue((PGresult *)handle, rowNumber, columnNumber);
	if(ptr != NULL) {
		result = strdup(ptr);
		*resultLength = strlen(result);
	}

	return result;
#else // !DBFLAG_POSTGRES
	DISPLAY_INVALID_ARGS;
	return -1;
#endif // DBFLAG_POSTGRES
}

int pgdb_freeHandle(Pgdb *pgdb, void *handle)
{
#if defined(DBFLAG_POSTGRES)
	int result = 0;

	if((pgdb == NULL) || (handle == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(!pgdb->isConnected) {
		return -1;
	}

	PQclear((PGresult *)handle);

	return result;
#else // !DBFLAG_POSTGRES
	DISPLAY_INVALID_ARGS;
	return -1;
#endif // DBFLAG_POSTGRES
}

