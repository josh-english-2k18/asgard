/*
 * pfs_unitTest.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * A simplified profile-function system (PFS), unit test.
 *
 * Written by Josh English.
 */

// preprocessor directives

#define _CORE_COMMON_BYPASS_STD_STREAM
#include "core/core.h"

// declare pfs unit test constants

#define PFS_BASIC_TEST_FILENAME				"assets/data/test/pfs.testfile.log"

// declare pfs unit tests

static int pfs_basicFunctionalityTest();

// main function

int main()
{
	signal_registerDefault();

	printf("[unit] pfs unit test (Asgard Ver %s on %s)\n",
			ASGARD_VERSION, ASGARD_DATE);

	if(pfs_basicFunctionalityTest() < 0) {
		printf("[unit]\t ...FAILED\n");
		return 1;
	}

	printf("[unit] ...PASSED\n");

	return 0;
}

// define pfs unit tests

static int pfs_testFunction01(Pfs *pfs)
{
	return pfs_profile(pfs, __FILE__, __FUNCTION__);
}

static int pfs_testFunction02(Pfs *pfs)
{
	return pfs_profile(pfs, __FILE__, __FUNCTION__);
}

static int pfs_testFunction03(Pfs *pfs)
{
	return pfs_profile(pfs, __FILE__, __FUNCTION__);
}

static int pfs_basicFunctionalityTest()
{
	aboolean fileExists = afalse;
	int ii = 0;
	int reportLength = 0;
	char *report = NULL;

	Pfs pfs;

	printf("[unit]\t pfs basic functionality test...\n");
	printf("[unit]\t\t pfs test filename: %s\n",
			PFS_BASIC_TEST_FILENAME);

	system_fileExists(PFS_BASIC_TEST_FILENAME, &fileExists);
	if(fileExists) {
		if(system_fileDelete(PFS_BASIC_TEST_FILENAME) < 0) {
			return -1;
		}
		printf("[unit]\t\t ...found and deleted test file '%s'\n",
				PFS_BASIC_TEST_FILENAME);
	}

	if(pfs_init(&pfs, atrue, PFS_BASIC_TEST_FILENAME) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	system_pickRandomSeed();

	for(ii = 0; ii < 8192; ii++) {
		if((rand() % 1024) > 512) {
			if(pfs_profile(&pfs, __FILE__, __FUNCTION__) < 0) {
				printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
						__LINE__);
				return -1;
			}
		}
		if((rand() % 1024) > 512) {
			if(pfs_testFunction01(&pfs) < 0) {
				printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
						__LINE__);
				return -1;
			}
		}
		if((rand() % 1024) > 512) {
			if(pfs_testFunction02(&pfs) < 0) {
				printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
						__LINE__);
				return -1;
			}
		}
		if((rand() % 1024) > 512) {
			if(pfs_testFunction03(&pfs) < 0) {
				printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
						__LINE__);
				return -1;
			}
		}
	}

	if(pfs_buildReport(&pfs, &report, &reportLength) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("%s", report);

	free(report);

	// cleanup

	if(pfs_free(&pfs) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t ...PASSED\n");

	return 0;
}

