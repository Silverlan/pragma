// SPDX-FileCopyrightText: (c) 2023 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __BASE_ENTITY_COMPONENT_LOGGING_HPP__
#define __BASE_ENTITY_COMPONENT_LOGGING_HPP__

#include "pragma/entities/components/base_entity_component.hpp"
#include "pragma/logging.hpp"
#include "pragma/engine.h"
#include "pragma/game/game.h"

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

template<typename TClass>
spdlog::logger *find_logger(Game &game) {
	std::type_index typeIndex = typeid(TClass);
	auto &componentManager = game.GetEntityComponentManager();
	pragma::ComponentId componentId;
	if (componentManager.GetComponentId(typeIndex, componentId)) {
		auto *info = componentManager.GetComponentInfo(componentId);
		if (info)
			return &pragma::register_logger("c_" + std::string {info->name.str});
	}
	return nullptr;
}

template<typename TClass>
spdlog::logger &pragma::BaseEntityComponent::get_logger() {
	auto *engine = pragma::get_engine();

	for (auto *state : {Engine::Get()->GetClientState(), Engine::Get()->GetServerNetworkState()}) {
		if (!state)
			continue;
		auto *game = state->GetGameState();
		if (!game)
			continue;
		auto *logger = find_logger<TClass>(*game);
		if (logger)
			return *logger;
	}
	return pragma::register_logger("c_unknown");
}

#endif
