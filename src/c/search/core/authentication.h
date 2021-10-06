/*
 * authentication.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * A library component for the search-engine to define the set of users who may
 * access the system, with a permission set defined by domain, header file.
 *
 * Written by Josh English.
 */

/*
 * TODO: add functions to manage the managed-index of tokens
 */

#if !defined(_SEARCH_CORE_AUTHENTICATION_H)

#define _SEARCH_CORE_AUTHENTICATION_H

#if !defined(_SEARCH_H) && !defined(_SEARCH_CORE_COMPONENT)
#	error "Incorrect use of this header file."
#endif // _SEARCH_H || _SEARCH_CORE_COMPONENT

#if defined(__cplusplus)
extern "C" {
#endif // __cplusplus


// define authentication public constants

typedef enum _AuthSystemPermissions {
	AUTHENTICATION_PERMISSIONS_SETTINGS = 0,
	AUTHENTICATION_PERMISSIONS_DOMAIN,
	AUTHENTICATION_PERMISSIONS_INDEX,
	AUTHENTICATION_PERMISSIONS_GET,
	AUTHENTICATION_PERMISSIONS_PUT,
	AUTHENTICATION_PERMISSIONS_UPDATE,
	AUTHENTICATION_PERMISSIONS_DELETE,
	AUTHENTICATION_PERMISSIONS_SEARCH,
	AUTHENTICATION_PERMISSIONS_LIST,
	AUTHENTICATION_PERMISSIONS_COMMAND_DOMAIN,
	AUTHENTICATION_PERMISSIONS_COMMAND_INDEX,
	AUTHENTICATION_PERMISSIONS_COMMAND_SERVER,
	AUTHENTICATION_PERMISSIONS_LIST_LENGTH,
	AUTHENTICATION_PERMISSIONS_END_OF_LIST = -1
} AuthSystemPermissions;

typedef enum _AuthSystemIpAddressControls {
	AUTHENTICATION_IP_ADDRESS_CONTROL_INIT = -1,
	AUTHENTICATION_IP_ADDRESS_CONTROL_WILDCARD = -2,
	AUTHENTICATION_IP_ADDRESS_CONTROL_ERROR = -3
} AuthSystemIpAddressControls;

#define AUTH_SYSTEM_TOKEN_LENGTH						32


// define authentication public data types

typedef struct _AuthSystemAccessControl {
	int ipAddressLength;
	int address[4];
	char *ipAddress;
} AuthSystemAccessControl;

typedef struct _AuthSystemUserAccess {
	aboolean permissions[AUTHENTICATION_PERMISSIONS_LIST_LENGTH];
	int domainKeyLength;
	int accessControlLength;
	char *domainKey;
	AuthSystemAccessControl *controls;
} AuthSystemUserAccess;

typedef struct _AuthSystemUser {
	aboolean isRoot;
	int uid;
	int firstNameLength;
	int lastNameLength;
	int emailAddressLength;
	int usernameLength;
	int passwordLength;
	int tokenLength;
	char *firstName;
	char *lastName;
	char *emailAddress;
	char *username;
	char *password;
	char *token;
	AuthSystemUserAccess access;
	Bptree accessIndex;
	Mutex mutex;
} AuthSystemUser;

typedef struct _AuthSystem {
	aboolean isEnabled;
	aboolean isLogInternallyAllocated;
	int uidCounter;
	Bptree users;
	ManagedIndex tokens;
	Log *log;
	Spinlock lock;
} AuthSystem;


// declare authentication public functions

int authSystem_init(AuthSystem *system, Log *log);

AuthSystem *authSystem_new(Log *log);

int authSystem_free(AuthSystem *system);

int authSystem_freePtr(AuthSystem *system);

char *authSystem_permissionsToString(AuthSystemPermissions permission);

AuthSystemPermissions authSystem_stringToPermission(char *string);

aboolean authSystem_isLocked(AuthSystem *system);

aboolean authSystem_isReadLocked(AuthSystem *system);

int authSystem_lock(AuthSystem *system);

int authSystem_unlock(AuthSystem *system);

int authSystem_setIsEnabled(AuthSystem *system, aboolean isEnabled);

aboolean authSystem_isEnabled(AuthSystem *system);

Json *authSystem_toJson(AuthSystem *system);

int authSystem_jsonToSystem(AuthSystem *system, Json *object);

// user index functions

int authSystem_newUser(AuthSystem *system, char *username, char *password);

AuthSystemUser *authSystem_userGet(AuthSystem *system, char *username);

AuthSystemUser *authSystem_userGetByToken(AuthSystem *system, char *token);

AuthSystemUser *authSystem_userLogin(AuthSystem *system, char *username,
		char *password);

aboolean authSystem_userLogout(AuthSystem *system, char *token);

int authSystem_removeUser(AuthSystem *system, char *username);

// user functions

aboolean authSystem_userIsRoot(AuthSystemUser *user);

int authSystem_userSetIsRoot(AuthSystemUser *user, aboolean isRoot);

int authSystem_userGetUid(AuthSystemUser *user);

int authSystem_userSetFirstName(AuthSystemUser *user, char *firstName);

char *authSystem_userGetFirstName(AuthSystemUser *user);

int authSystem_userSetLastName(AuthSystemUser *user, char *lastName);

char *authSystem_userGetLastName(AuthSystemUser *user);

int authSystem_userSetEmailAddress(AuthSystemUser *user, char *emailAddress);

char *authSystem_userGetEmailAddress(AuthSystemUser *user);

char *authSystem_userGetUsername(AuthSystemUser *user);

char *authSystem_userGetPassword(AuthSystemUser *user);

char *authSystem_userGetToken(AuthSystemUser *user, int *tokenLength);

void authSystem_userDisplay(void *stream, AuthSystemUser *user);

// user access functions

int authSystem_userNewAccess(AuthSystemUser *user, char *domainKey,
		int domainKeyLength);

AuthSystemUserAccess *authSystem_userGetAccess(AuthSystemUser *user,
		char *domainKey);

int authSystem_userRemoveAccess(AuthSystemUser *user, char *domainKey);

// access functions

int authSystem_accessAddIpAddress(AuthSystemUserAccess *access,
		char *ipAddress);

int authSystem_accessSetPermission(AuthSystemUserAccess *access,
		AuthSystemPermissions permission, aboolean isPermitted);

// permissions functions

AuthSystemUserAccess *authSystem_permissionGetAccess(AuthSystem *system,
		char *token, char *domainKey);

aboolean authSystem_isPermitted(AuthSystem *system, char *token,
		char *domainKey, char *ipAddress, AuthSystemPermissions permission);

int authSystem_setPermission(AuthSystem *system, char *token, char *domainKey,
		AuthSystemPermissions permission, aboolean isPermitted);

// helper functions

int authSystem_loadConfiguration(AuthSystem *system, char *filename);

int authSystem_saveConfiguration(AuthSystem *system, char *filename);


#if defined(__cplusplus)
};
#endif // __cplusplus

#endif // _SEARCH_CORE_AUTHENTICATION_H

