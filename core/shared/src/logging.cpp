/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/logging.hpp"
#include "pragma/console/conout.h"
#include "pragma/console/spdlog_anycolor_sink.hpp"
#include "pragma/engine.h"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/pattern_formatter.h>
#include <spdlog/formatter.h>
#include <spdlog/fmt/bundled/format.h>
#include <sharedutils/magic_enum.hpp>

const std::string PRAGMA_LOGGER_NAME = "pragma_logger";
const std::string PRAGMA_FILE_LOGGER_NAME = "pragma_logger_file";

extern DLLNETWORK Engine *engine;

static bool g_ansiColorCodesEnabled = true;
void pragma::logging::set_ansi_color_codes_enabled(bool enabled) { g_ansiColorCodesEnabled = enabled; }

int32_t pragma::logging::severity_to_spdlog_level(util::LogSeverity severity)
{
	switch(severity) {
	case util::LogSeverity::Info:
		return spdlog::level::info;
	case util::LogSeverity::Warning:
		return spdlog::level::warn;
	case util::LogSeverity::Error:
		return spdlog::level::err;
	case util::LogSeverity::Critical:
		return spdlog::level::critical;
	case util::LogSeverity::Debug:
		return spdlog::level::debug;
	case util::LogSeverity::Trace:
		return spdlog::level::trace;
	}
	static_assert(umath::to_integral(util::LogSeverity::Count) == 7, "Expand this list when more severity types are added!");
	return spdlog::level::info;
}

::util::LogSeverity pragma::logging::spdlog_level_to_severity(int32_t spdlogLevel)
{
	switch(spdlogLevel) {
	case spdlog::level::info:
		return util::LogSeverity::Info;
	case spdlog::level::warn:
		return util::LogSeverity::Warning;
	case spdlog::level::err:
		return util::LogSeverity::Error;
	case spdlog::level::critical:
		return util::LogSeverity::Critical;
	case spdlog::level::debug:
		return util::LogSeverity::Debug;
	case spdlog::level::trace:
		return util::LogSeverity::Trace;
	}
	static_assert(umath::to_integral(util::LogSeverity::Count) == 7, "Expand this list when more severity types are added!");
	return ::util::LogSeverity::Info;
}

void pragma::log(const std::string &msg, util::LogSeverity severity) { spdlog::log(static_cast<spdlog::level::level_enum>(pragma::logging::severity_to_spdlog_level(severity)), msg); }

bool pragma::is_log_level_enabled(::util::LogSeverity severity) { return spdlog::should_log(static_cast<spdlog::level::level_enum>(pragma::logging::severity_to_spdlog_level(severity))); }

static std::unordered_map<std::string, std::shared_ptr<spdlog::logger>> g_customLoggers;
void pragma::flush_loggers()
{
	auto logger0 = spdlog::get(PRAGMA_LOGGER_NAME);
	auto logger1 = spdlog::get(PRAGMA_FILE_LOGGER_NAME);
	if(logger0)
		logger0->flush();
	if(logger1)
		logger1->flush();

	for(auto &pair : g_customLoggers)
		pair.second->flush();
}

static void update_pragma_log_level()
{
	auto logger = spdlog::get(PRAGMA_LOGGER_NAME);
	if(!logger)
		return;
	auto &sinks = logger->sinks();
	if(sinks.empty())
		return;
	auto &sink0 = sinks.front();
	auto level = sink0->level();
	for(auto it = sinks.begin() + 1; it != sinks.end(); ++it)
		level = umath::min(level, (*it)->level());
	logger->set_level(level);
}
void pragma::set_console_log_level(::util::LogSeverity level)
{
	auto logger = spdlog::get(PRAGMA_LOGGER_NAME);
	if(!logger)
		return;
	auto &sinks = logger->sinks();
	if(sinks.empty())
		return;
	auto &sink = sinks.front();
	sink->set_level(static_cast<spdlog::level::level_enum>(pragma::logging::severity_to_spdlog_level(level)));

	update_pragma_log_level();
}
::util::LogSeverity pragma::get_console_log_level()
{
	auto logger = spdlog::get(PRAGMA_LOGGER_NAME);
	if(!logger)
		return ::util::LogSeverity::Disabled;
	auto &sinks = logger->sinks();
	if(sinks.empty())
		return ::util::LogSeverity::Disabled;
	return logging::spdlog_level_to_severity(sinks.front()->level());
}
void pragma::set_file_log_level(::util::LogSeverity level)
{
	auto logger = spdlog::get(PRAGMA_LOGGER_NAME);
	if(logger) {
		auto &sinks = logger->sinks();
		if(sinks.size() >= 2) {
			auto &sinkFile = sinks[1];
			sinkFile->set_level(static_cast<spdlog::level::level_enum>(pragma::logging::severity_to_spdlog_level(level)));
		}
	}

	auto loggerFile = spdlog::get(PRAGMA_FILE_LOGGER_NAME);
	if(loggerFile)
		loggerFile->set_level(static_cast<spdlog::level::level_enum>(pragma::logging::severity_to_spdlog_level(level)));

	update_pragma_log_level();
}
::util::LogSeverity pragma::get_file_log_level()
{
	auto loggerFile = spdlog::get(PRAGMA_FILE_LOGGER_NAME);
	if(!loggerFile)
		return ::util::LogSeverity::Disabled;
	return logging::spdlog_level_to_severity(loggerFile->level());
}

/////////////////////////////

static void append_string(spdlog::memory_buf_t &dest, const std::string &str)
{
#ifdef SPDLOG_USE_STD_FORMAT
	dest.append(str.begin(), str.end());
#else
	dest.append(str.data(), str.data() + str.length());
#endif
}
class SpdPragmaPrefixFormatter : public spdlog::custom_flag_formatter {
  public:
	virtual void format(const spdlog::details::log_msg &msg, const std::tm &tm, spdlog::memory_buf_t &dest) override
	{
		switch(msg.level) {
		case spdlog::level::level_enum::warn:
			{
				static auto prefix = "[warning] " + Con::COLOR_WARNING;
				static std::string prefixNoCol = "[warning] ";
				constexpr uint32_t prefixLen = 10;
				msg.color_range_start = dest.size() + 1;
				msg.color_range_end = dest.size() + prefixLen - 2;
				append_string(dest, g_ansiColorCodesEnabled ? prefix : prefixNoCol);
				break;
			}
		case spdlog::level::level_enum::err:
			{
				static auto prefix = "[error] " + Con::COLOR_ERROR;
				static std::string prefixNoCol = "[error] ";
				constexpr uint32_t prefixLen = 8;
				msg.color_range_start = dest.size() + 1;
				msg.color_range_end = dest.size() + prefixLen - 2;
				append_string(dest, g_ansiColorCodesEnabled ? prefix : prefixNoCol);
				break;
			}
		case spdlog::level::level_enum::critical:
			{
				static auto prefix = "[critical] " + Con::COLOR_CRITICAL;
				static std::string prefixNoCol = "[critical] ";
				constexpr uint32_t prefixLen = 11;
				msg.color_range_start = dest.size() + 1;
				msg.color_range_end = dest.size() + prefixLen - 2;
				append_string(dest, g_ansiColorCodesEnabled ? prefix : prefixNoCol);
				break;
			}
		case spdlog::level::level_enum::debug:
			{
				static std::string prefix = "[debug] ";
				constexpr uint32_t prefixLen = 8;
				msg.color_range_start = dest.size() + 1;
				msg.color_range_end = dest.size() + prefixLen - 2;
				append_string(dest, prefix);
				break;
			}
		case spdlog::level::level_enum::info:
			{
				static std::string prefix = "[info] ";
				constexpr uint32_t prefixLen = 7;
				msg.color_range_start = dest.size() + 1;
				msg.color_range_end = dest.size() + prefixLen - 2;
				append_string(dest, prefix);
				break;
			}
		}
	}

	virtual std::unique_ptr<custom_flag_formatter> clone() const override { return spdlog::details::make_unique<SpdPragmaPrefixFormatter>(); }
};

namespace pragma::logging::detail {
	extern DLLNETWORK std::atomic<bool> shouldLogOutput;
	extern DLLNETWORK std::shared_ptr<spdlog::logger> consoleOutputLogger;
};
static std::optional<std::string> g_logFileName = "log.txt";
static bool g_loggerInitialized = false;
std::optional<std::string> pragma::detail::get_log_file_name() { return g_logFileName; }

void pragma::detail::close_logger()
{
	pragma::logging::detail::shouldLogOutput = false;
	pragma::logging::detail::consoleOutputLogger = nullptr;

	auto logger = spdlog::get(PRAGMA_FILE_LOGGER_NAME);
	if(logger) {
		logger->flush();
		logger = nullptr;
	}
	spdlog::set_default_logger(nullptr);
	spdlog::drop(PRAGMA_LOGGER_NAME);
	spdlog::drop(PRAGMA_FILE_LOGGER_NAME);

	for(auto &logger : g_customLoggers)
		spdlog::drop(logger.first);
	g_customLoggers.clear();

	spdlog::shutdown();
}

static void init_logger(const std::string &name, std::shared_ptr<spdlog::logger> &logger)
{
	if(std::this_thread::get_id() != engine->GetMainThreadId())
		throw std::runtime_error {"Custom loggers must be created on the main thread!"};
	spdlog::info("Creating logger '{}'...", name);
	auto mainLogger = spdlog::get(PRAGMA_LOGGER_NAME);
	logger->sinks() = mainLogger->sinks();
	logger->set_level(mainLogger->level());
	g_customLoggers[name] = logger;
	spdlog::register_logger(logger);
}

spdlog::logger &pragma::get_logger(const std::string &name)
{
	auto it = g_customLoggers.find(name);
	if(it != g_customLoggers.end())
		return *it->second;
	auto logger = std::make_shared<spdlog::logger>(name);
	init_logger(name, logger);
	return *logger;
}

class short_level_formatter_c : public spdlog::custom_flag_formatter {
  public:
	virtual void format(const spdlog::details::log_msg &msg, const std::tm &tm, spdlog::memory_buf_t &dest) override
	{
		std::string v;
		if(g_ansiColorCodesEnabled) {
			switch(msg.level) {
			case spdlog::level::trace:
				break;
			case spdlog::level::debug:
				v = util::get_ansi_color_code(util::ConsoleColorFlags::Cyan);
				break;
			case spdlog::level::info:
			default:
				v = util::get_ansi_color_code(util::ConsoleColorFlags::Green);
				break;
			case spdlog::level::warn:
				v = Con::COLOR_WARNING;
				break;
			case spdlog::level::err:
				v = Con::COLOR_ERROR;
				break;
			case spdlog::level::critical:
				v = Con::COLOR_CRITICAL;
				break;
			}
		}
		v += spdlog::level::to_short_c_str(msg.level);
		if(g_ansiColorCodesEnabled)
			util::get_ansi_color_code(util::ConsoleColorFlags::Reset);
		dest.append(v.data(), v.data() + v.length());
	}

	virtual std::unique_ptr<custom_flag_formatter> clone() const override { return spdlog::details::make_unique<short_level_formatter_c>(); }
};

class color_reset_formatter : public spdlog::custom_flag_formatter {
  public:
	virtual void format(const spdlog::details::log_msg &msg, const std::tm &tm, spdlog::memory_buf_t &dest) override
	{
		if(g_ansiColorCodesEnabled) {
			static auto prefix = Con::COLOR_RESET;
			append_string(dest, prefix);
		}
	}

	virtual std::unique_ptr<custom_flag_formatter> clone() const override { return spdlog::details::make_unique<color_reset_formatter>(); }
};

class color_formatter : public spdlog::custom_flag_formatter {
  public:
	virtual void format(const spdlog::details::log_msg &msg, const std::tm &tm, spdlog::memory_buf_t &dest) override
	{
		if(!g_ansiColorCodesEnabled)
			return;
		switch(msg.level) {
		case spdlog::level::level_enum::warn:
			{
				static auto str = Con::COLOR_WARNING;
				append_string(dest, str);
				break;
			}
		case spdlog::level::level_enum::err:
			{
				static auto str = Con::COLOR_ERROR;
				append_string(dest, str);
				break;
			}
		case spdlog::level::level_enum::critical:
			{
				static auto str = Con::COLOR_CRITICAL;
				append_string(dest, str);
				break;
			}
		}
	}

	virtual std::unique_ptr<custom_flag_formatter> clone() const override { return spdlog::details::make_unique<color_formatter>(); }
};

static std::string CATEGORY_COLOR = util::get_true_color_code(Color {96, 211, 148});
static std::string CATEGORY_PREFIX = "[" + CATEGORY_COLOR;
static std::string CATEGORY_PREFIX_NOCOL = "[";
static std::string CATEGORY_POSTFIX = std::string {Con::COLOR_RESET} + "] ";
static std::string CATEGORY_POSTFIX_NOCOL = "] ";
class category_name_formatter : public spdlog::custom_flag_formatter {
  public:
	virtual void format(const spdlog::details::log_msg &msg, const std::tm &tm, spdlog::memory_buf_t &dest) override
	{
#ifdef SPDLOG_USE_STD_FORMAT
		auto &loggerName = msg.logger_name;
#else
		std::string_view loggerName {msg.logger_name.begin(), msg.logger_name.end()};
#endif
		if(loggerName.size() >= PRAGMA_LOGGER_NAME.length() && loggerName.substr(0, PRAGMA_LOGGER_NAME.length()) == PRAGMA_LOGGER_NAME)
			return; // Don't print category name for main logger
		auto &prefix = g_ansiColorCodesEnabled ? CATEGORY_PREFIX : CATEGORY_PREFIX_NOCOL;
		auto &postfix = g_ansiColorCodesEnabled ? CATEGORY_POSTFIX : CATEGORY_POSTFIX_NOCOL;
		dest.reserve(dest.size() + prefix.size() + msg.logger_name.size() + postfix.size());
		dest.append(prefix);
		dest.append(msg.logger_name);
		dest.append(postfix);
	}

	virtual std::unique_ptr<custom_flag_formatter> clone() const override { return spdlog::details::make_unique<category_name_formatter>(); }
};

static std::unordered_map<std::string, std::shared_ptr<spdlog::logger>> &get_pre_registered_loggers()
{
	static std::unordered_map<std::string, std::shared_ptr<spdlog::logger>> g_preRegisteredLoggers;
	return g_preRegisteredLoggers;
}

spdlog::logger &pragma::register_logger(const std::string &name)
{
	if(g_loggerInitialized)
		return get_logger(name);
	auto it = get_pre_registered_loggers().find(name);
	if(it != get_pre_registered_loggers().end())
		return *it->second;
	auto logger = std::make_shared<spdlog::logger>(name);
	get_pre_registered_loggers()[name] = logger;
	return *logger;
}

void pragma::detail::initialize_logger(::util::LogSeverity conLogLevel, ::util::LogSeverity fileLogLevel, const std::optional<std::string> &logFile)
{
	if(g_loggerInitialized)
		return;
	g_logFileName = logFile;
	auto consoleSink = std::make_shared<anycolor_color_sink_mt>();
	if(g_ansiColorCodesEnabled) {
		consoleSink->set_color(spdlog::level::trace, util::get_ansi_color_code(util::ConsoleColorFlags::Red | util::ConsoleColorFlags::Green | util::ConsoleColorFlags::Blue));
		consoleSink->set_color(spdlog::level::debug, util::get_ansi_color_code(util::ConsoleColorFlags::Green | util::ConsoleColorFlags::Blue));
		consoleSink->set_color(spdlog::level::info, util::get_true_color_code(Color {138, 201, 38})); // Green
		consoleSink->set_color(spdlog::level::warn, Con::COLOR_WARNING);
		consoleSink->set_color(spdlog::level::err, Con::COLOR_ERROR);
		consoleSink->set_color(spdlog::level::critical, Con::COLOR_CRITICAL);
		consoleSink->set_color(spdlog::level::off, Con::COLOR_RESET);
	}
	consoleSink->set_level(static_cast<spdlog::level::level_enum>(pragma::logging::severity_to_spdlog_level(conLogLevel)));

	auto formatter = std::make_unique<spdlog::pattern_formatter>();
	formatter->add_flag<SpdPragmaPrefixFormatter>('*');
	formatter->add_flag<short_level_formatter_c>('q');
	formatter->add_flag<color_reset_formatter>('Q');
	formatter->add_flag<category_name_formatter>('j');
	formatter->set_pattern("%j%*%v%Q");
	consoleSink->set_formatter(std::move(formatter));

	std::vector<spdlog::sink_ptr> sinks {};
	sinks.push_back(consoleSink);
	if(logFile.has_value()) {
		auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(*logFile, true);
		fileSink->set_level(static_cast<spdlog::level::level_enum>(pragma::logging::severity_to_spdlog_level(fileLogLevel)));
		auto formatter = std::make_unique<spdlog::pattern_formatter>();
		formatter->add_flag<short_level_formatter_c>('q');
		formatter->add_flag<color_reset_formatter>('Q');
		formatter->add_flag<color_formatter>('w');
		formatter->add_flag<category_name_formatter>('j');
		formatter->set_pattern("[%H:%M:%S.%e] [%^%q%$] %j%w%v%Q");
		fileSink->set_formatter(std::move(formatter));

		sinks.push_back(fileSink);

		// We want to log all regular console output to file, so we'll create an additional logger to handle that
		auto conFileLogger = std::make_shared<spdlog::logger>(PRAGMA_FILE_LOGGER_NAME, spdlog::sinks_init_list {fileSink});
		conFileLogger->set_level(spdlog::level::trace); // Always log all regular console output to file
		pragma::logging::detail::shouldLogOutput = true;
		pragma::logging::detail::consoleOutputLogger = conFileLogger;
	}

	auto logger = std::make_shared<spdlog::logger>(PRAGMA_LOGGER_NAME, sinks.begin(), sinks.end());
	logger->set_level(static_cast<spdlog::level::level_enum>(pragma::logging::severity_to_spdlog_level(static_cast<::util::LogSeverity>(umath::min(umath::to_integral(conLogLevel), umath::to_integral(fileLogLevel))))));
	spdlog::set_default_logger(logger);

	spdlog::info("Logging has started with logging level {}/{}.", magic_enum::enum_name(conLogLevel), magic_enum::enum_name(fileLogLevel));
	if(logFile.has_value()) {
		spdlog::info("Log will be written to file '{}'.", *logFile);
		if(pragma::logging::detail::consoleOutputLogger)
			pragma::logging::detail::consoleOutputLogger->info("This log file contains ANSI color codes. To properly view the log, you need a text-editor with ANSI color support.");
	}
	else
		spdlog::info("Log file has been disabled, log will not be written to disk.");

	g_loggerInitialized = true;
	for(auto &pair : get_pre_registered_loggers())
		init_logger(pair.first, pair.second);
	get_pre_registered_loggers().clear();
}
