/*
 * search_engine.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The engine component for the search-engine, header file.
 *
 * Written by Josh English.
 */

#if !defined(_SEARCH_SYSTEM_SEARCH_ENGINE_H)

#define _SEARCH_SYSTEM_SEARCH_ENGINE_H

#if !defined(_SEARCH_H) && !defined(_SEARCH_SYSTEM_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _SEARCH_H || _SEARCH_SYSTEM_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// define search engine public constants

typedef enum _SearchEngineErrorCodes {
	SEARCH_ENGINE_ERROR_OK = 0,
	SEARCH_ENGINE_ERROR_DEFAULT_ERROR = -1,
	SEARCH_ENGINE_ERROR_CORE_INVALID_ARGUMENTS = -100,
	SEARCH_ENGINE_ERROR_CORE_READ_LOCK_FAILURE = -101,
	SEARCH_ENGINE_ERROR_CORE_READ_UNLOCK_FAILURE = -102,
	SEARCH_ENGINE_ERROR_CORE_WRITE_LOCK_FAILURE = -103,
	SEARCH_ENGINE_ERROR_CORE_WRITE_UNLOCK_FAILURE = -104,
	SEARCH_ENGINE_ERROR_CORE_MISSING_UID = -105,
	SEARCH_ENGINE_ERROR_CORE_INVALID_UID = -106,
	SEARCH_ENGINE_ERROR_CORE_INVALID_DIRECTORY = -107,
	SEARCH_ENGINE_ERROR_CORE_FAILED_TO_INIT_CONFIG = -108,
	SEARCH_ENGINE_ERROR_CORE_STATE_WRITE_FAILURE = -109,
	SEARCH_ENGINE_ERROR_CORE_STATE_READ_FAILURE = -110,
	SEARCH_ENGINE_ERROR_CORE_DATA_WRITE_FAILURE = -111,
	SEARCH_ENGINE_ERROR_CORE_DATA_READ_FAILURE = -112,
	SEARCH_ENGINE_ERROR_CORE_INVALID_GEO_COORD_DISTANCE_TYPE = -113,
	SEARCH_ENGINE_ERROR_CORE_CONTAINER_DOES_NOT_EXIST = -120,
	SEARCH_ENGINE_ERROR_CORE_CONTAINER_ALREADY_EXISTS = -121,
	SEARCH_ENGINE_ERROR_CORE_CONTAINER_LOCK_FAILURE = -122,
	SEARCH_ENGINE_ERROR_CORE_CONTAINER_HAS_RESERVED_KEY = -123,
	SEARCH_ENGINE_ERROR_CORE_CONTAINER_MISSING_ATTRIBUTE = -124,
	SEARCH_ENGINE_ERROR_CORE_FAILED_TO_PUT_CONTAINER = -125,
	SEARCH_ENGINE_ERROR_CORE_FAILED_TO_LOCATE_CONTAINER = -126,
	SEARCH_ENGINE_ERROR_CORE_FAILED_TO_DELETE_CONTAINER = -127,
	SEARCH_ENGINE_ERROR_CORE_FAILED_TO_UPDATE_CONTAINER_INDEXES = -128,
	SEARCH_ENGINE_ERROR_CORE_FAILED_TO_COMBINE_CONTAINERS = -129,
	SEARCH_ENGINE_ERROR_CORE_CONTAINER_HAS_DUPLICATE_USER_KEY_INDEX = -130,
	SEARCH_ENGINE_ERROR_COMMON_FAILED_TO_BUILD_INDEX_STRING = -200,
	SEARCH_ENGINE_ERROR_COMMON_FAILED_TO_NORMALIZE_STRING = -201,
	SEARCH_ENGINE_ERROR_COMMON_FAILED_TO_BUILD_INDEX_KEY = -202,
	SEARCH_ENGINE_ERROR_COMMON_FAILED_TO_BUILD_UID_KEY = -203,
	SEARCH_ENGINE_ERROR_COMMON_FAILED_TO_BUILD_RELEVANCY_KEY = -204,
	SEARCH_ENGINE_ERROR_COMMON_FAILED_TO_BUILD_RELEVANCY_LENGTH_KEY = -205,
	SEARCH_ENGINE_ERROR_COMMON_FAILED_TO_DETERMINE_DATA_TYPE = -206,
	SEARCH_ENGINE_ERROR_COMMON_FAILED_TO_PAD_STRING = -207,
	SEARCH_ENGINE_ERROR_COMMON_FAILED_TO_TOKENIZE_STRING = -208,
	SEARCH_ENGINE_ERROR_INDEX_ALREADY_EXISTS = -300,
	SEARCH_ENGINE_ERROR_INDEX_INIT_FAILURE = -301,
	SEARCH_ENGINE_ERROR_INDEX_FAILED_TO_LOCATE = -302,
	SEARCH_ENGINE_ERROR_INDEX_FAILED_TO_CREATE = -303,
	SEARCH_ENGINE_ERROR_INDEX_FAILED_TO_REINDEX = -304,
	SEARCH_ENGINE_ERROR_INDEX_FAILED_TO_REMOVE = -305,
	SEARCH_ENGINE_ERROR_INDEX_FAILED_TO_REMOVE_REGISTRY = -306,
	SEARCH_ENGINE_ERROR_INDEX_TYPE_NOT_USER_KEY = -307,
	SEARCH_ENGINE_ERROR_INDEX_TYPE_INVALID = -308,
	SEARCH_ENGINE_ERROR_INDEX_RESET_FAILURE = -309,
	SEARCH_ENGINE_ERROR_INDEX_RENAME_FAILURE = -310,
	SEARCH_ENGINE_ERROR_DOMAIN_ALREADY_EXISTS = -400,
	SEARCH_ENGINE_ERROR_DOMAIN_FAILED_TO_LOCATE = -401,
	SEARCH_ENGINE_ERROR_DOMAIN_FAILED_TO_INIT = -402,
	SEARCH_ENGINE_ERROR_DOMAIN_FAILED_TO_CREATE = -403,
	SEARCH_ENGINE_ERROR_DOMAIN_FAILED_TO_CREATE_INDEX = -404,
	SEARCH_ENGINE_ERROR_DOMAIN_RENAME_FAILURE = -405,
	SEARCH_ENGINE_ERROR_DOMAIN_REMOVE_FAILURE = -406,
	SEARCH_ENGINE_ERROR_UNKNOWN = -1024
} SearchEngineErrorCodes;

typedef enum _SearchEngineGeoCoordTypes {
	SEARCH_ENGINE_GEO_COORD_TYPE_MILES = 1,
	SEARCH_ENGINE_GEO_COORD_TYPE_YARDS,
	SEARCH_ENGINE_GEO_COORD_TYPE_FEET,
	SEARCH_ENGINE_GEO_COORD_TYPE_KILOMETERS,
	SEARCH_ENGINE_GEO_COORD_TYPE_METERS,
	SEARCH_ENGINE_GEO_COORD_TYPE_UNKNOWN = -1
} SearchEngineGeoCoordTypes;

#define SEARCH_ENGINE_CONTAINER_FILE_MAGIC_NUMBER		53468721

#define SEARCH_ENGINE_DEFAULT_MAX_CONTAINERS			65536

#define SEARCH_ENGINE_DEFAULT_MAX_CONTAINER_MEMORY		65536

#define SEARCH_ENGINE_DEFAULT_CONTAINER_TIMEOUT			65536.0

#define SEARCH_ENGINE_DEFAULT_MIN_STRING_LENGTH			3

#define SEARCH_ENGINE_DEFAULT_MAX_STRING_LENGTH			18

#define SEARCH_ENGINE_DEFAULT_MAX_SORT_OPERATION_LENGTH	\
	8388608

#define SEARCH_ENGINE_DEFAULT_DELIMITERS				" |~,;:.][)(}{*@!&-_"

#define SEARCH_ENGINE_DEFAULT_STATE_BACKUP_SECONDS		1200.0

#define SEARCH_ENGINE_DEFAULT_STATE_BACKUP_PATH			"assets/data/searchd"

#define SEARCH_ENGINE_DEFAULT_DATA_BACKUP_SECONDS		1200.0

#define SEARCH_ENGINE_DEFAULT_DATA_BACKUP_PATH			"assets/data/searchd"

#define SEARCH_ENGINE_DEFAULT_EXCLUDED_WORDS_CONFIG		\
	"conf/searchd.excluded.words.default.config"

#define SEARCH_ENGINE_DEFAULT_AUTHENTICATION_CONFIG		\
	"conf/searchd.authentication.default.json"

#define SEARCH_ENGINE_DEFAULT_VALIDATION_TYPE_STRING	"warning"

#define SEARCH_ENGINE_DEFAULT_VALIDATION_TYPE			\
	SEARCH_JSON_VALIDATION_TYPE_WARNING


// define search engine public data types

typedef struct _SearchEngineSettings {
	aboolean isLogInternallyAllocated;
	SearchJsonValidationType validationType;
	int workerThreadId;
	int minStringLength;
	int maxStringLength;
	int uidCounter;
	int delimitersLength;
	alint maxSortOperationMemoryLength;
	double stateWriteThresholdSeconds;
	double containerWriteThresholdSeconds;
	char *delimiters;
	char *statePath;
	char *containerPath;
	char *excludedWordConfigFilename;
	char *authenticationConfigFilename;
	Mutex mutex;
} SearchEngineSettings;

typedef struct _SearchEngine {
	int domainLength;
	IndexRegistry registry;
	AuthSystem auth;
	Bptree indexes;
	Bptree excludedWords;
	SearchIndex **domains;
	ManagedIndex containers;
	SearchEngineSettings settings;
	FifoStack queue;
	TmfContext tmf;
	Esa esa;
	Log *log;
	Mutex queueMutex;
	Spinlock lock;
} SearchEngine;


// delcare search engine public functions

int searchEngine_init(SearchEngine *engine, Log *log);

SearchEngine *searchEngine_new(Log *log);

int searchEngine_free(SearchEngine *engine);

int searchEngine_freePtr(SearchEngine *engine);

char *searchEngine_errorCodeToString(SearchEngineErrorCodes code);

char *searchEngine_geoCoordTypeToString(SearchEngineGeoCoordTypes type);

aboolean searchEngine_isLocked(SearchEngine *engine);

aboolean searchEngine_isReadLocked(SearchEngine *engine);

int searchEngine_lock(SearchEngine *engine);

int searchEngine_unlock(SearchEngine *engine);

int searchEngine_setMaxContainerCount(SearchEngine *engine,
		alint maxCachedItemCount);

int searchEngine_setMaxContainerMemoryLength(SearchEngine *engine,
		alint maxCachedMemoryLength);

int searchEngine_setContainerTimeout(SearchEngine *engine,
		double timeoutSeconds);

int searchEngine_setStringIndexThresholds(SearchEngine *engine,
		int minStringLength, int maxStringLength);

int searchEngine_setMaxSortOperationMemoryLength(SearchEngine *engine,
		alint maxSortOperationMemoryLength);

int searchEngine_setStringDelimiters(SearchEngine *engine, char *delimiters,
		int delimitersLength);

int searchEngine_setStateBackupAttributes(SearchEngine *engine,
		double stateWriteThresholdSeconds, char *statePath);

int searchEngine_setIndexBackupAttributes(SearchEngine *engine,
		double containerWriteThresholdSeconds, char *containerPath);

int searchEngine_setJsonLangValidationType(SearchEngine *engine,
		SearchJsonValidationType validationType);

int searchEngine_loadExcludedWords(SearchEngine *engine, char *filename);

int searchEngine_loadAuthentication(SearchEngine *engine, char *filename);

int searchEngine_saveAuthentication(SearchEngine *engine);

int searchEngine_backupState(SearchEngine *engine);

int searchEngine_restoreState(SearchEngine *engine, char *filename);

int searchEngine_backupData(SearchEngine *engine);

int searchEngine_restoreData(SearchEngine *engine, char *filename);

int searchEngine_newIndex(SearchEngine *engine, IndexRegistryTypes type,
		char *key, char *name);

int searchEngine_newUserIndex(SearchEngine *engine, char *key, char *name);

int searchEngine_removeIndex(SearchEngine *engine, char *key);

int searchEngine_renameIndex(SearchEngine *engine, char *key, char *newKey,
		char *newName);

int searchEngine_setIndexStringThresholds(SearchEngine *engine, char *key,
		int minStringLength, int maxStringLength);

int searchEngine_resetIndex(SearchEngine *engine, char *key,
		IndexRegistryTypes newType, char *newKey, char *newName);

char **searchEngine_listIndexes(SearchEngine *engine, int *indexLength,
		int *errorCode);

int searchEngine_removeIndex(SearchEngine *engine, char *key);

IndexRegistryIndex *searchEngine_getIndex(SearchEngine *engine, char *indexKey,
		int *errorCode);

int searchEngine_getIndexType(SearchEngine *engine, char *indexKey);

int searchEngine_getIndexCount(SearchEngine *engine, char *domainKey,
		char *indexKey);

int searchEngine_getIndexDepth(SearchEngine *engine, char *domainKey,
		char *indexKey);

int searchEngine_getIndexUniqueValueCount(SearchEngine *engine,
		char *domainKey, char *indexKey);

int searchEngine_getIndexUniqueValueSummary(SearchEngine *engine,
		char *domainKey, char *indexKey, SearchIndexSummary *summary);

int searchEngine_newDomain(SearchEngine *engine, char *key, char *name);

int searchEngine_renameDomain(SearchEngine *engine, char *domainKey,
		char *newKey, char *newName);

int searchEngine_removeDomain(SearchEngine *engine, char *domainKey);

char **searchEngine_listDomains(SearchEngine *engine, int *domainLength,
		int *errorCode);

Container *searchEngine_get(SearchEngine *engine, int uid, int *errorCode);

Container *searchEngine_getByAttribute(SearchEngine *engine, char *attribute,
		char *value, int *errorCode);

int searchEngine_unlockGet(SearchEngine *engine, Container *container);

int searchEngine_put(SearchEngine *engine, char *domainKey,
		Container *container, aboolean isImmediate);

int searchEngine_update(SearchEngine *engine, char *domainKey,
		Container *container, aboolean isImmediate, char flags);

int searchEngine_updateByAttribute(SearchEngine *engine, char *domainKey,
		char *attribute, Container *container, aboolean isImmediate,
		char flags);

int searchEngine_delete(SearchEngine *engine, int uid);

int searchEngine_deleteByAttribute(SearchEngine *engine, char *attribute,
		char *value);

int searchEngine_searchNotList(SearchEngine *engine, char *domainKey,
		char *indexKey, Intersect *intersect);

int searchEngine_searchNotListGlobal(SearchEngine *engine, char *indexKey,
		Intersect *intersect);

int searchEngine_search(SearchEngine *engine, char *domainKey, char *attribute,
		char *value, Intersect *intersect);

int searchEngine_searchGlobal(SearchEngine *engine, char *attribute,
		char *value, Intersect *intersect);

int searchEngine_searchRange(SearchEngine *engine, char *domainKey,
		char *attribute, int searchType, int minValue, int maxValue,
		Intersect *intersect);

int searchEngine_searchRangeGlobal(SearchEngine *engine, char *attribute,
		int searchType, int minValue, int maxValue, Intersect *intersect);

int searchEngine_searchRangeGeocoord(SearchEngine *engine, char *domainKey,
		SearchEngineGeoCoordTypes type, double latitude, double longitude,
		double distance, Intersect *intersect);

int searchEngine_searchRangeGeocoordGlobal(SearchEngine *engine,
		SearchEngineGeoCoordTypes type, double latitude, double longitude,
		double distance, Intersect *intersect);


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _SEARCH_SYSTEM_SEARCH_ENGINE_H

