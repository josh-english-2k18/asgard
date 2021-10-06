/*
 * font_linux.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * A plugin to load a Linux font as an OpenGL compatible display list.
 *
 * Written by Josh English.
 */

#if defined(__linux__) && !defined(__ANDROID__)

// preprocessor directives

#include "core/core.h"
#define _ASGARD_CORE_COMPONENT
#include "asgard/core/common.h"
#include "asgard/core/colors.h"
#include "asgard/core/events.h"
#define _ASGARD_CORE_PLUGIN_COMPONENT
#include "asgard/core/plugin/opengl.h"
#include "asgard/core/plugin/linuxapi.h"
#include "asgard/core/canvas.h"
#define _ASGARD_ENGINE_COMPONENT
#include "asgard/engine/font.h"
#define _ASGARD_ENGINE_PLUGIN_COMPONENT
#include "asgard/engine/plugin/font_linux.h"

// define font (linux) public functions

int font_loadLinux(void *screenHandle, char *name, int height,
		aboolean isBold, aboolean isItalic, aboolean isUnderline, int charset,
		void **fontHandle, int *glFontId)
{
	char fontItalic = 'r';
	char *fontStyle = "medium";
	char fontBuffer[8192];

	XFontStruct *font = NULL;
	LinuxApiWindow *window = NULL;

	if((screenHandle == NULL) || (name == NULL) || (height < 1) ||
			((charset != FONT_CHARSET_ANSI) &&
			 (charset != FONT_CHARSET_BALTIC) &&
			 (charset != FONT_CHARSET_CHINESE) &&
			 (charset != FONT_CHARSET_EAST_EUROPE) &&
			 (charset != FONT_CHARSET_GREAT_BRITAIN) &&
			 (charset != FONT_CHARSET_GREEK) &&
			 (charset != FONT_CHARSET_HANGUL) &&
			 (charset != FONT_CHARSET_RUSSIAN) &&
			 (charset != FONT_CHARSET_TURKISH) &&
			 (charset != FONT_CHARSET_VIETNAMESE)) ||
			(fontHandle == NULL) || (glFontId == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	window = (LinuxApiWindow *)screenHandle;
	*fontHandle = NULL;
	*glFontId = -1;

	if(isBold) {
		fontStyle = "bold";
	}
	if(isItalic) {
		fontItalic = 'i';
	}

	snprintf(fontBuffer, sizeof(fontBuffer),
			"-*-%s-%s-%c-normal-*-%i-*-*-*-*-*-*-*",
			name, fontStyle, fontItalic, height);

	font = XLoadQueryFont(window->display, fontBuffer);

	if(font == NULL) {
		snprintf(fontBuffer, sizeof(fontBuffer),
				"-*-%s-%s-%c-normal-*-%i-*-*-*-*-*-*-*",
				name, "medium", 'r', height);

		font = XLoadQueryFont(window->display, fontBuffer);
	}

	if(font == NULL) {
		snprintf(fontBuffer, sizeof(fontBuffer),
				"-*-%s-%s-%c-normal-*-%i-*-*-*-*-*-*-*",
				name, "*", '*', height);

		font = XLoadQueryFont(window->display, fontBuffer);
	}

	if(font == NULL) {
		snprintf(fontBuffer, sizeof(fontBuffer),
				"-*-%s-%s-%c-normal-*-%i-*-*-*-*-*-*-*",
				name, "*", '*', 10);

		font = XLoadQueryFont(window->display, fontBuffer);
	}

	if(font == NULL) {
		strcpy(fontBuffer, "-*-*-*-r-*-sans-*-120-*");

		font = XLoadQueryFont(window->display, fontBuffer);
	}

	if(font == NULL) {
		strcpy(fontBuffer, "fixed");

		font = XLoadQueryFont(window->display, fontBuffer);
	}

	if(font == NULL) {
		fprintf(stderr,
				"error - failed to load font '%s' with X query '%s'.\n",
				name, fontBuffer);
		return -1;
	}

	*glFontId = cgi_glGenLists(FONT_CHARSET_LENGTH);

	glXUseXFont(font->fid, 0, (FONT_CHARSET_LENGTH - 1), *glFontId);

	*fontHandle = (void *)font;

	return 0;
}

int font_freeLinux(void *screenHandle, void *fontHandle)
{
	LinuxApiWindow *window = NULL;

	if((screenHandle == NULL) || (fontHandle == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	window = (LinuxApiWindow *)screenHandle;

	XFreeFont(window->display, (XFontStruct *)fontHandle);

	return 0;
}

#endif // __linux__ && !__ANDROID__

