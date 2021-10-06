/*
 * lfbc_unitTest.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Low-Fragmentation Block-Cache (LFBC) is a block-based file cache
 * including a background-thread management system, unit test.
 *
 * Written by Josh English.
 */

// preprocessor directives

#define _CORE_COMMON_BYPASS_STD_STREAM
#include "core/core.h"


// define low-fragmentation block-cache unit test constants

#define TESTING_LOG_LEVEL						LOG_LEVEL_INFO

#define UNIT_TEST_FILENAME						"lfbc.test.file"
#define UNIT_TEST_LENGTH						128
//#define UNIT_TEST_LENGTH						65536

#define BASIC_UNIT_TEST_MIN_BLOCK_LENGTH		1024
//#define BASIC_UNIT_TEST_MAX_BLOCK_LENGTH		65536
#define BASIC_UNIT_TEST_MAX_BLOCK_LENGTH		262144


// define low-fragmentation block-cache unit test data types

typedef enum _UnitTestResult {
	UTR_START_OF_LIST,
	UTR_TEST_PASSED,
	UTR_TEST_FAILED,
	UTR_TEST_NOT_SUPPORTED,
	UTR_END_OF_LIST
} UnitTestResult;

typedef struct _UnitTest {
	char *title;
	UnitTestResult (*function)();
	int log_level;
} UnitTest;

typedef struct _options {
	int log_level;
	char *log_file;
} Options;


// define low-fragmentation block-cache unit test global variables

Log *gLog = NULL;
Options options;


// declare low-fragmentation block-cache unit test functions

// helper functions

static int handle_options(int argc, char **argv, Options *options);

// unit test functions

static UnitTestResult lfbc_unit_test_basic();

static UnitTestResult lfbc_unit_test_many_files();


// define low-fragmentation block-cache unit tests


UnitTest test_list[] = {
	{	"lfbc_unit_test_basic",			lfbc_unit_test_basic,
			TESTING_LOG_LEVEL												},
	{	"lfbc_unit_test_many_files",	lfbc_unit_test_many_files,
			TESTING_LOG_LEVEL												},
	{	NULL,							NULL,
			TESTING_LOG_LEVEL												}
};


// main function

int main(int argc, char *argv[])
{
	int ii = 0;
	int testTotal = 0;
	int testsPassed = 0;
	int testsFailed = 0;
	int testsNotSupported = 0;
	int testsUnknown = 0;
	char *resultString = NULL;
	double elapsedTime = 0.0;
	double timer = 0.0;

	UnitTestResult testResult = UTR_TEST_PASSED;
	UnitTestResult finalResult = UTR_TEST_PASSED;

	signal_registerDefault();

	// handle input options

	if(handle_options(argc, argv, &options) < 0) {
		exit(1);
	}

	// setup test logging

	gLog = (Log *)malloc(sizeof(Log));

	if(strcasecmp(options.log_file, "stdout") == 0) {
		log_init(gLog, LOG_OUTPUT_STDOUT, NULL, options.log_level);
	}
	else if(strcasecmp(options.log_file, "stderr") == 0) {
		log_init(gLog, LOG_OUTPUT_STDERR, NULL, options.log_level);
	}
	else {
		log_init(gLog, LOG_OUTPUT_FILE, options.log_file, options.log_level);
	}

	log_logf(gLog, LOG_LEVEL_INFO, "lfbc unit test, using Ver "
			"%s on %s.", ASGARD_VERSION, ASGARD_DATE);

	// run tests

	for(ii = 0; test_list[ii].function != NULL; ii++) {
		testTotal++;

//		log_setLevel(gLog, test_list[ii].log_level);

		log_logf(gLog, LOG_LEVEL_INFO, "");
		log_logf(gLog, LOG_LEVEL_INFO, "Starting test #%i, %s.", ii,
				 test_list[ii].title);

		timer = time_getTimeMus();

		testResult = test_list[ii].function();

		elapsedTime = time_getElapsedMusInSeconds(timer);

		switch(testResult) {
			case UTR_TEST_PASSED:
				resultString = "PASSED";
				testsPassed++;
				break;

			case UTR_TEST_NOT_SUPPORTED:
				resultString = "NOT SUPPORTED";
				testsNotSupported++;
				break;

			case UTR_TEST_FAILED:
				resultString = "FAILED";
				testsFailed++;
				break;

			default:
				resultString = ">>>>> UNKNOWN RESULT <<<<<";
				testsUnknown++;
				break;
		}

		if(testResult != UTR_TEST_PASSED) {
			finalResult = UTR_TEST_FAILED;
		}

		log_logf(gLog, LOG_LEVEL_INFO, "Test #%i, %s (%.6f seconds): %s.",
				 ii, test_list[ii].title, elapsedTime, resultString);
	}

	log_logf(gLog, LOG_LEVEL_INFO, "");

	log_logf(gLog, LOG_LEVEL_INFO, "%i test%s run:", testTotal,
			 (testTotal == 1) ? "" : "s");

	log_logf(gLog, LOG_LEVEL_INFO, "   %i test%s passed", testsPassed,
			 (testsPassed == 1) ? "" : "s");

	if(testsFailed) {
		log_logf(gLog, LOG_LEVEL_INFO, "   %i test%s failed", testsFailed,
				 (testsFailed == 1) ? "" : "s");
	}

	if(testsNotSupported) {
		log_logf(gLog, LOG_LEVEL_INFO, "   %i test%s not supported",
				 testsNotSupported, (testsNotSupported == 1) ? "" : "s");
	}

	if(testsUnknown) {
		log_logf(gLog, LOG_LEVEL_INFO, "   %i test%s with unknown result",
				 testsUnknown, (testsUnknown == 1) ? "" : "s");
	}

	log_logf(gLog, LOG_LEVEL_INFO, "");
	log_logf(gLog, LOG_LEVEL_INFO, "stopping logger");

	// cleanup

	log_free(gLog);
	free(gLog);

	free(options.log_file);

	if(finalResult == UTR_TEST_PASSED) {
		return 0;
	}

	return 1;
}


// define low-fragmentation block-cache unit test functions

// helper functions

static int handle_options(int argc, char **argv, Options *options) 
{
	int opt = 0;
	int abortflag = 0;

	options->log_file = NULL;
	options->log_level = LOG_LEVEL_INFO;

	while((opt = getopt(argc, argv, "l:o:?")) != -1) {
		switch(opt) {
			case 'l':// log level
				if(strcasecmp(optarg, "ERROR") == 0) {
					options->log_level = LOG_LEVEL_ERROR;
				}
				else if(strcasecmp(optarg, "WARNING") == 0) {
					options->log_level = LOG_LEVEL_WARNING;
				}
				else if(strcasecmp(optarg, "INFO") == 0) {
					options->log_level = LOG_LEVEL_INFO;
				}
				else if(strcasecmp(optarg, "DEBUG") == 0) {
					options->log_level = LOG_LEVEL_DEBUG;
				}
				else {
					fprintf(stderr, "unknown log level '%s'\n", optarg);
					abortflag = -1;
				}
				break;
			case 'o':	// output
				options->log_file = strdup(optarg);
				break;
			case '?':	// help
				abortflag = -1;
				break;
			default:
				fprintf(stderr, "illegal option -- %c\n", opt);
				abortflag = -1;
				break;
		}
	}

	if(options->log_file == NULL) {
		options->log_file = strdup("stdout");
	}


	if(abortflag == -1) {
		fprintf(stderr,
				"usage: %s [-l log_level] [-o output_file]"
				"\n",
				argv[0]);
	}

	return abortflag;
}

static void buildRandomBlock(char *block, int block_length)
{
	int ii = 0;

	for(ii = 0; ii < block_length; ii++) {
		block[ii] = (char)(rand() % 256);
	}
}

static int fileError(FileHandle *fh, char *type, int rc)
{
	char *errorMessage = NULL;

	errorMessage = file_getErrorMessage(fh, rc);

	log_logf(gLog, LOG_LEVEL_ERROR,
			 "failed to execute '%s()' on '%s' with '%s'.",
			type, fh->filename, errorMessage);

	free(errorMessage);

	return -1;
}

static int createTestFile(int block_length, char *filename,
		aboolean deleteFile)
{
	aboolean exists = afalse;
	int ii = 0;
	int rc = 0;
	char *block = NULL;

	FileHandle fh;

	system_pickRandomSeed();

	if(deleteFile) {
		if((system_fileExists(filename, &exists) == 0) && (exists)) {
			system_fileDelete(filename);
		}
	}

	if((rc = file_init(&fh, filename, "wb", block_length)) < 0) {
		return fileError(&fh, "open", rc);
	}

	block = (char *)malloc(sizeof(char) * (block_length + 1));

	for(ii = 0; ii < UNIT_TEST_LENGTH; ii++) {
		if((ii + 1) >= UNIT_TEST_LENGTH) {
			block_length = ((rand() % block_length) + 1);
		}

		buildRandomBlock(block, block_length);

		if((rc = file_write(&fh, block, block_length)) < 0) {
			return fileError(&fh, "write", rc);
		}
	}

	free(block);

	file_free(&fh);

	return 0;
}

// unit test functions

static UnitTestResult lfbc_unit_test_many_files()
{
	int ii = 0;
	int nn = 0;
	int rc = 0;
	int block_length = 16384;
	int readBlockLength = 0;
	int cacheBlockLength = 0;
	int maxOpenFileHandles = 0;
	alint alpha = 0;
	alint beta = 0;
	alint blockId = 0;
	alint fileLength = 0;
	alint filePosition = 0;
	alint tempFileLength = 0;
	double timer = 0.0;
	double totalCacheTime = 0.0;
	double totalReadTime = 0.0;
	char *mask = "%s.%04i";
	char *cacheBlock = NULL;
	char block[(BASIC_UNIT_TEST_MAX_BLOCK_LENGTH + 1)];
	char filename[512];

	UnitTestResult final = UTR_TEST_PASSED;

	FileHandle fh;
	Lfbc cache;

	maxOpenFileHandles = 40;

	// build test files

	for(ii = 0; ii < (2 * maxOpenFileHandles); ii++) {
		snprintf(filename, sizeof(filename), mask, UNIT_TEST_FILENAME, ii);

		if(createTestFile(block_length, filename, atrue) < 0) {
			log_logf(gLog, LOG_LEVEL_ERROR,
					 "could not create file %s at line %i, aborting",
					 filename, __LINE__);
			return UTR_TEST_FAILED;
		}
	}

	// initialize the low-fragmentation block-cache

	lfbc_init(&cache, gLog);

	lfbc_setMaxOpenFileHandles(&cache, maxOpenFileHandles);
	lfbc_setBlockLength(&cache, block_length);

	if(lfbc_start(&cache) < 0) {
		log_logf(gLog, LOG_LEVEL_ERROR,
				 "failed to start cache at line %i, aborting", __LINE__);
		return UTR_TEST_FAILED;
	}

	// perform a read-comparison pass

	for(ii = 0; ii < (2 * maxOpenFileHandles); ii++) {
		snprintf(filename, sizeof(filename), mask, UNIT_TEST_FILENAME, ii);

		if((rc = file_init(&fh, filename, "rb", block_length)) < 0) {
			log_logf(gLog, LOG_LEVEL_ERROR,
					 "failed to open file at line %i, aborting", __LINE__);
			fileError(&fh, "open", rc);
			return UTR_TEST_FAILED;
		}

		if((rc = file_getFileLength(&fh, &fileLength)) < 0) {
			log_logf(gLog, LOG_LEVEL_ERROR,
					 "failed to get file size at line %i, aborting",
					 __LINE__);
			fileError(&fh, "file_size64", (int)rc);
			return UTR_TEST_FAILED;
		}

		filePosition = 0;
		totalReadTime = 0.0;
		totalCacheTime = 0.0;

		for(nn = 0; nn < UNIT_TEST_LENGTH; nn++) {
			if(!lfbc_hasCache(&cache, filename, &alpha, &beta)) {
				log_logf(gLog, LOG_LEVEL_ERROR,
						 "failed to get cache value at line %i, aborting",
						 __LINE__);
				return UTR_TEST_FAILED;
			}

			file_getFileLength(&fh, &tempFileLength);

			if((alpha != tempFileLength) || (beta != UNIT_TEST_LENGTH)) {
				log_logf(gLog, LOG_LEVEL_ERROR,
						 "failed to get file size at line %i, aborting",
						 __LINE__);
				return UTR_TEST_FAILED;
			}

			timer = time_getTimeMus();

			readBlockLength = block_length;
			if((fileLength - (alint)readBlockLength) < 0) {
				readBlockLength = (int)fileLength;
			}

			if((rc = file_read(&fh, block, readBlockLength)) < 0) {
				log_logf(gLog, LOG_LEVEL_ERROR,
						 "failed to read file at line %i, aborting", __LINE__);
				fileError(&fh, "read", rc);
				return UTR_TEST_FAILED;
			}
			else if(rc != readBlockLength) {
				log_logf(gLog, LOG_LEVEL_ERROR,
						 "failed to read right number of bytes at line %i, "
						 "aborting", __LINE__);
				return UTR_TEST_FAILED;
			}

			totalReadTime += time_getElapsedMusInSeconds(timer);
			timer = time_getTimeMus();

			cacheBlock = lfbc_get(&cache, filename, filePosition,
					&cacheBlockLength, &blockId);
			if((cacheBlock == NULL) || (cacheBlockLength != readBlockLength)) {
				log_logf(gLog, LOG_LEVEL_ERROR,
						 "failed at %i { cache-block 0x%lx, "
						 "lengths %i vs %i }, aborting", __LINE__,
						 (unsigned long int)cacheBlock, cacheBlockLength,
						 readBlockLength);
				return UTR_TEST_FAILED;
			}

			totalCacheTime += time_getElapsedMusInSeconds(timer);

			if(memcmp(block, cacheBlock, cacheBlockLength)) {
				log_logf(gLog, LOG_LEVEL_ERROR,
						 "failed to validate block at line %i, aborting",
						 __LINE__);
				return UTR_TEST_FAILED;
			}

			if(lfbc_releaseBlock(&cache, filename, blockId) < 0) {
				log_logf(gLog, LOG_LEVEL_ERROR,
						 "failed to release block at line %i, aborting",
						 __LINE__);
				return UTR_TEST_FAILED;
			}

			fileLength -= (alint)readBlockLength;
			filePosition += (alint)readBlockLength;
		}

		file_free(&fh);
	}

	// rebuild test files

	for(ii = 0; ii < (2 * maxOpenFileHandles); ii++) {
		snprintf(filename, sizeof(filename), mask, UNIT_TEST_FILENAME, ii);
		if(createTestFile(block_length, filename, afalse) < 0) {
			log_logf(gLog, LOG_LEVEL_ERROR,
					 "could not create file %s at line %i, aborting",
					 filename, __LINE__);
			return UTR_TEST_FAILED;
		}
	}

	// perform a read-comparison pass

	for(ii = 0; ii < (2 * maxOpenFileHandles); ii++) {
		snprintf(filename, sizeof(filename), mask, UNIT_TEST_FILENAME, ii);

		if((rc = file_init(&fh, filename, "rb", block_length)) < 0) {
			log_logf(gLog, LOG_LEVEL_ERROR,
					 "failed to open file at line %i, aborting", __LINE__);
			fileError(&fh, "open", rc);
			return UTR_TEST_FAILED;
		}

		if((rc = file_getFileLength(&fh, &fileLength)) < 0) {
			log_logf(gLog, LOG_LEVEL_ERROR,
					 "failed to get file size at line %i, aborting",
					 __LINE__);
			fileError(&fh, "file_size64", (int)rc);
			return UTR_TEST_FAILED;
		}

		filePosition = 0;
		totalReadTime = 0.0;
		totalCacheTime = 0.0;

		for(nn = 0; nn < UNIT_TEST_LENGTH; nn++) {
			if(!lfbc_hasCache(&cache, filename, &alpha, &beta)) {
				log_logf(gLog, LOG_LEVEL_ERROR,
						 "failed to get cache value at line %i, aborting",
						 __LINE__);
				return UTR_TEST_FAILED;
			}

			file_getFileLength(&fh, &tempFileLength);

			if((alpha != tempFileLength) || (beta != UNIT_TEST_LENGTH)) {
				log_logf(gLog, LOG_LEVEL_ERROR,
						 "failed to get file size at line %i, aborting",
						 __LINE__);
				return UTR_TEST_FAILED;
			}

			timer = time_getTimeMus();

			readBlockLength = block_length;
			if((fileLength - (alint)readBlockLength) < 0) {
				readBlockLength = (int)fileLength;
			}

			if((rc = file_read(&fh, block, readBlockLength)) < 0) {
				log_logf(gLog, LOG_LEVEL_ERROR,
						 "failed to read file at line %i, aborting", __LINE__);
				fileError(&fh, "read", rc);
				return UTR_TEST_FAILED;
			}
			else if(rc != readBlockLength) {
				log_logf(gLog, LOG_LEVEL_ERROR,
						 "failed to read right number of bytes at line %i, "
						 "aborting", __LINE__);
				return UTR_TEST_FAILED;
			}

			totalReadTime += time_getElapsedMusInSeconds(timer);
			timer = time_getTimeMus();

			cacheBlock = lfbc_get(&cache, filename, filePosition,
					&cacheBlockLength, &blockId);
			if((cacheBlock == NULL) || (cacheBlockLength != readBlockLength)) {
				log_logf(gLog, LOG_LEVEL_ERROR,
						 "failed at %i { cache-block 0x%lx, "
						 "lengths %i vs %i }, aborting", __LINE__,
						 (unsigned long int)cacheBlock, cacheBlockLength,
						 readBlockLength);
				return UTR_TEST_FAILED;
			}

			totalCacheTime += time_getElapsedMusInSeconds(timer);

			if(memcmp(block, cacheBlock, cacheBlockLength)) {
				log_logf(gLog, LOG_LEVEL_ERROR,
						 "failed to validate block at line %i, aborting",
						 __LINE__);
				return UTR_TEST_FAILED;
			}

			if(lfbc_releaseBlock(&cache, filename, blockId) < 0) {
				log_logf(gLog, LOG_LEVEL_ERROR,
						 "failed to release block at line %i, aborting",
						 __LINE__);
				return UTR_TEST_FAILED;
			}

			fileLength -= (alint)readBlockLength;
			filePosition += (alint)readBlockLength;
		}

		file_free(&fh);
	}

	// cleanup

	lfbc_free(&cache);

	for(ii = 0; ii < (2 * maxOpenFileHandles); ii++) {
		snprintf(filename, sizeof(filename), mask, UNIT_TEST_FILENAME, ii);
		system_fileDelete(filename);
	}

	return final;
}

static UnitTestResult lfbc_unit_test_basic()
{
	int ii = 0;
	int rc = 0;
	int blockLength = 0;
	int readBlockLength = 0;
	int cacheBlockLength = 0;
	alint alpha = 0;
	alint beta = 0;
	alint blockId = 0;
	alint fileLength = 0;
	alint filePosition = 0;
	alint tempFileLength = 0;
	double timer = 0.0;
	double totalReadTime = 0.0;
	double totalCacheTime = 0.0;
	char *cacheBlock = NULL;
	char block[(BASIC_UNIT_TEST_MAX_BLOCK_LENGTH + 1)];

	UnitTestResult final = UTR_TEST_PASSED;

	FileHandle fh;
	Lfbc cache;

	for(blockLength = BASIC_UNIT_TEST_MIN_BLOCK_LENGTH;
			blockLength <= BASIC_UNIT_TEST_MAX_BLOCK_LENGTH;
			blockLength *= 2) {
		// create a test file for the current block length

		log_logf(gLog, LOG_LEVEL_INFO, "test w/block-length: %i", blockLength);

		timer = time_getTimeMus();

		if(createTestFile(blockLength, UNIT_TEST_FILENAME, atrue) < 0) {
			log_logf(gLog, LOG_LEVEL_ERROR,
					 "could not create file at line %i, aborting", __LINE__);
			return UTR_TEST_FAILED;
		}

		log_logf(gLog, LOG_LEVEL_INFO,
				 "created test file (length %i) in %0.6f seconds",
				file_getFileLengthOnFilename(UNIT_TEST_FILENAME),
				time_getElapsedMusInSeconds(timer));

		// initialize the low-fragmentation block-cache

		lfbc_init(&cache, gLog);

		lfbc_setBlockLength(&cache, blockLength);

		if(lfbc_start(&cache) < 0) {
			log_logf(gLog, LOG_LEVEL_ERROR,
					 "failed to start cache at line %i, aborting", __LINE__);
			return UTR_TEST_FAILED;
		}

		// perform the 1st read-comparison pass

		if((rc = file_init(&fh, UNIT_TEST_FILENAME, "rb", blockLength)) < 0) {
			log_logf(gLog, LOG_LEVEL_ERROR,
					 "failed to open file at line %i, aborting", __LINE__);
			fileError(&fh, "open", rc);
			return UTR_TEST_FAILED;
		}

		if((rc = file_getFileLength(&fh, &fileLength)) < 0) {
			log_logf(gLog, LOG_LEVEL_ERROR,
					 "failed to get file size at line %i, aborting",
					 __LINE__);
			fileError(&fh, "file_size64", rc);
			return UTR_TEST_FAILED;
		}

		log_logf(gLog, LOG_LEVEL_DEBUG, "unit-test file length is %lli",
				fileLength);

		filePosition = 0;
		totalReadTime = 0.0;
		totalCacheTime = 0.0;

		for(ii = 0; ii < UNIT_TEST_LENGTH; ii++) {
			if(!lfbc_hasCache(&cache, UNIT_TEST_FILENAME, &alpha,
						&beta)) {
				log_logf(gLog, LOG_LEVEL_ERROR,
						 "failed to get cache value at line %i, aborting",
						 __LINE__);
				return UTR_TEST_FAILED;
			}

			file_getFileLength(&fh, &tempFileLength);

			if((alpha != tempFileLength) || (beta != UNIT_TEST_LENGTH)) {
				log_logf(gLog, LOG_LEVEL_ERROR,
						 "failed to get file size at line %i, aborting",
						 __LINE__);
				return UTR_TEST_FAILED;
			}

			timer = time_getTimeMus();

			readBlockLength = blockLength;
			if((fileLength - (alint)readBlockLength) < 0) {
				readBlockLength = (int)fileLength;
			}

			if((rc = file_read(&fh, block, readBlockLength)) < 0) {
				log_logf(gLog, LOG_LEVEL_ERROR,
						 "failed to read file at line %i, aborting", __LINE__);
				fileError(&fh, "read", rc);
				return UTR_TEST_FAILED;
			}
			else if(rc != readBlockLength) {
				log_logf(gLog, LOG_LEVEL_ERROR,
						 "failed to read right number of bytes at line %i, "
						 "aborting", __LINE__);
				return UTR_TEST_FAILED;
			}

			totalReadTime += time_getElapsedMusInSeconds(timer);
			timer = time_getTimeMus();

			cacheBlock = lfbc_get(&cache, UNIT_TEST_FILENAME, filePosition,
					&cacheBlockLength, &blockId);
			if((cacheBlock == NULL) || (cacheBlockLength != readBlockLength)) {
				log_logf(gLog, LOG_LEVEL_ERROR,
						 "failed at %i { cache-block 0x%lx, "
						"lengths %i vs %i }, aborting", __LINE__,
						(unsigned long int)cacheBlock, cacheBlockLength,
						readBlockLength);
				return UTR_TEST_FAILED;
			}

			totalCacheTime += time_getElapsedMusInSeconds(timer);

			if(memcmp(block, cacheBlock, cacheBlockLength)) {
				log_logf(gLog, LOG_LEVEL_ERROR,
						 "failed to validate block at line %i, aborting",
						 __LINE__);
				return UTR_TEST_FAILED;
			}

			if(lfbc_releaseBlock(&cache, UNIT_TEST_FILENAME, blockId) < 0) {
				log_logf(gLog, LOG_LEVEL_ERROR,
						 "failed to release block at line %i, aborting",
						 __LINE__);
				return UTR_TEST_FAILED;
			}

			fileLength -= (alint)readBlockLength;
			filePosition += (alint)readBlockLength;
		}

		file_free(&fh);

		log_logf(gLog, LOG_LEVEL_INFO,
				 "first-pass avg read time  : %0.6f (%0.6f total)",
				(totalReadTime / (double)UNIT_TEST_LENGTH), totalReadTime);
		log_logf(gLog, LOG_LEVEL_INFO,
				 "first-pass avg cache time : %0.6f (%0.6f total)",
				(totalCacheTime / (double)UNIT_TEST_LENGTH), totalCacheTime);

		// perform the 2nd read-comparison pass

		if((rc = file_init(&fh, UNIT_TEST_FILENAME, "rb", blockLength)) < 0) {
			log_logf(gLog, LOG_LEVEL_ERROR,
					 "failed to open file at line %i, aborting", __LINE__);
			fileError(&fh, "open", rc);
			return UTR_TEST_FAILED;
		}

		if((rc = file_getFileLength(&fh, &fileLength)) < 0) {
			log_logf(gLog, LOG_LEVEL_ERROR,
					 "failed to get file length at line %i, aborting",
					 __LINE__);
			fileError(&fh, "file_getFileLength", rc);
			return UTR_TEST_FAILED;
		}

		filePosition = 0;
		totalReadTime = 0.0;
		totalCacheTime = 0.0;

		for(ii = 0; ii < UNIT_TEST_LENGTH; ii++) {
			timer = time_getTimeMus();

			readBlockLength = blockLength;
			if((fileLength - (alint)readBlockLength) < 0) {
				readBlockLength = (int)fileLength;
			}

			if((rc = file_read(&fh, block, readBlockLength)) < 0) {
				log_logf(gLog, LOG_LEVEL_ERROR,
						 "failed to read file at line %i, aborting", __LINE__);
				fileError(&fh, "read", rc);
				return UTR_TEST_FAILED;
			}
			else if(rc != readBlockLength) {
				log_logf(gLog, LOG_LEVEL_ERROR,
						 "failed to read right number of bytes at line %i, "
						 "aborting", __LINE__);
				return UTR_TEST_FAILED;
			}

			totalReadTime += time_getElapsedMusInSeconds(timer);
			timer = time_getTimeMus();

			cacheBlock = lfbc_get(&cache, UNIT_TEST_FILENAME, filePosition,
					&cacheBlockLength, &blockId);
			if((cacheBlock == NULL) ||
					(cacheBlockLength != readBlockLength)) {
				log_logf(gLog, LOG_LEVEL_ERROR,
						 "failed at %i { cache-block 0x%lx, "
						"lengths %i vs %i }, aborting", __LINE__,
						(unsigned long int)cacheBlock, cacheBlockLength,
						readBlockLength);
				return UTR_TEST_FAILED;
			}

			totalCacheTime += time_getElapsedMusInSeconds(timer);

			if(memcmp(block, cacheBlock, cacheBlockLength)) {
				log_logf(gLog, LOG_LEVEL_ERROR,
						 "failed to validate block at line %i, aborting",
						 __LINE__);
				return UTR_TEST_FAILED;
			}

			if(lfbc_releaseBlock(&cache, UNIT_TEST_FILENAME, blockId) < 0) {
				log_logf(gLog, LOG_LEVEL_ERROR,
						 "failed to release block at line %i, aborting",
						 __LINE__);
				return UTR_TEST_FAILED;
			}

			fileLength -= (alint)readBlockLength;
			filePosition += (alint)readBlockLength;
		}

		file_free(&fh);

		log_logf(gLog, LOG_LEVEL_INFO,
				 "second-pass avg read time  : %0.6f (%0.6f total)",
				(totalReadTime / (double)UNIT_TEST_LENGTH), totalReadTime);
		log_logf(gLog, LOG_LEVEL_INFO,
				 "second-pass avg cache time : %0.6f (%0.6f total)",
				(totalCacheTime / (double)UNIT_TEST_LENGTH), totalCacheTime);

		// free the low-fragmentation block-cache

		log_logf(gLog, LOG_LEVEL_INFO, "lfbc-memory: %lli bytes",
				 lfbc_getAllocatedMemory(&cache));

		lfbc_free(&cache);
	}

	system_fileDelete(UNIT_TEST_FILENAME);

	return final;
}

