/*
 * string-comparison.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * A simple string-comparison algorithm test application.
 *
 * Written by Josh English.
 */

// preprocessor directives

#define _CORE_COMMON_BYPASS_STD_STREAM
#include "core/core.h"

static char *WORD_LIST[] = {
	"nam",
	"at",
	"nulla",
	"sed",
	"neque",
	"pulvinar",
	"pellentesque",
	"quis",
	"vitae",
	"turpis",
	"sed",
	"interdum",
	"rhoncus",
	"quam",
	"eu",
	"ultrices",
	"duis",
	"non",
	"accumsan",
	"enim",
	"duis",
	"malesuada",
	"justo",
	"ac",
	"laoreet",
	"porttitor",
	"eros",
	"mi",
	"luctus",
	"erat",
	"bibendum",
	"pulvinar",
	"lectus",
	"nibh",
	"non",
	"sapien",
	"donec",
	"nulla",
	"dui",
	"sagittis",
	"in",
	"tristique",
	"vitae",
	"ultrices",
	"quis",
	"nunc",
	"nulla",
	"pellentesque",
	"lorem",
	"et",
	"lectus",
	"malesuada",
	"nec",
	"facilisis",
	"urna",
	"placerat",
	"praesent",
	"a",
	"neque",
	"enim",
	"non",
	"porta",
	"odio",
	"nunc",
	"ante",
	"augue",
	"porta",
	"nec",
	"fringilla",
	"sit",
	"amet",
	"pellentesque",
	"eu",
	"odio",
	"nulla",
	"sit",
	"amet",
	"neque",
	"ipsum",
	"in",
	"magna",
	"est",
	"iaculis",
	"id",
	"tristique",
	"at",
	"tempor",
	"vitae",
	"tellus",
	"vivamus",
	"lacinia",
	"orci",
	"nec",
	"rutrum",
	"porttitor",
	"quam",
	"nulla",
	"pulvinar",
	"orci",
	"eget",
	"tincidunt",
	"ipsum",
	"mauris",
	"quis",
	"velit",
	"maecenas",
	"lobortis",
	"augue",
	"in",
	"metus",
	"adipiscing",
	"ultrices",
	"nulla",
	"elementum",
	"risus",
	"ac",
	"enim",
	"scelerisque",
	"tristique",
	"aliquam",
	"consequat",
	"suscipit",
	"vehicula",
	"maecenas",
	"a",
	"enim",
	"ante",
	"curabitur",
	NULL
};


// declare local functions

static void executeTests();


// main function

int main(int argc, char *argv[])
{
	signal_registerDefault();

	executeTests();

	return 0;
}


// define local functions

static void executeTests()
{
	int ii = 0;
	int nn = 0;
	int pick = 0;
	int length = 0;
	int counter = 0;
	int alphaLength = 0;
	int betaLength = 0;
	int stringLength = 0;
	double timer = 0.0;
	double result = 0.0;
	double totalTime = 0.0;
	char *alpha = NULL;
	char *beta = NULL;
	char **alphaArray = NULL;
	char **betaArray = NULL;

	alpha = "this is a test";
	beta = "this is a test";

	timer = time_getTimeMus();

	printf("%0.2f = compareStrings() '%s' vs '%s'\n",
			compareStrings(alpha, (alphaLength = strlen(alpha)),
				beta, (betaLength = strlen(beta))),
			alpha, beta);

	printf("...completed in %0.6f seconds\n",
			time_getElapsedMusInSeconds(timer));

	alpha = "this is a test";
	beta = "tihs si a tset";

	timer = time_getTimeMus();

	printf("%0.2f = compareStrings() '%s' vs '%s'\n",
			compareStrings(alpha, (alphaLength = strlen(alpha)),
				beta, (betaLength = strlen(beta))),
			alpha, beta);

	printf("...completed in %0.6f seconds\n",
			time_getElapsedMusInSeconds(timer));

	system_pickRandomSeed();

	counter = 0;
	totalTime = 0.0;

	for(ii = 0; WORD_LIST[ii] != NULL; ii++) {
		alpha = strdup(WORD_LIST[ii]);
		alphaLength = strlen(alpha);
		beta = strdup(WORD_LIST[ii]);
		betaLength = strlen(beta);

		if((rand() % 128) > 64) {
			length = ((rand() % 8) + 1);
			beta = (char *)realloc(beta,
					(sizeof(char) * (betaLength + length + 1)));

			for(nn = (betaLength - 1); nn < (betaLength + length); nn++) {
				if((rand() % 128) > 64) {
					beta[nn] = ' ';
				}
				else {
					pick = ((rand() % 26) + 97);
					beta[nn] = (char)pick;
				}
			}

			beta[nn] = '\0';

			betaLength += length;
		}

		if((rand() % 128) > 64) {
			for(nn = 0; nn < betaLength; nn++) {
				if((rand() % 128) > 64) {
					if((rand() % 128) > 64) {
						beta[nn] = ' ';
					}
					else {
						pick = ((rand() % 26) + 97);
						beta[nn] = (char)pick;
					}
				}
			}
		}

		timer = time_getTimeMus();

/*
		printf("%0.2f = compareStrings() '%s' vs '%s'\n",
				compareStrings(alpha, (alphaLength = strlen(alpha)),
					beta, (betaLength = strlen(beta))),
				alpha, beta);
*/

		result = compareStrings(alpha, (alphaLength = strlen(alpha)),
				beta, (betaLength = strlen(beta)));

		totalTime += time_getElapsedMusInSeconds(timer);
		counter++;

		free(alpha);
		free(beta);
	}

	printf("...executed %i comparisons in %0.6f (%0.6f avg) seconds\n",
			counter, totalTime, (totalTime / (double)counter));

	alphaLength = 0;

	for(ii = 0; WORD_LIST[ii] != NULL; ii++) {
		alphaLength++;
	}

	betaLength = alphaLength;

	alphaArray = (char **)malloc(sizeof(char *) * alphaLength);
	betaArray = (char **)malloc(sizeof(char *) * betaLength);

	for(ii = 0; ii < alphaLength; ii++) {
		alphaArray[ii] = strdup(WORD_LIST[ii]);
		betaArray[ii] = strdup(WORD_LIST[ii]);
	}

	timer = time_getTimeMus();

	printf("%0.2f = compareArrays()\n",
			compareArrays(alphaArray, alphaLength, betaArray, betaLength));

	printf("...completed in %0.6f seconds\n",
			time_getElapsedMusInSeconds(timer));

	alphaArray = (char **)malloc(sizeof(char *) * alphaLength);

	for(ii = 0; ii < alphaLength; ii++) {
		alphaArray[ii] = strdup(WORD_LIST[ii]);
		stringLength = strlen(alphaArray[ii]);

		if((rand() % 128) > 64) {
			length = ((rand() % 8) + 1);
			alphaArray[ii] = (char *)realloc(alphaArray[ii],
					(sizeof(char) * (stringLength + length + 1)));

			for(nn = (stringLength - 1); nn < (stringLength + length); nn++) {
				if((rand() % 128) > 64) {
					alphaArray[ii][nn] = ' ';
				}
				else {
					pick = ((rand() % 26) + 97);
					alphaArray[ii][nn] = (char)pick;
				}
			}

			alphaArray[ii][nn] = '\0';

			stringLength += length;
		}

		if((rand() % 128) > 64) {
			for(nn = 0; nn < stringLength; nn++) {
				if((rand() % 128) > 64) {
					if((rand() % 128) > 64) {
						alphaArray[ii][nn] = ' ';
					}
					else {
						pick = ((rand() % 26) + 97);
						alphaArray[ii][nn] = (char)pick;
					}
				}
			}
		}
	}

	betaLength = alphaLength;
	betaArray = (char **)malloc(sizeof(char *) * betaLength);

	for(ii = 0; ii < betaLength; ii++) {
		betaArray[ii] = strdup(alphaArray[ii]);
		stringLength = strlen(betaArray[ii]);

		if((rand() % 128) > 64) {
			length = ((rand() % 8) + 1);
			betaArray[ii] = (char *)realloc(betaArray[ii],
					(sizeof(char) * (stringLength + length + 1)));

			for(nn = (stringLength - 1); nn < (stringLength + length); nn++) {
				if((rand() % 128) > 64) {
					betaArray[ii][nn] = ' ';
				}
				else {
					pick = ((rand() % 26) + 97);
					betaArray[ii][nn] = (char)pick;
				}
			}

			betaArray[ii][nn] = '\0';

			stringLength += length;
		}

		if((rand() % 128) > 64) {
			for(nn = 0; nn < stringLength; nn++) {
				if((rand() % 128) > 64) {
					if((rand() % 128) > 64) {
						betaArray[ii][nn] = ' ';
					}
					else {
						pick = ((rand() % 26) + 97);
						betaArray[ii][nn] = (char)pick;
					}
				}
			}
		}
	}

	timer = time_getTimeMus();

	printf("%0.2f = compareArrays()\n",
			compareArrays(alphaArray, alphaLength, betaArray, betaLength));

	printf("...completed in %0.6f seconds\n",
			time_getElapsedMusInSeconds(timer));
}

