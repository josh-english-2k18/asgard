/*
 * smtest3.js
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * A JavaScript test to determine if the SpiderMonkey engine integrated into
 * Asgard is working properly.
 *
 * Written by Josh English.
 */

// indicate that this script was included

print('smtest3.js was included');

// define local variables

var ii = 0;
var nn = 0;
var width = 80;
var spaces = 0;
var direction = 0;
var string = '';

// simple logic to print out forward and backward slashes to the shell

//for(ii = 0; ii < 16384; ii++) {
//for(ii = 0; ii < 160; ii++) {
for(ii = 0; ii < 16; ii++) {
	string = '';

	for(nn = 0; nn < spaces; nn++) {
		string += ' ';
	}

	if(!direction) {
		string += '\\';
	}
	else {
		string += '/';
	}

	print(string);

	if(!direction) {
		spaces++;
		if(spaces >= width) {
			direction = 1;
		}
	}
	else {
		spaces--;
		if(spaces <= 0) {
			direction = 0;
		}
	}
}

