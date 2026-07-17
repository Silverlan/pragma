// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :engine;

static pragma::LaunchSettings &get_launch_settings() { return pragma::get_engine()->GetLaunchSettings(); }

namespace pragma::launch_options {
	static void windowed(const std::vector<std::string> &argv) { get_launch_settings().Set("windowed", true); }

	static void refresh(const std::vector<std::string> &argv)
	{
		if(argv.empty())
			return;
		int freq = string::to_int(argv[0]);
		if(freq > 0)
			get_launch_settings().Set("windowed", freq);
	}

	static void noborder(const std::vector<std::string> &argv) { get_launch_settings().Set("noborder", true); }

	static void w(const std::vector<std::string> &argv)
	{
		if(argv.empty())
			return;
		get_launch_settings().Set("w", string::to_int(argv[0]));
	}

	static void h(const std::vector<std::string> &argv)
	{
		if(argv.empty())
			return;
		get_launch_settings().Set("h", string::to_int(argv[0]));
	}

	static void fullbright(const std::vector<std::string> &argv) { get_cengine()->UseFullbrightShader(true); }

	static void enable_gfx_validation(const std::vector<std::string> &argv) { get_cengine()->SetGfxAPIValidationEnabled(true); }

	static void enable_gfx_diagnostics(const std::vector<std::string> &argv) { get_cengine()->SetGfxDiagnosticsModeEnabled(true); }

	static void render_api(const std::vector<std::string> &argv)
	{
		if(argv.empty())
			return;
		get_cengine()->SetRenderAPI(argv.front());
	}

	static void audio_api(const std::vector<std::string> &argv)
	{
		if(argv.empty())
			return;
		get_cengine()->SetAudioAPI(argv.front());
	}

	static void windowless(const std::vector<std::string> &argv)
	{
		auto windowless = true;
		if(!argv.empty())
			windowless = util::to_boolean(argv.front());
		get_launch_settings().Set("windowless", windowless);
	}

	static void force_single_threaded_mode(const std::vector<std::string> &argv)
	{
		auto forceSingleThreadedMode = true;
		if(!argv.empty())
			forceSingleThreadedMode = util::to_boolean(argv.front());
		get_launch_settings().Set("force_single_threaded_mode", forceSingleThreadedMode);
	}

	static void wait_idle_between_frames(const std::vector<std::string> &argv)
	{
		auto waitIdleBetweenFrames = true;
		if(!argv.empty())
			waitIdleBetweenFrames = util::to_boolean(argv.front());
		get_launch_settings().Set("wait_idle_between_frames", waitIdleBetweenFrames);
	}

	static void wayland_libdecor_plugin(const std::vector<std::string> &argv)
	{
		if(argv.empty())
			return;
		get_launch_settings().Set("wayland_libdecor_plugin", argv.front());
	}

	static void cpu_rendering(const std::vector<std::string> &argv) { get_launch_settings().Set("cpu_rendering", argv.empty() || pragma::util::to_boolean(argv.front())); }

	static void title_bar_color(const std::vector<std::string> &argv)
	{
		if(argv.empty())
			return;
		auto strHex = argv.front();
		if(!strHex.empty() && strHex.front() == '#')
			strHex.erase(strHex.begin());
		get_launch_settings().Set("title_bar_color", strHex);
	}

	static void border_bar_color(const std::vector<std::string> &argv)
	{
		if(argv.empty())
			return;
		auto strHex = argv.front();
		if(!strHex.empty() && strHex.front() == '#')
			strHex.erase(strHex.begin());
		get_launch_settings().Set("border_bar_color", strHex);
	}

	static void disable_global_shader_file_cache(const std::vector<std::string> &argv)
	{
		if(argv.empty())
			return;
		prosper::glsl::set_global_include_file_cache_enabled(pragma::util::to_boolean(argv.front()));
	}

	static void cli(const std::vector<std::string> &argv)
	{
		get_launch_settings().Set("cli", true);
		cpu_rendering(argv);
		windowless(argv);
		if(argv.empty() || pragma::util::to_boolean(argv.front()))
			audio_api({"dummy"});
	}

	static void window_backend(const std::vector<std::string> &argv)
	{
		if(argv.empty()) {
			get_launch_settings().Set<platform::Platform>("window_backend", platform::Platform::Unknown);
			return;
		}

		auto platform = magic_enum::enum_cast<platform::Platform>(argv.front());
		if(!platform) {
			Con::CERR << std::format("Unknown platform {}. Valid platform values are:\n", argv.front());
			auto values = magic_enum::enum_values<platform::Platform>();
			for(auto val : values) {
				if(val == platform::Platform::Count || val == platform::Platform::Unknown)
					continue;
				Con::CERR << magic_enum::enum_name(val) << "\n";
			}
			Con::CERR << Con::endl;
			return;
		}
		get_launch_settings().Set<platform::Platform>("window_backend", *platform);
	}

	static void auto_exec(const std::vector<std::string> &argv)
	{
		if(argv.empty())
			return;
		auto prop = get_launch_settings().GetProperty("auto_exec_scripts");
		if(!prop) {
			prop = udm::Property::Create<udm::Array>();
			prop->GetValue<udm::Array>().SetValueType(udm::Type::String);
			get_launch_settings().SetProperty("auto_exec_scripts", prop);
		}

		auto &a = prop->GetValue<udm::Array>();
		auto offset = a.GetSize();
		a.Resize(a.GetSize() +argv.size());
		for(auto &arg : argv)
			a[offset++] = arg;

		for(auto &arg : argv)
			a.InsertValue(0, arg);
	}
}

namespace pragma::debug {
	void enable_gfx_api_dump(const std::vector<std::string> &argv);
}

void pragma::register_client_launch_parameters(LaunchParaMap &map)
{
	map.RegisterParameterHelp("-windowed", &launch_options::windowed, "-window -startwindowed -sw", "start in windowed mode");
	map.RegisterParameter("-window", &launch_options::windowed);
	map.RegisterParameter("-startwindowed", &launch_options::windowed);
	map.RegisterParameter("-sw", &launch_options::windowed);

	map.RegisterParameterHelp("-refresh", &launch_options::refresh, "-refreshrate -freq", "monitor refresh rate in Hz. Only available in fullscreen mode");
	map.RegisterParameter("-refreshrate", &launch_options::refresh);
	map.RegisterParameter("-freq", &launch_options::refresh);

	map.RegisterParameterHelp("-noborder", &launch_options::noborder, "", "When used with the game set to windowed mode, will make the game act as if in fullscreen mode (no window border).");

	map.RegisterParameterHelp("-w", &launch_options::w, "<width>", "set the screen width");
	map.RegisterParameterHelp("-h", &launch_options::h, "<height>", "set the screen height");

	map.RegisterParameterHelp("-fullbright", &launch_options::fullbright, "", "start in fullbright mode");

	map.RegisterParameterHelp("-enable_global_shader_file_cache", &launch_options::disable_global_shader_file_cache, "<1/0>", "Enables or disables the global shader file cache. Disable this if you're working on shader files and need to reload them during runtime.");
	map.RegisterParameterHelp("-enable_gfx_api_dump", &debug::enable_gfx_api_dump, "<1/0>", "Enables or disables graphics API dump.");
	map.RegisterParameterHelp("-enable_gfx_validation", &launch_options::enable_gfx_validation, "<1/0>", "Enables or disables graphics API validation.");
	map.RegisterParameterHelp("-enable_gfx_diagnostics", &launch_options::enable_gfx_diagnostics, "<1/0>", "Enables or disables GPU diagnostics mode.");
	map.RegisterParameterHelp("-graphics_api", &launch_options::render_api, "<moduleName>", "Changes the graphics API to use for rendering.");
	map.RegisterParameterHelp("-audio_api", &launch_options::audio_api, "<moduleName>", "Changes the audio API to use for audio playback.");
	map.RegisterParameterHelp("-auto_exec", &launch_options::auto_exec, "<script>", "Auto-execute this Lua-script on launch.");
	map.RegisterParameterHelp("-windowless", &launch_options::windowless, "<1/0>", "If enabled, Pragma will be launched without a visible window.");
	map.RegisterParameterHelp("-force_single_threaded_rendering", &launch_options::force_single_threaded_mode, "<1/0>", "If enabled, render command buffers will be filled sequentially. This will significantly hurt frame rate and should only be used for debugging purposes.");
	map.RegisterParameterHelp("-wait_idle_between_frames", &launch_options::wait_idle_between_frames, "<1/0>", "If enabled, the GPU will be forced to wait idle between frames, clearing out the queue. This will significantly hurt frame rate and should only be used for debugging purposes.");
	map.RegisterParameterHelp("-title_bar_color", &launch_options::title_bar_color, "<hexColor>", "Hex color for the window title bar.");
	map.RegisterParameterHelp("-border_color", &launch_options::border_bar_color, "<hexColor>", "Hex color for the window border.");
	map.RegisterParameterHelp("-cpu_rendering", &launch_options::cpu_rendering, "<1/0>", "If enabled, the CPU will be used for rendering instead of GPU.");
	map.RegisterParameterHelp("-window_backend", &launch_options::window_backend, "<Win32/Cocoa/Wayland/X11/Windowless>", "Uses the specified window backend system for window creation.");
	map.RegisterParameterHelp("-cli", &launch_options::cli, "<1/0>", "If enabled, will automatically enable the options needed to run Pragma in a command-line-interface-only environment.");
	map.RegisterParameterHelp("-wayland_libdecor_plugin", &launch_options::wayland_libdecor_plugin, "", "If specified, this libdecor plugin will be used for window decoration drawing on Linux with wayland.");
}
