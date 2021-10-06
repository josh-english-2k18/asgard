/*
 * targa.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * A plugin to read targa (TGA) image files into an OpenGL-compatible RGBA
 * format, header file.
 *
 * Written by Josh English.
 */

#if !defined(_ASGARD_ENGINE_PLUGIN_TARGA_H)

#define _ASGARD_ENGINE_PLUGIN_TARGA_H

#if !defined(_ASGARD_H) && !defined(_ASGARD_ENGINE_PLUGIN_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _ASGARD_H || _ASGARD_ENGINE_PLUGIN_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// define targa public data types

typedef struct _Targa {
	int width;
	int height;
	int imageLength;
	unsigned char *image;
} Targa;

// declare targa public functions

int targa_init(Targa *targa);

int targa_free(Targa *targa);

int targa_getDimensions(Targa *targa, int *width, int *height);

int targa_getImageLength(Targa *targa, int *imageLength);

int targa_getRgbaTexture(Targa *targa, char **texture, int *textureLength);

int targa_loadFromFile(Targa *targa, char *filename);

int targa_loadFromData(Targa *targa, unsigned char *data, int dataLength);

int targa_setAlphaChannel(Targa *targa, unsigned char value);


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _ASGARD_ENGINE_PLUGIN_TARGA_H

