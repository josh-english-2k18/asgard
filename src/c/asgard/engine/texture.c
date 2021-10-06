/*
 * texture.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard Game Engine 2D texture system.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#define _ASGARD_CORE_COMPONENT
#include "asgard/core/common.h"
#include "asgard/core/colors.h"
#define _ASGARD_CORE_PLUGIN_COMPONENT
#include "asgard/core/plugin/opengl.h"
#include "asgard/core/canvas.h"
#define _ASGARD_ENGINE_COMPONENT
#include "asgard/engine/texture.h"
#define _ASGARD_ENGINE_PLUGIN_COMPONENT
#include "asgard/engine/plugin/targa.h"


// declare texture private functions

static void initTexture(Texture *texture);

static int checkForOpenGlError();

static int buildOpenGlTexture(Texture *texture, int qualityLevel,
		int renderMode, aboolean hardwareResident, aboolean isOffscreenRender);

static int getGlBlendSource(int blendSource);

static int getGlBlendDest(int blendDest);


// define texture private functions

static void initTexture(Texture *texture)
{
	memset((void *)texture, 0, sizeof(Texture));

	texture->isBlended = afalse;
	texture->hasColorMask = afalse;
	texture->hardwareResident = afalse;
	texture->renderIsTexture2d = afalse;
	texture->renderIsBlendEnabled = afalse;

	texture->id = -1;
	texture->width = 0;
	texture->height = 0;
	texture->imageLength = 0;
	texture->renderMode = 0;
	texture->qualityLevel = 0;
	texture->blendSourceType = 0;
	texture->blendSourceType = TEXTURE_BLEND_SOURCE_NONE;
	texture->blendDestType = TEXTURE_BLEND_DEST_NONE;
	texture->colorMaskMode = GL_REPLACE;

	texture->image = NULL;

	texture->colorMask.r = 0.0;
	texture->colorMask.g = 0.0;
	texture->colorMask.b = 0.0;
	texture->colorMask.a = 0.0;
}

static int checkForOpenGlError()
{
	char *errorMessage = NULL;

	GLenum rc = (GLenum)0;

	if((rc = cgi_glGetError()) != (GLenum)0) {
		switch(rc) {
			case GL_INVALID_ENUM:
				errorMessage = "invalid enumertor";
				break;

			case GL_INVALID_VALUE:
				errorMessage = "invalid value";
				break;

			case GL_INVALID_OPERATION:
				errorMessage = "invalid operation";
				break;

			default:
				errorMessage = "unknown error";
		}

		fprintf(stderr, "error - texture system detected OpenGL error '%s'.\n",
				errorMessage);

		return -1;
	}

	return 0;
}

static int buildOpenGlTexture(Texture *texture, int qualityLevel,
		int renderMode, aboolean hardwareResident, aboolean isOffscreenRender)
{
	GLuint textureId = (GLuint)0;

	// check texture properties

	if((texture->width % 2) != 0) {
		texture->width -= 1;
	}
	if((texture->height % 2) != 0) {
		texture->height -= 1;
	}

	// build texture

	cgi_glGenTextures(1, &textureId);
	cgi_glBindTexture(GL_TEXTURE_2D, textureId);

	cgi_glTexImage2D(
			GL_TEXTURE_2D,			// texture type
			0,						// level of detail
			GL_RGBA,				// internal format
			texture->width,			// image width
			texture->height,		// image height
			0,						// border width
			GL_RGBA,				// image format
			GL_UNSIGNED_BYTE,		// image type
			texture->image			// image bytes
			);

	if(checkForOpenGlError() < 0) {
		return -1;
	}

	// determine if is offscreen-render texture

	if(isOffscreenRender) {
		cgi_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		cgi_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		if(checkForOpenGlError() < 0) {
			return -1;
		}

		cgi_glBindTexture(GL_TEXTURE_2D, 0);

		texture->id = (int)textureId;

		return 0;
	}

	// set texture properties

	// quality
#if !defined(__ANDROID__)
	switch(qualityLevel) {
		case TEXTURE_QUALITY_LOWEST:
			cgi_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
					GL_NEAREST);
			cgi_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
					GL_NEAREST);
			cgi_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_LOD, -800);
			cgi_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, 800);
			break;

		case TEXTURE_QUALITY_LOW:
			cgi_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
					GL_LINEAR);
			cgi_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
					GL_NEAREST);
			cgi_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_LOD, -900);
			cgi_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, 900);
			break;

		case TEXTURE_QUALITY_MEDIUM:
			cgi_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
					GL_NEAREST_MIPMAP_NEAREST);
			cgi_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
					GL_LINEAR);
			cgi_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_LOD, -1000);
			cgi_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, 1000);
			break;

		case TEXTURE_QUALITY_MEDIUM_HIGH:
			cgi_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
					GL_LINEAR_MIPMAP_NEAREST);
			cgi_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
					GL_LINEAR);
			cgi_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_LOD, -1200);
			cgi_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, 1200);
			break;

		case TEXTURE_QUALITY_HIGH:
			cgi_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
					GL_NEAREST_MIPMAP_LINEAR);
			cgi_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
					GL_LINEAR);
			cgi_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_LOD, -1400);
			cgi_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, 1400);
			break;

		case TEXTURE_QUALITY_HIGHEST:
			cgi_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
					GL_LINEAR_MIPMAP_LINEAR);
			cgi_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
					GL_LINEAR);
			cgi_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_LOD, -1600);
			cgi_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, 1600);
			break;

		default:
			cgi_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
					GL_NEAREST_MIPMAP_NEAREST);
			cgi_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
					GL_LINEAR);
			cgi_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_LOD, -1000);
			cgi_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, 1000);
	}
#else // __ANDROID__
	cgi_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	cgi_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#endif // !__ANDROID__

	if(checkForOpenGlError() < 0) {
		return -1;
	}

	// render mode

#if !defined(__ANDROID__)
	switch(renderMode) {
		case TEXTURE_RENDER_CLAMP:
			cgi_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			cgi_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
			cgi_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP);
			break;

		case TEXTURE_RENDER_CLAMP_TO_BORDER:
			cgi_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
					GL_CLAMP_TO_BORDER);
			cgi_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
					GL_CLAMP_TO_BORDER);
			cgi_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R,
					GL_CLAMP_TO_BORDER);
			break;

		case TEXTURE_RENDER_CLAMP_TO_EDGE:
			cgi_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
					GL_CLAMP_TO_EDGE);
			cgi_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
					GL_CLAMP_TO_EDGE);
			cgi_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R,
					GL_CLAMP_TO_EDGE);
			break;

		case TEXTURE_RENDER_MIRRORED_REPEAT:
			cgi_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
					GL_MIRRORED_REPEAT);
			cgi_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
					GL_MIRRORED_REPEAT);
			cgi_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R,
					GL_MIRRORED_REPEAT);
			break;

		case TEXTURE_RENDER_REPEAT:
			cgi_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			cgi_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			cgi_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT);
			break;

		default:
			cgi_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			cgi_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			cgi_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT);
			break;
	}
#else // __ANDROID__
	cgi_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	cgi_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	cgi_glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
#endif // !__ANDROID__

	if(checkForOpenGlError() < 0) {
		return -1;
	}

	// hardware resident

#if !defined(__ANDROID__)
	if(hardwareResident) {
		cgi_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_PRIORITY, 1);
	}
	else {
		cgi_glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_PRIORITY, 0);
	}
#endif // !__ANDROID__

	// mipmap regeneration

	cgi_glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);

	// build texture mipmaps

	if((qualityLevel == TEXTURE_QUALITY_MEDIUM) ||
			(qualityLevel == TEXTURE_QUALITY_MEDIUM_HIGH) ||
			(qualityLevel == TEXTURE_QUALITY_HIGH) ||
			(qualityLevel == TEXTURE_QUALITY_HIGHEST)) {
		cgi_gluBuild2DMipmaps(
				GL_TEXTURE_2D,				// texture type
				GL_RGBA,					// internal format
				texture->width,				// texture width
				texture->height,			// texture height
				GL_RGBA,					// image format
				GL_UNSIGNED_BYTE,			// image type
				texture->image				// image bytes
				);

		if(checkForOpenGlError() < 0) {
			return -1;
		}
	}

	// don't blend the texture with the current color

	cgi_glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, texture->colorMaskMode);

	// unbind the texture

	cgi_glBindTexture(GL_TEXTURE_2D, 0);

	texture->id = (int)textureId;

	return 0;
}

static int getGlBlendSource(int blendSource)
{
	int result = TEXTURE_BLEND_SOURCE_NONE;

	switch(blendSource) {
		case TEXTURE_BLEND_SOURCE_SRC_COLOR:
			result = GL_SRC_COLOR;
			break;
		case TEXTURE_BLEND_SOURCE_ONE_MINUS_SRC_COLOR:
			result = GL_ONE_MINUS_SRC_COLOR;
			break;
		case TEXTURE_BLEND_SOURCE_DST_COLOR:
			result = GL_DST_COLOR;
			break;
		case TEXTURE_BLEND_SOURCE_ONE_MINUS_DST_COLOR:
			result = GL_ONE_MINUS_DST_COLOR;
			break;
		case TEXTURE_BLEND_SOURCE_SRC_ALPHA:
			result = GL_SRC_ALPHA;
			break;
		case TEXTURE_BLEND_SOURCE_ONE_MINUS_SRC_ALPHA:
			result = GL_ONE_MINUS_SRC_ALPHA;
			break;
		case TEXTURE_BLEND_SOURCE_DST_ALPHA:
			result = GL_DST_ALPHA;
			break;
		case TEXTURE_BLEND_SOURCE_ONE_MINUS_DST_ALPHA:
			result = GL_ONE_MINUS_DST_ALPHA;
			break;
	}

	return result;
}

static int getGlBlendDest(int blendDest)
{
	int result = TEXTURE_BLEND_DEST_NONE;

	switch(blendDest) {
		case TEXTURE_BLEND_DEST_SRC_COLOR:
			result = GL_SRC_COLOR;
			break;
		case TEXTURE_BLEND_DEST_ONE_MINUS_SRC_COLOR:
			result = GL_ONE_MINUS_SRC_COLOR;
			break;
		case TEXTURE_BLEND_DEST_DST_COLOR:
			result = GL_DST_COLOR;
			break;
		case TEXTURE_BLEND_DEST_ONE_MINUS_DST_COLOR:
			result = GL_ONE_MINUS_DST_COLOR;
			break;
		case TEXTURE_BLEND_DEST_SRC_ALPHA:
			result = GL_SRC_ALPHA;
			break;
		case TEXTURE_BLEND_DEST_ONE_MINUS_SRC_ALPHA:
			result = GL_ONE_MINUS_SRC_ALPHA;
			break;
		case TEXTURE_BLEND_DEST_DST_ALPHA:
			result = GL_DST_ALPHA;
			break;
		case TEXTURE_BLEND_DEST_ONE_MINUS_DST_ALPHA:
			result = GL_ONE_MINUS_DST_ALPHA;
			break;
	}

	return result;
}


// define texture public functions

int texture_initAsEmpty(Texture *texture, int qualityLevel,
		int renderMode, aboolean hardwareResident, int width, int height)
{
	if((texture == NULL) ||
			((qualityLevel != TEXTURE_QUALITY_LOWEST) &&
			 (qualityLevel != TEXTURE_QUALITY_LOW) &&
			 (qualityLevel != TEXTURE_QUALITY_MEDIUM) &&
			 (qualityLevel != TEXTURE_QUALITY_MEDIUM_HIGH) &&
			 (qualityLevel != TEXTURE_QUALITY_HIGH) &&
			 (qualityLevel != TEXTURE_QUALITY_HIGHEST)) ||
			((renderMode != TEXTURE_RENDER_CLAMP) &&
			 (renderMode != TEXTURE_RENDER_CLAMP_TO_BORDER) &&
			 (renderMode != TEXTURE_RENDER_CLAMP_TO_EDGE) &&
			 (renderMode != TEXTURE_RENDER_MIRRORED_REPEAT) &&
			 (renderMode != TEXTURE_RENDER_REPEAT)) ||
			(width < 1) || (height < 1)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	initTexture(texture);

	texture->isBlended = afalse;
	texture->hasColorMask = afalse;
	texture->hardwareResident = hardwareResident;
	texture->isOffscreenRender = afalse;
	texture->renderIsTexture2d = afalse;
	texture->renderIsBlendEnabled = afalse;
	texture->id = -1;
	texture->width = width;
	texture->height = height;
	texture->imageLength = (width * height * 4);
	texture->renderMode = renderMode;
	texture->qualityLevel = qualityLevel;
	texture->blendSourceType = TEXTURE_BLEND_SOURCE_NONE;
	texture->blendDestType = TEXTURE_BLEND_DEST_NONE;
	texture->colorMaskMode = GL_REPLACE;
	texture->image = NULL;

	if(buildOpenGlTexture(texture, texture->qualityLevel, texture->renderMode,
				texture->hardwareResident, texture->isOffscreenRender) < 0) {
		texture_free(texture);
		return -1;
	}

	return 0;
}

int texture_initAsOffscreen(Texture *texture, int qualityLevel,
		int renderMode, aboolean hardwareResident, int width, int height,
		Color *color)
{
	int ii = 0;

	if((texture == NULL) ||
			((qualityLevel != TEXTURE_QUALITY_LOWEST) &&
			 (qualityLevel != TEXTURE_QUALITY_LOW) &&
			 (qualityLevel != TEXTURE_QUALITY_MEDIUM) &&
			 (qualityLevel != TEXTURE_QUALITY_MEDIUM_HIGH) &&
			 (qualityLevel != TEXTURE_QUALITY_HIGH) &&
			 (qualityLevel != TEXTURE_QUALITY_HIGHEST)) ||
			((renderMode != TEXTURE_RENDER_CLAMP) &&
			 (renderMode != TEXTURE_RENDER_CLAMP_TO_BORDER) &&
			 (renderMode != TEXTURE_RENDER_CLAMP_TO_EDGE) &&
			 (renderMode != TEXTURE_RENDER_MIRRORED_REPEAT) &&
			 (renderMode != TEXTURE_RENDER_REPEAT)) ||
			(width < 1) || (height < 1) || (color == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	initTexture(texture);

	texture->isBlended = afalse;
	texture->hasColorMask = afalse;
	texture->hardwareResident = hardwareResident;
	texture->isOffscreenRender = atrue;
	texture->renderIsTexture2d = afalse;
	texture->renderIsBlendEnabled = afalse;
	texture->id = -1;
	texture->width = width;
	texture->height = height;
	texture->imageLength = (width * height * 4);
	texture->renderMode = renderMode;
	texture->qualityLevel = qualityLevel;
	texture->blendSourceType = TEXTURE_BLEND_SOURCE_NONE;
	texture->blendDestType = TEXTURE_BLEND_DEST_NONE;
	texture->colorMaskMode = GL_REPLACE;
	texture->image = (unsigned char *)malloc(sizeof(unsigned char) *
			texture->imageLength);

	for(ii = 0; ii < texture->imageLength; ii += 4) {
		texture->image[(ii + 0)] = (unsigned char)(color->r * 255.0);
		texture->image[(ii + 1)] = (unsigned char)(color->g * 255.0);
		texture->image[(ii + 2)] = (unsigned char)(color->b * 255.0);
		texture->image[(ii + 3)] = (unsigned char)(color->a * 255.0);
	}

	if(buildOpenGlTexture(texture, texture->qualityLevel, texture->renderMode,
				texture->hardwareResident, texture->isOffscreenRender) < 0) {
		texture_free(texture);
		return -1;
	}

	return 0;
}

int texture_initFromColor(Texture *texture, int qualityLevel,
		int renderMode, aboolean hardwareResident, int width, int height,
		Color *color)
{
	int ii = 0;

	if((texture == NULL) ||
			((qualityLevel != TEXTURE_QUALITY_LOWEST) &&
			 (qualityLevel != TEXTURE_QUALITY_LOW) &&
			 (qualityLevel != TEXTURE_QUALITY_MEDIUM) &&
			 (qualityLevel != TEXTURE_QUALITY_MEDIUM_HIGH) &&
			 (qualityLevel != TEXTURE_QUALITY_HIGH) &&
			 (qualityLevel != TEXTURE_QUALITY_HIGHEST)) ||
			((renderMode != TEXTURE_RENDER_CLAMP) &&
			 (renderMode != TEXTURE_RENDER_CLAMP_TO_BORDER) &&
			 (renderMode != TEXTURE_RENDER_CLAMP_TO_EDGE) &&
			 (renderMode != TEXTURE_RENDER_MIRRORED_REPEAT) &&
			 (renderMode != TEXTURE_RENDER_REPEAT)) ||
			(width < 1) || (height < 1) || (color == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	initTexture(texture);

	texture->isBlended = afalse;
	texture->hasColorMask = afalse;
	texture->hardwareResident = hardwareResident;
	texture->isOffscreenRender = afalse;
	texture->renderIsTexture2d = afalse;
	texture->renderIsBlendEnabled = afalse;
	texture->id = -1;
	texture->width = width;
	texture->height = height;
	texture->imageLength = (width * height * 4);
	texture->renderMode = renderMode;
	texture->qualityLevel = qualityLevel;
	texture->blendSourceType = TEXTURE_BLEND_SOURCE_NONE;
	texture->blendDestType = TEXTURE_BLEND_DEST_NONE;
	texture->colorMaskMode = GL_REPLACE;
	texture->image = (unsigned char *)malloc(sizeof(unsigned char) *
			texture->imageLength);

	for(ii = 0; ii < texture->imageLength; ii += 4) {
		texture->image[(ii + 0)] = (unsigned char)(color->r * 255.0);
		texture->image[(ii + 1)] = (unsigned char)(color->g * 255.0);
		texture->image[(ii + 2)] = (unsigned char)(color->b * 255.0);
		texture->image[(ii + 3)] = (unsigned char)(color->a * 255.0);
	}

	if(buildOpenGlTexture(texture, texture->qualityLevel, texture->renderMode,
				texture->hardwareResident, texture->isOffscreenRender) < 0) {
		texture_free(texture);
		return -1;
	}

	return 0;
}

int texture_initFromRgbaBuffer(Texture *texture, int qualityLevel,
		int renderMode, aboolean hardwareResident, char *buffer,
		int bufferLength, int width, int height)
{
	if((texture == NULL) ||
			((qualityLevel != TEXTURE_QUALITY_LOWEST) &&
			 (qualityLevel != TEXTURE_QUALITY_LOW) &&
			 (qualityLevel != TEXTURE_QUALITY_MEDIUM) &&
			 (qualityLevel != TEXTURE_QUALITY_MEDIUM_HIGH) &&
			 (qualityLevel != TEXTURE_QUALITY_HIGH) &&
			 (qualityLevel != TEXTURE_QUALITY_HIGHEST)) ||
			((renderMode != TEXTURE_RENDER_CLAMP) &&
			 (renderMode != TEXTURE_RENDER_CLAMP_TO_BORDER) &&
			 (renderMode != TEXTURE_RENDER_CLAMP_TO_EDGE) &&
			 (renderMode != TEXTURE_RENDER_MIRRORED_REPEAT) &&
			 (renderMode != TEXTURE_RENDER_REPEAT)) ||
			(buffer == NULL) || (bufferLength < 1) ||
			(width < 1) || (height < 1)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	initTexture(texture);

	texture->isBlended = afalse;
	texture->hasColorMask = afalse;
	texture->hardwareResident = hardwareResident;
	texture->isOffscreenRender = afalse;
	texture->renderIsTexture2d = afalse;
	texture->renderIsBlendEnabled = afalse;
	texture->id = -1;
	texture->width = width;
	texture->height = height;
	texture->imageLength = bufferLength;
	texture->renderMode = renderMode;
	texture->qualityLevel = qualityLevel;
	texture->blendSourceType = TEXTURE_BLEND_SOURCE_NONE;
	texture->blendDestType = TEXTURE_BLEND_DEST_NONE;
	texture->colorMaskMode = GL_REPLACE;
	texture->image = (unsigned char *)strndup(buffer, bufferLength);

	if(buildOpenGlTexture(texture, texture->qualityLevel, texture->renderMode,
				texture->hardwareResident, texture->isOffscreenRender) < 0) {
		texture_free(texture);
		return -1;
	}

	return 0;
}

int texture_initFromTargaFile(Texture *texture, int qualityLevel,
		int renderMode, aboolean hardwareResident, char *filename)
{
	Targa targa;

	if((texture == NULL) ||
			((qualityLevel != TEXTURE_QUALITY_LOWEST) &&
			 (qualityLevel != TEXTURE_QUALITY_LOW) &&
			 (qualityLevel != TEXTURE_QUALITY_MEDIUM) &&
			 (qualityLevel != TEXTURE_QUALITY_MEDIUM_HIGH) &&
			 (qualityLevel != TEXTURE_QUALITY_HIGH) &&
			 (qualityLevel != TEXTURE_QUALITY_HIGHEST)) ||
			((renderMode != TEXTURE_RENDER_CLAMP) &&
			 (renderMode != TEXTURE_RENDER_CLAMP_TO_BORDER) &&
			 (renderMode != TEXTURE_RENDER_CLAMP_TO_EDGE) &&
			 (renderMode != TEXTURE_RENDER_MIRRORED_REPEAT) &&
			 (renderMode != TEXTURE_RENDER_REPEAT)) ||
			(filename == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	initTexture(texture);

	targa_init(&targa);

	if(targa_loadFromFile(&targa, filename) < 0) {
		targa_free(&targa);
		return -1;
	}

	texture->isBlended = afalse;
	texture->hasColorMask = afalse;
	texture->hardwareResident = hardwareResident;
	texture->isOffscreenRender = afalse;
	texture->renderIsTexture2d = afalse;
	texture->renderIsBlendEnabled = afalse;
	texture->id = -1;
	texture->width = targa.width;
	texture->height = targa.height;
	texture->imageLength = targa.imageLength;
	texture->renderMode = renderMode;
	texture->qualityLevel = qualityLevel;
	texture->blendSourceType = TEXTURE_BLEND_SOURCE_NONE;
	texture->blendDestType = TEXTURE_BLEND_DEST_NONE;
	texture->colorMaskMode = GL_REPLACE;
	texture->image = targa.image;

	if(buildOpenGlTexture(texture, texture->qualityLevel, texture->renderMode,
				texture->hardwareResident, texture->isOffscreenRender) < 0) {
		texture_free(texture);
		return -1;
	}

	return 0;
}

int texture_initFromAsgardTextureFile(Texture *texture, int qualityLevel,
		int renderMode, aboolean hardwareResident, char *filename)
{
	if(texture == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	/*
	 * TODO: this function
	 */

	return -1;
}

Texture *texture_load(Log *log, char *assetFilename)
{
	aboolean bValue = afalse;
	aboolean hasError = afalse;
	aboolean hardwareResident = afalse;
	int ii = 0;
	int width = 0;
	int height = 0;
	int iValue = 0;
	int qualityLevel = 0;
	int renderMode = 0;
	int glProperty = 0;
	int glValue = 0;
	float glFloatArray[4];
	char buffer[1024];
	char *sValue = NULL;
	char *sProperty = NULL;
	char *loadType = NULL;

	Color color;
	Config config;
	Texture *result = NULL;

	if((log == NULL) || (assetFilename == NULL)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	if(config_init(&config, assetFilename) < 0) {
		log_logf(log, LOG_LEVEL_ERROR,
				"{TEXTURE} failed to load configuration file '%s'",
				assetFilename);
		return NULL;
	}

	// validate that the configuration file is for a 2D texture asset

	config_getString(&config, "asset", "class", "unknown", &sProperty);
	if(strcmp(sProperty, "2d")) {
		log_logf(log, LOG_LEVEL_ERROR,
				"{TEXTURE} invalid texture configuration - asset.class is '%s'",
				sProperty);
		return NULL;
	}

	config_getString(&config, "asset", "type", "unknown", &sProperty);
	if(strcmp(sProperty, "texture")) {
		log_logf(log, LOG_LEVEL_ERROR,
				"{TEXTURE} invalid texture configuration - asset.type is '%s'",
				sProperty);
		return NULL;
	}

	// load the texture configuration

	result = (Texture *)malloc(sizeof(Texture));

	initTexture(result);

	// quality

	config_getString(&config, "texture", "qualityLevel", "unknown", &sProperty);
	if(!strcmp(sProperty, "lowest")) {
		qualityLevel = TEXTURE_QUALITY_LOWEST;
	}
	else if(!strcmp(sProperty, "low")) {
		qualityLevel = TEXTURE_QUALITY_LOW;
	}
	else if(!strcmp(sProperty, "medium")) {
		qualityLevel = TEXTURE_QUALITY_MEDIUM;
	}
	else if(!strcmp(sProperty, "medium-high")) {
		qualityLevel = TEXTURE_QUALITY_MEDIUM_HIGH;
	}
	else if(!strcmp(sProperty, "high")) {
		qualityLevel = TEXTURE_QUALITY_HIGH;
	}
	else if(!strcmp(sProperty, "highest")) {
		qualityLevel = TEXTURE_QUALITY_HIGHEST;
	}
	else {
		log_logf(log, LOG_LEVEL_ERROR,
				"{TEXTURE} unknown quality level '%s' for texture '%s'",
				sProperty, assetFilename);
		texture_freePtr(result);
		return NULL;
	}

	// render mode

	config_getString(&config, "texture", "renderMode", "unknown", &sProperty);
	if(!strcmp(sProperty, "clamp")) {
		renderMode = TEXTURE_RENDER_CLAMP;
	}
	else if(!strcmp(sProperty, "clamp-to-border")) {
		renderMode = TEXTURE_RENDER_CLAMP_TO_BORDER;
	}
	else if(!strcmp(sProperty, "clamp-to-edge")) {
		renderMode = TEXTURE_RENDER_CLAMP_TO_EDGE;
	}
	else if(!strcmp(sProperty, "mirrored-repeat")) {
		renderMode = TEXTURE_RENDER_MIRRORED_REPEAT;
	}
	else if(!strcmp(sProperty, "repeat")) {
		renderMode = TEXTURE_RENDER_REPEAT;
	}
	else {
		log_logf(log, LOG_LEVEL_ERROR,
				"{TEXTURE} unknown render mode '%s' for texture '%s'",
				sProperty, assetFilename);
		texture_freePtr(result);
		return NULL;
	}

	// hardware resident

	config_getBoolean(&config, "texture", "hardwareResident", afalse,
			&hardwareResident);

	// load type

	config_getString(&config, "texture", "loadType", "unknown", &loadType);
	if(!strcmp(loadType, "empty")) {
		config_getInteger(&config, "texture.empty", "width", 0, &width);
		config_getInteger(&config, "texture.empty", "height", 0, &height);

		if(texture_initAsEmpty(result, qualityLevel, renderMode,
					hardwareResident, width, height) < 0) {
			log_logf(log, LOG_LEVEL_ERROR,
					"{TEXTURE} failed to create 'empty' texture for '%s'",
					assetFilename);
			texture_freePtr(result);
			return NULL;
		}
	}
	else if(!strcmp(loadType, "color")) {
		config_getInteger(&config, "texture.color", "width", 0, &width);
		config_getInteger(&config, "texture.color", "height", 0, &height);

		config_getBoolean(&config, "texture.color", "loadByName", afalse,
				&bValue);
		if(bValue) {
			config_getString(&config, "texture.color", "name", "unknown",
					&sProperty);
			colors_getColor(sProperty, &color);
		}
		else {
			config_getBoolean(&config, "texture.color", "loadByColor", afalse,
					&bValue);
			if(bValue) {
				config_getDouble(&config, "texture.color", "red", 0.0,
						&color.r);
				config_getDouble(&config, "texture.color", "green", 0.0,
						&color.g);
				config_getDouble(&config, "texture.color", "blue", 0.0,
						&color.b);
				config_getDouble(&config, "texture.color", "alpha", 0.0,
						&color.a);
			}
			else {
				log_logf(log, LOG_LEVEL_ERROR, "{TEXTURE} invalid texture "
						"color load properties for '%s'", assetFilename);
				texture_freePtr(result);
				return NULL;
			}
		}

		if(texture_initFromColor(result, qualityLevel, renderMode,
					hardwareResident, width, height, &color) < 0) {
			log_logf(log, LOG_LEVEL_ERROR,
					"{TEXTURE} failed to create 'color' texture for '%s'",
					assetFilename);
			texture_freePtr(result);
			return NULL;
		}
	}
	else if(!strcmp(loadType, "plugin-tga")) {
		config_getString(&config, "texture.plugin.tga", "filename", "unknown",
				&sProperty);
		if(texture_initFromTargaFile(result, qualityLevel, renderMode,
					hardwareResident, sProperty) < 0) {
			log_logf(log, LOG_LEVEL_ERROR,
					"{TEXTURE} failed to create 'plugin-tga' texture for '%s'",
					assetFilename);
			texture_freePtr(result);
			return NULL;
		}
	}
	else if(!strcmp(loadType, "native")) {
		config_getString(&config, "texture.native", "filename", "unknown",
				&sProperty);
		if(texture_initFromAsgardTextureFile(result, qualityLevel, renderMode,
					hardwareResident, sProperty) < 0) {
			log_logf(log, LOG_LEVEL_ERROR,
					"{TEXTURE} failed to create 'native' texture for '%s'",
					assetFilename);
			texture_freePtr(result);
			return NULL;
		}
	}

	// extended GL properties

	config_getBoolean(&config, "texture", "hasGlProperties", afalse, &bValue);
	if(bValue) {
		config_getInteger(&config, "texture.gl.properties", "length", 0,
				&iValue);

		for(ii = 0; ii < iValue; ii++) {
			snprintf(buffer, (sizeof(buffer) - 1), "property%03i", (ii + 1));
			config_getString(&config, "texture.gl.properties", buffer,
					"unknown", &sProperty);

			snprintf(buffer, (sizeof(buffer) - 1), "value%03i", (ii + 1));
			config_getString(&config, "texture.gl.properties", buffer,
					"unknown", &sValue);

			if(!strcmp(sProperty, "TextureMinFilter")) {
				glProperty = GL_TEXTURE_MIN_FILTER;
				if(!strcmp(sValue, "Nearest")) {
					glValue = GL_NEAREST;
				}
				else if(!strcmp(sValue, "Linear")) {
					glValue = GL_LINEAR;
				}
				else if(!strcmp(sValue, "NearestMipmapNearest")) {
					glValue = GL_NEAREST_MIPMAP_NEAREST;
				}
				else if(!strcmp(sValue, "LinearMipmapNearest")) {
					glValue = GL_LINEAR_MIPMAP_NEAREST;
				}
				else if(!strcmp(sValue, "NearestMipmapLinear")) {
					glValue = GL_NEAREST_MIPMAP_LINEAR;
				}
				else if(!strcmp(sValue, "LinearMipmapLinear")) {
					glValue = GL_LINEAR_MIPMAP_LINEAR;
				}
				else {
					log_logf(log, LOG_LEVEL_ERROR, "{TEXTURE} unrecognized GL "
							"property '%s' value '%s' for texture '%s'",
							sProperty, sValue, assetFilename);
					continue;
				}
			}
			else if(!strcmp(sProperty, "TextureMagFilter")) {
				glProperty = GL_TEXTURE_MAG_FILTER;
				if(!strcmp(sValue, "Nearest")) {
					glValue = GL_NEAREST;
				}
				else if(!strcmp(sValue, "Linear")) {
					glValue = GL_LINEAR;
				}
				else {
					log_logf(log, LOG_LEVEL_ERROR, "{TEXTURE} unrecognized GL "
							"property '%s' value '%s' for texture '%s'",
							sProperty, sValue, assetFilename);
					continue;
				}
			}
			else if(!strcmp(sProperty, "TextureMinLod")) {
#if !defined(__ANDROID__)
				glProperty = GL_TEXTURE_MIN_LOD;
				glValue = atoi(sValue);
#else // __ANDROID__
				continue;
#endif // !__ANDROID__
			}
			else if(!strcmp(sProperty, "TextureMaxLod")) {
#if !defined(__ANDROID__)
				glProperty = GL_TEXTURE_MAX_LOD;
				glValue = atoi(sValue);
#else // __ANDROID__
				continue;
#endif // !__ANDROID__
			}
			else if(!strcmp(sProperty, "TextureBaseLevel")) {
#if !defined(__ANDROID__)
				glProperty = GL_TEXTURE_BASE_LEVEL;
				glValue = atoi(sValue);
#else // __ANDROID__
				continue;
#endif // !__ANDROID__
			}
			else if(!strcmp(sProperty, "TetxtureMaxLevel")) {
#if !defined(__ANDROID__)
				glProperty = GL_TEXTURE_MAX_LEVEL;
				glValue = atoi(sValue);
#else // __ANDROID__
				continue;
#endif // !__ANDROID__
			}
			else if((!strcmp(sProperty, "TextureWrapS")) ||
					(!strcmp(sProperty, "TextureWrapT")) ||
					(!strcmp(sProperty, "TextureWrapR"))) {
#if !defined(__ANDROID__)
				if(!strcmp(sProperty, "TextureWrapS")) {
					glProperty = GL_TEXTURE_WRAP_S;
				}
				else if(!strcmp(sProperty, "TextureWrapT")) {
					glProperty = GL_TEXTURE_WRAP_T;
				}
				else if(!strcmp(sProperty, "TextureWrapR")) {
					glProperty = GL_TEXTURE_WRAP_R;
				}

				if(!strcmp(sValue, "Clamp")) {
					glValue = GL_CLAMP;
				}
				else if(!strcmp(sValue, "ClampToBorder")) {
					glValue = GL_CLAMP_TO_BORDER;
				}
				else if(!strcmp(sValue, "ClampToEdge")) {
					glValue = GL_CLAMP_TO_EDGE;
				}
				else if(!strcmp(sValue, "MirroredRepeat")) {
					glValue = GL_MIRRORED_REPEAT;
				}
				else if(!strcmp(sValue, "Repeat")) {
					glValue = GL_REPEAT;
				}
				else {
					log_logf(log, LOG_LEVEL_ERROR, "{TEXTURE} unrecognized GL "
							"property '%s' value '%s' for texture '%s'",
							sProperty, sValue, assetFilename);
					continue;
				}
#else // __ANDROID__
				continue;
#endif // !__ANDROID__
			}
			else if(!strcmp(sProperty, "TextureBorderColor")) {
#if !defined(__ANDROID__)
				glProperty = GL_TEXTURE_BORDER_COLOR;
				colors_getColor(sValue, &color);
				glFloatArray[0] = (float)color.r;
				glFloatArray[1] = (float)color.g;
				glFloatArray[2] = (float)color.b;
				glFloatArray[3] = (float)color.a;
#else // __ANDROID__
				continue;
#endif // !__ANDROID__
			}
			else if(!strcmp(sProperty, "TexturePriority")) {
#if !defined(__ANDROID__)
				glProperty = GL_TEXTURE_PRIORITY;
				if(!strcmp(sValue, "true")) {
					glValue = 1;
				}
				else if(!strcmp(sValue, "false")) {
					glValue = 0;
				}
				else {
					log_logf(log, LOG_LEVEL_ERROR, "{TEXTURE} unrecognized GL "
							"property '%s' value '%s' for texture '%s'",
							sProperty, sValue, assetFilename);
					continue;
				}
#else // __ANDROID__
				continue;
#endif // !__ANDROID__
			}
			else if(!strcmp(sProperty, "TextureCompareMode")) {
#if !defined(__ANDROID__)
				glProperty = GL_TEXTURE_COMPARE_MODE;
				if(!strcmp(sValue, "CompareRToTexture")) {
					glValue = GL_COMPARE_R_TO_TEXTURE;
				}
				else if(!strcmp(sValue, "None")) {
					glValue = GL_NONE;
				}
				else {
					log_logf(log, LOG_LEVEL_ERROR, "{TEXTURE} unrecognized GL "
							"property '%s' value '%s' for texture '%s'",
							sProperty, sValue, assetFilename);
					continue;
				}
#else // __ANDROID__
				continue;
#endif // !__ANDROID__
			}
			else if(!strcmp(sProperty, "TextureCompareFunc")) {
#if !defined(__ANDROID__)
				glProperty = GL_TEXTURE_COMPARE_FUNC;
				if(!strcmp(sValue, "Lequal")) {
					glValue = GL_LEQUAL;
				}
				else if(!strcmp(sValue, "Gequal")) {
					glValue = GL_GEQUAL;
				}
				else if(!strcmp(sValue, "Less")) {
					glValue = GL_LESS;
				}
				else if(!strcmp(sValue, "Greater")) {
					glValue = GL_GREATER;
				}
				else if(!strcmp(sValue, "Equal")) {
					glValue = GL_EQUAL;
				}
				else if(!strcmp(sValue, "NotEqual")) {
					glValue = GL_NOTEQUAL;
				}
				else if(!strcmp(sValue, "Always")) {
					glValue = GL_ALWAYS;
				}
				else if(!strcmp(sValue, "Never")) {
					glValue = GL_NEVER;
				}
				else {
					log_logf(log, LOG_LEVEL_ERROR, "{TEXTURE} unrecognized GL "
							"property '%s' value '%s' for texture '%s'",
							sProperty, sValue, assetFilename);
					continue;
				}
#else // __ANDROID__
				continue;
#endif // !__ANDROID__
			}
			else if(!strcmp(sProperty, "DepthTextureMode")) {
#if !defined(__ANDROID__)
				glProperty = GL_DEPTH_TEXTURE_MODE;
				if(!strcmp(sValue, "Luminance")) {
					glValue = GL_LUMINANCE;
				}
				else if(!strcmp(sValue, "Intensity")) {
					glValue = GL_INTENSITY;
				}
				else if(!strcmp(sValue, "Alpha")) {
					glValue = GL_ALPHA;
				}
				else {
					log_logf(log, LOG_LEVEL_ERROR, "{TEXTURE} unrecognized GL "
							"property '%s' value '%s' for texture '%s'",
							sProperty, sValue, assetFilename);
					continue;
				}
#else // __ANDROID__
				continue;
#endif // !__ANDROID__
			}
			else if(!strcmp(sProperty, "GenerateMipmap")) {
				glProperty = GL_GENERATE_MIPMAP;
				if(!strcmp(sValue, "true")) {
					glValue = GL_TRUE;
				}
				else if(!strcmp(sValue, "false")) {
					glValue = GL_FALSE;
				}
				else {
					log_logf(log, LOG_LEVEL_ERROR, "{TEXTURE} unrecognized GL "
							"property '%s' value '%s' for texture '%s'",
							sProperty, sValue, assetFilename);
					continue;
				}
			}
			else {
				log_logf(log, LOG_LEVEL_ERROR, "{TEXTURE} unrecognized GL "
						"property '%s' for texture '%s'",
						sProperty, assetFilename);
				continue;
			}

			cgi_glBindTexture(GL_TEXTURE_2D, (GLuint)result->id);

#if !defined(__ANDROID__)
			if(glProperty == GL_TEXTURE_BORDER_COLOR) {
				cgi_glTexParameterfv(GL_TEXTURE_2D, (GLenum)glProperty,
						(GLfloat *)glFloatArray);
			}
			else {
				cgi_glTexParameteri(GL_TEXTURE_2D, (GLenum)glProperty,
						(GLint)glValue);
			}
#else // __ANDROID__
			cgi_glTexParameteri(GL_TEXTURE_2D, (GLenum)glProperty,
					(GLint)glValue);
#endif // !__ANDROID__

			cgi_glBindTexture(GL_TEXTURE_2D, 0);
		}
	}

	// blending

	config_getBoolean(&config, "texture", "isBlended", afalse, &bValue);
	if(bValue) {
		hasError = afalse;

		config_getString(&config, "texture.blend", "source", "unknown",
				&sProperty);

		if(!strcmp(sProperty, "SourceColor")) {
			glProperty = TEXTURE_BLEND_SOURCE_SRC_COLOR;
		}
		else if(!strcmp(sProperty, "OneMinusSrcColor")) {
			glProperty = TEXTURE_BLEND_SOURCE_ONE_MINUS_SRC_COLOR;
		}
		else if(!strcmp(sProperty, "DestinationColor")) {
			glProperty = TEXTURE_BLEND_SOURCE_DST_COLOR;
		}
		else if(!strcmp(sProperty, "OneMinusDestinationColor")) {
			glProperty = TEXTURE_BLEND_SOURCE_ONE_MINUS_DST_COLOR;
		}
		else if(!strcmp(sProperty, "SourceAlpha")) {
			glProperty = TEXTURE_BLEND_SOURCE_SRC_ALPHA;
		}
		else if(!strcmp(sProperty, "OneMinusSourceAlpha")) {
			glProperty = TEXTURE_BLEND_SOURCE_ONE_MINUS_SRC_ALPHA;
		}
		else if(!strcmp(sProperty, "DestiationAlpha")) {
			glProperty = TEXTURE_BLEND_SOURCE_DST_ALPHA;
		}
		else if(!strcmp(sProperty, "OneMinusDestinationAlpha")) {
			glProperty = TEXTURE_BLEND_SOURCE_ONE_MINUS_DST_ALPHA;
		}
		else {
			log_logf(log, LOG_LEVEL_ERROR,
					"{TEXTURE} unknown blend source '%s' for texture '%s'",
					sProperty, assetFilename);
			hasError = atrue;
		}

		config_getString(&config, "texture.blend", "destination", "unknown",
				&sValue);

		if(!strcmp(sValue, "SourceColor")) {
			glValue = TEXTURE_BLEND_DEST_SRC_COLOR;
		}
		else if(!strcmp(sValue, "OneMinusSourceColor")) {
			glValue = TEXTURE_BLEND_DEST_ONE_MINUS_SRC_COLOR;
		}
		else if(!strcmp(sValue, "DestinationColor")) {
			glValue = TEXTURE_BLEND_DEST_DST_COLOR;
		}
		else if(!strcmp(sValue, "OneMinusDestinationColor")) {
			glValue = TEXTURE_BLEND_DEST_ONE_MINUS_DST_COLOR;
		}
		else if(!strcmp(sValue, "SourceAlpha")) {
			glValue = TEXTURE_BLEND_DEST_SRC_ALPHA;
		}
		else if(!strcmp(sValue, "OneMinusSourceAlpha")) {
			glValue = TEXTURE_BLEND_DEST_ONE_MINUS_SRC_ALPHA;
		}
		else if(!strcmp(sValue, "DestinationAlpha")) {
			glValue = TEXTURE_BLEND_DEST_DST_ALPHA;
		}
		else if(!strcmp(sValue, "OneMinusDestinationAlpha")) {
			glValue = TEXTURE_BLEND_DEST_ONE_MINUS_DST_ALPHA;
		}
		else {
			log_logf(log, LOG_LEVEL_ERROR,
					"{TEXTURE} unknown blend destination '%s' for texture '%s'",
					sValue, assetFilename);
			hasError = atrue;
		}

		if(!hasError) {
			if(texture_setBlend(result, glProperty, glValue) < 0) {
				log_logf(log, LOG_LEVEL_ERROR,
						"{TEXTURE} failed to enable blending for texture '%s'",
						sProperty, assetFilename);
			}
		}
	}

	// color masking

	config_getBoolean(&config, "texture", "hasColormask", afalse, &bValue);
	if(bValue) {
		hasError = afalse;

		config_getBoolean(&config, "texture.colormask", "loadByName", afalse,
				&bValue);
		if(bValue) {
			config_getString(&config, "texture.colormask", "name", "unknown",
					&sProperty);
			colors_getColor(sProperty, &color);
		}
		else {
			config_getBoolean(&config, "texture.colormask", "loadByColor",
					afalse, &bValue);
			if(bValue) {
				config_getDouble(&config, "texture.colormask", "red", 0.0,
						&color.r);
				config_getDouble(&config, "texture.colormask", "green", 0.0,
						&color.g);
				config_getDouble(&config, "texture.colormask", "blue", 0.0,
						&color.b);
				config_getDouble(&config, "texture.colormask", "alpha", 0.0,
						&color.a);
			}
			else {
				log_logf(log, LOG_LEVEL_ERROR, "{TEXTURE} invalid texture "
						"color mask properties for '%s'", assetFilename);
				hasError = atrue;
			}
		}

		if(!hasError) {
			config_getString(&config, "texture.colormask", "maskType",
					"unknown", &sProperty);
			if(!strcmp(sProperty, "Add")) {
				glProperty = TEXTURE_COLORMASK_MODE_ADD;
			}
			else if(!strcmp(sProperty, "Modulate")) {
				glProperty = TEXTURE_COLORMASK_MODE_MODULATE;
			}
			else if(!strcmp(sProperty, "Decal")) {
				glProperty = TEXTURE_COLORMASK_MODE_DECAL;
			}
			else if(!strcmp(sProperty, "Blend")) {
				glProperty = TEXTURE_COLORMASK_MODE_BLEND;
			}
			else if(!strcmp(sProperty, "Replace")) {
				glProperty = TEXTURE_COLORMASK_MODE_REPLACE;
			}
			else if(!strcmp(sProperty, "Combine")) {
				glProperty = TEXTURE_COLORMASK_MODE_COMBINE;
			}
			else {
				log_logf(log, LOG_LEVEL_ERROR, "{TEXTURE} unknown color mask "
						"property '%s' for texture '%s'",
						sProperty, assetFilename);
				hasError = atrue;
			}
		}

		if(!hasError) {
			if(texture_setColorMask(result, &color, glProperty) < 0) {
				log_logf(log, LOG_LEVEL_ERROR, "{TEXTURE} failed to enable "
						"color masking for texture '%s'",
						sProperty, assetFilename);
			}
		}
	}

	// RGBA channel change

	config_getBoolean(&config, "texture", "hasRgbaChange", afalse, &bValue);
	if(bValue) {
		config_getInteger(&config, "texture.apply.rgba.change", "length", 0,
				&iValue);

		for(ii = 0; ii < iValue; ii++) {
			snprintf(buffer, (sizeof(buffer) - 1), "property%03i", (ii + 1));
			config_getString(&config, "texture.apply.rgba.change", buffer,
					"unknown", &sProperty);

			if(!strcmp(sProperty, "red")) {
				glProperty = TEXTURE_COLOR_RED;
			}
			else if(!strcmp(sProperty, "green")) {
				glProperty = TEXTURE_COLOR_GREEN;
			}
			else if(!strcmp(sProperty, "blue")) {
				glProperty = TEXTURE_COLOR_BLUE;
			}
			else if(!strcmp(sProperty, "alpha")) {
				glProperty = TEXTURE_COLOR_ALPHA;
			}
			else {
				log_logf(log, LOG_LEVEL_ERROR, "{TEXTURE} unknown RGBA color "
						"change property '%s' for texture '%s'",
						sProperty, assetFilename);
				continue;
			}

			snprintf(buffer, (sizeof(buffer) - 1), "value%03i", (ii + 1));
			config_getInteger(&config, "texture.apply.rgba.change", buffer,
					0, &glValue);

			if(texture_applyRgbaChange(result, glProperty, glValue) < 0) {
				log_logf(log, LOG_LEVEL_ERROR, "{TEXTURE} failed to apply "
						"color change #%i - '%s' for texture '%s'",
						ii, sProperty, assetFilename);
			}
		}
	}

	// set RGBA channel

	config_getBoolean(&config, "texture", "hasSetRgbaChannel", afalse, &bValue);
	if(bValue) {
		config_getInteger(&config, "texture.set.rgba.channel", "length", 0,
				&iValue);

		for(ii = 0; ii < iValue; ii++) {
			snprintf(buffer, (sizeof(buffer) - 1), "property%03i", (ii + 1));
			config_getString(&config, "texture.set.rgba.channel", buffer,
					"unknown", &sProperty);

			if(!strcmp(sProperty, "red")) {
				glProperty = TEXTURE_COLOR_RED;
			}
			else if(!strcmp(sProperty, "green")) {
				glProperty = TEXTURE_COLOR_GREEN;
			}
			else if(!strcmp(sProperty, "blue")) {
				glProperty = TEXTURE_COLOR_BLUE;
			}
			else if(!strcmp(sProperty, "alpha")) {
				glProperty = TEXTURE_COLOR_ALPHA;
			}
			else {
				log_logf(log, LOG_LEVEL_ERROR, "{TEXTURE} unknown RGBA set "
						"color channel property '%s' for texture '%s'",
						sProperty, assetFilename);
				continue;
			}

			snprintf(buffer, (sizeof(buffer) - 1), "value%03i", (ii + 1));
			config_getInteger(&config, "texture.set.rgba.channel", buffer,
					0, &glValue);

			if(texture_applyRgbaChange(result, glProperty, glValue) < 0) {
				log_logf(log, LOG_LEVEL_ERROR, "{TEXTURE} failed to set "
						"color channel #%i - '%s' for texture '%s'",
						ii, sProperty, assetFilename);
			}
		}
	}

	log_logf(log, LOG_LEVEL_DEBUG,
			"{TEXTURE} loaded texture '%s'", assetFilename);

	// cleanup

	config_free(&config);

	return result;
}

int texture_free(Texture *texture)
{
	if(texture == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(texture->id > -1) {
		cgi_glDeleteTextures(1, (GLuint *)&texture->id);
	}

	if(texture->image != NULL) {
		free(texture->image);
	}

	memset((void *)texture, 0, (sizeof(Texture)));

	return 0;
}

int texture_freePtr(Texture *texture)
{
	int rc = 0;

	if(texture == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if((rc = texture_free(texture)) < 0) {
		return rc;
	}

	free(texture);

	return 0;
}

int texture_saveAsAsgardTextureFile(Texture *texture, char *filename)
{
	if(texture == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	/*
	 * TODO: this function
	 */

	return -1;
}

int texture_setHardwareResident(Texture *texture, aboolean hardwareResident)
{
	if(texture == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	texture->hardwareResident = hardwareResident;

	if(texture->id > -1) {
		cgi_glDeleteTextures(1, (GLuint *)&texture->id);
	}

	if(buildOpenGlTexture(texture, texture->qualityLevel, texture->renderMode,
				texture->hardwareResident, texture->isOffscreenRender) < 0) {
		return -1;
	}

	return 0;
}

int texture_setQualityLevel(Texture *texture, int qualityLevel)
{
	if((texture == NULL) ||
			((qualityLevel != TEXTURE_QUALITY_LOWEST) &&
			 (qualityLevel != TEXTURE_QUALITY_LOW) &&
			 (qualityLevel != TEXTURE_QUALITY_MEDIUM) &&
			 (qualityLevel != TEXTURE_QUALITY_MEDIUM_HIGH) &&
			 (qualityLevel != TEXTURE_QUALITY_HIGH) &&
			 (qualityLevel != TEXTURE_QUALITY_HIGHEST))) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	texture->qualityLevel = qualityLevel;

	if(texture->id > -1) {
		cgi_glDeleteTextures(1, (GLuint *)&texture->id);
	}

	if(buildOpenGlTexture(texture, texture->qualityLevel, texture->renderMode,
				texture->hardwareResident, texture->isOffscreenRender) < 0) {
		return -1;
	}

	return 0;
}

int texture_setRenderMode(Texture *texture, int renderMode)
{
	if((texture == NULL) ||
			((renderMode != TEXTURE_RENDER_CLAMP) &&
			 (renderMode != TEXTURE_RENDER_CLAMP_TO_BORDER) &&
			 (renderMode != TEXTURE_RENDER_CLAMP_TO_EDGE) &&
			 (renderMode != TEXTURE_RENDER_MIRRORED_REPEAT) &&
			 (renderMode != TEXTURE_RENDER_REPEAT))) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	texture->renderMode = renderMode;

	if(texture->id > -1) {
		cgi_glDeleteTextures(1, (GLuint *)&texture->id);
	}

	if(buildOpenGlTexture(texture, texture->renderMode, texture->renderMode,
				texture->hardwareResident, texture->isOffscreenRender) < 0) {
		return -1;
	}

	return 0;
}

int texture_setGlProperty(Texture *texture, int property, int value)
{
	if(texture == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	cgi_glBindTexture(GL_TEXTURE_2D, (GLuint)texture->id);

	cgi_glTexParameteri(GL_TEXTURE_2D, (GLenum)property, (GLint)value);

	cgi_glBindTexture(GL_TEXTURE_2D, 0);

	if(checkForOpenGlError() < 0) {
		return -1;
	}

	return 0;
}

int texture_setBlend(Texture *texture, int blendSource, int blendDest)
{
	if((texture == NULL) ||
			((blendSource != TEXTURE_BLEND_SOURCE_SRC_COLOR) &&
			 (blendSource != TEXTURE_BLEND_SOURCE_ONE_MINUS_SRC_COLOR) &&
			 (blendSource != TEXTURE_BLEND_SOURCE_DST_COLOR) &&
			 (blendSource != TEXTURE_BLEND_SOURCE_ONE_MINUS_DST_COLOR) &&
			 (blendSource != TEXTURE_BLEND_SOURCE_SRC_ALPHA) &&
			 (blendSource != TEXTURE_BLEND_SOURCE_ONE_MINUS_SRC_ALPHA) &&
			 (blendSource != TEXTURE_BLEND_SOURCE_DST_ALPHA) &&
			 (blendSource != TEXTURE_BLEND_SOURCE_ONE_MINUS_DST_ALPHA)) ||
			((blendDest != TEXTURE_BLEND_DEST_SRC_COLOR) &&
			 (blendDest != TEXTURE_BLEND_DEST_ONE_MINUS_SRC_COLOR) &&
			 (blendDest != TEXTURE_BLEND_DEST_DST_COLOR) &&
			 (blendDest != TEXTURE_BLEND_DEST_ONE_MINUS_DST_COLOR) &&
			 (blendDest != TEXTURE_BLEND_DEST_SRC_ALPHA) &&
			 (blendDest != TEXTURE_BLEND_DEST_ONE_MINUS_SRC_ALPHA) &&
			 (blendDest != TEXTURE_BLEND_DEST_DST_ALPHA) &&
			 (blendDest != TEXTURE_BLEND_DEST_ONE_MINUS_DST_ALPHA))) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	texture->isBlended = atrue;
	texture->blendSourceType = getGlBlendSource(blendSource);
	texture->blendDestType = getGlBlendDest(blendDest);

	return 0;
}

int texture_setColorMask(Texture *texture, Color *colorMask, int maskMode)
{
	if((texture == NULL) || (colorMask == NULL) ||
			((maskMode != TEXTURE_COLORMASK_MODE_ADD) &&
			 (maskMode != TEXTURE_COLORMASK_MODE_MODULATE) &&
			 (maskMode != TEXTURE_COLORMASK_MODE_DECAL) &&
			 (maskMode != TEXTURE_COLORMASK_MODE_BLEND) &&
			 (maskMode != TEXTURE_COLORMASK_MODE_REPLACE) &&
			 (maskMode != TEXTURE_COLORMASK_MODE_COMBINE))
	  ) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	texture->hasColorMask = atrue;

	switch(maskMode) {
		case TEXTURE_COLORMASK_MODE_ADD:
			texture->colorMaskMode = GL_ADD;
			break;
		case TEXTURE_COLORMASK_MODE_MODULATE:
			texture->colorMaskMode = GL_MODULATE;
			break;
		case TEXTURE_COLORMASK_MODE_DECAL:
			texture->colorMaskMode = GL_DECAL;
			break;
		case TEXTURE_COLORMASK_MODE_BLEND:
			texture->colorMaskMode = GL_BLEND;
			break;
		case TEXTURE_COLORMASK_MODE_REPLACE:
			texture->colorMaskMode = GL_REPLACE;
			break;
		case TEXTURE_COLORMASK_MODE_COMBINE:
			texture->colorMaskMode = GL_COMBINE;
			break;
	}

	texture->colorMask.r = colorMask->r;
	texture->colorMask.g = colorMask->g;
	texture->colorMask.b = colorMask->b;
	texture->colorMask.a = colorMask->a;

	cgi_glBindTexture(GL_TEXTURE_2D, (GLuint)texture->id);

	cgi_glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, texture->colorMaskMode);

	cgi_glBindTexture(GL_TEXTURE_2D, 0);

	if(checkForOpenGlError() < 0) {
		return -1;
	}

	return 0;
}

int texture_getDimensions(Texture *texture, int *width, int *height)
{
	if((texture == NULL) || (width == NULL) || (height == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	*width = texture->width;
	*height = texture->height;

	return 0;
}

int texture_applyRgbaChange(Texture *texture, int colorType,
		unsigned char value)
{
	int ii = 0;
	int startPosition = 0;

	if((texture == NULL) ||
			((colorType != TEXTURE_COLOR_RED) &&
			 (colorType != TEXTURE_COLOR_GREEN) &&
			 (colorType != TEXTURE_COLOR_BLUE) &&
			 (colorType != TEXTURE_COLOR_ALPHA))) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	switch(colorType) {
		case TEXTURE_COLOR_RED:
			startPosition = 0;
			break;
		case TEXTURE_COLOR_GREEN:
			startPosition = 1;
			break;
		case TEXTURE_COLOR_BLUE:
			startPosition = 2;
			break;
		case TEXTURE_COLOR_ALPHA:
			startPosition = 3;
			break;
	}

	for(ii = startPosition; ii < texture->imageLength; ii += 4) {
		texture->image[ii] += value;
	}

	if(texture->id > -1) {
		cgi_glDeleteTextures(1, (GLuint *)&texture->id);
	}

	if(buildOpenGlTexture(texture, texture->qualityLevel, texture->renderMode,
				texture->hardwareResident, texture->isOffscreenRender) < 0) {
		return -1;
	}

	return 0;
}

int texture_setRgbaChannel(Texture *texture, int colorType,
		unsigned char value)
{
	int ii = 0;
	int startPosition = 0;

	if((texture == NULL) ||
			((colorType != TEXTURE_COLOR_RED) &&
			 (colorType != TEXTURE_COLOR_GREEN) &&
			 (colorType != TEXTURE_COLOR_BLUE) &&
			 (colorType != TEXTURE_COLOR_ALPHA))) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	switch(colorType) {
		case TEXTURE_COLOR_RED:
			startPosition = 0;
			break;
		case TEXTURE_COLOR_GREEN:
			startPosition = 1;
			break;
		case TEXTURE_COLOR_BLUE:
			startPosition = 2;
			break;
		case TEXTURE_COLOR_ALPHA:
			startPosition = 3;
			break;
	}

	for(ii = startPosition; ii < texture->imageLength; ii += 4) {
		texture->image[ii] = value;
	}

	if(texture->id > -1) {
		cgi_glDeleteTextures(1, (GLuint *)&texture->id);
	}

	if(buildOpenGlTexture(texture, texture->qualityLevel, texture->renderMode,
				texture->hardwareResident, texture->isOffscreenRender) < 0) {
		return -1;
	}

	return 0;
}

int texture_apply(Texture *texture)
{
	if(texture == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

#if !defined(__ANDROID__)
	if(cgi_glIsEnabled(GL_TEXTURE_2D)) {
		texture->renderIsTexture2d = atrue;
	}
	else {
		texture->renderIsTexture2d = afalse;
		cgi_glEnable(GL_TEXTURE_2D);
	}
#else // __ANDROID__
	texture->renderIsTexture2d = atrue;
	cgi_glEnable(GL_TEXTURE_2D);
#endif // !__ANDROID__

	if(texture->isBlended) {
#if !defined(__ANDROID__)
		if(cgi_glIsEnabled(GL_BLEND)) {
			texture->renderIsBlendEnabled = atrue;
		}
		else {
			texture->renderIsBlendEnabled = afalse;
			cgi_glEnable(GL_BLEND);
		}
#else // __ANDROID__
		texture->renderIsBlendEnabled = atrue;
		cgi_glEnable(GL_BLEND);
#endif // !__ANDROID__

		cgi_glBlendFunc(texture->blendSourceType, texture->blendDestType);
	}

#if !defined(__ANDROID__)
	if(texture->hasColorMask) {
		cgi_glColor4d(texture->colorMask.r, texture->colorMask.g,
				texture->colorMask.b, texture->colorMask.a);
	}

	cgi_glBindTexture(GL_TEXTURE_2D, (GLuint)texture->id);

	cgi_glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, texture->colorMaskMode);
#else // __ANDROID__
	CGI_COLOR_WHITE;

	cgi_glBindTexture(GL_TEXTURE_2D, (GLuint)texture->id);
#endif // !__ANDROID__

	return 0;
}

int texture_unApply(Texture *texture)
{
	if(texture == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	cgi_glBindTexture(GL_TEXTURE_2D, 0);

	if((texture->isBlended) && (!texture->renderIsBlendEnabled)) {
		cgi_glDisable(GL_BLEND);
	}

	if(!texture->renderIsTexture2d) {
		cgi_glDisable(GL_TEXTURE_2D);
	}

	return 0;
}

int texture_toggleRender2dToTexture(Texture *texture)
{
	if(texture == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	// flat perspective

	cgi_glMatrixMode(GL_PROJECTION);
	cgi_glLoadIdentity();

	cgi_glOrtho(0,
			texture->width,
			texture->height,
			0,
			-1,
			1
			);

	// rendering mode

	cgi_glMatrixMode(GL_MODELVIEW);
	cgi_glLoadIdentity();

	// setup vieport

	cgi_glViewport(0, 0, texture->width, texture->height);

	return 0;
}

int texture_toggleRender3dToTexture(Texture *texture)
{
	if(texture == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	// perspective frustum

	cgi_glMatrixMode(GL_PROJECTION);
	cgi_glLoadIdentity();

	cgi_gluPerspective(
			CANVAS_Y_VIEW_ANGLE,						// y view angle
			(GLfloat)((double)texture->width /			// aspact ratio
				(double)texture->height),
			CANVAS_NEAR_Z_PLANE,						// near z plane
			CANVAS_FAR_Z_PLANE							// far z plane
			);

	// rendering mode

	cgi_glMatrixMode(GL_MODELVIEW);
	cgi_glLoadIdentity();

	return 0;
}

int texture_endRenderToTexture(Texture *texture, Canvas *canvas)
{
	int ii = 0;

	if((texture == NULL) || (canvas == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	// copy the framebuffer pixels into the allocated image

	glReadPixels(0, 0, texture->width, texture->height, GL_RGBA,
			GL_UNSIGNED_BYTE, texture->image);

/*	cgi_glBindTexture(GL_TEXTURE_2D, (GLuint)texture->id);

	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, texture->width,
			texture->height, 0);

	cgi_glBindTexture(GL_TEXTURE_2D, 0);*/

	// cleanup the viewport

	cgi_glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	cgi_glViewport(0, 0, canvas->screenWidth, canvas->screenHeight);

	cgi_glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// delete the texture

	if(texture->id > -1) {
		cgi_glDeleteTextures(1, (GLuint *)&texture->id);
	}

	// set all black background pixels to a clear alpha channel

	for(ii = 0; ii < texture->imageLength; ii += 4) {
		if((texture->image[(ii + 0)] <= 0) &&
				(texture->image[(ii + 1)] <= 0) &&
				(texture->image[(ii + 2)] <= 0)) {
			texture->image[(ii + 0)] = (unsigned char)0;
			texture->image[(ii + 1)] = (unsigned char)0;
			texture->image[(ii + 2)] = (unsigned char)0;
			texture->image[(ii + 3)] = (unsigned char)0;
		}
	}

	// rebuild the texture

	if(buildOpenGlTexture(texture, texture->qualityLevel, texture->renderMode,
				texture->hardwareResident, texture->isOffscreenRender) < 0) {
		return -1;
	}

	return 0;
}

int texture_draw2d(Texture *texture, int x, int y, int width, int height)
{
	if(texture == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(texture_apply(texture) < 0) {
		return -1;
	}

	cgi_glBegin(GL_QUADS);

	cgi_glTexCoord2d(0.0, 0.0);
	cgi_glVertex2d((GLdouble)x, (GLdouble)(y + height));

	cgi_glTexCoord2d(1.0, 0.0);
	cgi_glVertex2d((GLdouble)(x + width), (GLdouble)(y + height));

	cgi_glTexCoord2d(1.0, 1.0);
	cgi_glVertex2d((GLdouble)(x + width), (GLdouble)y);

	cgi_glTexCoord2d(0.0, 1.0);
	cgi_glVertex2d((GLdouble)x, (GLdouble)y);

	cgi_glEnd();

	if(texture_unApply(texture) < 0) {
		return -1;
	}

	return 0;
}

