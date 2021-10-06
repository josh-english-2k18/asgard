/*
 * esa.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The English Stemming Algorithm (ESA) (see English, Joshua S. 2003)
 * reference implementation for reducing the morphological variation of any
 * given English Lanugage token through ESA-type conflation.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#define _CORE_LIBRARY_TYPES_COMPONENT
#include "core/library/util/esa.h"
#include "core/library/util/esa_rules.h"


// declare esa private functions

static void reallocRuleList(Esa *esa);

static void initRuleList(Esa *esa);

static void freeRuleList(Esa *esa);

static void addNewRule(Esa *esa, char *rule);

static aboolean isVowel(char value);

static int popRuleValue(char *value, int ref, int length, char **result);

static aboolean backwardsMatch(char *token, int tokenLength,
		char *value, int valueLength);

static char *normalizeToken(char *token, int length);


// define esa private functions

static void reallocRuleList(Esa *esa)
{
	esa->rules.size = (esa->rules.size * 2);

	esa->rules.length = (int *)realloc(esa->rules.length,
			(sizeof(int) * esa->rules.size));

	esa->rules.value = (char **)realloc(esa->rules.value,
			(sizeof(char *) * esa->rules.size));
}

static void initRuleList(Esa *esa)
{
	esa->rules.ref = 0;

	esa->rules.size = 8;

	esa->rules.length = (int *)malloc(sizeof(int) * esa->rules.size);

	esa->rules.value = (char **)malloc(sizeof(char *) * esa->rules.size);
}

static void freeRuleList(Esa *esa)
{
	int ii = 0;

	if(esa->rules.length != NULL) {
		free(esa->rules.length);
	}

	if(esa->rules.value != NULL) {
		for(ii = 0; ii < esa->rules.ref; ii++) {
			if(esa->rules.value[ii] != NULL) {
				free(esa->rules.value[ii]);
			}
		}
		free(esa->rules.value);
	}
}

static void addNewRule(Esa *esa, char *rule)
{
	int ref = 0;
	int length = 0;

	if((esa == NULL) || (rule == NULL) ||
			((length = strlen(rule)) <= 0)) {
		return;
	}

	ref = esa->rules.ref;

	if(length > esa->longestRuleLength) {
		esa->longestRuleLength = length;
	}

	esa->rules.length[ref] = length;
	esa->rules.value[ref] = strdup(rule);

	esa->rules.ref += 1;
	if(esa->rules.ref >= esa->rules.size) {
		reallocRuleList(esa);
	}
}

static aboolean isVowel(char value)
{
	if((value == 'a') || (value == 'e') || (value == 'i') || (value == 'o') ||
			(value == 'u') || (value == 'y')) {
		return atrue;
	}
	return afalse;
}

static int popRuleValue(char *value, int ref, int length, char **result)
{
	int ii = 0;
	int resultRef = 0;

	for(ii = ref; ii < length; ii++) {
		if(value[ii] == ',') {
			break;
		}
		(*result)[resultRef] = value[ii];
		resultRef++;
	}

	(*result)[resultRef] = '\0';

	return (resultRef - 1);
}

static aboolean backwardsMatch(char *token, int tokenLength,
		char *value, int valueLength)
{
	int ii = 0;
	int nn = 0;

	if(valueLength < 0) {
		return afalse;
	}

	for(ii = tokenLength, nn = valueLength; ii > -1; ii--) {
		if(token[ii] != value[nn]) {
			return afalse;
		}
		nn--;
		if(nn < 0) {
			break;
		}
	}

	return atrue;
}

static char *normalizeToken(char *token, int length)
{
	aboolean hasFirstWhitespace = afalse;
	int ii = 0;
	int nn = 0;
	int lastWhitespace = -1;
	int resultLength = 0;
	unsigned int iValue = 0;
	char *result = NULL;

	result = (char *)malloc(sizeof(char) * (length + 8));

	memcpy(result, token, length);

	resultLength = strlen(result);

	for(ii = 0, nn = 0; ii < resultLength; ii++) {
		iValue = (unsigned int)result[ii];
		if((iValue < 32) && (iValue > 126)) {
			result[ii] = ' ';
		}
		if((!hasFirstWhitespace) && (result[ii] != ' ')) {
			hasFirstWhitespace = atrue;
		}
		if(hasFirstWhitespace) {
			iValue = (unsigned int)result[ii];
			if((iValue > 64) && (iValue < 91)) {
				result[nn] = (char)(iValue + 32);
			}
			else {
				result[nn] = result[ii];
			}
			if(result[nn] == ' ') {
				lastWhitespace = nn;
			}
			else {
				lastWhitespace = -1;
			}
			nn++;
		}
	}

	if(lastWhitespace > -1) {
		result[lastWhitespace] = '\0';
	}

	resultLength = strlen(result);
	for(ii = 0, nn = 0; ii < resultLength; ii++) {
		result[nn] = result[ii];
		iValue = (unsigned int)result[ii];
		if((iValue != 32) && // space
				((iValue < 97) || (iValue > 122))) {
			continue;
		}
		nn++;
	}

	return result;
}


// define esa public functions

void esa_init(Esa *esa)
{
	int ii = 0;

	if(esa == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	memset(esa, 0, sizeof(Esa));

	esa->isDebugMode = afalse;
	esa->longestRuleLength = 0;

	initRuleList(esa);

	for(ii = 0; ESA_RULE_LIST[ii] != NULL; ii++) {
		addNewRule(esa, (char *)ESA_RULE_LIST[ii]);
	}
}

void esa_free(Esa *esa)
{
	if(esa == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	freeRuleList(esa);

	memset(esa, 0, sizeof(Esa));
}

void esa_setDebugMode(Esa *esa, aboolean mode)
{
	if(esa == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	esa->isDebugMode = mode;
}

char *esa_stemToken(Esa *esa, char *token, int length, int *tokenResultLength)
{
	aboolean changed = afalse;
	aboolean hasVowel = afalse;
	aboolean hasConsonant = afalse;
	aboolean useLocal = afalse;
	int ii = 0;
	int nn = 0;
	int jj = 0;
	int ref = 0;
	int tempRef = 0;
	int subtract = 0;
	int resultRef = 0;
	int resultLength = 0;
	char *temp = NULL;
	char *result = NULL;

	if((esa == NULL) || (token == NULL) || (length <= 0) ||
			(tokenResultLength == NULL)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	*tokenResultLength = 0;

	if(esa->isDebugMode) {
		printf("[debug] stem_token(%s, %i)...\n", token, length);
	}

	result = normalizeToken(token, length);
	resultLength = strlen(result);
	resultRef = (resultLength - 1);

	if(resultRef < 4) {
		*tokenResultLength = resultLength;
		return result;
	}

	for(ii = 0; ii <= resultRef; ii++) {
		if(isVowel(result[ii])) {
			hasVowel = atrue;
		}
		else {
			hasConsonant = atrue;
		}
		if((hasVowel) && (hasConsonant)) {
			break;
		}
	}

	if((!hasVowel) || (!hasConsonant)) {
		*tokenResultLength = resultLength;
		return result;
	}

	temp = (char *)malloc(sizeof(char) * (esa->longestRuleLength + 8));

	for(ii = 0; ii < esa->rules.ref; ii++) {
		useLocal = afalse;
		ref = 0;
		tempRef = popRuleValue(esa->rules.value[ii], ref,
				esa->rules.length[ii], &temp);

		if(esa->isDebugMode) {
			printf("[debug]\t trying[%s](%i) vs [%s](%i)...\n", result,
					resultRef, temp, tempRef);
		}

		while(backwardsMatch(result, resultRef, temp, tempRef)) {
			if(esa->isDebugMode) {
				printf("[debug]\t matched token[%s] to rule(%i)[%s]{%s}\n",
						result, ii, esa->rules.value[ii], temp);
			}

			ref += (tempRef + 2);
			tempRef = popRuleValue(esa->rules.value[ii], ref,
					esa->rules.length[ii], &temp);
			if(useLocal) {
				if((temp[0] == 'Y') || (temp[0] == 'y')) {
					if(esa->isDebugMode) {
						printf("[debug]\t ...unable to apply rule becuase "
								"token already modified.\n");
					}
					break;
				}
			}
			else {
				if(((temp[0] == 'Y') || (temp[0] == 'y')) &&
						(changed == atrue)) {
					if(esa->isDebugMode) {
						printf("[debug]\t ...unable to apply rule becuase "
								"token already modified.\n");
					}
					break;
				}
			}

			ref += (tempRef + 2);
			tempRef = popRuleValue(esa->rules.value[ii], ref,
					esa->rules.length[ii], &temp);
			subtract = atoi(temp);
			if((resultRef - subtract) < 2) {
				if(esa->isDebugMode) {
					printf("[debug]\t ...unable to apply rule becuase "
							"token would be too truncated(%i).\n",
							(resultRef - subtract));
				}
				break;
			}
			resultRef -= subtract;
			result[(resultRef + 1)] = '\0';
			resultLength = (resultRef + 1);

			changed = atrue;

			ref += (tempRef + 2);
			tempRef = popRuleValue(esa->rules.value[ii], ref,
					esa->rules.length[ii], &temp);
			if(tempRef > -1) {
				nn = 0;
				jj = 0;
				for(nn = (resultRef + 1);
						nn <= (resultRef + tempRef + 1);
						nn++) {
					result[nn] = temp[jj];
					jj++;
				}
				result[nn] = '\0';
				resultLength = nn;
				resultRef += tempRef + 1;
			}
			else if(esa->isDebugMode) {
				printf("[debug]\t ...no addition to make, value is "
						"null.\n");
			}

			ref += (tempRef + 2);
			tempRef = popRuleValue(esa->rules.value[ii], ref,
					esa->rules.length[ii], &temp);

			useLocal = atrue;
		}
	}

	free(temp);

	*tokenResultLength = resultLength;

	return result;
}

