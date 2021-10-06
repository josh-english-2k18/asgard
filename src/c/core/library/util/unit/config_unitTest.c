/*
 * config_unitTest.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Unix/Linux-style configuration-file interface utility library, unit test.
 *
 * Written by Josh English.
 */

// preprocessor directives

#define _CORE_COMMON_BYPASS_STD_STREAM
#include "core/core.h"

// declare config unit test constants

#define CONFIG_BASIC_TEST_FILENAME						\
	"assets/data/test/testfile.config"

#define CONFIG_BASIC_TEST_WRITE_FILENAME				\
	"assets/data/test/testfile.wite.config"

// declare config unit tests

static int config_basicFunctionalityTest();

// main function

int main()
{
	signal_registerDefault();

	printf("[unit] config unit test (Asgard Ver %s on %s)\n",
			ASGARD_VERSION, ASGARD_DATE);

	if(config_basicFunctionalityTest() < 0) {
		printf("[unit]\t ...FAILED\n");
		return 1;
	}

	printf("[unit] ...PASSED\n");

	return 0;
}

// define config unit tests

static int config_basicFunctionalityTest()
{
	aboolean bValue = afalse;
	int iValue = 0;
	alint lValue = 0;
	double dValue = 0.0;
	char *sValue = NULL;

	Config config;

	printf("[unit]\t config basic functionality test...\n");
	printf("[unit]\t\t config test filename: %s\n",
			CONFIG_BASIC_TEST_FILENAME);

	if((system_fileExists(CONFIG_BASIC_TEST_WRITE_FILENAME, &bValue)) &&
			(bValue)) {
		system_fileDelete(CONFIG_BASIC_TEST_WRITE_FILENAME);
	}

	if(config_init(&config, CONFIG_BASIC_TEST_FILENAME) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(config_display(stdout, &config) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	// test boolean type-casting

	if(!config_exists(&config, "section.one", "booleanAttributeOne")) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(config_getBoolean(&config, "section.one", "booleanAttributeOne", afalse,
				&bValue) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(bValue != atrue) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(config_getBoolean(&config, "section.one", "booleanAttributeTwo", atrue,
				&bValue) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(bValue != afalse) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(config_getBoolean(&config, "section.one", "booleanAttributeThree",
				afalse, &bValue) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(bValue != atrue) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(config_getBoolean(&config, "section.one", "booleanAttributeFour", atrue,
				&bValue) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(bValue != afalse) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(config_putBoolean(&config, "section.one", "putBooleanAttribute",
				atrue) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(config_getBoolean(&config, "section.one", "putBooleanAttribute", afalse,
				&bValue) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(bValue != atrue) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(config_putBoolean(&config, "section.two", "putBooleanAttribute",
				atrue) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(config_getBoolean(&config, "section.two", "putBooleanAttribute", afalse,
				&bValue) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(bValue != atrue) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	// test integer type-casting

	if(config_getInteger(&config, "section.one", "integerAttributeOne", -1,
				&iValue) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(iValue != 12345) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(config_getInteger(&config, "section.one", "integerAttributeTwo", -1,
				&iValue) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(iValue != 67890) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(config_putInteger(&config, "section.one", "putIntegerAttribute",
				536245) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(config_getInteger(&config, "section.one", "putIntegerAttribute", -1,
				&iValue) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(iValue != 536245) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(config_putInteger(&config, "section.two", "putIntegerAttribute",
				536245) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(config_getInteger(&config, "section.two", "putIntegerAttribute", -1,
				&iValue) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(iValue != 536245) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	// test big-integer type-casting

	if(config_getBigInteger(&config, "section.one", "bigIntegerAttributeOne",
				-1, &lValue) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(lValue != 5361253546875) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(config_putBigInteger(&config, "section.one", "putBigIntegerAttribute",
				5321645687) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(config_getBigInteger(&config, "section.one", "putBigIntegerAttribute",
				-1, &lValue) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(lValue != 5321645687) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(config_putBigInteger(&config, "section.two", "putBigIntegerAttribute",
				5321645687) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(config_getBigInteger(&config, "section.two", "putBigIntegerAttribute",
				-1, &lValue) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(lValue != 5321645687) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	// test double type-casting

	if(config_getDouble(&config, "section.one", "doubleAttributeOne", -1.0,
				&dValue) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(dValue != 12345.12345) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(config_getDouble(&config, "section.one", "doubleAttributeTwo", -1.0,
				&dValue) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(dValue != 67890.06789) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(config_putDouble(&config, "section.one", "putDoubleAttribute",
				123456.45678) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(config_getDouble(&config, "section.one", "putDoubleAttribute", -1.0,
				&dValue) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(dValue != 123456.45678) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(config_putDouble(&config, "section.two", "putDoubleAttribute",
				123456.45678) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(config_getDouble(&config, "section.two", "putDoubleAttribute", -1.0,
				&dValue) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(dValue != 123456.45678) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	// test strings

	if(config_getString(&config, "section.one", "testAttributeOne", "unknown",
				&sValue) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(strcmp(sValue, "testValueOne")) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(config_getString(&config, "section.one", "testAttributeTwo", "unknown",
				&sValue) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(strcmp(sValue, "testValueTwo")) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(config_getString(&config, "section.one", "testAttributeThree", "unknown",
				&sValue) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(strcmp(sValue, "testValueThree")) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(config_getString(&config, "section.one", "testAttributeFour", "unknown",
				&sValue) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(strcmp(sValue, "   this is a test attribute value #4   ")) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(config_putString(&config, "section.one", "putTestAttribute",
				"some test string") < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(config_getString(&config, "section.one", "putTestAttribute", "unknown",
				&sValue) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(strcmp(sValue, "some test string")) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(config_putString(&config, "section.two", "putTestAttribute",
				"some test string") < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(config_getString(&config, "section.two", "putTestAttribute", "unknown",
				&sValue) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(strcmp(sValue, "some test string")) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	// write the configuration to a file

	if(config_write(&config, CONFIG_BASIC_TEST_WRITE_FILENAME) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	// cleanup

	if(config_free(&config) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	// verify written configuration file can be read

	if(config_init(&config, CONFIG_BASIC_TEST_WRITE_FILENAME) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(config_display(stdout, &config) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	// test boolean type-casting

	if(config_getBoolean(&config, "section.one", "booleanAttributeOne", afalse,
				&bValue) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(bValue != atrue) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(config_getBoolean(&config, "section.one", "booleanAttributeTwo", atrue,
				&bValue) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(bValue != afalse) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(config_getBoolean(&config, "section.one", "booleanAttributeThree",
				afalse, &bValue) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(bValue != atrue) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(config_getBoolean(&config, "section.one", "booleanAttributeFour", atrue,
				&bValue) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(bValue != afalse) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	// test integer type-casting

	if(config_getInteger(&config, "section.one", "integerAttributeOne", -1,
				&iValue) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(iValue != 12345) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(config_getInteger(&config, "section.one", "integerAttributeTwo", -1,
				&iValue) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(iValue != 67890) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	// test double type-casting

	if(config_getDouble(&config, "section.one", "doubleAttributeOne", -1.0,
				&dValue) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(dValue != 12345.12345) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(config_getDouble(&config, "section.one", "doubleAttributeTwo", -1.0,
				&dValue) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(dValue != 67890.06789) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	// test strings

	if(config_getString(&config, "section.one", "testAttributeOne", "unknown",
				&sValue) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(strcmp(sValue, "testValueOne")) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(config_getString(&config, "section.one", "testAttributeTwo", "unknown",
				&sValue) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(strcmp(sValue, "testValueTwo")) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(config_getString(&config, "section.one", "testAttributeThree", "unknown",
				&sValue) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(strcmp(sValue, "testValueThree")) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(config_getString(&config, "section.one", "testAttributeFour", "unknown",
				&sValue) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(strcmp(sValue, "   this is a test attribute value #4   ")) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	// cleanup

	if(config_free(&config) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t ...PASSED\n");

	return 0;
}

