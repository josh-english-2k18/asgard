/*
 * demo.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * A simple 2D Asgard engine demo, header file.
 *
 * Written by Josh English.
 */

#if !defined(_APPS_DEMO_DEMO_H)

#define _APPS_DEMO_DEMO_H

#if !defined(_APPS_DEMO_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _APPS_DEMO_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// define demo public constants

typedef enum _DemoStates {
	DEMO_STATE_INIT = 1,
	DEMO_STATE_LOADING,
	DEMO_STATE_GAME_START,
	DEMO_STATE_GAME_PLAYING,
	DEMO_STATE_ERROR = -1
} DemoStates;

typedef enum _DemoPowerupTypes {
	DEMO_POWERUP_TYPE_GUN = 0,
	DEMO_POWERUP_TYPE_MISSILE,
	DEMO_POWERUP_TYPE_SHIELD,
	DEMO_POWERUP_TYPE_LENGTH,
	DEMO_POWERUP_TYPE_UNKNOWN = -1
} DemoPowerupTypes;

//#define DEMO_IS_PRODUCTION_BUILD					atrue
#define DEMO_IS_PRODUCTION_BUILD					afalse

//#define DEMO_DRAW_COLLISION_MAP					atrue
#define DEMO_DRAW_COLLISION_MAP					afalse

//#define DEMO_MAX_SPRITES						2
//#define DEMO_MAX_SPRITES						128
//#define DEMO_MAX_SPRITES						256
//#define DEMO_MAX_SPRITES						512
#define DEMO_MAX_SPRITES						1024

#define DEMO_SPRITES_STARTING_THRUST			0.0001


// define demo public data types

typedef struct _DemoSprite {
	aboolean isAlive;
	aboolean isFirstPass;
	double rotation;
	Sprite2d sprite;
	Physics2d physics;
	Color colors[5];
	Texture texture;
} DemoSprite;

typedef struct _DemoAssets {
	Texture *loadingScreen;
	Texture *spriteOne;
	Texture *spriteTwo;
	Texture *spriteThree;
	DemoSprite sprites[DEMO_MAX_SPRITES];
} DemoAssets;

typedef struct _DemoMouse {
	aboolean isDown;
	int xPosition;
	int yPosition;
} DemoMouse;

typedef struct _Demo {
	DemoStates state;
	char *name;
	Engine engine;
	DemoMouse mouse;
	DemoAssets assets;
	void *zipContext;
} Demo;


// declare demo public functions

int demo_init(Demo *context);

int demo_free(Demo *context);

int demo_handleMouseEvents(Demo *context, AsgardMouseEvent *mouseEvent);

int demo_render(Demo *context);

int demo_processGamepley(Demo *context);

int demo_ioFunction(void *engineContext, void *gameContext, void *argument);

int demo_renderFunction(void *engineContext, void *gameContext,
		void *argument);

int demo_gameplayFunction(void *engineContext, void *gameContext,
		void *argument);


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _APPS_DEMO_DEMO_H

