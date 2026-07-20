// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

module pragma.shared;

import :engine;
import pragma.udm;

pragma::Engine::LaunchCommand::LaunchCommand(const std::string &cmd, const std::vector<std::string> &_args) : command(cmd), args(_args) {}

void pragma::Engine::InitLaunchOptions(int argc, char *argv[])
{
	auto &parameters = GetLaunchParaMap()->GetParameters();
	std::vector<std::string> launchCmdArgs {};
	struct LaunchOption {
		std::string cmd;
		std::vector<std::string> argv;
	};

	std::unordered_set<std::string> consoleDependentOptions {"-console_subsystem", "-non_interactive"};

	std::vector<LaunchOption> launchOptions;
	for(auto i = argc - 1; i > 0; --i) {
		std::string arg = argv[i];
		if(arg.empty())
			continue;
		auto token = arg.front();
		if(token != '-' && token != '+') {
			launchCmdArgs.insert(launchCmdArgs.begin(), arg);
			continue;
		}
		string::to_lower(arg);
		switch(token) {
		case '-':
			{
				auto it = parameters.find(arg);
				if(it != parameters.end()) {
					LaunchOption lo {};
					lo.cmd = arg;
					lo.argv = std::move(launchCmdArgs);

					if(consoleDependentOptions.find(arg) == consoleDependentOptions.end() && arg != "-log")
						launchOptions.push_back(lo);
					else // Console-dependent options need to be executed before -console, so we move them to the front. -log should also always be first
						launchOptions.insert(launchOptions.begin(), lo);
				}
				else
					spdlog::warn("Unknown launch option '{}' specified. Ignoring...", arg);
				break;
			}
		case '+':
			{
				std::vector<std::string> subArgs;
				string::explode_whitespace(arg, subArgs);
				if(subArgs.empty() == false) {
					arg = subArgs.front();
					subArgs.erase(subArgs.begin());
				}
				launchCmdArgs.reserve(launchCmdArgs.size() + subArgs.size());
				for(auto &arg : subArgs)
					launchCmdArgs.push_back(arg);
				m_launchCommands.push_back({arg.substr(1), launchCmdArgs});
				break;
			}
		}
		launchCmdArgs.clear();
	}

	for(auto &lo : launchOptions) {
		auto it = parameters.find(lo.cmd);
		if(it == parameters.end())
			continue;
		auto &f = it->second;
		spdlog::debug("Initializing launch option '{}'...", lo.cmd);
		f(lo.argv);
	}

	if(is_log_level_enabled(util::LogSeverity::Debug)) {
		std::vector<std::string> gameCmds;
		gameCmds.reserve(m_launchCommands.size());
		for(auto &cmd : m_launchCommands)
			gameCmds.push_back(cmd.command);
		spdlog::debug("{} game commands have been queued: {}", m_launchCommands.size(), string::implode(gameCmds, ", "));
	}
}

//////////////////////////////

extern pragma::Engine *engine;

static pragma::LaunchSettings &get_launch_settings() { return pragma::get_engine()->GetLaunchSettings(); }

namespace pragma::launch_options {
	static void console(const std::vector<std::string> &argv)
	{
		if(!argv.empty()) {
			auto conType = Engine::ConsoleType::Terminal;
			if(string::compare<std::string>(argv[0], "guid"))
				conType = Engine::ConsoleType::GUIDetached;
			else if(string::compare<std::string>(argv[0], "gui"))
				conType = Engine::ConsoleType::GUI;
			Engine::Get()->SetConsoleType(conType);
		}
		Engine::Get()->OpenConsole();
	}

	static void dev(const std::vector<std::string> &argv) { Engine::Get()->SetDeveloperMode(true); }

	static void log_file(const std::vector<std::string> &argv) { get_launch_settings().Set("log_file", !argv.empty() ? argv.front() : ""); }

	static void log(const std::vector<std::string> &argv)
	{
		auto logLevelCon = DEFAULT_CONSOLE_LOG_LEVEL;
		auto logLevelFile = DEFAULT_FILE_LOG_LEVEL;
		if(argv.size() > 0) {
			logLevelCon = static_cast<util::LogSeverity>(util::to_int(argv[0]));
			if(argv.size() > 1)
				logLevelFile = static_cast<util::LogSeverity>(util::to_int(argv[1]));
			else
				logLevelFile = logLevelCon;
		}
		get_launch_settings().Set<util::LogSeverity>("log_level_console", logLevelCon);
		get_launch_settings().Set<util::LogSeverity>("log_level_file", logLevelFile);
	}

	static void map(const std::vector<std::string> &argv)
	{
		if(argv.empty())
			return;
		get_launch_settings().Set("map", argv.front());
	}

	static void gamemode(const std::vector<std::string> &argv)
	{
		if(argv.empty())
			return;
		get_launch_settings().Set("gamemode", argv.front());
	}

	static void connect(const std::vector<std::string> &argv)
	{
		if(argv.empty())
			return;
		std::string arg = argv[0];
		std::string ip, port;
		size_t i = arg.find(':');
		if(i == size_t(-1)) {
			ip = arg;
			port = "29150";
		}
		else {
			ip = arg.substr(0, i);
			port = arg.substr(i + 1, arg.length());
		}
		Con::CERR << "connect launch option is currently not implemented!" << Con::endl;
	}

	static void use_vkv_parser(const std::vector<std::string> &argv)
	{
		if(argv.empty() || util::to_boolean(argv.front()))
			material::set_use_vkv_vmt_parser(true);
	}

	static void help(const std::vector<std::string> &argv)
	{
		Engine::Get()->OpenConsole();
		auto *map = GetLaunchParaMap();
		if(map == nullptr)
			return;
		Con::COUT << "Available parameters:" << Con::endl << map->LAUNCHPARAMETERS_HELP.str() << Con::endl;
	}

	static void title(const std::vector<std::string> &argv)
	{
		if(!argv.empty())
			get_launch_settings().Set("title", argv.front());
	}

	static void icon(const std::vector<std::string> &argv)
	{
		if(!argv.empty())
			get_launch_settings().Set("icon", argv.front());
	}

	static void luaext(const std::vector<std::string> &argv)
	{
		Lua::set_extended_lua_modules_enabled(true);
		// Lua::set_precompiled_files_enabled(false);
	}

	static void verbose(const std::vector<std::string> &argv) { Engine::Get()->SetVerbose(true); }
	static void console_subsystem(const std::vector<std::string> &argv) { Engine::Get()->SetConsoleSubsystem(true); }
	static void non_interactive(const std::vector<std::string> &argv) { Engine::Get()->SetNonInteractiveMode(true); }
	static void disable_linenoise(const std::vector<std::string> &argv)
	{
		// -disable_linenoise is handled in Engine constructor
	}

	static void disable_ansi_color_codes(const std::vector<std::string> &argv)
	{
		logging::set_ansi_color_codes_enabled(false);
		Con::disable_ansi_color_codes();
	}

	static void managed_by_package_manager(const std::vector<std::string> &argv)
	{
		get_launch_settings().Set("managed_by_package_manager", true);
	}

	static void sandboxed(const std::vector<std::string> &argv)
	{
		get_launch_settings().Set("sandboxed", true);
	}

	static void user_data_dir(const std::vector<std::string> &argv)
	{
		if(argv.empty())
			return;
		get_launch_settings().Set("user_data_dir", util::DirPath(argv.front()).GetString());
	}

	static void resource_dir(const std::vector<std::string> &argv)
	{
		auto prop = get_launch_settings().GetProperty("resource_dirs");
		if(!prop) {
			prop = udm::Property::Create<udm::Array>();
			prop->GetValue<udm::Array>().SetValueType(udm::Type::String);
			get_launch_settings().SetProperty("resource_dirs", prop);
		}

		auto &a = prop->GetValue<udm::Array>();
		for(auto &arg : argv)
			a.InsertValue(0, arg);
	}
}

void pragma::register_launch_parameters(LaunchParaMap &map)
{
	map.RegisterParameterHelp("-console", launch_options::console, "", "start with the console open");
	map.RegisterParameterHelp("-dev", launch_options::dev, "", "enable developer mode");

	get_launch_settings().Set<pragma::util::LogSeverity>("log_level_console", pragma::DEFAULT_CONSOLE_LOG_LEVEL);
	get_launch_settings().Set<pragma::util::LogSeverity>("log_level_file", pragma::DEFAULT_FILE_LOG_LEVEL);
	map.RegisterParameterHelp("-log", launch_options::log, "[<conLevel>][<fileLevel>]", "enable logging. error level can be: 0 = trace, 1 = debug, 2 = info, 3 = warning, 4 = error, 5 = critical, 6 = disabled.");

	get_launch_settings().Set("log_file", pragma::DEFAULT_LOG_FILE);
	map.RegisterParameterHelp("-log_file", launch_options::log_file, "[<logfile>]", "file to write the log to. Default log file is log.txt");

	map.RegisterParameterHelp("-map", launch_options::map, "<map>", "load this map on start");
	map.RegisterParameterHelp("-gamemode", launch_options::gamemode, "<gamemode>", "load this gamemode on start");
	map.RegisterParameterHelp("-luaext", launch_options::luaext, "", "enables several additional lua modules (e.g. package and io)");
	map.RegisterParameterHelp("-verbose", launch_options::verbose, "", "Enables additional debug messages.");
	map.RegisterParameterHelp("-console_subsystem", launch_options::console_subsystem, "", "should only be enabled if the executable was built for console/terminal only");
	map.RegisterParameterHelp("-non_interactive", launch_options::non_interactive, "", "if enabled, terminal user inputs will be ignored");
	map.RegisterParameterHelp("-disable_linenoise", launch_options::disable_linenoise, "", "if set, linenoise will not be used, which will disable auto-suggestions and hints for terminal inputs");
	map.RegisterParameterHelp("-disable_ansi_color_codes", launch_options::disable_ansi_color_codes, "", "if set, no ansi color codes will be used in console/log outputs.");
	map.RegisterParameterHelp("-managed_by_package_manager", launch_options::managed_by_package_manager, "", "if set, automatic updates are disabled.");
	map.RegisterParameterHelp("-sandboxed", launch_options::sandboxed, "", "if set, indicates that the application is sandboxed, and that access to the installation files is limited.");

	map.RegisterParameterHelp("-user_data_dir", launch_options::user_data_dir, "<path>", "the location where user data should be written to. If not specified, the installation directory will be used.");
	map.RegisterParameterHelp("-resource_dir", launch_options::resource_dir, "<path>",
	  "the location of an additional resource directory. Pragma will be able to load assets from this location, but not write to it. You can specify multiple resource locations by using this parameter multiple times.");

	map.RegisterParameterHelp("-connect", launch_options::connect, "<ip>[:<port>]", "connect to server immediately. Default port is 29150");
	map.RegisterParameterHelp("-experimental_use_vkv_parser", launch_options::use_vkv_parser, "<1/0>", "If enabled, VKV parser will be used for parsing VMT materials.");

	map.RegisterParameterHelp("-help", launch_options::help, "-? /?", "show this help message");
	map.RegisterParameterHelp("-title", launch_options::title, "<title>", "changes the window title");

	get_launch_settings().Set("icon", "materials/logo/pragma_window_icon.png");
	map.RegisterParameterHelp("-icon", launch_options::icon, "<icon>", "changes the program icon");

	map.RegisterParameter("/?", launch_options::help);
	map.RegisterParameter("-?", launch_options::help);
}
