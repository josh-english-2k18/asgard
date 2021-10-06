/*
 * ogg_vorbis.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * A plugin to use the OGG/Vorbis API.
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


// declare ogg/vorbis private functions

static void swapChannel(short *sampleOne, short *sampleTwo);

static size_t fileReadFunction(void *ptr, size_t size, size_t nmemb,
		void *datasource);

static int fileCloseFunction(void *datasource);

static int fileSeekFunction(void *datasource, ogg_int64_t offset, int whence);

static long fileTellFunction(void *datasource);

static unsigned long decodeStream(OggVorbis *oggVorbis);


// define ogg/vorbis private functions

static void swapChannel(short *sampleOne, short *sampleTwo)
{
	short sampleBuffer = (short)0;

	sampleBuffer = *sampleOne;
	*sampleOne = *sampleTwo;
	*sampleTwo = sampleBuffer;
}

static size_t fileReadFunction(void *ptr, size_t size, size_t nmemb,
		void *datasource)
{
	size_t result = 0;

	result = fread(ptr, size, nmemb, (FILE *)datasource);

	return result;
}

static int fileCloseFunction(void *datasource)
{
   return fclose((FILE*)datasource);
}

static int fileSeekFunction(void *datasource, ogg_int64_t offset, int whence)
{
	return fseek((FILE*)datasource, (long)offset, whence);
}

static long fileTellFunction(void *datasource)
{
	return ftell((FILE*)datasource);
}

static unsigned long decodeStream(OggVorbis *oggVorbis)
{
	int currentSection = 0;
	long decodeLength = 0;
	unsigned long sampleCount = 0;
	unsigned long bytesDecoded = 0;
	unsigned long bufferLength = 0;
	short *samples = NULL;

	bufferLength = oggVorbis->stream.bufferLength;

	do {
		decodeLength = oggVorbis->functionReadStream(&oggVorbis->fileReference,
				(oggVorbis->stream.decodeBuffer + bytesDecoded),
				(bufferLength - bytesDecoded),
				0,
				2,
				1,
				&currentSection);

		if(decodeLength <= 0) {
			break;
		}

		bytesDecoded += decodeLength;

		if(bytesDecoded >= bufferLength) {
				break;
		}
	} while(1);

	/*
	 * Note: mono, stereo and 4-Channel files decode into the same channel
	 * order as WAVEFORMATEXTENSIBLE, however 6-channel files need to be
	 * reordered.
	 *
	 * 		o WAVEFORMATEXTENSIBLE order : | FL| FR| FC|LFE| RL| RR|
	 * 		o OggVorbis order            : | FL| FC| FR| RL| RR|LFE|
	 */

	if(oggVorbis->stream.channels == 6) {
		samples = (short *)oggVorbis->stream.decodeBuffer;

		for(sampleCount = 0; sampleCount < (bufferLength >> 1);
				sampleCount += 6) {
			swapChannel(&samples[(sampleCount + 1)],
					&samples[(sampleCount + 2)]);
			swapChannel(&samples[(sampleCount + 3)],
					&samples[(sampleCount + 5)]);
			swapChannel(&samples[(sampleCount + 4)],
					&samples[(sampleCount + 5)]);
		}
	}

	return bytesDecoded;
}


// define ogg/vorbis public functions

int oggVorbis_init(OggVorbis *oggVorbis)
{
	int result = 0;
	char *filename = NULL;
	void *functionPtr = NULL;

	if(oggVorbis == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	memset(oggVorbis, 0, (int)(sizeof(OggVorbis)));

#if defined(__linux__)
	filename = "vorbisfile.so";
#elif defined(__APPLE__)
	filename = "vorbisfile.dylib";
#elif defined(WIN32)
	filename = "vorbisfile.dll";
#else // - no plugin available -
	return -1;
#endif // - plugins -

	if(dl_open(filename, &oggVorbis->libraryHandle) < 0) {
			return -1;
	}

	if(dl_lookup(oggVorbis->libraryHandle, "ov_open_callbacks",
				&functionPtr) < 0) {
		return -1;
	}

	oggVorbis->functionOpenStream = (OggVorbisOpenStreamFunction)functionPtr;

	if(dl_lookup(oggVorbis->libraryHandle, "ov_clear",
				&functionPtr) < 0) {
		return -1;
	}

	oggVorbis->functionCloseStream = (OggVorbisCloseStreamFunction)functionPtr;

	if(dl_lookup(oggVorbis->libraryHandle, "ov_read",
				&functionPtr) < 0) {
		return -1;
	}

	oggVorbis->functionReadStream = (OggVorbisReadStreamFunction)functionPtr;

	if(dl_lookup(oggVorbis->libraryHandle, "ov_info",
				&functionPtr) < 0) {
		return -1;
	}

	oggVorbis->functionGetFileInfo = (OggVorbisGetFileInfoFunction)functionPtr;

	return result;
}

void oggVorbis_free(OggVorbis *oggVorbis)
{
	if(oggVorbis == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	if(oggVorbis->libraryHandle != NULL) {
		dl_close(oggVorbis->libraryHandle);
		oggVorbis->libraryHandle = NULL;
	}
}

int oggVorbis_openFileStream(OggVorbis *oggVorbis, char *filename)
{
	aboolean exists = afalse;
	int ii = 0;
	int rc = 0;
	unsigned long bytesWritten = 0;

	if((oggVorbis == NULL) || (filename == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	// set the file stream callbacks

	oggVorbis->callbacks.read_func = fileReadFunction;
	oggVorbis->callbacks.close_func = fileCloseFunction;
	oggVorbis->callbacks.seek_func = fileSeekFunction;
	oggVorbis->callbacks.tell_func = fileTellFunction;

	// open the file for streaming

	if((system_fileExists(filename, &exists) < 0) || (!exists)) {
		fprintf(stdout, "[%s():%i] error - failed to locate file '%s'.\n",
				__FUNCTION__, __LINE__, filename);
		return -1;
	}

#if defined(WIN32) && !defined(MINGW)
	rc = fopen_s(&oggVorbis->fh, filename, "rb");
#else // !WIN32 || MINGW
	oggVorbis->fh = fopen(filename, "rb");
#endif // WIN32 && !MINGW

	if((rc != 0) || (oggVorbis->fh == NULL)) {
		fprintf(stdout, "[%s():%i] error - failed to open file '%s'.\n",
				__FUNCTION__, __LINE__, filename);
		return -1;
	}

	// create the file stream

	if(oggVorbis->functionOpenStream(oggVorbis->fh, &oggVorbis->fileReference,
				NULL, 0, oggVorbis->callbacks) != 0) {
		fprintf(stdout, "[%s():%i] error - failed to create the file stream "
				"for '%s'.\n", __FUNCTION__, __LINE__, filename);
		return -1;
	}

	// obtain the OggVorbis file information - channels, format & frequency

	if((oggVorbis->vorbisInfo = oggVorbis->functionGetFileInfo(
					&oggVorbis->fileReference, -1)) == NULL) {
		fprintf(stdout, "[%s():%i] error - failed to obtain information "
				"about the file stream from '%s'.\n",
				__FUNCTION__, __LINE__, filename);
		return -1;
	}

	oggVorbis->stream.frequency = oggVorbis->vorbisInfo->rate;
	oggVorbis->stream.channels = oggVorbis->vorbisInfo->channels;

	/*
	 * Notes:
	 * 			o the buffer-length must be an exact multiple of the block
	 * 			  alignment
	 * 			o set the buffer-length to 250 millis
	 */

	switch(oggVorbis->stream.channels) {
		/*
		 * Note: (frequency * 2 (16-bit) / 4 (quarter of a second))
		 */
		case 1:
			oggVorbis->stream.format = AL_FORMAT_MONO16;
			oggVorbis->stream.bufferLength = oggVorbis->stream.frequency >> 1;
			oggVorbis->stream.bufferLength -=
				(oggVorbis->stream.bufferLength % 2);
			break;

		/*
		 * Note: (frequency * 4 (16-bit stereo) / 4 (quarter of a second))
		 */
		case 2:
			oggVorbis->stream.format = AL_FORMAT_STEREO16;
			oggVorbis->stream.bufferLength = oggVorbis->stream.frequency;
			oggVorbis->stream.bufferLength -=
				(oggVorbis->stream.bufferLength % 4);
			break;

		/*
		 * Note: (frequency * 8 (16-bit 4-channel) / 4 (quarter of a second))
		 */
		case 4:
			oggVorbis->stream.format = alGetEnumValue("AL_FORMAT_QUAD16");
			oggVorbis->stream.bufferLength = oggVorbis->stream.frequency * 2;
			oggVorbis->stream.bufferLength -=
				(oggVorbis->stream.bufferLength % 8);
			break;

		/*
		 * Note: (frequency * 12 (16-bit 6-channel) / 4 (quarter of a second))
		 */
		case 6:
			oggVorbis->stream.format = alGetEnumValue("AL_FORMAT_51CHN16");
			oggVorbis->stream.bufferLength = oggVorbis->stream.frequency * 3;
			oggVorbis->stream.bufferLength -=
				(oggVorbis->stream.bufferLength % 12);
			break;

		default:
			fprintf(stdout, "[%s():%i] error - obtained invalid channel "
					"number %i from file stream '%s'.\n",
					__FUNCTION__, __LINE__,
					(int)oggVorbis->stream.channels, filename);
			return -1;
	}

	// allocate the decoded-data buffer

	oggVorbis->stream.decodeBuffer = (char *)malloc(
			oggVorbis->stream.bufferLength + 8);

	// generate OpenAL buffers for streaming

	alGenBuffers(OGG_VORBIS_BUFFER_COUNT, oggVorbis->stream.openalBuffers);

	// generate an OpenAL source for buffer playback

	alGenSources(1, &oggVorbis->stream.openalSourceId);

	// place decoded data into the OpenAL buffers

	for(ii = 0; ii < OGG_VORBIS_BUFFER_COUNT; ii++) {
		if((bytesWritten = decodeStream(oggVorbis)) > 0) {
			alBufferData(oggVorbis->stream.openalBuffers[ii],
					oggVorbis->stream.format,
					oggVorbis->stream.decodeBuffer,
					bytesWritten,
					oggVorbis->stream.frequency);

			alSourceQueueBuffers(oggVorbis->stream.openalSourceId,
					1,
					&oggVorbis->stream.openalBuffers[ii]);
		}
	}

	return 0;
}

int oggVorbis_closeFileStream(OggVorbis *oggVorbis)
{
	if(oggVorbis == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	oggVorbis->functionCloseStream(&oggVorbis->fileReference);

	if(oggVorbis->stream.decodeBuffer != NULL) {
		free(oggVorbis->stream.decodeBuffer);
		oggVorbis->stream.decodeBuffer = NULL;
	}

	alDeleteSources(1, &oggVorbis->stream.openalSourceId);
	alDeleteBuffers(OGG_VORBIS_BUFFER_COUNT, oggVorbis->stream.openalBuffers);

	return 0;
}

int oggVorbis_play(OggVorbis *oggVorbis)
{
	if(oggVorbis == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	alSourcePlay(oggVorbis->stream.openalSourceId);
	oggVorbis->stream.totalBuffersProcessed = 0;

	return 0;
}

int oggVorbis_processPlayStream(OggVorbis *oggVorbis)
{
	int openAlState = 0;
	int buffersQueued = 0;
	int buffersProcessed = 0;
	unsigned int openAlBuffer = 0;
	unsigned long bytesWritten = 0;

	if(oggVorbis == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	// determine the number of OpenAL buffers that have been processed

	alGetSourcei(oggVorbis->stream.openalSourceId,
			AL_BUFFERS_PROCESSED, &buffersProcessed);
	oggVorbis->stream.totalBuffersProcessed += buffersProcessed;

	fprintf(stdout, "%s() :: buffers processed => %i\r",
			__FUNCTION__, oggVorbis->stream.totalBuffersProcessed);

	/*
	 * Note: for each processed buffer, remove it from the source queue, read
	 * the next chunk of audio data from the stream, fill buffer with new data,
	 * and add it to the source queue.
	 */

	while(buffersProcessed > 0) {
		// remove the buffer from the queue

		openAlBuffer = 0;
		alSourceUnqueueBuffers(oggVorbis->stream.openalSourceId,
				1, &openAlBuffer);

		// read audio data from the stream

		if((bytesWritten = decodeStream(oggVorbis)) > 0) {
			alBufferData(openAlBuffer,
					oggVorbis->stream.format,
					oggVorbis->stream.decodeBuffer,
					bytesWritten,
					oggVorbis->stream.frequency);

			alSourceQueueBuffers(oggVorbis->stream.openalSourceId,
					1, &openAlBuffer);
		}

		buffersProcessed--;
	}

	// determine if the OpenAL status is still playing

	alGetSourcei(oggVorbis->stream.openalSourceId, AL_SOURCE_STATE,
			&openAlState);

	if(openAlState != AL_PLAYING) {
		// determine if there are any remaining queued buffers

		alGetSourcei(oggVorbis->stream.openalSourceId, AL_BUFFERS_QUEUED,
				&buffersQueued);
		if(buffersQueued > 0) {
			alSourcePlay(oggVorbis->stream.openalSourceId);
		}
		else {
			return -1;
		}
	}

	return 0;
}

int oggVorbis_stop(OggVorbis *oggVorbis)
{
	if(oggVorbis == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	alSourceStop(oggVorbis->stream.openalSourceId);
	alSourcei(oggVorbis->stream.openalSourceId, AL_BUFFER, 0);

	return 0;
}

