/*
 * physics2d.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The 2D physics engine, header file.
 *
 * Written by Josh English.
 */


// preprocessor directives

#include "core/core.h"
#include "asgard/asgard.h"
#define _ASGARD_ENGINE_2D_COMPONENT
#include "asgard/engine/2d/physics2d.h"


// declare 2d physics private functions

static int getRoundedDouble(double value);

static int invertDirection(int localDirection);

void calculateMovement(Physics2d *physics, int localDirection,
		double localThrust);

static void updateSpriteSpeed(Physics2d *physics,
		Sprite2dDirection spriteDirection, PhysicsDirection2d physicsDirection);

static void determineAlignedWithAngularVelocity(Physics2d *physics);

static void applyFriction(Physics2d *physics);

static void applyBoostFriction(Physics2d *physics);

static void snapshot(Physics2d *physics);


// define 2d physics private functions

static int getRoundedDouble(double value)
{
	if((value - (double)((int)value)) >= 0.5) {
		value += 1.0;
	}
	return (int)value;
}

static int invertDirection(int localDirection)
{
	int result = 0;

	switch(localDirection) {
		case SPRITE2D_DIRECTION_NORTH:
			result = SPRITE2D_DIRECTION_SOUTH;
			break;

		case SPRITE2D_DIRECTION_NORTH_NORTH_EAST:
			result = SPRITE2D_DIRECTION_SOUTH_SOUTH_WEST;
			break;

		case SPRITE2D_DIRECTION_NORTH_EAST:
			result = SPRITE2D_DIRECTION_SOUTH_WEST;
			break;

		case SPRITE2D_DIRECTION_NORTH_EAST_NORTH:
			result = SPRITE2D_DIRECTION_SOUTH_WEST_SOUTH;
			break;

		case SPRITE2D_DIRECTION_EAST:
			result = SPRITE2D_DIRECTION_WEST;
			break;

		case SPRITE2D_DIRECTION_SOUTH_EAST_SOUTH:
			result = SPRITE2D_DIRECTION_NORTH_WEST_NORTH;
			break;

		case SPRITE2D_DIRECTION_SOUTH_EAST:
			result = SPRITE2D_DIRECTION_NORTH_WEST;
			break;

		case SPRITE2D_DIRECTION_SOUTH_SOUTH_EAST:
			result = SPRITE2D_DIRECTION_NORTH_NORTH_WEST;
			break;

		case SPRITE2D_DIRECTION_SOUTH:
			result = SPRITE2D_DIRECTION_NORTH;
			break;

		case SPRITE2D_DIRECTION_SOUTH_SOUTH_WEST:
			result = SPRITE2D_DIRECTION_NORTH_NORTH_EAST;
			break;

		case SPRITE2D_DIRECTION_SOUTH_WEST:
			result = SPRITE2D_DIRECTION_NORTH_EAST;
			break;

		case SPRITE2D_DIRECTION_SOUTH_WEST_SOUTH:
			result = SPRITE2D_DIRECTION_NORTH_EAST_NORTH;
			break;

		case SPRITE2D_DIRECTION_WEST:
			result = SPRITE2D_DIRECTION_EAST;
			break;

		case SPRITE2D_DIRECTION_NORTH_WEST_NORTH:
			result = SPRITE2D_DIRECTION_SOUTH_EAST_SOUTH;
			break;

		case SPRITE2D_DIRECTION_NORTH_WEST:
			result = SPRITE2D_DIRECTION_SOUTH_EAST;
			break;

		case SPRITE2D_DIRECTION_NORTH_NORTH_WEST:
			result = SPRITE2D_DIRECTION_SOUTH_SOUTH_EAST;
			break;
	}

	return result;
}

void calculateMovement(Physics2d *physics, int localDirection,
		double localThrust)
{
	double xSpeedCalc = 0.0;
	double ySpeedCalc = 0.0;
	double millisPerFrame = 0.0;

	millisPerFrame = (physics->elapsedTime / 1000.0);

	xSpeedCalc = 0.0;
	ySpeedCalc = 0.0;

	switch(localDirection) {
		case SPRITE2D_DIRECTION_NORTH:
			ySpeedCalc -= ((localThrust * 4.0) * millisPerFrame);
			break;

		case SPRITE2D_DIRECTION_NORTH_NORTH_EAST:
			xSpeedCalc += (((localThrust * 2.0) * 0.894454) *
					millisPerFrame);
			ySpeedCalc -= (((localThrust * 4.0) * 0.894454) *
					millisPerFrame);
			break;

		case SPRITE2D_DIRECTION_NORTH_EAST:
			xSpeedCalc += (((localThrust * 4.0) * 0.707106) *
					millisPerFrame);
			ySpeedCalc -= (((localThrust * 4.0) * 0.707106) *
					millisPerFrame);
			break;

		case SPRITE2D_DIRECTION_NORTH_EAST_NORTH:
			xSpeedCalc += (((localThrust * 4.0) * 0.894454) *
					millisPerFrame);
			ySpeedCalc -= (((localThrust * 2.0) * 0.894454) *
					millisPerFrame);
			break;

		case SPRITE2D_DIRECTION_EAST:
			xSpeedCalc += ((localThrust * 4.0) * millisPerFrame);
			break;

		case SPRITE2D_DIRECTION_SOUTH_EAST_SOUTH:
			xSpeedCalc += (((localThrust * 4.0) * 0.894454) *
					millisPerFrame);
			ySpeedCalc += (((localThrust * 2.0) * 0.894454) *
					millisPerFrame);
			break;

		case SPRITE2D_DIRECTION_SOUTH_EAST:
			xSpeedCalc += (((localThrust * 4.0) * 0.707106) *
					millisPerFrame);
			ySpeedCalc += (((localThrust * 4.0) * 0.707106) *
					millisPerFrame);
			break;

		case SPRITE2D_DIRECTION_SOUTH_SOUTH_EAST:
			xSpeedCalc += (((localThrust * 2.0) * 0.894454) *
					millisPerFrame);
			ySpeedCalc += (((localThrust * 4.0) * 0.894454) *
					millisPerFrame);
			break;

		case SPRITE2D_DIRECTION_SOUTH:
			ySpeedCalc += ((localThrust * 4.0) * millisPerFrame);
			break;

		case SPRITE2D_DIRECTION_SOUTH_SOUTH_WEST:
			xSpeedCalc -= (((localThrust * 2.0) * 0.894454) *
					millisPerFrame);
			ySpeedCalc += (((localThrust * 4.0) * 0.894454) *
					millisPerFrame);
			break;

		case SPRITE2D_DIRECTION_SOUTH_WEST:
			xSpeedCalc -= (((localThrust * 4.0) * 0.707106) *
					millisPerFrame);
			ySpeedCalc += (((localThrust * 4.0) * 0.707106) *
					millisPerFrame);
			break;

		case SPRITE2D_DIRECTION_SOUTH_WEST_SOUTH:
			xSpeedCalc -= (((localThrust * 4.0) * 0.894454) *
					millisPerFrame);
			ySpeedCalc += (((localThrust * 2.0) * 0.894454) *
					millisPerFrame);
			break;

		case SPRITE2D_DIRECTION_WEST:
			xSpeedCalc -= ((localThrust * 4.0) * millisPerFrame);
			break;

		case SPRITE2D_DIRECTION_NORTH_WEST_NORTH:
			xSpeedCalc -= (((localThrust * 4.0) * 0.894454) *
					millisPerFrame);
			ySpeedCalc -= (((localThrust * 2.0) * 0.894454) *
					millisPerFrame);
			break;

		case SPRITE2D_DIRECTION_NORTH_WEST:
			xSpeedCalc -= (((localThrust * 4.0) * 0.707106) *
					millisPerFrame);
			ySpeedCalc -= (((localThrust * 4.0) * 0.707106) *
					millisPerFrame);
			break;

		case SPRITE2D_DIRECTION_NORTH_NORTH_WEST:
			xSpeedCalc -= (((localThrust * 2.0) * 0.894454) *
					millisPerFrame);
			ySpeedCalc -= (((localThrust * 4.0) * 0.894454) *
					millisPerFrame);
			break;
	}

	physics->position.xSpeedCalc = xSpeedCalc;
	physics->position.ySpeedCalc = ySpeedCalc;
}

static void updateSpriteSpeed(Physics2d *physics,
		Sprite2dDirection spriteDirection, PhysicsDirection2d physicsDirection)
{
	if(physics->isUsingTurbo) {
		physics->thrustCalc = (physics->thrust + physics->turboThrust);
	}
	else {
		physics->thrustCalc = physics->thrust;
	}

	if(physicsDirection == PHYSICS_DIRECTION_BACKWARD) {
		physics->thrustCalc /= 8.0;
	}

	physics->currentThrust += physics->thrustCalc;

	calculateMovement(physics, spriteDirection, physics->thrustCalc);

	if(physicsDirection == PHYSICS_DIRECTION_BACKWARD) {
		physics->position.xSpeedCalc *= -1.0;
		physics->position.ySpeedCalc *= -1.0;
	}

	physics->position.xSpeed += physics->position.xSpeedCalc;
	physics->position.ySpeed += physics->position.ySpeedCalc;
}

static void determineAlignedWithAngularVelocity(Physics2d *physics)
{
	int localX = 0;
	int localY = 0;
	int localDestX = 0;
	int localDestY = 0;
	int spriteX = 0;
	int spriteY = 0;
	int spriteDestX = 0;
	int spriteDestY = 0;
	double localMagnitudeA = 0.0;
	double localMagnitudeB = 0.0;
	double localAngle = 0.0;
	double spriteMagnitudeA = 0.0;
	double spriteMagnitudeB = 0.0;
	double spriteAngle = 0.0;

	// calculate sprite movement with no angular velocity

	physics->spriteDirection = sprite2d_getDirection(physics->sprite);
	physics->thrustCalc = physics->currentThrust;
	calculateMovement(physics, physics->spriteDirection, physics->thrustCalc);

	// determine coordinates

	localX = physics2d_getX(physics);
	localY = physics2d_getY(physics);
	localDestX = getRoundedDouble(physics->position.x +
			physics->position.xSpeed);
	localDestY = getRoundedDouble(physics->position.y +
			physics->position.ySpeed);
	spriteX = sprite2d_getX(physics->sprite);
	spriteY = sprite2d_getY(physics->sprite);
	spriteDestX = getRoundedDouble((double)spriteX +
			physics->position.xSpeedCalc);
	spriteDestY = getRoundedDouble((double)spriteY +
			physics->position.ySpeedCalc);

	// calcluate the local angle from the magnitude of the local vectors

	localMagnitudeA = sqrt((double)(localX * localX) +
			(double)(localY * localY));
	localMagnitudeB = sqrt((double)(localDestX * localDestX) +
			(double)(localDestY * localDestY));
	localAngle = acos(((double)(localX * localDestX) +
			(double)(localY * localDestY)) /
			(localMagnitudeA * localMagnitudeB));

	// calcluate the sprite angle from the magnitude of the sprite vectors

	spriteMagnitudeA = sqrt((double)(spriteX * spriteX) +
			(double)(spriteY * spriteY));
	spriteMagnitudeB = sqrt((double)(spriteDestX * spriteDestX) +
			(double)(spriteDestY * spriteDestY));

	spriteAngle = acos(((double)(spriteX * spriteDestX) +
			(double)(spriteY * spriteDestY)) /
			(spriteMagnitudeA * spriteMagnitudeB));

	// determine if has angular velocity

	if(fabs(localAngle - spriteAngle) > 0.001) {
		physics->isSliding = atrue;
	}
	else {
		physics->isSliding = afalse;
	}
}

static void applyFriction(Physics2d *physics)
{
	physics->position.xSpeed *= (physics->friction * physics->frictionModifier);
	physics->position.ySpeed *= (physics->friction * physics->frictionModifier);
	physics->currentThrust *= (physics->friction * physics->frictionModifier);
}

static void applyBoostFriction(Physics2d *physics)
{
	physics->position.xBoost *= (physics->friction * physics->frictionModifier);
	physics->position.yBoost *= (physics->friction * physics->frictionModifier);
}

static void snapshot(Physics2d *physics)
{
	double timeMus = 0;

	timeMus = time_getTimeMus();

	physics->elapsedTime = (timeMus - physics->currentTime);

	physics->currentTime = timeMus;
}


// define 2d physics public functions

void physics2d_init(Physics2d *physics, Sprite2d *sprite)
{
	Position2d *position = NULL;

	if((sprite == NULL) || (physics == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	memset(physics, 0, (sizeof(Physics2d)));

	physics->isUsingTurbo = afalse;
	physics->isSliding = afalse;
	physics->hasFriction = atrue;
	physics->hasGravity = afalse;
	physics->willBounce = afalse;

	physics->spriteDirection = sprite2d_getDirection(sprite);
	physics->gravityDirection = SPRITE2D_DIRECTION_SOUTH;

	physics->thrust = PHYSICS_DEFAULT_THRUST;
	physics->thrustCalc = 0.0;
	physics->turboThrust = PHYSICS_DEFAULT_TURBO_THRUST;
	physics->currentThrust = PHYSICS_DEFAULT_THRUST;
	physics->gravity = PHYSICS_DEFAULT_GRAVITY;
	physics->friction = PHYSICS_DEFAULT_FRICTION;
	physics->frictionModifier = 1.0;
	physics->elapsedTime = 0.0;
	physics->currentTime = time_getTimeMus();
	physics->rotateTimeSeconds = sprite2d_getRotateTimeSeconds(sprite);

	position = sprite2d_getPosition(sprite);

	physics->position.x = (double)position->x;
	physics->position.y = (double)position->y;
	physics->position.xSpeed = 0.0;
	physics->position.ySpeed = 0.0;
	physics->position.xBoost = 0.0;
	physics->position.yBoost = 0.0;
	physics->position.xSpeedCalc = 0.0;
	physics->position.ySpeedCalc = 0.0;

	physics->sprite = sprite;
}

aboolean physics2d_isUsingTurbo(Physics2d *physics)
{
	if(physics == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return physics->isUsingTurbo;
}

void physics2d_setIsUsingTurbo(Physics2d *physics, aboolean isUsingTurbo)
{
	if(physics == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	physics->isUsingTurbo = isUsingTurbo;
}

aboolean physics2d_isSliding(Physics2d *physics)
{
	if(physics == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return physics->isSliding;
}

aboolean physics2d_hasFriction(Physics2d *physics)
{
	if(physics == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return physics->hasFriction;
}

void physics2d_setHasFriction(Physics2d *physics, aboolean hasFriction)
{
	if(physics == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	physics->hasFriction = hasFriction;
}

aboolean physics2d_hasGravity(Physics2d *physics)
{
	if(physics == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return physics->hasGravity;
}

void physics2d_setHasGravity(Physics2d *physics, aboolean hasGravity)
{
	if(physics == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	physics->hasGravity = hasGravity;
}

aboolean physics2d_willBounce(Physics2d *physics)
{
	if(physics == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return physics->willBounce;
}

void physics2d_setWillBounce(Physics2d *physics, aboolean willBounce)
{
	if(physics == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	physics->willBounce = willBounce;
}

void physics2d_setPosition(Physics2d *physics, int x, int y)
{
	if(physics == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	physics->position.x = (double)x;
	physics->position.y = (double)y;

	sprite2d_setPosition(physics->sprite, x, y);
}

int physics2d_getX(Physics2d *physics)
{
	if(physics == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return getRoundedDouble(physics->position.x);
}

int physics2d_getY(Physics2d *physics)
{
	if(physics == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return getRoundedDouble(physics->position.y);
}

double physics2d_getXSpeed(Physics2d *physics)
{
	if(physics == NULL) {
		DISPLAY_INVALID_ARGS;
		return 0.0;
	}

	return physics->position.xSpeed;
}

void physics2d_setXSpeed(Physics2d *physics, double xSpeed)
{
	if(physics == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	physics->position.xSpeed = xSpeed;
}

double physics2d_getYSpeed(Physics2d *physics)
{
	if(physics == NULL) {
		DISPLAY_INVALID_ARGS;
		return 0.0;
	}

	return physics->position.ySpeed;
}

void physics2d_setYSpeed(Physics2d *physics, double ySpeed)
{
	if(physics == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	physics->position.ySpeed = ySpeed;
}

double physics2d_getThrust(Physics2d *physics)
{
	if(physics == NULL) {
		DISPLAY_INVALID_ARGS;
		return 0.0;
	}

	return physics->thrust;
}

void physics2d_setThrust(Physics2d *physics, double thrust)
{
	if(physics == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	physics->thrust = thrust;
}

double physics2d_getCurrentThrust(Physics2d *physics)
{
	if(physics == NULL) {
		DISPLAY_INVALID_ARGS;
		return 0.0;
	}

	return physics->currentThrust;
}

void physics2d_setCurrentThrust(Physics2d *physics, double currentThrust)
{
	if(physics == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	physics->currentThrust = currentThrust;
}

void physics2d_setMovementProperties(Physics2d *physics, double xSpeed,
		double ySpeed, double currentThrust)
{
	if(physics == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	physics->position.xSpeed = xSpeed;
	physics->position.ySpeed = ySpeed;
	physics->currentThrust = currentThrust;
}

void physics2d_reset(Physics2d *physics)
{
	Position2d *position = NULL;

	if(physics == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	physics->isUsingTurbo = afalse;
	physics->isSliding = afalse;

	physics->spriteDirection = sprite2d_getDirection(physics->sprite);

	physics->thrustCalc = 0.0;
	physics->currentThrust = PHYSICS_DEFAULT_THRUST;
	physics->elapsedTime = 0.0;
	physics->currentTime = time_getTimeMus();
	physics->rotateTimeSeconds = sprite2d_getRotateTimeSeconds(physics->sprite);

	position = sprite2d_getPosition(physics->sprite);

	physics->position.x = (double)position->x;
	physics->position.y = (double)position->y;
	physics->position.xSpeed = 0.0;
	physics->position.ySpeed = 0.0;
	physics->position.xBoost = 0.0;
	physics->position.yBoost = 0.0;
	physics->position.xSpeedCalc = 0.0;
	physics->position.ySpeedCalc = 0.0;
}

double physics2d_getFriction(Physics2d *physics)
{
	if(physics == NULL) {
		DISPLAY_INVALID_ARGS;
		return 0.0;
	}

	return physics->friction;
}

void physics2d_setFriction(Physics2d *physics, double friction)
{
	if(physics == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	physics->friction = friction;
}

double physics2d_getFrictionModifier(Physics2d *physics)
{
	if(physics == NULL) {
		DISPLAY_INVALID_ARGS;
		return 0.0;
	}

	return physics->frictionModifier;
}

void physics2d_setFrictionModifier(Physics2d *physics, double frictionModifier)
{
	if(physics == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	physics->frictionModifier = frictionModifier;
}

double physics2d_getGravity(Physics2d *physics)
{
	if(physics == NULL) {
		DISPLAY_INVALID_ARGS;
		return 0.0;
	}

	return physics->gravity;
}

void physics2d_setGravity(Physics2d *physics, double gravity)
{
	if(physics == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	physics->gravity = gravity;
}

int physics2d_getGravityDirection(Physics2d *physics)
{
	if(physics == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return physics->gravityDirection;
}

void physics2d_setGravityDirection(Physics2d *physics, int gravityDirection)
{
	if(physics == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	physics->gravityDirection = gravityDirection;
}

double physics2d_getRate(Physics2d *physics)
{
	double result = 0.0;

	if(physics == NULL) {
		DISPLAY_INVALID_ARGS;
		return 0.0;
	}

	result = sqrt((physics->position.xSpeed * physics->position.xSpeed) +
			(physics->position.ySpeed * physics->position.ySpeed));

	return result;
}

double physics2d_getPixelRate(Physics2d *physics)
{
	double result = 0.0;

	if(physics == NULL) {
		DISPLAY_INVALID_ARGS;
		return 0.0;
	}

	if(physics->position.xSpeed == 0.0) {
		result = fabs(physics->position.ySpeed);
	}
	else if(physics->position.ySpeed == 0.0) {
		result = fabs(physics->position.xSpeed);
	}
	else {
		result = (fabs(physics->position.xSpeed) *
				fabs(physics->position.ySpeed));
	}

	return result;
}

void physics2d_simpleBounce(Physics2d *physics, aboolean xAxis, aboolean yAxis)
{
	if(physics == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	if(xAxis) {
		physics->position.xSpeed *= -1.0;
	}

	if(yAxis) {
		physics->position.ySpeed *= -1.0;
	}

	physics->spriteDirection = invertDirection(
			sprite2d_getDirection(physics->sprite));

	sprite2d_resetDirection(physics->sprite, physics->spriteDirection);
}

void physics2d_antiGravityBounce(Physics2d *physics, aboolean xAxis,
		aboolean yAxis, double simulatedSeconds)
{
	int localDirection = 0;

	if(physics == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	physics->elapsedTime = (simulatedSeconds * 1000000.0);

	localDirection = invertDirection(physics->gravityDirection);

	calculateMovement(physics, localDirection, physics->gravity);

	if(physics->hasFriction) {
		physics->position.xSpeed += physics->position.xSpeedCalc;
		physics->position.ySpeed += physics->position.ySpeedCalc;
	}
	else {
		physics->position.xBoost += physics->position.xSpeedCalc;
		physics->position.yBoost += physics->position.ySpeedCalc;
	}

	sprite2d_setPosition(physics->sprite,
			getRoundedDouble(physics->position.x),
			getRoundedDouble(physics->position.y));

	physics->spriteDirection = invertDirection(
			sprite2d_getDirection(physics->sprite));

	sprite2d_resetDirection(physics->sprite, physics->spriteDirection);
}

void physics2d_processCurrentPhysicsOnTime(Physics2d *physics,
		double simulatedSeconds)
{
	double millisPerFrame = 0.0;
	double calculation = 0.0;

	if(physics == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	if(simulatedSeconds < 1.0) {
		return;
	}

	physics->elapsedTime = (simulatedSeconds * 1000000.0);

	millisPerFrame = (physics->elapsedTime / 1000.0);

	calculation = (physics->currentThrust * millisPerFrame);

	physics->currentThrust *= calculation;
	physics->position.xSpeed *= calculation;
	physics->position.ySpeed *= calculation;
}

void physics2d_processPhysics(Physics2d *physics, PhysicsDirection2d direction)
{
	if(physics == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	snapshot(physics);

	physics->spriteDirection = sprite2d_getDirection(physics->sprite);

	updateSpriteSpeed(physics, physics->spriteDirection, direction);

	determineAlignedWithAngularVelocity(physics);

	if(!physics->hasFriction) {
		applyFriction(physics); // normalize the speed
	}

	// apply gravity

	if(physics->hasGravity) {
		calculateMovement(physics, physics->gravityDirection,
				physics->gravity);
		physics->position.x += physics->position.xSpeedCalc;
		physics->position.y += physics->position.ySpeedCalc;
	}

	sprite2d_setPosition(physics->sprite,
			getRoundedDouble(physics->position.x),
			getRoundedDouble(physics->position.y));
}

void physics2d_processDirectionalPhysics(Physics2d *physics,
		PhysicsDirection2d direction, aboolean useGravity,
		double simulatedSeconds)
{
	if(physics == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	if(simulatedSeconds < 0.001) {
		return;
	}

	physics->elapsedTime = (simulatedSeconds * REAL_ONE_MILLION);

	physics->spriteDirection = sprite2d_getDirection(physics->sprite);

	updateSpriteSpeed(physics, physics->spriteDirection, direction);

	determineAlignedWithAngularVelocity(physics);

	if(!physics->hasFriction) {
		applyFriction(physics); // normalize the speed
	}

	// apply gravity

	if((useGravity) && (physics->hasGravity)) {
		calculateMovement(physics, physics->gravityDirection,
				physics->gravity);
		physics->position.x += physics->position.xSpeedCalc;
		physics->position.y += physics->position.ySpeedCalc;
	}

	sprite2d_setPosition(physics->sprite,
			getRoundedDouble(physics->position.x),
			getRoundedDouble(physics->position.y));
}

void physics2d_processGameplay(Physics2d *physics)
{
	double pixelRate = 0.0;
	double ratio = 0.0;
	double average = 0.0;
	double rotateTimeSeconds = 0.0;

	if(physics == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	// apply friction to speed & update location

	snapshot(physics);

	if(physics->hasFriction) {
		applyFriction(physics);
	}

	applyBoostFriction(physics);

	if(fabs(physics->position.xSpeed) < 0.01) {
		physics->position.xSpeed = 0.0;
	}
	if(fabs(physics->position.ySpeed) < 0.01) {
		physics->position.ySpeed = 0.0;
	}
	if(fabs(physics->position.xBoost) < 0.01) {
		physics->position.xBoost = 0.0;
	}
	if(fabs(physics->position.yBoost) < 0.01) {
		physics->position.yBoost = 0.0;
	}
	if(fabs(physics->currentThrust) < 0.01) {
		physics->currentThrust = 0.0;
	}
	if((physics->position.xSpeed == 0.0) && (physics->position.ySpeed == 0.0)) {
		physics->isSliding = afalse;
	}

	physics->position.x += physics->position.xSpeed;
	physics->position.y += physics->position.ySpeed;
	physics->position.x += physics->position.xBoost;
	physics->position.y += physics->position.yBoost;

	// apply gravity

	if(physics->hasGravity) {
		calculateMovement(physics, physics->gravityDirection, physics->gravity);
		physics->position.x += physics->position.xSpeedCalc;
		physics->position.y += physics->position.ySpeedCalc;
	}

	// update sprite location

	sprite2d_setPosition(physics->sprite,
			getRoundedDouble(physics->position.x),
			getRoundedDouble(physics->position.y));

	// normalize sprite rotation

	pixelRate = physics2d_getPixelRate(physics);
	if(pixelRate > physics->highestPixelRate) {
		physics->highestPixelRate = pixelRate;
	}
	if(pixelRate < physics->lowestPixelRate) {
		physics->lowestPixelRate = pixelRate;
	}

	if((physics->highestPixelRate - physics->lowestPixelRate) > 200.0) {
		ratio = (pixelRate /
				(physics->highestPixelRate + physics->lowestPixelRate));

		if(((ratio * 100.0) < 0.1) || ((ratio * 100.0) > 99.9)) {
			sprite2d_setRotateTimeSeconds(physics->sprite,
					physics->defaultRotateTimeSeconds);
			return;
		}

		average = ((physics->highestPixelRate + physics->lowestPixelRate) /
				2.0);

		ratio = (fabs(pixelRate - average) / average);

		rotateTimeSeconds = (physics->defaultRotateTimeSeconds +
				(physics->defaultRotateTimeSeconds * ratio));

		sprite2d_setRotateTimeSeconds(physics->sprite,
				physics->rotateTimeSeconds);
	}
}

