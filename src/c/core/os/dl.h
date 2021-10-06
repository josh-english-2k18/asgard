/*
 * dl.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard dynamic-library interface, header file.
 *
 * Written by Josh English.
 */

#if !defined(_CORE_OS_DL_H)

#define _CORE_OS_DL_H

#if !defined(_CORE_H) && !defined(_CORE_OS_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _CORE_H || _CORE_OS_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

// delcare dl public functions

int dl_open(char *filename, void **handle);

int dl_close(void *handle);

int dl_lookup(void *handle, char *functionName, void **function);

#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _CORE_OS_DL_H

