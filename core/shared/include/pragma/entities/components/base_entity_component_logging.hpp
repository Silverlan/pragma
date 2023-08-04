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

template<typename... Args>
void pragma::BaseEntityComponent::Log(spdlog::level::level_enum level, format_string_t<Args...> fmt, Args &&...args) const
{
	auto &logger = InitLogger();
	logger.log(level, fmt, std::forward<Args>(args)...);
}
template<typename... Args>
void pragma::BaseEntityComponent::LogTrace(format_string_t<Args...> fmt, Args &&...args) const
{
	auto &logger = InitLogger();
	logger.trace(fmt, std::forward<Args>(args)...);
}
template<typename... Args>
void pragma::BaseEntityComponent::LogDebug(format_string_t<Args...> fmt, Args &&...args) const
{
	auto &logger = InitLogger();
	logger.debug(fmt, std::forward<Args>(args)...);
}
template<typename... Args>
void pragma::BaseEntityComponent::LogInfo(format_string_t<Args...> fmt, Args &&...args) const
{
	auto &logger = InitLogger();
	logger.info(fmt, std::forward<Args>(args)...);
}
template<typename... Args>
void pragma::BaseEntityComponent::LogWarn(format_string_t<Args...> fmt, Args &&...args) const
{
	auto &logger = InitLogger();
	logger.warn(fmt, std::forward<Args>(args)...);
}
template<typename... Args>
void pragma::BaseEntityComponent::LogError(format_string_t<Args...> fmt, Args &&...args) const
{
	auto &logger = InitLogger();
	logger.error(fmt, std::forward<Args>(args)...);
}
template<typename... Args>
void pragma::BaseEntityComponent::LogCritical(format_string_t<Args...> fmt, Args &&...args) const
{
	auto &logger = InitLogger();
	logger.critical(fmt, std::forward<Args>(args)...);
}

#endif
