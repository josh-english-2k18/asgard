/*
 * file_unitTest.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard core library file-handling functions, unit test.
 *
 * Written by Josh English.
 */

// preprocessor directives

#define _CORE_COMMON_BYPASS_STD_STREAM
#include "core/core.h"

// define file-handling unit test constants

#define TEST01_FILENAME							"assets/data/test/fh.testfile"
#define TEST01_BLOCK_LENGTH						65536
#define TEST01_BUFFER_LENGTH					(TEST01_BLOCK_LENGTH * 2)
#define TEST01_TESTS							4096//16384

#define TEST02_FILENAME							"assets/data/test/fh.testfile"
#define TEST02_BUFFER_LENGTH					262144
#define TEST02_TESTS							4096

#define TEST03_FILENAME							\
	"assets/data/test/fh.readline.testfile"
#define TEST03_BUFFER_LENGTH					65536

// declare file-handling unit test common private functions

static int _error(int code, FileHandle *handle, const char *function,
		size_t line);
static alint getHandlePosition(FileHandle *handle);

// define file-handling unit test macros

#define error(code) _error(code, handle, __FUNCTION__, __LINE__);

// declare file-handling unit tests

static int file_basicFunctionalityTest();
static int file_typesIoTest();
static int file_readLineTest();

// main function

int main()
{
	signal_registerDefault();

	printf("[unit] file-handling unit test (Asgard Ver %s on %s)\n",
			ASGARD_VERSION, ASGARD_DATE);

	if(file_basicFunctionalityTest() < 0) {
		printf("[unit]\t ...FAILED\n");
		return 1;
	}

	if(file_typesIoTest() < 0) {
		printf("[unit]\t ...FAILED\n");
		return 1;
	}

	if(file_readLineTest() < 0) {
		printf("[unit]\t ...FAILED\n");
		return 1;
	}

	printf("[unit] ...PASSED\n");

	return 0;
}

// define file-handling unit test common private functions

static int _error(int code, FileHandle *handle, const char *function,
		size_t line)
{
	int rc = 0;
	char *errorMessage = NULL;

	if(code != 0) {
		rc = file_getError(handle, code, &errorMessage);
	}
	else {
		rc = -1;
	}
	if(rc < 0) {
		fprintf(stderr, "[%s():%i] error(%i) - aborting.\n", function, 
				(int)line, code);
	}
	else {
		fprintf(stderr, "[%s():%i] error(%i) - '%s', aborting.\n", function,
				(int)line, code, errorMessage);
	}

	if(errorMessage != NULL) {
		free(errorMessage);
	}

	return -1;
}

static alint getHandlePosition(FileHandle *handle)
{
	alint result = 0;

	result = ftello(handle->fd);
	if(result < 0) {
		fprintf(stderr, "[%s():%i] error - failed to obtain ftello() from "
				"0x%lx. with '%s'.\n", __FUNCTION__, __LINE__,
				(unsigned long int)handle->fd, strerror(errno));
		exit(1);
	}

	return result;
}

// define file-handling unit tests

static int file_basicFunctionalityTest()
{
	aboolean fileExists = afalse;
	int ii = 0;
	int nn = 0;
	int rc = 0;
	int length = 0;
	int positionCount = 0;
	int syncCount = 0;
	double writeLow = 0.0;
	double writeHigh = 0.0;
	double writeTime = 0.0;
	double positionLow = 0.0;
	double positionHigh = 0.0;
	double positionTime = 0.0;
	double seekLow = 0.0;
	double seekHigh = 0.0;
	double seekTime = 0.0;
	double readLow = 0.0;
	double readHigh = 0.0;
	double readTime = 0.0;
	double syncLow = 0.0;
	double syncHigh = 0.0;
	double syncTime = 0.0;
	double elapsedTime = 0.0;
	double timer = 0.0;
	double totalTimer = 0.0;
	alint position = 0;
	char temp[128];
	char blockOne[TEST01_BLOCK_LENGTH];
	char blockTwo[TEST01_BLOCK_LENGTH];

	FileHandle *handle = NULL;
	FileHandle fh;

	printf("[unit]\t file-handling basic functionality test...\n");
	printf("[unit]\t\t filename      : %s\n", TEST01_FILENAME);
	printf("[unit]\t\t buffer length : %i\n", TEST01_BUFFER_LENGTH);
	printf("[unit]\t\t block length  : %i\n", TEST01_BLOCK_LENGTH);
	printf("[unit]\t\t file length   : %0.0f\n",
			((double)TEST01_BLOCK_LENGTH * (double)TEST01_TESTS));
	printf("[unit]\t\t tests         : %i\n", TEST01_TESTS);

	totalTimer = time_getTimeMus();

	handle = &fh;

	system_fileExists(TEST01_FILENAME, &fileExists);
	if(fileExists) {
		if(system_fileDelete(TEST01_FILENAME) < 0) {
			return error(0);
		}
		printf("[unit]\t\t ...found and deleted test file '%s'\n",
				TEST01_FILENAME);
	}

	if((rc = file_init(&fh, TEST01_FILENAME, "w+b",
					TEST01_BUFFER_LENGTH)) < 0) {
		return error(rc);
	}

	printf("[unit]\t\t executing...");
	fflush(stdout);

	writeLow = MAX_SIGNED_DOUBLE;
	writeHigh = 0.0;
	positionLow = MAX_SIGNED_DOUBLE;
	positionHigh = 0.0;
	seekLow = MAX_SIGNED_DOUBLE;
	seekHigh = 0.0;
	readLow = MAX_SIGNED_DOUBLE;
	readHigh = 0.0;
	syncLow = MAX_SIGNED_DOUBLE;
	syncHigh = 0.0;

	memset(temp, 0, sizeof(temp));

	for(ii = 0; ii < TEST01_TESTS; ii++) {
		if((ii > 0) && ((ii % 8) == 0)) {
			length = (int)strlen(temp);
			for(nn = 0; nn < length; nn++) {
				printf("\b");
			}
			snprintf(temp, sizeof(temp), "%0.2f %%, %0.2f seconds remaining",
					(((double)ii / (double)TEST01_TESTS) * 100.0),
					((time_getElapsedMusInSeconds(totalTimer) /
					  ((double)ii / (double)TEST01_TESTS)) -
					 time_getElapsedMusInSeconds(totalTimer)));
			printf("%s", temp);
			fflush(stdout);
		}

		memset(blockOne, 0, TEST01_BLOCK_LENGTH);
		memset(blockTwo, 0, TEST01_BLOCK_LENGTH);

		system_pickRandomSeed();

		for(nn = 0; nn < TEST01_BLOCK_LENGTH; nn++) {
			blockOne[nn] = (char)(rand() % 256);
		}

		timer = time_getTimeMus();

		if((rc = file_write(&fh, blockOne, TEST01_BLOCK_LENGTH)) < 0) {
			return error(rc);
		}

		elapsedTime = time_getElapsedMusInSeconds(timer);
		if(elapsedTime < writeLow) {
			writeLow = elapsedTime;
		}
		if(elapsedTime > writeHigh) {
			writeHigh = elapsedTime;
		}
		writeTime += elapsedTime;
		timer = time_getTimeMus();

		if((rc = file_position(&fh, &position)) < 0) {
			return error(rc);
		}

		elapsedTime = time_getElapsedMusInSeconds(timer);
		if(elapsedTime < positionLow) {
			positionLow = elapsedTime;
		}
		if(elapsedTime > positionHigh) {
			positionHigh = elapsedTime;
		}
		positionTime += elapsedTime;
		positionCount++;

		if(getHandlePosition(&fh) != position) {
			return error(0);
		}

		timer = time_getTimeMus();

		if((rc = file_seek(&fh, (alint)(ii * TEST01_BLOCK_LENGTH))) < 0) {
			return error(rc);
		}

		elapsedTime = time_getElapsedMusInSeconds(timer);
		if(elapsedTime < seekLow) {
			seekLow = elapsedTime;
		}
		if(elapsedTime > seekHigh) {
			seekHigh = elapsedTime;
		}
		seekTime += elapsedTime;
		timer = time_getTimeMus();

		if((rc = file_position(&fh, &position)) < 0) {
			return error(rc);
		}

		elapsedTime = time_getElapsedMusInSeconds(timer);
		if(elapsedTime < positionLow) {
			positionLow = elapsedTime;
		}
		if(elapsedTime > positionHigh) {
			positionHigh = elapsedTime;
		}
		positionTime += elapsedTime;
		positionCount++;

		if(getHandlePosition(&fh) != position) {
			return error(0);
		}

		timer = time_getTimeMus();

		if((rc = file_read(&fh, blockTwo, TEST01_BLOCK_LENGTH)) < 0) {
			return error(rc);
		}

		elapsedTime = time_getElapsedMusInSeconds(timer);
		if(elapsedTime < readLow) {
			readLow = elapsedTime;
		}
		if(elapsedTime > readHigh) {
			readHigh = elapsedTime;
		}
		readTime += elapsedTime;

		if(memcmp(blockOne, blockTwo, TEST01_BLOCK_LENGTH)) {
			return error(0);
		}

		timer = time_getTimeMus();

		if((rc = file_position(&fh, &position)) < 0) {
			return error(rc);
		}

		elapsedTime = time_getElapsedMusInSeconds(timer);
		if(elapsedTime < positionLow) {
			positionLow = elapsedTime;
		}
		if(elapsedTime > positionHigh) {
			positionHigh = elapsedTime;
		}
		positionTime += elapsedTime;
		positionCount++;

		if(getHandlePosition(&fh) != position) {
			return error(0);
		}

		if((rand() % 100) > 49) {
			timer = time_getTimeMus();

			if((rc = file_sync(&fh)) < 0) {
				return error(rc);
			}

			elapsedTime = time_getElapsedMusInSeconds(timer);
			if(elapsedTime < syncLow) {
				syncLow = elapsedTime;
			}
			if(elapsedTime > syncHigh) {
				syncHigh = elapsedTime;
			}
			syncTime += elapsedTime;
			syncCount++;
			timer = time_getTimeMus();

			if((rc = file_position(&fh, &position)) < 0) {
				return error(rc);
			}

			elapsedTime = time_getElapsedMusInSeconds(timer);
			if(elapsedTime < positionLow) {
				positionLow = elapsedTime;
			}
			if(elapsedTime > positionHigh) {
				positionHigh = elapsedTime;
			}
			positionTime += elapsedTime;
			positionCount++;

			if(getHandlePosition(&fh) != position) {
				return error(0);
			}
		}
	}

	printf("\n");

	if((rc = file_free(&fh)) < 0) {
		return error(rc);
	}

	printf("[unit]\t\t avg write time     : %0.6f (low %0.6f, high %0.6f)\n",
			(writeTime / (double)TEST01_TESTS), writeLow, writeHigh);
	printf("[unit]\t\t avg position time  : %0.6f (low %0.6f, high %0.6f)\n",
			(positionTime / (double)positionCount), positionLow,
			positionHigh);
	printf("[unit]\t\t avg seek time      : %0.6f (low %0.6f, high %0.6f)\n",
			(seekTime / (double)TEST01_TESTS), seekLow, seekHigh);
	printf("[unit]\t\t avg read time      : %0.6f (low %0.6f, high %0.6f)\n",
			(readTime / (double)TEST01_TESTS), readLow, readHigh);
	printf("[unit]\t\t avg sync time      : %0.6f (low %0.6f, high %0.6f)\n",
			(syncTime / (double)syncCount), syncLow, syncHigh);
	printf("[unit]\t\t total time         : %0.6f\n",
			(writeTime + positionTime + seekTime + readTime + syncTime));
	printf("[unit]\t\t test time          : %0.2f seconds\n",
			elapsedTime = time_getElapsedMusInSeconds(totalTimer));

	if(system_fileDelete(TEST01_FILENAME) < 0) {
		return error(0);
	}

	printf("[unit]\t ...PASSED\n");

	return 0;
}

static int file_typesIoTest()
{
	int ii = 0;
	int rc = 0;
	aboolean fileExists = afalse;
	aboolean bValueOne = 0;
	aboolean bValueTwo = 0;
	char cValueOne = (char)0;
	char cValue2 = (char)0;
	short sValueOne = 0;
	short sValueTwo = 0;
	int iValueOne = 0;
	int iValueTwo = 0;
	float fValueOne = 0.0;
	float fValueTwo = 0.0;
	alint lValueOne = 0;
	alint lValueTwo = 0;
	alint position = 0;
	double dValueOne = 0.0;
	double dValueTwo = 0.0;

	FileHandle *handle = NULL;
	FileHandle fh;

	printf("[unit]\t file-handling types i/o test...\n");
	printf("[unit]\t\t filename      : %s\n", TEST02_FILENAME);
	printf("[unit]\t\t buffer length : %i\n", TEST02_BUFFER_LENGTH);
	printf("[unit]\t\t tests         : %i\n", TEST02_TESTS);

	handle = &fh;

	system_fileExists(TEST02_FILENAME, &fileExists);
	if(fileExists) {
		if(system_fileDelete(TEST02_FILENAME) < 0) {
			return error(0);
		}
		printf("[unit]\t\t ...found and deleted test file '%s'\n",
				TEST02_FILENAME);
	}

	if((rc = file_init(&fh, TEST02_FILENAME, "w+b",
					TEST02_BUFFER_LENGTH)) < 0) {
		return error(rc);
	}

	for(ii = 0; ii < TEST02_TESTS; ii++) {
		system_pickRandomSeed();

		// test bool i/o

		bValueOne = (aboolean)(rand() % 2);

		if((rc = file_writeBool(&fh, bValueOne)) < 0) {
			return error(rc);
		}

		if((rc = file_position(handle, &position)) < 0) {
			return error(rc);
		}

		if(getHandlePosition(handle) != position) {
			return error(0);
		}

		if((rc = file_seek(handle, (position - SIZEOF_BOOLEAN))) < 0) {
			return error(rc);
		}

		if((rc = file_position(handle, &position)) < 0) {
			return error(rc);
		}

		if(getHandlePosition(handle) != position) {
			return error(0);
		}

		if((rc = file_readBool(&fh, &bValueTwo)) < 0) {
			return error(rc);
		}

		if(bValueOne != bValueTwo) {
			return error(0);
		}

		// test char i/o

		cValueOne = (char)(rand() % 256);

		if((rc = file_writeChar(&fh, cValueOne)) < 0) {
			return error(rc);
		}

		if((rc = file_position(handle, &position)) < 0) {
			return error(rc);
		}

		if(getHandlePosition(handle) != position) {
			return error(0);
		}

		if((rc = file_seek(handle, (position - SIZEOF_CHAR))) < 0) {
			return error(rc);
		}

		if((rc = file_position(handle, &position)) < 0) {
			return error(rc);
		}

		if(getHandlePosition(handle) != position) {
			return error(0);
		}

		if((rc = file_readChar(&fh, &cValue2)) < 0) {
			return error(rc);
		}

		if(cValueOne != cValue2) {
			return error(0);
		}

		// test short i/o

		sValueOne = (short)rand();

		if((rc = file_writeShort(&fh, sValueOne)) < 0) {
			return error(rc);
		}

		if((rc = file_position(handle, &position)) < 0) {
			return error(rc);
		}

		if(getHandlePosition(handle) != position) {
			return error(0);
		}

		if((rc = file_seek(handle, (position - SIZEOF_SHORT))) < 0) {
			return error(rc);
		}

		if((rc = file_position(handle, &position)) < 0) {
			return error(rc);
		}

		if(getHandlePosition(handle) != position) {
			return error(0);
		}

		if((rc = file_readShort(&fh, &sValueTwo)) < 0) {
			return error(rc);
		}

		if(sValueOne != sValueTwo) {
			return error(0);
		}

		// test int i/o

		iValueOne = (int)rand();

		if((rc = file_writeInt(&fh, iValueOne)) < 0) {
			return error(rc);
		}

		if((rc = file_position(handle, &position)) < 0) {
			return error(rc);
		}

		if(getHandlePosition(handle) != position) {
			return error(0);
		}

		if((rc = file_seek(handle, (position - SIZEOF_INT))) < 0) {
			return error(rc);
		}

		if((rc = file_position(handle, &position)) < 0) {
			return error(rc);
		}

		if(getHandlePosition(handle) != position) {
			return error(0);
		}

		if((rc = file_readInt(&fh, &iValueTwo)) < 0) {
			return error(rc);
		}

		if(iValueOne != iValueTwo) {
			return error(0);
		}

		// test float i/o

		fValueOne = (float)rand();

		if((rc = file_writeFloat(&fh, fValueOne)) < 0) {
			return error(rc);
		}

		if((rc = file_position(handle, &position)) < 0) {
			return error(rc);
		}

		if(getHandlePosition(handle) != position) {
			return error(0);
		}

		if((rc = file_seek(handle, (position - SIZEOF_FLOAT))) < 0) {
			return error(rc);
		}

		if((rc = file_position(handle, &position)) < 0) {
			return error(rc);
		}

		if(getHandlePosition(handle) != position) {
			return error(0);
		}

		if((rc = file_readFloat(&fh, &fValueTwo)) < 0) {
			return error(rc);
		}

		if(fValueOne != fValueTwo) {
			return error(0);
		}

		// test alint i/o

		lValueOne = (alint)rand();

		if((rc = file_writeAlint(&fh, lValueOne)) < 0) {
			return error(rc);
		}

		if((rc = file_position(handle, &position)) < 0) {
			return error(rc);
		}

		if(getHandlePosition(handle) != position) {
			return error(0);
		}

		if((rc = file_seek(handle, (position - SIZEOF_ALINT))) < 0) {
			return error(rc);
		}

		if((rc = file_position(handle, &position)) < 0) {
			return error(rc);
		}

		if(getHandlePosition(handle) != position) {
			return error(0);
		}

		if((rc = file_readAlint(&fh, &lValueTwo)) < 0) {
			return error(rc);
		}

		if(lValueOne != lValueTwo) {
			return error(0);
		}

		// test double i/o

		dValueOne = (double)rand();

		if((rc = file_writeDouble(&fh, dValueOne)) < 0) {
			return error(rc);
		}

		if((rc = file_position(handle, &position)) < 0) {
			return error(rc);
		}

		if(getHandlePosition(handle) != position) {
			return error(0);
		}

		if((rc = file_seek(handle, (position - SIZEOF_DOUBLE))) < 0) {
			return error(rc);
		}

		if((rc = file_position(handle, &position)) < 0) {
			return error(rc);
		}

		if(getHandlePosition(handle) != position) {
			return error(0);
		}

		if((rc = file_readDouble(&fh, &dValueTwo)) < 0) {
			return error(rc);
		}

		if(dValueOne != dValueTwo) {
			return error(0);
		}
	}

	if((rc = file_free(&fh)) < 0) {
		return error(rc);
	}

	if(system_fileDelete(TEST02_FILENAME) < 0) {
		return error(0);
	}

	printf("[unit]\t ...PASSED\n");

	return 0;
}

static int file_readLineTest()
{
	aboolean fileExists = afalse;
	int rc = 0;
	int counter = 0;
	int readLength = 0;
	alint position = 0;
	char buffer[8192];

	FileHandle *handle = NULL;
	FileHandle fh;

	printf("[unit]\t file-handling read line test...\n");
	printf("[unit]\t\t filename      : %s\n", TEST03_FILENAME);
	printf("[unit]\t\t buffer length : %i\n", TEST03_BUFFER_LENGTH);

	handle = &fh;

	system_fileExists(TEST03_FILENAME, &fileExists);
	if(!fileExists) {
		printf("[unit]\t\t ...unable to execute when file '%s' does not "
				"exist\n", TEST03_FILENAME);
		return -1;
	}

	if((rc = file_init(&fh, TEST03_FILENAME, "rb", TEST03_BUFFER_LENGTH)) < 0) {
		return error(rc);
	}

	rc = file_readLine(&fh, buffer, sizeof(buffer), &readLength);
	while(rc == 0) {
		if((rc = file_position(handle, &position)) < 0) {
			return error(rc);
		}

		if(getHandlePosition(handle) != position) {
			return error(0);
		}

		printf("[unit]\t\t line #%6i: '%s'\n", counter, buffer);
		counter++;

		rc = file_readLine(&fh, buffer, sizeof(buffer), &readLength);
	}

	if(rc < 0) {
		return error(rc);
	}

	if((rc = file_free(&fh)) < 0) {
		return error(rc);
	}

	printf("[unit]\t ...PASSED\n");

	return 0;
}

