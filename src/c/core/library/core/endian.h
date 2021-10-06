/*
 * endian.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard core library endian functions, header file.
 *
 * Written by Josh English.
 */

#if !defined(_CORE_LIBRARY_CORE_ENDIAN_H)

#define _CORE_LIBRARY_CORE_ENDIAN_H

#if !defined(_CORE_H) && !defined(_CORE_LIBRARY_H) && \
		!defined(_CORE_LIBRARY_CORE_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _CORE_H || _CORE_LIBRARY_H || _CORE_LIBRARY_CORE_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

// define endian public constants

#define ENDIAN_BYTEORDER_LOHI							1
#define ENDIAN_BYTEORDER_HILO							2

// delcare endian public functions

int endian_determineByteOrder();

char *endian_byteOrderToString(int byteOrder);

short endian_invertShort(short value);

int endian_invertInt(int value);

float endian_invertFloat(float value);

alint endian_invertAlint(alint value);

double endian_invertDouble(double value);

#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _CORE_LIBRARY_CORE_ENDIAN_H

