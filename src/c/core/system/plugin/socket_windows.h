/*
 * socket_windows.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard socket interface, Windows plugin, header file.
 *
 * Written by Josh English.
 */

#if !defined(_CORE_SYSTEM_PLUGIN_SOCKET_WINDOWS_H)

#define _CORE_SYSTEM_PLUGIN_SOCKET_WINDOWS_H

#if !defined(_CORE_H) && !defined(_CORE_SYSTEM_PLUGIN_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _CORE_H || _CORE_SYSTEM_PLUGIN_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


#if defined(WIN32) && !defined(MINGW)


// define socket (windows plugin) public data types

typedef struct _WindowsSocket {
	int sd;
	struct sockaddr_in ref;
	Mutex mutex;
} WindowsSocket;


// delcare socket (windows plugin) public functions

int socket_initWindows(Socket *socket);

void socket_freeWindows(Socket *socket);

int socket_getSocketDescriptorWindows(Socket *socket);

int socket_openWindows(Socket *socketPtr);

int socket_closeWindows(Socket *socket);

int socket_closeChildSocketWindows(Socket *socket, int sd);

int socket_listenWindows(Socket *socket);

int socket_acceptWindows(Socket *socket, int microseconds);

char *socket_getPeerNameWindows(Socket *socket, int sd);

int socket_sendWindows(Socket *socket, int sd, char *value, int valueLength);

int socket_receiveWindows(Socket *socket, int sd, char *value,
		int valueLength, double timeoutSeconds);


#endif // WIN32 && !MINGW


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _CORE_SYSTEM_PLUGIN_SOCKET_WINDOWS_H

