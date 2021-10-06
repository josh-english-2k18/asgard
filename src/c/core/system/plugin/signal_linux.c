/*
 * signal_linux.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard system-signal interface, Linux plugin.
 *
 * Written by Josh English.
 */


#if defined(__linux__) || defined(__APPLE__)


// preprocessor directives

#define _CORE_COMMON_BYPASS_STD_STREAM
#include "core/core.h"
#define _CORE_SYSTEM_PLUGIN_COMPONENT
#include "core/system/plugin/signal_linux.h"


// define signal (linux plugin) public functions

int signal_registerDefaultLinux()
{
	struct sigaction signalAction;

	signalAction.sa_handler = signal_defaultHandler;
	signalAction.sa_flags = SA_RESETHAND;

	if(sigemptyset(&signalAction.sa_mask) != 0) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s():%i] error - failed to initialize the "
					"signal mask with '%s'.\n", __FUNCTION__, __LINE__,
					strerror(errno));
		}
		return -1;
	}

	if(sigaction(SIGNAL_INTERRUPT, &signalAction, NULL) != 0) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s():%i] error - failed to set the signal "
					"action for %i with '%s'.\n", __FUNCTION__, __LINE__,
					SIGNAL_INTERRUPT, strerror(errno));
		}
		return -1;
	}

	if(sigaction(SIGNAL_TERMINATE, &signalAction, NULL) != 0) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s():%i] error - failed to set the signal "
					"action for %i with '%s'.\n", __FUNCTION__, __LINE__,
					SIGNAL_TERMINATE, strerror(errno));
		}
		return -1;
	}

	if(sigaction(SIGNAL_SEGFAULT, &signalAction, NULL) != 0) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s():%i] error - failed to set the signal "
					"action for %i with '%s'.\n", __FUNCTION__, __LINE__,
					SIGNAL_SEGFAULT, strerror(errno));
		}
		return -1;
	}

	if(sigaction(SIGNAL_USERONE, &signalAction, NULL) != 0) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s():%i] error - failed to set the signal "
					"action for %i with '%s'.\n", __FUNCTION__, __LINE__,
					SIGNAL_USERONE, strerror(errno));
		}
		return -1;
	}

	if(sigaction(SIGNAL_USERTWO, &signalAction, NULL) != 0) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s():%i] error - failed to set the signal "
					"action for %i with '%s'.\n", __FUNCTION__, __LINE__,
					SIGNAL_USERTWO, strerror(errno));
		}
		return -1;
	}

	if(sigaction(SIGNAL_QUIT, &signalAction, NULL) != 0) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s():%i] error - failed to set the signal "
					"action for %i with '%s'.\n", __FUNCTION__, __LINE__,
					SIGNAL_QUIT, strerror(errno));
		}
		return -1;
	}

	return 0;
}

int signal_executeSignalLinux(int signalType)
{
	if(raise(signalType) != 0) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s():%i] error - failed to send signal %i to "
					"current process with '%s'.\n", __FUNCTION__, __LINE__,
					signalType, strerror(errno));
		}
		return -1;
	}

	return 0;
}

int signal_registerActionLinux(int signalType,
		SignalActionFunction actionFunction)
{
	struct sigaction signalAction;

	signalAction.sa_handler = actionFunction;
	signalAction.sa_flags = SA_RESETHAND;

	if(sigemptyset(&signalAction.sa_mask) != 0) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s():%i] error - failed to initialize the "
					"signal mask with '%s'.\n", __FUNCTION__, __LINE__,
					strerror(errno));
		}
		return -1;
	}

	if(sigaction(signalType, &signalAction, NULL) != 0) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s():%i] error - failed to set the signal "
					"action for %i with '%s'.\n", __FUNCTION__, __LINE__,
					signalType, strerror(errno));
		}
		return -1;
	}

	return 0;
}


#endif // __linux__ || __APPLE__

