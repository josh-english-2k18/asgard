/*
 * asgardEngineWfobj.c
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

// define Asgard game engine wfobj unit test constants

#define COLOR_BITS										32

#define USE_FULLSCREEN_MODE								afalse

//#define SCREEN_WIDTH									1024
#define SCREEN_WIDTH									1280
//#define SCREEN_WIDTH									2560

//#define SCREEN_HEIGHT									768
#define SCREEN_HEIGHT									1024
//#define SCREEN_HEIGHT									1600

#define WFOBJ_FILENAME									\
	"assets/meshes/wfobj/buggy/buggy.obj"
/*
	"assets/meshes/wfobj/buggy/buggy.obj"
	"assets/meshes/wfobj/fallencity/FallenCity.obj"
	"assets/meshes/wfobj/fallencity/FallenCity_Export01.obj"
	"assets/meshes/wfobj/dropship/DropShip.obj"
	"assets/meshes/wfobj/gatlingturret/GatlingTurret.obj"
	"assets/meshes/wfobj/blackhawk/blackhawk.obj"
*/

// define Asgard game engine wfobj unit test data types

typedef struct _UnitTestState {
	aboolean isLoaded;
	int backgroundGlId;
	char *loadingFilename;
	Wfobj *wfobj;
	Texture *crosshair;
	Texture *background;
	Texture **wfobjTextures;
	Color ambience;
	LightGL light;
	Camera camera;
	Vector3d upVector;
	Frustum3d frustum;
	AsgardEngineRenderModes renderMode;
	Log log;
	Engine engine;
} UnitTestState;

// declare Asgard game engine wfobj unit test functions

static int localIoFunction(void *engineContext, void *gameContext,
		void *argument);

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
	state.backgroundGlId = -1;
	state.loadingFilename = (char *)malloc(sizeof(char) * 8192);
	strcpy(state.loadingFilename, WFOBJ_FILENAME);
	state.wfobj = NULL;
	state.crosshair = NULL;
	state.background = NULL;
	state.wfobjTextures = NULL;
	state.renderMode = ASGARD_ENGINE_RENDER_MODE_SOLID;

	log_init(&state.log, LOG_OUTPUT_FILE,
			"log/asgard.engine.wfobj.unit.test.log",
			LOG_LEVEL_DEBUG);

	// initialize game engine

	snprintf(buffer, sizeof(buffer),
			"Asgard Game Engine Wfobj Unit Test %ix%i",
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
				NULL,
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


// define Asgard game engine wfobj unit test functions

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
				else if(keyboardEvent->keyCode == ASGARD_KEY_TAB) {
					state->renderMode += 1;
					if(state->renderMode > ASGARD_ENGINE_RENDER_MODE_POINTS) {
						state->renderMode = ASGARD_ENGINE_RENDER_MODE_SOLID;
					}
				}
/*				if(keyboardEvent->keyCode == ASGARD_KEY_W) {
					camera_move(&state->camera, 0.1);
				}
				else if(keyboardEvent->keyCode == ASGARD_KEY_S) {
					camera_move(&state->camera, -0.1);
				}
				else if(keyboardEvent->keyCode == ASGARD_KEY_A) {
					camera_strafe(&state->camera, -0.1);
				}
				else if(keyboardEvent->keyCode == ASGARD_KEY_D) {
					camera_strafe(&state->camera, 0.1);
				}
				else if(keyboardEvent->keyCode == ASGARD_KEY_E) {
					camera_climb(&state->camera, 0.1);
				}
				else if(keyboardEvent->keyCode == ASGARD_KEY_C) {
					camera_climb(&state->camera, -0.1);
				}*/
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
				"Wfobj unit test loading file '%s'...", state->loadingFilename);
		canvas_endRender(&engine->canvas);
		return 0;
	}

	// perform wfobj rendering

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

	wfobj_render3d(
//			ASGARD_ENGINE_RENDER_MODE_WIREFRAME,
//			ASGARD_ENGINE_RENDER_MODE_TEXTURED,
			state->renderMode,
			0.0, -1.0, -4.0,						// default
//			0.0, 31.6, -75.0,						// fallencity
//			0.0, -0.95, 0.0,						// dropship
//			0.0, -1.0, -4.0,						// gatlingturret
//			0.0, -1.0, -4.0,						// blackhawk
			state->wfobj,
			(void *)state->wfobjTextures,
			NULL,
			(void *)&state->frustum,
			0.2,									// default
//			0.1,									// fallencity
//			0.09,									// dropship
//			0.09,									// gatlingturret
//			0.01,									// blackhawk
//			atrue
			afalse
			);

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
			"Wfobj { polygons rendered %i }",
			state->wfobj->renderPolygonCount);

	texture_draw2d(state->crosshair,
			((engine->screen.screenWidth / 2) - 32),
			((engine->screen.screenHeight / 2) - 32),
			64,
			64);

	// end render

	canvas_endRender(&engine->canvas);

	return 0;
}

static int localGameplayFunction(void *engineContext, void *gameContext,
		void *argument)
{
	int ii = 0;

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

		state->wfobj = wfobj_load(&state->log, WFOBJ_FILENAME);
		if(state->wfobj == NULL) {
			log_logf(&state->log, LOG_LEVEL_ERROR,
					"failed to load the Wavefront object");
			exit(1);
		}

		log_logf(&state->log, LOG_LEVEL_DEBUG,
				"preparing to load %i textures",
				state->wfobj->materialCount);

		state->wfobjTextures = (Texture **)malloc(sizeof(Texture *) *
				state->wfobj->materialCount);
		for(ii = 0; ii < state->wfobj->materialCount; ii++) {
			snprintf(state->loadingFilename, (sizeof(char) * 8192), "#%i '%s'",
					ii, state->wfobj->materials[ii]->filename);

			log_logf(&state->log, LOG_LEVEL_DEBUG,
					"loading TARGA image '%s' as a 3d mesh texture",
					state->wfobj->materials[ii]->filename);

			state->wfobjTextures[ii] = (Texture *)malloc(sizeof(Texture));

			if(texture_initFromTargaFile(state->wfobjTextures[ii],
					TEXTURE_QUALITY_HIGHEST,
					TEXTURE_RENDER_REPEAT,
					atrue,
					state->wfobj->materials[ii]->filename) < 0) {
				log_logf(&state->log, LOG_LEVEL_ERROR,
						"failed to load TARGA texture '%s'",
						state->wfobj->materials[ii]->filename);
				if(texture_initFromTargaFile(state->wfobjTextures[ii],
						TEXTURE_QUALITY_HIGHEST,
						TEXTURE_RENDER_REPEAT,
						atrue,
						"assets/textures/surfaces/default02.tga") < 0) {
					continue;
				}
			}

			if(texture_setBlend(state->wfobjTextures[ii],
						TEXTURE_BLEND_SOURCE_SRC_ALPHA,
						TEXTURE_BLEND_DEST_ONE_MINUS_SRC_ALPHA) < 0) {
				log_logf(&state->log, LOG_LEVEL_ERROR,
						"unit test %s() failed at %i, aborting",
						__FUNCTION__, __LINE__);
			}
		}

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

