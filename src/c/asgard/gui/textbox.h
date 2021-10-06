/*
 * textbox.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * A GUI widget for handling textboxes, header file.
 *
 * Written by Tom Brecht.
 */

#if !defined(_ASGARD_GUI_TEXTBOX_H)

#define _ASGARD_GUI_TEXTBOX_H

#if !defined(_ASGARD_H) && !defined(_ASGARD_GUI_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _ASGARD_H || _ASGARD_GUI_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// define gui textbox public constants

#define GUI_TEXTBOX_STATES									2

typedef enum _GuiTextboxStates {
	GUI_TEXTBOX_STATE_NORMAL = 0,
	GUI_TEXTBOX_STATE_FOCUSED,
	GUI_TEXTBOX_STATE_UNKNOWN = -1
} GuiTextboxStates;


// define gui textbox public data types

typedef struct _GuiTextbox {
	aboolean isVisible;
	aboolean wasClicked;
	aboolean isMasked;
	int state;
	char *name;
	int maxChars;
	int fontX;
	int fontY;
	int historySize;
	int historyIndex;
	int bufferIndex;
	int visualBeginning;
	int visualEnd;
	int blink;
	Color fontColor;
	Color backgroundColor;
	char *submitted;
	ArrayList *contents;
	GuiPosition position;
	GuiFontInfo fontInfo;
	//Cursor gameCursor;
	//Cursor textCursor;
	Texture *background;
	Texture *textures[GUI_TEXTBOX_STATES];
	void *engine;
} GuiTextbox;


// declare gui textbox public functions

void guiTextbox_init(GuiTextbox *textbox, char *name, int x, int y, int width,
		int height, void *engine);

void guiTextbox_free(GuiTextbox *textbox);

aboolean guiTextbox_isNormal(GuiTextbox *textbox);

aboolean guiTextbox_isFocused(GuiTextbox *textbox);

int guiTextbox_getState(GuiTextbox *textbox);

char *guiTextbox_getName(GuiTextbox *textbox);

void guiTextbox_setName(GuiTextbox *textbox, char *name);

int guiTextbox_getX(GuiTextbox *textbox);

int guiTextbox_getY(GuiTextbox *textbox);

GuiPosition *guiTextbox_getPosition(GuiTextbox *textbox);

void guiTextbox_setPosition(GuiTextbox *textbox, int x, int y);

int guiTextbox_getWidth(GuiTextbox *textbox);

int guiTextbox_getHeight(GuiTextbox *textbox);

void guiTextbox_resize(GuiTextbox *textbox, int width, int height);

aboolean guiTextbox_isVisible(GuiTextbox *textbox);

void guiTextbox_setVisibility(GuiTextbox *textbox, aboolean isVisible);

aboolean guiTextbox_isMasked(GuiTextbox *textbox);

void guiTextbox_setMasking(GuiTextbox *textbox, aboolean isMasked);

Font *guiTextbox_getFont(GuiTextbox *textbox);

void guiTextbox_setFont(GuiTextbox *textbox, Font *font);

Color *guiTextbox_getFontColor(GuiTextbox *textbox);

void guiTextbox_setFontColor(GuiTextbox *textbox, Color *color);

Color *guiTextbox_getBackgroundColor(GuiTextbox *textbox);

void guiTextbox_setBackgroundColor(GuiTextbox *textbox, Color *color);

void guiTextbox_setColorScheme(GuiTextbox *textbox, Color *backgroundColor,
		Color *displayColor);

Texture *guiTextbox_getTexture(GuiTextbox *textbox, int textboxState);

void guiTextbox_setTexture(GuiTextbox *textbox, int textboxState,
		Texture *texture);

Texture *guiTextbox_getBackgroundTexture(GuiTextbox *textbox);

void guiTextbox_setBackgroundTexture(GuiTextbox *textbox, Texture *texture);

int guiTextbox_getMaxChars(GuiTextbox *textbox);

void guiTextbox_setMaxChars(GuiTextbox *textbox, int maxChars);

aboolean guiTextbox_hasSubmitted(GuiTextbox *textbox);

void guiTextbox_setContents(GuiTextbox *textbox, char *text);

char *guiTextbox_getContents(GuiTextbox *textbox);

char *guiTextbox_getSubmittedContents(GuiTextbox *textbox);

aboolean guiTextbox_hasFocus(GuiTextbox *textbox, int mouseX, int mouseY);

void guiTextbox_deFocus(GuiTextbox *textbox);

aboolean guiTextbox_wasClicked(GuiTextbox *textbox, int mouseX, int mouseY);

void guiTextbox_keyPressed(GuiTextbox *textbox, AsgardKeyboardEvent *event);

void guiTextbox_render2d(GuiTextbox *textbox);


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _ASGARD_GUI_TEXTBOX_H

