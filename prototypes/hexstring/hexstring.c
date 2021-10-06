/*
 * hexstring.c - a test of converting a hex-string to a number
 *
 * compile with: gcc -g -O -Wall hexstring.c -o hexstring
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BITMASK_01						0x00F0000000000000

int main()
{
	int number32 = 0;
	long long int number64 = 0;
	char *string32 = "0x12345678";
//	char *string64 = "0x000000000009E925";
	char *string64 = "0x0000000000099DAF";

	sscanf(string32, "%i", &number32);

	printf("32-bit: '%s' -> %i\n", string32, number32);

	sscanf(string64, "%lli", &number64);

	printf("64-bit: '%s' -> %lli\n", string64, number64);
	printf("bitmask 01: 0x%llx\n", (number64 & BITMASK_01));

	return 0;
}

