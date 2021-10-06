/*
 * asgardEngine2d_unitTest.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard Game Engine 2d engine functionality unit test application.
 *
 * Written by Josh English.
 */


// preprocessor directives

#include "core/core.h"
#include "asgard/asgard.h"


// define Asgard game engine 2d unit test constants

#define COLOR_BITS										32

#define USE_FULLSCREEN_MODE								afalse

//#define SCREEN_WIDTH									1024
#define SCREEN_WIDTH									1280

//#define SCREEN_HEIGHT									768
#define SCREEN_HEIGHT									1024

#define UT_FONT_SIZE									14

#define UT_TEXTURE_QUALITY								TEXTURE_QUALITY_HIGHEST
#define UT_TEXTURE_RENDER_MODE							TEXTURE_RENDER_REPEAT
#define UT_TEXTURE_HARDWARE_RESIDENT					atrue
#define UT_TEXTURE_FILENAME								\
	"assets/textures/unit.test/abstract.test.pattern.01.tga"

#define UT_TEST_AUDIO_FILE								\
	"assets/sounds/unit.test/unit-test02.ogg"
//	"assets/sounds/unit.test/unit-test.ogg"

typedef enum _UnitTestMode {
	UNIT_TEST_MODE_2D_DRAWING = 1,
	UNIT_TEST_MODE_2D_ANIMATION,
	UNIT_TEST_MODE_2D_SPRITE,
	UNIT_TEST_MODE_2D_TILE,
	UNIT_TEST_MODE_2D_AUDIO,
	UNIT_TEST_MODE_2D_ERROR = -1
} UnitTestMode;


// define Asgard game engine 2d unit test data types

typedef struct _UnitTestState {
	aboolean isLoaded;
	UnitTestMode mode;
	double rotation;
	double rotationTime;
	Color adjustColor;
	Texture rotateTexture;
	Sprite2d sprite;
	Camera2d camera;
	Physics2d physics;
	Tile2d tile;
	Animation2d animation;
	OpenAL openal;
	Mutex openalMutex;
	Log log;
	Engine engine;
} UnitTestState;


// declare Asgard game engine 2d unit test functions

static int localIoFunction(void *engineContext, void *gameContext,
		void *argument);

static int localRenderFunction(void *engineContext, void *gameContext,
		void *argument);

static int localGameplayFunction(void *engineContext, void *gameContext,
		void *argument);

static int unitTest_loadAssets(UnitTestState *state);

static int unitTest_freeAssets(UnitTestState *state);

static char *unitTest_modeToString(UnitTestMode mode);


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
	state.mode = UNIT_TEST_MODE_2D_DRAWING;
	state.rotation = 0.0;
	state.rotationTime = 0.0;

	log_init(&state.log, LOG_OUTPUT_FILE,
			"log/asgard.engine.2d.unit.test.log",
			LOG_LEVEL_DEBUG);

	// initialize game engine

	snprintf(buffer, sizeof(buffer),
			"Asgard Game Engine 2d Unit Test %ix%i",
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

	// multi-threaded game engine loop

	while(!state.engine.exitApplication) {
		if((rc = osapi_checkState(state.engine.window,
						&state.engine.exitApplication)) != 0) {
			break;
		}

		// take a nap

		time_usleep(1024);
	}

	// execute all unit shutdown tests, one at a time

	if(unitTest_freeAssets(&state) < 0) {
		return 1;
	}

	// cleanup

	engine_free(&state.engine, atrue);

	return rc;
}


// define Asgard game engine 2d unit test functions

static int localIoFunction(void *engineContext, void *gameContext,
		void *argument)
{
	int spriteDirection = 0;

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
					"[%s->%s()] keyboard event #%i",
					engine->name, __FUNCTION__, keyboardEvent->eventType);

			if((keyboardEvent->eventType == EVENT_KEYBOARD_RELEASED) &&
					(keyboardEvent->keyCode == ASGARD_KEY_TAB)) {
				state->mode += 1;
				if(state->mode > UNIT_TEST_MODE_2D_AUDIO) {
					state->mode = UNIT_TEST_MODE_2D_DRAWING;
					openal_stop(&state->openal);
				}

				if(state->mode == UNIT_TEST_MODE_2D_AUDIO) {
					// restart audio sequence

					mutex_lock(&state->openalMutex);

					openal_stop(&state->openal);
					openal_closeFileStream(&state->openal);
					openal_openFileStream(&state->openal,
							UT_TEST_AUDIO_FILE);
					openal_play(&state->openal);

					mutex_unlock(&state->openalMutex);
				}
			}
			else if((keyboardEvent->eventType == EVENT_KEYBOARD_RELEASED) &&
					(keyboardEvent->keyCode == ASGARD_KEY_LEFT)) {
				spriteDirection = sprite2d_getDirection(&state->sprite);
				spriteDirection -= 1;
				if(spriteDirection < SPRITE2D_DIRECTION_NORTH) {
					spriteDirection = SPRITE2D_DIRECTION_NORTH_NORTH_WEST;
				}
				sprite2d_changeDirection(&state->sprite, spriteDirection,
						SPRITE2D_TURN_COUNTER_CLOCKWISE,
						engine->canvas.framerate);
			}
			else if((keyboardEvent->eventType == EVENT_KEYBOARD_RELEASED) &&
					(keyboardEvent->keyCode == ASGARD_KEY_RIGHT)) {
				spriteDirection = sprite2d_getDirection(&state->sprite);
				spriteDirection += 1;
				if(spriteDirection > SPRITE2D_DIRECTION_NORTH_NORTH_WEST) {
					spriteDirection = SPRITE2D_DIRECTION_NORTH;
				}
				sprite2d_changeDirection(&state->sprite, spriteDirection,
						SPRITE2D_TURN_CLOCKWISE,
						engine->canvas.framerate);
			}

			break;

		/*
		 * handle mouse events
		 */

		case EVENT_TYPE_MOUSE:
			mouseEvent = (AsgardMouseEvent *)event->event;

			log_logf(&state->log, LOG_LEVEL_DEBUG,
					"[%s->%s()] mouse event #%i",
					engine->name, __FUNCTION__, mouseEvent->eventType);

			if(state->mode == UNIT_TEST_MODE_2D_SPRITE) {
				state->sprite.processMouseEvent(&state->sprite, mouseEvent);
				camera2d_processMouseEvent(&state->camera,
						CAMERA2D_MODE_INVERTED,
						mouseEvent);
			}

			if(mouseEvent->eventType != EVENT_MOUSE_PRESSED) {
				break;
			}

			if(state->mode == UNIT_TEST_MODE_2D_ANIMATION) {
				// restart animation sequence

				animation2d_stop(&state->animation);
/*				animation2d_setPosition(&state->animation,
						mouseEvent->xPosition,
						mouseEvent->yPosition);*/
				animation2d_start(&state->animation, atrue);
			}
			else if(state->mode == UNIT_TEST_MODE_2D_AUDIO) {
				// restart audio sequence

				mutex_lock(&state->openalMutex);

				openal_stop(&state->openal);
				openal_closeFileStream(&state->openal);
				openal_openFileStream(&state->openal,
						UT_TEST_AUDIO_FILE);
				openal_play(&state->openal);

				mutex_unlock(&state->openalMutex);
			}

			break;
	}

	return 0;
}

static int localRenderFunction(void *engineContext, void *gameContext,
		void *argument)
{
	int ii = 0;

	Color color;
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
				"Loading...");
		canvas_endRender(&engine->canvas);
		return 0;
	}

	// begin render

	canvas_beginRender(&engine->canvas);

	// 3d objects

	canvas_toggleRenderMode3d(&engine->canvas);

	// 2d objects

	canvas_toggleRenderMode2d(&engine->canvas);

	// create a blue color

	color.r = CGI_COLOR_BLUE_RED;
	color.g = CGI_COLOR_BLUE_GREEN;
	color.b = CGI_COLOR_BLUE_BLUE;
	color.a = CGI_COLOR_BLUE_ALPHA;

	// set the color

	draw2d_color(&engine->canvas, &color);

	if(state->mode == UNIT_TEST_MODE_2D_DRAWING) {
		// draw a line

		draw2d_line(&engine->canvas, &color, 256, 32, 512, 32);

		// draw a rectangle

		draw2d_rectangle(&engine->canvas, &color, 256, 64, 128, 128);

		// draw a rotated rectangle

		draw2d_rotatedRectangle(&engine->canvas, &color, 416, 64, 128, 128,
				state->rotation);

		// fill a rectangle

		draw2d_fillRectangle(&engine->canvas, &color, 256, 224, 128, 128);

		// fill a rotated rectangle

		draw2d_fillRotatedRectangle(&engine->canvas, &color, 416, 224, 128, 128,
				-(state->rotation));

		// draw a circle

		draw2d_circle(&engine->canvas, &color, 256, 384, 64.0);

		// fill a circle

		draw2d_fillCircle(&engine->canvas, &color, 256, 544, 64.0);

		// draw a texture

		draw2d_texture(&engine->canvas, 576, 64, 128, 128,
				&state->rotateTexture);

		// draw a rotated texture

		draw2d_rotatedTexture(&engine->canvas, 736, 64, 128, 128,
				state->rotation, &state->rotateTexture);
	}
	else if(state->mode == UNIT_TEST_MODE_2D_ANIMATION) {
		// draw a 2d animation

		animation2d_render2d(&state->animation);
	}
	else if(state->mode == UNIT_TEST_MODE_2D_SPRITE) {
		// draw a texture

		draw2d_texture(&engine->canvas,
				(64 - camera2d_getX(&state->camera)),
				(64 - camera2d_getY(&state->camera)),
				1024,
				1024,
				&state->rotateTexture);

		// draw a 2d sprite

		state->sprite.render2d(&state->sprite);
	}
	else if(state->mode == UNIT_TEST_MODE_2D_TILE) {
		// draw a 2d tile

		tile2d_render2d(&state->tile);
	}
	else if(state->mode == UNIT_TEST_MODE_2D_AUDIO) {
		/*
		 * TODO: render some info about the play track
		 */
	}

	// draw framerate and other status info

	CGI_COLOR_WHITE;

	font_draw2d(&engine->font, &engine->canvas, 4, 0, "Framerate: %0.2f",
			engine->canvas.framerate);

	font_draw2d(&engine->font, &engine->canvas, 4, 20,
			"Mouse: (%i, %i)",
			state->engine.mouse.x, state->engine.mouse.y);

	if(state->mode != UNIT_TEST_MODE_2D_SPRITE) {
		font_draw2d(&engine->font, &engine->canvas, 4, 40,
				"Keyboard: '%s'",
				state->engine.keyboard.buffer);
	}
	else {
		font_draw2d(&engine->font, &engine->canvas, 4, 40,
				"Keyboard: '%s'",
				"(output disabled)");
	}

	font_draw2d(&engine->font, &engine->canvas, 4, 60,
			"Unit Test Mode: '%s' (hit 'tab' to toggle modes)",
			unitTest_modeToString(state->mode));

	// draw a rectangle at the current mouse position

	draw2d_rectangle(&engine->canvas, &color,
			state->engine.mouse.x, state->engine.mouse.y, 24, 24);

	// end render

	canvas_endRender(&engine->canvas);

	return 0;
}

static int localGameplayFunction(void *engineContext, void *gameContext,
		void *argument)
{
	int x = 0;
	int y = 0;
	int spriteDirection = 0;
	double elapsedTime = 0.0;

	Engine *engine = NULL;
	UnitTestState *state = NULL;

	const double ROTATION_RATIO = 32.0;

	engine = (Engine *)engineContext;
	state = (UnitTestState *)gameContext;

	// execute unit tests

	if(!state->isLoaded) {
		unitTest_loadAssets(state);

		state->isLoaded = atrue;

		return 0;
	}

	if(state->mode == UNIT_TEST_MODE_2D_DRAWING) {
		// rotate texture

		if(state->rotationTime == 0.0) {
			state->rotationTime = time_getTimeMus();
		}
		else {
			elapsedTime = time_getElapsedMusInMilliseconds(state->rotationTime);
			state->rotationTime = time_getTimeMus();

			state->rotation += (elapsedTime / ROTATION_RATIO);

			if(state->rotation >= 360.0) {
				state->rotation = 0.0;
			}
		}
	}
	else if(state->mode == UNIT_TEST_MODE_2D_ANIMATION) {
		// process animation

		if(!state->animation.isPlaying) {
			animation2d_start(&state->animation, atrue);
		}

		animation2d_processAnimation(&state->animation);
	}
	else if(state->mode == UNIT_TEST_MODE_2D_SPRITE) {
		// process physics

		if(engine->keyboard.keyState[(int)'w'] > 0) {
			log_logf(&state->log, LOG_LEVEL_DEBUG,
					"moving 2d physics engine :: FORWARD");
			physics2d_processPhysics(&state->physics,
					PHYSICS_DIRECTION_FORWARD);
		}
		if(engine->keyboard.keyState[(int)'s'] > 0) {
			log_logf(&state->log, LOG_LEVEL_DEBUG,
					"moving 2d physics engine :: BACKWARD");
			physics2d_processPhysics(&state->physics,
					PHYSICS_DIRECTION_BACKWARD);
		}
		if(engine->keyboard.keyState[(int)'a'] > 0) {
			spriteDirection = sprite2d_getDirection(&state->sprite);
			spriteDirection -= 1;
			if(spriteDirection < SPRITE2D_DIRECTION_NORTH) {
				spriteDirection = SPRITE2D_DIRECTION_NORTH_NORTH_WEST;
			}
			sprite2d_changeDirection(&state->sprite, spriteDirection,
					SPRITE2D_TURN_COUNTER_CLOCKWISE,
					engine->canvas.framerate);
		}
		if(engine->keyboard.keyState[(int)'d'] > 0) {
			spriteDirection = sprite2d_getDirection(&state->sprite);
			spriteDirection += 1;
			if(spriteDirection > SPRITE2D_DIRECTION_NORTH_NORTH_WEST) {
				spriteDirection = SPRITE2D_DIRECTION_NORTH;
			}
			sprite2d_changeDirection(&state->sprite, spriteDirection,
					SPRITE2D_TURN_CLOCKWISE,
					engine->canvas.framerate);
		}
		if(engine->keyboard.keyState[(int)'t'] > 0) {
			physics2d_setIsUsingTurbo(&state->physics, atrue);
		}
		else {
			physics2d_setIsUsingTurbo(&state->physics, afalse);
		}

		physics2d_processGameplay(&state->physics);

		log_logf(&state->log, LOG_LEVEL_DEBUG,
				"2d physics engine { "
				"pos(%0.2f, %0.2f) / "
				"speed(%0.2f, %0.2f) / "
				"boost(%0.2f, %0.2f) / "
				"calc(%0.2f, %0.2f) } ",
				state->physics.position.x,
				state->physics.position.y,
				state->physics.position.xSpeed,
				state->physics.position.ySpeed,
				state->physics.position.xBoost,
				state->physics.position.yBoost,
				state->physics.position.xSpeedCalc,
				state->physics.position.ySpeedCalc);

		// process camera system

		x = sprite2d_getCameraDiffX(&state->sprite);
		y = sprite2d_getCameraDiffY(&state->sprite);

		log_logf(&state->log, LOG_LEVEL_DEBUG,
				"applying sprite camera diff of (%i, %i)", x, y);

		camera2d_applyCameraDiff(&state->camera, x, y);

		sprite2d_applyCamera(&state->sprite, &state->camera);

		// process sprite game-play

		state->sprite.processGameplay(&state->sprite, engine->canvas.framerate);
	}
	else if(state->mode == UNIT_TEST_MODE_2D_AUDIO) {
		// process sound

		mutex_lock(&state->openalMutex);

		if(openal_processPlayStream(&state->openal) < 0) {
			openal_stop(&state->openal);
			openal_closeFileStream(&state->openal);
			openal_openFileStream(&state->openal, UT_TEST_AUDIO_FILE);
			openal_play(&state->openal);
		}

		mutex_unlock(&state->openalMutex);
	}

	return 0;
}


// define Asgard game engine 2d unit test - test functions

static Texture *getTgaTexture(char *filename)
{
	Texture *result = NULL;

	result = (Texture *)malloc(sizeof(Texture));

	if(texture_initFromTargaFile(result,
			TEXTURE_QUALITY_HIGHEST,
			TEXTURE_RENDER_REPEAT,
			atrue,
			filename)) {
		return NULL;
	}

	if(texture_setBlend(result,
			TEXTURE_BLEND_SOURCE_SRC_ALPHA,
			TEXTURE_BLEND_DEST_ONE_MINUS_SRC_ALPHA) < 0) {
		return NULL;
	}

	return result;
}

static int unitTest_loadAssets(UnitTestState *state)
{
	int ii = 0;
	char *filename = NULL;
	char buffer[8192];

	Texture *texture = NULL;

	log_logf(&state->log, LOG_LEVEL_INFO,
			"unit test %s(): STARTED", __FUNCTION__);

	// load rotation-test texture

	filename = strdup(UT_TEXTURE_FILENAME);

	if(texture_initFromTargaFile(
				&state->rotateTexture,
				UT_TEXTURE_QUALITY,
				UT_TEXTURE_RENDER_MODE,
				UT_TEXTURE_HARDWARE_RESIDENT,
				filename
				) < 0) {
		log_logf(&state->log, LOG_LEVEL_ERROR,
				"unit test %s() failed at %i, aborting",
				__FUNCTION__, __LINE__);
		return -1;
	}

	free(filename);

	// initialize & load sprite textures

	sprite2d_init(&state->sprite, "Sandrail",
			((SCREEN_WIDTH - 132) / 2),
			((SCREEN_HEIGHT - 132) / 2),
			132, 132,
			&state->engine.canvas, &state->engine);

//	sprite2d_setRotateTimeSeconds(&state->sprite, 0.0625);
	sprite2d_setDebugMode(&state->sprite, atrue);
	sprite2d_setIsOutline(&state->sprite, atrue);
	sprite2d_setIsSelectable(&state->sprite, atrue);

	for(ii = 0; ii < SPRITE2D_DIRECTIONS; ii++) {
		snprintf(buffer, (sizeof(buffer) - 1),
				"assets/textures/sprites/sandrail/sandrail%02i.tga",
				(ii + 1));

		if((texture = getTgaTexture(buffer)) == NULL) {
			log_logf(&state->log, LOG_LEVEL_ERROR,
					"unit test %s() failed at %i, aborting",
					__FUNCTION__, __LINE__);
			return -1;
		}

		log_logf(&state->log, LOG_LEVEL_DEBUG,
				"constructed texture '%s' at %ix%i, image 0x%llx, id #%i",
				buffer, texture->width, texture->height,
				(aulint)texture->image, texture->id);

		sprite2d_setTexture(&state->sprite, (Sprite2dDirection)ii, texture);
	}

	// initialize 2d camera system

	camera2d_init(&state->camera, "UnitTestCamera", 0, 0,
			SCREEN_WIDTH, SCREEN_HEIGHT);

	// initialize 2d physics engine

	physics2d_init(&state->physics, &state->sprite);

	// 2d animation

	animation2d_init(&state->animation, "UnitTestAnimation", 384, 256, 512, 512,
			&state->engine);

	animation2d_setAnimationTime(&state->animation, 32.0);

	for(ii = 0; ii < 30; ii++) {
		snprintf(buffer, (sizeof(buffer) - 1),
				"assets/textures/animations/explosion01/explosion_%02i.tga",
				(ii + 1));

		if((texture = getTgaTexture(buffer)) == NULL) {
			log_logf(&state->log, LOG_LEVEL_ERROR,
					"unit test %s() failed at %i, aborting",
					__FUNCTION__, __LINE__);
			return -1;
		}

		animation2d_addFrameTexture(&state->animation, texture);
	}

	// 2d tile

	tile2d_init(&state->tile, "UnitTestTile", 32, 544, 128, 128,
			&state->engine.canvas);
	tile2d_setDebugMode(&state->tile, atrue);

	texture = texture_load(&state->log,
			"assets/textures/tiles/unit.test/test-tile.config");

	tile2d_setTexture(&state->tile, texture);

	// OpenAL audio, using Ogg/Vorbis

	mutex_init(&state->openalMutex);

	if(openal_init(&state->openal, OPENAL_DEFAULT_DEVICE_NAME) < 0) {
		log_logf(&state->log, LOG_LEVEL_ERROR,
				"unit test %s() failed at %i, aborting",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(openal_makeCurrent(&state->openal) < 0) {
		log_logf(&state->log, LOG_LEVEL_ERROR,
				"unit test %s() failed at %i, aborting",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(openal_openFileStream(&state->openal, UT_TEST_AUDIO_FILE) < 0) {
		log_logf(&state->log, LOG_LEVEL_ERROR,
				"unit test %s() failed at %i, aborting",
				__FUNCTION__, __LINE__);
		return -1;
	}

	log_logf(&state->log, LOG_LEVEL_INFO,
			"unit test %s(): PASSED", __FUNCTION__);

	return 0;
}

static int unitTest_freeAssets(UnitTestState *state)
{
	log_logf(&state->log, LOG_LEVEL_INFO,
			"unit test %s(): STARTED", __FUNCTION__);

	sprite2d_free(&state->sprite);

	log_logf(&state->log, LOG_LEVEL_INFO,
			"unit test %s(): PASSED", __FUNCTION__);

	return 0;
}

static char *unitTest_modeToString(UnitTestMode mode)
{
	char *result = NULL;

	switch(mode) {
		case UNIT_TEST_MODE_2D_DRAWING:
			result = "Drawing";
			break;

		case UNIT_TEST_MODE_2D_ANIMATION:
			result = "Animation";
			break;

		case UNIT_TEST_MODE_2D_SPRITE:
			result = "Sprite";
			break;

		case UNIT_TEST_MODE_2D_TILE:
			result = "Tile";
			break;

		case UNIT_TEST_MODE_2D_AUDIO:
			result = "Audio";
			break;

		case UNIT_TEST_MODE_2D_ERROR:
		default:
			result = "Error";
	}

	return result;
}

