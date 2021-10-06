/*
 * mutex.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard mutex interface, header file.
 *
 * Written by Josh English.
 */

#if !defined(_CORE_SYSTEM_MUTEX_H)

#define _CORE_SYSTEM_MUTEX_H

#if !defined(_CORE_H) && !defined(_CORE_SYSTEM_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _CORE_H || _CORE_SYSTEM_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

// define mutex public data types

typedef struct _Mutex {
	int lockCount;
	int lockerLine;
	char *lockerFile;
	char *lockerFunction;
	double lockerTime;
	void *lock;
	void *dataLock;
} Mutex;

// delcare mutex public functions

int mutex_init(Mutex *mutex);

int mutex_free(Mutex *mutex);

int __mutex_lock(Mutex *mutex, const char *file, const char *function,
		size_t line);

int __mutex_unlock(Mutex *mutex, const char *file, const char *function,
		size_t line);

int mutex_getLockCount(Mutex *mutex, int *lockCount);

int mutex_getLockerInfo(Mutex *mutex, char **file, char **function, int *line);

// define mutex public macros

#define mutex_lock(mutex) \
	__mutex_lock(mutex, __FILE__, __FUNCTION__, __LINE__)

#define mutex_unlock(mutex) \
	__mutex_unlock(mutex, __FILE__, __FUNCTION__, __LINE__)

#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _CORE_SYSTEM_MUTEX_H

