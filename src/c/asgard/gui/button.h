/*
 * button.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * A GUI widget for handling buttons, header file.
 *
 * Written by Josh English.
 */

#if !defined(_ASGARD_GUI_BUTTON_H)

#define _ASGARD_GUI_BUTTON_H

#if !defined(_ASGARD_H) && !defined(_ASGARD_GUI_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _ASGARD_H || _ASGARD_GUI_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// define gui button public constants

#define GUI_BUTTON_STATES								4

typedef enum _GuiButtonStates {
	GUI_BUTTON_STATE_NORMAL = 0,
	GUI_BUTTON_STATE_FOCUSED,
	GUI_BUTTON_STATE_HIGHLIGHTED,
	GUI_BUTTON_STATE_DOWN,
	GUI_BUTTON_STATE_UNKNOWN = -1
} GuiButtonStates;


// define gui button public data types

typedef struct _GuiButton {
	aboolean isVisible;
	aboolean isFixedState;
	aboolean wasHighlighted;
	int state;
	char *name;
	char *text;
	Color fontColor;
	Color backgroundColor;
	GuiPosition position;
	GuiFontInfo fontInfo;
	Texture *background;
	Texture *textures[GUI_BUTTON_STATES];
	void *engine;
} GuiButton;


// declare gui button public functions

void guiButton_init(GuiButton *button, char *name, int x, int y, int width,
		int height, void *engine);

void guiButton_free(GuiButton *button);

aboolean guiButton_isNormal(GuiButton *button);

aboolean guiButton_isFocused(GuiButton *button);

aboolean guiButton_isHighlighted(GuiButton *button);

aboolean guiButton_isDown(GuiButton *button);

int guiButton_getState(GuiButton *button);

char *guiButton_getName(GuiButton *button);

void guiButton_setName(GuiButton *button, char *name);

int guiButton_getX(GuiButton *button);

int guiButton_getY(GuiButton *button);

GuiPosition *guiButton_getPosition(GuiButton *button);

void guiButton_setPosition(GuiButton *button, int x, int y);

int guiButton_getWidth(GuiButton *button);

int guiButton_getHeight(GuiButton *button);

void guiButton_resize(GuiButton *button, int width, int height);

aboolean guiButton_isVisible(GuiButton *button);

void guiButton_setVisibility(GuiButton *button, aboolean isVisible);

aboolean guiButton_isFixedState(GuiButton *button);

void guiButton_setFixedState(GuiButton *button, aboolean isFixedState);

char *guiButton_getText(GuiButton *button);

void guiButton_setText(GuiButton *button, Font *font, char *text);

void guiButton_setColorText(GuiButton *button, Font *font, Color *color,
		char *text);

Color *guiButton_getFontColor(GuiButton *button);

void guiButton_setFontColor(GuiButton *button, Color *color);

Color *guiButton_getBackgroundColor(GuiButton *button);

void guiButton_setBackgroundColor(GuiButton *button, Color *color);

Texture *guiButton_getTexture(GuiButton *button, int buttonState);

void guiButton_setTexture(GuiButton *button, int buttonState, Texture *texture);

Texture *guiButton_getBackgroundTexture(GuiButton *button, int buttonState);

void guiButton_setBackgroundTexture(GuiButton *button, int buttonState,
		Texture *texture);

aboolean guiButton_hasFocus(GuiButton *button, int mouseX, int mouseY);

void guiButton_deFocus(GuiButton *button);

void guiButton_deHighlight(GuiButton *button);

aboolean guiButton_wasClicked(GuiButton *button, int mouseX, int mouseY);

void guiButton_keyPressed(GuiButton *button, AsgardKeyboardEvent *event);

void guiButton_render2d(GuiButton *button);


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _ASGARD_GUI_BUTTON_H

