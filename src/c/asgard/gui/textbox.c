/*
 * textbox.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * A GUI widget for handling textboxes.
 *
 * Written by Tom Brecht.
 */

// preprocessor directives

#include "core/core.h"
#include "asgard/asgard.h"
#define _ASGARD_GUI_COMPONENT
#include "asgard/gui/textbox.h"


// define gui textbox private constants

#define GUI_TEXTBOX_DEFAULT_HISTORY_SIZE				32

#define	GUI_TEXTBOX_DEFAULT_MAX_CHARS					64

#define GUI_TEXTBOX_BLINK_COUNT							64

#define GUI_TEXTBOX_PASSWORD_MASK						'*'


// declare gui textbox private functions

static void calculateFontCoordinates(GuiTextbox *textbox);

static void commitBuffer(GuiTextbox *textbox);

static char *getContentsBuffer(GuiTextbox *textbox);

static void calculateVisualIndices(GuiTextbox *textbox);


// define gui textbox private functions

static void calculateFontCoordinates(GuiTextbox *textbox)
{
	if(textbox == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	textbox->fontX = (textbox->position.x + 2);
	textbox->fontY = (textbox->position.y +
			(int)((double)textbox->position.height * 0.35));
}

static void commitBuffer(GuiTextbox *textbox)
{
	char *newBuffer = NULL;

	if(textbox == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	textbox->submitted = (char *)arraylist_get(textbox->contents,
			textbox->historyIndex);

	if(textbox->historyIndex != (arraylist_length(textbox->contents) - 1)) {
		arraylist_set(textbox->contents,
			(arraylist_length(textbox->contents) - 1),
			arraylist_get(textbox->contents, textbox->historyIndex));
	}

	newBuffer = (char *)malloc(sizeof(char) * (textbox->maxChars + 1));

	arraylist_add(textbox->contents, newBuffer);

	textbox->historyIndex = (arraylist_length(textbox->contents) - 1);
	textbox->bufferIndex = 0;

	if(arraylist_length(textbox->contents) > textbox->historySize) {
		arraylist_delPosition(textbox->contents, 0);
		textbox->historyIndex = (arraylist_length(textbox->contents) - 1);
	}
}

static char *getContentsBuffer(GuiTextbox *textbox)
{
	if(textbox == NULL) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	return (char *)arraylist_get(textbox->contents, textbox->historyIndex);
}

static void calculateVisualIndices(GuiTextbox *textbox)
{
	int length = 0;
	int stringWidth = 0;
	char *buffer = NULL;

	if(textbox == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	buffer = getContentsBuffer(textbox);

	if(buffer == NULL) {
		textbox->visualBeginning = 0;
		textbox->visualEnd = 0;
		return;
	}

	buffer = strdup(buffer);

	if(textbox->isMasked) {
		length = strlen(buffer);
		free(buffer);

		buffer = (char *)malloc(sizeof(char) * (length + 1));
		memset(buffer, (int)GUI_TEXTBOX_PASSWORD_MASK, length);
	}

	length = strlen(buffer);
	stringWidth = length * textbox->fontInfo.glyphWidth;

	if(stringWidth < textbox->position.width) {
		textbox->visualBeginning = 0;
		textbox->visualEnd = length;
		return;
	}

	textbox->visualEnd = textbox->bufferIndex;
	for(textbox->visualBeginning = 0;
			(((textbox->visualEnd - textbox->visualBeginning) *
			textbox->fontInfo.glyphWidth) > textbox->position.width);
			textbox->visualBeginning += 1) {
		// iterate
	}

	if(textbox->visualBeginning != 0) {
		return;
	}

	for(textbox->visualEnd = textbox->bufferIndex;
			(((textbox->visualEnd - textbox->visualBeginning) *
			textbox->fontInfo.glyphWidth) < textbox->position.width);
			textbox->visualEnd += 1) {
		// iterate
	}

	textbox->visualEnd -= 1;

	return;
}


// define gui textbox public functions

void guiTextbox_init(GuiTextbox *textbox, char *name, int x, int y, int width,
		int height, void *engine)
{
	char *text = NULL;

	if((textbox == NULL) || (engine == NULL)){
		DISPLAY_INVALID_ARGS;
		return;
	}

	memset(textbox, 0, (sizeof(GuiTextbox)));

	textbox->isVisible = atrue;
	textbox->wasClicked = afalse;
	textbox->isMasked = afalse;
	textbox->state = GUI_TEXTBOX_STATE_NORMAL;
	textbox->maxChars = GUI_TEXTBOX_DEFAULT_MAX_CHARS;
	textbox->fontX = 0;
	textbox->fontY = 0;
	textbox->historySize = GUI_TEXTBOX_DEFAULT_HISTORY_SIZE;
	textbox->historyIndex = 0;
	textbox->visualBeginning = 0;
	textbox->visualEnd = 0;
	textbox->blink = 0;
	textbox->submitted = NULL;

	text = (char *)malloc(sizeof(char) * (textbox->maxChars + 1));

	textbox->contents = (ArrayList *)malloc(sizeof(ArrayList));

	arraylist_init(textbox->contents);
	arraylist_add(textbox->contents, text);

	textbox->name = strdup(name);

	textbox->fontColor.r = CGI_COLOR_BLACK_RED;
	textbox->fontColor.g = CGI_COLOR_BLACK_GREEN;
	textbox->fontColor.b = CGI_COLOR_BLACK_BLUE;
	textbox->fontColor.a = CGI_COLOR_BLACK_ALPHA;

	textbox->backgroundColor.r = CGI_COLOR_LIGHTGREY_RED;
	textbox->backgroundColor.g = CGI_COLOR_LIGHTGREY_GREEN;
	textbox->backgroundColor.b = CGI_COLOR_LIGHTGREY_BLUE;
	textbox->backgroundColor.a = CGI_COLOR_LIGHTGREY_ALPHA;

	textbox->position.x = x;
	textbox->position.y = y;
	textbox->position.width = width;
	textbox->position.height = height;

	textbox->fontInfo.x = 0;
	textbox->fontInfo.y = 0;
	textbox->fontInfo.glyphWidth = GUI_DEFAULT_FONT_GLYPH_WIDTH;
	textbox->fontInfo.glyphHeight = GUI_DEFAULT_FONT_GLYPH_HEIGHT;
	textbox->fontInfo.font = NULL;

	textbox->background = NULL;
	textbox->textures[GUI_TEXTBOX_STATE_NORMAL] = NULL;
	textbox->textures[GUI_TEXTBOX_STATE_FOCUSED] = NULL;

	textbox->engine = engine;

	calculateFontCoordinates(textbox);
}

void guiTextbox_free(GuiTextbox *textbox)
{
	if(textbox == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	arraylist_free(textbox->contents);
}

aboolean guiTextbox_isNormal(GuiTextbox *textbox)
{
	if(textbox == NULL) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	if(textbox->state == GUI_TEXTBOX_STATE_NORMAL) {
		return atrue;
	}

	return afalse;
}

aboolean guiTextbox_isFocused(GuiTextbox *textbox)
{
	if(textbox == NULL) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	if(textbox->state == GUI_TEXTBOX_STATE_FOCUSED) {
		return atrue;
	}

	return afalse;
}

int guiTextbox_getState(GuiTextbox *textbox)
{
	if(textbox == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return textbox->state;
}

char *guiTextbox_getName(GuiTextbox *textbox)
{
	if(textbox == NULL) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	return textbox->name;
}

void guiTextbox_setName(GuiTextbox *textbox, char *name)
{
	if(textbox == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	if(textbox->name != NULL) {
		free(textbox->name);
	}

	textbox->name = strdup(name);
}

int guiTextbox_getX(GuiTextbox *textbox)
{
	if(textbox == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return textbox->position.x;
}

int guiTextbox_getY(GuiTextbox *textbox)
{
	if(textbox == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return textbox->position.y;
}

GuiPosition *guiTextbox_getPosition(GuiTextbox *textbox)
{
	if(textbox == NULL) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	return &(textbox->position);
}

void guiTextbox_setPosition(GuiTextbox *textbox, int x, int y)
{
	if(textbox == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	textbox->position.x = x;
	textbox->position.y = y;
}

int guiTextbox_getWidth(GuiTextbox *textbox)
{
	if(textbox == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return textbox->position.width;
}

int guiTextbox_getHeight(GuiTextbox *textbox)
{
	if(textbox == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return textbox->position.height;
}

void guiTextbox_resize(GuiTextbox *textbox, int width, int height)
{
	if(textbox == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	textbox->position.width = width;
	textbox->position.height = height;
}

aboolean guiTextbox_isVisible(GuiTextbox *textbox)
{
	if(textbox == NULL) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	return textbox->isVisible;
}

void guiTextbox_setVisibility(GuiTextbox *textbox, aboolean isVisible)
{
	if(textbox == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	textbox->isVisible = isVisible;
}

aboolean guiTextbox_isMasked(GuiTextbox *textbox)
{
	if(textbox == NULL) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	return textbox->isMasked;
}

void guiTextbox_setMasking(GuiTextbox *textbox, aboolean isMasked)
{
	if(textbox == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	textbox->isMasked = isMasked;
}

Font *guiTextbox_getFont(GuiTextbox *textbox)
{
	if(textbox == NULL) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	return textbox->fontInfo.font;
}

void guiTextbox_setFont(GuiTextbox *textbox, Font *font)
{
	if(textbox == NULL || font == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	textbox->fontInfo.font = font;
}

Color *guiTextbox_getFontColor(GuiTextbox *textbox)
{
	if(textbox == NULL) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	return &(textbox->fontColor);
}

void guiTextbox_setFontColor(GuiTextbox *textbox, Color *color)
{
	if(textbox == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	textbox->fontColor.r = color->r;
	textbox->fontColor.g = color->g;
	textbox->fontColor.b = color->b;
	textbox->fontColor.a = color->a;
}

Color *guiTextbox_getBackgroundColor(GuiTextbox *textbox)
{
	if(textbox == NULL) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	return &(textbox->backgroundColor);
}

void guiTextbox_setBackgroundColor(GuiTextbox *textbox, Color *color)
{
	if(textbox == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	textbox->backgroundColor.r = color->r;
	textbox->backgroundColor.g = color->g;
	textbox->backgroundColor.b = color->b;
	textbox->backgroundColor.a = color->a;
}

void guiTextbox_setColorScheme(GuiTextbox *textbox, Color *backgroundColor,
		Color *displayColor)
{
	if(textbox == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	textbox->backgroundColor.r = backgroundColor->r;
	textbox->backgroundColor.g = backgroundColor->g;
	textbox->backgroundColor.b = backgroundColor->b;
	textbox->backgroundColor.a = backgroundColor->a;

	textbox->fontColor.r = displayColor->r;
	textbox->fontColor.g = displayColor->g;
	textbox->fontColor.b = displayColor->b;
	textbox->fontColor.a = displayColor->a;
}

Texture *guiTextbox_getTexture(GuiTextbox *textbox, int textboxState)
{
	if((textbox == NULL) ||
			((textboxState != GUI_BUTTON_STATE_NORMAL) &&
			 (textboxState != GUI_BUTTON_STATE_FOCUSED))) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	return textbox->textures[textboxState];
}

void guiTextbox_setTexture(GuiTextbox *textbox, int textboxState,
		Texture *texture)
{
	if((textbox == NULL) ||
			((textboxState != GUI_BUTTON_STATE_NORMAL) &&
			 (textboxState != GUI_BUTTON_STATE_FOCUSED)) ||
			(texture == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	textbox->textures[textboxState] = texture;
}

Texture *guiTextbox_getBackgroundTexture(GuiTextbox *textbox)
{
	if(textbox == NULL) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	return textbox->background;
}

void guiTextbox_setBackgroundTexture(GuiTextbox *textbox, Texture *texture)
{
	if((textbox == NULL) || (texture == NULL)){
		DISPLAY_INVALID_ARGS;
		return;
	}

	textbox->background = texture;
}

int guiTextbox_getMaxChars(GuiTextbox *textbox)
{
	if(textbox == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return textbox->maxChars;
}

void guiTextbox_setMaxChars(GuiTextbox *textbox, int maxChars)
{
	if(textbox == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	textbox->maxChars = maxChars;
}

aboolean guiTextbox_hasSubmitted(GuiTextbox *textbox)
{
	if(textbox == NULL) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	if(textbox->submitted != NULL) {
		return atrue;
	}

	return afalse;
}

void guiTextbox_setContents(GuiTextbox *textbox, char *text)
{
	if(textbox == NULL || text == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	arraylist_add(textbox->contents, text);
	textbox->historyIndex = arraylist_length(textbox->contents);
	textbox->bufferIndex = 0;
}

char *guiTextbox_getContents(GuiTextbox *textbox)
{
	if(textbox == NULL) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	return arraylist_get(textbox->contents, textbox->historyIndex);
}

char *guiTextbox_getSubmittedContents(GuiTextbox *textbox)
{
	char *result = NULL;

	if(textbox == NULL) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	if(textbox->submitted != NULL) {
		result = strdup(textbox->submitted);
		textbox->submitted = NULL;
	}

	return result;
}

aboolean guiTextbox_hasFocus(GuiTextbox *textbox, int mouseX, int mouseY)
{
	aboolean result = afalse;

	if(textbox == NULL) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	if(!textbox->isVisible) {
		textbox->state = GUI_TEXTBOX_STATE_NORMAL;
		return afalse;
	}

	if((mouseX >= textbox->position.x) &&
			(mouseX <= (textbox->position.x + textbox->position.width)) &&
			(mouseY >= textbox->position.y) &&
			(mouseY <= (textbox->position.y + textbox->position.height))) {
		result = atrue;
	}

	if(result) {
		textbox->state = GUI_TEXTBOX_STATE_FOCUSED;
	}
	else {
		textbox->state = GUI_TEXTBOX_STATE_NORMAL;
	}

	return result;
}

void guiTextbox_deFocus(GuiTextbox *textbox)
{
	if(textbox == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	textbox->state = GUI_TEXTBOX_STATE_NORMAL;
}

aboolean guiTextbox_wasClicked(GuiTextbox *textbox, int mouseX, int mouseY)
{
	int ii = 0;
	int cursorX = 0;
	int bufferLength = 0;
	char *buffer = NULL;

	if(textbox == NULL) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	if(!textbox->isVisible) {
		textbox->wasClicked = afalse;
		return afalse;
	}

	if(!((mouseX >= textbox->position.x) &&
			(mouseX <= (textbox->position.x + textbox->position.width)) &&
			(mouseY >= textbox->position.y) &&
			(mouseY <= (textbox->position.y + textbox->position.height)))) {
		textbox->wasClicked = afalse;
		return afalse;
	}

	textbox->wasClicked = atrue;

	cursorX = textbox->position.x;
	buffer = getContentsBuffer(textbox);
	bufferLength = strlen(buffer);

	for(ii = 0; ii < bufferLength; ii++) {
		cursorX += textbox->fontInfo.glyphWidth;
		if(cursorX > mouseX) {
			textbox->bufferIndex = (ii + 1);
			return atrue;
		}
	}

	textbox->bufferIndex = bufferLength;

	return atrue;
}

void guiTextbox_keyPressed(GuiTextbox *textbox, AsgardKeyboardEvent *event)
{
	int ii = 0;
	char value = (char)0;
	char *buffer = NULL;
	int bufferLength = 0;

	if(textbox == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	if(!textbox->wasClicked) {
		return;
	}

	if(event->eventType != EVENT_KEYBOARD_RELEASED) {
		return;
	}

	buffer = getContentsBuffer(textbox);

	if(buffer == NULL) {
		buffer = (char *)malloc(sizeof(char) * (textbox->maxChars + 1));
	}

	bufferLength = strlen(buffer);

	switch(event->keyCode) {
		case ASGARD_KEY_BACKSPACE:
			if((textbox->bufferIndex > 0) &&
					(bufferLength >= textbox->bufferIndex)) {
				for(ii = textbox->bufferIndex; ii < bufferLength; ii++) {
					buffer[(ii - 1)] = buffer[ii];
				}
				buffer[(bufferLength - 1)] = (char)0;
				textbox->bufferIndex -= 1;
			}
			return;

		case ASGARD_KEY_DELETE:
			if(textbox->bufferIndex < bufferLength) {
				for(ii = (textbox->bufferIndex + 1); ii < bufferLength; ii++) {
					buffer[(ii - 1)] = buffer[ii];
				}
				buffer[(bufferLength - 1)] = (char)0;
			}
			return;

		case ASGARD_KEY_RETURN:
			if(bufferLength > 0) {
				commitBuffer(textbox);
			}
			return;

		case ASGARD_KEY_ESCAPE:
			textbox->wasClicked = afalse;
			return;

		case ASGARD_KEY_LEFT:
			if(textbox->bufferIndex > 0) {
				textbox->bufferIndex -= 1;
			}
			return;

		case ASGARD_KEY_RIGHT:
			if(textbox->bufferIndex < bufferLength) {
				textbox->bufferIndex += 1;
			}
			return;

		case ASGARD_KEY_UP:
			if(textbox->historyIndex > 0) {
				textbox->historyIndex -= 1;
			}
			return;

		case ASGARD_KEY_DOWN:
			if(textbox->historyIndex <
					(arraylist_length(textbox->contents) - 1)) {
				textbox->historyIndex += 1;
			}
			return;

		case ASGARD_KEY_HOME:
			textbox->bufferIndex = 0;
			return;

		case ASGARD_KEY_END:
			textbox->bufferIndex = bufferLength;
			return;
	}

	if(bufferLength < textbox->maxChars) {
		value = event->key;
		if(((int)value > 31) && ((int)value < 127)) {
			if(textbox->bufferIndex < bufferLength) {
				if(bufferLength == textbox->maxChars) {
					bufferLength--;
				}

				for(ii = bufferLength;
						ii > textbox->bufferIndex; ii--) {
					buffer[ii] = buffer[(ii - 1)];
				}
			}
			buffer[textbox->bufferIndex] = value;
			textbox->bufferIndex += 1;
		}
		return;
	}
}

void guiTextbox_render2d(GuiTextbox *textbox)
{
	int ii = 0;
	char *password = NULL;
	char *temp = NULL;
	char *text = NULL;

	if(textbox == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	if(!textbox->isVisible) {
		return;
	}

	// setup render space

	draw2d_fillRectangle(&((Engine *)textbox->engine)->canvas,
				&textbox->backgroundColor,
				textbox->position.x, textbox->position.y,
				textbox->position.width, textbox->position.height);


	// render text

	text = getContentsBuffer(textbox);

	if(text == NULL) {
		return;
	}

	calculateVisualIndices(textbox);

	if((textbox->visualEnd - textbox->visualBeginning) <= 0) {
		return;
	}

	if(textbox->isMasked) {
		password = (char *)malloc(sizeof(char) *
				((textbox->visualEnd - textbox->visualBeginning) + 1));
		memset(password, (int)GUI_TEXTBOX_PASSWORD_MASK,
				(textbox->visualEnd - textbox->visualBeginning));

		draw2d_colorFont(&((Engine *)textbox->engine)->canvas,
				textbox->fontX, textbox->fontY,
				password, &textbox->fontColor, textbox->fontInfo.font);
	}
	else {
		temp = strdup(text);

		text = (char *)malloc(sizeof(char) *
				((textbox->visualEnd - textbox->visualBeginning) + 1));

		for(ii = 0; ii < (textbox->visualEnd - textbox->visualBeginning);
				ii++) {
			text[ii] = temp[(textbox->visualBeginning + ii)];
		}

		draw2d_colorFont(&((Engine *)textbox->engine)->canvas,
				textbox->fontX, textbox->fontY,
				text, &textbox->fontColor, textbox->fontInfo.font);
	}

	// render cursor

	textbox->blink += 1;

	if(textbox->blink < (GUI_TEXTBOX_BLINK_COUNT >> 1)) {
		draw2d_fillRectangle(&((Engine *)textbox->engine)->canvas,
					&textbox->fontColor,
					(textbox->bufferIndex * textbox->fontInfo.glyphWidth),
					(textbox->fontY - 2), 2,
					(textbox->fontInfo.glyphHeight * 2));
	}

	if(textbox->blink == GUI_TEXTBOX_BLINK_COUNT) {
		textbox->blink = 0;
	}
}

