/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __BASE_ENTITY_COMPONENT_LOGGING_HPP__
#define __BASE_ENTITY_COMPONENT_LOGGING_HPP__

#include "pragma/entities/components/base_entity_component.hpp"
#include "pragma/logging.hpp"

template<typename... Args>
void pragma::BaseEntityComponent::Log(spdlog::level::level_enum level, const std::string &msg) const
{
	auto &logger = InitLogger();
	logger.log(level, msg);
}
#ifdef _WIN32
template<typename... Args>
void pragma::BaseEntityComponent::Log(spdlog::level::level_enum level, std::format_string<Args...> fmt, Args &&...args) const
{
	auto &logger = InitLogger();
	logger.log(level, fmt, std::forward<Args>(args)...);
}
template<typename... Args>
void pragma::BaseEntityComponent::LogTrace(std::format_string<Args...> fmt, Args &&...args) const
{
	auto &logger = InitLogger();
	logger.trace(fmt, std::forward<Args>(args)...);
}
template<typename... Args>
void pragma::BaseEntityComponent::LogDebug(std::format_string<Args...> fmt, Args &&...args) const
{
	auto &logger = InitLogger();
	logger.debug(fmt, std::forward<Args>(args)...);
}
template<typename... Args>
void pragma::BaseEntityComponent::LogInfo(std::format_string<Args...> fmt, Args &&...args) const
{
	auto &logger = InitLogger();
	logger.info(fmt, std::forward<Args>(args)...);
}
template<typename... Args>
void pragma::BaseEntityComponent::LogWarn(std::format_string<Args...> fmt, Args &&...args) const
{
	auto &logger = InitLogger();
	logger.warn(fmt, std::forward<Args>(args)...);
}
template<typename... Args>
void pragma::BaseEntityComponent::LogError(std::format_string<Args...> fmt, Args &&...args) const
{
	auto &logger = InitLogger();
	logger.error(fmt, std::forward<Args>(args)...);
}
template<typename... Args>
void pragma::BaseEntityComponent::LogCritical(std::format_string<Args...> fmt, Args &&...args) const
{
	auto &logger = InitLogger();
	logger.critical(fmt, std::forward<Args>(args)...);
}
#else
template<typename... Args>
void pragma::BaseEntityComponent::Log(spdlog::level::level_enum level, const std::string &fmt, Args &&...args) const
{
	auto &logger = InitLogger();
	logger.log(level, std::vformat(fmt, std::make_format_args(std::forward<Args>(args)...)));
}
template<typename... Args>
void pragma::BaseEntityComponent::LogTrace(const std::string &fmt, Args &&...args) const
{
	auto &logger = InitLogger();
	logger.trace(std::vformat(fmt, std::make_format_args(std::forward<Args>(args)...)));
}
template<typename... Args>
void pragma::BaseEntityComponent::LogDebug(const std::string &fmt, Args &&...args) const
{
	auto &logger = InitLogger();
	logger.debug(std::vformat(fmt, std::make_format_args(std::forward<Args>(args)...)));
}
template<typename... Args>
void pragma::BaseEntityComponent::LogInfo(const std::string &fmt, Args &&...args) const
{
	auto &logger = InitLogger();
	logger.info(std::vformat(fmt, std::make_format_args(std::forward<Args>(args)...)));
}
template<typename... Args>
void pragma::BaseEntityComponent::LogWarn(const std::string &fmt, Args &&...args) const
{
	auto &logger = InitLogger();
	logger.warn(std::vformat(fmt, std::make_format_args(std::forward<Args>(args)...)));
}
template<typename... Args>
void pragma::BaseEntityComponent::LogError(const std::string &fmt, Args &&...args) const
{
	auto &logger = InitLogger();
	logger.error(std::vformat(fmt, std::make_format_args(std::forward<Args>(args)...)));
}
template<typename... Args>
void pragma::BaseEntityComponent::LogCritical(const std::string &fmt, Args &&...args) const
{
	auto &logger = InitLogger();
	logger.critical(std::vformat(fmt, std::make_format_args(std::forward<Args>(args)...)));
}
#endif

#endif
