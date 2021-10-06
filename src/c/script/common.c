/*
 * common.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The SpiderMonkey JavaScript engine common function library for Asgard.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#define _SCRIPT_COMPONENT
#include "script/common.h"


// define common private data types

static JSClass SpiderMonkeyGlobalClass = {
	"global",
	JSCLASS_GLOBAL_FLAGS,
	JS_PropertyStub,
	JS_PropertyStub,
	JS_PropertyStub,
	JS_PropertyStub,
	JS_EnumerateStub,
	JS_ResolveStub,
	JS_ConvertStub,
	JS_FinalizeStub,
	JSCLASS_NO_OPTIONAL_MEMBERS
};


// define common private functions

// spidermonkey functions

static void spidermonkeyReportError(JSContext *cx, const char *message,
		JSErrorReport *report)
{
	char *filename = NULL;

	if(report->filename) {
		filename = (char *)report->filename;
	}
	else {
		filename = "<no filename>";
	}

	fprintf(stderr, "[%s():%i] error - [%s->%u]: '%s'\n",
			__FUNCTION__,
			__LINE__,
			filename,
			(unsigned int)report->lineno,
			message);
}

static int initSpiderMonkey(ScriptEngine *scriptEngine, int threadId,
		SpiderMonkey *spiderMonkey)
{
	log_logf(scriptEngine->log, LOG_LEVEL_INFO,
			"initializing spider monkey for thread %i context",
			threadId);

	// construct the runtime

	if((spiderMonkey->runtime = JS_NewRuntime(
					(uint32)scriptEngine->runtimeMemory)) == NULL) {
		log_logf(scriptEngine->log, LOG_LEVEL_ERROR,
				"[%s():%i] error - failed to execute :: %s()",
				__FUNCTION__, __LINE__, "JS_NewRuntime");
		return -1;
	}

	// construct the context

	if((spiderMonkey->context = JS_NewContext(
					spiderMonkey->runtime,
					SPIDER_MONKEY_STACK_CHUNK_LENGTH)) == NULL) {
		log_logf(scriptEngine->log, LOG_LEVEL_ERROR,
				"[%s():%i] error - failed to execute :: %s()",
				__FUNCTION__, __LINE__, "JS_NewContext");
		return -1;
	}

	// set context options

	JS_SetOptions(spiderMonkey->context, JSOPTION_VAROBJFIX);
	JS_SetVersion(spiderMonkey->context, JSVERSION_LATEST);
	JS_SetErrorReporter(spiderMonkey->context, spidermonkeyReportError);

	// create a global object with standard classes

	if((spiderMonkey->global = JS_NewObject(spiderMonkey->context,
					&SpiderMonkeyGlobalClass,
					NULL,
					NULL)) == NULL) {
		log_logf(scriptEngine->log, LOG_LEVEL_ERROR,
				"[%s():%i] error - failed to execute :: %s()",
				__FUNCTION__, __LINE__, "JS_NewObject");
		return -1;
	}

	if(JS_InitStandardClasses(spiderMonkey->context,
				spiderMonkey->global) == JS_FALSE) {
		log_logf(scriptEngine->log, LOG_LEVEL_ERROR,
				"[%s():%i] error - failed to execute :: %s()",
				__FUNCTION__, __LINE__, "JS_InitStandardClasses");
		return -1;
	}

	log_logf(scriptEngine->log, LOG_LEVEL_INFO,
			"spider monkey for thread %i context running",
			threadId);

	return 0;
}

static int freeSpiderMonkey(ScriptEngine *scriptEngine, int threadId,
		SpiderMonkey *spiderMonkey)
{
	int result = 0;

	log_logf(scriptEngine->log, LOG_LEVEL_INFO,
			"freeing spider monkey for thread %i context",
			threadId);

	if(spiderMonkey->context != NULL) {
		JS_DestroyContext(spiderMonkey->context);
	}
	else {
		result = -1;
	}

	if(spiderMonkey->runtime != NULL) {
		JS_DestroyRuntime(spiderMonkey->runtime);
	}
	else {
		result = -1;
	}

	log_logf(scriptEngine->log, LOG_LEVEL_INFO,
			"spider monkey for thread %i context freed",
			threadId);

	return result;
}

// context functions

static int getThreadId()
{
	int threadId = 0;

	if(thread_self(&threadId) < 0) {
		threadId = -1;
	}

	return threadId;
}

static int getContextId(ScriptEngine *scriptEngine, int threadId)
{
	int ii = 0;
	int contextId = -1;

	if(scriptEngine->threadIdList != NULL) {
		for(ii = 0; ii < scriptEngine->threadListLength; ii++) {
			if(scriptEngine->threadIdList[ii] == threadId) {
				contextId = ii;
				break;
			}
		}
	}

	return contextId;
}

static ScriptEngineContext *newContext(ScriptEngine *scriptEngine)
{
	int position = 0;
	int threadId = 0;
	int contextId = 0;

	ScriptEngineContext *result = NULL;

	threadId = getThreadId();

	log_logf(scriptEngine->log, LOG_LEVEL_INFO,
			"initializing scripting engine thread %i context",
			threadId);

	if((contextId = getContextId(scriptEngine, threadId)) >= 0) {
		return NULL;
	}

	if(scriptEngine->threadIdList == NULL) {
		position = 0;
		scriptEngine->threadListLength = 1;
		scriptEngine->threadIdList = (int *)malloc(sizeof(int) *
				scriptEngine->threadListLength);
		scriptEngine->threadContexts = (ScriptEngineContext *)malloc(
				sizeof(ScriptEngineContext) * scriptEngine->threadListLength);
	}
	else {
		position = scriptEngine->threadListLength;
		scriptEngine->threadListLength += 1;
		scriptEngine->threadIdList = (int *)realloc(scriptEngine->threadIdList,
				(sizeof(int) * scriptEngine->threadListLength));
		scriptEngine->threadContexts = (ScriptEngineContext *)realloc(
				scriptEngine->threadContexts,
				(sizeof(ScriptEngineContext) * scriptEngine->threadListLength));
	}

	scriptEngine->threadIdList[position] = threadId;

	result = &(scriptEngine->threadContexts[position]);

	result->status = SCRIPT_ENGINE_STATUS_INIT;
	result->threadId = threadId;
	result->spiderMonkey.global = NULL;
	result->spiderMonkey.runtime = NULL;
	result->spiderMonkey.context = NULL;
	result->log = scriptEngine->log;

	mutex_init(&result->mutex);

	log_logf(scriptEngine->log, LOG_LEVEL_INFO,
			"scripting engine thread %i context initialized",
			threadId);

	return result;
}

static void freeContext(ScriptEngine *scriptEngine,
		ScriptEngineContext *context)
{
	mutex_lock(&context->mutex);

	if(context->status == SPIDER_MONKEY_STATUS_RUNNING) {
		if(freeSpiderMonkey(scriptEngine,
				context->threadId,
				&context->spiderMonkey) < 0) {
			context->status = SPIDER_MONKEY_STATUS_ERROR;
		}
		else {
			context->status = SPIDER_MONKEY_STATUS_SHUTDOWN;
		}
	}

	context->log = NULL;

	mutex_unlock(&context->mutex);
	mutex_free(&context->mutex);
}


// define common public functions

void scriptCommon_init(ScriptEngine *scriptEngine, int runtimeMemory, Log *log)
{
	ScriptEngineContext *context = NULL;

	if(scriptEngine == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	memset(scriptEngine, 0, (int)(sizeof(ScriptEngine)));

	scriptEngine->status = SPIDER_MONKEY_STATUS_INIT;

	if(runtimeMemory > 0) {
		scriptEngine->runtimeMemory = runtimeMemory;
	}
	else {
		scriptEngine->runtimeMemory = SPIDER_MONKEY_DEFAULT_RUNTIME_MEMORY;
	}

	scriptEngine->threadListLength = 0;
	scriptEngine->threadIdList = NULL;
	scriptEngine->threadContexts = NULL;
	scriptEngine->native = NULL;

	if(log != NULL) {
		scriptEngine->isExternalLog = atrue;
		scriptEngine->log = log;
	}
	else {
		scriptEngine->isExternalLog = afalse;
		scriptEngine->log = (Log *)malloc(sizeof(Log));
		log_init(scriptEngine->log, LOG_OUTPUT_STDOUT, NULL, LOG_OUTPUT_STDOUT);
	}

	mutex_init(&scriptEngine->mutex);

	log_logf(scriptEngine->log, LOG_LEVEL_INFO,
			"initializing scripting engine");

	mutex_lock(&scriptEngine->mutex);

	if((context = newContext(scriptEngine)) == NULL) {
		scriptEngine->status = SCRIPT_ENGINE_STATUS_ERROR;
		mutex_unlock(&scriptEngine->mutex);
		return;
	}

	mutex_lock(&context->mutex);

	if(initSpiderMonkey(scriptEngine, context->threadId,
				&context->spiderMonkey) < 0) {
		context->status = SPIDER_MONKEY_STATUS_ERROR;

		mutex_unlock(&context->mutex);

		scriptEngine->status = SCRIPT_ENGINE_STATUS_ERROR;

		mutex_unlock(&scriptEngine->mutex);
		return;
	}

	context->status = SPIDER_MONKEY_STATUS_RUNNING;

	mutex_unlock(&context->mutex);

	scriptEngine->status = SCRIPT_ENGINE_STATUS_RUNNING;

	mutex_unlock(&scriptEngine->mutex);

	log_logf(scriptEngine->log, LOG_LEVEL_INFO,
			"scripting engine running");
}

void scriptCommon_free(ScriptEngine *scriptEngine)
{
	int ii = 0;

	if(scriptEngine == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	mutex_lock(&scriptEngine->mutex);

	if(scriptEngine->status == SCRIPT_ENGINE_STATUS_RUNNING) {
		if(scriptEngine->threadContexts != NULL) {
			for(ii = 0; ii < scriptEngine->threadListLength; ii++) {
				freeContext(scriptEngine, &(scriptEngine->threadContexts[ii]));
			}
			free(scriptEngine->threadContexts);
		}

		if(scriptEngine->threadIdList != NULL) {
			free(scriptEngine->threadIdList);
		}

		scriptEngine->status = SCRIPT_ENGINE_STATUS_SHUTDOWN;
	}

	if(!scriptEngine->isExternalLog) {
		log_free(scriptEngine->log);
		free(scriptEngine->log);
	}

	mutex_unlock(&scriptEngine->mutex);
	mutex_free(&scriptEngine->mutex);

	memset(scriptEngine, 0, (int)(sizeof(ScriptEngine)));

	// cleanup the SpiderMonkey engine

	JS_ShutDown();
}

int scriptCommon_getThreadId()
{
	return getThreadId();
}

int scriptCommon_getContextId(ScriptEngine *scriptEngine, int threadId)
{
	int result = -1;

	if((scriptEngine == NULL) ||
			(scriptEngine->status != SCRIPT_ENGINE_STATUS_RUNNING)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&scriptEngine->mutex);

	result = getContextId(scriptEngine, threadId);

	mutex_unlock(&scriptEngine->mutex);

	return result;
}

ScriptEngineContext *scriptCommon_getContext(ScriptEngine *scriptEngine)
{
	int contextId = 0;

	ScriptEngineContext *result = NULL;

	if((scriptEngine == NULL) ||
			(scriptEngine->status != SCRIPT_ENGINE_STATUS_RUNNING)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	mutex_lock(&scriptEngine->mutex);

	if((contextId = getContextId(scriptEngine, getThreadId())) < 0) {
		if((result = newContext(scriptEngine)) == NULL) {
			scriptEngine->status = SCRIPT_ENGINE_STATUS_ERROR;
			mutex_unlock(&scriptEngine->mutex);
			return NULL;
		}

		mutex_lock(&result->mutex);

		if(initSpiderMonkey(scriptEngine, result->threadId,
					&result->spiderMonkey) < 0) {
			result->status = SPIDER_MONKEY_STATUS_ERROR;

			mutex_unlock(&result->mutex);

			scriptEngine->status = SCRIPT_ENGINE_STATUS_ERROR;

			mutex_unlock(&scriptEngine->mutex);
			return NULL;
		}

		result->status = SPIDER_MONKEY_STATUS_RUNNING;

		mutex_unlock(&result->mutex);
	}
	else {
		result = &(scriptEngine->threadContexts[contextId]);
	}

	if(result->status != SPIDER_MONKEY_STATUS_RUNNING) {
		mutex_unlock(&scriptEngine->mutex);
		return NULL;
	}

	mutex_unlock(&scriptEngine->mutex);

	return result;
}

