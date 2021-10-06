/*
 * engine.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard Game Engine central game engine component.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#include "asgard/asgard.h"
#define _ASGARD_ENGINE_COMPONENT
#include "asgard/engine/engine.h"

// define engine private functions

static int defaultIoFunction(void *engineContext, void *gameContext,
		void *argument)
{
	Engine *engine = NULL;
	AsgardEvent *event = NULL;
	AsgardOsEvent *osEvent = NULL;
	AsgardKeyboardEvent *keyboardEvent = NULL;
	AsgardMouseEvent *mouseEvent = NULL;

	engine = (Engine *)engineContext;
	event = (AsgardEvent *)argument;

	switch(event->eventType) {
		/*
		 * handle operating system events
		 */

		case EVENT_TYPE_OS:
			osEvent = (AsgardOsEvent *)event->event;

			log_logf(&engine->log, LOG_LEVEL_DEBUG,
					"[%s->%s()] os event #%i",
					engine->name, __FUNCTION__, osEvent->eventType);

			switch(osEvent->eventType) {
				case EVENT_OS_SCREEN_RESIZED:
					mutex_lock(&engine->screen.mutex);
					engine->screen.screenWidth = osEvent->screenWidth;
					engine->screen.screenHeight = osEvent->screenHeight;
					engine->screen.requiresResize = atrue;
					mutex_unlock(&engine->screen.mutex);

					log_logf(&engine->log, LOG_LEVEL_INFO,
							"[%s->%s()} reset canvas to %ix%i pixels",
							engine->name, __FUNCTION__,
							osEvent->screenWidth, osEvent->screenHeight);
					break;
			}
			break;

		/*
		 * handle keyboard events
		 */

		case EVENT_TYPE_KEYBOARD:
			keyboardEvent = (AsgardKeyboardEvent *)event->event;

			log_logf(&engine->log, LOG_LEVEL_DEBUG,
					"[%s->%s()] keyboard event #%i",
					engine->name, __FUNCTION__, keyboardEvent->eventType);

			switch(keyboardEvent->eventType) {
				case EVENT_KEYBOARD_PRESSED:
					if(((unsigned int)keyboardEvent->key >= 0) &&
							((unsigned int)keyboardEvent->key <= 255)) {
						mutex_lock(&engine->keyboard.mutex);
						engine->keyboard.keyState[
							(unsigned int)keyboardEvent->key] = 1;
						engine->keyboard.lastUpdatedTime = time_getTimeMus();
						mutex_unlock(&engine->keyboard.mutex);
					}
					switch(keyboardEvent->keyCode) {
						case ASGARD_KEY_ESCAPE:
							mutex_lock(&engine->mutex);
							engine->exitApplication = atrue;
							mutex_unlock(&engine->mutex);
							break;
						case ASGARD_KEY_RETURN:
							mutex_lock(&engine->keyboard.mutex);
							engine->keyboard.bufferRef = 0;
							memset((void *)engine->keyboard.buffer, 0,
									ASGARD_ENGINE_KEY_BUFFER_LENGTH);
							engine->keyboard.lastUpdatedTime =
								time_getTimeMus();
							mutex_unlock(&engine->keyboard.mutex);
							break;
					}
					break;
				case EVENT_KEYBOARD_TYPED:
					mutex_lock(&engine->keyboard.mutex);

					engine->keyboard.eventType = keyboardEvent->eventType;
					engine->keyboard.lastKeyCode = engine->keyboard.keyCode;
					engine->keyboard.keyCode = keyboardEvent->keyCode;

					if(((unsigned int)keyboardEvent->key >= 32) &&
							((unsigned int)keyboardEvent->key <= 126) &&
							(engine->keyboard.bufferRef <
							 (ASGARD_ENGINE_KEY_BUFFER_LENGTH - 1))
							) {
						engine->keyboard.lastKey = engine->keyboard.key;
						engine->keyboard.key = keyboardEvent->key;
						engine->keyboard.buffer[engine->keyboard.bufferRef] =
							keyboardEvent->key;
						engine->keyboard.bufferRef += 1;
					}

					engine->keyboard.lastUpdatedTime = time_getTimeMus();

					mutex_unlock(&engine->keyboard.mutex);
					break;
				case EVENT_KEYBOARD_RELEASED:
					if(((unsigned int)keyboardEvent->key >= 0) &&
							((unsigned int)keyboardEvent->key <= 255)) {
						mutex_lock(&engine->keyboard.mutex);
						engine->keyboard.keyState[
							(unsigned int)keyboardEvent->key] = 0;
						engine->keyboard.lastUpdatedTime = time_getTimeMus();
						mutex_unlock(&engine->keyboard.mutex);
					}
					break;
			}
			break;

		/*
		 * handle mouse events
		 */

		case EVENT_TYPE_MOUSE:
			mouseEvent = (AsgardMouseEvent *)event->event;

			log_logf(&engine->log, LOG_LEVEL_DEBUG,
					"[%s->%s()] mouse event #%i",
					engine->name, __FUNCTION__, mouseEvent->eventType);

			mutex_lock(&engine->mouse.mutex);

			engine->mouse.clickCount = mouseEvent->clickCount;
			engine->mouse.buttonType = mouseEvent->buttonType;
			engine->mouse.lastX = engine->mouse.x;
			engine->mouse.x = mouseEvent->xPosition;
			engine->mouse.lastY = engine->mouse.y;
			engine->mouse.y = mouseEvent->yPosition;
			engine->mouse.eventType = mouseEvent->eventType;
			engine->mouse.lastUpdatedTime = time_getTimeMus();

			mutex_unlock(&engine->mouse.mutex);
			break;
	}

	return 0;
}

static void *engineIoThread(void *threadContext, void *argument)
{
	int counter = 0;
	double timestamp = 0.0;

	AsgardEvent *event = NULL;
	TmfThread *thread = NULL;
	Engine *engine = NULL;

	thread = (TmfThread *)threadContext;
	engine = (Engine *)argument;

	log_logf(&engine->log, LOG_LEVEL_INFO,
			"{%s}[engine->%s()] thread #%i (%i) running",
			engine->name, __FUNCTION__, thread->uid, thread->systemId);

	// wait for engine to initialize

	while((thread->state == THREAD_STATE_RUNNING) &&
			(thread->signal == THREAD_SIGNAL_OK) &&
			(!engine->isInitialized)) {
		time_usleep(65536);
	}

	// handle i/o events

	while((thread->state == THREAD_STATE_RUNNING) &&
			(thread->signal == THREAD_SIGNAL_OK)) {
		timestamp = time_getTimeMus();

		mutex_lock(thread->mutex);
		thread->status = THREAD_STATUS_BUSY;
		mutex_unlock(thread->mutex);

		// obtain & handle events

		if(event == NULL) {
			event = osapi_popEvent(engine->window);
		}

		while(event != NULL) {
			if(defaultIoFunction(engine, engine->gameContext,
						event) < 0) {
				mutex_lock(thread->mutex);
				thread->state = THREAD_STATE_HALT;
				mutex_unlock(thread->mutex);
				break;
			}

			if(engine->ioThread.function != NULL) {
				if(engine->ioThread.function(engine, engine->gameContext,
							event) < 0) {
					mutex_lock(thread->mutex);
					thread->state = THREAD_STATE_HALT;
					mutex_unlock(thread->mutex);
					break;
				}
			}

			events_freeEvent(event);

			event = osapi_popEvent(engine->window);
		}

		if((thread->state != THREAD_STATE_RUNNING) ||
				(thread->signal != THREAD_SIGNAL_OK)) {
			break;
		}

		// check thread state

		mutex_lock(thread->mutex);
		thread->status = THREAD_STATUS_NAPPING;
		mutex_unlock(thread->mutex);

		// take a nap

		counter = 0;
		while(((event = osapi_popEvent(engine->window)) == NULL) &&
				(time_getElapsedMusInSeconds(timestamp) < 0.01) &&
				(thread->state == THREAD_STATE_RUNNING) &&
				(thread->signal == THREAD_SIGNAL_OK) &&
				(counter < 1024)) {
			time_usleep(1024);
			counter++;
		}
	}

	mutex_lock(thread->mutex);
	if(thread->state == THREAD_STATE_RUNNING) {
		thread->state = THREAD_STATE_HALT;
	}
	if(thread->signal == THREAD_SIGNAL_OK) {
		thread->signal = THREAD_SIGNAL_QUIT;
	}
	mutex_unlock(thread->mutex);

	log_logf(&engine->log, LOG_LEVEL_INFO,
			"{%s}[engine->%s()] thread #%i (%i) halted",
			engine->name, __FUNCTION__, thread->uid, thread->systemId);

	return NULL;
}

static int defaultRenderFunction(void *engineContext, void *gameContext,
		void *argument)
{
	Engine *engine = NULL;

	engine = (Engine *)engineContext;

	// begin render

	canvas_beginRender(&engine->canvas);

	// 3d objects

	canvas_toggleRenderMode3d(&engine->canvas);

	/*
	 * TODO: this
	 */

	// 2d objects

	canvas_toggleRenderMode2d(&engine->canvas);

	{
		CGI_COLOR_WHITE;
		font_draw2d(&engine->font, &engine->canvas, 4, 0,
				"Framerate: %0.2f", engine->canvas.framerate);
	}

	/*
	 * TODO: this
	 */

	// end render

	canvas_endRender(&engine->canvas);

	return 0;
}

static void *engineRenderThread(void *threadContext, void *argument)
{
	int napTime = 0;
	double ticks = 0.0;
	double timing = 0.0;
	double totalTime = 0.0;
	double timestamp = 0.0;

	TmfThread *thread = NULL;
	Engine *engine = NULL;

	thread = (TmfThread *)threadContext;
	engine = (Engine *)argument;

	log_logf(&engine->log, LOG_LEVEL_INFO,
			"{%s}[engine->%s()] thread #%i (%i) running",
			engine->name, __FUNCTION__, thread->uid, thread->systemId);

	// wait for engine to initialize

	while((thread->state == THREAD_STATE_RUNNING) &&
			(thread->signal == THREAD_SIGNAL_OK) &&
			(!engine->isInitialized)) {
		time_usleep(65536);
	}

	mutex_lock(&engine->mutex);

	osapi_makeOpenGLCurrent(engine->window, OSAPI_THREAD_TYPE_RENDER);

	mutex_unlock(&engine->mutex);

	// render current scene

	while((thread->state == THREAD_STATE_RUNNING) &&
			(thread->signal == THREAD_SIGNAL_OK)) {
		timestamp = time_getTimeMus();

		mutex_lock(thread->mutex);
		thread->status = THREAD_STATUS_BUSY;
		mutex_unlock(thread->mutex);

		// execute render

		if(engine->isOpenglInitialized) {
			if(engine->renderThread.function(engine, engine->gameContext,
						NULL) < 0) {
				mutex_lock(thread->mutex);
				thread->state = THREAD_STATE_HALT;
				mutex_unlock(thread->mutex);
				break;
			}
		}

		// check thread state

		if((thread->state != THREAD_STATE_RUNNING) ||
				(thread->signal != THREAD_SIGNAL_OK)) {
			break;
		}

		// take a nap

		mutex_lock(thread->mutex);
		thread->status = THREAD_STATUS_NAPPING;
		mutex_unlock(thread->mutex);

		timing = time_getElapsedMusInSeconds(timestamp);

		totalTime += timing;
		ticks += 1.0;

		if(((int)ticks % 120) == 0) {
			log_logf(&engine->log, LOG_LEVEL_DEBUG, "{%s}[engine->%s()] "
					"average render time: %0.6f (%0.2f fps capable)",
					engine->name, __FUNCTION__, (totalTime / ticks),
					(1.0 / (totalTime / ticks)));
		}

		if(timing < 0.016666667) {
			napTime = ((int)((0.016666667 - timing) * REAL_ONE_MILLION) - 1024);
			if(napTime > 0) {
				time_usleep(napTime);
			}
		}
	}

	mutex_lock(thread->mutex);
	if(thread->state == THREAD_STATE_RUNNING) {
		thread->state = THREAD_STATE_HALT;
	}
	if(thread->signal == THREAD_SIGNAL_OK) {
		thread->signal = THREAD_SIGNAL_QUIT;
	}
	mutex_unlock(thread->mutex);

	log_logf(&engine->log, LOG_LEVEL_INFO,
			"{%s}[engine->%s()] thread #%i (%i) halted",
			engine->name, __FUNCTION__, thread->uid, thread->systemId);

	return NULL;
}

static int defaultPhysicsFunction(void *engineContext, void *gameContext,
		void *argument)
{
	Engine *engine = NULL;

	engine = (Engine *)engineContext;

	/*
	 * TODO: this
	 */

	return 0;
}

static void *enginePhysicsThread(void *threadContext, void *argument)
{
	int napTime = 0;
	double timing = 0.0;
	double timestamp = 0.0;

	TmfThread *thread = NULL;
	Engine *engine = NULL;

	thread = (TmfThread *)threadContext;
	engine = (Engine *)argument;

	log_logf(&engine->log, LOG_LEVEL_INFO,
			"{%s}[engine->%s()] thread #%i (%i) running",
			engine->name, __FUNCTION__, thread->uid, thread->systemId);

	// wait for engine to initialize

	while((thread->state == THREAD_STATE_RUNNING) &&
			(thread->signal == THREAD_SIGNAL_OK) &&
			(!engine->isInitialized)) {
		time_usleep(65536);
	}

	// execute physics calculations

	while((thread->state == THREAD_STATE_RUNNING) &&
			(thread->signal == THREAD_SIGNAL_OK)) {
		timestamp = time_getTimeMus();

		mutex_lock(thread->mutex);
		thread->status = THREAD_STATUS_BUSY;
		mutex_unlock(thread->mutex);

		// calucate physics step

		if(engine->physicsThread.function(engine, engine->gameContext,
					NULL) < 0) {
			mutex_lock(thread->mutex);
			thread->state = THREAD_STATE_HALT;
			mutex_unlock(thread->mutex);
			break;
		}

		// check thread state

		if((thread->state != THREAD_STATE_RUNNING) ||
				(thread->signal != THREAD_SIGNAL_OK)) {
			break;
		}

		// take a nap

		mutex_lock(thread->mutex);
		thread->status = THREAD_STATUS_NAPPING;
		mutex_unlock(thread->mutex);

		timing = time_getElapsedMusInSeconds(timestamp);
		if(timing < 0.01) {
			napTime = ((int)((0.01 - timing) * REAL_ONE_MILLION) - 1024);
			if(napTime > 0) {
				time_usleep(napTime);
			}
		}
	}

	mutex_lock(thread->mutex);
	if(thread->state == THREAD_STATE_RUNNING) {
		thread->state = THREAD_STATE_HALT;
	}
	if(thread->signal == THREAD_SIGNAL_OK) {
		thread->signal = THREAD_SIGNAL_QUIT;
	}
	mutex_unlock(thread->mutex);

	log_logf(&engine->log, LOG_LEVEL_INFO,
			"{%s}[engine->%s()] thread #%i (%i) halted",
			engine->name, __FUNCTION__, thread->uid, thread->systemId);

	return NULL;
}

static int defaultGameplayFunction(void *engineContext, void *gameContext,
		void *argument)
{
	Engine *engine = NULL;

	engine = (Engine *)engineContext;

	/*
	 * TODO: this
	 */

	return 0;
}

static void *engineGameplayThread(void *threadContext, void *argument)
{
	int napTime = 0;
	double timing = 0.0;
	double timestamp = 0.0;
	char *stdBuffer = NULL;

	TmfThread *thread = NULL;
	Engine *engine = NULL;

	thread = (TmfThread *)threadContext;
	engine = (Engine *)argument;

	log_logf(&engine->log, LOG_LEVEL_INFO,
			"{%s}[engine->%s()] thread #%i (%i) running",
			engine->name, __FUNCTION__, thread->uid, thread->systemId);

	// wait for engine and rendering thread to initialize

	while((thread->state == THREAD_STATE_RUNNING) &&
			(thread->signal == THREAD_SIGNAL_OK) &&
			(!engine->isInitialized)) {
		time_usleep(65536);
	}

	mutex_lock(&engine->mutex);

	// initialize gameplay-thread OpenGL

	osapi_makeOpenGLCurrent(engine->window, OSAPI_THREAD_TYPE_GAMEPLAY);

	// initialize canvas

	if(canvas_init(
				&engine->canvas,
				engine->name,
				afalse,
				engine->screen.screenWidth,
				engine->screen.screenHeight,
				engine->windowHandle,
				&engine->log) < 0) {
		log_logf(&engine->log, LOG_LEVEL_ERROR,
				"{%s}[%s()] failed to initialize canvas",
				engine->name, __FUNCTION__);
		mutex_unlock(&engine->mutex);
		mutex_lock(thread->mutex);
		thread->state = THREAD_STATE_ERROR;
		mutex_unlock(thread->mutex);
		return NULL;
	}

	log_logf(&engine->log, LOG_LEVEL_INFO,
			"{%s}[%s()] canvas initialization complete",
			engine->name, __FUNCTION__);

	engine->isOpenglInitialized = atrue;

	mutex_unlock(&engine->mutex);

	// initialize default font

	if(font_init(
				&engine->font,
				engine->windowHandle,
				ASGARD_ENGINE_DEFAULT_FONT,
				ASGARD_ENGINE_DEFAULT_FONT_SIZE,
				afalse,
				afalse,
				afalse,
				FONT_CHARSET_ANSI
				) < 0) {
		log_logf(&engine->log, LOG_LEVEL_ERROR,
				"{%s}[%s()] failed to load default font '%s' size %i",
				engine->name, __FUNCTION__,
				ASGARD_ENGINE_DEFAULT_FONT,
				ASGARD_ENGINE_DEFAULT_FONT_SIZE);
		mutex_unlock(&engine->mutex);
		mutex_lock(thread->mutex);
		thread->state = THREAD_STATE_ERROR;
		mutex_unlock(thread->mutex);
		return NULL;
	}

	log_logf(&engine->log, LOG_LEVEL_INFO,
			"{%s}[%s()] default font loaded",
			engine->name, __FUNCTION__);

	// execute gameplay logic

	while((thread->state == THREAD_STATE_RUNNING) &&
			(thread->signal == THREAD_SIGNAL_OK)) {
		timestamp = time_getTimeMus();

		mutex_lock(thread->mutex);
		thread->status = THREAD_STATUS_BUSY;
		mutex_unlock(thread->mutex);

		// handle screen resizing

		if(engine->screen.requiresResize) {
			mutex_lock(&engine->mutex);
			mutex_lock(&engine->screen.mutex);
			canvas_setScreenDimensions(&engine->canvas,
					engine->screen.screenWidth,
					engine->screen.screenHeight);
			engine->screen.requiresResize = afalse;
			mutex_unlock(&engine->screen.mutex);
			mutex_unlock(&engine->mutex);
		}

		// execute gameplay

		if(engine->gameplayThread.function(engine, engine->gameContext,
					NULL) < 0) {
			mutex_lock(thread->mutex);
			thread->state = THREAD_STATE_HALT;
			mutex_unlock(thread->mutex);
			break;
		}

		// check thread state

		if((thread->state != THREAD_STATE_RUNNING) ||
				(thread->signal != THREAD_SIGNAL_OK)) {
			break;
		}

		// purge stderr & stdout streams

		while((stdBuffer = common_popStdStreamError()) != NULL) {
			log_logf(&engine->log, LOG_LEVEL_ERROR, "STDERR %s", stdBuffer);
		}

		while((stdBuffer = common_popStdStreamOutput()) != NULL) {
			log_logf(&engine->log, LOG_LEVEL_INFO, "STDOUT %s", stdBuffer);
		}

		// check thread state

		if((thread->state != THREAD_STATE_RUNNING) ||
				(thread->signal != THREAD_SIGNAL_OK)) {
			break;
		}

		// take a nap

		mutex_lock(thread->mutex);
		thread->status = THREAD_STATUS_NAPPING;
		mutex_unlock(thread->mutex);

		timing = time_getElapsedMusInSeconds(timestamp);
		if(timing < 0.01) {
			napTime = ((int)((0.01 - timing) * REAL_ONE_MILLION) - 1024);
			if(napTime > 0) {
				time_usleep(napTime);
			}
		}
	}

	mutex_lock(thread->mutex);
	if(thread->state == THREAD_STATE_RUNNING) {
		thread->state = THREAD_STATE_HALT;
	}
	if(thread->signal == THREAD_SIGNAL_OK) {
		thread->signal = THREAD_SIGNAL_QUIT;
	}
	mutex_unlock(thread->mutex);

	log_logf(&engine->log, LOG_LEVEL_INFO,
			"{%s}[engine->%s()] thread #%i (%i) halted",
			engine->name, __FUNCTION__, thread->uid, thread->systemId);

	return NULL;
}

// define engine public functions

int engine_init(Engine *engine, void *handle, char *name, void *gameContext,
		aboolean createWindow, aboolean isFullscreen, int buildType,
		int screenWidth, int screenHeight, int colorBits,
		EngineFunction ioFunction, EngineFunction renderFunction,
		EngineFunction physicsFunction, EngineFunction gameplayFunction)
{
	if((engine == NULL) ||
			((buildType != ASGARD_ENGINE_DEVELOPMENT_BUILD) &&
			 (buildType != ASGARD_ENGINE_RELEASE_BUILD)) ||
			(screenWidth < 320) || (screenHeight < 240) ||
			((colorBits != 8) &&
			 (colorBits != 16) &&
			 (colorBits != 24) &&
			 (colorBits != 32))) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	// sanity-check remaning arguments

	if(name == NULL) {
		name = "Project Asgard Game Engine";
	}

	// initialize engine context

	memset(engine, 0, sizeof(Engine));

	engine->isActive = atrue;
	engine->isInitialized = afalse;
	engine->isOpenglInitialized = afalse;
	engine->exitApplication = afalse;
	engine->colorBits = colorBits;
	engine->buildType = buildType;
	engine->renderState = ASGARD_ENGINE_RENDER_STATE_INIT;
	engine->name = strdup(name);

	// mouse

	engine->mouse.clickCount = 0;
	engine->mouse.buttonType = 0;
	engine->mouse.x = 0;
	engine->mouse.y = 0;
	engine->mouse.lastX = 0;
	engine->mouse.lastY = 0;
	engine->mouse.eventType = EVENT_MOUSE_END_OF_LIST;
	engine->mouse.lastUpdatedTime = time_getTimeMus();

	mutex_init(&engine->mouse.mutex);

	// keyboard

	engine->keyboard.shiftKey = afalse;
	engine->keyboard.controlKey = afalse;
	engine->keyboard.key = (char)0;
	engine->keyboard.lastKey = (char)0;
	engine->keyboard.keyCode = 0;
	engine->keyboard.lastKeyCode = 0;
	engine->keyboard.eventType = EVENT_KEYBOARD_END_OF_LIST;
	engine->keyboard.bufferRef = 0;
	engine->keyboard.lastUpdatedTime = time_getTimeMus();

	memset(engine->keyboard.keyState, 0,
			(sizeof(int) * ASGARD_ENGINE_KEY_STATE_LENGTH));
	memset(engine->keyboard.buffer, 0,
			(sizeof(char) * ASGARD_ENGINE_KEY_BUFFER_LENGTH));
	mutex_init(&engine->keyboard.mutex);

	// screen

	engine->screen.requiresResize = afalse;
	engine->screen.screenX = 0;
	engine->screen.screenY = 0;
	engine->screen.screenWidth = screenWidth;
	engine->screen.screenHeight = screenHeight;
	engine->screen.lastUpdatedTime = time_getTimeMus();

	mutex_init(&engine->screen.mutex);

	// log

	if(engine->buildType == ASGARD_ENGINE_DEVELOPMENT_BUILD) {
		log_init(&engine->log, LOG_OUTPUT_FILE, "log/asgard.game.engine.log",
				LOG_LEVEL_DEBUG);
	}
	else {
		log_init(&engine->log, LOG_OUTPUT_FILE, "log/asgard.game.engine.log",
				LOG_LEVEL_ERROR);
	}

	// mutex

	mutex_init(&engine->mutex);
	mutex_lock(&engine->mutex);

	// game context

	engine->gameContext = gameContext;

	// obtain OS window context and window-handle

	if(createWindow) {
		if(osapi_createWindow(
					engine->name,
					isFullscreen,
					engine->screen.screenWidth,
					engine->screen.screenHeight,
					engine->colorBits,
					&engine->log,
					&engine->window
					) < 0) {
			log_logf(&engine->log, LOG_LEVEL_ERROR,
					"{%s}[%s()] failed to create window",
					engine->name, __FUNCTION__);
			mutex_unlock(&engine->mutex);
			return -1;
		}
	}
	else {
		if(osapi_initWithinWindow(
					handle,
					engine->name,
					isFullscreen,
					engine->screen.screenWidth,
					engine->screen.screenHeight,
					engine->colorBits,
					&engine->log,
					&engine->window
					) < 0) {
			log_logf(&engine->log, LOG_LEVEL_ERROR,
					"{%s}[%s()] failed to init within window",
					engine->name, __FUNCTION__);
			mutex_unlock(&engine->mutex);
			return -1;
		}
	}

	// enable OpenGL

	engine->windowHandle = osapi_enableOpenGL(atrue, engine->colorBits,
			engine->window);

	// setup and spawn threads

	engine->ioThread.id = 0;
	engine->renderThread.id = 0;
	engine->physicsThread.id = 0;
	engine->gameplayThread.id = 0;

	tmf_init(&engine->tmf);

	// i/o thread

	if(ioFunction != NULL) {
		engine->ioThread.function = ioFunction;
	}
	else {
		engine->ioThread.function = NULL;
	}

	if(tmf_spawnThread(&engine->tmf, engineIoThread, (void *)engine,
				&engine->ioThread.id) < 0) {
		log_logf(&engine->log, LOG_LEVEL_ERROR,
				"{%s}[%s()] failed to spawn i/o thread",
				engine->name, __FUNCTION__);
		mutex_unlock(&engine->mutex);
		return -1;
	}

	// render thread

	if(renderFunction != NULL) {
		engine->renderThread.function = renderFunction;
	}
	else {
		engine->renderThread.function = defaultRenderFunction;
	}

	if(tmf_spawnThread(&engine->tmf, engineRenderThread, (void *)engine,
				&engine->renderThread.id) < 0) {
		log_logf(&engine->log, LOG_LEVEL_ERROR,
				"{%s}[%s()] failed to spawn render thread",
				engine->name, __FUNCTION__);
		mutex_unlock(&engine->mutex);
		return -1;
	}

	// physics thread

	if(physicsFunction != NULL) {
		engine->physicsThread.function = physicsFunction;
	}
	else {
		engine->physicsThread.function = defaultPhysicsFunction;
	}

	if(tmf_spawnThread(&engine->tmf, enginePhysicsThread, (void *)engine,
				&engine->physicsThread.id) < 0) {
		log_logf(&engine->log, LOG_LEVEL_ERROR,
				"{%s}[%s()] failed to spawn physics thread",
				engine->name, __FUNCTION__);
		mutex_unlock(&engine->mutex);
		return -1;
	}

	// gameplay thread

	if(gameplayFunction != NULL) {
		engine->gameplayThread.function = gameplayFunction;
	}
	else {
		engine->gameplayThread.function = defaultGameplayFunction;
	}

	if(tmf_spawnThread(&engine->tmf, engineGameplayThread, (void *)engine,
				&engine->gameplayThread.id) < 0) {
		log_logf(&engine->log, LOG_LEVEL_ERROR,
				"{%s}[%s()] failed to spawn gameplay thread",
				engine->name, __FUNCTION__);
		mutex_unlock(&engine->mutex);
		return -1;
	}

	// engine initialization complete

	engine->isInitialized = atrue;

	log_logf(&engine->log, LOG_LEVEL_INFO,
			"{%s}[%s()] initialization complete",
			engine->name, __FUNCTION__);

	mutex_unlock(&engine->mutex);

	return 0;
}

int engine_free(Engine *engine, aboolean destroyWindow)
{
	int pfsReportLength = 0;
	char *pfsReport = NULL;

	if(engine == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&engine->mutex);

	// stop threads

	// i/o thread

	if(tmf_stopThread(&engine->tmf, engine->ioThread.id) < 0) {
		log_logf(&engine->log, LOG_LEVEL_ERROR,
				"{%s}[%s()] failed to stop i/o thread",
				engine->name, __FUNCTION__);
		exit(1);
	}

	// render thread

	if(tmf_stopThread(&engine->tmf, engine->renderThread.id) < 0) {
		log_logf(&engine->log, LOG_LEVEL_ERROR,
				"{%s}[%s()] failed to stop render thread",
				engine->name, __FUNCTION__);
		exit(1);
	}

	// physics thread

	if(tmf_stopThread(&engine->tmf, engine->physicsThread.id) < 0) {
		log_logf(&engine->log, LOG_LEVEL_ERROR,
				"{%s}[%s()] failed to stop physics thread",
				engine->name, __FUNCTION__);
		exit(1);
	}

	// gameplay thread

	if(tmf_stopThread(&engine->tmf, engine->gameplayThread.id) < 0) {
		log_logf(&engine->log, LOG_LEVEL_ERROR,
				"{%s}[%s()] failed to stop gameplay thread",
				engine->name, __FUNCTION__);
		exit(1);
	}

	// free thread management framework

	if(tmf_free(&engine->tmf) < 0) {
		log_logf(&engine->log, LOG_LEVEL_ERROR,
				"{%s}[%s()] failed to free thread management framework",
				engine->name, __FUNCTION__);
		exit(1);
	}

	// free canvas

	canvas_free(&engine->canvas);

	// construct and log function profiling report

	pfs_buildReport(asgard_getPfs(), &pfsReport, &pfsReportLength);

	log_logf(&engine->log, LOG_LEVEL_INFO, "%s", pfsReport);

	free(pfsReport);

	// disable OpenGL

	osapi_disableOpenGL(engine->window);

	// destroy window

	if(destroyWindow) {
		if(osapi_destroyWindow(engine->window) < 0) {
			log_logf(&engine->log, LOG_LEVEL_PANIC,
					"{%s}[%s()] failed to destroy window",
					engine->name, __FUNCTION__);
		}
	}

	// free remaining engine context

	log_logf(&engine->log, LOG_LEVEL_INFO,
			"{%s}[%s()] essentials free complete",
			engine->name, __FUNCTION__);

	if(engine->name != NULL) {
		free(engine->name);
	}

	mutex_free(&engine->mouse.mutex);
	mutex_free(&engine->keyboard.mutex);
	mutex_free(&engine->screen.mutex);

	font_free(&engine->font);

	log_free(&engine->log);

	// finish cleanup

	mutex_unlock(&engine->mutex);
	mutex_free(&engine->mutex);

	memset(engine, 0, sizeof(Engine));

	return 0;
}

