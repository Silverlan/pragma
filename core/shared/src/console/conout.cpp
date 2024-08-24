/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/engine.h"
#include "pragma/console/conout.h"
#include "pragma/console/cvar.h"
#include "pragma/logging.hpp"
#include <pragma/console/convars.h>
#include <sharedutils/util_debug.h>
#include <mathutil/color.h>

DLLNETWORK Con::c_cout Con::cout;
DLLNETWORK Con::c_cwar Con::cwar;
DLLNETWORK Con::c_cerr Con::cerr;
DLLNETWORK Con::c_crit Con::crit;
DLLNETWORK Con::c_csv Con::csv;
DLLNETWORK Con::c_ccl Con::ccl;

const std::string Con::COLOR_WARNING = util::get_true_color_code(Color {254, 228, 64}, {}, util::ConsoleDecoratorFlags::Bold);
const std::string Con::COLOR_ERROR = util::get_true_color_code(Color {233, 25, 15}, {}, util::ConsoleDecoratorFlags::Bold);
const std::string Con::COLOR_CRITICAL = util::get_true_color_code(Color {233, 25, 15}, Color {254, 250, 224}, util::ConsoleDecoratorFlags::Bold);
const std::string Con::COLOR_SERVER = util::get_true_color_code(Color {0, 245, 212});
const std::string Con::COLOR_CLIENT = util::get_true_color_code(Color {255, 73, 158});
const std::string Con::COLOR_LUA = util::get_true_color_code(Color {73, 182, 255});
const std::string Con::COLOR_GUI = util::get_true_color_code(Color {181, 23, 158});
const std::string Con::COLOR_RESET = util::get_reset_color_code();

std::string Con::PREFIX_WARNING = Con::COLOR_RESET + "[" + Con::COLOR_WARNING + "warning" + Con::COLOR_RESET + "] ";
std::string Con::PREFIX_ERROR = Con::COLOR_RESET + "[" + Con::COLOR_ERROR + "error" + Con::COLOR_RESET + "] ";
std::string Con::PREFIX_CRITICAL = Con::COLOR_RESET + "[" + Con::COLOR_CRITICAL + "critical" + Con::COLOR_RESET + "] ";
std::string Con::PREFIX_SERVER = Con::COLOR_RESET + "[" + Con::COLOR_SERVER + "server" + Con::COLOR_RESET + "] ";
std::string Con::PREFIX_CLIENT = Con::COLOR_RESET + "[" + Con::COLOR_CLIENT + "client" + Con::COLOR_RESET + "] ";
std::string Con::PREFIX_LUA = Con::COLOR_RESET + "[" + Con::COLOR_LUA + "lua" + Con::COLOR_RESET + "] ";
std::string Con::PREFIX_GUI = Con::COLOR_RESET + "[" + Con::COLOR_GUI + "gui" + Con::COLOR_RESET + "] ";

extern DLLNETWORK Engine *engine;
static CVar cvLog = GetConVar("log_enabled");

void Con::disable_ansi_color_codes()
{
	PREFIX_WARNING = "[warning] ";
	PREFIX_ERROR = "[error] ";
	PREFIX_CRITICAL = "[critical] ";
	PREFIX_SERVER = "[server] ";
	PREFIX_CLIENT = "[client] ";
	PREFIX_LUA = "[lua] ";
	PREFIX_GUI = "[gui] ";
}

int Con::GetLogLevel()
{
	if(engine == NULL || !engine->IsRunning())
		return 0;
	return cvLog->GetInt();
}

void Con::WriteToLog(std::stringstream &ss)
{
	if(engine == NULL)
		return;
	WriteToLog(ss.str());
}

void Con::WriteToLog(std::string str)
{
	if(engine == NULL)
		return;
	engine->WriteToLog(str);
}

void Con::set_output_callback(const std::function<void(const std::string_view &, MessageFlags, const ::Color *)> &callback) { detail::outputCallback = callback; }
const std::function<void(const std::string_view &, Con::MessageFlags, const Color *)> &Con::get_output_callback() { return detail::outputCallback; }
void Con::print(const std::string_view &sv, const ::Color &color, MessageFlags flags)
{
	util::set_console_color(util::color_to_console_color_flags(color));
	std::cout << sv;
	Con::flush();
	auto &outputCallback = Con::get_output_callback();
	if(outputCallback == nullptr)
		return;
	outputCallback(sv, flags, &color);
}
void Con::print(const std::string_view &sv, MessageFlags flags)
{
	std::cout << sv;
	Con::flush();
	auto &outputCallback = Con::get_output_callback();
	if(outputCallback == nullptr)
		return;
	outputCallback(sv, flags, nullptr);
}

////////////////////////////////

void Con::attr(DWORD attr)
{
#ifdef _WIN32
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hOut, static_cast<WORD>(attr));
#endif
}

void Con::flush()
{
#ifdef _WIN32
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hOut, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
#endif
}

namespace pragma::logging::detail {
	DLLNETWORK std::atomic<bool> shouldLogOutput = false;
	DLLNETWORK std::mutex logOutputMutex {};
	DLLNETWORK std::stringstream logOutput {};
	DLLNETWORK Type type = Type::None;

	DLLNETWORK std::shared_ptr<spdlog::logger> consoleOutputLogger {};
};

namespace Con::detail {
	DLLNETWORK std::atomic<util::LogSeverity> currentLevel = util::LogSeverity::Disabled;
	DLLNETWORK std::function<void(const std::string_view &, Con::MessageFlags, const Color *)> outputCallback = nullptr;
};

static void log_output()
{
	pragma::logging::detail::logOutputMutex.lock();
	if(pragma::logging::detail::consoleOutputLogger) {
		switch(pragma::logging::detail::type) {
		case pragma::logging::detail::Type::Info:
		case pragma::logging::detail::Type::None:
			pragma::logging::detail::consoleOutputLogger->info(pragma::logging::detail::logOutput.str());
			break;
		case pragma::logging::detail::Type::Warn:
			pragma::logging::detail::consoleOutputLogger->warn(pragma::logging::detail::logOutput.str());
			break;
		case pragma::logging::detail::Type::Err:
			pragma::logging::detail::consoleOutputLogger->error(pragma::logging::detail::logOutput.str());
			break;
		case pragma::logging::detail::Type::Crit:
			pragma::logging::detail::consoleOutputLogger->critical(pragma::logging::detail::logOutput.str());
			break;
		}
	}

	auto &ss = pragma::logging::detail::logOutput;
	ss.clear();
	ss.str("");

	pragma::logging::detail::type = pragma::logging::detail::Type::None;
	pragma::logging::detail::logOutputMutex.unlock();
}

Con::c_cout &operator<<(Con::c_cout &con, conmanipulator manipulator)
{
	std::cout << manipulator;
	PRAGMA_DETAIL_INVOKE_CONSOLE_OUTPUT_CALLBACK(manipulator, Con::MessageFlags::Generic);
	return con;
}

Con::c_cwar &operator<<(Con::c_cwar &con, conmanipulator manipulator)
{
	std::cout << manipulator;
	PRAGMA_DETAIL_INVOKE_CONSOLE_OUTPUT_CALLBACK(manipulator, Con::MessageFlags::Warning);
	return con;
}

Con::c_cerr &operator<<(Con::c_cerr &con, conmanipulator manipulator)
{
	std::cout << manipulator;
	PRAGMA_DETAIL_INVOKE_CONSOLE_OUTPUT_CALLBACK(manipulator, Con::MessageFlags::Error);
	return con;
}

Con::c_crit &operator<<(Con::c_crit &con, conmanipulator manipulator)
{
	std::cout << manipulator;
	PRAGMA_DETAIL_INVOKE_CONSOLE_OUTPUT_CALLBACK(manipulator, Con::MessageFlags::Critical);
	return con;
}

Con::c_csv &operator<<(Con::c_csv &con, conmanipulator manipulator)
{
	std::cout << manipulator;
	PRAGMA_DETAIL_INVOKE_CONSOLE_OUTPUT_CALLBACK(manipulator, Con::MessageFlags::ServerSide);
	return con;
}

Con::c_ccl &operator<<(Con::c_ccl &con, conmanipulator manipulator)
{
	std::cout << manipulator;
	PRAGMA_DETAIL_INVOKE_CONSOLE_OUTPUT_CALLBACK(manipulator, Con::MessageFlags::ClientSide);
	return con;
}

std::basic_ostream<char, std::char_traits<char>> &Con::endl(std::basic_ostream<char, std::char_traits<char>> &os)
{
	auto &bCrit = Con::crit.m_bActivated;

#ifdef _WIN32
	os.put('\n');
	//os.flush();
	//flush();
#else
	os << "\n";
	//os<<"\033[0m"<<"\n";
#endif
	switch(Con::detail::currentLevel) {
	case util::LogSeverity::Warning:
	case util::LogSeverity::Error:
	case util::LogSeverity::Critical:
		os << Con::COLOR_RESET;
		if(pragma::logging::detail::shouldLogOutput) {
			pragma::logging::detail::logOutputMutex.lock();
			pragma::logging::detail::logOutput << Con::COLOR_RESET;
			pragma::logging::detail::logOutputMutex.unlock();
		}
		break;
	}
	std::cout.flush();
	Con::detail::currentLevel = util::LogSeverity::Disabled;
	if(pragma::logging::detail::shouldLogOutput)
		log_output();
	switch(Con::detail::currentLevel) {
	case util::LogSeverity::Error:
	case util::LogSeverity::Critical:
		pragma::flush_loggers(); // Flush loggers immediately in case this will lead to a crash
		break;
	}
	if(bCrit == true) {
		bCrit = false;
		std::this_thread::sleep_for(std::chrono::seconds(5));
	}
	return os;
}

std::basic_ostream<char, std::char_traits<char>> &Con::prefix(std::basic_ostream<char, std::char_traits<char>> &os)
{
	// If the message has a prefix, the prefix may overwrite the color of the main message, so we
	// have to reset the color here.
	switch(Con::detail::currentLevel) {
	case util::LogSeverity::Warning:
		os << Con::COLOR_WARNING;
		if(pragma::logging::detail::shouldLogOutput) {
			pragma::logging::detail::logOutputMutex.lock();
			pragma::logging::detail::logOutput << Con::COLOR_WARNING;
			pragma::logging::detail::logOutputMutex.unlock();
		}
		break;
	case util::LogSeverity::Error:
		os << Con::COLOR_ERROR;
		if(pragma::logging::detail::shouldLogOutput) {
			pragma::logging::detail::logOutputMutex.lock();
			pragma::logging::detail::logOutput << Con::COLOR_ERROR;
			pragma::logging::detail::logOutputMutex.unlock();
		}
		break;
	case util::LogSeverity::Critical:
		os << Con::COLOR_CRITICAL;
		if(pragma::logging::detail::shouldLogOutput) {
			pragma::logging::detail::logOutputMutex.lock();
			pragma::logging::detail::logOutput << Con::COLOR_CRITICAL;
			pragma::logging::detail::logOutputMutex.unlock();
		}
		break;
	}
	return os;
}
