/*
 * protocol_http_unitTest.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard core networking library HTTP protcol functions, unit test.
 *
 * Written by Josh English.
 */

// preprocessor directives

#define _CORE_COMMON_BYPASS_STD_STREAM
#include "core/core.h"


// define networking library HTTP protocol unit test constants

#define BASIC_FUNCTIONALITY_VERBOSE_MODE				afalse

#define BASIC_FUNCTIONALITY_TESTS						1024

#define REMOTE_SERVER_CLIENT_TEST_VERBOSE_MODE			atrue


// declare networking library HTTP protocol unit test private functions

static int http_protocol_basicFunctionalityTest();

static int http_protocol_remoteServerClientTest();


// main function

int main()
{
	double timer = 0.0;

	signal_registerDefault();

	printf("[unit] networking library HTTP protocol unit test "
			"(Asgard Ver %s on %s)\n", ASGARD_VERSION, ASGARD_DATE);

	timer = time_getTimeMus();

	if(http_protocol_basicFunctionalityTest() < 0) {
		printf("[unit]\t ...FAILED\n");
		return 1;
	}

	printf("[unit]\t ...PASSED (%0.6f seconds)\n",
			time_getElapsedMusInSeconds(timer));
	printf("[unit]\n");

	timer = time_getTimeMus();

	if(http_protocol_remoteServerClientTest() < 0) {
		printf("[unit]\t ...FAILED\n");
		return 1;
	}

	printf("[unit]\t ...PASSED (%0.6f seconds)\n",
			time_getElapsedMusInSeconds(timer));
	printf("[unit] ...PASSED\n");

	return 0;
}


// define networking library HTTP protocol unit test private functions

static char *getFilenameFromHeader(char *buffer, int length, int offset)
{
	int ii = 0;
	int nn = 0;
	char *result = NULL;

	result = (char *)malloc(sizeof(char) * length);

	for(ii = offset, nn = 0; ii < length; ii++, nn++) {
		if(((unsigned int)buffer[ii] < 32) ||
				((unsigned int)buffer[ii] > 126)) {
			break;
		}
		result[nn] = buffer[ii];
	}

	return result;
}

static int http_protocol_basicFunctionalityTest()
{
	int ii = 0;
	int clientSd = 0;
	int serverSd = 0;
	int serverChildSd = 0;
	int valueLength = 0;
	char *ptr = NULL;
	char *value = NULL;
	char *peerName = NULL;
	char *filename = NULL;

	const char *REQUEST_TEMPLATE = ""
		"GET / HTTP/1.1\x0d\x0a"
		"Host: www.google.com\x0d\x0a"
		"User-Agent: HTTP Unit Test/%s (%s; en-US) Asgard/%s\x0d\x0a"
		"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\x0d\x0a"
		"Accept-Language: en-us,en;q=0.5\x0d\x0a"
		"Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7\x0d\x0a"
		"Keep-Alive: 115\x0d\x0a"
		"Connection: keep-alive\x0d\x0a"
		"Cache-Control: max-age=0\x0d\x0a"
		"Content-Type: text/html\x0d\x0a"
		"Content-Length: 71\x0d\x0a"
		"\x0d\x0a"
		"<html>\n"
		"\t<body>\n"
		"\t\t<p>Hello World, from the Client!</p>\n"
		"\t</body>\n"
		"</html>\n";

	const char *RESPONSE_TEMPLATE = ""
		"HTTP/1.1 200 Ok\x0d\x0a"
		"Content-Type: text/html\x0d\x0a"
		"Content-Length: 71\x0d\x0a"
		"Accept-Ranges: bytes, kilobytes, megabytes\x0d\x0a"
		"Date: Fri, 15 Apr 2011 11:14:09 GMT\x0d\x0a"
		"Server: Asgard HTTP Protocol Unit Test Server Ver %s\x0d\x0a"
		"\x0d\x0a"
		"<html>\n"
		"\t<body>\n"
		"\t\t<p>Hello World, from the Server!</p>\n"
		"\t</body>\n"
		"</html>\n";

	Socket clientSocket;
	Socket serverSocket;

	HttpProtocol protocol;

	printf("[unit]\t HTTP protocol basic functionality test...\n");

	// initialize HTTP protocol

	http_protocol_init(&protocol,
			atrue,			// isTempFileMode
			8,				// tempFileThresholdBytes
			65536,			// tempFileSpaceAllowedBytes
			0,				// tempFileSpaceInUseBytes
			"temp"			// tempFileBasePath
			);

	// create server socket

	if(socket_init(&serverSocket,
				SOCKET_MODE_SERVER,
				SOCKET_PROTOCOL_TCPIP,
				"127.0.0.1",
				6543) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(socket_open(&serverSocket) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if((serverSd = socket_getSocketDescriptor(&serverSocket)) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(BASIC_FUNCTIONALITY_VERBOSE_MODE) {
		printf("[unit]\t\t ...server socket descriptor: %i\n", serverSd);
	}

	if(socket_getPeerName(&serverSocket, serverSd, &peerName) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(BASIC_FUNCTIONALITY_VERBOSE_MODE) {
		printf("[unit]\t\t ...server socket peername: '%s'\n", peerName);
	}

	free(peerName);

	if(socket_listen(&serverSocket) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	// create client socket

	if(socket_init(&clientSocket,
				SOCKET_MODE_CLIENT,
				SOCKET_PROTOCOL_TCPIP,
				"127.0.0.1",
				6543
				) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	for(ii = 0; ii < BASIC_FUNCTIONALITY_TESTS; ii++) {
		// connect the client socket to the server

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

		free(value);

		if(BASIC_FUNCTIONALITY_VERBOSE_MODE) {
			printf("[unit]\t\t ...client socket sent %i bytes\n", valueLength);
		}

		// accept client socket transmission

		if(socket_accept(&serverSocket, &serverChildSd, 2097152) < 0) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		if(BASIC_FUNCTIONALITY_VERBOSE_MODE) {
			printf("[unit]\t\t ...server socket accepted socket #%i\n",
					serverChildSd);
		}

		// read client socket transmission

		protocol.isTempFileMode = atrue;

		value = http_protocol_receive(&protocol, &serverSocket, serverChildSd,
				&valueLength);

		if(BASIC_FUNCTIONALITY_VERBOSE_MODE) {
			printf("\n\n\n"
					"-----/client message begin/-----\n"
					"%s\n"
					"-----/client message end/------\n"
					"[unit]\t\t ...server socket received %i bytes\n",
					value, valueLength);
		}

		if((ptr = strncasestr(value, valueLength,
						"Content-in-file:", 16)) != NULL) {
			filename = getFilenameFromHeader(value, valueLength,
					(int)((ptr - value) + 17));

			if(http_protocol_freeTempFile(&protocol, filename) < 0) {
				printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
						__LINE__);
				return -1;
			}

			free(filename);
		}

		free(value);

		// send message back to the client

		valueLength = 8192;
		value = (char *)malloc(sizeof(char) * valueLength);

		valueLength = snprintf(value, (valueLength - 1), RESPONSE_TEMPLATE,
				ASGARD_VERSION);

		if(socket_send(&serverSocket, serverChildSd, value, valueLength) < 0) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		free(value);

		if(BASIC_FUNCTIONALITY_VERBOSE_MODE) {
			printf("[unit]\t\t ...server socket sent %i bytes\n", valueLength);
		}

		if(socket_closeChildSocket(&serverSocket, serverChildSd) < 0) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		// read server socket transmission

		protocol.isTempFileMode = afalse;

		value = http_protocol_receive(&protocol, &clientSocket, clientSd,
				&valueLength);

		if(BASIC_FUNCTIONALITY_VERBOSE_MODE) {
			printf("\n\n\n"
					"-----/server message begin/-----\n"
					"%s\n"
					"-----/server message end/------\n"
					"[unit]\t\t ...server socket received %i bytes\n",
					value, valueLength);
		}

		free(value);

		if(socket_close(&clientSocket) < 0) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}
	}

	printf("[unit]\t ...completed %i HTTP protocol basic functionality tests\n",
			BASIC_FUNCTIONALITY_TESTS);

	// display client & server socket states

	printf("[unit]\t {CLIENT} => mode                   : [%i]\n",
			(int)socket_getMode(&clientSocket));
	printf("[unit]\t {CLIENT} => protocol               : [%i]\n",
			(int)socket_getProtocol(&clientSocket));
	printf("[unit]\t {CLIENT} => reuse address          : %i\n",
			(int)socket_getReuseAddress(&clientSocket));
	printf("[unit]\t {CLIENT} => reuse port             : %i\n",
			(int)socket_getReusePort(&clientSocket));
	printf("[unit]\t {CLIENT} => use broadcast          : %i\n",
			(int)socket_getUseBroadcast(&clientSocket));
	printf("[unit]\t {CLIENT} => use write sync         : %i\n",
			(int)socket_getUseSocketWriteSync(&clientSocket));
	printf("[unit]\t {CLIENT} => use modulated timeout  : %i\n",
			(int)socket_getUseModulatedTimeout(&clientSocket));
	printf("[unit]\t {CLIENT} => queue length           : %i\n",
			socket_getQueueLength(&clientSocket));
	printf("[unit]\t {CLIENT} => timeout millis         : %i\n",
			socket_getTimeoutMillis(&clientSocket));
	printf("[unit]\t {CLIENT} => bytes sent             : %0.0f\n",
			(double)socket_getBytesSent(&clientSocket));
	printf("[unit]\t {CLIENT} => bytes received         : %0.0f\n",
			(double)socket_getBytesReceived(&clientSocket));
	printf("[unit]\t {CLIENT} => reads                  : %0.0f (%0.2f %%)\n",
			(double)socket_getReadsAttempted(&clientSocket),
			(((double)socket_getReadsCompleted(&clientSocket) /
			  (double)socket_getReadsAttempted(&clientSocket)) * 100.0));
	printf("[unit]\t {CLIENT} => writes                 : %0.0f (%0.2f %%)\n",
			(double)socket_getWritesAttempted(&clientSocket),
			(((double)socket_getWritesCompleted(&clientSocket) /
			  (double)socket_getWritesAttempted(&clientSocket)) * 100.0));
	printf("[unit]\t {CLIENT} => total read time        : %0.6f seconds\n",
			(double)socket_getTotalReadTime(&clientSocket));
	printf("[unit]\t {CLIENT} => total write time       : %0.6f seconds\n",
			(double)socket_getTotalWriteTime(&clientSocket));

	printf("[unit]\t {SERVER} => mode                   : [%i]\n",
			(int)socket_getMode(&serverSocket));
	printf("[unit]\t {SERVER} => protocol               : [%i]\n",
			(int)socket_getProtocol(&serverSocket));
	printf("[unit]\t {SERVER} => reuse address          : %i\n",
			(int)socket_getReuseAddress(&serverSocket));
	printf("[unit]\t {SERVER} => reuse port             : %i\n",
			(int)socket_getReusePort(&serverSocket));
	printf("[unit]\t {SERVER} => use broadcast          : %i\n",
			(int)socket_getUseBroadcast(&serverSocket));
	printf("[unit]\t {SERVER} => use write sync         : %i\n",
			(int)socket_getUseSocketWriteSync(&serverSocket));
	printf("[unit]\t {SERVER} => use modulated timeout  : %i\n",
			(int)socket_getUseModulatedTimeout(&serverSocket));
	printf("[unit]\t {SERVER} => queue length           : %i\n",
			socket_getQueueLength(&serverSocket));
	printf("[unit]\t {SERVER} => timeout millis         : %i\n",
			socket_getTimeoutMillis(&serverSocket));
	printf("[unit]\t {SERVER} => bytes sent             : %0.0f\n",
			(double)socket_getBytesSent(&serverSocket));
	printf("[unit]\t {SERVER} => bytes received         : %0.0f\n",
			(double)socket_getBytesReceived(&serverSocket));
	printf("[unit]\t {SERVER} => reads                  : %0.0f (%0.2f %%)\n",
			(double)socket_getReadsAttempted(&serverSocket),
			(((double)socket_getReadsCompleted(&serverSocket) /
			  (double)socket_getReadsAttempted(&serverSocket)) * 100.0));
	printf("[unit]\t {SERVER} => writes                 : %0.0f (%0.2f %%)\n",
			(double)socket_getWritesAttempted(&serverSocket),
			(((double)socket_getWritesCompleted(&serverSocket) /
			  (double)socket_getWritesAttempted(&serverSocket)) * 100.0));
	printf("[unit]\t {SERVER} => total read time        : %0.6f seconds\n",
			(double)socket_getTotalReadTime(&serverSocket));
	printf("[unit]\t {SERVER} => total write time       : %0.6f seconds\n",
			(double)socket_getTotalWriteTime(&serverSocket));

	// free the sockets

	if(socket_close(&serverSocket) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	socket_free(&clientSocket);
	socket_free(&serverSocket);

	// free the protocol

	http_protocol_free(&protocol);

	return 0;
}

static int http_protocol_remoteServerClientTest()
{
	int sd = 0;
	int valueLength = 0;
	char *value = NULL;
	char *peerName = NULL;

	const char *REQUEST_TEMPLATE = ""
		"GET / HTTP/1.1\x0d\x0a"
		"Host: www.google.com\x0d\x0a"
		"User-Agent: HTTP Unit Test/%s (%s; en-US) Asgard/%s\x0d\x0a"
		"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\x0d\x0a"
		"Accept-Language: en-us,en;q=0.5\x0d\x0a"
		"Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7\x0d\x0a"
		"Keep-Alive: 115\x0d\x0a"
		"Connection: keep-alive\x0d\x0a"
		"Cache-Control: max-age=0\x0d\x0a"
		"\x0d\x0a";

	Socket clientSocket;

	printf("[unit]\t HTTP protocol remote-server client test...\n");

	// create client socket

	if(socket_init(&clientSocket,
				SOCKET_MODE_CLIENT,
				SOCKET_PROTOCOL_TCPIP,
				"www.google.com",
				80
				) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	// connect the client socket to the server

	if(socket_open(&clientSocket) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if((sd = socket_getSocketDescriptor(&clientSocket)) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(REMOTE_SERVER_CLIENT_TEST_VERBOSE_MODE) {
		printf("[unit]\t\t ...client socket descriptor: %i\n", sd);
	}

	if(socket_getPeerName(&clientSocket, sd, &peerName) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(REMOTE_SERVER_CLIENT_TEST_VERBOSE_MODE) {
		printf("[unit]\t\t ...client socket peername: '%s'\n", peerName);
	}

	free(peerName);

	// send message from client to server

	valueLength = (strlen(REQUEST_TEMPLATE) + 128);
	value = (char *)malloc(sizeof(char) * valueLength);

	valueLength = snprintf(value, valueLength, REQUEST_TEMPLATE,
			ASGARD_VERSION, ASGARD_VERSION, ASGARD_DATE);

	if(REMOTE_SERVER_CLIENT_TEST_VERBOSE_MODE) {
		printf("\n\n\n"
				"-----/client message begin/-----\n"
				"%s\n"
				"-----/client message end/------\n",
				value);
	}

	if(socket_send(&clientSocket, sd, value, valueLength) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	free(value);

	if(REMOTE_SERVER_CLIENT_TEST_VERBOSE_MODE) {
		printf("[unit]\t\t ...client socket sent %i bytes\n", valueLength);
	}

	// read server socket transmission

	value = http_protocol_receive(NULL, &clientSocket, sd, &valueLength);

	if(REMOTE_SERVER_CLIENT_TEST_VERBOSE_MODE) {
		printf("\n\n\n"
				"-----/server message begin/-----\n"
				"%s\n"
				"-----/server message end/------\n"
				"[unit]\t\t ...server socket received %i bytes\n",
				value, valueLength);
	}

	free(value);

	if(socket_close(&clientSocket) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	// display client & server socket states

	printf("[unit]\t {CLIENT} => mode                   : [%i]\n",
			(int)socket_getMode(&clientSocket));
	printf("[unit]\t {CLIENT} => protocol               : [%i]\n",
			(int)socket_getProtocol(&clientSocket));
	printf("[unit]\t {CLIENT} => reuse address          : %i\n",
			(int)socket_getReuseAddress(&clientSocket));
	printf("[unit]\t {CLIENT} => reuse port             : %i\n",
			(int)socket_getReusePort(&clientSocket));
	printf("[unit]\t {CLIENT} => use broadcast          : %i\n",
			(int)socket_getUseBroadcast(&clientSocket));
	printf("[unit]\t {CLIENT} => use write sync         : %i\n",
			(int)socket_getUseSocketWriteSync(&clientSocket));
	printf("[unit]\t {CLIENT} => use modulated timeout  : %i\n",
			(int)socket_getUseModulatedTimeout(&clientSocket));
	printf("[unit]\t {CLIENT} => queue length           : %i\n",
			socket_getQueueLength(&clientSocket));
	printf("[unit]\t {CLIENT} => timeout millis         : %i\n",
			socket_getTimeoutMillis(&clientSocket));
	printf("[unit]\t {CLIENT} => bytes sent             : %0.0f\n",
			(double)socket_getBytesSent(&clientSocket));
	printf("[unit]\t {CLIENT} => bytes received         : %0.0f\n",
			(double)socket_getBytesReceived(&clientSocket));
	printf("[unit]\t {CLIENT} => reads                  : %0.0f (%0.2f %%)\n",
			(double)socket_getReadsAttempted(&clientSocket),
			(((double)socket_getReadsCompleted(&clientSocket) /
			  (double)socket_getReadsAttempted(&clientSocket)) * 100.0));
	printf("[unit]\t {CLIENT} => writes                 : %0.0f (%0.2f %%)\n",
			(double)socket_getWritesAttempted(&clientSocket),
			(((double)socket_getWritesCompleted(&clientSocket) /
			  (double)socket_getWritesAttempted(&clientSocket)) * 100.0));
	printf("[unit]\t {CLIENT} => total read time        : %0.6f seconds\n",
			(double)socket_getTotalReadTime(&clientSocket));
	printf("[unit]\t {CLIENT} => total write time       : %0.6f seconds\n",
			(double)socket_getTotalWriteTime(&clientSocket));

	// free the sockets

	socket_free(&clientSocket);

	return 0;
}

