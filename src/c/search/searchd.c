/*
 * searchd.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The search engine daemon.
 *
 * Written by Josh English.
 */

// preprocessor directives

#define _CORE_COMMON_BYPASS_STD_STREAM
#include "core/core.h"
#include "search/search.h"


// define search daemon private data types

typedef struct _SearchdOptions {
	aboolean displayHelp;
	char *configurationFilename;
	char *stateFilename;
	char *dataFilename;
} SearchdOptions;

typedef struct _SearchdConfig {
	/*
	 * logging properties
	 */
	int logLevel;
	int logOutput;
	char *logFilename;

	/*
	 * http protocol
	 */
	aboolean httpIsTempFileMode;
	alint httpTempFileThresholdBytes;
	alint httpTempFileSpaceAllowedBytes;
	alint httpTempFileSpaceInUseBytes;
	char *httpTempFileBasePath;

	/*
	 * server threading properties
	 */
	aboolean threadIsThreadPoolShrinkEnabled;
	int threadMinThreadPoolLength;
	int threadMaxThreadPoolLength;

	/*
	 * server packets properties
	 */
	aboolean packetsIsDumpPacketsEnabled;
	aboolean packetsUseIndividualFiles;
	aboolean packetsUsePacketHeaders;
	char *packetsPath;
	char *packetsFilename;

	/*
	 * server socket properties
	 */
	char *socketHostname;
	int socketPort;

	/*
	 * search engine properties
	 */
	SearchJsonValidationType validationType;
	alint searchMaxContainerCount;
	alint searchMaxContainerMemoryLength;
	double searchContainerTimeout;
	double searchStateWriteThresholdSeconds;
	double searchContainerWriteThresholdSeconds;
	int searchMinStringLength;
	int searchMaxStringLength;
	alint maxSortOperationMemoryLength;
	char *searchStringDelimiters;
	char *searchStatePath;
	char *searchContainerPath;
	char *searchExcludedWordConfigFilename;
	char *authenticationConfigFilename;
} SearchdConfig;

typedef struct _SearchdHandlerArgs {
	Log *jsonMessageLog;
	Log *sqlMessageLog;
	SearchEngine *engine;
} SearchdHandlerArgs;


// declare search daemon private functions

// general functions

static int handleCommandLineOptions(int argc, char *argv[],
		SearchdOptions *options);

static char *buildErrorResponse(char *errorName);

static char *getUrlFromMessage(char *message, int messageLength);

// handlers

static void *searchd_handleDefault(void *context, void *request,
		int *memorySizeOfResponse);

static void *searchd_handleGetApacheServerStatus(void *context, void *request,
		int *memorySizeOfResponse);

static void *searchd_handleGetServerStatus(void *context, void *request,
		int *memorySizeOfResponse);

static void *searchd_handleJsonMessage(void *context, void *request,
		int *memorySizeOfResponse);

static void *searchd_handleSqlMessage(void *context, void *request,
		int *memorySizeOfResponse);

// server functions

static void searchd_signalShutdown(int signalType);

static int searchd_loadConfiguration(SearchdConfig *searchdConfig,
		char *filename);

static int searchd_execute(SearchdConfig *config, SearchdOptions *options);


// define search daemon global variables

static aboolean serverIsRunning = afalse;


// main function

int main(int argc, char *argv[])
{
	int rc = 0;

	SearchdConfig config;
	SearchdOptions options;

	signal_registerDefault();
	signal_registerAction(SIGNAL_INTERRUPT, searchd_signalShutdown);
	signal_registerAction(SIGNAL_TERMINATE, searchd_signalShutdown);

	printf("Asgard Search Engine Daemon (Asgard Ver %s on %s)\n",
			ASGARD_VERSION, ASGARD_DATE);

	if((rc = handleCommandLineOptions(argc, argv, &options)) < 0) {
		fprintf(stderr, "error - invalid or missing argument(s).\n");
	}

	if(options.displayHelp) {
		printf("usage: searchd [options]\n");
		printf("\t -c [configuration filename]\n");
		printf("\t -s [state filename to restore]\n");
		printf("\t -d [data filename to restore]\n");
		printf("\t -h (display usage)\n");

		if(rc < 0) {
			return 1;
		}
		return 0;
	}

	fprintf(stdout, "\nLoading configuration file '%s'...",
			options.configurationFilename);
	fflush(stdout);

	if(searchd_loadConfiguration(&config, options.configurationFilename) < 0) {
		fprintf(stdout, "FAILED\n");
		return 1;
	}

	fprintf(stdout, "OK.\n");

	if(searchd_execute(&config, &options) < 0) {
		return 1;
	}

	// cleanup

	if(options.configurationFilename != NULL) {
		free(options.configurationFilename);
	}

	if(options.stateFilename != NULL) {
		free(options.stateFilename);
	}

	if(options.dataFilename != NULL) {
		free(options.dataFilename);
	}

	return 0;
}


// define search daemon private functions

// general functions

static int handleCommandLineOptions(int argc, char *argv[],
		SearchdOptions *options)
{
	aboolean hasError = afalse;
	int opt = 0;

	memset(options, 0, ((int)sizeof(SearchdOptions)));

	options->displayHelp = afalse;
	options->stateFilename = NULL;
	options->dataFilename = NULL;

#if !defined(WIN32)
	while((opt = getopt(argc, argv, "c:s:d:h?")) != -1) {
		switch(opt) {
			case 'c':	// configuration file
				options->configurationFilename = strdup(optarg);
				break;

			case 's':	// state file
				options->stateFilename = strdup(optarg);
				break;

			case 'd':	// data file
				options->dataFilename = strdup(optarg);
				break;

			case 'h':	// display help
			case '?':	// display help
				options->displayHelp = atrue;
				break;

			default:	// unknown option
				fprintf(stderr, "error - invalid argument '%c' ('%s').\n",
						opt, optarg);
				hasError = atrue;
				break;
		}
	}
#endif // !WIN32

	if(hasError) {
		options->displayHelp = atrue;
		return -1;
	}

	if(options->configurationFilename == NULL) {
		options->configurationFilename = strdup("conf/searchd.default.config");
	}

	return 0;
}

static char *buildErrorResponse(char *errorName)
{
	char *result = NULL;

	result = (char *)malloc(sizeof(char) * 128);

	if(!strcmp(result, "malformed-request-object")) {
		strcpy(result,
				"The JSON request object is missing one or more fields.");
	}
	else if(!strcmp(result, "invalid-request-object")) {
		strcpy(result, "The JSON request object is not valid.");
	}
	else if(!strcmp(result, "missing-request-object")) {
		strcpy(result, "Failed to locate JSON request object, "
				"expecting POST-field 'SearchEngineDaemonMessage'.");
	}
	else {
		snprintf(result, ((int)sizeof(char) * 128),
				"Failed with error '%s'.", errorName);
	}

	return result;
}

static char *getUrlFromMessage(char *message, int messageLength)
{
	aboolean isVisible = afalse;
	int ii = 0;
	int nn = 0;
	int length = 0;
	char *result = NULL;

	length = 128;
	result = (char *)malloc(sizeof(char) * (length + 1));

	for(ii = 0, nn = 0; ii < messageLength; ii++) {
		if(!isVisible) {
			if(message[ii] == ' ') {
				isVisible = atrue;
			}
			continue;
		}

		if(message[ii] == ' ') {
			if((ii <= (messageLength - 4)) &&
					(message[(ii + 1)] == 'H') &&
					(message[(ii + 2)] == 'T') &&
					(message[(ii + 3)] == 'T') &&
					(message[(ii + 4)] == 'P')) {
				break;
			}
		}

		result[nn] = message[ii];
		nn++;

		if(nn >= length) {
			length *= 2;
			result = (char *)realloc(result, (sizeof(char) * (length + 1)));
		}
	}

	return result;
}

// handlers

static void *searchd_handleDefault(void *context, void *request,
		int *memorySizeOfResponse)
{
	HttpTransactionManagerReceive *receive = NULL;
	HttpTransactionManagerSend *result = NULL;

	if((context == NULL) || (request == NULL) ||
			(memorySizeOfResponse == NULL)) {
		DISPLAY_INVALID_ARGS;

		if(memorySizeOfResponse != NULL) {
			*memorySizeOfResponse = 0;
		}

		return NULL;
	}

	*memorySizeOfResponse = 0;

	receive = (HttpTransactionManagerReceive *)request;

	if(SEARCHD_DEVELOPER_MODE) {
		common_display(stdout, receive->payload, receive->payloadLength);
		fprintf(stdout, "\n\n--------\n%s\n--------\n", receive->payload);
	}

	result = (HttpTransactionManagerSend *)malloc(
			sizeof(HttpTransactionManagerSend));

	result->payloadLength = 2048;
	result->payload = (char *)malloc(sizeof(char) *
			(result->payloadLength + 1));

	result->payloadLength = snprintf(result->payload,
			result->payloadLength,
			"HTTP/1.1 200 OK\x0d\x0a"
			"Server: Search Daemon (Asgard Ver %s on %s)\x0d\x0a"
			"\x0d\x0a"
			"<html>\n"
			"	<head>\n"
			"		<title>%s - %s</title>\n"
			"	</head>\n"
			"	<body>\n"
			"		<h1>%s - %s</h1>\n"
			"		<address>Asgard %s Ver %s</address>\n"
			"	</body>\n"
			"</html>\n",
			ASGARD_VERSION,
			ASGARD_DATE,
			"Search Engine Daemon",
			"Default Response",
			"Search Engine Daemon",
			"Default Response",
			ASGARD_VERSION,
			ASGARD_DATE
		);

	*memorySizeOfResponse = result->payloadLength;

	return result;
}

static void *searchd_handleGetApacheServerStatus(void *context, void *request,
		int *memorySizeOfResponse)
{
	int ii = 0;
	int busyWorkers = 0;
	int idleWorkers = 0;
	int currentThreadCount = 0;
	alint bytesPerReq = 0;
	alint totalKbytes = 0;
	alint totalAccesses = 0;
	double uptime = 0.0;
	double reqPerSec = 0.0;
	double bytesPerSec = 0.0;
	double totalTransactionTime = 0;
	char *scoreboard = NULL;

	Server *server = NULL;
	HttpTransactionManagerReceive *receive = NULL;
	HttpTransactionManagerSend *result = NULL;

	const char *TEMPLATE = ""
		"HTTP/1.1 200 OK\x0d\x0a"
		"Server: Search Daemon (Asgard Ver %s on %s)\x0d\x0a"
		"\x0d\x0a"
		"Total Accesses: %i\n"
		"Total kBytes: %i\n"
		"CPULoad: 0.0\n"
		"Uptime: %i\n"
		"ReqPerSec: %0.9f\n"
		"BytesPerSec: %0.2f\n"
		"BytesPerReq: %i\n"
		"BusyWorkers: %i\n"
		"IdleWorkers: %i\n"
		"Scoreboard: %s"
		"";

	if((context == NULL) || (request == NULL) ||
			(memorySizeOfResponse == NULL)) {
		DISPLAY_INVALID_ARGS;

		if(memorySizeOfResponse != NULL) {
			*memorySizeOfResponse = 0;
		}

		return NULL;
	}

	*memorySizeOfResponse = 0;

	server = (Server *)context;
	receive = (HttpTransactionManagerReceive *)request;

	if(SEARCHD_DEVELOPER_MODE) {
		common_display(stdout, receive->payload, receive->payloadLength);
		fprintf(stdout, "\n\n--------\n%s\n--------\n", receive->payload);
	}

	// lookup apache-style statistics

	transactionManager_getMetrics(server->manager, &totalAccesses,
				&totalTransactionTime);

	totalKbytes = ((socket_getBytesSent(&server->socket) +
				socket_getBytesReceived(&server->socket)) / 1024);

	uptime = time_getElapsedMusInSeconds(server->startTimestamp);

	reqPerSec = ((double)totalAccesses / uptime);

	bytesPerSec = ((double)(totalKbytes * 1024) / uptime);

	if(totalAccesses > 0) {
		bytesPerReq = ((totalKbytes * 1024) / totalAccesses);
	}

	tmf_getCurrentThreadCount(&server->tmf, &currentThreadCount);

	tmf_getActiveThreadCount(&server->tmf, &busyWorkers);

	idleWorkers = (currentThreadCount - busyWorkers);

	scoreboard = (char *)malloc(sizeof(char) *
			(server->threads.maxThreadPoolLength + 1));

	for(ii = 0; ii < server->threads.maxThreadPoolLength; ii++) {
		switch(server->threads.list[ii].status) {
			case THREAD_STATUS_INIT:
				scoreboard[ii] = '.';
				break;

			case THREAD_STATUS_BUSY:
				scoreboard[ii] = 'W';
				break;

			case THREAD_STATUS_NAPPING:
				scoreboard[ii] = '_';
				break;

			case THREAD_STATUS_ERROR:
			default:
				scoreboard[ii] = '.';
				break;

		}
	}

	// build result

	result = (HttpTransactionManagerSend *)malloc(
			sizeof(HttpTransactionManagerSend));

	result->payloadLength = (strlen(TEMPLATE) + strlen(scoreboard) + 512);
	result->payload = (char *)malloc(sizeof(char) *
			(result->payloadLength + 1));

	result->payloadLength = snprintf(result->payload, result->payloadLength,
			TEMPLATE,
			ASGARD_VERSION,
			ASGARD_DATE,
			(int)totalAccesses,
			(int)totalKbytes,
			(int)uptime,
			reqPerSec,
			bytesPerSec,
			(int)bytesPerReq,
			busyWorkers,
			idleWorkers,
			scoreboard
		);

	*memorySizeOfResponse = result->payloadLength;

	// cleanup

	free(scoreboard);

	return result;
}

static void *searchd_handleGetServerStatus(void *context, void *request,
		int *memorySizeOfResponse)
{
	Json *result = NULL;
	Json *object = NULL;

	Server *server = NULL;
	ServerStatus *entry = NULL;
	ArrayList *serverStatus = NULL;

	HttpTransactionManagerReceive *receive = NULL;
	HttpTransactionManagerSend *response = NULL;

	if((context == NULL) || (request == NULL) ||
			(memorySizeOfResponse == NULL)) {
		DISPLAY_INVALID_ARGS;

		if(memorySizeOfResponse != NULL) {
			*memorySizeOfResponse = 0;
		}

		return NULL;
	}

	*memorySizeOfResponse = 0;

	server = (Server *)context;
	receive = (HttpTransactionManagerReceive *)request;

	if(SEARCHD_DEVELOPER_MODE) {
		common_display(stdout, receive->payload, receive->payloadLength);
		fprintf(stdout, "\n\n--------\n%s\n--------\n", receive->payload);
	}

	// convert current server status to a JSON object

	result = json_new();

	if((serverStatus = server_getStatus(server)) == NULL) {
		log_logf(server->log, LOG_LEVEL_ERROR, "{SEARCHD} "
				"failed to obtain status from server context");
		return NULL;
	}

	while((entry = arraylist_pop(serverStatus)) != NULL) {
		object = json_new();

		json_addString(object, "name", entry->name);
		json_addString(object, "value", entry->value);
		json_addString(object, "description", entry->description);

		json_addObject(result, entry->key, object);

		free(entry->key);
		free(entry->name);
		free(entry->value);
		free(entry->description);
		free(entry);
	}

	arraylist_free(serverStatus);
	free(serverStatus);

	// build response

	response = (HttpTransactionManagerSend *)malloc(
			sizeof(HttpTransactionManagerSend));

	response->payload = json_toString(result, &response->payloadLength);

	if(!SEARCHD_DEVELOPER_MODE) {
		response->payloadLength = json_minimizeJsonString(response->payload,
				response->payloadLength, atrue);
	}

	// cleanup

	json_freePtr(result);

	*memorySizeOfResponse = response->payloadLength;

	return response;
}

static void *searchd_handleJsonMessage(void *context, void *request,
		int *memorySizeOfResponse)
{
	aboolean hasError = afalse;
	int resultStringLength = 0;
	double timer = 0.0;
	char *ptr = NULL;
	char *errorMessage = NULL;
	char *messageName = NULL;
	char *resultString = NULL;

	Json *requestObject = NULL;
	Json *responseObject = NULL;

	HttpTransactionManagerReceive *receive = NULL;
	HttpTransactionManagerSend *result = NULL;

	Log *jsonMessageLog = NULL;
	SearchEngine *engine = NULL;
	SearchCompiler compiler;
	SearchdHandlerArgs *args = NULL;

	if((context == NULL) || (request == NULL) ||
			(memorySizeOfResponse == NULL)) {
		DISPLAY_INVALID_ARGS;

		if(memorySizeOfResponse != NULL) {
			*memorySizeOfResponse = 0;
		}

		return NULL;
	}

	*memorySizeOfResponse = 0;

	args = (SearchdHandlerArgs *)context;

	jsonMessageLog = args->jsonMessageLog;
	engine = args->engine;

	receive = (HttpTransactionManagerReceive *)request;

/*	if(SEARCHD_DEVELOPER_MODE) {
		common_display(stdout, receive->payload, receive->payloadLength);
		fprintf(stdout, "\n\n--------\n%s\n--------\n", receive->payload);
	}*/

	if((ptr = strncasestr(receive->payload, receive->payloadLength,
					"SearchEngineDaemonMessage",
					25)) != NULL) {
		if((requestObject = json_newFromString(ptr + 26)) != NULL) {
			if((messageName = json_getString(requestObject,
							"messageName")) == NULL) {
				messageName = "unknown";
				errorMessage = "malformed-request-object";
				hasError = atrue;
			}
		}
		else {
			messageName = "unknown";
			errorMessage = "invalid-request-object";
			hasError = atrue;
		}
	}
	else {
		messageName = "unknown";
		errorMessage = "missing-request-object";
		hasError = atrue;
	}

	if(!hasError) {
		resultString = json_toString(requestObject, &resultStringLength);

		resultStringLength = json_minimizeJsonString(resultString,
				resultStringLength, afalse);

		if(SEARCHD_DEVELOPER_MODE) {
			fprintf(stdout, "{JSON} length %i, '%s'\n", resultStringLength,
					resultString);
		}

		log_logf(jsonMessageLog, LOG_LEVEL_INFO, "%s", resultString);

		free(resultString);

		timer = time_getTimeMus();

		if(searchCompiler_compileJson(&compiler,
					engine->log,
					receive->ipAddress,
					engine->settings.validationType,
					requestObject) == 0) {
			searchCompiler_execute(&compiler, engine);
		}

		responseObject = searchCompiler_buildResponse(&compiler);

		searchCompiler_free(&compiler);

		json_addNumber(responseObject, "execTimeSeconds",
				time_getElapsedMusInSeconds(timer));
	}

	if(responseObject == NULL) {
		responseObject = json_new();

		json_addString(responseObject, "messageName", messageName);
		json_addString(responseObject, "resultCode", "-1");

		resultString = buildErrorResponse(errorMessage);

		json_addString(responseObject, "errorMessage", resultString);
	}

	resultString = json_toString(responseObject, &resultStringLength);

	if(!SEARCHD_DEVELOPER_MODE) {
		resultStringLength = json_minimizeJsonString(resultString,
				resultStringLength, atrue);
	}

	result = (HttpTransactionManagerSend *)malloc(
			sizeof(HttpTransactionManagerSend));

	result->payloadLength = (resultStringLength + 256);
	result->payload = (char *)malloc(sizeof(char) *
			(result->payloadLength + 1));

	result->payloadLength = snprintf(result->payload,
			result->payloadLength,
			"HTTP/1.1 200 OK\x0d\x0a"
			"Server: Search Daemon (Asgard Ver %s on %s)\x0d\x0a"
			"Content-Length: %i\x0d\x0a"
			"\x0d\x0a"
			"%s",
			ASGARD_VERSION,
			ASGARD_DATE,
			resultStringLength,
			resultString
		);

	// cleanup

	if(resultString != NULL) {
		free(resultString);
	}

	if(requestObject != NULL) {
		json_freePtr(requestObject);
	}

	if(responseObject != NULL) {
		json_freePtr(responseObject);
	}

	// setup results

	*memorySizeOfResponse = result->payloadLength;

	return result;
}

static void *searchd_handleSqlMessage(void *context, void *request,
		int *memorySizeOfResponse)
{
	aboolean hasError = afalse;
	int resultStringLength = 0;
	double timer = 0.0;
	char *query = NULL;
	char *errorMessage = NULL;
	char *messageName = NULL;
	char *resultString = NULL;

	Json *responseObject = NULL;

	HttpTransactionManagerReceive *receive = NULL;
	HttpTransactionManagerSend *result = NULL;

	Log *sqlMessageLog = NULL;
	SearchEngine *engine = NULL;
	SearchCompiler compiler;
	SearchdHandlerArgs *args = NULL;

	if((context == NULL) || (request == NULL) ||
			(memorySizeOfResponse == NULL)) {
		DISPLAY_INVALID_ARGS;

		if(memorySizeOfResponse != NULL) {
			*memorySizeOfResponse = 0;
		}

		return NULL;
	}

	*memorySizeOfResponse = 0;

	args = (SearchdHandlerArgs *)context;

	sqlMessageLog = args->sqlMessageLog;
	engine = args->engine;

	receive = (HttpTransactionManagerReceive *)request;

	if(SEARCHD_DEVELOPER_MODE) {
		common_display(stdout, receive->payload, receive->payloadLength);
		fprintf(stdout, "\n\n--------\n%s\n--------\n", receive->payload);
	}

	timer = time_getTimeMus();

	if((query = getUrlFromMessage(receive->payload,
					receive->payloadLength)) == NULL) {
		hasError = atrue;
		errorMessage = "failed to obtain URL from request";
	}
	else {
		log_logf(sqlMessageLog, LOG_LEVEL_INFO, "%s", query);

		if(searchCompiler_compileSql(&compiler, engine->log, query) == 0) {
			searchCompiler_execute(&compiler, engine);
		}

		responseObject = searchCompiler_buildResponse(&compiler);

		searchCompiler_free(&compiler);
	}

	json_addNumber(responseObject, "execTimeSeconds",
			time_getElapsedMusInSeconds(timer));

	if(hasError) {
		responseObject = json_new();

		json_addString(responseObject, "messageName", messageName);
		json_addString(responseObject, "resultCode", "-1");

		resultString = buildErrorResponse(errorMessage);

		json_addString(responseObject, "errorMessage", resultString);

		free(resultString);
	}

	json_addNumber(responseObject, "execTimeSeconds",
			time_getElapsedMusInSeconds(timer));

	resultString = json_toString(responseObject, &resultStringLength);

	if(!SEARCHD_DEVELOPER_MODE) {
		resultStringLength = json_minimizeJsonString(resultString,
				resultStringLength, atrue);
	}

	result = (HttpTransactionManagerSend *)malloc(
			sizeof(HttpTransactionManagerSend));

	result->payloadLength = (resultStringLength + 256);
	result->payload = (char *)malloc(sizeof(char) *
			(result->payloadLength + 1));

	result->payloadLength = snprintf(result->payload,
			result->payloadLength,
			"HTTP/1.1 200 OK\x0d\x0a"
			"Server: Search Daemon (Asgard Ver %s on %s)\x0d\x0a"
			"\x0d\x0a"
			"%s",
			ASGARD_VERSION,
			ASGARD_DATE,
			resultString
		);

	// cleanup

	if(resultString != NULL) {
		free(resultString);
	}

	if(responseObject != NULL) {
		json_freePtr(responseObject);
	}

	// setup results

	*memorySizeOfResponse = result->payloadLength;

	return result;
}

// server functions

static void searchd_signalShutdown(int signalType)
{
	fprintf(stdout, "SIGNAL(%i): detected, shutting down server.\n",
			signalType);
	serverIsRunning = afalse;
}

static int searchd_loadConfiguration(SearchdConfig *searchdConfig,
		char *filename)
{
	char *sValue = NULL;

	Config config;

	// load configuration file

	if(config_init(&config, filename) < 0) {
		fprintf(stderr, "error - failed to locate configuration file '%s'.",
				filename);
		return -1;
	}

	// initialize configuration

	memset(searchdConfig, 0, ((int)sizeof(SearchdConfig)));

	// logging properties

	config_getString(&config, "log", "logOutput", "stdout", &sValue);

	if(!strcasecmp(sValue, "null")) {
		searchdConfig->logOutput = LOG_OUTPUT_NULL;
	}
	else if(!strcasecmp(sValue, "stdout")) {
		searchdConfig->logOutput = LOG_OUTPUT_STDOUT;
	}
	else if(!strcasecmp(sValue, "stderr")) {
		searchdConfig->logOutput = LOG_OUTPUT_STDERR;
	}
	else if(!strcasecmp(sValue, "file")) {
		searchdConfig->logOutput = LOG_OUTPUT_FILE;
	}
	else {
		searchdConfig->logOutput = LOG_OUTPUT_STDOUT;
	}

	config_getString(&config, "log", "logFilename", "log/searchd.log",
			&sValue);

	searchdConfig->logFilename = strdup(sValue);

	if((searchdConfig->logOutput == LOG_OUTPUT_FILE) &&
			(searchdConfig->logFilename == NULL)) {
		searchdConfig->logOutput = LOG_OUTPUT_STDOUT;
	}

	config_getString(&config, "log", "logLevel", "debug", &sValue);

	if(!strcasecmp(sValue, "debug")) {
		searchdConfig->logLevel = LOG_LEVEL_DEBUG;
	}
	else if(!strcasecmp(sValue, "info")) {
		searchdConfig->logLevel = LOG_LEVEL_INFO;
	}
	else if(!strcasecmp(sValue, "fine")) {
		searchdConfig->logLevel = LOG_LEVEL_FINE;
	}
	else if(!strcasecmp(sValue, "warning")) {
		searchdConfig->logLevel = LOG_LEVEL_WARNING;
	}
	else if(!strcasecmp(sValue, "error")) {
		searchdConfig->logLevel = LOG_LEVEL_ERROR;
	}
	else if(!strcasecmp(sValue, "severe")) {
		searchdConfig->logLevel = LOG_LEVEL_SEVERE;
	}
	else if(!strcasecmp(sValue, "panic")) {
		searchdConfig->logLevel = LOG_LEVEL_PANIC;
	}
	else {
		searchdConfig->logLevel = LOG_LEVEL_DEBUG;
	}

	// http protocol

	config_getBoolean(&config, "http.protocol", "isTempFileMode", afalse,
			&searchdConfig->httpIsTempFileMode);

	config_getBigInteger(&config, "http.protocol", "tempFileThresholdBytes",
			16384, &searchdConfig->httpTempFileThresholdBytes);

	config_getBigInteger(&config, "http.protocol", "tempFileSpaceAllowedBytes",
			16777216, &searchdConfig->httpTempFileSpaceAllowedBytes);

	config_getBigInteger(&config, "http.protocol", "tempFileSpaceInUseBytes",
			0, &searchdConfig->httpTempFileSpaceInUseBytes);

	config_getString(&config, "http.protocol", "tempFileBasePath",
			"/tmp", &sValue);

	searchdConfig->httpTempFileBasePath = strdup(sValue);

	// server threading properties

	config_getBoolean(&config, "server.threading", "isThreadPoolShrinkEnabled",
			atrue, &searchdConfig->threadIsThreadPoolShrinkEnabled);

	config_getInteger(&config, "server.threading", "minThreadPoolLength",
			4, &searchdConfig->threadMinThreadPoolLength);

	config_getInteger(&config, "server.threading", "maxThreadPoolLength",
			8, &searchdConfig->threadMaxThreadPoolLength);

	// server packets properties

	config_getBoolean(&config, "server.packets", "isDumpPacketsEnabled",
			afalse, &searchdConfig->packetsIsDumpPacketsEnabled);

	config_getBoolean(&config, "server.packets", "useIndividualFiles",
			afalse, &searchdConfig->packetsUseIndividualFiles);

	config_getBoolean(&config, "server.packets", "usePacketHeaders",
			atrue, &searchdConfig->packetsUsePacketHeaders);

	config_getString(&config, "server.packets", "path",
			"log/packets", &sValue);

	searchdConfig->packetsPath = strdup(sValue);

	config_getString(&config, "server.packets", "filename",
			"searchd.packets", &sValue);

	searchdConfig->packetsFilename = strdup(sValue);

	// server socket properties

	config_getString(&config, "server.socket", "hostname",
			"localhost", &sValue);

	searchdConfig->socketHostname = strdup(sValue);

	config_getInteger(&config, "server.socket", "port",
			8080, &searchdConfig->socketPort);

	// search engine properties

	config_getBigInteger(&config, "search.engine",
			"maxContainerCount",
			SEARCH_ENGINE_DEFAULT_MAX_CONTAINERS,
			&searchdConfig->searchMaxContainerCount);

	config_getBigInteger(&config, "search.engine",
			"maxContainerMemoryLength",
			SEARCH_ENGINE_DEFAULT_MAX_CONTAINER_MEMORY,
			&searchdConfig->searchMaxContainerMemoryLength);

	config_getDouble(&config, "search.engine",
			"containerTimeout",
			SEARCH_ENGINE_DEFAULT_CONTAINER_TIMEOUT,
			&searchdConfig->searchContainerTimeout);

	config_getInteger(&config, "search.engine", "minStringLength",
			SEARCH_ENGINE_DEFAULT_MIN_STRING_LENGTH,
			&searchdConfig->searchMinStringLength);

	config_getInteger(&config, "search.engine", "maxStringLength",
			SEARCH_ENGINE_DEFAULT_MAX_STRING_LENGTH,
			&searchdConfig->searchMaxStringLength);

	config_getBigInteger(&config, "search.engine",
			"maxSortOperationMemoryLength",
			SEARCH_ENGINE_DEFAULT_MAX_SORT_OPERATION_LENGTH,
			&searchdConfig->maxSortOperationMemoryLength);

	config_getString(&config, "search.engine",
			"stringDelimiters",
			SEARCH_ENGINE_DEFAULT_DELIMITERS,
			&sValue);

	searchdConfig->searchStringDelimiters = strdup(sValue);

	config_getDouble(&config, "search.engine.backup",
			"stateWriteThresholdSeconds",
			SEARCH_ENGINE_DEFAULT_STATE_BACKUP_SECONDS,
			&searchdConfig->searchStateWriteThresholdSeconds);

	config_getString(&config, "search.engine.backup",
			"statePath",
			SEARCH_ENGINE_DEFAULT_STATE_BACKUP_PATH,
			&sValue);

	searchdConfig->searchStatePath = strdup(sValue);

	config_getDouble(&config, "search.engine.backup",
			"containerWriteThresholdSeconds",
			SEARCH_ENGINE_DEFAULT_DATA_BACKUP_SECONDS,
			&searchdConfig->searchContainerWriteThresholdSeconds);

	config_getString(&config, "search.engine.backup",
			"containerPath",
			SEARCH_ENGINE_DEFAULT_DATA_BACKUP_PATH,
			&sValue);

	searchdConfig->searchContainerPath = strdup(sValue);

	config_getString(&config, "search.engine.exclusions",
			"excludedWordConfigFilename",
			SEARCH_ENGINE_DEFAULT_EXCLUDED_WORDS_CONFIG,
			&sValue);

	searchdConfig->searchExcludedWordConfigFilename = strdup(sValue);

	config_getString(&config, "search.engine.authentication",
			"authenticationConfigFilename",
			SEARCH_ENGINE_DEFAULT_AUTHENTICATION_CONFIG,
			&sValue);

	searchdConfig->authenticationConfigFilename = strdup(sValue);

	config_getString(&config, "search.engine.lang.json",
			"typeCheckLevel",
			SEARCH_ENGINE_DEFAULT_VALIDATION_TYPE_STRING,
			&sValue);

	searchdConfig->validationType = searchJson_stringToValidationType(sValue);

	// cleanup

	config_free(&config);

	return 0;
}

static int searchd_execute(SearchdConfig *config, SearchdOptions *options)
{
	Log log;
	Log jsonMessageLog;
	Log sqlMessageLog;
	HttpProtocol protocol;
	TransactionManager manager;
	Server server;
	SearchEngine engine;
	SearchdHandlerArgs args;

	ArrayList *serverStatus = NULL;
	ServerStatus *entry = NULL;

	fprintf(stdout, "Initializing search engine daemon...");
	fflush(stdout);

	// configure logging

	log_init(&log,
			config->logOutput,
			config->logFilename,
			config->logLevel);

	if(SEARCHD_DEVELOPER_MODE) {
		log_enablePeriodicFlush(&log, 128);
	}

	log_init(&jsonMessageLog,
			LOG_OUTPUT_FILE,
			"log/searchd.json.message.log",
			LOG_LEVEL_DEBUG);

	if(SEARCHD_DEVELOPER_MODE) {
		log_enablePeriodicFlush(&jsonMessageLog, 128);
	}

	log_init(&sqlMessageLog,
			LOG_OUTPUT_FILE,
			"log/searchd.sql.message.log",
			LOG_LEVEL_DEBUG);

	if(SEARCHD_DEVELOPER_MODE) {
		log_enablePeriodicFlush(&sqlMessageLog, 128);
	}

	// configure http protocol

	http_protocol_init(&protocol,
			config->httpIsTempFileMode,
			config->httpTempFileThresholdBytes,
			config->httpTempFileSpaceAllowedBytes,
			config->httpTempFileSpaceInUseBytes,
			config->httpTempFileBasePath
		);

	// configure transaction manager

	args.jsonMessageLog = &jsonMessageLog;
	args.sqlMessageLog = &sqlMessageLog;
	args.engine = &engine;

	transactionManager_init(&manager, &log);

	transactionManager_setReceiveFunction(&manager,
			&protocol,
			http_protocol_transactionManagerReceive);

	transactionManager_setRecvFreeFunction(&manager,
			http_protocol_transactionManagerReceiveFree);

	transactionManager_setSendFunction(&manager,
			&protocol,
			http_protocol_transactionManagerSend);

	transactionManager_setSendFreeFunction(&manager,
			http_protocol_transactionManagerSendFree);

	transactionManager_setIdFunction(&manager,
			&protocol,
			http_protocol_transactionManagerIdFunction);

	transactionManager_addHandler(&manager,
			afalse,				// is socket left open
			"default",
			"searchd_handleDefault",
			searchd_handleDefault,
			&protocol);

	transactionManager_addHandler(&manager,
			afalse,				// is socket left open
			"/",
			"searchd_handleDefault",
			searchd_handleDefault,
			&protocol);

	transactionManager_addHandler(&manager,
			afalse,				// is socket left open
			"/server-status",
			"searchd_handleGetApacheServerStatus",
			searchd_handleGetApacheServerStatus,
			&server);

	transactionManager_addHandler(&manager,
			afalse,				// is socket left open
			"/asgard-server-status",
			"searchd_handleGetServerStatus",
			searchd_handleGetServerStatus,
			&server);

	transactionManager_addHandler(&manager,
			afalse,				// is socket left open
			"/json-api",
			"searchd_handleJsonMessage",
			searchd_handleJsonMessage,
			&args);

	transactionManager_addHandler(&manager,
			afalse,				// is socket left open
			"/sql-api",
			"searchd_handleSqlMessage",
			searchd_handleSqlMessage,
			&args);

	// configure server

	server_init(&server, &log);

	server_setThreadPoolOptions(&server,
			config->threadIsThreadPoolShrinkEnabled,
			config->threadMinThreadPoolLength,
			config->threadMaxThreadPoolLength);

	if(config->packetsIsDumpPacketsEnabled) {
		server_setDumpPacketsOptions(&server,
				config->packetsUseIndividualFiles,
				config->packetsUsePacketHeaders,
				config->packetsPath,
				config->packetsFilename
			);

		server_setDumpPacketsMode(&server, atrue);
	}
	else {
		server_setDumpPacketsMode(&server, afalse);
	}

	server_setTransactionManager(&server, &manager);

	server_setSocketOptions(&server,
			config->socketHostname,
			config->socketPort,
			SOCKET_PROTOCOL_TCPIP);

	// configure search engine

	searchEngine_init(&engine, &log);

	searchEngine_setMaxContainerCount(&engine,
			config->searchMaxContainerCount);

	searchEngine_setMaxContainerMemoryLength(&engine,
			config->searchMaxContainerMemoryLength);

	searchEngine_setContainerTimeout(&engine,
			config->searchContainerTimeout);

	log_logf(&log, LOG_LEVEL_INFO,
			"{SEARCHD} container settings "
			"{ %lli max, %lli max memory, %0.6f timeout }",
			config->searchMaxContainerCount,
			config->searchMaxContainerMemoryLength,
			config->searchContainerTimeout);

	searchEngine_setStringIndexThresholds(&engine,
			config->searchMinStringLength,
			config->searchMaxStringLength);

	searchEngine_setMaxSortOperationMemoryLength(&engine,
			config->maxSortOperationMemoryLength);

	searchEngine_setStringDelimiters(&engine,
			config->searchStringDelimiters,
			strlen(config->searchStringDelimiters));

	log_logf(&log, LOG_LEVEL_INFO,
			"{SEARCHD} index settings "
			"{ %i min string length, %i max string length, '%s' delimiters }",
			config->searchMinStringLength,
			config->searchMaxStringLength,
			config->searchStringDelimiters);

	searchEngine_setStateBackupAttributes(&engine,
			config->searchStateWriteThresholdSeconds,
			config->searchStatePath);

	log_logf(&log, LOG_LEVEL_INFO,
			"{SEARCHD} state backup settings "
			"{ %0.6f write threshold seconds, '%s' write path }",
			config->searchStateWriteThresholdSeconds,
			config->searchStatePath);

	searchEngine_setIndexBackupAttributes(&engine,
			config->searchContainerWriteThresholdSeconds,
			config->searchContainerPath);

	log_logf(&log, LOG_LEVEL_INFO,
			"{SEARCHD} container backup settings "
			"{ %0.6f write threshold seconds, '%s' write path }",
			config->searchContainerWriteThresholdSeconds,
			config->searchContainerPath);

	searchEngine_loadExcludedWords(&engine,
			config->searchExcludedWordConfigFilename);

	searchEngine_loadAuthentication(&engine,
			config->authenticationConfigFilename);

	searchEngine_setJsonLangValidationType(&engine, config->validationType);

	log_logf(&log, LOG_LEVEL_INFO,
			"{SEARCHD} using JSON language validation level '%s'",
			searchJson_validationTypeToString(config->validationType));

	if(options->stateFilename != NULL) {
		searchEngine_restoreState(&engine, options->stateFilename);
	}

	if(options->dataFilename != NULL) {
		searchEngine_restoreData(&engine, options->dataFilename);
	}

	fprintf(stdout, "OK.\n");

	// start server & monitor server activity

	fprintf(stdout, "Starting server on '%s:%i'...", config->socketHostname,
			config->socketPort);
	fflush(stdout);

	if(server_start(&server) < 0) {
		fprintf(stdout, "FAILED. (Server refused to start)\n");
		return -1;
	}

	serverIsRunning = atrue;

	fprintf(stdout, "OK.\n");

	// wait for server shutdown

	do {
		time_usleep(8192);
	} while((serverIsRunning) && (server.state == SERVER_STATE_RUNNING));

	log_logf(&log, LOG_LEVEL_INFO, "detected server shutdown");

	// display server status prior to shutdown

	fprintf(stdout, "Reporting server status...");
	fflush(stdout);

	if((serverStatus = server_getStatus(&server)) == NULL) {
		fprintf(stdout, "FAILED. (Failed to obtain server status)\n");
		return -1;
	}

	fprintf(stdout, "OK.\n");

	while((entry = arraylist_pop(serverStatus)) != NULL) {
		log_logf(&log, LOG_LEVEL_INFO, "{SERVER} '%s' = '%s'",
				entry->name, entry->value);

		free(entry->key);
		free(entry->name);
		free(entry->value);
		free(entry->description);
		free(entry);
	}

	arraylist_free(serverStatus);
	free(serverStatus);

	// stop server

	fprintf(stdout, "Shutting down server...");
	fflush(stdout);

	if(server_stop(&server) < 0) {
		fprintf(stdout, "FAILED. (Failed to stop server)\n");
		return -1;
	}

	fprintf(stdout, "OK.\n");

	// backup search engine state & indexes

	fprintf(stdout, "Backing up search engine to disk...");
	fflush(stdout);

	if(searchEngine_saveAuthentication(&engine) < 0) {
		fprintf(stdout, "FAILED. (Failed to backup authentication).\n");
	}
	else if(searchEngine_backupState(&engine) < 0) {
		fprintf(stdout, "FAILED. (Failed to backup state).\n");
	}
	else if(searchEngine_backupData(&engine) < 0) {
		fprintf(stdout, "FAILED. (Failed to backup data).\n");
	}
	else {
		fprintf(stdout, "OK.\n");
	}

	// flush logs

	fprintf(stdout, "Flushing logs...");
	fflush(stdout);

	log_flush(&log);
	log_flush(&jsonMessageLog);
	log_flush(&sqlMessageLog);

	fprintf(stdout, "OK.\n");

	// free server context

	fprintf(stdout, "Free server context...");
	fflush(stdout);

	if(server_free(&server) < 0) {
		fprintf(stdout, "FAILED.\n");
	}
	else {
		fprintf(stdout, "OK.\n");
	}

	// free transaction manager context

	fprintf(stdout, "Free transaction-manager context...");
	fflush(stdout);

	if(transactionManager_free(&manager) < 0) {
		fprintf(stdout, "FAILED.\n");
	}
	else {
		fprintf(stdout, "OK.\n");
	}

	// free search engine context

	fprintf(stdout, "Free search engine context...");
	fflush(stdout);

	if(searchEngine_free(&engine) < 0) {
		fprintf(stdout, "FAILED.\n");
	}
	else {
		fprintf(stdout, "OK.\n");
	}

	// cleanup

	http_protocol_free(&protocol);

	log_free(&log);
	log_free(&jsonMessageLog);
	log_free(&sqlMessageLog);

	if(config->logFilename != NULL) {
		free(config->logFilename);
	}

	if(config->httpTempFileBasePath != NULL) {
		free(config->httpTempFileBasePath);
	}

	if(config->packetsPath != NULL) {
		free(config->packetsPath);
	}

	if(config->packetsFilename != NULL) {
		free(config->packetsFilename);
	}

	if(config->socketHostname != NULL) {
		free(config->socketHostname);
	}

	if(config->searchStringDelimiters != NULL) {
		free(config->searchStringDelimiters);
	}

	if(config->searchStatePath != NULL) {
		free(config->searchStatePath);
	}

	if(config->searchContainerPath != NULL) {
		free(config->searchContainerPath);
	}

	if(config->searchExcludedWordConfigFilename != NULL) {
		free(config->searchExcludedWordConfigFilename);
	}

	if(config->authenticationConfigFilename != NULL) {
		free(config->authenticationConfigFilename);
	}

	return 0;
}

