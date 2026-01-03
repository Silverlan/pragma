// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

module pragma.shared;

import :engine;

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
DLLNETWORK void LPARAM_console(const std::vector<std::string> &argv)
{
	if(!argv.empty()) {
		auto conType = pragma::Engine::ConsoleType::Terminal;
		if(pragma::string::compare<std::string>(argv[0], "guid"))
			conType = pragma::Engine::ConsoleType::GUIDetached;
		else if(pragma::string::compare<std::string>(argv[0], "gui"))
			conType = pragma::Engine::ConsoleType::GUI;
		pragma::Engine::Get()->SetConsoleType(conType);
	}
	pragma::Engine::Get()->OpenConsole();
}

DLLNETWORK void LPARAM_dev(const std::vector<std::string> &argv) { pragma::Engine::Get()->SetDeveloperMode(true); }

DLLNETWORK std::optional<std::string> g_lpLogFile = pragma::DEFAULT_LOG_FILE;
DLLNETWORK void LPARAM_log_file(const std::vector<std::string> &argv)
{
	if(argv.empty())
		g_lpLogFile = {};
	else
		g_lpLogFile = argv.front();
}

DLLNETWORK pragma::util::LogSeverity g_lpLogLevelCon = pragma::DEFAULT_CONSOLE_LOG_LEVEL;
DLLNETWORK pragma::util::LogSeverity g_lpLogLevelFile = pragma::DEFAULT_FILE_LOG_LEVEL;
DLLNETWORK void LPARAM_log(const std::vector<std::string> &argv)
{
	auto logLevelCon = pragma::DEFAULT_CONSOLE_LOG_LEVEL;
	auto logLevelFile = pragma::DEFAULT_FILE_LOG_LEVEL;
	if(argv.size() > 0) {
		logLevelCon = static_cast<pragma::util::LogSeverity>(pragma::util::to_int(argv[0]));
		if(argv.size() > 1)
			logLevelFile = static_cast<pragma::util::LogSeverity>(pragma::util::to_int(argv[1]));
		else
			logLevelFile = logLevelCon;
	}
	g_lpLogLevelCon = logLevelCon;
	g_lpLogLevelFile = logLevelFile;
}

std::string __lp_map = "";
std::string __lp_gamemode = "";
DLLNETWORK void LPARAM_map(const std::vector<std::string> &argv)
{
	if(argv.empty())
		return;
	__lp_map = argv[0];
}

DLLNETWORK void LPARAM_gamemode(const std::vector<std::string> &argv)
{
	if(argv.empty())
		return;
	__lp_gamemode = argv[0];
}

DLLNETWORK void LPARAM_tcpport(const std::vector<std::string> &argv)
{
	if(argv.empty())
		return;
	pragma::Engine::Get()->AddLaunchConVar("net_port_tcp", argv[0]); //WEAVETODO
}

DLLNETWORK void LPARAM_udpport(const std::vector<std::string> &argv)
{
	if(argv.empty())
		return;
	pragma::Engine::Get()->AddLaunchConVar("net_port_udp", argv[0]); //WEAVETODO
}

DLLNETWORK void LPARAM_tcponly(const std::vector<std::string> &argv) {} //m_bTCPOnly = true;}//WEAVETODO

DLLNETWORK void LPARAM_connect(const std::vector<std::string> &argv)
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
	//Connect(ip,port);//WEAVETODO
}

DLLNETWORK void LPARAM_USE_VKV_PARSER(const std::vector<std::string> &argv)
{
	if(argv.empty() || pragma::util::to_boolean(argv.front()))
		pragma::material::set_use_vkv_vmt_parser(true);
}

DLLNETWORK void LPARAM_help(const std::vector<std::string> &argv)
{
	pragma::Engine::Get()->OpenConsole();
	auto *map = pragma::GetLaunchParaMap();
	if(map == nullptr)
		return;
	Con::COUT << "Available parameters:" << Con::endl << map->LAUNCHPARAMETERS_HELP.str() << Con::endl;
}

DLLNETWORK std::optional<std::string> g_customTitle {};
static void LPARAM_title(const std::vector<std::string> &argv)
{
	if(!argv.empty())
		g_customTitle = argv.front();
}

DLLNETWORK pragma::util::Path g_programIcon {"materials/logo/pragma_window_icon.png"};
static void LPARAM_icon(const std::vector<std::string> &argv)
{
	if(!argv.empty())
		g_programIcon = pragma::util::FilePath(argv.front());
}

static void LPARAM_luaext(const std::vector<std::string> &argv)
{
	Lua::set_extended_lua_modules_enabled(true);
	// Lua::set_precompiled_files_enabled(false);
}

static void LPARAM_verbose(const std::vector<std::string> &argv) { pragma::Engine::Get()->SetVerbose(true); }
static void LPARAM_console_subsystem(const std::vector<std::string> &argv) { pragma::Engine::Get()->SetConsoleSubsystem(true); }
static void LPARAM_non_interactive(const std::vector<std::string> &argv) { pragma::Engine::Get()->SetNonInteractiveMode(true); }
static void LPARAM_disable_linenoise(const std::vector<std::string> &argv)
{
	// -disable_linenoise is handled in Engine constructor
}
static void LPARAM_disable_ansi_color_codes(const std::vector<std::string> &argv)
{
	pragma::logging::set_ansi_color_codes_enabled(false);
	Con::disable_ansi_color_codes();
}
bool g_lpManagedByPackageManager = false;

static void LPARAM_managed_by_package_manager(const std::vector<std::string> &argv) { g_lpManagedByPackageManager = true; }
bool g_lpSandboxed = false;
static void LPARAM_sandboxed(const std::vector<std::string> &argv) { g_lpSandboxed = true; }
DLLNETWORK std::string g_lpUserDataDir {};
static void LPARAM_user_data_dir(const std::vector<std::string> &argv)
{
	if(argv.empty())
		return;
	g_lpUserDataDir = argv.front();
}
DLLNETWORK std::vector<std::string> g_lpResourceDirs {};
static void LPARAM_resource_dir(const std::vector<std::string> &argv)
{
	for(auto &arg : argv)
		g_lpResourceDirs.insert(g_lpResourceDirs.begin(), arg);
}

void pragma::register_launch_parameters(LaunchParaMap &map)
{
	map.RegisterParameterHelp("-console", LPARAM_console, "", "start with the console open");
	map.RegisterParameterHelp("-dev", LPARAM_dev, "", "enable developer mode");
	map.RegisterParameterHelp("-log", LPARAM_log, "[<conLevel>][<fileLevel>]", "enable logging. error level can be: 0 = trace, 1 = debug, 2 = info, 3 = warning, 4 = error, 5 = critical, 6 = disabled.");
	map.RegisterParameterHelp("-log_file", LPARAM_log_file, "[<logfile>]", "file to write the log to. Default log file is log.txt");
	map.RegisterParameterHelp("-map", LPARAM_map, "<map>", "load this map on start");
	map.RegisterParameterHelp("-gamemode", LPARAM_gamemode, "<gamemode>", "load this gamemode on start");
	map.RegisterParameterHelp("-luaext", LPARAM_luaext, "", "enables several additional lua modules (e.g. package and io)");
	map.RegisterParameterHelp("-verbose", LPARAM_verbose, "", "Enables additional debug messages.");
	map.RegisterParameterHelp("-console_subsystem", LPARAM_console_subsystem, "", "should only be enabled if the executable was built for console/terminal only");
	map.RegisterParameterHelp("-non_interactive", LPARAM_non_interactive, "", "if enabled, terminal user inputs will be ignored");
	map.RegisterParameterHelp("-disable_linenoise", LPARAM_disable_linenoise, "", "if set, linenoise will not be used, which will disable auto-suggestions and hints for terminal inputs");
	map.RegisterParameterHelp("-disable_ansi_color_codes", LPARAM_disable_ansi_color_codes, "", "if set, no ansi color codes will be used in console/log outputs.");
	map.RegisterParameterHelp("-managed_by_package_manager", LPARAM_managed_by_package_manager, "", "if set, automatic updates are disabled.");
	map.RegisterParameterHelp("-sandboxed", LPARAM_sandboxed, "", "if set, indicates that the application is sandboxed, and that access to the installation files is limited.");

	map.RegisterParameterHelp("-user_data_dir", LPARAM_user_data_dir, "<path>", "the location where user data should be written to. If not specified, the installation directory will be used.");
	map.RegisterParameterHelp("-resource_dir", LPARAM_resource_dir, "<path>",
	  "the location of an additional resource directory. Pragma will be able to load assets from this location, but not write to it. You can specify multiple resource locations by using this parameter multiple times.");

	map.RegisterParameterHelp("-tcpport", LPARAM_tcpport, "<port>", "set TCP port");
	map.RegisterParameterHelp("-udpport", LPARAM_udpport, "<port>", "set UDP port");
	map.RegisterParameterHelp("-tcponly", LPARAM_tcponly, "", "use TCP for all communication");
	map.RegisterParameterHelp("-connect", LPARAM_connect, "<ip>[:<port>]", "connect to server immediately. Default port is 29150");
	map.RegisterParameterHelp("-experimental_use_vkv_parser", LPARAM_USE_VKV_PARSER, "<1/0>", "If enabled, VKV parser will be used for parsing VMT materials.");

	map.RegisterParameterHelp("-help", LPARAM_help, "-? /?", "show this help message");
	map.RegisterParameterHelp("-title", LPARAM_title, "<title>", "changes the window title");
	map.RegisterParameterHelp("-icon", LPARAM_icon, "<icon>", "changes the program icon");
	map.RegisterParameter("/?", LPARAM_help);
	map.RegisterParameter("-?", LPARAM_help);
}
