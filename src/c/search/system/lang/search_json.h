/*
 * search_json.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The JSON language component for the search-engine query compiler, header
 * file.
 *
 * Written by Josh English.
 */

#if !defined(_SEARCH_SYSTEM_LANG_SEARCH_JSON_H)

#define _SEARCH_SYSTEM_LANG_SEARCH_JSON_H

#if !defined(_SEARCH_H) && !defined(_SEARCH_SYSTEM_LANG_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _SEARCH_H || _SEARCH_SYSTEM_LANG_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// delcare search compiler json language public functions

char *searchJson_validationTypeToString(SearchJsonValidationType type);

SearchJsonValidationType searchJson_stringToValidationType(char *string);

aboolean searchJson_compileNewDomain(SearchCompiler *compiler,
		SearchJsonValidationType validationType, Json *message);

Json *searchJson_buildNewDomainResult(SearchCompiler *compiler);

aboolean searchJson_compileNewIndex(SearchCompiler *compiler,
		SearchJsonValidationType validationType, Json *message);

Json *searchJson_buildNewIndexResult(SearchCompiler *compiler);

aboolean searchJson_compileGet(SearchCompiler *compiler,
		SearchJsonValidationType validationType, Json *message);

Json *searchJson_buildGetResult(SearchCompiler *compiler);

aboolean searchJson_compileGetByAttribute(SearchCompiler *compiler,
		SearchJsonValidationType validationType, Json *message);

Json *searchJson_buildGetByAttributeResult(SearchCompiler *compiler);

aboolean searchJson_compilePut(SearchCompiler *compiler,
		SearchJsonValidationType validationType, Json *message);

Json *searchJson_buildPutResult(SearchCompiler *compiler);

aboolean searchJson_compileUpdate(SearchCompiler *compiler,
		SearchJsonValidationType validationType, Json *message);

Json *searchJson_buildUpdateResult(SearchCompiler *compiler);

aboolean searchJson_compileUpdateByAttribute(SearchCompiler *compiler,
		SearchJsonValidationType validationType, Json *message);

Json *searchJson_buildUpdateByAttributeResult(SearchCompiler *compiler);

aboolean searchJson_compileDelete(SearchCompiler *compiler,
		SearchJsonValidationType validationType, Json *message);

Json *searchJson_buildDeleteResult(SearchCompiler *compiler);

aboolean searchJson_compileDeleteByAttribute(SearchCompiler *compiler,
		SearchJsonValidationType validationType, Json *message);

Json *searchJson_buildDeleteByAttributeResult(SearchCompiler *compiler);

aboolean searchJson_compileSearch(SearchCompiler *compiler,
		SearchJsonValidationType validationType, Json *message);

Json *searchJson_buildSearchResult(SearchCompiler *compiler);

aboolean searchJson_compileList(SearchCompiler *compiler,
		SearchJsonValidationType validationType, Json *message);

Json *searchJson_buildListResult(SearchCompiler *compiler);

aboolean searchJson_compileCommand(SearchCompiler *compiler,
		SearchJsonValidationType validationType, Json *message);

Json *searchJson_buildCommandResult(SearchCompiler *compiler);

aboolean searchJson_compileLogin(SearchCompiler *compiler,
		SearchJsonValidationType validationType, Json *message);

Json *searchJson_buildLoginResult(SearchCompiler *compiler);

aboolean searchJson_compileLogout(SearchCompiler *compiler,
		SearchJsonValidationType validationType, Json *message);

Json *searchJson_buildLogoutResult(SearchCompiler *compiler);


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _SEARCH_SYSTEM_LANG_SEARCH_JSON_H

