/*
 * zlib_unitTest.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The zLib library wrapper API, unit test.
 *
 * Written by Josh English.
 */

// preprocessor directives

#define _CORE_COMMON_BYPASS_STD_STREAM
#include "core/core.h"


// define zlib unit test constants

#define BASIC_FUNCTIONALITY_TEST_LENGTH					4096


// declare zlib unit test functions

static int zlib_basicFunctionalityTest();

// main function


int main()
{
	signal_registerDefault();

	printf("[unit] zlib unit test (Asgard Ver %s on %s)\n",
			ASGARD_VERSION, ASGARD_DATE);

	if(zlib_basicFunctionalityTest() < 0) {
		printf("[unit]\t ...FAILED\n");
		return 1;
	}

	printf("[unit] ...PASSED\n");

	return 0;
}


// define zlib unit test functions

static int zlib_basicFunctionalityTest()
{
	int ii = 0;
	int nn = 0;
	int rc = 0;
	int inputLength = 0;
	int outputLength = 0;
	int inflateLength = 0;
	int failureCount = 0;
	int totalBytes = 0;
	int totalCompressedBytes = 0;
	char *input = NULL;
	char *duplicate = NULL;
	char *output = NULL;
	char *inflate = NULL;

	printf("[unit]\t zlib basic functionality test...\n");

	system_pickRandomSeed();

	for(ii = 0; ii < BASIC_FUNCTIONALITY_TEST_LENGTH; ii++) {
		inputLength = ((rand() % 65536) + 1024);
		input = (char *)malloc(sizeof(char) * inputLength);

		for(nn = 0; nn < inputLength; nn++) {
			input[nn] = 'a';
		}

		rc = zlib_compress(input, inputLength, ZLIB_COMPRESSION_BEST,
				&output, &outputLength);
		if(rc != 0) {
			printf("[unit]\t\t ...ERROR, failed at line %i with '%s', "
					"aborting.\n", __LINE__, zlib_errorToString(rc));
			return -1;
		}

		rc = zlib_inflate(output, outputLength, &inflate, &inflateLength);
		if(rc != 0) {
			printf("[unit]\t\t ...ERROR, failed at line %i with '%s', "
					"aborting.\n", __LINE__, zlib_errorToString(rc));
			return -1;
		}
		if((inflate == NULL) || (inflateLength != inputLength)) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		for(nn = 0; nn < inputLength; nn++) {
			if((input[nn] != 'a') || (inflate[nn] != input[nn])) {
				printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
						__LINE__);
				return -1;
			}
		}

		totalBytes += inputLength;
		totalCompressedBytes += outputLength;

		free(input);
		free(output);
		free(inflate);
	}

	printf("[unit]\t\t ...compressed %i bytes to %i (%0.02f %%)\n",
			totalBytes, totalCompressedBytes,
			(100.0 -
			 (((double)totalCompressedBytes / (double)totalBytes) * 100.0)));

	failureCount = 0;
	totalBytes = 0;
	totalCompressedBytes = 0;

	for(ii = 0; ii < BASIC_FUNCTIONALITY_TEST_LENGTH; ii++) {
		input = NULL;
		inputLength = 0;
		duplicate = NULL;
		output = NULL;
		outputLength = 0;
		inflate = NULL;
		inflateLength = 0;

		inputLength = ((rand() % 65536) + 1024);
		input = (char *)malloc(sizeof(char) * inputLength);

		for(nn = 0; nn < inputLength; nn++) {
			if((rand() % 100) > 50) {
				input[nn] = (char)(rand() % 256);
			}
			else {
				input[nn] = 'x';
			}
		}

		duplicate = strndup(input, inputLength);

		rc = zlib_compress(input, inputLength, ZLIB_COMPRESSION_FASTEST,
				&output, &outputLength);
		if((rc != 0) && (rc != ZLIB_ERROR_COMPRESSION_FAILED)) {
			printf("[unit]\t\t ...ERROR, failed at line %i with '%s', "
					"aborting.\n", __LINE__, zlib_errorToString(rc));
			return -1;
		}

		if(rc == ZLIB_ERROR_COMPRESSION_FAILED) {
			failureCount++;
		}
		else {
			rc = zlib_inflate(output, outputLength, &inflate, &inflateLength);
			if(rc != 0) {
				printf("[unit]\t\t ...ERROR, failed at line %i with '%s', "
						"aborting.\n", __LINE__, zlib_errorToString(rc));
				return -1;
			}
			if((inflate == NULL) || (inflateLength != inputLength)) {
				printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
						__LINE__);
				return -1;
			}
		}

		for(nn = 0; nn < inputLength; nn++) {
			if((input[nn] != duplicate[nn]) ||
					((inflate != NULL) && (inflate[nn] != input[nn]))) {
				printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
						__LINE__);
				return -1;
			}
		}

		totalBytes += inputLength;
		totalCompressedBytes += outputLength;

		free(input);
		free(duplicate);
		if(output != NULL) {
			free(output);
		}
		if(inflate != NULL) {
			free(inflate);
		}
	}

	printf("[unit]\t\t ...had %i failures\n", failureCount);

	printf("[unit]\t\t ...compressed %i bytes to %i (%0.02f %%)\n",
			totalBytes, totalCompressedBytes,
			(100.0 -
			 (((double)totalCompressedBytes / (double)totalBytes) * 100.0)));

	failureCount = 0;
	totalBytes = 0;
	totalCompressedBytes = 0;

	for(ii = 0; ii < BASIC_FUNCTIONALITY_TEST_LENGTH; ii++) {
		input = NULL;
		inputLength = 0;
		duplicate = NULL;
		output = NULL;
		outputLength = 0;
		inflate = NULL;
		inflateLength = 0;

		inputLength = ((rand() % 65536) + 1024);
		input = (char *)malloc(sizeof(char) * inputLength);

		if((rand() % 100) > 50) {
			for(nn = 0; nn < inputLength; nn++) {
				if((rand() % 100) > 50) {
					input[nn] = (char)((rand() % 94) + 32);
				}
				else {
					input[nn] = (char)(rand() % 256);
				}
			}
		}
		else {
			for(nn = 0; nn < inputLength; nn++) {
				input[nn] = (char)(rand() % 256);
			}
		}

		duplicate = strndup(input, inputLength);

		rc = zlib_compress(input, inputLength, ZLIB_COMPRESSION_FASTEST,
				&output, &outputLength);
		if((rc != 0) && (rc != ZLIB_ERROR_COMPRESSION_FAILED)) {
			printf("[unit]\t\t ...ERROR, failed at line %i with '%s'.\n",
					__LINE__, zlib_errorToString(rc));
		}

		if(rc != 0) {
			failureCount++;
		}
		else {
			rc = zlib_inflate(output, outputLength, &inflate, &inflateLength);
			if(rc != 0) {
				printf("[unit]\t\t ...ERROR, failed at line %i with '%s', "
						"aborting.\n", __LINE__, zlib_errorToString(rc));
				return -1;
			}
			if((inflate == NULL) || (inflateLength != inputLength)) {
				printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
						__LINE__);
				return -1;
			}
		}

		for(nn = 0; nn < inputLength; nn++) {
			if((input[nn] != duplicate[nn]) ||
					((inflate != NULL) && (inflate[nn] != input[nn]))) {
				printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
						__LINE__);
				return -1;
			}
		}

		totalBytes += inputLength;
		totalCompressedBytes += outputLength;

		free(input);
		free(duplicate);
		if(output != NULL) {
			free(output);
		}
		if(inflate != NULL) {
			free(inflate);
		}
	}

	printf("[unit]\t\t ...had %i failures\n", failureCount);

	printf("[unit]\t\t ...compressed %i bytes to %i (%0.02f %%)\n",
			totalBytes, totalCompressedBytes,
			(100.0 -
			 (((double)totalCompressedBytes / (double)totalBytes) * 100.0)));

	printf("[unit]\t ...PASSED\n");

	return 0;
}

