/*
 * library.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard Core-Library Package header file.
 *
 * Written by Josh English.
 */

#if !defined(_CORE_LIBRARY_H)

#define _CORE_LIBRARY_H

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// library core components

#include "core/library/core/ctype.h"
#include "core/library/core/error.h"
#include "core/library/core/endian.h"
#include "core/library/core/serialize.h"

// library system components

#include "core/library/system/file.h"
#include "core/library/system/dir.h"
#include "core/library/system/tmf.h"

// library types components

#include "core/library/types/arraylist.h"
#include "core/library/types/bptree.h"
#include "core/library/types/fifostack.h"
#include "core/library/types/integer.h"
#include "core/library/types/intersect.h"
#include "core/library/types/json.h"
#include "core/library/types/lfbc.h"
#include "core/library/types/managed_index.h"
#include "core/library/types/range_index.h"
#include "core/library/types/wildcard_index.h"
#include "core/library/types/xpath.h"

// library utilty components

#include "core/library/util/config.h"
#include "core/library/util/crc32.h"
#include "core/library/util/esa.h"
#include "core/library/util/log.h"
#include "core/library/util/pfs.h"
#include "core/library/util/sha1.h"
#include "core/library/util/zlib.h"

// library networking components

// protocols

#include "core/library/network/protocols/http.h"

// components

#include "core/library/network/transaction_manager.h"
#include "core/library/network/server.h"


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _CORE_LIBRARY_H

