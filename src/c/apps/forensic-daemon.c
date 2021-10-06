/*
 * forensic-daemon.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard forensic log analysis application for Apache/httpd.
 *
 * Written by Josh English.
 */

// preprocessor directives

#define _CORE_COMMON_BYPASS_STD_STREAM
#include "core/core.h"


// define forensic-daemon constants

#define FORENSIC_DAEMON_DEBUG_MODE				atrue

#define FORENSIC_DAEMON_VERBOSE_MODE			atrue

#define FORENSIC_DAEMON_LOG_RAW_FORENSIC		atrue

#define FORENSIC_DAEMON_CONFIG_FILENAME			"conf/forensicDaemon.config"

#define FORENSIC_DAEMON_LOG_LEVEL				LOG_LEVEL_DEBUG

#define FORENSIC_DAEMON_LOG_OUTPUT				LOG_OUTPUT_FILE

#define FORENSIC_DAEMON_LOG_FILENAME			"log/forensic-daemon.log"

#define FORENSIC_DAEMON_RAW_LOG_FILENAME		"log/forensic.raw.log"

#define FORENSIC_DAEMON_ACCESS_LOG_FILENAME		"log/access.log"

#define FORENSIC_DAEMON_DESTINATION_PORT		80

#define FORENSIC_DAEMON_DESTINATION_IP_ADDRESS	"localhost"

#define FORENSIC_DAEMON_DESTINATION_URL			"/actions/postLogForensic"


// define forensic-daemon data types

typedef struct _ForensicDaemonConfig {
	aboolean debugMode;
	aboolean verboseMode;
	aboolean logRawForensic;
	aboolean readAccessLog;
	int port;
	int logLevel;
	int logOutput;
	char *url;
	char *ipAddress;
	char *logFilename;
	char *rawForensicLog;
	char *accessForensicLog;
} ForensicDaemonConfig;

typedef struct _ForensicLogEntryHeader {
	int nameLength;
	int valueLength;
	char *name;
	char *value;
} ForensicLogEntryHeader;

typedef struct _ForensicLogEntry {
	int assemblyCounter;
	int uidLength;
	int requestTypeLength;
	int requestUrlLength;
	int requestProtocolLength;
	int requestVersionLength;
	int headerLength;
	int cookieLength;
	char *uid;
	char *requestType;
	char *requestUrl;
	char *requestProtocol;
	char *requestVersion;
	char **cookies;
	ForensicLogEntryHeader **headers;
} ForensicLogEntry;

typedef struct _AccessLogEntry {
	int uidLength;
	int remoteHostLength;
	int remoteLognameLength;
	int remoteUserLength;
	int timestampLength;
	int requestTypeLength;
	int requestUrlLength;
	int requestProtocolLength;
	int requestVersionLength;
	int statusLength;
	int responseLengthLength;
	int refererLength;
	int userAgentLength;
	double creationTime;
	char *uid;
	char *remoteHost;
	char *remoteLogname;
	char *remoteUser;
	char *timestamp;
	char *requestType;
	char *requestUrl;
	char *requestProtocol;
	char *requestVersion;
	char *status;
	char *responseLength;
	char *referer;
	char *userAgent;
} AccessLogEntry;

typedef struct _ForensicDaemon {
	ForensicDaemonConfig config;
	FileHandle fh;
	FifoStack lineQueue;
	HttpProtocol httpProtocol;
	Bptree index;
	TmfContext tmf;
	Log log;
	Mutex mutex;
} ForensicDaemon;


// define forensic-daemon global variables

static ForensicDaemon forensicDaemon;


// delcare forensic-daemon functions

static int getThreadId();

static aboolean fileExists(char *filename);

static void loadConfigFile(char *filename);

static alint getQueueItemCount(ForensicDaemon *forensicDaemon);

static void pushLineOntoQueue(ForensicDaemon *forensicDaemon, char *line,
		int lineLength);

static char *getLineFromQueue(ForensicDaemon *forensicDaemon, int *lineLength);

static char *copyToken(char *string, int *resultLength);

static char **string_tokenize(char *source, int sourceLength,
		char *delimiterList, int minimumTokenLength, aboolean recognizeQuotes,
		int *tokenCount);

static char *parseUid(char *token, int tokenLength, int *uidLength);

static aboolean parseRequest(ForensicLogEntry *logEntry, char *token,
		int tokenLength);

static ForensicLogEntryHeader *parseHeader(char *token, int tokenLength);

static ForensicLogEntry *buildLogEntry(ForensicDaemon *forensicDaemon,
		char *line, int lineLength);

static char *logEntryToString(ForensicLogEntry *logEntry,
		AccessLogEntry *accessLogEntry, int *stringLength);

static void freeLogEntry(void *memory);

static AccessLogEntry *buildAccessLogEntry(ForensicDaemon *forensicDaemon,
		char *line, int lineLength);

static void freeAccessLogEntry(void *memory);

static char *buildHtmlRequest(ForensicDaemon *forensicDaemon, char *message,
		int messageLength, int *requestLength);

static int executeHttpTransaction(ForensicDaemon *forensicDaemon, char *message,
		int messageLength);

static void *workerThread(void *threadContext, void *argument);

static void readStdin();


// main function

int main(int argc, char *argv[])
{
	int ii = 0;
	int workerThreadId = 0;
	char *filename = NULL;

	signal_registerDefault();

	memset(&forensicDaemon, 0, (int)(sizeof(ForensicDaemon)));

	fifostack_init(&forensicDaemon.lineQueue);

	http_protocol_init(&forensicDaemon.httpProtocol,
			afalse,			// is temp file mode
			0,				// temp file threshold bytes
			0,				// temp file space allowed bytes
			0,				// temp file space in-use bytes
			NULL			// temp file base path
			);

	tmf_init(&forensicDaemon.tmf);

	mutex_init(&forensicDaemon.mutex);

	if((argc > 1) && (argv[1] != NULL)) {
		filename = argv[1];
	}

	loadConfigFile(filename);

	if(forensicDaemon.config.verboseMode) {
		printf("Asgard Forensic Daemon Application (%i) Ver %s on %s.\n",
				getThreadId(), ASGARD_VERSION, ASGARD_DATE);
	}

	log_logf(&forensicDaemon.log, LOG_LEVEL_INFO,
			"Asgard Forensic Daemon Started");

	// log command-line argument(s)

	if(forensicDaemon.config.debugMode) {
		if(argc > 1) {
			for(ii = 0; ii < argc; ii++) {
				log_logf(&forensicDaemon.log, LOG_LEVEL_DEBUG,
						"argv[%i] is '%s'", ii, argv[ii]);
			}
		}
	}

	// display configuration properties

	if(forensicDaemon.config.debugMode) {
		log_logf(&forensicDaemon.log, LOG_LEVEL_DEBUG,
				"{CONFIG} debugMode => %i",
				(int)forensicDaemon.config.debugMode);
		log_logf(&forensicDaemon.log, LOG_LEVEL_DEBUG,
				"{CONFIG} verboseMode => %i",
				(int)forensicDaemon.config.verboseMode);
		log_logf(&forensicDaemon.log, LOG_LEVEL_DEBUG,
				"{CONFIG} logRawForensic => %i",
				(int)forensicDaemon.config.logRawForensic);
		log_logf(&forensicDaemon.log, LOG_LEVEL_DEBUG,
				"{CONFIG} readAccessLog => %i",
				(int)forensicDaemon.config.readAccessLog);
		log_logf(&forensicDaemon.log, LOG_LEVEL_DEBUG,
				"{CONFIG} logLevel => %i",
				forensicDaemon.config.logLevel);
		log_logf(&forensicDaemon.log, LOG_LEVEL_DEBUG,
				"{CONFIG} logOutput => %i",
				forensicDaemon.config.logOutput);
		log_logf(&forensicDaemon.log, LOG_LEVEL_DEBUG,
				"{CONFIG} logFilename => '%s'",
				forensicDaemon.config.logFilename);
		log_logf(&forensicDaemon.log, LOG_LEVEL_DEBUG,
				"{CONFIG} rawForensicLog => '%s'",
				forensicDaemon.config.rawForensicLog);
		log_logf(&forensicDaemon.log, LOG_LEVEL_DEBUG,
				"{CONFIG} accessForensicLog => '%s'",
				forensicDaemon.config.accessForensicLog);
		log_logf(&forensicDaemon.log, LOG_LEVEL_DEBUG,
				"{CONFIG} ipAddress => '%s'",
				forensicDaemon.config.ipAddress);
		log_logf(&forensicDaemon.log, LOG_LEVEL_DEBUG,
				"{CONFIG} url => '%i'",
				forensicDaemon.config.port);
		log_logf(&forensicDaemon.log, LOG_LEVEL_DEBUG,
				"{CONFIG} url => '%s'",
				forensicDaemon.config.url);
	}

	if(tmf_spawnThread(&forensicDaemon.tmf,
				workerThread,
				(void *)&forensicDaemon,
				&workerThreadId) < 0) {
		log_logf(&forensicDaemon.log, LOG_LEVEL_PANIC,
				"failed to spawn worker thread, aborting daemon");
		return 1;
	}

	log_logf(&forensicDaemon.log, LOG_LEVEL_INFO,
			"started worker thread #%i", workerThreadId);

	// read stdin input from the Apache mod-forensic module

	readStdin();

	// shutdown daemon

	tmf_stopThread(&forensicDaemon.tmf, workerThreadId);

	log_logf(&forensicDaemon.log, LOG_LEVEL_INFO,
			"Asgard Forensic Daemon Shutdown");

	if(forensicDaemon.config.verboseMode) {
		printf("Asgard Forensic Daemon Application (%i) Shutdown.\n",
				getThreadId());
	}

	// cleanup

	mutex_lock(&forensicDaemon.mutex);

	if(forensicDaemon.config.readAccessLog) {
		file_free(&forensicDaemon.fh);
	}

	fifostack_free(&forensicDaemon.lineQueue);

	http_protocol_free(&forensicDaemon.httpProtocol);

	bptree_free(&forensicDaemon.index);

	tmf_free(&forensicDaemon.tmf);

	log_free(&forensicDaemon.log);

	if(forensicDaemon.config.logFilename != NULL) {
		free(forensicDaemon.config.logFilename);
	}

	if(forensicDaemon.config.rawForensicLog != NULL) {
		free(forensicDaemon.config.rawForensicLog);
	}

	if(forensicDaemon.config.accessForensicLog != NULL) {
		free(forensicDaemon.config.accessForensicLog);
	}

	mutex_unlock(&forensicDaemon.mutex);

	mutex_free(&forensicDaemon.mutex);

	return 0;
}


// define forensic-daemon functions

static int getThreadId()
{
	int threadId = 0;

	if(thread_self(&threadId) < 0) {
		threadId = -1;
	}

	return threadId;
}

static aboolean fileExists(char *filename)
{
	aboolean result = afalse;

	if(filename == NULL) {
		return afalse;
	}

	if(system_fileExists(filename, &result) < 0) {
		return afalse;
	}

	return result;
}

static void loadConfigFile(char *filename)
{
	aboolean aValue = afalse;
	int rc = 0;
	int iValue = 0;
	char *sValue = NULL;
	char *errorMessage = NULL;
	char *configFilename = NULL;

	Config config;

	// determine config filename

	if(filename != NULL) {
		if(fileExists(filename)) {
			configFilename = filename;
		}
	}

	if(configFilename == NULL) {
		if(fileExists(FORENSIC_DAEMON_CONFIG_FILENAME)) {
			configFilename = FORENSIC_DAEMON_CONFIG_FILENAME;
		}
	}

	forensicDaemon.config.debugMode = FORENSIC_DAEMON_DEBUG_MODE;

	forensicDaemon.config.verboseMode = FORENSIC_DAEMON_VERBOSE_MODE;

	forensicDaemon.config.logRawForensic = FORENSIC_DAEMON_LOG_RAW_FORENSIC;

	forensicDaemon.config.readAccessLog = atrue;

	forensicDaemon.config.logLevel = FORENSIC_DAEMON_LOG_LEVEL;

	forensicDaemon.config.logOutput = FORENSIC_DAEMON_LOG_OUTPUT;

	forensicDaemon.config.logFilename = strdup(FORENSIC_DAEMON_LOG_FILENAME);

	forensicDaemon.config.rawForensicLog = strdup(
			FORENSIC_DAEMON_RAW_LOG_FILENAME);

	forensicDaemon.config.accessForensicLog = strdup(
			FORENSIC_DAEMON_ACCESS_LOG_FILENAME);

	forensicDaemon.config.port = FORENSIC_DAEMON_DESTINATION_PORT;

	forensicDaemon.config.ipAddress = strdup(
			FORENSIC_DAEMON_DESTINATION_IP_ADDRESS);

	forensicDaemon.config.url = strdup(FORENSIC_DAEMON_DESTINATION_URL);

	if(configFilename == NULL) {
		return;
	}

	if(config_init(&config, configFilename) < 0) {
		return;
	}

	// get general properties

	if(config_getBoolean(&config, "general", "debugMode",
				FORENSIC_DAEMON_DEBUG_MODE, &aValue) == 0) {
		forensicDaemon.config.debugMode = aValue;
	}

	if(config_getBoolean(&config, "general", "verboseMode",
				FORENSIC_DAEMON_VERBOSE_MODE, &aValue) == 0) {
		forensicDaemon.config.verboseMode = aValue;
	}

	if(config_getBoolean(&config, "general", "logRawForensic",
				FORENSIC_DAEMON_LOG_RAW_FORENSIC, &aValue) == 0) {
		forensicDaemon.config.logRawForensic = aValue;
	}

	if(config_getString(&config, "general", "rawLogFilename",
				FORENSIC_DAEMON_RAW_LOG_FILENAME, &sValue) == 0) {
		free(forensicDaemon.config.rawForensicLog);
		forensicDaemon.config.rawForensicLog = strdup(sValue);
	}

	if(config_getString(&config, "general", "accessLogFilename",
				FORENSIC_DAEMON_ACCESS_LOG_FILENAME, &sValue) == 0) {
		free(forensicDaemon.config.accessForensicLog);
		forensicDaemon.config.accessForensicLog = strdup(sValue);
	}

	// internal logging properties

	if(config_getInteger(&config, "log", "level", FORENSIC_DAEMON_LOG_LEVEL,
				&iValue) == 0) {
		forensicDaemon.config.logLevel = iValue;
	}

	if(config_getInteger(&config, "log", "output", FORENSIC_DAEMON_LOG_OUTPUT,
				&iValue) == 0) {
		forensicDaemon.config.logOutput = iValue;
	}

	if(config_getString(&config, "log", "filename",
				FORENSIC_DAEMON_LOG_FILENAME, &sValue) == 0) {
		free(forensicDaemon.config.logFilename);
		forensicDaemon.config.logFilename = strdup(sValue);
	}

	// destination properties

	if(config_getInteger(&config, "destination", "port",
				FORENSIC_DAEMON_DESTINATION_PORT, &iValue) == 0) {
		forensicDaemon.config.port = iValue;
	}

	if(config_getString(&config, "destination", "ipAddress",
				FORENSIC_DAEMON_DESTINATION_IP_ADDRESS, &sValue) == 0) {
		free(forensicDaemon.config.ipAddress);
		forensicDaemon.config.ipAddress = strdup(sValue);
	}

	if(config_getString(&config, "destination", "url",
				FORENSIC_DAEMON_DESTINATION_URL, &sValue) == 0) {
		free(forensicDaemon.config.url);
		forensicDaemon.config.url = strdup(sValue);
	}

	// setup internal logging

	if(log_init(&forensicDaemon.log,
				forensicDaemon.config.logOutput,
				forensicDaemon.config.logFilename,
				forensicDaemon.config.logLevel) < 0) {
		log_init(&forensicDaemon.log,
				LOG_OUTPUT_STDOUT, NULL, LOG_LEVEL_DEBUG);
	}

	// setup access-file reading

	if((rc = file_init(&forensicDaemon.fh,
					forensicDaemon.config.accessForensicLog,
					"r",
					8192)) < 0) {
		if(file_getError(&forensicDaemon.fh, rc, &errorMessage) < 0) {
			errorMessage = "unknown file error";
		}

		log_logf(&forensicDaemon.log, LOG_LEVEL_ERROR,
				"failed to open forensic access logfile '%s' with '%s'",
				forensicDaemon.config.accessForensicLog, errorMessage);

		file_free(&forensicDaemon.fh);

		forensicDaemon.config.readAccessLog = afalse;
	}
	else if((rc = file_seekEof(&forensicDaemon.fh)) < 0) {
		if(file_getError(&forensicDaemon.fh, rc, &errorMessage) < 0) {
			errorMessage = "unknown file error";
		}

		log_logf(&forensicDaemon.log, LOG_LEVEL_ERROR,
				"failed to seek to EOF of forensic access logfile '%s' "
				"with '%s'",
				forensicDaemon.config.accessForensicLog, errorMessage);

		file_free(&forensicDaemon.fh);

		forensicDaemon.config.readAccessLog = afalse;
	}

	// initialize access-log index

	bptree_init(&forensicDaemon.index);
	bptree_setFreeFunction(&forensicDaemon.index, freeAccessLogEntry);

	// cleanup

	config_free(&config);
}

static alint getQueueItemCount(ForensicDaemon *forensicDaemon)
{
	alint result = 0;

	mutex_lock(&forensicDaemon->mutex);

	if(fifostack_getItemCount(&forensicDaemon->lineQueue, &result) < 0) {
		result = 0;
	}

	mutex_unlock(&forensicDaemon->mutex);

	return result;
}

static void pushLineOntoQueue(ForensicDaemon *forensicDaemon, char *line,
		int lineLength)
{
	char *localCopy = NULL;

	localCopy = strndup(line, lineLength);

	mutex_lock(&forensicDaemon->mutex);

	fifostack_push(&forensicDaemon->lineQueue, localCopy);

	mutex_unlock(&forensicDaemon->mutex);
}

static char *getLineFromQueue(ForensicDaemon *forensicDaemon, int *lineLength)
{
	char *result = NULL;

	*lineLength = 0;

	mutex_lock(&forensicDaemon->mutex);

	if(fifostack_pop(&forensicDaemon->lineQueue, ((void *)&result)) < 0) {
		mutex_unlock(&forensicDaemon->mutex);
		return NULL;
	}

	mutex_unlock(&forensicDaemon->mutex);

	if(result == NULL) {
		return NULL;
	}

	*lineLength = strlen(result);

	return result;
}

static char *copyToken(char *string, int *resultLength)
{
	int length = 0;
	char *result = NULL;

	length = strlen(string);

	result = strndup(string, length);

	*resultLength = length;

	return result;
}

static char **string_tokenize(char *source, int sourceLength,
		char *delimiterList, int minimumTokenLength, aboolean recognizeQuotes,
		int *tokenCount)
{
	aboolean found = afalse;
	aboolean insideQuotes = afalse;
	int ii = 0;
	int jj = 0;
	int kk = 0;
	int ref = 0;
	int resultRef = 0;
	int delimiterLength = 0;
	int tokenLength = 0;
	int tokenListLength = 0;
	char *token = NULL;
	char **result = NULL;

	if((source == NULL) ||
			(sourceLength < 0) ||
			(delimiterList == NULL) ||
			((delimiterLength = strlen(delimiterList)) < 0) ||
			(minimumTokenLength < 0) ||
			(tokenCount == NULL)) {
		DISPLAY_INVALID_ARGS;
		if(tokenCount != NULL) {
			*tokenCount = -1;
		}
		return NULL;
	}

	*tokenCount = 0;

	// determine the number of tokens in the source string

	tokenListLength = 0;

	for(ii = 0, ref = 0; ii < sourceLength; ii++) {
		if((recognizeQuotes) && (source[ii] == '"')) {
			if(insideQuotes) {
				insideQuotes = afalse;
				if((ii + 1) < sourceLength) {
					continue;
				}
			}
			else {
				insideQuotes = atrue;
				continue;
			}
		}

		found = afalse;

		if((recognizeQuotes) && (insideQuotes)) {
			if((ii + 1) == sourceLength) {
				found = atrue;
				tokenLength = ((ii - ref) + 1);
			}
		}
		else {
			for(jj = 0; jj < delimiterLength; jj++) {
				if(source[ii] == delimiterList[jj]) {
					found = atrue;
					tokenLength = (ii - ref);
					break;
				}
			}

			if(!found) {
				if((ii + 1) == sourceLength) {
					found = atrue;
					tokenLength = ((ii - ref) + 1);
				}
			}
		}

		if(found) {
			if(tokenLength >= minimumTokenLength) {
				tokenListLength += 1;
			}
			ref = (ii + 1);
		}
	}

	if(tokenListLength == 0) {
		return NULL;
	}

	// execute source tokenization

	result = (char **)malloc(sizeof(char *) * tokenListLength);

	insideQuotes = afalse;

	for(ii = 0, ref = 0; ii < sourceLength; ii++) {
		if((recognizeQuotes) && (source[ii] == '"')) {
			if(insideQuotes) {
				insideQuotes = afalse;
				if((ii + 1) < sourceLength) {
					continue;
				}
			}
			else {
				insideQuotes = atrue;
				continue;
			}
		}

		found = afalse;

		if((recognizeQuotes) && (insideQuotes)) {
			if((ii + 1) == sourceLength) {
				found = atrue;
				tokenLength = ((ii - ref) + 1);
			}
		}
		else {
			for(jj = 0; jj < delimiterLength; jj++) {
				if(source[ii] == delimiterList[jj]) {
					found = atrue;
					tokenLength = (ii - ref);
					break;
				}
			}

			if(!found) {
				if((ii + 1) == sourceLength) {
					found = atrue;
					tokenLength = ((ii - ref) + 1);
				}
			}
		}

		if(found) {
			if(tokenLength >= minimumTokenLength) {
				token = malloc(sizeof(char) * (tokenLength + 1));

				for(kk = 0; kk < tokenLength; kk++, ref++) {
					token[kk] = source[ref];
				}

				result[resultRef] = token;

				resultRef++;

				if(resultRef >= tokenListLength) {
					break;
				}
			}

			ref = (ii + 1);
		}
	}

	*tokenCount = resultRef;

	return result;
}

static char *parseUid(char *token, int tokenLength, int *uidLength)
{
	int ii = 0;
	int nn = 0;
	char *result = NULL;

	if((token == NULL) || (tokenLength < 1) || (token[0] != '+') ||
			(uidLength == NULL)) {
		return NULL;
	}

	*uidLength = 0;

	result = (char *)malloc(sizeof(char) * tokenLength);

	for(ii = 0, nn = 1; nn < tokenLength; ii++, nn++) {
		result[ii] = token[nn];
	}

	*uidLength = ii;

	return result;
}

static aboolean parseRequest(ForensicLogEntry *logEntry, char *token,
		int tokenLength)
{
	int ii = 0;
	int nn = 0;
	int ref = 0;
	char *buffer = NULL;

	buffer = (char *)malloc(sizeof(char) * tokenLength);

	// obtain request type - e.g. GET, POST, etc.

	for(ii = 0, nn = 0; ii < tokenLength; ii++) {
		if(token[ii] == ' ') {
			break;
		}

		buffer[nn] = token[ii];
		nn++;
	}

	if(nn <= 0) {
		free(buffer);
		return afalse;
	}

	logEntry->requestType = strndup(buffer, nn);
	logEntry->requestTypeLength = nn;

	// obtain request URL

	ref = (ii + 1);

	memset(buffer, 0, (int)(sizeof(char) * tokenLength));

	for(ii = ref, nn = 0; ii < tokenLength; ii++) {
		if(token[ii] == ' ') {
			break;
		}

		buffer[nn] = token[ii];
		nn++;
	}

	if(nn <= 0) {
		free(buffer);
		return afalse;
	}

	logEntry->requestUrl = strndup(buffer, nn);
	logEntry->requestUrlLength = nn;

	// obtain request protocol - e.g. HTTP

	ref = (ii + 1);

	memset(buffer, 0, (int)(sizeof(char) * tokenLength));

	for(ii = ref, nn = 0; ii < tokenLength; ii++) {
		if(token[ii] == '/') {
			break;
		}

		buffer[nn] = token[ii];
		nn++;
	}

	if(nn <= 0) {
		free(buffer);
		return afalse;
	}

	logEntry->requestProtocol = strndup(buffer, nn);
	logEntry->requestProtocolLength = nn;

	// obtain request protocol version

	ref = (ii + 1);

	memset(buffer, 0, (int)(sizeof(char) * tokenLength));

	for(ii = ref, nn = 0; ii < tokenLength; ii++) {
		buffer[nn] = token[ii];
		nn++;
	}

	if(nn <= 0) {
		free(buffer);
		return afalse;
	}

	logEntry->requestVersion = strndup(buffer, nn);
	logEntry->requestVersionLength = nn;

	// cleanup

	free(buffer);

	return atrue;
}

static ForensicLogEntryHeader *parseHeader(char *token, int tokenLength)
{
	aboolean hasName = afalse;
	int ii = 0;
	int nn = 0;

	ForensicLogEntryHeader *result = NULL;

	result = (ForensicLogEntryHeader *)malloc(sizeof(ForensicLogEntryHeader));

	result->name = (char *)malloc(sizeof(char) * tokenLength);
	result->value = (char *)malloc(sizeof(char) * tokenLength);

	for(ii = 0, nn = 0; ii < tokenLength; ii++) {
		if(!hasName) {
			if(token[ii] == ':') {
				result->nameLength = nn;
				hasName = atrue;
				nn = 0;
			}
			else {
				result->name[nn] = token[ii];
				nn++;
			}
		}
		else {
			result->value[nn] = token[ii];
			nn++;
		}
	}

	if(!hasName) {
		free(result->name);
		free(result->value);
		free(result);
		return NULL;
	}

	result->valueLength = nn;

	if((result->nameLength <= 0) || (result->valueLength <= 0)) {
		free(result->name);
		free(result->value);
		free(result);
		return NULL;
	}

	return result;
}

static ForensicLogEntry *buildLogEntry(ForensicDaemon *forensicDaemon,
		char *line, int lineLength)
{
	int ii = 0;
	int ref = 0;
	int tokenCount = 0;
	char **tokens = NULL;

	const int MINIMUM_TOKEN_LENGTH = 3;

	const char DELIMITERS[] = { '|', '\0' };

	ForensicLogEntry *result = NULL;
	ForensicLogEntryHeader *header = NULL;

	if((line == NULL) || (lineLength <= 0)) {
		return NULL;
	}

	if(line[0] != '+') {
		return NULL;
	}

	if(((tokens = string_tokenize(line,
						lineLength,
						(char *)DELIMITERS, 
						MINIMUM_TOKEN_LENGTH,
						afalse,
						&tokenCount)) == NULL) ||
			(tokenCount < 1)) {
		if(tokens != NULL) {
			for(ii = 0; ii < tokenCount; ii++) {
				if(tokens[ii] != NULL) {
					free(tokens[ii]);
				}
			}
			free(tokens);
		}
		return NULL;
	}

	result = (ForensicLogEntry *)malloc(sizeof(ForensicLogEntry));

	for(ii = 0; ii < tokenCount; ii++) {
		if(forensicDaemon->config.debugMode) {
			log_logf(&forensicDaemon->log, LOG_LEVEL_DEBUG,
					"token[%02i] :: '%s'", ii, tokens[ii]);
		}

		if(ii == 0) {
			if(((result->uid = parseUid(tokens[ii],
								strlen(tokens[ii]),
								&result->uidLength)) == NULL) ||
					(result->uidLength <= 0)) {
				freeLogEntry(result);
				return NULL;
			}

			log_logf(&forensicDaemon->log, LOG_LEVEL_DEBUG,
					"constructed UID '%s'", result->uid);
		}
		else if(ii == 1) {
			if(!parseRequest(result, tokens[ii], strlen(tokens[ii]))) {
				freeLogEntry(result);
				return NULL;
			}

			log_logf(&forensicDaemon->log, LOG_LEVEL_DEBUG,
					"constructed request type '%s'", result->requestType);
			log_logf(&forensicDaemon->log, LOG_LEVEL_DEBUG,
					"constructed request URL '%s'", result->requestUrl);
			log_logf(&forensicDaemon->log, LOG_LEVEL_DEBUG,
					"constructed request protocol '%s'",
					result->requestProtocol);
			log_logf(&forensicDaemon->log, LOG_LEVEL_DEBUG,
					"constructed request version '%s'",
					result->requestVersion);
		}
		else {
			if((header = parseHeader(tokens[ii],
							strlen(tokens[ii]))) != NULL) {
				log_logf(&forensicDaemon->log, LOG_LEVEL_DEBUG,
						"constructed header '%s'->'%s'",
						header->name, header->value);

				if(!strcmp(header->name, "Cookie")) {
					/*
					 * TODO: handle cookies
					 */
				}
				else {
					if(result->headers == NULL) {
						ref = 0;
						result->headerLength = 1;
						result->headers = (ForensicLogEntryHeader **)malloc(
								sizeof(ForensicLogEntryHeader *) *
								result->headerLength);
					}
					else {
						ref = result->headerLength;
						result->headerLength += 1;
						result->headers = (ForensicLogEntryHeader **)realloc(
								result->headers,
								(sizeof(ForensicLogEntryHeader *) *
								 result->headerLength));
					}

					result->headers[ref] = header;
				}
			}
		}
	}

	result->assemblyCounter = 0;

	// cleanup

	for(ii = 0; ii < tokenCount; ii++) {
		if(tokens[ii] != NULL) {
			free(tokens[ii]);
		}
	}

	free(tokens);

	return result;
}

static char *logEntryToString(ForensicLogEntry *logEntry,
		AccessLogEntry *accessLogEntry, int *stringLength)
{
	int ii = 0;
	char *result = NULL;

	Json *object = NULL;
	Json *headers = NULL;
	Json *accessLog = NULL;

	*stringLength = 0;

	if((object = json_new()) == NULL) {
		return NULL;
	}

	if((headers = json_new()) == NULL) {
		return NULL;
	}

	if(accessLogEntry != NULL) {
		if((accessLog = json_new()) == NULL) {
			return NULL;
		}
	}

	json_addString(object, "uid", logEntry->uid);
	json_addString(object, "requestType", logEntry->requestType);
	json_addString(object, "requestUrl", logEntry->requestUrl);
	json_addString(object, "requestProtocol", logEntry->requestProtocol);
	json_addString(object, "requestVersion", logEntry->requestVersion);

	for(ii = 0; ii < logEntry->headerLength; ii++) {
		json_addString(headers, logEntry->headers[ii]->name,
				logEntry->headers[ii]->value);
	}

	json_addObject(object, "headers", headers);

	if(accessLogEntry != NULL) {
		json_addString(accessLog, "uid", accessLogEntry->uid);
		json_addString(accessLog, "remoteHost", accessLogEntry->remoteHost);
		json_addString(accessLog, "remoteLogname",
				accessLogEntry->remoteLogname);
		json_addString(accessLog, "remoteUser", accessLogEntry->remoteUser);
		json_addString(accessLog, "timestamp", accessLogEntry->timestamp);
		json_addString(accessLog, "requestType", accessLogEntry->requestType);
		json_addString(accessLog, "requestUrl", accessLogEntry->requestUrl);
		json_addString(accessLog, "requestProtocol",
				accessLogEntry->requestProtocol);
		json_addString(accessLog, "requestVersion",
				accessLogEntry->requestVersion);
		json_addString(accessLog, "status", accessLogEntry->status);
		json_addString(accessLog, "responseLength",
				accessLogEntry->responseLength);
		json_addString(accessLog, "referer", accessLogEntry->referer);
		json_addString(accessLog, "userAgent", accessLogEntry->userAgent);

		json_addObject(object, "accessLog", accessLog);
	}

	if(((result = json_toString(object, stringLength)) == NULL) ||
			(*stringLength < 1)) {
		if(result != NULL) {
			free(result);
		}
		*stringLength = 0;
		json_freePtr(object);
		return NULL;
	}

	json_freePtr(object);

	return result;
}

static void freeLogEntry(void *memory)
{
	int ii = 0;

	ForensicLogEntry *logEntry = NULL;

	if(memory == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	logEntry = (ForensicLogEntry *)memory;

	if(logEntry->uid != NULL) {
		free(logEntry->uid);
	}

	if(logEntry->requestType != NULL) {
		free(logEntry->requestType);
	}

	if(logEntry->requestUrl != NULL) {
		free(logEntry->requestUrl);
	}

	if(logEntry->requestProtocol != NULL) {
		free(logEntry->requestProtocol);
	}

	if(logEntry->requestVersion != NULL) {
		free(logEntry->requestVersion);
	}

	if(logEntry->cookies != NULL) {
		for(ii = 0; ii < logEntry->cookieLength; ii++) {
			if(logEntry->cookies[ii] != NULL) {
				free(logEntry->cookies[ii]);
			}
		}
		free(logEntry->cookies);
	}

	if(logEntry->headers != NULL) {
		for(ii = 0; ii < logEntry->cookieLength; ii++) {
			if(logEntry->headers[ii] != NULL) {
				if(logEntry->headers[ii]->name != NULL) {
					free(logEntry->headers[ii]->name);
				}
				if(logEntry->headers[ii]->value != NULL) {
					free(logEntry->headers[ii]->value);
				}
				free(logEntry->headers[ii]);
			}
		}
		free(logEntry->headers);
	}

	free(logEntry);
}

static AccessLogEntry *buildAccessLogEntry(ForensicDaemon *forensicDaemon,
		char *line, int lineLength)
{
	aboolean insideString = afalse;
	int ii = 0;
	int nn = 0;
	int tempLength = 0;
	int tokenRef = 0;
	int tokenLength = 0;
	char *temp = NULL;
	char **tokens = NULL;

	AccessLogEntry *result = NULL;
	ForensicLogEntry *forensic = NULL;

	// process the line into tokens

	tokenLength = 11;
	tokens = (char **)malloc(sizeof(char *) * tokenLength);
	tokens[0] = (char *)malloc(sizeof(char) * lineLength);

	for(ii = 0, nn = 0; ii < lineLength; ii++) {
		if(line[ii] == '"') {
			if(insideString) {
				insideString = afalse;
			}
			else {
				insideString = atrue;
			}
			continue;
		}

		if((!insideString) && (line[ii] == ' ')) {
			nn = 0;
			tokenRef++;
			if(tokenRef >= tokenLength) {
				tokenLength *= 2;
				tokens = (char **)realloc(tokens,
						(sizeof(char *) * tokenLength));
			}
			tokens[tokenRef] = (char *)malloc(sizeof(char) * lineLength);
		}
		else {
			tokens[tokenRef][nn] = line[ii];
			nn++;
		}
	}

	tokenLength = (tokenRef + 1);

	if((tokens == NULL) || (tokenLength < 11)) {
		if(tokens != NULL) {
			for(ii = 0; ii < tokenLength; ii++) {
				if(tokens[ii] != NULL) {
					free(tokens[ii]);
				}
			}
			free(tokens);
		}
		return NULL;
	}

	for(ii = 0; ii < tokenLength; ii++) {
		log_logf(&forensicDaemon->log, LOG_LEVEL_DEBUG,
				"{ACCESS} token[%02i] is '%s'", ii, tokens[ii]);
	}

	// build result

	result = (AccessLogEntry *)malloc(sizeof(AccessLogEntry));

	result->remoteHost = copyToken(tokens[0],
			&result->remoteHostLength);

	log_logf(&forensicDaemon->log, LOG_LEVEL_DEBUG,
			"{ACCESS} constructed remote host '%s'", result->remoteHost);

	result->remoteLogname = copyToken(tokens[1],
			&result->remoteLognameLength);

	log_logf(&forensicDaemon->log, LOG_LEVEL_DEBUG,
			"{ACCESS} constructed remote logname '%s'", result->remoteLogname);

	result->remoteUser = copyToken(tokens[2],
			&result->remoteUserLength);

	log_logf(&forensicDaemon->log, LOG_LEVEL_DEBUG,
			"{ACCESS} constructed remote user '%s'", result->remoteUser);

	result->timestamp = copyToken(tokens[3],
			&result->timestampLength);

	temp = copyToken(tokens[4], &tempLength);

	result->timestampLength += (tempLength + 1);

	result->timestamp = (char *)realloc(result->timestamp,
			(sizeof(char) * result->timestampLength));

	strcat(result->timestamp, temp);

	free(temp);

	log_logf(&forensicDaemon->log, LOG_LEVEL_DEBUG,
			"{ACCESS} constructed timestamp '%s'", result->timestamp);

	// parse the request token

	forensic = (ForensicLogEntry *)malloc(sizeof(ForensicLogEntry));

	if(!parseRequest(forensic, tokens[5], strlen(tokens[5]))) {
		if(tokens != NULL) {
			for(ii = 0; ii < tokenLength; ii++) {
				if(tokens[ii] != NULL) {
					free(tokens[ii]);
				}
			}
			free(tokens);
		}

		freeAccessLogEntry(result);
		freeLogEntry(forensic);

		return NULL;
	}

	result->requestTypeLength = forensic->requestTypeLength;
	result->requestType = strndup(forensic->requestType,
			forensic->requestTypeLength);

	log_logf(&forensicDaemon->log, LOG_LEVEL_DEBUG,
			"{ACCESS} constructed request type '%s'", result->requestType);

	result->requestUrlLength = forensic->requestUrlLength;
	result->requestUrl = strndup(forensic->requestUrl,
			forensic->requestUrlLength);

	log_logf(&forensicDaemon->log, LOG_LEVEL_DEBUG,
			"{ACCESS} constructed request url '%s'", result->requestUrl);

	result->requestProtocolLength = forensic->requestProtocolLength;
	result->requestProtocol = strndup(forensic->requestProtocol,
			forensic->requestProtocolLength);

	log_logf(&forensicDaemon->log, LOG_LEVEL_DEBUG,
			"{ACCESS} constructed request protocol '%s'",
			result->requestProtocol);

	result->requestVersionLength = forensic->requestVersionLength;
	result->requestVersion = strndup(forensic->requestVersion,
			forensic->requestVersionLength);

	log_logf(&forensicDaemon->log, LOG_LEVEL_DEBUG,
			"{ACCESS} constructed request version '%s'",
			result->requestVersion);

	freeLogEntry(forensic);

	// status

	result->status = copyToken(tokens[6],
			&result->statusLength);

	log_logf(&forensicDaemon->log, LOG_LEVEL_DEBUG,
			"{ACCESS} constructed status '%s'", result->status);

	// response length in bytes

	result->responseLength = copyToken(tokens[7],
			&result->responseLengthLength);

	log_logf(&forensicDaemon->log, LOG_LEVEL_DEBUG,
			"{ACCESS} constructed response-length '%s'",
			result->responseLength);

	// referer

	result->referer = copyToken(tokens[8],
			&result->refererLength);

	log_logf(&forensicDaemon->log, LOG_LEVEL_DEBUG,
			"{ACCESS} constructed referer '%s'", result->referer);

	// user agent

	result->userAgent = copyToken(tokens[9],
			&result->userAgentLength);

	log_logf(&forensicDaemon->log, LOG_LEVEL_DEBUG,
			"{ACCESS} constructed user agent '%s'", result->userAgent);

	// uid

	result->uid = copyToken(tokens[10],
			&result->uidLength);

	log_logf(&forensicDaemon->log, LOG_LEVEL_DEBUG,
			"{ACCESS} constructed uid '%s'", result->uid);

	result->creationTime = time_getTimeMus();

	// cleanup

	for(ii = 0; ii < tokenLength; ii++) {
		if(tokens[ii] != NULL) {
			free(tokens[ii]);
		}
	}

	free(tokens);

	return result;
}

static void freeAccessLogEntry(void *memory)
{
	AccessLogEntry *logEntry = NULL;

	if(memory == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	logEntry = (AccessLogEntry *)memory;

	if(logEntry->uid != NULL) {
		free(logEntry->uid);
	}

	if(logEntry->remoteHost != NULL) {
		free(logEntry->remoteHost);
	}

	if(logEntry->remoteLogname != NULL) {
		free(logEntry->remoteLogname);
	}

	if(logEntry->remoteUser != NULL) {
		free(logEntry->remoteUser);
	}

	if(logEntry->timestamp != NULL) {
		free(logEntry->timestamp);
	}

	if(logEntry->requestType != NULL) {
		free(logEntry->requestType);
	}

	if(logEntry->requestUrl != NULL) {
		free(logEntry->requestUrl);
	}

	if(logEntry->requestProtocol != NULL) {
		free(logEntry->requestProtocol);
	}

	if(logEntry->requestVersion != NULL) {
		free(logEntry->requestVersion);
	}

	if(logEntry->status != NULL) {
		free(logEntry->status);
	}

	if(logEntry->responseLength != NULL) {
		free(logEntry->responseLength);
	}

	if(logEntry->referer != NULL) {
		free(logEntry->referer);
	}

	if(logEntry->userAgent != NULL) {
		free(logEntry->userAgent);
	}

	free(logEntry);
}

static char *buildHtmlRequest(ForensicDaemon *forensicDaemon, char *message,
		int messageLength, int *requestLength)
{
	int length = 0;
	int payloadLength = 0;
	char *result = NULL;
	char *payload = NULL;

	const char *TEMPLATE = ""
		"POST %s HTTP/1.1\x0d\x0a"
		"Host: %s\x0d\x0a"
		"User-Agent: Forensic Daemon/%s (%s; en-US) Asgard/%s\x0d\x0a"
		"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\x0d\x0a"
		"Accept-Language: en-us,en;q=0.5\x0d\x0a"
		"Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7\x0d\x0a"
		"Keep-Alive: 115\x0d\x0a"
		"Connection: keep-alive\x0d\x0a"
		"Cache-Control: max-age=0\x0d\x0a"
		"Content-Type: application/x-www-form-urlencoded\x0d\x0a"
		"Content-Length: %i\x0d\x0a"
		"\x0d\x0a"
		"%s"
		"";

	const char *PAYLOAD_TEMPLATE = ""
		"attributes=%s"
		"";

	*requestLength = 0;

	payloadLength = (strlen(PAYLOAD_TEMPLATE) +
			messageLength +
			128);

	payload = (char *)malloc(sizeof(char) * (payloadLength + 8));

	payloadLength = snprintf(payload,
			(int)(sizeof(char) * payloadLength),
			PAYLOAD_TEMPLATE,
			message);

	length = (strlen(TEMPLATE) +
			strlen(forensicDaemon->config.url) +
			strlen(forensicDaemon->config.ipAddress) +
			payloadLength +
			128);

	result = (char *)malloc(sizeof(char) * (length + 8));

	*requestLength = snprintf(result,
			(int)(sizeof(char) * length),
			TEMPLATE,
			forensicDaemon->config.url,
			forensicDaemon->config.ipAddress,
			ASGARD_VERSION,
			ASGARD_VERSION,
			ASGARD_DATE,
			payloadLength,
			payload);

	free(payload);

	return result;
}

static int executeHttpTransaction(ForensicDaemon *forensicDaemon, char *message,
		int messageLength)
{
	int sd = 0;
	int result = 0;
	int bufferLength = 0;
	char *buffer = NULL;

	Socket socket;

	if(socket_init(&socket,
				SOCKET_MODE_CLIENT,
				SOCKET_PROTOCOL_TCPIP,
				forensicDaemon->config.ipAddress,
				forensicDaemon->config.port
				) < 0) {
		log_logf(&forensicDaemon->log, LOG_LEVEL_ERROR,
				"%s() failed at line %i", __FUNCTION__, __LINE__);
		return -1;
	}

	if(socket_open(&socket) < 0) {
		log_logf(&forensicDaemon->log, LOG_LEVEL_ERROR,
				"%s() failed at line %i", __FUNCTION__, __LINE__);
		return -1;
	}

	if((sd = socket_getSocketDescriptor(&socket)) < 0) {
		log_logf(&forensicDaemon->log, LOG_LEVEL_ERROR,
				"%s() failed at line %i", __FUNCTION__, __LINE__);
		return -1;
	}

	// send message from client to server

	buffer = buildHtmlRequest(forensicDaemon, message, messageLength,
			&bufferLength);

	if(http_protocol_send(&forensicDaemon->httpProtocol, &socket,
				sd, buffer, bufferLength) < 0) {
		free(buffer);
		log_logf(&forensicDaemon->log, LOG_LEVEL_ERROR,
				"%s() failed at line %i", __FUNCTION__, __LINE__);
		return -1;
	}

	log_logf(&forensicDaemon->log, LOG_LEVEL_DEBUG,
			"client sent %i bytes to server '%s:%i' on fd #%i => '%s'",
			bufferLength, forensicDaemon->config.ipAddress,
			forensicDaemon->config.port, sd, buffer);

	free(buffer);

	// read server socket transmission

	buffer = http_protocol_receive(&forensicDaemon->httpProtocol, &socket,
			sd, &bufferLength);

	log_logf(&forensicDaemon->log, LOG_LEVEL_DEBUG,
			"client received %i bytes from server '%s:%i' on fd #%i => '%s'",
			bufferLength, forensicDaemon->config.ipAddress,
			forensicDaemon->config.port, sd, buffer);

	/*
	 * TODO: check contents of response (if required)
	 */

	if(buffer != NULL) {
		free(buffer);
	}

	if(socket_close(&socket) < 0) {
		log_logf(&forensicDaemon->log, LOG_LEVEL_ERROR,
				"%s() failed at line %i", __FUNCTION__, __LINE__);
		result = -1;
	}

	socket_free(&socket);

	return result;
}

static void *workerThread(void *threadContext, void *argument)
{
	int rc = 0;
	int iValue = 0;
	int counter = 0;
	int keyLength = 0;
	int lineLength = 0;
	int stringLength = 0;
	int accessLineRef = 0;
	int accessLineLength = 0;
	int accessFileOpenAttempts = 0;
	alint itemCount = 0;
	double elapsedTime = 0.0;
	char cValue = (char)0;
	char *key = NULL;
	char *line = NULL;
	char *string = NULL;
	char *accessLine = NULL;
	char *errorMessage = NULL;

	FifoStack logQueue;

	TmfThread *thread = NULL;
	ForensicDaemon *forensicDaemon = NULL;
	AccessLogEntry *accessLogEntry = NULL;
	ForensicLogEntry *logEntry = NULL;

	thread = (TmfThread *)threadContext;
	forensicDaemon = (ForensicDaemon *)argument;

	log_logf(&forensicDaemon->log, LOG_LEVEL_DEBUG,
			"%s() executed on 0x%lx with argument 0x%lx",
			__FUNCTION__, (aptrcast)threadContext, (aptrcast)argument);

	log_logf(&forensicDaemon->log, LOG_LEVEL_INFO,
			"forensic daemon worker thread %s() #%i (%i) running",
			__FUNCTION__, thread->uid, thread->systemId);

	fifostack_init(&logQueue);
	fifostack_setFreeFunction(&logQueue, freeLogEntry);

	accessLineRef = 0;
	accessLineLength = 8192;
	accessLine = (char *)malloc(sizeof(char) * accessLineLength);

	while((thread->state == THREAD_STATE_RUNNING) &&
			(thread->signal == THREAD_SIGNAL_OK)) {
		// mark thread status as busy

		mutex_lock(thread->mutex);
		thread->status = THREAD_STATUS_BUSY;
		mutex_unlock(thread->mutex);

		// read from the forensic access log

		if(forensicDaemon->config.readAccessLog) {
			counter = 0;

			do {
				rc = 0;

				if(((rc = file_readChar(&forensicDaemon->fh, &cValue)) < 0) &&
						(rc != FILE_EOF)) {
					if(file_getError(&forensicDaemon->fh, rc,
								&errorMessage) < 0) {
						errorMessage = "unknown file error";
					}

					log_logf(&forensicDaemon->log, LOG_LEVEL_ERROR,
							"failed to read from forensic access logfile '%s' "
							"with '%s'",
							forensicDaemon->config.accessForensicLog,
							errorMessage);
				}
				else if(rc == FILE_EOF) {
					log_logf(&forensicDaemon->log, LOG_LEVEL_DEBUG,
							"read EOF from forensic access logfile '%s'",
							forensicDaemon->config.accessForensicLog);
				}

				if(rc >= 0) {
					iValue = ctype_ctoi(cValue);

					if(iValue == 10) {
						accessLine[accessLineRef] = '\0';

						log_logf(&forensicDaemon->log, LOG_LEVEL_DEBUG,
								"read forensic access line '%s'", accessLine);

						if((accessLogEntry = buildAccessLogEntry(
										forensicDaemon,
										accessLine,
										accessLineLength)) != NULL) {
							if(bptree_put(&forensicDaemon->index,
										accessLogEntry->uid,
										accessLogEntry->uidLength,
										(void *)accessLogEntry) < 0) {
								freeAccessLogEntry(accessLogEntry);
							}
						}

						memset(accessLine, 0, accessLineLength);

						accessLineRef = 0;
					}
					else if((iValue >= 32) && (iValue <= 126)) {
/*						log_logf(&forensicDaemon->log, LOG_LEVEL_DEBUG,
								"READ ACCESS CHAR '%c' (%s)",
								cValue, accessLine);*/

						accessLine[accessLineRef] = cValue;
						accessLineRef++;

						if(accessLineRef == 2) {
							if(accessLine[0] == accessLine[1]) {
								accessLineRef = 0;
							}
						}

						if(accessLineRef >= accessLineLength) {
							accessLineLength *= 2;
							accessLine = (char *)realloc(accessLine,
									(sizeof(char) * accessLineLength));
						}
					}
				}

				counter++;
			} while((rc >= 0) &&
					(counter < 1024) &&
					(thread->state == THREAD_STATE_RUNNING) &&
					(thread->signal == THREAD_SIGNAL_OK));
		}
		else if(accessFileOpenAttempts < 8) {
			if((rc = file_init(&forensicDaemon->fh,
						forensicDaemon->config.accessForensicLog,
						"r",
						8192)) < 0) {
				if(file_getError(&forensicDaemon->fh, rc, &errorMessage) < 0) {
					errorMessage = "unknown file error";
				}

				accessFileOpenAttempts++;

				log_logf(&forensicDaemon->log, LOG_LEVEL_ERROR,
						"failed to open (attempt #%i) forensic access "
						"logfile '%s' with '%s'",
						accessFileOpenAttempts,
						forensicDaemon->config.accessForensicLog,
						errorMessage);

				file_free(&forensicDaemon->fh);
			}
			else {
				forensicDaemon->config.readAccessLog = atrue;
			}
		}

		// obtain lines from the FIFO queue

		line = getLineFromQueue(forensicDaemon, &lineLength);

		while((line != NULL) &&
				(thread->state == THREAD_STATE_RUNNING) &&
				(thread->signal == THREAD_SIGNAL_OK)) {
			if(lineLength <= 0) {
				if(line != NULL) {
					free(line);
				}

				line = getLineFromQueue(forensicDaemon, &lineLength);

				continue;
			}

			log_logf(&forensicDaemon->log, LOG_LEVEL_DEBUG,
					"obtained line(length %i bytes) '%s' from queue",
					lineLength, line);

			if((logEntry = buildLogEntry(forensicDaemon, line,
							lineLength)) != NULL) {
				if(fifostack_push(&logQueue, logEntry) < 0) {
					freeLogEntry(logEntry);
				}
			}

			free(line);

			if((thread->state != THREAD_STATE_RUNNING) ||
					(thread->signal != THREAD_SIGNAL_OK)) {
				break;
			}

			line = getLineFromQueue(forensicDaemon, &lineLength);
		}

		if((thread->state != THREAD_STATE_RUNNING) ||
				(thread->signal != THREAD_SIGNAL_OK)) {
			break;
		}

		// obtain log entries from the log queue and assemble them

		counter = 0;

		if(fifostack_getItemCount(&logQueue, &itemCount) < 0) {
			itemCount = 0;
		}

		rc = fifostack_pop(&logQueue, ((void *)&logEntry));

		while(((alint)counter < itemCount) &&
				(counter < 128) &&
				(rc == 0) &&
				(logEntry != NULL) &&
				(thread->state == THREAD_STATE_RUNNING) &&
				(thread->signal == THREAD_SIGNAL_OK)) {
			accessLogEntry = NULL;

			log_logf(&forensicDaemon->log, LOG_LEVEL_DEBUG,
					"examining forensic log entry '%s'",
					logEntry->uid);

			if(forensicDaemon->config.readAccessLog) {
				if((bptree_get(&forensicDaemon->index,
								logEntry->uid,
								logEntry->uidLength,
								((void *)&accessLogEntry)) < 0) ||
						(accessLogEntry == NULL)) {
					logEntry->assemblyCounter += 1;

					if(logEntry->assemblyCounter >= 8) {
						log_logf(&forensicDaemon->log, LOG_LEVEL_DEBUG,
							"dropped log entry '%s', failed to assemble",
							logEntry->uid);

						freeLogEntry(logEntry);
					}
					else {
						if(fifostack_push(&logQueue, logEntry) < 0) {
							log_logf(&forensicDaemon->log, LOG_LEVEL_ERROR,
									"failed to re-add '%s' to the log queue",
									logEntry->uid);

							freeLogEntry(logEntry);
						}
					}

					counter++;

					rc = fifostack_pop(&logQueue, ((void *)&logEntry));

					continue;
				}
			}

			if(accessLogEntry != NULL) {
				log_logf(&forensicDaemon->log, LOG_LEVEL_DEBUG,
						"found access log entry for '%s'",
						logEntry->uid);
			}

			if((string = logEntryToString(logEntry, accessLogEntry,
							&stringLength)) != NULL) {
				log_logf(&forensicDaemon->log, LOG_LEVEL_DEBUG,
						"constructed JSON(length %i bytes) '%s' from line",
						stringLength, string);

				executeHttpTransaction(forensicDaemon, string, stringLength);

				if((forensicDaemon->config.readAccessLog) &&
						(accessLogEntry != NULL)) {
					bptree_remove(&forensicDaemon->index,
							accessLogEntry->uid,
							accessLogEntry->uidLength);
				}
			}
			else {
				log_logf(&forensicDaemon->log, LOG_LEVEL_ERROR,
						"failed to construct JSON object for '%s'",
						logEntry->uid);
			}

			freeLogEntry(logEntry);

			counter++;

			rc = fifostack_pop(&logQueue, ((void *)&logEntry));
		}

		if((thread->state != THREAD_STATE_RUNNING) ||
				(thread->signal != THREAD_SIGNAL_OK)) {
			break;
		}

		if((((alint)counter >= itemCount) || (counter >= 128)) &&
				((rc == 0) && (logEntry != NULL))) {
			if(fifostack_push(&logQueue, logEntry) < 0) {
				log_logf(&forensicDaemon->log, LOG_LEVEL_ERROR,
						"failed to re-add '%s' to the log queue",
						logEntry->uid);

				freeLogEntry(logEntry);
			}
		}

		// interate index and remove old access log entries

		counter = 0;

		key = strdup("");
		keyLength = strlen(key);

		rc = bptree_getNext(&forensicDaemon->index, key, keyLength,
				((void *)&accessLogEntry));

		while((counter < 1024) &&
				(rc == 0) &&
				(accessLogEntry != NULL) &&
				(thread->state == THREAD_STATE_RUNNING) &&
				(thread->signal == THREAD_SIGNAL_OK)) {
			free(key);

			key = strndup(accessLogEntry->uid, accessLogEntry->uidLength);
			keyLength = accessLogEntry->uidLength;

			elapsedTime = time_getElapsedMusInSeconds(
					accessLogEntry->creationTime);

			log_logf(&forensicDaemon->log, LOG_LEVEL_DEBUG,
					"examining b+tree index '%s' @ %0.2f seconds",
					accessLogEntry->uid, elapsedTime);

			if(elapsedTime >= 120.0) {
				bptree_remove(&forensicDaemon->index,
						accessLogEntry->uid,
						accessLogEntry->uidLength);
			}

			counter++;

			rc = bptree_getNext(&forensicDaemon->index, key, keyLength,
					((void *)&accessLogEntry));
		}

		free(key);

		if((thread->state != THREAD_STATE_RUNNING) ||
				(thread->signal != THREAD_SIGNAL_OK)) {
			break;
		}

		// report on internal status

		itemCount = getQueueItemCount(forensicDaemon);

		log_logf(&forensicDaemon->log, LOG_LEVEL_DEBUG,
				"the forensic line queue contains %i entries", (int)itemCount);

		if(fifostack_getItemCount(&logQueue, &itemCount) < 0) {
			itemCount = 0;
		}

		log_logf(&forensicDaemon->log, LOG_LEVEL_DEBUG,
				"the access log queue contains %i entries", (int)itemCount);

		bptree_getLeafCount(&forensicDaemon->index, &rc);

		log_logf(&forensicDaemon->log, LOG_LEVEL_DEBUG,
				"the b+tree index contains %i entries", rc);

		// mark thread status as napping

		mutex_lock(thread->mutex);
		thread->status = THREAD_STATUS_NAPPING;
		mutex_unlock(thread->mutex);

		// nap until the FIFO queue has items or the nap threshold has been met

		counter = 0;
		itemCount = getQueueItemCount(forensicDaemon);

		while((thread->state == THREAD_STATE_RUNNING) &&
				(thread->signal == THREAD_SIGNAL_OK) &&
				(itemCount <= 0) &&
				(counter < 1024)) {
			time_usleep(1024);

			counter++;

			itemCount = getQueueItemCount(forensicDaemon);
		}

		log_logf(&forensicDaemon->log, LOG_LEVEL_DEBUG,
				"%s() napped for %i iterations (%i items)",
				__FUNCTION__, counter, (int)itemCount);
	}

	// cleanup

	fifostack_free(&logQueue);

	free(accessLine);

	log_logf(&forensicDaemon->log, LOG_LEVEL_INFO,
			"forensic daemon worker thread %s() #%i (%i) halting",
			__FUNCTION__, thread->uid, thread->systemId);

	return (void *)NULL;
}

static void readStdin()
{
	unsigned char value = (unsigned char)0;
	int rc = 0;
	int iValue = 0;
	int lineRef = 0;
	int lineLength = 0;
	char *line = NULL;
	char *errorMessage = NULL;

	FileHandle fh;

	if(forensicDaemon.config.logRawForensic) {
		if((rc = file_init(&fh,
						forensicDaemon.config.rawForensicLog,
						"a",
						8192)) < 0) {
			if(file_getError(&fh, rc, &errorMessage) < 0) {
				errorMessage = "unknown file error";
			}

			log_logf(&forensicDaemon.log, LOG_LEVEL_ERROR,
					"failed to open raw forensic log '%s' with '%s'",
					forensicDaemon.config.rawForensicLog, errorMessage);

			file_free(&fh);

			forensicDaemon.config.logRawForensic = afalse;
		}
	}

	if(forensicDaemon.config.logRawForensic) {
		fprintf(fh.fd, "\n");
		fprintf(fh.fd, "------------/ daemon started  /------------\n");
	}

	lineRef = 0;
	lineLength = 8192;
	line = (char *)malloc(sizeof(char) * lineLength);

	while(!feof(stdin)) {
		if((rc = fgetc(stdin)) == EOF) {
			break;
		}

		value = (unsigned char)rc;
		iValue = ctype_ctoi((char)value);

		if(iValue == 10) {
			line[lineRef] = '\0';

			log_logf(&forensicDaemon.log, LOG_LEVEL_DEBUG,
					"read line '%s'", line);

			pushLineOntoQueue(&forensicDaemon, line, lineRef);

			memset(line, 0, lineLength);

			lineRef = 0;
		}
		else if((iValue >= 32) && (iValue <= 126)) {
			line[lineRef] = (char)value;
			lineRef++;
			if(lineRef >= lineLength) {
				lineLength *= 2;
				line = (char *)realloc(line, (sizeof(char) * lineLength));
			}
		}

		if(forensicDaemon.config.logRawForensic) {
			if((rc = file_writeChar(&fh, (char)value)) < 0) {
				if(file_getError(&fh, rc, &errorMessage) < 0) {
					errorMessage = "unknown file error";
				}
				log_logf(&forensicDaemon.log, LOG_LEVEL_ERROR,
						"failed to write to raw forensic log '%s' with '%s'",
						forensicDaemon.config.rawForensicLog, errorMessage);
			}
		}
	}

	free(line);

	if(forensicDaemon.config.logRawForensic) {
		fprintf(fh.fd, "------------/ daemon shutdown /------------\n\n");

		file_free(&fh);
	}
}

