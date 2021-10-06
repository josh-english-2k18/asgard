/*
 * troids.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * A simple 2D asteroids-style video game, header file.
 *
 * Written by Josh English.
 */

#if !defined(_APPS_TROIDS_TROIDS_H)

#define _APPS_TROIDS_TROIDS_H

#if !defined(_APPS_TROIDS_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _APPS_TROIDS_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// define troids public constants

typedef enum _TroidsStates {
	TROIDS_STATE_INIT = 1,
	TROIDS_STATE_LOADING,
	TROIDS_STATE_SPLASH_SCREEN,
	TROIDS_STATE_INSTRUCTION_SCREEN,
	TROIDS_STATE_GAME_START,
	TROIDS_STATE_GAME_PLAYING,
	TROIDS_STATE_GAME_END,
	TROIDS_STATE_ERROR = -1
} TroidsStates;

typedef enum _TroidsAsteroidTypes {
	TROIDS_ASTEROID_TYPE_BIG = 1,
	TROIDS_ASTEROID_TYPE_MEDIUM,
	TROIDS_ASTEROID_TYPE_SMALL,
	TROIDS_ASTEROID_TYPE_RANDOM = -1
} TroidsAsteroidTypes;

typedef enum _TroidsPowerupTypes {
	TROIDS_POWERUP_TYPE_GUN = 0,
	TROIDS_POWERUP_TYPE_MISSILE,
	TROIDS_POWERUP_TYPE_SHIELD,
	TROIDS_POWERUP_TYPE_LENGTH,
	TROIDS_POWERUP_TYPE_UNKNOWN = -1
} TroidsPowerupTypes;

#define TROIDS_IS_PRODUCTION_BUILD					atrue
//#define TROIDS_IS_PRODUCTION_BUILD					afalse

//#define TROIDS_DRAW_COLLISION_MAP					atrue
#define TROIDS_DRAW_COLLISION_MAP					afalse

#define TROIDS_MAX_ASTEROIDS						16

#define TROIDS_FIGHTER_THRUST						0.005

#define TROIDS_ASTEROIDS_STARTING_THRUST			0.0001

#define TROIDS_MAX_BULLETS							6

#define TROIDS_BULLET_FIRE_RADIUS					176.0

#define TROIDS_MAX_MISSILES							4

#define TROIDS_ANIMATION_TIME						40.0

//#define TROIDS_IS_WEB_ENABLED						atrue
#define TROIDS_IS_WEB_ENABLED						afalse

#define TROIDS_WEB_IP								"71.103.157.98"

#define TROIDS_WEB_BASE_URL							"troids"


// define troids public data types

typedef struct _TroidsFighter {
	aboolean isAlive;
	aboolean isRotating;
	aboolean hasBullets;
	aboolean hasMissiles;
	aboolean hasShields;
	int shieldStrength;
	int respawnsRemaining;
	int asteroidsDestroyed;
	int finalScore;
	double angle;
	double distance;
	double shieldRotation;
	double timestamp;
	double aliveTimestamp;
	double bulletTimestamp;
	double missileTimestamp;
	double powerupTimestamp;
	double currentThrust;
	Sprite2dDirection destination;
	Sprite2dTurn direction;
} TroidsFighter;

typedef struct _TroidsCube {
	double xRotation;
	double yRotation;
	double zRotation;
} TroidsCube;

typedef struct _TroidsAsteroid {
	aboolean isAlive;
	double rotation;
	TroidsAsteroidTypes type;
	Sprite2d sprite;
	Physics2d physics;
	Animation2d animation;
} TroidsAsteroid;

typedef struct _TroidsBullet {
	aboolean isAlive;
	double timestamp;
	Sprite2d sprite;
	Physics2d physics;
} TroidsBullet;

typedef struct _TroidsMissile {
	aboolean isAlive;
	int targetId;
	double rotation;
	double timestamp;
	double physicsTimestamp;
	Sprite2d sprite;
	Physics2d physics;
} TroidsMissile;

typedef struct _TroidsPowerup {
	aboolean isAlive;
	TroidsPowerupTypes type;
	Sprite2d sprite;
	Physics2d physics;
} TroidsPowerup;

typedef struct _TroidsAssets {
	Texture *loadingScreen;
	Texture *splashScreen;
	Texture *instructionScreen;
	Texture *gameOverScreen;
	Texture *cube;
	Texture *background;
	Texture *circleBlue;
	Texture *fighterOutline;
	Texture *fighterOutlineFaded;
	Texture *asteroidBig;
	Texture *asteroidMedium;
	Texture *asteroidSmall;
	Texture *bullet;
	Texture *bulletUpgrade;
	Texture *missile;
	Texture *missileUpgrade;
	Texture *gunRadius;
	Texture *gunTargeted;
	Texture *shield;
	Texture *thrusters;
	Texture *thrustersMedium;
	Texture *thrustersSmall;
	Texture *numbers[10];
	TroidsCube cubeData;
	Sprite2d fighter;
	Physics2d fighterPhysics;
	Animation2d fighterAnimation;
	Animation2d missileAnimation;
	TroidsFighter fighterData;
	TroidsAsteroid asteroids[TROIDS_MAX_ASTEROIDS];
	TroidsBullet bullets[TROIDS_MAX_BULLETS];
	TroidsMissile missiles[TROIDS_MAX_MISSILES];
	TroidsPowerup powerups[TROIDS_POWERUP_TYPE_LENGTH];
} TroidsAssets;

typedef struct _TroidsMouse {
	aboolean isDown;
	int xPosition;
	int yPosition;
} TroidsMouse;

typedef struct _Troids {
	TroidsStates state;
	char *name;
	Engine engine;
	TroidsMouse mouse;
	TroidsAssets assets;
	void *zipContext;
} Troids;


// declare troids public functions

int troids_init(Troids *context);

int troids_free(Troids *context);

int troids_handleMouseEvents(Troids *context, AsgardMouseEvent *mouseEvent);

int troids_render(Troids *context);

int troids_processGamepley(Troids *context);

int troids_ioFunction(void *engineContext, void *gameContext, void *argument);

int troids_renderFunction(void *engineContext, void *gameContext,
		void *argument);

int troids_gameplayFunction(void *engineContext, void *gameContext,
		void *argument);


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _APPS_TROIDS_TROIDS_H

