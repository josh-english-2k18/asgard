/*
 * authentication.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * A library component for the search-engine to define the set of users who may
 * access the system, with a permission set defined by domain.
 *
 * Written by Josh English.
 */

// preprocessor directives

#define _CORE_COMMON_BYPASS_STD_STREAM

#include "core/core.h"
#include "search/search.h"
#define _SEARCH_CORE_COMPONENT
#include "search/core/authentication.h"


// declare authentication private functions

// token functions

static char *token_new(int *resultLength);

static void token_free(void *argument, void *memory);

// ip address functions

static int ipAddress_convert(char *ipAddress, int ipAddressLength,
		int address[4]);

static aboolean ipAddress_compare(int addressMaster[4], int addressClient[4]);

// user access functions

static AuthSystemUserAccess *userAccess_new(char *domainKey,
		int domainKeyLength);

static void userAccess_free(void *memory);

// user functions

static AuthSystemUser *user_new(AuthSystem *system, aboolean isRoot,
		char *firstName, char *lastName, char *emailAddress, char *username,
		char *password);

static void user_free(void *memory);

static int user_calculateMemorySize(AuthSystemUser *user);

static Json *user_toJson(AuthSystemUser *user);

static int user_jsonToUser(AuthSystem *system, Json *object);

// system functions

static Json *system_toJson(AuthSystem *system);

static int system_jsonToSystem(AuthSystem *system, Json *object);


// define authentication private functions

// token functions

static char *token_new(int *resultLength)
{
	int ii = 0;
	char *result = NULL;

	result = (char *)malloc(sizeof(char) * (AUTH_SYSTEM_TOKEN_LENGTH + 1));

	system_pickRandomSeed();

	for(ii = 0; ii < AUTH_SYSTEM_TOKEN_LENGTH; ii++) {
		switch(rand() % 3) {
			case 0: // upper case letters
				result[ii] = (char)((rand() % 26) + 65);
				break;

			case 1: // lower case letters
				result[ii] = (char)((rand() % 26) + 97);
				break;

			case 2: // numbers
				result[ii] = (char)((rand() % 10) + 48);
				break;
		}
	}

	*resultLength = ii;

	return result;
}

static void token_free(void *argument, void *memory)
{
	AuthSystemUser *user = NULL;

	if((argument == NULL) || (memory == NULL)) {
		DISPLAY_INVALID_ARGS;
	}

	user = (AuthSystemUser *)memory;

	mutex_lock(&user->mutex);

	user->tokenLength = 0;

	if(user->token != NULL) {
		free(user->token);
	}

	mutex_unlock(&user->mutex);
}

// ip address functions

static int ipAddress_convert(char *ipAddress, int ipAddressLength,
		int address[4])
{
	int ii = 0;
	int nn = 0;
	int iValue = 0;
	int quadCounter = 0;
	char buffer[8];

	address[0] = AUTHENTICATION_IP_ADDRESS_CONTROL_INIT;
	address[1] = AUTHENTICATION_IP_ADDRESS_CONTROL_INIT;
	address[2] = AUTHENTICATION_IP_ADDRESS_CONTROL_INIT;
	address[3] = AUTHENTICATION_IP_ADDRESS_CONTROL_INIT;

	if(ipAddressLength < 1) {
		return -1;
	}

	for(ii = 0, nn = 0; ii < ipAddressLength; ii++) {
		if(ipAddress[ii] == '*') {
			if(nn > 0) {
				return -1;
			}

			if(quadCounter > 3) {
				return -1;
			}

			address[quadCounter] = AUTHENTICATION_IP_ADDRESS_CONTROL_WILDCARD;

			quadCounter++;

			if(((ii + 1) < ipAddressLength) && (ipAddress[(ii + 1)] != '.')) {
				return -1;
			}

			ii++;
		}
		else if(ipAddress[ii] == '.') {
			buffer[nn] = '\0';

			iValue = atoi(buffer);

			if((iValue < 0) || (iValue > 255)) {
				return -1;
			}

			if(quadCounter > 3) {
				return -1;
			}

			address[quadCounter] = iValue;

			quadCounter++;

			nn = 0;
		}
		else if(ctype_isNumeric(ipAddress[ii])) {
			buffer[nn] = ipAddress[ii];
			nn++;
			if(nn > 3) {
				return -1;
			}
		}
		else {
			return -1;
		}
	}

	if(quadCounter < 1) {
		return -1;
	}

	if((nn > 0) && (quadCounter < 4)) {
		buffer[nn] = '\0';

		iValue = atoi(buffer);

		if((iValue < 0) || (iValue > 255)) {
			return -1;
		}

		if(quadCounter > 3) {
			return -1;
		}

		address[quadCounter] = iValue;

		quadCounter++;

		nn = 0;
	}

	if((quadCounter < 4) &&
			(address[(quadCounter - 1)] !=
			 AUTHENTICATION_IP_ADDRESS_CONTROL_WILDCARD)) {
		return -1;
	}

	for(ii = quadCounter; ii < 4; ii++) {
		address[ii] = AUTHENTICATION_IP_ADDRESS_CONTROL_WILDCARD;
	}

	return 0;
}

static aboolean ipAddress_compare(int addressMaster[4], int addressClient[4])
{
	int ii = 0;

	for(ii = 0; ii < 4; ii++) {
		if(addressMaster[ii] == AUTHENTICATION_IP_ADDRESS_CONTROL_WILDCARD) {
			continue;
		}

		if(addressClient[ii] == AUTHENTICATION_IP_ADDRESS_CONTROL_WILDCARD) {
			return afalse;
		}

		if(addressMaster[ii] != addressClient[ii]) {
			return afalse;
		}
	}

	return atrue;
}

// user access functions

static AuthSystemUserAccess *userAccess_new(char *domainKey,
		int domainKeyLength)
{
	int ii = 0;

	AuthSystemUserAccess *result = NULL;

	result = (AuthSystemUserAccess *)malloc(sizeof(AuthSystemUserAccess));

	for(ii = 0; ii < AUTHENTICATION_PERMISSIONS_LIST_LENGTH; ii++) {
		result->permissions[ii] = afalse;
	}

	result->domainKeyLength = domainKeyLength;
	result->domainKey = strndup(domainKey, domainKeyLength);

	return result;
}

static void userAccess_free(void *memory)
{
	int ii = 0;

	AuthSystemUserAccess *access = NULL;

	if(memory == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	access = (AuthSystemUserAccess *)memory;

	if(access->domainKey != NULL) {
		free(access->domainKey);
	}

	if(access->controls != NULL) {
		for(ii = 0; ii < access->accessControlLength; ii++) {
			if(access->controls[ii].ipAddress != NULL) {
				free(access->controls[ii].ipAddress);
			}
		}

		free(access->controls);
	}

	memset(access, 0, (int)(sizeof(AuthSystemUserAccess)));

	free(access);
}

// user functions

static AuthSystemUser *user_new(AuthSystem *system, aboolean isRoot,
		char *firstName, char *lastName, char *emailAddress, char *username,
		char *password)
{
	Sha1 hash;
	AuthSystemUser *result = NULL;

	result = (AuthSystemUser *)malloc(sizeof(AuthSystemUser));

	result->isRoot = isRoot;

	result->uid = system->uidCounter;

	system->uidCounter += 1;

	if(firstName != NULL) {
		result->firstNameLength = strlen(firstName);
		result->firstName = strndup(firstName, result->firstNameLength);
	}
	else {
		result->firstNameLength = 7;
		result->firstName = strndup("unknown", result->firstNameLength);
	}

	if(lastName != NULL) {
		result->lastNameLength = strlen(lastName);
		result->lastName = strndup(lastName, result->lastNameLength);
	}
	else {
		result->lastNameLength = 7;
		result->lastName = strndup("unknown", result->lastNameLength);
	}

	if(emailAddress != NULL) {
		result->emailAddressLength = strlen(emailAddress);
		result->emailAddress = strndup(emailAddress,
				result->emailAddressLength);
	}
	else {
		result->emailAddressLength = 7;
		result->emailAddress = strndup("unknown", result->emailAddressLength);
	}

	result->usernameLength = strlen(username);
	result->username = strndup(username, result->usernameLength);

	sha1_init(&hash);
	sha1_hashData(&hash, (unsigned char *)password, strlen(password));
	sha1_calculateHash(&hash);
	sha1_constructSignature(&hash, &result->password, &result->passwordLength);

	bptree_init(&result->accessIndex);
	bptree_setFreeFunction(&result->accessIndex, userAccess_free);

	mutex_init(&result->mutex);

	return result;
}

static void user_free(void *memory)
{
	int ii = 0;

	AuthSystemUser *user = NULL;

	if(memory == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	user = (AuthSystemUser *)memory;

	mutex_lock(&user->mutex);

	if(user->firstName != NULL) {
		free(user->firstName);
	}

	if(user->lastName != NULL) {
		free(user->lastName);
	}

	if(user->emailAddress != NULL) {
		free(user->emailAddress);
	}

	if(user->username != NULL) {
		free(user->username);
	}

	if(user->password != NULL) {
		free(user->password);
	}

	if(user->access.domainKey != NULL) {
		free(user->access.domainKey);
	}

	if(user->access.controls != NULL) {
		for(ii = 0; ii < user->access.accessControlLength; ii++) {
			if(user->access.controls[ii].ipAddress != NULL) {
				free(user->access.controls[ii].ipAddress);
			}
		}

		free(user->access.controls);
	}

	bptree_free(&user->accessIndex);

	mutex_unlock(&user->mutex);
	mutex_free(&user->mutex);

	memset(user, 0, (int)(sizeof(AuthSystemUser)));

	free(user);
}

static int user_calculateMemorySize(AuthSystemUser *user)
{
	int result = 0;
	int leafCount = 0;

	result = (int)sizeof(AuthSystemUser);

	mutex_lock(&user->mutex);

	if(user->firstName != NULL) {
		result += user->firstNameLength;
	}

	if(user->lastName != NULL) {
		result += user->lastNameLength;
	}

	if(user->emailAddress != NULL) {
		result += user->emailAddressLength;
	}

	if(user->username != NULL) {
		result += user->usernameLength;
	}

	if(user->password != NULL) {
		result += user->passwordLength;
	}

	result += user->tokenLength;

	bptree_getLeafCount(&user->accessIndex, &leafCount);

	mutex_unlock(&user->mutex);

	result += (leafCount * (int)sizeof(AuthSystemUserAccess));

	return result;
}

static Json *user_toJson(AuthSystemUser *user)
{
	int ii = 0;
	int keyLength = 0;
	int permissionCount = 0;
	char *key = NULL;

	Json *result = NULL;
	Json *userAccess = NULL;
	AuthSystemUserAccess *access = NULL;

	result = json_new();

	mutex_lock(&user->mutex);

	json_addBoolean(result, "isRoot", user->isRoot);
	json_addNumber(result, "uid", (double)user->uid);

	if(user->firstName != NULL) {
		json_addString(result, "firstName", user->firstName);
	}

	if(user->lastName != NULL) {
		json_addString(result, "lastName", user->lastName);
	}

	if(user->emailAddress != NULL) {
		json_addString(result, "emailAddress", user->emailAddress);
	}

	if(user->username != NULL) {
		json_addString(result, "username", user->username);
	}

	if(user->password != NULL) {
		json_addString(result, "password", user->password);
	}

	userAccess = json_new();

	if(user->access.accessControlLength > 0) {
		json_addArray(userAccess, "access");
	}

	for(ii = 0; ii < user->access.accessControlLength; ii++) {
		json_addStringToArray(userAccess, "access",
				user->access.controls[ii].ipAddress);
	}

	json_addArray(userAccess, "permissions");

	for(ii = 0; ii < AUTHENTICATION_PERMISSIONS_LIST_LENGTH; ii++) {
		json_addBooleanToArray(userAccess, "permissions",
				user->access.permissions[ii]);
	}

	json_addObject(result, "basePermissions", userAccess);

	bptree_getLeafCount(&user->accessIndex, &permissionCount);

	if(permissionCount < 1) {
		mutex_unlock(&user->mutex);

		return result;
	}

	json_addArray(result, "permissions");

	keyLength = 1;
	key = (char *)malloc(sizeof(char) * (keyLength + 1));

	memset(key, 0, keyLength);

	while((bptree_getNext(&user->accessIndex, key, keyLength,
					((void *)&access)) == 0) &&
			(access != NULL)) {
		free(key);

		userAccess = json_new();

		json_addString(userAccess, "domainKey", access->domainKey);

		if(access->accessControlLength > 0) {
			json_addArray(userAccess, "access");
		}

		for(ii = 0; ii < access->accessControlLength; ii++) {
			json_addStringToArray(userAccess, "access",
					access->controls[ii].ipAddress);
		}

		json_addArray(userAccess, "permissions");

		for(ii = 0; ii < AUTHENTICATION_PERMISSIONS_LIST_LENGTH; ii++) {
			json_addBooleanToArray(userAccess, "permissions",
					access->permissions[ii]);
		}

		json_addObjectToArray(result, "permissions", userAccess);

		key = strndup(access->domainKey, access->domainKeyLength);
		keyLength = access->domainKeyLength;
	}

	free(key);

	mutex_unlock(&user->mutex);

	return result;
}

static int user_jsonToUser(AuthSystem *system, Json *object)
{
	int ii = 0;
	int nn = 0;
	int length = 0;
	int permissionsLength = 0;

	Json *userAccess = NULL;
	AuthSystemUser *user = NULL;
	AuthSystemUserAccess *access = NULL;

	if(authSystem_newUser(system,
				json_getString(object, "username"),
				json_getString(object, "password"))) {
		return -1;
	}

	if((user = authSystem_userGet(system,
					json_getString(object, "username"))) == NULL) {
		return -1;
	}

	authSystem_userSetIsRoot(user, json_getBoolean(object, "isRoot"));

	mutex_lock(&user->mutex);

	user->uid = (int)json_getNumber(object, "uid");

	mutex_unlock(&user->mutex);

	if(json_elementExists(object, "firstName")) {
		authSystem_userSetFirstName(user,
				json_getString(object, "firstName"));
	}

	if(json_elementExists(object, "lastName")) {
		authSystem_userSetLastName(user,
				json_getString(object, "lastName"));
	}

	if(json_elementExists(object, "emailAddress")) {
		authSystem_userSetEmailAddress(user,
				json_getString(object, "emailAddress"));
	}

	mutex_lock(&user->mutex);

	if(user->password != NULL) {
		free(user->password);
	}

	user->password = strdup(json_getString(object, "password"));

	mutex_unlock(&user->mutex);

	if((userAccess = json_getObject(object, "basePermissions")) != NULL) {
		if(json_elementExists(userAccess, "access")) {
			length = json_getArrayLength(userAccess, "access");

			for(nn = 0; nn < length; nn++) {
				authSystem_accessAddIpAddress(&user->access,
						json_getStringFromArray(userAccess, "access", nn));
			}
		}

		if(json_elementExists(userAccess, "permissions")) {
			length = json_getArrayLength(userAccess, "permissions");

			for(nn = 0; nn < length; nn++) {
				authSystem_accessSetPermission(&user->access, nn,
						json_getBooleanFromArray(userAccess, "permissions",
							nn));
			}
		}
	}

	permissionsLength = json_getArrayLength(object, "permissions");

	for(ii = 0; ii < permissionsLength; ii++) {
		if((userAccess = json_getObjectFromArray(object, "permissions",
						ii)) == NULL) {
			continue;
		}

		if(authSystem_userNewAccess(user,
					json_getString(userAccess, "domainKey"),
					strlen(json_getString(userAccess, "domainKey"))) < 0) {
			continue;
		}

		if((access = authSystem_userGetAccess(user,
						json_getString(userAccess, "domainKey"))) == NULL) {
			continue;
		}

		if(json_elementExists(userAccess, "access")) {
			length = json_getArrayLength(userAccess, "access");

			for(nn = 0; nn < length; nn++) {
				authSystem_accessAddIpAddress(access,
						json_getStringFromArray(userAccess, "access", nn));
			}
		}

		if(json_elementExists(userAccess, "permissions")) {
			length = json_getArrayLength(userAccess, "permissions");

			for(nn = 0; nn < length; nn++) {
				authSystem_accessSetPermission(access, nn,
						json_getBooleanFromArray(userAccess, "permissions",
							nn));
			}
		}
	}

	return 0;
}

// system functions

static Json *system_toJson(AuthSystem *system)
{
	int keyLength = 0;
	int userCount = 0;
	char *key = NULL;

	Json *result = NULL;
	Json *userObject = NULL;
	AuthSystemUser *user = NULL;

	result = json_new();

	json_addBoolean(result, "isEnabled", system->isEnabled);
	json_addNumber(result, "uidCounter", (double)system->uidCounter);

	bptree_getLeafCount(&system->users, &userCount);

	if(userCount < 1) {
		return result;
	}

	json_addArray(result, "users");

	keyLength = 1;
	key = (char *)malloc(sizeof(char) * (keyLength + 1));

	memset(key, 0, keyLength);

	while((bptree_getNext(&system->users, key, keyLength,
					((void *)&user)) == 0) &&
			(user != NULL)) {
		free(key);

		userObject = user_toJson(user);

		json_addObjectToArray(result, "users", userObject);

		keyLength = user->usernameLength;
		key = strndup(user->username, keyLength);
	}

	free(key);

	return result;
}

static int system_jsonToSystem(AuthSystem *system, Json *object)
{
	int ii = 0;
	int length = 0;

	Json *user = NULL;

	system->isEnabled = json_getBoolean(object, "isEnabled");

	system->uidCounter = (int)json_getNumber(object, "uidCounter");

	if(json_elementExists(object, "users")) {
		length = json_getArrayLength(object, "users");
	}

	for(ii = 0; ii < length; ii++) {
		user = json_getObjectFromArray(object, "users", ii);

		if(user_jsonToUser(system, user) < 0) {
			return -1;
		}
	}

	return 0;
}


// define authentication public functions

int authSystem_init(AuthSystem *system, Log *log)
{
	if(system == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	memset(system, 0, (int)(sizeof(AuthSystem)));

	system->isEnabled = atrue;
	system->uidCounter = 1;

	if(bptree_init(&system->users) < 0) {
		return -1;
	}

	if(bptree_setFreeFunction(&system->users, user_free) < 0) {
		return -1;
	}

	if(managedIndex_init(&system->tokens, atrue) < 0) {
		return -1;
	}

	if(managedIndex_setFreeFunction(&system->tokens, &system,
				token_free) < 0) {
		return -1;
	}

	if(log == NULL) {
		system->isLogInternallyAllocated = atrue;
		system->log = (Log *)malloc(sizeof(Log));

		log_init(system->log, LOG_OUTPUT_STDOUT, NULL, LOG_LEVEL_DEBUG);
	}
	else {
		system->isLogInternallyAllocated = afalse;
		system->log = log;
	}

	if(spinlock_init(&system->lock) < 0) {
		return -1;
	}

	log_logf(system->log, LOG_LEVEL_INFO,
			"authentication system initialized");

	return 0;
}

AuthSystem *authSystem_new(Log *log)
{
	AuthSystem *result = NULL;

	result = (AuthSystem *)malloc(sizeof(AuthSystem));

	if(authSystem_init(result, log) < 0) {
		authSystem_freePtr(result);
		return NULL;
	}

	return result;
}

int authSystem_free(AuthSystem *system)
{
	if(system == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(spinlock_writeLock(&system->lock) < 0) {
		return -1;
	}

	if(managedIndex_free(&system->tokens) < 0) {
		spinlock_writeUnlock(&system->lock);
		return -1;
	}

	if(bptree_free(&system->users) < 0) {
		spinlock_writeUnlock(&system->lock);
		return -1;
	}

	log_logf(system->log, LOG_LEVEL_INFO,
			"authentication system shut down");

	if((system->isLogInternallyAllocated) && (system->log != NULL)) {
		log_free(system->log);
		free(system->log);
	}

	spinlock_writeUnlock(&system->lock);

	if(spinlock_free(&system->lock) < 0) {
		return -1;
	}

	memset(system, 0, (int)(sizeof(AuthSystem)));

	return 0;
}

int authSystem_freePtr(AuthSystem *system)
{
	if(system == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(authSystem_free(system) < 0) {
		return -1;
	}

	free(system);

	return 0;
}

char *authSystem_permissionsToString(AuthSystemPermissions permission)
{
	char *result = NULL;

	switch(permission) {
		case AUTHENTICATION_PERMISSIONS_SETTINGS:
			result = "settings";
			break;

		case AUTHENTICATION_PERMISSIONS_DOMAIN:
			result = "domain";
			break;

		case AUTHENTICATION_PERMISSIONS_INDEX:
			result = "index";
			break;

		case AUTHENTICATION_PERMISSIONS_GET:
			result = "get";
			break;

		case AUTHENTICATION_PERMISSIONS_PUT:
			result = "put";
			break;

		case AUTHENTICATION_PERMISSIONS_UPDATE:
			result = "update";
			break;

		case AUTHENTICATION_PERMISSIONS_DELETE:
			result = "delete";
			break;

		case AUTHENTICATION_PERMISSIONS_SEARCH:
			result = "search";
			break;

		case AUTHENTICATION_PERMISSIONS_LIST:
			result = "list";
			break;

		case AUTHENTICATION_PERMISSIONS_COMMAND_DOMAIN:
			result = "command (domain)";
			break;

		case AUTHENTICATION_PERMISSIONS_COMMAND_INDEX:
			result = "command (index)";
			break;

		case AUTHENTICATION_PERMISSIONS_COMMAND_SERVER:
			result = "command (server)";
			break;

		case AUTHENTICATION_PERMISSIONS_LIST_LENGTH:
		case AUTHENTICATION_PERMISSIONS_END_OF_LIST:
		default:
			result = "Unknown";
	}

	return result;
}

AuthSystemPermissions authSystem_stringToPermission(char *string)
{
	AuthSystemPermissions result = AUTHENTICATION_PERMISSIONS_END_OF_LIST;

	if(string == NULL) {
		DISPLAY_INVALID_ARGS;
		return INDEX_REGISTRY_TYPE_END_OF_LIST;
	}

	if(!strcasecmp(string, "settings")) {
		result = AUTHENTICATION_PERMISSIONS_SETTINGS;
	}
	else if(!strcasecmp(string, "domain")) {
		result = AUTHENTICATION_PERMISSIONS_DOMAIN;
	}
	else if(!strcasecmp(string, "index")) {
		result = AUTHENTICATION_PERMISSIONS_INDEX;
	}
	else if(!strcasecmp(string, "get")) {
		result = AUTHENTICATION_PERMISSIONS_GET;
	}
	else if(!strcasecmp(string, "put")) {
		result = AUTHENTICATION_PERMISSIONS_PUT;
	}
	else if(!strcasecmp(string, "update")) {
		result = AUTHENTICATION_PERMISSIONS_UPDATE;
	}
	else if(!strcasecmp(string, "delete")) {
		result = AUTHENTICATION_PERMISSIONS_DELETE;
	}
	else if(!strcasecmp(string, "search")) {
		result = AUTHENTICATION_PERMISSIONS_SEARCH;
	}
	else if(!strcasecmp(string, "list")) {
		result = AUTHENTICATION_PERMISSIONS_LIST;
	}
	else if(!strcasecmp(string, "command (domain)")) {
		result = AUTHENTICATION_PERMISSIONS_COMMAND_DOMAIN;
	}
	else if(!strcasecmp(string, "command (index)")) {
		result = AUTHENTICATION_PERMISSIONS_COMMAND_INDEX;
	}
	else if(!strcasecmp(string, "command (server)")) {
		result = AUTHENTICATION_PERMISSIONS_COMMAND_SERVER;
	}

	return result;
}

aboolean authSystem_isLocked(AuthSystem *system)
{
	if(system == NULL) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	return spinlock_isLocked(&system->lock);
}

aboolean authSystem_isReadLocked(AuthSystem *system)
{
	int threadCount = 0;

	if(system == NULL) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	if(spinlock_getSpinThreadCount(&system->lock, &threadCount) < 0) {
		return afalse;
	}

	if(threadCount > 0) {
		return atrue;
	}

	return afalse;
}

int authSystem_lock(AuthSystem *system)
{
	if(system == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return spinlock_readLock(&system->lock);
}

int authSystem_unlock(AuthSystem *system)
{
	if(system == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return spinlock_readUnlock(&system->lock);
}

int authSystem_setIsEnabled(AuthSystem *system, aboolean isEnabled)
{
	if(system == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(spinlock_writeLock(&system->lock) < 0) {
		return -1;
	}

	system->isEnabled = isEnabled;

	spinlock_writeUnlock(&system->lock);

	if(system->isEnabled) {
		log_logf(system->log, LOG_LEVEL_INFO,
				"authentication system is enabled");
	}
	else {
		log_logf(system->log, LOG_LEVEL_INFO,
				"authentication system is disabled");
	}

	return 0;
}

aboolean authSystem_isEnabled(AuthSystem *system)
{
	if(system == NULL) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	return system->isEnabled;
}

Json *authSystem_toJson(AuthSystem *system)
{
	Json *result = NULL;

	if(system == NULL) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	if(spinlock_readLock(&system->lock) < 0) {
		return NULL;
	}

	result = system_toJson(system);

	spinlock_readUnlock(&system->lock);

	return result;
}

int authSystem_jsonToSystem(AuthSystem *system, Json *object)
{
	if((system == NULL) || (object == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	return system_jsonToSystem(system, object);
}

// user index functions

int authSystem_newUser(AuthSystem *system, char *username, char *password)
{
	int counter = 0;
	int keyLength = 0;
	char *key = NULL;

	AuthSystemUser *user = NULL;
	AuthSystemUser *userRecord = NULL;

	if((system == NULL) || (username == NULL) || (password == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	key = username;
	keyLength = strlen(key);

	if(spinlock_writeLock(&system->lock) < 0) {
		return -1;
	}

	if(bptree_get(&system->users, key, keyLength, ((void *)&user)) == 0) {
		spinlock_writeUnlock(&system->lock);

		log_logf(system->log, LOG_LEVEL_ERROR,
				"{AUTH} unable to create new user '%s' - alread exists",
				username);

		return -1;
	}

	user = user_new(system,
			afalse,		// is root
			NULL,		// first name
			NULL,		// last name
			NULL,		// email address
			username,
			password);

	if(bptree_put(&system->users, key, keyLength, (void *)user) < 0) {
		spinlock_writeUnlock(&system->lock);

		log_logf(system->log, LOG_LEVEL_ERROR,
				"{AUTH} unable to create new user '%s' "
				"- failed to insert into index",
				username);

		user_free(user);
		return -1;
	}

	do {
		key = token_new(&keyLength);

		counter++;

		if(managedIndex_get(&system->tokens, key, keyLength,
					((void *)&userRecord)) == 0) {
			free(key);
			continue;
		}

		user->tokenLength = keyLength;
		user->token = strndup(key, keyLength);

		if(managedIndex_put(&system->tokens, key, keyLength,
					(void *)user, user_calculateMemorySize(user)) < 0) {
			spinlock_writeUnlock(&system->lock);

			log_logf(system->log, LOG_LEVEL_ERROR,
					"{AUTH} unable to create new user '%s' "
					"- failed to insert '%s' into token index",
					username, key);

			free(key);
			return -1;
		}
		else {
			log_logf(system->log, LOG_LEVEL_DEBUG,
					"{AUTH} added user '%s' token '%s' to token index "
					"(%i iterations)",
					username, key, counter);

			free(key);
			break;
		}
	} while(atrue);

	spinlock_writeUnlock(&system->lock);

	log_logf(system->log, LOG_LEVEL_INFO,
			"{AUTH} created new user '%s' with token '%s'",
			user->username, user->token);

	return 0;
}

AuthSystemUser *authSystem_userGet(AuthSystem *system, char *username)
{
	int usernameLength = 0;

	AuthSystemUser *result = NULL;

	if((system == NULL) || (username == NULL) ||
			((usernameLength = strlen(username)) < 1)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	if(spinlock_readLock(&system->lock) < 0) {
		return NULL;
	}

	bptree_get(&system->users, username, usernameLength, ((void *)&result));

	spinlock_readUnlock(&system->lock);

	if(result == NULL) {
		log_logf(system->log, LOG_LEVEL_DEBUG,
				"{AUTH} failed to retrieve user '%s' from index", username);
	}
	else {
		log_logf(system->log, LOG_LEVEL_DEBUG,
				"{AUTH} retrieved user '%s' from index", username);
	}

	return result;
}

AuthSystemUser *authSystem_userGetByToken(AuthSystem *system, char *token)
{
	int tokenLength = 0;

	AuthSystemUser *result = NULL;

	if((system == NULL) || (token == NULL) ||
			((tokenLength = strlen(token)) < 1)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	if(spinlock_readLock(&system->lock) < 0) {
		return NULL;
	}

	managedIndex_get(&system->tokens, token, tokenLength, ((void *)&result));

	spinlock_readUnlock(&system->lock);

	if(result == NULL) {
		log_logf(system->log, LOG_LEVEL_DEBUG,
				"{AUTH} failed to retrieve user '%s' from token index", token);
	}
	else {
		log_logf(system->log, LOG_LEVEL_DEBUG,
				"{AUTH} retrieved user '%s' token '%s' from token index",
				result->username, token);
	}

	return result;
}

AuthSystemUser *authSystem_userLogin(AuthSystem *system, char *username,
		char *password)
{
	int counter = 0;
	int keyLength = 0;
	int usernameLength = 0;
	char *key = NULL;

	AuthSystemUser *user = NULL;
	AuthSystemUser *userRecord = NULL;

	if((system == NULL) || (username == NULL) ||
			((usernameLength = strlen(username)) < 1)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	if(spinlock_readLock(&system->lock) < 0) {
		return NULL;
	}

	bptree_get(&system->users, username, usernameLength, ((void *)&user));

	spinlock_readUnlock(&system->lock);

	if(user == NULL) {
		log_logf(system->log, LOG_LEVEL_DEBUG,
				"{AUTH} failed to retrieve user '%s' from index", username);
		return NULL;
	}

	log_logf(system->log, LOG_LEVEL_DEBUG,
			"{AUTH} retrieved user '%s' from index", username);

	if(strcmp(password, user->password)) {
		log_logf(system->log, LOG_LEVEL_INFO,
				"{AUTH} detected user '%s' password mismatch on login",
				username);
		return NULL;
	}

	if(user->token != NULL) {
		if(managedIndex_get(&system->tokens, user->token, user->tokenLength,
						((void *)&userRecord)) == 0) {
			if(managedIndex_remove(&system->tokens, user->token,
						user->tokenLength) < 0) {
				log_logf(system->log, LOG_LEVEL_WARNING,
						"{AUTH} failed to remove user '%s' token '%s' "
						"from index",
						user->username, user->token);

				mutex_lock(&user->mutex);

				free(user->token);

				user->token = NULL;
				user->tokenLength = 0;

				mutex_unlock(&user->mutex);
			}
		}
	}

	do {
		key = token_new(&keyLength);

		counter++;

		if(managedIndex_get(&system->tokens, key, keyLength,
					((void *)&userRecord)) == 0) {
			free(key);
			continue;
		}

		mutex_lock(&user->mutex);

		user->tokenLength = keyLength;
		user->token = strndup(key, keyLength);

		mutex_unlock(&user->mutex);

		if(managedIndex_put(&system->tokens, key, keyLength,
					(void *)user, user_calculateMemorySize(user)) < 0) {
			log_logf(system->log, LOG_LEVEL_ERROR,
					"{AUTH} unable to login '%s' - "
					"failed to insert '%s' into token index",
					username, key);

			free(key);
			return NULL;
		}

		log_logf(system->log, LOG_LEVEL_DEBUG,
				"{AUTH} added user (login) '%s' token '%s' to token index "
				"(%i iterations)",
				username, key, counter);

		free(key);

		break;
	} while(atrue);

	return user;
}

aboolean authSystem_userLogout(AuthSystem *system, char *token)
{
	AuthSystemUser *user = NULL;
	AuthSystemUser *userRecord = NULL;

	if((system == NULL) || (token == NULL)) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	if((user = authSystem_userGetByToken(system, token)) == NULL) {
		return afalse;
	}

	log_logf(system->log, LOG_LEVEL_DEBUG,
			"{AUTH} retrieved user '%s' from token index", user->username);

	if(user->token != NULL) {
		if(managedIndex_get(&system->tokens, user->token, user->tokenLength,
						((void *)&userRecord)) == 0) {
			if(managedIndex_remove(&system->tokens, user->token,
						user->tokenLength) < 0) {
				log_logf(system->log, LOG_LEVEL_WARNING,
						"{AUTH} failed to remove user '%s' token '%s' "
						"from index",
						user->username, user->token);

				mutex_lock(&user->mutex);

				free(user->token);

				user->token = NULL;
				user->tokenLength = 0;

				mutex_unlock(&user->mutex);
			}
		}
	}

	return atrue;
}

int authSystem_removeUser(AuthSystem *system, char *username)
{
	if((system == NULL) || (username == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	/*
	 * TODO: this
	 */

	return -1;
}

// user functions

aboolean authSystem_userIsRoot(AuthSystemUser *user)
{
	aboolean result = afalse;

	if(user == NULL) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	mutex_lock(&user->mutex);

	result = user->isRoot;

	mutex_unlock(&user->mutex);

	return result;
}

int authSystem_userSetIsRoot(AuthSystemUser *user, aboolean isRoot)
{
	if(user == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&user->mutex);

	user->isRoot = isRoot;

	mutex_unlock(&user->mutex);

	return 0;
}

int authSystem_userGetUid(AuthSystemUser *user)
{
	int result = 0;

	if(user == NULL) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&user->mutex);

	result = user->uid;

	mutex_unlock(&user->mutex);

	return result;
}

int authSystem_userSetFirstName(AuthSystemUser *user, char *firstName)
{
	if((user == NULL) || (firstName == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&user->mutex);

	if(user->firstName != NULL) {
		free(user->firstName);
	}

	user->firstNameLength = strlen(firstName);
	user->firstName = strndup(firstName, user->firstNameLength);

	mutex_unlock(&user->mutex);

	return 0;
}

char *authSystem_userGetFirstName(AuthSystemUser *user)
{
	char *result = NULL;

	if(user == NULL) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	mutex_lock(&user->mutex);

	result = user->firstName;

	mutex_unlock(&user->mutex);

	return result;
}

int authSystem_userSetLastName(AuthSystemUser *user, char *lastName)
{
	if((user == NULL) || (lastName == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&user->mutex);

	if(user->lastName != NULL) {
		free(user->lastName);
	}

	user->lastNameLength = strlen(lastName);
	user->lastName = strndup(lastName, user->lastNameLength);

	mutex_unlock(&user->mutex);

	return 0;
}

char *authSystem_userGetLastName(AuthSystemUser *user)
{
	char *result = NULL;

	if(user == NULL) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	mutex_lock(&user->mutex);

	result = user->lastName;

	mutex_unlock(&user->mutex);

	return result;
}

int authSystem_userSetEmailAddress(AuthSystemUser *user, char *emailAddress)
{
	if((user == NULL) || (emailAddress == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&user->mutex);

	if(user->emailAddress != NULL) {
		free(user->emailAddress);
	}

	user->emailAddressLength = strlen(emailAddress);
	user->emailAddress = strndup(emailAddress, user->emailAddressLength);

	mutex_unlock(&user->mutex);

	return 0;
}

char *authSystem_userGetEmailAddress(AuthSystemUser *user)
{
	char *result = NULL;

	if(user == NULL) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	mutex_lock(&user->mutex);

	result = user->emailAddress;

	mutex_unlock(&user->mutex);

	return result;
}

char *authSystem_userGetUsername(AuthSystemUser *user)
{
	char *result = NULL;

	if(user == NULL) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	mutex_lock(&user->mutex);

	result = user->username;

	mutex_unlock(&user->mutex);

	return result;
}

char *authSystem_userGetPassword(AuthSystemUser *user)
{
	char *result = NULL;

	if(user == NULL) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	mutex_lock(&user->mutex);

	result = user->password;

	mutex_unlock(&user->mutex);

	return result;
}

char *authSystem_userGetToken(AuthSystemUser *user, int *tokenLength)
{
	char *result = NULL;

	if((user == NULL) || (tokenLength == NULL)) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	mutex_lock(&user->mutex);

	if(user->token != NULL) {
		*tokenLength = user->tokenLength;
		result = user->token;
	}
	else {
		*tokenLength = 0;
		result = NULL;
	}

	mutex_unlock(&user->mutex);

	return result;
}

void authSystem_userDisplay(void *stream, AuthSystemUser *user)
{
	int ii = 0;
	int nn = 0;
	int keyLength = 0;
	int permissionCount = 0;
	char *key = NULL;
	char *string = NULL;

	AuthSystemUserAccess *access = NULL;

	if(user == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	if(stream == NULL) {
		stream = (void *)stdout;
	}

	mutex_lock(&user->mutex);

	fprintf(stream, "Auth System User 0x%lx ::\n", (aptrcast)user);
	fprintf(stream, "\t is root          : %i\n", (int)user->isRoot);
	fprintf(stream, "\t uid              : %i\n", user->uid);
	fprintf(stream, "\t token length     : %i\n", user->tokenLength);
	fprintf(stream, "\t token            : '%s'\n", user->token);
	fprintf(stream, "\t first name       : '%s'\n", user->firstName);
	fprintf(stream, "\t last name        : '%s'\n", user->lastName);
	fprintf(stream, "\t email address    : '%s'\n", user->emailAddress);
	fprintf(stream, "\t username         : '%s'\n", user->username);
	fprintf(stream, "\t password         : '%s'\n", user->password);

	fprintf(stream, "\t base permissions ::\n");

	fprintf(stream, "\t\t controls length :: %i\n",
			user->access.accessControlLength);

	for(ii = 0; ii < user->access.accessControlLength; ii++) {
		fprintf(stream, "\t\t\t control  : '%s'\n",
				user->access.controls[ii].ipAddress);

		fprintf(stream, "\t\t\t\t access   : '");

		for(nn = 0; nn < 4; nn++) {
			if(user->access.controls[ii].address[nn] ==
					AUTHENTICATION_IP_ADDRESS_CONTROL_WILDCARD) {
				fprintf(stream, "*");
			}
			else {
				fprintf(stream, "%i", user->access.controls[ii].address[nn]);
			}

			if(nn < 3) {
				fprintf(stream, "/");
			}
		}

		fprintf(stream, "'\n");
	}

	for(ii = 0; ii < AUTHENTICATION_PERMISSIONS_LIST_LENGTH; ii++) {
		if(user->access.permissions[ii]) {
			string = "true";
		}
		else {
			string = "false";
		}

		fprintf(stream, "\t\t access to '%20s' : %s\n",
				authSystem_permissionsToString(ii), string);
	}

	bptree_getLeafCount(&user->accessIndex, &permissionCount);

	fprintf(stream, "\t permission count : %i\n", permissionCount);

	keyLength = 1;
	key = (char *)malloc(sizeof(char) * (keyLength + 1));

	memset(key, 0, keyLength);

	while((bptree_getNext(&user->accessIndex, key, keyLength,
					((void *)&access)) == 0) &&
			(access != NULL)) {
		free(key);

		fprintf(stream, "\t\t user-access on domain '%s' ::\n",
				access->domainKey);

		fprintf(stream, "\t\t\t controls length :: %i\n",
				access->accessControlLength);

		for(ii = 0; ii < access->accessControlLength; ii++) {
			fprintf(stream, "\t\t\t\t control  : '%s'\n",
					access->controls[ii].ipAddress);

			fprintf(stream, "\t\t\t\t\t access   : '");

			for(nn = 0; nn < 4; nn++) {
				if(access->controls[ii].address[nn] ==
						AUTHENTICATION_IP_ADDRESS_CONTROL_WILDCARD) {
					fprintf(stream, "*");
				}
				else {
					fprintf(stream, "%i", access->controls[ii].address[nn]);
				}

				if(nn < 3) {
					fprintf(stream, "/");
				}
			}

			fprintf(stream, "'\n");
		}

		for(ii = 0; ii < AUTHENTICATION_PERMISSIONS_LIST_LENGTH; ii++) {
			if(access->permissions[ii]) {
				string = "true";
			}
			else {
				string = "false";
			}

			fprintf(stream, "\t\t\t access to '%20s' : %s\n",
					authSystem_permissionsToString(ii), string);
		}

		key = strndup(access->domainKey, access->domainKeyLength);
		keyLength = access->domainKeyLength;
	}

	free(key);

	mutex_unlock(&user->mutex);
}

// user access functions

int authSystem_userNewAccess(AuthSystemUser *user, char *domainKey,
		int domainKeyLength)
{
	int rc = 0;

	AuthSystemUserAccess *access = NULL;

	if((user == NULL) || (domainKey == NULL) || (domainKeyLength < 1)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if((access = authSystem_userGetAccess(user, domainKey)) != NULL) {
		return -1;
	}

	access = userAccess_new(domainKey, domainKeyLength);

	mutex_lock(&user->mutex);

	rc = bptree_put(&user->accessIndex, domainKey, domainKeyLength,
			(void *)access);

	mutex_unlock(&user->mutex);

	if(rc < 0) {
		userAccess_free(access);
		return -1;
	}

	return 0;
}

AuthSystemUserAccess *authSystem_userGetAccess(AuthSystemUser *user,
		char *domainKey)
{
	int domainKeyLength = 0;

	AuthSystemUserAccess *result = NULL;

	if((user == NULL) ||
			((domainKey != NULL) &&
			 ((domainKeyLength = strlen(domainKey)) < 1))) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	mutex_lock(&user->mutex);

	if(domainKey == NULL) {
		result = &(user->access);
	}
	else {
		bptree_get(&user->accessIndex, domainKey, domainKeyLength,
				((void *)&result));
	}

	mutex_unlock(&user->mutex);

	return result;
}

int authSystem_userRemoveAccess(AuthSystemUser *user, char *domainKey)
{
	int rc = 0;

	if((user == NULL) || (domainKey == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	mutex_lock(&user->mutex);

	rc = bptree_remove(&user->accessIndex, domainKey, strlen(domainKey));

	mutex_unlock(&user->mutex);

	return rc;
}

// access functions

int authSystem_accessAddIpAddress(AuthSystemUserAccess *access,
		char *ipAddress)
{
	int ref = 0;
	int ipAddressLength = 0;
	int address[4];

	if((access == NULL) || (ipAddress == NULL) ||
			((ipAddressLength = strlen(ipAddress)) < 1)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(ipAddress_convert(ipAddress, ipAddressLength, address) < 0) {
		return -1;
	}

	if(access->controls == NULL) {
		ref = 0;
		access->accessControlLength = 1;
		access->controls = (AuthSystemAccessControl *)malloc(
				sizeof(AuthSystemAccessControl) * access->accessControlLength);
	}
	else {
		ref = access->accessControlLength;
		access->accessControlLength += 1;
		access->controls = (AuthSystemAccessControl *)realloc(access->controls,
				(sizeof(AuthSystemAccessControl) *
				 access->accessControlLength));
	}

	access->controls[ref].ipAddressLength = ipAddressLength;
	access->controls[ref].ipAddress = strndup(ipAddress, ipAddressLength);
	access->controls[ref].address[0] = address[0];
	access->controls[ref].address[1] = address[1];
	access->controls[ref].address[2] = address[2];
	access->controls[ref].address[3] = address[3];

	return 0;
}

int authSystem_accessSetPermission(AuthSystemUserAccess *access,
		AuthSystemPermissions permission, aboolean isPermitted)
{
	if((access == NULL) ||
			((permission < 0) ||
			 (permission >= AUTHENTICATION_PERMISSIONS_LIST_LENGTH))) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	access->permissions[permission] = isPermitted;

	return 0;
}

// permissions functions

AuthSystemUserAccess *authSystem_permissionGetAccess(AuthSystem *system,
		char *token, char *domainKey)
{
	int domainKeyLength = 0;

	AuthSystemUser *user = NULL;
	AuthSystemUserAccess *result = NULL;

	if((system == NULL) || (token == NULL) ||
			((domainKey != NULL) &&
			 ((domainKeyLength = strlen(domainKey)) < 1))) {
		DISPLAY_INVALID_ARGS;
		return NULL;
	}

	if((user = authSystem_userGetByToken(system, token)) == NULL) {
		return NULL;
	}

	mutex_lock(&user->mutex);

	if(domainKey == NULL) {
		result = &(user->access);
	}
	else {
		bptree_get(&user->accessIndex, domainKey, domainKeyLength,
				((void *)&result));
	}

	mutex_unlock(&user->mutex);

	return result;
}

aboolean authSystem_isPermitted(AuthSystem *system, char *token,
		char *domainKey, char *ipAddress, AuthSystemPermissions permission)
{
	aboolean hasAccessControl = afalse;
	int ii = 0;
	int address[4];

	AuthSystemUser *user = NULL;
	AuthSystemUserAccess *access = NULL;

	if((system == NULL) || (token == NULL) || (ipAddress == NULL)) {
		DISPLAY_INVALID_ARGS;
		return afalse;
	}

	log_logf(system->log, LOG_LEVEL_DEBUG,
			"{AUTH} checking permissios for "
			"{ token '%s', domain '%s', ip address '%s', permission '%s' }",
			token, domainKey, ipAddress,
			authSystem_permissionsToString(permission));

	if((user = authSystem_userGetByToken(system, token)) == NULL) {
		return afalse;
	}

	if(user->isRoot) {
		return atrue;
	}

	mutex_lock(&user->mutex);

	if(domainKey == NULL) {
		access = &(user->access);
	}
	else {
		bptree_get(&user->accessIndex, domainKey, strlen(domainKey),
				((void *)&access));
	}

	mutex_unlock(&user->mutex);

	if(access == NULL) {
		return afalse;
	}

	if(ipAddress_convert(ipAddress, strlen(ipAddress), address) < 0) {
		return afalse;
	}

	for(ii = 0; ii < access->accessControlLength; ii++) {
		hasAccessControl = ipAddress_compare(access->controls[ii].address,
				address);

		if(hasAccessControl) {
			break;
		}
	}

	if((hasAccessControl) && (access->permissions[permission])) {
		return atrue;
	}

	return afalse;
}

int authSystem_setPermission(AuthSystem *system, char *token, char *domainKey,
		AuthSystemPermissions permission, aboolean isPermitted)
{
	AuthSystemUserAccess *access = NULL;

	if((system == NULL) || (token == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if((access = authSystem_permissionGetAccess(system, token,
					domainKey)) == NULL) {
		log_logf(system->log, LOG_LEVEL_DEBUG,
				"{AUTH} failed to locate permissions for token '%s' "
				"on domain '%s'",
				token, domainKey);

		return -1;
	}

	access->permissions[permission] = isPermitted;

	return 0;
}

// helper functions

int authSystem_loadConfiguration(AuthSystem *system, char *filename)
{
	int length = 0;
	alint fileLength = 0;
	char *buffer = NULL;

	FileHandle fh;
	Json *object = NULL;

	if((system == NULL) || (filename == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(file_init(&fh, filename, "r", 0) < 0) {
		return -1;
	}

	if((file_getFileLength(&fh, &fileLength) < 0) || (fileLength < 1)) {
		file_free(&fh);
		return -1;
	}

	length = (int)fileLength;
	buffer = (char *)malloc(sizeof(char) * (length + 1));

	if(file_read(&fh, buffer, length) < 0) {
		free(buffer);
		file_free(&fh);
		return -1;
	}

	file_free(&fh);

	if((object = json_newFromString(buffer)) == NULL) {
		free(buffer);
		return -1;
	}

	free(buffer);

	if(authSystem_jsonToSystem(system, object) < 0) {
		json_freePtr(object);
		return -1;
	}

	json_freePtr(object);

	log_logf(system->log, LOG_LEVEL_INFO,
			"authentication system loaded state from '%s'", filename);

	return 0;
}

int authSystem_saveConfiguration(AuthSystem *system, char *filename)
{
	int length = 0;
	char *buffer = NULL;

	FileHandle fh;
	Json *object = NULL;

	if((system == NULL) || (filename == NULL)) {
		DISPLAY_INVALID_ARGS;
		return -1;
	}

	if(file_init(&fh, filename, "w", 0) < 0) {
		return -1;
	}

	if((object = authSystem_toJson(system)) == NULL) {
		file_free(&fh);
		return -1;
	}

	buffer = json_toString(object, &length);

	if(file_write(&fh, buffer, length) < 0) {
		json_freePtr(object);
		free(buffer);
		file_free(&fh);
		return -1;
	}

	file_free(&fh);

	log_logf(system->log, LOG_LEVEL_INFO,
			"authentication system saved state from '%s'", filename);

	// cleanup

	json_freePtr(object);
	free(buffer);

	return 0;
}

