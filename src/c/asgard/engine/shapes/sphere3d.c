/*
 * sphere3d.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The 3D sphere shape.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#include "asgard/asgard.h"
#define _ASGARD_ENGINE_SHAPES_COMPONENT
#include "asgard/engine/shapes/sphere3d.h"


// declare sphere private functions

static int buildSphereTable(int length, double **sinList, double **cosList);

static void renderSphere3d(Sphere3d *sphere, int renderMode);


// define sphere private functions

static int buildSphereTable(int length, double **sinList, double **cosList)
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

static void renderSphere3d(Sphere3d *sphere, int renderMode)
{
	int ii = 0;
	int nn = 0;
	int colorRef = 0;
	double zOne = 0.0;
	double zTwo = 0.0;
	double rOne = 0.0;
	double rTwo = 0.0;

	// render stacks

	if((renderMode == ASGARD_ENGINE_RENDER_MODE_WIREFRAME) ||
			(renderMode == ASGARD_ENGINE_RENDER_MODE_WIREFRAME_WITH_NORMALS) ||
			(renderMode == ASGARD_ENGINE_RENDER_MODE_POINTS)) {
		for(ii = 1; ii < sphere->stackCount; ii++) {
			if(sphere->isMulticolor) {
				cgi_glColor4d(sphere->colors[colorRef].r,
						sphere->colors[colorRef].g,
						sphere->colors[colorRef].b,
						sphere->colors[colorRef].a);
				colorRef++;
				if(colorRef > 2) {
					colorRef = 0;
				}
			}

			if(renderMode == ASGARD_ENGINE_RENDER_MODE_POINTS) {
				cgi_glBegin(GL_POINTS);
			}
			else {
				cgi_glBegin(GL_LINE_LOOP);
			}

			for(nn = 0; nn <= sphere->sliceCount; nn++) {
				cgi_glNormal3d(sphere->sliceCosList[nn],
						sphere->sliceSinList[nn],
						sphere->stackCosList[ii]);

				cgi_glVertex3d(
						(sphere->sliceCosList[nn] * sphere->stackSinList[ii] *
						 sphere->radius),
						(sphere->sliceSinList[nn] * sphere->stackSinList[ii] *
						 sphere->radius),
						(sphere->stackCosList[ii] * sphere->radius));
			}

			cgi_glEnd();
		}
	}
	else {
		cgi_glBegin(GL_TRIANGLE_FAN);

		cgi_glNormal3d(0.0, 0.0, 1.0);
		cgi_glVertex3d(0.0, 0.0, sphere->radius);

		for(ii = sphere->sliceCount; ii >= 0; ii--) {
			if(sphere->isMulticolor) {
				cgi_glColor4d(sphere->colors[colorRef].r,
						sphere->colors[colorRef].g,
						sphere->colors[colorRef].b,
						sphere->colors[colorRef].a);
				colorRef++;
				if(colorRef > 2) {
					colorRef = 0;
				}
			}

			cgi_glNormal3d(
					(sphere->sliceCosList[ii] * sphere->stackSinList[1]),
					(sphere->sliceSinList[ii] * sphere->stackSinList[1]),
					sphere->stackCosList[1]);

			cgi_glVertex3d(
					(sphere->sliceCosList[ii] * sphere->stackSinList[1] *
					 sphere->radius),
					(sphere->sliceSinList[ii] * sphere->stackSinList[1] *
					 sphere->radius),
					(sphere->stackCosList[1] * sphere->radius));
		}

		cgi_glEnd();
	}

	// render slices

	if(sphere->isMulticolor) {
		colorRef = 0;
	}

	if((renderMode == ASGARD_ENGINE_RENDER_MODE_WIREFRAME) ||
			(renderMode == ASGARD_ENGINE_RENDER_MODE_WIREFRAME_WITH_NORMALS) ||
			(renderMode == ASGARD_ENGINE_RENDER_MODE_POINTS)) {
		for(ii = 0; ii < sphere->sliceCount; ii++) {
			if(sphere->isMulticolor) {
				cgi_glColor4d(sphere->colors[colorRef].r,
						sphere->colors[colorRef].g,
						sphere->colors[colorRef].b,
						sphere->colors[colorRef].a);
				colorRef++;
				if(colorRef > 2) {
					colorRef = 0;
				}
			}

			if(renderMode == ASGARD_ENGINE_RENDER_MODE_POINTS) {
				cgi_glBegin(GL_POINTS);
			}
			else {
				cgi_glBegin(GL_LINE_STRIP);
			}

			for(nn = 0; nn <= sphere->stackCount; nn++) {
				cgi_glNormal3d(
						(sphere->sliceCosList[ii] * sphere->stackSinList[nn]),
						(sphere->sliceSinList[ii] * sphere->stackSinList[nn]),
						sphere->stackCosList[nn]);

				cgi_glVertex3d(
						((sphere->sliceCosList[ii] *
						  sphere->stackSinList[nn]) * sphere->radius),
						((sphere->sliceSinList[ii] *
						  sphere->stackSinList[nn]) * sphere->radius),
						(sphere->stackCosList[nn] * sphere->radius));
			}

			cgi_glEnd();
		}
	}
	else {
		zTwo = sphere->stackCosList[1];
		rTwo = sphere->stackSinList[1];

		for(ii = 1; ii < (sphere->stackCount - 1); ii++) {
			zOne = zTwo;
			zTwo = sphere->stackCosList[(ii + 1)];

			rOne = rTwo;
			rTwo = sphere->stackSinList[(ii + 1)];

			cgi_glBegin(GL_QUAD_STRIP);

			for(nn = 0; nn <= sphere->sliceCount; nn++) {
				if(sphere->isMulticolor) {
					cgi_glColor4d(sphere->colors[colorRef].r,
							sphere->colors[colorRef].g,
							sphere->colors[colorRef].b,
							sphere->colors[colorRef].a);
					colorRef++;
					if(colorRef > 2) {
						colorRef = 0;
					}
				}

				cgi_glNormal3d((sphere->sliceCosList[nn] * rTwo),
						(sphere->sliceSinList[nn] * rTwo), zTwo);

				cgi_glVertex3d(
						(sphere->sliceCosList[nn] * rTwo * sphere->radius),
						(sphere->sliceSinList[nn] * rTwo * sphere->radius),
						(zTwo * sphere->radius));

				cgi_glNormal3d((sphere->sliceCosList[nn] * rOne),
						(sphere->sliceSinList[nn] * rOne), zOne);

				cgi_glVertex3d(
						(sphere->sliceCosList[nn] * rOne * sphere->radius),
						(sphere->sliceSinList[nn] * rOne * sphere->radius),
						(zOne * sphere->radius));
			}

			cgi_glEnd();
		}

		if(sphere->isMulticolor) {
			colorRef = 0;
		}

		zOne = zTwo;
		rOne = rTwo;

		cgi_glBegin(GL_TRIANGLE_FAN);

		cgi_glNormal3d(0.0, 0.0, -1.0);
		cgi_glVertex3d(0.0, 0.0, -sphere->radius);

		for(ii = 0; ii <= sphere->sliceCount; ii++) {
			if(sphere->isMulticolor) {
				cgi_glColor4d(sphere->colors[colorRef].r,
						sphere->colors[colorRef].g,
						sphere->colors[colorRef].b,
						sphere->colors[colorRef].a);
				colorRef++;
				if(colorRef > 2) {
					colorRef = 0;
				}
			}

			cgi_glNormal3d((sphere->sliceCosList[ii] * rOne),
					(sphere->sliceSinList[ii] * rOne),
					zOne);

			cgi_glVertex3d((sphere->sliceCosList[ii] * rOne * sphere->radius),
					(sphere->sliceSinList[ii] * rOne * sphere->radius),
					(zOne * sphere->radius));
		}

		cgi_glEnd();
	}
}


// define sphere public functions

Sphere3d *sphere3d_new(int sliceCount, int stackCount, double radius,
		double centerX, double centerY, double centerZ)
{
	Sphere3d *result = NULL;

	if((sliceCount < 1) || (stackCount < 1) || (radius < 0.001)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	result = (Sphere3d *)malloc(sizeof(Sphere3d));

	result->isListed = afalse;
	result->hasColor = afalse;
	result->isMulticolor = afalse;
	result->glId = 0;
	result->sliceCount = sliceCount;
	result->stackCount = stackCount;
	result->renderMode = ASGARD_ENGINE_RENDER_MODE_WIREFRAME;
	result->radius = radius;
	result->stackSinList = NULL;
	result->stackCosList = NULL;
	result->sliceSinList = NULL;
	result->sliceCosList = NULL;
	result->center.x = centerX;
	result->center.y = centerY;
	result->center.z = centerZ;

	// calculate the sphere tables

	if(buildSphereTable((stackCount * 2), &result->stackSinList,
				&result->stackCosList) < 0) {
		sphere3d_free(result);
		return NULL;
	}

	if(buildSphereTable(sliceCount, &result->sliceSinList,
				&result->sliceCosList) < 0) {
		sphere3d_free(result);
		return NULL;
	}

	return result;
}

Sphere3d *sphere3d_load(Log *log, char *assetFilename)
{
	int sliceCount = 0;
	int stackCount = 0;
	double radius = 0.0;
	char *sValue = NULL;

	Sphere3d *result = NULL;
	Config config;
	Point3d center;

	if((log == NULL) || (assetFilename == NULL)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	if(config_init(&config, assetFilename) < 0) {
		log_logf(log, LOG_LEVEL_ERROR,
				"{SPHERE} failed to load configuration file '%s'",
				assetFilename);
		return NULL;
	}

	// validate that the configuration file is for a 3D sphere asset

	config_getString(&config, "asset", "class", "unknown", &sValue);
	if(strcmp(sValue, "3d")) {
		log_logf(log, LOG_LEVEL_ERROR,
				"{SPHERE} invalid sphere configuration - asset.class is '%s'",
				sValue);
		return NULL;
	}

	config_getString(&config, "asset", "type", "unknown", &sValue);
	if(strcmp(sValue, "mesh-shape-sphere")) {
		log_logf(log, LOG_LEVEL_ERROR,
				"{SPHERE} invalid sphere configuration - asset.type is '%s'",
				sValue);
		return NULL;
	}

	// load the sphere configuration

	config_getInteger(&config, "mesh-shape-sphere", "sliceCount", 0,
			&sliceCount);
	if(sliceCount < 1) {
		log_logf(log, LOG_LEVEL_ERROR,
				"{SPHERE} invalid sphere configuration - slice count is '%i'",
				sliceCount);
		return NULL;
	}

	config_getInteger(&config, "mesh-shape-sphere", "stackCount", 0,
			&stackCount);
	if(stackCount < 1) {
		log_logf(log, LOG_LEVEL_ERROR,
				"{SPHERE} invalid sphere configuration - stack count is '%i'",
				stackCount);
		return NULL;
	}

	config_getDouble(&config, "mesh-shape-sphere", "radius", 0.0, &radius);
	if(radius <= 0.0) {
		log_logf(log, LOG_LEVEL_ERROR,
				"{SPHERE} invalid sphere configuration - radius is '%0.2f'",
				radius);
		return NULL;
	}

	config_getDouble(&config, "mesh-shape-sphere", "centerX", 0.0, &center.x);
	config_getDouble(&config, "mesh-shape-sphere", "centerY", 0.0, &center.y);
	config_getDouble(&config, "mesh-shape-sphere", "centerZ", 0.0, &center.z);

	// build the sphere

	result = sphere3d_new(sliceCount, stackCount, radius, center.x, center.y,
			center.z);

	log_logf(log, LOG_LEVEL_DEBUG, "{SPHERE} loaded sphere '%s'",
			assetFilename);

	// cleanup

	config_free(&config);

	return result;
}

void sphere3d_free(Sphere3d *sphere)
{
	if(sphere == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	if(sphere->isListed) {
		cgi_glDeleteLists(sphere->glId, 1);
	}

	if(sphere->stackSinList != NULL) {
		free(sphere->stackSinList);
	}

	if(sphere->stackCosList != NULL) {
		free(sphere->stackCosList);
	}

	if(sphere->sliceSinList != NULL) {
		free(sphere->sliceSinList);
	}

	if(sphere->sliceCosList != NULL) {
		free(sphere->sliceCosList);
	}

	free(sphere);
}

void sphere3d_setColor(Sphere3d *sphere, Color *color)
{
	if((sphere == NULL) || (color == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	sphere->hasColor = atrue;

	if(sphere->isListed) {
		cgi_glDeleteLists(sphere->glId, 1);
		sphere->isListed = afalse;
	}

	sphere->colors[0].r = color->r;
	sphere->colors[0].g = color->g;
	sphere->colors[0].b = color->b;
	sphere->colors[0].a = color->a;
}

void sphere3d_setMultiColor(Sphere3d *sphere, Color *colorOne, Color *colorTwo,
		Color *colorThree)
{
	if((sphere == NULL) || (colorOne == NULL) || (colorTwo == NULL) ||
			(colorThree == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	if(sphere->hasColor) {
		sphere->hasColor = afalse;
	}

	if(sphere->isListed) {
		cgi_glDeleteLists(sphere->glId, 1);
		sphere->isListed = afalse;
	}

	sphere->isMulticolor = atrue;

	sphere->colors[0].r = colorOne->r;
	sphere->colors[0].g = colorOne->g;
	sphere->colors[0].b = colorOne->b;
	sphere->colors[0].a = colorOne->a;

	sphere->colors[1].r = colorTwo->r;
	sphere->colors[1].g = colorTwo->g;
	sphere->colors[1].b = colorTwo->b;
	sphere->colors[1].a = colorTwo->a;

	sphere->colors[2].r = colorThree->r;
	sphere->colors[2].g = colorThree->g;
	sphere->colors[2].b = colorThree->b;
	sphere->colors[2].a = colorThree->a;
}

void sphere3d_render3d(Sphere3d *sphere, int renderMode)
{
	aboolean isTexture2d = afalse;
	aboolean isColorMaterial = afalse;

	if(sphere == NULL) {
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

	if(sphere->hasColor) {
		cgi_glColor4d(sphere->colors[0].r, sphere->colors[0].g,
				sphere->colors[0].b, sphere->colors[0].a);
	}
	else if((!sphere->hasColor) && (!sphere->isMulticolor)) {
		CGI_COLOR_YELLOWGREEN;
	}

	// define position

	cgi_glPushMatrix();
	cgi_glTranslated(sphere->center.x, sphere->center.y, sphere->center.z);

	// render sphere

	if(sphere->isListed) {
		if(sphere->renderMode != renderMode) {
			cgi_glDeleteLists(sphere->glId, 1);
			sphere->isListed = afalse;
		}
		else {
			cgi_glCallList(sphere->glId);
		}
	}

	if(!sphere->isListed) {
		sphere->glId = cgi_glGenLists(1);

		cgi_glNewList(sphere->glId, GL_COMPILE);

		renderSphere3d(sphere, renderMode);

		cgi_glEndList();

		sphere->isListed = atrue;
		sphere->renderMode = renderMode;
	}

	// release position translation

	cgi_glPopMatrix();

	// complete rendering

	if(isTexture2d) {
		cgi_glEnable(GL_TEXTURE_2D);
	}

	if(!isColorMaterial) {
		cgi_glDisable(GL_COLOR_MATERIAL);
	}
}

