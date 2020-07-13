/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_cengine.h"
#include "pragma/launchparameters.h"
#include "pragma/c_engine.h"

extern DLLCENGINE CEngine *c_engine;
static void LPARAM_windowed(const std::vector<std::string> &argv)
{
	c_engine->SetWindowedMode(true);
}

static void LPARAM_refresh(const std::vector<std::string> &argv)
{
	if(argv.empty())
		return;
	int freq = atoi(argv[0].c_str());
	if(freq > 0)
		c_engine->SetRefreshRate(freq);
}

static void LPARAM_noborder(const std::vector<std::string> &argv)
{
	c_engine->SetNoBorder(true);
}

static void LPARAM_w(const std::vector<std::string> &argv)
{
	if(argv.empty())
		return;
	c_engine->SetResolutionWidth(atoi(argv[0].c_str()));
}

static void LPARAM_h(const std::vector<std::string> &argv)
{
	if(argv.empty())
		return;
	c_engine->SetResolutionHeight(atoi(argv[0].c_str()));
}

static void LPARAM_fullbright(const std::vector<std::string> &argv) {c_engine->UseFullbrightShader(true);}

static void LPARAM_vk_enable_validation(const std::vector<std::string> &argv)
{
	c_engine->SetGfxAPIValidationEnabled(true);
}

static void LPARAM_render_api(const std::vector<std::string> &argv)
{
	if(argv.empty())
		return;
	c_engine->SetRenderAPI(argv.front());
}

REGISTER_LAUNCH_PARAMETER_HELP(-windowed,LPARAM_windowed,"-window -startwindowed -sw","start in windowed mode");
REGISTER_LAUNCH_PARAMETER(-window,LPARAM_windowed);
REGISTER_LAUNCH_PARAMETER(-startwindowed,LPARAM_windowed);
REGISTER_LAUNCH_PARAMETER(-sw,LPARAM_windowed);

REGISTER_LAUNCH_PARAMETER_HELP(-refresh,LPARAM_refresh,"-refreshrate -freq","monitor refresh rate in Hz. Only available in fullscreen mode");
REGISTER_LAUNCH_PARAMETER(-refreshrate,LPARAM_refresh);
REGISTER_LAUNCH_PARAMETER(-freq,LPARAM_refresh);

REGISTER_LAUNCH_PARAMETER_HELP(-noborder,LPARAM_noborder,"","When used with the game set to windowed mode, will make the game act as if in fullscreen mode (no window border).");

REGISTER_LAUNCH_PARAMETER_HELP(-w,LPARAM_w,"<width>","set the screen width");
REGISTER_LAUNCH_PARAMETER_HELP(-h,LPARAM_h,"<height>","set the screen height");

REGISTER_LAUNCH_PARAMETER_HELP(-fullbright,LPARAM_fullbright,"","start in fullbright mode");

REGISTER_LAUNCH_PARAMETER_HELP(-enable_gfx_validation,LPARAM_vk_enable_validation,"<1/0>","Enables or disables graphics API validation.");
REGISTER_LAUNCH_PARAMETER_HELP(-graphics_api,LPARAM_render_api,"<moduleName>","Changes the graphics API to use for rendering.");
