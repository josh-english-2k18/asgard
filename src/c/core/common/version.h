/*
 * version.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard version-macros, header file.
 *
 * Written by Josh English.
 */

#if !defined(_CORE_COMMON_VERSION_H)

#define _CORE_COMMON_VERSION_H

#if !defined(_CORE_H) && !defined(_CORE_COMMON_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _CORE_H || _CORE_COMMON_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// define version macros

#define FROM_DEFINE(arg)				#arg
#define DEFINE_TO_STRING(arg)			FROM_DEFINE(arg)

#define ASGARD_VERSION					DEFINE_TO_STRING(VERSION)
#define ASGARD_DATE						DEFINE_TO_STRING(VERSION_DATE)


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _CORE_COMMON_VERSION_H

