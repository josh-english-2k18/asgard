/*
 * serialize.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard core library serialization functions, header file.
 *
 * Written by Josh English.
 */

#if !defined(_CORE_LIBRARY_CORE_SERIALIZE_H)

#define _CORE_LIBRARY_CORE_SERIALIZE_H

#if !defined(_CORE_H) && !defined(_CORE_LIBRARY_H) && \
		!defined(_CORE_LIBRARY_CORE_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _CORE_H || _CORE_LIBRARY_H || _CORE_LIBRARY_CORE_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

// delcare serialize public functions

int serialize_encodeShort(short shortValue, char serialValue[SIZEOF_SHORT]);

int serialize_decodeShort(char *serialValue, int serialLength,
		short *shortValue);

int serialize_encodeInt(int intValue, char serialValue[SIZEOF_INT]);

int serialize_decodeInt(char *serialValue, int serialLength,
		int *intValue);

int serialize_encodeFloat(float floatValue, char serialValue[SIZEOF_FLOAT]);

int serialize_decodeFloat(char *serialValue, int serialLength,
		float *floatValue);

int serialize_encodeAlint(alint alintValue, char serialValue[SIZEOF_ALINT]);

int serialize_decodeAlint(char *serialValue, int serialLength,
		alint *alintValue);

int serialize_encodeDouble(double doubleValue, char serialValue[SIZEOF_DOUBLE]);

int serialize_decodeDouble(char *serialValue, int serialLength,
		double *doubleValue);

#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _CORE_LIBRARY_CORE_SERIALIZE_H

