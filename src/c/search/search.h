/*
 * search.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The search engine common header file.
 *
 * Written by Josh English.
 */

#if !defined(_SEARCH_H)

#define _SEARCH_H

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// define search constants

// geocoord

#define GEOCOORD_INTEGER_CONVERSION						10000000.0

#define GEOCOORD_MILES_PER_DEGREE						69.0933

#define GEOCOORD_DEGREES_PER_MILE						0.014473169

#define GEOCOORD_PI								\
	3.141592653589793238462643383279502884

#define GEOCOORD_CIRCUMFRENCE_OF_EARTH_IN_MILES			24873.612

#define GEOCOORD_RADIUS_OF_EARTH_IN_MILES				3963.1

// search engine core

#define SEARCH_ENGINE_CORE_MAX_STRING_INDEX_LENGTH		1024

// searchd

#define SEARCHD_DEVELOPER_MODE								atrue
//#define SEARCHD_DEVELOPER_MODE								afalse

#define SEARCHD_DOMAIN_KEY								"searchd_domainKey"

#define SEARCHD_DOMAIN_KEY_LENGTH						17

#define SEARCHD_CONTAINER_UID							"searchd_containerUid"

#define SEARCHD_CONTAINER_NAME							"searchd_containerName"

// json language plugin

typedef enum _SearchJsonValidationType {
	SEARCH_JSON_VALIDATION_TYPE_IGNORE = 1,
	SEARCH_JSON_VALIDATION_TYPE_WARNING,
	SEARCH_JSON_VALIDATION_TYPE_STRICT,
	SEARCH_JSON_VALIDATION_TYPE_UNKNOWN = -1
} SearchJsonValidationType;



// define search macros

#define geocoord_degreesToRadians(degrees)		\
	((degrees) * (GEOCOORD_PI / 180.0))



// include search packages

// search core packages

#include "search/core/common.h"
#include "search/core/container.h"
#include "search/core/index_registry.h"
#include "search/core/authentication.h"

// search system packages

#include "search/system/search_index.h"
#include "search/system/search_engine.h"
#include "search/system/search_sort.h"
#include "search/system/search_compiler.h"

// search system language packages

#include "search/system/lang/search_json.h"
#include "search/system/lang/search_sql.h"


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _SEARCH_H

