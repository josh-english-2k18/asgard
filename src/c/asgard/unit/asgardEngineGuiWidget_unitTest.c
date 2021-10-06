/*
 * asgardEngineGuiWidget.c
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


// define Asgard game engine gui widget unit test constants

#define COLOR_BITS										32

#define USE_FULLSCREEN_MODE								afalse

#define SCREEN_WIDTH									1024

#define SCREEN_HEIGHT									768


// define Asgard game engine gui widget unit test data types

typedef struct _UnitTestState {
	aboolean isLoaded;
	aboolean areTexturesLoaded;
	GuiButton button01;
	GuiButton button02;
	GuiButton button03;
	GuiCheckBox checkbox01;
	GuiTextbox textbox01;
/*
 *  Autoscroll GUI widget is not working, commented out for now
 *
	GuiAutoscroll autoscroll01;
 */
	Font font;
	Log log;
	Engine engine;
} UnitTestState;


// declare Asgard game engine gui widget unit test functions

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
	state.areTexturesLoaded = afalse;

	log_init(&state.log, LOG_OUTPUT_FILE,
			"log/asgard.engine.gui.widget.unit.test.log",
			LOG_LEVEL_DEBUG);

	// initialize game engine

	snprintf(buffer, sizeof(buffer),
			"Asgard Game Engine GUI Widget Unit Test %ix%i",
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

		time_usleep(32);
	}

	// cleanup

	engine_free(&state.engine, atrue);

	log_free(&state.log);

	return rc;
}


// define Asgard game engine gui widget unit test functions

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
					"[%s->%s()] keyboard event #%i",
					engine->name, __FUNCTION__, keyboardEvent->eventType);

			guiButton_keyPressed(&state->button01, keyboardEvent);
			guiButton_keyPressed(&state->button02, keyboardEvent);
			guiButton_keyPressed(&state->button03, keyboardEvent);

			guiCheckBox_keyPressed(&state->checkbox01, keyboardEvent);

			guiTextbox_keyPressed(&state->textbox01, keyboardEvent);

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
				if(guiButton_wasClicked(&state->button01,
							mouseEvent->xPosition,
							mouseEvent->yPosition)) {
					guiButton_deHighlight(&state->button02);
					guiButton_deHighlight(&state->button03);
				}
				else if(guiButton_wasClicked(&state->button02,
							mouseEvent->xPosition,
							mouseEvent->yPosition)) {
					guiButton_deHighlight(&state->button01);
					guiButton_deHighlight(&state->button03);
				}
				else if(guiButton_wasClicked(&state->button03,
							mouseEvent->xPosition,
							mouseEvent->yPosition)) {
					guiButton_deHighlight(&state->button01);
					guiButton_deHighlight(&state->button02);
				}
				else if(guiCheckBox_wasClicked(&state->checkbox01,
							mouseEvent->xPosition,
							mouseEvent->yPosition)) {
					// Check Box was clicked
				}
				else if(guiTextbox_wasClicked(&state->textbox01,
							mouseEvent->xPosition,
							mouseEvent->yPosition)) {
					// Text Box was clicked
				}
			}
			else {
				guiButton_hasFocus(&state->button01, mouseEvent->xPosition,
						mouseEvent->yPosition);
				guiButton_hasFocus(&state->button02, mouseEvent->xPosition,
						mouseEvent->yPosition);
				guiButton_hasFocus(&state->button03, mouseEvent->xPosition,
						mouseEvent->yPosition);
				guiCheckBox_hasFocus(&state->checkbox01, mouseEvent->xPosition,
						mouseEvent->yPosition);
				guiTextbox_hasFocus(&state->textbox01, mouseEvent->xPosition,
						mouseEvent->yPosition);
			}
			break;
	}

	return 0;
}

static int localRenderFunction(void *engineContext, void *gameContext,
		void *argument)
{
	char *message = NULL;

	Color color;

	Texture *texture = NULL;
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
				"Loading gui widgets");
		canvas_endRender(&engine->canvas);
		return 0;
	}

	if(!state->areTexturesLoaded) {
		texture = texture_load(&state->log,
				"assets/textures/gui/button/default/normal.config");
		guiButton_setTexture(&state->button01, GUI_BUTTON_STATE_NORMAL, texture);
		guiButton_setTexture(&state->button02, GUI_BUTTON_STATE_NORMAL, texture);
		guiButton_setTexture(&state->button03, GUI_BUTTON_STATE_NORMAL, texture);

		texture = texture_load(&state->log,
				"assets/textures/gui/button/default/focused.config");
		guiButton_setTexture(&state->button01, GUI_BUTTON_STATE_FOCUSED,
				texture);
		guiButton_setTexture(&state->button02, GUI_BUTTON_STATE_FOCUSED,
				texture);
		guiButton_setTexture(&state->button03, GUI_BUTTON_STATE_FOCUSED,
				texture);

		texture = texture_load(&state->log,
				"assets/textures/gui/button/default/highlighted.config");
		guiButton_setTexture(&state->button01, GUI_BUTTON_STATE_HIGHLIGHTED,
				texture);
		guiButton_setTexture(&state->button02, GUI_BUTTON_STATE_HIGHLIGHTED,
				texture);
		guiButton_setTexture(&state->button03, GUI_BUTTON_STATE_HIGHLIGHTED,
				texture);

		texture = texture_load(&state->log,
				"assets/textures/gui/button/default/down.config");
		guiButton_setTexture(&state->button01, GUI_BUTTON_STATE_DOWN, texture);
		guiButton_setTexture(&state->button02, GUI_BUTTON_STATE_DOWN, texture);
		guiButton_setTexture(&state->button03, GUI_BUTTON_STATE_DOWN, texture);

		texture = texture_load(&state->log,
				"assets/textures/gui/checkbox/default/box.config");
		guiCheckBox_setBoxImage(&state->checkbox01, texture);

		texture = texture_load(&state->log,
				"assets/textures/gui/checkbox/default/mark.config");
		guiCheckBox_setCheckImage(&state->checkbox01, texture);

		state->areTexturesLoaded = atrue;
	}

	// perform gui widget rendering

	// begin render

	canvas_beginRender(&engine->canvas);

	// 3d objects

	canvas_toggleRenderMode3d(&engine->canvas);

	// 2d objects

	canvas_toggleRenderMode2d(&engine->canvas);

	CGI_COLOR_WHITE;
	font_draw2d(&engine->font, &engine->canvas, 4, 0, "Framerate: %0.2f",
			engine->canvas.framerate);

	// render gui widgets

	guiButton_render2d(&state->button01);

	switch(guiButton_getState(&state->button01)) {
		case GUI_BUTTON_STATE_NORMAL:
			message = "Button01 state: normal";
			break;
		case GUI_BUTTON_STATE_FOCUSED:
			message = "Button01 state: focused";
			break;
		case GUI_BUTTON_STATE_HIGHLIGHTED:
			message = "Button01 state: highlighted";
			break;
		case GUI_BUTTON_STATE_DOWN:
			message = "Button01 state: down";
			break;
		default:
			message = "Button01 state: unknown";
	}

	CGI_COLOR_WHITE;
	font_draw2d(&engine->font, &engine->canvas, 4, 64, message);

	guiButton_render2d(&state->button02);

	switch(guiButton_getState(&state->button02)) {
		case GUI_BUTTON_STATE_NORMAL:
			message = "Button02 state: normal";
			break;
		case GUI_BUTTON_STATE_FOCUSED:
			message = "Button02 state: focused";
			break;
		case GUI_BUTTON_STATE_HIGHLIGHTED:
			message = "Button02 state: highlighted";
			break;
		case GUI_BUTTON_STATE_DOWN:
			message = "Button02 state: down";
			break;
		default:
			message = "Button02 state: unknown";
	}

	CGI_COLOR_WHITE;
	font_draw2d(&engine->font, &engine->canvas, 4, 84, message);

	guiButton_render2d(&state->button03);

	switch(guiButton_getState(&state->button03)) {
		case GUI_BUTTON_STATE_NORMAL:
			message = "Button03 state: normal";
			break;
		case GUI_BUTTON_STATE_FOCUSED:
			message = "Button03 state: focused";
			break;
		case GUI_BUTTON_STATE_HIGHLIGHTED:
			message = "Button03 state: highlighted";
			break;
		case GUI_BUTTON_STATE_DOWN:
			message = "Button03 state: down";
			break;
		default:
			message = "Button03 state: unknown";
	}

	CGI_COLOR_WHITE;
	font_draw2d(&engine->font, &engine->canvas, 4, 104, message);

	guiCheckBox_render2d(&state->checkbox01);

	if(guiCheckBox_isChecked(&state->checkbox01)) {
		message = "CheckBox01 state: checked";
	}
	else {
		message = "CheckBox01 state: unchecked";
	}

	CGI_COLOR_WHITE;
	font_draw2d(&engine->font, &engine->canvas, 640, 64, message);

	guiTextbox_render2d(&state->textbox01);

	if(guiTextbox_isFocused(&state->textbox01)) {
		message = "TextBox01 state: focused";
	}
	else {
		message = "TextBox01 state: normal";
	}

	CGI_COLOR_WHITE;
	font_draw2d(&engine->font, &engine->canvas, 2, 340, message);
/*
 *  Autoscroll GUI widget is not working, commented out for now
 *
	guiAutoscroll_render2d(&state->autoscroll01);

	if(guiAutoscroll_isRunning(&state->autoscroll01)) {
		message = "Autoscroll01 state: running";
	}
	else {
		message = "Autoscroll01 state: not running";
	}

	CGI_COLOR_WHITE;
	font_draw2d(&engine->font, &engine->canvas, 640, 510, message);
*/
	// draw a rectangle at the current mouse position
	
	color.r = CGI_COLOR_BLUE_RED;
	color.g = CGI_COLOR_BLUE_GREEN;
	color.b = CGI_COLOR_BLUE_BLUE;
	color.a = CGI_COLOR_BLUE_ALPHA;

	draw2d_rectangle(&engine->canvas, &color,
			state->engine.mouse.x, state->engine.mouse.y, 24, 24);

	// end render

	canvas_endRender(&engine->canvas);

	return 0;
}

static int localGameplayFunction(void *engineContext, void *gameContext,
		void *argument)
{
	Engine *engine = NULL;
	UnitTestState *state = NULL;

	Color color;
	Texture *texture = NULL;

	engine = (Engine *)engineContext;
	state = (UnitTestState *)gameContext;

	// execute initialization

	if(!state->isLoaded) {
		if(font_init(&state->font,
					state->engine.windowHandle,
					FONT_LUCIDA,
					14,
					atrue,
					afalse,
					afalse,
					FONT_CHARSET_ANSI) < 0) {
			log_logf(&state->log, LOG_LEVEL_ERROR,
					"unit test %s() failed at %i, aborting",
					__FUNCTION__, __LINE__);
			return -1;
		}

		guiButton_init(&state->button01, "UnitTestButton01", 4, 32, 64, 32,
				engine);

		guiButton_setText(&state->button01, &engine->font, "B01");

		guiButton_init(&state->button02, "UnitTestButton02", 68, 32, 64, 32,
				engine);

		guiButton_setText(&state->button02, &engine->font, "B02");

		guiButton_init(&state->button03, "UnitTestButton03", 132, 32, 64, 32,
				engine);

		guiButton_setText(&state->button03, &engine->font, "B03");

		guiCheckBox_init(&state->checkbox01, "UnitTestCheckBox01", 640, 32, 32, 
				32, engine);

		guiTextbox_init(&state->textbox01, "UnitTestTextBox01", 2, 306, 256, 32,
				engine);

		guiTextbox_setFont(&state->textbox01, &engine->font);

/*
 *  Autoscroll GUI widget is not working, commented out for now
 *
		guiAutoscroll_init(&state->autoscroll01, "UnitTestAutoscroll01", 640,
				512, 256, 256, engine);

		color.r = CGI_COLOR_BLACK_RED;
		color.g = CGI_COLOR_BLACK_GREEN;
		color.b = CGI_COLOR_BLACK_BLUE;
		color.a = CGI_COLOR_BLACK_ALPHA;

		guiAutoscroll_setBackgroundColor(&state->autoscroll01, &color);

		color.r = CGI_COLOR_WHITE_RED;
		color.g = CGI_COLOR_WHITE_GREEN;
		color.b = CGI_COLOR_WHITE_BLUE;
		color.a = CGI_COLOR_WHITE_ALPHA;

		guiAutoscroll_setOutlineColor(&state->autoscroll01, &color);

		guiAutoscroll_addColorText(&state->autoscroll01, &color, "");
		guiAutoscroll_addColorText(&state->autoscroll01, &color, "");
		guiAutoscroll_addColorText(&state->autoscroll01, &color, "");
		guiAutoscroll_addColorText(&state->autoscroll01, &color, "");
		guiAutoscroll_addColorText(&state->autoscroll01, &color, 
				"Test Line #01");
		guiAutoscroll_addColorText(&state->autoscroll01, &color, "");
		
		color.r = CGI_COLOR_BLUE_RED;
		color.g = CGI_COLOR_BLUE_GREEN;
		color.b = CGI_COLOR_BLUE_BLUE;
		color.a = CGI_COLOR_BLUE_ALPHA;

		guiAutoscroll_addColorText(&state->autoscroll01, &color,
			"Test Line #02");

		color.r = CGI_COLOR_GREEN_RED;
		color.g = CGI_COLOR_GREEN_GREEN;
		color.b = CGI_COLOR_GREEN_BLUE;
		color.a = CGI_COLOR_GREEN_ALPHA;

		guiAutoscroll_addColorText(&state->autoscroll01, &color,
			"Test Line #03");

		guiAutoscroll_addText(&state->autoscroll01, "Test Line #04");

		guiAutoscroll_addHyperlink(&state->autoscroll01, "Google");
		
		texture = texture_load(&state->log,
				"assets/textures/gui/checkbox/default/box.config");
		guiAutoscroll_addImage(&state->autoscroll01, texture, 32, 32);

		color.r = CGI_COLOR_WHITE_RED;
		color.g = CGI_COLOR_WHITE_GREEN;
		color.b = CGI_COLOR_WHITE_BLUE;
		color.a = CGI_COLOR_WHITE_ALPHA;
	
		guiAutoscroll_addColorText(&state->autoscroll01, &color,
			"Final Test Line");
*/	
		state->isLoaded = atrue;
	}

	return 0;
}

