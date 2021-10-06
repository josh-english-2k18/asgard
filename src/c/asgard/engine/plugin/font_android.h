/*
 * font_android.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * A plugin to load a Android font as an OpenGL compatible display list, header
 * file.
 *
 * Written by Josh English.
 */

#if !defined(_ASGARD_ENGINE_PLUGIN_FONT_ANDROID_H)

#define _ASGARD_ENGINE_PLUGIN_FONT_ANDROID_H

#if !defined(_ASGARD_H) && !defined(_ASGARD_ENGINE_PLUGIN_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _ASGARD_H || _ASGARD_ENGINE_PLUGIN_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// declare font (android) public functions

int font_loadAndroid(void *screenHandle, char *name, int height,
		aboolean isBold, aboolean isItalic, aboolean isUnderline, int charset,
		void **fontHandle, int *glFontId);

int font_freeAndroid(void *screenHandle, void *fontHandle);


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _ASGARD_ENGINE_PLUGIN_FONT_ANDROID_H

