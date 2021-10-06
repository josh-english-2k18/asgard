/*
 * hanoi_unitTest.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * A simple implementation of the Towers of Hanoi math problem for performance
 * testing comparisons with the SpiderMonkey JavaScript engine for Asgard, unit
 * test application.
 *
 * Written by Josh English.
 */

// preprocessor directives

#define _CORE_COMMON_BYPASS_STD_STREAM
#include "core/core.h"
#include "script/script.h"


// define unit test constants

#define DISKS									18

#define CHECK_ERRORS							afalse

#define DISPLAY_TOWERS							afalse


// define unit test global variables

static int Towers[3][DISKS];


// declare unit test functions

static int getId(char value);

static void displayTowers();

static void moveDisk(int diskId, char source, char dest);

static void hanoi(int diskId, char source, char dest, char spare);

static int executeHanoiCTest();

static int executeHanoiJavaScriptTest();


// main function

int main()
{
	signal_registerDefault();

	printf("Towers of Hanoi Performance Test (Asgard Ver %s on %s)\n",
			ASGARD_VERSION, ASGARD_DATE);

	if(executeHanoiCTest() < 0) {
		printf("[unit] ...FAILED\n");
		return 1;
	}

	if(executeHanoiJavaScriptTest() < 0) {
		printf("[unit] ...FAILED\n");
		return 1;
	}

	printf("[unit] ...PASSED\n");

	return 0;
}


// define unit test functions

static int getId(char value)
{
	switch(value) {
		case 'A':
			return 0;
		case 'B':
			return 1;
		case 'C':
			return 2;
		default:
			exit(1);
	}
}

static void displayTowers()
{
	int ii = 0;
	int nn = 0;

	if(!DISPLAY_TOWERS) {
		return;
	}

	for(ii = 0; ii < 3; ii++) {
		for(nn = 0; nn < DISKS; nn++) {
			printf("[unit]\t tower[%02i][%02i] :: %i\n",
					ii, nn, Towers[ii][nn]);
		}
	}
}

static void moveDisk(int diskId, char source, char dest)
{
	int ii = 0;
	int id = 0;
	int sourceId = 0;
	int destId = 0;

/*	printf("[unit]\t %s() :: #%i from %c to %c\n",
			__FUNCTION__, diskId, source, dest);*/

	sourceId = getId(source);
	destId = getId(dest);

	id = -1;

	for(ii = 0; ii < DISKS; ii++) {
		if(Towers[sourceId][ii] == diskId) {
			id = ii;
			break;
		}
	}

	if(CHECK_ERRORS) {
		if(id == -1) {
			fprintf(stderr,
					"error - failed to locate disk #%i in tower #%i.\n",
					diskId, sourceId);
			displayTowers();
			exit(1);
		}

		if((id < (DISKS - 1)) && (Towers[sourceId][(id + 1)] != -1)) {
			fprintf(stderr,
					"error - detected invalid state in src-tower #%i.\n",
					sourceId);
			displayTowers();
			exit(1);
		}
	}

	Towers[sourceId][id] = -1;

	for(ii = 0; ii < DISKS; ii++) {
		if(Towers[destId][ii] == -1) {
			if((CHECK_ERRORS) && (ii > 0) &&
					(Towers[destId][(ii - 1)] < diskId)) {
				fprintf(stderr,
						"error - detected invalid state in dest-tower #%i.\n",
						destId);
				displayTowers();
				exit(1);
			}
			Towers[destId][ii] = diskId;
			break;
		}
	}
}

static void hanoi(int diskId, char source, char dest, char spare)
{
	if(diskId == 0) {
		moveDisk(diskId, source, dest);
	}
	else {
		hanoi((diskId - 1), source, spare, dest);
		moveDisk(diskId, source, dest);
		hanoi((diskId - 1), spare, dest, source);
	}
}

static int executeHanoiCTest()
{
	int ii = 0;
	int nn = 0;
	double timer = 0.0;

	for(ii = 0; ii < 3; ii++) {
		for(nn = 0; nn < DISKS; nn++) {
			if(ii == 0) {
				Towers[ii][nn] = ((DISKS - 1) - nn);
			}
			else {
				Towers[ii][nn] = -1;
			}
		}
	}

	displayTowers();

	timer = time_getTimeMus();

	hanoi((DISKS - 1), 'A', 'C', 'B');

	printf("[unit]\t hanoi C test executed in %0.6f seconds\n",
			time_getElapsedMusInSeconds(timer));

	displayTowers();

	return 0;
}

static int executeHanoiJavaScriptTest()
{
	char *resultValue = NULL;

	ScriptEngine scriptEngine;

	scriptCommon_init(&scriptEngine, 0, NULL);

	// set native types

	scriptTypes_initFromEngine(&scriptEngine);

	// set native context

	native_setScriptEngineContext(&scriptEngine);

	if(!native_registerFunctions()) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	// execute the towers-of-hanoi performance test

	if(!script_includeScript(&scriptEngine, "src/js/test/hanoi.js",
				&resultValue)) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	printf("[unit] script result: %s\n", resultValue);

	// cleanup

	scriptCommon_free(&scriptEngine);

	return 0;
}

