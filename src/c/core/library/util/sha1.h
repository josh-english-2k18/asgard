/*
 * sha1.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The SHA-1 algorithmic calculation utility, header file.
 *
 * Written by Josh English.
 */

#if !defined(_CORE_LIBRARY_UTIL_SHA1_H)

#define _CORE_LIBRARY_UTIL_SHA1_H

#if !defined(_CORE_H) && !defined(_CORE_LIBRARY_H) && \
		!defined(_CORE_LIBRARY_UTIL_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _CORE_H || _CORE_LIBRARY_H || _CORE_LIBRARY_UTIL_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

// define sha-1 data types

typedef struct _Sha1 {
    unsigned long int total[2];
    unsigned long int state[5];
    unsigned char buffer[64];
	unsigned char padding[64];
	unsigned char digest[20];
} Sha1;

// declare sha-1 public functions

int sha1_init(Sha1 *hash);

int sha1_hashData(Sha1 *hash, unsigned char *input, int inputLength);

int sha1_calculateHash(Sha1 *hash);

int sha1_constructSignature(Sha1 *hash, char **signature,
		int *signatureLength);

int sha1_constructSignatureStatic(Sha1 *hash, char signature[40]);

#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _CORE_LIBRARY_UTIL_SHA1_H

