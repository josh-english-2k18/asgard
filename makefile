#
# makefile
#
# Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
# intellectual property of Joshua S. English, all rights reserved.
#
# The master make file for Asgard.
#
# Written by Josh English.
#
# Intended systems: Mingw, Linux32, Linux64
#

# define version

VERSION_NUMBER=0.9.34
VERSION_DATE=2012.04.30

# define system & platform settings

TARGET						= mingw
#TARGET						= linux32
#TARGET						= linux64

# set the directory structure (required for platform settings)

SRC_PATH					= src
INTERNAL_LIB_PATH			= lib
EXTERNAL_INC_PATH			= plugin/include
EXTERNAL_LIB_PATH			= plugin/lib

# obtain platform-specific settings

include conf/make/os/makefile.$(TARGET)

# define build settings

#OPTIMIZE_LEVEL				= 0
#OPTIMIZE_LEVEL				= 1
OPTIMIZE_LEVEL				= 3
#OPTIMIZE_LEVEL				= 9

BUILD_TYPE					= development
#BUILD_TYPE					= release

# define debugging settings

DEBUG_FLAGS					=

# set compilation flags

ifeq ($(BUILD_TYPE),development)
	BUILD_FLAGS				= -g -Wall
else
	BUILD_FLAGS				= -Wall -Werror
endif

ifeq ($(OPTIMIZE_LEVEL),0)
	OPTIMIZE_FLAGS			=
else
	ifeq ($(OPTIMIZE_LEVEL),1)
		OPTIMIZE_FLAGS		= -O
	else
		OPTIMIZE_FLAGS		= -O$(OPTIMIZE_LEVEL)
	endif
endif

ADDITIONAL_FLAGS			= \
#					-DMEMCACHE_BYPASS

# define internal libraries

INTERNAL_INCLUDES			= \
					-Isrc/c \
					-Isrc/cpp

INTERNAL_LIBS				= \
					-L$(INTERNAL_LIB_PATH) \
					-lasgardcore \
					-lasgardscript \
					-lasgardsearch \
					-lasgardhttpd \
					-lasgard \
					$(INTERNAL_LIB_PATH)/libasgardcore.a \
					$(INTERNAL_LIB_PATH)/libasgardscript.a \
					$(INTERNAL_LIB_PATH)/libasgardsearch.a \
					$(INTERNAL_LIB_PATH)/libasgardhttpd.a \
					$(INTERNAL_LIB_PATH)/libasgard.a
#					-lasgard \
#					$(INTERNAL_LIB_PATH)/libasgard.a

# define database libraries

DATABASE_INCLUDES			= \
#					-DDBFLAG_BERKELEY \
#					-DDBFLAG_POSTGRES

DATABASE_LIBS				= \
#					-L$(EXTERNAL_LIB_PATH)/berkeley \
#					-ldb \
#					-L$(EXTERNAL_LIB_PATH)/postgres \
#					-lpq

# define external libraries

EXTERNAL_INCLUDES			= \
					-I$(EXTERNAL_INC_PATH) \
					-I$(EXTERNAL_INC_PATH)/zlib \
					-I$(EXTERNAL_INC_PATH)/spidermonkey/linux \
					$(DATABASE_INCLUDES) \
					-I$(EXTERNAL_INC_PATH)/glew

EXTERNAL_LIBS				= \
					-L$(EXTERNAL_LIB_PATH) \
					-L$(EXTERNAL_LIB_PATH)/spidermonkey/linux \
					-ljs \
					$(DATABASE_LIBS) \
					-L$(EXTERNAL_LIB_PATH)/glew \
					-lGLEW \
#					-L/usr/X11R6/lib \
#					-lXxf86vm

# define compiler type

CC							= gcc
CC++						= g++
AR							= ar

# define compilation flags

COMPILE_FLAGS				= \
					$(DEBUG_FLAGS) \
					$(BUILD_FLAGS) \
					$(OPTIMIZE_FLAGS) \
					$(ADDITIONAL_FLAGS) \
					$(SYSTEM_FLAGS) \
					-DVERSION=$(VERSION_NUMBER) \
					-DVERSION_DATE=$(VERSION_DATE) \
					$(INTERNAL_INCLUDES) \
					$(EXTERNAL_INCLUDES)

LIBRARY_FLAGS				= \
					$(SYSTEM_LIBS) \
					$(EXTERNAL_LIBS) \
					$(INTERNAL_LIBS) \
					$(EXTERNAL_LIB_PATH)/zlib/libz.a \

# define make usages

CFLAGS						= $(COMPILE_FLAGS)
CPPFLAGS					=

# define unit test and application setup

TEST_PATH					= unit
APP_PATH					= bin

# define objects

include conf/make/makefile.core.objects
include conf/make/makefile.script.objects
include conf/make/makefile.search.objects
include conf/make/makefile.httpd.objects
include conf/make/makefile.asgard.objects

#
# legacy projects
#
# include conf/make/makefile.wowstats.objects

OBJS						= \
					$(CORE_OBJS) \
					$(SCRIPT_OBJS) \
					$(SEARCH_OBJS) \
					$(HTTPD_OBJS) \
					$(ASGARD_OBJS)
#
# legacy projects
#
#					$(WOWSTATS_OBJS) \
#

# define tests

include conf/make/makefile.tests

# define applications

include conf/make/makefile.apps

# make

all:		$(LIBS) $(TESTS) $(APPS)

# define object builds

include conf/make/makefile.core.object.dependency
include conf/make/makefile.script.object.dependency
include conf/make/makefile.search.object.dependency
include conf/make/makefile.httpd.object.dependency
include conf/make/makefile.asgard.object.dependency

#
# legacy projects
#
# include conf/make/makefile.wowstats.object.dependency
#


# define library builds

$(LIBS):			$(OBJS)
	$(REMOVE) $(INTERNAL_LIB_PATH)/libasgard*
	$(AR) cq $(INTERNAL_LIB_PATH)/libasgardcore.a $(CORE_OBJS)
	ranlib $(INTERNAL_LIB_PATH)/libasgardcore.a
	$(AR) cq $(INTERNAL_LIB_PATH)/libasgardscript.a $(SCRIPT_OBJS)
	ranlib $(INTERNAL_LIB_PATH)/libasgardscript.a
	$(AR) cq $(INTERNAL_LIB_PATH)/libasgardsearch.a $(SEARCH_OBJS)
	ranlib $(INTERNAL_LIB_PATH)/libasgardsearch.a
	$(AR) cq $(INTERNAL_LIB_PATH)/libasgardhttpd.a $(HTTPD_OBJS)
	ranlib $(INTERNAL_LIB_PATH)/libasgardhttpd.a
	$(AR) cq $(INTERNAL_LIB_PATH)/libasgard.a $(ASGARD_OBJS)
	ranlib $(INTERNAL_LIB_PATH)/libasgard.a
#	$(CC) -shared -o $(INTERNAL_LIB_PATH)/libasgardcore_unitTest.so $(CORE_OBJS)

#
# don't build shared-libraries
#
#	$(CC) -shared -o $(INTERNAL_LIB_PATH)/libasgardcore.so $(CORE_OBJS)
#	$(CC) -shared -o $(INTERNAL_LIB_PATH)/libasgardscript.so $(CORE_OBJS)
#	$(CC) -shared -o $(INTERNAL_LIB_PATH)/libasgardsearch.so $(CORE_OBJS)
#	$(CC) -shared -o $(INTERNAL_LIB_PATH)/libasgardhttpd.so $(CORE_OBJS)
#	$(CC) -shared -o $(INTERNAL_LIB_PATH)/libasgard.so $(ASGARD_OBJS)


#
# legacy projects
#
#	$(AR) cq $(INTERNAL_LIB_PATH)/libasgardwowstats.a $(WOWSTATS_OBJS)
#	ranlib $(INTERNAL_LIB_PATH)/libasgardwowstats.a
#	$(CC) -shared -o $(INTERNAL_LIB_PATH)/libasgardwowstats.so $(WOWSTATS_OBJS)


# define test application builds

include conf/make/makefile.test.dependency

# define application builds

include conf/make/makefile.app.dependency

apps: $(APPS)

# rebuild the project-dependencies

depends:
	./src/perl/make/makeDepends.pl

# generate doxygen-documentation

docs:
	$(REMOVE) doc/doxygen/html/*
	doxygen conf/doxygen/doxygen.config

# execute unit tests

tests: $(TESTS)
	./bin/executeUnitTests.sh

# execute cleanup

clean:
	$(REMOVE) $(OBJS)
	$(REMOVE) $(LIBS)
	$(REMOVE) $(TESTS)
	$(REMOVE) $(APPS)

