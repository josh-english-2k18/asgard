/*
 * system.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard system-commands library.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#define _CORE_SYSTEM_COMPONENT
#include "core/system/system.h"
#define _CORE_SYSTEM_PLUGIN_COMPONENT
#include "core/system/plugin/system_linux.h"
#include "core/system/plugin/system_windows.h"

// define system public functions

// file-related functions

int system_fileSetNative(char *filename)
{
#if defined(__linux__) || defined(__APPLE__) || defined(MINGW)
	return system_fileSetNativeLinux(filename);
#elif defined(WIN32)
	return system_fileSetNativeWindows(filename);
#else // - no plugin available -
	return -1;
#endif // - plugins -
}

int system_fileExists(char *filename, aboolean *exists)
{
#if defined(__linux__) || defined(__APPLE__) || defined(MINGW)
	return system_fileExistsLinux(filename, exists);
#elif defined(WIN32)
	return system_fileExistsWindows(filename, exists);
#else // - no plugin available -
	return -1;
#endif // - plugins -
}

int system_fileLength(char *filename, alint *length)
{
#if defined(__linux__) || defined(__APPLE__) || defined(MINGW)
	return system_fileLengthLinux(filename, length);
#elif defined(WIN32)
	return system_fileLengthWindows(filename, length);
#else // - no plugin available -
	return -1;
#endif // - plugins -
}

int system_fileModifiedTime(char *filename, double *timestamp)
{
#if defined(__linux__) || defined(__APPLE__) || defined(MINGW)
	return system_fileModifiedTimeLinux(filename, timestamp);
#elif defined(WIN32)
	return system_fileModifiedTimeWindows(filename, timestamp);
#else // - no plugin available -
	return -1;
#endif // - plugins -
}

int system_fileDelete(char *filename)
{
#if defined(__linux__) || defined(__APPLE__) || defined(MINGW)
	return system_fileDeleteLinux(filename);
#elif defined(WIN32)
	return system_fileDeleteWindows(filename);
#else // - no plugin available -
	return -1;
#endif // - plugins -
}

int system_fileMove(char *sourceFilename, char *destFilename)
{
#if defined(__linux__) || defined(__APPLE__) || defined(MINGW)
	return system_fileMoveLinux(sourceFilename, destFilename);
#elif defined(WIN32)
	return system_fileMoveWindows(sourceFilename, destFilename);
#else // - no plugin available -
	return -1;
#endif // - plugins -
}

int system_fileCopy(char *sourceFilename, char *destFilename)
{
#if defined(__linux__) || defined(__APPLE__) || defined(MINGW)
	return system_fileCopyLinux(sourceFilename, destFilename);
#elif defined(WIN32)
	return system_fileCopyWindows(sourceFilename, destFilename);
#else // - no plugin available -
	return -1;
#endif // - plugins -
}

int system_fileExecute(char *filename, char *arguments)
{
#if defined(__linux__) || defined(__APPLE__) || defined(MINGW)
	return system_fileExecuteLinux(filename, arguments);
#elif defined(WIN32)
	return system_fileExecuteWindows(filename, arguments);
#else // - no plugin available -
	return -1;
#endif // - plugins -
}

// dir-related functions

int system_dirExists(char *dirname, aboolean *exists)
{
#if defined(__linux__) || defined(__APPLE__) || defined(MINGW)
	return system_dirExistsLinux(dirname, exists);
#elif defined(WIN32)
	return system_dirExistsWindows(dirname, exists);
#else // - no plugin available -
	return -1;
#endif // - plugins -
}

int system_dirChangeTo(char *dirname)
{
#if defined(__linux__) || defined(__APPLE__) || defined(MINGW)
	return system_dirChangeToLinux(dirname);
#elif defined(WIN32)
	return system_dirChangeToWindows(dirname);
#else // - no plugin available -
	return -1;
#endif // - plugins -
}

int system_dirCreate(char *dirname)
{
#if defined(__linux__) || defined(__APPLE__) || defined(MINGW)
	return system_dirCreateLinux(dirname);
#elif defined(WIN32)
	return system_dirCreateWindows(dirname);
#else // - no plugin available -
	return -1;
#endif // - plugins -
}

int system_dirDelete(char *dirname)
{
#if defined(__linux__) || defined(__APPLE__) || defined(MINGW)
	return system_dirDeleteLinux(dirname);
#elif defined(WIN32)
	return system_dirDeleteWindows(dirname);
#else // - no plugin available -
	return -1;
#endif // - plugins -
}

// random-related functions

void system_pickRandomSeed()
{
	int ii = 0;

	union {
		int uiValue;
		char cValue[SIZEOF_INT];
	} integerUnion;

	srand((unsigned int)time_getTimeMus());

	for(ii = 0; ii < SIZEOF_INT; ii++) {
		integerUnion.cValue[ii] = (char)(rand() % 256);
	}

	system_setRandomSeed((unsigned int)integerUnion.uiValue);
}

void system_setRandomSeed(unsigned int seed)
{
	srand(seed);
}

