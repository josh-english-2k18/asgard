/*
 * signal_windows.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard system-signal interface, Windows plugin, header file.
 *
 * Written by Josh English.
 */

#if !defined(_CORE_SYSTEM_PLUGIN_SIGNAL_WINDOWS_H)

#define _CORE_SYSTEM_PLUGIN_SIGNAL_WINDOWS_H

#if !defined(_CORE_H) && !defined(_CORE_SYSTEM_PLUGIN_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _CORE_H || _CORE_SYSTEM_PLUGIN_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

#if defined(WIN32)

// delcare signal (windows plugin) public functions

int signal_registerDefaultWindows();

int signal_executeSignalWindows(int signalType);

int signal_registerActionWindows(int signalType,
		SignalActionFunction actionFunction);

#endif // WIN32

#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _CORE_SYSTEM_PLUGIN_SIGNAL_WINDOWS_H

