/*
 * beyblade_loadTest.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The BeyBlade Battles load test application.
 *
 * Written by Josh English.
 */

// preprocessor directives

#define _CORE_COMMON_BYPASS_STD_STREAM
#include "core/core.h"


// define beyblade load test constants

// environment

//#define ENVIRONMENT								1 // local
#define ENVIRONMENT								2 // dev
//#define ENVIRONMENT								3 // production

// debugging

#define DEBUG_MODE								atrue
//#define DEBUG_MODE								afalse

#define DEBUG_STREAM							stdout


// logging

#define LOG_OUTPUT								LOG_OUTPUT_STDOUT
//#define LOG_OUTPUT								LOG_OUTPUT_FILE

#define LOG_FILENAME							"logs/beyblade.load.test.log"

#define LOG_LEVEL								LOG_LEVEL_DEBUG

// protocols

//#define SFS_PROTOCOL_EMPTY_COUNT_THRESHOLD		32
//#define SFS_PROTOCOL_EMPTY_COUNT_THRESHOLD		256
//#define SFS_PROTOCOL_EMPTY_COUNT_THRESHOLD		1024
#define SFS_PROTOCOL_EMPTY_COUNT_THRESHOLD		1024


// servers

// web server

#if (ENVIRONMENT == 1) // local
#	define SFS_WEB_SERVER_HOSTNAME				"www.beybladebattles.com"
#	define SFS_WEB_SERVER_IP					"192.168.0.1"
#elif (ENVIRONMENT == 2) // dev
#	define SFS_WEB_SERVER_HOSTNAME				\
	"www.stage.beyblades.interfuel.com"
#	define SFS_WEB_SERVER_IP					"209.151.228.169"
#endif

#define SFS_WEB_SERVER_PORT						80

// lobby server

#if (ENVIRONMENT == 1) // local
#	define SFS_LOBBY_SERVER_IP					"192.168.0.1"
#elif (ENVIRONMENT == 2) // dev
#	define SFS_LOBBY_SERVER_IP					"209.151.238.68"
#endif

#define SFS_LOBBY_SERVER_PORT					9339

#define SFS_LOBBY_SERVER_PORT_STRING			"9339"

// game server

#if (ENVIRONMENT == 1) // local
#	define SFS_GAME_SERVER_IP					"192.168.0.1"
#elif (ENVIRONMENT == 2) // dev
#	define SFS_GAME_SERVER_IP					"209.151.238.66"
#endif

#define SFS_GAME_SERVER_PORT					10000

#define SFS_GAME_SERVER_PORT_STRING				"10000"

// sfs servers

#define SFS_SERVER_RETRY_THRESHOLD				3


// user data

#define USER_DATA_FILENAME						\
	"/home/josh/files/projects/dev-temple/beyblade-blattles/data/bb.users.csv"

// load test

//#define LOAD_TEST_CLIENTS						1
#define LOAD_TEST_CLIENTS						2
//#define LOAD_TEST_CLIENTS						10
//#define LOAD_TEST_CLIENTS						20
//#define LOAD_TEST_CLIENTS						128

//#define LOAD_TEST_THREADS						1
#define LOAD_TEST_THREADS						2
//#define LOAD_TEST_THREADS						10
//#define LOAD_TEST_THREADS						20


// client status

typedef enum _ClientStatus {
	CLIENT_STATUS_INIT = 1,
	CLIENT_STATUS_IS_HTTP_LOGIN,
	CLIENT_STATUS_HAS_POLICY_FILE,
	CLIENT_STATUS_HAS_VERSION_CHECK,
	CLIENT_STATUS_IS_LOBBY_LOGIN,
	CLIENT_STATUS_IS_GET_ROOM_LIST,
	CLIENT_STATUS_IS_JOIN_ROOM,
	CLIENT_STATUS_IS_ENABLE_AUTO_BATTLE,
	CLIENT_STATUS_IS_CREATE_GAME,
	CLIENT_STATUS_IS_GAME_READY,
	CLIENT_STATUS_HAS_GAME_SERVER_POLICY_FILE,
	CLIENT_STATUS_HAS_GAME_SERVER_VERSION_CHECK,
	CLIENT_STATUS_IS_GAME_SERVER_LOGIN,
	CLIENT_STATUS_IS_GAME_SERVER_LOGIN_HAS_ROOM_LIST,
	CLIENT_STATUS_IS_GAME_SERVER_LOGIN_IS_JOIN_OK,
	CLIENT_STATUS_IS_GAME_SERVER_LOGIN_HAS_JOINED,
	CLIENT_STATUS_IS_GAME_SERVER_READY,
	CLIENT_STATUS_IS_GAME_SERVER_WAITING,
	CLIENT_STATUS_IS_BATTLE_INIT,
	CLIENT_STATUS_IS_BATTLE_INIT_WAITING,
	CLIENT_STATUS_IS_BATTLE_IN_PROGRESS,
	CLIENT_STATUS_ERROR = -1
} ClientStatus;

// lobby rooms names

typedef enum _LobbyRoomTypes {
	LOBBY_ROOM_TYPE_FOREST = 1,
	LOBBY_ROOM_TYPE_ICE,
	LOBBY_ROOM_TYPE_SKYSCRAPER,
	LOBBY_ROOM_TYPE_BLACKHOLE,
	LOBBY_ROOM_TYPE_MOON,
	LOBBY_ROOM_TYPE_VOLCANO,
	LOBBY_ROOM_TYPE_PIRATE,
	LOBBY_ROOM_TYPE_SPACESHIP,
	LOBBY_ROOM_TYPE_PYRAMID,
	LOBBY_ROOM_TYPE_SKATEPARK,
	LOBBY_ROOM_TYPE_UNKNOWN = -1
} LobbyRoomTypes;


// define beyblade load test data types

typedef struct _LobbyRoom {
	int id;
	char *name;
} LobbyRoom;

typedef struct _BeyBladeLoadSocketMetrics {
	alint bytesSent;
	alint bytesReceived;
	alint readsAttempted;
	alint readsCompleted;
	alint writesAttempted;
	alint writesCompleted;
	double totalReadTime;
	double totalWriteTime;
	Mutex mutex;
} BeyBladeLoadSocketMetrics;

typedef struct _ClientConnection {
	int sd;
	int port;
	char *hostname;
	Socket socket;
	Mutex mutex;
} ClientConnection;

typedef struct _ClientHttpProperties {
	char *userId;
	char *userName;
	char *password;
	char *sessionId;
	char *playerToken;
} ClientHttpProperties;

typedef struct _ClientSfsLobbyOpponent {
	char *artName;
	char *level;
	char *name;
	char *points;
} ClientSfsLobbyOpponent;

typedef struct _ClientSfsLobbyProperties {
	int opponentLength;
	int autoJoinCounter;
	char *userId;
	char *userName;
	char *roomName;
	ClientSfsLobbyOpponent *opponents;
} ClientSfsLobbyProperties;

typedef struct _ClientSfsGameServerProperties {
	int topPick;
	char *userId;
	char *userName;
	char *roomId;
	char *gameType;
	Xpath *roomList;
	Json *sessionInfo;
} ClientSfsGameServerProperties;

typedef struct _ClientMessage {
	int length;
	char *body;
} ClientMessage;

typedef struct _Client {
	aboolean isBusy;
	int status;
	ClientHttpProperties http;
	ClientSfsLobbyProperties sfsLobby;
	ClientSfsGameServerProperties sfsGameServer;
	ClientConnection connection;
	FifoStack messageQueue;
	Mutex mutex;
} Client;

typedef struct _BeyBladeLoadTest {
	aboolean isTestReady;
	int clientLength;
	BeyBladeLoadSocketMetrics socketMetrics;
	HttpProtocol httpProtocol;
	Client *clients;
	Bptree clientIndex;
	TmfContext tmf;
	Mutex mutex;
	Log log;
} BeyBladeLoadTest;


// declare beyblade load test functions

// general functions

static void freeNothing(void *memory);

static void displayString(void *stream, char *string, int length);

static char *getPropertyFromString(char *string, int stringLength,
		int propertyRef, char delimiter);

static char *pickRandomRoom();

// context functions

static int context_init();

static int context_free();

// message functions

static char *message_html_getPayloadPtr(char *message);

static char *message_html_buildLoginRequest(char *hostname, Client *client,
		int *messageLength);

static aboolean message_html_isLoginValid(Client *client, char *message,
		int messageLength);

static aboolean message_sfs_enqueueAdditionalMessages(Client *client,
		char *message, int messageLength);

static aboolean message_xml_isResponseASystemMessage(Xpath *xpath);

static aboolean message_xml_isResponseARoomListMessage(Xpath *xpath);

static aboolean message_xml_isResponseAJoinOkMessage(Client *client,
		Xpath *xpath);

static char *message_xml_buildPolicyFileRequest(int *messageLength);

static aboolean message_xml_isPolicyFileResponseValid(char *message,
		int messageLength, aboolean isLobby);

static char *message_xml_buildVersionCheck(int *messageLength);

static aboolean message_xml_isVersionCheckResponseValid(char *message,
		int messageLength, aboolean isLobby);

static char *message_xml_buildSfsServerLogin(char *encodedUserName,
		int *messageLength);

static aboolean message_json_isSfsServerLoginResponseValid(Client *client,
		char *message, int messageLength, aboolean isLobby);

static char *message_xml_buildGetRoomList(int *messageLength);

static aboolean message_xml_isGetRoomListResponseValid(Client *client,
		char *message, int messageLength);

static char *message_json_buildJoinRoom(char *oldRoomName, char *newRoomName,
		int *messageLength);

static aboolean message_json_isJoinRoomResponseValid(Client *client,
		char *message, int messageLength);

static char *message_json_buildEnableAutoBattle(int *messageLength);

static char *message_json_buildChangeAutoBattleParams(int *messageLength);

static char *message_json_buildCreateGame(char *opponentName,
		int *messageLength);

static char *message_json_buildIsBattleReady(char *roomId, char *gameType,
		int *messageLength);

static char *message_json_buildIsBattleInitialized(char *roomId,
		char *gameType, int *messageLength);

static char *message_json_buildBattleSelection(Client *client,
		int *messageLength);

static Json *message_json_messageToJson(Client *client, char *message,
		int messageLength, char **command);

static char *message_json_getStringFrommessage(Json *object, char *key);

static Xpath *message_xpath_messageToXpath(Client *client, char *message,
		int messageLength);

// client connection functions

static int clientConnection_init(ClientConnection *client, char *hostname,
		int port);

static int clientConnection_free(ClientConnection *client);

static int clientConnection_sendMessage(ClientConnection *client,
		char *message, int messageLength);

static char *clientConnection_listenForMessage(ClientConnection *client,
		int *responseLength);

static char *clientConnection_executeTransaction(ClientConnection *client,
		char *message, int messageLength, int *responseLength);

static char *clientConnection_executeHttpTransaction(ClientConnection *client,
		char *message, int messageLength, int *responseLength);

// client message functions

static ClientMessage *clientMessage_new(char *message, int messageLength);

static void clientMessage_free(void *memory);

// client functions

static void client_init(Client *client);

static int client_setProperties(Client *client, char *string, int stringLength);

static void client_free(Client *client);

static char *client_statusToString(Client *client);

static void client_display(void *stream, Client *client);

static aboolean client_addToOpponentListFromJson(Client *client,
		Json *opponentObject);

static aboolean client_updateOpponentListFromJson(Client *client,
		Json *object, aboolean isJoinRoomResponse, aboolean rebuildList);

static int client_executeHttpLogin(Client *client);

static int client_executePolicyFileRequest(Client *client, aboolean isLobby);

static int client_executeVersionCheck(Client *client, aboolean isLobby);

static int client_executeSfsServerLogin(Client *client, aboolean isLobby);

static int client_executeGetRoomList(Client *client);

static int client_executeJoinRoom(Client *client);

static int client_executeEnableAutoBattle(Client *client);

static int client_executeCreateGame(Client *client);

static int client_executeIsBattleReady(Client *client);

static int client_executeIsBattleInitialized(Client *client);

// thread functions

static void *thread_clientSocketListener(void *threadContext, void *argument);

static void *thread_clientLoadTest(void *threadContext, void *argument);


// define beyblade load test global variables

BeyBladeLoadTest *context = NULL;

LobbyRoom LOBBY_ROOM_LIST[] = {
	{	LOBBY_ROOM_TYPE_FOREST,		"Forest"		},
	{	LOBBY_ROOM_TYPE_ICE,		"Ice"			},
	{	LOBBY_ROOM_TYPE_SKYSCRAPER,	"Skyscraper"	},
	{	LOBBY_ROOM_TYPE_BLACKHOLE,	"Blackhole"		},
	{	LOBBY_ROOM_TYPE_MOON,		"Moon"			},
	{	LOBBY_ROOM_TYPE_VOLCANO,	"Volcano"		},
	{	LOBBY_ROOM_TYPE_PIRATE,		"Pirate"		},
	{	LOBBY_ROOM_TYPE_SPACESHIP,	"SpaceShip"		},
	{	LOBBY_ROOM_TYPE_PYRAMID,	"Pyramid"		},
	{	LOBBY_ROOM_TYPE_SKATEPARK,	"Skatepark"		},
	{	LOBBY_ROOM_TYPE_UNKNOWN,	NULL			}
};


// main function

int main()
{
	int ii = 0;
	int rc = 0;
	int threadId = 0;

	signal_registerDefault();

	printf("BeyBlade Battles Load Test Application\n");
	printf("Using Asgard Ver %s (build %s).\n\n",
			ASGARD_VERSION, ASGARD_DATE);

	if(DEBUG_MODE) {
		printf("Note: debug mode is on.\n");
	}

	if((rc = context_init()) < 0) {
		return (rc * -1);
	}

	mutex_lock(&context->mutex);

	for(ii = 0; ii < LOAD_TEST_THREADS; ii++) {
		if(tmf_spawnThread(&context->tmf,
					thread_clientLoadTest,
					(void *)context,
					&threadId) < 0) {
			log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
					__FUNCTION__, __LINE__);
			mutex_unlock(&context->mutex);
			return 1;
		}
	}

	context->isTestReady = atrue;

	mutex_unlock(&context->mutex);

//	time_sleep(10);
//	time_sleep(32);
//	time_sleep(120);
	time_sleep(300);

	if((rc = context_free()) < 0) {
		return (rc * -1);
	}

	printf("Load test completed.\n");

	return 0;
}


// define beyblade load test functions

// general functions

static void freeNothing(void *memory)
{
	// do nothing
}

static void displayString(void *stream, char *string, int length)
{
	int ii = 0;

	fprintf(stream, "STRING(%i):'", length);

	if(string != NULL) {
		for(ii = 0; ii < length; ii++) {
			if(((unsigned int)string[ii] < 32) ||
					((unsigned int)string[ii] > 126)) {
				fprintf(stream, "(%02i)", (int)((unsigned int)string[ii]));
			}
			else {
				fprintf(stream, "%c", string[ii]);
			}
		}
	}

	fprintf(stream, "'\n");
}

static char *getPropertyFromString(char *string, int stringLength,
		int propertyRef, char delimiter)
{
	int ii = 0;
	int counter = 0;
	int resultRef = 0;
	int resultLength = 0;
	char *result = NULL;

	for(ii = 0; ii < stringLength; ii++) {
		if(string[ii] == delimiter) {
			counter++;
			continue;
		}
		else if(((unsigned int)string[ii] < 32) ||
				((unsigned int)string[ii] > 126)) {
			continue;
		}

		if(counter == propertyRef) {
			if(result == NULL) {
				resultRef = 0;
				resultLength = 8;
				result = (char *)malloc(sizeof(char) * (resultLength + 1));
			}

			result[resultRef] = string[ii];
			resultRef++;

			if(resultRef >= resultLength) {
				resultLength *= 2;
				result = (char *)realloc(result,
						(sizeof(char) * (resultLength + 1)));
			}
		}
		else if(counter > propertyRef) {
			break;
		}
	}

	if(result != NULL) {
		result[resultRef] = '\0';
	}

	return result;
}

static char *pickRandomRoom()
{
	int ii = 0;
	int pick = 0;

	static int lobbyRoomListLength = 0;

	if(lobbyRoomListLength == 0) {
		for(ii = 0; LOBBY_ROOM_LIST[ii].id != LOBBY_ROOM_TYPE_UNKNOWN; ii++) {
			lobbyRoomListLength++;
		}
	}

	system_pickRandomSeed();

	pick = (rand() % lobbyRoomListLength);

	return LOBBY_ROOM_LIST[pick].name;
}

// context functions

static int context_init()
{
	int ii = 0;
	int rc = 0;
	int threadId = 0;
	int lineLength = 0;
	char line[8192];

	FileHandle fh;

	context = (BeyBladeLoadTest *)malloc(sizeof(BeyBladeLoadTest));

	mutex_init(&context->mutex);

	mutex_lock(&context->mutex);

	mutex_init(&context->socketMetrics.mutex);

	http_protocol_init(&context->httpProtocol,
			afalse,			// is temp file mode
			0,				// temp file threshold bytes
			0,				// temp file space allowed bytes
			0,				// temp file space in-use bytes
			NULL			// temp file base path
			);

	if(log_init(&context->log,
				LOG_OUTPUT,
				LOG_FILENAME,
				LOG_LEVEL) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		mutex_unlock(&context->mutex);
		return -1;
	}

	context->clientLength = LOAD_TEST_CLIENTS;
	context->clients = (Client *)malloc(sizeof(Client) * context->clientLength);

	for(ii = 0; ii < context->clientLength; ii++) {
		client_init(&context->clients[ii]);
	}

	bptree_init(&context->clientIndex);
	bptree_setFreeFunction(&context->clientIndex, freeNothing);

	if((rc = file_init(&fh, USER_DATA_FILENAME, "r", 0)) < 0) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i "
				"with '%s'", __FUNCTION__, __LINE__,
				file_getErrorMessage(&fh, rc));
		mutex_unlock(&context->mutex);
		return -1;
	}

	if((rc = file_readLine(&fh,
					line,
					(int)(sizeof(line)),
					&lineLength)) < 0) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i "
				"with '%s'", __FUNCTION__, __LINE__,
				file_getErrorMessage(&fh, rc));
		mutex_unlock(&context->mutex);
		return -1;
	}

	ii = 0;

	do {
		if((rc = file_readLine(&fh,
						line,
						(int)(sizeof(line)),
						&lineLength)) < 0) {
			log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i "
					"with '%s'", __FUNCTION__, __LINE__,
					file_getErrorMessage(&fh, rc));
			mutex_unlock(&context->mutex);
			return -1;
		}

		if(client_setProperties(&context->clients[ii], line, lineLength) < 0) {
			log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
					__FUNCTION__, __LINE__);
			mutex_unlock(&context->mutex);
			return -1;
		}

		if(client_executeHttpLogin(&context->clients[ii]) == 0) {
			ii++;
		}
	} while(ii < context->clientLength);

	if((rc = file_free(&fh)) < 0) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i "
				"with '%s'", __FUNCTION__, __LINE__,
				file_getErrorMessage(&fh, rc));
		mutex_unlock(&context->mutex);
		return -1;
	}

	if(tmf_init(&context->tmf) < 0) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		mutex_unlock(&context->mutex);
		return -1;
	}

	if(tmf_spawnThread(&context->tmf, thread_clientSocketListener,
				(void *)context, &threadId) < 0) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		mutex_unlock(&context->mutex);
		return -1;
	}

	mutex_unlock(&context->mutex);

	return 0;
}

static int context_free()
{
	int ii = 0;

	mutex_lock(&context->mutex);

	// shutdown thread pools

	tmf_free(&context->tmf);

	// free the clients

	if(context->clients != NULL) {
		for(ii = 0; ii < context->clientLength; ii++) {
			client_display(stdout, &context->clients[ii]);
			client_free(&context->clients[ii]);
		}
	}

	// display global metrics

	mutex_lock(&context->socketMetrics.mutex);

	printf("Load Test Total Socket Metrics ::\n");

	printf("\t bytes sent            : %0.0f\n",
			(double)context->socketMetrics.bytesSent);
	printf("\t bytes received        : %0.0f\n",
			(double)context->socketMetrics.bytesReceived);
	printf("\t reads                 : %0.0f (%0.2f %%)\n",
			(double)context->socketMetrics.readsAttempted,
			(((double)context->socketMetrics.readsCompleted /
			  (double)context->socketMetrics.readsAttempted)
			 * 100.0));
	printf("\t writes                : %0.0f (%0.2f %%)\n",
			(double)context->socketMetrics.writesAttempted,
			(((double)context->socketMetrics.writesCompleted /
			  (double)context->socketMetrics.writesAttempted)
			 * 100.0));
	printf("\t total read time       : %0.6f seconds\n",
			(double)context->socketMetrics.totalReadTime);
	printf("\t total write time      : %0.6f seconds\n",
			(double)context->socketMetrics.totalWriteTime);

	mutex_unlock(&context->socketMetrics.mutex);
	mutex_free(&context->socketMetrics.mutex);

	/*
	 * TODO: create & display client-function execution time metrics
	 */

	// cleanup remaining context items

	bptree_free(&context->clientIndex);

	http_protocol_free(&context->httpProtocol);

	if(log_free(&context->log) < 0) {
		fprintf(stderr, "[%s():%i] error - failed here, aborting.\n",
				__FUNCTION__, __LINE__);
		mutex_unlock(&context->mutex);
		return -1;
	}

	mutex_unlock(&context->mutex);

	mutex_free(&context->mutex);

	// cleanup context

	memset(context, 0, (int)(sizeof(BeyBladeLoadTest)));

	free(context);

	context = NULL;

	return 0;
}

// message functions

static char *message_html_getPayloadPtr(char *message)
{
	const char *searchString = "\x0d\x0a\x0d\x0a";

	return strstr(message, searchString);
}

static char *message_html_buildLoginRequest(char *hostname, Client *client,
		int *messageLength)
{
	int length = 0;
	int payloadLength = 0;
	char *result = NULL;
	char *payload = NULL;

	const char *TEMPLATE = ""
		"POST /login-load-test.php HTTP/1.1\x0d\x0a"
		"Host: %s\x0d\x0a"
		"User-Agent: BeyBlade Load Test/%s (%s; en-US) Asgard/%s\x0d\x0a"
		"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\x0d\x0a"
		"Accept-Language: en-us,en;q=0.5\x0d\x0a"
		"Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7\x0d\x0a"
		"Keep-Alive: 115\x0d\x0a"
		"Connection: keep-alive\x0d\x0a"
		"Cache-Control: max-age=0\x0d\x0a"
		"Content-Type: application/x-www-form-urlencoded\x0d\x0a"
		"Content-Length: %i\x0d\x0a"
		"\x0d\x0a"
		"%s"
		"";

	const char *PAYLOAD_TEMPLATE = ""
		"login=true"
		"&"
		"username=%s"
		"&"
		"password=%s"
		"";

	payloadLength = (strlen(PAYLOAD_TEMPLATE) +
			strlen(client->http.userName) +
			strlen(client->http.password) +
			128);

	payload = (char *)malloc(sizeof(char) * (payloadLength + 8));

	payloadLength = snprintf(payload,
			(int)(sizeof(char) * payloadLength),
			PAYLOAD_TEMPLATE,
			client->http.userName,
			client->http.password);

	if(DEBUG_MODE) {
		displayString(DEBUG_STREAM, payload, payloadLength);
	}

	length = (strlen(TEMPLATE) +
			strlen(hostname) +
			payloadLength +
			128);

	result = (char *)malloc(sizeof(char) * (length + 8));

	*messageLength = snprintf(result,
			(int)(sizeof(char) * length),
			TEMPLATE,
			hostname,
			ASGARD_VERSION,
			ASGARD_VERSION,
			ASGARD_DATE,
			payloadLength,
			payload);

	free(payload);

	return result;
}

static aboolean message_html_isLoginValid(Client *client, char *message,
		int messageLength)
{
	int stringLength = 0;
	char *ptr = NULL;
	char *string = NULL;

	Xpath xpath;

	if((ptr = message_html_getPayloadPtr(message)) == NULL) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		return afalse;
	}

	if(xpath_init(&xpath) < 0) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		return afalse;
	}

	if(xpath_parseXmlDocument(&xpath, ptr,
				(messageLength - (int)(ptr - message))) < 0) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		return afalse;
	}

	if(DEBUG_MODE) {
		xpath_display(DEBUG_STREAM, &xpath);
	}

	// obtain user id

	string = xpath_getString(&xpath,
			"//beybladeLoginResponse/response",
			strlen("//beybladeLoginResponse/response"),
			"userId",
			strlen("userId"),
			&stringLength);

	if((string == NULL) || (stringLength < 1)) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		return afalse;
	}

	if(client->http.userId != NULL) {
		free(client->http.userId);
	}

	client->http.userId = string;

	// obtain session id

	string = xpath_getString(&xpath,
			"//beybladeLoginResponse/response",
			strlen("//beybladeLoginResponse/response"),
			"sessionId",
			strlen("sessionId"),
			&stringLength);

	if((string == NULL) || (stringLength < 1)) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		return afalse;
	}

	if(client->http.sessionId != NULL) {
		free(client->http.sessionId);
	}

	client->http.sessionId = string;

	// obtain player token

	string = xpath_getString(&xpath,
			"//beybladeLoginResponse/response",
			strlen("//beybladeLoginResponse/response"),
			"playerToken",
			strlen("playerToken"),
			&stringLength);

	if((string == NULL) || (stringLength < 1)) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		return afalse;
	}

	if(client->http.playerToken != NULL) {
		free(client->http.playerToken);
	}

	client->http.playerToken = string;

	if(xpath_free(&xpath) < 0) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		return afalse;
	}

	return atrue;
}

static aboolean message_sfs_enqueueAdditionalMessages(Client *client,
		char *message, int messageLength)
{
	int ii = 0;
	int counter = 0;
	int bufferRef = 0;
	int bufferLength = 0;
	char *copy = NULL;
	char *buffer = NULL;

	ClientMessage *item = NULL;

	// split multi-messages sent by SmartFox using delimiter (NULL)

	bufferRef = 0;
	bufferLength = 1024;
	buffer = (char *)malloc(sizeof(char) * (bufferLength + 1));

	for(ii = 0; ii < messageLength; ii++) {
		if(((unsigned int)message[ii] < 32) ||
				((unsigned int)message[ii] > 126)) {
			if(message[ii] != '\0') {
				continue;
			}
		}

		if(message[ii] == '\0') {
			buffer[bufferRef] = '\0';

			if(counter > 0) {
				if(DEBUG_MODE) {
					displayString(DEBUG_STREAM, buffer, bufferRef);
				}

				copy = strndup(buffer, bufferRef);

				if((item = clientMessage_new(copy, bufferRef)) != NULL) {
					if(fifostack_push(&client->messageQueue, item) < 0) {
						clientMessage_free(item);
					}
				}
				else {
					free(copy);
				}
			}

			bufferRef = 0;
			memset(buffer, 0, bufferLength);

			counter++;

			continue;
		}

		buffer[bufferRef] = message[ii];
		bufferRef++;

		if(bufferRef >= bufferLength) {
			bufferLength *= 2;
			buffer = (char *)realloc(buffer,
					(sizeof(char) * (bufferLength + 1)));
		}
	}

	free(buffer);

	return atrue;
}

static aboolean message_xml_isResponseASystemMessage(Xpath *xpath)
{
	int ii = 0;
	int validationCount = 0;
	int attributesLength = 0;

	XpathAttributes *attributes = NULL;

	static int pathLength = 0;
	static int nameLength = 0;
	const char *path = "//msg";
	const char *name = "msg";

	if(pathLength == 0) {
		pathLength = strlen(path);
	}

	if(nameLength == 0) {
		nameLength = strlen(name);
	}

	attributes = xpath_getAttributes(xpath, (char *)path, pathLength,
			(char *)name, nameLength, &attributesLength);

	if((attributes == NULL) || (attributesLength == 0)) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		return afalse;
	}

	for(ii = 0; ii < attributesLength; ii++) {
		if((!strcmp(attributes[ii].name, "t")) &&
				(!strcmp(attributes[ii].value, "sys"))) {
			validationCount++;
		}
	}

	if(validationCount != 1) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		return afalse;
	}

	return atrue;
}

static aboolean message_xml_isResponseARoomListMessage(Xpath *xpath)
{
	int attributesLength = 0;

	XpathAttributes *attributes = NULL;

	static int pathLength = 0;
	static int nameLength = 0;
	const char *path = "//msg/body/rmList";
	const char *name = "rm";

	if(pathLength == 0) {
		pathLength = strlen(path);
	}

	if(nameLength == 0) {
		nameLength = strlen(name);
	}

	attributes = xpath_getAttributes(xpath, (char *)path, pathLength,
			(char *)name, nameLength, &attributesLength);

	if((attributes == NULL) || (attributesLength == 0)) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		return afalse;
	}

	return atrue;
}

static aboolean message_xml_isResponseAJoinOkMessage(Client *client,
		Xpath *xpath)
{
	int ii = 0;
	int attributesLength = 0;

	XpathAttributes *attributes = NULL;

	static int pathLength = 0;
	static int nameLength = 0;
	const char *path = "//msg/body/rmList";
	const char *name = "rm";

	if(pathLength == 0) {
		pathLength = strlen(path);
	}

	if(nameLength == 0) {
		nameLength = strlen(name);
	}

	attributes = xpath_getAttributes(xpath,
			"//msg/body",
			strlen("//msg/body"),
			"uLs",
			strlen("uLs"),
			&attributesLength);

	if((attributes == NULL) || (attributesLength == 0)) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		return afalse;
	}

	if(client->sfsGameServer.roomId != NULL) {
		free(client->sfsGameServer.roomId);
		client->sfsGameServer.roomId = NULL;
	}

	for(ii = 0; ii < attributesLength; ii++) {
		if(!strcmp(attributes[ii].name, "r")) {
			client->sfsGameServer.roomId = strdup(attributes[ii].value);
			break;
		}
	}

	if(client->sfsGameServer.roomId == NULL) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		return afalse;
	}

	return atrue;
}

static char *message_xml_buildPolicyFileRequest(int *messageLength)
{
	int length = 0;
	char *result = NULL;

	const char *TEMPLATE = ""
		"<policy-file-request/>"
		"";

	length = (strlen(TEMPLATE) + 1);
	result = (char *)malloc(sizeof(char) * (length + 8));

	*messageLength = snprintf(result,
			(int)(sizeof(char) * length),
			TEMPLATE);

	if((*messageLength) > 0) {
		(*messageLength) += 1;
	}

	return result;
}

static aboolean message_xml_isPolicyFileResponseValid(char *message,
		int messageLength, aboolean isLobby)
{
	int ii = 0;
	int validationCount = 0;
	int attributesLength = 0;
	char *portComparison = NULL;

	Xpath xpath;
	XpathAttributes *attributes = NULL;

	static int pathLength = 0;
	static int nameLength = 0;
	const char *path = "//cross-domain-policy";
	const char *name = "allow-access-from";

	if(xpath_init(&xpath) < 0) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		return afalse;
	}

	if(xpath_parseXmlDocument(&xpath, message, messageLength) < 0) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		return afalse;
	}

	if(DEBUG_MODE) {
		xpath_display(DEBUG_STREAM, &xpath);
	}

	if(pathLength == 0) {
		pathLength = strlen(path);
	}

	if(nameLength == 0) {
		nameLength = strlen(name);
	}

	attributes = xpath_getAttributes(&xpath, (char *)path, pathLength,
			(char *)name, nameLength, &attributesLength);

	if((attributes == NULL) || (attributesLength == 0)) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		return afalse;
	}

	if(isLobby) {
		portComparison = SFS_LOBBY_SERVER_PORT_STRING;
	}
	else {
		portComparison = SFS_GAME_SERVER_PORT_STRING;
	}

	for(ii = 0; ii < attributesLength; ii++) {
		if((!strcmp(attributes[ii].name, "domain")) &&
				(!strcmp(attributes[ii].value, "*"))) {
			validationCount++;
		}
		else if((!strcmp(attributes[ii].name, "to-ports")) &&
				(!strcmp(attributes[ii].value, portComparison))) {
			validationCount++;
		}
	}

	if(xpath_free(&xpath) < 0) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		return afalse;
	}

	if(validationCount != 2) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		return afalse;
	}

	return atrue;
}

static char *message_xml_buildVersionCheck(int *messageLength)
{
	int length = 0;
	char *result = NULL;

	const char *TEMPLATE = ""
		"<msg t='sys'><body action='verChk' r='0'><ver v='165' /></body></msg>"
		"";

	length = (strlen(TEMPLATE) + 1);
	result = (char *)malloc(sizeof(char) * (length + 8));

	*messageLength = snprintf(result,
			(int)(sizeof(char) * length),
			TEMPLATE);

	if((*messageLength) > 0) {
		(*messageLength) += 1;
	}

	return result;
}

static aboolean message_xml_isVersionCheckResponseValid(char *message,
		int messageLength, aboolean isLobby)
{
	int ii = 0;
	int validationCount = 0;
	int attributesLength = 0;

	Xpath xpath;
	XpathAttributes *attributes = NULL;

	static int pathLength = 0;
	static int nameLength = 0;
	const char *path = "//msg";
	const char *name = "body";

	if(xpath_init(&xpath) < 0) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		return afalse;
	}

	if(xpath_parseXmlDocument(&xpath, message, messageLength) < 0) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		xpath_free(&xpath);
		return afalse;
	}

	if(DEBUG_MODE) {
		xpath_display(DEBUG_STREAM, &xpath);
	}

	if(!message_xml_isResponseASystemMessage(&xpath)) {
		if(isLobby) {
			log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
					__FUNCTION__, __LINE__);
			xpath_free(&xpath);
			return afalse;
		}
		else {
			log_logf(&context->log, LOG_LEVEL_WARNING,
					"%s() failed to locate 'apiOK' in game-server response",
					__FUNCTION__, __LINE__);

			if(xpath_free(&xpath) < 0) {
				log_logf(&context->log, LOG_LEVEL_ERROR,
						"%s() failed at line %i",
						__FUNCTION__, __LINE__);
				return afalse;
			}

			return atrue;
		}
	}

	if(pathLength == 0) {
		pathLength = strlen(path);
	}

	if(nameLength == 0) {
		nameLength = strlen(name);
	}

	attributes = xpath_getAttributes(&xpath, (char *)path, pathLength,
			(char *)name, nameLength, &attributesLength);

	if((attributes == NULL) || (attributesLength == 0)) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		xpath_free(&xpath);
		return afalse;
	}

	for(ii = 0; ii < attributesLength; ii++) {
		if((!strcmp(attributes[ii].name, "action")) &&
				(!strcmp(attributes[ii].value, "apiOK"))) {
			validationCount++;
		}
		else if((!strcmp(attributes[ii].name, "r")) &&
				(!strcmp(attributes[ii].value, "0"))) {
			validationCount++;
		}
	}

	if(xpath_free(&xpath) < 0) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		return afalse;
	}

	if(validationCount != 2) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		return afalse;
	}

	return atrue;
}

static char *message_xml_buildSfsServerLogin(char *encodedUserName,
		int *messageLength)
{
	int length = 0;
	char *result = NULL;

	const char *TEMPLATE = ""
		"<msg t='sys'>"
		"<body action='login' r='0'>"
		"<login z='BeyLobby'>"
		"<nick><![CDATA[%s]]></nick>"
		"<pword><![CDATA[]]></pword>"
		"</login>"
		"</body>"
		"</msg>"
		"";

	length = (strlen(TEMPLATE) +
			strlen(encodedUserName) +
			1);

	result = (char *)malloc(sizeof(char) * (length + 8));

	*messageLength = snprintf(result,
			(int)(sizeof(char) * length),
			TEMPLATE,
			encodedUserName);

	if((*messageLength) > 0) {
		(*messageLength) += 1;
	}

	return result;
}

static aboolean message_json_isSfsServerLoginResponseValid(Client *client,
		char *message, int messageLength, aboolean isLobby)
{
	int length = 0;
	double id = 0.0;
	char *string = NULL;

	Json *object = NULL;
	Json *bodyObject = NULL;
	Json *payloadObject = NULL;

	if((object = json_newFromString(message)) == NULL) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		return afalse;
	}

	if(DEBUG_MODE) {
		string = json_toString(object, &length);

		if(string != NULL) {
			fprintf(DEBUG_STREAM, "JSON(%i)::\n%s", length, string);

			free(string);
		}
	}

	if((bodyObject = json_getObject(object, "b")) == NULL) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		return afalse;
	}

	if((payloadObject = json_getObject(bodyObject, "o")) == NULL) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		return afalse;
	}

	id = json_getNumber(payloadObject, "id");

	if(isLobby) {
		client->sfsLobby.userId = (char *)malloc(sizeof(char) * 128);

		snprintf(client->sfsLobby.userId, (int)(sizeof(char) * 128),
				"%0.0f", id);
	}
	else {
		client->sfsGameServer.userId = (char *)malloc(sizeof(char) * 128);

		snprintf(client->sfsGameServer.userId, (int)(sizeof(char) * 128),
				"%0.0f", id);
	}

	if((string = json_getString(payloadObject, "name")) == NULL) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		return afalse;
	}

	if(isLobby) {
		client->sfsLobby.userName = strdup(string);
	}
	else {
		client->sfsGameServer.userName = strdup(string);
	}

	if(json_freePtr(object) < 0) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		return afalse;
	}

	return atrue;
}

static char *message_xml_buildGetRoomList(int *messageLength)
{
	int length = 0;
	char *result = NULL;

	const char *TEMPLATE = ""
		"<msg t='sys'><body action='getRmList' r='-1'></body></msg>"
		"";

	length = (strlen(TEMPLATE) +
			1);

	result = (char *)malloc(sizeof(char) * (length + 8));

	*messageLength = snprintf(result,
			(int)(sizeof(char) * length),
			TEMPLATE);

	if((*messageLength) > 0) {
		(*messageLength) += 1;
	}

	return result;
}

static aboolean message_xml_isGetRoomListResponseValid(Client *client,
		char *message, int messageLength)
{
	int stringLength = 0;
	char *string = NULL;

	Xpath xpath;

	static int pathLength = 0;
	static int nameLength = 0;
	const char *path = "//msg/body/rmList/rm";
	const char *name = "n";

	if(xpath_init(&xpath) < 0) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		return afalse;
	}

	if(xpath_parseXmlDocument(&xpath, message, messageLength) < 0) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		xpath_free(&xpath);
		return afalse;
	}

	if(DEBUG_MODE) {
		xpath_display(DEBUG_STREAM, &xpath);
	}

	if(!message_xml_isResponseASystemMessage(&xpath)) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		xpath_free(&xpath);
		return afalse;
	}

	if(pathLength == 0) {
		pathLength = strlen(path);
	}

	if(nameLength == 0) {
		nameLength = strlen(name);
	}

	string = xpath_getString(&xpath, (char *)path, pathLength, (char *)name,
			nameLength, &stringLength);

	if((string == NULL) || (stringLength < 1)) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		return afalse;
	}

	if(client->sfsLobby.roomName != NULL) {
		free(client->sfsLobby.roomName);
	}

	client->sfsLobby.roomName = string;

	if(xpath_free(&xpath) < 0) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		return afalse;
	}

	return atrue;
}

static char *message_json_buildJoinRoom(char *oldRoomName, char *newRoomName,
		int *messageLength)
{
	int length = 0;
	char *result = NULL;

	const char *TEMPLATE = ""
		"{\"b\":{"
		"\"r\":-1,"
		"\"x\":\"clobby\","
		"\"c\":\"joinRoom\","
		"\"p\":{\"oldRoom\":\"%s\",\"newRoom\":\"%s\"}"
		"},\"t\":\"xt\"}"
		"";

	length = (strlen(TEMPLATE) +
			strlen(oldRoomName) +
			strlen(newRoomName) +
			1);

	result = (char *)malloc(sizeof(char) * (length + 8));

	*messageLength = snprintf(result,
			(int)(sizeof(char) * length),
			TEMPLATE,
			oldRoomName,
			newRoomName);

	if((*messageLength) > 0) {
		(*messageLength) += 1;
	}

	return result;
}

static aboolean message_json_isJoinRoomResponseValid(Client *client,
		char *message, int messageLength)
{
	int length = 0;
	char *string = NULL;

	Json *object = NULL;

	if((object = json_newFromString(message)) == NULL) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		return afalse;
	}

	if(DEBUG_MODE) {
		string = json_toString(object, &length);

		if(string != NULL) {
			fprintf(DEBUG_STREAM, "JSON(%i)::\n%s", length, string);

			free(string);
		}
	}

	if(!client_updateOpponentListFromJson(client,
				object,
				atrue,		// is join room response
				atrue		// rebuild the opponent list
				)) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		return afalse;
	}

	if(json_freePtr(object) < 0) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		return afalse;
	}

	return atrue;
}

static char *message_json_buildEnableAutoBattle(int *messageLength)
{
	int length = 0;
	char *result = NULL;

	const char *TEMPLATE = ""
		"{\"b\":{"
		"\"r\":-1,\"x\":\"clobby\","
		"\"c\":\"enableAutoBattle\",\"p\":{}},\"t\":\"xt\"}"
		"";

	length = (strlen(TEMPLATE) +
			1);

	result = (char *)malloc(sizeof(char) * (length + 8));

	*messageLength = snprintf(result,
			(int)(sizeof(char) * length),
			TEMPLATE);

	if((*messageLength) > 0) {
		(*messageLength) += 1;
	}

	return result;
}

static char *message_json_buildChangeAutoBattleParams(int *messageLength)
{
	int length = 0;
	char *result = NULL;

	const char *TEMPLATE = ""
		"{\"b\":{"
		"\"r\":-1,\"x\":\"clobby\","
		"\"c\":\"changeAutoBattleParams\",\"p\":{}},\"t\":\"xt\"}"
		"";

	length = (strlen(TEMPLATE) +
			1);

	result = (char *)malloc(sizeof(char) * (length + 8));

	*messageLength = snprintf(result,
			(int)(sizeof(char) * length),
			TEMPLATE);

	if((*messageLength) > 0) {
		(*messageLength) += 1;
	}

	return result;
}

static char *message_json_buildCreateGame(char *opponentName,
		int *messageLength)
{
	int length = 0;
	char *result = NULL;

	const char *TEMPLATE = ""
		"{\"b\":"
		"{"
		"\"r\":-1,"
		"\"x\":\"clobby\","
		"\"c\":\"createGame\","
		"\"p\":{\"opponentName\":\"%s\"}"
		"},"
		"\"t\":\"xt\"}"
		"";

	length = (strlen(TEMPLATE) +
			strlen(opponentName) +
			1);

	result = (char *)malloc(sizeof(char) * (length + 8));

	*messageLength = snprintf(result,
			(int)(sizeof(char) * length),
			TEMPLATE,
			opponentName);

	if((*messageLength) > 0) {
		(*messageLength) += 1;
	}

	return result;
}

static char *message_json_buildIsBattleReady(char *roomId, char *gameType,
		int *messageLength)
{
	int length = 0;
	char *result = NULL;

	const char *TEMPLATE = ""
		"{\"b\":{\"r\":%s,\"x\":\"%s\",\"c\":\"ready\",\"p\":{}},\"t\":\"xt\"}"
		"";

	length = (strlen(TEMPLATE) +
			strlen(roomId) +
			strlen(gameType) +
			1);

	result = (char *)malloc(sizeof(char) * (length + 8));

	*messageLength = snprintf(result,
			(int)(sizeof(char) * length),
			TEMPLATE,
			roomId,
			gameType);

	if((*messageLength) > 0) {
		(*messageLength) += 1;
	}

	return result;
}

static char *message_json_buildIsBattleInitialized(char *roomId,
		char *gameType, int *messageLength)
{
	int length = 0;
	char *result = NULL;

	const char *TEMPLATE = ""
		"{\"b\":{"
		"\"r\":%s,\"x\":\"%s\",\"c\":\"initialized\",\"p\":{}},\"t\":\"xt\"}"
		"";

	length = (strlen(TEMPLATE) +
			strlen(roomId) +
			strlen(gameType) +
			1);

	result = (char *)malloc(sizeof(char) * (length + 8));

	*messageLength = snprintf(result,
			(int)(sizeof(char) * length),
			TEMPLATE,
			roomId,
			gameType);

	if((*messageLength) > 0) {
		(*messageLength) += 1;
	}

	return result;
}

static char *message_json_buildBattleSelection(Client *client,
		int *messageLength)
{
	int length = 0;
	int topListLength = 0;
	char *result = NULL;
	char *string = NULL;

	Json *bodyObject = NULL;
	Json *payloadObject = NULL;
	Json *playerObject = NULL;
	Json *topObject = NULL;

	const char *TEMPLATE = ""
		"{\"b\":{"
		"\"r\":%s,"
		"\"x\":\"%s\","
		"\"c\":\"selection\",\"p\":{\"id\":\"%s\"}},\"t\":\"xt\"}"
		"";

	if((bodyObject = json_getObject(client->sfsGameServer.sessionInfo,
					"b")) == NULL) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		return NULL;
	}

	if((payloadObject = json_getObject(bodyObject, "o")) == NULL) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		return NULL;
	}

	if((playerObject = json_getObject(payloadObject, "player")) == NULL) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		return NULL;
	}

	topListLength = json_getArrayLength(playerObject, "top");

	if(topListLength < 1) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		return NULL;
	}

	system_pickRandomSeed();

	client->sfsGameServer.topPick = (rand() % topListLength);

	if((topObject = json_getObjectFromArray(playerObject, "top",
					client->sfsGameServer.topPick)) == NULL) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		return NULL;
	}

	if((string = json_getString(topObject, "top_id")) == NULL) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		return NULL;
	}

	length = (strlen(TEMPLATE) +
			strlen(client->sfsGameServer.roomId) +
			strlen(client->sfsGameServer.gameType) +
			strlen(string) +
			1);

	result = (char *)malloc(sizeof(char) * (length + 8));

	*messageLength = snprintf(result,
			(int)(sizeof(char) * length),
			TEMPLATE,
			client->sfsGameServer.roomId,
			client->sfsGameServer.gameType,
			string);

	if((*messageLength) > 0) {
		(*messageLength) += 1;
	}

	return result;
}

static Json *message_json_messageToJson(Client *client, char *message,
		int messageLength, char **command)
{
	int length = 0;
	char *string = NULL;

	Json *object = NULL;
	Json *bodyObject = NULL;
	Json *payloadObject = NULL;

	*command = NULL;

	if((message == NULL) || (messageLength < 1)) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		return NULL;
	}

	if(message[0] != '{') {
		return NULL;
	}

	if((object = json_newFromString(message)) == NULL) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		return NULL;
	}

	if(DEBUG_MODE) {
		string = json_toString(object, &length);

		if(string != NULL) {
			fprintf(DEBUG_STREAM, "JSON(%i)::\n%s", length, string);

			free(string);
		}
	}

	if((bodyObject = json_getObject(object, "b")) == NULL) {
		json_freePtr(object);
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		return NULL;
	}

	if((payloadObject = json_getObject(bodyObject, "o")) == NULL) {
		json_freePtr(object);
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		return NULL;
	}

	if((string = json_getString(payloadObject, "_cmd")) == NULL) {
		json_freePtr(object);
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		return NULL;
	}

	(*command) = strdup(string);

	return object;
}

static char *message_json_getStringFrommessage(Json *object, char *key)
{
	char *string = NULL;
	char *result = NULL;

	Json *bodyObject = NULL;
	Json *payloadObject = NULL;

	if((bodyObject = json_getObject(object, "b")) == NULL) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		return NULL;
	}

	if((payloadObject = json_getObject(bodyObject, "o")) == NULL) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		return NULL;
	}

	if((string = json_getString(payloadObject, key)) == NULL) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		return NULL;
	}

	result = strdup(string);

	return result;
}

static Xpath *message_xpath_messageToXpath(Client *client, char *message,
		int messageLength)
{
	Xpath *xpath = NULL;

	if((message == NULL) || (messageLength < 1)) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		return NULL;
	}

	if(message[0] != '<') {
		return NULL;
	}

	xpath = (Xpath *)malloc(sizeof(Xpath));

	if(xpath_init(xpath) < 0) {
		free(xpath);
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		return NULL;
	}

	if(xpath_parseXmlDocument(xpath, message, messageLength) < 0) {
		xpath_free(xpath);
		free(xpath);
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		return NULL;
	}

	if(DEBUG_MODE) {
		xpath_display(DEBUG_STREAM, xpath);
	}

	return xpath;
}

// client connection functions

static int clientConnection_init(ClientConnection *client, char *hostname,
		int port)
{
	mutex_init(&client->mutex);

	mutex_lock(&client->mutex);

	if(socket_init(&client->socket,
				SOCKET_MODE_CLIENT,
				SOCKET_PROTOCOL_TCPIP,
				hostname,
				port
				) < 0) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		mutex_unlock(&client->mutex);
		return -1;
	}

	if(socket_open(&client->socket) < 0) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		mutex_unlock(&client->mutex);
		return -1;
	}

	if((client->sd = socket_getSocketDescriptor(&client->socket)) < 0) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		mutex_unlock(&client->mutex);
		return -1;
	}

	client->port = port;
	client->hostname = strdup(hostname);

	log_logf(&context->log, LOG_LEVEL_DEBUG,
			"client opened connection to '%s:%i' with fd #%i",
			client->hostname, client->port, client->sd);

	mutex_unlock(&client->mutex);

	return 0;
}

static int clientConnection_free(ClientConnection *client)
{
	int result = 0;

	mutex_lock(&client->mutex);

	if(socket_close(&client->socket) < 0) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		result = -1;
	}

	socket_free(&client->socket);

	log_logf(&context->log, LOG_LEVEL_DEBUG,
			"client closed connection to '%s:%i' on fd #%i",
			client->hostname, client->port, client->sd);

	free(client->hostname);

	mutex_unlock(&client->mutex);

	mutex_free(&client->mutex);

	return result;
}

static int clientConnection_sendMessage(ClientConnection *client,
		char *message, int messageLength)
{
	// send message from client to server

	if((message != NULL) && (messageLength > 0)) {
		if(socket_send(&client->socket, client->sd, message,
					messageLength) < 0) {
			log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
					__FUNCTION__, __LINE__);
			return -1;
		}

		log_logf(&context->log, LOG_LEVEL_DEBUG,
				"client sent %i bytes to server '%s:%i' on fd #%i => '%s'",
				messageLength, client->hostname, client->port, client->sd,
				message);
	}

	// update socket metrics

	mutex_lock(&context->socketMetrics.mutex);

	context->socketMetrics.bytesSent +=
		socket_getBytesSent(&client->socket);

	context->socketMetrics.bytesReceived +=
		socket_getBytesReceived(&client->socket);

	context->socketMetrics.readsAttempted +=
		socket_getReadsAttempted(&client->socket);

	context->socketMetrics.readsCompleted +=
		socket_getReadsCompleted(&client->socket);

	context->socketMetrics.writesAttempted +=
		socket_getWritesAttempted(&client->socket);

	context->socketMetrics.writesCompleted +=
		socket_getWritesCompleted(&client->socket);

	context->socketMetrics.totalReadTime +=
		socket_getTotalReadTime(&client->socket);

	context->socketMetrics.totalWriteTime +=
		socket_getTotalWriteTime(&client->socket);

	mutex_unlock(&context->socketMetrics.mutex);

	return 0;
}

static char *clientConnection_listenForMessage(ClientConnection *client,
		int *responseLength)
{
	int rc = 0;
	int counter = 0;
	int emptyCounter = 0;
	int resultRef = 0;
	int resultLength = 0;
	double timer = 0.0;
	double timeout = 0.0;
	double elapsedTime = 0.0;
	double remainingTime = 0.0;
	char chunk[4];
	char *result = NULL;

	*responseLength = 0;

	// check for a incoming message

	timeout = 0.001024;

	memset(chunk, 0, (int)sizeof(chunk));

	if((rc = socket_receive(&client->socket,
					client->sd,
					chunk,
					1,
					timeout)) <= 0) {
		return NULL;
	}

	// read server socket transmission

	resultRef = 0;
	resultLength = 1024;
	result = (char *)malloc(sizeof(char) * resultLength);

	memcpy((result + resultRef), chunk, rc);
	resultRef += rc;

	counter = 0;
	emptyCounter = 0;

	timer = 0.0;
	timeout = 4.096;
	elapsedTime = 0.0;
	remainingTime = 4.096;

	memset(chunk, 0, (int)sizeof(chunk));

	do {
		timer = time_getTimeMus();

		if((rc = socket_receive(&client->socket,
						client->sd,
						chunk,
						1,
						timeout)) < 0) {
			break;
		}

		elapsedTime = time_getElapsedMusInSeconds(timer);

		if(elapsedTime < timeout) {
			timeout = elapsedTime;
		}

		remainingTime -= elapsedTime;

		if(rc > 0) {
			memcpy((result + resultRef), chunk, rc);
			resultRef += rc;
			emptyCounter = 0;
		}
		else if(rc == 0) {
			emptyCounter++;
		}

		if(resultRef >= resultLength) {
			resultLength *= 2;
			result = (char *)realloc(result, (sizeof(char) * resultLength));
		}

		if(emptyCounter >= SFS_PROTOCOL_EMPTY_COUNT_THRESHOLD) {
			break;
		}
	} while(remainingTime > 0.0);

	if(resultRef == 0) {
		resultRef = 0;
		resultLength = 0;
		free(result);
		result = NULL;
	}
	else {
		result[resultRef] = '\0';
	}

	log_logf(&context->log, LOG_LEVEL_DEBUG,
			"client received %i bytes from server '%s:%i' on fd #%i => '%s'",
			resultRef, client->hostname, client->port, client->sd, result);

	// update socket metrics

	mutex_lock(&context->socketMetrics.mutex);

	context->socketMetrics.bytesSent +=
		socket_getBytesSent(&client->socket);

	context->socketMetrics.bytesReceived +=
		socket_getBytesReceived(&client->socket);

	context->socketMetrics.readsAttempted +=
		socket_getReadsAttempted(&client->socket);

	context->socketMetrics.readsCompleted +=
		socket_getReadsCompleted(&client->socket);

	context->socketMetrics.writesAttempted +=
		socket_getWritesAttempted(&client->socket);

	context->socketMetrics.writesCompleted +=
		socket_getWritesCompleted(&client->socket);

	context->socketMetrics.totalReadTime +=
		socket_getTotalReadTime(&client->socket);

	context->socketMetrics.totalWriteTime +=
		socket_getTotalWriteTime(&client->socket);

	mutex_unlock(&context->socketMetrics.mutex);

	*responseLength = resultRef;

	return result;
}

static char *clientConnection_executeTransaction(ClientConnection *client,
		char *message, int messageLength, int *responseLength)
{
	int rc = 0;
	int counter = 0;
	int emptyCounter = 0;
	int resultRef = 0;
	int resultLength = 0;
	double timer = 0.0;
	double timeout = 0.0;
	double elapsedTime = 0.0;
	double remainingTime = 0.0;
	char chunk[4];
	char *result = NULL;

	*responseLength = 0;

	// send message from client to server

	if((message != NULL) && (messageLength > 0)) {
		if(socket_send(&client->socket, client->sd, message,
					messageLength) < 0) {
			log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
					__FUNCTION__, __LINE__);
			return NULL;
		}

		log_logf(&context->log, LOG_LEVEL_DEBUG,
				"client sent %i bytes to server '%s:%i' on fd #%i => '%s'",
				messageLength, client->hostname, client->port, client->sd,
				message);
	}

	// read server socket transmission

	resultRef = 0;
	resultLength = 1024;
	result = (char *)malloc(sizeof(char) * resultLength);

	counter = 0;
	emptyCounter = 0;

	timer = 0.0;
	timeout = 4.096;
	elapsedTime = 0.0;
	remainingTime = 4.096;

	memset(chunk, 0, (int)sizeof(chunk));

	do {
		timer = time_getTimeMus();

		if((rc = socket_receive(&client->socket,
						client->sd,
						chunk,
						1,
						timeout)) < 0) {
			break;
		}

		elapsedTime = time_getElapsedMusInSeconds(timer);

		if(elapsedTime < timeout) {
			timeout = elapsedTime;
		}

		remainingTime -= elapsedTime;

		if(rc > 0) {
			memcpy((result + resultRef), chunk, rc);
			resultRef += rc;
			emptyCounter = 0;
		}
		else if(rc == 0) {
			emptyCounter++;
		}

		if(resultRef >= resultLength) {
			resultLength *= 2;
			result = (char *)realloc(result, (sizeof(char) * resultLength));
		}

		if(emptyCounter >= SFS_PROTOCOL_EMPTY_COUNT_THRESHOLD) {
			break;
		}
	} while(remainingTime > 0.0);

	if(resultRef == 0) {
		resultRef = 0;
		resultLength = 0;
		free(result);
		result = NULL;
	}
	else {
		result[resultRef] = '\0';
	}

	log_logf(&context->log, LOG_LEVEL_DEBUG,
			"client received %i bytes from server '%s:%i' on fd #%i => '%s'",
			resultRef, client->hostname, client->port, client->sd, result);

	// update socket metrics

	mutex_lock(&context->socketMetrics.mutex);

	context->socketMetrics.bytesSent +=
		socket_getBytesSent(&client->socket);

	context->socketMetrics.bytesReceived +=
		socket_getBytesReceived(&client->socket);

	context->socketMetrics.readsAttempted +=
		socket_getReadsAttempted(&client->socket);

	context->socketMetrics.readsCompleted +=
		socket_getReadsCompleted(&client->socket);

	context->socketMetrics.writesAttempted +=
		socket_getWritesAttempted(&client->socket);

	context->socketMetrics.writesCompleted +=
		socket_getWritesCompleted(&client->socket);

	context->socketMetrics.totalReadTime +=
		socket_getTotalReadTime(&client->socket);

	context->socketMetrics.totalWriteTime +=
		socket_getTotalWriteTime(&client->socket);

	mutex_unlock(&context->socketMetrics.mutex);

	*responseLength = resultRef;

	return result;
}

static char *clientConnection_executeHttpTransaction(ClientConnection *client,
		char *message, int messageLength, int *responseLength)
{
	int resultLength = 0;
	char *result = NULL;

	*responseLength = 0;

	// send message from client to server

	if((message != NULL) && (messageLength > 0)) {
		if(http_protocol_send(&context->httpProtocol, &client->socket,
					client->sd, message, messageLength) < 0) {
			log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
					__FUNCTION__, __LINE__);
			return NULL;
		}

		log_logf(&context->log, LOG_LEVEL_DEBUG,
				"client sent %i bytes to server '%s:%i' on fd #%i => '%s'",
				messageLength, client->hostname, client->port, client->sd,
				message);
	}

	// read server socket transmission

	result = http_protocol_receive(&context->httpProtocol, &client->socket,
			client->sd, &resultLength);

	log_logf(&context->log, LOG_LEVEL_DEBUG,
			"client received %i bytes from server '%s:%i' on fd #%i => '%s'",
			resultLength, client->hostname, client->port, client->sd, result);

	// update socket metrics

	mutex_lock(&context->socketMetrics.mutex);

	context->socketMetrics.bytesSent +=
		socket_getBytesSent(&client->socket);

	context->socketMetrics.bytesReceived +=
		socket_getBytesReceived(&client->socket);

	context->socketMetrics.readsAttempted +=
		socket_getReadsAttempted(&client->socket);

	context->socketMetrics.readsCompleted +=
		socket_getReadsCompleted(&client->socket);

	context->socketMetrics.writesAttempted +=
		socket_getWritesAttempted(&client->socket);

	context->socketMetrics.writesCompleted +=
		socket_getWritesCompleted(&client->socket);

	context->socketMetrics.totalReadTime +=
		socket_getTotalReadTime(&client->socket);

	context->socketMetrics.totalWriteTime +=
		socket_getTotalWriteTime(&client->socket);

	mutex_unlock(&context->socketMetrics.mutex);

	*responseLength = resultLength;

	return result;
}

// client message functions

static ClientMessage *clientMessage_new(char *message, int messageLength)
{
	ClientMessage *result = NULL;

	result = (ClientMessage *)malloc(sizeof(ClientMessage));

	result->length = messageLength;
	result->body = message;

	return result;
}

static void clientMessage_free(void *memory)
{
	ClientMessage *message = NULL;

	if(memory == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	message = (ClientMessage *)memory;

	if(message->body != NULL) {
		free(message->body);
	}

	memset(message, 0, (int)(sizeof(ClientMessage)));

	free(message);
}

// client functions

static void client_init(Client *client)
{
	memset(client, 0, (int)(sizeof(Client)));

	client->status = CLIENT_STATUS_INIT;

	mutex_init(&client->mutex);

	mutex_lock(&client->mutex);

	fifostack_init(&client->messageQueue);
	fifostack_setFreeFunction(&client->messageQueue, clientMessage_free);

	mutex_unlock(&client->mutex);
}

static int client_setProperties(Client *client, char *string, int stringLength)
{
	char *value = NULL;
	void *temp = NULL;

	mutex_lock(&client->mutex);

	value = getPropertyFromString(string, stringLength, 2, '\t');

	if(value != NULL) {
		if(bptree_get(&context->clientIndex, value, strlen(value),
					&temp) == 0) {
			mutex_unlock(&client->mutex);
			log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
					__FUNCTION__, __LINE__);
			return -1;
		}

		if(client->http.userName != NULL) {
			free(client->http.userName);
		}

		client->http.userName = value;
	}
	else {
		mutex_unlock(&client->mutex);
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		return -1;
	}

	value = getPropertyFromString(string, stringLength, 3, '\t');

	if(value != NULL) {
		if(client->http.password != NULL) {
			free(client->http.password);
		}

		client->http.password = value;
	}
	else {
		mutex_unlock(&client->mutex);
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		return -1;
	}

	if(bptree_put(&context->clientIndex,
				client->http.userName,
				strlen(client->http.userName),
				(void *)&client) < 0) {
		mutex_unlock(&client->mutex);
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		return -1;
	}

	mutex_unlock(&client->mutex);

	return 0;
}

static void client_free(Client *client)
{
	int ii = 0;

	mutex_lock(&client->mutex);

	if(client->http.userId != NULL) {
		free(client->http.userId);
	}

	if(client->http.userName != NULL) {
		free(client->http.userName);
	}

	if(client->http.password != NULL) {
		free(client->http.password);
	}

	if(client->http.sessionId != NULL) {
		free(client->http.sessionId);
	}

	if(client->http.playerToken != NULL) {
		free(client->http.playerToken);
	}

	if(client->sfsLobby.userId != NULL) {
		free(client->sfsLobby.userId);
	}

	if(client->sfsLobby.userName != NULL) {
		free(client->sfsLobby.userName);
	}

	if(client->sfsLobby.roomName != NULL) {
		free(client->sfsLobby.roomName);
	}

	if(client->sfsLobby.opponents != NULL) {
		for(ii = 0; ii < client->sfsLobby.opponentLength; ii++) {
			if(client->sfsLobby.opponents[ii].artName != NULL) {
				free(client->sfsLobby.opponents[ii].artName);
			}
			if(client->sfsLobby.opponents[ii].level != NULL) {
				free(client->sfsLobby.opponents[ii].level);
			}
			if(client->sfsLobby.opponents[ii].name != NULL) {
				free(client->sfsLobby.opponents[ii].name);
			}
			if(client->sfsLobby.opponents[ii].points != NULL) {
				free(client->sfsLobby.opponents[ii].points);
			}
		}

		free(client->sfsLobby.opponents);
	}

	if(client->sfsGameServer.userId != NULL) {
		free(client->sfsGameServer.userId);
	}

	if(client->sfsGameServer.userName != NULL) {
		free(client->sfsGameServer.userName);
	}

	if(client->sfsGameServer.roomId != NULL) {
		free(client->sfsGameServer.roomId);
	}

	if(client->sfsGameServer.gameType != NULL) {
		free(client->sfsGameServer.gameType);
	}

	if(client->sfsGameServer.roomList != NULL) {
		xpath_free(client->sfsGameServer.roomList);
		free(client->sfsGameServer.roomList);
	}

	if(client->sfsGameServer.sessionInfo != NULL) {
		json_freePtr(client->sfsGameServer.sessionInfo);
	}

	fifostack_free(&client->messageQueue);

	mutex_unlock(&client->mutex);
	mutex_free(&client->mutex);

	memset(client, 0, (int)(sizeof(Client)));
}

static char *client_statusToString(Client *client)
{
	char *result = "unknown";

	switch(client->status) {
		case CLIENT_STATUS_INIT:
			result = "(1) initialized";
			break;

		case CLIENT_STATUS_IS_HTTP_LOGIN:
			result = "(2) is HTTP login";
			break;

		case CLIENT_STATUS_HAS_POLICY_FILE:
			result = "(3) has policy file";
			break;

		case CLIENT_STATUS_HAS_VERSION_CHECK:
			result = "(4) has version check";
			break;

		case CLIENT_STATUS_IS_LOBBY_LOGIN:
			result = "(5) is lobby login";
			break;

		case CLIENT_STATUS_IS_GET_ROOM_LIST:
			result = "(6) is get room list";
			break;

		case CLIENT_STATUS_IS_JOIN_ROOM:
			result = "(7) is join room";
			break;

		case CLIENT_STATUS_IS_ENABLE_AUTO_BATTLE:
			result = "(8) is enable auto battle";
			break;

		case CLIENT_STATUS_IS_CREATE_GAME:
			result = "(9) is create game";
			break;

		case CLIENT_STATUS_IS_GAME_READY:
			result = "(10) is game ready";
			break;

		case CLIENT_STATUS_HAS_GAME_SERVER_POLICY_FILE:
			result = "(11) has game server policy file";
			break;

		case CLIENT_STATUS_HAS_GAME_SERVER_VERSION_CHECK:
			result = "(12) has game server version check";
			break;

		case CLIENT_STATUS_IS_GAME_SERVER_LOGIN:
			result = "(13) is game server login";
			break;

		case CLIENT_STATUS_IS_GAME_SERVER_LOGIN_HAS_ROOM_LIST:
			result = "(14) is game server login";
			break;

		case CLIENT_STATUS_IS_GAME_SERVER_LOGIN_IS_JOIN_OK:
			result = "(15) is game server login";
			break;

		case CLIENT_STATUS_IS_GAME_SERVER_LOGIN_HAS_JOINED:
			result = "(16) is game server login";
			break;

		case CLIENT_STATUS_IS_GAME_SERVER_READY:
			result = "(17) is game server ready";
			break;

		case CLIENT_STATUS_IS_GAME_SERVER_WAITING:
			result = "(18) is game server waiting";
			break;

		case CLIENT_STATUS_IS_BATTLE_INIT:
			result = "(19) is battle initialized";
			break;

		case CLIENT_STATUS_IS_BATTLE_INIT_WAITING:
			result = "(20) is battle initialized (waiting)";
			break;

		case CLIENT_STATUS_IS_BATTLE_IN_PROGRESS:
			result = "(21) is battle in progress";
			break;

		case CLIENT_STATUS_ERROR:
			result = "(-1) error condition";
			break;
	}

	return result;
}

static void client_display(void *stream, Client *client)
{
	int ii = 0;
	int length = 0;
	char *string = NULL;

	fprintf(stream, "Client 0x%lx ::\n", (aptrcast)client);

	fprintf(stream, "\t status         : [%s]\n",
			client_statusToString(client));

	fprintf(stream, "\t HTTP ::\n");

	fprintf(stream, "\t\t user id      : '%s'\n", client->http.userId);
	fprintf(stream, "\t\t user name    : '%s'\n", client->http.userName);
	fprintf(stream, "\t\t password     : '%s'\n", client->http.password);
	fprintf(stream, "\t\t session id   : '%s'\n", client->http.sessionId);
	fprintf(stream, "\t\t player token : '%s'\n", client->http.playerToken);

	fprintf(stream, "\t SFS Lobby ::\n");

	fprintf(stream, "\t\t user id      : '%s'\n", client->sfsLobby.userId);
	fprintf(stream, "\t\t user name    : '%s'\n", client->sfsLobby.userName);
	fprintf(stream, "\t\t room name    : '%s'\n", client->sfsLobby.roomName);
	fprintf(stream, "\t\t opponents    ::\n");

	if(client->sfsLobby.opponents != NULL) {
		for(ii = 0; ii < client->sfsLobby.opponentLength; ii++) {
			fprintf(stream, "\t\t\t opponent %03i 'artName' :: '%s'\n", ii,
					client->sfsLobby.opponents[ii].artName);
			fprintf(stream, "\t\t\t opponent %03i 'level'   :: '%s'\n", ii,
					client->sfsLobby.opponents[ii].level);
			fprintf(stream, "\t\t\t opponent %03i 'name'    :: '%s'\n", ii,
					client->sfsLobby.opponents[ii].name);
			fprintf(stream, "\t\t\t opponent %03i 'points'  :: '%s'\n", ii,
					client->sfsLobby.opponents[ii].points);
		}
	}

	fprintf(stream, "\t SFS Game Server ::\n");

	fprintf(stream, "\t\t top pick     : %i\n",
			client->sfsGameServer.topPick);
	fprintf(stream, "\t\t user id      : '%s'\n",
			client->sfsGameServer.userId);
	fprintf(stream, "\t\t user name    : '%s'\n",
			client->sfsGameServer.userName);
	fprintf(stream, "\t\t room id      : '%s'\n",
			client->sfsGameServer.roomId);
	fprintf(stream, "\t\t game type    : '%s'\n",
			client->sfsGameServer.gameType);

	if(client->sfsGameServer.roomList != NULL) {
		xpath_display(stream, client->sfsGameServer.roomList);
	}

	if(client->sfsGameServer.sessionInfo != NULL) {
		string = json_toString(client->sfsGameServer.sessionInfo, &length);

		if(string != NULL) {
			fprintf(stream, "JSON(%i)::\n%s", length, string);

			free(string);
		}
	}

	fprintf(stream, "\t Connection ::\n");

	fprintf(stream, "\t\t hostname     : '%s'\n", client->connection.hostname);
	fprintf(stream, "\t\t port         : %i\n", client->connection.port);
	fprintf(stream, "\t\t sd           : %i\n", client->connection.sd);

	fprintf(stream, "\t Socket ::\n");

	if(client->connection.socket.state <= 0) {
		fprintf(stream, "\t\t status                : offline\n");
		return;
	}

	fprintf(stream, "\t\t mode                  : [%i]\n",
			(int)socket_getMode(&client->connection.socket));
	fprintf(stream, "\t\t protocol              : [%i]\n",
			(int)socket_getProtocol(&client->connection.socket));
	fprintf(stream, "\t\t reuse address         : %i\n",
			(int)socket_getReuseAddress(&client->connection.socket));
	fprintf(stream, "\t\t reuse port            : %i\n",
			(int)socket_getReusePort(&client->connection.socket));
	fprintf(stream, "\t\t use broadcast         : %i\n",
			(int)socket_getUseBroadcast(&client->connection.socket));
	fprintf(stream, "\t\t use write sync        : %i\n",
			(int)socket_getUseSocketWriteSync(&client->connection.socket));
	fprintf(stream, "\t\t use modulated timeout : %i\n",
			(int)socket_getUseModulatedTimeout(&client->connection.socket));
	fprintf(stream, "\t\t queue length          : %i\n",
			socket_getQueueLength(&client->connection.socket));
	fprintf(stream, "\t\t timeout millis        : %i\n",
			socket_getTimeoutMillis(&client->connection.socket));
	fprintf(stream, "\t\t bytes sent            : %0.0f\n",
			(double)socket_getBytesSent(&client->connection.socket));
	fprintf(stream, "\t\t bytes received        : %0.0f\n",
			(double)socket_getBytesReceived(&client->connection.socket));
	fprintf(stream, "\t\t reads                 : %0.0f (%0.2f %%)\n",
			(double)socket_getReadsAttempted(&client->connection.socket),
			(((double)socket_getReadsCompleted(&client->connection.socket) /
			  (double)socket_getReadsAttempted(&client->connection.socket))
			 * 100.0));
	fprintf(stream, "\t\t writes                : %0.0f (%0.2f %%)\n",
			(double)socket_getWritesAttempted(&client->connection.socket),
			(((double)socket_getWritesCompleted(&client->connection.socket) /
			  (double)socket_getWritesAttempted(&client->connection.socket))
			 * 100.0));
	fprintf(stream, "\t\t total read time       : %0.6f seconds\n",
			(double)socket_getTotalReadTime(&client->connection.socket));
	fprintf(stream, "\t\t total write time      : %0.6f seconds\n",
			(double)socket_getTotalWriteTime(&client->connection.socket));
}

static aboolean client_addToOpponentListFromJson(Client *client,
		Json *opponentObject)
{
	int ref = 0;
	char *string = NULL;

	if(client->sfsLobby.opponents == NULL) {
		ref = 0;
		client->sfsLobby.opponentLength = 1;
		client->sfsLobby.opponents = (ClientSfsLobbyOpponent *)malloc(
				sizeof(ClientSfsLobbyOpponent) *
				client->sfsLobby.opponentLength);
	}
	else {
		ref = client->sfsLobby.opponentLength;
		client->sfsLobby.opponentLength += 1;
		client->sfsLobby.opponents = (ClientSfsLobbyOpponent *)realloc(
				client->sfsLobby.opponents,
				(sizeof(ClientSfsLobbyOpponent) *
				 client->sfsLobby.opponentLength));
	}

	if((string = json_getString(opponentObject, "artName")) == NULL) {
		log_logf(&context->log, LOG_LEVEL_ERROR,
				"%s() failed at line %i",
				__FUNCTION__, __LINE__);
		return afalse;
	}

	client->sfsLobby.opponents[ref].artName = strdup(string);

	if((string = json_getString(opponentObject, "level")) == NULL) {
		log_logf(&context->log, LOG_LEVEL_ERROR,
				"%s() failed at line %i",
				__FUNCTION__, __LINE__);
		return afalse;
	}

	client->sfsLobby.opponents[ref].level = strdup(string);

	if((string = json_getString(opponentObject, "name")) == NULL) {
		log_logf(&context->log, LOG_LEVEL_ERROR,
				"%s() failed at line %i",
				__FUNCTION__, __LINE__);
		return afalse;
	}

	client->sfsLobby.opponents[ref].name = strdup(string);

	if((string = json_getString(opponentObject, "points")) == NULL) {
		log_logf(&context->log, LOG_LEVEL_ERROR,
				"%s() failed at line %i",
				__FUNCTION__, __LINE__);
		return afalse;
	}

	client->sfsLobby.opponents[ref].points = strdup(string);

	log_logf(&context->log, LOG_LEVEL_DEBUG,
			"client { lobby-id %s } noted opponent '%s' joined lobby '%s'",
			client->sfsLobby.userId,
			client->sfsLobby.opponents[ref].name,
			client->sfsLobby.roomName);

	return atrue;
}

static aboolean client_updateOpponentListFromJson(Client *client,
		Json *object, aboolean isJoinRoomResponse, aboolean rebuildList)
{
	int ii = 0;
	int length = 0;
	char *string = NULL;

	Json *bodyObject = NULL;
	Json *payloadObject = NULL;
	Json *opponentObject = NULL;

	if((bodyObject = json_getObject(object, "b")) == NULL) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		return afalse;
	}

	if((payloadObject = json_getObject(bodyObject, "o")) == NULL) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		return afalse;
	}

	if(isJoinRoomResponse) {
		if((string = json_getString(payloadObject, "result")) == NULL) {
			log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
					__FUNCTION__, __LINE__);
			return afalse;
		}

		if(strcmp(string, "OK")) {
			log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
					__FUNCTION__, __LINE__);
			return afalse;
		}
	}
	else {
		if((string = json_getString(payloadObject, "_cmd")) == NULL) {
			log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
					__FUNCTION__, __LINE__);
			return afalse;
		}

		if(strcmp(string, "userArrived")) {
			log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
					__FUNCTION__, __LINE__);
			return afalse;
		}
	}

	if(isJoinRoomResponse) {
		length = json_getArrayLength(payloadObject, "opponents");

		if((length > 0) &&
				(rebuildList) &&
				(client->sfsLobby.opponents != NULL)) {
			for(ii = 0; ii < client->sfsLobby.opponentLength; ii++) {
				if(client->sfsLobby.opponents[ii].artName != NULL) {
					free(client->sfsLobby.opponents[ii].artName);
				}
				if(client->sfsLobby.opponents[ii].level != NULL) {
					free(client->sfsLobby.opponents[ii].level);
				}
				if(client->sfsLobby.opponents[ii].name != NULL) {
					free(client->sfsLobby.opponents[ii].name);
				}
				if(client->sfsLobby.opponents[ii].points != NULL) {
					free(client->sfsLobby.opponents[ii].points);
				}
			}

			free(client->sfsLobby.opponents);

			client->sfsLobby.opponentLength = 0;
			client->sfsLobby.opponents = NULL;
		}
	}

	if((isJoinRoomResponse) && (length > 0)) {
		for(ii = 0; ii < length; ii++) {
			if((opponentObject = json_getObjectFromArray(payloadObject,
							"opponents",
							ii)) == NULL) {
				log_logf(&context->log, LOG_LEVEL_ERROR,
						"%s() failed at line %i",
						__FUNCTION__, __LINE__);
				return afalse;
			}


			if(!client_addToOpponentListFromJson(client, opponentObject)) {
				log_logf(&context->log, LOG_LEVEL_ERROR,
						"%s() failed at line %i",
						__FUNCTION__, __LINE__);
				return afalse;
			}
		}
	}
	else if(!isJoinRoomResponse) {
		if(!client_addToOpponentListFromJson(client, payloadObject)) {
			log_logf(&context->log, LOG_LEVEL_ERROR,
					"%s() failed at line %i",
					__FUNCTION__, __LINE__);
			return afalse;
		}
	}

	return atrue;
}

static int client_executeHttpLogin(Client *client)
{
	int requestLength = 0;
	int responseLength = 0;
	char *request = NULL;
	char *response = NULL;

	mutex_lock(&client->mutex);

	if(client->status != CLIENT_STATUS_INIT) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		mutex_unlock(&client->mutex);
		return -1;
	}

	clientConnection_init(&client->connection, SFS_WEB_SERVER_IP,
			SFS_WEB_SERVER_PORT);

	request = message_html_buildLoginRequest(SFS_WEB_SERVER_HOSTNAME, client,
			&requestLength);

	if((request == NULL) || (requestLength < 1)) {
		clientConnection_free(&client->connection);
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		mutex_unlock(&client->mutex);
		return -1;
	}

	if(DEBUG_MODE) {
		displayString(DEBUG_STREAM, request, requestLength);
	}

	response = clientConnection_executeHttpTransaction(&client->connection,
			request, requestLength, &responseLength);

	if(request != NULL) {
		free(request);
	}

	clientConnection_free(&client->connection);

	if((response == NULL) || (responseLength < 1)) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		mutex_unlock(&client->mutex);
		return -1;
	}
	else if(message_html_isLoginValid(client, response, responseLength)) {
		client->status = CLIENT_STATUS_IS_HTTP_LOGIN;
	}
	else {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		if(response != NULL) {
			free(response);
		}
		mutex_unlock(&client->mutex);
		return -1;
	}

	if(DEBUG_MODE) {
		displayString(DEBUG_STREAM, response, responseLength);
	}

	if(response != NULL) {
		free(response);
	}

	mutex_unlock(&client->mutex);

	return 0;
}

static int client_executePolicyFileRequest(Client *client, aboolean isLobby)
{
	int counter = 0;
	int requestLength = 0;
	int responseLength = 0;
	char *request = NULL;
	char *response = NULL;

	mutex_lock(&client->mutex);

	if((!isLobby) && (client->status == CLIENT_STATUS_IS_GAME_READY)) {
		clientConnection_free(&client->connection);
	}
	else if(client->status != CLIENT_STATUS_IS_HTTP_LOGIN) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		mutex_unlock(&client->mutex);
		return -1;
	}

	if(isLobby) {
		clientConnection_init(&client->connection, SFS_LOBBY_SERVER_IP,
				SFS_LOBBY_SERVER_PORT);
	}
	else {
		clientConnection_init(&client->connection, SFS_GAME_SERVER_IP,
				SFS_GAME_SERVER_PORT);
	}

	request = message_xml_buildPolicyFileRequest(&requestLength);

	if((request == NULL) || (requestLength < 1)) {
		clientConnection_free(&client->connection);
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		mutex_unlock(&client->mutex);
		return -1;
	}

	if(DEBUG_MODE) {
		displayString(DEBUG_STREAM, request, requestLength);
	}

	counter = 0;

	do {
		response = clientConnection_executeTransaction(&client->connection,
				request, requestLength, &responseLength);
		counter++;
	} while((counter < SFS_SERVER_RETRY_THRESHOLD) && (response == NULL));

	if(request != NULL) {
		free(request);
	}

	clientConnection_free(&client->connection);

	if((response == NULL) || (responseLength < 1)) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		mutex_unlock(&client->mutex);
		return -1;
	}
	else if(message_xml_isPolicyFileResponseValid(response, responseLength,
				isLobby)) {
		if(client->status == CLIENT_STATUS_IS_HTTP_LOGIN) {
			client->status = CLIENT_STATUS_HAS_POLICY_FILE;
		}
		else {
			client->status = CLIENT_STATUS_HAS_GAME_SERVER_POLICY_FILE;
		}
	}
	else {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		if(response != NULL) {
			free(response);
		}
		mutex_unlock(&client->mutex);
		return -1;
	}

	if(DEBUG_MODE) {
		displayString(DEBUG_STREAM, response, responseLength);
	}

	if(response != NULL) {
		free(response);
	}

	mutex_unlock(&client->mutex);

	return 0;
}

static int client_executeVersionCheck(Client *client, aboolean isLobby)
{
	int counter = 0;
	int requestLength = 0;
	int responseLength = 0;
	char *request = NULL;
	char *response = NULL;

	mutex_lock(&client->mutex);

	if((client->status != CLIENT_STATUS_HAS_POLICY_FILE) &&
			(client->status != CLIENT_STATUS_HAS_GAME_SERVER_POLICY_FILE)) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		mutex_unlock(&client->mutex);
		return -1;
	}

	if(isLobby) {
		clientConnection_init(&client->connection, SFS_LOBBY_SERVER_IP,
				SFS_LOBBY_SERVER_PORT);
	}
	else {
		clientConnection_init(&client->connection, SFS_GAME_SERVER_IP,
				SFS_GAME_SERVER_PORT);
	}

	request = message_xml_buildVersionCheck(&requestLength);

	if((request == NULL) || (requestLength < 1)) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		mutex_unlock(&client->mutex);
		return -1;
	}

	if(DEBUG_MODE) {
		displayString(DEBUG_STREAM, request, requestLength);
	}

	counter = 0;

	do {
		response = clientConnection_executeTransaction(&client->connection,
				request, requestLength, &responseLength);
		counter++;
	} while((counter < SFS_SERVER_RETRY_THRESHOLD) && (response == NULL));

	if(request != NULL) {
		free(request);
	}

	if((response == NULL) || (responseLength < 1)) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		mutex_unlock(&client->mutex);
		return -1;
	}
	else if(message_xml_isVersionCheckResponseValid(response, responseLength,
				isLobby)) {
		if(client->status == CLIENT_STATUS_HAS_POLICY_FILE) {
			client->status = CLIENT_STATUS_HAS_VERSION_CHECK;
		}
		else {
			client->status = CLIENT_STATUS_HAS_GAME_SERVER_VERSION_CHECK;
		}

//		message_sfs_enqueueAdditionalMessages(client, response, responseLength);
	}
	else {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		if(response != NULL) {
			free(response);
		}
		mutex_unlock(&client->mutex);
		return -1;
	}

	if(DEBUG_MODE) {
		displayString(DEBUG_STREAM, response, responseLength);
	}

	if(response != NULL) {
		free(response);
	}

	mutex_unlock(&client->mutex);

	return 0;
}

static int client_executeSfsServerLogin(Client *client, aboolean isLobby)
{
	int counter = 0;
	int requestLength = 0;
	int responseLength = 0;
	char *request = NULL;
	char *response = NULL;

	mutex_lock(&client->mutex);

	if((client->status != CLIENT_STATUS_HAS_VERSION_CHECK) &&
			(client->status != CLIENT_STATUS_HAS_GAME_SERVER_VERSION_CHECK)) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		mutex_unlock(&client->mutex);
		return -1;
	}

	if(isLobby) {
		request = message_xml_buildSfsServerLogin(client->http.playerToken,
				&requestLength);
	}
	else {
		request = message_xml_buildSfsServerLogin(client->sfsLobby.userName,
				&requestLength);
	}

	if((request == NULL) || (requestLength < 1)) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		mutex_unlock(&client->mutex);
		return -1;
	}

	if(DEBUG_MODE) {
		displayString(DEBUG_STREAM, request, requestLength);
	}

	counter = 0;

	do {
		response = clientConnection_executeTransaction(&client->connection,
				request, requestLength, &responseLength);
		counter++;
	} while((counter < SFS_SERVER_RETRY_THRESHOLD) && (response == NULL));

	if(request != NULL) {
		free(request);
	}

	if((response == NULL) || (responseLength < 1)) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		mutex_unlock(&client->mutex);
		return -1;
	}
	else if(message_json_isSfsServerLoginResponseValid(client, response,
				responseLength, isLobby)) {
		if(client->status == CLIENT_STATUS_HAS_VERSION_CHECK) {
			client->status = CLIENT_STATUS_IS_LOBBY_LOGIN;
		}
		else if(!isLobby) {
			client->status = CLIENT_STATUS_IS_GAME_SERVER_LOGIN;

			message_sfs_enqueueAdditionalMessages(client, response,
					responseLength);
		}
	}
	else {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		if(response != NULL) {
			free(response);
		}
		mutex_unlock(&client->mutex);
		return -1;
	}

	if(DEBUG_MODE) {
		displayString(DEBUG_STREAM, response, responseLength);
	}

	if(response != NULL) {
		free(response);
	}

	mutex_unlock(&client->mutex);

	return 0;
}

static int client_executeGetRoomList(Client *client)
{
	int counter = 0;
	int requestLength = 0;
	int responseLength = 0;
	char *request = NULL;
	char *response = NULL;

	mutex_lock(&client->mutex);

	if(client->status != CLIENT_STATUS_IS_LOBBY_LOGIN) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		mutex_unlock(&client->mutex);
		return -1;
	}

	request = message_xml_buildGetRoomList(&requestLength);

	if((request == NULL) || (requestLength < 1)) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		mutex_unlock(&client->mutex);
		return -1;
	}

	if(DEBUG_MODE) {
		displayString(DEBUG_STREAM, request, requestLength);
	}

	counter = 0;

	do {
		response = clientConnection_executeTransaction(&client->connection,
				request, requestLength, &responseLength);
		counter++;
	} while((counter < SFS_SERVER_RETRY_THRESHOLD) && (response == NULL));

	if(request != NULL) {
		free(request);
	}

	if((response == NULL) || (responseLength < 1)) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		mutex_unlock(&client->mutex);
		return -1;
	}
	else if(message_xml_isGetRoomListResponseValid(client, response,
				responseLength)) {
		client->status = CLIENT_STATUS_IS_GET_ROOM_LIST;

		message_sfs_enqueueAdditionalMessages(client, response, responseLength);
	}
	else {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		if(response != NULL) {
			free(response);
		}
		mutex_unlock(&client->mutex);
		return -1;
	}

	if(DEBUG_MODE) {
		displayString(DEBUG_STREAM, response, responseLength);
	}

	if(response != NULL) {
		free(response);
	}

	mutex_unlock(&client->mutex);

	return 0;
}

static int client_executeJoinRoom(Client *client)
{
	int counter = 0;
	int requestLength = 0;
	int responseLength = 0;
	char *newRoomName = NULL;
	char *request = NULL;
	char *response = NULL;

	mutex_lock(&client->mutex);

	if(client->status < CLIENT_STATUS_IS_GET_ROOM_LIST) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		mutex_unlock(&client->mutex);
		return -1;
	}

//	newRoomName = pickRandomRoom();
	newRoomName = "Skyscraper";

	request = message_json_buildJoinRoom(client->sfsLobby.roomName,
			newRoomName, &requestLength);

	if((request == NULL) || (requestLength < 1)) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		mutex_unlock(&client->mutex);
		return -1;
	}

	if(DEBUG_MODE) {
		displayString(DEBUG_STREAM, request, requestLength);
	}

	counter = 0;

	do {
		response = clientConnection_executeTransaction(&client->connection,
				request, requestLength, &responseLength);
		counter++;
	} while((counter < SFS_SERVER_RETRY_THRESHOLD) && (response == NULL));

	if(request != NULL) {
		free(request);
	}

	if((response == NULL) || (responseLength < 1)) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		mutex_unlock(&client->mutex);
		return -1;
	}
	else if(message_json_isJoinRoomResponseValid(client, response,
				responseLength)) {
		if(client->status == CLIENT_STATUS_IS_GET_ROOM_LIST) {
			client->status = CLIENT_STATUS_IS_JOIN_ROOM;
		}

		if(client->sfsLobby.roomName != NULL) {
			free(client->sfsLobby.roomName);
		}
		client->sfsLobby.roomName = strdup(newRoomName);

		message_sfs_enqueueAdditionalMessages(client, response, responseLength);
	}
	else {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		if(response != NULL) {
			free(response);
		}
		mutex_unlock(&client->mutex);
		return -1;
	}

	if(DEBUG_MODE) {
		displayString(DEBUG_STREAM, response, responseLength);
	}

	if(response != NULL) {
		free(response);
	}

	mutex_unlock(&client->mutex);

	return 0;
}

static int client_executeEnableAutoBattle(Client *client)
{
	int rc = 0;
	int requestLength = 0;
	char *request = NULL;

	mutex_lock(&client->mutex);

	if(client->status < CLIENT_STATUS_IS_JOIN_ROOM) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		mutex_unlock(&client->mutex);
		return -1;
	}

	request = message_json_buildEnableAutoBattle(&requestLength);

	if((request == NULL) || (requestLength < 1)) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		mutex_unlock(&client->mutex);
		return -1;
	}

	if(DEBUG_MODE) {
		displayString(DEBUG_STREAM, request, requestLength);
	}

	rc = clientConnection_sendMessage(&client->connection, request,
			requestLength);

	if(request != NULL) {
		free(request);
	}

	if(rc < 0) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		mutex_unlock(&client->mutex);
		return -1;
	}

	request = message_json_buildChangeAutoBattleParams(&requestLength);

	if((request == NULL) || (requestLength < 1)) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		mutex_unlock(&client->mutex);
		return -1;
	}

	if(DEBUG_MODE) {
		displayString(DEBUG_STREAM, request, requestLength);
	}

	rc = clientConnection_sendMessage(&client->connection, request,
			requestLength);

	if(request != NULL) {
		free(request);
	}

	if(rc < 0) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		mutex_unlock(&client->mutex);
		return -1;
	}

	if(client->status == CLIENT_STATUS_IS_JOIN_ROOM) {
		client->status = CLIENT_STATUS_IS_ENABLE_AUTO_BATTLE;
	}

	mutex_unlock(&client->mutex);

	return 0;
}

static int client_executeCreateGame(Client *client)
{
	int pick = 0;
	int rc = 0;
	int requestLength = 0;
	char *request = NULL;

	mutex_lock(&client->mutex);

	if(client->status < CLIENT_STATUS_IS_ENABLE_AUTO_BATTLE) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		mutex_unlock(&client->mutex);
		return -1;
	}

	if(client->sfsLobby.opponentLength < 1) {
		mutex_unlock(&client->mutex);
		return -2;
	}

	system_pickRandomSeed();

	pick = (rand() % client->sfsLobby.opponentLength);

	request = message_json_buildCreateGame(
			client->sfsLobby.opponents[pick].name, &requestLength);

	if((request == NULL) || (requestLength < 1)) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		mutex_unlock(&client->mutex);
		return -1;
	}

	if(DEBUG_MODE) {
		displayString(DEBUG_STREAM, request, requestLength);
	}

	rc = clientConnection_sendMessage(&client->connection, request,
			requestLength);

	if(request != NULL) {
		free(request);
	}

	if(rc < 0) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		mutex_unlock(&client->mutex);
		return -1;
	}

	if(client->status == CLIENT_STATUS_IS_ENABLE_AUTO_BATTLE) {
		client->status = CLIENT_STATUS_IS_CREATE_GAME;
	}

	mutex_unlock(&client->mutex);

	return 0;
}

static int client_executeIsBattleReady(Client *client)
{
	int pick = 0;
	int rc = 0;
	int requestLength = 0;
	char *request = NULL;

	mutex_lock(&client->mutex);

	if((client->status != CLIENT_STATUS_IS_GAME_SERVER_LOGIN_HAS_JOINED) &&
			(client->status != CLIENT_STATUS_IS_GAME_SERVER_READY)) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		mutex_unlock(&client->mutex);
		return -1;
	}

	if(client->sfsLobby.opponentLength < 1) {
		mutex_unlock(&client->mutex);
		return -2;
	}

	system_pickRandomSeed();

	pick = (rand() % client->sfsLobby.opponentLength);

	request = message_json_buildIsBattleReady(
			client->sfsGameServer.roomId,
			client->sfsGameServer.gameType,
			&requestLength);

	if((request == NULL) || (requestLength < 1)) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		mutex_unlock(&client->mutex);
		return -1;
	}

	if(DEBUG_MODE) {
		displayString(DEBUG_STREAM, request, requestLength);
	}

	rc = clientConnection_sendMessage(&client->connection, request,
			requestLength);

	if(request != NULL) {
		free(request);
	}

	if(rc < 0) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		mutex_unlock(&client->mutex);
		return -1;
	}

	client->status = CLIENT_STATUS_IS_GAME_SERVER_WAITING;

	mutex_unlock(&client->mutex);

	return 0;
}

static int client_executeIsBattleInitialized(Client *client)
{
	int rc = 0;
	int requestLength = 0;
	char *request = NULL;

	mutex_lock(&client->mutex);

	if(client->status != CLIENT_STATUS_IS_BATTLE_INIT) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		mutex_unlock(&client->mutex);
		return -1;
	}

	request = message_json_buildIsBattleInitialized(
			client->sfsGameServer.roomId,
			client->sfsGameServer.gameType,
			&requestLength);

	if((request == NULL) || (requestLength < 1)) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		mutex_unlock(&client->mutex);
		return -1;
	}

	if(DEBUG_MODE) {
		displayString(DEBUG_STREAM, request, requestLength);
	}

	rc = clientConnection_sendMessage(&client->connection, request,
			requestLength);

	if(request != NULL) {
		free(request);
	}

	if(rc < 0) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		mutex_unlock(&client->mutex);
		return -1;
	}

	request = message_json_buildBattleSelection(client, &requestLength);

	if((request == NULL) || (requestLength < 1)) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		mutex_unlock(&client->mutex);
		return -1;
	}

	if(DEBUG_MODE) {
		displayString(DEBUG_STREAM, request, requestLength);
	}

	rc = clientConnection_sendMessage(&client->connection, request,
			requestLength);

	if(request != NULL) {
		free(request);
	}

	if(rc < 0) {
		log_logf(&context->log, LOG_LEVEL_ERROR, "%s() failed at line %i",
				__FUNCTION__, __LINE__);
		mutex_unlock(&client->mutex);
		return -1;
	}

	client->status = CLIENT_STATUS_IS_BATTLE_INIT_WAITING;

	mutex_unlock(&client->mutex);

	return 0;
}

// thread functions

static void *thread_clientSocketListener(void *threadContext, void *argument)
{
	int ii = 0;
	int counter = 0;
	int messageLength = 0;
	char *message = NULL;

	TmfThread *thread = NULL;

	Client *client = NULL;
	ClientMessage *item = NULL;

	log_logf(&context->log, LOG_LEVEL_INFO,
			"%s() executed with arguments 0x%lx, 0x%lx",
			__FUNCTION__, (aptrcast)threadContext, (aptrcast)argument);

	thread = (TmfThread *)threadContext;

	log_logf(&context->log, LOG_LEVEL_INFO,
			"%s() thread #%i (%i) running",
			__FUNCTION__, thread->uid, thread->systemId);

	while((thread->state == THREAD_STATE_RUNNING) &&
			(thread->signal == THREAD_SIGNAL_OK)) {
		mutex_lock(thread->mutex);
		thread->status = THREAD_STATUS_BUSY;
		mutex_unlock(thread->mutex);

		for(ii = 0; ii < context->clientLength; ii++) {
			client = &(context->clients[ii]);

			mutex_lock(&client->mutex);

			if((client->status < CLIENT_STATUS_IS_GET_ROOM_LIST) ||
					(client->connection.socket.state != SOCKET_STATE_OPEN)) {
				mutex_unlock(&client->mutex);
				continue;
			}

			message = clientConnection_listenForMessage(
					&client->connection,
					&messageLength);

			if((message == NULL) || (messageLength < 1)) {
				if(message != NULL) {
					free(message);
				}
				mutex_unlock(&client->mutex);
				continue;
			}

			if(DEBUG_MODE) {
				displayString(DEBUG_STREAM, message, messageLength);
			}

			log_logf(&context->log, LOG_LEVEL_INFO,
					"%s() thread #%i (%i) :: client #%i obtained message '%s'",
					__FUNCTION__, thread->uid, thread->systemId, ii,
					message);

			if((item = clientMessage_new(message, messageLength)) != NULL) {
				if(fifostack_push(&client->messageQueue, item) < 0) {
					clientMessage_free(item);
				}
			}
			else {
				free(message);
			}

			message_sfs_enqueueAdditionalMessages(client, message,
					messageLength);

			mutex_unlock(&client->mutex);
		}

		mutex_lock(thread->mutex);
		thread->status = THREAD_STATUS_NAPPING;
		mutex_unlock(thread->mutex);

		counter = 0;

		while((thread->state == THREAD_STATE_RUNNING) &&
				(thread->signal == THREAD_SIGNAL_OK) &&
				(counter < 128)) {
			time_usleep(1024);
			counter++;
		}
	}

	log_logf(&context->log, LOG_LEVEL_INFO,
			"%s() thread #%i (%i) halted",
			__FUNCTION__, thread->uid, thread->systemId);

	return NULL;
}

static void *thread_clientLoadTest(void *threadContext, void *argument)
{
	aboolean mayFree = atrue;
	int pick = 0;
	int counter = 0;
	char *command = NULL;

	TmfThread *thread = NULL;

	Json *json = NULL;
	Xpath *xpath = NULL;
	Client *client = NULL;
	ClientMessage *message = NULL;

	log_logf(&context->log, LOG_LEVEL_INFO,
			"%s() executed with arguments 0x%lx, 0x%lx",
			__FUNCTION__, (aptrcast)threadContext, (aptrcast)argument);

	thread = (TmfThread *)threadContext;

	log_logf(&context->log, LOG_LEVEL_INFO,
			"%s() thread #%i (%i) running",
			__FUNCTION__, thread->uid, thread->systemId);

	while((thread->state == THREAD_STATE_RUNNING) &&
			(thread->signal == THREAD_SIGNAL_OK)) {
		mutex_lock(thread->mutex);
		thread->status = THREAD_STATUS_BUSY;
		mutex_unlock(thread->mutex);


		// determine if test state is ready

		if(!context->isTestReady) {
			time_usleep(1024);
			continue;
		}


		// execute load-test

		// pick a random client that is not currently busy

		counter = 0;

		system_pickRandomSeed();

		do {
			pick = (rand() % context->clientLength);

			client = &(context->clients[pick]);

			if((client->isBusy) || (client->status == CLIENT_STATUS_ERROR)) {
				pick = -1;
				if(client->isBusy) {
					counter++;
				}
				time_usleep(128);
				continue;
			}

			mutex_lock(&client->mutex);

			if((!client->isBusy) && (client->status != CLIENT_STATUS_ERROR)) {
				client->isBusy = atrue;
			}
			else {
				pick = -1;
			}

			if(client->isBusy) {
				counter++;
			}

			mutex_unlock(&client->mutex);

			if(pick == -1) {
				time_usleep(128);
			}
		} while((pick == -1) && (counter < 8));

		if(pick == -1) {
			// take a nap

			mutex_lock(thread->mutex);
			thread->status = THREAD_STATUS_NAPPING;
			mutex_unlock(thread->mutex);

			counter = 0;

			while((thread->state == THREAD_STATE_RUNNING) &&
					(thread->signal == THREAD_SIGNAL_OK) &&
					(counter < 256)) {
				time_usleep(128);
				counter++;
			}

			continue;
		}

/*		log_logf(&context->log, LOG_LEVEL_DEBUG,
				"%s() thread #%i (%i) picked client #%i",
				__FUNCTION__, thread->uid, thread->systemId, pick);*/

		if((thread->state != THREAD_STATE_RUNNING) ||
				(thread->signal != THREAD_SIGNAL_OK)) {
			break;
		}

		// process any pending messages in the client's queue

		mutex_lock(&client->mutex);

		while((fifostack_pop(&client->messageQueue,
						((void *)&message)) == 0) &&
				(message != NULL)) {
			log_logf(&context->log, LOG_LEVEL_DEBUG,
					"processing client message queue, message length %i",
					message->length);

			if(DEBUG_MODE) {
				displayString(DEBUG_STREAM, message->body, message->length);
			}

			if((json = message_json_messageToJson(client,
							message->body,
							message->length,
							&command)) != NULL) {
				mayFree = atrue;

				if(!strcmp(command, "userArrived")) {
					client_updateOpponentListFromJson(client,
							json,
							afalse,		// is join room response
							afalse		// rebuild list
							);
				}
				else if(!strcmp(command, "alreadyChallenged")) {
					if(client->status == CLIENT_STATUS_IS_ENABLE_AUTO_BATTLE) {
						client->sfsLobby.autoJoinCounter = 0;
						client->status = CLIENT_STATUS_IS_GET_ROOM_LIST;
					}
				}
				else if((!strcmp(command, "initAutoChallenger")) ||
						(!strcmp(command, "initAutoChallenged"))) {
					if(client->status == CLIENT_STATUS_IS_ENABLE_AUTO_BATTLE) {
						client->status = CLIENT_STATUS_IS_GAME_READY;
					}
				}
				else if(!strcmp(command, "joined")) {
					if(client->status ==
							CLIENT_STATUS_IS_GAME_SERVER_LOGIN_IS_JOIN_OK) {
						if(client->sfsGameServer.gameType != NULL) {
							free(client->sfsGameServer.gameType);
						}

						client->sfsGameServer.gameType =
							message_json_getStringFrommessage(
									json,
									"gameType");

						if(client->sfsGameServer.gameType != NULL) {
							client->status =
								CLIENT_STATUS_IS_GAME_SERVER_LOGIN_HAS_JOINED;
						}
					}
					else {
						/*
						 * TODO: handle error condition
						 */
					}
				}
				else if(!strcmp(command, "ud")) {
					if(client->status == CLIENT_STATUS_IS_GAME_SERVER_WAITING) {
						client->status = CLIENT_STATUS_IS_BATTLE_INIT;

						if(client->sfsGameServer.sessionInfo != NULL) {
							json_freePtr(client->sfsGameServer.sessionInfo);
						}

						client->sfsGameServer.sessionInfo = json;

						mayFree = afalse;
					}
				}
//				else if(!strcmp(command, "tst")) {
				else if(!strcmp(command, "sp")) {
					if(client->status == CLIENT_STATUS_IS_BATTLE_INIT_WAITING) {
						client->status = CLIENT_STATUS_IS_BATTLE_IN_PROGRESS;
					}
				}

				if(mayFree) {
					json_freePtr(json);
					free(command);
				}
			}
			else if((xpath = message_xpath_messageToXpath(client,
							message->body,
							message->length)) != NULL) {
				mayFree = atrue;

				if(client->status == CLIENT_STATUS_IS_GAME_SERVER_LOGIN) {
					if((message_xml_isResponseASystemMessage(xpath)) &&
							(message_xml_isResponseARoomListMessage(xpath))) {
						mayFree = afalse;

						if(client->sfsGameServer.roomList != NULL) {
							xpath_free(client->sfsGameServer.roomList);
							free(client->sfsGameServer.roomList);
						}

						client->sfsGameServer.roomList = xpath;

						client->status =
							CLIENT_STATUS_IS_GAME_SERVER_LOGIN_HAS_ROOM_LIST;
					}
					else {
						/*
						 * TODO: handle error condition
						 */
					}
				}
				else if(client->status ==
						CLIENT_STATUS_IS_GAME_SERVER_LOGIN_HAS_ROOM_LIST) {
					if(message_xml_isResponseAJoinOkMessage(client, xpath)) {
						client->status =
							CLIENT_STATUS_IS_GAME_SERVER_LOGIN_IS_JOIN_OK;
					}
					else {
						/*
						 * TODO: handle error condition
						 */
					}
				}

				if(mayFree) {
					xpath_free(xpath);
					free(xpath);
				}
			}

			clientMessage_free(message);
		}

		mutex_unlock(&client->mutex);

		if((thread->state != THREAD_STATE_RUNNING) ||
				(thread->signal != THREAD_SIGNAL_OK)) {
			break;
		}

		// process the next client action, depending upon state

		switch(client->status) {
			case CLIENT_STATUS_INIT:
				if(client_executeHttpLogin(client) < 0) {
					mutex_lock(&client->mutex);
					client->status = CLIENT_STATUS_ERROR;
					mutex_unlock(&client->mutex);
				}
				break;

			case CLIENT_STATUS_IS_HTTP_LOGIN:
				if(client_executePolicyFileRequest(client,
							atrue	// is lobby
							) < 0) {
					mutex_lock(&client->mutex);
					client->status = CLIENT_STATUS_ERROR;
					mutex_unlock(&client->mutex);
				}
				break;

			case CLIENT_STATUS_HAS_POLICY_FILE:
				if(client_executeVersionCheck(client,
							atrue	// is lobby
							) < 0) {
					mutex_lock(&client->mutex);
					client->status = CLIENT_STATUS_ERROR;
					mutex_unlock(&client->mutex);
				}
				break;

			case CLIENT_STATUS_HAS_VERSION_CHECK:
				if(client_executeSfsServerLogin(client,
							atrue	// is lobby
							) < 0) {
					mutex_lock(&client->mutex);
					client->status = CLIENT_STATUS_ERROR;
					mutex_unlock(&client->mutex);
				}
				break;

			case CLIENT_STATUS_IS_LOBBY_LOGIN:
				if(client_executeGetRoomList(client) < 0) {
					mutex_lock(&client->mutex);
					client->status = CLIENT_STATUS_ERROR;
					mutex_unlock(&client->mutex);
				}
				break;

			case CLIENT_STATUS_IS_GET_ROOM_LIST:
				client_executeJoinRoom(client);
				break;

			case CLIENT_STATUS_IS_JOIN_ROOM:
				if(client->sfsLobby.opponentLength > 0) {
					client_executeEnableAutoBattle(client);
				}
				break;

			case CLIENT_STATUS_IS_ENABLE_AUTO_BATTLE:
			case CLIENT_STATUS_IS_CREATE_GAME:
				mutex_lock(&client->mutex);

				if(client->sfsLobby.opponentLength > 0) {
					client->sfsLobby.autoJoinCounter += 1;

/*					log_logf(&context->log, LOG_LEVEL_INFO,
							"%s() client #%i waited for %i ticks for battle",
							__FUNCTION__, pick,
							client->sfsLobby.autoJoinCounter);*/
				}

				if(client->sfsLobby.autoJoinCounter > 1024) {
					client->sfsLobby.autoJoinCounter = 0;
					client->status = CLIENT_STATUS_IS_GET_ROOM_LIST;
				}

				mutex_unlock(&client->mutex);
				break;

			case CLIENT_STATUS_IS_GAME_READY:
				if(client_executePolicyFileRequest(client,
							afalse	// is lobby
							) < 0) {
					mutex_lock(&client->mutex);
//					client->status = CLIENT_STATUS_INIT;
//					clientConnection_free(&client->connection);
					client->status = CLIENT_STATUS_ERROR;
					mutex_unlock(&client->mutex);
				}
				break;

			case CLIENT_STATUS_HAS_GAME_SERVER_POLICY_FILE:
				if(client_executeVersionCheck(client,
							afalse	// is lobby
							) < 0) {
					mutex_lock(&client->mutex);
//					client->status = CLIENT_STATUS_INIT;
//					clientConnection_free(&client->connection);
					client->status = CLIENT_STATUS_ERROR;
					mutex_unlock(&client->mutex);
				}
				break;

			case CLIENT_STATUS_HAS_GAME_SERVER_VERSION_CHECK:
				if(client_executeSfsServerLogin(client,
							afalse	// is lobby
							) < 0) {
					mutex_lock(&client->mutex);
//					client->status = CLIENT_STATUS_INIT;
//					clientConnection_free(&client->connection);
					client->status = CLIENT_STATUS_ERROR;
					mutex_unlock(&client->mutex);
				}
				break;

			case CLIENT_STATUS_IS_GAME_SERVER_LOGIN_HAS_JOINED:
			case CLIENT_STATUS_IS_GAME_SERVER_READY:
				if(client_executeIsBattleReady(client) < 0) {
					mutex_lock(&client->mutex);
//					client->status = CLIENT_STATUS_INIT;
//					clientConnection_free(&client->connection);
					client->status = CLIENT_STATUS_ERROR;
					mutex_unlock(&client->mutex);
				}
				break;

			case CLIENT_STATUS_IS_BATTLE_INIT:
				if(client_executeIsBattleInitialized(client) < 0) {
					mutex_lock(&client->mutex);
//					client->status = CLIENT_STATUS_INIT;
//					clientConnection_free(&client->connection);
					client->status = CLIENT_STATUS_ERROR;
					mutex_unlock(&client->mutex);
				}
				break;

			case CLIENT_STATUS_IS_BATTLE_IN_PROGRESS:
				/*
				 * TODO: this
				 */
				break;
		}

		mutex_lock(&client->mutex);
		client->isBusy = afalse;
		mutex_unlock(&client->mutex);

		if((thread->state != THREAD_STATE_RUNNING) ||
				(thread->signal != THREAD_SIGNAL_OK)) {
			break;
		}


		// take a nap

		mutex_lock(thread->mutex);
		thread->status = THREAD_STATUS_NAPPING;
		mutex_unlock(thread->mutex);

		counter = 0;

		while((thread->state == THREAD_STATE_RUNNING) &&
				(thread->signal == THREAD_SIGNAL_OK) &&
				(counter < 32)) {
			time_usleep(128);
			counter++;
		}
	}

	log_logf(&context->log, LOG_LEVEL_INFO,
			"%s() thread #%i (%i) halted",
			__FUNCTION__, thread->uid, thread->systemId);

	return NULL;
}

