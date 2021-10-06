/*
 * httpd_common.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The httpd server common core function library.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#include "httpd/httpd.h"
#define _HTPD_CORE_COMPONENT
#include "httpd/core/httpd_common.h"


// define httpd common private constants

#define HTTPD_DEFAULT_HTDOCS_DIR						"assets/data/htdocs"

#define HTTPD_DEFAULT_HOSTNAME							"localhost"

#define HTTPD_DEFAULT_LOGFILE							"log/httpd.server.log"

#define HTTPD_DEFAULT_PORT								7305

#define HTTPD_DEFAULT_LOG_LEVEL							LOG_LEVEL_INFO

#define HTTPD_DEFAULT_MINIMUM_THREADS					4

#define HTTPD_DEFAULT_MAXIMUM_THREADS					128

#define HTTPD_DEFAULT_BLOCK_LENGTH						65536

#define HTTPD_DEFAULT_MAXIMUM_BLOCKS					16384

#define HTTPD_MAX_PATH_LENGTH							4096


static HttpdStatusCode HTTP_STATUS_LIST[] = {
	{	HTTPD_STATUS_OK,				200,	"Ok"						},
	{	HTTPD_STATUS_NO_CONTENT,		204,	"No content"				},
	{	HTTPD_STATUS_PARTIAL_CONTENT,	206,	"Partial content"			},
	{	HTTPD_STATUS_BAD_REQUEST,		400,	"Bad request."				},
	{	HTTPD_STATUS_UNAUTHORIZED,		401,	"Unauthorized."				},
	{	HTTPD_STATUS_FORBIDDEN,			403,	"Forbidden."				},
	{	HTTPD_STATUS_NOT_FOUND,			404,	"Not found."				},
	{	HTTPD_STATUS_LENGTH_REQUIRED,	411,	"Length required."			},
	{	HTTPD_STATUS_BAD_RANGE,			416,	"Invalid range requested."	},
	{	HTTPD_STATUS_INTERNAL_ERROR,	500,	"Internal server error."	},
	{	HTTPD_STATUS_NOT_IMPLEMENTED,	501,	"Unimplemented method."		},
	{	HTTPD_STATUS_BAD_VERSION,		505,	"Version not supported."	},
	{	HTTPD_STATUS_BANDWIDTH_EXCEEDED,
										509,	"Bandwidth limit exceeded."	},
	{	HTTPD_STATUS_DISK_ALLOTMENT_EXCEEDED,
										510,	"Disk Allotment Exceeded"	},
	{	HTTPD_STATUS_UNKNOWN,			0,		NULL						}
};


// declare httpd common private functions

static int readConfigList(Config *config, char *listName,
		char ***nameList, char ***valueList, int *listCount);


// define httpd common private functions

static int readConfigList(Config *config, char *listName,
		char ***nameList, char ***valueList, int *listCount)
{
	int ii = 0;
	int id = 0;
	int length = 0;
	char **localNameList = NULL;
	char **localValueList = NULL;

	id = -1;

	for(ii = 0; ii < config->sectionLength; ii++) {
		if(!strcmp(config->sections[ii].name, listName)) {
			id = ii;
			break;
		}
	}

	if(id == -1) {
		return 0;
	}

	*nameList = NULL;
	if(valueList != NULL) {
		*valueList = NULL;
	}

	if((length = config->sections[id].propertyLength) < 1) {
		return 0;
	}

	localNameList = (char **)malloc(sizeof(char *) * (length));

	if(valueList != NULL) {
		localValueList = (char **)malloc(sizeof(char *) * (length));
	}

	for(ii = 0; ii < length; ii++) {
		localNameList[ii] = strdup(config->sections[id].keys[ii]);

		if(valueList != NULL) {
			localValueList[ii] = strdup(config->sections[id].values[ii]);
		}
	}

	*listCount = length;
	*nameList = localNameList;

	if(valueList != NULL) {
		*valueList = localValueList;
	}

	return 0;
}


// define httpd common public functions

void httpdCommon_calculateDiskInUse(HttpdConfig *httpdConfig, char *dirname)
{
	aboolean exists = afalse;
	int rc = 0;
	alint length = 0L;
	char *filename = NULL;
	char *errorMessage = NULL;
	char local[HTTPD_MAX_PATH_LENGTH];

	DirHandle dir;

	if((rc = dir_init(&dir, dirname)) < 0) {
		dir_getError(&dir, rc, &errorMessage);

		log_logf(&httpdConfig->log, LOG_LEVEL_ERROR,
				 "failed to open directory '%s' with '%s'",
				 dirname, errorMessage);

		free(errorMessage);
		dir_free(&dir);
		return;
	}

	while((dir_read(&dir, &filename) == 0) && (filename != NULL)) {
		if((!strcmp(filename, ".")) || (!strcmp(filename, ".."))) {
			free(filename);
			continue;
		}

		length = snprintf(local, ((int)sizeof(local) - 1), "%s/%s",
				dirname, filename);
		if(length >= (int)sizeof(local)) {
			log_logf(&httpdConfig->log, LOG_LEVEL_ERROR,
					"path too large to calculate disk usage");
			exit(1);
		}

		system_fileExists(local, &exists);

		if(exists) {
			mutex_lock(&httpdConfig->protocol.mutex);
			httpdConfig->protocol.tempFileSpaceInUseBytes +=
				file_getFileLengthOnFilename(local);
			mutex_unlock(&httpdConfig->protocol.mutex);
		}

		system_dirExists(local, &exists);

		if(exists) {
			httpdCommon_calculateDiskInUse(httpdConfig, local);
		}

		free(filename);
	}

	dir_free(&dir);
}

HttpdStatusCode *httpdCommon_getStatusCode(HttpdStatusCodes id)
{
	return &HTTP_STATUS_LIST[id];
}

int httpdCommon_decodeUrl(char url[HTTPD_MAX_URL_LENGTH], int urlLength,
		char decodedUrl[HTTPD_MAX_URL_LENGTH], int *decodedUrlLength,
		HttpdUri *uri)
{
	aboolean isName = afalse;
	aboolean isBadChar = afalse;
	aboolean isEndOfFilename = afalse;
	aboolean isEndOfQueryString = afalse;
	int ii = 0;
	int nn = 0;
	int jj = 0;
	int kk = 0;
	int mm = 0;
	int power = 0;
	int lastPathDelimiterPosition = 0;
	char byte = (char)0;

	if((urlLength < 1) || (decodedUrlLength == NULL) || (uri == NULL)) {
		fprintf(stderr, "[%s():%i] error - invalid or missing argument(s).\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	memset(decodedUrl, 0, HTTPD_MAX_URL_LENGTH);
	memset(uri, 0, (int)(sizeof(HttpdUri)));

	*decodedUrlLength = 0;

	for(ii = 0, nn = 0, jj = 0, mm = 0; ii < urlLength; ii++, nn++, jj++) {
		if(url[ii] == '+') {
			decodedUrl[nn] = ' ';
		}
		else if(url[ii] == '%') {
			power = 16;	// change if other than two digits to process
			byte = (char)0;
			isBadChar = afalse;
			for(kk = ii + 1; kk < (ii + 3); kk++) {
				if((url[kk] >= '0') && (url[kk] <= '9')) {
					byte += (url[kk] - '0') * power;
				}
				else if((url[kk] >= 'a') && (url[kk] <= 'f')) {
					byte += ((url[kk] - 'a') + 10) * power;
				}
				else if((url[kk] >= 'A') && (url[kk] <= 'F')) {
					byte += ((url[kk] - 'A') + 10) * power;
				}
				else {
					isBadChar = atrue;
				}
				power /= 16;
			}
			if(isBadChar) {
				decodedUrl[nn] = url[ii];
			}
			else {
				decodedUrl[nn] = byte;
				ii += 2;
			}
		}
		else {
			decodedUrl[nn] = url[ii];
		}

		if((isEndOfFilename) && (!isEndOfQueryString)) {
			uri->queryString[mm] = decodedUrl[nn];
			mm++;
		}

		if((!isEndOfFilename) &&
				((decodedUrl[nn] == ';') ||
				 (decodedUrl[nn] == '?') ||
				 (decodedUrl[nn] == '#') ||
				 (decodedUrl[nn] == '&'))) {
			isEndOfFilename = atrue;
			mm = 0;
		}

		if((isEndOfFilename) && (!isEndOfQueryString) &&
				(decodedUrl[nn] == '#')) {
			isEndOfQueryString = atrue;
			uri->queryStringLength = mm;
			uri->queryString[mm] = '\0';
			mm = 0;
		}

		if(!isEndOfFilename) {
			uri->filename[jj] = decodedUrl[nn];
		}

		if(decodedUrl[nn] == '/') {
			lastPathDelimiterPosition = nn;
			if(!isEndOfFilename) {
				jj = -1;
			}
		}

		uri->path[nn] = decodedUrl[nn];
	}

	uri->pathLength = (lastPathDelimiterPosition + 1);
	uri->path[(lastPathDelimiterPosition + 1)] = '\0';

	if(jj >= 0) {
		uri->filenameLength = jj;
		uri->filename[jj] = '\0';
	}

	if(!isEndOfQueryString) {
		uri->queryStringLength = mm;
		uri->queryString[mm] = '\0';
	}

	*decodedUrlLength = nn;

	// create query string argument(s) list

	isName = atrue;

	for(ii = 0, nn = 0, jj = 0; ii < uri->queryStringLength; ii++) {
		if(uri->queryString[ii] == '=') {
			if(isName) {
				isName = afalse;
				uri->args[jj].name[nn] = '\0';
				uri->args[jj].nameLength = nn;
				nn = 0;
				continue;
			}
		}
		else if(uri->queryString[ii] == '&') {
			if(!isName) {
				uri->args[jj].value[nn] = '\0';
				uri->args[jj].valueLength = nn;
			}

			isName = atrue;
			nn = 0;

			jj++;
			if(jj >= HTTPD_MAX_URL_ARG_COUNT) {
				break;
			}

			continue;
		}

		if(isName) {
			uri->args[jj].name[nn] = uri->queryString[ii];
			nn++;
			if(nn >= HTTPD_MAX_URL_ARG_NAME_LENGTH) {
				break;
			}
		}
		else {
			uri->args[jj].value[nn] = uri->queryString[ii];
			nn++;
			if(nn >= HTTPD_MAX_URL_ARG_VALUE_LENGTH) {
				break;
			}
		}
	}

	if(uri->queryStringLength > 0) {
		uri->argCount = (jj + 1);
	}

	return 0;
}

int httpdCommon_readConfig(Config *config, char *filename,
		HttpdConfig *httpdConfig)
{
	aboolean bValue = afalse;
	int length = 0;
	int iValue = 0;
	alint tempFileSpaceAllowedBytes = 0;
	char *sValue = NULL;
	char buffer[1024];

	memset((void *)httpdConfig, 0, sizeof(HttpdConfig));

	if(config_init(config, filename) < 0) {
		fprintf(stderr, "[%s():%d] - failed to open config file '%s'.\n",
				__FUNCTION__, __LINE__, filename);
		config_free(config);
		return -1;
	}

	// general settings

	config_getString(config, "general", "hostname",
			HTTPD_DEFAULT_HOSTNAME, &sValue);

	httpdConfig->hostname = strdup(sValue);

	config_getInteger(config, "general", "port", HTTPD_DEFAULT_PORT, &iValue);

	httpdConfig->port = iValue;

	config_getBoolean(config, "general", "writeApacheLog", atrue, &bValue);

	httpdConfig->writeApacheLog = bValue;

	config_getString(config, "general", "documentRoot",
			HTTPD_DEFAULT_HTDOCS_DIR, &sValue);

	httpdConfig->documentRoot = strdup(sValue);

	config_getBoolean(config, "general", "allowFileRetrieval", atrue, &bValue);

	httpdConfig->allowFileRetrieval = bValue;

	config_getBoolean(config, "general", "allowDirectoryListing", atrue,
			&bValue);

	httpdConfig->allowDirectoryListing = bValue;

	config_getBoolean(config, "general", "enablePut", afalse, &bValue);

	httpdConfig->enablePut = bValue;

	config_getBoolean(config, "general", "enableDelete", afalse, &bValue);

	httpdConfig->enableDelete = bValue;

	config_getBoolean(config, "general", "allowPutToOverwrite", afalse,
			&bValue);

	httpdConfig->allowPutToOverwrite = bValue;

	config_getBoolean(config, "general", "allowCreateDirectory", afalse,
			&bValue);

	httpdConfig->allowCreateDirectory = bValue;

	config_getString(config, "general", "logfile", HTTPD_DEFAULT_LOGFILE,
			&sValue);

	httpdConfig->logfile = strdup(sValue);

	snprintf(buffer, ((int)sizeof(buffer) - 1), "%i", HTTPD_DEFAULT_LOG_LEVEL);

	config_getString(config, "general", "logLevel", buffer, &sValue);

	if(strcasecmp(sValue, "debug") == 0) {
		httpdConfig->logLevel = LOG_LEVEL_DEBUG;
	}
	else if(strcasecmp(sValue, "info") == 0) {
		httpdConfig->logLevel = LOG_LEVEL_INFO;
	}
	else if(strcasecmp(sValue, "warning") == 0) {
		httpdConfig->logLevel = LOG_LEVEL_WARNING;
	}
	else if(strcasecmp(sValue, "error") == 0) {
		httpdConfig->logLevel = LOG_LEVEL_ERROR;
	}
	else if(strcasecmp(sValue, "panic") == 0) {
		httpdConfig->logLevel = LOG_LEVEL_PANIC;
	}
	else {
		httpdConfig->logLevel = atoi(sValue);
	}

	config_getString(config, "general", "documentSpaceAllowed", "0", &sValue);

	tempFileSpaceAllowedBytes = atoi(sValue);

	if((length = (strlen(sValue) - 1)) > 0) {
		if((sValue[length] == 'K') || (sValue[length] == 'k')) {
			tempFileSpaceAllowedBytes *= 1024LL;
		}
		else if((sValue[length] == 'M') || (sValue[length] == 'm')) {
			tempFileSpaceAllowedBytes *= (1024LL * 1024LL);
		}
		else if((sValue[length] == 'G') || (sValue[length] == 'g')) {
			tempFileSpaceAllowedBytes *= (1024LL * 1024LL * 1024LL);
		}
		else if((sValue[length] == 'T') || (sValue[length] == 't')) {
			tempFileSpaceAllowedBytes *= (1024LL * 1024LL * 1024LL * 1024LL);
		}
	}

	http_protocol_init(&httpdConfig->protocol,
			atrue,						// is temp file mode
			1048576,					// temp-file threshold bytes
			tempFileSpaceAllowedBytes,	// temp-file allowed bytes
			0,							// temp-file in-use bytes
			"/tmp"						// temp-file base path
		);

	config_getInteger(config, "general", "megabitsPerSecondAllowed", 0,
			&iValue);

	httpdConfig->megabitsAllowed = iValue;

	// set up log

	if(strcasecmp(httpdConfig->logfile, "stdout") == 0) {
		log_init(&httpdConfig->log, LOG_OUTPUT_STDOUT, NULL,
				httpdConfig->logLevel);
	}
	else if(strcasecmp(httpdConfig->logfile, "stderr") == 0) {
		log_init(&httpdConfig->log, LOG_OUTPUT_STDERR, NULL,
				httpdConfig->logLevel);
	}
	else {
		log_init(&httpdConfig->log, LOG_OUTPUT_FILE, httpdConfig->logfile,
				httpdConfig->logLevel);
	}

	// server settings

	config_getInteger(config, "server", "minimumThreads",
			HTTPD_DEFAULT_MINIMUM_THREADS, &iValue);

	httpdConfig->minimumThreads = iValue;

	config_getInteger(config, "server", "maximumThreads",
			HTTPD_DEFAULT_MINIMUM_THREADS, &iValue);

	httpdConfig->maximumThreads = iValue;

	// cache settings

	config_getInteger(config, "cache", "blockLength",
			HTTPD_DEFAULT_BLOCK_LENGTH, &iValue);

	httpdConfig->blockLength = iValue;

	config_getInteger(config, "cache", "maximumBlocks",
			HTTPD_DEFAULT_MAXIMUM_BLOCKS, &iValue);

	httpdConfig->maximumBlocks = iValue;

	// miscellaneous lists

	readConfigList(config, "directory.include.list",
						  &httpdConfig->directoryIncludeList, NULL,
						  &httpdConfig->directoryIncludeListLength);

	readConfigList(config, "directory.exclude.list",
						  &httpdConfig->directoryExcludeList, NULL,
						  &httpdConfig->directoryExcludeListLength);

	readConfigList(config, "disable.functions",
						  &httpdConfig->disableFunctionsList, NULL,
						  &httpdConfig->disableFunctionsListLength);

	httpdCommon_calculateDiskInUse(httpdConfig, httpdConfig->documentRoot);

	if((httpdConfig->protocol.tempFileSpaceAllowedBytes > 0) &&
			(httpdConfig->protocol.tempFileSpaceInUseBytes >
			 httpdConfig->protocol.tempFileSpaceAllowedBytes)) {
		log_logf(&httpdConfig->log, LOG_LEVEL_ERROR,
				 "too much disk space in use, allowed %.2fMB, using %.2fMB",
				 (double)(
					 (double)httpdConfig->protocol.tempFileSpaceAllowedBytes
						  / (1024.0 * 1024.0)),
				 (double)(
					 (double)httpdConfig->protocol.tempFileSpaceInUseBytes
						  / (1024.0 * 1024.0))
			 );
	}

	return 0;
}

void httpdCommon_freeConfig(HttpdConfig *httpdConfig)
{
	int ii = 0;

	if(httpdConfig->hostname != NULL) {
		free(httpdConfig->hostname);
	}

	if(httpdConfig->documentRoot != NULL) {
		free(httpdConfig->documentRoot);
	}

	if(httpdConfig->directoryIncludeList != NULL) {
		for(ii = 0; ii < httpdConfig->directoryIncludeListLength; ii++) {
			if(httpdConfig->directoryIncludeList[ii] != NULL) {
				free(httpdConfig->directoryIncludeList[ii]);
			}
		}

		free(httpdConfig->directoryIncludeList);
	}

	if(httpdConfig->directoryExcludeList != NULL) {
		for(ii = 0; ii < httpdConfig->directoryExcludeListLength; ii++) {
			if(httpdConfig->directoryExcludeList[ii] != NULL) {
				free(httpdConfig->directoryExcludeList[ii]);
			}
		}

		free(httpdConfig->directoryExcludeList);
	}

	if(httpdConfig->disableFunctionsList != NULL) {
		for(ii = 0; ii < httpdConfig->disableFunctionsListLength; ii++) {
			if(httpdConfig->disableFunctionsList[ii] != NULL) {
				free(httpdConfig->disableFunctionsList[ii]);
			}
		}

		free(httpdConfig->disableFunctionsList);
	}

	if(httpdConfig->logfile != NULL) {
		free(httpdConfig->logfile);
	}

	if(httpdConfig->htaccessUsername != NULL) {
		free(httpdConfig->htaccessUsername);
	}

	if(httpdConfig->htaccessPassword != NULL) {
		free(httpdConfig->htaccessPassword);
	}

	http_protocol_free(&httpdConfig->protocol);
}

