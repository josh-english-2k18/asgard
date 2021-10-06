/*
 * tile2d.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The 2D tile system, header file.
 *
 * Written by Josh English.
 */


// preprocessor directives

#include "core/core.h"
#include "asgard/asgard.h"
#define _ASGARD_ENGINE_2D_COMPONENT
#include "asgard/engine/2d/tile2d.h"


// define 2d tile public functions

void tile2d_init(Tile2d *tile, char *name, int x, int y, int width,
		int height, Canvas *canvas)
{
	if((tile == NULL) || (canvas == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	memset(tile, 0, (sizeof(Tile2d)));

	tile->debugMode = afalse;
	tile->isVisible = atrue;
	tile->canCollide = afalse;
	tile->canIntersect = afalse;
	tile->xOffset = 0;
	tile->yOffset = 0;
	tile->friction = 0.0;
	tile->name = strdup(name);
	tile->debugColor = colors_newByName(CGI_COLOR_RED_NAME);
	tile->texture = NULL;
	tile->position.x = x;
	tile->position.y = y;
	tile->position.width = width;
	tile->position.height = height;
	tile->canvas = canvas;
}

Tile2d *tile2d_new(char *name, int x, int y, int width, int height,
		Canvas *canvas)
{
	Tile2d *result = NULL;

	if(canvas == NULL) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	result = (Tile2d *)malloc(sizeof(Tile2d));

	tile2d_init(result, name, x, y, width, height, canvas);

	return result;
}

Tile2d *tile2d_load(Log *log, char *assetFilename)
{
	if((log == NULL) || (assetFilename == NULL)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	/*
	 * TODO: this function
	 */

	return NULL;
}

void tile2d_free(Tile2d *tile)
{
	if(tile == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	/*
	 * TODO: this function
	 */

	return;
}

Position2d *tile2d_getPosition(Tile2d *tile)
{
	if(tile == NULL) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	return &(tile->position);
}

void tile2d_setPosition(Tile2d *tile, int x, int y)
{
	if(tile == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	tile->position.x = x;
	tile->position.y = y;
}

void tile2d_setOffset(Tile2d *tile, int x, int y)
{
	if(tile == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	tile->xOffset = x;
	tile->yOffset = y;
}

int tile2d_getX(Tile2d *tile)
{
	if(tile == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return tile->position.x;
}

int tile2d_getXWithOffset(Tile2d *tile)
{
	if(tile == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return (tile->position.x + tile->xOffset);
}

int tile2d_getOffsetX(Tile2d *tile)
{
	if(tile == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return tile->xOffset;
}

int tile2d_getY(Tile2d *tile)
{
	if(tile == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return tile->position.y;
}

int tile2d_getYWithOffset(Tile2d *tile)
{
	if(tile == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return (tile->position.y + tile->yOffset);
}

int tile2d_getOffsetY(Tile2d *tile)
{
	if(tile == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return tile->yOffset;
}

int tile2d_getWidth(Tile2d *tile)
{
	if(tile == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return tile->position.width;
}

int tile2d_getHeight(Tile2d *tile)
{
	if(tile == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return tile->position.height;
}

void tile2d_resize(Tile2d *tile, int width, int height)
{
	if(tile == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	tile->position.width = width;
	tile->position.height = height;
}

aboolean tile2d_isDebugMode(Tile2d *tile)
{
	if(tile == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return tile->debugMode;
}

void tile2d_setDebugMode(Tile2d *tile, aboolean debugMode)
{
	if(tile == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	tile->debugMode = debugMode;
}

aboolean tile2d_isVisible(Tile2d *tile)
{
	if(tile == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return tile->isVisible;
}

aboolean tile2d_canCollide(Tile2d *tile)
{
	if(tile == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return tile->canCollide;
}

void tile2d_setCanCollide(Tile2d *tile, aboolean canCollide)
{
	if(tile == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	tile->canCollide = canCollide;
}

aboolean tile2d_canIntersect(Tile2d *tile)
{
	if(tile == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return tile->canIntersect;
}

void tile2d_setCanIntersect(Tile2d *tile, aboolean canIntersect)
{
	if(tile == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	tile->canIntersect = canIntersect;
}

double tile2d_getFriction(Tile2d *tile)
{
	if(tile == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return tile->friction;
}

void tile2d_setFriction(Tile2d *tile, double friction)
{
	if(tile == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	tile->friction = friction;
}

Texture *tile2d_getTexture(Tile2d *tile)
{
	if(tile == NULL) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	return tile->texture;
}

void tile2d_setTexture(Tile2d *tile, Texture *texture)
{
	if((tile == NULL) || (texture == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	tile->texture = texture;
}

aboolean tile2d_detectIntersection(Tile2d *tile, int x, int y)
{
	aboolean result = afalse;

	if(tile == NULL) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	if((x >= (tile->position.x + tile->xOffset)) &&
			(x <= (tile->position.x + tile->xOffset + tile->position.width)) &&
			(y >= (tile->position.y + tile->yOffset)) &&
			(y <= (tile->position.y + tile->yOffset + tile->position.height))) {
		result = atrue;
	}

	return result;
}

aboolean tile2d_detectCollision(Tile2d *tile, Position2d *position)
{
	aboolean result = afalse;

	if((tile == NULL) || (position == NULL)) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	if(tile2d_detectIntersection(tile, position->x, position->y)) {
		result = atrue;
	}
	else if(tile2d_detectIntersection(tile, (position->x + position->width),
				position->y)) {
		result = atrue;
	}
	else if(tile2d_detectIntersection(tile, position->x,
				(position->y + position->height))) {
		result = atrue;
	}
	else if(tile2d_detectIntersection(tile, (position->x + position->width),
				(position->y + position->height))) {
		result = atrue;
	}

	return atrue;
}

aboolean tile2d_applyCamera(Tile2d *tile, Camera2d *camera)
{
	if((tile == NULL) || (camera == NULL)) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	if(((tile->position.x >= camera->position.x) ||
				((tile->position.x + tile->position.width) >=
				 camera->position.x)) &&
			(tile->position.x <=
			 (camera->position.x + camera->position.width)) &&
			((tile->position.y >= camera->position.y) ||
			 ((tile->position.y + tile->position.height) >=
			  camera->position.y)) &&
			(tile->position.y <=
			 (camera->position.y + camera->position.height))) {
		tile->isVisible = atrue;
		tile->xOffset = (0 - camera->position.x);
		tile->yOffset = (0 - camera->position.y);
	}
	else {
		tile->isVisible = afalse;
	}

	return tile->isVisible;
}

void tile2d_render2d(Tile2d *tile)
{
	aboolean isBlendEnabled = afalse;

	if(tile == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	if(!tile->isVisible) {
		return;
	}

	// render tile

	draw2d_texture(tile->canvas,
			(tile->position.x + tile->xOffset),
			(tile->position.y + tile->yOffset),
			tile->position.width,
			tile->position.height,
			tile->texture);

	// render debug mode

	if(tile->debugMode) {
		if(cgi_glIsEnabled(GL_BLEND)) {
			isBlendEnabled = atrue;
		}
		else {
			cgi_glEnable(GL_BLEND);
		}

		cgi_glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);

		draw2d_rectangle(tile->canvas, tile->debugColor,
				(tile->position.x + tile->xOffset),
				(tile->position.y + tile->yOffset),
				tile->position.width,
				tile->position.height);

		if(!isBlendEnabled) {
			cgi_glDisable(GL_BLEND);
		}
	}
}

