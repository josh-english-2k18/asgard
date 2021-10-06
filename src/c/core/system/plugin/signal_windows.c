/*
 * signal_windows.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard system-signal interface, Windows plugin.
 *
 * Written by Josh English.
 */

#if defined(WIN32)

// preprocessor directives

#define _CORE_COMMON_BYPASS_STD_STREAM
#include "core/core.h"
#define _CORE_SYSTEM_PLUGIN_COMPONENT
#include "core/system/plugin/signal_windows.h"

// define signal (windows plugin) public functions

int signal_registerDefaultWindows()
{
	signal(SIGNAL_INTERRUPT, signal_defaultHandler);
	signal(SIGNAL_TERMINATE, signal_defaultHandler);
	signal(SIGNAL_SEGFAULT, signal_defaultHandler);
	signal(SIGNAL_ABORT, signal_defaultHandler);
	signal(SIGNAL_FLOATING_POINT_EXCEPTION, signal_defaultHandler);
	signal(SIGNAL_ILLEGAL, signal_defaultHandler);

	return 0;
}

int signal_executeSignalWindows(int signalType)
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

int signal_registerActionWindows(int signalType,
		SignalActionFunction actionFunction)
{
	if(signal(signalType, actionFunction) == SIG_ERR) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s():%i] error - failed to set the signal "
					"action for %i with '%s'.\n", __FUNCTION__, __LINE__,
					signalType, strerror(errno));
		}
		return -1;
	}

	return 0;
}

#endif // WIN32

