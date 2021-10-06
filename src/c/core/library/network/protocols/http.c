/*
 * http.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard core networking library HTTP protcol functions.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#define _CORE_LIBRARY_NETWORK_PROTOCOLS_COMPONENT
#include "core/library/network/protocols/http.h"


// declare networking library HTTP protocol private functions

static char *buildRandomTempFilename(HttpProtocol *protocol);

static char *getFilenameFromHeader(char *buffer, int length, int offset);


// define networking library HTTP protocol private functions

static char *buildRandomTempFilename(HttpProtocol *protocol)
{
	int length = 0;
	char *result = NULL;

	length = (strlen(protocol->tempFileBasePath) +
			strlen("http.temp.file") +
			32);

	result = (char *)malloc(sizeof(char) * length);

	system_pickRandomSeed();

	snprintf(result, (length - 1), "%s/%s.%04i",
			protocol->tempFileBasePath,
			"http.temp.file",
			(rand() % 1024));

	return result;
}

static char *getFilenameFromHeader(char *buffer, int length, int offset)
{
	int ii = 0;
	int nn = 0;
	char *result = NULL;

	result = (char *)malloc(sizeof(char) * length);

	for(ii = offset, nn = 0; ii < length; ii++, nn++) {
		if(((unsigned int)buffer[ii] < 32) ||
				((unsigned int)buffer[ii] > 126)) {
			break;
		}
		result[nn] = buffer[ii];
	}

	return result;
}


// define networking library HTTP protocol public functions

// protocol functions

void http_protocol_init(HttpProtocol *protocol, aboolean isTempFileMode,
		alint tempFileThresholdBytes, alint tempFileSpaceAllowedBytes,
		alint tempFileSpaceInUseBytes, char *tempFileBasePath)
{
	if(protocol == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	memset(protocol, 0, (int)(sizeof(HttpProtocol)));

	mutex_init(&protocol->mutex);

	mutex_lock(&protocol->mutex);

	protocol->isTempFileMode = isTempFileMode;
	protocol->tempFileThresholdBytes = tempFileThresholdBytes;
	protocol->tempFileSpaceAllowedBytes = tempFileSpaceAllowedBytes;
	protocol->tempFileSpaceInUseBytes = tempFileSpaceInUseBytes;

	if(tempFileBasePath != NULL) {
		protocol->tempFileBasePath = strdup(tempFileBasePath);
	}
	else {
		protocol->tempFileBasePath = NULL;
	}

	mutex_unlock(&protocol->mutex);
}

void http_protocol_free(HttpProtocol *protocol)
{
	if(protocol == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	mutex_lock(&protocol->mutex);

	if(protocol->tempFileBasePath != NULL) {
		free(protocol->tempFileBasePath);
	}

	mutex_unlock(&protocol->mutex);

	mutex_free(&protocol->mutex);

	memset(protocol, 0, (int)(sizeof(HttpProtocol)));
}

int http_protocol_freeTempFile(HttpProtocol *protocol, char *filename)
{
	alint fileLength = 0;

	if((protocol == NULL) || (filename == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&protocol->mutex);

	if(!protocol->isTempFileMode) {
		mutex_unlock(&protocol->mutex);
		fprintf(stderr, "[%s():%i] error - the HTTP protocol is not in "
				"temp-file mode.\n", __FUNCTION__, __LINE__);
		return -1;
	}

	if(system_fileLength(filename, &fileLength) < 0) {
		mutex_unlock(&protocol->mutex);
		fprintf(stderr, "[%s():%i] error - failed to obtain valid length "
				"from temp-file '%s'.\n", __FUNCTION__, __LINE__, filename);
		return -1;
	}

	if(system_fileDelete(filename) < 0) {
		mutex_unlock(&protocol->mutex);
		fprintf(stderr, "[%s():%i] error - failed to delete temp-file '%s'.\n",
				__FUNCTION__, __LINE__, filename);
		return -1;
	}

	protocol->tempFileSpaceInUseBytes -= fileLength;

	if(protocol->tempFileSpaceInUseBytes < 0) {
		protocol->tempFileSpaceInUseBytes = 0;
	}

	mutex_unlock(&protocol->mutex);

	return 0;
}

int http_protocol_send(void *context, Socket *socket, int sd, char *value,
		int valueLength)
{
	if((context == NULL) || (socket == NULL) || (sd < 0) || (value == NULL) ||
			(valueLength < 1)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return socket_send(socket, sd, value, valueLength);
}

char *http_protocol_receive(void *context, Socket *socket, int sd,
		int *receiveLength)
{
	aboolean firstTime = atrue;
	aboolean foundHeader = afalse;
	aboolean foundHeaderProtocol = afalse;
	aboolean invalidRequest = afalse;
	aboolean isWriteTempFileMode = afalse;
	int rc = 0;
	int counter = 0;
	int valueRef = 0;
	int valueLength = 0;
	int chunkLength = 0;
	alint payloadLength = 0;
	alint payloadBytesRead = 0;
	unsigned int byteValue = (unsigned int)0;
	double timer = 0.0;
	double timeout = 0.0;
	double chunkTimeout = 0.0;
	double elapsedTime = 0.0;
	char chunk[16384];
	char *ptr = NULL;
	char *value = NULL;
	char *errorMessage = NULL;
	char *filename = NULL;

	FileHandle fh;
	HttpProtocol *protocol = NULL;

	if((socket == NULL) || (sd < 0) || (receiveLength == NULL)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	protocol = (HttpProtocol *)context;

	*receiveLength = 0;

	// read HTTP message header

	rc = 0;
	counter = 0;

	valueRef = 0;
	valueLength = 1024;
	value = (char *)malloc(sizeof(char) * valueLength);

	timer = time_getTimeMus();
	timeout = 5.0;

	memset(chunk, 0, (int)sizeof(chunk));

	do {
		rc = socket_receive(socket, sd, chunk, 1, timeout);

		while((rc == 0) && (counter < 3)) {
			time_usleep(16384);

			rc = socket_receive(socket, sd, chunk, 1, timeout);

			counter++;
		}

		if(rc != 1) {
			break;
		}

		if(firstTime) {
			firstTime = afalse;
			timeout = 0.008;
		}
		else if((valueRef > 0) && ((valueRef % 8) == 0)) {
			timeout = (time_getElapsedMusInSeconds(timer) / (double)valueRef);

			if(timeout < 0.000128) {
				timeout = 0.000128;
			}
			else if(timeout > 4.096) {
				timeout = 4.096;
			}
		}

		byteValue = (unsigned int)chunk[0];

		if((byteValue != 10) &&
				(byteValue != 13) &&
				((byteValue < 32) ||
				 (byteValue > 126))) {
			invalidRequest = atrue;
			break;
		}

		value[valueRef] = (char)byteValue;
		valueRef++;

		if(valueRef >= valueLength) {
			valueLength *= 2;
			value = (char *)realloc(value, (sizeof(char) * valueLength));
		}

		if((byteValue == 10) && (valueRef > 4)) {
			if(((unsigned int)value[(valueRef - 4)] == 13) &&
					((unsigned int)value[(valueRef - 3)] == 10) &&
					((unsigned int)value[(valueRef - 2)] == 13)) {
				foundHeader = atrue;
				break;
			}
		}
		else if((!foundHeaderProtocol) && (byteValue == 'P') &&
				(valueRef >= 4)) {
			if(((unsigned int)value[(valueRef - 4)] == 'H') &&
					((unsigned int)value[(valueRef - 3)] == 'T') &&
					((unsigned int)value[(valueRef - 2)] == 'T')) {
				foundHeaderProtocol = atrue;
			}
		}

		if((!foundHeaderProtocol) &&
				(valueRef > 65536)) { // max standard URL length
			break;
		}
	} while(rc == 1);

	if(rc < 0) {
		fprintf(stderr, "[%s():%i] error(%i) - failed to read from socket "
				"after reading %i bytes "
				"on socket #%i\n",
				__FUNCTION__, __LINE__, rc, valueRef, sd);
		free(value);
		return NULL;
	}
	else if(invalidRequest) {
		fprintf(stderr, "[%s():%i] error(%i) - invalid HTTP request "
				"after reading %i bytes "
				"on socket #%i\n",
				__FUNCTION__, __LINE__, rc, valueRef, sd);
		free(value);
		return NULL;
	}
	else if(valueRef < 4) {
		fprintf(stderr, "[%s():%i] error(%i) - message too small to be HTTP "
				"after reading %i bytes "
				"on socket #%i\n",
				__FUNCTION__, __LINE__, rc, valueRef, sd);
		free(value);
		return NULL;
	}
	else if(!foundHeader) {
		fprintf(stderr, "[%s():%i] error(%i) - failed to locate header "
				"after reading %i bytes "
				"on socket #%i\n",
				__FUNCTION__, __LINE__, rc, valueRef, sd);
		free(value);
		return NULL;
	}
	else if(!foundHeaderProtocol) {
		fprintf(stderr, "[%s():%i] error(%i) - failed to locate HTTP header "
				"after reading %i bytes "
				"on socket #%i\n",
				__FUNCTION__, __LINE__, rc, valueRef, sd);
		free(value);
		return NULL;
	}

	value[valueRef] = '\0';

	// determine if HTTP message payload exists

	if((ptr = strncasestr(value, valueRef, "Content-Length:", 15)) != NULL) {
		if((payloadLength =
					(alint)atod((char *)((aptrcast)ptr + (alint)16))) < 0) {
			payloadLength = 0;
		}
	}
	else if((ptr = strncasestr(value, valueRef,
					"Transfer-encoding: chunked", 26)) != NULL) {
		payloadLength = -1;
	}

	if(payloadLength == 0) {
		*receiveLength = valueRef;
		return value;
	}

	// receive the HTTP message payload

	if((protocol != NULL) &&
			(protocol->isTempFileMode) &&
			(payloadLength >= protocol->tempFileThresholdBytes)) {
		if(payloadLength >= (protocol->tempFileSpaceAllowedBytes -
					protocol->tempFileSpaceInUseBytes)) {
			fprintf(stderr, "[%s():%i] error - payload length %0.0f is "
					"greater than available temp-file space %0.0f.\n",
					__FUNCTION__, __LINE__, (double)payloadLength,
					(double)(protocol->tempFileSpaceAllowedBytes -
						protocol->tempFileSpaceInUseBytes));
			free(value);
			return NULL;
		}

		isWriteTempFileMode = atrue;

		filename = buildRandomTempFilename(protocol);

		if((rc = file_init(&fh, filename, "w+b", 0)) < 0) {
			errorMessage = file_getErrorMessage(&fh, rc);
			fprintf(stderr, "[%s():%i] error - failed to open temp file '%s' "
					"with '%s'.\n", __FUNCTION__, __LINE__, filename,
					errorMessage);
			free(value);
			free(filename);
			free(errorMessage);
			file_free(&fh);
			return NULL;
		}
	}
	else if(payloadLength != -1) {
		if((valueRef + payloadLength) >= valueLength) {
			valueLength += ((int)payloadLength + 16);
			value = (char *)realloc(value, (sizeof(char) * valueLength));
		}
	}

	counter = 0;

	do {
		// read chunked transfer encoding

		if(payloadLength == -1) {
			chunkLength = 0;
			elapsedTime = 0.0;

			timer = time_getTimeMus();

			do {
				rc = socket_receive(socket, sd, (chunk + chunkLength), 1,
						timeout);

				while((rc == 0) && (counter < 3)) {
					time_usleep(16384);

					rc = socket_receive(socket, sd, (chunk + chunkLength), 1,
							timeout);

					counter++;
				}

				if(rc < 0) {
					chunkLength = -1;
					break;
				}
				else if(rc > 0) {
					chunkLength += rc;

					if((chunkLength >= 3) &&
							((unsigned int)chunk[(chunkLength - 1)] == 10) &&
							((unsigned int)chunk[(chunkLength - 2)] == 13)) {
						chunk[(chunkLength - 2)] = '\0';

						if(sscanf(chunk, "%8x", &chunkLength) <= 0) {
							chunkLength = -1;
						}

						break;
					}
				}
			} while(time_getElapsedMusInSeconds(timer) < 2.048);

			if(chunkLength <= 0) {
				if(chunkLength < 0) {
					rc = -1;
				}
				break;
			}

			elapsedTime = time_getElapsedMusInSeconds(timer);

			timeout = (elapsedTime / (double)chunkLength);

			if(timeout < 0.000128) {
				timeout = 0.000128;
			}
			else if(timeout > 4.096) {
				timeout = 4.096;
			}
		}
		else {
			chunkLength = (int)(sizeof(chunk));

			if(chunkLength > (int)(payloadLength - payloadBytesRead)) {
				chunkLength = (int)(payloadLength - payloadBytesRead);
			}
		}

		elapsedTime = 0.0;
		chunkTimeout = (timeout * (double)chunkLength);

		if(chunkTimeout < 0.000128) {
			chunkTimeout = 0.000128;
		}
		else if(chunkTimeout > 4.096) {
			chunkTimeout = 4.096;
		}

		do {
			timer = time_getTimeMus();

			rc = socket_receive(socket, sd, chunk, (int)chunkLength,
					chunkTimeout);

			if(rc != 0) {
				break;
			}

			elapsedTime = time_getElapsedMusInSeconds(timer);

			chunkTimeout -= elapsedTime;
		} while(chunkTimeout > 0.0);

		if(rc <= 0) {
			break;
		}

		chunkLength = rc;

		if(isWriteTempFileMode) {
			if((rc = file_write(&fh, chunk, chunkLength)) < 0) {
				errorMessage = file_getErrorMessage(&fh, rc);

				fprintf(stderr, "[%s():%i] error - failed to write to temp "
						"file '%s' with '%s'.\n", __FUNCTION__, __LINE__,
						filename, errorMessage);

				system_fileDelete(filename);

				if(filename != NULL) {
					free(filename);
				}

				free(errorMessage);
				free(value);
				file_free(&fh);

				return NULL;
			}
		}
		else {
			if((valueRef + chunkLength) >= valueLength) {
				while((valueRef + chunkLength) >= valueLength) {
					valueLength *= 2;
				}
				value = (char *)realloc(value, (sizeof(char) * valueLength));
			}

			memcpy((value + valueRef), chunk, chunkLength);

			valueRef += chunkLength;
		}

		payloadBytesRead += (alint)rc;
	} while((payloadBytesRead < payloadLength) || (payloadLength == -1));

	if(isWriteTempFileMode) {
		file_free(&fh);
	}

	if(rc < 0) {
		fprintf(stderr, "[%s():%i] error(%i) - failed to read from socket "
				"after reading %0.0f bytes "
				"on socket #%i\n",
				__FUNCTION__, __LINE__, rc, (double)payloadBytesRead, sd);
		if(isWriteTempFileMode) {
			system_fileDelete(filename);
			free(filename);
		}
		free(value);
		return NULL;
	}
	else if((payloadBytesRead != payloadLength) && (payloadLength != -1)) {
		fprintf(stderr, "[%s():%i] error - received %0.0f bytes when "
				"expecting %0.0f bytes on socket #%i\n",
				__FUNCTION__, __LINE__, (double)payloadBytesRead,
				(double)payloadLength, sd);
		if(isWriteTempFileMode) {
			system_fileDelete(filename);
			free(filename);
		}
		free(value);
		return NULL;
	}

	if(isWriteTempFileMode) {
		chunkLength = snprintf(chunk, (int)(sizeof(chunk) - 1),
				"Content-in-file: %s\r\n\r\n", filename);

		if((valueRef + chunkLength) >= valueLength) {
			valueLength += (chunkLength + 16);
			value = (char *)realloc(value, (sizeof(char) * valueLength));
		}

		memcpy((value + valueRef), chunk, chunkLength);

		valueRef += chunkLength;

		mutex_lock(&protocol->mutex);

		protocol->tempFileSpaceInUseBytes += payloadBytesRead;

		mutex_unlock(&protocol->mutex);
	}

	if(filename != NULL) {
		free(filename);
	}

	*receiveLength = valueRef;

	return value;
}

// http protocol transaction managager functions

void *http_protocol_transactionManagerReceive(void *manager, void *context,
		Socket *socket, int sd, int *bytesReceived, int *errorCode)
{
	char *ptr = NULL;

	HttpTransactionManagerReceive *result = NULL;

	if((context == NULL) || (socket == NULL) || (sd < 0) ||
			(bytesReceived == NULL) || (errorCode == NULL)) {
		DISPLAY_INVALID_ARGS;

		if(bytesReceived != NULL) {
			*bytesReceived = 0;
		}
		if(errorCode != NULL) {
			*errorCode = TRANSACTION_ERROR_INVALID_ARGS;
		}

		return NULL;
	}

	*bytesReceived = 0;
	*errorCode = 0;

	result = (HttpTransactionManagerReceive *)malloc(
			sizeof(HttpTransactionManagerReceive));

	result->isContentInFile = afalse;
	result->payloadLength = 0;
	result->ipAddress = NULL;
	result->contentFilename = NULL;
	result->payload = NULL;

	socket_getPeerName(socket, sd, &(result->ipAddress));

	result->payload = http_protocol_receive(context, socket, sd,
			&result->payloadLength);

	if(result->payload == NULL) {
		*errorCode = TRANSACTION_ERROR_READ;
		free(result);
		return NULL;
	}
	else if(result->payloadLength < 1) {
		*errorCode = TRANSACTION_ERROR_TIMEOUT;
		free(result->payload);
		free(result);
		return NULL;
	}

	if((ptr = strncasestr(result->payload, result->payloadLength,
					"Content-in-file:", 16)) != NULL) {
		result->isContentInFile = atrue;
		result->contentFilename = getFilenameFromHeader(result->payload,
				result->payloadLength, (int)((ptr - result->payload) + 17));
	}

	*bytesReceived = result->payloadLength;

	return result;
}

void http_protocol_transactionManagerReceiveFree(void *argument, void *memory)
{
	HttpTransactionManagerReceive *receive = NULL;

	if(memory == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	receive = (HttpTransactionManagerReceive *)memory;

	if(receive->ipAddress != NULL) {
		free(receive->ipAddress);
	}

	if(receive->contentFilename != NULL) {
		free(receive->contentFilename);
	}

	if(receive->payload != NULL) {
		free(receive->payload);
	}

	free(receive);
}

int http_protocol_transactionManagerSend(void *manager, void *context,
		Socket *socket, int sd, void *response)
{
	HttpTransactionManagerSend *send = NULL;

	if((socket == NULL) || (sd < 0) || (response == NULL)) {
		DISPLAY_INVALID_ARGS;
		return TRANSACTION_ERROR_INVALID_ARGS;
	}

	send = (HttpTransactionManagerSend *)response;

	if(http_protocol_send(context, socket, sd, send->payload,
				send->payloadLength) == 0) {
		return send->payloadLength;
	}

	return TRANSACTION_ERROR_WRITE;
}

void http_protocol_transactionManagerSendFree(void *argument, void *memory)
{
	HttpTransactionManagerSend *send = NULL;

	if(memory == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	send = (HttpTransactionManagerSend *)memory;

	if(send->payload != NULL) {
		free(send->payload);
	}

	free(send);
}

char *http_protocol_transactionManagerIdFunction(void *context, void *request,
		int *messageIdLength)
{
	int ii = 0;
	int resultRef = 0;
	int resultLength = 0;
	int whitespaceCounter = 0;
	char *result = NULL;

	HttpTransactionManagerReceive *receive = NULL;

	if((request == NULL) || (messageIdLength == NULL)) {
		DISPLAY_INVALID_ARGS;

		if(messageIdLength != NULL) {
			*messageIdLength = 0;
		}

		return NULL;
	}

	*messageIdLength = 0;

	receive = (HttpTransactionManagerReceive *)request;

	resultRef = 0;
	resultLength = 16;
	result = (char *)malloc(sizeof(char) * (resultLength + 1));

	for(ii = 0; ii < receive->payloadLength; ii++) {
		if(ctype_isWhitespace(receive->payload[ii])) {
			whitespaceCounter++;
		}
		else if(whitespaceCounter == 1) {
			if(receive->payload[ii] == '?') {
				break;
			}

			result[resultRef] = receive->payload[ii];
			resultRef++;
			if(resultRef >= resultLength) {
				resultLength *= 2;
				result = (char *)realloc(result,
						(sizeof(char) * (resultLength + 1)));
			}
		}
		else if(whitespaceCounter > 1) {
			break;
		}
	}

	if(resultRef < 1) {
		free(result);
		return strdup("unknown");
	}

	*messageIdLength = resultRef;

	return result;
}

