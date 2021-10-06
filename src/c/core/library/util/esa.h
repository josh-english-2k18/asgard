/*
 * esa.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * Notes:
 *
 * The English Stemming Algorithm (ESA) (see English, Joshua S. 2003)
 * reference implementation for reducing the morphological variation of any
 * given English Lanugage token through ESA-type conflation, header file.
 *
 * Boundary Conditions:
 *
 * 		1. input must be greater than 2 characters in length
 * 			a. has at least one vowel
 * 			b. has at least one consonant
 *
 * 		2. output must be:
 * 			a. greater than 2 characters in length
 *
 * Rule Definition:
 *
 * 		1. consists of 4 parts that may be repeated
 * 			a. an ending of 1 or more characters
 * 			b. an intact flag (only modify the token if it has not already
 * 			   been modified)
 * 			c. the number of characters to remove, 0 or more
 * 			d. an append string of 1 or more characters
 *
 * 		2. examples:
 * 			a. 'ies,n,3,y'
 * 			b. 'mu,y,2,'
 * 			c. 'ply,n,0,'
 * 			d. 'soin,n,4,j'
 *
 * Written by Josh English.
 */

#if !defined(_CORE_LIBRARY_UTIL_ESA_H)

#define _CORE_LIBRARY_UTIL_ESA_H

#if !defined(_CORE_H) && !defined(_CORE_LIBRARY_H) && \
		!defined(_CORE_LIBRARY_UTIL_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _CORE_H || _CORE_LIBRARY_H || _CORE_LIBRARY_UTIL_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// define esa public data types

typedef struct _EsaRuleList {
	int ref;
	int size;
	int *length;
	char **value;
} EsaRuleList;

typedef struct _Esa {
	aboolean isDebugMode;
	int longestRuleLength;
	EsaRuleList rules;
} Esa;


// declare esa public functions

void esa_init(Esa *esa);

void esa_free(Esa *esa);

void esa_setDebugMode(Esa *esa, aboolean mode);

char *esa_stemToken(Esa *esa, char *token, int length, int *tokenResultLength);


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _CORE_LIBRARY_UTIL_ESA_H

