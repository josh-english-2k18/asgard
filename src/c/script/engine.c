/*
 * engine.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The SpiderMonkey JavaScript engine for Asgard, scripting engine API.
 *
 * Written by Josh English.
 */

// preprocessor directives

#define _CORE_COMMON_BYPASS_STD_STREAM
#include "core/core.h"
#define _SCRIPT_COMPONENT
#include "script/common.h"
#include "script/debug.h"
#include "script/variables.h"
#include "script/functions.h"
#include "script/engine.h"

// define engine public functions

char *script_getVersion(ScriptEngine *scriptEngine)
{
	char *result = NULL;

	ScriptEngineContext *context = NULL;

	if((scriptEngine == NULL) ||
			(scriptEngine->status != SCRIPT_ENGINE_STATUS_RUNNING)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	if((context = scriptCommon_getContext(scriptEngine)) == NULL) {
		return NULL;
	}

	mutex_lock(&context->mutex);

	result = (char *)JS_VersionToString(
			JS_GetVersion(context->spiderMonkey.context));

	mutex_unlock(&context->mutex);

	return result;
}

aboolean script_defineFunctions(ScriptEngine *scriptEngine,
		JSFunctionSpec *functionList)
{
	ScriptEngineContext *context = NULL;

	if((scriptEngine == NULL) ||
			(scriptEngine->status != SCRIPT_ENGINE_STATUS_RUNNING) ||
			(functionList == NULL)) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	if((context = scriptCommon_getContext(scriptEngine)) == NULL) {
		return afalse;
	}

	mutex_lock(&context->mutex);

	if(JS_DefineFunctions(context->spiderMonkey.context,
				context->spiderMonkey.global,
				functionList) == JS_FALSE) {
		log_logf(context->log, LOG_LEVEL_ERROR,
				"[%s->%s()] failed to define functions 0x%lx from %s()",
				__FILE__, __FUNCTION__, (aptrcast)functionList,
				"JS_DefineFunctions");
		mutex_unlock(&context->mutex);
		return afalse;
	}

	mutex_unlock(&context->mutex);

	return atrue;
}

aboolean script_execFunction(ScriptEngine *scriptEngine, char *functionName,
		uintN argc, jsval *argv, char **functionResult)
{
	jsval returnValue;

	ScriptEngineContext *context = NULL;

	if((scriptEngine == NULL) ||
			(scriptEngine->status != SCRIPT_ENGINE_STATUS_RUNNING) ||
			(functionName == NULL)) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	if((context = scriptCommon_getContext(scriptEngine)) == NULL) {
		return afalse;
	}

	mutex_lock(&context->mutex);

	if(JS_CallFunctionName(context->spiderMonkey.context,
				context->spiderMonkey.global,
				functionName,
				argc,
				argv,
				&returnValue) == JS_FALSE) {
		log_logf(context->log, LOG_LEVEL_ERROR,
				"[%s->%s()] failed to exec function '%s' from %s()",
				__FILE__, __FUNCTION__, functionName, "JS_CallFunctionName");
		mutex_unlock(&context->mutex);
		return afalse;
	}

	if(functionResult != NULL) {
		*functionResult = scriptVar_jsvalToString(
				context->spiderMonkey.context,
				&returnValue);
	}

	mutex_unlock(&context->mutex);

	return atrue;
}

aboolean script_includeScriptString(ScriptEngine *scriptEngine, char *script,
		int scriptLength, char **scriptResult)
{
	jsval returnValue;

	ScriptEngineContext *context = NULL;

	if((scriptEngine == NULL) ||
			(scriptEngine->status != SCRIPT_ENGINE_STATUS_RUNNING) ||
			(script == NULL) ||
			(scriptLength <= 0)) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	if((context = scriptCommon_getContext(scriptEngine)) == NULL) {
		return afalse;
	}

	mutex_lock(&context->mutex);

	if(JS_EvaluateScript(context->spiderMonkey.context,
				context->spiderMonkey.global,
				script,
				scriptLength,
				NULL,
				0,
				&returnValue) == JS_FALSE) {
		log_logf(context->log, LOG_LEVEL_ERROR,
				"[%s->%s()] failed to load script 0x%lx from %s()",
				__FILE__, __FUNCTION__, (aptrcast)script,
				"JS_EvaluateScript");
		mutex_unlock(&context->mutex);
		return afalse;
	}

	if(scriptResult != NULL) {
		*scriptResult = scriptVar_jsvalToString(
				context->spiderMonkey.context,
				&returnValue);
	}

	mutex_unlock(&context->mutex);

	return atrue;
}

aboolean script_includeScript(ScriptEngine *scriptEngine, char *filename,
		char **scriptResult)
{
	int rc = 0;
	char *errorMessage = NULL;

	jsval returnValue;
	JSScript *scriptHandle = NULL;

	FileHandle fh;
	ScriptEngineContext *context = NULL;

	if((scriptEngine == NULL) ||
			(scriptEngine->status != SCRIPT_ENGINE_STATUS_RUNNING) ||
			(filename == NULL)) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	if((context = scriptCommon_getContext(scriptEngine)) == NULL) {
		return afalse;
	}

	mutex_lock(&context->mutex);

	if((rc = file_init(&fh, filename, "r", 0)) < 0) {
		if(file_getError(&fh, rc, &errorMessage) < 0) {
			errorMessage = "unknown file error";
		}
		log_logf(context->log, LOG_LEVEL_ERROR,
				"[%s->%s()] failed to open '%s' with '%s'",
				__FILE__, __FUNCTION__, filename, errorMessage);
		file_free(&fh);
		mutex_unlock(&context->mutex);
		return afalse;
	}

	if((scriptHandle = JS_CompileFileHandle(context->spiderMonkey.context,
					context->spiderMonkey.global,
					filename,
					fh.fd)) != NULL) {
		if(JS_ExecuteScript(context->spiderMonkey.context,
					context->spiderMonkey.global,
					scriptHandle,
					&returnValue) == JS_FALSE) {
			log_logf(context->log, LOG_LEVEL_ERROR,
					"[%s->%s()] failed to execute script from '%s'",
					__FILE__, __FUNCTION__, filename);
			file_free(&fh);
			mutex_unlock(&context->mutex);
			return afalse;
		}
	}
	else {
		log_logf(context->log, LOG_LEVEL_ERROR,
				"[%s->%s()] failed to compile script from '%s'",
				__FILE__, __FUNCTION__, filename);
		file_free(&fh);
		mutex_unlock(&context->mutex);
		return afalse;
	}

	file_free(&fh);

	if(scriptResult != NULL) {
		*scriptResult = scriptVar_jsvalToString(
				context->spiderMonkey.context,
				&returnValue);
	}

	mutex_unlock(&context->mutex);

	return atrue;
}

aboolean script_collectGarbage(ScriptEngine *scriptEngine)
{
	ScriptEngineContext *context = NULL;

	if((scriptEngine == NULL) ||
			(scriptEngine->status != SCRIPT_ENGINE_STATUS_RUNNING)) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	if((context = scriptCommon_getContext(scriptEngine)) == NULL) {
		return afalse;
	}

	mutex_lock(&context->mutex);

	JS_GC(context->spiderMonkey.context);

	mutex_unlock(&context->mutex);

	return atrue;
}

aboolean script_suggestCollectGarbage(ScriptEngine *scriptEngine)
{
	ScriptEngineContext *context = NULL;

	if((scriptEngine == NULL) ||
			(scriptEngine->status != SCRIPT_ENGINE_STATUS_RUNNING)) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	if((context = scriptCommon_getContext(scriptEngine)) == NULL) {
		return afalse;
	}

	mutex_lock(&context->mutex);

	JS_MaybeGC(context->spiderMonkey.context);

	mutex_unlock(&context->mutex);

	return atrue;
}

aboolean script_dumpHeap(ScriptEngine *scriptEngine, void *stream)
{
	ScriptEngineContext *context = NULL;

	if((scriptEngine == NULL) ||
			(scriptEngine->status != SCRIPT_ENGINE_STATUS_RUNNING)) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	if((context = scriptCommon_getContext(scriptEngine)) == NULL) {
		return afalse;
	}

	mutex_lock(&context->mutex);

#if defined(DEBUG) // the flag used in SpiderMonkey to indicate a debug build
		JS_DumpHeap(context->spiderMonkey.context, stream,
				NULL, 0, NULL, 128, NULL);
	}
#endif // DEBUG

	mutex_unlock(&context->mutex);

	return atrue;
}

