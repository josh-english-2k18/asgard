/*
 * types.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The SpiderMonkey JavaScript engine for Asgard, native types function
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
#include "script/types.h"


// declare types private functions

static void freeNativeVar(void *memory);

static char *nativeVarTypeToString(ScriptNativeVarType type);

static aboolean isValidNativeType(int type);

static char *buildVariableIndexKey(char *nameSpace, int nameSpaceLength,
		char *name, int nameLength, int *resultLength);


// define types private functions

static void freeNativeVar(void *memory)
{
	ScriptNativeVar *variable = NULL;

	if(memory == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	variable = (ScriptNativeVar *)memory;

	if(variable->name != NULL) {
		free(variable->name);
	}

	if(variable->value != NULL) {
		free(variable->value);
	}
}

static char *nativeVarTypeToString(ScriptNativeVarType type)
{
	char *result = NULL;

	switch(type) {
		case SCRIPT_NATIVE_VAR_TYPE_BOOLEAN:
			result = "boolean";
			break;

		case SCRIPT_NATIVE_VAR_TYPE_INTEGER:
			result = "integer";
			break;

		case SCRIPT_NATIVE_VAR_TYPE_DOUBLE:
			result = "double";
			break;

		case SCRIPT_NATIVE_VAR_TYPE_STRING:
			result = "string";
			break;

		case SCRIPT_NATIVE_VAR_TYPE_UNKNOWN:
		default:
			result = "unknown";
	}

	return result;
}

static aboolean isValidNativeType(int type)
{
	aboolean result = afalse;

	switch((ScriptNativeType)type) {
		case SCRIPT_NATIVE_TYPE_DL:
		case SCRIPT_NATIVE_TYPE_MUTEX:
		case SCRIPT_NATIVE_TYPE_SIGNAL:
		case SCRIPT_NATIVE_TYPE_SOCKET:
		case SCRIPT_NATIVE_TYPE_THREAD:
			result = atrue;
			break;

		default:
			result = afalse;
	}

	return result;
}

static char *buildVariableIndexKey(char *nameSpace, int nameSpaceLength,
		char *name, int nameLength, int *resultLength)
{
	int length = 0;
	char *result = NULL;

	length = (nameSpaceLength + nameLength + 8);

	result = (char *)malloc(sizeof(char) * length);

	*resultLength = snprintf(result, (length - 1), "i/%s/%s",
			nameSpace,
			name);

	return result;
}



// define types public functions

// general functions

void scriptTypes_init(ScriptNative *scriptNative)
{
	if(scriptNative == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	memset(scriptNative, 0, (int)(sizeof(ScriptNative)));

	scriptNative->mutexLength = 0;
	scriptNative->signalLength = 0;
	scriptNative->threadLength = 0;
	scriptNative->socketLength = 0;
	scriptNative->mutexes = NULL;
	scriptNative->signalFunctions = NULL;
	scriptNative->threads = NULL;
	scriptNative->sockets = NULL;

	bptree_init(&scriptNative->varIndex);
	bptree_setFreeFunction(&scriptNative->varIndex, freeNativeVar);

	bptree_init(&scriptNative->constIndex);
	bptree_setFreeFunction(&scriptNative->constIndex, freeNativeVar);

	mutex_init(&scriptNative->mutex);
}

void scriptTypes_initFromEngine(ScriptEngine *scriptEngine)
{
	ScriptNative *scriptNative = NULL;

	if((scriptEngine == NULL) ||
			(scriptEngine->status != SCRIPT_ENGINE_STATUS_RUNNING) ||
			(scriptEngine->native != NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	mutex_lock(&scriptEngine->mutex);

	scriptNative = (ScriptNative *)malloc(sizeof(ScriptNative));

	scriptTypes_init(scriptNative);

	scriptEngine->native = (void *)scriptNative;

	mutex_unlock(&scriptEngine->mutex);
}

void scriptTypes_free(ScriptNative *scriptNative)
{
	if(scriptNative == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	/*
	 * TODO: this function
	 */
}

// helper functions

double scriptTypes_encodeNativeId(ScriptNativeType type, int id)
{
	double result = 0.0;

	union {
		int iValue;
		char cValue[SIZEOF_INT];
	} IntU;

	union {
		double dValue;
		char cValue[SIZEOF_DOUBLE];
	} DoubleU;

	IntU.iValue = (int)type;

	memcpy(DoubleU.cValue, IntU.cValue, SIZEOF_INT);

	IntU.iValue = id;

	memcpy((DoubleU.cValue + SIZEOF_INT), IntU.cValue, SIZEOF_INT);

	result = DoubleU.dValue;

	return result;
}

aboolean scriptTypes_decodeNativeId(double nativeId, ScriptNativeType *type,
		int *id)
{
	union {
		int iValue;
		char cValue[SIZEOF_INT];
	} IntU;

	union {
		double dValue;
		char cValue[SIZEOF_DOUBLE];
	} DoubleU;

	DoubleU.dValue = nativeId;

	memcpy(IntU.cValue, DoubleU.cValue, SIZEOF_INT);

	*type = (ScriptNativeType)IntU.iValue;

	if(!isValidNativeType(*type)) {
		return afalse;
	}

	memcpy(IntU.cValue, (DoubleU.cValue + SIZEOF_INT), SIZEOF_INT);

	*id = IntU.iValue;

	if(*id < 0) {
		return afalse;
	}

	return atrue;
}

// native variable functions

aboolean scriptTypes_getBooleanValueOfNativeVar(ScriptNativeVar *variable)
{
	aboolean result = afalse;

	if(variable->type != SCRIPT_NATIVE_VAR_TYPE_BOOLEAN) {
		return afalse;
	}

	result = (aboolean)(((char *)variable->value)[0]);

	return result;
}

int scriptTypes_getIntegerValueOfNativeVar(ScriptNativeVar *variable)
{
	int result = 0;

	if(variable->type != SCRIPT_NATIVE_VAR_TYPE_INTEGER) {
		return 0;
	}

	if(serialize_decodeInt(variable->value, SIZEOF_INT, &result) < 0) {
		return 0;
	}

	return result;
}

double scriptTypes_getDoubleValueOfNativeVar(ScriptNativeVar *variable)
{
	double result = 0.0;

	if(variable->type != SCRIPT_NATIVE_VAR_TYPE_DOUBLE) {
		return 0.0;
	}

	if(serialize_decodeDouble(variable->value, SIZEOF_DOUBLE, &result) < 0) {
		return 0.0;
	}

	return result;
}

char *scriptTypes_getStringValueOfNativeVar(ScriptNativeVar *variable)
{
	char *result = NULL;

	if(variable->type != SCRIPT_NATIVE_VAR_TYPE_STRING) {
		return NULL;
	}

	result = (char *)variable->value;

	return result;
}

// constants functions

aboolean scriptTypes_registerConstant(ScriptEngine *scriptEngine,
		ScriptNativeVarType type, char *name, void *value)
{
	aboolean result = atrue;
	int nameLength = 0;

	ScriptNative *scriptNative = NULL;
	ScriptNativeVar *constant = NULL;

	if((scriptEngine == NULL) ||
			(scriptEngine->status != SCRIPT_ENGINE_STATUS_RUNNING) ||
			(scriptEngine->native == NULL) ||
			((type != SCRIPT_NATIVE_VAR_TYPE_BOOLEAN) &&
			 (type != SCRIPT_NATIVE_VAR_TYPE_INTEGER) &&
			 (type != SCRIPT_NATIVE_VAR_TYPE_DOUBLE) &&
			 (type != SCRIPT_NATIVE_VAR_TYPE_STRING)) ||
			(name == NULL) || ((nameLength = strlen(name)) <= 0) ||
			(value == NULL)) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	mutex_lock(&scriptEngine->mutex);

	scriptNative = (ScriptNative *)scriptEngine->native;

	mutex_unlock(&scriptEngine->mutex);
	mutex_lock(&scriptNative->mutex);

	if(bptree_get(&scriptNative->constIndex, name, nameLength,
				((void *)&constant)) == 0) {
		log_logf(scriptEngine->log, LOG_LEVEL_ERROR,
				"unable to register constant '%s' type '%s', already exists",
				name, nativeVarTypeToString(type));
		result = afalse;
	}
	else {
		constant = (ScriptNativeVar *)malloc(sizeof(ScriptNativeVar));

		constant->type = type;
		constant->name = strdup(name);
		constant->value = value;

		if(bptree_put(&scriptNative->constIndex, name, nameLength,
					(void *)constant) < 0) {
			log_logf(scriptEngine->log, LOG_LEVEL_ERROR,
					"failed to register constant '%s' type '%s' in index",
					name, nativeVarTypeToString(type));
			result = afalse;
			constant->value = NULL;
			freeNativeVar(constant);
		}
	}

	mutex_unlock(&scriptNative->mutex);

	return result;
}

aboolean scriptTypes_registerBooleanConstant(ScriptEngine *scriptEngine,
		char *name, aboolean value)
{
	aboolean result = atrue;
	char *serialValue = NULL;

	if((scriptEngine == NULL) ||
			(scriptEngine->status != SCRIPT_ENGINE_STATUS_RUNNING) ||
			(scriptEngine->native == NULL) || (name == NULL)) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	serialValue = (char *)malloc(sizeof(char) * SIZEOF_BOOLEAN);
	serialValue[0] = (char)value;

	if((result = scriptTypes_registerConstant(scriptEngine,
					SCRIPT_NATIVE_VAR_TYPE_BOOLEAN,
					name,
					serialValue)) == afalse) {
		free(serialValue);
	}

	return result;
}

aboolean scriptTypes_registerIntegerConstant(ScriptEngine *scriptEngine,
		char *name, int value)
{
	aboolean result = atrue;
	void *serialValue = NULL;

	if((scriptEngine == NULL) ||
			(scriptEngine->status != SCRIPT_ENGINE_STATUS_RUNNING) ||
			(scriptEngine->native == NULL) || (name == NULL)) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	serialValue = (void *)malloc(sizeof(char) * SIZEOF_INT);

	if(serialize_encodeInt(value, serialValue) < 0) {
		result = afalse;
	}
	else {
		if((result = scriptTypes_registerConstant(scriptEngine,
						SCRIPT_NATIVE_VAR_TYPE_INTEGER,
						name,
						serialValue)) == afalse) {
			free(serialValue);
		}
	}

	return result;
}

aboolean scriptTypes_registerDoubleConstant(ScriptEngine *scriptEngine,
		char *name, double value)
{
	aboolean result = atrue;
	void *serialValue = NULL;

	if((scriptEngine == NULL) ||
			(scriptEngine->status != SCRIPT_ENGINE_STATUS_RUNNING) ||
			(scriptEngine->native == NULL) || (name == NULL)) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	serialValue = (void *)malloc(sizeof(char) * SIZEOF_DOUBLE);

	if(serialize_encodeDouble(value, serialValue) < 0) {
		result = afalse;
	}
	else {
		if((result = scriptTypes_registerConstant(scriptEngine,
						SCRIPT_NATIVE_VAR_TYPE_DOUBLE,
						name,
						serialValue)) == afalse) {
			free(serialValue);
		}
	}

	return result;
}

aboolean scriptTypes_registerStringConstant(ScriptEngine *scriptEngine,
		char *name, char *value)
{
	aboolean result = atrue;
	int valueLength = 0;
	char *serialValue = NULL;

	if((scriptEngine == NULL) ||
			(scriptEngine->status != SCRIPT_ENGINE_STATUS_RUNNING) ||
			(scriptEngine->native == NULL) || (name == NULL) ||
			(value == NULL) || ((valueLength = strlen(value)) <= 0)) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	if((serialValue = strndup(value, valueLength)) == NULL) {
		result = afalse;
	}
	else {
		if((result = scriptTypes_registerConstant(scriptEngine,
						SCRIPT_NATIVE_VAR_TYPE_STRING,
						name,
						serialValue)) == afalse) {
				free(serialValue);
		}
	}

	return result;
}

ScriptNativeVar *scriptTypes_getConstant(ScriptEngine *scriptEngine,
		char *name)
{
	int nameLength = 0;

	ScriptNativeVar *result = NULL;
	ScriptNative *scriptNative = NULL;

	if((scriptEngine == NULL) ||
			(scriptEngine->status != SCRIPT_ENGINE_STATUS_RUNNING) ||
			(scriptEngine->native == NULL) ||
			(name == NULL) || ((nameLength = strlen(name)) <= 0)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	mutex_lock(&scriptEngine->mutex);

	scriptNative = (ScriptNative *)scriptEngine->native;

	mutex_unlock(&scriptEngine->mutex);
	mutex_lock(&scriptNative->mutex);

	if(bptree_get(&scriptNative->constIndex, name, nameLength,
				((void *)&result)) < 0) {
		log_logf(scriptEngine->log, LOG_LEVEL_ERROR,
				"unable to locate constant '%s' in index", name);
		mutex_unlock(&scriptNative->mutex);
		return NULL;
	}

	mutex_unlock(&scriptNative->mutex);

	return result;
}

// variables functions

aboolean scriptTypes_registerVariable(ScriptEngine *scriptEngine,
		ScriptNativeVarType type, char *nameSpace, char *name, void *value)
{
	aboolean result = atrue;
	int keyLength = 0;
	int nameLength = 0;
	int nameSpaceLength = 0;
	char *key = NULL;

	ScriptNative *scriptNative = NULL;
	ScriptNativeVar *variable = NULL;

	if((scriptEngine == NULL) ||
			(scriptEngine->status != SCRIPT_ENGINE_STATUS_RUNNING) ||
			(scriptEngine->native == NULL) ||
			((type != SCRIPT_NATIVE_VAR_TYPE_BOOLEAN) &&
			 (type != SCRIPT_NATIVE_VAR_TYPE_INTEGER) &&
			 (type != SCRIPT_NATIVE_VAR_TYPE_DOUBLE) &&
			 (type != SCRIPT_NATIVE_VAR_TYPE_STRING)) ||
			(nameSpace == NULL) ||
			((nameSpaceLength = strlen(nameSpace)) <= 0) ||
			(name == NULL) ||
			((nameLength = strlen(name)) <= 0) ||
			(value == NULL)) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	mutex_lock(&scriptEngine->mutex);

	scriptNative = (ScriptNative *)scriptEngine->native;

	mutex_unlock(&scriptEngine->mutex);
	mutex_lock(&scriptNative->mutex);

	key = buildVariableIndexKey(nameSpace, nameSpaceLength, name, nameLength,
			&keyLength);

	if(bptree_get(&scriptNative->varIndex, key, keyLength,
				((void *)&variable)) == 0) {
		variable->type = type;

		if(variable->name != NULL) {
			if(strcmp(variable->name, name)) {
				free(variable->name);
				variable->name = strdup(name);
			}
		}
		else {
			variable->name = strdup(name);
		}

		if(variable->value != NULL) {
			free(variable->value);
		}

		variable->value = value;
	}
	else {
		variable = (ScriptNativeVar *)malloc(sizeof(ScriptNativeVar));

		variable->type = type;
		variable->name = strdup(name);
		variable->value = value;

		if(bptree_put(&scriptNative->varIndex, key, keyLength,
					(void *)variable) < 0) {
			log_logf(scriptEngine->log, LOG_LEVEL_ERROR,
					"failed to register variable '%s' type '%s' in index",
					name, nativeVarTypeToString(type));
			result = afalse;
			variable->value = NULL;
			freeNativeVar(variable);
		}
	}

	mutex_unlock(&scriptNative->mutex);

	return result;
}

aboolean scriptTypes_registerBooleanVariable(ScriptEngine *scriptEngine,
		char *nameSpace, char *name, aboolean value)
{
	aboolean result = atrue;
	char *serialValue = NULL;

	if((scriptEngine == NULL) ||
			(scriptEngine->status != SCRIPT_ENGINE_STATUS_RUNNING) ||
			(scriptEngine->native == NULL) || (name == NULL) ||
			(nameSpace == NULL)) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	serialValue = (char *)malloc(sizeof(char) * SIZEOF_BOOLEAN);
	serialValue[0] = (char)value;

	if((result = scriptTypes_registerVariable(scriptEngine,
					SCRIPT_NATIVE_VAR_TYPE_BOOLEAN,
					nameSpace,
					name,
					serialValue)) == afalse) {
		free(serialValue);
	}

	return result;
}

aboolean scriptTypes_registerIntegerVariable(ScriptEngine *scriptEngine,
		char *nameSpace, char *name, int value)
{
	aboolean result = atrue;
	void *serialValue = NULL;

	if((scriptEngine == NULL) ||
			(scriptEngine->status != SCRIPT_ENGINE_STATUS_RUNNING) ||
			(scriptEngine->native == NULL) || (name == NULL) ||
			(nameSpace == NULL)) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	serialValue = (void *)malloc(sizeof(char) * SIZEOF_INT);

	if(serialize_encodeInt(value, serialValue) < 0) {
		result = afalse;
	}
	else {
		if((result = scriptTypes_registerVariable(scriptEngine,
						SCRIPT_NATIVE_VAR_TYPE_INTEGER,
						nameSpace,
						name,
						serialValue)) == afalse) {
			free(serialValue);
		}
	}

	return result;
}

aboolean scriptTypes_registerDoubleVariable(ScriptEngine *scriptEngine,
		char *nameSpace, char *name, double value)
{
	aboolean result = atrue;
	void *serialValue = NULL;

	if((scriptEngine == NULL) ||
			(scriptEngine->status != SCRIPT_ENGINE_STATUS_RUNNING) ||
			(scriptEngine->native == NULL) || (name == NULL) ||
			(nameSpace == NULL)) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	serialValue = (void *)malloc(sizeof(char) * SIZEOF_DOUBLE);

	if(serialize_encodeDouble(value, serialValue) < 0) {
		result = afalse;
	}
	else {
		if((result = scriptTypes_registerVariable(scriptEngine,
						SCRIPT_NATIVE_VAR_TYPE_DOUBLE,
						nameSpace,
						name,
						serialValue)) == afalse) {
			free(serialValue);
		}
	}

	return result;
}

aboolean scriptTypes_registerStringVariable(ScriptEngine *scriptEngine,
		char *nameSpace, char *name, char *value)
{
	aboolean result = atrue;
	int valueLength = 0;
	char *serialValue = NULL;

	if((scriptEngine == NULL) ||
			(scriptEngine->status != SCRIPT_ENGINE_STATUS_RUNNING) ||
			(scriptEngine->native == NULL) || (name == NULL) ||
			(nameSpace == NULL) || (value == NULL) ||
			((valueLength = strlen(value)) <= 0)) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	if((serialValue = strndup(value, valueLength)) == NULL) {
		result = afalse;
	}
	else {
		if((result = scriptTypes_registerVariable(scriptEngine,
						SCRIPT_NATIVE_VAR_TYPE_STRING,
						nameSpace,
						name,
						serialValue)) == afalse) {
				free(serialValue);
		}
	}

	return result;
}

ScriptNativeVar *scriptTypes_getVariable(ScriptEngine *scriptEngine,
		char *nameSpace, char *name)
{
	int keyLength = 0;
	int nameLength = 0;
	int nameSpaceLength = 0;
	char *key = NULL;

	ScriptNativeVar *result = NULL;
	ScriptNative *scriptNative = NULL;

	if((scriptEngine == NULL) ||
			(scriptEngine->status != SCRIPT_ENGINE_STATUS_RUNNING) ||
			(scriptEngine->native == NULL) ||
			(nameSpace == NULL) ||
			((nameSpaceLength = strlen(nameSpace)) <= 0) ||
			(name == NULL) ||
			((nameLength = strlen(name)) <= 0)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	mutex_lock(&scriptEngine->mutex);

	scriptNative = (ScriptNative *)scriptEngine->native;

	mutex_unlock(&scriptEngine->mutex);
	mutex_lock(&scriptNative->mutex);

	key = buildVariableIndexKey(nameSpace, nameSpaceLength, name, nameLength,
			&keyLength);

	if(bptree_get(&scriptNative->varIndex, key, keyLength,
				((void *)&result)) < 0) {
		log_logf(scriptEngine->log, LOG_LEVEL_ERROR,
				"unable to locate variable '%s' in index namespace '%s'",
				name, nameSpace);
		mutex_unlock(&scriptNative->mutex);
		return NULL;
	}

	mutex_unlock(&scriptNative->mutex);

	return result;
}

// mutex functions

int scriptTypes_mutexNew(ScriptNative *scriptNative)
{
	int id = 0;

	if(scriptNative == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&scriptNative->mutex);

	if(scriptNative->mutexes == NULL) {
		id = 0;
		scriptNative->mutexLength = 1;
		scriptNative->mutexes = (Mutex *)malloc(sizeof(Mutex) *
				scriptNative->mutexLength);
	}
	else {
		id = scriptNative->mutexLength;
		scriptNative->mutexLength += 1;
		scriptNative->mutexes = (Mutex *)realloc(scriptNative->mutexes,
				(sizeof(Mutex) * scriptNative->mutexLength));
	}

	if(mutex_init(&scriptNative->mutexes[id]) < 0) {
		mutex_unlock(&scriptNative->mutex);
		return -1;
	}

	mutex_unlock(&scriptNative->mutex);

	return id;
}

aboolean scriptTypes_mutexLock(ScriptNative *scriptNative, int id)
{
	if((scriptNative == NULL) || (scriptNative->mutexes == NULL) || (id < 0) ||
			(id >= scriptNative->mutexLength)) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	mutex_lock(&scriptNative->mutex);

	mutex_lock(&scriptNative->mutexes[id]);

	mutex_unlock(&scriptNative->mutex);

	return atrue;
}

aboolean scriptTypes_mutexUnlock(ScriptNative *scriptNative, int id)
{
	if((scriptNative == NULL) || (scriptNative->mutexes == NULL) || (id < 0) ||
			(id >= scriptNative->mutexLength)) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	mutex_lock(&scriptNative->mutex);

	mutex_unlock(&scriptNative->mutexes[id]);

	mutex_unlock(&scriptNative->mutex);

	return atrue;
}

aboolean scriptTypes_mutexFree(ScriptNative *scriptNative, int id)
{
	if((scriptNative == NULL) || (scriptNative->mutexes == NULL) || (id < 0) ||
			(id >= scriptNative->mutexLength)) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	mutex_lock(&scriptNative->mutex);

	mutex_free(&scriptNative->mutexes[id]);

	memset(&scriptNative->mutexes[id], 0, (int)(sizeof(Mutex)));

	mutex_unlock(&scriptNative->mutex);

	return atrue;
}

// signal functions

int scryptTypes_signalRegisterAction(ScriptNative *scriptNative,
		int signalType, char *functionName)
{
	int id = 0;

	if(scriptNative == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&scriptNative->mutex);

	if(scriptNative->signalFunctions == NULL) {
		id = 0;
		scriptNative->signalLength = 1;
		scriptNative->signalFunctions = (ScriptNativeSignal *)malloc(
				sizeof(ScriptNativeSignal) * scriptNative->signalLength);
	}
	else {
		id = scriptNative->signalLength;
		scriptNative->signalLength += 1;
		scriptNative->signalFunctions = (ScriptNativeSignal *)realloc(
				scriptNative->signalFunctions,
				(sizeof(ScriptNativeSignal) * scriptNative->signalLength));
	}

	scriptNative->signalFunctions[id].signalType = signalType;
	scriptNative->signalFunctions[id].functionName = strdup(functionName);

	mutex_unlock(&scriptNative->mutex);

	return id;
}

// socket functions

int scriptTypes_socketNew(ScriptEngine *scriptEngine, int mode, int protocol,
		char *hostname, int port)
{
	int id = 0;

	ScriptNative *scriptNative = NULL;

	if((scriptEngine == NULL) || (scriptEngine->native == NULL) ||
			((mode != NATIVE_SOCKET_MODE_CLIENT) &&
			 (mode != NATIVE_SOCKET_MODE_SERVER)) ||
			((protocol != NATIVE_SOCKET_PROTOCOL_TCPIP) &&
			 (protocol != NATIVE_SOCKET_PROTOCOL_UDP)) ||
			(hostname == NULL) || (port < 0) || (port > 65536)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	scriptNative = (ScriptNative *)scriptEngine->native;

	mutex_lock(&scriptNative->mutex);

	if(scriptNative->sockets == NULL) {
		id = 0;
		scriptNative->socketLength = 1;
		scriptNative->sockets = (Socket *)malloc(
				sizeof(Socket) * scriptNative->socketLength);
	}
	else {
		id = scriptNative->socketLength;
		scriptNative->socketLength += 1;
		scriptNative->sockets = (Socket *)realloc(
				scriptNative->sockets,
				(sizeof(Socket) * scriptNative->socketLength));
	}

	if(socket_init(&scriptNative->sockets[id], (SocketMode)mode,
				(SocketProtocol)protocol, hostname, port) < 0) {
		mutex_unlock(&scriptNative->mutex);
		return -1;
	}

	mutex_unlock(&scriptNative->mutex);

	return id;
}

Socket *scriptTypes_socketGet(ScriptNative *scriptNative, int id)
{
	Socket *result = NULL;

	if((scriptNative == NULL) || (scriptNative->sockets == NULL) || (id < 0) ||
			(id >= scriptNative->socketLength)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	mutex_lock(&scriptNative->mutex);

	result = &(scriptNative->sockets[id]);

	mutex_unlock(&scriptNative->mutex);

	return result;
}

// thread functions

int scriptTypes_threadNew(ScriptEngine *scriptEngine, char *functionName)
{
	int id = 0;

	ScriptNative *scriptNative = NULL;

	if((scriptEngine == NULL) || (scriptEngine->native == NULL) ||
			(functionName == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	scriptNative = (ScriptNative *)scriptEngine->native;

	mutex_lock(&scriptNative->mutex);

	if(scriptNative->threads == NULL) {
		id = 0;
		scriptNative->threadLength = 1;
		scriptNative->threads = (ScriptNativeThread *)malloc(
				sizeof(ScriptNativeThread) * scriptNative->threadLength);
	}
	else {
		id = scriptNative->threadLength;
		scriptNative->threadLength += 1;
		scriptNative->threads = (ScriptNativeThread *)realloc(
				scriptNative->threads,
				(sizeof(ScriptNativeThread) * scriptNative->threadLength));
	}

	scriptNative->threads[id].state = NATIVE_THREAD_STATE_INIT;
	scriptNative->threads[id].functionName = strdup(functionName);
	scriptNative->threads[id].context = scriptCommon_getContext(
			scriptEngine);

	thread_init(&scriptNative->threads[id].thread);

	mutex_unlock(&scriptNative->mutex);

	return id;
}

ScriptNativeThread *scriptTypes_threadGet(ScriptNative *scriptNative, int id)
{
	ScriptNativeThread *result = NULL;

	if((scriptNative == NULL) || (scriptNative->threads == NULL) || (id < 0) ||
			(id >= scriptNative->threadLength)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	mutex_lock(&scriptNative->mutex);

	result = &(scriptNative->threads[id]);

	mutex_unlock(&scriptNative->mutex);

	return result;
}

