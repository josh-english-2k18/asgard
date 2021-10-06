/*
 * xpath.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard core library XML library with an Xpath interface.
 *
 * Written by Josh English.
 */

// preprocessor directives

#define _CORE_COMMON_BYPASS_STD_STREAM
#include "core/core.h"
#define _CORE_LIBRARY_TYPES_COMPONENT
#include "core/library/types/xpath.h"


// delcare xpath private functions

// helper functions

static void __expandcat(char **string, int *stringRef, int *stringLength,
		char *addString, int addStringLength, const char *file,
		const char *function, size_t line);

// attribute functions

static void attributes_new(XpathNode *node, char *name, int nameLength,
		char *value, int valueLength);

// path functions

static int path_getDepth(char *path, int pathLength);

static char *path_getName(char *path, int pathLength, int id, int *nameLength);

// node functions

static void node_init(XpathNode *node);

static void node_new(Xpath *xpath);

static void node_free(XpathNode *node);

static void node_newChild(XpathNode *node);

static XpathNode *node_search(Xpath *xpath, char *path, int pathLength);

static XpathNode *node_buildSearch(Xpath *xpath, char *path, int pathLength);

static void node_display(void *stream, XpathNode *node, int id, int indent);

static void node_buildXml(XpathNode *node, char **document, int *documentRef,
		int *documentLength, int indent);

// xml functions

static int xml_parseNode(XpathNode *node, char *document, int documentRef,
		int documentLength, int *parseLength);

static int xml_parseDocument(Xpath *xpath, char *document, int documentLength);


// define xpath private macros

#define string_expandcat(string, stringRef, stringLength, addString, \
			addStringLength) \
		__expandcat(string, stringRef, stringLength, addString, \
				addStringLength, __FILE__, __FUNCTION__, __LINE__)


// define xpath private functions

// helper functions

static void __expandcat(char **string, int *stringRef, int *stringLength,
		char *addString, int addStringLength, const char *file,
		const char *function, size_t line)
{
	if((string == NULL) || (stringRef == NULL) || (stringLength == NULL) ||
			(addString == NULL) || (addStringLength < 1)) {
		return;
	}

	*string = memory_strxpndcat(*string, stringRef, stringLength, addString,
			addStringLength, file, function, line);
}

// attribute functions

static void attributes_new(XpathNode *node, char *name, int nameLength,
		char *value, int valueLength)
{
	int ref = 0;

	if((nameLength <= 0) || (valueLength <= 0)) {
		return;
	}

	if(node->attributes == NULL) {
		ref = 0;
		node->attributeLength = 1;
		node->attributes = (XpathAttributes *)malloc(sizeof(XpathAttributes) *
				node->attributeLength);
	}
	else {
		ref = node->attributeLength;
		node->attributeLength += 1;
		node->attributes = (XpathAttributes *)realloc(node->attributes,
				(sizeof(XpathAttributes) * node->attributeLength));
	}

	node->attributes[ref].name = strndup(name, nameLength);
	node->attributes[ref].nameLength = nameLength;
	node->attributes[ref].value = strndup(value, valueLength);
	node->attributes[ref].valueLength = valueLength;
}

// path functions

static int path_getDepth(char *path, int pathLength)
{
	int ii = 0;
	int counter = 0;

	for(ii = 0; ii < pathLength; ii++) {
		if(path[ii] == '/') {
			if((ii > 1) && (ii == (pathLength - 1))) {
				return -1; // path cannot end in '/'
			}
			counter++;
		}
	}

	return (counter - 1);
}

static char *path_getName(char *path, int pathLength, int id, int *nameLength)
{
	int ii = 0;
	int ref = 0;
	int length = 0;
	int counter = 0;
	char *result = NULL;

	*nameLength = 0;

	for(ii = 0; ii < pathLength; ii++) {
		if(path[ii] == '/') {
			counter++;
		}
		else if((counter - 2) == id) {
			if(result == NULL) {
				ref = 0;
				length = 8;
				result = (char *)malloc(sizeof(char) * length);
			}

			result[ref] = path[ii];
			ref++;

			if(ref >= length) {
				length *= 2;
				result = (char *)realloc(result, (sizeof(char) * length));
			}
		}
		else if((counter - 2) > id) {
			break;
		}
	}

	if(result != NULL) {
		result[ref] = '\0';
		*nameLength = ref;
	}

	return result;
}

// node functions

static void node_init(XpathNode *node)
{
	memset(node, 0, (int)(sizeof(XpathNode)));

	node->nameLength = 0;
	node->valueLength = 0;
	node->childrenLength = 0;
	node->attributeLength = 0;
	node->name = NULL;
	node->value = NULL;
	node->attributes = NULL;
	node->children = NULL;
}

static void node_new(Xpath *xpath)
{
	int ref = 0;

	if(xpath->children == NULL) {
		ref = 0;
		xpath->childrenLength = 1;
		xpath->children = (void *)malloc(sizeof(XpathNode) *
				xpath->childrenLength);
	}
	else {
		ref = xpath->childrenLength;
		xpath->childrenLength += 1;
		xpath->children = (void *)realloc(xpath->children,
				(sizeof(XpathNode) * xpath->childrenLength));
	}

	node_init(&xpath->children[ref]);
}

static void node_free(XpathNode *node)
{
	int ii = 0;

	XpathNode *children = NULL;

	if(node->name != NULL) {
		free(node->name);
	}

	if(node->value != NULL) {
		free(node->value);
	}

	if(node->attributes != NULL) {
		for(ii = 0; ii < node->attributeLength; ii++) {
			if(node->attributes[ii].name != NULL) {
				free(node->attributes[ii].name);
			}
			if(node->attributes[ii].value != NULL) {
				free(node->attributes[ii].value);
			}
		}

		free(node->attributes);
	}

	if(node->children != NULL) {
		children = (XpathNode *)node->children;

		for(ii = 0; ii < node->childrenLength; ii++) {
			node_free(&children[ii]);
		}

		free(node->children);
	}
}

static void node_newChild(XpathNode *node)
{
	int ref = 0;

	if(node->children == NULL) {
		ref = 0;
		node->childrenLength = 1;
		node->children = (void *)malloc(sizeof(XpathNode) *
				node->childrenLength);
	}
	else {
		ref = node->childrenLength;
		node->childrenLength += 1;
		node->children = (void *)realloc(node->children,
				(sizeof(XpathNode) * node->childrenLength));
	}

	node_init(&((XpathNode *)node->children)[ref]);
}

static XpathNode *node_search(Xpath *xpath, char *path, int pathLength)
{
	int ii = 0;
	int nn = 0;
	int pathDepth = 0;
	int pathNameLength = 0;
	char *pathName = NULL;

	XpathNode *node = NULL;
	XpathNode *result = NULL;

	if((path == NULL) || (pathLength < 3)) {
		return NULL;
	}

	if((path[0] != '/') || (path[1] != '/')) {
		return NULL;
	}

	if((pathDepth = path_getDepth(path, pathLength)) < 1) {
		return NULL;
	}

	for(ii = 0; ii < pathDepth; ii++) {
		if(((pathName = path_getName(path, pathLength, ii,
							&pathNameLength)) == NULL) ||
				(pathNameLength < 1)) {
			break;
		}

		if(ii == 0) {
			for(nn = 0; nn < xpath->childrenLength; nn++) {
				if((xpath->children[nn].nameLength == pathNameLength) &&
						(!strncasecmp(xpath->children[nn].name,
									  pathName,
									  pathNameLength))) {
					node = &(xpath->children[nn]);
					break;
				}
			}
		}
		else {
			for(nn = 0; nn < node->childrenLength; nn++) {
				if((((XpathNode *)node->children)[nn].nameLength ==
							pathNameLength) &&
						(!strncasecmp(((XpathNode *)node->children)[nn].name,
									  pathName,
									  pathNameLength))) {
					node = &(((XpathNode *)node->children)[nn]);
					break;
				}
			}
		}

		free(pathName);

		if(node == NULL) {
			break;
		}
	}

	if((node != NULL) && (ii == pathDepth)) {
		result = node;
	}

	return result;
}

static XpathNode *node_buildSearch(Xpath *xpath, char *path, int pathLength)
{
	aboolean buildNode = afalse;
	int ii = 0;
	int nn = 0;
	int pathDepth = 0;
	int pathNameLength = 0;
	char *pathName = NULL;

	XpathNode *node = NULL;
	XpathNode *result = NULL;

	if((path == NULL) || (pathLength < 3)) {
		return NULL;
	}

	if((path[0] != '/') || (path[1] != '/')) {
		return NULL;
	}

	if((pathDepth = path_getDepth(path, pathLength)) < 1) {
		return NULL;
	}

	for(ii = 0; ii < pathDepth; ii++) {
		if(((pathName = path_getName(path, pathLength, ii,
							&pathNameLength)) == NULL) ||
				(pathNameLength < 1)) {
			break;
		}

		if(ii == 0) {
			buildNode = atrue;

			if(xpath->childrenLength > 0) {
				for(nn = 0; nn < xpath->childrenLength; nn++) {
					if((xpath->children[nn].nameLength == pathNameLength) &&
							(!strncasecmp(xpath->children[nn].name,
										  pathName,
										  pathNameLength))) {
						buildNode = afalse;
						node = &(xpath->children[nn]);
						break;
					}
				}
			}

			if(buildNode) {
				nn = xpath->childrenLength;

				node_new(xpath);

				xpath->children[nn].name = strndup(pathName, pathNameLength);
				xpath->children[nn].nameLength = pathNameLength;

				node = &(xpath->children[nn]);
			}
		}
		else {
			buildNode = atrue;

			if(node->childrenLength > 0) {
				for(nn = 0; nn < node->childrenLength; nn++) {
					if((((XpathNode *)node->children)[nn].nameLength ==
								pathNameLength) &&
							(!strncasecmp(
								((XpathNode *)node->children)[nn].name,
								pathName,
								pathNameLength))) {
						buildNode = afalse;
						node = &(((XpathNode *)node->children)[nn]);
						break;
					}
				}
			}

			if(buildNode) {
				nn = node->childrenLength;

				node_newChild(node);

				((XpathNode *)node->children)[nn].name = strndup(pathName,
					pathNameLength);
				((XpathNode *)node->children)[nn].nameLength = pathNameLength;

				node = &(((XpathNode *)node->children)[nn]);
			}
		}

		free(pathName);

		if(node == NULL) {
			break;
		}
	}

	if((node != NULL) && (ii == pathDepth)) {
		result = node;
	}

	return result;
}

static void node_display(void *stream, XpathNode *node, int id, int indent)
{
	int ii = 0;
	char tabs[8192];

	memset(tabs, 0, (int)(sizeof(tabs)));

	if(indent > 0) {
		if(id == 0) {
			strcat(tabs, " ");
			for(ii = 0; ii < (indent - 1); ii++) {
				strcat(tabs, "    ");
			}
			strcat(tabs, "+---+---");
		}
		else {
			for(ii = 0; ii < indent; ii++) {
				strcat(tabs, "    ");
			}
			strcat(tabs, " +---");
		}
	}

	fprintf(stream, "Xpath :: %s [ Node '%s' ]\n", tabs, node->name);

	memset(tabs, 0, (int)(sizeof(tabs)));

	if(indent == 0) {
		strcat(tabs, " | - ");
	}
	else {
		for(ii = 0; ii < indent; ii++) {
			strcat(tabs, "    ");
		}
		strcat(tabs, " |      - ");
	}

	if(node->attributeLength > 0) {
		for(ii = 0; ii < node->attributeLength; ii++) {
			fprintf(stream, "Xpath :: %s Attr[%03i] : '%s' => '%s'\n", tabs,
					ii, node->attributes[ii].name, node->attributes[ii].value);
		}
	}
	else {
		fprintf(stream, "Xpath :: %s Attrs      : n/a\n", tabs);
	}

	fprintf(stream, "Xpath :: %s Value      : '%s'\n", tabs, node->value);

	if(node->childrenLength > 0) {
		for(ii = 0; ii < node->childrenLength; ii++) {
			node_display(stream, &((XpathNode *)node->children)[ii], ii,
					(indent + 1));
		}
	}
}

static void node_buildXml(XpathNode *node, char **document, int *documentRef,
		int *documentLength, int indent)
{
	int ii = 0;
	int length = 0;
	char tabs[8192];
	char buffer[8192];

	if(node->name == NULL) {
		return;
	}

	memset(tabs, 0, (int)(sizeof(tabs)));

	if(indent > 0) {
		for(ii = 0; ii < indent; ii++) {
			strcat(tabs, "\t");
		}

		string_expandcat(document, documentRef, documentLength,
				tabs, strlen(tabs));
	}

	string_expandcat(document, documentRef, documentLength, "<", 1);
	string_expandcat(document, documentRef, documentLength,
			node->name, node->nameLength);

	if(node->attributeLength > 0) {
		for(ii = 0; ii < node->attributeLength; ii++) {
			length = snprintf(buffer, (int)(sizeof(buffer) - 1),
					" %s=\"%s\"",
					node->attributes[ii].name,
					node->attributes[ii].value);

			string_expandcat(document, documentRef, documentLength,
					buffer, length);
		}
	}

	string_expandcat(document, documentRef, documentLength, ">", 1);

	if(node->value != NULL) {
		string_expandcat(document, documentRef, documentLength,
				node->value, node->valueLength);
	}

	if(node->childrenLength > 0) {
		string_expandcat(document, documentRef, documentLength, "\x0a", 1);

		for(ii = 0; ii < node->childrenLength; ii++) {
			node_buildXml(&((XpathNode *)node->children)[ii],
					document,
					documentRef,
					documentLength,
					(indent + 1));
		}

		length = snprintf(buffer, (int)(sizeof(buffer) - 1),
				"%s</%s>\x0a", tabs, node->name);
	}
	else {
		length = snprintf(buffer, (int)(sizeof(buffer) - 1),
				"</%s>\x0a", node->name);
	}

	string_expandcat(document, documentRef, documentLength,
			buffer, length);
}

// xml functions

static int xml_parseNode(XpathNode *node, char *document, int documentRef,
		int documentLength, int *parseLength)
{
	aboolean isNodeBegin = afalse;
	aboolean isNodeEnd = afalse;
	aboolean isVisible = atrue;
	aboolean isInComment = afalse;
	aboolean isInCdata = afalse;
	aboolean isInTag = afalse;
	aboolean isInTagName = afalse;
	aboolean hasTagName = afalse;
	aboolean hasTagAttributes = afalse;
	aboolean isAttrKey = afalse;
	aboolean isAttrValue = afalse;
	char isVisibleChar = (char)0;
	int ii = 0;
	int tagRef = 0;
	int tagLength = 0;
	int tagBeginRef = 0;
	int tagCounter = 0;
	int attrRef = 0;
	int attrLength = 0;
	int nodeRef = 0;
	int nodeBeginRef = 0;
	int nodeParseLength = 0;
	int cdataRef = 0;
	int contentRef = 0;
	int contentLength = 0;
	int childParseLength = 0;
	char *beginTag = NULL;
	char *currentTag = NULL;
	char *attrKey = NULL;
	char *attrValue = NULL;
	char *content = NULL;

	if((node == NULL) || (document == NULL) || (parseLength == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

/*
printf("BEGIN :::::::::::::::::::::::::::::::::::::::::::::\n");
printf("BEGIN document ref      : %i\n", documentRef);
printf("BEGIN document length   : %i\n", documentLength);
printf("BEGIN :::::::::::::::::::::::::::::::::::::::::::::\n");
printf("BEGIN ---------------------------------------------\n");
for(ii = documentRef; ii < documentLength; ii++) {
	printf("%c", document[ii]);
}
printf("BEGIN ---------------------------------------------\n");
printf("BEGIN :::::::::::::::::::::::::::::::::::::::::::::\n");
*/

	*parseLength = 0;

	if((documentRef < 0) || (documentLength < 0) ||
			(documentRef >= documentLength)) {
		return 0;
	}

	tagRef = 0;
	tagLength = 8;
	currentTag = (char *)malloc(sizeof(char) * tagLength);

	contentRef = 0;
	contentLength = 128;
	content = (char *)malloc(sizeof(char) * contentLength);

	for(ii = documentRef; ii < documentLength; ii++) {
//printf("PARSE :: %03i { inb %i, ine %i, iv %i, iic %i, iicd %i, iit %i, iitn %i, htn %i, hta %i, iak %i, iav %i, t#%i } => '%c' ['%s'/'%s']\n", ii, isNodeBegin, isNodeEnd, isVisible, isInComment, isInCdata, isInTag, isInTagName, hasTagName, hasTagAttributes, isAttrKey, isAttrValue, tagCounter, document[ii], beginTag, currentTag);

		if((isVisible) && ((document[ii] == '"') || (document[ii] == '\''))) {
			isVisible = afalse;
			isVisibleChar = document[ii];
			continue;
		}

		if((isVisible) &&
				(!isInComment) &&
				(ii < (documentLength - 3)) &&
				(document[(ii + 0)] == '<') &&
				(document[(ii + 1)] == '!') &&
				(document[(ii + 2)] == '-') &&
				(document[(ii + 3)] == '-')) {
			isInComment = atrue;
			continue;
		}

		if((isVisible) &&
				(!isInComment) &&
				(!isInCdata) &&
				(ii < (documentLength - 9)) &&
				(document[(ii + 0)] == '<') &&
				(document[(ii + 1)] == '!') &&
				(document[(ii + 2)] == '[') &&
				((document[(ii + 3)] == 'C') || (document[(ii + 3)] == 'c')) &&
				((document[(ii + 4)] == 'D') || (document[(ii + 4)] == 'd')) &&
				((document[(ii + 5)] == 'A') || (document[(ii + 5)] == 'a')) &&
				((document[(ii + 6)] == 'T') || (document[(ii + 6)] == 't')) &&
				((document[(ii + 7)] == 'A') || (document[(ii + 7)] == 'a')) &&
				(document[(ii + 8)] == '[')) {
			isInCdata = atrue;
			cdataRef = (ii + 9);
			continue;
		}

		if((isVisible) && (!isInComment) && (!isInCdata)) {
			if((!isInTag) && (document[ii] == '<')) {
				isInTag = atrue;
				isInTagName = atrue;
				hasTagAttributes = afalse;
				isAttrKey = afalse;
				isAttrValue = afalse;
				tagBeginRef = ii;
				tagRef = 0;
				memset(currentTag, 0, tagLength);
				continue;
			}
			else if(isInTag) {
				if(((ii < (documentLength - 1)) &&
							(document[(ii + 0)] == '/') &&
							(document[(ii + 1)] == '>')) ||
						(document[ii] == '>')) {
					isInTag = afalse;
					isInTagName = afalse;
					hasTagAttributes = afalse;
					isAttrKey = afalse;
					isAttrValue = afalse;

					if((tagCounter == 0) && (document[ii] == '>')) {
						isNodeBegin = atrue;
						nodeBeginRef = (ii + 1);
//printf("NODE %03i >>> BEGIN\n", nodeBeginRef);
					}

					if(!hasTagName) {
						hasTagName = atrue;

						beginTag = strndup(currentTag, tagRef);

						tagRef = 0;
						memset(currentTag, 0, tagLength);
//printf("TAG NAME %03i >>> '%s'\n", ii, beginTag);
					}
					else if((tagCounter == 0) &&\
							(document[(ii + 0)] == '/') &&
							(document[(ii + 1)] == '>')) {
						isNodeEnd = atrue;
						break;
					}
					else {
//printf("TAG NAME %03i <<< #%i => '%s'\n", ii, tagCounter, currentTag);
						if(!strcmp(beginTag, currentTag)) {
							isNodeEnd = atrue;
							break;
						}

						tagRef = 0;
						memset(currentTag, 0, tagLength);
					}

					tagCounter++;

					continue;
				}
				else if((document[ii] == ' ') ||
						(document[ii] == (char)9)) {
					isInTagName = afalse;

					if(!hasTagName) {
						hasTagName = atrue;

						beginTag = strndup(currentTag, tagRef);

						tagRef = 0;
						memset(currentTag, 0, tagLength);

//printf("TAG NAME %03i >>> '%s' (has attributes)\n", ii, beginTag);
					}
					else if(!hasTagAttributes) {
//printf("TAG NAME %03i <<< #%i => '%s'\n", ii, tagCounter, currentTag);
						if(!strcmp(beginTag, currentTag)) {
							isNodeEnd = atrue;
							break;
						}

						tagRef = 0;
						memset(currentTag, 0, tagLength);
					}

					hasTagAttributes = atrue;
				}

				if(isInTagName) {
					if((tagRef > 0) ||
							((tagRef == 0) && (document[ii] != '/'))) {
						currentTag[tagRef] = document[ii];
						tagRef++;
					}
					if(tagRef >= tagLength) {
						tagLength *= 2;
						currentTag = (char *)realloc(currentTag,
								(sizeof(char) * tagLength));
					}
					currentTag[tagRef] = '\0';
				}
			}
		}

		if((isVisible) &&
				(!isInComment) &&
				((!isInCdata) ||
				 ((isInCdata) && (ii >= cdataRef))) &&
				(!isInTag) &&
				(tagCounter == 1)) {
			if(((unsigned int)document[ii] >= 32) &&
					((unsigned int)document[ii] <= 126)) {
				content[contentRef] = document[ii];
				contentRef++;
				if(contentRef >= contentLength) {
					contentRef *= 2;
					content = (char *)realloc(content,
							(sizeof(char) * contentLength));
				}
			}
		}

		if((!isInComment) &&
				(!isInCdata) &&
				(isInTag) &&
				(hasTagName) &&
				(hasTagAttributes) &&
				(tagCounter == 0)) {
//printf("TAG ATTR [%s] %03i { %i, %i, %i } => '%c' :: '%s'/'%s'\n", beginTag, ii, hasTagAttributes, isAttrKey, isAttrValue, document[ii], attrKey, attrValue);
			if((!isAttrKey) && (!isAttrValue)) {
				if(((unsigned int)document[ii] > 32) &&
						((unsigned int)document[ii] < 126)) {
					isAttrKey = atrue;

					attrRef = 0;
					attrLength = 8;

					if(attrKey != NULL) {
						free(attrKey);
					}

					attrKey = (char *)malloc(sizeof(char) * attrLength);
				}
			}

			if(isAttrKey) {
				if((ii < (documentLength - 1)) &&
						(document[ii] == '=') &&
						((document[(ii + 1)] == '"') ||
						 (document[(ii + 1)] == '\''))) {
					isAttrKey = afalse;
					isAttrValue = atrue;

					attrRef = 0;
					attrLength = 8;

					if(attrValue != NULL) {
						free(attrValue);
					}

					attrValue = (char *)malloc(sizeof(char) *
							attrLength);
				}
				else {
					attrKey[attrRef] = document[ii];
					attrRef++;
					if(attrRef >= attrLength) {
						attrLength *= 2;
						attrKey = (char *)realloc(attrKey,
								(sizeof(char) * attrLength));
					}
					attrKey[attrRef] = '\0';
				}
			}
			else if(isAttrValue) {
				if((document[ii] == '"') || (document[ii] == '\'')) {
					attributes_new(node,
							attrKey,
							strlen(attrKey),
							attrValue,
							strlen(attrValue));

					isAttrValue = afalse;
					isAttrKey = afalse;
				}
				else if((attrRef > 0) ||
						((attrRef == 0) &&
						 (document[ii] != '"') &&
						 (document[ii] != '\''))) {
					attrValue[attrRef] = document[ii];
					attrRef++;
					if(attrRef >= attrLength) {
						attrLength *= 2;
						attrValue = (char *)realloc(attrValue,
								(sizeof(char) * attrLength));
					}
					attrValue[attrRef] = '\0';
				}
			}
		}
		else {
			attrRef = 0;
			attrLength = 0;

			if(attrKey != NULL) {
				free(attrKey);
				attrKey = NULL;
			}
			if(attrValue != NULL) {
				free(attrValue);
				attrValue = NULL;
			}
		}

		if((isInComment) &&
				(document[(ii - 2)] == '-') &&
				(document[(ii - 1)] == '-') &&
				(document[(ii - 0)] == '>')) {
			isInComment = afalse;
		}

		if((isInCdata) &&
				(document[(ii - 2)] == ']') &&
				(document[(ii - 1)] == ']') &&
				(document[(ii - 0)] == '>')) {
			isInCdata = afalse;

			if((contentRef >= 4) &&
					(content[(contentRef - 3)] == ']') &&
					(content[(contentRef - 2)] == ']') &&
					(content[(contentRef - 1)] == '>')) {
				contentRef -= 3;
				content[contentRef] = '\0';
			}
		}

		if(!isVisible) {
			if((document[ii] == isVisibleChar) &&
					((ii <= 0) ||
					 ((ii > 0) && (document[(ii - 1)] != '\\')))) {
				isVisible = atrue;
			}
		}
	}

	if(isNodeEnd) {
		node->nameLength = strlen(beginTag);
		node->name = strndup(beginTag, node->nameLength);

		if(contentRef > 0) {
			node->valueLength = contentRef;
			node->value = strndup(content, contentRef);
		}

		nodeParseLength = tagBeginRef;

		do {
			nodeRef = node->childrenLength;
			childParseLength = 0;

			node_newChild(node);

			if(xml_parseNode(&((XpathNode *)node->children)[nodeRef],
						document,
						nodeBeginRef,
						nodeParseLength,
						&childParseLength) < 0) {
				break;
			}

			nodeBeginRef = (childParseLength + 1);
		} while(childParseLength > 0);

		if(node->childrenLength > 0) {
			node->childrenLength -= 1;
		}

		*parseLength = ii;
	}

	// cleanup

	if(beginTag != NULL) {
		free(beginTag);
	}

	if(currentTag != NULL) {
		free(currentTag);
	}

	if(attrKey != NULL) {
		free(attrKey);
	}

	if(attrValue != NULL) {
		free(attrValue);
	}

	if(content != NULL) {
		free(content);
	}

//printf("END :::::::::::::::::::::::::::::::::::::::::::::\n");

	return 0;
}

static int xml_parseDocument(Xpath *xpath, char *document, int documentLength)
{
	int ref = 0;
	int result = 0;
	int parseLength = 0;
	int documentRef = 0;

	do {
		node_new(xpath);

		if((result = xml_parseNode(&xpath->children[ref],
						document,
						documentRef,
						documentLength,
						&parseLength)) < 0) {
			break;
		}

//printf("DOCUMENT REF     : %i\n", documentRef);
//printf("DOCUMENT LENGTH  : %i\n", documentLength);
//printf("PARSE LENGTH     : %i\n", parseLength);
//printf("DOC ----------------------------------------------------------\n");
//printf("DOC %s\n", (document + parseLength + 1));
//printf("DOC ----------------------------------------------------------\n");

		ref++;
		documentRef = (parseLength + 1);

//		break;
	} while(parseLength > 0);

	if(xpath->childrenLength > 0) {
		xpath->childrenLength -= 1;
	}

	return result;
}



// define xpath public functions

int xpath_init(Xpath *xpath)
{
	if(xpath == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	memset(xpath, 0, (sizeof(Xpath)));

	xpath->childrenLength = 0;
	xpath->children = NULL;

	return 0;
}

Xpath *xpath_new()
{
	Xpath *result = NULL;

	result = (Xpath *)malloc(sizeof(Xpath));

	xpath_init(result);

	return result;
}

int xpath_free(Xpath *xpath)
{
	int ii = 0;

	if(xpath == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(xpath->children != NULL) {
		for(ii = 0; ii < xpath->childrenLength; ii++) {
			node_free(&xpath->children[ii]);
		}

		free(xpath->children);
	}

	memset(xpath, 0, (sizeof(Xpath)));

	return 0;
}

int xpath_freePtr(Xpath *xpath)
{
	if(xpath == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	xpath_free(xpath);

	free(xpath);

	return 0;
}

int xpath_display(void *stream, Xpath *xpath)
{
	int ii = 0;

	if(xpath == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(stream == NULL) {
		stream = stdout;
	}

	for(ii = 0; ii < xpath->childrenLength; ii++) {
		node_display(stream, &xpath->children[ii], ii, 0);
	}

	return 0;
}

int xpath_parseXmlDocument(Xpath *xpath, char *document, int documentLength)
{
	if((xpath == NULL) || (document == NULL) || (documentLength < 1)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return xml_parseDocument(xpath, document, documentLength);
}

char *xpath_buildXmlDocudment(Xpath *xpath, int *documentLength)
{
	int ii = 0;
	int ref = 0;
	int length = 0;
	char *result = NULL;

	if((xpath == NULL) || (documentLength == NULL)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	length = 128;
	result = (char *)malloc(sizeof(char) * (length + 1));

	for(ii = 0; ii < xpath->childrenLength; ii++) {
		node_buildXml(&xpath->children[ii], &result, &ref, &length, 0);
	}

	*documentLength = length;

	return result;
}

aboolean xpath_getBoolean(Xpath *xpath, char *path, int pathLength, char *name,
		int nameLength)
{
	aboolean result = afalse;
	int ii = 0;

	XpathNode *node = NULL;
	XpathNode *childNode = NULL;

	if((xpath == NULL) || (path == NULL) || (pathLength < 1) ||
			(name == NULL) || (nameLength < 1)) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	if((node = node_search(xpath, path, pathLength)) == NULL) {
		return afalse;
	}

	if((node->nameLength == nameLength) &&
			(!strncasecmp(node->name, name, nameLength))) {
		if((node->value != NULL) &&
				(node->valueLength == 4) &&
				(!strncasecmp(node->value, "true", 4))) {
			result = atrue;
		}
	}
	else {
		for(ii = 0; ii < node->childrenLength; ii++) {
			if((((XpathNode *)node->children)[ii].nameLength == nameLength) &&
					(!strncasecmp(((XpathNode *)node->children)[ii].name,
								  name,
								  nameLength))) {
				childNode = &(((XpathNode *)node->children)[ii]);

				if(childNode->value == NULL) {
					break;
				}
			}
			else {
				continue;
			}

			if((childNode->valueLength == 4) &&
					(!strncasecmp(childNode->value, "true", 4))) {
				result = atrue;
			}

			break;
		}
	}

	return result;
}

int xpath_setBoolean(Xpath *xpath, char *path, int pathLength, char *name,
		int nameLength, aboolean value)
{
	int ii = 0;
	int result = 0;

	XpathNode *node = NULL;
	XpathNode *childNode = NULL;

	if((xpath == NULL) || (path == NULL) || (pathLength < 1) ||
			(name == NULL) || (nameLength < 1)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if((node = node_buildSearch(xpath, path, pathLength)) == NULL) {
		return -1;
	}

	result = -1;

	if((node->nameLength == nameLength) &&
			(!strncasecmp(node->name, name, nameLength))) {
		if(value) {
			node->value = strdup("true");
			node->valueLength = strlen(node->value);
		}
		else {
			node->value = strdup("false");
			node->valueLength = strlen(node->value);
		}

		result = 0;
	}
	else {
		for(ii = 0; ii < node->childrenLength; ii++) {
			if((((XpathNode *)node->children)[ii].nameLength == nameLength) &&
					(!strncasecmp(((XpathNode *)node->children)[ii].name,
								  name,
								  nameLength))) {
				childNode = &(((XpathNode *)node->children)[ii]);
			}
			else {
				continue;
			}

			if(value) {
				childNode->value = strdup("true");
				childNode->valueLength = strlen(childNode->value);
			}
			else {
				childNode->value = strdup("false");
				childNode->valueLength = strlen(childNode->value);
			}

			result = 0;

			break;
		}
	}

	return result;
}

int xpath_getInteger(Xpath *xpath, char *path, int pathLength, char *name,
		int nameLength)
{
	int result = 0;
	int ii = 0;

	XpathNode *node = NULL;
	XpathNode *childNode = NULL;

	if((xpath == NULL) || (path == NULL) || (pathLength < 1) ||
			(name == NULL) || (nameLength < 1)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if((node = node_search(xpath, path, pathLength)) == NULL) {
		return -1;
	}

	if((node->nameLength == nameLength) &&
			(!strncasecmp(node->name, name, nameLength))) {
		if(node->value != NULL) {
			result = atoi(node->value);
		}
	}
	else {
		for(ii = 0; ii < node->childrenLength; ii++) {
			if((((XpathNode *)node->children)[ii].nameLength == nameLength) &&
					(!strncasecmp(((XpathNode *)node->children)[ii].name,
								  name,
								  nameLength))) {
				childNode = &(((XpathNode *)node->children)[ii]);

				if(childNode->value == NULL) {
					break;
				}
			}
			else {
				continue;
			}

			result = atoi(childNode->value);

			break;
		}
	}

	return result;
}

int xpath_setInteger(Xpath *xpath, char *path, int pathLength, char *name,
		int nameLength, int value)
{
	int ii = 0;
	int length = 0;
	int result = 0;
	char buffer[128];

	XpathNode *node = NULL;
	XpathNode *childNode = NULL;

	if((xpath == NULL) || (path == NULL) || (pathLength < 1) ||
			(name == NULL) || (nameLength < 1)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if((node = node_buildSearch(xpath, path, pathLength)) == NULL) {
		return -1;
	}

	result = -1;

	if((node->nameLength == nameLength) &&
			(!strncasecmp(node->name, name, nameLength))) {
		length = snprintf(buffer, (int)(sizeof(buffer) - 1), "%i", value);

		node->value = strndup(buffer, length);
		node->valueLength = length;

		result = 0;
	}
	else {
		for(ii = 0; ii < node->childrenLength; ii++) {
			if((((XpathNode *)node->children)[ii].nameLength == nameLength) &&
					(!strncasecmp(((XpathNode *)node->children)[ii].name,
								  name,
								  nameLength))) {
				childNode = &(((XpathNode *)node->children)[ii]);
			}
			else {
				continue;
			}

			length = snprintf(buffer, (int)(sizeof(buffer) - 1), "%i", value);

			childNode->value = strndup(buffer, length);
			childNode->valueLength = length;

			result = 0;

			break;
		}
	}

	return result;
}

double xpath_getDouble(Xpath *xpath, char *path, int pathLength, char *name,
		int nameLength)
{
	int ii = 0;
	double result = 0.0;

	XpathNode *node = NULL;
	XpathNode *childNode = NULL;

	if((xpath == NULL) || (path == NULL) || (pathLength < 1) ||
			(name == NULL) || (nameLength < 1)) {
		DISPLAY_INVALID_ARGS;
		return 0.0;
	}

	if((node = node_search(xpath, path, pathLength)) == NULL) {
		return 0.0;
	}

	if((node->nameLength == nameLength) &&
			(!strncasecmp(node->name, name, nameLength))) {
		if(node->value != NULL) {
			result = atod(node->value);
		}
	}
	else {
		for(ii = 0; ii < node->childrenLength; ii++) {
			if((((XpathNode *)node->children)[ii].nameLength == nameLength) &&
					(!strncasecmp(((XpathNode *)node->children)[ii].name,
								  name,
								  nameLength))) {
				childNode = &(((XpathNode *)node->children)[ii]);

				if(childNode->value == NULL) {
					break;
				}
			}
			else {
				continue;
			}

			result = atod(childNode->value);

			break;
		}
	}

	return result;
}

int xpath_setDouble(Xpath *xpath, char *path, int pathLength, char *name,
		int nameLength, double value)
{
	int ii = 0;
	int length = 0;
	int result = 0;
	char buffer[128];

	XpathNode *node = NULL;
	XpathNode *childNode = NULL;

	if((xpath == NULL) || (path == NULL) || (pathLength < 1) ||
			(name == NULL) || (nameLength < 1)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if((node = node_buildSearch(xpath, path, pathLength)) == NULL) {
		return -1;
	}

	result = -1;

	if((node->nameLength == nameLength) &&
			(!strncasecmp(node->name, name, nameLength))) {
		length = snprintf(buffer, (int)(sizeof(buffer) - 1), "%0.6f", value);

		node->value = strndup(buffer, length);
		node->valueLength = length;

		result = 0;
	}
	else {
		for(ii = 0; ii < node->childrenLength; ii++) {
			if((((XpathNode *)node->children)[ii].nameLength == nameLength) &&
					(!strncasecmp(((XpathNode *)node->children)[ii].name,
								  name,
								  nameLength))) {
				childNode = &(((XpathNode *)node->children)[ii]);
			}
			else {
				continue;
			}

			length = snprintf(buffer, (int)(sizeof(buffer) - 1),
					"%0.6f", value);

			childNode->value = strndup(buffer, length);
			childNode->valueLength = length;

			result = 0;

			break;
		}
	}

	return result;
}

char *xpath_getString(Xpath *xpath, char *path, int pathLength, char *name,
		int nameLength, int *resultLength)
{
	int ii = 0;
	char *result = NULL;

	XpathNode *node = NULL;
	XpathNode *childNode = NULL;

	if((xpath == NULL) || (path == NULL) || (pathLength < 1) ||
			(name == NULL) || (nameLength < 1) || (resultLength == NULL)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	*resultLength = 0;

	if((node = node_search(xpath, path, pathLength)) == NULL) {
		return NULL;
	}

	if((node->nameLength == nameLength) &&
			(!strncasecmp(node->name, name, nameLength))) {
		*resultLength = node->valueLength;
		result = strndup(node->value, node->valueLength);
	}
	else {
		for(ii = 0; ii < node->childrenLength; ii++) {
			if((((XpathNode *)node->children)[ii].nameLength == nameLength) &&
					(!strncasecmp(((XpathNode *)node->children)[ii].name,
								  name,
								  nameLength))) {
				childNode = &(((XpathNode *)node->children)[ii]);

				if(childNode->value == NULL) {
					break;
				}
			}
			else {
				continue;
			}

			*resultLength = childNode->valueLength;
			result = strndup(childNode->value, childNode->valueLength);

			break;
		}
	}

	return result;
}

int xpath_setString(Xpath *xpath, char *path, int pathLength, char *name,
		int nameLength, char *value, int valueLength)
{
	int ii = 0;
	int result = 0;

	XpathNode *node = NULL;
	XpathNode *childNode = NULL;

	if((xpath == NULL) || (path == NULL) || (pathLength < 1) ||
			(name == NULL) || (nameLength < 1) || (value == NULL) ||
			(valueLength < 1)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if((node = node_buildSearch(xpath, path, pathLength)) == NULL) {
		return -1;
	}

	result = -1;

	if((node->nameLength == nameLength) &&
			(!strncasecmp(node->name, name, nameLength))) {
		node->value = strndup(value, valueLength);
		node->valueLength = valueLength;

		result = 0;
	}
	else {
		for(ii = 0; ii < node->childrenLength; ii++) {
			if((((XpathNode *)node->children)[ii].nameLength == nameLength) &&
					(!strncasecmp(((XpathNode *)node->children)[ii].name,
								  name,
								  nameLength))) {
				childNode = &(((XpathNode *)node->children)[ii]);
			}
			else {
				continue;
			}

			childNode->value = strndup(value, valueLength);
			childNode->valueLength = valueLength;

			result = 0;

			break;
		}
	}

	return result;
}

XpathAttributes *xpath_getAttributes(Xpath *xpath, char *path, int pathLength,
		char *name, int nameLength, int *resultLength)
{
	int ii = 0;

	XpathNode *node = NULL;
	XpathNode *childNode = NULL;
	XpathAttributes *result = NULL;

	if((xpath == NULL) || (path == NULL) || (pathLength < 1) ||
			(name == NULL) || (nameLength < 1) || (resultLength == NULL)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	*resultLength = 0;

	if((node = node_search(xpath, path, pathLength)) == NULL) {
		return NULL;
	}

	if((node->nameLength == nameLength) &&
			(!strncasecmp(node->name, name, nameLength))) {
		*resultLength = node->attributeLength;
		result = node->attributes;
	}
	else {
		for(ii = 0; ii < node->childrenLength; ii++) {
			if((((XpathNode *)node->children)[ii].nameLength == nameLength) &&
					(!strncasecmp(((XpathNode *)node->children)[ii].name,
								  name,
								  nameLength))) {
				childNode = &(((XpathNode *)node->children)[ii]);

				if((childNode->attributes == NULL) ||
						(childNode->attributeLength < 1)) {
					break;
				}
			}
			else {
				continue;
			}

			*resultLength = childNode->attributeLength;
			result = childNode->attributes;

			break;
		}
	}

	return result;
}

int xpath_setAttribute(Xpath *xpath, char *path, int pathLength, char *name,
		int nameLength, char *attrName, int attrNameLength, char *attrValue,
		int attrValueLength)
{
	int ii = 0;
	int result = 0;

	XpathNode *node = NULL;
	XpathNode *childNode = NULL;

	if((xpath == NULL) || (path == NULL) || (pathLength < 1) ||
			(name == NULL) || (nameLength < 1) ||
			(attrName == NULL) || (attrNameLength < 1) ||
			(attrValue == NULL) || (attrValueLength < 1)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if((node = node_buildSearch(xpath, path, pathLength)) == NULL) {
		return -1;
	}

	result = -1;

	if((node->nameLength == nameLength) &&
			(!strncasecmp(node->name, name, nameLength))) {
		attributes_new(node, attrName, attrNameLength, attrValue,
				attrValueLength);

		result = 0;
	}
	else {
		for(ii = 0; ii < node->childrenLength; ii++) {
			if((((XpathNode *)node->children)[ii].nameLength == nameLength) &&
					(!strncasecmp(((XpathNode *)node->children)[ii].name,
								  name,
								  nameLength))) {
				childNode = &(((XpathNode *)node->children)[ii]);
			}
			else {
				continue;
			}

			attributes_new(childNode, attrName, attrNameLength, attrValue,
					attrValueLength);

			result = 0;

			break;
		}
	}

	return result;
}

char *xpath_getStrings(Xpath *xpath, char *path, int pathLength, char *name,
		int nameLength, char *delimiter, int delimiterLength,
		int *resultLength)
{
	aboolean useDelimiter = afalse;
	int ii = 0;
	int ref = 0;
	int length = 0;
	char *result = NULL;

	XpathNode *node = NULL;
	XpathNode *childNode = NULL;

	if((xpath == NULL) || (path == NULL) || (pathLength < 1) ||
			(name == NULL) || (nameLength < 1) || (resultLength == NULL)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	*resultLength = 0;

	if((node = node_search(xpath, path, pathLength)) == NULL) {
		return NULL;
	}

	if((delimiter != NULL) && (delimiterLength > 0)) {
		useDelimiter = atrue;
	}

	ref = 0;
	length = 16;
	result = (char *)malloc(sizeof(char) * (length + 1));

	if((node->nameLength == nameLength) &&
			(!strncasecmp(node->name, name, nameLength))) {
		string_expandcat(&result, &ref, &length,
				node->value, node->valueLength);
	}

	for(ii = 0; ii < node->childrenLength; ii++) {
		if((((XpathNode *)node->children)[ii].nameLength == nameLength) &&
				(!strncasecmp(((XpathNode *)node->children)[ii].name,
							  name,
							  nameLength))) {
			childNode = &(((XpathNode *)node->children)[ii]);

			if(childNode->value == NULL) {
				continue;
			}
		}
		else {
			continue;
		}

		if((ref > 0) && (useDelimiter)) {
			string_expandcat(&result, &ref, &length,
					delimiter, delimiterLength);
		}

		string_expandcat(&result, &ref, &length,
				childNode->value, childNode->valueLength);
	}

	if(ref > 0) {
		result[ref] = '\0';
		*resultLength = ref;
	}
	else {
		free(result);
		result = NULL;
	}

	return result;
}

