/*
 * socket_linux.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard socket interface, Linux plugin, header file.
 *
 * Written by Josh English.
 */

#if !defined(_CORE_SYSTEM_PLUGIN_SOCKET_LINUX_H)

#define _CORE_SYSTEM_PLUGIN_SOCKET_LINUX_H

#if !defined(_CORE_H) && !defined(_CORE_SYSTEM_PLUGIN_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _CORE_H || _CORE_SYSTEM_PLUGIN_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


#if defined(__linux__) || defined(__APPLE__) || defined(MINGW)


// define socket (linux plugin) public data types

typedef struct _LinuxSocket {
	int sd;
	struct sockaddr_in ref;
	Mutex mutex;
} LinuxSocket;


// delcare socket (linux plugin) public functions

int socket_initLinux(Socket *socket);

void socket_freeLinux(Socket *socket);

int socket_getSocketDescriptorLinux(Socket *socket);

int socket_openLinux(Socket *socketPtr);

int socket_closeLinux(Socket *socket);

int socket_closeChildSocketLinux(Socket *socket, int sd);

int socket_listenLinux(Socket *socket);

int socket_acceptLinux(Socket *socket, int microseconds);

char *socket_getPeerNameLinux(Socket *socket, int sd);

int socket_sendLinux(Socket *socket, int sd, char *value, int valueLength);

int socket_receiveLinux(Socket *socket, int sd, char *value,
		int valueLength, double timeoutSeconds);


#endif // __linux__ || __APPLE__ || MINGW


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _CORE_SYSTEM_PLUGIN_SOCKET_LINUX_H

