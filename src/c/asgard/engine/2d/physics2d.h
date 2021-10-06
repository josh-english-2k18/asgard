/*
 * physics2d.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The 2D physics engine, header file.
 *
 * Written by Josh English.
 */

#if !defined(_ASGARD_ENGINE_2D_PHYSICS2D_H)

#define _ASGARD_ENGINE_2D_PHYSICS2D_H

#if !defined(_ASGARD_H) && !defined(_ASGARD_ENGINE_2D_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _ASGARD_H || _ASGARD_ENGINE_2D_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// define 2d physics public constants

typedef enum _PhysicsDirection2d {
	PHYSICS_DIRECTION_FORWARD = 1,
	PHYSICS_DIRECTION_BACKWARD,
	PHYSICS_DIRECTION_UNKNOWN = -1
} PhysicsDirection2d;

#define PHYSICS_DEFAULT_THRUST					0.105

#define PHYSICS_DEFAULT_TURBO_THRUST			0.105

#define PHYSICS_DEFAULT_FRICTION				0.65

#define PHYSICS_DEFAULT_GRAVITY					0.096


// define 2d physics public data types

typedef struct _PhysicsPosition2d {
	double x;
	double y;
	double xSpeed;
	double ySpeed;
	double xBoost;
	double yBoost;
	double xSpeedCalc;
	double ySpeedCalc;
} PhysicsPosition2d;

typedef struct _Physics2d {
	aboolean isUsingTurbo;
	aboolean isSliding;
	aboolean hasFriction;
	aboolean hasGravity;
	aboolean willBounce;
	int spriteDirection;
	int gravityDirection;
	double thrust;
	double thrustCalc;
	double turboThrust;
	double currentThrust;
	double gravity;
	double friction;
	double frictionModifier;
	double elapsedTime;
	double currentTime;
	double rotateTimeSeconds;
	double highestPixelRate;
	double lowestPixelRate;
	double defaultRotateTimeSeconds;
	PhysicsPosition2d position;
	Sprite2d *sprite;
} Physics2d;


// declare 2d physics public functions

void physics2d_init(Physics2d *physics, Sprite2d *sprite);

aboolean physics2d_isUsingTurbo(Physics2d *physics);

void physics2d_setIsUsingTurbo(Physics2d *physics, aboolean isUsingTurbo);

aboolean physics2d_isSliding(Physics2d *physics);

aboolean physics2d_hasFriction(Physics2d *physics);

void physics2d_setHasFriction(Physics2d *physics, aboolean hasFriction);

aboolean physics2d_hasGravity(Physics2d *physics);

void physics2d_setHasGravity(Physics2d *physics, aboolean hasGravity);

aboolean physics2d_willBounce(Physics2d *physics);

void physics2d_setWillBounce(Physics2d *physics, aboolean willBounce);

void physics2d_setPosition(Physics2d *physics, int x, int y);

int physics2d_getX(Physics2d *physics);

int physics2d_getY(Physics2d *physics);

double physics2d_getXSpeed(Physics2d *physics);

void physics2d_setXSpeed(Physics2d *physics, double xSpeed);

double physics2d_getYSpeed(Physics2d *physics);

void physics2d_setYSpeed(Physics2d *physics, double ySpeed);

double physics2d_getThrust(Physics2d *physics);

void physics2d_setThrust(Physics2d *physics, double thrust);

double physics2d_getCurrentThrust(Physics2d *physics);

void physics2d_setCurrentThrust(Physics2d *physics, double currentThrust);

void physics2d_setMovementProperties(Physics2d *physics, double xSpeed,
		double ySpeed, double currentThrust);

void physics2d_reset(Physics2d *physics);

double physics2d_getFriction(Physics2d *physics);

void physics2d_setFriction(Physics2d *physics, double friction);

double physics2d_getFrictionModifier(Physics2d *physics);

void physics2d_setFrictionModifier(Physics2d *physics, double frictionModifier);

double physics2d_getGravity(Physics2d *physics);

void physics2d_setGravity(Physics2d *physics, double gravity);

int physics2d_getGravityDirection(Physics2d *physics);

void physics2d_setGravityDirection(Physics2d *physics, int gravityDirection);

double physics2d_getRate(Physics2d *physics);

double physics2d_getPixelRate(Physics2d *physics);

void physics2d_simpleBounce(Physics2d *physics, aboolean xAxis, aboolean yAxis);

void physics2d_antiGravityBounce(Physics2d *physics, aboolean xAxis,
		aboolean yAxis, double simulatedSeconds);

void physics2d_processCurrentPhysicsOnTime(Physics2d *physics,
		double simulatedSeconds);

void physics2d_processPhysics(Physics2d *physics, PhysicsDirection2d direction);

void physics2d_processDirectionalPhysics(Physics2d *physics,
		PhysicsDirection2d direction, aboolean useGravity,
		double simulatedSeconds);

void physics2d_processGameplay(Physics2d *physics);


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _ASGARD_ENGINE_2D_PHYSICS2D_H

