/*
 * integer_unitTest.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * A simple API for managing an optimized array of 32-bit integers, unit test.
 *
 * Written by Josh English.
 */

// preprocessor directives

#define _CORE_COMMON_BYPASS_STD_STREAM
#include "core/core.h"


// declare integer unit test constants

#define TEST_ONE_LENGTH						65536//1048576
#define TEST_TWO_LENGTH						65536//1048576


// declare integer unit test private functions

static int integer_unit_test_one();

static int integer_unit_test_two();


// main function

int main()
{
	signal_registerDefault();

	printf("[unit] Integer Library Unit Test, using Ver %s on %s.\n",
			ASGARD_VERSION, ASGARD_DATE);

	if(integer_unit_test_one() < 0) {
		printf("[unit]\t ...FAILED\n");
		return 1;
	}

	if(integer_unit_test_two() < 0) {
		printf("[unit]\t ...FAILED\n");
		return 1;
	}

	printf("[unit] ...PASSED\n");

	return 0;
}


// define integer unit test private functions

static int lrand()
{
	int ii = 0;
	int length = 0;
	int result = 0;

	length = ((rand() % 128) + 1);
	for(ii = 0; ii < length; ii++) {
		result += rand();
	}

	return result;
}

static void display_array(int *array, int length)
{
	int ii = 0;

	for(ii = 0; ii < length; ii++) {
		fprintf(stderr, "\t %4i \t -> \t %i\n", ii, array[ii]);
	}
}

static int integer_unit_test_one()
{
	int ii = 0;
	int pick = 0;
	int result = 0;
	int position = 0;
	int searchCount = 0;
	int removeCount = 0;
	int isSaneCount = 0;
	double timer = 0.0;
	double searchTime = 0.0;
	double addTime = 0.0;
	double getTime = 0.0;
	double removeTime = 0.0;
	double isSaneTime = 0.0;

	IntegerArray integer;

	printf("[unit]\t integer unit test #1...\n");

	system_pickRandomSeed();

	integer_init(&integer);

	for(ii = 0; ii < TEST_ONE_LENGTH; ii++) {
		if((ii > 0) && ((ii % 1024) == 0)) {
			fprintf(stderr, "[unit]\t ...%i of %i (%0.2f %%)\n", ii,
					TEST_ONE_LENGTH,
					(((double)ii / (double)TEST_ONE_LENGTH) * 100.0));
			system_pickRandomSeed();
		}

		// select a random number not already in the integer system

		pick = lrand();

		timer = time_getTimeMus();

		result = integer_contains(&integer, pick);

		searchTime += time_getElapsedMusInSeconds(timer);
		searchCount++;

		while(result >= 0) {
			pick = lrand();

			timer = time_getTimeMus();

			result = integer_contains(&integer, pick);

			searchTime += time_getElapsedMusInSeconds(timer);
			searchCount++;
		}

		// add the random number to the integer system

		timer = time_getTimeMus();

		if((position = integer_put(&integer, pick)) < 0) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		addTime += time_getElapsedMusInSeconds(timer);
		timer = time_getTimeMus();

		if(integer_get(&integer, position) != pick) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		getTime += time_getElapsedMusInSeconds(timer);

		// remove the number from the system at random

		if((rand() % 100) > 74) {
			pick = integer.array[(rand() % integer.length)];

			timer = time_getTimeMus();

			if(integer_remove(&integer, pick) < 0) {
				fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
						__FUNCTION__, __LINE__);
				return -1;
			}

			removeTime += time_getElapsedMusInSeconds(timer);
			removeCount++;
		}

		// check for sanity in the system at random

		if((rand() % 100) > 98) {
			timer = time_getTimeMus();

			if(!integer_isSane(&integer)) {
				fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
						__FUNCTION__, __LINE__);
				display_array(integer.array, integer.length);
				return -1;
			}

			isSaneTime += time_getElapsedMusInSeconds(timer);
			isSaneCount++;
		}
	}

	if(!integer_isSane(&integer)) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		display_array(integer.array, integer.length);
		return -1;
	}

	printf("[unit]\t\t current array length      : %i\n", integer.length);
	printf("[unit]\t\t total searches performed  : %i\n", searchCount);
	printf("[unit]\t\t total search time         : %0.6f\n", searchTime);
	printf("[unit]\t\t average search time       : %0.6f\n",
			(searchTime / (double)searchCount));
	printf("[unit]\t\t total adds performed      : %i\n", TEST_ONE_LENGTH);
	printf("[unit]\t\t total add time            : %0.6f\n", addTime);
	printf("[unit]\t\t average add time          : %0.6f\n",
			(addTime / (double)TEST_ONE_LENGTH));
	printf("[unit]\t\t total get time            : %0.6f\n", getTime);
	printf("[unit]\t\t average get time          : %0.6f\n",
			(addTime / (double)TEST_ONE_LENGTH));
	printf("[unit]\t\t total removes performed   : %i\n", removeCount);
	printf("[unit]\t\t total remove time         : %0.6f\n", removeTime);
	printf("[unit]\t\t average remove time       : %0.6f\n",
			(removeTime / (double)removeCount));
	printf("[unit]\t\t total is sane performed   : %i\n", isSaneCount);
	printf("[unit]\t\t total is sane time        : %0.6f\n", isSaneTime);
	printf("[unit]\t\t average is sane time      : %0.6f\n",
			(isSaneTime / (double)isSaneCount));

	integer_free(&integer);

	printf("[unit]\t ...PASSED\n");

	return 0;
}

static int integer_unit_test_two()
{
	int ii = 0;
	int pick = 0;
	int result = 0;
	int selection = 0;
	int searchCount = 0;
	int add_count = 0;
	int removeCount = 0;
	int isSaneCount = 0;
	double timer = 0.0;
	double searchTime = 0.0;
	double addTime = 0.0;
	double removeTime = 0.0;
	double isSaneTime = 0.0;

	IntegerArray integer;

	printf("[unit]\t integer unit test #2...\n");

	system_pickRandomSeed();

	integer_init(&integer);

	for(ii = 0; ii < 16; ii++) {
		// select a random number not already in the integer system

		pick = lrand();
		result = integer_contains(&integer, pick);
		while(result >= 0) {
			pick = lrand();
			result = integer_contains(&integer, pick);
		}

		// add the random number to the integer system

		if(integer_put(&integer, pick) < 0) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}
	}

	for(ii = 0; ii < TEST_TWO_LENGTH; ii++) {
		if((ii > 0) && ((ii % 1024) == 0)) {
			fprintf(stderr, "[unit]\t ...%i of %i (%0.2f %%)\n", ii,
					TEST_TWO_LENGTH,
					(((double)ii / (double)TEST_TWO_LENGTH) * 100.0));
			system_pickRandomSeed();
		}

		selection = (rand() % 3);

		if(selection == 0) {
			// select a random number not already in the integer system

			pick = lrand();

			timer = time_getTimeMus();

			result = integer_contains(&integer, pick);

			searchTime += time_getElapsedMusInSeconds(timer);
			searchCount++;

			while(result >= 0) {
				pick = lrand();

				timer = time_getTimeMus();

				result = integer_contains(&integer, pick);

				searchTime += time_getElapsedMusInSeconds(timer);
				searchCount++;
			}

			// add the random number to the integer system

			timer = time_getTimeMus();

			if(integer_put(&integer, pick) < 0) {
				fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
						__FUNCTION__, __LINE__);
				return -1;
			}

			addTime += time_getElapsedMusInSeconds(timer);
			add_count++;
		}
		else if((selection == 1) && (integer.length > 0)) {
			// remove the number from the system at random

			pick = integer.array[(rand() % integer.length)];

			timer = time_getTimeMus();

			if(integer_remove(&integer, pick) < 0) {
				fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
						__FUNCTION__, __LINE__);
				return -1;
			}

			removeTime += time_getElapsedMusInSeconds(timer);
			removeCount++;
		}
		else if(selection == 2) {
			// check for sanity in the system at random

			timer = time_getTimeMus();

			if(!integer_isSane(&integer)) {
				fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
						__FUNCTION__, __LINE__);
				display_array(integer.array, integer.length);
				return -1;
			}

			isSaneTime += time_getElapsedMusInSeconds(timer);
			isSaneCount++;
		}
	}

	if(!integer_isSane(&integer)) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		display_array(integer.array, integer.length);
		return -1;
	}

	printf("[unit]\t\t current array length      : %i\n", integer.length);
	printf("[unit]\t\t total searches performed  : %i\n", searchCount);
	printf("[unit]\t\t total search time         : %0.6f\n", searchTime);
	printf("[unit]\t\t average search time       : %0.6f\n",
			(searchTime / (double)searchCount));
	printf("[unit]\t\t total adds performed      : %i\n", add_count);
	printf("[unit]\t\t total add time            : %0.6f\n", addTime);
	printf("[unit]\t\t average add time          : %0.6f\n",
			(addTime / (double)add_count));
	printf("[unit]\t\t total removes performed   : %i\n", removeCount);
	printf("[unit]\t\t total remove time         : %0.6f\n", removeTime);
	printf("[unit]\t\t average remove time       : %0.6f\n",
			(removeTime / (double)removeCount));
	printf("[unit]\t\t total is sane performed   : %i\n", isSaneCount);
	printf("[unit]\t\t total is sane time        : %0.6f\n", isSaneTime);
	printf("[unit]\t\t average is sane time      : %0.6f\n",
			(isSaneTime / (double)isSaneCount));

	integer_free(&integer);

	printf("[unit]\t ...PASSED\n");

	return 0;
}

