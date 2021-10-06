/*
 * json.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard core library JSON type library.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#define _CORE_LIBRARY_TYPES_COMPONENT
#include "core/library/types/json.h"


// delcare json private functions

// generic functions

static char *buildStringIndent(int indentLevel, int *indentLength);

// element functions

static JsonElement *element_search(Json *object, char *name);

static int element_add(Json *object, int type, char *name, void *value);

static int element_addElement(Json *object, JsonElement *element);

static void element_freeValue(int type, void *value);

static void element_free(JsonElement *element);

static void element_freePtr(JsonElement *element);

static char *element_toString(JsonElement *element, char *string,
		int *stringRef, int *stringLength, int indentLevel,
		aboolean isLastElement);

// array functions

static JsonArray *array_getFromElement(Json *object, char *name, int index);

static int array_add(JsonArray *array, int type, void *value);

static char *array_toString(JsonArray *array, char *string,
		int *stringRef, int *stringLength, int indentLevel);

// object functions

static char *object_toString(Json *object, int *stringLength, int indentLevel);

// parsing functions

static void parse_consume(char **ptr, int *consumedBytes, int bytes);

static double parse_number(char *string, int *consumedBytes,
		aboolean *hasError);

static char *parse_string(char *string, int *consumedBytes);

static JsonElement *parse_value(char *string, int *consumedBytes,
		aboolean *hasError);

static Json *parse_json(char *string, int stringLength, int *consumedBytes);


// define json private functions

// generic functions

static char *buildStringIndent(int indentLevel, int *indentLength)
{
	int ii = 0;
	char *result = NULL;

	*indentLength = (indentLevel * 4);

	result = (char *)malloc(sizeof(char) * (*indentLength + 1));

	for(ii = 0; ii < (*indentLength); ii++) {
		result[ii] = ' ';
	}

	return result;
}

// element functions

static JsonElement *element_search(Json *object, char *name)
{
	int ii = 0;

	JsonElement *result = NULL;

	for(ii = 0; ii < object->elementLength; ii++) {
		if(!strcmp(name, object->elements[ii].name)) {
			result = &(object->elements[ii]);
			break;
		}
	}

	return result;
}

static int element_add(Json *object, int type, char *name, void *value)
{
	int ref = 0;
	int nameLength = 0;
	void *localValue = NULL;

	nameLength = strlen(name);
	if(nameLength < 1) {
		return -1;
	}

	if(element_search(object, name) != NULL) {
		return -1;
	}

	switch(type) {
		case JSON_VALUE_TYPE_BOOLEAN:
			localValue = (void *)malloc(sizeof(aboolean));
			memcpy(localValue, value, SIZEOF_BOOLEAN);
			break;

		case JSON_VALUE_TYPE_NUMBER:
			localValue = (void *)malloc(sizeof(double));
			memcpy(localValue, value, sizeof(double));
			break;

		case JSON_VALUE_TYPE_STRING:
			localValue = strdup((char *)value);
			break;

		case JSON_VALUE_TYPE_ARRAY:
			localValue = (void *)malloc(sizeof(JsonArray));
			((JsonArray *)localValue)->length = 0;
			((JsonArray *)localValue)->nameLength = strlen(name);
			((JsonArray *)localValue)->types = NULL;
			((JsonArray *)localValue)->name = strdup(name);
			((JsonArray *)localValue)->values = NULL;
			break;

		case JSON_VALUE_TYPE_OBJECT:
			localValue = (void *)value;
			break;

		case JSON_VALUE_TYPE_NULL:
			localValue = NULL;
			break;

		default:
			return -1;
	}

	if(object->elements == NULL) {
		ref = 0;
		object->elementLength = 1;
		object->elements = (JsonElement *)malloc(sizeof(JsonElement) *
				object->elementLength);
	}
	else {
		ref = object->elementLength;
		object->elementLength += 1;
		object->elements = (JsonElement *)realloc(object->elements,
				(sizeof(JsonElement) * object->elementLength));
	}

	object->elements[ref].type = type;
	object->elements[ref].nameLength = nameLength;
	object->elements[ref].name = strdup(name);
	object->elements[ref].value = localValue;

	return 0;
}

static int element_addElement(Json *object, JsonElement *element)
{
	int ref = 0;

	if(element_search(object, element->name) != NULL) {
		return -1;
	}

	if(object->elements == NULL) {
		ref = 0;
		object->elementLength = 1;
		object->elements = (JsonElement *)malloc(sizeof(JsonElement) *
				object->elementLength);
	}
	else {
		ref = object->elementLength;
		object->elementLength += 1;
		object->elements = (JsonElement *)realloc(object->elements,
				(sizeof(JsonElement) * object->elementLength));
	}

	memcpy(&(object->elements[ref]), element, sizeof(JsonElement));

	free(element);

	return 0;
}

static void element_freeValue(int type, void *value)
{
	int ii = 0;

	switch(type) {
		case JSON_VALUE_TYPE_BOOLEAN:
		case JSON_VALUE_TYPE_NUMBER:
		case JSON_VALUE_TYPE_STRING:
			if(value != NULL) {
				free(value);
			}
			break;

		case JSON_VALUE_TYPE_ARRAY:
			if(value != NULL) {
				if(((JsonArray *)value)->name != NULL) {
					free(((JsonArray *)value)->name);
				}
				if(((JsonArray *)value)->values != NULL) {
					for(ii = 0; ii < ((JsonArray *)value)->length; ii++) {
						element_freeValue(((JsonArray *)value)->types[ii],
								((void **)((JsonArray *)value)->values)[ii]);
					}
					free(((JsonArray *)value)->values);
				}
				if(((JsonArray *)value)->types != NULL) {
					free(((JsonArray *)value)->types);
				}
				free(value);
			}
			break;

		case JSON_VALUE_TYPE_OBJECT:
			json_freePtr(value);
			break;

		default:
			return;
	}
}

static void element_free(JsonElement *element)
{
	if(element->value != NULL) {
		element_freeValue(element->type, element->value);
	}

	if(element->name != NULL) {
		free(element->name);
	}

	memset(element, 0, (sizeof(JsonElement)));
}

static void element_freePtr(JsonElement *element)
{
	element_free(element);

	free(element);
}

static char *element_toString(JsonElement *element, char *string,
		int *stringRef, int *stringLength, int indentLevel,
		aboolean isLastElement)
{
	int valueLength = 0;
	int indentLength = 0;
	int bufferLength = 0;
	double dValue = 0.0;
	char *sValue = NULL;
	char *indent = NULL;
	char *buffer = NULL;
	char numberBuffer[1024];

	indent = buildStringIndent(indentLevel, &indentLength);

	string = strxpndcat(string, stringRef, stringLength, indent,
			indentLength);

	free(indent);

	string = strxpndcat(string, stringRef, stringLength, "\"",
			strlen("\""));
	string = strxpndcat(string, stringRef, stringLength, element->name,
			element->nameLength);
	string = strxpndcat(string, stringRef, stringLength, "\": ",
			strlen("\": "));

	switch(element->type) {
		case JSON_VALUE_TYPE_BOOLEAN:
			if(*((aboolean *)element->value)) {
				string = strxpndcat(string, stringRef, stringLength,
						"true", strlen("true"));
			}
			else {
				string = strxpndcat(string, stringRef, stringLength,
						"false", strlen("false"));
			}
			break;

		case JSON_VALUE_TYPE_NUMBER:
			dValue = (*((double *)element->value));

			if((double)((int)dValue) == dValue) {
				bufferLength = snprintf(numberBuffer,
						(sizeof(numberBuffer) - 1), "%i", (int)dValue);
			}
			else if((double)((alint)dValue) == dValue) {
				bufferLength = snprintf(numberBuffer,
						(sizeof(numberBuffer) - 1), "%lli", (alint)dValue);
			}
			else {
				bufferLength = snprintf(numberBuffer,
						(sizeof(numberBuffer) - 1), "%f", dValue);
			}

			string = strxpndcat(string, stringRef, stringLength, numberBuffer,
					bufferLength);
			break;

		case JSON_VALUE_TYPE_STRING:
			sValue = ((char *)element->value);

			if((valueLength = strlen(sValue)) > 0) {
				string = strxpndcat(string, stringRef, stringLength,
						"\"", strlen("\""));

				string = strxpndcat(string, stringRef, stringLength,
						sValue, valueLength);

				if(sValue[(valueLength - 1)] == '\\') {
					string = strxpndcat(string, stringRef, stringLength,
							"\\", strlen("\\"));
				}

				string = strxpndcat(string, stringRef, stringLength,
						"\"", strlen("\""));
			}
			else {
				string = strxpndcat(string, stringRef, stringLength,
						"\"\"", strlen("\"\""));
			}
			break;

		case JSON_VALUE_TYPE_ARRAY:
			string = array_toString(((JsonArray *)element->value), string,
					stringRef, stringLength, indentLevel);
			break;

		case JSON_VALUE_TYPE_OBJECT:
			buffer = object_toString((Json *)element->value, &bufferLength,
					indentLevel);
			string = strxpndcat(string, stringRef, stringLength, buffer,
					bufferLength);
			free(buffer);
			break;
	}

	if(!isLastElement) {
		string = strxpndcat(string, stringRef, stringLength, ",\n",
				strlen(",\n"));
	}
	else {
		string = strxpndcat(string, stringRef, stringLength, "\n",
				strlen("\n"));
	}

	return string;
}

// array functions

static JsonArray *array_getFromElement(Json *object, char *name, int index)
{
	JsonArray *result = NULL;
	JsonElement *element = NULL;

	if((element = element_search(object, name)) == NULL) {
		return NULL;
	}

	if(element->type != JSON_VALUE_TYPE_ARRAY) {
		return NULL;
	}

	result = (JsonArray *)element->value;

	if((index < 0) || (index >= result->length)) {
		return NULL;
	}

	return result;
}

static int array_add(JsonArray *array, int type, void *value)
{
	int ref = 0;
	void *localValue = NULL;

	switch(type) {
		case JSON_VALUE_TYPE_BOOLEAN:
			localValue = (void *)malloc(sizeof(aboolean));
			memcpy(localValue, value, SIZEOF_BOOLEAN);
			break;

		case JSON_VALUE_TYPE_NUMBER:
			localValue = (void *)malloc(sizeof(double));
			memcpy(localValue, value, sizeof(double));
			break;

		case JSON_VALUE_TYPE_STRING:
			localValue = strdup((char *)value);
			break;

		case JSON_VALUE_TYPE_ARRAY:
			localValue = (void *)malloc(sizeof(JsonArray));
			((JsonArray *)localValue)->length = 0;
			((JsonArray *)localValue)->types = NULL;
			((JsonArray *)localValue)->values = NULL;
			break;

		case JSON_VALUE_TYPE_OBJECT:
			localValue = (void *)value;
			break;

		default:
			return -1;
	}

	if(array->values == NULL) {
		ref = 0;
		array->length = 1;
		array->types = (int *)malloc(sizeof(int) * array->length);
		array->values = (void *)malloc(sizeof(void *) * array->length);
	}
	else {
		ref = array->length;
		array->length += 1;
		array->types = (int *)realloc(array->types,
				(sizeof(int) * array->length));
		array->values = (void *)realloc(array->values,
				(sizeof(void *) * array->length));
	}

	array->types[ref] = type;
	((void **)array->values)[ref] = localValue;

	return 0;
}

static char *array_toString(JsonArray *array, char *string,
		int *stringRef, int *stringLength, int indentLevel)
{
	int ii = 0;
	int valueLength = 0;
	int indentLength = 0;
	int bufferLength = 0;
	double dValue = 0.0;
	char *sValue = NULL;
	char *indent = NULL;
	char *buffer = NULL;
	char numberBuffer[1024];

	if((array->length < 0) || (array->types == NULL) ||
			(array->values == NULL)) {
		return string;
	}

	indent = buildStringIndent(indentLevel, &indentLength);

	string = strxpndcat(string, stringRef, stringLength, "[\n",
			strlen("[\n"));

	free(indent);
	indent = buildStringIndent((indentLevel + 1), &indentLength);

	for(ii = 0; ii < array->length; ii++) {
		string = strxpndcat(string, stringRef, stringLength, indent,
				indentLength);

		switch(array->types[ii]) {
			case JSON_VALUE_TYPE_BOOLEAN:
				if(*((aboolean *)((void **)array->values)[ii])) {
					string = strxpndcat(string, stringRef, stringLength,
							"true", strlen("true"));
				}
				else {
					string = strxpndcat(string, stringRef, stringLength,
							"false", strlen("false"));
				}
				break;

			case JSON_VALUE_TYPE_NUMBER:
				dValue = (*((double *)((void **)array->values)[ii]));

				if((double)((int)dValue) == dValue) {
					bufferLength = snprintf(numberBuffer,
							(sizeof(numberBuffer) - 1), "%i", (int)dValue);
				}
				else if((double)((alint)dValue) == dValue) {
					bufferLength = snprintf(numberBuffer,
							(sizeof(numberBuffer) - 1), "%lli", (alint)dValue);
				}
				else {
					bufferLength = snprintf(numberBuffer,
							(sizeof(numberBuffer) - 1), "%f", dValue);
				}

				string = strxpndcat(string, stringRef, stringLength,
						numberBuffer, bufferLength);
				break;

			case JSON_VALUE_TYPE_STRING:
				sValue = ((char *)((void **)array->values)[ii]);

				if((valueLength = strlen(sValue)) > 0) {
					string = strxpndcat(string, stringRef, stringLength,
							"\"", strlen("\""));

					string = strxpndcat(string, stringRef, stringLength,
							sValue, valueLength);

					if(sValue[(valueLength - 1)] == '\\') {
						string = strxpndcat(string, stringRef, stringLength,
								"\\", strlen("\\"));
					}

					string = strxpndcat(string, stringRef, stringLength,
							"\"", strlen("\""));
				}
				else {
					string = strxpndcat(string, stringRef, stringLength,
							"\"\"", strlen("\"\""));
				}
				break;

			case JSON_VALUE_TYPE_ARRAY:
				if((array->name != NULL) && (array->nameLength >= 0)) {
					string = strxpndcat(string, stringRef, stringLength, "\"",
							strlen("\""));
					string = strxpndcat(string, stringRef, stringLength,
							array->name, array->nameLength);
					string = strxpndcat(string, stringRef, stringLength,
							"\": ", strlen("\": "));
				}

				string = array_toString(
						((JsonArray *)((void **)array->values)[ii]),
						string, stringRef, stringLength, indentLevel);
				break;

			case JSON_VALUE_TYPE_OBJECT:
				buffer = object_toString(((Json *)((void **)array->values)[ii]),
						&bufferLength, (indentLevel + 1));

				string = strxpndcat(string, stringRef, stringLength,
						buffer, bufferLength);

				free(buffer);
				break;
		}

		if(ii < (array->length - 1)) {
			string = strxpndcat(string, stringRef, stringLength, ",\n",
					strlen(",\n"));
		}
		else {
			string = strxpndcat(string, stringRef, stringLength, "\n",
					strlen("\n"));
		}
	}

	free(indent);
	indent = buildStringIndent(indentLevel, &indentLength);

	string = strxpndcat(string, stringRef, stringLength, indent,
			indentLength);
	string = strxpndcat(string, stringRef, stringLength, "]",
			strlen("]"));

	free(indent);

	return string;
}

// object functions

static char *object_toString(Json *object, int *stringLength, int indentLevel)
{
	aboolean isLastElement = afalse;
	int ii = 0;
	int stringRef = 0;
	int indentLength = 0;
	char *indent = NULL;
	char *string = NULL;

	if(indentLevel < 0) {
		indentLevel = 0;
	}

	indent = buildStringIndent(indentLevel, &indentLength);

	stringRef = 0;
	*stringLength = 1024;
	string = (char *)malloc(sizeof(char) * (*stringLength));

/*	if(indentLength > 0) {
		string = strxpndcat(string, &stringRef, stringLength, indent,
				indentLength);
	}*/

	string = strxpndcat(string, &stringRef, stringLength, "{\n",
			strlen("{\n"));

	for(ii = 0; ii < object->elementLength; ii++) {
		if(ii == (object->elementLength - 1)) {
			isLastElement = atrue;
		}

		string = element_toString(&(object->elements[ii]), string, &stringRef,
				stringLength, (indentLevel + 1), isLastElement);
	}

	if(indentLength > 0) {
		string = strxpndcat(string, &stringRef, stringLength, indent,
				indentLength);
	}

	if(indentLength > 0) {
		string = strxpndcat(string, &stringRef, stringLength, "}",
				strlen("}"));
	}
	else {
		string = strxpndcat(string, &stringRef, stringLength, "}\n",
				strlen("}\n"));
	}

	free(indent);

	*stringLength = stringRef;

	return string;
}

// parsing functions

static void parse_consume(char **ptr, int *consumedBytes, int bytes)
{
/*	{
		int ii = 0;
		char value = (char)0;

		printf("CONSUME %03i :: '", bytes);

		for(ii = 0; ii < bytes; ii++) {
			value = (*ptr)[ii];

			if(((unsigned int)value < 32) || ((unsigned int)value > 126)) {
				printf("(%02i)", (int)((unsigned int)value));
			}
			else {
				printf("%c", value);
			}
		}

		printf("'\n");
	}*/

	(*ptr) += bytes;
	(*consumedBytes) += bytes;
}

static double parse_number(char *string, int *consumedBytes,
		aboolean *hasError)
{
	aboolean isInExponent = afalse;
	aboolean isNegative = afalse;
	aboolean isExpNegative = afalse;
	aboolean isInFraction = afalse;
	double number = 0.0;
	char *ptr = NULL;
	char *buffer = NULL;

	ptr = string;

	if((*ptr) == '-') {
		isNegative = atrue;
		ptr++;
	}
	else {
		isNegative = afalse;
	}

	while((*ptr) != '\0') {
		if((*ptr) == 'e') {
			if(isInExponent) {
				(*hasError) = atrue;
				break;
			}
			else {
				isInExponent = atrue;
				if((*ptr) == '-') {
					isExpNegative = atrue;
					ptr++;
				}
				else if((*ptr) == '+') {
					ptr++;
				}
			}

			continue;
		}

		if((*ptr) == '.') {
			if(isInFraction) {
				(*hasError) = atrue;
				break;
			}

			isInFraction = atrue;
			ptr++;

			continue;
		}

		if(!ctype_isNumeric(*ptr)) {
			ptr--;
			break;
		}

		ptr++;
	}

	if(*hasError) {
		return number;
	}

	*consumedBytes = ((int)(ptr - string) + 1);

	buffer = strndup(string, (*consumedBytes));

	number = atod(buffer);

	free(buffer);

	return number;
}

static char *parse_string(char *string, int *consumedBytes)
{
	aboolean hasError = afalse;
	aboolean isEncoded = afalse;
	int ii = 0;
	int nn = 0;
	char *ptr = NULL;
	char *result = NULL;

	ptr = string;

	if((*ptr) != '"') {
		return NULL;
	}

	ptr++;

	// check for empty string

	if((*ptr) == '"') {
		*consumedBytes = 2;
		return strdup("");
	}

	// determine string length

	while(((*ptr) != '\0') && ((*ptr) != '"')) {
		if((*ptr) == '\\') {
			isEncoded = atrue;
		}
		ptr++;
	}

	(*consumedBytes) = ((int)(ptr - string) + 1);
	result = strndup((string + 1), ((*consumedBytes) - 2));

	if(!isEncoded) {
		for(ii = 0, nn = 0; result[ii] != '\0'; ii++) {
			if(((unsigned char)result[ii] < 32) ||
					((unsigned char)result[ii] > 126)) {
				continue;
			}
			result[nn] = result[ii];
			nn++;
		}
		result[nn] = '\0';
		return result;
	}

	for(ii = 0, nn = 0; ((!hasError) && (result[ii] != '\0')); ii++, nn++) {
		if(result[ii] == '\\') {

			ii++;

			switch(result[ii]) {
				case '\0':
					result[ii] = '\0';
					hasError = atrue;
					break;

				case 'b':
					result[nn] = '\b';
					break;

				case 'f':
					result[nn] = '\f';
					break;

				case 'n':
					result[nn] = '\n';
					break;

				case 'r':
					result[nn] = '\r';
					break;

				case 't':
					result[nn] = '\t';
					break;

				case 'u':
					if((!ctype_isNumeric(result[(ii + 1)])) ||
							(ctype_isNumeric(result[(ii + 2)])) ||
							(ctype_isNumeric(result[(ii + 3)])) ||
							(ctype_isNumeric(result[(ii + 4)]))) {
						hasError = atrue;
						break;
					}

					/*
					 * TODO: consider supporting full unicode here
					 */

					result[nn] = ' ';
					ii += 4;
					break;

				default:
					result[nn] = result[ii];
					break;
			}
		}
		else {
			result[nn] = result[ii];
		}
	}

	result[nn] = '\0';

	return result;
}

static JsonElement *parse_value(char *string, int *consumedBytes,
		aboolean *hasError)
{
	aboolean bValue = afalse;
	int bytes = 0;
	double number = 0.0;
	char *ptr = NULL;

	JsonArray *array = NULL;
	JsonElement *value = NULL;
	JsonElement *element = NULL;
	Json *object = NULL;

	value = (JsonElement *)malloc(sizeof(JsonElement));
	value->type = JSON_VALUE_TYPE_UNKNOWN;

	ptr = string;

	(*consumedBytes) = 0;

	while(ctype_isWhitespace(*ptr)) {
		parse_consume(&ptr, consumedBytes, 1);
	}

	if(!strncmp(ptr, "true", 4)) {
		value->type = JSON_VALUE_TYPE_BOOLEAN;

		bValue = atrue;

		value->value = (void *)malloc(sizeof(aboolean));
		memcpy(value->value, (void *)&bValue, SIZEOF_BOOLEAN);

		parse_consume(&ptr, consumedBytes, 4);

		return value;
	}
	else if(!strncmp(ptr, "false", 5)) {
		value->type = JSON_VALUE_TYPE_BOOLEAN;

		bValue = afalse;

		value->value = (void *)malloc(sizeof(aboolean));
		memcpy(value->value, (void *)&bValue, SIZEOF_BOOLEAN);

		parse_consume(&ptr, consumedBytes, 5);

		return value;
	}
	else if(!strncmp(ptr, "null", 4)) {
		value->type = JSON_VALUE_TYPE_NULL;
		value->value = NULL;

		parse_consume(&ptr, consumedBytes, 4);

		return value;
	}
	else if((*ptr) == '"') { // extract string
		value->type = JSON_VALUE_TYPE_STRING;
		value->value = parse_string(ptr, &bytes);

		if(value->value == NULL) {
			fprintf(stderr, "[%s():%i] error - failed to parse value of JSON "
					"string.\n", __FUNCTION__, __LINE__);
			(*hasError) = atrue;
			return NULL;
		}

		parse_consume(&ptr, consumedBytes, bytes);

		return value;
	}
	else if((*ptr) == '{') { // extract object
		object = parse_json(ptr, strlen(ptr), &bytes);
		if(object == NULL) {
			fprintf(stderr, "[%s():%i] error - failed to parse value of JSON "
					"object.\n", __FUNCTION__, __LINE__);
			(*hasError) = atrue;
			return NULL;
		}

		value->type = JSON_VALUE_TYPE_OBJECT;
		value->value = (void *)object;

		parse_consume(&ptr, consumedBytes, bytes);

		return value;
	}
	else if((*ptr) == '[') { // extract array
		array = (JsonArray *)malloc(sizeof(JsonArray));

		parse_consume(&ptr, consumedBytes, 1);

		while(((*ptr) != '\0') && ((*ptr) != ']')) {
			if(((*ptr) == ',') || (ctype_isWhitespace((*ptr)))) {
				parse_consume(&ptr, consumedBytes, 1);
				continue;
			}

			element = parse_value(ptr, &bytes, hasError);

			if((element == NULL)  || ((*hasError) == atrue)) {
				fprintf(stderr, "[%s():%i] error - failed to parse value of "
						"JSON array.\n", __FUNCTION__, __LINE__);
				if(element != NULL) {
					element_freePtr(element);
				}
				free(value);
				(*hasError) = atrue;
				return NULL;
			}

			array_add(array, element->type, element->value);

			if(element->type != JSON_VALUE_TYPE_OBJECT) {
				element_freePtr(element);
			}
			else {
				if(element->name != NULL) {
					free(element->name);
				}
				free(element);
			}

			parse_consume(&ptr, consumedBytes, bytes);
		}

		value->type = JSON_VALUE_TYPE_ARRAY;
		value->value = array;

		if((*ptr) == ']') {
			parse_consume(&ptr, consumedBytes, 1);
		}

		return value;
	}
	else if((ctype_isNumeric(*ptr)) || ((*ptr) == '-')) { // extract number
		number = parse_number(ptr, &bytes, hasError);

		value->type = JSON_VALUE_TYPE_NUMBER;

		value->value = (void *)malloc(sizeof(double));
		memcpy(value->value, (void *)&number, sizeof(double));

		parse_consume(&ptr, consumedBytes, bytes);

		return value;
	}
	else if((!strncmp(ptr, "nan", 3)) || (!strncmp(ptr, "inf", 3))) {
		number = 0.0;

		value->type = JSON_VALUE_TYPE_NUMBER;

		value->value = (void *)malloc(sizeof(double));
		memcpy(value->value, (void *)&number, sizeof(double));

		parse_consume(&ptr, consumedBytes, 3);

		return value;
	}

	free(value);

	(*hasError) = atrue;

	return NULL;
}

static Json *parse_json(char *string, int stringLength, int *consumedBytes)
{
	aboolean hasError = afalse;
	int bytes = 0;
	char *ptr = NULL;
	char *name = NULL;

	Json *object = NULL;
	JsonElement *element = NULL;

	ptr = string;

	if((*ptr) != '{') {
		return NULL;
	}

	object = json_new();

	parse_consume(&ptr, consumedBytes, 1);

	while(((*ptr) != '\0') &&
			((*ptr) != '}') &&
			(!hasError) &&
			((stringLength - (int)(ptr - string)) > 0)) {
		if(ctype_isWhitespace(*ptr)) {
			parse_consume(&ptr, consumedBytes, 1);
			continue;
		}

		switch(*ptr) {
			/*
			 * extract a name
			 */
			case '"':
				if(name != NULL) {
					hasError = atrue;
					break;
				}

				name = parse_string(ptr, &bytes);

				if((name == NULL) || (strlen(name) < 1)) {
					fprintf(stderr, "[%s():%i] error - failed to parse name "
							"of JSON key, name is missing or invalid.\n",
							__FUNCTION__, __LINE__);
					json_freePtr(object);
					return NULL;
				}

				parse_consume(&ptr, consumedBytes, bytes);

				break;

			/*
			 * extract a value
			 */
			case ':':
				if(name == NULL) {
					hasError = atrue;
					break;
				}

				parse_consume(&ptr, consumedBytes, 1);

				element = parse_value(ptr, &bytes, &hasError);

				if((element == NULL) || (hasError)) {
					fprintf(stderr, "[%s():%i] error - failed to parse value "
							"of JSON key '%s'.\n", __FUNCTION__, __LINE__,
							name);
					if(element != NULL) {
						element_freePtr(element);
					}
					json_freePtr(object);
					return NULL;
				}

				parse_consume(&ptr, consumedBytes, bytes);

				element->name = name;
				element->nameLength = strlen(name);

				if(element_addElement(object, element) < 0) {
					element_freePtr(element);
				}

				name = NULL;
				element = NULL;

				break;

			/*
			 * start the next element in the object
			 */
			case ',':
				parse_consume(&ptr, consumedBytes, 1);
				break;

			default:
				hasError = atrue;
		}
	}

	if((*ptr) == '}') {
		parse_consume(&ptr, consumedBytes, 1);
	}

	return object;
}


// define json public functions

int json_init(Json *object)
{
	if(object == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	memset(object, 0, (sizeof(Json)));

	object->elementLength = 0;
	object->elements = NULL;

	return 0;
}

Json *json_new()
{
	Json *result = NULL;

	result = (Json *)malloc(sizeof(Json));

	json_init(result);

	return result;
}

Json *json_newFromString(char *string)
{
	int stringLength = 0;
	int consumedBytes = 0;

	Json *result = NULL;

	if((string == NULL) || ((stringLength = strlen(string)) < 1)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	result = parse_json(string, stringLength, &consumedBytes);

	if((result != NULL) && (stringLength < consumedBytes)) {
		json_freePtr(result);
		result = NULL;
	}

	return result;
}

int json_free(Json *object)
{
	int ii = 0;

	if(object == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(object->elements != NULL) {
		for(ii = 0; ii < object->elementLength; ii++) {
			element_free(&object->elements[ii]);
		}
		free(object->elements);
	}

	memset(object, 0, (sizeof(Json)));

	return 0;
}

int json_freePtr(Json *object)
{
	if(object == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	json_free(object);

	free(object);

	return 0;
}

char *json_valueTypeToString(JsonValueType type)
{
	char *result = "unknown";

	switch(type) {
		case JSON_VALUE_TYPE_BOOLEAN:
			result = "Boolean";
			break;

		case JSON_VALUE_TYPE_NUMBER:
			result = "Number";
			break;

		case JSON_VALUE_TYPE_STRING:
			result = "String";
			break;

		case JSON_VALUE_TYPE_ARRAY:
			result = "Array";
			break;

		case JSON_VALUE_TYPE_OBJECT:
			result = "Object";
			break;

		case JSON_VALUE_TYPE_NULL:
			result = "Null";
			break;

		case JSON_VALUE_TYPE_UNKNOWN:
			result = "Unknown";
			break;

		default:
			result = "error";
	}

	return result;
}

char *json_toString(Json *object, int *stringLength)
{
	if((object == NULL) || (stringLength == NULL)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	return object_toString(object, stringLength, 0);
}

int json_addBoolean(Json *object, char *name, aboolean value)
{
	if((object == NULL) || (name == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return element_add(object, JSON_VALUE_TYPE_BOOLEAN, name, (void *)&value);
}

int json_addNumber(Json *object, char *name, double value)
{
	if((object == NULL) || (name == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return element_add(object, JSON_VALUE_TYPE_NUMBER, name, (void *)&value);
}

int json_addString(Json *object, char *name, char *value)
{
	if((object == NULL) || (name == NULL) || (value == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return element_add(object, JSON_VALUE_TYPE_STRING, name, (void *)value);
}

int json_addArray(Json *object, char *name)
{
	if((object == NULL) || (name == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return element_add(object, JSON_VALUE_TYPE_ARRAY, name, NULL);
}

int json_addObject(Json *object, char *name, Json *value)
{
	if((object == NULL) || (name == NULL) || (value == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return element_add(object, JSON_VALUE_TYPE_OBJECT, name, value);
}

int json_addBooleanToArray(Json *object, char *name, aboolean value)
{
	JsonElement *element = NULL;

	if((object == NULL) || (name == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if((element = element_search(object, name)) == NULL) {
		return -1;
	}

	return array_add((JsonArray *)element->value, JSON_VALUE_TYPE_BOOLEAN,
			(void *)&value);
}

int json_addNumberToArray(Json *object, char *name, double value)
{
	JsonElement *element = NULL;

	if((object == NULL) || (name == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if((element = element_search(object, name)) == NULL) {
		return -1;
	}

	return array_add((JsonArray *)element->value, JSON_VALUE_TYPE_NUMBER,
			(void *)&value);
}

int json_addStringToArray(Json *object, char *name, char *value)
{
	JsonElement *element = NULL;

	if((object == NULL) || (name == NULL) || (value == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if((element = element_search(object, name)) == NULL) {
		return -1;
	}

	return array_add((JsonArray *)element->value, JSON_VALUE_TYPE_STRING,
			(void *)value);
}

int json_addArrayToArray(Json *object, char *name)
{
	int result = 0;

	JsonElement *element = NULL;

	if((object == NULL) || (name == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if((element = element_search(object, name)) == NULL) {
		return -1;
	}

	if((result = array_add((JsonArray *)element->value, JSON_VALUE_TYPE_ARRAY,
					NULL)) == 0) {
		((JsonArray *)element->value)->nameLength = strlen(name);
		((JsonArray *)element->value)->name = strdup(name);
	}

	return result;
}

int json_addObjectToArray(Json *object, char *name, Json *value)
{
	JsonElement *element = NULL;

	if((object == NULL) || (name == NULL) || (value == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if((element = element_search(object, name)) == NULL) {
		return -1;
	}

	return array_add((JsonArray *)element->value, JSON_VALUE_TYPE_OBJECT,
			(void *)value);
}

aboolean json_elementExists(Json *object, char *name)
{
	JsonElement *element = NULL;

	if((object == NULL) || (name == NULL)) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	if((element = element_search(object, name)) == NULL) {
		return afalse;
	}

	return atrue;
}

JsonValueType json_getElementType(Json *object, char *name)
{
	JsonElement *element = NULL;

	if((object == NULL) || (name == NULL)) {
		DISPLAY_INVALID_ARGS;
		return JSON_VALUE_TYPE_UNKNOWN;
	}

	if((element = element_search(object, name)) == NULL) {
		return JSON_VALUE_TYPE_UNKNOWN;
	}

	return (JsonValueType)element->type;
}

aboolean json_getBoolean(Json *object, char *name)
{
	aboolean result = afalse;
	double dValue = 0;
	char *sValue = NULL;

	JsonElement *element = NULL;

	if((object == NULL) || (name == NULL)) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	if((element = element_search(object, name)) == NULL) {
		return afalse;
	}

	switch(element->type) {
		case JSON_VALUE_TYPE_BOOLEAN:
			result = *((aboolean *)element->value);
			break;

		case JSON_VALUE_TYPE_NUMBER:
			dValue = *((double *)element->value);
			if(dValue == 0.0) {
				result = afalse;
			}
			else {
				result = atrue;
			}
			break;

		case JSON_VALUE_TYPE_STRING:
			sValue = ((char *)element->value);
			if((!strcasecmp(sValue, "true")) ||
					(!strcasecmp(sValue, "1")) ||
					(atoi(sValue) != 0)) {
				result = atrue;
			}
			else {
				result = afalse;
			}
			break;

		case JSON_VALUE_TYPE_ARRAY:
		case JSON_VALUE_TYPE_OBJECT:
		case JSON_VALUE_TYPE_NULL:
		case JSON_VALUE_TYPE_UNKNOWN:
		default:
			result = afalse;
	}

	return result;
}

double json_getNumber(Json *object, char *name)
{
	aboolean bValue = afalse;
	double result = 0.0;
	char *sValue = NULL;

	JsonElement *element = NULL;

	if((object == NULL) || (name == NULL)) {
		DISPLAY_INVALID_ARGS;
		return 0.0;
	}

	if((element = element_search(object, name)) == NULL) {
		return 0.0;
	}

	switch(element->type) {
		case JSON_VALUE_TYPE_BOOLEAN:
			bValue = *((aboolean *)element->value);
			if(bValue) {
				result = 1.0;
			}
			else {
				result = 0.0;
			}
			break;

		case JSON_VALUE_TYPE_NUMBER:
			result = *((double *)element->value);
			break;

		case JSON_VALUE_TYPE_STRING:
			sValue = ((char *)element->value);
			result = atod(sValue);
			break;

		case JSON_VALUE_TYPE_ARRAY:
		case JSON_VALUE_TYPE_OBJECT:
		case JSON_VALUE_TYPE_NULL:
		case JSON_VALUE_TYPE_UNKNOWN:
		default:
			result = 0.0;
	}

	return result;
}

char *json_getString(Json *object, char *name)
{
	aboolean bValue = afalse;
	double dValue = 0.0;
	char *result = NULL;
	char buffer[128];

	JsonElement *element = NULL;

	if((object == NULL) || (name == NULL)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	if((element = element_search(object, name)) == NULL) {
		return NULL;
	}

	switch(element->type) {
		case JSON_VALUE_TYPE_BOOLEAN:
			bValue = *((aboolean *)element->value);
			if(bValue) {
				result = "true";
			}
			else {
				result = "false";
			}
			break;

		case JSON_VALUE_TYPE_NUMBER:
			dValue = *((double *)element->value);
			snprintf(buffer, ((int)sizeof(buffer) - 1), "%f", dValue);
			result = buffer;
			break;

		case JSON_VALUE_TYPE_STRING:
			result = ((char *)element->value);
			break;

		case JSON_VALUE_TYPE_ARRAY:
		case JSON_VALUE_TYPE_OBJECT:
		case JSON_VALUE_TYPE_NULL:
		case JSON_VALUE_TYPE_UNKNOWN:
		default:
			result = NULL;
	}

	return result;
}

Json *json_getObject(Json *object, char *name)
{
	Json *result = NULL;
	JsonElement *element = NULL;

	if((object == NULL) || (name == NULL)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	if((element = element_search(object, name)) == NULL) {
		return NULL;
	}

	if(element->type != JSON_VALUE_TYPE_OBJECT) {
		return NULL;
	}

	result = ((Json *)element->value);

	return result;
}

aboolean json_getBooleanFromArray(Json *object, char *name, int index)
{
	aboolean result = afalse;

	JsonArray *array = NULL;

	if((object == NULL) || (name == NULL) || (index < 0)) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	if((array = array_getFromElement(object, name, index)) == NULL) {
		return afalse;
	}

	if(array->types[index] != JSON_VALUE_TYPE_BOOLEAN) {
		return afalse;
	}

	result = *((aboolean *)((void **)array->values)[index]);

	return result;
}

double json_getNumberFromArray(Json *object, char *name, int index)
{
	double result = 0.0;

	JsonArray *array = NULL;

	if((object == NULL) || (name == NULL) || (index < 0)) {
		DISPLAY_INVALID_ARGS;
		return 0.0;
	}

	if((array = array_getFromElement(object, name, index)) == NULL) {
		return 0.0;
	}

	if(array->types[index] != JSON_VALUE_TYPE_NUMBER) {
		return 0.0;
	}

	result = *((double *)((void **)array->values)[index]);

	return result;
}

char *json_getStringFromArray(Json *object, char *name, int index)
{
	char *result = NULL;

	JsonArray *array = NULL;

	if((object == NULL) || (name == NULL) || (index < 0)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	if((array = array_getFromElement(object, name, index)) == NULL) {
		return NULL;
	}

	if(array->types[index] != JSON_VALUE_TYPE_STRING) {
		return NULL;
	}

	result = ((char *)((void **)array->values)[index]);

	return result;
}

Json *json_getObjectFromArray(Json *object, char *name, int index)
{
	Json *result = NULL;
	JsonArray *array = NULL;

	if((object == NULL) || (name == NULL) || (index < 0)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	if((array = array_getFromElement(object, name, index)) == NULL) {
		return NULL;
	}

	if(array->types[index] != JSON_VALUE_TYPE_OBJECT) {
		return NULL;
	}

	result = ((Json *)((void **)array->values)[index]);

	return result;
}

int json_getArrayLength(Json *object, char *name)
{
	int result = 0;

	JsonArray *array = NULL;

	if((object == NULL) || (name == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if((array = array_getFromElement(object, name, 0)) == NULL) {
		return -1;
	}

	result = array->length;

	return result;
}

// helper functions

int json_minimizeJsonString(char *string, int stringLength,
		aboolean isNetworkOptimized)
{
	aboolean isVisible = atrue;
	int ii = 0;
	int nn = 0;

	if((string == NULL) || (stringLength < 1)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	for(ii = 0, nn = 0; ii < stringLength; ii++) {
		if(!ctype_isPlainText(string[ii])) {
			continue;
		}

		if((ctype_ctoi(string[ii]) == 10) || (ctype_ctoi(string[ii]) == 13)) {
			continue;
		}

		if((string[ii] == '"') &&
				(ii > 0) &&
				(string[(ii - 1)] != '\\')) {
			if(isVisible) {
				isVisible = afalse;
			}
			else {
				isVisible = atrue;
			}
		}

		if(ctype_ctoi(string[ii]) == 9) {
			string[ii] = ' ';
		}

		if(isNetworkOptimized) {
			if((isVisible) && (string[ii] == ' ')) {
				continue;
			}
		}
		else {
			if((isVisible) &&
					(ii > 0) &&
					(string[ii] == ' ') &&
					(string[(ii - 1)] == ' ')) {
				continue;
			}
		}

		string[nn] = string[ii];
		nn++;
	}

	string[nn] = '\0';

	return nn;
}

