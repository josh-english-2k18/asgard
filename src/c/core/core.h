/*
 * core.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard Core Package header file.
 *
 * Written by Josh English.
 */

#if !defined(_CORE_H)

#define _CORE_H

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// common components

#include "core/common/common.h"
#include "core/common/macros.h"
#include "core/common/version.h"
#include "core/common/string.h"

// operating system components

#include "core/os/dl.h"
#include "core/os/time.h"
#include "core/os/types.h"

// memory components

#include "core/memory/memory.h"

// system components

#include "core/system/mutex.h"
#include "core/system/thread.h"
#include "core/system/system.h"
#include "core/system/stacktrace.h"
#include "core/system/signal.h"
#include "core/system/socket.h"
#include "core/system/spinlock.h"

// library components

#include "core/library/library.h"


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _CORE_H

