/*
 * variables.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The SpiderMonkey JavaScript engine for Asgard, variable-handling function
 * library, header file.
 *
 * Written by Josh English.
 */

#if !defined(_SCRIPT_VARIABLES_H)

#define _SCRIPT_VARIABLES_H

#if !defined(_SCRIPT_H) && !defined(_SCRIPT_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _SCRIPT_H || _SCRIPT_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// declare variable public functions

aboolean scriptVar_jsvalToBoolean(JSContext *context, jsval *value);

jsval scriptVar_booleanToJsval(aboolean value);

aboolean scriptVar_getBooleanArg(int id, JSContext *context, uintN argc,
		jsval *argv);

int scriptVar_jsvalToInteger(JSContext *context, jsval *value);

jsval scriptVar_integerToJsval(int value);

int scriptVar_getIntegerArg(int id, JSContext *context, uintN argc,
		jsval *argv);

double scriptVar_jsvalToDouble(JSContext *context, jsval *value);

jsval scriptVar_doubleToJsval(JSContext *context, double value);

double scriptVar_getDoubleArg(int id, JSContext *context, uintN argc,
		jsval *argv);

char *scriptVar_jsvalToString(JSContext *context, jsval *value);

jsval scriptVar_stringToJsval(JSContext *context, char *value);

char *scriptVar_getStringArg(int id, JSContext *context, uintN argc,
		jsval *argv);


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _SCRIPT_VARIABLES_H

