/*
 * zlib.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The zLib library wrapper API.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#define _CORE_LIBRARY_TYPES_COMPONENT
#include "core/library/util/zlib.h"


// define zlib private constants

#define ZLIB_MEMORY_MARGIN_BYTES				128


// define zlib public functions

int zlib_compress(char *value, int valueLength, zLibCompression flag,
		char **output, int *outputLength)
{
	int rc = 0;
	char *buffer = NULL;

	z_stream zStream;

	if((value == NULL) || (valueLength < 1) || (output == NULL) ||
			(outputLength == NULL)) {
		DISPLAY_INVALID_ARGS;
		return ZLIB_ERROR_INVALID_ARGS;
	}

	*output = NULL;
	*outputLength = 0;

	if((flag < ZLIB_COMPRESSION_NONE) || (flag > ZLIB_COMPRESSION_BEST)) {
		flag = ZLIB_COMPRESSION_DEFAULT;
	}

	memset((void *)&zStream, 0, sizeof(z_stream));

    zStream.zalloc = (alloc_func)NULL;
    zStream.zfree = (free_func)NULL;
    zStream.opaque = (voidpf)NULL;

	if(deflateInit2(&zStream,
				(int)flag,
				Z_DEFLATED,
				15, // window bits
				9, // memory level
				Z_DEFAULT_STRATEGY\
			) != Z_OK) {
		return ZLIB_ERROR_DEFLATE_INIT_FAILED;
	}

	buffer = (char *)malloc(sizeof(char) *
			(valueLength + ZLIB_MEMORY_MARGIN_BYTES));

	zStream.next_in = (Byte *)value;
	zStream.next_out = (Byte *)buffer;
	zStream.avail_in = valueLength;
	zStream.avail_out = valueLength;

	while((zStream.total_in != valueLength) &&
			((int)zStream.total_out < valueLength)) {
		if(deflate(&zStream, Z_NO_FLUSH) != Z_OK) {
			free(buffer);
			return ZLIB_ERROR_DEFLATE_FAILED;
		}
	}

	rc = deflate(&zStream, Z_FINISH);
	if((rc != Z_STREAM_END) && (rc != Z_OK)) {
		free(buffer);
		return ZLIB_ERROR_DEFLATE_FINISH_FAILED;
	}

	if(deflateEnd(&zStream) != Z_OK) {
		free(buffer);
		return ZLIB_ERROR_DEFLATE_END_FAILED;
	}

	if((int)zStream.total_out >= valueLength) {
		free(buffer);
		return ZLIB_ERROR_COMPRESSION_FAILED;
	}

	*output = buffer;
	*outputLength = (int)zStream.total_out;

	return 0;
}

int zlib_inflate(char *value, int valueLength, char **output,
		int *outputLength)
{
	int rc = 0;
	int bufferLength = 0;
	char *buffer = NULL;

	z_stream zStream;

	if((value == NULL) || (valueLength < 1) || (output == NULL) ||
			(outputLength == NULL)) {
		DISPLAY_INVALID_ARGS;
		return ZLIB_ERROR_INVALID_ARGS;
	}

	*output = NULL;
	*outputLength = 0;

	bufferLength = (valueLength * 2);
	buffer = (char *)malloc(sizeof(char) *
			(bufferLength + ZLIB_MEMORY_MARGIN_BYTES));

	memset(&zStream, 0, (sizeof(z_stream)));

    zStream.zalloc = (alloc_func)NULL;
    zStream.zfree = (free_func)NULL;
    zStream.opaque = (voidpf)NULL;
	zStream.next_in = (Byte *)value;
	zStream.next_out = (Byte *)buffer;
	zStream.avail_in = valueLength;
	zStream.avail_out = bufferLength;

    if(inflateInit(&zStream) != Z_OK) {
		free(buffer);
		return ZLIB_ERROR_INFLATE_INIT_FAILED;
	}

	while((int)zStream.total_in < valueLength) {
		if((rc = inflate(&zStream, Z_NO_FLUSH)) != Z_OK) {
			if(rc != Z_STREAM_END) {
				free(buffer);
				return ZLIB_ERROR_INFLATE_FAILED;
			}
			else {
				break;
			}
		}

		if((int)zStream.total_out >= bufferLength) {
			if(inflateEnd(&zStream) != Z_OK) {
				free(buffer);
				return ZLIB_ERROR_INFLATE_END_FAILED;
			}

			bufferLength = ((bufferLength + zStream.total_out) * 2);
			buffer = (char *)realloc(buffer,
					(sizeof(char) * (bufferLength + ZLIB_MEMORY_MARGIN_BYTES)));

			memset(&zStream, 0, (sizeof(z_stream)));

		    zStream.zalloc = (alloc_func)NULL;
		    zStream.zfree = (free_func)NULL;
		    zStream.opaque = (voidpf)NULL;
			zStream.next_in = (Byte *)value;
			zStream.next_out = (Byte *)buffer;
			zStream.avail_in = valueLength;
			zStream.avail_out = bufferLength;
			zStream.total_in = 0;
			zStream.total_out = 0;

		    if(inflateInit(&zStream) != Z_OK) {
				free(buffer);
				return ZLIB_ERROR_INFLATE_REINIT_FAILED;
			}
		}
	}

	if(inflateEnd(&zStream) != Z_OK) {
		free(buffer);
		return ZLIB_ERROR_INFLATE_COMPLETE_FAILED;
	}

	*output = buffer;
	*outputLength = (int)zStream.total_out;

	return 0;
}

char *zlib_errorToString(zLibErrors errorCode)
{
	char *result = "Unknown";

	switch(errorCode) {
		case ZLIB_ERROR_INVALID_ARGS:
			result = "missing or invalid argument(s)";
			break;

		case ZLIB_ERROR_DEFLATE_INIT_FAILED:
			result = "deflate initialization failed";
			break;

		case ZLIB_ERROR_DEFLATE_FAILED:
			result = "default failed";
			break;

		case ZLIB_ERROR_DEFLATE_FINISH_FAILED:
			result = "deflate finish failed";
			break;

		case ZLIB_ERROR_DEFLATE_END_FAILED:
			result = "deflate end failed";
			break;

		case ZLIB_ERROR_COMPRESSION_FAILED:
			result = "compression failed";
			break;

		case ZLIB_ERROR_INFLATE_INIT_FAILED:
			result = "inflate initialization failed";
			break;

		case ZLIB_ERROR_INFLATE_FAILED:
			result = "inflate failed";
			break;

		case ZLIB_ERROR_INFLATE_END_FAILED:
			result = "inflate end failed";
			break;

		case ZLIB_ERROR_INFLATE_REINIT_FAILED:
			result = "infalate re-initialization failed";
			break;

		case ZLIB_ERROR_INFLATE_COMPLETE_FAILED:
			result = "inflate completion failed";
			break;

		case ZLIB_ERROR_UNKNOWN:
		default:
			break;
	}

	return result;
}

