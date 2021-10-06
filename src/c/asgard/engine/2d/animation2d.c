/*
 * animation2d.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The 2D animation system.
 *
 * Written by Josh English.
 */


// preprocessor directives

#include "core/core.h"
#include "asgard/asgard.h"
#define _ASGARD_ENGINE_2D_COMPONENT
#include "asgard/engine/2d/animation2d.h"


// define 2d animation public functions

void animation2d_init(Animation2d *animation, char *name, int x, int y,
		int width, int height, void *engine)
{
	if((animation == NULL) || (engine == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	memset(animation, 0, (sizeof(Animation2d)));

	animation->isVisible = atrue;
	animation->isPlaying = afalse;
	animation->willAutoStop = afalse;
	animation->position.x = x;
	animation->position.y = y;
	animation->position.width = width;
	animation->position.height = height;
	animation->offsetX = 0;
	animation->offsetY = 0;
	animation->frameCount = 0;
	animation->animationRef = 0;
	animation->animationTime = 0.0;
	animation->animationThresholdMillis = 0.0;
	animation->name = strdup(name);
	animation->frames = NULL;
	animation->engine = engine;
}

Animation2d *animation2d_new(char *name, int x, int y, int width, int height,
		void *engine)
{
	Animation2d *result = NULL;

	if(engine == NULL) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	result = (Animation2d *)malloc(sizeof(Animation2d));

	animation2d_init(result, name, x, y, width, height, engine);

	return result;
}

Animation2d *animation2d_load(Log *log, char *assetFilename)
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

void animation2d_free(Animation2d *animation)
{
	if(animation == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	/*
	 * TODO: this function
	 */

	return;
}

Position2d *animation2d_getPosition(Animation2d *animation)
{
	if(animation == NULL) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	return &(animation->position);
}

void animation2d_setPosition(Animation2d *animation, int x, int y)
{
	if(animation == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	animation->position.x = x;
	animation->position.y = y;
}

int animation2d_getWidth(Animation2d *animation)
{
	if(animation == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return animation->position.width;
}

int animation2d_getHeight(Animation2d *animation)
{
	if(animation == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return animation->position.height;
}

void animation2d_resize(Animation2d *animation, int width, int height)
{
	if(animation == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	animation->position.width = width;
	animation->position.height = height;
}

aboolean animation2d_isVisible(Animation2d *animation)
{
	if(animation == NULL) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	return animation->isVisible;
}

void animation2d_setVisibility(Animation2d *animation, aboolean isVisible)
{
	if(animation == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	animation->isVisible = isVisible;
}

aboolean animation2d_apply2dCamera(Animation2d *animation, int cameraX,
		int cameraY, int cameraWidth, int cameraHeight)
{
	if(animation == NULL) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	if(
		((animation->position.x >= cameraX) ||
		 ((animation->position.x + animation->position.width) >= cameraX)) &&
		((animation->position.y >= cameraY) ||
		 ((animation->position.y + animation->position.height) >= cameraY)) &&
		(animation->position.x <= (cameraX + cameraWidth)) &&
		(animation->position.y <= (cameraY + cameraHeight))
			) {
		animation->isVisible = atrue;
		animation->offsetX = (0 - cameraX);
		animation->offsetY = (0 - cameraY);
	}
	else {
		animation->isVisible = afalse;
	}

	return animation->isVisible;
}

void animation2d_addFrameTexture(Animation2d *animation, Texture *texture)
{
	int id = 0;

	if((animation == NULL) || (texture == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	if(animation->frames == NULL) {
		id = 0;
		animation->frameCount = 1;
		animation->frames = (Texture **)malloc(sizeof(Texture *) *
				animation->frameCount);
	}
	else {
		id = animation->frameCount;
		animation->frameCount += 1;
		animation->frames = (Texture **)realloc(animation->frames,
				(sizeof(Texture *) * animation->frameCount));
	}

	animation->frames[id] = texture;
}

Texture *animation2d_getFrameTexture(Animation2d *animation, int id)
{
	if((animation == NULL) || (id < 0) || (id >= animation->frameCount)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	return animation->frames[id];
}

double animation2d_getAnimationTime(Animation2d *animation)
{
	if(animation == NULL) {
		DISPLAY_INVALID_ARGS;
		return 0.0;
	}

	return animation->animationThresholdMillis;
}

void animation2d_setAnimationTime(Animation2d *animation, double timeMillis)
{
	if(animation == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	animation->animationThresholdMillis = timeMillis;
}

aboolean animation2d_isPlaying(Animation2d *animation)
{
	if(animation == NULL) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	return animation->isPlaying;
}

void animation2d_start(Animation2d *animation, aboolean willAutoStop)
{
	if(animation == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	if(!animation->isVisible) {
		animation->isPlaying = afalse;
		return;
	}

	if(animation->isPlaying) {
		return;
	}

	animation->isPlaying = atrue;
	animation->willAutoStop = willAutoStop;
	animation->animationRef = 0;
	animation->animationTime = time_getTimeMus();
}

void animation2d_stop(Animation2d *animation)
{
	if(animation == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	animation->isPlaying = afalse;
}

void animation2d_processAnimation(Animation2d *animation)
{
	double currentTime = 0.0;

	if(animation == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	if(!animation->isPlaying) {
		return;
	}

	currentTime = time_getTimeMus();
	if(currentTime >= (animation->animationTime +
				(animation->animationThresholdMillis * REAL_ONE_THOUSAND))) {
		animation->animationTime = currentTime;
		animation->animationRef += 1;
		if(animation->animationRef >= animation->frameCount) {
			animation->animationRef = 0;
			if(animation->willAutoStop) {
				animation->isPlaying = afalse;
			}
		}
	}
}

void animation2d_render2d(Animation2d *animation)
{
	if(animation == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	if((animation->isVisible) && (animation->isPlaying)) {
		draw2d_texture(&((Engine *)animation->engine)->canvas,
				(animation->position.x + animation->offsetX),
				(animation->position.y + animation->offsetY),
				animation->position.width, animation->position.height,
				animation->frames[animation->animationRef]);

	}
}

