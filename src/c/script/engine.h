/*
 * engine.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The SpiderMonkey JavaScript engine for Asgard, scripting engine API, header
 * file.
 *
 * Written by Josh English.
 */

#if !defined(_SCRIPT_ENGINE_H)

#define _SCRIPT_ENGINE_H

#if !defined(_SCRIPT_H) && !defined(_SCRIPT_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _SCRIPT_H || _SCRIPT_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// declare engine public functions

char *script_getVersion(ScriptEngine *scriptEngine);

aboolean script_defineFunctions(ScriptEngine *scriptEngine,
		JSFunctionSpec *functionList);

aboolean script_execFunction(ScriptEngine *scriptEngine, char *functionName,
		uintN argc, jsval *argv, char **functionResult);

aboolean script_includeScriptString(ScriptEngine *scriptEngine, char *script,
		int scriptLength, char **scriptResult);

aboolean script_includeScript(ScriptEngine *scriptEngine, char *filename,
		char **scriptResult);

aboolean script_collectGarbage(ScriptEngine *scriptEngine);

aboolean script_suggestCollectGarbage(ScriptEngine *scriptEngine);

aboolean script_dumpHeap(ScriptEngine *scriptEngine, void *stream);


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _SCRIPT_ENGINE_H

