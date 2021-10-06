/*
 * autoscroll.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * A GUI widget for handling autoscroll.
 *
 * Written by Tom Brecht.
 */

// preprocessor directives

#include "core/core.h"
#include "asgard/asgard.h"
#define _ASGARD_GUI_COMPONENT
#include "asgard/gui/autoscroll.h"


// declare gui autoscroll private functions

static void guiAutoscrollEntry_init(GuiAutoscrollEntry *entry, int type,
		int width, int height, void *object);


// define gui autoscroll private functions

static void guiAutoscrollEntry_init(GuiAutoscrollEntry *entry, int type,
		int width, int height, void *object)
{
	if(entry == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	memset(entry, 0, (sizeof(GuiAutoscrollEntry)));

	entry->type = type;
	entry->width = width;
	entry->height = height;
	entry->object = object;

	entry->color.r = CGI_COLOR_BLACK_RED;
	entry->color.g = CGI_COLOR_BLACK_GREEN;
	entry->color.b = CGI_COLOR_BLACK_BLUE;
	entry->color.a = CGI_COLOR_BLACK_ALPHA;
}


// define gui autoscroll public functions

void guiAutoscroll_init(GuiAutoscroll *autoscroll, char *name, int x, int y,
		int width, int height, void *engine)
{
	if((autoscroll == NULL) || (engine == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	memset(autoscroll, 0, (sizeof(GuiAutoscroll)));
	
	autoscroll->isVisible = atrue;
	autoscroll->isRunning = atrue;
	autoscroll->hasOutline = afalse;
	autoscroll->scrollTicks = 0;
	autoscroll->scrollCount = 0;

	autoscroll->name = strdup(name);

	autoscroll->fontColor.r = CGI_COLOR_BLACK_RED;
	autoscroll->fontColor.g = CGI_COLOR_BLACK_GREEN;
	autoscroll->fontColor.b = CGI_COLOR_BLACK_BLUE;
	autoscroll->fontColor.a = CGI_COLOR_BLACK_ALPHA;

	autoscroll->backgroundColor.r = CGI_COLOR_LIGHTGREY_RED;
	autoscroll->backgroundColor.g = CGI_COLOR_LIGHTGREY_GREEN;
	autoscroll->backgroundColor.b = CGI_COLOR_LIGHTGREY_BLUE;
	autoscroll->backgroundColor.a = CGI_COLOR_LIGHTGREY_ALPHA;

	autoscroll->outlineColor.r = CGI_COLOR_BLACK_RED;
	autoscroll->outlineColor.g = CGI_COLOR_BLACK_GREEN;
	autoscroll->outlineColor.b = CGI_COLOR_BLACK_BLUE;
	autoscroll->outlineColor.a = CGI_COLOR_BLACK_ALPHA;

	autoscroll->position.x = x;
	autoscroll->position.y = y;
	autoscroll->position.width = width;
	autoscroll->position.height = height;

	autoscroll->fontInfo.x = 0;
	autoscroll->fontInfo.y = 0;
	autoscroll->fontInfo.glyphWidth = GUI_DEFAULT_FONT_GLYPH_WIDTH;
	autoscroll->fontInfo.glyphHeight = GUI_DEFAULT_FONT_GLYPH_HEIGHT;
	autoscroll->fontInfo.font = NULL;

	autoscroll->background = NULL;

	memset(autoscroll->entries, 0, 
		(GUI_AUTOSCROLL_MAX_ENTRIES * (sizeof(GuiAutoscrollEntry))));

	autoscroll->engine = engine;
}

void guiAutoscroll_free(GuiAutoscroll *autoscroll)
{
	if(autoscroll == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	/*
	 * TODO: this function
	 */
}

char *guiAutoscroll_getName(GuiAutoscroll *autoscroll)
{
	if(autoscroll == NULL) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	return autoscroll->name;
}

void guiAutoscroll_setName(GuiAutoscroll *autoscroll, char *name)
{
	if(autoscroll == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	if(autoscroll->name != NULL) {
		free(autoscroll->name);
	}

	autoscroll->name = strdup(name);
}

int guiAutoscroll_getX(GuiAutoscroll *autoscroll)
{
	if(autoscroll == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return autoscroll->position.x;
}

int guiAutoscroll_getY(GuiAutoscroll *autoscroll)
{
	if(autoscroll == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return autoscroll->position.y;
}

GuiPosition *guiAutoscroll_getPosition(GuiAutoscroll *autoscroll)
{
	if(autoscroll == NULL) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	return &(autoscroll->position);
}

void guiAutoscroll_setPosition(GuiAutoscroll *autoscroll, int x, int y)
{
	if(autoscroll == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	autoscroll->position.x = x;
	autoscroll->position.y = y;
}

int guiAutoscroll_getWidth(GuiAutoscroll *autoscroll)
{
	if(autoscroll == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return autoscroll->position.width;
}

int guiAutoscroll_getHeight(GuiAutoscroll *autoscroll)
{
	if(autoscroll == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return autoscroll->position.height;
}

void guiAutoscroll_resize(GuiAutoscroll *autoscroll, int width, int height)
{
	if(autoscroll == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	autoscroll->position.width = width;
	autoscroll->position.height = height;
}

aboolean guiAutoscroll_isVisible(GuiAutoscroll *autoscroll)
{
	if(autoscroll == NULL) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	return autoscroll->isVisible;
}

void guiAutoscroll_setVisibility(GuiAutoscroll *autoscroll, aboolean isVisible)
{
	if(autoscroll == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	autoscroll->isVisible = isVisible;
}

aboolean guiAutoscroll_isRunning(GuiAutoscroll *autoscroll)
{
	if(autoscroll == NULL) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	return autoscroll->isRunning;
}

aboolean guiAutoscroll_hasOutline(GuiAutoscroll *autoscroll)
{
	if(autoscroll == NULL) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	return autoscroll->hasOutline;
}

void guiAutoscroll_setHasOutline(GuiAutoscroll *autoscroll, 
		aboolean hasOutline)
{
	if(autoscroll == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	autoscroll->hasOutline = hasOutline;
}

Color *guiAutoscroll_getFontColor(GuiAutoscroll *autoscroll)
{
	if(autoscroll == NULL) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	return &(autoscroll->fontColor);
}

void guiAutoscroll_setFontColor(GuiAutoscroll *autoscroll, Color *color)
{
	if((autoscroll == NULL) || (color == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	autoscroll->fontColor.r = color->r;
	autoscroll->fontColor.g = color->g;
	autoscroll->fontColor.b = color->b;
	autoscroll->fontColor.a = color->a;
}

Color *guiAutoscroll_getBackgroundColor(GuiAutoscroll *autoscroll)
{
	if(autoscroll == NULL) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	return &(autoscroll->backgroundColor);
}

void guiAutoscroll_setBackgroundColor(GuiAutoscroll *autoscroll, Color *color)
{
	if((autoscroll == NULL) || (color == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	autoscroll->backgroundColor.r = color->r;
	autoscroll->backgroundColor.g = color->g;
	autoscroll->backgroundColor.b = color->b;
	autoscroll->backgroundColor.a = color->a;
}

Color *guiAutoscroll_getOutlineColor(GuiAutoscroll *autoscroll)
{
	if(autoscroll == NULL) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	return &(autoscroll->outlineColor);
}

void guiAutoscroll_setOutlineColor(GuiAutoscroll *autoscroll, Color *color)
{
	if((autoscroll == NULL)  || (color == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	autoscroll->outlineColor.r = color->r;
	autoscroll->outlineColor.g = color->g;
	autoscroll->outlineColor.b = color->b;
	autoscroll->outlineColor.a = color->a;
}

Texture *guiAutoscroll_getBackgroundTexture(GuiAutoscroll *autoscroll)
{
	if(autoscroll == NULL) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	return autoscroll->background;
}

void guiAutoscroll_setBackgroundTexture(GuiAutoscroll *autoscroll, 
		Texture *texture)
{
	if((autoscroll == NULL) || (texture == NULL)){
		DISPLAY_INVALID_ARGS;
		return;
	}

	autoscroll->background = texture;
}

void guiAutoscroll_addColorText(GuiAutoscroll *autoscroll, Color *color, 
		char *text)
{
	int textLength = 0;

	GuiAutoscrollEntry *entry = NULL;

	if((autoscroll == NULL) || (color == NULL) || (text == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	if(autoscroll->scrollCount >= GUI_AUTOSCROLL_MAX_ENTRIES) {
		return;
	}

	textLength = strlen(text);

	entry = (GuiAutoscrollEntry *)malloc(sizeof(GuiAutoscrollEntry));

	guiAutoscrollEntry_init(entry, GUI_AUTOSCROLL_ENTRY_TYPE_TEXT, 
			(textLength * autoscroll->fontInfo.glyphWidth),
			autoscroll->fontInfo.glyphHeight, text);

	entry->color.r = color->r;
	entry->color.g = color->g;
	entry->color.b = color->b;
	entry->color.a = color->a;

	autoscroll->entries[autoscroll->scrollCount] = entry;
	autoscroll->scrollCount++;
}

void guiAutoscroll_addText(GuiAutoscroll *autoscroll, char *text)
{
	int textLength = 0;

	GuiAutoscrollEntry *entry = NULL;

	if((autoscroll == NULL) || (text == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	if(autoscroll->scrollCount >= GUI_AUTOSCROLL_MAX_ENTRIES) {
		return;
	}

	textLength = strlen(text);

	entry = (GuiAutoscrollEntry *)malloc(sizeof(GuiAutoscrollEntry));

	guiAutoscrollEntry_init(entry, GUI_AUTOSCROLL_ENTRY_TYPE_TEXT, 
			(textLength * autoscroll->fontInfo.glyphWidth),
			autoscroll->fontInfo.glyphHeight, text);

	autoscroll->entries[autoscroll->scrollCount] = entry;
	autoscroll->scrollCount++;
}

void guiAutoscroll_addImage(GuiAutoscroll *autoscroll, Texture *texture, 
		int width, int height)
{
	GuiAutoscrollEntry *entry = NULL;

	if((autoscroll == NULL) || (texture == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	if(autoscroll->scrollCount >= GUI_AUTOSCROLL_MAX_ENTRIES) {
		return;
	}

	entry = (GuiAutoscrollEntry *)malloc(sizeof(GuiAutoscrollEntry));

	guiAutoscrollEntry_init(entry, GUI_AUTOSCROLL_ENTRY_TYPE_IMAGE, 
			width, height, texture);

	autoscroll->entries[autoscroll->scrollCount] = entry;
	autoscroll->scrollCount++;
}

void guiAutoscroll_addHyperlink(GuiAutoscroll *autoscroll, char *hyperlink)
{
	int textLength = 0;

	GuiAutoscrollEntry *entry = NULL;

	if((autoscroll == NULL) || (hyperlink == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	if(autoscroll->scrollCount >= GUI_AUTOSCROLL_MAX_ENTRIES) {
		return;
	}

	textLength = strlen(hyperlink);

	entry = (GuiAutoscrollEntry *)malloc(sizeof(GuiAutoscrollEntry));

	guiAutoscrollEntry_init(entry, GUI_AUTOSCROLL_ENTRY_TYPE_HYPERLINK, 
			(textLength * autoscroll->fontInfo.glyphWidth),
			autoscroll->fontInfo.glyphHeight, hyperlink);

	autoscroll->entries[autoscroll->scrollCount] = entry;
	autoscroll->scrollCount++;
}

aboolean guiAutoscroll_hasFocus(GuiAutoscroll *autoscroll, int mouseX, 
		int mouseY)
{
	aboolean result = afalse;

	if(autoscroll == NULL) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	if(!autoscroll->isVisible) {
		return afalse;
	}

	if((mouseX >= autoscroll->position.x) &&
			(mouseX <= (autoscroll->position.x + autoscroll->position.width)) &&
			(mouseY >= autoscroll->position.y) &&
			(mouseY <= (autoscroll->position.y + autoscroll->position.height))) {
		result = atrue;
	}

	if(result) {
		// TODO :: hyperlink has focus
	}

	return result;
}

void guiAutoscroll_deFocus(GuiAutoscroll *autoscroll)
{
	if(autoscroll == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	/*
	 * TODO: this function
	 */
}

aboolean guiAutoscroll_wasClicked(GuiAutoscroll *autoscroll, int mouseX, 
		int mouseY)
{
	aboolean result = afalse;

	if(autoscroll == NULL) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	if(!autoscroll->isVisible) {
		return afalse;
	}

	if((mouseX >= autoscroll->position.x) &&
			(mouseX <= (autoscroll->position.x + autoscroll->position.width)) &&
			(mouseY >= autoscroll->position.y) &&
			(mouseY <= (autoscroll->position.y + autoscroll->position.height))) {
		result = atrue;
	}

	if(result) {
		// TODO :: Click hyperlink
	}

	return result;
}

void guiAutoscroll_keyPressed(GuiAutoscroll *autoscroll,
		AsgardKeyboardEvent *event)
{
	if(autoscroll == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	/*
	 * TODO: this function
	 */
}

void guiAutoscroll_reset(GuiAutoscroll *autoscroll)
{
	if(autoscroll == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	autoscroll->isRunning = atrue;
	autoscroll->scrollTicks = 0;
}

void guiAutoscroll_process(GuiAutoscroll *autoscroll)
{
	if(autoscroll == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	autoscroll->scrollTicks++;
}

void guiAutoscroll_render2d(GuiAutoscroll *autoscroll)
{
	int ii = 0;
	int localX = 0;
	int localY = 0;
	int localTicks = 0;
	int localOffset = 0;
	int renderCount = 0;

	char *text;
	Texture *texture;
	char *link;

	if(autoscroll == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	if(!autoscroll->isVisible) {
		return;
	}

	// render scroll area

	if(autoscroll->background == NULL) {
		draw2d_texture(&((Engine *)autoscroll->engine)->canvas,
				autoscroll->position.x, autoscroll->position.y,
				autoscroll->position.width, autoscroll->position.height,
				autoscroll->background);
	}
	else {
		draw2d_fillRectangle(&((Engine *)autoscroll->engine)->canvas,
				&autoscroll->backgroundColor,
				autoscroll->position.x, autoscroll->position.y,
				autoscroll->position.width, autoscroll->position.height);
	}

	if(autoscroll->hasOutline) {
		draw2d_rectangle(&((Engine *)autoscroll->engine)->canvas,
				&autoscroll->outlineColor,
				autoscroll->position.x, autoscroll->position.y,
				autoscroll->position.width, autoscroll->position.height);
	}

	if(!autoscroll->isRunning) {
		return;
	}

	// render scroll entries

	localX = autoscroll->position.x;
	localY = (autoscroll->position.y + 2);
	localOffset = (autoscroll->scrollTicks % autoscroll->position.height);
	for(ii = 0; ii < autoscroll->scrollCount; ii++) {
		if((localY + autoscroll->entries[ii]->height) >=
				(autoscroll->position.y + autoscroll->position.height)) {
			break;
		}

		localTicks += (autoscroll->entries[ii]->height + 4);
		if(localTicks < autoscroll->scrollTicks) {
			if(ii < (autoscroll->scrollCount - 1)) {
				if((localTicks + autoscroll->entries[(ii + 1)]->height + 4) >=
						autoscroll->scrollTicks) {
					localOffset = (autoscroll->scrollTicks - localTicks);
				}
			}
		}
		else if((localY - localOffset) < (autoscroll->position.y + 2)) {
			localY += (autoscroll->entries[ii]->height + 4);
		}
		else {
			if(autoscroll->entries[ii]->type == 
					GUI_AUTOSCROLL_ENTRY_TYPE_TEXT) {
				text = strdup(autoscroll->entries[ii]->object);
				draw2d_colorFont(&((Engine *)autoscroll->engine)->canvas,
					(localX + ((autoscroll->position.width / 2) - 
					autoscroll->entries[ii]->width / 2)), 
					((localY + autoscroll->entries[ii]->height) -
					localOffset),
					text, &autoscroll->entries[ii]->color, 
					autoscroll->fontInfo.font);
			}
			else if(autoscroll->entries[ii]->type == 
					GUI_AUTOSCROLL_ENTRY_TYPE_IMAGE) {
				texture = autoscroll->entries[ii]->object;
				draw2d_texture(&((Engine *)autoscroll->engine)->canvas,
					(localX + ((autoscroll->position.width / 2) - 
					autoscroll->entries[ii]->width / 2)),
					(localY - localOffset),
					autoscroll->entries[ii]->width,
					autoscroll->entries[ii]->height,
					texture);
			}
			else if(autoscroll->entries[ii]->type == 
					GUI_AUTOSCROLL_ENTRY_TYPE_HYPERLINK) {
				link = strdup(autoscroll->entries[ii]->object);
				draw2d_colorFont(&((Engine *)autoscroll->engine)->canvas,
					(localX + ((autoscroll->position.width / 2) - 
					autoscroll->entries[ii]->width / 2)),
					(localY - localOffset),
					link, &autoscroll->entries[ii]->color, 
					autoscroll->fontInfo.font);
			}

			renderCount++;

			localY += (autoscroll->entries[ii]->height + 4);
			if(localY >= 
					(autoscroll->position.y + autoscroll->position.height)) {
				break;
			}
		}
	}

	if(renderCount < 1) {
		autoscroll->isRunning = afalse;
	}
}

