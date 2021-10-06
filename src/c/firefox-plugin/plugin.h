/*
 * plugin.h
 *
 * Copyright (C) 2008 - 2011 by Joshua S. English. This document is the
 * intellectual property of Joshua S. English, all rights reserved.
 *
 * The Asgard Game Engine plug-in wrapper header-file for Mozilla Firefox.
 *
 * Written by Josh English
 */

// preprocessor directives

#if !defined(_FIREFOX_PLUGIN_H)

#define _FIREFOX_PLUGIN_H

#include "pluginbase.h"

class nsPluginInstance : public nsPluginInstanceBase
{
	private:
		NPP mInstance;
		NPBool mInitialized;
		HWND mhWnd;

	public:
		nsPluginInstance(NPP aInstance);
		~nsPluginInstance();
		NPBool init(NPWindow* aWindow);
		void shut();
		NPBool isInitialized();

		const char *getVersion();
};

#endif // _FIREFOX_PLUGIN_H

