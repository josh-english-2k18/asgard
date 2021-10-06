/*
 * search_sql.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The SQL language component for the search-engine query compiler, header
 * file.
 *
 * Written by Josh English.
 */

#if !defined(_SEARCH_SYSTEM_LANG_SEARCH_SQL_H)

#define _SEARCH_SYSTEM_LANG_SEARCH_SQL_H

#if !defined(_SEARCH_H) && !defined(_SEARCH_SYSTEM_LANG_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _SEARCH_H || _SEARCH_SYSTEM_LANG_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// delcare search compiler sql language public functions

aboolean searchSql_compileNewDomain(SearchCompiler *compiler, char **tokens,
		int *tokenLengths, int tokenListLength);


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _SEARCH_SYSTEM_LANG_SEARCH_SQL_H

