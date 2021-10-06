/*
 * parser.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard-WoW statistical analysis file-parsing library, header file.
 *
 * Written by Josh English.
 */

#if !defined(_WOWSTATS_CORE_PARSER_H)

#define _WOWSTATS_CORE_PARSER_H

#if !defined(_WOWSTATS_CORE_H) && !defined(_WOWSTATS_CORE_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _WOWSTATS_CORE_H || _WOWSTATS_CORE_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus

// define wowstats parser public constants

#define WOWSTATS_PARSER_TOKENS						32
#define WOWSTATS_PARSER_TOKEN_LENGTH				128

// define wowstats parser public data types

typedef struct _WowParser {
	int tokenCount;
	int tokenLengths[WOWSTATS_PARSER_TOKENS];
	char tokens[WOWSTATS_PARSER_TOKENS][WOWSTATS_PARSER_TOKEN_LENGTH];
} WowParser;

// declare wowstats parser public functions

int wowstats_parserInit(WowParser *parser);

int wowstats_parserParse(WowParser *parser, char *line, int lineLength);

#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _WOWSTATS_CORE_PARSER_H

