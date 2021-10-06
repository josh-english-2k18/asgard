/*
 * button.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * A GUI widget for handling buttons.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#include "asgard/asgard.h"
#define _ASGARD_GUI_COMPONENT
#include "asgard/gui/button.h"


// declare gui button private functions

static void setFontCoords(GuiButton *button);


// define gui button private functions

static void setFontCoords(GuiButton *button)
{
	int length = 0;

	if(button->text == NULL) {
		return;
	}

	length = strlen(button->text);

	button->fontInfo.x = (button->position.x + (button->position.width / 2) -
			((button->fontInfo.glyphWidth * length) / 2));

	button->fontInfo.y = ((button->position.y +
				(button->position.height / 2)) - button->fontInfo.glyphHeight);
}


// define gui button public functions

void guiButton_init(GuiButton *button, char *name, int x, int y, int width,
		int height, void *engine)
{
	if((button == NULL) || (engine == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	memset(button, 0, (sizeof(GuiButton)));

	button->isVisible = atrue;
	button->isFixedState = afalse;
	button->wasHighlighted = afalse;
	button->state = GUI_BUTTON_STATE_NORMAL;

	button->name = strdup(name);
	button->text = NULL;

	button->fontColor.r = CGI_COLOR_BLACK_RED;
	button->fontColor.g = CGI_COLOR_BLACK_GREEN;
	button->fontColor.b = CGI_COLOR_BLACK_BLUE;
	button->fontColor.a = CGI_COLOR_BLACK_ALPHA;

	button->backgroundColor.r = CGI_COLOR_LIGHTGREY_RED;
	button->backgroundColor.g = CGI_COLOR_LIGHTGREY_GREEN;
	button->backgroundColor.b = CGI_COLOR_LIGHTGREY_BLUE;
	button->backgroundColor.a = CGI_COLOR_LIGHTGREY_ALPHA;

	button->position.x = x;
	button->position.y = y;
	button->position.width = width;
	button->position.height = height;

	button->fontInfo.x = 0;
	button->fontInfo.y = 0;
	button->fontInfo.glyphWidth = GUI_DEFAULT_FONT_GLYPH_WIDTH;
	button->fontInfo.glyphHeight = GUI_DEFAULT_FONT_GLYPH_HEIGHT;
	button->fontInfo.font = NULL;

	button->background = NULL;
	button->textures[GUI_BUTTON_STATE_NORMAL] = NULL;
	button->textures[GUI_BUTTON_STATE_FOCUSED] = NULL;
	button->textures[GUI_BUTTON_STATE_HIGHLIGHTED] = NULL;
	button->textures[GUI_BUTTON_STATE_DOWN] = NULL;

	button->engine = engine;
}

void guiButton_free(GuiButton *button)
{
	if(button == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	/*
	 * TODO: this function
	 */
}

aboolean guiButton_isNormal(GuiButton *button)
{
	if(button == NULL) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	if(button->state == GUI_BUTTON_STATE_NORMAL) {
		return atrue;
	}

	return afalse;
}

aboolean guiButton_isFocused(GuiButton *button)
{
	if(button == NULL) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	if(button->state == GUI_BUTTON_STATE_FOCUSED) {
		return atrue;
	}

	return afalse;
}

aboolean guiButton_isHighlighted(GuiButton *button)
{
	if(button == NULL) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	if(button->state == GUI_BUTTON_STATE_HIGHLIGHTED) {
		return atrue;
	}

	return afalse;
}

aboolean guiButton_isDown(GuiButton *button)
{
	if(button == NULL) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	if(button->state == GUI_BUTTON_STATE_DOWN) {
		return atrue;
	}

	return afalse;
}

int guiButton_getState(GuiButton *button)
{
	if(button == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return button->state;
}

char *guiButton_getName(GuiButton *button)
{
	if(button == NULL) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	return button->name;
}

void guiButton_setName(GuiButton *button, char *name)
{
	if(button == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	if(button->name != NULL) {
		free(button->name);
	}

	button->name = strdup(name);
}

int guiButton_getX(GuiButton *button)
{
	if(button == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return button->position.x;
}

int guiButton_getY(GuiButton *button)
{
	if(button == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return button->position.y;
}

GuiPosition *guiButton_getPosition(GuiButton *button)
{
	if(button == NULL) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	return &(button->position);
}

void guiButton_setPosition(GuiButton *button, int x, int y)
{
	if(button == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	button->position.x = x;
	button->position.y = y;
}

int guiButton_getWidth(GuiButton *button)
{
	if(button == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return button->position.width;
}

int guiButton_getHeight(GuiButton *button)
{
	if(button == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return button->position.height;
}

void guiButton_resize(GuiButton *button, int width, int height)
{
	if(button == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	button->position.width = width;
	button->position.height = height;
}

aboolean guiButton_isVisible(GuiButton *button)
{
	if(button == NULL) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	return button->isVisible;
}

void guiButton_setVisibility(GuiButton *button, aboolean isVisible)
{
	if(button == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	button->isVisible = isVisible;
}

aboolean guiButton_isFixedState(GuiButton *button)
{
	if(button == NULL) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	return button->isFixedState;
}

void guiButton_setFixedState(GuiButton *button, aboolean isFixedState)
{
	if(button == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	button->isFixedState = isFixedState;
}

char *guiButton_getText(GuiButton *button)
{
	if(button == NULL) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	return button->text;
}

void guiButton_setText(GuiButton *button, Font *font, char *text)
{
	if((button == NULL) || (text == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	if(button->text != NULL) {
		free(button->text);
	}

	button->text = strdup(text);

	button->fontInfo.font = font;

	setFontCoords(button);
}

void guiButton_setColorText(GuiButton *button, Font *font, Color *color,
		char *text)
{
	if((button == NULL) || (color == NULL) || (text == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	guiButton_setFontColor(button, color);
	guiButton_setText(button, font, text);
}

Color *guiButton_getFontColor(GuiButton *button)
{
	if(button == NULL) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	return &(button->fontColor);
}

void guiButton_setFontColor(GuiButton *button, Color *color)
{
	if((button == NULL) || (color == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	button->fontColor.r = color->r;
	button->fontColor.g = color->g;
	button->fontColor.b = color->b;
	button->fontColor.a = color->a;
}

Color *guiButton_getBackgroundColor(GuiButton *button)
{
	if(button == NULL) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	return &(button->backgroundColor);
}

void guiButton_setBackgroundColor(GuiButton *button, Color *color)
{
	if((button == NULL) || (color == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	button->backgroundColor.r = color->r;
	button->backgroundColor.g = color->g;
	button->backgroundColor.b = color->b;
	button->backgroundColor.a = color->a;
}

Texture *guiButton_getTexture(GuiButton *button, int buttonState)
{
	if((button == NULL) ||
			((buttonState != GUI_BUTTON_STATE_NORMAL) &&
			 (buttonState != GUI_BUTTON_STATE_FOCUSED) &&
			 (buttonState != GUI_BUTTON_STATE_HIGHLIGHTED) &&
			 (buttonState != GUI_BUTTON_STATE_DOWN))) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	return button->textures[buttonState];
}

void guiButton_setTexture(GuiButton *button, int buttonState, Texture *texture)
{
	if((button == NULL) ||
			((buttonState != GUI_BUTTON_STATE_NORMAL) &&
			 (buttonState != GUI_BUTTON_STATE_FOCUSED) &&
			 (buttonState != GUI_BUTTON_STATE_HIGHLIGHTED) &&
			 (buttonState != GUI_BUTTON_STATE_DOWN)) ||
			(texture == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	button->textures[buttonState] = texture;
}

Texture *guiButton_getBackgroundTexture(GuiButton *button, int buttonState)
{
	if((button == NULL) ||
			((buttonState != GUI_BUTTON_STATE_NORMAL) &&
			 (buttonState != GUI_BUTTON_STATE_FOCUSED) &&
			 (buttonState != GUI_BUTTON_STATE_HIGHLIGHTED) &&
			 (buttonState != GUI_BUTTON_STATE_DOWN))) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	return button->background;
}

void guiButton_setBackgroundTexture(GuiButton *button, int buttonState,
		Texture *texture)
{
	if((button == NULL) ||
			((buttonState != GUI_BUTTON_STATE_NORMAL) &&
			 (buttonState != GUI_BUTTON_STATE_FOCUSED) &&
			 (buttonState != GUI_BUTTON_STATE_HIGHLIGHTED) &&
			 (buttonState != GUI_BUTTON_STATE_DOWN)) ||
			(texture == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	button->background = texture;
}

aboolean guiButton_hasFocus(GuiButton *button, int mouseX, int mouseY)
{
	aboolean result = afalse;

	if(button == NULL) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	if(!button->isVisible) {
		button->state = GUI_BUTTON_STATE_NORMAL;
		return afalse;
	}

	if((mouseX >= button->position.x) &&
			(mouseX <= (button->position.x + button->position.width)) &&
			(mouseY >= button->position.y) &&
			(mouseY <= (button->position.y + button->position.height))) {
		result = atrue;
	}

	if(button->state == GUI_BUTTON_STATE_DOWN) {
		if(result) {
			button->state = GUI_BUTTON_STATE_HIGHLIGHTED;
			button->wasHighlighted = atrue;
		}
		else {
			button->state = GUI_BUTTON_STATE_NORMAL;
		}
	}
	else if(button->state == GUI_BUTTON_STATE_HIGHLIGHTED) {
		if(result) {
			button->state = GUI_BUTTON_STATE_FOCUSED;
		}
		else {
			if(button->wasHighlighted) {
				button->state = GUI_BUTTON_STATE_HIGHLIGHTED;
			}
			else {
				button->state = GUI_BUTTON_STATE_NORMAL;
			}
		}
	}
	else {
		if(result) {
			button->state = GUI_BUTTON_STATE_FOCUSED;
		}
		else {
			if(button->wasHighlighted) {
				button->state = GUI_BUTTON_STATE_HIGHLIGHTED;
			}
			else {
				button->state = GUI_BUTTON_STATE_NORMAL;
			}
		}
	}

	return result;
}

void guiButton_deFocus(GuiButton *button)
{
	if(button == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	if(button->state != GUI_BUTTON_STATE_HIGHLIGHTED) {
		button->state = GUI_BUTTON_STATE_NORMAL;
	}
}

void guiButton_deHighlight(GuiButton *button)
{
	if(button == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	button->wasHighlighted = afalse;
	button->state = GUI_BUTTON_STATE_NORMAL;
}

aboolean guiButton_wasClicked(GuiButton *button, int mouseX, int mouseY)
{
	aboolean result = afalse;

	if(button == NULL) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	if(!button->isVisible) {
		return afalse;
	}

	if((mouseX >= button->position.x) &&
			(mouseX <= (button->position.x + button->position.width)) &&
			(mouseY >= button->position.y) &&
			(mouseY <= (button->position.y + button->position.height))) {
		result = atrue;
	}

	if(result) {
		if(button->isFixedState) {
			if(button->state == GUI_BUTTON_STATE_DOWN) {
				button->state = GUI_BUTTON_STATE_FOCUSED;
			}
			else {
				button->state = GUI_BUTTON_STATE_DOWN;
			}
		}
		else {
			button->state = GUI_BUTTON_STATE_DOWN;
		}
	}
	else if(!button->isFixedState) {
		if(button->state == GUI_BUTTON_STATE_DOWN) {
			button->state = GUI_BUTTON_STATE_HIGHLIGHTED;
			button->wasHighlighted = atrue;
		}
		else {
			button->state = GUI_BUTTON_STATE_NORMAL;
		}
	}

	return result;
}

void guiButton_keyPressed(GuiButton *button, AsgardKeyboardEvent *event)
{
	if((button == NULL) || (event == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	if(!button->isVisible) {
		return;
	}

	if((event->eventType == EVENT_KEYBOARD_TYPED) &&
			((event->keyCode == 10) || ((event->keyCode == 13)))) {
		if(button->isFixedState) {
			if(button->state == GUI_BUTTON_STATE_DOWN) {
				button->state = GUI_BUTTON_STATE_FOCUSED;
			}
			else {
				button->state = GUI_BUTTON_STATE_DOWN;
			}
		}
		else {
			button->state = GUI_BUTTON_STATE_DOWN;
		}
	}
}

void guiButton_render2d(GuiButton *button)
{
	if(button == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	if(!button->isVisible) {
		return;
	}

	// render background

	if(button->background != NULL) {
		draw2d_texture(&((Engine *)button->engine)->canvas,
				button->position.x, button->position.y,
				button->position.width, button->position.height,
				button->background);
	}
	else {
		draw2d_fillRectangle(&((Engine *)button->engine)->canvas,
				&button->backgroundColor,
				button->position.x, button->position.y,
				button->position.width, button->position.height);
	}

	// render state texture

	if(button->textures[button->state] != NULL) {
		draw2d_texture(&((Engine *)button->engine)->canvas,
				button->position.x, button->position.y,
				button->position.width, button->position.height,
				button->textures[button->state]);
	}

	// render text

	if(button->text != NULL) {
		draw2d_colorFont(&((Engine *)button->engine)->canvas,
				button->fontInfo.x, button->fontInfo.y,
				button->text, &button->fontColor, button->fontInfo.font);
	}
}

