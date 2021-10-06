/*
 * asgard-shell.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * A simple shell wrapper for the Asgard Game Engine designed to execute
 * applications built on the engine API.
 *
 * Written by Josh English.
 */


// preprocessor directives

#include "core/core.h"
#include "asgard/asgard.h"

#define _APPS_TROIDS_COMPONENT
#include "apps/troids/troids.h"

#define _APPS_DEMO_COMPONENT
#include "apps/demo/demo.h"


// define Asgard shell constants

#define COLOR_BITS									32

#define USE_FULLSCREEN_MODE							afalse

#define SCREEN_WIDTH								480

#define SCREEN_HEIGHT								800

#define SHELL_EXECUTE_TROIDS
//#define SHELL_EXECUTE_DEMO


// define Asgard shell components

#if defined(SHELL_EXECUTE_TROIDS)

#	define SHELL_INIT_FUNCTION							troids_init
#	define SHELL_FREE_FUNCTION							troids_free
#	define SHELL_IO_FUNCTION							troids_ioFunction
#	define SHELL_RENDER_FUNCTION						troids_renderFunction
#	define SHELL_PHYSICS_FUNCTION						NULL
#	define SHELL_GAMEPLAY_FUNCTION						troids_gameplayFunction

#elif defined(SHELL_EXECUTE_DEMO)

#	define SHELL_INIT_FUNCTION							demo_init
#	define SHELL_FREE_FUNCTION							demo_free
#	define SHELL_IO_FUNCTION							demo_ioFunction
#	define SHELL_RENDER_FUNCTION						demo_renderFunction
#	define SHELL_PHYSICS_FUNCTION						NULL
#	define SHELL_GAMEPLAY_FUNCTION						demo_gameplayFunction

#else // - uknown shell application -

#	error "unknown shell application"

#endif


// define Asgard shell global variables

#if defined(SHELL_EXECUTE_TROIDS)

Troids context;

#elif defined(SHELL_EXECUTE_DEMO)

Demo context;

#else // - uknown shell application -
#	error "unknown shell application"
#endif


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

	signal_registerDefault();

	// initialize shell context

	if(SHELL_INIT_FUNCTION(&context)) {
		return 1;
	}

	// initialize game engine

	snprintf(buffer, sizeof(buffer),
			"Asgard Game Engine Shell '%s' at %ix%i",
			context.name, SCREEN_WIDTH, SCREEN_HEIGHT);

	if(engine_init(
				&context.engine,
				NULL,
				buffer,
				(void *)&context,
				atrue,
				USE_FULLSCREEN_MODE,
				ASGARD_ENGINE_DEVELOPMENT_BUILD,
				SCREEN_WIDTH,
				SCREEN_HEIGHT,
				COLOR_BITS,
				SHELL_IO_FUNCTION,
				SHELL_RENDER_FUNCTION,
				SHELL_PHYSICS_FUNCTION,
				SHELL_GAMEPLAY_FUNCTION
				) < 0) {
		return -1;
	}

//	log_setFlushOutput(&context.engine.log, atrue);

	// multi-threaded game engine loop

	while(!context.engine.exitApplication) {
		if((rc = osapi_checkState(context.engine.window,
						&context.engine.exitApplication)) != 0) {
			break;
		}

		// take a nap

		time_usleep(1024);
	}

	// cleanup

	engine_free(&context.engine, atrue);

	SHELL_FREE_FUNCTION(&context);

	return rc;
}

