/*
 * transaction_manager_unitTest.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard core library system for desginating and executing transactions
 * through message-recognition in order to execute handler functions designated
 * directly or through symbol tables, unit test.
 *
 * Written by Josh English.
 */

// preprocessor directives

#define _CORE_COMMON_BYPASS_STD_STREAM
#include "core/core.h"


// define transaction manager unit test constants

#define BASIC_FUNCTIONALITY_VERBOSE_MODE				afalse

#define BASIC_FUNCTIONALITY_TESTS						1024


// declare transaction manager unit test data types

typedef struct _DefaultRequestContext {
	int requestLength;
	char *request;
} DefaultRequestContext;

typedef struct _DefaultResponseContext {
	int responseLength;
	char *response;
} DefaultResponseContext;


// declare transaction manager unit test private functions

static int transaction_manager_basicFunctionalityTest();


// main function

int main()
{
	double timer = 0.0;

	signal_registerDefault();

	printf("[unit] transaction manager unit test "
			"(Asgard Ver %s on %s)\n", ASGARD_VERSION, ASGARD_DATE);

	timer = time_getTimeMus();

	if(transaction_manager_basicFunctionalityTest() < 0) {
		printf("[unit]\t ...FAILED\n");
		return 1;
	}

	printf("[unit]\t ...PASSED (%0.6f seconds)\n",
			time_getElapsedMusInSeconds(timer));
	printf("[unit] ...PASSED\n");

	return 0;
}


// define transaction manager unit test private functions

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

static int transaction_manager_basicFunctionalityTest()
{
	int ii = 0;
	int stringLength = 0;
	alint transactionsExecuted = 0;
	double totalTransactionTime = 0;
	char *string = NULL;
	void *handle = NULL;
	void *hashFunction = NULL;
	void *cloneFunction = NULL;

	DefaultRequestContext request;
	DefaultResponseContext *response = NULL;
	Log log;
	TransactionManager manager;

	printf("[unit]\t transaction manager basic functionality test...\n");

	if(dl_open("lib/libasgardcore_unitTest.so", &handle) < 0) {
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

	log_init(&log, LOG_OUTPUT_STDOUT, NULL, LOG_LEVEL_DEBUG);

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

	if(transactionManager_disableCache(&manager) < 0) {
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
				"lib/libasgardcore_unitTest.so") < 0) {
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

	for(ii = 0; ii < BASIC_FUNCTIONALITY_TESTS; ii++) {
		request.request = "this is a test";
		request.requestLength = strlen(request.request);

		string = transactionManager_execIdFunction(&manager,
				(void *)&request,
				&stringLength);

		if((string == NULL) || (stringLength < 1)) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		printf("[unit]\t\t id function returned (%i)    : '%s'\n", stringLength,
				string);

		free(string);

		response = transactionManager_execFunction(&manager, (void *)&request);

		if((response == NULL) || (response->response == NULL) ||
				(response->responseLength < 1)) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		printf("[unit]\t\t exec function returned (%i) : '%s'\n",
				response->responseLength,
				response->response);

		defaultResponse_free(NULL, response);
	}

	if(transactionManager_getMetrics(&manager, &transactionsExecuted,
				&totalTransactionTime) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t\t transaction metrics - executions       : %lli\n",
			transactionsExecuted);
	printf("[unit]\t\t transaction metrics - transaction time : %0.6f\n",
			totalTransactionTime);
	printf("[unit]\t\t transaction metrics - avg time         : %0.6f\n",
			(totalTransactionTime / (double)transactionsExecuted));

	if(transactionManager_free(&manager) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	// second pass, with no caching

	log_setLevel(&log, LOG_LEVEL_INFO);

	if(transactionManager_init(&manager, &log) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(transactionManager_bindSymbolTable(&manager,
				"lib/libasgardcore_unitTest.so") < 0) {
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

	for(ii = 0; ii < BASIC_FUNCTIONALITY_TESTS; ii++) {
		request.request = "this is a test";
		request.requestLength = strlen(request.request);

		string = transactionManager_execIdFunction(&manager,
				(void *)&request,
				&stringLength);

		if((string == NULL) || (stringLength < 1)) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		free(string);

		response = transactionManager_execFunction(&manager, (void *)&request);

		if((response == NULL) || (response->response == NULL) ||
				(response->responseLength < 1)) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		defaultResponse_free(NULL, response);
	}

	if(transactionManager_getMetrics(&manager, &transactionsExecuted,
				&totalTransactionTime) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t\t transaction metrics - executions       : %lli\n",
			transactionsExecuted);
	printf("[unit]\t\t transaction metrics - transaction time : %0.6f\n",
			totalTransactionTime);
	printf("[unit]\t\t transaction metrics - avg time         : %0.6f\n",
			(totalTransactionTime / (double)transactionsExecuted));

	if(transactionManager_free(&manager) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	log_free(&log);

	return 0;
}

