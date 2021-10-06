/*
 * esa_unitTest.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The English Stemming Algorithm (ESA) (see English, Joshua S. 2003)
 * reference implementation for reducing the morphological variation of any
 * given English Lanugage token through ESA-type conflation, unit test.
 *
 * Written by Josh English.
 */

// preprocessor directives

#define _CORE_COMMON_BYPASS_STD_STREAM
#include "core/core.h"


// define esa unit-test data

static const char *UNIT_TEST_TOKENS[] = {
	"This",
	"is",
	"a",
	"test",
	"of",
	"the",
	"English",
	"Stemming",
	"Algorithm",
	"(ESA)",
	"(English,",
	"Joshua",
	"S.",
	"2003).",
	"Here",
	"is",
	"a",
	"typical",
	"sentence:",
	"the",
	"quick",
	"brown",
	"fox",
	"jumped",
	"over",
	"the",
	"lazy",
	"dogs.",
	"Theses",
	"here",
	"words",
	"type",
	"thingamajiggers",
	"will",
	"hopefully",
	"be",
	"correctly",
	"stemmed",
	"by",
	"this",
	"heary",
	"algorithmic",
	"and",
	"stemmed",
	"out",
	"to",
	"their",
	"proper",
	"selves",
	NULL
};

static const char *UNIT_TEST_RESULTS[] = {
	"this",
	"is",
	"a",
	"test",
	"of",
	"the",
	"english",
	"stem",
	"algorithm",
	"esa))",
	"english,,",
	"joshua",
	"s.",
	".003).",
	"here",
	"is",
	"a",
	"typic",
	"sentence:",
	"the",
	"quick",
	"brown",
	"fox",
	"jump",
	"over",
	"the",
	"lazy",
	"dogs.",
	"thes",
	"here",
	"word",
	"type",
	"thingamajig",
	"will",
	"hopefully",
	"be",
	"correctly",
	"stem",
	"by",
	"this",
	"heary",
	"algorithm",
	"and",
	"stem",
	"out",
	"to",
	"their",
	"prop",
	"selv",
	NULL
};


// declare esa unit test functions

static int esa_basicFunctionalityTest();


// main function

int main()
{
	signal_registerDefault();

	printf("[unit] ESA unit test (Asgard Ver %s on %s)\n",
			ASGARD_VERSION, ASGARD_DATE);

	if(esa_basicFunctionalityTest() < 0) {
		printf("[unit]\t ...FAILED\n");
		return 1;
	}

	printf("[unit] ...PASSED\n");

	return 0;
}


// define esa unit test functions

static int esa_basicFunctionalityTest()
{
	int ii = 0;
	int tokenLength = 0;
	int tokenResultLength = 0;
	char *token = NULL;
	char *stem = NULL;

	Esa esa;

	printf("[unit]\t ESA basic functionality test...\n");

	esa_init(&esa);
	esa_setDebugMode(&esa, (unsigned char)0);

	for(ii = 0; UNIT_TEST_TOKENS[ii] != NULL; ii++) {
		token = (char *)UNIT_TEST_TOKENS[ii];
		tokenLength = strlen(token);

		if((stem = esa_stemToken(&esa, token, tokenLength,
						&tokenResultLength)) == NULL) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		if(strlen(stem) != tokenResultLength) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		printf("[unit]\t\t token '%s' => transform => '%s'\n", token, stem);

		if(strcmp(stem, (char *)UNIT_TEST_RESULTS[ii])) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		free(stem);
	}

	esa_free(&esa);

	printf("[unit]\t ...PASSED\n");

	return 0;
}

