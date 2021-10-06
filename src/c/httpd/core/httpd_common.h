/*
 * httpd_common.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The httpd server common core function library, header file.
 *
 * Written by Josh English.
 */

#if !defined(_HTTP_CORE_HTTPD_COMMON_H)

#define _HTTP_CORE_HTTPD_COMMON_H

#if !defined(_CORE_H) && !defined(_HTTPD_H) && \
		!defined(_HTPD_CORE_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _CORE_H || _HTTPD_H || _HTPD_CORE_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// define httpd common public data types

typedef struct _HttpdUriArgs {
	int nameLength;
	int valueLength;
	char name[HTTPD_MAX_URL_ARG_NAME_LENGTH];
	char value[HTTPD_MAX_URL_ARG_VALUE_LENGTH];
} HttpdUriArgs;

typedef struct _HttpdUri {
	int pathLength;
	int filenameLength;
	int queryStringLength;
	int argCount;
	char path[HTTPD_MAX_URL_LENGTH];
	char filename[HTTPD_MAX_URL_LENGTH];
	char queryString[HTTPD_MAX_URL_LENGTH];
	HttpdUriArgs args[HTTPD_MAX_URL_ARG_COUNT];
} HttpdUri;


// declare httpd common public functions

void httpdCommon_calculateDiskInUse(HttpdConfig *httpdConfig, char *dirname);

HttpdStatusCode *httpdCommon_getStatusCode(HttpdStatusCodes id);

int httpdCommon_decodeUrl(char url[HTTPD_MAX_URL_LENGTH], int urlLength,
		char decodedUrl[HTTPD_MAX_URL_LENGTH], int *decodedUrlLength,
		HttpdUri *uri);

int httpdCommon_readConfig(Config *config, char *filename,
		HttpdConfig *httpdConfig);

void httpdCommon_freeConfig(HttpdConfig *httpdConfig);


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _HTTP_CORE_HTTPD_COMMON_H

