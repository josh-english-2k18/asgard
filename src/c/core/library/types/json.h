/*
 * json.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard core library JSON type library, header file.
 *
 * Written by Josh English.
 */

#if !defined(_CORE_LIBRARY_TYPES_JSON_H)

#define _CORE_LIBRARY_TYPES_JSON_H

#if !defined(_CORE_H) && !defined(_CORE_LIBRARY_H) && \
		!defined(_CORE_LIBRARY_TYPES_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _CORE_H || _CORE_LIBRARY_H || _CORE_LIBRARY_TYPES_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// define json public constants

typedef enum _JsonValueType {
	JSON_VALUE_TYPE_BOOLEAN = 1,
	JSON_VALUE_TYPE_NUMBER,
	JSON_VALUE_TYPE_STRING,
	JSON_VALUE_TYPE_ARRAY,
	JSON_VALUE_TYPE_OBJECT,
	JSON_VALUE_TYPE_NULL,
	JSON_VALUE_TYPE_UNKNOWN = -1
} JsonValueType;


// declare json public data types

typedef struct _JsonElement {
	int type;
	int nameLength;
	char *name;
	void *value;
} JsonElement;

typedef struct _JsonArray {
	int length;
	int nameLength;
	int *types;
	char *name;
	void *values;
} JsonArray;

typedef struct _Json {
	int elementLength;
	JsonElement *elements;
} Json;


// declare json public functions

int json_init(Json *object);

Json *json_new();

Json *json_newFromString(char *string);

int json_free(Json *object);

int json_freePtr(Json *object);

char *json_valueTypeToString(JsonValueType type);

char *json_toString(Json *object, int *stringLength);

int json_addBoolean(Json *object, char *name, aboolean value);

int json_addNumber(Json *object, char *name, double value);

int json_addString(Json *object, char *name, char *value);

int json_addArray(Json *object, char *name);

int json_addObject(Json *object, char *name, Json *value);

int json_addBooleanToArray(Json *object, char *name, aboolean value);

int json_addNumberToArray(Json *object, char *name, double value);

int json_addStringToArray(Json *object, char *name, char *value);

int json_addArrayToArray(Json *object, char *name);

int json_addObjectToArray(Json *object, char *name, Json *value);

aboolean json_elementExists(Json *object, char *name);

JsonValueType json_getElementType(Json *object, char *name);

aboolean json_getBoolean(Json *object, char *name);

double json_getNumber(Json *object, char *name);

char *json_getString(Json *object, char *name);

Json *json_getObject(Json *object, char *name);

aboolean json_getBooleanFromArray(Json *object, char *name, int index);

double json_getNumberFromArray(Json *object, char *name, int index);

char *json_getStringFromArray(Json *object, char *name, int index);

Json *json_getObjectFromArray(Json *object, char *name, int index);

int json_getArrayLength(Json *object, char *name);

// helper functions

int json_minimizeJsonString(char *string, int stringLength,
		aboolean isNetworkOptimized);


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _CORE_LIBRARY_TYPES_JSON_H

