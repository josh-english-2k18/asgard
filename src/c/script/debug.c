/*
 * debug.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The SpiderMonkey JavaScript engine for Asgard, debugging function library.
 *
 * Written by Josh English.
 */

// preprocessor directives

#define _CORE_COMMON_BYPASS_STD_STREAM
#include "core/core.h"
#define _SCRIPT_COMPONENT
#include "script/common.h"
#include "script/debug.h"


// define debug public functions

void scriptDebug_displayNativeFunctionCall(const char *file,
		const char *function, JSContext *context, JSObject *object, uintN argc,
		jsval *argv)
{
	int ii = 0;
	int localArgc = 0;
	char *type = NULL;
	char *string = NULL;

	JSString *jstring = NULL;

	if(!SCRIPT_DEBUG_DISPLAY_NATIVE_CALLS) {
		return;
	}

	localArgc = (int)argc;

	fprintf(SCRIPT_DEBUG_NATIVE_CALL_DISPLAY_STREAM,
			"[%s->%s()] { context: 0x%lx, object: 0x%lx } with %i arg(s)",
			(char *)file,
			(char *)function,
			(unsigned long int)context,
			(unsigned long int)object,
			(int)argc);

	if((localArgc > 0) && (argv != NULL)) {
		fprintf(SCRIPT_DEBUG_NATIVE_CALL_DISPLAY_STREAM, " [ ");

		for(ii = 0; ii < localArgc; ii++) {
			if(JSVAL_IS_NULL(argv[ii])) {
				type = "null";
			}
			else if(JSVAL_IS_VOID(argv[ii])) {
				type = "void";
			}
			else if(JSVAL_IS_BOOLEAN(argv[ii])) {
				type = "boolean";
			}
			else if(JSVAL_IS_INT(argv[ii])) {
				type = "integer";
			}
			else if(JSVAL_IS_DOUBLE(argv[ii])) {
				type = "double";
			}
			else if(JSVAL_IS_STRING(argv[ii])) {
				type = "string";
			}
			else if(JSVAL_IS_OBJECT(argv[ii])) {
				type = "object";
			}
	
			if((jstring = JS_ValueToString(context, argv[ii])) != NULL) {
				string = JS_GetStringBytes(jstring);
			}
			else {
				string = "unknown";
			}

			fprintf(SCRIPT_DEBUG_NATIVE_CALL_DISPLAY_STREAM,
					"(%i){%s='%s'}", ii, type, string);

			if(ii < (localArgc - 1)) {
				fprintf(SCRIPT_DEBUG_NATIVE_CALL_DISPLAY_STREAM, ", ");
			}
		}

		fprintf(SCRIPT_DEBUG_NATIVE_CALL_DISPLAY_STREAM, " ]");
	}

	fprintf(SCRIPT_DEBUG_NATIVE_CALL_DISPLAY_STREAM,
			" from 0x%lx.\n", (unsigned long int)argv);
}

