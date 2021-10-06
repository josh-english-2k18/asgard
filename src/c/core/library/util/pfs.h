/*
 * pfs.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * A simplified profile-function system (PFS), header file.
 *
 * Written by Josh English.
 */

#if !defined(_CORE_LIBRARY_UTIL_PFS_H)

#define _CORE_LIBRARY_UTIL_PFS_H

#if !defined(_CORE_H) && !defined(_CORE_LIBRARY_H) && \
		!defined(_CORE_LIBRARY_UTIL_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _CORE_H || _CORE_LIBRARY_H || _CORE_LIBRARY_UTIL_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

// define pfs public data types

typedef struct _PfsEntry {
	alint ticks;
	char *file;
	char *function;
} PfsEntry;

typedef struct _Pfs {
	Bptree index;
	Log log;
	Mutex mutex;
} Pfs;

// declare pfs public functions

int pfs_init(Pfs *pfs, aboolean writeLog, char *filename);

int pfs_free(Pfs *pfs);

int pfs_profile(Pfs *pfs, const char *file, const char *function);

int pfs_buildReport(Pfs *pfs, char **report, int *reportLength);

#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _CORE_LIBRARY_UTIL_PFS_H

