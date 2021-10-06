/*
 * socket_windows.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard socket interface, Windows plugin.
 *
 * Written by Josh English.
 */


#if defined(WIN32) && !defined(MINGW)


// preprocessor directives

#include "core/core.h"
#define _CORE_SYSTEM_PLUGIN_COMPONENT
#include "core/system/plugin/socket_windows.h"


// define socket (windows plugin) private constants

#if !defined(MSG_NOSIGNAL)
#	define MSG_NOSIGNAL					0
#endif // MSG_NOSIGNAL


// define socket (windows plugin) private functions

static LPHOSTENT obtainHostEntity(Socket *socket, char *buffer,
		int bufferLength)
{
	int rc = 0;
	int flags = 0;

	LPHOSTENT hostPtr = NULL;

	if((hostPtr = gethostbyname(socket->hostname)) == NULL) {
		strcpy(buffer, "detected gethostbyname() failure");
	}

	return hostPtr;
}

static int setSocketOption(Socket *socket, int type, int option, int value)
{
	WindowsSocket *windowsSocket = (WindowsSocket *)socket->plugin;

	if(setsockopt(windowsSocket->sd,
				type,
				option,
				(const void *)&value,
				sizeof(value)) == -1) {
		return -1;
	}

	return 0;
}


// define socket (windows plugin) public functions

int socket_initWindows(Socket *socket)
{
	WindowsSocket *windowsSocket = NULL;

	if(socket == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	windowsSocket = (WindowsSocket *)malloc(sizeof(WindowsSocket));

	windowsSocket->sd = 0;

	memset((void *)&windowsSocket->ref, 0, (int)(sizeof(windowsSocket->ref)));

	mutex_init(&windowsSocket->mutex);

	socket->plugin = (void *)windowsSocket;

	return 0;
}

void socket_freeWindows(Socket *socket)
{
	WindowsSocket *windowsSocket = NULL;

	if((socket == NULL) || (socket->plugin == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	windowsSocket = (WindowsSocket *)socket->plugin;

	mutex_free(&windowsSocket->mutex);

	free(windowsSocket);
}

int socket_getSocketDescriptorWindows(Socket *socket)
{
	WindowsSocket *windowsSocket = NULL;

	if((socket == NULL) || (socket->plugin == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	windowsSocket = (WindowsSocket *)socket->plugin;

	return windowsSocket->sd;
}

int socket_openWindows(Socket *socketPtr)
{
	int ioctlFlag = 0;
	int counter = 0;
	int bufferLength = 0;
	char buffer[8192];

	LPHOSTENT hostPtr = NULL;
#if defined(SO_LINGER)
	struct linger willLinger;
#endif // SO_LINGER

	WindowsSocket *windowsSocket = NULL;

	if((socketPtr == NULL) || (socketPtr->plugin == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	windowsSocket = (WindowsSocket *)socketPtr->plugin;

	// setup socket reference

	windowsSocket->ref.sin_family = AF_INET;

	if(socketPtr->config.useBroadcast) {
		windowsSocket->ref.sin_addr.s_addr = INADDR_ANY;
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

		if((hostPtr == NULL) || ((LPIN_ADDR)*hostPtr->h_addr_list == NULL)) {
			return -1;
		}

		windowsSocket->ref.sin_addr = *((LPIN_ADDR)*hostPtr->h_addr_list);
	}

	windowsSocket->ref.sin_port = htons(socketPtr->port);

	// initialize the socket

	if(socketPtr->protocol == SOCKET_PROTOCOL_TCPIP) {
		if((windowsSocket->sd = (int)socket(AF_INET, SOCK_STREAM,
						IPPROTO_TCP)) == INVALID_SOCKET) {
			closesocket(windowsSocket->sd);
			return -1;
		}
	}
	else if(socketPtr->protocol == SOCKET_PROTOCOL_UDP) {
		if((windowsSocket->sd = (int)socket(AF_INET, SOCK_DGRAM,
						IPPROTO_UDP)) == INVALID_SOCKET) {
			closesocket(windowsSocket->sd);
			return -1;
		}
	}

	// open the socket

	if((socketPtr->mode == SOCKET_MODE_CLIENT) &&
			(!socketPtr->config.useBroadcast)) {
#if defined(SO_LINGER)
		if(socketPtr->protocol == SOCKET_PROTOCOL_TCPIP) {
			willLinger.l_onoff = 1;

			if(setsockopt(windowsSocket->sd,
						SOL_SOCKET,
						SO_LINGER,
						(const char *)&willLinger,
						sizeof(struct linger)) == -1) {
				closesocket(windowsSocket->sd);
				return -1;
			}
		}
#endif // SO_LINGER

		if(connect(windowsSocket->sd, (LPSOCKADDR)&windowsSocket->ref,
					sizeof(windowsSocket->ref)) == -1) {
			closesocket(windowsSocket->sd);
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

		if(bind(windowsSocket->sd, (LPSOCKADDR)&windowsSocket->ref,
					sizeof(windowsSocket->ref)) == -1) {
			closesocket(windowsSocket->sd);
			return -1;
		}

		if(socketPtr->protocol == SOCKET_PROTOCOL_TCPIP) {
			ioctlFlag = 1;
		}
		else if(socketPtr->protocol == SOCKET_PROTOCOL_UDP) {
			ioctlFlag = 0;
		}

		ioctlsocket(windowsSocket->sd, FIONBIO, (u_long *)&ioctlFlag);
	}

	return 0;
}

int socket_closeWindows(Socket *socket)
{
	WindowsSocket *windowsSocket = NULL;

	if((socket == NULL) || (socket->plugin == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	windowsSocket = (WindowsSocket *)socket->plugin;

	if(closesocket(windowsSocket->sd) < 0) {
		return -1;
	}

	return 0;
}

int socket_closeChildSocketWindows(Socket *socket, int sd)
{
	WindowsSocket *windowsSocket = NULL;

	if((socket == NULL) || (socket->plugin == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	windowsSocket = (WindowsSocket *)socket->plugin;

	if((socket->mode == SOCKET_MODE_SERVER) &&
			(socket->protocol == SOCKET_PROTOCOL_UDP)) {
		/*
		 * TODO: this
		 */
	}

	if(windowsSocket->sd == sd) {
		return -1;
	}

	if(closesocket(sd) < 0) {
		return -1;
	}

	return 0;
}

int socket_listenWindows(Socket *socket)
{
	WindowsSocket *windowsSocket = NULL;

	if((socket == NULL) || (socket->plugin == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	windowsSocket = (WindowsSocket *)socket->plugin;

	if(listen(windowsSocket->sd, socket->config.queueLength) == -1) {
		return -1;
	}

	return 0;
}

int socket_acceptWindows(Socket *socket, int microseconds)
{
	int sd = 0;
	unsigned int addressLength = (unsigned int)sizeof(SOCKADDR_IN);

	struct timeval timeout;

	fd_set fdRead;

	WindowsSocket *windowsSocket = NULL;

	if((socket == NULL) || (socket->plugin == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	windowsSocket = (WindowsSocket *)socket->plugin;

	FD_ZERO(&fdRead);

	while((!socket->config.shutdownRequested) &&
			(!FD_ISSET(windowsSocket->sd, &fdRead))) {
		timeout.tv_sec = 0;
		timeout.tv_usec = microseconds;

		FD_SET(windowsSocket->sd, &fdRead);

		select((windowsSocket->sd + 1), &fdRead, NULL, NULL, &timeout);
	}

	if(socket->config.shutdownRequested) {
		return -1;
	}

	if((sd = (int)accept(windowsSocket->sd,
					(LPSOCKADDR)&windowsSocket->ref,
					&addressLength)) < 0) {
		return -1;
	}

	return sd;
}

char *socket_getPeerNameWindows(Socket *socket, int sd)
{
	unsigned int addressLength = (unsigned int)sizeof(SOCKADDR_IN);
	char *result = NULL;
	char *string = NULL;

	WindowsSocket *windowsSocket = NULL;

	if((socket == NULL) || (socket->plugin == NULL)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	windowsSocket = (WindowsSocket *)socket->plugin;

	if(sd < 0) {
		sd = windowsSocket->sd;
	}

	if(socket->protocol == SOCKET_PROTOCOL_TCPIP) {
		if(getpeername(sd,
					(LPSOCKADDR)&windowsSocket->ref,
					&addressLength) == 0) {
			string = inet_ntoa(windowsSocket->ref.sin_addr);
		}
	}

	if(string == NULL) {
		string = "0.0.0.0";
	}

	result = strdup(string);

	return result;
}

int socket_sendWindows(Socket *socket, int sd, char *value, int valueLength)
{
	int flags = MSG_NOSIGNAL;

	WindowsSocket *windowsSocket = NULL;

	if((socket == NULL) || (socket->plugin == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	windowsSocket = (WindowsSocket *)socket->plugin;

	if(socket->protocol == SOCKET_PROTOCOL_TCPIP) {
		if(send(sd, value, valueLength, flags) == -1) {
			return -1;
		}
	}
	else if(socket->mode == SOCKET_MODE_CLIENT) {
		if(sendto(sd, value, valueLength, flags,
					(LPSOCKADDR)&windowsSocket->ref,
					sizeof(windowsSocket->ref)) == -1) {
			return -1;
		}
	}
	else if(socket->config.useBroadcast) {
		mutex_lock(&windowsSocket->mutex);

		windowsSocket->ref.sin_addr.s_addr = inet_addr(SOCKET_BROADCAST_MASK);

		if(sendto(sd, value, valueLength, flags,
					(LPSOCKADDR)&windowsSocket->ref,
					sizeof(windowsSocket->ref)) == -1) {
			mutex_unlock(&windowsSocket->mutex);
			return -1;
		}

		mutex_unlock(&windowsSocket->mutex);
	}
	else {
		/*
		 * TODO: handle UDP server
		 */
	}

	return 0;
}

int socket_receiveWindows(Socket *socket, int sd, char *value,
		int valueLength, double timeoutSeconds)
{
	int rc = 0;
	int bytesReceived = 0;
	long timeoutInSeconds = 0;
	long timeoutInMicroseconds = 0;
	double timer = 0.0;
	double elapsedTime = 0.0;
	double remainingTime = 0.0;

	fd_set fd;

	struct timeval timeout;

	WindowsSocket *windowsSocket = NULL;

	if((socket == NULL) || (socket->plugin == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	windowsSocket = (WindowsSocket *)socket->plugin;

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

		FD_ZERO(&fd);
		FD_SET(sd, &fd);

		if(socket->protocol == SOCKET_PROTOCOL_TCPIP) {
			if((rc = select((sd + 1), &fd, NULL, NULL, &timeout)) < 0) {
				return -1; // timeout
			}
			else if(rc == 0) {
				if(socket->mode == SOCKET_MODE_SERVER) {
					return -1; // closed socket
				}
				break;
			}
		}
		else if((socket->protocol == SOCKET_PROTOCOL_UDP) &&
				(socket->mode == SOCKET_MODE_CLIENT)) {
			if((rc = select((sd + 1), &fd, NULL, NULL, &timeout)) < 0) {
				return -1; // read error
			}
			else if(rc == 0) {
				return -1; // timeout
			}
		}

		if(setsockopt(windowsSocket->sd,
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
				0);

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


#endif // WIN32 && !MINGW

