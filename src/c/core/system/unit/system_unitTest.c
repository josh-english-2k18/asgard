/*
 * system_unitTest.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard system-commands library, unit test.
 *
 * Written by Josh English.
 */

// preprocessor directives

#define _CORE_COMMON_BYPASS_STD_STREAM
#include "core/core.h"

// define system unit test constants

#define FILENAME						"assets\\data\\test\\system.testfile"

#if defined(__linux__) || defined(__APPLE__)

#define EXECUTE_TARGET					"ls"
#define EXECUTE_ARGS					"-l"

#elif defined(WIN32)

#define EXECUTE_TARGET					"unit\\config_unitTest.exe"
#define EXECUTE_ARGS					"unit\\config_unitTest.exe /all"

#else // - no plugin available -

#error "No plugin available for this operating system."

#endif // - plugins -

#define DIRNAME							"assets\\data\\test"

#define DIRNAME_CREATE					"assets/data/test/testDir"

// declare system unit test private functions

static int system_basicFunctionalityTest();

// main function

int main()
{
	signal_registerDefault();

	printf("[unit] system unit test (Asgard Ver %s on %s)\n", ASGARD_VERSION,
			ASGARD_DATE);

	if(system_basicFunctionalityTest() < 0) {
		printf("[unit]\t ...FAILED\n");
		return 1;
	}

	printf("[unit] ...PASSED\n");

	return 0;
}

// define system unit test private functions

static int system_basicFunctionalityTest()
{
	aboolean exists = afalse;
	alint fileLength = 0;
	double timestamp = 0.0;
	char *filename = NULL;
	char *compareFilename = NULL;
	char *sourceFilename = NULL;
	char *destFilename = NULL;

	printf("[unit]\t system basic functionality test...\n");

	// test filename conversion to localized name

#if defined(__linux__) || defined(__APPLE__)
	compareFilename = "assets/data/test/system.testfile";
#elif defined(WIN32)
	compareFilename = "assets\\data\\test\\system.testfile";
#else // - no known system -
	printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n", __LINE__);
	return -1;
#endif // - systems -

	filename = (char *)malloc(sizeof(char) * 1024);
	strcpy(filename, FILENAME);

	if(system_fileSetNative(filename) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(strcmp(filename, compareFilename)) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t\t local file name is: '%s'\n", filename);

	free(filename);

	// test file-exists

	if((system_fileExists(FILENAME, &exists) < 0) || (!exists)) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if((system_fileExists("qwertyqwertyqwerty", &exists) < 0) || (exists)) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	// test file length

	if(system_fileLength(FILENAME, &fileLength) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(fileLength != 9342) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t\t file '%s' file length: %lli bytes\n", FILENAME,
			fileLength);

	// test file modification time

	if(system_fileModifiedTime(FILENAME, &timestamp) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t\t file '%s' modified %i seconds ago\n", FILENAME,
			time_getElapsedSeconds((int)timestamp));

	// test file copy

	destFilename = (char *)malloc(sizeof(char) * 1024);
	strcpy(destFilename, FILENAME);
	strcat(destFilename, ".new");

	if(system_fileCopy(FILENAME, destFilename) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	free(destFilename);

	// test file move

	sourceFilename = (char *)malloc(sizeof(char) * 1024);
	strcpy(sourceFilename, FILENAME);
	strcat(sourceFilename, ".new");

	destFilename = (char *)malloc(sizeof(char) * 1024);
	strcpy(destFilename, FILENAME);
	strcat(destFilename, ".next");

	if(system_fileMove(sourceFilename, destFilename) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	// test file deletion

	if(system_fileDelete(destFilename) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	free(sourceFilename);
	free(destFilename);

	// test file execution

	if(system_fileExecute(EXECUTE_TARGET, EXECUTE_ARGS) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	// test dir exists

	if((system_dirExists(DIRNAME, &exists) < 0) || (!exists)) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if((system_dirExists("qwertyqwertyqwerty", &exists) < 0) || (exists)) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	// test change dir

	if(system_dirChangeTo(DIRNAME) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(system_dirChangeTo("../../..") < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if((system_fileExists(FILENAME, &exists) < 0) || (!exists)) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	// test create dir

	if(system_dirCreate(DIRNAME_CREATE) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(system_dirChangeTo(DIRNAME_CREATE) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(system_dirChangeTo("../../../..") < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if((system_fileExists(FILENAME, &exists) < 0) || (!exists)) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	// test delete dir

	if(system_dirDelete(DIRNAME_CREATE) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t ...PASSED\n");

	return 0;
}

