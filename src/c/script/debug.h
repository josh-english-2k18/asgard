/*
 * debug.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The SpiderMonkey JavaScript engine for Asgard, debugging function library,
 * header file.
 *
 * Written by Josh English.
 */

#if !defined(_SCRIPT_DEBUG_H)

#define _SCRIPT_DEBUG_H

#if !defined(_SCRIPT_H) && !defined(_SCRIPT_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _SCRIPT_H || _SCRIPT_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// define debug public constants

//#define SCRIPT_DEBUG_DISPLAY_NATIVE_CALLS				atrue
#define SCRIPT_DEBUG_DISPLAY_NATIVE_CALLS				afalse

//#define SCRIPT_DEBUG_NATIVE_CALL_DISPLAY_STREAM			stdout
#define SCRIPT_DEBUG_NATIVE_CALL_DISPLAY_STREAM			stderr


// declare debug public functions

void scriptDebug_displayNativeFunctionCall(const char *file,
		const char *function, JSContext *context, JSObject *object, uintN argc,
		jsval *argv);


// define debug public macros

#define SCRIPT_DISPLAY_NATIVE_FUNCTION_CALL \
	if(SCRIPT_DEBUG_DISPLAY_NATIVE_CALLS) { \
		scriptDebug_displayNativeFunctionCall(__FILE__, __FUNCTION__, context, \
				object, argc, argv); \
	}


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _SCRIPT_DEBUG_H

