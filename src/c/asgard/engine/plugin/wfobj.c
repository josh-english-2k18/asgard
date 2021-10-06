/*
 * wfobj.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * A plugin to read 3D Wavefront Object (.obj) files along with their
 * corresponding material files and processing them into a renderable static
 * mesh.
 *
 * Written by Josh English.
 */

/*
 * example lines

--- object file ---

# a comment line
mtllib a_material_file.mtl
	- material library
g default
	- consider ignoring this line?
g StaticMeshActor0
	- the following faces belong to this object
v -0.396748 -0.025845 0.100926
	- vertex
vt 0.000500 -0.000500
vt 0.000500 -0.000500 0.01230
	- texture
vn -0.421209 0.888682 -0.181182
	- normal
s off
	- smoothing
usemtl some_material_name
	- use a material for the following faces
f 100 101 102
f 100/100 101/101 102/102
f 100/100/ 101/101/ 102/102/
f 100// 101// 102//
f 100//100 101//101 102//102
f 100/100/100 101/101/101 102/102/102
	- faces

--- material file ---

newmtl a material name
illum 4
	- illumination
Kd 0.00 0.00 0.00
Kd 0.00 0.00 0.00 0.00
	- diffusion
Ka 0.00 0.00 0.00
Ka 0.00 0.00 0.00 0.00
	- ambient
Ks float float float
Ks float float float float
	- specular
Ke float float float
Ke float float float float
	- emissive
Tf float float float
	(r, g, b) values
Tf xyz float float float
	CIEXYZ (x, y, z) values
	- transmission filter
map_Kd fol_tree_pinebranch01.tga
	- texture file name
N 1.00
Ni 1.00
	- shininess

*/

// preprocessor directives

#include "core/core.h"
#include "asgard/asgard.h"
#define _ASGARD_ENGINE_PLUGIN_COMPONENT
#include "asgard/engine/plugin/wfobj.h"


// define wfobj private constants

typedef enum _WfobjLineTypes {
	TYPE_COMMENT = 1,
	TYPE_MATERIAL_LIBRARY,
	TYPE_OBJECT_NAME,
	TYPE_VERTEX,
	TYPE_TEXTURE,
	TYPE_NORMAL,
	TYPE_SMOOTHING,
	TYPE_MATERIAL_NAME,
	TYPE_FACE,
	TYPE_UNKNOWN = -1
} WfobjLineTypes;

typedef enum _WfobjMaterialTypes {
	MAT_TYPE_COMMENT = 1,
	MAT_TYPE_NEW,
	MAT_TYPE_ILLUM,
	MAT_TYPE_DIFFUSION,
	MAT_TYPE_AMBIENT,
	MAT_TYPE_SPECULAR,
	MAT_TYPE_EMISSIVE,
	MAT_TYPE_TRANS_FILTER,
	MAT_TYPE_FILENAME,
	MAT_TYPE_SHINY,
	MAT_TYPE_UNKNOWN = -1
} WfobjMaterialTypes;


// define wfobj private macros

#define fileError(code) \
	_handleFileError(code, log, &fh, __FUNCTION__, __LINE__)


// define wfobj private functions

static int stringClean(char *string, int length)
{
	aboolean foundFirstChar = afalse;
	int ii = 0;
	int nn = 0;
	
	if((string == NULL) | (length < 0)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(length == 0) {
		string[0] = '\0';
		return 0;
	}

	for(ii = 0, nn = 0; ii < length; ii++) {
		if((ctype_ctoi(string[ii]) < 32) ||
				(ctype_ctoi(string[ii]) > 126)) {
			continue;
		}
		else if(!foundFirstChar) {
			if(string[ii] == ' ') {
				continue;
			}
			else {
				foundFirstChar = atrue;
			}
		}

		string[nn] = string[ii];
		nn++;
	}

	string[nn] = '\0';
	length = nn;

	for(ii = (length - 1); ii >= 0; ii--) {
		if((ctype_ctoi(string[ii]) < 32) ||
				(ctype_ctoi(string[ii]) > 126)) {
			string[ii] = '\0';
		}
		else if(string[ii] == ' ') {
			string[ii] = '\0';
		}
		else {
			break;
		}
	}

	return (ii + 1);
}

aboolean stringDelimiterParse(char *string, int start, int length,
		char delimiter, char buffer[WFOBJ_READ_LINE_LENGTH], int *bufferLength)
{
	int ii = 0;
	int nn = 0;

	if((string == NULL) || (start < 0) || (length < 0)) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	*bufferLength = 0;

	if(start > length) {
		return afalse;
	}

	for(ii = start, nn = 0; ii < length; ii++, nn++) {
		if(string[ii] == delimiter) {
			break;
		}
		buffer[nn] = string[ii];
	}

	buffer[nn] = '\0';
	*bufferLength = nn;

	return atrue;
}

static int _handleFileError(int code, Log *log, FileHandle *handle,
		const char *function, size_t line)
{
	char *errorMessage = NULL;

	if(code != 0) {
		file_getError(handle, code, &errorMessage);
	}

	if(errorMessage == NULL) {
		errorMessage = strdup("unknown error");
	}

	log_logf(log, LOG_LEVEL_ERROR, "[%s():%i] (%i) - '%s'",
			(char *)function, (int)line, code, errorMessage);

	if(errorMessage != NULL) {
		free(errorMessage);
	}

	return -1;
}

static char *parseFilepath(char *filename)
{
	int ii = 0;
	int length = 0;
	char *result = NULL;

	result = strdup(filename);
	length = stringClean(result, strlen(result));

	for(ii = (length - 1); ii >= 0; ii--) {
		if((result[ii] == '/') || (result[ii] == '\\')) {
			result[ii] = '\0';
			break;
		}
		result[ii] = '\0';
	}

	return result;
}

static int getLineType(char *line, int length)
{
	if((line[0] != 's') && (length < 4)) {
		return TYPE_UNKNOWN;
	}
	else if((line[0] == 's') && (length < 3)) {
		return TYPE_UNKNOWN;
	}

	if(line[0] == '#') {
		return TYPE_COMMENT;
	}
	else if(line[0] == 'm') {
		if(length < 8) {
			return TYPE_UNKNOWN;
		}
		else if((line[1] == 't') && (line[2] == 'l') && (line[3] == 'l') &&
				(line[4] == 'i') && (line[5] == 'b') && (line[6] == ' ')) {
			return TYPE_MATERIAL_LIBRARY;
		}
	}
	else if(line[0] == 'g') {
		if(line[1] == ' ') {
			return TYPE_OBJECT_NAME;
		}
	}
	else if(line[0] == 'v') {
		if(line[1] == ' ') {
			return TYPE_VERTEX;
		}
		else if((line[1] == 't') && (line[2] == ' ')) {
			return TYPE_TEXTURE;
		}
		else if((line[1] == 'n') && (line[2] == ' ')) {
			return TYPE_NORMAL;
		}
	}
	else if(line[0] == 's') {
		if(line[1] == ' ') {
			return TYPE_SMOOTHING;
		}
	}
	else if(line[0] == 'u') {
		if(length < 8) {
			return TYPE_UNKNOWN;
		}
		else if((line[1] == 's') && (line[2] == 'e') && (line[3] == 'm') &&
				(line[4] == 't') && (line[5] == 'l') && (line[6] == ' ')) {
			return TYPE_MATERIAL_NAME;
		}
	}
	else if(line[0] == 'f') {
		if(line[1] == ' ') {
			return TYPE_FACE;
		}
	}

	return TYPE_UNKNOWN;
}

static int parseIntegerField(char *line, int length)
{
	aboolean isFirstToken = afalse;
	int ref = 0;
	int result = 0;
	int bufferLength = 0;
	char buffer[WFOBJ_READ_LINE_LENGTH];

	while(stringDelimiterParse(line, ref, length, ' ', buffer,
				&bufferLength)) {
		if(!isFirstToken) {
			isFirstToken = atrue;
		}
		else {
			result = atoi(buffer);
			break;
		}
		ref += (bufferLength + 1);
	}

	return result;
}

static float parseFloatField(char *line, int length)
{
	aboolean isFirstToken = afalse;
	int ref = 0;
	int bufferLength = 0;
	float result = 0.0;
	char buffer[WFOBJ_READ_LINE_LENGTH];

	while(stringDelimiterParse(line, ref, length, ' ', buffer,
				&bufferLength)) {
		if(!isFirstToken) {
			isFirstToken = atrue;
		}
		else {
			result = (float)atod(buffer);
			break;
		}
		ref += (bufferLength + 1);
	}

	return result;
}

static int parse2xFloatField(char *line, int length, float array[2])
{
	aboolean isFirstToken = afalse;
	int ref = 0;
	int pos = 0;
	int bufferLength = 0;
	char buffer[WFOBJ_READ_LINE_LENGTH];

	while(stringDelimiterParse(line, ref, length, ' ', buffer,
				&bufferLength)) {
		if(!isFirstToken) {
			isFirstToken = atrue;
		}
		else {
			if(pos > 1) {
				break;
			}
			array[pos] = (float)atod(buffer);
			pos++;
		}
		ref += (bufferLength + 1);
	}

	if(pos != 2) {
		return -1;
	}

	return 0;
}

static int parse3xFloatField(char *line, int length, float array[3])
{
	aboolean isFirstToken = afalse;
	int ref = 0;
	int pos = 0;
	int bufferLength = 0;
	char buffer[WFOBJ_READ_LINE_LENGTH];

	while(stringDelimiterParse(line, ref, length, ' ', buffer,
				&bufferLength)) {
		if(!isFirstToken) {
			isFirstToken = atrue;
		}
		else {
			if(pos > 2) {
				break;
			}
			array[pos] = (float)atod(buffer);
			pos++;
		}
		ref += (bufferLength + 1);
	}

	if(pos != 3) {
		return -1;
	}

	return 0;
}

static int parse4xFloatField(char *line, int length, float array[4])
{
	aboolean isFirstToken = afalse;
	int ref = 0;
	int pos = 0;
	int bufferLength = 0;
	char buffer[WFOBJ_READ_LINE_LENGTH];

	while(stringDelimiterParse(line, ref, length, ' ', buffer,
				&bufferLength)) {
		if(!isFirstToken) {
			isFirstToken = atrue;
		}
		else {
			if(pos > 3) {
				break;
			}
			array[pos] = (float)atod(buffer);
			pos++;
		}
		ref += (bufferLength + 1);
	}

	if(pos != 4) {
		return -1;
	}

	return 0;
}

static char *parseStringField(int start, char *line, int length)
{
	char *result = NULL;
	int bufferLength = 0;
	char buffer[WFOBJ_READ_LINE_LENGTH];

	if(!stringDelimiterParse(line, start, length, ' ', buffer,
				&bufferLength)) {
		return NULL;
	}

	result = strndup(buffer, bufferLength);

	if(stringClean(result, bufferLength) < 1) {
		free(result);
		return NULL;
	}

	return result;
}

static char *parseObjectName(char *line, int length)
{
	aboolean isFirstToken = afalse;
	int ref = 0;
	int result_ref = 0;
	int result_length = 0;
	int bufferLength = 0;
	char buffer[WFOBJ_READ_LINE_LENGTH];
	char *result = NULL;

	result_ref = 0;
	result_length = 8;
	result = (char *)malloc(sizeof(char) * result_length);

	while(stringDelimiterParse(line, ref, length, ' ', buffer,
				&bufferLength)) {
		if(!isFirstToken) {
			isFirstToken = atrue;
		}
		else {
			if((result_ref + bufferLength + 2) >= result_length) {
				result_length *= 2;
				result_length += bufferLength;
				result = (char *)realloc(result, sizeof(char) * result_length);
			}
			if(result_ref > 0) {
				strcat(result, "/");
			}
			strcat(result, buffer);
			result_ref += (bufferLength + 1);
		}
		ref += (bufferLength + 1);
	}

	return result;
}

static WfobjVertex3d *newVertex(char *line, int length)
{
	float array[3];

	WfobjVertex3d *result = NULL;

	if(parse3xFloatField(line, length, array) < 0) {
		return NULL;
	}

	result = (WfobjVertex3d *)malloc(sizeof(WfobjVertex3d));

	result->isNormalized = afalse;
	result->x = array[0];
	result->y = array[1];
	result->z = array[2];

	return result;
}

static WfobjTexture3d *newTexture(char *line, int length)
{
	float array[3];

	WfobjTexture3d *result = NULL;

	if(parse2xFloatField(line, length, array) < 0) {
		return NULL;
	}

	result = (WfobjTexture3d *)malloc(sizeof(WfobjTexture3d));

	result->u = array[0];
	result->v = array[1];

	return result;
}

static WfobjNormal3d *newNormal(char *line, int length)
{
	float array[3];

	WfobjNormal3d *result = NULL;

	if(parse3xFloatField(line, length, array) < 0) {
		return NULL;
	}

	result = (WfobjNormal3d *)malloc(sizeof(WfobjNormal3d));

	result->x = array[0];
	result->y = array[1];
	result->z = array[2];

	return result;
}

static int getMaterialLineType(char *line, int length)
{
	if(length < 4) {
		return MAT_TYPE_UNKNOWN;
	}

	if(line[0] == '#') {
		return MAT_TYPE_COMMENT;
	}
	else if(line[0] == 'n') {
		if(length < 8) {
			return MAT_TYPE_UNKNOWN;
		}
		else if((line[1] == 'e') && (line[2] == 'w') && (line[3] == 'm') &&
				(line[4] == 't') && (line[5] == 'l') && (line[6] == ' ')) {
			return MAT_TYPE_NEW;
		}
	}
	else if(line[0] == 'i') {
		if(length < 7) {
			return MAT_TYPE_UNKNOWN;
		}
		else if((line[1] == 'l') && (line[2] == 'l') && (line[3] == 'u') &&
				(line[4] == 'm') && (line[5] == ' ')) {
			return MAT_TYPE_ILLUM;
		}
	}
	else if(line[0] == 'K') {
		if((line[1] == 'd') && (line[2] == ' ')) {
			return MAT_TYPE_DIFFUSION;
		}
		else if((line[1] == 'a') && (line[2] == ' ')) {
			return MAT_TYPE_AMBIENT;
		}
		else if((line[1] == 's') && (line[2] == ' ')) {
			return MAT_TYPE_SPECULAR;
		}
		else if((line[1] == 'e') && (line[2] == ' ')) {
			return MAT_TYPE_EMISSIVE;
		}
	}
	else if(line[0] == 'T') {
		if((line[1] == 'f') && (line[2] == ' ')) {
			return MAT_TYPE_TRANS_FILTER;
		}
	}
	else if(line[0] == 'm') {
		if(length < 8) {
			return MAT_TYPE_UNKNOWN;
		}
		else if((line[1] == 'a') && (line[2] == 'p') && (line[3] == '_') &&
				(line[4] == 'K') && (line[5] == 'd') && (line[6] == ' ')) {
			return MAT_TYPE_FILENAME;
		}
	}
	else if(line[0] == 'N') {
		if(line[1] == ' ') {
			return MAT_TYPE_SHINY;
		}
		else if((line[1] == 'i') && (line[2] == ' ')) {
			return MAT_TYPE_SHINY;
		}
	}

	return MAT_TYPE_UNKNOWN;
}

static WfobjMaterial3d *newMaterial(char *line, int length)
{
	char *name = NULL;

	WfobjMaterial3d *result = NULL;

	name = parseStringField(7, line, length);
	if(name == NULL) {
		return NULL;
	}

	result = (WfobjMaterial3d *)malloc(sizeof(WfobjMaterial3d));

	result->name = name;
	result->filename = NULL;
	result->illumination_model = 4;
	result->shininess = 8.0f;

	result->diffuse[0] = 0.8f;
	result->diffuse[1] = 0.8f;
	result->diffuse[2] = 0.8f;
	result->diffuse[3] = 1.0f;

	result->ambient[0] = 0.8f;
	result->ambient[1] = 0.8f;
	result->ambient[2] = 0.8f;
	result->ambient[3] = 1.0f;

	result->specular[0] = 0.8f;
	result->specular[1] = 0.8f;
	result->specular[2] = 0.8f;
	result->specular[3] = 1.0f;

	result->emissive[0] = 0.8f;
	result->emissive[1] = 0.8f;
	result->emissive[2] = 0.8f;
	result->emissive[3] = 1.0f;

	result->filter[0] = 1.0f;
	result->filter[1] = 1.0f;
	result->filter[2] = 1.0f;

	return result;
}

static void process_material_line(Wfobj *obj, char *path, char *line,
		int length)
{
	int type = 0;
	int name_length = 0;
	char *name = NULL;

	WfobjMaterial3d *material = NULL;

	static int ref = -1;

	type = getMaterialLineType(line, length);
	if(type == MAT_TYPE_UNKNOWN) {
		return;
	}

	switch(type) {
		case MAT_TYPE_COMMENT:
			break;
		case MAT_TYPE_NEW:
			material = newMaterial(line, length);
			if(material == NULL) {
				break;
			}
			ref = obj->materialCount;
			obj->materialCount += 1;
			if(obj->materials == NULL) {
				obj->materials = (WfobjMaterial3d **)malloc(
						sizeof(WfobjMaterial3d *) * obj->materialCount);
			}
			else {
				obj->materials = (WfobjMaterial3d **)realloc(obj->materials,
						(sizeof(WfobjMaterial3d *) * obj->materialCount));
			}
			obj->materials[ref] = material;
			break;
		case MAT_TYPE_ILLUM:
			if(ref < 0) {
				fprintf(stderr, "[%s():%i] error - unable to assign %i "
						"without a material referent.\n", __FUNCTION__,
						__LINE__, type);
				break;
			}
			obj->materials[ref]->illumination_model = parseIntegerField(line,
					length);
			break;
		case MAT_TYPE_DIFFUSION:
			if(ref < 0) {
				fprintf(stderr, "[%s():%i] error - unable to assign %i "
						"without a material referent.\n", __FUNCTION__,
						__LINE__, type);
				break;
			}
			parse4xFloatField(line, length, obj->materials[ref]->diffuse);
			break;
		case MAT_TYPE_AMBIENT:
			if(ref < 0) {
				fprintf(stderr, "[%s():%i] error - unable to assign %i "
						"without a material referent.\n", __FUNCTION__,
						__LINE__, type);
				break;
			}
			parse4xFloatField(line, length, obj->materials[ref]->ambient);
			break;
		case MAT_TYPE_SPECULAR:
			if(ref < 0) {
				fprintf(stderr, "[%s():%i] error - unable to assign %i "
						"without a material referent.\n", __FUNCTION__,
						__LINE__, type);
				break;
			}
			parse4xFloatField(line, length, obj->materials[ref]->specular);
			break;
		case MAT_TYPE_EMISSIVE:
			if(ref < 0) {
				fprintf(stderr, "[%s():%i] error - unable to assign %i "
						"without a material referent.\n", __FUNCTION__,
						__LINE__, type);
				break;
			}
			parse4xFloatField(line, length, obj->materials[ref]->emissive);
			break;
		case MAT_TYPE_TRANS_FILTER:
			if(ref < 0) {
				fprintf(stderr, "[%s():%i] error - unable to assign %i "
						"without a material referent.\n", __FUNCTION__,
						__LINE__, type);
				break;
			}
			parse3xFloatField(line, length, obj->materials[ref]->filter);
			break;
		case MAT_TYPE_FILENAME:
			if(ref < 0) {
				fprintf(stderr, "[%s():%i] error - unable to assign %i "
						"without a material referent.\n", __FUNCTION__,
						__LINE__, type);
				break;
			}
			if(obj->materials[ref]->filename != NULL) {
				free(obj->materials[ref]->filename);
			}
			name = parseStringField(7, line, length);
			if(name != NULL) {
				name_length = (strlen(name) + strlen(path) + 8);
				obj->materials[ref]->filename = (char *)malloc(sizeof(char) *
						name_length);
				if(strlen(path) > 0) {
					snprintf(obj->materials[ref]->filename, name_length,
							"%s/%s", path, name);
				}
				else {
					snprintf(obj->materials[ref]->filename, name_length, "%s",
							name);
				}
				free(name);
			}
			break;
		case MAT_TYPE_SHINY:
			if(ref < 0) {
				fprintf(stderr, "[%s():%i] error - unable to assign %i "
						"without a material referent.\n", __FUNCTION__,
						__LINE__, type);
				break;
			}
			obj->materials[ref]->shininess = parseFloatField(line, length);
			break;
		default:
			obj->lineErrorCount += 1;
			return;
	}
}

static void processMaterialLibrary(Wfobj *obj, char *path, char *filename)
{
	int rc = 0;
	int length = 0;
	char line[WFOBJ_READ_LINE_LENGTH];

	FileHandle fh;

	Log *log = (Log *)stdout;

	if((rc = file_init(&fh, filename, "r", WFOBJ_READ_LINE_LENGTH)) < 0) {
		fileError(rc);
		return;
	}

	rc = file_readLine(&fh, line, WFOBJ_READ_LINE_LENGTH, &length);
	while(rc == 0) {
		length = stringClean(line, length);

		if(length > 0) {
			process_material_line(obj, path, line, length);
		}

		rc = file_readLine(&fh, line, WFOBJ_READ_LINE_LENGTH, &length);
	}

	file_free(&fh);
}

static int lookupMaterial(Wfobj *obj, char *name)
{
	int ii = 0;

	for(ii = 0; ii < obj->materialCount; ii++) {
		if(!strcmp(obj->materials[ii]->name, name)) {
			return ii;
		}
	}

	return -1;
}

static int lookupObject(Wfobj *obj, char *name)
{
	int ii = 0;

	for(ii = 0; ii < obj->objectCount; ii++) {
		if(!strcmp(obj->objects[ii]->name, name)) {
			return ii;
		}
	}

	return -1;
}

static int getFaceType(char *line, int length)
{
	int ii = 0;
	int counter = 0;
	int token_length = 0;
	char *token = NULL;

	token = parseStringField(2, line, length);
	if(token == NULL) {
		return WFOBJ_OBJ_TYPE_UNKNOWN;
	}

	token_length = strlen(token);
	for(ii = 0; ii < token_length; ii++) {
		if(token[ii] == '/') {
			counter++;
		}
	}
	free(token);

	if(counter == 0) {
		return WFOBJ_OBJ_TYPE_V;
	}
	else if(counter == 1) {
		return WFOBJ_OBJ_TYPE_VT;
	}
	else if(counter == 2) {
		return WFOBJ_OBJ_TYPE_VTN;
	}

	return WFOBJ_OBJ_TYPE_UNKNOWN;
}

static WfobjObject3d *newObject(char *line, int length)
{
	char *name = NULL;

	WfobjObject3d *result = NULL;

	name = parseObjectName(line, length);
	if(name == NULL) {
		return NULL;
	}

	result = (WfobjObject3d *)malloc(sizeof(WfobjObject3d));

	result->isListed = afalse;
	result->listId = -1;
	result->type = WFOBJ_OBJ_TYPE_UNKNOWN;
	result->material = -1;
	result->name = name;
	result->smoothing = NULL;
	result->points = 0;
	result->faces = 0;
	result->types = NULL;
	result->vertexCount = 0;
	result->vertexErrorCount = 0;
	result->verticies = NULL;
	result->textureCount = 0;
	result->textureErrorCount = 0;
	result->textures = NULL;
	result->normalCount = 0;
	result->normalErrorCount = 0;
	result->normals = NULL;
	result->renderPolygonCount = -1;

	return result;
}

static int parseFaceFieldParameter(int pos, char *string, int length)
{
	int ii = 0;
	int nn = 0;
	int location = 0;
	int error = 0;
	int result = 0;
	char temp[128];

	memset(temp, 0, sizeof(temp));

	for(ii = 0, nn = 0; ii < length; ii++) {
		if(string[ii] == '/') {
			location++;
		}
		else if(pos == location) {
			temp[nn] = string[ii];
			nn++;
			if(nn >= 126) {
				break;
			}
		}
		else if(location > pos) {
			break;
		}
	}

	result = atoi(temp);
	if(error < 0) {
		return 1;
	}

	return result;
}

static void addTypeToObject(Wfobj *obj, int id, int type)
{
	int ref = 0;

	ref = obj->objects[id]->faces;
	obj->objects[id]->faces++;
	if(obj->objects[id]->types == NULL) {
		obj->objects[id]->types = (int *)malloc(sizeof(int) *
				obj->objects[id]->faces);
	}
	else {
		obj->objects[id]->types = (int *)realloc(
				obj->objects[id]->types,
				(sizeof(int) * obj->objects[id]->faces));
	}

	obj->objects[id]->types[ref] = type;
}

static void addVertexToObject(Wfobj *obj, int id, int vertex)
{
	int ref = 0;

	ref = obj->objects[id]->vertexCount;
	obj->objects[id]->vertexCount += 1;
	if(obj->objects[id]->verticies == NULL) {
		obj->objects[id]->verticies = (int *)malloc(sizeof(int) *
				obj->objects[id]->vertexCount);
	}
	else {
		obj->objects[id]->verticies = (int *)realloc(
				obj->objects[id]->verticies,
				(sizeof(int) * obj->objects[id]->vertexCount));
	}

	obj->objects[id]->verticies[ref] = vertex;
}

static void addTextureToObject(Wfobj *obj, int id, int texture)
{
	int ref = 0;

	ref = obj->objects[id]->textureCount;
	obj->objects[id]->textureCount += 1;
	if(obj->objects[id]->textures == NULL) {
		obj->objects[id]->textures = (int *)malloc(sizeof(int) *
				obj->objects[id]->textureCount);
	}
	else {
		obj->objects[id]->textures = (int *)realloc(
				obj->objects[id]->textures,
				(sizeof(int) * obj->objects[id]->textureCount));
	}

	obj->objects[id]->textures[ref] = texture;
}

static void addNormalToObject(Wfobj *obj, int id, int normal)
{
	int ref = 0;

	ref = obj->objects[id]->normalCount;
	obj->objects[id]->normalCount += 1;
	if(obj->objects[id]->normals == NULL) {
		obj->objects[id]->normals = (int *)malloc(sizeof(int) *
				obj->objects[id]->normalCount);
	}
	else {
		obj->objects[id]->normals = (int *)realloc(
				obj->objects[id]->normals,
				(sizeof(int) * obj->objects[id]->normalCount));
	}

	obj->objects[id]->normals[ref] = normal;
}

static void processFace(Wfobj *obj, int id, char *line, int length)
{
	aboolean isFirstToken = afalse;
	int ref = 0;
	int type = 0;
	int counter = 0;
	int param = 0;
	int bufferLength = 0;
	char buffer[WFOBJ_READ_LINE_LENGTH];

	type = getFaceType(line, length);
	if(type == WFOBJ_OBJ_TYPE_UNKNOWN) {
		return;
	}
	if(obj->objects[id]->type == WFOBJ_OBJ_TYPE_UNKNOWN) {
		obj->objects[id]->type = type;
	}
	else {
		if(type != obj->objects[id]->type) {
			return;
		}
	}

	ref = 0;
	while(stringDelimiterParse(line, ref, length, ' ', buffer,
				&bufferLength)) {
		if(!isFirstToken) {
			isFirstToken = atrue;
		}
		else {
			if((type == WFOBJ_OBJ_TYPE_V) || (type == WFOBJ_OBJ_TYPE_VT) ||
					(type == WFOBJ_OBJ_TYPE_VTN)) {
				param = parseFaceFieldParameter(0, buffer, bufferLength);
				if(param >= 0) {
					addVertexToObject(obj, id, (param - 1));
				}
				else {
					addVertexToObject(obj, id,
							(obj->vertexCount + param));
				}
			}
			if((type == WFOBJ_OBJ_TYPE_VT) ||
					(type == WFOBJ_OBJ_TYPE_VTN)) {
				param = parseFaceFieldParameter(1, buffer, bufferLength);
				if(param >= 0) {
					addTextureToObject(obj, id, (param - 1));
				}
				else {
					addTextureToObject(obj, id,
							(obj->textureCount + param));
				}
			}
			if(type == WFOBJ_OBJ_TYPE_VTN) {
				param = parseFaceFieldParameter(2, buffer, bufferLength);
				if(param >= 0) {
					addNormalToObject(obj, id, (param - 1));
				}
				else {
					addNormalToObject(obj, id,
							(obj->normalCount + param));
				}
			}
			obj->objects[id]->points += 1;
			counter++;
		}
		ref += (bufferLength + 1);
	}

	if(counter == 3) {
		addTypeToObject(obj, id, WFOBJ_FACE_TYPE_TRIANGLES);
	}
	else if(counter == 4) {
		addTypeToObject(obj, id, WFOBJ_FACE_TYPE_QUADS);
	}
	else if(counter > 4) {
		addTypeToObject(obj, id, counter);
	}
	else {
		addTypeToObject(obj, id, WFOBJ_FACE_TYPE_UNKNOWN);
	}
}

static void processLine(Wfobj *obj, int flag, char *path, char *line,
		int length)
{
	int ref = 0;
	int type = 0;
	int name_length = 0;
	char *temp = NULL;

	WfobjVertex3d *vertex = NULL;
	WfobjTexture3d *texture = NULL;
	WfobjNormal3d *normal = NULL;
	WfobjObject3d *object = NULL;

	static aboolean first_pass = atrue;
	static aboolean found_object_first = afalse;
	static int current_object_id = -1;
	static char *current_material_library = NULL;
	static char *current_object_name = NULL;
	static char *current_material_name = NULL;

	obj->lineCount += 1;

	type = getLineType(line, length);

	if(type == TYPE_UNKNOWN) {
		obj->lineErrorCount += 1;
		return;
	}

	switch(type) {
		case TYPE_COMMENT:
			obj->commentLineCount += 1;
			break;
		case TYPE_MATERIAL_LIBRARY:
			if(current_material_library != NULL) {
				free(current_material_library);
			}
			temp = parseStringField(7, line, length);
			if(temp != NULL) {
				name_length = (strlen(temp) + strlen(path) + 8);
				current_material_library = (char *)malloc(
						sizeof(char) * name_length);
				if(strlen(path) > 0) {
					snprintf(current_material_library, name_length, "%s/%s",
							path, temp);
				}
				else {
					snprintf(current_material_library, name_length, "%s",
							temp);
				}
				free(temp);
				if(!flag) {
					return;
				}
				if(obj->materialFilename == NULL) {
					obj->materialFilename = strdup(current_material_library);
				}
				processMaterialLibrary(obj, path, current_material_library);
			}
			break;
		case TYPE_OBJECT_NAME:
			if(first_pass) {
				if(current_material_name == NULL) {
					found_object_first = atrue;
				}
				first_pass = afalse;
			}
			if(current_object_name != NULL) {
				free(current_object_name);
			}
			current_object_name = parseObjectName(line, length);
			ref = lookupObject(obj, current_object_name);
			if(ref == -1) {
				current_object_id = -1;
				object = newObject(line, length);
				if(object != NULL) {
					if(!found_object_first) {
						ref = -1;
						if(current_material_name != NULL) {
							ref = lookupMaterial(obj, current_material_name);
						}
						if(ref != -1) {
							object->material = ref;
						}
					}
					ref = obj->objectCount;
					obj->objectCount += 1;
					if(obj->objects == NULL) {
						obj->objects = (WfobjObject3d **)malloc(
								sizeof(WfobjObject3d *) * obj->objectCount);
					}
					else {
						obj->objects = (WfobjObject3d **)realloc(
								obj->objects, (sizeof(WfobjObject3d *) *
									obj->objectCount));
					}
					obj->objects[ref] = object;
					current_object_id = ref;
				}
				else {
					obj->lineErrorCount += 1;
				}
			}
			else {
				current_object_id = ref;
			}
			break;
		case TYPE_VERTEX:
			vertex = newVertex(line, length);
			if(vertex != NULL) {
				ref = obj->vertexCount;
				obj->vertexCount += 1;
				if(obj->verticies == NULL) {
					obj->verticies = (WfobjVertex3d **)malloc(
							sizeof(WfobjVertex3d *) * obj->vertexCount);
				}
				else {
					obj->verticies = (WfobjVertex3d **)realloc(
							obj->verticies,
							(sizeof(WfobjVertex3d *) * obj->vertexCount));
				}
				obj->verticies[ref] = vertex;
			}
			else {
				obj->lineErrorCount += 1;
			}
			break;
		case TYPE_TEXTURE:
			texture = newTexture(line, length);
			if(texture != NULL) {
				ref = obj->textureCount;
				obj->textureCount += 1;
				if(obj->textures == NULL) {
					obj->textures = (WfobjTexture3d **)malloc(
							sizeof(WfobjTexture3d *) * obj->textureCount);
				}
				else {
					obj->textures = (WfobjTexture3d **)realloc(
							obj->textures,
							(sizeof(WfobjTexture3d *) * obj->textureCount));
				}
				obj->textures[ref] = texture;
			}
			else {
				obj->lineErrorCount += 1;
			}
			break;
		case TYPE_NORMAL:
			normal = newNormal(line, length);
			if(normal != NULL) {
				ref = obj->normalCount;
				obj->normalCount += 1;
				if(obj->normals == NULL) {
					obj->normals = (WfobjNormal3d **)malloc(
							sizeof(WfobjNormal3d *) * obj->normalCount);
				}
				else {
					obj->normals = (WfobjNormal3d **)realloc(
							obj->normals,
							(sizeof(WfobjNormal3d *) * obj->normalCount));
				}
				obj->normals[ref] = normal;
			}
			else {
				obj->lineErrorCount += 1;
			}
			break;
		case TYPE_SMOOTHING:
			temp = parseStringField(2, line, length);
			if(temp != NULL) {
				if(current_object_name != NULL) {
					if(current_object_id < 0) {
						ref = lookupObject(obj, current_object_name);
						if(ref < 0) {
							obj->lineErrorCount += 1;
						}
						else {
							current_object_id = ref;
						}
					}
					if(current_object_id >= 0) {
						obj->objects[current_object_id]->smoothing = strdup(
								temp);
					}
				}
				else {
					obj->lineErrorCount += 1;
				}
				free(temp);
			}
			else {
				obj->lineErrorCount += 1;
			}
			break;
		case TYPE_MATERIAL_NAME:
			if(current_material_name != NULL) {
				free(current_material_name);
			}
			current_material_name = parseStringField(7, line, length);
			if((current_material_name != NULL) && (found_object_first) &&
					(current_object_id >= 0) &&
					(obj->objects[current_object_id]->material == -1)) {
				ref = lookupMaterial(obj, current_material_name);
				if(ref != -1) {
					obj->objects[current_object_id]->material = ref;
				}
			}
			break;
		case TYPE_FACE:
			if(current_object_name != NULL) {
				if(current_object_id < 0) {
					ref = lookupObject(obj, current_object_name);
					if(ref < 0) {
						obj->lineErrorCount += 1;
					}
					else {
						current_object_id = ref;
					}
				}
				else {
					ref = 0;
				}
				if((ref >= 0) && (current_object_id >= 0)) {
					processFace(obj, current_object_id, line, length);
				}
			}
			else {
				obj->lineErrorCount += 1;
			}
			break;
		default:
			obj->lineErrorCount += 1;
			return;
	}
}

static void perform_normalization(Wfobj *obj, int object)
{
	int ii = 0;
	int nn = 0;
	int jj = 0;
	int ref = 0;
	double x = 0.0;
	double y = 0.0;
	double z = 0.0;

	x = MAX_SIGNED_DOUBLE;
	y = MAX_SIGNED_DOUBLE;
	z = MAX_SIGNED_DOUBLE;

	for(ii = 0, nn = 0; ii < obj->objects[object]->faces; ii++) {
		for(jj = 0; jj < obj->objects[object]->types[ii]; jj++, nn++) {
			if(((obj->objects[object]->type == WFOBJ_OBJ_TYPE_V) ||
				(obj->objects[object]->type == WFOBJ_OBJ_TYPE_VT) ||
				(obj->objects[object]->type == WFOBJ_OBJ_TYPE_VTN)) &&
			(nn < obj->objects[object]->vertexCount)) {
				ref = obj->objects[object]->verticies[nn];
				if((ref >= 0) && (ref < obj->vertexCount)) {
					if((double)obj->verticies[ref]->x < x) {
						x = (double)obj->verticies[ref]->x;
					}
					if((double)obj->verticies[ref]->y < y) {
						y = (double)obj->verticies[ref]->y;
					}
					if((double)obj->verticies[ref]->z < z) {
						z = (double)obj->verticies[ref]->z;
					}
				}
			}
		}
	}

	if(x == MAX_SIGNED_DOUBLE) {
		x = 0.0;
	}
	if(y == MAX_SIGNED_DOUBLE) {
		y = 0.0;
	}
	if(z == MAX_SIGNED_DOUBLE) {
		z = 0.0;
	}

	for(ii = 0, nn = 0; ii < obj->objects[object]->faces; ii++) {
		for(jj = 0; jj < obj->objects[object]->types[ii]; jj++, nn++) {
			if(((obj->objects[object]->type == WFOBJ_OBJ_TYPE_V) ||
				(obj->objects[object]->type == WFOBJ_OBJ_TYPE_VT) ||
				(obj->objects[object]->type == WFOBJ_OBJ_TYPE_VTN)) &&
			(nn < obj->objects[object]->vertexCount)) {
				ref = obj->objects[object]->verticies[nn];
				if((ref >= 0) && (ref < obj->vertexCount) &&
						(!obj->verticies[ref]->isNormalized)) {
					obj->verticies[ref]->isNormalized = atrue;
					obj->verticies[ref]->x -= (float)x;
					obj->verticies[ref]->y -= (float)y;
					obj->verticies[ref]->z -= (float)z;
				}
			}
		}
	}
}

static void calculate_bounding_sphere(Wfobj *obj, int object)
{
	int ii = 0;
	int nn = 0;
	int jj = 0;
	int ref = 0;
	int counter = 0;
	double distance = 0.0;

	WfobjBoundingSphere3d *sphere = NULL;

//	perform_normalization(obj, object);

	sphere = &obj->objects[object]->sphere;

	sphere->radius = 0.0;
	sphere->render = NULL;

	vector3d_init(&sphere->center);

	for(ii = 0, nn = 0; ii < obj->objects[object]->faces; ii++) {
		for(jj = 0; jj < obj->objects[object]->types[ii]; jj++, nn++) {
			if(((obj->objects[object]->type == WFOBJ_OBJ_TYPE_V) ||
				(obj->objects[object]->type == WFOBJ_OBJ_TYPE_VT) ||
				(obj->objects[object]->type == WFOBJ_OBJ_TYPE_VTN)) &&
			(nn < obj->objects[object]->vertexCount)) {
				ref = obj->objects[object]->verticies[nn];
				if((ref >= 0) && (ref < obj->vertexCount)) {
					sphere->center.x += obj->verticies[ref]->x;
					sphere->center.y += obj->verticies[ref]->y;
					sphere->center.z += obj->verticies[ref]->z;
					counter++;
				}
			}
		}
	}

	sphere->center.x /= (double)counter;
	sphere->center.y /= (double)counter;
	sphere->center.z /= (double)counter;

	for(ii = 0, nn = 0; ii < obj->objects[object]->faces; ii++) {
		for(jj = 0; jj < obj->objects[object]->types[ii]; jj++, nn++) {
			if(((obj->objects[object]->type == WFOBJ_OBJ_TYPE_V) ||
				(obj->objects[object]->type == WFOBJ_OBJ_TYPE_VT) ||
				(obj->objects[object]->type == WFOBJ_OBJ_TYPE_VTN)) &&
			(nn < obj->objects[object]->vertexCount)) {
				ref = obj->objects[object]->verticies[nn];
				if((ref >= 0) && (ref < obj->vertexCount)) {
					distance = sqrt(
							((obj->verticies[ref]->x - sphere->center.x) *
							 (obj->verticies[ref]->x - sphere->center.x)) +
							((obj->verticies[ref]->y - sphere->center.y) *
							 (obj->verticies[ref]->y - sphere->center.y)) +
							((obj->verticies[ref]->z - sphere->center.z) *
							 (obj->verticies[ref]->z - sphere->center.z)));
					if(distance > sphere->radius) {
						sphere->radius = distance;
					}
				}
			}
		}
	}

	sphere->render = (void *)sphere3d_new(16, 16, sphere->radius,
			sphere->center.x, sphere->center.y, sphere->center.z);
}

static void constructBoundingBox(Wfobj *obj)
{
	int ii = 0;

	obj->boundingBox.high.x = 0.0;
	obj->boundingBox.high.y = 0.0;
	obj->boundingBox.high.z = 0.0;

	obj->boundingBox.low.x = MAX_SIGNED_DOUBLE;
	obj->boundingBox.low.y = MAX_SIGNED_DOUBLE;
	obj->boundingBox.low.z = MAX_SIGNED_DOUBLE;

	for(ii = 0; ii < obj->vertexCount; ii++) {
		if((obj->verticies[ii]->x * obj->renderScale) >
				obj->boundingBox.high.x) {
			obj->boundingBox.high.x = (obj->verticies[ii]->x *
					obj->renderScale);
		}
		if((obj->verticies[ii]->x * obj->renderScale) <
				obj->boundingBox.low.x) {
			obj->boundingBox.low.x = (obj->verticies[ii]->x *
					obj->renderScale);
		}
		if((obj->verticies[ii]->y * obj->renderScale) >
				obj->boundingBox.high.y) {
			obj->boundingBox.high.y = (obj->verticies[ii]->y *
					obj->renderScale);
		}
		if((obj->verticies[ii]->y * obj->renderScale) <
				obj->boundingBox.low.y) {
			obj->boundingBox.low.y = (obj->verticies[ii]->y *
					obj->renderScale);
		}
		if((obj->verticies[ii]->z * obj->renderScale) >
				obj->boundingBox.high.z) {
			obj->boundingBox.high.z = (obj->verticies[ii]->z *
					obj->renderScale);
		}
		if((obj->verticies[ii]->z * obj->renderScale) <
				obj->boundingBox.low.z) {
			obj->boundingBox.low.z = (obj->verticies[ii]->z *
					obj->renderScale);
		}
	}
}

static void perform_error_correction(Wfobj *obj)
{
	int ii = 0;
	int nn = 0;
	int ref = 0;
	int counter = 0;
	double distance = 0.0;

	constructBoundingBox(obj);

	obj->sphere.radius = 0.0;
	vector3d_init(&obj->sphere.center);

	for(ii = 0; ii < obj->objectCount; ii++) {
		if((obj->objects[ii]->type == WFOBJ_OBJ_TYPE_V) ||
				(obj->objects[ii]->type == WFOBJ_OBJ_TYPE_VT) ||
				(obj->objects[ii]->type == WFOBJ_OBJ_TYPE_VTN)) {
			calculate_bounding_sphere(obj, ii);
		}

		if((obj->objects[ii]->type == WFOBJ_OBJ_TYPE_V) ||
				(obj->objects[ii]->type == WFOBJ_OBJ_TYPE_VT) ||
				(obj->objects[ii]->type == WFOBJ_OBJ_TYPE_VTN)) {
			if(obj->objects[ii]->points != obj->objects[ii]->vertexCount) {
				fprintf(stderr, "[%s():%i] error - detected object vertex "
						"count mismatch, %i vs %i.\n", __FUNCTION__, __LINE__,
						obj->objects[ii]->points,
						obj->objects[ii]->vertexCount);
			}
			else {
				for(nn = 0; nn < obj->objects[ii]->vertexCount; nn++) {
					ref = obj->objects[ii]->verticies[nn];
					if((ref < 0) || (ref >= obj->vertexCount)) {
						obj->objects[ii]->verticies[nn] = -1;
						obj->objects[ii]->vertexErrorCount += 1;
					}
					else {
						obj->sphere.center.x += obj->verticies[ref]->x;
						obj->sphere.center.y += obj->verticies[ref]->y;
						obj->sphere.center.z += obj->verticies[ref]->z;
						counter++;
					}
				}
			}
		}
		if((obj->objects[ii]->type == WFOBJ_OBJ_TYPE_VT) ||
				(obj->objects[ii]->type == WFOBJ_OBJ_TYPE_VTN)) {
			if(obj->objects[ii]->points != obj->objects[ii]->textureCount) {
				fprintf(stderr, "[%s():%i] error - detected object texture "
						"count mismatch, %i vs %i.\n", __FUNCTION__, __LINE__,
						obj->objects[ii]->points,
						obj->objects[ii]->textureCount);
			}
			else {
				for(nn = 0; nn < obj->objects[ii]->textureCount; nn++) {
					ref = obj->objects[ii]->textures[nn];
					if((ref < 0) || (ref >= obj->textureCount)) {
						obj->objects[ii]->textures[nn] = -1;
						obj->objects[ii]->textureErrorCount += 1;
					}
				}
			}
		}
		if(obj->objects[ii]->type == WFOBJ_OBJ_TYPE_VTN) {
			if(obj->objects[ii]->points != obj->objects[ii]->normalCount) {
				fprintf(stderr, "[%s():%i] error - detected object normal "
						"count mismatch, %i vs %i.\n", __FUNCTION__, __LINE__,
						obj->objects[ii]->points,
						obj->objects[ii]->normalCount);
			}
			else {
				for(nn = 0; nn < obj->objects[ii]->normalCount; nn++) {
					ref = obj->objects[ii]->normals[nn];
					if((ref < 0) || (ref >= obj->normalCount)) {
						obj->objects[ii]->normals[nn] = -1;
						obj->objects[ii]->normalErrorCount += 1;
					}
				}
			}
		}
	}

	obj->sphere.center.x /= (double)counter;
	obj->sphere.center.y /= (double)counter;
	obj->sphere.center.z /= (double)counter;

	for(ii = 0; ii < obj->vertexCount; ii++) {
		if(obj->verticies[ii] != NULL) {
			distance = sqrt(
					((obj->verticies[ii]->x - obj->sphere.center.x) *
					 (obj->verticies[ii]->x - obj->sphere.center.x)) +
					((obj->verticies[ii]->y - obj->sphere.center.y) *
					 (obj->verticies[ii]->y - obj->sphere.center.y)) +
					((obj->verticies[ii]->z - obj->sphere.center.z) *
					 (obj->verticies[ii]->z - obj->sphere.center.z)));
			if(distance > obj->sphere.radius) {
				obj->sphere.radius = distance;
			}
		}
	}
}

static void reverse_polygon_direction(Wfobj *obj, int object_index, 
		int texture_index_beg, int polygon_size)
{
	int ref_beg = 0;
	int ref_end = 0;
	int texture_index_end = 0;

	texture_index_end = texture_index_beg + polygon_size -1;

	do {
		ref_beg = obj->objects[object_index]->textures[texture_index_beg];
		ref_end = obj->objects[object_index]->textures[texture_index_end];

		obj->objects[object_index]->textures[texture_index_beg] = ref_end;
		obj->objects[object_index]->textures[texture_index_end] = ref_beg;
		
		texture_index_beg++;
		texture_index_end--;
	} while (texture_index_end > texture_index_beg);
}

static void renderBoundingBox(WfobjBoundingBox *bbox)
{
	aboolean isTexture2d = afalse;
	aboolean isColorMaterial = atrue;
	aboolean isLighting = afalse;

	if(cgi_glIsEnabled(GL_TEXTURE_2D)) {
		isTexture2d = atrue;
		cgi_glDisable(GL_TEXTURE_2D);
	}

	if(!cgi_glIsEnabled(GL_COLOR_MATERIAL)) {
		isColorMaterial = afalse;
		cgi_glEnable(GL_COLOR_MATERIAL);
	}

	if(cgi_glIsEnabled(GL_LIGHTING)) {
		isLighting = atrue;
		cgi_glDisable(GL_LIGHTING);
	}

	CGI_COLOR_BLUE;

	cgi_glBegin(GL_LINE_LOOP);
	cgi_glVertex3d(bbox->low.x, bbox->low.y, bbox->low.z);
	cgi_glVertex3d(bbox->high.x, bbox->low.y, bbox->low.z);
	cgi_glVertex3d(bbox->high.x, bbox->low.y, bbox->high.z);
	cgi_glVertex3d(bbox->low.x, bbox->low.y, bbox->high.z);
	cgi_glEnd();

	cgi_glBegin(GL_LINE_LOOP);
	cgi_glVertex3d(bbox->low.x, bbox->high.y, bbox->low.z);
	cgi_glVertex3d(bbox->high.x, bbox->high.y, bbox->low.z);
	cgi_glVertex3d(bbox->high.x, bbox->high.y, bbox->high.z);
	cgi_glVertex3d(bbox->low.x, bbox->high.y, bbox->high.z);
	cgi_glEnd();

	cgi_glBegin(GL_LINES);
	cgi_glVertex3d(bbox->low.x, bbox->low.y, bbox->low.z);
	cgi_glVertex3d(bbox->low.x, bbox->high.y, bbox->low.z);
	cgi_glVertex3d(bbox->high.x, bbox->low.y, bbox->low.z);
	cgi_glVertex3d(bbox->high.x, bbox->high.y, bbox->low.z);
	cgi_glEnd();

	cgi_glBegin(GL_LINES);
	cgi_glVertex3d(bbox->high.x, bbox->low.y, bbox->high.z);
	cgi_glVertex3d(bbox->high.x, bbox->high.y, bbox->high.z);
	cgi_glVertex3d(bbox->low.x, bbox->low.y, bbox->high.z);
	cgi_glVertex3d(bbox->low.x, bbox->high.y, bbox->high.z);
	cgi_glEnd();

	if(isLighting) {
		cgi_glEnable(GL_LIGHTING);
	}

	if(!isColorMaterial) {
		cgi_glDisable(GL_COLOR_MATERIAL);
	}

	if(isTexture2d) {
		cgi_glEnable(GL_TEXTURE_2D);
	}
}


// define wfobj public functions

Wfobj *wfobj_load(Log *log, char *filename)
{
	aboolean fileExists = afalse;
	int rc = 0;
	int length = 0;
	double timing = 0.0;
	char *path = NULL;
	char line[WFOBJ_READ_LINE_LENGTH];

	FileHandle fh;
	Wfobj *result = NULL;

	if((log == NULL) || (filename == NULL)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	log_logf(log, LOG_LEVEL_DEBUG,
			"loading wavefront object from '%s'", filename);

	timing = time_getTimeMus();

	if(((rc = system_fileExists(filename, &fileExists)) < 0) ||
			(!fileExists)) {
		log_logf(log, LOG_LEVEL_ERROR, "failed to locate file '%s'", filename);
		return NULL;
	}

	if((rc = file_init(&fh, filename, "r", WFOBJ_READ_LINE_LENGTH)) < 0) {
		fileError(rc);
		return NULL;
	}

	result = (Wfobj *)malloc(sizeof(Wfobj));

	result->filename = strdup(filename);
	result->materialFilename = NULL;
	result->lineCount = 0;
	result->lineErrorCount = 0;
	result->commentLineCount = 0;
	result->vertexCount = 0;
	result->normalCount = 0;
	result->textureCount = 0;
	result->objectCount = 0;
	result->renderScale = 1.0;
	result->verticies = NULL;
	result->normals = NULL;
	result->textures = NULL;
	result->objects = NULL;
	result->materialCount = 0;
	result->materials = NULL;

	path = parseFilepath(filename);

	rc = file_readLine(&fh, line, WFOBJ_READ_LINE_LENGTH, &length);
	while(rc == 0) {
		length = stringClean(line, length);

		processLine(result, 1, path, line, length);

		rc = file_readLine(&fh, line, WFOBJ_READ_LINE_LENGTH, &length);
	}

	free(path);

	file_free(&fh);

	log_logf(log, LOG_LEVEL_DEBUG,
			"loaded %i lines (%i errors) in %0.6f seconds from '%s'",
			result->lineCount, result->lineErrorCount, 
			time_getElapsedMusInSeconds(timing), filename);

	if(result->lineCount > result->lineErrorCount) {
		perform_error_correction(result);
	}

	return result;
}

void wfobj_free(Wfobj *obj)
{
	if(obj == NULL) {
		fprintf(stderr, "[%s():%i] error - invalid or missing argument(s).\n",
				__FUNCTION__, __LINE__);
		return;
	}

	/*
	 * TODO: this function
	 */
}

void wfobj_render3d(int renderMode, float x, float y, float z, Wfobj *obj,
		void **textures, Vector3d *rotation, void *frustum,
		double scale, aboolean render_bb)
{
	aboolean render_inverted = afalse;
	aboolean renderModeChanged = afalse;
	int ii = 0;
	int nn = 0;
	int jj = 0;
	int kk = 0;
	int ref = 0;
	int flag = 0;
	int matid = 0;
	int temp = 0;
	int localPolygonCount = 0;
	double radius = 0.0;

	GLboolean color_material_status = (GLboolean)0;
	GLboolean gl_texture_2d_status = (GLboolean)0;

	Point3d point;
	Vector3d corner;
	Vector3d center;
	Vector3d location;
	Sphere3d *sphere = NULL;
	Frustum3d *frustum3d = NULL;
	AxisAlignedBox aab;

	static int myRenderMode = 0;
	static double myRotation = 0.0;

	frustum3d = (Frustum3d *)frustum;

	if(scale != obj->renderScale) {
		obj->renderScale = scale;
		constructBoundingBox(obj);
	}

	obj->renderPolygonCount = 0;

	if((frustum3d != NULL) && (afalse)) {
		vector3d_initSet(&obj->sphere.center,
				((obj->boundingBox.low.x * scale) + (double)x),
				((obj->boundingBox.low.y * scale) + (double)y),
				((obj->boundingBox.low.z * scale) + (double)z));

		aab3d_initSet(&aab,
				((obj->boundingBox.high.x * scale) + (double)x),
				((obj->boundingBox.high.y * scale) + (double)y),
				((obj->boundingBox.high.z * scale) + (double)z),
				&corner);

		if(frustum3d_containsBox(frustum3d, &aab) == FRUSTUM_LOCATION_OUTSIDE) {
			return;
		}
	}

	if(myRenderMode != renderMode) {
		myRenderMode = renderMode;
		renderModeChanged = atrue;
	}

	// setup gl environment

	color_material_status = glIsEnabled(GL_COLOR_MATERIAL);
	gl_texture_2d_status = glIsEnabled(GL_TEXTURE_2D);

	if(!color_material_status) {
		glEnable(GL_COLOR_MATERIAL);
	}
	if(!gl_texture_2d_status) {
		glEnable(GL_TEXTURE_2D);
	}

	// define location

	glPushMatrix();
	glTranslatef(x, y, z);

	// perform rotation

	if(rotation != NULL) {
		glRotated(rotation->x, 1.0, 0.0, 0.0);
		glRotated(rotation->y, 0.0, 1.0, 0.0);
		glRotated(rotation->z, 0.0, 0.0, 1.0);
	}

	// render bounding box

	if(render_bb) {
		renderBoundingBox(&obj->boundingBox);
	}

	// render object

	for(ii = 0; ii < obj->objectCount; ii++) {
		if((obj->objects[ii]->points <= 0) ||
				(obj->objects[ii]->type == WFOBJ_OBJ_TYPE_UNKNOWN)) {
			continue;
		}

		if((frustum != NULL) && (!renderModeChanged) &&
				(obj->objects[ii]->sphere.radius > 0.0)) {
			radius = (obj->objects[ii]->sphere.radius * scale);
			vector3d_initSet(&center,
					(obj->objects[ii]->sphere.center.x * scale),
					(obj->objects[ii]->sphere.center.y * scale),
					(obj->objects[ii]->sphere.center.z * scale));
			vector3d_initSet(&location, x, y, z);
			vector3d_addEquals(&center, &location);

			point.x = center.x;
			point.y = center.y;
			point.z = center.z;

			if(frustum3d_containsSphere(frustum3d, &point, radius) ==
					FRUSTUM_LOCATION_OUTSIDE) {
				continue;
			}
/*
			sphere = (Sphere3d *)obj->objects[ii]->sphere.render;
			sphere->radius = radius;
			sphere->center.x = (obj->objects[ii]->sphere.center.x * scale);
			sphere->center.y = (obj->objects[ii]->sphere.center.y * scale);
			sphere->center.z = (obj->objects[ii]->sphere.center.z * scale);

			CGI_COLOR_BLUE;
//			glDisable(GL_LIGHTING);
			sphere3d_render3d(sphere, ASGARD_ENGINE_RENDER_MODE_WIREFRAME);
//			glEnable(GL_LIGHTING);
*/
		}

		if((obj->objects[ii]->isListed) && (renderModeChanged)) {
			obj->objects[ii]->isListed = afalse;
		}

		if(!obj->objects[ii]->isListed) {
			obj->objects[ii]->listId = cgi_glGenLists(1);

			cgi_glNewList(obj->objects[ii]->listId, GL_COMPILE);
		}
		else {
/*
 * Notes: this is a rotation experiment with the Blackhawk object
 *
{
	if(ii == 4) {
		myRotation += ((double)(rand() % 10) + 10.0);

		glPushMatrix();

//		glTranslatef(x, y, z);

		glRotated(0.0, 1.0, 0.0, 0.0); // x
		glRotated(myRotation, 0.0, 1.0, 0.0); // y
		glRotated(0.0, 0.0, 0.0, 1.0); // z
	}
}
 */

		cgi_glCallList(obj->objects[ii]->listId);
/*
{
	if(ii == 4) {
		glPopMatrix();
	}
}
*/
			obj->renderPolygonCount += obj->objects[ii]->renderPolygonCount;
			continue;
		}

		flag = 0;
		render_inverted = afalse;

		matid = obj->objects[ii]->material;

/*		if((matid != -1) && (textures != NULL) && (textures[matid] != NULL)) {
			texture_apply((Texture *)textures[matid]);
		}*/

		if((matid != -1) &&
				((renderMode == ASGARD_ENGINE_RENDER_MODE_TEXTURED) ||
				 (renderMode ==
				  ASGARD_ENGINE_RENDER_MODE_TEXTURED_NORMALIZED)) &&
				(obj->materialCount > 0) &&
				(obj->objects[ii]->material >= 0) &&
				(obj->objects[ii]->material < obj->materialCount)) {
			// set rendering flag

			flag = 1;

			// set material lighting properties

			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,
					obj->materials[matid]->diffuse);
			glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,
					obj->materials[matid]->ambient);
			glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,
					obj->materials[matid]->specular);
			glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION,
					obj->materials[matid]->emissive);
			glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS,
					obj->materials[matid]->shininess);

			// object properties

			if(0) { // TODO: determine if the object should be inverted
				render_inverted = atrue;
			}

			/*
			 * TODO: load texture properties - such as blending - from the
			 * wfobject material file
			 */

			// performing texturing

			if((textures != NULL) && (textures[matid] != NULL)) {
				texture_apply((Texture *)textures[matid]);
			}
			else {
				flag = 0;
			}
		}
		else {
			flag = 0;
//			matid = -1;
		}

		if(!flag) {
			if((gl_texture_2d_status = glIsEnabled(GL_TEXTURE_2D)) !=
					(GLboolean)0) {
				glDisable(GL_TEXTURE_2D);
			}

			if(matid == -1) {
				CGI_COLOR_GREEN;
			}
			else if((textures != NULL) && (textures[matid] == NULL)) {
				CGI_COLOR_RED;
			}
			else {
				CGI_COLOR_WHITE;
				CGI_COLOR_BLUE;
			}
		}

/*		{
			double xx = 0.0;
			double yy = 0.0;
			double zz = 0.0;

	for(xx = -100.0, yy = 0.0, zz = -100.0;
			xx <= 100.0;
			xx += 10.0, zz += 10.0) {
		cgi_glBegin(GL_QUADS);

		cgi_glTexCoord2d(0.0, 0.0);
		cgi_glNormal3d(0.0, 1.0, 0.0);
		cgi_glVertex3d(xx, 0.0, (zz + 10.0));

		cgi_glTexCoord2d(1.0, 0.0);
		cgi_glNormal3d(0.0, 1.0, 0.0);
		cgi_glVertex3d((xx + 10.0), 0.0, (zz + 10.0));

		cgi_glTexCoord2d(1.0, 1.0);
		cgi_glNormal3d(0.0, 1.0, 0.0);
		cgi_glVertex3d((xx + 10.0), 0.0, zz);

		cgi_glTexCoord2d(0.0, 1.0);
		cgi_glNormal3d(0.0, 1.0, 0.0);
		cgi_glVertex3d(xx, 0.0, zz);

		cgi_glEnd();
	}
		}
*/
		localPolygonCount = 0;

		if(render_inverted) {
			for(nn = (obj->objects[ii]->faces - 1), jj = 0; nn >= 0; nn--) {
				localPolygonCount += 1;
				obj->renderPolygonCount += 1;
				if(obj->objects[ii]->types[nn] == WFOBJ_FACE_TYPE_TRIANGLES) {
					if(flag) {
						cgi_glBegin(GL_TRIANGLES);
					}
					else {
						cgi_glBegin(GL_LINE_LOOP);
					}
				}
				else if(obj->objects[ii]->types[nn] == WFOBJ_FACE_TYPE_QUADS) {
					if(flag) {
						cgi_glBegin(GL_QUADS);
					}
					else {
						cgi_glBegin(GL_LINE_LOOP);
					}
				}
				else if((obj->objects[ii]->types[nn] >=
						WFOBJ_FACE_TYPE_POLYGONS) &&
						(obj->objects[ii]->types[nn] < 16)) {
					if(flag) {
						cgi_glBegin(GL_POLYGON);
					}
					else {
						cgi_glBegin(GL_LINE_LOOP);
					}
				}
				else {
					printf("...obj #%i, face: %i, type: ERROR(%i)\n", ii, nn,
							obj->objects[ii]->types[nn]);
					break;
				}

				for(kk = 0; kk < obj->objects[ii]->types[nn]; kk++) {
					temp = ((obj->objects[ii]->points - 1) - jj);
					if((obj->objects[ii]->type == WFOBJ_OBJ_TYPE_VTN) &&
							(temp < obj->objects[ii]->normalCount)) {
						ref = obj->objects[ii]->normals[temp];
						if((ref >= 0) && (ref < obj->normalCount)) {
							cgi_glNormal3d((GLdouble)obj->normals[ref]->x,
									(GLdouble)obj->normals[ref]->y,
									(GLdouble)obj->normals[ref]->z);
						}
					}
					if(((obj->objects[ii]->type == WFOBJ_OBJ_TYPE_VT) ||
								(obj->objects[ii]->type ==
								 WFOBJ_OBJ_TYPE_VTN)) &&
							(temp < obj->objects[ii]->textureCount)) {
						ref = obj->objects[ii]->textures[temp];
						if((ref >= 0) && (ref < obj->textureCount)) {
							cgi_glTexCoord2d((double)obj->textures[ref]->u,
									(double)obj->textures[ref]->v);
						}
					}
					if(((obj->objects[ii]->type == WFOBJ_OBJ_TYPE_V) ||
								(obj->objects[ii]->type == WFOBJ_OBJ_TYPE_VT) ||
								(obj->objects[ii]->type ==
								 WFOBJ_OBJ_TYPE_VTN)) &&
							(temp < obj->objects[ii]->vertexCount)) {
						ref = obj->objects[ii]->verticies[temp];
						if((ref >= 0) && (ref < obj->vertexCount)) {
							cgi_glVertex3d(
									(GLdouble)(obj->verticies[ref]->x * scale),
									(GLdouble)(obj->verticies[ref]->y * scale),
									(GLdouble)(obj->verticies[ref]->z * scale));
						}
					}
					jj++;
				}

				cgi_glEnd();
			}
		}

		for(nn = 0, jj = 0; nn < obj->objects[ii]->faces; nn++) {
			localPolygonCount += 1;
			obj->renderPolygonCount += 1;
//printf("(%i) rendering object #%i '%s' (of %i) face #%i (%i polygon faces)\n", flag, (ii + 1), obj->objects[ii]->name, obj->objectCount, nn, obj->objects[ii]->types[nn]);
			if(obj->objects[ii]->types[nn] == WFOBJ_FACE_TYPE_TRIANGLES) {
				if(flag) {
					cgi_glBegin(GL_TRIANGLES);
				}
				else {
					cgi_glBegin(GL_LINE_LOOP);
				}
			}
			else if(obj->objects[ii]->types[nn] == WFOBJ_FACE_TYPE_QUADS) {
				if(flag) {
					cgi_glBegin(GL_QUADS);
				}
				else {
					cgi_glBegin(GL_LINE_LOOP);
				}
			}
			else if((obj->objects[ii]->types[nn] >=
					WFOBJ_FACE_TYPE_POLYGONS) &&
					(obj->objects[ii]->types[nn] < 16)) {
				if(flag) {
					cgi_glBegin(GL_POLYGON);
				}
				else {
					cgi_glBegin(GL_LINE_LOOP);
				}
			}
			else {
				printf("...obj #%i, face: %i, type: ERROR(%i)\n", ii, nn,
						obj->objects[ii]->types[nn]);
				break;
			}

			for(kk = 0; kk < obj->objects[ii]->types[nn]; kk++) {
				if((obj->objects[ii]->type == WFOBJ_OBJ_TYPE_VTN) &&
						(jj < obj->objects[ii]->normalCount)) {
					ref = obj->objects[ii]->normals[jj];
					if((ref >= 0) && (ref < obj->normalCount)) {
//printf("\t glNormal3f #%i (%0.6f, %0.6f, %0.6f)\n", ref, obj->normals[ref]->x, obj->normals[ref]->y, obj->normals[ref]->z);
						cgi_glNormal3d((GLdouble)obj->normals[ref]->x,
								(GLdouble)obj->normals[ref]->y,
								(GLdouble)obj->normals[ref]->z);
					}
				}
				if(((obj->objects[ii]->type == WFOBJ_OBJ_TYPE_VT) ||
							(obj->objects[ii]->type == WFOBJ_OBJ_TYPE_VTN)) &&
						(jj < obj->objects[ii]->textureCount)) {
					ref = obj->objects[ii]->textures[jj];
					if((ref >= 0) && (ref < obj->textureCount)) {
						cgi_glTexCoord2d((GLdouble)obj->textures[ref]->u,
								(GLdouble)obj->textures[ref]->v);
//printf("\t glTexCoord2f #%i (%0.6f, %0.6f)\n", ref, obj->textures[ref]->u, obj->textures[ref]->v);
					}
				}
				if(((obj->objects[ii]->type == WFOBJ_OBJ_TYPE_V) ||
							(obj->objects[ii]->type == WFOBJ_OBJ_TYPE_VT) ||
							(obj->objects[ii]->type == WFOBJ_OBJ_TYPE_VTN)) &&
						(jj < obj->objects[ii]->vertexCount)) {
					ref = obj->objects[ii]->verticies[jj];
					if((ref >= 0) && (ref < obj->vertexCount)) {
						cgi_glVertex3d(
								(GLdouble)(obj->verticies[ref]->x * scale),
								(GLdouble)(obj->verticies[ref]->y * scale),
								(GLdouble)(obj->verticies[ref]->z * scale));
//printf("\t glVertex3f #%i (%0.6f, %0.6f, %0.6f)\n", ref, obj->verticies[ref]->x, obj->verticies[ref]->y, obj->verticies[ref]->z);
					}
				}
				jj++;
			}

			cgi_glEnd();
//exit(1);
		}

		cgi_glEndList();

		obj->objects[ii]->isListed = atrue;

		if(obj->objects[ii]->renderPolygonCount == -1) {
			obj->objects[ii]->renderPolygonCount = localPolygonCount;
		}
	}

	glPopMatrix();

	if(!color_material_status) {
		glDisable(GL_COLOR_MATERIAL);
	}
	if(!gl_texture_2d_status) {
		glDisable(GL_TEXTURE_2D);
	}
}

int wfobj_set_textures_direction(Wfobj *obj, int direction)
{
/*
	int ii = 0;
	int jj = 0;
	int ff = 0;
	int nn = 0;
	int ref = 0;
	int faces = 0;
	int polygon_size = 0;
	int current_direction = 0;
	float *x = NULL;
	float *y = NULL;

	if ((direction != TEXTURE_COORDINATES_CW) &&
			(direction !=  TEXTURE_COORDINATES_CCW)) {
		fprintf(stderr, "[%s():%i] error - unknown direction",
				__FUNCTION__, __LINE__);
		return -1;
	}

	for(ii = 0; ii < obj->objectCount; ii++) {
		if((obj->objects[ii]->type == WFOBJ_OBJ_TYPE_VT) ||
				(obj->objects[ii]->type == WFOBJ_OBJ_TYPE_VTN)) {

			nn = 0;
			faces = obj->objects[ii]->faces;
			for (ff = 0; ff < faces; ff++) {

				polygon_size = obj->objects[ii]->types[ff];
				if(x == NULL) {
					x = (float *)malloc(sizeof(float) * polygon_size);
				}
				else {
					x = (float *)realloc(x, sizeof(float) * polygon_size);
				}
				if(y == NULL) {
					y = (float *)malloc(sizeof(float) * polygon_size);
				}
				else {
					y = (float *)realloc(y, sizeof(float) * polygon_size);
				}
				memset(x, 0, sizeof(float) * polygon_size);
				memset(y, 0, sizeof(float) * polygon_size);

				for (jj = 0; jj < polygon_size; jj++) {

					ref = obj->objects[ii]->textures[nn];
					x[jj] = obj->textures[ref]->u;
					y[jj] = obj->textures[ref]->v;
					nn++;
				}
 
				current_direction = determine_polygon_ordering(x, y, 
						polygon_size);

				if (current_direction == TEXTURE_COORDINATES_COLINEAR) {
					fprintf(stderr, "[%s():%i] warning - vertexes are "
							"colinear (object: %s face %d)\n\t", 
							__FUNCTION__, __LINE__, obj->objects[ii]->name, ff);

					for (jj = 0; jj < polygon_size; jj++) {
						fprintf(stderr, " (%f, %f)", x[jj], y[jj]);
					}
					fprintf(stderr, "\n\n");
				}

				if (current_direction != direction) {
					reverse_polygon_direction(obj, ii, nn - polygon_size,
							polygon_size);
				}
			}
		}
	}

	free(x);
	free(y);
*/
	return 0;
}

int wfobj_write(Wfobj *obj, char *filename, aboolean auto_correct,
		float scale)
{
/*
	int ii = 0;
	int nn = 0;
	int jj = 0;
	int kk = 0;
	int rc = 0;
	int ref = 0;
	float x = 0.0;
	float y = 0.0;
	float z = 0.0;
	char *temp = NULL;
	char temp_two[128];
	char buffer[1024];

	FH fh;

	file_init(&fh);

	if((rc = file_open(&fh, filename, "w+")) < 0) {
		fprintf(stderr, "[%s():%i] error - failed to open file '%s' with "
				"'%s'.\n", __FUNCTION__, __LINE__, filename,
				file_strerror(&fh, rc));
		file_free(&fh);
		return -1;
	}

	if(auto_correct) {
		x = MAX_SIGNED_DOUBLE;
		y = MAX_SIGNED_DOUBLE;
		z = MAX_SIGNED_DOUBLE;

		for(ii = 0; ii < obj->vertexCount; ii++) {
			if(obj->verticies[ii]->x < x) {
				x = obj->verticies[ii]->x;
			}
			if(obj->verticies[ii]->y < y) {
				y = obj->verticies[ii]->y;
			}
			if(obj->verticies[ii]->z < z) {
				z = obj->verticies[ii]->z;
			}
		}
	}

	temp = timer_get_time_stamp();
	snprintf(buffer, sizeof(buffer), "# %s - written on %s\n", filename, temp);
	if((rc = file_write(&fh, buffer, strlen(buffer))) < 0) {
		fprintf(stderr, "[%s():%i] error - failed to write to '%s' with "
				"'%s'.\n", __FUNCTION__, __LINE__, filename,
				file_strerror(&fh, rc));
		file_close(&fh);
		file_free(&fh);
		return -1;
	}

	snprintf(buffer, sizeof(buffer), "mtllib %s\n", obj->materialFilename);
	if((rc = file_write(&fh, buffer, strlen(buffer))) < 0) {
		fprintf(stderr, "[%s():%i] error - failed to write to '%s' with "
				"'%s'.\n", __FUNCTION__, __LINE__, filename,
				file_strerror(&fh, rc));
		file_close(&fh);
		file_free(&fh);
		return -1;
	}

	for(ii = 0; ii < obj->objectCount; ii++) {
		if((obj->objects[ii]->points <= 0) ||
				(obj->objects[ii]->type == WFOBJ_OBJ_TYPE_UNKNOWN)) {
			continue;
		}

		snprintf(buffer, sizeof(buffer), "g %s\n", obj->objects[ii]->name);
		if((rc = file_write(&fh, buffer, strlen(buffer))) < 0) {
			fprintf(stderr, "[%s():%i] error - failed to write to '%s' with "
					"'%s'.\n", __FUNCTION__, __LINE__, filename,
					file_strerror(&fh, rc));
			file_close(&fh);
			file_free(&fh);
			return -1;
		}

		if(obj->objects[ii]->smoothing != NULL) {
			snprintf(buffer, sizeof(buffer), "s %s\n",
					obj->objects[ii]->smoothing);
			if((rc = file_write(&fh, buffer, strlen(buffer))) < 0) {
				fprintf(stderr, "[%s():%i] error - failed to write to '%s' "
						"with '%s'.\n", __FUNCTION__, __LINE__, filename,
						file_strerror(&fh, rc));
				file_close(&fh);
				file_free(&fh);
				return -1;
			}
		}

		if((obj->materialCount > 0) &&
				(obj->objects[ii]->material >= 0) &&
				(obj->objects[ii]->material < obj->materialCount)) {
			snprintf(buffer, sizeof(buffer), "usemtl %s\n",
					obj->materials[(obj->objects[ii]->material)]->name);
			if((rc = file_write(&fh, buffer, strlen(buffer))) < 0) {
				fprintf(stderr, "[%s():%i] error - failed to write to '%s' "
						"with '%s'.\n", __FUNCTION__, __LINE__, filename,
						file_strerror(&fh, rc));
				file_close(&fh);
				file_free(&fh);
				return -1;
			}
		}

		for(nn = 0, jj = 0; nn < obj->objects[ii]->faces; nn++) {
			for(kk = 0; kk < obj->objects[ii]->types[nn]; kk++) {
				if((obj->objects[ii]->type == WFOBJ_OBJ_TYPE_VTN) &&
						(jj < obj->objects[ii]->normalCount)) {
					ref = obj->objects[ii]->normals[jj];
					if((ref >= 0) && (ref < obj->normalCount)) {
						snprintf(buffer, sizeof(buffer), "vn %f %f %f\n",
								obj->normals[ref]->x, obj->normals[ref]->y,
								obj->normals[ref]->z);
						if((rc = file_write(&fh, buffer,
										strlen(buffer))) < 0) {
							fprintf(stderr, "[%s():%i] error - failed to "
									"write to '%s' with '%s'.\n", __FUNCTION__,
									__LINE__, filename,
									file_strerror(&fh, rc));
							file_close(&fh);
							file_free(&fh);
							return -1;
						}
					}
				}
				jj++;
			}
		}

		for(nn = 0, jj = 0; nn < obj->objects[ii]->faces; nn++) {
			for(kk = 0; kk < obj->objects[ii]->types[nn]; kk++) {
				if(((obj->objects[ii]->type == WFOBJ_OBJ_TYPE_VT) ||
							(obj->objects[ii]->type == WFOBJ_OBJ_TYPE_VTN)) &&
						(jj < obj->objects[ii]->textureCount)) {
					ref = obj->objects[ii]->textures[jj];
					if((ref >= 0) && (ref < obj->textureCount)) {
						snprintf(buffer, sizeof(buffer), "vt %f %f\n",
								obj->textures[ref]->u, obj->textures[ref]->v);
						if((rc = file_write(&fh, buffer,
										strlen(buffer))) < 0) {
							fprintf(stderr, "[%s():%i] error - failed to "
									"write to '%s' with '%s'.\n", __FUNCTION__,
									__LINE__, filename,
									file_strerror(&fh, rc));
							file_close(&fh);
							file_free(&fh);
							return -1;
						}
					}
				}
				jj++;
			}
		}

		for(nn = 0, jj = 0; nn < obj->objects[ii]->faces; nn++) {
			for(kk = 0; kk < obj->objects[ii]->types[nn]; kk++) {
				if(((obj->objects[ii]->type == WFOBJ_OBJ_TYPE_V) ||
							(obj->objects[ii]->type == WFOBJ_OBJ_TYPE_VT) ||
							(obj->objects[ii]->type == WFOBJ_OBJ_TYPE_VTN)) &&
						(jj < obj->objects[ii]->vertexCount)) {
					ref = obj->objects[ii]->verticies[jj];
					if((ref >= 0) && (ref < obj->vertexCount)) {
						if(auto_correct) {
							snprintf(buffer, sizeof(buffer), "v %f %f %f\n",
									((obj->verticies[ref]->x - x) * scale),
									((obj->verticies[ref]->y - y) * scale),
									((obj->verticies[ref]->z - z) * scale));
						}
						else {
							snprintf(buffer, sizeof(buffer), "v %f %f %f\n",
									(obj->verticies[ref]->x * scale),
									(obj->verticies[ref]->y * scale),
									(obj->verticies[ref]->z * scale));
						}
						if((rc = file_write(&fh, buffer,
										strlen(buffer))) < 0) {
							fprintf(stderr, "[%s():%i] error - failed to "
									"write to '%s' with '%s'.\n", __FUNCTION__,
									__LINE__, filename,
									file_strerror(&fh, rc));
							file_close(&fh);
							file_free(&fh);
							return -1;
						}
					}
				}
				jj++;
			}
		}

		for(nn = 0, jj = 0; nn < obj->objects[ii]->faces; nn++) {
			memset(buffer, 0, sizeof(buffer));
			strcpy(buffer, "f ");
			for(kk = 0; kk < obj->objects[ii]->types[nn]; kk++) {
				if(((obj->objects[ii]->type == WFOBJ_OBJ_TYPE_V) ||
							(obj->objects[ii]->type == WFOBJ_OBJ_TYPE_VT) ||
							(obj->objects[ii]->type == WFOBJ_OBJ_TYPE_VTN)) &&
						(jj < obj->objects[ii]->vertexCount)) {
					ref = obj->objects[ii]->verticies[jj];
					if((ref >= 0) && (ref < obj->vertexCount)) {
						snprintf(temp_two, sizeof(temp_two), "%i", (ref + 1));
						strcat(buffer, temp_two);
						if(obj->objects[ii]->type == WFOBJ_OBJ_TYPE_VT) {
							strcat(buffer, "/");
						}
						else if(obj->objects[ii]->type == WFOBJ_OBJ_TYPE_VTN) {
							strcat(buffer, "/");
						}
					}
				}
				if(((obj->objects[ii]->type == WFOBJ_OBJ_TYPE_VT) ||
							(obj->objects[ii]->type == WFOBJ_OBJ_TYPE_VTN)) &&
						(jj < obj->objects[ii]->textureCount)) {
					ref = obj->objects[ii]->textures[jj];
					if((ref >= 0) && (ref < obj->textureCount)) {
						snprintf(temp_two, sizeof(temp_two), "%i", (ref + 1));
						strcat(buffer, temp_two);
						if(obj->objects[ii]->type == WFOBJ_OBJ_TYPE_VT) {
							strcat(buffer, " ");
						}
						else if(obj->objects[ii]->type == WFOBJ_OBJ_TYPE_VTN) {
							strcat(buffer, "/");
						}
					}
				}
				if((obj->objects[ii]->type == WFOBJ_OBJ_TYPE_VTN) &&
						(jj < obj->objects[ii]->normalCount)) {
					ref = obj->objects[ii]->normals[jj];
					if((ref >= 0) && (ref < obj->normalCount)) {
						snprintf(temp_two, sizeof(temp_two), "%i", (ref + 1));
						strcat(buffer, temp_two);
					}
				}
				strcat(buffer, " ");
				jj++;
			}
			strcat(buffer, "\n");
			file_write(&fh, buffer, strlen(buffer));
			if((rc = file_write(&fh, buffer, strlen(buffer))) < 0) {
				fprintf(stderr, "[%s():%i] error - failed to write to '%s' "
						"with '%s'.\n", __FUNCTION__, __LINE__, filename,
						file_strerror(&fh, rc));
				file_close(&fh);
				file_free(&fh);
				return -1;
			}
		}
	}

	file_close(&fh);
	file_free(&fh);
*/
	return 0;
}

