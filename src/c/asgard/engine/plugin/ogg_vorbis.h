/*
 * ogg_vorbis.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * A plugin to use the OGG/Vorbis API, header file.
 *
 * Written by Josh English.
 */

#if !defined(_ASGARD_ENGINE_PLUGIN_OGG_VORBIS_H)

#define _ASGARD_ENGINE_PLUGIN_OGG_VORBIS_H

#if !defined(_ASGARD_H) && !defined(_ASGARD_ENGINE_PLUGIN_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _ASGARD_H || _ASGARD_ENGINE_PLUGIN_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// define ogg/vorbis public constants

#define OGG_VORBIS_BUFFER_COUNT					6


// define ogg/vorbis public data types

typedef int (*OggVorbisOpenStreamFunction)(void *datasource,
		OggVorbis_File *fileReference,char *initialBuffer, long initialBytes,
		ov_callbacks callbacks);

typedef int (*OggVorbisCloseStreamFunction)(OggVorbis_File *fileReference);

typedef long (*OggVorbisReadStreamFunction)(OggVorbis_File *fileReference,
		char *buffer, int bufferLength, int isBigEndian, int isWord,
		int iSigned, int *bitStream);

typedef vorbis_info *(*OggVorbisGetFileInfoFunction)(
		OggVorbis_File *fileReference, int link);

typedef struct _OggVorbisStream {
	int totalBuffersProcessed;
	unsigned int openalSourceId;
	unsigned int openalBuffers[OGG_VORBIS_BUFFER_COUNT];
	unsigned long frequency;
	unsigned long channels;
	unsigned long format;
	unsigned long bufferLength;
	char *decodeBuffer;
} OggVorbisStream;

typedef struct _OggVorbis {
	ov_callbacks callbacks;
	vorbis_info *vorbisInfo;
	OggVorbis_File fileReference;
	OggVorbisOpenStreamFunction functionOpenStream;
	OggVorbisCloseStreamFunction functionCloseStream;
	OggVorbisReadStreamFunction functionReadStream;
	OggVorbisGetFileInfoFunction functionGetFileInfo;
	OggVorbisStream stream;
	FILE *fh;
	void *libraryHandle;
} OggVorbis;


// declare ogg/vorbis public functions

int oggVorbis_init(OggVorbis *oggVorbis);

void oggVorbis_free(OggVorbis *oggVorbis);

int oggVorbis_openFileStream(OggVorbis *oggVorbis, char *filename);

int oggVorbis_closeFileStream(OggVorbis *oggVorbis);

int oggVorbis_play(OggVorbis *oggVorbis);

int oggVorbis_processPlayStream(OggVorbis *oggVorbis);

int oggVorbis_stop(OggVorbis *oggVorbis);


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _ASGARD_ENGINE_PLUGIN_OGG_VORBIS_H

