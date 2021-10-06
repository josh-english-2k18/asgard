/*
 * animation2d.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The 2D animation system, header file.
 *
 * Written by Josh English.
 */

#if !defined(_ASGARD_ENGINE_2D_ANIMATION2D_H)

#define _ASGARD_ENGINE_2D_ANIMATION2D_H

#if !defined(_ASGARD_H) && !defined(_ASGARD_ENGINE_2D_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _ASGARD_H || _ASGARD_ENGINE_2D_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// define 2d animation public data types

typedef struct _Animation2d {
	aboolean isVisible;
	aboolean isPlaying;
	aboolean willAutoStop;
	int offsetX;
	int offsetY;
	int frameCount;
	int animationRef;
	double animationTime;
	double animationThresholdMillis;
	char *name;
	Position2d position;
	Texture **frames;
	void *engine;
} Animation2d;


// declare 2d animation public functions

void animation2d_init(Animation2d *animation, char *name, int x, int y,
		int width, int height, void *engine);

Animation2d *animation2d_new(char *name, int x, int y, int width, int height,
		void *engine);

Animation2d *animation2d_load(Log *log, char *assetFilename);

void animation2d_free(Animation2d *animation);

Position2d *animation2d_getPosition(Animation2d *animation);

void animation2d_setPosition(Animation2d *animation, int x, int y);

int animation2d_getWidth(Animation2d *animation);

int animation2d_getHeight(Animation2d *animation);

void animation2d_resize(Animation2d *animation, int width, int height);

aboolean animation2d_isVisible(Animation2d *animation);

void animation2d_setVisibility(Animation2d *animation, aboolean isVisible);

aboolean animation2d_apply2dCamera(Animation2d *animation, int cameraX,
		int cameraY, int cameraWidth, int cameraHeight);

void animation2d_addFrameTexture(Animation2d *animation, Texture *texture);

Texture *animation2d_getFrameTexture(Animation2d *animation, int id);

double animation2d_getAnimationTime(Animation2d *animation);

void animation2d_setAnimationTime(Animation2d *animation, double timeMillis);

aboolean animation2d_isPlaying(Animation2d *animation);

void animation2d_start(Animation2d *animation, aboolean willAutoStop);

void animation2d_stop(Animation2d *animation);

void animation2d_processAnimation(Animation2d *animation);

void animation2d_render2d(Animation2d *animation);


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _ASGARD_ENGINE_2D_ANIMATION2D_H

