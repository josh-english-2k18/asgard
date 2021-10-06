/*
 * memory.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard memory-wrapping functions and macros, header file.
 *
 * Written by Josh English.
 */

#if !defined(_CORE_MEMORY_MEMORY_H)

#define _CORE_MEMORY_MEMORY_H

#if !defined(_CORE_H) && !defined(_CORE_MEMORY_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _CORE_H || _CORE_MEMORY_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

// delcare memory public functions

void memory_init();

void *memory_allocate(int length, const char *file, const char *function,
		size_t line);

void *memory_reallocate(void *value, int length, const char *file,
		const char *function, size_t line);

void memory_free(void *value, const char *file, const char *function,
		size_t line);

void memory_systemFree(void *value, const char *file, const char *function,
		size_t line);

char *memory_strdup(char *string, const char *file, const char *function,
		size_t line);

char *memory_strndup(char *string, int length, const char *file,
		const char *function, size_t line);

char *memory_strxpndcat(char *string, int *stringRef, int *stringLength,
		char *source, int sourceLength, const char *file, const char *function,
		size_t line);

char *memory_itoa(int value, const char *file, const char *function,
		size_t line);

char *memory_bitoa(alint value, const char *file, const char *function,
		size_t line);

char *memory_dtoa(double value, const char *file, const char *function,
		size_t line);

void memory_displayStatus();

void memory_displaySlotProfile();

void *memory_getState();

int memory_setState(void *state);


// define memory public macros

#if !defined(MEMORY_NATIVE)
#	define MEMORY_PROTECT
#endif // !MEMORY_NATIVE


#if defined(MEMORY_PROTECT)

#define MEMORY_REPLACED


#undef malloc
#define malloc(length) \
	memory_allocate(length, __FILE__, __FUNCTION__, __LINE__)

#undef calloc
#define calloc(length) \
	memory_allocate(length, __FILE__, __FUNCTION__, __LINE__)

#undef realloc
#define realloc(addr, length) \
	memory_reallocate(addr, length, __FILE__, __FUNCTION__, __LINE__)

#undef free
#define free(addr) \
	memory_free(addr, __FILE__, __FUNCTION__, __LINE__)

#undef systemFree
#define systemFree(addr) \
	memory_systemFree(addr, __FILE__, __FUNCTION__, __LINE__)

#undef strdup
#define strdup(string) \
	memory_strdup(string, __FILE__, __FUNCTION__, __LINE__)

#undef strndup
#define strndup(string, length) \
	memory_strndup(string, length, __FILE__, __FUNCTION__, __LINE__)

#undef strxpndcat
#define strxpndcat(string, stringRef, stringLength, source, sourceLength) \
	memory_strxpndcat(string, stringRef, stringLength, source, sourceLength, \
			__FILE__, __FUNCTION__, __LINE__)

#undef itoa
#define itoa(value) \
	memory_itoa(value, __FILE__, __FUNCTION__, __LINE__)

#undef bitoa
#define bitoa(value) \
	memory_bitoa(value, __FILE__, __FUNCTION__, __LINE__)

#undef dtoa
#define dtoa(value) \
	memory_dtoa(value, __FILE__, __FUNCTION__, __LINE__)


#endif // MEMORY_PROTECT


// define memory public constants

#define MEMORY_MALLOC_FUNCTION			memory_allocate

#define MEMORY_CALLOC_FUNCTION			memory_allocate

#define MEMORY_REALLOC_FUNCTION			memory_reallocate

#define MEMORY_FREE_FUNCTION			memory_free

#define MEMORY_STRDUP_FUNCTION			memory_strdup

#define MEMORY_STRNDUP_FUNCTION			memory_strndup


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _CORE_MEMORY_MEMORY_H

