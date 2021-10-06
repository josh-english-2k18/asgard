/*
 * system_linux.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard system-commands library, Linux plugin.
 *
 * Written by Josh English.
 */


#if defined(__linux__) || defined(__APPLE__) || defined(MINGW)


// preprocessor directives


#include "core/core.h"
#define _CORE_SYSTEM_PLUGIN_COMPONENT
#include "core/system/plugin/system_linux.h"


// declare system (linux plugin) private functions

static aboolean isLocalFormat(char *filename);

static int executeStat(char *filename, struct stat *statistics);

static aboolean fileExists(char *filename);


// define system (linux plugin) private functions

static aboolean isLocalFormat(char *filename)
{
	int ii = 0;
	int length = 0;

	length = strlen(filename);
	for(ii = 0; ii < length; ii++) {
		if(filename[ii] == '\\') {
			return afalse;
		}
	}

	return atrue;
}

static int executeStat(char *filename, struct stat *statistics)
{
	aboolean isLocal = afalse;
	int rc = 0;
	char *localFilename = NULL;

	isLocal = isLocalFormat(filename);

	if(isLocal) {
		localFilename = filename;
	}
	else {
		localFilename = strdup(filename);
		system_fileSetNativeLinux(localFilename);
	}

	rc = stat(localFilename, statistics);

	if(!isLocal) {
		free(localFilename);
	}

	if(rc != 0) {
		return -1;
	}

	return 0;
}

static aboolean fileExists(char *filename)
{
	aboolean exists = afalse;

	system_fileExistsLinux(filename, &exists);

	return exists;
}

// define system (linux plugin) public functions

// file-related functions

int system_fileSetNativeLinux(char *filename)
{
	int ii = 0;
	int length = 0;

	if(filename == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	length = strlen(filename);
	for(ii = 0; ii < length; ii++) {
		if((filename[ii] == '/') || (filename[ii] == '\\')) {
			filename[ii] = DIR_SEPARATOR;
		}
	}

	return 0;
}

int system_fileExistsLinux(char *filename, aboolean *exists)
{
	struct stat fileStatistics;

	if((filename == NULL) || (exists == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	*exists = afalse;

	if(executeStat(filename, &fileStatistics) < 0) {
		return 0;
	}

	if((fileStatistics.st_mode & S_IFREG) == S_IFREG) {
		*exists = atrue;
	}

	return 0;
}

int system_fileLengthLinux(char *filename, alint *length)
{
	struct stat fileStatistics;

	if((filename == NULL) || (length == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	*length = 0;

	if(executeStat(filename, &fileStatistics) < 0) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s():%i] error - failed to stat '%s' with "
					"'%s'.\n", __FUNCTION__, __LINE__, filename,
					strerror(errno));
		}
		return -1;
	}

	*length = (alint)fileStatistics.st_size;

	return 0;
}

int system_fileModifiedTimeLinux(char *filename, double *timestamp)
{
	struct stat fileStatistics;

	if((filename == NULL) || (timestamp == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	*timestamp = 0.0;

	if(executeStat(filename, &fileStatistics) < 0) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s():%i] error - failed to stat '%s' with "
					"'%s'.\n", __FUNCTION__, __LINE__, filename,
					strerror(errno));
		}
		return -1;
	}

	*timestamp = (double)fileStatistics.st_mtime;

	return 0;
}

int system_fileDeleteLinux(char *filename)
{
	aboolean isLocal = afalse;
	int rc = 0;
	char *localFilename = NULL;

	if(filename == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	isLocal = isLocalFormat(filename);

	if(isLocal) {
		localFilename = filename;
	}
	else {
		localFilename = strdup(filename);
		system_fileSetNativeLinux(localFilename);
	}

	rc = unlink(localFilename);

	if(!isLocal) {
		free(localFilename);
	}

	if(rc != 0) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s():%i] error - failed to unlink '%s' with "
					"'%s'.\n", __FUNCTION__, __LINE__, filename,
					strerror(errno));
		}
		return -1;
	}

	return 0;
}

int system_fileMoveLinux(char *sourceFilename, char *destFilename)
{
	aboolean isLocal = afalse;
	int rc = 0;
	int result = 0;
	char *localSourceFilename = NULL;
	char *localDestFilename = NULL;

	if((sourceFilename == NULL) || (destFilename == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	isLocal = isLocalFormat(sourceFilename);
	if(isLocal) {
		isLocal = isLocalFormat(destFilename);
	}

	if(isLocal) {
		localSourceFilename = sourceFilename;
		localDestFilename = destFilename;
	}
	else {
		localSourceFilename = strdup(sourceFilename);
		system_fileSetNativeLinux(localSourceFilename);
		localDestFilename = strdup(destFilename);
		system_fileSetNativeLinux(localDestFilename);
	}

	if(!fileExists(localSourceFilename)) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s():%i] error - unable to move '%s' to '%s', "
					"source file does not exit.\n", __FUNCTION__, __LINE__,
					sourceFilename, destFilename);
		}
		if(!isLocal) {
			free(localSourceFilename);
			free(localDestFilename);
		}
		return -1;
	}
	if(fileExists(localDestFilename)) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s():%i] error - unable to move '%s' to '%s', "
					"dest file already exists.\n", __FUNCTION__, __LINE__,
					sourceFilename, destFilename);
		}
		if(!isLocal) {
			free(localSourceFilename);
			free(localDestFilename);
		}
		return -1;
	}

	errno = 0;
	rc = rename(localSourceFilename, localDestFilename);

	if((rc != 0) && (errno == EXDEV)) {
		result = system_fileCopyLinux(localSourceFilename, localDestFilename);
	}
	else if(rc != 0) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s():%i] error - failed to move '%s' to '%s' "
					"with '%s'.\n", __FUNCTION__, __LINE__, sourceFilename,
					destFilename, strerror(errno));
		}
		result = -1;
	}

	if(!isLocal) {
		free(localSourceFilename);
		free(localDestFilename);
	}

	return result;
}

int system_fileCopyLinux(char *sourceFilename, char *destFilename)
{
	aboolean isLocal = afalse;
	int inRc = 0;
	int outRc = 0;
	int result = 0;
	int blockLength = 0;
	char *localSourceFilename = NULL;
	char *localDestFilename = NULL;
	char block[65536];

	FILE *inFd = NULL;
	FILE *outFd = NULL;

	if((sourceFilename == NULL) || (destFilename == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	isLocal = isLocalFormat(sourceFilename);
	if(isLocal) {
		isLocal = isLocalFormat(destFilename);
	}

	if(isLocal) {
		localSourceFilename = sourceFilename;
		localDestFilename = destFilename;
	}
	else {
		localSourceFilename = strdup(sourceFilename);
		system_fileSetNativeLinux(localSourceFilename);
		localDestFilename = strdup(destFilename);
		system_fileSetNativeLinux(localDestFilename);
	}

	if(!fileExists(localSourceFilename)) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s():%i] error - unable to copy '%s' to '%s', "
					"source file does not exit.\n", __FUNCTION__, __LINE__,
					sourceFilename, destFilename);
		}
		if(!isLocal) {
			free(localSourceFilename);
			free(localDestFilename);
		}
		return -1;
	}
	if(fileExists(localDestFilename)) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s():%i] error - unable to copy '%s' to '%s', "
					"dest file already exists.\n", __FUNCTION__, __LINE__,
					sourceFilename, destFilename);
		}
		if(!isLocal) {
			free(localSourceFilename);
			free(localDestFilename);
		}
		return -1;
	}

	if((inFd = fopen(localSourceFilename, "rb")) == NULL) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s():%i] error - failed to open file '%s' for "
					"copy with '%s'.\n", __FUNCTION__, __LINE__,
					sourceFilename, strerror(errno));
		}
		if(!isLocal) {
			free(localSourceFilename);
			free(localDestFilename);
		}
		return -1;
	}

	if((outFd = fopen(localDestFilename, "wb")) == NULL) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s():%i] error - failed to open file '%s' for "
					"copy with '%s'.\n", __FUNCTION__, __LINE__,
					destFilename, strerror(errno));
		}
		fclose(inFd);
		if(!isLocal) {
			free(localSourceFilename);
			free(localDestFilename);
		}
		return -1;
	}

	blockLength = sizeof(block);
	inRc = fread(block, SIZEOF_CHAR, blockLength, inFd);
	while(inRc > 0) {
		outRc = fwrite(block, SIZEOF_CHAR, inRc, outFd);
		if(outRc <= 0) {
			break;
		}
		inRc = fread(block, SIZEOF_CHAR, blockLength, inFd);
	}

	if((inRc <= 0) && (!feof(inFd))) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s():%i] error - failed to read from file '%s' "
					"for copy with '%s'.\n", __FUNCTION__, __LINE__,
					sourceFilename, strerror(errno));
		}
		result = -1;
	}

	if(outRc <= 0) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s():%i] error - failed to write to file '%s' "
					"for copy with '%s'.\n", __FUNCTION__, __LINE__,
					destFilename, strerror(errno));
		}
		result = -1;
	}

	if(fclose(inFd) != 0) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s():%i] error - failed to close file '%s' for "
					"copy with '%s'.\n", __FUNCTION__, __LINE__,
					sourceFilename, strerror(errno));
		}
	}

	if(fclose(outFd) != 0) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s():%i] error - failed to close file '%s' for "
					"copy with '%s'.\n", __FUNCTION__, __LINE__,
					destFilename, strerror(errno));
		}
	}

	if(!isLocal) {
		free(localSourceFilename);
		free(localDestFilename);
	}

	return result;
}

int system_fileExecuteLinux(char *filename, char *arguments)
{
#if !defined(MINGW)
	aboolean isLocal = afalse;
	int rc = 0;
	int pid = 0;
	char *localFilename = NULL;

	if(filename == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	isLocal = isLocalFormat(filename);

	if(isLocal) {
		localFilename = filename;
	}
	else {
		localFilename = strdup(filename);
		system_fileSetNativeLinux(localFilename);
	}

	pid = (int)fork();
	if(pid < 0) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s():%i] error - failed to create new process "
					"with '%s'.\n", __FUNCTION__, __LINE__, strerror(errno));
		}
		return -1;
	}

	if(pid == 0) {
		rc = execlp(localFilename, localFilename, arguments, NULL);
	}

	if(!isLocal) {
		free(localFilename);
	}

	if(rc != 0) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s():%i] error - failed to execute '%s' with "
					"'%s'.\n", __FUNCTION__, __LINE__, filename,
					strerror(errno));
		}
		return -1;
	}

	return 0;
#else // MINGW
	aboolean isLocal = afalse;
	int rc = 0;
	char *localFilename = NULL;

	STARTUPINFO startupInfo;
	PROCESS_INFORMATION processInfo;

	if(filename == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	isLocal = isLocalFormat(filename);

	if(isLocal) {
		localFilename = filename;
	}
	else {
		localFilename = strdup(filename);
		system_fileSetNativeLinux(localFilename);
	}

	memset(&startupInfo, 0, (sizeof(startupInfo)));

	startupInfo.cb = sizeof(startupInfo);

	memset(&processInfo, 0, (sizeof(processInfo)));

	if(!CreateProcess(
				(LPCSTR)localFilename,		// the application to execute
				(LPTSTR)arguments,			// arguments
				NULL,						// process handle not inheritable
				NULL,						// thread handle not inheritable
				FALSE,						// no handle interitance
				0,							// no creation flags
				NULL,						// use parent's environment
				NULL,						// use parent's starting directory
				&startupInfo,				// startup information
				&processInfo				// process information
			)) {
		rc = -1;
	}

	if(!isLocal) {
		free(localFilename);
	}

	if(rc != 0) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s():%i] error - failed to execute '%s' with "
					"%i.\n", __FUNCTION__, __LINE__, filename,
					(int)GetLastError());
		}
		return -1;
	}

	return 0;
#endif // !MINGW
}

// dir-related functions

int system_dirExistsLinux(char *dirname, aboolean *exists)
{
	struct stat fileStatistics;

	if((dirname == NULL) || (exists == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	*exists = afalse;

	if(executeStat(dirname, &fileStatistics) < 0) {
		return 0;
	}

	if((fileStatistics.st_mode & S_IFDIR) == S_IFDIR) {
		*exists = atrue;
	}

	return 0;
}

int system_dirChangeToLinux(char *dirname)
{
	aboolean isLocal = afalse;
	int rc = 0;
	char *localDirname = NULL;

	isLocal = isLocalFormat(dirname);

	if(isLocal) {
		localDirname = dirname;
	}
	else {
		localDirname = strdup(dirname);
		system_fileSetNativeLinux(localDirname);
	}

	rc = chdir(localDirname);

	if(!isLocal) {
		free(localDirname);
	}

	if(rc != 0) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s():%i] error - failed to change to directory "
					"'%s' with '%s'.\n", __FUNCTION__, __LINE__, dirname,
					strerror(errno));
		}
		return -1;
	}

	return 0;
}

int system_dirCreateLinux(char *dirname)
{
	aboolean isLocal = afalse;
	int rc = 0;
	char *localDirname = NULL;

	isLocal = isLocalFormat(dirname);

	if(isLocal) {
		localDirname = dirname;
	}
	else {
		localDirname = strdup(dirname);
		system_fileSetNativeLinux(localDirname);
	}

#if !defined(MINGW)
	rc = mkdir(localDirname, 0777);
#else // MINGW
	rc = mkdir(localDirname);
#endif // !MINGW

	if(!isLocal) {
		free(localDirname);
	}

	if(rc != 0) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s():%i] error - failed to create directory "
					"'%s' with '%s'.\n", __FUNCTION__, __LINE__, dirname,
					strerror(errno));
		}
		return -1;
	}

	return 0;
}

int system_dirDeleteLinux(char *dirname)
{
	aboolean isLocal = afalse;
	int rc = 0;
	char *localDirname = NULL;

	isLocal = isLocalFormat(dirname);

	if(isLocal) {
		localDirname = dirname;
	}
	else {
		localDirname = strdup(dirname);
		system_fileSetNativeLinux(localDirname);
	}

	rc = rmdir(localDirname);

	if(!isLocal) {
		free(localDirname);
	}

	if(rc != 0) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s():%i] error - failed to delete directory "
					"'%s' with '%s'.\n", __FUNCTION__, __LINE__, dirname,
					strerror(errno));
		}
		return -1;
	}

	return 0;
}


#endif // __linux__ || __APPLE__ || MINGW

