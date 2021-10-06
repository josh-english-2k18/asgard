/*
 * string.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard C string library wrapper functions.
 *
 * Written by Josh English.
 */

// preprocessor directives

#define _CORE_COMMON_COMPONENT
#include "core/common/common.h"
#include "core/common/macros.h"
#include "core/common/version.h"
#define _CORE_OS_COMPONENT
#include "core/os/types.h"
#define _CORE_MEMORY_COMPONENT
#define MEMORY_NATIVE
#include "core/memory/memory.h"

// define string library wrapper function macros

#if defined(__linux__) || defined(__APPLE__)
#	define VSNPRINTF_MULTI_CALL_SAFE					0
#else // !__linux__ && !__APPLE__
#	define VSNPRINTF_MULTI_CALL_SAFE					1
#endif // __linux__ || __APPLE__


// convenience declaration of string library wrapper public functions

char *string_strncasestr(const char *haystack, int haystackLength,
		const char *needle, int needleLength, const char *file,
		const char *function, size_t line);


// define string library wrapper private functions

static int cicmp(char alpha, char beta)
{
	char conv = (char)0;

	if(((unsigned int)alpha > 96) && ((unsigned int)alpha < 123)) {
		conv = (char)((unsigned int)alpha - 32);
	}
	else if(((unsigned int)alpha > 64) && ((unsigned int)alpha < 91)) {
		conv = (char)((unsigned int)alpha + 32);
	}
	else {
		conv = alpha;
	}

	if((alpha == beta) || (conv == beta)) {
		return 1;
	}

	return 0;
}


// define string library wrapper public functions

void *string_memcpy(void *destination, const void *source, size_t length,
		const char *file, const char *function, size_t line)
{
	if((destination == NULL) || (source == NULL) || (destination == source) ||
			(length < 1)) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s->%s():%i] error - invalid or missing "
					"arguments to %s()", (char *)file, (char *)function,
					(int)line, __FUNCTION__);
			if(destination == NULL) {
				fprintf(stderr, ", destination is NULL");
			}
			if(source == NULL) {
				fprintf(stderr, ", source is NULL");
			}
			if(destination == source) {
				fprintf(stderr, ", destination equals source");
			}
			if(length < 1) {
				fprintf(stderr, ", length is %i", (int)length);
			}
			fprintf(stderr, ".\n");
		}
		if((destination == NULL) || (source == NULL) ||
				(destination == source)) {
			return NULL;
		}
		return destination;
	}

	return memcpy(destination, source, length);
}

void *string_memmove(void *destination, const void *source, size_t length,
		const char *file, const char *function, size_t line)
{
	if((destination == NULL) || (source == NULL) || (destination == source) ||
			(length < 1)) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s->%s():%i] error - invalid or missing "
					"arguments to %s()", (char *)file, (char *)function,
					(int)line, __FUNCTION__);
			if(destination == NULL) {
				fprintf(stderr, ", destination is NULL");
			}
			if(source == NULL) {
				fprintf(stderr, ", source is NULL");
			}
			if(destination == source) {
				fprintf(stderr, ", destination equals source");
			}
			if(length < 1) {
				fprintf(stderr, ", length is %i", (int)length);
			}
			fprintf(stderr, ".\n");
		}
		if((destination == NULL) || (source == NULL) ||
				(destination == source)) {
			return NULL;
		}
		return destination;
	}

	return memmove(destination, source, length);
}

void *string_memset(void *string, int charValue, size_t length,
		const char *file, const char *function, size_t line)
{
	if((string == NULL) || (charValue < 0) || (charValue > 255) ||
			(length < 1)) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s->%s():%i] error - invalid or missing "
					"arguments to %s()", (char *)file, (char *)function,
					(int)line, __FUNCTION__);
			if(string == NULL) {
				fprintf(stderr, ", string is NULL");
			}
			if((charValue < 0) || (charValue > 255)) {
				fprintf(stderr, ", charValue is %i", charValue);
			}
			if(length < 1) {
				fprintf(stderr, ", length is %i", (int)length);
			}
			fprintf(stderr, ".\n");
		}
		if((string == NULL) || (length < 1)) {
			return NULL;
		}
		charValue = 0;
	}

	return memset(string, charValue, length);
}

char *string_strchr(const char *string, int charValue, const char *file,
		const char *function, size_t line)
{
	if((string == NULL) || (charValue < 0) || (charValue > 255)) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s->%s():%i] error - invalid or missing "
					"arguments to %s()", (char *)file, (char *)function,
					(int)line, __FUNCTION__);
			if(string == NULL) {
				fprintf(stderr, ", string is NULL");
			}
			if((charValue < 0) || (charValue > 255)) {
				fprintf(stderr, ", charValue is %i", charValue);
			}
			fprintf(stderr, ".\n");
		}
		return NULL;
	}

	return strchr(string, charValue);
}

char *string_strrchr(const char *string, int charValue, const char *file,
		const char *function, size_t line)
{
	if((string == NULL) || (charValue < 0) || (charValue > 255)) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s->%s():%i] error - invalid or missing "
					"arguments to %s()", (char *)file, (char *)function,
					(int)line, __FUNCTION__);
			if(string == NULL) {
				fprintf(stderr, ", string is NULL");
			}
			if((charValue < 0) || (charValue > 255)) {
				fprintf(stderr, ", charValue is %i", charValue);
			}
			fprintf(stderr, ".\n");
		}
		return NULL;
	}

	return strrchr(string, charValue);
}

char *string_strstr(const char *haystack, const char *needle, const char *file,
		const char *function, size_t line)
{
	if((haystack == NULL) || (needle == NULL)) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s->%s():%i] error - invalid or missing "
					"arguments to %s()", (char *)file, (char *)function,
					(int)line, __FUNCTION__);
			if(haystack == NULL) {
				fprintf(stderr, ", haystack is NULL");
			}
			if(needle == NULL) {
				fprintf(stderr, ", needle is %i", needle);
			}
			fprintf(stderr, ".\n");
		}
		return NULL;
	}

	return strstr(haystack, needle);
}

char *string_strnstr(const char *haystack, int haystackLength,
		const char *needle, int needleLength, const char *file,
		const char *function, size_t line)
{
	int ii = 0;
	int nn = 0;
	char *result = NULL;

	if((haystack == NULL) ||
			(needle == NULL) ||
			(haystackLength < 1) ||
			(needleLength < 1)) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s->%s():%i] error - invalid or missing "
					"arguments to %s()", (char *)file, (char *)function,
					(int)line, __FUNCTION__);
			if(haystack == NULL) {
				fprintf(stderr, ", haystack is NULL");
			}
			if(haystackLength < 1) {
				fprintf(stderr, ", haystack length is %i", haystackLength);
			}
			if(needle == NULL) {
				fprintf(stderr, ", needle is %i", needle);
			}
			if(needleLength < 1) {
				fprintf(stderr, ", needle length is %i", needleLength);
			}
			fprintf(stderr, ".\n");
		}
		return NULL;
	}

	for(ii = 0, nn = 0; ii < haystackLength; ii++) {
		if(haystack[ii] == needle[nn]) {
			nn++;
			if(nn >= needleLength) {
				nn = needleLength;
				result = (char *)((aptrcast)haystack + (alint)(ii - (nn - 1)));
				break;
			}
		}
		else {
			if(nn > 0) {
				ii -= (nn - 1);
			}
			nn = 0;
		}
	}

	return result;
}

char *string_strcasestr(const char *haystack, const char *needle,
		const char *file, const char *function, size_t line)
{
	if((haystack == NULL) || (needle == NULL)) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s->%s():%i] error - invalid or missing "
					"arguments to %s()", (char *)file, (char *)function,
					(int)line, __FUNCTION__);
			if(haystack == NULL) {
				fprintf(stderr, ", haystack is NULL");
			}
			if(needle == NULL) {
				fprintf(stderr, ", needle is %i", needle);
			}
			fprintf(stderr, ".\n");
		}
		return NULL;
	}

	return string_strncasestr(haystack, (int)strlen(haystack), needle,
			(int)strlen(needle), file, function, line);
}

char *string_strncasestr(const char *haystack, int haystackLength,
		const char *needle, int needleLength, const char *file,
		const char *function, size_t line)
{
	int ii = 0;
	int nn = 0;
	char *result = NULL;

	if((haystack == NULL) ||
			(needle == NULL) ||
			(haystackLength < 1) ||
			(needleLength < 1)) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s->%s():%i] error - invalid or missing "
					"arguments to %s()", (char *)file, (char *)function,
					(int)line, __FUNCTION__);
			if(haystack == NULL) {
				fprintf(stderr, ", haystack is NULL");
			}
			if(haystackLength < 1) {
				fprintf(stderr, ", haystack length is %i", haystackLength);
			}
			if(needle == NULL) {
				fprintf(stderr, ", needle is %i", needle);
			}
			if(needleLength < 1) {
				fprintf(stderr, ", needle length is %i", needleLength);
			}
			fprintf(stderr, ".\n");
		}
		return NULL;
	}

	for(ii = 0, nn = 0; ii < haystackLength; ii++) {
		if((haystack[ii] == needle[nn]) || (cicmp(haystack[ii], needle[nn]))) {
			nn++;
			if(nn >= needleLength) {
				nn = needleLength;
				result = (char *)((aptrcast)haystack + (alint)(ii - (nn - 1)));
				break;
			}
		}
		else {
			if(nn > 0) {
				ii -= (nn - 1);
			}
			nn = 0;
		}
	}

	return result;
}

int string_strcmp(const char *stringOne, const char *stringTwo,
		const char *file, const char *function, size_t line)
{
	if((stringOne == NULL) || (stringTwo == NULL)) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s->%s():%i] error - invalid or missing "
					"arguments to %s()", (char *)file, (char *)function,
					(int)line, __FUNCTION__);
			if(stringOne == NULL) {
				fprintf(stderr, ", stringOne is NULL");
			}
			if(stringTwo == NULL) {
				fprintf(stderr, ", stringTwo is NULL");
			}
			fprintf(stderr, ".\n");
		}
		return -1;
	}

	return strcmp(stringOne, stringTwo);
}

int string_strcasecmp(const char *stringOne, const char *stringTwo,
		const char *file, const char *function, size_t line)
{
	if((stringOne == NULL) || (stringTwo == NULL)) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s->%s():%i] error - invalid or missing "
					"arguments to %s()", (char *)file, (char *)function,
					(int)line, __FUNCTION__);
			if(stringOne == NULL) {
				fprintf(stderr, ", stringOne is NULL");
			}
			if(stringTwo == NULL) {
				fprintf(stderr, ", stringTwo is %i", stringTwo);
			}
			fprintf(stderr, ".\n");
		}
		return -1;
	}

#if !defined(WIN32)
	return strcasecmp(stringOne, stringTwo);
#else // WIN32
	return _stricmp(stringOne, stringTwo);
#endif // !WIN32
}

int string_strncmp(const char *stringOne, const char *stringTwo, size_t length,
		const char *file, const char *function, size_t line)
{
	if((stringOne == NULL) || (stringTwo == NULL) || (length < 1)) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s->%s():%i] error - invalid or missing "
					"arguments to %s()", (char *)file, (char *)function,
					(int)line, __FUNCTION__);
			if(stringOne == NULL) {
				fprintf(stderr, ", stringOne is NULL");
			}
			if(stringTwo == NULL) {
				fprintf(stderr, ", stringTwo is NULL");
			}
			if(length < 1) {
				fprintf(stderr, ", length is %i", (int)length);
			}
			fprintf(stderr, ".\n");
		}
		if((stringOne == NULL) || (stringTwo == NULL)) {
			return -1;
		}
		length = 0;
	}

	return strncmp(stringOne, stringTwo, length);
}

int string_strncasecmp(const char *stringOne, const char *stringTwo,
		size_t length, const char *file, const char *function, size_t line)
{
	if((stringOne == NULL) || (stringTwo == NULL) || (length < 1)) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s->%s():%i] error - invalid or missing "
					"arguments to %s()", (char *)file, (char *)function,
					(int)line, __FUNCTION__);
			if(stringOne == NULL) {
				fprintf(stderr, ", stringOne is NULL");
			}
			if(stringTwo == NULL) {
				fprintf(stderr, ", stringTwo is %i", stringTwo);
			}
			if(length < 1) {
				fprintf(stderr, ", length is %i", (int)length);
			}
			fprintf(stderr, ".\n");
		}
		return -1;
	}

#if !defined(WIN32)
	return strncasecmp(stringOne, stringTwo, length);
#else // WIN32
	return _strnicmp(stringOne, stringTwo, length);
#endif // !WIN32
}

size_t string_strlen(const char *string, const char *file,
		const char *function, size_t line)
{
	if(string == NULL) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s->%s():%i] error - invalid or missing "
					"arguments to %s(), string is NULL.\n", (char *)file,
					(char *)function, (int)line, __FUNCTION__);
		}
		string = "";
	}

	return strlen(string);
}

char *string_strerror(int errorNumber, const char *file, const char *function,
		size_t line)
{
	char *result = NULL;

	result = strerror(errorNumber);

	if(result == NULL) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s->%s():%i] error - invalid or missing "
					"arguments to %s(), error %i returned NULL.\n",
					(char *)file, (char *)function, (int)line, __FUNCTION__,
					errorNumber);
		}
		result = "(asgard) unknown error";
	}

	return result;
}

char *string_strcpy(char *destination, const char *source, const char *file,
		const char *function, size_t line)
{
	if((destination == NULL) || (source == NULL)) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s->%s():%i] error - invalid or missing "
					"arguments to %s()", (char *)file, (char *)function,
					(int)line, __FUNCTION__);
			if(destination == NULL) {
				fprintf(stderr, ", destination is NULL");
			}
			if(source == NULL) {
				fprintf(stderr, ", source is NULL");
			}
			fprintf(stderr, ".\n");
		}
		return NULL;
	}

	return strcpy(destination, source);
}

char *string_strncpy(char *destination, const char *source, size_t length,
		const char *file, const char *function, size_t line)
{
	if((destination == NULL) || (source == NULL) || (length < 1)) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s->%s():%i] error - invalid or missing "
					"arguments to %s()", (char *)file, (char *)function,
					(int)line, __FUNCTION__);
			if(destination == NULL) {
				fprintf(stderr, ", destination is NULL");
			}
			if(source == NULL) {
				fprintf(stderr, ", source is NULL");
			}
			if(length < 1) {
				fprintf(stderr, ", length is %i", (int)length);
			}
			fprintf(stderr, ".\n");
		}
		return NULL;
	}

	return strncpy(destination, source, length);
}

char *string_strcat(char *destination, const char *source, const char *file,
		const char *function, size_t line)
{
	if((destination == NULL) || (source == NULL)) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s->%s():%i] error - invalid or missing "
					"arguments to %s()", (char *)file, (char *)function,
					(int)line, __FUNCTION__);
			if(destination == NULL) {
				fprintf(stderr, ", destination is NULL");
			}
			if(source == NULL) {
				fprintf(stderr, ", source is NULL");
			}
			fprintf(stderr, ".\n");
		}
		return NULL;
	}

	return strcat(destination, source);
}

char *string_strncat(char *destination, const char *source, size_t length,
		const char *file, const char *function, size_t line)
{
	if((destination == NULL) || (source == NULL) || (length < 1)) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s->%s():%i] error - invalid or missing "
					"arguments to %s()", (char *)file, (char *)function,
					(int)line, __FUNCTION__);
			if(destination == NULL) {
				fprintf(stderr, ", destination is NULL");
			}
			if(source == NULL) {
				fprintf(stderr, ", source is NULL");
			}
			if(length < 1) {
				fprintf(stderr, ", length is %i", (int)length);
			}
			fprintf(stderr, ".\n");
		}
		return NULL;
	}

	return strncat(destination, source, length);
}

char *string_vaprintf(const char *format, va_list vaArgs, int *resultLength,
		const char *file, const char *function, size_t line)
{
	int counter = 0;
	int formatLength = 0;
	int bufferLength = 0;
	char *result = NULL;

	if((format == NULL) || (resultLength == NULL)) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s->%s():%i] error - invalid or missing "
					"arguments to %s()", (char *)file, (char *)function,
					(int)line, __FUNCTION__);
			if(format == NULL) {
				fprintf(stderr, ", format is NULL");
			}
			if(resultLength == NULL) {
				fprintf(stderr, ", resultLength is NULL");
			}
			fprintf(stderr, ".\n");
		}
		return NULL;
	}

	if(VSNPRINTF_MULTI_CALL_SAFE) {
		bufferLength = 128;
	}
	else {
		bufferLength = 8192;
	}

	result = (char *)MEMORY_MALLOC_FUNCTION(
			(sizeof(char) * (bufferLength + 1)),
			file, function, line);

	formatLength = vsnprintf(result, bufferLength, format, vaArgs);

	while((VSNPRINTF_MULTI_CALL_SAFE) &&
			((formatLength >= bufferLength) || (formatLength < 0)) &&
			(counter < 8)) {
		if(formatLength > 0) {
			bufferLength = (bufferLength + formatLength + 1);
		}
		else {
			bufferLength *= 2;
		}

		result = (char *)MEMORY_REALLOC_FUNCTION(result,
				(sizeof(char) * bufferLength),
				file, function, line);

		formatLength = vsnprintf(result, bufferLength, format, vaArgs);

		counter++;
	}

	*resultLength = (int)strlen(result);

	return result;
}

int string_atoi(const char *number, const char *file, const char *function,
		size_t line)
{
	if(number == NULL) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s->%s():%i] error - invalid or missing "
					"arguments to %s()", (char *)file, (char *)function,
					(int)line, __FUNCTION__);
			if(number == NULL) {
				fprintf(stderr, ", number is NULL");
			}
			fprintf(stderr, ".\n");
		}
		return 0;
	}

	return atoi(number);
}

double string_atod(const char *number, const char *file, const char *function,
		size_t line)
{
	if(number == NULL) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s->%s():%i] error - invalid or missing "
					"arguments to %s()", (char *)file, (char *)function,
					(int)line, __FUNCTION__);
			if(number == NULL) {
				fprintf(stderr, ", number is NULL");
			}
			fprintf(stderr, ".\n");
		}
		return 0;
	}

	return atof(number);
}

int string_strtotokens(char *string, int stringLength, char *delimiters,
		int delimitersLength, int minTokenLength, char ***tokenList,
		int **tokenLengths, int *tokenCount, const char *file,
		const char *function, size_t line)
{
	aboolean found = afalse;
	int ii = 0;
	int nn = 0;
	int tokenRef = 0;
	int tokenLength = 0;
	int resultRef = 0;
	int resultLength = 0;
	int *resultTokenLength = NULL;
	char **result = NULL;

	if((string == NULL) || (stringLength < 1) || (delimiters == NULL) ||
			(delimitersLength < 1) || (minTokenLength < 0) ||
			(tokenList == NULL) || (tokenLengths == NULL) ||
			(tokenCount == NULL)) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s->%s():%i] error - invalid or missing "
					"arguments to %s()", (char *)file, (char *)function,
					(int)line, __FUNCTION__);
			if(string == NULL) {
				fprintf(stderr, ", string is NULL");
			}
			if(stringLength < 1) {
				fprintf(stderr, ", stringLength is %i", stringLength);
			}
			if(delimiters == NULL) {
				fprintf(stderr, ", delimiters is NULL");
			}
			if(delimitersLength < 1) {
				fprintf(stderr, ", delimitersLength is %i", delimitersLength);
			}
			if(minTokenLength < 1) {
				fprintf(stderr, ", minTokenLength is %i", minTokenLength);
			}
			if(tokenList == NULL) {
				fprintf(stderr, ", tokenList is NULL");
			}
			else {
				*tokenList = NULL;
			}
			if(tokenLengths == NULL) {
				fprintf(stderr, ", tokenLengths is NULL");
			}
			else {
				*tokenLengths = NULL;
			}
			if(tokenCount == NULL) {
				fprintf(stderr, ", tokenCount is NULL");
			}
			else {
				*tokenCount = 0;
			}
			fprintf(stderr, ".\n");
		}
		return -1;
	}

	// initialize results

	*tokenList = NULL;
	*tokenLengths = NULL;
	*tokenCount = 0;

	// search the string for tokens matching the delimiter & length criteria

	for(ii = 0, tokenRef = 0; ii < stringLength; ii++) {
		found = afalse;

		for(nn = 0; nn < delimitersLength; nn++) {
			if(string[ii] == delimiters[nn]) {
				found = atrue;
				tokenLength = (ii - tokenRef);
				break;
			}
		}

		if((!found) && ((ii + 1) >= stringLength)) {
			found = atrue;
			tokenLength = ((ii - tokenRef) + 1);
		}

		if(found) {
			if(tokenLength >= minTokenLength) {
				if(result == NULL) {
					resultRef = 0;
					resultLength = 2;

					resultTokenLength = (int *)MEMORY_MALLOC_FUNCTION(
							((sizeof(int) * resultLength) + 1),
							file, function, line);

					result = (char **)MEMORY_MALLOC_FUNCTION(
							((sizeof(char *) * resultLength) + 1),
							file, function, line);
				}

				resultTokenLength[resultRef] = tokenLength;

				result[resultRef] = (char *)MEMORY_MALLOC_FUNCTION(
						((sizeof(char) * tokenLength) + 1),
						file, function, line);

				memcpy(result[resultRef], (string + tokenRef), tokenLength);

				resultRef += 1;

				if(resultRef >= resultLength) {
					resultLength *= 2;

					resultTokenLength = (int *)MEMORY_REALLOC_FUNCTION(
							resultTokenLength,
							(sizeof(int) * resultLength),
							file, function, line);

					result = (char **)MEMORY_REALLOC_FUNCTION(result,
							(sizeof(char *) * resultLength),
							file, function, line);
				}
			}

			tokenRef = (ii + 1);
		}
	}

	// assign result-set to results

	if(result != NULL) {
		*tokenList = result;
		*tokenLengths = resultTokenLength;
		*tokenCount = resultRef;
	}

	return 0;
}

double string_compareStrings(char *alpha, int alphaLength, char *beta,
		int betaLength, const char *file, const char *function,
		size_t line)
{
	aboolean found = afalse;
	int ii = 0;
	int nn = 0;
	int distance = 0;
	int totalDistance = 0;
	int averageLength = 0;
	int exactMatchLength = 0;
	int distanceMatchLength = 0;

	if((alpha == NULL) || (alphaLength < 1) || (beta == NULL) ||
			(betaLength < 1)) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s->%s():%i] error - invalid or missing "
					"arguments to %s()", (char *)file, (char *)function,
					(int)line, __FUNCTION__);
			if(alpha == NULL) {
				fprintf(stderr, ", alpha is NULL");
			}
			if(alphaLength < 1) {
				fprintf(stderr, ", alphaLength is %i", alphaLength);
			}
			if(beta == NULL) {
				fprintf(stderr, ", beta is NULL");
			}
			if(betaLength < 1) {
				fprintf(stderr, ", betaLength is %i", betaLength);
			}
			fprintf(stderr, ".\n");
		}
		return 0.0;
	}

	for(ii = 0; ((ii < alphaLength) && (ii < betaLength)); ii++) {
		if(alpha[ii] == beta[ii]) {
			exactMatchLength++;
		}
		else {
			found = afalse;
			distance = 0;

			for(nn = (ii - 1); ((nn >= 0) && (nn > (ii - 4))); nn--) {
				distance++;
				if(alpha[ii] == beta[nn]) {
					found = atrue;
					break;
				}
			}

			if(!found) {
				distance = 0;

				for(nn = (ii + 1);
						((nn < betaLength) && (nn < (ii + 4)));
						nn++) {
					distance++;
					if(alpha[ii] == beta[nn]) {
						found = atrue;
						break;
					}
				}
			}

			if(found) {
				distanceMatchLength++;
				totalDistance += (distance + 1);
			}
		}
	}

	if((exactMatchLength < 1) && (distanceMatchLength < 1)) {
		return 0.0;
	}

	averageLength = ((alphaLength + betaLength) / 2);

	if(exactMatchLength > averageLength) {
		averageLength = exactMatchLength;
	}

	if(exactMatchLength == averageLength) {
		if(alphaLength > betaLength) {
			return ((double)betaLength / (double)alphaLength);
		}
		else if(alphaLength < betaLength) {
			return ((double)alphaLength / (double)betaLength);
		}
		return 1.0;
	}

	if((exactMatchLength > 0) && (distanceMatchLength < 1)) {
		return ((double)exactMatchLength / (double)averageLength);
	}
	else if((exactMatchLength < 1) && (distanceMatchLength > 0)) {
		return (((double)distanceMatchLength / (double)averageLength) /
				((double)totalDistance / (double)distanceMatchLength));
	}
	else if((exactMatchLength > 0) && (distanceMatchLength > 0)) {
		return (((double)exactMatchLength / (double)averageLength) +
				(((double)distanceMatchLength / (double)averageLength) /
				 ((double)totalDistance / (double)distanceMatchLength)));
	}

	return 0.0;
}

double string_compareArrays(char **alphaArray, int alphaArrayLength,
		char **betaArray, int betaArrayLength, int maxDistance,
		const char *file, const char *function, size_t line)
{
	aboolean found = afalse;
	int ii = 0;
	int nn = 0;
	int alphaLength = 0;
	int betaLength = 0;
	int distance = 0;
	int totalDistance = 0;
	int highestMatchDistance = 0;
	int averageLength = 0;
	double match = 0.0;
	double highestMatch = 0.0;
	double exactMatches = 0.0;
	double distanceMatches = 0.0;

	if((alphaArray == NULL) || (alphaArrayLength < 1) || (betaArray == NULL) ||
			(betaArrayLength < 1)) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s->%s():%i] error - invalid or missing "
					"arguments to %s()", (char *)file, (char *)function,
					(int)line, __FUNCTION__);
			if(alphaArray == NULL) {
				fprintf(stderr, ", alphaArray is NULL");
			}
			if(alphaArrayLength < 1) {
				fprintf(stderr, ", alphaArrayLength is %i", alphaArrayLength);
			}
			if(betaArray == NULL) {
				fprintf(stderr, ", betaArray is NULL");
			}
			if(betaArrayLength < 1) {
				fprintf(stderr, ", betaArrayLength is %i", betaArrayLength);
			}
			fprintf(stderr, ".\n");
		}
		return 0.0;
	}

	for(ii = 0; ((ii < alphaArrayLength) && (ii < betaArrayLength)); ii++) {
		alphaLength = strlen(alphaArray[ii]);
		betaLength = strlen(betaArray[ii]);

		if((match = string_compareStrings(alphaArray[ii], alphaLength,
						betaArray[ii], betaLength, file, function,
						line)) == 1.0) {
			exactMatches += match;
		}
		else {
			found = afalse;
			distance = 0;
			highestMatch = match;
			highestMatchDistance = 0;

			for(nn = (ii - 1); ((nn >= 0) && (nn > (ii - maxDistance))); nn--) {
				distance++;

				betaLength = strlen(betaArray[nn]);

				if((match = string_compareStrings(alphaArray[ii], alphaLength,
								betaArray[nn], betaLength, file, function,
								line)) == 1.0) {
					found = atrue;
					break;
				}
				else if(match > highestMatch) {
					highestMatch = match;
					highestMatchDistance = distance;
				}
			}

			if(!found) {
				distance = 0;

				for(nn = (ii + 1);
						((nn < betaArrayLength) && (nn < (ii + maxDistance)));
						nn++) {
					distance++;

					betaLength = strlen(betaArray[nn]);

					if((match = string_compareStrings(alphaArray[ii],
									alphaLength, betaArray[nn], betaLength,
									file, function, line)) == 1.0) {
						found = atrue;
						break;
					}
					else if(match > highestMatch) {
						highestMatch = match;
						highestMatchDistance = distance;
					}
				}
			}

			if(found) {
				distanceMatches += 1.0;
				totalDistance += (distance + 1);
			}
			else {
				distanceMatches += highestMatch;
				totalDistance += (highestMatchDistance + 1);
			}
		}
	}

	if((exactMatches <= 0.0) && (distanceMatches <= 0.0)) {
		return 0.0;
	}

	averageLength = ((alphaArrayLength + betaArrayLength) / 2);

	if((int)exactMatches > averageLength) {
		averageLength = (int)exactMatches;
	}

	if((int)exactMatches == averageLength) {
		if(alphaLength > betaLength) {
			return ((double)betaLength / (double)alphaLength);
		}
		else if(alphaLength < betaLength) {
			return ((double)alphaLength / (double)betaLength);
		}
		return (exactMatches / (double)averageLength);
	}

	if((exactMatches > 0.0) && (distanceMatches <= 0.0)) {
		return (exactMatches / (double)averageLength);
	}
	else if((exactMatches <= 0.0) && (distanceMatches > 0.0)) {
		return ((distanceMatches / (double)averageLength) /
				((double)totalDistance / distanceMatches));
	}
	else if((exactMatches > 0.0) && (distanceMatches > 0.0)) {
		return ((exactMatches / (double)averageLength) +
				((distanceMatches / (double)averageLength) /
				 ((double)totalDistance / distanceMatches)));
	}

	return 0.0;
}

char *string_serializeArray(char **stringArray, int *stringArrayLengths,
		int arrayLength, int *resultLength, const char *file,
		const char *function, size_t line)
{
	int ii = 0;
	int length = 0;
	int headerLength = 0;
	char *stringPtr = NULL;
	char *result = NULL;
	char **resultArrayReference = NULL;

	if((stringArray == NULL) || (stringArrayLengths == NULL) ||
			(arrayLength < 1) || (resultLength == NULL)) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s->%s():%i] error - invalid or missing "
					"arguments to %s()", (char *)file, (char *)function,
					(int)line, __FUNCTION__);
			if(stringArray == NULL) {
				fprintf(stderr, ", stringArray is NULL");
			}
			if(stringArrayLengths == NULL) {
				fprintf(stderr, ", stringArrayLengths is NULL");
			}
			if(arrayLength < 1) {
				fprintf(stderr, ", arrayLength is %i", arrayLength);
			}
			if(resultLength == NULL) {
				fprintf(stderr, ", resultLength is NULL");
			}
			else {
				*resultLength = 0;
			}
			fprintf(stderr, ".\n");
		}
		return NULL;
	}

	*resultLength = 0;

	headerLength = (int)(sizeof(char **) * arrayLength);
	length = headerLength;

	for(ii = 0; ii < arrayLength; ii++) {
		length += (stringArrayLengths[ii] + 1);
	}

	result = (char *)MEMORY_MALLOC_FUNCTION(
			(sizeof(char) * (length + 1)),
			file, function, line);

	resultArrayReference = ((char **)result);
	stringPtr = (result + headerLength);

	for(ii = 0; ii < arrayLength; ii++) {
		strncpy(stringPtr, stringArray[ii], stringArrayLengths[ii]);

		resultArrayReference[ii] = stringPtr;

		stringPtr += (stringArrayLengths[ii] + 1);

		if((ii < (arrayLength - 1)) && ((int)(stringPtr - result) >= length)) {
			MEMORY_FREE_FUNCTION(result, file, function, line);
			return NULL;
		}
	}

	if((int)(stringPtr - result) != length) {
		MEMORY_FREE_FUNCTION(result, file, function, line);
		return NULL;
	}

	*resultLength = length;

	return result;
}

