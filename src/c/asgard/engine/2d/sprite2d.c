/*
 * sprite2d.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The 2D sprite system, header file.
 *
 * Written by Josh English.
 */


// preprocessor directives

#include "core/core.h"
#include "asgard/asgard.h"
#define _ASGARD_ENGINE_2D_COMPONENT
#include "asgard/engine/2d/sprite2d.h"


// define 2d sprite private constants

static char *DIRECTION_NAMES[] = {
	"North",
	"North-North-East",
	"North-East",
	"North-East-North",
	"East",
	"South-East-South",
	"South-East",
	"South-South-East",
	"South",
	"South-South-West",
	"South-West",
	"South-West-South",
	"West",
	"North-West-North",
	"North-West",
	"North-North-West",
};


// declare 2d sprite private functions

static void calculateRotation(Sprite2d *sprite, double frameRate);


// define 2d sprite private functions

static void calculateRotation(Sprite2d *sprite, double frameRate)
{
	double degreesPerMilli = 0.0;
	double millisPerFrame = 0.0;

	degreesPerMilli = (SPRITE2D_ROTATE_DEGREES /
			(sprite->rotateTimeSeconds * 1000.0));
	millisPerFrame = (1000.0 / frameRate);
	sprite->rotateAmount = (degreesPerMilli * millisPerFrame);

	sprite->rotationCount = 0;
	sprite->rotationTotal = (int)(SPRITE2D_ROTATE_DEGREES /
			sprite->rotateAmount);
	sprite->rotationMidPoint = (int)((SPRITE2D_ROTATE_DEGREES /
				sprite->rotateAmount) / 2.0);

	if(sprite->rotationTotal < 3) {
		sprite->rotationTotal = 3;
		sprite->rotationMidPoint = 2;
		sprite->rotateAmount = (SPRITE2D_ROTATE_DEGREES / 3.0);
	}
}

// define 2d sprite public functions

void sprite2d_init(Sprite2d *sprite, char *name, int x, int y, int width,
		int height, Canvas *canvas, void *engine)
{
	if((sprite == NULL) || (canvas == NULL) || (engine == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	memset(sprite, 0, (sizeof(Sprite2d)));

	sprite->debugMode = afalse;
	sprite->isVisible = atrue;
	sprite->isStatic = afalse;
	sprite->isOutline = afalse;
	sprite->isSelectable = afalse;
	sprite->isSelected = afalse;
	sprite->hasOutline = afalse;
	sprite->outlineWidth = width;
	sprite->outlineHeight = height;
	sprite->xOffset = 0;
	sprite->yOffset = 0;
	sprite->xLastPosition = 0;
	sprite->yLastPosition = 0;
	sprite->direction = SPRITE2D_DEFAULT_DIRECTION;
	sprite->destDirection = SPRITE2D_DEFAULT_DIRECTION;
	sprite->nextDirection = SPRITE2D_DEFAULT_DIRECTION;
	sprite->clockDirection = SPRITE2D_DEFAULT_TURN;
	sprite->rotationCount = 0;
	sprite->rotationTotal = 0;
	sprite->rotationMidPoint = 0;
	sprite->currentRotation = 0.0;
	sprite->rotateAmount = 0.0;
	sprite->rotateTimeSeconds = 0.0;
	sprite->outlineRatio = SPRITE2D_DEFAULT_OUTLINE_RATIO;
	sprite->name = strdup(name);
	sprite->debugColor = colors_newByName(CGI_COLOR_RED_NAME);
	sprite->outlineColor = colors_newByName(CGI_COLOR_BLUE_NAME);
	sprite->outlineTexture = NULL;
	sprite->textures = (Texture **)malloc(sizeof(Texture *) *
			SPRITE2D_DIRECTIONS);
	sprite->position.x = x;
	sprite->position.y = y;
	sprite->position.width = width;
	sprite->position.height = height;
	sprite->canvas = canvas;
	sprite->processMouseEvent =
		(Sprite2dProcessMouseEventFunction)sprite2d_processMouseEvent;
	sprite->processGameplay =
		(Sprite2dProcessGameplayFunction)sprite2d_processGameplay;
	sprite->render2d =
		(Sprite2dRender2dFunction)sprite2d_render2d;
	sprite->engine = engine;
}

Sprite2d *sprite2d_new(char *name, int x, int y, int width, int height,
		Canvas *canvas, void *engine)
{
	Sprite2d *result = NULL;

	if((canvas == NULL) || (engine == NULL)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	result = (Sprite2d *)malloc(sizeof(Sprite2d));

	sprite2d_init(result, name, x, y, width, height, canvas, engine);

	return result;
}

Sprite2d *sprite2d_load(Log *log, char *assetFilename)
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

void sprite2d_free(Sprite2d *sprite)
{
	if(sprite == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	/*
	 * TODO: this function
	 */

	return;
}

Position2d *sprite2d_getPosition(Sprite2d *sprite)
{
	if(sprite == NULL) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	return &(sprite->position);
}

void sprite2d_setPosition(Sprite2d *sprite, int x, int y)
{
	if(sprite == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	sprite->xLastPosition = sprite->position.x;
	sprite->yLastPosition = sprite->position.y;

	sprite->position.x = x;
	sprite->position.y = y;
}

void sprite2d_setOffset(Sprite2d *sprite, int x, int y)
{
	if(sprite == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	sprite->xOffset = x;
	sprite->yOffset = y;
}

int sprite2d_getX(Sprite2d *sprite)
{
	if(sprite == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return sprite->position.x;
}

int sprite2d_getXWithOffset(Sprite2d *sprite)
{
	if(sprite == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return (sprite->position.x + sprite->xOffset);
}

int sprite2d_getOffsetX(Sprite2d *sprite)
{
	if(sprite == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return sprite->xOffset;
}

int sprite2d_getCameraDiffX(Sprite2d *sprite)
{
	int result = 0;

	if(sprite == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	result = (sprite->position.x - sprite->xLastPosition);
	sprite->xLastPosition = sprite->position.x;

	return result;
}

int sprite2d_getY(Sprite2d *sprite)
{
	if(sprite == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return sprite->position.y;
}

int sprite2d_getYWithOffset(Sprite2d *sprite)
{
	if(sprite == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return (sprite->position.y + sprite->yOffset);
}

int sprite2d_getOffsetY(Sprite2d *sprite)
{
	if(sprite == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return sprite->yOffset;
}

int sprite2d_getCameraDiffY(Sprite2d *sprite)
{
	int result = 0;

	if(sprite == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	result = (sprite->position.y - sprite->yLastPosition);
	sprite->yLastPosition = sprite->position.y;

	return result;
}

int sprite2d_getWidth(Sprite2d *sprite)
{
	if(sprite == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return sprite->position.width;
}

int sprite2d_getHeight(Sprite2d *sprite)
{
	if(sprite == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return sprite->position.height;
}

void sprite2d_resize(Sprite2d *sprite, int width, int height)
{
	if(sprite == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	if(sprite->position.width == sprite->outlineWidth) {
		sprite->outlineWidth = width;
	}

	sprite->position.width = width;

	if(sprite->position.height == sprite->outlineHeight) {
		sprite->outlineHeight = height;
	}

	sprite->position.height = height;
}

aboolean sprite2d_isDebugMode(Sprite2d *sprite)
{
	if(sprite == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return sprite->debugMode;
}

void sprite2d_setDebugMode(Sprite2d *sprite, aboolean debugMode)
{
	if(sprite == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	sprite->debugMode = debugMode;
}

aboolean sprite2d_isVisible(Sprite2d *sprite)
{
	if(sprite == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return sprite->isVisible;
}

aboolean sprite2d_isStatic(Sprite2d *sprite)
{
	if(sprite == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return sprite->isStatic;
}

void sprite2d_setIsStatic(Sprite2d *sprite, aboolean isStatic)
{
	if(sprite == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	sprite->isStatic = isStatic;
}

aboolean sprite2d_isOutline(Sprite2d *sprite)
{
	if(sprite == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return sprite->isOutline;
}

void sprite2d_setIsOutline(Sprite2d *sprite, aboolean isOutline)
{
	if(sprite == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	sprite->isOutline = isOutline;
}

aboolean sprite2d_hasOutline(Sprite2d *sprite)
{
	if(sprite == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return sprite->hasOutline;
}

void sprite2d_setHasOutline(Sprite2d *sprite, aboolean hasOutline)
{
	if(sprite == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	sprite->hasOutline = hasOutline;
}

aboolean sprite2d_isSelectable(Sprite2d *sprite)
{
	if(sprite == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return sprite->isSelectable;
}

void sprite2d_setIsSelectable(Sprite2d *sprite, aboolean isSelectable)
{
	if(sprite == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	sprite->isSelectable = isSelectable;
}

aboolean sprite2d_isSelected(Sprite2d *sprite)
{
	if(sprite == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return sprite->isSelected;
}

void sprite2d_setIsSelected(Sprite2d *sprite, aboolean isSelected)
{
	if(sprite == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	sprite->isSelected = isSelected;
}

void sprite2d_setOutlineRatio(Sprite2d *sprite, double outlineRatio)
{
	if(sprite == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	sprite->outlineRatio = outlineRatio;
}

void sprite2d_setOutlineSize(Sprite2d *sprite, int width, int height)
{
	if(sprite == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	sprite->outlineWidth = width;
	sprite->outlineHeight = height;
}

void sprite2d_setOutlineColor(Sprite2d *sprite, Color *outlineColor)
{
	if(sprite == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	if(sprite->outlineColor != NULL) {
		colors_free(sprite->outlineColor);
	}

	sprite->isOutline = atrue;
	sprite->outlineColor = outlineColor;
}

void sprite2d_setOutlineTexture(Sprite2d *sprite, Texture *outlineTexture)
{
	if((sprite == NULL) || (outlineTexture == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	sprite->isOutline = atrue;
	sprite->outlineTexture = outlineTexture;
}

double sprite2d_getRotateTimeSeconds(Sprite2d *sprite)
{
	if(sprite == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return sprite->rotateTimeSeconds;
}

void sprite2d_setRotateTimeSeconds(Sprite2d *sprite, double rotateTimeSeconds)
{
	if(sprite == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	sprite->rotateTimeSeconds = rotateTimeSeconds;
}

int sprite2d_getDirection(Sprite2d *sprite)
{
	if(sprite == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return sprite->direction;
}

void sprite2d_resetDirection(Sprite2d *sprite, Sprite2dDirection direction)
{
	if((sprite == NULL) ||
			(direction < SPRITE2D_DIRECTION_NORTH) ||
			(direction > SPRITE2D_DIRECTION_NORTH_NORTH_WEST)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	sprite->direction = direction;
	sprite->destDirection = direction;
	sprite->nextDirection = direction;
}

void sprite2d_changeDirection(Sprite2d *sprite, Sprite2dDirection direction,
		Sprite2dTurn clockDirection, double frameRate)
{
	if((sprite == NULL) ||
			(direction < SPRITE2D_DIRECTION_NORTH) ||
			(direction > SPRITE2D_DIRECTION_NORTH_NORTH_WEST) ||
			(clockDirection < SPRITE2D_TURN_CLOCKWISE) ||
			(clockDirection > SPRITE2D_TURN_COUNTER_CLOCKWISE)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	if((sprite->destDirection != sprite->direction) ||
			(sprite->currentRotation != 0.0)) {
		return;
	}

	sprite->destDirection = direction;

	if(clockDirection == SPRITE2D_TURN_CLOCKWISE) {
		sprite->nextDirection = ((sprite->direction + 1) % SPRITE2D_DIRECTIONS);
		calculateRotation(sprite, frameRate);
	}
	else {
		sprite->nextDirection = (sprite->direction - 1);
		if(sprite->nextDirection < 0) {
			sprite->nextDirection = (SPRITE2D_DIRECTIONS - 1);
		}
		calculateRotation(sprite, frameRate);
	}

	sprite->clockDirection = clockDirection;
}

Texture *sprite2d_getTexture(Sprite2d *sprite, Sprite2dDirection direction)
{
	if((sprite == NULL) ||
			(direction < SPRITE2D_DIRECTION_NORTH) ||
			(direction > SPRITE2D_DIRECTION_NORTH_NORTH_WEST)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	return sprite->textures[direction];
}

void sprite2d_setTexture(Sprite2d *sprite, Sprite2dDirection direction,
		Texture *texture)
{
	if((sprite == NULL) || (texture == NULL) ||
			(direction < SPRITE2D_DIRECTION_NORTH) ||
			(direction > SPRITE2D_DIRECTION_NORTH_NORTH_WEST)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	sprite->textures[direction] = texture;
}

aboolean sprite2d_applyCamera(Sprite2d *sprite, Camera2d *camera)
{
	if((sprite == NULL) || (camera == NULL)) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	if(((sprite->position.x >= camera->position.x) ||
				((sprite->position.x + sprite->position.width) >=
				 camera->position.x)) &&
			(sprite->position.x <=
			 (camera->position.x + camera->position.width)) &&
			((sprite->position.y >= camera->position.y) ||
			 ((sprite->position.y + sprite->position.height) >=
			  camera->position.y)) &&
			(sprite->position.y <=
			 (camera->position.y + camera->position.height))) {
		sprite->isVisible = atrue;
		sprite->xOffset = (0 - camera->position.x);
		sprite->yOffset = (0 - camera->position.y);
	}
	else {
		sprite->isVisible = afalse;
	}

	return sprite->isVisible;
}

void sprite2d_processMouseEvent(Sprite2d *sprite, AsgardMouseEvent *event)
{
	if((sprite == NULL) || (event == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	if((!sprite->isOutline) && (!sprite->isSelectable)) {
		return;
	}

	if((sprite->isSelectable) && (event->eventType == EVENT_MOUSE_PRESSED)) {
		if((event->xPosition >= (sprite->position.x + sprite->xOffset)) &&
				(event->xPosition <=
				 ((sprite->position.x + sprite->xOffset) +
				  sprite->position.width)) &&
				(event->yPosition >=
				 (sprite->position.y + sprite->yOffset)) &&
				(event->yPosition <=
				 ((sprite->position.y + sprite->yOffset) +
				  sprite->position.height))) {
			sprite->isSelected = atrue;
			if(sprite->isOutline) {
				sprite->hasOutline = atrue;
			}
		}
		else {
			sprite->isSelected = afalse;
			if(sprite->isOutline) {
				sprite->hasOutline = afalse;
			}
		}
	}

	if(sprite->isSelected) {
		return;
	}

	if((event->xPosition >= (sprite->position.x + sprite->xOffset)) &&
			(event->xPosition <=
			 ((sprite->position.x + sprite->xOffset) +
			  sprite->position.width)) &&
			(event->yPosition >= (sprite->position.y + sprite->yOffset)) &&
			(event->yPosition <=
			 ((sprite->position.y + sprite->yOffset) +
			  sprite->position.height))) {
		sprite->hasOutline = atrue;
	}
	else {
		sprite->hasOutline = afalse;
	}
}

void sprite2d_processGameplay(Sprite2d *sprite, double frameRate)
{
	if(sprite == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	if((sprite->destDirection == sprite->direction) &&
			(sprite->currentRotation == 0.0)) {
		return;
	}

	if((sprite->nextDirection == sprite->direction) &&
			(sprite->direction != sprite->destDirection)) {
		if(sprite->clockDirection == SPRITE2D_TURN_CLOCKWISE) {
			sprite->nextDirection = ((sprite->direction + 1) %
					SPRITE2D_DIRECTIONS);
			calculateRotation(sprite, frameRate);
			sprite->currentRotation = sprite->rotateAmount;
		}
		else {
			sprite->nextDirection = (sprite->direction - 1);
			if(sprite->nextDirection < 0) {
				sprite->nextDirection = (SPRITE2D_DIRECTIONS - 1);
			}
			calculateRotation(sprite, frameRate);
			sprite->currentRotation = -sprite->rotateAmount;
		}
	}
	else {
		if(sprite->clockDirection == SPRITE2D_TURN_CLOCKWISE) {
			sprite->currentRotation += sprite->rotateAmount;
		}
		else {
			sprite->currentRotation -= sprite->rotateAmount;
		}
		sprite->rotationCount += 1;
		if(sprite->rotationCount == sprite->rotationMidPoint) {
			sprite->currentRotation *= -1.0;
			sprite->direction = sprite->nextDirection;
		}
		else if(sprite->rotationCount >= sprite->rotationTotal) {
			sprite->currentRotation = 0.0;
		}
	}
}

void sprite2d_render2d(Sprite2d *sprite)
{
	aboolean isBlendEnabled = afalse;
	char buffer[8192];

	if(sprite == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	if(!sprite->isVisible) {
		return;
	}

	// render texture outline (if set & mouse hover-over)

	if((sprite->isOutline) && (sprite->hasOutline) &&
			(sprite->outlineTexture != NULL)) {
		draw2d_texture(sprite->canvas,
				(sprite->position.x + sprite->xOffset),
				(sprite->position.y + sprite->yOffset),
				sprite->position.width,
				sprite->position.height,
				sprite->outlineTexture);
	}

	// render sprite

	if(sprite->currentRotation != 0.0) {
		draw2d_rotatedTexture(sprite->canvas,
				(sprite->position.x + sprite->xOffset),
				(sprite->position.y + sprite->yOffset),
				sprite->position.width,
				sprite->position.height,
				sprite->currentRotation,
				sprite->textures[sprite->direction]);
	}
	else {
		draw2d_texture(sprite->canvas,
				(sprite->position.x + sprite->xOffset),
				(sprite->position.y + sprite->yOffset),
				sprite->position.width,
				sprite->position.height,
				sprite->textures[sprite->direction]);
	}

	// render debug mode

	if(sprite->debugMode) {
		CGI_COLOR_WHITE;

		snprintf(buffer, (sizeof(buffer) - 1),
				"Sprite{%s}/d%1i|o%1i|s%1i|is%1i|h%1i/[#%i->%s]@(%i, %i):%ix%i",
				sprite->name,
				(int)sprite->debugMode,
				(int)sprite->isOutline,
				(int)sprite->isSelectable,
				(int)sprite->isSelected,
				(int)sprite->hasOutline,
				(int)sprite->direction,
				DIRECTION_NAMES[sprite->direction],
				(sprite->position.x + sprite->xOffset),
				(sprite->position.y + sprite->yOffset),
				sprite->position.width,
				sprite->position.height);

		draw2d_font(sprite->canvas,
				((sprite->position.x + sprite->xOffset) + 2),
				((sprite->position.y + sprite->yOffset) - 22),
				buffer,
				&((Engine *)sprite->engine)->font);

		if(cgi_glIsEnabled(GL_BLEND)) {
			isBlendEnabled = atrue;
		}
		else {
			cgi_glEnable(GL_BLEND);
		}

		cgi_glBlendFunc(GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA);

		draw2d_rectangle(sprite->canvas, sprite->debugColor,
				(sprite->position.x + sprite->xOffset),
				(sprite->position.y + sprite->yOffset),
				sprite->position.width,
				sprite->position.height);

		if(!isBlendEnabled) {
			cgi_glDisable(GL_BLEND);
		}
	}

	// render color outline (if set & mouse hover-over)

	if((sprite->isOutline) && (sprite->hasOutline) &&
			(sprite->outlineTexture == NULL)) {
		// draw outline top

		draw2d_line(sprite->canvas, sprite->outlineColor,
				(sprite->position.x + sprite->xOffset),
				(sprite->position.y + sprite->yOffset),
				((sprite->position.x + sprite->xOffset) +
				 (int)((double)sprite->outlineWidth *
					 (sprite->outlineRatio / 100.0))),
				(sprite->position.y + sprite->yOffset));

		draw2d_line(sprite->canvas, sprite->outlineColor, 
				(((sprite->position.x + sprite->xOffset) +
				  sprite->outlineWidth) -
				 (int)((double)sprite->outlineWidth *
					  (sprite->outlineRatio / 100.0))),
				(sprite->position.y + sprite->yOffset),
				((sprite->position.x + sprite->xOffset) +
				 sprite->outlineWidth),
				(sprite->position.y + sprite->yOffset));

		// draw outline right

		draw2d_line(sprite->canvas, sprite->outlineColor,
				((sprite->position.x + sprite->xOffset) +
				 sprite->outlineWidth),
				(sprite->position.y + sprite->yOffset),
				((sprite->position.x + sprite->xOffset) +
				 sprite->outlineWidth),
				((sprite->position.y + sprite->yOffset) +
				 (int)((double)sprite->outlineHeight *
					 (sprite->outlineRatio / 100.00))));

		draw2d_line(sprite->canvas, sprite->outlineColor,
				((sprite->position.x + sprite->xOffset) +
				 sprite->outlineWidth),
				((sprite->position.y + sprite->yOffset) +
				 (sprite->outlineHeight -
				  (int)((double)sprite->outlineHeight *
					  (sprite->outlineRatio / 100.00)))),
				((sprite->position.x + sprite->xOffset) +
				 sprite->outlineWidth),
				((sprite->position.y + sprite->yOffset) +
				 sprite->outlineHeight));

		// draw outline bottom

		draw2d_line(sprite->canvas, sprite->outlineColor,
				((sprite->position.x + sprite->xOffset) +
				 sprite->outlineWidth),
				((sprite->position.y + sprite->yOffset) +
				 sprite->outlineHeight),
				(((sprite->position.x + sprite->xOffset) +
				  sprite->outlineWidth) -
				 (int)((double)sprite->outlineWidth *
					 (sprite->outlineRatio / 100.0))),
				((sprite->position.y + sprite->yOffset) +
				 sprite->outlineHeight));

		draw2d_line(sprite->canvas, sprite->outlineColor,
				((sprite->position.x + sprite->xOffset) +
				 (int)((double)sprite->outlineWidth *
					 (sprite->outlineRatio / 100.0))),
				((sprite->position.y + sprite->yOffset) +
				 sprite->outlineHeight),
				(sprite->position.x + sprite->xOffset),
				((sprite->position.y + sprite->yOffset) +
				 sprite->outlineHeight));

		// draw outline left

		draw2d_line(sprite->canvas, sprite->outlineColor,
				(sprite->position.x + sprite->xOffset),
				((sprite->position.y + sprite->yOffset) +
				 sprite->outlineHeight),
				(sprite->position.x + sprite->xOffset),
				((sprite->position.y + sprite->yOffset) +
				 (sprite->outlineHeight -
				  (int)((double)sprite->outlineHeight *
					  (sprite->outlineRatio / 100.00)))));

		draw2d_line(sprite->canvas, sprite->outlineColor,
				(sprite->position.x + sprite->xOffset),
				((sprite->position.y + sprite->yOffset) +
				 (int)((double)sprite->outlineHeight *
					 (sprite->outlineRatio / 100.00))),
				(sprite->position.x + sprite->xOffset),
				(sprite->position.y + sprite->yOffset));
	}
}

