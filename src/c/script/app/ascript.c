/*
 * ascript.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The SpiderMonkey JavaScript engine for Asgard command-line application.
 *
 * Written by Josh English.
 */

// preprocessor directives

#define _CORE_COMMON_BYPASS_STD_STREAM
#include "core/core.h"
#include "script/script.h"


// main function

int main(int argc, char *argv[])
{
	int ref = 0;
	int iValue = 0;
	int length = 0;
	int result = 0;
	char *buffer = NULL;
	char *scriptResult = NULL;

	Log log;
	ScriptEngine scriptEngine;

	signal_registerDefault();

	log_init(&log, LOG_OUTPUT_NULL, NULL, LOG_LEVEL_PANIC);
//	log_init(&log, LOG_OUTPUT_STDOUT, NULL, LOG_LEVEL_DEBUG);

	scriptCommon_init(&scriptEngine, 0, &log);

	// set native types

	scriptTypes_initFromEngine(&scriptEngine);

	// set native context

	native_setScriptEngineContext(&scriptEngine);

	if(!native_registerConstants()) {
		fprintf(stderr, "error - failed to register native constants, "
				"aborting execution.\n");
		return 1;
	}

	if(!native_registerFunctions()) {
		fprintf(stderr, "error - failed to register native functions, "
				"aborting execution.\n");
		return 1;
	}

	// execute scripting engine

	if((argc >= 2) && (argv[1] != NULL)) {
		if(!script_includeScript(&scriptEngine,
					argv[1],
					&scriptResult)) {
			fprintf(stderr, "error - failed to access script '%s'.\n",
					argv[1]);
			result = 2;
		}
	}
	else {
		ref = 0;
		length = 128;
		buffer = (char *)malloc(sizeof(char) * (length + 1));

		while((iValue = getc(stdin)) != EOF) {
			buffer[ref] = (char)((unsigned char)iValue);
			ref++;
			if(ref >= length) {
				length *= 2;
				buffer = (char *)realloc(buffer, (sizeof(char) * (length + 1)));
			}
		}

		if(!script_includeScriptString(&scriptEngine,
					buffer,
					ref,
					&scriptResult) < 0) {
			fprintf(stderr, "error - failed to access script from stdin.\n");
			result = 2;
		}

		free(buffer);
	}

	if(scriptResult != NULL) {
		printf("%s", scriptResult);
	}

	// collect garbage

	if(!script_collectGarbage(&scriptEngine)) {
		fprintf(stderr, "[%s():%i] error - application aborted here.\n",
				__FUNCTION__, __LINE__);
		return 1;
	}

	// cleanup

	scriptCommon_free(&scriptEngine);

	return result;
}

