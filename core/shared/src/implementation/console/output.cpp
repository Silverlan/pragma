// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

#ifdef _WIN32
#include <Windows.h>
#else
#define DWORD unsigned int
#endif

module pragma.shared;

import :console.output;

#define PRAGMA_DETAIL_INVOKE_CONSOLE_OUTPUT_CALLBACK(v, type)                                                                                                                                                                                                                                    \
	if(Con::detail::get_output_callback() != nullptr)                                                                                                                                                                                                                                                   \
		Con::invoke_output_callback(v, type);

#ifdef WINDOWS_CLANG_COMPILER_FIX
DLLNETWORK Con::c_cout &Con::COUT {
	static Con::c_cout stream;
	return stream;
}
DLLNETWORK Con::c_cwar &Con::CWAR {
	static Con::c_cwar stream;
	return stream;
}
DLLNETWORK Con::c_cerr &Con::CERR {
	static Con::c_cerr stream;
	return stream;
}
DLLNETWORK Con::c_crit &Con::CRIT {
	static Con::c_crit stream;
	return stream;
}
DLLNETWORK Con::c_csv &Con::CSV {
	static Con::c_csv stream;
	return stream;
}
DLLNETWORK Con::c_ccl &Con::CCL {
	static Con::c_ccl stream;
	return stream;
}
#else
DLLNETWORK Con::c_cout Con::COUT;
DLLNETWORK Con::c_cwar Con::CWAR;
DLLNETWORK Con::c_cerr Con::CERR;
DLLNETWORK Con::c_crit Con::CRIT;
DLLNETWORK Con::c_csv Con::CSV;
DLLNETWORK Con::c_ccl Con::CCL;
#endif

#ifdef WINDOWS_CLANG_COMPILER_FIX
const std::string &Con::GET_COLOR_WARNING()
{
	static std::string str {pragma::console::get_true_color_code(Color {254, 228, 64}, {}, pragma::console::ConsoleDecoratorFlags::Bold)};
	return str;
}
const std::string &Con::GET_COLOR_ERROR()
{
	static std::string str {pragma::console::get_true_color_code(Color {233, 25, 15}, {}, pragma::console::ConsoleDecoratorFlags::Bold)};
	return str;
}
const std::string &Con::GET_COLOR_CRITICAL()
{
	static std::string str {pragma::console::get_true_color_code(Color {233, 25, 15}, Color {254, 250, 224}, pragma::console::ConsoleDecoratorFlags::Bold)};
	return str;
}
const std::string &Con::GET_COLOR_SERVER()
{
	static std::string str {pragma::console::get_true_color_code(Color {0, 245, 212})};
	return str;
}
const std::string &Con::GET_COLOR_CLIENT()
{
	static std::string str {pragma::console::get_true_color_code(Color {255, 73, 158})};
	return str;
}
const std::string &Con::GET_COLOR_LUA()
{
	static std::string str {pragma::console::get_true_color_code(Color {73, 182, 255})};
	return str;
}
const std::string &Con::GET_COLOR_GUI()
{
	static std::string str {pragma::console::get_true_color_code(Color {181, 23, 158})};
	return str;
}
const std::string &Con::GET_COLOR_RESET()
{
	static std::string str {pragma::console::get_reset_color_code()};
	return str;
}

std::string &Con::GET_PREFIX_WARNING()
{
	static std::string str {Con::COLOR_RESET + "[" + Con::COLOR_WARNING + "warning" + Con::COLOR_RESET + "] "};
	return str;
}
std::string &Con::GET_PREFIX_ERROR()
{
	static std::string str {Con::COLOR_RESET + "[" + Con::COLOR_ERROR + "error" + Con::COLOR_RESET + "] "};
	return str;
}
std::string &Con::GET_PREFIX_CRITICAL()
{
	static std::string str {Con::COLOR_RESET + "[" + Con::COLOR_CRITICAL + "critical" + Con::COLOR_RESET + "] "};
	return str;
}
std::string &Con::GET_PREFIX_SERVER()
{
	static std::string str {Con::COLOR_RESET + "[" + Con::COLOR_SERVER + "server" + Con::COLOR_RESET + "] "};
	return str;
}
std::string &Con::GET_PREFIX_CLIENT()
{
	static std::string str {Con::COLOR_RESET + "[" + Con::COLOR_CLIENT + "client" + Con::COLOR_RESET + "] "};
	return str;
}
std::string &Con::GET_PREFIX_LUA()
{
	static std::string str {Con::COLOR_RESET + "[" + Con::COLOR_LUA + "lua" + Con::COLOR_RESET + "] "};
	return str;
}
std::string &Con::GET_PREFIX_GUI()
{
	static std::string str {Con::COLOR_RESET + "[" + Con::COLOR_GUI + "gui" + Con::COLOR_RESET + "] "};
	return str;
}
#else
const std::string Con::COLOR_WARNING = pragma::console::get_true_color_code(Color {254, 228, 64}, {}, pragma::console::ConsoleDecoratorFlags::Bold);
const std::string Con::COLOR_ERROR = pragma::console::get_true_color_code(Color {233, 25, 15}, {}, pragma::console::ConsoleDecoratorFlags::Bold);
const std::string Con::COLOR_CRITICAL = pragma::console::get_true_color_code(Color {233, 25, 15}, Color {254, 250, 224}, pragma::console::ConsoleDecoratorFlags::Bold);
const std::string Con::COLOR_SERVER = pragma::console::get_true_color_code(Color {0, 245, 212});
const std::string Con::COLOR_CLIENT = pragma::console::get_true_color_code(Color {255, 73, 158});
const std::string Con::COLOR_LUA = pragma::console::get_true_color_code(Color {73, 182, 255});
const std::string Con::COLOR_GUI = pragma::console::get_true_color_code(Color {181, 23, 158});
const std::string Con::COLOR_RESET = pragma::console::get_reset_color_code();

std::string Con::PREFIX_WARNING = Con::COLOR_RESET + "[" + Con::COLOR_WARNING + "warning" + Con::COLOR_RESET + "] ";
std::string Con::PREFIX_ERROR = Con::COLOR_RESET + "[" + Con::COLOR_ERROR + "error" + Con::COLOR_RESET + "] ";
std::string Con::PREFIX_CRITICAL = Con::COLOR_RESET + "[" + Con::COLOR_CRITICAL + "critical" + Con::COLOR_RESET + "] ";
std::string Con::PREFIX_SERVER = Con::COLOR_RESET + "[" + Con::COLOR_SERVER + "server" + Con::COLOR_RESET + "] ";
std::string Con::PREFIX_CLIENT = Con::COLOR_RESET + "[" + Con::COLOR_CLIENT + "client" + Con::COLOR_RESET + "] ";
std::string Con::PREFIX_LUA = Con::COLOR_RESET + "[" + Con::COLOR_LUA + "lua" + Con::COLOR_RESET + "] ";
std::string Con::PREFIX_GUI = Con::COLOR_RESET + "[" + Con::COLOR_GUI + "gui" + Con::COLOR_RESET + "] ";
#endif

std::atomic<pragma::util::LogSeverity> &Con::detail::get_current_level()
{
	static std::atomic<pragma::util::LogSeverity> level;
	return level;
}
std::function<void(const std::string_view &, pragma::console::MessageFlags, const Color *)> &Con::detail::get_output_callback()
{
	static std::function<void(const std::string_view &, pragma::console::MessageFlags, const Color *)> callback;
	return callback;
}

static std::atomic<bool> g_shouldLogOutput {false};
bool pragma::logging::detail::should_log_output() {return g_shouldLogOutput;}
void pragma::logging::detail::set_should_log_output(bool shouldLogOutput) {g_shouldLogOutput = shouldLogOutput;}
std::mutex &pragma::logging::detail::get_log_output_mutex()
{
	static std::mutex logOutputMutex;
	return logOutputMutex;
}
std::stringstream &pragma::logging::detail::get_log_output_stream()
{
	static std::stringstream logOutput;
	return logOutput;
}
static pragma::logging::detail::Type g_logOutputType;
pragma::logging::detail::Type pragma::logging::detail::get_log_output_type() {return g_logOutputType;}
void pragma::logging::detail::set_log_output_type(Type type) {g_logOutputType = type;}

static auto cvLog = pragma::console::get_con_var("log_enabled");

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
	if(pragma::Engine::Get() == nullptr || !pragma::Engine::Get()->IsRunning())
		return 0;
	return cvLog->GetInt();
}

void Con::WriteToLog(std::stringstream &ss)
{
	if(pragma::Engine::Get() == nullptr)
		return;
	WriteToLog(ss.str());
}

void Con::WriteToLog(std::string str)
{
	if(pragma::Engine::Get() == nullptr)
		return;
	pragma::Engine::Get()->WriteToLog(str);
}

void Con::set_output_callback(const std::function<void(const std::string_view &, pragma::console::MessageFlags, const Color *)> &callback) { detail::get_output_callback() = callback; }
const std::function<void(const std::string_view &, pragma::console::MessageFlags, const Color *)> &Con::get_output_callback() { return detail::get_output_callback(); }
void Con::print(const std::string_view &sv, const Color &color, pragma::console::MessageFlags flags)
{
	pragma::console::set_console_color(pragma::console::color_to_console_color_flags(color));
	std::cout << sv;
	flush();
	auto &outputCallback = get_output_callback();
	if(outputCallback == nullptr)
		return;
	outputCallback(sv, flags, &color);
}
void Con::print(const std::string_view &sv, pragma::console::MessageFlags flags)
{
	std::cout << sv;
	flush();
	auto &outputCallback = get_output_callback();
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
	DLLNETWORK std::shared_ptr<spdlog::logger> consoleOutputLogger {};
};

static void log_output()
{
	pragma::logging::detail::get_log_output_mutex().lock();
	if(pragma::logging::detail::consoleOutputLogger) {
		switch(pragma::logging::detail::get_log_output_type()) {
		case pragma::logging::detail::Type::Info:
		case pragma::logging::detail::Type::None:
			pragma::logging::detail::consoleOutputLogger->info(pragma::logging::detail::get_log_output_stream().str());
			break;
		case pragma::logging::detail::Type::Warn:
			pragma::logging::detail::consoleOutputLogger->warn(pragma::logging::detail::get_log_output_stream().str());
			break;
		case pragma::logging::detail::Type::Err:
			pragma::logging::detail::consoleOutputLogger->error(pragma::logging::detail::get_log_output_stream().str());
			break;
		case pragma::logging::detail::Type::Crit:
			pragma::logging::detail::consoleOutputLogger->critical(pragma::logging::detail::get_log_output_stream().str());
			break;
		}
	}

	auto &ss = pragma::logging::detail::get_log_output_stream();
	ss.clear();
	ss.str("");

	pragma::logging::detail::set_log_output_type(pragma::logging::detail::Type::None);
	pragma::logging::detail::get_log_output_mutex().unlock();
}

Con::c_cout &operator<<(Con::c_cout &con, conmanipulator manipulator)
{
	std::cout << manipulator;
	PRAGMA_DETAIL_INVOKE_CONSOLE_OUTPUT_CALLBACK(manipulator, pragma::console::MessageFlags::Generic);
	return con;
}

Con::c_cwar &operator<<(Con::c_cwar &con, conmanipulator manipulator)
{
	std::cout << manipulator;
	PRAGMA_DETAIL_INVOKE_CONSOLE_OUTPUT_CALLBACK(manipulator, pragma::console::MessageFlags::Warning);
	return con;
}

Con::c_cerr &operator<<(Con::c_cerr &con, conmanipulator manipulator)
{
	std::cout << manipulator;
	PRAGMA_DETAIL_INVOKE_CONSOLE_OUTPUT_CALLBACK(manipulator, pragma::console::MessageFlags::Error);
	return con;
}

Con::c_crit &operator<<(Con::c_crit &con, conmanipulator manipulator)
{
	std::cout << manipulator;
	PRAGMA_DETAIL_INVOKE_CONSOLE_OUTPUT_CALLBACK(manipulator, pragma::console::MessageFlags::Critical);
	return con;
}

Con::c_csv &operator<<(Con::c_csv &con, conmanipulator manipulator)
{
	std::cout << manipulator;
	PRAGMA_DETAIL_INVOKE_CONSOLE_OUTPUT_CALLBACK(manipulator, pragma::console::MessageFlags::ServerSide);
	return con;
}

Con::c_ccl &operator<<(Con::c_ccl &con, conmanipulator manipulator)
{
	std::cout << manipulator;
	PRAGMA_DETAIL_INVOKE_CONSOLE_OUTPUT_CALLBACK(manipulator, pragma::console::MessageFlags::ClientSide);
	return con;
}

std::basic_ostream<char, std::char_traits<char>> &Con::endl(std::basic_ostream<char, std::char_traits<char>> &os)
{
	auto &bCrit = Con::CRIT.m_bActivated;

#ifdef _WIN32
	os.put('\n');
	//os.flush();
	//flush();
#else
	os << "\n";
	//os<<"\033[0m"<<"\n";
#endif
	switch(detail::get_current_level()) {
	case pragma::util::LogSeverity::Warning:
	case pragma::util::LogSeverity::Error:
	case pragma::util::LogSeverity::Critical:
		os << Con::COLOR_RESET;
		if(pragma::logging::detail::should_log_output()) {
			pragma::logging::detail::get_log_output_mutex().lock();
			pragma::logging::detail::get_log_output_stream() << Con::COLOR_RESET;
			pragma::logging::detail::get_log_output_mutex().unlock();
		}
		break;
	}
	std::cout.flush();
	detail::get_current_level() = pragma::util::LogSeverity::Disabled;
	if(pragma::logging::detail::should_log_output())
		log_output();
	switch(detail::get_current_level()) {
	case pragma::util::LogSeverity::Error:
	case pragma::util::LogSeverity::Critical:
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
	switch(detail::get_current_level()) {
	case pragma::util::LogSeverity::Warning:
		os << Con::COLOR_WARNING;
		if(pragma::logging::detail::should_log_output()) {
			pragma::logging::detail::get_log_output_mutex().lock();
			pragma::logging::detail::get_log_output_stream() << Con::COLOR_WARNING;
			pragma::logging::detail::get_log_output_mutex().unlock();
		}
		break;
	case pragma::util::LogSeverity::Error:
		os << Con::COLOR_ERROR;
		if(pragma::logging::detail::should_log_output()) {
			pragma::logging::detail::get_log_output_mutex().lock();
			pragma::logging::detail::get_log_output_stream() << Con::COLOR_ERROR;
			pragma::logging::detail::get_log_output_mutex().unlock();
		}
		break;
	case pragma::util::LogSeverity::Critical:
		os << Con::COLOR_CRITICAL;
		if(pragma::logging::detail::should_log_output()) {
			pragma::logging::detail::get_log_output_mutex().lock();
			pragma::logging::detail::get_log_output_stream() << Con::COLOR_CRITICAL;
			pragma::logging::detail::get_log_output_mutex().unlock();
		}
		break;
	}
	return os;
}
