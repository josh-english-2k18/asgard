/*
 * html2text.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard web spidering engine - HTML-to-text prototype.
 *
 * Written by Josh English.
 */

// preprocessor directives

#define _CORE_COMMON_BYPASS_STD_STREAM
#include "core/core.h"


// define html2text data types

typedef struct _HtmlText {
	int textLength;
	char *textBody;
} HtmlText;


// declare html2text functions

int _fileError(int code, FileHandle *handle, const char *function, size_t line);

int prototype_html2text_processHtml(HtmlText *ht, char *html, int htmlLength);

int prototype_html2text_processFile(HtmlText *ht, char *filename);


// define html2text macros

#define fileError(code) _fileError(code, &handle, __FUNCTION__, __LINE__);


// main function

int main(int argc, char *argv[])
{
	double timer = 0.0;

	HtmlText ht;

	signal_registerDefault();

	timer = time_getTimeMus();

//	printf("Asgard HTML-to-Text Prototype Ver %s on %s.\n\n",
//			ASGARD_VERSION, ASGARD_DATE);

	if(argc != 2) {
		fprintf(stderr, "error - invalid or missing filename argument.\n");
		return 1;
	}

	if(prototype_html2text_processFile(&ht, argv[1]) < 0) {
		return 1;
	}

//	printf("\nProcessing complete, total run time: %0.6f seconds\n",
//			time_getElapsedMusInSeconds(timer));

	return 0;
}


// define html2text functions

int _fileError(int code, FileHandle *handle, const char *function, size_t line)
{
	int rc = 0;
	char *errorMessage = NULL;

	if(code != 0) {
		rc = file_getError(handle, code, &errorMessage);
	}
	else {
		rc = -1;
	}
	if(rc < 0) {
		fprintf(stderr, "[%s():%i] error(%i) - aborting.\n", function, 
				(int)line, code);
	}
	else {
		fprintf(stderr, "[%s():%i] error(%i) - '%s', aborting.\n", function,
				(int)line, code, errorMessage);
	}

	if(errorMessage != NULL) {
		free(errorMessage);
	}

	return -1;
}

aboolean compareString(char *html, int ref, int htmlLength, char *string,
		int stringLength)
{
	int ii = 0;
	int nn = 0;

	if((ref + stringLength) >= htmlLength) {
		return afalse;
	}

	for(ii = ref, nn = 0; ((ii < htmlLength) && (nn < stringLength));
			ii++, nn++) {
		if((ctype_toLower(html[ii]) != string[nn]) &&
				(ctype_toUpper(html[ii]) != string[nn])) {
			return afalse;
		}
	}

	return atrue;
}

int prototype_html2text_processHtml(HtmlText *ht, char *html, int htmlLength)
{
	aboolean isVisible = atrue;
	aboolean isInTag = afalse;
	aboolean isInStyle = afalse;
	aboolean isInScript = afalse;
	int ii = 0;
	int nn = 0;
	int intValue = 0;
	int tagDepth = 0;

	if((ht == NULL) || (html == NULL) || (htmlLength < 1)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	ht->textLength = (htmlLength + 1);
	ht->textBody = (char *)malloc(sizeof(char) * ht->textLength);

	for(ii = 0, nn = 0; ii < htmlLength; ii++) {
		if(!ctype_isPlainText(html[ii])) {
			continue;
		}

		if(isVisible) {
			if((!isInTag) && (!isInStyle) && (!isInScript)) {
				if(html[ii] == '<') {
					if(compareString(html, ii, htmlLength, "<style", 6)) {
						ii += 5;
						isInStyle = atrue;
//printf("[STYLE ] => '%c'\n", html[ii]);
					}
					else if(compareString(html, ii, htmlLength, "<script", 7)) {
						ii += 6;
						isInScript = atrue;
//printf("[SCRIPT] => '%c'\n", html[ii]);
					}
					else {
						isInTag = atrue;
						tagDepth = 1;
//printf("[TAG   ] => '%c'\n", html[ii]);
					}
					continue;
				}
			}
			else if((isInTag) && (html[ii] == '<')) {
				tagDepth++;
			}

			if(html[ii] == '"') {
				isVisible = afalse;
				continue;
			}

			if(isInStyle) {
				if(compareString(html, ii, htmlLength, "</style>", 8)) {
					ii += 7;
					isInStyle = afalse;
				}
				continue;
			}
			else if(isInScript) {
				if(compareString(html, ii, htmlLength, "</script>", 9)) {
					ii += 8;
					isInScript = afalse;
				}
				continue;
			}
			else if(isInTag) {
//printf("[IN TAG] => '%c'\n", html[ii]);
				if(html[ii] == '>') {
					tagDepth--;
					if(tagDepth <= 0) {
						isInTag = afalse;
					}
				}
				continue;
			}

			intValue = (int)((unsigned char)html[ii]);
			if(intValue == 9) {
				ht->textBody[nn] = ' ';
			}
			else if(intValue != 13) {
				ht->textBody[nn] = html[ii];
			}

			if(nn > 0) {
				if(((ht->textBody[nn] == ' ') ||
							(ht->textBody[nn] == '\x0a')) &&
						((ht->textBody[(nn - 1)] == ' ') ||
						 (ht->textBody[(nn - 1)] == '\x0a'))) {
					nn--;
				}
			}
//printf("[ADDED ] => '%c' : '%s'\n", ht->textBody[nn], ht->textBody);

			nn++;
		}
		else {
//printf("[NO VIS] => '%c'\n", html[ii]);
			if(html[ii] == '"') {
				isVisible = atrue;
			}
		}
	}

//	printf("Text:\n\n%s\n", ht->textBody);
	printf("%s\n", ht->textBody);

	return 0;
}

int prototype_html2text_processFile(HtmlText *ht, char *filename)
{
	int rc = 0;
	int bufferLength = 0;
	alint fileLength = 0;
	double timer = 0.0;
	char *buffer = NULL;

	FileHandle handle;

	if((ht == NULL) || (filename == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	timer = time_getTimeMus();

	memset(ht, 0, (sizeof(HtmlText)));

	if((rc = file_init(&handle, filename, "rb", 8129)) < 0) {
		return fileError(rc);
	}

	if((rc = file_getFileLength(&handle, &fileLength)) < 0) {
		return fileError(rc);
	}

	bufferLength = (int)fileLength;
	buffer = (char *)malloc(sizeof(char) * (bufferLength + 1));

	if((rc = file_read(&handle, buffer, bufferLength)) < 0) {
		return fileError(rc);
	}

	if((rc = file_free(&handle)) < 0) {
		return fileError(rc);
	}

//	printf("\t...read %i bytes from file in %0.6f seconds\n",
//			bufferLength, time_getElapsedMusInSeconds(timer));

	rc = prototype_html2text_processHtml(ht, buffer, bufferLength);

	free(buffer);

	return rc;
}

