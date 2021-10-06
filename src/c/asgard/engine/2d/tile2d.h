/*
 * tile2d.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The 2D tile system, header file.
 *
 * Written by Josh English.
 */

#if !defined(_ASGARD_ENGINE_2D_TILE2D_H)

#define _ASGARD_ENGINE_2D_TILE2D_H

#if !defined(_ASGARD_H) && !defined(_ASGARD_ENGINE_2D_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _ASGARD_H || _ASGARD_ENGINE_2D_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// define 2d tile public data types

typedef struct _Tile2d {
	aboolean debugMode;
	aboolean isVisible;
	aboolean canCollide;
	aboolean canIntersect;
	int xOffset;
	int yOffset;
	double friction;
	char *name;
	Color *debugColor;
	Texture *texture;
	Position2d position;
	Canvas *canvas;
} Tile2d;


// declare 2d tile public functions

void tile2d_init(Tile2d *tile, char *name, int x, int y, int width,
		int height, Canvas *canvas);

Tile2d *tile2d_new(char *name, int x, int y, int width, int height,
		Canvas *canvas);

Tile2d *tile2d_load(Log *log, char *assetFilename);

void tile2d_free(Tile2d *tile);

Position2d *tile2d_getPosition(Tile2d *tile);

void tile2d_setPosition(Tile2d *tile, int x, int y);

void tile2d_setOffset(Tile2d *tile, int x, int y);

int tile2d_getX(Tile2d *tile);

int tile2d_getXWithOffset(Tile2d *tile);

int tile2d_getOffsetX(Tile2d *tile);

int tile2d_getY(Tile2d *tile);

int tile2d_getYWithOffset(Tile2d *tile);

int tile2d_getOffsetY(Tile2d *tile);

int tile2d_getWidth(Tile2d *tile);

int tile2d_getHeight(Tile2d *tile);

void tile2d_resize(Tile2d *tile, int width, int height);

aboolean tile2d_isDebugMode(Tile2d *tile);

void tile2d_setDebugMode(Tile2d *tile, aboolean debugMode);

aboolean tile2d_isVisible(Tile2d *tile);

aboolean tile2d_canCollide(Tile2d *tile);

void tile2d_setCanCollide(Tile2d *tile, aboolean canCollide);

aboolean tile2d_canIntersect(Tile2d *tile);

void tile2d_setCanIntersect(Tile2d *tile, aboolean canIntersect);

double tile2d_getFriction(Tile2d *tile);

void tile2d_setFriction(Tile2d *tile, double friction);

Texture *tile2d_getTexture(Tile2d *tile);

void tile2d_setTexture(Tile2d *tile, Texture *texture);

aboolean tile2d_detectIntersection(Tile2d *tile, int x, int y);

aboolean tile2d_detectCollision(Tile2d *tile, Position2d *position);

aboolean tile2d_applyCamera(Tile2d *tile, Camera2d *camera);

void tile2d_render2d(Tile2d *tile);


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _ASGARD_ENGINE_2D_TILE2D_H

