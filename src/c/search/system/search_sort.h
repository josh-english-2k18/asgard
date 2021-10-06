/*
 * search_sort.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The sorting component for the search-engine, header file.
 *
 * Written by Josh English.
 */

#if !defined(_SEARCH_SYSTEM_SEARCH_SORT_H)

#define _SEARCH_SYSTEM_SEARCH_SORT_H

#if !defined(_SEARCH_H) && !defined(_SEARCH_SYSTEM_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _SEARCH_H || _SEARCH_SYSTEM_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// define search sort public constants

typedef enum _SearchSortTypes {
	SEARCH_SORT_TYPE_ATTRIBUTE = 1,
	SEARCH_SORT_TYPE_RELEVANCY,
	SEARCH_SORT_TYPE_DISTANCE,
	SEARCH_SORT_TYPE_UNKNOWN = -1
} SearchSortTypes;


// define search sort public data types

typedef struct _SearchSortFacetEntry {
	int count;
	char *value;
} SearchSortFacetEntry;

typedef struct _SearchSortEntry {
	int uid;
	void *context;
	void **items;
} SearchSortEntry;

typedef struct _SearchSortConfig {
	aboolean isStandardOrder;
	SearchSortTypes type;
} SearchSortConfig;

typedef struct _SearchSort {
	int length;
	int configLength;
	SearchSortConfig *config;
	SearchSortEntry *entries;
	Intersect *intersect;
	SearchEngine *engine;
} SearchSort;


// delcare search sort public functions

// direct sort functions

int searchSort_sortByAttributeName(SearchEngine *engine, char *attribute,
		Intersect *intersect, aboolean isStandardOrder);

int searchSort_sortByRelevancy(SearchEngine *engine, char *attribute,
		char *value, Intersect *intersect, aboolean isStandardOrder);

int searchSort_sortByDistance(SearchEngine *engine, double latitude,
		double longitude, Intersect *intersect, aboolean isStandardOrder);

// multi-value sort functions

int searchSort_mvInit(SearchSort *sort, SearchEngine *engine,
		Intersect *intersect);

int searchSort_mvFree(SearchSort *sort);

int searchSort_mvAddSortByAttributeName(SearchSort *sort, char *attribute,
		aboolean isStandardOrder);

int searchSort_mvAddSortByRelevancy(SearchSort *sort, char *attribute,
		char *value, aboolean isStandardOrder);

int searchSort_mvAddSortByDistance(SearchEngine *engine, SearchSort *sort,
		double latitude, double longitude, aboolean isStandardOrder);

int searchSort_mvExecute(SearchSort *sort);

// facet functions

int searchSort_calculateFacets(SearchEngine *engine, Intersect *intersect,
		char *attribute, SearchSortFacetEntry ***facets);

// helper functions

alint searchSort_calculateMvInitMemoryLength(Intersect *intersect);

alint searchSort_calculateMvAddSortByAttributeNameMemoryLength(
		SearchSort *sort, char *attribute, alint memoryLengthLimit);

alint searchSort_calculateMvAddSortByRelevancyMemoryLength(SearchSort *sort);

alint searchSort_calculateMvAddSortByDistanceMemoryLength(SearchSort *sort);

double searchSort_calculateGeoCoordDistanceMiles(double latitudeOne,
		double longitudeOne, double latitudeTwo, double longitudeTwo);

aboolean searchSort_determineGeoCoordsOnEntity(SearchEngine *engine,
		double latitude, double longitude,
		int uidListId, int *uidList, int uidListLength,
		double *resultLatitude, double *resultLongitude,
		double *resultDistance);


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _SEARCH_SYSTEM_SEARCH_SORT_H

