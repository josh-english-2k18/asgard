/*
 * functions.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The SpiderMonkey JavaScript engine for Asgard, native function wrapper
 * library, header file.
 *
 * Written by Josh English.
 */

#if !defined(_SCRIPT_FUNCTIONS_H)

#define _SCRIPT_FUNCTIONS_H

#if !defined(_SCRIPT_H) && !defined(_SCRIPT_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _SCRIPT_H || _SCRIPT_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// declare functions public functions

// native core/common functions

JSBool native_asgard_getVersion(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval);

JSBool native_asgard_getDate(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval);

// native core/os functions

JSBool native_time_getTimeSeconds(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval);

JSBool native_time_getTimeMus(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval);

JSBool native_time_getElapsedSeconds(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval);

JSBool native_time_getElapsedMus(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval);

JSBool native_time_getElapsedMusInMillis(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval);

JSBool native_time_getElapsedMusInSeconds(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval);

JSBool native_time_getTimestamp(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval);

JSBool native_time_sleep(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval);

JSBool native_time_usleep(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval);

JSBool native_time_nanosleep(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval);

// native core/system functions

// mutex functions

JSBool native_mutex_new(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval);

JSBool native_mutex_lock(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval);

JSBool native_mutex_unlock(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval);

JSBool native_mutex_free(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval);

// signal functions

JSBool native_signal_registerAction(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval);

JSBool native_signal_executeSignal(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval);

JSBool native_signal_toString(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval);

// system functions

JSBool native_system_fileSetNative(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval);

JSBool native_system_fileExists(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval);

JSBool native_system_fileLength(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval);

JSBool native_system_fileModifiedTime(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval);

JSBool native_system_fileDelete(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval);

JSBool native_system_fileExecute(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval);

JSBool native_system_dirExists(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval);

JSBool native_system_dirChangeTo(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval);

JSBool native_system_dirCreate(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval);

JSBool native_system_dirDelete(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval);

JSBool native_system_pickRandomSeed(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval);

JSBool native_system_setRandomSeed(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval);

// socket functions

JSBool native_socket_new(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval);

JSBool native_socket_socketStateToString(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval);

JSBool native_socket_socketModeToString(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval);

JSBool native_socket_socketProtocolToString(JSContext *context,
		JSObject *object, uintN argc, jsval *argv, jsval *rval);

JSBool native_socket_getBytesSent(JSContext *context,
		JSObject *object, uintN argc, jsval *argv, jsval *rval);

JSBool native_socket_getBytesReceived(JSContext *context,
		JSObject *object, uintN argc, jsval *argv, jsval *rval);

JSBool native_socket_getReadsAttempted(JSContext *context,
		JSObject *object, uintN argc, jsval *argv, jsval *rval);

JSBool native_socket_getReadsCompleted(JSContext *context,
		JSObject *object, uintN argc, jsval *argv, jsval *rval);

JSBool native_socket_getWritesAttempted(JSContext *context,
		JSObject *object, uintN argc, jsval *argv, jsval *rval);

JSBool native_socket_getWritesCompleted(JSContext *context,
		JSObject *object, uintN argc, jsval *argv, jsval *rval);

JSBool native_socket_getTotalReadTime(JSContext *context,
		JSObject *object, uintN argc, jsval *argv, jsval *rval);

JSBool native_socket_getTotalWriteTime(JSContext *context,
		JSObject *object, uintN argc, jsval *argv, jsval *rval);

JSBool native_socket_getSocketDescriptor(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval);

JSBool native_socket_open(JSContext *context, JSObject *object, uintN argc,
		jsval *argv, jsval *rval);

JSBool native_socket_close(JSContext *context, JSObject *object, uintN argc,
		jsval *argv, jsval *rval);

JSBool native_socket_closeChildSocket(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval);

JSBool native_socket_requestShutdown(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval);

JSBool native_socket_listen(JSContext *context, JSObject *object, uintN argc,
		jsval *argv, jsval *rval);

JSBool native_socket_accept(JSContext *context, JSObject *object, uintN argc,
		jsval *argv, jsval *rval);

JSBool native_socket_getPeerName(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval);

JSBool native_socket_send(JSContext *context, JSObject *object, uintN argc,
		jsval *argv, jsval *rval);

JSBool native_socket_receive(JSContext *context, JSObject *object, uintN argc,
		jsval *argv, jsval *rval);

// thread functions

JSBool native_thread_new(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval);

JSBool native_thread_create(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval);

JSBool native_thread_join(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval);

JSBool native_thread_self(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval);

// language native interface functions

JSBool native_getOsType(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval);

JSBool native_getConstant(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval);

JSBool native_getVariable(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval);

JSBool native_setVariable(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval);

// language extension functions

JSBool native_include(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval);

JSBool native_print(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval);

JSBool native_rand(JSContext *context, JSObject *object,
		uintN argc, jsval *argv, jsval *rval);

// engine interface functions

ScriptEngine *native_getScriptEngineContext();

void native_setScriptEngineContext(ScriptEngine *scriptEngine);

aboolean native_registerConstants();

aboolean native_registerFunctions();

aboolean native_registerFunctionsDirect(SpiderMonkey *spiderMonkey);


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _SCRIPT_FUNCTIONS_H

