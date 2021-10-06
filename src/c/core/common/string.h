/*
 * string.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard C string library wrapper functions, header file.
 *
 * Written by Josh English.
 */

#if !defined(_CORE_COMMON_STRING_H)

#define _CORE_COMMON_STRING_H

#if !defined(_CORE_H) && !defined(_CORE_COMMON_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _CORE_H || _CORE_COMMON_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

// declare string library wrapper functions

void *string_memcpy(void *destination, const void *source, size_t length,
		const char *file, const char *function, size_t line);

void *string_memmove(void *destination, const void *source, size_t length,
		const char *file, const char *function, size_t line);

void *string_memset(void *string, int charValue, size_t length,
		const char *file, const char *function, size_t line);

char *string_strchr(const char *string, int charValue, const char *file,
		const char *function, size_t line);

char *string_strrchr(const char *string, int charValue, const char *file,
		const char *function, size_t line);

char *string_strstr(const char *haystack, const char *needle, const char *file,
		const char *function, size_t line);

char *string_strnstr(const char *haystack, int haystackLength,
		const char *needle, int needleLength, const char *file,
		const char *function, size_t line);

char *string_strcasestr(const char *haystack, const char *needle,
		const char *file, const char *function, size_t line);

char *string_strncasestr(const char *haystack, int haystackLength,
		const char *needle, int needleLength, const char *file,
		const char *function, size_t line);

int string_strcmp(const char *stringOne, const char *stringTwo,
		const char *file, const char *function, size_t line);

int string_strcasecmp(const char *stringOne, const char *stringTwo,
		const char *file, const char *function, size_t line);

int string_strncmp(const char *stringOne, const char *stringTwo, size_t length,
		const char *file, const char *function, size_t line);

int string_strncasecmp(const char *stringOne, const char *stringTwo,
		size_t length, const char *file, const char *function, size_t line);

size_t string_strlen(const char *string, const char *file,
		const char *function, size_t line);

char *string_strerror(int errorNumber, const char *file, const char *function,
		size_t line);

char *string_strcpy(char *destination, const char *source, const char *file,
		const char *function, size_t line);

char *string_strncpy(char *destination, const char *source, size_t length,
		const char *file, const char *function, size_t line);

char *string_strcat(char *destination, const char *source, const char *file,
		const char *function, size_t line);

char *string_strncat(char *destination, const char *source, size_t length,
		const char *file, const char *function, size_t line);

char *string_vaprintf(const char *format, va_list vaArgs, int *resultLength,
		const char *file, const char *function, size_t line);

int string_atoi(const char *number, const char *file, const char *function,
		size_t line);

double string_atod(const char *number, const char *file, const char *function,
		size_t line);

int string_strtotokens(char *string, int stringLength, char *delimiters,
		int delimitersLength, int minTokenLength, char ***tokenList,
		int **tokenLengths, int *tokenCount, const char *file,
		const char *function, size_t line);

double string_compareStrings(char *alpha, int alphaLength, char *beta,
		int betaLength, const char *file, const char *function,
		size_t line);

double string_compareArrays(char **alphaArray, int alphaArrayLength,
		char **betaArray, int betaArrayLength, int maxDistance,
		const char *file, const char *function, size_t line);

char *string_serializeArray(char **stringArray, int *stringArrayLengths,
		int arrayLength, int *resultLength, const char *file,
		const char *function, size_t line);


// define string library wrapper macros

#undef memcpy
#define memcpy(destination, source, length) \
	string_memcpy(destination, source, length, __FILE__, __FUNCTION__, \
			__LINE__)

#undef memmove
#define memmove(destination, source, length) \
	string_memmove(destination, source, length, __FILE__, __FUNCTION__, \
			__LINE__)

#undef memset
#define memset(string, charValue, length) \
	string_memset(string, charValue, length, __FILE__, __FUNCTION__, __LINE__)

#undef strchr
#define strchr(string, charValue) \
	string_strchr(string, charValue, __FILE__, __FUNCTION__, __LINE__)

#undef strrchr
#define strrchr(string, charValue) \
	string_strrchr(string, charValue, __FILE__, __FUNCTION__, __LINE__)

#undef strstr
#define strstr(haystack, needle) \
	string_strstr(haystack, needle, __FILE__, __FUNCTION__, __LINE__)

#undef strnstr
#define strnstr(haystack, haystackLength, needle, needleLength) \
	string_strnstr(haystack, haystackLength, needle, needleLength, \
			__FILE__, __FUNCTION__, __LINE__)

#undef strcasestr
#define strcasestr(haystack, needle) \
	string_strcasestr(haystack, needle, __FILE__, __FUNCTION__, __LINE__)

#undef strncasestr
#define strncasestr(haystack, haystackLength, needle, needleLength) \
	string_strncasestr(haystack, haystackLength, needle, needleLength, \
			__FILE__, __FUNCTION__, __LINE__)

#undef strcmp
#define strcmp(stringOne, stringTwo) \
	string_strcmp(stringOne, stringTwo, __FILE__, __FUNCTION__, __LINE__)

#undef strcasecmp
#define strcasecmp(stringOne, stringTwo) \
	string_strcasecmp(stringOne, stringTwo, \
			__FILE__, __FUNCTION__, __LINE__)

#undef strncmp
#define strncmp(stringOne, stringTwo, length) \
	string_strncmp(stringOne, stringTwo, length, __FILE__, __FUNCTION__, \
			__LINE__)

#undef strncasecmp
#define strncasecmp(stringOne, stringTwo, length) \
	string_strncasecmp(stringOne, stringTwo, length, \
			__FILE__, __FUNCTION__, __LINE__)

#undef strlen
#define strlen(string) \
	(int)string_strlen(string, __FILE__, __FUNCTION__, __LINE__)

#undef strerror
#define strerror(errorNumber) \
	string_strerror(errorNumber, __FILE__, __FUNCTION__, __LINE__)

#undef strcpy
#define strcpy(destination, source) \
	string_strcpy(destination, source, __FILE__, __FUNCTION__, __LINE__)

#undef strncpy
#define strncpy(destination, source, length) \
	string_strncpy(destination, source, length, __FILE__, __FUNCTION__, \
			__LINE__)

#undef strcat
#define strcat(destination, source) \
	string_strcat(destination, source, __FILE__, __FUNCTION__, __LINE__)

#undef strncat
#define strncat(destination, source, length) \
	string_strncat(destination, source, length, __FILE__, __FUNCTION__, \
			__LINE__)

#undef vaprintf
#define vaprintf(format, vaArgs, resultLength) \
	string_vaprintf(format, vaArgs, resultLength, __FILE__, __FUNCTION__, \
			__LINE__)

#undef atoi
#define atoi(number) \
	string_atoi(number, __FILE__, __FUNCTION__, __LINE__)

#undef atof
#define atof(number) \
	string_atod(number, __FILE__, __FUNCTION__, __LINE__)

#undef atod
#define atod(number) \
	string_atod(number, __FILE__, __FUNCTION__, __LINE__)

#undef strtotokens
#define strtotokens(string, stringLength, delimiters, delimitersLength, \
		minTokenLength, tokenList, tokenLengths, tokenCount) \
	string_strtotokens(string, stringLength, delimiters, delimitersLength, \
			minTokenLength, tokenList, tokenLengths, tokenCount, \
			__FILE__, __FUNCTION__, __LINE__)

#undef compareStrings
#define compareStrings(alpha, alphaLength, beta, betaLength) \
	string_compareStrings(alpha, alphaLength, beta, betaLength, __FILE__, \
			__FUNCTION__, __LINE__)

#undef compareArrays
#define compareArrays(alphaArray, alphaArrayLength, betaArray, \
		betaArrayLength, maxDistance) \
	string_compareArrays(alphaArray, alphaArrayLength, betaArray, \
			betaArrayLength, maxDistance, __FILE__, __FUNCTION__, __LINE__)

#undef serializeArray
#define serializeArray(stringArray, stringArrayLengths, arrayLength, \
		resultLength) \
	string_serializeArray(stringArray, stringArrayLengths, arrayLength, \
			resultLength, __FILE__, __FUNCTION__, __LINE__)


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _CORE_COMMON_STRING_H

