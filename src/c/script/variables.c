/*
 * variables.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The SpiderMonkey JavaScript engine for Asgard, variable-handling function
 * library.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#define _SCRIPT_COMPONENT
#include "script/common.h"
#include "script/debug.h"
#include "script/variables.h"


// define variables public functions

aboolean scriptVar_jsvalToBoolean(JSContext *context, jsval *value)
{
	aboolean result = afalse;

	JSBool localValue = JS_FALSE;

	if((context == NULL) || (value == NULL) || (JSVAL_IS_NULL(*value)) ||
			(JSVAL_IS_VOID(*value))) {
		return afalse;
	}

	if(JSVAL_IS_BOOLEAN(*value)) {
		if((JSVAL_TO_BOOLEAN(*value)) == JS_TRUE) {
			result = atrue;
		}
		else {
			result = afalse;
		}
	}
	else {
		if(JS_ValueToBoolean(context, *value, &localValue)) {
			if(localValue == JS_TRUE) {
				result = atrue;
			}
			else {
				result = afalse;
			}
		}
		else {
			result = afalse;
		}
	}

	return result;
}

jsval scriptVar_booleanToJsval(aboolean value)
{
	jsval result;
	JSBool localValue = JS_FALSE;

	if(value) {
		localValue = JS_TRUE;
	}

	result = BOOLEAN_TO_JSVAL(localValue);

	return result;
}

aboolean scriptVar_getBooleanArg(int id, JSContext *context, uintN argc,
		jsval *argv)
{
	if(((int)argc <= 0) || (id >= (int)argc) || (argv == NULL)) {
		return afalse;
	}

	return scriptVar_jsvalToBoolean(context, &(argv[id]));
}

int scriptVar_jsvalToInteger(JSContext *context, jsval *value)
{
	int result = 0;

	int32 localValue = (int32)0;

	if((context == NULL) || (value == NULL) || (JSVAL_IS_NULL(*value)) ||
			(JSVAL_IS_VOID(*value))) {
		return 0;
	}

	if(JSVAL_IS_INT(*value)) {
		result = JSVAL_TO_INT(*value);
	}
	else {
		if(JS_ValueToInt32(context, *value, &localValue)) {
			result = (int)localValue;
		}
	}

	return result;
}

jsval scriptVar_integerToJsval(int value)
{
	jsval result;

	result = INT_TO_JSVAL(value);

	return result;
}

int scriptVar_getIntegerArg(int id, JSContext *context, uintN argc,
		jsval *argv)
{
	if(((int)argc <= 0) || (id >= (int)argc) || (argv == NULL)) {
		return 0;
	}

	return scriptVar_jsvalToInteger(context, &(argv[id]));
}

double scriptVar_jsvalToDouble(JSContext *context, jsval *value)
{
	double result = 0.0;

	jsdouble localValue = (jsdouble)0.0;

	if((context == NULL) || (value == NULL) || (JSVAL_IS_NULL(*value)) ||
			(JSVAL_IS_VOID(*value))) {
		return 0.0;
	}

	if(JSVAL_IS_DOUBLE(*value)) {
		result = *((double *)JSVAL_TO_DOUBLE(*value));
	}
	else {
		if(JS_ValueToNumber(context, *value, &localValue)) {
			result = (double)localValue;
		}
	}

	return result;
}

jsval scriptVar_doubleToJsval(JSContext *context, double value)
{
	jsval result;

	if(JS_NewNumberValue(context, (jsdouble)value, &result) == JS_FALSE) {
		result = BOOLEAN_TO_JSVAL(JS_FALSE);
	}

	return result;
}

double scriptVar_getDoubleArg(int id, JSContext *context, uintN argc,
		jsval *argv)
{
	if(((int)argc <= 0) || (id >= (int)argc) || (argv == NULL)) {
		return 0.0;
	}

	return scriptVar_jsvalToDouble(context, &(argv[id]));
}

char *scriptVar_jsvalToString(JSContext *context, jsval *value)
{
	char *result = NULL;

	JSString *localValue = NULL;

	if((context == NULL) || (value == NULL) || (JSVAL_IS_NULL(*value)) ||
			(JSVAL_IS_VOID(*value))) {
		return NULL;
	}

	if(JSVAL_IS_STRING(*value)) {
		result = JS_GetStringBytes(JSVAL_TO_STRING(*value));
	}
	else {
		if((localValue = JS_ValueToString(context, *value)) != NULL) {
			result = JS_GetStringBytes(localValue);
		}
	}

	return result;
}

jsval scriptVar_stringToJsval(JSContext *context, char *value)
{
	jsval result;
	JSString *string = (JSString *)NULL;

	if((string = JS_NewStringCopyN(context, value, strlen(value))) != NULL) {
		result = STRING_TO_JSVAL(string);
	}
	else {
		result = BOOLEAN_TO_JSVAL(JS_FALSE);
	}

	return result;
}

char *scriptVar_getStringArg(int id, JSContext *context, uintN argc,
		jsval *argv)
{
	if(((int)argc <= 0) || (id >= (int)argc) || (argv == NULL)) {
		return NULL;
	}

	return scriptVar_jsvalToString(context, &(argv[id]));
}

