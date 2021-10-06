/*
 * common.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The master common header-file for the Asgard Game Engine.
 *
 * Written by Josh English.
 */

#if !defined(_ASGARD_CORE_COMMON_H)

#define _ASGARD_CORE_COMMON_H

#if !defined(_ASGARD_H) && !defined(_ASGARD_CORE_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _ASGARD_H || _ASGARD_CORE_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// setup standard includes

#if !defined(__ANDROID__)
#	include "glew.h"
#endif // !__ANDROID__

// setup operating-system-specific includes

#if defined(__linux__) && !defined(__ANDROID__)
#	include <GL/glx.h>
#	include <X11/extensions/xf86vmode.h>
#	include <X11/keysym.h>
#elif defined(WIN32)
#	include <zmouse.h>
#elif defined(__ANDROID__)
#	include <GLES/gl.h>
#	include <GLES/glext.h>
#endif // - operating-system-specific includes -


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _ASGARD_CORE_COMMON_H

