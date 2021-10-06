/*
 * file.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard core library file-handling functions.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#define _CORE_LIBRARY_SYSTEM_COMPONENT
#include "core/library/system/file.h"


// define file-handling public functions

int file_init(FileHandle *handle, char *filename, char *mode, int bufferLength)
{
	int result = 0;

	if((handle == NULL) || (filename == NULL) || (mode == NULL)) {
		result = errorf(stderr, "invalid or missing argument(s) { handle "
				"0x%lx, filename 0x%lx, mode 0x%lx }",
				(aptrcast)handle, (aptrcast)filename, (aptrcast)mode);
		return result;
	}

	memset(handle, 0, (sizeof(FileHandle)));

	handle->bufferLength = 0;
	handle->position = 0;
	handle->filename = strdup(filename);
	handle->buffer = NULL;

	error_init(&handle->errors);

	if(system_fileSetNative(handle->filename) < 0) {
		result = errorf(&handle->errors,
				"failed to set file '%s' to native filename",
				handle->filename);
		return result;
	}

#if defined(WIN32) && !defined(MINGW)
	if(fopen_s(&handle->fd, handle->filename, mode) != 0) {
#else // !WIN32 || MINGW
	handle->fd = fopen(handle->filename, mode);
	if(handle->fd == NULL) {
#endif // WIN32 && !MINGW
		result = errorf(&handle->errors,
				"failed to open file '%s' mode '%s' with '%s'",
				handle->filename, mode, strerror(errno));
		return result;
	}

	if(bufferLength > 0) {
		handle->bufferLength = bufferLength;
		handle->buffer = (char *)malloc(sizeof(char) *
				(handle->bufferLength + 1));
		result = setvbuf(handle->fd, handle->buffer, _IOFBF,
				handle->bufferLength);
		if(result != 0) {
			result = errorf(&handle->errors,
					"failed to set buffer 0x%lx on file '%s' length %i",
					(aptrcast)handle->buffer, handle->filename,
					handle->bufferLength);
		}
	}

	return result;
}

int file_free(FileHandle *handle)
{
	int result = 0;

	if(handle == NULL) {
		result = errorf(stderr, "invalid or missing argument(s) { handle "
				"0x%lx }", (aptrcast)handle);
		return result;
	}

	if(handle->fd != NULL) {
		result = fclose(handle->fd);
		if(result != 0) {
			result = errorf(stderr, "failed to close file '%s' with '%s'",
					handle->filename, strerror(errno));
		}
	}

	if(handle->filename != NULL) {
		free(handle->filename);
	}

	if(handle->buffer != NULL) {
		free(handle->buffer);
	}

	error_free(&handle->errors);

	memset(handle, 0, (sizeof(FileHandle)));

	return result;
}

int file_write(FileHandle *handle, char *data, int length)
{
	int result = 0;

	if((handle == NULL) || (handle->fd == NULL) || (data == NULL) ||
			(length < 1)) {
		result = errorf(stderr, "invalid or missing argument(s) { handle "
				"0x%lx, data 0x%lx, length %i }", (aptrcast)handle,
				(aptrcast)data, length);
		return result;
	}

	result = (int)fwrite(data, SIZEOF_CHAR, length, handle->fd);
	if(result != length) {
		result = errorf(&handle->errors,
				"failed to write %i bytes to file '%s' at %li with (%i)'%s'",
				length, handle->filename, handle->position, result,
				strerror(errno));
	}
	else {
		handle->position += (alint)length;
	}

	return result;
}

int file_writeBool(FileHandle *handle, aboolean value)
{
	int result = 0;
	char data[2];

	if(handle == NULL) {
		result = errorf(stderr, "invalid or missing argument(s) { handle "
				"0x%lx }", (aptrcast)handle);
		return result;
	}

	data[0] = (char)value;
	data[1] = '\0';

	result = file_write(handle, data, SIZEOF_BOOLEAN);

	return result;
}

int file_writeChar(FileHandle *handle, char value)
{
	int result = 0;
	char data[2];

	if(handle == NULL) {
		result = errorf(stderr, "invalid or missing argument(s) { handle "
				"0x%lx }", (aptrcast)handle);
		return result;
	}

	data[0] = value;
	data[1] = '\0';

	result = file_write(handle, data, SIZEOF_CHAR);

	return result;
}

int file_writeShort(FileHandle *handle, short value)
{
	int result = 0;
	char data[SIZEOF_SHORT];

	if(handle == NULL) {
		result = errorf(stderr, "invalid or missing argument(s) { handle "
				"0x%lx }", (aptrcast)handle);
		return result;
	}

	serialize_encodeShort(value, data);

	result = file_write(handle, data, SIZEOF_SHORT);

	return result;
}

int file_writeInt(FileHandle *handle, int value)
{
	int result = 0;
	char data[SIZEOF_INT];

	if(handle == NULL) {
		result = errorf(stderr, "invalid or missing argument(s) { handle "
				"0x%lx }", (aptrcast)handle);
		return result;
	}

	serialize_encodeInt(value, data);

	result = file_write(handle, data, SIZEOF_INT);

	return result;
}

int file_writeFloat(FileHandle *handle, float value)
{
	int result = 0;
	char data[SIZEOF_FLOAT];

	if(handle == NULL) {
		result = errorf(stderr, "invalid or missing argument(s) { handle "
				"0x%lx }", (aptrcast)handle);
		return result;
	}

	serialize_encodeFloat(value, data);

	result = file_write(handle, data, SIZEOF_FLOAT);

	return result;
}

int file_writeAlint(FileHandle *handle, alint value)
{
	int result = 0;
	char data[SIZEOF_ALINT];

	if(handle == NULL) {
		result = errorf(stderr, "invalid or missing argument(s) { handle "
				"0x%lx }", (aptrcast)handle);
		return result;
	}

	serialize_encodeAlint(value, data);

	result = file_write(handle, data, SIZEOF_ALINT);

	return result;
}

int file_writeDouble(FileHandle *handle, double value)
{
	int result = 0;
	char data[SIZEOF_DOUBLE];

	if(handle == NULL) {
		result = errorf(stderr, "invalid or missing argument(s) { handle "
				"0x%lx }", (aptrcast)handle);
		return result;
	}

	serialize_encodeDouble(value, data);

	result = file_write(handle, data, SIZEOF_DOUBLE);

	return result;
}

int file_read(FileHandle *handle, char *data, int length)
{
	int rc = 0;
	int result = 0;

	if((handle == NULL) || (handle->fd == NULL) || (data == NULL) ||
			(length < 1)) {
		result = errorf(stderr, "invalid or missing argument(s) { handle "
				"0x%lx, data 0x%lx, length %i }", (aptrcast)handle,
				(aptrcast)data, length);
		return result;
	}

	result = (int)fread(data, SIZEOF_CHAR, length, handle->fd);
	if(result != length) {
		rc = feof(handle->fd);
		if((rc != 0) && (rc != -1)) {
			result = FILE_EOF;
			clearerr(handle->fd);
		}
		else {
			result = errorf(&handle->errors,
					"failed to read %i bytes from file '%s' at %li "
					"with (%i)'%s'",
					length, handle->filename, handle->position, result,
					strerror(errno));
		}
	}
	else {
		handle->position += (alint)length;
#if defined(WIN32) && !defined(MINGW)
		file_sync(handle);
#endif // WIN32
	}

	return result;
}

int file_readBool(FileHandle *handle, aboolean *value)
{
	int result = 0;
	char data[2];

	if((handle == NULL) || (value == NULL)) {
		result = errorf(stderr, "invalid or missing argument(s) { handle "
				"0x%lx, value 0x%lx }", (aptrcast)handle,
				(aptrcast)value);
		return result;
	}

	result = file_read(handle, data, SIZEOF_BOOLEAN);
	if(result < 0) {
		return result;
	}

	*value = data[0];

	return 0;
}

int file_readChar(FileHandle *handle, char *value)
{
	int result = 0;
	char data[2];

	if((handle == NULL) || (value == NULL)) {
		result = errorf(stderr, "invalid or missing argument(s) { handle "
				"0x%lx, value 0x%lx }", (aptrcast)handle,
				(aptrcast)value);
		return result;
	}

	result = file_read(handle, data, SIZEOF_CHAR);
	if(result < 0) {
		return result;
	}

	*value = data[0];

	return 0;
}

int file_readShort(FileHandle *handle, short *value)
{
	int result = 0;
	char data[SIZEOF_SHORT];

	if((handle == NULL) || (value == NULL)) {
		result = errorf(stderr, "invalid or missing argument(s) { handle "
				"0x%lx, value 0x%lx }", (aptrcast)handle,
				(aptrcast)value);
		return result;
	}

	result = file_read(handle, data, SIZEOF_SHORT);
	if(result < 0) {
		return result;
	}

	if((result = serialize_decodeShort(data, SIZEOF_SHORT, value)) < 0) {
		return result;
	}

	return 0;
}

int file_readInt(FileHandle *handle, int *value)
{
	int result = 0;
	char data[SIZEOF_INT];

	if((handle == NULL) || (value == NULL)) {
		result = errorf(stderr, "invalid or missing argument(s) { handle "
				"0x%lx, value 0x%lx }", (aptrcast)handle,
				(aptrcast)value);
		return result;
	}

	result = file_read(handle, data, SIZEOF_INT);
	if(result < 0) {
		return result;
	}

	if((result = serialize_decodeInt(data, SIZEOF_INT, value)) < 0) {
		return result;
	}

	return 0;
}

int file_readFloat(FileHandle *handle, float *value)
{
	int result = 0;
	char data[SIZEOF_FLOAT];

	if((handle == NULL) || (value == NULL)) {
		result = errorf(stderr, "invalid or missing argument(s) { handle "
				"0x%lx, value 0x%lx }", (aptrcast)handle,
				(aptrcast)value);
		return result;
	}

	result = file_read(handle, data, SIZEOF_FLOAT);
	if(result < 0) {
		return result;
	}

	if((result = serialize_decodeFloat(data, SIZEOF_FLOAT, value)) < 0) {
		return result;
	}

	return 0;
}

int file_readAlint(FileHandle *handle, alint *value)
{
	int result = 0;
	char data[SIZEOF_ALINT];

	if((handle == NULL) || (value == NULL)) {
		result = errorf(stderr, "invalid or missing argument(s) { handle "
				"0x%lx, value 0x%lx }", (aptrcast)handle,
				(aptrcast)value);
		return result;
	}

	result = file_read(handle, data, SIZEOF_ALINT);
	if(result < 0) {
		return result;
	}

	if((result = serialize_decodeAlint(data, SIZEOF_ALINT, value)) < 0) {
		return result;
	}

	return 0;
}

int file_readDouble(FileHandle *handle, double *value)
{
	int result = 0;
	char data[SIZEOF_DOUBLE];

	if((handle == NULL) || (value == NULL)) {
		result = errorf(stderr, "invalid or missing argument(s) { handle "
				"0x%lx, value 0x%lx }", (aptrcast)handle,
				(aptrcast)value);
		return result;
	}

	result = file_read(handle, data, SIZEOF_DOUBLE);
	if(result < 0) {
		return result;
	}

	if((result = serialize_decodeDouble(data, SIZEOF_DOUBLE, value)) < 0) {
		return result;
	}

	return 0;
}

int file_readLine(FileHandle *handle, char *data, int dataLength,
		int *readLength)
{
	int rc = 0;
	int result = 0;

	if((handle == NULL) || (handle->fd == NULL) || (data == NULL) ||
			(dataLength < 1) || (readLength == NULL)) {
		result = errorf(stderr, "invalid or missing argument(s) { handle "
				"0x%lx, data 0x%lx, dataLength %i, readLength 0x%lx }",
				(aptrcast)handle, (aptrcast)data, dataLength,
				(aptrcast)readLength);
		return result;
	}

	if(fgets(data, dataLength, handle->fd) == NULL) {
		rc = feof(handle->fd);
		if((rc != 0) && (rc != -1)) {
			result = FILE_EOF;
			clearerr(handle->fd);
		}
		else {
			result = errorf(&handle->errors,
					"failed to read line from file '%s' at %li with '%s'",
					handle->filename, handle->position, strerror(errno));
		}
	}
	else {
		*readLength = (int)strlen(data);
		handle->position += (alint)(*readLength);
	}

	return result;
}

int file_position(FileHandle *handle, alint *position)
{
	int result = 0;

	if((handle == NULL) || (position == NULL)) {
		result = errorf(stderr, "invalid or missing argument(s) { handle "
				"0x%lx, position 0x%lx }", (aptrcast)handle,
				(aptrcast)position);
		return result;
	}

	*position = handle->position;

	return result;
}

int file_seek(FileHandle *handle, alint position)
{
	int result = 0;

	if((handle == NULL) || (handle->fd == NULL) || (position < 0)) {
		result = errorf(stderr, "invalid or missing argument(s) { handle "
				"0x%lx, position %i }", (aptrcast)handle, position);
		return result;
	}

	result = fseeko(handle->fd, position, SEEK_SET);
	if(result != 0) {
		result = errorf(&handle->errors,
				"failed to seek to %li on file '%s' with '%s'",
				position, handle->filename, strerror(errno));
	}
	else {
		handle->position = position;
	}

	return result;
}

int file_seekEof(FileHandle *handle)
{
	int result = 0;

	if((handle == NULL) || (handle->fd == NULL)) {
		result = errorf(stderr, "invalid or missing argument(s) { handle "
				"0x%lx }", (aptrcast)handle);
		return result;
	}

	result = fseeko(handle->fd, 0, SEEK_END);
	if(result != 0) {
		result = errorf(&handle->errors,
				"failed to seek to EOF on file '%s' with '%s'",
				handle->filename, strerror(errno));
	}
	else {
		handle->position = ftello(handle->fd);
		if(handle->position < 0) {
			result = errorf(&handle->errors,
					"failed to tell on file '%s' with '%s'",
					handle->filename, strerror(errno));
		}
	}

	return result;
}

int file_sync(FileHandle *handle)
{
	int result = 0;

	if((handle == NULL) || (handle->fd == NULL)) {
		result = errorf(stderr, "invalid or missing argument(s) { handle "
				"0x%lx }", (aptrcast)handle);
		return result;
	}

	result = fflush(handle->fd);
	if(result != 0) {
		result = errorf(&handle->errors, "failed to flush file '%s' with '%s'",
				handle->filename, strerror(errno));
		return result;
	}

	result = file_seek(handle, handle->position);

	return result;
}

int file_getFileLength(FileHandle *handle, alint *length)
{
	int result = 0;
	alint location = 0;

	if((handle == NULL) || (length == NULL)) {
		result = errorf(stderr, "invalid or missing argument(s) { handle "
				"0x%lx, length 0x%lx }", (aptrcast)handle, (aptrcast)length);
		return result;
	}

	location = handle->position;

	result = file_sync(handle);
	if(result < 0) {
		return result;
	}

	result = file_seekEof(handle);
	if(result < 0) {
		return result;
	}

	*length = handle->position;

	result = file_seek(handle, location);
	if(result < 0) {
		return result;
	}

	return result;
}

int file_getError(FileHandle *handle, int errorCode, char **errorMessage)
{
	int result = 0;

	if((handle == NULL) || (errorMessage == NULL)) {
		result = errorf(stderr, "invalid or missing argument(s) { handle "
				"0x%lx, errorMessage 0x%lx }", (aptrcast)handle,
				(aptrcast)errorMessage);
		return result;
	}

	return error_getError(&handle->errors, errorCode, errorMessage);
}

char *file_getErrorMessage(FileHandle *handle, int errorCode)
{
	int rc = 0;
	char *result = NULL;

	if(handle == NULL) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	if((rc = error_getError(&handle->errors, errorCode, &result)) < 0) {
		result = strdup("unkonwn error");
	}

	return result;
}

alint file_getFileLengthOnFilename(char *filename)
{
	int rc = 0;
	alint result = 0;
	char *errorMessage = NULL;

	FileHandle handle;

	if(filename == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if((rc = file_init(&handle, filename, "rb", 0)) < 0) {
		errorMessage = file_getErrorMessage(&handle, rc);

		fprintf(stderr, "[%s():%i] error - failed to open file '%s' with "
				"'%s'.\n",  __FUNCTION__, __LINE__, filename, errorMessage);

		free(errorMessage);

		return -1;
	}

	if((rc = file_getFileLength(&handle, &result)) < 0) {
		errorMessage = file_getErrorMessage(&handle, rc);

		fprintf(stderr, "[%s():%i] error - failed to get file '%s' length "
				"with '%s'.\n",  __FUNCTION__, __LINE__, filename,
				errorMessage);

		free(errorMessage);

		return -1;
	}

	if((rc = file_free(&handle)) < 0) {
		fprintf(stderr, "[%s():%i] error - failed to free file '%s' file "
				"handle.\n", __FUNCTION__, __LINE__, filename);
		return -1;
	}

	return result;
}

