/*
 * json_unitTest.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard core library JSON type library, unit test.
 *
 * Written by Josh English.
 */

// preprocessor directives

#define _CORE_COMMON_BYPASS_STD_STREAM
#include "core/core.h"

// declare json unit tests

static int json_basicFunctionalityTest();

// main function

int main()
{
	signal_registerDefault();

	printf("[unit] json unit test (Asgard Ver %s on %s)\n",
			ASGARD_VERSION, ASGARD_DATE);

	if(json_basicFunctionalityTest() < 0) {
		printf("[unit]\t ...FAILED\n");
		return 1;
	}

	printf("[unit] ...PASSED\n");

	return 0;
}

// define json unit tests

static int json_basicFunctionalityTest()
{
	int stringLength = 0;
	char *string = NULL;

	Json *object = NULL;
	Json *childObject = NULL;

	const char *SOURCE_ONE = ""
		"{\n"
		"\t\"glossary\": {\n"
		"\t\t\"title\": \"example glossary\",\n"
		"\t\t\"emptyTitle\": \"\",\n"
		"\t\t\"GlossDiv\": {\n"
		"\t\t\t\"title\": \"S\",\n"
		"\t\t\t\"GlossList\": {\n"
		"\t\t\t\t\"GlossEntry\": {\n"
		"\t\t\t\t\t\"ID\": \"SGML\",\n"
		"\t\t\t\t\t\"SortAs\": \"SGML\",\n"
		"\t\t\t\t\t\"GlossTerm\": \"Standard Generalized Markup Language\",\n"
		"\t\t\t\t\t\"Acronym\": \"SGML\",\n"
		"\t\t\t\t\t\"EmptyAcronym\": \"\",\n"
		"\t\t\t\t\t\"Abbrev\": \"ISO 8879:1986\",\n"
		"\t\t\t\t\t\"GlossDef\": {\n"
		"\t\t\t\t\t\t\"para\": \"A meta-markup language, used to "
			"create markup lan\x0agua\x0dges such as DocBook.\",\n"
		"\t\t\t\t\t\t\"GlossSeeAlso\": [\"GML\", \"XML\"]\n"
		"\t\t\t\t\t},\n"
		"\t\t\t\t\t\"GlossSee\": \"markup\"\n"
		"\t\t\t\t}\n"
		"\t\t\t}\n"
		"\t\t}\n"
		"\t}\n"
		"}\n"
		"";

	const char *SOURCE_TWO = ""
		"{\"t\":\"xt\",\"b\":"
		"{\"r\":-1,\"o\":"
		"{\"id\":1234567890,\"result\":\"OK\","
		"\"name\":\"StormPegasus1234\",\"_cmd\":\"login\"}}}"
		"";

	const char *SOURCE_THREE = ""
		"{\"t\":\"xt\",\"b\":{\"r\":-1,\"o\":{\"zoneStats\":{\"Ice\":2,\"Pyramid\":0},\"result\":\"OK\",\"opponents\":[{\"artName\":\"null\",\"level\":\"13\",\"name\":\"StormPegasus1234\",\"string\":\"this is a    string test\",\"points\":\"1238\"}],\"_cmd\":\"joinedRoom\",\"room\":\"Ice\"}}}"
		"";

	printf("[unit]\t json basic functionality test...\n");

	if((object = json_new()) == NULL) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	// create child object

	if((childObject = json_new()) == NULL) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(json_addBoolean(childObject, "child_booleanValue", afalse) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(json_addNumber(childObject, "child_numberValue", 789.456) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(json_addString(childObject, "child_stringValue",
				"this is a test string for the child object") < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(json_addArray(childObject, "child_arrayValue") < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(json_addBooleanToArray(childObject, "child_arrayValue", afalse) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(json_addNumberToArray(childObject, "child_arrayValue", 321.654) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(json_addStringToArray(childObject, "child_arrayValue",
				"this is an array string for the child object") < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	// create parent object

	if(json_addBoolean(object, "booleanValue", atrue) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(json_addNumber(object, "numberValue", 123.456) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(json_addString(object, "stringValue", "this is a test string") < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(json_addString(object, "stringValue2", "5678") < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(json_addArray(object, "arrayValue") < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(json_addBooleanToArray(object, "arrayValue", atrue) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(json_addNumberToArray(object, "arrayValue", 456.123) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(json_addStringToArray(object, "arrayValue",
				"this is an array string") < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(json_addObject(object, "objectValue", childObject) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	// check parent object values

	if(json_getBoolean(object, "booleanValue") != atrue) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(json_getNumber(object, "numberValue") != 123.456) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(strcmp(json_getString(object, "stringValue"), "this is a test string")) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(json_getArrayLength(object, "arrayValue") != 3) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(json_getBooleanFromArray(object, "arrayValue", 0) != atrue) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(json_getNumberFromArray(object, "arrayValue", 1) != 456.123) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(strcmp(json_getStringFromArray(object, "arrayValue", 2),
				"this is an array string")) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if((childObject = json_getObject(object, "objectValue")) == NULL) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	// check child object values

	if(json_getBoolean(childObject, "child_booleanValue") != afalse) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(json_getNumber(childObject, "child_numberValue") != 789.456) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(strcmp(json_getString(childObject, "child_stringValue"),
				"this is a test string for the child object")) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(json_getArrayLength(childObject, "child_arrayValue") != 3) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(json_getBooleanFromArray(childObject, "child_arrayValue", 0) != afalse) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(json_getNumberFromArray(childObject, "child_arrayValue", 1) != 321.654) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(strcmp(json_getStringFromArray(childObject, "child_arrayValue", 2),
				"this is an array string for the child object")) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	// existence & type tests

	if(!json_elementExists(object, "booleanValue")) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(json_getElementType(object, "booleanValue") != JSON_VALUE_TYPE_BOOLEAN) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(!json_elementExists(object, "numberValue")) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(json_getElementType(object, "numberValue") != JSON_VALUE_TYPE_NUMBER) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(!json_elementExists(object, "stringValue")) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(json_getElementType(object, "stringValue") != JSON_VALUE_TYPE_STRING) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	// conversion tests

	if(json_getBoolean(object, "booleanValue") != atrue) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(json_getNumber(object, "booleanValue") != 1.0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(strcmp(json_getString(object, "booleanValue"), "true")) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(json_getBoolean(object, "numberValue") != atrue) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(json_getNumber(object, "numberValue") != 123.456) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(strcmp(json_getString(object, "numberValue"), "123.456000")) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(json_getBoolean(object, "stringValue") != afalse) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(json_getNumber(object, "stringValue") != 0.0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(strcmp(json_getString(object, "stringValue"), "this is a test string")) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(json_getBoolean(object, "stringValue2") != atrue) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(json_getNumber(object, "stringValue2") != 5678.0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(strcmp(json_getString(object, "stringValue2"), "5678")) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	// display the object

	if(((string = json_toString(object, &stringLength)) == NULL) ||
			(stringLength < 1)) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("JSON Object (length %i bytes):\n%s", stringLength, string);

	free(string);

	// cleanup object

	if(json_freePtr(object) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	// parsing test #1

	if((object = json_newFromString((char *)SOURCE_ONE)) == NULL) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	// display the object

	if(((string = json_toString(object, &stringLength)) == NULL) ||
			(stringLength < 1)) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("JSON Object (length %i bytes):\n%s", stringLength, string);

	free(string);

	// cleanup object

	if(json_freePtr(object) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	// parsing test #2

	if((object = json_newFromString((char *)SOURCE_TWO)) == NULL) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	// display the object

	if(((string = json_toString(object, &stringLength)) == NULL) ||
			(stringLength < 1)) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("JSON Object (length %i bytes):\n%s", stringLength, string);

	free(string);

	// cleanup object

	if(json_freePtr(object) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	// parsing test #3

	if((object = json_newFromString((char *)SOURCE_THREE)) == NULL) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	// display the object

	if(((string = json_toString(object, &stringLength)) == NULL) ||
			(stringLength < 1)) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("JSON Object (length %i bytes):\n%s", stringLength, string);

	if((stringLength = json_minimizeJsonString(string, stringLength,
					afalse)) < 1) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("JSON Object, Minimized (length %i bytes):\n\n%s\n\n",
			stringLength, string);

	free(string);

	if(((string = json_toString(object, &stringLength)) == NULL) ||
			(stringLength < 1)) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if((stringLength = json_minimizeJsonString(string, stringLength,
					atrue)) < 1) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("JSON Object, Network Minimized (length %i bytes):\n\n%s\n\n",
			stringLength, string);

	if(strcmp(string, SOURCE_THREE)) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	free(string);

	// cleanup object

	if(json_freePtr(object) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t ...PASSED\n");

	return 0;
}

