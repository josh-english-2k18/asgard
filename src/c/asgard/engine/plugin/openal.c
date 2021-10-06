/*
 * openal.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * A plugin to use the OpenAL API.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#define _ASGARD_ENGINE_PLUGIN_COMPONENT
#include "openal/al.h"
#include "openal/alc.h"
#include "vorbis/vorbisfile.h"
#include "asgard/engine/plugin/ogg_vorbis.h"
#include "asgard/engine/plugin/openal.h"


// define openal public functions

int openal_init(OpenAL *openal, char *deviceName)
{
	int result = 0;

	if(openal == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	memset(openal, 0, (int)(sizeof(OpenAL)));

	openal->status = OPENAL_STATUS_INIT;

	if(deviceName != OPENAL_DEFAULT_DEVICE_NAME) {
		openal->deviceName = strdup(deviceName);
	}
	else {
		openal->deviceName = OPENAL_DEFAULT_DEVICE_NAME;
	}

	if((openal->device = alcOpenDevice(openal->deviceName)) != NULL) {
		if((openal->context = alcCreateContext(openal->device, NULL)) != NULL) {
			openal->status = OPENAL_STATUS_OK;

			fprintf(stdout, "[%s():%i] note - opened device '%s'.\n",
					__FUNCTION__, __LINE__,
					alcGetString(openal->device, ALC_DEVICE_SPECIFIER));
		}
		else {
			alcCloseDevice(openal->device);

			openal->status = OPENAL_STATUS_ERROR;
			openal->device = NULL;
			openal->context = NULL;

			result = -1;

			fprintf(stdout, "[%s():%i] error - failed to initialize OpenAL "
					"on device '%s'.\n", __FUNCTION__, __LINE__,
					alcGetString(openal->device, ALC_DEVICE_SPECIFIER));
		}
	}

	if(result == 0) {
		if(oggVorbis_init(&openal->codec)) {
			fprintf(stdout, "[%s():%i] error - failed to find Ogg/Vorbis "
					"libraries - vorbisfile, ogg, or vorbis.\n",
					__FUNCTION__, __LINE__);
			result = -1;
		}
	}

	return result;
}

int openal_free(OpenAL *openal)
{
	int result = 0;

	if(openal == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	// free the ogg/vorbis codec

	oggVorbis_free(&openal->codec);

	// remove the current context (if any)

	alcMakeContextCurrent(NULL);

	if(openal->status == OPENAL_STATUS_OK) {
		// destroy the openal  context

		if(openal->context) {
			alcDestroyContext(openal->context);
		}
		else {
			result = -1;
		}

		// free the openal device

		if(openal->device) {
			alcCloseDevice(openal->device);
		}
		else {
			result = -1;
		}
	}

	openal->status = OPENAL_STATUS_INIT;

	return result;
}

int openal_makeCurrent(OpenAL *openal)
{
	if((openal == NULL) || (openal->status != OPENAL_STATUS_OK)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	alcMakeContextCurrent(openal->context);

	return 0;
}

int openal_openFileStream(OpenAL *openal, char *filename)
{
	int result = 0;

	if((openal == NULL) || (openal->status != OPENAL_STATUS_OK) ||
			(filename == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if((result = oggVorbis_openFileStream(&openal->codec, filename)) < 0) {
		openal->status = OPENAL_STATUS_ERROR;
	}

	return result;
}

int openal_closeFileStream(OpenAL *openal)
{
	int result = 0;

	if((openal == NULL) || (openal->status != OPENAL_STATUS_OK)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if((result = oggVorbis_closeFileStream(&openal->codec)) < 0) {
		openal->status = OPENAL_STATUS_ERROR;
	}

	return result;
}

int openal_play(OpenAL *openal)
{
	int result = 0;

	if((openal == NULL) || (openal->status != OPENAL_STATUS_OK)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if((result = oggVorbis_play(&openal->codec)) < 0) {
		openal->status = OPENAL_STATUS_ERROR;
	}

	return result;
}

int openal_stop(OpenAL *openal)
{
	int result = 0;

	if((openal == NULL) || (openal->status != OPENAL_STATUS_OK)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if((result = oggVorbis_stop(&openal->codec)) < 0) {
		openal->status = OPENAL_STATUS_ERROR;
	}

	return result;
}

int openal_processPlayStream(OpenAL *openal)
{
	int result = 0;

	if((openal == NULL) || (openal->status != OPENAL_STATUS_OK)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if((result = oggVorbis_processPlayStream(&openal->codec)) < 0) {
		openal->status = OPENAL_STATUS_ERROR;
	}

	return result;
}

