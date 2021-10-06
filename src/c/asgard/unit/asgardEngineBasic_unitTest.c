/*
 * asgardEngineBasic_unitTest.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard Game Engine multithread-game-engine basic-functionality unit test
 * application.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#include "asgard/asgard.h"

// define Asgard game engine basic unit test constants

#define COLOR_BITS										32

#define USE_FULLSCREEN_MODE								afalse

#define SCREEN_WIDTH									1024

#define SCREEN_HEIGHT									768

#define UT_FONT_SIZE									14

#define UT_TEXTURE_QUALITY								TEXTURE_QUALITY_HIGHEST
#define UT_TEXTURE_RENDER_MODE							TEXTURE_RENDER_REPEAT
#define UT_TEXTURE_HARDWARE_RESIDENT					atrue
#define UT_TEXTURE_FILENAME								\
	"assets/data/test/asgard/targaTest01.tga"
#define UT_TEXTURE_RGBA_ALTERATION						-24

// define Asgard game engine basic unit test data types

typedef struct _UnitTestState {
	aboolean isLoaded;
	aboolean isFontLoaded;
	aboolean isTextureLoaded;
	int textureQuality;
	int blendSource;
	int blendDest;
	Draw2dObject *colorWhite;
	Draw2dObject *loadColor;
	Draw2dObject *defaultFont;
	Draw2dObject *defaultTexture;
	Draw3dObject *sphere;
	Draw3dObject *cone;
	Color adjustColor;
	Font osFont;
	Font lFont;
	Font aFont;
	Texture texture;
	Texture mTexture;
	Texture aTexture;
	Texture rTexture;
	Texture cTexture;
	Texture bTexture;
	Sprite2d sprite;
	Log log;
	AssetCache assetCache;
	Engine engine;
} UnitTestState;

// declare Asgard game engine basic unit test functions

static int localIoFunction(void *engineContext, void *gameContext,
		void *argument);

static int localRenderFunction(void *engineContext, void *gameContext,
		void *argument);

static int localGameplayFunction(void *engineContext, void *gameContext,
		void *argument);

static int unitTest_loadFont(UnitTestState *state);

static int unitTest_freeFont(UnitTestState *state);

static int unitTest_loadTexture(UnitTestState *state);

static int unitTest_freeTexture(UnitTestState *state);

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
	state.isFontLoaded = afalse;
	state.isTextureLoaded = afalse;
	state.textureQuality = UT_TEXTURE_QUALITY;
	state.blendSource = TEXTURE_BLEND_SOURCE_SRC_ALPHA;
	state.blendDest = TEXTURE_BLEND_DEST_ONE_MINUS_SRC_ALPHA;
	state.colorWhite = NULL;
	state.loadColor = NULL;

	log_init(&state.log, LOG_OUTPUT_FILE,
			"log/asgard.engine.basic.unit.test.log",
			LOG_LEVEL_DEBUG);

	// initialize game engine

	snprintf(buffer, sizeof(buffer),
			"Asgard Game Engine Basic Unit Test %ix%i",
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

	// initialize the asset cache

	if(aac_init(&state.assetCache, &state.engine.canvas,
				state.engine.windowHandle, &state.log) < 0) {
		return -1;
	}

	aac_queueAssetLoad2dColor(&state.assetCache,
			"White",
			"Color-White");
	aac_queueAssetLoad2dCustomColor(&state.assetCache,
			"assets/colors/unit.test/blue.config",
			"Color-Blue");
	aac_queueAssetLoad2dFont(&state.assetCache,
			"assets/fonts/unit.test/default.config",
			"Font-Default");
	aac_queueAssetLoad2dTexture(&state.assetCache,
			"assets/textures/unit.test/milla.jovovich.config",
			"Texture-Default");

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

	if(unitTest_freeFont(&state) < 0) {
		return 1;
	}

	if(unitTest_freeTexture(&state) < 0) {
		return 1;
	}

	// cleanup

	engine_free(&state.engine, atrue);

	aac_releaseAsset(&state.assetCache, "Color-White");
	aac_releaseAsset(&state.assetCache, "Color-Blue");
	aac_releaseAsset(&state.assetCache, "Font-Default");
	aac_releaseAsset(&state.assetCache, "Texture-Default");
	aac_free(&state.assetCache);

	return rc;
}


// define Asgard game engine basic unit test functions

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

			if(keyboardEvent->eventType == EVENT_KEYBOARD_PRESSED) {
				if(keyboardEvent->keyCode == ASGARD_KEY_LEFT) {
					spriteDirection = sprite2d_getDirection(&state->sprite);
					spriteDirection -= 1;
					if(spriteDirection < SPRITE2D_DIRECTION_NORTH) {
						spriteDirection = SPRITE2D_DIRECTION_NORTH_NORTH_WEST;
					}
					sprite2d_changeDirection(&state->sprite, spriteDirection,
							SPRITE2D_TURN_COUNTER_CLOCKWISE,
							engine->canvas.framerate);
				}
				else if(keyboardEvent->keyCode == ASGARD_KEY_RIGHT) {
					spriteDirection = sprite2d_getDirection(&state->sprite);
					spriteDirection += 1;
					if(spriteDirection > SPRITE2D_DIRECTION_NORTH_NORTH_WEST) {
						spriteDirection = SPRITE2D_DIRECTION_NORTH;
					}
					sprite2d_changeDirection(&state->sprite, spriteDirection,
							SPRITE2D_TURN_CLOCKWISE,
							engine->canvas.framerate);
				}
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

			state->sprite.processMouseEvent(&state->sprite, mouseEvent);
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
		if(!state->isFontLoaded) {
			canvas_beginRender(&engine->canvas);
			canvas_toggleRenderMode3d(&engine->canvas);
			canvas_toggleRenderMode2d(&engine->canvas);
			CGI_COLOR_WHITE;
			font_draw2d(&engine->font, &engine->canvas, 4, 0,
					"Loading fonts...");
			canvas_endRender(&engine->canvas);
		}
		else if(!state->isTextureLoaded) {
			canvas_beginRender(&engine->canvas);
			canvas_toggleRenderMode3d(&engine->canvas);
			canvas_toggleRenderMode2d(&engine->canvas);
			CGI_COLOR_WHITE;
			font_draw2d(&engine->font, &engine->canvas, 4, 0,
					"Loading textures...");
			canvas_endRender(&engine->canvas);
		}
		return 0;
	}

	// begin render

	canvas_beginRender(&engine->canvas);

	// 3d objects

	canvas_toggleRenderMode3d(&engine->canvas);

	// render position marker

	cgi_glPushMatrix();

	cgi_glTranslated(0.0, 0.0, 0.0);

	CGI_COLOR_BLUE;

	cgi_glBegin(GL_LINES); 
	cgi_glVertex3d(-10000.0, 0.0, 0.0);
	cgi_glVertex3d(10000.0, 0.0, 0.0);
	cgi_glEnd();

	CGI_COLOR_RED;

	cgi_glBegin(GL_LINES); 
	cgi_glVertex3d(0.0, -10000.0, 0.0);
	cgi_glVertex3d(0.0, 10000.0, 0.0);
	cgi_glEnd();

	CGI_COLOR_GREEN;

	cgi_glBegin(GL_LINES); 
	cgi_glVertex3d(0.0, 0.0, -10000.0);
	cgi_glVertex3d(0.0, 0.0, 10000.0);
	cgi_glEnd();

	cgi_glPopMatrix();

	// draw 3d objects

	draw3d_draw3dObject(state->sphere);
	draw3d_draw3dObject(state->cone);

	// 2d objects

	canvas_toggleRenderMode2d(&engine->canvas);

	draw2d_draw2dObject(state->colorWhite);
	font_draw2d(&state->lFont, &engine->canvas, 4, 0, "Framerate: %0.2f",
			engine->canvas.framerate);

	// regular texture

	texture_draw2d(&state->texture, 0, 96, 256, 256);

	// color-masked texture (modulate)

	texture_draw2d(&state->mTexture, 288, 96, 256, 256);

	// color-masked texture (add)

	texture_draw2d(&state->aTexture, 576, 96, 256, 256);

	// font draw2d test

	draw2d_draw2dObject(state->colorWhite);
	font_draw2d(&state->lFont, &engine->canvas, 4, 0, "Framerate: %0.2f",
			engine->canvas.framerate);

	// color test (via draw2d)

	color.r = CGI_COLOR_RED_RED;
	color.g = CGI_COLOR_RED_GREEN;
	color.b = CGI_COLOR_RED_BLUE;
	color.a = CGI_COLOR_RED_ALPHA;

	draw2d_colorFont(&engine->canvas, 4, 14, "Red-Color", &color,
			&state->lFont);

	color.r = CGI_COLOR_BLUE_RED;
	color.g = CGI_COLOR_BLUE_GREEN;
	color.b = CGI_COLOR_BLUE_BLUE;
	color.a = CGI_COLOR_BLUE_ALPHA;

	draw2d_colorFont(&engine->canvas, 4, 28, "Blue-Color", &color,
			&state->lFont);

	color.r = CGI_COLOR_GREEN_RED;
	color.g = CGI_COLOR_GREEN_GREEN;
	color.b = CGI_COLOR_GREEN_BLUE;
	color.a = CGI_COLOR_GREEN_ALPHA;

	draw2d_colorFont(&engine->canvas, 4, 42, "Green-Color", &color,
			&state->lFont);

	// other fonts test

	CGI_COLOR_MAGENTA;
	font_draw2d(&state->osFont, &engine->canvas, 4, 56,
			"The quick brown fox jumped over the lazy dogs "
			"(default OS font).");
	CGI_COLOR_GOLD;
	font_draw2d(&state->aFont, &engine->canvas, 4, 70,
			"The quick brown fox jumped over the lazy dogs "
			"(Arial font).");

	// draw font over texture test

	CGI_COLOR_BLACK;
	font_draw2d(&state->lFont, &engine->canvas, 4, 96,
			"1234567890123456789012345678901");
	CGI_COLOR_AQUA;
	font_draw2d(&state->lFont, &engine->canvas, 4, 110,
			"[texture quality: %i]", state->textureQuality);
	CGI_COLOR_LINEN;
	font_draw2d(&state->lFont, &engine->canvas, 292, 96,
			"[mask (%0.2f, %0.2f, %0.2f)]",
			state->mTexture.colorMask.r,
			state->mTexture.colorMask.g,
			state->mTexture.colorMask.b);
	CGI_COLOR_BLACK;
	font_draw2d(&state->lFont, &engine->canvas, 580, 96,
			"[mask #%i (%0.2f, %0.2f, %0.2f)]",
			state->aTexture.colorMaskMode,
			state->aTexture.colorMask.r,
			state->aTexture.colorMask.g,
			state->aTexture.colorMask.b);

	// mouse & keyboard states

	draw2d_draw2dObject(state->colorWhite);
	font_draw2d(&state->lFont, &engine->canvas, 4, 352,
			"Mouse: (%i, %i)",
			state->engine.mouse.x, state->engine.mouse.y);
	font_draw2d(&state->lFont, &engine->canvas, 4, 366,
			"Keyboard: '%s'",
			state->engine.keyboard.buffer);

	// RGBA altered texture

	texture_draw2d(&state->rTexture, 0, 390, 256, 256);

	// texture from an RGBA color

	texture_draw2d(&state->cTexture, 288, 390, 256, 256);

	// blended texture

	for(ii = 0; ii < 32; ii++) {
		texture_draw2d(&state->bTexture, (576 + ii), (390 + ii),
				256, 256);
	}

	// draw font over texture test

	CGI_COLOR_BLACK;
	font_draw2d(&state->lFont, &engine->canvas, 4, 390,
			"[RGBA (%i, %i, %i)]",
			UT_TEXTURE_RGBA_ALTERATION,
			UT_TEXTURE_RGBA_ALTERATION,
			UT_TEXTURE_RGBA_ALTERATION);
	font_draw2d(&state->lFont, &engine->canvas, 292, 390,
			"[from-color: orchid]");
	font_draw2d(&state->lFont, &engine->canvas, 580, 390,
			"[blended texture: %i, %i]",
			state->blendSource,
			state->blendDest);

	// texture from an RGBA color

	draw2d_draw2dObject(state->defaultTexture);

	// draw a rectangle at the current mouse position


	color.r = CGI_COLOR_BLUE_RED;
	color.g = CGI_COLOR_BLUE_GREEN;
	color.b = CGI_COLOR_BLUE_BLUE;
	color.a = CGI_COLOR_BLUE_ALPHA;

	draw2d_rectangle(&engine->canvas, &color,
			state->engine.mouse.x, state->engine.mouse.y, 24, 24);

	// draw a 2d sprite

	state->sprite.render2d(&state->sprite);

	// end render

	canvas_endRender(&engine->canvas);

	return 0;
}

static int localGameplayFunction(void *engineContext, void *gameContext,
		void *argument)
{
	int assetQueueLength = 0;

	Color *color = NULL;
	Point3d location;
	Vertex3d rotation;
	Engine *engine = NULL;
	UnitTestState *state = NULL;

	static double sphereTiming = 0.0;
	static double coneTiming = 0.0;
	static double qualityTiming = 0.0;
	static double blendTiming = 0.0;

	engine = (Engine *)engineContext;
	state = (UnitTestState *)gameContext;

	// execute unit tests

	if(!state->isLoaded) {
		if(!state->isFontLoaded) {
			assetQueueLength = aac_getAssetLoadQueueLength(&state->assetCache);
			if(assetQueueLength > 0) {
				aac_processLoadAssetQueue(&state->assetCache, 1);
				return 0;
			}

			state->colorWhite = aac_getAsset2d(&state->assetCache,
					"Color-White");
			state->loadColor = aac_getAsset2d(&state->assetCache,
					"Color-Blue");
			state->defaultFont = aac_getAsset2d(&state->assetCache,
					"Font-Default");
			state->defaultTexture = aac_getAsset2d(&state->assetCache,
					"Texture-Default");

			if(state->defaultTexture != NULL) {
				((Draw2dTexture *)state->defaultTexture->payload)->x = 0;
				((Draw2dTexture *)state->defaultTexture->payload)->y = 678;
				((Draw2dTexture *)state->defaultTexture->payload)->width = 256;
				((Draw2dTexture *)state->defaultTexture->payload)->height = 320;
			}

			location.x = 1.0;
			location.y = 0.0;
			location.z = -5.0;

			rotation.x = 0.0;
			rotation.y = 0.0;
			rotation.z = 0.0;

			state->sphere = draw3d_newObjectSphere(&location, &rotation,
					16, 16, 1.8, 0.0, 0.0, -5.0,
					ASGARD_ENGINE_RENDER_MODE_WIREFRAME);

			location.x = -1.0;

			rotation.x = 180.0;
			rotation.y = 180.0;
			rotation.z = 180.0;

			state->cone = draw3d_newObjectCone(&location, &rotation,
					16, 16, 1.8, 1.8, 0.0, 0.0, -5.0,
					ASGARD_ENGINE_RENDER_MODE_WIREFRAME);

			color = colors_newByName("SteelBlue");

			cone3d_setColor(((Draw3dCone *)state->cone->payload)->cone,
					color);

			colors_free(color);

			unitTest_loadFont(state);
			state->isFontLoaded = atrue;
			return 0;
		}
		else if(!state->isTextureLoaded) {
			unitTest_loadTexture(state);
			state->isTextureLoaded = atrue;
			return 0;
		}
		else {
			state->isLoaded = atrue;

			sphereTiming = time_getTimeMus();
			coneTiming = time_getTimeMus();
			qualityTiming = time_getTimeMus();
			blendTiming = time_getTimeMus();

			// setup color-masked texture adjustment

			state->adjustColor.r = 1;
			state->adjustColor.g = -1;
			state->adjustColor.b = 1;
		}
	}

	// change the sphere rotation

	if(time_getElapsedMusInSeconds(sphereTiming) >= 0.01) {
		sphereTiming = time_getTimeMus();
		state->sphere->rotation.x += 0.1;
		state->sphere->rotation.y += 0.1;
		state->sphere->rotation.z += 0.1;
	}

	// change the cone rotation

	if(time_getElapsedMusInSeconds(coneTiming) >= 0.01) {
		coneTiming = time_getTimeMus();
		state->cone->rotation.x += 0.1;
		state->cone->rotation.y += 0.1;
		state->cone->rotation.z += 0.1;
	}

	// change quality of primary texture

	if(time_getElapsedMusInSeconds(qualityTiming) >= 2.0) {
		qualityTiming = time_getTimeMus();
		state->textureQuality += 1;
		if(state->textureQuality > TEXTURE_QUALITY_HIGHEST) {
			state->textureQuality = TEXTURE_QUALITY_LOWEST;
		}
		texture_setQualityLevel(&state->texture, state->textureQuality);
	}

	// adjust color-masked texture mask

	if(state->adjustColor.r == 1) {
		state->mTexture.colorMask.r += 0.001;
		state->aTexture.colorMask.r += 0.001;
		if(state->mTexture.colorMask.r >= 1.0) {
			state->adjustColor.r = -1;
			state->mTexture.colorMask.r = 1.0;
			state->aTexture.colorMask.r = 1.0;
		}
	}
	else {
		state->mTexture.colorMask.r -= 0.001;
		state->aTexture.colorMask.r -= 0.001;
		if(state->mTexture.colorMask.r <= 0.001) {
			state->adjustColor.r = 1;
			state->mTexture.colorMask.r = 0.001;
			state->aTexture.colorMask.r = 0.001;
		}
	}

	if(state->adjustColor.g == 1) {
		state->mTexture.colorMask.g += 0.001;
		state->aTexture.colorMask.g += 0.001;
		if(state->mTexture.colorMask.g >= 1.0) {
			state->adjustColor.g = -1;
			state->mTexture.colorMask.g = 1.0;
			state->aTexture.colorMask.g = 1.0;
		}
	}
	else {
		state->mTexture.colorMask.g -= 0.001;
		state->aTexture.colorMask.g -= 0.001;
		if(state->mTexture.colorMask.g <= 0.001) {
			state->adjustColor.g = 1;
			state->mTexture.colorMask.g = 0.001;
			state->aTexture.colorMask.g = 0.001;
		}
	}

	if(state->adjustColor.b == 1) {
		state->mTexture.colorMask.b += 0.001;
		state->aTexture.colorMask.b += 0.001;
		if(state->mTexture.colorMask.b >= 1.0) {
			state->adjustColor.b = -1;
			state->mTexture.colorMask.b = 1.0;
			state->aTexture.colorMask.b = 1.0;
		}
	}
	else {
		state->mTexture.colorMask.b -= 0.001;
		state->aTexture.colorMask.b -= 0.001;
		if(state->mTexture.colorMask.b <= 0.001) {
			state->adjustColor.b = 1;
			state->mTexture.colorMask.b = 0.001;
			state->aTexture.colorMask.b = 0.001;
		}
	}

	// alter blended-texture blend function, and a-texture color mask

	if(time_getElapsedMusInSeconds(blendTiming) >= 0.256) {
		blendTiming = time_getTimeMus();
		state->blendDest += 1;
		if(state->blendDest > TEXTURE_BLEND_DEST_ONE_MINUS_DST_ALPHA) {
			state->blendDest = TEXTURE_BLEND_DEST_SRC_COLOR;

			state->blendSource += 1;
			if(state->blendSource >
					TEXTURE_BLEND_SOURCE_ONE_MINUS_DST_ALPHA) {
				state->blendSource = TEXTURE_BLEND_SOURCE_SRC_COLOR;
			}
		}

		texture_setBlend(&state->bTexture, state->blendSource,
				state->blendDest);

		state->aTexture.colorMaskMode += 1;
		if(state->aTexture.colorMaskMode > TEXTURE_COLORMASK_MODE_COMBINE) {
			state->aTexture.colorMaskMode = TEXTURE_COLORMASK_MODE_ADD;
		}
	}

	state->sprite.processGameplay(&state->sprite, engine->canvas.framerate);

	return 0;
}


// define Asgard game engine basic unit test - test functions

static int unitTest_loadFont(UnitTestState *state)
{
	log_logf(&state->log, LOG_LEVEL_INFO,
			"unit test %s(): STARTED", __FUNCTION__);

	if(font_init(&state->osFont,
				state->engine.windowHandle,
				FONT_OS_DEFAULT,
				UT_FONT_SIZE,
				afalse,
				afalse,
				afalse,
				FONT_CHARSET_ANSI) < 0) {
		log_logf(&state->log, LOG_LEVEL_ERROR,
				"unit test %s() failed at %i, aborting",
				__FUNCTION__, __LINE__);
		return -1;
	}

	log_logf(&state->log, LOG_LEVEL_DEBUG,
			"constructed font '%s', id #%i",
			state->osFont.name, state->osFont.id);

	if(font_init(&state->lFont,
				state->engine.windowHandle,
				FONT_LUCIDA,
				UT_FONT_SIZE,
				afalse,
				afalse,
				afalse,
				FONT_CHARSET_ANSI) < 0) {
		log_logf(&state->log, LOG_LEVEL_ERROR,
				"unit test %s() failed at %i, aborting",
				__FUNCTION__, __LINE__);
		return -1;
	}

	log_logf(&state->log, LOG_LEVEL_DEBUG,
			"constructed font '%s', id #%i",
			state->lFont.name, state->lFont.id);

	if(font_init(&state->aFont,
				state->engine.windowHandle,
				FONT_ARIAL,
				UT_FONT_SIZE,
				afalse,
				afalse,
				afalse,
				FONT_CHARSET_ANSI) < 0) {
		log_logf(&state->log, LOG_LEVEL_ERROR,
				"unit test %s() failed at %i, aborting",
				__FUNCTION__, __LINE__);
		return -1;
	}

	log_logf(&state->log, LOG_LEVEL_DEBUG,
			"constructed font '%s', id #%i",
			state->aFont.name, state->aFont.id);

	log_logf(&state->log, LOG_LEVEL_INFO,
			"unit test %s(): PASSED", __FUNCTION__);

	return 0;
}

static int unitTest_freeFont(UnitTestState *state)
{
	log_logf(&state->log, LOG_LEVEL_INFO,
			"unit test %s(): STARTED", __FUNCTION__);

	if(font_free(&state->osFont) < 0) {
		log_logf(&state->log, LOG_LEVEL_ERROR,
				"unit test %s() failed at %i, aborting",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(font_free(&state->lFont) < 0) {
		log_logf(&state->log, LOG_LEVEL_ERROR,
				"unit test %s() failed at %i, aborting",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(font_free(&state->aFont) < 0) {
		log_logf(&state->log, LOG_LEVEL_ERROR,
				"unit test %s() failed at %i, aborting",
				__FUNCTION__, __LINE__);
		return -1;
	}

	log_logf(&state->log, LOG_LEVEL_INFO,
			"unit test %s(): PASSED", __FUNCTION__);

	return 0;
}

static int unitTest_loadTexture(UnitTestState *state)
{
	int ii = 0;

	char *filename = NULL;
	char buffer[8192];

	Color colorMask;
	Texture *texture = NULL;

	log_logf(&state->log, LOG_LEVEL_INFO,
			"unit test %s(): STARTED", __FUNCTION__);

	filename = strdup(UT_TEXTURE_FILENAME);

	if(texture_initFromTargaFile(
				&state->texture,
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

	log_logf(&state->log, LOG_LEVEL_DEBUG,
			"constructed texture '%s' at %ix%i, image 0x%llx, id #%i",
			filename, state->texture.width, state->texture.height,
			(aulint)state->texture.image, state->texture.id);

	if(texture_initFromTargaFile(
				&state->mTexture,
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

	log_logf(&state->log, LOG_LEVEL_DEBUG,
			"constructed texture '%s' at %ix%i, image 0x%llx, id #%i",
			filename, state->mTexture.width, state->mTexture.height,
			(aulint)state->mTexture.image, state->mTexture.id);

	colorMask.r = CGI_COLOR_DARKSEAGREEN_RED;
	colorMask.g = CGI_COLOR_DARKSEAGREEN_GREEN;
	colorMask.b = CGI_COLOR_DARKSEAGREEN_BLUE;
	colorMask.a = CGI_COLOR_DARKSEAGREEN_ALPHA;

	if(texture_setColorMask(&state->mTexture, &colorMask,
				TEXTURE_COLORMASK_MODE_MODULATE) < 0) {
		log_logf(&state->log, LOG_LEVEL_ERROR,
				"unit test %s() failed at %i, aborting",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(texture_initFromTargaFile(
				&state->aTexture,
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

	log_logf(&state->log, LOG_LEVEL_DEBUG,
			"constructed texture '%s' at %ix%i, image 0x%llx, id #%i",
			filename, state->aTexture.width, state->aTexture.height,
			(aulint)state->aTexture.image, state->aTexture.id);

	if(texture_setColorMask(&state->aTexture, &colorMask,
				TEXTURE_COLORMASK_MODE_ADD) < 0) {
		log_logf(&state->log, LOG_LEVEL_ERROR,
				"unit test %s() failed at %i, aborting",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(texture_initFromTargaFile(
				&state->rTexture,
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

	log_logf(&state->log, LOG_LEVEL_DEBUG,
			"constructed texture '%s' at %ix%i, image 0x%llx, id #%i",
			filename, state->rTexture.width, state->rTexture.height,
			(aulint)state->rTexture.image, state->rTexture.id);

	if(texture_applyRgbaChange(&state->rTexture, TEXTURE_COLOR_RED,
				UT_TEXTURE_RGBA_ALTERATION) < 0) {
		log_logf(&state->log, LOG_LEVEL_ERROR,
				"unit test %s() failed at %i, aborting",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(texture_applyRgbaChange(&state->rTexture, TEXTURE_COLOR_GREEN,
				UT_TEXTURE_RGBA_ALTERATION) < 0) {
		log_logf(&state->log, LOG_LEVEL_ERROR,
				"unit test %s() failed at %i, aborting",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(texture_applyRgbaChange(&state->rTexture, TEXTURE_COLOR_BLUE,
				UT_TEXTURE_RGBA_ALTERATION) < 0) {
		log_logf(&state->log, LOG_LEVEL_ERROR,
				"unit test %s() failed at %i, aborting",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(texture_applyRgbaChange(&state->rTexture, TEXTURE_COLOR_ALPHA,
				UT_TEXTURE_RGBA_ALTERATION) < 0) {
		log_logf(&state->log, LOG_LEVEL_ERROR,
				"unit test %s() failed at %i, aborting",
				__FUNCTION__, __LINE__);
		return -1;
	}

	colorMask.r = CGI_COLOR_ORCHID_RED;
	colorMask.g = CGI_COLOR_ORCHID_GREEN;
	colorMask.b = CGI_COLOR_ORCHID_BLUE;
	colorMask.a = CGI_COLOR_ORCHID_ALPHA;

	if(texture_initFromColor(
				&state->cTexture,
				UT_TEXTURE_QUALITY,
				UT_TEXTURE_RENDER_MODE,
				UT_TEXTURE_HARDWARE_RESIDENT,
				256,
				256,
				&colorMask
				) < 0) {
		log_logf(&state->log, LOG_LEVEL_ERROR,
				"unit test %s() failed at %i, aborting",
				__FUNCTION__, __LINE__);
		return -1;
	}

	log_logf(&state->log, LOG_LEVEL_DEBUG,
			"constructed texture '%s' at %ix%i, image 0x%llx, id #%i",
			filename, state->cTexture.width, state->cTexture.height,
			(aulint)state->cTexture.image, state->cTexture.id);

	if(texture_initFromTargaFile(
				&state->bTexture,
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

	log_logf(&state->log, LOG_LEVEL_DEBUG,
			"constructed texture '%s' at %ix%i, image 0x%llx, id #%i",
			filename, state->bTexture.width, state->bTexture.height,
			(aulint)state->bTexture.image, state->bTexture.id);

	if(texture_setBlend(&state->bTexture,
				TEXTURE_BLEND_SOURCE_SRC_ALPHA,
				TEXTURE_BLEND_DEST_ONE_MINUS_SRC_ALPHA) < 0) {
		log_logf(&state->log, LOG_LEVEL_ERROR,
				"unit test %s() failed at %i, aborting",
				__FUNCTION__, __LINE__);
		return -1;
	}

	free(filename);

	sprite2d_init(&state->sprite, "Sandrail", 880, 624, 144, 144,
			&state->engine.canvas, &state->engine);

//	sprite2d_setRotateTimeSeconds(&state->sprite, 0.0625);
	sprite2d_setDebugMode(&state->sprite, atrue);
	sprite2d_setIsOutline(&state->sprite, atrue);
	sprite2d_setIsSelectable(&state->sprite, atrue);

	for(ii = 0; ii < SPRITE2D_DIRECTIONS; ii++) {
		snprintf(buffer, (sizeof(buffer) - 1),
				"assets/textures/sprites/sandrail/sandrail%02i.tga",
				(ii + 1));

		texture = (Texture *)malloc(sizeof(Texture));

		if(texture_initFromTargaFile(
					texture,
					UT_TEXTURE_QUALITY,
					UT_TEXTURE_RENDER_MODE,
					UT_TEXTURE_HARDWARE_RESIDENT,
					buffer
					) < 0) {
			log_logf(&state->log, LOG_LEVEL_ERROR,
					"unit test %s() failed at %i, aborting",
					__FUNCTION__, __LINE__);
			return -1;
		}

		if(texture_setBlend(texture,
					TEXTURE_BLEND_SOURCE_SRC_ALPHA,
					TEXTURE_BLEND_DEST_ONE_MINUS_SRC_ALPHA) < 0) {
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

	log_logf(&state->log, LOG_LEVEL_INFO,
			"unit test %s(): PASSED", __FUNCTION__);

	return 0;
}

static int unitTest_freeTexture(UnitTestState *state)
{
	log_logf(&state->log, LOG_LEVEL_INFO,
			"unit test %s(): STARTED", __FUNCTION__);

	if(texture_free(&state->texture) < 0) {
		log_logf(&state->log, LOG_LEVEL_ERROR,
				"unit test %s() failed at %i, aborting",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(texture_free(&state->mTexture) < 0) {
		log_logf(&state->log, LOG_LEVEL_ERROR,
				"unit test %s() failed at %i, aborting",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(texture_free(&state->aTexture) < 0) {
		log_logf(&state->log, LOG_LEVEL_ERROR,
				"unit test %s() failed at %i, aborting",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(texture_free(&state->rTexture) < 0) {
		log_logf(&state->log, LOG_LEVEL_ERROR,
				"unit test %s() failed at %i, aborting",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(texture_free(&state->cTexture) < 0) {
		log_logf(&state->log, LOG_LEVEL_ERROR,
				"unit test %s() failed at %i, aborting",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(texture_free(&state->bTexture) < 0) {
		log_logf(&state->log, LOG_LEVEL_ERROR,
				"unit test %s() failed at %i, aborting",
				__FUNCTION__, __LINE__);
		return -1;
	}

	log_logf(&state->log, LOG_LEVEL_INFO,
			"unit test %s(): PASSED", __FUNCTION__);

	return 0;
}

