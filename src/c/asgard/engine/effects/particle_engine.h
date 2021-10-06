/*
 * particle_engine.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard 2D & 3D particle effects engine, header file.
 *
 * Written by Josh English.
 */

#if !defined(_ASGARD_ENGINE_EFFECTS_PARTICLE_ENGINE_H)

#define _ASGARD_ENGINE_EFFECTS_PARTICLE_ENGINE_H

#if !defined(_ASGARD_H) && !defined(_ASGARD_ENGINE_EFFECTS_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _ASGARD_H || _ASGARD_ENGINE_EFFECTS_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// define particle engine constants

typedef enum _ParticleEngineTypes {
	PARTICLE_ENGINE_TYPE_POINT = 1,
	PARTICLE_ENGINE_TYPE_TRIANGLE_STRIP,
	PARTICLE_ENGINE_TYPE_OBJECT,
	PARTICLE_ENGINE_TYPE_UNKNOWN = -1
} ParticleEngineTypes;


// define particle engine data types

typedef void (*ParticleEngineInitFunction)(void *context, int id);

typedef void (*ParticleEnginePhysicsFunction)(void *context, int id,
		double elapsedTime);

typedef struct _ParticleGravity {
	double x;
	double y;
	double z;
} ParticleGravity;

typedef struct _Particle {
	aboolean isActive;
	int colorMaskMode;
	double life;
	double fadeRate;
	Point3d position;
	Vertex3d direction;
	Color color;
} Particle;

typedef struct _ParticleEngine {
	ParticleEngineTypes type;
	int particleLength;
	double timestamp;
	double deceleration;
	double renderRatio;
	double renderVertexLength;
	Point3d position;
	Vertex3d rotation;
	ParticleGravity gravity;
	Particle *particles;
	ParticleEngineInitFunction initFunction;
	ParticleEnginePhysicsFunction physicsFunction;
	Texture *texture;
	Draw3dObject *object;
	void *context;
} ParticleEngine;


// declare particle engine public functions

void particleEngine_init(ParticleEngine *engine);

ParticleEngine *particleEngine_new();

void particleEngine_free(ParticleEngine *engine);

void particleEngine_freePtr(ParticleEngine *engine);

void particleEngine_setDeceleration(ParticleEngine *engine,
		double deceleration);

void particleEngine_setRenderRatio(ParticleEngine *engine, double renderRatio,
		double renderVertexLength);

void particleEngine_setPosition(ParticleEngine *engine, Point3d *point);

void particleEngine_setPositionCoords(ParticleEngine *engine, double x,
		double y, double z);

void particleEngine_setRotation(ParticleEngine *engine, Vertex3d *point);

void particleEngine_setRotationCoords(ParticleEngine *engine, double x,
		double y, double z);

void particleEngine_setGravity(ParticleEngine *engine, ParticleGravity *point);

void particleEngine_setGravityDirectly(ParticleEngine *engine, double x,
		double y, double z);

void particleEngine_setParticleColor(ParticleEngine *engine, Color *color,
		int colorMaskMode);

void particleEngine_setEffect(ParticleEngine *engine,
		ParticleEngineInitFunction initFunction,
		ParticleEnginePhysicsFunction physicsFunction,
		void *context);

void particleEngine_allocate(ParticleEngine *engine, int length);

void particleEngine_setTexture(ParticleEngine *engine, Texture *texture);

void particleEngine_reset(ParticleEngine *engine);

void particleEngine_draw(ParticleEngine *engine, Canvas *canvas);

void particleEngine_resetPhysics(ParticleEngine *engine);

void particleEngine_processPhysics(ParticleEngine *engine);


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _ASGARD_ENGINE_EFFECTS_PARTICLE_ENGINE_H

