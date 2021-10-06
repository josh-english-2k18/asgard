/*
 * texture.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard Game Engine 2D texture system.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#include "asgard/asgard.h"
#define _ASGARD_ENGINE_COMPONENT
#include "asgard/engine/material.h"


// define material private functions

static void applyMaterialProperty(int type, Color *color)
{
	GLfloat value[4];

	value[0] = (GLfloat)color->r;
	value[1] = (GLfloat)color->g;
	value[2] = (GLfloat)color->b;
	value[3] = (GLfloat)color->a;

	glMaterialfv(GL_FRONT_AND_BACK, type, value);
}

static void applyMaterialProperties(Material *material)
{
	cgi_glMateriali(GL_FRONT_AND_BACK, GL_SHININESS, material->shininess);

	applyMaterialProperty(GL_DIFFUSE, &material->diffuse);
	applyMaterialProperty(GL_AMBIENT, &material->ambient);
	applyMaterialProperty(GL_SPECULAR, &material->specular);
	applyMaterialProperty(GL_EMISSION, &material->emissive);
}

static int getMultiTextureId(int id)
{
	int result = 0;

	switch(id) {
		case 0:
			result = GL_TEXTURE0;
			break;
		case 1:
			result = GL_TEXTURE1;
			break;
		case 2:
			result = GL_TEXTURE2;
			break;
		case 3:
			result = GL_TEXTURE3;
			break;
		case 4:
			result = GL_TEXTURE4;
			break;
		case 5:
			result = GL_TEXTURE5;
			break;
		case 6:
			result = GL_TEXTURE6;
			break;
		case 7:
			result = GL_TEXTURE7;
			break;
		case 8:
			result = GL_TEXTURE8;
			break;
		case 9:
			result = GL_TEXTURE9;
			break;
		case 10:
			result = GL_TEXTURE10;
			break;
		case 11:
			result = GL_TEXTURE11;
			break;
		case 12:
			result = GL_TEXTURE12;
			break;
		case 13:
			result = GL_TEXTURE13;
			break;
		case 14:
			result = GL_TEXTURE14;
			break;
		case 15:
			result = GL_TEXTURE15;
			break;
		case 16:
			result = GL_TEXTURE16;
			break;
		case 17:
			result = GL_TEXTURE17;
			break;
		case 18:
			result = GL_TEXTURE18;
			break;
		case 19:
			result = GL_TEXTURE19;
			break;
		case 20:
			result = GL_TEXTURE20;
			break;
		case 21:
			result = GL_TEXTURE21;
			break;
		case 22:
			result = GL_TEXTURE22;
			break;
		case 23:
			result = GL_TEXTURE23;
			break;
		case 24:
			result = GL_TEXTURE24;
			break;
		case 25:
			result = GL_TEXTURE25;
			break;
		case 26:
			result = GL_TEXTURE26;
			break;
		case 27:
			result = GL_TEXTURE27;
			break;
		case 28:
			result = GL_TEXTURE28;
			break;
		case 29:
			result = GL_TEXTURE29;
			break;
		case 30:
			result = GL_TEXTURE30;
			break;
		case 31:
			result = GL_TEXTURE31;
			break;
		default:
			result = GL_TEXTURE0;
	}

	return result;
}


// define material public functions

int material_init(Material *material, int type, char *name)
{
	if((material == NULL) ||
			((type != MATERIAL_TYPE_TEXTURE) &&
			 (type != MATERIAL_TYPE_MULTI_TEXTURE) &&
			 (type != MATERIAL_TYPE_BUMP_MAP) &&
			 (type != MATERIAL_TYPE_PARALLAX_MAP)) ||
			(name == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	memset(material, 0, (sizeof(Material)));

	material->type = type;
	material->shininess = MATERIAL_DEFAULT_SHININESS;
	material->layerCount = 0;
	material->name = strdup(name);

	material->diffuse.r = CGI_COLOR_WHITE_RED;
	material->diffuse.g = CGI_COLOR_WHITE_GREEN;
	material->diffuse.b = CGI_COLOR_WHITE_BLUE;
	material->diffuse.a = CGI_COLOR_WHITE_ALPHA;

	material->ambient.r = CGI_COLOR_WHITE_RED;
	material->ambient.g = CGI_COLOR_WHITE_GREEN;
	material->ambient.b = CGI_COLOR_WHITE_BLUE;
	material->ambient.a = CGI_COLOR_WHITE_ALPHA;

	material->specular.r = CGI_COLOR_WHITE_RED;
	material->specular.g = CGI_COLOR_WHITE_GREEN;
	material->specular.b = CGI_COLOR_WHITE_BLUE;
	material->specular.a = CGI_COLOR_WHITE_ALPHA;

	material->emissive.r = CGI_COLOR_WHITE_RED;
	material->emissive.g = CGI_COLOR_WHITE_GREEN;
	material->emissive.b = CGI_COLOR_WHITE_BLUE;
	material->emissive.a = CGI_COLOR_WHITE_ALPHA;

	material->layers = NULL;

	return 0;
}

Material *material_load(Log *log, char *assetFilename)
{
	if((log == NULL) || (assetFilename == NULL)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	/*
	 * TODO: this function
	 */

	return NULL;
}

int material_free(Material *material)
{
	if(material == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	/*
	 * TODO: this function
	 */

	return -1;
}

int material_freePtr(Material *material)
{
	if(material == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(material_free(material) < 0) {
		return -1;
	}

	free(material);

	return 0;
}

int material_saveAsAsgardMaterialFile(Material *material, char *filename)
{
	if((material == NULL) || (filename == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	/*
	 * TODO: this function
	 */

	return -1;
}

int material_setShininess(Material *material, int shininess)
{
	if((material == NULL) || (shininess < 0) || (shininess > 128)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	material->shininess = shininess;

	return 0;
}

int material_setDiffuse(Material *material, Color *color)
{
	if((material == NULL) || (color == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	material->diffuse.r = color->r;
	material->diffuse.g = color->g;
	material->diffuse.b = color->b;
	material->diffuse.a = color->a;

	return 0;
}

int material_setAmbient(Material *material, Color *color)
{
	if((material == NULL) || (color == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	material->ambient.r = color->r;
	material->ambient.g = color->g;
	material->ambient.b = color->b;
	material->ambient.a = color->a;

	return 0;
}

int material_setSpecular(Material *material, Color *color)
{
	if((material == NULL) || (color == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	material->specular.r = color->r;
	material->specular.g = color->g;
	material->specular.b = color->b;
	material->specular.a = color->a;

	return 0;
}

int material_setEmissive(Material *material, Color *color)
{
	if((material == NULL) || (color == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	material->emissive.r = color->r;
	material->emissive.g = color->g;
	material->emissive.b = color->b;
	material->emissive.a = color->a;

	return 0;
}

int material_addLayer(Material *material, Texture *texture)
{
	int ref = 0;

	if((material == NULL) || (texture == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if((material->type == MATERIAL_TYPE_TEXTURE) &&
			(material->layerCount > 0)) {
		return -1;
	}
	else if((material->type == MATERIAL_TYPE_MULTI_TEXTURE) &&
			(material->layerCount > MATERIAL_MAX_MULTI_TEXTURE_LAYERS)) {
		return -1;
	}
	else if((material->type == MATERIAL_TYPE_BUMP_MAP) &&
			(material->layerCount > MATERIAL_MAX_BUMP_MAP_LAYERS)) {
		return -1;
	}
	else if((material->type == MATERIAL_TYPE_PARALLAX_MAP) &&
			(material->layerCount > MATERIAL_MAX_PARALLAX_LAYERS)) {
		return -1;
	}

	if(material->layers == NULL) {
		ref = 0;
		material->layerCount = 1;
		material->layers = (Texture **)malloc(sizeof(Texture *) *
				material->layerCount);
	}
	else {
		ref = material->layerCount;
		material->layerCount += 1;
		material->layers = (Texture **)realloc(material->layers,
				(sizeof(Texture *) * material->layerCount));
	}

	material->layers[ref] = texture;

	return 0;
}

int material_apply(Material *material)
{
	int ii = 0;

	if(material == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(material->layerCount < 1) {
		return -1;
	}

	if(material->type == MATERIAL_TYPE_TEXTURE) {
		if(texture_apply(material->layers[0]) < 0) {
			return -1;
		}
		applyMaterialProperties(material);
	}
	else if(material->type == MATERIAL_TYPE_MULTI_TEXTURE) {
		for(ii = 0; ii < material->layerCount; ii++) {
			glActiveTexture(getMultiTextureId(ii));

			cgi_glEnable(GL_TEXTURE_2D);

			if(texture_apply(material->layers[ii]) < 0) {
				return -1;
			}

			applyMaterialProperties(material);
		}
	}
	else if(material->type == MATERIAL_TYPE_BUMP_MAP) {
		/*
		 * TODO: this
		 */
		return -1;
	}
	else if(material->type == MATERIAL_TYPE_PARALLAX_MAP) {
		/*
		 * TODO: this
		 */
		return -1;
	}

	return 0;
}

int material_renderMultiTexCoord2d(Material *material, double x, double y)
{
	int ii = 0;

	if(material == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if((material->type != MATERIAL_TYPE_MULTI_TEXTURE) ||
			(material->layerCount < 1)) {
		return -1;
	}

	for(ii = 0; ii < material->layerCount; ii++) {
		glMultiTexCoord2d(getMultiTextureId(ii), x, y);
	}

	return 0;
}

int material_unApply(Material *material)
{
	if(material == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	/*
	 * TODO: this function
	 */

	return -1;
}

int material_draw2d(Material *material, int x, int y, int width, int height)
{
	if(material == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	/*
	 * TODO: this function
	 */

	return -1;
}

