/*
 * config.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Unix/Linux-style configuration-file interface utility library, header
 * file.
 *
 * Written by Josh English.
 */

#if !defined(_CORE_LIBRARY_UTIL_CONFIG_H)

#define _CORE_LIBRARY_UTIL_CONFIG_H

#if !defined(_CORE_H) && !defined(_CORE_LIBRARY_H) && \
		!defined(_CORE_LIBRARY_UTIL_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _CORE_H || _CORE_LIBRARY_H || _CORE_LIBRARY_UTIL_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// define config public data types

typedef struct _ConfigSection {
	int propertyLength;
	char *name;
	char *comment;
	char **keys;
	char **values;
	char **comments;
} ConfigSection;

typedef struct _Config {
	int lineCount;
	int sectionLength;
	char *filename;
	char *comment;
	ConfigSection *sections;
	Bptree index;
} Config;


// declare config public functions

int config_init(Config *config, char *filename);

int config_free(Config *config);

int config_display(void *stream, Config *config);

int config_write(Config *config, char *filename);

aboolean config_exists(Config *config, char *sectionName, char *propertyName);

int config_getBoolean(Config *config, char *sectionName, char *propertyName,
		aboolean defaultPropertyValue, aboolean *propertyValue);

int config_putBoolean(Config *config, char *sectionName, char *propertyName,
		aboolean propertyValue);

int config_getInteger(Config *config, char *sectionName, char *propertyName,
		int defaultPropertyValue, int *propertyValue);

int config_putInteger(Config *config, char *sectionName, char *propertyName,
		int propertyValue);

int config_getBigInteger(Config *config, char *sectionName, char *propertyName,
		alint defaultPropertyValue, alint *propertyValue);

int config_putBigInteger(Config *config, char *sectionName, char *propertyName,
		alint propertyValue);

int config_getDouble(Config *config, char *sectionName, char *propertyName,
		double defaultPropertyValue, double *propertyValue);

int config_putDouble(Config *config, char *sectionName, char *propertyName,
		double propertyValue);

int config_getString(Config *config, char *sectionName, char *propertyName,
		char *defaultPropertyValue, char **propertyValue);

int config_putString(Config *config, char *sectionName, char *propertyName,
		char *propertyValue);


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _CORE_LIBRARY_UTIL_CONFIG_H

