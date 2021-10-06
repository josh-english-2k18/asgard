/*
 * asgard.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard Game Engine header file.
 *
 * Written by Josh English.
 */

#if !defined(_ASGARD_H)

#define _ASGARD_H

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// asgard framework components

#include "core/core.h"


// external components

#if !defined(__ANDROID__)
#	include "openal/al.h"
#	include "openal/alc.h"
#	include "vorbis/vorbisfile.h"
#endif // __ANDROID__


// common components

#include "asgard/core/common.h"
#include "asgard/core/events.h"


// core plugin components

#include "asgard/core/plugin/keys_android.h"
#include "asgard/core/plugin/keys_linux.h"
#include "asgard/core/plugin/keys_windows.h"
#include "asgard/core/plugin/opengl.h"
#include "asgard/core/plugin/linuxapi.h"
#include "asgard/core/plugin/win32api.h"


// core components

#include "asgard/core/keys.h"
#include "asgard/core/osapi.h"
#include "asgard/core/canvas.h"
#include "asgard/core/colors.h"


// math components

#include "asgard/math/common.h"
#include "asgard/math/vector3d.h"
#include "asgard/math/plane3d.h"
#include "asgard/math/aab3d.h"
#include "asgard/math/frustum3d.h"


// engine plugin components

#include "asgard/engine/plugin/font_android.h"
#include "asgard/engine/plugin/font_linux.h"
#include "asgard/engine/plugin/font_windows.h"
#include "asgard/engine/plugin/targa.h"
#include "asgard/engine/plugin/wfobj.h"

#if !defined(__ANDROID__)
#	include "asgard/engine/plugin/ogg_vorbis.h"
#	include "asgard/engine/plugin/openal.h"
#endif // __ANDROID__


// engine common components

#include "asgard/engine/common.h"


// engine 3D shapes components

#include "asgard/engine/shapes/sphere3d.h"
#include "asgard/engine/shapes/cone3d.h"


// engine components

#include "asgard/engine/font.h"
#include "asgard/engine/texture.h"
#include "asgard/engine/material.h"
#include "asgard/engine/draw2d.h"
#include "asgard/engine/draw3d.h"
#include "asgard/engine/aac.h"


// engine effects components

#include "asgard/engine/effects/lightgl.h"
#include "asgard/engine/effects/particle_engine.h"


// engine 2d components

#include "asgard/engine/2d/common.h"
#include "asgard/engine/2d/animation2d.h"
#include "asgard/engine/2d/camera2d.h"
#include "asgard/engine/2d/sprite2d.h"
#include "asgard/engine/2d/tile2d.h"
#include "asgard/engine/2d/physics2d.h"


// gui components

#include "asgard/gui/common.h"
#include "asgard/gui/button.h"
#include "asgard/gui/checkbox.h"
#include "asgard/gui/textbox.h"
/*
 *  Autoscroll GUI widget is not working, commented out for now
 *
#include "asgard/gui/autoscroll.h"
 */


// engine components

#include "asgard/engine/camera.h"
#include "asgard/engine/pipeline.h"
#include "asgard/engine/engine.h"


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _ASGARD_H

