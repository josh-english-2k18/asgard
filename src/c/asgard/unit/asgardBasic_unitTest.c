/*
 * asgardBasic_unitTest.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard Game Engine basic-functionality unit test application.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#include "asgard/asgard.h"

// define Asgard basic unit test constants

#define COLOR_BITS										32

#define USE_FULLSCREEN_MODE								afalse

#define SCREEN_WIDTH									1024

#define SCREEN_HEIGHT									768

#define KEYBOARD_BUFFER_LENGTH							128

#define UT_LOAD_TARGA_FILENAME							\
	"assets/data/test/asgard/targaTest01.tga"

#define UT_FONT_SIZE									14

#define UT_TEXTURE_QUALITY								TEXTURE_QUALITY_HIGHEST
#define UT_TEXTURE_RENDER_MODE							TEXTURE_RENDER_REPEAT
#define UT_TEXTURE_HARDWARE_RESIDENT					atrue
#define UT_TEXTURE_FILENAME								\
	"assets/data/test/asgard/targaTest01.tga"
#define UT_TEXTURE_RGBA_ALTERATION						-24

// define Asgard basic unit test data types

typedef struct _UnitTestListEntry {
	char *testName;
	int (*function)();
} UnitTestListEntry;

typedef struct _UnitTestState {
	aboolean isActive;
	aboolean exitApplication;
	int mouseX;
	int mouseY;
	int keyboardBufferRef;
	char keyboardBuffer[KEYBOARD_BUFFER_LENGTH];
	void *window;
	Font osFont;
	Font lFont;
	Font aFont;
	Texture texture;
	Texture mTexture;
	Texture aTexture;
	Texture rTexture;
	Texture cTexture;
	Texture bTexture;
	Canvas canvas;
	Log log;
} UnitTestState;

// declare Asgard basic unit test functions

static void displayFatalError(char *error);

static void checkMessageQueue();

// declare Asgard basic unit test - test functions

static int unitTest_loadTarga();

static int unitTest_loadFont();

static int unitTest_freeFont();

static int unitTest_loadTexture();

static int unitTest_freeTexture();

// define Asgard basic unit test global variables

UnitTestListEntry UnitTestList[] = {
	{	"Load Targa File",			unitTest_loadTarga			},
	{	"Load 2D Font",				unitTest_loadFont			},
	{	"Load 2D Texture",			unitTest_loadTexture		},
	{	NULL,						NULL						}
};

UnitTestListEntry UnitTestShutdownList[] = {
	{	"Free 2D Font",				unitTest_freeFont			},
	{	"Free 2D Texture",			unitTest_freeTexture		},
	{	NULL,						NULL						}
};

UnitTestState state;

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
	int ii = 0;
	int rc = 0;
	int testRef = 0;
	int textureQuality = UT_TEXTURE_QUALITY;
	int blendSource = TEXTURE_BLEND_SOURCE_SRC_ALPHA;
	int blendDest = TEXTURE_BLEND_DEST_ONE_MINUS_SRC_ALPHA;
	int pfsReportLength = 0;
	double timing = 0.0;
	double timestamp = 0.0;
	char *pfsReport = NULL;
	char *stdBuffer = NULL;
	char buffer[1024];

	Color adjustColor;

	signal_registerDefault();

	// initialize unit test state

	memset((void *)&state, 0, sizeof(UnitTestState));

	state.isActive = atrue;
	state.exitApplication = afalse;
	state.mouseX = 0;
	state.mouseY = 0;
	state.keyboardBufferRef = 0;
	memset((void *)state.keyboardBuffer, 0, KEYBOARD_BUFFER_LENGTH);
	state.window = NULL;

	log_init(&state.log, LOG_OUTPUT_FILE, "log/asgard.basic.unit.test.log",
			LOG_LEVEL_DEBUG);

	// create unit test window

	snprintf(buffer, sizeof(buffer), "Asgard Basic Unit Test %ix%i",
			SCREEN_WIDTH, SCREEN_HEIGHT);

	if(osapi_createWindow(
				buffer,
				USE_FULLSCREEN_MODE,
				SCREEN_WIDTH,
				SCREEN_HEIGHT,
				COLOR_BITS,
				&state.log,
				&state.window) < 0) {
		displayFatalError("failed to create window");
		return -1;
	}

	if(osapi_enableOpenGL(
				afalse,
				COLOR_BITS,
				state.window) == NULL) {
		displayFatalError("failed to initialize OpenGL");
		return -1;
	}

	// initialize unit test canvas

	if(canvas_init(
				&state.canvas,
				buffer,
				USE_FULLSCREEN_MODE,
				SCREEN_WIDTH,
				SCREEN_HEIGHT,
				osapi_getWindowHandle(state.window),
				&state.log) < 0) {
		log_logf(&state.log, LOG_LEVEL_ERROR, "failed to initialize canvas");
		displayFatalError("failed to initialize canvas");
		return -1;
	}

	// setup color-masked texture adjustment

	adjustColor.r = 1;
	adjustColor.g = -1;
	adjustColor.b = 1;

	// simple single-threaded application loop

	while(!state.exitApplication) {
		timestamp = time_getTimeMus();

		// check on window state

		if((rc = osapi_checkState(state.window, &state.exitApplication)) != 0) {
			break;
		}

		// render scene

		canvas_beginRender(&state.canvas);
		canvas_toggleRenderMode3d(&state.canvas);

		if(UnitTestList[testRef].testName == NULL) {
			canvas_toggleRenderMode2d(&state.canvas);

			// texture draw2d test

			// regular texture

			texture_draw2d(&state.texture, 0, 96, 256, 256);

			// color-masked texture (modulate)

			texture_draw2d(&state.mTexture, 288, 96, 256, 256);

			// color-masked texture (add)

			texture_draw2d(&state.aTexture, 576, 96, 256, 256);

			// font draw2d test

			CGI_COLOR_WHITE;
			font_draw2d(&state.lFont, &state.canvas, 4, 0, "Framerate: %0.2f",
					state.canvas.framerate);

			// color test

			CGI_COLOR_RED;
			font_draw2d(&state.lFont, &state.canvas, 4, 14, "Red-Color");
			CGI_COLOR_BLUE;
			font_draw2d(&state.lFont, &state.canvas, 4, 28, "Blue-Color");
			CGI_COLOR_GREEN;
			font_draw2d(&state.lFont, &state.canvas, 4, 42, "Green-Color");

			// other fonts test

			CGI_COLOR_MAGENTA;
			font_draw2d(&state.osFont, &state.canvas, 4, 56,
					"The quick brown fox jumped over the lazy dogs "
					"(default OS font).");
			CGI_COLOR_GOLD;
			font_draw2d(&state.aFont, &state.canvas, 4, 70,
					"The quick brown fox jumped over the lazy dogs "
					"(Arial font).");

			// draw font over texture test

			CGI_COLOR_BLACK;
			font_draw2d(&state.lFont, &state.canvas, 4, 96,
					"1234567890123456789012345678901");
			CGI_COLOR_AQUA;
			font_draw2d(&state.lFont, &state.canvas, 4, 110,
					"[texture quality: %i]", textureQuality);
			CGI_COLOR_LINEN;
			font_draw2d(&state.lFont, &state.canvas, 292, 96,
					"[mask (%0.2f, %0.2f, %0.2f)]",
					state.mTexture.colorMask.r,
					state.mTexture.colorMask.g,
					state.mTexture.colorMask.b);
			CGI_COLOR_BLACK;
			font_draw2d(&state.lFont, &state.canvas, 580, 96,
					"[mask (%0.2f, %0.2f, %0.2f)]",
					state.aTexture.colorMask.r,
					state.aTexture.colorMask.g,
					state.aTexture.colorMask.b);

			// mouse & keyboard states

			CGI_COLOR_WHITE;
			font_draw2d(&state.lFont, &state.canvas, 4, 352, "Mouse: (%i, %i)",
					state.mouseX, state.mouseY);
			font_draw2d(&state.lFont, &state.canvas, 4, 366, "Keyboard: '%s'",
					state.keyboardBuffer);

			// RGBA altered texture

			texture_draw2d(&state.rTexture, 0, 390, 256, 256);

			// texture from an RGBA color

			texture_draw2d(&state.cTexture, 288, 390, 256, 256);

			// blended texture

			for(ii = 0; ii < 32; ii++) {
				texture_draw2d(&state.bTexture, (576 + ii), (390 + ii),
						256, 256);
			}

			// draw font over texture test

			CGI_COLOR_BLACK;
			font_draw2d(&state.lFont, &state.canvas, 4, 390,
					"[RGBA (%i, %i, %i)]",
					UT_TEXTURE_RGBA_ALTERATION,
					UT_TEXTURE_RGBA_ALTERATION,
					UT_TEXTURE_RGBA_ALTERATION);
			font_draw2d(&state.lFont, &state.canvas, 292, 390,
					"[from-color: orchid]");
			font_draw2d(&state.lFont, &state.canvas, 580, 390,
					"[blended texture: %i, %i]",
					blendSource,
					blendDest);

			// draw a backslash at the current mouse position

			CGI_COLOR_BLUE;
			font_draw2d(&state.lFont, &state.canvas, state.mouseX, state.mouseY,
					"\\");
		}

		canvas_endRender(&state.canvas);

		// execute all unit tests, one at a time

		if(UnitTestList[testRef].testName != NULL) {
			log_logf(&state.log, LOG_LEVEL_INFO,
					"executing unit test: %s", UnitTestList[testRef].testName);

			UnitTestList[testRef].function();

			testRef++;
		}
		else {
			// change quality of primary texture

			if((state.canvas.renderTicks % 120) == 0) {
				textureQuality++;
				if(textureQuality > TEXTURE_QUALITY_HIGHEST) {
					textureQuality = TEXTURE_QUALITY_LOWEST;
				}
				texture_setQualityLevel(&state.texture, textureQuality);
			}

			// adjust color-masked texture mask

			if(adjustColor.r == 1) {
				state.mTexture.colorMask.r += 0.001;
				state.aTexture.colorMask.r += 0.001;
				if(state.mTexture.colorMask.r >= 1.0) {
					adjustColor.r = -1;
					state.mTexture.colorMask.r = 1.0;
					state.aTexture.colorMask.r = 1.0;
				}
			}
			else {
				state.mTexture.colorMask.r -= 0.001;
				state.aTexture.colorMask.r -= 0.001;
				if(state.mTexture.colorMask.r <= 0.001) {
					adjustColor.r = 1;
					state.mTexture.colorMask.r = 0.001;
					state.aTexture.colorMask.r = 0.001;
				}
			}

			if(adjustColor.g == 1) {
				state.mTexture.colorMask.g += 0.001;
				state.aTexture.colorMask.g += 0.001;
				if(state.mTexture.colorMask.g >= 1.0) {
					adjustColor.g = -1;
					state.mTexture.colorMask.g = 1.0;
					state.aTexture.colorMask.g = 1.0;
				}
			}
			else {
				state.mTexture.colorMask.g -= 0.001;
				state.aTexture.colorMask.g -= 0.001;
				if(state.mTexture.colorMask.g <= 0.001) {
					adjustColor.g = 1;
					state.mTexture.colorMask.g = 0.001;
					state.aTexture.colorMask.g = 0.001;
				}
			}

			if(adjustColor.b == 1) {
				state.mTexture.colorMask.b += 0.001;
				state.aTexture.colorMask.b += 0.001;
				if(state.mTexture.colorMask.b >= 1.0) {
					adjustColor.b = -1;
					state.mTexture.colorMask.b = 1.0;
					state.aTexture.colorMask.b = 1.0;
				}
			}
			else {
				state.mTexture.colorMask.b -= 0.001;
				state.aTexture.colorMask.b -= 0.001;
				if(state.mTexture.colorMask.b <= 0.001) {
					adjustColor.b = 1;
					state.mTexture.colorMask.b = 0.001;
					state.aTexture.colorMask.b = 0.001;
				}
			}

			// alter blended-texture blend function

			if((state.canvas.renderTicks % 60) == 0) {
				blendDest++;
				if(blendDest > TEXTURE_BLEND_DEST_ONE_MINUS_DST_ALPHA) {
					blendDest = TEXTURE_BLEND_DEST_SRC_COLOR;

					blendSource++;
					if(blendSource > TEXTURE_BLEND_SOURCE_ONE_MINUS_DST_ALPHA) {
						blendSource = TEXTURE_BLEND_SOURCE_SRC_COLOR;
					}
				}

				texture_setBlend(&state.bTexture, blendSource, blendDest);
			}
		}

		// check on OS message queue, such as user input

		checkMessageQueue();

		timing = time_getElapsedMusInSeconds(timestamp);
		if(timing < 0.016666667) {
			time_usleep((int)((0.016666667 - timing) * REAL_ONE_MILLION) -
					1024);
		}

		// purge stderr & stdout streams

		while((stdBuffer = common_popStdStreamError()) != NULL) {
			log_logf(&state.log, LOG_LEVEL_ERROR, "STDERR %s", stdBuffer);
		}

		while((stdBuffer = common_popStdStreamOutput()) != NULL) {
			log_logf(&state.log, LOG_LEVEL_INFO, "STDOUT %s", stdBuffer);
		}
	}

	// execute all unit shutdown tests, one at a time

	for(ii = 0; UnitTestShutdownList[ii].testName != NULL; ii++) {
		log_logf(&state.log, LOG_LEVEL_INFO,
				"executing unit test: %s", UnitTestShutdownList[ii].testName);

		UnitTestShutdownList[ii].function();
	}

	// free unit test canvas

	canvas_free(&state.canvas);

	// construct and log function profiling report

	pfs_buildReport(asgard_getPfs(), &pfsReport, &pfsReportLength);

	log_logf(&state.log, LOG_LEVEL_INFO, "%s", pfsReport);

	free(pfsReport);

	// destroy unit test window

	if(osapi_destroyWindow(state.window) < 0) {
		log_logf(&state.log, LOG_LEVEL_PANIC, "failed to destroy window");
	}

	// cleanup unit test state

	log_free(&state.log);

	return rc;
}


// define Asgard basic unit test functions

static void displayFatalError(char *error)
{
#if defined(WIN32)
	MessageBox(NULL,
			error,
			"Asgard Game Engine - Fatal Error",
			MB_OK | MB_ICONSTOP
			);
#elif defined(__linux__)
	log_logf(&state.log, LOG_LEVEL_PANIC,
			"the Asgard game engine encountered a fatal error: %s",
			error);
#else // - no plugins available -
#	warning "no windowing plugins available"
#endif // - plugins -
}

static void checkMessageQueue()
{
	AsgardEvent *event = NULL;
	AsgardOsEvent *osEvent = NULL;
	AsgardKeyboardEvent *keyboardEvent = NULL;
	AsgardMouseEvent *mouseEvent = NULL;

	while((event = osapi_popEvent(state.window)) != NULL) {
		switch(event->eventType) {
			/*
			 * handle operating system events
			 */

			case EVENT_TYPE_OS:
				osEvent = (AsgardOsEvent *)event->event;

				log_logf(&state.log, LOG_LEVEL_DEBUG,
						"message queue: os event #%i", osEvent->eventType);

				switch(osEvent->eventType) {
					case EVENT_OS_SCREEN_RESIZED:
						canvas_setScreenDimensions(&state.canvas,
								osEvent->screenWidth,
								osEvent->screenHeight);
						log_logf(&state.log, LOG_LEVEL_INFO,
								"reset canvas to %ix%i dimensions",
								osEvent->screenWidth,
								osEvent->screenHeight);
						break;
				}
				break;

			/*
			 * handle keyboard events
			 */

			case EVENT_TYPE_KEYBOARD:
				keyboardEvent = (AsgardKeyboardEvent *)event->event;

				log_logf(&state.log, LOG_LEVEL_DEBUG,
						"message queue: keyboard event #%i",
						keyboardEvent->eventType);

				switch(keyboardEvent->eventType) {
					case EVENT_KEYBOARD_PRESSED:
						switch(keyboardEvent->keyCode) {
							case ASGARD_KEY_ESCAPE:
								state.exitApplication = atrue;
								break;
							case ASGARD_KEY_RETURN:
								state.keyboardBufferRef = 0;
								memset((void *)state.keyboardBuffer, 0,
										KEYBOARD_BUFFER_LENGTH);
								break;
						}
						break;
					case EVENT_KEYBOARD_TYPED:
						if(((unsigned int)keyboardEvent->key >= 32) &&
								((unsigned int)keyboardEvent->key <= 126) &&
								(state.keyboardBufferRef <
								 (KEYBOARD_BUFFER_LENGTH - 1))
								) {
							state.keyboardBuffer[state.keyboardBufferRef] =
								keyboardEvent->key;
							state.keyboardBufferRef += 1;
						}
						break;
				}
				break;

			/*
			 * handle mouse events
			 */

			case EVENT_TYPE_MOUSE:
				mouseEvent = (AsgardMouseEvent *)event->event;

				log_logf(&state.log, LOG_LEVEL_DEBUG,
						"message queue: mouse event #%i",
						mouseEvent->eventType);

				switch(mouseEvent->eventType) {
					case EVENT_MOUSE_MOVED:
						log_logf(&state.log, LOG_LEVEL_DEBUG,
								"mouse[MOVED]: (%i, %i)",
								mouseEvent->xPosition,
								mouseEvent->yPosition);
						state.mouseX = mouseEvent->xPosition;
						state.mouseY = mouseEvent->yPosition;
						break;
				}
				break;
		}

		events_freeEvent(event);
	}
}

static int unitTest_loadTarga()
{
	char *filename = NULL;

	Targa targa;

	log_logf(&state.log, LOG_LEVEL_INFO,
			"unit test %s(): STARTED", __FUNCTION__);

	filename = strdup(UT_LOAD_TARGA_FILENAME);

	if(targa_init(&targa) < 0) {
		log_logf(&state.log, LOG_LEVEL_ERROR,
				"unit test %s() failed at %i, aborting",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(targa_loadFromFile(&targa, filename) < 0) {
		log_logf(&state.log, LOG_LEVEL_ERROR,
				"unit test %s() failed at %i, aborting",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(targa_free(&targa) < 0) {
		log_logf(&state.log, LOG_LEVEL_ERROR,
				"unit test %s() failed at %i, aborting",
				__FUNCTION__, __LINE__);
		return -1;
	}

	free(filename);

	log_logf(&state.log, LOG_LEVEL_INFO,
			"unit test %s(): PASSED", __FUNCTION__);

	return 0;
}


// define Asgard basic unit test - test functions

static int unitTest_loadFont()
{
	log_logf(&state.log, LOG_LEVEL_INFO,
			"unit test %s(): STARTED", __FUNCTION__);

	if(font_init(&state.osFont,
				osapi_getWindowHandle(state.window),
				FONT_OS_DEFAULT,
				UT_FONT_SIZE,
				afalse,
				afalse,
				afalse,
				FONT_CHARSET_ANSI) < 0) {
		log_logf(&state.log, LOG_LEVEL_ERROR,
				"unit test %s() failed at %i, aborting",
				__FUNCTION__, __LINE__);
		return -1;
	}

	log_logf(&state.log, LOG_LEVEL_DEBUG,
			"constructed font '%s', id #%i",
			state.osFont.name, state.osFont.id);

	if(font_init(&state.lFont,
				osapi_getWindowHandle(state.window),
				FONT_LUCIDA,
				UT_FONT_SIZE,
				afalse,
				afalse,
				afalse,
				FONT_CHARSET_ANSI) < 0) {
		log_logf(&state.log, LOG_LEVEL_ERROR,
				"unit test %s() failed at %i, aborting",
				__FUNCTION__, __LINE__);
		return -1;
	}

	log_logf(&state.log, LOG_LEVEL_DEBUG,
			"constructed font '%s', id #%i",
			state.lFont.name, state.lFont.id);

	if(font_init(&state.aFont,
				osapi_getWindowHandle(state.window),
				FONT_ARIAL,
				UT_FONT_SIZE,
				afalse,
				afalse,
				afalse,
				FONT_CHARSET_ANSI) < 0) {
		log_logf(&state.log, LOG_LEVEL_ERROR,
				"unit test %s() failed at %i, aborting",
				__FUNCTION__, __LINE__);
		return -1;
	}

	log_logf(&state.log, LOG_LEVEL_DEBUG,
			"constructed font '%s', id #%i",
			state.aFont.name, state.aFont.id);

	log_logf(&state.log, LOG_LEVEL_INFO,
			"unit test %s(): PASSED", __FUNCTION__);

	return 0;
}

static int unitTest_freeFont()
{
	log_logf(&state.log, LOG_LEVEL_INFO,
			"unit test %s(): STARTED", __FUNCTION__);

	if(font_free(&state.osFont) < 0) {
		log_logf(&state.log, LOG_LEVEL_ERROR,
				"unit test %s() failed at %i, aborting",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(font_free(&state.lFont) < 0) {
		log_logf(&state.log, LOG_LEVEL_ERROR,
				"unit test %s() failed at %i, aborting",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(font_free(&state.aFont) < 0) {
		log_logf(&state.log, LOG_LEVEL_ERROR,
				"unit test %s() failed at %i, aborting",
				__FUNCTION__, __LINE__);
		return -1;
	}

	log_logf(&state.log, LOG_LEVEL_INFO,
			"unit test %s(): PASSED", __FUNCTION__);

	return 0;
}

static int unitTest_loadTexture()
{
	char *filename = NULL;

	Color colorMask;

	log_logf(&state.log, LOG_LEVEL_INFO,
			"unit test %s(): STARTED", __FUNCTION__);

	filename = strdup(UT_TEXTURE_FILENAME);

	if(texture_initFromTargaFile(
				&state.texture,
				UT_TEXTURE_QUALITY,
				UT_TEXTURE_RENDER_MODE,
				UT_TEXTURE_HARDWARE_RESIDENT,
				filename
				) < 0) {
		log_logf(&state.log, LOG_LEVEL_ERROR,
				"unit test %s() failed at %i, aborting",
				__FUNCTION__, __LINE__);
		return -1;
	}

	log_logf(&state.log, LOG_LEVEL_DEBUG,
			"constructed texture '%s' at %ix%i, image 0x%llx, id #%i",
			filename, state.texture.width, state.texture.height,
			(aulint)state.texture.image, state.texture.id);

	if(texture_initFromTargaFile(
				&state.mTexture,
				UT_TEXTURE_QUALITY,
				UT_TEXTURE_RENDER_MODE,
				UT_TEXTURE_HARDWARE_RESIDENT,
				filename
				) < 0) {
		log_logf(&state.log, LOG_LEVEL_ERROR,
				"unit test %s() failed at %i, aborting",
				__FUNCTION__, __LINE__);
		return -1;
	}

	log_logf(&state.log, LOG_LEVEL_DEBUG,
			"constructed texture '%s' at %ix%i, image 0x%llx, id #%i",
			filename, state.mTexture.width, state.mTexture.height,
			(aulint)state.mTexture.image, state.mTexture.id);

	colorMask.r = CGI_COLOR_DARKSEAGREEN_RED;
	colorMask.g = CGI_COLOR_DARKSEAGREEN_GREEN;
	colorMask.b = CGI_COLOR_DARKSEAGREEN_BLUE;
	colorMask.a = CGI_COLOR_DARKSEAGREEN_ALPHA;

	if(texture_setColorMask(&state.mTexture, &colorMask,
				TEXTURE_COLORMASK_MODE_MODULATE) < 0) {
		log_logf(&state.log, LOG_LEVEL_ERROR,
				"unit test %s() failed at %i, aborting",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(texture_initFromTargaFile(
				&state.aTexture,
				UT_TEXTURE_QUALITY,
				UT_TEXTURE_RENDER_MODE,
				UT_TEXTURE_HARDWARE_RESIDENT,
				filename
				) < 0) {
		log_logf(&state.log, LOG_LEVEL_ERROR,
				"unit test %s() failed at %i, aborting",
				__FUNCTION__, __LINE__);
		return -1;
	}

	log_logf(&state.log, LOG_LEVEL_DEBUG,
			"constructed texture '%s' at %ix%i, image 0x%llx, id #%i",
			filename, state.aTexture.width, state.aTexture.height,
			(aulint)state.aTexture.image, state.aTexture.id);

	if(texture_setColorMask(&state.aTexture, &colorMask,
				TEXTURE_COLORMASK_MODE_ADD) < 0) {
		log_logf(&state.log, LOG_LEVEL_ERROR,
				"unit test %s() failed at %i, aborting",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(texture_initFromTargaFile(
				&state.rTexture,
				UT_TEXTURE_QUALITY,
				UT_TEXTURE_RENDER_MODE,
				UT_TEXTURE_HARDWARE_RESIDENT,
				filename
				) < 0) {
		log_logf(&state.log, LOG_LEVEL_ERROR,
				"unit test %s() failed at %i, aborting",
				__FUNCTION__, __LINE__);
		return -1;
	}

	log_logf(&state.log, LOG_LEVEL_DEBUG,
			"constructed texture '%s' at %ix%i, image 0x%llx, id #%i",
			filename, state.rTexture.width, state.rTexture.height,
			(aulint)state.rTexture.image, state.rTexture.id);

	if(texture_applyRgbaChange(&state.rTexture, TEXTURE_COLOR_RED,
				UT_TEXTURE_RGBA_ALTERATION) < 0) {
		log_logf(&state.log, LOG_LEVEL_ERROR,
				"unit test %s() failed at %i, aborting",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(texture_applyRgbaChange(&state.rTexture, TEXTURE_COLOR_GREEN,
				UT_TEXTURE_RGBA_ALTERATION) < 0) {
		log_logf(&state.log, LOG_LEVEL_ERROR,
				"unit test %s() failed at %i, aborting",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(texture_applyRgbaChange(&state.rTexture, TEXTURE_COLOR_BLUE,
				UT_TEXTURE_RGBA_ALTERATION) < 0) {
		log_logf(&state.log, LOG_LEVEL_ERROR,
				"unit test %s() failed at %i, aborting",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(texture_applyRgbaChange(&state.rTexture, TEXTURE_COLOR_ALPHA,
				UT_TEXTURE_RGBA_ALTERATION) < 0) {
		log_logf(&state.log, LOG_LEVEL_ERROR,
				"unit test %s() failed at %i, aborting",
				__FUNCTION__, __LINE__);
		return -1;
	}

	colorMask.r = CGI_COLOR_ORCHID_RED;
	colorMask.g = CGI_COLOR_ORCHID_GREEN;
	colorMask.b = CGI_COLOR_ORCHID_BLUE;
	colorMask.a = CGI_COLOR_ORCHID_ALPHA;

	if(texture_initFromColor(
				&state.cTexture,
				UT_TEXTURE_QUALITY,
				UT_TEXTURE_RENDER_MODE,
				UT_TEXTURE_HARDWARE_RESIDENT,
				256,
				256,
				&colorMask
				) < 0) {
		log_logf(&state.log, LOG_LEVEL_ERROR,
				"unit test %s() failed at %i, aborting",
				__FUNCTION__, __LINE__);
		return -1;
	}

	log_logf(&state.log, LOG_LEVEL_DEBUG,
			"constructed texture '%s' at %ix%i, image 0x%llx, id #%i",
			filename, state.cTexture.width, state.cTexture.height,
			(aulint)state.cTexture.image, state.cTexture.id);

	if(texture_initFromTargaFile(
				&state.bTexture,
				UT_TEXTURE_QUALITY,
				UT_TEXTURE_RENDER_MODE,
				UT_TEXTURE_HARDWARE_RESIDENT,
				filename
				) < 0) {
		log_logf(&state.log, LOG_LEVEL_ERROR,
				"unit test %s() failed at %i, aborting",
				__FUNCTION__, __LINE__);
		return -1;
	}

	log_logf(&state.log, LOG_LEVEL_DEBUG,
			"constructed texture '%s' at %ix%i, image 0x%llx, id #%i",
			filename, state.bTexture.width, state.bTexture.height,
			(aulint)state.bTexture.image, state.bTexture.id);

	if(texture_setBlend(&state.bTexture,
				TEXTURE_BLEND_SOURCE_SRC_ALPHA,
				TEXTURE_BLEND_DEST_ONE_MINUS_SRC_ALPHA) < 0) {
		log_logf(&state.log, LOG_LEVEL_ERROR,
				"unit test %s() failed at %i, aborting",
				__FUNCTION__, __LINE__);
		return -1;
	}

	free(filename);

	log_logf(&state.log, LOG_LEVEL_INFO,
			"unit test %s(): PASSED", __FUNCTION__);

	return 0;
}

static int unitTest_freeTexture()
{
	log_logf(&state.log, LOG_LEVEL_INFO,
			"unit test %s(): STARTED", __FUNCTION__);

	if(texture_free(&state.texture) < 0) {
		log_logf(&state.log, LOG_LEVEL_ERROR,
				"unit test %s() failed at %i, aborting",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(texture_free(&state.mTexture) < 0) {
		log_logf(&state.log, LOG_LEVEL_ERROR,
				"unit test %s() failed at %i, aborting",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(texture_free(&state.aTexture) < 0) {
		log_logf(&state.log, LOG_LEVEL_ERROR,
				"unit test %s() failed at %i, aborting",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(texture_free(&state.rTexture) < 0) {
		log_logf(&state.log, LOG_LEVEL_ERROR,
				"unit test %s() failed at %i, aborting",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(texture_free(&state.cTexture) < 0) {
		log_logf(&state.log, LOG_LEVEL_ERROR,
				"unit test %s() failed at %i, aborting",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(texture_free(&state.bTexture) < 0) {
		log_logf(&state.log, LOG_LEVEL_ERROR,
				"unit test %s() failed at %i, aborting",
				__FUNCTION__, __LINE__);
		return -1;
	}

	log_logf(&state.log, LOG_LEVEL_INFO,
			"unit test %s(): PASSED", __FUNCTION__);

	return 0;
}

