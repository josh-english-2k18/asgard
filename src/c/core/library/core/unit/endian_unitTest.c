/*
 * endian_unitTest.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard core library endian functions, unit test.
 *
 * Written by Josh English.
 */

// preprocessor directives

#define _CORE_COMMON_BYPASS_STD_STREAM
#include "core/core.h"

// declare endian unit test private functions

static int endian_basicFunctionalityTest();

// main function

int main()
{
	signal_registerDefault();

	printf("[unit] endian unit test (Asgard Ver %s on %s)\n", ASGARD_VERSION,
			ASGARD_DATE);

	if(endian_basicFunctionalityTest() < 0) {
		printf("[unit]\t ...FAILED\n");
		return 1;
	}

	printf("[unit] ...PASSED\n");

	return 0;
}

// define endian unit test private functions

static int endian_basicFunctionalityTest()
{
	int byteOrder = 0;
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

	printf("[unit]\t endian basic functionality test...\n");

	byteOrder = endian_determineByteOrder();

	if((byteOrder != ENDIAN_BYTEORDER_LOHI) &&
			(byteOrder != ENDIAN_BYTEORDER_HILO)) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t\t system endian-ness is: %s\n",
			endian_byteOrderToString(byteOrder));

	shortValueOne = (short)rand();
	shortValueTwo = endian_invertShort(shortValueOne);
	if(shortValueOne == shortValueTwo) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t\t short value one: %i\n", shortValueOne);
	printf("[unit]\t\t short value two: %i\n", shortValueTwo);

	intValueOne = (int)rand();
	intValueTwo = endian_invertInt(intValueOne);
	if(intValueOne == intValueTwo) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t\t int value one: %i\n", intValueOne);
	printf("[unit]\t\t int value two: %i\n", intValueTwo);

	floatValueOne = (float)rand();
	floatValueTwo = endian_invertFloat(floatValueOne);
	if(floatValueOne == floatValueTwo) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t\t float value one: %f\n", floatValueOne);
	printf("[unit]\t\t float value two: %f\n", floatValueTwo);

	alintValueOne = (alint)rand();
	alintValueTwo = endian_invertAlint(alintValueOne);
	if(alintValueOne == alintValueTwo) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t\t alint value one: %lli\n", alintValueOne);
	printf("[unit]\t\t alint value two: %lli\n", alintValueTwo);

	doubleValueOne = (double)rand();
	doubleValueTwo = endian_invertDouble(doubleValueOne);
	if(doubleValueOne == doubleValueTwo) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t\t double value one: %f\n", doubleValueOne);
	printf("[unit]\t\t double value two: %f\n", doubleValueTwo);

	printf("[unit]\t ...PASSED\n");

	return 0;
}

