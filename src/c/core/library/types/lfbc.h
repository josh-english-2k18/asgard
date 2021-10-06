/*
 * lfbc.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Low-Fragmentation Block-Cache (LFBC) is a block-based file cache
 * including a background-thread management system, header file.
 *
 * Written by Josh English.
 */

#if !defined(_CORE_LIBRARY_TYPES_LFBC_H)

#define _CORE_LIBRARY_TYPES_LFBC_H

#if !defined(_CORE_H) && !defined(_CORE_LIBRARY_H) && \
		!defined(_CORE_LIBRARY_TYPES_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _CORE_H || _CORE_LIBRARY_H || _CORE_LIBRARY_TYPES_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// define low-fragmentation block-cache constants

#define LFBC_STATE_INIT									1
#define LFBC_STATE_RUNNING								2
#define LFBC_STATE_SHUTDOWN								3
#define LFBC_STATE_STOPPED								4
#define LFBC_STATE_ERROR								-1

#define LFBC_STATE_DEFAULT_BLOCK_LENGTH					65536

#define LFBC_STATE_DEFAULT_MAX_BLOCKS					16384

#define LFBC_STATE_DEFAULT_MAX_OPEN_FILE_HANDLES		512


// define low-fragmentation block-cache data types

typedef struct _LfbcBlock {
	int lock;
	int length;
	double timestamp;
	char *block;
} LfbcBlock;

typedef struct _LfbcEntry {
	aboolean isFileHandleOpen;
	aboolean isLockedForReload;
	alint fileLength;
	alint blockCount;
	double mostRecentTimestamp;
	double lastModifiedTime;
	char *filename;
	LfbcBlock *blocks;
	FileHandle fh;
	Mutex mutex;
} LfbcEntry;

typedef struct _Lfbc {
	aboolean isLogInternallyAllocated;
	int state;
	int blockLength;
	int maxBlocks;
	int allocatedBlocks;
	int managerThreadId;
	int maxOpenFileHandles;
	int currentOpenFileHandles;
	Bptree index;
	ArrayList entries;
	ArrayList cache;
	TmfContext tmf;
	void *log;
	Mutex mutex;
} Lfbc;


// declare low-fragmentation block-cache public functions

void lfbc_init(Lfbc *cache, void *log);

void lfbc_free(Lfbc *cache);

int lfbc_getState(Lfbc *cache);

int lfbc_getBlockLength(Lfbc *cache);

void lfbc_setBlockLength(Lfbc *cache, int blockLength);

int lfbc_getMaxBlocks(Lfbc *cache);

void lfbc_setMaxBlocks(Lfbc *cache, int maxBlocks);

int lfbc_getMaxOpenFileHandles(Lfbc *cache);

void lfbc_setMaxOpenFileHandles(Lfbc *cache, int maxOpenFileHandles);

alint lfbc_getAllocatedMemory(Lfbc *cache);

void *cache_getLog(Lfbc *cache);

int lfbc_start(Lfbc *cache);

aboolean lfbc_hasCache(Lfbc *cache, char *filename, alint *fileLength,
		alint *blockCount);

char *lfbc_get(Lfbc *cache, char *filename, alint position,
		int *blockLength, alint *blockId);

int lfbc_releaseBlock(Lfbc *cache, char *filename, alint blockId);


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _CORE_LIBRARY_TYPES_LFBC_H

