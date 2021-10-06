/*
 * sprite2d.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The 2D sprite system, header file.
 *
 * Written by Josh English.
 */

#if !defined(_ASGARD_ENGINE_2D_SPRITE2D_H)

#define _ASGARD_ENGINE_2D_SPRITE2D_H

#if !defined(_ASGARD_H) && !defined(_ASGARD_ENGINE_2D_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _ASGARD_H || _ASGARD_ENGINE_2D_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// define 2d sprite public constants

typedef enum _Sprite2dDirection {
	SPRITE2D_DIRECTION_NORTH = 0,
	SPRITE2D_DIRECTION_NORTH_NORTH_EAST,
	SPRITE2D_DIRECTION_NORTH_EAST,
	SPRITE2D_DIRECTION_NORTH_EAST_NORTH,
	SPRITE2D_DIRECTION_EAST,
	SPRITE2D_DIRECTION_SOUTH_EAST_SOUTH,
	SPRITE2D_DIRECTION_SOUTH_EAST,
	SPRITE2D_DIRECTION_SOUTH_SOUTH_EAST,
	SPRITE2D_DIRECTION_SOUTH,
	SPRITE2D_DIRECTION_SOUTH_SOUTH_WEST,
	SPRITE2D_DIRECTION_SOUTH_WEST,
	SPRITE2D_DIRECTION_SOUTH_WEST_SOUTH,
	SPRITE2D_DIRECTION_WEST,
	SPRITE2D_DIRECTION_NORTH_WEST_NORTH,
	SPRITE2D_DIRECTION_NORTH_WEST,
	SPRITE2D_DIRECTION_NORTH_NORTH_WEST,
	SPRITE2D_DIRECTIONS,
	SPRITE2D_DIRECTION_ERROR = -1
} Sprite2dDirection;

typedef enum _Sprite2dTurn {
	SPRITE2D_TURN_CLOCKWISE = 1,
	SPRITE2D_TURN_COUNTER_CLOCKWISE,
	SPRITE2D_TURN_ERROR = -1
} Sprite2dTurn;

#define SPRITE2D_ROTATE_DEGREES							\
	(360.0 / SPRITE2D_DIRECTIONS)

#define SPRITE2D_DEFAULT_OUTLINE_RATIO					25.0

#define SPRITE2D_DEFAULT_ROTATE_TIME_SECONDS			0.125

#define SPRITE2D_DEFAULT_TURN							SPRITE2D_TURN_CLOCKWISE

#define SPRITE2D_DEFAULT_DIRECTION						SPRITE2D_DIRECTION_EAST


// define 2d sprite public data types

typedef void (*Sprite2dProcessMouseEventFunction)(void *sprite,
		AsgardMouseEvent *event);

typedef void (*Sprite2dProcessGameplayFunction)(void *sprite,
		double frameRate);

typedef void (*Sprite2dRender2dFunction)(void *sprite);

typedef struct _Sprite2d {
	aboolean debugMode;
	aboolean isVisible;
	aboolean isStatic;
	aboolean isOutline;
	aboolean isSelectable;
	aboolean isSelected;
	aboolean hasOutline;
	int outlineWidth;
	int outlineHeight;
	int xOffset;
	int yOffset;
	int xLastPosition;
	int yLastPosition;
	int direction;
	int destDirection;
	int nextDirection;
	int clockDirection;
	int rotationCount;
	int rotationTotal;
	int rotationMidPoint;
	double currentRotation;
	double rotateAmount;
	double rotateAmountSample;
	double rotateTimeSeconds;
	double rotateTimeStamp;
	double outlineRatio;
	char *name;
	Color *debugColor;
	Color *outlineColor;
	Texture *outlineTexture;
	Texture **textures;
	Position2d position;
	Canvas *canvas;
	Sprite2dProcessMouseEventFunction processMouseEvent;
	Sprite2dProcessGameplayFunction processGameplay;
	Sprite2dRender2dFunction render2d;
	void *engine;
} Sprite2d;


// declare 2d sprite public functions

void sprite2d_init(Sprite2d *sprite, char *name, int x, int y, int width,
		int height, Canvas *canvas, void *engine);

Sprite2d *sprite2d_new(char *name, int x, int y, int width, int height,
		Canvas *canvas, void *engine);

Sprite2d *sprite2d_load(Log *log, char *assetFilename);

void sprite2d_free(Sprite2d *sprite);

Position2d *sprite2d_getPosition(Sprite2d *sprite);

void sprite2d_setPosition(Sprite2d *sprite, int x, int y);

void sprite2d_setOffset(Sprite2d *sprite, int x, int y);

int sprite2d_getX(Sprite2d *sprite);

int sprite2d_getXWithOffset(Sprite2d *sprite);

int sprite2d_getOffsetX(Sprite2d *sprite);

int sprite2d_getCameraDiffX(Sprite2d *sprite);

int sprite2d_getY(Sprite2d *sprite);

int sprite2d_getYWithOffset(Sprite2d *sprite);

int sprite2d_getOffsetY(Sprite2d *sprite);

int sprite2d_getCameraDiffY(Sprite2d *sprite);

int sprite2d_getWidth(Sprite2d *sprite);

int sprite2d_getHeight(Sprite2d *sprite);

void sprite2d_resize(Sprite2d *sprite, int width, int height);

aboolean sprite2d_isDebugMode(Sprite2d *sprite);

void sprite2d_setDebugMode(Sprite2d *sprite, aboolean debugMode);

aboolean sprite2d_isVisible(Sprite2d *sprite);

aboolean sprite2d_isStatic(Sprite2d *sprite);

void sprite2d_setIsStatic(Sprite2d *sprite, aboolean isStatic);

aboolean sprite2d_isOutline(Sprite2d *sprite);

void sprite2d_setIsOutline(Sprite2d *sprite, aboolean isOutline);

aboolean sprite2d_hasOutline(Sprite2d *sprite);

void sprite2d_setHasOutline(Sprite2d *sprite, aboolean hasOutline);

aboolean sprite2d_isSelectable(Sprite2d *sprite);

void sprite2d_setIsSelectable(Sprite2d *sprite, aboolean isSelectable);

aboolean sprite2d_isSelected(Sprite2d *sprite);

void sprite2d_setIsSelected(Sprite2d *sprite, aboolean isSelected);

void sprite2d_setOutlineRatio(Sprite2d *sprite, double outlineRatio);

void sprite2d_setOutlineSize(Sprite2d *sprite, int width, int height);

void sprite2d_setOutlineColor(Sprite2d *sprite, Color *outlineColor);

void sprite2d_setOutlineTexture(Sprite2d *sprite, Texture *outlineTexture);

double sprite2d_getRotateTimeSeconds(Sprite2d *sprite);

void sprite2d_setRotateTimeSeconds(Sprite2d *sprite, double rotateTimeSeconds);

int sprite2d_getDirection(Sprite2d *sprite);

void sprite2d_resetDirection(Sprite2d *sprite, Sprite2dDirection direction);

void sprite2d_changeDirection(Sprite2d *sprite, Sprite2dDirection direction,
		Sprite2dTurn clockDirection, double frameRate);

Texture *sprite2d_getTexture(Sprite2d *sprite, Sprite2dDirection direction);

void sprite2d_setTexture(Sprite2d *sprite, Sprite2dDirection direction,
		Texture *texture);

aboolean sprite2d_applyCamera(Sprite2d *sprite, Camera2d *camera);

void sprite2d_processMouseEvent(Sprite2d *sprite, AsgardMouseEvent *event);

void sprite2d_processGameplay(Sprite2d *sprite, double frameRate);

void sprite2d_render2d(Sprite2d *sprite);


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _ASGARD_ENGINE_2D_SPRITE2D_H

