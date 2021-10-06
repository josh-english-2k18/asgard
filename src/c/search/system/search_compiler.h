/*
 * search_compiler.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The component for the search-engine to compile a given input syntax into
 * commands for execution within the search environment, header file.
 *
 * Written by Josh English.
 */

#if !defined(_SEARCH_SYSTEM_SEARCH_COMPILER_H)

#define _SEARCH_SYSTEM_SEARCH_COMPILER_H

#if !defined(_SEARCH_H) && !defined(_SEARCH_SYSTEM_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _SEARCH_H || _SEARCH_SYSTEM_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// define search compiler public constants

typedef enum _SearchCompilerActions {
	SEARCH_COMPILER_ACTION_NEW_DOMAIN = 1,
	SEARCH_COMPILER_ACTION_NEW_INDEX,
	SEARCH_COMPILER_ACTION_GET,
	SEARCH_COMPILER_ACTION_GET_BY_ATTRIBUTE,
	SEARCH_COMPILER_ACTION_PUT,
	SEARCH_COMPILER_ACTION_UPDATE,
	SEARCH_COMPILER_ACTION_UPDATE_BY_ATTRIBUTE,
	SEARCH_COMPILER_ACTION_DELETE,
	SEARCH_COMPILER_ACTION_DELETE_BY_ATTRIBUTE,
	SEARCH_COMPILER_ACTION_SEARCH,
	SEARCH_COMPILER_ACTION_LIST,
	SEARCH_COMPILER_ACTION_COMMAND,
	SEARCH_COMPILER_ACTION_LOGIN,
	SEARCH_COMPILER_ACTION_LOGOUT,
	SEARCH_COMPILER_ACTION_ERROR = -1
} SearchCompilerActions;

typedef enum _SearchCompilerCommandTypes {
	SEARCH_COMPILER_COMMAND_TYPE_INDEX = 1,
	SEARCH_COMPILER_COMMAND_TYPE_DOMAIN,
	SEARCH_COMPILER_COMMAND_TYPE_SERVER,
	SEARCH_COMPILER_COMMAND_TYPE_ERROR = -1
} SearchCompilerCommandTypes;

typedef enum _SearchCompilerCommandActions {
	SEARCH_COMPILER_COMMAND_ACTION_UPDATE = 1,
	SEARCH_COMPILER_COMMAND_ACTION_RENAME,
	SEARCH_COMPILER_COMMAND_ACTION_DELETE,
	SEARCH_COMPILER_COMMAND_ACTION_CHANGE_TYPE,
	SEARCH_COMPILER_COMMAND_ACTION_COPY,
	SEARCH_COMPILER_COMMAND_ACTION_STOP,
	SEARCH_COMPILER_COMMAND_ACTION_BACKUP,
	SEARCH_COMPILER_COMMAND_ACTION_ERROR = -1
} SearchCompilerCommandActions;

typedef enum _SearchCompilerErrors {
	SEARCH_COMPILER_ERROR_DEFAULT = -1,
	SEARCH_COMPILER_ERROR_JSON_MISSING_MESSAGE_NAME = -1001,
	SEARCH_COMPILER_ERROR_JSON_INVALID_MESSAGE_NAME = -1002,
	SEARCH_COMPILER_ERROR_JSON_COMPILE_FAILED = -1003,
	SEARCH_COMPILER_ERROR_JSON_COMPILE_INVALID_PARAMETER = -1004,
	SEARCH_COMPILER_ERROR_SQL_TOKENIZE_FAILED = -2001,
	SEARCH_COMPILER_ERROR_SQL_UNRECOGNIZED_COMMAND = -2002,
	SEARCH_COMPILER_ERROR_SQL_COMPILE_FAILED = -2003,
	SEARCH_COMPILER_ERROR_EXEC_COMPILE_INCOMPLETE = -3001,
	SEARCH_COMPILER_ERROR_EXEC_UNKNOWN_ACTION = -3002,
	SEARCH_COMPILER_ERROR_EXEC_FAILED = -3003,
	SEARCH_COMPILER_ERROR_EXEC_CONTAINER_DOES_NOT_EXIST = -3004,
	SEARCH_COMPILER_ERROR_EXEC_AUTHENTICATION_OFFLINE = -3005,
	SEARCH_COMPILER_ERROR_EXEC_PERMISSION_DENIED = -3006,
	SEARCH_COMPILER_ERROR_EXEC_SORT_OUT_OF_MEMORY = -3007,
	SEARCH_COMPILER_ERROR_UNKNOWN = -2048
} SearchCompilerErrors;

typedef enum _SearchCompilerSearchConditionType {
	SEARCH_COMPILER_SEARCH_CONDITION_TYPE_WILDCARD = 1,
	SEARCH_COMPILER_SEARCH_CONDITION_TYPE_EXACT,
	SEARCH_COMPILER_SEARCH_CONDITION_TYPE_USER,
	SEARCH_COMPILER_SEARCH_CONDITION_TYPE_RANGE,
	SEARCH_COMPILER_SEARCH_CONDITION_TYPE_GEO_COORD,
	SEARCH_COMPILER_SEARCH_CONDITION_TYPE_ERROR = -1
} SearchCompilerSearchConditionType;

typedef enum _SearchCompilerSearchSortType {
	SEARCH_COMPILER_SEARCH_SORT_TYPE_RELEVANCY = 1,
	SEARCH_COMPILER_SEARCH_SORT_TYPE_ATTRIBUTE,
	SEARCH_COMPILER_SEARCH_SORT_TYPE_GEO_COORD,
	SEARCH_COMPILER_SEARCH_SORT_TYPE_UNKNOWN,
	SEARCH_COMPILER_SEARCH_SORT_TYPE_ERROR = -1
} SearchCompilerSearchSortType;


// define search compiler public data types

typedef struct _SearchActionNewDomain {
	char *key;
	char *name;
} SearchActionNewDomain;

typedef struct _SearchActionNewIndex {
	int type;
	char *key;
	char *name;
} SearchActionNewIndex;

typedef struct _SearchActionGet {
	int uid;
	Container *container;
	SearchEngine *engine;
} SearchActionGet;

typedef struct _SearchActionGetByAttribute {
	char *attribute;
	char *value;
	Container *container;
	SearchEngine *engine;
} SearchActionGetByAttribute;

typedef struct _SearchActionPut {
	aboolean isImmediate;
	int uid;
	char *domainKey;
	Container *container;
} SearchActionPut;

typedef struct _SearchActionUpdate {
	aboolean isImmediate;
	char flags;
	int uid;
	char *domainKey;
	Container *container;
} SearchActionUpdate;

typedef struct _SearchActionUpdateByAttribute {
	aboolean isImmediate;
	char flags;
	int uid;
	char *attribute;
	char *domainKey;
	Container *container;
} SearchActionUpdateByAttribute;

typedef struct _SearchActionDelete {
	int uid;
} SearchActionDelete;

typedef struct _SearchActionDeleteByAttribute {
	int uid;
	char *attribute;
	char *value;
} SearchActionDeleteByAttribute;

typedef struct _SearchActionSearchSetting {
	aboolean isGlobalSearch;
	aboolean isEntireContainer;
	aboolean isAndIntersection;
} SearchActionSearchSetting;

typedef struct _SearchActionSearchCondition {
	SearchCompilerSearchConditionType type;
	RangeIndexSearchTypes rangeType;
	SearchEngineGeoCoordTypes geoCoordType;
	aboolean isNotEqualTo;
	int minValue;
	int maxValue;
	double distance;
	double latitude;
	double longitude;
	char *attribute;
	char *value;
} SearchActionSearchCondition;

typedef struct _SearchActionSearchConditionGroup {
	aboolean isAndIntersection;
	int conditionsLength;
	SearchActionSearchCondition *conditions;
	Intersect intersect;
} SearchActionSearchConditionGroup;

typedef struct _SearchActionSearchSortDirective {
	aboolean isStandardSortOrder;
	SearchCompilerSearchSortType type;
	double latitude;
	double longitude;
	char *attribute;
} SearchActionSearchSortDirective;

typedef struct _SearchActionSearchSort {
	aboolean isSorted;
	int directiveLength;
	SearchActionSearchSortDirective *directives;
} SearchActionSearchSort;

typedef struct _SearchActionSearchResultOption {
	aboolean hasResultOptions;
	aboolean isIncludeDistances;
	int offset;
	int limit;
	double latitude;
	double longitude;
} SearchActionSearchResultOption;

typedef struct _SearchActionSearchResultFacet {
	aboolean hasResultFacets;
	aboolean isDisplayIndexSummary;
	aboolean isDisplayFacets;
	int attributesLength;
	char **attributes;
} SearchActionSearchResultFacet;

typedef struct _SearchActionSearch {
	SearchActionSearchSetting settings;
	int requestAttributesLength;
	int domainKeysLength;
	int groupsLength;
	char **requestAttributes;
	char **domainKeys;
	SearchActionSearchConditionGroup *groups;
	SearchActionSearchSort sort;
	SearchActionSearchResultOption options;
	SearchActionSearchResultFacet facets;
	Intersect intersect;
	SearchEngine *engine;
} SearchActionSearch;

typedef struct _SearchActionList {
	aboolean isListDomains;
	aboolean isListIndexes;
	aboolean isListIndexDetails;
	int domainListLength;
	int indexListLength;
	int indexCount;
	int indexUniqueValueCount;
	int indexDepth;
	char *indexKey;
	char *indexType;
	char *domainKey;
	char **domainList;
	char **indexList;
	SearchEngine *engine;
} SearchActionList;

typedef struct _SearchActionCommand {
	SearchCompilerCommandTypes type;
	SearchCompilerCommandActions action;
	aboolean isOverrideIndexing;
	aboolean isIndexFullString;
	aboolean isIndexTokenizedString;
	int minStringLength;
	int maxStringLength;
	char *indexKey;
	char *indexName;
	char *indexType;
	char *newIndexKey;
	char *newIndexName;
	char *domainKey;
	char *newDomainKey;
	char *newDomainName;
	char *delimiters;
	char *excludedWordFilename;
} SearchActionCommand;

typedef struct _SearchActionLogin {
	char *username;
	char *password;
	char *token;
} SearchActionLogin;

typedef struct _SearchActionLogout {
	char *token;
} SearchActionLogout;

typedef struct _SearchCompiler {
	aboolean isCompileComplete;
	aboolean isExecutionComplete;
	SearchCompilerActions type;
	int errorCode;
	char *token;
	char *ipAddress;
	char *errorMessage;
	Log *log;
	void *action;
} SearchCompiler;


// delcare search compiler public functions

int searchCompiler_compileJson(SearchCompiler *compiler, Log *log,
		char *ipAddress, SearchJsonValidationType validationType,
		Json *message);

int searchCompiler_compileSql(SearchCompiler *compiler, Log *log, char *query);

void searchCompiler_free(SearchCompiler *compiler);

int searchCompiler_execute(SearchCompiler *compiler, SearchEngine *engine);

Json *searchCompiler_buildResponse(SearchCompiler *compiler);

// helper functions for compiler extensions

void searchCompiler_buildErrorMessage(SearchCompiler *compiler,
		const char *template, int errorCode, char *errorMessage);

char *searchCompiler_actionToString(SearchCompilerActions action);

char *searchCompiler_commandTypeToString(SearchCompilerCommandTypes type);

char *searchCompiler_commandActionToString(SearchCompilerCommandActions action);


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _SEARCH_SYSTEM_SEARCH_COMPILER_H

