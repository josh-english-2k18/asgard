/*
 * fifostack_unitTest.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard core library FIFO stack type library, unit test.
 *
 * Written by Josh English.
 */

// preprocessor directives

#define _CORE_COMMON_BYPASS_STD_STREAM
#include "core/core.h"

// declare fifo stack unit test constants

#define FIFOSTACK_BASIC_TEST_LENGTH					8192

// declare fifo stack unit tests

static int fifostack_basicFunctionalityTest();

// main function

int main()
{
	signal_registerDefault();

	printf("[unit] fifo stack unit test (Asgard Ver %s on %s)\n",
			ASGARD_VERSION, ASGARD_DATE);

	if(fifostack_basicFunctionalityTest() < 0) {
		printf("[unit]\t ...FAILED\n");
		return 1;
	}

	printf("[unit] ...PASSED\n");

	return 0;
}

// define fifo stack unit tests

static int fifostack_basicFunctionalityTest()
{
	aboolean isEmpty = afalse;
	int ii = 0;
	alint itemCount = 0;
	char *item = NULL;
	char buffer[1024];

	FifoStack stack;

	printf("[unit]\t fifo stack basic functionality test...\n");

	if(fifostack_init(&stack) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(fifostack_isEmpty(&stack, &isEmpty) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(!isEmpty) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	for(ii = 0; ii < FIFOSTACK_BASIC_TEST_LENGTH; ii++) {
		snprintf(buffer, sizeof(buffer), "fifostack test item %08i", ii);

		item = strdup(buffer);

		if(fifostack_push(&stack, item) < 0) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		if(fifostack_isEmpty(&stack, &isEmpty) < 0) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		if(isEmpty) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		if(fifostack_getItemCount(&stack, &itemCount) < 0) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		if(itemCount != (alint)(ii + 1)) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}
	}

	for(ii = 0; ii < FIFOSTACK_BASIC_TEST_LENGTH; ii++) {
		if(fifostack_pop(&stack, ((void *)&item)) < 0) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		snprintf(buffer, sizeof(buffer), "fifostack test item %08i", ii);

		if(strcmp(buffer, item)) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		free(item);

		if(fifostack_getItemCount(&stack, &itemCount) < 0) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}

		if(itemCount != (alint)(FIFOSTACK_BASIC_TEST_LENGTH - (ii + 1))) {
			printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
					__LINE__);
			return -1;
		}
	}

	if(fifostack_isEmpty(&stack, &isEmpty) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(!isEmpty) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	if(fifostack_free(&stack) < 0) {
		printf("[unit]\t\t ...ERROR, failed at line %i, aborting.\n",
				__LINE__);
		return -1;
	}

	printf("[unit]\t ...PASSED\n");

	return 0;
}

