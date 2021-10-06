/*
 * types.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The SpiderMonkey JavaScript engine for Asgard, native types function
 * library, header file.
 *
 * Written by Josh English.
 */

#if !defined(_SCRIPT_TYPES_H)

#define _SCRIPT_TYPES_H

#if !defined(_SCRIPT_H) && !defined(_SCRIPT_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _SCRIPT_H || _SCRIPT_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// define types public constants

typedef enum _ScriptNativeType {
	SCRIPT_NATIVE_TYPE_DL = 1,
	SCRIPT_NATIVE_TYPE_MUTEX,
	SCRIPT_NATIVE_TYPE_SIGNAL,
	SCRIPT_NATIVE_TYPE_SOCKET,
	SCRIPT_NATIVE_TYPE_THREAD,
	SCRIPT_NATIVE_TYPE_UNKNOWN = -1,
} ScriptNativeType;

typedef enum _ScriptNativeVarType {
	SCRIPT_NATIVE_VAR_TYPE_BOOLEAN = 1,
	SCRIPT_NATIVE_VAR_TYPE_INTEGER,
	SCRIPT_NATIVE_VAR_TYPE_DOUBLE,
	SCRIPT_NATIVE_VAR_TYPE_STRING,
	SCRIPT_NATIVE_VAR_TYPE_UNKNOWN = -1,
} ScriptNativeVarType;

typedef enum _NativeThreadState {
	NATIVE_THREAD_STATE_INIT = 1,
	NATIVE_THREAD_STATE_RUNNING,
	NATIVE_THREAD_STATE_HALT,
	NATIVE_THREAD_STATE_STOPPED,
	NATIVE_THREAD_STATE_ERROR = -1
} NativeThreadState;

typedef enum _NativeSocketMode {
	NATIVE_SOCKET_MODE_CLIENT = 1,
	NATIVE_SOCKET_MODE_SERVER,
	NATIVE_SOCKET_MODE_ERROR = -1
} NativeSocketMode;

typedef enum _NativeSocketProtocol {
	NATIVE_SOCKET_PROTOCOL_TCPIP = 1,
	NATIVE_SOCKET_PROTOCOL_UDP,
	NATIVE_SOCKET_PROTOCOL_ERROR = -1
} NativeSocketProtocol;



// define types public data types

typedef struct _ScriptNativeVar {
	ScriptNativeVarType type;
	char *name;
	void *value;
} ScriptNativeVar;

typedef struct _ScriptNativeSignal {
	int signalType;
	char *functionName;
} ScriptNativeSignal;

typedef struct _ScriptNativeThread {
	NativeThreadState state;
	char *functionName;
	Thread thread;
	ScriptEngineContext *context;
} ScriptNativeThread;

typedef struct _ScriptNative {
	int mutexLength;
	int signalLength;
	int threadLength;
	int socketLength;
	Mutex *mutexes;
	ScriptNativeSignal *signalFunctions;
	ScriptNativeThread *threads;
	Socket *sockets;
	Bptree varIndex;
	Bptree constIndex;
	Mutex mutex;
} ScriptNative;


// declare types public functions

// general functions

void scriptTypes_init(ScriptNative *scriptNative);

void scriptTypes_initFromEngine(ScriptEngine *scriptEngine);

void scriptTypes_free(ScriptNative *scriptNative);

// helper functions

double scriptTypes_encodeNativeId(ScriptNativeType type, int id);

aboolean scriptTypes_decodeNativeId(double nativeId, ScriptNativeType *type,
		int *id);

// native variable functions

aboolean scriptTypes_getBooleanValueOfNativeVar(ScriptNativeVar *variable);

int scriptTypes_getIntegerValueOfNativeVar(ScriptNativeVar *variable);

double scriptTypes_getDoubleValueOfNativeVar(ScriptNativeVar *variable);

char *scriptTypes_getStringValueOfNativeVar(ScriptNativeVar *variable);

// constants functions

aboolean scriptTypes_registerConstant(ScriptEngine *scriptEngine,
		ScriptNativeVarType type, char *name, void *value);

aboolean scriptTypes_registerBooleanConstant(ScriptEngine *scriptEngine,
		char *name, aboolean value);

aboolean scriptTypes_registerIntegerConstant(ScriptEngine *scriptEngine,
		char *name, int value);

aboolean scriptTypes_registerDoubleConstant(ScriptEngine *scriptEngine,
		char *name, double value);

aboolean scriptTypes_registerStringConstant(ScriptEngine *scriptEngine,
		char *name, char *value);

ScriptNativeVar *scriptTypes_getConstant(ScriptEngine *scriptEngine,
		char *name);

// variables functions

aboolean scriptTypes_registerVariable(ScriptEngine *scriptEngine,
		ScriptNativeVarType type, char *nameSpace, char *name, void *value);

aboolean scriptTypes_registerBooleanVariable(ScriptEngine *scriptEngine,
		char *nameSpace, char *name, aboolean value);

aboolean scriptTypes_registerIntegerVariable(ScriptEngine *scriptEngine,
		char *nameSpace, char *name, int value);

aboolean scriptTypes_registerDoubleVariable(ScriptEngine *scriptEngine,
		char *nameSpace, char *name, double value);

aboolean scriptTypes_registerStringVariable(ScriptEngine *scriptEngine,
		char *nameSpace, char *name, char *value);

ScriptNativeVar *scriptTypes_getVariable(ScriptEngine *scriptEngine,
		char *nameSpace, char *name);

// mutex functions

int scriptTypes_mutexNew(ScriptNative *scriptNative);

aboolean scriptTypes_mutexLock(ScriptNative *scriptNative, int id);

aboolean scriptTypes_mutexUnlock(ScriptNative *scriptNative, int id);

aboolean scriptTypes_mutexFree(ScriptNative *scriptNative, int id);

// signal functions

int scryptTypes_signalRegisterAction(ScriptNative *scriptNative,
		int signalType, char *functionName);

// socket functions

int scriptTypes_socketNew(ScriptEngine *scriptEngine, int mode, int protocol,
		char *hostname, int port);

Socket *scriptTypes_socketGet(ScriptNative *scriptNative, int id);

// thread functions

int scriptTypes_threadNew(ScriptEngine *scriptEngine, char *functionName);

ScriptNativeThread *scriptTypes_threadGet(ScriptNative *scriptNative, int id);


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _SCRIPT_TYPES_H

