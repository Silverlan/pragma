// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "pragma/networkdefinitions.h"
#include <format>
#include "pragma/logging.hpp"
#include "pragma/lua/core.hpp"

export module pragma.shared:scripting.lua.logging;

export import :core.logging;
import pragma.lua;

export namespace Lua::logging {
	DLLNETWORK std::string to_string(lua_State *l, int i);

	template<size_t N>
	void log_with_args(const std::string &loggerIdentifier, const char *msg, spdlog::level::level_enum logLevel, lua_State *l, int32_t argOffset)
	{
		std::array<std::string, N> args;
		for(size_t i = 0; i < args.size(); ++i)
			args[i] = to_string(l, argOffset + (i + 1));

		auto &logger = pragma::register_logger(loggerIdentifier);
		auto log = [&](const auto &...elements) { logger.log(logLevel, std::vformat(msg, std::make_format_args(elements...))); };
		std::apply(log, args);
	}

	template<spdlog::level::level_enum TLevel, int LOG_FUNC(lua_State *, spdlog::level::level_enum)>
	void add_log_func(lua_State *l, luabind::object &oClass, const char *name)
	{
		lua_pushcfunction(
		  l, +[](lua_State *l) -> int { return LOG_FUNC(l, TLevel); });
		oClass[name] = luabind::object {luabind::from_stack(l, -1)};
		Lua::Pop(l, 1);
	}
};
