/*
 * checkbox.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * A GUI widget for handling checkboxes.
 *
 * Written by Tom Brecht.
 */

// preprocessor directives

#include "core/core.h"
#include "asgard/asgard.h"
#define _ASGARD_GUI_COMPONENT
#include "asgard/gui/checkbox.h"


// define gui checkbox public functions

void guiCheckBox_init(GuiCheckBox *checkbox, char *name, int x, int y, 
		int width, int height, void *engine)
{
	if((checkbox == NULL) || (engine == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	memset(checkbox, 0, (sizeof(GuiCheckBox)));

	checkbox->isVisible = atrue;
	checkbox->isChecked = afalse;

	checkbox->name = strdup(name);

	checkbox->position.x = x;
	checkbox->position.y = y;
	checkbox->position.width = width;
	checkbox->position.height = height;

	checkbox->boxImage = NULL;
	checkbox->checkImage = NULL;

	checkbox->engine = engine;
}

void guiCheckBox_free(GuiCheckBox *checkbox)
{
	if(checkbox == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	/*
	 * TODO: this function
	 */
}

char *guiCheckBox_getName(GuiCheckBox *checkbox)
{
	if(checkbox == NULL) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	return checkbox->name;
}

void guiCheckBox_setName(GuiCheckBox *checkbox, char *name)
{
	if(checkbox == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	if(checkbox->name != NULL) {
		free(checkbox->name);
	}

	checkbox->name = strdup(name);
}

int guiCheckBox_getX(GuiCheckBox *checkbox)
{
	if(checkbox == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return checkbox->position.x;
}

int guiCheckBox_getY(GuiCheckBox *checkbox)
{
	if(checkbox == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return checkbox->position.y;
}

GuiPosition *guiCheckBox_getPosition(GuiCheckBox *checkbox)
{
	if(checkbox == NULL) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	return &(checkbox->position);
}

void guiCheckBox_setPosition(GuiCheckBox *checkbox, int x, int y)
{
	if(checkbox == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	checkbox->position.x = x;
	checkbox->position.y = y;
}

int guiCheckBox_getWidth(GuiCheckBox *checkbox)
{
	if(checkbox == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return checkbox->position.width;
}

int guiCheckBox_getHeight(GuiCheckBox *checkbox)
{
	if(checkbox == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return checkbox->position.height;
}

void guiCheckBox_resize(GuiCheckBox *checkbox, int width, int height)
{
	if(checkbox == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	checkbox->position.width = width;
	checkbox->position.height = height;
}

aboolean guiCheckBox_isVisible(GuiCheckBox *checkbox)
{
	if(checkbox == NULL) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	return checkbox->isVisible;
}

void guiCheckBox_setVisibility(GuiCheckBox *checkbox, aboolean isVisible)
{
	if(checkbox == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	checkbox->isVisible = isVisible;
}

aboolean guiCheckBox_isChecked(GuiCheckBox *checkbox)
{
	if(checkbox == NULL) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	return checkbox->isChecked;
}

void guiCheckBox_setChecked(GuiCheckBox *checkbox, aboolean isChecked)
{
	if(checkbox == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	checkbox->isChecked = isChecked;
}

Texture *guiCheckBox_getBoxImage(GuiCheckBox *checkbox)
{
	if(checkbox == NULL) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	return checkbox->boxImage;
}

void guiCheckBox_setBoxImage(GuiCheckBox *checkbox, Texture *texture)
{
	if((checkbox == NULL) ||
			(texture == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	checkbox->boxImage = texture;
}

Texture *guiCheckBox_getCheckImage(GuiCheckBox *checkbox)
{
	if(checkbox == NULL) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	return checkbox->checkImage;
}

void guiCheckBox_setCheckImage(GuiCheckBox *checkbox, Texture *texture)
{
	if(checkbox == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	checkbox->checkImage = texture;
}

void guiCheckBox_setImages(GuiCheckBox *checkbox, Texture *boxTexture,
		Texture *checkTexture)
{
	if((checkbox == NULL) ||
			(boxTexture == NULL) ||
			(checkTexture == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	checkbox->boxImage = boxTexture;
	checkbox->checkImage = checkTexture;
}

aboolean guiCheckBox_hasFocus(GuiCheckBox *checkbox, int mouseX, int mouseY)
{
	if(checkbox == NULL) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	if(!checkbox->isVisible) {
		return afalse;
	}

	if((mouseX >= checkbox->position.x) &&
			(mouseX <= (checkbox->position.x + checkbox->position.width)) &&
			(mouseY >= checkbox->position.y) &&
			(mouseY <= (checkbox->position.y + checkbox->position.height))) {
		return atrue;
	}

	return afalse;
}

void guiCheckBox_deFocus(GuiCheckBox *checkbox)
{
	if(checkbox == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	/*
	 * TODO: this function
	 */
}

aboolean guiCheckBox_wasClicked(GuiCheckBox *checkbox, int mouseX, int mouseY)
{
	aboolean result = afalse;

	if(checkbox == NULL) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	if(!checkbox->isVisible) {
		return afalse;
	}

	if((mouseX >= checkbox->position.x) &&
			(mouseX <= (checkbox->position.x + checkbox->position.width)) &&
			(mouseY >= checkbox->position.y) &&
			(mouseY <= (checkbox->position.y + checkbox->position.height))) {
		result = atrue;
	}

	if(result) {
		if(checkbox->isChecked) {
			checkbox->isChecked = afalse;
		}
		else {
			checkbox->isChecked = atrue;
		}
	}

	return result;
}

void guiCheckBox_keyPressed(GuiCheckBox *checkbox, AsgardKeyboardEvent *event)
{
	if(checkbox == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	/*
	 * TODO: this function
	 */
}

void guiCheckBox_render2d(GuiCheckBox *checkbox)
{
	if(checkbox == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	if(!checkbox->isVisible) {
		return;
	}

	// render box

	if(checkbox->boxImage != NULL) {
		draw2d_texture(&((Engine *)checkbox->engine)->canvas,
				checkbox->position.x, checkbox->position.y,
				checkbox->position.width, checkbox->position.height,
				checkbox->boxImage);
	}

	// render check

	if((checkbox->isChecked == atrue) &&
			(checkbox->checkImage != NULL)) {
		draw2d_texture(&((Engine *)checkbox->engine)->canvas,
				checkbox->position.x, checkbox->position.y,
				checkbox->position.width, checkbox->position.height,
				checkbox->checkImage);
	}
}

