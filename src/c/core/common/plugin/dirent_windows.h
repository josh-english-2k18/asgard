/*
 * dirent_windows.h
 *
 * Copyright Kevlin Henney, 1997, 2003. All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * this copyright and permissions notice appear in all copies and derivatives.
 *
 * This software is supplied "as is" without express or implied warranty. But
 * that said, if there are any problems please get in touch.
 *
 * The POSIX directory browsing functions and types plugin fow Windows systems.
 */

#if !defined(_CORE_COMMON_PLUGIN_DIRENT_WINDOWS_H)

#define _CORE_COMMON_PLUGIN_DIRENT_WINDOWS_H

#if !defined(_CORE_H) && !defined(_CORE_COMMON_PLUGIN_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _CORE_H || _CORE_COMMON_PLUGIN_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


#if defined(WIN32)

// define dirent public data types

typedef struct DIR DIR;

struct dirent
{
    char *d_name;
};

// declare dirent public functions

DIR *opendir(const char *);

int closedir(DIR *);

struct dirent *readdir(DIR *);

void rewinddir(DIR *);

#endif // WIN32


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _CORE_COMMON_PLUGIN_DIRENT_WINDOWS_H

