/*
 * serialize_unitTest.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard core library serialization functions, unit test.
 *
 * Written by Josh English.
 */

// preprocessor directives

#define _CORE_COMMON_BYPASS_STD_STREAM
#include "core/core.h"

// declare serialize unit test private functions

static int serialize_basicFunctionalityTest();

// main function

int main()
{
	signal_registerDefault();

	printf("[unit] serialize unit test (Asgard Ver %s on %s)\n", ASGARD_VERSION,
			ASGARD_DATE);

	if(serialize_basicFunctionalityTest() < 0) {
		printf("[unit]\t ...FAILED\n");
		return 1;
	}

	printf("[unit] ...PASSED\n");

	return 0;
}

// define serialize unit test private functions

static int serialize_basicFunctionalityTest()
{
	short shortValueOne = 0;
	short shortValueTwo = 0;
	int intValueOne = 0;
	int intValueTwo = 0;
	float floatValueOne = 0;
	float floatValueTwo = 0;
	alint alintValueOne = 0;
	alint alintValueTwo = 0;
	double doubleValueOne = 0;
	double doubleValueTwo = 0;
	char buffer[1024];

	printf("[unit]\t serialize basic functionality test...\n");

	shortValueOne = (short)rand();
	if(serialize_encodeShort(shortValueOne, buffer) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(serialize_decodeShort(buffer, SIZEOF_SHORT, &shortValueTwo) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(shortValueOne != shortValueTwo) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t\t short value one: %i\n", shortValueOne);
	printf("[unit]\t\t short value two: %i\n", shortValueTwo);

	intValueOne = (int)rand();
	if(serialize_encodeInt(intValueOne, buffer) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(serialize_decodeInt(buffer, SIZEOF_INT, &intValueTwo) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(intValueOne != intValueTwo) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t\t int value one: %i\n", intValueOne);
	printf("[unit]\t\t int value two: %i\n", intValueTwo);

	floatValueOne = (float)rand();
	if(serialize_encodeFloat(floatValueOne, buffer) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(serialize_decodeFloat(buffer, SIZEOF_FLOAT, &floatValueTwo) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(floatValueOne != floatValueTwo) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t\t float value one: %f\n", floatValueOne);
	printf("[unit]\t\t float value two: %f\n", floatValueTwo);

	alintValueOne = (alint)rand();
	if(serialize_encodeAlint(alintValueOne, buffer) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(serialize_decodeAlint(buffer, SIZEOF_ALINT, &alintValueTwo) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(alintValueOne != alintValueTwo) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t\t alint value one: %lli\n", alintValueOne);
	printf("[unit]\t\t alint value two: %lli\n", alintValueTwo);

	doubleValueOne = (double)rand();
	if(serialize_encodeDouble(doubleValueOne, buffer) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(serialize_decodeDouble(buffer, SIZEOF_DOUBLE, &doubleValueTwo) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(doubleValueOne != doubleValueTwo) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t\t double value one: %f\n", doubleValueOne);
	printf("[unit]\t\t double value two: %f\n", doubleValueTwo);

	printf("[unit]\t ...PASSED\n");

	return 0;
}

