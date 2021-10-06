/*
 * server.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The multithreaded server API to provide a framework for managed network
 * server transactions.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#define _CORE_LIBRARY_NETWORK_COMPONENT
#include "core/library/network/server.h"


// define server private data types

typedef struct _WorkerThreadArgs {
	int serverThreadId;
	Server *server;
} WorkerThreadArgs;


// declare server private functions

// error, metrics and tracking functions

static void recordError(Server *server, int type);

static void recordMetric(Server *server, int type, double timing);

static void recordTracking(Server *server, double timing);

// server status functions

static ServerStatus *newServerStatus(char *key, char *name, char *value,
		char *description);

static void freeServerStatus(void *memory);

// thread functions

static aboolean isWorkerRequired(Server *server);

static int pickAvailableThread(Server *server);

static aboolean isWorkerShutdownRequired(Server *server, int *threadId);

static void *manageServerThread(void *threadContext, void *argument);

static void *serverWorkerThread(void *threadContext, void *argument);


// define server private functions

// error, metrics and tracking functions

static void recordError(Server *server, int type)
{
	if((type <= SERVER_ERROR_TOTAL) || (type >= SERVER_ERROR_LENGTH)) {
		fprintf(stderr, "[%s():%i] error - unable to record error for type "
				"%i.\n", __FUNCTION__, __LINE__, type);
		return;
	}

	mutex_lock(&server->errors.mutex);

	server->errors.counter[SERVER_ERROR_TOTAL] += 1;
	server->errors.counter[type] += 1;

	mutex_unlock(&server->errors.mutex);
}

static void recordMetric(Server *server, int type, double timing)
{
	if((type <= SERVER_METRIC_TOTAL) || (type >= SERVER_METRIC_LENGTH)) {
		fprintf(stderr, "[%s():%i] error - unable to record metric for type "
				"%i.\n", __FUNCTION__, __LINE__, type);
		return;
	}

	mutex_lock(&server->metrics.mutex);

	server->metrics.transactionCounter += 1;

	server->metrics.counter[type] += 1;
	server->metrics.timings[type] += timing;

	server->metrics.counter[SERVER_METRIC_TOTAL] += 1;
	server->metrics.timings[SERVER_METRIC_TOTAL] += timing;

	mutex_unlock(&server->metrics.mutex);

	if(type == SERVER_METRIC_TRANSACTION) {
		recordTracking(server, timing);
	}
}

static void recordTracking(Server *server, double timing)
{
	mutex_lock(&server->tracking.mutex);

	server->tracking.timing += timing;
	server->tracking.counter += 1.0;

	mutex_unlock(&server->tracking.mutex);
}

// server status functions

static ServerStatus *newServerStatus(char *key, char *name, char *value,
		char *description)
{
	ServerStatus *result = NULL;

	result = (ServerStatus *)malloc(sizeof(ServerStatus));
	result->key = strdup(key);
	result->name = strdup(name);
	result->value = strdup(value);
	result->description = strdup(description);

	return result;
}

static void freeServerStatus(void *memory)
{
	ServerStatus *status = NULL;

	if(memory == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	status = (ServerStatus *)memory;

	if(status->key != NULL) {
		free(status->key);
	}

	if(status->name != NULL) {
		free(status->name);
	}

	if(status->value != NULL) {
		free(status->value);
	}

	if(status->description != NULL) {
		free(status->description);
	}

	free(status);
}

// thread functions

static aboolean isWorkerRequired(Server *server)
{
	aboolean result = afalse;
	int currentThreadCount = 0;
	int activeThreadCount = 0;
	double inUsePercent = 0.0;

	mutex_lock(&server->threads.mutex);

	if(server->threads.currentThreadPoolLength <
			server->threads.minThreadPoolLength) {
		log_logf(server->log, LOG_LEVEL_INFO,
				"new worker required, pool is %i of minimum %i",
				server->threads.currentThreadPoolLength,
				server->threads.minThreadPoolLength);

		mutex_unlock(&server->threads.mutex);

		return atrue;
	}

	if(server->threads.currentThreadPoolLength >=
			server->threads.maxThreadPoolLength) {
/*		log_logf(server->log, LOG_LEVEL_DEBUG,
				"new worker not required, pool is %i of maximum %i",
				server->threads.currentThreadPoolLength,
				server->threads.minThreadPoolLength);*/

		mutex_unlock(&server->threads.mutex);

		return afalse;
	}

	mutex_unlock(&server->threads.mutex);

	tmf_getCurrentThreadCount(&server->tmf, &currentThreadCount);
	tmf_getActiveThreadCount(&server->tmf, &activeThreadCount);

	inUsePercent = (((double)activeThreadCount /
				(double)currentThreadCount) * 100.0);

	if(inUsePercent >= 85.0) { // 85.0 %
		result = atrue;

		log_logf(server->log, LOG_LEVEL_INFO,
				"new worker required, pool is %i active of %i "
				"available (%0.2f %%)",
				activeThreadCount,
				currentThreadCount,
				inUsePercent);
	}
	else {
/*		log_logf(server->log, LOG_LEVEL_DEBUG,
				"new worker not required, pool is %i active of %i "
				"available (%0.2f %%)",
				activeThreadCount,
				currentThreadCount,
				inUsePercent);*/
	}

	return result;
}

static int pickAvailableThread(Server *server)
{
	int ii = 0;
	int result = -1;

	mutex_lock(&server->threads.mutex);

	for(ii = 0; ii < server->threads.maxThreadPoolLength; ii++) {
		if(server->threads.list[ii].state == THREAD_STATE_INIT) {
			result = ii;
			break;
		}
	}

	mutex_unlock(&server->threads.mutex);

	return result;
}

static aboolean isWorkerShutdownRequired(Server *server, int *threadId)
{
	int ii = 0;
	int currentThreadCount = 0;
	int activeThreadCount = 0;
	double inUsePercent = 0.0;

	*threadId = -1;

	mutex_lock(&server->threads.mutex);

	if(server->threads.currentThreadPoolLength <=
			server->threads.minThreadPoolLength) {
		mutex_unlock(&server->threads.mutex);

		return afalse;
	}

	mutex_unlock(&server->threads.mutex);

	tmf_getCurrentThreadCount(&server->tmf, &currentThreadCount);
	tmf_getActiveThreadCount(&server->tmf, &activeThreadCount);

	inUsePercent = (((double)activeThreadCount /
				(double)currentThreadCount) * 100.0);

	if(inUsePercent >= 35.0) { // 25.0 %
		return afalse;
	}

	log_logf(server->log, LOG_LEVEL_INFO,
			"worker shutdown required, pool is %i active of %i "
			"available (%0.2f %%)",
			activeThreadCount,
			currentThreadCount,
			inUsePercent);

	mutex_lock(&server->threads.mutex);

	for(ii = 0; ii < server->threads.maxThreadPoolLength; ii++) {
		if((server->threads.list[ii].state == THREAD_STATE_RUNNING) &&
				(server->threads.list[ii].status == THREAD_STATUS_NAPPING) &&
				(server->threads.list[ii].signal == THREAD_SIGNAL_OK)) {
			mutex_unlock(&server->threads.mutex);
			*threadId = server->threads.list[ii].id;
			return atrue;
		}
	}

	mutex_unlock(&server->threads.mutex);

	return afalse;
}

static void *manageServerThread(void *threadContext, void *argument)
{
	aboolean tookAction = afalse;
	aboolean hasCriticalError = afalse;
	int ii = 0;
	int counter = 0;
	int trackingRef = 0;
	int threadId = 0;
	int serverThreadId = 0;
	int currentThreadPoolLength = 0;
	double trackingTimestamp = 0.0;
	char *stdBuffer = NULL;

	TmfThread *thread = NULL;
	Server *server = NULL;

	WorkerThreadArgs worker;

	thread = (TmfThread *)threadContext;
	server = (Server *)argument;

	log_logf(server->log, LOG_LEVEL_INFO,
			"server '%s' management thread #%i (%i) started",
			server->name, thread->uid, thread->systemId);

	trackingTimestamp = time_getTimeMus();

	while((thread->state == THREAD_STATE_RUNNING) &&
			(thread->signal == THREAD_SIGNAL_OK) &&
			(server->state == SERVER_STATE_RUNNING)) {
		mutex_lock(thread->mutex);
		thread->status = THREAD_STATUS_BUSY;
		mutex_unlock(thread->mutex);

		tookAction = afalse;

		// update the tracking information

		if(time_getElapsedMusInSeconds(trackingTimestamp) >= 1.0) {
			mutex_lock(&server->tracking.mutex);

			trackingRef += 1;
			if(trackingRef > SERVER_TRACKING_ENTRY_LENGTH) {
				trackingRef = SERVER_TRACKING_ENTRY_LENGTH;
			}

			for(ii = (SERVER_TRACKING_ENTRY_LENGTH - 1); ii >= 0; ii--) {
				server->tracking.timings[ii] =
					server->tracking.timings[(ii - 1)];

				server->tracking.counts[ii] = server->tracking.counts[(ii - 1)];
			}

			server->tracking.timings[0] = server->tracking.timing;
			server->tracking.counts[0] = server->tracking.counter;

			server->tracking.timingAverage = 0.0;
			server->tracking.countAverage = 0.0;

			for(ii = 0; ii < trackingRef; ii++) {
				server->tracking.timingAverage +=
					server->tracking.timings[ii];
				server->tracking.countAverage +=
					server->tracking.counts[ii];
			}

			server->tracking.countAverage /= (double)trackingRef;

			server->tracking.timingAverage /= (double)trackingRef;

			server->tracking.timingAverage /= server->tracking.countAverage;

			server->tracking.timing = 0.0;
			server->tracking.counter = 0.0;

			mutex_unlock(&server->tracking.mutex);

			trackingTimestamp = time_getTimeMus();
		}

		if((thread->state != THREAD_STATE_RUNNING) ||
				(thread->signal != THREAD_SIGNAL_OK)) {
			break;
		}

		// spawn a new worker thread, if required

		while((isWorkerRequired(server)) &&
				((serverThreadId = pickAvailableThread(server)) >= 0)) {
			worker.serverThreadId = serverThreadId;
			worker.server = server;

			mutex_lock(&server->threads.mutex);

			currentThreadPoolLength = server->threads.currentThreadPoolLength;

			if(tmf_spawnThread(&server->tmf,
						serverWorkerThread,
						(void *)&worker,
						&(server->threads.list[serverThreadId].id)) < 0) {
				mutex_unlock(&server->threads.mutex);

				log_logf(server->log, LOG_LEVEL_ERROR,
						"failed to spawn worker thread, aborting");

				hasCriticalError = atrue;

				break;
			}

			mutex_unlock(&server->threads.mutex);

			counter = 0;

			while((thread->state == THREAD_STATE_RUNNING) &&
					(thread->signal == THREAD_SIGNAL_OK) &&
					(currentThreadPoolLength ==
					 server->threads.currentThreadPoolLength) &&
					(counter < 128)) {
				time_usleep(1024);

				counter++;
			}

			if((thread->state != THREAD_STATE_RUNNING) ||
					(thread->signal != THREAD_SIGNAL_OK)) {
				break;
			}

			if(counter >= 128) {
				break;
			}
		}

		if((thread->state != THREAD_STATE_RUNNING) ||
				(thread->signal != THREAD_SIGNAL_OK)) {
			break;
		}

		// shutdown inactive workers, if required

		if((server->threads.isThreadPoolShrinkEnabled) &&
				(isWorkerShutdownRequired(server, &threadId))) {
			if(tmf_signalThread(&server->tmf, threadId,
						THREAD_SIGNAL_QUIT) < 0) {
				log_logf(server->log, LOG_LEVEL_ERROR,
						"failed to stop worker thread, aborting");

				hasCriticalError = atrue;

				break;
			}
			else {
				log_logf(server->log, LOG_LEVEL_INFO,
						"shutdown worker thread #%i", threadId);
			}
		}

		if((thread->state != THREAD_STATE_RUNNING) ||
				(thread->signal != THREAD_SIGNAL_OK)) {
			break;
		}

		// log the contents from the stderr & stdout streams

		while((stdBuffer = common_popStdStreamError()) != NULL) {
			log_logf(server->log, LOG_LEVEL_ERROR, "STDERR %s", stdBuffer);
			free(stdBuffer);
		}

		while((stdBuffer = common_popStdStreamOutput()) != NULL) {
			log_logf(server->log, LOG_LEVEL_INFO, "STDOUT %s", stdBuffer);
			free(stdBuffer);
		}

		if((thread->state != THREAD_STATE_RUNNING) ||
				(thread->signal != THREAD_SIGNAL_OK)) {
			break;
		}

		mutex_lock(thread->mutex);
		thread->status = THREAD_STATUS_NAPPING;
		mutex_unlock(thread->mutex);

		// take a nap

		if(!tookAction) {
			counter = 0;

			while((thread->state == THREAD_STATE_RUNNING) &&
					(thread->signal == THREAD_SIGNAL_OK) &&
					(counter < 128)) {
				time_usleep(1024);
				counter++;
			}
		}
	}

	if(hasCriticalError) {
		log_logf(server->log, LOG_LEVEL_PANIC,
				"server encountered a critical error and is "
				"aborting execution");

		socket_requestShutdown(&server->socket);

		for(ii = 0; ii < server->threads.maxThreadPoolLength; ii++) {
			if((server->threads.list[ii].state == THREAD_STATE_RUNNING) &&
					(server->threads.list[ii].signal == THREAD_SIGNAL_OK)) {
				tmf_stopThread(&server->tmf, server->threads.list[ii].id);
			}
		}

		socket_close(&server->socket);

		mutex_lock(&server->mutex);
		server->state = SERVER_STATE_ERROR;
		mutex_unlock(&server->mutex);
	}

	log_logf(server->log, LOG_LEVEL_INFO,
			"server '%s' management thread #%i (%i) shut down",
			server->name, thread->uid, thread->systemId);

	if(hasCriticalError) {
		mutex_lock(thread->mutex);
		thread->state = THREAD_STATE_HALT;
		thread->status = THREAD_STATUS_ERROR;
		mutex_unlock(thread->mutex);
	}
	else if(thread->state == THREAD_STATE_RUNNING) {
		mutex_lock(thread->mutex);
		thread->state = THREAD_STATE_HALT;
		mutex_unlock(thread->mutex);
	}

	return NULL;
}

static void *serverWorkerThread(void *threadContext, void *argument)
{
	aboolean isSocketLeftOpen = afalse;
	aboolean shutdownRequested = afalse;
	int rc = 0;
	int sd = 0;
	int serverThreadId = 0;
	double timer = 0.0;
	double timing = 0.0;
	char *peerName = NULL;

	TmfThread *thread = NULL;
	Server *server = NULL;
	WorkerThreadArgs *worker = NULL;

	if((threadContext == NULL) || (argument == NULL)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	thread = (TmfThread *)threadContext;
	worker = (WorkerThreadArgs *)argument;
	server = worker->server;
	serverThreadId = worker->serverThreadId;

	log_logf(server->log, LOG_LEVEL_INFO,
			"server '%s' worker #%i thread #%i (%i) started",
			server->name, serverThreadId, thread->uid, thread->systemId);

	mutex_lock(&server->threads.mutex);
	mutex_lock(thread->mutex);

	server->threads.list[serverThreadId].state = (int)thread->state;
	server->threads.list[serverThreadId].status = (int)thread->status;
	server->threads.list[serverThreadId].signal = (int)thread->signal;
	server->threads.list[serverThreadId].id = (int)thread->uid;

	mutex_unlock(thread->mutex);

	server->threads.currentThreadPoolLength += 1;

	mutex_unlock(&server->threads.mutex);

	// set status to napping

	mutex_lock(&server->threads.mutex);
	mutex_lock(thread->mutex);

	thread->status = THREAD_STATUS_NAPPING;
	server->threads.list[serverThreadId].status = (int)thread->status;

	mutex_unlock(thread->mutex);
	mutex_unlock(&server->threads.mutex);

	while((thread->state == THREAD_STATE_RUNNING) &&
			(thread->signal == THREAD_SIGNAL_OK) &&
			(server->state == SERVER_STATE_RUNNING)) {
		log_logf(server->log, LOG_LEVEL_DEBUG,
				"worker #%i waiting for socket traffic",
				serverThreadId);

		// accept a socket connection

		if(socket_accept(&server->socket, &sd, 131072) < 0) {
			if(server->socket.config.shutdownRequested) {
				log_logf(server->log, LOG_LEVEL_INFO,
						"worker #%i noted shutdown requested on socket",
						serverThreadId);

				shutdownRequested = atrue;

				break;
			}

			log_logf(server->log, LOG_LEVEL_ERROR,
					"worker #%i failed to accept socket connection",
					serverThreadId);

			recordError(server, SERVER_ERROR_SOCKET_ACCEPT);

			continue;
		}

		if((thread->state != THREAD_STATE_RUNNING) ||
				(thread->signal == THREAD_SIGNAL_PANIC_STOP) ||
				(server->state != SERVER_STATE_RUNNING)) {
			break;
		}
		else if(thread->signal != THREAD_SIGNAL_OK) {
			mutex_lock(&server->threads.mutex);
			server->threads.list[serverThreadId].signal = (int)thread->signal;
			mutex_unlock(&server->threads.mutex);
		}

		log_logf(server->log, LOG_LEVEL_DEBUG,
				"worker #%i accepted socket connection #%i",
				serverThreadId, sd);

		// set status to busy

		mutex_lock(&server->threads.mutex);
		mutex_lock(thread->mutex);

		thread->status = THREAD_STATUS_BUSY;
		server->threads.list[serverThreadId].status = (int)thread->status;
		server->threads.list[serverThreadId].signal = (int)thread->signal;

		mutex_unlock(thread->mutex);
		mutex_unlock(&server->threads.mutex);

		// obtain peer name for client connection

		peerName = NULL;

		if(server->socket.protocol == SOCKET_PROTOCOL_TCPIP) {
			socket_getPeerName(&server->socket, sd, &peerName);
		}

		if(server->threads.list[serverThreadId].ip != NULL) {
			free(server->threads.list[serverThreadId].ip);
			server->threads.list[serverThreadId].ip = NULL;
		}

		if(peerName != NULL) {
			server->threads.list[serverThreadId].ip = peerName;

			log_logf(server->log, LOG_LEVEL_DEBUG,
					"worker #%i noted connection from '%s' on #%i",
					serverThreadId,
					server->threads.list[serverThreadId].ip,
					sd);
		}

		if((thread->state != THREAD_STATE_RUNNING) ||
				(thread->signal == THREAD_SIGNAL_PANIC_STOP) ||
				(server->state != SERVER_STATE_RUNNING)) {
			break;
		}
		else if(thread->signal != THREAD_SIGNAL_OK) {
			mutex_lock(&server->threads.mutex);
			server->threads.list[serverThreadId].signal = (int)thread->signal;
			mutex_unlock(&server->threads.mutex);
		}

		// execute transaction manager

		do {
			isSocketLeftOpen = afalse;

			timer = time_getTimeMus();

			if((rc = transactionManager_execute(server->manager,
							&server->socket,
							sd,
							&isSocketLeftOpen)) < 0) {
				log_logf(server->log, LOG_LEVEL_ERROR,
						"worker #%i transaction manager execution failure '%s'",
						serverThreadId, transactionManager_getErrorString(rc));

				switch(rc) {
					case TRANSACTION_ERROR_READ:
						recordError(server, SERVER_ERROR_SOCKET_READ);
						break;

					case TRANSACTION_ERROR_WRITE:
						recordError(server, SERVER_ERROR_SOCKET_WRITE);
						break;

					case TRANSACTION_ERROR_CLOSE:
						recordError(server, SERVER_ERROR_SOCKET_CLOSE);
						break;

					case TRANSACTION_ERROR_TIMEOUT:
						recordError(server, SERVER_ERROR_SOCKET_TIMEOUT);
						break;

					case TRANSACTION_ERROR_INVALID_ARGS:
					case TRANSACTION_ERROR_FAILED_IDENT:
					case TRANSACTION_ERROR_MISSING_HANDLER:
					case TRANSACTION_ERROR_INVALID_HANDLER:
					case TRANSACTION_ERROR_NULL_RESULT:
					case TRANSACTION_ERROR_INVALID_RESULT:
					default:
						recordError(server, SERVER_ERROR_TRANSACTION);
				}
			}

			timing = time_getElapsedMusInSeconds(timer);

			recordMetric(server, SERVER_METRIC_TRANSACTION, timing);

			log_logf(server->log, LOG_LEVEL_DEBUG,
					"worker #%i executed transaction manager in %0.6f seconds",
					serverThreadId, timing);
		} while((thread->state == THREAD_STATE_RUNNING) &&
				(thread->signal == THREAD_SIGNAL_OK) &&
				(server->state == SERVER_STATE_RUNNING) &&
				(isSocketLeftOpen));

		if((thread->state != THREAD_STATE_RUNNING) ||
				(thread->signal == THREAD_SIGNAL_PANIC_STOP) ||
				(server->state != SERVER_STATE_RUNNING)) {
			break;
		}
		else if(thread->signal != THREAD_SIGNAL_OK) {
			mutex_lock(&server->threads.mutex);
			server->threads.list[serverThreadId].signal = (int)thread->signal;
			mutex_unlock(&server->threads.mutex);
		}

		// close the socket

		if(socket_closeChildSocket(&server->socket, sd) < 0) {
			log_logf(server->log, LOG_LEVEL_ERROR,
					"worker #%i failed to close child socket on %i",
					serverThreadId, sd);

			recordError(server, SERVER_ERROR_SOCKET_CLOSE);
		}

		// set status to napping

		mutex_lock(&server->threads.mutex);
		mutex_lock(thread->mutex);

		thread->status = THREAD_STATUS_NAPPING;
		server->threads.list[serverThreadId].status = (int)thread->status;
		server->threads.list[serverThreadId].signal = (int)thread->signal;

		mutex_unlock(thread->mutex);
		mutex_unlock(&server->threads.mutex);
	}

	if(shutdownRequested) {
		while((thread->state == THREAD_STATE_RUNNING) &&
				(thread->signal == THREAD_SIGNAL_OK)) {
			time_usleep(1024);
		}
	}

	if(thread->state == THREAD_STATE_RUNNING) {
		mutex_lock(thread->mutex);
		thread->state = THREAD_STATE_HALT;
		mutex_unlock(thread->mutex);
	}

	mutex_lock(&server->threads.mutex);

	server->threads.list[serverThreadId].state = THREAD_STATE_INIT;
	server->threads.list[serverThreadId].status = THREAD_STATUS_INIT;
	server->threads.list[serverThreadId].signal = THREAD_SIGNAL_OK;
	server->threads.list[serverThreadId].id = 0;

	if(server->threads.list[serverThreadId].ip != NULL) {
		free(server->threads.list[serverThreadId].ip);
	}

	server->threads.list[serverThreadId].ip = NULL;

	server->threads.currentThreadPoolLength -= 1;

	mutex_unlock(&server->threads.mutex);

	log_logf(server->log, LOG_LEVEL_INFO,
			"server '%s' worker #%i thread #%i (%i) shut down",
			server->name, serverThreadId, thread->uid, thread->systemId);

	return NULL;
}


// define server public functions

int server_init(Server *server, Log *log)
{
	if(server == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	memset(server, 0, (int)(sizeof(Server)));

	server->isSocketInit = afalse;

	if(log == NULL) {
		server->isLogInternallyAllocated = atrue;
		server->log = (Log *)malloc(sizeof(Log));

		log_init(server->log, LOG_OUTPUT_STDOUT, NULL, LOG_LEVEL_INFO);
	}
	else {
		server->isLogInternallyAllocated = afalse;
		server->log = log;
	}

	server->state = SERVER_STATE_INIT;
	server->startTimestamp = 0.0;
	server->name = strdup("Asgard Server");

	mutex_init(&server->errors.mutex);

	mutex_init(&server->metrics.mutex);

	mutex_init(&server->tracking.mutex);

	server->threads.isThreadPoolShrinkEnabled = atrue;

	mutex_init(&server->threads.mutex);

	mutex_init(&server->packets.mutex);

	server->manager = NULL;

	tmf_init(&server->tmf);

	mutex_init(&server->mutex);

	server->memory = NULL;

#if defined(MEMORY_REPLACED)
	server->memory = memory_getState();
#endif // MEMORY_REPLACED

	return 0;
}

int server_free(Server *server)
{
	int ii = 0;

	if(server == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&server->mutex);

	if(server->state == SERVER_STATE_RUNNING) {
		server->state = SERVER_STATE_SHUTDOWN;

		mutex_unlock(&server->mutex);

		if(tmf_free(&server->tmf) < 0) {
			mutex_lock(&server->mutex);
			server->state = SERVER_STATE_ERROR;
			mutex_unlock(&server->mutex);

			log_logf(server->log, LOG_LEVEL_ERROR,
					"failed to shut down thread pool");

			return -1;
		}
		else {
			mutex_lock(&server->mutex);
			server->state = SERVER_STATE_STOPPED;
			mutex_unlock(&server->mutex);
		}

		mutex_lock(&server->mutex);

		if(socket_close(&server->socket) < 0) {
			server->state = SERVER_STATE_ERROR;
		}
		else {
			server->state = SERVER_STATE_STOPPED;
		}
	}
	else {
		tmf_free(&server->tmf);
	}

	socket_free(&server->socket);

	if(server->name != NULL) {
		free(server->name);
	}

	mutex_lock(&server->threads.mutex);

	if(server->threads.list != NULL) {
		for(ii = 0; ii < server->threads.maxThreadPoolLength; ii++) {
			if(server->threads.list[ii].ip != NULL) {
				free(server->threads.list[ii].ip);
			}
		}

		free(server->threads.list);
	}

	mutex_unlock(&server->threads.mutex);

	mutex_lock(&server->packets.mutex);

	if(server->packets.path != NULL) {
		free(server->packets.path);
	}

	if(server->packets.filename != NULL) {
		free(server->packets.filename);
	}

	if(server->packets.isEnabled) {
		file_free(&server->packets.handle);
	}

	mutex_unlock(&server->packets.mutex);

	if(server->isLogInternallyAllocated) {
		log_free(server->log);
		free(server->log);
	}

	mutex_free(&server->errors.mutex);

	mutex_free(&server->metrics.mutex);

	mutex_free(&server->tracking.mutex);

	mutex_free(&server->threads.mutex);

	mutex_free(&server->packets.mutex);

	mutex_unlock(&server->mutex);

	mutex_free(&server->mutex);

	memset(server, 0, (int)(sizeof(Server)));

	return 0;
}

char *server_getServerStateString(ServerStates state)
{
	char *result = NULL;

	switch(state) {
		case SERVER_STATE_INIT:
			result = "Init";
			break;

		case SERVER_STATE_RUNNING:
			result = "Running";
			break;

		case SERVER_STATE_SHUTDOWN:
			result = "Shutdown";
			break;

		case SERVER_STATE_STOPPED:
			result = "Stopped";
			break;

		case SERVER_STATE_ERROR:
			result = "Error";
			break;

		default:
			result = "Unknokwn";
	}

	return result;
}

char *server_getServerErrorString(ServerErrors error)
{
	char *result = NULL;

	switch(error) {
		case SERVER_ERROR_TOTAL:
			result = "Total";
			break;

		case SERVER_ERROR_SOCKET_ACCEPT:
			result = "Socket Accept";
			break;

		case SERVER_ERROR_SOCKET_READ:
			result = "Socket Read";
			break;

		case SERVER_ERROR_SOCKET_WRITE:
			result = "Socket Write";
			break;

		case SERVER_ERROR_SOCKET_CLOSE:
			result = "Socket Close";
			break;

		case SERVER_ERROR_SOCKET_TIMEOUT:
			result = "Socket Timeout";
			break;

		case SERVER_ERROR_TRANSACTION:
			result = "Transaction";
			break;

		case SERVER_ERROR_LENGTH:
		case SERVER_ERROR_UNKNOWN:
		default:
			result = "Unknokwn";
	}

	return result;
}

char *server_getServerMetricString(ServerMetrics metric)
{
	char *result = NULL;

	switch(metric) {
		case SERVER_METRIC_TOTAL:
			result = "Total";
			break;

		case SERVER_METRIC_TRANSACTION:
			result = "Transaction";
			break;

		case SERVER_METRIC_LENGTH:
		case SERVER_METRIC_UNKNOWN:
		default:
			result = "Unknokwn";
	}

	return result;
}

char *server_getName(Server *server)
{
	char *result = NULL;

	if(server == NULL) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	mutex_lock(&server->mutex);

	result = server->name;

	mutex_unlock(&server->mutex);

	return result;
}

int server_setName(Server *server, char *name)
{
	if((server == NULL) || (name == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&server->mutex);

	if(server->name != NULL) {
		free(server->name);
	}

	server->name = strdup(name);;

	mutex_unlock(&server->mutex);

	return 0;
}

int server_setThreadPoolOptions(Server *server,
		aboolean isThreadPoolShrinkEnabled, int minThreadPoolLength,
		int maxThreadPoolLength)
{
	int ii = 0;

	ServerThread *thread = NULL;

	if((server == NULL) ||
			(minThreadPoolLength < 1) ||
			(maxThreadPoolLength < 1) ||
			(minThreadPoolLength > maxThreadPoolLength)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&server->mutex);

	if(server->state != SERVER_STATE_INIT) {
		mutex_unlock(&server->mutex);

		log_logf(server->log, LOG_LEVEL_ERROR,
				"unable to set thread pool options when not in init mode");

		return -1;
	}

	mutex_unlock(&server->mutex);

	mutex_lock(&server->threads.mutex);

	if(server->threads.list != NULL) {
		free(server->threads.list);
	}

	server->threads.isThreadPoolShrinkEnabled = isThreadPoolShrinkEnabled;
	server->threads.minThreadPoolLength = minThreadPoolLength;
	server->threads.maxThreadPoolLength = maxThreadPoolLength;
	server->threads.currentThreadPoolLength = 0;

	server->threads.list = (ServerThread *)malloc(sizeof(ServerThread) *
			server->threads.maxThreadPoolLength);

	for(ii = 0; ii < server->threads.maxThreadPoolLength; ii++) {
		thread = &(server->threads.list[ii]);

		thread->state = THREAD_STATE_INIT;
		thread->status = THREAD_STATUS_INIT;
		thread->signal = THREAD_SIGNAL_OK;
		thread->id = 0;
		thread->ip = NULL;
	}

	mutex_unlock(&server->threads.mutex);

	return 0;
}

int server_setDumpPacketsMode(Server *server, aboolean isEnabled)
{
	if(server == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&server->packets.mutex);

	server->packets.isEnabled = isEnabled;

	mutex_unlock(&server->packets.mutex);

	return 0;
}

aboolean server_isDumpPackets(Server *server)
{
	aboolean result = afalse;

	if(server == NULL) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	mutex_lock(&server->packets.mutex);

	result = server->packets.isEnabled;

	mutex_unlock(&server->packets.mutex);

	return result;
}

int server_setDumpPacketsOptions(Server *server, aboolean useIndividualFiles,
		aboolean usePacketHeaders, char *path, char *filename)
{
	int rc = 0;
	int length = 0;
	char *errorMessage = NULL;
	char *packetFilename = NULL;

	if((server == NULL) || (path == NULL) || (filename == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&server->mutex);

	if(server->manager == NULL) {
		log_logf(server->log, LOG_LEVEL_ERROR,
				"unable to set dump packet options without a transaction "
				"manager context");
		mutex_unlock(&server->mutex);
		return -1;
	}

	if(transactionManager_setPacketDumpFunction(server->manager, server,
				server_dumpPacket) < 0) {
		log_logf(server->log, LOG_LEVEL_ERROR,
				"failed to register server's dump packet function with the "
				"transaction manager");
		mutex_unlock(&server->mutex);
		return -1;
	}


	mutex_unlock(&server->mutex);

	mutex_lock(&server->packets.mutex);

	if(server->packets.path != NULL) {
		free(server->packets.path);
	}

	if(server->packets.filename != NULL) {
		free(server->packets.filename);
	}

	if((server->packets.isEnabled) && (!server->packets.useIndividualFiles)) {
		file_free(&server->packets.handle);
	}

	server->packets.useIndividualFiles = useIndividualFiles;
	server->packets.usePacketHeaders = usePacketHeaders;
	server->packets.path = strdup(path);

	length = strlen(server->packets.path);

	while((server->packets.path[(length - 1)] == '/') ||
			(server->packets.path[(length - 1)] == '\\')) {
		server->packets.path[(length - 1)] = '\0';
		length--;
	}

	server->packets.filename = strdup(filename);

	if(server->packets.useIndividualFiles) {
		mutex_unlock(&server->packets.mutex);
		return 0;
	}

	length = (strlen(server->packets.path) +
			strlen(server->packets.filename) +
			8);

	packetFilename = (char *)malloc(sizeof(char) * (length + 1));

	snprintf(packetFilename, (sizeof(char) * length), "%s/%s",
			server->packets.path, server->packets.filename);

	if((rc = file_init(&server->packets.handle, packetFilename, "wb+",
					16384)) < 0) {
		errorMessage = file_getErrorMessage(&server->packets.handle, rc);

		log_logf(server->log, LOG_LEVEL_ERROR,
				"failed to open packet file '%s' with '%s'",
				packetFilename, errorMessage);

		free(errorMessage);

		file_free(&server->packets.handle);

		server->packets.isEnabled = afalse;
	}

	free(packetFilename);

	mutex_unlock(&server->packets.mutex);

	if(rc < 0) {
		return -1;
	}

	return 0;
}

int server_setTransactionManager(Server *server, TransactionManager *manager)
{
	if((server == NULL) || (manager == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&server->mutex);

	if(server->state != SERVER_STATE_INIT) {
		mutex_unlock(&server->mutex);

		log_logf(server->log, LOG_LEVEL_ERROR,
				"unable to set transaction manager when not in init mode");

		return -1;
	}

	server->manager = manager;

	mutex_unlock(&server->mutex);

	return 0;
}

char *server_getSocketHostname(Server *server)
{
	char *result = NULL;

	if(server == NULL) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	mutex_lock(&server->mutex);

	result = server->socket.hostname;

	mutex_unlock(&server->mutex);

	return result;
}

int server_getSocketPort(Server *server)
{
	int result = 0;

	if(server == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&server->mutex);

	result = server->socket.port;

	mutex_unlock(&server->mutex);

	return result;
}

SocketProtocol server_getSocketProtocol(Server *server)
{
	SocketProtocol result = SOCKET_PROTOCOL_ERROR;

	if(server == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&server->mutex);

	result = server->socket.protocol;

	mutex_unlock(&server->mutex);

	return result;
}

int server_setSocketOptions(Server *server, char *hostname, int port,
		SocketProtocol protocol)
{
	if((server == NULL) || (hostname == NULL) || (port < 0)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&server->mutex);

	if(server->state != SERVER_STATE_INIT) {
		mutex_unlock(&server->mutex);

		log_logf(server->log, LOG_LEVEL_ERROR,
				"unable to set socket options when not in init mode");

		return -1;
	}

	if(server->isSocketInit) {
		socket_free(&server->socket);
		server->isSocketInit = afalse;
	}

	if(socket_init(&server->socket,
				SOCKET_MODE_SERVER,
				protocol,
				hostname,
				port) < 0) {
		mutex_unlock(&server->mutex);

		log_logf(server->log, LOG_LEVEL_ERROR,
				"failed to initialize socket");

		return -1;
	}

	server->isSocketInit = atrue;

	mutex_unlock(&server->mutex);

	return 0;
}

Socket *server_getSocket(Server *server)
{
	Socket *result = NULL;

	if(server == NULL) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	mutex_lock(&server->mutex);

	if(!server->isSocketInit) {
		mutex_unlock(&server->mutex);

		log_logf(server->log, LOG_LEVEL_ERROR,
				"socket is not initialized");

		return NULL;
	}

	result = &(server->socket);

	mutex_unlock(&server->mutex);

	return result;
}

Log *server_getLog(Server *server)
{
	Log *result = NULL;

	if(server == NULL) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	mutex_lock(&server->mutex);

	result = server->log;

	mutex_unlock(&server->mutex);

	return result;
}

int server_start(Server *server)
{
	if(server == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&server->mutex);

	if((server->state != SERVER_STATE_INIT) ||
			(!server->isSocketInit) ||
			(server->threads.list == NULL) ||
			(server->manager == NULL)) {
		mutex_unlock(&server->mutex);

		log_logf(server->log, LOG_LEVEL_ERROR,
				"unable to start server in current state");

		return -1;
	}

	if(socket_open(&server->socket) < 0) {
		mutex_unlock(&server->mutex);

		log_logf(server->log, LOG_LEVEL_ERROR,
				"failed to open '%s' socket on '%s:%i'",
				socket_socketProtocolToString(server->socket.protocol),
				server->socket.hostname,
				server->socket.port);

		return -1;
	}

	if(server->socket.protocol == SOCKET_PROTOCOL_TCPIP) {
		if(socket_listen(&server->socket) < 0) {
			mutex_unlock(&server->mutex);

			log_logf(server->log, LOG_LEVEL_ERROR,
					"failed to listen on open socket");

			return -1;
		}
	}

	log_logf(server->log, LOG_LEVEL_INFO,
			"server '%s' started on '%s:%i' using protocol '%s'",
			server->name,
			server->socket.hostname,
			server->socket.port,
			socket_socketProtocolToString(server->socket.protocol));

	server->state = SERVER_STATE_RUNNING;

	if(tmf_spawnThread(&server->tmf,
				manageServerThread,
				(void *)server,
				&server->managerThreadId) < 0) {
		server->state = SERVER_STATE_ERROR;
		mutex_unlock(&server->mutex);

		log_logf(server->log, LOG_LEVEL_ERROR,
				"failed to spawn manager thread");

		return -1;
	}

	server->startTimestamp = time_getTimeMus();

	mutex_unlock(&server->mutex);

	return 0;
}

int server_stop(Server *server)
{
	int ii = 0;

	if(server == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&server->mutex);

	if(server->state != SERVER_STATE_RUNNING) {
		mutex_unlock(&server->mutex);

		log_logf(server->log, LOG_LEVEL_ERROR,
				"unable to stop server in current state");

		return -1;
	}

	log_logf(server->log, LOG_LEVEL_INFO, "stopping server");

	server->state = SERVER_STATE_SHUTDOWN;

	socket_requestShutdown(&server->socket);

	mutex_unlock(&server->mutex);

	time_usleep(8192);

	log_logf(server->log, LOG_LEVEL_INFO, "stopping thread pool");

	for(ii = 0; ii < server->threads.maxThreadPoolLength; ii++) {
		if(server->threads.list[ii].state == THREAD_STATE_RUNNING) {
			tmf_signalThread(&server->tmf, server->threads.list[ii].id,
					THREAD_SIGNAL_QUIT);
		}
	}

	time_usleep(8192);

	for(ii = 0; ii < server->threads.maxThreadPoolLength; ii++) {
		if(server->threads.list[ii].state == THREAD_STATE_RUNNING) {
			tmf_stopThread(&server->tmf, server->threads.list[ii].id);
		}
	}

	time_usleep(8192);

	log_logf(server->log, LOG_LEVEL_INFO, "evicting thread pool");

	if(tmf_free(&server->tmf) < 0) {
		mutex_lock(&server->mutex);
		server->state = SERVER_STATE_ERROR;
		mutex_unlock(&server->mutex);

		log_logf(server->log, LOG_LEVEL_ERROR,
				"failed to shut down thread pool");

		return -1;
	}

	log_logf(server->log, LOG_LEVEL_INFO, "thread pool shutdown");

	mutex_lock(&server->mutex);

	tmf_init(&server->tmf);

	log_logf(server->log, LOG_LEVEL_INFO, "closing socket");

	if(socket_close(&server->socket) < 0) {
		log_logf(server->log, LOG_LEVEL_ERROR,
				"failed to close server socket");

		server->state = SERVER_STATE_ERROR;
	}
	else {
		server->state = SERVER_STATE_STOPPED;
	}

	mutex_unlock(&server->mutex);

	return 0;
}

ArrayList *server_getStatus(Server *server)
{
	int ii = 0;
	int id = 0;
	int currentThreadCount = 0;
	int activeThreadCount = 0;
	alint transactionsExecuted = 0;
	double elapsedTime = 0.0;
	double totalTransactionTime = 0;
	char key[1024];
	char name[1024];
	char value[1024];
	char description[1024];

	ArrayList *result = NULL;
	ServerStatus *entry = NULL;

	if(server == NULL) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	mutex_lock(&server->mutex);

	if(server->state != SERVER_STATE_RUNNING) {
		mutex_unlock(&server->mutex);

		log_logf(server->log, LOG_LEVEL_ERROR,
				"unable to stop server in current state");

		return NULL;
	}

	mutex_unlock(&server->mutex);

	result = (ArrayList *)malloc(sizeof(ArrayList));

	arraylist_init(result);
	arraylist_setFreeFunction(result, freeServerStatus);

	// server state

	entry = newServerStatus("serverState",
			"Server State",
			server_getServerStateString(server->state),
			"The current state of the server.");
	arraylist_add(result, entry);

	// server version & build date

	entry = newServerStatus("serverVersion",
			"Server Version",
			ASGARD_VERSION,
			"The current version of the server.");
	arraylist_add(result, entry);

	entry = newServerStatus("serverBuildDate",
			"Server Build Date",
			ASGARD_DATE,
			"The build date of the server.");
	arraylist_add(result, entry);

	// server uptime

	elapsedTime = time_getElapsedMusInSeconds(server->startTimestamp);
	snprintf(value, ((int)sizeof(value) - 1), "%0.6f", elapsedTime);

	entry = newServerStatus("serverUpTime",
			"Server Up Time",
			value,
			"The amount of the time the server has been running in seconds.");
	arraylist_add(result, entry);

	// server name

	entry = newServerStatus("serverName",
			"Server Name",
			server->name,
			"The name of the server instance.");
	arraylist_add(result, entry);

	// server errors

	mutex_lock(&server->errors.mutex);

	for(ii = 0; ii < SERVER_ERROR_LENGTH; ii++) {
		switch(ii) {
			case SERVER_ERROR_TOTAL:
				strcpy(key, "errorTotal");
				break;

			case SERVER_ERROR_SOCKET_ACCEPT:
				strcpy(key, "errorSocketAccept");
				break;

			case SERVER_ERROR_SOCKET_READ:
				strcpy(key, "errorSocketRead");
				break;

			case SERVER_ERROR_SOCKET_WRITE:
				strcpy(key, "errorSocketWrite");
				break;

			case SERVER_ERROR_SOCKET_CLOSE:
				strcpy(key, "errorSocketClose");
				break;

			case SERVER_ERROR_SOCKET_TIMEOUT:
				strcpy(key, "errorSocketTimeout");
				break;

			case SERVER_ERROR_TRANSACTION:
				strcpy(key, "errorTransaction");
				break;

			case SERVER_ERROR_LENGTH:
			case SERVER_ERROR_UNKNOWN:
			default:
				strcpy(key, "errorUnknown");
		}

		snprintf(name, ((int)sizeof(name) - 1), "%s Errors",
				server_getServerErrorString((ServerErrors)ii));

		snprintf(value, ((int)sizeof(value) - 1), "%lli",
				server->errors.counter[ii]);

		entry = newServerStatus(key, name, value,
				"An error tracked by the server instance.");

		arraylist_add(result, entry);
	}

	mutex_unlock(&server->errors.mutex);

	// server metrics

	mutex_lock(&server->metrics.mutex);

	for(ii = 0; ii < SERVER_METRIC_LENGTH; ii++) {
		switch(ii) {
			case SERVER_METRIC_TOTAL:
				strcpy(key, "metricTotalCount");
				break;

			case SERVER_METRIC_TRANSACTION:
				strcpy(key, "metricTransactionCount");
				break;

			case SERVER_METRIC_LENGTH:
			case SERVER_METRIC_UNKNOWN:
			default:
				strcpy(key, "metricUnknownCount");
		}

		snprintf(name, ((int)sizeof(name) - 1), "Metric %s Count",
				server_getServerMetricString((ServerMetrics)ii));

		snprintf(value, ((int)sizeof(value) - 1), "%lli",
				server->metrics.counter[ii]);

		entry = newServerStatus(key, name, value,
				"A total count metric tracked by the server instance.");

		arraylist_add(result, entry);

		switch(ii) {
			case SERVER_METRIC_TOTAL:
				strcpy(key, "metricTotalTiming");
				break;

			case SERVER_METRIC_TRANSACTION:
				strcpy(key, "metricTransactionTiming");
				break;

			case SERVER_METRIC_LENGTH:
			case SERVER_METRIC_UNKNOWN:
			default:
				strcpy(key, "metricUnknownTiming");
		}

		snprintf(name, ((int)sizeof(name) - 1), "Metric %s Timing",
				server_getServerMetricString((ServerMetrics)ii));

		snprintf(value, ((int)sizeof(value) - 1), "%0.6f",
				server->metrics.timings[ii]);

		entry = newServerStatus(key, name, value,
				"A total timing metric tracked by the server instance.");

		arraylist_add(result, entry);

		switch(ii) {
			case SERVER_METRIC_TOTAL:
				strcpy(key, "metricTotalAvg");
				break;

			case SERVER_METRIC_TRANSACTION:
				strcpy(key, "metricTransactionAvg");
				break;

			case SERVER_METRIC_LENGTH:
			case SERVER_METRIC_UNKNOWN:
			default:
				strcpy(key, "metricUnknownAvg");
		}

		snprintf(name, ((int)sizeof(name) - 1), "Metric %s Average",
				server_getServerMetricString((ServerMetrics)ii));

		snprintf(value, ((int)sizeof(value) - 1), "%0.6f",
				(server->metrics.timings[ii] /
				 (double)server->metrics.counter[ii]));

		entry = newServerStatus(key, name, value,
				"An averaged performance metric tracked by the server "
				"instance.");

		arraylist_add(result, entry);
	}

	mutex_unlock(&server->metrics.mutex);

	// server tracking

	mutex_lock(&server->tracking.mutex);

	snprintf(value, ((int)sizeof(value) - 1), "%lli",
			(alint)server->tracking.countAverage);

	snprintf(description, ((int)sizeof(description) - 1),
			"A %i-second running average count metric, per-second.",
			SERVER_TRACKING_ENTRY_LENGTH);

	entry = newServerStatus(
			"metricTransactionRunningAverageCount",
			"Metric Transaction Running Average Count",
			value,
			description);

	arraylist_add(result, entry);

	snprintf(value, ((int)sizeof(value) - 1), "%0.6f",
			server->tracking.timingAverage);

	snprintf(description, ((int)sizeof(description) - 1),
			"A %i-second running average timing metric, per-second.",
			SERVER_TRACKING_ENTRY_LENGTH);

	entry = newServerStatus(
			"metricTransactionRunningAverageTiming",
			"Metric Transaction Running Average Timing",
			value,
			description);

	arraylist_add(result, entry);

	mutex_unlock(&server->tracking.mutex);

	// server threads

	mutex_lock(&server->threads.mutex);

	for(ii = 0; ii < server->threads.maxThreadPoolLength; ii++) {
		snprintf(key, ((int)sizeof(key) - 1), "thread%03iState", ii);

		snprintf(name, ((int)sizeof(name) - 1), "Thread %03i State", ii);

		snprintf(description, ((int)sizeof(description) - 1),
				"The current state for server thread %03i.", ii);

		entry = newServerStatus(
				key,
				name,
				tmf_threadStateToString(server->threads.list[ii].state),
				description);

		arraylist_add(result, entry);

		snprintf(key, ((int)sizeof(key) - 1), "thread%03iStatus", ii);

		snprintf(name, ((int)sizeof(name) - 1), "Thread %03i Status", ii);

		snprintf(description, ((int)sizeof(description) - 1),
				"The current status for server thread %03i.", ii);

		entry = newServerStatus(
				key,
				name,
				tmf_threadStatusToString(server->threads.list[ii].status),
				description);

		arraylist_add(result, entry);

		snprintf(key, ((int)sizeof(key) - 1), "thread%03iSignal", ii);

		snprintf(name, ((int)sizeof(name) - 1), "Thread %03i Signal", ii);

		snprintf(description, ((int)sizeof(description) - 1),
				"The current signal for server thread %03i.", ii);

		entry = newServerStatus(
				key,
				name,
				tmf_threadSignalToString(server->threads.list[ii].signal),
				description);

		arraylist_add(result, entry);

		snprintf(key, ((int)sizeof(key) - 1), "thread%03iId", ii);

		snprintf(name, ((int)sizeof(name) - 1), "Thread %03i ID", ii);

		snprintf(value, ((int)sizeof(value) - 1), "%i",
				server->threads.list[ii].id);

		snprintf(description, ((int)sizeof(description) - 1),
				"The thread ID for server thread %03i.", ii);

		entry = newServerStatus(key, name, value, description);

		arraylist_add(result, entry);

		snprintf(key, ((int)sizeof(key) - 1), "thread%03iConnection", ii);

		snprintf(name, ((int)sizeof(name) - 1), "Thread %03i Connection", ii);

		snprintf(description, ((int)sizeof(description) - 1),
				"The current client-connection IP for server thread %03i.", ii);

		if(server->threads.list[ii].ip == NULL) {
			strcpy(value, "n/a");
		}
		else {
			strcpy(value, server->threads.list[ii].ip);
		}

		entry = newServerStatus(key, name, value, description);

		arraylist_add(result, entry);
	}

	mutex_unlock(&server->threads.mutex);

	// server socket packets

	mutex_lock(&server->packets.mutex);

	if(server->packets.isEnabled) {
		strcpy(value, "true");
	}
	else {
		strcpy(value, "false");
	}

	entry = newServerStatus("packetsIsEnabled",
			"Packets Is Enabled",
			value,
			"Is the server packet-recording system enabled.");

	arraylist_add(result, entry);

	if(server->packets.useIndividualFiles) {
		strcpy(value, "true");
	}
	else {
		strcpy(value, "false");
	}

	entry = newServerStatus("packetsUseIndividualFiles",
			"Packets Use Individual Files",
			value,
			"Use individual files in the server packet-recording system.");

	arraylist_add(result, entry);

	if(server->packets.usePacketHeaders) {
		strcpy(value, "true");
	}
	else {
		strcpy(value, "false");
	}

	entry = newServerStatus("packetsUsePacketHeaders",
			"Packets Use Packet Headers",
			value,
			"Use packet headers in the server packet-recording system.");

	arraylist_add(result, entry);

	snprintf(value, ((int)sizeof(value) - 1), "%lli", server->packets.uid);

	entry = newServerStatus("packetsUid",
			"Packets UID",
			value,
			"The server packet-recording system UID value.");

	arraylist_add(result, entry);

	if(server->packets.path != NULL) {
		strcpy(value, server->packets.path);
	}
	else {
		strcpy(value, "");
	}

	entry = newServerStatus("packetsPath",
			"Packets Path",
			value,
			"The server packet-recording system path value.");

	arraylist_add(result, entry);

	if(server->packets.filename != NULL) {
		strcpy(value, server->packets.filename);
	}
	else {
		strcpy(value, "");
	}

	entry = newServerStatus("packetsFilename",
			"Packets File Name",
			value,
			"The server packet-recording system file name value.");

	arraylist_add(result, entry);

	mutex_unlock(&server->packets.mutex);

	// server thread-management framework statistics

	tmf_getCurrentThreadCount(&server->tmf, &currentThreadCount);

	snprintf(value, ((int)sizeof(value) - 1), "%i", currentThreadCount);

	entry = newServerStatus("threadManagementFrameworkCurrentThreadCount",
			"Thread Management Framework Current Thread Count",
			value,
			"The current thread pool length of the server's "
			"thread-management framework (tmf).");

	arraylist_add(result, entry);

	tmf_getActiveThreadCount(&server->tmf, &activeThreadCount);

	snprintf(value, ((int)sizeof(value) - 1), "%i", activeThreadCount);

	entry = newServerStatus("threadManagementFrameworkActiveThreadCount",
			"Thread Management Framework Active Thread Count",
			value,
			"The active thread pool length of the server's "
			"thread-management framework (tmf).");

	arraylist_add(result, entry);

	// transaction manager metrics

	transactionManager_getMetrics(server->manager, &transactionsExecuted,
				&totalTransactionTime);

	snprintf(value, ((int)sizeof(value) - 1), "%lli", transactionsExecuted);

	entry = newServerStatus("transactionManagerExecutions",
			"Transaction Manager Executions",
			value,
			"The number of transaction manager handler executions.");

	arraylist_add(result, entry);

	snprintf(value, ((int)sizeof(value) - 1), "%0.6f", totalTransactionTime);

	entry = newServerStatus("transactionManagerTotalExecutionTime",
			"Transaction Manager Total Execution Time",
			value,
			"The total time of transaction manager handler execution.");

	arraylist_add(result, entry);

	snprintf(value, ((int)sizeof(value) - 1), "%0.6f",
			(totalTransactionTime / (double)transactionsExecuted));

	entry = newServerStatus("transactionManagerAvgExecutionTime",
			"Transaction Manager Average Execution Time",
			value,
			"The average time of transaction manager handler execution.");

	arraylist_add(result, entry);

	// transaction manager errors

	mutex_lock(&server->manager->errors.mutex);

	for(ii = 0; ii < TRANSACTION_ERROR_LENGTH; ii++) {
		if(ii == (TRANSACTION_ERROR_LENGTH - 1)) {
			id = ii;
		}
		else {
			id = ((ii + 1) * -1);
		}

		switch(id) {
			case TRANSACTION_ERROR_INVALID_ARGS:
				strcpy(key, "transactionManagerErrorInvalidArgs");
				break;

			case TRANSACTION_ERROR_READ:
				strcpy(key, "transactionManagerErrorSocketRead");
				break;

			case TRANSACTION_ERROR_FAILED_IDENT:
				strcpy(key, "transactionManagerErrorFailedIdent");
				break;

			case TRANSACTION_ERROR_MISSING_HANDLER:
				strcpy(key, "transactionManagerErrorMissingHandler");
				break;

			case TRANSACTION_ERROR_INVALID_HANDLER:
				strcpy(key, "transactionManagerErrorInvalidHandler");
				break;

			case TRANSACTION_ERROR_NULL_RESULT:
				strcpy(key, "transactionManagerErrorNullResult");
				break;

			case TRANSACTION_ERROR_INVALID_RESULT:
				strcpy(key, "transactionManagerErrorInvalidResult");
				break;

			case TRANSACTION_ERROR_WRITE:
				strcpy(key, "transactionManagerErrorSocketWrite");
				break;

			case TRANSACTION_ERROR_CLOSE:
				strcpy(key, "transactionManagerErrorSocketClose");
				break;

			case TRANSACTION_ERROR_TIMEOUT:
				strcpy(key, "transactionManagerErrorSocketTimeout");
				break;

			case TRANSACTION_ERROR_TOTAL:
				strcpy(key, "transactionManagerErrorTotal");
				break;

			case TRANSACTION_ERROR_LENGTH:
			default:
				strcpy(key, "transactionManagerErrorUnknown");
		}

		snprintf(name, ((int)sizeof(name) - 1),
				"Transaction Manager %s Errors",
				transactionManager_getErrorString((TransactionErrors)id));

		snprintf(value, ((int)sizeof(value) - 1), "%lli",
				server->manager->errors.counter[ii]);

		entry = newServerStatus(key, name, value,
				"An error tracked by the transaction manager.");

		arraylist_add(result, entry);
	}

	mutex_unlock(&server->manager->errors.mutex);

	// server socket

	entry = newServerStatus("socketState",
			"Socket State",
			socket_socketStateToString(server->socket.state),
			"The current state of the socket.");

	arraylist_add(result, entry);

	entry = newServerStatus("socketMode",
			"Socket Mode",
			socket_socketModeToString(server->socket.mode),
			"The current socket mode.");

	arraylist_add(result, entry);

	entry = newServerStatus("socketProtocol",
			"Socket protocol",
			socket_socketProtocolToString(server->socket.protocol),
			"Which protocol the socket is using.");

	arraylist_add(result, entry);

	if(socket_getReuseAddress(&server->socket)) {
		entry = newServerStatus("socketReuseAddress",
				"Socket Reuse Address",
				"true",
				"Will the socket reuse a host address.");
	}
	else {
		entry = newServerStatus("socketReuseAddress",
				"Socket Reuse Address",
				"false",
				"Will the socket reuse a host address.");
	}

	arraylist_add(result, entry);

	if(socket_getReusePort(&server->socket)) {
		entry = newServerStatus("socketReusePort",
				"Socket Reuse Port",
				"true",
				"Will the socket reuse a host port.");
	}
	else {
		entry = newServerStatus("socketReusePort",
				"Socket Reuse Port",
				"false",
				"Will the socket reuse a host port.");
	}

	arraylist_add(result, entry);

	if(socket_getUseBroadcast(&server->socket)) {
		entry = newServerStatus("socketUseBroadcast",
				"Socket Use Broadcast",
				"true",
				"Is the socket in packet-broadcast mode.");
	}
	else {
		entry = newServerStatus("socketUseBroadcast",
				"Socket Use Broadcast",
				"false",
				"Is the socket in packet-broadcast mode.");
	}

	arraylist_add(result, entry);

	if(socket_getUseSocketWriteSync(&server->socket)) {
		entry = newServerStatus("socketUseSocketWriteSync",
				"Socket Use Socket Write Sync",
				"true",
				"Will the socket use write synchronicity.");
	}
	else {
		entry = newServerStatus("socketUseSocketWriteSync",
				"Socket Use Socket Write Sync",
				"false",
				"Will the socket use write synchronicity.");
	}

	arraylist_add(result, entry);

	if(socket_getUseModulatedTimeout(&server->socket)) {
		entry = newServerStatus("socketUseModulatedTimeout",
				"Socket Use Modulated Timeout",
				"true",
				"Will the socket use modulated timeouts.");
	}
	else {
		entry = newServerStatus("socketUseModulatedTimeout",
				"Socket Use Modulated Timeout",
				"false",
				"Will the socket use modulated timeouts.");
	}

	arraylist_add(result, entry);

	snprintf(value, ((int)sizeof(value) - 1), "%i",
			socket_getQueueLength(&server->socket));

	entry = newServerStatus("socketQueueLength",
			"Socket Queue Length",
			value,
			"The number of clients allowed to queue requests.");

	arraylist_add(result, entry);

	snprintf(value, ((int)sizeof(value) - 1), "%i",
			socket_getTimeoutMillis(&server->socket));

	entry = newServerStatus("socketTimeoutMillis",
			"Socket Timeout Millis",
			value,
			"The current socket timeout in milliseconds.");

	arraylist_add(result, entry);

	snprintf(value, ((int)sizeof(value) - 1), "%lli",
			socket_getBytesSent(&server->socket));

	entry = newServerStatus("socketBytesSent",
			"Socket bytes sent",
			value,
			"The number of bytes sent on the socket.");

	arraylist_add(result, entry);

	snprintf(value, ((int)sizeof(value) - 1), "%lli",
			socket_getBytesReceived(&server->socket));

	entry = newServerStatus("socketBytesReceived",
			"Socket bytes received",
			value,
			"The number of bytes recieved from the socket.");

	arraylist_add(result, entry);

	snprintf(value, ((int)sizeof(value) - 1), "%lli",
			socket_getReadsAttempted(&server->socket));

	entry = newServerStatus("socketReadsAttempted",
			"Socket Reads Attempted",
			value,
			"The number of attempted socket reads.");

	arraylist_add(result, entry);

	snprintf(value, ((int)sizeof(value) - 1), "%lli",
			socket_getReadsCompleted(&server->socket));

	entry = newServerStatus("socketReadsCompleted",
			"Socket Reads Completed",
			value,
			"The number of completed socket reads.");

	arraylist_add(result, entry);

	snprintf(value, ((int)sizeof(value) - 1), "%0.2f",
			(((double)socket_getReadsCompleted(&server->socket) /
			  (double)socket_getReadsAttempted(&server->socket)) * 100.0));

	entry = newServerStatus("socketReadSuccess",
			"Socket Read Success %",
			value,
			"The socket read success percentage.");

	arraylist_add(result, entry);

	snprintf(value, ((int)sizeof(value) - 1), "%0.6f",
			(double)socket_getTotalReadTime(&server->socket));

	entry = newServerStatus("socketTotalReadTime",
			"Socket Total Read Time",
			value,
			"The total time spent reading on a socket, in seconds.");

	arraylist_add(result, entry);

	snprintf(value, ((int)sizeof(value) - 1), "%0.6f",
			(socket_getTotalReadTime(&server->socket) /
			 (double)socket_getReadsAttempted(&server->socket)));

	entry = newServerStatus("socketAvgReadTime",
			"Socket Average Read Time",
			value,
			"The average time spent reading on a socket, in seconds.");

	arraylist_add(result, entry);

	snprintf(value, ((int)sizeof(value) - 1), "%lli",
			socket_getWritesAttempted(&server->socket));

	entry = newServerStatus("socketWritesAttempted",
			"Socket Writes Attempted",
			value,
			"The number of attempted socket writes.");

	arraylist_add(result, entry);

	snprintf(value, ((int)sizeof(value) - 1), "%lli",
			socket_getWritesCompleted(&server->socket));

	entry = newServerStatus("socketWritesCompleted",
			"Socket Writes Completed",
			value,
			"The number of completed socket writes.");

	arraylist_add(result, entry);

	snprintf(value, ((int)sizeof(value) - 1), "%0.2f",
			(((double)socket_getWritesCompleted(&server->socket) /
			  (double)socket_getWritesAttempted(&server->socket)) * 100.0));

	entry = newServerStatus("socketWriteSuccess",
			"Socket Write Success %",
			value,
			"The socket write success percentage.");

	arraylist_add(result, entry);

	snprintf(value, ((int)sizeof(value) - 1), "%0.6f",
			(double)socket_getTotalWriteTime(&server->socket));

	entry = newServerStatus("socketTotalWriteTime",
			"Socket Total Write Time",
			value,
			"The total time spent writing on a socket, in seconds.");

	arraylist_add(result, entry);

	snprintf(value, ((int)sizeof(value) - 1), "%0.6f",
			(socket_getTotalWriteTime(&server->socket) /
			 (double)socket_getWritesAttempted(&server->socket)));

	entry = newServerStatus("socketAvgWriteTime",
			"Socket Average Write Time",
			value,
			"The average time spent writing on a socket, in seconds.");

	arraylist_add(result, entry);

	return result;
}

void server_dumpPacket(void *context, char *packet, int packetLength)
{
	int rc = 0;
	int length = 0;
	alint packetId = 0;
	char *errorMessage = NULL;
	char *packetFilename = NULL;
	char buffer[8192];

	Server *server = NULL;
	FileHandle *handle = NULL;
	FileHandle localFileHandle;

	if((context == NULL) || (packet == NULL) || (packetLength < 1)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	server = (Server *)context;

	mutex_lock(&server->mutex);

	if(server->state != SERVER_STATE_RUNNING) {
		mutex_unlock(&server->mutex);

		log_logf(server->log, LOG_LEVEL_ERROR,
				"unable to dump packet in current state");

		return;
	}
	else if(!server->packets.isEnabled) {
		mutex_unlock(&server->mutex);

		log_logf(server->log, LOG_LEVEL_WARNING,
				"dump packet functionality is currently disabled");

		return;
	}

	mutex_unlock(&server->mutex);

	// dump packet logic

	mutex_lock(&server->packets.mutex);
	packetId = server->packets.uid;
	server->packets.uid += 1;
	mutex_unlock(&server->packets.mutex);

	if(server->packets.useIndividualFiles) {
		mutex_lock(&server->packets.mutex);

		length = (strlen(server->packets.path) +
				strlen(server->packets.filename) +
				8);

		packetFilename = (char *)malloc(sizeof(char) * (length + 1));

		snprintf(packetFilename, (sizeof(char) * length),
				"%s/%s.%lli",
				server->packets.path,
				server->packets.filename,
				packetId);

		mutex_unlock(&server->packets.mutex);

		if((rc = file_init(&localFileHandle, packetFilename, "wb+",
						16384)) < 0) {
			errorMessage = file_getErrorMessage(&localFileHandle, rc);

			log_logf(server->log, LOG_LEVEL_ERROR,
					"failed to open packet file '%s' with '%s'",
					packetFilename, errorMessage);

			free(errorMessage);
			free(packetFilename);

			file_free(&localFileHandle);

			return;
		}

		handle = &(localFileHandle);
	}
	else {
		mutex_lock(&server->packets.mutex);
		handle = &(server->packets.handle);
		mutex_unlock(&server->packets.mutex);
	}

	if(server->packets.usePacketHeaders) {
		length = snprintf(buffer, ((int)sizeof(buffer) - 1),
				"----------------------------------------"
				"/ begin packet #%lli /"
				"----------------------------------------"
				"\n",
				packetId);

		mutex_lock(&server->packets.mutex);

		if((rc = file_write(handle, buffer, length)) < 0) {
			errorMessage = file_getErrorMessage(handle, rc);

			log_logf(server->log, LOG_LEVEL_ERROR,
					"failed to write to packet file '%s' with '%s'",
					handle->filename, errorMessage);

			free(errorMessage);
		}

		mutex_unlock(&server->packets.mutex);
	}

	mutex_lock(&server->packets.mutex);

	if((rc = file_write(handle, packet, packetLength)) < 0) {
		errorMessage = file_getErrorMessage(handle, rc);

		log_logf(server->log, LOG_LEVEL_ERROR,
				"failed to write to packet file '%s' with '%s'",
				handle->filename, errorMessage);

		free(errorMessage);
	}

	mutex_unlock(&server->packets.mutex);

	if(server->packets.usePacketHeaders) {
		length = snprintf(buffer, ((int)sizeof(buffer) - 1),
				"\n"
				"----------------------------------------"
				"/ end packet #%lli /"
				"----------------------------------------"
				"\n",
				packetId);

		mutex_lock(&server->packets.mutex);

		if((rc = file_write(handle, buffer, length)) < 0) {
			errorMessage = file_getErrorMessage(handle, rc);

			log_logf(server->log, LOG_LEVEL_ERROR,
					"failed to write to packet file '%s' with '%s'",
					handle->filename, errorMessage);

			free(errorMessage);
		}

		mutex_unlock(&server->packets.mutex);
	}

	if(server->packets.useIndividualFiles) {
		file_free(&localFileHandle);
	}

	return;
}

