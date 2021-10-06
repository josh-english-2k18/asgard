/*
 * hanoi.js
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

// define global variables

var DISKS = 18;

var CHECK_ERRORS = false;
//var CHECK_ERRORS = true;

var DISPLAY_TOWERS = false;
//var DISPLAY_TOWERS = true;

var Towers = null;


// define local functions

function getId(value)
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

function displayTowers()
{
	var ii = 0;
	var nn = 0;

	if(!DISPLAY_TOWERS) {
		return;
	}

	for(ii = 0; ii < 3; ii++) {
		for(nn = 0; nn < DISKS; nn++) {
			print('[unit] Tower[' + ii + '][' + nn + '] :: ' + Towers[ii][nn]);
		}
	}
}

function moveDisk(diskId, source, dest)
{
	var ii = 0;
	var id = 0;
	var sourceId = 0;
	var destId = 0;

/*	print('[unit] moveDisk' + '() :: #' + diskId + ' from ' + source +
			' to ' + dest);*/

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
			print('error - failed to locate disk #' + diskId + ' in tower # ' +
					sourceId);
			displayTowers();
			exit(1);
		}

		if((id < (DISKS - 1)) && (Towers[sourceId][(id + 1)] != -1)) {
			print('error - detected invalid state in src-tower #' + sourceId);
			displayTowers();
			exit(1);
		}
	}

	Towers[sourceId][id] = -1;

	for(ii = 0; ii < DISKS; ii++) {
		if(Towers[destId][ii] == -1) {
			if((CHECK_ERRORS) && (ii > 0) &&
					(Towers[destId][(ii - 1)] < diskId)) {
				print('error - detected invalid state in dest-tower #' +
						destId);
				displayTowers();
				exit(1);
			}
			Towers[destId][ii] = diskId;
			break;
		}
	}
}

function hanoi(diskId, source, dest, spare)
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


// execute hanoi test

var ii = 0;
var nn = 0;
var timer = 0.0;

Towers = new Array();

for(ii = 0; ii < 3; ii++) {
	Towers[ii] = new Array();
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

print('[unit] hanoi JavaScript test executed in ' +
		time_getElapsedMusInSeconds(timer) + ' seconds');

displayTowers();

