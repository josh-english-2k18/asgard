/*
 * climits.c - a simple C-limit test
 *
 * compile with: gcc -g -O -Wall climits.c -o climits
 */

#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <limits.h>

//#define SIZEOF_CHAR							sizeof(char)
//#define SIZEOF_UNSIGNED_CHAR				sizeof(unsigned char)

//#if (SIZEOF_CHAR != 1)
//#if SIZEOF_CHAR != CHAR_MAX
//#	error "Determined size of char is incorrect for this compiler."
//#endif

#define MAX_SIGNED_CHAR						127

#if (MAX_SIGNED_CHAR != CHAR_MAX)
#	error "Determined MAX_SIGNED_CHAR is incorrect for this compiler."
#endif

#define MAX_UNSIGNED_CHAR					255u

#if (MAX_UNSIGNED_CHAR != UCHAR_MAX)
#	error "Determined MAX_UNSIGNED_CHAR is incorrect for this compiler."
#endif

#define MAX_SIGNED_SHORT					32767

#if (MAX_SIGNED_SHORT != SHRT_MAX)
#	error "Determined MAX_SIGNED_SHORT is incorrect for this compiler."
#endif

#define MAX_UNSIGNED_SHORT					65535u

#if (MAX_UNSIGNED_SHORT != USHRT_MAX)
#	error "Determined MAX_UNSIGNED_SHORT is incorrect for this compiler."
#endif

#define MAX_SIGNED_INT						2147483647

#if (MAX_SIGNED_INT != INT_MAX)
#	error "Determined MAX_SIGNED_INT is incorrect for this compiler."
#endif

#define MAX_UNSIGNED_INT					4294967295u

#if (MAX_UNSIGNED_INT != UINT_MAX)
#	error "Determined MAX_UNSIGNED_INT is incorrect for this compiler."
#endif

#define MAX_SIGNED_FLOAT					\
	340282346638528859811704183484516925440.0

#if defined(__MINGW_H)
#	define MAX_SIGNED_LONG_INT				2147483647
#else // !__MINGW_H
#	define MAX_SIGNED_LONG_INT				9223372036854775807
#endif // __MINGW_H

#if (MAX_SIGNED_LONG_INT != LONG_MAX)
#	error "Determined MAX_SIGNED_LONG_INT is incorrect for this compiler."
#endif

#if defined(__MINGW_H)
#	define MAX_UNSIGNED_LONG_INT			4294967295u
#else // !__MINGW_H
#	define MAX_UNSIGNED_LONG_INT			18446744073709551615u
#endif // __MINGW_H

#if (MAX_UNSIGNED_LONG_INT != ULONG_MAX)
#	error "Determined MAX_UNSIGNED_LONG_LONG is incorrect for this compiler."
#endif

#define MAX_SIGNED_DOUBLE					179769313486231570814527423731704356798070567525844996598917476803157260780028538760589558632766878171540458953514382464234321326889464182768467546703537516986049910576551282076245490090389328944075868508455133942304583236903222948165808559332123348274797826204144723168738177180919299881250404026184124858368.0

int main()
{
	char cvalue = 0;
	unsigned char ucvalue = 0;
	short svalue = 0;
	unsigned short usvalue = 0;
	int ivalue = 0;
	unsigned int uivalue = 0;
	long long lvalue = 0;
	unsigned long long ulvalue = 0;
	float fvalue = 0.0;
	double dvalue = 0.0;
	char *ptr = (char *)malloc(sizeof(char) * 8);

#if defined(__MINGW_H)
	printf("using MinGW compiler\n");
#endif // __MINGW_H

	printf("max char   : %i\n", CHAR_MAX);
	cvalue = CHAR_MAX;
	printf("max char   : %i\n", cvalue);

	printf("max uchar  : %i\n", UCHAR_MAX);
	ucvalue = UCHAR_MAX;
	printf("max uchar  : %i\n", ucvalue);

	printf("max short  : %i\n", SHRT_MAX);
	svalue = SHRT_MAX;
	printf("max short  : %i\n", svalue);

	printf("max ushort : %i\n", USHRT_MAX);
	usvalue = USHRT_MAX;
	printf("max ushort : %i\n", usvalue);

	printf("max int    : %i\n", INT_MAX);
	ivalue = INT_MAX;
	printf("max int    : %i\n", ivalue);

	printf("max uint   : %u\n", UINT_MAX);
	uivalue = UINT_MAX;
	printf("max uint   : %u\n", uivalue);

	printf("max float  : %f\n", FLT_MAX);
	fvalue = FLT_MAX;
	printf("max float  : %f\n", fvalue);

	printf("max long   : %li\n", LONG_MAX);
	lvalue = LONG_MAX;
#if defined(__MINGW_H)
	printf("max long   : %0.0f\n", (double)lvalue);
#else // !__MINGW_H
	printf("max long   : %lli\n", lvalue);
#endif // __MINGW_H

	printf("max ulong  : %lu\n", ULONG_MAX);
	ulvalue = ULONG_MAX;
#if defined(__MINGW_H)
	printf("max ulong  : %0.0f (%i)\n", (double)ulvalue, sizeof(ulvalue));
#else // !__MINGW_H
	printf("max ulong  : %llu\n", ulvalue);
#endif // __MINGW_H

	printf("max double : %f\n", DBL_MAX);
	dvalue = DBL_MAX;
	printf("max double : %f\n", dvalue);

	printf("location is 0x%0.0f\n", (double)((unsigned long int)ptr));

	return 0;
}

