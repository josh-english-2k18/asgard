/*
 * font.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard Game Engine 2D font system.
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
#include "asgard/engine/font.h"
#define _ASGARD_ENGINE_PLUGIN_COMPONENT
#include "asgard/engine/plugin/font_android.h"
#include "asgard/engine/plugin/font_linux.h"
#include "asgard/engine/plugin/font_windows.h"

// define font private constants

#define FONT_WIN32											1
#define FONT_LINUX											2
#define FONT_ANDROID										3

typedef struct _FontType {
	int fontType;
	int systemType;
	char *name;
} FontType;

const FontType FONTS[] = {
	{	FONT_OS_DEFAULT,	FONT_WIN32,		"Times New Roman",				},
	{	FONT_OS_DEFAULT,	FONT_LINUX,		"Courier",						},
	{	FONT_OS_DEFAULT,	FONT_ANDROID,	"Courier",						},
	{	FONT_LUCIDA,		FONT_WIN32,		"Lucida Console",				},
	{	FONT_LUCIDA,		FONT_LINUX,		"LucidaTypewriter",				},
	{	FONT_LUCIDA,		FONT_ANDROID,	"LucidaTypewriter",				},
	{	FONT_ARIAL,			FONT_WIN32,		"Arial",						},
	{	FONT_ARIAL,			FONT_LINUX,		"Utopia",						},
	{	FONT_ARIAL,			FONT_ANDROID,	"Utopia",						},
	{	FONT_NONE,			FONT_NONE,		NULL							}
};

// define font private functions

static int getSystemType()
{
	int result = 0;

#if defined(WIN32)
	result = FONT_WIN32;
#elif defined(__linux__) && !defined(__ANDROID__)
	result = FONT_LINUX;
#elif defined(__ANDROID__)
	result = FONT_ANDROID;
#else // - no plugins available -
	result = -1;
#endif // - plugins -

	return result;
}

static char *getFontName(int fontType)
{
	int ii = 0;
	int systemType = 0;
	char *result = NULL;

	systemType = getSystemType();

	for(ii = 0; FONTS[ii].fontType != FONT_NONE; ii++) {
		if((FONTS[ii].fontType == fontType) &&
				(FONTS[ii].systemType == systemType)) {
			result = strdup((char *)FONTS[ii].name);
			break;
		}
	}

	return result;
}

// define font public functions

int font_init(Font *font, void *screenHandle, int fontType, int height,
		aboolean isBold, aboolean isItalic, aboolean isUnderline, int charset)
{
	int rc = 0;

	if((font == NULL) || (screenHandle == NULL) ||
			((fontType != FONT_OS_DEFAULT) &&
			 (fontType != FONT_LUCIDA) &&
			 (fontType != FONT_ARIAL)) ||
			(height < 1) ||
			((charset != FONT_CHARSET_ANSI) &&
			 (charset != FONT_CHARSET_BALTIC) &&
			 (charset != FONT_CHARSET_CHINESE) &&
			 (charset != FONT_CHARSET_EAST_EUROPE) &&
			 (charset != FONT_CHARSET_GREAT_BRITAIN) &&
			 (charset != FONT_CHARSET_GREEK) &&
			 (charset != FONT_CHARSET_HANGUL) &&
			 (charset != FONT_CHARSET_RUSSIAN) &&
			 (charset != FONT_CHARSET_TURKISH) &&
			 (charset != FONT_CHARSET_VIETNAMESE))
	  ) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	memset((void *)font, 0, sizeof(Font));

	font->id = 0;
	font->type = fontType;
	font->height = height;
	font->name = NULL;
	font->fontHandle = NULL;
	font->screenHandle = screenHandle;

	if((font->name = getFontName(font->type)) == NULL) {
		return -1;
	}

#if defined(WIN32)
	rc = font_loadWindows(font->screenHandle, font->name, font->height, isBold,
			isItalic, isUnderline, charset, (void *)&font->fontHandle,
			&font->id);
#elif defined(__linux__) && !defined(__ANDROID__)
	rc = font_loadLinux(font->screenHandle, font->name, font->height, isBold,
			isItalic, isUnderline, charset, (void *)&font->fontHandle,
			&font->id);
#elif defined(__ANDROID__)
	rc = font_loadAndroid(font->screenHandle, font->name, font->height, isBold,
			isItalic, isUnderline, charset, (void *)&font->fontHandle,
			&font->id);
#else // - no plugins available
	rc = -1;
#endif // - plugins -

	return rc;
}

Font *font_load(Log *log, void *screenHandle, char *assetFilename,
		aboolean *hasColor, Color *color)
{
	aboolean isBold = afalse;
	aboolean isItalic = afalse;
	aboolean isUnderline = afalse;
	aboolean bValue = afalse;
	int rc = 0;
	int charset = 0;
	char *sValue = NULL;

	Font *result = NULL;
	Config config;

	if((log == NULL) || (screenHandle == NULL) || (assetFilename == NULL) ||
			(hasColor == NULL) || (color == NULL)) {
		return NULL;
	}

	if(config_init(&config, assetFilename) < 0) {
		log_logf(log, LOG_LEVEL_ERROR,
				"{FONT} failed to load configuration file '%s'",
				assetFilename);
		return NULL;
	}

	// validate that the configuration file is for a 2D font asset

	config_getString(&config, "asset", "class", "unknown", &sValue);
	if(strcmp(sValue, "2d")) {
		log_logf(log, LOG_LEVEL_ERROR,
				"{FONT} invalid font configuration - asset.class is '%s'",
				sValue);
		return NULL;
	}

	config_getString(&config, "asset", "type", "unknown", &sValue);
	if(strcmp(sValue, "font")) {
		log_logf(log, LOG_LEVEL_ERROR,
				"{FONT} invalid font configuration - asset.type is '%s'",
				sValue);
		return NULL;
	}

	// load the font configuration

	result = (Font *)malloc(sizeof(Font));

	config_getString(&config, "font", "fontType", "unknown", &sValue);
	if(!strcmp(sValue, "default")) {
		result->type = FONT_OS_DEFAULT;
	}
	else if(!strcmp(sValue, "lucida")) {
		result->type = FONT_LUCIDA;
	}
	else if(!strcmp(sValue, "arial")) {
		result->type = FONT_ARIAL;
	}
	else {
		log_logf(log, LOG_LEVEL_ERROR,
				"{FONT} unknown font type '%s' for asset '%s'",
				sValue, assetFilename);
		free(result);
		return NULL;
	}

	config_getInteger(&config, "font", "height", 12, &result->height);
	config_getBoolean(&config, "font", "isBold", afalse, &isBold);
	config_getBoolean(&config, "font", "isItalic", afalse, &isItalic);
	config_getBoolean(&config, "font", "isUnderline", afalse, &isUnderline);

	config_getString(&config, "font", "charset", "unknown", &sValue);
	if(!strcmp(sValue, "ansi")) {
		charset = FONT_CHARSET_ANSI;
	}
	else if(!strcmp(sValue, "baltic")) {
		charset = FONT_CHARSET_BALTIC;
	}
	else if(!strcmp(sValue, "chinese")) {
		charset = FONT_CHARSET_CHINESE;
	}
	else if(!strcmp(sValue, "east-europe")) {
		charset = FONT_CHARSET_EAST_EUROPE;
	}
	else if(!strcmp(sValue, "great-britian")) {
		charset = FONT_CHARSET_GREAT_BRITAIN;
	}
	else if(!strcmp(sValue, "greek")) {
		charset = FONT_CHARSET_GREEK;
	}
	else if(!strcmp(sValue, "hangul")) {
		charset = FONT_CHARSET_HANGUL;
	}
	else if(!strcmp(sValue, "russian")) {
		charset = FONT_CHARSET_RUSSIAN;
	}
	else if(!strcmp(sValue, "turkish")) {
		charset = FONT_CHARSET_TURKISH;
	}
	else if(!strcmp(sValue, "vietnamese")) {
		charset = FONT_CHARSET_VIETNAMESE;
	}
	else {
		log_logf(log, LOG_LEVEL_ERROR,
				"{FONT} unknown font charset '%s' for asset '%s'",
				sValue, assetFilename);
		free(result);
		return NULL;
	}

	// construct the font

	if((rc = font_init(result, screenHandle, result->type, result->height,
					isBold, isItalic, isUnderline, charset)) < 0) {
		log_logf(log, LOG_LEVEL_ERROR,
				"{FONT} failed to build font from asset '%s'",
				assetFilename);
		free(result);
		return NULL;
	}

	// obtain the font color

	config_getBoolean(&config, "font", "hasColor", afalse, &bValue);
	if(bValue) {
		config_getBoolean(&config, "font.color", "loadByName", afalse, &bValue);
		if(bValue) {
			config_getString(&config, "font.color", "name", "unknown", &sValue);
			colors_getColor(sValue, color);
		}
		else {
			config_getDouble(&config, "font.color", "red", 0.0, &color->r);
			config_getDouble(&config, "font.color", "green", 0.0, &color->g);
			config_getDouble(&config, "font.color", "blue", 0.0, &color->b);
			config_getDouble(&config, "font.color", "alpha", 0.0, &color->a);
		}
		*hasColor = atrue;
	}
	else {
		*hasColor = afalse;
	}

	log_logf(log, LOG_LEVEL_DEBUG, "{FONT} loaded font '%s'", assetFilename);

	// cleanup

	config_free(&config);

	return result;
}

int font_free(Font *font)
{
	int rc = 0;

	if(font == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	cgi_glDeleteLists(font->id, FONT_CHARSET_LENGTH);

#if defined(WIN32)
	rc = font_freeWindows(font->screenHandle, font->fontHandle);
#elif defined(__linux__) && !defined(__ANDROID__)
	rc = font_freeLinux(font->screenHandle, font->fontHandle);
#elif defined(__ANDROID__)
	rc = font_freeAndroid(font->screenHandle, font->fontHandle);
#else // - no plugins available
	rc = -1;
#endif // - plugins -

	if(font->name != NULL) {
		free(font->name);
	}

	memset(font, 0, (sizeof(Font)));

	return rc;
}

int font_freePtr(Font *font)
{
	int rc = 0;

	if(font == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if((rc = font_free(font)) < 0) {
		return rc;
	}

	free(font);

	return rc;
}

int font_draw2d(Font *font, Canvas *canvas, int xPosition, int yPosition,
		const char *format, ...)
{
	int bufferLength = 0;
	char *buffer = NULL;

	va_list vaArgs;

	if((font == NULL) || (canvas == NULL) || (xPosition < 0) ||
			(yPosition < 0)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	// parse input argument(s)

	va_start(vaArgs, format);
	buffer = vaprintf(format, vaArgs, &bufferLength);
	va_end(vaArgs);

	// calculate y position

	yPosition = (canvas->screenHeight - font->height - yPosition);

	// set viewport and raster position for drawing

	cgi_glViewport((xPosition - 1), (yPosition - 1), 0, 0);
	cgi_glRasterPos4d(0.0, 0.0, 0.0, 1.0);

	// render the font from the GL list

	cgi_glPushAttrib(GL_LIST_BIT);
	cgi_glListBase(font->id);
	cgi_glCallLists(bufferLength, GL_UNSIGNED_BYTE, buffer);
	cgi_glPopAttrib();

	// reset the viewport back to default

	cgi_glViewport(0, 0, canvas->screenWidth, canvas->screenHeight);

	// cleanup

	free(buffer);

	return 0;
}

