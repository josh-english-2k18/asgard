/*
 * zlib.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The zLib library wrapper API, header file.
 *
 * Written by Josh English.
 */

#if !defined(_CORE_LIBRARY_UTIL_ZLIB_H)

#define _CORE_LIBRARY_UTIL_ZLIB_H

#if !defined(_CORE_H) && !defined(_CORE_LIBRARY_H) && \
		!defined(_CORE_LIBRARY_UTIL_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _CORE_H || _CORE_LIBRARY_H || _CORE_LIBRARY_UTIL_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// define zlib public constants

typedef enum _zLibCompression {
	ZLIB_COMPRESSION_NONE = 0,
	ZLIB_COMPRESSION_FASTEST = 1,
	ZLIB_COMPRESSION_BEST = 9,
	ZLIB_COMPRESSION_DEFAULT = -1,
} zLibCompression;

typedef enum _zLibErrors {
	ZLIB_ERROR_INVALID_ARGS = -1,
	ZLIB_ERROR_DEFLATE_INIT_FAILED = -2,
	ZLIB_ERROR_DEFLATE_FAILED = -3,
	ZLIB_ERROR_DEFLATE_FINISH_FAILED = -4,
	ZLIB_ERROR_DEFLATE_END_FAILED = -5,
	ZLIB_ERROR_COMPRESSION_FAILED = -6,
	ZLIB_ERROR_INFLATE_INIT_FAILED = -7,
	ZLIB_ERROR_INFLATE_FAILED = -8,
	ZLIB_ERROR_INFLATE_END_FAILED = -9,
	ZLIB_ERROR_INFLATE_REINIT_FAILED = -10,
	ZLIB_ERROR_INFLATE_COMPLETE_FAILED = -11,
	ZLIB_ERROR_UNKNOWN = -12,
} zLibErrors;


// declare zlib public functions

int zlib_compress(char *value, int valueLength, zLibCompression flag,
		char **output, int *outputLength);

int zlib_inflate(char *value, int valueLength, char **output,
		int *outputLength);

char *zlib_errorToString(zLibErrors errorCode);


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _CORE_LIBRARY_UTIL_ZLIB_H

