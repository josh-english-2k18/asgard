/*
 * endian.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard core library endian functions.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#define _CORE_LIBRARY_CORE_COMPONENT
#include "core/library/core/endian.h"

// define endian public functions

int endian_determineByteOrder()
{
	union {
		short shortValue;
		char charValue[SIZEOF_SHORT];
	} endian;

	endian.shortValue = 1;

	if(endian.charValue[0] == endian.shortValue) {
		return ENDIAN_BYTEORDER_LOHI;
	}

	return ENDIAN_BYTEORDER_HILO;
}

char *endian_byteOrderToString(int byteOrder)
{
	switch(byteOrder) {
		case ENDIAN_BYTEORDER_LOHI:
			return "LOHI";
		case ENDIAN_BYTEORDER_HILO:
			return "HILO";
	}

	return "UNKNOWN";
}

short endian_invertShort(short value)
{
	int ii = 0;
	int nn = 0;

	typedef union {
		short shortValue;
		char charValue[SIZEOF_SHORT];
	} ShortUnion;

	ShortUnion local;
	ShortUnion result;

	local.shortValue = value;

	for(ii = 0, nn = (SIZEOF_SHORT - 1); ii < SIZEOF_SHORT; ii++, nn--) {
		result.charValue[ii] = local.charValue[nn];
	}

	return result.shortValue;
}

int endian_invertInt(int value)
{
	int ii = 0;
	int nn = 0;

	typedef union {
		int intValue;
		char charValue[SIZEOF_INT];
	} IntUnion;

	IntUnion local;
	IntUnion result;

	local.intValue = value;

	for(ii = 0, nn = (SIZEOF_INT - 1); ii < SIZEOF_INT; ii++, nn--) {
		result.charValue[ii] = local.charValue[nn];
	}

	return result.intValue;
}

float endian_invertFloat(float value)
{
	int ii = 0;
	int nn = 0;

	typedef union {
		float floatValue;
		char charValue[SIZEOF_FLOAT];
	} FloatUnion;

	FloatUnion local;
	FloatUnion result;

	local.floatValue = value;

	for(ii = 0, nn = (SIZEOF_FLOAT - 1); ii < SIZEOF_FLOAT; ii++, nn--) {
		result.charValue[ii] = local.charValue[nn];
	}

	return result.floatValue;
}

alint endian_invertAlint(alint value)
{
	int ii = 0;
	int nn = 0;

	typedef union {
		alint alintValue;
		char charValue[SIZEOF_ALINT];
	} AlintUnion;

	AlintUnion local;
	AlintUnion result;

	local.alintValue = value;

	for(ii = 0, nn = (SIZEOF_ALINT - 1); ii < SIZEOF_ALINT; ii++, nn--) {
		result.charValue[ii] = local.charValue[nn];
	}

	return result.alintValue;
}

double endian_invertDouble(double value)
{
	int ii = 0;
	int nn = 0;

	typedef union {
		double doubleValue;
		char charValue[SIZEOF_DOUBLE];
	} DoubleUnion;

	DoubleUnion local;
	DoubleUnion result;

	local.doubleValue = value;

	for(ii = 0, nn = (SIZEOF_DOUBLE - 1); ii < SIZEOF_DOUBLE; ii++, nn--) {
		result.charValue[ii] = local.charValue[nn];
	}

	return result.doubleValue;
}

