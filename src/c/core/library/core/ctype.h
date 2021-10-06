/*
 * ctype.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard core library ctype functions, header file.
 *
 * Written by Josh English.
 */

#if !defined(_CORE_LIBRARY_CORE_CTYPE_H)

#define _CORE_LIBRARY_CORE_CTYPE_H

#if !defined(_CORE_H) && !defined(_CORE_LIBRARY_H) && \
		!defined(_CORE_LIBRARY_CORE_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _CORE_H || _CORE_LIBRARY_H || _CORE_LIBRARY_CORE_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// delcare ctype public functions

aboolean ctype_isAlphabetic(char value);

aboolean ctype_isVowel(char value);

aboolean ctype_isNumeric(char value);

aboolean ctype_isPlainText(char value);

aboolean ctype_isWhitespace(char value);

aboolean ctype_isLower(char value);

aboolean ctype_isUpper(char value);

char ctype_toLower(char value);

char ctype_toUpper(char value);

int ctype_ctoi(char value);

int ctype_numericValue(char value);


// define cstring public functions

aboolean cstring_isAlphabetic(char *string, int stringLength);

aboolean cstring_containsAlphabetic(char *string, int stringLength);

aboolean cstring_containsVowel(char *string, int stringLength);

aboolean cstring_isNumericInteger(char *string, int stringLength);

aboolean cstring_isNumericReal(char *string, int stringLength);

aboolean cstring_isPlainText(char *string, int stringLength);

aboolean cstring_isWhitespace(char *string, int stringLength);

aboolean cstring_isLowerCase(char *string, int stringLength);

aboolean cstring_isUpperCase(char *string, int stringLength);


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _CORE_LIBRARY_CORE_CTYPE_H

