/*
 * stacktrace.js
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard Native API core library to deal with the JavaScript stack.
 *
 * Written by Josh English.
 */

// define stacktrace functions

function stacktrace_getStacktrace(error)
{
	var callstack = null;
	var currentFunction = '';
	var currentFunctionName = '';
	var currentFunctionString = '';
 
	try {
		if(error.stack) { // firefox
			callstack = error.stack.split('\n');
		}
		else {
			callstack = new Array();

			currentFunction = arguments.callee.caller;

			while(currentFunction) {
				currentFunctionString = currentFunction.toString();

				currentFunctionName = currentFunctionString.substring(
						currentFunctionString.indexOf(
							"function") + 8,
						currentFunctionString.indexOf(''));

				if(currentFunctionName = undefined) {
					currentFunctionName = 'unknown';
				}

				callstack.push(currentFunctionName);

				currentFunction = currentFunction.caller;
			}
		}
	}
	catch(localError) {
		print('error - stacktrace_getStacktrace() :: ' + localError + ' (' +
					localError.description + ')');
	}

	return callstack;
}

function stacktrace_printStacktrace(error)
{
	var ii = 0;
	var trace = null;

	try {
		trace = stacktrace_getStacktrace(error);

		if((trace != undefined) && (trace != null)) {
			for(ii = 0; ii < trace.length; ii++) {
				print('[stack ' + ii + '] ' + trace[ii]);
			}
		}
	}
	catch(localError) {
		print('error - stacktrace_printStacktrace() :: ' + localError + ' (' +
					localError.description + ')');
	}
}

