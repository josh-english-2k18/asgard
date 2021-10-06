/*
 * types.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard types definitions.
 *
 * Written by Josh English.
 */

#if !defined(_CORE_OS_TYPES_H)

#define _CORE_OS_TYPES_H

#if !defined(_CORE_H) && !defined(_CORE_OS_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _CORE_H || _CORE_OS_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// define types

// type: boolean - aboolean

#if defined(aboolean) || defined(SIZEOF_BOOLEAN) || defined(atrue) || \
		defined(afalse)
#	warn "Redefining 'aboolean' type, check your compiler environment"
#	undef aboolean
#	undef SIZEOF_BOOLEAN
#endif // aboolean

typedef unsigned char									aboolean;

#define SIZEOF_BOOLEAN									sizeof(aboolean)

#define atrue											1
#define afalse											0

// type: 64-bit integer - alint

#if defined(alint) || defined(SIZEOF_ALINT)
#	warn "Redefining 'alint' type, check your compiler environment"
#	undef alint
#	undef SIZEOF_ALINT
#endif // alint

typedef signed long long								alint;

#define SIZEOF_ALINT									sizeof(alint)

// type: unsigned 64-bit integer - alint

#if defined(aulint) || defined(SIZEOF_AULINT)
#	warn "Redefining 'alint' type, check your compiler environment"
#	undef aulint
#	undef SIZEOF_AULINT
#endif // alint

typedef unsigned long long int							aulint;

#define SIZEOF_AULINT									sizeof(aulint)

// type: pointer casting - aptrcast

#if defined(aptrcast) || defined(SIZEOF_APTRCAST)
#	warn "Redefining 'aptrcast' type, check your compiler environment"
#	undef aptrcast
#endif // aptrcast

#if defined(__linux__) && !defined(__ANDROID__)
#	define aptrcast									\
		unsigned long int)(signed long long
#elif defined(MINGW)
#	define aptrcast									\
		unsigned long int)(signed long long int
#elif defined(WIN32)
#	define aptrcast									\
		unsigned long int)(unsigned long long int
#elif defined(__APPLE__)
#	define aptrcast									\
		unsigned long int)(signed long int
#elif defined(__ANDROID__)
#	define aptrcast									\
		unsigned long int)(signed long int
#else // - no plugins available -
#	error "No plugins available for type 'aptrcast'."
#endif // - plugins -

// type: size of a char

#if defined(SIZEOF_CHAR)
#	warn "Redefining 'SIZEOF_CHAR' define, check your compiler environment"
#	undef SIZEOF_CHAR
#endif // SIZEOF_CHAR

#define SIZEOF_CHAR										sizeof(char)

// type: size of a short

#if defined(SIZEOF_SHORT)
#	warn "Redefining 'SIZEOF_SHORT' define, check your compiler environment"
#	undef SIZEOF_SHORT
#endif // SIZEOF_SHORT

#define SIZEOF_SHORT									sizeof(short)

// type: size of an int

#if defined(SIZEOF_INT)
#	warn "Redefining 'SIZEOF_INT' define, check your compiler environment"
#	undef SIZEOF_INT
#endif // SIZEOF_INT

#define SIZEOF_INT										sizeof(int)

// type: size of a float

#if defined(SIZEOF_FLOAT)
#	warn "Redefining 'SIZEOF_FLOAT' define, check your compiler environment"
#	undef SIZEOF_FLOAT
#endif // SIZEOF_FLOAT

#define SIZEOF_FLOAT									sizeof(float)

// type: size of a double

#if defined(SIZEOF_DOUBLE)
#	warn "Redefining 'SIZEOF_DOUBLE' define, check your compiler environment"
#	undef SIZEOF_DOUBLE
#endif // SIZEOF_DOUBLE

#define SIZEOF_DOUBLE									sizeof(double)


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _CORE_OS_TYPES_H

