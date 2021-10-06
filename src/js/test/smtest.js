/*
 * smtest.js
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * A JavaScript test to determine if the SpiderMonkey engine integrated into
 * Asgard is working properly.
 *
 * Written by Josh English.
 */

// define local functions

function myScriptFunc()
{
	argCheckFunction('myScriptFunc!!!');

	return false;
}


// check argument conversion, using a native function

argCheckFunction('hello world!',
		'this is a test string',
		123,
		12345.234,
		false,
		null,
		undefined);


// include a file, and execute a function from it

print('including "smtest2.js"...');

include('src/js/test/smtest2.js');

smtestTwoFunction();

print('this is an alert');


// validate that native variables are accesible

print('Native variable [global->nativeBooleanVar] is: ' +
		remit('global', 'nativeBooleanVar'));
print('Native variable [global->nativeIntegerVar] is: ' +
		remit('global', 'nativeIntegerVar'));
print('Native variable [global->nativeDoubleVar] is: ' +
		remit('global', 'nativeDoubleVar'));
print('Native variable [global->nativeStringVar] is: ' +
		remit('global', 'nativeStringVar'));


// validate that native variables are definable

print('Native variable [script->nativeBooleanVar] is: ' +
		emit('script', 'nativeBooleanVar', true));
print('Native variable [script->nativeIntegerVar] is: ' +
		emit('script', 'nativeIntegerVar', 12345));
print('Native variable [script->nativeDoubleVar] is: ' +
		emit('script', 'nativeDoubleVar', 5678.9012));
print('Native variable [script->nativeStringVar] is: ' +
		emit('script', 'nativeStringVar', 'fresh from the script'));


// validate the asgard native function library

// javascript signal handler

function signalHandler(signalType)
{
	print('------------------------------------------------------');
	print('Executed :: signalHandler() on signal => "' +
			signal_toString(signalType) + '"');
	print('------------------------------------------------------');

	return 'ok';
}

// javascript thread function

var workerStatus = true;

var workerMutex = mutex_new();

function testWorkerThread()
{
	mutex_lock(workerMutex);
	workerStatus = true;
	mutex_unlock(workerMutex);

	print('Thread :: testWorkerThread() :: started');

	time_sleep(2);

	print('Thread :: testWorkerThread() :: shutdown');

	mutex_lock(workerMutex);
	workerStatus = false;
	mutex_unlock(workerMutex);

	return 'ok';
}

// Asgard Native API Unit Test

function executeAsgardNativeApiTest()
{
	// native core/common functions

	print('\n\n');
	print('Asgard Native Function Library, build ' +
			asgard_getVersion() + ', date ' + asgard_getDate() + '.');

	print('\nUsing operating system: ' + native_getOsType());

	// native core/common functions

	print('\nAsgard native core/common API ::\n');

	print('\t Constants ::\n');

	print('\t\t INT_ONE_THOUSAND  => ' +
			native_getConstant('INT_ONE_THOUSAND'));
	print('\t\t REAL_ONE_THOUSAND => ' +
			native_getConstant('REAL_ONE_THOUSAND'));
	print('\t\t INT_ONE_MILLION   => ' +
			native_getConstant('INT_ONE_MILLION'));
	print('\t\t REAL_ONE_MILLION  => ' +
			native_getConstant('REAL_ONE_MILLION'));
	print('\t\t INT_ONE_BILLION   => ' +
			native_getConstant('INT_ONE_BILLION'));
	print('\t\t REAL_ONE_BILLION  => ' +
			native_getConstant('REAL_ONE_BILLION'));
	print('\t\t DIR_SEPARATOR     => ' +
			native_getConstant('DIR_SEPARATOR'));

	// native core/os functions

	print('\nAsgard native core/os API ::\n');

	print('\t Constants ::\n');

	print('\t\t SIZEOF_BOOLEAN  => ' + native_getConstant('SIZEOF_BOOLEAN'));
	print('\t\t SIZEOF_ALINT    => ' + native_getConstant('SIZEOF_ALINT'));
	print('\t\t SIZEOF_AULINT   => ' + native_getConstant('SIZEOF_AULINT'));
	print('\t\t SIZEOF_CHAR     => ' + native_getConstant('SIZEOF_CHAR'));
	print('\t\t SIZEOF_SHORT    => ' + native_getConstant('SIZEOF_SHORT'));
	print('\t\t SIZEOF_INT      => ' + native_getConstant('SIZEOF_INT'));
	print('\t\t SIZEOF_FLOAT    => ' + native_getConstant('SIZEOF_FLOAT'));
	print('\t\t SIZEOF_DOUBLE   => ' + native_getConstant('SIZEOF_DOUBLE'));

	print('\n\t Functions ::\n');

	var timestampSeconds = time_getTimeSeconds();

	var timestampMus = time_getTimeMus();

	print('\t\t time_getTimeSeconds() :: ' + timestampSeconds);

	print('\t\t time_getTimeMus() :: ' + timestampMus);

	print('\t\t time_getElapsedSeconds() :: ' +
			time_getElapsedSeconds(timestampSeconds));

	print('\t\t time_getElapsedMus() :: ' + time_getElapsedMus(timestampMus));

	print('\t\t time_getElapsedMusInMillis() :: ' +
			time_getElapsedMusInMillis(timestampMus));

	print('\t\t time_getElapsedMusInSeconds() :: ' +
			time_getElapsedMusInSeconds(timestampMus));

	print('\t\t time_getTimestamp() :: ' + time_getTimestamp());

	time_sleep(1);

	print('\t\t time_getElapsedMusInSeconds() :: ' +
			time_getElapsedMusInSeconds(timestampMus));

	time_usleep(1024);

	print('\t\t time_getElapsedMusInSeconds() :: ' +
			time_getElapsedMusInSeconds(timestampMus));

	time_nanosleep(131072);

	print('\t\t time_getElapsedMusInSeconds() :: ' +
			time_getElapsedMusInSeconds(timestampMus));

	// native core/system functions

	print('\nAsgard native core/system API ::\n');

	print('\t Constants ::\n');

	print('\t signals:\n');

	print('\t\t SIGNAL_INTERRUPT                 => ' +
			native_getConstant('SIGNAL_INTERRUPT'));
	print('\t\t SIGNAL_TERMINATE                 => ' +
			native_getConstant('SIGNAL_TERMINATE'));
	print('\t\t SIGNAL_SEGFAULT                  => ' +
			native_getConstant('SIGNAL_SEGFAULT'));
	print('\t\t SIGNAL_USERONE                   => ' +
			native_getConstant('SIGNAL_USERONE'));
	print('\t\t SIGNAL_USERTWO                   => ' +
			native_getConstant('SIGNAL_USERTWO'));
	print('\t\t SIGNAL_QUIT                      => ' +
			native_getConstant('SIGNAL_QUIT'));
	print('\t\t SIGNAL_ABORT                     => ' +
			native_getConstant('SIGNAL_ABORT'));
	print('\t\t SIGNAL_FLOATING_POINT_EXCEPTION  => ' +
			native_getConstant('SIGNAL_FLOATING_POINT_EXCEPTION'));
	print('\t\t SIGNAL_ILLEGAL                   => ' +
			native_getConstant('SIGNAL_ILLEGAL'));

	print('\n\t sockets:\n');

	print('\t\t SOCKET_MODE_CLIENT               => ' +
			native_getConstant('SOCKET_MODE_CLIENT'));
	print('\t\t SOCKET_MODE_SERVER               => ' +
			native_getConstant('SOCKET_MODE_SERVER'));
	print('\t\t SOCKET_PROTOCOL_TCPIP            => ' +
			native_getConstant('SOCKET_PROTOCOL_TCPIP'));
	print('\t\t SOCKET_PROTOCOL_UDP              => ' +
			native_getConstant('SOCKET_PROTOCOL_UDP'));

	print('\n\t threads:\n');

	print('\t\t THREAD_STATE_INIT                => ' +
			native_getConstant('THREAD_STATE_INIT'));
	print('\t\t THREAD_STATE_RUNNING             => ' +
			native_getConstant('THREAD_STATE_RUNNING'));
	print('\t\t THREAD_STATE_HALT                => ' +
			native_getConstant('THREAD_STATE_HALT'));
	print('\t\t THREAD_STATE_STOPPED             => ' +
			native_getConstant('THREAD_STATE_STOPPED'));
	print('\t\t THREAD_STATE_ERROR               => ' +
			native_getConstant('THREAD_STATE_ERROR'));

	print('\n\t Functions ::\n');
	print('\t mutexes:\n');

	mutex = mutex_new();

	print('\t\t mutex_new() :: ' + mutex);

	mutex_lock(mutex);

	print('\t\t mutex_lock(' + mutex + ')');

	mutex_unlock(mutex);

	print('\t\t mutex_unlock(' + mutex + ')');

	mutex_free(mutex);

	print('\t\t mutex_free(' + mutex + ')');

	print('\n\t signals:\n');

	var SIGNAL_ABORT = native_getConstant('SIGNAL_ABORT');

	signalHandle = signal_registerAction(SIGNAL_ABORT, 'signalHandler');

	print('\t\t signal_registerAction() :: ' + signalHandle);

	print('\t\t signal_executeSignal(' + SIGNAL_ABORT + ')');

	signal_executeSignal(SIGNAL_ABORT);

	print('\t\t signal_toString(' + SIGNAL_ABORT + ') => ' +
		signal_toString(SIGNAL_ABORT));

	print('\n\t system:\n');

	var filename = 'some/test\\filename.file';

	print('\t\t system_fileSetNative() :: ' + system_fileSetNative(filename));

	print('\t\t system_fileExists() :: ' + system_fileExists('makefile'));

	print('\t\t system_fileLength() :: ' + system_fileLength('makefile') +
		' bytes');

	print('\t\t system_fileModifiedTime() :: ' +
		system_fileModifiedTime('makefile'));

	print('\t\t system_fileDelete() :: ' +
		system_fileDelete('some.none.existant.file'));

	if(native_getOsType() == 'Linux') {
//		print('\t\t system_fileExecute("/bin/ls", "-1") :: ' +
//			system_fileExecute('/bin/ls', '-l'));
	}

	print('');

	print('\t\t system_dirExists() :: ' + system_dirExists('src/js/test'));

	print('\t\t system_dirChangeTo() :: ' + system_dirChangeTo('src/js/test'));

	if(native_getOsType() == 'Linux') {
//		print('\t\t system_fileExecute("/bin/ls", "-1") :: ' +
//			system_fileExecute('/bin/ls', '-l'));
	}

	print('\t\t system_dirChangeTo() :: ' + system_dirChangeTo('../../..'));

	print('\t\t system_dirCreate() :: ' + system_dirCreate('smtest-dir'));

	print('\t\t system_dirDelete() :: ' + system_dirDelete('smtest-dir'));

	print('\t\t system_pickRandomSeed() :: ' + system_pickRandomSeed());

	print('\t\t system_setRandomSeed(1234) :: ' + system_setRandomSeed(1234));

	print('\n\t sockets:\n');

	var socket = socket_new(SOCKET_MODE_CLIENT, SOCKET_PROTOCOL_TCPIP,
			'www.google.com', 80);

	var socketRequest = '' +
		'GET / HTTP/1.1\x0d\x0a' +
		'Host: www.google.com\x0d\x0a' +
		'User-Agent: Asgard Scripting Engine, Unit Test/' +
		asgard_getVersion() +
		'(' +
		asgard_getVersion() +
		'; en-US) Asgard/' +
		asgard_getDate() +
		'\x0d\x0a' +
		'Accept: text/html,application/xhtml+xml,application/xml;' +
		'q=0.9,*/*;q=0.8\x0d\x0a' +
		'Accept-Language: en-us,en;q=0.5\x0d\x0a' +
		'Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7\x0d\x0a' +
		'Keep-Alive: 115\x0d\x0a' +
		'Connection: keep-alive\x0d\x0a' +
		'Cache-Control: max-age=0\x0d\x0a' +
		'\x0d\x0a' +
		'';

	print('\t\t socket_new() :: ' + socket);

	print('\t\t socket_open() :: ' +
			socket_open(socket));

	print('\t\t socket_socketStateToString() :: ' +
			socket_socketStateToString(socket));

	print('\t\t socket_socketModeToString() :: ' +
			socket_socketModeToString(socket));

	print('\t\t socket_socketProtocolToString() :: ' +
			socket_socketProtocolToString(socket));

	var sd = socket_getSocketDescriptor(socket);

	print('\t\t socket_getSocketDescriptor() :: ' + sd);

//	print('REQUEST[' + socketRequest + ']');

	print('\t\t socket_send() :: ' +
			socket_send(socket, sd, socketRequest, socketRequest.length) +
			', sent ' + socketRequest.length + ' bytes');

	var chunk = '';
	var buffer = '';

	var timer = 0.0;
	var elapsedTime = 0.0;
	var remainingTime = 1.0;

	do {
		timer = time_getTimeMus();

		chunk = socket_receive(socket, sd, 4, remainingTime);

		if((typeof chunk == 'boolean') && (chunk == false)) {
			break;
		}

		elapsedTime = time_getElapsedMusInSeconds(timer);
		remainingTime -= elapsedTime;

		buffer += chunk;
	} while(remainingTime > 0.0);

//	print('RESPONSE[' + buffer + ']');
	print('\t\t socket_receive() :: received ' + buffer.length + ' bytes');

	print('\t\t socket_close() :: ' +
			socket_close(socket));

	print('\t\t socket_getBytesSent() :: ' +
			socket_getBytesSent(socket));

	print('\t\t socket_getBytesReceived() :: ' +
			socket_getBytesReceived(socket));

	print('\t\t socket_getReadsAttempted() :: ' +
			socket_getReadsAttempted(socket));

	print('\t\t socket_getReadsCompleted() :: ' +
			socket_getReadsCompleted(socket));

	print('\t\t socket_getWritesAttempted() :: ' +
			socket_getWritesAttempted(socket));

	print('\t\t socket_getWritesCompleted() :: ' +
			socket_getWritesCompleted(socket));

	print('\t\t socket_getTotalReadTime() :: ' +
			socket_getTotalReadTime(socket));

	print('\t\t socket_getTotalWriteTime() :: ' +
			socket_getTotalWriteTime(socket));

	print('\n\t threads:\n');

	print('\t\t THREAD_STATE_INIT :: ' + THREAD_STATE_INIT);

	var thread = thread_new('testWorkerThread');

	print('\t\t thread_new() :: ' + thread);

	print('\t\t thread_create(' + thread + ') :: ' + thread_create(thread));

	var counter = 0;

	while(workerStatus) {
		time_usleep(1024);
		counter++;
	}

	print('\t\t counted to ' + counter + ' while waiting for thread shutdown');

	print('\t\t thread_join(' + thread + ') :: ' + thread_join(thread));

	print('\t\t thread_self() :: ' + thread_self());

	return 'passed';
}

