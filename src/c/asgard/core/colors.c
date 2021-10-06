/*
 * colors.c
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The set of RGBA colors for the Asgard Game Engine based upon the set of
 * common HTML color codes, along with color-related data types and helper
 * functions.
 *
 * Written by Josh English.
 */

// preprocessor directives

#include "core/core.h"
#define _ASGARD_CORE_COMPONENT
#include "asgard/core/common.h"
#include "asgard/core/colors.h"

// define colors public functions

Color *colors_new()
{
	Color *result = NULL;

	result = (Color *)malloc(sizeof(Color));

	return result;
}

Color *colors_newByName(char *colorName)
{
	Color *result = NULL;

	if(colorName == NULL) {
		DISPLAY_INVALID_ARGS;
	}

	result = colors_new();

	if(colorName != NULL) {
		colors_getColor(colorName, result);
	}

	return result;
}

Color *colors_clone(Color *color)
{
	Color *result = NULL;

	if(color == NULL) {
		DISPLAY_INVALID_ARGS;
	}

	result = colors_new();

	if(color != NULL) {
		result->r = color->r;
		result->g = color->g;
		result->b = color->b;
		result->a = color->a;
	}

	return result;
}

Color *colors_load(Log *log, char *assetFilename)
{
	char *sValue = NULL;

	Color *result = NULL;
	Config config;

	if((log == NULL) || (assetFilename == NULL)) {
		DISPLAY_INVALID_ARGS;
		return colors_new();
	}

	if(config_init(&config, assetFilename) < 0) {
		log_logf(log, LOG_LEVEL_ERROR,
				"{COLORS} failed to load configuration file '%s'",
				assetFilename);
		return colors_new();
	}

	// validate that the configuration file is for a 2D color asset

	config_getString(&config, "asset", "class", "unknown", &sValue);
	if(strcmp(sValue, "2d")) {
		log_logf(log, LOG_LEVEL_ERROR,
				"{COLORS} invalid color configuration - asset.class is '%s'",
				sValue);
		return colors_new();
	}

	config_getString(&config, "asset", "type", "unknown", &sValue);
	if(strcmp(sValue, "color")) {
		log_logf(log, LOG_LEVEL_ERROR,
				"{COLORS} invalid color configuration - asset.type is '%s'",
				sValue);
		return colors_new();
	}

	// load the color configuration

	result = colors_new();

	config_getDouble(&config, "color", "red", 0.0, &result->r);
	config_getDouble(&config, "color", "green", 0.0, &result->g);
	config_getDouble(&config, "color", "blue", 0.0, &result->b);
	config_getDouble(&config, "color", "alpha", 0.0, &result->a);

	log_logf(log, LOG_LEVEL_DEBUG,
			"{COLORS} loaded color '%s'", assetFilename);

	// cleanup

	config_free(&config);

	return result;
}

void colors_free(Color *color)
{
	if(color == NULL) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	free(color);
}

void colors_getColor(char *colorName, Color *color)
{
	if((colorName == NULL) || (color == NULL)) {
		DISPLAY_INVALID_ARGS;
		return;
	}

	if(!strcmp(colorName, CGI_COLOR_INDIANRED_NAME)) {
		color->r = CGI_COLOR_INDIANRED_RED;
		color->g = CGI_COLOR_INDIANRED_GREEN;
		color->b = CGI_COLOR_INDIANRED_BLUE;
		color->a = CGI_COLOR_INDIANRED_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_LIGHTCORAL_NAME)) {
		color->r = CGI_COLOR_LIGHTCORAL_RED;
		color->g = CGI_COLOR_LIGHTCORAL_GREEN;
		color->b = CGI_COLOR_LIGHTCORAL_BLUE;
		color->a = CGI_COLOR_LIGHTCORAL_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_SALMON_NAME)) {
		color->r = CGI_COLOR_SALMON_RED;
		color->g = CGI_COLOR_SALMON_GREEN;
		color->b = CGI_COLOR_SALMON_BLUE;
		color->a = CGI_COLOR_SALMON_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_DARKSALMON_NAME)) {
		color->r = CGI_COLOR_DARKSALMON_RED;
		color->g = CGI_COLOR_DARKSALMON_GREEN;
		color->b = CGI_COLOR_DARKSALMON_BLUE;
		color->a = CGI_COLOR_DARKSALMON_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_LIGHTSALMON_NAME)) {
		color->r = CGI_COLOR_LIGHTSALMON_RED;
		color->g = CGI_COLOR_LIGHTSALMON_GREEN;
		color->b = CGI_COLOR_LIGHTSALMON_BLUE;
		color->a = CGI_COLOR_LIGHTSALMON_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_CRIMSON_NAME)) {
		color->r = CGI_COLOR_CRIMSON_RED;
		color->g = CGI_COLOR_CRIMSON_GREEN;
		color->b = CGI_COLOR_CRIMSON_BLUE;
		color->a = CGI_COLOR_CRIMSON_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_RED_NAME)) {
		color->r = CGI_COLOR_RED_RED;
		color->g = CGI_COLOR_RED_GREEN;
		color->b = CGI_COLOR_RED_BLUE;
		color->a = CGI_COLOR_RED_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_FIREBRICK_NAME)) {
		color->r = CGI_COLOR_FIREBRICK_RED;
		color->g = CGI_COLOR_FIREBRICK_GREEN;
		color->b = CGI_COLOR_FIREBRICK_BLUE;
		color->a = CGI_COLOR_FIREBRICK_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_DARKRED_NAME)) {
		color->r = CGI_COLOR_DARKRED_RED;
		color->g = CGI_COLOR_DARKRED_GREEN;
		color->b = CGI_COLOR_DARKRED_BLUE;
		color->a = CGI_COLOR_DARKRED_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_PINK_NAME)) {
		color->r = CGI_COLOR_PINK_RED;
		color->g = CGI_COLOR_PINK_GREEN;
		color->b = CGI_COLOR_PINK_BLUE;
		color->a = CGI_COLOR_PINK_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_LIGHTPINK_NAME)) {
		color->r = CGI_COLOR_LIGHTPINK_RED;
		color->g = CGI_COLOR_LIGHTPINK_GREEN;
		color->b = CGI_COLOR_LIGHTPINK_BLUE;
		color->a = CGI_COLOR_LIGHTPINK_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_HOTPINK_NAME)) {
		color->r = CGI_COLOR_HOTPINK_RED;
		color->g = CGI_COLOR_HOTPINK_GREEN;
		color->b = CGI_COLOR_HOTPINK_BLUE;
		color->a = CGI_COLOR_HOTPINK_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_DEEPPINK_NAME)) {
		color->r = CGI_COLOR_DEEPPINK_RED;
		color->g = CGI_COLOR_DEEPPINK_GREEN;
		color->b = CGI_COLOR_DEEPPINK_BLUE;
		color->a = CGI_COLOR_DEEPPINK_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_MEDIUMVIOLETRED_NAME)) {
		color->r = CGI_COLOR_MEDIUMVIOLETRED_RED;
		color->g = CGI_COLOR_MEDIUMVIOLETRED_GREEN;
		color->b = CGI_COLOR_MEDIUMVIOLETRED_BLUE;
		color->a = CGI_COLOR_MEDIUMVIOLETRED_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_PALEVIOLETRED_NAME)) {
		color->r = CGI_COLOR_PALEVIOLETRED_RED;
		color->g = CGI_COLOR_PALEVIOLETRED_GREEN;
		color->b = CGI_COLOR_PALEVIOLETRED_BLUE;
		color->a = CGI_COLOR_PALEVIOLETRED_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_LIGHTSALMON_NAME)) {
		color->r = CGI_COLOR_LIGHTSALMON_RED;
		color->g = CGI_COLOR_LIGHTSALMON_GREEN;
		color->b = CGI_COLOR_LIGHTSALMON_BLUE;
		color->a = CGI_COLOR_LIGHTSALMON_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_CORAL_NAME)) {
		color->r = CGI_COLOR_CORAL_RED;
		color->g = CGI_COLOR_CORAL_GREEN;
		color->b = CGI_COLOR_CORAL_BLUE;
		color->a = CGI_COLOR_CORAL_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_TOMATO_NAME)) {
		color->r = CGI_COLOR_TOMATO_RED;
		color->g = CGI_COLOR_TOMATO_GREEN;
		color->b = CGI_COLOR_TOMATO_BLUE;
		color->a = CGI_COLOR_TOMATO_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_ORANGERED_NAME)) {
		color->r = CGI_COLOR_ORANGERED_RED;
		color->g = CGI_COLOR_ORANGERED_GREEN;
		color->b = CGI_COLOR_ORANGERED_BLUE;
		color->a = CGI_COLOR_ORANGERED_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_DARKORANGE_NAME)) {
		color->r = CGI_COLOR_DARKORANGE_RED;
		color->g = CGI_COLOR_DARKORANGE_GREEN;
		color->b = CGI_COLOR_DARKORANGE_BLUE;
		color->a = CGI_COLOR_DARKORANGE_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_ORANGE_NAME)) {
		color->r = CGI_COLOR_ORANGE_RED;
		color->g = CGI_COLOR_ORANGE_GREEN;
		color->b = CGI_COLOR_ORANGE_BLUE;
		color->a = CGI_COLOR_ORANGE_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_GOLD_NAME)) {
		color->r = CGI_COLOR_GOLD_RED;
		color->g = CGI_COLOR_GOLD_GREEN;
		color->b = CGI_COLOR_GOLD_BLUE;
		color->a = CGI_COLOR_GOLD_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_YELLOW_NAME)) {
		color->r = CGI_COLOR_YELLOW_RED;
		color->g = CGI_COLOR_YELLOW_GREEN;
		color->b = CGI_COLOR_YELLOW_BLUE;
		color->a = CGI_COLOR_YELLOW_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_LIGHTYELLOW_NAME)) {
		color->r = CGI_COLOR_LIGHTYELLOW_RED;
		color->g = CGI_COLOR_LIGHTYELLOW_GREEN;
		color->b = CGI_COLOR_LIGHTYELLOW_BLUE;
		color->a = CGI_COLOR_LIGHTYELLOW_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_LEMONCHIFFON_NAME)) {
		color->r = CGI_COLOR_LEMONCHIFFON_RED;
		color->g = CGI_COLOR_LEMONCHIFFON_GREEN;
		color->b = CGI_COLOR_LEMONCHIFFON_BLUE;
		color->a = CGI_COLOR_LEMONCHIFFON_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_LIGHTGOLDENRODYELLOW_NAME)) {
		color->r = CGI_COLOR_LIGHTGOLDENRODYELLOW_RED;
		color->g = CGI_COLOR_LIGHTGOLDENRODYELLOW_GREEN;
		color->b = CGI_COLOR_LIGHTGOLDENRODYELLOW_BLUE;
		color->a = CGI_COLOR_LIGHTGOLDENRODYELLOW_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_PAPAYAWHIP_NAME)) {
		color->r = CGI_COLOR_PAPAYAWHIP_RED;
		color->g = CGI_COLOR_PAPAYAWHIP_GREEN;
		color->b = CGI_COLOR_PAPAYAWHIP_BLUE;
		color->a = CGI_COLOR_PAPAYAWHIP_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_MOCCASIN_NAME)) {
		color->r = CGI_COLOR_MOCCASIN_RED;
		color->g = CGI_COLOR_MOCCASIN_GREEN;
		color->b = CGI_COLOR_MOCCASIN_BLUE;
		color->a = CGI_COLOR_MOCCASIN_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_PEACHPUFF_NAME)) {
		color->r = CGI_COLOR_PEACHPUFF_RED;
		color->g = CGI_COLOR_PEACHPUFF_GREEN;
		color->b = CGI_COLOR_PEACHPUFF_BLUE;
		color->a = CGI_COLOR_PEACHPUFF_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_PALEGOLDENROD_NAME)) {
		color->r = CGI_COLOR_PALEGOLDENROD_RED;
		color->g = CGI_COLOR_PALEGOLDENROD_GREEN;
		color->b = CGI_COLOR_PALEGOLDENROD_BLUE;
		color->a = CGI_COLOR_PALEGOLDENROD_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_KHAKI_NAME)) {
		color->r = CGI_COLOR_KHAKI_RED;
		color->g = CGI_COLOR_KHAKI_GREEN;
		color->b = CGI_COLOR_KHAKI_BLUE;
		color->a = CGI_COLOR_KHAKI_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_DARKKHAKI_NAME)) {
		color->r = CGI_COLOR_DARKKHAKI_RED;
		color->g = CGI_COLOR_DARKKHAKI_GREEN;
		color->b = CGI_COLOR_DARKKHAKI_BLUE;
		color->a = CGI_COLOR_DARKKHAKI_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_LAVENDER_NAME)) {
		color->r = CGI_COLOR_LAVENDER_RED;
		color->g = CGI_COLOR_LAVENDER_GREEN;
		color->b = CGI_COLOR_LAVENDER_BLUE;
		color->a = CGI_COLOR_LAVENDER_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_THISTLE_NAME)) {
		color->r = CGI_COLOR_THISTLE_RED;
		color->g = CGI_COLOR_THISTLE_GREEN;
		color->b = CGI_COLOR_THISTLE_BLUE;
		color->a = CGI_COLOR_THISTLE_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_PLUM_NAME)) {
		color->r = CGI_COLOR_PLUM_RED;
		color->g = CGI_COLOR_PLUM_GREEN;
		color->b = CGI_COLOR_PLUM_BLUE;
		color->a = CGI_COLOR_PLUM_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_VIOLET_NAME)) {
		color->r = CGI_COLOR_VIOLET_RED;
		color->g = CGI_COLOR_VIOLET_GREEN;
		color->b = CGI_COLOR_VIOLET_BLUE;
		color->a = CGI_COLOR_VIOLET_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_ORCHID_NAME)) {
		color->r = CGI_COLOR_ORCHID_RED;
		color->g = CGI_COLOR_ORCHID_GREEN;
		color->b = CGI_COLOR_ORCHID_BLUE;
		color->a = CGI_COLOR_ORCHID_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_FUCHSIA_NAME)) {
		color->r = CGI_COLOR_FUCHSIA_RED;
		color->g = CGI_COLOR_FUCHSIA_GREEN;
		color->b = CGI_COLOR_FUCHSIA_BLUE;
		color->a = CGI_COLOR_FUCHSIA_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_MAGENTA_NAME)) {
		color->r = CGI_COLOR_MAGENTA_RED;
		color->g = CGI_COLOR_MAGENTA_GREEN;
		color->b = CGI_COLOR_MAGENTA_BLUE;
		color->a = CGI_COLOR_MAGENTA_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_MEDIUMORCHID_NAME)) {
		color->r = CGI_COLOR_MEDIUMORCHID_RED;
		color->g = CGI_COLOR_MEDIUMORCHID_GREEN;
		color->b = CGI_COLOR_MEDIUMORCHID_BLUE;
		color->a = CGI_COLOR_MEDIUMORCHID_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_MEDIUMPURPLE_NAME)) {
		color->r = CGI_COLOR_MEDIUMPURPLE_RED;
		color->g = CGI_COLOR_MEDIUMPURPLE_GREEN;
		color->b = CGI_COLOR_MEDIUMPURPLE_BLUE;
		color->a = CGI_COLOR_MEDIUMPURPLE_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_AMETHYST_NAME)) {
		color->r = CGI_COLOR_AMETHYST_RED;
		color->g = CGI_COLOR_AMETHYST_GREEN;
		color->b = CGI_COLOR_AMETHYST_BLUE;
		color->a = CGI_COLOR_AMETHYST_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_BLUEVIOLET_NAME)) {
		color->r = CGI_COLOR_BLUEVIOLET_RED;
		color->g = CGI_COLOR_BLUEVIOLET_GREEN;
		color->b = CGI_COLOR_BLUEVIOLET_BLUE;
		color->a = CGI_COLOR_BLUEVIOLET_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_DARKVIOLET_NAME)) {
		color->r = CGI_COLOR_DARKVIOLET_RED;
		color->g = CGI_COLOR_DARKVIOLET_GREEN;
		color->b = CGI_COLOR_DARKVIOLET_BLUE;
		color->a = CGI_COLOR_DARKVIOLET_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_DARKORCHID_NAME)) {
		color->r = CGI_COLOR_DARKORCHID_RED;
		color->g = CGI_COLOR_DARKORCHID_GREEN;
		color->b = CGI_COLOR_DARKORCHID_BLUE;
		color->a = CGI_COLOR_DARKORCHID_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_DARKMAGENTA_NAME)) {
		color->r = CGI_COLOR_DARKMAGENTA_RED;
		color->g = CGI_COLOR_DARKMAGENTA_GREEN;
		color->b = CGI_COLOR_DARKMAGENTA_BLUE;
		color->a = CGI_COLOR_DARKMAGENTA_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_PURPLE_NAME)) {
		color->r = CGI_COLOR_PURPLE_RED;
		color->g = CGI_COLOR_PURPLE_GREEN;
		color->b = CGI_COLOR_PURPLE_BLUE;
		color->a = CGI_COLOR_PURPLE_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_INDIGO_NAME)) {
		color->r = CGI_COLOR_INDIGO_RED;
		color->g = CGI_COLOR_INDIGO_GREEN;
		color->b = CGI_COLOR_INDIGO_BLUE;
		color->a = CGI_COLOR_INDIGO_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_SLATEBLUE_NAME)) {
		color->r = CGI_COLOR_SLATEBLUE_RED;
		color->g = CGI_COLOR_SLATEBLUE_GREEN;
		color->b = CGI_COLOR_SLATEBLUE_BLUE;
		color->a = CGI_COLOR_SLATEBLUE_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_DARKSLATEBLUE_NAME)) {
		color->r = CGI_COLOR_DARKSLATEBLUE_RED;
		color->g = CGI_COLOR_DARKSLATEBLUE_GREEN;
		color->b = CGI_COLOR_DARKSLATEBLUE_BLUE;
		color->a = CGI_COLOR_DARKSLATEBLUE_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_MEDIUMSLATEBLUE_NAME)) {
		color->r = CGI_COLOR_MEDIUMSLATEBLUE_RED;
		color->g = CGI_COLOR_MEDIUMSLATEBLUE_GREEN;
		color->b = CGI_COLOR_MEDIUMSLATEBLUE_BLUE;
		color->a = CGI_COLOR_MEDIUMSLATEBLUE_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_GREENYELLOW_NAME)) {
		color->r = CGI_COLOR_GREENYELLOW_RED;
		color->g = CGI_COLOR_GREENYELLOW_GREEN;
		color->b = CGI_COLOR_GREENYELLOW_BLUE;
		color->a = CGI_COLOR_GREENYELLOW_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_CHARTREUSE_NAME)) {
		color->r = CGI_COLOR_CHARTREUSE_RED;
		color->g = CGI_COLOR_CHARTREUSE_GREEN;
		color->b = CGI_COLOR_CHARTREUSE_BLUE;
		color->a = CGI_COLOR_CHARTREUSE_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_LAWNGREEN_NAME)) {
		color->r = CGI_COLOR_LAWNGREEN_RED;
		color->g = CGI_COLOR_LAWNGREEN_GREEN;
		color->b = CGI_COLOR_LAWNGREEN_BLUE;
		color->a = CGI_COLOR_LAWNGREEN_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_LIME_NAME)) {
		color->r = CGI_COLOR_LIME_RED;
		color->g = CGI_COLOR_LIME_GREEN;
		color->b = CGI_COLOR_LIME_BLUE;
		color->a = CGI_COLOR_LIME_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_LIMEGREEN_NAME)) {
		color->r = CGI_COLOR_LIMEGREEN_RED;
		color->g = CGI_COLOR_LIMEGREEN_GREEN;
		color->b = CGI_COLOR_LIMEGREEN_BLUE;
		color->a = CGI_COLOR_LIMEGREEN_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_PALEGREEN_NAME)) {
		color->r = CGI_COLOR_PALEGREEN_RED;
		color->g = CGI_COLOR_PALEGREEN_GREEN;
		color->b = CGI_COLOR_PALEGREEN_BLUE;
		color->a = CGI_COLOR_PALEGREEN_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_LIGHTGREEN_NAME)) {
		color->r = CGI_COLOR_LIGHTGREEN_RED;
		color->g = CGI_COLOR_LIGHTGREEN_GREEN;
		color->b = CGI_COLOR_LIGHTGREEN_BLUE;
		color->a = CGI_COLOR_LIGHTGREEN_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_MEDIUMSPRINGGREEN_NAME)) {
		color->r = CGI_COLOR_MEDIUMSPRINGGREEN_RED;
		color->g = CGI_COLOR_MEDIUMSPRINGGREEN_GREEN;
		color->b = CGI_COLOR_MEDIUMSPRINGGREEN_BLUE;
		color->a = CGI_COLOR_MEDIUMSPRINGGREEN_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_SPRINGGREEN_NAME)) {
		color->r = CGI_COLOR_SPRINGGREEN_RED;
		color->g = CGI_COLOR_SPRINGGREEN_GREEN;
		color->b = CGI_COLOR_SPRINGGREEN_BLUE;
		color->a = CGI_COLOR_SPRINGGREEN_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_MEDIUMSEAGREEN_NAME)) {
		color->r = CGI_COLOR_MEDIUMSEAGREEN_RED;
		color->g = CGI_COLOR_MEDIUMSEAGREEN_GREEN;
		color->b = CGI_COLOR_MEDIUMSEAGREEN_BLUE;
		color->a = CGI_COLOR_MEDIUMSEAGREEN_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_SEAGREEN_NAME)) {
		color->r = CGI_COLOR_SEAGREEN_RED;
		color->g = CGI_COLOR_SEAGREEN_GREEN;
		color->b = CGI_COLOR_SEAGREEN_BLUE;
		color->a = CGI_COLOR_SEAGREEN_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_FORESTGREEN_NAME)) {
		color->r = CGI_COLOR_FORESTGREEN_RED;
		color->g = CGI_COLOR_FORESTGREEN_GREEN;
		color->b = CGI_COLOR_FORESTGREEN_BLUE;
		color->a = CGI_COLOR_FORESTGREEN_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_GREEN_NAME)) {
		color->r = CGI_COLOR_GREEN_RED;
		color->g = CGI_COLOR_GREEN_GREEN;
		color->b = CGI_COLOR_GREEN_BLUE;
		color->a = CGI_COLOR_GREEN_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_DARKGREEN_NAME)) {
		color->r = CGI_COLOR_DARKGREEN_RED;
		color->g = CGI_COLOR_DARKGREEN_GREEN;
		color->b = CGI_COLOR_DARKGREEN_BLUE;
		color->a = CGI_COLOR_DARKGREEN_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_YELLOWGREEN_NAME)) {
		color->r = CGI_COLOR_YELLOWGREEN_RED;
		color->g = CGI_COLOR_YELLOWGREEN_GREEN;
		color->b = CGI_COLOR_YELLOWGREEN_BLUE;
		color->a = CGI_COLOR_YELLOWGREEN_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_OLIVEDRAB_NAME)) {
		color->r = CGI_COLOR_OLIVEDRAB_RED;
		color->g = CGI_COLOR_OLIVEDRAB_GREEN;
		color->b = CGI_COLOR_OLIVEDRAB_BLUE;
		color->a = CGI_COLOR_OLIVEDRAB_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_OLIVE_NAME)) {
		color->r = CGI_COLOR_OLIVE_RED;
		color->g = CGI_COLOR_OLIVE_GREEN;
		color->b = CGI_COLOR_OLIVE_BLUE;
		color->a = CGI_COLOR_OLIVE_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_DARKOLIVEGREEN_NAME)) {
		color->r = CGI_COLOR_DARKOLIVEGREEN_RED;
		color->g = CGI_COLOR_DARKOLIVEGREEN_GREEN;
		color->b = CGI_COLOR_DARKOLIVEGREEN_BLUE;
		color->a = CGI_COLOR_DARKOLIVEGREEN_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_MEDIUMAQUAMARINE_NAME)) {
		color->r = CGI_COLOR_MEDIUMAQUAMARINE_RED;
		color->g = CGI_COLOR_MEDIUMAQUAMARINE_GREEN;
		color->b = CGI_COLOR_MEDIUMAQUAMARINE_BLUE;
		color->a = CGI_COLOR_MEDIUMAQUAMARINE_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_DARKSEAGREEN_NAME)) {
		color->r = CGI_COLOR_DARKSEAGREEN_RED;
		color->g = CGI_COLOR_DARKSEAGREEN_GREEN;
		color->b = CGI_COLOR_DARKSEAGREEN_BLUE;
		color->a = CGI_COLOR_DARKSEAGREEN_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_LIGHTSEAGREEN_NAME)) {
		color->r = CGI_COLOR_LIGHTSEAGREEN_RED;
		color->g = CGI_COLOR_LIGHTSEAGREEN_GREEN;
		color->b = CGI_COLOR_LIGHTSEAGREEN_BLUE;
		color->a = CGI_COLOR_LIGHTSEAGREEN_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_DARKCYAN_NAME)) {
		color->r = CGI_COLOR_DARKCYAN_RED;
		color->g = CGI_COLOR_DARKCYAN_GREEN;
		color->b = CGI_COLOR_DARKCYAN_BLUE;
		color->a = CGI_COLOR_DARKCYAN_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_TEAL_NAME)) {
		color->r = CGI_COLOR_TEAL_RED;
		color->g = CGI_COLOR_TEAL_GREEN;
		color->b = CGI_COLOR_TEAL_BLUE;
		color->a = CGI_COLOR_TEAL_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_AQUA_NAME)) {
		color->r = CGI_COLOR_AQUA_RED;
		color->g = CGI_COLOR_AQUA_GREEN;
		color->b = CGI_COLOR_AQUA_BLUE;
		color->a = CGI_COLOR_AQUA_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_CYAN_NAME)) {
		color->r = CGI_COLOR_CYAN_RED;
		color->g = CGI_COLOR_CYAN_GREEN;
		color->b = CGI_COLOR_CYAN_BLUE;
		color->a = CGI_COLOR_CYAN_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_LIGHTCYAN_NAME)) {
		color->r = CGI_COLOR_LIGHTCYAN_RED;
		color->g = CGI_COLOR_LIGHTCYAN_GREEN;
		color->b = CGI_COLOR_LIGHTCYAN_BLUE;
		color->a = CGI_COLOR_LIGHTCYAN_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_PALETURQUOISE_NAME)) {
		color->r = CGI_COLOR_PALETURQUOISE_RED;
		color->g = CGI_COLOR_PALETURQUOISE_GREEN;
		color->b = CGI_COLOR_PALETURQUOISE_BLUE;
		color->a = CGI_COLOR_PALETURQUOISE_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_AQUAMARINE_NAME)) {
		color->r = CGI_COLOR_AQUAMARINE_RED;
		color->g = CGI_COLOR_AQUAMARINE_GREEN;
		color->b = CGI_COLOR_AQUAMARINE_BLUE;
		color->a = CGI_COLOR_AQUAMARINE_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_TURQUOISE_NAME)) {
		color->r = CGI_COLOR_TURQUOISE_RED;
		color->g = CGI_COLOR_TURQUOISE_GREEN;
		color->b = CGI_COLOR_TURQUOISE_BLUE;
		color->a = CGI_COLOR_TURQUOISE_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_MEDIUMTURQUOISE_NAME)) {
		color->r = CGI_COLOR_MEDIUMTURQUOISE_RED;
		color->g = CGI_COLOR_MEDIUMTURQUOISE_GREEN;
		color->b = CGI_COLOR_MEDIUMTURQUOISE_BLUE;
		color->a = CGI_COLOR_MEDIUMTURQUOISE_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_DARKTURQUOISE_NAME)) {
		color->r = CGI_COLOR_DARKTURQUOISE_RED;
		color->g = CGI_COLOR_DARKTURQUOISE_GREEN;
		color->b = CGI_COLOR_DARKTURQUOISE_BLUE;
		color->a = CGI_COLOR_DARKTURQUOISE_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_CADETBLUE_NAME)) {
		color->r = CGI_COLOR_CADETBLUE_RED;
		color->g = CGI_COLOR_CADETBLUE_GREEN;
		color->b = CGI_COLOR_CADETBLUE_BLUE;
		color->a = CGI_COLOR_CADETBLUE_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_STEELBLUE_NAME)) {
		color->r = CGI_COLOR_STEELBLUE_RED;
		color->g = CGI_COLOR_STEELBLUE_GREEN;
		color->b = CGI_COLOR_STEELBLUE_BLUE;
		color->a = CGI_COLOR_STEELBLUE_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_LIGHTSTEELBLUE_NAME)) {
		color->r = CGI_COLOR_LIGHTSTEELBLUE_RED;
		color->g = CGI_COLOR_LIGHTSTEELBLUE_GREEN;
		color->b = CGI_COLOR_LIGHTSTEELBLUE_BLUE;
		color->a = CGI_COLOR_LIGHTSTEELBLUE_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_POWDERBLUE_NAME)) {
		color->r = CGI_COLOR_POWDERBLUE_RED;
		color->g = CGI_COLOR_POWDERBLUE_GREEN;
		color->b = CGI_COLOR_POWDERBLUE_BLUE;
		color->a = CGI_COLOR_POWDERBLUE_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_LIGHTBLUE_NAME)) {
		color->r = CGI_COLOR_LIGHTBLUE_RED;
		color->g = CGI_COLOR_LIGHTBLUE_GREEN;
		color->b = CGI_COLOR_LIGHTBLUE_BLUE;
		color->a = CGI_COLOR_LIGHTBLUE_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_SKYBLUE_NAME)) {
		color->r = CGI_COLOR_SKYBLUE_RED;
		color->g = CGI_COLOR_SKYBLUE_GREEN;
		color->b = CGI_COLOR_SKYBLUE_BLUE;
		color->a = CGI_COLOR_SKYBLUE_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_LIGHTSKYBLUE_NAME)) {
		color->r = CGI_COLOR_LIGHTSKYBLUE_RED;
		color->g = CGI_COLOR_LIGHTSKYBLUE_GREEN;
		color->b = CGI_COLOR_LIGHTSKYBLUE_BLUE;
		color->a = CGI_COLOR_LIGHTSKYBLUE_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_DEEPSKYBLUE_NAME)) {
		color->r = CGI_COLOR_DEEPSKYBLUE_RED;
		color->g = CGI_COLOR_DEEPSKYBLUE_GREEN;
		color->b = CGI_COLOR_DEEPSKYBLUE_BLUE;
		color->a = CGI_COLOR_DEEPSKYBLUE_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_DODGERBLUE_NAME)) {
		color->r = CGI_COLOR_DODGERBLUE_RED;
		color->g = CGI_COLOR_DODGERBLUE_GREEN;
		color->b = CGI_COLOR_DODGERBLUE_BLUE;
		color->a = CGI_COLOR_DODGERBLUE_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_CORNFLOWERBLUE_NAME)) {
		color->r = CGI_COLOR_CORNFLOWERBLUE_RED;
		color->g = CGI_COLOR_CORNFLOWERBLUE_GREEN;
		color->b = CGI_COLOR_CORNFLOWERBLUE_BLUE;
		color->a = CGI_COLOR_CORNFLOWERBLUE_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_MEDIUMSLATEBLUE_NAME)) {
		color->r = CGI_COLOR_MEDIUMSLATEBLUE_RED;
		color->g = CGI_COLOR_MEDIUMSLATEBLUE_GREEN;
		color->b = CGI_COLOR_MEDIUMSLATEBLUE_BLUE;
		color->a = CGI_COLOR_MEDIUMSLATEBLUE_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_ROYALBLUE_NAME)) {
		color->r = CGI_COLOR_ROYALBLUE_RED;
		color->g = CGI_COLOR_ROYALBLUE_GREEN;
		color->b = CGI_COLOR_ROYALBLUE_BLUE;
		color->a = CGI_COLOR_ROYALBLUE_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_BLUE_NAME)) {
		color->r = CGI_COLOR_BLUE_RED;
		color->g = CGI_COLOR_BLUE_GREEN;
		color->b = CGI_COLOR_BLUE_BLUE;
		color->a = CGI_COLOR_BLUE_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_MEDIUMBLUE_NAME)) {
		color->r = CGI_COLOR_MEDIUMBLUE_RED;
		color->g = CGI_COLOR_MEDIUMBLUE_GREEN;
		color->b = CGI_COLOR_MEDIUMBLUE_BLUE;
		color->a = CGI_COLOR_MEDIUMBLUE_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_DARKBLUE_NAME)) {
		color->r = CGI_COLOR_DARKBLUE_RED;
		color->g = CGI_COLOR_DARKBLUE_GREEN;
		color->b = CGI_COLOR_DARKBLUE_BLUE;
		color->a = CGI_COLOR_DARKBLUE_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_NAVY_NAME)) {
		color->r = CGI_COLOR_NAVY_RED;
		color->g = CGI_COLOR_NAVY_GREEN;
		color->b = CGI_COLOR_NAVY_BLUE;
		color->a = CGI_COLOR_NAVY_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_MIDNIGHTBLUE_NAME)) {
		color->r = CGI_COLOR_MIDNIGHTBLUE_RED;
		color->g = CGI_COLOR_MIDNIGHTBLUE_GREEN;
		color->b = CGI_COLOR_MIDNIGHTBLUE_BLUE;
		color->a = CGI_COLOR_MIDNIGHTBLUE_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_CORNSILK_NAME)) {
		color->r = CGI_COLOR_CORNSILK_RED;
		color->g = CGI_COLOR_CORNSILK_GREEN;
		color->b = CGI_COLOR_CORNSILK_BLUE;
		color->a = CGI_COLOR_CORNSILK_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_BLANCHEDALMOND_NAME)) {
		color->r = CGI_COLOR_BLANCHEDALMOND_RED;
		color->g = CGI_COLOR_BLANCHEDALMOND_GREEN;
		color->b = CGI_COLOR_BLANCHEDALMOND_BLUE;
		color->a = CGI_COLOR_BLANCHEDALMOND_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_BISQUE_NAME)) {
		color->r = CGI_COLOR_BISQUE_RED;
		color->g = CGI_COLOR_BISQUE_GREEN;
		color->b = CGI_COLOR_BISQUE_BLUE;
		color->a = CGI_COLOR_BISQUE_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_NAVAJOWHITE_NAME)) {
		color->r = CGI_COLOR_NAVAJOWHITE_RED;
		color->g = CGI_COLOR_NAVAJOWHITE_GREEN;
		color->b = CGI_COLOR_NAVAJOWHITE_BLUE;
		color->a = CGI_COLOR_NAVAJOWHITE_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_WHEAT_NAME)) {
		color->r = CGI_COLOR_WHEAT_RED;
		color->g = CGI_COLOR_WHEAT_GREEN;
		color->b = CGI_COLOR_WHEAT_BLUE;
		color->a = CGI_COLOR_WHEAT_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_BURLYWOOD_NAME)) {
		color->r = CGI_COLOR_BURLYWOOD_RED;
		color->g = CGI_COLOR_BURLYWOOD_GREEN;
		color->b = CGI_COLOR_BURLYWOOD_BLUE;
		color->a = CGI_COLOR_BURLYWOOD_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_TAN_NAME)) {
		color->r = CGI_COLOR_TAN_RED;
		color->g = CGI_COLOR_TAN_GREEN;
		color->b = CGI_COLOR_TAN_BLUE;
		color->a = CGI_COLOR_TAN_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_ROSYBROWN_NAME)) {
		color->r = CGI_COLOR_ROSYBROWN_RED;
		color->g = CGI_COLOR_ROSYBROWN_GREEN;
		color->b = CGI_COLOR_ROSYBROWN_BLUE;
		color->a = CGI_COLOR_ROSYBROWN_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_SANDYBROWN_NAME)) {
		color->r = CGI_COLOR_SANDYBROWN_RED;
		color->g = CGI_COLOR_SANDYBROWN_GREEN;
		color->b = CGI_COLOR_SANDYBROWN_BLUE;
		color->a = CGI_COLOR_SANDYBROWN_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_GOLDENROD_NAME)) {
		color->r = CGI_COLOR_GOLDENROD_RED;
		color->g = CGI_COLOR_GOLDENROD_GREEN;
		color->b = CGI_COLOR_GOLDENROD_BLUE;
		color->a = CGI_COLOR_GOLDENROD_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_DARKGOLDENROD_NAME)) {
		color->r = CGI_COLOR_DARKGOLDENROD_RED;
		color->g = CGI_COLOR_DARKGOLDENROD_GREEN;
		color->b = CGI_COLOR_DARKGOLDENROD_BLUE;
		color->a = CGI_COLOR_DARKGOLDENROD_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_PERU_NAME)) {
		color->r = CGI_COLOR_PERU_RED;
		color->g = CGI_COLOR_PERU_GREEN;
		color->b = CGI_COLOR_PERU_BLUE;
		color->a = CGI_COLOR_PERU_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_CHOCOLATE_NAME)) {
		color->r = CGI_COLOR_CHOCOLATE_RED;
		color->g = CGI_COLOR_CHOCOLATE_GREEN;
		color->b = CGI_COLOR_CHOCOLATE_BLUE;
		color->a = CGI_COLOR_CHOCOLATE_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_SADDLEBROWN_NAME)) {
		color->r = CGI_COLOR_SADDLEBROWN_RED;
		color->g = CGI_COLOR_SADDLEBROWN_GREEN;
		color->b = CGI_COLOR_SADDLEBROWN_BLUE;
		color->a = CGI_COLOR_SADDLEBROWN_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_SIENNA_NAME)) {
		color->r = CGI_COLOR_SIENNA_RED;
		color->g = CGI_COLOR_SIENNA_GREEN;
		color->b = CGI_COLOR_SIENNA_BLUE;
		color->a = CGI_COLOR_SIENNA_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_BROWN_NAME)) {
		color->r = CGI_COLOR_BROWN_RED;
		color->g = CGI_COLOR_BROWN_GREEN;
		color->b = CGI_COLOR_BROWN_BLUE;
		color->a = CGI_COLOR_BROWN_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_MAROON_NAME)) {
		color->r = CGI_COLOR_MAROON_RED;
		color->g = CGI_COLOR_MAROON_GREEN;
		color->b = CGI_COLOR_MAROON_BLUE;
		color->a = CGI_COLOR_MAROON_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_WHITE_NAME)) {
		color->r = CGI_COLOR_WHITE_RED;
		color->g = CGI_COLOR_WHITE_GREEN;
		color->b = CGI_COLOR_WHITE_BLUE;
		color->a = CGI_COLOR_WHITE_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_SNOW_NAME)) {
		color->r = CGI_COLOR_SNOW_RED;
		color->g = CGI_COLOR_SNOW_GREEN;
		color->b = CGI_COLOR_SNOW_BLUE;
		color->a = CGI_COLOR_SNOW_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_HONEYDEW_NAME)) {
		color->r = CGI_COLOR_HONEYDEW_RED;
		color->g = CGI_COLOR_HONEYDEW_GREEN;
		color->b = CGI_COLOR_HONEYDEW_BLUE;
		color->a = CGI_COLOR_HONEYDEW_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_MINTCREAM_NAME)) {
		color->r = CGI_COLOR_MINTCREAM_RED;
		color->g = CGI_COLOR_MINTCREAM_GREEN;
		color->b = CGI_COLOR_MINTCREAM_BLUE;
		color->a = CGI_COLOR_MINTCREAM_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_AZURE_NAME)) {
		color->r = CGI_COLOR_AZURE_RED;
		color->g = CGI_COLOR_AZURE_GREEN;
		color->b = CGI_COLOR_AZURE_BLUE;
		color->a = CGI_COLOR_AZURE_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_ALICEBLUE_NAME)) {
		color->r = CGI_COLOR_ALICEBLUE_RED;
		color->g = CGI_COLOR_ALICEBLUE_GREEN;
		color->b = CGI_COLOR_ALICEBLUE_BLUE;
		color->a = CGI_COLOR_ALICEBLUE_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_GHOSTWHITE_NAME)) {
		color->r = CGI_COLOR_GHOSTWHITE_RED;
		color->g = CGI_COLOR_GHOSTWHITE_GREEN;
		color->b = CGI_COLOR_GHOSTWHITE_BLUE;
		color->a = CGI_COLOR_GHOSTWHITE_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_WHITESMOKE_NAME)) {
		color->r = CGI_COLOR_WHITESMOKE_RED;
		color->g = CGI_COLOR_WHITESMOKE_GREEN;
		color->b = CGI_COLOR_WHITESMOKE_BLUE;
		color->a = CGI_COLOR_WHITESMOKE_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_SEASHELL_NAME)) {
		color->r = CGI_COLOR_SEASHELL_RED;
		color->g = CGI_COLOR_SEASHELL_GREEN;
		color->b = CGI_COLOR_SEASHELL_BLUE;
		color->a = CGI_COLOR_SEASHELL_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_BEIGE_NAME)) {
		color->r = CGI_COLOR_BEIGE_RED;
		color->g = CGI_COLOR_BEIGE_GREEN;
		color->b = CGI_COLOR_BEIGE_BLUE;
		color->a = CGI_COLOR_BEIGE_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_OLDLACE_NAME)) {
		color->r = CGI_COLOR_OLDLACE_RED;
		color->g = CGI_COLOR_OLDLACE_GREEN;
		color->b = CGI_COLOR_OLDLACE_BLUE;
		color->a = CGI_COLOR_OLDLACE_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_FLORALWHITE_NAME)) {
		color->r = CGI_COLOR_FLORALWHITE_RED;
		color->g = CGI_COLOR_FLORALWHITE_GREEN;
		color->b = CGI_COLOR_FLORALWHITE_BLUE;
		color->a = CGI_COLOR_FLORALWHITE_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_IVORY_NAME)) {
		color->r = CGI_COLOR_IVORY_RED;
		color->g = CGI_COLOR_IVORY_GREEN;
		color->b = CGI_COLOR_IVORY_BLUE;
		color->a = CGI_COLOR_IVORY_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_ANTIQUEWHITE_NAME)) {
		color->r = CGI_COLOR_ANTIQUEWHITE_RED;
		color->g = CGI_COLOR_ANTIQUEWHITE_GREEN;
		color->b = CGI_COLOR_ANTIQUEWHITE_BLUE;
		color->a = CGI_COLOR_ANTIQUEWHITE_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_LINEN_NAME)) {
		color->r = CGI_COLOR_LINEN_RED;
		color->g = CGI_COLOR_LINEN_GREEN;
		color->b = CGI_COLOR_LINEN_BLUE;
		color->a = CGI_COLOR_LINEN_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_LAVENDERBLUSH_NAME)) {
		color->r = CGI_COLOR_LAVENDERBLUSH_RED;
		color->g = CGI_COLOR_LAVENDERBLUSH_GREEN;
		color->b = CGI_COLOR_LAVENDERBLUSH_BLUE;
		color->a = CGI_COLOR_LAVENDERBLUSH_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_MISTYROSE_NAME)) {
		color->r = CGI_COLOR_MISTYROSE_RED;
		color->g = CGI_COLOR_MISTYROSE_GREEN;
		color->b = CGI_COLOR_MISTYROSE_BLUE;
		color->a = CGI_COLOR_MISTYROSE_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_GAINSBORO_NAME)) {
		color->r = CGI_COLOR_GAINSBORO_RED;
		color->g = CGI_COLOR_GAINSBORO_GREEN;
		color->b = CGI_COLOR_GAINSBORO_BLUE;
		color->a = CGI_COLOR_GAINSBORO_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_LIGHTGREY_NAME)) {
		color->r = CGI_COLOR_LIGHTGREY_RED;
		color->g = CGI_COLOR_LIGHTGREY_GREEN;
		color->b = CGI_COLOR_LIGHTGREY_BLUE;
		color->a = CGI_COLOR_LIGHTGREY_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_SILVER_NAME)) {
		color->r = CGI_COLOR_SILVER_RED;
		color->g = CGI_COLOR_SILVER_GREEN;
		color->b = CGI_COLOR_SILVER_BLUE;
		color->a = CGI_COLOR_SILVER_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_DARKGRAY_NAME)) {
		color->r = CGI_COLOR_DARKGRAY_RED;
		color->g = CGI_COLOR_DARKGRAY_GREEN;
		color->b = CGI_COLOR_DARKGRAY_BLUE;
		color->a = CGI_COLOR_DARKGRAY_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_GRAY_NAME)) {
		color->r = CGI_COLOR_GRAY_RED;
		color->g = CGI_COLOR_GRAY_GREEN;
		color->b = CGI_COLOR_GRAY_BLUE;
		color->a = CGI_COLOR_GRAY_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_DIMGRAY_NAME)) {
		color->r = CGI_COLOR_DIMGRAY_RED;
		color->g = CGI_COLOR_DIMGRAY_GREEN;
		color->b = CGI_COLOR_DIMGRAY_BLUE;
		color->a = CGI_COLOR_DIMGRAY_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_LIGHTSLATEGRAY_NAME)) {
		color->r = CGI_COLOR_LIGHTSLATEGRAY_RED;
		color->g = CGI_COLOR_LIGHTSLATEGRAY_GREEN;
		color->b = CGI_COLOR_LIGHTSLATEGRAY_BLUE;
		color->a = CGI_COLOR_LIGHTSLATEGRAY_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_SLATEGRAY_NAME)) {
		color->r = CGI_COLOR_SLATEGRAY_RED;
		color->g = CGI_COLOR_SLATEGRAY_GREEN;
		color->b = CGI_COLOR_SLATEGRAY_BLUE;
		color->a = CGI_COLOR_SLATEGRAY_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_DARKSLATEGRAY_NAME)) {
		color->r = CGI_COLOR_DARKSLATEGRAY_RED;
		color->g = CGI_COLOR_DARKSLATEGRAY_GREEN;
		color->b = CGI_COLOR_DARKSLATEGRAY_BLUE;
		color->a = CGI_COLOR_DARKSLATEGRAY_ALPHA;
	}
	else if(!strcmp(colorName, CGI_COLOR_BLACK_NAME)) {
		color->r = CGI_COLOR_BLACK_RED;
		color->g = CGI_COLOR_BLACK_GREEN;
		color->b = CGI_COLOR_BLACK_BLUE;
		color->a = CGI_COLOR_BLACK_ALPHA;
	}
	else {
		color->r = CGI_COLOR_BLACK_RED;
		color->g = CGI_COLOR_BLACK_GREEN;
		color->b = CGI_COLOR_BLACK_BLUE;
		color->a = CGI_COLOR_BLACK_ALPHA;
	}
}

