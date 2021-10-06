/*
 * autoscroll.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * A GUI widget for handling auto-scrolling widgets, header file.
 *
 * Written by Tom Brecht.
 */

#if !defined(_ASGARD_GUI_AUTOSCROLL_H)

#define _ASGARD_GUI_AUTOSCROLL_H

#if !defined(_ASGARD_H) && !defined(_ASGARD_GUI_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _ASGARD_H || _ASGARD_GUI_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// define gui autoscroll private constants

typedef enum _GuiAutoscrollEntryTypes {
	GUI_AUTOSCROLL_ENTRY_TYPE_TEXT = 1,
	GUI_AUTOSCROLL_ENTRY_TYPE_IMAGE = 2,
	GUI_AUTOSCROLL_ENTRY_TYPE_HYPERLINK = 3
} GuiAutoscrollEntryTypes;


// define gui autoscroll private data types

typedef struct _GuiAutoscrollEntry {
	int type;
	int width;
	int height;
	Color color;
	void *object;
} GuiAutoscrollEntry;


// define gui autoscroll public constants

#define GUI_AUTOSCROLL_MAX_ENTRIES								128


// define gui autoscroll public data types

typedef struct _GuiAutoscroll {
	aboolean isVisible;
	aboolean isRunning;
	aboolean hasOutline;
	int scrollTicks;
	int scrollCount;
	char *name;
	Color fontColor;
	Color backgroundColor;
	Color outlineColor;
	GuiPosition position;
	GuiFontInfo fontInfo;
	Texture *background;
	GuiAutoscrollEntry *entries[GUI_AUTOSCROLL_MAX_ENTRIES];
	void *engine;
} GuiAutoscroll;


// declare gui autoscroll public functions

void guiAutoscroll_init(GuiAutoscroll *autoscroll, char *name, int x, int y,
		int width, int height, void *engine);

void guiAutoscroll_free(GuiAutoscroll *autoscroll);

char *guiAutoscroll_getName(GuiAutoscroll *autoscroll);

void guiAutoscroll_setName(GuiAutoscroll *autoscroll, char *name);

int guiAutoscroll_getX(GuiAutoscroll *autoscroll);

int guiAutoscroll_getY(GuiAutoscroll *autoscroll);

GuiPosition *guiAutoscroll_getPosition(GuiAutoscroll *autoscroll);

void guiAutoscroll_setPosition(GuiAutoscroll *autoscroll, int x, int y);

int guiAutoscroll_getWidth(GuiAutoscroll *autoscroll);

int guiAutoscroll_getHeight(GuiAutoscroll *autoscroll);

void guiAutoscroll_resize(GuiAutoscroll *autoscroll, int width, int height);

aboolean guiAutoscroll_isVisible(GuiAutoscroll *autoscroll);

void guiAutoscroll_setVisibility(GuiAutoscroll *autoscroll, aboolean isVisible);

aboolean guiAutoscroll_isRunning(GuiAutoscroll *autoscroll);

aboolean guiAutoscroll_hasOutline(GuiAutoscroll *autoscroll);

void guiAutoscroll_setHasOutline(GuiAutoscroll *autoscroll, 
		aboolean hasOutline);

Color *guiAutoscroll_getFontColor(GuiAutoscroll *autoscroll);

void guiAutoscroll_setFontColor(GuiAutoscroll *autoscroll, Color *color);

Color *guiAutoscroll_getBackgroundColor(GuiAutoscroll *autoscroll);

void guiAutoscroll_setBackgroundColor(GuiAutoscroll *autoscroll, Color *color);

Color *guiAutoscroll_getOutlineColor(GuiAutoscroll *autoscroll);

void guiAutoscroll_setOutlineColor(GuiAutoscroll *autoscroll, Color *color);

Texture *guiAutoscroll_getBackgroundTexture(GuiAutoscroll *autoscroll);

void guiAutoscroll_setBackgroundTexture(GuiAutoscroll *autoscroll, 
		Texture *texture);

void guiAutoscroll_addColorText(GuiAutoscroll *autoscroll, Color *color, 
		char *text);

void guiAutoscroll_addText(GuiAutoscroll *autoscroll, char *text);

void guiAutoscroll_addImage(GuiAutoscroll *autoscroll, Texture *texture, 
		int width, int height);

void guiAutoscroll_addHyperlink(GuiAutoscroll *autoscroll, char *hyperlink);

aboolean guiAutoscroll_hasFocus(GuiAutoscroll *autoscroll, int mouseX, 
		int mouseY);

void guiAutoscroll_deFocus(GuiAutoscroll *autoscroll);

aboolean guiAutoscroll_wasClicked(GuiAutoscroll *autoscroll, int mouseX, 
		int mouseY);

void guiAutoscroll_keyPressed(GuiAutoscroll *autoscroll,
		AsgardKeyboardEvent *event);

void guiAutoscroll_reset(GuiAutoscroll *autoscroll);

void guiAutoscroll_process(GuiAutoscroll *autoscroll);

void guiAutoscroll_render2d(GuiAutoscroll *autoscroll);


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _ASGARD_GUI_AUTOSCROLL_H
