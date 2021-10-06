/*
 * crc32.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The 32-bit Cyclic Redundancy Check (CRC-32) algorithmic calculation utility,
 * header file.
 *
 * Written by Josh English.
 */

#if !defined(_CORE_LIBRARY_UTIL_CRC32_H)

#define _CORE_LIBRARY_UTIL_CRC32_H

#if !defined(_CORE_H) && !defined(_CORE_LIBRARY_H) && \
		!defined(_CORE_LIBRARY_UTIL_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _CORE_H || _CORE_LIBRARY_H || _CORE_LIBRARY_UTIL_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

// declare crc-32 public functions

unsigned int crc32_calculateHash(char *value, int valueLength);

#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _CORE_LIBRARY_UTIL_CRC32_H

