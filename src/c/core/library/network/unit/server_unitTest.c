/*
 * server_unitTest.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The multithreaded server API to provide a framework for managed network
 * server transactions, unit test.
 *
 * Written by Josh English.
 */

// preprocessor directives

#define _CORE_COMMON_BYPASS_STD_STREAM
#include "core/core.h"


// define server unit test constants

#if defined(__linux__)
#	define BASIC_FUNCTIONALITY_LIB						\
		"lib/libasgardcore_unitTest.so"
#elif defined(WIN32)
#	define BASIC_FUNCTIONALITY_LIB						\
		"lib/libasgardcore_unitTest.dll"
#elif defined(__APPLE__)
#	define BASIC_FUNCTIONALITY_LIB						\
		"lib/libasgardcore_unitTest.dylib"
#else // - no plugin available -
#	error "no plugins defined for this unit-test"
#endif // - plugins -

//#define BASIC_FUNCTIONALITY_VERBOSE_MODE				atrue
#define BASIC_FUNCTIONALITY_VERBOSE_MODE				afalse

//#define BASIC_FUNCTIONALITY_LOG_LEVEL					LOG_LEVEL_DEBUG
#define BASIC_FUNCTIONALITY_LOG_LEVEL					LOG_LEVEL_INFO

#define BASIC_FUNCTIONALITY_LOG_OUTPUT					NULL
//#define BASIC_FUNCTIONALITY_LOG_OUTPUT					"log/server.unit.test.log"

//#define BASIC_FUNCTIONALITY_TESTS						128
#define BASIC_FUNCTIONALITY_TESTS						1024

#define BASIC_FUNCTIONALITY_MIN_THREADS					4

#define BASIC_FUNCTIONALITY_MAX_THREADS					16

//#define BASIC_FUNCTIONALITY_HOSTNAME					"localhost"
#define BASIC_FUNCTIONALITY_HOSTNAME					"192.168.0.1"
//#define BASIC_FUNCTIONALITY_HOSTNAME					"192.168.0.3"

#define BASIC_FUNCTIONALITY_PORT						6543


// declare server unit test data types

typedef struct _DefaultRequestContext {
	int requestLength;
	char *request;
} DefaultRequestContext;

typedef struct _DefaultResponseContext {
	int responseLength;
	char *response;
} DefaultResponseContext;


// declare server unit test private functions

static void defaultRequest_free(void *argument, void *memory);

static void defaultResponse_free(void *argument, void *memory);

static int executeClientTransaction();

static int server_basicFunctionalityTest();


// main function

int main()
{
	double timer = 0.0;

	signal_registerDefault();

	printf("[unit] server unit test "
			"(Asgard Ver %s on %s)\n", ASGARD_VERSION, ASGARD_DATE);

	timer = time_getTimeMus();

	if(server_basicFunctionalityTest() < 0) {
		printf("[unit]\t ...FAILED\n");
		return 1;
	}

	printf("[unit]\t ...PASSED (%0.6f seconds)\n",
			time_getElapsedMusInSeconds(timer));
	printf("[unit] ...PASSED\n");

	return 0;
}


// define server unit test private functions

static void defaultRequest_free(void *argument, void *memory)
{
	DefaultRequestContext *request = NULL;

	if(memory == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	request = (DefaultRequestContext *)memory;

	if(request->request != NULL) {
		free(request->request);
	}

	free(request);
}

static void defaultResponse_free(void *argument, void *memory)
{
	DefaultResponseContext *response = NULL;

	if(memory == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	response = (DefaultResponseContext *)memory;

	if(response->response != NULL) {
		free(response->response);
	}

	free(response);
}

static int executeClientTransaction()
{
	aboolean firstTime = atrue;
	int rc = 0;
	int clientSd = 0;
	int valueRef = 0;
	int sentLength = 0;
	int valueLength = 0;
	double timer = 0.0;
	double totalTimer = 0.0;
	double elapsedTime = 0.0;
	double remainingTime = 0.0;
	char chunk[4];
	char *value = NULL;
	char *peerName = NULL;

	const char *REQUEST_TEMPLATE = ""
		"GET / HTTP/1.1\x0d\x0a"
		"Host: www.google.com\x0d\x0a"
		"User-Agent: Socket Unit Test/%s (%s; en-US) Asgard/%s\x0d\x0a"
		"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\x0d\x0a"
		"Accept-Language: en-us,en;q=0.5\x0d\x0a"
		"Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7\x0d\x0a"
		"Keep-Alive: 115\x0d\x0a"
		"Connection: keep-alive\x0d\x0a"
		"Cache-Control: max-age=0\x0d\x0a"
		"\x0d\x0a";

	Socket clientSocket;

	// create client socket

	if(socket_init(&clientSocket,
				SOCKET_MODE_CLIENT,
				SOCKET_PROTOCOL_TCPIP,
				BASIC_FUNCTIONALITY_HOSTNAME,
				BASIC_FUNCTIONALITY_PORT
				) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(socket_open(&clientSocket) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if((clientSd = socket_getSocketDescriptor(&clientSocket)) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(BASIC_FUNCTIONALITY_VERBOSE_MODE) {
		printf("[unit]\t\t ...client socket descriptor: %i\n", clientSd);
	}

	if(socket_getPeerName(&clientSocket, clientSd, &peerName) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(BASIC_FUNCTIONALITY_VERBOSE_MODE) {
		printf("[unit]\t\t ...client socket peername: '%s'\n", peerName);
	}

	free(peerName);

	// send message from client to server

	valueLength = (strlen(REQUEST_TEMPLATE) + 128);
	value = (char *)malloc(sizeof(char) * valueLength);

	valueLength = snprintf(value, valueLength, REQUEST_TEMPLATE,
			ASGARD_VERSION, ASGARD_VERSION, ASGARD_DATE);

	if(socket_send(&clientSocket, clientSd, value, valueLength) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	sentLength = valueLength;

	free(value);

	if(BASIC_FUNCTIONALITY_VERBOSE_MODE) {
		printf("[unit]\t\t ...client socket sent %i bytes\n", valueLength);
	}

	// read server socket transmission

	valueRef = 0;
	valueLength = 1024;
	value = (char *)malloc(sizeof(char) * valueLength);

	timer = 0.0;
	elapsedTime = 0.0;
	remainingTime = 2.048;

	totalTimer = time_getTimeMus();

	do {
		timer = time_getTimeMus();

		memset(chunk, 0, (int)sizeof(chunk));

		if((rc = socket_receive(&clientSocket, clientSd, chunk, sizeof(chunk),
						remainingTime)) < 0) {
			break;
		}

		elapsedTime = time_getElapsedMusInSeconds(timer);

		if(firstTime) {
			if(elapsedTime > 0.008192) {
				remainingTime = elapsedTime;
			}
			else {
				remainingTime = 0.008192;
			}
			if(remainingTime > 2.048) {
				remainingTime = (2.048 - elapsedTime);
			}
			firstTime = afalse;
		}
		else {
			remainingTime -= elapsedTime;
		}

		if(rc > 0) {
			memcpy((value + valueRef), chunk, rc);
			valueRef += rc;
		}

		if(valueRef >= valueLength) {
			valueLength *= 2;
			value = (char *)realloc(value, (sizeof(char) * valueLength));
		}
	} while(remainingTime > 0.0);

	value[valueRef] = '\0';

	if(BASIC_FUNCTIONALITY_VERBOSE_MODE) {
		printf("\n\n\n"
				"-----/server message begin/-----\n"
				"%s\n"
				"-----/server message end/------\n"
				"[unit]\t\t ...server socket received %i bytes in "
				"%0.6f seconds\n",
				value, valueRef, time_getElapsedMusInSeconds(totalTimer));
	}

	free(value);

	// free the sockets

	if(socket_close(&clientSocket) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	socket_free(&clientSocket);

	// check lengths

	if(sentLength != valueRef) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	return 0;
}

static int server_basicFunctionalityTest()
{
	int ii = 0;
	void *handle = NULL;
	void *hashFunction = NULL;
	void *cloneFunction = NULL;

	TransactionManager manager;

	Server server;

	ArrayList *serverStatus = NULL;
	ServerStatus *entry = NULL;

	Log log;

	printf("[unit]\t server basic functionality test...\n");

	if(dl_open(BASIC_FUNCTIONALITY_LIB, &handle) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(dl_lookup(handle,
				"transactionManager_defaultHashFunction",
				&hashFunction) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(dl_lookup(handle,
				"transactionManager_defaultCloneFunction",
				&cloneFunction) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(BASIC_FUNCTIONALITY_LOG_OUTPUT != NULL) {
		log_init(&log, LOG_OUTPUT_FILE, BASIC_FUNCTIONALITY_LOG_OUTPUT,
				BASIC_FUNCTIONALITY_LOG_LEVEL);
	}
	else {
		log_init(&log, LOG_OUTPUT_STDOUT, BASIC_FUNCTIONALITY_LOG_OUTPUT,
				BASIC_FUNCTIONALITY_LOG_LEVEL);
	}

	if(transactionManager_init(&manager, &log) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(transactionManager_enableCache(&manager,
				defaultResponse_free,
				(void *)&manager,
				(TransactionManagerHashFunction)hashFunction,
				(void *)&manager,
				(TransactionManagerCloneFunction)cloneFunction) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(transactionManager_bindSymbolTable(&manager,
				BASIC_FUNCTIONALITY_LIB) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(transactionManager_setReceiveFunctionByName(&manager,
				(void *)&manager,
				"transactionManager_defaultReceiveFunction") < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(transactionManager_setRecvFreeFunction(&manager,
				defaultRequest_free) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(transactionManager_setSendFunctionByName(&manager,
				(void *)&manager,
				"transactionManager_defaultSendFunction") < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(transactionManager_setIdFunctionByName(&manager,
				(void *)&manager,
				"transactionManager_defaultIdFunction") < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(transactionManager_addHandlerByName(&manager,
				afalse,
				"default",
				"transactionManager_defaultExecFunction",
				(void *)&manager) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(server_init(&server, &log) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(server_setThreadPoolOptions(&server,
				atrue,
				BASIC_FUNCTIONALITY_MIN_THREADS,
				BASIC_FUNCTIONALITY_MAX_THREADS) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(server_setTransactionManager(&server, &manager) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(server_setSocketOptions(&server,
				BASIC_FUNCTIONALITY_HOSTNAME,
				BASIC_FUNCTIONALITY_PORT,
				SOCKET_PROTOCOL_TCPIP) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(server_setDumpPacketsOptions(&server,
				afalse,					// use individual files
				atrue,					// use packet headers
				"assets/data/test",		// packet dump path
				"server.packet"			// packet filename
				) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(server_setDumpPacketsMode(&server, atrue) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(server_start(&server) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	for(ii = 0; ii < BASIC_FUNCTIONALITY_TESTS; ii++) {
		executeClientTransaction();
	}

	if((serverStatus = server_getStatus(&server)) == NULL) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	while((entry = arraylist_pop(serverStatus)) != NULL) {
		printf("[unit]\t\t {SERVER-STATUS} '%s' = '%s'\n",
				entry->name, entry->value);

/*		printf("[unit]\t\t {SERVER-STATUS} [%30s] '%36s' = '%s' (%s)\n",
				entry->key,
				entry->name,
				entry->value,
				entry->description);*/

/*		printf(""
				"\t\t\t<tr>\n"
				"\t\t\t\t<td>%s</td>\n"
				"\t\t\t\t<td>%s</td>\n"
				"\t\t\t\t<td>%s</td>\n"
				"\t\t\t\t<td>%s</td>\n"
				"\t\t\t</tr>\n",
				entry->key,
				entry->name,
				entry->value,
				entry->description);*/

		free(entry->key);
		free(entry->name);
		free(entry->value);
		free(entry->description);
		free(entry);
	}

	arraylist_free(serverStatus);
	free(serverStatus);

	if(server_stop(&server) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(server_free(&server) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	log_free(&log);

	if(transactionManager_free(&manager) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	return 0;
}

