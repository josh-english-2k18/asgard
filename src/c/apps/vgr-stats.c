/*
 * vgr-stats.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Villa Group statistical-processing application, using Asgard.
 *
 * Written by Josh English.
 */

// preprocessor directives

#define _CORE_COMMON_BYPASS_STD_STREAM
#include "core/core.h"


// define vgr stats constants

//#define DEBUG_MODE											atrue
#define DEBUG_MODE											afalse

//#define VERBOSE_MODE										atrue
#define VERBOSE_MODE										afalse

#define REBUILD_TABLES										atrue
//#define REBUILD_TABLES										afalse

//#define VISITOR_TIMEOUT_SECONDS								1800
#define VISITOR_TIMEOUT_SECONDS								3600

//#define VISITOR_PAGE_TIMEOUT_SECONDS						300
#define VISITOR_PAGE_TIMEOUT_SECONDS						1800

//#define MAX_DAILY_RECORDS									10
#define MAX_DAILY_RECORDS									16
//#define MAX_DAILY_RECORDS									32

#define PERFORM_GEOIP_LOOKUP								atrue
//#define PERFORM_GEOIP_LOOKUP								afalse

#define HTTP_MAX_URL_LENGTH									65536

#define HTTP_MAX_URL_ARG_NAME_LENGTH						128

#define HTTP_MAX_URL_ARG_VALUE_LENGTH						1024

#define HTTP_MAX_URL_ARG_COUNT								64

typedef enum _VgrDateElement {
	VGR_DATE_ELEMENT_YEAR = 0,
	VGR_DATE_ELEMENT_MONTH,
	VGR_DATE_ELEMENT_DAY,
	VGR_DATE_ELEMENT_ERROR = -1
} VgrDateElement;

typedef enum _VgrDestinations {
	VGR_DESTINATION_CABO = 0,
	VGR_DESTINATION_CANCUN,
	VGR_DESTINATION_LORETO,
	VGR_DESTINATION_PUERTO_VALLARTA,
	VGR_DESTINATION_NUEVO_VALLARTA,
	VGR_DESTINATION_ERROR = -1
} VgrDestinations;

typedef enum _VgrProperties {
	VGR_PROPERTY_VLE_CABO = 0,
	VGR_PROPERTY_VDP_CABO,
	VGR_PROPERTY_VILLA_DEL_ARCO,
	VGR_PROPERTY_VDP_LORETO,
	VGR_PROPERTY_VDP_PUERTO_VALLARTA,
	VGR_PROPERTY_VLE_NAYARIT,
	VGR_PROPERTY_FLAMINGOS,
	VGR_PROPERTY_VDP_CANCUN,
	VGR_PROPERTY_ERROR = -1
} VgrProperties;

typedef enum _VgrDailySummaryType {
	VGR_DAILY_SUMMARY_TYPE_CAMPAIGN = 1,
	VGR_DAILY_SUMMARY_TYPE_LOCALE,
	VGR_DAILY_SUMMARY_TYPE_SPIDER,
	VGR_DAILY_SUMMARY_TYPE_URL,
	VGR_DAILY_SUMMARY_TYPE_REFERER,
	VGR_DAILY_SUMMARY_TYPE_REFERER_NORMALIZED,
	VGR_DAILY_SUMMARY_TYPE_EXIT_URL,
	VGR_DAILY_SUMMARY_TYPE_OS,
	VGR_DAILY_SUMMARY_TYPE_SEARCH_ENGINE,
	VGR_DAILY_SUMMARY_TYPE_SEARCH_ENGINE_PHRASE,
	VGR_DAILY_SUMMARY_TYPE_SEARCH_ENGINE_KEYWORD,
	VGR_DAILY_SUMMARY_TYPE_WEB_BROWSER,
	VGR_DAILY_SUMMARY_TYPE_CITY,
	VGR_DAILY_SUMMARY_TYPE_STATE,
	VGR_DAILY_SUMMARY_TYPE_RATE_PLAN,
	VGR_DAILY_SUMMARY_TYPE_ERROR = -1
} VgrDailySummaryType;

typedef enum _VgrWebBrowserTypes {
	WEB_BROWSER_OPERA = 1,
	WEB_BROWSER_OPERA_MINI,
	WEB_BROWSER_WEBTV,
	WEB_BROWSER_IE,
	WEB_BROWSER_POCKET_IE,
	WEB_BROWSER_KONQUEROR,
	WEB_BROWSER_ICAB,
	WEB_BROWSER_OMNIWEB,
	WEB_BROWSER_FIREBIRD,
	WEB_BROWSER_FIREFOX,
	WEB_BROWSER_ICEWEASEL,
	WEB_BROWSER_SHIRETOKO,
	WEB_BROWSER_MOZILLA,
	WEB_BROWSER_AMAYA,
	WEB_BROWSER_LYNX,
	WEB_BROWSER_SAFARI,
	WEB_BROWSER_IPHONE,
	WEB_BROWSER_IPOD,
	WEB_BROWSER_IPAD,
	WEB_BROWSER_CHROME,
	WEB_BROWSER_ANDROID,
	WEB_BROWSER_GOOGLEBOT,
	WEB_BROWSER_SLURP,
	WEB_BROWSER_W3CVALIDATOR,
	WEB_BROWSER_BLACKBERRY,
	WEB_BROWSER_ICECAT,
	WEB_BROWSER_NOKIA_S60,
	WEB_BROWSER_NOKIA,
	WEB_BROWSER_MSN,
	WEB_BROWSER_MSNBOT,
	WEB_BROWSER_NETSCAPE_NAVIGATOR,
	WEB_BROWSER_GALEON,
	WEB_BROWSER_NETPOSITIVE,
	WEB_BROWSER_PHOENIX,
	WEB_BROWSER_YANDEXBOT,
	WEB_BROWSER_WEBSTER_PRO,
	WEB_BROWSER_BAIDU,
	WEB_BROWSER_GOOGLE_APP,
	WEB_BROWSER_CURL,
	WEB_BROWSER_JAVA,
	WEB_BROWSER_PERL,
	WEB_BROWSER_MICROSOFT,
	WEB_BROWSER_MOZILLA_APP,
	WEB_BROWSER_UNKNOWN,
	WEB_BROWSER_ERROR = -1
} VgrWebBrowserTypes;


// define vgr stats data types

typedef struct _VgrStatsCountry {
	int uid;
	char *key;
	char *countryCode;
	char *countryName;
	char *dateCreated;
	char *dateModified;
} VgrStatsCountry;

typedef struct _VgrStatsCity {
	int uid;
	int siteCountryId;
	int siteStateId;
	char *code;
	char *name;
	aulint startIp;
	aulint endIp;
	char *dateCreated;
	char *dateModified;
} VgrStatsCity;

typedef struct _VgrStatsSpider {
	int uid;
	char *key;
	char *name;
	char *description;
	char *userAgent;
	char *isEnabled;
	char *url;
	char *dateCreated;
	char *dateModified;
} VgrStatsSpider;

typedef struct _VgrStatsOS {
	int uid;
	char *name;
	char *description;
} VgrStatsOS;

typedef struct _VgrStatsSearchEngine {
	int uid;
	int keyLength;
	char *key;
	char *name;
} VgrStatsSearchEngine;

typedef struct _VgrWebBrowser {
	int id;
	char *name;
} VgrWebBrowser;

typedef struct _VgrUriArgs {
	int uriArgNameLength;
	char uriArgName[HTTP_MAX_URL_ARG_NAME_LENGTH];
	int uriArgValueLength;
	char uriArgValue[HTTP_MAX_URL_ARG_VALUE_LENGTH];
} VgrUriArgs;

typedef struct _VgrUri {
	int uriPathLength;
	char uriPath[HTTP_MAX_URL_LENGTH];
	int uriFilenameLength;
	char uriFilename[HTTP_MAX_URL_LENGTH];
	int uriQueryStringLength;
	char uriQueryString[HTTP_MAX_URL_LENGTH];
	int uriArgCount;
	VgrUriArgs args[HTTP_MAX_URL_ARG_COUNT];
} VgrUri;

typedef struct _VgrStatsVisitorDailyRecords {
	aboolean isConverted;
	int pageCount;
	int trackingTimestamp;
	int timeOnSite;
	int dailySecondStamp;
	int dailyHourStamp;
	int exitUrlCounter;
	int destinationId;
	int propertyId;
	int ratePlanId;
	char *referer;
	char *normalizedReferer;
	char *exitUrl;
	char *searchEngine;
	char *searchPhrase;
} VgrStatsVisitorDailyRecords;

typedef struct _VgrStatsVisitor {
	aboolean hasSpiderCheck;
	int siteTrackingSpiderId;
	int uid;
	int siteTrackingCampaignId;
	int siteUserId;
	int visits;
	char *key;
	char *hash;
	char *platform;
	char *userAgent;
	int browserId;
	char *browserName;
	char *clientIp;
	char *countryCode;
	char *countryName;
	char *dateCreated;
	char *dateModified;
	int siteStateId;
	int siteCityId;
	VgrStatsVisitorDailyRecords daily;
} VgrStatsVisitor;

typedef struct _VgrStatsDetail {
	int uid;
	int siteTrackingVisitorId;
	char *key;
	char *host;
	char *hostname;
	char *url;
	char *normalizedUrl;
	char *referer;
	char *normalizedReferer;
	char *dateCreated;
	char *dateModified;
} VgrStatsDetail;

typedef struct _VgrStatsTrackingOverview {
	int uniqueVisitors;
	int visits;
	int pageCount;
	int conversions;
} VgrStatsTrackingOverview;

typedef struct _VgrStatsDailySummary {
	int type;
	int linkId;
	int linkIdTwo;
	int uniqueVisitors;
	int visits;
	int pageCount;
	int conversions;
	int bounceCount;
	int totalTimeOnSite;
	char *url;
	char *referer;
	char *dateStamp;
} VgrStatsDailySummary;

typedef struct _VgrStats {
	int cityListLength;
	int visitorListLength;
	int spiderListLength;
	VgrStatsTrackingOverview trackingOverview;
	VgrStatsCity **cityList;
	VgrStatsVisitor **visitorList;
	VgrStatsSpider **spiderList;
	Bptree cities;
	Bptree citiesById;
	Bptree countries;
	Bptree countriesById;
	Bptree visitors;
	Bptree wordList;
	FileHandle *stcampaign;
	FileHandle stlocale;
	FileHandle stspider;
	FileHandle sturl;
	FileHandle streferer;
	FileHandle stdomain;
	FileHandle stexiturl;
	FileHandle stvtr;
	FileHandle stos;
	FileHandle stse;
	FileHandle stsp;
	FileHandle stsk;
	FileHandle stwb;
	FileHandle stcity;
	FileHandle ststate;
	FileHandle strp;
} VgrStats;


// define vgr stats static data types

static VgrStatsSearchEngine SEARCH_ENGINE_LIST[] = {
	{	1,		0,		"google.com",					"Google"		},
	{	2,		0,		"bing.com",						"Bing"			},
	{	3,		0,		"search.aol.com",				"AOL"			},
	{	4,		0,		"search.yahoo.com",				"Yahoo"			},
	{	5,		0,		"ask.com",						"Ask"			},
	{	6,		0,		"search.isp.netscape.com",		"Netscape"		},
	{	7,		0,		"snap.com",						"Snap"			},
	{	8,		0,		"gigablast.com",				"Gigablast"		},
	{	-1,		0,		NULL,							NULL			}
};

static VgrWebBrowser WEB_BROWSER_LIST[] = {
	{	WEB_BROWSER_OPERA,					"Opera"						},
	{	WEB_BROWSER_OPERA_MINI,				"Opera Mini"				},
	{	WEB_BROWSER_WEBTV,					"WebTV"						},
	{	WEB_BROWSER_IE,						"Internet Explorer"			},
	{	WEB_BROWSER_POCKET_IE,				"Pocket Internet Explorer"	},
	{	WEB_BROWSER_KONQUEROR,				"Konqueror"					},
	{	WEB_BROWSER_ICAB,					"iCab"						},
	{	WEB_BROWSER_OMNIWEB,				"OmniWeb"					},
	{	WEB_BROWSER_FIREBIRD,				"Firebird"					},
	{	WEB_BROWSER_FIREFOX,				"Firefox"					},
	{	WEB_BROWSER_ICEWEASEL,				"Iceweasel"					},
	{	WEB_BROWSER_SHIRETOKO,				"Shiretoko"					},
	{	WEB_BROWSER_MOZILLA,				"Mozilla"					},
	{	WEB_BROWSER_AMAYA,					"Amaya"						},
	{	WEB_BROWSER_LYNX,					"Lynx"						},
	{	WEB_BROWSER_SAFARI,					"Safari"					},
	{	WEB_BROWSER_IPHONE,					"iPhone"					},
	{	WEB_BROWSER_IPOD,					"iPod"						},
	{	WEB_BROWSER_IPAD,					"iPad"						},
	{	WEB_BROWSER_CHROME,					"Chrome"					},
	{	WEB_BROWSER_ANDROID,				"Android"					},
	{	WEB_BROWSER_GOOGLEBOT,				"GoogleBot"					},
	{	WEB_BROWSER_SLURP,					"Yahoo! Slurp"				},
	{	WEB_BROWSER_W3CVALIDATOR,			"W3C Validator"				},
	{	WEB_BROWSER_BLACKBERRY,				"BlackBerry"				},
	{	WEB_BROWSER_ICECAT,					"IceCat"					},
	{	WEB_BROWSER_NOKIA_S60,				"Nokia S60 OSS Browser"		},
	{	WEB_BROWSER_NOKIA,					"Nokia Browser"				},
	{	WEB_BROWSER_MSN,					"MSN Browser"				},
	{	WEB_BROWSER_MSNBOT,					"MSN Bot"					},
	{	WEB_BROWSER_NETSCAPE_NAVIGATOR,		"Netscape Navigator"		},
	{	WEB_BROWSER_GALEON,					"Galeon"					},
	{	WEB_BROWSER_NETPOSITIVE,			"NetPositive"				},
	{	WEB_BROWSER_PHOENIX,				"Phoenix"					},
	{	WEB_BROWSER_YANDEXBOT,				"YandexBot"					},
	{	WEB_BROWSER_WEBSTER_PRO,			"Webster Pro Control"		},
	{	WEB_BROWSER_BAIDU,					"BaiduBot"					},
	{	WEB_BROWSER_GOOGLE_APP,				"Google Application"		},
	{	WEB_BROWSER_CURL,					"Curl"						},
	{	WEB_BROWSER_JAVA,					"Java"						},
	{	WEB_BROWSER_PERL,					"Perl"						},
	{	WEB_BROWSER_MICROSOFT,				"Microsoft Application"		},
	{	WEB_BROWSER_MOZILLA_APP,			"Mozilla Compatible"		},
	{	WEB_BROWSER_UNKNOWN,				"Unknown"					},
	{	WEB_BROWSER_ERROR,					NULL						}
};

static char *ENGLISH_KEYWORD_LIST[] = {
	"the",
	"of",
	"and",
	"a",
	"to",
	"in",
	"is",
	"you",
	"that",
	"it",
	"he",
	"was",
	"for",
	"on",
	"are",
	"as",
	"with",
	"his",
	"they",
	"I",
	"at",
	"be",
	"this",
	"have",
	"from",
	"or",
	"one",
	"had",
	"by",
	"word",
	"but",
	"not",
	"what",
	"all",
	"were",
	"we",
	"when",
	"your",
	"can",
	"said",
	"there",
	"use",
	"an",
	"each",
	"which",
	"she",
	"do",
	"how",
	"their",
	"if",
	"will",
	"up",
	"other",
	"about",
	"out",
	"many",
	"then",
	"them",
	"these",
	"so",
	"some",
	"her",
	"would",
	"make",
	"like",
	"him",
	"into",
	"time",
	"has",
	"look",
	"two",
	"more",
	"write",
	"go",
	"see",
	"number",
	"no",
	"way",
	"could",
	"people",
	"my",
	"than",
	"first",
	"water",
	"been",
	"call",
	"who",
	"oil",
	"its",
	"now",
	"find",
	"long",
	"down",
	"day",
	"did",
	"get",
	"come",
	"made",
	"may",
	"part",
	NULL
};


// declare vgr stats functions

// general functions

static void free_nothing(void *memory);

static int getElementFromDateStamp(char *dateStamp, int dateStampLength,
		int element);

static int getDateStampSeconds(char *dateStamp, int dateStampLength,
		int *hourValue);

static char *dateStampFromNumbers(int year, int month, int day);

static char *getNormalizedString(char *string, int *resultLength);

static char *getNormalizedUrl(char *url);

static char *getNormalizedReferer(char *referer);

int decodeUrl(char url[HTTP_MAX_URL_LENGTH], int urlLength,
		char decodedUrl[HTTP_MAX_URL_LENGTH], int *decodedUrlLength,
		VgrUri *uri);

static char **stringTokenize(char *source, int sourceLength,
		char *delimiterList, int delimiterLength,
		int minimumTokenLength,
		aboolean recognizeQuotes,
		int *tokenCount);

static char **buildKeywordList(char *phrase, int phraseLength,
		int *keywordListLength);

aulint ipToNumber(char *ip);

char *sqlStrdup(char *string);

// country functions

static VgrStatsCountry *country_buildCountry(char *line, int lineLength);

static void country_free(void *memory);

static void country_display(VgrStatsCountry *country);

static int country_analyzeCountryFile(char *filename);

// city functions

static VgrStatsCity *city_buildCity(char *line, int lineLength);

static void city_free(void *memory);

static void city_display(VgrStatsCity *city);

static int city_analyzeCityFile(char *filename);

// spider functions

static VgrStatsSpider *spider_buildSpider(char *line, int lineLength);

static void spider_free(void *memory);

static void spider_display(VgrStatsSpider *spider);

static int spider_analyzeSpiderFile(char *filename);

// visitor functions

static int visitor_getBrowser(char *userAgent);

static VgrStatsVisitor *visitor_buildVisitor(char *line, int lineLength);

static void visitor_free(void *memory);

static void visitor_display(VgrStatsVisitor *visitor);

static int visitor_analyzeVisitorFile(char *filename);

// detail functions

static VgrStatsDetail *detail_buildDetail(char *line, int lineLength);

static void detail_free(void *memory);

static void detail_display(VgrStatsDetail *detail);

static void daily_visitorFree(void *memory);

static void daily_summaryFree(void *memory);

static int daily_getDestinationUid(VgrDestinations destination);

static int daily_getPropertyUid(VgrProperties property);

static void detail_getDailyVisitorInfo(int arrayLength, void **values,
		int *bounceCount, int *totalTimeOnSite);

static int detail_getSpiderUidFromVisitor(VgrStatsVisitor *visitor);

static int detail_descendingVgrStatsDailySummaryComparison(const void *left,
		const void *right);

static int detail_buildDestinationAndPropertyUrlSummaries(int arrayLength,
		void **values);

static void detail_freeOperatingSystem(void *memory);

static int detail_ascendingVgrStatsOsComparison(const void *left,
		const void *right);

static int detail_analyzeDetailFile(char *filename);

// tracking functions

static void tracking_displayOverview();


// define vgr stats global variables

static VgrStats *context = NULL;


// main function

int main(int argc, char *argv[])
{
	int ii = 0;
	int keyLength = 0;
	char *key = NULL;
	char *countryFilename = NULL;
	char *cityFilename = NULL;
	char *spiderFilename = NULL;
	char *visitorFilename = NULL;
	char *detailFilename = NULL;

	signal_registerDefault();

	// check argument(s)

	if((argc > 1) && (argv[1] != NULL)) {
		countryFilename = argv[1];
	}
	if((argc > 2) && (argv[2] != NULL)) {
		cityFilename = argv[2];
	}
	if((argc > 3) && (argv[3] != NULL)) {
		spiderFilename = argv[3];
	}
	if((argc > 4) && (argv[4] != NULL)) {
		visitorFilename = argv[4];
	}
	if((argc > 5) && (argv[5] != NULL)) {
		detailFilename = argv[5];
	}

	if((countryFilename == NULL) ||
			(cityFilename == NULL) ||
			(spiderFilename == NULL) ||
			(visitorFilename == NULL) ||
			(detailFilename == NULL)) {
		fprintf(stderr, "error - invalid or missing argument(s).\n");
		fprintf(stderr, "usage: vgr-stats "
				"[country file] "
				"[city file] "
				"[spider file] "
				"[visitor file] "
				"[detail file]\n");
		return 1;
	}

	// initialize context

	context = (VgrStats *)malloc(sizeof(VgrStats));

	context->cityListLength = 0;
	context->cityList = NULL;

	context->visitorListLength = 0;
	context->visitorList = NULL;

	context->spiderListLength = 0;
	context->spiderList = NULL;

	bptree_init(&context->cities);
	bptree_setFreeFunction(&context->cities, city_free);

	bptree_init(&context->citiesById);
	bptree_setFreeFunction(&context->citiesById, free_nothing);

	bptree_init(&context->countries);
	bptree_setFreeFunction(&context->countries, country_free);

	bptree_init(&context->countriesById);
	bptree_setFreeFunction(&context->countriesById, free_nothing);

	bptree_init(&context->visitors);
	bptree_setFreeFunction(&context->visitors, visitor_free);

	bptree_init(&context->wordList);

	for(ii = 0; ENGLISH_KEYWORD_LIST[ii] != NULL; ii++) {
		key = getNormalizedString(ENGLISH_KEYWORD_LIST[ii], &keyLength);

		bptree_put(&context->wordList, key, keyLength, key);
	}

	// analyze country file

	if(country_analyzeCountryFile(countryFilename) < 0) {
		return 1;
	}

	// analyze city file

	if(city_analyzeCityFile(cityFilename) < 0) {
		return 1;
	}

	// analyze spider file

	if(spider_analyzeSpiderFile(spiderFilename) < 0) {
		return 1;
	}

	// analyze visitor file

	if(visitor_analyzeVisitorFile(visitorFilename) < 0) {
		return 1;
	}

	tracking_displayOverview();

	// analyze detail file

	if(detail_analyzeDetailFile(detailFilename) < 0) {
		return 1;
	}

	tracking_displayOverview();

	// free context

	if(context->cityList != NULL) {
		free(context->cityList);
	}

	if(context->visitorList != NULL) {
		free(context->visitorList);
	}

	if(context->spiderList != NULL) {
		for(ii = 0; ii < context->spiderListLength; ii++) {
			if(context->spiderList[ii] != NULL) {
				spider_free(context->spiderList[ii]);
			}
		}

		free(context->spiderList);
	}

	bptree_free(&context->cities);
	bptree_free(&context->citiesById);

	bptree_free(&context->countries);
	bptree_free(&context->countriesById);

	bptree_free(&context->visitors);

	bptree_free(&context->wordList);

	memset(context, 0, (int)(sizeof(VgrStats)));

	free(context);

	return 0;
}


// define vgr stats functions

// general functions

static void free_nothing(void *memory)
{
	// do nothing
}

static int getElementFromDateStamp(char *dateStamp, int dateStampLength,
		int element)
{
	int ii = 0;
	int nn = 0;
	int result = 0;
	int counter = 0;
	char *number = NULL;

	number = (char *)malloc(sizeof(char) * dateStampLength);

	for(ii = 0, nn = 0;
			((ii < dateStampLength) && (nn < dateStampLength));
			ii++) {
		if(dateStamp[ii] == ' ') {
			break;
		}
		else if(dateStamp[ii] == '-') {
			counter++;
			continue;
		}

		if(counter == element) {
			number[nn] = dateStamp[ii];
			nn++;
		}
		else if(counter > element) {
			break;
		}
	}

	if(nn <= 0) {
		result = -1;
	}
	else {
		result = atoi(number);
	}

	free(number);

	return result;
}

static int getDateStampSeconds(char *dateStamp, int dateStampLength,
		int *hourValue)
{
	aboolean begin = afalse;
	int ii = 0;
	int nn = 0;
	int result = 0;
	int counter = 0;
	char *hourNumber = NULL;
	char *minuteNumber = NULL;
	char *secondNumber = NULL;

	*hourValue = 0;

	hourNumber = (char *)malloc(sizeof(char) * dateStampLength);
	minuteNumber = (char *)malloc(sizeof(char) * dateStampLength);
	secondNumber = (char *)malloc(sizeof(char) * dateStampLength);

	for(ii = 0, nn = 0;
			((ii < dateStampLength) && (nn < dateStampLength));
			ii++) {
		if(dateStamp[ii] == ' ') {
			begin = atrue;
			continue;
		}
		else if(dateStamp[ii] == '.') {
			break;
		}
		else if(dateStamp[ii] == ':') {
			nn = 0;
			counter++;
			continue;
		}

		if(begin) {
			switch(counter) {
				case 0:
					hourNumber[nn] = dateStamp[ii];
					nn++;
					break;

				case 1:
					minuteNumber[nn] = dateStamp[ii];
					nn++;
					break;

				case 2:
					secondNumber[nn] = dateStamp[ii];
					nn++;
					break;
			}
		}
	}

	if((counter == 2) && (nn > 0)) {
		*hourValue = atoi(hourNumber);

		result = (
				(atoi(hourNumber) * 3600) +
				(atoi(minuteNumber) * 60) +
				(atoi(secondNumber))
				);
//printf("%s() :: ('%s', '%s', '%s') :: %i = ((%i * 3600) + (%i * 60) + %i) :: '%s'\n", __FUNCTION__, hourNumber, minuteNumber, secondNumber, result, atoi(hourNumber), atoi(minuteNumber), atoi(secondNumber), dateStamp);
	}
	else {
		result = -1;
	}

	free(hourNumber);
	free(minuteNumber);
	free(secondNumber);

	return result;
}

static char *dateStampFromNumbers(int year, int month, int day)
{
	char *result = NULL;

	result = (char *)malloc(sizeof(char) * 16);

	snprintf(result, ((int)(sizeof(char) * 16) - 1), "%04i-%02i-%02i",
			year, month, day);

	return result;
}

static char *getNormalizedString(char *string, int *resultLength)
{
	int ii = 0;
	int nn = 0;
	int length = 0;
	char *result = NULL;

	*resultLength = 0;

	length = strlen(string);

	result = (char *)malloc(sizeof(char) * (length + 2));

	for(ii = 0, nn = 0; ((ii < length) && (nn < length)); ii++) {
		if((ctype_isAlphabetic(string[ii])) ||
				(ctype_isNumeric(string[ii])) ||
				(string[ii] == ' ') ||
				(string[ii] == '.')) {
			if(ctype_isUpper(string[ii])) {
				result[nn] = ctype_toLower(string[ii]);
				nn++;
			}
			else if(ctype_isPlainText(string[ii])) {
				result[nn] = string[ii];
				nn++;
			}
		}
	}

	*resultLength = nn;

	return result;
}

static char *getNormalizedUrl(char *url)
{
	int ii = 0;
	int nn = 0;
	int length = 0;
	int counter = 0;
	char *result = NULL;

	length = strlen(url);
	result = (char *)malloc(sizeof(char) * (length + 1));

	for(ii = 0, nn = 0; ((ii < length) && (nn < length)); ii++) {
		if((url[ii] == '#') ||
				(url[ii] == '?') ||
				(url[ii] == '&') ||
				(url[ii] == '%')) {
			break;
		}

		if(url[ii] == '/') {
			counter++;
		}

		if(counter >= 3) {
			if(ctype_isUpper(url[ii])) {
				result[nn] = ctype_toLower(url[ii]);
				nn++;
			}
			else if(ctype_isPlainText(url[ii])) {
				result[nn] = ctype_toLower(url[ii]);
				nn++;
			}
		}
	}

	if((nn > 1) && (result[(nn - 1)] == '/')) {
		result[(nn - 1)] = '\0';
	}

	return result;
}

static char *getNormalizedReferer(char *referer)
{
	int ii = 0;
	int nn = 0;
	int length = 0;
	int counter = 0;
	char *result = NULL;

	length = strlen(referer);
	result = (char *)malloc(sizeof(char) * (length + 1));

	for(ii = 0, nn = 0; ((ii < length) && (nn < length)); ii++) {
		if((referer[ii] == '#') ||
				(referer[ii] == '?') ||
				(referer[ii] == '&') ||
				(referer[ii] == '%')) {
			break;
		}

		if(referer[ii] == '/') {
			counter++;
			continue;
		}

		if(counter == 2) {
			if(ctype_isUpper(referer[ii])) {
				result[nn] = ctype_toLower(referer[ii]);
				nn++;
			}
			else if(ctype_isPlainText(referer[ii])) {
				result[nn] = ctype_toLower(referer[ii]);
				nn++;
			}
		}
		else if(counter > 2) {
			break;
		}
	}

	if((nn > 1) && (result[(nn - 1)] == '/')) {
		result[(nn - 1)] = '\0';
	}

	return result;
}

int decodeUrl(char url[HTTP_MAX_URL_LENGTH], int urlLength,
		char decodedUrl[HTTP_MAX_URL_LENGTH], int *decodedUrlLength,
		VgrUri *uri)
{
	aboolean isName = afalse;
	aboolean badChar = afalse;
	aboolean endOfFilename = afalse;
	aboolean endOfQueryString = afalse;
	int ii = 0;
	int nn = 0;
	int jj = 0;
	int kk = 0;
	int mm = 0;
	int power = 0;
	int lastPathDelimiterRef = 0;
	char byte = (char)0;

	if ((urlLength < 1) || (decodedUrlLength == NULL) || (uri == NULL)) {
		fprintf(stderr, "[%s():%i] error - invalid or missing argument(s).\n",
				__FUNCTION__, __LINE__);
		return -1;
	}

	memset(decodedUrl, 0, HTTP_MAX_URL_LENGTH);
	memset(uri, 0, sizeof(VgrUri));

	*decodedUrlLength = 0;

	for(ii = 0, nn = 0, jj = 0, mm = 0; ii < urlLength; ii++, nn++, jj++) {
		if (url[ii] == '+') {
			decodedUrl[nn] = ' ';
		}
		else if (url[ii] == '%') {
			power = 16;	// change if other than two digits to process
			byte = (char)0;
			badChar = afalse;
			for(kk = ii + 1; kk < (ii + 3); kk++) {
				if ((url[kk] >= '0') && (url[kk] <= '9')) {
					byte += (url[kk] - '0') * power;
				}
				else if ((url[kk] >= 'a') && (url[kk] <= 'f')) {
					byte += ((url[kk] - 'a') + 10) * power;
				}
				else if ((url[kk] >= 'A') && (url[kk] <= 'F')) {
					byte += ((url[kk] - 'A') + 10) * power;
				}
				else {
					badChar = atrue;
				}
				power /= 16;
			}
			if (badChar) {
				decodedUrl[nn] = url[ii];
			}
			else {
				decodedUrl[nn] = byte;
				ii += 2;
			}
		}
		else {
			decodedUrl[nn] = url[ii];
		}

		if((endOfFilename) && (!endOfQueryString)) {
			uri->uriQueryString[mm] = decodedUrl[nn];
			mm++;
		}

		if ((!endOfFilename) &&
				((decodedUrl[nn] == ';') ||
				 (decodedUrl[nn] == '?') ||
				 (decodedUrl[nn] == '#') ||
				 (decodedUrl[nn] == '&'))) {
			endOfFilename = atrue;
			mm = 0;
		}

		if ((endOfFilename) && (!endOfQueryString) &&
				(decodedUrl[nn] == '#')) {
			endOfQueryString = atrue;
			uri->uriQueryStringLength = mm;
			uri->uriQueryString[mm] = '\0';
			mm = 0;
		}

		if (!endOfFilename) {
			uri->uriFilename[jj] = decodedUrl[nn];
		}

		if (decodedUrl[nn] == '/') {
			lastPathDelimiterRef = nn;
			if (!endOfFilename) {
				jj = -1;
			}
		}

		uri->uriPath[nn] = decodedUrl[nn];
	}

	uri->uriPathLength = (lastPathDelimiterRef + 1);
	uri->uriPath[(lastPathDelimiterRef + 1)] = '\0';

	if(jj >= 0) {
		uri->uriFilenameLength = jj;
		uri->uriFilename[jj] = '\0';
	}

	if(!endOfQueryString) {
		uri->uriQueryStringLength = mm;
		uri->uriQueryString[mm] = '\0';
	}

	*decodedUrlLength = nn;

	// create query string argument(s) list

	isName = atrue;

	for(ii = 0, nn = 0, jj = 0; ii < uri->uriQueryStringLength; ii++) {
		if(uri->uriQueryString[ii] == '=') {
			if(isName) {
				isName = afalse;
				uri->args[jj].uriArgName[nn] = '\0';
				uri->args[jj].uriArgNameLength = nn;
				nn = 0;
				continue;
			}
		}
		else if(uri->uriQueryString[ii] == '&') {
			if(!isName) {
				uri->args[jj].uriArgValue[nn] = '\0';
				uri->args[jj].uriArgValueLength = nn;
			}

			isName = atrue;
			nn = 0;

			jj++;
			if(jj >= HTTP_MAX_URL_ARG_COUNT) {
				break;
			}

			continue;
		}

		if(isName) {
			uri->args[jj].uriArgName[nn] = uri->uriQueryString[ii];
			nn++;
			if(nn >= HTTP_MAX_URL_ARG_NAME_LENGTH) {
				break;
			}
		}
		else {
			uri->args[jj].uriArgValue[nn] = uri->uriQueryString[ii];
			nn++;
			if(nn >= HTTP_MAX_URL_ARG_VALUE_LENGTH) {
				break;
			}
		}
	}

	if(uri->uriQueryStringLength > 0) {
		uri->uriArgCount = (jj + 1);
	}

	return 0;
}

static char **stringTokenize(char *source, int sourceLength,
		char *delimiterList, int delimiterLength,
		int minimumTokenLength,
		aboolean recognizeQuotes,
		int *tokenCount)
{
	aboolean found = afalse;
	aboolean insideQuotes = afalse;
	int ii = 0;
	int jj = 0;
	int kk = 0;
	int ref = 0;
	int resultRef = 0;
	int tokenLength = 0;
	int tokenListLength = 0;
	char *token = NULL;
	char **result = NULL;

	if((source == NULL) ||
			(sourceLength < 0) ||
			(delimiterList == NULL) ||
			(delimiterLength < 0) ||
			(minimumTokenLength < 0) ||
			(tokenCount == NULL)) {
		DISPLAY_INVALID_ARGS;
		if(tokenCount != NULL) {
			*tokenCount = -1;
		}
		return NULL;
	}

	*tokenCount = 0;

	// determine the number of tokens in the source string

	tokenListLength = 0;

	for(ii = 0, ref = 0; ii < sourceLength; ii++) {
		if((recognizeQuotes) && (source[ii] == '"')) {
			if(insideQuotes) {
				insideQuotes = afalse;
				if((ii + 1) < sourceLength) {
					continue;
				}
			}
			else {
				insideQuotes = atrue;
				continue;
			}
		}

		found = afalse;

		if((recognizeQuotes) && (insideQuotes)) {
			if((ii + 1) == sourceLength) {
				found = atrue;
				tokenLength = ((ii - ref) + 1);
			}
		}
		else {
			for(jj = 0; jj < delimiterLength; jj++) {
				if(source[ii] == delimiterList[jj]) {
					found = atrue;
					tokenLength = (ii - ref);
					break;
				}
			}

			if(!found) {
				if((ii + 1) == sourceLength) {
					found = atrue;
					tokenLength = ((ii - ref) + 1);
				}
			}
		}

		if(found) {
			if(tokenLength >= minimumTokenLength) {
				tokenListLength += 1;
			}
			ref = (ii + 1);
		}
	}

	if(tokenListLength == 0) {
		return NULL;
	}

	// execute source tokenization

	result = (char **)malloc(sizeof(char *) * tokenListLength);

	insideQuotes = afalse;

	for(ii = 0, ref = 0; ii < sourceLength; ii++) {
		if((recognizeQuotes) && (source[ii] == '"')) {
			if(insideQuotes) {
				insideQuotes = afalse;
				if((ii + 1) < sourceLength) {
					continue;
				}
			}
			else {
				insideQuotes = atrue;
				continue;
			}
		}

		found = afalse;

		if((recognizeQuotes) && (insideQuotes)) {
			if((ii + 1) == sourceLength) {
				found = atrue;
				tokenLength = ((ii - ref) + 1);
			}
		}
		else {
			for(jj = 0; jj < delimiterLength; jj++) {
				if(source[ii] == delimiterList[jj]) {
					found = atrue;
					tokenLength = (ii - ref);
					break;
				}
			}

			if(!found) {
				if((ii + 1) == sourceLength) {
					found = atrue;
					tokenLength = ((ii - ref) + 1);
				}
			}
		}

		if(found) {
			if(tokenLength >= minimumTokenLength) {
				token = malloc(sizeof(char) * (tokenLength + 1));

				for(kk = 0; kk < tokenLength; kk++, ref++) {
					token[kk] = source[ref];
				}

				result[resultRef] = token;

				resultRef++;

				if(resultRef >= tokenListLength) {
					break;
				}
			}

			ref = (ii + 1);
		}
	}

	if(DEBUG_MODE) {
		for(ii = 0; ii < resultRef; ii++) {
			printf("[PARSED] TOKEN[%03i] :: '%s'\n", ii, result[ii]);
		}
	}

	*tokenCount = resultRef;

	return result;
}

static char **buildKeywordList(char *phrase, int phraseLength,
		int *keywordListLength)
{
	aboolean hasVowel = afalse;
	aboolean hasChanged = afalse;
	int ii = 0;
	int nn = 0;
	int jj = 0;
	int rc = 0;
	int tokenCount = 0;
	int tokenLength = 0;
	int resultRef = 0;
	char *token = NULL;
	char *keyword = NULL;
	char **tokens = NULL;
	char **result = NULL;

	static int delimiterLength = 0;

	const int MINIMUM_TOKEN_LENGTH = 3;

	const char DELIMITERS[] = { ' ', '\t', '\0' };

	*keywordListLength = 0;

	if((phrase == NULL) || (phraseLength < 1)) {
		return NULL;
	}

	if(delimiterLength == 0) {
		delimiterLength = strlen(DELIMITERS);
	}

	tokens = stringTokenize(phrase, phraseLength,
			(char *)DELIMITERS, delimiterLength,
			MINIMUM_TOKEN_LENGTH,
			afalse,
			&tokenCount);

	if((tokens == NULL) || (tokenCount < 1)) {
		rc = -1;
	}
	else {
		resultRef = 0;
		result = (char **)malloc(sizeof(char *) * tokenCount);

		for(ii = 0; ii < tokenCount; ii++) {
			tokenLength = strlen(tokens[ii]);

			if((tokenLength < 3) || (tokenLength > 28)) {
				continue;
			}

			token = (char *)malloc(sizeof(char) * (tokenLength + 1));

			hasVowel = afalse;

			for(nn = 0, jj = 0; nn < tokenLength; nn++) {
				if((ctype_isAlphabetic(tokens[ii][nn])) ||
						(ctype_isNumeric(tokens[ii][nn])) ||
						(tokens[ii][nn] == '.')) {
					token[jj] = ctype_toLower(tokens[ii][nn]);

					if((token[jj] == 'a') ||
							(token[jj] == 'e') ||
							(token[jj] == 'i') ||
							(token[jj] == 'o') ||
							(token[jj] == 'u') ||
							(token[jj] == 'y')) {
						hasVowel = atrue;
					}

					jj++;
				}
			}

			if((!hasVowel) || (jj < 3) || (jj > 28)) {
				free(token);
				continue;
			}

			hasChanged = afalse;

			if((token[(jj - 3)] == 'i') &&
					(token[(jj - 2)] == 'e') &&
					(token[(jj - 1)] == 's')) {
				token[(jj - 3)] = 'y';
				token[(jj - 2)] = '\0';
				jj -= 2;
				hasChanged = atrue;
			}
			else if((token[(jj - 2)] == 'e') && (token[(jj - 1)] == 's')) {
				token[(jj - 2)] = '\0';
				jj -= 2;
				hasChanged = atrue;
			}
			else if(token[(jj - 1)] == 's') {
				token[(jj - 1)] = '\0';
				jj -= 1;
				hasChanged = atrue;
			}

			if(hasChanged) {
				if((jj < 3) || (jj > 28)) {
					free(token);
					continue;
				}

				hasVowel = afalse;

				for(nn = 0; nn < jj; nn++) {
					if((token[nn] == 'a') ||
							(token[nn] == 'e') ||
							(token[nn] == 'i') ||
							(token[nn] == 'o') ||
							(token[nn] == 'u') ||
							(token[nn] == 'y')) {
						hasVowel = atrue;
						break;
					}
				}

				if(!hasVowel) {
					free(token);
					continue;
				}
			}

			keyword = NULL;

			if((bptree_get(&context->wordList, token, strlen(token),
							(void *)&keyword) == 0) ||
					(keyword != NULL)) {
				free(token);
				continue;
			}

			result[resultRef] = token;
			resultRef++;
		}
	}

	if(tokens != NULL) {
		for(ii = 0; ii < tokenCount; ii++) {
			if(tokens[ii] != NULL) {
				free(tokens[ii]);
			}
		}

		free(tokens);
	}

	if(rc == -1) {
		return NULL;
	}

	*keywordListLength = resultRef;

	return result;
}

aulint ipToNumber(char *ip)
{
	int ii = 0;
	int nn = 0;
	int length = 0;
	int counter = 0;
	char token[128];
	aulint value = 0;
	aulint result = 0;

	aulint factors[4] = { 
		16777216,  // 256 ^ 3
		65536,     // 256 ^ 2
		256,       // 256 ^ 1
		1          // 256 ^ 0
	};

	length = strlen(ip);

	memset(token, 0, (int)(sizeof(token)));

	for(ii = 0, nn = 0, counter = 0; ii < length; ii++) {
		if(ip[ii] == '.') {
			token[nn] = '\0';

			value = (aulint)atod(token);
			result += (value * factors[counter]);

			counter++;
			if(counter > 3) {
				break;
			}

			memset(token, 0, (int)(sizeof(token)));
			nn = 0;
		}
		else {
			token[nn] = ip[ii];
			nn++;
		}
	}

	if(counter == 3) {
		token[nn] = '\0';
		value = (aulint)atod(token);
		result += (value * factors[counter]);
	}

	return result;
}

char *sqlStrdup(char *string)
{
	int ii = 0;
	int nn = 0;
	int length = 0;
	char *result = NULL;

	length = strlen(string);

	result = (char *)malloc(sizeof(char) * ((length * 2) + 1));

	for(ii = 0, nn = 0; ii < length; ii++) {
		if(string[ii] == '\'') {
			result[nn] = '\'';
			nn++;
		}

		result[nn] = string[ii];
		nn++;
	}

	return result;
}

// country functions

static VgrStatsCountry *country_buildCountry(char *line, int lineLength)
{
	int ii = 0;
	int rc = 0;
	int tokenCount = 0;
	char **tokens = NULL;

	VgrStatsCountry *result = NULL;

	static int delimiterLength = 0;

	const int MINIMUM_TOKEN_LENGTH = 1;

	const char DELIMITERS[] = { '\t', '\0' };

	if((line == NULL) || (lineLength < 1)) {
		return NULL;
	}

	if(delimiterLength == 0) {
		delimiterLength = strlen(DELIMITERS);
	}

	tokens = stringTokenize(line, lineLength,
			(char *)DELIMITERS, delimiterLength,
			MINIMUM_TOKEN_LENGTH,
			afalse,
			&tokenCount);

	if((tokens == NULL) || (tokenCount < 5)) {
		rc = -1;
	}

	if(rc == 0) {
		result = (VgrStatsCountry *)malloc(sizeof(VgrStatsCountry));

		result->uid = atoi(tokens[0]);
		result->key = strdup(tokens[1]);
		result->countryCode = tokens[1];
		result->countryName = tokens[2];
		result->dateCreated = tokens[3];
		result->dateModified = tokens[4];

		tokens[1] = NULL;
		tokens[2] = NULL;
		tokens[3] = NULL;
		tokens[4] = NULL;
	}

	if(tokens != NULL) {
		for(ii = 0; ii < tokenCount; ii++) {
			if(tokens[ii] != NULL) {
				free(tokens[ii]);
			}
		}

		free(tokens);
	}

	if(rc == -1) {
		return NULL;
	}

	return result;
}

static void country_free(void *memory)
{
	VgrStatsCountry *country = NULL;

	if(memory == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	country = (VgrStatsCountry *)memory;

	if(country->key != NULL) {
		free(country->key);
	}

	if(country->countryCode != NULL) {
		free(country->countryCode);
	}

	if(country->countryName != NULL) {
		free(country->countryName);
	}

	if(country->dateCreated != NULL) {
		free(country->dateCreated);
	}

	if(country->dateModified != NULL) {
		free(country->dateModified);
	}

	free(country);
}

static void country_display(VgrStatsCountry *country)
{
	printf("Country #%06i (%s) ::\n", country->uid, country->key);
	printf("\t country code                 : '%s'\n", country->countryCode);
	printf("\t country name                 : '%s'\n", country->countryName);
	printf("\t date created                 : '%s'\n", country->dateCreated);
	printf("\t date modified                : '%s'\n", country->dateModified);
}

static int country_analyzeCountryFile(char *filename)
{
	int rc = 0;
	int keyLength = 0;
	int readLength = 0;
	int lineCounter = 0;
	double timer = 0.0;
	double elapsedTime = 0.0;
	char key[128];
	char buffer[8192];

	VgrStatsCountry *country = NULL;

	FileHandle fh;

	timer = time_getTimeMus();

	// process the file

	printf("Processing country data file '%s'...", filename);
	fflush(NULL);

	if((rc = file_init(&fh, filename, "r", 8192)) < 0) {
		printf("FAILED at line %i, aborting.\n", __LINE__);
		return -1;
	}

	rc = file_readLine(&fh, buffer, sizeof(buffer), &readLength);
	while(rc == 0) {
		if((country = country_buildCountry(buffer, readLength)) != NULL) {
			if(VERBOSE_MODE) {
				country_display(country);
			}

			if(bptree_put(&context->countries, country->key,
						strlen(country->key), (void *)country) < 0) {
				country_free(country);
			}
			else {
				keyLength = snprintf(key, ((int)sizeof(key) - 1), "%i",
						country->uid);

				bptree_put(&context->countriesById, key, keyLength,
						(void *)country);
			}
		}

		lineCounter++;

		rc = file_readLine(&fh, buffer, sizeof(buffer), &readLength);
	}

	if(rc < 0) {
		printf("FAILED at line %i, aborting.\n", __LINE__);
		return -1;
	}

	if((rc = file_free(&fh)) < 0) {
		printf("FAILED at line %i, aborting.\n", __LINE__);
		return -1;
	}

	printf("OK\n");

	elapsedTime = time_getElapsedMusInSeconds(timer);

	printf("\t ...parsed %i lines in %0.6f seconds. (%0.6f seconds per line)\n",
			lineCounter,
			elapsedTime,
			(elapsedTime / (double)lineCounter));

	return 0;
}

// city functions

static VgrStatsCity *city_buildCity(char *line, int lineLength)
{
	int ii = 0;
	int rc = 0;
	int tokenCount = 0;
	char **tokens = NULL;

	VgrStatsCity *result = NULL;

	static int delimiterLength = 0;

	const int MINIMUM_TOKEN_LENGTH = 0;

	const char DELIMITERS[] = { '\t', '\0' };

	if((line == NULL) || (lineLength < 1)) {
		return NULL;
	}

	if(delimiterLength == 0) {
		delimiterLength = strlen(DELIMITERS);
	}

	tokens = stringTokenize(line, lineLength,
			(char *)DELIMITERS, delimiterLength,
			MINIMUM_TOKEN_LENGTH,
			afalse,
			&tokenCount);

	if((tokens == NULL) || (tokenCount < 9)) {
		rc = -1;
	}

	if(rc == 0) {
		result = (VgrStatsCity *)malloc(sizeof(VgrStatsCity));

		result->uid = atoi(tokens[0]);
		result->siteCountryId = atoi(tokens[1]);
		result->siteStateId = atoi(tokens[2]);
		result->code = tokens[3];
		result->name = tokens[4];
		result->startIp = (aulint)atod(tokens[5]);
		result->endIp = (aulint)atod(tokens[6]);
		result->dateCreated = tokens[7];
		result->dateModified = tokens[8];

		tokens[3] = NULL;
		tokens[4] = NULL;
		tokens[7] = NULL;
		tokens[8] = NULL;
	}

	if(tokens != NULL) {
		for(ii = 0; ii < tokenCount; ii++) {
			if(tokens[ii] != NULL) {
				free(tokens[ii]);
			}
		}

		free(tokens);
	}

	if(rc == -1) {
		return NULL;
	}

	return result;
}

static void city_free(void *memory)
{
	VgrStatsCity *city = NULL;

	if(memory == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	city = (VgrStatsCity *)memory;

	if(city->name != NULL) {
		free(city->name);
	}

	if(city->code != NULL) {
		free(city->code);
	}

	if(city->dateCreated != NULL) {
		free(city->dateCreated);
	}

	if(city->dateModified != NULL) {
		free(city->dateModified);
	}

	free(city);
}

static void city_display(VgrStatsCity *city)
{
	printf("City #%06i ::\n", city->uid);
	printf("\t site country id              : %i\n", city->siteCountryId);
	printf("\t site state id                : %i\n", city->siteStateId);
	printf("\t name                         : '%s'\n", city->name);
	printf("\t code                         : '%s'\n", city->code);
	printf("\t start ip                     : %llu\n", city->startIp);
	printf("\t end ip                       : %llu\n", city->endIp);
	printf("\t date created                 : '%s'\n", city->dateCreated);
	printf("\t date modified                : '%s'\n", city->dateModified);
}

static int city_analyzeCityFile(char *filename)
{
	int ii = 0;
	int rc = 0;
	int keyLength = 0;
	int readLength = 0;
	int displayLength = 0;
	int lineCounter = 0;
	int cityRef = 0;
	int cityLength = 0;
	double timer = 0.0;
	double elapsedTime = 0.0;
	char key[128];
	char buffer[8192];
	char displayBuffer[8192];

	VgrStatsCity *city = NULL;
	VgrStatsCity **cities = NULL;

	FileHandle fh;

	timer = time_getTimeMus();

	// process the file

	printf("Processing city data file '%s'...", filename);
	fflush(NULL);

	if((rc = file_init(&fh, filename, "r", 8192)) < 0) {
		printf("FAILED at line %i, aborting.\n", __LINE__);
		return -1;
	}

	cityRef = 0;
	cityLength = 8;
	cities = (VgrStatsCity **)malloc(sizeof(VgrStatsCity *) *
			cityLength);

	memset((void *)displayBuffer, 0, (int)(sizeof(displayBuffer)));

	rc = file_readLine(&fh, buffer, sizeof(buffer), &readLength);
	while(rc == 0) {
		if((city = city_buildCity(buffer, readLength)) != NULL) {
			if(VERBOSE_MODE) {
				city_display(city);
			}

			keyLength = snprintf(key, ((int)sizeof(key) - 1), "%llu",
					city->startIp);

			if(bptree_put(&context->cities, key, keyLength, (void *)city) < 0) {
				city_free(city);
			}
			else {
				keyLength = snprintf(key, ((int)sizeof(key) - 1), "%i",
						city->uid);

				bptree_put(&context->citiesById, key, keyLength, (void *)city);

				cities[cityRef] = city;

				cityRef++;

				if(cityRef >= cityLength) {
					cityLength *= 2;
					cities = (VgrStatsCity **)realloc(cities,
							(sizeof(VgrStatsCity *) * cityLength));
				}
			}
		}

		if((lineCounter % 8192) == 0) {
			for(ii = 0; ii < displayLength; ii++) {
				printf("\b");
			}

			fflush(stdout);

			displayLength = snprintf(displayBuffer,
					(((int)sizeof(displayBuffer)) - 1),
					"%i lines",
					lineCounter);

			printf("%s", displayBuffer);
			fflush(stdout);
		}

		lineCounter++;

		rc = file_readLine(&fh, buffer, sizeof(buffer), &readLength);
	}

	if(rc < 0) {
		printf("FAILED at line %i, aborting.\n", __LINE__);
		return -1;
	}

	if((rc = file_free(&fh)) < 0) {
		printf("FAILED at line %i, aborting.\n", __LINE__);
		return -1;
	}

	for(ii = 0; ii < displayLength; ii++) {
		printf("\b");
	}

	printf("OK\n");

	elapsedTime = time_getElapsedMusInSeconds(timer);

	printf("\t ...parsed %i lines in %0.6f seconds. (%0.6f seconds per line)\n",
			lineCounter,
			elapsedTime,
			(elapsedTime / (double)lineCounter));

	context->cityListLength = cityRef;
	context->cityList = cities;

	return 0;
}

// spider functions

static VgrStatsSpider *spider_buildSpider(char *line, int lineLength)
{
	int ii = 0;
	int rc = 0;
	int tokenCount = 0;
	char **tokens = NULL;

	VgrStatsSpider *result = NULL;

	static int delimiterLength = 0;

	const int MINIMUM_TOKEN_LENGTH = 0;

	const char DELIMITERS[] = { '\t', '\0' };

	if((line == NULL) || (lineLength < 1)) {
		return NULL;
	}

	if(delimiterLength == 0) {
		delimiterLength = strlen(DELIMITERS);
	}

	tokens = stringTokenize(line, lineLength,
			(char *)DELIMITERS, delimiterLength,
			MINIMUM_TOKEN_LENGTH,
			afalse,
			&tokenCount);

	if((tokens == NULL) || (tokenCount < 8)) {
		rc = -1;
	}

	if(rc == 0) {
		result = (VgrStatsSpider *)malloc(sizeof(VgrStatsSpider));

		result->uid = atoi(tokens[0]);
		result->key = strdup(tokens[3]);
		result->name = tokens[1];
		result->description = tokens[2];
		result->userAgent = tokens[3];
		result->isEnabled = tokens[4];
		result->url = tokens[5];
		result->dateCreated = tokens[6];
		result->dateModified = tokens[7];

		tokens[1] = NULL;
		tokens[2] = NULL;
		tokens[3] = NULL;
		tokens[4] = NULL;
		tokens[5] = NULL;
		tokens[6] = NULL;
		tokens[7] = NULL;
	}

	if(tokens != NULL) {
		for(ii = 0; ii < tokenCount; ii++) {
			if(tokens[ii] != NULL) {
				free(tokens[ii]);
			}
		}

		free(tokens);
	}

	if(rc == -1) {
		return NULL;
	}

	return result;
}

static void spider_free(void *memory)
{
	VgrStatsSpider *spider = NULL;

	if(memory == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	spider = (VgrStatsSpider *)memory;

	if(spider->key != NULL) {
		free(spider->key);
	}

	if(spider->name != NULL) {
		free(spider->name);
	}

	if(spider->description != NULL) {
		free(spider->description);
	}

	if(spider->userAgent != NULL) {
		free(spider->userAgent);
	}

	if(spider->isEnabled != NULL) {
		free(spider->isEnabled);
	}

	if(spider->url != NULL) {
		free(spider->url);
	}

	if(spider->dateCreated != NULL) {
		free(spider->dateCreated);
	}

	if(spider->dateModified != NULL) {
		free(spider->dateModified);
	}

	free(spider);
}

static void spider_display(VgrStatsSpider *spider)
{
	printf("Spider #%06i (%s) ::\n", spider->uid, spider->key);
	printf("\t name                         : '%s'\n", spider->name);
	printf("\t description                  : '%s'\n", spider->description);
	printf("\t user agent                   : '%s'\n", spider->userAgent);
	printf("\t is enabled                   : '%s'\n", spider->isEnabled);
	printf("\t url                          : '%s'\n", spider->url);
	printf("\t date created                 : '%s'\n", spider->dateCreated);
	printf("\t date modified                : '%s'\n", spider->dateModified);
}

static int spider_analyzeSpiderFile(char *filename)
{
	int rc = 0;
	int readLength = 0;
	int lineCounter = 0;
	int spiderRef = 0;
	int spiderLength = 0;
	double timer = 0.0;
	double elapsedTime = 0.0;
	char buffer[8192];

	VgrStatsSpider *spider = NULL;
	VgrStatsSpider **spiders = NULL;

	FileHandle fh;

	timer = time_getTimeMus();

	// process the file

	printf("Processing spider data file '%s'...", filename);
	fflush(NULL);

	if((rc = file_init(&fh, filename, "r", 8192)) < 0) {
		printf("FAILED at line %i, aborting.\n", __LINE__);
		return -1;
	}

	spiderRef = 0;
	spiderLength = 8;
	spiders = (VgrStatsSpider **)malloc(sizeof(VgrStatsSpider *) *
			spiderLength);

	rc = file_readLine(&fh, buffer, sizeof(buffer), &readLength);
	while(rc == 0) {
		if((spider = spider_buildSpider(buffer, readLength)) != NULL) {
			if(VERBOSE_MODE) {
				spider_display(spider);
			}

			spiders[spiderRef] = spider;

			spiderRef++;

			if(spiderRef >= spiderLength) {
				spiderLength *= 2;
				spiders = (VgrStatsSpider **)realloc(spiders,
						(sizeof(VgrStatsSpider *) * spiderLength));
			}
		}

		lineCounter++;

		rc = file_readLine(&fh, buffer, sizeof(buffer), &readLength);
	}

	if(rc < 0) {
		printf("FAILED at line %i, aborting.\n", __LINE__);
		return -1;
	}

	if((rc = file_free(&fh)) < 0) {
		printf("FAILED at line %i, aborting.\n", __LINE__);
		return -1;
	}

	printf("OK\n");

	elapsedTime = time_getElapsedMusInSeconds(timer);

	printf("\t ...parsed %i lines in %0.6f seconds. (%0.6f seconds per line)\n",
			lineCounter,
			elapsedTime,
			(elapsedTime / (double)lineCounter));

	context->spiderListLength = spiderRef;
	context->spiderList = spiders;

	return 0;
}

// visitor functions

static int visitor_getBrowser(char *userAgent)
{
	int result = -1;

	if(strcasestr(userAgent, "opera mini")) {
		result = WEB_BROWSER_OPERA_MINI;
	}
	else if(strcasestr(userAgent, "opera")) {
		result = WEB_BROWSER_OPERA;
	}
	else if(strcasestr(userAgent, "webtv")) {
		result = WEB_BROWSER_WEBTV;
	}
	else if((strcasestr(userAgent, "microsoft internet explorer")) ||
			(strcasestr(userAgent, "msie"))) {
		result = WEB_BROWSER_IE;
	}
	else if(strcasestr(userAgent, "mspie")) {
		result = WEB_BROWSER_POCKET_IE;
	}
	else if(strcasestr(userAgent, "konqueror")) {
		result = WEB_BROWSER_KONQUEROR;
	}
	else if(strcasestr(userAgent, "icab")) {
		result = WEB_BROWSER_ICAB;
	}
	else if(strcasestr(userAgent, "omniweb")) {
		result = WEB_BROWSER_OMNIWEB;
	}
	else if(strcasestr(userAgent, "firebird")) {
		result = WEB_BROWSER_FIREBIRD;
	}
	else if(strcasestr(userAgent, "chrome")) {
		result = WEB_BROWSER_CHROME;
	}
	else if(strcasestr(userAgent, "firefox")) {
		if(strcasestr(userAgent, "navigator")) {
			result = WEB_BROWSER_NETSCAPE_NAVIGATOR;
		}
		else {
			result = WEB_BROWSER_FIREFOX;
		}
	}
	else if(strcasestr(userAgent, "iceweasel")) {
		result = WEB_BROWSER_ICEWEASEL;
	}
	else if((strcasestr(userAgent, "mozilla")) &&
			(strcasestr(userAgent, "shiretoko"))) {
		result = WEB_BROWSER_SHIRETOKO;
	}
	else if((strcasestr(userAgent, "mozilla")) &&
			(strcasestr(userAgent, "netscape"))) {
		result = WEB_BROWSER_MOZILLA;
	}
	else if(strcasestr(userAgent, "amaya")) {
		result = WEB_BROWSER_AMAYA;
	}
	else if(strcasestr(userAgent, "lynx")) {
		result = WEB_BROWSER_LYNX;
	}
	else if(strcasestr(userAgent, "iphone")) {
		result = WEB_BROWSER_IPHONE;
	}
	else if(strcasestr(userAgent, "ipod")) {
		result = WEB_BROWSER_IPOD;
	}
	else if(strcasestr(userAgent, "ipad")) {
		result = WEB_BROWSER_IPAD;
	}
	else if(strcasestr(userAgent, "safari")) {
		result = WEB_BROWSER_SAFARI;
	}
	else if(strcasestr(userAgent, "android")) {
		result = WEB_BROWSER_ANDROID;
	}
	else if(strcasestr(userAgent, "googlebot")) {
		result = WEB_BROWSER_GOOGLEBOT;
	}
	else if(strcasestr(userAgent, "slurp")) {
		result = WEB_BROWSER_SLURP;
	}
	else if((strcasestr(userAgent, "w3c-checklink")) ||
			(strcasestr(userAgent, "w3c_validator"))) {
		result = WEB_BROWSER_W3CVALIDATOR;
	}
	else if(strcasestr(userAgent, "blackberry")) {
		result = WEB_BROWSER_BLACKBERRY;
	}
	else if((strcasestr(userAgent, "mozilla")) &&
			(strcasestr(userAgent, "icecat"))) {
		result = WEB_BROWSER_ICECAT;
	}
	else if(strcasestr(userAgent, "nokia")) {
		if((strcasestr(userAgent, "series60")) ||
				(strcasestr(userAgent, "s60"))) {
			result = WEB_BROWSER_NOKIA_S60;
		}
		else {
			result = WEB_BROWSER_NOKIA;
		}
	}
	else if(strcasestr(userAgent, "msnb")) {
		if(strcasestr(userAgent, "msnbot")) {
			result = WEB_BROWSER_MSNBOT;
		}
		else {
			result = WEB_BROWSER_MSN;
		}
	}
	else if(strcasestr(userAgent, "galeon")) {
		result = WEB_BROWSER_GALEON;
	}
	else if(strcasestr(userAgent, "netpositive")) {
		result = WEB_BROWSER_NETPOSITIVE;
	}
	else if(strcasestr(userAgent, "yandexbot")) {
		result = WEB_BROWSER_YANDEXBOT;
	}
	else if(strcasestr(userAgent, "webster")) {
		result = WEB_BROWSER_WEBSTER_PRO;
	}
	else if(strcasestr(userAgent, "baidu")) {
		result = WEB_BROWSER_BAIDU;
	}
	else if(strcasestr(userAgent, "google")) {
		result = WEB_BROWSER_GOOGLE_APP;
	}
	else if(strcasestr(userAgent, "curl")) {
		result = WEB_BROWSER_CURL;
	}
	else if(strcasestr(userAgent, "java")) {
		result = WEB_BROWSER_JAVA;
	}
	else if(strcasestr(userAgent, "perl")) {
		result = WEB_BROWSER_PERL;
	}
	else if(strcasestr(userAgent, "microsoft")) {
		result = WEB_BROWSER_MICROSOFT;
	}
	else if(strcasestr(userAgent, "mozilla")) {
		result = WEB_BROWSER_MOZILLA_APP;
	}
	else {
//printf("%s() :: '%s'\n", __FUNCTION__, userAgent);
//exit(1);
		result = WEB_BROWSER_UNKNOWN;
	}

	return result;
}

static VgrStatsVisitor *visitor_buildVisitor(char *line, int lineLength)
{
	aboolean found = afalse;
	int ii = 0;
	int rc = 0;
	int counter = 0;
	int keyLength = 0;
	int tokenCount = 0;
	aulint ipNumber = 0;
	char key[128];
	char **tokens = NULL;

	VgrStatsVisitor *result = NULL;
	VgrStatsCity *city = NULL;
	VgrStatsCountry *country = NULL;

	static int delimiterLength = 0;

	const int MINIMUM_TOKEN_LENGTH = 1;

	const char DELIMITERS[] = { '\t', '\0' };

	if((line == NULL) || (lineLength < 1)) {
		return NULL;
	}

	if(delimiterLength == 0) {
		delimiterLength = strlen(DELIMITERS);
	}

	tokens = stringTokenize(line, lineLength,
			(char *)DELIMITERS, delimiterLength,
			MINIMUM_TOKEN_LENGTH,
			afalse,
			&tokenCount);

	if((tokens == NULL) ||
			(tokenCount < 12) ||
			((!PERFORM_GEOIP_LOOKUP) && (tokenCount < 14))) {
		rc = -1;
	}
	else {
		result = (VgrStatsVisitor *)malloc(sizeof(VgrStatsVisitor));

		result->uid = atoi(tokens[0]);
		result->siteTrackingCampaignId = atoi(tokens[1]);
		result->siteUserId = atoi(tokens[2]);
		result->visits = atoi(tokens[3]);
		result->key = strdup(tokens[0]);
		result->hash = strndup(tokens[4], 40);
		result->platform = tokens[5];
		result->userAgent = tokens[6];

		result->browserId = visitor_getBrowser(result->userAgent);

		if(result->browserId < 1) {
			result->browserId = WEB_BROWSER_UNKNOWN;
		}

		result->browserName = strdup(
				WEB_BROWSER_LIST[(result->browserId - 1)].name);

		result->clientIp = tokens[7];
		result->countryCode = tokens[8];
		result->countryName = tokens[9];

		tokens[5] = NULL;
		tokens[6] = NULL;
		tokens[7] = NULL;
		tokens[8] = NULL;
		tokens[9] = NULL;

		if((atrue) && (PERFORM_GEOIP_LOOKUP)) {
			result->dateCreated = tokens[10];
			result->dateModified = tokens[11];

			tokens[10] = NULL;
			tokens[11] = NULL;
		}
		else {
			result->siteStateId = (aulint)atod(tokens[10]);
			result->siteCityId = (aulint)atod(tokens[11]);
			result->dateCreated = tokens[12];
			result->dateModified = tokens[13];

			tokens[12] = NULL;
			tokens[13] = NULL;
		}

		if(PERFORM_GEOIP_LOOKUP) {
			ipNumber = ipToNumber(result->clientIp);

			keyLength = snprintf(key, ((int)sizeof(key) - 1), "%llu", ipNumber);

			if(bptree_get(&context->cities, key, keyLength,
						(void *)&city) < 0) {
				do {
					if((rc = bptree_getNext(&context->cities, key, keyLength,
									(void *)&city)) < 0) {
						break;
					}

					if((ipNumber >= city->startIp) &&
							(ipNumber <= city->endIp)) {
						found = atrue;
						break;
					}

					keyLength = snprintf(key, ((int)sizeof(key) - 1), "%llu",
							city->startIp);

					counter++;
				} while(counter < 8);

				if(!found) {
					counter = 0;

					keyLength = snprintf(key, ((int)sizeof(key) - 1), "%llu",
							ipNumber);

					do {
						if((rc = bptree_getPrevious(&context->cities, key,
										keyLength, (void *)&city)) < 0) {
							break;
						}

						if((ipNumber >= city->startIp) &&
								(ipNumber <= city->endIp)) {
							found = atrue;
							break;
						}

						keyLength = snprintf(key,
								((int)sizeof(key) - 1),
								"%llu",
								city->startIp);

						counter++;
					} while(counter < 8);
				}
			}
			else {
				found = atrue;
			}

			if((found) && (city != NULL)) {
				result->siteStateId = city->siteStateId;
				result->siteCityId = city->uid;

				keyLength = snprintf(key, ((int)sizeof(key) - 1), "%i",
						city->siteCountryId);

				if(bptree_get(&context->countriesById, key, keyLength,
							(void *)&country) == 0) {
					free(result->countryCode);
					free(result->countryName);

					result->countryCode = strdup(country->countryCode);
					result->countryName = strdup(country->countryName);
				}
			}
			else {
				// do nothing, scans take too long

/*				for(ii = 0; ii < context->cityListLength; ii++) {
					if((ipNumber >= context->cityList[ii]->startIp) &&
							(ipNumber <= context->cityList[ii]->endIp)) {
						result->siteStateId = context->cityList[ii]->siteStateId;
						result->siteCityId = context->cityList[ii]->uid;

						keyLength = snprintf(key, ((int)sizeof(key) - 1), "%i",
								context->cityList[ii]->siteCountryId);

						if(bptree_get(&context->countriesById, key, keyLength,
										(void *)&country) == 0) {
								free(result->countryCode);
								free(result->countryName);

							result->countryCode = strdup(country->countryCode);
							result->countryName = strdup(country->countryName);
						}

						break;
					}
				}*/
			}
		}
	}

	if(tokens != NULL) {
		for(ii = 0; ii < tokenCount; ii++) {
			if(tokens[ii] != NULL) {
				free(tokens[ii]);
			}
		}

		free(tokens);
	}

	if(rc == -1) {
		return NULL;
	}

	return result;
}

static void visitor_free(void *memory)
{
	VgrStatsVisitor *visitor = NULL;

	if(memory == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	visitor = (VgrStatsVisitor *)memory;

	if(visitor->key != NULL) {
		free(visitor->key);
	}

	if(visitor->hash != NULL) {
		free(visitor->hash);
	}

	if(visitor->platform != NULL) {
		free(visitor->platform);
	}

	if(visitor->userAgent != NULL) {
		free(visitor->userAgent);
	}

	if(visitor->browserName != NULL) {
		free(visitor->browserName);
	}

	if(visitor->clientIp != NULL) {
		free(visitor->clientIp);
	}

	if(visitor->countryCode != NULL) {
		free(visitor->countryCode);
	}

	if(visitor->countryName != NULL) {
		free(visitor->countryName);
	}

	if(visitor->dateCreated != NULL) {
		free(visitor->dateCreated);
	}

	if(visitor->dateModified != NULL) {
		free(visitor->dateModified);
	}

	free(visitor);
}

static void visitor_display(VgrStatsVisitor *visitor)
{
	printf("Visitor #%06i (%s) ::\n", visitor->uid, visitor->hash);
	printf("\t site tracking campaign id    : %i\n",
			visitor->siteTrackingCampaignId);
	printf("\t site user id                 : %i\n", visitor->siteUserId);
	printf("\t visits                       : %i\n", visitor->visits);
	printf("\t key                          : '%s'\n", visitor->key);
	printf("\t platform                     : '%s'\n", visitor->platform);
	printf("\t user agent                   : '%s'\n", visitor->userAgent);
	printf("\t browser name                 : '%s'\n", visitor->browserName);
	printf("\t client ip                    : '%s'\n", visitor->clientIp);
	printf("\t country code                 : '%s'\n", visitor->countryCode);
	printf("\t country name                 : '%s'\n", visitor->countryName);
	printf("\t date created                 : '%s'\n", visitor->dateCreated);
	printf("\t date modified                : '%s'\n", visitor->dateModified);
	printf("\t siteStateId                  : %i\n", visitor->siteStateId);
	printf("\t siteCityId                   : %i\n", visitor->siteCityId);
}

static int visitor_analyzeVisitorFile(char *filename)
{
	int rc = 0;
	int readLength = 0;
	int lineCounter = 0;
	int visitorRef = 0;
	int visitorLength = 0;
	double timer = 0.0;
	double elapsedTime = 0.0;
	char buffer[8192];

	VgrStatsVisitor *visitor = NULL;
	VgrStatsVisitor **visitors = NULL;

	FileHandle fh;

	timer = time_getTimeMus();

	// process the file

	printf("Processing visitor data file '%s'...", filename);
	fflush(NULL);

	if((rc = file_init(&fh, filename, "r", 8192)) < 0) {
		printf("FAILED at line %i, aborting.\n", __LINE__);
		return -1;
	}

	visitorRef = 0;
	visitorLength = 8;
	visitors = (VgrStatsVisitor **)malloc(sizeof(VgrStatsVisitor *) *
			visitorLength);

	rc = file_readLine(&fh, buffer, sizeof(buffer), &readLength);
	while(rc == 0) {
		if((visitor = visitor_buildVisitor(buffer, readLength)) != NULL) {
			if(VERBOSE_MODE) {
				visitor_display(visitor);
			}

			if(bptree_put(&context->visitors, visitor->key,
						strlen(visitor->key), (void *)visitor) < 0) {
				visitor_free(visitor);
			}
			else {
				context->trackingOverview.uniqueVisitors += 1;
				context->trackingOverview.visits += visitor->visits;

				visitors[visitorRef] = visitor;

				visitorRef++;

				if(visitorRef >= visitorLength) {
					visitorLength *= 2;
					visitors = (VgrStatsVisitor **)realloc(visitors,
							(sizeof(VgrStatsVisitor *) * visitorLength));
				}
			}
		}

//		if(lineCounter > 8) {
//		if(lineCounter > 128) {
//		if(lineCounter > 1024) {
//		if(lineCounter > 16384) {
//			break;
//		}

		lineCounter++;

		rc = file_readLine(&fh, buffer, sizeof(buffer), &readLength);
	}

	if(rc < 0) {
		printf("FAILED at line %i, aborting.\n", __LINE__);
		return -1;
	}

	if((rc = file_free(&fh)) < 0) {
		printf("FAILED at line %i, aborting.\n", __LINE__);
		return -1;
	}

	printf("OK\n");

	elapsedTime = time_getElapsedMusInSeconds(timer);

	printf("\t ...parsed %i lines in %0.6f seconds. (%0.6f seconds per line)\n",
			lineCounter,
			elapsedTime,
			(elapsedTime / (double)lineCounter));

	context->visitorListLength = visitorRef;
	context->visitorList = visitors;

	return 0;
}

// detail functions

static VgrStatsDetail *detail_buildDetail(char *line, int lineLength)
{
	int ii = 0;
	int rc = 0;
	int tokenCount = 0;
	char **tokens = NULL;

	VgrStatsDetail *result = NULL;

	static int delimiterLength = 0;

	const int MINIMUM_TOKEN_LENGTH = 0;

	const char DELIMITERS[] = { '\t', '\0' };

	if((line == NULL) || (lineLength < 1)) {
		return NULL;
	}

	if(delimiterLength == 0) {
		delimiterLength = strlen(DELIMITERS);
	}

	tokens = stringTokenize(line, lineLength,
			(char *)DELIMITERS, delimiterLength,
			MINIMUM_TOKEN_LENGTH,
			afalse,
			&tokenCount);

	if((tokens == NULL) || (tokenCount < 8)) {
		rc = -1;
	}
	else {
		result = (VgrStatsDetail *)malloc(sizeof(VgrStatsDetail));

		result->uid = atoi(tokens[0]);
		result->siteTrackingVisitorId = atoi(tokens[1]);
		result->key = strdup(tokens[1]);
		result->host = tokens[2];
		result->hostname = tokens[3];
		result->url = tokens[4];
		result->normalizedUrl = getNormalizedUrl(result->url);
		result->referer = tokens[5];

		if((result->referer != NULL) && (strlen(result->referer) > 0)) {
			result->normalizedReferer = getNormalizedReferer(result->referer);
		}
		else {
			if(result->referer != NULL) {
				free(result->referer);
			}
			result->referer = NULL;
			result->normalizedReferer = NULL;
		}

		result->dateCreated = tokens[6];
		result->dateModified = tokens[7];

		tokens[2] = NULL;
		tokens[3] = NULL;
		tokens[4] = NULL;
		tokens[5] = NULL;
		tokens[6] = NULL;
		tokens[7] = NULL;
	}

	if(tokens != NULL) {
		for(ii = 0; ii < tokenCount; ii++) {
			if(tokens[ii] != NULL) {
				free(tokens[ii]);
			}
		}

		free(tokens);
	}

	if(((result->key == NULL) ||
				(strlen(result->key) < 1)) ||
			((result->host == NULL) ||
			 (strlen(result->host) < 1)) ||
			((result->hostname == NULL) ||
			 (strlen(result->hostname) < 1)) ||
			((result->url == NULL) ||
			 (strlen(result->url) < 1)) ||
			((result->normalizedUrl == NULL) ||
			 (strlen(result->normalizedUrl) < 1)) ||
			((result->dateCreated == NULL) ||
			 (strlen(result->dateCreated) < 1)) ||
			((result->dateModified == NULL) ||
			 (strlen(result->dateModified) < 1))) {
//			((result->referer == NULL) ||
//			 (strlen(result->referer) < 1)) ||
//			((result->normalizedReferer == NULL) ||
//			 (strlen(result->normalizedReferer) < 1)) ||
		detail_free(result);
		return NULL;
	}

	if(rc == -1) {
		return NULL;
	}

	return result;
}

static void detail_free(void *memory)
{
	VgrStatsDetail *detail = NULL;

	if(memory == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	detail = (VgrStatsDetail *)memory;

	if(detail->key != NULL) {
		free(detail->key);
	}

	if(detail->host != NULL) {
		free(detail->host);
	}

	if(detail->hostname != NULL) {
		free(detail->hostname);
	}

	if(detail->url != NULL) {
		free(detail->url);
	}

	if(detail->normalizedUrl != NULL) {
		free(detail->normalizedUrl);
	}

	if(detail->referer != NULL) {
		free(detail->referer);
	}

	if(detail->normalizedReferer != NULL) {
		free(detail->normalizedReferer);
	}

	if(detail->dateCreated != NULL) {
		free(detail->dateCreated);
	}

	if(detail->dateModified != NULL) {
		free(detail->dateModified);
	}

	free(detail);
}

static void detail_display(VgrStatsDetail *detail)
{
	printf("Detail #%06i (%s) ::\n", detail->uid, detail->key);
	printf("\t site tracking visitor id     : %i\n",
			detail->siteTrackingVisitorId);
	printf("\t uid                          : %i\n", detail->uid);
	printf("\t host                         : '%s'\n", detail->host);
	printf("\t hostname                     : '%s'\n", detail->hostname);
	printf("\t url                          : '%s'\n", detail->url);
	printf("\t normalized url               : '%s'\n", detail->normalizedUrl);
	printf("\t referer                      : '%s'\n", detail->referer);
	printf("\t normalized referer           : '%s'\n",
			detail->normalizedReferer);
	printf("\t date created                 : '%s'\n", detail->dateCreated);
	printf("\t date modified                : '%s'\n", detail->dateModified);
}

static void daily_visitorFree(void *memory)
{
	VgrStatsVisitor *visitor = NULL;

	if(memory == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	visitor = (VgrStatsVisitor *)memory;

	visitor->daily.isConverted = afalse;
	visitor->daily.pageCount = 0;
	visitor->daily.trackingTimestamp = 0;
	visitor->daily.timeOnSite = 0;
	visitor->daily.dailySecondStamp = 0;
	visitor->daily.dailyHourStamp = 0;
	visitor->daily.exitUrlCounter = 0;
	visitor->daily.destinationId = -1;
	visitor->daily.propertyId = -1;
	visitor->daily.ratePlanId = 0;

	if(visitor->daily.referer != NULL) {
		free(visitor->daily.referer);
	}

	if(visitor->daily.normalizedReferer != NULL) {
		free(visitor->daily.normalizedReferer);
	}

	if(visitor->daily.exitUrl != NULL) {
		free(visitor->daily.exitUrl);
	}

	if(visitor->daily.searchEngine != NULL) {
		free(visitor->daily.searchEngine);
	}

	if(visitor->daily.searchPhrase != NULL) {
		free(visitor->daily.searchPhrase);
	}

	visitor->daily.referer = NULL;
	visitor->daily.normalizedReferer = NULL;
	visitor->daily.exitUrl = NULL;
	visitor->daily.searchEngine = NULL;
	visitor->daily.searchPhrase = NULL;
}

static void daily_summaryFree(void *memory)
{
	int writeLength = 0;
	char *sqlString = NULL;
	char buffer[8192];

	VgrStatsDailySummary *summary = NULL;

	if(memory == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	summary = (VgrStatsDailySummary *)memory;

	if(summary->linkId > 0) {
		switch(summary->type) {
			case VGR_DAILY_SUMMARY_TYPE_CAMPAIGN:
				writeLength = snprintf(buffer, (((int)sizeof(buffer)) - 1),
						"insert into siteTrackingDailySummary ("
						"siteTrackingCampaignId, "
						"uniqueVisitors, "
						"visits, "
						"pageCount, "
						"conversionCount, "
						"bounceCount, "
						"totalTimeOnSite, "
						"summaryDate) "
						"values ("
						"'%i', '%i', '%i', '%i', '%i', '%i', '%i', '%s'"
						");\n",
						summary->linkId,
						summary->uniqueVisitors,
						summary->visits,
						summary->pageCount,
						summary->conversions,
						summary->bounceCount,
						summary->totalTimeOnSite,
						summary->dateStamp);

//				printf("\n\n%s\n\n", buffer);

				file_write(context->stcampaign, buffer, writeLength);
				break;

			case VGR_DAILY_SUMMARY_TYPE_LOCALE:
				writeLength = snprintf(buffer, (((int)sizeof(buffer)) - 1),
						"insert into siteTrackingLocaleSummary ("
						"siteCountryId, "
						"uniqueVisitors, "
						"visits, "
						"pageCount, "
						"conversionCount, "
						"bounceCount, "
						"totalTimeOnSite, "
						"summaryDate) "
						"values ("
						"'%i', '%i', '%i', '%i', '%i', '%i', '%i', '%s'"
						");\n",
						summary->linkId,
						summary->uniqueVisitors,
						summary->visits,
						summary->pageCount,
						summary->conversions,
						summary->bounceCount,
						summary->totalTimeOnSite,
						summary->dateStamp);

//				printf("\n\n%s\n\n", buffer);

				file_write(&context->stlocale, buffer, writeLength);
				break;

			case VGR_DAILY_SUMMARY_TYPE_SPIDER:
				writeLength = snprintf(buffer, (((int)sizeof(buffer)) - 1),
						"insert into siteTrackingSpiderSummary ("
						"siteTrackingSpiderId, "
						"uniqueVisitors, "
						"visits, "
						"pageCount, "
						"conversionCount, "
						"bounceCount, "
						"totalTimeOnSite, "
						"summaryDate) "
						"values ("
						"'%i', '%i', '%i', '%i', '%i', '%i', '%i', '%s'"
						");\n",
						summary->linkId,
						summary->uniqueVisitors,
						summary->visits,
						summary->pageCount,
						summary->conversions,
						summary->bounceCount,
						summary->totalTimeOnSite,
						summary->dateStamp);

//				printf("\n\n%s\n\n", buffer);

				file_write(&context->stspider, buffer, writeLength);
				break;

			case VGR_DAILY_SUMMARY_TYPE_URL:
				if((summary->uniqueVisitors > 0) &&
						(summary->visits > 0) &&
						(summary->pageCount > 0)) {
					writeLength = snprintf(buffer, (((int)sizeof(buffer)) - 1),
							"insert into siteTrackingUrlSummary ("
							"url, "
							"uniqueVisitors, "
							"visits, "
							"pageCount, "
							"summaryDate) "
							"values ("
							"'%s', '%i', '%i', '%i', '%s'"
							");\n",
							summary->url,
							summary->uniqueVisitors,
							summary->visits,
							summary->pageCount,
							summary->dateStamp);

//					printf("\n\n%s\n\n", buffer);

					file_write(&context->sturl, buffer, writeLength);
				}
				break;

			case VGR_DAILY_SUMMARY_TYPE_REFERER:
				if((summary->uniqueVisitors > 0) &&
						(summary->visits > 0) &&
						(summary->pageCount > 0)) {
					sqlString = sqlStrdup(summary->referer);

					writeLength = snprintf(buffer, (((int)sizeof(buffer)) - 1),
							"insert into siteTrackingRefererSummary ("
							"referer, "
							"uniqueVisitors, "
							"visits, "
							"pageCount, "
							"conversionCount, "
							"bounceCount, "
							"totalTimeOnSite, "
							"summaryDate) "
							"values ("
							"'%s', '%i', '%i', '%i', '%i', '%i', '%i', '%s'"
							");\n",
							sqlString,
							summary->uniqueVisitors,
							summary->visits,
							summary->pageCount,
							summary->conversions,
							summary->bounceCount,
							summary->totalTimeOnSite,
							summary->dateStamp);

//					printf("\n\n%s\n\n", buffer);

					file_write(&context->streferer, buffer, writeLength);

					free(sqlString);
				}
				break;

			case VGR_DAILY_SUMMARY_TYPE_REFERER_NORMALIZED:
				if((summary->uniqueVisitors > 0) &&
						(summary->visits > 0) &&
						(summary->pageCount > 0)) {
					writeLength = snprintf(buffer, (((int)sizeof(buffer)) - 1),
							"insert into siteTrackingDomainSummary ("
							"domain, "
							"uniqueVisitors, "
							"visits, "
							"pageCount, "
							"conversionCount, "
							"bounceCount, "
							"totalTimeOnSite, "
							"summaryDate) "
							"values ("
							"'%s', '%i', '%i', '%i', '%i', '%i', '%i', '%s'"
							");\n",
							summary->referer,
							summary->uniqueVisitors,
							summary->visits,
							summary->pageCount,
							summary->conversions,
							summary->bounceCount,
							summary->totalTimeOnSite,
							summary->dateStamp);

//					printf("\n\n%s\n\n", buffer);

					file_write(&context->stdomain, buffer, writeLength);
				}
				break;

			case VGR_DAILY_SUMMARY_TYPE_EXIT_URL:
				if((summary->uniqueVisitors > 0) &&
						(summary->pageCount > 0)) {
					writeLength = snprintf(buffer, (((int)sizeof(buffer)) - 1),
							"insert into siteTrackingExitUrlSummary ("
							"url, "
							"uniqueVisitors, "
							"pageCount, "
							"summaryDate) "
							"values ("
							"'%s', '%i', '%i', '%s'"
							");\n",
							summary->url,
							summary->uniqueVisitors,
							summary->pageCount,
							summary->dateStamp);

//					printf("\n\n%s\n\n", buffer);

					file_write(&context->stexiturl, buffer, writeLength);
				}
				break;

			case VGR_DAILY_SUMMARY_TYPE_OS:
				if((summary->uniqueVisitors > 0) &&
						(summary->visits > 0) &&
						(summary->pageCount > 0)) {
					writeLength = snprintf(buffer, (((int)sizeof(buffer)) - 1),
							"insert into siteTrackingOperatingSystemSummary ("
							"siteTrackingOperatingSystemId, "
							"uniqueVisitors, "
							"visits, "
							"pageCount, "
							"conversionCount, "
							"bounceCount, "
							"totalTimeOnSite, "
							"summaryDate) "
							"values ("
							"'%i', '%i', '%i', '%i', '%i', '%i', '%i', '%s'"
							");\n",
							summary->linkId,
							summary->uniqueVisitors,
							summary->visits,
							summary->pageCount,
							summary->conversions,
							summary->bounceCount,
							summary->totalTimeOnSite,
							summary->dateStamp);

//					printf("\n\n%s\n\n", buffer);

					file_write(&context->stos, buffer, writeLength);
				}
				break;

			case VGR_DAILY_SUMMARY_TYPE_SEARCH_ENGINE:
				if((summary->uniqueVisitors > 0) &&
						(summary->visits > 0) &&
						(summary->pageCount > 0)) {
					writeLength = snprintf(buffer, (((int)sizeof(buffer)) - 1),
							"insert into siteTrackingSearchEngineSummary ("
							"siteTrackingSearchEngineId, "
							"uniqueVisitors, "
							"visits, "
							"pageCount, "
							"conversionCount, "
							"bounceCount, "
							"totalTimeOnSite, "
							"summaryDate) "
							"values ("
							"'%i', '%i', '%i', '%i', '%i', '%i', '%i', '%s'"
							");\n",
							summary->linkId,
							summary->uniqueVisitors,
							summary->visits,
							summary->pageCount,
							summary->conversions,
							summary->bounceCount,
							summary->totalTimeOnSite,
							summary->dateStamp);

//					printf("\n\n%s\n\n", buffer);

					file_write(&context->stse, buffer, writeLength);
				}
				break;

			case VGR_DAILY_SUMMARY_TYPE_SEARCH_ENGINE_PHRASE:
				if((summary->uniqueVisitors > 0) &&
						(summary->visits > 0) &&
						(summary->pageCount > 0)) {
					writeLength = snprintf(buffer, (((int)sizeof(buffer)) - 1),
							"insert into "
							"siteTrackingSearchEnginePhraseSummary "
							"("
							"siteTrackingSearchEngineId, "
							"phrase, "
							"uniqueVisitors, "
							"visits, "
							"pageCount, "
							"conversionCount, "
							"bounceCount, "
							"totalTimeOnSite, "
							"summaryDate) "
							"values ("
							"'%i', '%s', '%i', '%i', '%i', '%i', '%i', '%i', '%s'"
							");\n",
							summary->linkId,
							summary->url,
							summary->uniqueVisitors,
							summary->visits,
							summary->pageCount,
							summary->conversions,
							summary->bounceCount,
							summary->totalTimeOnSite,
							summary->dateStamp);

//					printf("\n\n%s\n\n", buffer);

					file_write(&context->stsp, buffer, writeLength);
				}
				break;

			case VGR_DAILY_SUMMARY_TYPE_SEARCH_ENGINE_KEYWORD:
				if((summary->uniqueVisitors > 0) &&
						(summary->visits > 0) &&
						(summary->pageCount > 0)) {
					writeLength = snprintf(buffer, (((int)sizeof(buffer)) - 1),
							"insert into "
							"siteTrackingSearchEngineKeywordSummary "
							"("
							"siteTrackingSearchEngineId, "
							"keyword, "
							"uniqueVisitors, "
							"visits, "
							"pageCount, "
							"conversionCount, "
							"bounceCount, "
							"totalTimeOnSite, "
							"summaryDate) "
							"values ("
							"'%i', '%s', '%i', '%i', '%i', '%i', '%i', '%i', '%s'"
							");\n",
							summary->linkId,
							summary->url,
							summary->uniqueVisitors,
							summary->visits,
							summary->pageCount,
							summary->conversions,
							summary->bounceCount,
							summary->totalTimeOnSite,
							summary->dateStamp);

//					printf("\n\n%s\n\n", buffer);

					file_write(&context->stsk, buffer, writeLength);
				}
				break;

			case VGR_DAILY_SUMMARY_TYPE_WEB_BROWSER:
				if((summary->uniqueVisitors > 0) &&
						(summary->visits > 0) &&
						(summary->pageCount > 0)) {
					writeLength = snprintf(buffer, (((int)sizeof(buffer)) - 1),
							"insert into siteTrackingWebBrowserSummary ("
							"siteTrackingWebBrowserId, "
							"uniqueVisitors, "
							"visits, "
							"pageCount, "
							"conversionCount, "
							"bounceCount, "
							"totalTimeOnSite, "
							"summaryDate) "
							"values ("
							"'%i', '%i', '%i', '%i', '%i', '%i', '%i', '%s'"
							");\n",
							summary->linkId,
							summary->uniqueVisitors,
							summary->visits,
							summary->pageCount,
							summary->conversions,
							summary->bounceCount,
							summary->totalTimeOnSite,
							summary->dateStamp);

//					printf("\n\n%s\n\n", buffer);

					file_write(&context->stwb, buffer, writeLength);
				}
				break;

			case VGR_DAILY_SUMMARY_TYPE_CITY:
				if((summary->uniqueVisitors > 0) &&
						(summary->visits > 0) &&
						(summary->pageCount > 0)) {
					sqlString = sqlStrdup(summary->referer);

					writeLength = snprintf(buffer, (((int)sizeof(buffer)) - 1),
							"insert into siteTrackingCitySummary ("
							"siteCityId, "
							"siteStateId, "
							"cityName, "
							"uniqueVisitors, "
							"visits, "
							"pageCount, "
							"conversionCount, "
							"bounceCount, "
							"totalTimeOnSite, "
							"summaryDate) "
							"values ("
							"'%i', '%i', '%s', '%i', '%i', '%i', '%i', '%i', '%i', '%s'"
							");\n",
							summary->linkId,
							summary->linkIdTwo,
							sqlString,
							summary->uniqueVisitors,
							summary->visits,
							summary->pageCount,
							summary->conversions,
							summary->bounceCount,
							summary->totalTimeOnSite,
							summary->dateStamp);

//					printf("\n\n%s\n\n", buffer);

					file_write(&context->stcity, buffer, writeLength);

					free(sqlString);
				}
				break;

			case VGR_DAILY_SUMMARY_TYPE_STATE:
				if((summary->uniqueVisitors > 0) &&
						(summary->visits > 0) &&
						(summary->pageCount > 0)) {
					writeLength = snprintf(buffer, (((int)sizeof(buffer)) - 1),
							"insert into siteTrackingStateSummary ("
							"siteStateId, "
							"uniqueVisitors, "
							"visits, "
							"pageCount, "
							"conversionCount, "
							"bounceCount, "
							"totalTimeOnSite, "
							"summaryDate) "
							"values ("
							"'%i', '%i', '%i', '%i', '%i', '%i', '%i', '%s'"
							");\n",
							summary->linkId,
							summary->uniqueVisitors,
							summary->visits,
							summary->pageCount,
							summary->conversions,
							summary->bounceCount,
							summary->totalTimeOnSite,
							summary->dateStamp);

//					printf("\n\n%s\n\n", buffer);

					file_write(&context->ststate, buffer, writeLength);
				}
				break;

			case VGR_DAILY_SUMMARY_TYPE_RATE_PLAN:
				if((summary->uniqueVisitors > 0) &&
						(summary->visits > 0) &&
						(summary->pageCount > 0)) {
					writeLength = snprintf(buffer, (((int)sizeof(buffer)) - 1),
							"insert into siteTrackingRatePlanSummary ("
							"bePropertyRatePlanId, "
							"uniqueVisitors, "
							"visits, "
							"pageCount, "
							"conversionCount, "
							"bounceCount, "
							"totalTimeOnSite, "
							"summaryDate) "
							"values ("
							"'%i', '%i', '%i', '%i', '%i', '%i', '%i', '%s'"
							");\n",
							summary->linkId,
							summary->uniqueVisitors,
							summary->visits,
							summary->pageCount,
							summary->conversions,
							summary->bounceCount,
							summary->totalTimeOnSite,
							summary->dateStamp);

//					printf("\n\n%s\n\n", buffer);

					file_write(&context->strp, buffer, writeLength);
				}
				break;

			case VGR_DAILY_SUMMARY_TYPE_ERROR:
			default:
				break;

		}
	}

	if(summary->url != NULL) {
		free(summary->url);
	}

	if(summary->referer != NULL) {
		free(summary->referer);
	}

	if(summary->dateStamp != NULL) {
		free(summary->dateStamp);
	}

	free(summary);
}

static int daily_getDestinationUid(VgrDestinations destination)
{
	int result = 0;

	switch(destination) {
		case VGR_DESTINATION_CABO:
			result = 1;
			break;

		case VGR_DESTINATION_CANCUN:
			result = 2;
			break;

		case VGR_DESTINATION_LORETO:
			result = 3;
			break;

		case VGR_DESTINATION_PUERTO_VALLARTA:
			result = 5;
			break;

		case VGR_DESTINATION_NUEVO_VALLARTA:
			result = 4;
			break;

		case VGR_DESTINATION_ERROR:
		default:
			result = 0;
	}

	return result;
}

static int daily_getPropertyUid(VgrProperties property)
{
	int result = 0;

	switch(property) {
		case VGR_PROPERTY_VLE_CABO:
			result = 1;
			break;

		case VGR_PROPERTY_VDP_CABO:
			result = 5;
			break;

		case VGR_PROPERTY_VILLA_DEL_ARCO:
			result = 3;
			break;

		case VGR_PROPERTY_VDP_LORETO:
			result = 7;
			break;

		case VGR_PROPERTY_VDP_PUERTO_VALLARTA:
			result = 4;
			break;

		case VGR_PROPERTY_VLE_NAYARIT:
			result = 6;
			break;

		case VGR_PROPERTY_FLAMINGOS:
			result = 2;
			break;

		case VGR_PROPERTY_VDP_CANCUN:
			result = 8;
			break;

		case VGR_PROPERTY_ERROR:
		default:
			result = 0;
	}

	return result;
}

static void detail_getDailyVisitorInfo(int arrayLength, void **values,
		int *bounceCount, int *totalTimeOnSite)
{
	int ii = 0;

	VgrStatsVisitor **visitors = NULL;

	*bounceCount = 0;
	*totalTimeOnSite = 0;

	visitors = (VgrStatsVisitor **)values;

	for(ii = 0; ii < arrayLength; ii++) {
		if(visitors[ii]->daily.pageCount <= 1) {
			*bounceCount += 1;
		}
		else {
			*totalTimeOnSite += visitors[ii]->daily.timeOnSite;
		}
	}
}

static int detail_getSpiderUidFromVisitor(VgrStatsVisitor *visitor)
{
	int ii = 0;
	int result = 0;

	if(visitor == NULL) {
		return -1;
	}

	if(visitor->hasSpiderCheck) {
		return visitor->siteTrackingSpiderId;
	}

	for(ii = 0; ii < context->spiderListLength; ii++) {
		if(strcasestr(visitor->userAgent, context->spiderList[ii]->key)) {
			result = context->spiderList[ii]->uid;
			break;
		}
	}

	visitor->hasSpiderCheck = atrue;
	visitor->siteTrackingSpiderId = result;

	return result;
}

static int detail_descendingVgrStatsDailySummaryComparison(const void *left,
		const void *right)
{
	VgrStatsDailySummary *leftSummary = NULL;
	VgrStatsDailySummary *rightSummary = NULL;

	if(left == NULL) {
		if(right == NULL) {
			return 0;
		}
		return -1;
	}
	else if(right == NULL) {
		return 1;
	}

	leftSummary = (VgrStatsDailySummary *)(*(unsigned long int *)left);
	rightSummary = (VgrStatsDailySummary *)(*(unsigned long int *)right);

	return (rightSummary->pageCount - leftSummary->pageCount);
}

static int detail_buildDestinationAndPropertyUrlSummaries(int arrayLength,
		void **values)
{
	int ii = 0;
	int nn = 0;
	int counter = 0;
	int writeLength = 0;
	char buffer[8192];
	char *string = NULL;
	char *sqlString = NULL;

	VgrStatsDailySummary **urls = NULL;

	urls = (VgrStatsDailySummary **)values;

	for(ii = 0; ii < 5; ii++) {
		switch(ii) {
			case 0:
				string = "/destinations/cabo-san-lucas";
				break;

			case 1:
				string = "/destinations/cancun";
				break;

			case 2:
				string = "/destinations/loreto";
				break;

			case 3:
				string = "/destinations/puerto-vallarta";
				break;

			case 4:
				string = "/destinations/riviera-nayarit";
				break;

			default:
				string = NULL;
		}

		if(string == NULL) {
			break;
		}

		counter = 0;

		for(nn = 0; nn < arrayLength; nn++) {
			if((urls[nn]->uniqueVisitors < 1) ||
					(urls[nn]->visits < 1) ||
					(urls[nn]->pageCount < 1) ||
					(urls[nn]->url == NULL)) {
				continue;
			}

			if(strcasestr(urls[nn]->url, string)) {
				if(urls[nn]->type == VGR_DAILY_SUMMARY_TYPE_URL) {
					writeLength = snprintf(buffer, (((int)sizeof(buffer)) - 1),
							"insert into siteTrackingUrlSummary ("
							"beDestinationId, "
							"url, "
							"uniqueVisitors, "
							"visits, "
							"pageCount, "
							"summaryDate) "
							"values ("
							"'%i', '%s', '%i', '%i', '%i', '%s'"
							");\n",
							daily_getDestinationUid(ii),
							urls[nn]->url,
							urls[nn]->uniqueVisitors,
							urls[nn]->visits,
							urls[nn]->pageCount,
							urls[nn]->dateStamp);

					file_write(&context->sturl, buffer, writeLength);
				}
				else if(urls[nn]->type == VGR_DAILY_SUMMARY_TYPE_REFERER) {
					sqlString = sqlStrdup(urls[nn]->referer);

					writeLength = snprintf(buffer, (((int)sizeof(buffer)) - 1),
							"insert into siteTrackingRefererSummary ("
							"beDestinationId, "
							"referer, "
							"uniqueVisitors, "
							"visits, "
							"pageCount, "
							"summaryDate) "
							"values ("
							"'%i', '%s', '%i', '%i', '%i', '%s'"
							");\n",
							daily_getDestinationUid(ii),
							sqlString,
							urls[nn]->uniqueVisitors,
							urls[nn]->visits,
							urls[nn]->pageCount,
							urls[nn]->dateStamp);

					file_write(&context->streferer, buffer, writeLength);

					free(sqlString);
				}
				else if(urls[nn]->type ==
						VGR_DAILY_SUMMARY_TYPE_REFERER_NORMALIZED) {
					writeLength = snprintf(buffer, (((int)sizeof(buffer)) - 1),
							"insert into siteTrackingDomainSummary ("
							"beDestinationId, "
							"domain, "
							"uniqueVisitors, "
							"visits, "
							"pageCount, "
							"summaryDate) "
							"values ("
							"'%i', '%s', '%i', '%i', '%i', '%s'"
							");\n",
							daily_getDestinationUid(ii),
							urls[nn]->referer,
							urls[nn]->uniqueVisitors,
							urls[nn]->visits,
							urls[nn]->pageCount,
							urls[nn]->dateStamp);

					file_write(&context->stdomain, buffer, writeLength);
				}

//				printf("\n\n%s\n\n", buffer);

				counter++;
			}

			if(counter >= MAX_DAILY_RECORDS) {
				break;
			}
		}
	}

	for(ii = 0; ii < 8; ii++) {
		switch(ii) {
			case 0:
				string = "/resorts/villa-la-estancia-cabo";
				break;

			case 1:
				string = "/resorts/villa-del-palmar-cabo";
				break;

			case 2:
				string = "/resorts/villa-del-arco";
				break;

			case 3:
				string = "/resorts/villa-del-palmar-loreto";
				break;

			case 4:
				string = "/resorts/villa-del-palmar-puerto-vallarta";
				break;

			case 5:
				string = "/resorts/villa-la-estancia-riviera-nayarit";
				break;

			case 6:
				string = "/resorts/villa-del-palmar-flamingos";
				break;

			case 7:
				string = "/resorts/villa-del-palmar-cancun";
				break;

			default:
				string = NULL;
		}

		if(string == NULL) {
			break;
		}

		counter = 0;

		for(nn = 0; nn < arrayLength; nn++) {
			if((urls[nn]->uniqueVisitors < 1) ||
					(urls[nn]->visits < 1) ||
					(urls[nn]->pageCount < 1) ||
					(urls[nn]->url == NULL)) {
				continue;
			}

			if(strcasestr(urls[nn]->url, string)) {
				if(urls[nn]->type == VGR_DAILY_SUMMARY_TYPE_URL) {
					writeLength = snprintf(buffer, (((int)sizeof(buffer)) - 1),
							"insert into siteTrackingUrlSummary ("
							"bePropertyId, "
							"url, "
							"uniqueVisitors, "
							"visits, "
							"pageCount, "
							"summaryDate) "
							"values ("
							"'%i', '%s', '%i', '%i', '%i', '%s'"
							");\n",
							daily_getPropertyUid(ii),
							urls[nn]->url,
							urls[nn]->uniqueVisitors,
							urls[nn]->visits,
							urls[nn]->pageCount,
							urls[nn]->dateStamp);

					file_write(&context->sturl, buffer, writeLength);
				}
				else if(urls[nn]->type == VGR_DAILY_SUMMARY_TYPE_REFERER) {
					sqlString = sqlStrdup(urls[nn]->referer);

					writeLength = snprintf(buffer, (((int)sizeof(buffer)) - 1),
							"insert into siteTrackingRefererSummary ("
							"bePropertyId, "
							"referer, "
							"uniqueVisitors, "
							"visits, "
							"pageCount, "
							"summaryDate) "
							"values ("
							"'%i', '%s', '%i', '%i', '%i', '%s'"
							");\n",
							daily_getPropertyUid(ii),
							sqlString,
							urls[nn]->uniqueVisitors,
							urls[nn]->visits,
							urls[nn]->pageCount,
							urls[nn]->dateStamp);

					file_write(&context->streferer, buffer, writeLength);

					free(sqlString);
				}
				else if(urls[nn]->type ==
						VGR_DAILY_SUMMARY_TYPE_REFERER_NORMALIZED) {
					writeLength = snprintf(buffer, (((int)sizeof(buffer)) - 1),
							"insert into siteTrackingDomainSummary ("
							"bePropertyId, "
							"domain, "
							"uniqueVisitors, "
							"visits, "
							"pageCount, "
							"summaryDate) "
							"values ("
							"'%i', '%s', '%i', '%i', '%i', '%s'"
							");\n",
							daily_getPropertyUid(ii),
							urls[nn]->referer,
							urls[nn]->uniqueVisitors,
							urls[nn]->visits,
							urls[nn]->pageCount,
							urls[nn]->dateStamp);

					file_write(&context->stdomain, buffer, writeLength);
				}

//				printf("\n\n%s\n\n", buffer);

				counter++;
			}

			if(counter >= MAX_DAILY_RECORDS) {
				break;
			}
		}
	}

	return 0;
}

static void detail_freeOperatingSystem(void *memory)
{
	VgrStatsOS *operatingSystem = NULL;

	if(memory == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	operatingSystem = (VgrStatsOS *)memory;

	if(operatingSystem->name != NULL) {
		free(operatingSystem->name);
	}

	if(operatingSystem->description != NULL) {
		free(operatingSystem->description);
	}

	free(operatingSystem);
}

static int detail_ascendingVgrStatsOsComparison(const void *left,
		const void *right)
{
	VgrStatsOS *leftOs = NULL;
	VgrStatsOS *rightOs = NULL;

	if(left == NULL) {
		if(right == NULL) {
			return 0;
		}
		return 1;
	}
	else if(right == NULL) {
		return -1;
	}

	leftOs = (VgrStatsOS *)(*(unsigned long int *)left);
	rightOs = (VgrStatsOS *)(*(unsigned long int *)right);

	return (leftOs->uid - rightOs->uid);
}

static int detail_analyzeDetailFile(char *filename)
{
	aboolean firstTime = atrue;
	int ii = 0;
	int nn = 0;
	int rc = 0;
	int keyLength = 0;
	int readLength = 0;
	int writeLength = 0;
	int displayLength = 0;
	int dateCreatedLength = 0;
	int dayCounter = 0;
	int lastDayCounter = 0;
	int lineCounter = 0;
	double timer = 0.0;
	double elapsedTime = 0.0;
	char buffer[8192];
	char displayBuffer[8192];
	char *sqlString = NULL;

	VgrStatsDetail *detail = NULL;

	FileHandle fh;

	int year = 0;
	int currentYear = 0;
	int month = 0;
	int currentMonth = 0;
	int day = 0;
	int currentDay = 0;
	int dailySecondStamp = 0;

	VgrStatsVisitor *visitor = NULL;

	int indexArrayLength = 0;
	int *indexKeyLengths = NULL;
	char **indexKeys = NULL;
	void **indexValues = NULL;
	int bounceCount = 0;
	int totalTimeOnSite = 0;
	Bptree dailyVisitors;
	VgrStatsDailySummary *dailySummary = NULL;
	FileHandle stds;

	int campaignKeyLength = 0;
	char campaignKey[32];
	Bptree dailyCampaigns;
	VgrStatsDailySummary *dailyCampaign = NULL;

	int destinationId = 0;
	int propertyId = 0;
	VgrStatsDailySummary destinations[5];
	VgrStatsDailySummary properties[8];
	FileHandle stdest;
	FileHandle stprop;

	int dailyLocaleKeyLength = 0;
	Bptree dailyLocales;
	VgrStatsCountry *countryLocale = NULL;
	VgrStatsDailySummary *dailyLocale = NULL;

	int spiderKeyLength = 0;
	char spiderKey[32];
	Bptree dailySpiders;
	VgrStatsDailySummary *dailySpider = NULL;

	int urlCounter = 0;
	int urlKeyLength = 0;
	char *url = NULL;
	Bptree dailyUrls;
	VgrStatsDailySummary *dailyUrl = NULL;

	int refererCounter = 0;
	int refererKeyLength = 0;
	char *referer = NULL;
	Bptree dailyReferers;
	VgrStatsDailySummary *dailyReferer = NULL;

	int normalizedRefererCounter = 0;
	int normalizedRefererKeyLength = 0;
	char *normalizedReferer = NULL;
	Bptree dailyNormalizedReferers;
	VgrStatsDailySummary *dailyNormalizedReferer = NULL;

	int dailyExitUrlCounter = 0;
	int dailyExitUrlKeyLength = 0;
	Bptree dailyExitUrls;
	VgrStatsDailySummary *dailyExitUrl = NULL;

	int masterOSUidCounter = 1;
	Bptree masterOSs;
	VgrStatsOS *operatingSystem = NULL;

	int dailyOSKeyLength = 0;
	Bptree dailyOSs;
	VgrStatsDailySummary *dailyOS = NULL;

	int dailySearchEngineKeyLength = 0;
	Bptree dailySearchEngines;
	VgrStatsDailySummary *dailySearchEngine = NULL;

	aboolean dailySearchFound = afalse;
	int dailySearchLength = 0;
	int dailySearchCounter = 0;
	char *dailySearchKey = NULL;
	Bptree dailySearches;
	VgrStatsDailySummary *dailySearch = NULL;
	int decodedUrlLength = 0;
	char decodedUrl[HTTP_MAX_URL_LENGTH];
	VgrUri searchUri;

	int dailySearchKeywordLength = 0;
	int dailySearchKeywordCounter = 0;
	int dailySearchKeywordListLength = 0;
	char **dailySearchKeywordList = NULL;
	Bptree dailySearchKeywords;
	VgrStatsDailySummary *dailySearchKeyword = NULL;

	int dailyWebBrowserKeyLength = 0;
	Bptree dailyWebBrowsers;
	VgrStatsDailySummary *dailyWebBrowser = NULL;

	int dailyCityKeyLength = 0;
	char dailyCityKey[128];
	Bptree dailyCities;
	VgrStatsDailySummary *dailyCity = NULL;
	VgrStatsCity *city = NULL;

	int dailyStateKeyLength = 0;
	char dailyStateKey[128];
	Bptree dailyStates;
	VgrStatsDailySummary *dailyState = NULL;

	aboolean dailyRatePlanFound = afalse;
	int dailyRatePlanId = 0;
	int dailyRatePlanKeyLength = 0;
	char dailyRatePlanKey[128];
	Bptree dailyRatePlans;
	VgrStatsDailySummary *dailyRatePlan = NULL;

	int dailyHourStamp = 0;
	VgrStatsDailySummary hourlySummary[24];
	FileHandle sthourly;

	char *visitorPlatform = NULL;
	char *visitorUserAgent = NULL;
	char *visitorBrowserName = NULL;
	char *visitorClientIp = NULL;
	char *visitorCountryCode = NULL;
	char *visitorCountryName = NULL;
	char *visitorDateCreated = NULL;
	char *visitorDateModified = NULL;
	FileHandle stvisitor;

	timer = time_getTimeMus();

	// initialize file

	printf("Initializing detail data file...");
	fflush(NULL);

	if((rc = file_init(&fh, filename, "r", 8192)) < 0) {
		printf("FAILED at line %i, aborting.\n", __LINE__);
		return -1;
	}

	if(PERFORM_GEOIP_LOOKUP) {
		if((rc = file_init(&stvisitor, "siteTrackingVisitor.sql",
						"w", 8192)) < 0) {
			printf("FAILED at line %i, aborting.\n", __LINE__);
			return -1;
		}
	}

	if((rc = file_init(&stds, "siteTrackingDailySummary.sql",
					"w", 8192)) < 0) {
		printf("FAILED at line %i, aborting.\n", __LINE__);
		return -1;
	}

	context->stcampaign = &stds;

	if((rc = file_init(&stdest, "siteTrackingDestinationSummary.sql",
					"w", 8192)) < 0) {
		printf("FAILED at line %i, aborting.\n", __LINE__);
		return -1;
	}

	if((rc = file_init(&stprop, "siteTrackingPropertySummary.sql",
					"w", 8192)) < 0) {
		printf("FAILED at line %i, aborting.\n", __LINE__);
		return -1;
	}

	if((rc = file_init(&context->stlocale, "siteTrackingLocaleSummary.sql",
					"w", 8192)) < 0) {
		printf("FAILED at line %i, aborting.\n", __LINE__);
		return -1;
	}

	if((rc = file_init(&context->stspider, "siteTrackingSpiderSummary.sql",
					"w", 8192)) < 0) {
		printf("FAILED at line %i, aborting.\n", __LINE__);
		return -1;
	}

	if((rc = file_init(&context->sturl, "siteTrackingUrlSummary.sql",
					"w", 8192)) < 0) {
		printf("FAILED at line %i, aborting.\n", __LINE__);
		return -1;
	}

	if((rc = file_init(&context->streferer, "siteTrackingRefererSummary.sql",
					"w", 8192)) < 0) {
		printf("FAILED at line %i, aborting.\n", __LINE__);
		return -1;
	}

	if((rc = file_init(&context->stdomain, "siteTrackingDomainSummary.sql",
					"w", 8192)) < 0) {
		printf("FAILED at line %i, aborting.\n", __LINE__);
		return -1;
	}

	if((rc = file_init(&context->stexiturl, "siteTrackingExitUrlSummary.sql",
					"w", 8192)) < 0) {
		printf("FAILED at line %i, aborting.\n", __LINE__);
		return -1;
	}

	if((rc = file_init(&context->stvtr, "siteTrackingVisitorToReferer.sql",
					"w", 8192)) < 0) {
		printf("FAILED at line %i, aborting.\n", __LINE__);
		return -1;
	}

	if((rc = file_init(&context->stos, "siteTrackingOperatingSystemSummary.sql",
					"w", 8192)) < 0) {
		printf("FAILED at line %i, aborting.\n", __LINE__);
		return -1;
	}

	if((rc = file_init(&context->stse, "siteTrackingSearchEngineSummary.sql",
					"w", 8192)) < 0) {
		printf("FAILED at line %i, aborting.\n", __LINE__);
		return -1;
	}

	if((rc = file_init(&context->stsp,
					"siteTrackingSearchEnginePhraseSummary.sql",
					"w", 8192)) < 0) {
		printf("FAILED at line %i, aborting.\n", __LINE__);
		return -1;
	}

	if((rc = file_init(&context->stsk,
					"siteTrackingSearchEngineKeywordSummary.sql",
					"w", 8192)) < 0) {
		printf("FAILED at line %i, aborting.\n", __LINE__);
		return -1;
	}

	if((rc = file_init(&context->stwb, "siteTrackingWebBrowserSummary.sql",
					"w", 8192)) < 0) {
		printf("FAILED at line %i, aborting.\n", __LINE__);
		return -1;
	}

	if((rc = file_init(&context->stcity, "siteTrackingCitySummary.sql",
					"w", 8192)) < 0) {
		printf("FAILED at line %i, aborting.\n", __LINE__);
		return -1;
	}

	if((rc = file_init(&context->ststate, "siteTrackingStateSummary.sql",
					"w", 8192)) < 0) {
		printf("FAILED at line %i, aborting.\n", __LINE__);
		return -1;
	}

	if((rc = file_init(&context->strp, "siteTrackingRatePlanSummary.sql",
					"w", 8192)) < 0) {
		printf("FAILED at line %i, aborting.\n", __LINE__);
		return -1;
	}

	if((rc = file_init(&sthourly, "siteTrackingHourlySummary.sql",
					"w", 8192)) < 0) {
		printf("FAILED at line %i, aborting.\n", __LINE__);
		return -1;
	}

	if(REBUILD_TABLES) {
		writeLength = snprintf(buffer, (((int)sizeof(buffer)) - 1),
				"drop table if exists siteTrackingVisitor;\n"
				"create table siteTrackingVisitor\n"
				"(\n"
				"	uid						serial,\n"
				"	siteTrackingCampaignId	integer default 0,\n"
				"	siteUserId				integer default 0,\n"
				"	visits					integer default 1,\n"
				"	hash					char(40) not null,\n"
				"	platform				varchar(32) not null,\n"
				"	userAgent				varchar(512) not null,\n"
				"	clientIp				varchar(15) not null,\n"
				"	countryCode				varchar(2) not null default '',\n"
				"	countryName				varchar(128) not null default '',\n"
				"	siteStateId				numeric(32, 0) default 0,\n"
				"	siteCityId				numeric(32, 0) default 0,\n"
				"	dateCreated				timestamp not null default now(),\n"
				"	dateModified			timestamp not null default now()\n"
				");\n"
				"\n"
				"");

		if(PERFORM_GEOIP_LOOKUP) {
			file_write(&stvisitor, buffer, writeLength);
		}

		writeLength = snprintf(buffer, (((int)sizeof(buffer)) - 1),
				"drop table if exists siteTrackingDailySummary;\n"
				"create table siteTrackingDailySummary\n"
				"(\n"
				"	uid						serial,\n"
				"	siteTrackingCampaignId	integer default 0,\n"
				"	uniqueVisitors			integer default 0,\n"
				"	visits					integer default 0,\n"
				"	pageCount				integer default 0,\n"
				"	conversionCount			integer default 0,\n"
				"	totalTimeOnSite			integer default 0,\n"
				"	bounceCount				integer default 0,\n"
				"	summaryDate				timestamp not null default now(),\n"
				"	dateCreated				timestamp not null default now(),\n"
				"	dateModified			timestamp not null default now()\n"
				");\n"
				"\n"
				"");

		file_write(&stds, buffer, writeLength);

		writeLength = snprintf(buffer, (((int)sizeof(buffer)) - 1),
				"drop table if exists siteTrackingDestinationSummary;\n"
				"create table siteTrackingDestinationSummary\n"
				"(\n"
				"	uid						serial,\n"
				"	beDestinationId			integer default 0,\n"
				"	uniqueVisitors			integer default 0,\n"
				"	visits					integer default 0,\n"
				"	pageCount				integer default 0,\n"
				"	conversionCount			integer default 0,\n"
				"	totalTimeOnSite			integer default 0,\n"
				"	bounceCount				integer default 0,\n"
				"	summaryDate				timestamp not null default now(),\n"
				"	dateCreated				timestamp not null default now(),\n"
				"	dateModified			timestamp not null default now()\n"
				");\n"
				"\n"
				"");

		file_write(&stdest, buffer, writeLength);

		writeLength = snprintf(buffer, (((int)sizeof(buffer)) - 1),
				"drop table if exists siteTrackingPropertySummary;\n"
				"create table siteTrackingPropertySummary\n"
				"(\n"
				"	uid						serial,\n"
				"	bePropertyId			integer default 0,\n"
				"	uniqueVisitors			integer default 0,\n"
				"	visits					integer default 0,\n"
				"	pageCount				integer default 0,\n"
				"	conversionCount			integer default 0,\n"
				"	totalTimeOnSite			integer default 0,\n"
				"	bounceCount				integer default 0,\n"
				"	summaryDate				timestamp not null default now(),\n"
				"	dateCreated				timestamp not null default now(),\n"
				"	dateModified			timestamp not null default now()\n"
				");\n"
				"\n"
				"");

		file_write(&stprop, buffer, writeLength);

		writeLength = snprintf(buffer, (((int)sizeof(buffer)) - 1),
				"drop table if exists siteTrackingLocaleSummary;\n"
				"create table siteTrackingLocaleSummary\n"
				"(\n"
				"	uid						serial,\n"
				"	siteCountryId			integer default 0,\n"
				"	siteStateId				integer default 0,\n"
				"	siteCityId				integer default 0,\n"
				"	uniqueVisitors			integer default 0,\n"
				"	visits					integer default 0,\n"
				"	pageCount				integer default 0,\n"
				"	conversionCount			integer default 0,\n"
				"	totalTimeOnSite			integer default 0,\n"
				"	bounceCount				integer default 0,\n"
				"	summaryDate				timestamp not null default now(),\n"
				"	dateCreated				timestamp not null default now(),\n"
				"	dateModified			timestamp not null default now()\n"
				");\n"
				"\n"
				"");

		file_write(&context->stlocale, buffer, writeLength);

		writeLength = snprintf(buffer, (((int)sizeof(buffer)) - 1),
				"drop table if exists siteTrackingSpiderSummary;\n"
				"create table siteTrackingSpiderSummary\n"
				"(\n"
				"	uid						serial,\n"
				"	siteTrackingSpiderId	integer default 0,\n"
				"	uniqueVisitors			integer default 0,\n"
				"	visits					integer default 0,\n"
				"	pageCount				integer default 0,\n"
				"	conversionCount			integer default 0,\n"
				"	totalTimeOnSite			integer default 0,\n"
				"	bounceCount				integer default 0,\n"
				"	summaryDate				timestamp not null default now(),\n"
				"	dateCreated				timestamp not null default now(),\n"
				"	dateModified			timestamp not null default now()\n"
				");\n"
				"\n"
				"");

		file_write(&context->stspider, buffer, writeLength);

		writeLength = snprintf(buffer, (((int)sizeof(buffer)) - 1),
				"drop table if exists siteTrackingUrlSummary;\n"
				"create table siteTrackingUrlSummary\n"
				"(\n"
				"	uid						serial,\n"
				"	url						varchar(65536) not null,\n"
				"	uniqueVisitors			integer default 0,\n"
				"	visits					integer default 0,\n"
				"	pageCount				integer default 0,\n"
				"	beDestinationId			integer default 0,\n"
				"	bePropertyId			integer default 0,\n"
				"	summaryDate				timestamp not null default now(),\n"
				"	dateCreated				timestamp not null default now(),\n"
				"	dateModified			timestamp not null default now()\n"
				");\n"
				"\n"
				"");

		file_write(&context->sturl, buffer, writeLength);

		writeLength = snprintf(buffer, (((int)sizeof(buffer)) - 1),
				"drop table if exists siteTrackingRefererSummary;\n"
				"create table siteTrackingRefererSummary\n"
				"(\n"
				"	uid						serial,\n"
				"	referer					varchar(65536) not null,\n"
				"	uniqueVisitors			integer default 0,\n"
				"	visits					integer default 0,\n"
				"	pageCount				integer default 0,\n"
				"	beDestinationId			integer default 0,\n"
				"	bePropertyId			integer default 0,\n"
				"	conversionCount			integer default 0,\n"
				"	totalTimeOnSite			integer default 0,\n"
				"	bounceCount				integer default 0,\n"
				"	summaryDate				timestamp not null default now(),\n"
				"	dateCreated				timestamp not null default now(),\n"
				"	dateModified			timestamp not null default now()\n"
				");\n"
				"\n"
				"");

		file_write(&context->streferer, buffer, writeLength);

		writeLength = snprintf(buffer, (((int)sizeof(buffer)) - 1),
				"drop table if exists siteTrackingDomainSummary;\n"
				"create table siteTrackingDomainSummary\n"
				"(\n"
				"	uid						serial,\n"
				"	domain					varchar(65536) not null,\n"
				"	uniqueVisitors			integer default 0,\n"
				"	visits					integer default 0,\n"
				"	pageCount				integer default 0,\n"
				"	beDestinationId			integer default 0,\n"
				"	bePropertyId			integer default 0,\n"
				"	conversionCount			integer default 0,\n"
				"	totalTimeOnSite			integer default 0,\n"
				"	bounceCount				integer default 0,\n"
				"	summaryDate				timestamp not null default now(),\n"
				"	dateCreated				timestamp not null default now(),\n"
				"	dateModified			timestamp not null default now()\n"
				");\n"
				"\n"
				"");

		file_write(&context->stdomain, buffer, writeLength);

		writeLength = snprintf(buffer, (((int)sizeof(buffer)) - 1),
				"drop table if exists siteTrackingExitUrlSummary;\n"
				"create table siteTrackingExitUrlSummary\n"
				"(\n"
				"	uid						serial,\n"
				"	url						varchar(65536) not null,\n"
				"	uniqueVisitors			integer default 0,\n"
				"	pageCount				integer default 0,\n"
				"	summaryDate				timestamp not null default now(),\n"
				"	dateCreated				timestamp not null default now(),\n"
				"	dateModified			timestamp not null default now()\n"
				");\n"
				"\n"
				"");

		file_write(&context->stexiturl, buffer, writeLength);

		writeLength = snprintf(buffer, (((int)sizeof(buffer)) - 1),
				"drop table if exists siteTrackingVisitorToReferer;\n"
				"create table siteTrackingVisitorToReferer\n"
				"(\n"
				"	uid						serial,\n"
				"	siteTrackingVisitorId	integer default 0,\n"
				"	referer					varchar(65536) not null,\n"
				"	domain					varchar(65536) not null,\n"
				"	siteTrackingWebBrowserId	integer default 0,\n"
				"	summaryDate				timestamp not null default now(),\n"
				"	dateCreated				timestamp not null default now(),\n"
				"	dateModified			timestamp not null default now()\n"
				");\n"
				"\n"
				"");

		file_write(&context->stvtr, buffer, writeLength);

		writeLength = snprintf(buffer, (((int)sizeof(buffer)) - 1),
				"drop table if exists siteTrackingOperatingSystemSummary;\n"
				"create table siteTrackingOperatingSystemSummary\n"
				"(\n"
				"	uid									serial,\n"
				"	siteTrackingOperatingSystemId		integer default 0,\n"
				"	uniqueVisitors						integer default 0,\n"
				"	visits								integer default 0,\n"
				"	pageCount							integer default 0,\n"
				"	conversionCount						integer default 0,\n"
				"	totalTimeOnSite						integer default 0,\n"
				"	bounceCount							integer default 0,\n"
				"	summaryDate							timestamp not null default now(),\n"
				"	dateCreated							timestamp not null default now(),\n"
				"	dateModified						timestamp not null default now()\n"
				");\n"
				"\n"
				"");

		file_write(&context->stos, buffer, writeLength);

		writeLength = snprintf(buffer, (((int)sizeof(buffer)) - 1),
				"drop table if exists siteTrackingSearchEngineSummary;\n"
				"create table siteTrackingSearchEngineSummary\n"
				"(\n"
				"	uid							serial,\n"
				"	siteTrackingSearchEngineId	integer default 0,\n"
				"	uniqueVisitors				integer default 0,\n"
				"	visits						integer default 0,\n"
				"	pageCount					integer default 0,\n"
				"	conversionCount				integer default 0,\n"
				"	totalTimeOnSite				integer default 0,\n"
				"	bounceCount					integer default 0,\n"
				"	summaryDate					timestamp not null default now(),\n"
				"	dateCreated					timestamp not null default now(),\n"
				"	dateModified				timestamp not null default now()\n"
				");\n"
				"\n"
				"");

		file_write(&context->stse, buffer, writeLength);

		writeLength = snprintf(buffer, (((int)sizeof(buffer)) - 1),
				"drop table if exists siteTrackingSearchEnginePhraseSummary;\n"
				"create table siteTrackingSearchEnginePhraseSummary\n"
				"(\n"
				"	uid							serial,\n"
				"	siteTrackingSearchEngineId	integer default 0,\n"
				"	phrase						varchar(512) not null,\n"
				"	siteTrackingCampaignId		integer default 0,\n"
				"	uniqueVisitors				integer default 0,\n"
				"	visits						integer default 0,\n"
				"	pageCount					integer default 0,\n"
				"	conversionCount				integer default 0,\n"
				"	totalTimeOnSite				integer default 0,\n"
				"	bounceCount					integer default 0,\n"
				"	summaryDate					timestamp not null default now(),\n"
				"	dateCreated					timestamp not null default now(),\n"
				"	dateModified				timestamp not null default now()\n"
				");\n"
				"\n"
				"");

		file_write(&context->stsp, buffer, writeLength);

		writeLength = snprintf(buffer, (((int)sizeof(buffer)) - 1),
				"drop table if exists siteTrackingSearchEngineKeywordSummary;\n"
				"create table siteTrackingSearchEngineKeywordSummary\n"
				"(\n"
				"	uid							serial,\n"
				"	siteTrackingSearchEngineId	integer default 0,\n"
				"	keyword						varchar(512) not null,\n"
				"	siteTrackingCampaignId		integer default 0,\n"
				"	uniqueVisitors				integer default 0,\n"
				"	visits						integer default 0,\n"
				"	pageCount					integer default 0,\n"
				"	conversionCount				integer default 0,\n"
				"	totalTimeOnSite				integer default 0,\n"
				"	bounceCount					integer default 0,\n"
				"	summaryDate					timestamp not null default now(),\n"
				"	dateCreated					timestamp not null default now(),\n"
				"	dateModified				timestamp not null default now()\n"
				");\n"
				"\n"
				"");

		file_write(&context->stsk, buffer, writeLength);

		writeLength = snprintf(buffer, (((int)sizeof(buffer)) - 1),
				"drop table if exists siteTrackingWebBrowser;\n"
				"create table siteTrackingWebBrowser\n"
				"(\n"
				"	uid						serial,\n"
				"	name					varchar(128) not null,\n"
				"	description				varchar(1024) not null,\n"
				"	isEnabled				char(1) not null default '1',\n"
				"	url						varchar(65536) not null default '',\n"
				"	dateCreated				timestamp not null default now(),\n"
				"	dateModified			timestamp not null default now()\n"
				");\n"
				"\n"
				"create index siteTrackingWebBrowserUidIndex on siteTrackingWebBrowser(uid);\n"
				"\n"
				"");

		file_write(&context->stwb, buffer, writeLength);

		for(ii = 0; WEB_BROWSER_LIST[ii].id > 0; ii++) {
			writeLength = snprintf(buffer, (((int)sizeof(buffer)) - 1),
					"insert into siteTrackingWebBrowser ("
					"name, "
					"description) "
					"values ("
					"'%s', "
					"'Entry ''%s'' auto-generated.'"
					");\n",
					WEB_BROWSER_LIST[ii].name,
					WEB_BROWSER_LIST[ii].name);

			file_write(&context->stwb, buffer, writeLength);
		}

		writeLength = snprintf(buffer, (((int)sizeof(buffer)) - 1),
				"drop table if exists siteTrackingWebBrowserSummary;\n"
				"create table siteTrackingWebBrowserSummary\n"
				"(\n"
				"	uid							serial,\n"
				"	siteTrackingWebBrowserId	integer default 0,\n"
				"	uniqueVisitors				integer default 0,\n"
				"	visits						integer default 0,\n"
				"	pageCount					integer default 0,\n"
				"	conversionCount				integer default 0,\n"
				"	totalTimeOnSite				integer default 0,\n"
				"	bounceCount					integer default 0,\n"
				"	summaryDate					timestamp not null default now(),\n"
				"	dateCreated					timestamp not null default now(),\n"
				"	dateModified				timestamp not null default now()\n"
				");\n"
				"\n"
				"");

		file_write(&context->stwb, buffer, writeLength);

		writeLength = snprintf(buffer, (((int)sizeof(buffer)) - 1),
				"drop table if exists siteTrackingCitySummary;\n"
				"create table siteTrackingCitySummary\n"
				"(\n"
				"	uid							serial,\n"
				"	siteStateId					numeric(32, 0) default 0,\n"
				"	siteCityId					numeric(32, 0) default 0,\n"
				"	cityName					varchar(1024) not null default '',\n"
				"	uniqueVisitors				integer default 0,\n"
				"	visits						integer default 0,\n"
				"	pageCount					integer default 0,\n"
				"	conversionCount				integer default 0,\n"
				"	totalTimeOnSite 			integer default 0,\n"
				"	bounceCount 				integer default 0,\n"
				"	summaryDate					timestamp not null default now(),\n"
				"	dateCreated					timestamp not null default now(),\n"
				"	dateModified				timestamp not null default now()\n"
				");\n"
				"\n"
				"");

		file_write(&context->stcity, buffer, writeLength);

		writeLength = snprintf(buffer, (((int)sizeof(buffer)) - 1),
				"drop table if exists siteTrackingStateSummary;\n"
				"create table siteTrackingStateSummary\n"
				"(\n"
				"	uid							serial,\n"
				"	siteStateId					numeric(32, 0) default 0,\n"
				"	uniqueVisitors				integer default 0,\n"
				"	visits						integer default 0,\n"
				"	pageCount					integer default 0,\n"
				"	conversionCount				integer default 0,\n"
				"	totalTimeOnSite 			integer default 0,\n"
				"	bounceCount 				integer default 0,\n"
				"	summaryDate					timestamp not null default now(),\n"
				"	dateCreated					timestamp not null default now(),\n"
				"	dateModified				timestamp not null default now()\n"
				");\n"
				"\n"
				"");

		file_write(&context->ststate, buffer, writeLength);

		writeLength = snprintf(buffer, (((int)sizeof(buffer)) - 1),
				"drop table if exists siteTrackingRatePlanSummary;\n"
				"create table siteTrackingRatePlanSummary\n"
				"(\n"
				"	uid						serial,\n"
				"	bePropertyRatePlanId	integer default 0,\n"
				"	uniqueVisitors			integer default 0,\n"
				"	visits					integer default 0,\n"
				"	pageCount				integer default 0,\n"
				"	conversionCount			integer default 0,\n"
				"	totalTimeOnSite 		integer default 0,\n"
				"	bounceCount 			integer default 0,\n"
				"	summaryDate				timestamp not null default now(),\n"
				"	dateCreated				timestamp not null default now(),\n"
				"	dateModified			timestamp not null default now()\n"
				");\n"
				"\n"
				"");

		file_write(&context->strp, buffer, writeLength);

		writeLength = snprintf(buffer, (((int)sizeof(buffer)) - 1),
				"drop table if exists siteTrackingHourlySummary;\n"
				"create table siteTrackingHourlySummary\n"
				"(\n"
				"	uid						serial,\n"
				"	hour					integer default 0,\n"
				"	uniqueVisitors			integer default 0,\n"
				"	visits					integer default 0,\n"
				"	pageCount				integer default 0,\n"
				"	conversionCount			integer default 0,\n"
				"	totalTimeOnSite 		integer default 0,\n"
				"	bounceCount 			integer default 0,\n"
				"	summaryDate				timestamp not null default now(),\n"
				"	dateCreated				timestamp not null default now(),\n"
				"	dateModified			timestamp not null default now()\n"
				");\n"
				"\n"
				"");

		file_write(&sthourly, buffer, writeLength);
	}

	printf("OK\n");

	// process the file

	printf("Processing detail data file '%s'...", filename);
	fflush(NULL);

	memset((void *)displayBuffer, 0, (int)(sizeof(displayBuffer)));

	bptree_init(&dailyVisitors);
	bptree_setFreeFunction(&dailyVisitors, daily_visitorFree);

	bptree_init(&dailyCampaigns);
	bptree_setFreeFunction(&dailyCampaigns, daily_summaryFree);

	bptree_init(&dailyLocales);
	bptree_setFreeFunction(&dailyLocales, daily_summaryFree);

	bptree_init(&dailySpiders);
	bptree_setFreeFunction(&dailySpiders, daily_summaryFree);

	bptree_init(&dailyUrls);
	bptree_setFreeFunction(&dailyUrls, daily_summaryFree);

	bptree_init(&dailyReferers);
	bptree_setFreeFunction(&dailyReferers, daily_summaryFree);

	bptree_init(&dailyNormalizedReferers);
	bptree_setFreeFunction(&dailyNormalizedReferers, daily_summaryFree);

	bptree_init(&dailyExitUrls);
	bptree_setFreeFunction(&dailyExitUrls, daily_summaryFree);

	bptree_init(&masterOSs);
	bptree_setFreeFunction(&masterOSs, detail_freeOperatingSystem);

	bptree_init(&dailyOSs);
	bptree_setFreeFunction(&dailyOSs, daily_summaryFree);

	bptree_init(&dailySearchEngines);
	bptree_setFreeFunction(&dailySearchEngines, daily_summaryFree);

	bptree_init(&dailySearches);
	bptree_setFreeFunction(&dailySearches, daily_summaryFree);

	bptree_init(&dailySearchKeywords);
	bptree_setFreeFunction(&dailySearchKeywords, daily_summaryFree);

	bptree_init(&dailyWebBrowsers);
	bptree_setFreeFunction(&dailyWebBrowsers, daily_summaryFree);

	bptree_init(&dailyCities);
	bptree_setFreeFunction(&dailyCities, daily_summaryFree);

	bptree_init(&dailyStates);
	bptree_setFreeFunction(&dailyStates, daily_summaryFree);

	bptree_init(&dailyRatePlans);
	bptree_setFreeFunction(&dailyRatePlans, daily_summaryFree);

	memset(hourlySummary, 0, (int)(sizeof(hourlySummary)));

	for(ii = 0; SEARCH_ENGINE_LIST[ii].uid > 0; ii++) {
		SEARCH_ENGINE_LIST[ii].keyLength = strlen(SEARCH_ENGINE_LIST[ii].key);
	}

	dailySummary = (VgrStatsDailySummary *)malloc(
			sizeof(VgrStatsDailySummary));

	memset((void *)destinations, 0, (int)(sizeof(destinations)));
	memset((void *)properties, 0, (int)(sizeof(properties)));

	rc = file_readLine(&fh, buffer, sizeof(buffer), &readLength);
	while(rc == 0) {
/*		if(lineCounter < 5918903) {
			lineCounter++;
			rc = file_readLine(&fh, buffer, sizeof(buffer), &readLength);
			continue;
		}*/

		if((detail = detail_buildDetail(buffer, readLength)) == NULL) {
			lineCounter++;
			rc = file_readLine(&fh, buffer, sizeof(buffer), &readLength);
			continue;
		}

		if(VERBOSE_MODE) {
			detail_display(detail);
		}

		// increment tracking overview page counter

		context->trackingOverview.pageCount += 1;

		// get current year/month/day

		dateCreatedLength = strlen(detail->dateCreated);

		year = getElementFromDateStamp(detail->dateCreated, dateCreatedLength,
				VGR_DATE_ELEMENT_YEAR);

		month = getElementFromDateStamp(detail->dateCreated, dateCreatedLength,
				VGR_DATE_ELEMENT_MONTH);

		day = getElementFromDateStamp(detail->dateCreated, dateCreatedLength,
				VGR_DATE_ELEMENT_DAY);

		dailySecondStamp = getDateStampSeconds(detail->dateCreated,
				dateCreatedLength, &dailyHourStamp);

		// determine if it is a new day (by date)

		if((year != currentYear) ||
				(month != currentMonth) ||
				(day != currentDay)) {
			if(firstTime) {
				firstTime = afalse;
				dailySummary->dateStamp = dateStampFromNumbers(year,
						month, day);
			}
			else {
				// daily summary

				bptree_toArray(&dailyVisitors,
						&indexArrayLength,
						&indexKeyLengths,
						&indexKeys,
						&indexValues);

				detail_getDailyVisitorInfo(indexArrayLength, indexValues,
						&bounceCount, &totalTimeOnSite);

				// daily summary - destinations & properties

				for(ii = 0; ii < indexArrayLength; ii++) {
					visitor = (VgrStatsVisitor *)indexValues[ii];

					if(visitor->daily.destinationId >= 0) {
						destinationId = visitor->daily.destinationId;

						destinations[destinationId].uniqueVisitors += 1;
						destinations[destinationId].visits += 1;

						if(visitor->daily.pageCount <= 1) {
							destinations[destinationId].bounceCount += 1;
						}
						else {
							destinations[
								visitor->daily.destinationId].totalTimeOnSite +=
								visitor->daily.timeOnSite;
						}
					}

					if(visitor->daily.propertyId >= 0) {
						propertyId = visitor->daily.propertyId;

						properties[propertyId].uniqueVisitors += 1;
						properties[propertyId].visits += 1;

						if(visitor->daily.pageCount <= 1) {
							properties[propertyId].bounceCount += 1;
						}
						else {
							properties[propertyId].totalTimeOnSite +=
								visitor->daily.timeOnSite;
						}
					}
				}

				// daily summary - campaigns

				for(ii = 0; ii < indexArrayLength; ii++) {
					visitor = (VgrStatsVisitor *)indexValues[ii];

					if(visitor->siteTrackingCampaignId == 0) {
						continue;
					}

					campaignKeyLength = snprintf(campaignKey,
							(((int)sizeof(campaignKey)) - 1),
							"%i",
							visitor->siteTrackingCampaignId);

					if(bptree_get(&dailyCampaigns,
								campaignKey,
								campaignKeyLength,
								(void *)&dailyCampaign) < 0) {
						continue;
					}

					if(dailyCampaign == NULL) {
						continue;
					}

					if(visitor->daily.pageCount <= 1) {
						dailyCampaign->bounceCount += 1;
					}
					else {
						dailyCampaign->totalTimeOnSite +=
							visitor->daily.timeOnSite;
					}
				}

				// daily summary - locales

				for(ii = 0; ii < indexArrayLength; ii++) {
					visitor = (VgrStatsVisitor *)indexValues[ii];

					if(bptree_get(&dailyLocales,
								visitor->countryCode,
								strlen(visitor->countryCode),
								(void *)&dailyLocale) < 0) {
						continue;
					}

					if(dailyLocale == NULL) {
						continue;
					}

					if(visitor->daily.pageCount <= 1) {
						dailyLocale->bounceCount += 1;
					}
					else {
						dailyLocale->totalTimeOnSite +=
							visitor->daily.timeOnSite;
					}
				}

				// daily summary - spiders

				for(ii = 0; ii < indexArrayLength; ii++) {
					visitor = (VgrStatsVisitor *)indexValues[ii];

					if(visitor->siteTrackingSpiderId <= 0) {
						continue;
					}

					spiderKeyLength = snprintf(spiderKey,
							(((int)sizeof(spiderKey)) - 1),
							"%i",
							visitor->siteTrackingSpiderId);

					if(bptree_get(&dailySpiders,
								spiderKey,
								spiderKeyLength,
								(void *)&dailySpider) < 0) {
						continue;
					}

					if(dailySpider == NULL) {
						continue;
					}

					if(visitor->daily.pageCount <= 1) {
						dailySpider->bounceCount += 1;
					}
					else {
						dailySpider->totalTimeOnSite +=
							visitor->daily.timeOnSite;
					}
				}

				// daily summary - referers

				for(ii = 0; ii < indexArrayLength; ii++) {
					visitor = (VgrStatsVisitor *)indexValues[ii];

					if((visitor->daily.referer == NULL) ||
							((refererKeyLength = strlen(
									visitor->daily.referer)) < 1)) {
						continue;
					}

					if(bptree_get(&dailyReferers,
								visitor->daily.referer,
								refererKeyLength,
								(void *)&dailyReferer) < 0) {
						continue;
					}

					if(dailyReferer == NULL) {
						continue;
					}

					if(visitor->daily.pageCount <= 1) {
						dailyReferer->bounceCount += 1;
					}
					else {
						dailyReferer->totalTimeOnSite +=
							visitor->daily.timeOnSite;
					}
				}

				// daily summary - normalized referers

				for(ii = 0; ii < indexArrayLength; ii++) {
					visitor = (VgrStatsVisitor *)indexValues[ii];

					if((visitor->daily.normalizedReferer == NULL) ||
							((refererKeyLength = strlen(
									visitor->daily.normalizedReferer)) < 1)) {
						continue;
					}

					if(bptree_get(&dailyNormalizedReferers,
								visitor->daily.normalizedReferer,
								refererKeyLength,
								(void *)&dailyNormalizedReferer) < 0) {
						continue;
					}

					if(dailyNormalizedReferer == NULL) {
						continue;
					}

					if(visitor->daily.pageCount <= 1) {
						dailyNormalizedReferer->bounceCount += 1;
					}
					else {
						dailyNormalizedReferer->totalTimeOnSite +=
							visitor->daily.timeOnSite;
					}
				}

				// daily summary - exit urls

				for(ii = 0; ii < indexArrayLength; ii++) {
					visitor = (VgrStatsVisitor *)indexValues[ii];

					if((visitor->daily.exitUrl == NULL) ||
							(strcasestr(visitor->daily.exitUrl,
										"/admin")) ||
							(strcasestr(visitor->daily.exitUrl,
										"/web-services"))) {
						continue;
					}

					dailyExitUrlKeyLength = strlen(visitor->daily.exitUrl);

					if(bptree_get(&dailyExitUrls,
								visitor->daily.exitUrl,
								dailyExitUrlKeyLength,
								(void *)&dailyExitUrl) < 0) {
						dailyExitUrl = (VgrStatsDailySummary *)malloc(
								sizeof(VgrStatsDailySummary));
						dailyExitUrl->type = VGR_DAILY_SUMMARY_TYPE_EXIT_URL;
						dailyExitUrl->linkId = visitor->uid;
						dailyExitUrl->url = strdup(visitor->daily.exitUrl);
						dailyExitUrl->dateStamp = strdup(
								dailySummary->dateStamp);

						bptree_put(&dailyExitUrls,
								visitor->daily.exitUrl,
								dailyExitUrlKeyLength,
								(void *)dailyExitUrl);
					}

					dailyExitUrl->uniqueVisitors = (
							dailyExitUrl->uniqueVisitors +
							visitor->daily.exitUrlCounter +
							1);

					dailyExitUrl->pageCount += 1;
				}

				// daily summary - operating systems

				for(ii = 0; ii < indexArrayLength; ii++) {
					visitor = (VgrStatsVisitor *)indexValues[ii];

					if(bptree_get(&dailyOSs,
								visitor->platform,
								strlen(visitor->platform),
								(void *)&dailyOS) < 0) {
						continue;
					}

					if(dailyOS == NULL) {
						continue;
					}

					if(visitor->daily.pageCount <= 1) {
						dailyOS->bounceCount += 1;
					}
					else {
						dailyOS->totalTimeOnSite +=
							visitor->daily.timeOnSite;
					}
				}

				// daily summary - search engines

				for(ii = 0; ii < indexArrayLength; ii++) {
					visitor = (VgrStatsVisitor *)indexValues[ii];

					if((visitor->daily.searchEngine == NULL) ||
							((dailySearchEngineKeyLength = strlen(
									visitor->daily.searchEngine)) < 1)) {
						continue;
					}

					if(bptree_get(&dailySearchEngines,
								visitor->daily.searchEngine,
								dailySearchEngineKeyLength,
								(void *)&dailySearchEngine) < 0) {
						continue;
					}

					if(dailySearchEngine == NULL) {
						continue;
					}

					if(visitor->daily.pageCount <= 1) {
						dailySearchEngine->bounceCount += 1;
					}
					else {
						dailySearchEngine->totalTimeOnSite +=
							visitor->daily.timeOnSite;
					}
				}

				// daily summary - search phrases

				for(ii = 0; ii < indexArrayLength; ii++) {
					visitor = (VgrStatsVisitor *)indexValues[ii];

					if((visitor->daily.searchPhrase == NULL) ||
							((dailySearchLength = strlen(
									visitor->daily.searchPhrase)) < 1)) {
						continue;
					}

					if(bptree_get(&dailySearches,
								visitor->daily.searchPhrase,
								dailySearchLength,
								(void *)&dailySearch) < 0) {
						continue;
					}

					if(dailySearch == NULL) {
						continue;
					}

					if(visitor->daily.pageCount <= 1) {
						dailySearch->bounceCount += 1;
					}
					else {
						dailySearch->totalTimeOnSite +=
							visitor->daily.timeOnSite;
					}
				}

				// daily summary - web browser

				for(ii = 0; ii < indexArrayLength; ii++) {
					visitor = (VgrStatsVisitor *)indexValues[ii];

					if((visitor->browserName == NULL) ||
							((dailyWebBrowserKeyLength = strlen(
									visitor->browserName)) < 1)) {
						continue;
					}

					if(bptree_get(&dailyWebBrowsers,
								visitor->browserName,
								dailyWebBrowserKeyLength,
								(void *)&dailyWebBrowser) < 0) {
						continue;
					}

					if(dailyWebBrowser == NULL) {
						continue;
					}

					if(visitor->daily.pageCount <= 1) {
						dailyWebBrowser->bounceCount += 1;
					}
					else {
						dailyWebBrowser->totalTimeOnSite +=
							visitor->daily.timeOnSite;
					}
				}

				// daily summary - cities

				for(ii = 0; ii < indexArrayLength; ii++) {
					visitor = (VgrStatsVisitor *)indexValues[ii];

					if(visitor->siteCityId < 1) {
						continue;
					}

					dailyCityKeyLength = snprintf(dailyCityKey,
							((int)sizeof(dailyCityKey) - 1),
							"%i",
							visitor->siteCityId);

					if(bptree_get(&dailyCities,
								dailyCityKey,
								dailyCityKeyLength,
								(void *)&dailyCity) < 0) {
						continue;
					}

					if(dailyCity == NULL) {
						continue;
					}

					if(visitor->daily.pageCount <= 1) {
						dailyCity->bounceCount += 1;
					}
					else {
						dailyCity->totalTimeOnSite +=
							visitor->daily.timeOnSite;
					}
				}

				// daily summary - states

				for(ii = 0; ii < indexArrayLength; ii++) {
					visitor = (VgrStatsVisitor *)indexValues[ii];

					if(visitor->siteStateId < 1) {
						continue;
					}

					dailyStateKeyLength = snprintf(dailyStateKey,
							((int)sizeof(dailyStateKey) - 1),
							"%i",
							visitor->siteStateId);

					if(bptree_get(&dailyStates,
								dailyStateKey,
								dailyStateKeyLength,
								(void *)&dailyState) < 0) {
						continue;
					}

					if(dailyState == NULL) {
						continue;
					}

					if(visitor->daily.pageCount <= 1) {
						dailyState->bounceCount += 1;
					}
					else {
						dailyState->totalTimeOnSite +=
							visitor->daily.timeOnSite;
					}
				}

				// daily summary - rate plan

				for(ii = 0; ii < indexArrayLength; ii++) {
					visitor = (VgrStatsVisitor *)indexValues[ii];

					if(visitor->daily.ratePlanId <= 0) {
						continue;
					}

					dailyRatePlanKeyLength = snprintf(dailyRatePlanKey,
							((int)sizeof(dailyRatePlanKey) - 1),
							"%i",
							visitor->daily.ratePlanId);

					if(bptree_get(&dailyRatePlans,
								dailyRatePlanKey,
								dailyRatePlanKeyLength,
								(void *)&dailyRatePlan) < 0) {
						continue;
					}

					if(dailyRatePlan == NULL) {
						continue;
					}

					if(visitor->daily.pageCount <= 1) {
						dailyRatePlan->bounceCount += 1;
					}
					else {
						dailyRatePlan->totalTimeOnSite +=
							visitor->daily.timeOnSite;
					}
				}

				// hourly tracking summary

				for(ii = 0; ii < indexArrayLength; ii++) {
					visitor = (VgrStatsVisitor *)indexValues[ii];

					if(visitor == NULL) {
						continue;
					}

					if((visitor->daily.dailyHourStamp >= 0) &&
							(visitor->daily.dailyHourStamp <= 23)) {
						if(visitor->daily.pageCount <= 1) {
							hourlySummary[
								visitor->daily.dailyHourStamp
								].bounceCount += 1;
						}
						else {
							hourlySummary[
								visitor->daily.dailyHourStamp
								].totalTimeOnSite +=
								visitor->daily.timeOnSite;
						}
					}
				}

				// daily tracking - visitors to referers

				for(ii = 0; ii < indexArrayLength; ii++) {
					visitor = (VgrStatsVisitor *)indexValues[ii];

					if((visitor->daily.referer == NULL) ||
							(strlen(visitor->daily.referer) < 1) ||
							(visitor->daily.normalizedReferer == NULL) ||
							(strlen(visitor->daily.normalizedReferer) < 1) ||
							(strcasestr(visitor->daily.referer,
										"ntp.com")) ||
							(strcasestr(visitor->daily.referer,
										"onlinetravelconnect")) ||
							(strcasestr(visitor->daily.referer,
										"villagroupresorts")) ||
							(strcasestr(visitor->daily.referer,
										"islandsofloreto")) ||
							(strcasestr(visitor->daily.referer,
										"villadelpalmarloreto")) ||
							(strcasestr(visitor->daily.referer,
										"villapalmarcancun"))) {
						continue;
					}

					sqlString = sqlStrdup(visitor->daily.referer);

					writeLength = snprintf(buffer, (((int)sizeof(buffer)) - 1),
							"insert into siteTrackingVisitorToReferer ("
							"siteTrackingVisitorId, "
							"referer, "
							"domain, "
							"siteTrackingWebBrowserId, "
							"summaryDate) "
							"values ("
							"'%i', '%s', '%s', '%i', '%s'"
							");\n",
							visitor->uid,
							sqlString,
							visitor->daily.normalizedReferer,
							visitor->browserId,
							dailySummary->dateStamp);

//					printf("\n\n%s\n\n", buffer);

					file_write(&context->stvtr, buffer, writeLength);

					free(sqlString);
				}

				// destination summaries

				for(ii = 0; ii < 5; ii++) {
					if(destinations[ii].uniqueVisitors > 0) {
						writeLength = snprintf(buffer,
								(((int)sizeof(buffer)) - 1),
								"insert into siteTrackingDestinationSummary ("
								"beDestinationId, "
								"uniqueVisitors, "
								"visits, "
								"pageCount, "
								"conversionCount, "
								"bounceCount, "
								"totalTimeOnSite, "
								"summaryDate) "
								"values ("
								"'%i', '%i', '%i', '%i', '%i', '%i', '%i', '%s'"
								");\n",
								daily_getDestinationUid(ii),
								destinations[ii].uniqueVisitors,
								destinations[ii].visits,
								destinations[ii].pageCount,
								destinations[ii].conversions,
								destinations[ii].bounceCount,
								destinations[ii].totalTimeOnSite,
								destinations[ii].dateStamp);

//						printf("\n\n%s\n\n", buffer);

						file_write(&stdest, buffer, writeLength);
					}
				}

				// property summaries

				for(ii = 0; ii < 8; ii++) {
					if(properties[ii].uniqueVisitors > 0) {
						writeLength = snprintf(buffer,
								(((int)sizeof(buffer)) - 1),
								"insert into siteTrackingPropertySummary ("
								"bePropertyId, "
								"uniqueVisitors, "
								"visits, "
								"pageCount, "
								"conversionCount, "
								"bounceCount, "
								"totalTimeOnSite, "
								"summaryDate) "
								"values ("
								"'%i', '%i', '%i', '%i', '%i', '%i', '%i', '%s'"
								");\n",
								daily_getPropertyUid(ii),
								properties[ii].uniqueVisitors,
								properties[ii].visits,
								properties[ii].pageCount,
								properties[ii].conversions,
								properties[ii].bounceCount,
								properties[ii].totalTimeOnSite,
								properties[ii].dateStamp);

//						printf("\n\n%s\n\n", buffer);

						file_write(&stprop, buffer, writeLength);
					}
				}

				// write daily summary

				writeLength = snprintf(buffer, (((int)sizeof(buffer)) - 1),
						"insert into siteTrackingDailySummary ("
						"siteTrackingCampaignId, "
						"uniqueVisitors, "
						"visits, "
						"pageCount, "
						"conversionCount, "
						"bounceCount, "
						"totalTimeOnSite, "
						"summaryDate) "
						"values ("
						"'%i', '%i', '%i', '%i', '%i', '%i', '%i', '%s'"
						");\n",
						0,
						dailySummary->uniqueVisitors,
						dailySummary->visits,
						dailySummary->pageCount,
						dailySummary->conversions,
						bounceCount,
						totalTimeOnSite,
						dailySummary->dateStamp);

//				printf("\n\n%s\n\n", buffer);

				file_write(&stds, buffer, writeLength);

				// free daily summary b+tree arrays

				if(indexKeys != NULL) {
					for(ii = 0; ii < indexArrayLength; ii++) {
						if(indexKeys[ii] != NULL) {
							free(indexKeys[ii]);
						}
					}

					free(indexKeys);
				}

				if(indexValues != NULL) {
					free(indexValues);
				}

				// sort the daily urls

				bptree_toArray(&dailyUrls,
						&indexArrayLength,
						&indexKeyLengths,
						&indexKeys,
						&indexValues);

				qsort((void *)indexValues,
						indexArrayLength,
						(int)sizeof(VgrStatsDailySummary *),
						detail_descendingVgrStatsDailySummaryComparison);

				detail_buildDestinationAndPropertyUrlSummaries(indexArrayLength,
						indexValues);

				for(ii = 0, urlCounter = 0; ii < indexArrayLength; ii++) {
					if(urlCounter >= MAX_DAILY_RECORDS) {
						((VgrStatsDailySummary **)indexValues)[ii]->linkId = 0;
					}
					else if(urlCounter < MAX_DAILY_RECORDS) {
						if(((VgrStatsDailySummary **)
									indexValues)[ii]->pageCount < 1) {
							urlCounter = MAX_DAILY_RECORDS;
						}
						else {
							urlCounter++;
						}
					}

//printf("URL :: %06i :: %i -> '%s'\n", ii, ((VgrStatsDailySummary **)indexValues)[ii]->pageCount, ((VgrStatsDailySummary **)indexValues)[ii]->url);
				}

				if(indexKeys != NULL) {
					for(ii = 0; ii < indexArrayLength; ii++) {
						if(indexKeys[ii] != NULL) {
							free(indexKeys[ii]);
						}
					}

					free(indexKeys);
				}

				if(indexValues != NULL) {
					free(indexValues);
				}

				// sort the daily referers

				bptree_toArray(&dailyReferers,
						&indexArrayLength,
						&indexKeyLengths,
						&indexKeys,
						&indexValues);

				qsort((void *)indexValues,
						indexArrayLength,
						(int)sizeof(VgrStatsDailySummary *),
						detail_descendingVgrStatsDailySummaryComparison);

				detail_buildDestinationAndPropertyUrlSummaries(indexArrayLength,
						indexValues);

				for(ii = 0, refererCounter = 0; ii < indexArrayLength; ii++) {
//printf("REFERER :: %06i :: %i -> '%s' -> '%s'\n", ii, ((VgrStatsDailySummary **)indexValues)[ii]->pageCount, ((VgrStatsDailySummary **)indexValues)[ii]->referer, ((VgrStatsDailySummary **)indexValues)[ii]->url);
					if(refererCounter >= MAX_DAILY_RECORDS) {
						((VgrStatsDailySummary **)indexValues)[ii]->linkId = 0;
					}
					else if(refererCounter < MAX_DAILY_RECORDS) {
						if(((VgrStatsDailySummary **)
									indexValues)[ii]->pageCount < 1) {
							refererCounter = MAX_DAILY_RECORDS;
						}
						else {
							refererCounter++;
						}
					}
				}

				if(indexKeys != NULL) {
					for(ii = 0; ii < indexArrayLength; ii++) {
						if(indexKeys[ii] != NULL) {
							free(indexKeys[ii]);
						}
					}

					free(indexKeys);
				}

				if(indexValues != NULL) {
					free(indexValues);
				}

				// sort the normalized daily referers

				bptree_toArray(&dailyNormalizedReferers,
						&indexArrayLength,
						&indexKeyLengths,
						&indexKeys,
						&indexValues);

				qsort((void *)indexValues,
						indexArrayLength,
						(int)sizeof(VgrStatsDailySummary *),
						detail_descendingVgrStatsDailySummaryComparison);

				detail_buildDestinationAndPropertyUrlSummaries(indexArrayLength,
						indexValues);

				for(ii = 0, normalizedRefererCounter = 0;
						ii < indexArrayLength;
						ii++) {
//printf("REFERER :: %06i :: %i -> '%s' -> '%s'\n", ii, ((VgrStatsDailySummary **)indexValues)[ii]->pageCount, ((VgrStatsDailySummary **)indexValues)[ii]->referer, ((VgrStatsDailySummary **)indexValues)[ii]->url);
					if(normalizedRefererCounter >= MAX_DAILY_RECORDS) {
						((VgrStatsDailySummary **)indexValues)[ii]->linkId = 0;
					}
					else if(normalizedRefererCounter < MAX_DAILY_RECORDS) {
						if(((VgrStatsDailySummary **)
									indexValues)[ii]->pageCount < 1) {
							normalizedRefererCounter = MAX_DAILY_RECORDS;
						}
						else {
							normalizedRefererCounter++;
						}
					}
				}

				if(indexKeys != NULL) {
					for(ii = 0; ii < indexArrayLength; ii++) {
						if(indexKeys[ii] != NULL) {
							free(indexKeys[ii]);
						}
					}

					free(indexKeys);
				}

				if(indexValues != NULL) {
					free(indexValues);
				}

				// sort the exit urls

				bptree_toArray(&dailyExitUrls,
						&indexArrayLength,
						&indexKeyLengths,
						&indexKeys,
						&indexValues);

				qsort((void *)indexValues,
						indexArrayLength,
						(int)sizeof(VgrStatsDailySummary *),
						detail_descendingVgrStatsDailySummaryComparison);

// TODO: determine if daily exit urls should be broken down this way
//				detail_buildDestinationAndPropertyUrlSummaries(indexArrayLength,
//						indexValues);

				for(ii = 0, dailyExitUrlCounter = 0;
						ii < indexArrayLength;
						ii++) {
//printf("EXIT URL :: %06i :: %i -> '%s'\n", ii, ((VgrStatsDailySummary **)indexValues)[ii]->pageCount, ((VgrStatsDailySummary **)indexValues)[ii]->url);
					if(dailyExitUrlCounter >= MAX_DAILY_RECORDS) {
						((VgrStatsDailySummary **)indexValues)[ii]->linkId = 0;
					}
					else if(dailyExitUrlCounter < MAX_DAILY_RECORDS) {
						if(((VgrStatsDailySummary **)
									indexValues)[ii]->pageCount < 1) {
							dailyExitUrlCounter = MAX_DAILY_RECORDS;
						}
						else {
							dailyExitUrlCounter++;
						}
					}
				}

				if(indexKeys != NULL) {
					for(ii = 0; ii < indexArrayLength; ii++) {
						if(indexKeys[ii] != NULL) {
							free(indexKeys[ii]);
						}
					}

					free(indexKeys);
				}

				if(indexValues != NULL) {
					free(indexValues);
				}

				// sort the search phrases

				bptree_toArray(&dailySearches,
						&indexArrayLength,
						&indexKeyLengths,
						&indexKeys,
						&indexValues);

				qsort((void *)indexValues,
						indexArrayLength,
						(int)sizeof(VgrStatsDailySummary *),
						detail_descendingVgrStatsDailySummaryComparison);

				for(ii = 0, dailySearchCounter = 0;
						ii < indexArrayLength;
						ii++) {
//printf("SEARCH PHRASE :: %06i :: %i -> '%s'\n", ii, ((VgrStatsDailySummary **)indexValues)[ii]->pageCount, ((VgrStatsDailySummary **)indexValues)[ii]->url);
					if(dailySearchCounter >= MAX_DAILY_RECORDS) {
						((VgrStatsDailySummary **)indexValues)[ii]->linkId = 0;
					}
					else if(dailySearchCounter < MAX_DAILY_RECORDS) {
						if(((VgrStatsDailySummary **)
									indexValues)[ii]->pageCount < 1) {
							dailySearchCounter = MAX_DAILY_RECORDS;
						}
						else {
							dailySearchCounter++;
						}
					}

					dailySearch = ((VgrStatsDailySummary **)indexValues)[ii];

					// build the daily keyword index

					dailySearchKeywordList = NULL;
					dailySearchKeywordListLength = 0;

					dailySearchKeywordList = buildKeywordList(
							dailySearch->url, strlen(dailySearch->url),
							&dailySearchKeywordListLength);

					if(dailySearchKeywordList != NULL) {
						for(nn = 0; nn < dailySearchKeywordListLength; nn++) {
//printf("SEARCH KEYWORDS (%s) :: %02i :: '%s'\n", dailySearch->url, nn, dailySearchKeywordList[nn]);
							dailySearchKeywordLength = strlen(
									dailySearchKeywordList[nn]);

							if(bptree_get(&dailySearchKeywords,
										dailySearchKeywordList[nn],
										dailySearchKeywordLength,
										(void *)&dailySearchKeyword) < 0) {
								dailySearchKeyword =
									(VgrStatsDailySummary *)malloc(
										sizeof(VgrStatsDailySummary));
								dailySearchKeyword->type =
									VGR_DAILY_SUMMARY_TYPE_SEARCH_ENGINE_KEYWORD;
								dailySearchKeyword->linkId = 1;
								dailySearchKeyword->uniqueVisitors =
									dailySearch->uniqueVisitors;
								dailySearchKeyword->visits =
									dailySearch->visits;
								dailySearchKeyword->pageCount =
									dailySearch->pageCount;
								dailySearchKeyword->conversions =
									dailySearch->conversions;
								dailySearchKeyword->bounceCount =
									dailySearch->bounceCount;
								dailySearchKeyword->totalTimeOnSite =
									dailySearch->totalTimeOnSite;
								dailySearchKeyword->url = strndup(
										dailySearchKeywordList[nn],
										dailySearchKeywordLength);
								dailySearchKeyword->dateStamp = strdup(
										dailySummary->dateStamp);

								bptree_put(&dailySearchKeywords,
										dailySearchKeywordList[nn],
										dailySearchKeywordLength,
										(void *)dailySearchKeyword);
							}
							else {
								dailySearchKeyword->uniqueVisitors +=
									dailySearch->uniqueVisitors;
								dailySearchKeyword->visits +=
									dailySearch->visits;
								dailySearchKeyword->pageCount +=
									dailySearch->pageCount;
								dailySearchKeyword->conversions +=
									dailySearch->conversions;
								dailySearchKeyword->bounceCount +=
									dailySearch->bounceCount;
								dailySearchKeyword->totalTimeOnSite +=
									dailySearch->totalTimeOnSite;
							}

							free(dailySearchKeywordList[nn]);
						}

						free(dailySearchKeywordList);
					}
				}

				if(indexKeys != NULL) {
					for(ii = 0; ii < indexArrayLength; ii++) {
						if(indexKeys[ii] != NULL) {
							free(indexKeys[ii]);
						}
					}

					free(indexKeys);
				}

				if(indexValues != NULL) {
					free(indexValues);
				}

				// sort the search keywords

				bptree_toArray(&dailySearchKeywords,
						&indexArrayLength,
						&indexKeyLengths,
						&indexKeys,
						&indexValues);

				qsort((void *)indexValues,
						indexArrayLength,
						(int)sizeof(VgrStatsDailySummary *),
						detail_descendingVgrStatsDailySummaryComparison);

				for(ii = 0, dailySearchKeywordCounter = 0;
						ii < indexArrayLength;
						ii++) {
//printf("SEARCH KEYWORD :: %06i :: %i -> '%s'\n", ii, ((VgrStatsDailySummary **)indexValues)[ii]->pageCount, ((VgrStatsDailySummary **)indexValues)[ii]->url);
					if(dailySearchKeywordCounter >= MAX_DAILY_RECORDS) {
						((VgrStatsDailySummary **)indexValues)[ii]->linkId = 0;
					}
					else if(dailySearchKeywordCounter < MAX_DAILY_RECORDS) {
						if(((VgrStatsDailySummary **)
									indexValues)[ii]->pageCount < 1) {
							dailySearchKeywordCounter = MAX_DAILY_RECORDS;
						}
						else {
							dailySearchKeywordCounter++;
						}
					}
				}

				if(indexKeys != NULL) {
					for(ii = 0; ii < indexArrayLength; ii++) {
						if(indexKeys[ii] != NULL) {
							free(indexKeys[ii]);
						}
					}

					free(indexKeys);
				}

				if(indexValues != NULL) {
					free(indexValues);
				}

				// reset daily summary

				dailySummary->uniqueVisitors = 0;
				dailySummary->visits = 0;
				dailySummary->pageCount = 0;
				dailySummary->conversions = 0;

				free(dailySummary->dateStamp);

				dailySummary->dateStamp = dateStampFromNumbers(year,
						month, day);

				// reset destination summaries

				for(ii = 0; ii < 5; ii++) {
					if(destinations[ii].dateStamp != NULL) {
						free(destinations[ii].dateStamp);
					}
				}

				memset((void *)destinations, 0, (int)(sizeof(destinations)));

				for(ii = 0; ii < 5; ii++) {
					destinations[ii].dateStamp = strdup(
							dailySummary->dateStamp);
				}

				// reset property summaries

				for(ii = 0; ii < 8; ii++) {
					if(properties[ii].dateStamp != NULL) {
						free(properties[ii].dateStamp);
					}
				}

				memset((void *)properties, 0, (int)(sizeof(properties)));

				for(ii = 0; ii < 8; ii++) {
					properties[ii].dateStamp = strdup(dailySummary->dateStamp);
				}
			}

//			printf("DETAIL @ -=-=-=- NEW DAY -=-=-=-\n");

			bptree_free(&dailyVisitors);
			bptree_init(&dailyVisitors);
			bptree_setFreeFunction(&dailyVisitors, daily_visitorFree);

			bptree_free(&dailyCampaigns);
			bptree_init(&dailyCampaigns);
			bptree_setFreeFunction(&dailyCampaigns, daily_summaryFree);

			bptree_free(&dailyLocales);
			bptree_init(&dailyLocales);
			bptree_setFreeFunction(&dailyLocales, daily_summaryFree);

			bptree_free(&dailySpiders);
			bptree_init(&dailySpiders);
			bptree_setFreeFunction(&dailySpiders, daily_summaryFree);

			bptree_free(&dailyUrls);
			bptree_init(&dailyUrls);
			bptree_setFreeFunction(&dailyUrls, daily_summaryFree);

			bptree_free(&dailyReferers);
			bptree_init(&dailyReferers);
			bptree_setFreeFunction(&dailyReferers, daily_summaryFree);

			bptree_free(&dailyNormalizedReferers);
			bptree_init(&dailyNormalizedReferers);
			bptree_setFreeFunction(&dailyNormalizedReferers, daily_summaryFree);

			bptree_free(&dailyExitUrls);
			bptree_init(&dailyExitUrls);
			bptree_setFreeFunction(&dailyExitUrls, daily_summaryFree);

			bptree_free(&dailyOSs);
			bptree_init(&dailyOSs);
			bptree_setFreeFunction(&dailyOSs, daily_summaryFree);

			bptree_free(&dailySearchEngines);
			bptree_init(&dailySearchEngines);
			bptree_setFreeFunction(&dailySearchEngines, daily_summaryFree);

			bptree_free(&dailySearches);
			bptree_init(&dailySearches);
			bptree_setFreeFunction(&dailySearches, daily_summaryFree);

			bptree_free(&dailySearchKeywords);
			bptree_init(&dailySearchKeywords);
			bptree_setFreeFunction(&dailySearchKeywords, daily_summaryFree);

			bptree_free(&dailyWebBrowsers);
			bptree_init(&dailyWebBrowsers);
			bptree_setFreeFunction(&dailyWebBrowsers, daily_summaryFree);

			bptree_free(&dailyCities);
			bptree_init(&dailyCities);
			bptree_setFreeFunction(&dailyCities, daily_summaryFree);

			bptree_free(&dailyStates);
			bptree_init(&dailyStates);
			bptree_setFreeFunction(&dailyStates, daily_summaryFree);

			bptree_free(&dailyRatePlans);
			bptree_init(&dailyRatePlans);
			bptree_setFreeFunction(&dailyRatePlans, daily_summaryFree);

			currentYear = year;
			currentMonth = month;
			currentDay = day;

			dayCounter++;
		}

//		printf("DETAIL @ %04i-%02i-%02i :: %i\n", year, month, day,
//				dailySecondStamp);

		// update destination & property page counts

		destinationId = -1;
		propertyId = -1;

		if(strcasestr(detail->url,
					"/destinations/cabo-san-lucas")) {
			destinationId = VGR_DESTINATION_CABO;
		}
		else if(strcasestr(detail->url,
					"/destinations/cancun")) {
			destinationId = VGR_DESTINATION_CANCUN;
		}
		else if(strcasestr(detail->url,
					"/destinations/loreto")) {
			destinationId = VGR_DESTINATION_LORETO;
		}
		else if(strcasestr(detail->url,
					"/destinations/puerto-vallarta")) {
			destinationId = VGR_DESTINATION_PUERTO_VALLARTA;
		}
		else if(strcasestr(detail->url,
					"/destinations/riviera-nayarit")) {
			destinationId = VGR_DESTINATION_NUEVO_VALLARTA;
		}
		else if(strcasestr(detail->url,
					"/resorts/villa-la-estancia-cabo")) {
			propertyId = VGR_PROPERTY_VLE_CABO;
		}
		else if(strcasestr(detail->url,
					"/resorts/villa-del-palmar-cabo")) {
			propertyId = VGR_PROPERTY_VDP_CABO;
		}
		else if(strcasestr(detail->url,
					"/resorts/villa-del-arco")) {
			propertyId = VGR_PROPERTY_VILLA_DEL_ARCO;
		}
		else if(strcasestr(detail->url,
					"/resorts/villa-del-palmar-loreto")) {
			propertyId = VGR_PROPERTY_VDP_LORETO;
		}
		else if(strcasestr(detail->url,
					"/resorts/villa-del-palmar-puerto-vallarta")) {
			propertyId = VGR_PROPERTY_VDP_PUERTO_VALLARTA;
		}
		else if(strcasestr(detail->url,
					"/resorts/villa-la-estancia-riviera-nayarit")) {
			propertyId = VGR_PROPERTY_VLE_NAYARIT;
		}
		else if(strcasestr(detail->url,
					"/resorts/villa-del-palmar-flamingos")) {
			propertyId = VGR_PROPERTY_FLAMINGOS;
		}
		else if(strcasestr(detail->url,
					"/resorts/villa-del-palmar-cancun")) {
			propertyId = VGR_PROPERTY_VDP_CANCUN;
		}

		if(destinationId >= 0) {
			destinations[destinationId].pageCount += 1;
		}
		else if(propertyId >= 0) {
			properties[propertyId].pageCount += 1;
		}

		// update daily summary page count

		dailySummary->pageCount += 1;

		// perform URL lookup & tracking

		url = detail->normalizedUrl;
		urlKeyLength = strlen(url);
		dailyUrl = NULL;

		if((strcasestr(detail->url, "/admin") == NULL) &&
				(strcasestr(detail->url, "/web-services") == NULL) &&
				(strcasestr(detail->url, ".png") == NULL) &&
				(strcasestr(detail->url, ".jpg") == NULL) &&
				(strcasestr(detail->url, ".gif") == NULL) &&
				(strcasestr(detail->url, ".js") == NULL) &&
				(strcasestr(detail->url, ".txt") == NULL) &&
				(strcasestr(detail->url, ".pdf") == NULL) &&
				(strcasestr(detail->url, ".css") == NULL) &&
				(strcasestr(detail->url, ".style") == NULL) &&
				(strcasestr(detail->url, ".defaultview") == NULL) &&
				(strcasestr(detail->url, ".php") == NULL) &&
				(strcasestr(detail->url, ".swf") == NULL) &&
				(strcasestr(detail->url, ".ico") == NULL) &&
				(strcasestr(detail->url, "this.") == NULL) &&
				(strcasestr(detail->url, "/http:") == NULL)) {
			if(bptree_get(&dailyUrls, url, urlKeyLength,
						(void *)&dailyUrl) < 0) {
				dailyUrl = (VgrStatsDailySummary *)malloc(
						sizeof(VgrStatsDailySummary));
				dailyUrl->type = VGR_DAILY_SUMMARY_TYPE_URL;
				dailyUrl->linkId = 1;
				dailyUrl->uniqueVisitors += 1;
				dailyUrl->visits += 1;
				dailyUrl->url = strndup(url, urlKeyLength);
				dailyUrl->dateStamp = strdup(dailySummary->dateStamp);

				bptree_put(&dailyUrls, url, urlKeyLength, (void *)dailyUrl);
			}
		}

		if(dailyUrl != NULL) {
			dailyUrl->pageCount += 1;
		}

		// perform referer lookup & tracking

		dailyReferer = NULL;

		if((detail->referer != NULL) &&
				((refererKeyLength = strlen(detail->referer)) > 0) &&
				(strcasestr(detail->referer, "ntp.com") == NULL) &&
				(strcasestr(detail->referer, "onlinetravelconnect") == NULL) &&
				(strcasestr(detail->referer, "villagroupresorts") == NULL) &&
				(strcasestr(detail->referer, "islandsofloreto") == NULL) &&
				(strcasestr(detail->referer, "villadelpalmarloreto") == NULL) &&
				(strcasestr(detail->referer, "villapalmarcancun") == NULL)) {
			referer = detail->referer;

			if(bptree_get(&dailyReferers, referer, refererKeyLength,
						(void *)&dailyReferer) < 0) {
				dailyReferer = (VgrStatsDailySummary *)malloc(
						sizeof(VgrStatsDailySummary));
				dailyReferer->type = VGR_DAILY_SUMMARY_TYPE_REFERER;
				dailyReferer->linkId = 1;
				dailyReferer->uniqueVisitors += 1;
				dailyReferer->visits += 1;
				dailyReferer->url = strdup(detail->normalizedUrl);
				dailyReferer->referer = strndup(referer, refererKeyLength);
				dailyReferer->dateStamp = strdup(dailySummary->dateStamp);

				bptree_put(&dailyReferers, referer, refererKeyLength,
						(void *)dailyReferer);
			}
		}

		if(dailyReferer != NULL) {
			dailyReferer->pageCount += 1;
		}

		// perform normalized referer lookup & tracking

		dailyNormalizedReferer = NULL;

		if((detail->normalizedReferer != NULL) &&
				((normalizedRefererKeyLength = strlen(
							detail->normalizedReferer)) > 0) &&
				(strcasestr(detail->normalizedReferer,
							"ntp.com") == NULL) &&
				(strcasestr(detail->normalizedReferer,
							"onlinetravelconnect") == NULL) &&
				(strcasestr(detail->normalizedReferer,
							"villagroupresorts") == NULL) &&
				(strcasestr(detail->normalizedReferer,
							"islandsofloreto") == NULL) &&
				(strcasestr(detail->normalizedReferer,
							"villadelpalmarloreto") == NULL) &&
				(strcasestr(detail->normalizedReferer,
							"villapalmarcancun") == NULL)) {
			normalizedReferer = detail->normalizedReferer;

			if(bptree_get(&dailyNormalizedReferers, normalizedReferer,
						normalizedRefererKeyLength,
						(void *)&dailyNormalizedReferer) < 0) {
				dailyNormalizedReferer = (VgrStatsDailySummary *)malloc(
						sizeof(VgrStatsDailySummary));
				dailyNormalizedReferer->type =
					VGR_DAILY_SUMMARY_TYPE_REFERER_NORMALIZED;
				dailyNormalizedReferer->linkId = 1;
				dailyNormalizedReferer->uniqueVisitors += 1;
				dailyNormalizedReferer->visits += 1;
				dailyNormalizedReferer->url = strdup(detail->normalizedUrl);
				dailyNormalizedReferer->referer = strndup(normalizedReferer,
						normalizedRefererKeyLength);
				dailyNormalizedReferer->dateStamp = strdup(
						dailySummary->dateStamp);

				bptree_put(&dailyNormalizedReferers, normalizedReferer,
						normalizedRefererKeyLength,
						(void *)dailyNormalizedReferer);
			}
		}

		if(dailyNormalizedReferer != NULL) {
			dailyNormalizedReferer->pageCount += 1;
		}

		// perform visitor lookup & tracking

		keyLength = strlen(detail->key);

		visitor = NULL;
		dailyCampaign = NULL;
		dailyLocale = NULL;
		dailySpider = NULL;
		dailyOS = NULL;
		dailySearchEngine = NULL;
		dailySearch = NULL;
		dailyWebBrowser = NULL;
		dailyCity = NULL;
		dailyState = NULL;
		dailyRatePlan = NULL;

		if(bptree_get(&dailyVisitors, detail->key, keyLength,
					(void *)&visitor) < 0) {
			if(bptree_get(&context->visitors, detail->key, keyLength,
						(void *)&visitor) < 0) {
				fprintf(stderr, "error - failed to find visitor record "
						"#%s at line %i on day %s.\n",
						detail->key, lineCounter, dailySummary->dateStamp);
			}
			else {
//printf("\t NEW VISITOR #%i => %s\n", visitor->uid, detail->referer);
				visitor->daily.trackingTimestamp = dailySecondStamp;
				visitor->daily.timeOnSite = 0;
				visitor->daily.dailySecondStamp = dailySecondStamp;
				visitor->daily.dailyHourStamp = dailyHourStamp;

				if(detail->referer != NULL) {
					visitor->daily.referer = strdup(detail->referer);
				}

				if(detail->normalizedReferer != NULL) {
					visitor->daily.normalizedReferer = strdup(
							detail->normalizedReferer);
				}

				if(visitor->siteTrackingCampaignId > 0) {
//printf("\n[day: %i (%s)] NEW CAMPAIGN #%03i VISITOR #%i => %s\n", dayCounter, dailySummary->dateStamp, visitor->siteTrackingCampaignId, visitor->uid, detail->url);
					campaignKeyLength = snprintf(campaignKey,
							(((int)sizeof(campaignKey)) - 1),
							"%i",
							visitor->siteTrackingCampaignId);

					if(bptree_get(&dailyCampaigns,
								campaignKey,
								campaignKeyLength,
								(void *)&dailyCampaign) < 0) {
						dailyCampaign = (VgrStatsDailySummary *)malloc(
								sizeof(VgrStatsDailySummary));
						dailyCampaign->type = VGR_DAILY_SUMMARY_TYPE_CAMPAIGN;
						dailyCampaign->linkId = visitor->siteTrackingCampaignId;
						dailyCampaign->uniqueVisitors += 1;
						dailyCampaign->visits += 1;
						dailyCampaign->dateStamp = strdup(
								dailySummary->dateStamp);

						bptree_put(&dailyCampaigns,
								campaignKey,
								campaignKeyLength,
								(void *)dailyCampaign);
					}
					else {
						dailyCampaign->uniqueVisitors += 1;
						dailyCampaign->visits += 1;
					}
				}

				if(bptree_get(&context->countries,
							visitor->countryCode,
							strlen(visitor->countryCode),
							(void *)&countryLocale) < 0) {
					fprintf(stderr, "error - failed to locate country locale "
							"record '%s' at line %i on day %s.\n",
							visitor->countryCode,
							lineCounter,
							dailySummary->dateStamp);
				}
				else {
					dailyLocaleKeyLength = strlen(visitor->countryCode);

					if(bptree_get(&dailyLocales,
								visitor->countryCode,
								dailyLocaleKeyLength,
								(void *)&dailyLocale) < 0) {
						dailyLocale = (VgrStatsDailySummary *)malloc(
								sizeof(VgrStatsDailySummary));
						dailyLocale->type = VGR_DAILY_SUMMARY_TYPE_LOCALE;
						dailyLocale->linkId = countryLocale->uid;
						dailyLocale->uniqueVisitors += 1;
						dailyLocale->visits += 1;
						dailyLocale->dateStamp = strdup(
								dailySummary->dateStamp);

						bptree_put(&dailyLocales,
								visitor->countryCode,
								dailyLocaleKeyLength,
								(void *)dailyLocale);
					}
					else {
						dailyLocale->uniqueVisitors += 1;
						dailyLocale->visits += 1;
					}
				}

				detail_getSpiderUidFromVisitor(visitor);

				if(visitor->siteTrackingSpiderId > 0) {
//printf("\n[day: %i (%s)] NEW SPIDER #%03i VISITOR #%i => '%s' (%s)\n", dayCounter, dailySummary->dateStamp, visitor->siteTrackingSpiderId, visitor->uid, visitor->userAgent, detail->url);

					spiderKeyLength = snprintf(spiderKey,
							(((int)sizeof(spiderKey)) - 1),
							"%i",
							visitor->siteTrackingSpiderId);

					if(bptree_get(&dailySpiders,
								spiderKey,
								spiderKeyLength,
								(void *)&dailySpider) < 0) {
						dailySpider = (VgrStatsDailySummary *)malloc(
								sizeof(VgrStatsDailySummary));
						dailySpider->type = VGR_DAILY_SUMMARY_TYPE_SPIDER;
						dailySpider->linkId = visitor->siteTrackingSpiderId;
						dailySpider->uniqueVisitors += 1;
						dailySpider->visits += 1;
						dailySpider->dateStamp = strdup(
								dailySummary->dateStamp);

						bptree_put(&dailySpiders,
								spiderKey,
								spiderKeyLength,
								(void *)dailySpider);
					}
					else {
						dailySpider->uniqueVisitors += 1;
						dailySpider->visits += 1;
					}
				}

				if((visitor->platform != NULL) &&
						((dailyOSKeyLength = strlen(visitor->platform)) > 0)) {
					operatingSystem = NULL;

					// update the master operating system index

					if(bptree_get(&masterOSs, visitor->platform,
								dailyOSKeyLength,
								(void *)&operatingSystem) < 0) {
						operatingSystem = (VgrStatsOS *)malloc(
								sizeof(VgrStatsOS));
						operatingSystem->uid = masterOSUidCounter;
						operatingSystem->name = strdup(visitor->platform);

						writeLength = snprintf(buffer,
								(((int)sizeof(buffer)) - 1),
								"Operating System ''%s'' was auto-generated "
								"by the Villa Group statistics engine.",
								visitor->platform);

						operatingSystem->description = strndup(buffer,
								writeLength);

						bptree_put(&masterOSs, visitor->platform,
								dailyOSKeyLength, (void *)operatingSystem);

						masterOSUidCounter += 1;
					}

					if(bptree_get(&dailyOSs, visitor->platform,
								dailyOSKeyLength,
								(void *)&dailyOS) < 0) {
						dailyOS = (VgrStatsDailySummary *)malloc(
								sizeof(VgrStatsDailySummary));
						dailyOS->type = VGR_DAILY_SUMMARY_TYPE_OS;
						dailyOS->linkId = operatingSystem->uid;;
						dailyOS->uniqueVisitors += 1;
						dailyOS->visits += 1;
						dailyOS->dateStamp = strdup(
								dailySummary->dateStamp);

						bptree_put(&dailyOSs,
								visitor->platform,
								dailyOSKeyLength,
								(void *)dailyOS);
					}
					else {
						dailyOS->uniqueVisitors += 1;
						dailyOS->visits += 1;
					}
				}

				if((detail->referer != NULL) &&
						(strlen(detail->referer) > 0)) {
					for(ii = 0; SEARCH_ENGINE_LIST[ii].uid > 0; ii++) {
						if(strcasestr(detail->referer,
									SEARCH_ENGINE_LIST[ii].key)) {
							dailySearchEngineKeyLength =
								SEARCH_ENGINE_LIST[ii].keyLength;

							if(bptree_get(&dailySearchEngines,
										SEARCH_ENGINE_LIST[ii].key,
										dailySearchEngineKeyLength,
										(void *)&dailySearchEngine) < 0) {
								dailySearchEngine =
									(VgrStatsDailySummary *)malloc(
										sizeof(VgrStatsDailySummary));
								dailySearchEngine->type =
									VGR_DAILY_SUMMARY_TYPE_SEARCH_ENGINE;
								dailySearchEngine->linkId =
									SEARCH_ENGINE_LIST[ii].uid;
								dailySearchEngine->uniqueVisitors += 1;
								dailySearchEngine->visits += 1;
								dailySearchEngine->referer = strndup(
										SEARCH_ENGINE_LIST[ii].key,
										SEARCH_ENGINE_LIST[ii].keyLength);
								dailySearchEngine->dateStamp = strdup(
										dailySummary->dateStamp);

								bptree_put(&dailySearchEngines,
										SEARCH_ENGINE_LIST[ii].key,
										dailySearchEngineKeyLength,
										(void *)dailySearchEngine);
							}
							else {
								dailySearchEngine->uniqueVisitors += 1;
								dailySearchEngine->visits += 1;
							}

							break;
						}
					}
				}

				if(dailySearchEngine != NULL) {
					visitor->daily.searchEngine = strdup(
							dailySearchEngine->referer);
//printf("FOUND QUERY[%s] :: '%s'\n", dailySearchEngine->referer, detail->referer);

					if(decodeUrl(detail->referer,
								strlen(detail->referer),
								decodedUrl,
								&decodedUrlLength,
								&searchUri) == 0) {
						dailySearchFound = afalse;
						dailySearchLength = 0;
						dailySearchKey = NULL;

						for(ii = 0; ii < searchUri.uriArgCount; ii++) {
							if((searchUri.args[ii].uriArgNameLength < 1) ||
									(searchUri.args[ii].uriArgValueLength < 1)) {
								continue;
							}

							switch(dailySearchEngine->linkId) {
								case 1:		// google
								case 2:		// bing
								case 3:		// aol
								case 5:		// ask
								case 8:		// gigablast
									if(!strcasecmp(
												searchUri.args[ii].uriArgName,
												"q")) {
										dailySearchFound = atrue;
									}
									break;

								case 4:		// yahoo
									if(!strcasecmp(
												searchUri.args[ii].uriArgName,
												"p")) {
										dailySearchFound = atrue;
									}
									break;

								case 6:		// netscape
								case 7:		// snap
									if(!strcasecmp(
												searchUri.args[ii].uriArgName,
												"query")) {
										dailySearchFound = atrue;
									}
									break;
							}

							if(dailySearchFound) {
								if(searchUri.args[ii].uriArgValueLength > 1) {
									dailySearchKey = getNormalizedString(
											searchUri.args[ii].uriArgValue,
											&dailySearchLength);
								}
								else {
									dailySearchFound = afalse;
								}
								break;
							}
						}

						if((dailySearchKey != NULL) &&
								(dailySearchLength > 3)) {
							if(bptree_get(&dailySearches,
										dailySearchKey,
										dailySearchLength,
										(void *)&dailySearch) < 0) {
								dailySearch = (VgrStatsDailySummary *)malloc(
										sizeof(VgrStatsDailySummary));
								dailySearch->type =
									VGR_DAILY_SUMMARY_TYPE_SEARCH_ENGINE_PHRASE;
								dailySearch->linkId = 1;
								dailySearch->uniqueVisitors += 1;
								dailySearch->visits += 1;
								dailySearch->url = strndup(dailySearchKey,
										dailySearchLength);
								dailySearch->dateStamp = strdup(
										dailySummary->dateStamp);

								bptree_put(&dailySearches,
										dailySearchKey,
										dailySearchLength,
										(void *)dailySearch);
							}
							else {
								dailySearch->uniqueVisitors += 1;
								dailySearch->visits += 1;
							}
						}

						if(dailySearchKey != NULL) {
							free(dailySearchKey);
						}
					}

					if(dailySearch != NULL) {
						visitor->daily.searchPhrase = strdup(dailySearch->url);
					}
				} // (dailySearchEngine != NULL)

				dailyWebBrowserKeyLength = strlen(visitor->browserName);

				if(bptree_get(&dailyWebBrowsers,
							visitor->browserName,
							dailyWebBrowserKeyLength,
							(void *)&dailyWebBrowser) < 0) {
					dailyWebBrowser = (VgrStatsDailySummary *)malloc(
							sizeof(VgrStatsDailySummary));
					dailyWebBrowser->type = VGR_DAILY_SUMMARY_TYPE_WEB_BROWSER;
					dailyWebBrowser->linkId = visitor->browserId;
					dailyWebBrowser->uniqueVisitors += 1;
					dailyWebBrowser->visits += 1;
					dailyWebBrowser->dateStamp = strdup(
							dailySummary->dateStamp);

					bptree_put(&dailyWebBrowsers,
							visitor->browserName,
							dailyWebBrowserKeyLength,
							(void *)dailyWebBrowser);
				}
				else {
					dailyWebBrowser->uniqueVisitors += 1;
					dailyWebBrowser->visits += 1;
				}

				if(visitor->siteCityId > 0) {
					dailyCityKeyLength = snprintf(dailyCityKey,
							((int)sizeof(dailyCityKey) - 1),
							"%i",
							visitor->siteCityId);

					city = NULL;

					bptree_get(&context->citiesById,
								dailyCityKey,
								dailyCityKeyLength,
								(void *)&city);

					if((city != NULL) && 
							(bptree_get(&dailyCities,
										dailyCityKey,
										dailyCityKeyLength,
										(void *)&dailyCity) < 0)) {
						dailyCity = (VgrStatsDailySummary *)malloc(
								sizeof(VgrStatsDailySummary));
						dailyCity->type = VGR_DAILY_SUMMARY_TYPE_CITY;
						dailyCity->linkId = visitor->siteCityId;
						dailyCity->linkIdTwo = visitor->siteStateId;
						dailyCity->referer = strdup(city->name);
						dailyCity->uniqueVisitors += 1;
						dailyCity->visits += 1;
						dailyCity->dateStamp = strdup(
								dailySummary->dateStamp);

						bptree_put(&dailyCities,
								dailyCityKey,
								dailyCityKeyLength,
								(void *)dailyCity);
					}
					else if(dailyCity != NULL) {
						dailyCity->uniqueVisitors += 1;
						dailyCity->visits += 1;
					}
				}

				if(visitor->siteStateId > 0) {
					dailyStateKeyLength = snprintf(dailyStateKey,
							((int)sizeof(dailyStateKey) - 1),
							"%i",
							visitor->siteStateId);

					if(bptree_get(&dailyStates,
								dailyStateKey,
								dailyStateKeyLength,
								(void *)&dailyState) < 0) {
						dailyState = (VgrStatsDailySummary *)malloc(
								sizeof(VgrStatsDailySummary));
						dailyState->type = VGR_DAILY_SUMMARY_TYPE_STATE;
						dailyState->linkId = visitor->siteStateId;
						dailyState->uniqueVisitors += 1;
						dailyState->visits += 1;
						dailyState->dateStamp = strdup(
								dailySummary->dateStamp);

						bptree_put(&dailyStates,
								dailyStateKey,
								dailyStateKeyLength,
								(void *)dailyState);
					}
					else {
						dailyState->uniqueVisitors += 1;
						dailyState->visits += 1;
					}
				}

				if((detail->url != NULL) &&
						(strlen(detail->url) > 0) &&
						(strcasestr(detail->url, "/app/booking?ratePlan="))) {
					dailyRatePlanFound = afalse;

					if(decodeUrl(detail->url,
								strlen(detail->url),
								decodedUrl,
								&decodedUrlLength,
								&searchUri) == 0) {
						for(ii = 0; ii < searchUri.uriArgCount; ii++) {
							if((searchUri.args[ii].uriArgNameLength < 1) ||
									(searchUri.args[ii].uriArgValueLength < 1)) {
								continue;
							}

							if(!strcasecmp(searchUri.args[ii].uriArgName,
										"ratePlan")) {
								dailyRatePlanFound = atrue;
								dailyRatePlanId = atoi(
										searchUri.args[ii].uriArgValue);
								break;
							}
						}
					}

					if((dailyRatePlanFound) && (dailyRatePlanId > 0)) {
						visitor->daily.ratePlanId = dailyRatePlanId;

						dailyRatePlanKeyLength = snprintf(dailyRatePlanKey,
								((int)sizeof(dailyRatePlanKey) - 1),
								"%i",
								visitor->daily.ratePlanId);

						if(bptree_get(&dailyRatePlans,
									dailyRatePlanKey,
									dailyRatePlanKeyLength,
									(void *)&dailyRatePlan) < 0) {
							dailyRatePlan = (VgrStatsDailySummary *)malloc(
									sizeof(VgrStatsDailySummary));
							dailyRatePlan->type =
								VGR_DAILY_SUMMARY_TYPE_RATE_PLAN;
							dailyRatePlan->linkId = dailyRatePlanId;
							dailyRatePlan->uniqueVisitors += 1;
							dailyRatePlan->visits += 1;
							dailyRatePlan->dateStamp = strdup(
									dailySummary->dateStamp);

							bptree_put(&dailyRatePlans,
									dailyRatePlanKey,
									dailyRatePlanKeyLength,
									(void *)dailyRatePlan);
						}
						else {
							dailyRatePlan->uniqueVisitors += 1;
							dailyRatePlan->visits += 1;
						}
					}
				}

				if(dailyUrl != NULL) {
					dailyUrl->uniqueVisitors += 1;
					dailyUrl->visits += 1;
				}

				if(dailyReferer != NULL) {
					dailyReferer->uniqueVisitors += 1;
					dailyReferer->visits += 1;
				}

				if(dailyNormalizedReferer != NULL) {
					dailyNormalizedReferer->uniqueVisitors += 1;
					dailyNormalizedReferer->visits += 1;
				}

				if((dailyHourStamp >= 0) && (dailyHourStamp <= 23)) {
					hourlySummary[dailyHourStamp].uniqueVisitors += 1;
					hourlySummary[dailyHourStamp].visits += 1;
				}

				dailySummary->uniqueVisitors += 1;
				dailySummary->visits += 1;

				bptree_put(&dailyVisitors, detail->key, keyLength,
						(void *)visitor);
			}
		}
		else if(visitor != NULL) {
//printf("\t VISITOR #%i => time on site : %i seconds\n", visitor->uid, (dailySecondStamp - visitor->daily.dailySecondStamp));

			if(destinationId >= 0) {
				if(visitor->daily.destinationId < 0) {
					visitor->daily.destinationId = destinationId;
				}
			}
			else if(propertyId >= 0) {
				if(visitor->daily.propertyId < 0) {
					visitor->daily.propertyId = propertyId;
				}
			}

			if(visitor->siteTrackingCampaignId > 0) {
				campaignKeyLength = snprintf(campaignKey,
						(((int)sizeof(campaignKey)) - 1),
						"%i",
						visitor->siteTrackingCampaignId);

				bptree_get(&dailyCampaigns, campaignKey, campaignKeyLength,
						(void *)&dailyCampaign);
			}

			dailyLocaleKeyLength = strlen(visitor->countryCode);

			bptree_get(&dailyLocales,
					visitor->countryCode,
					dailyLocaleKeyLength,
					(void *)&dailyLocale);

			if(visitor->siteTrackingSpiderId > 0) {
				spiderKeyLength = snprintf(spiderKey,
						(((int)sizeof(spiderKey)) - 1),
						"%i",
						visitor->siteTrackingSpiderId);

				bptree_get(&dailySpiders, spiderKey, spiderKeyLength,
						(void *)&dailySpider);
			}

			if((visitor->platform != NULL) &&
					((dailyOSKeyLength = strlen(visitor->platform)) > 0)) {
				operatingSystem = NULL;

				if((bptree_get(&masterOSs,
								visitor->platform,
								dailyOSKeyLength,
								(void *)&operatingSystem) == 0) &&
						(operatingSystem != NULL)) {
					bptree_get(&dailyOSs,
							visitor->platform,
							dailyOSKeyLength,
							(void *)&dailyOS);
				}
			}

			if((detail->referer != NULL) && (strlen(detail->referer) > 0)) {
				for(ii = 0; SEARCH_ENGINE_LIST[ii].uid > 0; ii++) {
					if(strcasestr(detail->referer,
								SEARCH_ENGINE_LIST[ii].key)) {
						dailySearchEngineKeyLength =
							SEARCH_ENGINE_LIST[ii].keyLength;

						bptree_get(&dailySearchEngines,
								SEARCH_ENGINE_LIST[ii].key,
								dailySearchEngineKeyLength,
								(void *)&dailySearchEngine);
	
						break;
					}
				}
			}

			if((dailySearchEngine != NULL) &&
					(visitor->daily.searchEngine == NULL)) {
				visitor->daily.searchEngine = strdup(
						dailySearchEngine->referer);

				if(decodeUrl(detail->referer,
							strlen(detail->referer),
							decodedUrl,
							&decodedUrlLength,
							&searchUri) == 0) {
					dailySearchFound = afalse;
					dailySearchLength = 0;
					dailySearchKey = NULL;

					for(ii = 0; ii < searchUri.uriArgCount; ii++) {
						if((searchUri.args[ii].uriArgNameLength < 1) ||
								(searchUri.args[ii].uriArgValueLength < 1)) {
							continue;
						}

						switch(dailySearchEngine->linkId) {
							case 1:		// google
							case 2:		// bing
							case 3:		// aol
							case 5:		// ask
							case 8:		// gigablast
								if(!strcasecmp(
											searchUri.args[ii].uriArgName,
											"q")) {
									dailySearchFound = atrue;
								}
								break;

							case 4:		// yahoo
								if(!strcasecmp(
											searchUri.args[ii].uriArgName,
											"p")) {
									dailySearchFound = atrue;
								}
								break;

							case 6:		// netscape
							case 7:		// snap
								if(!strcasecmp(
											searchUri.args[ii].uriArgName,
											"query")) {
									dailySearchFound = atrue;
								}
								break;
						}

						if(dailySearchFound) {
							if(searchUri.args[ii].uriArgValueLength > 1) {
								dailySearchKey = getNormalizedString(
										searchUri.args[ii].uriArgValue,
										&dailySearchLength);
							}
							else {
								dailySearchFound = afalse;
							}
							break;
						}
					}

					if((dailySearchKey != NULL) && (dailySearchLength > 3)) {
						bptree_get(&dailySearches,
									dailySearchKey,
									dailySearchLength,
									(void *)&dailySearch);
					}

					if(dailySearchKey != NULL) {
						free(dailySearchKey);
					}

					if(dailySearch != NULL) {
						visitor->daily.searchPhrase = strdup(dailySearch->url);
					}
				}
			}

			dailyWebBrowserKeyLength = strlen(visitor->browserName);

			bptree_get(&dailyWebBrowsers,
					visitor->browserName,
					dailyWebBrowserKeyLength,
					(void *)&dailyWebBrowser);

			if(visitor->siteCityId > 0) {
				dailyCityKeyLength = snprintf(dailyCityKey,
						((int)sizeof(dailyCityKey) - 1),
						"%i",
						visitor->siteCityId);

				bptree_get(&dailyCities, dailyCityKey, dailyCityKeyLength,
						(void *)&dailyCity);
			}

			if(visitor->siteStateId > 0) {
				dailyStateKeyLength = snprintf(dailyStateKey,
						((int)sizeof(dailyStateKey) - 1),
						"%i",
						visitor->siteStateId);

				bptree_get(&dailyStates, dailyStateKey, dailyStateKeyLength,
						(void *)&dailyState);
			}

			if((detail->url != NULL) &&
					(strlen(detail->url) > 0) &&
					(strcasestr(detail->url, "/app/booking?ratePlan="))) {
				dailyRatePlanFound = afalse;

				if(decodeUrl(detail->url,
							strlen(detail->url),
							decodedUrl,
							&decodedUrlLength,
							&searchUri) == 0) {
					for(ii = 0; ii < searchUri.uriArgCount; ii++) {
						if((searchUri.args[ii].uriArgNameLength < 1) ||
								(searchUri.args[ii].uriArgValueLength < 1)) {
							continue;
						}

						if(!strcasecmp(searchUri.args[ii].uriArgName,
									"ratePlan")) {
							dailyRatePlanFound = atrue;
							dailyRatePlanId = atoi(
									searchUri.args[ii].uriArgValue);
							break;
						}
					}
				}

				if((dailyRatePlanFound) && (dailyRatePlanId > 0)) {
					dailyRatePlanKeyLength = snprintf(dailyRatePlanKey,
							((int)sizeof(dailyRatePlanKey) - 1),
							"%i",
							visitor->daily.ratePlanId);

					bptree_get(&dailyRatePlans,
							dailyRatePlanKey,
							dailyRatePlanKeyLength,
							(void *)&dailyRatePlan);
				}
			}
		}

		// update campaign page count

		if(dailyCampaign != NULL) {
			dailyCampaign->pageCount += 1;
		}

		// update locale page count

		if(dailyLocale != NULL) {
			dailyLocale->pageCount += 1;
		}

		// update spider page count

		if(dailySpider != NULL) {
			dailySpider->pageCount += 1;
		}

		// update daily os page count

		if(dailyOS != NULL) {
			dailyOS->pageCount += 1;
		}

		// update daily search engine page count

		if((dailySearchEngine == NULL) && (visitor != NULL) &&
				(visitor->daily.searchEngine != NULL)) {
			bptree_get(&dailySearchEngines,
					visitor->daily.searchEngine,
					strlen(visitor->daily.searchEngine),
					(void *)&dailySearchEngine);
		}

		if(dailySearchEngine != NULL) {
			dailySearchEngine->pageCount += 1;
		}

		if((dailySearch == NULL) && (visitor != NULL) &&
				(visitor->daily.searchPhrase != NULL)) {
			bptree_get(&dailySearches,
					visitor->daily.searchPhrase,
					strlen(visitor->daily.searchPhrase),
					(void *)&dailySearch);
		}

		if(dailySearch != NULL) {
			dailySearch->pageCount += 1;
		}

		// update daily web browser page count

		if(dailyWebBrowser != NULL) {
			dailyWebBrowser->pageCount += 1;
		}

		// update daily city page count

		if(dailyCity != NULL) {
			dailyCity->pageCount += 1;
		}

		// update daily state page count

		if(dailyState != NULL) {
			dailyState->pageCount += 1;
		}

		// update daily rate plan page count

		if(dailyRatePlan != NULL) {
			dailyRatePlan->pageCount += 1;
		}

		// update hourly summary page count

		if((dailyHourStamp >= 0) && (dailyHourStamp <= 23)) {
			hourlySummary[dailyHourStamp].pageCount += 1;
		}

		// update visitor-based metrics

		if(visitor != NULL) {
			if((dailyHourStamp >= 0) && (dailyHourStamp <= 23)) {
				visitor->daily.dailyHourStamp = dailyHourStamp;
			}

			// determine if this is a new visit

			if((dailySecondStamp - visitor->daily.dailySecondStamp) >
					VISITOR_TIMEOUT_SECONDS) {
				visitor->daily.dailySecondStamp = dailySecondStamp;

				if((visitor->siteTrackingCampaignId > 0) &&
						(dailyCampaign != NULL)) {
					dailyCampaign->visits += 1;
				}
				if(dailyLocale != NULL) {
					dailyLocale->visits += 1;
				}
				if(dailySpider != NULL) {
					dailySpider->visits += 1;
				}
				if(dailyUrl != NULL) {
					dailyUrl->visits += 1;
				}
				if(dailyReferer != NULL) {
					dailyReferer->visits += 1;
				}
				if(dailyNormalizedReferer != NULL) {
					dailyNormalizedReferer->visits += 1;
				}
				if(dailyOS != NULL) {
					dailyOS->visits += 1;
				}
				if(dailySearchEngine != NULL) {
					dailySearchEngine->visits += 1;
				}
				if(dailySearch != NULL) {
					dailySearch->visits += 1;
				}
				if(dailyWebBrowser != NULL) {
					dailyWebBrowser->visits += 1;
				}
				if(dailyCity != NULL) {
					dailyCity->visits += 1;
				}
				if(dailyState != NULL) {
					dailyState->visits += 1;
				}
				if(dailyRatePlan != NULL) {
					dailyRatePlan->visits += 1;
				}

				if((dailyHourStamp >= 0) && (dailyHourStamp <= 23)) {
					hourlySummary[dailyHourStamp].visits += 1;
				}

				if(destinationId >= 0) {
					destinations[destinationId].visits += 1;

					if(visitor->daily.destinationId < 0) {
						visitor->daily.destinationId = destinationId;
					}
				}
				else if(propertyId >= 0) {
					properties[propertyId].visits += 1;

					if(visitor->daily.propertyId < 0) {
						visitor->daily.propertyId = propertyId;
					}
				}

				dailySummary->visits += 1;
			}

			// determine if this is a conversion

			if((!visitor->daily.isConverted) &&
					((strcasestr(detail->url, "/app/booking")) ||
					 (strcasestr(detail->url, "/app/promo")) ||
					 (strcasestr(detail->url, "/offers")))) {
				visitor->daily.isConverted = atrue;

				if((visitor->siteTrackingCampaignId > 0) &&
						(dailyCampaign != NULL)) {
					dailyCampaign->conversions += 1;
				}
				if(dailyLocale != NULL) {
					dailyLocale->conversions += 1;
				}
				if(dailySpider != NULL) {
					dailySpider->conversions += 1;
				}
				if(dailyReferer != NULL) {
					dailyReferer->conversions += 1;
				}
				if(dailyNormalizedReferer != NULL) {
					dailyNormalizedReferer->conversions += 1;
				}
				if(dailyOS != NULL) {
					dailyOS->conversions += 1;
				}
				if(dailySearchEngine != NULL) {
					dailySearchEngine->conversions += 1;
				}
				if(dailySearch != NULL) {
					dailySearch->conversions += 1;
				}
				if(dailyWebBrowser != NULL) {
					dailyWebBrowser->conversions += 1;
				}
				if(dailyCity != NULL) {
					dailyCity->conversions += 1;
				}
				if(dailyState != NULL) {
					dailyState->conversions += 1;
				}
				if(dailyRatePlan != NULL) {
					dailyRatePlan->conversions += 1;
				}
				if((dailyHourStamp >= 0) && (dailyHourStamp <= 23)) {
					hourlySummary[dailyHourStamp].conversions += 1;
				}

				if((detail->referer != NULL) && (strlen(detail->referer) > 0)) {
					if(strcasestr(detail->referer,
								"/destinations/cabo-san-lucas")) {
						destinations[VGR_DESTINATION_CABO].conversions += 1;
					}
					else if(strcasestr(detail->referer,
								"/destinations/cancun")) {
						destinations[VGR_DESTINATION_CANCUN].conversions += 1;
					}
					else if(strcasestr(detail->referer,
								"/destinations/loreto")) {
						destinations[VGR_DESTINATION_LORETO].conversions += 1;
					}
					else if(strcasestr(detail->referer,
								"/destinations/puerto-vallarta")) {
						destinations[VGR_DESTINATION_PUERTO_VALLARTA].conversions += 1;
					}
					else if(strcasestr(detail->referer,
								"/destinations/riviera-nayarit")) {
						destinations[VGR_DESTINATION_NUEVO_VALLARTA].conversions += 1;
					}
					else if(strcasestr(detail->referer,
								"/resorts/villa-la-estancia-cabo")) {
						properties[VGR_PROPERTY_VLE_CABO].conversions += 1;
					}
					else if(strcasestr(detail->referer,
								"/resorts/villa-del-palmar-cabo")) {
						properties[VGR_PROPERTY_VDP_CABO].conversions += 1;
					}
					else if(strcasestr(detail->referer,
								"/resorts/villa-del-arco")) {
						properties[VGR_PROPERTY_VILLA_DEL_ARCO].conversions += 1;
					}
					else if(strcasestr(detail->referer,
								"/resorts/villa-del-palmar-loreto")) {
						properties[VGR_PROPERTY_VDP_LORETO].conversions += 1;
					}
					else if(strcasestr(detail->referer,
								"/resorts/villa-del-palmar-puerto-vallarta")) {
						properties[VGR_PROPERTY_VDP_PUERTO_VALLARTA].conversions += 1;
					}
					else if(strcasestr(detail->referer,
								"/resorts/villa-la-estancia-riviera-nayarit")) {
						properties[VGR_PROPERTY_VLE_NAYARIT].conversions += 1;
					}
					else if(strcasestr(detail->referer,
								"/resorts/villa-del-palmar-flamingos")) {
						properties[VGR_PROPERTY_FLAMINGOS].conversions += 1;
					}
					else if(strcasestr(detail->referer,
									"/resorts/villa-del-palmar-cancun")) {
						properties[VGR_PROPERTY_VDP_CANCUN].conversions += 1;
					}
				}

				dailySummary->conversions += 1;

				context->trackingOverview.conversions += 1;
			}

			// update visitor record

			if((strcasestr(detail->url, "/admin") == NULL) &&
					(strcasestr(detail->url, "/web-services") == NULL) &&
					(strcasestr(detail->url, ".png") == NULL) &&
					(strcasestr(detail->url, ".jpg") == NULL) &&
					(strcasestr(detail->url, ".gif") == NULL) &&
					(strcasestr(detail->url, ".js") == NULL) &&
					(strcasestr(detail->url, ".txt") == NULL) &&
					(strcasestr(detail->url, ".pdf") == NULL) &&
					(strcasestr(detail->url, ".css") == NULL) &&
					(strcasestr(detail->url, ".style") == NULL) &&
					(strcasestr(detail->url, ".defaultview") == NULL) &&
					(strcasestr(detail->url, ".php") == NULL) &&
					(strcasestr(detail->url, ".swf") == NULL) &&
					(strcasestr(detail->url, ".ico") == NULL) &&
					(strcasestr(detail->url, "this.") == NULL) &&
					(strcasestr(detail->url, "/http:") == NULL)) {
				if(visitor->daily.exitUrl != NULL) {
					if(!strcasecmp(visitor->daily.exitUrl,
								detail->normalizedUrl)) {
						visitor->daily.exitUrlCounter = 1;
					}
					else {
						free(visitor->daily.exitUrl);
						visitor->daily.exitUrl = strdup(detail->normalizedUrl);
					}
				}
				else {
					visitor->daily.exitUrl = strdup(detail->normalizedUrl);
				}
			}

			visitor->daily.pageCount += 1;

			if(strcasestr(detail->url, "/admin") == NULL) {
				if((dailySecondStamp - visitor->daily.trackingTimestamp) <
						VISITOR_PAGE_TIMEOUT_SECONDS) {
					visitor->daily.timeOnSite += (dailySecondStamp -
							visitor->daily.trackingTimestamp);
//printf("VISITOR :: %03i :: PAGE :: '%s' :: Visitor at @ %i, Page @ %i, Time-on-site: %i\n", visitor->uid, detail->url, visitor->daily.trackingTimestamp, dailySecondStamp, visitor->daily.timeOnSite);
				}
			}

			visitor->daily.trackingTimestamp = dailySecondStamp;
		}

/* ------------------------------------------------------------------------- */
		if(dayCounter != lastDayCounter) {
			for(ii = 0; ii < displayLength; ii++) {
				printf("\b");
			}

			fflush(stdout);

			displayLength = snprintf(displayBuffer,
					(((int)sizeof(displayBuffer)) - 1),
					"%i days (%s)",
					dayCounter, dailySummary->dateStamp);

			printf("%s", displayBuffer);
			fflush(stdout);

			lastDayCounter = dayCounter;
		}
/* ------------------------------------------------------------------------- */

		detail_free(detail);

//		if(dayCounter > 8) {
//		if(dayCounter > 32) {
//		if(dayCounter > 64) {
//		if(dayCounter > 128) {
//		if(dayCounter > 256) {
//			break;
//		}

//		if(lineCounter > 8) {
//		if(lineCounter > 32) {
//		if(lineCounter > 1024) {
//			break;
//		}

		lineCounter++;

		rc = file_readLine(&fh, buffer, sizeof(buffer), &readLength);
	}

	if(rc < 0) {
		printf("FAILED at line %i, aborting.\n", __LINE__);
		return -1;
	}

	if((rc = file_free(&fh)) < 0) {
		printf("FAILED at line %i, aborting.\n", __LINE__);
		return -1;
	}

	// sort the master operating system index & create the SQL

	bptree_toArray(&masterOSs,
			&indexArrayLength,
			&indexKeyLengths,
			&indexKeys,
			&indexValues);

	qsort((void *)indexValues,
			indexArrayLength,
			(int)sizeof(VgrStatsDailySummary *),
			detail_ascendingVgrStatsOsComparison);

	if((rc = file_init(&fh, "siteTrackingOperatingSystem.sql",
					"w", 8192)) < 0) {
		printf("FAILED at line %i, aborting.\n", __LINE__);
		return -1;
	}

	writeLength = snprintf(buffer, (((int)sizeof(buffer)) - 1),
			"drop table if exists siteTrackingOperatingSystem;\n"
			"create table siteTrackingOperatingSystem\n"
			"(\n"
			"	uid						serial,\n"
			"	name					varchar(128) not null,\n"
			"	description				varchar(1024) not null default '',\n"
			"	isEnabled				char(1) not null default '1',\n"
			"	url						varchar(65536) not null default '',\n"
			"	dateCreated				timestamp not null default now(),\n"
			"	dateModified			timestamp not null default now()\n"
			");\n"
			"\n"
			"create index siteTrackingOperatingSystemUidIndex on siteTrackingOperatingSystem(uid);\n"
			"\n"
			"");

	file_write(&fh, buffer, writeLength);

	for(ii = 0; ii < indexArrayLength; ii++) {
		writeLength = snprintf(buffer, (((int)sizeof(buffer)) - 1),
				"insert into siteTrackingOperatingSystem ("
				"name, "
				"description) "
				"values ("
				"'%s', '%s'"
				");\n",
				((VgrStatsOS **)indexValues)[ii]->name,
				((VgrStatsOS **)indexValues)[ii]->description);

//		printf("\n\n%s\n\n", buffer);

		file_write(&fh, buffer, writeLength);
	}

	if(indexKeys != NULL) {
		for(ii = 0; ii < indexArrayLength; ii++) {
			if(indexKeys[ii] != NULL) {
				free(indexKeys[ii]);
			}
		}

		free(indexKeys);
	}

	if(indexValues != NULL) {
		free(indexValues);
	}

	// write the hourly summaries

	for(ii = 0; ii < 24; ii++) {
		writeLength = snprintf(buffer, (((int)sizeof(buffer)) - 1),
				"insert into siteTrackingHourlySummary ("
				"hour, "
				"uniqueVisitors, "
				"visits, "
				"pageCount, "
				"conversionCount, "
				"bounceCount, "
				"totalTimeOnSite "
				") "
				"values ("
				"'%i', '%i', '%i', '%i', '%i', '%i', '%i'"
				");\n",
				ii,
				hourlySummary[ii].uniqueVisitors,
				hourlySummary[ii].visits,
				hourlySummary[ii].pageCount,
				hourlySummary[ii].conversions,
				hourlySummary[ii].bounceCount,
				hourlySummary[ii].totalTimeOnSite);

//		printf("\n\n%s\n\n", buffer);

		file_write(&sthourly, buffer, writeLength);
	}

	// create the SQL for the site visitors

	if(PERFORM_GEOIP_LOOKUP) {
		for(ii = 0; ii < context->visitorListLength; ii++) {
			visitorPlatform = sqlStrdup(
					context->visitorList[ii]->platform);
			visitorUserAgent = sqlStrdup(
					context->visitorList[ii]->userAgent);
			visitorBrowserName = sqlStrdup(
					context->visitorList[ii]->browserName);
			visitorClientIp = sqlStrdup(
					context->visitorList[ii]->clientIp);
			visitorCountryCode = sqlStrdup(
					context->visitorList[ii]->countryCode);
			visitorCountryName = sqlStrdup(
					context->visitorList[ii]->countryName);
			visitorDateCreated = sqlStrdup(
					context->visitorList[ii]->dateCreated);
			visitorDateModified = sqlStrdup(
					context->visitorList[ii]->dateModified);

			writeLength = snprintf(buffer, (((int)sizeof(buffer)) - 1),
					"insert into siteTrackingVisitor ("
					"	uid, "
					"	siteTrackingCampaignId, "
					"	siteUserId, "
					"	visits, "
					"	hash, "
					"	platform, "
					"	userAgent, "
					"	clientIp, "
					"	countryCode, "
					"	countryName, "
					"	siteStateId, "
					"	siteCityId, "
					"	dateCreated, "
					"	dateModified "
					") "
					"values ("
					"'%i', "
					"'%i', "
					"'%i', "
					"'%i', "
					"'%s', "
					"'%s', "
					"'%s', "
					"'%s', "
					"'%s', "
					"'%s', "
					"'%i', "
					"'%i', "
					"'%s', "
					"'%s' "
					");\n",
					context->visitorList[ii]->uid,
					context->visitorList[ii]->siteTrackingCampaignId,
					context->visitorList[ii]->siteUserId,
					context->visitorList[ii]->visits,
					context->visitorList[ii]->hash,
					visitorPlatform,
					visitorUserAgent,
					visitorClientIp,
					visitorCountryCode,
					visitorCountryName,
					context->visitorList[ii]->siteStateId,
					context->visitorList[ii]->siteCityId,
					visitorDateCreated,
					visitorDateModified);

			file_write(&stvisitor, buffer, writeLength);

			free(visitorPlatform);
			free(visitorUserAgent);
			free(visitorBrowserName);
			free(visitorClientIp);
			free(visitorCountryCode);
			free(visitorCountryName);
			free(visitorDateCreated);
			free(visitorDateModified);
		}
	}

	// finish up

	if((rc = file_free(&fh)) < 0) {
		printf("FAILED at line %i, aborting.\n", __LINE__);
		return -1;
	}

	bptree_free(&masterOSs);

	// write the SQL table indexing commands

	if(REBUILD_TABLES) {
		if(PERFORM_GEOIP_LOOKUP) {
			writeLength = snprintf(buffer, (((int)sizeof(buffer)) - 1),
					"\n"
					"create index siteTrackingVisitorUidIndex on siteTrackingVisitor(uid);\n"
					"\n"
					"");

			file_write(&stvisitor, buffer, writeLength);
		}

		writeLength = snprintf(buffer, (((int)sizeof(buffer)) - 1),
				"\n"
				"\n"
				"create index siteTrackingDailySummaryUidIndex on siteTrackingDailySummary(uid);\n"
				"create index siteTrackingDailySummarySummaryDateIndex on siteTrackingDailySummary(summaryDate);\n"
				"\n"
				"");

		file_write(&stds, buffer, writeLength);

		writeLength = snprintf(buffer, (((int)sizeof(buffer)) - 1),
				"\n"
				"\n"
				"create index siteTrackingDestinationSummaryUidIndex on siteTrackingDestinationSummary(uid);\n"
					"create index siteTrackingDestinationSummarySummaryDateIndex on siteTrackingDestinationSummary(summaryDate);\n"
				"\n"
				"");

		file_write(&stdest, buffer, writeLength);

		writeLength = snprintf(buffer, (((int)sizeof(buffer)) - 1),
				"\n"
				"\n"
				"create index siteTrackingPropertySummaryUidIndex on siteTrackingPropertySummary(uid);\n"
				"create index siteTrackingPropertySummarySummaryDateIndex on siteTrackingPropertySummary(summaryDate);\n"
				"\n"
				"");

		file_write(&stprop, buffer, writeLength);

		writeLength = snprintf(buffer, (((int)sizeof(buffer)) - 1),
				"\n"
				"\n"
				"create index siteTrackingLocaleSummaryUidIndex on siteTrackingLocaleSummary(uid);\n"
				"create index siteTrackingLocaleSummarySummaryDateIndex on siteTrackingLocaleSummary(summaryDate);\n"
				"\n"
				"");

		file_write(&context->stlocale, buffer, writeLength);

		writeLength = snprintf(buffer, (((int)sizeof(buffer)) - 1),
				"\n"
				"\n"
				"create index siteTrackingSpiderSummaryUidIndex on siteTrackingSpiderSummary(uid);\n"
				"create index siteTrackingSpiderSummarySummaryDateIndex on siteTrackingSpiderSummary(summaryDate);\n"
				"\n"
				"");

		file_write(&context->stspider, buffer, writeLength);

		writeLength = snprintf(buffer, (((int)sizeof(buffer)) - 1),
				"\n"
				"\n"
				"create index siteTrackingUrlSummaryUidIndex on siteTrackingUrlSummary(uid);\n"
				"create index siteTrackingUrlSummarySummaryDateIndex on siteTrackingUrlSummary(summaryDate);\n"
				"\n"
				"");

		file_write(&context->sturl, buffer, writeLength);

		writeLength = snprintf(buffer, (((int)sizeof(buffer)) - 1),
				"\n"
				"\n"
				"create index siteTrackingRefererSummaryUidIndex on siteTrackingRefererSummary(uid);\n"
				"create index siteTrackingRefererSummarySummaryDateIndex on siteTrackingRefererSummary(summaryDate);\n"
				"\n"
				"");

		file_write(&context->streferer, buffer, writeLength);

		writeLength = snprintf(buffer, (((int)sizeof(buffer)) - 1),
				"\n"
				"\n"
				"create index siteTrackingDomainSummaryUidIndex on siteTrackingDomainSummary(uid);\n"
				"create index siteTrackingDomainSummarySummaryDateIndex on siteTrackingDomainSummary(summaryDate);\n"
				"\n"
				"");

		file_write(&context->stdomain, buffer, writeLength);

		writeLength = snprintf(buffer, (((int)sizeof(buffer)) - 1),
				"\n"
				"\n"
				"create index siteTrackingExitUrlSummaryUidIndex on siteTrackingExitUrlSummary(uid);\n"
				"create index siteTrackingExitUrlSummarySummaryDateIndex on siteTrackingExitUrlSummary(summaryDate);\n"
				"\n"
				"");

		file_write(&context->stexiturl, buffer, writeLength);

		writeLength = snprintf(buffer, (((int)sizeof(buffer)) - 1),
				"\n"
				"\n"
				"create index siteTrackingVisitorToRefererUidIndex on siteTrackingVisitorToReferer(uid);\n"
				"create index siteTrackingVTRSTVIIndex on siteTrackingVisitorToReferer(siteTrackingVisitorId);\n"
				"\n"
				"");

		file_write(&context->stvtr, buffer, writeLength);

		writeLength = snprintf(buffer, (((int)sizeof(buffer)) - 1),
				"\n"
				"\n"
				"create index siteTrackingOperatingSystemSummaryUidIndex on siteTrackingOperatingSystemSummary(uid);\n"
				"create index siteTrackingOperatingSystemSummarySummaryDateIndex on siteTrackingOperatingSystemSummary(summaryDate);\n"
				"\n"
				"");

		file_write(&context->stos, buffer, writeLength);

		writeLength = snprintf(buffer, (((int)sizeof(buffer)) - 1),
				"\n"
				"\n"
				"create index siteTrackingSearchEngineSummaryUidIndex on siteTrackingSearchEngineSummary(uid);\n"
				"create index siteTrackingSearchEngineSummarySummaryDateIndex on siteTrackingSearchEngineSummary(summaryDate);\n"
				"\n"
				"");

		file_write(&context->stse, buffer, writeLength);

		writeLength = snprintf(buffer, (((int)sizeof(buffer)) - 1),
				"\n"
				"\n"
				"create index siteTrackingSearchEnginePhraseSummaryUidIndex on siteTrackingSearchEnginePhraseSummary(uid);\n"
				"create index siteTrackingSearchEnginePhraseSummarySummaryDateIndex on siteTrackingSearchEnginePhraseSummary(summaryDate);\n"
				"\n"
				"");

		file_write(&context->stsp, buffer, writeLength);

		writeLength = snprintf(buffer, (((int)sizeof(buffer)) - 1),
				"\n"
				"\n"
				"create index siteTrackingSearchEngineKeywordSummaryUidIndex on siteTrackingSearchEngineKeywordSummary(uid);\n"
				"create index siteTrackingSearchEngineKeywordSummarySummaryDateIndex on siteTrackingSearchEngineKeywordSummary(summaryDate);\n"
				"\n"
				"");

		file_write(&context->stsk, buffer, writeLength);

		writeLength = snprintf(buffer, (((int)sizeof(buffer)) - 1),
				"\n"
				"\n"
				"create index siteTrackingWebBrowserSummaryUidIndex on siteTrackingWebBrowserSummary(uid);\n"
				"create index siteTrackingWebBrowserSummarySummaryDateIndex on siteTrackingWebBrowserSummary(summaryDate);\n"
				"\n"
				"");

		file_write(&context->stwb, buffer, writeLength);

		writeLength = snprintf(buffer, (((int)sizeof(buffer)) - 1),
				"\n"
				"\n"
				"create index siteTrackingCitySummaryUidIndex on siteTrackingCitySummary(uid);\n"
				"create index siteTrackingCitySummarySummaryDateIndex on siteTrackingCitySummary(summaryDate);\n"
				"\n"
				"");

		file_write(&context->stcity, buffer, writeLength);

		writeLength = snprintf(buffer, (((int)sizeof(buffer)) - 1),
				"\n"
				"\n"
				"create index siteTrackingStateSummaryUidIndex on siteTrackingStateSummary(uid);\n"
				"create index siteTrackingStateSummarySummaryDateIndex on siteTrackingStateSummary(summaryDate);\n"
				"\n"
				"");

		file_write(&context->ststate, buffer, writeLength);

		writeLength = snprintf(buffer, (((int)sizeof(buffer)) - 1),
				"\n"
				"\n"
				"create index siteTrackingRatePlanSummaryUidIndex on siteTrackingRatePlanSummary(uid);\n"
				"create index siteTrackingRatePlanSummarySummaryDateIndex on siteTrackingRatePlanSummary(summaryDate);\n"
				"\n"
				"");

		file_write(&context->strp, buffer, writeLength);

		writeLength = snprintf(buffer, (((int)sizeof(buffer)) - 1),
				"\n"
				"\n"
				"create index siteTrackingHourlySummaryUidIndex on siteTrackingHourlySummary(uid);\n"
				"create index siteTrackingHourlySummarySummaryDateIndex on siteTrackingHourlySummary(summaryDate);\n"
				"\n"
				"");

		file_write(&sthourly, buffer, writeLength);
	}

	if(PERFORM_GEOIP_LOOKUP) {
		if((rc = file_free(&stvisitor)) < 0) {
			printf("FAILED at line %i, aborting.\n", __LINE__);
			return -1;
		}
	}

	if((rc = file_free(&stds)) < 0) {
		printf("FAILED at line %i, aborting.\n", __LINE__);
		return -1;
	}

	if((rc = file_free(&stdest)) < 0) {
		printf("FAILED at line %i, aborting.\n", __LINE__);
		return -1;
	}

	if((rc = file_free(&stprop)) < 0) {
		printf("FAILED at line %i, aborting.\n", __LINE__);
		return -1;
	}

	if((rc = file_free(&context->stlocale)) < 0) {
		printf("FAILED at line %i, aborting.\n", __LINE__);
		return -1;
	}

	if((rc = file_free(&context->stspider)) < 0) {
		printf("FAILED at line %i, aborting.\n", __LINE__);
		return -1;
	}

	if((rc = file_free(&context->sturl)) < 0) {
		printf("FAILED at line %i, aborting.\n", __LINE__);
		return -1;
	}

	if((rc = file_free(&context->streferer)) < 0) {
		printf("FAILED at line %i, aborting.\n", __LINE__);
		return -1;
	}

	if((rc = file_free(&context->stdomain)) < 0) {
		printf("FAILED at line %i, aborting.\n", __LINE__);
		return -1;
	}

	if((rc = file_free(&context->stexiturl)) < 0) {
		printf("FAILED at line %i, aborting.\n", __LINE__);
		return -1;
	}

	if((rc = file_free(&context->stvtr)) < 0) {
		printf("FAILED at line %i, aborting.\n", __LINE__);
		return -1;
	}

	if((rc = file_free(&context->stos)) < 0) {
		printf("FAILED at line %i, aborting.\n", __LINE__);
		return -1;
	}

	if((rc = file_free(&context->stse)) < 0) {
		printf("FAILED at line %i, aborting.\n", __LINE__);
		return -1;
	}

	if((rc = file_free(&context->stsp)) < 0) {
		printf("FAILED at line %i, aborting.\n", __LINE__);
		return -1;
	}

	if((rc = file_free(&context->stsk)) < 0) {
		printf("FAILED at line %i, aborting.\n", __LINE__);
		return -1;
	}

	if((rc = file_free(&context->stwb)) < 0) {
		printf("FAILED at line %i, aborting.\n", __LINE__);
		return -1;
	}

	if((rc = file_free(&context->stcity)) < 0) {
		printf("FAILED at line %i, aborting.\n", __LINE__);
		return -1;
	}

	if((rc = file_free(&context->ststate)) < 0) {
		printf("FAILED at line %i, aborting.\n", __LINE__);
		return -1;
	}

	if((rc = file_free(&context->strp)) < 0) {
		printf("FAILED at line %i, aborting.\n", __LINE__);
		return -1;
	}

	if((rc = file_free(&sthourly)) < 0) {
		printf("FAILED at line %i, aborting.\n", __LINE__);
		return -1;
	}

	for(ii = 0; ii < displayLength; ii++) {
		printf("\b");
	}

	fflush(stdout);

	printf("OK                     \n");

	elapsedTime = time_getElapsedMusInSeconds(timer);

	printf("\t ...parsed %i lines in %0.6f seconds. (%0.6f seconds per line)\n",
			lineCounter,
			elapsedTime,
			(elapsedTime / (double)lineCounter));

//	context->visitorListLength = visitorRef;
//	context->visitorList = visitors;

	return 0;
}

// tracking functions

static void tracking_displayOverview()
{
	printf("Tracking Overview ::\n");
	printf("\t unique visitors    : %i\n",
			context->trackingOverview.uniqueVisitors);
	printf("\t visits             : %i\n",
			context->trackingOverview.visits);
	printf("\t page count         : %i\n",
			context->trackingOverview.pageCount);
	printf("\t conversions        : %i\n",
			context->trackingOverview.conversions);
}

