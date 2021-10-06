/*
 * pipeline.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard Game Engine rendering pipeline management system, header file.
 *
 * Written by Josh English.
 */

#if !defined(_ASGARD_ENGINE_PIPELINE_H)

#define _ASGARD_ENGINE_PIPELINE_H

#if !defined(_ASGARD_H) && !defined(_ASGARD_ENGINE_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _ASGARD_H || _ASGARD_ENGINE_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// define pipeline public constants

typedef enum _PipelineTypes {
	PIPELINE_TYPE_2D_OBJECT = 1,
	PIPELINE_TYPE_3D_OBJECT,
	PIPELINE_TYPE_UNKNOWN_OBJECT = -1
} PipelineTypes;


// define pipeline public data types

typedef struct _PipelineObjectStats {
	alint executionCount;
	double totalExecutionTime;
} PipelineObjectStats;

typedef struct _PipelineObject {
	aboolean isEnabled;
	int type;
	char *name;
	void *localPayload;
	void *objectPtr;
	PipelineObjectStats stats;
} PipelineObject;

typedef struct _PipelineSection {
	aboolean isLoaded;
	int objectLength;
	int sectionLength;
	char *name;
	PipelineObject **objects;
	void **childSections;
} PipelineSection;

typedef struct _PipelineRender {
	int sectionLength;
	PipelineSection **rootSections;
} PipelineRender;

typedef struct _PipelineMode {
	int index;
	char *name;
	PipelineRender render2d;
	PipelineRender render3d;
} PipelineMode;

typedef struct _Pipeline {
	aboolean hasConfig;
	int modeLength;
	int currentMode;
	Bptree index;
	Config config;
	PipelineMode **modes;
	Canvas *canvas;
	AssetCache *assetCache;
	Log *log;
	Mutex mutex;
} Pipeline;


// define pipeline public functions

int pipeline_init(Pipeline *pipeline, Canvas *canvas, AssetCache *assetCache,
		Log *log);

int pipeline_free(Pipeline *pipeline);

int pipeline_loadConfiguration(Pipeline *pipeline, char *assetFilename);

int pipeline_processConfigurationLoad(Pipeline *pipeline, char *modeName,
		aboolean *isComplete, double *completionPercentage);

int pipeline_addMode(Pipeline *pipeline, char *modeName);

int pipeline_delMode(Pipeline *pipeline, char *modeName);

char *pipeline_getCurrentMode(Pipeline *pipeline);

int pipeline_switchMode(Pipeline *pipeline, char *modeName);

int pipeline_addSection(Pipeline *pipeline, char *pathName, char *sectionName);

int pipeline_delSection(Pipeline *pipeline, char *pathName, char *sectionName);

int pipeline_addObject(Pipeline *pipeline, char *pathName, char *objectName,
		void *object, void *objectPayload);

int pipeline_addEmptyObject(Pipeline *pipeline, char *pathName,
		char *objectName);

int pipeline_setObject(Pipeline *pipeline, char *pathName, char *objectName,
		void *object);

int pipeline_enableObject(Pipeline *pipeline, char *pathName, char *objectName);

int pipeline_disableObject(Pipeline *pipeline, char *pathName,
		char *objectName);

aboolean pipeline_isObjectEnabled(Pipeline *pipeline, char *pathName,
		char *objectName);

int pipeline_delObject(Pipeline *pipeline, char *pathName, char *objectName);

int pipeline_render(Pipeline *pipeline);


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _ASGARD_ENGINE_PIPELINE_H

