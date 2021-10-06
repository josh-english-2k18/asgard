/* 
 * checkbox.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * A GUI widget for handling checkboxes, header file.
 *
 * Written by Tom Brecht.
 */

#if !defined(_ASGARD_GUI_CHECKBOX_H)

#define _ASGARD_GUI_CHECKBOX_H

#if !defined(_ASGARD_H) && !defined(_ASGARD_GUI_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _ASGARD_H || _ASGARD_GUI_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// define gui checkbox public data types

typedef struct _GuiCheckBox {
	aboolean isVisible;
	aboolean isChecked;
	char *name;
	GuiPosition position;
	Texture *boxImage;
	Texture *checkImage;
	void *engine;
} GuiCheckBox;


// declare gui checkbox public functions

void guiCheckBox_init(GuiCheckBox *checkbox, char *name, int x, int y, 
		int width, int height, void *engine);

void guiCheckBox_free(GuiCheckBox *checkbox);

char *guiCheckBox_getName(GuiCheckBox *checkbox);

void guiCheckBox_setName(GuiCheckBox *checkbox, char *name);

int guiCheckBox_getX(GuiCheckBox *checkbox);

int guiCheckBox_getY(GuiCheckBox *checkbox);

GuiPosition *guiCheckBox_getPosition(GuiCheckBox *checkbox);

void guiCheckBox_setPosition(GuiCheckBox *checkbox, int x, int y);

int guiCheckBox_getWidth(GuiCheckBox *checkbox);

int guiCheckBox_getHeight(GuiCheckBox *checkbox);

void guiCheckBox_resize(GuiCheckBox *checkbox, int width, int height);

aboolean guiCheckBox_isVisible(GuiCheckBox *checkbox);

void guiCheckBox_setVisibility(GuiCheckBox *checkbox, aboolean isVisible);

aboolean guiCheckBox_isChecked(GuiCheckBox *checkbox);

void guiCheckBox_setChecked(GuiCheckBox *checkbox, aboolean isVisible);

Texture *guiCheckBox_getBoxImage(GuiCheckBox *checkbox);

void guiCheckBox_setBoxImage(GuiCheckBox *checkbox, Texture *texture);

Texture *guiCheckBox_getCheckImage(GuiCheckBox *checkbox);

void guiCheckBox_setCheckImage(GuiCheckBox *checkbox, Texture *texture);

void guiCheckBox_setImages(GuiCheckBox *checkbox, Texture *boxTexture,
		Texture *checkTexture);

aboolean guiCheckBox_hasFocus(GuiCheckBox *checkbox, int mouseX, int mouseY);

void guiCheckBox_deFocus(GuiCheckBox *checkbox);

aboolean guiCheckBox_wasClicked(GuiCheckBox *checkbox, int mouseX, int mouseY);

void guiCheckBox_keyPressed(GuiCheckBox *checkbox, AsgardKeyboardEvent *event);

void guiCheckBox_render2d(GuiCheckBox *checkbox);


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _ASGARD_GUI_CHECKBOX_H

