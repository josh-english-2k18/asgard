/*
 * smtest2.js
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

print('smtest2.js was included');

// define local functions

function smtestTwoFunction()
{
	print('smtestTwoFunction() :: the include stuff works');
}

// include a 3rd file in the chain

print('including "smtest3.js"...');

include('src/js/test/smtest3.js');

// verify the asgard native api

include('src/js/core/core.js');

function executeError()
{
	some.missing.variable += 123;
}

try {
	executeError();
}
catch(error) {
	stacktrace_printStacktrace(error);
}

