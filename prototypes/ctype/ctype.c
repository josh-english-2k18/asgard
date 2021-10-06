/*
 * ctype.c - a simple language frequency counter
 *
 * compile with: gcc -g -O -Wall ctype.c -o ctype
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct _array {
	char value;
	int frequency;
} ARRAY;

static int sortArray(const void *iAlpha, const void *iBeta)
{
	ARRAY *alpha = (ARRAY *)iAlpha;
	ARRAY *beta = (ARRAY *)iBeta;

	return (beta->frequency - alpha->frequency);
}

int main()
{
	int ii = 0;
	int value = 0;
	int length = 0;
	int lowerFrequency[26];
	int upperFrequency[26];
	char buffer[65536];

	FILE *fd = NULL;

	ARRAY frequencyArray[52];

	memset(lowerFrequency, 0, sizeof(lowerFrequency));
	memset(upperFrequency, 0, sizeof(upperFrequency));

	fd = fopen("command.txt", "rb");
	if(fd == NULL) {
		return -1;
	}

	length = fread(buffer, sizeof(char), sizeof(buffer), fd);
	while(length > 0) {
		for(ii = 0; ii < length; ii++) {
			value = (int)((unsigned char)buffer[ii]);
			if((value >= 65) && (value <= 90)) { // uppercase
				upperFrequency[(value - 65)] += 1;
			}
			else if((value >= 97) && (value <= 122)) { // lowercase
				lowerFrequency[(value - 97)] += 1;
			}
		}

		length = fread(buffer, sizeof(char), sizeof(buffer), fd);
	}

	fclose(fd);

	for(ii = 0; ii < 26; ii++) {
//		printf("  %c : %8i\n", (char)(ii + 65), upperFrequency[ii]);
		frequencyArray[ii].value = (char)(ii + 65);
		frequencyArray[ii].frequency = upperFrequency[ii];
	}
	for(ii = 0; ii < 26; ii++) {
//		printf("  %c : %8i\n", (char)(ii + 97), lowerFrequency[ii]);
		frequencyArray[(ii + 26)].value = (char)(ii + 97);
		frequencyArray[(ii + 26)].frequency = lowerFrequency[ii];
	}

	qsort(frequencyArray, 52, sizeof(ARRAY), sortArray);

	for(ii = 0; ii < 52; ii++) {
		printf("  %c : %8i\n", frequencyArray[ii].value,
				frequencyArray[ii].frequency);
	}

	printf("array->");
	for(ii = 0; ii < 52; ii++) {
		printf("%c", frequencyArray[ii].value);
	}
	printf("<-\n");

	return 0;
}

