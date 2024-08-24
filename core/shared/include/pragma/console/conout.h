/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __CONOUT_H__
#define __CONOUT_H__
#include "pragma/definitions.h"
#include "pragma/console/util_console_color.hpp"
#include "pragma/util/global_string_table.hpp"
#include <sharedutils/util_log.hpp>
#include <iostream>
#include <sstream>
#include <atomic>
#include <mutex>
#include <functional>
#include <string_view>
#ifdef _WIN32
#include <Windows.h>
#else
#define DWORD unsigned int
#define FOREGROUND_BLUE 1
#define FOREGROUND_GREEN 2
#define FOREGROUND_RED 4
#define FOREGROUND_INTENSITY 8
#define BACKGROUND_BLUE 16
#define BACKGROUND_GREEN 32
#define BACKGROUND_RED 64
#define BACKGROUND_INTENSITY 128
#endif

struct Color;
namespace Con {
	class c_crit;
};
template<class T>
Con::c_crit &operator<<(Con::c_crit &con, const T &t);

namespace Con {
	enum class MessageFlags : uint8_t {
		None = 0u,
		Generic = 1u,
		Warning = Generic << 1u,
		Error = Warning << 1u,
		Critical = Error << 1u,

		ServerSide = Critical << 1u,
		ClientSide = ServerSide << 1u
	};

	namespace detail {
		extern DLLNETWORK std::atomic<::util::LogSeverity> currentLevel;
		extern DLLNETWORK std::function<void(const std::string_view &, Con::MessageFlags, const Color *)> outputCallback;
	};
	class DLLNETWORK c_cout {};
	class DLLNETWORK c_cwar {};
	class DLLNETWORK c_cerr {};
	class DLLNETWORK c_crit {
	  private:
		std::atomic<bool> m_bActivated = false;
	  public:
		friend DLLNETWORK std::basic_ostream<char, std::char_traits<char>> &endl(std::basic_ostream<char, std::char_traits<char>> &os);
		template<class T>
		friend Con::c_crit & ::operator<<(Con::c_crit &con, const T &t);
	};
	class DLLNETWORK c_csv {};
	class DLLNETWORK c_ccl {};
	extern DLLNETWORK c_cout cout;
	extern DLLNETWORK c_cwar cwar;
	extern DLLNETWORK c_cerr cerr;
	extern DLLNETWORK c_crit crit;
	extern DLLNETWORK c_csv csv;
	extern DLLNETWORK c_ccl ccl;
	DLLNETWORK std::basic_ostream<char, std::char_traits<char>> &endl(std::basic_ostream<char, std::char_traits<char>> &os);
	DLLNETWORK std::basic_ostream<char, std::char_traits<char>> &prefix(std::basic_ostream<char, std::char_traits<char>> &os);
	DLLNETWORK void flush();
	DLLNETWORK void attr(DWORD attr);
	DLLNETWORK void WriteToLog(std::stringstream &ss);
	DLLNETWORK void WriteToLog(std::string str);
	DLLNETWORK int GetLogLevel();

	DLLNETWORK void disable_ansi_color_codes();
	DLLNETWORK void set_output_callback(const std::function<void(const std::string_view &, MessageFlags, const ::Color *)> &callback);
	DLLNETWORK const std::function<void(const std::string_view &, MessageFlags, const ::Color *)> &get_output_callback();
	DLLNETWORK void print(const std::string_view &sv, const ::Color &color, MessageFlags flags = MessageFlags::None);
	DLLNETWORK void print(const std::string_view &sv, MessageFlags flags = MessageFlags::None);
	template<typename T>
	inline void invoke_output_callback(const T &value, MessageFlags flags)
	{
		auto &outputCallback = Con::get_output_callback();
		if(outputCallback == nullptr)
			return;
		auto color = util::console_color_flags_to_color(util::get_active_console_color_flags());
		std::stringstream ss;
		ss << value;
		outputCallback(ss.str(), flags, color.has_value() ? &(*color) : nullptr);
	}

	DLLNETWORK extern const std::string COLOR_WARNING;
	DLLNETWORK extern const std::string COLOR_ERROR;
	DLLNETWORK extern const std::string COLOR_CRITICAL;
	DLLNETWORK extern const std::string COLOR_SERVER;
	DLLNETWORK extern const std::string COLOR_CLIENT;
	DLLNETWORK extern const std::string COLOR_LUA;
	DLLNETWORK extern const std::string COLOR_GUI;
	DLLNETWORK extern const std::string COLOR_RESET;

	DLLNETWORK extern std::string PREFIX_WARNING;
	DLLNETWORK extern std::string PREFIX_ERROR;
	DLLNETWORK extern std::string PREFIX_CRITICAL;
	DLLNETWORK extern std::string PREFIX_SERVER;
	DLLNETWORK extern std::string PREFIX_CLIENT;
	DLLNETWORK extern std::string PREFIX_LUA;
	DLLNETWORK extern std::string PREFIX_GUI;
};
REGISTER_BASIC_BITWISE_OPERATORS(Con::MessageFlags)

namespace pragma::logging::detail {
	enum class Type : uint8_t { None = 0, Info, Warn, Err, Crit };
	extern DLLNETWORK std::atomic<bool> shouldLogOutput;
	extern DLLNETWORK std::mutex logOutputMutex;
	extern DLLNETWORK std::stringstream logOutput;
	extern DLLNETWORK Type type;
};

#define PRAGMA_DETAIL_LOG_OUTPUT(v, etype)                                                                                                                                                                                                                                                       \
	if(pragma::logging::detail::shouldLogOutput) {                                                                                                                                                                                                                                               \
		pragma::logging::detail::logOutputMutex.lock();                                                                                                                                                                                                                                          \
		pragma::logging::detail::type = etype;                                                                                                                                                                                                                                                   \
		pragma::logging::detail::logOutput << v;                                                                                                                                                                                                                                                 \
		pragma::logging::detail::logOutputMutex.unlock();                                                                                                                                                                                                                                        \
	}

#define PRAGMA_DETAIL_INVOKE_CONSOLE_OUTPUT_CALLBACK(v, type)                                                                                                                                                                                                                                    \
	if(Con::detail::outputCallback != nullptr)                                                                                                                                                                                                                                                   \
		Con::invoke_output_callback(v, type);

// c_cout
template<class T>
Con::c_cout &operator<<(Con::c_cout &con, const T &t)
{
	Con::detail::currentLevel = ::util::LogSeverity::Info;
	std::cout << t;
	PRAGMA_DETAIL_LOG_OUTPUT(t, pragma::logging::detail::Type::Info)
	PRAGMA_DETAIL_INVOKE_CONSOLE_OUTPUT_CALLBACK(t, Con::MessageFlags::Generic);
	return con;
}
typedef std::ostream &(*conmanipulator)(std::ostream &);
DLLNETWORK Con::c_cout &operator<<(Con::c_cout &con, conmanipulator manipulator);
//

// c_cwar
template<class T>
Con::c_cwar &operator<<(Con::c_cwar &con, const T &t)
{
	if(Con::detail::currentLevel == ::util::LogSeverity::Disabled) {
		Con::detail::currentLevel = ::util::LogSeverity::Warning;
		std::cout << Con::PREFIX_WARNING << Con::prefix;
	}

	std::cout << t;
	PRAGMA_DETAIL_LOG_OUTPUT(t, pragma::logging::detail::Type::Warn)
	PRAGMA_DETAIL_INVOKE_CONSOLE_OUTPUT_CALLBACK(t, Con::MessageFlags::Warning);
	return con;
}
typedef std::ostream &(*conmanipulator)(std::ostream &);
DLLNETWORK Con::c_cwar &operator<<(Con::c_cwar &con, conmanipulator manipulator);
//

// c_cerr
template<class T>
Con::c_cerr &operator<<(Con::c_cerr &con, const T &t)
{
	if(Con::detail::currentLevel == ::util::LogSeverity::Disabled) {
		Con::detail::currentLevel = ::util::LogSeverity::Error;
		std::cout << Con::PREFIX_ERROR << Con::prefix;
	}

	std::cout << t;
	PRAGMA_DETAIL_LOG_OUTPUT(t, pragma::logging::detail::Type::Err)
	PRAGMA_DETAIL_INVOKE_CONSOLE_OUTPUT_CALLBACK(t, Con::MessageFlags::Error);
	return con;
}
typedef std::ostream &(*conmanipulator)(std::ostream &);
DLLNETWORK Con::c_cerr &operator<<(Con::c_cerr &con, conmanipulator manipulator);
//

// c_crit
template<class T>
Con::c_crit &operator<<(Con::c_crit &con, const T &t)
{
	Con::crit.m_bActivated = true;
	if(Con::detail::currentLevel == ::util::LogSeverity::Disabled) {
		Con::detail::currentLevel = ::util::LogSeverity::Critical;
		std::cout << Con::PREFIX_CRITICAL << Con::prefix;
	}

	std::cout << t;
	PRAGMA_DETAIL_LOG_OUTPUT(t, pragma::logging::detail::Type::Crit)
	PRAGMA_DETAIL_INVOKE_CONSOLE_OUTPUT_CALLBACK(t, Con::MessageFlags::Critical);
	return con;
}
typedef std::ostream &(*conmanipulator)(std::ostream &);
DLLNETWORK Con::c_crit &operator<<(Con::c_crit &con, conmanipulator manipulator);
//

// c_csv
template<class T>
Con::c_csv &operator<<(Con::c_csv &con, const T &t)
{
	if(Con::detail::currentLevel == ::util::LogSeverity::Disabled) {
		Con::detail::currentLevel = ::util::LogSeverity::Info;
		std::cout << Con::PREFIX_SERVER << Con::prefix;
	}
	std::cout << t;
	if(pragma::logging::detail::shouldLogOutput) {
		pragma::logging::detail::logOutputMutex.lock();
		if(pragma::logging::detail::type == pragma::logging::detail::Type::None)
			pragma::logging::detail::logOutput << Con::PREFIX_SERVER << Con::prefix;
		pragma::logging::detail::type = pragma::logging::detail::Type::Info;
		pragma::logging::detail::logOutput << t;
		pragma::logging::detail::logOutputMutex.unlock();
	}
	PRAGMA_DETAIL_INVOKE_CONSOLE_OUTPUT_CALLBACK(t, Con::MessageFlags::ServerSide);
	return con;
}
typedef std::ostream &(*conmanipulator)(std::ostream &);
DLLNETWORK Con::c_csv &operator<<(Con::c_csv &con, conmanipulator manipulator);
//

// c_ccl
template<class T>
Con::c_ccl &operator<<(Con::c_ccl &con, const T &t)
{
	if(Con::detail::currentLevel == ::util::LogSeverity::Disabled) {
		Con::detail::currentLevel = ::util::LogSeverity::Info;
		std::cout << Con::PREFIX_CLIENT << Con::prefix;
	}
	std::cout << t;
	if(pragma::logging::detail::shouldLogOutput) {
		pragma::logging::detail::logOutputMutex.lock();
		if(pragma::logging::detail::type == pragma::logging::detail::Type::None)
			pragma::logging::detail::logOutput << Con::PREFIX_CLIENT << Con::prefix;
		pragma::logging::detail::type = pragma::logging::detail::Type::Info;
		pragma::logging::detail::logOutput << t;
		pragma::logging::detail::logOutputMutex.unlock();
	}
	PRAGMA_DETAIL_INVOKE_CONSOLE_OUTPUT_CALLBACK(t, Con::MessageFlags::ClientSide);
	return con;
}
typedef std::ostream &(*conmanipulator)(std::ostream &);
DLLNETWORK Con::c_ccl &operator<<(Con::c_ccl &con, conmanipulator manipulator);
//

#endif
