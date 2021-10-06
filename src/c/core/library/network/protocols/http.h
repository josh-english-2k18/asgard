/*
 * http.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard core networking library HTTP protcol functions, header file.
 *
 * Written by Josh English.
 */

#if !defined(_CORE_LIBRARY_NETWORK_PROTOCOLS_FILE_H)

#define _CORE_LIBRARY_NETWORK_PROTOCOLS_FILE_H

#if !defined(_CORE_H) && !defined(_CORE_LIBRARY_H) && \
		!defined(_CORE_LIBRARY_NETWORK_PROTOCOLS_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _CORE_H || _CORE_LIBRARY_H || _CORE_LIBRARY_NETWORK_PROTOCOLS_COMPONENT


#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// define networking library HTTP protocol public datatypes

typedef struct _HttpProtocol {
	aboolean isTempFileMode;
	alint tempFileThresholdBytes;
	alint tempFileSpaceAllowedBytes;
	alint tempFileSpaceInUseBytes;
	char *tempFileBasePath;
	Mutex mutex;
} HttpProtocol;

typedef struct _HttpTransactionManagerReceive {
	aboolean isContentInFile;
	int payloadLength;
	char *ipAddress;
	char *contentFilename;
	char *payload;
} HttpTransactionManagerReceive;

typedef struct _HttpTransactionManagerSend {
	int payloadLength;
	char *payload;
} HttpTransactionManagerSend;


// delcare networking library HTTP protocol public functions

// protocol functions

void http_protocol_init(HttpProtocol *protocol, aboolean isTempFileMode,
		alint tempFileThresholdBytes, alint tempFileSpaceAllowedBytes,
		alint tempFileSpaceInUseBytes, char *tempFileBasePath);

void http_protocol_free(HttpProtocol *protocol);

int http_protocol_freeTempFile(HttpProtocol *protocol, char *filename);

int http_protocol_send(void *context, Socket *socket, int sd, char *value,
		int valueLength);

char *http_protocol_receive(void *context, Socket *socket, int sd,
		int *receiveLength);

// http protocol transaction managager functions

void *http_protocol_transactionManagerReceive(void *manager, void *context,
		Socket *socket, int sd, int *bytesReceived, int *errorCode);

void http_protocol_transactionManagerReceiveFree(void *argument, void *memory);

int http_protocol_transactionManagerSend(void *manager, void *context,
		Socket *socket, int sd, void *response);

void http_protocol_transactionManagerSendFree(void *argument, void *memory);

char *http_protocol_transactionManagerIdFunction(void *context, void *request,
		int *messageIdLength);


#if defined(__cplusplus)
};
#endif // __cplusplus


#endif // _CORE_LIBRARY_NETWORK_PROTOCOLS_FILE_H

