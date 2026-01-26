// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :engine;

std::optional<bool> g_launchParamWindowedMode {};
std::optional<int> g_launchParamRefreshRate {};
std::optional<bool> g_launchParamNoBorder {};
std::optional<uint32_t> g_launchParamWidth {};
std::optional<uint32_t> g_launchParamHeight {};
std::optional<Color> g_titleBarColor {};
std::optional<Color> g_borderColor {};
bool g_cpuRendering = false;
bool g_windowless = false;
bool g_cli = false;
static void LPARAM_windowed(const std::vector<std::string> &argv) { g_launchParamWindowedMode = true; }

static void LPARAM_refresh(const std::vector<std::string> &argv)
{
	if(argv.empty())
		return;
	int freq = pragma::string::to_int(argv[0]);
	if(freq > 0)
		g_launchParamRefreshRate = freq;
}

static void LPARAM_noborder(const std::vector<std::string> &argv) { g_launchParamNoBorder = true; }

static void LPARAM_w(const std::vector<std::string> &argv)
{
	if(argv.empty())
		return;
	g_launchParamWidth = pragma::string::to_int(argv[0]);
}

static void LPARAM_h(const std::vector<std::string> &argv)
{
	if(argv.empty())
		return;
	g_launchParamHeight = pragma::string::to_int(argv[0]);
}

static void LPARAM_fullbright(const std::vector<std::string> &argv) { pragma::get_cengine()->UseFullbrightShader(true); }

static void LPARAM_vk_enable_validation(const std::vector<std::string> &argv) { pragma::get_cengine()->SetGfxAPIValidationEnabled(true); }

static void LPARAM_vk_enable_gfx_diagnostics(const std::vector<std::string> &argv) { pragma::get_cengine()->SetGfxDiagnosticsModeEnabled(true); }

void LPARAM_enable_gfx_api_dump(const std::vector<std::string> &argv);

static void LPARAM_render_api(const std::vector<std::string> &argv)
{
	if(argv.empty())
		return;
	pragma::get_cengine()->SetRenderAPI(argv.front());
}

static void LPARAM_audio_api(const std::vector<std::string> &argv)
{
	if(argv.empty())
		return;
	pragma::get_cengine()->SetAudioAPI(argv.front());
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

static void LPARAM_windowless(const std::vector<std::string> &argv)
{
	auto windowless = true;
	if(!argv.empty())
		windowless = pragma::util::to_boolean(argv.front());
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

static void LPARAM_cpu_rendering(const std::vector<std::string> &argv) { g_cpuRendering = (argv.empty() || pragma::util::to_boolean(argv.front())); }

static void LPARAM_cli(const std::vector<std::string> &argv)
{
	g_cli = true;
	LPARAM_cpu_rendering(argv);
	LPARAM_windowless(argv);
	if(argv.empty() || pragma::util::to_boolean(argv.front()))
		LPARAM_audio_api({"dummy"});
}

extern std::optional<std::string> g_waylandLibdecorPlugin;
static void LPARAM_wayland_libdecor_plugin(const std::vector<std::string> &argv)
{
	if(argv.empty())
		return;
	g_waylandLibdecorPlugin = argv.front();
}

void pragma::register_client_launch_parameters(LaunchParaMap &map)
{
	map.RegisterParameterHelp("-windowed", &LPARAM_windowed, "-window -startwindowed -sw", "start in windowed mode");
	map.RegisterParameter("-window", &LPARAM_windowed);
	map.RegisterParameter("-startwindowed", &LPARAM_windowed);
	map.RegisterParameter("-sw", &LPARAM_windowed);

	map.RegisterParameterHelp("-refresh", &LPARAM_refresh, "-refreshrate -freq", "monitor refresh rate in Hz. Only available in fullscreen mode");
	map.RegisterParameter("-refreshrate", &LPARAM_refresh);
	map.RegisterParameter("-freq", &LPARAM_refresh);

	map.RegisterParameterHelp("-noborder", &LPARAM_noborder, "", "When used with the game set to windowed mode, will make the game act as if in fullscreen mode (no window border).");

	map.RegisterParameterHelp("-w", &LPARAM_w, "<width>", "set the screen width");
	map.RegisterParameterHelp("-h", &LPARAM_h, "<height>", "set the screen height");

	map.RegisterParameterHelp("-fullbright", &LPARAM_fullbright, "", "start in fullbright mode");

	map.RegisterParameterHelp("-enable_gfx_api_dump", &LPARAM_enable_gfx_api_dump, "<1/0>", "Enables or disables graphics API dump.");
	map.RegisterParameterHelp("-enable_gfx_validation", &LPARAM_vk_enable_validation, "<1/0>", "Enables or disables graphics API validation.");
	map.RegisterParameterHelp("-enable_gfx_diagnostics", &LPARAM_vk_enable_gfx_diagnostics, "<1/0>", "Enables or disables GPU diagnostics mode.");
	map.RegisterParameterHelp("-graphics_api", &LPARAM_render_api, "<moduleName>", "Changes the graphics API to use for rendering.");
	map.RegisterParameterHelp("-audio_api", &LPARAM_audio_api, "<moduleName>", "Changes the audio API to use for audio playback.");
	map.RegisterParameterHelp("-auto_exec", &LPARAM_auto_exec, "<script>", "Auto-execute this Lua-script on launch.");
	map.RegisterParameterHelp("-windowless", &LPARAM_windowless, "<1/0>", "If enabled, Pragma will be launched without a visible window.");
	map.RegisterParameterHelp("-title_bar_color", &LPARAM_title_bar_color, "<hexColor>", "Hex color for the window title bar.");
	map.RegisterParameterHelp("-border_color", &LPARAM_border_bar_color, "<hexColor>", "Hex color for the window border.");
	map.RegisterParameterHelp("-cpu_rendering", &LPARAM_cpu_rendering, "<1/0>", "If enabled, the CPU will be used for rendering instead of GPU.");
	map.RegisterParameterHelp("-cli", &LPARAM_cli, "<1/0>", "If enabled, will automatically enable the options needed to run Pragma in a command-line-interface-only environment.");
	map.RegisterParameterHelp("-wayland_libdecor_plugin", &LPARAM_wayland_libdecor_plugin, "", "If specified, this libdecor plugin will be used for window decoration drawing on Linux with wayland.");
}
