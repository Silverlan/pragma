/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_cengine.h"
#include "pragma/launchparameters.h"
#include "pragma/c_engine.h"

extern DLLCLIENT CEngine *c_engine;
std::optional<bool> g_launchParamWindowedMode {};
std::optional<int> g_launchParamRefreshRate {};
std::optional<bool> g_launchParamNoBorder {};
std::optional<uint32_t> g_launchParamWidth {};
std::optional<uint32_t> g_launchParamHeight {};
std::optional<Color> g_titleBarColor {};
std::optional<Color> g_borderColor {};
bool g_launchParamExperimentalMemoryOptimizationEnabled = false;
bool g_cpuRendering = false;
bool g_windowless = false;
static void LPARAM_windowed(const std::vector<std::string> &argv) { g_launchParamWindowedMode = true; }

static void LPARAM_refresh(const std::vector<std::string> &argv)
{
	if(argv.empty())
		return;
	int freq = atoi(argv[0].c_str());
	if(freq > 0)
		g_launchParamRefreshRate = freq;
}

static void LPARAM_noborder(const std::vector<std::string> &argv) { g_launchParamNoBorder = true; }

static void LPARAM_w(const std::vector<std::string> &argv)
{
	if(argv.empty())
		return;
	g_launchParamWidth = atoi(argv[0].c_str());
}

static void LPARAM_h(const std::vector<std::string> &argv)
{
	if(argv.empty())
		return;
	g_launchParamHeight = atoi(argv[0].c_str());
}

static void LPARAM_fullbright(const std::vector<std::string> &argv) { c_engine->UseFullbrightShader(true); }

static void LPARAM_vk_enable_validation(const std::vector<std::string> &argv) { c_engine->SetGfxAPIValidationEnabled(true); }

static void LPARAM_render_api(const std::vector<std::string> &argv)
{
	if(argv.empty())
		return;
	c_engine->SetRenderAPI(argv.front());
}

static void LPARAM_audio_api(const std::vector<std::string> &argv)
{
	if(argv.empty())
		return;
	c_engine->SetAudioAPI(argv.front());
}

extern std::optional<std::vector<std::string>> g_autoExecScripts;
static void LPARAM_auto_exec(const std::vector<std::string> &argv)
{
	if(argv.empty())
		return;
	if(!g_autoExecScripts)
		g_autoExecScripts = std::vector<std::string> {};
	for(auto &arg : argv)
		g_autoExecScripts->push_back(arg);
}

extern std::optional<std::string> g_customWindowIcon;
static void LPARAM_icon(const std::vector<std::string> &argv)
{
	if(argv.empty())
		return;
	g_customWindowIcon = argv.front();
}

static void LPARAM_windowless(const std::vector<std::string> &argv)
{
	auto windowless = true;
	if(!argv.empty())
		windowless = util::to_boolean(argv.front());
	g_windowless = windowless;
}

static void LPARAM_title_bar_color(const std::vector<std::string> &argv)
{
	if(argv.empty())
		return;
	auto strHex = argv.front();
	if(!strHex.empty() && strHex.front() == '#')
		strHex.erase(strHex.begin());
	g_titleBarColor = Color::CreateFromHexColor(strHex);
}

static void LPARAM_border_bar_color(const std::vector<std::string> &argv)
{
	if(argv.empty())
		return;
	auto strHex = argv.front();
	if(!strHex.empty() && strHex.front() == '#')
		strHex.erase(strHex.begin());
	g_borderColor = Color::CreateFromHexColor(strHex);
}

static void LPARAM_EXPERIMENTAL_MEMORY_OPTIMIZATION(const std::vector<std::string> &argv) { g_launchParamExperimentalMemoryOptimizationEnabled = (argv.empty() || util::to_boolean(argv.front())); }

static void LPARAM_cpu_rendering(const std::vector<std::string> &argv)
{
	g_cpuRendering = (argv.empty() || util::to_boolean(argv.front()));
	// Without optimizations enabled, loading with CPU rendering will take a very long time
	g_launchParamExperimentalMemoryOptimizationEnabled = true;
}

REGISTER_LAUNCH_PARAMETER_HELP(-windowed, LPARAM_windowed, "-window -startwindowed -sw", "start in windowed mode");
REGISTER_LAUNCH_PARAMETER(-window, LPARAM_windowed);
REGISTER_LAUNCH_PARAMETER(-startwindowed, LPARAM_windowed);
REGISTER_LAUNCH_PARAMETER(-sw, LPARAM_windowed);

REGISTER_LAUNCH_PARAMETER_HELP(-refresh, LPARAM_refresh, "-refreshrate -freq", "monitor refresh rate in Hz. Only available in fullscreen mode");
REGISTER_LAUNCH_PARAMETER(-refreshrate, LPARAM_refresh);
REGISTER_LAUNCH_PARAMETER(-freq, LPARAM_refresh);

REGISTER_LAUNCH_PARAMETER_HELP(-noborder, LPARAM_noborder, "", "When used with the game set to windowed mode, will make the game act as if in fullscreen mode (no window border).");

REGISTER_LAUNCH_PARAMETER_HELP(-w, LPARAM_w, "<width>", "set the screen width");
REGISTER_LAUNCH_PARAMETER_HELP(-h, LPARAM_h, "<height>", "set the screen height");

REGISTER_LAUNCH_PARAMETER_HELP(-fullbright, LPARAM_fullbright, "", "start in fullbright mode");

REGISTER_LAUNCH_PARAMETER_HELP(-enable_gfx_validation, LPARAM_vk_enable_validation, "<1/0>", "Enables or disables graphics API validation.");
REGISTER_LAUNCH_PARAMETER_HELP(-graphics_api, LPARAM_render_api, "<moduleName>", "Changes the graphics API to use for rendering.");
REGISTER_LAUNCH_PARAMETER_HELP(-audio_api, LPARAM_audio_api, "<moduleName>", "Changes the audio API to use for audio playback.");
REGISTER_LAUNCH_PARAMETER_HELP(-auto_exec, LPARAM_auto_exec, "<script>", "Auto-execute this Lua-script on launch.");
REGISTER_LAUNCH_PARAMETER_HELP(-icon, LPARAM_icon, "<iconPath>", "Path to custom window icon location.");
REGISTER_LAUNCH_PARAMETER_HELP(-windowless, LPARAM_windowless, "<1/0>", "If enabled, Pragma will be launched without a visible window.");
REGISTER_LAUNCH_PARAMETER_HELP(-title_bar_color, LPARAM_title_bar_color, "<hexColor>", "Hex color for the window title bar.");
REGISTER_LAUNCH_PARAMETER_HELP(-border_color, LPARAM_border_bar_color, "<hexColor>", "Hex color for the window border.");
REGISTER_LAUNCH_PARAMETER_HELP(-experimental_memory_optimization, LPARAM_EXPERIMENTAL_MEMORY_OPTIMIZATION, "<1/0>", "Enables experimental code for RAM usage reduction.");
REGISTER_LAUNCH_PARAMETER_HELP(-cpu_rendering, LPARAM_cpu_rendering, "<1/0>", "If enabled, the CPU will be used for rendering instead of GPU.");
