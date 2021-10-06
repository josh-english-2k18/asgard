/*
 * common.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The common header-file for the Asgard Game Engine GUI widget library.
 *
 * Written by Josh English.
 */

#if !defined(_ASGARD_GUI_COMMON_H)

#define _ASGARD_GUI_COMMON_H

#if !defined(_ASGARD_H) && !defined(_ASGARD_GUI_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _ASGARD_H || _ASGARD_GUI_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// define common gui public constants

#define GUI_DEFAULT_FONT_GLYPH_WIDTH						9
#define GUI_DEFAULT_FONT_GLYPH_HEIGHT						9


// define common gui public data types

typedef struct _GuiPosition {
	int x;
	int y;
	int width;
	int height;
} GuiPosition;

typedef struct _GuiFontInfo {
	int x;
	int y;
	int glyphWidth;
	int glyphHeight;
	Font *font;
} GuiFontInfo;


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _ASGARD_GUI_COMMON_H

