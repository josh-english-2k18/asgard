/*
 * file.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard core library file-handling functions, header file.
 *
 * Written by Josh English.
 */

#if !defined(_CORE_LIBRARY_SYSTEM_FILE_H)

#define _CORE_LIBRARY_SYSTEM_FILE_H

#if !defined(_CORE_H) && !defined(_CORE_LIBRARY_H) && \
		!defined(_CORE_LIBRARY_SYSTEM_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _CORE_H || _CORE_LIBRARY_H || _CORE_LIBRARY_SYSTEM_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// delcare file-handling public constants

#define FILE_EOF									1


// delcare file-handling public data types

typedef struct _FileHandle {
	int bufferLength;
	alint position;
	char *filename;
	char *buffer;
	FILE *fd;
	Error errors;
} FileHandle;


// delcare file-handling public functions

int file_init(FileHandle *handle, char *filename, char *mode, int bufferLength);

int file_free(FileHandle *handle);

int file_write(FileHandle *handle, char *data, int length);

int file_writeBool(FileHandle *handle, aboolean value);

int file_writeChar(FileHandle *handle, char value);

int file_writeShort(FileHandle *handle, short value);

int file_writeInt(FileHandle *handle, int value);

int file_writeFloat(FileHandle *handle, float value);

int file_writeAlint(FileHandle *handle, alint value);

int file_writeDouble(FileHandle *handle, double value);

int file_read(FileHandle *handle, char *data, int length);

int file_readBool(FileHandle *handle, aboolean *value);

int file_readChar(FileHandle *handle, char *value);

int file_readShort(FileHandle *handle, short *value);

int file_readInt(FileHandle *handle, int *value);

int file_readFloat(FileHandle *handle, float *value);

int file_readAlint(FileHandle *handle, alint *value);

int file_readDouble(FileHandle *handle, double *value);

int file_readLine(FileHandle *handle, char *data, int dataLength,
		int *readLength);

int file_position(FileHandle *handle, alint *position);

int file_seek(FileHandle *handle, alint position);

int file_seekEof(FileHandle *handle);

int file_sync(FileHandle *handle);

int file_getFileLength(FileHandle *handle, alint *length);

int file_getError(FileHandle *handle, int errorCode, char **errorMessage);

char *file_getErrorMessage(FileHandle *handle, int errorCode);

alint file_getFileLengthOnFilename(char *filename);


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _CORE_LIBRARY_SYSTEM_FILE_H

