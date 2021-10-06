/*
 * lightgl.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The OpenGL lighting engine wrapper.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#include "asgard/asgard.h"
#define _ASGARD_ENGINE_EFFECTS_COMPONENT
#include "asgard/engine/effects/lightgl.h"


// define OpenGL light private functions

static int obtainOpenGlLightId(int id);

static void setLightColor(int id, int type, Color *setting);

static void setLightPosition(LightGL *light);

static void setLightDirection(LightGL *light);


// define OpenGL light private functions

static int obtainOpenGlLightId(int id)
{
	int result = 0;

	switch(id) {
		case 0:
			result = GL_LIGHT0;
			break;
		case 1:
			result = GL_LIGHT1;
			break;
		case 2:
			result = GL_LIGHT2;
			break;
		case 3:
			result = GL_LIGHT3;
			break;
		case 4:
			result = GL_LIGHT4;
			break;
		case 5:
			result = GL_LIGHT5;
			break;
		case 6:
			result = GL_LIGHT6;
			break;
		case 7:
			result = GL_LIGHT7;
			break;
		default:
			result = GL_LIGHT0;
	}

	return result;
}

static void setLightColor(int id, int type, Color *setting)
{
	GLfloat value[4];

	value[0] = (GLfloat)setting->r;
	value[1] = (GLfloat)setting->g;
	value[2] = (GLfloat)setting->b;
	value[3] = (GLfloat)setting->a;

	cgi_glLightfv(id, type, value);
}

static void setLightPosition(LightGL *light)
{
	GLfloat value[4];

	value[0] = (GLfloat)light->position.x;
	value[1] = (GLfloat)light->position.y;
	value[2] = (GLfloat)light->position.z;
	value[3] = (GLfloat)light->position.w;

	cgi_glLightfv(light->glid, GL_POSITION, value);
}

static void setLightDirection(LightGL *light)
{
	GLfloat value[4];

	value[0] = (GLfloat)light->spotDirection.x;
	value[1] = (GLfloat)light->spotDirection.y;
	value[2] = (GLfloat)light->spotDirection.z;
	value[3] = (GLfloat)light->spotDirection.w;

	cgi_glLightfv(light->glid, GL_SPOT_DIRECTION, value);
}


// define OpenGL light public functions

void light_init(LightGL *light, int lightId, double intensity)
{
	if((light == NULL) || ((lightId < 0) || (lightId > 7))) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	if((intensity < 0.01) || (intensity > 1.0)) {
		intensity = 0.5;
	}

	memset(light, 0, (sizeof(LightGL)));

	light->isSetup = afalse;
	light->isSpotlight = afalse;
	light->isOn = afalse;
	light->id = lightId;
	light->glid = 0;
	light->spotCutoff = 0.0;
	light->spotExponent = 0.0;

	light->ambient.r = (CGI_COLOR_WHITE_RED * intensity);
	light->ambient.g = (CGI_COLOR_WHITE_GREEN * intensity);
	light->ambient.b = (CGI_COLOR_WHITE_BLUE * intensity);
	light->ambient.a = (CGI_COLOR_WHITE_ALPHA * intensity);

	light->diffuse.r = CGI_COLOR_WHITE_RED;
	light->diffuse.g = CGI_COLOR_WHITE_GREEN;
	light->diffuse.b = CGI_COLOR_WHITE_BLUE;
	light->diffuse.a = CGI_COLOR_WHITE_ALPHA;

	light->specular.r = CGI_COLOR_WHITE_RED;
	light->specular.g = CGI_COLOR_WHITE_GREEN;
	light->specular.b = CGI_COLOR_WHITE_BLUE;
	light->specular.a = CGI_COLOR_WHITE_ALPHA;

	light->spotDirection.x = 0.0;
	light->spotDirection.y = 0.0;
	light->spotDirection.z = 0.0;
	light->spotDirection.w = 0.0;

	light->position.x = 0.0;
	light->position.y = 0.0;
	light->position.z = 0.0;
	light->position.w = 0.0;
}

void light_setColorAmbient(LightGL *light, Color *color)
{
	if((light == NULL) || (color == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	light->ambient.r = color->r;
	light->ambient.g = color->g;
	light->ambient.b = color->b;
	light->ambient.a = color->a;

	if(light->isSetup) {
		setLightColor(light->glid, GL_AMBIENT, &light->ambient);
	}
}

void light_setColorDiffuse(LightGL *light, Color *color)
{
	if((light == NULL) || (color == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	light->diffuse.r = color->r;
	light->diffuse.g = color->g;
	light->diffuse.b = color->b;
	light->diffuse.a = color->a;

	if(light->isSetup) {
		setLightColor(light->glid, GL_AMBIENT, &light->ambient);
	}
}

void light_setColorSpecular(LightGL *light, Color *color)
{
	if((light == NULL) || (color == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	light->specular.r = color->r;
	light->specular.g = color->g;
	light->specular.b = color->b;
	light->specular.a = color->a;

	if(light->isSetup) {
		setLightColor(light->glid, GL_AMBIENT, &light->ambient);
	}
}

void light_setPosition(LightGL *light, double x, double y, double z)
{
	if(light == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	light->position.x = x;
	light->position.y = y;
	light->position.z = z;
	light->position.w = 1.0;

	if(light->isSetup) {
		setLightPosition(light);
	}
}

void light_setSpotlight(LightGL *light, double cutoff, double exponent,
		double directionX, double directionY, double directionZ)
{
	if(light == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	light->isSpotlight = atrue;
	light->spotCutoff = cutoff;
	light->spotExponent = exponent;

	light->spotDirection.x = directionX;
	light->spotDirection.y = directionY;
	light->spotDirection.z = directionZ;
	light->spotDirection.w = 1.0;

	if(light->isSetup) {
		setLightDirection(light);
	}
}

void light_create(LightGL *light)
{
	if(light == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	light->glid = obtainOpenGlLightId(light->id);

	// enable lighting

	if(!cgi_glIsEnabled(GL_LIGHTING)) {
		cgi_glEnable(GL_LIGHTING);
	}

	// enable light

	cgi_glEnable(light->glid); 

	// light colors

	setLightColor(light->glid, GL_AMBIENT, &light->ambient);
	setLightColor(light->glid, GL_DIFFUSE, &light->diffuse);
	setLightColor(light->glid, GL_SPECULAR, &light->specular);

	// set default attenuation

	cgi_glLightf(light->glid, GL_CONSTANT_ATTENUATION, (GLfloat)1.0);
	cgi_glLightf(light->glid, GL_LINEAR_ATTENUATION, (GLfloat)0.2);
	cgi_glLightf(light->glid, GL_QUADRATIC_ATTENUATION, (GLfloat)0.08);

	// light position

	setLightPosition(light);

	// spotlight settings

	if(light->isSpotlight) {
		cgi_glLightf(light->glid, GL_SPOT_CUTOFF,
				(GLfloat)light->spotCutoff);
		cgi_glLightf(light->glid, GL_SPOT_EXPONENT,
				(GLfloat)light->spotExponent);
		setLightDirection(light);
	}

	// update light state

	light->isSetup = atrue;
	light->isOn = atrue;
}

void light_switchOn(LightGL *light)
{
	if(light == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	if(!light->isSetup) {
		return;
	}

	if(light->isOn) {
		return;
	}

	if(!cgi_glIsEnabled(GL_LIGHTING)) {
		cgi_glEnable(GL_LIGHTING);
	}

	cgi_glEnable(light->glid); 

	light->isOn = atrue;
}

void light_switchOff(LightGL *light)
{
	if(light == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	if(!light->isSetup) {
		return;
	}

	if(!light->isOn) {
		return;
	}

	cgi_glDisable(light->glid); 

	light->isOn = afalse;
}

