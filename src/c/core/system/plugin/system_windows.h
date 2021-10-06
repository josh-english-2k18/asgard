/*
 * system_windows.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard system-commands library, Windows plugin, header file.
 *
 * Written by Josh English.
 */

#if !defined(_CORE_SYSTEM_PLUGIN_SYSTEM_WINDOWS_H)

#define _CORE_SYSTEM_PLUGIN_SYSTEM_WINDOWS_H

#if !defined(_CORE_H) && !defined(_CORE_SYSTEM_PLUGIN_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _CORE_H || _CORE_SYSTEM_PLUGIN_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

#if defined(WIN32) && !defined(MINGW)

// delcare system (windows plugin) public functions

// file-related functions

int system_fileSetNativeWindows(char *filename);

int system_fileExistsWindows(char *filename, aboolean *exists);

int system_fileLengthWindows(char *filename, alint *length);

int system_fileModifiedTimeWindows(char *filename, double *timestamp);

int system_fileDeleteWindows(char *filename);

int system_fileMoveWindows(char *sourceFilename, char *destFilename);

int system_fileCopyWindows(char *sourceFilename, char *destFilename);

int system_fileExecuteWindows(char *filename, char *arguments);

// dir-related functions

int system_dirExistsWindows(char *dirname, aboolean *exists);

int system_dirChangeToWindows(char *dirname);

int system_dirCreateWindows(char *dirname);

int system_dirDeleteWindows(char *dirname);

#endif // WIN32 && !MINGW

#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _CORE_SYSTEM_PLUGIN_SYSTEM_WINDOWS_H

