/*
 * pipeline.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard Game Engine rendering pipeline management system.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#include "asgard/asgard.h"
#define _ASGARD_ENGINE_COMPONENT
#include "asgard/engine/pipeline.h"


// define pipeline private constants

typedef enum _PipelineItemTypes {
	PIPELINE_ITEM_TYPE_OBJECT = 1,
	PIPELINE_ITEM_TYPE_SECTION,
	PIPELINE_ITEM_TYPE_RENDER,
	PIPELINE_ITEM_TYPE_MODE,
	PIPELINE_ITEM_TYPE_UNKNOWN = -1
} PipelineItemTypes;


// define pipeline private data types

typedef struct _PipelineItem {
	int type;
	void *payload;
} PipelineItem;


// define pipeline private global variables

Log *localLog = NULL;


// declare pipeline private functions

// general functions

static void pipelineFreeIndexEntry(void *memory);

static int pipelineAddItem(Pipeline *pipeline, char *key, int keyLength,
		int type, void *payload);

// path functions

static int pathGetFieldCount(char *pathName, int pathNameLength);

static int pathGetField(char *pathName, int pathNameLength, int fieldNumber,
		char field[1024]);

static int pathGetParent(char *pathName, int pathNameLength,
		int pathFieldCount, char parentPath[1024]);

static int pathGetRender(char *pathName, int pathNameLength,
		char renderPath[1024]);

// mode functions

static int modeBuildKey(char *modeName, char key[1024]);

static void modeFreeMemory(PipelineMode *mode);

static PipelineMode *modeGet(Pipeline *pipeline, char *modeName);

static PipelineMode *modeNew(Pipeline *pipeline, char *modeName);

// render functions

static int renderBuildKey(char *modeName, char *renderName, char key[1024]);

static void renderFreeMemory(PipelineRender *render);

static PipelineRender *renderGet(Pipeline *pipeline, char *pathName);

// section functions

static int sectionBuildKey(char *pathName, char *sectionName, char key[1024]);

static PipelineSection *sectionGet(Pipeline *pipeline, char *pathName,
		char *sectionName);

static PipelineSection *sectionNew(Pipeline *pipeline, char *pathName,
		char *sectionName);

// object functions

static int objectBuildKey(char *pathName, char *objectName, char key[1024]);

static void objectFreeMemory(PipelineObject *object);

static PipelineObject *objectGet(Pipeline *pipeline, char *pathName,
		char *objectName);

static PipelineObject *objectNew(Pipeline *pipeline, char *pathName,
		char *objectName);

// rendering functions

static void renderObject(PipelineObject *object);

static void renderSection(Pipeline *pipeline, PipelineSection *section);



// define pipeline private functions

// general functions

static void pipelineFreeIndexEntry(void *memory)
{
	PipelineItem *item = NULL;

	if(memory == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	item = (PipelineItem *)memory;

	switch(item->type) {
		case PIPELINE_ITEM_TYPE_OBJECT:
			objectFreeMemory(item->payload);
			break;
		case PIPELINE_ITEM_TYPE_SECTION:
			// TODO: this
			break;
		case PIPELINE_ITEM_TYPE_RENDER:
			renderFreeMemory(item->payload);
			break;
		case PIPELINE_ITEM_TYPE_MODE:
			modeFreeMemory(item->payload);
			break;
		default:
			log_logf(localLog, LOG_LEVEL_ERROR, "unable to free index item, "
					"invalid item type %i", item->type);
	}

	memset(item, 0, (sizeof(PipelineItem)));

	free(item);
}

static int pipelineAddItem(Pipeline *pipeline, char *key, int keyLength,
		int type, void *payload)
{
	PipelineItem *item = NULL;

	item = (PipelineItem *)malloc(sizeof(PipelineItem));

	item->type = type;
	item->payload = payload;

	if(bptree_put(&pipeline->index, key, keyLength, (void *)item) < 0) {
		free(item);
		return -1;
	}

	return 0;
}

// path functions

static int pathGetFieldCount(char *pathName, int pathNameLength)
{
	int ii = 0;
	int pathCounter = 0;

	for(ii = 0; ii < pathNameLength; ii++) {
		if(pathName[ii] == '/') {
			pathCounter++;
		}
	}

	return pathCounter;
}

static int pathGetField(char *pathName, int pathNameLength, int fieldNumber,
		char field[1024])
{
	int ii = 0;
	int nn = 0;
	int pathCounter = 0;

	for(ii = 0; ii < pathNameLength; ii++) {
		if(pathName[ii] == '/') {
			pathCounter++;
		}
		else if(pathCounter == fieldNumber) {
			field[nn] = pathName[ii];
			nn++;
			if(nn >= 1024) {
				break;
			}
		}
		else if(pathCounter > fieldNumber) {
			break;
		}
	}

	if(nn < 1024) {
		field[nn] = '\0';
	}

	return nn;
}

static int pathGetParent(char *pathName, int pathNameLength,
		int pathFieldCount, char parentPath[1024])
{
	int ii = 0;
	int nn = 0;
	int pathCounter = 0;

	for(ii = 0; ii < pathNameLength; ii++) {
		if(pathName[ii] == '/') {
			pathCounter++;
		}
		if(pathCounter < pathFieldCount) {
			parentPath[nn] = pathName[ii];
			nn++;
			if(nn >= 1024) {
				break;
			}
		}
		else if(pathCounter >= pathFieldCount) {
			break;
		}
	}

	if(nn < 1024) {
		parentPath[nn] = '\0';
	}

	return nn;
}

static int pathGetRender(char *pathName, int pathNameLength,
		char renderPath[1024])
{
	int rc = 0;
	int renderPathLength = 0;
	char modeName[1024];
	char renderName[1024];

	if((rc = pathGetField(pathName, pathNameLength, 1, modeName)) < 1) {
		return 0;
	}
	if((rc = pathGetField(pathName, pathNameLength, 2, renderName)) < 1) {
		return 0;
	}

	renderPathLength = snprintf(renderPath, (sizeof(char) * 1023), "/%s/%s",
			modeName, renderName);

	return renderPathLength;
}

// mode functions

static int modeBuildKey(char *modeName, char key[1024])
{
	int keyLength = 0;

	keyLength = snprintf(key, (sizeof(char) * 1023), "/%s", modeName);

	return keyLength;
}

static void modeFreeMemory(PipelineMode *mode)
{
	if(mode->name != NULL) {
		free(mode->name);
	}

	memset(mode, 0, (sizeof(PipelineMode)));

	free(mode);
}

static PipelineMode *modeGet(Pipeline *pipeline, char *modeName)
{
	int keyLength = 0;
	char key[1024];

	PipelineItem *item = NULL;

	keyLength = modeBuildKey(modeName, key);

	if(bptree_get(&pipeline->index, key, keyLength, ((void *)&item)) < 0) {
		return NULL;
	}
	else if((item == NULL) || (item->type != PIPELINE_ITEM_TYPE_MODE)) {
		log_logf(pipeline->log, LOG_LEVEL_ERROR,
				"{PIPELINE} located null or invalid entry for mode '%s'",
				modeName);
		return NULL;
	}

	return (PipelineMode *)item->payload;
}

static PipelineMode *modeNew(Pipeline *pipeline, char *modeName)
{
	int keyLength = 0;
	char key[1024];

	PipelineMode *mode = NULL;

	if((mode = modeGet(pipeline, modeName)) != NULL) {
		log_logf(pipeline->log, LOG_LEVEL_ERROR,
				"{PIPELINE} unable to add new mode '%s', already exists",
				modeName);
		return NULL;
	}

	mode = (PipelineMode *)malloc(sizeof(PipelineMode));

	mode->index = 0;
	mode->name = strdup(modeName);
	mode->render2d.sectionLength = 0;
	mode->render2d.rootSections = NULL;
	mode->render3d.sectionLength = 0;
	mode->render3d.rootSections = NULL;

	// add mode to index

	keyLength = modeBuildKey(modeName, key);

	if(pipelineAddItem(pipeline, key, keyLength, PIPELINE_ITEM_TYPE_MODE,
				(void *)mode) < 0) {
		log_logf(pipeline->log, LOG_LEVEL_ERROR,
				"{PIPELINE} failed to add mode '%s' to the pipeline",
				modeName);
		free(mode);
		return NULL;
	}

	// add mode, 2d rendering to index

	keyLength = renderBuildKey(modeName, "2d", key);

	if(pipelineAddItem(pipeline, key, keyLength, PIPELINE_ITEM_TYPE_RENDER,
				(void *)&mode->render2d) < 0) {
		log_logf(pipeline->log, LOG_LEVEL_ERROR,
				"{PIPELINE} failed to add mode '%s' render 2d to the pipeline",
				modeName);
		free(mode);
		return NULL;
	}

	// add mode, 3d rendering to index

	keyLength = renderBuildKey(modeName, "3d", key);

	if(pipelineAddItem(pipeline, key, keyLength, PIPELINE_ITEM_TYPE_RENDER,
				(void *)&mode->render3d) < 0) {
		log_logf(pipeline->log, LOG_LEVEL_ERROR,
				"{PIPELINE} failed to add mode '%s' render 3d to the pipeline",
				modeName);
		free(mode);
		return NULL;
	}

	return mode;
}

// render functions

static int renderBuildKey(char *modeName, char *renderName, char key[1024])
{
	int keyLength = 0;

	keyLength = snprintf(key, (sizeof(char) * 1023), "/%s/%s", modeName,
			renderName);

	return keyLength;
}

static void renderFreeMemory(PipelineRender *render)
{
	if(render->rootSections != NULL) {
		free(render->rootSections);
	}

	memset(render, 0, (sizeof(PipelineRender)));
}

static PipelineRender *renderGet(Pipeline *pipeline, char *pathName)
{
	PipelineItem *item = NULL;

	if(bptree_get(&pipeline->index, pathName, (int)strlen(pathName),
				((void *)&item)) < 0) {
		return NULL;
	}
	else if((item == NULL) || (item->type != PIPELINE_ITEM_TYPE_RENDER)) {
		log_logf(pipeline->log, LOG_LEVEL_ERROR,
				"{PIPELINE} located null or invalid entry for render '%s'",
				pathName);
		return NULL;
	}

	return (PipelineRender *)item->payload;
}

// section functions

static int sectionBuildKey(char *pathName, char *sectionName, char key[1024])
{
	int keyLength = 0;

	keyLength = snprintf(key, (sizeof(char) * 1023), "%s/%s", pathName,
			sectionName);

	return keyLength;
}

static PipelineSection *sectionGet(Pipeline *pipeline, char *pathName,
		char *sectionName)
{
	int keyLength = 0;
	char key[1024];

	PipelineItem *item = NULL;

	if(sectionName != NULL) {
		keyLength = sectionBuildKey(pathName, sectionName, key);
	}
	else {
		keyLength = snprintf(key, (sizeof(char) * 1023), "%s", pathName);
	}

	if(bptree_get(&pipeline->index, key, keyLength, ((void *)&item)) < 0) {
		return NULL;
	}
	else if((item == NULL) || (item->type != PIPELINE_ITEM_TYPE_SECTION)) {
		log_logf(pipeline->log, LOG_LEVEL_ERROR,
				"{PIPELINE} located null or invalid entry for section '%s' "
				"path '%s'", sectionName, pathName);
		return NULL;
	}

	return (PipelineSection *)item->payload;
}

static PipelineSection *sectionNew(Pipeline *pipeline, char *pathName,
		char *sectionName)
{
	int keyLength = 0;
	char key[1024];

	PipelineSection *section = NULL;

	if((section = sectionGet(pipeline, pathName, sectionName)) != NULL) {
		log_logf(pipeline->log, LOG_LEVEL_ERROR,
				"{PIPELINE} unable to add new section '%s' path '%s', "
				"already exists", pathName, sectionName);
		return NULL;
	}

	section = (PipelineSection *)malloc(sizeof(PipelineSection));

	section->isLoaded = afalse;
	section->objectLength = 0;
	section->sectionLength = 0;
	section->name = strdup(sectionName);
	section->objects = NULL;
	section->childSections = NULL;

	keyLength = sectionBuildKey(pathName, sectionName, key);

	if(pipelineAddItem(pipeline, key, keyLength, PIPELINE_ITEM_TYPE_SECTION,
				(void *)section) < 0) {
		log_logf(pipeline->log, LOG_LEVEL_ERROR,
				"{PIPELINE} failed to add section '%s' path '%s' to the "
				"pipeline", sectionName, pathName);
		free(section);
		return NULL;
	}

	return section;
}

// object functions

static int objectBuildKey(char *pathName, char *objectName, char key[1024])
{
	int keyLength = 0;

	keyLength = snprintf(key, (sizeof(char) * 1023), "%s/%s", pathName,
			objectName);

	return keyLength;
}

static void objectFreeMemory(PipelineObject *object)
{
	log_logf(localLog, LOG_LEVEL_INFO, "{PIPELINE} render object '%s' "
			"{ %lli renders, %0.6f time, %0.6f average }",
			object->name,
			object->stats.executionCount,
			object->stats.totalExecutionTime,
			(object->stats.totalExecutionTime /
			 (double)object->stats.executionCount));

	if(object->name != NULL) {
		free(object->name);
	}

	memset(object, 0, (sizeof(PipelineObject)));
}

static PipelineObject *objectGet(Pipeline *pipeline, char *pathName,
		char *objectName)
{
	int keyLength = 0;
	char key[1024];

	PipelineItem *item = NULL;

	if(objectName != NULL) {
		keyLength = objectBuildKey(pathName, objectName, key);
	}
	else {
		keyLength = snprintf(key, (sizeof(char) * 1023), "%s", pathName);
	}

	if(bptree_get(&pipeline->index, key, keyLength, ((void *)&item)) < 0) {
		return NULL;
	}
	else if((item == NULL) || (item->type != PIPELINE_ITEM_TYPE_OBJECT)) {
		log_logf(pipeline->log, LOG_LEVEL_ERROR,
				"{PIPELINE} located null or invalid entry for object '%s' "
				"path '%s'", objectName, pathName);
		return NULL;
	}

	return (PipelineObject *)item->payload;
}

static PipelineObject *objectNew(Pipeline *pipeline, char *pathName,
		char *objectName)
{
	int keyLength = 0;
	char key[1024];

	PipelineObject *object = NULL;

	if((object = objectGet(pipeline, pathName, objectName)) != NULL) {
		log_logf(pipeline->log, LOG_LEVEL_ERROR,
				"{PIPELINE} unable to add new object '%s' path '%s', "
				"already exists", pathName, objectName);
		return NULL;
	}

	object = (PipelineObject *)malloc(sizeof(PipelineObject));

	object->isEnabled = atrue;
	object->type = PIPELINE_TYPE_UNKNOWN_OBJECT;
	object->name = strdup(objectName);
	object->localPayload = NULL;
	object->objectPtr = NULL;

	object->stats.executionCount = 0;
	object->stats.totalExecutionTime = 0.0;

	keyLength = objectBuildKey(pathName, objectName, key);

	if(pipelineAddItem(pipeline, key, keyLength, PIPELINE_ITEM_TYPE_OBJECT,
				(void *)object) < 0) {
		log_logf(pipeline->log, LOG_LEVEL_ERROR,
				"{PIPELINE} failed to add object '%s' path '%s' to the "
				"pipeline", objectName, pathName);
		free(object);
		return NULL;
	}

	return object;
}

// rendering functions

static void renderObject(PipelineObject *object)
{
	double timestamp = 0.0;

	timestamp = time_getTimeMus();

	if(object->type == PIPELINE_TYPE_2D_OBJECT) {
		draw2d_draw2dObject((Draw2dObject *)object->objectPtr);
	}
	else if(object->type == PIPELINE_TYPE_3D_OBJECT) {
		draw3d_draw3dObject((Draw3dObject *)object->objectPtr);
	}

	object->stats.totalExecutionTime += time_getElapsedMusInSeconds(timestamp);
	object->stats.executionCount += 1;
}

static void renderSection(Pipeline *pipeline, PipelineSection *section)
{
	int ii = 0;

	log_logf(pipeline->log, LOG_LEVEL_DEBUG,
			"{PIPELINE} render section '%s' with %i objects",
			section->name, section->objectLength);

	if(!section->isLoaded) {
		log_logf(pipeline->log, LOG_LEVEL_INFO,
				"{PIPELINE} render section '%s' is not loaded, aborting",
				section->name);
		return;
	}

	if(section->objects != NULL) {
		for(ii = 0; ii < section->objectLength; ii++) {
			if(section->objects[ii]->isEnabled) {
/*				log_logf(pipeline->log, LOG_LEVEL_DEBUG,
						"{PIPELINE} render section '%s' object '%s'",
						section->name, section->objects[ii]->name);*/
				renderObject(section->objects[ii]);
			}
		}
	}

	if(section->childSections != NULL) {
		for(ii = 0; ii < section->sectionLength; ii++) {
			renderSection(pipeline,
					(PipelineSection *)section->childSections[ii]);
		}
	}
}


// define pipeline public functions

int pipeline_init(Pipeline *pipeline, Canvas *canvas, AssetCache *assetCache,
		Log *log)
{
	if((pipeline == NULL) || (canvas == NULL) || (assetCache == NULL) ||
			(log == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	memset(pipeline, 0, (sizeof(Pipeline)));

	pipeline->hasConfig = afalse;
	pipeline->modeLength = 0;
	pipeline->currentMode = -1;
	pipeline->modes = NULL;
	pipeline->canvas = canvas;
	pipeline->assetCache = assetCache;
	pipeline->log = log;

	bptree_init(&pipeline->index);
	bptree_setFreeFunction(&pipeline->index, pipelineFreeIndexEntry);

	localLog = pipeline->log;

	mutex_init(&pipeline->mutex);

	return 0;
}

int pipeline_free(Pipeline *pipeline)
{
	if(pipeline == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&pipeline->mutex);

	bptree_free(&pipeline->index);

	if(pipeline->hasConfig) {
		config_free(&pipeline->config);
	}

	if(pipeline->modes != NULL) {
		free(pipeline->modes);
	}

	mutex_unlock(&pipeline->mutex);
	mutex_free(&pipeline->mutex);

	return 0;
}

int pipeline_loadConfiguration(Pipeline *pipeline, char *assetFilename)
{
	if((pipeline == NULL) || (assetFilename == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	/*
	 * TODO: this function
	 */

	return 0;
}

int pipeline_processConfigurationLoad(Pipeline *pipeline, char *modeName,
		aboolean *isComplete, double *completionPercentage)
{
	if((pipeline == NULL) || (modeName == NULL) || (isComplete == NULL) ||
			(completionPercentage == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	/*
	 * TODO: this function
	 */

	return 0;
}

int pipeline_addMode(Pipeline *pipeline, char *modeName)
{
	int modeRef = 0;

	PipelineMode *mode = NULL;

	if((pipeline == NULL) || (modeName == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&pipeline->mutex);

	if((mode = modeNew(pipeline, modeName)) == NULL) {
		mutex_unlock(&pipeline->mutex);
		return -1;
	}

	if(pipeline->modes == NULL) {
		modeRef = 0;
		pipeline->modeLength = 1;
		pipeline->modes = (PipelineMode **)malloc(
				sizeof(PipelineMode *) * pipeline->modeLength);
	}
	else {
		modeRef = pipeline->modeLength;
		pipeline->modeLength += 1;
		pipeline->modes = (PipelineMode **)realloc(pipeline->modes,
				(sizeof(PipelineMode *) * pipeline->modeLength));
	}

	mode->index = modeRef;

	pipeline->modes[modeRef] = mode;

	if(pipeline->currentMode == -1) {
		pipeline->currentMode = 0;
	}

	log_logf(pipeline->log, LOG_LEVEL_INFO,
			"{PIPELINE} added pipeline mode '%s'", modeName);

	mutex_unlock(&pipeline->mutex);

	return 0;
}

int pipeline_delMode(Pipeline *pipeline, char *modeName)
{
	int ii = 0;
	int nn = 0;

	PipelineMode *mode = NULL;

	if((pipeline == NULL) || (modeName == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&pipeline->mutex);

	if(pipeline->modeLength < 1) {
		log_logf(pipeline->log, LOG_LEVEL_ERROR,
				"{PIPELINE} unable to delete mode '%s' from pipeline, "
				"no modes", modeName);
		mutex_unlock(&pipeline->mutex);
		return -1;
	}

	if((mode = modeGet(pipeline, modeName)) == NULL) {
		log_logf(pipeline->log, LOG_LEVEL_ERROR,
				"{PIPELINE} unable to delete mode '%s' from pipeline, "
				"does not exist", modeName);
		mutex_unlock(&pipeline->mutex);
		return -1;
	}

	for(ii = 0, nn = 0; ii < pipeline->modeLength; ii++) {
		if(!strcmp(pipeline->modes[ii]->name, modeName)) {
			continue;
		}
		pipeline->modes[nn] = pipeline->modes[ii];
		nn++;
	}

	pipeline->modeLength -= 1;
	if(pipeline->modeLength <= 0) {
		free(pipeline->modes);
		pipeline->modeLength = 0;
		pipeline->modes = NULL;
	}

	if(bptree_remove(&pipeline->index, modeName, (int)strlen(modeName)) < 0) {
		log_logf(pipeline->log, LOG_LEVEL_ERROR,
				"{PIPELINE} unable to delete mode '%s' from pipeline, "
				"failed to remove from index", modeName);
		mutex_unlock(&pipeline->mutex);
		return -1;
	}

	log_logf(pipeline->log, LOG_LEVEL_INFO,
			"{PIPELINE} removed pipeline mode '%s'", modeName);

	mutex_unlock(&pipeline->mutex);

	return 0;
}

char *pipeline_getCurrentMode(Pipeline *pipeline)
{
	char *currentMode = NULL;

	if(pipeline == NULL) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	mutex_lock(&pipeline->mutex);

	if(pipeline->currentMode > -1) {
		currentMode = pipeline->modes[pipeline->currentMode]->name;
	}

	mutex_unlock(&pipeline->mutex);

	return currentMode;
}

int pipeline_switchMode(Pipeline *pipeline, char *modeName)
{
	PipelineMode *mode = NULL;

	if((pipeline == NULL) || (modeName == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&pipeline->mutex);

	if(pipeline->modeLength < 1) {
		log_logf(pipeline->log, LOG_LEVEL_ERROR,
				"{PIPELINE} unable to delete mode '%s' from pipeline, "
				"no modes", modeName);
		mutex_unlock(&pipeline->mutex);
		return -1;
	}

	if((mode = modeGet(pipeline, modeName)) == NULL) {
		log_logf(pipeline->log, LOG_LEVEL_ERROR,
				"{PIPELINE} unable to switch to pipeline mode '%s', "
				"does not exist", modeName);
		mutex_unlock(&pipeline->mutex);
		return -1;
	}

	pipeline->currentMode = mode->index;

	log_logf(pipeline->log, LOG_LEVEL_INFO,
			"{PIPELINE} switched to pipeline mode '%s'", mode->name);

	mutex_unlock(&pipeline->mutex);

	return 0;
}

int pipeline_addSection(Pipeline *pipeline, char *pathName, char *sectionName)
{
	int ref = 0;
	int pathFieldCount = 0;
	int pathNameLength = 0;
	int localPathLength = 0;
	char localPath[1024];

	PipelineRender *render = NULL;
	PipelineSection *section = NULL;
	PipelineSection *parentSection = NULL;

	if((pipeline == NULL) || (pathName == NULL) || (sectionName == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&pipeline->mutex);

	if(pipeline->modeLength < 1) {
		log_logf(pipeline->log, LOG_LEVEL_ERROR,
				"{PIPELINE} unable to add section '%s' path '%s' to "
				"pipeline, no modes", sectionName, pathName);
		mutex_unlock(&pipeline->mutex);
		return -1;
	}

	pathNameLength = (int)strlen(pathName);

	localPathLength = pathGetRender(pathName, pathNameLength, localPath);

	if((render = renderGet(pipeline, localPath)) == NULL) {
		log_logf(pipeline->log, LOG_LEVEL_ERROR,
				"{PIPELINE} unable to add section '%s' path '%s' to "
				"pipeline, render path '%s' does not exist",
				sectionName, pathName, localPath);
		mutex_unlock(&pipeline->mutex);
		return -1;
	}

	if((section = sectionNew(pipeline, pathName, sectionName)) == NULL) {
		mutex_unlock(&pipeline->mutex);
		return -1;
	}

	if(!pipeline->hasConfig) {
		section->isLoaded = atrue;
	}

	pathFieldCount = pathGetFieldCount(pathName, pathNameLength);
	if(pathFieldCount > 2) {
		if((parentSection = sectionGet(pipeline, pathName, NULL)) == NULL) {
			log_logf(pipeline->log, LOG_LEVEL_ERROR,
					"{PIPELINE} unable to add section '%s' path '%s' to "
					"pipeline, parent section '%s' does not exist",
					sectionName, pathName, pathName);
			mutex_unlock(&pipeline->mutex);
			return -1;
		}

		if(parentSection->childSections == NULL) {
			ref = 0;
			parentSection->sectionLength = 1;
			parentSection->childSections = (void **)malloc(
					sizeof(PipelineSection *) * parentSection->sectionLength);
		}
		else {
			ref = parentSection->sectionLength;
			parentSection->sectionLength += 1;
			parentSection->childSections = (void **)realloc(
					parentSection->childSections,
					(sizeof(PipelineSection *) * parentSection->sectionLength));
		}

		parentSection->childSections[ref] = (void *)section;
	}
	else {
		if(render->rootSections == NULL) {
			ref = 0;
			render->sectionLength = 1;
			render->rootSections = (PipelineSection **)malloc(
					sizeof(PipelineSection *) * render->sectionLength);
		}
		else {
			ref = render->sectionLength;
			render->sectionLength += 1;
			render->rootSections = (PipelineSection **)realloc(
					render->rootSections,
					(sizeof(PipelineSection *) * render->sectionLength));
		}

		render->rootSections[ref] = section;
	}

	log_logf(pipeline->log, LOG_LEVEL_INFO,
			"{PIPELINE} added section '%s' path '%s' to the pipeline",
			sectionName, pathName);

	mutex_unlock(&pipeline->mutex);

	return 0;
}

int pipeline_delSection(Pipeline *pipeline, char *pathName, char *sectionName)
{
	if((pipeline == NULL) || (pathName == NULL) || (sectionName == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	/*
	 * TODO: this function
	 */

	return 0;
}

int pipeline_addObject(Pipeline *pipeline, char *pathName, char *objectName,
		void *object, void *objectPayload)
{
	int rc = 0;
	int ref = 0;
	int pathNameLength = 0;
	char renderType[1024];

	PipelineObject *pipelineObject = NULL;
	PipelineSection *section = NULL;

	if((pipeline == NULL) || (pathName == NULL) || (objectName == NULL) ||
			(object == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&pipeline->mutex);

	if(pipeline->modeLength < 1) {
		log_logf(pipeline->log, LOG_LEVEL_ERROR,
				"{PIPELINE} unable to add object '%s' path '%s' to "
				"pipeline, no modes", objectName, pathName);
		mutex_unlock(&pipeline->mutex);
		return -1;
	}

	if((section = sectionGet(pipeline, pathName, NULL)) == NULL) {
		log_logf(pipeline->log, LOG_LEVEL_ERROR,
				"{PIPELINE} unable to add object '%s' path '%s' to pipeline, "
				"section '%s' does not exist",
				objectName, pathName, pathName);
		mutex_unlock(&pipeline->mutex);
		return -1;
	}

	if((pipelineObject = objectNew(pipeline, pathName, objectName)) == NULL) {
		log_logf(pipeline->log, LOG_LEVEL_ERROR,
				"{PIPELINE} unable to add object '%s' path '%s' to pipeline, "
				"object already exists",
				objectName, pathName);
		mutex_unlock(&pipeline->mutex);
		return -1;
	}

	pathNameLength = (int)strlen(pathName);

	if((rc = pathGetField(pathName, pathNameLength, 2, renderType)) < 1) {
		log_logf(pipeline->log, LOG_LEVEL_ERROR,
				"{PIPELINE} unable to add object '%s' path '%s' to pipeline, "
				"failed to determine render type",
				objectName, pathName);
		mutex_unlock(&pipeline->mutex);
		return -1;
	}

	if(!strcmp(renderType, "2d")) {
		pipelineObject->type = PIPELINE_TYPE_2D_OBJECT;
	}
	else if(!strcmp(renderType, "3d")) {
		pipelineObject->type = PIPELINE_TYPE_3D_OBJECT;
	}
	else {
		log_logf(pipeline->log, LOG_LEVEL_ERROR,
				"{PIPELINE} unable to add object '%s' path '%s' to pipeline, "
				"invalid render type '%s'",
				objectName, pathName, renderType);
		mutex_unlock(&pipeline->mutex);
		return -1;
	}

	pipelineObject->localPayload = objectPayload;
	pipelineObject->objectPtr = object;

	if(section->objects == NULL) {
		ref = 0;
		section->objectLength = 1;
		section->objects = (PipelineObject **)malloc(
				sizeof(PipelineObject **) * section->objectLength);
	}
	else {
		ref = section->objectLength;
		section->objectLength += 1;
		section->objects = (PipelineObject **)realloc(section->objects,
				(sizeof(PipelineObject **) * section->objectLength));
	}

	section->objects[ref] = pipelineObject;

	log_logf(pipeline->log, LOG_LEVEL_INFO,
			"{PIPELINE} added %s object '%s' path '%s' to the pipeline",
			renderType, objectName, pathName);

	mutex_unlock(&pipeline->mutex);

	return 0;
}

int pipeline_addEmptyObject(Pipeline *pipeline, char *pathName,
		char *objectName)
{
	if((pipeline == NULL) || (pathName == NULL) || (objectName == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	/*
	 * TODO: this function
	 */

	return 0;
}

int pipeline_setObject(Pipeline *pipeline, char *pathName, char *objectName,
		void *object)
{
	if((pipeline == NULL) || (pathName == NULL) || (objectName == NULL) ||
			(object == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	/*
	 * TODO: this function
	 */

	return 0;
}

int pipeline_enableObject(Pipeline *pipeline, char *pathName, char *objectName)
{
	if((pipeline == NULL) || (pathName == NULL) || (objectName == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	/*
	 * TODO: this function
	 */

	return 0;
}

int pipeline_disableObject(Pipeline *pipeline, char *pathName, char *objectName)
{
	if((pipeline == NULL) || (pathName == NULL) || (objectName == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	/*
	 * TODO: this function
	 */

	return 0;
}

aboolean pipeline_isObjectEnabled(Pipeline *pipeline, char *pathName,
		char *objectName)
{
	if((pipeline == NULL) || (pathName == NULL) || (objectName == NULL)) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	/*
	 * TODO: this function
	 */

	return afalse;
}

int pipeline_delObject(Pipeline *pipeline, char *pathName, char *objectName)
{
	if((pipeline == NULL) || (pathName == NULL) || (objectName == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	/*
	 * TODO: this function
	 */

	return 0;
}

int pipeline_render(Pipeline *pipeline)
{
	int ii = 0;
	int id = 0;
	int sectionLength = 0;

	if(pipeline == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&pipeline->mutex);

	id = pipeline->currentMode;
	if((id < 0) || (id >= pipeline->modeLength)) {
		log_logf(pipeline->log, LOG_LEVEL_ERROR,
				"{PIPELINE} render error, invalid current mode");
		id = 0;
	}

	log_logf(pipeline->log, LOG_LEVEL_DEBUG,
			"{PIPELINE} beginning render for mode (%i)'%s'",
			id, pipeline->modes[id]->name);

	// begin render

	canvas_beginRender(pipeline->canvas);

	// 3d objects

	canvas_toggleRenderMode3d(pipeline->canvas);

	sectionLength = pipeline->modes[id]->render3d.sectionLength;

	log_logf(pipeline->log, LOG_LEVEL_DEBUG,
			"{PIPELINE} render mode (%i)'%s' 3d sections: %i",
			id, pipeline->modes[id]->name, sectionLength);

	for(ii = 0; ii < sectionLength; ii++) {
		renderSection(pipeline,
				pipeline->modes[id]->render3d.rootSections[ii]);
	}

	// 2d objects

	canvas_toggleRenderMode2d(pipeline->canvas);

	sectionLength = pipeline->modes[id]->render2d.sectionLength;

	log_logf(pipeline->log, LOG_LEVEL_DEBUG,
			"{PIPELINE} render mode (%i)'%s' 2d sections: %i",
			id, pipeline->modes[id]->name, sectionLength);

	for(ii = 0; ii < sectionLength; ii++) {
		renderSection(pipeline,
				pipeline->modes[id]->render2d.rootSections[ii]);
	}

	// end render

	canvas_endRender(pipeline->canvas);

	mutex_unlock(&pipeline->mutex);

	return 0;
}

