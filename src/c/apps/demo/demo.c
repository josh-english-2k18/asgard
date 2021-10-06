/*
 * demo.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * A simple 2D Asgard engine demo.
 *
 * Written by Josh English.
 */


// preprocessor directives

#include "core/core.h"
#include "asgard/asgard.h"
#define _APPS_DEMO_COMPONENT
#include "apps/demo/demo.h"
#define _ASGARD_CORE_PLUGIN_COMPONENT
#include "asgard/core/plugin/androidapi.h"


// define demo private functions

// initialization functions

static Texture *load_createTgaTexture(Demo *context, char *filename)
{
	Texture *result = NULL;

	if((context == NULL) || (filename == NULL)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	result = (Texture *)malloc(sizeof(Texture));

#if !defined(__ANDROID__)
	if(texture_initFromTargaFile(result,
			TEXTURE_QUALITY_HIGHEST,
			TEXTURE_RENDER_REPEAT,
			atrue,
			filename)) {
		log_logf(&context->engine.log, LOG_LEVEL_ERROR,
				"failed to init texture '%s' from TGA file", filename);
		free(result);
		return NULL;
	}
#else // __ANDROID__
	if(androidapi_textureUtil_loadTexture(context->zipContext,
				&context->engine.log,
				result,
				filename) < 0) {
		log_logf(&context->engine.log, LOG_LEVEL_ERROR,
				"failed to load texture '%s' from zipped TGA file",
				filename);
		free(result);
		return NULL;
	}
#endif // !__ANDROID__

	if(texture_setBlend(result,
			TEXTURE_BLEND_SOURCE_SRC_ALPHA,
			TEXTURE_BLEND_DEST_ONE_MINUS_SRC_ALPHA) < 0) {
		log_logf(&context->engine.log, LOG_LEVEL_ERROR,
				"failed to set blending on texture '%s'", filename);
		free(result);
		return NULL;
	}

	log_logf(&context->engine.log, LOG_LEVEL_DEBUG,
			"constructed texture '%s', size %ix%i, image 0x%llx, id #%i",
			filename, result->width, result->height,
			(aulint)result->image, result->id);

	return result;
}

static int load_loadingScreen(Demo *context)
{
	Texture *texture = NULL;

#if !defined(__ANDROID__)
	char *AssetList[] = {
		"assets/textures/demo/loading.screen.tga",
		NULL
	};
#else // __ANDROID__
	char *AssetList[] = {
		"assets/textures/loading.screen.tga",
		NULL
	};
#endif // !__ANDROID__

	if(context == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	// load textures

	if((context->assets.loadingScreen = load_createTgaTexture(context,
					AssetList[0])) == NULL) {
		return -1;
	}

	return 0;
}

static void load_pickRandomColor(Color *color)
{
	switch(rand() % 11) {
		case 0:
			color->r = 0.507812;
			color->g = 0.542968;
			color->b = 0.125000;
			color->a = 1.0;
			break;

		case 1:
			color->r = 0.6875;
			color->g = 0.671875;
			color->b = 0.19140625;
			color->a = 1.0;
			break;

		case 2:
			color->r = 0.79296875;
			color->g = 0.76953125;
			color->b = 0.23828125;
			color->a = 1.0;
			break;

		case 3:
			color->r = 0.9765625;
			color->g = 0.83984375;
			color->b = 0.47265625;
			color->a = 1.0;
			break;

		case 4:
			color->r = 0.97265625;
			color->g = 0.890625;
			color->b = 0.67578125;
			color->a = 1.0;
			break;

		case 5:
			color->r = 0.9765625;
			color->g = 0.9453125;
			color->b = 0.85546875;
			color->a = 1.0;
			break;

		case 6:
			color->r = 0.3359375;
			color->g = 0.20703125;
			color->b = 0.0703125;
			color->a = 1.0;
			break;

		case 7:
			color->r = 0.60546875;
			color->g = 0.2890625;
			color->b = 0.04296875;
			color->a = 1.0;
			break;

		case 8:
			color->r = 0.82421875;
			color->g = 0.3984375;
			color->b = 0.0;
			color->a = 1.0;
			break;

		case 9:
			color->r = 0.9921875;
			color->g = 0.5390625;
			color->b = 0.0;
			color->a = 1.0;
			break;

		case 10:
			color->r = 0.97265625;
			color->g = 0.65234375;
			color->b = 0.12109375;
			color->a = 1.0;
			break;
	}
}

static int load_gameAssets(Demo *context)
{
	int ii = 0;
	int nn = 0;
	double timing = 0.0;
	char buffer[8192];

	Color color;
	Texture *texture = NULL;

#if !defined(__ANDROID__)
	char *AssetList[] = {
		"assets/textures/demo/sprite.tga",
		"assets/textures/demo/sprite02.tga",
		"assets/textures/demo/sprite03.tga",
		NULL
	};
#else // __ANDROID__
	char *AssetList[] = {
		"assets/textures/sprite.tga",
		"assets/textures/sprite02.tga",
		"assets/textures/sprite03.tga",
		NULL
	};
#endif // !__ANDROID__

	if(context == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	timing = time_getTimeMus();

	system_pickRandomSeed();

	log_logf(&context->engine.log, LOG_LEVEL_INFO, "began loading assets");

	// load textures

	if((context->assets.spriteOne = load_createTgaTexture(context,
					AssetList[0])) == NULL) {
		return -1;
	}

	if((context->assets.spriteTwo = load_createTgaTexture(context,
					AssetList[1])) == NULL) {
		return -1;
	}

	if((context->assets.spriteThree = load_createTgaTexture(context,
					AssetList[2])) == NULL) {
		return -1;
	}

	// initialize sprites

	color.r = 0.0;
	color.g = 0.0;
	color.b = 0.0;
	color.a = CGI_COLOR_BLACK_ALPHA;

	for(ii = 0; ii < DEMO_MAX_SPRITES; ii++) {
		context->assets.sprites[ii].isAlive = afalse;
		context->assets.sprites[ii].isFirstPass = atrue;

		sprite2d_init(&context->assets.sprites[ii].sprite,
				"Sprite",
				-100, -100,
				40, 40,
				&context->engine.canvas, &context->engine);

		sprite2d_setDebugMode(&context->assets.sprites[ii].sprite, afalse);
		sprite2d_setIsOutline(&context->assets.sprites[ii].sprite, afalse);
		sprite2d_setIsSelectable(&context->assets.sprites[ii].sprite, afalse);

		switch(ii % 3) {
			case 0:
				sprite2d_setTexture(&context->assets.sprites[ii].sprite,
						SPRITE2D_DIRECTION_EAST,
						context->assets.spriteOne);
				break;

			case 1:
				sprite2d_setTexture(&context->assets.sprites[ii].sprite,
						SPRITE2D_DIRECTION_EAST,
						context->assets.spriteTwo);
				break;

			case 2:
				sprite2d_setTexture(&context->assets.sprites[ii].sprite,
						SPRITE2D_DIRECTION_EAST,
						context->assets.spriteThree);
				break;
		}

		physics2d_init(&context->assets.sprites[ii].physics,
				&context->assets.sprites[ii].sprite);
		physics2d_setHasFriction(&context->assets.sprites[ii].physics,
				afalse);
//		physics2d_setFriction(&context->assets.sprites[ii].physics, 0.999);
		physics2d_setFriction(&context->assets.sprites[ii].physics, 4.0);
//		physics2d_setFriction(&context->assets.sprites[ii].physics, 10.0);
		physics2d_setThrust(&context->assets.sprites[ii].physics,
				DEMO_SPRITES_STARTING_THRUST);

		for(nn = 0; nn < 5; nn++) {
			load_pickRandomColor(&context->assets.sprites[ii].colors[nn]);
		}

		if(texture_initAsOffscreen(&context->assets.sprites[ii].texture,
					TEXTURE_QUALITY_HIGHEST,
					TEXTURE_RENDER_REPEAT,
					afalse,
					64, 64,
					&color) < 0) {
			return -1;
		}
	}

	log_logf(&context->engine.log, LOG_LEVEL_INFO,
			"finished loading assets in %0.6f seconds",
			time_getElapsedMusInSeconds(timing));

	return 0;
}

// game functions

static int game_xScreenCoordToGrid(Demo *context, int x)
{
	int divisor = 0;

	if(context == NULL) {
		DISPLAY_INVALID_ARGS;
		return 0;
	}

	divisor = (context->engine.canvas.screenWidth / 2);

	if(x > divisor) {
		return (x - divisor);
	}
	else if(x < divisor) {
		return ((divisor - x) * -1);
	}

	return 0;
}

static int game_yScreenCoordToGrid(Demo *context, int y)
{
	int divisor = 0;

	if(context == NULL) {
		DISPLAY_INVALID_ARGS;
		return 0;
	}

	divisor = (context->engine.canvas.screenHeight / 2);

	if(y > divisor) {
		return ((y - divisor) * -1);
	}
	else if(y < divisor) {
		return (divisor - y);
	}

	return 0;
}

static double game_calculateDistance(Sprite2d *spriteAlpha,
		Sprite2d *spriteBeta)
{
	int alphaX = 0;
	int alphaY = 0;
	int betaX = 0;
	int betaY = 0;
	double distance = 0.0;

	if((spriteAlpha == NULL) || (spriteBeta == NULL)) {
		DISPLAY_INVALID_ARGS;
		return 0.0;;
	}

	alphaX = (spriteAlpha->position.x + (spriteAlpha->position.width / 2));
	alphaY = (spriteAlpha->position.y + (spriteAlpha->position.height / 2));

	betaX = (spriteBeta->position.x + (spriteBeta->position.width / 2));
	betaY = (spriteBeta->position.y + (spriteBeta->position.height / 2));

	distance = sqrt(((alphaX - betaX) * (alphaX - betaX)) +
			((alphaY - betaY) * (alphaY - betaY)));

	return distance;
}

static aboolean game_isCollided(Sprite2d *spriteAlpha, Sprite2d *spriteBeta)
{
	int radiusAlpha = 0;
	int radiusBeta = 0;
	double distance = 0.0;

	if((spriteAlpha == NULL) || (spriteBeta == NULL)) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	radiusAlpha = ((spriteAlpha->position.width +
				spriteAlpha->position.height) / 4);

	radiusBeta = ((spriteBeta->position.width +
				spriteBeta->position.height) / 4);

	distance = game_calculateDistance(spriteAlpha, spriteBeta);

	if(distance < (double)((radiusAlpha + radiusBeta) - 2)) {
		return atrue;
	}

	return afalse;
}

// sprites functions

static int sprite_getAliveCount(Demo *context)
{
	int ii = 0;
	int result = 0;

	if(context == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	for(ii = 0; ii < DEMO_MAX_SPRITES; ii++) {
		if(context->assets.sprites[ii].isAlive) {
			result++;
		}
	}

	return result;
}

static int sprite_buildNewSprite(Demo *context, int xPosition, int yPosition)
{
	int ii = 0;
	int id = 0;
	int xMin = 0;
	int xMax = 0;
	int yMin = 0;
	int yMax = 0;
	double thrust = 0.0;

	if(context == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	id = -1;

	for(ii = 0; ii < DEMO_MAX_SPRITES; ii++) {
		if(!context->assets.sprites[ii].isAlive) {
			id = ii;
			break;
		}
	}

	if(id == -1) {
		return -1;
	}

	context->assets.sprites[id].isAlive = atrue;
	context->assets.sprites[id].rotation = 0.0;

	physics2d_reset(&context->assets.sprites[id].physics);

	physics2d_setThrust(&context->assets.sprites[id].physics,
				DEMO_SPRITES_STARTING_THRUST);

	// spawn algorithm - not in central screen area

if(afalse) { // not in use
	xMin = ((context->engine.canvas.screenWidth / 2) - 128);
	xMax = ((context->engine.canvas.screenWidth / 2) + 128);
	yMin = ((context->engine.canvas.screenHeight / 2) - 128);
	yMax = ((context->engine.canvas.screenHeight / 2) + 128);

	do {
		xPosition = (rand() % context->engine.canvas.screenWidth);
		yPosition = (rand() % context->engine.canvas.screenHeight);
	} while(((xPosition >= xMin) && (xPosition <= xMax)) ||
			((yPosition >= yMin) && (yPosition <= yMax)));
}

	// spawn algorithm - at edge of screen only

if(afalse) { // not in use
	if((xPosition == -1) && (yPosition == -1)) {
		if((rand() % 2) == 0) {
			xPosition = (rand() % context->engine.canvas.screenWidth);
			if((rand() % 2) == 0) {
				yPosition = (0 - 64);
			}
			else {
				yPosition = (context->engine.canvas.screenHeight + 64);
			}
		}
		else {
			if((rand() % 2) == 0) {
				xPosition = (0 - 64);
			}
			else {
				xPosition = (context->engine.canvas.screenWidth + 64);
			}
			yPosition = (rand() % context->engine.canvas.screenHeight);
		}
	}
}

	// spawn algorithm - random

	xPosition = (rand() % context->engine.canvas.screenWidth);
	yPosition = (rand() % context->engine.canvas.screenHeight);

	physics2d_setPosition(&context->assets.sprites[id].physics,
			xPosition, yPosition);

	sprite2d_resetDirection(&context->assets.sprites[id].sprite,
			(rand() % SPRITE2D_DIRECTIONS));

	thrust = ((double)((rand() % 100) + 64) / 100.0);

	physics2d_processDirectionalPhysics(
			&context->assets.sprites[id].physics,
			PHYSICS_DIRECTION_FORWARD,
			afalse,
			thrust);

	sprite2d_resetDirection(&context->assets.sprites[id].sprite,
			SPRITE2D_DIRECTION_EAST);

	log_logf(&context->engine.log, LOG_LEVEL_DEBUG,
			"constructed new sprite #%i at (%i, %i)",
			id,
			context->assets.sprites[id].sprite.position.x,
			context->assets.sprites[id].sprite.position.y);

	return id;
}

// rendering functions

static void render_loadingScreen(Demo *context, Canvas *canvas)
{
	// 3d objects

	canvas_toggleRenderMode3d(canvas);

	// 2d objects

	canvas_toggleRenderMode2d(canvas);

	// draw the loading screen

	texture_draw2d(context->assets.loadingScreen,
			((context->engine.canvas.screenWidth / 2) -
			 (context->assets.loadingScreen->width / 2)),
			((context->engine.canvas.screenHeight / 2) -
			 (context->assets.loadingScreen->height / 2)),
			context->assets.loadingScreen->width,
			context->assets.loadingScreen->height);

#if !defined(__ANDROID__)
	if(!DEMO_IS_PRODUCTION_BUILD) {
		CGI_COLOR_WHITE;

		font_draw2d(&context->engine.font, canvas, 4, 0, "Loading...");
	}
#endif // !__ANDROID__
}

static void render_playing(Demo *context, Canvas *canvas)
{
	aboolean isTargeted = afalse;
	int ii = 0;
	int nn = 0;
	int number = 0;
	int aliveTime = 0;
	double radius = 0.0;

	Color color;

	// 3d objects

	canvas_toggleRenderMode3d(canvas);

	// 2d objects

	canvas_toggleRenderMode2d(canvas);

	for(ii = 0; ii < DEMO_MAX_SPRITES; ii++) {
		if((!context->assets.sprites[ii].isAlive) ||
				(!context->assets.sprites[ii].isFirstPass)) {
			continue;
		}

		texture_toggleRender2dToTexture(&context->assets.sprites[ii].texture);

		for(nn = 0; nn < 5; nn++) {
			radius = ((context->assets.sprites[ii].texture.width +
						context->assets.sprites[ii].texture.height) /
					4);

			radius *= (1.0 - ((double)nn / 5));

			draw2d_fillCircle(canvas,
					&context->assets.sprites[ii].colors[nn],
					(0 +
					 ((context->assets.sprites[ii].texture.width / 2) -
					  (int)radius)),
					(0 +
					 ((context->assets.sprites[ii].texture.height / 2) -
					  (int)radius)),
					radius);
		}

		texture_endRenderToTexture(
				&context->assets.sprites[ii].texture,
				canvas);

		sprite2d_setTexture(&context->assets.sprites[ii].sprite,
				SPRITE2D_DIRECTION_EAST,
				&context->assets.sprites[ii].texture);

		texture_setBlend(&context->assets.sprites[ii].texture,
				TEXTURE_BLEND_SOURCE_SRC_ALPHA,
				TEXTURE_BLEND_DEST_ONE_MINUS_SRC_ALPHA);

		sprite2d_resize(&context->assets.sprites[ii].sprite, 40, 40);

		context->assets.sprites[ii].isFirstPass = afalse;
	}

	// draw the sprites

	for(ii = 0; ii < DEMO_MAX_SPRITES; ii++) {
		if(!context->assets.sprites[ii].isAlive) {
			continue;
		}
/*
		context->assets.sprites[ii].sprite.currentRotation =
				context->assets.sprites[ii].rotation;

		context->assets.sprites[ii].sprite.render2d(
				&context->assets.sprites[ii].sprite);

		context->assets.sprites[ii].sprite.currentRotation = 0.0;
*/

/*		{
			color.r = CGI_COLOR_BLACK_RED;
			color.g = CGI_COLOR_BLACK_GREEN;
			color.b = CGI_COLOR_BLACK_BLUE;
			color.a = CGI_COLOR_BLACK_ALPHA;

			draw2d_fillCircle(canvas, &color,
					context->assets.sprites[ii].sprite.position.x,
					context->assets.sprites[ii].sprite.position.y,
					((context->assets.sprites[ii].sprite.position.width +
					  context->assets.sprites[ii].sprite.position.height) /
					 4));
		}*/

		if(!context->assets.sprites[ii].isFirstPass) {
			context->assets.sprites[ii].sprite.render2d(
					&context->assets.sprites[ii].sprite);
		}

		// draw the sprite collision map

		if(DEMO_DRAW_COLLISION_MAP) {
			color.r = CGI_COLOR_BLUE_RED;
			color.g = CGI_COLOR_BLUE_GREEN;
			color.b = CGI_COLOR_BLUE_BLUE;
			color.a = CGI_COLOR_BLUE_ALPHA;

			draw2d_rectangle(canvas, &color,
					context->assets.sprites[ii].sprite.position.x,
					context->assets.sprites[ii].sprite.position.y,
					context->assets.sprites[ii].sprite.position.width,
					context->assets.sprites[ii].sprite.position.height);
		}

/*		{
			// debug troid location

			char buffer[8192];

			snprintf(buffer, (int)(sizeof(buffer) -1),
					"Troid[%03i](%i, %i)\n",
					ii,
					context->assets.sprites[ii].sprite.position.x,
					context->assets.sprites[ii].sprite.position.y);

			CGI_COLOR_WHITE;

			draw2d_font(canvas,
					((context->assets.sprites[ii].sprite.position.x +
					  context->assets.sprites[ii].sprite.xOffset) + 2),
					((context->assets.sprites[ii].sprite.position.y +
					  context->assets.sprites[ii].sprite.yOffset) - 22),
					buffer,
					&context->engine.font);
		}*/
	}

	// draw a cross-hatch at the current mouse position

	if(!DEMO_IS_PRODUCTION_BUILD) {
		if(context->mouse.isDown) {
			color.r = CGI_COLOR_RED_RED;
			color.g = CGI_COLOR_RED_GREEN;
			color.b = CGI_COLOR_RED_BLUE;
			color.a = CGI_COLOR_RED_ALPHA;
		}
		else {
			color.r = CGI_COLOR_BLUE_RED;
			color.g = CGI_COLOR_BLUE_GREEN;
			color.b = CGI_COLOR_BLUE_BLUE;
			color.a = CGI_COLOR_BLUE_ALPHA;
		}

		draw2d_line(canvas, &color,
				(context->mouse.xPosition - 10), context->mouse.yPosition,
				(context->mouse.xPosition + 10), context->mouse.yPosition);
		draw2d_line(canvas, &color,
				context->mouse.xPosition, (context->mouse.yPosition - 10),
				context->mouse.xPosition, (context->mouse.yPosition + 10));
	}

#if !defined(__ANDROID__)

	CGI_COLOR_WHITE;

	font_draw2d(&context->engine.font, canvas, 4, 0,
			"Framerate: %0.2f", canvas->framerate);

/*
	for(ii = 0; ii < DEMO_MAX_SPRITES; ii++) {
		if(context->assets.sprites[ii].isAlive) {
			font_draw2d(&context->engine.font, canvas, 4, (16 * (ii + 2)),
					"Sprite #%03i { (%03i, %03i) c%i }",
					ii,
					context->assets.sprites[ii].sprite.position.x,
					context->assets.sprites[ii].sprite.position.y,
					(int)context->assets.sprites[ii].isAlive);
		}
	}
*/
#endif // !__ANDROID__
}

// gameplay functions

static void gameplay_processPlaying(Demo *context)
{
	int ii = 0;
	int nn = 0;
	int jj = 0;
	int id = 0;
	int length = 0;
	int xPosition = 0;
	int yPosition = 0;
	int aliveTime = 0;
	double rotation = 0.0;
	double elapsedTime = 0.0;

	static int aliveTimestamp = 0;
	static double processTimestamp = 0.0;

	if(context == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	if(processTimestamp == 0.0) {
		processTimestamp = time_getTimeMus();
	}
	else {
		elapsedTime = time_getElapsedMusInSeconds(processTimestamp);
		processTimestamp = time_getTimeMus();
	}

	rotation = (36.0 * elapsedTime);

	// process sprites

	for(ii = 0; ii < DEMO_MAX_SPRITES; ii++) {
		if(!context->assets.sprites[ii].isAlive) {
			continue;
		}

		if((ii % 2) == 0) {
			context->assets.sprites[ii].rotation += rotation;
		}
		else {
			context->assets.sprites[ii].rotation -= rotation;
		}

		if(((ii % 2) == 0) &&
				(context->assets.sprites[ii].rotation >= 360.0)) {
			context->assets.sprites[ii].rotation = 0.0;
		}
		else if(((ii % 2) != 0) &&
				(context->assets.sprites[ii].rotation < 0.0)) {
			context->assets.sprites[ii].rotation = 360.0;
		}

		physics2d_processGameplay(&context->assets.sprites[ii].physics);

		if(physics2d_getX(&context->assets.sprites[ii].physics) >
				context->engine.canvas.screenWidth) {
			physics2d_setPosition(&context->assets.sprites[ii].physics,
					(0 - context->assets.sprites[ii].sprite.position.width),
					physics2d_getY(&context->assets.sprites[ii].physics));
		}
		else if(physics2d_getX(&context->assets.sprites[ii].physics) <
				(0 - context->assets.sprites[ii].sprite.position.width)) {
			physics2d_setPosition(&context->assets.sprites[ii].physics,
					context->engine.canvas.screenWidth,
					physics2d_getY(&context->assets.sprites[ii].physics));
		}

		if(physics2d_getY(&context->assets.sprites[ii].physics) >
				context->engine.canvas.screenHeight) {
			physics2d_setPosition(&context->assets.sprites[ii].physics,
					physics2d_getX(&context->assets.sprites[ii].physics),
					(0 - context->assets.sprites[ii].sprite.position.height));
		}
		else if(physics2d_getY(&context->assets.sprites[ii].physics)  <
				(0 - context->assets.sprites[ii].sprite.position.height)) {
			physics2d_setPosition(&context->assets.sprites[ii].physics,
					physics2d_getX(&context->assets.sprites[ii].physics),
					context->engine.canvas.screenHeight);
		}
	}
}


// define demo public functions

int demo_init(Demo *context)
{
	if(context == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	memset(context, 0, (int)(sizeof(Demo)));

	context->state = DEMO_STATE_INIT;
	context->name = strdup("Demo");

	return 0;
}

int demo_free(Demo *context)
{
	if(context == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(context->name != NULL) {
		free(context->name);
	}

	if(context->state < DEMO_STATE_INIT) {
		return 0;
	}

	texture_freePtr(context->assets.loadingScreen);
	texture_freePtr(context->assets.spriteOne);
	texture_freePtr(context->assets.spriteTwo);
	texture_freePtr(context->assets.spriteThree);

	return 0;
}

int demo_handleMouseEvents(Demo *context, AsgardMouseEvent *mouseEvent)
{
	int ii = 0;

	if((context == NULL) || (mouseEvent == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	switch(context->state) {
		case DEMO_STATE_GAME_PLAYING:
			/*
			 * TODO: this
			 */
			break;

		case DEMO_STATE_INIT:
		case DEMO_STATE_LOADING:
		case DEMO_STATE_GAME_START:
			return 0;

		case DEMO_STATE_ERROR:
			return -1;
	}

	if((mouseEvent->eventType == EVENT_MOUSE_PRESSED) ||
			(mouseEvent->eventType == EVENT_MOUSE_RELEASED)) {
		if(mouseEvent->eventType == EVENT_MOUSE_PRESSED) {
			context->mouse.isDown = atrue;
		}
		else {
			context->mouse.isDown = afalse;
		}

		context->mouse.xPosition = mouseEvent->xPosition;
		context->mouse.yPosition = mouseEvent->yPosition;
	}

	return 0;
}

int demo_render(Demo *context)
{
	Canvas *canvas = NULL;

	if(context == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if((context->state == DEMO_STATE_INIT) ||
			(context->state == DEMO_STATE_ERROR)) {
		return 0;
	}

	canvas = &(context->engine.canvas);

	// begin render

	canvas_beginRender(canvas);

	switch(context->state) {
		case DEMO_STATE_LOADING:
			render_loadingScreen(context, canvas);
			break;

		case DEMO_STATE_GAME_START:
			render_loadingScreen(context, canvas);
			break;

		case DEMO_STATE_GAME_PLAYING:
			render_playing(context, canvas);
			break;
	}

	// end render

	canvas_endRender(canvas);

	return 0;
}

int demo_processGamepley(Demo *context)
{
	int ii = 0;
#if defined(__ANDROID__)
	static int initCounter = 0;
#endif // __ANDROID__

	if(context == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	switch(context->state) {
		case DEMO_STATE_INIT:
			if(load_loadingScreen(context) < 0) {
				context->state = DEMO_STATE_ERROR;
			}
			else {
				context->state = DEMO_STATE_LOADING;

				log_logf(&context->engine.log, LOG_LEVEL_INFO,
						"init complete, set state to LOADING");
			}
			break;

		case DEMO_STATE_LOADING:
#if defined(__ANDROID__)
			initCounter++;
			if(initCounter < 8) {
				break;
			}
#endif // __ANDROID__
			if(load_gameAssets(context) < 0) {
				context->state = DEMO_STATE_ERROR;
			}
			else {
				context->state = DEMO_STATE_GAME_START;

				log_logf(&context->engine.log, LOG_LEVEL_INFO,
						"load complete, set state to START");
			}
			break;

		case DEMO_STATE_GAME_START:
			log_logf(&context->engine.log, LOG_LEVEL_INFO,
					"start beginning, building sprites");

			for(ii = 0; ii < DEMO_MAX_SPRITES; ii++) {
				sprite_buildNewSprite(context, -1, -1);
			}

			context->state = DEMO_STATE_GAME_PLAYING;

			log_logf(&context->engine.log, LOG_LEVEL_INFO,
					"start complete, set state to PLAYING");
			break;

		case DEMO_STATE_GAME_PLAYING:
			gameplay_processPlaying(context);
			break;

		case DEMO_STATE_ERROR:
			return -1;
	}

	return 0;
}

int demo_ioFunction(void *engineContext, void *gameContext, void *argument)
{
	AsgardEvent *event = NULL;
	AsgardKeyboardEvent *keyboardEvent = NULL;
	AsgardMouseEvent *mouseEvent = NULL;
	Engine *engine = NULL;
	Demo *context = NULL;

	engine = (Engine *)engineContext;
	context = (Demo *)gameContext;
	event = (AsgardEvent *)argument;

	switch(event->eventType) {
		/*
		 * handle keyboard events
		 */

		case EVENT_TYPE_KEYBOARD:
			keyboardEvent = (AsgardKeyboardEvent *)event->event;

			log_logf(&context->engine.log, LOG_LEVEL_DEBUG,
					"[%s->%s()] keyboard event #%i",
					engine->name, __FUNCTION__, keyboardEvent->eventType);

			break;

		/*
		 * handle mouse events
		 */

		case EVENT_TYPE_MOUSE:
			mouseEvent = (AsgardMouseEvent *)event->event;

			log_logf(&context->engine.log, LOG_LEVEL_DEBUG,
					"[%s->%s()] mouse event #%i",
					engine->name, __FUNCTION__, mouseEvent->eventType);

			demo_handleMouseEvents(context, mouseEvent);

			break;
	}

	return 0;
}

int demo_renderFunction(void *engineContext, void *gameContext,
		void *argument)
{
	Engine *engine = NULL;
	Demo *context = NULL;

	engine = (Engine *)engineContext;
	context = (Demo *)gameContext;

	return demo_render(context);
}

int demo_gameplayFunction(void *engineContext, void *gameContext,
		void *argument)
{
	Engine *engine = NULL;
	Demo *context = NULL;

	engine = (Engine *)engineContext;
	context = (Demo *)gameContext;

	return demo_processGamepley(context);
}

