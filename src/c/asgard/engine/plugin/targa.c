/*
 * targa.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * A plugin to read targa (TGA) image files into an OpenGL-compatible RGBA
 * format.
 *
 * Written by Josh English.
 *
 * TARGA Spec:
 *
 * Pos:		Len:		Field:
 * 0		1			Length of image ID
 * 1		1			Type of color map (if any) included
 * 							0	->	no color map
 * 							1	->	has color map
 * 2		1			image type
 * 							0	->	no image data
 * 							1	->	color-mapped image
 * 							2	->	true-color image
 * 							3	->	black & white image
 * 							9	->	RLE color-mapped image
 * 							10	->	RLE true-color image
 * 							11	->	RLE black & white image
 * 3		2			index of first color-map entry, e.g. if 1024 color map
 * 						entries, only storing 72 entries, field starts in middle
 * 						of color-map
 * 5		2			color-map length
 * 7		1			color-map entry size - the number of bits per entry
 * 8		2			x-origin of image
 * 10		2			y-origin of image
 * 12		2			image width in pixels
 * 14		2			image height in pixels
 * 16		1			pixel depth - the number of bits per pixel
 * 17		1			image descriptor
 * n		var			image id - only exists if non-zero
 * n		var			color-map data - only exists if non-zero
 * n		var			image data
 *
 */

// preprocessor directives

#include "core/core.h"
#define _ASGARD_ENGINE_PLUGIN_COMPONENT
#include "asgard/engine/plugin/targa.h"

// define targa private constants

#define TGA_COLOR_MAP_NONE							0

#define TGA_IMAGE_TYPE_NONE							0
#define TGA_IMAGE_TYPE_CM							1
#define TGA_IMAGE_TYPE_BGR							2
#define TGA_IMAGE_TYPE_BW							3
#define TGA_IMAGE_TYPE_RLE_CM						9
#define TGA_IMAGE_TYPE_RLE_BGR						10
#define TGA_IMAGE_TYPE_RLE_BW						11

#define TGA_R										0
#define TGA_G										1
#define TGA_B										2
#define TGA_A										3

// declare targa private functions

static int handleTargaError(FileHandle *fh, int errorCode,
		const char *function, size_t line);

// define targa private macros

#define targaErrorf() \
	handleTargaError(&fh, rc, __FUNCTION__, __LINE__)

// define targa private functions

static int handleTargaError(FileHandle *fh, int errorCode,
		const char *function, size_t line)
{
	char *errorMessage = NULL;

	if(file_getError(fh, errorCode, &errorMessage) < 0) {
		errorMessage = strdup("uknown file error");
	}

	fprintf(stderr, "[%s():%i] error(%i) - '%s'.\n",
			(char *)function, (int)line, errorCode, errorMessage);

	free(errorMessage);

	file_free(fh);

	return -1;
}

// define targa public functions

int targa_init(Targa *targa)
{
	if(targa == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	memset((void *)targa, 0, sizeof(Targa));

	targa->width = 0;
	targa->height = 0;
	targa->imageLength = 0;
	targa->image = NULL;

	return 0;
}

int targa_free(Targa *targa)
{
	if(targa == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(targa->image != NULL) {
		free(targa->image);
	}

	memset((void *)targa, 0, sizeof(Targa));

	return 0;
}

int targa_getDimensions(Targa *targa, int *width, int *height)
{
	if((targa == NULL) || (width == NULL) || (height == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	*width = targa->width;
	*height = targa->height;

	return 0;
}

int targa_getImageLength(Targa *targa, int *imageLength)
{
	if((targa == NULL) || (imageLength == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	*imageLength = targa->imageLength;

	return 0;
}

int targa_getRgbaTexture(Targa *targa, char **texture, int *textureLength)
{
	if((targa == NULL) || (texture == NULL) || (textureLength == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	*texture = (char *)targa->image;
	*textureLength = targa->imageLength;

	return 0;
}

int targa_loadFromFile(Targa *targa, char *filename)
{
	int rc = 0;
	int fileLength32 = 0;
	alint fileLength64 = 0;
	unsigned char *buffer = NULL;

	FileHandle fh;

	if((targa == NULL) || (filename == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if((rc = file_init(&fh, filename, "r+b", 65536)) < 0) {
		return targaErrorf();
	}

	if((rc = file_getFileLength(&fh, &fileLength64)) < 0) {
		return targaErrorf();
	}

	fileLength32 = (int)fileLength64;
	if(fileLength32 < 18) {
		fprintf(stderr, "error - TGA file '%s' length %i invalid.\n",
				filename, fileLength32);
		file_free(&fh);
		return -1;
	}

	buffer = (unsigned char *)malloc(sizeof(unsigned char) * fileLength32);

	if((rc = file_read(&fh, (char *)buffer, fileLength32)) != fileLength32) {
		return targaErrorf();
	}

	rc = targa_loadFromData(targa, buffer, fileLength32);

	file_free(&fh);

	free(buffer);

	return rc;
}

int targa_loadFromData(Targa *targa, unsigned char *data, int dataLength)
{
	short sNumber = 0;
	int ii = 0;
	int nn = 0;
	int imageIdLength = 0;
	int colorMap = 0;
	int imageType = 0;
	int bitLength = 0;
	int colorMode = 0;
	int length = 0;
	int rleId = 0;
	int pixel[4];
	unsigned char *imageId = NULL;
	unsigned char *ptr = NULL;

	if((targa == NULL) || (data == NULL) || (dataLength < 18)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	ptr = data;

	// determine image ID length

	imageIdLength = (int)ptr[0];
	ptr++;
	if((int)(ptr - data) > dataLength) {
		fprintf(stderr, "[%s():%i] error - detected data overrun with %i vs "
				"%i.\n", __FUNCTION__, __LINE__, (int)(ptr - data),
				dataLength);
		return -1;
	}

	// check for color map

	colorMap = (int)ptr[0];
	ptr++;
	if((int)(ptr - data) > dataLength) {
		fprintf(stderr, "[%s():%i] error - detected data overrun with %i vs "
				"%i.\n", __FUNCTION__, __LINE__, (int)(ptr - data),
				dataLength);
		return -1;
	}
	if(colorMap != TGA_COLOR_MAP_NONE) {
		fprintf(stderr, "[%s():%i] error - unable to read TARGA color map "
				"%i.\n", __FUNCTION__, __LINE__, colorMap);
		return -1;
	}

	// obtain image type

	imageType = (int)ptr[0];
	ptr++;
	if((int)(ptr - data) > dataLength) {
		fprintf(stderr, "[%s():%i] error - detected data overrun with %i vs "
				"%i.\n", __FUNCTION__, __LINE__, (int)(ptr - data),
				dataLength);
		return -1;
	}
	if((imageType == TGA_IMAGE_TYPE_NONE) ||
			(imageType == TGA_IMAGE_TYPE_CM) ||
			(imageType == TGA_IMAGE_TYPE_RLE_CM)) {
		fprintf(stderr, "[%s():%i] error - unsupported image type %i.\n",
				__FUNCTION__, __LINE__, imageType);
		return -1;
	}

	// skip 9 bytes (color-map information and x & y origins)

	ptr += 9;
	if((int)(ptr - data) > dataLength) {
		fprintf(stderr, "[%s():%i] error - detected data overrun with %i vs "
				"%i.\n", __FUNCTION__, __LINE__, (int)(ptr - data),
				dataLength);
		return -1;
	}

	// obtain image width

	memcpy((char *)&sNumber, ptr, sizeof(short));
	ptr += sizeof(short);
	if((int)(ptr - data) > dataLength) {
		fprintf(stderr, "[%s():%i] error - detected data overrun with %i vs "
				"%i.\n", __FUNCTION__, __LINE__, (int)(ptr - data),
				dataLength);
		return -1;
	}
	if(sNumber < 1) {
		fprintf(stderr, "[%s():%i] error - invalid image width %i.\n",
				__FUNCTION__, __LINE__, (int)sNumber);
		return -1;
	}

	targa->width = (int)sNumber;

	// obtain image height

	memcpy((char *)&sNumber, ptr, sizeof(short));
	ptr += sizeof(short);
	if((int)(ptr - data) > dataLength) {
		fprintf(stderr, "[%s():%i] error - detected data overrun with %i vs "
				"%i.\n", __FUNCTION__, __LINE__, (int)(ptr - data),
				dataLength);
		return -1;
	}
	if(sNumber < 1) {
		fprintf(stderr, "[%s():%i] error - invalid image height %i.\n",
				__FUNCTION__, __LINE__, (int)sNumber);
		return -1;
	}

	targa->height = (int)sNumber;

	// determine pixel depth

	bitLength = (int)ptr[0];
	ptr++;
	if((int)(ptr - data) > dataLength) {
		fprintf(stderr, "[%s():%i] error - detected data overrun with %i vs "
				"%i.\n", __FUNCTION__, __LINE__, (int)(ptr - data),
				dataLength);
		return -1;
	}
	if((bitLength != 16) && (bitLength != 24) && (bitLength != 32)) {
		fprintf(stderr, "[%s():%i] error - unknown pixel depth of %i-bits.\n",
				__FUNCTION__, __LINE__, bitLength);
		return -1;
	}
	if((bitLength == 16) &&
			((imageType != TGA_IMAGE_TYPE_BGR) &&
			 (imageType != TGA_IMAGE_TYPE_BW))) {
		fprintf(stderr, "[%s():%i] error - unable to RLE-decode pixel depth "
				"of %i-bits.\n", __FUNCTION__, __LINE__, bitLength);
		return -1;
	}

	// skip 1 byte (image descriptor)

	ptr++;
	if((int)(ptr - data) > dataLength) {
		fprintf(stderr, "[%s():%i] error - detected data overrun with %i vs "
				"%i.\n", __FUNCTION__, __LINE__, (int)(ptr - data),
				dataLength);
		return -1;
	}

	// obtain the image ID

	if(imageIdLength > 0) {
		if(((int)(ptr - data) + imageIdLength) > dataLength) {
			fprintf(stderr, "[%s():%i] error - detected data overrun at %i "
					"(image ID) by %i bytes.\n", __FUNCTION__, __LINE__,
					(int)(ptr - data),
					(((int)(ptr - data) + imageIdLength) - dataLength));
			return -1;
		}
		imageId = (unsigned char *)strndup((char *)ptr, imageIdLength);
		ptr += imageIdLength;
		if((int)(ptr - data) > dataLength) {
			fprintf(stderr, "[%s():%i] error - detected data overrun with %i "
					"vs %i.\n", __FUNCTION__, __LINE__, (int)(ptr - data),
					dataLength);
			return -1;
		}
	}

	// process the image

	targa->imageLength = (long int)(targa->width * targa->height * 4);
	targa->image = (unsigned char *)malloc(sizeof(unsigned char) *
			targa->imageLength);

	if((imageType == TGA_IMAGE_TYPE_BGR) || (imageType == TGA_IMAGE_TYPE_BW)) {
		if(bitLength == 16) {
			colorMode = 2;
		}
		else {
			colorMode = (bitLength / 8);
		}
		length = (targa->width * targa->height * colorMode);
		if(((int)(ptr - data) + length) > dataLength) {
			fprintf(stderr, "[%s():%i] error - detected data overrun at %i "
					"(image pixels) by %i bytes.\n", __FUNCTION__, __LINE__,
					(int)(ptr - data),
					(((int)(ptr - data) + length) - dataLength));
			return -1;
		}
		for(ii = 0, nn = 0; ((ii < length) && (nn < targa->imageLength));
				ii += colorMode, nn += 4) {
			if(colorMode == 2) {
				memcpy((char *)&sNumber, ptr, sizeof(short));
				pixel[TGA_R] = ctype_ctoi((sNumber & 0x1f) << 3);
				pixel[TGA_G] = ctype_ctoi(((sNumber >> 5) & 0x1f) << 3);
				pixel[TGA_B] = ctype_ctoi(((sNumber >> 10) & 0x1f) << 3);
				pixel[TGA_A] = 255;
			}
			else {
				pixel[TGA_R] = ctype_ctoi(ptr[2]);
				pixel[TGA_G] = ctype_ctoi(ptr[1]);
				pixel[TGA_B] = ctype_ctoi(ptr[0]);
				if(colorMode == 3) {
					pixel[TGA_A] = 255;
				}
				else {
					pixel[TGA_A] = ctype_ctoi(ptr[3]);
				}
			}

			targa->image[(nn + 0)] = (unsigned char)pixel[TGA_R];
			targa->image[(nn + 1)] = (unsigned char)pixel[TGA_G];
			targa->image[(nn + 2)] = (unsigned char)pixel[TGA_B];
			targa->image[(nn + 3)] = (unsigned char)pixel[TGA_A];

			ptr += colorMode;
		}
	}
	else { // RLE image
		ii = 0;
		nn = 0;
		rleId = 0;
		colorMode = (bitLength / 8);
		length = (targa->width * targa->height);
		while(ii < length) {
			rleId = (int)ptr[0];
			ptr++;
			if((int)(ptr - data) > dataLength) {
				fprintf(stderr, "[%s():%i] error - detected data overrun with "
						"%i vs %i.\n", __FUNCTION__, __LINE__,
						(int)(ptr - data), dataLength);
				return -1;
			}

			if(rleId < 128) {
				rleId++;
				while(rleId > 0) {
					pixel[TGA_R] = ctype_ctoi(ptr[2]);
					pixel[TGA_G] = ctype_ctoi(ptr[1]);
					pixel[TGA_B] = ctype_ctoi(ptr[0]);
					if(colorMode == 3) {
						pixel[TGA_A] = 255;
					}
					else {
						pixel[TGA_A] = ctype_ctoi(ptr[3]);
					}

					targa->image[(nn + 0)] = (unsigned char)pixel[TGA_R];
					targa->image[(nn + 1)] = (unsigned char)pixel[TGA_G];
					targa->image[(nn + 2)] = (unsigned char)pixel[TGA_B];
					targa->image[(nn + 3)] = (unsigned char)pixel[TGA_A];

					rleId--;
					ii++;
					nn += 4;
					if(nn >= targa->imageLength) {
						break;
					}
					ptr += colorMode;
					if((int)(ptr - data) > dataLength) {
						fprintf(stderr, "[%s():%i] error - detected data "
								"overrun with %i vs %i.\n", __FUNCTION__,
								__LINE__, (int)(ptr - data), dataLength);
						return -1;
					}
				}
			}
			else {
				pixel[TGA_R] = ctype_ctoi(ptr[2]);
				pixel[TGA_G] = ctype_ctoi(ptr[1]);
				pixel[TGA_B] = ctype_ctoi(ptr[0]);
				if(colorMode == 3) {
					pixel[TGA_A] = 255;
				}
				else {
					pixel[TGA_A] = ctype_ctoi(ptr[3]);
				}
				ptr += colorMode;
				if((int)(ptr - data) > dataLength) {
					fprintf(stderr, "[%s():%i] error - detected data overrun "
							"with %i vs %i.\n", __FUNCTION__, __LINE__,
							(int)(ptr - data), dataLength);
					return -1;
				}

				rleId -= 127;
				while(rleId > 0) {
					targa->image[(nn + 0)] = (unsigned char)pixel[TGA_R];
					targa->image[(nn + 1)] = (unsigned char)pixel[TGA_G];
					targa->image[(nn + 2)] = (unsigned char)pixel[TGA_B];
					targa->image[(nn + 3)] = (unsigned char)pixel[TGA_A];

					rleId--;
					ii++;
					nn += 4;
					if(nn >= targa->imageLength) {
						break;
					}
				}
			}
			if(nn >= targa->imageLength) {
				break;
			}
		}
	}

	if(imageId != NULL) {
		free(imageId);
	}

	return 0;
}

int targa_setAlphaChannel(Targa *targa, unsigned char value)
{
	int ii = 0;

	if((targa == NULL) || (targa->image == NULL) || (targa->imageLength < 1)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	for(ii = 3; ii < targa->imageLength; ii += 4) {
		targa->image[ii] = value;
	}

	return 0;
}

