/*
 * ctype.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard core library ctype functions, header file.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#define _CORE_LIBRARY_CORE_COMPONENT
#include "core/library/core/ctype.h"


// define ctype private constants

const char *ENGLISH_LANGUAGE =
		"etoanisrhldcumpfgwybvkIxTSAMBOWDERLCPUNHGjqzFVYJKXQZ";

const char *ENGLISH_VOWELS =
		"eEaAiIoOuUyY";

const char *NUMBERS = "0123456789";


// define ctype public functions

aboolean ctype_isAlphabetic(char value)
{
	if(strchr(ENGLISH_LANGUAGE, value)) {
		return atrue;
	}

	return afalse;
}

aboolean ctype_isVowel(char value)
{
	if(strchr(ENGLISH_VOWELS, value)) {
		return atrue;
	}

	return afalse;
}

aboolean ctype_isNumeric(char value)
{
	if(strchr(NUMBERS, value)) {
		return atrue;
	}

	return afalse;
}

aboolean ctype_isPlainText(char value)
{
	int intValue = (int)((unsigned char)value);

	if(((intValue < 32) || (intValue > 126)) &&
			(intValue != 9) &&
			(intValue != 10) &&
			(intValue != 13)) {
		return afalse;
	}

	return atrue;
}

aboolean ctype_isWhitespace(char value)
{
	int intValue = (int)((unsigned char)value);

	if((intValue == 9) || (intValue == 10) || (intValue == 13) ||
			(intValue == 32)) {
		return atrue;
	}

	return afalse;
}

aboolean ctype_isLower(char value)
{
	int intValue = (int)((unsigned char)value);

	if((intValue >= 97) && (intValue <= 122)) {
		return atrue;
	}

	return afalse;
}

aboolean ctype_isUpper(char value)
{
	int intValue = (int)((unsigned char)value);

	if((intValue >= 65) && (intValue <= 90)) {
		return atrue;
	}

	return afalse;
}

char ctype_toLower(char value)
{
	char result = value;
	int intValue = (int)((unsigned char)value);

	if((intValue >= 65) && (intValue <= 90)) {
		result = (char)(intValue + 32);
	}

	return result;
}

char ctype_toUpper(char value)
{
	char result = value;
	int intValue = (int)((unsigned char)value);

	if((intValue >= 97) && (intValue <= 122)) {
		result = (char)(intValue - 32);
	}

	return result;
}

int ctype_ctoi(char value)
{
	return (int)((unsigned char)value);
}

int ctype_numericValue(char value)
{
	int intValue = (int)((unsigned char)value);
	int result = 0;

	if((intValue >= 48) && (intValue <= 57)) {
		result = (intValue - 48);
	}

	return result;
}

// define cstring public functions

aboolean cstring_isAlphabetic(char *string, int stringLength)
{
	int ii = 0;

	if((string == NULL) || (stringLength < 1)) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	for(ii = 0; ii < stringLength; ii++) {
		if(!ctype_isAlphabetic(string[ii])) {
			return afalse;
		}
	}

	return atrue;
}

aboolean cstring_containsAlphabetic(char *string, int stringLength)
{
	int ii = 0;

	if((string == NULL) || (stringLength < 1)) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	for(ii = 0; ii < stringLength; ii++) {
		if(ctype_isAlphabetic(string[ii])) {
			return atrue;
		}
	}

	return afalse;
}

aboolean cstring_containsVowel(char *string, int stringLength)
{
	int ii = 0;

	if((string == NULL) || (stringLength < 1)) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	for(ii = 0; ii < stringLength; ii++) {
		if(ctype_isVowel(string[ii])) {
			return atrue;
		}
	}

	return afalse;
}

aboolean cstring_isNumericInteger(char *string, int stringLength)
{
	int ii = 0;

	if((string == NULL) || (stringLength < 1)) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	for(ii = 0; ii < stringLength; ii++) {
		if(!ctype_isNumeric(string[ii])) {
			return afalse;
		}
	}

	return atrue;
}

aboolean cstring_isNumericReal(char *string, int stringLength)
{
	int ii = 0;

	if((string == NULL) || (stringLength < 1)) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	for(ii = 0; ii < stringLength; ii++) {
		if((!ctype_isNumeric(string[ii])) && (string[ii] != '.')) {
			return afalse;
		}
	}

	return atrue;
}

aboolean cstring_isPlainText(char *string, int stringLength)
{
	int ii = 0;

	if((string == NULL) || (stringLength < 1)) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	for(ii = 0; ii < stringLength; ii++) {
		if(!ctype_isPlainText(string[ii])) {
			return afalse;
		}
	}

	return atrue;
}

aboolean cstring_isWhitespace(char *string, int stringLength)
{
	int ii = 0;

	if((string == NULL) || (stringLength < 1)) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	for(ii = 0; ii < stringLength; ii++) {
		if(!ctype_isWhitespace(string[ii])) {
			return afalse;
		}
	}

	return atrue;
}

aboolean cstring_isLowerCase(char *string, int stringLength)
{
	int ii = 0;

	if((string == NULL) || (stringLength < 1)) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	for(ii = 0; ii < stringLength; ii++) {
		if(!ctype_isLower(string[ii])) {
			return afalse;
		}
	}

	return atrue;
}

aboolean cstring_isUpperCase(char *string, int stringLength)
{
	int ii = 0;

	if((string == NULL) || (stringLength < 1)) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	for(ii = 0; ii < stringLength; ii++) {
		if(!ctype_isUpper(string[ii])) {
			return afalse;
		}
	}

	return atrue;
}

