/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/launchparameters.h"
#include "pragma/engine.h"
#include "pragma/console/conout.h"
#include "pragma/logging_wrapper.hpp"

Engine::LaunchCommand::LaunchCommand(const std::string &cmd, const std::vector<std::string> &_args) : command(cmd), args(_args) {}

std::stringstream LaunchParaMap::LAUNCHPARAMETERS_HELP;

const std::unordered_map<std::string, std::function<void(const std::vector<std::string> &)>> &LaunchParaMap::GetParameters() const { return m_parameters; }
void LaunchParaMap::RegisterParameter(const std::string &name, const std::function<void(const std::vector<std::string> &)> &f) { m_parameters.insert(std::make_pair(name, f)); }

DLLNETWORK LaunchParaMap *g_LaunchParameters = NULL;

DLLNETWORK LaunchParaMap *GetLaunchParaMap() { return g_LaunchParameters; }

DLLNETWORK void RegisterLaunchParameter(std::string name, const std::function<void(const std::vector<std::string> &)> &function)
{
	if(g_LaunchParameters == NULL) {
		static LaunchParaMap map;
		g_LaunchParameters = &map;
	}
	g_LaunchParameters->RegisterParameter(name, function);
}

DLLNETWORK void RegisterLaunchParameterHelp(std::string name, const std::function<void(const std::vector<std::string> &)> &function, std::string descCmd, std::string descHelp)
{
	RegisterLaunchParameter(name, function);
	std::stringstream pre;
	pre << "\t" << name << " " << descCmd;
	std::string sp = "";
	int l = 22 - static_cast<int>(pre.str().length());
	if(l < 3) {
		pre << "\n\t";
		l = 21;
	}
	for(int j = 0; j < l; j++)
		sp += " ";
	g_LaunchParameters->LAUNCHPARAMETERS_HELP << pre.str() << sp << descHelp << "\n";
}

void Engine::InitLaunchOptions(int argc, char *argv[])
{
	auto &parameters = g_LaunchParameters->GetParameters();
	std::vector<std::string> launchCmdArgs {};
	for(auto i = argc - 1; i > 0; --i) {
		std::string arg = argv[i];
		if(arg.empty())
			continue;
		auto token = arg.front();
		if(token != '-' && token != '+') {
			launchCmdArgs.insert(launchCmdArgs.begin(), arg);
			continue;
		}
		ustring::to_lower(arg);
		switch(token) {
		case '-':
			{
				auto it = parameters.find(arg);
				if(it != parameters.end()) {
					auto &f = it->second;
					f(launchCmdArgs);
				}
				break;
			}
		case '+':
			{
				std::vector<std::string> subArgs;
				ustring::explode_whitespace(arg, subArgs);
				if(subArgs.empty() == false) {
					arg = subArgs.front();
					subArgs.erase(subArgs.begin());
				}
				launchCmdArgs.reserve(launchCmdArgs.size() + subArgs.size());
				for(auto &arg : subArgs)
					launchCmdArgs.insert(launchCmdArgs.begin(), arg);
				m_launchCommands.push_back({arg.substr(1), launchCmdArgs});
				break;
			}
		}
		launchCmdArgs.clear();
	}
}

//////////////////////////////

extern Engine *engine;
DLLNETWORK void LPARAM_console(const std::vector<std::string> &argv)
{
	if(!argv.empty()) {
		auto conType = Engine::ConsoleType::Terminal;
		if(ustring::compare<std::string>(argv[0], "guid"))
			conType = Engine::ConsoleType::GUIDetached;
		else if(ustring::compare<std::string>(argv[0], "gui"))
			conType = Engine::ConsoleType::GUI;
		engine->SetConsoleType(conType);
	}
	engine->OpenConsole();
}

DLLNETWORK void LPARAM_dev(const std::vector<std::string> &argv) { engine->SetDeveloperMode(true); }

DLLNETWORK std::optional<std::string> g_lpLogFile = pragma::DEFAULT_LOG_FILE;
DLLNETWORK void LPARAM_log_file(const std::vector<std::string> &argv)
{
	if(argv.empty())
		g_lpLogFile = {};
	else
		g_lpLogFile = argv.front();
}

DLLNETWORK util::LogSeverity g_lpLogLevelCon = pragma::DEFAULT_CONSOLE_LOG_LEVEL;
DLLNETWORK util::LogSeverity g_lpLogLevelFile = pragma::DEFAULT_FILE_LOG_LEVEL;
DLLNETWORK void LPARAM_log(const std::vector<std::string> &argv)
{
	auto logLevelCon = pragma::DEFAULT_CONSOLE_LOG_LEVEL;
	auto logLevelFile = pragma::DEFAULT_FILE_LOG_LEVEL;
	if(argv.size() > 0) {
		logLevelCon = static_cast<util::LogSeverity>(util::to_int(argv[0]));
		if(argv.size() > 1)
			logLevelFile = static_cast<util::LogSeverity>(util::to_int(argv[1]));
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
	engine->AddLaunchConVar("net_port_tcp", argv[0]); //WEAVETODO
}

DLLNETWORK void LPARAM_udpport(const std::vector<std::string> &argv)
{
	if(argv.empty())
		return;
	engine->AddLaunchConVar("net_port_udp", argv[0]); //WEAVETODO
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

DLLNETWORK void LPARAM_help(const std::vector<std::string> &argv)
{
	engine->OpenConsole();
	LaunchParaMap *map = GetLaunchParaMap();
	if(map == NULL)
		return;
	Con::cout << "Available parameters:" << Con::endl << map->LAUNCHPARAMETERS_HELP.str() << Con::endl;
}

DLLNETWORK std::optional<std::string> g_customTitle {};
static void LPARAM_title(const std::vector<std::string> &argv)
{
	if(!argv.empty())
		g_customTitle = argv.front();
}

static void LPARAM_luaext(const std::vector<std::string> &argv)
{
	Lua::set_extended_lua_modules_enabled(true);
	// Lua::set_precompiled_files_enabled(false);
}

static void LPARAM_verbose(const std::vector<std::string> &argv) { engine->SetVerbose(true); }

REGISTER_LAUNCH_PARAMETER_HELP(-console, LPARAM_console, "", "start with the console open");
REGISTER_LAUNCH_PARAMETER_HELP(-dev, LPARAM_dev, "", "enable developer mode");
REGISTER_LAUNCH_PARAMETER_HELP(-log, LPARAM_log, "[<conLevel>][<fileLevel>]", "enable logging. error level can be: 0 = trace, 1 = debug, 2 = info, 3 = warning, 4 = error, 5 = critical, 6 = disabled.");
REGISTER_LAUNCH_PARAMETER_HELP(-log_file, LPARAM_log_file, "[<logfile>]", "file to write the log to. Default log file is log.txt");
REGISTER_LAUNCH_PARAMETER_HELP(-map, LPARAM_map, "<map>", "load this map on start");
REGISTER_LAUNCH_PARAMETER_HELP(-gamemode, LPARAM_gamemode, "<gamemode>", "load this gamemode on start");
REGISTER_LAUNCH_PARAMETER_HELP(-luaext, LPARAM_luaext, "", "enables several additional lua modules (e.g. package and io)");
REGISTER_LAUNCH_PARAMETER_HELP(-verbose, LPARAM_verbose, "", "Enables additional debug messages.");

REGISTER_LAUNCH_PARAMETER_HELP(-tcpport, LPARAM_tcpport, "<port>", "set TCP port");
REGISTER_LAUNCH_PARAMETER_HELP(-udpport, LPARAM_udpport, "<port>", "set UDP port");
REGISTER_LAUNCH_PARAMETER_HELP(-tcponly, LPARAM_tcponly, "", "use TCP for all communication");
REGISTER_LAUNCH_PARAMETER_HELP(-connect, LPARAM_connect, "<ip>[:<port>]", "connect to server immediately. Default port is 29150");

REGISTER_LAUNCH_PARAMETER_HELP(-help, LPARAM_help, "-? /?", "show this help message");
REGISTER_LAUNCH_PARAMETER_HELP(-title, LPARAM_title, "<title>", "changes the window title");
REGISTER_LAUNCH_PARAMETER(/?,LPARAM_help);
REGISTER_LAUNCH_PARAMETER(-?,LPARAM_help);
