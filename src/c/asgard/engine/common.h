/*
 * common.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The common header-file for the Asgard Game Engine engine library.
 *
 * Written by Josh English.
 */

#if !defined(_ASGARD_ENGINE_COMMON_H)

#define _ASGARD_ENGINE_COMMON_H

#if !defined(_ASGARD_H) && !defined(_ASGARD_ENGINE_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _ASGARD_H || _ASGARD_ENGINE_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// define common engine constants

typedef enum _AsgardEngineRenderModes {
	ASGARD_ENGINE_RENDER_MODE_SOLID = 1,
	ASGARD_ENGINE_RENDER_MODE_SOLID_NORMALIZED,
	ASGARD_ENGINE_RENDER_MODE_TEXTURED,
	ASGARD_ENGINE_RENDER_MODE_TEXTURED_NORMALIZED,
	ASGARD_ENGINE_RENDER_MODE_WIREFRAME,
	ASGARD_ENGINE_RENDER_MODE_WIREFRAME_WITH_NORMALS,
	ASGARD_ENGINE_RENDER_MODE_POINTS,
	ASGARD_ENGINE_RENDER_MODE_NONE = -1
} AsgardEngineRenderModes;


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _ASGARD_ENGINE_COMMON_H

