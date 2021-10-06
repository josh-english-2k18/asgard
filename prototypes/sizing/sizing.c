/*
 * sizing.c - a data-type size test
 *
 * compile with: gcc -g -O -Wall sizing.c -o sizing
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int main()
{
	struct stat stats;

	printf("sizeof off_t         : %i\n", (int)sizeof(stats.st_size));
	printf("sizeof long long int : %i\n", (int)sizeof(long long int));

	return 0;
}

