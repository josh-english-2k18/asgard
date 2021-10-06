/*
 * system.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard system-commands library, header file.
 *
 * Written by Josh English.
 */

#if !defined(_CORE_SYSTEM_SYSTEM_H)

#define _CORE_SYSTEM_SYSTEM_H

#if !defined(_CORE_H) && !defined(_CORE_SYSTEM_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _CORE_H || _CORE_SYSTEM_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

// delcare system public functions

// file-related functions

int system_fileSetNative(char *filename);

int system_fileExists(char *filename, aboolean *exists);

int system_fileLength(char *filename, alint *length);

int system_fileModifiedTime(char *filename, double *timestamp);

int system_fileDelete(char *filename);

int system_fileMove(char *sourceFilename, char *destFilename);

int system_fileCopy(char *sourceFilename, char *destFilename);

int system_fileExecute(char *filename, char *arguments);

// dir-related functions

int system_dirExists(char *dirname, aboolean *exists);

int system_dirChangeTo(char *dirname);

int system_dirCreate(char *dirname);

int system_dirDelete(char *dirname);

// random-related functions

void system_pickRandomSeed();

void system_setRandomSeed(unsigned int seed);

#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _CORE_SYSTEM_SYSTEM_H

