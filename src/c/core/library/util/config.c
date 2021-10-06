/*
 * config.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Unix/Linux-style configuration-file interface utility library.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#define _CORE_LIBRARY_TYPES_COMPONENT
#include "core/library/util/config.h"

// define config private constants

typedef enum _ConfigLineTypes {
	CONFIG_LINE_TYPE_COMMENT = 1,
	CONFIG_LINE_TYPE_SECTION,
	CONFIG_LINE_TYPE_PROPERTY,
	CONFIG_LINE_TYPE_UNKNOWN,
	CONFIG_LINE_TYPE_ERROR = -1
} ConfigLineTypes;


// declare config private functions

// general functions

static int cleanLine(char *line, int lineLength);

static aboolean stringHasWhitespace(char *string, int length);

static int getLineType(char *line, int lineLength);

// section functions

static char *parseSectionName(char *line, int lineLength);

static void initSection(ConfigSection *section, char *sectionName,
		char *comment);

static int newSection(Config *config, char *sectionName, char *comment);

// property functions

static char *getPropertyKey(char *line, int lineLength);

static char *getPropertyValue(char *line, int lineLength);

static char *buildPropertyKey(char *sectionName, char *propertyKey);

static int newProperty(Config *config, int sectionRef, char *propertyKey,
		char *propertyValue, char *propertyComment);

// config functions

static int readConfigFile(Config *config);

static int writeConfigFile(Config *config, char *filename);


// define config private functions

// general functions

static int cleanLine(char *line, int lineLength)
{
	aboolean lineStarted = afalse;
	int ii = 0;
	int nn = 0;
	int lineEndRef = 0;

	for(ii = 0, nn = 0; ii < lineLength; ii++) {
		if(((unsigned int)line[ii] == 9) || // tab
				((unsigned int)line[ii] == 10) || // new line
				((unsigned int)line[ii] == 13) // carriage return
		  		) {
			line[ii] = ' ';
		}
		if(((unsigned int)line[ii] < 32) || (((unsigned int)line[ii] > 126))) {
			continue;
		}
		if(!lineStarted) {
			if(line[ii] == ' ') {
				continue;
			}
			lineStarted = atrue;
		}
		line[nn] = line[ii];
		if(line[nn] != ' ') {
			lineEndRef = (nn + 1);
		}
		nn++;
	}

	for(ii = lineEndRef; ii < lineLength; ii++) {
		line[ii] = '\0';
	}

	return lineEndRef;
}

static aboolean stringHasWhitespace(char *string, int length)
{
	int ii = 0;

	for(ii = 0; ii < length; ii++) {
		if(ctype_isWhitespace(string[ii])) {
			return atrue;
		}
	}

	return afalse;
}

static int getLineType(char *line, int lineLength)
{
	aboolean hasDelimiter = afalse;
	int ii = 0;
	int preDelimiterCount = 0;
	int postDelimiterCount = 0;

	if(lineLength < 1) {
		return CONFIG_LINE_TYPE_UNKNOWN;
	}

	// determine if the line is a comment

	if((line[0] == '#') ||
			(line[0] == '\'') ||
			(line[0] == ';')) {
		return CONFIG_LINE_TYPE_COMMENT;
	}

	if(lineLength < 2) {
		return CONFIG_LINE_TYPE_UNKNOWN;
	}

	if(((line[0] == '/') && (line[1] == '/')) ||
			((line[0] == '/') && (line[1] == '*'))
			) {
		return CONFIG_LINE_TYPE_COMMENT;
	}

	// determine if the line is a section

	if((line[0] == '[') && (line[(lineLength - 1)] == ']')) {
		return CONFIG_LINE_TYPE_SECTION;
	}

	// determine if the line is a property

	for(ii = 0; ii < lineLength; ii++) {
		if(hasDelimiter) {
			postDelimiterCount++;
		}
		else {
			if(line[ii] == '=') {
				hasDelimiter = atrue;
			}
			else {
				preDelimiterCount++;
			}
		}
	}

	if((hasDelimiter) && (preDelimiterCount > 0) &&
			(postDelimiterCount > 0)) {
		return CONFIG_LINE_TYPE_PROPERTY;
	}

	return CONFIG_LINE_TYPE_UNKNOWN;
}

static char *getComment(char *line, int lineLength)
{
	aboolean isVisible = afalse;
	int ii = 0;
	int nn = 0;
	char *result = NULL;

	result = (char *)malloc(sizeof(char) * (lineLength + 1));

	for(ii = 0, nn = 0; ii < lineLength; ii++) {
		if(!isVisible) {
			if((line[ii] == '#') || (line[ii] == '\'') || (line[ii] == ';')) {
				isVisible = atrue;
				continue;
			}

			if(ii < (lineLength - 1)) {
				if(((line[(ii + 0)] == '/') && (line[(ii + 1)] == '/')) ||
						((line[(ii + 0)] == '/') && (line[(ii + 1)] == '*'))) {
					isVisible = atrue;
					continue;
				}
			}
		}
		else {
			result[nn] = line[ii];
			nn++;
		}
	}

	if(nn == 0) {
		free(result);
		return NULL;
	}

	return result;
}

// section functions

static char *parseSectionName(char *line, int lineLength)
{
	aboolean isVisible = afalse;
	int ii = 0;
	int nn = 0;
	char *result = NULL;

	result = (char *)malloc(sizeof(char) * (lineLength + 1));

	for(ii = 0, nn = 0; ii < lineLength; ii++) {
		if(line[ii] == ']') {
			break;
		}

		if(isVisible) {
			result[nn] = line[ii];
			nn++;
		}

		if(line[ii] == '[') {
			isVisible = atrue;
		}
	}

	if(nn == 0) {
		free(result);
		return NULL;
	}

	return result;
}

static void initSection(ConfigSection *section, char *sectionName,
		char *comment)
{
	memset(section, 0, sizeof(ConfigSection));

	section->propertyLength = 0;
	section->name = sectionName;
	section->comment = comment;
	section->keys = NULL;
	section->values = NULL;
	section->comments = NULL;
}

static int newSection(Config *config, char *sectionName, char *comment)
{
	int sectionRef = 0;
	char *sectionValue = NULL;

	if(config->sections == NULL) {
		sectionRef = 0;
		config->sectionLength = 1;
		config->sections = (ConfigSection *)malloc(
				sizeof(ConfigSection) * config->sectionLength);
	}
	else {
		sectionRef = config->sectionLength;
		config->sectionLength += 1;
		config->sections = (ConfigSection *)realloc(config->sections,
				(sizeof(ConfigSection) * config->sectionLength));
	}

	initSection(&config->sections[sectionRef], sectionName, comment);

	sectionValue = strdup(sectionName);

	if(bptree_put(&config->index, sectionName, (int)strlen(sectionName),
				sectionValue) < 0) {
		free(sectionValue);
		return -1;
	}

	return sectionRef;
}

// property functions

static char *getPropertyKey(char *line, int lineLength)
{
	int ii = 0;
	int nn = 0;
	int lineEndRef = 0;
	char *result = NULL;

	result = (char *)malloc(sizeof(char) * (lineLength + 1));

	for(ii = 0, nn = 0; ii < lineLength; ii++) {
		if((line[ii] == ':') || (line[ii] == '=')) {
			break;
		}

		result[nn] = line[ii];

		if(result[nn] != ' ') {
			lineEndRef = (nn + 1);
		}

		nn++;
	}

	if(nn == 0) {
		free(result);
		return NULL;
	}

	if(lineEndRef > 0) {
		for(ii = lineEndRef; ii < lineLength; ii++) {
			result[ii] = '\0';
		}
	}

	return result;
}

static char *getPropertyValue(char *line, int lineLength)
{
	aboolean isVisible = atrue;
	aboolean hasDelimiter = afalse;
	int ii = 0;
	int nn = 0;
	int lineEndRef = 0;
	char *result = NULL;

	result = (char *)malloc(sizeof(char) * (lineLength + 1));

	for(ii = 0, nn = 0; ii < lineLength; ii++) {
		if(!hasDelimiter) {
			if(line[ii] == '=') {
				hasDelimiter = atrue;
				ii++;
				while(ii < lineLength) {
					if((line[ii] != ' ') && (line[ii] != '=')) {
						ii--;
						break;
					}
					ii++;
				}
			}
		}
		else {
			if((isVisible) && (line[ii] == '#') && (line[(ii - 1)] == ' ')) {
				break;
			}
			else if(line[ii] == '"') {
				if(isVisible) {
					isVisible = afalse;
				}
				else {
					isVisible = atrue;
				}
			}
			else {
				result[nn] = line[ii];
				if((isVisible) && (result[nn] != ' ')) {
					lineEndRef = (nn + 1);
				}
				nn++;
			}
		}
	}

	if(nn == 0) {
		free(result);
		return NULL;
	}

	if(lineEndRef > 0) {
		for(ii = lineEndRef; ii < lineLength; ii++) {
			result[ii] = '\0';
		}
	}

	return result;
}

static char *buildPropertyKey(char *sectionName, char *propertyKey)
{
	int length = 0;
	char *result = NULL;

	length = (int)(strlen(sectionName) +
			strlen(propertyKey) +
			8);

	result = (char *)malloc(sizeof(char) * (length + 1));

	snprintf(result, length, "/%s/%s", sectionName, propertyKey);

	return result;
}

static int newProperty(Config *config, int sectionRef, char *propertyKey,
		char *propertyValue, char *propertyComment)
{
	int propertyRef = 0;
	char *indexKey = NULL;
	char *indexValue = NULL;

	if(config->sectionLength < 1) {
		return -1;
	}

	indexKey = buildPropertyKey(config->sections[sectionRef].name, propertyKey);
	indexValue = strdup(propertyValue);
	if(bptree_put(&config->index, indexKey, (int)strlen(indexKey),
				indexValue) < 0) {
		free(indexKey);
		free(indexValue);
		return -2;
	}

	free(indexKey);

	if(config->sections[sectionRef].propertyLength == 0) {
		propertyRef = 0;
		config->sections[sectionRef].propertyLength = 1;
		config->sections[sectionRef].keys = (char **)malloc(
				sizeof(char *) * config->sections[sectionRef].propertyLength);
		config->sections[sectionRef].values = (char **)malloc(
				sizeof(char *) * config->sections[sectionRef].propertyLength);
		config->sections[sectionRef].comments = (char **)malloc(
				sizeof(char *) * config->sections[sectionRef].propertyLength);
	}
	else {
		propertyRef = config->sections[sectionRef].propertyLength;
		config->sections[sectionRef].propertyLength += 1;
		config->sections[sectionRef].keys = (char **)realloc(
				config->sections[sectionRef].keys,
				(sizeof(char *) * config->sections[sectionRef].propertyLength));
		config->sections[sectionRef].values = (char **)realloc(
				config->sections[sectionRef].values,
				(sizeof(char *) * config->sections[sectionRef].propertyLength));
		config->sections[sectionRef].comments = (char **)realloc(
				config->sections[sectionRef].comments,
				(sizeof(char *) * config->sections[sectionRef].propertyLength));
	}

	config->sections[sectionRef].keys[propertyRef] = propertyKey;
	config->sections[sectionRef].values[propertyRef] = propertyValue;
	config->sections[sectionRef].comments[propertyRef] = propertyComment;

	return 0;
}

// config functions

static int readConfigFile(Config *config)
{
	aboolean hasError = afalse;
	aboolean isFirstBlock = afalse;
	int rc = 0;
	int lineType = 0;
	int readLength = 0;
	int bufferLength = 0;
	int commentLength = 0;
	int commentBufferLength = 0;
	char buffer[8192];
	char *sectionName = NULL;
	char *propertyKey = NULL;
	char *propertyValue = NULL;
	char *comment = NULL;
	char *commentBuffer = NULL;
	char *errorMessage = NULL;

	FileHandle fh;

	if((rc = file_init(&fh, config->filename, "r", 65536)) < 0) {
		if(EXPLICIT_ERRORS) {
			file_getError(&fh, rc, &errorMessage);
			fprintf(stderr, "[%s():%i] error - failed to open file '%s' "
					"with '%s'.\n", __FUNCTION__, __LINE__, config->filename,
					errorMessage);
			free(errorMessage);
		}
		file_free(&fh);
		return -1;
	}

	rc = file_readLine(&fh, buffer, ((int)sizeof(buffer) - 1), &readLength);
	while(rc == 0) {
		config->lineCount++;

		bufferLength = cleanLine(buffer, readLength);
		lineType = getLineType(buffer, bufferLength);

		switch(lineType) {
			case CONFIG_LINE_TYPE_COMMENT:
				if((comment = getComment(buffer, bufferLength)) != NULL) {
					if((commentLength = strlen(comment)) > 0) {
						commentBufferLength += (commentLength + 8);
						if(commentBuffer == NULL) {
							commentBuffer = (char *)malloc(sizeof(char) *
									commentBufferLength);
							strcat(commentBuffer, comment);
						}
						else {
							commentBuffer = (char *)realloc(commentBuffer,
									(sizeof(char) * commentBufferLength));
							strcat(commentBuffer, "\n");
							strcat(commentBuffer, comment);
						}
					}

					free(comment);
				}

				if(config->lineCount <= 1) {
					isFirstBlock = atrue;
				}
				break;

			case CONFIG_LINE_TYPE_SECTION:
				isFirstBlock = afalse;
				sectionName = parseSectionName(buffer, bufferLength);
				if(sectionName == NULL) {
					errorMessage = "invalid section name";
					hasError = atrue;
				}
				else {
					if(newSection(config, sectionName, commentBuffer) < 0) {
						errorMessage = "duplicate section name";
						hasError = atrue;
					}
				}

				commentBufferLength = 0;
				commentBuffer = NULL;
				break;

			case CONFIG_LINE_TYPE_PROPERTY:
				isFirstBlock = afalse;
				if((propertyKey = getPropertyKey(buffer,
								bufferLength)) == NULL) {
					if(commentBuffer != NULL) {
						free(commentBuffer);
					}
					errorMessage = "invalid property key";
					hasError = atrue;
					break;
				}

				if((propertyValue = getPropertyValue(buffer,
								bufferLength)) == NULL) {
					free(propertyKey);
					if(commentBuffer != NULL) {
						free(commentBuffer);
					}
					errorMessage = "invalid property value";
					hasError = atrue;
					break;
				}

				rc = newProperty(config, (config->sectionLength - 1),
						propertyKey, propertyValue, commentBuffer);
				if(rc == -1) {
					free(propertyKey);
					free(propertyValue);
					errorMessage = "unable to specify property with no section";
					hasError = atrue;
				}
				else if(rc == -2) {
					free(propertyKey);
					free(propertyValue);
					errorMessage = "duplicate property key";
					hasError = atrue;
				}

				if((hasError) && (commentBuffer != NULL)) {
					free(commentBuffer);
				}

				commentBufferLength = 0;
				commentBuffer = NULL;
				break;

			case CONFIG_LINE_TYPE_UNKNOWN:
				if(isFirstBlock) {
					isFirstBlock = afalse;
					if(commentBuffer != NULL) {
						config->comment = commentBuffer;
					}

					commentBufferLength = 0;
					commentBuffer = NULL;
				}
				break;
		}

		if(hasError) {
			break;
		}

		rc = file_readLine(&fh, buffer, ((int)sizeof(buffer) - 1), &readLength);
	}

	if(hasError) {
		if(EXPLICIT_ERRORS) {
			fprintf(stderr, "[%s():%i] error - failed to read file '%s' at "
					"line %i with '%s'.\n", __FUNCTION__, __LINE__,
					config->filename, config->lineCount, errorMessage);
		}
	}
	else if(rc < 0) {
		if(EXPLICIT_ERRORS) {
			file_getError(&fh, rc, &errorMessage);
			fprintf(stderr, "[%s():%i] error - failed to read file '%s' at "
					"line %i with '%s'.\n", __FUNCTION__, __LINE__,
					config->filename, config->lineCount, errorMessage);
			free(errorMessage);
		}
		hasError = atrue;
	}

	if((rc = file_free(&fh)) < 0) {
		if(EXPLICIT_ERRORS) {
			file_getError(&fh, rc, &errorMessage);
			fprintf(stderr, "[%s():%i] error - failed to close file '%s' "
					"with '%s'.\n", __FUNCTION__, __LINE__, config->filename,
					errorMessage);
			free(errorMessage);
		}
		hasError = atrue;
	}

	if(commentBuffer != NULL) {
		free(commentBuffer);
	}

	if(hasError) {
		return -1;
	}

	return 0;
}

static int writeConfigFile(Config *config, char *filename)
{
	int ii = 0;
	int nn = 0;
	int rc = 0;
	int length = 0;
	char *errorMessage = NULL;
	char timestamp[32];

	FileHandle fh;

	if((rc = file_init(&fh, filename, "w", 65536)) < 0) {
		if(EXPLICIT_ERRORS) {
			file_getError(&fh, rc, &errorMessage);
			fprintf(stderr, "[%s():%i] error - failed to open file '%s' "
					"with '%s'.\n", __FUNCTION__, __LINE__, filename,
					errorMessage);
			free(errorMessage);
		}
		file_free(&fh);
		return -1;
	}

	if(config->comment != NULL) {
		file_writeChar(&fh, '#');

		length = strlen(config->comment);
		for(ii = 0; ii < length; ii++) {
			if((ctype_ctoi(config->comment[ii]) == 10) ||
					(ctype_ctoi(config->comment[ii]) == 13)) {
				file_write(&fh, "\n#", strlen("\n#"));
			}
			else {
				file_writeChar(&fh, config->comment[ii]);
			}
		}

		file_write(&fh, "\n", strlen("\n"));
	}

	time_getTimestamp(timestamp);

	file_write(&fh, "#\n", strlen("#\n"));
	file_write(&fh, "# Configuration file written: ",
			strlen("# Configuration file written: "));
	file_write(&fh, timestamp, strlen(timestamp));
	file_write(&fh, "\n", strlen("\n"));
	file_write(&fh, "#\n", strlen("#\n"));

	if(config->comment != NULL) {
		file_write(&fh, "\n", strlen("\n"));
	}

	if(config->sections != NULL) {
		for(ii = 0; ii < config->sectionLength; ii++) {
			if(config->sections[ii].comment != NULL) {
				file_write(&fh, "\n#\n#", strlen("\n#\n#"));

				length = strlen(config->sections[ii].comment);
				for(nn = 0; nn < length; nn++) {
					if((ctype_ctoi(config->sections[ii].comment[nn]) ==
								10) ||
							(ctype_ctoi(config->sections[ii].comment[nn]) ==
							 13)) {
						file_write(&fh, "\n#", strlen("\n#"));
					}
					else {
						file_writeChar(&fh, config->sections[ii].comment[nn]);
					}
				}

				file_write(&fh, "\n#\n", strlen("\n#\n"));
			}
			else {
				file_write(&fh, "\n#\n", strlen("\n#\n"));
				file_write(&fh, "# ", strlen("# "));
				file_write(&fh, config->sections[ii].name,
						strlen(config->sections[ii].name));
				file_write(&fh, "\n#\n", strlen("\n#\n"));
			}

			file_write(&fh, "\n", strlen("\n"));
			file_writeChar(&fh, '[');
			file_write(&fh, config->sections[ii].name,
					strlen(config->sections[ii].name));
			file_writeChar(&fh, ']');
			file_write(&fh, "\n", strlen("\n"));

			if((config->sections[ii].keys == NULL) ||
					(config->sections[ii].values == NULL) ||
					(config->sections[ii].comments == NULL)) {
				continue;
			}

			for(nn = 0; nn < config->sections[ii].propertyLength; nn++) {
				file_write(&fh, config->sections[ii].keys[nn],
						strlen(config->sections[ii].keys[nn]));

				file_writeChar(&fh, '=');

				if(stringHasWhitespace(config->sections[ii].values[nn],
							strlen(config->sections[ii].values[nn]))) {
					file_writeChar(&fh, '"');
					file_write(&fh, config->sections[ii].values[nn],
							strlen(config->sections[ii].values[nn]));
					file_writeChar(&fh, '"');
				}
				else {
					file_write(&fh, config->sections[ii].values[nn],
							strlen(config->sections[ii].values[nn]));
				}

				file_write(&fh, "\n", strlen("\n"));

				if(config->sections[ii].comments[nn] != NULL) {
					file_writeChar(&fh, '#');
					file_write(&fh, config->sections[ii].comments[nn],
							strlen(config->sections[ii].comments[nn]));
					file_write(&fh, "\n", strlen("\n"));
				}
			}
		}
	}

	if((rc = file_free(&fh)) < 0) {
		if(EXPLICIT_ERRORS) {
			file_getError(&fh, rc, &errorMessage);
			fprintf(stderr, "[%s():%i] error - failed to close file '%s' "
					"with '%s'.\n", __FUNCTION__, __LINE__, filename,
					errorMessage);
			free(errorMessage);
		}
		return -1;
	}

	return 0;
}


// define config public functions

int config_init(Config *config, char *filename)
{
	aboolean fileExists = afalse;

	if(config == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	memset(config, 0, (int)(sizeof(Config)));

	if(filename != NULL) {
		system_fileExists(filename, &fileExists);
		if(!fileExists) {
			if(EXPLICIT_ERRORS) {
				fprintf(stderr, "[%s():%i] error - unable to open config file "
						"'%s', file not found.\n", __FUNCTION__, __LINE__,
						filename);
			}
			return -1;
		}
	}

	config->lineCount = 0;
	config->sectionLength = 0;
	config->filename = NULL;
	config->comment = NULL;
	config->sections = NULL;

	if(bptree_init(&config->index) < 0) {
		return -1;
	}

	if(filename != NULL) {
		config->filename = strdup(filename);
		return readConfigFile(config);
	}

	return 0;
}

int config_free(Config *config)
{
	int ii = 0;
	int nn = 0;

	if(config == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(config->filename != NULL) {
		free(config->filename);
	}

	if(config->comment != NULL) {
		free(config->comment);
	}

	if(config->sections != NULL) {
		for(ii = 0; ii < config->sectionLength; ii++) {
			if(config->sections[ii].name != NULL) {
				free(config->sections[ii].name);
			}
			if(config->sections[ii].comment != NULL) {
				free(config->sections[ii].comment);
			}
			if((config->sections[ii].keys != NULL) &&
					(config->sections[ii].values != NULL) &&
					(config->sections[ii].comments != NULL)) {
				for(nn = 0; nn < config->sections[ii].propertyLength; nn++) {
					if(config->sections[ii].keys[nn] != NULL) {
						free(config->sections[ii].keys[nn]);
					}
					if(config->sections[ii].values[nn] != NULL) {
						free(config->sections[ii].values[nn]);
					}
					if(config->sections[ii].comments[nn] != NULL) {
						free(config->sections[ii].comments[nn]);
					}
				}
				free(config->sections[ii].keys);
				free(config->sections[ii].values);
				free(config->sections[ii].comments);
			}
		}
		free(config->sections);
	}

	if(bptree_free(&config->index) < 0) {
		return -1;
	}

	memset(config, 0, sizeof(Config));

	return 0;
}

int config_display(void *stream, Config *config)
{
	int ii = 0;
	int nn = 0;

	if((stream == NULL) || (config == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	fprintf(stream, "Config filename '%s' ::\n", config->filename);
	fprintf(stream, "Config header comment '%s' ::\n", config->comment);

	if(config->sections == NULL) {
		return -1;
	}

	for(ii = 0; ii < config->sectionLength; ii++) {
		fprintf(stream, "Config section '%s' ::\n", config->sections[ii].name);
		fprintf(stream, "\t Comment: '%s'\n", config->sections[ii].comment);

		if((config->sections[ii].keys == NULL) ||
				(config->sections[ii].values == NULL) ||
				(config->sections[ii].comments == NULL)) {
			continue;
		}

		for(nn = 0; nn < config->sections[ii].propertyLength; nn++) {
			fprintf(stream, "\t %03i Property '%s' => '%s'\n", nn,
					config->sections[ii].keys[nn],
					config->sections[ii].values[nn]);
			if(config->sections[ii].comments[nn] != NULL) {
				fprintf(stream, "\t\t Comment: '%s'\n",
						config->sections[ii].comments[nn]);
			}
		}
	}

	return 0;
}

int config_write(Config *config, char *filename)
{
	if((config == NULL) || (filename == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return writeConfigFile(config, filename);
}

aboolean config_exists(Config *config, char *sectionName, char *propertyName)
{
	char *key = NULL;
	char *value = NULL;

	if((config == NULL) || (sectionName == NULL) || (propertyName == NULL)) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	key = buildPropertyKey(sectionName, propertyName);

	if(bptree_get(&config->index, key, (int)strlen(key), (void *)&value) < 0) {
		free(key);
		return afalse;
	}

	free(key);

	return atrue;
}

int config_getBoolean(Config *config, char *sectionName, char *propertyName,
		aboolean defaultPropertyValue, aboolean *propertyValue)
{
	char *key = NULL;
	char *value = NULL;

	if((config == NULL) || (sectionName == NULL) || (propertyName == NULL) ||
			(propertyValue == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	*propertyValue = defaultPropertyValue;

	key = buildPropertyKey(sectionName, propertyName);
	if(bptree_get(&config->index, key, (int)strlen(key), (void *)&value) < 0) {
		free(key);
		return 0;
	}

	free(key);

	if(!strcasecmp(value, "true")) {
		*propertyValue = atrue;
	}
	else if(!strcasecmp(value, "false")) {
		*propertyValue = afalse;
	}
	else {
		if(atoi(value) != 0) {
			*propertyValue = atrue;
		}
		else {
			*propertyValue = afalse;
		}
	}

	return 0;
}

int config_putBoolean(Config *config, char *sectionName, char *propertyName,
		aboolean propertyValue)
{
	int ii = 0;
	int sectionRef = -1;
	char *key = NULL;
	char *value = NULL;

	if((config == NULL) || (sectionName == NULL) || (propertyName == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	key = buildPropertyKey(sectionName, propertyName);

	if(bptree_get(&config->index, key, (int)strlen(key), (void *)&value) == 0) {
		free(key);
		return -1;
	}

	free(key);

	for(ii = 0; ii < config->sectionLength; ii++) {
		if(!strcmp(config->sections[ii].name, sectionName)) {
			sectionRef = ii;
			break;
		}
	}

	if(sectionRef < 0) {
		key = strdup(sectionName);
		if((sectionRef = newSection(config, key, NULL)) < 0) {
			return -1;
		}
	}

	key = strdup(propertyName);

	if(propertyValue) {
		value = strdup("true");
	}
	else {
		value = strdup("false");
	}

	if(newProperty(config, sectionRef, key, value, NULL) < 0) {
		free(key);
		free(value);
		return -1;
	}

	return 0;
}

int config_getInteger(Config *config, char *sectionName, char *propertyName,
		int defaultPropertyValue, int *propertyValue)
{
	char *key = NULL;
	char *value = NULL;

	if((config == NULL) || (sectionName == NULL) || (propertyName == NULL) ||
			(propertyValue == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	*propertyValue = defaultPropertyValue;

	key = buildPropertyKey(sectionName, propertyName);
	if(bptree_get(&config->index, key, (int)strlen(key), (void *)&value) < 0) {
		free(key);
		return 0;
	}

	free(key);

	*propertyValue = atoi(value);

	return 0;
}

int config_putInteger(Config *config, char *sectionName, char *propertyName,
		int propertyValue)
{
	int ii = 0;
	int sectionRef = -1;
	char *key = NULL;
	char *value = NULL;

	if((config == NULL) || (sectionName == NULL) || (propertyName == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	key = buildPropertyKey(sectionName, propertyName);

	if(bptree_get(&config->index, key, (int)strlen(key), (void *)&value) == 0) {
		free(key);
		return -1;
	}

	free(key);

	for(ii = 0; ii < config->sectionLength; ii++) {
		if(!strcmp(config->sections[ii].name, sectionName)) {
			sectionRef = ii;
			break;
		}
	}

	if(sectionRef < 0) {
		key = strdup(sectionName);
		if((sectionRef = newSection(config, key, NULL)) < 0) {
			return -1;
		}
	}

	key = strdup(propertyName);
	value = itoa(propertyValue);

	if(newProperty(config, sectionRef, key, value, NULL) < 0) {
		free(key);
		free(value);
		return -1;
	}

	return 0;
}

int config_getBigInteger(Config *config, char *sectionName, char *propertyName,
		alint defaultPropertyValue, alint *propertyValue)
{
	char *key = NULL;
	char *value = NULL;

	if((config == NULL) || (sectionName == NULL) || (propertyName == NULL) ||
			(propertyValue == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	*propertyValue = defaultPropertyValue;

	key = buildPropertyKey(sectionName, propertyName);
	if(bptree_get(&config->index, key, (int)strlen(key), (void *)&value) < 0) {
		free(key);
		return 0;
	}

	free(key);

	*propertyValue = (alint)atod(value);

	return 0;
}

int config_putBigInteger(Config *config, char *sectionName, char *propertyName,
		alint propertyValue)
{
	int ii = 0;
	int sectionRef = -1;
	char *key = NULL;
	char *value = NULL;

	if((config == NULL) || (sectionName == NULL) || (propertyName == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	key = buildPropertyKey(sectionName, propertyName);

	if(bptree_get(&config->index, key, (int)strlen(key), (void *)&value) == 0) {
		free(key);
		return -1;
	}

	free(key);

	for(ii = 0; ii < config->sectionLength; ii++) {
		if(!strcmp(config->sections[ii].name, sectionName)) {
			sectionRef = ii;
			break;
		}
	}

	if(sectionRef < 0) {
		key = strdup(sectionName);
		if((sectionRef = newSection(config, key, NULL)) < 0) {
			return -1;
		}
	}

	key = strdup(propertyName);
	value = bitoa(propertyValue);

	if(newProperty(config, sectionRef, key, value, NULL) < 0) {
		free(key);
		free(value);
		return -1;
	}

	return 0;
}

int config_getDouble(Config *config, char *sectionName, char *propertyName,
		double defaultPropertyValue, double *propertyValue)
{
	char *key = NULL;
	char *value = NULL;

	if((config == NULL) || (sectionName == NULL) || (propertyName == NULL) ||
			(propertyValue == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	*propertyValue = defaultPropertyValue;

	key = buildPropertyKey(sectionName, propertyName);
	if(bptree_get(&config->index, key, (int)strlen(key), (void *)&value) < 0) {
		free(key);
		return 0;
	}

	free(key);

	*propertyValue = atod(value);

	return 0;
}

int config_putDouble(Config *config, char *sectionName, char *propertyName,
		double propertyValue)
{
	int ii = 0;
	int sectionRef = -1;
	char *key = NULL;
	char *value = NULL;

	if((config == NULL) || (sectionName == NULL) || (propertyName == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	key = buildPropertyKey(sectionName, propertyName);

	if(bptree_get(&config->index, key, (int)strlen(key), (void *)&value) == 0) {
		free(key);
		return -1;
	}

	free(key);

	for(ii = 0; ii < config->sectionLength; ii++) {
		if(!strcmp(config->sections[ii].name, sectionName)) {
			sectionRef = ii;
			break;
		}
	}

	if(sectionRef < 0) {
		key = strdup(sectionName);
		if((sectionRef = newSection(config, key, NULL)) < 0) {
			return -1;
		}
	}

	key = strdup(propertyName);
	value = dtoa(propertyValue);

	if(newProperty(config, sectionRef, key, value, NULL) < 0) {
		free(key);
		free(value);
		return -1;
	}

	return 0;
}

int config_getString(Config *config, char *sectionName, char *propertyName,
		char *defaultPropertyValue, char **propertyValue)
{
	char *key = NULL;
	char *value = NULL;

	if((config == NULL) || (sectionName == NULL) || (propertyName == NULL) ||
			(defaultPropertyValue == NULL) || (propertyValue == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	*propertyValue = NULL;

	key = buildPropertyKey(sectionName, propertyName);
	if(bptree_get(&config->index, key, (int)strlen(key), (void *)&value) < 0) {
		*propertyValue = defaultPropertyValue;
	}
	else {
		*propertyValue = value;
	}

	free(key);

	return 0;
}

int config_putString(Config *config, char *sectionName, char *propertyName,
		char *propertyValue)
{
	int ii = 0;
	int sectionRef = -1;
	char *key = NULL;
	char *value = NULL;

	if((config == NULL) || (sectionName == NULL) || (propertyName == NULL) ||
			(propertyValue == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	key = buildPropertyKey(sectionName, propertyName);

	if(bptree_get(&config->index, key, (int)strlen(key), (void *)&value) == 0) {
		free(key);
		return -1;
	}

	free(key);

	for(ii = 0; ii < config->sectionLength; ii++) {
		if(!strcmp(config->sections[ii].name, sectionName)) {
			sectionRef = ii;
			break;
		}
	}

	if(sectionRef < 0) {
		key = strdup(sectionName);
		if((sectionRef = newSection(config, key, NULL)) < 0) {
			return -1;
		}
	}

	key = strdup(propertyName);
	value = strdup(propertyValue);

	if(newProperty(config, sectionRef, key, value, NULL) < 0) {
		free(key);
		free(value);
		return -1;
	}

	return 0;
}

