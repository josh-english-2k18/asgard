/*
 * aac.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard Game Engine managed asset caching system.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#include "asgard/asgard.h"
#define _ASGARD_ENGINE_COMPONENT
#include "asgard/engine/aac.h"


// define asset cache private constants

typedef enum _AssetCacheLoadType {
	ASSET_CACHE_LOAD_TYPE_2D_COLOR = 1,
	ASSET_CACHE_LOAD_TYPE_2D_CUSTOM_COLOR,
	ASSET_CACHE_LOAD_TYPE_2D_FONT,
	ASSET_CACHE_LOAD_TYPE_2D_TEXTURE,
	ASSET_CACHE_LOAD_TYPE_3D_SHAPE_SPHERE,
	ASSET_CACHE_LOAD_TYPE_3D_SHAPE_CONE,
	ASSET_CACHE_LOAD_TYPE_UNKNOWN = -1
} AssetCacheLoadType;


// define asset cache private data types

typedef struct _AssetCacheLoadItem {
	int type;
	char *filename;
	char *name;
} AssetCacheLoadItem;


// declare asset cache private functions

// load items

static AssetCacheLoadItem *newLoadItem(int type, char *filename, char *name);

static void freeLoadItem(void *memory);

// cache entries

static AssetCacheEntry *newCacheEntry(int type, char *name, void *asset);

static void freeCacheEntry(void *memory);


// define asset cache private functions

// load items

static AssetCacheLoadItem *newLoadItem(int type, char *filename, char *name)
{
	AssetCacheLoadItem *result = NULL;

	result = (AssetCacheLoadItem *)malloc(sizeof(AssetCacheLoadItem));

	result->type = type;

	if(filename != NULL) {
		result->filename = strdup(filename);
	}
	else {
		result->filename = NULL;
	}

	result->name = strdup(name);

	return result;
}

static void freeLoadItem(void *memory)
{
	AssetCacheLoadItem *item = NULL;

	if(memory == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	item = (AssetCacheLoadItem *)memory;

	if(item->filename != NULL) {
		free(item->filename);
	}

	if(item->name != NULL) {
		free(item->name);
	}

	memset(item, 0, (sizeof(AssetCacheLoadItem)));

	free(item);
}

// cache entries

static AssetCacheEntry *newCacheEntry(int type, char *name, void *asset)
{
	AssetCacheEntry *result = NULL;

	result = (AssetCacheEntry *)malloc(sizeof(AssetCacheEntry));

	result->type = type;
	result->inUseCount = 0;
	result->name = strdup(name);
	result->asset = asset;

	return result;
}

static void freeCacheEntry(void *memory)
{
	AssetCacheEntry *entry = NULL;

	if(memory == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	entry = (AssetCacheEntry *)memory;

	if(entry->inUseCount > 0) {
		fprintf(stderr, "[%s():%i] error - unable to free asset '%s', "
				"still in-use by %i clients.\n", __FUNCTION__, __LINE__,
				entry->name, entry->inUseCount);
		return;
	}

	if(entry->name != NULL) {
		free(entry->name);
	}

	if(entry->asset != NULL) {
		switch(entry->type) {
			case ASSET_CACHE_ENTRY_TYPE_2D:
				draw2d_freeObject((Draw2dObject *)entry->asset);
				break;
			case ASSET_CACHE_ENTRY_TYPE_3D:
				draw3d_freeObject((Draw3dObject *)entry->asset);
				break;
		}
	}

	memset(entry, 0, (sizeof(AssetCacheEntry)));

	free(entry);
}


// define asset cache public functions

int aac_init(AssetCache *assetCache, Canvas *canvas, void *windowHandle,
		Log *log)
{
	if((assetCache == NULL) || (windowHandle == NULL) || (log == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	memset(assetCache, 0, (sizeof(AssetCache)));

	assetCache->log = log;
	assetCache->canvas = canvas;
	assetCache->windowHandle = windowHandle;

	fifostack_init(&assetCache->assetLoadQueue);
	fifostack_setFreeFunction(&assetCache->assetLoadQueue, freeLoadItem);

	bptree_init(&assetCache->cacheIndex);
	bptree_setFreeFunction(&assetCache->cacheIndex, freeCacheEntry);

	mutex_init(&assetCache->mutex);

	return 0;
}

int aac_free(AssetCache *assetCache)
{
	if(assetCache == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&assetCache->mutex);

	fifostack_free(&assetCache->assetLoadQueue);
	bptree_free(&assetCache->cacheIndex);

	mutex_unlock(&assetCache->mutex);
	mutex_free(&assetCache->mutex);

	memset(assetCache, 0, (sizeof(AssetCache)));

	return 0;
}

void aac_queueAssetLoad2dColor(AssetCache *assetCache, char *assetName,
		char *colorName)
{
	AssetCacheLoadItem *item = NULL;

	if((assetCache == NULL) || (assetName == NULL) || (colorName == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	mutex_lock(&assetCache->mutex);

	item = newLoadItem(ASSET_CACHE_LOAD_TYPE_2D_COLOR, assetName, colorName);
	if(fifostack_push(&assetCache->assetLoadQueue, (void *)item) < 0) {
		log_logf(assetCache->log, LOG_LEVEL_ERROR,
				"{AAC} failed to add color '%s' to load queue",
				colorName);
		freeLoadItem(item);
	}

	mutex_unlock(&assetCache->mutex);
}

void aac_queueAssetLoad2dCustomColor(AssetCache *assetCache,
		char *assetFilename, char *colorName)
{
	AssetCacheLoadItem *item = NULL;

	if((assetCache == NULL) || (assetFilename == NULL) ||
			(colorName == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	mutex_lock(&assetCache->mutex);

	item = newLoadItem(ASSET_CACHE_LOAD_TYPE_2D_CUSTOM_COLOR, assetFilename,
			colorName);
	if(fifostack_push(&assetCache->assetLoadQueue, (void *)item) < 0) {
		log_logf(assetCache->log, LOG_LEVEL_ERROR,
				"{AAC} failed to add custom color '%s', file '%s' to load "
				"queue", colorName, assetFilename);
		freeLoadItem(item);
	}

	mutex_unlock(&assetCache->mutex);
}

void aac_queueAssetLoad2dFont(AssetCache *assetCache, char *assetFilename,
		char *fontName)
{
	AssetCacheLoadItem *item = NULL;

	if((assetCache == NULL) || (assetFilename == NULL) || (fontName == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	mutex_lock(&assetCache->mutex);

	item = newLoadItem(ASSET_CACHE_LOAD_TYPE_2D_FONT, assetFilename,
			fontName);
	if(fifostack_push(&assetCache->assetLoadQueue, (void *)item) < 0) {
		log_logf(assetCache->log, LOG_LEVEL_ERROR,
				"{AAC} failed to add font '%s', file '%s' to load queue",
				fontName, assetFilename);
		freeLoadItem(item);
	}

	mutex_unlock(&assetCache->mutex);
}

void aac_queueAssetLoad2dTexture(AssetCache *assetCache, char *assetFilename,
		char *textureName)
{
	AssetCacheLoadItem *item = NULL;

	if((assetCache == NULL) || (assetFilename == NULL) ||
			(textureName == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	mutex_lock(&assetCache->mutex);

	item = newLoadItem(ASSET_CACHE_LOAD_TYPE_2D_TEXTURE, assetFilename,
			textureName);
	if(fifostack_push(&assetCache->assetLoadQueue, (void *)item) < 0) {
		log_logf(assetCache->log, LOG_LEVEL_ERROR,
				"{AAC} failed to add texture '%s', file '%s' to load queue",
				textureName, assetFilename);
		freeLoadItem(item);
	}

	mutex_unlock(&assetCache->mutex);
}

void aac_queueAssetLoad3dShapeSphere(AssetCache *assetCache,
		char *assetFilename, char *sphereName)
{
	AssetCacheLoadItem *item = NULL;

	if((assetCache == NULL) || (assetFilename == NULL) ||
			(sphereName == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	mutex_lock(&assetCache->mutex);

	item = newLoadItem(ASSET_CACHE_LOAD_TYPE_3D_SHAPE_SPHERE, assetFilename,
			sphereName);
	if(fifostack_push(&assetCache->assetLoadQueue, (void *)item) < 0) {
		log_logf(assetCache->log, LOG_LEVEL_ERROR,
				"{AAC} failed to add sphere file '%s' to load queue",
				assetFilename);
		freeLoadItem(item);
	}

	mutex_unlock(&assetCache->mutex);
}

void aac_queueAssetLoad3dShapeCone(AssetCache *assetCache,
		char *assetFilename, char *coneName)
{
	AssetCacheLoadItem *item = NULL;

	if((assetCache == NULL) || (assetFilename == NULL) ||
			(coneName == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	mutex_lock(&assetCache->mutex);

	item = newLoadItem(ASSET_CACHE_LOAD_TYPE_3D_SHAPE_CONE, assetFilename,
			coneName);
	if(fifostack_push(&assetCache->assetLoadQueue, (void *)item) < 0) {
		log_logf(assetCache->log, LOG_LEVEL_ERROR,
				"{AAC} failed to add cone file '%s' to load queue",
				assetFilename);
		freeLoadItem(item);
	}

	mutex_unlock(&assetCache->mutex);
}

int aac_getAssetLoadQueueLength(AssetCache *assetCache)
{
	alint result = 0;

	if(assetCache == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&assetCache->mutex);

	if(fifostack_getItemCount(&assetCache->assetLoadQueue, &result) < 0) {
		log_logf(assetCache->log, LOG_LEVEL_ERROR, "{AAC} failed to obtain "
				"valid item count from asset load queue");
		result = -1;
	}

	mutex_unlock(&assetCache->mutex);

	return (int)result;
}

void aac_processLoadAssetQueue(AssetCache *assetCache, int assetLoadCount)
{
	aboolean hasColor = afalse;
	int ii = 0;
	alint itemCount = 0;
	char *string = NULL;

	Color *color = NULL;
	Font *font = NULL;
	Texture *texture = NULL;
	Sphere3d *sphere = NULL;
	Cone3d *cone = NULL;
	Point3d location;
	Vertex3d rotation;
	Draw2dObject *object2d = NULL;
	Draw3dObject *object3d = NULL;
	AssetCacheLoadItem *item = NULL;
	AssetCacheEntry *entry = NULL;

	if((assetCache == NULL) || (assetLoadCount < 1)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	mutex_lock(&assetCache->mutex);

	if(fifostack_getItemCount(&assetCache->assetLoadQueue, &itemCount) < 0) {
		itemCount = -1;
	}

	if((int)itemCount < assetLoadCount) {
		log_logf(assetCache->log, LOG_LEVEL_INFO,
				"{AAC} found only %i items in load queue, unable to load "
				"%i assets", (int)itemCount, assetLoadCount);
		assetLoadCount = (int)itemCount;
	}

	for(ii = 0; ii < assetLoadCount; ii++) {
		item = NULL;
		entry = NULL;

		if(fifostack_pop(&assetCache->assetLoadQueue, ((void *)&item)) < 0) {
			log_logf(assetCache->log, LOG_LEVEL_ERROR,
					"{AAC} failed to pop item from asset load queue");
			break;
		}

		if(item == NULL) {
			continue;
		}

		log_logf(assetCache->log, LOG_LEVEL_DEBUG,
				"{AAC} obtained load item '%s' from queue", item->name);

		switch(item->type) {
			case ASSET_CACHE_LOAD_TYPE_2D_COLOR:
				color = colors_newByName(item->filename);
				object2d = draw2d_newObjectColor(assetCache->canvas, color);
				entry = newCacheEntry(ASSET_CACHE_ENTRY_TYPE_2D, item->name,
						(void *)object2d);
				break;
			case ASSET_CACHE_LOAD_TYPE_2D_CUSTOM_COLOR:
				color = colors_load(assetCache->log, item->filename);
				if(color != NULL) {
					object2d = draw2d_newObjectColor(assetCache->canvas, color);
					entry = newCacheEntry(ASSET_CACHE_ENTRY_TYPE_2D, item->name,
							(void *)object2d);
				}
				else {
					colors_free(color);
				}
				break;
			case ASSET_CACHE_LOAD_TYPE_2D_FONT:
				color = colors_new();
				font = font_load(assetCache->log, assetCache->windowHandle,
						item->filename, &hasColor, color);
				if(font != NULL) {
					string = strdup("font-loaded");
					if(hasColor) {
						object2d = draw2d_newObjectColorFont(
								assetCache->canvas,
								0,
								0,
								string,
								color,
								font);
					}
					else {
						object2d = draw2d_newObjectFont(
								assetCache->canvas,
								0,
								0,
								string,
								font);
					}
					entry = newCacheEntry(ASSET_CACHE_ENTRY_TYPE_2D, item->name,
							(void *)object2d);
				}
				else {
					colors_free(color);
				}
				break;
			case ASSET_CACHE_LOAD_TYPE_2D_TEXTURE:
				texture = texture_load(assetCache->log, item->filename);
				if(texture != NULL) {
					object2d = draw2d_newObjectTexture(
							assetCache->canvas,
							0,
							0,
							texture->width,
							texture->height,
							texture);
					entry = newCacheEntry(ASSET_CACHE_ENTRY_TYPE_2D, item->name,
							(void *)object2d);
				}
				break;
			case ASSET_CACHE_LOAD_TYPE_3D_SHAPE_SPHERE:
				sphere = sphere3d_load(assetCache->log, item->filename);
				if(sphere != NULL) {
					location.x = sphere->center.x;
					location.y = sphere->center.y;
					location.z = sphere->center.z;

					rotation.x = 0.0;
					rotation.y = 0.0;
					rotation.z = 0.0;

					object3d = draw3d_newObjectSphereFromObject(
							&location,
							&rotation,
							sphere);

					entry = newCacheEntry(ASSET_CACHE_ENTRY_TYPE_3D, item->name,
							(void *)object3d);
				}
				break;
			case ASSET_CACHE_LOAD_TYPE_3D_SHAPE_CONE:
				cone = cone3d_load(assetCache->log, item->filename);
				if(cone != NULL) {
					location.x = cone->center.x;
					location.y = cone->center.y;
					location.z = cone->center.z;

					rotation.x = 0.0;
					rotation.y = 0.0;
					rotation.z = 0.0;

					object3d = draw3d_newObjectConeFromObject(
							&location,
							&rotation,
							cone);

					entry = newCacheEntry(ASSET_CACHE_ENTRY_TYPE_3D, item->name,
							(void *)object3d);
				}
				break;
		}

		if(entry != NULL) {
			if(bptree_put(&assetCache->cacheIndex, entry->name,
						(int)strlen(entry->name), (void *)entry) < 0) {
				log_logf(assetCache->log, LOG_LEVEL_ERROR,
						"{AAC} failed to add entry '%s' type %i to the asset "
						"cache index", entry->name, entry->type);
				freeCacheEntry(entry);
			}
			else {
				log_logf(assetCache->log, LOG_LEVEL_DEBUG,
						"{AAC} added entry '%s' type %i to the asset cache "
						"index", entry->name, entry->type);
			}
		}

		freeLoadItem(item);
	}

	mutex_unlock(&assetCache->mutex);
}

Draw2dObject *aac_getAsset2d(AssetCache *assetCache, char *assetName)
{
	Draw2dObject *result = NULL;
	AssetCacheEntry *entry = NULL;

	if((assetCache == NULL) || (assetName == NULL)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	mutex_lock(&assetCache->mutex);

	bptree_get(&assetCache->cacheIndex, assetName, (int)strlen(assetName),
			((void *)&entry));
	if((entry != NULL) && (entry->type == ASSET_CACHE_ENTRY_TYPE_2D)) {
		entry->inUseCount += 1;
		result = (Draw2dObject *)entry->asset;
	}

	mutex_unlock(&assetCache->mutex);

	return result;
}

Draw3dObject *aac_getAsset3d(AssetCache *assetCache, char *assetName)
{
	Draw3dObject *result = NULL;
	AssetCacheEntry *entry = NULL;

	if((assetCache == NULL) || (assetName == NULL)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	mutex_lock(&assetCache->mutex);

	bptree_get(&assetCache->cacheIndex, assetName, (int)strlen(assetName),
			((void *)&entry));
	if((entry != NULL) && (entry->type == ASSET_CACHE_ENTRY_TYPE_3D)) {
		entry->inUseCount += 1;
		result = (Draw3dObject *)entry->asset;
	}

	mutex_unlock(&assetCache->mutex);

	return result;
}

void aac_releaseAsset(AssetCache *assetCache, char *assetName)
{
	AssetCacheEntry *entry = NULL;

	if((assetCache == NULL) || (assetName == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	mutex_lock(&assetCache->mutex);

	bptree_get(&assetCache->cacheIndex, assetName, (int)strlen(assetName),
			((void *)&entry));
	if(entry != NULL) {
		if(entry->inUseCount > 0) {
			entry->inUseCount -= 1;
			log_logf(assetCache->log, LOG_LEVEL_INFO,
					"released asset '%s', current usage count is %i",
					entry->name, entry->inUseCount);
		}
		else {
			entry->inUseCount = 0;
			log_logf(assetCache->log, LOG_LEVEL_ERROR,
					"unable to release asset '%s', not in-use",
					entry->name);
		}
	}

	mutex_unlock(&assetCache->mutex);
}

