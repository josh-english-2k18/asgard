/*
 * cone3d.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The 3D cone shape.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#include "asgard/asgard.h"
#define _ASGARD_ENGINE_SHAPES_COMPONENT
#include "asgard/engine/shapes/cone3d.h"


// declare cone private functions

static int buildConeTable(int length, double **sinList, double **cosList);

static void renderCone3d(Cone3d *cone, int renderMode);


// define cone private functions

static int buildConeTable(int length, double **sinList, double **cosList)
{
	int ii = 0;
	double angle = 0.0;

	if(length <= 0) {
		return -1;
	}

	angle = ((2.0 * math_PI) / (double)length);

	*sinList = (double *)malloc(sizeof(double) * (length + 1));
	*cosList = (double *)malloc(sizeof(double) * (length + 1));

	for(ii = 0; ii < length; ii++) {
		(*sinList)[ii] = sin(angle * (double)ii);
		(*cosList)[ii] = cos(angle * (double)ii);
	}

	(*sinList)[length] = (*sinList)[0];
	(*cosList)[length] = (*cosList)[0];

	return length;
}

static void renderCone3d(Cone3d *cone, int renderMode)
{
	int ii = 0;
	int nn = 0;
	int colorRef = 0;
	double zOne = 0.0;
	double zTwo = 0.0;
	double rOne = 0.0;
	double rTwo = 0.0;
	double zStep = 0.0;
	double rStep = 0.0;
	double cosValue = 0.0;
	double sinValue = 0.0;

	// calculate steps

	rOne = cone->base;
	zStep = (cone->height / (double)cone->stackCount);
	rStep = (cone->base / (double)cone->stackCount);

	// calculate scaling factors for vertex normals

	cosValue = (cone->height / sqrt((cone->height * cone->height) +
				(cone->base * cone->base)));

	sinValue = (cone->base / sqrt((cone->height * cone->height) +
				(cone->base * cone->base)));

	// render stacks

	if((renderMode == ASGARD_ENGINE_RENDER_MODE_WIREFRAME) ||
			(renderMode == ASGARD_ENGINE_RENDER_MODE_WIREFRAME_WITH_NORMALS) ||
			(renderMode == ASGARD_ENGINE_RENDER_MODE_POINTS)) {
		for(ii = 0; ii < cone->stackCount; ii++) {
			cgi_glBegin(GL_LINE_LOOP);

			for(nn = 0; nn < cone->sliceCount; nn++) {
				cgi_glNormal3d(
						(cone->stackCosList[nn] * sinValue),
						(cone->stackSinList[nn] * sinValue),
						cosValue);

				cgi_glVertex3d((cone->stackCosList[nn] * rOne),
						(cone->stackSinList[nn] * rOne),
						zOne);
			}

			cgi_glEnd();

			rOne -= rStep;
			zOne += zStep;
		}
	}
	else {
		zOne = 0.0;
		zTwo = zStep;
		rTwo = (rOne - rStep);

		cgi_glBegin(GL_TRIANGLE_FAN);

		cgi_glNormal3d(0.0, 0.0, -1.0);
		cgi_glVertex3d(0.0, 0.0, zOne);

		for(ii = 0; ii <= cone->sliceCount; ii++) {
			if(cone->isMulticolor) {
				cgi_glColor4d(cone->colors[colorRef].r,
						cone->colors[colorRef].g,
						cone->colors[colorRef].b,
						cone->colors[colorRef].a);
				colorRef++;
				if(colorRef > 2) {
					colorRef = 0;
				}
			}

			cgi_glVertex3d((cone->stackCosList[ii] * rOne),
					(cone->stackSinList[ii] * rOne),
					zOne);
		}

		cgi_glEnd();
	}

	// render slices

	if((renderMode == ASGARD_ENGINE_RENDER_MODE_WIREFRAME) ||
			(renderMode == ASGARD_ENGINE_RENDER_MODE_WIREFRAME_WITH_NORMALS) ||
			(renderMode == ASGARD_ENGINE_RENDER_MODE_POINTS)) {
		rOne = cone->base;

		cgi_glBegin(GL_LINES);

		for(ii = 0; ii < cone->sliceCount; ii++) {
			cgi_glNormal3d(
					(cone->stackCosList[ii] * sinValue),
					(cone->stackSinList[ii] * sinValue),
					cosValue);

			cgi_glVertex3d((cone->stackCosList[ii] * rOne),
					(cone->stackSinList[ii] * rOne),
					0.0);

			cgi_glVertex3d(0.0, 0.0, cone->height);
		}

		cgi_glEnd();
	}
	else {
		for(ii = 0; ii < (cone->stackCount - 1); ii++) {
			cgi_glBegin(GL_QUAD_STRIP);

			for(nn = 0; nn <= cone->sliceCount; nn++) {
				if(cone->isMulticolor) {
					cgi_glColor4d(cone->colors[colorRef].r,
							cone->colors[colorRef].g,
							cone->colors[colorRef].b,
							cone->colors[colorRef].a);
					colorRef++;
					if(colorRef > 2) {
						colorRef = 0;
					}
				}

				cgi_glNormal3d((cone->stackCosList[nn] * sinValue),
						(cone->stackSinList[nn] * sinValue),
						cosValue);

				cgi_glVertex3d((cone->stackCosList[nn] * rOne),
						(cone->stackSinList[nn] * rOne),
						zOne);

				cgi_glVertex3d((cone->stackCosList[nn] * rTwo),
						(cone->stackSinList[nn] * rTwo),
						zTwo);
			}

			cgi_glEnd();

			zOne = zTwo;
			zTwo += zStep;
			rOne = rTwo;
			rTwo -= rStep;
		}

		// complete stack render

		cgi_glBegin(GL_TRIANGLES);

		cgi_glNormal3d((cone->stackCosList[0] * sinValue),
				(cone->stackSinList[0] * sinValue),
				cosValue);

		for(ii = 0; ii < cone->sliceCount; ii++) {
			if(cone->isMulticolor) {
				cgi_glColor4d(cone->colors[colorRef].r,
						cone->colors[colorRef].g,
						cone->colors[colorRef].b,
						cone->colors[colorRef].a);
				colorRef++;
				if(colorRef > 2) {
					colorRef = 0;
				}
			}

			cgi_glVertex3d((cone->stackCosList[ii] * rOne),
					(cone->stackSinList[ii] * rOne),
					zOne);

			cgi_glVertex3d(0.0, 0.0, cone->height);

			cgi_glNormal3d((cone->stackCosList[(ii + 1)] * sinValue),
					(cone->stackSinList[(ii + 1)] * sinValue),
					cosValue);

			cgi_glVertex3d((cone->stackCosList[(ii + 1)] * rOne),
					(cone->stackSinList[(ii + 1)] * rOne),
					zOne);
		}

		cgi_glEnd();
	}
}


// define cone public functions

Cone3d *cone3d_new(int sliceCount, int stackCount, double base, double height,
		double centerX, double centerY, double centerZ)
{
	Cone3d *result = NULL;

	if((sliceCount < 1) || (stackCount < 1) || (base < 0.001) ||
			(height < 0.001)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	result = (Cone3d *)malloc(sizeof(Cone3d));

	result->isListed = afalse;
	result->hasColor = afalse;
	result->isMulticolor = afalse;
	result->glId = 0;
	result->sliceCount = sliceCount;
	result->stackCount = stackCount;
	result->renderMode = ASGARD_ENGINE_RENDER_MODE_WIREFRAME;
	result->base = base;
	result->height = height;
	result->stackSinList = NULL;
	result->stackCosList = NULL;
	result->center.x = centerX;
	result->center.y = centerY;
	result->center.z = centerZ;

	// calculate the cone tables

	if(buildConeTable(sliceCount, &result->stackSinList,
				&result->stackCosList) < 0) {
		cone3d_free(result);
		return NULL;
	}

	return result;
}

Cone3d *cone3d_load(Log *log, char *assetFilename)
{
	int sliceCount = 0;
	int stackCount = 0;
	double base = 0.0;
	double height = 0.0;
	char *sValue = NULL;

	Cone3d *result = NULL;
	Config config;
	Point3d center;

	if((log == NULL) || (assetFilename == NULL)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	if(config_init(&config, assetFilename) < 0) {
		log_logf(log, LOG_LEVEL_ERROR,
				"{CONE} failed to load configuration file '%s'",
				assetFilename);
		return NULL;
	}

	// validate that the configuration file is for a 3D cone asset

	config_getString(&config, "asset", "class", "unknown", &sValue);
	if(strcmp(sValue, "3d")) {
		log_logf(log, LOG_LEVEL_ERROR,
				"{CONE} invalid cone configuration - asset.class is '%s'",
				sValue);
		return NULL;
	}

	config_getString(&config, "asset", "type", "unknown", &sValue);
	if(strcmp(sValue, "mesh-shape-cone")) {
		log_logf(log, LOG_LEVEL_ERROR,
				"{CONE} invalid cone configuration - asset.type is '%s'",
				sValue);
		return NULL;
	}

	// load the cone configuration

	config_getInteger(&config, "mesh-shape-cone", "sliceCount", 0,
			&sliceCount);
	if(sliceCount < 1) {
		log_logf(log, LOG_LEVEL_ERROR,
				"{CONE} invalid cone configuration - slice count is '%i'",
				sliceCount);
		return NULL;
	}

	config_getInteger(&config, "mesh-shape-cone", "stackCount", 0,
			&stackCount);
	if(stackCount < 1) {
		log_logf(log, LOG_LEVEL_ERROR,
				"{CONE} invalid cone configuration - stack count is '%i'",
				stackCount);
		return NULL;
	}

	config_getDouble(&config, "mesh-shape-cone", "base", 0.0, &base);
	if(base <= 0.0) {
		log_logf(log, LOG_LEVEL_ERROR,
				"{CONE} invalid cone configuration - base is '%0.2f'",
				base);
		return NULL;
	}

	config_getDouble(&config, "mesh-shape-cone", "height", 0.0, &height);
	if(height <= 0.0) {
		log_logf(log, LOG_LEVEL_ERROR,
				"{CONE} invalid cone configuration - height is '%0.2f'",
				height);
		return NULL;
	}

	config_getDouble(&config, "mesh-shape-cone", "centerX", 0.0, &center.x);
	config_getDouble(&config, "mesh-shape-cone", "centerY", 0.0, &center.y);
	config_getDouble(&config, "mesh-shape-cone", "centerZ", 0.0, &center.z);

	// build the cone

	result = cone3d_new(sliceCount, stackCount, base, height, center.x,
			center.y, center.z);

	log_logf(log, LOG_LEVEL_DEBUG, "{CONE} loaded cone '%s'",
			assetFilename);

	// cleanup

	config_free(&config);

	return result;
}

void cone3d_free(Cone3d *cone)
{
	if(cone == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	if(cone->isListed) {
		cgi_glDeleteLists(cone->glId, 1);
	}

	if(cone->stackSinList != NULL) {
		free(cone->stackSinList);
	}

	if(cone->stackCosList != NULL) {
		free(cone->stackCosList);
	}

	free(cone);
}

void cone3d_setColor(Cone3d *cone, Color *color)
{
	if((cone == NULL) || (color == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	cone->hasColor = atrue;

	if(cone->isListed) {
		cgi_glDeleteLists(cone->glId, 1);
		cone->isListed = afalse;
	}

	cone->colors[0].r = color->r;
	cone->colors[0].g = color->g;
	cone->colors[0].b = color->b;
	cone->colors[0].a = color->a;
}

void cone3d_setMultiColor(Cone3d *cone, Color *colorOne, Color *colorTwo,
		Color *colorThree)
{
	if((cone == NULL) || (colorOne == NULL) || (colorTwo == NULL) ||
			(colorThree == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	if(cone->hasColor) {
		cone->hasColor = afalse;
	}

	if(cone->isListed) {
		cgi_glDeleteLists(cone->glId, 1);
		cone->isListed = afalse;
	}

	cone->isMulticolor = atrue;

	cone->colors[0].r = colorOne->r;
	cone->colors[0].g = colorOne->g;
	cone->colors[0].b = colorOne->b;
	cone->colors[0].a = colorOne->a;

	cone->colors[1].r = colorTwo->r;
	cone->colors[1].g = colorTwo->g;
	cone->colors[1].b = colorTwo->b;
	cone->colors[1].a = colorTwo->a;

	cone->colors[2].r = colorThree->r;
	cone->colors[2].g = colorThree->g;
	cone->colors[2].b = colorThree->b;
	cone->colors[2].a = colorThree->a;
}

void cone3d_render3d(Cone3d *cone, int renderMode)
{
	aboolean isTexture2d = afalse;
	aboolean isColorMaterial = afalse;

	if(cone == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	// setup rendering properties

	if(cgi_glIsEnabled(GL_TEXTURE_2D)) {
		isTexture2d = atrue;
		cgi_glDisable(GL_TEXTURE_2D);
	}

	if(cgi_glIsEnabled(GL_COLOR_MATERIAL)) {
		isColorMaterial = atrue;
	}
	else {
		cgi_glEnable(GL_COLOR_MATERIAL);
	}

	if(cone->hasColor) {
		cgi_glColor4d(cone->colors[0].r, cone->colors[0].g,
				cone->colors[0].b, cone->colors[0].a);
	}
	else if((!cone->hasColor) && (!cone->isMulticolor)) {
		CGI_COLOR_YELLOWGREEN;
	}

	// render cone

	if(cone->isListed) {
		if(cone->renderMode != renderMode) {
			cgi_glDeleteLists(cone->glId, 1);
			cone->isListed = afalse;
		}
		else {
			cgi_glCallList(cone->glId);
		}
	}

	if(!cone->isListed) {
		cone->glId = cgi_glGenLists(1);

		cgi_glNewList(cone->glId, GL_COMPILE);

		renderCone3d(cone, renderMode);

		cgi_glEndList();

		cone->isListed = atrue;
		cone->renderMode = renderMode;
	}

	// complete rendering

	if(isTexture2d) {
		cgi_glEnable(GL_TEXTURE_2D);
	}

	if(!isColorMaterial) {
		cgi_glDisable(GL_COLOR_MATERIAL);
	}
}

