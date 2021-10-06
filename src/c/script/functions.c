/*
 * functions.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The SpiderMonkey JavaScript engine for Asgard, native function wrapper
 * library.
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
#include "script/types.h"
#include "script/functions.h"


// define functions private constants

static JSFunctionSpec FUNCTION_LIST[] = {
	/*
	 * native core/common functions
	 */

	{	"asgard_getVersion",		native_asgard_getVersion,	0, 0, 0,	},
	{	"asgard_getDate",			native_asgard_getDate,		0, 0, 0,	},

	/*
	 * native core/os functions
	 */

	{	"time_getTimeSeconds",		native_time_getTimeSeconds,	0, 0, 0,	},
	{	"time_getTimeMus",			native_time_getTimeMus,		0, 0, 0,	},
	{	"time_getElapsedSeconds",
			native_time_getElapsedSeconds,						0, 0, 0,	},
	{	"time_getElapsedMus",		native_time_getElapsedMus,	0, 0, 0,	},
	{	"time_getElapsedMusInMillis",
			native_time_getElapsedMusInMillis,					0, 0, 0,	},
	{	"time_getElapsedMusInSeconds",
			native_time_getElapsedMusInSeconds,					0, 0, 0,	},
	{	"time_getTimestamp",		native_time_getTimestamp,	0, 0, 0,	},
	{	"time_sleep",				native_time_sleep,			0, 0, 0,	},
	{	"time_usleep",				native_time_usleep,			0, 0, 0,	},
	{	"time_nanosleep",			native_time_nanosleep,		0, 0, 0,	},

	/*
	 * native core/system functions
	 */

	// mutex

	{	"mutex_new",				native_mutex_new,			0, 0, 0,	},
	{	"mutex_lock",				native_mutex_lock,			0, 0, 0,	},
	{	"mutex_unlock",				native_mutex_unlock,		0, 0, 0,	},
	{	"mutex_free",				native_mutex_free,			0, 0, 0,	},

	// signal

	{	"signal_registerAction",
			native_signal_registerAction,						0, 0, 0,	},
	{	"signal_executeSignal",
			native_signal_executeSignal,						0, 0, 0,	},
	{	"signal_toString",			native_signal_toString,		0, 0, 0,	},

	// system

	{	"system_fileSetNative",
			native_system_fileSetNative,						0, 0, 0,	},
	{	"system_fileExists",
			native_system_fileExists,							0, 0, 0,	},
	{	"system_fileLength",
			native_system_fileLength,							0, 0, 0,	},
	{	"system_fileModifiedTime",
			native_system_fileModifiedTime,						0, 0, 0,	},
	{	"system_fileDelete",
			native_system_fileDelete,							0, 0, 0,	},
	{	"system_fileExecute",
			native_system_fileExecute,							0, 0, 0,	},
	{	"system_dirExists",
			native_system_dirExists,							0, 0, 0,	},
	{	"system_dirChangeTo",
			native_system_dirChangeTo,							0, 0, 0,	},
	{	"system_dirCreate",
			native_system_dirCreate,							0, 0, 0,	},
	{	"system_dirDelete",
			native_system_dirDelete,							0, 0, 0,	},
	{	"system_pickRandomSeed",
			native_system_pickRandomSeed,						0, 0, 0,	},
	{	"system_setRandomSeed",
			native_system_setRandomSeed,						0, 0, 0,	},

	// socket

	{	"socket_new",
			native_socket_new,									0, 0, 0,	},
	{	"socket_socketStateToString",
			native_socket_socketStateToString,					0, 0, 0,	},
	{	"socket_socketModeToString",
			native_socket_socketModeToString,					0, 0, 0,	},
	{	"socket_socketProtocolToString",
			native_socket_socketProtocolToString,				0, 0, 0,	},
	{	"socket_getBytesSent",
			native_socket_getBytesSent,							0, 0, 0,	},
	{	"socket_getBytesReceived",
			native_socket_getBytesReceived,						0, 0, 0,	},
	{	"socket_getReadsAttempted",
			native_socket_getReadsAttempted,					0, 0, 0,	},
	{	"socket_getReadsCompleted",
			native_socket_getReadsCompleted,					0, 0, 0,	},
	{	"socket_getWritesAttempted",
			native_socket_getWritesAttempted,					0, 0, 0,	},
	{	"socket_getWritesCompleted",
			native_socket_getWritesCompleted,					0, 0, 0,	},
	{	"socket_getTotalReadTime",
			native_socket_getTotalReadTime,						0, 0, 0,	},
	{	"socket_getTotalWriteTime",
			native_socket_getTotalWriteTime,					0, 0, 0,	},
	{	"socket_getSocketDescriptor",
			native_socket_getSocketDescriptor,					0, 0, 0,	},
	{	"socket_open",
			native_socket_open,									0, 0, 0,	},
	{	"socket_close",
			native_socket_close,								0, 0, 0,	},
	{	"socket_closeChildSocket",
			native_socket_closeChildSocket,						0, 0, 0,	},
	{	"socket_requestShutdown",
			native_socket_requestShutdown,						0, 0, 0,	},
	{	"socket_listen",
			native_socket_listen,								0, 0, 0,	},
	{	"socket_accept",
			native_socket_accept,								0, 0, 0,	},
	{	"socket_getPeerName",
			native_socket_getPeerName,							0, 0, 0,	},
	{	"socket_send",
			native_socket_send,									0, 0, 0,	},
	{	"socket_receive",
			native_socket_receive,								0, 0, 0,	},

	// thread

	{	"thread_new",				native_thread_new,			0, 0, 0,	},
	{	"thread_create",			native_thread_create,		0, 0, 0,	},
	{	"thread_join",				native_thread_join,			0, 0, 0,	},
	{	"thread_self",				native_thread_self,			0, 0, 0,	},

	/*
	 * language native interface functions
	 */

	{	"native_getOsType",			native_getOsType,			0, 0, 0,	},
	{	"native_getConstant",		native_getConstant,			0, 0, 0,	},
	{	"native_getVariable",		native_getVariable,			0, 0, 0,	},
	{	"native_setVariable",		native_setVariable,			0, 0, 0,	},
	{	"remit",					native_getVariable,			0, 0, 0,	},
	{	"emit",						native_setVariable,			0, 0, 0,	},

	/*
	 * language extension functions
	 */

	{	"include",					native_include,				0, 0, 0,	},
	{	"print",					native_print,				0, 0, 0,	},
	{	"rand",						native_rand,				0, 0, 0,	},

	/*
	 * end of function definitions
	 */

	{	NULL,						NULL,						0, 0, 0,	}
};


// define functions private global variables

ScriptEngine *nativeEngine = NULL;


// define functions private macros

#define CHECK_ENGINE_ARGS \
	if((context == NULL) || (object == NULL) || (argc < 0) || \
			(rval == NULL)) { \
		DISPLAY_INVALID_ARGS; \
		return JS_FALSE; \
	}


// define external functions

extern aboolean script_execFunction(ScriptEngine *scriptEngine,
		char *functionName, uintN argc, jsval *argv, char **functionResult);


// declare functions private functions

static void nativeSignalHandler(int signalType);

static aboolean execJavaScriptFunctionOnContext(ScriptEngineContext *context,
		char *functionName, uintN argc, jsval *argv, char **functionResult);

static aboolean execJavaScriptFunction(char *functionName, uintN argc,
		jsval *argv, char **functionResult);

static void *nativeWorkerThread(void *argument);


// define functions private functions

static void nativeSignalHandler(int signalType)
{
	int ii = 0;
	char *functionResult = NULL;

	jsval argv[1];

	ScriptNative *scriptNative = NULL;

	if((scriptNative = (ScriptNative *)nativeEngine->native) == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	argv[0] = scriptVar_integerToJsval(signalType);

	mutex_lock(&scriptNative->mutex);

	if(scriptNative->signalFunctions != NULL) {
		for(ii = 0; ii < scriptNative->signalLength; ii++) {
			if(scriptNative->signalFunctions[ii].signalType == signalType) {
				execJavaScriptFunction(
						scriptNative->signalFunctions[ii].functionName,
						1,
						argv,
						&functionResult);
				log_logf(nativeEngine->log, LOG_LEVEL_INFO,
						"signal #%i executed %s(), result '%s'",
						signalType,
						scriptNative->signalFunctions[ii].functionName,
						functionResult);
			}
		}
	}

	mutex_unlock(&scriptNative->mutex);
}

static void *nativeWorkerThread(void *argument)
{
	int nativeThreadId = 0;
	char *functionResult = NULL;

	ScriptNative *native = NULL;
	ScriptNativeThread *thread = NULL;

	log_logf(nativeEngine->log, LOG_LEVEL_INFO,
			"thread %s() started with 0x%lx",
			__FUNCTION__, (aptrcast)argument);

	nativeThreadId = *((int *)argument);

	log_logf(nativeEngine->log, LOG_LEVEL_DEBUG,
			"thread %s() is native thread #%i",
			__FUNCTION__, nativeThreadId);

	native = (ScriptNative *)nativeEngine->native;

	if((thread = scriptTypes_threadGet(native, nativeThreadId)) == NULL) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"thread %s() failed to obtain native thread #%i",
				__FUNCTION__, nativeThreadId);
		return NULL;
	}

	mutex_lock(&native->mutex);
	thread->state = NATIVE_THREAD_STATE_RUNNING;
	mutex_unlock(&native->mutex);

	if(!execJavaScriptFunctionOnContext(
				thread->context,
				thread->functionName,
				0,
				NULL,
				&functionResult)) {
		mutex_lock(&native->mutex);
		thread->state = NATIVE_THREAD_STATE_ERROR;
		mutex_unlock(&native->mutex);
	}
	else {
		mutex_lock(&native->mutex);
		thread->state = NATIVE_THREAD_STATE_STOPPED;
		mutex_unlock(&native->mutex);
	}

	log_logf(nativeEngine->log, LOG_LEVEL_INFO,
			"thread %s() native #%i shutdown with '%s'",
			__FUNCTION__, nativeThreadId, functionResult);

	return NULL;
}

static aboolean execJavaScriptFunctionOnContext(ScriptEngineContext *context,
		char *functionName, uintN argc, jsval *argv, char **functionResult)
{
	jsval returnValue;

	if((context == NULL) ||
			(functionName == NULL)) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	if(JS_CallFunctionName(context->spiderMonkey.context,
				context->spiderMonkey.global,
				functionName,
				argc,
				argv,
				&returnValue) == JS_FALSE) {
		log_logf(context->log, LOG_LEVEL_ERROR,
				"[%s->%s()] failed to exec function '%s' from %s()",
				__FILE__, __FUNCTION__, functionName, "JS_CallFunctionName");
		return afalse;
	}

	if(functionResult != NULL) {
		*functionResult = scriptVar_jsvalToString(
				context->spiderMonkey.context,
				&returnValue);
	}

	return atrue;
}

static aboolean execJavaScriptFunction(char *functionName, uintN argc,
		jsval *argv, char **functionResult)
{
	aboolean result = afalse;

	ScriptEngineContext *context = NULL;

	if((nativeEngine == NULL) ||
			(nativeEngine->status != SCRIPT_ENGINE_STATUS_RUNNING) ||
			(functionName == NULL)) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	if((context = scriptCommon_getContext(nativeEngine)) == NULL) {
		return afalse;
	}

	result = execJavaScriptFunctionOnContext(context, functionName, argc,
			argv, functionResult);

	return result;
}


// define functions public functions

// native core/common functions

JSBool native_asgard_getVersion(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval)
{
	SCRIPT_DISPLAY_NATIVE_FUNCTION_CALL;

	CHECK_ENGINE_ARGS;

	*rval = scriptVar_stringToJsval(context, ASGARD_VERSION);

	return JS_TRUE;
}

JSBool native_asgard_getDate(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval)
{
	SCRIPT_DISPLAY_NATIVE_FUNCTION_CALL;

	CHECK_ENGINE_ARGS;

	*rval = scriptVar_stringToJsval(context, ASGARD_DATE);

	return JS_TRUE;
}

// native core/os functions

JSBool native_time_getTimeSeconds(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval)
{
	SCRIPT_DISPLAY_NATIVE_FUNCTION_CALL;

	CHECK_ENGINE_ARGS;

	*rval = scriptVar_integerToJsval(time_getTimeSeconds());

	return JS_TRUE;
}

JSBool native_time_getTimeMus(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval)
{
	SCRIPT_DISPLAY_NATIVE_FUNCTION_CALL;

	CHECK_ENGINE_ARGS;

	*rval = scriptVar_doubleToJsval(context, time_getTimeMus());

	return JS_TRUE;
}

JSBool native_time_getElapsedSeconds(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval)
{
	int seconds = 0;

	SCRIPT_DISPLAY_NATIVE_FUNCTION_CALL;

	CHECK_ENGINE_ARGS;

	if((argc < 1) || (argv == NULL)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] invalid or missing argument(s)",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}

	seconds = scriptVar_getIntegerArg(0, context, argc, argv);

	*rval = scriptVar_integerToJsval(time_getElapsedSeconds(seconds));

	return JS_TRUE;
}

JSBool native_time_getElapsedMus(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval)
{
	double microseconds = 0.0;

	SCRIPT_DISPLAY_NATIVE_FUNCTION_CALL;

	CHECK_ENGINE_ARGS;

	if((argc < 1) || (argv == NULL)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] invalid or missing argument(s)",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}

	microseconds = scriptVar_getDoubleArg(0, context, argc, argv);

	*rval = scriptVar_doubleToJsval(context, time_getElapsedMus(microseconds));

	return JS_TRUE;
}

JSBool native_time_getElapsedMusInMillis(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval)
{
	double microseconds = 0.0;

	SCRIPT_DISPLAY_NATIVE_FUNCTION_CALL;

	CHECK_ENGINE_ARGS;

	if((argc < 1) || (argv == NULL)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] invalid or missing argument(s)",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}

	microseconds = scriptVar_getDoubleArg(0, context, argc, argv);

	*rval = scriptVar_doubleToJsval(context,
			time_getElapsedMusInMilliseconds(microseconds));

	return JS_TRUE;
}

JSBool native_time_getElapsedMusInSeconds(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval)
{
	double microseconds = 0.0;

	SCRIPT_DISPLAY_NATIVE_FUNCTION_CALL;

	CHECK_ENGINE_ARGS;

	if((argc < 1) || (argv == NULL)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] invalid or missing argument(s)",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}

	microseconds = scriptVar_getDoubleArg(0, context, argc, argv);

	*rval = scriptVar_doubleToJsval(context,
			time_getElapsedMusInSeconds(microseconds));

	return JS_TRUE;
}

JSBool native_time_getTimestamp(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval)
{
	char timestamp[32];

	SCRIPT_DISPLAY_NATIVE_FUNCTION_CALL;

	CHECK_ENGINE_ARGS;

	time_getTimestamp(timestamp);

	*rval = scriptVar_stringToJsval(context, timestamp);

	return JS_TRUE;
}

JSBool native_time_sleep(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval)
{
	int seconds = 0;

	SCRIPT_DISPLAY_NATIVE_FUNCTION_CALL;

	CHECK_ENGINE_ARGS;

	if((argc < 1) || (argv == NULL)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] invalid or missing argument(s)",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}

	seconds = scriptVar_getIntegerArg(0, context, argc, argv);

	if(seconds > 0) {
		time_sleep(seconds);
	}

	return JS_TRUE;
}

JSBool native_time_usleep(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval)
{
	int microseconds = 0;

	SCRIPT_DISPLAY_NATIVE_FUNCTION_CALL;

	CHECK_ENGINE_ARGS;

	if((argc < 1) || (argv == NULL)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] invalid or missing argument(s)",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}

	microseconds = scriptVar_getIntegerArg(0, context, argc, argv);

	if(microseconds > 0) {
		time_usleep(microseconds);
	}

	return JS_TRUE;
}

JSBool native_time_nanosleep(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval)
{
	int nanoseconds = 0;

	SCRIPT_DISPLAY_NATIVE_FUNCTION_CALL;

	CHECK_ENGINE_ARGS;

	if((argc < 1) || (argv == NULL)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] invalid or missing argument(s)",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}

	nanoseconds = scriptVar_getIntegerArg(0, context, argc, argv);

	if(nanoseconds > 0) {
		time_nanosleep(nanoseconds);
	}

	return JS_TRUE;
}

// native core/system functions

// mutex functions

JSBool native_mutex_new(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval)
{
	int id = 0;
	double uid = 0.0;

	SCRIPT_DISPLAY_NATIVE_FUNCTION_CALL;

	CHECK_ENGINE_ARGS;

	if((id = scriptTypes_mutexNew(nativeEngine->native)) < 0) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] failed to execute :: "
				"scriptTypes_newMutex()",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}

	uid = scriptTypes_encodeNativeId(SCRIPT_NATIVE_TYPE_MUTEX, id);

	*rval = scriptVar_doubleToJsval(context, uid);

	return JS_TRUE;
}

JSBool native_mutex_lock(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval)
{
	int id = 0;
	double uid = 0.0;

	ScriptNativeType type = SCRIPT_NATIVE_TYPE_UNKNOWN;

	SCRIPT_DISPLAY_NATIVE_FUNCTION_CALL;

	CHECK_ENGINE_ARGS;

	if((argc < 1) || (argv == NULL)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] invalid or missing argument(s)",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}

	uid = scriptVar_getDoubleArg(0, context, argc, argv);

	if(!scriptTypes_decodeNativeId(uid, &type, &id)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] native types id #%0.0f is invalid",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}
	else if(type != SCRIPT_NATIVE_TYPE_MUTEX) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] native types id #%0.0f is not a mutex",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}
	else if(!scriptTypes_mutexLock(nativeEngine->native, id)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] failed to execute :: "
				"scriptTypes_mutexLock()",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}

	return JS_TRUE;
}

JSBool native_mutex_unlock(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval)
{
	int id = 0;
	double uid = 0.0;

	ScriptNativeType type = SCRIPT_NATIVE_TYPE_UNKNOWN;

	SCRIPT_DISPLAY_NATIVE_FUNCTION_CALL;

	CHECK_ENGINE_ARGS;

	if((argc < 1) || (argv == NULL)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] invalid or missing argument(s)",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}

	uid = scriptVar_getDoubleArg(0, context, argc, argv);

	if(!scriptTypes_decodeNativeId(uid, &type, &id)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] native types id #%0.0f is invalid",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}
	else if(type != SCRIPT_NATIVE_TYPE_MUTEX) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] native types id #%0.0f is not a mutex",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}
	else if(!scriptTypes_mutexUnlock(nativeEngine->native, id)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] failed to execute :: "
				"scriptTypes_mutexUnlock()",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}

	return JS_TRUE;
}

JSBool native_mutex_free(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval)
{
	int id = 0;
	double uid = 0.0;

	ScriptNativeType type = SCRIPT_NATIVE_TYPE_UNKNOWN;

	SCRIPT_DISPLAY_NATIVE_FUNCTION_CALL;

	CHECK_ENGINE_ARGS;

	if((argc < 1) || (argv == NULL)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] invalid or missing argument(s)",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}

	uid = scriptVar_getDoubleArg(0, context, argc, argv);

	if(!scriptTypes_decodeNativeId(uid, &type, &id)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] native types id #%0.0f is invalid",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}
	else if(type != SCRIPT_NATIVE_TYPE_MUTEX) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] native types id #%0.0f is not a mutex",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}
	else if(!scriptTypes_mutexFree(nativeEngine->native, id)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] failed to execute :: "
				"scriptTypes_mutexUnlock()",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}

	return JS_TRUE;
}

// signal functions

JSBool native_signal_registerAction(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval)
{
	int id = 0;
	int signalType = 0;
	double uid = 0.0;
	char *functionName = NULL;

	SCRIPT_DISPLAY_NATIVE_FUNCTION_CALL;

	CHECK_ENGINE_ARGS;

	if((argc < 2) || (argv == NULL)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] invalid or missing argument(s)",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}

	signalType = scriptVar_getIntegerArg(0, context, argc, argv);

	if((functionName = scriptVar_getStringArg(1, context, argc,
					argv)) == NULL) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] failed to obtain 'functionName' argument",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(JS_FALSE);
		return JS_TRUE;
	}

	if((id = scryptTypes_signalRegisterAction(nativeEngine->native,
					signalType, functionName)) < 0) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] failed to execute :: "
				"scriptTypes_newMutex()",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}

	if(signal_registerAction(signalType, nativeSignalHandler) < 0) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] failed to execute :: "
				"signal_registerAction()",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}

	uid = scriptTypes_encodeNativeId(SCRIPT_NATIVE_TYPE_MUTEX, id);

	*rval = scriptVar_doubleToJsval(context, uid);

	return JS_TRUE;
}

JSBool native_signal_executeSignal(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval)
{
	int signalType = 0;

	SCRIPT_DISPLAY_NATIVE_FUNCTION_CALL;

	CHECK_ENGINE_ARGS;

	if((argc < 1) || (argv == NULL)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] invalid or missing argument(s)",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}

	signalType = scriptVar_getIntegerArg(0, context, argc, argv);

	if(signal_executeSignal(signalType)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] failed to execute :: "
				"signal_executeSignal()",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}

	*rval = scriptVar_booleanToJsval(JS_TRUE);

	return JS_TRUE;
}

JSBool native_signal_toString(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval)
{
	int signalType = 0;
	char *result = NULL;

	SCRIPT_DISPLAY_NATIVE_FUNCTION_CALL;

	CHECK_ENGINE_ARGS;

	if((argc < 1) || (argv == NULL)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] invalid or missing argument(s)",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}

	signalType = scriptVar_getIntegerArg(0, context, argc, argv);

	switch(signalType) {
		case SIGNAL_INTERRUPT:
			result = "Interrupt";
			break;

		case SIGNAL_TERMINATE:
			result = "Terminate";
			break;

		case SIGNAL_SEGFAULT:
			result = "Segmentation Fault";
			break;

		case SIGNAL_USERONE:
			result = "User One";
			break;

		case SIGNAL_USERTWO:
			result = "User Two";
			break;

		case SIGNAL_QUIT:
			result = "Quit";
			break;

		case SIGNAL_ABORT:
			result = "Abort";
			break;

		case SIGNAL_FLOATING_POINT_EXCEPTION:
			result = "Floating Point Exception";
			break;

		case SIGNAL_ILLEGAL:
			result = "Illegal";
			break;

		default:
			result = "Unknown";
	}

	*rval = scriptVar_stringToJsval(context, result);

	return JS_TRUE;
}

// system functions

JSBool native_system_fileSetNative(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval)
{
	char *filename = NULL;

	SCRIPT_DISPLAY_NATIVE_FUNCTION_CALL;

	CHECK_ENGINE_ARGS;

	if((argc < 1) || (argv == NULL)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] invalid or missing argument(s)",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}

	if((filename = scriptVar_getStringArg(0, context, argc, argv)) == NULL) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] failed to obtain 'filename' argument",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(JS_FALSE);
		return JS_TRUE;
	}

	if(system_fileSetNative(filename) < 0) {
		*rval = scriptVar_booleanToJsval(JS_FALSE);
	}
	else {
		*rval = scriptVar_stringToJsval(context, filename);
	}

	return JS_TRUE;
}

JSBool native_system_fileExists(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval)
{
	aboolean fileExists = afalse;
	char *filename = NULL;

	SCRIPT_DISPLAY_NATIVE_FUNCTION_CALL;

	CHECK_ENGINE_ARGS;

	if((argc < 1) || (argv == NULL)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] invalid or missing argument(s)",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}

	if((filename = scriptVar_getStringArg(0, context, argc, argv)) == NULL) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] failed to obtain 'filename' argument",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(JS_FALSE);
		return JS_TRUE;
	}

	if(system_fileExists(filename, &fileExists) < 0) {
		*rval = scriptVar_booleanToJsval(JS_FALSE);
	}
	else {
		if(fileExists) {
			*rval = scriptVar_booleanToJsval(JS_TRUE);
		}
		else {
			*rval = scriptVar_booleanToJsval(JS_FALSE);
		}
	}

	return JS_TRUE;
}

JSBool native_system_fileLength(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval)
{
	alint fileLength = 0;
	char *filename = NULL;

	SCRIPT_DISPLAY_NATIVE_FUNCTION_CALL;

	CHECK_ENGINE_ARGS;

	if((argc < 1) || (argv == NULL)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] invalid or missing argument(s)",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}

	if((filename = scriptVar_getStringArg(0, context, argc, argv)) == NULL) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] failed to obtain 'filename' argument",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(JS_FALSE);
		return JS_TRUE;
	}

	if(system_fileLength(filename, &fileLength) < 0) {
		*rval = scriptVar_booleanToJsval(JS_FALSE);
	}
	else {
		*rval = scriptVar_doubleToJsval(context, (double)fileLength);
	}

	return JS_TRUE;
}

JSBool native_system_fileModifiedTime(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval)
{
	double timestamp = 0.0;
	char *filename = NULL;

	SCRIPT_DISPLAY_NATIVE_FUNCTION_CALL;

	CHECK_ENGINE_ARGS;

	if((argc < 1) || (argv == NULL)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] invalid or missing argument(s)",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}

	if((filename = scriptVar_getStringArg(0, context, argc, argv)) == NULL) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] failed to obtain 'filename' argument",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(JS_FALSE);
		return JS_TRUE;
	}

	if(system_fileModifiedTime(filename, &timestamp) < 0) {
		*rval = scriptVar_booleanToJsval(JS_FALSE);
	}
	else {
		*rval = scriptVar_doubleToJsval(context, timestamp);
	}

	return JS_TRUE;
}

JSBool native_system_fileDelete(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval)
{
	char *filename = NULL;

	SCRIPT_DISPLAY_NATIVE_FUNCTION_CALL;

	CHECK_ENGINE_ARGS;

	if((argc < 1) || (argv == NULL)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] invalid or missing argument(s)",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}

	if((filename = scriptVar_getStringArg(0, context, argc, argv)) == NULL) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] failed to obtain 'filename' argument",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(JS_FALSE);
		return JS_TRUE;
	}

	if(system_fileDelete(filename) < 0) {
		*rval = scriptVar_booleanToJsval(JS_FALSE);
	}
	else {
		*rval = scriptVar_booleanToJsval(JS_TRUE);
	}

	return JS_TRUE;
}

JSBool native_system_fileExecute(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval)
{
	char *filename = NULL;
	char *arguments = NULL;

	SCRIPT_DISPLAY_NATIVE_FUNCTION_CALL;

	CHECK_ENGINE_ARGS;

	if((argc < 2) || (argv == NULL)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] invalid or missing argument(s)",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}

	if((filename = scriptVar_getStringArg(0, context, argc, argv)) == NULL) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] failed to obtain 'filename' argument",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(JS_FALSE);
		return JS_TRUE;
	}

	if((arguments = scriptVar_getStringArg(1, context, argc, argv)) == NULL) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] failed to obtain 'arguments' argument",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(JS_FALSE);
		return JS_TRUE;
	}

	if(system_fileExecute(filename, arguments) < 0) {
		*rval = scriptVar_booleanToJsval(JS_FALSE);
	}
	else {
		*rval = scriptVar_booleanToJsval(JS_TRUE);
	}

	return JS_TRUE;
}

JSBool native_system_dirExists(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval)
{
	aboolean dirExists = afalse;
	char *dirname = NULL;

	SCRIPT_DISPLAY_NATIVE_FUNCTION_CALL;

	CHECK_ENGINE_ARGS;

	if((argc < 1) || (argv == NULL)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] invalid or missing argument(s)",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}

	if((dirname = scriptVar_getStringArg(0, context, argc, argv)) == NULL) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] failed to obtain 'dirname' argument",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(JS_FALSE);
		return JS_TRUE;
	}

	if(system_dirExists(dirname, &dirExists) < 0) {
		*rval = scriptVar_booleanToJsval(JS_FALSE);
	}
	else {
		if(dirExists) {
			*rval = scriptVar_booleanToJsval(JS_TRUE);
		}
		else {
			*rval = scriptVar_booleanToJsval(JS_FALSE);
		}
	}

	return JS_TRUE;
}

JSBool native_system_dirChangeTo(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval)
{
	char *dirname = NULL;

	SCRIPT_DISPLAY_NATIVE_FUNCTION_CALL;

	CHECK_ENGINE_ARGS;

	if((argc < 1) || (argv == NULL)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] invalid or missing argument(s)",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}

	if((dirname = scriptVar_getStringArg(0, context, argc, argv)) == NULL) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] failed to obtain 'dirname' argument",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(JS_FALSE);
		return JS_TRUE;
	}

	if(system_dirChangeTo(dirname) < 0) {
		*rval = scriptVar_booleanToJsval(JS_FALSE);
	}
	else {
		*rval = scriptVar_booleanToJsval(JS_TRUE);
	}

	return JS_TRUE;
}

JSBool native_system_dirCreate(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval)
{
	char *dirname = NULL;

	SCRIPT_DISPLAY_NATIVE_FUNCTION_CALL;

	CHECK_ENGINE_ARGS;

	if((argc < 1) || (argv == NULL)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] invalid or missing argument(s)",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}

	if((dirname = scriptVar_getStringArg(0, context, argc, argv)) == NULL) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] failed to obtain 'dirname' argument",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(JS_FALSE);
		return JS_TRUE;
	}

	if(system_dirCreate(dirname) < 0) {
		*rval = scriptVar_booleanToJsval(JS_FALSE);
	}
	else {
		*rval = scriptVar_booleanToJsval(JS_TRUE);
	}

	return JS_TRUE;
}

JSBool native_system_dirDelete(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval)
{
	char *dirname = NULL;

	SCRIPT_DISPLAY_NATIVE_FUNCTION_CALL;

	CHECK_ENGINE_ARGS;

	if((argc < 1) || (argv == NULL)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] invalid or missing argument(s)",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}

	if((dirname = scriptVar_getStringArg(0, context, argc, argv)) == NULL) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] failed to obtain 'dirname' argument",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(JS_FALSE);
		return JS_TRUE;
	}

	if(system_dirDelete(dirname) < 0) {
		*rval = scriptVar_booleanToJsval(JS_FALSE);
	}
	else {
		*rval = scriptVar_booleanToJsval(JS_TRUE);
	}

	return JS_TRUE;
}

JSBool native_system_pickRandomSeed(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval)
{
	SCRIPT_DISPLAY_NATIVE_FUNCTION_CALL;

	CHECK_ENGINE_ARGS;

	system_pickRandomSeed();

	*rval = scriptVar_booleanToJsval(JS_TRUE);

	return JS_TRUE;
}

JSBool native_system_setRandomSeed(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval)
{
	unsigned int seed = (unsigned int)0;

	SCRIPT_DISPLAY_NATIVE_FUNCTION_CALL;

	CHECK_ENGINE_ARGS;

	if((argc < 1) || (argv == NULL)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] invalid or missing argument(s)",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}

	seed = (unsigned int)scriptVar_getIntegerArg(0, context, argc, argv);

	system_setRandomSeed(seed);

	*rval = scriptVar_booleanToJsval(JS_TRUE);

	return JS_TRUE;
}

// socket functions

JSBool native_socket_new(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval)
{
	int id = 0;
	int mode = 0;
	int port = 0;
	int protocol = 0;
	double uid = 0.0;
	char *hostname = NULL;

	Socket *socket = NULL;

	SCRIPT_DISPLAY_NATIVE_FUNCTION_CALL;

	CHECK_ENGINE_ARGS;

	if((argc < 1) || (argv == NULL)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] invalid or missing argument(s)",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}

	mode = scriptVar_getIntegerArg(0, context, argc, argv);
	protocol = scriptVar_getIntegerArg(1, context, argc, argv);

	if((hostname = scriptVar_getStringArg(2, context, argc,
					argv)) == NULL) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] failed to obtain 'hostname' argument",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(JS_FALSE);
		return JS_TRUE;
	}

	port = scriptVar_getIntegerArg(3, context, argc, argv);

	if((id = scriptTypes_socketNew(nativeEngine, mode, protocol, hostname,
					port)) < 0) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] failed to create new native socket",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(JS_FALSE);
	}
	else if((socket = scriptTypes_socketGet(nativeEngine->native,
					id)) == NULL) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] failed to obtain new native socket #%i",
				__FILE__, __FUNCTION__, id);
		*rval = scriptVar_booleanToJsval(JS_FALSE);
	}
	else {
		log_logf(nativeEngine->log, LOG_LEVEL_DEBUG,
				"created native socket context #%i",
				__FILE__, __FUNCTION__, id);

		uid = scriptTypes_encodeNativeId(SCRIPT_NATIVE_TYPE_SOCKET, id);

		*rval = scriptVar_doubleToJsval(context, uid);
	}

	return JS_TRUE;
}

JSBool native_socket_socketStateToString(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval)
{
	int id = 0;
	double uid = 0.0;
	char *string = NULL;

	ScriptNativeType type = SCRIPT_NATIVE_TYPE_UNKNOWN;
	Socket *socket = NULL;

	SCRIPT_DISPLAY_NATIVE_FUNCTION_CALL;

	CHECK_ENGINE_ARGS;

	if((argc < 1) || (argv == NULL)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] invalid or missing argument(s)",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}

	uid = scriptVar_getDoubleArg(0, context, argc, argv);

	if(!scriptTypes_decodeNativeId(uid, &type, &id)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] native types id #%0.0f is invalid",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}
	else if(type != SCRIPT_NATIVE_TYPE_SOCKET) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] native types id #%0.0f is not a socket",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}
	else if((socket = scriptTypes_socketGet(nativeEngine->native,
					id)) == NULL) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] failed to obtain new native socket #%i",
				__FILE__, __FUNCTION__, id);
		*rval = scriptVar_booleanToJsval(JS_FALSE);
	}
	else {
		string = socket_socketStateToString(socket->state);

		log_logf(nativeEngine->log, LOG_LEVEL_INFO,
				"socket #%i status is '%s'", id, string);

		*rval = scriptVar_stringToJsval(context, string);
	}

	return JS_TRUE;
}

JSBool native_socket_socketModeToString(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval)
{
	int id = 0;
	double uid = 0.0;
	char *string = NULL;

	ScriptNativeType type = SCRIPT_NATIVE_TYPE_UNKNOWN;
	Socket *socket = NULL;

	SCRIPT_DISPLAY_NATIVE_FUNCTION_CALL;

	CHECK_ENGINE_ARGS;

	if((argc < 1) || (argv == NULL)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] invalid or missing argument(s)",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}

	uid = scriptVar_getDoubleArg(0, context, argc, argv);

	if(!scriptTypes_decodeNativeId(uid, &type, &id)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] native types id #%0.0f is invalid",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}
	else if(type != SCRIPT_NATIVE_TYPE_SOCKET) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] native types id #%0.0f is not a socket",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}
	else if((socket = scriptTypes_socketGet(nativeEngine->native,
					id)) == NULL) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] failed to obtain new native socket #%i",
				__FILE__, __FUNCTION__, id);
		*rval = scriptVar_booleanToJsval(JS_FALSE);
	}
	else {
		string = socket_socketModeToString(socket->mode);

		log_logf(nativeEngine->log, LOG_LEVEL_INFO,
				"socket #%i mode is '%s'", id, string);

		*rval = scriptVar_stringToJsval(context, string);
	}

	return JS_TRUE;
}

JSBool native_socket_socketProtocolToString(JSContext *context,
		JSObject *object, uintN argc, jsval *argv, jsval *rval)
{
	int id = 0;
	double uid = 0.0;
	char *string = NULL;

	ScriptNativeType type = SCRIPT_NATIVE_TYPE_UNKNOWN;
	Socket *socket = NULL;

	SCRIPT_DISPLAY_NATIVE_FUNCTION_CALL;

	CHECK_ENGINE_ARGS;

	if((argc < 1) || (argv == NULL)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] invalid or missing argument(s)",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}

	uid = scriptVar_getDoubleArg(0, context, argc, argv);

	if(!scriptTypes_decodeNativeId(uid, &type, &id)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] native types id #%0.0f is invalid",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}
	else if(type != SCRIPT_NATIVE_TYPE_SOCKET) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] native types id #%0.0f is not a socket",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}
	else if((socket = scriptTypes_socketGet(nativeEngine->native,
					id)) == NULL) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] failed to obtain new native socket #%i",
				__FILE__, __FUNCTION__, id);
		*rval = scriptVar_booleanToJsval(JS_FALSE);
	}
	else {
		string = socket_socketProtocolToString(socket->protocol);

		log_logf(nativeEngine->log, LOG_LEVEL_INFO,
				"socket #%i protocol is '%s'", id, string);

		*rval = scriptVar_stringToJsval(context, string);
	}

	return JS_TRUE;
}

JSBool native_socket_getBytesSent(JSContext *context,
		JSObject *object, uintN argc, jsval *argv, jsval *rval)
{
	int id = 0;
	double uid = 0.0;

	ScriptNativeType type = SCRIPT_NATIVE_TYPE_UNKNOWN;
	Socket *socket = NULL;

	SCRIPT_DISPLAY_NATIVE_FUNCTION_CALL;

	CHECK_ENGINE_ARGS;

	if((argc < 1) || (argv == NULL)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] invalid or missing argument(s)",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}

	uid = scriptVar_getDoubleArg(0, context, argc, argv);

	if(!scriptTypes_decodeNativeId(uid, &type, &id)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] native types id #%0.0f is invalid",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}
	else if(type != SCRIPT_NATIVE_TYPE_SOCKET) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] native types id #%0.0f is not a socket",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}
	else if((socket = scriptTypes_socketGet(nativeEngine->native,
					id)) == NULL) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] failed to obtain new native socket #%i",
				__FILE__, __FUNCTION__, id);
		*rval = scriptVar_booleanToJsval(JS_FALSE);
	}
	else {
		*rval = scriptVar_integerToJsval(
				(int)socket_getBytesSent(socket));
	}

	return JS_TRUE;
}

JSBool native_socket_getBytesReceived(JSContext *context,
		JSObject *object, uintN argc, jsval *argv, jsval *rval)
{
	int id = 0;
	double uid = 0.0;

	ScriptNativeType type = SCRIPT_NATIVE_TYPE_UNKNOWN;
	Socket *socket = NULL;

	SCRIPT_DISPLAY_NATIVE_FUNCTION_CALL;

	CHECK_ENGINE_ARGS;

	if((argc < 1) || (argv == NULL)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] invalid or missing argument(s)",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}

	uid = scriptVar_getDoubleArg(0, context, argc, argv);

	if(!scriptTypes_decodeNativeId(uid, &type, &id)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] native types id #%0.0f is invalid",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}
	else if(type != SCRIPT_NATIVE_TYPE_SOCKET) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] native types id #%0.0f is not a socket",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}
	else if((socket = scriptTypes_socketGet(nativeEngine->native,
					id)) == NULL) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] failed to obtain new native socket #%i",
				__FILE__, __FUNCTION__, id);
		*rval = scriptVar_booleanToJsval(JS_FALSE);
	}
	else {
		*rval = scriptVar_integerToJsval(
				(int)socket_getBytesReceived(socket));
	}

	return JS_TRUE;
}

JSBool native_socket_getReadsAttempted(JSContext *context,
		JSObject *object, uintN argc, jsval *argv, jsval *rval)
{
	int id = 0;
	double uid = 0.0;

	ScriptNativeType type = SCRIPT_NATIVE_TYPE_UNKNOWN;
	Socket *socket = NULL;

	SCRIPT_DISPLAY_NATIVE_FUNCTION_CALL;

	CHECK_ENGINE_ARGS;

	if((argc < 1) || (argv == NULL)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] invalid or missing argument(s)",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}

	uid = scriptVar_getDoubleArg(0, context, argc, argv);

	if(!scriptTypes_decodeNativeId(uid, &type, &id)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] native types id #%0.0f is invalid",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}
	else if(type != SCRIPT_NATIVE_TYPE_SOCKET) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] native types id #%0.0f is not a socket",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}
	else if((socket = scriptTypes_socketGet(nativeEngine->native,
					id)) == NULL) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] failed to obtain new native socket #%i",
				__FILE__, __FUNCTION__, id);
		*rval = scriptVar_booleanToJsval(JS_FALSE);
	}
	else {
		*rval = scriptVar_integerToJsval(
				(int)socket_getReadsAttempted(socket));
	}

	return JS_TRUE;
}

JSBool native_socket_getReadsCompleted(JSContext *context,
		JSObject *object, uintN argc, jsval *argv, jsval *rval)
{
	int id = 0;
	double uid = 0.0;

	ScriptNativeType type = SCRIPT_NATIVE_TYPE_UNKNOWN;
	Socket *socket = NULL;

	SCRIPT_DISPLAY_NATIVE_FUNCTION_CALL;

	CHECK_ENGINE_ARGS;

	if((argc < 1) || (argv == NULL)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] invalid or missing argument(s)",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}

	uid = scriptVar_getDoubleArg(0, context, argc, argv);

	if(!scriptTypes_decodeNativeId(uid, &type, &id)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] native types id #%0.0f is invalid",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}
	else if(type != SCRIPT_NATIVE_TYPE_SOCKET) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] native types id #%0.0f is not a socket",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}
	else if((socket = scriptTypes_socketGet(nativeEngine->native,
					id)) == NULL) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] failed to obtain new native socket #%i",
				__FILE__, __FUNCTION__, id);
		*rval = scriptVar_booleanToJsval(JS_FALSE);
	}
	else {
		*rval = scriptVar_integerToJsval(
				(int)socket_getReadsCompleted(socket));
	}

	return JS_TRUE;
}

JSBool native_socket_getWritesAttempted(JSContext *context,
		JSObject *object, uintN argc, jsval *argv, jsval *rval)
{
	int id = 0;
	double uid = 0.0;

	ScriptNativeType type = SCRIPT_NATIVE_TYPE_UNKNOWN;
	Socket *socket = NULL;

	SCRIPT_DISPLAY_NATIVE_FUNCTION_CALL;

	CHECK_ENGINE_ARGS;

	if((argc < 1) || (argv == NULL)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] invalid or missing argument(s)",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}

	uid = scriptVar_getDoubleArg(0, context, argc, argv);

	if(!scriptTypes_decodeNativeId(uid, &type, &id)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] native types id #%0.0f is invalid",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}
	else if(type != SCRIPT_NATIVE_TYPE_SOCKET) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] native types id #%0.0f is not a socket",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}
	else if((socket = scriptTypes_socketGet(nativeEngine->native,
					id)) == NULL) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] failed to obtain new native socket #%i",
				__FILE__, __FUNCTION__, id);
		*rval = scriptVar_booleanToJsval(JS_FALSE);
	}
	else {
		*rval = scriptVar_integerToJsval(
				(int)socket_getWritesAttempted(socket));
	}

	return JS_TRUE;
}

JSBool native_socket_getWritesCompleted(JSContext *context,
		JSObject *object, uintN argc, jsval *argv, jsval *rval)
{
	int id = 0;
	double uid = 0.0;

	ScriptNativeType type = SCRIPT_NATIVE_TYPE_UNKNOWN;
	Socket *socket = NULL;

	SCRIPT_DISPLAY_NATIVE_FUNCTION_CALL;

	CHECK_ENGINE_ARGS;

	if((argc < 1) || (argv == NULL)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] invalid or missing argument(s)",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}

	uid = scriptVar_getDoubleArg(0, context, argc, argv);

	if(!scriptTypes_decodeNativeId(uid, &type, &id)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] native types id #%0.0f is invalid",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}
	else if(type != SCRIPT_NATIVE_TYPE_SOCKET) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] native types id #%0.0f is not a socket",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}
	else if((socket = scriptTypes_socketGet(nativeEngine->native,
					id)) == NULL) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] failed to obtain new native socket #%i",
				__FILE__, __FUNCTION__, id);
		*rval = scriptVar_booleanToJsval(JS_FALSE);
	}
	else {
		*rval = scriptVar_integerToJsval(
				(int)socket_getWritesCompleted(socket));
	}

	return JS_TRUE;
}

JSBool native_socket_getTotalReadTime(JSContext *context,
		JSObject *object, uintN argc, jsval *argv, jsval *rval)
{
	int id = 0;
	double uid = 0.0;

	ScriptNativeType type = SCRIPT_NATIVE_TYPE_UNKNOWN;
	Socket *socket = NULL;

	SCRIPT_DISPLAY_NATIVE_FUNCTION_CALL;

	CHECK_ENGINE_ARGS;

	if((argc < 1) || (argv == NULL)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] invalid or missing argument(s)",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}

	uid = scriptVar_getDoubleArg(0, context, argc, argv);

	if(!scriptTypes_decodeNativeId(uid, &type, &id)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] native types id #%0.0f is invalid",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}
	else if(type != SCRIPT_NATIVE_TYPE_SOCKET) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] native types id #%0.0f is not a socket",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}
	else if((socket = scriptTypes_socketGet(nativeEngine->native,
					id)) == NULL) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] failed to obtain new native socket #%i",
				__FILE__, __FUNCTION__, id);
		*rval = scriptVar_booleanToJsval(JS_FALSE);
	}
	else {
		*rval = scriptVar_doubleToJsval(context,
				socket_getTotalReadTime(socket));
	}

	return JS_TRUE;
}

JSBool native_socket_getTotalWriteTime(JSContext *context,
		JSObject *object, uintN argc, jsval *argv, jsval *rval)
{
	int id = 0;
	double uid = 0.0;

	ScriptNativeType type = SCRIPT_NATIVE_TYPE_UNKNOWN;
	Socket *socket = NULL;

	SCRIPT_DISPLAY_NATIVE_FUNCTION_CALL;

	CHECK_ENGINE_ARGS;

	if((argc < 1) || (argv == NULL)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] invalid or missing argument(s)",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}

	uid = scriptVar_getDoubleArg(0, context, argc, argv);

	if(!scriptTypes_decodeNativeId(uid, &type, &id)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] native types id #%0.0f is invalid",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}
	else if(type != SCRIPT_NATIVE_TYPE_SOCKET) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] native types id #%0.0f is not a socket",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}
	else if((socket = scriptTypes_socketGet(nativeEngine->native,
					id)) == NULL) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] failed to obtain new native socket #%i",
				__FILE__, __FUNCTION__, id);
		*rval = scriptVar_booleanToJsval(JS_FALSE);
	}
	else {
		*rval = scriptVar_doubleToJsval(context,
				socket_getTotalWriteTime(socket));
	}

	return JS_TRUE;
}

JSBool native_socket_getSocketDescriptor(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval)
{
	int id = 0;
	int sd = 0;
	double uid = 0.0;

	ScriptNativeType type = SCRIPT_NATIVE_TYPE_UNKNOWN;
	Socket *socket = NULL;

	SCRIPT_DISPLAY_NATIVE_FUNCTION_CALL;

	CHECK_ENGINE_ARGS;

	if((argc < 1) || (argv == NULL)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] invalid or missing argument(s)",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}

	uid = scriptVar_getDoubleArg(0, context, argc, argv);

	if(!scriptTypes_decodeNativeId(uid, &type, &id)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] native types id #%0.0f is invalid",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}
	else if(type != SCRIPT_NATIVE_TYPE_SOCKET) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] native types id #%0.0f is not a socket",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}
	else if((socket = scriptTypes_socketGet(nativeEngine->native,
					id)) == NULL) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] failed to obtain new native socket #%i",
				__FILE__, __FUNCTION__, id);
		*rval = scriptVar_booleanToJsval(JS_FALSE);
	}
	else if((sd = socket_getSocketDescriptor(socket)) < 0) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] failed to obtain descriptor from socket #%i",
				__FILE__, __FUNCTION__, id);
		*rval = scriptVar_booleanToJsval(JS_FALSE);
	}
	else {
		log_logf(nativeEngine->log, LOG_LEVEL_DEBUG,
				"obtained descriptor #%i from socket #%i on '%s:%i'",
				sd, id, socket->hostname, socket->port);

		*rval = scriptVar_integerToJsval(sd);
	}

	return JS_TRUE;
}

JSBool native_socket_open(JSContext *context, JSObject *object, uintN argc,
		jsval *argv, jsval *rval)
{
	int id = 0;
	double uid = 0.0;

	ScriptNativeType type = SCRIPT_NATIVE_TYPE_UNKNOWN;
	Socket *socket = NULL;

	SCRIPT_DISPLAY_NATIVE_FUNCTION_CALL;

	CHECK_ENGINE_ARGS;

	if((argc < 1) || (argv == NULL)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] invalid or missing argument(s)",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}

	uid = scriptVar_getDoubleArg(0, context, argc, argv);

	if(!scriptTypes_decodeNativeId(uid, &type, &id)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] native types id #%0.0f is invalid",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}
	else if(type != SCRIPT_NATIVE_TYPE_SOCKET) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] native types id #%0.0f is not a socket",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}
	else if((socket = scriptTypes_socketGet(nativeEngine->native,
					id)) == NULL) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] failed to obtain new native socket #%i",
				__FILE__, __FUNCTION__, id);
		*rval = scriptVar_booleanToJsval(JS_FALSE);
	}
	else if(socket_open(socket) < 0) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] failed to open socket #%i",
				__FILE__, __FUNCTION__, id);
		*rval = scriptVar_booleanToJsval(JS_FALSE);
	}
	else {
		log_logf(nativeEngine->log, LOG_LEVEL_INFO,
				"opened socket #%i on '%s:%i'",
				id, socket->hostname, socket->port);

		*rval = scriptVar_booleanToJsval(JS_TRUE);
	}

	return JS_TRUE;
}

JSBool native_socket_close(JSContext *context, JSObject *object, uintN argc,
		jsval *argv, jsval *rval)
{
	int id = 0;
	double uid = 0.0;

	ScriptNativeType type = SCRIPT_NATIVE_TYPE_UNKNOWN;
	Socket *socket = NULL;

	SCRIPT_DISPLAY_NATIVE_FUNCTION_CALL;

	CHECK_ENGINE_ARGS;

	if((argc < 1) || (argv == NULL)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] invalid or missing argument(s)",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}

	uid = scriptVar_getDoubleArg(0, context, argc, argv);

	if(!scriptTypes_decodeNativeId(uid, &type, &id)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] native types id #%0.0f is invalid",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}
	else if(type != SCRIPT_NATIVE_TYPE_SOCKET) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] native types id #%0.0f is not a socket",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}
	else if((socket = scriptTypes_socketGet(nativeEngine->native,
					id)) == NULL) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] failed to obtain new native socket #%i",
				__FILE__, __FUNCTION__, id);
		*rval = scriptVar_booleanToJsval(JS_FALSE);
	}
	else if(socket_close(socket) < 0) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] failed to close socket #%i",
				__FILE__, __FUNCTION__, id);
		*rval = scriptVar_booleanToJsval(JS_FALSE);
	}
	else {
		log_logf(nativeEngine->log, LOG_LEVEL_INFO,
				"closed socket #%i on '%s:%i'",
				id, socket->hostname, socket->port);

		*rval = scriptVar_booleanToJsval(JS_TRUE);
	}

	return JS_TRUE;
}

JSBool native_socket_closeChildSocket(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval)
{
	int id = 0;
	int sd = 0;
	double uid = 0.0;

	ScriptNativeType type = SCRIPT_NATIVE_TYPE_UNKNOWN;
	Socket *socket = NULL;

	SCRIPT_DISPLAY_NATIVE_FUNCTION_CALL;

	CHECK_ENGINE_ARGS;

	if((argc < 1) || (argv == NULL)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] invalid or missing argument(s)",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}

	uid = scriptVar_getDoubleArg(0, context, argc, argv);
	sd = scriptVar_getIntegerArg(1, context, argc, argv);

	if(!scriptTypes_decodeNativeId(uid, &type, &id)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] native types id #%0.0f is invalid",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}
	else if(type != SCRIPT_NATIVE_TYPE_SOCKET) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] native types id #%0.0f is not a socket",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}
	else if((socket = scriptTypes_socketGet(nativeEngine->native,
					id)) == NULL) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] failed to obtain new native socket #%i",
				__FILE__, __FUNCTION__, id);
		*rval = scriptVar_booleanToJsval(JS_FALSE);
	}
	else if(socket_closeChildSocket(socket, sd) < 0) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] failed to close child socket #%i on socket #%i",
				__FILE__, __FUNCTION__, sd, id);
		*rval = scriptVar_booleanToJsval(JS_FALSE);
	}
	else {
		log_logf(nativeEngine->log, LOG_LEVEL_DEBUG,
				"closed child socket #%i on socket #%i on '%s:%i'",
				sd, id, socket->hostname, socket->port);

		*rval = scriptVar_booleanToJsval(JS_TRUE);
	}

	return JS_TRUE;
}

JSBool native_socket_requestShutdown(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval)
{
	int id = 0;
	double uid = 0.0;

	ScriptNativeType type = SCRIPT_NATIVE_TYPE_UNKNOWN;
	Socket *socket = NULL;

	SCRIPT_DISPLAY_NATIVE_FUNCTION_CALL;

	CHECK_ENGINE_ARGS;

	if((argc < 1) || (argv == NULL)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] invalid or missing argument(s)",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}

	uid = scriptVar_getDoubleArg(0, context, argc, argv);

	if(!scriptTypes_decodeNativeId(uid, &type, &id)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] native types id #%0.0f is invalid",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}
	else if(type != SCRIPT_NATIVE_TYPE_SOCKET) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] native types id #%0.0f is not a socket",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}
	else if((socket = scriptTypes_socketGet(nativeEngine->native,
					id)) == NULL) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] failed to obtain new native socket #%i",
				__FILE__, __FUNCTION__, id);
		*rval = scriptVar_booleanToJsval(JS_FALSE);
	}
	else if(socket_requestShutdown(socket) < 0) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] failed to request shutdown for socket #%i",
				__FILE__, __FUNCTION__, id);
		*rval = scriptVar_booleanToJsval(JS_FALSE);
	}
	else {
		log_logf(nativeEngine->log, LOG_LEVEL_DEBUG,
				"requested shutdown for socket #%i on '%s:%i'",
				id, socket->hostname, socket->port);

		*rval = scriptVar_booleanToJsval(JS_TRUE);
	}

	return JS_TRUE;
}

JSBool native_socket_listen(JSContext *context, JSObject *object, uintN argc,
		jsval *argv, jsval *rval)
{
	int id = 0;
	double uid = 0.0;

	ScriptNativeType type = SCRIPT_NATIVE_TYPE_UNKNOWN;
	Socket *socket = NULL;

	SCRIPT_DISPLAY_NATIVE_FUNCTION_CALL;

	CHECK_ENGINE_ARGS;

	if((argc < 1) || (argv == NULL)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] invalid or missing argument(s)",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}

	uid = scriptVar_getDoubleArg(0, context, argc, argv);

	if(!scriptTypes_decodeNativeId(uid, &type, &id)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] native types id #%0.0f is invalid",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}
	else if(type != SCRIPT_NATIVE_TYPE_SOCKET) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] native types id #%0.0f is not a socket",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}
	else if((socket = scriptTypes_socketGet(nativeEngine->native,
					id)) == NULL) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] failed to obtain new native socket #%i",
				__FILE__, __FUNCTION__, id);
		*rval = scriptVar_booleanToJsval(JS_FALSE);
	}
	else if(socket_listen(socket) < 0) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] failed to listen on socket #%i",
				__FILE__, __FUNCTION__, id);
		*rval = scriptVar_booleanToJsval(JS_FALSE);
	}
	else {
		log_logf(nativeEngine->log, LOG_LEVEL_DEBUG,
				"listening on socket #%i on '%s:%i'",
				id, socket->hostname, socket->port);

		*rval = scriptVar_booleanToJsval(JS_TRUE);
	}

	return JS_TRUE;
}

JSBool native_socket_accept(JSContext *context, JSObject *object, uintN argc,
		jsval *argv, jsval *rval)
{
	int id = 0;
	int sd = 0;
	int microseconds = 0;
	double uid = 0.0;

	ScriptNativeType type = SCRIPT_NATIVE_TYPE_UNKNOWN;
	Socket *socket = NULL;

	SCRIPT_DISPLAY_NATIVE_FUNCTION_CALL;

	CHECK_ENGINE_ARGS;

	if((argc < 1) || (argv == NULL)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] invalid or missing argument(s)",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}

	uid = scriptVar_getDoubleArg(0, context, argc, argv);
	microseconds = scriptVar_getIntegerArg(1, context, argc, argv);

	if(!scriptTypes_decodeNativeId(uid, &type, &id)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] native types id #%0.0f is invalid",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}
	else if(type != SCRIPT_NATIVE_TYPE_SOCKET) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] native types id #%0.0f is not a socket",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}
	else if((socket = scriptTypes_socketGet(nativeEngine->native,
					id)) == NULL) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] failed to obtain new native socket #%i",
				__FILE__, __FUNCTION__, id);
		*rval = scriptVar_booleanToJsval(JS_FALSE);
	}
	else if(socket_accept(socket, &sd, microseconds) < 0) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] failed to accept socket #%i",
				__FILE__, __FUNCTION__, id);
		*rval = scriptVar_booleanToJsval(JS_FALSE);
	}
	else {
		log_logf(nativeEngine->log, LOG_LEVEL_DEBUG,
				"accepted socket #%i on '%s:%i'",
				id, socket->hostname, socket->port);

		*rval = scriptVar_integerToJsval(sd);
	}

	return JS_TRUE;
}

JSBool native_socket_getPeerName(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval)
{
	int id = 0;
	int sd = 0;
	double uid = 0.0;
	char *peerName = NULL;

	ScriptNativeType type = SCRIPT_NATIVE_TYPE_UNKNOWN;
	Socket *socket = NULL;

	SCRIPT_DISPLAY_NATIVE_FUNCTION_CALL;

	CHECK_ENGINE_ARGS;

	if((argc < 1) || (argv == NULL)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] invalid or missing argument(s)",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}

	uid = scriptVar_getDoubleArg(0, context, argc, argv);

	if(!scriptTypes_decodeNativeId(uid, &type, &id)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] native types id #%0.0f is invalid",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}
	else if(type != SCRIPT_NATIVE_TYPE_SOCKET) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] native types id #%0.0f is not a socket",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}
	else if((socket = scriptTypes_socketGet(nativeEngine->native,
					id)) == NULL) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] failed to obtain new native socket #%i",
				__FILE__, __FUNCTION__, id);
		*rval = scriptVar_booleanToJsval(JS_FALSE);
	}
	else if(socket_getPeerName(socket, sd, &peerName) < 0) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] failed to get peer name from socket #%i",
				__FILE__, __FUNCTION__, id);
		*rval = scriptVar_booleanToJsval(JS_FALSE);
	}
	else {
		log_logf(nativeEngine->log, LOG_LEVEL_DEBUG,
				"obtained peer name '%s' from socket #%i on '%s:%i'",
				peerName, id, socket->hostname, socket->port);

		*rval = scriptVar_stringToJsval(context, peerName);
	}

	return JS_TRUE;
}

JSBool native_socket_send(JSContext *context, JSObject *object, uintN argc,
		jsval *argv, jsval *rval)
{
	int id = 0;
	int sd = 0;
	int valueLength = 0;
	double uid = 0.0;
	char *value = NULL;

	ScriptNativeType type = SCRIPT_NATIVE_TYPE_UNKNOWN;
	Socket *socket = NULL;

	SCRIPT_DISPLAY_NATIVE_FUNCTION_CALL;

	CHECK_ENGINE_ARGS;

	if((argc < 1) || (argv == NULL)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] invalid or missing argument(s)",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}

	uid = scriptVar_getDoubleArg(0, context, argc, argv);
	sd = scriptVar_getIntegerArg(1, context, argc, argv);
	value = scriptVar_getStringArg(2, context, argc, argv);
	valueLength = scriptVar_getIntegerArg(3, context, argc, argv);

	if(!scriptTypes_decodeNativeId(uid, &type, &id)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] native types id #%0.0f is invalid",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}
	else if(type != SCRIPT_NATIVE_TYPE_SOCKET) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] native types id #%0.0f is not a socket",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}
	else if((socket = scriptTypes_socketGet(nativeEngine->native,
					id)) == NULL) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] failed to obtain new native socket #%i",
				__FILE__, __FUNCTION__, id);
		*rval = scriptVar_booleanToJsval(JS_FALSE);
	}
	else if(socket_send(socket, sd, value, valueLength) < 0) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] failed to send on socket #%i",
				__FILE__, __FUNCTION__, id);
		*rval = scriptVar_booleanToJsval(JS_FALSE);
	}
	else {
		log_logf(nativeEngine->log, LOG_LEVEL_DEBUG,
				"sent %i bytes on socket #%i on '%s:%i'",
				valueLength, id, socket->hostname, socket->port);

		*rval = scriptVar_booleanToJsval(JS_TRUE);
	}

	return JS_TRUE;
}

JSBool native_socket_receive(JSContext *context, JSObject *object, uintN argc,
		jsval *argv, jsval *rval)
{
	int id = 0;
	int sd = 0;
	int valueLength = 0;
	double uid = 0.0;
	double timeoutSeconds = 0.0;
	char *value = NULL;

	ScriptNativeType type = SCRIPT_NATIVE_TYPE_UNKNOWN;
	Socket *socket = NULL;

	SCRIPT_DISPLAY_NATIVE_FUNCTION_CALL;

	CHECK_ENGINE_ARGS;

	if((argc < 1) || (argv == NULL)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] invalid or missing argument(s)",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}

	uid = scriptVar_getDoubleArg(0, context, argc, argv);
	sd = scriptVar_getIntegerArg(1, context, argc, argv);
	valueLength = scriptVar_getIntegerArg(2, context, argc, argv);
	timeoutSeconds = scriptVar_getDoubleArg(3, context, argc, argv);

	if(!scriptTypes_decodeNativeId(uid, &type, &id)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] native types id #%0.0f is invalid",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}
	else if(type != SCRIPT_NATIVE_TYPE_SOCKET) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] native types id #%0.0f is not a socket",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}
	else if((socket = scriptTypes_socketGet(nativeEngine->native,
					id)) == NULL) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] failed to obtain new native socket #%i",
				__FILE__, __FUNCTION__, id);
		*rval = scriptVar_booleanToJsval(JS_FALSE);
	}
	else {
		value = (char *)malloc(sizeof(char) * (valueLength + 1));

		if(socket_receive(socket, sd, value, valueLength, timeoutSeconds) < 0) {
			log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
					"[%s->%s()] failed to receive from socket #%i",
					__FILE__, __FUNCTION__, id);
			*rval = scriptVar_booleanToJsval(JS_FALSE);
		}
		else {
			log_logf(nativeEngine->log, LOG_LEVEL_DEBUG,
					"received %i bytes from socket #%i on '%s:%i'",
					valueLength, id, socket->hostname, socket->port);

			*rval = scriptVar_stringToJsval(context, value);
		}

		free(value);
	}

	return JS_TRUE;
}

// thread functions

JSBool native_thread_new(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval)
{
	int id = 0;
	double uid = 0.0;
	char *functionName = NULL;

	ScriptNativeThread *thread = NULL;

	SCRIPT_DISPLAY_NATIVE_FUNCTION_CALL;

	CHECK_ENGINE_ARGS;

	if((argc < 1) || (argv == NULL)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] invalid or missing argument(s)",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}

	if((functionName = scriptVar_getStringArg(0, context, argc,
					argv)) == NULL) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] failed to obtain 'functionName' argument",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(JS_FALSE);
		return JS_TRUE;
	}

	if((id = scriptTypes_threadNew(nativeEngine, functionName)) < 0) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] failed to create new native thread",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(JS_FALSE);
	}
	else if((thread = scriptTypes_threadGet(nativeEngine->native,
					id)) == NULL) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] failed to obtain new native thread #%i",
				__FILE__, __FUNCTION__, id);
		*rval = scriptVar_booleanToJsval(JS_FALSE);
	}
	else {
		log_logf(nativeEngine->log, LOG_LEVEL_DEBUG,
				"created native thread context #%i",
				__FILE__, __FUNCTION__, id);

		uid = scriptTypes_encodeNativeId(SCRIPT_NATIVE_TYPE_THREAD, id);

		*rval = scriptVar_doubleToJsval(context, uid);
	}

	return JS_TRUE;
}

JSBool native_thread_create(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval)
{
	int id = 0;
	int counter = 0;
	double uid = 0.0;

	ScriptNativeType type = SCRIPT_NATIVE_TYPE_UNKNOWN;
	ScriptNativeThread *thread = NULL;

	SCRIPT_DISPLAY_NATIVE_FUNCTION_CALL;

	CHECK_ENGINE_ARGS;

	if((argc < 1) || (argv == NULL)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] invalid or missing argument(s)",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}

	uid = scriptVar_getDoubleArg(0, context, argc, argv);

	if(!scriptTypes_decodeNativeId(uid, &type, &id)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] native types id #%0.0f is invalid",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}
	else if(type != SCRIPT_NATIVE_TYPE_THREAD) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] native types id #%0.0f is not a thread",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}
	else if((thread = scriptTypes_threadGet(nativeEngine->native,
					id)) == NULL) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] failed to obtain new native thread #%i",
				__FILE__, __FUNCTION__, id);
		*rval = scriptVar_booleanToJsval(JS_FALSE);
	}
	else if(thread_create(&thread->thread, nativeWorkerThread,
				(void *)&id) < 0) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] failed to spawn thread on native thread #%i",
				__FILE__, __FUNCTION__, id);
		*rval = scriptVar_booleanToJsval(JS_FALSE);
	}
	else {
		log_logf(nativeEngine->log, LOG_LEVEL_INFO,
				"spawned thread on native thread #%i", id);

		do {
			time_usleep(1024);
			counter++;
		} while((counter < 8192) &&
				(thread->state != NATIVE_THREAD_STATE_RUNNING));

		if(thread->state != NATIVE_THREAD_STATE_RUNNING) {
			log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
					"[%s->%s()] failed to start thread on native thread #%i",
					__FILE__, __FUNCTION__, id);

			*rval = scriptVar_booleanToJsval(JS_FALSE);
		}
		else {
			*rval = scriptVar_booleanToJsval(JS_TRUE);
		}
	}

	return JS_TRUE;
}

JSBool native_thread_join(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval)
{
	int id = 0;
	double uid = 0.0;
	void *exitValue = NULL;

	ScriptNativeType type = SCRIPT_NATIVE_TYPE_UNKNOWN;
	ScriptNativeThread *thread = NULL;

	SCRIPT_DISPLAY_NATIVE_FUNCTION_CALL;

	CHECK_ENGINE_ARGS;

	if((argc < 1) || (argv == NULL)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] invalid or missing argument(s)",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}

	uid = scriptVar_getDoubleArg(0, context, argc, argv);

	if(!scriptTypes_decodeNativeId(uid, &type, &id)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] native types id #%0.0f is invalid",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}
	else if(type != SCRIPT_NATIVE_TYPE_THREAD) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] native types id #%0.0f is not a thread",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}
	else if((thread = scriptTypes_threadGet(nativeEngine->native,
					id)) == NULL) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] failed to obtain new native thread #%i",
				__FILE__, __FUNCTION__, id);
		*rval = scriptVar_booleanToJsval(JS_FALSE);
	}
	else if(thread_join(&thread->thread, &exitValue) < 0) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] failed to join thread on native thread #%i",
				__FILE__, __FUNCTION__, id);
		*rval = scriptVar_booleanToJsval(JS_FALSE);
	}
	else {
		log_logf(nativeEngine->log, LOG_LEVEL_INFO,
				"joined thread on native thread #%i", id);

		*rval = scriptVar_booleanToJsval(JS_TRUE);
	}

	return JS_TRUE;
}

JSBool native_thread_self(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval)
{
	int threadId = 0;

	SCRIPT_DISPLAY_NATIVE_FUNCTION_CALL;

	CHECK_ENGINE_ARGS;

	if(thread_self(&threadId) < 0) {
		*rval = scriptVar_booleanToJsval(JS_FALSE);
	}
	else {
		*rval = scriptVar_integerToJsval(threadId);
	}

	return JS_TRUE;
}


// language native interface functions

JSBool native_getOsType(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval)
{
	SCRIPT_DISPLAY_NATIVE_FUNCTION_CALL;
	CHECK_ENGINE_ARGS;

#if defined (__linux__)
	*rval = scriptVar_stringToJsval(context, "Linux");
#elif defined (__APPLE__)
	*rval = scriptVar_stringToJsval(context, "Apple");
#elif defined (WIN32)
	*rval = scriptVar_stringToJsval(context, "Windows");
#else // - unknown operating system
	*rval = scriptVar_stringToJsval(context, "Unknown");
#endif // - operating systems

	return JS_TRUE;
}

JSBool native_getConstant(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval)
{
	char *name = NULL;

	ScriptNativeVar *nativeVar = NULL;

	SCRIPT_DISPLAY_NATIVE_FUNCTION_CALL;

	CHECK_ENGINE_ARGS;

	if((argc < 1) || (argv == NULL)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] invalid or missing argument(s)",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}

	if((name = scriptVar_getStringArg(0, context, argc, argv)) == NULL) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] failed to obtain 'name' argument",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(JS_FALSE);
		return JS_TRUE;
	}

	if((nativeVar = scriptTypes_getConstant(nativeEngine, name)) == NULL) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] failed to locate '%s' constant",
				__FILE__, __FUNCTION__, name);
		*rval = scriptVar_booleanToJsval(JS_FALSE);
		return JS_TRUE;
	}

	switch(nativeVar->type) {
		case SCRIPT_NATIVE_VAR_TYPE_BOOLEAN:
			*rval = scriptVar_booleanToJsval(
					scriptTypes_getBooleanValueOfNativeVar(nativeVar));
			break;

		case SCRIPT_NATIVE_VAR_TYPE_INTEGER:
			*rval = scriptVar_integerToJsval(
					scriptTypes_getIntegerValueOfNativeVar(nativeVar));
			break;

		case SCRIPT_NATIVE_VAR_TYPE_DOUBLE:
			*rval = scriptVar_doubleToJsval(context,
					scriptTypes_getDoubleValueOfNativeVar(nativeVar));
			break;

		case SCRIPT_NATIVE_VAR_TYPE_STRING:
			*rval = scriptVar_stringToJsval(context,
					scriptTypes_getStringValueOfNativeVar(nativeVar));
			break;

		case SCRIPT_NATIVE_VAR_TYPE_UNKNOWN:
		default:
			*rval = scriptVar_booleanToJsval(JS_FALSE);
	}

	return JS_TRUE;
}

JSBool native_getVariable(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval)
{
	char *name = NULL;
	char *nameSpace = NULL;

	ScriptNativeVar *nativeVar = NULL;

	SCRIPT_DISPLAY_NATIVE_FUNCTION_CALL;

	CHECK_ENGINE_ARGS;

	if((argc < 2) || (argv == NULL)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] invalid or missing argument(s)",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}

	if((nameSpace = scriptVar_getStringArg(0, context, argc, argv)) == NULL) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] failed to obtain 'nameSpace' argument",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(JS_FALSE);
		return JS_TRUE;
	}

	if((name = scriptVar_getStringArg(1, context, argc, argv)) == NULL) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] failed to obtain 'name' argument",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(JS_FALSE);
		return JS_TRUE;
	}

	if((nativeVar = scriptTypes_getVariable(nativeEngine, nameSpace,
					name)) == NULL) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] failed to locate '%s' variable in name space '%s'",
				__FILE__, __FUNCTION__, name, nameSpace);
		*rval = scriptVar_booleanToJsval(JS_FALSE);
		return JS_TRUE;
	}

	switch(nativeVar->type) {
		case SCRIPT_NATIVE_VAR_TYPE_BOOLEAN:
			*rval = scriptVar_booleanToJsval(
					scriptTypes_getBooleanValueOfNativeVar(nativeVar));
			break;

		case SCRIPT_NATIVE_VAR_TYPE_INTEGER:
			*rval = scriptVar_integerToJsval(
					scriptTypes_getIntegerValueOfNativeVar(nativeVar));
			break;

		case SCRIPT_NATIVE_VAR_TYPE_DOUBLE:
			*rval = scriptVar_doubleToJsval(context,
					scriptTypes_getDoubleValueOfNativeVar(nativeVar));
			break;

		case SCRIPT_NATIVE_VAR_TYPE_STRING:
			*rval = scriptVar_stringToJsval(context,
					scriptTypes_getStringValueOfNativeVar(nativeVar));
			break;

		case SCRIPT_NATIVE_VAR_TYPE_UNKNOWN:
		default:
			*rval = scriptVar_booleanToJsval(JS_FALSE);
	}

	return JS_TRUE;
}

JSBool native_setVariable(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval)
{
	int ii = 0;
	char *name = NULL;
	char *nameSpace = NULL;

	SCRIPT_DISPLAY_NATIVE_FUNCTION_CALL;

	CHECK_ENGINE_ARGS;

	if((argc < 3) || (argv == NULL)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] invalid or missing argument(s)",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}

	for(ii = 0; ii < (int)argc; ii += 3) {
		if((nameSpace = scriptVar_getStringArg(ii, context, argc,
						argv)) == NULL) {
			log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
					"[%s->%s()] failed to obtain 'nameSpace' argument",
					__FILE__, __FUNCTION__);
			*rval = scriptVar_booleanToJsval(JS_FALSE);
			return JS_TRUE;
		}

		if((name = scriptVar_getStringArg((ii + 1), context, argc,
						argv)) == NULL) {
			log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
					"[%s->%s()] failed to obtain 'name' argument",
					__FILE__, __FUNCTION__);
			*rval = scriptVar_booleanToJsval(JS_FALSE);
			return JS_TRUE;
		}

		if(JSVAL_IS_BOOLEAN(argv[(ii + 2)])) {
			if(!scriptTypes_registerBooleanVariable(nativeEngine,
						nameSpace,
						name,
						scriptVar_jsvalToBoolean(context, &argv[(ii + 2)]))) {
				log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
						"[%s->%s()] failed to set Boolean variable '%s' "
						"in name space '%s'",
						__FILE__, __FUNCTION__, name, nameSpace);
				*rval = scriptVar_booleanToJsval(JS_FALSE);
				return JS_TRUE;
			}
		}
		else if(JSVAL_IS_INT(argv[(ii + 2)])) {
			if(!scriptTypes_registerIntegerVariable(nativeEngine,
						nameSpace,
						name,
						scriptVar_jsvalToInteger(context, &argv[(ii + 2)]))) {
				log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
						"[%s->%s()] failed to set Integer variable '%s' "
						"in name space '%s'",
						__FILE__, __FUNCTION__, name, nameSpace);
				*rval = scriptVar_booleanToJsval(JS_FALSE);
				return JS_TRUE;
			}
		}
		else if(JSVAL_IS_DOUBLE(argv[(ii + 2)])) {
			if(!scriptTypes_registerDoubleVariable(nativeEngine,
						nameSpace,
						name,
						scriptVar_jsvalToDouble(context, &argv[(ii + 2)]))) {
				log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
						"[%s->%s()] failed to set Double variable '%s' "
						"in name space '%s'",
						__FILE__, __FUNCTION__, name, nameSpace);
				*rval = scriptVar_booleanToJsval(JS_FALSE);
				return JS_TRUE;
			}
		}
		else if(JSVAL_IS_STRING(argv[(ii + 2)])) {
			if(!scriptTypes_registerStringVariable(nativeEngine,
						nameSpace,
						name,
						scriptVar_jsvalToString(context, &argv[(ii + 2)]))) {
				log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
						"[%s->%s()] failed to set String variable '%s' "
						"in name space '%s'",
						__FILE__, __FUNCTION__, name, nameSpace);
				*rval = scriptVar_booleanToJsval(JS_FALSE);
				return JS_TRUE;
			}
		}
	}

	*rval = scriptVar_booleanToJsval(JS_TRUE);

	return JS_TRUE;
}

// language extension functions

JSBool native_include(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval)
{
	int rc = 0;
	char *filename = NULL;
	char *errorMessage = NULL;

	FileHandle fh;

	jsval localRval;
	JSScript *scriptHandle = NULL;

	SCRIPT_DISPLAY_NATIVE_FUNCTION_CALL;

	CHECK_ENGINE_ARGS;

	if((argc < 1) || (argv == NULL)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] invalid or missing argument(s)",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}

	if((filename = scriptVar_getStringArg(0, context, argc, argv)) == NULL) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] failed to obtain 'filename' argument",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(JS_FALSE);
		return JS_TRUE;
	}

	if((rc = file_init(&fh, filename, "r", 0)) < 0) {
		if(file_getError(&fh, rc, &errorMessage) < 0) {
			errorMessage = "unknown file error";
		}

		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] failed to open '%s' with '%s'",
				__FILE__, __FUNCTION__, filename, errorMessage);

		file_free(&fh);

		*rval = scriptVar_booleanToJsval(JS_FALSE);

		return JS_TRUE;
	}

	if((scriptHandle = JS_CompileFileHandle(context, object, filename,
					fh.fd)) != NULL) {
		if(JS_ExecuteScript(context, object, scriptHandle,
					&localRval) == JS_FALSE) {
			log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
					"[%s->%s()] failed to execute script from '%s'.",
					__FILE__, __FUNCTION__, filename);
			*rval = scriptVar_booleanToJsval(JS_FALSE);
			file_free(&fh);
			return JS_TRUE;
		}
	}
	else {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] failed to compile script from '%s'.",
				__FILE__, __FUNCTION__, filename);
		*rval = scriptVar_booleanToJsval(JS_FALSE);
		file_free(&fh);
		return JS_TRUE;
	}

	file_free(&fh);

	*rval = scriptVar_booleanToJsval(JS_TRUE);

	return JS_TRUE;
}

JSBool native_print(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval)
{
	char *string = NULL;

	SCRIPT_DISPLAY_NATIVE_FUNCTION_CALL;
	CHECK_ENGINE_ARGS;

	if((argc < 1) || (argv == NULL)) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] invalid or missing argument(s)",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(afalse);
		return JS_TRUE;
	}

	if((string = scriptVar_getStringArg(0, context, argc, argv)) == NULL) {
		log_logf(nativeEngine->log, LOG_LEVEL_ERROR,
				"[%s->%s()] failed to obtain 'string' argument",
				__FILE__, __FUNCTION__);
		*rval = scriptVar_booleanToJsval(JS_FALSE);
		return JS_TRUE;
	}

	fprintf(stdout, "%s\n", string);

	*rval = scriptVar_booleanToJsval(JS_TRUE);

	return JS_TRUE;
}

JSBool native_rand(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval)
{
	int result = 0;

	SCRIPT_DISPLAY_NATIVE_FUNCTION_CALL;
	CHECK_ENGINE_ARGS;

	result = rand();

	*rval = scriptVar_integerToJsval(result);

	return JS_TRUE;
}

// engine interface functions

ScriptEngine *native_getScriptEngineContext()
{
	return nativeEngine;
}

void native_setScriptEngineContext(ScriptEngine *scriptEngine)
{
	if(scriptEngine != NULL) {
		/*
		 * TODO: free existing context, or determine if this is not valid
		 */
	}

	nativeEngine = scriptEngine;
}

aboolean native_registerConstants()
{
	char *string = NULL;

	if(nativeEngine == NULL) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	// register native function constants

	// native core/common constants

	scriptTypes_registerIntegerConstant(nativeEngine,
			"INT_ONE_THOUSAND", INT_ONE_THOUSAND);
	scriptTypes_registerDoubleConstant(nativeEngine,
			"REAL_ONE_THOUSAND", REAL_ONE_THOUSAND);
	scriptTypes_registerIntegerConstant(nativeEngine,
			"INT_ONE_MILLION", INT_ONE_MILLION);
	scriptTypes_registerDoubleConstant(nativeEngine,
			"REAL_ONE_MILLION", REAL_ONE_MILLION);
	scriptTypes_registerIntegerConstant(nativeEngine,
			"INT_ONE_BILLION", INT_ONE_BILLION);
	scriptTypes_registerDoubleConstant(nativeEngine,
			"REAL_ONE_BILLION", REAL_ONE_BILLION);

	string = (char *)malloc(sizeof(char) * 8);
	string[0] = DIR_SEPARATOR;

	scriptTypes_registerStringConstant(nativeEngine,
			"DIR_SEPARATOR", string);

	free(string);

	scriptTypes_registerStringConstant(nativeEngine,
			"ASGARD_VERSION", ASGARD_VERSION);
	scriptTypes_registerStringConstant(nativeEngine,
			"ASGARD_DATE", ASGARD_DATE);

	// native core/os constants

	scriptTypes_registerIntegerConstant(nativeEngine,
			"SIZEOF_BOOLEAN", SIZEOF_BOOLEAN);
	scriptTypes_registerIntegerConstant(nativeEngine,
			"SIZEOF_ALINT", SIZEOF_ALINT);
	scriptTypes_registerIntegerConstant(nativeEngine,
			"SIZEOF_AULINT", SIZEOF_AULINT);
	scriptTypes_registerIntegerConstant(nativeEngine,
			"SIZEOF_CHAR", SIZEOF_CHAR);
	scriptTypes_registerIntegerConstant(nativeEngine,
			"SIZEOF_SHORT", SIZEOF_SHORT);
	scriptTypes_registerIntegerConstant(nativeEngine,
			"SIZEOF_INT", SIZEOF_INT);
	scriptTypes_registerIntegerConstant(nativeEngine,
			"SIZEOF_FLOAT", SIZEOF_FLOAT);
	scriptTypes_registerIntegerConstant(nativeEngine,
			"SIZEOF_DOUBLE", SIZEOF_DOUBLE);

	// native core/system constants

	scriptTypes_registerIntegerConstant(nativeEngine,
			"SIGNAL_INTERRUPT", SIGNAL_INTERRUPT);
	scriptTypes_registerIntegerConstant(nativeEngine,
			"SIGNAL_TERMINATE", SIGNAL_TERMINATE);
	scriptTypes_registerIntegerConstant(nativeEngine,
			"SIGNAL_SEGFAULT", SIGNAL_SEGFAULT);
	scriptTypes_registerIntegerConstant(nativeEngine,
			"SIGNAL_USERONE", SIGNAL_USERONE);
	scriptTypes_registerIntegerConstant(nativeEngine,
			"SIGNAL_USERTWO", SIGNAL_USERTWO);
	scriptTypes_registerIntegerConstant(nativeEngine,
			"SIGNAL_QUIT", SIGNAL_QUIT);
	scriptTypes_registerIntegerConstant(nativeEngine,
			"SIGNAL_ABORT", SIGNAL_ABORT);
	scriptTypes_registerIntegerConstant(nativeEngine,
			"SIGNAL_FLOATING_POINT_EXCEPTION",
			SIGNAL_FLOATING_POINT_EXCEPTION);
	scriptTypes_registerIntegerConstant(nativeEngine,
			"SIGNAL_ILLEGAL", SIGNAL_ILLEGAL);

	scriptTypes_registerIntegerConstant(nativeEngine,
			"SOCKET_MODE_CLIENT", NATIVE_SOCKET_MODE_CLIENT);
	scriptTypes_registerIntegerConstant(nativeEngine,
			"SOCKET_MODE_SERVER", NATIVE_SOCKET_MODE_SERVER);

	scriptTypes_registerIntegerConstant(nativeEngine,
			"SOCKET_PROTOCOL_TCPIP", NATIVE_SOCKET_PROTOCOL_TCPIP);
	scriptTypes_registerIntegerConstant(nativeEngine,
			"SOCKET_PROTOCOL_UDP", NATIVE_SOCKET_PROTOCOL_UDP);

	scriptTypes_registerIntegerConstant(nativeEngine,
			"THREAD_STATE_INIT", NATIVE_THREAD_STATE_INIT);
	scriptTypes_registerIntegerConstant(nativeEngine,
			"THREAD_STATE_RUNNING", NATIVE_THREAD_STATE_RUNNING);
	scriptTypes_registerIntegerConstant(nativeEngine,
			"THREAD_STATE_HALT", NATIVE_THREAD_STATE_HALT);
	scriptTypes_registerIntegerConstant(nativeEngine,
			"THREAD_STATE_STOPPED", NATIVE_THREAD_STATE_STOPPED);
	scriptTypes_registerIntegerConstant(nativeEngine,
			"THREAD_STATE_ERROR", NATIVE_THREAD_STATE_ERROR);

	return atrue;
}

aboolean native_registerFunctions()
{
	ScriptEngineContext *context = NULL;

	if(nativeEngine == NULL) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	if((context = scriptCommon_getContext(nativeEngine)) == NULL) {
		return afalse;
	}

	mutex_lock(&context->mutex);

	if(JS_DefineFunctions(context->spiderMonkey.context,
				context->spiderMonkey.global,
				FUNCTION_LIST) == JS_FALSE) {
		mutex_unlock(&context->mutex);
		return afalse;
	}

	mutex_unlock(&context->mutex);

	return atrue;
}

aboolean native_registerFunctionsDirect(SpiderMonkey *spiderMonkey)
{
	if(spiderMonkey == NULL) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	if(JS_DefineFunctions(spiderMonkey->context,
				spiderMonkey->global,
				FUNCTION_LIST) == JS_FALSE) {
		return afalse;
	}

	return atrue;
}

