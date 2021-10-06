/*
 * font.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard Game Engine 2D font system, header file.
 *
 * Written by Josh English.
 */

#if !defined(_ASGARD_ENGINE_FONT_H)

#define _ASGARD_ENGINE_FONT_H

#if !defined(_ASGARD_H) && !defined(_ASGARD_ENGINE_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _ASGARD_H || _ASGARD_ENGINE_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

// define font public constants

#define FONT_CHARSET_ANSI									1
#define FONT_CHARSET_BALTIC									2
#define FONT_CHARSET_CHINESE								3
#define FONT_CHARSET_EAST_EUROPE							4
#define FONT_CHARSET_GREAT_BRITAIN							5
#define FONT_CHARSET_GREEK									6
#define FONT_CHARSET_HANGUL									7
#define FONT_CHARSET_RUSSIAN								8
#define FONT_CHARSET_TURKISH								9
#define FONT_CHARSET_VIETNAMESE								10

#define FONT_CHARSET_LENGTH									256

#define FONT_OS_DEFAULT										1
#define FONT_LUCIDA											2
#define FONT_ARIAL											3
#define FONT_NONE											-1

// define font public data types

typedef struct __Font {
	int id;
	int type;
	int height;
	char *name;
	void *fontHandle;
	void *screenHandle;
} _Font;


// define font public macros (prevents conflict with X11 Font type)

#define Font _Font


// declare font public functions

int font_init(Font *font, void *screenHandle, int fontType, int height,
		aboolean isBold, aboolean isItalic, aboolean isUnderline, int charset);

Font *font_load(Log *log, void *screenHandle, char *assetFilename,
		aboolean *hasColor, Color *color);

int font_free(Font *font);

int font_draw2d(Font *font, Canvas *canvas, int xPosition, int yPosition,
		const char *format, ...);


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _ASGARD_ENGINE_FONT_H

