/*
 * aac.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard Game Engine managed asset caching system, header file.
 *
 * Written by Josh English.
 *
 * TODO: add a function to free all released assets
 */

#if !defined(_ASGARD_ENGINE_AAC_H)

#define _ASGARD_ENGINE_AAC_H

#if !defined(_ASGARD_H) && !defined(_ASGARD_ENGINE_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _ASGARD_H || _ASGARD_ENGINE_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// define asset cache public constants

typedef enum _AssetCacheEntryType {
	ASSET_CACHE_ENTRY_TYPE_2D = 1,
	ASSET_CACHE_ENTRY_TYPE_3D,
	ASSET_CACHE_ENTRY_UNKNOWN = -1
} AssetCacheEntryType;


// define asset cache public data types

typedef struct _AssetCacheEntry {
	int type;
	int inUseCount;
	char *name;
	void *asset;
} AssetCacheEntry;

typedef struct _AssetCache {
	FifoStack assetLoadQueue;
	Bptree cacheIndex;
	Log *log;
	Canvas *canvas;
	Mutex mutex;
	void *windowHandle;
} AssetCache;


// declare asset cache public functions

int aac_init(AssetCache *assetCache, Canvas *canvas, void *windowHandle,
		Log *log);

int aac_free(AssetCache *assetCache);

void aac_queueAssetLoad2dColor(AssetCache *assetCache, char *assetName,
		char *colorName);

void aac_queueAssetLoad2dCustomColor(AssetCache *assetCache,
		char *assetFilename, char *colorName);

void aac_queueAssetLoad2dFont(AssetCache *assetCache, char *assetFilename,
		char *fontName);

void aac_queueAssetLoad2dTexture(AssetCache *assetCache, char *assetFilename,
		char *textureName);

void aac_queueAssetLoad3dShapeSphere(AssetCache *assetCache,
		char *assetFilename, char *sphereName);

void aac_queueAssetLoad3dShapeCone(AssetCache *assetCache,
		char *assetFilename, char *coneName);

int aac_getAssetLoadQueueLength(AssetCache *assetCache);

void aac_processLoadAssetQueue(AssetCache *assetCache, int assetLoadCount);

Draw2dObject *aac_getAsset2d(AssetCache *assetCache, char *assetName);

Draw3dObject *aac_getAsset3d(AssetCache *assetCache, char *assetName);

void aac_releaseAsset(AssetCache *assetCache, char *assetName);


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _ASGARD_ENGINE_AAC_H

