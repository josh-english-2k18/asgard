/*
 * draw2d.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard Game Engine 2D rendering system.
 *
 * Written by Josh English.
 */


// preprocessor directives

#include "core/core.h"
#include "asgard/asgard.h"
#define _ASGARD_ENGINE_COMPONENT
#include "asgard/engine/draw2d.h"


// declare draw2d private functions

static void beginRectangleRotation(int x, int y, int width, int height,
		double rotationDegrees);

static void endRectangleRotation();


// define draw2d private functions

static void beginRectangleRotation(int x, int y, int width, int height,
		double rotationDegrees)
{
	cgi_glPushMatrix();

	cgi_glTranslated(
			(GLdouble)((double)x + ((double)width / 2.0)),
			(GLdouble)((double)y + ((double)height / 2.0)),
			0.0);

	cgi_glRotated(rotationDegrees,
			0.0,
			0.0,
			1.0);

	cgi_glTranslated(
			-(GLdouble)((double)width / 2.0),
			-(GLdouble)((double)height / 2.0),
			0.0);
}

static void endRectangleRotation()
{
	cgi_glPopMatrix();
}


// define draw2d public functions

// rendering functions

void draw2d_beginRectangleRotation(int x, int y, int width, int height,
		double rotationDegrees)
{
	beginRectangleRotation(x, y, width, height, rotationDegrees);
}

void draw2d_endRectangleRotation()
{
	endRectangleRotation();
}

void draw2d_color(Canvas *canvas, Color *color)
{
	if((canvas == NULL) || (color == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	cgi_glColor4d(color->r, color->g, color->b, color->a);

	canvas_reportOpenGLErrors(canvas, __FUNCTION__);
}

void draw2d_line(Canvas *canvas, Color *color, int x1, int y1, int x2, int y2)
{
	if((canvas == NULL) || (color == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	cgi_glColor4d(color->r, color->g, color->b, color->a);

	cgi_glBegin(GL_LINES);

	cgi_glVertex2d((GLdouble)x1, (GLdouble)y1);
	cgi_glVertex2d((GLdouble)x2, (GLdouble)y2);

	cgi_glEnd();

	canvas_reportOpenGLErrors(canvas, __FUNCTION__);
}

void draw2d_rectangle(Canvas *canvas, Color *color, int x, int y, int width,
		int height)
{
	if((canvas == NULL) || (color == NULL) || (width < 0) || (height < 0)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	cgi_glColor4d(color->r, color->g, color->b, color->a);

	cgi_glBegin(GL_LINE_LOOP);

	cgi_glVertex2d((GLdouble)x, (GLdouble)(y + height));
	cgi_glVertex2d((GLdouble)(x + width), (GLdouble)(y + height));
	cgi_glVertex2d((GLdouble)(x + width), (GLdouble)y);
	cgi_glVertex2d((GLdouble)x, (GLdouble)y);

	cgi_glEnd();

	canvas_reportOpenGLErrors(canvas, __FUNCTION__);
}

void draw2d_rotatedRectangle(Canvas *canvas, Color *color, int x, int y,
		int width, int height, double rotationDegrees)
{
	if((canvas == NULL) || (color == NULL) || (width < 0) || (height < 0)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	cgi_glColor4d(color->r, color->g, color->b, color->a);

	beginRectangleRotation(x, y, width, height, rotationDegrees);

	cgi_glBegin(GL_LINE_LOOP);

	cgi_glVertex2d((GLdouble)0.0, (GLdouble)height);
	cgi_glVertex2d((GLdouble)width, (GLdouble)height);
	cgi_glVertex2d((GLdouble)width, (GLdouble)0.0);
	cgi_glVertex2d((GLdouble)0.0, (GLdouble)0.0);

	cgi_glEnd();

	endRectangleRotation();

	canvas_reportOpenGLErrors(canvas, __FUNCTION__);
}

void draw2d_fillRectangle(Canvas *canvas, Color *color, int x, int y, int width,
		int height)
{
	if((canvas == NULL) || (color == NULL) || (width < 0) || (height < 0)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	cgi_glColor4d(color->r, color->g, color->b, color->a);

	cgi_glBegin(GL_QUADS);

	cgi_glVertex2d((GLdouble)x, (GLdouble)(y + height));
	cgi_glVertex2d((GLdouble)(x + width), (GLdouble)(y + height));
	cgi_glVertex2d((GLdouble)(x + width), (GLdouble)y);
	cgi_glVertex2d((GLdouble)x, (GLdouble)y);

	cgi_glEnd();

	canvas_reportOpenGLErrors(canvas, __FUNCTION__);
}

void draw2d_fillRotatedRectangle(Canvas *canvas, Color *color, int x, int y,
		int width, int height, double rotationDegrees)
{
	if((canvas == NULL) || (color == NULL) || (width < 0) || (height < 0)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	cgi_glColor4d(color->r, color->g, color->b, color->a);

	beginRectangleRotation(x, y, width, height, rotationDegrees);

	cgi_glBegin(GL_QUADS);

	cgi_glVertex2d((GLdouble)0.0, (GLdouble)height);
	cgi_glVertex2d((GLdouble)width, (GLdouble)height);
	cgi_glVertex2d((GLdouble)width, (GLdouble)0.0);
	cgi_glVertex2d((GLdouble)0.0, (GLdouble)0.0);

	cgi_glEnd();

	endRectangleRotation();

	canvas_reportOpenGLErrors(canvas, __FUNCTION__);
}

void draw2d_circle(Canvas *canvas, Color *color, int x, int y, double radius)
{
	int ii = 0;
	double angle = 0.0;

	if((canvas == NULL) || (color == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	x += (int)radius;
	y += (int)radius;

	cgi_glColor4d(color->r, color->g, color->b, color->a);

	cgi_glBegin(GL_LINE_LOOP);

	for(ii = 0; ii < 360; ii++) {
		angle = (double)ii;

		cgi_glVertex2d((GLdouble)((double)x + sin(angle) * radius),
				(GLdouble)((double)y + cos(angle) * radius));
	}

	cgi_glEnd();

	canvas_reportOpenGLErrors(canvas, __FUNCTION__);
}

void draw2d_fillCircle(Canvas *canvas, Color *color, int x, int y,
		double radius)
{
	int ii = 0;
	double angle = 0.0;

	if((canvas == NULL) || (color == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	x += (int)radius;
	y += (int)radius;

	cgi_glColor4d(color->r, color->g, color->b, color->a);

	cgi_glBegin(GL_TRIANGLE_FAN);

	for(ii = 0; ii < 360; ii++) {
		angle = (double)ii;

		cgi_glVertex2d((GLdouble)((double)x + sin(angle) * radius),
				(GLdouble)((double)y + cos(angle) * radius));
	}

	cgi_glEnd();

	canvas_reportOpenGLErrors(canvas, __FUNCTION__);
}

void draw2d_font(Canvas *canvas, int x, int y, char *string, Font *font)
{
	if((canvas == NULL) || (font == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	if(string == NULL) {
		string = "";
	}

	font_draw2d(font, canvas, x, y, "%s", string);

	canvas_reportOpenGLErrors(canvas, __FUNCTION__);
}

void draw2d_colorFont(Canvas *canvas, int x, int y, char *string, Color *color,
		Font *font)
{
	if((canvas == NULL) || (color == NULL) || (font == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	if(string == NULL) {
		string = "";
	}

	cgi_glColor4d(color->r, color->g, color->b, color->a);

	font_draw2d(font, canvas, x, y, "%s", string);

	canvas_reportOpenGLErrors(canvas, __FUNCTION__);
}

void draw2d_texture(Canvas *canvas, int x, int y, int width, int height,
		Texture *texture)
{
	if((canvas == NULL) || (texture == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	texture_draw2d(texture, x, y, width, height);

	canvas_reportOpenGLErrors(canvas, __FUNCTION__);
}

void draw2d_rotatedTexture(Canvas *canvas, int x, int y, int width, int height,
		double rotationDegrees, Texture *texture)
{
	if((canvas == NULL) || (texture == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	beginRectangleRotation(x, y, width, height, rotationDegrees);

	texture_draw2d(texture, 0, 0, width, height);

	endRectangleRotation();

	canvas_reportOpenGLErrors(canvas, __FUNCTION__);
}

void draw2d_draw2dObject(Draw2dObject *object)
{
	if(object == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	switch(object->type) {
		case DRAW2D_TYPE_COLOR:
			draw2d_color(object->canvas,
					((Draw2dColor *)object->payload)->color);
			break;

		case DRAW2D_TYPE_FONT:
			draw2d_font(object->canvas,
					((Draw2dFont *)object->payload)->x,
					((Draw2dFont *)object->payload)->y,
					((Draw2dFont *)object->payload)->string,
					((Draw2dFont *)object->payload)->font);
			break;

		case DRAW2D_TYPE_COLOR_FONT:
			draw2d_color(object->canvas,
					((Draw2dColorFont *)object->payload)->color);
			draw2d_font(object->canvas,
					((Draw2dColorFont *)object->payload)->x,
					((Draw2dColorFont *)object->payload)->y,
					((Draw2dColorFont *)object->payload)->string,
					((Draw2dColorFont *)object->payload)->font);
			break;

		case DRAW2D_TYPE_TEXTURE:
			draw2d_texture(object->canvas,
					((Draw2dTexture *)object->payload)->x,
					((Draw2dTexture *)object->payload)->y,
					((Draw2dTexture *)object->payload)->width,
					((Draw2dTexture *)object->payload)->height,
					((Draw2dTexture *)object->payload)->texture);
			break;

		default:
			DISPLAY_INVALID_ARGS;
	}
}

// object functions

Draw2dObject *draw2d_newObjectColor(Canvas *canvas, Color *color)
{
	Draw2dColor *payload = NULL;
	Draw2dObject *result = NULL;

	if((canvas == NULL) || (color == NULL)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	payload = (Draw2dColor *)malloc(sizeof(Draw2dColor));

	payload->color = color;

	result = (Draw2dObject *)malloc(sizeof(Draw2dObject));

	result->type = DRAW2D_TYPE_COLOR;
	result->payload = (void *)payload;
	result->canvas = canvas;

	return result;
}

Draw2dObject *draw2d_newObjectFont(Canvas *canvas, int x, int y, char *string,
		Font *font)
{
	Draw2dFont *payload = NULL;
	Draw2dObject *result = NULL;

	if((canvas == NULL) || (string == NULL) || (font == NULL)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	payload = (Draw2dFont *)malloc(sizeof(Draw2dFont));

	payload->x = x;
	payload->y = y;
	payload->string = string; // TODO: should this be string-duped?
	payload->font = font;

	result = (Draw2dObject *)malloc(sizeof(Draw2dObject));

	result->type = DRAW2D_TYPE_FONT;
	result->payload = (void *)payload;
	result->canvas = canvas;

	return result;
}

Draw2dObject *draw2d_newObjectColorFont(Canvas *canvas, int x, int y,
		char *string, Color *color, Font *font)
{
	Draw2dColorFont *payload = NULL;
	Draw2dObject *result = NULL;

	if((canvas == NULL) || (string == NULL) || (font == NULL)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	payload = (Draw2dColorFont *)malloc(sizeof(Draw2dColorFont));

	payload->x = x;
	payload->y = y;
	payload->string = string; // TODO: should this be string-duped?
	payload->color = color;
	payload->font = font;

	result = (Draw2dObject *)malloc(sizeof(Draw2dObject));

	result->type = DRAW2D_TYPE_COLOR_FONT;
	result->payload = (void *)payload;
	result->canvas = canvas;

	return result;
}

Draw2dObject *draw2d_newObjectTexture(Canvas *canvas, int x, int y, int width,
		int height, Texture *texture)
{
	Draw2dTexture *payload = NULL;
	Draw2dObject *result = NULL;

	if((canvas == NULL) || (texture == NULL)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	payload = (Draw2dTexture *)malloc(sizeof(Draw2dTexture));

	payload->x = x;
	payload->y = y;
	payload->width = width;
	payload->height = height;
	payload->texture = texture;

	result = (Draw2dObject *)malloc(sizeof(Draw2dObject));

	result->type = DRAW2D_TYPE_TEXTURE;
	result->payload = (void *)payload;
	result->canvas = canvas;

	return result;
}

int draw2d_freeObject(Draw2dObject *object)
{
	if(object == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	switch(object->type) {
		case DRAW2D_TYPE_COLOR:
		case DRAW2D_TYPE_TEXTURE:
			if(object->payload != NULL) {
				free(object->payload);
			}
			break;
		case DRAW2D_TYPE_COLOR_FONT:
		case DRAW2D_TYPE_FONT:
			/*
			 * TODO: not freeing the payload string at this time, not duped
			 */
			if(object->payload != NULL) {
				free(object->payload);
			}
			break;
		default:
			DISPLAY_INVALID_ARGS;
	}

	free(object);

	return 0;
}

