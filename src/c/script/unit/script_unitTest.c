/*
 * script_unitTest.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The SpiderMonkey JavaScript engine for Asgard unit test application.
 *
 * Written by Josh English.
 */

// preprocessor directives

#define _CORE_COMMON_BYPASS_STD_STREAM
#include "core/core.h"
#include "script/script.h"


// declare unit test functions

static JSBool native_argCheckFunction(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval);

static int basicFunctionalityTest(ScriptEngine *scriptEngine);


// define unit test constants

static JSFunctionSpec FUNCTION_LIST[] = {
	{	"argCheckFunction",		native_argCheckFunction,			0, 0, 0	},
	{	NULL, NULL, 0, 0, 0	}
};


// main function

int main(int argc, char *argv[])
{
	ScriptEngine scriptEngine;

	signal_registerDefault();

	printf("SpiderMonkey Scripting Engine Unit Test (Asgard Ver %s on %s)\n",
			ASGARD_VERSION, ASGARD_DATE);

	scriptCommon_init(&scriptEngine, 0, NULL);

	printf("Using SpiderMonkey Ver %s\n", script_getVersion(&scriptEngine));

	// set native types

	scriptTypes_initFromEngine(&scriptEngine);

	// set native context

	native_setScriptEngineContext(&scriptEngine);

	if(!native_registerConstants()) {
		fprintf(stderr, "[%s():%i] error - application aborted here.\n",
				__FUNCTION__, __LINE__);
		return 1;
	}

	if(!native_registerFunctions()) {
		fprintf(stderr, "[%s():%i] error - application aborted here.\n",
				__FUNCTION__, __LINE__);
		return 1;
	}

	// load unit-test native functions

	if(!script_defineFunctions(&scriptEngine, FUNCTION_LIST)) {
		fprintf(stderr, "[%s():%i] error - application aborted here.\n",
				__FUNCTION__, __LINE__);
		return 1;
	}

	// execute tests

	if(basicFunctionalityTest(&scriptEngine) < 0) {
		fprintf(stderr, "[%s():%i] error - application aborted here.\n",
				__FUNCTION__, __LINE__);
		return 1;
	}

	// collect garbage

	if(!script_collectGarbage(&scriptEngine)) {
		fprintf(stderr, "[%s():%i] error - application aborted here.\n",
				__FUNCTION__, __LINE__);
		return 1;
	}

	if(!script_suggestCollectGarbage(&scriptEngine)) {
		fprintf(stderr, "[%s():%i] error - application aborted here.\n",
				__FUNCTION__, __LINE__);
		return 1;
	}

	// dump the heap (only works in debug mode)

	script_dumpHeap(&scriptEngine, stderr);

	// cleanup

	scriptCommon_free(&scriptEngine);

	return 0;
}


// define unit test functions

static JSBool native_argCheckFunction(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval)
{
	int ii = 0;

	SCRIPT_DISPLAY_NATIVE_FUNCTION_CALL;

	for(ii = 0; ii < (int)argc; ii++) {
		printf("[unit] argument check (boolean) :: [%02i} => %i\n", ii,
				scriptVar_getBooleanArg(ii, context, argc, argv));
	}

	for(ii = 0; ii < (int)argc; ii++) {
		printf("[unit] argument check (integer) [%02i} => %i\n", ii,
				scriptVar_getIntegerArg(ii, context, argc, argv));
	}

	for(ii = 0; ii < (int)argc; ii++) {
		printf("[unit] argument check (double) [%02i} => %0.6f\n", ii,
				scriptVar_getDoubleArg(ii, context, argc, argv));
	}

	for(ii = 0; ii < (int)argc; ii++) {
		printf("[unit] argument check (string) [%02i} => '%s'\n", ii,
				scriptVar_getStringArg(ii, context, argc, argv));
	}

	*rval = scriptVar_booleanToJsval(JS_TRUE);

	return JS_TRUE;
}

static int basicFunctionalityTest(ScriptEngine *scriptEngine)
{
	char *resultValue = NULL;
	char *script = "argCheckFunction('Hello world!',"
		 "'this is a test string', 123, 12345.234, false, null, undefined);";

	jsval argv[4];

	SpiderMonkey *spiderMonkey = NULL;
	ScriptNativeVar *nativeVar = NULL;
	ScriptEngineContext *context = NULL;

	// obtain the context

	if((context = scriptCommon_getContext(scriptEngine)) == NULL) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	spiderMonkey = &(context->spiderMonkey);

	// evaluate a simple script, calling a native arg-checking function

	if(!script_includeScriptString(scriptEngine, script, strlen(script),
				&resultValue)) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	printf("[unit] script result: %s\n", resultValue);

	// create native arguments, and execute a native function

	argv[0] = scriptVar_stringToJsval(spiderMonkey->context, "this is a test");
	argv[1] = scriptVar_booleanToJsval(atrue);
	argv[2] = scriptVar_integerToJsval(1234);
	argv[3] = scriptVar_doubleToJsval(spiderMonkey->context, 1234.567);

	if(!script_execFunction(scriptEngine, "argCheckFunction", 4, argv,
				&resultValue)) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	printf("[unit] function result: %s\n", resultValue);

	// set a native variable for access in a script

	if(!scriptTypes_registerBooleanVariable(scriptEngine, "global",
				"nativeBooleanVar", atrue)) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(!scriptTypes_registerIntegerVariable(scriptEngine, "global",
				"nativeIntegerVar", 12345)) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(!scriptTypes_registerDoubleVariable(scriptEngine, "global",
				"nativeDoubleVar", 6789.1234)) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(!scriptTypes_registerStringVariable(scriptEngine, "global",
				"nativeStringVar", "this is a test")) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	// load a script from a file

	if(!script_includeScript(scriptEngine, "src/js/test/smtest.js",
				&resultValue)) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
			__FUNCTION__, __LINE__);
		return -1;
	}

	printf("[unit] script result: %s\n", resultValue);

	// execute a JavaScript function from native code

	if(!script_execFunction(scriptEngine, "myScriptFunc", 0, NULL,
				&resultValue)) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	printf("[unit] function result: %s\n", resultValue);

	// get a native variable from a script

	if((nativeVar = scriptTypes_getVariable(scriptEngine, "script",
				"nativeBooleanVar")) == NULL) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	printf("[unit] script native Boolean [script/nativeBooleanVar]: %i\n",
			(int)scriptTypes_getBooleanValueOfNativeVar(nativeVar));

	if((nativeVar = scriptTypes_getVariable(scriptEngine, "script",
				"nativeIntegerVar")) == NULL) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	printf("[unit] script native Integer [script/nativeIntegerVar]: %i\n",
			scriptTypes_getIntegerValueOfNativeVar(nativeVar));

	if((nativeVar = scriptTypes_getVariable(scriptEngine, "script",
				"nativeDoubleVar")) == NULL) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	printf("[unit] script native Double [script/nativeDoubleVar]: %0.6f\n",
			scriptTypes_getDoubleValueOfNativeVar(nativeVar));

	if((nativeVar = scriptTypes_getVariable(scriptEngine, "script",
				"nativeStringVar")) == NULL) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	printf("[unit] script native String [script/nativeStringVar]: '%s'\n",
			scriptTypes_getStringValueOfNativeVar(nativeVar));

	// execute the Asgard Native API test

	if(!script_execFunction(scriptEngine, "executeAsgardNativeApiTest",
				0, NULL, &resultValue)) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	printf("[unit] function result: %s\n", resultValue);

	return 0;
}

