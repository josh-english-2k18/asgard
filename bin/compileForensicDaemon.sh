#!/bin/bash

#
# compileForensicdaemon.sh
#
# Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
# intellectual property of Joshua S. English, all rights reserved.
#
# A simple script to compile the Asgard Forensic Daemon for use with Apache.
#
# Written by Josh English.
#

gcc -g -Wall -O3 -fPIC -DVERSION=0.5.13 -DVERSION_DATE=2011.04.06 -Isrc/c -Isrc/cpp -Iplugin/include -Iplugin/include/zlib src/c/forensic/forensic-daemon.c -o bin/forensic-daemon -lm -lcrypt -lpthread -rdynamic -ldl -Lplugin/lib -Llib -lasgardcore lib/libasgardcore.a plugin/lib/zlib/libz.a

