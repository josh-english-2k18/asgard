/*
 * asgardEngineParticleEngine_unitTest.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard Game Engine multithread-game-engine Wavefront 3D object static-
 * mesh plug-in unit test application.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#include "asgard/asgard.h"


// define Asgard game engine particle engine unit test constants

#define COLOR_BITS										32

#define USE_FULLSCREEN_MODE								afalse

//#define SCREEN_WIDTH									1024
#define SCREEN_WIDTH									1280
//#define SCREEN_WIDTH									2560

//#define SCREEN_HEIGHT									768
#define SCREEN_HEIGHT									1024
//#define SCREEN_HEIGHT									1600

#define PARTICLE_ENGINE_PARTICLES						1024

#define PARTICLE_EFFECT_FIRE_RADIUS						2.0
#define PARTICLE_EFFECT_FIRE_HEIGHT						10.0


// define Asgard game engine particle engine unit test data types

typedef struct _UnitTestState {
	aboolean isLoaded;
	aboolean isExplosion;
	aboolean isPaused;
	int backgroundGlId;
	int srcBlend;
	int destBlend;
	int colorMask;
	Texture *crosshair;
	Texture *background;
	Color ambience;
	LightGL light;
	Camera camera;
	Vector3d upVector;
	Frustum3d frustum;
	Color particleColor;
	ParticleEngine particle;
	Log log;
	Engine engine;
} UnitTestState;


// declare Asgard game engine particle engine unit test functions

static void particleEffect_explosion_initFunction(void *context, int id);

static void particleEffect_explosion_physicsFunction(void *context, int id,
		double elapsedTime);

static void particleEffect_fire_initFunction(void *context, int id);

static void particleEffect_fire_physicsFunction(void *context, int id,
		double elapsedTime);

static int localIoFunction(void *engineContext, void *gameContext,
		void *argument);

static int localRenderFunction(void *engineContext, void *gameContext,
		void *argument);

static int localPhysicsFunction(void *engineContext, void *gameContext,
		void *argument);

static int localGameplayFunction(void *engineContext, void *gameContext,
		void *argument);


// main function

#if defined(WIN32)
int WINAPI WinMain(HINSTANCE instance, HINSTANCE prevInstance,
		LPSTR commandLine, int commandShow)
#elif defined(__linux__)
int main(int argc, char *argv[])
#else // - no plugins available -
#	error "no plugins available for this application"
#endif // - plugins -
{
	int rc = 0;
	char buffer[1024];

	UnitTestState state;

	signal_registerDefault();

	// initialize unit test state

	memset((void *)&state, 0, sizeof(UnitTestState));

	state.isLoaded = afalse;
	state.isExplosion = atrue;
	state.isPaused = afalse;
	state.backgroundGlId = -1;
	state.crosshair = NULL;
	state.background = NULL;

	log_init(&state.log, LOG_OUTPUT_FILE,
			"log/asgard.engine.particle.engine.unit.test.log",
			LOG_LEVEL_DEBUG);

	// initialize game engine

	snprintf(buffer, sizeof(buffer),
			"Asgard Game Engine Particle Engine Unit Test %ix%i",
			SCREEN_WIDTH, SCREEN_HEIGHT);

	if(engine_init(
				&state.engine,
				NULL,
				buffer,
				(void *)&state,
				atrue,
				USE_FULLSCREEN_MODE,
				ASGARD_ENGINE_DEVELOPMENT_BUILD,
				SCREEN_WIDTH,
				SCREEN_HEIGHT,
				COLOR_BITS,
				localIoFunction,
				localRenderFunction,
				localPhysicsFunction,
				localGameplayFunction
				) < 0) {
		return -1;
	}

	// disable the mouse display

	osapi_disableMouseDisplay(state.engine.window);

	// multi-threaded game engine loop

	while(!state.engine.exitApplication) {
		if((rc = osapi_checkState(state.engine.window,
						&state.engine.exitApplication)) != 0) {
			break;
		}

		// take a nap

		time_usleep(1024);
	}

	// cleanup

	engine_free(&state.engine, atrue);

	log_free(&state.log);

	return rc;
}


// define Asgard game engine particle engine unit test functions

static void particleEffect_explosion_initFunction(void *context, int id)
{
	Color color;
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

	engine->particles[id].fadeRate = 0.001;

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

	colors_getColor("Red", &color);

	switch(rand() % 10) {
		case 0:
			colors_getColor("IndianRed", &color);
			break;

		case 1:
			colors_getColor("Red", &color);
			break;

		case 2:
			colors_getColor("FireBrick", &color);
			break;

		case 3:
			colors_getColor("DarkRed", &color);
			break;

		case 4:
			colors_getColor("OrangeRed", &color);
			break;

		case 5:
			colors_getColor("DarkOrange", &color);
			break;

		case 6:
			colors_getColor("Orange", &color);
			break;

		case 7:
			colors_getColor("Yellow", &color);
			break;

		case 8:
			colors_getColor("LightYellow", &color);
			break;

		case 9:
			colors_getColor("LightGoldenrodYellow", &color);
			break;
	}

	engine->particles[id].color.r = color.r;
	engine->particles[id].color.g = color.g;
	engine->particles[id].color.b = color.b;
	engine->particles[id].color.a = color.a;

	engine->particles[id].colorMaskMode = TEXTURE_COLORMASK_MODE_COMBINE;
}

static void particleEffect_explosion_physicsFunction(void *context, int id,
		double elapsedTime)
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

static void particleEffect_fire_initFunction(void *context, int id)
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

	engine->particles[id].fadeRate = 0.0;

	engine->particles[id].direction.x = (double)(rand() % 256);

	if((rand() % 2) == 0) {
		engine->particles[id].direction.x *= -1.0;
	}

	do {
		engine->particles[id].direction.y = (double)(rand() % 256);
	} while(engine->particles[id].direction.y <= 0.0);

	engine->particles[id].direction.z = (double)(rand() % 256);

	if((rand() % 2) == 0) {
		engine->particles[id].direction.z *= -1;
	}

	if((rand() % 2) == 0) {
		engine->particles[id].color.r = CGI_COLOR_RED_RED;
		engine->particles[id].color.g = CGI_COLOR_RED_GREEN;
		engine->particles[id].color.b = CGI_COLOR_RED_BLUE;
		engine->particles[id].color.a = CGI_COLOR_RED_ALPHA;
	}
	else {
		engine->particles[id].color.r = CGI_COLOR_YELLOW_RED;
		engine->particles[id].color.g = CGI_COLOR_YELLOW_GREEN;
		engine->particles[id].color.b = CGI_COLOR_YELLOW_BLUE;
		engine->particles[id].color.a = CGI_COLOR_YELLOW_ALPHA;
	}

	engine->particles[id].colorMaskMode = TEXTURE_COLORMASK_MODE_COMBINE;
}

static void particleEffect_fire_physicsFunction(void *context, int id,
		double elapsedTime)
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

	if((engine->particles[id].position.x >
				(engine->position.x + PARTICLE_EFFECT_FIRE_RADIUS)) ||
			(engine->particles[id].position.x <
			 (engine->position.x - PARTICLE_EFFECT_FIRE_RADIUS))) {
		engine->particles[id].direction.x *= -1.0;
	}

	engine->particles[id].position.y += ((engine->particles[id].direction.y /
				engine->deceleration) * elapsedTime);

	if((engine->particles[id].position.y < engine->position.y) ||
			(engine->particles[id].position.y > 
			 (engine->position.y + PARTICLE_EFFECT_FIRE_HEIGHT))) {
//		engine->particles[id].direction.y *= -1.0;
		engine->particles[id].position.y = engine->position.x;
	}

	engine->particles[id].position.z += ((engine->particles[id].direction.z /
				engine->deceleration) * elapsedTime);

	if((engine->particles[id].position.z >
				(engine->position.z + PARTICLE_EFFECT_FIRE_RADIUS)) ||
			(engine->particles[id].position.z <
			 (engine->position.z - PARTICLE_EFFECT_FIRE_RADIUS))) {
		engine->particles[id].direction.z *= -1.0;
	}
}

static int localIoFunction(void *engineContext, void *gameContext,
		void *argument)
{
	AsgardEvent *event = NULL;
	AsgardKeyboardEvent *keyboardEvent = NULL;
	AsgardMouseEvent *mouseEvent = NULL;
	Engine *engine = NULL;
	UnitTestState *state = NULL;

	engine = (Engine *)engineContext;
	state = (UnitTestState *)gameContext;

	if(!state->isLoaded) {
		return 0;
	}

	event = (AsgardEvent *)argument;

	switch(event->eventType) {
		/*
		 * handle keyboard events
		 */

		case EVENT_TYPE_KEYBOARD:
			keyboardEvent = (AsgardKeyboardEvent *)event->event;

			log_logf(&state->log, LOG_LEVEL_DEBUG,
					"[%s->%s()] keyboard event #%i, code %i (%c)",
					engine->name, __FUNCTION__, keyboardEvent->eventType,
					keyboardEvent->keyCode, keyboardEvent->key);

			if(keyboardEvent->eventType == EVENT_KEYBOARD_PRESSED) {
				if(keyboardEvent->keyCode == ASGARD_KEY_UP) {
					state->ambience.r += 0.1;
					state->ambience.g += 0.1;
					state->ambience.b += 0.1;
					state->ambience.a += 0.1;

					light_setColorAmbient(&state->light, &state->ambience);
					light_setColorDiffuse(&state->light, &state->ambience);
					light_setColorSpecular(&state->light, &state->ambience);
				}
				else if(keyboardEvent->keyCode == ASGARD_KEY_DOWN) {
					state->ambience.r -= 0.1;
					state->ambience.g -= 0.1;
					state->ambience.b -= 0.1;
					state->ambience.a -= 0.1;

					light_setColorAmbient(&state->light, &state->ambience);
					light_setColorDiffuse(&state->light, &state->ambience);
					light_setColorSpecular(&state->light, &state->ambience);
				}
				else if(keyboardEvent->keyCode == ASGARD_KEY_SPACE) {
					if(state->isPaused) {
						state->isPaused = afalse;
					}
					else {
						state->isPaused = atrue;
					}
					particleEngine_resetPhysics(&state->particle);
				}
			}
			else if(keyboardEvent->eventType == EVENT_KEYBOARD_RELEASED) {
				if(keyboardEvent->keyCode == ASGARD_KEY_TAB) {
					if(state->isExplosion) {
						particleEngine_setEffect(&state->particle,
								particleEffect_fire_initFunction,
								particleEffect_fire_physicsFunction,
								NULL);

						particleEngine_allocate(&state->particle,
								PARTICLE_ENGINE_PARTICLES);

						state->isExplosion = afalse;
					}
					else {
						particleEngine_setEffect(&state->particle,
								particleEffect_explosion_initFunction,
								particleEffect_explosion_physicsFunction,
								NULL);

						particleEngine_allocate(&state->particle,
								PARTICLE_ENGINE_PARTICLES);

						state->isExplosion = atrue;
					}

					particleEngine_reset(&state->particle);
				}
				else if(keyboardEvent->keyCode == ASGARD_KEY_B) {
					state->destBlend += 1;
					if(state->destBlend >
							TEXTURE_BLEND_DEST_ONE_MINUS_DST_ALPHA) {
						state->srcBlend += 1;
						if(state->srcBlend >
								TEXTURE_BLEND_SOURCE_ONE_MINUS_DST_ALPHA) {
							state->srcBlend = TEXTURE_BLEND_SOURCE_SRC_COLOR;
						}
						state->destBlend = TEXTURE_BLEND_DEST_SRC_COLOR;
					}

					texture_setBlend(state->particle.texture, state->srcBlend,
							state->destBlend);
				}
				else if(keyboardEvent->keyCode == ASGARD_KEY_N) {
					state->colorMask += 1;
					if(state->colorMask > TEXTURE_COLORMASK_MODE_COMBINE) {
						state->colorMask = TEXTURE_COLORMASK_MODE_ADD;
					}
					texture_setColorMask(state->particle.texture,
							&state->particleColor, state->colorMask);
					particleEngine_setParticleColor(&state->particle,
							&state->particleColor,
							state->colorMask);
				}
			}

			break;

		/*
		 * handle mouse events
		 */

		case EVENT_TYPE_MOUSE:
			mouseEvent = (AsgardMouseEvent *)event->event;

			log_logf(&state->log, LOG_LEVEL_DEBUG,
					"[%s->%s()] mouse event #%i, pos (%i, %i)",
					engine->name, __FUNCTION__, mouseEvent->eventType,
					mouseEvent->xPosition, mouseEvent->yPosition);

			if((mouseEvent->xPosition != (engine->screen.screenWidth / 2)) ||
					(mouseEvent->yPosition !=
					 (engine->screen.screenHeight / 2))) {
				camera_applyMouse(&state->camera, mouseEvent->xPosition,
						mouseEvent->yPosition);

				osapi_setMousePosition(engine->window,
						(engine->screen.screenWidth / 2),
						(engine->screen.screenHeight / 2));
			}

			camera_resetMouse(&state->camera,
					(engine->screen.screenWidth / 2),
					(engine->screen.screenHeight / 2));
			break;
	}

	return 0;
}

static int localRenderFunction(void *engineContext, void *gameContext,
		void *argument)
{
	int lIsSetup = 0;
	int lIsOn = 0;
	int lId = 0;
	int lGlOn = 0;
	double xx = 0.0;
	double yy = 0.0;
	double zz = 0.0;
	double tileIncrement = 5.0;
	double tileBoundary = 75.0;

	Engine *engine = NULL;
	UnitTestState *state = NULL;

	engine = (Engine *)engineContext;
	state = (UnitTestState *)gameContext;

	// render unit test state

	if(!state->isLoaded) {
		canvas_beginRender(&engine->canvas);
		canvas_toggleRenderMode3d(&engine->canvas);
		canvas_toggleRenderMode2d(&engine->canvas);
		CGI_COLOR_WHITE;
		font_draw2d(&engine->font, &engine->canvas, 4, 0,
				"Loading particle engine unit test...");
		canvas_endRender(&engine->canvas);
		return 0;
	}

	// perform particle engine rendering

	// begin render

	canvas_beginRender(&engine->canvas);

	// 3d objects

	canvas_toggleRenderMode3d(&engine->canvas);

	camera_apply(&state->camera);

	frustum3d_setCameraState(&state->frustum, &state->camera.location,
			&state->camera.view, &state->upVector);

	light_switchOn(&state->light);

	lIsSetup = state->light.isSetup;
	lIsOn = state->light.isOn;
	lId = state->light.id;
	lGlOn = cgi_glIsEnabled(GL_LIGHTING);

	if(state->backgroundGlId == -1) {
		state->backgroundGlId = cgi_glGenLists(1);

		cgi_glNewList(state->backgroundGlId, GL_COMPILE);

		cgi_glEnable(GL_TEXTURE_2D);
		texture_apply(state->background);

		xx = -tileBoundary;
		yy = -1.0;
		zz = -tileBoundary;

		while(xx <= tileBoundary) {
			cgi_glBegin(GL_QUADS);

			cgi_glNormal3d(0.0, 1.0, 0.0);
			cgi_glTexCoord2d(0.0, 0.0);
			cgi_glVertex3d(xx, yy, (zz + tileIncrement));

			cgi_glNormal3d(0.0, 1.0, 0.0);
			cgi_glTexCoord2d(1.0, 0.0);
			cgi_glVertex3d((xx + tileIncrement), yy, (zz + tileIncrement));

			cgi_glNormal3d(0.0, 1.0, 0.0);
			cgi_glTexCoord2d(1.0, 1.0);
			cgi_glVertex3d((xx + tileIncrement), yy, zz);

			cgi_glNormal3d(0.0, 1.0, 0.0);
			cgi_glTexCoord2d(0.0, 1.0);
			cgi_glVertex3d(xx, yy, zz);

			cgi_glEnd();

			zz += tileIncrement;
			if(zz >= tileBoundary) {
				xx += tileIncrement;
				zz = -tileBoundary;
			}
		}

		texture_unApply(state->background);
		cgi_glDisable(GL_TEXTURE_2D);

		cgi_glEndList();
	}
	else {
		cgi_glCallList(state->backgroundGlId);
	}

	particleEngine_draw(&state->particle, &engine->canvas);

	light_switchOff(&state->light);

	// 2d objects

	canvas_toggleRenderMode2d(&engine->canvas);

	CGI_COLOR_WHITE;

	font_draw2d(&engine->font, &engine->canvas, 4, 0, "Framerate: %0.2f",
			engine->canvas.framerate);

	font_draw2d(&engine->font, &engine->canvas, 4, 16,
			"Mouse (%i, %i) last (%i, %i)",
			engine->mouse.x,
			engine->mouse.y,
			engine->mouse.lastX,
			engine->mouse.lastY);

	font_draw2d(&engine->font, &engine->canvas, 4, 32,
			"Camera { "
			"l ( %0.2f, %0.2f, %0.2f ) "
			"v ( %0.2f, %0.2f, %0.2f ) "
			"r ( %0.2f, %0.2f, %0.2f ) }",
			state->camera.location.x,
			state->camera.location.y,
			state->camera.location.z,
			state->camera.view.x,
			state->camera.view.y,
			state->camera.view.z,
			state->camera.rotation.x,
			state->camera.rotation.y,
			state->camera.rotation.z);

	font_draw2d(&engine->font, &engine->canvas, 4, 48,
			"Lighting { init %i, on %i, id %i, gl %i, "
			"pos (%0.2f, %0.2f, %0.2f) amb (%0.2f, %0.2f, %0.2f, %0.2f) }",
			lIsSetup, lIsOn, lId, lGlOn,
			state->light.position.x,
			state->light.position.y,
			state->light.position.z,
			state->ambience.r,
			state->ambience.g,
			state->ambience.b,
			state->ambience.a);

	font_draw2d(&engine->font, &engine->canvas, 4, 64,
			"Particle Texture { %i x %i @ %i }",
			state->srcBlend,
			state->destBlend,
			state->colorMask);

	texture_draw2d(state->crosshair,
			((engine->screen.screenWidth / 2) - 32),
			((engine->screen.screenHeight / 2) - 32),
			64,
			64);

	// end render

	canvas_endRender(&engine->canvas);

	return 0;
}

static int localPhysicsFunction(void *engineContext, void *gameContext,
		void *argument)
{
	Engine *engine = NULL;
	UnitTestState *state = NULL;

	engine = (Engine *)engineContext;
	state = (UnitTestState *)gameContext;

	if((!state->isLoaded) || (state->isPaused)) {
		return 0;
	}

	particleEngine_processPhysics(&state->particle);

	return 0;
}

static int localGameplayFunction(void *engineContext, void *gameContext,
		void *argument)
{
	int ii = 0;

	Texture *texture = NULL;
	Engine *engine = NULL;
	UnitTestState *state = NULL;

	engine = (Engine *)engineContext;
	state = (UnitTestState *)gameContext;

	// execute initialization

	if(!state->isLoaded) {
		state->ambience.r = CGI_COLOR_WHITE_RED;
		state->ambience.g = CGI_COLOR_WHITE_GREEN;
		state->ambience.b = CGI_COLOR_WHITE_BLUE;
		state->ambience.a = CGI_COLOR_WHITE_ALPHA;

		light_init(&state->light, 0, 1.0);
		light_setColorAmbient(&state->light, &state->ambience);
		light_setPosition(&state->light, 0.0, 16.0, 0.0);
		light_create(&state->light);

		camera_init(&state->camera);
		camera_setMode(&state->camera, CAMERA_MODE_NO_Y_AXIS);
		camera_setLocation(&state->camera, 0.0, 0.2, 2.0);
		camera_setView(&state->camera, 0.0, 0.2, 0.0);

		vector3d_initSet(&state->upVector, 0.0, 1.0, 0.0);

		frustum3d_init(&state->frustum);
		frustum3d_setSceneState(&state->frustum, CANVAS_Y_VIEW_ANGLE,
				(engine->screen.screenWidth / engine->screen.screenHeight),
				CANVAS_NEAR_Z_PLANE, CANVAS_FAR_Z_PLANE);

		osapi_setMousePosition(engine->window,
				(engine->screen.screenWidth / 2),
				(engine->screen.screenHeight / 2));

		camera_resetMouse(&state->camera,
				(engine->screen.screenWidth / 2),
				(engine->screen.screenHeight / 2));

		state->crosshair = texture_load(&state->log,
				"assets/textures/crosshairs/circle01.config");

		state->background = texture_load(&state->log,
				"assets/textures/surfaces/concrete/concrete01.config");
//				"assets/textures/surfaces/stone/blocktile01.config");

		// particle engine

		particleEngine_init(&state->particle);
		particleEngine_setDeceleration(&state->particle, 40.0);
		particleEngine_setPositionCoords(&state->particle, 0.0, -1.0, -20.0);

		texture = (Texture *)malloc(sizeof(Texture));

		if(texture_initFromTargaFile(texture,
				TEXTURE_QUALITY_HIGHEST,
				TEXTURE_RENDER_REPEAT,
				atrue,
//				"assets/textures/particles/particle.tga") < 0) {
				"assets/textures/particles/glow01.tga") < 0) {
			log_logf(&state->log, LOG_LEVEL_ERROR,
					"failed to load TARGA texture '%s'",
					"assets/textures/particles/glow01.tga");
		}
		else {
			log_logf(&state->log, LOG_LEVEL_DEBUG,
					"loaded TARGA texture '%s'",
					"assets/textures/particles/glow01.tga");
		}

		state->srcBlend = TEXTURE_BLEND_SOURCE_SRC_ALPHA;
		state->destBlend = TEXTURE_BLEND_DEST_DST_ALPHA;
		state->colorMask = TEXTURE_COLORMASK_MODE_COMBINE;

		if(texture_setBlend(texture,
					state->srcBlend,
					state->destBlend) < 0) {
			log_logf(&state->log, LOG_LEVEL_ERROR,
					"unit test %s() failed at %i, aborting",
					__FUNCTION__, __LINE__);
		}

		colors_getColor("Blue", &state->particleColor);

		texture_setColorMask(texture, &state->particleColor, state->colorMask);

		particleEngine_setTexture(&state->particle, texture);

		particleEngine_setEffect(&state->particle,
				particleEffect_explosion_initFunction,
				particleEffect_explosion_physicsFunction,
				NULL);

		particleEngine_allocate(&state->particle,
				PARTICLE_ENGINE_PARTICLES);

		state->isLoaded = atrue;
	}

	if(engine->keyboard.keyState[(int)'w'] > 0) {
		camera_move(&state->camera, 0.1);
	}
	if(engine->keyboard.keyState[(int)'s'] > 0) {
		camera_move(&state->camera, -0.1);
	}
	if(engine->keyboard.keyState[(int)'a'] > 0) {
		camera_strafe(&state->camera, -0.1);
	}
	if(engine->keyboard.keyState[(int)'d'] > 0) {
		camera_strafe(&state->camera, 0.1);
	}
	if(engine->keyboard.keyState[(int)'e'] > 0) {
		camera_climb(&state->camera, 0.1);
	}
	if(engine->keyboard.keyState[(int)'c'] > 0) {
		camera_climb(&state->camera, -0.1);
	}

	return 0;
}

