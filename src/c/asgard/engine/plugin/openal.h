/*
 * openal.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * A plugin to use the OpenAL API, header file.
 *
 * Written by Josh English.
 */

#if !defined(_ASGARD_ENGINE_PLUGIN_OPENAL_H)

#define _ASGARD_ENGINE_PLUGIN_OPENAL_H

#if !defined(_ASGARD_H) && !defined(_ASGARD_ENGINE_PLUGIN_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _ASGARD_H || _ASGARD_ENGINE_PLUGIN_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// define openal public constants

typedef enum _OpenALStatus {
	OPENAL_STATUS_INIT = 1,
	OPENAL_STATUS_OK,
	OPENAL_STATUS_ERROR = -1
} OpenALStatus;

#define OPENAL_DEFAULT_DEVICE_NAME						NULL


// define openal public data types

typedef struct _OpenAL {
	OpenALStatus status;
	char *deviceName;
	ALCcontext *context;
	ALCdevice *device;
	OggVorbis codec;
} OpenAL;


// declare openal public functions

int openal_init(OpenAL *openal, char *deviceName);

int openal_free(OpenAL *openal);

int openal_makeCurrent(OpenAL *openal);

int openal_openFileStream(OpenAL *openal, char *filename);

int openal_closeFileStream(OpenAL *openal);

int openal_play(OpenAL *openal);

int openal_stop(OpenAL *openal);

int openal_processPlayStream(OpenAL *openal);


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _ASGARD_ENGINE_PLUGIN_OPENAL_H

