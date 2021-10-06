/*
 * socket.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard socket interface, header file.
 *
 * Written by Josh English.
 */

#if !defined(_CORE_SYSTEM_SOCKET_H)

#define _CORE_SYSTEM_SOCKET_H

#if !defined(_CORE_H) && !defined(_CORE_SYSTEM_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _CORE_H || _CORE_SYSTEM_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// define socket public constants

typedef enum _SocketState {
	SOCKET_STATE_INIT = 1,
	SOCKET_STATE_OPEN,
	SOCKET_STATE_CLOSED,
	SOCKET_STATE_ERROR = -1
} SocketState;

typedef enum _SocketMode {
	SOCKET_MODE_CLIENT = 1,
	SOCKET_MODE_SERVER,
	SOCKET_MODE_ERROR = -1
} SocketMode;

typedef enum _SocketProtocol {
	SOCKET_PROTOCOL_TCPIP = 1,
	SOCKET_PROTOCOL_UDP,
	SOCKET_PROTOCOL_ERROR = -1
} SocketProtocol;

#define SOCKET_BROADCAST_MASK						"255.255.255.255"


// define socket defaults

#define SOCKET_DEFAULT_REUSE_ADDRESS				atrue

#define SOCKET_DEFAULT_REUSE_PORT					atrue

#define SOCKET_DEFAULT_USE_BROADCAST				afalse

#define SOCKET_DEFAULT_USE_SOCKET_WRITE_SYNC		afalse

#define SOCKET_DEFAULT_USE_MODULATED_TIMEOUT		afalse

#define SOCKET_DEFAULT_QUEUE_LENGTH					1024

#define SOCKET_DEFAULT_CHUNK_LENGTH					8192

#define SOCKET_DEFAULT_TIMEOUT_MILLIS				1024

#define SOCKET_DEFAULT_MAX_RECEIVE_LENGTH			16777216


// define socket public data types

typedef struct _SocketConfig {
	aboolean reuseAddress;
	aboolean reusePort;
	aboolean useBroadcast;
	aboolean useSocketWriteSync;
	aboolean useModulatedTimeout;
	aboolean shutdownRequested;
	int queueLength;
	int timeoutMillis;
	Mutex mutex;
} SocketConfig;

typedef struct _SocketMetrics {
	alint bytesSent;
	alint bytesReceived;
	alint readsAttempted;
	alint readsCompleted;
	alint writesAttempted;
	alint writesCompleted;
	double totalReadTime;
	double totalWriteTime;
	Mutex mutex;
} SocketMetrics;

typedef struct _Socket {
	int port;
	char *hostname;
	void *plugin;
	SocketState state;
	SocketMode mode;
	SocketProtocol protocol;
	SocketConfig config;
	SocketMetrics metrics;
	Mutex mutex;
} Socket;


// delcare socket public functions

int socket_init(Socket *socket, SocketMode mode, SocketProtocol protocol,
		char *hostname, int port);

void socket_free(Socket *socket);

char *socket_socketStateToString(SocketState state);

char *socket_socketModeToString(SocketMode mode);

char *socket_socketProtocolToString(SocketProtocol protocol);

SocketMode socket_getMode(Socket *socket);

int socket_setMode(Socket *socket, SocketMode mode);

SocketProtocol socket_getProtocol(Socket *socket);

int socket_setProtocol(Socket *socket, SocketProtocol protocol);

aboolean socket_getReuseAddress(Socket *socket);

int socket_setReuseAddress(Socket *socket, aboolean value);

aboolean socket_getReusePort(Socket *socket);

int socket_setReusePort(Socket *socket, aboolean value);

aboolean socket_getUseBroadcast(Socket *socket);

int socket_setUseBroadcast(Socket *socket, aboolean value);

aboolean socket_getUseSocketWriteSync(Socket *socket);

int socket_setUseSocketWriteSync(Socket *socket, aboolean value);

aboolean socket_getUseModulatedTimeout(Socket *socket);

int socket_setUseModulatedTimeout(Socket *socket, aboolean value);

int socket_getQueueLength(Socket *socket);

int socket_setQueueLength(Socket *socket, int value);

int socket_getTimeoutMillis(Socket *socket);

int socket_setTimeoutMillis(Socket *socket, int value);

alint socket_getBytesSent(Socket *socket);

alint socket_getBytesReceived(Socket *socket);

alint socket_getReadsAttempted(Socket *socket);

alint socket_getReadsCompleted(Socket *socket);

alint socket_getWritesAttempted(Socket *socket);

alint socket_getWritesCompleted(Socket *socket);

double socket_getTotalReadTime(Socket *socket);

double socket_getTotalWriteTime(Socket *socket);

int socket_getSocketDescriptor(Socket *socket);

int socket_open(Socket *socket);

int socket_close(Socket *socket);

int socket_closeChildSocket(Socket *socket, int sd);

int socket_requestShutdown(Socket *socket);

int socket_listen(Socket *socket);

int socket_accept(Socket *socket, int *sd, int microseconds);

int socket_getPeerName(Socket *socket, int sd, char **peerName);

int socket_send(Socket *socket, int sd, char *value, int valueLength);

int socket_receive(Socket *socket, int sd, char *value, int valueLength,
		double timeoutSeconds);


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _CORE_SYSTEM_SOCKET_H

