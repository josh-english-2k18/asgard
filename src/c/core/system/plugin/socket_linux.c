/*
 * socket_linux.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard socket interface, Linux plugin.
 *
 * Written by Josh English.
 */


#if defined(__linux__) || defined(__APPLE__) || defined(MINGW)


// preprocessor directives

#include "core/core.h"
#define _CORE_SYSTEM_PLUGIN_COMPONENT
#include "core/system/plugin/socket_linux.h"


// define socket (linux plugin) private constants

#if !defined(MSG_NOSIGNAL)
#	define MSG_NOSIGNAL					0
#endif // MSG_NOSIGNAL

#if defined(__linux__) || defined(__APPLE__)
#	define ADDRLENTYPE							unsigned int
#elif defined(MINGW)
#	define ADDRLENTYPE							int
#	define MSG_WAITALL							0
#else // - no OS defined
#error	"Invalid OS type."
#endif


// define socket (linux plugin) private functions

static struct hostent *obtainHostEntity(Socket *socket,
		char *buffer, int bufferLength)
{
#if defined(__linux__)
	int rc = 0;
	int flags = 0;

	struct hostent host;
#endif // __linux__

	struct hostent *hostPtr = NULL;

#if defined(__linux__) // note: not Apple Compatible
	if((rc = gethostbyname_r(socket->hostname, &host, buffer, bufferLength,
					&hostPtr, &flags)) != 0) {
#endif // __linux__
		if((hostPtr = gethostbyname(socket->hostname)) == NULL) {
			strcpy(buffer, "detected gethostbyname() failure");
		}
#if defined(__linux__) // note: not Apple Compatible
	}
#endif // __linux__

	return hostPtr;
}

static int setSocketOption(Socket *socket, int type, int option, int value)
{
	LinuxSocket *linuxSocket = (LinuxSocket *)socket->plugin;

	if(setsockopt(linuxSocket->sd,
				type,
				option,
				(const void *)&value,
				sizeof(value)) == -1) {
		return -1;
	}

	return 0;
}


// define socket (linux plugin) public functions

int socket_initLinux(Socket *socket)
{
	LinuxSocket *linuxSocket = NULL;

	if(socket == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	linuxSocket = (LinuxSocket *)malloc(sizeof(LinuxSocket));

	linuxSocket->sd = 0;

	memset((void *)&linuxSocket->ref, 0, (int)(sizeof(linuxSocket->ref)));

	mutex_init(&linuxSocket->mutex);

	socket->plugin = (void *)linuxSocket;

	return 0;
}

void socket_freeLinux(Socket *socket)
{
	LinuxSocket *linuxSocket = NULL;

	if((socket == NULL) || (socket->plugin == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	linuxSocket = (LinuxSocket *)socket->plugin;

	mutex_free(&linuxSocket->mutex);

	free(linuxSocket);
}

int socket_getSocketDescriptorLinux(Socket *socket)
{
	LinuxSocket *linuxSocket = NULL;

	if((socket == NULL) || (socket->plugin == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	linuxSocket = (LinuxSocket *)socket->plugin;

	return linuxSocket->sd;
}

int socket_openLinux(Socket *socketPtr)
{
	int counter = 0;
	int bufferLength = 0;
	char buffer[8192];

	struct timeval timeout;
	struct hostent *hostPtr = NULL;

	LinuxSocket *linuxSocket = NULL;

	if((socketPtr == NULL) || (socketPtr->plugin == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	linuxSocket = (LinuxSocket *)socketPtr->plugin;

	// setup socket reference

	linuxSocket->ref.sin_family = AF_INET;

	if(socketPtr->config.useBroadcast) {
		linuxSocket->ref.sin_addr.s_addr = INADDR_ANY;
	}
	else {
		counter = 0;
		hostPtr = NULL;

		bufferLength = (int)(sizeof(buffer) - 1);
		memset(buffer, 0, sizeof(buffer));

		do {
			if((hostPtr = obtainHostEntity(socketPtr, buffer,
							bufferLength)) != NULL) {
				break;
			}

			counter++;

			time_usleep(1024);
		} while((hostPtr == NULL) && (counter < 3));

		if((hostPtr == NULL) || (hostPtr->h_addr == NULL)) {
			return -1;
		}

		linuxSocket->ref.sin_addr.s_addr =
			((struct in_addr *)hostPtr->h_addr)->s_addr;
	}

	linuxSocket->ref.sin_port = htons(socketPtr->port);

	// initialize the socket

	if(socketPtr->protocol == SOCKET_PROTOCOL_TCPIP) {
		if((linuxSocket->sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
			close(linuxSocket->sd);
			return -1;
		}
	}
	else if(socketPtr->protocol == SOCKET_PROTOCOL_UDP) {
		if((linuxSocket->sd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
			close(linuxSocket->sd);
			return -1;
		}
	}

	// open the socket

	if((socketPtr->mode == SOCKET_MODE_CLIENT) &&
			(!socketPtr->config.useBroadcast)) {
		timeout.tv_sec = (long)(socketPtr->config.timeoutMillis / 1000);
		timeout.tv_usec =
			(long)((socketPtr->config.timeoutMillis % 1000) * 1000);

		if(setsockopt(linuxSocket->sd,
					SOL_SOCKET,
					SO_RCVTIMEO,
					(const void *)&timeout,
					sizeof(timeout)) == -1) {
			return -1;
		}

		if(connect(linuxSocket->sd, (struct sockaddr *)&linuxSocket->ref,
					sizeof(linuxSocket->ref)) == -1) {
			close(linuxSocket->sd);
			return -1;
		}
	}
	else {
#if defined(SO_REUSEADDR)
		if(socketPtr->config.reuseAddress) {
			if(setSocketOption(socketPtr, SOL_SOCKET, SO_REUSEADDR, 1) < 0) {
				return -1;
			}
		}
		else {
			if(setSocketOption(socketPtr, SOL_SOCKET, SO_REUSEADDR, 0) < 0) {
				return -1;
			}
		}
#endif // SO_REUSEADDR

#if defined(SO_REUSEPORT)
		if(socketPtr->config.reusePort) {
			if(setSocketOption(socketPtr, SOL_SOCKET, SO_REUSEPORT, 1) < 0) {
				return -1;
			}
		}
		else {
			if(setSocketOption(socketPtr, SOL_SOCKET, SO_REUSEPORT, 0) < 0) {
				return -1;
			}
		}
#endif // SO_REUSEPORT

		if(socketPtr->config.useBroadcast) {
			if(setSocketOption(socketPtr, SOL_SOCKET, SO_BROADCAST, 1) < 0) {
				return -1;
			}
		}

		if(bind(linuxSocket->sd, (struct sockaddr *)&linuxSocket->ref,
					sizeof(linuxSocket->ref)) == -1) {
			close(linuxSocket->sd);
			return -1;
		}
	}

	return 0;
}

int socket_closeLinux(Socket *socket)
{
	LinuxSocket *linuxSocket = NULL;

	if((socket == NULL) || (socket->plugin == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	linuxSocket = (LinuxSocket *)socket->plugin;

	if(close(linuxSocket->sd) < 0) {
		return -1;
	}

	return 0;
}

int socket_closeChildSocketLinux(Socket *socket, int sd)
{
	LinuxSocket *linuxSocket = NULL;

	if((socket == NULL) || (socket->plugin == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	linuxSocket = (LinuxSocket *)socket->plugin;

	if((socket->mode == SOCKET_MODE_SERVER) &&
			(socket->protocol == SOCKET_PROTOCOL_UDP)) {
		/*
		 * TODO: this
		 */
	}

	if(linuxSocket->sd == sd) {
		return -1;
	}

	if(close(sd) < 0) {
		return -1;
	}

	return 0;
}

int socket_listenLinux(Socket *socket)
{
	LinuxSocket *linuxSocket = NULL;

	if((socket == NULL) || (socket->plugin == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	linuxSocket = (LinuxSocket *)socket->plugin;

	if(listen(linuxSocket->sd, socket->config.queueLength) == -1) {
		return -1;
	}

	return 0;
}

int socket_acceptLinux(Socket *socket, int microseconds)
{
	int sd = 0;
	ADDRLENTYPE addressLength = (ADDRLENTYPE)sizeof(struct sockaddr_in);

	struct timeval timeout;

	fd_set fdRead;

	LinuxSocket *linuxSocket = NULL;

	if((socket == NULL) || (socket->plugin == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	linuxSocket = (LinuxSocket *)socket->plugin;

	FD_ZERO(&fdRead);

	while((!socket->config.shutdownRequested) &&
			(!FD_ISSET(linuxSocket->sd, &fdRead))) {
		timeout.tv_sec = 0;
		timeout.tv_usec = microseconds;

		FD_SET(linuxSocket->sd, &fdRead);

		select((linuxSocket->sd + 1), &fdRead, NULL, NULL, &timeout);
	}

	if(socket->config.shutdownRequested) {
		return -1;
	}

	if((sd = accept(linuxSocket->sd,
					(struct sockaddr *)&linuxSocket->ref,
					&addressLength)) < 0) {
		return -1;
	}

	return sd;
}

char *socket_getPeerNameLinux(Socket *socket, int sd)
{
	ADDRLENTYPE addressLength = (ADDRLENTYPE)sizeof(struct sockaddr_in);
	char *result = NULL;
	char *string = NULL;

	LinuxSocket *linuxSocket = NULL;

	if((socket == NULL) || (socket->plugin == NULL)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	linuxSocket = (LinuxSocket *)socket->plugin;

	if(sd < 0) {
		sd = linuxSocket->sd;
	}

	if(socket->protocol == SOCKET_PROTOCOL_TCPIP) {
		if(getpeername(sd,
					(struct sockaddr *)&linuxSocket->ref,
					&addressLength) == 0) {
			string = inet_ntoa(linuxSocket->ref.sin_addr);
		}
	}

	if(string == NULL) {
		string = "0.0.0.0";
	}

	result = strdup(string);

	return result;
}

int socket_sendLinux(Socket *socket, int sd, char *value, int valueLength)
{
	int flags = MSG_NOSIGNAL;

	LinuxSocket *linuxSocket = NULL;

	if((socket == NULL) || (socket->plugin == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	linuxSocket = (LinuxSocket *)socket->plugin;

	if(socket->protocol == SOCKET_PROTOCOL_TCPIP) {
		if(send(sd, value, valueLength, flags) == -1) {
			return -1;
		}
	}
	else if(socket->mode == SOCKET_MODE_CLIENT) {
		if(sendto(sd, value, valueLength, flags,
					(struct sockaddr *)&linuxSocket->ref,
					sizeof(linuxSocket->ref)) == -1) {
			return -1;
		}
	}
	else if(socket->config.useBroadcast) {
		mutex_lock(&linuxSocket->mutex);

		linuxSocket->ref.sin_addr.s_addr = inet_addr(SOCKET_BROADCAST_MASK);

		if(sendto(sd, value, valueLength, flags,
					(struct sockaddr *)&linuxSocket->ref,
					sizeof(linuxSocket->ref)) == -1) {
			mutex_unlock(&linuxSocket->mutex);
			return -1;
		}

		mutex_unlock(&linuxSocket->mutex);
	}
	else {
		/*
		 * TODO: handle UDP server
		 */
	}

	return 0;
}

int socket_receiveLinux(Socket *socket, int sd, char *value,
		int valueLength, double timeoutSeconds)
{
	int rc = 0;
	int bytesReceived = 0;
	long timeoutInSeconds = 0;
	long timeoutInMicroseconds = 0;
	double timer = 0.0;
	double elapsedTime = 0.0;
	double remainingTime = 0.0;

	struct timeval timeout;

	LinuxSocket *linuxSocket = NULL;

	if((socket == NULL) || (socket->plugin == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	linuxSocket = (LinuxSocket *)socket->plugin;

	timer = time_getTimeMus();

	remainingTime = timeoutSeconds;

	do {
		if((remainingTime - (double)((long)remainingTime)) >= 1.0) {
			timeoutInSeconds = (long)(remainingTime -
					(double)((long)remainingTime));
			timeoutInMicroseconds = (long)((remainingTime -
						(double)timeoutInSeconds) * REAL_ONE_MILLION);
		}
		else if((remainingTime - (double)((long)remainingTime)) == 0.0) {
			timeoutInSeconds = 1;
			timeoutInMicroseconds = 0;
		}
		else {
			timeoutInSeconds = 0;
			timeoutInMicroseconds = (long)((remainingTime -
						(double)((long)remainingTime)) * REAL_ONE_MILLION);
		}

		timeout.tv_sec = timeoutInSeconds;
		timeout.tv_usec = timeoutInMicroseconds;

		if(setsockopt(linuxSocket->sd,
					SOL_SOCKET,
					SO_RCVTIMEO,
					(const void *)&timeout,
					sizeof(timeout)) == -1) {
			return -1;
		}

		if(setsockopt(sd,
					SOL_SOCKET,
					SO_RCVTIMEO,
					(const void *)&timeout,
					sizeof(timeout)) == -1) {
			return -1;
		}

		rc = recv(sd,
				(value + bytesReceived),
				(valueLength - bytesReceived),
				MSG_WAITALL);

		if(rc == -1) {
			if(errno == EAGAIN) {
				elapsedTime = time_getElapsedMusInSeconds(timer);
				remainingTime = (timeoutSeconds - elapsedTime);
				continue;
			}
			break;
		}

		bytesReceived += rc;

		elapsedTime = time_getElapsedMusInSeconds(timer);

		remainingTime = (timeoutSeconds - elapsedTime);
	} while((bytesReceived < valueLength) && (elapsedTime < timeoutSeconds));

	return bytesReceived;
}


#endif // __linux__ || __APPLE__ || MINGW

