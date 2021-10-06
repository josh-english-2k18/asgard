/*
 * texture.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard Game Engine 2D texture system, header file.
 *
 * Written by Josh English.
 */

#if !defined(_ASGARD_ENGINE_TEXTURE_H)

#define _ASGARD_ENGINE_TEXTURE_H

#if !defined(_ASGARD_H) && !defined(_ASGARD_ENGINE_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _ASGARD_H || _ASGARD_ENGINE_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

// define texture public constants

typedef enum _TextureQuality {
	TEXTURE_QUALITY_LOWEST = 1,
	TEXTURE_QUALITY_LOW,
	TEXTURE_QUALITY_MEDIUM,
	TEXTURE_QUALITY_MEDIUM_HIGH,
	TEXTURE_QUALITY_HIGH,
	TEXTURE_QUALITY_HIGHEST,
	TEXTURE_QUALITY_NONE = -1
} TextureQuality;

typedef enum _TextureRender {
	TEXTURE_RENDER_CLAMP = 1,
	TEXTURE_RENDER_CLAMP_TO_BORDER,
	TEXTURE_RENDER_CLAMP_TO_EDGE,
	TEXTURE_RENDER_MIRRORED_REPEAT,
	TEXTURE_RENDER_REPEAT,
	TEXTURE_RENDER_NONE = -1
} TextureRender;

typedef enum TextureColor {
	TEXTURE_COLOR_RED = 1,
	TEXTURE_COLOR_GREEN,
	TEXTURE_COLOR_BLUE,
	TEXTURE_COLOR_ALPHA,
	TEXTURE_COLOR_NONE = -1
} TextureColor;

typedef enum _TextureBlendSource {
	TEXTURE_BLEND_SOURCE_SRC_COLOR = 1,
	TEXTURE_BLEND_SOURCE_ONE_MINUS_SRC_COLOR,
	TEXTURE_BLEND_SOURCE_DST_COLOR,
	TEXTURE_BLEND_SOURCE_ONE_MINUS_DST_COLOR,
	TEXTURE_BLEND_SOURCE_SRC_ALPHA,
	TEXTURE_BLEND_SOURCE_ONE_MINUS_SRC_ALPHA,
	TEXTURE_BLEND_SOURCE_DST_ALPHA,
	TEXTURE_BLEND_SOURCE_ONE_MINUS_DST_ALPHA,
	TEXTURE_BLEND_SOURCE_NONE = -1
} TextureBlendSource;

typedef enum _TextureBlendDest {
	TEXTURE_BLEND_DEST_SRC_COLOR = 1,
	TEXTURE_BLEND_DEST_ONE_MINUS_SRC_COLOR,
	TEXTURE_BLEND_DEST_DST_COLOR,
	TEXTURE_BLEND_DEST_ONE_MINUS_DST_COLOR,
	TEXTURE_BLEND_DEST_SRC_ALPHA,
	TEXTURE_BLEND_DEST_ONE_MINUS_SRC_ALPHA,
	TEXTURE_BLEND_DEST_DST_ALPHA,
	TEXTURE_BLEND_DEST_ONE_MINUS_DST_ALPHA,
	TEXTURE_BLEND_DEST_NONE = -1
} TextureBlendDest;

typedef enum _TextureColorMaskMode {
	TEXTURE_COLORMASK_MODE_ADD = 1,
	TEXTURE_COLORMASK_MODE_MODULATE,
	TEXTURE_COLORMASK_MODE_DECAL,
	TEXTURE_COLORMASK_MODE_BLEND,
	TEXTURE_COLORMASK_MODE_REPLACE,
	TEXTURE_COLORMASK_MODE_COMBINE,
	TEXTURE_COLORMASK_MODE_NONE = -1
} TextureColorMaskMode;


// define texture public data types

typedef struct _Texture {
	aboolean isBlended;
	aboolean hasColorMask;
	aboolean hardwareResident;
	aboolean isOffscreenRender;
	aboolean renderIsTexture2d;
	aboolean renderIsBlendEnabled;
	int id;
	int width;
	int height;
	int imageLength;
	int renderMode;
	int qualityLevel;
	int blendSourceType;
	int blendDestType;
	int colorMaskMode;
	unsigned char *image;
	Color colorMask;
} Texture;


// declare texture public functions

int texture_initAsEmpty(Texture *texture, int qualityLevel,
		int renderMode, aboolean hardwareResident, int width, int height);

int texture_initAsOffscreen(Texture *texture, int qualityLevel,
		int renderMode, aboolean hardwareResident, int width, int height,
		Color *color);

int texture_initFromColor(Texture *texture, int qualityLevel,
		int renderMode, aboolean hardwareResident, int width, int height,
		Color *color);

int texture_initFromRgbaBuffer(Texture *texture, int qualityLevel,
		int renderMode, aboolean hardwareResident, char *buffer,
		int bufferLength, int width, int height);

int texture_initFromTargaFile(Texture *texture, int qualityLevel,
		int renderMode, aboolean hardwareResident, char *filename);

int texture_initFromAsgardTextureFile(Texture *texture, int qualityLevel,
		int renderMode, aboolean hardwareResident, char *filename);

Texture *texture_load(Log *log, char *assetFilename);

int texture_free(Texture *texture);

int texture_freePtr(Texture *texture);

int texture_saveAsAsgardTextureFile(Texture *texture, char *filename);

int texture_setHardwareResident(Texture *texture, aboolean hardwareResident);

int texture_setQualityLevel(Texture *texture, int qualityLevel);

int texture_setRenderMode(Texture *texture, int renderMode);

int texture_setGlProperty(Texture *texture, int property, int value);

int texture_setBlend(Texture *texture, int blendSource, int blendDest);

int texture_setColorMask(Texture *texture, Color *colorMask, int maskMode);

int texture_getDimensions(Texture *texture, int *width, int *height);

int texture_applyRgbaChange(Texture *texture, int colorType,
		unsigned char value);

int texture_setRgbaChannel(Texture *texture, int colorType,
		unsigned char value);

int texture_apply(Texture *texture);

int texture_unApply(Texture *texture);

int texture_toggleRender2dToTexture(Texture *texture);

int texture_toggleRender3dToTexture(Texture *texture);

int texture_endRenderToTexture(Texture *texture, Canvas *canvas);

int texture_draw2d(Texture *texture, int x, int y, int width, int height);


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _ASGARD_ENGINE_TEXTURE_H

