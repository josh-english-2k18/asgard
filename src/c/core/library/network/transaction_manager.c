/*
 * transaction_manager.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard core library system for desginating and executing transactions
 * through message-recognition in order to execute handler functions designated
 * directly or through symbol tables.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#define _CORE_LIBRARY_NETWORK_COMPONENT
#include "core/library/network/transaction_manager.h"


// define transaction manager private data types

typedef struct _DefaultRequestContext {
	int requestLength;
	char *request;
} DefaultRequestContext;

typedef struct _DefaultResponseContext {
	int responseLength;
	char *response;
} DefaultResponseContext;


// define transaction manager private functions

// general functions

static int recordError(TransactionManager *manager, int type);

// default transaction functions

void *transactionManager_defaultReceiveFunction(void *manager, void *context,
		Socket *socket, int sd, int *bytesReceived, int *errorCode);

int transactionManager_defaultSendFunction(void *manager, void *context,
		Socket *socket, int sd, void *response);

char *transactionManager_defaultIdFunction(void *context, void *request,
		int *messageIdLength);

DefaultResponseContext *transactionManager_defaultExecFunction(void *context,
		void *request, int *memorySizeOfResponse);

char *transactionManager_defaultHashFunction(void *context, void *request,
		int *hashLength);

void *transactionManager_defaultCloneFunction(void *context, void *response,
		int *memorySizeOfResponse);

// request functions

static void defaultRequest_free(void *argument, void *memory);

// response functions

static void defaultResponse_free(void *argument, void *memory);

// handler functions

static TransactionHandler *handler_new(aboolean isSocketLeftOpen,
		char *messageId, char *functionName,
		TransactionManagerExecFunction function, void *context);

static void handler_free(void *memory);


// define transaction manager private functions

// general functions

static int recordError(TransactionManager *manager, int type)
{
	int id = 0;

	if((type < TRANSACTION_ERROR_TIMEOUT) ||
			(type > TRANSACTION_ERROR_INVALID_ARGS)) {
		fprintf(stderr, "[%s():%i] error - unable to record error for type "
				"%i.\n", __FUNCTION__, __LINE__, type);
		return TRANSACTION_ERROR_INVALID_ARGS;
	}

	mutex_lock(&manager->errors.mutex);

	id = ((type * -1) - 1);

	manager->errors.counter[id] += 1;
	manager->errors.counter[TRANSACTION_ERROR_TOTAL] += 1;

	mutex_unlock(&manager->errors.mutex);

	return type;
}

// default transaction functions

void *transactionManager_defaultReceiveFunction(void *manager, void *context,
		Socket *socket, int sd, int *bytesReceived, int *errorCode)
{
	aboolean firstTime = atrue;
	int rc = 0;
	int receiveMessageRef = 0;
	int receiveMessageLength = 0;
	double timer = 0.0;
	double elapsedTime = 0.0;
	double remainingTime = 0.0;
	char chunk[4];
	char *receiveMessage = NULL;

	TransactionManager *tm = NULL;
	DefaultRequestContext *result = NULL;

	if((manager == NULL) || (context == NULL) || (socket == NULL) ||
			(sd < 1) || (bytesReceived == NULL) || (errorCode == NULL)) {
		DISPLAY_INVALID_ARGS;

		if(bytesReceived != NULL) {
			*bytesReceived = 0;
		}
		if(errorCode != NULL) {
			*errorCode = -1;
		}

		return NULL;
	}

	tm = (TransactionManager *)manager;

	*bytesReceived = 0;
	*errorCode = 0;

	// read server socket transmission

	receiveMessageRef = 0;
	receiveMessageLength = 1024;
	receiveMessage = (char *)malloc(sizeof(char) * receiveMessageLength);

	timer = 0.0;
	elapsedTime = 0.0;
	remainingTime = 2.048;

	do {
		timer = time_getTimeMus();

		memset(chunk, 0, (int)(sizeof(chunk)));

		if((rc = socket_receive(socket, sd, chunk, (int)(sizeof(chunk)),
						remainingTime)) < 0) {
			break;
		}

		elapsedTime = time_getElapsedMusInSeconds(timer);

		if(firstTime) {
			remainingTime = (elapsedTime * 1024);
			if(remainingTime > 2.048) {
				remainingTime = (2.048 - elapsedTime);
			}
			firstTime = afalse;
		}
		else {
			remainingTime -= elapsedTime;
		}

		if(rc > 0) {
			memcpy((receiveMessage + receiveMessageRef), chunk, rc);
			receiveMessageRef += rc;
		}

		if(receiveMessageRef >= receiveMessageLength) {
			receiveMessageLength *= 2;
			receiveMessage = (char *)realloc(receiveMessage,
					(sizeof(char) * receiveMessageLength));
		}
	} while(remainingTime > 0.0);

	*bytesReceived = receiveMessageRef;

	if(receiveMessageRef < 1) {
		if(rc < 0) {
			*errorCode = TRANSACTION_ERROR_READ;
		}
		else {
			*errorCode = TRANSACTION_ERROR_TIMEOUT;
		}

		free(receiveMessage);
	}
	else {
		receiveMessage[receiveMessageRef] = '\0';

		result = (DefaultRequestContext *)malloc(sizeof(DefaultRequestContext));
		result->requestLength = receiveMessageRef;
		result->request = receiveMessage;

		if(tm->packetDumpFunction != NULL) {
			tm->packetDumpFunction(tm->packetDumpContext, result->request,
					result->requestLength);
		}
	}

	return result;
}

int transactionManager_defaultSendFunction(void *manager, void *context,
		Socket *socket, int sd, void *response)
{
	int result = 0;

	TransactionManager *tm = NULL;
	DefaultResponseContext *responseContext = NULL;

	if((manager == NULL) || (context == NULL) || (socket == NULL) ||
			(sd < 1) || (response == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	tm = (TransactionManager *)manager;
	responseContext = (DefaultResponseContext *)response;

	if((responseContext->response != NULL) &&
			(responseContext->responseLength > 0)) {
		if(socket_send(socket, sd, responseContext->response,
					responseContext->responseLength) < 0) {
			result = TRANSACTION_ERROR_WRITE;
		}
		else {
			if(tm->packetDumpFunction != NULL) {
				tm->packetDumpFunction(tm->packetDumpContext,
						responseContext->response,
						responseContext->responseLength);
			}

			result = responseContext->responseLength;
		}
	}
	else {
		result = TRANSACTION_ERROR_INVALID_RESULT;
	}

	return result;
}

char *transactionManager_defaultIdFunction(void *context, void *request,
		int *messageIdLength)
{
	char *result = NULL;

	DefaultRequestContext *requestContext = NULL;

	if((context == NULL) || (request == NULL) || (messageIdLength == NULL)) {
		DISPLAY_INVALID_ARGS;

		if(messageIdLength != NULL) {
			*messageIdLength = 0;
		}

		return NULL;
	}

	requestContext = (DefaultRequestContext *)request;

	if((requestContext->request != NULL) &&
			(requestContext->requestLength > 0)) {
		result = strndup("default", 7);

		*messageIdLength = 7;
	}

	return result;
}

DefaultResponseContext *transactionManager_defaultExecFunction(void *context,
		void *request, int *memorySizeOfResponse)
{
	DefaultRequestContext *requestContext = NULL;
	DefaultResponseContext *result = NULL;

	if((context == NULL) || (request == NULL) ||
			(memorySizeOfResponse == NULL)) {
		DISPLAY_INVALID_ARGS;

		if(memorySizeOfResponse != NULL) {
			*memorySizeOfResponse = 0;
		}

		return NULL;
	}

	*memorySizeOfResponse = 0;

	requestContext = (DefaultRequestContext *)request;

	if((requestContext->request != NULL) &&
			(requestContext->requestLength > 0)) {
		result = (DefaultResponseContext *)malloc(
				sizeof(DefaultResponseContext));
		result->response = strndup(requestContext->request,
				requestContext->requestLength);
		result->responseLength = requestContext->requestLength;

		*memorySizeOfResponse = (((int)sizeof(DefaultResponseContext)) +
				result->responseLength +
				2); // accounts for standard memory allocation scheme
	}

	return result;
}

char *transactionManager_defaultHashFunction(void *context, void *request,
		int *hashLength)
{
	char *result = NULL;

	Sha1 hash;
	DefaultRequestContext *requestContext = NULL;

	if((context == NULL) || (request == NULL) || (hashLength == NULL)) {
		DISPLAY_INVALID_ARGS;

		if(hashLength != NULL) {
			*hashLength = 0;
		}

		return NULL;
	}

	*hashLength = 0;

	requestContext = (DefaultRequestContext *)request;

	if((requestContext->request != NULL) &&
			(requestContext->requestLength > 0)) {
		sha1_init(&hash);
		sha1_hashData(&hash,
				(unsigned char *)requestContext->request,
				requestContext->requestLength);
		sha1_calculateHash(&hash);

		sha1_constructSignature(&hash, &result, hashLength);
	}

	return result;
}

void *transactionManager_defaultCloneFunction(void *context, void *response,
		int *memorySizeOfResponse)
{
	DefaultResponseContext *result = NULL;
	DefaultResponseContext *responseContext = NULL;

	if((context == NULL) || (response == NULL) ||
			(memorySizeOfResponse == NULL)) {
		DISPLAY_INVALID_ARGS;

		if(memorySizeOfResponse != NULL) {
			*memorySizeOfResponse = 0;
		}

		return NULL;
	}

	*memorySizeOfResponse = 0;

	responseContext = (DefaultResponseContext *)response;

	if((responseContext->response != NULL) &&
			(responseContext->responseLength > 0)) {
		result = (DefaultResponseContext *)malloc(
				sizeof(DefaultResponseContext));
		result->response = strndup(responseContext->response,
				responseContext->responseLength);
		result->responseLength = responseContext->responseLength;

		*memorySizeOfResponse = (((int)sizeof(DefaultResponseContext)) +
				result->responseLength +
				2); // accounts for standard memory allocation scheme
	}

	return result;
}

// request functions

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

// response functions

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

// handler functions

static TransactionHandler *handler_new(aboolean isSocketLeftOpen,
		char *messageId, char *functionName,
		TransactionManagerExecFunction function, void *context)
{
	TransactionHandler *result = NULL;

	result = (TransactionHandler *)malloc(sizeof(TransactionHandler));

	result->isSocketLeftOpen = isSocketLeftOpen;
	result->messageId = strdup(messageId);
	result->functionName = strdup(functionName);
	result->function = function;
	result->context = context;

	mutex_init(&result->metrics.mutex);

	return result;
}

static void handler_free(void *memory)
{
	TransactionHandler *handler = NULL;

	if(memory == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	handler = (TransactionHandler *)memory;

	if(handler->messageId != NULL) {
		free(handler->messageId);
	}

	if(handler->functionName != NULL) {
		free(handler->functionName);
	}

	mutex_free(&handler->metrics.mutex);

	free(handler);
}


// define transaction manager public functions

int transactionManager_init(TransactionManager *manager, Log *log)
{
	if(manager == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	memset(manager, 0, (int)(sizeof(TransactionManager)));

	manager->isCacheEnabled = afalse;
	manager->tableLength = 0;
	manager->tables = NULL;
	manager->receiveFunction = transactionManager_defaultReceiveFunction;
	manager->sendFunction = transactionManager_defaultSendFunction;
	manager->idFunction = transactionManager_defaultIdFunction;
	manager->hashFunction = transactionManager_defaultHashFunction;
	manager->cloneFunction = transactionManager_defaultCloneFunction;
	manager->recvFreeFunction = defaultRequest_free;
	manager->sendFreeFunction = defaultResponse_free;
	manager->receiveContext = manager;
	manager->sendContext = manager;
	manager->idContext = manager;
	manager->hashContext = manager;
	manager->cloneContext = manager;

	mutex_init(&manager->errors.mutex);

	bptree_init(&manager->index);
	bptree_setFreeFunction(&manager->index, handler_free);

	spinlock_init(&manager->lock);

	if(log == NULL) {
		manager->isLogInternallyAllocated = atrue;
		manager->log = (Log *)malloc(sizeof(Log));

		log_init(manager->log, LOG_OUTPUT_STDOUT, NULL, LOG_LEVEL_INFO);
	}
	else {
		manager->isLogInternallyAllocated = afalse;
		manager->log = log;
	}

	return 0;
}

int transactionManager_free(TransactionManager *manager)
{
	int ii = 0;

	if(manager == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	spinlock_writeLock(&manager->lock);

	if(manager->tables != NULL) {
		for(ii = 0; ii < manager->tableLength; ii++) {
			if(manager->tables[ii].filename != NULL) {
				free(manager->tables[ii].filename);
			}

			if(manager->tables[ii].handle != NULL) {
				dl_close(manager->tables[ii].handle);
			}
		}

		free(manager->tables);
	}

	mutex_free(&manager->errors.mutex);

	bptree_free(&manager->index);

	if(manager->isCacheEnabled) {
		managedIndex_free(&manager->cache);
	}

	if(manager->isLogInternallyAllocated) {
		log_free(manager->log);
		free(manager->log);
	}

	spinlock_writeUnlock(&manager->lock);
	spinlock_free(&manager->lock);

	memset(manager, 0, (int)(sizeof(TransactionManager)));

	return 0;
}

char *transactionManager_getErrorString(TransactionErrors errorCode)
{
	char *result = NULL;

	switch(errorCode) {
		case TRANSACTION_ERROR_INVALID_ARGS:
			result = "Invalid Args";
			break;

		case TRANSACTION_ERROR_READ:
			result = "Read";
			break;

		case TRANSACTION_ERROR_FAILED_IDENT:
			result = "Identification";
			break;

		case TRANSACTION_ERROR_MISSING_HANDLER:
			result = "Missing Handler";
			break;

		case TRANSACTION_ERROR_INVALID_HANDLER:
			result = "Invalid Handler";
			break;

		case TRANSACTION_ERROR_NULL_RESULT:
			result = "Null Result";
			break;

		case TRANSACTION_ERROR_INVALID_RESULT:
			result = "Invalid Result";
			break;

		case TRANSACTION_ERROR_WRITE:
			result = "Write";
			break;

		case TRANSACTION_ERROR_CLOSE:
			result = "Close";
			break;

		case TRANSACTION_ERROR_TIMEOUT:
			result = "Timeout";
			break;

		case TRANSACTION_ERROR_TOTAL:
			result = "Total";
			break;

		default:
			result = "Unknown";
	}

	return result;
}

int transactionManager_enableCache(TransactionManager *manager,
		ManagedIndexFreeFunction freeFunction,
		void *hashContext, TransactionManagerHashFunction hashFunction,
		void *cloneContext, TransactionManagerCloneFunction cloneFunction)
{
	if((manager == NULL) || (hashFunction == NULL) || (cloneFunction == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	spinlock_writeLock(&manager->lock);

	managedIndex_init(&manager->cache, afalse);

	if(freeFunction != NULL) {
		managedIndex_setFreeFunction(&manager->cache, manager->sendContext,
				freeFunction);

		manager->sendFreeFunction = freeFunction;
	}

	manager->hashFunction = hashFunction;
	manager->hashContext = hashContext;

	manager->cloneFunction = cloneFunction;
	manager->cloneContext = cloneContext;

	manager->isCacheEnabled = atrue;

	log_logf(manager->log, LOG_LEVEL_INFO,
			"transaction manager cache enabled");

	spinlock_writeUnlock(&manager->lock);

	return 0;
}

int transactionManager_disableCache(TransactionManager *manager)
{
	if(manager == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	spinlock_writeLock(&manager->lock);

	if(manager->isCacheEnabled) {
		managedIndex_free(&manager->cache);

		manager->hashFunction = NULL;
		manager->hashContext = NULL;

		manager->cloneFunction = NULL;
		manager->cloneContext = NULL;

		manager->isCacheEnabled = afalse;
	}

	log_logf(manager->log, LOG_LEVEL_INFO,
			"transaction manager cache disabled");

	spinlock_writeUnlock(&manager->lock);

	return 0;
}

ManagedIndex *transactionManager_getCacheIndex(TransactionManager *manager)
{
	ManagedIndex *result = NULL;

	if(manager == NULL) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	spinlock_writeLock(&manager->lock);

	if(manager->isCacheEnabled) {
		result = &(manager->cache);
	}

	spinlock_writeUnlock(&manager->lock);

	return result;
}

Log *transactionManager_getLog(TransactionManager *manager)
{
	Log *result = NULL;

	if(manager == NULL) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	spinlock_writeLock(&manager->lock);

	result = manager->log;

	spinlock_writeUnlock(&manager->lock);

	return result;
}

int transactionManager_bindSymbolTable(TransactionManager *manager,
		char *filename)
{
	int ref = 0;
	int result = 0;
	void *handle = NULL;

	if((manager == NULL) || (filename == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	spinlock_writeLock(&manager->lock);

	if(dl_open(filename, &handle) < 0) {
		result = -1;

		log_logf(manager->log, LOG_LEVEL_ERROR,
				"failed to bind symbol table '%s' to transaction manager",
				filename);
	}
	else {
		if(manager->tables == NULL) {
			ref = 0;
			manager->tableLength = 1;
			manager->tables = (TransactionTable *)malloc(
					sizeof(TransactionTable) * manager->tableLength);
		}
		else {
			ref = manager->tableLength;
			manager->tableLength += 1;
			manager->tables = (TransactionTable *)realloc(manager->tables,
					(sizeof(TransactionTable) * manager->tableLength));
		}

		manager->tables[ref].filename = strdup(filename);
		manager->tables[ref].handle = handle;

		log_logf(manager->log, LOG_LEVEL_INFO,
				"bound symbol table '%s' to transaction manager",
				manager->tables[ref].filename);
	}

	spinlock_writeUnlock(&manager->lock);

	return result;
}

int transactionManager_setPacketDumpFunction(TransactionManager *manager,
		void *context, TransactionManagerPacketDumpFunction packetDumpFunction)
{
	if((manager == NULL) || (context == NULL) || (packetDumpFunction == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	spinlock_writeLock(&manager->lock);

	manager->packetDumpFunction = packetDumpFunction;
	manager->packetDumpContext = context;

	spinlock_writeUnlock(&manager->lock);

	return 0;
}

int transactionManager_setPacketDumpFunctionByName(TransactionManager *manager,
		void *context, char *functionName)
{
	int ii = 0;
	int result = 0;
	void *function = NULL;

	if((manager == NULL) || (context == NULL) || (functionName == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	spinlock_writeLock(&manager->lock);

	result = -1;

	if(manager->tables != NULL) {
		for(ii = 0; ii < manager->tableLength; ii++) {
			if(dl_lookup(manager->tables[ii].handle, functionName,
						&function) == 0) {
				manager->packetDumpFunction =
					(TransactionManagerPacketDumpFunction)function;
				manager->packetDumpContext = context;
				result = 0;
				break;
			}
		}
	}

	if(result == 0) {
		log_logf(manager->log, LOG_LEVEL_INFO,
				"set transaction manager packet dump function '%s()'",
				functionName);
	}
	else {
		log_logf(manager->log, LOG_LEVEL_ERROR,
				"failed to set transaction manager packet dump function '%s()'",
				functionName);
	}

	spinlock_writeUnlock(&manager->lock);

	return result;
}

int transactionManager_setReceiveFunction(TransactionManager *manager,
		void *context, TransactionManagerReceiveFunction receiveFunction)
{
	if((manager == NULL) || (context == NULL) || (receiveFunction == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	spinlock_writeLock(&manager->lock);

	manager->receiveFunction = receiveFunction;
	manager->receiveContext = context;

	spinlock_writeUnlock(&manager->lock);

	return 0;
}

int transactionManager_setReceiveFunctionByName(TransactionManager *manager,
		void *context, char *functionName)
{
	int ii = 0;
	int result = 0;
	void *function = NULL;

	if((manager == NULL) || (context == NULL) || (functionName == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	spinlock_writeLock(&manager->lock);

	result = -1;

	if(manager->tables != NULL) {
		for(ii = 0; ii < manager->tableLength; ii++) {
			if(dl_lookup(manager->tables[ii].handle, functionName,
						&function) == 0) {
				manager->receiveFunction =
					(TransactionManagerReceiveFunction)function;
				manager->receiveContext = context;
				result = 0;
				break;
			}
		}
	}

	if(result == 0) {
		log_logf(manager->log, LOG_LEVEL_INFO,
				"set transaction manager receive function '%s()'",
				functionName);
	}
	else {
		log_logf(manager->log, LOG_LEVEL_ERROR,
				"failed to set transaction manager receive function '%s()'",
				functionName);
	}

	spinlock_writeUnlock(&manager->lock);

	return result;
}

int transactionManager_setSendFunction(TransactionManager *manager,
		void *context, TransactionManagerSendFunction sendFunction)
{
	if((manager == NULL) || (context == NULL) || (sendFunction == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	spinlock_writeLock(&manager->lock);

	manager->sendFunction = sendFunction;
	manager->sendContext = context;

	spinlock_writeUnlock(&manager->lock);

	return 0;
}

int transactionManager_setSendFunctionByName(TransactionManager *manager,
		void *context, char *functionName)
{
	int ii = 0;
	int result = 0;
	void *function = NULL;

	if((manager == NULL) || (context == NULL) || (functionName == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	spinlock_writeLock(&manager->lock);

	result = -1;

	if(manager->tables != NULL) {
		for(ii = 0; ii < manager->tableLength; ii++) {
			if(dl_lookup(manager->tables[ii].handle, functionName,
						&function) == 0) {
				manager->sendFunction =
					(TransactionManagerSendFunction)function;
				manager->sendContext = context;
				result = 0;
				break;
			}
		}
	}

	if(result == 0) {
		log_logf(manager->log, LOG_LEVEL_INFO,
				"set transaction manager send function '%s()'",
				functionName);
	}
	else {
		log_logf(manager->log, LOG_LEVEL_ERROR,
				"failed to set transaction manager send function '%s()'",
				functionName);
	}

	spinlock_writeUnlock(&manager->lock);

	return result;
}

int transactionManager_setIdFunction(TransactionManager *manager,
		void *context, TransactionManagerIdFunction idFunction)
{
	if((manager == NULL) || (context == NULL) || (idFunction == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	spinlock_writeLock(&manager->lock);

	manager->idFunction = idFunction;
	manager->idContext = context;

	spinlock_writeUnlock(&manager->lock);

	return 0;
}

int transactionManager_setIdFunctionByName(TransactionManager *manager,
		void *context, char *functionName)
{
	int ii = 0;
	int result = 0;
	void *function = NULL;

	if((manager == NULL) || (context == NULL) || (functionName == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	spinlock_writeLock(&manager->lock);

	result = -1;

	if(manager->tables != NULL) {
		for(ii = 0; ii < manager->tableLength; ii++) {
			if(dl_lookup(manager->tables[ii].handle, functionName,
						&function) == 0) {
				manager->idFunction = (TransactionManagerIdFunction)function;
				manager->idContext = context;
				result = 0;
				break;
			}
		}
	}

	if(result == 0) {
		log_logf(manager->log, LOG_LEVEL_INFO,
				"set transaction manager ID function '%s()'",
				functionName);
	}
	else {
		log_logf(manager->log, LOG_LEVEL_ERROR,
				"failed to set transaction manager ID function '%s()'",
				functionName);
	}

	spinlock_writeUnlock(&manager->lock);

	return result;
}

int transactionManager_setRecvFreeFunction(TransactionManager *manager,
		ManagedIndexFreeFunction freeFunction)
{
	if((manager == NULL) || (freeFunction == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	spinlock_writeLock(&manager->lock);

	manager->recvFreeFunction = freeFunction;

	spinlock_writeUnlock(&manager->lock);

	return 0;
}

int transactionManager_setRecvFreeFunctionByName(TransactionManager *manager,
		char *functionName)
{
	int ii = 0;
	int result = 0;
	void *function = NULL;

	if((manager == NULL) || (functionName == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	spinlock_writeLock(&manager->lock);

	result = -1;

	if(manager->tables != NULL) {
		for(ii = 0; ii < manager->tableLength; ii++) {
			if(dl_lookup(manager->tables[ii].handle, functionName,
						&function) == 0) {
				manager->recvFreeFunction = (ManagedIndexFreeFunction)function;
				result = 0;
				break;
			}
		}
	}

	if(result == 0) {
		log_logf(manager->log, LOG_LEVEL_INFO,
				"set transaction manager free function '%s()'",
				functionName);
	}
	else {
		log_logf(manager->log, LOG_LEVEL_ERROR,
				"failed to set transaction manager free function '%s()'",
				functionName);
	}

	spinlock_writeUnlock(&manager->lock);

	return result;
}

int transactionManager_setSendFreeFunction(TransactionManager *manager,
		ManagedIndexFreeFunction freeFunction)
{
	if((manager == NULL) || (freeFunction == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	spinlock_writeLock(&manager->lock);

	manager->sendFreeFunction = freeFunction;

	spinlock_writeUnlock(&manager->lock);

	return 0;
}

int transactionManager_setSendFreeFunctionByName(TransactionManager *manager,
		char *functionName)
{
	int ii = 0;
	int result = 0;
	void *function = NULL;

	if((manager == NULL) || (functionName == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	spinlock_writeLock(&manager->lock);

	result = -1;

	if(manager->tables != NULL) {
		for(ii = 0; ii < manager->tableLength; ii++) {
			if(dl_lookup(manager->tables[ii].handle, functionName,
						&function) == 0) {
				manager->sendFreeFunction = (ManagedIndexFreeFunction)function;
				result = 0;
				break;
			}
		}
	}

	if(result == 0) {
		log_logf(manager->log, LOG_LEVEL_INFO,
				"set transaction manager free function '%s()'",
				functionName);
	}
	else {
		log_logf(manager->log, LOG_LEVEL_ERROR,
				"failed to set transaction manager free function '%s()'",
				functionName);
	}

	spinlock_writeUnlock(&manager->lock);

	return result;
}

int transactionManager_addHandler(TransactionManager *manager,
		aboolean isSocketLeftOpen, char *messageId, char *functionName,
		TransactionManagerExecFunction function, void *context)
{
	int result = 0;

	TransactionHandler *handler = NULL;

	if((manager == NULL) || (messageId == NULL) || (functionName == NULL) ||
			(function == NULL) || (context == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	spinlock_writeLock(&manager->lock);

	if(bptree_get(&manager->index, messageId, strlen(messageId),
				(void *)&handler) == 0) {
		result = -1;
	}
	else {
		handler = handler_new(isSocketLeftOpen, messageId, functionName,
				function, context);

		if(bptree_put(&manager->index, messageId, strlen(messageId),
					(void *)handler) < 0) {
			handler_free(handler);
			result = -1;
		}
	}

	if(result == 0) {
		log_logf(manager->log, LOG_LEVEL_INFO,
				"added handler function '%s()' to transaction manager "
				"for '%s'",
				functionName, messageId);
	}
	else {
		log_logf(manager->log, LOG_LEVEL_ERROR,
				"failed to add handler function '%s()' to transaction manager "
				"for '%s'",
				functionName, messageId);
	}

	spinlock_writeUnlock(&manager->lock);

	return result;
}

int transactionManager_addHandlerByName(TransactionManager *manager,
		aboolean isSocketLeftOpen, char *messageId, char *functionName,
		void *context)
{
	int ii = 0;
	int result = 0;
	void *function = NULL;

	TransactionHandler *handler = NULL;

	if((manager == NULL) || (messageId == NULL) || (functionName == NULL) ||
			(context == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	spinlock_writeLock(&manager->lock);

	if(bptree_get(&manager->index, messageId, strlen(messageId),
				(void *)&handler) == 0) {
		result = -1;
	}
	else {
		result = -1;

		if(manager->tables != NULL) {
			for(ii = 0; ii < manager->tableLength; ii++) {
				if(dl_lookup(manager->tables[ii].handle, functionName,
							&function) == 0) {
					result = 0;
					break;
				}
			}
		}

		if(result == 0) {
			handler = handler_new(isSocketLeftOpen, messageId, functionName,
					function, context);

			if(bptree_put(&manager->index, messageId, strlen(messageId),
						(void *)handler) < 0) {
				handler_free(handler);
				result = -1;
			}
		}
	}

	if(result == 0) {
		log_logf(manager->log, LOG_LEVEL_INFO,
				"added handler function '%s()' to transaction manager "
				"for '%s'",
				functionName, messageId);
	}
	else {
		log_logf(manager->log, LOG_LEVEL_ERROR,
				"failed to add handler function '%s()' to transaction manager "
				"for '%s'",
				functionName, messageId);
	}

	spinlock_writeUnlock(&manager->lock);

	return result;
}

int transactionManager_getMetrics(TransactionManager *manager,
		alint *transactionsExecuted, double *totalTransactionTime)
{
	int keyLength = 0;
	char *key = NULL;

	TransactionHandler *handler = NULL;

	if((manager == NULL) || (transactionsExecuted == NULL) ||
			(totalTransactionTime == NULL)) {
		DISPLAY_INVALID_ARGS;

		if(transactionsExecuted != NULL) {
			*transactionsExecuted = 0;
		}

		if(totalTransactionTime != NULL) {
			*totalTransactionTime = 0.0;
		}

		return -1;
	}

	*transactionsExecuted = 0;
	*totalTransactionTime = 0.0;

	spinlock_readLock(&manager->lock);

	keyLength = 1;
	key = (char *)malloc(sizeof(char) * (keyLength + 1));

	memset(key, 0, (int)(sizeof(char) * (keyLength + 1)));

	while(bptree_getNext(&manager->index, key, keyLength,
				((void *)&handler)) == 0) {
		free(key);

		key = strdup(handler->messageId);
		keyLength = strlen(key);

		mutex_lock(&handler->metrics.mutex);

		*transactionsExecuted += handler->metrics.transactionsExecuted;
		*totalTransactionTime += handler->metrics.totalTransactionTime;

		mutex_unlock(&handler->metrics.mutex);
	}

	free(key);

	spinlock_readUnlock(&manager->lock);

	return 0;
}

char *transactionManager_execIdFunction(TransactionManager *manager,
		void *request, int *messageIdLength)
{
	char *result = NULL;

	if((manager == NULL) || (request == NULL) || (messageIdLength == NULL)) {
		DISPLAY_INVALID_ARGS;

		if(messageIdLength != NULL) {
			*messageIdLength = 0;
		}

		return NULL;
	}

	spinlock_readLock(&manager->lock);

	result = manager->idFunction(manager->idContext, request,
			messageIdLength);

	log_logf(manager->log, LOG_LEVEL_DEBUG,
			"executed transaction manager ID function");

	spinlock_readUnlock(&manager->lock);

	return result;
}

void *transactionManager_execFunction(TransactionManager *manager,
		void *request)
{
	int hashLength = 0;
	int messageIdLength = 0;
	int memorySizeOfResponse = 0;
	char *hash = NULL;
	char *messageId = NULL;
	double timer = 0.0;
	void *result = NULL;
	void *response = NULL;

	TransactionHandler *handler = NULL;

	if((manager == NULL) || (request == NULL)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	spinlock_readLock(&manager->lock);

	timer = time_getTimeMus();

	messageId = manager->idFunction(manager->idContext, request,
			&messageIdLength);

	if((messageId == NULL) || (messageIdLength < 1)) {
		if(messageId != NULL) {
			free(messageId);
		}
		spinlock_readUnlock(&manager->lock);
		return NULL;
	}

	if(manager->isCacheEnabled) {
		hash = manager->hashFunction(manager->hashContext, request,
				&hashLength);
		if((hash != NULL) && (hashLength > 0)) {
			if((managedIndex_get(&manager->cache, hash, hashLength,
							(void *)&response) == 0) &&
					(response != NULL)) {
				result = manager->cloneFunction(manager->cloneContext,
						response, &memorySizeOfResponse);
			}
		}

		if(hash != NULL) {
			free(hash);
		}

		if(result != NULL) {
			spinlock_readUnlock(&manager->lock);
			free(messageId);
			return result;
		}
	}

	if(bptree_get(&manager->index, messageId, messageIdLength,
				(void *)&handler) < 0) {
		if(bptree_get(&manager->index, "default", 7, (void *)&handler) < 0) {
			spinlock_readUnlock(&manager->lock);
			free(messageId);
			return NULL;
		}
	}

	if(handler == NULL) {
		spinlock_readUnlock(&manager->lock);
		free(messageId);
		return NULL;
	}

	result = handler->function(handler->context, request,
			&memorySizeOfResponse);

	if((result != NULL) && (manager->isCacheEnabled)) {
		hash = manager->hashFunction(manager->hashContext, request,
				&hashLength);
		if((hash != NULL) && (hashLength > 0)) {
			response = manager->cloneFunction(manager->cloneContext, result,
					&memorySizeOfResponse);
			if((response != NULL) && (memorySizeOfResponse > 0)) {
				if(managedIndex_put(&manager->cache, hash, hashLength,
							(void *)response, memorySizeOfResponse) < 0) {
					manager->sendFreeFunction(manager->sendContext,
							response);
				}
			}
			else if(response != NULL) {
				manager->sendFreeFunction(manager->sendContext,
						response);
			}
		}

		if(hash != NULL) {
			free(hash);
		}
	}

	mutex_lock(&handler->metrics.mutex);
	handler->metrics.transactionsExecuted += 1;
	handler->metrics.totalTransactionTime += time_getElapsedMusInSeconds(timer);
	mutex_unlock(&handler->metrics.mutex);

	log_logf(manager->log, LOG_LEVEL_DEBUG,
			"executed transaction manager exec function");

	spinlock_readUnlock(&manager->lock);

	free(messageId);

	return result;
}

int transactionManager_execute(TransactionManager *manager, Socket *socket,
		int sd, aboolean *isSocketLeftOpen)
{
	aboolean isCached = afalse;
	aboolean isInCache = afalse;
	int rc = 0;
	int errorCode = 0;
	int hashLength = 0;
	int bytesReceived = 0;
	int messageIdLength = 0;
	int memorySizeOfResponse = 0;
	char *hash = NULL;
	char *messageId = NULL;
	double timer = 0.0;
	void *result = NULL;
	void *request = NULL;
	void *response = NULL;

	TransactionHandler *handler = NULL;

	if((manager == NULL) || (socket == NULL) || (sd < 0) ||
			(isSocketLeftOpen == NULL)) {
		DISPLAY_INVALID_ARGS;
		return recordError(manager, TRANSACTION_ERROR_INVALID_ARGS);
	}

	*isSocketLeftOpen = afalse;

	spinlock_readLock(&manager->lock);

	timer = time_getTimeMus();

	request = manager->receiveFunction(manager, manager->receiveContext,
			socket, sd, &bytesReceived, &errorCode);
	if(request == NULL) {
		spinlock_readUnlock(&manager->lock);

		if(errorCode >= 0) {
			errorCode = TRANSACTION_ERROR_READ;
		}

		log_logf(manager->log, LOG_LEVEL_ERROR, "tm failed to receive message");

		return recordError(manager, errorCode);
	}

	log_logf(manager->log, LOG_LEVEL_DEBUG,
			"tm received %i bytes from socket connection #%i",
			bytesReceived, sd);

	messageId = manager->idFunction(manager->idContext, request,
			&messageIdLength);

	if((messageId == NULL) || (messageIdLength < 1)) {
		if(messageId != NULL) {
			free(messageId);
		}

		manager->recvFreeFunction(manager->receiveContext, request);

		spinlock_readUnlock(&manager->lock);

		log_logf(manager->log, LOG_LEVEL_ERROR,
				"tm failed to identify message");

		return recordError(manager, TRANSACTION_ERROR_FAILED_IDENT);
	}

	log_logf(manager->log, LOG_LEVEL_DEBUG,
			"tm identified request as '%s'", messageId);

	if(manager->isCacheEnabled) {
		hash = manager->hashFunction(manager->hashContext, request,
				&hashLength);
		if((hash != NULL) && (hashLength > 0)) {
			if((managedIndex_get(&manager->cache, hash, hashLength,
							(void *)&response) == 0) &&
					(response != NULL)) {
				result = manager->cloneFunction(manager->cloneContext,
						response, &memorySizeOfResponse);
			}
		}

		if(result != NULL) {
			log_logf(manager->log, LOG_LEVEL_DEBUG,
					"tm obtained cached result for message '%s' hash '%s'",
					messageId, hash);

			isInCache = atrue;
		}
		else {
			log_logf(manager->log, LOG_LEVEL_DEBUG,
					"tm has no cache entry for message '%s' hash '%s'",
					messageId, hash);
		}

		if(hash != NULL) {
			free(hash);
		}
	}

	if(bptree_get(&manager->index, messageId, messageIdLength,
				(void *)&handler) < 0) {
		if(bptree_get(&manager->index, "default", 7,
					(void *)&handler) < 0) {
			manager->recvFreeFunction(manager->receiveContext, request);

			spinlock_readUnlock(&manager->lock);

			log_logf(manager->log, LOG_LEVEL_ERROR,
					"tm failed to locate handler for '%s'",
					messageId);

			free(messageId);

			return recordError(manager, TRANSACTION_ERROR_MISSING_HANDLER);
		}
	}

	if(handler == NULL) {
		manager->recvFreeFunction(manager->receiveContext, request);

		spinlock_readUnlock(&manager->lock);

		log_logf(manager->log, LOG_LEVEL_ERROR,
				"tm obtained NULL handler for '%s'",
				messageId);

		free(messageId);

		return recordError(manager, TRANSACTION_ERROR_INVALID_HANDLER);
	}

	*isSocketLeftOpen = handler->isSocketLeftOpen;

	if(handler->functionName != NULL) {
		log_logf(manager->log, LOG_LEVEL_DEBUG,
				"tm using handler '%s' at 0x%lx for messsage '%s'",
				handler->functionName, (aptrcast)handler, messageId);
	}
	else {
		log_logf(manager->log, LOG_LEVEL_DEBUG,
				"tm using handler 0x%lx for messsage '%s'",
				(aptrcast)handler, messageId);
	}

	if(result == NULL) {
		result = handler->function(handler->context, request,
				&memorySizeOfResponse);

		log_logf(manager->log, LOG_LEVEL_DEBUG,
				"tm obtained result 0x%lx of %i bytes from "
				"handler 0x%lx for message '%s'",
				(aptrcast)result, memorySizeOfResponse,
				(aptrcast)handler, messageId);

		if((result != NULL) && (memorySizeOfResponse > 0) &&
				(manager->isCacheEnabled)) {
			hash = manager->hashFunction(manager->hashContext, request,
					&hashLength);
			if((hash != NULL) && (hashLength > 0)) {
				if(managedIndex_put(&manager->cache, hash, hashLength,
							(void *)result, memorySizeOfResponse) < 0) {
					isCached = afalse;
				}
				else {
					log_logf(manager->log, LOG_LEVEL_DEBUG,
							"tm cached result 0x%lx of %i "
							"bytes for message '%s' hash '%s'",
							(aptrcast)result, memorySizeOfResponse,
							messageId, hash);

					isCached = atrue;
				}
			}

			if(hash != NULL) {
				free(hash);
			}
		}
	}

	manager->recvFreeFunction(manager->receiveContext, request);

	if(result != NULL) {
		log_logf(manager->log, LOG_LEVEL_DEBUG,
				"tm sending response 0x%lx for message '%s'",
				(aptrcast)result, messageId);

		if((rc = manager->sendFunction(manager, manager->sendContext, socket,
						sd, result)) < 0) {
			log_logf(manager->log, LOG_LEVEL_ERROR,
					"tm failed to send response 0x%lx for message '%s'",
					(aptrcast)result, messageId);

			rc = TRANSACTION_ERROR_WRITE;
		}
		else {
			log_logf(manager->log, LOG_LEVEL_DEBUG,
					"tm sent response 0x%lx of %i bytes for message '%s'",
					(aptrcast)result, rc, messageId);
		}

		if(!isCached) {
			manager->sendFreeFunction(manager->sendContext, result);
		}
	}
	else {
		log_logf(manager->log, LOG_LEVEL_ERROR,
				"tm obtained NULL result, no response sent to client");

		rc = TRANSACTION_ERROR_NULL_RESULT;
	}

	spinlock_readUnlock(&manager->lock);

	if((handler != NULL) && (!isInCache)) {
		mutex_lock(&handler->metrics.mutex);
		handler->metrics.transactionsExecuted += 1;
		handler->metrics.totalTransactionTime +=
			time_getElapsedMusInSeconds(timer);
		mutex_unlock(&handler->metrics.mutex);

		if(handler->functionName != NULL) {
			log_logf(manager->log, LOG_LEVEL_DEBUG,
					"tm updated metrics for handler '%s' at 0x%lx with %0.6f "
					"avg execution time for %i transactions for message '%s'",
					handler->functionName, (aptrcast)handler,
					(handler->metrics.totalTransactionTime /
					 (double)handler->metrics.transactionsExecuted),
					handler->metrics.transactionsExecuted,
					messageId);
		}
		else {
			log_logf(manager->log, LOG_LEVEL_DEBUG,
					"tm updated metrics for handler 0x%lx with %0.6f "
					"avg execution time for %i transactions for message '%s'",
					(aptrcast)handler,
					(handler->metrics.totalTransactionTime /
					 (double)handler->metrics.transactionsExecuted),
					handler->metrics.transactionsExecuted,
					messageId);
		}
	}

	free(messageId);

	if(rc < 0) {
		return recordError(manager, rc);
	}

	return 0;
}

