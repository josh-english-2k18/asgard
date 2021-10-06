/*
 * font_android.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * A plugin to load a Android font as an OpenGL compatible display list.
 *
 * Written by Josh English.
 */

#if defined(__ANDROID__)

// preprocessor directives

#include "core/core.h"
#define _ASGARD_CORE_COMPONENT
#include "asgard/core/common.h"
#include "asgard/core/colors.h"
#include "asgard/core/events.h"
#define _ASGARD_CORE_PLUGIN_COMPONENT
#include "asgard/core/plugin/opengl.h"
#include "asgard/core/plugin/androidapi.h"
#include "asgard/core/canvas.h"
#define _ASGARD_ENGINE_COMPONENT
#include "asgard/engine/font.h"
#define _ASGARD_ENGINE_PLUGIN_COMPONENT
#include "asgard/engine/plugin/font_android.h"

// define font (android) public functions

int font_loadAndroid(void *screenHandle, char *name, int height,
		aboolean isBold, aboolean isItalic, aboolean isUnderline, int charset,
		void **fontHandle, int *glFontId)
{
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

	*fontHandle = NULL;
	*glFontId = -1;

	return 0;
}

int font_freeAndroid(void *screenHandle, void *fontHandle)
{
	if((screenHandle == NULL) || (fontHandle == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return 0;
}

#endif // __ANDROID__

