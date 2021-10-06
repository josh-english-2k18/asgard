/*
 * font_windows.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * A plugin to load a Windows font as an OpenGL compatible display list.
 *
 * Written by Josh English.
 */

#if defined(WIN32)

// preprocessor directives

#include "core/core.h"
#define _ASGARD_CORE_COMPONENT
#include "asgard/core/common.h"
#include "asgard/core/colors.h"
#define _ASGARD_CORE_PLUGIN_COMPONENT
#include "asgard/core/plugin/opengl.h"
#include "asgard/core/canvas.h"
#define _ASGARD_ENGINE_COMPONENT
#include "asgard/engine/font.h"
#define _ASGARD_ENGINE_PLUGIN_COMPONENT
#include "asgard/engine/plugin/font_windows.h"

// define font (windows) private constants

//#define OUTPUT_QUALITY							ANTIALIASED_QUALITY
#define OUTPUT_QUALITY							PROOF_QUALITY

// define font (windows) public functions

int font_loadWindows(void *screenHandle, char *name, int height,
		aboolean isBold, aboolean isItalic, aboolean isUnderline, int charset,
		void **fontHandle, int *glFontId)
{
	int fontWeight = FW_NORMAL;

	DWORD win32Charset = (DWORD)0;
	HFONT handle = (HFONT)NULL;

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

	if(isBold) {
		fontWeight = FW_BOLD;
	}

	switch(charset) {
		case FONT_CHARSET_ANSI:
			win32Charset = ANSI_CHARSET;
			break;
		case FONT_CHARSET_BALTIC:
			win32Charset = BALTIC_CHARSET;
			break;
		case FONT_CHARSET_CHINESE:
			win32Charset = CHINESEBIG5_CHARSET;
			break;
		case FONT_CHARSET_EAST_EUROPE:
			win32Charset = EASTEUROPE_CHARSET;
			break;
		case FONT_CHARSET_GREAT_BRITAIN:
			win32Charset = GB2312_CHARSET;
			break;
		case FONT_CHARSET_GREEK:
			win32Charset = GREEK_CHARSET;
			break;
		case FONT_CHARSET_HANGUL:
			win32Charset = HANGUL_CHARSET;
			break;
		case FONT_CHARSET_RUSSIAN:
			win32Charset = RUSSIAN_CHARSET;
			break;
		case FONT_CHARSET_TURKISH:
			win32Charset = TURKISH_CHARSET;
			break;
		case FONT_CHARSET_VIETNAMESE:
			win32Charset = VIETNAMESE_CHARSET;
			break;
		default:
			win32Charset = ANSI_CHARSET;
	}

	handle = CreateFont(
			height,							// height
			0,								// width
			0,								// angle of escapement
			0,								// angle of orientation
			fontWeight,						// font weight
			(DWORD)isItalic,				// italic
			(DWORD)isUnderline,				// underline
			(DWORD)0,						// strike through
			win32Charset,					// charset
			OUT_TT_PRECIS,					// use a true-type font
			CLIP_DEFAULT_PRECIS,			// default clipping precision
			OUTPUT_QUALITY,					// output quality
			DEFAULT_PITCH | FF_DONTCARE,	// pitch & family
			(LPCTSTR)name					// name
			);
	if(handle == NULL) {
		return -1;
	}

	*fontHandle = (void *)SelectObject(screenHandle, handle);

	*glFontId = cgi_glGenLists(FONT_CHARSET_LENGTH);

	wglUseFontBitmaps(screenHandle, 0, (FONT_CHARSET_LENGTH - 1), *glFontId);

	return 0;
}

int font_freeWindows(void *screenHandle, void *fontHandle)
{
	if((screenHandle == NULL) || (fontHandle == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	SelectObject(screenHandle, fontHandle);

	return 0;
}

#endif // WIN32

