/*
 * container_unitTest.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * A library component for the search-engine to contain key-value attributes
 * in a highly-memory-optmized serial buffer, unit test.
 *
 * Written by Josh English.
 */

// preprocessor directives

#define _CORE_COMMON_BYPASS_STD_STREAM
#include "core/core.h"
#include "search/search.h"


// define search engine container unit test constants

#define BASIC_UNIT_TEST_LENGTH							128//8192

#define BASIC_UNIT_TEST_STRING_SEED						1024

static char *BASIC_UNIT_TEST_JSON_STRING = ""
"{\n"
"	\"messageName\": \"put\",\n"
"	\"domainKey\": \"domain02\",\n"
"	\"isImmediate\": true,\n"
"	\"payload\": {\n"
"		\"exact\": \"Alpha\",\n"
"		\"description\": \"Yet now the company that Gates and Allen founded is selling operating systems like Gillette sells razor blades. New releases of operating systems are launched as if they were Hollywood blockbusters, with celebrity endorsements, talk show appearances, and world tours. The market for them is vast enough that people worry about whether it has been monopolized by one company. Even the least technically-minded people in our society now have at least a hazy idea of what operating systems do; what is more, they have strong opinions about their relative merits. It is commonly understood, even by technically unsophisticated computer users, that if you have a piece of software that works on your Macintosh, and you move it over onto a Windows machine, it will not run. That this would, in fact, be a laughable and idiotic mistake, like nailing horseshoes to the tires of a Buick.\",\n"
"		\"myUid\": \"1001\",\n"
"		\"geocoords\": [\n"
"			{\n"
"				\"latitude\": 34.47067,\n"
"				\"longitude\": -118.986621\n"
"			},\n"
"			{\n"
"				\"latitude\": 34.47067,\n"
"				\"longitude\": -118.986621\n"
"			}\n"
"		],\n"
"		\"latitude\": 34.47067,\n"
"		\"longitude\": -118.986621,\n"
"		\"address\": \"29281 Agoura Road, Agoura Hills, CA 91301\",\n"
"		\"category\": \"French Restaurant, Brewery, Pub\",\n"
"		\"at-a-glance\": \"fish and chips � craft beer � mac and cheese � moules frites � blind ambition\",\n"
"		\"menu\": \"ladyfaceale.com\",\n"
"		\"name\": \"Ladyface Ale Companie\",\n"
"		\"review\": \"This place was amazing when it first opened. The food isn't as good, but it's still okay. The brews are always amazing! Great atmosphere too!\"\n"
"	}\n"
"}\n"
"";


// declare search engine container unit test private functions

static int container_basicUnitTest();


// main

int main()
{
	signal_registerDefault();

	system_pickRandomSeed();

	printf("[unit] Search Engine Container unit test, using Ver %s on %s.\n\n",
			ASGARD_VERSION, ASGARD_DATE);

	if(container_basicUnitTest() < 0) {
		printf("[unit]\t ...FAILED\n");
		return 1;
	}

	printf("[unit] ...PASSED\n");

	return 0;
}

// define search engine container unit test private functions

static char *generateRandomString(int seed)
{
	int ii = 0;
	int pick = 0;
	int length = 0;
	char *result = NULL;

	length = ((rand() % seed) + (rand() % (seed / 4)) + 8);
	result = (char *)malloc(sizeof(char) * (length + 1));

	for(ii = 0; ii < length; ii++) {
		pick = (int)(rand() % 27);
		if(pick < 0) {
			pick *= -1;
		}
		if(pick == 0) {
			pick = 32;
		}
		else {
			pick += 97 - 1;
		}
		result[ii] = (char)pick;
	}

	return result;
}

static char *generateRandomBinaryString(int seed, int *length)
{
	int ii = 0;
	char *result = NULL;

	*length = ((rand() % seed) + (rand() % (seed / 4)) + 8);
	result = (char *)malloc(sizeof(char) * (*length + 1));

	for(ii = 0; ii < *length; ii++) {
		result[ii] = (char)(rand() % 256);
	}

	return result;
}

static int container_basicUnitTest()
{
	aboolean hasName = afalse;
	aboolean bValue = afalse;
	aboolean bResultValue = afalse;
	int ii = 0;
	int nn = 0;
	int rc = 0;
	int iValue = 0;
	int iResultValue = 0;
	int counter = 0;
	int nameLength = 0;
	int bufferLength = 0;
	int entityLength = 0;
	int stringLength = 0;
	int stringResultLength = 0;
	alint totalSerialLength = 0;
	double dValue = 0.0;
	double dResultValue = 0.0;
	double timer = 0.0;
	double totalInitTime = 0.0;
	double totalFreeTime = 0.0;
	double totalPutTime = 0.0;
	double totalGetTime = 0.0;
	double totalIterateTime = 0.0;
	double totalSerializeTime = 0.0;
	double totalDeserializeTime = 0.0;
	char *string = NULL;
	char *stringResult = NULL;
	char *name = NULL;
	char buffer[1024];

	Json *object = NULL;
	Json *jsonObject = NULL;
	Container container;
	Container newContainer;
	Container *serialContainer = NULL;
	ContainerIterator iterator;

	printf("[unit]\t search engine container basic unit test...\n");

	system_pickRandomSeed();

	printf("[unit]\t\t executing tests...");
	fflush(stdout);

	bufferLength = snprintf(buffer, ((int)sizeof(buffer) - 1),
			"%0.2f %%    ", 0.0);

	printf("%s", buffer);
	fflush(stdout);

	for(ii = 0; ii < BASIC_UNIT_TEST_LENGTH; ii++) {
		if((ii > 0) && ((ii % 8) == 0)) {
			for(nn = 0; nn < bufferLength; nn++) {
				printf("\b");
			}

			bufferLength = snprintf(buffer, ((int)sizeof(buffer) - 1),
					"%0.2f %%    ",
					(((double)ii / (double)BASIC_UNIT_TEST_LENGTH) * 100.0));

			printf("%s", buffer);
			fflush(stdout);

			system_pickRandomSeed();
		}

		hasName = afalse;

		// initialize container

		if((rand() % 100) > 49) {
			hasName = atrue;
			name = generateRandomString((rand() % 32) + 8);

			timer = time_getTimeMus();

			if(container_initWithName(&container, name, strlen(name)) < 0) {
				fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
						__FUNCTION__, __LINE__);
				return -1;
			}

			totalInitTime += time_getElapsedMusInSeconds(timer);
		}
		else {
			timer = time_getTimeMus();

			if(container_init(&container) < 0) {
				fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
						__FUNCTION__, __LINE__);
				return -1;
			}

			totalInitTime += time_getElapsedMusInSeconds(timer);
		}

		// check container status

		if(container_isLocked(&container)) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		if(container_isReadLocked(&container)) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		if(container_lock(&container) < 0) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		if(!container_isReadLocked(&container)) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		if(container_unlock(&container) < 0) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		if(container_isLocked(&container)) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		if(container_isReadLocked(&container)) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		if(container_setUid(&container, ii) < 0) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		if(container_getUid(&container) != ii) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		if(hasName) {
			if((container_getName(&container, &nameLength) == NULL) ||
					(nameLength < 1)) {
				fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
						__FUNCTION__, __LINE__);
				return -1;
			}

			if(strcmp(name, container_getName(&container, &nameLength))) {
				fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
						__FUNCTION__, __LINE__);
				return -1;
			}

			free(name);
		}

		name = generateRandomString((rand() % 32) + 8);

		if(container_setName(&container, name, strlen(name)) < 0) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		if((container_getName(&container, &nameLength) == NULL) ||
				(nameLength < 1)) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		if(strcmp(name, container_getName(&container, &nameLength))) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		free(name);

		if(container_getAgeInSeconds(&container) <= 0.0) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		timer = time_getTimeMus();

		if(container_peg(&container) < 0) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		if(timer > container.timestamp) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		entityLength = ((rand() % 256) + 16);

		for(nn = 0; nn < entityLength; nn++) {
			// boolean

			name = generateRandomString((rand() % 32) + 8);

			if((rand() % 100) > 49) {
				bValue = atrue;
			}
			else {
				bValue = afalse;
			}

			timer = time_getTimeMus();

			if(container_putBoolean(&container, name, strlen(name),
						bValue) < 0) {
				fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
						__FUNCTION__, __LINE__);
				return -1;
			}

			totalPutTime += time_getElapsedMusInSeconds(timer);
			timer = time_getTimeMus();

			bResultValue = container_getBoolean(&container, name, strlen(name));

			totalGetTime += time_getElapsedMusInSeconds(timer);

			if(bValue != bResultValue) {
				fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
						__FUNCTION__, __LINE__);
				return -1;
			}

			free(name);

			// integer

			name = generateRandomString((rand() % 32) + 8);

			iValue = (rand() + rand());

			timer = time_getTimeMus();

			if(container_putInteger(&container, name, strlen(name),
						iValue) < 0) {
				fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
						__FUNCTION__, __LINE__);
				return -1;
			}

			totalPutTime += time_getElapsedMusInSeconds(timer);
			timer = time_getTimeMus();

			iResultValue = container_getInteger(&container, name, strlen(name));

			totalGetTime += time_getElapsedMusInSeconds(timer);

			if(iValue != iResultValue) {
				fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
						__FUNCTION__, __LINE__);
				return -1;
			}

			free(name);

			// double

			name = generateRandomString((rand() % 32) + 8);

			dValue = ((double)(rand() + rand()) +
					((double)rand() / (double)rand()));

			timer = time_getTimeMus();

			if(container_putDouble(&container, name, strlen(name),
						dValue) < 0) {
				fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
						__FUNCTION__, __LINE__);
				return -1;
			}

			totalPutTime += time_getElapsedMusInSeconds(timer);
			timer = time_getTimeMus();

			dResultValue = container_getDouble(&container, name, strlen(name));

			totalGetTime += time_getElapsedMusInSeconds(timer);

			if(dValue != dResultValue) {
				fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
						__FUNCTION__, __LINE__);
				return -1;
			}

			free(name);

			// string

			name = generateRandomString((rand() % 32) + 8);

			string = generateRandomString(BASIC_UNIT_TEST_STRING_SEED);
			stringLength = strlen(string);

			timer = time_getTimeMus();

			if(container_putString(&container, name, strlen(name), string,
						stringLength) < 0) {
				fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
						__FUNCTION__, __LINE__);
				return -1;
			}

			totalPutTime += time_getElapsedMusInSeconds(timer);
			timer = time_getTimeMus();

			stringResult = container_getString(&container, name, strlen(name),
					&stringResultLength);

			totalGetTime += time_getElapsedMusInSeconds(timer);

			if((stringResult == NULL) ||
					(stringLength != stringResultLength) ||
					(strcmp(string, stringResult))) {
				fprintf(stderr, "[%s():%i] error - failed here "
						"{ '%s' vs '%s', length %i vs %i }, "
						"aborting.\n",
						__FUNCTION__, __LINE__,
						string, stringResult, stringLength, stringResultLength);
				return -1;
			}

			free(name);
			free(string);
			free(stringResult);
		}

		counter += (entityLength * 4);

		// iteration

		timer = time_getTimeMus();

		if(containerIterator_init(&container, &iterator) < 0) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		nn = 0;

		do {
			rc = containerIterator_next(&iterator);

			if(rc == CONTAINER_ITERATOR_OK) {
				nn++;
			}
		} while(rc == CONTAINER_ITERATOR_OK);

		if(rc != CONTAINER_ITERATOR_END_OF_LIST) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		if(nn != (entityLength * 4)) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		if(containerIterator_free(&iterator) < 0) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		totalIterateTime += time_getElapsedMusInSeconds(timer);

		// serialization

		timer = time_getTimeMus();

		string = container_serialize(&container, &stringLength);

		totalSerializeTime += time_getElapsedMusInSeconds(timer);

		if((string == NULL) || (stringLength < 1)) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		totalSerialLength += (alint)stringLength;

		// deserialization

		timer = time_getTimeMus();

		serialContainer = container_deserialize(string, stringLength);

		totalDeserializeTime += time_getElapsedMusInSeconds(timer);

		if(serialContainer == NULL) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		timer = time_getTimeMus();

		if(container_freePtr(serialContainer) < 0) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		totalFreeTime += time_getElapsedMusInSeconds(timer);

		// randomly corrupt the serialized container & deserialize

		for(nn = 0; nn < stringLength; nn++) {
			if((rand() % 100) > 49) {
				string[nn] = (char)(rand() % 256);
			}
		}

		serialContainer = container_deserialize(string, stringLength);
		if(serialContainer != NULL) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		free(string);

		// attempt to decode random string as a container

		string = generateRandomBinaryString(8192, &stringLength);

		serialContainer = container_deserialize(string, stringLength);
		if(serialContainer != NULL) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		free(string);

		// cleanup

		timer = time_getTimeMus();

		if(container_free(&container) < 0) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		totalFreeTime += time_getElapsedMusInSeconds(timer);
	}

	printf("\n");

	printf("[unit]\t\t average init time        : %0.6f\n",
			(totalInitTime / (double)BASIC_UNIT_TEST_LENGTH));
	printf("[unit]\t\t average free time        : %0.6f\n",
			(totalFreeTime / (double)(BASIC_UNIT_TEST_LENGTH * 2)));
	printf("[unit]\t\t average iteration time   : %0.6f (per container)\n",
			(totalIterateTime / (double)BASIC_UNIT_TEST_LENGTH));
	printf("[unit]\t\t average iteration time   : %0.9f (per entity)\n",
			(totalIterateTime / (double)counter));
	printf("[unit]\t\t average serialize time   : %0.6f\n",
			(totalSerializeTime / (double)BASIC_UNIT_TEST_LENGTH));
	printf("[unit]\t\t average serial length    : %lli\n",
			(totalSerialLength / (alint)BASIC_UNIT_TEST_LENGTH));
	printf("[unit]\t\t average deserialize time : %0.6f\n",
			(totalDeserializeTime / (double)BASIC_UNIT_TEST_LENGTH));
	printf("[unit]\t\t average put time         : %0.6f\n",
			(totalPutTime / (double)counter));
	printf("[unit]\t\t average get time         : %0.6f\n",
			(totalGetTime / (double)counter));

	// iteration tests

	printf("[unit]\t\t testing iterators...\n");

	for(ii = 0; ii < (BASIC_UNIT_TEST_LENGTH / 8); ii++) {
		// initialize container

		if((rand() % 100) > 49) {
			hasName = atrue;
			name = generateRandomString((rand() % 32) + 8);

			if(container_initWithName(&container, name, strlen(name)) < 0) {
				fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
						__FUNCTION__, __LINE__);
				return -1;
			}

			free(name);
		}
		else {
			if(container_init(&container) < 0) {
				fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
						__FUNCTION__, __LINE__);
				return -1;
			}
		}

		// boolean

		if((rand() % 100) > 49) {
			bValue = atrue;
		}
		else {
			bValue = afalse;
		}

		if(container_putBoolean(&container, "bValue", 6, bValue) < 0) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		bResultValue = container_getBoolean(&container, "bValue", 6);

		if(bValue != bResultValue) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		// integer

		iValue = (rand() + rand());

		if(container_putInteger(&container, "iValue", 6, iValue) < 0) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		iResultValue = container_getInteger(&container, "iValue", 6);

		if(iValue != iResultValue) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		// double

		dValue = ((double)(rand() + rand()) +
				((double)rand() / (double)rand()));

		if(container_putDouble(&container, "dValue", 6, dValue) < 0) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		dResultValue = container_getDouble(&container, "dValue", 6);

		if(dValue != dResultValue) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		// string

		string = generateRandomString(BASIC_UNIT_TEST_STRING_SEED);
		stringLength = strlen(string);

		if(container_putString(&container, "sValue", 6, string,
					stringLength) < 0) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		stringResult = container_getString(&container, "sValue", 6,
				&stringResultLength);

		if((stringResult == NULL) ||
				(stringLength != stringResultLength) ||
				(strcmp(string, stringResult))) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		free(stringResult);

		// iteration

		if(containerIterator_init(&container, &iterator) < 0) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		if((rc = containerIterator_next(&iterator)) !=
				CONTAINER_ITERATOR_OK) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		nn = 1;

		do {
			switch(iterator.entity.type) {
				case CONTAINER_TYPE_BOOLEAN:
					if(iterator.entity.bValue != bValue) {
						fprintf(stderr, "[%s():%i] error - failed here, "
								"aborting.\n", __FUNCTION__, __LINE__);
						return -1;
					}
					break;

				case CONTAINER_TYPE_INTEGER:
					if(iterator.entity.iValue != iValue) {
						fprintf(stderr, "[%s():%i] error - failed here, "
								"aborting.\n", __FUNCTION__, __LINE__);
						return -1;
					}
					break;

				case CONTAINER_TYPE_DOUBLE:
					if(iterator.entity.dValue != dValue) {
						fprintf(stderr, "[%s():%i] error - failed here, "
								"aborting.\n", __FUNCTION__, __LINE__);
						return -1;
					}
					break;

				case CONTAINER_TYPE_STRING:
					if(strcmp(iterator.entity.sValue, string)) {
						fprintf(stderr, "[%s():%i] error - failed here, "
								"aborting.\n", __FUNCTION__, __LINE__);
						return -1;
					}
					break;

				case CONTAINER_TYPE_END_OF_LIST:
				default:
					fprintf(stderr, "[%s():%i] error - failed here, "
							"aborting.\n", __FUNCTION__, __LINE__);
					return -1;
			}

			if((rc = containerIterator_next(&iterator)) !=
					CONTAINER_ITERATOR_OK) {
				break;
			}

			nn++;
		} while(rc == CONTAINER_ITERATOR_OK);

		if(rc == CONTAINER_ITERATOR_ERROR) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}
		else if((rc == CONTAINER_ITERATOR_END_OF_LIST) && (nn != 4)) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}
		else if(rc != CONTAINER_ITERATOR_END_OF_LIST) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		if(containerIterator_rewind(&iterator) < 0) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		if(containerIterator_free(&iterator) < 0) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}

		// cleanup

		free(string);

		if(container_free(&container) < 0) {
			fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
					__FUNCTION__, __LINE__);
			return -1;
		}
	}

	// conversion tests

	printf("[unit]\t\t testing conversions...\n");

	// initialize container

	if(container_init(&container) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	// boolean (true)

	bValue = atrue;

	if(container_putBoolean(&container, "bValue", 6, bValue) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	bResultValue = container_getBoolean(&container, "bValue", 6);

	if(bValue != bResultValue) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if((iResultValue = container_getInteger(&container, "bValue", 6)) != 1) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if((dResultValue = container_getDouble(&container, "bValue", 6)) != 1.0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	stringResult = container_getString(&container, "bValue", 6,
			&stringResultLength);

	if((stringResult == NULL) ||
			(stringResultLength != 4) ||
			(strcmp(stringResult, "true"))) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	free(stringResult);

	// boolean (false)

	bValue = afalse;

	if(container_putBoolean(&container, "b2Value", 7, bValue) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	bResultValue = container_getBoolean(&container, "b2Value", 7);

	if(bValue != bResultValue) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if((iResultValue = container_getInteger(&container, "b2Value", 7)) != 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if((dResultValue = container_getDouble(&container, "b2Value", 7)) != 0.0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	stringResult = container_getString(&container, "b2Value", 7,
			&stringResultLength);

	if((stringResult == NULL) ||
			(stringResultLength != 5) ||
			(strcmp(stringResult, "false"))) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	free(stringResult);

	// integer

	iValue = 123456;

	if(container_putInteger(&container, "iValue", 6, iValue) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	iResultValue = container_getInteger(&container, "iValue", 6);

	if(iValue != iResultValue) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if((bResultValue = container_getBoolean(&container,
					"iValue", 6)) != atrue) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if((dResultValue = container_getDouble(&container,
					"iValue", 6)) != 123456.0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	stringResult = container_getString(&container, "iValue", 6,
			&stringResultLength);

	if((stringResult == NULL) ||
			(stringResultLength != 6) ||
			(strcmp(stringResult, "123456"))) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	free(stringResult);

	// double

	dValue = 6543.9856;

	if(container_putDouble(&container, "dValue", 6, dValue) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	dResultValue = container_getDouble(&container, "dValue", 6);

	if(dValue != dResultValue) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if((bResultValue = container_getBoolean(&container,
					"dValue", 6)) != atrue) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if((iResultValue = container_getInteger(&container,
					"dValue", 6)) != 6543) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	stringResult = container_getString(&container, "dValue", 6,
			&stringResultLength);

	if((stringResult == NULL) ||
			(stringResultLength != 11) ||
			(strcmp(stringResult, "6543.985600"))) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	free(stringResult);

	// string

	string = strdup("this is a test string");
	stringLength = strlen(string);

	if(container_putString(&container, "sValue", 6, string,
				stringLength) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	stringResult = container_getString(&container, "sValue", 6,
			&stringResultLength);

	if((stringResult == NULL) ||
			(stringLength != stringResultLength) ||
			(strcmp(string, stringResult))) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	free(string);
	free(stringResult);

	if((bResultValue = container_getBoolean(&container,
					"sValue", 6)) != afalse) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if((iResultValue = container_getInteger(&container, "sValue", 6)) != 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if((dResultValue = container_getDouble(&container, "sValue", 6)) != 0.0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	// string (numeric)

	string = strdup("987654");
	stringLength = strlen(string);

	if(container_putString(&container, "s2Value", 7, string,
				stringLength) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	stringResult = container_getString(&container, "s2Value", 7,
			&stringResultLength);

	if((stringResult == NULL) ||
			(stringLength != stringResultLength) ||
			(strcmp(string, stringResult))) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	free(string);
	free(stringResult);

	if((bResultValue = container_getBoolean(&container,
					"s2Value", 7)) != atrue) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if((iResultValue = container_getInteger(&container,
					"s2Value", 7)) != 987654) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if((dResultValue = container_getDouble(&container,
					"s2Value", 7)) != 987654.0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	// convert to JSON object

	if((object = container_containerToJson(&container)) == NULL) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if((string = json_toString(object, &stringLength)) == NULL) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(stringLength < 1) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	printf("[unit]\t\t ----------/ begin json object /----------\n");
	printf("%s", string);
	printf("[unit]\t\t -----------/ end json object /-----------\n");

	free(string);

	// convert JSON to a container

	if((serialContainer = container_jsonToContainer(object)) == NULL) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	// re-convert to a JSON object

	json_freePtr(object);

	if((object = container_containerToJson(&container)) == NULL) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if((string = json_toString(object, &stringLength)) == NULL) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(stringLength < 1) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	printf("[unit]\t\t ----------/ begin json object /----------\n");
	printf("%s", string);
	printf("[unit]\t\t -----------/ end json object /-----------\n");

	free(string);

	// cleanup

	json_freePtr(object);

	if(container_freePtr(serialContainer) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(container_free(&container) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	// convert basic unit-test JSON object to a container

	if((object = json_newFromString(BASIC_UNIT_TEST_JSON_STRING)) == NULL) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if((jsonObject = json_getObject(object, "payload")) == NULL) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if((serialContainer = container_jsonToContainer(jsonObject)) == NULL) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	container_display(stdout, serialContainer);

	json_freePtr(object);

	if(container_freePtr(serialContainer) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	// combine two containers

	// build the "old" container

	if(container_initWithName(&container, "oldName", 7) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(container_setUid(&container, 1234) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(container_putBoolean(&container, "bValue", 6, atrue) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(container_putInteger(&container, "iValue", 6, 654321) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(container_putDouble(&container, "dValue", 6, 987.654) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	string = strdup("this is a test string");
	stringLength = strlen(string);

	if(container_putString(&container, "sValue", 6, string,
				stringLength) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	free(string);

	// build the "new" container

	if(container_initWithName(&newContainer, "newName", 7) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(container_setUid(&newContainer, 6543) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(container_putBoolean(&newContainer, "bValue2", 7, atrue) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(container_putInteger(&newContainer, "iValue", 6, 12345678) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(container_putInteger(&newContainer, "iValue2", 7, 654321) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(container_putDouble(&newContainer, "dValue2", 7, 987.654) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	string = strdup("this is a test string - new container");
	stringLength = strlen(string);

	if(container_putString(&newContainer, "sValue", 6, string,
				stringLength) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	free(string);

	string = strdup("this is a test string");
	stringLength = strlen(string);

	if(container_putString(&newContainer, "sValue2", 7, string,
				stringLength) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	free(string);

	// combine the containers

	string = NULL;

	if((serialContainer = container_combine(&container,
					&newContainer, CONTAINER_FLAG_UPDATE, &string)) != NULL) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(string == NULL) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	free(string);

	if((serialContainer = container_combine(&container,
					&newContainer, CONTAINER_FLAG_APPEND, &string)) != NULL) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(string == NULL) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	free(string);

	if((serialContainer = container_combine(&container,
					&newContainer, CONTAINER_FLAG_UPPEND, &string)) == NULL) {
		fprintf(stderr, "[%s():%i] error - failed here with '%s', aborting.\n",
				__FUNCTION__, __LINE__, string);
		return -1;
	}

	if(string != NULL) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if((bResultValue = container_getBoolean(serialContainer,
					"bValue", 6)) != atrue) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if((bResultValue = container_getBoolean(serialContainer,
					"bValue2", 7)) != atrue) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	iResultValue = container_getInteger(serialContainer, "iValue", 6);

	if(iResultValue != 12345678) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	iResultValue = container_getInteger(serialContainer, "iValue2", 7);

	if(iResultValue != 654321) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if((dResultValue = container_getDouble(serialContainer,
					"dValue", 6)) != 987.654) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if((dResultValue = container_getDouble(serialContainer,
					"dValue2", 7)) != 987.654) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	stringResult = container_getString(serialContainer, "sValue", 6,
			&stringResultLength);

	if((stringResult == NULL) ||
			(strcmp(stringResult, "this is a test string - new container"))) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	free(stringResult);

	stringResult = container_getString(serialContainer, "sValue2", 7,
			&stringResultLength);

	if((stringResult == NULL) ||
			(strcmp(stringResult, "this is a test string"))) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	free(stringResult);

	// cleanup

	container_display(stdout, serialContainer);

	if(container_free(&container) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(container_free(&newContainer) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(container_freePtr(serialContainer) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	printf("[unit]\t ...PASSED\n");

	return 0;
}

