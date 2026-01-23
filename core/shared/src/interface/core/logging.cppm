// SPDX-FileCopyrightText: (c) 2023 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:core.logging;

export import pragma.util;

export namespace pragma {
	constexpr auto DEFAULT_CONSOLE_LOG_LEVEL = util::LogSeverity::Warning;
	constexpr auto DEFAULT_FILE_LOG_LEVEL = util::LogSeverity::Info;
	constexpr auto DEFAULT_LOG_FILE = "log.txt";
	namespace logging {
		DLLNETWORK int32_t severity_to_spdlog_level(util::LogSeverity severity);
		DLLNETWORK util::LogSeverity spdlog_level_to_severity(int32_t spdlogLevel);
		DLLNETWORK void set_ansi_color_codes_enabled(bool enabled);
	};
	DLLNETWORK spdlog::logger &get_logger(const std::string &name);
	DLLNETWORK spdlog::logger &register_logger(const std::string &name, const std::optional<util::LogSeverity> &defaultLogLevel = {});
	DLLNETWORK void log(const std::string &msg, util::LogSeverity severity = util::LogSeverity::Info);
	DLLNETWORK bool is_log_level_enabled(util::LogSeverity severity);
	DLLNETWORK void flush_loggers();
	DLLNETWORK void set_console_log_level(util::LogSeverity level);
	DLLNETWORK util::LogSeverity get_console_log_level();
	DLLNETWORK void set_file_log_level(util::LogSeverity level);
	DLLNETWORK util::LogSeverity get_file_log_level();

	namespace detail {
		DLLNETWORK void initialize_logger(util::LogSeverity conLogLevel = DEFAULT_CONSOLE_LOG_LEVEL, util::LogSeverity fileLogLevel = DEFAULT_FILE_LOG_LEVEL, const std::optional<std::string> &logFile = DEFAULT_LOG_FILE);
		DLLNETWORK void close_logger();
		DLLNETWORK std::optional<std::string> get_log_file_name();
	};
};
