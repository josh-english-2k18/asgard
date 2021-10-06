/*
 * tmf.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard core library thread management framework.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#define _CORE_LIBRARY_SYSTEM_COMPONENT
#include "core/library/system/tmf.h"

// declare thread management framework private functions

static TmfThread *newTmfThread(TmfContext *tmf, int uid,
		TmfThreadFunction function, void *argument);

static void *threadManagementFunction(void *argument);

// define thread management framework private functions

static TmfThread *newTmfThread(TmfContext *tmf, int uid,
		TmfThreadFunction function, void *argument)
{
	TmfThread *result = NULL;

	result = (TmfThread *)malloc(sizeof(TmfThread));

	result->uid = uid;
	result->systemId = 0;
	result->state = THREAD_STATE_INIT;
	result->status = THREAD_STATUS_INIT;
	result->signal = THREAD_SIGNAL_OK;
	result->function = function;
	result->mutex = &tmf->mutex;
	result->argument = argument;

	return result;
}

static void *threadManagementFunction(void *argument)
{
	void *result = NULL;

	TmfThread *thread = NULL;

	if(argument == NULL) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	thread = (TmfThread *)argument;

	mutex_lock(thread->mutex);

	if(thread_self(&thread->systemId) < 0) {
		thread->state = THREAD_STATE_STOPPED;
		thread->status = THREAD_STATE_ERROR;
		mutex_unlock(thread->mutex);

		fprintf(stderr, "[%s():%i] error - failed to determine thread system "
				"id.\n", __FUNCTION__, __LINE__);
		return NULL;
	}

	thread->state = THREAD_STATE_RUNNING;

	mutex_unlock(thread->mutex);

	while((thread->state == THREAD_STATE_RUNNING) &&
			(thread->signal == THREAD_SIGNAL_OK)) {
		result = thread->function((void *)thread, thread->argument);
	}

	mutex_lock(thread->mutex);

	if(thread->state == THREAD_STATE_HALT) {
		thread->state = THREAD_STATE_STOPPED;
	}
	else {
		thread->state = THREAD_STATE_ERROR;
	}

	mutex_unlock(thread->mutex);

	return result;
}

// define thread management framework public functions

int tmf_init(TmfContext *tmf)
{
	if(tmf == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	memset(tmf, 0, sizeof(TmfContext));

	tmf->threadPoolSize = 0;
	tmf->threads = NULL;

	if(mutex_init(&tmf->mutex) < 0) {
		return -1;
	}

	if(thread_init(&tmf->thread) < 0) {
		return -1;
	}

	return 0;
}

int tmf_free(TmfContext *tmf)
{
	int ii = 0;
	int resultCode = 0;
	void *threadExitValue = NULL;

	if(tmf == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&tmf->mutex);

	if(tmf->threads != NULL) {
		mutex_unlock(&tmf->mutex);

		// stop all active thread(s)

		for(ii = 0; ii < tmf->threadPoolSize; ii++) {
			if(tmf->threads[ii]->state == THREAD_STATE_RUNNING) {
				tmf_stopThread(tmf, ii);
			}
		}

		// free thread state(s)

		mutex_lock(&tmf->mutex);

		for(ii = 0; ii < tmf->threadPoolSize; ii++) {
			if(tmf->threads[ii]->state != THREAD_STATE_STOPPED) {
				fprintf(stderr, "[%s():%i] error - detected thread #%i (%i) "
						"is in state %i, seg-fault warning.\n", __FUNCTION__,
						__LINE__, ii, tmf->threads[ii]->systemId,
						tmf->threads[ii]->state);
				fflush(stderr);
			}
			free(tmf->threads[ii]);
		}

		free(tmf->threads);
	}

	// join thread context

	if(thread_join(&tmf->thread, &threadExitValue) < 0) {
		resultCode = -1;
	}

	mutex_unlock(&tmf->mutex);

	if(mutex_free(&tmf->mutex) < 0) {
		resultCode = -1;
	}

	memset(tmf, 0, sizeof(TmfContext));

	return resultCode;
}

char *tmf_threadStateToString(ThreadState threadState)
{
	char *result = NULL;

	switch(threadState) {
		case THREAD_STATE_INIT:
			result = "Init";
			break;

		case THREAD_STATE_RUNNING:
			result = "Running";
			break;

		case THREAD_STATE_HALT:
			result = "Halt";
			break;

		case THREAD_STATE_STOPPED:
			result = "Stopped";
			break;

		case THREAD_STATE_ERROR:
			result = "Error";
			break;

		default:
			result = "Unknown";
	}

	return result;
}

char *tmf_threadStatusToString(ThreadStatus threadStatus)
{
	char *result = NULL;

	switch(threadStatus) {
		case THREAD_STATUS_INIT:
			result = "Init";
			break;

		case THREAD_STATUS_BUSY:
			result = "Busy";
			break;

		case THREAD_STATUS_NAPPING:
			result = "Napping";
			break;

		case THREAD_STATUS_ERROR:
			result = "Error";
			break;

		default:
			result = "Unknown";
	}

	return result;
}

char *tmf_threadSignalToString(ThreadSignal threadSignal)
{
	char *result = NULL;

	switch(threadSignal) {
		case THREAD_SIGNAL_OK:
			result = "Ok";
			break;

		case THREAD_SIGNAL_QUIT:
			result = "Quit";
			break;

		case THREAD_SIGNAL_PANIC_STOP:
			result = "Panic Stop";
			break;

		case THREAD_SIGNAL_ERROR:
			result = "Error";
			break;

		default:
			result = "Unknown";
	}

	return result;
}

int tmf_getThreadPoolSize(TmfContext *tmf, int *threadPoolSize)
{
	if((tmf == NULL) || (threadPoolSize == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&tmf->mutex);

	*threadPoolSize = tmf->threadPoolSize;

	mutex_unlock(&tmf->mutex);

	return 0;
}

int tmf_getCurrentThreadCount(TmfContext *tmf, int *currentThreadCount)
{
	int ii = 0;

	if((tmf == NULL) || (currentThreadCount == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	*currentThreadCount = 0;

	mutex_lock(&tmf->mutex);

	if(tmf->threads != NULL) {
		for(ii = 0; ii < tmf->threadPoolSize; ii++) {
			if((tmf->threads[ii]->state == THREAD_STATE_RUNNING) &&
					(tmf->threads[ii]->signal == THREAD_SIGNAL_OK)) {
				*currentThreadCount += 1;
			}
		}
	}

	mutex_unlock(&tmf->mutex);

	return 0;
}

int tmf_getActiveThreadCount(TmfContext *tmf, int *activeThreadCount)
{
	int ii = 0;

	if((tmf == NULL) || (activeThreadCount == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	*activeThreadCount = 0;

	mutex_lock(&tmf->mutex);

	if(tmf->threads != NULL) {
		for(ii = 0; ii < tmf->threadPoolSize; ii++) {
			if((tmf->threads[ii]->state == THREAD_STATE_RUNNING) &&
					(tmf->threads[ii]->status == THREAD_STATUS_BUSY) &&
					(tmf->threads[ii]->signal == THREAD_SIGNAL_OK)) {
				*activeThreadCount += 1;
			}
		}
	}

	mutex_unlock(&tmf->mutex);

	return 0;
}

int tmf_spawnThread(TmfContext *tmf, TmfThreadFunction function,
		void *argument, int *threadId)
{
	int ref = 0;
	int counter = 0;

	if((tmf == NULL) || (threadId == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	*threadId = 0;

	mutex_lock(&tmf->mutex);

	if(tmf->threads == NULL) {
		ref = 0;
		tmf->threadPoolSize = 1;
		tmf->threads = (TmfThread **)malloc(sizeof(TmfThread) *
				tmf->threadPoolSize);
	}
	else {
		ref = tmf->threadPoolSize;
		tmf->threadPoolSize += 1;
		tmf->threads = (TmfThread **)realloc(tmf->threads,
				(sizeof(TmfThread) * tmf->threadPoolSize));
	}

	tmf->threads[ref] = newTmfThread(tmf, ref, function, argument);

	if(thread_create(&tmf->thread, threadManagementFunction,
				(void *)tmf->threads[ref]) < 0) {
		mutex_unlock(&tmf->mutex);
		fprintf(stderr, "[%s():%i] error - failed to create thread #%i.\n",
				__FUNCTION__, __LINE__, ref);
		return -1;
	}

	mutex_unlock(&tmf->mutex);

	while((tmf->threads[ref]->state == THREAD_STATE_INIT) && (counter < 8192)) {
		time_usleep(1024);
		counter++;
	}

	if(tmf->threads[ref]->state != THREAD_STATE_RUNNING) {
		fprintf(stderr, "[%s():%i] error - failed to start thread #%i.\n",
				__FUNCTION__, __LINE__, ref);
		return -1;
	}

	*threadId = ref;

	return 0;
}

int tmf_stopThread(TmfContext *tmf, int threadId)
{
	aboolean isThreadActive = afalse;
	int counter = 0;

	if((tmf == NULL) || (threadId < 0) || (threadId >= tmf->threadPoolSize)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&tmf->mutex);

	if(tmf->threads[threadId]->state != THREAD_STATE_RUNNING) {
		mutex_unlock(&tmf->mutex);
		fprintf(stderr, "[%s():%i] error - unable to stop thread #%i, "
				"not running.\n", __FUNCTION__, __LINE__, threadId);
		return -1;
	}

	// set thread state to halt execution

	tmf->threads[threadId]->state = THREAD_STATE_HALT;

	mutex_unlock(&tmf->mutex);

	// take a short break to determine if the thread requires a signal

	counter = 0;
	isThreadActive = atrue;

	while((isThreadActive) && (counter < 16)) {
		time_usleep(1024);

		isThreadActive = afalse;

		mutex_lock(&tmf->mutex);

		if((tmf->threads[threadId]->state != THREAD_STATE_STOPPED) &&
				(tmf->threads[threadId]->state != THREAD_STATE_ERROR)) {
			isThreadActive = atrue;
		}

		mutex_unlock(&tmf->mutex);

		counter++;
	}

	// if thread remains active, signal it to quit

	mutex_lock(&tmf->mutex);

	if((tmf->threads[threadId]->state != THREAD_STATE_STOPPED) &&
			(tmf->threads[threadId]->state != THREAD_STATE_ERROR)) {
		tmf->threads[threadId]->signal = THREAD_SIGNAL_QUIT;
		isThreadActive = atrue;
	}

	mutex_unlock(&tmf->mutex);

	// wait for thread to shutdown

	counter = 0;
	while((isThreadActive) && (counter < 8192)) {
		time_usleep(1024);

		isThreadActive = afalse;

		mutex_lock(&tmf->mutex);

		if((tmf->threads[threadId]->state != THREAD_STATE_STOPPED) &&
				(tmf->threads[threadId]->state != THREAD_STATE_ERROR)) {
			isThreadActive = atrue;
		}

		mutex_unlock(&tmf->mutex);

		counter++;
	}

	// if thread remains active, signal it to panic stop

	if(isThreadActive) {
		isThreadActive = afalse;

		mutex_lock(&tmf->mutex);

		if((tmf->threads[threadId]->state != THREAD_STATE_STOPPED) &&
				(tmf->threads[threadId]->state != THREAD_STATE_ERROR)) {
			tmf->threads[threadId]->signal = THREAD_SIGNAL_PANIC_STOP;
			isThreadActive = atrue;
		}

		mutex_unlock(&tmf->mutex);
	}

	// wait for thread to shutdown

	counter = 0;
	while((isThreadActive) && (counter < 2048)) {
		time_usleep(1024);

		isThreadActive = afalse;

		mutex_lock(&tmf->mutex);

		if((tmf->threads[threadId]->state != THREAD_STATE_STOPPED) &&
				(tmf->threads[threadId]->state != THREAD_STATE_ERROR)) {
			isThreadActive = atrue;
		}

		mutex_unlock(&tmf->mutex);

		counter++;
	}

	if(isThreadActive) {
		fprintf(stderr, "[%s():%i] error - failed to stop thread #%i.\n",
				__FUNCTION__, __LINE__, threadId);
		return -1;
	}

	return 0;
}

int tmf_signalThread(TmfContext *tmf, int threadId, int signal)
{
	if((tmf == NULL) ||
			(threadId < 0) || (threadId >= tmf->threadPoolSize) ||
			((signal != THREAD_SIGNAL_QUIT) &&
			 (signal != THREAD_SIGNAL_PANIC_STOP))) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&tmf->mutex);

	if(tmf->threads[threadId]->state != THREAD_STATE_RUNNING) {
		mutex_unlock(&tmf->mutex);
		fprintf(stderr, "[%s():%i] error - unable to signal thread #%i, "
				"not running.\n", __FUNCTION__, __LINE__, threadId);
		return -1;
	}

	if(tmf->threads[threadId]->signal == THREAD_SIGNAL_ERROR) {
		mutex_unlock(&tmf->mutex);
		fprintf(stderr, "[%s():%i] error - unable to signal thread #%i, "
				"in signal-status '%s'.\n", __FUNCTION__, __LINE__,
				threadId, tmf_threadSignalToString(signal));
		return -1;
	}

	tmf->threads[threadId]->signal = (ThreadSignal)signal;

	mutex_unlock(&tmf->mutex);

	return 0;
}

