/*
 * pokerGamePrototype.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard Game Engine 2d engine poker game prototype application.
 *
 * Written by Josh English.
 */


// preprocessor directives

#include "core/core.h"
#include "asgard/asgard.h"


// define Asgard game engine 2d poker game prototype constants

#define COLOR_BITS										32

#define USE_FULLSCREEN_MODE								afalse

#define SCREEN_WIDTH									1024

#define SCREEN_HEIGHT									768

#define FONT_SIZE										14

#define TEXTURE_QUALITY									TEXTURE_QUALITY_HIGHEST
#define TEXTURE_RENDER_MODE								TEXTURE_RENDER_REPEAT
#define TEXTURE_HARDWARE_RESIDENT						atrue

#define MOUSE_TEXTURE									\
	"assets/textures/gui/mouse/fractal/fractal.config"

#define GAME_SCREEN_WIDTH								320
#define GAME_SCREEN_HEIGHT								480

#define GAME_BACKGROUND_TEXTURE							\
	"assets/poker/textures/dev-background.config"


// define Asgard game engine 2d poker game prototype data types

typedef struct _PokerGameWindow {
	int x;
	int y;
	int width;
	int height;
	Texture *background;
} PokerGameWindow;

typedef struct _PokerGameState {
	aboolean isLoaded;
	aboolean isShutdown;
	Texture *cardTexture;
	Texture *mouseTexture;
	PokerGameWindow window;
	Log log;
	Engine engine;
	Mutex mutex;
} PokerGameState;


// declare Asgard game engine 2d poker game prototype functions

static int localIoFunction(void *engineContext, void *gameContext,
		void *argument);

static int localRenderFunction(void *engineContext, void *gameContext,
		void *argument);

static int localGameplayFunction(void *engineContext, void *gameContext,
		void *argument);

static int pokerGame_loadAssets(PokerGameState *state);

static int pokerGame_freeAssets(PokerGameState *state);


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

	PokerGameState state;

	signal_registerDefault();

	// initialize poker game state

	memset((void *)&state, 0, sizeof(PokerGameState));

	mutex_init(&state.mutex);
	mutex_lock(&state.mutex);

	state.isLoaded = afalse;
	state.isShutdown = afalse;
	state.cardTexture = NULL;
	state.mouseTexture = NULL;

	state.window.x = ((SCREEN_WIDTH - GAME_SCREEN_WIDTH) / 2);
	state.window.y = ((SCREEN_HEIGHT - GAME_SCREEN_HEIGHT) / 2);
	state.window.width = GAME_SCREEN_WIDTH;
	state.window.height = GAME_SCREEN_HEIGHT;

	mutex_unlock(&state.mutex);

	log_init(&state.log, LOG_OUTPUT_FILE,
			"log/asgard.engine.2d.poker.game.prototype.log",
			LOG_LEVEL_DEBUG);

	// initialize game engine

	snprintf(buffer, sizeof(buffer),
			"Asgard Game Engine 2d Poker Game Prototype %ix%i",
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

//	osapi_disableMouseDisplay(state.engine.window);

	// multi-threaded game engine loop

	while(!state.engine.exitApplication) {
		if((rc = osapi_checkState(state.engine.window,
						&state.engine.exitApplication)) != 0) {
			break;
		}

		// take a nap

		time_usleep(1024);
	}

	mutex_lock(&state.mutex);
	state.isShutdown = atrue;
	mutex_unlock(&state.mutex);

	// execute shutdown

	if(pokerGame_freeAssets(&state) < 0) {
		return 1;
	}

	// cleanup

	engine_free(&state.engine, atrue);

	return rc;
}


// define Asgard game engine 2d poker game prototype functions

static int localIoFunction(void *engineContext, void *gameContext,
		void *argument)
{
	AsgardEvent *event = NULL;
	AsgardKeyboardEvent *keyboardEvent = NULL;
	AsgardMouseEvent *mouseEvent = NULL;
	Engine *engine = NULL;
	PokerGameState *state = NULL;

	engine = (Engine *)engineContext;
	state = (PokerGameState *)gameContext;

	mutex_lock(&state->mutex);

	if(!state->isLoaded) {
		mutex_unlock(&state->mutex);
		return 0;
	}
	else if(state->isShutdown) {
		mutex_unlock(&state->mutex);
		return 0;
	}

	mutex_unlock(&state->mutex);

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
				/*
				 * TODO: this
				 */
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

			if(mouseEvent->eventType == EVENT_MOUSE_PRESSED) {
				/*
				 * TODO: this
				 */
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
	PokerGameState *state = NULL;

	engine = (Engine *)engineContext;
	state = (PokerGameState *)gameContext;

	// render poker game state

	mutex_lock(&state->mutex);

	if(!state->isLoaded) {
		mutex_unlock(&state->mutex);

		canvas_beginRender(&engine->canvas);
		canvas_toggleRenderMode3d(&engine->canvas);
		canvas_toggleRenderMode2d(&engine->canvas);
		CGI_COLOR_WHITE;
		font_draw2d(&engine->font, &engine->canvas, 4, 0,
				"Loading...");
		canvas_endRender(&engine->canvas);

		return 0;
	}
	else if(state->isShutdown) {
		mutex_unlock(&state->mutex);

		canvas_beginRender(&engine->canvas);
		canvas_toggleRenderMode3d(&engine->canvas);
		canvas_toggleRenderMode2d(&engine->canvas);
		CGI_COLOR_WHITE;
		font_draw2d(&engine->font, &engine->canvas, 4, 0,
				"Shutting down...");
		canvas_endRender(&engine->canvas);

		return 0;
	}

	mutex_unlock(&state->mutex);

	// begin render

	canvas_beginRender(&engine->canvas);

	// 3d objects

	canvas_toggleRenderMode3d(&engine->canvas);

	// 2d objects

	canvas_toggleRenderMode2d(&engine->canvas);

	CGI_COLOR_WHITE;

	font_draw2d(&engine->font, &engine->canvas, 4, 0, "Framerate: %0.2f",
			engine->canvas.framerate);

	font_draw2d(&engine->font, &engine->canvas, 4, 20,
			"Mouse: [%03i][%03i]",
			state->engine.mouse.x, state->engine.mouse.y);
	font_draw2d(&engine->font, &engine->canvas, 4, 40,
			"Keyboard: '%s'",
			state->engine.keyboard.buffer);

	// create a blue color

	color.r = CGI_COLOR_BLUE_RED;
	color.g = CGI_COLOR_BLUE_GREEN;
	color.b = CGI_COLOR_BLUE_BLUE;
	color.a = CGI_COLOR_BLUE_ALPHA;

	// draw the game's visible area

	font_draw2d(&engine->font, &engine->canvas,
			state->window.x, (state->window.y - 20),
			"Poker Game (%ix%i)",
			state->window.width, state->window.height);

	draw2d_texture(&engine->canvas,
			state->window.x, state->window.y,
			state->window.width, state->window.height,
			state->window.background);

	// draw the top-bar

	color.r = CGI_COLOR_BLUE_RED;
	color.g = CGI_COLOR_BLUE_GREEN;
	color.b = CGI_COLOR_BLUE_BLUE;
	color.a = CGI_COLOR_BLUE_ALPHA;

	draw2d_rectangle(&engine->canvas, &color,
			state->window.x, state->window.y,
			320, 58);

	color.r = CGI_COLOR_LIGHTGREY_RED;
	color.g = CGI_COLOR_LIGHTGREY_GREEN;
	color.b = CGI_COLOR_LIGHTGREY_BLUE;
	color.a = CGI_COLOR_LIGHTGREY_ALPHA;

	draw2d_fillRectangle(&engine->canvas, &color,
			(state->window.x + 4), (state->window.y + 4),
			192, 20);

	CGI_COLOR_BLUE;

	font_draw2d(&engine->font, &engine->canvas,
			(state->window.x + 4), (state->window.y + 4),
			"[top bar goes here]");


	// draw the game board

	color.r = CGI_COLOR_RED_RED;
	color.g = CGI_COLOR_RED_GREEN;
	color.b = CGI_COLOR_RED_BLUE;
	color.a = CGI_COLOR_RED_ALPHA;

	draw2d_rectangle(&engine->canvas, &color,
			state->window.x, (state->window.y + 60),
			320, 298);

	// draw the card layout (zoomed in)

/*	{
		int ii = 0;
		int nn = 0;
		int width = 0;
		int height = 0;

		for(ii = (state->window.x + 2), width = 0;
				((ii <= (state->window.x + state->window.width)) &&
				 (width < 9));
				ii += 35, width++) {
			for(nn = (state->window.y + 60), height = 0;
					((nn < (state->window.y + state->window.height)) &&
					 (height < 9));
					nn += 33, height++) {
				draw2d_texture(&engine->canvas,
						(ii + 7),
						(nn + 2),
						22,
						29,
						state->cardTexture);
			}
		}
	}*/

	// draw the card layout (zoomed out)

	{
		int ii = 0;
		int nn = 0;
		int width = 0;
		int height = 0;

		for(ii = (state->window.x + 2), width = 0;
				((ii <= (state->window.x + state->window.width)) &&
				 (width < 7));
				ii += 45, width++) {
			for(nn = (state->window.y + 60), height = 0;
					((nn < (state->window.y + state->window.height)) &&
					 (height < 7));
					nn += 42, height++) {
				draw2d_texture(&engine->canvas,
						(ii + 8),
						(nn + 2),
						28,
						38,
						state->cardTexture);
			}
		}
	}

	color.r = CGI_COLOR_LIGHTGREY_RED;
	color.g = CGI_COLOR_LIGHTGREY_GREEN;
	color.b = CGI_COLOR_LIGHTGREY_BLUE;
	color.a = CGI_COLOR_LIGHTGREY_ALPHA;

	draw2d_fillRectangle(&engine->canvas, &color,
			(state->window.x + 4), (state->window.y + 4 + 60),
			192, 20);

	CGI_COLOR_RED;

	font_draw2d(&engine->font, &engine->canvas,
			(state->window.x + 4), (state->window.y + 4 + 60),
			"[game board goes here]");

	// draw the tray

	color.r = CGI_COLOR_GREEN_RED;
	color.g = CGI_COLOR_GREEN_GREEN;
	color.b = CGI_COLOR_GREEN_BLUE;
	color.a = CGI_COLOR_GREEN_ALPHA;

	draw2d_rectangle(&engine->canvas, &color,
			state->window.x, (state->window.y + 60 + 300),
			320, 58);

	// draw the card layout

	{
		int ii = 0;
		int nn = 0;
		int width = 0;
		int height = 0;

		for(ii = (state->window.x + 2), width = 0;
				((ii <= (state->window.x + state->window.width)) &&
				 (width < 7));
				ii += 45, width++) {
			for(nn = (state->window.y + 60 + 300), height = 0;
					((nn < (state->window.y + state->window.height)) &&
					 (height < 1));
					nn += 60, height++) {
				draw2d_texture(&engine->canvas,
						(ii + 5),
						(nn + 5),
						35,
						50,
						state->cardTexture);
			}
		}
	}

	color.r = CGI_COLOR_LIGHTGREY_RED;
	color.g = CGI_COLOR_LIGHTGREY_GREEN;
	color.b = CGI_COLOR_LIGHTGREY_BLUE;
	color.a = CGI_COLOR_LIGHTGREY_ALPHA;

	draw2d_fillRectangle(&engine->canvas, &color,
			(state->window.x + 4), (state->window.y + 4 + 60 + 300),
			192, 20);

	CGI_COLOR_GREEN;

	font_draw2d(&engine->font, &engine->canvas,
			(state->window.x + 4), (state->window.y + 4 + 60 + 300),
			"[tray goes here]");

	// draw the interface

	color.r = CGI_COLOR_PURPLE_RED;
	color.g = CGI_COLOR_PURPLE_GREEN;
	color.b = CGI_COLOR_PURPLE_BLUE;
	color.a = CGI_COLOR_PURPLE_ALPHA;

	draw2d_rectangle(&engine->canvas, &color,
			state->window.x, (state->window.y + 60 + 300 + 60),
			320, 60);

	color.r = CGI_COLOR_LIGHTGREY_RED;
	color.g = CGI_COLOR_LIGHTGREY_GREEN;
	color.b = CGI_COLOR_LIGHTGREY_BLUE;
	color.a = CGI_COLOR_LIGHTGREY_ALPHA;

	draw2d_fillRectangle(&engine->canvas, &color,
			(state->window.x + 4), (state->window.y + 4 + 60 + 300 + 60),
			192, 20);

	CGI_COLOR_PURPLE;

	font_draw2d(&engine->font, &engine->canvas,
			(state->window.x + 4), (state->window.y + 4 + 60 + 300 + 60),
			"[interface goes here]");

	// draw the mouse at the current mouse position

	draw2d_texture(&engine->canvas,
			state->engine.mouse.x, state->engine.mouse.y, 24, 24,
			state->mouseTexture);

	// end render

	canvas_endRender(&engine->canvas);

	return 0;
}

static int localGameplayFunction(void *engineContext, void *gameContext,
		void *argument)
{
	Engine *engine = NULL;
	PokerGameState *state = NULL;

	engine = (Engine *)engineContext;
	state = (PokerGameState *)gameContext;

	// execute poker game

	mutex_lock(&state->mutex);

	if(!state->isLoaded) {
		mutex_unlock(&state->mutex);

		pokerGame_loadAssets(state);

		mutex_lock(&state->mutex);
		state->isLoaded = atrue;
		mutex_unlock(&state->mutex);

		return 0;
	}
	else if(state->isShutdown) {
		mutex_unlock(&state->mutex);
		return 0;
	}

	mutex_unlock(&state->mutex);

	/*
	 * TODO: gameplay logic goes here
	 */

	return 0;
}


// define Asgard game engine 2d poker game prototype - test functions

static int pokerGame_loadAssets(PokerGameState *state)
{
	Texture *texture = NULL;

	log_logf(&state->log, LOG_LEVEL_INFO,
			"poker game %s(): STARTED", __FUNCTION__);

	mutex_lock(&state->mutex);

	if((texture = texture_load(&state->log,
					"assets/poker/textures/deck/club-01.config")) == NULL) {
		mutex_unlock(&state->mutex);
		log_logf(&state->log, LOG_LEVEL_ERROR,
				"poker game %s(): FAILED", __FUNCTION__);
		return -1;
	}

	state->cardTexture = texture;

	if((texture = texture_load(&state->log, MOUSE_TEXTURE)) == NULL) {
		mutex_unlock(&state->mutex);
		log_logf(&state->log, LOG_LEVEL_ERROR,
				"poker game %s(): FAILED", __FUNCTION__);
		return -1;
	}

	state->mouseTexture = texture;

	if((texture = texture_load(&state->log, GAME_BACKGROUND_TEXTURE)) == NULL) {
		mutex_unlock(&state->mutex);
		log_logf(&state->log, LOG_LEVEL_ERROR,
				"poker game %s(): FAILED", __FUNCTION__);
		return -1;
	}

	state->window.background = texture;

	mutex_unlock(&state->mutex);

	log_logf(&state->log, LOG_LEVEL_INFO,
			"poker game %s(): PASSED", __FUNCTION__);

	return 0;
}

static int pokerGame_freeAssets(PokerGameState *state)
{
	log_logf(&state->log, LOG_LEVEL_INFO,
			"poker game %s(): STARTED", __FUNCTION__);

	mutex_lock(&state->mutex);

	if(state->cardTexture != NULL) {
		texture_freePtr(state->cardTexture);
	}

	if(state->mouseTexture != NULL) {
		texture_freePtr(state->mouseTexture);
	}

	if(state->window.background != NULL) {
		texture_freePtr(state->window.background);
	}

	mutex_unlock(&state->mutex);

	log_logf(&state->log, LOG_LEVEL_INFO,
			"poker game %s(): PASSED", __FUNCTION__);

	return 0;
}

