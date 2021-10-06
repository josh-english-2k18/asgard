/*
 * particle_engine.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard 2D & 3D particle effects engine.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#include "asgard/asgard.h"
#define _ASGARD_ENGINE_EFFECTS_COMPONENT
#include "asgard/engine/effects/particle_engine.h"


// define particle engine private functions

static void defaultInitFunction(void *context, int id);

static void defaultPhysicsFunction(void *context, int id, double elapsedTime);

static void updateParticlePosition(ParticleEngine *engine);


// define particle engine private functions

static void defaultInitFunction(void *context, int id)
{
	ParticleEngine *engine = NULL;

	if(context == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	engine = (ParticleEngine *)context;

	if((id < 0) ||
			(id >= engine->particleLength) ||
			(engine->particleLength < 1) ||
			(engine->particles == NULL)) {
		return;
	}

	system_pickRandomSeed();

	engine->particles[id].direction.x = (double)(rand() % 256);

	if((rand() % 2) == 0) {
		engine->particles[id].direction.x *= -1;
	}

	engine->particles[id].direction.y = (double)(rand() % 256);

	if((rand() % 2) == 0) {
		engine->particles[id].direction.y *= -1;
	}

	engine->particles[id].direction.z = (double)(rand() % 256);

	if((rand() % 2) == 0) {
		engine->particles[id].direction.z *= -1;
	}

	engine->particles[id].color.r = CGI_COLOR_BLUE_RED;
	engine->particles[id].color.g = CGI_COLOR_BLUE_GREEN;
	engine->particles[id].color.b = CGI_COLOR_BLUE_BLUE;
	engine->particles[id].color.a = CGI_COLOR_BLUE_ALPHA;

	engine->particles[id].colorMaskMode = TEXTURE_COLORMASK_MODE_COMBINE;
}

static void defaultPhysicsFunction(void *context, int id, double elapsedTime)
{
	ParticleEngine *engine = NULL;

	if(context == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	engine = (ParticleEngine *)context;

	if((id < 0) ||
			(id >= engine->particleLength) ||
			(engine->particleLength < 1) ||
			(engine->particles == NULL)) {
		return;
	}

	engine->particles[id].position.x += ((engine->particles[id].direction.x /
				engine->deceleration) * elapsedTime);
	engine->particles[id].position.y += ((engine->particles[id].direction.y /
				engine->deceleration) * elapsedTime);
	engine->particles[id].position.z += ((engine->particles[id].direction.z /
				engine->deceleration) * elapsedTime);
}

static void updateParticlePosition(ParticleEngine *engine)
{
	int ii = 0;

	if((engine->particleLength < 1) || (engine->particles == NULL)) {
		return;
	}

	for(ii = 0; ii < engine->particleLength; ii++) {
		engine->particles[ii].position.x = engine->position.x;
		engine->particles[ii].position.y = engine->position.y;
		engine->particles[ii].position.z = engine->position.z;
	}
}


// define particle engine public functions

void particleEngine_init(ParticleEngine *engine)
{
	if(engine == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	memset(engine, 0, (int)(sizeof(ParticleEngine)));

	engine->type = PARTICLE_ENGINE_TYPE_POINT;

	engine->particleLength = 0;
	engine->timestamp = 0.0;
	engine->deceleration = 0.0;
	engine->renderRatio = 1.0;
	engine->renderVertexLength = 0.5;

	engine->position.x = 0.0;
	engine->position.y = 0.0;
	engine->position.z = 0.0;

	engine->rotation.x = 0.0;
	engine->rotation.y = 0.0;
	engine->rotation.z = 0.0;

	engine->gravity.x = 0.0;
	engine->gravity.y = 0.0;
	engine->gravity.z = 0.0;

	engine->particles = NULL;

	engine->initFunction = defaultInitFunction;
	engine->physicsFunction = defaultPhysicsFunction;

	engine->texture = NULL;

	engine->object = NULL;

	engine->context = NULL;
}

ParticleEngine *particleEngine_new()
{
	ParticleEngine *result = NULL;

	result = (ParticleEngine *)malloc(sizeof(ParticleEngine));

	particleEngine_init(result);

	return result;
}

void particleEngine_free(ParticleEngine *engine)
{
	int ii = 0;

	if(engine == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	if(engine->particles != NULL) {
		free(engine->particles);
	}

	if(engine->texture != NULL) {
		texture_freePtr(engine->texture);
	}

	if(engine->object != NULL) {
		draw3d_freeObject(engine->object);
	}

	memset(engine, 0, (int)(sizeof(ParticleEngine)));
}

void particleEngine_freePtr(ParticleEngine *engine)
{
	if(engine == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	particleEngine_free(engine);
	free(engine);
}

void particleEngine_setDeceleration(ParticleEngine *engine, double deceleration)
{
	if(engine == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	engine->deceleration = deceleration;
}

void particleEngine_setRenderRatio(ParticleEngine *engine, double renderRatio,
		double renderVertexLength)
{
	if(engine == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	engine->renderRatio = renderRatio;
	engine->renderVertexLength = renderVertexLength;
}

void particleEngine_setPosition(ParticleEngine *engine, Point3d *point)
{
	if(engine == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	engine->position.x = point->x;
	engine->position.y = point->y;
	engine->position.z = point->z;

	updateParticlePosition(engine);
}

void particleEngine_setPositionCoords(ParticleEngine *engine, double x,
		double y, double z)
{
	if(engine == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	engine->position.x = x;
	engine->position.y = y;
	engine->position.z = z;

	updateParticlePosition(engine);
}

void particleEngine_setRotation(ParticleEngine *engine, Vertex3d *point)
{
	if(engine == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	engine->rotation.x = point->x;
	engine->rotation.y = point->y;
	engine->rotation.z = point->z;
}

void particleEngine_setRotationCoords(ParticleEngine *engine, double x,
		double y, double z)
{
	if(engine == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	engine->rotation.x = x;
	engine->rotation.y = y;
	engine->rotation.z = z;
}

void particleEngine_setGravity(ParticleEngine *engine, ParticleGravity *point)
{
	if(engine == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	engine->gravity.x = point->x;
	engine->gravity.y = point->y;
	engine->gravity.z = point->z;
}

void particleEngine_setGravityDirectly(ParticleEngine *engine, double x,
		double y, double z)
{
	if(engine == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	engine->gravity.x = x;
	engine->gravity.y = y;
	engine->gravity.z = z;
}

void particleEngine_setParticleColor(ParticleEngine *engine, Color *color,
		int colorMaskMode)
{
	int ii = 0;

	if((engine == NULL) || (color == NULL) ||
			(colorMaskMode < TEXTURE_COLORMASK_MODE_ADD) ||
			(colorMaskMode > TEXTURE_COLORMASK_MODE_COMBINE)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	if((engine->particleLength < 1) || (engine->particles == NULL)) {
		return;
	}

	for(ii = 0; ii < engine->particleLength; ii++) {
		engine->particles[ii].color.r = color->r;
		engine->particles[ii].color.g = color->g;
		engine->particles[ii].color.b = color->b;
		engine->particles[ii].color.a = color->a;

		engine->particles[ii].colorMaskMode = colorMaskMode;
	}
}

void particleEngine_setEffect(ParticleEngine *engine,
		ParticleEngineInitFunction initFunction,
		ParticleEnginePhysicsFunction physicsFunction,
		void *context)
{
	if((engine == NULL) || (initFunction == NULL) ||
			(physicsFunction == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	engine->initFunction = initFunction;
	engine->physicsFunction = physicsFunction;
	engine->context = context;
}

void particleEngine_allocate(ParticleEngine *engine, int length)
{
	int ii = 0;

	if((engine == NULL) || (length < 1)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	if(engine->particles != NULL) {
		free(engine->particles);
	}

	engine->particleLength = length;
	engine->particles = (Particle *)malloc(sizeof(Particle) *
			engine->particleLength);

	system_pickRandomSeed();

	for(ii = 0; ii < engine->particleLength; ii++) {
		engine->particles[ii].isActive = atrue;
		engine->particles[ii].life = 1.0;
		engine->particles[ii].fadeRate = 0.001;

		engine->initFunction(engine, ii);
	}

	engine->timestamp = time_getTimeMus();
}

void particleEngine_setTexture(ParticleEngine *engine, Texture *texture)
{
	if((engine == NULL) || (texture == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	engine->type = PARTICLE_ENGINE_TYPE_TRIANGLE_STRIP;
	engine->texture = texture;
}

void particleEngine_reset(ParticleEngine *engine)
{
	int ii = 0;

	if(engine == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	if((engine->particleLength < 1) || (engine->particles == NULL)) {
		return;
	}

	for(ii = 0; ii < engine->particleLength; ii++) {
		engine->particles[ii].isActive = atrue;
		engine->particles[ii].life = 1.0;

		engine->particles[ii].position.x = engine->position.x;
		engine->particles[ii].position.y = engine->position.y;
		engine->particles[ii].position.z = engine->position.z;

		engine->particles[ii].color.a = 1.0;
	}

	if(engine->texture != NULL) {
		engine->texture->colorMask.a = 1.0;
	}

	engine->timestamp = time_getTimeMus();
}

void particleEngine_draw(ParticleEngine *engine, Canvas *canvas)
{
	aboolean isBlendEnabled = afalse;
	aboolean isDepthTestEnabled = afalse;
	int ii = 0;

	if((engine == NULL) || (canvas == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	if((engine->particleLength < 1) || (engine->particles == NULL)) {
		return;
	}

	// turn off depth testing

	if(cgi_glIsEnabled(GL_DEPTH_TEST)) {
		isDepthTestEnabled = atrue;
		cgi_glDisable(GL_DEPTH_TEST);
	}

	// setup the viewable matrix

	cgi_glPushMatrix();

	cgi_glTranslated(engine->position.x, engine->position.y,
			engine->position.z);

	cgi_glRotated(engine->rotation.x, 1.0, 0.0, 0.0);
	cgi_glRotated(engine->rotation.y, 0.0, 1.0, 0.0);
	cgi_glRotated(engine->rotation.z, 0.0, 0.0, 1.0);

	if(engine->texture != NULL) {
		texture_apply(engine->texture);
	}

	for(ii = 0; ii < engine->particleLength; ii++) {
		if(!engine->particles[ii].isActive) {
			continue;
		}

		// render particle

		switch(engine->type) {
			case PARTICLE_ENGINE_TYPE_POINT:
				engine->particles[ii].color.a = engine->particles[ii].life;
				draw2d_color(canvas, &engine->particles[ii].color);

				cgi_glBegin(GL_POINTS);

				cgi_glVertex3d(
						(engine->particles[ii].position.x *
						 engine->renderRatio),
						(engine->particles[ii].position.y *
						 engine->renderRatio),
						(engine->particles[ii].position.z *
						 engine->renderRatio));

				cgi_glEnd();
				break;

			case PARTICLE_ENGINE_TYPE_TRIANGLE_STRIP:
				if(engine->texture != NULL) {
					cgi_glColor4d(engine->particles[ii].color.r,
							engine->particles[ii].color.g,
							engine->particles[ii].color.b,
							engine->particles[ii].color.a);

					cgi_glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE,
							engine->particles[ii].colorMaskMode);
				}

				cgi_glBegin(GL_TRIANGLE_STRIP);

				cgi_glTexCoord2d(1, 1);

				cgi_glVertex3d(
						((engine->particles[ii].position.x +
						  engine->renderVertexLength) * engine->renderRatio),
						((engine->particles[ii].position.y +
						  engine->renderVertexLength) * engine->renderRatio),
						(engine->particles[ii].position.z *
						 engine->renderRatio));

				cgi_glTexCoord2d(0, 1);

				cgi_glVertex3d(
						((engine->particles[ii].position.x -
						  engine->renderVertexLength) * engine->renderRatio),
						((engine->particles[ii].position.y +
						  engine->renderVertexLength) * engine->renderRatio),
						(engine->particles[ii].position.z *
						 engine->renderRatio));

				cgi_glTexCoord2d(1, 0);

				cgi_glVertex3d(
						((engine->particles[ii].position.x +
						  engine->renderVertexLength) * engine->renderRatio),
						((engine->particles[ii].position.y -
						  engine->renderVertexLength) * engine->renderRatio),
						(engine->particles[ii].position.z *
						 engine->renderRatio));

				cgi_glTexCoord2d(0, 0);

				cgi_glVertex3d(
						((engine->particles[ii].position.x -
						  engine->renderVertexLength) * engine->renderRatio),
						((engine->particles[ii].position.y -
						  engine->renderVertexLength) * engine->renderRatio),
						(engine->particles[ii].position.z *
						 engine->renderRatio));

				cgi_glEnd();
				break;

			case PARTICLE_ENGINE_TYPE_OBJECT:
				draw3d_draw3dObject(engine->object);
				break;

			case PARTICLE_ENGINE_TYPE_UNKNOWN:
			default:
				// do nothing
				break;
		}
	}

	if(engine->texture != NULL) {
		texture_unApply(engine->texture);
	}

	cgi_glPopMatrix();

	if(isDepthTestEnabled) {
		cgi_glEnable(GL_DEPTH_TEST);
	}
}

void particleEngine_resetPhysics(ParticleEngine *engine)
{
	int ii = 0;
	double elapsedTime = 0.0;

	if(engine == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	engine->timestamp = time_getTimeMus();
}

void particleEngine_processPhysics(ParticleEngine *engine)
{
	int ii = 0;
	double elapsedTime = 0.0;

	if(engine == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	if((engine->particleLength < 1) || (engine->particles == NULL)) {
		return;
	}

	elapsedTime = time_getElapsedMusInSeconds(engine->timestamp);

	for(ii = 0; ii < engine->particleLength; ii++) {
		if(!engine->particles[ii].isActive) {
			continue;
		}

		// apply particle effect movement function

		engine->physicsFunction(engine, ii, elapsedTime);

		// apply particle gravity

		engine->particles[ii].direction.x += engine->gravity.x;
		engine->particles[ii].direction.y += engine->gravity.y;
		engine->particles[ii].direction.z += engine->gravity.z;

		// apply particle fade

		engine->particles[ii].life -= engine->particles[ii].fadeRate;
		engine->particles[ii].color.a -= engine->particles[ii].fadeRate;

		if(engine->particles[ii].life <= 0.0) {
			engine->particles[ii].isActive = afalse;
			engine->particles[ii].life = 0.0;
		}
	}

	if(engine->texture != NULL) {
		engine->texture->colorMask.a -= engine->particles[0].fadeRate;
	}

	engine->timestamp = time_getTimeMus();
}

