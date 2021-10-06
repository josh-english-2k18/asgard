/*
 * socket.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard socket interface.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#define _CORE_SYSTEM_COMPONENT
#include "core/system/socket.h"
#define _CORE_SYSTEM_PLUGIN_COMPONENT
#include "core/system/plugin/socket_linux.h"
#include "core/system/plugin/socket_windows.h"


// define socket public functions

int socket_init(Socket *socket, SocketMode mode, SocketProtocol protocol,
		char *hostname, int port)
{
	int rc = 0;

	if((socket == NULL) ||
			((mode != SOCKET_MODE_CLIENT) &&
			 (mode != SOCKET_MODE_SERVER)) ||
			((protocol != SOCKET_PROTOCOL_TCPIP) &&
			 (protocol != SOCKET_PROTOCOL_UDP)) ||
			(hostname == NULL) || (port <= 0) || (port >= 65536)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	memset(socket, 0, (int)sizeof(Socket));

	socket->state = SOCKET_STATE_INIT;
	socket->mode = mode;
	socket->protocol = protocol;

	socket->config.reuseAddress = SOCKET_DEFAULT_REUSE_ADDRESS;
	socket->config.reusePort = SOCKET_DEFAULT_REUSE_PORT;
	socket->config.useBroadcast = SOCKET_DEFAULT_USE_BROADCAST;
	socket->config.useSocketWriteSync = SOCKET_DEFAULT_USE_SOCKET_WRITE_SYNC;
	socket->config.useModulatedTimeout = SOCKET_DEFAULT_USE_MODULATED_TIMEOUT;
	socket->config.shutdownRequested = afalse;
	socket->config.queueLength = SOCKET_DEFAULT_QUEUE_LENGTH;
	socket->config.timeoutMillis = SOCKET_DEFAULT_TIMEOUT_MILLIS;

	socket->metrics.bytesSent = 0;
	socket->metrics.bytesReceived = 0;
	socket->metrics.readsAttempted = 0;
	socket->metrics.readsCompleted = 0;
	socket->metrics.writesAttempted = 0;
	socket->metrics.writesCompleted = 0;
	socket->metrics.totalReadTime = 0.0;
	socket->metrics.totalWriteTime = 0.0;

	socket->port = port;
	socket->hostname = strdup(hostname);
	socket->plugin = NULL;

	mutex_init(&socket->config.mutex);
	mutex_init(&socket->metrics.mutex);
	mutex_init(&socket->mutex);

#if defined(__linux__) || defined(__APPLE__) || defined(MINGW)
	rc = socket_initLinux(socket);
#elif defined(WIN32)
	rc = socket_initWindows(socket);
#else // - no plugin available -
	rc = -1;
#endif // - plugins -

	return rc;
}

void socket_free(Socket *socket)
{
	if(socket == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	mutex_lock(&socket->mutex);

	if(socket->hostname != NULL) {
		free(socket->hostname);
	}

#if defined(__linux__) || defined(__APPLE__) || defined(MINGW)
	socket_freeLinux(socket);
#elif defined(WIN32)
	socket_freeWindows(socket);
#else // - no plugin available -
	// do nothing
#endif // - plugins -

	mutex_free(&socket->config.mutex);

	mutex_free(&socket->metrics.mutex);

	mutex_unlock(&socket->mutex);

	mutex_free(&socket->mutex);

	memset(socket, 0, (int)sizeof(Socket));
}

char *socket_socketStateToString(SocketState state)
{
	char *result = NULL;

	switch(state) {
		case SOCKET_STATE_INIT:
			result = "Init";
			break;

		case SOCKET_STATE_OPEN:
			result = "Open";
			break;

		case SOCKET_STATE_CLOSED:
			result = "Closed";
			break;

		case SOCKET_STATE_ERROR:
			result = "Error";
			break;

		default:
			result = "Unknown";
	}

	return result;
}

char *socket_socketModeToString(SocketMode mode)
{
	char *result = NULL;

	switch(mode) {
		case SOCKET_MODE_CLIENT:
			result = "Client";
			break;

		case SOCKET_MODE_SERVER:
			result = "Server";
			break;

		case SOCKET_MODE_ERROR:
		default:
			result = "Unknown";
	}

	return result;
}

char *socket_socketProtocolToString(SocketProtocol protocol)
{
	char *result = NULL;

	switch(protocol) {
		case SOCKET_PROTOCOL_TCPIP:
			result = "TCP/IP";
			break;

		case SOCKET_PROTOCOL_UDP:
			result = "UDP";
			break;

		case SOCKET_PROTOCOL_ERROR:
		default:
			result = "Unknown";
	}

	return result;
}

SocketMode socket_getMode(Socket *socket)
{
	SocketMode result = SOCKET_MODE_ERROR;

	if(socket == NULL) {
		DISPLAY_INVALID_ARGS;
		return SOCKET_MODE_ERROR;
	}

	mutex_lock(&socket->mutex);

	result = socket->mode;

	mutex_unlock(&socket->mutex);

	return result;
}

int socket_setMode(Socket *socket, SocketMode mode)
{
	if((socket == NULL) || (socket->state != SOCKET_STATE_INIT) ||
			((mode != SOCKET_MODE_CLIENT) && (mode != SOCKET_MODE_SERVER))) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&socket->mutex);

	socket->mode = mode;

	mutex_unlock(&socket->mutex);

	return 0;
}

SocketProtocol socket_getProtocol(Socket *socket)
{
	SocketProtocol result = SOCKET_PROTOCOL_ERROR;

	if(socket == NULL) {
		DISPLAY_INVALID_ARGS;
		return SOCKET_PROTOCOL_ERROR;
	}

	mutex_lock(&socket->mutex);

	result = socket->protocol;

	mutex_unlock(&socket->mutex);

	return result;
}

int socket_setProtocol(Socket *socket, SocketProtocol protocol)
{
	if((socket == NULL) || (socket->state != SOCKET_STATE_INIT) ||
			((protocol != SOCKET_PROTOCOL_TCPIP) &&
			 (protocol != SOCKET_PROTOCOL_UDP))) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&socket->mutex);

	socket->protocol = protocol;

	mutex_unlock(&socket->mutex);

	return 0;
}

aboolean socket_getReuseAddress(Socket *socket)
{
	aboolean result = afalse;

	if(socket == NULL) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	mutex_lock(&socket->config.mutex);

	result = socket->config.reuseAddress;

	mutex_unlock(&socket->config.mutex);

	return result;
}

int socket_setReuseAddress(Socket *socket, aboolean value)
{
	if((socket == NULL) || (socket->state != SOCKET_STATE_INIT)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&socket->config.mutex);

	socket->config.reuseAddress = value;

	mutex_unlock(&socket->config.mutex);

	return 0;
}

aboolean socket_getReusePort(Socket *socket)
{
	aboolean result = afalse;

	if(socket == NULL) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	mutex_lock(&socket->config.mutex);

	result = socket->config.reusePort;

	mutex_unlock(&socket->config.mutex);

	return result;
}

int socket_setReusePort(Socket *socket, aboolean value)
{
	if((socket == NULL) || (socket->state != SOCKET_STATE_INIT)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&socket->config.mutex);

	socket->config.reusePort = value;

	mutex_unlock(&socket->config.mutex);

	return 0;
}

aboolean socket_getUseBroadcast(Socket *socket)
{
	aboolean result = afalse;

	if(socket == NULL) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	mutex_lock(&socket->config.mutex);

	result = socket->config.useBroadcast;

	mutex_unlock(&socket->config.mutex);

	return result;
}

int socket_setUseBroadcast(Socket *socket, aboolean value)
{
	if((socket == NULL) || (socket->state != SOCKET_STATE_INIT)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&socket->config.mutex);

	socket->config.useBroadcast = value;

	mutex_unlock(&socket->config.mutex);

	return 0;
}

aboolean socket_getUseSocketWriteSync(Socket *socket)
{
	aboolean result = afalse;

	if(socket == NULL) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	mutex_lock(&socket->config.mutex);

	result = socket->config.useSocketWriteSync;

	mutex_unlock(&socket->config.mutex);

	return result;
}

int socket_setUseSocketWriteSync(Socket *socket, aboolean value)
{
	if(socket == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&socket->config.mutex);

	socket->config.useSocketWriteSync = value;

	mutex_unlock(&socket->config.mutex);

	return 0;
}

aboolean socket_getUseModulatedTimeout(Socket *socket)
{
	aboolean result = afalse;

	if(socket == NULL) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	mutex_lock(&socket->config.mutex);

	result = socket->config.useModulatedTimeout;

	mutex_unlock(&socket->config.mutex);

	return result;
}

int socket_setUseModulatedTimeout(Socket *socket, aboolean value)
{
	if(socket == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&socket->config.mutex);

	socket->config.useModulatedTimeout = value;

	mutex_unlock(&socket->config.mutex);

	return 0;
}

int socket_getQueueLength(Socket *socket)
{
	int result = 0;

	if(socket == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&socket->config.mutex);

	result = socket->config.queueLength;

	mutex_unlock(&socket->config.mutex);

	return result;
}

int socket_setQueueLength(Socket *socket, int value)
{
	if(socket == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&socket->config.mutex);

	socket->config.queueLength = value;

	mutex_unlock(&socket->config.mutex);

	return 0;
}

int socket_getTimeoutMillis(Socket *socket)
{
	int result = 0;

	if(socket == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&socket->config.mutex);

	result = socket->config.timeoutMillis;

	mutex_unlock(&socket->config.mutex);

	return result;
}

int socket_setTimeoutMillis(Socket *socket, int value)
{
	if(socket == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&socket->config.mutex);

	socket->config.timeoutMillis = value;

	mutex_unlock(&socket->config.mutex);

	return 0;
}

alint socket_getBytesSent(Socket *socket)
{
	alint result = 0;

	if(socket == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&socket->metrics.mutex);

	result = socket->metrics.bytesSent;

	mutex_unlock(&socket->metrics.mutex);

	return result;
}

alint socket_getBytesReceived(Socket *socket)
{
	alint result = 0;

	if(socket == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&socket->metrics.mutex);

	result = socket->metrics.bytesReceived;

	mutex_unlock(&socket->metrics.mutex);

	return result;
}

alint socket_getReadsAttempted(Socket *socket)
{
	alint result = 0;

	if(socket == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&socket->metrics.mutex);

	result = socket->metrics.readsAttempted;

	mutex_unlock(&socket->metrics.mutex);

	return result;
}

alint socket_getReadsCompleted(Socket *socket)
{
	alint result = 0;

	if(socket == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&socket->metrics.mutex);

	result = socket->metrics.readsCompleted;

	mutex_unlock(&socket->metrics.mutex);

	return result;
}

alint socket_getWritesAttempted(Socket *socket)
{
	alint result = 0;

	if(socket == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&socket->metrics.mutex);

	result = socket->metrics.writesAttempted;

	mutex_unlock(&socket->metrics.mutex);

	return result;
}

alint socket_getWritesCompleted(Socket *socket)
{
	alint result = 0;

	if(socket == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&socket->metrics.mutex);

	result = socket->metrics.writesCompleted;

	mutex_unlock(&socket->metrics.mutex);

	return result;
}

double socket_getTotalReadTime(Socket *socket)
{
	double result = 0;

	if(socket == NULL) {
		DISPLAY_INVALID_ARGS;
		return 0.0;
	}

	mutex_lock(&socket->metrics.mutex);

	result = socket->metrics.totalReadTime;

	mutex_unlock(&socket->metrics.mutex);

	return result;
}

double socket_getTotalWriteTime(Socket *socket)
{
	double result = 0;

	if(socket == NULL) {
		DISPLAY_INVALID_ARGS;
		return 0.0;
	}

	mutex_lock(&socket->metrics.mutex);

	result = socket->metrics.totalWriteTime;

	mutex_unlock(&socket->metrics.mutex);

	return result;
}

int socket_getSocketDescriptor(Socket *socket)
{
	int rc = 0;

	if((socket == NULL) || (socket->state != SOCKET_STATE_OPEN)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&socket->mutex);

#if defined(__linux__) || defined(__APPLE__) || defined(MINGW)
	rc = socket_getSocketDescriptorLinux(socket);
#elif defined(WIN32)
	rc = socket_getSocketDescriptorWindows(socket);
#else // - no plugin available -
	rc = -1;
#endif // - plugins -

	mutex_unlock(&socket->mutex);

	return rc;
}

int socket_open(Socket *socket)
{
	int rc = 0;

	if((socket == NULL) ||
			((socket->state != SOCKET_STATE_INIT) &&
			 (socket->state != SOCKET_STATE_CLOSED))) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&socket->mutex);

#if defined(__linux__) || defined(__APPLE__) || defined(MINGW)
	rc = socket_openLinux(socket);
#elif defined(WIN32)
	rc = socket_openWindows(socket);
#else // - no plugin available -
	rc = -1;
#endif // - plugins -

	if(rc == 0) {
		socket->state = SOCKET_STATE_OPEN;
	}

	mutex_unlock(&socket->mutex);

	return rc;
}

int socket_close(Socket *socket)
{
	int rc = 0;

	if((socket == NULL) || (socket->state != SOCKET_STATE_OPEN)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&socket->mutex);

#if defined(__linux__) || defined(__APPLE__) || defined(MINGW)
	rc = socket_closeLinux(socket);
#elif defined(WIN32)
	rc = socket_closeWindows(socket);
#else // - no plugin available -
	rc = -1;
#endif // - plugins -

	if(rc == 0) {
		socket->state = SOCKET_STATE_CLOSED;
	}

	mutex_unlock(&socket->mutex);

	return rc;
}

int socket_closeChildSocket(Socket *socket, int sd)
{
	int rc = 0;

	if((socket == NULL) || (socket->state != SOCKET_STATE_OPEN)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

#if defined(__linux__) || defined(__APPLE__) || defined(MINGW)
	rc = socket_closeChildSocketLinux(socket, sd);
#elif defined(WIN32)
	rc = socket_closeChildSocketWindows(socket, sd);
#else // - no plugin available -
	rc = -1;
#endif // - plugins -

	return rc;
}

int socket_requestShutdown(Socket *socket)
{
	if((socket == NULL) ||
			(socket->state != SOCKET_STATE_OPEN) ||
			(socket->mode != SOCKET_MODE_SERVER)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&socket->config.mutex);

	socket->config.shutdownRequested = atrue;

	mutex_unlock(&socket->config.mutex);

	return 0;
}

int socket_listen(Socket *socket)
{
	int rc = 0;

	if((socket == NULL) || (socket->state != SOCKET_STATE_OPEN) ||
			(socket->mode != SOCKET_MODE_SERVER) ||
			(socket->protocol != SOCKET_PROTOCOL_TCPIP)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&socket->mutex);

#if defined(__linux__) || defined(__APPLE__) || defined(MINGW)
	rc = socket_listenLinux(socket);
#elif defined(WIN32)
	rc = socket_listenWindows(socket);
#else // - no plugin available -
	rc = -1;
#endif // - plugins -

	mutex_unlock(&socket->mutex);

	return rc;
}

int socket_accept(Socket *socket, int *sd, int microseconds)
{
	int rc = 0;

	if((socket == NULL) || (socket->state != SOCKET_STATE_OPEN) ||
			(socket->mode != SOCKET_MODE_SERVER) ||
			(socket->protocol != SOCKET_PROTOCOL_TCPIP) ||
			(sd == NULL) ||
			(microseconds < 0.0)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	*sd = 0;

	mutex_lock(&socket->mutex);

#if defined(__linux__) || defined(__APPLE__) || defined(MINGW)
	rc = socket_acceptLinux(socket, microseconds);
#elif defined(WIN32)
	rc = socket_acceptWindows(socket, microseconds);
#else // - no plugin available -
	rc = -1;
#endif // - plugins -

	if(rc >= 0) {
		*sd = rc;
	}

	mutex_unlock(&socket->mutex);

	return rc;
}

int socket_getPeerName(Socket *socket, int sd, char **peerName)
{
	int rc = 0;

	if(socket == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

#if defined(__linux__) || defined(__APPLE__) || defined(MINGW)
	*peerName = socket_getPeerNameLinux(socket, sd);
#elif defined(WIN32)
	*peerName = socket_getPeerNameWindows(socket, sd);
#else // - no plugin available -
	rc = -1;
#endif // - plugins -

	return rc;
}

int socket_send(Socket *socket, int sd, char *value, int valueLength)
{
	int rc = 0;
	double timer = 0.0;
	double elapsedTime = 0.0;
	double sleepTime = 0.0;

	if((socket == NULL) || (socket->state != SOCKET_STATE_OPEN) ||
			(sd < 0) || (value == NULL) || (valueLength < 1)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	timer = time_getTimeMus();

	mutex_lock(&socket->metrics.mutex);
	socket->metrics.writesAttempted += 1;
	mutex_unlock(&socket->metrics.mutex);

#if defined(__linux__) || defined(__APPLE__) || defined(MINGW)
	rc = socket_sendLinux(socket, sd, value, valueLength);
#elif defined(WIN32)
	rc = socket_sendWindows(socket, sd, value, valueLength);
#else // - no plugin available -
	rc = -1;
#endif // - plugins -

	elapsedTime = time_getElapsedMusInSeconds(timer);

	mutex_lock(&socket->metrics.mutex);

	if(rc == 0) {
		socket->metrics.bytesSent += (alint)valueLength;
		socket->metrics.writesCompleted += 1;
	}

	socket->metrics.totalWriteTime += elapsedTime;

	mutex_unlock(&socket->metrics.mutex);

	if((rc == 0) && (socket->config.useSocketWriteSync)) {
		sleepTime = ((double)socket->config.timeoutMillis -
				(elapsedTime * 1000.0));
		if(sleepTime > 0.0) {
			time_usleep((int)(sleepTime * 1000.0));
		}
	}

	return rc;
}

int socket_receive(Socket *socket, int sd, char *value, int valueLength,
		double timeoutSeconds)
{
	int rc = 0;
	double timer = 0.0;
	double elapsedTime = 0.0;

	if((socket == NULL) || (socket->state != SOCKET_STATE_OPEN) ||
			(sd < 0) || (value == NULL) || (valueLength < 1) ||
			(timeoutSeconds < 0.0)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	timer = time_getTimeMus();

	mutex_lock(&socket->metrics.mutex);
	socket->metrics.readsAttempted += 1;
	mutex_unlock(&socket->metrics.mutex);

#if defined(__linux__) || defined(__APPLE__) || defined(MINGW)
	rc = socket_receiveLinux(socket, sd, value, valueLength, timeoutSeconds);
#elif defined(WIN32)
	rc = socket_receiveWindows(socket, sd, value, valueLength, timeoutSeconds);
#else // - no plugin available -
	rc = -1;
#endif // - plugins -

	elapsedTime = time_getElapsedMusInSeconds(timer);

	mutex_lock(&socket->metrics.mutex);

	if(rc >= 0) {
		socket->metrics.bytesReceived += (alint)rc;
		socket->metrics.readsCompleted += 1;
	}

	socket->metrics.totalReadTime += elapsedTime;

	mutex_unlock(&socket->metrics.mutex);

	return rc;
}

