/*
 * transaction_manager.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard core library system for desginating and executing transactions
 * through message-recognition in order to execute handler functions designated
 * directly or through symbol tables, header file.
 *
 * Written by Josh English.
 */

#if !defined(_CORE_LIBRARY_NETWORK_TRANSACTION_MANAGER_H)

#define _CORE_LIBRARY_NETWORK_TRANSACTION_MANAGER_H

#if !defined(_CORE_H) && !defined(_CORE_LIBRARY_H) && \
		!defined(_CORE_LIBRARY_NETWORK_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _CORE_H || _CORE_LIBRARY_H || _CORE_LIBRARY_NETWORK_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// define transaction manager public constants

typedef enum _TransactionErrors {
	TRANSACTION_ERROR_INVALID_ARGS = -1,
	TRANSACTION_ERROR_READ = -2,
	TRANSACTION_ERROR_FAILED_IDENT = -3,
	TRANSACTION_ERROR_MISSING_HANDLER = -4,
	TRANSACTION_ERROR_INVALID_HANDLER = -5,
	TRANSACTION_ERROR_NULL_RESULT = -6,
	TRANSACTION_ERROR_INVALID_RESULT = -7,
	TRANSACTION_ERROR_WRITE = -8,
	TRANSACTION_ERROR_CLOSE = -9,
	TRANSACTION_ERROR_TIMEOUT = -10,
	TRANSACTION_ERROR_TOTAL = 10,
	TRANSACTION_ERROR_LENGTH = 11,
} TransactionErrors;


// define transaction manager public data types

typedef void (*TransactionManagerPacketDumpFunction)(void *context,
		char *packet, int packgetLength);

typedef void *(*TransactionManagerReceiveFunction)(void *manager,
		void *context, Socket *socket, int sd, int *bytesReceived,
		int *errorCode);

typedef int (*TransactionManagerSendFunction)(void *manager, void *context,
		Socket *socket, int sd, void *response);

typedef char *(*TransactionManagerIdFunction)(void *context, void *request,
		int *messageIdLength);

typedef void *(*TransactionManagerExecFunction)(void *context, void *request,
		int *memorySizeOfResponse);

typedef char *(*TransactionManagerHashFunction)(void *context, void *request,
		int *hashLength);

typedef void *(*TransactionManagerCloneFunction)(void *context, void *response,
		int *memorySizeOfResponse);

typedef struct _TransactionMetrics {
	alint transactionsExecuted;
	double totalTransactionTime;
	Mutex mutex;
} TransactionMetrics;

typedef struct _TransactionHandler {
	aboolean isSocketLeftOpen;
	char *messageId;
	char *functionName;
	TransactionMetrics metrics;
	TransactionManagerExecFunction function;
	void *context;
} TransactionHandler;

typedef struct _TransactionTable {
	char *filename;
	void *handle;
} TransactionTable;

typedef struct _TransactionErrorMetrics {
	alint counter[TRANSACTION_ERROR_LENGTH];
	Mutex mutex;
} TransactionErrorMetrics;

typedef struct _TransactionManager {
	aboolean isCacheEnabled;
	aboolean isLogInternallyAllocated;
	int tableLength;
	TransactionTable *tables;
	TransactionErrorMetrics errors;
	TransactionManagerPacketDumpFunction packetDumpFunction;
	TransactionManagerReceiveFunction receiveFunction;
	TransactionManagerSendFunction sendFunction;
	TransactionManagerIdFunction idFunction;
	TransactionManagerHashFunction hashFunction;
	TransactionManagerCloneFunction cloneFunction;
	ManagedIndexFreeFunction recvFreeFunction;
	ManagedIndexFreeFunction sendFreeFunction;
	Bptree index;
	ManagedIndex cache;
	Spinlock lock;
	Log *log;
	void *packetDumpContext;
	void *receiveContext;
	void *sendContext;
	void *idContext;
	void *hashContext;
	void *cloneContext;
} TransactionManager;


// declare transaction manager public functions

int transactionManager_init(TransactionManager *manager, Log *log);

int transactionManager_free(TransactionManager *manager);

char *transactionManager_getErrorString(TransactionErrors errorCode);

int transactionManager_enableCache(TransactionManager *manager,
		ManagedIndexFreeFunction freeFunction,
		void *hashContext, TransactionManagerHashFunction hashFunction,
		void *cloneContext, TransactionManagerCloneFunction cloneFunction);

int transactionManager_disableCache(TransactionManager *manager);

ManagedIndex *transactionManager_getCacheIndex(TransactionManager *manager);

Log *transactionManager_getLog(TransactionManager *manager);

int transactionManager_bindSymbolTable(TransactionManager *manager,
		char *filename);

int transactionManager_setPacketDumpFunction(TransactionManager *manager,
		void *context, TransactionManagerPacketDumpFunction packetDumpFunction);

int transactionManager_setPacketDumpFunctionByName(TransactionManager *manager,
		void *context, char *functionName);

int transactionManager_setReceiveFunction(TransactionManager *manager,
		void *context, TransactionManagerReceiveFunction receiveFunction);

int transactionManager_setReceiveFunctionByName(TransactionManager *manager,
		void *context, char *functionName);

int transactionManager_setSendFunction(TransactionManager *manager,
		void *context, TransactionManagerSendFunction sendFunction);

int transactionManager_setSendFunctionByName(TransactionManager *manager,
		void *context, char *functionName);

int transactionManager_setIdFunction(TransactionManager *manager,
		void *context, TransactionManagerIdFunction idFunction);

int transactionManager_setIdFunctionByName(TransactionManager *manager,
		void *context, char *functionName);

int transactionManager_setRecvFreeFunction(TransactionManager *manager,
		ManagedIndexFreeFunction freeFunction);

int transactionManager_setRecvFreeFunctionByName(TransactionManager *manager,
		char *functionName);

int transactionManager_setSendFreeFunction(TransactionManager *manager,
		ManagedIndexFreeFunction freeFunction);

int transactionManager_setSendFreeFunctionByName(TransactionManager *manager,
		char *functionName);

int transactionManager_addHandler(TransactionManager *manager,
		aboolean isSocketLeftOpen, char *messageId, char *functionName,
		TransactionManagerExecFunction function, void *context);

int transactionManager_addHandlerByName(TransactionManager *manager,
		aboolean isSocketLeftOpen, char *messageId, char *functionName,
		void *context);

int transactionManager_getMetrics(TransactionManager *manager,
		alint *transactionsExecuted, double *totalTransactionTime);

char *transactionManager_execIdFunction(TransactionManager *manager,
		void *request, int *messageIdLength);

void *transactionManager_execFunction(TransactionManager *manager,
		void *request);

int transactionManager_execute(TransactionManager *manager, Socket *socket,
		int sd, aboolean *isSocketLeftOpen);


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _CORE_LIBRARY_NETWORK_TRANSACTION_MANAGER_H

