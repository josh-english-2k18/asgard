/*
 * xpath.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard core library XML library with an Xpath interface, header file.
 *
 * Written by Josh English.
 */

#if !defined(_CORE_LIBRARY_TYPES_XPATH_H)

#define _CORE_LIBRARY_TYPES_XPATH_H

#if !defined(_CORE_H) && !defined(_CORE_LIBRARY_H) && \
		!defined(_CORE_LIBRARY_TYPES_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _CORE_H || _CORE_LIBRARY_H || _CORE_LIBRARY_TYPES_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// define xpath public data types

typedef struct _XpathAttributes {
	int nameLength;
	int valueLength;
	char *name;
	char *value;
} XpathAttributes;

typedef struct _XpathNode {
	int nameLength;
	int valueLength;
	int childrenLength;
	int attributeLength;
	char *name;
	char *value;
	XpathAttributes *attributes;
	void *children;
} XpathNode;

typedef struct _Xpath {
	int childrenLength;
	XpathNode *children;
} Xpath;


// declare xpath public functions

int xpath_init(Xpath *xpath);

Xpath *xpath_new();

int xpath_free(Xpath *xpath);

int xpath_freePtr(Xpath *xpath);

int xpath_display(void *stream, Xpath *xpath);

int xpath_parseXmlDocument(Xpath *xpath, char *document, int documentLength);

char *xpath_buildXmlDocudment(Xpath *xpath, int *documentLength);

aboolean xpath_getBoolean(Xpath *xpath, char *path, int pathLength, char *name,
		int nameLength);

int xpath_setBoolean(Xpath *xpath, char *path, int pathLength, char *name,
		int nameLength, aboolean value);

int xpath_getInteger(Xpath *xpath, char *path, int pathLength, char *name,
		int nameLength);

int xpath_setInteger(Xpath *xpath, char *path, int pathLength, char *name,
		int nameLength, int value);

double xpath_getDouble(Xpath *xpath, char *path, int pathLength, char *name,
		int nameLength);

int xpath_setDouble(Xpath *xpath, char *path, int pathLength, char *name,
		int nameLength, double value);

char *xpath_getString(Xpath *xpath, char *path, int pathLength, char *name,
		int nameLength, int *resultLength);

int xpath_setString(Xpath *xpath, char *path, int pathLength, char *name,
		int nameLength, char *value, int valueLength);

XpathAttributes *xpath_getAttributes(Xpath *xpath, char *path, int pathLength,
		char *name, int nameLength, int *resultLength);

int xpath_setAttribute(Xpath *xpath, char *path, int pathLength, char *name,
		int nameLength, char *attrName, int attrNameLength, char *attrValue,
		int attrValueLength);

char *xpath_getStrings(Xpath *xpath, char *path, int pathLength, char *name,
		int nameLength, char *delimiter, int delimiterLength,
		int *resultLength);



#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _CORE_LIBRARY_TYPES_XPATH_H

