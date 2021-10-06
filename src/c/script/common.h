/*
 * common.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The SpiderMonkey JavaScript engine common header-file for Asgard.
 *
 * Written by Josh English.
 */

#if !defined(_SCRIPT_COMMON_H)

#define _SCRIPT_COMMON_H

#if !defined(_SCRIPT_H) && !defined(_SCRIPT_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _SCRIPT_H || _SCRIPT_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// preprocessor directives

#if defined(WIN32)
#	define XP_WIN
#elif defined(__APPLE__)
#	define XP_OS2
#else // !WIN32 && !__APPLE__
#	define XP_UNIX
#endif // !WIN32

#include "jsapi.h"


// define common public constants

typedef enum _SpiderMonkeyStatus {
	SPIDER_MONKEY_STATUS_INIT = 1,
	SPIDER_MONKEY_STATUS_RUNNING,
	SPIDER_MONKEY_STATUS_SHUTDOWN,
	SPIDER_MONKEY_STATUS_ERROR = -1,
} SpiderMonkeyStatus;

typedef enum _ScriptEngineStatus {
	SCRIPT_ENGINE_STATUS_INIT = 1,
	SCRIPT_ENGINE_STATUS_RUNNING,
	SCRIPT_ENGINE_STATUS_SHUTDOWN,
	SCRIPT_ENGINE_STATUS_ERROR = -1,
} ScriptEngineStatus;

#define SPIDER_MONKEY_DEFAULT_RUNTIME_MEMORY				16777216

#define SPIDER_MONKEY_STACK_CHUNK_LENGTH					8192


// define common public data types

typedef struct _SpiderMonkey {
	JSObject *global;
	JSRuntime *runtime;
	JSContext *context;
} SpiderMonkey;

typedef struct _ScriptEngineContext {
	SpiderMonkeyStatus status;
	int threadId;
	SpiderMonkey spiderMonkey;
	Log *log;
	Mutex mutex;
} ScriptEngineContext;

typedef struct _ScriptEngine {
	ScriptEngineStatus status;
	aboolean isExternalLog;
	int runtimeMemory;
	int threadListLength;
	int *threadIdList;
	ScriptEngineContext *threadContexts;
	void *native;
	Log *log;
	Mutex mutex;
} ScriptEngine;


// declare common public functions

void scriptCommon_init(ScriptEngine *scriptEngine, int runtimeMemory, Log *log);

void scriptCommon_free(ScriptEngine *scriptEngine);

int scriptCommon_getThreadId();

int scriptCommon_getContextId(ScriptEngine *scriptEngine, int threadId);

ScriptEngineContext *scriptCommon_getContext(ScriptEngine *scriptEngine);


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _SCRIPT_COMMON_H

