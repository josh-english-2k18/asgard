/*
 * troids.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Troids (an Asteroid clone) multiplatform video game demo.
 *
 * Written by Josh English.
 */


// preprocessor directives

#include "core/core.h"
#include "asgard/asgard.h"
#define _APPS_TROIDS_COMPONENT
#include "apps/troids/troids.h"
#define _ASGARD_CORE_PLUGIN_COMPONENT
#include "asgard/core/plugin/androidapi.h"


// define troids private functions

// initialization functions

static char *load_downloadAsset(Troids *context, char *filename,
		int *downloadLength)
{
	int sd = 0;
	int valueLength = 0;
	char *value = NULL;

	const char *REQUEST_TEMPLATE = ""
		"GET /%s/%s HTTP/1.1\x0d\x0a"
		"Host: %s\x0d\x0a"
		"User-Agent: Asgard Game Engine/%s (%s; en-US) Asgard/%s\x0d\x0a"
		"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,"
			"*/*;q=0.8\x0d\x0a"
		"Accept-Language: en-us,en;q=0.5\x0d\x0a"
		"Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7\x0d\x0a"
		"Keep-Alive: 115\x0d\x0a"
		"Connection: keep-alive\x0d\x0a"
		"Cache-Control: max-age=0\x0d\x0a"
		"\x0d\x0a";

	Socket socket;

	if((context == NULL) || (filename == NULL) || (downloadLength == NULL)) {
		if(downloadLength != NULL) {
			*downloadLength = 0;
		}
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	*downloadLength = 0;

	log_logf(&context->engine.log, LOG_LEVEL_DEBUG,
			"attempting to download asset '%s' from '%s'",
			filename, TROIDS_WEB_IP);

	// create client socket

	if(socket_init(&socket,
				SOCKET_MODE_CLIENT,
				SOCKET_PROTOCOL_TCPIP,
				TROIDS_WEB_IP,
				80
				) < 0) {
		log_logf(&context->engine.log, LOG_LEVEL_ERROR,
				"failed to initialize socket connection to '%s'",
				TROIDS_WEB_IP);
		return NULL;
	}

	// connect the client socket to the server

	if(socket_open(&socket) < 0) {
		log_logf(&context->engine.log, LOG_LEVEL_ERROR,
				"failed to open socket connection to '%s'",
				TROIDS_WEB_IP);
		return NULL;
	}

	if((sd = socket_getSocketDescriptor(&socket)) < 0) {
		log_logf(&context->engine.log, LOG_LEVEL_ERROR,
				"failed to get socket descriptor from socket connection '%s'",
				TROIDS_WEB_IP);
		return NULL;
	}

	// send message from client to server

	valueLength = (strlen(REQUEST_TEMPLATE) + 512);
	value = (char *)malloc(sizeof(char) * (valueLength + 1));

	valueLength = snprintf(value, valueLength, REQUEST_TEMPLATE,
			TROIDS_WEB_BASE_URL,
			filename,
			TROIDS_WEB_IP,
			ASGARD_VERSION,
			ASGARD_VERSION,
			ASGARD_DATE);

	log_logf(&context->engine.log, LOG_LEVEL_DEBUG,
			"download URL is http://%s/%s/%s",
			TROIDS_WEB_IP, TROIDS_WEB_BASE_URL, filename);

	if(socket_send(&socket, sd, value, valueLength) < 0) {
		log_logf(&context->engine.log, LOG_LEVEL_ERROR,
				"failed to HTTP send on socket connection to '%s'",
				TROIDS_WEB_IP);
		return NULL;
	}

	free(value);

	// read server socket transmission

	value = http_protocol_receive(NULL, &socket, sd, &valueLength);

	// cleanup

	if(socket_close(&socket) < 0) {
		log_logf(&context->engine.log, LOG_LEVEL_ERROR,
				"failed to close socket connection to '%s'",
				TROIDS_WEB_IP);
		return NULL;
	}

	socket_free(&socket);

	// return results

	log_logf(&context->engine.log, LOG_LEVEL_DEBUG,
			"downloaded asset '%s' from '%s'",
			filename, TROIDS_WEB_IP);

	*downloadLength = valueLength;

	return value;
}

static int load_createWebTexture(Troids *context, char *filename,
		Texture *texture)
{
	int ptrLength = 0;
	int bufferLength = 0;
	char *ptr = NULL;
	char *buffer = NULL;
	char *copy = NULL;

	Targa targa;

	const char *searchString = "\x0d\x0a\x0d\x0a";

	if((context == NULL) || (filename == NULL) || (texture == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(((buffer = load_downloadAsset(context, filename,
						&bufferLength)) == NULL) ||
			(bufferLength < 1)) {
		if(buffer != NULL) {
			free(buffer);
		}
		return -1;
	}

	if((ptr = strstr(buffer, searchString)) == NULL) {
		log_logf(&context->engine.log, LOG_LEVEL_ERROR,
				"failed to locate end of HTTP header in texture");
		free(buffer);
		return -1;
	}

	ptr += 4;

	ptrLength = (bufferLength - (int)(ptr - buffer));

	copy = strndup(ptr, ptrLength);

	free(buffer);

	buffer = copy;
	bufferLength = ptrLength;

	log_logf(&context->engine.log, LOG_LEVEL_DEBUG,
			"read %i bytes from texture '%s' into RGBA buffer",
			bufferLength, filename);

	targa_init(&targa);

	if(targa_loadFromData(&targa, (unsigned char *)buffer, bufferLength) < 0) {
		log_logf(&context->engine.log, LOG_LEVEL_ERROR,
				"failed to decode TGA from texture '%s'", filename);
		targa_free(&targa);
		return -1;
	}

	if(texture_initFromRgbaBuffer(texture,
				TEXTURE_QUALITY_MEDIUM,
				TEXTURE_RENDER_REPEAT,
				afalse,
				(char *)targa.image,
				targa.imageLength,
				targa.width,
				targa.height) < 0) {
		log_logf(&context->engine.log, LOG_LEVEL_ERROR,
				"failed to initialize TGA texture from '%s' "
				"decoded RGBA buffer", filename);
		targa_free(&targa);
		return -1;
	}

	log_logf(&context->engine.log, LOG_LEVEL_INFO,
			"loaded %ix%i TGA texture '%s' from RGBA buffer as #%i",
			targa.width, targa.height, filename, (int)texture->id);

	targa_free(&targa);

	return 0;
}

static Texture *load_createTgaTexture(Troids *context, char *filename)
{
	Texture *result = NULL;

	if((context == NULL) || (filename == NULL)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	result = (Texture *)malloc(sizeof(Texture));

	if(TROIDS_IS_WEB_ENABLED) {
		if(load_createWebTexture(context, filename, result) < 0) {
			log_logf(&context->engine.log, LOG_LEVEL_ERROR,
					"failed to init texture '%s' from HTTP", filename);
			texture_freePtr(result);
			return NULL;
		}
	}
	else {
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
	}

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

static int load_loadingScreen(Troids *context)
{
	Texture *texture = NULL;

#if !defined(__ANDROID__)
	char *AssetList[] = {
		"assets/textures/troids/loading.screen.tga",
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

static int load_gameAssets(Troids *context)
{
	int ii = 0;
	int nn = 0;
	double timing = 0.0;
	char buffer[8192];

	Texture *texture = NULL;

#if !defined(__ANDROID__)
	char *AssetList[] = {
		"assets/textures/troids/splash.screen.tga",
		"assets/textures/troids/game.over.screen.tga",
		"assets/textures/troids/night.sky.tga",
		"assets/textures/troids/background.tga",
		"assets/textures/troids/circle.blue.tga",
		"assets/textures/troids/fighter.outline.tga",
		"assets/textures/troids/asteroid.big.tga",
		"assets/textures/troids/bullet.tga",
		"assets/textures/troids/missile.tga",
		"assets/textures/troids/gun.radius.tga",
		"assets/textures/troids/shield.tga",
		"assets/textures/troids/powerup.gun.tga",
		"assets/textures/troids/powerup.missile.tga",
		"assets/textures/troids/powerup.shield.tga",
		"assets/textures/troids/instruction.screen.tga",
		"assets/textures/troids/asteroid.medium.tga",
		"assets/textures/troids/asteroid.small.tga",
		"assets/textures/troids/fighter.outline.faded.tga",
		"assets/textures/troids/bullet.upgrade.tga",
		"assets/textures/troids/missile.upgrade.tga",
		"assets/textures/troids/thrusters.tga",
		"assets/textures/troids/thrusters.medium.tga",
		"assets/textures/troids/thrusters.small.tga",
		"assets/textures/troids/gun.targeted.tga",
		NULL
	};
#else // __ANDROID__
	char *AssetList[] = {
		"assets/textures/splash.screen.tga",
		"assets/textures/game.over.screen.tga",
		"assets/textures/night.sky.tga",
		"assets/textures/background.tga",
		"assets/textures/circle.blue.tga",
		"assets/textures/fighter.outline.tga",
		"assets/textures/asteroid.big.tga",
		"assets/textures/bullet.tga",
		"assets/textures/missile.tga",
		"assets/textures/gun.radius.tga",
		"assets/textures/shield.tga",
		"assets/textures/powerup.gun.tga",
		"assets/textures/powerup.missile.tga",
		"assets/textures/powerup.shield.tga",
		"assets/textures/instruction.screen.tga",
		"assets/textures/asteroid.medium.tga",
		"assets/textures/asteroid.small.tga",
		"assets/textures/fighter.outline.faded.tga",
		"assets/textures/bullet.upgrade.tga",
		"assets/textures/missile.upgrade.tga",
		"assets/textures/thrusters.tga",
		"assets/textures/thrusters.medium.tga",
		"assets/textures/thrusters.small.tga",
		"assets/textures/gun.targeted.tga",
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

	if((context->assets.splashScreen = load_createTgaTexture(context,
					AssetList[0])) == NULL) {
		return -1;
	}

	if((context->assets.gameOverScreen = load_createTgaTexture(context,
					AssetList[1])) == NULL) {
		return -1;
	}

	if((context->assets.cube = load_createTgaTexture(context,
					AssetList[2])) == NULL) {
		return -1;
	}

	if((context->assets.background = load_createTgaTexture(context,
					AssetList[3])) == NULL) {
		return -1;
	}

	if((context->assets.circleBlue = load_createTgaTexture(context,
					AssetList[4])) == NULL) {
		return -1;
	}

	if((context->assets.fighterOutline = load_createTgaTexture(context,
					AssetList[5])) == NULL) {
		return -1;
	}

	if((context->assets.asteroidBig = load_createTgaTexture(context,
					AssetList[6])) == NULL) {
		return -1;
	}

	if((context->assets.bullet = load_createTgaTexture(context,
					AssetList[7])) == NULL) {
		return -1;
	}

	if((context->assets.missile = load_createTgaTexture(context,
					AssetList[8])) == NULL) {
		return -1;
	}

	if((context->assets.gunRadius = load_createTgaTexture(context,
					AssetList[9])) == NULL) {
		return -1;
	}

	if((context->assets.shield = load_createTgaTexture(context,
					AssetList[10])) == NULL) {
		return -1;
	}

	if((context->assets.instructionScreen = load_createTgaTexture(context,
					AssetList[14])) == NULL) {
		return -1;
	}

	if((context->assets.asteroidMedium = load_createTgaTexture(context,
					AssetList[15])) == NULL) {
		return -1;
	}

	if((context->assets.asteroidSmall = load_createTgaTexture(context,
					AssetList[16])) == NULL) {
		return -1;
	}

	if((context->assets.fighterOutlineFaded = load_createTgaTexture(context,
					AssetList[17])) == NULL) {
		return -1;
	}

	if((context->assets.bulletUpgrade = load_createTgaTexture(context,
					AssetList[18])) == NULL) {
		return -1;
	}

	if((context->assets.missileUpgrade = load_createTgaTexture(context,
					AssetList[19])) == NULL) {
		return -1;
	}

	if((context->assets.thrusters = load_createTgaTexture(context,
					AssetList[20])) == NULL) {
		return -1;
	}

	if((context->assets.thrustersMedium = load_createTgaTexture(context,
					AssetList[21])) == NULL) {
		return -1;
	}

	if((context->assets.thrustersSmall = load_createTgaTexture(context,
					AssetList[22])) == NULL) {
		return -1;
	}

	if((context->assets.gunTargeted = load_createTgaTexture(context,
					AssetList[23])) == NULL) {
		return -1;
	}

	// initialize & load sprite textures

	context->assets.fighterData.isAlive = atrue;
	context->assets.fighterData.hasBullets = afalse;
	context->assets.fighterData.hasMissiles = afalse;
	context->assets.fighterData.hasShields = afalse;
	context->assets.fighterData.shieldStrength = 0;
	context->assets.fighterData.respawnsRemaining = 3;
	context->assets.fighterData.currentThrust =
		TROIDS_ASTEROIDS_STARTING_THRUST;

	sprite2d_init(&context->assets.fighter,
			"Fighter",
			((context->engine.canvas.screenWidth - 56) / 2),
			((context->engine.canvas.screenHeight - 56) / 2),
			56, 56,
			&context->engine.canvas, &context->engine);

	sprite2d_setRotateTimeSeconds(&context->assets.fighter, 0.0625);
	sprite2d_setDebugMode(&context->assets.fighter, afalse);
	sprite2d_setIsOutline(&context->assets.fighter, afalse);
	sprite2d_setIsSelectable(&context->assets.fighter, afalse);

	for(ii = 0; ii < SPRITE2D_DIRECTIONS; ii++) {
#if !defined(__ANDROID__)
		snprintf(buffer, (sizeof(buffer) - 1),
				"assets/textures/sprites/fighter/fighter.%02i.tga", ii);
#else // __ANDROID__
		snprintf(buffer, (sizeof(buffer) - 1),
				"assets/textures/sprites/fighter.%02i.tga", ii);
#endif // !__ANDROID__

		if((texture = load_createTgaTexture(context, buffer)) == NULL) {
			return -1;
		}

		sprite2d_setTexture(&context->assets.fighter, (Sprite2dDirection)ii,
				texture);
	}

	// initialize 2d physics engine

	physics2d_init(&context->assets.fighterPhysics, &context->assets.fighter);
	physics2d_setHasFriction(&context->assets.fighterPhysics, atrue);
	physics2d_setFriction(&context->assets.fighterPhysics, 0.98);
	physics2d_setThrust(&context->assets.fighterPhysics,
			TROIDS_FIGHTER_THRUST);

	// initialize asteroids

	for(ii = 0; ii < TROIDS_MAX_ASTEROIDS; ii++) {
		context->assets.asteroids[ii].isAlive = afalse;

		sprite2d_init(&context->assets.asteroids[ii].sprite,
				"Asteroid",
				-100, -100,
				64, 64,
				&context->engine.canvas, &context->engine);

		sprite2d_setDebugMode(&context->assets.asteroids[ii].sprite, afalse);
		sprite2d_setIsOutline(&context->assets.asteroids[ii].sprite, afalse);
		sprite2d_setIsSelectable(&context->assets.asteroids[ii].sprite, afalse);

		physics2d_init(&context->assets.asteroids[ii].physics,
				&context->assets.asteroids[ii].sprite);
		physics2d_setHasFriction(&context->assets.asteroids[ii].physics,
				afalse);
		physics2d_setFriction(&context->assets.asteroids[ii].physics, 0.999);
		physics2d_setThrust(&context->assets.asteroids[ii].physics,
				context->assets.fighterData.currentThrust);
	}

	// initialize powerups

	for(ii = 0; ii < TROIDS_POWERUP_TYPE_LENGTH; ii++) {
		context->assets.powerups[ii].isAlive = afalse;
		context->assets.powerups[ii].type = (TroidsPowerupTypes)ii;

		sprite2d_init(&context->assets.powerups[ii].sprite,
				"Asteroid",
				-100, -100,
				64, 64,
				&context->engine.canvas, &context->engine);

		sprite2d_setDebugMode(&context->assets.powerups[ii].sprite, afalse);
		sprite2d_setIsOutline(&context->assets.powerups[ii].sprite, afalse);
		sprite2d_setIsSelectable(&context->assets.powerups[ii].sprite, afalse);
	
		if((texture = load_createTgaTexture(context,
						AssetList[(ii + 11)])) == NULL) {
			return -1;
		}

		sprite2d_setTexture(&context->assets.powerups[ii].sprite,
				SPRITE2D_DIRECTION_EAST,
				texture);

		physics2d_init(&context->assets.powerups[ii].physics,
				&context->assets.powerups[ii].sprite);
		physics2d_setHasFriction(&context->assets.powerups[ii].physics,
				afalse);
		physics2d_setFriction(&context->assets.powerups[ii].physics, 0.999);
		physics2d_setThrust(&context->assets.powerups[ii].physics,
				context->assets.fighterData.currentThrust);
	}

	// initialize fighter & missile explosion animation sequence

	animation2d_init(&context->assets.fighterAnimation,
			"FighterExplosion",
			0, 0, 128, 128,
			&context->engine);

	animation2d_setAnimationTime(&context->assets.fighterAnimation,
			TROIDS_ANIMATION_TIME);

	animation2d_init(&context->assets.missileAnimation,
			"FighterExplosion",
			0, 0, 128, 128,
			&context->engine);

	animation2d_setAnimationTime(&context->assets.missileAnimation,
			TROIDS_ANIMATION_TIME);

	for(ii = 0; ii < 30; ii++) {
#if !defined(__ANDROID__)
		snprintf(buffer, (sizeof(buffer) - 1),
				"assets/textures/animations/explosion01/explosion_%02i.tga",
				(ii + 1));
#else // __ANDROID__
		snprintf(buffer, (sizeof(buffer) - 1),
				"assets/textures/animations/explosion_%02i.tga",
				(ii + 1));
#endif // !__ANDROID__

		if((texture = load_createTgaTexture(context, buffer)) == NULL) {
			return -1;
		}

		animation2d_addFrameTexture(&context->assets.fighterAnimation, texture);
		animation2d_addFrameTexture(&context->assets.missileAnimation, texture);
	}

	// initialize asteroids animation sequence

	for(ii = 0; ii < TROIDS_MAX_ASTEROIDS; ii++) {
		animation2d_init(&context->assets.asteroids[ii].animation,
				"AsteroidsExplosion",
				0, 0, 128, 128,
				&context->engine);

		animation2d_setAnimationTime(&context->assets.asteroids[ii].animation,
				TROIDS_ANIMATION_TIME);
	}

	for(ii = 0; ii < 16; ii++) {
#if !defined(__ANDROID__)
		snprintf(buffer, (sizeof(buffer) - 1),
				"assets/textures/animations/explosion02/explosion.%02i.tga",
				ii);
#else // __ANDROID__
		snprintf(buffer, (sizeof(buffer) - 1),
				"assets/textures/animations/explosion.%02i.tga",
				ii);
#endif // !__ANDROID__

		if((texture = load_createTgaTexture(context, buffer)) == NULL) {
			return -1;
		}

		for(nn = 0; nn < TROIDS_MAX_ASTEROIDS; nn++) {
			animation2d_addFrameTexture(
					&context->assets.asteroids[nn].animation,
					texture);
		}
	}

	// load number textures

	for(ii = 0; ii < 10; ii++) {
#if !defined(__ANDROID__)
		snprintf(buffer, (sizeof(buffer) - 1),
				"assets/textures/troids/numbers/number.%i.tga", ii);
#else // __ANDROID__
		snprintf(buffer, (sizeof(buffer) - 1),
				"assets/textures/numbers/number.%i.tga", ii);
#endif // !__ANDROID__

		if((texture = load_createTgaTexture(context, buffer)) == NULL) {
			return -1;
		}

		context->assets.numbers[ii] = texture;
	}

	// initialize bullets

	for(ii = 0; ii < TROIDS_MAX_BULLETS; ii++) {
		context->assets.bullets[ii].isAlive = afalse;

		sprite2d_init(&context->assets.bullets[ii].sprite,
				"Bullet",
				-100, -100,
				10, 10,
				&context->engine.canvas, &context->engine);

		sprite2d_setDebugMode(&context->assets.bullets[ii].sprite, afalse);
		sprite2d_setIsOutline(&context->assets.bullets[ii].sprite, afalse);
		sprite2d_setIsSelectable(&context->assets.bullets[ii].sprite, afalse);

		sprite2d_setTexture(&context->assets.bullets[ii].sprite,
				SPRITE2D_DIRECTION_EAST,
				context->assets.bullet);

		physics2d_init(&context->assets.bullets[ii].physics,
				&context->assets.bullets[ii].sprite);
		physics2d_setHasFriction(&context->assets.bullets[ii].physics,
				afalse);
		physics2d_setFriction(&context->assets.bullets[ii].physics, 0.999);
		physics2d_setThrust(&context->assets.bullets[ii].physics, 0.002);
	}

	// initialize missiles

	for(ii = 0; ii < TROIDS_MAX_MISSILES; ii++) {
		context->assets.missiles[ii].isAlive = afalse;

		sprite2d_init(&context->assets.missiles[ii].sprite,
				"Missile",
				-100, -100,
				36, 36,
				&context->engine.canvas, &context->engine);

		sprite2d_setDebugMode(&context->assets.missiles[ii].sprite, afalse);
		sprite2d_setIsOutline(&context->assets.missiles[ii].sprite, afalse);
		sprite2d_setIsSelectable(&context->assets.missiles[ii].sprite, afalse);

		sprite2d_setTexture(&context->assets.missiles[ii].sprite,
				SPRITE2D_DIRECTION_EAST,
				context->assets.missile);

		physics2d_init(&context->assets.missiles[ii].physics,
				&context->assets.missiles[ii].sprite);
		physics2d_setHasFriction(&context->assets.missiles[ii].physics,
				afalse);
		physics2d_setFriction(&context->assets.missiles[ii].physics, 0.999);
		physics2d_setThrust(&context->assets.missiles[ii].physics, 0.001);
	}

	log_logf(&context->engine.log, LOG_LEVEL_INFO,
			"finished loading assets in %0.6f seconds",
			time_getElapsedMusInSeconds(timing));

	return 0;
}

// game functions

static int game_xScreenCoordToGrid(Troids *context, int x)
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

static int game_yScreenCoordToGrid(Troids *context, int y)
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

// fighter sprite functions

static void fighter_calculateTurn(Troids *context, int mouseX, int mouseY)
{
	int localX = 0;
	int localY = 0;
	int destination = 0;
	double angle = 0.0;
	double distance = 0.0;

	if((context == NULL) || (mouseX < 0) || (mouseY < 0)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	localX = (context->assets.fighter.position.x +
			(context->assets.fighter.position.width / 2));
	localY = (context->assets.fighter.position.y +
			(context->assets.fighter.position.height / 2));

	distance = sqrt(((mouseX - localX) * (mouseX - localX)) +
			((mouseY - localY) * (mouseY - localY)));

	context->assets.fighterData.distance = distance;

	angle = math_RadiansToDegrees(
				atan2(
					(game_yScreenCoordToGrid(context, mouseY) -
					 game_yScreenCoordToGrid(context, localY)
					),
					(game_xScreenCoordToGrid(context, mouseX) -
					 game_xScreenCoordToGrid(context, localX)
					)
				));

	if(angle < 0.0) {
		angle += 360.0;
	}

	angle = (360.0 - (angle - 90.0));

	if(angle < 0.0) {
		angle += 360.0;
	}
	else if(angle >= 360.0) {
		angle -= 360.0;
	}

	context->assets.fighterData.angle = angle;

	destination = (int)(angle / 22.5);

	if(destination == context->assets.fighter.direction) {
		return;
	}

	context->assets.fighterData.isRotating = atrue;
	context->assets.fighterData.destination = (Sprite2dDirection)destination;

	if(destination > context->assets.fighter.direction) {
		if((destination - context->assets.fighter.direction) < 8) {
			context->assets.fighterData.direction = SPRITE2D_TURN_CLOCKWISE;
		}
		else {
			context->assets.fighterData.direction =
				SPRITE2D_TURN_COUNTER_CLOCKWISE;
		}
	}
	else if(destination < context->assets.fighter.direction) {
		if((context->assets.fighter.direction - destination) < 8) {
			context->assets.fighterData.direction =
				SPRITE2D_TURN_COUNTER_CLOCKWISE;
		}
		else {
			context->assets.fighterData.direction = SPRITE2D_TURN_CLOCKWISE;
		}
	}
}

static void fighter_processMouseEvent(Troids *context,
		AsgardMouseEvent *mouseEvent)
{
	int destination = 0;
	double angle = 0.0;

	if((context == NULL) || (mouseEvent == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	if((mouseEvent->eventType != EVENT_MOUSE_PRESSED) ||
			((context->mouse.xPosition == mouseEvent->xPosition) &&
			 (context->mouse.yPosition == mouseEvent->yPosition))) {
		return;
	}

	fighter_calculateTurn(context, mouseEvent->xPosition,
			mouseEvent->yPosition);

	context->assets.fighterData.timestamp = time_getTimeMus();
}

// asteroids functions

static int asteroid_getAliveCount(Troids *context)
{
	int ii = 0;
	int result = 0;

	if(context == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	for(ii = 0; ii < TROIDS_MAX_ASTEROIDS; ii++) {
		if(context->assets.asteroids[ii].isAlive) {
			result++;
		}
	}

	return result;
}

static int asteroid_buildNewAsteroid(Troids *context, TroidsAsteroidTypes type,
		int xPosition, int yPosition)
{
	int ii = 0;
	int id = 0;
	int xMin = 0;
	int xMax = 0;
	int yMin = 0;
	int yMax = 0;

	if(context == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	id = -1;

	for(ii = 0; ii < TROIDS_MAX_ASTEROIDS; ii++) {
		if(!context->assets.asteroids[ii].isAlive) {
			id = ii;
			break;
		}
	}

	if(id == -1) {
		return -1;
	}

	context->assets.asteroids[id].isAlive = atrue;
	context->assets.asteroids[id].rotation = 0.0;

	if(type == TROIDS_ASTEROID_TYPE_RANDOM) {
		type = ((rand() % 3) + 1);
	}

	switch(type) {
		case TROIDS_ASTEROID_TYPE_BIG:
			context->assets.asteroids[id].type = TROIDS_ASTEROID_TYPE_BIG;

			sprite2d_setTexture(&context->assets.asteroids[id].sprite,
					SPRITE2D_DIRECTION_EAST,
					context->assets.asteroidBig);

			sprite2d_resize(&context->assets.asteroids[id].sprite, 128, 128);
			break;

		case TROIDS_ASTEROID_TYPE_MEDIUM:
			context->assets.asteroids[id].type = TROIDS_ASTEROID_TYPE_MEDIUM;

			sprite2d_setTexture(&context->assets.asteroids[id].sprite,
					SPRITE2D_DIRECTION_EAST,
					context->assets.asteroidMedium);

			sprite2d_resize(&context->assets.asteroids[id].sprite, 64, 64);
			break;

		case TROIDS_ASTEROID_TYPE_SMALL:
			context->assets.asteroids[id].type = TROIDS_ASTEROID_TYPE_SMALL;

			sprite2d_setTexture(&context->assets.asteroids[id].sprite,
					SPRITE2D_DIRECTION_EAST,
					context->assets.asteroidSmall);

			sprite2d_resize(&context->assets.asteroids[id].sprite, 32, 32);
			break;
	}

	physics2d_reset(&context->assets.asteroids[id].physics);

	physics2d_setThrust(&context->assets.asteroids[id].physics,
				context->assets.fighterData.currentThrust);

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

	if((type == TROIDS_ASTEROID_TYPE_RANDOM) ||
			((xPosition == -1) && (yPosition == -1))) {
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

	physics2d_setPosition(&context->assets.asteroids[id].physics,
			xPosition, yPosition);

	sprite2d_resetDirection(&context->assets.asteroids[id].sprite,
			(rand() % SPRITE2D_DIRECTIONS));

	switch(context->assets.asteroids[id].type) {
		case TROIDS_ASTEROID_TYPE_BIG:
			physics2d_processDirectionalPhysics(
					&context->assets.asteroids[id].physics,
					PHYSICS_DIRECTION_FORWARD,
					afalse,
					0.3);
			break;

		case TROIDS_ASTEROID_TYPE_MEDIUM:
			physics2d_processDirectionalPhysics(
					&context->assets.asteroids[id].physics,
					PHYSICS_DIRECTION_FORWARD,
					afalse,
					0.55);
			break;

		case TROIDS_ASTEROID_TYPE_SMALL:
			physics2d_processDirectionalPhysics(
					&context->assets.asteroids[id].physics,
					PHYSICS_DIRECTION_FORWARD,
					afalse,
					0.9);
			break;
	}

	sprite2d_resetDirection(&context->assets.asteroids[id].sprite,
			SPRITE2D_DIRECTION_EAST);

	log_logf(&context->engine.log, LOG_LEVEL_DEBUG,
			"construct new asteroid #%i at (%i, %i) type %i",
			id,
			context->assets.asteroids[id].sprite.position.x,
			context->assets.asteroids[id].sprite.position.y,
			context->assets.asteroids[id].type);

	return id;
}

// bullets functions

static void bullet_fire(Troids *context)
{
	int ii = 0;
	int bulletX = 0;
	int bulletY = 0;
	int bulletId = -1;
	int asteroidX = 0;
	int asteroidY = 0;
	int asteroidId = -1;
	int destination = 0;
	double angle = 0.0;
	double timestamp = 0.0;
	double distance = 0.0;
	double lowestDistance = 0.0;

	if(context == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	// determine if a bullet can fire

	if(time_getElapsedMusInSeconds(
				context->assets.fighterData.bulletTimestamp) < 0.18) {
		return;
	}

	// determine if any bullets are available for firing

	for(ii = 0; ii < TROIDS_MAX_BULLETS; ii++) {
		if(!context->assets.bullets[ii].isAlive) {
			bulletId = ii;
			break;
		}
	}

	if(bulletId == -1) {
		return;
	}

	// locate the closest asteroid

	lowestDistance = 100000000.0;

	for(ii = 0; ii < TROIDS_MAX_ASTEROIDS; ii++) {
		if(!context->assets.asteroids[ii].isAlive) {
			continue;
		}

		distance = game_calculateDistance(&context->assets.fighter,
				&context->assets.asteroids[ii].sprite);

		if(distance < lowestDistance) {
			asteroidId = ii;
			lowestDistance = distance;
		}
	}

	if((asteroidId == -1) || (lowestDistance >= TROIDS_BULLET_FIRE_RADIUS)) {
		return;
	}

	// fire the bullet

	context->assets.bullets[bulletId].isAlive = atrue;

	bulletX = (context->assets.fighter.position.x +
			(context->assets.fighter.position.width / 2));
	bulletY = (context->assets.fighter.position.y +
			(context->assets.fighter.position.height / 2));

	asteroidX = (context->assets.asteroids[asteroidId].sprite.position.x +
			(context->assets.asteroids[asteroidId].sprite.position.width / 2));
	asteroidY = (context->assets.asteroids[asteroidId].sprite.position.y +
			(context->assets.asteroids[asteroidId].sprite.position.height / 2));

	physics2d_reset(&context->assets.bullets[bulletId].physics);

	physics2d_setPosition(&context->assets.bullets[bulletId].physics,
			bulletX, bulletY);

	angle = math_RadiansToDegrees(
				atan2(
					(game_yScreenCoordToGrid(context, asteroidY) -
					 game_yScreenCoordToGrid(context, bulletY)
					),
					(game_xScreenCoordToGrid(context, asteroidX) -
					 game_xScreenCoordToGrid(context, bulletX)
					)
				));

	if(angle < 0.0) {
		angle += 360.0;
	}

	angle = (360.0 - (angle - 90.0));

	if(angle < 0.0) {
		angle += 360.0;
	}
	else if(angle >= 360.0) {
		angle -= 360.0;
	}

	destination = (int)(angle / 22.5);

	sprite2d_resetDirection(&context->assets.bullets[bulletId].sprite,
			destination);

	physics2d_processDirectionalPhysics(
			&context->assets.bullets[bulletId].physics,
			PHYSICS_DIRECTION_FORWARD,
			afalse,
			1.0);

	sprite2d_resetDirection(&context->assets.bullets[bulletId].sprite,
			SPRITE2D_DIRECTION_EAST);

	timestamp = time_getTimeMus();

	context->assets.fighterData.bulletTimestamp = timestamp;
	context->assets.bullets[bulletId].timestamp = timestamp;
}

// missiles functions

static void missile_fire(Troids *context)
{
	int ii = 0;
	int missileX = 0;
	int missileY = 0;
	int missileId = -1;
	int asteroidX = 0;
	int asteroidY = 0;
	int asteroidId = -1;
	int destination = 0;
	double angle = 0.0;
	double timestamp = 0.0;
	double distance = 0.0;
	double lowestDistance = 0.0;
	double highestDistance = 0.0;

	if(context == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	// determine if a missile can fire

	if(time_getElapsedMusInSeconds(
				context->assets.fighterData.missileTimestamp) < 0.75) {
		return;
	}

	// determine if any missiles are available for firing

	for(ii = 0; ii < TROIDS_MAX_MISSILES; ii++) {
		if(!context->assets.missiles[ii].isAlive) {
			missileId = ii;
			break;
		}
	}

	if(missileId == -1) {
		return;
	}

	// locate the closest asteroid

	lowestDistance = 100000000.0;

	for(ii = 0; ii < TROIDS_MAX_ASTEROIDS; ii++) {
		if(!context->assets.asteroids[ii].isAlive) {
			continue;
		}

		distance = game_calculateDistance(&context->assets.fighter,
				&context->assets.asteroids[ii].sprite);

		if(distance < lowestDistance) {
			asteroidId = ii;
			lowestDistance = distance;
		}
	}

	if(asteroidId == -1) {
		return;
	}

	// locate the furthest asteroid

	if(lowestDistance >= 200.0) {
		for(ii = 0; ii < TROIDS_MAX_ASTEROIDS; ii++) {
			if(!context->assets.asteroids[ii].isAlive) {
				continue;
			}

			distance = game_calculateDistance(&context->assets.fighter,
					&context->assets.asteroids[ii].sprite);

			if(distance > highestDistance) {
				asteroidId = ii;
				highestDistance = distance;
			}
		}

		if(asteroidId == -1) {
			return;
		}
	}

	// locate a random asteroid

	asteroidId = -1;

	do {
		ii = (rand() % TROIDS_MAX_ASTEROIDS);

		if(context->assets.asteroids[ii].isAlive) {
			asteroidId = ii;
		}
	} while(asteroidId == -1);

	// fire the missile

	context->assets.missiles[missileId].isAlive = atrue;
	context->assets.missiles[missileId].targetId = asteroidId;

	missileX = (context->assets.fighter.position.x +
			(context->assets.fighter.position.width / 2));
	missileY = (context->assets.fighter.position.y +
			(context->assets.fighter.position.height / 2));

	asteroidX = (context->assets.asteroids[asteroidId].sprite.position.x +
			(context->assets.asteroids[asteroidId].sprite.position.width / 2));
	asteroidY = (context->assets.asteroids[asteroidId].sprite.position.y +
			(context->assets.asteroids[asteroidId].sprite.position.height / 2));

	physics2d_reset(&context->assets.missiles[missileId].physics);

	physics2d_setPosition(&context->assets.missiles[missileId].physics,
			missileX, missileY);

	angle = math_RadiansToDegrees(
				atan2(
					(game_yScreenCoordToGrid(context, asteroidY) -
					 game_yScreenCoordToGrid(context, missileY)
					),
					(game_xScreenCoordToGrid(context, asteroidX) -
					 game_xScreenCoordToGrid(context, missileX)
					)
				));

	if(angle < 0.0) {
		angle += 360.0;
	}

	angle = (360.0 - (angle - 90.0));

	if(angle < 0.0) {
		angle += 360.0;
	}
	else if(angle >= 360.0) {
		angle -= 360.0;
	}

	context->assets.missiles[missileId].rotation = angle;

	destination = (int)(angle / 22.5);

	sprite2d_resetDirection(&context->assets.missiles[missileId].sprite,
			destination);

	physics2d_processDirectionalPhysics(
			&context->assets.missiles[missileId].physics,
			PHYSICS_DIRECTION_FORWARD,
			afalse,
			0.6);

	sprite2d_resetDirection(&context->assets.missiles[missileId].sprite,
			SPRITE2D_DIRECTION_EAST);

	timestamp = time_getTimeMus();

	context->assets.fighterData.missileTimestamp = timestamp;
	context->assets.missiles[missileId].timestamp = timestamp;
	context->assets.missiles[missileId].physicsTimestamp = timestamp;
}

static void missile_processPhysics(Troids *context, int missileId)
{
	int ii = 0;
	int missileX = 0;
	int missileY = 0;
	int asteroidX = 0;
	int asteroidY = 0;
	int asteroidId = -1;
	int destination = 0;
	double angle = 0.0;
	double distance = 0.0;
	double lowestDistance = 0.0;

	if(context == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	// locate the closest asteroid if target destroyed

	asteroidId = context->assets.missiles[missileId].targetId;

	if(!context->assets.asteroids[asteroidId].isAlive) {
		asteroidId = -1;

		lowestDistance = 100000000.0;

		for(ii = 0; ii < TROIDS_MAX_ASTEROIDS; ii++) {
			if(!context->assets.asteroids[ii].isAlive) {
				continue;
			}

			distance = game_calculateDistance(
					&context->assets.missiles[missileId].sprite,
					&context->assets.asteroids[ii].sprite);

			if(distance < lowestDistance) {
				asteroidId = ii;
				lowestDistance = distance;
			}
		}
	}

	if(asteroidId == -1) {
		return;
	}

	// re-aim and accelerate the missile

	context->assets.missiles[missileId].targetId = asteroidId;

	missileX = (context->assets.missiles[missileId].sprite.position.x +
			(context->assets.missiles[missileId].sprite.position.width / 2));
	missileY = (context->assets.missiles[missileId].sprite.position.y +
			(context->assets.missiles[missileId].sprite.position.height / 2));

	asteroidX = (context->assets.asteroids[asteroidId].sprite.position.x +
			(context->assets.asteroids[asteroidId].sprite.position.width / 2));
	asteroidY = (context->assets.asteroids[asteroidId].sprite.position.y +
			(context->assets.asteroids[asteroidId].sprite.position.height / 2));

	angle = math_RadiansToDegrees(
				atan2(
					(game_yScreenCoordToGrid(context, asteroidY) -
					 game_yScreenCoordToGrid(context, missileY)
					),
					(game_xScreenCoordToGrid(context, asteroidX) -
					 game_xScreenCoordToGrid(context, missileX)
					)
				));

	if(angle < 0.0) {
		angle += 360.0;
	}

	angle = (360.0 - (angle - 90.0));

	if(angle < 0.0) {
		angle += 360.0;
	}
	else if(angle >= 360.0) {
		angle -= 360.0;
	}

	context->assets.missiles[missileId].rotation = (angle - 90);

	if(context->assets.missiles[missileId].rotation < 0.0) {
		angle += 360.0;
	}

	// determine if a missile can course correct & accelerate

	if(time_getElapsedMusInSeconds(
				context->assets.missiles[missileId].physicsTimestamp) < 0.15) {
		return;
	}

	destination = (int)(angle / 22.5);

	sprite2d_resetDirection(&context->assets.missiles[missileId].sprite,
			destination);

	physics2d_processDirectionalPhysics(
			&context->assets.missiles[missileId].physics,
			PHYSICS_DIRECTION_FORWARD,
			afalse,
			0.05);

	sprite2d_resetDirection(&context->assets.missiles[missileId].sprite,
			SPRITE2D_DIRECTION_EAST);

	context->assets.missiles[missileId].physicsTimestamp = time_getTimeMus();
}

// powerups function

static int powerup_buildNewPowerup(Troids *context)
{
	aboolean isValid = afalse;
	int ii = 0;
	int id = 0;
	int counter = 0;
	int xPosition = 0;
	int yPosition = 0;

	if(context == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	// determine if fighter has all powerups

	if((context->assets.fighterData.hasBullets) &&
			(context->assets.fighterData.hasMissiles) &&
			(context->assets.fighterData.hasShields)) {
		return 0;
	}

	// check to see if any powerups are live

	for(ii = 0; ii < TROIDS_MAX_ASTEROIDS; ii++) {
		if(context->assets.powerups[id].isAlive) {
			return 0;
		}
	}

	// pick a random powerup

	do {
		isValid = atrue;

		id = (rand() % TROIDS_POWERUP_TYPE_LENGTH);

		if(context->assets.powerups[id].isAlive) {
			isValid = afalse;
		}
		else if((id == TROIDS_POWERUP_TYPE_GUN) &&
				(context->assets.fighterData.hasBullets)) {
			isValid = afalse;
		}
		else if((id == TROIDS_POWERUP_TYPE_MISSILE) &&
				(context->assets.fighterData.hasMissiles)) {
			isValid = afalse;
		}
		else if((id == TROIDS_POWERUP_TYPE_SHIELD) &&
				(context->assets.fighterData.hasShields)) {
			isValid = afalse;
		}

		counter++;

		if(counter > 3) {
			return 0;
		}
	} while(!isValid);

	// construct powerup

	context->assets.powerups[id].isAlive = atrue;

	physics2d_reset(&context->assets.powerups[id].physics);

	physics2d_setThrust(&context->assets.powerups[id].physics,
				context->assets.fighterData.currentThrust);

	// spawn algorithm - at edge of screen only

	if((rand() % 2) == 0) {
		xPosition = (rand() % context->engine.canvas.screenWidth);
		if((rand() % 2) == 0) {
			yPosition = 0;
		}
		else {
			yPosition = context->engine.canvas.screenHeight;
		}
	}
	else {
		if((rand() % 2) == 0) {
			xPosition = 0;
		}
		else {
			xPosition = context->engine.canvas.screenWidth;
		}
		yPosition = (rand() % context->engine.canvas.screenHeight);
	}

	physics2d_setPosition(&context->assets.powerups[id].physics,
			xPosition, yPosition);

	sprite2d_resetDirection(&context->assets.powerups[id].sprite,
			(rand() % SPRITE2D_DIRECTIONS));

	physics2d_processDirectionalPhysics(
			&context->assets.powerups[id].physics,
			PHYSICS_DIRECTION_FORWARD,
			afalse,
			0.55);

	sprite2d_resetDirection(&context->assets.powerups[id].sprite,
			SPRITE2D_DIRECTION_EAST);

	log_logf(&context->engine.log, LOG_LEVEL_DEBUG,
			"construct new powerup #%i at (%i, %i) type %i",
			id,
			context->assets.powerups[id].sprite.position.x,
			context->assets.powerups[id].sprite.position.y,
			context->assets.powerups[id].type);

	context->assets.fighterData.powerupTimestamp = time_getTimeMus();

	return id;
}

// rendering functions

static void render_drawCube(Troids *context)
{
	int ii = 0;
	int nn = 0;

	double vertices[] = {
		-1.0, -1.0, 1.0,
		1.0, -1.0, 1.0,
		1.0, 1.0, 1.0,
		-1.0, 1.0, 1.0,

		1.0, -1.0, 1.0,
		1.0, -1.0, -1.0,
		1.0, 1.0, -1.0,
		1.0, 1.0, 1.0,
		
		1.0, -1.0, -1.0,
		-1.0, -1.0, -1.0,
		-1.0, 1.0, -1.0,
		1.0, 1.0, -1.0,
		
		-1.0, -1.0, -1.0,
		-1.0, -1.0, 1.0,
		-1.0, 1.0, 1.0,
		-1.0, 1.0, -1.0,
		
		-1.0, -1.0, -1.0,
		1.0, -1.0, -1.0,
		1.0, -1.0, 1.0,
		-1.0, -1.0, 1.0,
		
		-1.0, 1.0, 1.0,
		1.0, 1.0, 1.0,
		1.0, 1.0, -1.0,
		-1.0, 1.0, -1.0
	};
	
	double textureCoords[] = {	
		0.0, 0.0,
		0.0, 1.0,
		1.0, 1.0, 
		1.0, 0.0,
		
		0.0, 0.0,
		0.0, 1.0,
		1.0, 1.0,
		1.0, 0.0,
		
		0.0, 0.0,
		0.0, 1.0,
		1.0, 1.0,
		1.0, 0.0,
		
		0.0, 0.0,
		0.0, 1.0,
		1.0, 1.0,
		1.0, 0.0,
		
		0.0, 0.0,
		0.0, 1.0,
		1.0, 1.0,
		1.0, 0.0,
		
		0.0, 0.0,
		0.0, 1.0,
		1.0, 1.0,
		1.0, 0.0
	};

	const double scalar = 1.0;

	cgi_glPushMatrix();
	cgi_glTranslated(0.0, 0.0, -2.5);

	cgi_glRotated(context->assets.cubeData.xRotation, 1.0, 0.0, 0.0);
	cgi_glRotated(context->assets.cubeData.yRotation, 0.0, 1.0, 0.0);
	cgi_glRotated(context->assets.cubeData.zRotation, 0.0, 0.0, 1.0);

	texture_apply(context->assets.cube);

	cgi_glBegin(GL_QUADS);

	for(ii = 0, nn = 0; ((ii < 72) && (nn < 48)); ii += 3, nn += 2) {
		cgi_glVertex3d(
				(vertices[(ii + 0)] * scalar),
				(vertices[(ii + 1)] * scalar),
				(vertices[(ii + 2)] * scalar)
			);

		cgi_glTexCoord2d(
				textureCoords[(nn + 0)],
				textureCoords[(nn + 1)]);
	}

	cgi_glEnd();

	texture_unApply(context->assets.cube);

	cgi_glPopMatrix();
}

static void render_loadingScreen(Troids *context, Canvas *canvas)
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
	if(!TROIDS_IS_PRODUCTION_BUILD) {
		CGI_COLOR_WHITE;

		font_draw2d(&context->engine.font, canvas, 4, 0, "Loading...");
	}
#endif // !__ANDROID__
}

static void render_splashScreen(Troids *context, Canvas *canvas)
{
	// 3d objects

	canvas_toggleRenderMode3d(canvas);

	// 2d objects

	canvas_toggleRenderMode2d(canvas);

	// draw the splash screen

	texture_draw2d(context->assets.splashScreen, 0, 0,
			context->engine.canvas.screenWidth,
			context->engine.canvas.screenHeight);

#if !defined(__ANDROID__)
	if(!TROIDS_IS_PRODUCTION_BUILD) {
		CGI_COLOR_WHITE;

		font_draw2d(&context->engine.font, canvas, 4, 0,
				"Framerate: %0.2f", canvas->framerate);
		font_draw2d(&context->engine.font, canvas, 4, 16,
				"Mouse: (%i, %i)",
				context->engine.mouse.x, context->engine.mouse.y);
	}
#endif // !__ANDROID__
}

static void render_instructionScreen(Troids *context, Canvas *canvas)
{
	// 3d objects

	canvas_toggleRenderMode3d(canvas);

	// 2d objects

	canvas_toggleRenderMode2d(canvas);

	// draw the splash screen

	texture_draw2d(context->assets.instructionScreen, 0, 0,
			context->engine.canvas.screenWidth,
			context->engine.canvas.screenHeight);

#if !defined(__ANDROID__)
	if(!TROIDS_IS_PRODUCTION_BUILD) {
		CGI_COLOR_WHITE;

		font_draw2d(&context->engine.font, canvas, 4, 0,
				"Framerate: %0.2f", canvas->framerate);
		font_draw2d(&context->engine.font, canvas, 4, 16,
				"Mouse: (%i, %i)",
				context->engine.mouse.x, context->engine.mouse.y);
	}
#endif // !__ANDROID__
}

static void render_playing(Troids *context, Canvas *canvas)
{
	aboolean isTargeted = afalse;
	int ii = 0;
	int nn = 0;
	int number = 0;
	int aliveTime = 0;

	Color color;

	// 3d objects

	canvas_toggleRenderMode3d(canvas);

	render_drawCube(context);

	// 2d objects

	canvas_toggleRenderMode2d(canvas);

	// draw the asteroids

	for(ii = 0; ii < TROIDS_MAX_ASTEROIDS; ii++) {
		if(context->assets.asteroids[ii].isAlive) {
			context->assets.asteroids[ii].sprite.currentRotation =
					context->assets.asteroids[ii].rotation;

			context->assets.asteroids[ii].sprite.render2d(
					&context->assets.asteroids[ii].sprite);

			context->assets.asteroids[ii].sprite.currentRotation = 0.0;

			// draw the asteroid collision map

			if(TROIDS_DRAW_COLLISION_MAP) {
				texture_draw2d(context->assets.circleBlue,
						context->assets.asteroids[ii].sprite.position.x,
						context->assets.asteroids[ii].sprite.position.y,
						context->assets.asteroids[ii].sprite.position.width,
						context->assets.asteroids[ii].sprite.position.height);
			}

			// draw targeting cirlce

			isTargeted = afalse;

			if((context->assets.fighterData.hasBullets) &&
					(game_calculateDistance(&context->assets.fighter,
							&context->assets.asteroids[ii].sprite) <
					 TROIDS_BULLET_FIRE_RADIUS)) {
				isTargeted = atrue;
			}
			else {
				for(nn = 0; nn < TROIDS_MAX_MISSILES; nn++) {
					if((context->assets.missiles[nn].isAlive) &&
							(context->assets.missiles[nn].targetId == ii)) {
						isTargeted = atrue;
						break;
					}
				}
			}

			if(isTargeted) {
				draw2d_rotatedTexture(canvas,
						context->assets.asteroids[ii].sprite.position.x,
						context->assets.asteroids[ii].sprite.position.y,
						context->assets.asteroids[ii].sprite.position.width,
						context->assets.asteroids[ii].sprite.position.height,
						(context->assets.asteroids[ii].rotation * -1),
						context->assets.gunRadius);

				if(context->assets.asteroids[ii].type ==
						TROIDS_ASTEROID_TYPE_SMALL) {
					draw2d_rotatedTexture(canvas,
						context->assets.asteroids[ii].sprite.position.x,
						context->assets.asteroids[ii].sprite.position.y,
						context->assets.asteroids[ii].sprite.position.width,
						context->assets.asteroids[ii].sprite.position.height,
						(context->assets.asteroids[ii].rotation * -1),
						context->assets.gunRadius);
				}

				texture_draw2d(context->assets.gunTargeted,
						(context->assets.asteroids[ii].sprite.position.x +
						 (context->assets.asteroids[ii].sprite.position.width /
						  2)) + 8,
						(context->assets.asteroids[ii].sprite.position.y +
						 (context->assets.asteroids[ii].sprite.position.height /
						  2)),
						33, 16);
			}

/*			{
				// debug troid location & distance from fighter

				char buffer[8192];

				snprintf(buffer, (int)(sizeof(buffer) -1),
						"Troid[%03i](%i, %i) @ %0.2f\n",
						ii,
						context->assets.asteroids[ii].sprite.position.x,
						context->assets.asteroids[ii].sprite.position.y,
						game_calculateDistance(&context->assets.fighter,
							&context->assets.asteroids[ii].sprite));

				CGI_COLOR_WHITE;

				draw2d_font(canvas,
						((context->assets.asteroids[ii].sprite.position.x +
						  context->assets.asteroids[ii].sprite.xOffset) + 2),
						((context->assets.asteroids[ii].sprite.position.y +
						  context->assets.asteroids[ii].sprite.yOffset) - 22),
						buffer,
						&context->engine.font);
			}*/
		}
	}

	// draw the powerups

	for(ii = 0; ii < TROIDS_POWERUP_TYPE_LENGTH; ii++) {
		if(context->assets.powerups[ii].isAlive) {
			context->assets.powerups[ii].sprite.render2d(
					&context->assets.powerups[ii].sprite);

			// draw the asteroid collision map

			if(TROIDS_DRAW_COLLISION_MAP) {
				texture_draw2d(context->assets.circleBlue,
						context->assets.powerups[ii].sprite.position.x,
						context->assets.powerups[ii].sprite.position.y,
						context->assets.powerups[ii].sprite.position.width,
						context->assets.powerups[ii].sprite.position.height);
			}
		}
	}

	// draw the bullets

	if(context->assets.fighterData.isAlive) {
		for(ii = 0; ii < TROIDS_MAX_BULLETS; ii++) {
			if(context->assets.bullets[ii].isAlive) {
				context->assets.bullets[ii].sprite.render2d(
						&context->assets.bullets[ii].sprite);

				// draw the bullet collision map

				if(TROIDS_DRAW_COLLISION_MAP) {
					texture_draw2d(context->assets.circleBlue,
							context->assets.bullets[ii].sprite.position.x,
							context->assets.bullets[ii].sprite.position.y,
							context->assets.bullets[ii].sprite.position.width,
							context->assets.bullets[ii].sprite.position.height);
				}
			}
		}
	}

	// draw the missiles

	if(context->assets.fighterData.isAlive) {
		for(ii = 0; ii < TROIDS_MAX_MISSILES; ii++) {
			if(context->assets.missiles[ii].isAlive) {
				context->assets.missiles[ii].sprite.currentRotation =
					context->assets.missiles[ii].rotation;

				context->assets.missiles[ii].sprite.render2d(
						&context->assets.missiles[ii].sprite);

				context->assets.missiles[ii].sprite.currentRotation = 0.0;

				// draw the missile collision map

				if(TROIDS_DRAW_COLLISION_MAP) {
					texture_draw2d(context->assets.circleBlue,
						context->assets.missiles[ii].sprite.position.x,
						context->assets.missiles[ii].sprite.position.y,
						context->assets.missiles[ii].sprite.position.width,
						context->assets.missiles[ii].sprite.position.height);
				}
			}
		}
	}

	// draw the fighter's missile upgrade

	if(context->assets.fighterData.hasMissiles) {
		draw2d_rotatedTexture(canvas,
				context->assets.fighter.position.x,
				context->assets.fighter.position.y,
				context->assets.fighter.position.width,
				context->assets.fighter.position.height,
				(context->assets.fighter.direction * 22.5),
				context->assets.missileUpgrade);
	}

	// draw the fighter

	context->assets.fighter.render2d(&context->assets.fighter);

	// draw the fighter's thrusters

	if((context->assets.fighterPhysics.position.xSpeed > 0.5) ||
			(context->assets.fighterPhysics.position.ySpeed > 0.5)) {
		draw2d_rotatedTexture(canvas,
				(context->assets.fighter.position.x -
				 (context->assets.fighter.position.width / 2)),
				(context->assets.fighter.position.y -
				 (context->assets.fighter.position.height / 2)),
				(context->assets.fighter.position.width * 2),
				(context->assets.fighter.position.height * 2),
				(context->assets.fighter.direction * 22.5),
				context->assets.thrusters);
	}
	else if((context->assets.fighterPhysics.position.xSpeed > 0.25) ||
			(context->assets.fighterPhysics.position.ySpeed > 0.25)) {
		draw2d_rotatedTexture(canvas,
				(context->assets.fighter.position.x -
				 (context->assets.fighter.position.width / 2)),
				(context->assets.fighter.position.y -
				 (context->assets.fighter.position.height / 2)),
				(context->assets.fighter.position.width * 2),
				(context->assets.fighter.position.height * 2),
				(context->assets.fighter.direction * 22.5),
				context->assets.thrustersMedium);
	}
	else if((context->assets.fighterPhysics.position.xSpeed > 0.1) ||
			(context->assets.fighterPhysics.position.ySpeed > 0.1)) {
		draw2d_rotatedTexture(canvas,
				(context->assets.fighter.position.x -
				 (context->assets.fighter.position.width / 2)),
				(context->assets.fighter.position.y -
				 (context->assets.fighter.position.height / 2)),
				(context->assets.fighter.position.width * 2),
				(context->assets.fighter.position.height * 2),
				(context->assets.fighter.direction * 22.5),
				context->assets.thrustersSmall);
	}

	// draw the fighter's bullet upgrade

	if(context->assets.fighterData.hasBullets) {
		draw2d_rotatedTexture(canvas,
				context->assets.fighter.position.x,
				context->assets.fighter.position.y,
				context->assets.fighter.position.width,
				context->assets.fighter.position.height,
				(context->assets.fighter.direction * 22.5),
				context->assets.bulletUpgrade);
	}

	// draw the fighter collision map

	if(TROIDS_DRAW_COLLISION_MAP) {
		texture_draw2d(context->assets.circleBlue,
				context->assets.fighter.position.x,
				context->assets.fighter.position.y,
				context->assets.fighter.position.width,
				context->assets.fighter.position.height);
	}

	// draw the fighter's gun radius

	if(!TROIDS_IS_PRODUCTION_BUILD) {
		if(context->assets.fighterData.hasBullets) {
			texture_draw2d(context->assets.circleBlue,
					((context->assets.fighter.position.x +
					  (context->assets.fighter.position.width / 2)) -
					 (int)TROIDS_BULLET_FIRE_RADIUS),
					((context->assets.fighter.position.y +
					  (context->assets.fighter.position.height / 2)) -
					 (int)TROIDS_BULLET_FIRE_RADIUS),
					(int)(TROIDS_BULLET_FIRE_RADIUS * 2.0),
					(int)(TROIDS_BULLET_FIRE_RADIUS * 2.0));
		}
	}

	// draw the fighter's shields

	if(context->assets.fighterData.hasShields) {
/*		draw2d_rotatedTexture(canvas,
				(context->assets.fighter.position.x - 8),
				(context->assets.fighter.position.y - 8),
				(context->assets.fighter.position.width + 16),
				(context->assets.fighter.position.height + 16),
				(context->assets.fighter.direction * 22.5),
				context->assets.shield);*/
		draw2d_rotatedTexture(canvas,
				(context->assets.fighter.position.x - 16),
				(context->assets.fighter.position.y - 16),
				(context->assets.fighter.position.width + 32),
				(context->assets.fighter.position.height + 32),
				context->assets.fighterData.shieldRotation,
				context->assets.shield);
	}

	// draw a 2d animations

	animation2d_render2d(&context->assets.fighterAnimation);
	animation2d_render2d(&context->assets.missileAnimation);

	for(ii = 0; ii < TROIDS_MAX_ASTEROIDS; ii++) {
		animation2d_render2d(&context->assets.asteroids[ii].animation);
	}

	// draw a cross-hatch at the current mouse position

	if(!TROIDS_IS_PRODUCTION_BUILD) {
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

	// draw the background

	texture_draw2d(context->assets.background, 0, 0,
			context->engine.canvas.screenWidth,
			context->engine.canvas.screenHeight);

	// draw the alive time & score

	aliveTime = (int)time_getElapsedMusInSeconds(
			context->assets.fighterData.aliveTimestamp);

	aliveTime += context->assets.fighterData.asteroidsDestroyed;

	number = (aliveTime / 1000);

	if((number < 0) || (number > 9)) {
		number = 0;
	}

	aliveTime -= (number * 1000);

	texture_draw2d(context->assets.numbers[number],
			((context->engine.canvas.screenWidth / 2) -
			 ((((32 + 4) * 4) / 2)) + ((32 + 4) * 0)),
			8, 32, 38);

	number = (aliveTime  / 100);

	if((number < 0) || (number > 9)) {
		number = 0;
	}

	aliveTime -= (number * 100);

	texture_draw2d(context->assets.numbers[number],
			((context->engine.canvas.screenWidth / 2) -
			 ((((32 + 4) * 4) / 2)) + ((32 + 4) * 1)),
			8, 32, 38);

	number = (aliveTime / 10);

	if((number < 0) || (number > 9)) {
		number = 0;
	}

	texture_draw2d(context->assets.numbers[number],
			((context->engine.canvas.screenWidth / 2) -
			 ((((32 + 4) * 4) / 2)) + ((32 + 4) * 2)),
			8, 32, 38);

	number = (aliveTime % 10);

	if((number < 0) || (number > 9)) {
		number = 0;
	}

	texture_draw2d(context->assets.numbers[number],
			((context->engine.canvas.screenWidth / 2) -
			 ((((32 + 4) * 4) / 2)) + ((32 + 4) * 3)),
			8, 32, 38);

	// draw the remaining lives

	for(ii = 0; ii < 3; ii++) {
		if(ii < context->assets.fighterData.respawnsRemaining) {
			texture_draw2d(context->assets.fighterOutline,
					(8 + (ii * 40)), 8, 48, 48);
		}
		else {
			texture_draw2d(context->assets.fighterOutlineFaded,
					(8 + (ii * 40)), 8, 48, 48);
		}
	}

#if !defined(__ANDROID__)
/*
	CGI_COLOR_WHITE;

	font_draw2d(&context->engine.font, canvas, 4, 0,
			"Framerate: %0.2f", canvas->framerate);

	font_draw2d(&context->engine.font, canvas, 4, 16,
			"Sprite { %i, %0.2f, %0.2f %i, %i } pw { %i, %i }",
			context->assets.fighterData.isRotating,
			context->assets.fighterData.angle,
			context->assets.fighterData.distance,
			context->assets.fighterData.destination,
			context->assets.fighterData.direction,
			context->assets.fighterData.hasShields,
			context->assets.fighterData.shieldStrength);

	for(ii = 0; ii < TROIDS_MAX_ASTEROIDS; ii++) {
		if(context->assets.asteroids[ii].isAlive) {
			font_draw2d(&context->engine.font, canvas, 4, (16 * (ii + 2)),
					"Asteroid #%03i { (%03i, %03i) c%i }",
					ii,
					context->assets.asteroids[ii].sprite.position.x,
					context->assets.asteroids[ii].sprite.position.y,
					(int)game_isCollided(&context->assets.fighter,
						&context->assets.asteroids[ii].sprite));
		}
	}

	for(ii = 0; ii < TROIDS_POWERUP_TYPE_LENGTH; ii++) {
		if(context->assets.powerups[ii].isAlive) {
			font_draw2d(&context->engine.font, canvas, 4, (16 * (ii + 2)),
					"Powerup #%03i { (%03i, %03i) c%i }",
					ii,
					context->assets.powerups[ii].sprite.position.x,
					context->assets.powerups[ii].sprite.position.y,
					(int)game_isCollided(&context->assets.fighter,
						&context->assets.powerups[ii].sprite));
		}
	}

	for(ii = 0; ii < TROIDS_MAX_BULLETS; ii++) {
		if(context->assets.bullets[ii].isAlive) {
			font_draw2d(&context->engine.font, canvas, 4, (16 * (ii + 2)),
					"Bullet #%02i { (%03i, %03i) }",
					ii,
					context->assets.bullets[ii].sprite.position.x,
					context->assets.bullets[ii].sprite.position.y);
		}
	}

	for(ii = 0; ii < TROIDS_MAX_MISSILES; ii++) {
		if(context->assets.missiles[ii].isAlive) {
			font_draw2d(&context->engine.font, canvas, 4, (16 * (ii + 2)),
					"Missile #%02i { (%03i, %03i) }",
					ii,
					context->assets.missiles[ii].sprite.position.x,
					context->assets.missiles[ii].sprite.position.y);
		}
	}
*/
#endif // !__ANDROID__
}

static void render_gameOverScreen(Troids *context, Canvas *canvas)
{
	int number = 0;
	int aliveTime = 0;

	// 3d objects

	canvas_toggleRenderMode3d(canvas);

	// 2d objects

	canvas_toggleRenderMode2d(canvas);

	// draw the splash screen

	texture_draw2d(context->assets.gameOverScreen, 0, 0,
			context->engine.canvas.screenWidth,
			context->engine.canvas.screenHeight);

	// draw the alive time & score

	aliveTime = context->assets.fighterData.finalScore;

	number = (aliveTime / 1000);

	if((number < 0) || (number > 9)) {
		number = 0;
	}

	aliveTime -= (number * 1000);

	texture_draw2d(context->assets.numbers[number],
			((context->engine.canvas.screenWidth / 2) -
			 ((((32 + 4) * 4) / 2)) + ((32 + 4) * 0)),
			426, 32, 38);

	number = (aliveTime  / 100);

	if((number < 0) || (number > 9)) {
		number = 0;
	}

	aliveTime -= (number * 100);

	texture_draw2d(context->assets.numbers[number],
			((context->engine.canvas.screenWidth / 2) -
			 ((((32 + 4) * 4) / 2)) + ((32 + 4) * 1)),
			426, 32, 38);

	number = (aliveTime / 10);

	if((number < 0) || (number > 9)) {
		number = 0;
	}

	texture_draw2d(context->assets.numbers[number],
			((context->engine.canvas.screenWidth / 2) -
			 ((((32 + 4) * 4) / 2)) + ((32 + 4) * 2)),
			426, 32, 38);

	number = (aliveTime % 10);

	if((number < 0) || (number > 9)) {
		number = 0;
	}

	texture_draw2d(context->assets.numbers[number],
			((context->engine.canvas.screenWidth / 2) -
			 ((((32 + 4) * 4) / 2)) + ((32 + 4) * 3)),
			426, 32, 38);

#if !defined(__ANDROID__)
	if(!TROIDS_IS_PRODUCTION_BUILD) {
		CGI_COLOR_WHITE;

		font_draw2d(&context->engine.font, canvas, 4, 0,
				"Framerate: %0.2f", canvas->framerate);
		font_draw2d(&context->engine.font, canvas, 4, 16,
				"Mouse: (%i, %i)",
				context->engine.mouse.x, context->engine.mouse.y);
	}
#endif // !__ANDROID__
}

// gameplay functions

static void gameplay_processPlaying(Troids *context)
{
	int ii = 0;
	int nn = 0;
	int jj = 0;
	int id = 0;
	int length = 0;
	int xPosition = 0;
	int yPosition = 0;
	int aliveTime = 0;

	static int aliveTimestamp = 0;

	if(context == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	if(context->assets.fighterData.isAlive) {
		// handle fighter sprite rotation

		if((context->assets.fighterData.isRotating) &&
				(context->assets.fighter.currentRotation == 0.0)) {
			if(context->assets.fighterData.destination ==
					context->assets.fighter.direction) {
				context->assets.fighterData.isRotating = afalse;
			}
			else {
				sprite2d_changeDirection(&context->assets.fighter,
						context->assets.fighterData.destination,
						context->assets.fighterData.direction,
						context->engine.canvas.framerate);
			}
		}

		// handle fighter sprite physics

		if(context->mouse.isDown) {
			if(time_getElapsedMusInSeconds(
						context->assets.fighterData.timestamp) > 1.0) {
				context->mouse.isDown = afalse;
			}
			else {
				fighter_calculateTurn(context, context->mouse.xPosition,
						context->mouse.yPosition);
				physics2d_processPhysics(&context->assets.fighterPhysics,
						PHYSICS_DIRECTION_FORWARD);
			}
		}
	}
	else if(!animation2d_isPlaying(&context->assets.fighterAnimation)) {
		context->assets.fighterData.isAlive = atrue;

		physics2d_reset(&context->assets.fighterPhysics);

		physics2d_setPosition(&context->assets.fighterPhysics,
				((context->engine.canvas.screenWidth - 64) / 2),
				((context->engine.canvas.screenHeight - 64) / 2));

		for(ii = 0; ii < TROIDS_MAX_ASTEROIDS; ii++) {
			context->assets.asteroids[ii].isAlive = afalse;
		}

		for(ii = 0; ii < TROIDS_MAX_BULLETS; ii++) {
			context->assets.bullets[ii].isAlive = afalse;
		}

		for(ii = 0; ii < TROIDS_MAX_MISSILES; ii++) {
			context->assets.missiles[ii].isAlive = afalse;
		}

		for(ii = 0; ii < TROIDS_POWERUP_TYPE_LENGTH; ii++) {
			context->assets.powerups[ii].isAlive = afalse;
		}

		asteroid_buildNewAsteroid(context, TROIDS_ASTEROID_TYPE_RANDOM, -1, -1);
		asteroid_buildNewAsteroid(context, TROIDS_ASTEROID_TYPE_RANDOM, -1, -1);
		asteroid_buildNewAsteroid(context, TROIDS_ASTEROID_TYPE_RANDOM, -1, -1);
		asteroid_buildNewAsteroid(context, TROIDS_ASTEROID_TYPE_RANDOM, -1, -1);
	}

	// process fighter sprite physics

	context->assets.fighterData.currentThrust =
		(TROIDS_ASTEROIDS_STARTING_THRUST +
			(time_getElapsedMusInSeconds(
				context->assets.fighterData.aliveTimestamp) / 1000000.0));

	physics2d_processGameplay(&context->assets.fighterPhysics);

	if(physics2d_getX(&context->assets.fighterPhysics) >
			context->engine.canvas.screenWidth) {
		physics2d_setPosition(&context->assets.fighterPhysics, 0,
				physics2d_getY(&context->assets.fighterPhysics));
	}
	else if(physics2d_getX(&context->assets.fighterPhysics) < 0) {
		physics2d_setPosition(&context->assets.fighterPhysics,
				context->engine.canvas.screenWidth,
				physics2d_getY(&context->assets.fighterPhysics));
	}

	if(physics2d_getY(&context->assets.fighterPhysics) >
			context->engine.canvas.screenHeight) {
		physics2d_setPosition(&context->assets.fighterPhysics,
				physics2d_getX(&context->assets.fighterPhysics),
				0);
	}
	else if(physics2d_getY(&context->assets.fighterPhysics)  < 0) {
		physics2d_setPosition(&context->assets.fighterPhysics,
				physics2d_getX(&context->assets.fighterPhysics),
				context->engine.canvas.screenHeight);
	}

	// process fighter sprite

	sprite2d_processGameplay(&context->assets.fighter,
			context->engine.canvas.framerate);

	if(context->assets.fighterData.hasShields) {
		context->assets.fighterData.shieldRotation += 0.5;
		if(context->assets.fighterData.shieldRotation >= 360.0) {
			context->assets.fighterData.shieldRotation = 0.0;
		}
	}

	// process asteroids

	for(ii = 0; ii < TROIDS_MAX_ASTEROIDS; ii++) {
		if(!context->assets.asteroids[ii].isAlive) {
			continue;
		}


		switch(context->assets.asteroids[ii].type) {
			case TROIDS_ASTEROID_TYPE_BIG:
				if((ii % 2) == 0) {
					context->assets.asteroids[ii].rotation += 0.2;
				}
				else {
					context->assets.asteroids[ii].rotation -= 0.2;
				}
				break;

			case TROIDS_ASTEROID_TYPE_MEDIUM:
				if((ii % 2) == 0) {
					context->assets.asteroids[ii].rotation += 0.6;
				}
				else {
					context->assets.asteroids[ii].rotation -= 0.6;
				}
				break;

			case TROIDS_ASTEROID_TYPE_SMALL:
				if((ii % 2) == 0) {
					context->assets.asteroids[ii].rotation += 1.1;
				}
				else {
					context->assets.asteroids[ii].rotation -= 1.1;
				}
				break;
		}

		if(((ii % 2) == 0) &&
				(context->assets.asteroids[ii].rotation >= 360.0)) {
			context->assets.asteroids[ii].rotation = 0.0;
		}
		else if(((ii % 2) != 0) &&
				(context->assets.asteroids[ii].rotation < 0.0)) {
			context->assets.asteroids[ii].rotation = 360.0;
		}

		physics2d_processGameplay(&context->assets.asteroids[ii].physics);

		if(physics2d_getX(&context->assets.asteroids[ii].physics) >
				context->engine.canvas.screenWidth) {
			physics2d_setPosition(&context->assets.asteroids[ii].physics, 0,
					physics2d_getY(&context->assets.asteroids[ii].physics));
		}
		else if(physics2d_getX(&context->assets.asteroids[ii].physics) < 0) {
			physics2d_setPosition(&context->assets.asteroids[ii].physics,
					context->engine.canvas.screenWidth,
					physics2d_getY(&context->assets.asteroids[ii].physics));
		}

		if(physics2d_getY(&context->assets.asteroids[ii].physics) >
				context->engine.canvas.screenHeight) {
			physics2d_setPosition(&context->assets.asteroids[ii].physics,
					physics2d_getX(&context->assets.asteroids[ii].physics),
					0);
		}
		else if(physics2d_getY(&context->assets.asteroids[ii].physics)  < 0) {
			physics2d_setPosition(&context->assets.asteroids[ii].physics,
					physics2d_getX(&context->assets.asteroids[ii].physics),
					context->engine.canvas.screenHeight);
		}
	}

	aliveTime = (int)time_getElapsedMusInSeconds(
			context->assets.fighterData.aliveTimestamp);

	if((aliveTime > 0) &&
			(aliveTime > aliveTimestamp) &&
			((aliveTime % 8) == 0)) {
		asteroid_buildNewAsteroid(context, TROIDS_ASTEROID_TYPE_RANDOM, -1, -1);
		aliveTimestamp = aliveTime;
	}

	// process bullets

	if(context->assets.fighterData.hasBullets) {
		bullet_fire(context);
	}

	for(ii = 0; ii < TROIDS_MAX_BULLETS; ii++) {
		if(!context->assets.bullets[ii].isAlive) {
			continue;
		}

		if(time_getElapsedMusInSeconds(
					context->assets.bullets[ii].timestamp) >= 2.0) {
			context->assets.bullets[ii].isAlive = afalse;
			continue;
		}

		physics2d_processGameplay(&context->assets.bullets[ii].physics);

		if(physics2d_getX(&context->assets.bullets[ii].physics) >
				context->engine.canvas.screenWidth) {
			context->assets.bullets[ii].isAlive = afalse;
			continue;
		}
		else if(physics2d_getX(&context->assets.bullets[ii].physics) < 0) {
			context->assets.bullets[ii].isAlive = afalse;
			continue;
		}

		if(physics2d_getY(&context->assets.bullets[ii].physics) >
				context->engine.canvas.screenHeight) {
			context->assets.bullets[ii].isAlive = afalse;
			continue;
		}
		else if(physics2d_getY(&context->assets.bullets[ii].physics)  < 0) {
			context->assets.bullets[ii].isAlive = afalse;
			continue;
		}
	}

	// process missiles

	if(context->assets.fighterData.hasMissiles) {
		missile_fire(context);
	}

	for(ii = 0; ii < TROIDS_MAX_MISSILES; ii++) {
		if(!context->assets.missiles[ii].isAlive) {
			continue;
		}

		if(time_getElapsedMusInSeconds(
					context->assets.missiles[ii].timestamp) >= 2.5) {
			animation2d_setPosition(&context->assets.missileAnimation,
				(context->assets.missiles[ii].sprite.position.x -
				 (context->assets.missiles[ii].sprite.position.width / 2)),
				(context->assets.missiles[ii].sprite.position.y -
				 (context->assets.missiles[ii].sprite.position.height / 2)));

			animation2d_start(&context->assets.missileAnimation, atrue);

			context->assets.missiles[ii].isAlive = afalse;
			continue;
		}

		missile_processPhysics(context, ii);

		physics2d_processGameplay(&context->assets.missiles[ii].physics);

		if(physics2d_getX(&context->assets.missiles[ii].physics) >
				context->engine.canvas.screenWidth) {
			physics2d_setPosition(&context->assets.missiles[ii].physics, 0,
					physics2d_getY(&context->assets.missiles[ii].physics));
		}
		else if(physics2d_getX(&context->assets.missiles[ii].physics) < 0) {
			physics2d_setPosition(&context->assets.missiles[ii].physics,
					context->engine.canvas.screenWidth,
					physics2d_getY(&context->assets.missiles[ii].physics));
		}

		if(physics2d_getY(&context->assets.missiles[ii].physics) >
				context->engine.canvas.screenHeight) {
			physics2d_setPosition(&context->assets.missiles[ii].physics,
					physics2d_getX(&context->assets.missiles[ii].physics),
					0);
		}
		else if(physics2d_getY(&context->assets.missiles[ii].physics)  < 0) {
			physics2d_setPosition(&context->assets.missiles[ii].physics,
					physics2d_getX(&context->assets.missiles[ii].physics),
					context->engine.canvas.screenHeight);
		}
	}

	// process powerups

	if(time_getElapsedMusInSeconds(
				context->assets.fighterData.powerupTimestamp) >= 5.0) {
		powerup_buildNewPowerup(context);
	}

	for(ii = 0; ii < TROIDS_POWERUP_TYPE_LENGTH; ii++) {
		if(!context->assets.powerups[ii].isAlive) {
			continue;
		}

		physics2d_processGameplay(&context->assets.powerups[ii].physics);

		if(physics2d_getX(&context->assets.powerups[ii].physics) >
				context->engine.canvas.screenWidth) {
			physics2d_setPosition(&context->assets.powerups[ii].physics, 0,
					physics2d_getY(&context->assets.powerups[ii].physics));
		}
		else if(physics2d_getX(&context->assets.powerups[ii].physics) < 0) {
			physics2d_setPosition(&context->assets.powerups[ii].physics,
					context->engine.canvas.screenWidth,
					physics2d_getY(&context->assets.powerups[ii].physics));
		}

		if(physics2d_getY(&context->assets.powerups[ii].physics) >
				context->engine.canvas.screenHeight) {
			physics2d_setPosition(&context->assets.powerups[ii].physics,
					physics2d_getX(&context->assets.powerups[ii].physics),
					0);
		}
		else if(physics2d_getY(&context->assets.powerups[ii].physics)  < 0) {
			physics2d_setPosition(&context->assets.powerups[ii].physics,
					physics2d_getX(&context->assets.powerups[ii].physics),
					context->engine.canvas.screenHeight);
		}
	}

	// process 3d cube

	context->assets.cubeData.xRotation += (float)0.005;
	context->assets.cubeData.yRotation += (float)0.005;
	context->assets.cubeData.zRotation += (float)0.005;

	// handle collisions

	if(!animation2d_isPlaying(&context->assets.fighterAnimation)) {
		if(context->assets.fighterData.respawnsRemaining < 0) {
			context->state = TROIDS_STATE_GAME_END;

			context->assets.fighterData.finalScore = (
					(int)time_getElapsedMusInSeconds(
						context->assets.fighterData.aliveTimestamp) +
					context->assets.fighterData.asteroidsDestroyed);
		}

		for(ii = 0; ii < TROIDS_MAX_ASTEROIDS; ii++) {
			if(!context->assets.asteroids[ii].isAlive) {
				continue;
			}

			if(game_isCollided(&context->assets.fighter,
						&context->assets.asteroids[ii].sprite)) {
				if((context->assets.fighterData.hasShields) &&
						(context->assets.fighterData.shieldStrength > 0)) {
					context->assets.fighterData.shieldStrength -= 1;

					if(context->assets.fighterData.shieldStrength <= 0) {
						context->assets.fighterData.hasShields = afalse;
						context->assets.fighterData.shieldStrength = 0;
					}

					xPosition = (
						context->assets.asteroids[ii].sprite.position.x -
						(context->assets.asteroids[ii].sprite.position.width /
						 2));

					yPosition = (
						context->assets.asteroids[ii].sprite.position.y -
						 (context->assets.asteroids[ii].sprite.position.height
						  / 2));

					animation2d_setPosition(
							&context->assets.asteroids[ii].animation,
							xPosition, yPosition);

					animation2d_resize(
						&context->assets.asteroids[ii].animation,
						(context->assets.asteroids[ii].sprite.position.width *
						 2),
						(context->assets.asteroids[ii].sprite.position.height *
						 2));

					animation2d_start(&context->assets.asteroids[ii].animation,
							atrue);

					xPosition = (
						context->assets.asteroids[ii].sprite.position.x +
						(context->assets.asteroids[ii].sprite.position.width /
						 2));

					yPosition = (
						context->assets.asteroids[ii].sprite.position.y +
						 (context->assets.asteroids[ii].sprite.position.height
						  / 2));

					switch(context->assets.asteroids[ii].type) {
						case TROIDS_ASTEROID_TYPE_BIG:
							length = ((rand() % 4) + 1);

							for(jj = 0; jj < length; jj++) {
								asteroid_buildNewAsteroid(context,
										TROIDS_ASTEROID_TYPE_MEDIUM,
										xPosition, yPosition);
							}
							break;

						case TROIDS_ASTEROID_TYPE_MEDIUM:
							length = ((rand() % 3) + 1);

							for(jj = 0; jj < length; jj++) {
								id = asteroid_buildNewAsteroid(context,
										TROIDS_ASTEROID_TYPE_SMALL,
										xPosition, yPosition);
							}
							break;

						case TROIDS_ASTEROID_TYPE_SMALL:
							if((aliveTime > 0) &&
									((4 + (aliveTime / 8)) >=
									 asteroid_getAliveCount(context))) {
								asteroid_buildNewAsteroid(context,
										TROIDS_ASTEROID_TYPE_RANDOM,
										-1, -1);
							}
							break;
					}

					context->assets.asteroids[ii].isAlive = afalse;
					context->assets.missiles[nn].isAlive = afalse;

					context->assets.fighterData.asteroidsDestroyed += 1;

					continue;
				}
				else {
					animation2d_setPosition(&context->assets.fighterAnimation,
							(context->assets.fighter.position.x -
							 (context->assets.fighter.position.width / 2)),
							(context->assets.fighter.position.y -
							 (context->assets.fighter.position.height / 2)));
					animation2d_start(&context->assets.fighterAnimation, atrue);

					context->assets.fighterData.isAlive = afalse;
					context->assets.fighterData.hasShields = afalse;
					context->assets.fighterData.shieldStrength = 0;
					context->assets.fighterData.respawnsRemaining -= 1;
				}
			}

			for(nn = 0; nn < TROIDS_MAX_BULLETS; nn++) {
				if(!context->assets.bullets[nn].isAlive) {
					continue;
				}

				if(game_isCollided(&context->assets.bullets[nn].sprite,
							&context->assets.asteroids[ii].sprite)) {
					xPosition = (
						context->assets.asteroids[ii].sprite.position.x -
						(context->assets.asteroids[ii].sprite.position.width /
						 2));

					yPosition = (
						context->assets.asteroids[ii].sprite.position.y -
						 (context->assets.asteroids[ii].sprite.position.height
						  / 2));

					animation2d_setPosition(
							&context->assets.asteroids[ii].animation,
							xPosition, yPosition);

					animation2d_resize(&context->assets.asteroids[ii].animation,
						(context->assets.asteroids[ii].sprite.position.width *
						 2),
						(context->assets.asteroids[ii].sprite.position.height *
						 2));

					animation2d_start(&context->assets.asteroids[ii].animation,
							atrue);

					xPosition = (
						context->assets.asteroids[ii].sprite.position.x +
						(context->assets.asteroids[ii].sprite.position.width /
						 2));

					yPosition = (
						context->assets.asteroids[ii].sprite.position.y +
						 (context->assets.asteroids[ii].sprite.position.height
						  / 2));

					switch(context->assets.asteroids[ii].type) {
						case TROIDS_ASTEROID_TYPE_BIG:
							length = ((rand() % 4) + 1);

							for(jj = 0; jj < length; jj++) {
								id = asteroid_buildNewAsteroid(context,
										TROIDS_ASTEROID_TYPE_MEDIUM,
										xPosition, yPosition);
							}
							break;

						case TROIDS_ASTEROID_TYPE_MEDIUM:
							length = ((rand() % 3) + 1);

							for(jj = 0; jj < length; jj++) {
								id = asteroid_buildNewAsteroid(context,
										TROIDS_ASTEROID_TYPE_SMALL,
										xPosition, yPosition);
							}
							break;

						case TROIDS_ASTEROID_TYPE_SMALL:
							if((aliveTime > 0) &&
									((4 + (aliveTime / 8)) >=
									 asteroid_getAliveCount(context))) {
								asteroid_buildNewAsteroid(context,
										TROIDS_ASTEROID_TYPE_RANDOM,
										-1, -1);
							}
							break;
					}

					context->assets.asteroids[ii].isAlive = afalse;
					context->assets.bullets[nn].isAlive = afalse;

					context->assets.fighterData.asteroidsDestroyed += 1;

					continue;
				}
			}

			for(nn = 0; nn < TROIDS_MAX_MISSILES; nn++) {
				if(!context->assets.missiles[nn].isAlive) {
					continue;
				}

				if(game_isCollided(&context->assets.missiles[nn].sprite,
							&context->assets.asteroids[ii].sprite)) {
					xPosition = (
						context->assets.asteroids[ii].sprite.position.x -
						(context->assets.asteroids[ii].sprite.position.width /
						 2));

					yPosition = (
						context->assets.asteroids[ii].sprite.position.y -
						 (context->assets.asteroids[ii].sprite.position.height
						  / 2));

					animation2d_setPosition(
							&context->assets.asteroids[ii].animation,
							xPosition, yPosition);

					animation2d_resize(&context->assets.asteroids[ii].animation,
						(context->assets.asteroids[ii].sprite.position.width *
						 2),
						(context->assets.asteroids[ii].sprite.position.height *
						 2));

					animation2d_start(&context->assets.asteroids[ii].animation,
							atrue);

					xPosition = (
						context->assets.asteroids[ii].sprite.position.x +
						(context->assets.asteroids[ii].sprite.position.width /
						 2));

					yPosition = (
						context->assets.asteroids[ii].sprite.position.y +
						 (context->assets.asteroids[ii].sprite.position.height
						  / 2));

					switch(context->assets.asteroids[ii].type) {
						case TROIDS_ASTEROID_TYPE_BIG:
							length = ((rand() % 4) + 1);

							for(jj = 0; jj < length; jj++) {
								id = asteroid_buildNewAsteroid(context,
										TROIDS_ASTEROID_TYPE_MEDIUM,
										xPosition, yPosition);
							}
							break;

						case TROIDS_ASTEROID_TYPE_MEDIUM:
							length = ((rand() % 3) + 1);

							for(jj = 0; jj < length; jj++) {
								id = asteroid_buildNewAsteroid(context,
										TROIDS_ASTEROID_TYPE_SMALL,
										xPosition, yPosition);
							}
							break;

						case TROIDS_ASTEROID_TYPE_SMALL:
							if((aliveTime > 0) &&
									((4 + (aliveTime / 8)) >=
									 asteroid_getAliveCount(context))) {
								asteroid_buildNewAsteroid(context,
										TROIDS_ASTEROID_TYPE_RANDOM,
										-1, -1);
							}
							break;
					}

					context->assets.asteroids[ii].isAlive = afalse;
					context->assets.missiles[nn].isAlive = afalse;

					context->assets.fighterData.asteroidsDestroyed += 1;

					continue;
				}
			}
		}

		for(ii = 0; ii < TROIDS_POWERUP_TYPE_LENGTH; ii++) {
			if(!context->assets.powerups[ii].isAlive) {
				continue;
			}

			if(game_isCollided(&context->assets.fighter,
						&context->assets.powerups[ii].sprite)) {
				switch(context->assets.powerups[ii].type) {
					case TROIDS_POWERUP_TYPE_GUN:
						context->assets.fighterData.hasBullets = atrue;
						break;

					case TROIDS_POWERUP_TYPE_MISSILE:
						context->assets.fighterData.hasMissiles = atrue;
						break;

					case TROIDS_POWERUP_TYPE_SHIELD:
						context->assets.fighterData.hasShields = atrue;
						context->assets.fighterData.shieldStrength = 3;
						break;
				}

				context->assets.powerups[ii].isAlive = afalse;
			}
		}
	}
	else {
		animation2d_setPosition(&context->assets.fighterAnimation,
				(context->assets.fighter.position.x -
				 (context->assets.fighter.position.width / 2)),
				(context->assets.fighter.position.y -
				 (context->assets.fighter.position.height / 2)));
	}

	// process animation sequences

	animation2d_processAnimation(&context->assets.fighterAnimation);
	animation2d_processAnimation(&context->assets.missileAnimation);

	for(ii = 0; ii < TROIDS_MAX_ASTEROIDS; ii++) {
		animation2d_processAnimation(&context->assets.asteroids[ii].animation);
	}
}


// define troids public functions

int troids_init(Troids *context)
{
	if(context == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	memset(context, 0, (int)(sizeof(Troids)));

	context->state = TROIDS_STATE_INIT;
	context->name = strdup("Troids");

	return 0;
}

int troids_free(Troids *context)
{
	if(context == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(context->name != NULL) {
		free(context->name);
	}

	if(context->state < TROIDS_STATE_INIT) {
		return 0;
	}

	sprite2d_free(&context->assets.fighter);

	texture_freePtr(context->assets.splashScreen);
	texture_freePtr(context->assets.cube);
	texture_freePtr(context->assets.background);
	texture_freePtr(context->assets.asteroidBig);

	return 0;
}

int troids_handleMouseEvents(Troids *context, AsgardMouseEvent *mouseEvent)
{
	int ii = 0;

	if((context == NULL) || (mouseEvent == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	switch(context->state) {
		case TROIDS_STATE_SPLASH_SCREEN:
			if(mouseEvent->eventType == EVENT_MOUSE_PRESSED) {
				if((mouseEvent->xPosition >= 146) &&
						(mouseEvent->xPosition <= 408) &&
						(mouseEvent->yPosition >= 408) &&
						(mouseEvent->yPosition <= 475)) {
					context->state = TROIDS_STATE_GAME_START;
				}
				else if((mouseEvent->xPosition >= 65) &&
						(mouseEvent->xPosition <= 400) &&
						(mouseEvent->yPosition >= 495) &&
						(mouseEvent->yPosition <= 535)) {
					context->state = TROIDS_STATE_INSTRUCTION_SCREEN;
				}
			}
			break;

		case TROIDS_STATE_INSTRUCTION_SCREEN:
			if(mouseEvent->eventType == EVENT_MOUSE_PRESSED) {
				context->state = TROIDS_STATE_SPLASH_SCREEN;
			}
			break;

		case TROIDS_STATE_GAME_START:
			break;

		case TROIDS_STATE_GAME_PLAYING:
			fighter_processMouseEvent(context, mouseEvent);
			break;

		case TROIDS_STATE_GAME_END:
			if(mouseEvent->eventType == EVENT_MOUSE_PRESSED) {
				context->assets.fighterData.isAlive = atrue;
				context->assets.fighterData.hasBullets = afalse;
				context->assets.fighterData.hasMissiles = afalse;
				context->assets.fighterData.hasShields = afalse;
				context->assets.fighterData.shieldStrength = 0;
				context->assets.fighterData.respawnsRemaining = 3;

				physics2d_reset(&context->assets.fighterPhysics);

				physics2d_setPosition(&context->assets.fighterPhysics,
						((context->engine.canvas.screenWidth - 64) / 2),
						((context->engine.canvas.screenHeight - 64) / 2));

				for(ii = 0; ii < TROIDS_MAX_ASTEROIDS; ii++) {
					context->assets.asteroids[ii].isAlive = afalse;
					animation2d_stop(&context->assets.asteroids[ii].animation);
				}

				for(ii = 0; ii < TROIDS_MAX_BULLETS; ii++) {
					context->assets.bullets[ii].isAlive = afalse;
				}

				for(ii = 0; ii < TROIDS_MAX_MISSILES; ii++) {
					context->assets.missiles[ii].isAlive = afalse;
				}

				for(ii = 0; ii < TROIDS_POWERUP_TYPE_LENGTH; ii++) {
					context->assets.powerups[ii].isAlive = afalse;
				}

				animation2d_stop(&context->assets.fighterAnimation);
				animation2d_stop(&context->assets.missileAnimation);

				context->state = TROIDS_STATE_GAME_START;
			}
			break;

		case TROIDS_STATE_INIT:
		case TROIDS_STATE_LOADING:
			return 0;

		case TROIDS_STATE_ERROR:
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

int troids_render(Troids *context)
{
	Canvas *canvas = NULL;

	if(context == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(context->state == TROIDS_STATE_ERROR) {
		return 0;
	}
/*
	if((context->state == TROIDS_STATE_INIT) ||
			(context->state == TROIDS_STATE_ERROR)) {
		return 0;
	}
*/
	canvas = &(context->engine.canvas);

	// load asets

	switch(context->state) {
		case TROIDS_STATE_INIT:
			if(load_loadingScreen(context) < 0) {
				context->state = TROIDS_STATE_ERROR;
				return -1;
			}
			else {
				context->state = TROIDS_STATE_LOADING;
				return 0;
			}
			break;

		case TROIDS_STATE_LOADING:
			render_loadingScreen(context, canvas);

#if defined(__ANDROID__)
			initCounter++;
			if(initCounter < 8) {
				break;
			}
#endif // __ANDROID__

			if(load_gameAssets(context) < 0) {
				context->state = TROIDS_STATE_ERROR;
				return -1;
			}
			else {
				context->state = TROIDS_STATE_SPLASH_SCREEN;
				return 0;
			}
			break;
	}

	// begin render

	canvas_beginRender(canvas);

	switch(context->state) {
		case TROIDS_STATE_INIT:
			// do nothing
			break;

		case TROIDS_STATE_LOADING:
			render_loadingScreen(context, canvas);
			break;

		case TROIDS_STATE_SPLASH_SCREEN:
			render_splashScreen(context, canvas);
			break;

		case TROIDS_STATE_INSTRUCTION_SCREEN:
			render_instructionScreen(context, canvas);
			break;

		case TROIDS_STATE_GAME_START:
			break;

		case TROIDS_STATE_GAME_PLAYING:
			render_playing(context, canvas);
			break;

		case TROIDS_STATE_GAME_END:
			render_gameOverScreen(context, canvas);
			break;
	}

	// end render

	canvas_endRender(canvas);

	return 0;
}

int troids_processGamepley(Troids *context)
{
#if defined(__ANDROID__)
	static int initCounter = 0;
#endif // __ANDROID__

	if(context == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	switch(context->state) {
		case TROIDS_STATE_INIT:
/*
			if(load_loadingScreen(context) < 0) {
				context->state = TROIDS_STATE_ERROR;
			}
			else {
				context->state = TROIDS_STATE_LOADING;
			}
*/
			break;

		case TROIDS_STATE_LOADING:
/*
#if defined(__ANDROID__)
			initCounter++;
			if(initCounter < 8) {
				break;
			}
#endif // __ANDROID__
			if(load_gameAssets(context) < 0) {
				context->state = TROIDS_STATE_ERROR;
			}
			else {
				context->state = TROIDS_STATE_SPLASH_SCREEN;
			}
*/
			break;

		case TROIDS_STATE_SPLASH_SCREEN:
			break;

		case TROIDS_STATE_INSTRUCTION_SCREEN:
			break;

		case TROIDS_STATE_GAME_START:
			asteroid_buildNewAsteroid(context, TROIDS_ASTEROID_TYPE_RANDOM,
					-1, -1);
			asteroid_buildNewAsteroid(context, TROIDS_ASTEROID_TYPE_RANDOM,
					-1, -1);
			asteroid_buildNewAsteroid(context, TROIDS_ASTEROID_TYPE_RANDOM,
					-1, -1);
			asteroid_buildNewAsteroid(context, TROIDS_ASTEROID_TYPE_RANDOM,
					-1, -1);

	
			context->assets.fighterData.isAlive = atrue;
			context->assets.fighterData.hasBullets = afalse;
			context->assets.fighterData.hasMissiles = afalse;
			context->assets.fighterData.hasShields = afalse;
			context->assets.fighterData.shieldStrength = 0;
			context->assets.fighterData.respawnsRemaining = 3;
			context->assets.fighterData.asteroidsDestroyed = 0;
			context->assets.fighterData.currentThrust =
				TROIDS_ASTEROIDS_STARTING_THRUST;
			context->assets.fighterData.aliveTimestamp = time_getTimeMus();
			context->assets.fighterData.bulletTimestamp = time_getTimeMus();
			context->assets.fighterData.missileTimestamp = time_getTimeMus();
			context->assets.fighterData.powerupTimestamp = time_getTimeMus();

			context->state = TROIDS_STATE_GAME_PLAYING;
			break;

		case TROIDS_STATE_GAME_PLAYING:
			gameplay_processPlaying(context);
			break;

		case TROIDS_STATE_GAME_END:
			break;

			return 0;

		case TROIDS_STATE_ERROR:
			return -1;
	}

	return 0;
}

int troids_ioFunction(void *engineContext, void *gameContext, void *argument)
{
	AsgardEvent *event = NULL;
	AsgardKeyboardEvent *keyboardEvent = NULL;
	AsgardMouseEvent *mouseEvent = NULL;
	Engine *engine = NULL;
	Troids *context = NULL;

	engine = (Engine *)engineContext;
	context = (Troids *)gameContext;
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

			troids_handleMouseEvents(context, mouseEvent);

			break;
	}

	return 0;
}

int troids_renderFunction(void *engineContext, void *gameContext,
		void *argument)
{
	Engine *engine = NULL;
	Troids *context = NULL;

	engine = (Engine *)engineContext;
	context = (Troids *)gameContext;

	return troids_render(context);
}

int troids_gameplayFunction(void *engineContext, void *gameContext,
		void *argument)
{
	Engine *engine = NULL;
	Troids *context = NULL;

	engine = (Engine *)engineContext;
	context = (Troids *)gameContext;

	return troids_processGamepley(context);
}

