/*
 * search_sql.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The SQL language component for the search-engine query compiler.
 *
 * Written by Josh English.
 */

// preprocessor directives

#define _CORE_COMMON_BYPASS_STD_STREAM

#include "core/core.h"
#include "search/search.h"
#define _SEARCH_SYSTEM_LANG_COMPONENT
#include "search/system/lang/search_sql.h"


// delcare search compiler sql language private functions

static char *sqlHelper_parseValue(char *string, int length);


// define search compiler sql language private functions

static char *sqlHelper_parseValue(char *string, int length)
{
	int ii = 0;
	int nn = 0;
	char *result = NULL;

	result = (char *)malloc(sizeof(char) * (length + 1));

	for(ii = 0, nn = 0; ii < length; ii++) {
		if(string[ii] == '\'') {
			if((ii > 0) && (string[(ii - 1)] == '\\')) {
				if(nn > 0) {
					result[(nn - 1)] = '\'';
				}
				else {
					result[nn] = '\'';
					nn++;
				}
				continue;
			}
			else if(nn > 0) {
				break;
			}
			continue;
		}

		result[nn] = string[ii];
		nn++;
	}

	return result;
}


// define search compiler sql language public functions

aboolean searchSql_compileNewDomain(SearchCompiler *compiler, char **tokens,
		int *tokenLengths, int tokenListLength)
{
	int ii = 0;
	int mode = 0;
	char *domainKey = NULL;
	char *domainName = NULL;
	char buffer[1024];

	SearchActionNewDomain *action = NULL;

	typedef enum _SqlCompileNewDomainModes {
		SQL_COMPILE_NEW_DOMAIN_MODE_INIT = 1,
		SQL_COMPILE_NEW_DOMAIN_MODE_CREATE,
		SQL_COMPILE_NEW_DOMAIN_MODE_DOMAIN,
		SQL_COMPILE_NEW_DOMAIN_MODE_DOMAIN_NAME,
		SQL_COMPILE_NEW_DOMAIN_MODE_WITH,
		SQL_COMPILE_NEW_DOMAIN_MODE_KEY_KEYWORD,
		SQL_COMPILE_NEW_DOMAIN_MODE_EQUALS,
		SQL_COMPILE_NEW_DOMAIN_MODE_KEY,
		SQL_COMPILE_NEW_DOMAIN_MODE_ERROR = -1
	} SqlCompileNewDomains;

	const char *TEMPLATE = "failed to create new domain with '%s'";

	if((compiler == NULL) || (tokens == NULL) || (tokenLengths == NULL) ||
			(tokenListLength < 1)) {
		DISPLAY_INVALID_ARGS;
		if(compiler != NULL) {
			searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
					"invalid or missing argument(s)");
		}
		return afalse;
	}

	mode = SQL_COMPILE_NEW_DOMAIN_MODE_INIT;

	for(ii = 0; ii < tokenListLength; ii++) {
		switch(mode) {
			case SQL_COMPILE_NEW_DOMAIN_MODE_INIT:
				if(!strcasecmp(tokens[ii], "create")) {
					mode = SQL_COMPILE_NEW_DOMAIN_MODE_CREATE;
				}
				else {
					snprintf(buffer, (int)(sizeof(buffer) - 1),
							"unrecognized token '%s'", tokens[ii]);
					searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
							buffer);
					return afalse;
				}
				break;

			case SQL_COMPILE_NEW_DOMAIN_MODE_CREATE:
				if(!strcasecmp(tokens[ii], "domain")) {
					mode = SQL_COMPILE_NEW_DOMAIN_MODE_DOMAIN;
				}
				else {
					snprintf(buffer, (int)(sizeof(buffer) - 1),
							"unrecognized token '%s'", tokens[ii]);
					searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
							buffer);
					return afalse;
				}
				break;

			case SQL_COMPILE_NEW_DOMAIN_MODE_DOMAIN:
				domainName = tokens[ii];
				mode = SQL_COMPILE_NEW_DOMAIN_MODE_DOMAIN_NAME;
				break;

			case SQL_COMPILE_NEW_DOMAIN_MODE_DOMAIN_NAME:
				if(!strcasecmp(tokens[ii], "with")) {
					mode = SQL_COMPILE_NEW_DOMAIN_MODE_WITH;
				}
				else {
					snprintf(buffer, (int)(sizeof(buffer) - 1),
							"unrecognized token '%s'", tokens[ii]);
					searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
							buffer);
					return afalse;
				}
				break;

			case SQL_COMPILE_NEW_DOMAIN_MODE_WITH:
				if(!strcasecmp(tokens[ii], "key")) {
					mode = SQL_COMPILE_NEW_DOMAIN_MODE_KEY_KEYWORD;
				}
				else {
					snprintf(buffer, (int)(sizeof(buffer) - 1),
							"unrecognized token '%s'", tokens[ii]);
					searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
							buffer);
					return afalse;
				}
				break;

			case SQL_COMPILE_NEW_DOMAIN_MODE_KEY_KEYWORD:
				if(!strcasecmp(tokens[ii], "=")) {
					mode = SQL_COMPILE_NEW_DOMAIN_MODE_EQUALS;
				}
				else {
					snprintf(buffer, (int)(sizeof(buffer) - 1),
							"unrecognized token '%s'", tokens[ii]);
					searchCompiler_buildErrorMessage(compiler, TEMPLATE, -1,
							buffer);
					return afalse;
				}
				break;

			case SQL_COMPILE_NEW_DOMAIN_MODE_EQUALS:
				domainKey = sqlHelper_parseValue(tokens[ii], tokenLengths[ii]);
				mode = SQL_COMPILE_NEW_DOMAIN_MODE_KEY;
				break;

			case SQL_COMPILE_NEW_DOMAIN_MODE_KEY:
			case SQL_COMPILE_NEW_DOMAIN_MODE_ERROR:
			default:
				break;
		}

		if((mode == SQL_COMPILE_NEW_DOMAIN_MODE_KEY) ||
				(mode == SQL_COMPILE_NEW_DOMAIN_MODE_ERROR)) {
			break;
		}
	}

	action = (SearchActionNewDomain *)malloc(sizeof(SearchActionNewDomain));

	action->key = domainKey;
	action->name = strdup(domainName);

	compiler->type = SEARCH_COMPILER_ACTION_NEW_DOMAIN;
	compiler->action = (void *)action;

	return atrue;
}

