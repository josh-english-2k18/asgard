/*
 * draw3d.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard Game Engine 3D rendering system.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#include "asgard/asgard.h"
#define _ASGARD_ENGINE_COMPONENT
#include "asgard/engine/draw3d.h"


// define draw3d private functions

static void setLocationAndRotation(Point3d *location, Vertex3d *rotation);


// define draw3d private functions

static void setLocationAndRotation(Point3d *location, Vertex3d *rotation)
{
	cgi_glTranslated(location->x, location->y, location->z);

	cgi_glRotated(rotation->x, 1.0, 0.0, 0.0);
	cgi_glRotated(rotation->y, 0.0, 1.0, 0.0);
	cgi_glRotated(rotation->z, 0.0, 0.0, 1.0);
}

// define draw3d public functions

// rendering functions

void draw3d_sphere(Point3d *location, Vertex3d *rotation, Sphere3d *sphere,
		int renderMode)
{
	if((location == NULL) || (rotation == NULL) || (sphere == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	cgi_glPushMatrix();

	setLocationAndRotation(location, rotation);

	sphere3d_render3d(sphere, renderMode);

	cgi_glPopMatrix();
}

void draw3d_cone(Point3d *location, Vertex3d *rotation, Cone3d *cone,
		int renderMode)
{
	if((location == NULL) || (rotation == NULL) || (cone == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	cgi_glPushMatrix();

	setLocationAndRotation(location, rotation);

	cone3d_render3d(cone, renderMode);

	cgi_glPopMatrix();
}

void draw3d_frustum(Point3d *location, Vertex3d *rotation, Frustum3d *frustum,
		int renderMode)
{
	Vector3d alpha;
	Vector3d beta;

	if((location == NULL) || (rotation == NULL) || (frustum == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	cgi_glPushMatrix();

	setLocationAndRotation(location, rotation);

	CGI_COLOR_BLUE;

	if(renderMode == ASGARD_ENGINE_RENDER_MODE_POINTS) {
		cgi_glBegin(GL_POINTS);
	}
	else if((renderMode == ASGARD_ENGINE_RENDER_MODE_SOLID) ||
			(renderMode == ASGARD_ENGINE_RENDER_MODE_SOLID_NORMALIZED) ||
			(renderMode == ASGARD_ENGINE_RENDER_MODE_TEXTURED) ||
			(renderMode == ASGARD_ENGINE_RENDER_MODE_TEXTURED_NORMALIZED)) {
		cgi_glBegin(GL_QUADS);
	}
	else if((renderMode == ASGARD_ENGINE_RENDER_MODE_WIREFRAME) ||
			(renderMode == ASGARD_ENGINE_RENDER_MODE_WIREFRAME_WITH_NORMALS)) {
		cgi_glBegin(GL_LINE_LOOP);
	}

	cgi_glVertex3d(frustum->nearTopLeft.x, frustum->nearTopLeft.y,
			frustum->nearTopLeft.z);
	cgi_glVertex3d(frustum->nearTopRight.x, frustum->nearTopRight.y,
			frustum->nearTopRight.z);
	cgi_glVertex3d(frustum->nearBottomRight.x, frustum->nearBottomRight.y,
			frustum->nearBottomRight.z);
	cgi_glVertex3d(frustum->nearBottomLeft.x, frustum->nearBottomLeft.y,
			frustum->nearBottomLeft.z);

	if((renderMode == ASGARD_ENGINE_RENDER_MODE_WIREFRAME) ||
			(renderMode == ASGARD_ENGINE_RENDER_MODE_WIREFRAME_WITH_NORMALS)) {
		cgi_glEnd();
	}

	CGI_COLOR_BLUE;

	if((renderMode == ASGARD_ENGINE_RENDER_MODE_WIREFRAME) ||
			(renderMode == ASGARD_ENGINE_RENDER_MODE_WIREFRAME_WITH_NORMALS)) {
		cgi_glBegin(GL_LINE_LOOP);
	}

	cgi_glVertex3d(frustum->farTopRight.x, frustum->farTopRight.y,
			frustum->farTopRight.z);
	cgi_glVertex3d(frustum->farTopLeft.x, frustum->farTopLeft.y,
			frustum->farTopLeft.z);
	cgi_glVertex3d(frustum->farBottomLeft.x, frustum->farBottomLeft.y,
			frustum->farBottomLeft.z);
	cgi_glVertex3d(frustum->farBottomRight.x, frustum->farBottomRight.y,
			frustum->farBottomRight.z);

	if((renderMode == ASGARD_ENGINE_RENDER_MODE_WIREFRAME) ||
			(renderMode == ASGARD_ENGINE_RENDER_MODE_WIREFRAME_WITH_NORMALS)) {
		cgi_glEnd();
	}

	CGI_COLOR_RED;

	if((renderMode == ASGARD_ENGINE_RENDER_MODE_WIREFRAME) ||
			(renderMode == ASGARD_ENGINE_RENDER_MODE_WIREFRAME_WITH_NORMALS)) {
		cgi_glBegin(GL_LINE_LOOP);
	}

	cgi_glVertex3d(frustum->nearBottomLeft.x, frustum->nearBottomLeft.y,
			frustum->nearBottomLeft.z);
	cgi_glVertex3d(frustum->nearBottomRight.x, frustum->nearBottomRight.y,
			frustum->nearBottomRight.z);
	cgi_glVertex3d(frustum->farBottomRight.x, frustum->farBottomRight.y,
			frustum->farBottomRight.z);
	cgi_glVertex3d(frustum->farBottomLeft.x, frustum->farBottomLeft.y,
			frustum->farBottomLeft.z);

	if((renderMode == ASGARD_ENGINE_RENDER_MODE_WIREFRAME) ||
			(renderMode == ASGARD_ENGINE_RENDER_MODE_WIREFRAME_WITH_NORMALS)) {
		cgi_glEnd();
	}

	CGI_COLOR_RED;

	if((renderMode == ASGARD_ENGINE_RENDER_MODE_WIREFRAME) ||
			(renderMode == ASGARD_ENGINE_RENDER_MODE_WIREFRAME_WITH_NORMALS)) {
		cgi_glBegin(GL_LINE_LOOP);
	}

	cgi_glVertex3d(frustum->nearTopRight.x, frustum->nearTopRight.y,
			frustum->nearTopRight.z);
	cgi_glVertex3d(frustum->nearTopLeft.x, frustum->nearTopLeft.y,
			frustum->nearTopLeft.z);
	cgi_glVertex3d(frustum->farTopLeft.x, frustum->farTopLeft.y,
			frustum->farTopLeft.z);
	cgi_glVertex3d(frustum->farTopRight.x, frustum->farTopRight.y,
			frustum->farTopRight.z);

	if((renderMode == ASGARD_ENGINE_RENDER_MODE_WIREFRAME) ||
			(renderMode == ASGARD_ENGINE_RENDER_MODE_WIREFRAME_WITH_NORMALS)) {
		cgi_glEnd();
	}

	CGI_COLOR_GREEN;

	if((renderMode == ASGARD_ENGINE_RENDER_MODE_WIREFRAME) ||
			(renderMode == ASGARD_ENGINE_RENDER_MODE_WIREFRAME_WITH_NORMALS)) {
		cgi_glBegin(GL_LINE_LOOP);
	}

	cgi_glVertex3d(frustum->nearTopLeft.x, frustum->nearTopLeft.y,
			frustum->nearTopLeft.z);
	cgi_glVertex3d(frustum->nearBottomLeft.x, frustum->nearBottomLeft.y,
			frustum->nearBottomLeft.z);
	cgi_glVertex3d(frustum->farBottomLeft.x, frustum->farBottomLeft.y,
			frustum->farBottomLeft.z);
	cgi_glVertex3d(frustum->farTopLeft.x, frustum->farTopLeft.y,
			frustum->farTopLeft.z);

	if((renderMode == ASGARD_ENGINE_RENDER_MODE_WIREFRAME) ||
			(renderMode == ASGARD_ENGINE_RENDER_MODE_WIREFRAME_WITH_NORMALS)) {
		cgi_glEnd();
	}

	CGI_COLOR_GREEN;

	if((renderMode == ASGARD_ENGINE_RENDER_MODE_WIREFRAME) ||
			(renderMode == ASGARD_ENGINE_RENDER_MODE_WIREFRAME_WITH_NORMALS)) {
		cgi_glBegin(GL_LINE_LOOP);
	}

	cgi_glVertex3d(frustum->nearBottomRight.x, frustum->nearBottomRight.y,
			frustum->nearBottomRight.z);
	cgi_glVertex3d(frustum->nearTopRight.x, frustum->nearTopRight.y,
			frustum->nearTopRight.z);
	cgi_glVertex3d(frustum->farTopRight.x, frustum->farTopRight.y,
			frustum->farTopRight.z);
	cgi_glVertex3d(frustum->farBottomRight.x, frustum->farBottomRight.y,
			frustum->farBottomRight.z);

	cgi_glEnd();

	cgi_glBegin(GL_LINES);

	CGI_COLOR_BLUE;

	// near

	vector3d_initSetByVector(&alpha, &frustum->nearTopRight);
	vector3d_addEquals(&alpha, &frustum->nearTopLeft);
	vector3d_addEquals(&alpha, &frustum->nearBottomRight);
	vector3d_addEquals(&alpha, &frustum->nearBottomLeft);
	vector3d_multiplyEqualsValue(&alpha, 0.25);
	vector3d_initSetByVector(&beta, &alpha);
	vector3d_addEquals(&beta, &frustum->plane[FRUSTUM_PLANE_NEAR].normal);

	cgi_glVertex3d(alpha.x, alpha.y, alpha.z);
	cgi_glVertex3d(beta.x, beta.y, beta.z);

	// far

	vector3d_initSetByVector(&alpha, &frustum->farTopRight);
	vector3d_addEquals(&alpha, &frustum->farTopLeft);
	vector3d_addEquals(&alpha, &frustum->farBottomRight);
	vector3d_addEquals(&alpha, &frustum->farBottomLeft);
	vector3d_multiplyEqualsValue(&alpha, 0.25);
	vector3d_initSetByVector(&beta, &alpha);
	vector3d_addEquals(&beta, &frustum->plane[FRUSTUM_PLANE_FAR].normal);

	cgi_glVertex3d(alpha.x, alpha.y, alpha.z);
	cgi_glVertex3d(beta.x, beta.y, beta.z);

	// left

	vector3d_initSetByVector(&alpha, &frustum->farTopLeft);
	vector3d_addEquals(&alpha, &frustum->farBottomLeft);
	vector3d_addEquals(&alpha, &frustum->nearBottomLeft);
	vector3d_addEquals(&alpha, &frustum->nearTopLeft);
	vector3d_multiplyEqualsValue(&alpha, 0.25);
	vector3d_initSetByVector(&beta, &alpha);
	vector3d_addEquals(&beta, &frustum->plane[FRUSTUM_PLANE_LEFT].normal);

	cgi_glVertex3d(alpha.x, alpha.y, alpha.z);
	cgi_glVertex3d(beta.x, beta.y, beta.z);

	// right

	vector3d_initSetByVector(&alpha, &frustum->farTopRight);
	vector3d_addEquals(&alpha, &frustum->nearBottomRight);
	vector3d_addEquals(&alpha, &frustum->farBottomRight);
	vector3d_addEquals(&alpha, &frustum->nearTopRight);
	vector3d_multiplyEqualsValue(&alpha, 0.25);
	vector3d_initSetByVector(&beta, &alpha);
	vector3d_addEquals(&beta, &frustum->plane[FRUSTUM_PLANE_RIGHT].normal);

	cgi_glVertex3d(alpha.x, alpha.y, alpha.z);
	cgi_glVertex3d(beta.x, beta.y, beta.z);

	// top

	vector3d_initSetByVector(&alpha, &frustum->farTopRight);
	vector3d_addEquals(&alpha, &frustum->farTopLeft);
	vector3d_addEquals(&alpha, &frustum->nearTopRight);
	vector3d_addEquals(&alpha, &frustum->nearTopLeft);
	vector3d_multiplyEqualsValue(&alpha, 0.25);
	vector3d_initSetByVector(&beta, &alpha);
	vector3d_addEquals(&beta, &frustum->plane[FRUSTUM_PLANE_TOP].normal);

	cgi_glVertex3d(alpha.x, alpha.y, alpha.z);
	cgi_glVertex3d(beta.x, beta.y, beta.z);

	// bottom

	vector3d_initSetByVector(&alpha, &frustum->farBottomRight);
	vector3d_addEquals(&alpha, &frustum->farBottomLeft);
	vector3d_addEquals(&alpha, &frustum->nearBottomRight);
	vector3d_addEquals(&alpha, &frustum->nearBottomLeft);
	vector3d_multiplyEqualsValue(&alpha, 0.25);
	vector3d_initSetByVector(&beta, &alpha);
	vector3d_addEquals(&beta, &frustum->plane[FRUSTUM_PLANE_BOTTOM].normal);

	cgi_glVertex3d(alpha.x, alpha.y, alpha.z);
	cgi_glVertex3d(beta.x, beta.y, beta.z);

	cgi_glEnd();

	cgi_glPopMatrix();
}

void draw3d_draw3dObject(Draw3dObject *object)
{
	if(object == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	switch(object->type) {
		case DRAW3D_TYPE_SHAPE_SPHERE:
			draw3d_sphere(&object->location, &object->rotation,
					((Draw3dSphere *)object->payload)->sphere,
					object->renderMode);
			break;
		case DRAW3D_TYPE_SHAPE_CONE:
			draw3d_cone(&object->location, &object->rotation,
					((Draw3dCone *)object->payload)->cone,
					object->renderMode);
			break;
		default:
			DISPLAY_INVALID_ARGS;
	}
}

// object functions

Draw3dObject *draw3d_newObjectSphere(Point3d *location, Vertex3d *rotation,
		int sliceCount, int stackCount, double radius, double centerX,
		double centerY, double centerZ, int renderMode)
{
	Draw3dSphere *payload = NULL;
	Draw3dObject *result = NULL;

	if((location == NULL) || (rotation == NULL) || (sliceCount < 1) ||
			(stackCount < 1) || (radius <= 0.0)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	result = (Draw3dObject *)malloc(sizeof(Draw3dObject));

	result->type = DRAW3D_TYPE_SHAPE_SPHERE;
	result->renderMode = ASGARD_ENGINE_RENDER_MODE_WIREFRAME;
	result->payload = NULL;
	result->location.x = location->x;
	result->location.y = location->y;
	result->location.z = location->z;
	result->rotation.x = rotation->x;
	result->rotation.y = rotation->y;
	result->rotation.z = rotation->z;

	payload = (Draw3dSphere *)malloc(sizeof(Draw3dSphere));
	payload->sphere = sphere3d_new(sliceCount, stackCount, radius, centerX,
			centerY, centerZ);

	result->payload = (void *)payload;

	return result;
}

Draw3dObject *draw3d_newObjectSphereFromObject(Point3d *location,
		Vertex3d *rotation, Sphere3d *sphere)
{
	Draw3dSphere *payload = NULL;
	Draw3dObject *result = NULL;

	if((location == NULL) || (rotation == NULL) || (sphere == NULL)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	payload = (Draw3dSphere *)malloc(sizeof(Draw3dSphere));
	payload->sphere = sphere;

	result = (Draw3dObject *)malloc(sizeof(Draw3dObject));

	result->type = DRAW3D_TYPE_SHAPE_SPHERE;
	result->renderMode = ASGARD_ENGINE_RENDER_MODE_WIREFRAME;
	result->payload = (void *)payload;
	result->location.x = location->x;
	result->location.y = location->y;
	result->location.z = location->z;
	result->rotation.x = rotation->x;
	result->rotation.y = rotation->y;
	result->rotation.z = rotation->z;

	return result;
}

Draw3dObject *draw3d_newObjectCone(Point3d *location, Vertex3d *rotation,
		int sliceCount, int stackCount, double base, double height,
		double centerX, double centerY, double centerZ, int renderMode)
{
	Draw3dCone *payload = NULL;
	Draw3dObject *result = NULL;

	if((location == NULL) || (rotation == NULL) || (sliceCount < 1) ||
			(stackCount < 1) || (base <= 0.0) || (height < 0.0)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	result = (Draw3dObject *)malloc(sizeof(Draw3dObject));

	result->type = DRAW3D_TYPE_SHAPE_CONE;
	result->renderMode = ASGARD_ENGINE_RENDER_MODE_WIREFRAME;
	result->payload = NULL;
	result->location.x = location->x;
	result->location.y = location->y;
	result->location.z = location->z;
	result->rotation.x = rotation->x;
	result->rotation.y = rotation->y;
	result->rotation.z = rotation->z;

	payload = (Draw3dCone *)malloc(sizeof(Draw3dCone));
	payload->cone = cone3d_new(sliceCount, stackCount, base, height, centerX,
			centerY, centerZ);

	result->payload = (void *)payload;

	return result;
}

Draw3dObject *draw3d_newObjectConeFromObject(Point3d *location,
		Vertex3d *rotation, Cone3d *cone)
{
	Draw3dCone *payload = NULL;
	Draw3dObject *result = NULL;

	if((location == NULL) || (rotation == NULL) || (cone == NULL)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	payload = (Draw3dCone *)malloc(sizeof(Draw3dCone));
	payload->cone = cone;

	result = (Draw3dObject *)malloc(sizeof(Draw3dObject));

	result->type = DRAW3D_TYPE_SHAPE_CONE;
	result->renderMode = ASGARD_ENGINE_RENDER_MODE_WIREFRAME;
	result->payload = (void *)payload;
	result->location.x = location->x;
	result->location.y = location->y;
	result->location.z = location->z;
	result->rotation.x = rotation->x;
	result->rotation.y = rotation->y;
	result->rotation.z = rotation->z;

	return result;
}

int draw3d_freeObject(Draw3dObject *object)
{
	if(object == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	switch(object->type) {
		case DRAW3D_TYPE_SHAPE_SPHERE:
			sphere3d_free(((Draw3dSphere *)object->payload)->sphere);
			free(object->payload);
			break;
		case DRAW3D_TYPE_SHAPE_CONE:
			cone3d_free(((Draw3dCone *)object->payload)->cone);
			free(object->payload);
			break;
		default:
			DISPLAY_INVALID_ARGS;
	}

	free(object);

	return 0;
}

