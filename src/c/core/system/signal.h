/*
 * signal.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard system-signal interface, header file.
 *
 * Written by Josh English.
 */

#if !defined(_CORE_SYSTEM_SIGNAL_H)

#define _CORE_SYSTEM_SIGNAL_H

#if !defined(_CORE_H) && !defined(_CORE_SYSTEM_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _CORE_H || _CORE_SYSTEM_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// define signal public data types

#if defined(SIGINT) || defined(WIN32)
#	define SIGNAL_INTERRUPT						SIGINT
#else // !defined(SIGINT)
#	define SIGNAL_INTERRUPT						1001
#endif // SIGINT

#if defined(SIGTERM) || defined(WIN32)
#	define SIGNAL_TERMINATE						SIGTERM
#else // !defined(SIGTERM)
#	define SIGNAL_TERMINATE						1002
#endif // SIGTERM

#if defined(SIGSEGV) || defined(WIN32)
#	define SIGNAL_SEGFAULT						SIGSEGV
#else // !defined(SIGSEGV)
#	define SIGNAL_SEGFAULT						1003
#endif // SIGSEGV

#if defined(SIGUSR1)
#	define SIGNAL_USERONE						SIGUSR1
#else // !defined(SIGUSR1)
#	define SIGNAL_USERONE						1004
#endif // SIGUSR1

#if defined(SIGUSR2)
#	define SIGNAL_USERTWO						SIGUSR2
#else // !defined(SIGUSR2)
#	define SIGNAL_USERTWO						1005
#endif // SIGUSR2

#if defined(SIGQUIT)
#	define SIGNAL_QUIT							SIGQUIT
#else // !defined(SIGQUIT)
#	define SIGNAL_QUIT							1006
#endif // SIGQUIT

#if defined(SIGABRT) || defined(WIN32)
#	define SIGNAL_ABORT							SIGABRT
#else // !defined(SIGABRT)
#	define SIGNAL_ABORT							1007
#endif // SIGABRT

#if defined(SIGFPE) || defined(WIN32)
#	define SIGNAL_FLOATING_POINT_EXCEPTION		SIGFPE
#else // !defined(SIGFPE)
#	define SIGNAL_FLOATING_POINT_EXCEPTION		1008
#endif // SIGFPE

#if defined(SIGILL) || defined(WIN32)
#	define SIGNAL_ILLEGAL						SIGILL
#else // !defined(SIGILL)
#	define SIGNAL_ILLEGAL						1009
#endif // SIGILL

typedef void (*SignalActionFunction)(int signalType);


// delcare signal public functions

int signal_registerDefault();

int signal_executeSignal(int signalType);

int signal_registerAction(int signalType, SignalActionFunction actionFunction);

void signal_defaultHandler(int signalType);


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _CORE_SYSTEM_SIGNAL_H

