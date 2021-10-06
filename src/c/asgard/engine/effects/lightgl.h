/*
 * lightgl.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The OpenGL lighting engine wrapper, header file.
 *
 * Written by Josh English.
 */

#if !defined(_ASGARD_ENGINE_EFFECTS_LIGHTGL_H)

#define _ASGARD_ENGINE_EFFECTS_LIGHTGL_H

#if !defined(_ASGARD_H) && !defined(_ASGARD_ENGINE_EFFECTS_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _ASGARD_H || _ASGARD_ENGINE_EFFECTS_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// define OpenGL light data types

typedef struct _LightGL {
	aboolean isSetup;
	aboolean isSpotlight;
	aboolean isOn;
	int id;
	int glid;
	int status;
	double spotCutoff;
	double spotExponent;
	Color ambient;
	Color diffuse;
	Color specular;
	Vertex4d spotDirection;
	Point4d position;
} LightGL;


// declare OpenGL light public functions

void light_init(LightGL *light, int lightId, double intensity);

void light_setColorAmbient(LightGL *light, Color *color);

void light_setColorDiffuse(LightGL *light, Color *color);

void light_setColorSpecular(LightGL *light, Color *color);

void light_setPosition(LightGL *light, double x, double y, double z);

void light_setSpotlight(LightGL *light, double cutoff, double exponent,
		double directionX, double directionY, double directionZ);

void light_create(LightGL *light);

void light_switchOn(LightGL *light);

void light_switchOff(LightGL *light);


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _ASGARD_ENGINE_EFFECTS_LIGHTGL_H

