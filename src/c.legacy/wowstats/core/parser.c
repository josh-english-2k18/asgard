/*
 * parser.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard-WoW statistical analysis file-parsing library.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#define _WOWSTATS_CORE_COMPONENT
#include "wowstats/core/parser.h"

// delcare wowstats parser private functions

static int parseStatLine(WowParser *parser, int tokenRef, char *line,
		int lineRef, int lineLength, int *parseLength);


// define wowstats parser private functions

static int parseStatLine(WowParser *parser, int tokenRef, char *line,
		int lineRef, int lineLength, int *parseLength)
{
	aboolean isVisible = atrue;
	int ii = 0;
	int nn = 0;
	int cId = 0;

	*parseLength = 0;

	for(ii = lineRef, nn = 0; ((ii < lineLength) && (nn < lineLength)); ii++) {
		*parseLength += 1;

		cId = (int)line[ii];
		if(cId < 0) {
			cId += 256;
		}
		if((cId < 32) || (cId > 126)) {
//			if((cId < 192) || (cId > 254)) {
				continue;
//			}
		}

		if(line[ii] == '"') {
			if(isVisible) {
				isVisible = afalse;
			}
			else {
				isVisible = atrue;
			}
			continue;
		}

		if(isVisible) {
			if((line[ii] == ' ') || (line[ii] == ',')) {
				if((line[ii] == ' ') && (nn == 0)) {
					continue;
				}
				break;
			}
		}

		parser->tokens[tokenRef][nn] = line[ii];
		nn++;

		if((nn + 1) >= WOWSTATS_PARSER_TOKEN_LENGTH) {
			if(((ii + 1) < lineLength) &&
					((line[(ii + 1)] == ' ') || (line[(ii + 1)] == ','))) {
				break;
			}
			return -1;
		}
	}

	parser->tokens[tokenRef][nn] = '\0';
	parser->tokenLengths[tokenRef] = nn;

	return 0;
}


// define wowstats parser public functions

int wowstats_parserInit(WowParser *parser)
{
	int ii = 0;

	if(parser == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	parser->tokenCount = 0;

	for(ii = 0; ii < WOWSTATS_PARSER_TOKENS; ii++) {
		parser->tokenLengths[ii] = 0;
	}

	return 0;
}

int wowstats_parserParse(WowParser *parser, char *line, int lineLength)
{
	int rc = 0;
	int lineRef = 0;
	int tokenRef = 0;
	int parseLength = 0;

	if((parser == NULL) || (line == NULL) || (lineLength < 1)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	while((rc = parseStatLine(parser, tokenRef, line, lineRef,
					lineLength, &parseLength)) == 0) {
		if(parseLength == 0) {
			break;
		}

		tokenRef++;
		if((tokenRef + 1) >= WOWSTATS_PARSER_TOKENS) {
			break;
		}

		lineRef += parseLength;
		if(lineRef >= lineLength) {
			break;
		}
	}

	if(rc < 0) {
		return -1;
	}

	parser->tokenCount = tokenRef;

	return 0;
}

