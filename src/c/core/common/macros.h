/*
 * macros.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The master macros header-file for Asgard.
 *
 * Written by Josh English.
 */

#if !defined(_CORE_COMMON_MACROS_H)

#define _CORE_COMMON_MACROS_H

#if !defined(_CORE_H) && !defined(_CORE_COMMON_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _CORE_H || _CORE_COMMON_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// define global macros

// common usage constants

#define INT_ONE_THOUSAND					1000

#define REAL_ONE_THOUSAND					1000.0

#define INT_ONE_MILLION						1000000

#define REAL_ONE_MILLION					1000000.0

#define INT_ONE_BILLION						1000000000

#define REAL_ONE_BILLION					1000000000.0

// compiler compatibility

#if !defined(__ANDROID__)
#	define MAX_SIGNED_CHAR						127
#else // __ANDROID__
#	define MAX_SIGNED_CHAR						255
#endif // !__ANDROID__

#if (MAX_SIGNED_CHAR != CHAR_MAX)
#	error "Determined MAX_SIGNED_CHAR is incorrect for this compiler."
#endif

#define MAX_UNSIGNED_CHAR					255u

#if (MAX_UNSIGNED_CHAR != UCHAR_MAX)
#	error "Determined MAX_UNSIGNED_CHAR is incorrect for this compiler."
#endif

#define MAX_SIGNED_SHORT					32767

#if (MAX_SIGNED_SHORT != SHRT_MAX)
#	error "Determined MAX_SIGNED_SHORT is incorrect for this compiler."
#endif

#define MAX_UNSIGNED_SHORT					65535u

#if (MAX_UNSIGNED_SHORT != USHRT_MAX)
#	error "Determined MAX_UNSIGNED_SHORT is incorrect for this compiler."
#endif

#define MAX_SIGNED_INT						2147483647

#if (MAX_SIGNED_INT != INT_MAX)
#	error "Determined MAX_SIGNED_INT is incorrect for this compiler."
#endif

#define MAX_UNSIGNED_INT					4294967295u

#if (MAX_UNSIGNED_INT != UINT_MAX)
#	error "Determined MAX_UNSIGNED_INT is incorrect for this compiler."
#endif

#define MAX_SIGNED_FLOAT					\
	340282346638528859811704183484516925440.0

#if defined(MINGW)
#	define MAX_SIGNED_LONG_INT				2147483647
#else // !MINGW
#	define MAX_SIGNED_LONG_INT				9223372036854775807
#endif // MINGW

#if (MAX_SIGNED_LONG_INT != LONG_MAX) && !defined(WIN32) && !defined(__APPLE__) && !defined(__ANDROID__)
#	error "Determined MAX_SIGNED_LONG_INT is incorrect for this compiler."
#endif

#if defined(MINGW)
#	define MAX_UNSIGNED_LONG_INT			4294967295u
#else // !MINGW
#	define MAX_UNSIGNED_LONG_INT			18446744073709551615u
#endif // MINGW

#if (MAX_UNSIGNED_LONG_INT != ULONG_MAX) && !defined(WIN32) && !defined(__APPLE__) && !defined(__ANDROID__)
#	error "Determined MAX_UNSIGNED_LONG_INT is incorrect for this compiler."
#endif

#if defined(WIN32)

#define MAX_SIGNED_DOUBLE					(double)MAX_SIGNED_LONG_INT

#else // !WIN32

#define MAX_SIGNED_DOUBLE					179769313486231570814527423731704356798070567525844996598917476803157260780028538760589558632766878171540458953514382464234321326889464182768467546703537516986049910576551282076245490090389328944075868508455133942304583236903222948165808559332123348274797826204144723168738177180919299881250404026184124858368.0

#endif // WIN32

// directory structure

#if defined(WIN32)

#define DIR_SEPARATOR						'\\'

#else // !WIN32

#define DIR_SEPARATOR						'/'

#endif // WIN32

// set file offsets to 64-bit

#if defined(_FILE_OFFSET_BITS)
#	undef _FILE_OFFSET_BITS
#endif // _FILE_OFFSET_BITS

#define _FILE_OFFSET_BITS					64

#define EXPLICIT_ERRORS						1

#define DISPLAY_INVALID_ARGS \
	if(EXPLICIT_ERRORS) { \
		fprintf(stderr, "[%s():%i] error - invalid or missing argument(s).\n", \
				__FUNCTION__, __LINE__); \
	}

// cast characters to signed integers

#define chartoint							int)(unsigned int


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _CORE_COMMON_MACROS_H

