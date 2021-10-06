/*
 * constants.js
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard Native API core constants library.
 *
 * Written by Josh English.
 */

// define native constants

/*
 * core/common constants
 */

var INT_ONE_THOUSAND = native_getConstant('INT_ONE_THOUSAND');

var REAL_ONE_THOUSAND = native_getConstant('REAL_ONE_THOUSAND');

var INT_ONE_MILLION = native_getConstant('INT_ONE_MILLION');

var REAL_ONE_MILLION = native_getConstant('REAL_ONE_MILLION');

var INT_ONE_BILLION = native_getConstant('INT_ONE_BILLION');

var REAL_ONE_BILLION = native_getConstant('REAL_ONE_BILLION');

var DIR_SEPARATOR = native_getConstant('DIR_SEPARATOR');


/*
 * core/os constants
 */

var SIZEOF_BOOLEAN = native_getConstant('SIZEOF_BOOLEAN');

var SIZEOF_ALINT = native_getConstant('SIZEOF_ALINT');

var SIZEOF_AULINT = native_getConstant('SIZEOF_AULINT');

var SIZEOF_CHAR = native_getConstant('SIZEOF_CHAR');

var SIZEOF_SHORT = native_getConstant('SIZEOF_SHORT');

var SIZEOF_INT = native_getConstant('SIZEOF_INT');

var SIZEOF_FLOAT = native_getConstant('SIZEOF_FLOAT');

var SIZEOF_DOUBLE = native_getConstant('SIZEOF_DOUBLE');


/*
 * core/system constants
 */

// signal

var SIGNAL_INTERRUPT = native_getConstant('SIGNAL_INTERRUPT');

var SIGNAL_TERMINATE = native_getConstant('SIGNAL_TERMINATE');

var SIGNAL_SEGFAULT = native_getConstant('SIGNAL_SEGFAULT');

var SIGNAL_USERONE = native_getConstant('SIGNAL_USERONE');

var SIGNAL_USERTWO = native_getConstant('SIGNAL_USERTWO');

var SIGNAL_QUIT = native_getConstant('SIGNAL_QUIT');

var SIGNAL_ABORT = native_getConstant('SIGNAL_ABORT');

var SIGNAL_FLOATING_POINT_EXCEPTION = native_getConstant(
		'SIGNAL_FLOATING_POINT_EXCEPTION');

var SIGNAL_ILLEGAL = native_getConstant('SIGNAL_ILLEGAL');

// socket

var SOCKET_MODE_CLIENT = native_getConstant('SOCKET_MODE_CLIENT');
var SOCKET_MODE_SERVER = native_getConstant('SOCKET_MODE_SERVER');

var SOCKET_PROTOCOL_TCPIP = native_getConstant('SOCKET_PROTOCOL_TCPIP');
var SOCKET_PROTOCOL_UDP = native_getConstant('SOCKET_PROTOCOL_UDP');

// thread

var THREAD_STATE_INIT = native_getConstant('THREAD_STATE_INIT');

var THREAD_STATE_RUNNING = native_getConstant('THREAD_STATE_RUNNING');

var THREAD_STATE_HALT = native_getConstant('THREAD_STATE_HALT');

var THREAD_STATE_STOPPED = native_getConstant('THREAD_STATE_STOPPED');

var THREAD_STATE_ERROR = native_getConstant('THREAD_STATE_ERROR');

