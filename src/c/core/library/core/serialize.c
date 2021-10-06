/*
 * serialize.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard core library serialization functions.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#define _CORE_LIBRARY_CORE_COMPONENT
#include "core/library/core/serialize.h"

// define serialize public functions

int serialize_encodeShort(short shortValue, char serialValue[SIZEOF_SHORT])
{
	short localValue = 0;

	if(endian_determineByteOrder() == ENDIAN_BYTEORDER_HILO) {
		localValue = endian_invertShort(shortValue);
	}
	else {
		localValue = shortValue;
	}

	memcpy(serialValue, (char *)&localValue, SIZEOF_SHORT);

	return 0;
}

int serialize_decodeShort(char *serialValue, int serialLength,
		short *shortValue)
{
	short localValue = 0;

	if((serialValue == NULL) || (serialLength < SIZEOF_SHORT) ||
			(shortValue == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	*shortValue = 0;

	memcpy((char *)&localValue, serialValue, SIZEOF_SHORT);

	if(endian_determineByteOrder() == ENDIAN_BYTEORDER_HILO) {
		*shortValue = endian_invertShort(localValue);
	}
	else {
		*shortValue = localValue;
	}

	return 0;
}

int serialize_encodeInt(int intValue, char serialValue[SIZEOF_INT])
{
	int localValue = 0;

	if(endian_determineByteOrder() == ENDIAN_BYTEORDER_HILO) {
		localValue = endian_invertInt(intValue);
	}
	else {
		localValue = intValue;
	}

	memcpy(serialValue, (char *)&localValue, SIZEOF_INT);

	return 0;
}

int serialize_decodeInt(char *serialValue, int serialLength,
		int *intValue)
{
	int localValue = 0;

	if((serialValue == NULL) || (serialLength < SIZEOF_INT) ||
			(intValue == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	*intValue = 0;

	memcpy((char *)&localValue, serialValue, SIZEOF_INT);

	if(endian_determineByteOrder() == ENDIAN_BYTEORDER_HILO) {
		*intValue = endian_invertInt(localValue);
	}
	else {
		*intValue = localValue;
	}

	return 0;
}

int serialize_encodeFloat(float floatValue, char serialValue[SIZEOF_FLOAT])
{
	float localValue = 0;

	if(endian_determineByteOrder() == ENDIAN_BYTEORDER_HILO) {
		localValue = endian_invertFloat(floatValue);
	}
	else {
		localValue = floatValue;
	}

	memcpy(serialValue, (char *)&localValue, SIZEOF_FLOAT);

	return 0;
}

int serialize_decodeFloat(char *serialValue, int serialLength,
		float *floatValue)
{
	float localValue = 0;

	if((serialValue == NULL) || (serialLength < SIZEOF_FLOAT) ||
			(floatValue == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	*floatValue = 0;

	memcpy((char *)&localValue, serialValue, SIZEOF_FLOAT);

	if(endian_determineByteOrder() == ENDIAN_BYTEORDER_HILO) {
		*floatValue = endian_invertFloat(localValue);
	}
	else {
		*floatValue = localValue;
	}

	return 0;
}

int serialize_encodeAlint(alint alintValue, char serialValue[SIZEOF_ALINT])
{
	alint localValue = 0;

	if(endian_determineByteOrder() == ENDIAN_BYTEORDER_HILO) {
		localValue = endian_invertAlint(alintValue);
	}
	else {
		localValue = alintValue;
	}

	memcpy(serialValue, (char *)&localValue, SIZEOF_ALINT);

	return 0;
}

int serialize_decodeAlint(char *serialValue, int serialLength,
		alint *alintValue)
{
	alint localValue = 0;

	if((serialValue == NULL) || (serialLength < SIZEOF_ALINT) ||
			(alintValue == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	*alintValue = 0;

	memcpy((char *)&localValue, serialValue, SIZEOF_ALINT);

	if(endian_determineByteOrder() == ENDIAN_BYTEORDER_HILO) {
		*alintValue = endian_invertAlint(localValue);
	}
	else {
		*alintValue = localValue;
	}

	return 0;
}

int serialize_encodeDouble(double doubleValue, char serialValue[SIZEOF_DOUBLE])
{
	double localValue = 0;

	if(endian_determineByteOrder() == ENDIAN_BYTEORDER_HILO) {
		localValue = endian_invertDouble(doubleValue);
	}
	else {
		localValue = doubleValue;
	}

	memcpy(serialValue, (char *)&localValue, SIZEOF_DOUBLE);

	return 0;
}

int serialize_decodeDouble(char *serialValue, int serialLength,
		double *doubleValue)
{
	double localValue = 0;

	if((serialValue == NULL) || (serialLength < SIZEOF_DOUBLE) ||
			(doubleValue == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	*doubleValue = 0;

	memcpy((char *)&localValue, serialValue, SIZEOF_DOUBLE);

	if(endian_determineByteOrder() == ENDIAN_BYTEORDER_HILO) {
		*doubleValue = endian_invertDouble(localValue);
	}
	else {
		*doubleValue = localValue;
	}

	return 0;
}

