/*
 * material.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard Game Engine 2D materials system, header file.
 *
 * Written by Josh English.
 */

#if !defined(_ASGARD_ENGINE_MATERIAL_H)

#define _ASGARD_ENGINE_MATERIAL_H

#if !defined(_ASGARD_H) && !defined(_ASGARD_ENGINE_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _ASGARD_H || _ASGARD_ENGINE_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

// define material public constants

#define MATERIAL_DEFAULT_SHININESS					8

#define MATERIAL_MAX_MULTI_TEXTURE_LAYERS			32

#define MATERIAL_MAX_BUMP_MAP_LAYERS				2

#define MATERIAL_MAX_PARALLAX_LAYERS				3


typedef enum _MaterialType {
	MATERIAL_TYPE_TEXTURE = 1,
	MATERIAL_TYPE_MULTI_TEXTURE,
	MATERIAL_TYPE_BUMP_MAP,
	MATERIAL_TYPE_PARALLAX_MAP,
	MATERIAL_TYPE_UNKNOWN = -1
} MaterialType;


// define material public data types

typedef struct _Material {
	int type;
	int shininess;
	int layerCount;
	char *name;
	Color diffuse;
	Color ambient;
	Color specular;
	Color emissive;
	Texture **layers;
} Material;


// declare material public functions

int material_init(Material *material, int type, char *name);

Material *material_load(Log *log, char *assetFilename);

int material_free(Material *material);

int material_freePtr(Material *material);

int material_saveAsAsgardMaterialFile(Material *material, char *filename);

int material_setShininess(Material *material, int shininess);

int material_setDiffuse(Material *material, Color *color);

int material_setAmbient(Material *material, Color *color);

int material_setSpecular(Material *material, Color *color);

int material_setEmissive(Material *material, Color *color);

int material_addLayer(Material *material, Texture *texture);

int material_apply(Material *material);

int material_renderMultiTexCoord2d(Material *material, double x, double y);

int material_unApply(Material *material);

int material_draw2d(Material *material, int x, int y, int width, int height);


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _ASGARD_ENGINE_MATERIAL_H

