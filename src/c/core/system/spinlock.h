/*
 * spinlock.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard spin-lock system for providing enhanced availability to shared
 * resources in a multithreaded environment, header file.
 *
 * Written by Josh English.
 */

#if !defined(_CORE_SYSTEM_SPINLOCK_H)

#define _CORE_SYSTEM_SPINLOCK_H

#if !defined(_CORE_H) && !defined(_CORE_SYSTEM_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _CORE_H || _CORE_SYSTEM_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// define spinlock public constants

typedef enum _SpinlockStatus {
	SPINLOCK_STATUS_AVAILABLE = 1,
	SPINLOCK_STATUS_WAITING,
	SPINLOCK_STATUS_LOCKED,
	SPINLOCK_STATUS_ERROR = -1
} SpinlockStatus;

#define SPINLOCK_DEFAULT_TIMEOUT_MICROSECONDS				2097152


// define spinlock public data types

typedef struct _SpinlockOwner {
	int line;
	int threadId;
	char *file;
	char *function;
	double timestamp;
} SpinlockOwner;

typedef struct _Spinlock {
	int status;
	int threadCount;
	int spinThreshold;
	int timeoutMicroseconds;
	SpinlockOwner owner;
	Mutex mutex;
} Spinlock;


// delcare spinlock public functions

int spinlock_init(Spinlock *spinlock);

int spinlock_free(Spinlock *spinlock);

int spinlock_setTimeout(Spinlock *spinlock, int timeoutMicroseconds);

int __spinlock_writeLock(Spinlock *spinlock, const char *file,
		const char *function, size_t line);

int __spinlock_writeUnlock(Spinlock *spinlock, const char *file,
		const char *function, size_t line);

int __spinlock_readLock(Spinlock *spinlock, const char *file,
		const char *function, size_t line);

int __spinlock_readUnlock(Spinlock *spinlock, const char *file,
		const char *function, size_t line);

int spinlock_getSpinThreadCount(Spinlock *spinlock, int *threadCount);

aboolean spinlock_isLocked(Spinlock *spinlock);

int spinlock_getLockerInfo(Spinlock *spinlock, int *threadId, char **file,
		char **function, int *line, double *lockedSeconds);


// define spinlock public macros

#define spinlock_writeLock(spinlock) \
	__spinlock_writeLock(spinlock, __FILE__, __FUNCTION__, __LINE__)

#define spinlock_writeUnlock(spinlock) \
	__spinlock_writeUnlock(spinlock, __FILE__, __FUNCTION__, __LINE__)

#define spinlock_readLock(spinlock) \
	__spinlock_readLock(spinlock, __FILE__, __FUNCTION__, __LINE__)

#define spinlock_readUnlock(spinlock) \
	__spinlock_readUnlock(spinlock, __FILE__, __FUNCTION__, __LINE__)


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _CORE_SYSTEM_SPINLOCK_H

