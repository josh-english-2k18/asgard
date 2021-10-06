/*
 * system_linux.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard system-commands library, Linux plugin, header file.
 *
 * Written by Josh English.
 */

#if !defined(_CORE_SYSTEM_PLUGIN_SYSTEM_LINUX_H)

#define _CORE_SYSTEM_PLUGIN_SYSTEM_LINUX_H

#if !defined(_CORE_H) && !defined(_CORE_SYSTEM_PLUGIN_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _CORE_H || _CORE_SYSTEM_PLUGIN_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


#if defined(__linux__) || defined(__APPLE__) || defined(MINGW)


// delcare system (linux plugin) public functions

// file-related functions

int system_fileSetNativeLinux(char *filename);

int system_fileExistsLinux(char *filename, aboolean *exists);

int system_fileLengthLinux(char *filename, alint *length);

int system_fileModifiedTimeLinux(char *filename, double *timestamp);

int system_fileDeleteLinux(char *filename);

int system_fileMoveLinux(char *sourceFilename, char *destFilename);

int system_fileCopyLinux(char *sourceFilename, char *destFilename);

int system_fileExecuteLinux(char *filename, char *arguments);

// dir-related functions

int system_dirExistsLinux(char *dirname, aboolean *exists);

int system_dirChangeToLinux(char *dirname);

int system_dirCreateLinux(char *dirname);

int system_dirDeleteLinux(char *dirname);


#endif // __linux__ || __APPLE__ || MINGW


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _CORE_SYSTEM_PLUGIN_SYSTEM_LINUX_H

