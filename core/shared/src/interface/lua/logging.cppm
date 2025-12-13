// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:scripting.lua.logging;

export import :core.logging;
import pragma.lua;

export namespace Lua::logging {
	DLLNETWORK std::string to_string(lua::State *l, int i);

	template<size_t N>
	void log_with_args(const std::string &loggerIdentifier, const char *msg, spdlog::level::level_enum logLevel, lua::State *l, int32_t argOffset)
	{
		std::array<std::string, N> args;
		for(size_t i = 0; i < args.size(); ++i)
			args[i] = to_string(l, argOffset + (i + 1));

		auto &logger = pragma::register_logger(loggerIdentifier);
		auto log = [&](const auto &...elements) { logger.log(logLevel, std::vformat(msg, std::make_format_args(elements...))); };
		std::apply(log, args);
	}

	template<spdlog::level::level_enum TLevel, int LOG_FUNC(lua::State *, spdlog::level::level_enum)>
	void add_log_func(lua::State *l, luabind::object &oClass, const char *name)
	{
		Lua::PushCFunction(l, +[](lua::State *l) -> int { return LOG_FUNC(l, TLevel); });
		oClass[name] = luabind::object {luabind::from_stack(l, -1)};
		Pop(l, 1);
	}
};
