/*
 * db.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard-WoW Statistical Analysis db package header file.
 *
 * Written by Josh English.
 */

#if !defined(_WOWSTATS_DB_H)

#define _WOWSTATS_DB_H

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// external components

#include "berkeley/db.h"
#include "postgres/libpq-fe.h"

// db components

#include "wowstats/db/bdb.h"
#include "wowstats/db/pgdb.h"


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _WOWSTATS_DB_H

