/*
 * httpd.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The http server common header file.
 *
 * Written by Josh English.
 */

#if !defined(_HTTPD_H)

#define _HTTPD_H

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// define httpd common public constants

#define HTTPD_MAX_HEADER_NAME_LENGTH						128

#define HTTPD_MAX_HEADER_VALUE_LENGTH						1024

#define HTTPD_MAX_HEADER_COUNT								64

#define HTTPD_MAX_URL_LENGTH								65536

#define HTTPD_MAX_URL_ARG_NAME_LENGTH						128

#define HTTPD_MAX_URL_ARG_VALUE_LENGTH						1024

#define HTTPD_MAX_URL_ARG_COUNT								64

#define HTTPD_HTTP_VERSION									"1.1"

#define HTTPD_SERVER_NAME									"Asgard Web Server"

#define HTTPD_SERVER_VERSION								"0.0.1 - Alpha"


typedef enum _HttpdRequestMethods {
	HTTPD_REQUEST_GET = 0,
	HTTPD_REQUEST_HEAD,
	HTTPD_REQUEST_POST,
	HTTPD_REQUEST_PUT,
	HTTPD_REQUEST_TRACE,
	HTTPD_REQUEST_DELETE,	// an extension to HTTP
	HTTPD_REQUEST_UNKNOWN,
} HttpdRequestMethods;

typedef enum _HttpdStatusCodes {
	/*
	 * success codes
	 */
	HTTPD_STATUS_OK = 0,
	HTTPD_STATUS_NO_CONTENT,
	HTTPD_STATUS_PARTIAL_CONTENT,

	/*
	 * client errors
	 */
	HTTPD_STATUS_BAD_REQUEST,
	HTTPD_STATUS_UNAUTHORIZED,
	HTTPD_STATUS_FORBIDDEN,
	HTTPD_STATUS_NOT_FOUND,
	HTTPD_STATUS_LENGTH_REQUIRED,
	HTTPD_STATUS_BAD_RANGE,

	/*
	 * server errors
	 */
	HTTPD_STATUS_INTERNAL_ERROR,
	HTTPD_STATUS_NOT_IMPLEMENTED,
	HTTPD_STATUS_BAD_VERSION,
	HTTPD_STATUS_BANDWIDTH_EXCEEDED,
	HTTPD_STATUS_DISK_ALLOTMENT_EXCEEDED,

	/*
	 * end of list
	 */
	HTTPD_STATUS_UNKNOWN
} HttpdStatusCodes;

typedef enum _HttpdFunctionResponseTypes {
	HTTPD_FUNCTION_RESPONSE_TYPE_TEXT = 1,
	HTTPD_FUNCTION_RESPONSE_TYPE_GZIP,
	HTTPD_FUNCTION_RESPONSE_TYPE_JFIF,
	HTTPD_FUNCTION_RESPONSE_TYPE_BINARY,
	HTTPD_FUNCTION_RESPONSE_TYPE_UNKNOWN,
	HTTPD_FUNCTION_RESPONSE_TYPE_ERROR = -1
} HttpdFunctionResponseTypes;

typedef enum _HttpdResponseHeaders {
	HTTPD_RESPONSE_HEADER_ACCEPT_RANGES = 0,
	HTTPD_RESPONSE_HEADER_CACHE_CONTROL,
	HTTPD_RESPONSE_HEADER_CONTENT_ENCODING,
	HTTPD_RESPONSE_HEADER_CONTENT_LANGUAGE,
	HTTPD_RESPONSE_HEADER_CONTENT_LENGTH,
	HTTPD_RESPONSE_HEADER_CONTENT_LOCATION,
	HTTPD_RESPONSE_HEADER_CONTENT_TYPE,
	HTTPD_RESPONSE_HEADER_CONTENT_DISPOSITION,
	HTTPD_RESPONSE_HEADER_CONTENT_MD5,
	HTTPD_RESPONSE_HEADER_CONTENT_RANGE,
	HTTPD_RESPONSE_HEADER_ETAG,
	HTTPD_RESPONSE_HEADER_EXPIRES,
	HTTPD_RESPONSE_HEADER_LAST_MODIFIED,
	HTTPD_RESPONSE_HEADER_LOCATION,
	HTTPD_RESPONSE_HEADER_PRAGMA,
	HTTPD_RESPONSE_HEADER_RETRY_AFTER,
	HTTPD_RESPONSE_HEADER_SET_COOKIE,
	HTTPD_RESPONSE_HEADER_WWW_AUTHENTICATE,
	HTTPD_RESPONSE_HEADER_UNKNOWN
} HttpdResponseHeaders;


// define httpd common data types

typedef struct _HttpdRequestMethod {
	HttpdRequestMethods code;
	char *name;
	int nameLength;
} HttpdRequestMethod;

typedef struct _HttpdStatusCode {
	HttpdStatusCodes id;
	int code;
	char *description;
} HttpdStatusCode;

typedef struct _HttpdResponseHeader {
	HttpdResponseHeaders id;
	char *name;
	int nameLength;
} HttpdResponseHeader;

typedef struct _HttpdConfig {
	char *hostname;
	int port;
	aboolean writeApacheLog;
	char *documentRoot;
	aboolean allowFileRetrieval;
	aboolean allowDirectoryListing;
	int directoryIncludeListLength;
	char **directoryIncludeList;
	int directoryExcludeListLength;
	char **directoryExcludeList;
	int disableFunctionsListLength;
	char **disableFunctionsList;
	aboolean enablePut;
	aboolean enableDelete;
	aboolean allowPutToOverwrite;
	aboolean allowCreateDirectory;	// for "put" files
	char *logfile;
	int logLevel;
	Log log;
	HttpProtocol protocol;
	double megabitsAllowed;
	// server settings
	int minimumThreads;
	int maximumThreads;
	// cache settings
	int blockLength;
	int maximumBlocks;
	// access settings
	char *htaccessUsername;
	char *htaccessPassword;
} HttpdConfig;


// httpd core packages

#include "httpd/core/httpd_common.h"
//#include "httpd/core/httpd_request_parser.h"
//#include "httpd/core/httpd_response_header.h"
//#include "httpd/core/httpd_function_manager.h"


// httpd system packages

//#include "httpd/system/httpd_request_handler.h"


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _HTTPD_H

