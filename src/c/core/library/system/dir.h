/*
 * dir.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard core library dir-handling functions, header file.
 *
 * Written by Josh English.
 */

#if !defined(_CORE_LIBRARY_SYSTEM_DIR_H)

#define _CORE_LIBRARY_SYSTEM_DIR_H

#if !defined(_CORE_H) && !defined(_CORE_LIBRARY_H) && \
		!defined(_CORE_LIBRARY_SYSTEM_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _CORE_H || _CORE_LIBRARY_H || _CORE_LIBRARY_SYSTEM_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

// delcare dir-handling public data types

typedef struct _DirHandle {
	char *directoryName;
	DIR *dir;
	Error errors;
} DirHandle;

// delcare dir-handling public functions

int dir_init(DirHandle *handle, char *directoryName);

int dir_free(DirHandle *handle);

int dir_read(DirHandle *handle, char **filename);

int dir_getError(DirHandle *handle, int errorCode, char **errorMessage);

#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _CORE_LIBRARY_SYSTEM_DIR_H

