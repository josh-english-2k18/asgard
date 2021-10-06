/*
 * signal.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard system-signal interface.
 *
 * Written by Josh English.
 */

// preprocessor directives

#define _CORE_COMMON_BYPASS_STD_STREAM
#include "core/core.h"
#define _CORE_SYSTEM_COMPONENT
#include "core/system/signal.h"
#define _CORE_SYSTEM_PLUGIN_COMPONENT
#include "core/system/plugin/signal_linux.h"
#include "core/system/plugin/signal_windows.h"

// define signal public functions

int signal_registerDefault()
{
#if defined(__linux__) || defined(__APPLE__)
	return signal_registerDefaultLinux();
#elif defined(WIN32)
	return signal_registerDefaultWindows();
#else // - no plugin available -
	return -1;
#endif // - plugins -
}

int signal_executeSignal(int signalType)
{
#if defined(__linux__) || defined(__APPLE__)
	return signal_executeSignalLinux(signalType);
#elif defined(WIN32)
	return signal_executeSignalWindows(signalType);
#else // - no plugin available -
	return -1;
#endif // - plugins -
}

int signal_registerAction(int signalType, SignalActionFunction actionFunction)
{
#if defined(__linux__) || defined(__APPLE__)
	return signal_registerActionLinux(signalType, actionFunction);
#elif defined(WIN32)
	return signal_registerActionWindows(signalType, actionFunction);
#else // - no plugin available -
	return -1;
#endif // - plugins -
}

void signal_defaultHandler(int signalType)
{
	int stackStringLength = 0;
	char *stackString = NULL;

	if(EXPLICIT_ERRORS) {
		fprintf(stderr, "\n\n\nSIGNAL: received signal #%i ", signalType);

		switch(signalType) {
			case SIGNAL_INTERRUPT:
				fprintf(stderr, "(Interrupt)");
				break;
			case SIGNAL_TERMINATE:
				fprintf(stderr, "(Terminate)");
				break;
			case SIGNAL_SEGFAULT:
				fprintf(stderr, "(Segfault)");
				break;
			case SIGNAL_USERONE:
				fprintf(stderr, "(User One)");
				return;
			case SIGNAL_USERTWO:
				fprintf(stderr, "(User Two)");
				return;
			case SIGNAL_QUIT:
				fprintf(stderr, "(Quit)");
				break;
			case SIGNAL_ABORT:
				fprintf(stderr, "(Abort)");
				break;
			case SIGNAL_FLOATING_POINT_EXCEPTION:
				fprintf(stderr, "(Floating Point Exception)");
				break;
			case SIGNAL_ILLEGAL:
				fprintf(stderr, "(Illegal Instruction)");
				break;
			default:
				fprintf(stderr, "(Unknown)");
		}

		fprintf(stderr, "\n\n");

		if(stacktrace_getStack(&stackString, &stackStringLength) < 0) {
			fprintf(stderr, "error - unable to obtain valid stack trace.\n");
		}
		else {
			fprintf(stderr, "%s", stackString);
			free(stackString);
		}
	}
}

