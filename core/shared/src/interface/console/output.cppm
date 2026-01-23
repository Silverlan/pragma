// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"
#include "util_enum_flags.hpp"

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

export module pragma.shared:console.output;

export import :console.enums;
export import pragma.util;

export {
	namespace Con {
		class c_crit;
	};
	template<class T>
	Con::c_crit &operator<<(Con::c_crit &con, const T &t);

	template<class T>
	Con::c_crit &operator<<(Con::c_crit &con, const T &t);

	namespace Con {
		namespace detail {
			DLLNETWORK std::atomic<pragma::util::LogSeverity> &get_current_level();
			DLLNETWORK std::function<void(const std::string_view &, pragma::console::MessageFlags, const Color *)> &get_output_callback();
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
			friend c_crit & ::operator<<(c_crit & con, const T & t);
		};
		class DLLNETWORK c_csv {};
		class DLLNETWORK c_ccl {};
#ifdef WINDOWS_CLANG_COMPILER_FIX
		DLLNETWORK c_cout &COUT;
		DLLNETWORK c_cwar &CWAR;
		DLLNETWORK c_cerr &CERR;
		DLLNETWORK c_crit &CRIT;
		DLLNETWORK c_csv &CSV;
		DLLNETWORK c_ccl &CCL;
#else
		extern DLLNETWORK c_cout COUT;
		extern DLLNETWORK c_cwar CWAR;
		extern DLLNETWORK c_cerr CERR;
		extern DLLNETWORK c_crit CRIT;
		extern DLLNETWORK c_csv CSV;
		extern DLLNETWORK c_ccl CCL;
#endif
		DLLNETWORK std::basic_ostream<char, std::char_traits<char>> &endl(std::basic_ostream<char, std::char_traits<char>> &os);
		DLLNETWORK std::basic_ostream<char, std::char_traits<char>> &prefix(std::basic_ostream<char, std::char_traits<char>> &os);
		DLLNETWORK void flush();
		DLLNETWORK void attr(DWORD attr);
		DLLNETWORK void WriteToLog(std::stringstream &ss);
		DLLNETWORK void WriteToLog(std::string str);
		DLLNETWORK int GetLogLevel();

		DLLNETWORK void disable_ansi_color_codes();
		DLLNETWORK void set_output_callback(const std::function<void(const std::string_view &, pragma::console::MessageFlags, const Color *)> &callback);
		DLLNETWORK const std::function<void(const std::string_view &, pragma::console::MessageFlags, const Color *)> &get_output_callback();
		DLLNETWORK void print(const std::string_view &sv, const Color &color, pragma::console::MessageFlags flags = pragma::console::MessageFlags::None);
		DLLNETWORK void print(const std::string_view &sv, pragma::console::MessageFlags flags = pragma::console::MessageFlags::None);
		template<typename T>
		inline void invoke_output_callback(const T &value, pragma::console::MessageFlags flags)
		{
			auto &outputCallback = get_output_callback();
			if(outputCallback == nullptr)
				return;
			auto color = pragma::console::console_color_flags_to_color(pragma::console::get_active_console_color_flags());
			std::stringstream ss;
			ss << value;
			outputCallback(ss.str(), flags, color.has_value() ? &(*color) : nullptr);
		}

#ifdef WINDOWS_CLANG_COMPILER_FIX
		DLLNETWORK const std::string &GET_COLOR_WARNING();
		DLLNETWORK const std::string &GET_COLOR_ERROR();
		DLLNETWORK const std::string &GET_COLOR_CRITICAL();
		DLLNETWORK const std::string &GET_COLOR_SERVER();
		DLLNETWORK const std::string &GET_COLOR_CLIENT();
		DLLNETWORK const std::string &GET_COLOR_LUA();
		DLLNETWORK const std::string &GET_COLOR_GUI();
		DLLNETWORK const std::string &GET_COLOR_RESET();

		DLLNETWORK std::string &GET_PREFIX_WARNING();
		DLLNETWORK std::string &GET_PREFIX_ERROR();
		DLLNETWORK std::string &GET_PREFIX_CRITICAL();
		DLLNETWORK std::string &GET_PREFIX_SERVER();
		DLLNETWORK std::string &GET_PREFIX_CLIENT();
		DLLNETWORK std::string &GET_PREFIX_LUA();
		DLLNETWORK std::string &GET_PREFIX_GUI();
#else
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
#endif
		using namespace pragma::math::scoped_enum::bitwise;
	};
	REGISTER_ENUM_FLAGS(pragma::console::MessageFlags)

	namespace pragma::logging::detail {
		enum class Type : uint8_t { None = 0, Info, Warn, Err, Crit };
		DLLNETWORK bool should_log_output();
		DLLNETWORK void set_should_log_output(bool shouldLogOutput);
		DLLNETWORK std::mutex &get_log_output_mutex();
		DLLNETWORK std::stringstream &get_log_output_stream();
		DLLNETWORK Type get_log_output_type();
		DLLNETWORK void set_log_output_type(Type type);
	};

#define PRAGMA_DETAIL_LOG_OUTPUT(v, etype)                                                                                                                                                                                                                                                       \
	if(pragma::logging::detail::should_log_output()) {                                                                                                                                                                                                                                               \
		pragma::logging::detail::get_log_output_mutex().lock();                                                                                                                                                                                                                                          \
		pragma::logging::detail::set_log_output_type(etype);                                                                                                                                                                                                                                                   \
		pragma::logging::detail::get_log_output_stream() << v;                                                                                                                                                                                                                                                 \
		pragma::logging::detail::get_log_output_mutex().unlock();                                                                                                                                                                                                                                        \
	}

#define PRAGMA_DETAIL_INVOKE_CONSOLE_OUTPUT_CALLBACK(v, type)                                                                                                                                                                                                                                    \
	if(Con::detail::get_output_callback() != nullptr)                                                                                                                                                                                                                                                   \
		Con::invoke_output_callback(v, type);

	// c_cout
	template<class T>
	inline Con::c_cout &operator<<(Con::c_cout &con, const T &t)
	{
		Con::detail::get_current_level() = pragma::util::LogSeverity::Info;
		std::cout << t;
		PRAGMA_DETAIL_LOG_OUTPUT(t, pragma::logging::detail::Type::Info)
		PRAGMA_DETAIL_INVOKE_CONSOLE_OUTPUT_CALLBACK(t, pragma::console::MessageFlags::Generic);
		return con;
	}
	typedef std::ostream &(*conmanipulator)(std::ostream &);
	DLLNETWORK Con::c_cout &operator<<(Con::c_cout &con, conmanipulator manipulator);
	//

	// c_cwar
	template<class T>
	Con::c_cwar &operator<<(Con::c_cwar &con, const T &t)
	{
		if(Con::detail::get_current_level() == pragma::util::LogSeverity::Disabled) {
			Con::detail::get_current_level() = pragma::util::LogSeverity::Warning;
			std::cout << Con::PREFIX_WARNING << Con::prefix;
		}

		std::cout << t;
		PRAGMA_DETAIL_LOG_OUTPUT(t, pragma::logging::detail::Type::Warn)
		PRAGMA_DETAIL_INVOKE_CONSOLE_OUTPUT_CALLBACK(t, pragma::console::MessageFlags::Warning);
		return con;
	}
	typedef std::ostream &(*conmanipulator)(std::ostream &);
	DLLNETWORK Con::c_cwar &operator<<(Con::c_cwar &con, conmanipulator manipulator);
	//

	// c_cerr
	template<class T>
	Con::c_cerr &operator<<(Con::c_cerr &con, const T &t)
	{
		if(Con::detail::get_current_level() == pragma::util::LogSeverity::Disabled) {
			Con::detail::get_current_level() = pragma::util::LogSeverity::Error;
			std::cout << Con::PREFIX_ERROR << Con::prefix;
		}

		std::cout << t;
		PRAGMA_DETAIL_LOG_OUTPUT(t, pragma::logging::detail::Type::Err)
		PRAGMA_DETAIL_INVOKE_CONSOLE_OUTPUT_CALLBACK(t, pragma::console::MessageFlags::Error);
		return con;
	}
	typedef std::ostream &(*conmanipulator)(std::ostream &);
	DLLNETWORK Con::c_cerr &operator<<(Con::c_cerr &con, conmanipulator manipulator);
	//

	// c_crit
	template<class T>
	Con::c_crit &operator<<(Con::c_crit &con, const T &t)
	{
		Con::CRIT.m_bActivated = true;
		if(Con::detail::get_current_level() == pragma::util::LogSeverity::Disabled) {
			Con::detail::get_current_level() = pragma::util::LogSeverity::Critical;
			std::cout << Con::PREFIX_CRITICAL << Con::prefix;
		}

		std::cout << t;
		PRAGMA_DETAIL_LOG_OUTPUT(t, pragma::logging::detail::Type::Crit)
		PRAGMA_DETAIL_INVOKE_CONSOLE_OUTPUT_CALLBACK(t, pragma::console::MessageFlags::Critical);
		return con;
	}
	typedef std::ostream &(*conmanipulator)(std::ostream &);
	DLLNETWORK Con::c_crit &operator<<(Con::c_crit &con, conmanipulator manipulator);
	//

	// c_csv
	template<class T>
	Con::c_csv &operator<<(Con::c_csv &con, const T &t)
	{
		if(Con::detail::get_current_level() == pragma::util::LogSeverity::Disabled) {
			Con::detail::get_current_level() = pragma::util::LogSeverity::Info;
			std::cout << Con::PREFIX_SERVER << Con::prefix;
		}
		std::cout << t;
		if(pragma::logging::detail::should_log_output()) {
			pragma::logging::detail::get_log_output_mutex().lock();
			if(pragma::logging::detail::get_log_output_type() == pragma::logging::detail::Type::None)
				pragma::logging::detail::get_log_output_stream() << Con::PREFIX_SERVER << Con::prefix;
			pragma::logging::detail::set_log_output_type(pragma::logging::detail::Type::Info);
			pragma::logging::detail::get_log_output_stream() << t;
			pragma::logging::detail::get_log_output_mutex().unlock();
		}
		PRAGMA_DETAIL_INVOKE_CONSOLE_OUTPUT_CALLBACK(t, pragma::console::MessageFlags::ServerSide);
		return con;
	}
	typedef std::ostream &(*conmanipulator)(std::ostream &);
	DLLNETWORK Con::c_csv &operator<<(Con::c_csv &con, conmanipulator manipulator);
	//

	// c_ccl
	template<class T>
	Con::c_ccl &operator<<(Con::c_ccl &con, const T &t)
	{
		if(Con::detail::get_current_level() == pragma::util::LogSeverity::Disabled) {
			Con::detail::get_current_level() = pragma::util::LogSeverity::Info;
			std::cout << Con::PREFIX_CLIENT << Con::prefix;
		}
		std::cout << t;
		if(pragma::logging::detail::should_log_output()) {
			pragma::logging::detail::get_log_output_mutex().lock();
			if(pragma::logging::detail::get_log_output_type() == pragma::logging::detail::Type::None)
				pragma::logging::detail::get_log_output_stream() << Con::PREFIX_CLIENT << Con::prefix;
			pragma::logging::detail::set_log_output_type(pragma::logging::detail::Type::Info);
			pragma::logging::detail::get_log_output_stream() << t;
			pragma::logging::detail::get_log_output_mutex().unlock();
		}
		PRAGMA_DETAIL_INVOKE_CONSOLE_OUTPUT_CALLBACK(t, pragma::console::MessageFlags::ClientSide);
		return con;
	}
	typedef std::ostream &(*conmanipulator)(std::ostream &);
	DLLNETWORK Con::c_ccl &operator<<(Con::c_ccl &con, conmanipulator manipulator);
	//

	namespace Con {
		using ::operator<<;
	}
};
