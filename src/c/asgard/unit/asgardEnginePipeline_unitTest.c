/*
 * asgardEnginePipeline_unitTest.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard Game Engine multithread-game-engine pipeline unit test
 * application.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#include "asgard/asgard.h"

// define Asgard game engine pipeline unit test constants

#define COLOR_BITS										32

#define USE_FULLSCREEN_MODE								afalse

#define SCREEN_WIDTH									1024

#define SCREEN_HEIGHT									768

// define Asgard game engine pipeline unit test data types

typedef struct _UnitTestState {
	aboolean isLoaded;
	Draw2dObject *defaultFont;
	Draw3dObject *sphere;
	Draw3dObject *cone;
	Log log;
	AssetCache assetCache;
	Pipeline pipeline;
	Engine engine;
} UnitTestState;

// declare Asgard game engine pipeline unit test functions

static int localRenderFunction(void *engineContext, void *gameContext,
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
	state.defaultFont = NULL;
	state.sphere = NULL;
	state.cone = NULL;

	log_init(&state.log, LOG_OUTPUT_FILE,
			"log/asgard.engine.pipeline.unit.test.log",
			LOG_LEVEL_DEBUG);

	// initialize game engine

	snprintf(buffer, sizeof(buffer),
			"Asgard Game Engine Pipeline Unit Test %ix%i",
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
				NULL,
				localRenderFunction,
				NULL,
				localGameplayFunction
				) < 0) {
		return -1;
	}

	// initialize the asset cache

	if(aac_init(&state.assetCache, &state.engine.canvas,
				state.engine.windowHandle, &state.log) < 0) {
		return -1;
	}

	aac_queueAssetLoad2dFont(&state.assetCache,
			"assets/fonts/unit.test/default.config",
			"Font-Default");
	aac_queueAssetLoad2dTexture(&state.assetCache,
			"assets/textures/unit.test/milla.jovovich.config",
			"Texture-Default");
	aac_queueAssetLoad3dShapeSphere(&state.assetCache,
			"assets/meshes/shapes/unit.test/sphere.config",
			"Sphere-Default");
	aac_queueAssetLoad3dShapeCone(&state.assetCache,
			"assets/meshes/shapes/unit.test/cone.config",
			"Cone-Default");

	if(pipeline_init(&state.pipeline, &state.engine.canvas, &state.assetCache,
				&state.log) < 0) {
		return -1;
	}

	if(pipeline_addMode(&state.pipeline, "loading") < 0) {
		return -1;
	}

	if(pipeline_addSection(&state.pipeline, "/loading/2d",
				"loading-screen") < 0) {
		return -1;
	}

	if(pipeline_addSection(&state.pipeline, "/loading/2d/loading-screen",
				"loading-screen-child-section") < 0) {
		return -1;
	}

	pipeline_addMode(&state.pipeline, "unit-test");
	pipeline_addSection(&state.pipeline, "/unit-test/2d", "unit-test");
	pipeline_addSection(&state.pipeline, "/unit-test/3d", "unit-test");

	log_logf(&state.log, LOG_LEVEL_DEBUG,
			"current pipeline mode is '%s'",
			pipeline_getCurrentMode(&state.pipeline));

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

	pipeline_free(&state.pipeline);

	aac_releaseAsset(&state.assetCache, "Font-Default");
	aac_releaseAsset(&state.assetCache, "Texture-Default");
	aac_releaseAsset(&state.assetCache, "Sphere-Default");
	aac_releaseAsset(&state.assetCache, "Cone-Default");
	aac_free(&state.assetCache);

	log_free(&state.log);

	return rc;
}


// define Asgard game engine pipeline unit test functions

static int localRenderFunction(void *engineContext, void *gameContext,
		void *argument)
{
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
		font_draw2d(&engine->font, &engine->canvas, 4, 0, "Initializing...");
		canvas_endRender(&engine->canvas);
		return 0;
	}

	// perform pipeline rendering

	pipeline_render(&state->pipeline);

	return 0;
}

static int localGameplayFunction(void *engineContext, void *gameContext,
		void *argument)
{
	int assetQueueLength = 0;

	Engine *engine = NULL;
	Draw2dObject *object2d = NULL;
	Draw3dObject *object3d = NULL;
	UnitTestState *state = NULL;

	static double sphereTiming = 0.0;
	static double coneTiming = 0.0;

	engine = (Engine *)engineContext;
	state = (UnitTestState *)gameContext;

	// execute initialization

	if(!state->isLoaded) {
		assetQueueLength = aac_getAssetLoadQueueLength(&state->assetCache);
		if(assetQueueLength > 0) {
			aac_processLoadAssetQueue(&state->assetCache, 1);
			return 0;
		}

		state->defaultFont = aac_getAsset2d(&state->assetCache,
				"Font-Default");

		object2d = aac_getAsset2d(&state->assetCache, "Texture-Default");

		if(object2d != NULL) {
			((Draw2dTexture *)object2d->payload)->x = 32;
			((Draw2dTexture *)object2d->payload)->y = 32;
			((Draw2dTexture *)object2d->payload)->width = 256;
			((Draw2dTexture *)object2d->payload)->height = 320;
		}

		pipeline_addObject(&state->pipeline, "/unit-test/2d/unit-test",
				"Texture-Default", object2d, NULL);

		object3d = aac_getAsset3d(&state->assetCache, "Sphere-Default");

		if(object3d != NULL) {
			object3d->location.x = 1.0;
			object3d->location.y = 0.0;
			object3d->location.z = -5.0;
		}

		pipeline_addObject(&state->pipeline, "/unit-test/3d/unit-test",
				"Sphere-Default", object3d, NULL);

		state->sphere = object3d;

		object3d = aac_getAsset3d(&state->assetCache, "Cone-Default");

		if(object3d != NULL) {
			object3d->location.x = -1.0;
			object3d->location.y = 0.0;
			object3d->location.z = -5.0;
		}

		pipeline_addObject(&state->pipeline, "/unit-test/3d/unit-test",
				"Cone-Default", object3d, NULL);

		state->cone = object3d;

		pipeline_switchMode(&state->pipeline, "unit-test");

		state->isLoaded = atrue;
	}

	// change the sphere rotation

	if(time_getElapsedMusInSeconds(sphereTiming) >= 0.001) {
		sphereTiming = time_getTimeMus();

		state->sphere->rotation.x += 0.05;
		state->sphere->rotation.y += 0.05;
		state->sphere->rotation.z += 0.05;
	}

	// change the cone rotation

	if(time_getElapsedMusInSeconds(coneTiming) >= 0.01) {
		coneTiming = time_getTimeMus();

		state->cone->rotation.x += 0.1;
		state->cone->rotation.y += 0.1;
		state->cone->rotation.z += 0.1;
	}

	return 0;
}

