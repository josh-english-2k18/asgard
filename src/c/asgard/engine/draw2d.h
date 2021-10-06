/*
 * draw2d.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard Game Engine 2D rendering system, header file.
 *
 * Written by Josh English.
 */

#if !defined(_ASGARD_ENGINE_DRAW2D_H)

#define _ASGARD_ENGINE_DRAW2D_H

#if !defined(_ASGARD_H) && !defined(_ASGARD_ENGINE_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _ASGARD_H || _ASGARD_ENGINE_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

// define texture public constants

typedef enum _Draw2dObjectType {
	DRAW2D_TYPE_COLOR = 1,
	DRAW2D_TYPE_FONT,
	DRAW2D_TYPE_COLOR_FONT,
	DRAW2D_TYPE_TEXTURE,
	DRAW2D_TYPE_UNKNOWN = -1
} Draw2dObjectType;


// define draw2d public data types

typedef struct _Draw2dColor {
	Color *color;
} Draw2dColor;

typedef struct _Draw2dFont {
	int x;
	int y;
	char *string;
	Font *font;
} Draw2dFont;

typedef struct _Draw2dColorFont {
	int x;
	int y;
	char *string;
	Color *color;
	Font *font;
} Draw2dColorFont;

typedef struct _Draw2dTexture {
	int x;
	int y;
	int width;
	int height;
	Texture *texture;
} Draw2dTexture;

typedef struct _Draw2dObject {
	int type;
	void *payload;
	Canvas *canvas;
} Draw2dObject;


// declare draw2d public functions

// rendering functions

void draw2d_beginRectangleRotation(int x, int y, int width, int height,
		double rotationDegrees);

void draw2d_endRectangleRotation();

void draw2d_color(Canvas *canvas, Color *color);

void draw2d_line(Canvas *canvas, Color *color, int x1, int y1, int x2, int y2);

void draw2d_rectangle(Canvas *canvas, Color *color, int x, int y, int width,
		int height);

void draw2d_rotatedRectangle(Canvas *canvas, Color *color, int x, int y,
		int width, int height, double rotationDegrees);

void draw2d_fillRectangle(Canvas *canvas, Color *color, int x, int y, int width,
		int height);

void draw2d_fillRotatedRectangle(Canvas *canvas, Color *color, int x, int y,
		int width, int height, double rotationDegrees);

void draw2d_circle(Canvas *canvas, Color *color, int x, int y, double radius);

void draw2d_fillCircle(Canvas *canvas, Color *color, int x, int y,
		double radius);

void draw2d_font(Canvas *canvas, int x, int y, char *string, Font *font);

void draw2d_colorFont(Canvas *canvas, int x, int y, char *string, Color *color,
		Font *font);

void draw2d_texture(Canvas *canvas, int x, int y, int width, int height,
		Texture *texture);

void draw2d_rotatedTexture(Canvas *canvas, int x, int y, int width, int height,
		double rotationDegrees, Texture *texture);

void draw2d_draw2dObject(Draw2dObject *object);

// object functions

Draw2dObject *draw2d_newObjectColor(Canvas *canvas, Color *color);

Draw2dObject *draw2d_newObjectFont(Canvas *canvas, int x, int y, char *string,
		Font *font);

Draw2dObject *draw2d_newObjectColorFont(Canvas *canvas, int x, int y,
		char *string, Color *color, Font *font);

Draw2dObject *draw2d_newObjectTexture(Canvas *canvas, int x, int y, int width,
		int height, Texture *texture);

int draw2d_freeObject(Draw2dObject *object);


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _ASGARD_ENGINE_DRAW2D_H

