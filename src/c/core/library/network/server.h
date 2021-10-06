/*
 * server.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The multithreaded server API to provide a framework for managed network
 * server transactions, header file.
 *
 * Written by Josh English.
 */

#if !defined(_CORE_LIBRARY_NETWORK_SERVER_H)

#define _CORE_LIBRARY_NETWORK_SERVER_H

#if !defined(_CORE_H) && !defined(_CORE_LIBRARY_H) && \
		!defined(_CORE_LIBRARY_NETWORK_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _CORE_H || _CORE_LIBRARY_H || _CORE_LIBRARY_NETWORK_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// define server public constants

typedef enum _ServerStates {
	SERVER_STATE_INIT = 1,
	SERVER_STATE_RUNNING,
	SERVER_STATE_SHUTDOWN,
	SERVER_STATE_STOPPED,
	SERVER_STATE_ERROR = -1
} ServerStates;

typedef enum _ServerErrors {
	SERVER_ERROR_TOTAL = 0,
	SERVER_ERROR_SOCKET_ACCEPT,
	SERVER_ERROR_SOCKET_READ,
	SERVER_ERROR_SOCKET_WRITE,
	SERVER_ERROR_SOCKET_CLOSE,
	SERVER_ERROR_SOCKET_TIMEOUT,
	SERVER_ERROR_TRANSACTION,
	SERVER_ERROR_LENGTH,
	SERVER_ERROR_UNKNOWN = -1
} ServerErrors;

typedef enum _ServerMetrics {
	SERVER_METRIC_TOTAL = 0,
	SERVER_METRIC_TRANSACTION,
	SERVER_METRIC_LENGTH,
	SERVER_METRIC_UNKNOWN = -1
} ServerMetrics;

#define SERVER_TRACKING_ENTRY_LENGTH						30


// define server data types

typedef struct _ServerError {
	alint counter[SERVER_ERROR_LENGTH];
	Mutex mutex;
} ServerError;

typedef struct _ServerMetric {
	alint transactionCounter;
	alint counter[SERVER_METRIC_LENGTH];
	double timings[SERVER_METRIC_LENGTH];
	Mutex mutex;
} ServerMetric;

typedef struct _ServerTracking {
	double timing;
	double timingAverage;
	double timings[SERVER_TRACKING_ENTRY_LENGTH];
	double counter;
	double counts[SERVER_TRACKING_ENTRY_LENGTH];
	double countAverage;
	Mutex mutex;
} ServerTracking;

typedef struct _ServerThread {
	int state;
	int status;
	int signal;
	int id;
	char *ip;
} ServerThread;

typedef struct _ServerThreadList {
	aboolean isThreadPoolShrinkEnabled;
	int minThreadPoolLength;
	int maxThreadPoolLength;
	int currentThreadPoolLength;
	ServerThread *list;
	Mutex mutex;
} ServerThreadList;

typedef struct _ServerPacket {
	aboolean isEnabled;
	aboolean useIndividualFiles;
	aboolean usePacketHeaders;
	alint uid;
	char *path;
	char *filename;
	FileHandle handle;
	Mutex mutex;
} ServerPacket;

typedef struct _ServerStatus {
	char *key;
	char *name;
	char *value;
	char *description;
} ServerStatus;

typedef struct _Server {
	aboolean isSocketInit;
	aboolean isLogInternallyAllocated;
	int state;
	int managerThreadId;
	double startTimestamp;
	char *name;
	ServerError errors;
	ServerMetric metrics;
	ServerTracking tracking;
	ServerThreadList threads;
	ServerPacket packets;
	TransactionManager *manager;
	TmfContext tmf;
	Socket socket;
	Mutex mutex;
	Log *log;
	void *memory;
} Server;


// declare server public functions

int server_init(Server *server, Log *log);

int server_free(Server *server);

char *server_getServerStateString(ServerStates state);

char *server_getServerErrorString(ServerErrors error);

char *server_getServerMetricString(ServerMetrics metric);

char *server_getName(Server *server);

int server_setName(Server *server, char *name);

int server_setThreadPoolOptions(Server *server,
		aboolean isThreadPoolShrinkEnabled, int minThreadPoolLength,
		int maxThreadPoolLength);

int server_setDumpPacketsMode(Server *server, aboolean isEnabled);

aboolean server_isDumpPackets(Server *server);

int server_setDumpPacketsOptions(Server *server, aboolean useIndividualFiles,
		aboolean usePacketHeaders, char *path, char *filename);

int server_setTransactionManager(Server *server, TransactionManager *manager);

char *server_getSocketHostname(Server *server);

int server_getSocketPort(Server *server);

SocketProtocol server_getSocketProtocol(Server *server);

int server_setSocketOptions(Server *server, char *hostname, int port,
		SocketProtocol protocol);

Socket *server_getSocket(Server *server);

Log *server_getLog(Server *server);

int server_start(Server *server);

int server_stop(Server *server);

ArrayList *server_getStatus(Server *server);

void server_dumpPacket(void *context, char *packet, int packetLength);


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _CORE_LIBRARY_NETWORK_SERVER_H

