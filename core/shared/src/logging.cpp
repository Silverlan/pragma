/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/logging.hpp"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/pattern_formatter.h>
#include <spdlog/formatter.h>
#include <spdlog/fmt/bundled/format.h>

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

void pragma::log(const std::string &msg, util::LogSeverity severity) { spdlog::log(static_cast<spdlog::level::level_enum>(pragma::logging::severity_to_spdlog_level(severity)), msg); }

bool pragma::is_log_level_enabled(::util::LogSeverity severity) { return spdlog::should_log(static_cast<spdlog::level::level_enum>(pragma::logging::severity_to_spdlog_level(severity))); }

void pragma::flush_loggers()
{
	auto logger0 = spdlog::get("pragma_logger");
	auto logger1 = spdlog::get("pragma_file_logger");
	if(logger0)
		logger0->flush();
	if(logger1)
		logger1->flush();
}

/////////////////////////////

class SpdPragmaPrefixFormatter : public spdlog::custom_flag_formatter {
  public:
	virtual void format(const spdlog::details::log_msg &msg, const std::tm &tm, spdlog::memory_buf_t &dest) override
	{
		switch(msg.level) {
		case spdlog::level::level_enum::warn:
			{
				constexpr const char *prefix = "[warning] " PRAGMA_CON_COLOR_WARNING;
				constexpr uint32_t prefixLen = 10;
				msg.color_range_start = dest.size() + 1;
				msg.color_range_end = dest.size() + prefixLen - 2;
				dest.append(prefix, prefix + prefixLen + ustring::length(PRAGMA_CON_COLOR_WARNING));
				break;
			}
		case spdlog::level::level_enum::err:
			{
				constexpr const char *prefix = "[error] " PRAGMA_CON_COLOR_ERROR;
				constexpr uint32_t prefixLen = 8;
				msg.color_range_start = dest.size() + 1;
				msg.color_range_end = dest.size() + prefixLen - 2;
				dest.append(prefix, prefix + prefixLen + ustring::length(PRAGMA_CON_COLOR_ERROR));
				break;
			}
		case spdlog::level::level_enum::critical:
			{
				constexpr const char *prefix = "[critical] " PRAGMA_CON_COLOR_CRITICAL;
				constexpr uint32_t prefixLen = 11;
				msg.color_range_start = dest.size() + 1;
				msg.color_range_end = dest.size() + prefixLen - 2;
				dest.append(prefix, prefix + prefixLen + ustring::length(PRAGMA_CON_COLOR_CRITICAL));
				break;
			}
		case spdlog::level::level_enum::debug:
			{
				constexpr const char *prefix = "[debug] ";
				constexpr uint32_t prefixLen = 8;
				msg.color_range_start = dest.size() + 1;
				msg.color_range_end = dest.size() + prefixLen - 2;
				dest.append(prefix, prefix + prefixLen);
				break;
			}
		case spdlog::level::level_enum::info:
			{
				constexpr const char *prefix = "[info] ";
				constexpr uint32_t prefixLen = 7;
				msg.color_range_start = dest.size() + 1;
				msg.color_range_end = dest.size() + prefixLen - 2;
				dest.append(prefix, prefix + prefixLen);
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
std::optional<std::string> pragma::detail::get_log_file_name() { return g_logFileName; }
void pragma::detail::close_logger()
{
	pragma::logging::detail::shouldLogOutput = false;
	pragma::logging::detail::consoleOutputLogger = nullptr;

	auto logger = spdlog::get("pragma_file_logger");
	if(logger) {
		logger->flush();
		logger = nullptr;
	}
	spdlog::set_default_logger(nullptr);
	spdlog::drop("pragma_logger");
	spdlog::drop("pragma_file_logger");
}

class short_level_formatter_c : public spdlog::custom_flag_formatter {
  public:
	virtual void format(const spdlog::details::log_msg &msg, const std::tm &tm, spdlog::memory_buf_t &dest) override
	{
		std::string v;
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
			v = util::get_ansi_color_code(util::ConsoleColorFlags::Yellow | util::ConsoleColorFlags::Intensity);
			break;
		case spdlog::level::err:
			v = util::get_ansi_color_code(util::ConsoleColorFlags::Red | util::ConsoleColorFlags::Intensity);
			break;
		case spdlog::level::critical:
			v = util::get_ansi_color_code(util::ConsoleColorFlags::White | util::ConsoleColorFlags::Intensity | util::ConsoleColorFlags::BackgroundRed);
			break;
		}
		v += spdlog::level::to_short_c_str(msg.level) + util::get_ansi_color_code(util::ConsoleColorFlags::Reset);
		dest.append(v.data(), v.data() + v.length());
	}

	virtual std::unique_ptr<custom_flag_formatter> clone() const override { return spdlog::details::make_unique<short_level_formatter_c>(); }
};

class color_reset_formatter : public spdlog::custom_flag_formatter {
  public:
	virtual void format(const spdlog::details::log_msg &msg, const std::tm &tm, spdlog::memory_buf_t &dest) override
	{
		constexpr const char *prefix = PRAGMA_CON_COLOR_RESET;
		constexpr uint32_t prefixLen = ustring::length(PRAGMA_CON_COLOR_RESET);
		dest.append(prefix, prefix + prefixLen);
	}

	virtual std::unique_ptr<custom_flag_formatter> clone() const override { return spdlog::details::make_unique<color_reset_formatter>(); }
};

class color_formatter : public spdlog::custom_flag_formatter {
  public:
	virtual void format(const spdlog::details::log_msg &msg, const std::tm &tm, spdlog::memory_buf_t &dest) override
	{
		switch(msg.level) {
		case spdlog::level::level_enum::warn:
			{
				constexpr const char *str = PRAGMA_CON_COLOR_WARNING;
				dest.append(str, str + ustring::length(PRAGMA_CON_COLOR_WARNING));
				break;
			}
		case spdlog::level::level_enum::err:
			{
				constexpr const char *str = PRAGMA_CON_COLOR_ERROR;
				dest.append(str, str + ustring::length(PRAGMA_CON_COLOR_ERROR));
				break;
			}
		case spdlog::level::level_enum::critical:
			{
				constexpr const char *str = PRAGMA_CON_COLOR_CRITICAL;
				dest.append(str, str + ustring::length(PRAGMA_CON_COLOR_CRITICAL));
				break;
			}
		}
	}

	virtual std::unique_ptr<custom_flag_formatter> clone() const override { return spdlog::details::make_unique<color_formatter>(); }
};

void pragma::detail::initialize_logger(::util::LogSeverity conLogLevel, ::util::LogSeverity fileLogLevel, const std::optional<std::string> &logFile)
{
	g_logFileName = logFile;
	auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
	consoleSink->set_level(static_cast<spdlog::level::level_enum>(pragma::logging::severity_to_spdlog_level(conLogLevel)));

	auto formatter = std::make_unique<spdlog::pattern_formatter>();
	formatter->add_flag<SpdPragmaPrefixFormatter>('*');
	formatter->add_flag<short_level_formatter_c>('q');
	formatter->add_flag<color_reset_formatter>('Q');
	formatter->set_pattern("%*%v%Q");
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
		formatter->set_pattern("[%H:%M:%S %z] [%^%q%$] %w%v%Q");
		fileSink->set_formatter(std::move(formatter));

		sinks.push_back(fileSink);

		// We want to log all regular console output to file, so we'll create an additional logger to handle that
		auto conFileLogger = std::make_shared<spdlog::logger>("pragma_file_logger", spdlog::sinks_init_list {fileSink});
		conFileLogger->set_level(spdlog::level::trace); // Always log all regular console output to file
		pragma::logging::detail::shouldLogOutput = true;
		pragma::logging::detail::consoleOutputLogger = conFileLogger;
	}

	auto logger = std::make_shared<spdlog::logger>("pragma_logger", sinks.begin(), sinks.end());
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
}
