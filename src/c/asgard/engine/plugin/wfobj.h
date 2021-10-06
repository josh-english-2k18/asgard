/*
 * wfobj.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * A plugin to read 3D Wavefront Object (.obj) files along with their
 * corresponding material files and processing them into a renderable static
 * mesh, header file.
 *
 * Written by Josh English.
 */

#if !defined(_ASGARD_ENGINE_PLUGIN_WFOBJ_H)

#define _ASGARD_ENGINE_PLUGIN_WFOBJ_H

#if !defined(_ASGARD_H) && !defined(_ASGARD_ENGINE_PLUGIN_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _ASGARD_H || _ASGARD_ENGINE_PLUGIN_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// define wfobj public constants

typedef enum _WfobjObjectTypes {
	WFOBJ_OBJ_TYPE_V = 1,
	WFOBJ_OBJ_TYPE_VT,
	WFOBJ_OBJ_TYPE_VTN,
	WFOBJ_OBJ_TYPE_UNKNOWN = -1
} WfobjObjectTypes;

typedef enum _WfobjFaceTypes {
	WFOBJ_FACE_TYPE_TRIANGLES = 3,
	WFOBJ_FACE_TYPE_QUADS,
	WFOBJ_FACE_TYPE_POLYGONS,
	WFOBJ_FACE_TYPE_UNKNOWN = -1
} WfobjFaceTypes;

#define WFOBJ_READ_LINE_LENGTH					2048


// define wfobj public data types

typedef struct _WfobjVertex3d {
	aboolean isNormalized;
	float x;
	float y;
	float z;
} WfobjVertex3d;

typedef struct _WfobjNormal3d {
	float x;
	float y;
	float z;
} WfobjNormal3d;

typedef struct _WfobjTexture3d {
	float u;
	float v;
} WfobjTexture3d;

typedef struct _WfobjBoundingSphere3d {
	double radius;
	Vector3d center;
	void *render;
} WfobjBoundingSphere3d;

typedef struct _WfobjBoundingBox {
	Vector3d high;
	Vector3d low;
} WfobjBoundingBox;

typedef struct _WfobjObject3d {
	aboolean isListed;
	unsigned int listId;
	int type;
	int material;
	char *name;
	char *smoothing;
	int points;
	int faces;
	int *types;
	int vertexCount;
	int vertexErrorCount;
	int *verticies;
	int textureCount;
	int textureErrorCount;
	int *textures;
	int normalCount;
	int normalErrorCount;
	int *normals;
	int renderPolygonCount;
	WfobjBoundingSphere3d sphere;
} WfobjObject3d;

typedef struct _WfobjMaterial3d {
	char *name;
	char *filename;
	int illumination_model;
	float shininess;
	float diffuse[4];
	float ambient[4];
	float specular[4];
	float emissive[4];
	float filter[3];
} WfobjMaterial3d;

typedef struct _Wfobj {
	char *filename;
	char *materialFilename;
	int lineCount;
	int lineErrorCount;
	int commentLineCount;
	int vertexCount;
	int normalCount;
	int textureCount;
	int objectCount;
	int materialCount;
	int renderPolygonCount;
	double renderScale;
	WfobjVertex3d **verticies;
	WfobjNormal3d **normals;
	WfobjTexture3d **textures;
	WfobjObject3d **objects;
	WfobjMaterial3d **materials;
	WfobjBoundingSphere3d sphere;
	WfobjBoundingBox boundingBox;
} Wfobj;


// define wfobj public functions

Wfobj *wfobj_load(Log *log, char *filename);

void wfobj_free(Wfobj *obj);

void wfobj_render3d(int renderMode, float x, float y, float z, Wfobj *obj,
		void **textures, Vector3d *rotation, void *frustum,
		double scale, aboolean render_bb);


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _ASGARD_ENGINE_PLUGIN_WFOBJ_H

