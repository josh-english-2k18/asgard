/*
 * xpath_unitTest.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard core library XML library with an Xpath interface, unit test.
 *
 * Written by Josh English.
 */

// preprocessor directives

#define _CORE_COMMON_BYPASS_STD_STREAM
#include "core/core.h"


// define xpath unit test constants

static char *BASIC_FUNCTIONALITY_TEST_DOCUMENT = ""
"<someXmlDocument>\n"
"	<desc>This is a description.</desc>\n"
"	<attr name=\"someAttrName\" value=\"someAttrValue\">attribute content</attr>\n"
"	<integerValue desc=\"for xpath-parsing integers\">12345</integerValue>\n"
"	<doubleValue desc=\"for xpath-parsing doubles\">12345.6543</doubleValue>\n"
"</someXmlDocument>\n"
"<someOtherXmlDocument>\n"
"	<doc>This is a description.</doc>\n"
"	<anotherOtherAttr someName=\"someAttrName\" someValue=\"someAttrValue\">attribute content</anotherOtherAttr>\n"
"	<booleanValue desc=\"for xpath-parsing booleans\">true</booleanValue>\n"
"</someOtherXmlDocument>\n"
"<nodeServerConfig>\n"
"	<environmentName>Win-32 Development</environmentName>\n"
"	<privateRsaKeyName>conf/sso.private.rsa.key</privateRsaKeyName>\n"
"	<publicRsaKeyName>conf/sso.public.rsa.key</publicRsaKeyName>\n"
"	<hostname>192.168.0.102</hostname>\n"
"	<port>3001</port>\n"
"	<logLevel>0</logLevel>\n"
"	<logFile></logFile>\n"
"	<maxLogFileSize>100000000</maxLogFileSize>\n"
"	<logFileBackup>true</logFileBackup>\n"
"	<minThreadPoolSize>8</minThreadPoolSize>\n"
"	<maxThreadPoolSize>32</maxThreadPoolSize>\n"
"	<dataFile>fst.data.dump</dataFile>\n"
"	<fileCheckInterval>10</fileCheckInterval>\n"
"	<installPath>fst/client/</installPath>\n"
"	<storagePath>fst/client_data/</storagePath>\n"
"	<uploadedPath>fst/client_upload/</uploadedPath>\n"
"	<updatePath>fst/update/</updatePath>\n"
"	<masterServerAddress>192.168.0.102</masterServerAddress>\n"
"	<masterServerAddress>192.168.0.123</masterServerAddress>\n"
"	<masterServerPort>3000</masterServerPort>\n"
"	<httpProperties>\n"
"		<logLevel>0</logLevel>\n"
"		<logFile></logFile>\n"
"		<maxLogFileSize>100000000</maxLogFileSize>\n"
"		<logFileBackup>true</logFileBackup>\n"
"		<minThreadPoolSize>2</minThreadPoolSize>\n"
"		<maxThreadPoolSize>4</maxThreadPoolSize>\n"
"		<someStringValue>this is a string</someStringValue>\n"
"		<port>8081</port>\n"
"	</httpProperties>\n"
"</nodeServerConfig>\n"
"<msg t='sys'>\n"
"	<body action='rmList' r='0'>\n"
"		<rmList>\n"
"			<rm id='415121' priv='0' temp='1' game='1' ucnt='2' scnt='0' maxu='2' maxs='0'>\n"
"				<n><![CDATA[FurySerpent7271]]></n>\n"
"			</rm>\n"
"			<rm id='415120' priv='0' temp='1' game='1' ucnt='2' scnt='0' maxu='2' maxs='0'>\n"
"				<n><![CDATA[LightningLeone4891]]></n>\n"
"			</rm>\n"
"			<rm id='1' priv='0' temp='0' game='0' ucnt='0' maxu='2000' maxs='0'>\n"
"				<n><![CDATA[Lobby]]></n>\n"
"			</rm>\n"
"			<rm id='415122' priv='0' temp='1' game='1' ucnt='2' scnt='0' maxu='2' maxs='0'>\n"
"				<n><![CDATA[EarthSerpent6233]]></n>\n"
"			</rm>\n"
"			<rm id='155880' priv='0' temp='1' game='1' ucnt='1' scnt='0' maxu='2' maxs='0'>\n"
"				<n><![CDATA[HyperWolf4515]]></n>\n"
"			</rm>\n"
"			<rm id='415125' priv='0' temp='1' game='1' ucnt='2' scnt='0' maxu='2' maxs='0'>\n"
"				<n><![CDATA[TorchScorpio1741]]></n>\n"
"			</rm>\n"
"			<rm id='415124' priv='0' temp='1' game='1' ucnt='2' scnt='0' maxu='2' maxs='0'>\n"
"				<n><![CDATA[NightBull9793]]></n>\n"
"			</rm>\n"
"			<rm id='415127' priv='0' temp='1' game='1' ucnt='2' scnt='0' maxu='2' maxs='0'>\n"
"				<n><![CDATA[EvilFireblaze8440]]></n>\n"
"			</rm>\n"
"			<rm id='415126' priv='0' temp='1' game='1' ucnt='2' scnt='0' maxu='2' maxs='0'>\n"
"				<n><![CDATA[FlameLeone5483]]></n>\n"
"			</rm>\n"
"			<rm id='415129' priv='0' temp='1' game='1' ucnt='2' scnt='0' maxu='2' maxs='0'>\n"
"				<n><![CDATA[HyperWolf6893]]></n>\n"
"			</rm>\n"
"			<rm id='415130' priv='0' temp='1' game='1' ucnt='2' scnt='0' maxu='2' maxs='0'>\n"
"				<n><![CDATA[ThunderAquario4858]]></n>\n"
"			</rm>\n"
"			<rm id='415133' priv='0' temp='1' game='1' ucnt='2' scnt='0' maxu='2' maxs='0'>\n"
"				<n><![CDATA[ShadowScorpio3587]]></n>\n"
"			</rm>\n"
"			<rm id='415132' priv='0' temp='1' game='1' ucnt='2' scnt='0' maxu='2' maxs='0'>\n"
"				<n><![CDATA[DarkL-Drago4611]]></n>\n"
"			</rm>\n"
"			<rm id='415135' priv='0' temp='1' game='1' ucnt='2' scnt='0' maxu='2' maxs='0'>\n"
"				<n><![CDATA[CyberSagittario9351]]></n>\n"
"			</rm>\n"
"			<rm id='415134' priv='0' temp='1' game='1' ucnt='2' scnt='0' maxu='2' maxs='0'>\n"
"				<n><![CDATA[EvilSerpent6762]]></n>\n"
"			</rm>\n"
"			<rm id='415104' priv='0' temp='1' game='1' ucnt='2' scnt='0' maxu='2' maxs='0'>\n"
"				<n><![CDATA[CounterBull8547]]></n>\n"
"			</rm>\n"
"			<rm id='415105' priv='0' temp='1' game='1' ucnt='2' scnt='0' maxu='2' maxs='0'>\n"
"				<n><![CDATA[RockSagittario4871]]></n>\n"
"			</rm>\n"
"			<rm id='415106' priv='0' temp='1' game='1' ucnt='2' scnt='0' maxu='2' maxs='0'>\n"
"				<n><![CDATA[LightningPegasus8642]]></n>\n"
"			</rm>\n"
"			<rm id='415107' priv='0' temp='1' game='1' ucnt='2' scnt='0' maxu='2' maxs='0'>\n"
"				<n><![CDATA[CyberOrso2621]]></n>\n"
"			</rm>\n"
"			<rm id='415108' priv='0' temp='1' game='1' ucnt='2' scnt='0' maxu='2' maxs='0'>\n"
"				<n><![CDATA[PoisonLibra2687]]></n>\n"
"			</rm>\n"
"			<rm id='415109' priv='0' temp='1' game='1' ucnt='2' scnt='0' maxu='2' maxs='0'>\n"
"				<n><![CDATA[FuryScorpio7395]]></n>\n"
"			</rm>\n"
"			<rm id='415110' priv='0' temp='1' game='1' ucnt='2' scnt='0' maxu='2' maxs='0'>\n"
"				<n><![CDATA[ThunderVirgo4927]]></n>\n"
"			</rm>\n"
"			<rm id='415111' priv='0' temp='1' game='1' ucnt='2' scnt='0' maxu='2' maxs='0'>\n"
"				<n><![CDATA[CounterFireblaze3245]]></n>\n"
"			</rm>\n"
"			<rm id='415112' priv='0' temp='1' game='1' ucnt='2' scnt='0' maxu='2' maxs='0'>\n"
"				<n><![CDATA[PoisonLeone7185]]></n>\n"
"			</rm>\n"
"			<rm id='415113' priv='0' temp='1' game='1' ucnt='2' scnt='0' maxu='2' maxs='0'>\n"
"				<n><![CDATA[EarthPhoenix3992]]></n>\n"
"			</rm>\n"
"			<rm id='415114' priv='0' temp='1' game='1' ucnt='2' scnt='0' maxu='2' maxs='0'>\n"
"				<n><![CDATA[EarthAries7126]]></n>\n"
"			</rm>\n"
"			<rm id='415115' priv='0' temp='1' game='1' ucnt='2' scnt='0' maxu='2' maxs='0'>\n"
"				<n><![CDATA[FuryL-Drago5542]]></n>\n"
"			</rm>\n"
"			<rm id='415117' priv='0' temp='1' game='1' ucnt='2' scnt='0' maxu='2' maxs='0'>\n"
"				<n><![CDATA[LightningAries7221]]></n>\n"
"			</rm>\n"
"			<rm id='415118' priv='0' temp='1' game='1' ucnt='2' scnt='0' maxu='2' maxs='0'>\n"
"				<n><![CDATA[CyberPisces7848]]></n>\n"
"			</rm>\n"
"			<rm id='415119' priv='0' temp='1' game='1' ucnt='1' scnt='0' maxu='2' maxs='0'>\n"
"				<n><![CDATA[CounterEagle7493]]></n>\n"
"			</rm>\n"
"			<rm id='415138' priv='0' temp='1' game='1' ucnt='2' scnt='0' maxu='2' maxs='0'>\n"
"				<n><![CDATA[EvilSagittario3686]]></n>\n"
"			</rm>\n"
"			<rm id='415139' priv='0' temp='1' game='1' ucnt='1' scnt='0' maxu='2' maxs='0'>\n"
"				<n><![CDATA[LightningPisces4744]]></n>\n"
"			</rm>\n"
"			<rm id='415136' priv='0' temp='1' game='1' ucnt='2' scnt='0' maxu='2' maxs='0'>\n"
"				<n><![CDATA[StormFireblaze4670]]></n>\n"
"			</rm>\n"
"			<rm id='415142' priv='0' temp='1' game='1' ucnt='1' scnt='0' maxu='2' maxs='0'>\n"
"				<n><![CDATA[TorchPhoenix4038]]></n>\n"
"			</rm>\n"
"			<rm id='415143' priv='0' temp='1' game='1' ucnt='0' scnt='0' maxu='2' maxs='0'>\n"
"				<n><![CDATA[StormPegasus1234]]></n>\n"
"			</rm>\n"
"			<rm id='415140' priv='0' temp='1' game='1' ucnt='1' scnt='0' maxu='2' maxs='0'>\n"
"				<n><![CDATA[PoisonCapricorn5872]]></n>\n"
"			</rm>\n"
"			<rm id='415141' priv='0' temp='1' game='1' ucnt='1' scnt='0' maxu='2' maxs='0'>\n"
"				<n><![CDATA[LightningFireblaze96]]></n>\n"
"			</rm>\n"
"			<rm id='414957' priv='0' temp='1' game='1' ucnt='2' scnt='0' maxu='2' maxs='0'>\n"
"				<n><![CDATA[FuryPhoenix6326]]></n>\n"
"			</rm>\n"
"			<rm id='415037' priv='0' temp='1' game='1' ucnt='1' scnt='0' maxu='2' maxs='0'>\n"
"				<n><![CDATA[NightPhoenix4953]]></n>\n"
"			</rm>\n"
"			<rm id='415018' priv='0' temp='1' game='1' ucnt='2' scnt='0' maxu='2' maxs='0'>\n"
"				<n><![CDATA[ThunderCapricorn8911]]></n>\n"
"			</rm>\n"
"			<rm id='415014' priv='0' temp='1' game='1' ucnt='2' scnt='0' maxu='2' maxs='0'>\n"
"				<n><![CDATA[HyperGemios0201]]></n>\n"
"			</rm>\n"
"			<rm id='415064' priv='0' temp='1' game='1' ucnt='2' scnt='0' maxu='2' maxs='0'>\n"
"				<n><![CDATA[PoisonOrso7613]]></n>\n"
"			</rm>\n"
"			<rm id='415066' priv='0' temp='1' game='1' ucnt='2' scnt='0' maxu='2' maxs='0'>\n"
"				<n><![CDATA[CyberGasher5969]]></n>\n"
"			</rm>\n"
"			<rm id='415103' priv='0' temp='1' game='1' ucnt='2' scnt='0' maxu='2' maxs='0'>\n"
"				<n><![CDATA[EvilCapricorn3876]]></n>\n"
"			</rm>\n"
"			<rm id='415099' priv='0' temp='1' game='1' ucnt='2' scnt='0' maxu='2' maxs='0'>\n"
"				<n><![CDATA[HyperSagittario3381]]></n>\n"
"			</rm>\n"
"			<rm id='415094' priv='0' temp='1' game='1' ucnt='2' scnt='0' maxu='2' maxs='0'>\n"
"				<n><![CDATA[TorchPegasus6723]]></n>\n"
"			</rm>\n"
"			<rm id='415093' priv='0' temp='1' game='1' ucnt='2' scnt='0' maxu='2' maxs='0'>\n"
"				<n><![CDATA[FuryBull4896]]></n>\n"
"			</rm>\n"
"			<rm id='415091' priv='0' temp='1' game='1' ucnt='2' scnt='0' maxu='2' maxs='0'>\n"
"				<n><![CDATA[PoisonPisces8462]]></n>\n"
"			</rm>\n"
"			<rm id='415087' priv='0' temp='1' game='1' ucnt='2' scnt='0' maxu='2' maxs='0'>\n"
"				<n><![CDATA[StormVirgo4082]]></n>\n"
"			</rm>\n"
"			<rm id='415086' priv='0' temp='1' game='1' ucnt='2' scnt='0' maxu='2' maxs='0'>\n"
"				<n><![CDATA[HyperSerpent7260]]></n>\n"
"			</rm>\n"
"			<rm id='415082' priv='0' temp='1' game='1' ucnt='2' scnt='0' maxu='2' maxs='0'>\n"
"				<n><![CDATA[StormPegasus5682]]></n>\n"
"			</rm>\n"
"			<rm id='415077' priv='0' temp='1' game='1' ucnt='2' scnt='0' maxu='2' maxs='0'>\n"
"				<n><![CDATA[HyperBull6472]]></n>\n"
"			</rm>\n"
"		</rmList>\n"
"	</body>\n"
"</msg>\n"
"";


// declare xpath unit test private functions

static int xpath_basicFunctionalityTest();

int main(int argc, char *argv[])
{
	signal_registerDefault();

	printf("[unit] xpath unit test (Asgard Ver %s on %s)\n",
			ASGARD_VERSION, ASGARD_DATE);

	if(xpath_basicFunctionalityTest() < 0) {
		printf("[unit]\t ...FAILED\n");
		return 1;
	}

	printf("[unit] ...PASSED\n");

	return 0;
}


// define xpath unit test private functions

static int xpath_basicFunctionalityTest()
{
	aboolean bValue = afalse;
	int ii = 0;
	int length = 0;
	int iValue = 0;
	double timer = 0.0;
	double dValue = 0.0;
	char *sValue = NULL;
	char *document = NULL;

	Xpath xpath;
	XpathAttributes *attributes = NULL;

	printf("[unit]\t xpath basic functionality test ...\n");

	if(xpath_init(&xpath) < 0) {
		fprintf(stderr, "[%s():%i] error - failed at line %i, aborting.\n",
				__FUNCTION__, __LINE__, __LINE__);
		return -1;
	}

	// parse an XML document

	timer = time_getTimeMus();

	if(xpath_parseXmlDocument(&xpath, BASIC_FUNCTIONALITY_TEST_DOCUMENT,
				strlen(BASIC_FUNCTIONALITY_TEST_DOCUMENT)) < 0) {
		fprintf(stderr, "[%s():%i] error - failed at line %i, aborting.\n",
				__FUNCTION__, __LINE__, __LINE__);
		return -1;
	}

	printf("[unit]\t\t ...XML document parsed in %0.6f seconds\n",
			time_getElapsedMusInSeconds(timer));

	// display the contents of the xpath context

	xpath_display(stdout, &xpath);

	// build an XML document string

	timer = time_getTimeMus();

	if((document = xpath_buildXmlDocudment(&xpath, &length)) == NULL) {
		fprintf(stderr, "[%s():%i] error - failed at line %i, aborting.\n",
				__FUNCTION__, __LINE__, __LINE__);
		return -1;
	}

	printf("%s\n", document);

	free(document);

	printf("[unit]\t\t ...built XML document in %0.6f seconds\n",
			time_getElapsedMusInSeconds(timer));

	// use xpath to obtain a boolean value

	timer = time_getTimeMus();

	bValue = xpath_getBoolean(&xpath,
			"//someOtherXmlDocument",
			strlen("//someOtherXmlDocument"),
			"booleanValue",
			strlen("booleanValue"));

	printf("[unit]\t\t ...executed xpath_getBoolean() in %0.6f seconds\n",
			time_getElapsedMusInSeconds(timer));
	printf("[unit]\t\t xpath_getBoolean() :: '%i'\n", bValue);

	if(bValue != atrue) {
		fprintf(stderr, "[%s():%i] error - failed at line %i, aborting.\n",
				__FUNCTION__, __LINE__, __LINE__);
		return -1;
	}

	// use xpath to obtain a integer value

	timer = time_getTimeMus();

	iValue = xpath_getInteger(&xpath,
			"//someXmlDocument",
			strlen("//someXmlDocument"),
			"integerValue",
			strlen("integerValue"));

	printf("[unit]\t\t ...executed xpath_getInteger() in %0.6f seconds\n",
			time_getElapsedMusInSeconds(timer));
	printf("[unit]\t\t xpath_getInteger() :: '%i'\n", iValue);

	if(iValue != 12345) {
		fprintf(stderr, "[%s():%i] error - failed at line %i, aborting.\n",
				__FUNCTION__, __LINE__, __LINE__);
		return -1;
	}

	// use xpath to obtain a double value

	timer = time_getTimeMus();

	dValue = xpath_getDouble(&xpath,
			"//someXmlDocument",
			strlen("//someXmlDocument"),
			"doubleValue",
			strlen("doubleValue"));

	printf("[unit]\t\t ...executed xpath_getDouble() in %0.6f seconds\n",
			time_getElapsedMusInSeconds(timer));
	printf("[unit]\t\t xpath_getDouble() :: '%0.6f'\n", dValue);

	if(dValue != 12345.6543) {
		fprintf(stderr, "[%s():%i] error - failed at line %i, aborting.\n",
				__FUNCTION__, __LINE__, __LINE__);
		return -1;
	}

	// use xpath to obtain a string

	timer = time_getTimeMus();

	sValue = xpath_getString(&xpath,
			"//nodeServerConfig/httpProperties",
			strlen("//nodeServerConfig/httpProperties"),
			"someStringValue",
			strlen("someStringValue"),
			&length);

	printf("[unit]\t\t ...executed xpath_getString() in %0.6f seconds\n",
			time_getElapsedMusInSeconds(timer));
	printf("[unit]\t\t xpath_getString() :: (%i) => '%s'\n",
			length, sValue);

	if((strcmp(sValue, "this is a string")) || (length != 16)) {
		fprintf(stderr, "[%s():%i] error - failed at line %i, aborting.\n",
				__FUNCTION__, __LINE__, __LINE__);
		return -1;
	}

	free(sValue);

	// use xpath to obtain nodal attributes

	timer = time_getTimeMus();

	attributes = xpath_getAttributes(&xpath,
			"//msg/body/rmList",
			strlen("//msg/body/rmList"),
			"rm",
			strlen("rm"),
			&length);

	printf("[unit]\t\t ...executed xpath_getAttributes() in %0.6f seconds\n",
			time_getElapsedMusInSeconds(timer));
	printf("[unit]\t\t xpath_getAttributes() :: (%i) => 0x%lx\n",
			length, (aptrcast)attributes);

	if((attributes == NULL) || (length != 8)) {
		fprintf(stderr, "[%s():%i] error - failed at line %i, aborting.\n",
				__FUNCTION__, __LINE__, __LINE__);
		return -1;
	}

	for(ii = 0; ii < length; ii++) {
		printf("[unit]\t\t xpath_getAttributes() :: '//msg/body/rmList/rm' "
				"=> '%s'/'%s'\n",
				attributes[ii].name, attributes[ii].value);
	}

	timer = time_getTimeMus();

	attributes = xpath_getAttributes(&xpath,
			"//msg",
			strlen("//msg"),
			"msg",
			strlen("msg"),
			&length);

	printf("[unit]\t\t ...executed xpath_getAttributes() in %0.6f seconds\n",
			time_getElapsedMusInSeconds(timer));
	printf("[unit]\t\t xpath_getAttributes() :: (%i) => 0x%lx\n",
			length, (aptrcast)attributes);

	if((attributes == NULL) || (length != 1)) {
		fprintf(stderr, "[%s():%i] error - failed at line %i, aborting.\n",
				__FUNCTION__, __LINE__, __LINE__);
		return -1;
	}

	for(ii = 0; ii < length; ii++) {
		printf("[unit]\t\t xpath_getAttributes() :: '//msg' => '%s'/'%s'\n",
				attributes[ii].name, attributes[ii].value);
	}

	// use xpath to obtain a delimited set of strings

	timer = time_getTimeMus();

	sValue = xpath_getStrings(&xpath,
			"//nodeServerConfig",
			strlen("//nodeServerConfig"),
			"masterServerAddress",
			strlen("masterServerAddress"),
			"|",
			1,
			&length);

	printf("[unit]\t\t ...executed xpath_getStrings() in %0.6f seconds\n",
			time_getElapsedMusInSeconds(timer));
	printf("[unit]\t\t xpath_getStrings() :: (%i) => '%s'\n",
			length, sValue);

	if((strcmp(sValue, "192.168.0.102|192.168.0.123")) || (length != 27)) {
		fprintf(stderr, "[%s():%i] error - failed at line %i, aborting.\n",
				__FUNCTION__, __LINE__, __LINE__);
		return -1;
	}

	free(sValue);

	// cleanup

	if(xpath_free(&xpath) < 0) {
		fprintf(stderr, "[%s():%i] error - failed at line %i, aborting.\n",
				__FUNCTION__, __LINE__, __LINE__);
		return -1;
	}

	// build an xml document from stratch

	
	if(xpath_init(&xpath) < 0) {
		fprintf(stderr, "[%s():%i] error - failed at line %i, aborting.\n",
				__FUNCTION__, __LINE__, __LINE__);
		return -1;
	}

	if(xpath_setAttribute(&xpath,
				"//newXmlDocument",
				strlen("//newXmlDocument"),
				"newXmlDocument",
				strlen("newXmlDocument"),
				"attrName",
				strlen("attrName"),
				"attrValue",
				strlen("attrValue")) < 0) {
		fprintf(stderr, "[%s():%i] error - failed at line %i, aborting.\n",
				__FUNCTION__, __LINE__, __LINE__);
		return -1;
	}

	if(xpath_setAttribute(&xpath,
				"//newXmlDocument",
				strlen("//newXmlDocument"),
				"newXmlDocument",
				strlen("newXmlDocument"),
				"attrName",
				strlen("attrName"),
				"attrValue",
				strlen("attrValue")) < 0) {
		fprintf(stderr, "[%s():%i] error - failed at line %i, aborting.\n",
				__FUNCTION__, __LINE__, __LINE__);
		return -1;
	}

	if(xpath_setBoolean(&xpath,
				"//newXmlDocument/booleanField",
				strlen("//newXmlDocument/booleanField"),
				"booleanField",
				strlen("booleanField"),
				atrue) < 0) {
		fprintf(stderr, "[%s():%i] error - failed at line %i, aborting.\n",
				__FUNCTION__, __LINE__, __LINE__);
		return -1;
	}

	if(xpath_setInteger(&xpath,
				"//newXmlDocument/integerField",
				strlen("//newXmlDocument/integerField"),
				"integerField",
				strlen("integerField"),
				123456) < 0) {
		fprintf(stderr, "[%s():%i] error - failed at line %i, aborting.\n",
				__FUNCTION__, __LINE__, __LINE__);
		return -1;
	}

	if(xpath_setDouble(&xpath,
				"//newXmlDocument/doubleField",
				strlen("//newXmlDocument/doubleField"),
				"doubleField",
				strlen("doubleField"),
				123.456) < 0) {
		fprintf(stderr, "[%s():%i] error - failed at line %i, aborting.\n",
				__FUNCTION__, __LINE__, __LINE__);
		return -1;
	}

	if(xpath_setString(&xpath,
				"//newXmlDocument/stringField",
				strlen("//newXmlDocument/stringField"),
				"stringField",
				strlen("stringField"),
				"this is a test string",
				strlen("this is a test string")) < 0) {
		fprintf(stderr, "[%s():%i] error - failed at line %i, aborting.\n",
				__FUNCTION__, __LINE__, __LINE__);
		return -1;
	}

	// build an XML document string

	timer = time_getTimeMus();

	if((document = xpath_buildXmlDocudment(&xpath, &length)) == NULL) {
		fprintf(stderr, "[%s():%i] error - failed at line %i, aborting.\n",
				__FUNCTION__, __LINE__, __LINE__);
		return -1;
	}

	printf("%s\n", document);

	free(document);

	printf("[unit]\t\t ...built XML document in %0.6f seconds\n",
			time_getElapsedMusInSeconds(timer));

	// cleanup

	if(xpath_free(&xpath) < 0) {
		fprintf(stderr, "[%s():%i] error - failed at line %i, aborting.\n",
				__FUNCTION__, __LINE__, __LINE__);
		return -1;
	}

	printf("[unit]\t ...PASSED\n");

	return 0;
}

