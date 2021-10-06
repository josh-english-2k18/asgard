/*
 * tmf.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard core library thread management framework, header file.
 *
 * Written by Josh English.
 */

#if !defined(_CORE_LIBRARY_SYSTEM_TMF_H)

#define _CORE_LIBRARY_SYSTEM_TMF_H

#if !defined(_CORE_H) && !defined(_CORE_LIBRARY_H) && \
		!defined(_CORE_LIBRARY_SYSTEM_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _CORE_H || _CORE_LIBRARY_H || _CORE_LIBRARY_SYSTEM_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// delcare thread management framework public constants

typedef enum _ThreadState {
	THREAD_STATE_INIT = 1,
	THREAD_STATE_RUNNING,
	THREAD_STATE_HALT,
	THREAD_STATE_STOPPED,
	THREAD_STATE_ERROR = -1
} ThreadState;

typedef enum _ThreadStatus {
	THREAD_STATUS_INIT = 1,
	THREAD_STATUS_BUSY,
	THREAD_STATUS_NAPPING,
	THREAD_STATUS_ERROR = -1
} ThreadStatus;

typedef enum _ThreadSignal {
	THREAD_SIGNAL_OK = 1,
	THREAD_SIGNAL_QUIT,
	THREAD_SIGNAL_PANIC_STOP,
	THREAD_SIGNAL_ERROR = -1
} ThreadSignal;


// define thread management framework public data types

typedef void *(*TmfThreadFunction)(void *threadContext, void *argument);

typedef struct _TmfThread {
	int uid;
	int systemId;
	ThreadState state;
	ThreadStatus status;
	ThreadSignal signal;
	TmfThreadFunction function;
	Mutex *mutex;
	void *argument;
} TmfThread;

typedef struct _TmfContext {
	int threadPoolSize;
	TmfThread **threads;
	Thread thread;
	Mutex mutex;
} TmfContext;


// declare thread management framework public functions

int tmf_init(TmfContext *tmf);

int tmf_free(TmfContext *tmf);

char *tmf_threadStateToString(ThreadState threadState);

char *tmf_threadStatusToString(ThreadStatus threadStatus);

char *tmf_threadSignalToString(ThreadSignal threadSignal);

int tmf_getThreadPoolSize(TmfContext *tmf, int *threadPoolSize);

int tmf_getCurrentThreadCount(TmfContext *tmf, int *currentThreadCount);

int tmf_getActiveThreadCount(TmfContext *tmf, int *activeThreadCount);

int tmf_spawnThread(TmfContext *tmf, TmfThreadFunction function,
		void *argument, int *threadId);

int tmf_stopThread(TmfContext *tmf, int threadId);

int tmf_signalThread(TmfContext *tmf, int threadId, int signal);


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _CORE_LIBRARY_SYSTEM_TMF_H

